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
#include <vector>
#include <fstream>
#include <sys/stat.h>
#include "utils.h"
#include "vi_vo.h"
#include "self_learning.h"

using std::cerr;
using std::cout;
using std::endl;

std::atomic<bool> isp_stop(false);

bool isFolderExist(string folderPath) {
    struct stat info;
    if (stat(folderPath.c_str(), &info) != 0) {
        return false;
    }
    return (info.st_mode & S_IFDIR);
}

void print_usage(const char *name)
{
    cout << "Usage: " << name << " <kmodel> <input_mode> <debug_mode> <topK> <mode>" << endl
         << "For example: " << endl
         << " [for isp] ./self_learning.elf recognition.kmodel None 0 3 work" << endl
         << "Options:" << endl
         << " 1> kmodel          kmodel文件路径 \n"
         << " 2> input_mode      摄像头(None) \n"
         << " 3> debug_mode      是否需要调试，0、1、2分别表示不调试、简单调试、详细调试\n"
         << " 4> topK            返回topK结果\n"
         << " 5> mode            操作模式，[work] or [label] \n" 
         << "\n"
         << endl;
}


void video_proc(char *argv[])
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

    bool sure_flag = true;
    bool cate_flag = true;

    SL sl(argv[1],{SENSOR_CHANNEL, SENSOR_HEIGHT, SENSOR_WIDTH}, reinterpret_cast<uintptr_t>(vaddr), reinterpret_cast<uintptr_t>(paddr), atoi(argv[3]));

    while (!isp_stop)
    {

        {
            ScopedTiming st("read capture", atoi(argv[3]));
            // VICAP_CHN_ID_1 out rgb888p
            memset(&dump_info, 0 , sizeof(k_video_frame_info));
            ret = kd_mpi_vicap_dump_frame(vicap_dev, VICAP_CHN_ID_1, VICAP_DUMP_YUV, &dump_info, 1000);
            if (ret) {
                printf("sample_vicap...kd_mpi_vicap_dump_frame failed.\n");
                continue;
            }
        }
            

        {
            ScopedTiming st("isp copy", atoi(argv[3]));
            // 从vivcap中读取一帧图像到dump_info
            auto vbvaddr = kd_mpi_sys_mmap_cached(dump_info.v_frame.phys_addr[0], size);
            memcpy(vaddr, (void *)vbvaddr, SENSOR_HEIGHT * SENSOR_WIDTH * 3);  // 这里以后可以去掉，不用copy
            kd_mpi_sys_munmap(vbvaddr, size);
        }

        cv::Mat osd_frame(osd_height, osd_width, CV_8UC4, cv::Scalar(0, 0, 0, 0));

        sl.pre_process();
        sl.inference();

        vector<Evec> results;
        results.clear();
        int topK = atoi( argv[4] );
        std::string mode = argv[5];
        
        if(strcmp(mode.c_str(),"work")==0)
        {
            sl.post_process(results,topK);

            std::string text;
            cv::Point origin;

            int width = 100;
            int i = 0;

            for (auto r : results)
            {
                ScopedTiming st("add texts", atoi(argv[3]));
                text = r.category + " : " + std::to_string(round( r.score * 100) / 100).substr(0,4);

                int x1 = 1.0 / 2 * osd_width - 200 ;
                int y1 = width;

                origin.x = x1;
                origin.y = y1 + width * i;
                i++;

                cv::putText(osd_frame, text, origin, cv::FONT_HERSHEY_COMPLEX, 2, cv::Scalar(255,255, 0, 0), 4, 8, 0);
                
            }
        }
        else if( strcmp(mode.c_str(),"label")==0 )
        { 
            
            std:string sure ;
            float* output;
            while(sure_flag)
            {
                std::cout << "Are you sure? \n" << "If sure, please s[sure] " << std::endl;
                getline(std::cin, sure);

                if( (strcmp(sure.c_str(),"s")==0) || (strcmp(sure.c_str(),"sure")==0))
                {
                    sure_flag = false;
                    cate_flag = true;
                }
                output = sl.get_vecOutput();
                
            }
            
            std::string category ;

            while( cate_flag )  
            {
                std::cout << "Its label : " << std::endl;
                getline(std::cin, category);
                
                std::cout << " category = " << category << std::endl;
                sure_flag = true;
                cate_flag = false;
                
                std::string cate_dir = "vectors/" + category;
                if (!isFolderExist(cate_dir)) 
                {
                    std::cout <<  "Please create " << "'" << cate_dir << "'"  << " folder on the 'small core window'" << std::endl;
                    continue;
                } 
                

                int length = sl.get_len();
                vector<float> vec(output, output + length);
                

                ifstream f;
                f.open(  cate_dir + "/index.txt" ,std::ios::in);
                std::string index;
                f>>index;
                std::cout<< "index = " << index <<endl; //显示读取内容 
                f.close();

                Utils::dump_binary_file( (cate_dir + "/" + index + ".bin").c_str(), (char *)output, length * sizeof(float));

                int num = atoi( index.c_str() );
                index = std::to_string((num+1));

                std::ofstream fo;
                fo.open((cate_dir +  "/index.txt" ),std::ios::out);
                fo<<index;
                fo.close();

            }

        }
        else
        {
            std::cout << " Please input  the correct mode, [work] or [label]" << std::endl;

        }


        {
            ScopedTiming st("osd copy", atoi(argv[3]));
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
    if (argc != 6)
    {
        print_usage(argv[0]);
        return -1;
    }

    if (strcmp(argv[2], "None") == 0)
    {
        std::thread thread_isp(video_proc, argv);

        if( strcmp(argv[5], "work") == 0 )
        {
            while (getchar() != 'q')
            {
                usleep(10000);
            }

        }
        else if( strcmp(argv[5], "label") == 0 )
        {

            while (1)
            {
                usleep(1000000);
            }
            
        }
        else
        {
            std::cout << " Please input the correct mode, 'work' or 'label' !!" << std::endl;
        }

        isp_stop = true;
        thread_isp.join();
    }
    
    return 0;
}
