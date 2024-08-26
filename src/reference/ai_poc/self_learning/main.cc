/* Copyright (c) 2023, Canaan Bright Sight Co., Ltd
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 * 1. Redistributions of source code must retain the above copyright
 * notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 * notice, this list of conditions and the following disclaimer in the
 * documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND
 * CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES,
 * INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 * BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include <iostream>
#include <thread>
#include <dirent.h>
#include <unistd.h>
#include <fcntl.h>
#include <termios.h>
#include <chrono>
#include <sys/stat.h>
#include "vi_vo.h"
#include "self_learning.h"

using std::cerr;
using std::cout;
using std::endl;

using namespace std;
using namespace cv;

std::atomic<bool> isp_stop(false);
std::atomic<bool> reg_stop(false);
int flags;

void print_usage(const char *name)
{
    cout << "Usage: " << name << "<kmodel> <crop_w> <crop_h> <topk> <debug_mode> " << endl
         << "For example: " << endl
         << " [for isp]  ./self_learning.elf --------------" << endl
         << "Options:" << endl
         << " 1> kmodel         kmodel文件路径 \n"
         << " 2> crop_w         剪切范围w \n"
         << " 3> crop_h         剪切范围h \n"
         << " 4> thres          判别阈值 \n"
         << " 5> topk           识别范围 \n"
         << " 6> debug_mode     是否需要调试，0、1、2分别表示不调试、简单调试、详细调试\n"
         << "\n"
         << endl;
}

void getFileNames(string path,vector<string>& filenames)
{
    DIR *pDir;
    struct dirent* ptr;
    if(!(pDir = opendir(path.c_str())))
    {
        cout <<"Folder doesn't Exist!"<< endl;
        return;
    }
    while((ptr = readdir(pDir))!=0) 
    {
        if (strcmp(ptr->d_name, ".") != 0 && strcmp(ptr->d_name, "..") != 0)
        {
            filenames.push_back(ptr->d_name);
            // filenames.push_back(path + "/" + ptr->d_name);
        }
    }
    closedir(pDir);
}

bool createFolder(string folderPath) {
    if (mkdir(folderPath.c_str(), S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH) != 0) 
    {
        return false;
    }
    return true;
}

bool isFolderExist(string folderPath) {
    struct stat info;
    if (stat(folderPath.c_str(), &info) != 0) 
    {
        return false;
    }
    return (info.st_mode & S_IFDIR);
}

void dump_binary_file(const char *file_name, char *data, const size_t size)
{
    std::ofstream outf;
    outf.open(file_name, std::ofstream::binary);
    outf.write(data, size);
    outf.close();
}

//设置终端属性
void set_terminal_mode(bool buffered) {
    struct termios t;
    tcgetattr(STDIN_FILENO, &t);

    if (buffered) {
        t.c_lflag |= ICANON | ECHO;  // 启用缓冲和回显
    } else {
        t.c_lflag &= ~(ICANON | ECHO);  // 禁用缓冲和回显
    }

    tcsetattr(STDIN_FILENO, TCSANOW, &t);
}

void set_read_block_mode(bool blocked)
{
    if (flags != -1)
    {
        if (blocked)
        {   
            // 设置为阻塞模式
            flags &= ~O_NONBLOCK;
            fcntl(STDIN_FILENO, F_SETFL, flags);
        }
        else
        {
            // 设置为非阻塞模式
            flags |= O_NONBLOCK;
            fcntl(STDIN_FILENO, F_SETFL, flags);
        }
    }
}

void video_proc_v1(char *argv[])
{
    vivcap_start();

    k_video_frame_info vf_info;
    void *pic_vaddr = NULL;       //osd

    memset(&vf_info, 0, sizeof(vf_info));

    vf_info.v_frame.width = osd_width;
    vf_info.v_frame.height = osd_height;
    vf_info.v_frame.stride[0] = osd_width;
    vf_info.v_frame.pixel_format = PIXEL_FORMAT_ARGB_8888;
    block = vo_insert_frame(&vf_info, &pic_vaddr);

    // alloc memory
    size_t paddr = 0;
    void *vaddr = nullptr;
    size_t size = SENSOR_CHANNEL * SENSOR_HEIGHT * SENSOR_WIDTH;
    int ret = kd_mpi_sys_mmz_alloc_cached(&paddr, &vaddr, "allocate", "anonymous", size);
    if (ret)
    {
        std::cerr << "physical_memory_block::allocate failed: ret = " << ret << ", errno = " << strerror(errno) << std::endl;
        std::abort();
    }

    FrameSize crop_wh;
    crop_wh.width = atoi(argv[2]);
    crop_wh.height = atoi(argv[3]);

    crop_wh.width = std::min(int(crop_wh.width),SENSOR_WIDTH - 5);
    crop_wh.width = std::max(int(crop_wh.width),(SENSOR_WIDTH - 5)/2);
    crop_wh.height = std::min(int(crop_wh.height),SENSOR_HEIGHT - 5);
    crop_wh.height = std::max(int(crop_wh.height),(SENSOR_HEIGHT - 5)/2);

    Bbox crop_box_osd;
    crop_box_osd.x = osd_width / 2.0 - float(crop_wh.width) / SENSOR_WIDTH * osd_width / 2.0;
    crop_box_osd.y = osd_height / 2.0 - float(crop_wh.height) / SENSOR_HEIGHT * osd_height / 2.0;
    crop_box_osd.w = float(crop_wh.width) / SENSOR_WIDTH * osd_width;
    crop_box_osd.h = float(crop_wh.height) / SENSOR_HEIGHT * osd_height;


    if (!isFolderExist("features"))
    {
        createFolder("features");
    }

    //only for face reg
    int flags = fcntl(STDIN_FILENO, F_GETFL, 0);
    if (flags == -1)
    {
        cerr << "Failed to get flags for stdin." << endl;
    }
    set_terminal_mode(false);
    set_read_block_mode(false);

    float thres = atof(argv[4]);
    SelfLearning self_learning(argv[1], crop_wh, thres, atoi(argv[5]),{SENSOR_CHANNEL, SENSOR_HEIGHT, SENSOR_WIDTH}, reinterpret_cast<uintptr_t>(vaddr), reinterpret_cast<uintptr_t>(paddr), atoi(argv[6]));

    while (!isp_stop)
    {
        ScopedTiming st("total time", 1);

        {
            ScopedTiming st("read capture", atoi(argv[6]));
            // VICAP_CHN_ID_1 out rgb888p
            memset(&dump_info, 0 , sizeof(k_video_frame_info));
            ret = kd_mpi_vicap_dump_frame(vicap_dev, VICAP_CHN_ID_1, VICAP_DUMP_YUV, &dump_info, 1000);
            if (ret) {
                printf("sample_vicap...kd_mpi_vicap_dump_frame failed.\n");
                continue;
            }
        }
        

        {
            ScopedTiming st("isp copy", atoi(argv[6]));
            // 从vivcap中读取一帧图像到dump_info
            auto vbvaddr = kd_mpi_sys_mmap_cached(dump_info.v_frame.phys_addr[0], size);
            memcpy(vaddr, (void *)vbvaddr, SENSOR_HEIGHT * SENSOR_WIDTH * 3);  // 这里以后可以去掉，不用copy
            kd_mpi_sys_munmap(vbvaddr, size);
        }

        cv::Mat osd_frame(osd_height, osd_width, CV_8UC4, cv::Scalar(0, 0, 0, 0));
        cv::rectangle(osd_frame, cv::Rect(crop_box_osd.x, crop_box_osd.y , crop_box_osd.w, crop_box_osd.h), cv::Scalar(255, 255, 255), 2, 2, 0);

        self_learning.pre_process();
        self_learning.inference();

        char ch;    
        if (read(STDIN_FILENO, &ch, 1) > 0)
        {
            if (ch == 'i')      //for i key
            {
                set_terminal_mode(true);
                set_read_block_mode(true);

                std::cout << "<<< input: n -> create new category feature, d -> delete one category feature >>>" << std::endl;
                std::string input_;
                getline(std::cin, input_);

                std::vector<std::string> features;
                getFileNames("features", features);
                std::string features_out = "Already have : ";
                for (int i = 0; i < features.size(); i++)
                {
                    features_out += " | " + std::to_string(i) + "->" + features[i];
                }
                std::cout << features_out << std::endl;

                if (input_ == "n")
                {
                    std::cout << "Please enter one filename. format : {category}_{index}.bin  eg: apple_0.bin" << std::endl;
                    std::string input_feature;
                    getline(std::cin, input_feature);
                    int len_;
                    float *output = self_learning.get_kpu_output(&len_);
                    dump_binary_file(("features/" + input_feature).c_str(), (char *)output, (const size_t) len_ * sizeof(float));
                    std::cout << "Successfully created " + input_feature + " feature." << std::endl;
                }
                else if (input_ == "d")
                {
                    std::cout << "Please enter one feature index." << std::endl;
                    std::string str_index;
                    getline(std::cin, str_index);
                    int index = str_index[0] - 48;
                    if (index < 0 || index >= features.size())
                    {
                        std::cout << "Fail to delete." << std::endl;
                    }
                    else
                    {
                        remove(("features/" + features[index]).c_str());
                        std::cout << "Successfully deleted " + features[index] + "." << std::endl;
                    }
                }

                set_read_block_mode(false);
                set_terminal_mode(false);
            }
            else if(ch == 27)         //for ESC key
            {
                reg_stop = true;
            }
        }
        else
        {
            std::vector<Evec> results;
            std::vector<std::string> features;
            getFileNames("features", features);
            self_learning.post_process(features, results);
            cv::Point origin;
            origin.x = 200;
            origin.y = 100;
            for (int i = 0; i < results.size(); i++)
            {

                origin.y += 50;
                std::string text = results[i].category + " " + std::to_string(results[i].score);
                cv::putText(osd_frame, text, origin, cv::FONT_HERSHEY_COMPLEX, 2, cv::Scalar(255,255, 0, 0), 4, 8, 0);
            }
        }

        {
            ScopedTiming st("osd copy", atoi(argv[6]));
            
            memcpy(pic_vaddr, osd_frame.data, osd_width * osd_height * 4);
            //显示通道插入帧
            kd_mpi_vo_chn_insert_frame(osd_id+3, &vf_info);  //K_VO_OSD0
            // printf("kd_mpi_vo_chn_insert_frame success \n");

            ret = kd_mpi_vicap_dump_release(vicap_dev, VICAP_CHN_ID_1, &dump_info);
            if (ret) {
                printf("sample_vicap...kd_mpi_vicap_dump_release failed.\n");
            }
        }    
    }

    vo_osd_release_block();
    vivcap_stop();


    // free memory
    ret = kd_mpi_sys_mmz_free(paddr, vaddr);
    if (ret)
    {
        std::cerr << "free failed: ret = " << ret << ", errno = " << strerror(errno) << std::endl;
        std::abort();
    }

}

void video_proc_v2(char *argv[])
{
    vivcap_start();

    k_video_frame_info vf_info;
    void *pic_vaddr = NULL;       //osd

    memset(&vf_info, 0, sizeof(vf_info));

    vf_info.v_frame.width = osd_width;
    vf_info.v_frame.height = osd_height;
    vf_info.v_frame.stride[0] = osd_width;
    vf_info.v_frame.pixel_format = PIXEL_FORMAT_ARGB_8888;
    block = vo_insert_frame(&vf_info, &pic_vaddr);

    // alloc memory
    size_t paddr = 0;
    void *vaddr = nullptr;
    size_t size = SENSOR_CHANNEL * SENSOR_HEIGHT * SENSOR_WIDTH;
    int ret = kd_mpi_sys_mmz_alloc_cached(&paddr, &vaddr, "allocate", "anonymous", size);
    if (ret)
    {
        std::cerr << "physical_memory_block::allocate failed: ret = " << ret << ", errno = " << strerror(errno) << std::endl;
        std::abort();
    }

    FrameSize crop_wh;
    crop_wh.width = atoi(argv[2]);
    crop_wh.height = atoi(argv[3]);

    crop_wh.width = std::min(int(crop_wh.width),SENSOR_WIDTH - 5);
    crop_wh.width = std::max(int(crop_wh.width),(SENSOR_WIDTH - 5)/2);
    crop_wh.height = std::min(int(crop_wh.height),SENSOR_HEIGHT - 5);
    crop_wh.height = std::max(int(crop_wh.height),(SENSOR_HEIGHT - 5)/2);

    Bbox crop_box_osd;
    crop_box_osd.x = osd_width / 2.0 - float(crop_wh.width) / SENSOR_WIDTH * osd_width / 2.0;
    crop_box_osd.y = osd_height / 2.0 - float(crop_wh.height) / SENSOR_HEIGHT * osd_height / 2.0;
    crop_box_osd.w = float(crop_wh.width) / SENSOR_WIDTH * osd_width;
    crop_box_osd.h = float(crop_wh.height) / SENSOR_HEIGHT * osd_height;



    if (!isFolderExist("features"))
    {
        createFolder("features");
    }

    //only for face reg
    int flags = fcntl(STDIN_FILENO, F_GETFL, 0);
    if (flags == -1)
    {
        cerr << "Failed to get flags for stdin." << endl;
    }
    set_terminal_mode(false);
    set_read_block_mode(false);

    float thres = atof(argv[4]);
    SelfLearning self_learning(argv[1], crop_wh, thres, atoi(argv[5]),{SENSOR_CHANNEL, SENSOR_HEIGHT, SENSOR_WIDTH}, reinterpret_cast<uintptr_t>(vaddr), reinterpret_cast<uintptr_t>(paddr), atoi(argv[6]));

    while (!isp_stop)
    {
        ScopedTiming st("total time", 1);

        {
            ScopedTiming st("read capture", atoi(argv[6]));
            // VICAP_CHN_ID_1 out rgb888p
            memset(&dump_info, 0 , sizeof(k_video_frame_info));
            ret = kd_mpi_vicap_dump_frame(vicap_dev, VICAP_CHN_ID_1, VICAP_DUMP_YUV, &dump_info, 1000);
            if (ret) {
                printf("sample_vicap...kd_mpi_vicap_dump_frame failed.\n");
                continue;
            }
        }
        

        {
            ScopedTiming st("isp copy", atoi(argv[6]));
            // 从vivcap中读取一帧图像到dump_info
            auto vbvaddr = kd_mpi_sys_mmap_cached(dump_info.v_frame.phys_addr[0], size);
            memcpy(vaddr, (void *)vbvaddr, SENSOR_HEIGHT * SENSOR_WIDTH * 3);  // 这里以后可以去掉，不用copy
            kd_mpi_sys_munmap(vbvaddr, size);
        }

        cv::Mat osd_frame(osd_height, osd_width, CV_8UC4, cv::Scalar(0, 0, 0, 0));
        cv::rectangle(osd_frame, cv::Rect(crop_box_osd.x, crop_box_osd.y , crop_box_osd.w, crop_box_osd.h), cv::Scalar(255, 255, 255), 2, 2, 0);

        self_learning.pre_process();
        self_learning.inference();

        char ch;    
        if (read(STDIN_FILENO, &ch, 1) > 0)
        {
            if (ch == 'i')      //for i key
            {
                set_terminal_mode(true);
                set_read_block_mode(true);

                std::cout << "<<< input: n -> create new category feature, d -> delete one category feature >>>" << std::endl;
                std::string input_;
                getline(std::cin, input_);

                std::vector<std::string> features;
                getFileNames("features", features);
                std::string features_out = "Already have : ";
                for (int i = 0; i < features.size(); i++)
                {
                    features_out += " | " + std::to_string(i) + "->" + features[i];
                }
                std::cout << features_out << std::endl;

                if (input_ == "n")
                {
                    std::cout << "Please enter one filename. format : {category}_{index}.bin  eg: apple_0.bin" << std::endl;
                    std::string input_feature;
                    getline(std::cin, input_feature);
                    int len_;
                    float *output = self_learning.get_kpu_output(&len_);
                    dump_binary_file(("features/" + input_feature).c_str(), (char *)output, (const size_t) len_ * sizeof(float));
                    std::cout << "Successfully created " + input_feature + " feature." << std::endl;
                }
                else if (input_ == "d")
                {
                    std::cout << "Please enter one feature index." << std::endl;
                    std::string str_index;
                    getline(std::cin, str_index);
                    int index = str_index[0] - 48;
                    if (index < 0 || index >= features.size())
                    {
                        std::cout << "Fail to delete." << std::endl;
                    }
                    else
                    {
                        remove(("features/" + features[index]).c_str());
                        std::cout << "Successfully deleted " + features[index] + "." << std::endl;
                    }
                }

                set_read_block_mode(false);
                set_terminal_mode(false);
            }
            else if(ch == 27)         //for ESC key
            {
                reg_stop = true;
            }
        }
        else
        {
            std::vector<Evec> results;
            std::vector<std::string> features;
            getFileNames("features", features);
            self_learning.post_process(features, results);
            cv::Point origin;
            origin.x = 200;
            origin.y = 100;
            for (int i = 0; i < results.size(); i++)
            {

                origin.y += 50;
                std::string text = results[i].category + " " + std::to_string(results[i].score);
                cv::putText(osd_frame, text, origin, cv::FONT_HERSHEY_COMPLEX, 2, cv::Scalar(255,255, 0, 0), 4, 8, 0);
            }
        }

        {
            ScopedTiming st("osd copy", atoi(argv[6]));
            
            memcpy(pic_vaddr, osd_frame.data, osd_width * osd_height * 4);
            //显示通道插入帧
            kd_mpi_vo_chn_insert_frame(osd_id+3, &vf_info);  //K_VO_OSD0
            // printf("kd_mpi_vo_chn_insert_frame success \n");

            ret = kd_mpi_vicap_dump_release(vicap_dev, VICAP_CHN_ID_1, &dump_info);
            if (ret) {
                printf("sample_vicap...kd_mpi_vicap_dump_release failed.\n");
            }
        }    
    }

    vo_osd_release_block();
    vivcap_stop();


    // free memory
    ret = kd_mpi_sys_mmz_free(paddr, vaddr);
    if (ret)
    {
        std::cerr << "free failed: ret = " << ret << ", errno = " << strerror(errno) << std::endl;
        std::abort();
    }

}

void video_proc_k230d(char *argv[])
{
    vivcap_start();

    k_video_frame_info vf_info;
    void *pic_vaddr = NULL;       //osd

    memset(&vf_info, 0, sizeof(vf_info));

    vf_info.v_frame.width = osd_width;
    vf_info.v_frame.height = osd_height;
    vf_info.v_frame.stride[0] = osd_width;
    vf_info.v_frame.pixel_format = PIXEL_FORMAT_ARGB_8888;
    block = vo_insert_frame(&vf_info, &pic_vaddr);

    // alloc memory
    size_t paddr = 0;
    void *vaddr = nullptr;
    size_t size = SENSOR_CHANNEL * SENSOR_HEIGHT * SENSOR_WIDTH;
    int ret = kd_mpi_sys_mmz_alloc_cached(&paddr, &vaddr, "allocate", "anonymous", size);
    if (ret)
    {
        std::cerr << "physical_memory_block::allocate failed: ret = " << ret << ", errno = " << strerror(errno) << std::endl;
        std::abort();
    }

    FrameSize crop_wh;
    crop_wh.width = atoi(argv[2]);
    crop_wh.height = atoi(argv[3]);

    crop_wh.width = std::min(int(crop_wh.width),SENSOR_WIDTH - 5);
    crop_wh.width = std::max(int(crop_wh.width),(SENSOR_WIDTH - 5)/2);
    crop_wh.height = std::min(int(crop_wh.height),SENSOR_HEIGHT - 5);
    crop_wh.height = std::max(int(crop_wh.height),(SENSOR_HEIGHT - 5)/2);

    Bbox crop_box_osd;
    crop_box_osd.x = osd_height / 2.0 - float(crop_wh.width) / SENSOR_WIDTH * osd_height / 2.0;
    crop_box_osd.y = osd_width / 2.0 - float(crop_wh.height) / SENSOR_HEIGHT * osd_width / 2.0;
    crop_box_osd.w = float(crop_wh.width) / SENSOR_WIDTH * osd_height;
    crop_box_osd.h = float(crop_wh.height) / SENSOR_HEIGHT * osd_width;


    if (!isFolderExist("features"))
    {
        createFolder("features");
    }

    //only for face reg
    int flags = fcntl(STDIN_FILENO, F_GETFL, 0);
    if (flags == -1)
    {
        cerr << "Failed to get flags for stdin." << endl;
    }
    set_terminal_mode(false);
    set_read_block_mode(false);

    float thres = atof(argv[4]);
    SelfLearning self_learning(argv[1], crop_wh, thres, atoi(argv[5]),{SENSOR_CHANNEL, SENSOR_HEIGHT, SENSOR_WIDTH}, reinterpret_cast<uintptr_t>(vaddr), reinterpret_cast<uintptr_t>(paddr), atoi(argv[6]));

    while (!isp_stop)
    {
        ScopedTiming st("total time", 1);

        {
            ScopedTiming st("read capture", atoi(argv[6]));
            // VICAP_CHN_ID_1 out rgb888p
            memset(&dump_info, 0 , sizeof(k_video_frame_info));
            ret = kd_mpi_vicap_dump_frame(vicap_dev, VICAP_CHN_ID_1, VICAP_DUMP_YUV, &dump_info, 1000);
            if (ret) {
                printf("sample_vicap...kd_mpi_vicap_dump_frame failed.\n");
                continue;
            }
        }
        

        {
            ScopedTiming st("isp copy", atoi(argv[6]));
            // 从vivcap中读取一帧图像到dump_info
            auto vbvaddr = kd_mpi_sys_mmap_cached(dump_info.v_frame.phys_addr[0], size);
            memcpy(vaddr, (void *)vbvaddr, SENSOR_HEIGHT * SENSOR_WIDTH * 3);  // 这里以后可以去掉，不用copy
            kd_mpi_sys_munmap(vbvaddr, size);
        }

        cv::Mat osd_frame(osd_height, osd_width, CV_8UC4, cv::Scalar(0, 0, 0, 0));
        cv::rotate(osd_frame, osd_frame, cv::ROTATE_90_COUNTERCLOCKWISE);
        cv::rectangle(osd_frame, cv::Rect(crop_box_osd.x, crop_box_osd.y , crop_box_osd.w, crop_box_osd.h), cv::Scalar(255, 255, 255), 2, 2, 0);

        self_learning.pre_process();
        self_learning.inference();

        char ch;    
        if (read(STDIN_FILENO, &ch, 1) > 0)
        {
            if (ch == 'i')      //for i key
            {
                set_terminal_mode(true);
                set_read_block_mode(true);

                std::cout << "<<< input: n -> create new category feature, d -> delete one category feature >>>" << std::endl;
                std::string input_;
                getline(std::cin, input_);

                std::vector<std::string> features;
                getFileNames("features", features);
                std::string features_out = "Already have : ";
                for (int i = 0; i < features.size(); i++)
                {
                    features_out += " | " + std::to_string(i) + "->" + features[i];
                }
                std::cout << features_out << std::endl;

                if (input_ == "n")
                {
                    std::cout << "Please enter one filename. format : {category}_{index}.bin  eg: apple_0.bin" << std::endl;
                    std::string input_feature;
                    getline(std::cin, input_feature);
                    int len_;
                    float *output = self_learning.get_kpu_output(&len_);
                    dump_binary_file(("features/" + input_feature).c_str(), (char *)output, (const size_t) len_ * sizeof(float));
                    std::cout << "Successfully created " + input_feature + " feature." << std::endl;
                }
                else if (input_ == "d")
                {
                    std::cout << "Please enter one feature index." << std::endl;
                    std::string str_index;
                    getline(std::cin, str_index);
                    int index = str_index[0] - 48;
                    if (index < 0 || index >= features.size())
                    {
                        std::cout << "Fail to delete." << std::endl;
                    }
                    else
                    {
                        remove(("features/" + features[index]).c_str());
                        std::cout << "Successfully deleted " + features[index] + "." << std::endl;
                    }
                }

                set_read_block_mode(false);
                set_terminal_mode(false);
            }
            else if(ch == 27)         //for ESC key
            {
                reg_stop = true;
            }
        }
        else
        {
            std::vector<Evec> results;
            std::vector<std::string> features;
            getFileNames("features", features);
            self_learning.post_process(features, results);
            cv::Point origin;
            origin.x = 200;
            origin.y = 100;
            for (int i = 0; i < results.size(); i++)
            {

                origin.y += 50;
                std::string text = results[i].category + " " + std::to_string(results[i].score);
                cv::putText(osd_frame, text, origin, cv::FONT_HERSHEY_COMPLEX, 2, cv::Scalar(255,255, 0, 0), 4, 8, 0);
            }
        }
        cv::rotate(osd_frame, osd_frame, cv::ROTATE_90_CLOCKWISE);

        {
            ScopedTiming st("osd copy", atoi(argv[6]));
            
            memcpy(pic_vaddr, osd_frame.data, osd_width * osd_height * 4);
            //显示通道插入帧
            kd_mpi_vo_chn_insert_frame(osd_id+3, &vf_info);  //K_VO_OSD0
            // printf("kd_mpi_vo_chn_insert_frame success \n");

            ret = kd_mpi_vicap_dump_release(vicap_dev, VICAP_CHN_ID_1, &dump_info);
            if (ret) {
                printf("sample_vicap...kd_mpi_vicap_dump_release failed.\n");
            }
        }    
    }

    vo_osd_release_block();
    vivcap_stop();


    // free memory
    ret = kd_mpi_sys_mmz_free(paddr, vaddr);
    if (ret)
    {
        std::cerr << "free failed: ret = " << ret << ", errno = " << strerror(errno) << std::endl;
        std::abort();
    }

}


void video_proc_01(char *argv[])
{
    vivcap_start();

    k_video_frame_info vf_info;
    void *pic_vaddr = NULL;       //osd

    memset(&vf_info, 0, sizeof(vf_info));

    vf_info.v_frame.width = osd_width;
    vf_info.v_frame.height = osd_height;
    vf_info.v_frame.stride[0] = osd_width;
    vf_info.v_frame.pixel_format = PIXEL_FORMAT_ARGB_8888;
    block = vo_insert_frame(&vf_info, &pic_vaddr);

    // alloc memory
    size_t paddr = 0;
    void *vaddr = nullptr;
    size_t size = SENSOR_CHANNEL * SENSOR_HEIGHT * SENSOR_WIDTH;
    int ret = kd_mpi_sys_mmz_alloc_cached(&paddr, &vaddr, "allocate", "anonymous", size);
    if (ret)
    {
        std::cerr << "physical_memory_block::allocate failed: ret = " << ret << ", errno = " << strerror(errno) << std::endl;
        std::abort();
    }

    FrameSize crop_wh;
    crop_wh.width = atoi(argv[2]);
    crop_wh.height = atoi(argv[3]);

    crop_wh.width = std::min(int(crop_wh.width),SENSOR_WIDTH - 5);
    crop_wh.width = std::max(int(crop_wh.width),(SENSOR_WIDTH - 5)/2);
    crop_wh.height = std::min(int(crop_wh.height),SENSOR_HEIGHT - 5);
    crop_wh.height = std::max(int(crop_wh.height),(SENSOR_HEIGHT - 5)/2);

    Bbox crop_box_osd;
    #if defined(STUDIO_HDMI)
    {
        crop_box_osd.x = osd_width / 2.0 - float(crop_wh.width) / SENSOR_WIDTH * osd_width / 2.0;
        crop_box_osd.y = osd_height / 2.0 - float(crop_wh.height) / SENSOR_HEIGHT * osd_height / 2.0;
        crop_box_osd.w = float(crop_wh.width) / SENSOR_WIDTH * osd_width;
        crop_box_osd.h = float(crop_wh.height) / SENSOR_HEIGHT * osd_height;
    }
    #else
    {
        crop_box_osd.x = osd_height / 2.0 - float(crop_wh.width) / SENSOR_WIDTH * osd_height / 2.0;
        crop_box_osd.y = osd_width / 2.0 - float(crop_wh.height) / SENSOR_HEIGHT * osd_width / 2.0;
        crop_box_osd.w = float(crop_wh.width) / SENSOR_WIDTH * osd_height;
        crop_box_osd.h = float(crop_wh.height) / SENSOR_HEIGHT * osd_width;
    }
    #endif



    if (!isFolderExist("features"))
    {
        createFolder("features");
    }

    //only for face reg
    int flags = fcntl(STDIN_FILENO, F_GETFL, 0);
    if (flags == -1)
    {
        cerr << "Failed to get flags for stdin." << endl;
    }
    set_terminal_mode(false);
    set_read_block_mode(false);

    float thres = atof(argv[4]);
    SelfLearning self_learning(argv[1], crop_wh, thres, atoi(argv[5]),{SENSOR_CHANNEL, SENSOR_HEIGHT, SENSOR_WIDTH}, reinterpret_cast<uintptr_t>(vaddr), reinterpret_cast<uintptr_t>(paddr), atoi(argv[6]));

    while (!isp_stop)
    {
        ScopedTiming st("total time", 1);

        {
            ScopedTiming st("read capture", atoi(argv[6]));
            // VICAP_CHN_ID_1 out rgb888p
            memset(&dump_info, 0 , sizeof(k_video_frame_info));
            ret = kd_mpi_vicap_dump_frame(vicap_dev, VICAP_CHN_ID_1, VICAP_DUMP_YUV, &dump_info, 1000);
            if (ret) {
                printf("sample_vicap...kd_mpi_vicap_dump_frame failed.\n");
                continue;
            }
        }
        

        {
            ScopedTiming st("isp copy", atoi(argv[6]));
            // 从vivcap中读取一帧图像到dump_info
            auto vbvaddr = kd_mpi_sys_mmap_cached(dump_info.v_frame.phys_addr[0], size);
            memcpy(vaddr, (void *)vbvaddr, SENSOR_HEIGHT * SENSOR_WIDTH * 3);  // 这里以后可以去掉，不用copy
            kd_mpi_sys_munmap(vbvaddr, size);
        }

        cv::Mat osd_frame(osd_height, osd_width, CV_8UC4, cv::Scalar(0, 0, 0, 0));
        #if defined(STUDIO_HDMI)
        {
            cv::rectangle(osd_frame, cv::Rect(crop_box_osd.x, crop_box_osd.y , crop_box_osd.w, crop_box_osd.h), cv::Scalar(255, 255, 255), 2, 2, 0);

            self_learning.pre_process();
            self_learning.inference();

            char ch;    
            if (read(STDIN_FILENO, &ch, 1) > 0)
            {
                if (ch == 'i')      //for i key
                {
                    set_terminal_mode(true);
                    set_read_block_mode(true);

                    std::cout << "<<< input: n -> create new category feature, d -> delete one category feature >>>" << std::endl;
                    std::string input_;
                    getline(std::cin, input_);

                    std::vector<std::string> features;
                    getFileNames("features", features);
                    std::string features_out = "Already have : ";
                    for (int i = 0; i < features.size(); i++)
                    {
                        features_out += " | " + std::to_string(i) + "->" + features[i];
                    }
                    std::cout << features_out << std::endl;

                    if (input_ == "n")
                    {
                        std::cout << "Please enter one filename. format : {category}_{index}.bin  eg: apple_0.bin" << std::endl;
                        std::string input_feature;
                        getline(std::cin, input_feature);
                        int len_;
                        float *output = self_learning.get_kpu_output(&len_);
                        dump_binary_file(("features/" + input_feature).c_str(), (char *)output, (const size_t) len_ * sizeof(float));
                        std::cout << "Successfully created " + input_feature + " feature." << std::endl;
                    }
                    else if (input_ == "d")
                    {
                        std::cout << "Please enter one feature index." << std::endl;
                        std::string str_index;
                        getline(std::cin, str_index);
                        int index = str_index[0] - 48;
                        if (index < 0 || index >= features.size())
                        {
                            std::cout << "Fail to delete." << std::endl;
                        }
                        else
                        {
                            remove(("features/" + features[index]).c_str());
                            std::cout << "Successfully deleted " + features[index] + "." << std::endl;
                        }
                    }

                    set_read_block_mode(false);
                    set_terminal_mode(false);
                }
                else if(ch == 27)         //for ESC key
                {
                    reg_stop = true;
                }
            }
            else
            {
                std::vector<Evec> results;
                std::vector<std::string> features;
                getFileNames("features", features);
                self_learning.post_process(features, results);
                cv::Point origin;
                origin.x = 200;
                origin.y = 100;
                for (int i = 0; i < results.size(); i++)
                {

                    origin.y += 50;
                    std::string text = results[i].category + " " + std::to_string(results[i].score);
                    cv::putText(osd_frame, text, origin, cv::FONT_HERSHEY_COMPLEX, 2, cv::Scalar(255,255, 0, 0), 4, 8, 0);
                }
            }
        }
        #else
        {
            cv::rotate(osd_frame, osd_frame, cv::ROTATE_90_COUNTERCLOCKWISE);
            cv::rectangle(osd_frame, cv::Rect(crop_box_osd.x, crop_box_osd.y , crop_box_osd.w, crop_box_osd.h), cv::Scalar(255, 255, 255), 2, 2, 0);

            self_learning.pre_process();
            self_learning.inference();

            char ch;    
            if (read(STDIN_FILENO, &ch, 1) > 0)
            {
                if (ch == 'i')      //for i key
                {
                    set_terminal_mode(true);
                    set_read_block_mode(true);

                    std::cout << "<<< input: n -> create new category feature, d -> delete one category feature >>>" << std::endl;
                    std::string input_;
                    getline(std::cin, input_);

                    std::vector<std::string> features;
                    getFileNames("features", features);
                    std::string features_out = "Already have : ";
                    for (int i = 0; i < features.size(); i++)
                    {
                        features_out += " | " + std::to_string(i) + "->" + features[i];
                    }
                    std::cout << features_out << std::endl;

                    if (input_ == "n")
                    {
                        std::cout << "Please enter one filename. format : {category}_{index}.bin  eg: apple_0.bin" << std::endl;
                        std::string input_feature;
                        getline(std::cin, input_feature);
                        int len_;
                        float *output = self_learning.get_kpu_output(&len_);
                        dump_binary_file(("features/" + input_feature).c_str(), (char *)output, (const size_t) len_ * sizeof(float));
                        std::cout << "Successfully created " + input_feature + " feature." << std::endl;
                    }
                    else if (input_ == "d")
                    {
                        std::cout << "Please enter one feature index." << std::endl;
                        std::string str_index;
                        getline(std::cin, str_index);
                        int index = str_index[0] - 48;
                        if (index < 0 || index >= features.size())
                        {
                            std::cout << "Fail to delete." << std::endl;
                        }
                        else
                        {
                            remove(("features/" + features[index]).c_str());
                            std::cout << "Successfully deleted " + features[index] + "." << std::endl;
                        }
                    }

                    set_read_block_mode(false);
                    set_terminal_mode(false);
                }
                else if(ch == 27)         //for ESC key
                {
                    reg_stop = true;
                }
            }
            else
            {
                std::vector<Evec> results;
                std::vector<std::string> features;
                getFileNames("features", features);
                self_learning.post_process(features, results);
                cv::Point origin;
                origin.x = 200;
                origin.y = 100;
                for (int i = 0; i < results.size(); i++)
                {

                    origin.y += 50;
                    std::string text = results[i].category + " " + std::to_string(results[i].score);
                    cv::putText(osd_frame, text, origin, cv::FONT_HERSHEY_COMPLEX, 2, cv::Scalar(255,255, 0, 0), 4, 8, 0);
                }
            }
            cv::rotate(osd_frame, osd_frame, cv::ROTATE_90_CLOCKWISE);
        }
        #endif

        {
            ScopedTiming st("osd copy", atoi(argv[6]));
            
            memcpy(pic_vaddr, osd_frame.data, osd_width * osd_height * 4);
            //显示通道插入帧
            kd_mpi_vo_chn_insert_frame(osd_id+3, &vf_info);  //K_VO_OSD0
            // printf("kd_mpi_vo_chn_insert_frame success \n");

            ret = kd_mpi_vicap_dump_release(vicap_dev, VICAP_CHN_ID_1, &dump_info);
            if (ret) {
                printf("sample_vicap...kd_mpi_vicap_dump_release failed.\n");
            }
        }    
    }

    vo_osd_release_block();
    vivcap_stop();


    // free memory
    ret = kd_mpi_sys_mmz_free(paddr, vaddr);
    if (ret)
    {
        std::cerr << "free failed: ret = " << ret << ", errno = " << strerror(errno) << std::endl;
        std::abort();
    }

}



int main(int argc, char *argv[])
{
    std::cout << "case " << argv[0] << " built at " << __DATE__ << " " << __TIME__ << std::endl;
    std::cout << "Press 'i' to register." << std::endl;
    std::cout << "Press 'ESC' to exit." << std::endl;

    if (argc != 7)
    {
        print_usage(argv[0]);
        return -1;
    }

    {
        #if defined(CONFIG_BOARD_K230_CANMV)
        {
            std::thread thread_isp(video_proc_v1, argv);
            while(!reg_stop)
            {
                usleep(10000);
            }
            
            isp_stop = true;
            thread_isp.join();
            set_read_block_mode(true);
            set_terminal_mode(true);
        }
        #elif defined(CONFIG_BOARD_K230_CANMV_V2)
        {
            std::thread thread_isp(video_proc_v2, argv);
            while(!reg_stop)
            {
                usleep(10000);
            }
            
            isp_stop = true;
            thread_isp.join();
            set_read_block_mode(true);
            set_terminal_mode(true);
        }
        #elif defined(CONFIG_BOARD_K230D_CANMV)
        {
            std::thread thread_isp(video_proc_k230d, argv);
            while(!reg_stop)
            {
                usleep(10000);
            }
            
            isp_stop = true;
            thread_isp.join();
            set_read_block_mode(true);
            set_terminal_mode(true);
        }
        #elif defined(CONFIG_BOARD_K230_CANMV_01STUDIO)
        {
            std::thread thread_isp(video_proc_01, argv);
            while(!reg_stop)
            {
                usleep(10000);
            }
            
            isp_stop = true;
            thread_isp.join();
            set_read_block_mode(true);
            set_terminal_mode(true);
        }
        #else
        {
            std::thread thread_isp(video_proc_v1, argv);
            while(!reg_stop)
            {
                usleep(10000);
            }
            
            isp_stop = true;
            thread_isp.join();
            set_read_block_mode(true);
            set_terminal_mode(true);
        }
        #endif
    }
    
    return 0;
}
