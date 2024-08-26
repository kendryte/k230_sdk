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
	cout << "Usage: " << name << "<kmodel_det> <obj_thresh> <nms_thresh> <kmodel_kp> <debug_mode>" << endl
		 << "Options:" << endl
		 << "  kmodel_det      手掌检测kmodel路径\n"
         << "  obj_thresh      手掌检测阈值\n"
         << "  nms_thresh      手掌检测非极大值抑制阈值\n"
		 << "  kmodel_kp       手势关键点检测kmodel路径\n"
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

    HandDetection hd(argv[1], atof(argv[2]), atof(argv[3]), {SENSOR_WIDTH, SENSOR_HEIGHT}, {SENSOR_CHANNEL, SENSOR_HEIGHT, SENSOR_WIDTH}, reinterpret_cast<uintptr_t>(vaddr), reinterpret_cast<uintptr_t>(paddr), atoi(argv[5]));

    HandKeypoint hk(argv[4], {SENSOR_CHANNEL, SENSOR_HEIGHT, SENSOR_WIDTH}, reinterpret_cast<uintptr_t>(vaddr), reinterpret_cast<uintptr_t>(paddr), atoi(argv[5]));

    std::vector<BoxInfo> results;
    std::vector<int> two_point;
    bool first_start = true;
    int two_point_left_x = 0;
    int two_point_top_y = 0;
    int two_point_mean_w = 0;
    int two_point_mean_h = 0;
    int two_point_crop_w = 0;
    int two_point_crop_h = 0;
    int osd_plot_x = 0;
    int osd_plot_y = 0;
    float ori_new_ratio = 0;
    int new_resize_w = 0;
    int new_resize_h = 0;
    int crop_area = 0;
    int rect_frame_x = 0;
    int rect_frame_y = 0;

    int max_new_resize_w = 450;
    int max_new_resize_h = 450;

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
                ScopedTiming st("osd draw", atoi(argv[5]));
                hk.draw_keypoints(osd_frame, text, bbox, false, two_point);
            }
        }

        if(max_id_hand != -1)
        {
            if(first_start)
            {
                if(two_point[0] > 0 && two_point[0] < SENSOR_WIDTH && two_point[2] > 0 && two_point[2] < SENSOR_WIDTH && two_point[1] > 0 && two_point[1] < SENSOR_HEIGHT && two_point[3] > 0 && two_point[3] < SENSOR_HEIGHT)
                {
                    two_point_mean_w = std::sqrt(std::pow((two_point[0] - two_point[2]),2) + std::pow((two_point[1] - two_point[3]),2))*0.8;
                    two_point_mean_h = std::sqrt(std::pow((two_point[0] - two_point[2]),2) + std::pow((two_point[1] - two_point[3]),2))*0.8;
                    first_start = false;
                }
            }
            else
            {
                if(two_point[0] > 0 && two_point[0] < SENSOR_WIDTH && two_point[2] > 0 && two_point[2] < SENSOR_WIDTH && two_point[1] > 0 && two_point[1] < SENSOR_HEIGHT && two_point[3] > 0 && two_point[3] < SENSOR_HEIGHT)
                {
                    two_point_left_x = std::max((two_point[0] + two_point[2]) / 2 - two_point_mean_w / 2, 0);
                    two_point_top_y = std::max((two_point[1] + two_point[3]) / 2 - two_point_mean_h / 2, 0);
                    two_point_crop_w = std::min(std::min((two_point[0] + two_point[2]) / 2 - two_point_mean_w / 2 + two_point_mean_w , two_point_mean_w), SENSOR_WIDTH - ((two_point[0] + two_point[2]) / 2 - two_point_mean_w / 2));
                    two_point_crop_h = std::min(std::min((two_point[1] + two_point[3]) / 2 - two_point_mean_h / 2 + two_point_mean_h , two_point_mean_h), SENSOR_HEIGHT - ((two_point[1] + two_point[3]) / 2 - two_point_mean_h / 2));

                    ori_new_ratio = std::sqrt(std::pow((two_point[0] - two_point[2]),2) + std::pow((two_point[1] - two_point[3]),2))*0.8 / two_point_mean_w;

                    new_resize_w = two_point_crop_w * ori_new_ratio / SENSOR_WIDTH * osd_width;
                    new_resize_h = two_point_crop_h * ori_new_ratio / SENSOR_HEIGHT * osd_height;

                    new_resize_w = new_resize_w < max_new_resize_w  ? new_resize_w : max_new_resize_w;
                    new_resize_h = new_resize_h < max_new_resize_h ? new_resize_h : max_new_resize_h;

                    Bbox bbox_crop = {two_point_left_x,two_point_top_y,two_point_crop_w,two_point_crop_h};

                    std::unique_ptr<ai2d_builder> ai2d_builder_crop;
                    dims_t in_shape_crop{1, SENSOR_CHANNEL, SENSOR_HEIGHT, SENSOR_WIDTH};
                    dims_t out_shape_crop{1, 3, new_resize_h, new_resize_w};

                    runtime_tensor ai2d_in_tensor_crop = hrt::create(typecode_t::dt_uint8, in_shape_crop, hrt::pool_shared).expect("create ai2d input tensor failed");
                    runtime_tensor ai2d_out_tensor_crop = hrt::create(typecode_t::dt_uint8, out_shape_crop, hrt::pool_shared).expect("create ai2d input tensor failed");

                    size_t isp_size = SENSOR_CHANNEL * SENSOR_HEIGHT * SENSOR_WIDTH;
                    auto buf = ai2d_in_tensor_crop.impl()->to_host().unwrap()->buffer().as_host().unwrap().map(map_access_::map_write).unwrap().buffer();
                    memcpy(reinterpret_cast<char *>(buf.data()), (void *)vaddr, isp_size);
                    hrt::sync(ai2d_in_tensor_crop, sync_op_t::sync_write_back, true).expect("sync write_back failed");

                    Utils::crop_resize(bbox_crop, ai2d_builder_crop, ai2d_in_tensor_crop, ai2d_out_tensor_crop);

                    auto vaddr_out_buf = ai2d_out_tensor_crop.impl()->to_host().unwrap()->buffer().as_host().unwrap().map(map_access_::map_read).unwrap().buffer();
                    unsigned char *output = reinterpret_cast<unsigned char *>(vaddr_out_buf.data());

                    crop_area = new_resize_h*new_resize_w;
                    for(uint32_t hh = 0; hh < new_resize_h; hh++)
                    {
                        for(uint32_t ww = 0; ww < new_resize_w; ww++)
                        {
                            int new_hh = (hh + two_point_top_y * 1.0 / SENSOR_HEIGHT * osd_height);
                            int new_ww = (ww + two_point_left_x * 1.0 / SENSOR_WIDTH * osd_width);
                            int osd_channel_index = (new_hh * osd_width + new_ww) * 4;
                            if(osd_frame.data[osd_channel_index + 0] == 0)                        
                            {
                                int ori_pix_index = hh * new_resize_w + ww;
                                osd_frame.data[osd_channel_index + 0] = 255;
                                osd_frame.data[osd_channel_index + 1] =  output[ori_pix_index];
                                osd_frame.data[osd_channel_index + 2] =  output[ori_pix_index + crop_area];
                                osd_frame.data[osd_channel_index + 3] =  output[ori_pix_index + crop_area * 2]; 
                            }                        
                        }
                    }

                    rect_frame_x = two_point_left_x * 1.0 / SENSOR_WIDTH * osd_width;
                    rect_frame_y = two_point_top_y * 1.0 / SENSOR_HEIGHT * osd_height;
                    cv::rectangle(osd_frame, cv::Rect(rect_frame_x, rect_frame_y , new_resize_w, new_resize_h), cv::Scalar( 255,0, 255, 255), 2, 2, 0);
                }
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

    HandDetection hd(argv[1], atof(argv[2]), atof(argv[3]), {SENSOR_WIDTH, SENSOR_HEIGHT}, {SENSOR_CHANNEL, SENSOR_HEIGHT, SENSOR_WIDTH}, reinterpret_cast<uintptr_t>(vaddr), reinterpret_cast<uintptr_t>(paddr), atoi(argv[5]));

    HandKeypoint hk(argv[4], {SENSOR_CHANNEL, SENSOR_HEIGHT, SENSOR_WIDTH}, reinterpret_cast<uintptr_t>(vaddr), reinterpret_cast<uintptr_t>(paddr), atoi(argv[5]));

    std::vector<BoxInfo> results;
    std::vector<int> two_point;
    bool first_start = true;
    int two_point_left_x = 0;
    int two_point_top_y = 0;
    int two_point_mean_w = 0;
    int two_point_mean_h = 0;
    int two_point_crop_w = 0;
    int two_point_crop_h = 0;
    int osd_plot_x = 0;
    int osd_plot_y = 0;
    float ori_new_ratio = 0;
    int new_resize_w = 0;
    int new_resize_h = 0;
    int crop_area = 0;
    int rect_frame_x = 0;
    int rect_frame_y = 0;

    int max_new_resize_w = 450;
    int max_new_resize_h = 450;

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
                ScopedTiming st("osd draw", atoi(argv[5]));
                hk.draw_keypoints(osd_frame, text, bbox, false, two_point);
            }
        }

        if(max_id_hand != -1)
        {
            if(first_start)
            {
                if(two_point[0] > 0 && two_point[0] < SENSOR_WIDTH && two_point[2] > 0 && two_point[2] < SENSOR_WIDTH && two_point[1] > 0 && two_point[1] < SENSOR_HEIGHT && two_point[3] > 0 && two_point[3] < SENSOR_HEIGHT)
                {
                    two_point_mean_w = std::sqrt(std::pow((two_point[0] - two_point[2]),2) + std::pow((two_point[1] - two_point[3]),2))*0.8;
                    two_point_mean_h = std::sqrt(std::pow((two_point[0] - two_point[2]),2) + std::pow((two_point[1] - two_point[3]),2))*0.8;
                    first_start = false;
                }
            }
            else
            {
                if(two_point[0] > 0 && two_point[0] < SENSOR_WIDTH && two_point[2] > 0 && two_point[2] < SENSOR_WIDTH && two_point[1] > 0 && two_point[1] < SENSOR_HEIGHT && two_point[3] > 0 && two_point[3] < SENSOR_HEIGHT)
                {
                    two_point_left_x = std::max((two_point[0] + two_point[2]) / 2 - two_point_mean_w / 2, 0);
                    two_point_top_y = std::max((two_point[1] + two_point[3]) / 2 - two_point_mean_h / 2, 0);
                    two_point_crop_w = std::min(std::min((two_point[0] + two_point[2]) / 2 - two_point_mean_w / 2 + two_point_mean_w , two_point_mean_w), SENSOR_WIDTH - ((two_point[0] + two_point[2]) / 2 - two_point_mean_w / 2));
                    two_point_crop_h = std::min(std::min((two_point[1] + two_point[3]) / 2 - two_point_mean_h / 2 + two_point_mean_h , two_point_mean_h), SENSOR_HEIGHT - ((two_point[1] + two_point[3]) / 2 - two_point_mean_h / 2));

                    ori_new_ratio = std::sqrt(std::pow((two_point[0] - two_point[2]),2) + std::pow((two_point[1] - two_point[3]),2))*0.8 / two_point_mean_w;

                    new_resize_w = two_point_crop_w * ori_new_ratio / SENSOR_WIDTH * osd_width;
                    new_resize_h = two_point_crop_h * ori_new_ratio / SENSOR_HEIGHT * osd_height;

                    new_resize_w = new_resize_w < max_new_resize_w  ? new_resize_w : max_new_resize_w;
                    new_resize_h = new_resize_h < max_new_resize_h ? new_resize_h : max_new_resize_h;

                    Bbox bbox_crop = {two_point_left_x,two_point_top_y,two_point_crop_w,two_point_crop_h};

                    std::unique_ptr<ai2d_builder> ai2d_builder_crop;
                    dims_t in_shape_crop{1, SENSOR_CHANNEL, SENSOR_HEIGHT, SENSOR_WIDTH};
                    dims_t out_shape_crop{1, 3, new_resize_h, new_resize_w};

                    runtime_tensor ai2d_in_tensor_crop = hrt::create(typecode_t::dt_uint8, in_shape_crop, hrt::pool_shared).expect("create ai2d input tensor failed");
                    runtime_tensor ai2d_out_tensor_crop = hrt::create(typecode_t::dt_uint8, out_shape_crop, hrt::pool_shared).expect("create ai2d input tensor failed");

                    size_t isp_size = SENSOR_CHANNEL * SENSOR_HEIGHT * SENSOR_WIDTH;
                    auto buf = ai2d_in_tensor_crop.impl()->to_host().unwrap()->buffer().as_host().unwrap().map(map_access_::map_write).unwrap().buffer();
                    memcpy(reinterpret_cast<char *>(buf.data()), (void *)vaddr, isp_size);
                    hrt::sync(ai2d_in_tensor_crop, sync_op_t::sync_write_back, true).expect("sync write_back failed");

                    Utils::crop_resize(bbox_crop, ai2d_builder_crop, ai2d_in_tensor_crop, ai2d_out_tensor_crop);

                    auto vaddr_out_buf = ai2d_out_tensor_crop.impl()->to_host().unwrap()->buffer().as_host().unwrap().map(map_access_::map_read).unwrap().buffer();
                    unsigned char *output = reinterpret_cast<unsigned char *>(vaddr_out_buf.data());

                    crop_area = new_resize_h*new_resize_w;
                    for(uint32_t hh = 0; hh < new_resize_h; hh++)
                    {
                        for(uint32_t ww = 0; ww < new_resize_w; ww++)
                        {
                            int new_hh = (hh + two_point_top_y * 1.0 / SENSOR_HEIGHT * osd_height);
                            int new_ww = (ww + two_point_left_x * 1.0 / SENSOR_WIDTH * osd_width);
                            int osd_channel_index = (new_hh * osd_width + new_ww) * 4;
                            if(osd_frame.data[osd_channel_index + 0] == 0)                        
                            {
                                int ori_pix_index = hh * new_resize_w + ww;
                                osd_frame.data[osd_channel_index + 0] = 255;
                                osd_frame.data[osd_channel_index + 1] =  output[ori_pix_index];
                                osd_frame.data[osd_channel_index + 2] =  output[ori_pix_index + crop_area];
                                osd_frame.data[osd_channel_index + 3] =  output[ori_pix_index + crop_area * 2]; 
                            }                        
                        }
                    }

                    rect_frame_x = two_point_left_x * 1.0 / SENSOR_WIDTH * osd_width;
                    rect_frame_y = two_point_top_y * 1.0 / SENSOR_HEIGHT * osd_height;
                    cv::rectangle(osd_frame, cv::Rect(rect_frame_x, rect_frame_y , new_resize_w, new_resize_h), cv::Scalar( 255,0, 255, 255), 2, 2, 0);
                }
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

    HandDetection hd(argv[1], atof(argv[2]), atof(argv[3]), {SENSOR_WIDTH, SENSOR_HEIGHT}, {SENSOR_CHANNEL, SENSOR_HEIGHT, SENSOR_WIDTH}, reinterpret_cast<uintptr_t>(vaddr), reinterpret_cast<uintptr_t>(paddr), atoi(argv[5]));

    HandKeypoint hk(argv[4], {SENSOR_CHANNEL, SENSOR_HEIGHT, SENSOR_WIDTH}, reinterpret_cast<uintptr_t>(vaddr), reinterpret_cast<uintptr_t>(paddr), atoi(argv[5]));

    std::vector<BoxInfo> results;
    std::vector<int> two_point;
    bool first_start = true;
    int two_point_left_x = 0;
    int two_point_top_y = 0;
    int two_point_mean_w = 0;
    int two_point_mean_h = 0;
    int two_point_crop_w = 0;
    int two_point_crop_h = 0;
    int osd_plot_x = 0;
    int osd_plot_y = 0;
    float ori_new_ratio = 0;
    int new_resize_w = 0;
    int new_resize_h = 0;
    int crop_area = 0;
    int rect_frame_x = 0;
    int rect_frame_y = 0;

    int max_new_resize_w = 300;
    int max_new_resize_h = 300;

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
                ScopedTiming st("osd draw", atoi(argv[5]));
                hk.draw_keypoints(osd_frame, text, bbox, false, two_point);
            }
        }

        if(max_id_hand != -1)
        {
            if(first_start)
            {
                if(two_point[0] > 0 && two_point[0] < SENSOR_WIDTH && two_point[2] > 0 && two_point[2] < SENSOR_WIDTH && two_point[1] > 0 && two_point[1] < SENSOR_HEIGHT && two_point[3] > 0 && two_point[3] < SENSOR_HEIGHT)
                {
                    two_point_mean_w = std::sqrt(std::pow((two_point[0] - two_point[2]),2) + std::pow((two_point[1] - two_point[3]),2))*0.8;
                    two_point_mean_h = std::sqrt(std::pow((two_point[0] - two_point[2]),2) + std::pow((two_point[1] - two_point[3]),2))*0.8;
                    first_start = false;
                }
            }
            else
            {
                if(two_point[0] > 0 && two_point[0] < SENSOR_WIDTH && two_point[2] > 0 && two_point[2] < SENSOR_WIDTH && two_point[1] > 0 && two_point[1] < SENSOR_HEIGHT && two_point[3] > 0 && two_point[3] < SENSOR_HEIGHT)
                {
                    two_point_left_x = std::max((two_point[0] + two_point[2]) / 2 - two_point_mean_w / 2, 0);
                    two_point_top_y = std::max((two_point[1] + two_point[3]) / 2 - two_point_mean_h / 2, 0);
                    two_point_crop_w = std::min(std::min((two_point[0] + two_point[2]) / 2 - two_point_mean_w / 2 + two_point_mean_w , two_point_mean_w), SENSOR_WIDTH - ((two_point[0] + two_point[2]) / 2 - two_point_mean_w / 2));
                    two_point_crop_h = std::min(std::min((two_point[1] + two_point[3]) / 2 - two_point_mean_h / 2 + two_point_mean_h , two_point_mean_h), SENSOR_HEIGHT - ((two_point[1] + two_point[3]) / 2 - two_point_mean_h / 2));

                    ori_new_ratio = std::sqrt(std::pow((two_point[0] - two_point[2]),2) + std::pow((two_point[1] - two_point[3]),2))*0.8 / two_point_mean_w;

                    new_resize_w = two_point_crop_w * ori_new_ratio / SENSOR_WIDTH * osd_frame.cols;
                    new_resize_h = two_point_crop_h * ori_new_ratio / SENSOR_HEIGHT * osd_frame.rows;

                    new_resize_w = new_resize_w < max_new_resize_w  ? new_resize_w : max_new_resize_w;
                    new_resize_h = new_resize_h < max_new_resize_h ? new_resize_h : max_new_resize_h;

                    Bbox bbox_crop = {two_point_left_x,two_point_top_y,two_point_crop_w,two_point_crop_h};

                    std::unique_ptr<ai2d_builder> ai2d_builder_crop;
                    dims_t in_shape_crop{1, SENSOR_CHANNEL, SENSOR_HEIGHT, SENSOR_WIDTH};
                    dims_t out_shape_crop{1, 3, new_resize_h, new_resize_w};

                    runtime_tensor ai2d_in_tensor_crop = hrt::create(typecode_t::dt_uint8, in_shape_crop, hrt::pool_shared).expect("create ai2d input tensor failed");
                    runtime_tensor ai2d_out_tensor_crop = hrt::create(typecode_t::dt_uint8, out_shape_crop, hrt::pool_shared).expect("create ai2d input tensor failed");

                    size_t isp_size = SENSOR_CHANNEL * SENSOR_HEIGHT * SENSOR_WIDTH;
                    auto buf = ai2d_in_tensor_crop.impl()->to_host().unwrap()->buffer().as_host().unwrap().map(map_access_::map_write).unwrap().buffer();
                    memcpy(reinterpret_cast<char *>(buf.data()), (void *)vaddr, isp_size);
                    hrt::sync(ai2d_in_tensor_crop, sync_op_t::sync_write_back, true).expect("sync write_back failed");

                    Utils::crop_resize(bbox_crop, ai2d_builder_crop, ai2d_in_tensor_crop, ai2d_out_tensor_crop);

                    auto vaddr_out_buf = ai2d_out_tensor_crop.impl()->to_host().unwrap()->buffer().as_host().unwrap().map(map_access_::map_read).unwrap().buffer();
                    unsigned char *output = reinterpret_cast<unsigned char *>(vaddr_out_buf.data());

                    crop_area = new_resize_h*new_resize_w;
                    for(uint32_t hh = 0; hh < new_resize_h; hh++)
                    {
                        for(uint32_t ww = 0; ww < new_resize_w; ww++)
                        {
                            int new_hh = (hh + two_point_top_y * 1.0 / SENSOR_HEIGHT * osd_frame.rows);
                            int new_ww = (ww + two_point_left_x * 1.0 / SENSOR_WIDTH * osd_frame.cols);
                            int osd_channel_index = (new_hh * osd_frame.cols + new_ww) * 4;
                            if(osd_frame.data[osd_channel_index + 0] == 0)                        
                            {
                                int ori_pix_index = hh * new_resize_w + ww;
                                osd_frame.data[osd_channel_index + 0] = 255;
                                osd_frame.data[osd_channel_index + 1] =  output[ori_pix_index];
                                osd_frame.data[osd_channel_index + 2] =  output[ori_pix_index + crop_area];
                                osd_frame.data[osd_channel_index + 3] =  output[ori_pix_index + crop_area * 2]; 
                            }                        
                        }
                    }

                    rect_frame_x = two_point_left_x * 1.0 / SENSOR_WIDTH * osd_frame.cols;
                    rect_frame_y = two_point_top_y * 1.0 / SENSOR_HEIGHT * osd_frame.rows;
                    cv::rectangle(osd_frame, cv::Rect(rect_frame_x, rect_frame_y , new_resize_w, new_resize_h), cv::Scalar( 255,0, 255, 255), 2, 2, 0);
                }
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

    HandDetection hd(argv[1], atof(argv[2]), atof(argv[3]), {SENSOR_WIDTH, SENSOR_HEIGHT}, {SENSOR_CHANNEL, SENSOR_HEIGHT, SENSOR_WIDTH}, reinterpret_cast<uintptr_t>(vaddr), reinterpret_cast<uintptr_t>(paddr), atoi(argv[5]));

    HandKeypoint hk(argv[4], {SENSOR_CHANNEL, SENSOR_HEIGHT, SENSOR_WIDTH}, reinterpret_cast<uintptr_t>(vaddr), reinterpret_cast<uintptr_t>(paddr), atoi(argv[5]));

    std::vector<BoxInfo> results;
    std::vector<int> two_point;
    bool first_start = true;
    int two_point_left_x = 0;
    int two_point_top_y = 0;
    int two_point_mean_w = 0;
    int two_point_mean_h = 0;
    int two_point_crop_w = 0;
    int two_point_crop_h = 0;
    int osd_plot_x = 0;
    int osd_plot_y = 0;
    float ori_new_ratio = 0;
    int new_resize_w = 0;
    int new_resize_h = 0;
    int crop_area = 0;
    int rect_frame_x = 0;
    int rect_frame_y = 0;

    int max_new_resize_w = 300;
    int max_new_resize_h = 300;

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
                    ScopedTiming st("osd draw", atoi(argv[5]));
                    hk.draw_keypoints(osd_frame, text, bbox, false, two_point);
                }
            }

            if(max_id_hand != -1)
            {
                if(first_start)
                {
                    if(two_point[0] > 0 && two_point[0] < SENSOR_WIDTH && two_point[2] > 0 && two_point[2] < SENSOR_WIDTH && two_point[1] > 0 && two_point[1] < SENSOR_HEIGHT && two_point[3] > 0 && two_point[3] < SENSOR_HEIGHT)
                    {
                        two_point_mean_w = std::sqrt(std::pow((two_point[0] - two_point[2]),2) + std::pow((two_point[1] - two_point[3]),2))*0.8;
                        two_point_mean_h = std::sqrt(std::pow((two_point[0] - two_point[2]),2) + std::pow((two_point[1] - two_point[3]),2))*0.8;
                        first_start = false;
                    }
                }
                else
                {
                    if(two_point[0] > 0 && two_point[0] < SENSOR_WIDTH && two_point[2] > 0 && two_point[2] < SENSOR_WIDTH && two_point[1] > 0 && two_point[1] < SENSOR_HEIGHT && two_point[3] > 0 && two_point[3] < SENSOR_HEIGHT)
                    {
                        two_point_left_x = std::max((two_point[0] + two_point[2]) / 2 - two_point_mean_w / 2, 0);
                        two_point_top_y = std::max((two_point[1] + two_point[3]) / 2 - two_point_mean_h / 2, 0);
                        two_point_crop_w = std::min(std::min((two_point[0] + two_point[2]) / 2 - two_point_mean_w / 2 + two_point_mean_w , two_point_mean_w), SENSOR_WIDTH - ((two_point[0] + two_point[2]) / 2 - two_point_mean_w / 2));
                        two_point_crop_h = std::min(std::min((two_point[1] + two_point[3]) / 2 - two_point_mean_h / 2 + two_point_mean_h , two_point_mean_h), SENSOR_HEIGHT - ((two_point[1] + two_point[3]) / 2 - two_point_mean_h / 2));

                        ori_new_ratio = std::sqrt(std::pow((two_point[0] - two_point[2]),2) + std::pow((two_point[1] - two_point[3]),2))*0.8 / two_point_mean_w;

                        new_resize_w = two_point_crop_w * ori_new_ratio / SENSOR_WIDTH * osd_frame.cols;
                        new_resize_h = two_point_crop_h * ori_new_ratio / SENSOR_HEIGHT * osd_frame.rows;

                        new_resize_w = new_resize_w < max_new_resize_w  ? new_resize_w : max_new_resize_w;
                        new_resize_h = new_resize_h < max_new_resize_h ? new_resize_h : max_new_resize_h;

                        Bbox bbox_crop = {two_point_left_x,two_point_top_y,two_point_crop_w,two_point_crop_h};

                        std::unique_ptr<ai2d_builder> ai2d_builder_crop;
                        dims_t in_shape_crop{1, SENSOR_CHANNEL, SENSOR_HEIGHT, SENSOR_WIDTH};
                        dims_t out_shape_crop{1, 3, new_resize_h, new_resize_w};

                        runtime_tensor ai2d_in_tensor_crop = hrt::create(typecode_t::dt_uint8, in_shape_crop, hrt::pool_shared).expect("create ai2d input tensor failed");
                        runtime_tensor ai2d_out_tensor_crop = hrt::create(typecode_t::dt_uint8, out_shape_crop, hrt::pool_shared).expect("create ai2d input tensor failed");

                        size_t isp_size = SENSOR_CHANNEL * SENSOR_HEIGHT * SENSOR_WIDTH;
                        auto buf = ai2d_in_tensor_crop.impl()->to_host().unwrap()->buffer().as_host().unwrap().map(map_access_::map_write).unwrap().buffer();
                        memcpy(reinterpret_cast<char *>(buf.data()), (void *)vaddr, isp_size);
                        hrt::sync(ai2d_in_tensor_crop, sync_op_t::sync_write_back, true).expect("sync write_back failed");

                        Utils::crop_resize(bbox_crop, ai2d_builder_crop, ai2d_in_tensor_crop, ai2d_out_tensor_crop);

                        auto vaddr_out_buf = ai2d_out_tensor_crop.impl()->to_host().unwrap()->buffer().as_host().unwrap().map(map_access_::map_read).unwrap().buffer();
                        unsigned char *output = reinterpret_cast<unsigned char *>(vaddr_out_buf.data());

                        crop_area = new_resize_h*new_resize_w;
                        for(uint32_t hh = 0; hh < new_resize_h; hh++)
                        {
                            for(uint32_t ww = 0; ww < new_resize_w; ww++)
                            {
                                int new_hh = (hh + two_point_top_y * 1.0 / SENSOR_HEIGHT * osd_frame.rows);
                                int new_ww = (ww + two_point_left_x * 1.0 / SENSOR_WIDTH * osd_frame.cols);
                                int osd_channel_index = (new_hh * osd_frame.cols + new_ww) * 4;
                                if(osd_frame.data[osd_channel_index + 0] == 0)                        
                                {
                                    int ori_pix_index = hh * new_resize_w + ww;
                                    osd_frame.data[osd_channel_index + 0] = 255;
                                    osd_frame.data[osd_channel_index + 1] =  output[ori_pix_index];
                                    osd_frame.data[osd_channel_index + 2] =  output[ori_pix_index + crop_area];
                                    osd_frame.data[osd_channel_index + 3] =  output[ori_pix_index + crop_area * 2]; 
                                }                        
                            }
                        }

                        rect_frame_x = two_point_left_x * 1.0 / SENSOR_WIDTH * osd_frame.cols;
                        rect_frame_y = two_point_top_y * 1.0 / SENSOR_HEIGHT * osd_frame.rows;
                        cv::rectangle(osd_frame, cv::Rect(rect_frame_x, rect_frame_y , new_resize_w, new_resize_h), cv::Scalar( 255,0, 255, 255), 2, 2, 0);
                    }
                }
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
                    ScopedTiming st("osd draw", atoi(argv[5]));
                    hk.draw_keypoints(osd_frame, text, bbox, false, two_point);
                }
            }

            if(max_id_hand != -1)
            {
                if(first_start)
                {
                    if(two_point[0] > 0 && two_point[0] < SENSOR_WIDTH && two_point[2] > 0 && two_point[2] < SENSOR_WIDTH && two_point[1] > 0 && two_point[1] < SENSOR_HEIGHT && two_point[3] > 0 && two_point[3] < SENSOR_HEIGHT)
                    {
                        two_point_mean_w = std::sqrt(std::pow((two_point[0] - two_point[2]),2) + std::pow((two_point[1] - two_point[3]),2))*0.8;
                        two_point_mean_h = std::sqrt(std::pow((two_point[0] - two_point[2]),2) + std::pow((two_point[1] - two_point[3]),2))*0.8;
                        first_start = false;
                    }
                }
                else
                {
                    if(two_point[0] > 0 && two_point[0] < SENSOR_WIDTH && two_point[2] > 0 && two_point[2] < SENSOR_WIDTH && two_point[1] > 0 && two_point[1] < SENSOR_HEIGHT && two_point[3] > 0 && two_point[3] < SENSOR_HEIGHT)
                    {
                        two_point_left_x = std::max((two_point[0] + two_point[2]) / 2 - two_point_mean_w / 2, 0);
                        two_point_top_y = std::max((two_point[1] + two_point[3]) / 2 - two_point_mean_h / 2, 0);
                        two_point_crop_w = std::min(std::min((two_point[0] + two_point[2]) / 2 - two_point_mean_w / 2 + two_point_mean_w , two_point_mean_w), SENSOR_WIDTH - ((two_point[0] + two_point[2]) / 2 - two_point_mean_w / 2));
                        two_point_crop_h = std::min(std::min((two_point[1] + two_point[3]) / 2 - two_point_mean_h / 2 + two_point_mean_h , two_point_mean_h), SENSOR_HEIGHT - ((two_point[1] + two_point[3]) / 2 - two_point_mean_h / 2));

                        ori_new_ratio = std::sqrt(std::pow((two_point[0] - two_point[2]),2) + std::pow((two_point[1] - two_point[3]),2))*0.8 / two_point_mean_w;

                        new_resize_w = two_point_crop_w * ori_new_ratio / SENSOR_WIDTH * osd_frame.cols;
                        new_resize_h = two_point_crop_h * ori_new_ratio / SENSOR_HEIGHT * osd_frame.rows;

                        new_resize_w = new_resize_w < max_new_resize_w  ? new_resize_w : max_new_resize_w;
                        new_resize_h = new_resize_h < max_new_resize_h ? new_resize_h : max_new_resize_h;

                        Bbox bbox_crop = {two_point_left_x,two_point_top_y,two_point_crop_w,two_point_crop_h};

                        std::unique_ptr<ai2d_builder> ai2d_builder_crop;
                        dims_t in_shape_crop{1, SENSOR_CHANNEL, SENSOR_HEIGHT, SENSOR_WIDTH};
                        dims_t out_shape_crop{1, 3, new_resize_h, new_resize_w};

                        runtime_tensor ai2d_in_tensor_crop = hrt::create(typecode_t::dt_uint8, in_shape_crop, hrt::pool_shared).expect("create ai2d input tensor failed");
                        runtime_tensor ai2d_out_tensor_crop = hrt::create(typecode_t::dt_uint8, out_shape_crop, hrt::pool_shared).expect("create ai2d input tensor failed");

                        size_t isp_size = SENSOR_CHANNEL * SENSOR_HEIGHT * SENSOR_WIDTH;
                        auto buf = ai2d_in_tensor_crop.impl()->to_host().unwrap()->buffer().as_host().unwrap().map(map_access_::map_write).unwrap().buffer();
                        memcpy(reinterpret_cast<char *>(buf.data()), (void *)vaddr, isp_size);
                        hrt::sync(ai2d_in_tensor_crop, sync_op_t::sync_write_back, true).expect("sync write_back failed");

                        Utils::crop_resize(bbox_crop, ai2d_builder_crop, ai2d_in_tensor_crop, ai2d_out_tensor_crop);

                        auto vaddr_out_buf = ai2d_out_tensor_crop.impl()->to_host().unwrap()->buffer().as_host().unwrap().map(map_access_::map_read).unwrap().buffer();
                        unsigned char *output = reinterpret_cast<unsigned char *>(vaddr_out_buf.data());

                        crop_area = new_resize_h*new_resize_w;
                        for(uint32_t hh = 0; hh < new_resize_h; hh++)
                        {
                            for(uint32_t ww = 0; ww < new_resize_w; ww++)
                            {
                                int new_hh = (hh + two_point_top_y * 1.0 / SENSOR_HEIGHT * osd_frame.rows);
                                int new_ww = (ww + two_point_left_x * 1.0 / SENSOR_WIDTH * osd_frame.cols);
                                int osd_channel_index = (new_hh * osd_frame.cols + new_ww) * 4;
                                if(osd_frame.data[osd_channel_index + 0] == 0)                        
                                {
                                    int ori_pix_index = hh * new_resize_w + ww;
                                    osd_frame.data[osd_channel_index + 0] = 255;
                                    osd_frame.data[osd_channel_index + 1] =  output[ori_pix_index];
                                    osd_frame.data[osd_channel_index + 2] =  output[ori_pix_index + crop_area];
                                    osd_frame.data[osd_channel_index + 3] =  output[ori_pix_index + crop_area * 2]; 
                                }                        
                            }
                        }

                        rect_frame_x = two_point_left_x * 1.0 / SENSOR_WIDTH * osd_frame.cols;
                        rect_frame_y = two_point_top_y * 1.0 / SENSOR_HEIGHT * osd_frame.rows;
                        cv::rectangle(osd_frame, cv::Rect(rect_frame_x, rect_frame_y , new_resize_w, new_resize_h), cv::Scalar( 255,0, 255, 255), 2, 2, 0);
                    }
                }
            }
            cv::rotate(osd_frame, osd_frame, cv::ROTATE_90_CLOCKWISE);
        }
        #endif

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
