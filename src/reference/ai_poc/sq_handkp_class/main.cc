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
#include <chrono>
#include <fstream>
#include <thread>

#include "utils.h"
#include "vi_vo.h"
#include "hand_detection.h"
#include "hand_keypoint.h"


std::atomic<bool> isp_stop(false);

void print_usage(const char *name)
{
	cout << "Usage: " << name << "<kmodel_det> <input_mode> <obj_thresh> <nms_thresh> <kmodel_kp> <debug_mode>" << endl
		 << "Options:" << endl
		 << "  kmodel_det      手掌检测kmodel路径\n"
		 << "  input_mode      本地图片(图片路径)/ 摄像头(None) \n"
         << "  obj_thresh      手掌检测阈值\n"
         << "  nms_thresh      手掌检测非极大值抑制阈值\n"
		 << "  kmodel_kp       手势关键点检测kmodel路径\n"
		 << "  debug_mode      是否需要调试，0、1、2分别表示不调试、简单调试、详细调试\n"
		 << "\n"
		 << endl;
}

void video_proc(char *argv[])
{
    vivcap_start();
    // 设置osd参数
    k_video_frame_info vf_info;
    void *pic_vaddr = NULL; // osd
    memset(&vf_info, 0, sizeof(vf_info));
    vf_info.v_frame.width = osd_width;
    vf_info.v_frame.height = osd_height;
    vf_info.v_frame.stride[0] = osd_width;
    vf_info.v_frame.pixel_format = PIXEL_FORMAT_ARGB_8888;
    block = vo_insert_frame(&vf_info, &pic_vaddr);

    // alloc memory,get isp memory
    size_t paddr = 0;
    void *vaddr = nullptr;
    size_t size = SENSOR_CHANNEL * SENSOR_HEIGHT * SENSOR_WIDTH;
    int ret = kd_mpi_sys_mmz_alloc_cached(&paddr, &vaddr, "allocate", "anonymous", size);
    if (ret)
    {
        std::cerr << "physical_memory_block::allocate failed: ret = " << ret << ", errno = " << strerror(errno) << std::endl;
        std::abort();
    }

    HandDetection hd(argv[1], atof(argv[3]), atof(argv[4]), {SENSOR_WIDTH, SENSOR_HEIGHT}, {SENSOR_CHANNEL, SENSOR_HEIGHT, SENSOR_WIDTH}, reinterpret_cast<uintptr_t>(vaddr), reinterpret_cast<uintptr_t>(paddr), atoi(argv[6]));

    HandKeypoint hk(argv[5], {SENSOR_CHANNEL, SENSOR_HEIGHT, SENSOR_WIDTH}, reinterpret_cast<uintptr_t>(vaddr), reinterpret_cast<uintptr_t>(paddr), atoi(argv[6]));

    std::vector<BoxInfo> results;
    while (!isp_stop)
    {
        ScopedTiming st("total time", 1);

        {
            ScopedTiming st("read capture", atoi(argv[6]));
            // 从vivcap中读取一帧图像到dump_info
            memset(&dump_info, 0, sizeof(k_video_frame_info));
            ret = kd_mpi_vicap_dump_frame(vicap_dev, VICAP_CHN_ID_1, VICAP_DUMP_YUV, &dump_info, 1000);
            if (ret)
            {
                printf("sample_vicap...kd_mpi_vicap_dump_frame failed.\n");
                continue;
            }
        }

        {
            ScopedTiming st("isp copy", atoi(argv[6]));
            auto vbvaddr = kd_mpi_sys_mmap_cached(dump_info.v_frame.phys_addr[0], size);
            memcpy(vaddr, (void *)vbvaddr, SENSOR_HEIGHT * SENSOR_WIDTH * 3); 
            kd_mpi_sys_munmap(vbvaddr, size);
        }

        results.clear();

        hd.pre_process();
        hd.inference();
        hd.post_process(results);

        cv::Mat osd_frame(osd_height, osd_width, CV_8UC4, cv::Scalar(0, 0, 0, 0));
        #if defined(CONFIG_BOARD_K230D_CANMV)
        {
            cv::rotate(osd_frame, osd_frame, cv::ROTATE_90_COUNTERCLOCKWISE);
        }
        #elif defined(CONFIG_BOARD_K230_CANMV_01STUDIO)
        {
            #if defined(STUDIO_HDMI)
            {}
            #else
            {
                cv::rotate(osd_frame, osd_frame, cv::ROTATE_90_COUNTERCLOCKWISE);
            }
            #endif
        }
        #else
        {
        }
		#endif

        
        for (auto r: results)
        {
            std::string text = hd.labels_[r.label] + ":" + std::to_string(round(r.score * 100) / 100.0);
            std::cout << "text = " << text << std::endl;

            int w = r.x2 - r.x1 + 1;
            int h = r.y2 - r.y1 + 1;
            
            int length = std::max(w,h)/2;
            int cx = (r.x1+r.x2)/2;
            int cy = (r.y1+r.y2)/2;
            int ratio_num = 1.26*length;

            int x1_1 = std::max(0,cx-ratio_num);
            int y1_1 = std::max(0,cy-ratio_num);
            int x2_1 = std::min(SENSOR_WIDTH-1, cx+ratio_num);
            int y2_1 = std::min(SENSOR_HEIGHT-1, cy+ratio_num);
            int w_1 = x2_1 - x1_1 + 1;
            int h_1 = y2_1 - y1_1 + 1;
            
            struct Bbox bbox = {x:x1_1,y:y1_1,w:w_1,h:h_1};
            hk.pre_process(bbox);
            hk.inference();
            hk.post_process(bbox);

            std::vector<double> angle_list = hk.hand_angle();
            std::string gesture = hk.h_gesture(angle_list);

            #if defined(CONFIG_BOARD_K230D_CANMV)
            {
                int rect_x = r.x1/ SENSOR_WIDTH * osd_height;
                int rect_y = r.y1/ SENSOR_HEIGHT * osd_width;
                int rect_w = (float)w / SENSOR_WIDTH * osd_height;
                int rect_h = (float)h / SENSOR_HEIGHT  * osd_width;

                cv::rectangle(osd_frame, cv::Rect(rect_x, rect_y , rect_w, rect_h), cv::Scalar( 255,255, 255, 255), 2, 2, 0);
                std::string text1 = "Gesture: " + gesture;
                cv::putText(osd_frame, text1, cv::Point(rect_x,rect_y),cv::FONT_HERSHEY_COMPLEX, 1, cv::Scalar(255, 255, 195, 0), 2);
                cv::putText(osd_frame, text1, cv::Point(rect_x,rect_y),cv::FONT_HERSHEY_COMPLEX, 1, cv::Scalar(255, 0, 150, 255), 2);
            }
            #elif defined(CONFIG_BOARD_K230_CANMV_01STUDIO)
            {
                #if defined(STUDIO_HDMI)
                {
                    int rect_x = r.x1/ SENSOR_WIDTH * osd_width;
                    int rect_y = r.y1/ SENSOR_HEIGHT * osd_height;
                    int rect_w = (float)w / SENSOR_WIDTH * osd_width;
                    int rect_h = (float)h / SENSOR_HEIGHT  * osd_height;

                    cv::rectangle(osd_frame, cv::Rect(rect_x, rect_y , rect_w, rect_h), cv::Scalar( 255,255, 255, 255), 2, 2, 0);
                    std::string text1 = "Gesture: " + gesture;
                    cv::putText(osd_frame, text1, cv::Point(rect_x,rect_y),cv::FONT_HERSHEY_COMPLEX, 1, cv::Scalar(255, 255, 195, 0), 2);
                    cv::putText(osd_frame, text1, cv::Point(rect_x,rect_y),cv::FONT_HERSHEY_COMPLEX, 1, cv::Scalar(255, 0, 150, 255), 2);
                }
                #else
                {
                    int rect_x = r.x1/ SENSOR_WIDTH * osd_height;
                    int rect_y = r.y1/ SENSOR_HEIGHT * osd_width;
                    int rect_w = (float)w / SENSOR_WIDTH * osd_height;
                    int rect_h = (float)h / SENSOR_HEIGHT  * osd_width;

                    cv::rectangle(osd_frame, cv::Rect(rect_x, rect_y , rect_w, rect_h), cv::Scalar( 255,255, 255, 255), 2, 2, 0);
                    std::string text1 = "Gesture: " + gesture;
                    cv::putText(osd_frame, text1, cv::Point(rect_x,rect_y),cv::FONT_HERSHEY_COMPLEX, 1, cv::Scalar(255, 255, 195, 0), 2);
                    cv::putText(osd_frame, text1, cv::Point(rect_x,rect_y),cv::FONT_HERSHEY_COMPLEX, 1, cv::Scalar(255, 0, 150, 255), 2);
                }
                #endif
            }
            #else
            {
                int rect_x = r.x1/ SENSOR_WIDTH * osd_width;
                int rect_y = r.y1/ SENSOR_HEIGHT * osd_height;
                int rect_w = (float)w / SENSOR_WIDTH * osd_width;
                int rect_h = (float)h / SENSOR_HEIGHT  * osd_height;

                cv::rectangle(osd_frame, cv::Rect(rect_x, rect_y , rect_w, rect_h), cv::Scalar( 255,255, 255, 255), 2, 2, 0);
                std::string text1 = "Gesture: " + gesture;
                cv::putText(osd_frame, text1, cv::Point(rect_x,rect_y),cv::FONT_HERSHEY_COMPLEX, 1, cv::Scalar(255, 255, 195, 0), 2);
                cv::putText(osd_frame, text1, cv::Point(rect_x,rect_y),cv::FONT_HERSHEY_COMPLEX, 1, cv::Scalar(255, 0, 150, 255), 2);
            }
            #endif
            
            {
                ScopedTiming st("osd draw keypoints", atoi(argv[6]));
                hk.draw_keypoints(osd_frame, text, bbox, false);
            }
        }

        #if defined(CONFIG_BOARD_K230D_CANMV)
        {
            cv::rotate(osd_frame, osd_frame, cv::ROTATE_90_CLOCKWISE);
        }
        #elif defined(CONFIG_BOARD_K230_CANMV_01STUDIO)
        {
            #if defined(STUDIO_HDMI)
            {}
            #else
            {
                cv::rotate(osd_frame, osd_frame, cv::ROTATE_90_CLOCKWISE);
            }
            #endif
        }
        #else
        {
        }
		#endif

        {
            ScopedTiming st("osd copy", atoi(argv[6]));
            memcpy(pic_vaddr, osd_frame.data, osd_width * osd_height * 4);
            // 显示通道插入帧
            kd_mpi_vo_chn_insert_frame(osd_id + 3, &vf_info); // K_VO_OSD0

            ret = kd_mpi_vicap_dump_release(vicap_dev, VICAP_CHN_ID_1, &dump_info);
            if (ret)
            {
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

    if (strcmp(argv[2], "None") == 0)
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
        cv::Mat img = cv::imread(argv[2]);
        cv::Mat img_draw = img.clone();

        int origin_w = img.cols;
        int origin_h = img.rows;
        FrameSize handimg_size = {origin_w, origin_h};

        HandDetection hd(argv[1], atof(argv[3]), atof(argv[4]),  handimg_size, atoi(argv[6]));

        HandKeypoint hk(argv[5], atoi(argv[6]));

        hd.pre_process(img);

        hd.inference();

        std::vector<BoxInfo> result_hd;
        hd.post_process(result_hd);

        for (auto r : result_hd)
        {
            std::string text = hd.labels_[r.label] + ":" + std::to_string(round(r.score * 100) / 100.0);
            std::cout << "text = " << text << std::endl;

            int w = r.x2 - r.x1 + 1;
            int h = r.y2 - r.y1 + 1;
            cv::rectangle(img_draw, cv::Rect(static_cast<int>(r.x1), static_cast<int>(r.y1) , w, h), cv::Scalar(255, 255, 255), 2, 2, 0);
            
            int length = std::max(w,h)/2;
            int cx = (r.x1+r.x2)/2;
            int cy = (r.y1+r.y2)/2;
            int ratio_num = 1.26*length;

            int x1_1 = std::max(0,cx-ratio_num);
            int y1_1 = std::max(0,cy-ratio_num);
            int x2_1 = std::min(origin_w-1, cx+ratio_num);
            int y2_1 = std::min(origin_h-1, cy+ratio_num);
            int w_1 = x2_1 - x1_1+1;
            int h_1 = y2_1 - y1_1+1;

            struct Bbox bbox = {x:x1_1,y:y1_1,w:w_1,h:h_1};
            hk.pre_process(img, bbox);
            hk.inference();
            hk.post_process(bbox);
            hk.draw_keypoints(img_draw,text, bbox, true);

            std::vector<double> angle_list = hk.hand_angle();
            std::string gesture = hk.h_gesture(angle_list);

            std::string text1 = "Gesture: " + gesture;
            cv::putText(img_draw, text1, cv::Point(r.x1,r.y1),cv::FONT_HERSHEY_COMPLEX, 0.45, cv::Scalar(255, 195, 0), 1);
            cv::putText(img_draw, text1, cv::Point(r.x1,r.y1),cv::FONT_HERSHEY_COMPLEX, 0.45, cv::Scalar(0, 150, 255), 1);
        }

        cv::imwrite("handkpclass_result.jpg", img_draw);
    }
    return 0;
}
