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
#include "key.h"


std::atomic<bool> isp_stop(false);

void print_usage(const char *name)
{
	cout << "Usage: " << name << "<kmodel_det> <obj_thresh> <nms_thresh> <kmodel_kp> <debug_mode>" << endl
		 << "Options:" << endl
		 << "  kmodel_det      手掌检测kmodel路径\n"
         << "  obj_thresh      手掌检测阈值\n"
         << "  nms_thresh      手掌检测非极大值抑制阈值\n"
		 << "  kmodel_kp       手势关键点检测kmodel路径\n"
		 << "  debug_mode      是否需要调试, 0、1、2分别表示不调试、简单调试、详细调试\n"
		 << "\n"
		 << endl;
}

float calculateIntDidtance(cv::Point pt1, cv::Point pt2) {
    return cv::sqrt((pt1.x - pt2.x) * (pt1.x - pt2.x) + (pt1.y - pt2.y) * (pt1.y - pt2.y));
}

void video_proc_K230D_CANMV(char *argv[])
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

    HandDetection hd(argv[1], atof(argv[2]), atof(argv[3]), {SENSOR_WIDTH, SENSOR_HEIGHT}, {SENSOR_CHANNEL, SENSOR_HEIGHT, SENSOR_WIDTH}, reinterpret_cast<uintptr_t>(vaddr), reinterpret_cast<uintptr_t>(paddr), atoi(argv[5]));
    HandKeypoint hk(argv[4], {SENSOR_CHANNEL, SENSOR_HEIGHT, SENSOR_WIDTH}, reinterpret_cast<uintptr_t>(vaddr), reinterpret_cast<uintptr_t>(paddr), atoi(argv[5]));

    std::vector<BoxInfo> results;

    std::vector<Key> keys;
    std::string letters = "QWERTYUIOPASDFGHJKLZXCVBNM";

    int character_len = 39;
    int box_w = 140 * 1.0 / 1920 * osd_height;
    int box_h = 100 * 1.0 / 1080 * osd_width;
    int startX = 215 * 1.0 / 1920 * osd_height;
    int startY = 360 * 1.0 / 1080 * osd_width;
    int margin = 10 * 1.0 / 1920 * osd_height;
    int startX2 = 290 * 1.0 / 1920 * osd_height;
    int startX3 = 365 * 1.0 / 1920 * osd_height;
    int box_w_space = 650 * 1.0 / 1920 * osd_height;
    for (int i = 0; i < letters.size(); i++) {
        if (i < 10) 
        {
            keys.push_back(Key(startX + i * box_w + i * margin, startY, box_w, box_h, std::string(1, letters[i])));
        }
        else if (i<19)
        {
            keys.push_back(Key(startX2 + (i-10) * box_w + (i-10) * margin, startY+box_h+margin, box_w, box_h, std::string(1, letters[i])));
        }
        else
        {
            keys.push_back(Key(startX3 + (i-19) * box_w + (i-19) * margin, startY+2*box_h+2*margin, box_w, box_h, std::string(1, letters[i])));
        }
    }

    keys.push_back(Key(startX3 + (26-19) * box_w + (26-19) * margin, startY+2*box_h+2*margin, box_w, box_h, "clr"));
    keys.push_back(Key(startX2, startY+3*box_h+3*margin, box_w_space, box_h, "Space"));
    keys.push_back(Key(startX2 + margin + box_w_space, startY+3*box_h+3*margin, box_w_space, box_h, "<--"));
    
    Key textBox(startX, startY-box_h-margin, 10*box_w+9*margin, box_h, " ");

    float previousClick = 0.0;

    while (!isp_stop)
    {
        ScopedTiming st("total time", 1);
        {
            ScopedTiming st("read capture", atoi(argv[5]));
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
            ScopedTiming st("isp copy", atoi(argv[5]));
            auto vbvaddr = kd_mpi_sys_mmap_cached(dump_info.v_frame.phys_addr[0], size);
            memcpy(vaddr, (void *)vbvaddr, SENSOR_HEIGHT * SENSOR_WIDTH * 3); 
            kd_mpi_sys_munmap(vbvaddr, size);
        }

        results.clear();

        hd.pre_process();
        hd.inference();
        hd.post_process(results);

        int signTipX = 0, signTipY = 0;
        int thumbTipX = 0, thumbTipY = 0;

        cv::Mat osd_frame(osd_height, osd_width, CV_8UC4, cv::Scalar(0, 0, 0, 0));
        cv::rotate(osd_frame, osd_frame, cv::ROTATE_90_COUNTERCLOCKWISE);

        cv::Point2f index_top;
        cv::Point2f thumb_top;
        float ratio;

        for (auto r: results)
        {
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

            {
                ScopedTiming st("osd draw keypoints", atoi(argv[5]));
                hk.draw_keypoints(osd_frame, bbox, false);
            }

            float *pred = hk.get_out()[0];
            int draw_x,draw_y;

            index_top.x = pred[8*2] * w_1 + x1_1;
            index_top.y = pred[8*2+1] * h_1 + y1_1;
            signTipX = static_cast<int>(index_top.x / SENSOR_WIDTH * osd_height);
            signTipY = static_cast<int>(index_top.y / SENSOR_HEIGHT * osd_width);

            thumb_top.x = pred[4*2] * w_1 + x1_1;
            thumb_top.y = pred[4*2+1] * h_1 + y1_1;
            thumbTipX = static_cast<int>(thumb_top.x / SENSOR_WIDTH * osd_height);
            thumbTipY = static_cast<int>(thumb_top.y / SENSOR_HEIGHT * osd_width);

            float dis = calculateIntDidtance(cv::Point(signTipX, signTipY), cv::Point(thumbTipX, thumbTipY));
            float dis_hand = calculateIntDidtance(cv::Point(hk.minX, hk.minY), cv::Point(hk.maxX, hk.maxY));
            ratio = dis/dis_hand;
            if (ratio < 0.25) 
            {
                int centerX = (signTipX + thumbTipX) / 2;
                int centerY = (signTipY + thumbTipY) / 2;
                cv::circle(osd_frame, cv::Point(centerX, centerY), 5, cv::Scalar(255, 0, 255, 0), cv::FILLED);
            }
        }
        {
            ScopedTiming st("osd draw keyboard", atoi(argv[5]));
            float alpha = 0.5;
            textBox.drawKey(osd_frame, (float)0.3, 0.5);
            for (auto& k : keys) 
            {
                if (k.isOver(thumbTipX, thumbTipY)) 
                {
                    alpha = 0.2;
                    if ((k.isOver(signTipX, signTipY)) && (ratio < 0.25))
                    {
                        float clickTime = cv::getTickCount();
                        if ((clickTime - previousClick) / cv::getTickFrequency() > 0.6) 
                        {
                            if (k.text_ == "<--") 
                            {
                                textBox.text_ = textBox.text_.substr(0, textBox.text_.size() - 1);
                            }
                            else if (k.text_ == "clr") 
                            {
                                textBox.text_ = "";
                            }
                            else if (textBox.text_.size() < character_len) 
                            {
                                if (k.text_ == "Space") {
                                    textBox.text_ += " ";
                                }
                                else {
                                    textBox.text_ += k.text_;
                                }
                            }
                            previousClick = clickTime;
                        }
                    }
                }
                k.drawKey(osd_frame, alpha, 0.5);
                alpha = 0.5;
            }
        }
        cv::rotate(osd_frame, osd_frame, cv::ROTATE_90_CLOCKWISE);
        {
            ScopedTiming st("osd copy", atoi(argv[5]));
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

void video_proc_CANMV_01STUDIO(char *argv[])
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

    HandDetection hd(argv[1], atof(argv[2]), atof(argv[3]), {SENSOR_WIDTH, SENSOR_HEIGHT}, {SENSOR_CHANNEL, SENSOR_HEIGHT, SENSOR_WIDTH}, reinterpret_cast<uintptr_t>(vaddr), reinterpret_cast<uintptr_t>(paddr), atoi(argv[5]));
    HandKeypoint hk(argv[4], {SENSOR_CHANNEL, SENSOR_HEIGHT, SENSOR_WIDTH}, reinterpret_cast<uintptr_t>(vaddr), reinterpret_cast<uintptr_t>(paddr), atoi(argv[5]));

    std::vector<BoxInfo> results;

    std::vector<Key> keys;
    std::string letters = "QWERTYUIOPASDFGHJKLZXCVBNM";

    int character_len = 39;
    int box_w = 140 * 1.0 / 1920 * osd_height;
    int box_h = 100 * 1.0 / 1080 * osd_width;
    int startX = 215 * 1.0 / 1920 * osd_height;
    int startY = 360 * 1.0 / 1080 * osd_width;
    int margin = 10 * 1.0 / 1920 * osd_height;
    int startX2 = 290 * 1.0 / 1920 * osd_height;
    int startX3 = 365 * 1.0 / 1920 * osd_height;
    int box_w_space = 650 * 1.0 / 1920 * osd_height;
    for (int i = 0; i < letters.size(); i++) {
        if (i < 10) 
        {
            keys.push_back(Key(startX + i * box_w + i * margin, startY, box_w, box_h, std::string(1, letters[i])));
        }
        else if (i<19)
        {
            keys.push_back(Key(startX2 + (i-10) * box_w + (i-10) * margin, startY+box_h+margin, box_w, box_h, std::string(1, letters[i])));
        }
        else
        {
            keys.push_back(Key(startX3 + (i-19) * box_w + (i-19) * margin, startY+2*box_h+2*margin, box_w, box_h, std::string(1, letters[i])));
        }
    }

    keys.push_back(Key(startX3 + (26-19) * box_w + (26-19) * margin, startY+2*box_h+2*margin, box_w, box_h, "clr"));
    keys.push_back(Key(startX2, startY+3*box_h+3*margin, box_w_space, box_h, "Space"));
    keys.push_back(Key(startX2 + margin + box_w_space, startY+3*box_h+3*margin, box_w_space, box_h, "<--"));
    
    Key textBox(startX, startY-box_h-margin, 10*box_w+9*margin, box_h, " ");

    float previousClick = 0.0;

    while (!isp_stop)
    {
        ScopedTiming st("total time", 1);
        {
            ScopedTiming st("read capture", atoi(argv[5]));
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
            ScopedTiming st("isp copy", atoi(argv[5]));
            auto vbvaddr = kd_mpi_sys_mmap_cached(dump_info.v_frame.phys_addr[0], size);
            memcpy(vaddr, (void *)vbvaddr, SENSOR_HEIGHT * SENSOR_WIDTH * 3); 
            kd_mpi_sys_munmap(vbvaddr, size);
        }

        results.clear();

        hd.pre_process();
        hd.inference();
        hd.post_process(results);

        int signTipX = 0, signTipY = 0;
        int thumbTipX = 0, thumbTipY = 0;

        cv::Mat osd_frame(osd_height, osd_width, CV_8UC4, cv::Scalar(0, 0, 0, 0));
        cv::rotate(osd_frame, osd_frame, cv::ROTATE_90_COUNTERCLOCKWISE);

        cv::Point2f index_top;
        cv::Point2f thumb_top;
        float ratio;

        for (auto r: results)
        {
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

            {
                ScopedTiming st("osd draw keypoints", atoi(argv[5]));
                hk.draw_keypoints(osd_frame, bbox, false);
            }

            float *pred = hk.get_out()[0];
            int draw_x,draw_y;

            index_top.x = pred[8*2] * w_1 + x1_1;
            index_top.y = pred[8*2+1] * h_1 + y1_1;
            signTipX = static_cast<int>(index_top.x / SENSOR_WIDTH * osd_height);
            signTipY = static_cast<int>(index_top.y / SENSOR_HEIGHT * osd_width);

            thumb_top.x = pred[4*2] * w_1 + x1_1;
            thumb_top.y = pred[4*2+1] * h_1 + y1_1;
            thumbTipX = static_cast<int>(thumb_top.x / SENSOR_WIDTH * osd_height);
            thumbTipY = static_cast<int>(thumb_top.y / SENSOR_HEIGHT * osd_width);

            float dis = calculateIntDidtance(cv::Point(signTipX, signTipY), cv::Point(thumbTipX, thumbTipY));
            float dis_hand = calculateIntDidtance(cv::Point(hk.minX, hk.minY), cv::Point(hk.maxX, hk.maxY));
            ratio = dis/dis_hand;
            if (ratio < 0.25) 
            {
                int centerX = (signTipX + thumbTipX) / 2;
                int centerY = (signTipY + thumbTipY) / 2;
                cv::circle(osd_frame, cv::Point(centerX, centerY), 5, cv::Scalar(255, 0, 255, 0), cv::FILLED);
            }
        }
        {
            ScopedTiming st("osd draw keyboard", atoi(argv[5]));
            float alpha = 0.5;
            textBox.drawKey(osd_frame, (float)0.3, 0.5);
            for (auto& k : keys) 
            {
                if (k.isOver(thumbTipX, thumbTipY)) 
                {
                    alpha = 0.2;
                    if ((k.isOver(signTipX, signTipY)) && (ratio < 0.25))
                    {
                        float clickTime = cv::getTickCount();
                        if ((clickTime - previousClick) / cv::getTickFrequency() > 0.6) 
                        {
                            if (k.text_ == "<--") 
                            {
                                textBox.text_ = textBox.text_.substr(0, textBox.text_.size() - 1);
                            }
                            else if (k.text_ == "clr") 
                            {
                                textBox.text_ = "";
                            }
                            else if (textBox.text_.size() < character_len) 
                            {
                                if (k.text_ == "Space") {
                                    textBox.text_ += " ";
                                }
                                else {
                                    textBox.text_ += k.text_;
                                }
                            }
                            previousClick = clickTime;
                        }
                    }
                }
                k.drawKey(osd_frame, alpha, 0.5);
                alpha = 0.5;
            }
        }
        cv::rotate(osd_frame, osd_frame, cv::ROTATE_90_CLOCKWISE);
        {
            ScopedTiming st("osd copy", atoi(argv[5]));
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

    HandDetection hd(argv[1], atof(argv[2]), atof(argv[3]), {SENSOR_WIDTH, SENSOR_HEIGHT}, {SENSOR_CHANNEL, SENSOR_HEIGHT, SENSOR_WIDTH}, reinterpret_cast<uintptr_t>(vaddr), reinterpret_cast<uintptr_t>(paddr), atoi(argv[5]));
    HandKeypoint hk(argv[4], {SENSOR_CHANNEL, SENSOR_HEIGHT, SENSOR_WIDTH}, reinterpret_cast<uintptr_t>(vaddr), reinterpret_cast<uintptr_t>(paddr), atoi(argv[5]));

    
    std::vector<BoxInfo> results;

    std::vector<Key> keys;
    std::string letters = "QWERTYUIOPASDFGHJKLZXCVBNM";

    int character_len = 39;
    int box_w = 140;
    int box_h = 100;
    int startX = 215;
    int startY = 360;
    int margin = 10;
    for (int i = 0; i < letters.size(); i++) {
        if (i < 10) 
        {
            keys.push_back(Key(startX + i * box_w + i * margin, startY, box_w, box_h, std::string(1, letters[i])));
        }
        else if (i<19)
        {
            keys.push_back(Key(startX + (i-10) * box_w + (i-10) * margin + 75, startY+box_h+margin, box_w, box_h, std::string(1, letters[i])));
        }
        else
        {
            keys.push_back(Key(startX + (i-19) * box_w + (i-19) * margin + 150, startY+2*box_h+2*margin, box_w, box_h, std::string(1, letters[i])));
        }
    }

    keys.push_back(Key(startX + (26-19) * box_w + (26-19) * margin + 150, startY+2*box_h+2*margin, box_w, box_h, "clr"));
    keys.push_back(Key(startX + 90, startY+3*box_h+3*margin, 650, box_h, "Space"));
    keys.push_back(Key(startX + 100 + 650, startY+3*box_h+3*margin, 650, box_h, "<--"));
    
    Key textBox(startX, startY-box_h-margin, 10*box_w+9*margin, box_h, " ");

    float previousClick = 0.0;

    while (!isp_stop)
    {
        ScopedTiming st("total time", 1);
        {
            ScopedTiming st("read capture", atoi(argv[5]));
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
            ScopedTiming st("isp copy", atoi(argv[5]));
            auto vbvaddr = kd_mpi_sys_mmap_cached(dump_info.v_frame.phys_addr[0], size);
            memcpy(vaddr, (void *)vbvaddr, SENSOR_HEIGHT * SENSOR_WIDTH * 3); 
            kd_mpi_sys_munmap(vbvaddr, size);
        }

        results.clear();

        hd.pre_process();
        hd.inference();
        hd.post_process(results);

        int signTipX = 0, signTipY = 0;
        int thumbTipX = 0, thumbTipY = 0;

        cv::Mat osd_frame(osd_height, osd_width, CV_8UC4, cv::Scalar(0, 0, 0, 0));

        cv::Point2f index_top;
        cv::Point2f thumb_top;
        float ratio;

        for (auto r: results)
        {
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

            {
                ScopedTiming st("osd draw keypoints", atoi(argv[5]));
                hk.draw_keypoints(osd_frame, bbox, false);
            }

            float *pred = hk.get_out()[0];
            int draw_x,draw_y;

            index_top.x = pred[8*2] * w_1 + x1_1;
            index_top.y = pred[8*2+1] * h_1 + y1_1;
            signTipX = static_cast<int>(index_top.x / SENSOR_WIDTH * osd_width);
            signTipY = static_cast<int>(index_top.y / SENSOR_HEIGHT * osd_height);

            thumb_top.x = pred[4*2] * w_1 + x1_1;
            thumb_top.y = pred[4*2+1] * h_1 + y1_1;
            thumbTipX = static_cast<int>(thumb_top.x / SENSOR_WIDTH * osd_width);
            thumbTipY = static_cast<int>(thumb_top.y / SENSOR_HEIGHT * osd_height);

            float dis = calculateIntDidtance(cv::Point(signTipX, signTipY), cv::Point(thumbTipX, thumbTipY));
            float dis_hand = calculateIntDidtance(cv::Point(hk.minX, hk.minY), cv::Point(hk.maxX, hk.maxY));
            ratio = dis/dis_hand;
            if (ratio < 0.25) 
            {
                int centerX = (signTipX + thumbTipX) / 2;
                int centerY = (signTipY + thumbTipY) / 2;
                cv::circle(osd_frame, cv::Point(centerX, centerY), 5, cv::Scalar(255, 0, 255, 0), cv::FILLED);
            }
        }
        {
            ScopedTiming st("osd draw keyboard", atoi(argv[5]));
            float alpha = 0.5;
            textBox.drawKey(osd_frame, (float)0.3, 2);
            for (auto& k : keys) 
            {
                if (k.isOver(thumbTipX, thumbTipY)) 
                {
                    alpha = 0.2;
                    if ((k.isOver(signTipX, signTipY)) && (ratio < 0.25))
                    {
                        float clickTime = cv::getTickCount();
                        if ((clickTime - previousClick) / cv::getTickFrequency() > 0.6) 
                        {
                            if (k.text_ == "<--") 
                            {
                                textBox.text_ = textBox.text_.substr(0, textBox.text_.size() - 1);
                            }
                            else if (k.text_ == "clr") 
                            {
                                textBox.text_ = "";
                            }
                            else if (textBox.text_.size() < character_len) 
                            {
                                if (k.text_ == "Space") {
                                    textBox.text_ += " ";
                                }
                                else {
                                    textBox.text_ += k.text_;
                                }
                            }
                            previousClick = clickTime;
                        }
                    }
                }
                k.drawKey(osd_frame, alpha, 2);
                alpha = 0.5;
            }
        }
        {
            ScopedTiming st("osd copy", atoi(argv[5]));
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
    if (argc != 6)
    {
        print_usage(argv[0]);
        return -1;
    }
    {
        #if defined(CONFIG_BOARD_K230D_CANMV)
            std::thread thread_isp(video_proc_K230D_CANMV, argv);
        #elif defined(CONFIG_BOARD_K230_CANMV_01STUDIO)
        
            #if defined(STUDIO_HDMI)
                std::thread thread_isp(video_proc, argv);
            #else
                std::thread thread_isp(video_proc_CANMV_01STUDIO, argv);
            #endif  
        #else
            std::thread thread_isp(video_proc, argv);
        #endif
        
        while (getchar() != 'q')
        {
            usleep(10000);
        }

        isp_stop = true;
        thread_isp.join();
    }
    return 0;
}
