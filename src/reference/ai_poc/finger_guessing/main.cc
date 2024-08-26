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
	cout << "Usage: " << name << "<kmodel_det> <obj_thresh> <nms_thresh> <kmodel_kp> <guess_mode> <debug_mode>" << endl
		 << "Options:" << endl
		 << "  kmodel_det      手掌检测kmodel路径\n"
         << "  obj_thresh      手掌检测阈值\n"
         << "  nms_thresh      手掌检测非极大值抑制阈值\n"
		 << "  kmodel_kp       手势关键点检测kmodel路径\n"
         << "  guess_mode      石头剪刀布的游戏模式 0(玩家稳赢) 1(玩家必输) 奇数n(n局定输赢)\n"
		 << "  debug_mode      是否需要调试，0、1、2分别表示不调试、简单调试、详细调试\n"
		 << "\n"
		 << endl;
}


void video_proc_v1(char *argv[])
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

    // 读取石头剪刀布的bin文件数据 并且转换为mat类型
    int bu_width = 400;
    int bu_height = 400;
    cv::Mat image_bu_argb;
    Utils::bin_2_mat("bu.bin", bu_width, bu_height, image_bu_argb);

    int shitou_width = 400;
    int shitou_height = 400;
    cv::Mat image_shitou_argb;
    Utils::bin_2_mat("shitou.bin", shitou_width, shitou_height, image_shitou_argb);

    int jiandao_width = 400;
    int jiandao_height = 400;
    cv::Mat image_jiandao_argb;
    Utils::bin_2_mat("jiandao.bin", jiandao_width, jiandao_height, image_jiandao_argb);

    // 设置游戏模式
    static int MODE = atoi(argv[5]);
    int counts_guess = -1;
    int player_win = 0;
    int k230_win = 0;
    bool sleep_end = false;
    bool set_stop_id = true;
    std::vector<std::string> LIBRARY = {"fist","yeah","five"};

    HandDetection hd(argv[1], atof(argv[2]), atof(argv[3]), {SENSOR_WIDTH, SENSOR_HEIGHT}, {SENSOR_CHANNEL, SENSOR_HEIGHT, SENSOR_WIDTH}, reinterpret_cast<uintptr_t>(vaddr), reinterpret_cast<uintptr_t>(paddr), atoi(argv[6]));
    sync();
    HandKeypoint hk(argv[4], {SENSOR_CHANNEL, SENSOR_HEIGHT, SENSOR_WIDTH}, reinterpret_cast<uintptr_t>(vaddr), reinterpret_cast<uintptr_t>(paddr), atoi(argv[6]));
    sync();

    cv::Mat osd_frame_tmp(osd_height, osd_width, CV_8UC4, cv::Scalar(0, 0, 0, 0));
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

        cv::Mat osd_frame(osd_height, osd_width, CV_8UC4, cv::Scalar(0, 0, 0, 0));
        cv::Mat osd_frame_out;//(osd_height, osd_width, CV_8UC4, cv::Scalar(0, 0, 0, 0));
        cv::Mat osd_frame_vertical;
        cv::Mat osd_frame_horizontal;

        results.clear();
        hd.pre_process();
        hd.inference();
        hd.post_process(results);

        float max_area_hand = 0;
        int max_id_hand = -1;
        for (int i = 0; i < results.size(); ++i)
        {
            float area_i = (results[i].x2 - results[i].x1) * (results[i].y2 - results[i].y1);
            if (area_i > max_area_hand)
            {
                max_area_hand = area_i;
                max_id_hand = i;
            }
        }

        std::string gesture = "";
        if (max_id_hand != -1)
        {
            std::string text = hd.labels_[results[max_id_hand].label] + ":" + std::to_string(round(results[max_id_hand].score * 100) / 100.0);

            int w = results[max_id_hand].x2 - results[max_id_hand].x1 + 1;
            int h = results[max_id_hand].y2 - results[max_id_hand].y1 + 1;
            
            int rect_x = results[max_id_hand].x1/ SENSOR_WIDTH * osd_width;
            int rect_y = results[max_id_hand].y1/ SENSOR_HEIGHT * osd_height;
            int rect_w = (float)w / SENSOR_WIDTH * osd_width;
            int rect_h = (float)h / SENSOR_HEIGHT  * osd_height;
            // cv::rectangle(osd_frame, cv::Rect(rect_x, rect_y , rect_w, rect_h), cv::Scalar( 255,255, 255, 255), 2, 2, 0);
            
            int length = std::max(w,h)/2;
            int cx = (results[max_id_hand].x1+results[max_id_hand].x2)/2;
            int cy = (results[max_id_hand].y1+results[max_id_hand].y2)/2;
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
            gesture = hk.h_gesture(angle_list);

            std::string text1 = "Gesture: " + gesture;
        }


        if(MODE == 0)
        {
            {
                ScopedTiming st("osd draw", atoi(argv[6]));

                if(gesture == "fist")
                {
                    cv::Mat copy_ori_image = osd_frame(cv::Rect(20,20,jiandao_width,jiandao_height));
                    image_jiandao_argb.copyTo(copy_ori_image,image_jiandao_argb); 
                }
                else if(gesture == "five")
                {
                    cv::Mat copy_ori_image = osd_frame(cv::Rect(20,20,shitou_width,shitou_height));
                    image_shitou_argb.copyTo(copy_ori_image,image_shitou_argb); 
                }
                else if(gesture == "yeah")
                {
                    cv::Mat copy_ori_image = osd_frame(cv::Rect(20,20,bu_width,bu_height));
                    image_bu_argb.copyTo(copy_ori_image,image_bu_argb);  
                }
            }
        }
        else if (MODE == 1)
        {
            {
                ScopedTiming st("osd draw", atoi(argv[6]));

                if(gesture == "fist")
                {
                    cv::Mat copy_ori_image = osd_frame(cv::Rect(20,20,bu_width,bu_height));
                    image_bu_argb.copyTo(copy_ori_image,image_bu_argb); 
                }
                else if(gesture == "five")
                {
                    cv::Mat copy_ori_image = osd_frame(cv::Rect(20,20,jiandao_width,jiandao_height));
                    image_jiandao_argb.copyTo(copy_ori_image,image_jiandao_argb);  
                }
                else if(gesture == "yeah")
                {
                    cv::Mat copy_ori_image = osd_frame(cv::Rect(20,20,shitou_width,shitou_height));
                    image_shitou_argb.copyTo(copy_ori_image,image_shitou_argb); 
                }
            }
        }
        else
        {
            if(sleep_end)
            {
                usleep(2000000);
                sleep_end = false;
            }

            if(max_id_hand == -1)
            {
                set_stop_id = true;
            }

            if(counts_guess == -1 && gesture != "fist" && gesture != "yeah" && gesture != "five")
            {
                std::string start_txt = " G A M E   S T A R T ";
                std::string oneset_txt = std::to_string(1) + "  S E T";
                cv::putText(osd_frame, start_txt, cv::Point(200,500),cv::FONT_HERSHEY_COMPLEX, 2, cv::Scalar(255, 255, 0, 0), 4);
                cv::putText(osd_frame, oneset_txt, cv::Point(400,600),cv::FONT_HERSHEY_COMPLEX, 2, cv::Scalar(255, 0, 150, 255), 4);
            }
            else if(counts_guess == MODE)
            {
                // osd_frame = cv::Mat(osd_height, osd_width, CV_8UC4, cv::Scalar(0, 0, 0, 0));
                if(k230_win > player_win)
                {
                    cv::putText(osd_frame, "Y O U   L O S E", cv::Point(340,500),cv::FONT_HERSHEY_COMPLEX, 2, cv::Scalar(255, 255, 0, 0), 4);
                }
                else if(k230_win < player_win)
                {
                    cv::putText(osd_frame, "Y O U   W I N", cv::Point(340,500),cv::FONT_HERSHEY_COMPLEX, 2, cv::Scalar(255, 255, 0, 0), 4);
                }
                else
                {
                    cv::putText(osd_frame, "T I E   G A M E", cv::Point(340,500),cv::FONT_HERSHEY_COMPLEX, 2, cv::Scalar(255, 255, 0, 0), 4);
                }
                counts_guess = -1;
                player_win = 0;
                k230_win = 0;

                sleep_end = true;
            }
            else
            {
                if(set_stop_id)
                {
                    if(counts_guess == -1 && (gesture == "fist" || gesture == "yeah" || gesture == "five"))
                    {
                        counts_guess = 0;
                    }

                    if(counts_guess != -1 && (gesture == "fist" || gesture == "yeah" || gesture == "five"))
                    {
                        int k230_guess=rand()%3;
                        if(gesture == "fist" && LIBRARY[k230_guess] == "yeah")
                        {
                            player_win += 1;
                        }
                        else if(gesture == "fist" && LIBRARY[k230_guess] == "five")
                        {
                            k230_win += 1;
                        }
                        if(gesture == "yeah" && LIBRARY[k230_guess] == "fist")
                        {
                            k230_win += 1;
                        }
                        else if(gesture == "yeah" && LIBRARY[k230_guess] == "five")
                        {
                            player_win += 1;
                        }
                        if(gesture == "five" && LIBRARY[k230_guess] == "fist")
                        {
                            player_win += 1;
                        }
                        else if(gesture == "five" && LIBRARY[k230_guess] == "yeah")
                        {
                            k230_win += 1;
                        }

                        if(LIBRARY[k230_guess] == "fist")
                        {
                            cv::Mat copy_ori_image = osd_frame(cv::Rect(20,20,shitou_width,shitou_height));
                            image_shitou_argb.copyTo(copy_ori_image,image_shitou_argb);  
                        }
                        else if(LIBRARY[k230_guess] == "five")
                        {
                            cv::Mat copy_ori_image = osd_frame(cv::Rect(20,20,bu_width,bu_height));
                            image_bu_argb.copyTo(copy_ori_image,image_bu_argb);  
                        }
                        else if(LIBRARY[k230_guess] == "yeah")
                        {
                            cv::Mat copy_ori_image = osd_frame(cv::Rect(20,20,jiandao_width,jiandao_height));
                            image_jiandao_argb.copyTo(copy_ori_image,image_jiandao_argb);  
                        }
                        counts_guess += 1;

                        std::string set_txt = std::to_string(counts_guess) + "  S E T";
                        cv::putText(osd_frame, set_txt, cv::Point(400,600),cv::FONT_HERSHEY_COMPLEX, 2, cv::Scalar(255, 0, 150, 255), 4);
                        osd_frame_tmp = osd_frame;
                        set_stop_id = false;
                        sleep_end = true;
                    }
                    else
                    {
                        std::string set_txt = std::to_string(counts_guess + 1) + "  S E T";
                        cv::putText(osd_frame, set_txt, cv::Point(400,600),cv::FONT_HERSHEY_COMPLEX, 2, cv::Scalar(255, 0, 150, 255), 4);
                    }
                }
                else
                {
                    osd_frame = osd_frame_tmp;
                }
            }
        }
            
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

