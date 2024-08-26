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
#include "utils.h"
#include "vi_vo.h"
#include "person_detect.h"

using std::cerr;
using std::cout;
using std::endl;

std::atomic<bool> isp_stop(false);


void print_usage(const char *name)
{
    cout << "Usage: " << name << "<kmodel> <pd_thresh> <nms_thresh> <input_mode> <debug_mode>" << endl
         << "For example: " << endl
         << " [for isp] ./person_distance.elf person_detect.kmodel 0.5 0.45 None 170 0" << endl
         << "Options:" << endl
         << " 1> kmodel         行人检测kmodel文件路径 \n"
         << " 2> pd_thresh      行人检测阈值\n"
         << " 3> nms_thresh     NMS阈值\n"
         << " 4> input_mode     本地图片(图片路径)/ 摄像头(None) \n"
         << " 5> person_height  行人身高 \n"
         << " 6> debug_mode     是否需要调试, 0、1、2分别表示不调试、简单调试、详细调试\n"
         << "\n"
         << endl;
}


float detect_distance_people(float person_height, int height)
{
    float foc = 2337.8;   // 焦距
    float person = person_height / 2.54;  // 行人身高
    float dis_i = (person * foc) / (height - 2);
    float dis_c = dis_i * 2.54;
    return dis_c;
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

    personDetect pd(argv[1], atof(argv[2]),atof(argv[3]), {SENSOR_CHANNEL, SENSOR_HEIGHT, SENSOR_WIDTH}, reinterpret_cast<uintptr_t>(vaddr), reinterpret_cast<uintptr_t>(paddr), atoi(argv[6]));

    vector<BoxInfo> results;

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

        results.clear();

        pd.pre_process();
        pd.inference();
        pd.post_process({SENSOR_WIDTH, SENSOR_HEIGHT}, results);

        cv::Mat osd_frame(osd_height, osd_width, CV_8UC4, cv::Scalar(0, 0, 0, 0));

        std::string text;
        cv::Point origin;

        
        #if defined(CONFIG_BOARD_K230D_CANMV)
        {
            cv::rotate(osd_frame, osd_frame, cv::ROTATE_90_COUNTERCLOCKWISE);
            for (auto r : results)
            {
                ScopedTiming st("draw img", atoi(argv[6]));
                text = pd.labels[r.label] + ":" + std::to_string(round(r.score * 100) / 100).substr(0,4);

                int x1 =   r.x1 / SENSOR_WIDTH * osd_height;
                int y1 =  r.y1 / SENSOR_HEIGHT  * osd_width;

                int w = (r.x2-r.x1) / SENSOR_WIDTH * osd_height;
                int h = (r.y2-r.y1) / SENSOR_HEIGHT  * osd_width;

                float dis = detect_distance_people(atof(argv[5]), h);

                cv::rectangle(osd_frame, cv::Rect( x1,y1,w,h ), cv::Scalar(255, 255,0, 0), 6, 2, 0); // ARGB
                std::string text = "dis:" + std::to_string(int(dis)) + "cm";
                cv::putText(osd_frame, text, cv::Point(x1,y1+20),cv::FONT_HERSHEY_COMPLEX, 1, cv::Scalar(255, 255, 195, 0), 2);
            }
            cv::rotate(osd_frame, osd_frame, cv::ROTATE_90_CLOCKWISE);

        }
        #elif defined(CONFIG_BOARD_K230_CANMV_01STUDIO)
        {
            #if defined(STUDIO_HDMI)
            {
                for (auto r : results)
                {
                    ScopedTiming st("draw img", atoi(argv[6]));
                    text = pd.labels[r.label] + ":" + std::to_string(round(r.score * 100) / 100).substr(0,4);

                    int x1 =   r.x1 / SENSOR_WIDTH * osd_width;
                    int y1 =  r.y1 / SENSOR_HEIGHT  * osd_height;

                    int w = (r.x2-r.x1) / SENSOR_WIDTH * osd_width;
                    int h = (r.y2-r.y1) / SENSOR_HEIGHT  * osd_height;

                    float dis = detect_distance_people(atof(argv[5]), h);

                    cv::rectangle(osd_frame, cv::Rect( x1,y1,w,h ), cv::Scalar(255, 255,0, 0), 6, 2, 0); // ARGB
                    std::string text = "dis:" + std::to_string(int(dis)) + "cm";
                    cv::putText(osd_frame, text, cv::Point(x1,y1+20),cv::FONT_HERSHEY_COMPLEX, 1, cv::Scalar(255, 255, 195, 0), 2);
                }
            }
            #else
            {
                cv::rotate(osd_frame, osd_frame, cv::ROTATE_90_COUNTERCLOCKWISE);
                for (auto r : results)
                {
                    ScopedTiming st("draw img", atoi(argv[6]));
                    text = pd.labels[r.label] + ":" + std::to_string(round(r.score * 100) / 100).substr(0,4);

                    int x1 =   r.x1 / SENSOR_WIDTH * osd_height;
                    int y1 =  r.y1 / SENSOR_HEIGHT  * osd_width;

                    int w = (r.x2-r.x1) / SENSOR_WIDTH * osd_height;
                    int h = (r.y2-r.y1) / SENSOR_HEIGHT  * osd_width;

                    float dis = detect_distance_people(atof(argv[5]), h);

                    cv::rectangle(osd_frame, cv::Rect( x1,y1,w,h ), cv::Scalar(255, 255,0, 0), 6, 2, 0); // ARGB
                    std::string text = "dis:" + std::to_string(int(dis)) + "cm";
                    cv::putText(osd_frame, text, cv::Point(x1,y1+20),cv::FONT_HERSHEY_COMPLEX, 1, cv::Scalar(255, 255, 195, 0), 2);
                }
                cv::rotate(osd_frame, osd_frame, cv::ROTATE_90_CLOCKWISE);
            }
            #endif
        }
        #else
        {
            for (auto r : results)
            {
                ScopedTiming st("draw img", atoi(argv[6]));
                text = pd.labels[r.label] + ":" + std::to_string(round(r.score * 100) / 100).substr(0,4);

                int x1 =   r.x1 / SENSOR_WIDTH * osd_width;
                int y1 =  r.y1 / SENSOR_HEIGHT  * osd_height;

                int w = (r.x2-r.x1) / SENSOR_WIDTH * osd_width;
                int h = (r.y2-r.y1) / SENSOR_HEIGHT  * osd_height;

                float dis = detect_distance_people(atof(argv[5]), h);

                cv::rectangle(osd_frame, cv::Rect( x1,y1,w,h ), cv::Scalar(255, 255,0, 0), 6, 2, 0); // ARGB
                std::string text = "dis:" + std::to_string(int(dis)) + "cm";
                cv::putText(osd_frame, text, cv::Point(x1,y1+20),cv::FONT_HERSHEY_COMPLEX, 1, cv::Scalar(255, 255, 195, 0), 2);
            }
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
    if (argc != 7)
    {
        print_usage(argv[0]);
        return -1;
    }

    if (strcmp(argv[4], "None") == 0)
    {
        std::thread thread_isp(video_proc, argv);
        while (getchar() != 'q')
        {
            usleep(10000);
        }

        isp_stop = true;
        thread_isp.join();
    }
    else
    {
        cv::Mat ori_img = cv::imread(argv[4]);
        int ori_w = ori_img.cols;
        int ori_h = ori_img.rows;

        personDetect pd(argv[1], atof(argv[2]),atof(argv[3]), atoi(argv[6]));

        pd.pre_process(ori_img);
        pd.inference();

        vector<BoxInfo> results;
        pd.post_process({ori_w, ori_h}, results);

        cv::Point origin;
        for (auto r : results)
        {
            ScopedTiming st("draw boxes", atoi(argv[6]));
            cv::rectangle(ori_img, cv::Rect(r.x1, r.y1, r.x2 - r.x1 + 1, r.y2 - r.y1 + 1), cv::Scalar(0, 0, 255), 6, 2, 0);

            origin.x = r.x1;
            origin.y = r.y1 - 20;

            int w = (r.x2-r.x1) / ori_w * osd_width;
            int h = (r.y2-r.y1) / ori_h  * osd_height;
            float dis = detect_distance_people(atof(argv[5]), h);

            std::string text = "dis:" + std::to_string(int(dis)) + "cm";
            cv::putText(ori_img, text, cv::Point(r.x1,r.y1+20),cv::FONT_HERSHEY_COMPLEX, 1, cv::Scalar(255, 195, 0), 2);
        }

        cv::imwrite("pd_result.jpg", ori_img);
    }
    return 0;
}