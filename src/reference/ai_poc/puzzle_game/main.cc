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
	cout << "Usage: " << name << "<kmodel_det> <obj_thresh> <nms_thresh> <kmodel_kp> <bin_file> <level> <debug_mode>" << endl
		 << "Options:" << endl
		 << "  kmodel_det      手掌检测kmodel路径\n"
         << "  obj_thresh      手掌检测阈值\n"
         << "  nms_thresh      手掌检测非极大值抑制阈值\n"
		 << "  kmodel_kp       手势关键点检测kmodel路径\n"
         << "  bin_file        拼图文件 (文件名 或者 None(表示排序数字拼图模式))\n"
         << "  level           拼图游戏难度\n"
		 << "  debug_mode      是否需要调试,0、1、2分别表示不调试、简单调试、详细调试\n"
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

    int puzzle_width;
    int puzzle_height;
    int puzzle_ori_width;
    int puzzle_ori_height;
    #if defined(CONFIG_BOARD_K230_CANMV) || defined(CONFIG_BOARD_K230_CANMV_V2)
    puzzle_width = osd_height;
    puzzle_height = osd_height;
    puzzle_ori_width = osd_width-puzzle_height-5;
    puzzle_ori_height = osd_width-puzzle_height-5;
    #else
    puzzle_width = osd_width;
    puzzle_height = osd_width;
    puzzle_ori_width = osd_height-puzzle_height-5;
    puzzle_ori_height = osd_height-puzzle_height-5;
    #endif


    int level = atoi(argv[6]);
    int every_block_width = puzzle_width/level;
    int every_block_height = puzzle_height/level;
    float ratio_num = every_block_width/360.0;
    int blank_x = 0;
    int blank_y = 0;
    std::vector<int> direction_vec = {-1,1,-1,1};

    cv::Mat image_puzzle_argb;
    cv::Mat image_puzzle_ori;
    cv::Mat osd_frame_tmp(osd_height, osd_width, CV_8UC4, cv::Scalar(0, 0, 0, 0));

    int exact_division_x = 0;
    int exact_division_y = 0;
    int distance_tow_points = osd_width;
    int distance_thred = every_block_width*0.3;
    cv::Rect move_rect;
    cv::Mat move_mat;
    cv::Mat copy_blank;
    cv::Mat copy_move;

    if (strcmp(argv[5], "None") == 0)
    {
        image_puzzle_argb = cv::Mat(puzzle_height, puzzle_width, CV_8UC4, cv::Scalar(200, 130, 150, 100));
        for(int i = 0; i < level*level; i++)
        {
            cv::rectangle(image_puzzle_argb, cv::Rect((i%level)*every_block_width, (i/level)*every_block_height, every_block_width, every_block_height), cv::Scalar(255, 0, 0, 0), 5);
            std::string classString = to_string(i);
            cv::Size textSize = cv::getTextSize(classString, cv::FONT_HERSHEY_DUPLEX, 7*ratio_num, 8*ratio_num, 0);
            cv::putText(image_puzzle_argb, classString, cv::Point((i%level)*every_block_width + (every_block_width-textSize.width)/2,(i/level)*every_block_height + (every_block_height+textSize.height)/2),cv::FONT_HERSHEY_COMPLEX, 7*ratio_num, cv::Scalar(255, 0, 0, 255), 8*ratio_num, 0);
        }
    }
    else
    {
        Utils::bin_2_mat(argv[5], puzzle_width, puzzle_height, image_puzzle_argb);
        for(int i = 0; i < level*level; i++)
        {
            cv::rectangle(image_puzzle_argb, cv::Rect((i%level)*every_block_width, (i/level)*every_block_height, every_block_width, every_block_height), cv::Scalar(255, 0, 0, 0), 5);
        }
    }

    cv::Mat blank_block_puzzle(every_block_height, every_block_width, CV_8UC4, cv::Scalar(220, 114, 114, 114));
    cv::Mat image_puzzle_argb_blank = image_puzzle_argb(cv::Rect(0,0,every_block_width,every_block_height));
    blank_block_puzzle.copyTo(image_puzzle_argb_blank);

    cv::resize(image_puzzle_argb, image_puzzle_ori, cv::Size(puzzle_ori_width, puzzle_ori_height), cv::INTER_AREA);
    cv::Mat copy_ori_image_0 = osd_frame_tmp(cv::Rect(0,0,puzzle_width,puzzle_height));
    image_puzzle_argb.copyTo(copy_ori_image_0);
    cv::Mat copy_ori_image_1;
    #if defined(CONFIG_BOARD_K230_CANMV) || defined(CONFIG_BOARD_K230_CANMV_V2)
    copy_ori_image_1 = osd_frame_tmp(cv::Rect(puzzle_width+2,(1080-puzzle_ori_height)/2,puzzle_ori_width,puzzle_ori_height));
    image_puzzle_ori.copyTo(copy_ori_image_1);
    #else
    copy_ori_image_1 = osd_frame_tmp(cv::Rect((1080-puzzle_ori_width)/2,puzzle_height+2,puzzle_ori_width,puzzle_ori_height));
    image_puzzle_ori.copyTo(copy_ori_image_1);
    #endif
    cv::Mat blank_block(every_block_height, every_block_width, CV_8UC4, cv::Scalar(220, 114, 114, 114));

    
    srand((unsigned)time(NULL));
    for(int i = 0; i < level*10; i++)
    {
        int k230_random = rand() % 4;
        int blank_x_tmp = blank_x;
        int blank_y_tmp = blank_y;
        if (k230_random < 2)
        {
            blank_x_tmp = blank_x + direction_vec[k230_random];
        }
        else
        {
            blank_y_tmp = blank_y + direction_vec[k230_random];
        }

        if((blank_x_tmp >= 0 && blank_x_tmp < level) && (blank_y_tmp >= 0 && blank_y_tmp < level) && (std::abs(blank_x - blank_x_tmp) <= 1 && std::abs(blank_y - blank_y_tmp) <= 1))
        {
            move_rect = cv::Rect(blank_x_tmp*every_block_width,blank_y_tmp*every_block_height,every_block_width,every_block_height);
            move_mat = osd_frame_tmp(move_rect);

            copy_blank = osd_frame_tmp(cv::Rect(blank_x_tmp*every_block_width,blank_y_tmp*every_block_height,every_block_width,every_block_height));
            copy_move = osd_frame_tmp(cv::Rect(blank_x*every_block_width,blank_y*every_block_height,every_block_width,every_block_height));
            move_mat.copyTo(copy_move);
            blank_block.copyTo(copy_blank);

            blank_x = blank_x_tmp;
            blank_y = blank_y_tmp;
        }
    }

    HandDetection hd(argv[1], atof(argv[2]), atof(argv[3]), {SENSOR_WIDTH, SENSOR_HEIGHT}, {SENSOR_CHANNEL, SENSOR_HEIGHT, SENSOR_WIDTH}, reinterpret_cast<uintptr_t>(vaddr), reinterpret_cast<uintptr_t>(paddr), atoi(argv[7]));
    sync();
    HandKeypoint hk(argv[4], {SENSOR_CHANNEL, SENSOR_HEIGHT, SENSOR_WIDTH}, reinterpret_cast<uintptr_t>(vaddr), reinterpret_cast<uintptr_t>(paddr), atoi(argv[7]));
    sync();

    std::vector<BoxInfo> results;
    std::vector<int> two_point;

    while (!isp_stop)
    {
        ScopedTiming st("total time", 1);

        {
            ScopedTiming st("read capture", atoi(argv[7]));
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
            ScopedTiming st("isp copy", atoi(argv[7]));
            auto vbvaddr = kd_mpi_sys_mmap_cached(dump_info.v_frame.phys_addr[0], size);
            memcpy(vaddr, (void *)vbvaddr, SENSOR_HEIGHT * SENSOR_WIDTH * 3);  
            kd_mpi_sys_munmap(vbvaddr, size);
        }

        results.clear();
        two_point.clear();

        hd.pre_process();
        hd.inference();
        hd.post_process(results);

        cv::Mat osd_frame(osd_height, osd_width, CV_8UC4, cv::Scalar(0, 0, 0, 0));
        cv::Mat osd_frame_vertical;

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

        if (max_id_hand != -1)
        {
            std::string text = hd.labels_[results[max_id_hand].label] + ":" + std::to_string(round(results[max_id_hand].score * 100) / 100.0);

            int w = results[max_id_hand].x2 - results[max_id_hand].x1 + 1;
            int h = results[max_id_hand].y2 - results[max_id_hand].y1 + 1;
            
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

            {
                ScopedTiming st("osd draw", atoi(argv[7]));
                hk.draw_keypoints(osd_frame, text, bbox, false, two_point);
            }
        }

        if(max_id_hand != -1 && two_point[1] <= SENSOR_WIDTH)
        {
            distance_tow_points = std::sqrt(std::pow((two_point[0] - two_point[2]),2) + std::pow((two_point[1] - two_point[3]),2))* 1.0 / SENSOR_WIDTH * osd_width;
            exact_division_x = (two_point[0] * 1.0 / SENSOR_WIDTH * osd_width)/every_block_width;
            exact_division_y = (two_point[1] * 1.0 / SENSOR_HEIGHT * osd_height)/every_block_height;
            if(distance_tow_points < distance_thred && exact_division_x >= 0 && exact_division_x < level && exact_division_y >= 0 && exact_division_y < level)
            {   
                if(std::abs(blank_x - exact_division_x) == 1 && std::abs(blank_y - exact_division_y) == 0)
                {
                    move_rect = cv::Rect(exact_division_x*every_block_width,exact_division_y*every_block_height,every_block_width,every_block_height);
                    move_mat = osd_frame_tmp(move_rect);

                    copy_blank = osd_frame_tmp(cv::Rect(exact_division_x*every_block_width,exact_division_y*every_block_height,every_block_width,every_block_height));
                    copy_move = osd_frame_tmp(cv::Rect(blank_x*every_block_width,blank_y*every_block_height,every_block_width,every_block_height));
                    move_mat.copyTo(copy_move);
                    blank_block.copyTo(copy_blank);

                    blank_x = exact_division_x;
                }
                else if (std::abs(blank_y - exact_division_y) == 1 && std::abs(blank_x - exact_division_x) == 0)
                {
                    move_rect = cv::Rect(exact_division_x*every_block_width,exact_division_y*every_block_height,every_block_width,every_block_height);
                    move_mat = osd_frame_tmp(move_rect);

                    copy_blank = osd_frame_tmp(cv::Rect(exact_division_x*every_block_width,exact_division_y*every_block_height,every_block_width,every_block_height));
                    copy_move = osd_frame_tmp(cv::Rect(blank_x*every_block_width,blank_y*every_block_height,every_block_width,every_block_height));
                    move_mat.copyTo(copy_move);
                    blank_block.copyTo(copy_blank);

                    blank_y = exact_division_y;
                }

                osd_frame = osd_frame_tmp.clone();
                if(two_point.size() > 0)
                {
                    int x1 = two_point[0] * 1.0 / SENSOR_WIDTH * osd_width;
                    int y1 = two_point[1] * 1.0 / SENSOR_HEIGHT * osd_height;
                    cv::circle(osd_frame, cv::Point(x1, y1), 10, cv::Scalar(255, 0, 255, 255), 10);
                }
            }
            else
            {
                osd_frame = osd_frame_tmp.clone();
                if(two_point.size() > 0)
                {
                    int x1 = two_point[0] * 1.0 / SENSOR_WIDTH * osd_width;
                    int y1 = two_point[1] * 1.0 / SENSOR_HEIGHT * osd_height;
                    cv::circle(osd_frame, cv::Point(x1, y1), 10, cv::Scalar(255, 255, 255, 0), 10);
                }
            }
            sync();
        }
        else
        {
            osd_frame = osd_frame_tmp.clone();
        }

        {
            ScopedTiming st("osd copy", atoi(argv[7]));
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

    int puzzle_width;
    int puzzle_height;
    int puzzle_ori_width;
    int puzzle_ori_height;
    #if defined(CONFIG_BOARD_K230_CANMV) || defined(CONFIG_BOARD_K230_CANMV_V2)
    puzzle_width = osd_height;
    puzzle_height = osd_height;
    puzzle_ori_width = osd_width-puzzle_height-5;
    puzzle_ori_height = osd_width-puzzle_height-5;
    #else
    puzzle_width = osd_width;
    puzzle_height = osd_width;
    puzzle_ori_width = osd_height-puzzle_height-5;
    puzzle_ori_height = osd_height-puzzle_height-5;
    #endif


    int level = atoi(argv[6]);
    int every_block_width = puzzle_width/level;
    int every_block_height = puzzle_height/level;
    float ratio_num = every_block_width/360.0;
    int blank_x = 0;
    int blank_y = 0;
    std::vector<int> direction_vec = {-1,1,-1,1};

    cv::Mat image_puzzle_argb;
    cv::Mat image_puzzle_ori;
    cv::Mat osd_frame_tmp(osd_height, osd_width, CV_8UC4, cv::Scalar(0, 0, 0, 0));

    int exact_division_x = 0;
    int exact_division_y = 0;
    int distance_tow_points = osd_width;
    int distance_thred = every_block_width*0.3;
    cv::Rect move_rect;
    cv::Mat move_mat;
    cv::Mat copy_blank;
    cv::Mat copy_move;

    if (strcmp(argv[5], "None") == 0)
    {
        image_puzzle_argb = cv::Mat(puzzle_height, puzzle_width, CV_8UC4, cv::Scalar(200, 130, 150, 100));
        for(int i = 0; i < level*level; i++)
        {
            cv::rectangle(image_puzzle_argb, cv::Rect((i%level)*every_block_width, (i/level)*every_block_height, every_block_width, every_block_height), cv::Scalar(255, 0, 0, 0), 5);
            std::string classString = to_string(i);
            cv::Size textSize = cv::getTextSize(classString, cv::FONT_HERSHEY_DUPLEX, 7*ratio_num, 8*ratio_num, 0);
            cv::putText(image_puzzle_argb, classString, cv::Point((i%level)*every_block_width + (every_block_width-textSize.width)/2,(i/level)*every_block_height + (every_block_height+textSize.height)/2),cv::FONT_HERSHEY_COMPLEX, 7*ratio_num, cv::Scalar(255, 0, 0, 255), 8*ratio_num, 0);
        }
    }
    else
    {
        Utils::bin_2_mat(argv[5], puzzle_width, puzzle_height, image_puzzle_argb);
        for(int i = 0; i < level*level; i++)
        {
            cv::rectangle(image_puzzle_argb, cv::Rect((i%level)*every_block_width, (i/level)*every_block_height, every_block_width, every_block_height), cv::Scalar(255, 0, 0, 0), 5);
        }
    }

    cv::Mat blank_block_puzzle(every_block_height, every_block_width, CV_8UC4, cv::Scalar(220, 114, 114, 114));
    cv::Mat image_puzzle_argb_blank = image_puzzle_argb(cv::Rect(0,0,every_block_width,every_block_height));
    blank_block_puzzle.copyTo(image_puzzle_argb_blank);

    cv::resize(image_puzzle_argb, image_puzzle_ori, cv::Size(puzzle_ori_width, puzzle_ori_height), cv::INTER_AREA);
    cv::Mat copy_ori_image_0 = osd_frame_tmp(cv::Rect(0,0,puzzle_width,puzzle_height));
    image_puzzle_argb.copyTo(copy_ori_image_0);
    cv::Mat copy_ori_image_1;
    #if defined(CONFIG_BOARD_K230_CANMV) || defined(CONFIG_BOARD_K230_CANMV_V2)
    copy_ori_image_1 = osd_frame_tmp(cv::Rect(puzzle_width+2,(1080-puzzle_ori_height)/2,puzzle_ori_width,puzzle_ori_height));
    image_puzzle_ori.copyTo(copy_ori_image_1);
    #else
    copy_ori_image_1 = osd_frame_tmp(cv::Rect((1080-puzzle_ori_width)/2,puzzle_height+2,puzzle_ori_width,puzzle_ori_height));
    image_puzzle_ori.copyTo(copy_ori_image_1);
    #endif
    cv::Mat blank_block(every_block_height, every_block_width, CV_8UC4, cv::Scalar(220, 114, 114, 114));

    
    srand((unsigned)time(NULL));
    for(int i = 0; i < level*10; i++)
    {
        int k230_random = rand() % 4;
        int blank_x_tmp = blank_x;
        int blank_y_tmp = blank_y;
        if (k230_random < 2)
        {
            blank_x_tmp = blank_x + direction_vec[k230_random];
        }
        else
        {
            blank_y_tmp = blank_y + direction_vec[k230_random];
        }

        if((blank_x_tmp >= 0 && blank_x_tmp < level) && (blank_y_tmp >= 0 && blank_y_tmp < level) && (std::abs(blank_x - blank_x_tmp) <= 1 && std::abs(blank_y - blank_y_tmp) <= 1))
        {
            move_rect = cv::Rect(blank_x_tmp*every_block_width,blank_y_tmp*every_block_height,every_block_width,every_block_height);
            move_mat = osd_frame_tmp(move_rect);

            copy_blank = osd_frame_tmp(cv::Rect(blank_x_tmp*every_block_width,blank_y_tmp*every_block_height,every_block_width,every_block_height));
            copy_move = osd_frame_tmp(cv::Rect(blank_x*every_block_width,blank_y*every_block_height,every_block_width,every_block_height));
            move_mat.copyTo(copy_move);
            blank_block.copyTo(copy_blank);

            blank_x = blank_x_tmp;
            blank_y = blank_y_tmp;
        }
    }

    HandDetection hd(argv[1], atof(argv[2]), atof(argv[3]), {SENSOR_WIDTH, SENSOR_HEIGHT}, {SENSOR_CHANNEL, SENSOR_HEIGHT, SENSOR_WIDTH}, reinterpret_cast<uintptr_t>(vaddr), reinterpret_cast<uintptr_t>(paddr), atoi(argv[7]));
    sync();
    HandKeypoint hk(argv[4], {SENSOR_CHANNEL, SENSOR_HEIGHT, SENSOR_WIDTH}, reinterpret_cast<uintptr_t>(vaddr), reinterpret_cast<uintptr_t>(paddr), atoi(argv[7]));
    sync();

    std::vector<BoxInfo> results;
    std::vector<int> two_point;

    while (!isp_stop)
    {
        ScopedTiming st("total time", 1);

        {
            ScopedTiming st("read capture", atoi(argv[7]));
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
            ScopedTiming st("isp copy", atoi(argv[7]));
            auto vbvaddr = kd_mpi_sys_mmap_cached(dump_info.v_frame.phys_addr[0], size);
            memcpy(vaddr, (void *)vbvaddr, SENSOR_HEIGHT * SENSOR_WIDTH * 3);  
            kd_mpi_sys_munmap(vbvaddr, size);
        }

        results.clear();
        two_point.clear();

        hd.pre_process();
        hd.inference();
        hd.post_process(results);

        cv::Mat osd_frame(osd_height, osd_width, CV_8UC4, cv::Scalar(0, 0, 0, 0));
        cv::Mat osd_frame_vertical;

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

        if (max_id_hand != -1)
        {
            std::string text = hd.labels_[results[max_id_hand].label] + ":" + std::to_string(round(results[max_id_hand].score * 100) / 100.0);

            int w = results[max_id_hand].x2 - results[max_id_hand].x1 + 1;
            int h = results[max_id_hand].y2 - results[max_id_hand].y1 + 1;
            
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

            {
                ScopedTiming st("osd draw", atoi(argv[7]));
                hk.draw_keypoints(osd_frame, text, bbox, false, two_point);
            }
        }

        if(max_id_hand != -1 && two_point[1] <= SENSOR_WIDTH)
        {
            distance_tow_points = std::sqrt(std::pow((two_point[0] - two_point[2]),2) + std::pow((two_point[1] - two_point[3]),2))* 1.0 / SENSOR_WIDTH * osd_width;
            exact_division_x = (two_point[0] * 1.0 / SENSOR_WIDTH * osd_width)/every_block_width;
            exact_division_y = (two_point[1] * 1.0 / SENSOR_HEIGHT * osd_height)/every_block_height;
            if(distance_tow_points < distance_thred && exact_division_x >= 0 && exact_division_x < level && exact_division_y >= 0 && exact_division_y < level)
            {   
                if(std::abs(blank_x - exact_division_x) == 1 && std::abs(blank_y - exact_division_y) == 0)
                {
                    move_rect = cv::Rect(exact_division_x*every_block_width,exact_division_y*every_block_height,every_block_width,every_block_height);
                    move_mat = osd_frame_tmp(move_rect);

                    copy_blank = osd_frame_tmp(cv::Rect(exact_division_x*every_block_width,exact_division_y*every_block_height,every_block_width,every_block_height));
                    copy_move = osd_frame_tmp(cv::Rect(blank_x*every_block_width,blank_y*every_block_height,every_block_width,every_block_height));
                    move_mat.copyTo(copy_move);
                    blank_block.copyTo(copy_blank);

                    blank_x = exact_division_x;
                }
                else if (std::abs(blank_y - exact_division_y) == 1 && std::abs(blank_x - exact_division_x) == 0)
                {
                    move_rect = cv::Rect(exact_division_x*every_block_width,exact_division_y*every_block_height,every_block_width,every_block_height);
                    move_mat = osd_frame_tmp(move_rect);

                    copy_blank = osd_frame_tmp(cv::Rect(exact_division_x*every_block_width,exact_division_y*every_block_height,every_block_width,every_block_height));
                    copy_move = osd_frame_tmp(cv::Rect(blank_x*every_block_width,blank_y*every_block_height,every_block_width,every_block_height));
                    move_mat.copyTo(copy_move);
                    blank_block.copyTo(copy_blank);

                    blank_y = exact_division_y;
                }

                osd_frame = osd_frame_tmp.clone();
                if(two_point.size() > 0)
                {
                    int x1 = two_point[0] * 1.0 / SENSOR_WIDTH * osd_width;
                    int y1 = two_point[1] * 1.0 / SENSOR_HEIGHT * osd_height;
                    cv::circle(osd_frame, cv::Point(x1, y1), 10, cv::Scalar(255, 0, 255, 255), 10);
                }
            }
            else
            {
                osd_frame = osd_frame_tmp.clone();
                if(two_point.size() > 0)
                {
                    int x1 = two_point[0] * 1.0 / SENSOR_WIDTH * osd_width;
                    int y1 = two_point[1] * 1.0 / SENSOR_HEIGHT * osd_height;
                    cv::circle(osd_frame, cv::Point(x1, y1), 10, cv::Scalar(255, 255, 255, 0), 10);
                }
            }
            sync();
        }
        else
        {
            osd_frame = osd_frame_tmp.clone();
        }

        {
            ScopedTiming st("osd copy", atoi(argv[7]));
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

    int puzzle_width;
    int puzzle_height;
    int puzzle_ori_width;
    int puzzle_ori_height;
    puzzle_width = osd_width;
    puzzle_height = osd_width;
    puzzle_ori_width = osd_height-puzzle_height-5;
    puzzle_ori_height = osd_height-puzzle_height-5;


    int level = atoi(argv[6]);
    int every_block_width = puzzle_width/level;
    int every_block_height = puzzle_height/level;
    float ratio_num = every_block_width/360.0;
    int blank_x = 0;
    int blank_y = 0;
    std::vector<int> direction_vec = {-1,1,-1,1};

    cv::Mat image_puzzle_argb;
    cv::Mat image_puzzle_ori;
    cv::Mat osd_frame_tmp(osd_height, osd_width, CV_8UC4, cv::Scalar(0, 0, 0, 0));
    cv::rotate(osd_frame_tmp, osd_frame_tmp, cv::ROTATE_90_COUNTERCLOCKWISE);

    int exact_division_x = 0;
    int exact_division_y = 0;
    int distance_tow_points = osd_width;
    int distance_thred = every_block_width*0.3;
    cv::Rect move_rect;
    cv::Mat move_mat;
    cv::Mat copy_blank;
    cv::Mat copy_move;

    // if (strcmp(argv[5], "None") == 0)
    // {
    image_puzzle_argb = cv::Mat(puzzle_height, puzzle_width, CV_8UC4, cv::Scalar(200, 130, 150, 100));
    for(int i = 0; i < level*level; i++)
    {
        cv::rectangle(image_puzzle_argb, cv::Rect((i%level)*every_block_width, (i/level)*every_block_height, every_block_width, every_block_height), cv::Scalar(255, 0, 0, 0), 5);
        std::string classString = to_string(i);
        cv::Size textSize = cv::getTextSize(classString, cv::FONT_HERSHEY_DUPLEX, 7*ratio_num, 8*ratio_num, 0);
        cv::putText(image_puzzle_argb, classString, cv::Point((i%level)*every_block_width + (every_block_width-textSize.width)/2,(i/level)*every_block_height + (every_block_height+textSize.height)/2),cv::FONT_HERSHEY_COMPLEX, 7*ratio_num, cv::Scalar(255, 0, 0, 255), 8*ratio_num, 0);
    }
    // }
    // else
    // {
    //     // Utils::bin_2_mat(argv[5], puzzle_width, puzzle_height, image_puzzle_argb);
    //     // for(int i = 0; i < level*level; i++)
    //     // {
    //     //     cv::rectangle(image_puzzle_argb, cv::Rect((i%level)*every_block_width, (i/level)*every_block_height, every_block_width, every_block_height), cv::Scalar(255, 0, 0, 0), 5);
    //     // }
    //     std::cout << "Non-empty is not supported !!!" << std::endl;
    //     return ;
    // }

    cv::Mat blank_block_puzzle(every_block_height, every_block_width, CV_8UC4, cv::Scalar(220, 114, 114, 114));
    cv::Mat image_puzzle_argb_blank = image_puzzle_argb(cv::Rect(0,0,every_block_width,every_block_height));
    blank_block_puzzle.copyTo(image_puzzle_argb_blank);

    cv::resize(image_puzzle_argb, image_puzzle_ori, cv::Size(puzzle_ori_width, puzzle_ori_height), cv::INTER_AREA);
    cv::Mat copy_ori_image_0 = osd_frame_tmp(cv::Rect(0,0,puzzle_width,puzzle_height));
    image_puzzle_argb.copyTo(copy_ori_image_0);
    cv::Mat copy_ori_image_1;

    copy_ori_image_1 = osd_frame_tmp(cv::Rect(puzzle_width+2,(puzzle_width-puzzle_ori_height)/2,puzzle_ori_width,puzzle_ori_height));
    image_puzzle_ori.copyTo(copy_ori_image_1);

    cv::Mat blank_block(every_block_height, every_block_width, CV_8UC4, cv::Scalar(220, 114, 114, 114));

    
    srand((unsigned)time(NULL));
    for(int i = 0; i < level*10; i++)
    {
        int k230_random = rand() % 4;
        int blank_x_tmp = blank_x;
        int blank_y_tmp = blank_y;
        if (k230_random < 2)
        {
            blank_x_tmp = blank_x + direction_vec[k230_random];
        }
        else
        {
            blank_y_tmp = blank_y + direction_vec[k230_random];
        }

        if((blank_x_tmp >= 0 && blank_x_tmp < level) && (blank_y_tmp >= 0 && blank_y_tmp < level) && (std::abs(blank_x - blank_x_tmp) <= 1 && std::abs(blank_y - blank_y_tmp) <= 1))
        {
            move_rect = cv::Rect(blank_x_tmp*every_block_width,blank_y_tmp*every_block_height,every_block_width,every_block_height);
            move_mat = osd_frame_tmp(move_rect);

            copy_blank = osd_frame_tmp(cv::Rect(blank_x_tmp*every_block_width,blank_y_tmp*every_block_height,every_block_width,every_block_height));
            copy_move = osd_frame_tmp(cv::Rect(blank_x*every_block_width,blank_y*every_block_height,every_block_width,every_block_height));
            move_mat.copyTo(copy_move);
            blank_block.copyTo(copy_blank);

            blank_x = blank_x_tmp;
            blank_y = blank_y_tmp;
        }
    }

    HandDetection hd(argv[1], atof(argv[2]), atof(argv[3]), {SENSOR_WIDTH, SENSOR_HEIGHT}, {SENSOR_CHANNEL, SENSOR_HEIGHT, SENSOR_WIDTH}, reinterpret_cast<uintptr_t>(vaddr), reinterpret_cast<uintptr_t>(paddr), atoi(argv[7]));
    sync();
    HandKeypoint hk(argv[4], {SENSOR_CHANNEL, SENSOR_HEIGHT, SENSOR_WIDTH}, reinterpret_cast<uintptr_t>(vaddr), reinterpret_cast<uintptr_t>(paddr), atoi(argv[7]));
    sync();

    std::vector<BoxInfo> results;
    std::vector<int> two_point;

    while (!isp_stop)
    {
        ScopedTiming st("total time", 1);

        {
            ScopedTiming st("read capture", atoi(argv[7]));
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
            ScopedTiming st("isp copy", atoi(argv[7]));
            auto vbvaddr = kd_mpi_sys_mmap_cached(dump_info.v_frame.phys_addr[0], size);
            memcpy(vaddr, (void *)vbvaddr, SENSOR_HEIGHT * SENSOR_WIDTH * 3);  
            kd_mpi_sys_munmap(vbvaddr, size);
        }

        results.clear();
        two_point.clear();

        hd.pre_process();
        hd.inference();
        hd.post_process(results);

        cv::Mat osd_frame(osd_height, osd_width, CV_8UC4, cv::Scalar(0, 0, 0, 0));
        cv::rotate(osd_frame, osd_frame, cv::ROTATE_90_COUNTERCLOCKWISE);

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

        if (max_id_hand != -1)
        {
            std::string text = hd.labels_[results[max_id_hand].label] + ":" + std::to_string(round(results[max_id_hand].score * 100) / 100.0);

            int w = results[max_id_hand].x2 - results[max_id_hand].x1 + 1;
            int h = results[max_id_hand].y2 - results[max_id_hand].y1 + 1;
            
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

            {
                ScopedTiming st("osd draw", atoi(argv[7]));
                hk.draw_keypoints(osd_frame, text, bbox, false, two_point);
            }
        }

        if(max_id_hand != -1 && two_point[1] <= SENSOR_WIDTH)
        {
            distance_tow_points = std::sqrt(std::pow((two_point[0] - two_point[2]),2) + std::pow((two_point[1] - two_point[3]),2))* 1.0 / SENSOR_WIDTH * osd_frame.cols;
            exact_division_x = (two_point[0] * 1.0 / SENSOR_WIDTH * osd_frame.cols)/every_block_width;
            exact_division_y = (two_point[1] * 1.0 / SENSOR_HEIGHT * osd_frame.rows)/every_block_height;
            if(distance_tow_points < distance_thred && exact_division_x >= 0 && exact_division_x < level && exact_division_y >= 0 && exact_division_y < level)
            {   
                if(std::abs(blank_x - exact_division_x) == 1 && std::abs(blank_y - exact_division_y) == 0)
                {
                    move_rect = cv::Rect(exact_division_x*every_block_width,exact_division_y*every_block_height,every_block_width,every_block_height);
                    move_mat = osd_frame_tmp(move_rect);

                    copy_blank = osd_frame_tmp(cv::Rect(exact_division_x*every_block_width,exact_division_y*every_block_height,every_block_width,every_block_height));
                    copy_move = osd_frame_tmp(cv::Rect(blank_x*every_block_width,blank_y*every_block_height,every_block_width,every_block_height));
                    move_mat.copyTo(copy_move);
                    blank_block.copyTo(copy_blank);

                    blank_x = exact_division_x;
                }
                else if (std::abs(blank_y - exact_division_y) == 1 && std::abs(blank_x - exact_division_x) == 0)
                {
                    move_rect = cv::Rect(exact_division_x*every_block_width,exact_division_y*every_block_height,every_block_width,every_block_height);
                    move_mat = osd_frame_tmp(move_rect);

                    copy_blank = osd_frame_tmp(cv::Rect(exact_division_x*every_block_width,exact_division_y*every_block_height,every_block_width,every_block_height));
                    copy_move = osd_frame_tmp(cv::Rect(blank_x*every_block_width,blank_y*every_block_height,every_block_width,every_block_height));
                    move_mat.copyTo(copy_move);
                    blank_block.copyTo(copy_blank);

                    blank_y = exact_division_y;
                }

                osd_frame = osd_frame_tmp.clone();
                if(two_point.size() > 0)
                {
                    int x1 = two_point[0] * 1.0 / SENSOR_WIDTH * osd_frame.cols;
                    int y1 = two_point[1] * 1.0 / SENSOR_HEIGHT * osd_frame.rows;
                    cv::circle(osd_frame, cv::Point(x1, y1), 10, cv::Scalar(255, 0, 255, 255), 10);
                }
            }
            else
            {
                osd_frame = osd_frame_tmp.clone();
                if(two_point.size() > 0)
                {
                    int x1 = two_point[0] * 1.0 / SENSOR_WIDTH * osd_frame.cols;
                    int y1 = two_point[1] * 1.0 / SENSOR_HEIGHT * osd_frame.rows;
                    cv::circle(osd_frame, cv::Point(x1, y1), 10, cv::Scalar(255, 255, 255, 0), 10);
                }
            }
            sync();
        }
        else
        {
            osd_frame = osd_frame_tmp.clone();
        }
        cv::rotate(osd_frame, osd_frame, cv::ROTATE_90_CLOCKWISE);

        {
            ScopedTiming st("osd copy", atoi(argv[7]));
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

    int puzzle_width;
    int puzzle_height;
    int puzzle_ori_width;
    int puzzle_ori_height;
    #if defined(STUDIO_HDMI)
    {
        puzzle_width = osd_height;
        puzzle_height = osd_height;
        puzzle_ori_width = osd_width-puzzle_height-5;
        puzzle_ori_height = osd_width-puzzle_height-5;
    }
    #else
    {
        puzzle_width = osd_width;
        puzzle_height = osd_width;
        puzzle_ori_width = osd_height-puzzle_height-5;
        puzzle_ori_height = osd_height-puzzle_height-5;
    }
    #endif


    int level = atoi(argv[6]);
    int every_block_width = puzzle_width/level;
    int every_block_height = puzzle_height/level;
    float ratio_num = every_block_width/360.0;
    int blank_x = 0;
    int blank_y = 0;
    std::vector<int> direction_vec = {-1,1,-1,1};

    cv::Mat image_puzzle_argb;
    cv::Mat image_puzzle_ori;
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

    int exact_division_x = 0;
    int exact_division_y = 0;
    int distance_tow_points;
    #if defined(STUDIO_HDMI)
    {
        distance_tow_points = osd_height;
    }
    #else
    {
        distance_tow_points = osd_width;
    }
    #endif
    int distance_thred = every_block_width*0.3;
    cv::Rect move_rect;
    cv::Mat move_mat;
    cv::Mat copy_blank;
    cv::Mat copy_move;

    // if (strcmp(argv[5], "None") == 0)
    // {
    image_puzzle_argb = cv::Mat(puzzle_height, puzzle_width, CV_8UC4, cv::Scalar(200, 130, 150, 100));
    for(int i = 0; i < level*level; i++)
    {
        cv::rectangle(image_puzzle_argb, cv::Rect((i%level)*every_block_width, (i/level)*every_block_height, every_block_width, every_block_height), cv::Scalar(255, 0, 0, 0), 5);
        std::string classString = to_string(i);
        cv::Size textSize = cv::getTextSize(classString, cv::FONT_HERSHEY_DUPLEX, 7*ratio_num, 8*ratio_num, 0);
        cv::putText(image_puzzle_argb, classString, cv::Point((i%level)*every_block_width + (every_block_width-textSize.width)/2,(i/level)*every_block_height + (every_block_height+textSize.height)/2),cv::FONT_HERSHEY_COMPLEX, 7*ratio_num, cv::Scalar(255, 0, 0, 255), 8*ratio_num, 0);
    }
    // }
    // else
    // {
    //     // Utils::bin_2_mat(argv[5], puzzle_width, puzzle_height, image_puzzle_argb);
    //     // for(int i = 0; i < level*level; i++)
    //     // {
    //     //     cv::rectangle(image_puzzle_argb, cv::Rect((i%level)*every_block_width, (i/level)*every_block_height, every_block_width, every_block_height), cv::Scalar(255, 0, 0, 0), 5);
    //     // }
    //     std::cout << "Non-empty is not supported !!!" << std::endl;
    //     return ;
    // }

    cv::Mat blank_block_puzzle(every_block_height, every_block_width, CV_8UC4, cv::Scalar(220, 114, 114, 114));
    cv::Mat image_puzzle_argb_blank = image_puzzle_argb(cv::Rect(0,0,every_block_width,every_block_height));
    blank_block_puzzle.copyTo(image_puzzle_argb_blank);

    cv::resize(image_puzzle_argb, image_puzzle_ori, cv::Size(puzzle_ori_width, puzzle_ori_height), cv::INTER_AREA);
    cv::Mat copy_ori_image_0 = osd_frame_tmp(cv::Rect(0,0,puzzle_width,puzzle_height));
    image_puzzle_argb.copyTo(copy_ori_image_0);
    cv::Mat copy_ori_image_1;

    copy_ori_image_1 = osd_frame_tmp(cv::Rect(puzzle_width+2,(puzzle_width-puzzle_ori_height)/2,puzzle_ori_width,puzzle_ori_height));
    image_puzzle_ori.copyTo(copy_ori_image_1);

    cv::Mat blank_block(every_block_height, every_block_width, CV_8UC4, cv::Scalar(220, 114, 114, 114));

    
    srand((unsigned)time(NULL));
    for(int i = 0; i < level*10; i++)
    {
        int k230_random = rand() % 4;
        int blank_x_tmp = blank_x;
        int blank_y_tmp = blank_y;
        if (k230_random < 2)
        {
            blank_x_tmp = blank_x + direction_vec[k230_random];
        }
        else
        {
            blank_y_tmp = blank_y + direction_vec[k230_random];
        }

        if((blank_x_tmp >= 0 && blank_x_tmp < level) && (blank_y_tmp >= 0 && blank_y_tmp < level) && (std::abs(blank_x - blank_x_tmp) <= 1 && std::abs(blank_y - blank_y_tmp) <= 1))
        {
            move_rect = cv::Rect(blank_x_tmp*every_block_width,blank_y_tmp*every_block_height,every_block_width,every_block_height);
            move_mat = osd_frame_tmp(move_rect);

            copy_blank = osd_frame_tmp(cv::Rect(blank_x_tmp*every_block_width,blank_y_tmp*every_block_height,every_block_width,every_block_height));
            copy_move = osd_frame_tmp(cv::Rect(blank_x*every_block_width,blank_y*every_block_height,every_block_width,every_block_height));
            move_mat.copyTo(copy_move);
            blank_block.copyTo(copy_blank);

            blank_x = blank_x_tmp;
            blank_y = blank_y_tmp;
        }
    }

    HandDetection hd(argv[1], atof(argv[2]), atof(argv[3]), {SENSOR_WIDTH, SENSOR_HEIGHT}, {SENSOR_CHANNEL, SENSOR_HEIGHT, SENSOR_WIDTH}, reinterpret_cast<uintptr_t>(vaddr), reinterpret_cast<uintptr_t>(paddr), atoi(argv[7]));
    sync();
    HandKeypoint hk(argv[4], {SENSOR_CHANNEL, SENSOR_HEIGHT, SENSOR_WIDTH}, reinterpret_cast<uintptr_t>(vaddr), reinterpret_cast<uintptr_t>(paddr), atoi(argv[7]));
    sync();

    std::vector<BoxInfo> results;
    std::vector<int> two_point;

    while (!isp_stop)
    {
        ScopedTiming st("total time", 1);

        {
            ScopedTiming st("read capture", atoi(argv[7]));
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
            ScopedTiming st("isp copy", atoi(argv[7]));
            auto vbvaddr = kd_mpi_sys_mmap_cached(dump_info.v_frame.phys_addr[0], size);
            memcpy(vaddr, (void *)vbvaddr, SENSOR_HEIGHT * SENSOR_WIDTH * 3);  
            kd_mpi_sys_munmap(vbvaddr, size);
        }

        results.clear();
        two_point.clear();

        hd.pre_process();
        hd.inference();
        hd.post_process(results);

        cv::Mat osd_frame(osd_height, osd_width, CV_8UC4, cv::Scalar(0, 0, 0, 0));

        #if defined(STUDIO_HDMI)
        {
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

            if (max_id_hand != -1)
            {
                std::string text = hd.labels_[results[max_id_hand].label] + ":" + std::to_string(round(results[max_id_hand].score * 100) / 100.0);

                int w = results[max_id_hand].x2 - results[max_id_hand].x1 + 1;
                int h = results[max_id_hand].y2 - results[max_id_hand].y1 + 1;
                
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

                {
                    ScopedTiming st("osd draw", atoi(argv[7]));
                    hk.draw_keypoints(osd_frame, text, bbox, false, two_point);
                }
            }

            if(max_id_hand != -1 && two_point[1] <= SENSOR_WIDTH)
            {
                distance_tow_points = std::sqrt(std::pow((two_point[0] - two_point[2]),2) + std::pow((two_point[1] - two_point[3]),2))* 1.0 / SENSOR_WIDTH * osd_frame.cols;
                exact_division_x = (two_point[0] * 1.0 / SENSOR_WIDTH * osd_frame.cols)/every_block_width;
                exact_division_y = (two_point[1] * 1.0 / SENSOR_HEIGHT * osd_frame.rows)/every_block_height;
                if(distance_tow_points < distance_thred && exact_division_x >= 0 && exact_division_x < level && exact_division_y >= 0 && exact_division_y < level)
                {   
                    if(std::abs(blank_x - exact_division_x) == 1 && std::abs(blank_y - exact_division_y) == 0)
                    {
                        move_rect = cv::Rect(exact_division_x*every_block_width,exact_division_y*every_block_height,every_block_width,every_block_height);
                        move_mat = osd_frame_tmp(move_rect);

                        copy_blank = osd_frame_tmp(cv::Rect(exact_division_x*every_block_width,exact_division_y*every_block_height,every_block_width,every_block_height));
                        copy_move = osd_frame_tmp(cv::Rect(blank_x*every_block_width,blank_y*every_block_height,every_block_width,every_block_height));
                        move_mat.copyTo(copy_move);
                        blank_block.copyTo(copy_blank);

                        blank_x = exact_division_x;
                    }
                    else if (std::abs(blank_y - exact_division_y) == 1 && std::abs(blank_x - exact_division_x) == 0)
                    {
                        move_rect = cv::Rect(exact_division_x*every_block_width,exact_division_y*every_block_height,every_block_width,every_block_height);
                        move_mat = osd_frame_tmp(move_rect);

                        copy_blank = osd_frame_tmp(cv::Rect(exact_division_x*every_block_width,exact_division_y*every_block_height,every_block_width,every_block_height));
                        copy_move = osd_frame_tmp(cv::Rect(blank_x*every_block_width,blank_y*every_block_height,every_block_width,every_block_height));
                        move_mat.copyTo(copy_move);
                        blank_block.copyTo(copy_blank);

                        blank_y = exact_division_y;
                    }

                    osd_frame = osd_frame_tmp.clone();
                    if(two_point.size() > 0)
                    {
                        int x1 = two_point[0] * 1.0 / SENSOR_WIDTH * osd_frame.cols;
                        int y1 = two_point[1] * 1.0 / SENSOR_HEIGHT * osd_frame.rows;
                        cv::circle(osd_frame, cv::Point(x1, y1), 10, cv::Scalar(255, 0, 255, 255), 10);
                    }
                }
                else
                {
                    osd_frame = osd_frame_tmp.clone();
                    if(two_point.size() > 0)
                    {
                        int x1 = two_point[0] * 1.0 / SENSOR_WIDTH * osd_frame.cols;
                        int y1 = two_point[1] * 1.0 / SENSOR_HEIGHT * osd_frame.rows;
                        cv::circle(osd_frame, cv::Point(x1, y1), 10, cv::Scalar(255, 255, 255, 0), 10);
                    }
                }
                sync();
            }
            else
            {
                osd_frame = osd_frame_tmp.clone();
            }
        }
        #else
        {
            cv::rotate(osd_frame, osd_frame, cv::ROTATE_90_COUNTERCLOCKWISE);

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

            if (max_id_hand != -1)
            {
                std::string text = hd.labels_[results[max_id_hand].label] + ":" + std::to_string(round(results[max_id_hand].score * 100) / 100.0);

                int w = results[max_id_hand].x2 - results[max_id_hand].x1 + 1;
                int h = results[max_id_hand].y2 - results[max_id_hand].y1 + 1;
                
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

                {
                    ScopedTiming st("osd draw", atoi(argv[7]));
                    hk.draw_keypoints(osd_frame, text, bbox, false, two_point);
                }
            }

            if(max_id_hand != -1 && two_point[1] <= SENSOR_WIDTH)
            {
                distance_tow_points = std::sqrt(std::pow((two_point[0] - two_point[2]),2) + std::pow((two_point[1] - two_point[3]),2))* 1.0 / SENSOR_WIDTH * osd_frame.cols;
                exact_division_x = (two_point[0] * 1.0 / SENSOR_WIDTH * osd_frame.cols)/every_block_width;
                exact_division_y = (two_point[1] * 1.0 / SENSOR_HEIGHT * osd_frame.rows)/every_block_height;
                if(distance_tow_points < distance_thred && exact_division_x >= 0 && exact_division_x < level && exact_division_y >= 0 && exact_division_y < level)
                {   
                    if(std::abs(blank_x - exact_division_x) == 1 && std::abs(blank_y - exact_division_y) == 0)
                    {
                        move_rect = cv::Rect(exact_division_x*every_block_width,exact_division_y*every_block_height,every_block_width,every_block_height);
                        move_mat = osd_frame_tmp(move_rect);

                        copy_blank = osd_frame_tmp(cv::Rect(exact_division_x*every_block_width,exact_division_y*every_block_height,every_block_width,every_block_height));
                        copy_move = osd_frame_tmp(cv::Rect(blank_x*every_block_width,blank_y*every_block_height,every_block_width,every_block_height));
                        move_mat.copyTo(copy_move);
                        blank_block.copyTo(copy_blank);

                        blank_x = exact_division_x;
                    }
                    else if (std::abs(blank_y - exact_division_y) == 1 && std::abs(blank_x - exact_division_x) == 0)
                    {
                        move_rect = cv::Rect(exact_division_x*every_block_width,exact_division_y*every_block_height,every_block_width,every_block_height);
                        move_mat = osd_frame_tmp(move_rect);

                        copy_blank = osd_frame_tmp(cv::Rect(exact_division_x*every_block_width,exact_division_y*every_block_height,every_block_width,every_block_height));
                        copy_move = osd_frame_tmp(cv::Rect(blank_x*every_block_width,blank_y*every_block_height,every_block_width,every_block_height));
                        move_mat.copyTo(copy_move);
                        blank_block.copyTo(copy_blank);

                        blank_y = exact_division_y;
                    }

                    osd_frame = osd_frame_tmp.clone();
                    if(two_point.size() > 0)
                    {
                        int x1 = two_point[0] * 1.0 / SENSOR_WIDTH * osd_frame.cols;
                        int y1 = two_point[1] * 1.0 / SENSOR_HEIGHT * osd_frame.rows;
                        cv::circle(osd_frame, cv::Point(x1, y1), 10, cv::Scalar(255, 0, 255, 255), 10);
                    }
                }
                else
                {
                    osd_frame = osd_frame_tmp.clone();
                    if(two_point.size() > 0)
                    {
                        int x1 = two_point[0] * 1.0 / SENSOR_WIDTH * osd_frame.cols;
                        int y1 = two_point[1] * 1.0 / SENSOR_HEIGHT * osd_frame.rows;
                        cv::circle(osd_frame, cv::Point(x1, y1), 10, cv::Scalar(255, 255, 255, 0), 10);
                    }
                }
                sync();
            }
            else
            {
                osd_frame = osd_frame_tmp.clone();
            }
            cv::rotate(osd_frame, osd_frame, cv::ROTATE_90_CLOCKWISE);
        }
        #endif

        {
            ScopedTiming st("osd copy", atoi(argv[7]));
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
    if (argc != 8)
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