void video_proc_v2(char *argv[])
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

    // 读取石头剪刀布的bin文件数据 并且转换为mat类型
    int bu_width = 400;
    int bu_height = 400;
    cv::Mat image_bu_argb;
    Utils::bin_2_mat("bu.bin", bu_width, bu_height, image_bu_argb);

    int shitou_width = 400;
    int shitou_height = 400;
    cv::Mat image_shitou_argb;
    Utils::bin_2_mat("shitou.bin", shitou_width, shitou_height, image_shitou_argb);

    int jiandao_width = 400;
    int jiandao_height = 400;
    cv::Mat image_jiandao_argb;
    Utils::bin_2_mat("jiandao.bin", jiandao_width, jiandao_height, image_jiandao_argb);

    // 设置游戏模式
    static int MODE = atoi(argv[5]);
    int counts_guess = -1;
    int player_win = 0;
    int k230_win = 0;
    bool sleep_end = false;
    bool set_stop_id = true;
    std::vector<std::string> LIBRARY = {"fist","yeah","five"};

    HandDetection hd(argv[1], atof(argv[2]), atof(argv[3]), {SENSOR_WIDTH, SENSOR_HEIGHT}, {SENSOR_CHANNEL, SENSOR_HEIGHT, SENSOR_WIDTH}, reinterpret_cast<uintptr_t>(vaddr), reinterpret_cast<uintptr_t>(paddr), atoi(argv[6]));
    sync();
    HandKeypoint hk(argv[4], {SENSOR_CHANNEL, SENSOR_HEIGHT, SENSOR_WIDTH}, reinterpret_cast<uintptr_t>(vaddr), reinterpret_cast<uintptr_t>(paddr), atoi(argv[6]));
    sync();

    cv::Mat osd_frame_tmp(osd_height, osd_width, CV_8UC4, cv::Scalar(0, 0, 0, 0));
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

        cv::Mat osd_frame(osd_height, osd_width, CV_8UC4, cv::Scalar(0, 0, 0, 0));
        cv::Mat osd_frame_out;//(osd_height, osd_width, CV_8UC4, cv::Scalar(0, 0, 0, 0));
        cv::Mat osd_frame_vertical;
        cv::Mat osd_frame_horizontal;

        results.clear();
        hd.pre_process();
        hd.inference();
        hd.post_process(results);

        float max_area_hand = 0;
        int max_id_hand = -1;
        for (int i = 0; i < results.size(); ++i)
        {
            float area_i = (results[i].x2 - results[i].x1) * (results[i].y2 - results[i].y1);
            if (area_i > max_area_hand)
            {
                max_area_hand = area_i;
                max_id_hand = i;
            }
        }

        std::string gesture = "";
        if (max_id_hand != -1)
        {
            std::string text = hd.labels_[results[max_id_hand].label] + ":" + std::to_string(round(results[max_id_hand].score * 100) / 100.0);

            int w = results[max_id_hand].x2 - results[max_id_hand].x1 + 1;
            int h = results[max_id_hand].y2 - results[max_id_hand].y1 + 1;
            
            int rect_x = results[max_id_hand].x1/ SENSOR_WIDTH * osd_width;
            int rect_y = results[max_id_hand].y1/ SENSOR_HEIGHT * osd_height;
            int rect_w = (float)w / SENSOR_WIDTH * osd_width;
            int rect_h = (float)h / SENSOR_HEIGHT  * osd_height;
            // cv::rectangle(osd_frame, cv::Rect(rect_x, rect_y , rect_w, rect_h), cv::Scalar( 255,255, 255, 255), 2, 2, 0);
            
            int length = std::max(w,h)/2;
            int cx = (results[max_id_hand].x1+results[max_id_hand].x2)/2;
            int cy = (results[max_id_hand].y1+results[max_id_hand].y2)/2;
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
            gesture = hk.h_gesture(angle_list);

            std::string text1 = "Gesture: " + gesture;
        }


        if(MODE == 0)
        {
            {
                ScopedTiming st("osd draw", atoi(argv[6]));

                if(gesture == "fist")
                {
                    cv::Mat copy_ori_image = osd_frame(cv::Rect(20,20,jiandao_width,jiandao_height));
                    image_jiandao_argb.copyTo(copy_ori_image,image_jiandao_argb); 
                }
                else if(gesture == "five")
                {
                    cv::Mat copy_ori_image = osd_frame(cv::Rect(20,20,shitou_width,shitou_height));
                    image_shitou_argb.copyTo(copy_ori_image,image_shitou_argb); 
                }
                else if(gesture == "yeah")
                {
                    cv::Mat copy_ori_image = osd_frame(cv::Rect(20,20,bu_width,bu_height));
                    image_bu_argb.copyTo(copy_ori_image,image_bu_argb);  
                }
            }
        }
        else if (MODE == 1)
        {
            {
                ScopedTiming st("osd draw", atoi(argv[6]));

                if(gesture == "fist")
                {
                    cv::Mat copy_ori_image = osd_frame(cv::Rect(20,20,bu_width,bu_height));
                    image_bu_argb.copyTo(copy_ori_image,image_bu_argb); 
                }
                else if(gesture == "five")
                {
                    cv::Mat copy_ori_image = osd_frame(cv::Rect(20,20,jiandao_width,jiandao_height));
                    image_jiandao_argb.copyTo(copy_ori_image,image_jiandao_argb);  
                }
                else if(gesture == "yeah")
                {
                    cv::Mat copy_ori_image = osd_frame(cv::Rect(20,20,shitou_width,shitou_height));
                    image_shitou_argb.copyTo(copy_ori_image,image_shitou_argb); 
                }
            }
        }
        else
        {
            if(sleep_end)
            {
                usleep(2000000);
                sleep_end = false;
            }

            if(max_id_hand == -1)
            {
                set_stop_id = true;
            }

            if(counts_guess == -1 && gesture != "fist" && gesture != "yeah" && gesture != "five")
            {
                std::string start_txt = " G A M E   S T A R T ";
                std::string oneset_txt = std::to_string(1) + "  S E T";
                cv::putText(osd_frame, start_txt, cv::Point(200,500),cv::FONT_HERSHEY_COMPLEX, 2, cv::Scalar(255, 255, 0, 0), 4);
                cv::putText(osd_frame, oneset_txt, cv::Point(400,600),cv::FONT_HERSHEY_COMPLEX, 2, cv::Scalar(255, 0, 150, 255), 4);
            }
            else if(counts_guess == MODE)
            {
                // osd_frame = cv::Mat(osd_height, osd_width, CV_8UC4, cv::Scalar(0, 0, 0, 0));
                if(k230_win > player_win)
                {
                    cv::putText(osd_frame, "Y O U   L O S E", cv::Point(340,500),cv::FONT_HERSHEY_COMPLEX, 2, cv::Scalar(255, 255, 0, 0), 4);
                }
                else if(k230_win < player_win)
                {
                    cv::putText(osd_frame, "Y O U   W I N", cv::Point(340,500),cv::FONT_HERSHEY_COMPLEX, 2, cv::Scalar(255, 255, 0, 0), 4);
                }
                else
                {
                    cv::putText(osd_frame, "T I E   G A M E", cv::Point(340,500),cv::FONT_HERSHEY_COMPLEX, 2, cv::Scalar(255, 255, 0, 0), 4);
                }
                counts_guess = -1;
                player_win = 0;
                k230_win = 0;

                sleep_end = true;
            }
            else
            {
                if(set_stop_id)
                {
                    if(counts_guess == -1 && (gesture == "fist" || gesture == "yeah" || gesture == "five"))
                    {
                        counts_guess = 0;
                    }

                    if(counts_guess != -1 && (gesture == "fist" || gesture == "yeah" || gesture == "five"))
                    {
                        int k230_guess=rand()%3;
                        if(gesture == "fist" && LIBRARY[k230_guess] == "yeah")
                        {
                            player_win += 1;
                        }
                        else if(gesture == "fist" && LIBRARY[k230_guess] == "five")
                        {
                            k230_win += 1;
                        }
                        if(gesture == "yeah" && LIBRARY[k230_guess] == "fist")
                        {
                            k230_win += 1;
                        }
                        else if(gesture == "yeah" && LIBRARY[k230_guess] == "five")
                        {
                            player_win += 1;
                        }
                        if(gesture == "five" && LIBRARY[k230_guess] == "fist")
                        {
                            player_win += 1;
                        }
                        else if(gesture == "five" && LIBRARY[k230_guess] == "yeah")
                        {
                            k230_win += 1;
                        }

                        if(LIBRARY[k230_guess] == "fist")
                        {
                            cv::Mat copy_ori_image = osd_frame(cv::Rect(20,20,shitou_width,shitou_height));
                            image_shitou_argb.copyTo(copy_ori_image,image_shitou_argb);  
                        }
                        else if(LIBRARY[k230_guess] == "five")
                        {
                            cv::Mat copy_ori_image = osd_frame(cv::Rect(20,20,bu_width,bu_height));
                            image_bu_argb.copyTo(copy_ori_image,image_bu_argb);  
                        }
                        else if(LIBRARY[k230_guess] == "yeah")
                        {
                            cv::Mat copy_ori_image = osd_frame(cv::Rect(20,20,jiandao_width,jiandao_height));
                            image_jiandao_argb.copyTo(copy_ori_image,image_jiandao_argb);  
                        }
                        counts_guess += 1;

                        std::string set_txt = std::to_string(counts_guess) + "  S E T";
                        cv::putText(osd_frame, set_txt, cv::Point(400,600),cv::FONT_HERSHEY_COMPLEX, 2, cv::Scalar(255, 0, 150, 255), 4);
                        osd_frame_tmp = osd_frame;
                        set_stop_id = false;
                        sleep_end = true;
                    }
                    else
                    {
                        std::string set_txt = std::to_string(counts_guess + 1) + "  S E T";
                        cv::putText(osd_frame, set_txt, cv::Point(400,600),cv::FONT_HERSHEY_COMPLEX, 2, cv::Scalar(255, 0, 150, 255), 4);
                    }
                }
                else
                {
                    osd_frame = osd_frame_tmp;
                }
            }
        }
            
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

