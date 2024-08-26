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
#include "face_detection.h"
#include "hand_detection.h"

using std::cerr;
using std::cout;
using std::endl;

std::atomic<bool> isp_stop(false);

void print_usage(const char *name)
{
    cout << "Usage: " << name << "<face_kmodel_det> <face_obj_thres> <face_nms_thres> <hand_kmodel_det> <hand_obj_thresh> <hand_nms_thresh> <init_area> <init_len> <warning_amount> <debug_mode>" << endl
         << "Options:" << endl
         << "  face_kmodel_det      人脸检测kmodel路径\n"
         << "  face_obj_thres       人脸检测kmodel阈值\n"
         << "  face_nms_thres       人脸检测kmodel nms阈值\n"
		 << "  hand_kmodel_det      手掌检测kmodel路径\n"
         << "  hand_obj_thresh      手掌检测阈值\n"
         << "  hand_nms_thresh      手掌检测非极大值抑制阈值\n"
         << "  init_area            定义基准人脸面积\n"
         << "  init_len             定义基准距离长度\n"
         << "  warning_amount       达到需要提醒注意的帧数\n"
         << "  debug_mode      是否需要调试，0、1、2分别表示不调试、简单调试、详细调试\n"
         << "\n"
         << endl;
}

void video_proc_v1(char *argv[])
{
    vivcap_start();
    // 设置osd参数
    k_video_frame_info vf_info;
    void *pic_vaddr = NULL;       //osd
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

    FaceDetection fd(argv[1], atof(argv[2]),atof(argv[3]), {SENSOR_CHANNEL, SENSOR_HEIGHT, SENSOR_WIDTH}, reinterpret_cast<uintptr_t>(vaddr), reinterpret_cast<uintptr_t>(paddr), atoi(argv[10]));
    HandDetection hd(argv[4], atof(argv[5]), atof(argv[6]), {SENSOR_WIDTH, SENSOR_HEIGHT}, {SENSOR_CHANNEL, SENSOR_HEIGHT, SENSOR_WIDTH}, reinterpret_cast<uintptr_t>(vaddr), reinterpret_cast<uintptr_t>(paddr), atoi(argv[10]));

    vector<FaceDetectionInfo> face_results;
    vector<BoxInfo> hand_results;

    float init_area = atof(argv[7]);
    float init_len = atof(argv[8]);
    float now_area;
    float now_len;
    int warning_amount = atoi(argv[9]);
    int warning_count_smk_drk = 0;
    int warning_count_drk = 0;
    int warning_count_call = 0;

    while (!isp_stop)
    {
        ScopedTiming st("total time", 1);
        {
            ScopedTiming st("read capture", atoi(argv[10]));
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
            ScopedTiming st("isp copy", atoi(argv[10]));
            auto vbvaddr = kd_mpi_sys_mmap_cached(dump_info.v_frame.phys_addr[0], size);
            memcpy(vaddr, (void *)vbvaddr, SENSOR_HEIGHT * SENSOR_WIDTH * 3);  // 这里以后可以去掉，不用copy
            kd_mpi_sys_munmap(vbvaddr, size);
        }

        face_results.clear();
        fd.pre_process();
        fd.inference();
        // 旋转后图像
        fd.post_process({SENSOR_WIDTH, SENSOR_HEIGHT}, face_results);

        hand_results.clear();
        hd.pre_process();
        hd.inference();
        // 旋转后图像
        hd.post_process(hand_results);

        cv::Mat osd_frame(osd_height, osd_width, CV_8UC4, cv::Scalar(0, 0, 0, 0));

        float max_area_face = 0;
        int max_id_face = -1;
        for (int i = 0; i < face_results.size(); ++i)
        {
            float area_i = face_results[i].bbox.w * face_results[i].bbox.h;
            if (area_i > max_area_face)
            {
                max_area_face = area_i;
                max_id_face = i;
            }
        }

        float max_area_hand = 0;
        int max_id_hand = -1;
        for (int i = 0; i < hand_results.size(); ++i)
        {
            float area_i = (hand_results[i].x2 - hand_results[i].x1) * (hand_results[i].y2 - hand_results[i].y1);
            if (area_i > max_area_hand)
            {
                max_area_hand = area_i;
                max_id_hand = i;
            }
        }
        
        string text_dms_call = "Calling : No ";
        string text_dms_smoke_drink = "Smoking or Drinking : No ";
        if (max_id_face != -1 && max_id_hand != -1)
        {
            float face_x = face_results[max_id_face].bbox.x + (face_results[max_id_face].bbox.w / 2.0);
            float face_y = face_results[max_id_face].bbox.y + (face_results[max_id_face].bbox.h / 2.0);
            float hand_x = (hand_results[max_id_hand].x1 + hand_results[max_id_hand].x2) / 2.0;
            float hand_y = (hand_results[max_id_hand].y1 + hand_results[max_id_hand].y2) / 2.0;

            now_area = max_area_face;
            now_len = sqrt(pow(face_x - hand_x, 2) + pow(face_y - hand_y, 2));

            if (now_len < (now_area / init_area) * init_len)
            {
                if (face_y < hand_y && (abs(face_y - hand_y) / abs(face_x - hand_x)) > 1)
                {
                    warning_count_smk_drk += 1;
                    warning_count_drk = 0;
                    warning_count_call = 0;
                }
                else if (face_y > hand_y && (abs(face_y - hand_y) / abs(face_x - hand_x)) > 0.8)
                {
                    warning_count_smk_drk = 0;
                    warning_count_drk += 1;
                    warning_count_call = 0;
                }
                else
                {
                    warning_count_smk_drk = 0;
                    warning_count_drk = 0;
                    warning_count_call += 1;
                }

                if (warning_count_smk_drk > warning_amount)
                {
                    text_dms_smoke_drink = "Smoking or Drinking : Yes ";
                    cv::putText(osd_frame, text_dms_call, cv::Point(20,50),cv::FONT_HERSHEY_COMPLEX, 1.5, cv::Scalar(255, 0, 255, 0), 2, 4, 0);
                    cv::putText(osd_frame, text_dms_smoke_drink, cv::Point(20,100),cv::FONT_HERSHEY_COMPLEX, 1.5, cv::Scalar(255, 255, 0, 0), 2, 4, 0);
                }
                else if (warning_count_drk > warning_amount)
                {
                    text_dms_smoke_drink = "Drinking : Yes ";
                    cv::putText(osd_frame, text_dms_call, cv::Point(20,50),cv::FONT_HERSHEY_COMPLEX, 1.5, cv::Scalar(255, 0, 255, 0), 2, 4, 0);
                    cv::putText(osd_frame, text_dms_smoke_drink, cv::Point(20,100),cv::FONT_HERSHEY_COMPLEX, 1.5, cv::Scalar(255, 255, 0, 0), 2, 4, 0);
                }
                else if (warning_count_call > warning_amount)
                {
                    text_dms_call = "Calling : Yes ";
                    cv::putText(osd_frame, text_dms_call, cv::Point(20,50),cv::FONT_HERSHEY_COMPLEX, 1.5, cv::Scalar(255, 255, 0, 0), 2, 4, 0);
                    cv::putText(osd_frame, text_dms_smoke_drink, cv::Point(20,100),cv::FONT_HERSHEY_COMPLEX, 1.5, cv::Scalar(255, 0, 255, 0), 2, 4, 0);
                }
                else
                {
                    cv::putText(osd_frame, text_dms_call, cv::Point(20,50),cv::FONT_HERSHEY_COMPLEX, 1.5, cv::Scalar(255, 0, 255, 0), 2, 4, 0);
                    cv::putText(osd_frame, text_dms_smoke_drink, cv::Point(20,100),cv::FONT_HERSHEY_COMPLEX, 1.5, cv::Scalar(255, 0, 255, 0), 2, 4, 0);
                }
            }
            else
            {
                warning_count_smk_drk = 0;
                warning_count_drk = 0;
                warning_count_call = 0;
                cv::putText(osd_frame, text_dms_call, cv::Point(20,50),cv::FONT_HERSHEY_COMPLEX, 1.5, cv::Scalar(255, 0, 255, 0), 2, 4, 0);
                cv::putText(osd_frame, text_dms_smoke_drink, cv::Point(20,100),cv::FONT_HERSHEY_COMPLEX, 1.5, cv::Scalar(255, 0, 255, 0), 2, 4, 0);
            }


            int w = hand_results[max_id_hand].x2 - hand_results[max_id_hand].x1 + 1;
            int h = hand_results[max_id_hand].y2 - hand_results[max_id_hand].y1 + 1;
            
            int rect_x = hand_results[max_id_hand].x1/ SENSOR_WIDTH * osd_width;
            int rect_y = hand_results[max_id_hand].y1/ SENSOR_HEIGHT * osd_height;
            int rect_w = (float)w / SENSOR_WIDTH * osd_width;
            int rect_h = (float)h / SENSOR_HEIGHT  * osd_height;
            cv::rectangle(osd_frame, cv::Rect(rect_x, rect_y , rect_w, rect_h), cv::Scalar( 255, 0, 255, 255), 6, 2, 0);
        }
        else
        {
            warning_count_smk_drk = 0;
            warning_count_drk = 0;
            warning_count_call = 0;
            cv::putText(osd_frame, text_dms_call, cv::Point(20,50),cv::FONT_HERSHEY_COMPLEX, 1.5, cv::Scalar(255, 0, 255, 0), 2, 4, 0);
            cv::putText(osd_frame, text_dms_smoke_drink, cv::Point(20,100),cv::FONT_HERSHEY_COMPLEX, 1.5, cv::Scalar(255, 0, 255, 0), 2, 4, 0);
        }

        {
            ScopedTiming st("osd copy", atoi(argv[10]));
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
    void *pic_vaddr = NULL;       //osd
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

    FaceDetection fd(argv[1], atof(argv[2]),atof(argv[3]), {SENSOR_CHANNEL, SENSOR_HEIGHT, SENSOR_WIDTH}, reinterpret_cast<uintptr_t>(vaddr), reinterpret_cast<uintptr_t>(paddr), atoi(argv[10]));
    HandDetection hd(argv[4], atof(argv[5]), atof(argv[6]), {SENSOR_WIDTH, SENSOR_HEIGHT}, {SENSOR_CHANNEL, SENSOR_HEIGHT, SENSOR_WIDTH}, reinterpret_cast<uintptr_t>(vaddr), reinterpret_cast<uintptr_t>(paddr), atoi(argv[10]));

    vector<FaceDetectionInfo> face_results;
    vector<BoxInfo> hand_results;

    float init_area = atof(argv[7]);
    float init_len = atof(argv[8]);
    float now_area;
    float now_len;
    int warning_amount = atoi(argv[9]);
    int warning_count_smk_drk = 0;
    int warning_count_drk = 0;
    int warning_count_call = 0;

    while (!isp_stop)
    {
        ScopedTiming st("total time", 1);
        {
            ScopedTiming st("read capture", atoi(argv[10]));
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
            ScopedTiming st("isp copy", atoi(argv[10]));
            auto vbvaddr = kd_mpi_sys_mmap_cached(dump_info.v_frame.phys_addr[0], size);
            memcpy(vaddr, (void *)vbvaddr, SENSOR_HEIGHT * SENSOR_WIDTH * 3);  // 这里以后可以去掉，不用copy
            kd_mpi_sys_munmap(vbvaddr, size);
        }

        face_results.clear();
        fd.pre_process();
        fd.inference();
        // 旋转后图像
        fd.post_process({SENSOR_WIDTH, SENSOR_HEIGHT}, face_results);

        hand_results.clear();
        hd.pre_process();
        hd.inference();
        // 旋转后图像
        hd.post_process(hand_results);

        cv::Mat osd_frame(osd_height, osd_width, CV_8UC4, cv::Scalar(0, 0, 0, 0));

        float max_area_face = 0;
        int max_id_face = -1;
        for (int i = 0; i < face_results.size(); ++i)
        {
            float area_i = face_results[i].bbox.w * face_results[i].bbox.h;
            if (area_i > max_area_face)
            {
                max_area_face = area_i;
                max_id_face = i;
            }
        }

        float max_area_hand = 0;
        int max_id_hand = -1;
        for (int i = 0; i < hand_results.size(); ++i)
        {
            float area_i = (hand_results[i].x2 - hand_results[i].x1) * (hand_results[i].y2 - hand_results[i].y1);
            if (area_i > max_area_hand)
            {
                max_area_hand = area_i;
                max_id_hand = i;
            }
        }
        
        string text_dms_call = "Calling : No ";
        string text_dms_smoke_drink = "Smoking or Drinking : No ";
        if (max_id_face != -1 && max_id_hand != -1)
        {
            float face_x = face_results[max_id_face].bbox.x + (face_results[max_id_face].bbox.w / 2.0);
            float face_y = face_results[max_id_face].bbox.y + (face_results[max_id_face].bbox.h / 2.0);
            float hand_x = (hand_results[max_id_hand].x1 + hand_results[max_id_hand].x2) / 2.0;
            float hand_y = (hand_results[max_id_hand].y1 + hand_results[max_id_hand].y2) / 2.0;

            now_area = max_area_face;
            now_len = sqrt(pow(face_x - hand_x, 2) + pow(face_y - hand_y, 2));

            if (now_len < (now_area / init_area) * init_len)
            {
                if (face_y < hand_y && (abs(face_y - hand_y) / abs(face_x - hand_x)) > 1)
                {
                    warning_count_smk_drk += 1;
                    warning_count_drk = 0;
                    warning_count_call = 0;
                }
                else if (face_y > hand_y && (abs(face_y - hand_y) / abs(face_x - hand_x)) > 0.8)
                {
                    warning_count_smk_drk = 0;
                    warning_count_drk += 1;
                    warning_count_call = 0;
                }
                else
                {
                    warning_count_smk_drk = 0;
                    warning_count_drk = 0;
                    warning_count_call += 1;
                }

                if (warning_count_smk_drk > warning_amount)
                {
                    text_dms_smoke_drink = "Smoking or Drinking : Yes ";
                    cv::putText(osd_frame, text_dms_call, cv::Point(20,50),cv::FONT_HERSHEY_COMPLEX, 1.5, cv::Scalar(255, 0, 255, 0), 2, 4, 0);
                    cv::putText(osd_frame, text_dms_smoke_drink, cv::Point(20,100),cv::FONT_HERSHEY_COMPLEX, 1.5, cv::Scalar(255, 255, 0, 0), 2, 4, 0);
                }
                else if (warning_count_drk > warning_amount)
                {
                    text_dms_smoke_drink = "Drinking : Yes ";
                    cv::putText(osd_frame, text_dms_call, cv::Point(20,50),cv::FONT_HERSHEY_COMPLEX, 1.5, cv::Scalar(255, 0, 255, 0), 2, 4, 0);
                    cv::putText(osd_frame, text_dms_smoke_drink, cv::Point(20,100),cv::FONT_HERSHEY_COMPLEX, 1.5, cv::Scalar(255, 255, 0, 0), 2, 4, 0);
                }
                else if (warning_count_call > warning_amount)
                {
                    text_dms_call = "Calling : Yes ";
                    cv::putText(osd_frame, text_dms_call, cv::Point(20,50),cv::FONT_HERSHEY_COMPLEX, 1.5, cv::Scalar(255, 255, 0, 0), 2, 4, 0);
                    cv::putText(osd_frame, text_dms_smoke_drink, cv::Point(20,100),cv::FONT_HERSHEY_COMPLEX, 1.5, cv::Scalar(255, 0, 255, 0), 2, 4, 0);
                }
                else
                {
                    cv::putText(osd_frame, text_dms_call, cv::Point(20,50),cv::FONT_HERSHEY_COMPLEX, 1.5, cv::Scalar(255, 0, 255, 0), 2, 4, 0);
                    cv::putText(osd_frame, text_dms_smoke_drink, cv::Point(20,100),cv::FONT_HERSHEY_COMPLEX, 1.5, cv::Scalar(255, 0, 255, 0), 2, 4, 0);
                }
            }
            else
            {
                warning_count_smk_drk = 0;
                warning_count_drk = 0;
                warning_count_call = 0;
                cv::putText(osd_frame, text_dms_call, cv::Point(20,50),cv::FONT_HERSHEY_COMPLEX, 1.5, cv::Scalar(255, 0, 255, 0), 2, 4, 0);
                cv::putText(osd_frame, text_dms_smoke_drink, cv::Point(20,100),cv::FONT_HERSHEY_COMPLEX, 1.5, cv::Scalar(255, 0, 255, 0), 2, 4, 0);
            }


            int w = hand_results[max_id_hand].x2 - hand_results[max_id_hand].x1 + 1;
            int h = hand_results[max_id_hand].y2 - hand_results[max_id_hand].y1 + 1;
            
            int rect_x = hand_results[max_id_hand].x1/ SENSOR_WIDTH * osd_width;
            int rect_y = hand_results[max_id_hand].y1/ SENSOR_HEIGHT * osd_height;
            int rect_w = (float)w / SENSOR_WIDTH * osd_width;
            int rect_h = (float)h / SENSOR_HEIGHT  * osd_height;
            cv::rectangle(osd_frame, cv::Rect(rect_x, rect_y , rect_w, rect_h), cv::Scalar( 255, 0, 255, 255), 6, 2, 0);
        }
        else
        {
            warning_count_smk_drk = 0;
            warning_count_drk = 0;
            warning_count_call = 0;
            cv::putText(osd_frame, text_dms_call, cv::Point(20,50),cv::FONT_HERSHEY_COMPLEX, 1.5, cv::Scalar(255, 0, 255, 0), 2, 4, 0);
            cv::putText(osd_frame, text_dms_smoke_drink, cv::Point(20,100),cv::FONT_HERSHEY_COMPLEX, 1.5, cv::Scalar(255, 0, 255, 0), 2, 4, 0);
        }

        {
            ScopedTiming st("osd copy", atoi(argv[10]));
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
    void *pic_vaddr = NULL;       //osd
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

    FaceDetection fd(argv[1], atof(argv[2]),atof(argv[3]), {SENSOR_CHANNEL, SENSOR_HEIGHT, SENSOR_WIDTH}, reinterpret_cast<uintptr_t>(vaddr), reinterpret_cast<uintptr_t>(paddr), atoi(argv[10]));
    HandDetection hd(argv[4], atof(argv[5]), atof(argv[6]), {SENSOR_WIDTH, SENSOR_HEIGHT}, {SENSOR_CHANNEL, SENSOR_HEIGHT, SENSOR_WIDTH}, reinterpret_cast<uintptr_t>(vaddr), reinterpret_cast<uintptr_t>(paddr), atoi(argv[10]));

    vector<FaceDetectionInfo> face_results;
    vector<BoxInfo> hand_results;

    float init_area = atof(argv[7]);
    float init_len = atof(argv[8]);
    float now_area;
    float now_len;
    int warning_amount = atoi(argv[9]);
    int warning_count_smk_drk = 0;
    int warning_count_drk = 0;
    int warning_count_call = 0;

    while (!isp_stop)
    {
        ScopedTiming st("total time", 1);
        {
            ScopedTiming st("read capture", atoi(argv[10]));
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
            ScopedTiming st("isp copy", atoi(argv[10]));
            auto vbvaddr = kd_mpi_sys_mmap_cached(dump_info.v_frame.phys_addr[0], size);
            memcpy(vaddr, (void *)vbvaddr, SENSOR_HEIGHT * SENSOR_WIDTH * 3);  // 这里以后可以去掉，不用copy
            kd_mpi_sys_munmap(vbvaddr, size);
        }

        face_results.clear();
        fd.pre_process();
        fd.inference();
        // 旋转后图像
        fd.post_process({SENSOR_WIDTH, SENSOR_HEIGHT}, face_results);

        hand_results.clear();
        hd.pre_process();
        hd.inference();
        // 旋转后图像
        hd.post_process(hand_results);

        cv::Mat osd_frame(osd_height, osd_width, CV_8UC4, cv::Scalar(0, 0, 0, 0));
        cv::rotate(osd_frame, osd_frame, cv::ROTATE_90_COUNTERCLOCKWISE);

        float max_area_face = 0;
        int max_id_face = -1;
        for (int i = 0; i < face_results.size(); ++i)
        {
            float area_i = face_results[i].bbox.w * face_results[i].bbox.h;
            if (area_i > max_area_face)
            {
                max_area_face = area_i;
                max_id_face = i;
            }
        }

        float max_area_hand = 0;
        int max_id_hand = -1;
        for (int i = 0; i < hand_results.size(); ++i)
        {
            float area_i = (hand_results[i].x2 - hand_results[i].x1) * (hand_results[i].y2 - hand_results[i].y1);
            if (area_i > max_area_hand)
            {
                max_area_hand = area_i;
                max_id_hand = i;
            }
        }
        
        string text_dms_call = "Calling : No ";
        string text_dms_smoke_drink = "Smoking or Drinking : No ";
        if (max_id_face != -1 && max_id_hand != -1)
        {
            float face_x = face_results[max_id_face].bbox.x + (face_results[max_id_face].bbox.w / 2.0);
            float face_y = face_results[max_id_face].bbox.y + (face_results[max_id_face].bbox.h / 2.0);
            float hand_x = (hand_results[max_id_hand].x1 + hand_results[max_id_hand].x2) / 2.0;
            float hand_y = (hand_results[max_id_hand].y1 + hand_results[max_id_hand].y2) / 2.0;

            now_area = max_area_face;
            now_len = sqrt(pow(face_x - hand_x, 2) + pow(face_y - hand_y, 2));

            if (now_len < (now_area / init_area) * init_len)
            {
                if (face_y < hand_y && (abs(face_y - hand_y) / abs(face_x - hand_x)) > 1)
                {
                    warning_count_smk_drk += 1;
                    warning_count_drk = 0;
                    warning_count_call = 0;
                }
                else if (face_y > hand_y && (abs(face_y - hand_y) / abs(face_x - hand_x)) > 0.8)
                {
                    warning_count_smk_drk = 0;
                    warning_count_drk += 1;
                    warning_count_call = 0;
                }
                else
                {
                    warning_count_smk_drk = 0;
                    warning_count_drk = 0;
                    warning_count_call += 1;
                }

                if (warning_count_smk_drk > warning_amount)
                {
                    text_dms_smoke_drink = "Smoking or Drinking : Yes ";
                    cv::putText(osd_frame, text_dms_call, cv::Point(20,50),cv::FONT_HERSHEY_COMPLEX, 1.5, cv::Scalar(255, 0, 255, 0), 2, 4, 0);
                    cv::putText(osd_frame, text_dms_smoke_drink, cv::Point(20,100),cv::FONT_HERSHEY_COMPLEX, 1.5, cv::Scalar(255, 255, 0, 0), 2, 4, 0);
                }
                else if (warning_count_drk > warning_amount)
                {
                    text_dms_smoke_drink = "Drinking : Yes ";
                    cv::putText(osd_frame, text_dms_call, cv::Point(20,50),cv::FONT_HERSHEY_COMPLEX, 1.5, cv::Scalar(255, 0, 255, 0), 2, 4, 0);
                    cv::putText(osd_frame, text_dms_smoke_drink, cv::Point(20,100),cv::FONT_HERSHEY_COMPLEX, 1.5, cv::Scalar(255, 255, 0, 0), 2, 4, 0);
                }
                else if (warning_count_call > warning_amount)
                {
                    text_dms_call = "Calling : Yes ";
                    cv::putText(osd_frame, text_dms_call, cv::Point(20,50),cv::FONT_HERSHEY_COMPLEX, 1.5, cv::Scalar(255, 255, 0, 0), 2, 4, 0);
                    cv::putText(osd_frame, text_dms_smoke_drink, cv::Point(20,100),cv::FONT_HERSHEY_COMPLEX, 1.5, cv::Scalar(255, 0, 255, 0), 2, 4, 0);
                }
                else
                {
                    cv::putText(osd_frame, text_dms_call, cv::Point(20,50),cv::FONT_HERSHEY_COMPLEX, 1.5, cv::Scalar(255, 0, 255, 0), 2, 4, 0);
                    cv::putText(osd_frame, text_dms_smoke_drink, cv::Point(20,100),cv::FONT_HERSHEY_COMPLEX, 1.5, cv::Scalar(255, 0, 255, 0), 2, 4, 0);
                }
            }
            else
            {
                warning_count_smk_drk = 0;
                warning_count_drk = 0;
                warning_count_call = 0;
                cv::putText(osd_frame, text_dms_call, cv::Point(20,50),cv::FONT_HERSHEY_COMPLEX, 1.5, cv::Scalar(255, 0, 255, 0), 2, 4, 0);
                cv::putText(osd_frame, text_dms_smoke_drink, cv::Point(20,100),cv::FONT_HERSHEY_COMPLEX, 1.5, cv::Scalar(255, 0, 255, 0), 2, 4, 0);
            }


            int w = hand_results[max_id_hand].x2 - hand_results[max_id_hand].x1 + 1;
            int h = hand_results[max_id_hand].y2 - hand_results[max_id_hand].y1 + 1;
            
            int rect_x = hand_results[max_id_hand].x1/ SENSOR_WIDTH * osd_frame.cols;
            int rect_y = hand_results[max_id_hand].y1/ SENSOR_HEIGHT * osd_frame.rows;
            int rect_w = (float)w / SENSOR_WIDTH * osd_frame.cols;
            int rect_h = (float)h / SENSOR_HEIGHT  * osd_frame.rows;
            cv::rectangle(osd_frame, cv::Rect(rect_x, rect_y , rect_w, rect_h), cv::Scalar( 255, 0, 255, 255), 6, 2, 0);
        }
        else
        {
            warning_count_smk_drk = 0;
            warning_count_drk = 0;
            warning_count_call = 0;
            cv::putText(osd_frame, text_dms_call, cv::Point(20,50),cv::FONT_HERSHEY_COMPLEX, 1.5, cv::Scalar(255, 0, 255, 0), 2, 4, 0);
            cv::putText(osd_frame, text_dms_smoke_drink, cv::Point(20,100),cv::FONT_HERSHEY_COMPLEX, 1.5, cv::Scalar(255, 0, 255, 0), 2, 4, 0);
        }
        cv::rotate(osd_frame, osd_frame, cv::ROTATE_90_CLOCKWISE);

        {
            ScopedTiming st("osd copy", atoi(argv[10]));
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
    void *pic_vaddr = NULL;       //osd
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

    FaceDetection fd(argv[1], atof(argv[2]),atof(argv[3]), {SENSOR_CHANNEL, SENSOR_HEIGHT, SENSOR_WIDTH}, reinterpret_cast<uintptr_t>(vaddr), reinterpret_cast<uintptr_t>(paddr), atoi(argv[10]));
    HandDetection hd(argv[4], atof(argv[5]), atof(argv[6]), {SENSOR_WIDTH, SENSOR_HEIGHT}, {SENSOR_CHANNEL, SENSOR_HEIGHT, SENSOR_WIDTH}, reinterpret_cast<uintptr_t>(vaddr), reinterpret_cast<uintptr_t>(paddr), atoi(argv[10]));

    vector<FaceDetectionInfo> face_results;
    vector<BoxInfo> hand_results;

    float init_area = atof(argv[7]);
    float init_len = atof(argv[8]);
    float now_area;
    float now_len;
    int warning_amount = atoi(argv[9]);
    int warning_count_smk_drk = 0;
    int warning_count_drk = 0;
    int warning_count_call = 0;

    while (!isp_stop)
    {
        ScopedTiming st("total time", 1);
        {
            ScopedTiming st("read capture", atoi(argv[10]));
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
            ScopedTiming st("isp copy", atoi(argv[10]));
            auto vbvaddr = kd_mpi_sys_mmap_cached(dump_info.v_frame.phys_addr[0], size);
            memcpy(vaddr, (void *)vbvaddr, SENSOR_HEIGHT * SENSOR_WIDTH * 3);  // 这里以后可以去掉，不用copy
            kd_mpi_sys_munmap(vbvaddr, size);
        }

        face_results.clear();
        fd.pre_process();
        fd.inference();
        // 旋转后图像
        fd.post_process({SENSOR_WIDTH, SENSOR_HEIGHT}, face_results);

        hand_results.clear();
        hd.pre_process();
        hd.inference();
        // 旋转后图像
        hd.post_process(hand_results);

        cv::Mat osd_frame(osd_height, osd_width, CV_8UC4, cv::Scalar(0, 0, 0, 0));
        

        #if defined(STUDIO_HDMI)
        {
            float max_area_face = 0;
            int max_id_face = -1;
            for (int i = 0; i < face_results.size(); ++i)
            {
                float area_i = face_results[i].bbox.w * face_results[i].bbox.h;
                if (area_i > max_area_face)
                {
                    max_area_face = area_i;
                    max_id_face = i;
                }
            }

            float max_area_hand = 0;
            int max_id_hand = -1;
            for (int i = 0; i < hand_results.size(); ++i)
            {
                float area_i = (hand_results[i].x2 - hand_results[i].x1) * (hand_results[i].y2 - hand_results[i].y1);
                if (area_i > max_area_hand)
                {
                    max_area_hand = area_i;
                    max_id_hand = i;
                }
            }
            
            string text_dms_call = "Calling : No ";
            string text_dms_smoke_drink = "Smoking or Drinking : No ";
            if (max_id_face != -1 && max_id_hand != -1)
            {
                float face_x = face_results[max_id_face].bbox.x + (face_results[max_id_face].bbox.w / 2.0);
                float face_y = face_results[max_id_face].bbox.y + (face_results[max_id_face].bbox.h / 2.0);
                float hand_x = (hand_results[max_id_hand].x1 + hand_results[max_id_hand].x2) / 2.0;
                float hand_y = (hand_results[max_id_hand].y1 + hand_results[max_id_hand].y2) / 2.0;

                now_area = max_area_face;
                now_len = sqrt(pow(face_x - hand_x, 2) + pow(face_y - hand_y, 2));

                if (now_len < (now_area / init_area) * init_len)
                {
                    if (face_y < hand_y && (abs(face_y - hand_y) / abs(face_x - hand_x)) > 1)
                    {
                        warning_count_smk_drk += 1;
                        warning_count_drk = 0;
                        warning_count_call = 0;
                    }
                    else if (face_y > hand_y && (abs(face_y - hand_y) / abs(face_x - hand_x)) > 0.8)
                    {
                        warning_count_smk_drk = 0;
                        warning_count_drk += 1;
                        warning_count_call = 0;
                    }
                    else
                    {
                        warning_count_smk_drk = 0;
                        warning_count_drk = 0;
                        warning_count_call += 1;
                    }

                    if (warning_count_smk_drk > warning_amount)
                    {
                        text_dms_smoke_drink = "Smoking or Drinking : Yes ";
                        cv::putText(osd_frame, text_dms_call, cv::Point(20,50),cv::FONT_HERSHEY_COMPLEX, 1.5, cv::Scalar(255, 0, 255, 0), 2, 4, 0);
                        cv::putText(osd_frame, text_dms_smoke_drink, cv::Point(20,100),cv::FONT_HERSHEY_COMPLEX, 1.5, cv::Scalar(255, 255, 0, 0), 2, 4, 0);
                    }
                    else if (warning_count_drk > warning_amount)
                    {
                        text_dms_smoke_drink = "Drinking : Yes ";
                        cv::putText(osd_frame, text_dms_call, cv::Point(20,50),cv::FONT_HERSHEY_COMPLEX, 1.5, cv::Scalar(255, 0, 255, 0), 2, 4, 0);
                        cv::putText(osd_frame, text_dms_smoke_drink, cv::Point(20,100),cv::FONT_HERSHEY_COMPLEX, 1.5, cv::Scalar(255, 255, 0, 0), 2, 4, 0);
                    }
                    else if (warning_count_call > warning_amount)
                    {
                        text_dms_call = "Calling : Yes ";
                        cv::putText(osd_frame, text_dms_call, cv::Point(20,50),cv::FONT_HERSHEY_COMPLEX, 1.5, cv::Scalar(255, 255, 0, 0), 2, 4, 0);
                        cv::putText(osd_frame, text_dms_smoke_drink, cv::Point(20,100),cv::FONT_HERSHEY_COMPLEX, 1.5, cv::Scalar(255, 0, 255, 0), 2, 4, 0);
                    }
                    else
                    {
                        cv::putText(osd_frame, text_dms_call, cv::Point(20,50),cv::FONT_HERSHEY_COMPLEX, 1.5, cv::Scalar(255, 0, 255, 0), 2, 4, 0);
                        cv::putText(osd_frame, text_dms_smoke_drink, cv::Point(20,100),cv::FONT_HERSHEY_COMPLEX, 1.5, cv::Scalar(255, 0, 255, 0), 2, 4, 0);
                    }
                }
                else
                {
                    warning_count_smk_drk = 0;
                    warning_count_drk = 0;
                    warning_count_call = 0;
                    cv::putText(osd_frame, text_dms_call, cv::Point(20,50),cv::FONT_HERSHEY_COMPLEX, 1.5, cv::Scalar(255, 0, 255, 0), 2, 4, 0);
                    cv::putText(osd_frame, text_dms_smoke_drink, cv::Point(20,100),cv::FONT_HERSHEY_COMPLEX, 1.5, cv::Scalar(255, 0, 255, 0), 2, 4, 0);
                }


                int w = hand_results[max_id_hand].x2 - hand_results[max_id_hand].x1 + 1;
                int h = hand_results[max_id_hand].y2 - hand_results[max_id_hand].y1 + 1;
                
                int rect_x = hand_results[max_id_hand].x1/ SENSOR_WIDTH * osd_frame.cols;
                int rect_y = hand_results[max_id_hand].y1/ SENSOR_HEIGHT * osd_frame.rows;
                int rect_w = (float)w / SENSOR_WIDTH * osd_frame.cols;
                int rect_h = (float)h / SENSOR_HEIGHT  * osd_frame.rows;
                cv::rectangle(osd_frame, cv::Rect(rect_x, rect_y , rect_w, rect_h), cv::Scalar( 255, 0, 255, 255), 6, 2, 0);
            }
            else
            {
                warning_count_smk_drk = 0;
                warning_count_drk = 0;
                warning_count_call = 0;
                cv::putText(osd_frame, text_dms_call, cv::Point(20,50),cv::FONT_HERSHEY_COMPLEX, 1.5, cv::Scalar(255, 0, 255, 0), 2, 4, 0);
                cv::putText(osd_frame, text_dms_smoke_drink, cv::Point(20,100),cv::FONT_HERSHEY_COMPLEX, 1.5, cv::Scalar(255, 0, 255, 0), 2, 4, 0);
            }
        }
        #else
        {
            cv::rotate(osd_frame, osd_frame, cv::ROTATE_90_COUNTERCLOCKWISE);

            float max_area_face = 0;
            int max_id_face = -1;
            for (int i = 0; i < face_results.size(); ++i)
            {
                float area_i = face_results[i].bbox.w * face_results[i].bbox.h;
                if (area_i > max_area_face)
                {
                    max_area_face = area_i;
                    max_id_face = i;
                }
            }

            float max_area_hand = 0;
            int max_id_hand = -1;
            for (int i = 0; i < hand_results.size(); ++i)
            {
                float area_i = (hand_results[i].x2 - hand_results[i].x1) * (hand_results[i].y2 - hand_results[i].y1);
                if (area_i > max_area_hand)
                {
                    max_area_hand = area_i;
                    max_id_hand = i;
                }
            }
            
            string text_dms_call = "Calling : No ";
            string text_dms_smoke_drink = "Smoking or Drinking : No ";
            if (max_id_face != -1 && max_id_hand != -1)
            {
                float face_x = face_results[max_id_face].bbox.x + (face_results[max_id_face].bbox.w / 2.0);
                float face_y = face_results[max_id_face].bbox.y + (face_results[max_id_face].bbox.h / 2.0);
                float hand_x = (hand_results[max_id_hand].x1 + hand_results[max_id_hand].x2) / 2.0;
                float hand_y = (hand_results[max_id_hand].y1 + hand_results[max_id_hand].y2) / 2.0;

                now_area = max_area_face;
                now_len = sqrt(pow(face_x - hand_x, 2) + pow(face_y - hand_y, 2));

                if (now_len < (now_area / init_area) * init_len)
                {
                    if (face_y < hand_y && (abs(face_y - hand_y) / abs(face_x - hand_x)) > 1)
                    {
                        warning_count_smk_drk += 1;
                        warning_count_drk = 0;
                        warning_count_call = 0;
                    }
                    else if (face_y > hand_y && (abs(face_y - hand_y) / abs(face_x - hand_x)) > 0.8)
                    {
                        warning_count_smk_drk = 0;
                        warning_count_drk += 1;
                        warning_count_call = 0;
                    }
                    else
                    {
                        warning_count_smk_drk = 0;
                        warning_count_drk = 0;
                        warning_count_call += 1;
                    }

                    if (warning_count_smk_drk > warning_amount)
                    {
                        text_dms_smoke_drink = "Smoking or Drinking : Yes ";
                        cv::putText(osd_frame, text_dms_call, cv::Point(20,50),cv::FONT_HERSHEY_COMPLEX, 1.5, cv::Scalar(255, 0, 255, 0), 2, 4, 0);
                        cv::putText(osd_frame, text_dms_smoke_drink, cv::Point(20,100),cv::FONT_HERSHEY_COMPLEX, 1.5, cv::Scalar(255, 255, 0, 0), 2, 4, 0);
                    }
                    else if (warning_count_drk > warning_amount)
                    {
                        text_dms_smoke_drink = "Drinking : Yes ";
                        cv::putText(osd_frame, text_dms_call, cv::Point(20,50),cv::FONT_HERSHEY_COMPLEX, 1.5, cv::Scalar(255, 0, 255, 0), 2, 4, 0);
                        cv::putText(osd_frame, text_dms_smoke_drink, cv::Point(20,100),cv::FONT_HERSHEY_COMPLEX, 1.5, cv::Scalar(255, 255, 0, 0), 2, 4, 0);
                    }
                    else if (warning_count_call > warning_amount)
                    {
                        text_dms_call = "Calling : Yes ";
                        cv::putText(osd_frame, text_dms_call, cv::Point(20,50),cv::FONT_HERSHEY_COMPLEX, 1.5, cv::Scalar(255, 255, 0, 0), 2, 4, 0);
                        cv::putText(osd_frame, text_dms_smoke_drink, cv::Point(20,100),cv::FONT_HERSHEY_COMPLEX, 1.5, cv::Scalar(255, 0, 255, 0), 2, 4, 0);
                    }
                    else
                    {
                        cv::putText(osd_frame, text_dms_call, cv::Point(20,50),cv::FONT_HERSHEY_COMPLEX, 1.5, cv::Scalar(255, 0, 255, 0), 2, 4, 0);
                        cv::putText(osd_frame, text_dms_smoke_drink, cv::Point(20,100),cv::FONT_HERSHEY_COMPLEX, 1.5, cv::Scalar(255, 0, 255, 0), 2, 4, 0);
                    }
                }
                else
                {
                    warning_count_smk_drk = 0;
                    warning_count_drk = 0;
                    warning_count_call = 0;
                    cv::putText(osd_frame, text_dms_call, cv::Point(20,50),cv::FONT_HERSHEY_COMPLEX, 1.5, cv::Scalar(255, 0, 255, 0), 2, 4, 0);
                    cv::putText(osd_frame, text_dms_smoke_drink, cv::Point(20,100),cv::FONT_HERSHEY_COMPLEX, 1.5, cv::Scalar(255, 0, 255, 0), 2, 4, 0);
                }


                int w = hand_results[max_id_hand].x2 - hand_results[max_id_hand].x1 + 1;
                int h = hand_results[max_id_hand].y2 - hand_results[max_id_hand].y1 + 1;
                
                int rect_x = hand_results[max_id_hand].x1/ SENSOR_WIDTH * osd_frame.cols;
                int rect_y = hand_results[max_id_hand].y1/ SENSOR_HEIGHT * osd_frame.rows;
                int rect_w = (float)w / SENSOR_WIDTH * osd_frame.cols;
                int rect_h = (float)h / SENSOR_HEIGHT  * osd_frame.rows;
                cv::rectangle(osd_frame, cv::Rect(rect_x, rect_y , rect_w, rect_h), cv::Scalar( 255, 0, 255, 255), 6, 2, 0);
            }
            else
            {
                warning_count_smk_drk = 0;
                warning_count_drk = 0;
                warning_count_call = 0;
                cv::putText(osd_frame, text_dms_call, cv::Point(20,50),cv::FONT_HERSHEY_COMPLEX, 1.5, cv::Scalar(255, 0, 255, 0), 2, 4, 0);
                cv::putText(osd_frame, text_dms_smoke_drink, cv::Point(20,100),cv::FONT_HERSHEY_COMPLEX, 1.5, cv::Scalar(255, 0, 255, 0), 2, 4, 0);
            }
            cv::rotate(osd_frame, osd_frame, cv::ROTATE_90_CLOCKWISE);
        }
        #endif

        {
            ScopedTiming st("osd copy", atoi(argv[10]));
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
    if (argc != 11)
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