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
//
#include <iostream>
#include <thread>
#include "vi_vo.h"
#include "utils.h"
#include "ai_base.h"

#include "crop.h"
#include "src.h"
#include "cv2_utils.h"

using std::cerr;
using std::cout;
using std::endl;

using namespace std;
using namespace cv;

std::atomic<bool> isp_stop(false);

#define USE_CACHE 0

template <class T>
std::vector<T> read_binary_file(const char *file_name)
{
    std::ifstream ifs(file_name, std::ios::binary);
    ifs.seekg(0, ifs.end);
    size_t len = ifs.tellg();
    std::vector<T> vec(len / sizeof(T), 0);
    ifs.seekg(0, ifs.beg);
    ifs.read(reinterpret_cast<char *>(vec.data()), len);
    ifs.close();
    return vec;
}


void print_usage(const char *name)
{
    cout << "Usage: " << name << "<crop_kmodel> <src_kmodel> <head_kmodel> <crop_net_len> <src_net_len> <head_thresh>  <debug_mode> " << endl
         << "For example: " << endl
         << " [for isp]  ./nanotracker.elf cropped_test127.kmodel nanotrack_backbone_sim.kmodel nanotracker_head_calib_k230.kmodel 127 255 0.1 0" << endl
         << "Options:" << endl
         << " 1> crop_kmodel    模板template kmodel文件路径 \n"
         << " 2> src_kmodel  跟踪目标 kmodel文件路径 \n"
         << " 3> head_kmodel  检测头 kmodel文件路径 \n"
         << " 4> crop_net_len     模板template 模型输入尺寸  \n"
         << " 5> src_net_len    跟踪目标 kmodel 模型输入尺寸   \n"
         << " 6> head_thresh    检测头 检测阈值  \n"
         << " 7> debug_mode      是否需要调试，0、1、2分别表示不调试、简单调试、详细调试\n"
         << "\n"
         << endl;
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

    int crop_net_len = atoi(argv[4]);
    int src_net_len = atoi(argv[5]);

    float thresh = atof(argv[6]);

    init();
    
    Crop crop(argv[1], crop_net_len , atoi(argv[7]));
    Crop crop_video(argv[1], crop_net_len ,{SENSOR_CHANNEL, SENSOR_HEIGHT, SENSOR_WIDTH}, reinterpret_cast<uintptr_t>(vaddr), reinterpret_cast<uintptr_t>(paddr), atoi(argv[7]));
    
    Src src(argv[2],src_net_len , atoi(argv[7]));
    Src src_video(argv[2],src_net_len ,{SENSOR_CHANNEL, SENSOR_HEIGHT, SENSOR_WIDTH}, reinterpret_cast<uintptr_t>(vaddr), reinterpret_cast<uintptr_t>(paddr), atoi(argv[7]));

    interpreter interp;

    std::ifstream ifs(argv[3], std::ios::binary);
    interp.load_model(ifs).expect("Invalid kmodel");


    crop_video.pre_process();
    cv::Mat ori_img = cv::imread("ori_img.jpg",cv::IMREAD_UNCHANGED);

    float* pos = get_position(ori_img);
    float* cen = get_center(ori_img);

    bool enter_init = true;

    std::vector<cv::Mat> src_img_vec;
    cv::Mat src_img;
    cv::Mat src_input;

    while (!isp_stop)
    {
        ScopedTiming st("total time", 1);

        cv::Mat osd_frame(osd_height, osd_width, CV_8UC4, cv::Scalar(0, 0, 0, 0));

        float w_z, h_z, s_z;

        {
            ScopedTiming st("get_updated_position : ", atoi(argv[7]));

            pos = get_updated_position();
            w_z = pos[0] + CONTEXT_AMOUNT * (pos[0] + pos[1]);
            h_z = pos[1] + CONTEXT_AMOUNT * (pos[0] + pos[1]);
            s_z = round(sqrt(w_z * h_z));
        }
        
        {
            ScopedTiming st("src_video.pre_process_video : ", atoi(argv[7]));
            src_img_vec.clear();
            src_video.pre_process_video( src_img );
        }
        

        {
            ScopedTiming st("src sub_window : ", atoi(argv[7]));
            src_input = sub_window(src_img, INSTANCE_SIZE, round(s_z * INSTANCE_SIZE / EXEMPLAR_SIZE));
        }
        

        float *crop_output;
        float *src_output;

        if (enter_init)
        {

            int seconds = 8;  //倒计时时长，单位秒
            time_t endtime = time(NULL) + seconds;  //倒计时结束时间

            while (time(NULL) <= endtime) {
                
                {
                    ScopedTiming st("read capture", atoi(argv[7]));
                    // VICAP_CHN_ID_1 out rgb888p
                    memset(&dump_info, 0 , sizeof(k_video_frame_info));
                    ret = kd_mpi_vicap_dump_frame(vicap_dev, VICAP_CHN_ID_1, VICAP_DUMP_YUV, &dump_info, 1000);
                    if (ret) {
                        printf("sample_vicap...kd_mpi_vicap_dump_frame failed.\n");
                        continue;
                    }
                }
                

                {
                    ScopedTiming st("isp copy", atoi(argv[7]));
                    // 从vivcap中读取一帧图像到dump_info
                    auto vbvaddr = kd_mpi_sys_mmap_cached(dump_info.v_frame.phys_addr[0], size);
                    memcpy(vaddr, (void *)vbvaddr, SENSOR_HEIGHT * SENSOR_WIDTH * 3);  // 这里以后可以去掉，不用copy
                    kd_mpi_sys_munmap(vbvaddr, size);
                }

                cout << "倒计时：" << endtime - time(NULL) << "秒" << endl;


                // 初始化跟踪框.
                int init_x0 = MARGIN * osd_width;
                int init_x1 = (1 - MARGIN) * osd_width;
                int init_y0 = osd_height / 2.0 - ((1 - 2 * MARGIN) * osd_width) / 2.0;
                int init_y1 = osd_height / 2.0 + ((1 - 2 * MARGIN) * osd_width) / 2.0;

                int init_w = init_x1 - init_x0;
                int init_h = init_y1 - init_y0;

                cv::rectangle(osd_frame, cv::Rect( init_x0,init_y0,init_w,init_h ), cv::Scalar(255, 0,255, 0), 8, 2, 0); // ARGB

                if( (endtime - time(NULL)==2) || (endtime - time(NULL)==3) || (endtime - time(NULL)==4) )
                {
                    std::cout << " >>>>>> get trackWindow <<<<<<<<" << std::endl;

                    crop_video.pre_process();

                    cv::Mat crop_img = cv::imread( "crop_template.jpg",cv::IMREAD_UNCHANGED );
                    cv::Mat crop_input = sub_window(crop_img, EXEMPLAR_SIZE, round(s_z));
                    cv::imwrite("crop_window.jpg",crop_input);

                    crop.pre_process( crop_input );
                    crop.inference();
                    crop.post_process();
                    crop_output = crop.output;

                }

                std::this_thread::sleep_for(std::chrono::seconds(1));
                
                {
                    ScopedTiming st("osd copy", atoi(argv[7]));
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

            cout << ">>>>>>>  Play  <<<<<<<" << endl;
            enter_init = false;

        }
        else
        {
            {
                ScopedTiming st("read capture", atoi(argv[7]));
                // VICAP_CHN_ID_1 out rgb888p
                memset(&dump_info, 0 , sizeof(k_video_frame_info));
                ret = kd_mpi_vicap_dump_frame(vicap_dev, VICAP_CHN_ID_1, VICAP_DUMP_YUV, &dump_info, 1000);
                if (ret) {
                    printf("sample_vicap...kd_mpi_vicap_dump_frame failed.\n");
                    continue;
                }
            }
                

            {
                ScopedTiming st("isp copy", atoi(argv[7]));
                // 从vivcap中读取一帧图像到dump_info
                auto vbvaddr = kd_mpi_sys_mmap_cached(dump_info.v_frame.phys_addr[0], size);
                memcpy(vaddr, (void *)vbvaddr, SENSOR_HEIGHT * SENSOR_WIDTH * 3);  // 这里以后可以去掉，不用copy
                kd_mpi_sys_munmap(vbvaddr, size);
            }

            {
                ScopedTiming st("src pipeline: ", atoi(argv[7]));
                src.pre_process(src_input);
                src.inference();
                src.post_process();
            }
            

            src_output = src.output;

            std::vector<float*> inputs;
            std::vector<float*> outputs;

            {
                ScopedTiming st("head pipeline: ", atoi(argv[7]));
                inputs.clear();
                inputs.push_back(crop_output);
                inputs.push_back(src_output);

                std::vector<int> input_shapes = { 1*48*8*8,1*48*16*16 };
                for ( int j = 0; j < 2; j++)
                {
                    auto desc = interp.input_desc(j);
                    auto shape = interp.input_shape(j);
                    auto tensor = host_runtime_tensor::create(desc.datatype, shape, hrt::pool_shared).expect("cannot create input tensor");
                    auto mapped_buf = std::move(hrt::map(tensor, map_access_::map_write).unwrap());
                    
                    memcpy(reinterpret_cast<void *>(mapped_buf.buffer().data()), reinterpret_cast<void *>( inputs[j]), input_shapes[j]*4);
                    auto ret = mapped_buf.unmap();
                    ret = hrt::sync(tensor, sync_op_t::sync_write_back, true);
                    if (!ret.is_ok())
                    {
                        std::cerr << "hrt::sync failed" << std::endl;
                        std::abort();
                    }

                    interp.input_tensor(j, tensor).expect("cannot set input tensor");
                }

                for (size_t i = 0; i < interp.outputs_size(); i++)
                {
                    auto desc = interp.output_desc(i);
                    auto shape = interp.output_shape(i);
                    auto tensor = host_runtime_tensor::create(desc.datatype, shape, hrt::pool_shared).expect("cannot create output tensor");
                    interp.output_tensor(i, tensor).expect("cannot set output tensor");
                }

                auto start = std::chrono::steady_clock::now();
                interp.run().expect("error occurred in running model");
                auto stop = std::chrono::steady_clock::now();
                double duration = std::chrono::duration<double, std::milli>(stop - start).count();

                outputs.clear();
                for ( int j = 0; j<2;  j++)
                {
                    auto out = interp.output_tensor(j).expect("cannot get output tensor");
                    auto mapped_buf = std::move(hrt::map(out, map_access_::map_read).unwrap());

                    outputs.push_back( (float *)mapped_buf.buffer().data() );
                    
                }
            }
            

            float* score, * box;
            score = outputs[0];
            box = outputs[1];

            int box_x, box_y, box_h, box_w;
            float best_score;
            {
                ScopedTiming st("head post_process : ", atoi(argv[7]));
                post_process(score, box, SENSOR_WIDTH, SENSOR_HEIGHT, box_x, box_y, box_w, box_h, best_score);
            }
            

            {
                ScopedTiming st("draw box", atoi(argv[7]));
                if (best_score > thresh) 
                {

                    int r_x1 = box_x;
                    int r_y1 = box_y;
                    int r_x2 = (box_x+box_w);
                    int r_y2 = (box_y+box_h);

                    int x1 =   r_x1*1.0 / SENSOR_WIDTH * osd_width;
                    int y1 =    r_y1*1.0 / SENSOR_HEIGHT  * osd_height;

                    int w = (r_x2-r_x1)*1.0 / SENSOR_WIDTH * osd_width;
                    int h = (r_y2-r_y1)*1.0 / SENSOR_HEIGHT  * osd_height;

                    cv::rectangle(osd_frame, cv::Rect( x1,y1,w,h ), cv::Scalar(255, 255,0, 0), 8, 2, 0); // ARGB

                }
            }


            {
                ScopedTiming st("osd copy", atoi(argv[7]));
                
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

    int crop_net_len = atoi(argv[4]);
    int src_net_len = atoi(argv[5]);

    float thresh = atof(argv[6]);

    init();
    
    Crop crop(argv[1], crop_net_len , atoi(argv[7]));
    Crop crop_video(argv[1], crop_net_len ,{SENSOR_CHANNEL, SENSOR_HEIGHT, SENSOR_WIDTH}, reinterpret_cast<uintptr_t>(vaddr), reinterpret_cast<uintptr_t>(paddr), atoi(argv[7]));
    
    Src src(argv[2],src_net_len , atoi(argv[7]));
    Src src_video(argv[2],src_net_len ,{SENSOR_CHANNEL, SENSOR_HEIGHT, SENSOR_WIDTH}, reinterpret_cast<uintptr_t>(vaddr), reinterpret_cast<uintptr_t>(paddr), atoi(argv[7]));

    interpreter interp;

    std::ifstream ifs(argv[3], std::ios::binary);
    interp.load_model(ifs).expect("Invalid kmodel");


    crop_video.pre_process();
    cv::Mat ori_img = cv::imread("ori_img.jpg",cv::IMREAD_UNCHANGED);

    float* pos = get_position(ori_img);
    float* cen = get_center(ori_img);

    bool enter_init = true;

    std::vector<cv::Mat> src_img_vec;
    cv::Mat src_img;
    cv::Mat src_input;

    while (!isp_stop)
    {
        ScopedTiming st("total time", 1);

        cv::Mat osd_frame(osd_height, osd_width, CV_8UC4, cv::Scalar(0, 0, 0, 0));

        float w_z, h_z, s_z;

        {
            ScopedTiming st("get_updated_position : ", atoi(argv[7]));

            pos = get_updated_position();
            w_z = pos[0] + CONTEXT_AMOUNT * (pos[0] + pos[1]);
            h_z = pos[1] + CONTEXT_AMOUNT * (pos[0] + pos[1]);
            s_z = round(sqrt(w_z * h_z));
        }
        
        {
            ScopedTiming st("src_video.pre_process_video : ", atoi(argv[7]));
            src_img_vec.clear();
            src_video.pre_process_video( src_img );
        }
        

        {
            ScopedTiming st("src sub_window : ", atoi(argv[7]));
            src_input = sub_window(src_img, INSTANCE_SIZE, round(s_z * INSTANCE_SIZE / EXEMPLAR_SIZE));
        }
        

        float *crop_output;
        float *src_output;

        if (enter_init)
        {

            int seconds = 8;  //倒计时时长，单位秒
            time_t endtime = time(NULL) + seconds;  //倒计时结束时间

            while (time(NULL) <= endtime) {
                
                {
                    ScopedTiming st("read capture", atoi(argv[7]));
                    // VICAP_CHN_ID_1 out rgb888p
                    memset(&dump_info, 0 , sizeof(k_video_frame_info));
                    ret = kd_mpi_vicap_dump_frame(vicap_dev, VICAP_CHN_ID_1, VICAP_DUMP_YUV, &dump_info, 1000);
                    if (ret) {
                        printf("sample_vicap...kd_mpi_vicap_dump_frame failed.\n");
                        continue;
                    }
                }
                

                {
                    ScopedTiming st("isp copy", atoi(argv[7]));
                    // 从vivcap中读取一帧图像到dump_info
                    auto vbvaddr = kd_mpi_sys_mmap_cached(dump_info.v_frame.phys_addr[0], size);
                    memcpy(vaddr, (void *)vbvaddr, SENSOR_HEIGHT * SENSOR_WIDTH * 3);  // 这里以后可以去掉，不用copy
                    kd_mpi_sys_munmap(vbvaddr, size);
                }

                cout << "倒计时：" << endtime - time(NULL) << "秒" << endl;


                // 初始化跟踪框.
                int init_x0 = MARGIN * osd_width;
                int init_x1 = (1 - MARGIN) * osd_width;
                int init_y0 = osd_height / 2.0 - ((1 - 2 * MARGIN) * osd_width) / 2.0;
                int init_y1 = osd_height / 2.0 + ((1 - 2 * MARGIN) * osd_width) / 2.0;

                int init_w = init_x1 - init_x0;
                int init_h = init_y1 - init_y0;

                cv::rectangle(osd_frame, cv::Rect( init_x0,init_y0,init_w,init_h ), cv::Scalar(255, 0,255, 0), 8, 2, 0); // ARGB

                if( (endtime - time(NULL)==2) || (endtime - time(NULL)==3) || (endtime - time(NULL)==4) )
                {
                    std::cout << " >>>>>> get trackWindow <<<<<<<<" << std::endl;

                    crop_video.pre_process();

                    cv::Mat crop_img = cv::imread( "crop_template.jpg",cv::IMREAD_UNCHANGED );
                    cv::Mat crop_input = sub_window(crop_img, EXEMPLAR_SIZE, round(s_z));
                    cv::imwrite("crop_window.jpg",crop_input);

                    crop.pre_process( crop_input );
                    crop.inference();
                    crop.post_process();
                    crop_output = crop.output;

                }

                std::this_thread::sleep_for(std::chrono::seconds(1));
                
                {
                    ScopedTiming st("osd copy", atoi(argv[7]));
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

            cout << ">>>>>>>  Play  <<<<<<<" << endl;
            enter_init = false;

        }
        else
        {
            {
                ScopedTiming st("read capture", atoi(argv[7]));
                // VICAP_CHN_ID_1 out rgb888p
                memset(&dump_info, 0 , sizeof(k_video_frame_info));
                ret = kd_mpi_vicap_dump_frame(vicap_dev, VICAP_CHN_ID_1, VICAP_DUMP_YUV, &dump_info, 1000);
                if (ret) {
                    printf("sample_vicap...kd_mpi_vicap_dump_frame failed.\n");
                    continue;
                }
            }
                

            {
                ScopedTiming st("isp copy", atoi(argv[7]));
                // 从vivcap中读取一帧图像到dump_info
                auto vbvaddr = kd_mpi_sys_mmap_cached(dump_info.v_frame.phys_addr[0], size);
                memcpy(vaddr, (void *)vbvaddr, SENSOR_HEIGHT * SENSOR_WIDTH * 3);  // 这里以后可以去掉，不用copy
                kd_mpi_sys_munmap(vbvaddr, size);
            }

            {
                ScopedTiming st("src pipeline: ", atoi(argv[7]));
                src.pre_process(src_input);
                src.inference();
                src.post_process();
            }
            

            src_output = src.output;

            std::vector<float*> inputs;
            std::vector<float*> outputs;

            {
                ScopedTiming st("head pipeline: ", atoi(argv[7]));
                inputs.clear();
                inputs.push_back(crop_output);
                inputs.push_back(src_output);

                std::vector<int> input_shapes = { 1*48*8*8,1*48*16*16 };
                for ( int j = 0; j < 2; j++)
                {
                    auto desc = interp.input_desc(j);
                    auto shape = interp.input_shape(j);
                    auto tensor = host_runtime_tensor::create(desc.datatype, shape, hrt::pool_shared).expect("cannot create input tensor");
                    auto mapped_buf = std::move(hrt::map(tensor, map_access_::map_write).unwrap());
                    
                    memcpy(reinterpret_cast<void *>(mapped_buf.buffer().data()), reinterpret_cast<void *>( inputs[j]), input_shapes[j]*4);
                    auto ret = mapped_buf.unmap();
                    ret = hrt::sync(tensor, sync_op_t::sync_write_back, true);
                    if (!ret.is_ok())
                    {
                        std::cerr << "hrt::sync failed" << std::endl;
                        std::abort();
                    }

                    interp.input_tensor(j, tensor).expect("cannot set input tensor");
                }

                for (size_t i = 0; i < interp.outputs_size(); i++)
                {
                    auto desc = interp.output_desc(i);
                    auto shape = interp.output_shape(i);
                    auto tensor = host_runtime_tensor::create(desc.datatype, shape, hrt::pool_shared).expect("cannot create output tensor");
                    interp.output_tensor(i, tensor).expect("cannot set output tensor");
                }

                auto start = std::chrono::steady_clock::now();
                interp.run().expect("error occurred in running model");
                auto stop = std::chrono::steady_clock::now();
                double duration = std::chrono::duration<double, std::milli>(stop - start).count();

                outputs.clear();
                for ( int j = 0; j<2;  j++)
                {
                    auto out = interp.output_tensor(j).expect("cannot get output tensor");
                    auto mapped_buf = std::move(hrt::map(out, map_access_::map_read).unwrap());

                    outputs.push_back( (float *)mapped_buf.buffer().data() );
                    
                }
            }
            

            float* score, * box;
            score = outputs[0];
            box = outputs[1];

            int box_x, box_y, box_h, box_w;
            float best_score;
            {
                ScopedTiming st("head post_process : ", atoi(argv[7]));
                post_process(score, box, SENSOR_WIDTH, SENSOR_HEIGHT, box_x, box_y, box_w, box_h, best_score);
            }
            

            {
                ScopedTiming st("draw box", atoi(argv[7]));
                if (best_score > thresh) 
                {

                    int r_x1 = box_x;
                    int r_y1 = box_y;
                    int r_x2 = (box_x+box_w);
                    int r_y2 = (box_y+box_h);

                    int x1 =   r_x1*1.0 / SENSOR_WIDTH * osd_width;
                    int y1 =    r_y1*1.0 / SENSOR_HEIGHT  * osd_height;

                    int w = (r_x2-r_x1)*1.0 / SENSOR_WIDTH * osd_width;
                    int h = (r_y2-r_y1)*1.0 / SENSOR_HEIGHT  * osd_height;

                    cv::rectangle(osd_frame, cv::Rect( x1,y1,w,h ), cv::Scalar(255, 255,0, 0), 8, 2, 0); // ARGB

                }
            }


            {
                ScopedTiming st("osd copy", atoi(argv[7]));
                
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

    int crop_net_len = atoi(argv[4]);
    int src_net_len = atoi(argv[5]);

    float thresh = atof(argv[6]);

    init();
    
    Crop crop(argv[1], crop_net_len , atoi(argv[7]));
    Crop crop_video(argv[1], crop_net_len ,{SENSOR_CHANNEL, SENSOR_HEIGHT, SENSOR_WIDTH}, reinterpret_cast<uintptr_t>(vaddr), reinterpret_cast<uintptr_t>(paddr), atoi(argv[7]));
    
    Src src(argv[2],src_net_len , atoi(argv[7]));
    Src src_video(argv[2],src_net_len ,{SENSOR_CHANNEL, SENSOR_HEIGHT, SENSOR_WIDTH}, reinterpret_cast<uintptr_t>(vaddr), reinterpret_cast<uintptr_t>(paddr), atoi(argv[7]));

    interpreter interp;

    std::ifstream ifs(argv[3], std::ios::binary);
    interp.load_model(ifs).expect("Invalid kmodel");


    crop_video.pre_process();
    cv::Mat ori_img = cv::imread("ori_img.jpg",cv::IMREAD_UNCHANGED);

    float* pos = get_position(ori_img);
    float* cen = get_center(ori_img);

    bool enter_init = true;

    std::vector<cv::Mat> src_img_vec;
    cv::Mat src_img;
    cv::Mat src_input;

    while (!isp_stop)
    {
        ScopedTiming st("total time", 1);

        cv::Mat osd_frame(osd_height, osd_width, CV_8UC4, cv::Scalar(0, 0, 0, 0));
        cv::rotate(osd_frame, osd_frame, cv::ROTATE_90_COUNTERCLOCKWISE);
        bool osd_rotate = false;

        float w_z, h_z, s_z;

        {
            ScopedTiming st("get_updated_position : ", atoi(argv[7]));

            pos = get_updated_position();
            w_z = pos[0] + CONTEXT_AMOUNT * (pos[0] + pos[1]);
            h_z = pos[1] + CONTEXT_AMOUNT * (pos[0] + pos[1]);
            s_z = round(sqrt(w_z * h_z));
        }
        
        {
            ScopedTiming st("src_video.pre_process_video : ", atoi(argv[7]));
            src_img_vec.clear();
            src_video.pre_process_video( src_img );
        }
        

        {
            ScopedTiming st("src sub_window : ", atoi(argv[7]));
            src_input = sub_window(src_img, INSTANCE_SIZE, round(s_z * INSTANCE_SIZE / EXEMPLAR_SIZE));
        }
        

        float *crop_output;
        float *src_output;

        if (enter_init)
        {

            int seconds = 8;  //倒计时时长，单位秒
            time_t endtime = time(NULL) + seconds;  //倒计时结束时间

            while (time(NULL) <= endtime) {
                
                {
                    ScopedTiming st("read capture", atoi(argv[7]));
                    // VICAP_CHN_ID_1 out rgb888p
                    memset(&dump_info, 0 , sizeof(k_video_frame_info));
                    ret = kd_mpi_vicap_dump_frame(vicap_dev, VICAP_CHN_ID_1, VICAP_DUMP_YUV, &dump_info, 1000);
                    if (ret) {
                        printf("sample_vicap...kd_mpi_vicap_dump_frame failed.\n");
                        continue;
                    }
                }
                

                {
                    ScopedTiming st("isp copy", atoi(argv[7]));
                    // 从vivcap中读取一帧图像到dump_info
                    auto vbvaddr = kd_mpi_sys_mmap_cached(dump_info.v_frame.phys_addr[0], size);
                    memcpy(vaddr, (void *)vbvaddr, SENSOR_HEIGHT * SENSOR_WIDTH * 3);  // 这里以后可以去掉，不用copy
                    kd_mpi_sys_munmap(vbvaddr, size);
                }

                cout << "倒计时：" << endtime - time(NULL) << "秒" << endl;

                if(osd_rotate)
                {
                    cv::rotate(osd_frame, osd_frame, cv::ROTATE_90_COUNTERCLOCKWISE);
                    osd_rotate = false;
                }


                // 初始化跟踪框.
                int init_x0 = MARGIN * osd_frame.cols;
                int init_x1 = (1 - MARGIN) * osd_frame.cols;
                int init_y0 = osd_frame.rows / 2.0 - ((1 - 2 * MARGIN) * osd_frame.cols) / 2.0;
                int init_y1 = osd_frame.rows / 2.0 + ((1 - 2 * MARGIN) * osd_frame.cols) / 2.0;

                int init_w = init_x1 - init_x0;
                int init_h = init_y1 - init_y0;

                cv::rectangle(osd_frame, cv::Rect( init_x0,init_y0,init_w,init_h ), cv::Scalar(255, 0,255, 0), 8, 2, 0); // ARGB

                if( (endtime - time(NULL)==2) || (endtime - time(NULL)==3) || (endtime - time(NULL)==4) )
                {
                    std::cout << " >>>>>> get trackWindow <<<<<<<<" << std::endl;

                    crop_video.pre_process();

                    cv::Mat crop_img = cv::imread( "crop_template.jpg",cv::IMREAD_UNCHANGED );
                    cv::Mat crop_input = sub_window(crop_img, EXEMPLAR_SIZE, round(s_z));
                    cv::imwrite("crop_window.jpg",crop_input);

                    crop.pre_process( crop_input );
                    crop.inference();
                    crop.post_process();
                    crop_output = crop.output;

                }

                std::this_thread::sleep_for(std::chrono::seconds(1));

                if(! osd_rotate)
                {
                    cv::rotate(osd_frame, osd_frame, cv::ROTATE_90_CLOCKWISE);
                    osd_rotate = true;
                }
                
                {
                    ScopedTiming st("osd copy", atoi(argv[7]));
                    memcpy(pic_vaddr, osd_frame.data, osd_frame.cols * osd_frame.rows * 4);
                    //显示通道插入帧
                    kd_mpi_vo_chn_insert_frame(osd_id+3, &vf_info);  //K_VO_OSD0
                    // printf("kd_mpi_vo_chn_insert_frame success \n");

                    ret = kd_mpi_vicap_dump_release(vicap_dev, VICAP_CHN_ID_1, &dump_info);
                    if (ret) {
                        printf("sample_vicap...kd_mpi_vicap_dump_release failed.\n");
                    }
                }

            }

            cout << ">>>>>>>  Play  <<<<<<<" << endl;
            enter_init = false;

        }
        else
        {
            {
                ScopedTiming st("read capture", atoi(argv[7]));
                // VICAP_CHN_ID_1 out rgb888p
                memset(&dump_info, 0 , sizeof(k_video_frame_info));
                ret = kd_mpi_vicap_dump_frame(vicap_dev, VICAP_CHN_ID_1, VICAP_DUMP_YUV, &dump_info, 1000);
                if (ret) {
                    printf("sample_vicap...kd_mpi_vicap_dump_frame failed.\n");
                    continue;
                }
            }
                

            {
                ScopedTiming st("isp copy", atoi(argv[7]));
                // 从vivcap中读取一帧图像到dump_info
                auto vbvaddr = kd_mpi_sys_mmap_cached(dump_info.v_frame.phys_addr[0], size);
                memcpy(vaddr, (void *)vbvaddr, SENSOR_HEIGHT * SENSOR_WIDTH * 3);  // 这里以后可以去掉，不用copy
                kd_mpi_sys_munmap(vbvaddr, size);
            }

            {
                ScopedTiming st("src pipeline: ", atoi(argv[7]));
                src.pre_process(src_input);
                src.inference();
                src.post_process();
            }
            

            src_output = src.output;

            std::vector<float*> inputs;
            std::vector<float*> outputs;

            {
                ScopedTiming st("head pipeline: ", atoi(argv[7]));
                inputs.clear();
                inputs.push_back(crop_output);
                inputs.push_back(src_output);

                std::vector<int> input_shapes = { 1*48*8*8,1*48*16*16 };
                for ( int j = 0; j < 2; j++)
                {
                    auto desc = interp.input_desc(j);
                    auto shape = interp.input_shape(j);
                    auto tensor = host_runtime_tensor::create(desc.datatype, shape, hrt::pool_shared).expect("cannot create input tensor");
                    auto mapped_buf = std::move(hrt::map(tensor, map_access_::map_write).unwrap());
                    
                    memcpy(reinterpret_cast<void *>(mapped_buf.buffer().data()), reinterpret_cast<void *>( inputs[j]), input_shapes[j]*4);
                    auto ret = mapped_buf.unmap();
                    ret = hrt::sync(tensor, sync_op_t::sync_write_back, true);
                    if (!ret.is_ok())
                    {
                        std::cerr << "hrt::sync failed" << std::endl;
                        std::abort();
                    }

                    interp.input_tensor(j, tensor).expect("cannot set input tensor");
                }

                for (size_t i = 0; i < interp.outputs_size(); i++)
                {
                    auto desc = interp.output_desc(i);
                    auto shape = interp.output_shape(i);
                    auto tensor = host_runtime_tensor::create(desc.datatype, shape, hrt::pool_shared).expect("cannot create output tensor");
                    interp.output_tensor(i, tensor).expect("cannot set output tensor");
                }

                auto start = std::chrono::steady_clock::now();
                interp.run().expect("error occurred in running model");
                auto stop = std::chrono::steady_clock::now();
                double duration = std::chrono::duration<double, std::milli>(stop - start).count();

                outputs.clear();
                for ( int j = 0; j<2;  j++)
                {
                    auto out = interp.output_tensor(j).expect("cannot get output tensor");
                    auto mapped_buf = std::move(hrt::map(out, map_access_::map_read).unwrap());

                    outputs.push_back( (float *)mapped_buf.buffer().data() );
                    
                }
            }
            

            float* score, * box;
            score = outputs[0];
            box = outputs[1];

            int box_x, box_y, box_h, box_w;
            float best_score;
            {
                ScopedTiming st("head post_process : ", atoi(argv[7]));
                post_process(score, box, SENSOR_WIDTH, SENSOR_HEIGHT, box_x, box_y, box_w, box_h, best_score);
            }
            

            {
                ScopedTiming st("draw box", atoi(argv[7]));
                if (best_score > thresh) 
                {

                    int r_x1 = box_x;
                    int r_y1 = box_y;
                    int r_x2 = (box_x+box_w);
                    int r_y2 = (box_y+box_h);

                    int x1 =   r_x1*1.0 / SENSOR_WIDTH * osd_frame.cols;
                    int y1 =    r_y1*1.0 / SENSOR_HEIGHT  * osd_frame.rows;

                    int w = (r_x2-r_x1)*1.0 / SENSOR_WIDTH * osd_frame.cols;
                    int h = (r_y2-r_y1)*1.0 / SENSOR_HEIGHT  * osd_frame.rows;

                    cv::rectangle(osd_frame, cv::Rect( x1,y1,w,h ), cv::Scalar(255, 255,0, 0), 8, 2, 0); // ARGB

                }
            }
            cv::rotate(osd_frame, osd_frame, cv::ROTATE_90_CLOCKWISE);


            {
                ScopedTiming st("osd copy", atoi(argv[7]));
                
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

    int crop_net_len = atoi(argv[4]);
    int src_net_len = atoi(argv[5]);

    float thresh = atof(argv[6]);

    init();
    
    Crop crop(argv[1], crop_net_len , atoi(argv[7]));
    Crop crop_video(argv[1], crop_net_len ,{SENSOR_CHANNEL, SENSOR_HEIGHT, SENSOR_WIDTH}, reinterpret_cast<uintptr_t>(vaddr), reinterpret_cast<uintptr_t>(paddr), atoi(argv[7]));
    
    Src src(argv[2],src_net_len , atoi(argv[7]));
    Src src_video(argv[2],src_net_len ,{SENSOR_CHANNEL, SENSOR_HEIGHT, SENSOR_WIDTH}, reinterpret_cast<uintptr_t>(vaddr), reinterpret_cast<uintptr_t>(paddr), atoi(argv[7]));

    interpreter interp;

    std::ifstream ifs(argv[3], std::ios::binary);
    interp.load_model(ifs).expect("Invalid kmodel");


    crop_video.pre_process();
    cv::Mat ori_img = cv::imread("ori_img.jpg",cv::IMREAD_UNCHANGED);

    float* pos = get_position(ori_img);
    float* cen = get_center(ori_img);

    bool enter_init = true;

    std::vector<cv::Mat> src_img_vec;
    cv::Mat src_img;
    cv::Mat src_input;

    while (!isp_stop)
    {
        ScopedTiming st("total time", 1);

        cv::Mat osd_frame(osd_height, osd_width, CV_8UC4, cv::Scalar(0, 0, 0, 0));
        
        #if defined(STUDIO_HDMI)
        {
            float w_z, h_z, s_z;

            {
                ScopedTiming st("get_updated_position : ", atoi(argv[7]));

                pos = get_updated_position();
                w_z = pos[0] + CONTEXT_AMOUNT * (pos[0] + pos[1]);
                h_z = pos[1] + CONTEXT_AMOUNT * (pos[0] + pos[1]);
                s_z = round(sqrt(w_z * h_z));
            }
            
            {
                ScopedTiming st("src_video.pre_process_video : ", atoi(argv[7]));
                src_img_vec.clear();
                src_video.pre_process_video( src_img );
            }
            

            {
                ScopedTiming st("src sub_window : ", atoi(argv[7]));
                src_input = sub_window(src_img, INSTANCE_SIZE, round(s_z * INSTANCE_SIZE / EXEMPLAR_SIZE));
            }
            

            float *crop_output;
            float *src_output;

            if (enter_init)
            {

                int seconds = 8;  //倒计时时长，单位秒
                time_t endtime = time(NULL) + seconds;  //倒计时结束时间

                while (time(NULL) <= endtime) {
                    
                    {
                        ScopedTiming st("read capture", atoi(argv[7]));
                        // VICAP_CHN_ID_1 out rgb888p
                        memset(&dump_info, 0 , sizeof(k_video_frame_info));
                        ret = kd_mpi_vicap_dump_frame(vicap_dev, VICAP_CHN_ID_1, VICAP_DUMP_YUV, &dump_info, 1000);
                        if (ret) {
                            printf("sample_vicap...kd_mpi_vicap_dump_frame failed.\n");
                            continue;
                        }
                    }
                    

                    {
                        ScopedTiming st("isp copy", atoi(argv[7]));
                        // 从vivcap中读取一帧图像到dump_info
                        auto vbvaddr = kd_mpi_sys_mmap_cached(dump_info.v_frame.phys_addr[0], size);
                        memcpy(vaddr, (void *)vbvaddr, SENSOR_HEIGHT * SENSOR_WIDTH * 3);  // 这里以后可以去掉，不用copy
                        kd_mpi_sys_munmap(vbvaddr, size);
                    }

                    cout << "倒计时：" << endtime - time(NULL) << "秒" << endl;


                    // 初始化跟踪框.
                    int init_x0 = MARGIN * osd_frame.cols;
                    int init_x1 = (1 - MARGIN) * osd_frame.cols;
                    int init_y0 = osd_frame.rows / 2.0 - ((1 - 2 * MARGIN) * osd_frame.cols) / 2.0;
                    int init_y1 = osd_frame.rows / 2.0 + ((1 - 2 * MARGIN) * osd_frame.cols) / 2.0;

                    int init_w = init_x1 - init_x0;
                    int init_h = init_y1 - init_y0;

                    cv::rectangle(osd_frame, cv::Rect( init_x0,init_y0,init_w,init_h ), cv::Scalar(255, 0,255, 0), 8, 2, 0); // ARGB

                    if( (endtime - time(NULL)==2) || (endtime - time(NULL)==3) || (endtime - time(NULL)==4) )
                    {
                        std::cout << " >>>>>> get trackWindow <<<<<<<<" << std::endl;

                        crop_video.pre_process();

                        cv::Mat crop_img = cv::imread( "crop_template.jpg",cv::IMREAD_UNCHANGED );
                        cv::Mat crop_input = sub_window(crop_img, EXEMPLAR_SIZE, round(s_z));
                        cv::imwrite("crop_window.jpg",crop_input);

                        crop.pre_process( crop_input );
                        crop.inference();
                        crop.post_process();
                        crop_output = crop.output;

                    }

                    std::this_thread::sleep_for(std::chrono::seconds(1));
                    
                    {
                        ScopedTiming st("osd copy", atoi(argv[7]));
                        memcpy(pic_vaddr, osd_frame.data, osd_frame.cols * osd_frame.rows * 4);
                        //显示通道插入帧
                        kd_mpi_vo_chn_insert_frame(osd_id+3, &vf_info);  //K_VO_OSD0
                        // printf("kd_mpi_vo_chn_insert_frame success \n");

                        ret = kd_mpi_vicap_dump_release(vicap_dev, VICAP_CHN_ID_1, &dump_info);
                        if (ret) {
                            printf("sample_vicap...kd_mpi_vicap_dump_release failed.\n");
                        }
                    }

                }

                cout << ">>>>>>>  Play  <<<<<<<" << endl;
                enter_init = false;

            }
            else
            {
                {
                    ScopedTiming st("read capture", atoi(argv[7]));
                    // VICAP_CHN_ID_1 out rgb888p
                    memset(&dump_info, 0 , sizeof(k_video_frame_info));
                    ret = kd_mpi_vicap_dump_frame(vicap_dev, VICAP_CHN_ID_1, VICAP_DUMP_YUV, &dump_info, 1000);
                    if (ret) {
                        printf("sample_vicap...kd_mpi_vicap_dump_frame failed.\n");
                        continue;
                    }
                }
                    

                {
                    ScopedTiming st("isp copy", atoi(argv[7]));
                    // 从vivcap中读取一帧图像到dump_info
                    auto vbvaddr = kd_mpi_sys_mmap_cached(dump_info.v_frame.phys_addr[0], size);
                    memcpy(vaddr, (void *)vbvaddr, SENSOR_HEIGHT * SENSOR_WIDTH * 3);  // 这里以后可以去掉，不用copy
                    kd_mpi_sys_munmap(vbvaddr, size);
                }

                {
                    ScopedTiming st("src pipeline: ", atoi(argv[7]));
                    src.pre_process(src_input);
                    src.inference();
                    src.post_process();
                }
                

                src_output = src.output;

                std::vector<float*> inputs;
                std::vector<float*> outputs;

                {
                    ScopedTiming st("head pipeline: ", atoi(argv[7]));
                    inputs.clear();
                    inputs.push_back(crop_output);
                    inputs.push_back(src_output);

                    std::vector<int> input_shapes = { 1*48*8*8,1*48*16*16 };
                    for ( int j = 0; j < 2; j++)
                    {
                        auto desc = interp.input_desc(j);
                        auto shape = interp.input_shape(j);
                        auto tensor = host_runtime_tensor::create(desc.datatype, shape, hrt::pool_shared).expect("cannot create input tensor");
                        auto mapped_buf = std::move(hrt::map(tensor, map_access_::map_write).unwrap());
                        
                        memcpy(reinterpret_cast<void *>(mapped_buf.buffer().data()), reinterpret_cast<void *>( inputs[j]), input_shapes[j]*4);
                        auto ret = mapped_buf.unmap();
                        ret = hrt::sync(tensor, sync_op_t::sync_write_back, true);
                        if (!ret.is_ok())
                        {
                            std::cerr << "hrt::sync failed" << std::endl;
                            std::abort();
                        }

                        interp.input_tensor(j, tensor).expect("cannot set input tensor");
                    }

                    for (size_t i = 0; i < interp.outputs_size(); i++)
                    {
                        auto desc = interp.output_desc(i);
                        auto shape = interp.output_shape(i);
                        auto tensor = host_runtime_tensor::create(desc.datatype, shape, hrt::pool_shared).expect("cannot create output tensor");
                        interp.output_tensor(i, tensor).expect("cannot set output tensor");
                    }

                    auto start = std::chrono::steady_clock::now();
                    interp.run().expect("error occurred in running model");
                    auto stop = std::chrono::steady_clock::now();
                    double duration = std::chrono::duration<double, std::milli>(stop - start).count();

                    outputs.clear();
                    for ( int j = 0; j<2;  j++)
                    {
                        auto out = interp.output_tensor(j).expect("cannot get output tensor");
                        auto mapped_buf = std::move(hrt::map(out, map_access_::map_read).unwrap());

                        outputs.push_back( (float *)mapped_buf.buffer().data() );
                        
                    }
                }
                

                float* score, * box;
                score = outputs[0];
                box = outputs[1];

                int box_x, box_y, box_h, box_w;
                float best_score;
                {
                    ScopedTiming st("head post_process : ", atoi(argv[7]));
                    post_process(score, box, SENSOR_WIDTH, SENSOR_HEIGHT, box_x, box_y, box_w, box_h, best_score);
                }
                

                {
                    ScopedTiming st("draw box", atoi(argv[7]));
                    if (best_score > thresh) 
                    {

                        int r_x1 = box_x;
                        int r_y1 = box_y;
                        int r_x2 = (box_x+box_w);
                        int r_y2 = (box_y+box_h);

                        int x1 =   r_x1*1.0 / SENSOR_WIDTH * osd_frame.cols;
                        int y1 =    r_y1*1.0 / SENSOR_HEIGHT  * osd_frame.rows;

                        int w = (r_x2-r_x1)*1.0 / SENSOR_WIDTH * osd_frame.cols;
                        int h = (r_y2-r_y1)*1.0 / SENSOR_HEIGHT  * osd_frame.rows;

                        cv::rectangle(osd_frame, cv::Rect( x1,y1,w,h ), cv::Scalar(255, 255,0, 0), 8, 2, 0); // ARGB

                    }
                }


                {
                    ScopedTiming st("osd copy", atoi(argv[7]));
                    
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
        }
        #else
        {
            cv::rotate(osd_frame, osd_frame, cv::ROTATE_90_COUNTERCLOCKWISE);
            bool osd_rotate = false;

            float w_z, h_z, s_z;

            {
                ScopedTiming st("get_updated_position : ", atoi(argv[7]));

                pos = get_updated_position();
                w_z = pos[0] + CONTEXT_AMOUNT * (pos[0] + pos[1]);
                h_z = pos[1] + CONTEXT_AMOUNT * (pos[0] + pos[1]);
                s_z = round(sqrt(w_z * h_z));
            }
            
            {
                ScopedTiming st("src_video.pre_process_video : ", atoi(argv[7]));
                src_img_vec.clear();
                src_video.pre_process_video( src_img );
            }
            

            {
                ScopedTiming st("src sub_window : ", atoi(argv[7]));
                src_input = sub_window(src_img, INSTANCE_SIZE, round(s_z * INSTANCE_SIZE / EXEMPLAR_SIZE));
            }
            

            float *crop_output;
            float *src_output;

            if (enter_init)
            {

                int seconds = 8;  //倒计时时长，单位秒
                time_t endtime = time(NULL) + seconds;  //倒计时结束时间

                while (time(NULL) <= endtime) {
                    
                    {
                        ScopedTiming st("read capture", atoi(argv[7]));
                        // VICAP_CHN_ID_1 out rgb888p
                        memset(&dump_info, 0 , sizeof(k_video_frame_info));
                        ret = kd_mpi_vicap_dump_frame(vicap_dev, VICAP_CHN_ID_1, VICAP_DUMP_YUV, &dump_info, 1000);
                        if (ret) {
                            printf("sample_vicap...kd_mpi_vicap_dump_frame failed.\n");
                            continue;
                        }
                    }
                    

                    {
                        ScopedTiming st("isp copy", atoi(argv[7]));
                        // 从vivcap中读取一帧图像到dump_info
                        auto vbvaddr = kd_mpi_sys_mmap_cached(dump_info.v_frame.phys_addr[0], size);
                        memcpy(vaddr, (void *)vbvaddr, SENSOR_HEIGHT * SENSOR_WIDTH * 3);  // 这里以后可以去掉，不用copy
                        kd_mpi_sys_munmap(vbvaddr, size);
                    }

                    cout << "倒计时：" << endtime - time(NULL) << "秒" << endl;

                    if(osd_rotate)
                    {
                        cv::rotate(osd_frame, osd_frame, cv::ROTATE_90_COUNTERCLOCKWISE);
                        osd_rotate = false;
                    }


                    // 初始化跟踪框.
                    int init_x0 = MARGIN * osd_frame.cols;
                    int init_x1 = (1 - MARGIN) * osd_frame.cols;
                    int init_y0 = osd_frame.rows / 2.0 - ((1 - 2 * MARGIN) * osd_frame.cols) / 2.0;
                    int init_y1 = osd_frame.rows / 2.0 + ((1 - 2 * MARGIN) * osd_frame.cols) / 2.0;

                    int init_w = init_x1 - init_x0;
                    int init_h = init_y1 - init_y0;

                    cv::rectangle(osd_frame, cv::Rect( init_x0,init_y0,init_w,init_h ), cv::Scalar(255, 0,255, 0), 8, 2, 0); // ARGB

                    if( (endtime - time(NULL)==2) || (endtime - time(NULL)==3) || (endtime - time(NULL)==4) )
                    {
                        std::cout << " >>>>>> get trackWindow <<<<<<<<" << std::endl;

                        crop_video.pre_process();

                        cv::Mat crop_img = cv::imread( "crop_template.jpg",cv::IMREAD_UNCHANGED );
                        cv::Mat crop_input = sub_window(crop_img, EXEMPLAR_SIZE, round(s_z));
                        cv::imwrite("crop_window.jpg",crop_input);

                        crop.pre_process( crop_input );
                        crop.inference();
                        crop.post_process();
                        crop_output = crop.output;

                    }

                    std::this_thread::sleep_for(std::chrono::seconds(1));

                    if(! osd_rotate)
                    {
                        cv::rotate(osd_frame, osd_frame, cv::ROTATE_90_CLOCKWISE);
                        osd_rotate = true;
                    }
                    
                    {
                        ScopedTiming st("osd copy", atoi(argv[7]));
                        memcpy(pic_vaddr, osd_frame.data, osd_frame.cols * osd_frame.rows * 4);
                        //显示通道插入帧
                        kd_mpi_vo_chn_insert_frame(osd_id+3, &vf_info);  //K_VO_OSD0
                        // printf("kd_mpi_vo_chn_insert_frame success \n");

                        ret = kd_mpi_vicap_dump_release(vicap_dev, VICAP_CHN_ID_1, &dump_info);
                        if (ret) {
                            printf("sample_vicap...kd_mpi_vicap_dump_release failed.\n");
                        }
                    }

                }

                cout << ">>>>>>>  Play  <<<<<<<" << endl;
                enter_init = false;

            }
            else
            {
                {
                    ScopedTiming st("read capture", atoi(argv[7]));
                    // VICAP_CHN_ID_1 out rgb888p
                    memset(&dump_info, 0 , sizeof(k_video_frame_info));
                    ret = kd_mpi_vicap_dump_frame(vicap_dev, VICAP_CHN_ID_1, VICAP_DUMP_YUV, &dump_info, 1000);
                    if (ret) {
                        printf("sample_vicap...kd_mpi_vicap_dump_frame failed.\n");
                        continue;
                    }
                }
                    

                {
                    ScopedTiming st("isp copy", atoi(argv[7]));
                    // 从vivcap中读取一帧图像到dump_info
                    auto vbvaddr = kd_mpi_sys_mmap_cached(dump_info.v_frame.phys_addr[0], size);
                    memcpy(vaddr, (void *)vbvaddr, SENSOR_HEIGHT * SENSOR_WIDTH * 3);  // 这里以后可以去掉，不用copy
                    kd_mpi_sys_munmap(vbvaddr, size);
                }

                {
                    ScopedTiming st("src pipeline: ", atoi(argv[7]));
                    src.pre_process(src_input);
                    src.inference();
                    src.post_process();
                }
                

                src_output = src.output;

                std::vector<float*> inputs;
                std::vector<float*> outputs;

                {
                    ScopedTiming st("head pipeline: ", atoi(argv[7]));
                    inputs.clear();
                    inputs.push_back(crop_output);
                    inputs.push_back(src_output);

                    std::vector<int> input_shapes = { 1*48*8*8,1*48*16*16 };
                    for ( int j = 0; j < 2; j++)
                    {
                        auto desc = interp.input_desc(j);
                        auto shape = interp.input_shape(j);
                        auto tensor = host_runtime_tensor::create(desc.datatype, shape, hrt::pool_shared).expect("cannot create input tensor");
                        auto mapped_buf = std::move(hrt::map(tensor, map_access_::map_write).unwrap());
                        
                        memcpy(reinterpret_cast<void *>(mapped_buf.buffer().data()), reinterpret_cast<void *>( inputs[j]), input_shapes[j]*4);
                        auto ret = mapped_buf.unmap();
                        ret = hrt::sync(tensor, sync_op_t::sync_write_back, true);
                        if (!ret.is_ok())
                        {
                            std::cerr << "hrt::sync failed" << std::endl;
                            std::abort();
                        }

                        interp.input_tensor(j, tensor).expect("cannot set input tensor");
                    }

                    for (size_t i = 0; i < interp.outputs_size(); i++)
                    {
                        auto desc = interp.output_desc(i);
                        auto shape = interp.output_shape(i);
                        auto tensor = host_runtime_tensor::create(desc.datatype, shape, hrt::pool_shared).expect("cannot create output tensor");
                        interp.output_tensor(i, tensor).expect("cannot set output tensor");
                    }

                    auto start = std::chrono::steady_clock::now();
                    interp.run().expect("error occurred in running model");
                    auto stop = std::chrono::steady_clock::now();
                    double duration = std::chrono::duration<double, std::milli>(stop - start).count();

                    outputs.clear();
                    for ( int j = 0; j<2;  j++)
                    {
                        auto out = interp.output_tensor(j).expect("cannot get output tensor");
                        auto mapped_buf = std::move(hrt::map(out, map_access_::map_read).unwrap());

                        outputs.push_back( (float *)mapped_buf.buffer().data() );
                        
                    }
                }
                

                float* score, * box;
                score = outputs[0];
                box = outputs[1];

                int box_x, box_y, box_h, box_w;
                float best_score;
                {
                    ScopedTiming st("head post_process : ", atoi(argv[7]));
                    post_process(score, box, SENSOR_WIDTH, SENSOR_HEIGHT, box_x, box_y, box_w, box_h, best_score);
                }
                

                {
                    ScopedTiming st("draw box", atoi(argv[7]));
                    if (best_score > thresh) 
                    {

                        int r_x1 = box_x;
                        int r_y1 = box_y;
                        int r_x2 = (box_x+box_w);
                        int r_y2 = (box_y+box_h);

                        int x1 =   r_x1*1.0 / SENSOR_WIDTH * osd_frame.cols;
                        int y1 =    r_y1*1.0 / SENSOR_HEIGHT  * osd_frame.rows;

                        int w = (r_x2-r_x1)*1.0 / SENSOR_WIDTH * osd_frame.cols;
                        int h = (r_y2-r_y1)*1.0 / SENSOR_HEIGHT  * osd_frame.rows;

                        cv::rectangle(osd_frame, cv::Rect( x1,y1,w,h ), cv::Scalar(255, 255,0, 0), 8, 2, 0); // ARGB

                    }
                }
                cv::rotate(osd_frame, osd_frame, cv::ROTATE_90_CLOCKWISE);


                {
                    ScopedTiming st("osd copy", atoi(argv[7]));
                    
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
        }
        #endif

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
    if (argc != 8)
    {
        print_usage(argv[0]);
        return -1;
    }

    {
        #if defined(CONFIG_BOARD_K230_CANMV)
        {
            std::thread thread_isp(video_proc_v1, argv);
            while (getchar() != 'q')
            {
                usleep(10000);
            }

            isp_stop = true;
            thread_isp.join();
        }
        #elif defined(CONFIG_BOARD_K230_CANMV_V2)
        {
            std::thread thread_isp(video_proc_v2, argv);
            while (getchar() != 'q')
            {
                usleep(10000);
            }

            isp_stop = true;
            thread_isp.join();
        }
        #elif defined(CONFIG_BOARD_K230D_CANMV)
        {
            std::thread thread_isp(video_proc_k230d, argv);
            while (getchar() != 'q')
            {
                usleep(10000);
            }

            isp_stop = true;
            thread_isp.join();
        }
        #elif defined(CONFIG_BOARD_K230_CANMV_01STUDIO)
        {
            std::thread thread_isp(video_proc_01, argv);
            while (getchar() != 'q')
            {
                usleep(10000);
            }

            isp_stop = true;
            thread_isp.join();
        }
        #else
        {
            std::thread thread_isp(video_proc_v1, argv);
            while (getchar() != 'q')
            {
                usleep(10000);
            }

            isp_stop = true;
            thread_isp.join();
        }
        #endif
    }
    
    return 0;
}