void video_proc_k230d(char *argv[])
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

    // 读取石头剪刀布的bin文件数据 并且转换为mat类型
    int bu_width = 400;
    int bu_height = 400;
    cv::Mat image_bu_argb;
    Utils::bin_2_mat("bu.bin", bu_width, bu_height, image_bu_argb);

    int shitou_width = 400;
    int shitou_height = 400;
    cv::Mat image_shitou_argb;
    Utils::bin_2_mat("shitou.bin", shitou_width, shitou_height, image_shitou_argb);

    int jiandao_width = 400;
    int jiandao_height = 400;
    cv::Mat image_jiandao_argb;
    Utils::bin_2_mat("jiandao.bin", jiandao_width, jiandao_height, image_jiandao_argb);

    // 设置游戏模式
    static int MODE = atoi(argv[5]);
    int counts_guess = -1;
    int player_win = 0;
    int k230_win = 0;
    bool sleep_end = false;
    bool set_stop_id = true;
    std::vector<std::string> LIBRARY = {"fist","yeah","five"};

    HandDetection hd(argv[1], atof(argv[2]), atof(argv[3]), {SENSOR_WIDTH, SENSOR_HEIGHT}, {SENSOR_CHANNEL, SENSOR_HEIGHT, SENSOR_WIDTH}, reinterpret_cast<uintptr_t>(vaddr), reinterpret_cast<uintptr_t>(paddr), atoi(argv[6]));
    sync();
    HandKeypoint hk(argv[4], {SENSOR_CHANNEL, SENSOR_HEIGHT, SENSOR_WIDTH}, reinterpret_cast<uintptr_t>(vaddr), reinterpret_cast<uintptr_t>(paddr), atoi(argv[6]));
    sync();

    cv::Mat osd_frame_tmp(osd_height, osd_width, CV_8UC4, cv::Scalar(0, 0, 0, 0));
    cv::rotate(osd_frame_tmp, osd_frame_tmp, cv::ROTATE_90_COUNTERCLOCKWISE);
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

        cv::Mat osd_frame(osd_height, osd_width, CV_8UC4, cv::Scalar(0, 0, 0, 0));
        cv::rotate(osd_frame, osd_frame, cv::ROTATE_90_COUNTERCLOCKWISE);
        cv::Mat osd_frame_out;//(osd_height, osd_width, CV_8UC4, cv::Scalar(0, 0, 0, 0));
        cv::Mat osd_frame_vertical;
        cv::Mat osd_frame_horizontal;

        results.clear();
        hd.pre_process();
        hd.inference();
        hd.post_process(results);

        float max_area_hand = 0;
        int max_id_hand = -1;
        for (int i = 0; i < results.size(); ++i)
        {
            float area_i = (results[i].x2 - results[i].x1) * (results[i].y2 - results[i].y1);
            if (area_i > max_area_hand)
            {
                max_area_hand = area_i;
                max_id_hand = i;
            }
        }

        std::string gesture = "";
        if (max_id_hand != -1)
        {
            std::string text = hd.labels_[results[max_id_hand].label] + ":" + std::to_string(round(results[max_id_hand].score * 100) / 100.0);

            int w = results[max_id_hand].x2 - results[max_id_hand].x1 + 1;
            int h = results[max_id_hand].y2 - results[max_id_hand].y1 + 1;
            
            int rect_x = results[max_id_hand].x1/ SENSOR_WIDTH * osd_frame.cols;
            int rect_y = results[max_id_hand].y1/ SENSOR_HEIGHT * osd_frame.rows;
            int rect_w = (float)w / SENSOR_WIDTH * osd_frame.cols;
            int rect_h = (float)h / SENSOR_HEIGHT  * osd_frame.rows;
            // cv::rectangle(osd_frame, cv::Rect(rect_x, rect_y , rect_w, rect_h), cv::Scalar( 255,255, 255, 255), 2, 2, 0);
            
            int length = std::max(w,h)/2;
            int cx = (results[max_id_hand].x1+results[max_id_hand].x2)/2;
            int cy = (results[max_id_hand].y1+results[max_id_hand].y2)/2;
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
            gesture = hk.h_gesture(angle_list);

            std::string text1 = "Gesture: " + gesture;
        }


        if(MODE == 0)
        {
            {
                ScopedTiming st("osd draw", atoi(argv[6]));

                if(gesture == "fist")
                {
                    cv::Mat copy_ori_image = osd_frame(cv::Rect(20,20,jiandao_width,jiandao_height));
                    image_jiandao_argb.copyTo(copy_ori_image,image_jiandao_argb); 
                }
                else if(gesture == "five")
                {
                    cv::Mat copy_ori_image = osd_frame(cv::Rect(20,20,shitou_width,shitou_height));
                    image_shitou_argb.copyTo(copy_ori_image,image_shitou_argb); 
                }
                else if(gesture == "yeah")
                {
                    cv::Mat copy_ori_image = osd_frame(cv::Rect(20,20,bu_width,bu_height));
                    image_bu_argb.copyTo(copy_ori_image,image_bu_argb);  
                }
            }
        }
        else if (MODE == 1)
        {
            {
                ScopedTiming st("osd draw", atoi(argv[6]));

                if(gesture == "fist")
                {
                    cv::Mat copy_ori_image = osd_frame(cv::Rect(20,20,bu_width,bu_height));
                    image_bu_argb.copyTo(copy_ori_image,image_bu_argb); 
                }
                else if(gesture == "five")
                {
                    cv::Mat copy_ori_image = osd_frame(cv::Rect(20,20,jiandao_width,jiandao_height));
                    image_jiandao_argb.copyTo(copy_ori_image,image_jiandao_argb);  
                }
                else if(gesture == "yeah")
                {
                    cv::Mat copy_ori_image = osd_frame(cv::Rect(20,20,shitou_width,shitou_height));
                    image_shitou_argb.copyTo(copy_ori_image,image_shitou_argb); 
                }
            }
        }
        else
        {
            if(sleep_end)
            {
                usleep(2000000);
                sleep_end = false;
            }

            if(max_id_hand == -1)
            {
                set_stop_id = true;
            }

            if(counts_guess == -1 && gesture != "fist" && gesture != "yeah" && gesture != "five")
            {
                std::string start_txt = " G A M E   S T A R T ";
                std::string oneset_txt = std::to_string(1) + "  S E T";
                cv::putText(osd_frame, start_txt, cv::Point(50,200),cv::FONT_HERSHEY_COMPLEX, 2, cv::Scalar(255, 255, 0, 0), 4);
                cv::putText(osd_frame, oneset_txt, cv::Point(150,300),cv::FONT_HERSHEY_COMPLEX, 2, cv::Scalar(255, 0, 150, 255), 4);
            }
            else if(counts_guess == MODE)
            {
                // osd_frame = cv::Mat(osd_height, osd_width, CV_8UC4, cv::Scalar(0, 0, 0, 0));
                if(k230_win > player_win)
                {
                    cv::putText(osd_frame, "Y O U   L O S E", cv::Point(100,200),cv::FONT_HERSHEY_COMPLEX, 2, cv::Scalar(255, 255, 0, 0), 4);
                }
                else if(k230_win < player_win)
                {
                    cv::putText(osd_frame, "Y O U   W I N", cv::Point(100,200),cv::FONT_HERSHEY_COMPLEX, 2, cv::Scalar(255, 255, 0, 0), 4);
                }
                else
                {
                    cv::putText(osd_frame, "T I E   G A M E", cv::Point(100,200),cv::FONT_HERSHEY_COMPLEX, 2, cv::Scalar(255, 255, 0, 0), 4);
                }
                counts_guess = -1;
                player_win = 0;
                k230_win = 0;

                sleep_end = true;
            }
            else
            {
                if(set_stop_id)
                {
                    if(counts_guess == -1 && (gesture == "fist" || gesture == "yeah" || gesture == "five"))
                    {
                        counts_guess = 0;
                    }

                    if(counts_guess != -1 && (gesture == "fist" || gesture == "yeah" || gesture == "five"))
                    {
                        int k230_guess=rand()%3;
                        if(gesture == "fist" && LIBRARY[k230_guess] == "yeah")
                        {
                            player_win += 1;
                        }
                        else if(gesture == "fist" && LIBRARY[k230_guess] == "five")
                        {
                            k230_win += 1;
                        }
                        if(gesture == "yeah" && LIBRARY[k230_guess] == "fist")
                        {
                            k230_win += 1;
                        }
                        else if(gesture == "yeah" && LIBRARY[k230_guess] == "five")
                        {
                            player_win += 1;
                        }
                        if(gesture == "five" && LIBRARY[k230_guess] == "fist")
                        {
                            player_win += 1;
                        }
                        else if(gesture == "five" && LIBRARY[k230_guess] == "yeah")
                        {
                            k230_win += 1;
                        }

                        if(LIBRARY[k230_guess] == "fist")
                        {
                            cv::Mat copy_ori_image = osd_frame(cv::Rect(20,20,shitou_width,shitou_height));
                            image_shitou_argb.copyTo(copy_ori_image,image_shitou_argb);  
                        }
                        else if(LIBRARY[k230_guess] == "five")
                        {
                            cv::Mat copy_ori_image = osd_frame(cv::Rect(20,20,bu_width,bu_height));
                            image_bu_argb.copyTo(copy_ori_image,image_bu_argb);  
                        }
                        else if(LIBRARY[k230_guess] == "yeah")
                        {
                            cv::Mat copy_ori_image = osd_frame(cv::Rect(20,20,jiandao_width,jiandao_height));
                            image_jiandao_argb.copyTo(copy_ori_image,image_jiandao_argb);  
                        }
                        counts_guess += 1;

                        std::string set_txt = std::to_string(counts_guess) + "  S E T";
                        cv::putText(osd_frame, set_txt, cv::Point(150,300),cv::FONT_HERSHEY_COMPLEX, 2, cv::Scalar(255, 0, 150, 255), 4);
                        osd_frame_tmp = osd_frame;
                        set_stop_id = false;
                        sleep_end = true;
                    }
                    else
                    {
                        std::string set_txt = std::to_string(counts_guess + 1) + "  S E T";
                        cv::putText(osd_frame, set_txt, cv::Point(150,300),cv::FONT_HERSHEY_COMPLEX, 2, cv::Scalar(255, 0, 150, 255), 4);
                    }
                }
                else
                {
                    osd_frame = osd_frame_tmp;
                }
            }
        }
        cv::rotate(osd_frame, osd_frame, cv::ROTATE_90_CLOCKWISE);
            
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

void video_proc_01(char *argv[])
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

    // 读取石头剪刀布的bin文件数据 并且转换为mat类型
    int bu_width = 400;
    int bu_height = 400;
    cv::Mat image_bu_argb;
    Utils::bin_2_mat("bu.bin", bu_width, bu_height, image_bu_argb);

    int shitou_width = 400;
    int shitou_height = 400;
    cv::Mat image_shitou_argb;
    Utils::bin_2_mat("shitou.bin", shitou_width, shitou_height, image_shitou_argb);

    int jiandao_width = 400;
    int jiandao_height = 400;
    cv::Mat image_jiandao_argb;
    Utils::bin_2_mat("jiandao.bin", jiandao_width, jiandao_height, image_jiandao_argb);

    // 设置游戏模式
    static int MODE = atoi(argv[5]);
    int counts_guess = -1;
    int player_win = 0;
    int k230_win = 0;
    bool sleep_end = false;
    bool set_stop_id = true;
    std::vector<std::string> LIBRARY = {"fist","yeah","five"};

    HandDetection hd(argv[1], atof(argv[2]), atof(argv[3]), {SENSOR_WIDTH, SENSOR_HEIGHT}, {SENSOR_CHANNEL, SENSOR_HEIGHT, SENSOR_WIDTH}, reinterpret_cast<uintptr_t>(vaddr), reinterpret_cast<uintptr_t>(paddr), atoi(argv[6]));
    sync();
    HandKeypoint hk(argv[4], {SENSOR_CHANNEL, SENSOR_HEIGHT, SENSOR_WIDTH}, reinterpret_cast<uintptr_t>(vaddr), reinterpret_cast<uintptr_t>(paddr), atoi(argv[6]));
    sync();

    cv::Mat osd_frame_tmp(osd_height, osd_width, CV_8UC4, cv::Scalar(0, 0, 0, 0));
    
    #if defined(STUDIO_HDMI)
    {
        osd_frame_tmp = osd_frame_tmp;
    }
    #else
    {
        cv::rotate(osd_frame_tmp, osd_frame_tmp, cv::ROTATE_90_COUNTERCLOCKWISE);
    }
    #endif
    
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

        cv::Mat osd_frame(osd_height, osd_width, CV_8UC4, cv::Scalar(0, 0, 0, 0));

        #if defined(STUDIO_HDMI)
        {
            cv::Mat osd_frame_out;//(osd_height, osd_width, CV_8UC4, cv::Scalar(0, 0, 0, 0));
            cv::Mat osd_frame_vertical;
            cv::Mat osd_frame_horizontal;

            results.clear();
            hd.pre_process();
            hd.inference();
            hd.post_process(results);

            float max_area_hand = 0;
            int max_id_hand = -1;
            for (int i = 0; i < results.size(); ++i)
            {
                float area_i = (results[i].x2 - results[i].x1) * (results[i].y2 - results[i].y1);
                if (area_i > max_area_hand)
                {
                    max_area_hand = area_i;
                    max_id_hand = i;
                }
            }

            std::string gesture = "";
            if (max_id_hand != -1)
            {
                std::string text = hd.labels_[results[max_id_hand].label] + ":" + std::to_string(round(results[max_id_hand].score * 100) / 100.0);

                int w = results[max_id_hand].x2 - results[max_id_hand].x1 + 1;
                int h = results[max_id_hand].y2 - results[max_id_hand].y1 + 1;
                
                int rect_x = results[max_id_hand].x1/ SENSOR_WIDTH * osd_frame.cols;
                int rect_y = results[max_id_hand].y1/ SENSOR_HEIGHT * osd_frame.rows;
                int rect_w = (float)w / SENSOR_WIDTH * osd_frame.cols;
                int rect_h = (float)h / SENSOR_HEIGHT  * osd_frame.rows;
                // cv::rectangle(osd_frame, cv::Rect(rect_x, rect_y , rect_w, rect_h), cv::Scalar( 255,255, 255, 255), 2, 2, 0);
                
                int length = std::max(w,h)/2;
                int cx = (results[max_id_hand].x1+results[max_id_hand].x2)/2;
                int cy = (results[max_id_hand].y1+results[max_id_hand].y2)/2;
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
                gesture = hk.h_gesture(angle_list);

                std::string text1 = "Gesture: " + gesture;
            }


            if(MODE == 0)
            {
                {
                    ScopedTiming st("osd draw", atoi(argv[6]));

                    if(gesture == "fist")
                    {
                        cv::Mat copy_ori_image = osd_frame(cv::Rect(20,20,jiandao_width,jiandao_height));
                        image_jiandao_argb.copyTo(copy_ori_image,image_jiandao_argb); 
                    }
                    else if(gesture == "five")
                    {
                        cv::Mat copy_ori_image = osd_frame(cv::Rect(20,20,shitou_width,shitou_height));
                        image_shitou_argb.copyTo(copy_ori_image,image_shitou_argb); 
                    }
                    else if(gesture == "yeah")
                    {
                        cv::Mat copy_ori_image = osd_frame(cv::Rect(20,20,bu_width,bu_height));
                        image_bu_argb.copyTo(copy_ori_image,image_bu_argb);  
                    }
                }
            }
            else if (MODE == 1)
            {
                {
                    ScopedTiming st("osd draw", atoi(argv[6]));

                    if(gesture == "fist")
                    {
                        cv::Mat copy_ori_image = osd_frame(cv::Rect(20,20,bu_width,bu_height));
                        image_bu_argb.copyTo(copy_ori_image,image_bu_argb); 
                    }
                    else if(gesture == "five")
                    {
                        cv::Mat copy_ori_image = osd_frame(cv::Rect(20,20,jiandao_width,jiandao_height));
                        image_jiandao_argb.copyTo(copy_ori_image,image_jiandao_argb);  
                    }
                    else if(gesture == "yeah")
                    {
                        cv::Mat copy_ori_image = osd_frame(cv::Rect(20,20,shitou_width,shitou_height));
                        image_shitou_argb.copyTo(copy_ori_image,image_shitou_argb); 
                    }
                }
            }
            else
            {
                if(sleep_end)
                {
                    usleep(2000000);
                    sleep_end = false;
                }

                if(max_id_hand == -1)
                {
                    set_stop_id = true;
                }

                if(counts_guess == -1 && gesture != "fist" && gesture != "yeah" && gesture != "five")
                {
                    std::string start_txt = " G A M E   S T A R T ";
                    std::string oneset_txt = std::to_string(1) + "  S E T";
                    cv::putText(osd_frame, start_txt, cv::Point(50,200),cv::FONT_HERSHEY_COMPLEX, 2, cv::Scalar(255, 255, 0, 0), 4);
                    cv::putText(osd_frame, oneset_txt, cv::Point(150,300),cv::FONT_HERSHEY_COMPLEX, 2, cv::Scalar(255, 0, 150, 255), 4);
                }
                else if(counts_guess == MODE)
                {
                    // osd_frame = cv::Mat(osd_height, osd_width, CV_8UC4, cv::Scalar(0, 0, 0, 0));
                    if(k230_win > player_win)
                    {
                        cv::putText(osd_frame, "Y O U   L O S E", cv::Point(100,200),cv::FONT_HERSHEY_COMPLEX, 2, cv::Scalar(255, 255, 0, 0), 4);
                    }
                    else if(k230_win < player_win)
                    {
                        cv::putText(osd_frame, "Y O U   W I N", cv::Point(100,200),cv::FONT_HERSHEY_COMPLEX, 2, cv::Scalar(255, 255, 0, 0), 4);
                    }
                    else
                    {
                        cv::putText(osd_frame, "T I E   G A M E", cv::Point(100,200),cv::FONT_HERSHEY_COMPLEX, 2, cv::Scalar(255, 255, 0, 0), 4);
                    }
                    counts_guess = -1;
                    player_win = 0;
                    k230_win = 0;

                    sleep_end = true;
                }
                else
                {
                    if(set_stop_id)
                    {
                        if(counts_guess == -1 && (gesture == "fist" || gesture == "yeah" || gesture == "five"))
                        {
                            counts_guess = 0;
                        }

                        if(counts_guess != -1 && (gesture == "fist" || gesture == "yeah" || gesture == "five"))
                        {
                            int k230_guess=rand()%3;
                            if(gesture == "fist" && LIBRARY[k230_guess] == "yeah")
                            {
                                player_win += 1;
                            }
                            else if(gesture == "fist" && LIBRARY[k230_guess] == "five")
                            {
                                k230_win += 1;
                            }
                            if(gesture == "yeah" && LIBRARY[k230_guess] == "fist")
                            {
                                k230_win += 1;
                            }
                            else if(gesture == "yeah" && LIBRARY[k230_guess] == "five")
                            {
                                player_win += 1;
                            }
                            if(gesture == "five" && LIBRARY[k230_guess] == "fist")
                            {
                                player_win += 1;
                            }
                            else if(gesture == "five" && LIBRARY[k230_guess] == "yeah")
                            {
                                k230_win += 1;
                            }

                            if(LIBRARY[k230_guess] == "fist")
                            {
                                cv::Mat copy_ori_image = osd_frame(cv::Rect(20,20,shitou_width,shitou_height));
                                image_shitou_argb.copyTo(copy_ori_image,image_shitou_argb);  
                            }
                            else if(LIBRARY[k230_guess] == "five")
                            {
                                cv::Mat copy_ori_image = osd_frame(cv::Rect(20,20,bu_width,bu_height));
                                image_bu_argb.copyTo(copy_ori_image,image_bu_argb);  
                            }
                            else if(LIBRARY[k230_guess] == "yeah")
                            {
                                cv::Mat copy_ori_image = osd_frame(cv::Rect(20,20,jiandao_width,jiandao_height));
                                image_jiandao_argb.copyTo(copy_ori_image,image_jiandao_argb);  
                            }
                            counts_guess += 1;

                            std::string set_txt = std::to_string(counts_guess) + "  S E T";
                            cv::putText(osd_frame, set_txt, cv::Point(150,300),cv::FONT_HERSHEY_COMPLEX, 2, cv::Scalar(255, 0, 150, 255), 4);
                            osd_frame_tmp = osd_frame;
                            set_stop_id = false;
                            sleep_end = true;
                        }
                        else
                        {
                            std::string set_txt = std::to_string(counts_guess + 1) + "  S E T";
                            cv::putText(osd_frame, set_txt, cv::Point(150,300),cv::FONT_HERSHEY_COMPLEX, 2, cv::Scalar(255, 0, 150, 255), 4);
                        }
                    }
                    else
                    {
                        osd_frame = osd_frame_tmp;
                    }
                }
            }
        }
        #else
        {
            cv::rotate(osd_frame, osd_frame, cv::ROTATE_90_COUNTERCLOCKWISE);
            cv::Mat osd_frame_out;//(osd_height, osd_width, CV_8UC4, cv::Scalar(0, 0, 0, 0));
            cv::Mat osd_frame_vertical;
            cv::Mat osd_frame_horizontal;

            results.clear();
            hd.pre_process();
            hd.inference();
            hd.post_process(results);

            float max_area_hand = 0;
            int max_id_hand = -1;
            for (int i = 0; i < results.size(); ++i)
            {
                float area_i = (results[i].x2 - results[i].x1) * (results[i].y2 - results[i].y1);
                if (area_i > max_area_hand)
                {
                    max_area_hand = area_i;
                    max_id_hand = i;
                }
            }

            std::string gesture = "";
            if (max_id_hand != -1)
            {
                std::string text = hd.labels_[results[max_id_hand].label] + ":" + std::to_string(round(results[max_id_hand].score * 100) / 100.0);

                int w = results[max_id_hand].x2 - results[max_id_hand].x1 + 1;
                int h = results[max_id_hand].y2 - results[max_id_hand].y1 + 1;
                
                int rect_x = results[max_id_hand].x1/ SENSOR_WIDTH * osd_frame.cols;
                int rect_y = results[max_id_hand].y1/ SENSOR_HEIGHT * osd_frame.rows;
                int rect_w = (float)w / SENSOR_WIDTH * osd_frame.cols;
                int rect_h = (float)h / SENSOR_HEIGHT  * osd_frame.rows;
                // cv::rectangle(osd_frame, cv::Rect(rect_x, rect_y , rect_w, rect_h), cv::Scalar( 255,255, 255, 255), 2, 2, 0);
                
                int length = std::max(w,h)/2;
                int cx = (results[max_id_hand].x1+results[max_id_hand].x2)/2;
                int cy = (results[max_id_hand].y1+results[max_id_hand].y2)/2;
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
                gesture = hk.h_gesture(angle_list);

                std::string text1 = "Gesture: " + gesture;
            }


            if(MODE == 0)
            {
                {
                    ScopedTiming st("osd draw", atoi(argv[6]));

                    if(gesture == "fist")
                    {
                        cv::Mat copy_ori_image = osd_frame(cv::Rect(20,20,jiandao_width,jiandao_height));
                        image_jiandao_argb.copyTo(copy_ori_image,image_jiandao_argb); 
                    }
                    else if(gesture == "five")
                    {
                        cv::Mat copy_ori_image = osd_frame(cv::Rect(20,20,shitou_width,shitou_height));
                        image_shitou_argb.copyTo(copy_ori_image,image_shitou_argb); 
                    }
                    else if(gesture == "yeah")
                    {
                        cv::Mat copy_ori_image = osd_frame(cv::Rect(20,20,bu_width,bu_height));
                        image_bu_argb.copyTo(copy_ori_image,image_bu_argb);  
                    }
                }
            }
            else if (MODE == 1)
            {
                {
                    ScopedTiming st("osd draw", atoi(argv[6]));

                    if(gesture == "fist")
                    {
                        cv::Mat copy_ori_image = osd_frame(cv::Rect(20,20,bu_width,bu_height));
                        image_bu_argb.copyTo(copy_ori_image,image_bu_argb); 
                    }
                    else if(gesture == "five")
                    {
                        cv::Mat copy_ori_image = osd_frame(cv::Rect(20,20,jiandao_width,jiandao_height));
                        image_jiandao_argb.copyTo(copy_ori_image,image_jiandao_argb);  
                    }
                    else if(gesture == "yeah")
                    {
                        cv::Mat copy_ori_image = osd_frame(cv::Rect(20,20,shitou_width,shitou_height));
                        image_shitou_argb.copyTo(copy_ori_image,image_shitou_argb); 
                    }
                }
            }
            else
            {
                if(sleep_end)
                {
                    usleep(2000000);
                    sleep_end = false;
                }

                if(max_id_hand == -1)
                {
                    set_stop_id = true;
                }

                if(counts_guess == -1 && gesture != "fist" && gesture != "yeah" && gesture != "five")
                {
                    std::string start_txt = " G A M E   S T A R T ";
                    std::string oneset_txt = std::to_string(1) + "  S E T";
                    cv::putText(osd_frame, start_txt, cv::Point(50,200),cv::FONT_HERSHEY_COMPLEX, 2, cv::Scalar(255, 255, 0, 0), 4);
                    cv::putText(osd_frame, oneset_txt, cv::Point(150,300),cv::FONT_HERSHEY_COMPLEX, 2, cv::Scalar(255, 0, 150, 255), 4);
                }
                else if(counts_guess == MODE)
                {
                    // osd_frame = cv::Mat(osd_height, osd_width, CV_8UC4, cv::Scalar(0, 0, 0, 0));
                    if(k230_win > player_win)
                    {
                        cv::putText(osd_frame, "Y O U   L O S E", cv::Point(100,200),cv::FONT_HERSHEY_COMPLEX, 2, cv::Scalar(255, 255, 0, 0), 4);
                    }
                    else if(k230_win < player_win)
                    {
                        cv::putText(osd_frame, "Y O U   W I N", cv::Point(100,200),cv::FONT_HERSHEY_COMPLEX, 2, cv::Scalar(255, 255, 0, 0), 4);
                    }
                    else
                    {
                        cv::putText(osd_frame, "T I E   G A M E", cv::Point(100,200),cv::FONT_HERSHEY_COMPLEX, 2, cv::Scalar(255, 255, 0, 0), 4);
                    }
                    counts_guess = -1;
                    player_win = 0;
                    k230_win = 0;

                    sleep_end = true;
                }
                else
                {
                    if(set_stop_id)
                    {
                        if(counts_guess == -1 && (gesture == "fist" || gesture == "yeah" || gesture == "five"))
                        {
                            counts_guess = 0;
                        }

                        if(counts_guess != -1 && (gesture == "fist" || gesture == "yeah" || gesture == "five"))
                        {
                            int k230_guess=rand()%3;
                            if(gesture == "fist" && LIBRARY[k230_guess] == "yeah")
                            {
                                player_win += 1;
                            }
                            else if(gesture == "fist" && LIBRARY[k230_guess] == "five")
                            {
                                k230_win += 1;
                            }
                            if(gesture == "yeah" && LIBRARY[k230_guess] == "fist")
                            {
                                k230_win += 1;
                            }
                            else if(gesture == "yeah" && LIBRARY[k230_guess] == "five")
                            {
                                player_win += 1;
                            }
                            if(gesture == "five" && LIBRARY[k230_guess] == "fist")
                            {
                                player_win += 1;
                            }
                            else if(gesture == "five" && LIBRARY[k230_guess] == "yeah")
                            {
                                k230_win += 1;
                            }

                            if(LIBRARY[k230_guess] == "fist")
                            {
                                cv::Mat copy_ori_image = osd_frame(cv::Rect(20,20,shitou_width,shitou_height));
                                image_shitou_argb.copyTo(copy_ori_image,image_shitou_argb);  
                            }
                            else if(LIBRARY[k230_guess] == "five")
                            {
                                cv::Mat copy_ori_image = osd_frame(cv::Rect(20,20,bu_width,bu_height));
                                image_bu_argb.copyTo(copy_ori_image,image_bu_argb);  
                            }
                            else if(LIBRARY[k230_guess] == "yeah")
                            {
                                cv::Mat copy_ori_image = osd_frame(cv::Rect(20,20,jiandao_width,jiandao_height));
                                image_jiandao_argb.copyTo(copy_ori_image,image_jiandao_argb);  
                            }
                            counts_guess += 1;

                            std::string set_txt = std::to_string(counts_guess) + "  S E T";
                            cv::putText(osd_frame, set_txt, cv::Point(150,300),cv::FONT_HERSHEY_COMPLEX, 2, cv::Scalar(255, 0, 150, 255), 4);
                            osd_frame_tmp = osd_frame;
                            set_stop_id = false;
                            sleep_end = true;
                        }
                        else
                        {
                            std::string set_txt = std::to_string(counts_guess + 1) + "  S E T";
                            cv::putText(osd_frame, set_txt, cv::Point(150,300),cv::FONT_HERSHEY_COMPLEX, 2, cv::Scalar(255, 0, 150, 255), 4);
                        }
                    }
                    else
                    {
                        osd_frame = osd_frame_tmp;
                    }
                }
            }
            cv::rotate(osd_frame, osd_frame, cv::ROTATE_90_CLOCKWISE);
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
    return 0;
}
