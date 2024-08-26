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
#include "face_pose.h"

using std::cerr;
using std::cout;
using std::endl;

std::atomic<bool> isp_stop(false);

void print_usage(const char *name)
{
    cout << "Usage: " << name << "<kmodel_det> <obj_thres> <nms_thres> <kmodel_fp> <warning_amount> <warning_angle_roll> <warning_angle_yaw> <warning_angle_pitch> <debug_mode>" << endl
         << "Options:" << endl
         << "  kmodel_det           人脸检测kmodel路径\n"
         << "  obj_thres            人脸检测阈值\n"
         << "  nms_thres            人脸检测nms阈值\n"
         << "  kmodel_fp            人脸姿态估计kmodel路径\n"
         << "  warning_amount       达到需要提醒注意的帧数\n"
         << "  warning_angle_roll   达到需要提醒注意的滚转角偏离角度\n"
         << "  warning_angle_yaw    达到需要提醒注意的偏航角偏离角度\n"
         << "  warning_angle_pitch  达到需要提醒注意的俯仰角偏离角度\n"
         << "  debug_mode           是否需要调试，0、1、2分别表示不调试、简单调试、详细调试\n"
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

    FaceDetection fd(argv[1], atof(argv[2]),atof(argv[3]), {SENSOR_CHANNEL, SENSOR_HEIGHT, SENSOR_WIDTH}, reinterpret_cast<uintptr_t>(vaddr), reinterpret_cast<uintptr_t>(paddr), atoi(argv[9]));
    FacePose fp(argv[4], {SENSOR_CHANNEL, SENSOR_HEIGHT, SENSOR_WIDTH}, reinterpret_cast<uintptr_t>(vaddr), reinterpret_cast<uintptr_t>(paddr), atoi(argv[9]));

    vector<FaceDetectionInfo> det_results;
    FacePoseInfo pose_result;
    int warning_amount = atoi(argv[5]);
    float warning_angle_roll = atof(argv[6]);
    float warning_angle_yaw = atof(argv[7]);
    float warning_angle_pitch = atof(argv[8]);
    int warning_count = 0;
    while (!isp_stop)
    {
        ScopedTiming st("total time", 1);

        {
            ScopedTiming st("read capture", atoi(argv[9]));
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
            ScopedTiming st("isp copy", atoi(argv[9]));
            auto vbvaddr = kd_mpi_sys_mmap_cached(dump_info.v_frame.phys_addr[0], size);
            memcpy(vaddr, (void *)vbvaddr, SENSOR_HEIGHT * SENSOR_WIDTH * 3);  // 这里以后可以去掉，不用copy
            kd_mpi_sys_munmap(vbvaddr, size);
        }

        det_results.clear();

        fd.pre_process();
        fd.inference();
        // 旋转后图像
        fd.post_process({SENSOR_WIDTH, SENSOR_HEIGHT}, det_results);

        cv::Mat osd_frame(osd_height, osd_width, CV_8UC4, cv::Scalar(0, 0, 0, 0));

        float max_area_face = 0;
        int max_id_face = -1;
        for (int i = 0; i < det_results.size(); ++i)
        {
            float area_i = det_results[i].bbox.w * det_results[i].bbox.h;
            if (area_i > max_area_face)
            {
                max_area_face = area_i;
                max_id_face = i;
            }
        }

        if (max_id_face != -1)
        {
            fp.pre_process(det_results[max_id_face].bbox);
            fp.inference();
            fp.post_process(pose_result);
            {
                ScopedTiming st("osd draw", atoi(argv[9]));
                fp.draw_result(osd_frame,det_results[max_id_face].bbox,pose_result,false);
            }
            if (std::abs(pose_result.roll) > warning_angle_roll || std::abs(pose_result.yaw) > warning_angle_yaw || std::abs(pose_result.pitch) > warning_angle_pitch)
            {
                warning_count += 1; 
            }
            else
            {
                warning_count = 0;
            }
            if (warning_count > warning_amount)
            {
                cv::putText(osd_frame, " Please look straight ahead ! ", {0, 100}, cv::FONT_HERSHEY_COMPLEX, 2, cv::Scalar(255, 255, 0, 0), 2, 4, 0);
            }
        }

        {
            ScopedTiming st("osd copy", atoi(argv[9]));
            memcpy(pic_vaddr, osd_frame.data, osd_width * osd_height * 4);
            // 显示通道插入帧
            kd_mpi_vo_chn_insert_frame(osd_id + 3, &vf_info); // K_VO_OSD0
            // printf("kd_mpi_vo_chn_insert_frame success \n");

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

    FaceDetection fd(argv[1], atof(argv[2]),atof(argv[3]), {SENSOR_CHANNEL, SENSOR_HEIGHT, SENSOR_WIDTH}, reinterpret_cast<uintptr_t>(vaddr), reinterpret_cast<uintptr_t>(paddr), atoi(argv[9]));
    FacePose fp(argv[4], {SENSOR_CHANNEL, SENSOR_HEIGHT, SENSOR_WIDTH}, reinterpret_cast<uintptr_t>(vaddr), reinterpret_cast<uintptr_t>(paddr), atoi(argv[9]));

    vector<FaceDetectionInfo> det_results;
    FacePoseInfo pose_result;
    int warning_amount = atoi(argv[5]);
    float warning_angle_roll = atof(argv[6]);
    float warning_angle_yaw = atof(argv[7]);
    float warning_angle_pitch = atof(argv[8]);
    int warning_count = 0;
    while (!isp_stop)
    {
        ScopedTiming st("total time", 1);

        {
            ScopedTiming st("read capture", atoi(argv[9]));
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
            ScopedTiming st("isp copy", atoi(argv[9]));
            auto vbvaddr = kd_mpi_sys_mmap_cached(dump_info.v_frame.phys_addr[0], size);
            memcpy(vaddr, (void *)vbvaddr, SENSOR_HEIGHT * SENSOR_WIDTH * 3);  // 这里以后可以去掉，不用copy
            kd_mpi_sys_munmap(vbvaddr, size);
        }

        det_results.clear();

        fd.pre_process();
        fd.inference();
        // 旋转后图像
        fd.post_process({SENSOR_WIDTH, SENSOR_HEIGHT}, det_results);

        cv::Mat osd_frame(osd_height, osd_width, CV_8UC4, cv::Scalar(0, 0, 0, 0));

        float max_area_face = 0;
        int max_id_face = -1;
        for (int i = 0; i < det_results.size(); ++i)
        {
            float area_i = det_results[i].bbox.w * det_results[i].bbox.h;
            if (area_i > max_area_face)
            {
                max_area_face = area_i;
                max_id_face = i;
            }
        }

        if (max_id_face != -1)
        {
            fp.pre_process(det_results[max_id_face].bbox);
            fp.inference();
            fp.post_process(pose_result);
            {
                ScopedTiming st("osd draw", atoi(argv[9]));
                fp.draw_result(osd_frame,det_results[max_id_face].bbox,pose_result,false);
            }
            if (std::abs(pose_result.roll) > warning_angle_roll || std::abs(pose_result.yaw) > warning_angle_yaw || std::abs(pose_result.pitch) > warning_angle_pitch)
            {
                warning_count += 1; 
            }
            else
            {
                warning_count = 0;
            }
            if (warning_count > warning_amount)
            {
                cv::putText(osd_frame, " Please look straight ahead ! ", {0, 100}, cv::FONT_HERSHEY_COMPLEX, 2, cv::Scalar(255, 255, 0, 0), 2, 4, 0);
            }
        }

        {
            ScopedTiming st("osd copy", atoi(argv[9]));
            memcpy(pic_vaddr, osd_frame.data, osd_width * osd_height * 4);
            // 显示通道插入帧
            kd_mpi_vo_chn_insert_frame(osd_id + 3, &vf_info); // K_VO_OSD0
            // printf("kd_mpi_vo_chn_insert_frame success \n");

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

    FaceDetection fd(argv[1], atof(argv[2]),atof(argv[3]), {SENSOR_CHANNEL, SENSOR_HEIGHT, SENSOR_WIDTH}, reinterpret_cast<uintptr_t>(vaddr), reinterpret_cast<uintptr_t>(paddr), atoi(argv[9]));
    FacePose fp(argv[4], {SENSOR_CHANNEL, SENSOR_HEIGHT, SENSOR_WIDTH}, reinterpret_cast<uintptr_t>(vaddr), reinterpret_cast<uintptr_t>(paddr), atoi(argv[9]));

    vector<FaceDetectionInfo> det_results;
    FacePoseInfo pose_result;
    int warning_amount = atoi(argv[5]);
    float warning_angle_roll = atof(argv[6]);
    float warning_angle_yaw = atof(argv[7]);
    float warning_angle_pitch = atof(argv[8]);
    int warning_count = 0;
    while (!isp_stop)
    {
        ScopedTiming st("total time", 1);

        {
            ScopedTiming st("read capture", atoi(argv[9]));
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
            ScopedTiming st("isp copy", atoi(argv[9]));
            auto vbvaddr = kd_mpi_sys_mmap_cached(dump_info.v_frame.phys_addr[0], size);
            memcpy(vaddr, (void *)vbvaddr, SENSOR_HEIGHT * SENSOR_WIDTH * 3);  // 这里以后可以去掉，不用copy
            kd_mpi_sys_munmap(vbvaddr, size);
        }

        det_results.clear();

        fd.pre_process();
        fd.inference();
        // 旋转后图像
        fd.post_process({SENSOR_WIDTH, SENSOR_HEIGHT}, det_results);

        cv::Mat osd_frame(osd_height, osd_width, CV_8UC4, cv::Scalar(0, 0, 0, 0));
        cv::rotate(osd_frame, osd_frame, cv::ROTATE_90_COUNTERCLOCKWISE);

        float max_area_face = 0;
        int max_id_face = -1;
        for (int i = 0; i < det_results.size(); ++i)
        {
            float area_i = det_results[i].bbox.w * det_results[i].bbox.h;
            if (area_i > max_area_face)
            {
                max_area_face = area_i;
                max_id_face = i;
            }
        }

        if (max_id_face != -1)
        {
            fp.pre_process(det_results[max_id_face].bbox);
            fp.inference();
            fp.post_process(pose_result);
            {
                ScopedTiming st("osd draw", atoi(argv[9]));
                fp.draw_result(osd_frame,det_results[max_id_face].bbox,pose_result,false);
            }
            if (std::abs(pose_result.roll) > warning_angle_roll || std::abs(pose_result.yaw) > warning_angle_yaw || std::abs(pose_result.pitch) > warning_angle_pitch)
            {
                warning_count += 1; 
            }
            else
            {
                warning_count = 0;
            }
            if (warning_count > warning_amount)
            {
                cv::putText(osd_frame, " Please look straight ahead ! ", {0, 100}, cv::FONT_HERSHEY_COMPLEX, 2, cv::Scalar(255, 255, 0, 0), 2, 4, 0);
            }
        }
        cv::rotate(osd_frame, osd_frame, cv::ROTATE_90_CLOCKWISE);

        {
            ScopedTiming st("osd copy", atoi(argv[9]));
            memcpy(pic_vaddr, osd_frame.data, osd_width * osd_height * 4);
            // 显示通道插入帧
            kd_mpi_vo_chn_insert_frame(osd_id + 3, &vf_info); // K_VO_OSD0
            // printf("kd_mpi_vo_chn_insert_frame success \n");

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

    FaceDetection fd(argv[1], atof(argv[2]),atof(argv[3]), {SENSOR_CHANNEL, SENSOR_HEIGHT, SENSOR_WIDTH}, reinterpret_cast<uintptr_t>(vaddr), reinterpret_cast<uintptr_t>(paddr), atoi(argv[9]));
    FacePose fp(argv[4], {SENSOR_CHANNEL, SENSOR_HEIGHT, SENSOR_WIDTH}, reinterpret_cast<uintptr_t>(vaddr), reinterpret_cast<uintptr_t>(paddr), atoi(argv[9]));

    vector<FaceDetectionInfo> det_results;
    FacePoseInfo pose_result;
    int warning_amount = atoi(argv[5]);
    float warning_angle_roll = atof(argv[6]);
    float warning_angle_yaw = atof(argv[7]);
    float warning_angle_pitch = atof(argv[8]);
    int warning_count = 0;
    while (!isp_stop)
    {
        ScopedTiming st("total time", 1);

        {
            ScopedTiming st("read capture", atoi(argv[9]));
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
            ScopedTiming st("isp copy", atoi(argv[9]));
            auto vbvaddr = kd_mpi_sys_mmap_cached(dump_info.v_frame.phys_addr[0], size);
            memcpy(vaddr, (void *)vbvaddr, SENSOR_HEIGHT * SENSOR_WIDTH * 3);  // 这里以后可以去掉，不用copy
            kd_mpi_sys_munmap(vbvaddr, size);
        }

        det_results.clear();

        fd.pre_process();
        fd.inference();
        // 旋转后图像
        fd.post_process({SENSOR_WIDTH, SENSOR_HEIGHT}, det_results);

        cv::Mat osd_frame(osd_height, osd_width, CV_8UC4, cv::Scalar(0, 0, 0, 0));
        

        #if defined(STUDIO_HDMI)
        {
            float max_area_face = 0;
            int max_id_face = -1;
            for (int i = 0; i < det_results.size(); ++i)
            {
                float area_i = det_results[i].bbox.w * det_results[i].bbox.h;
                if (area_i > max_area_face)
                {
                    max_area_face = area_i;
                    max_id_face = i;
                }
            }

            if (max_id_face != -1)
            {
                fp.pre_process(det_results[max_id_face].bbox);
                fp.inference();
                fp.post_process(pose_result);
                {
                    ScopedTiming st("osd draw", atoi(argv[9]));
                    fp.draw_result(osd_frame,det_results[max_id_face].bbox,pose_result,false);
                }
                if (std::abs(pose_result.roll) > warning_angle_roll || std::abs(pose_result.yaw) > warning_angle_yaw || std::abs(pose_result.pitch) > warning_angle_pitch)
                {
                    warning_count += 1; 
                }
                else
                {
                    warning_count = 0;
                }
                if (warning_count > warning_amount)
                {
                    cv::putText(osd_frame, " Please look straight ahead ! ", {0, 100}, cv::FONT_HERSHEY_COMPLEX, 2, cv::Scalar(255, 255, 0, 0), 2, 4, 0);
                }
            }
        }
        #else
        {
            cv::rotate(osd_frame, osd_frame, cv::ROTATE_90_COUNTERCLOCKWISE);

            float max_area_face = 0;
            int max_id_face = -1;
            for (int i = 0; i < det_results.size(); ++i)
            {
                float area_i = det_results[i].bbox.w * det_results[i].bbox.h;
                if (area_i > max_area_face)
                {
                    max_area_face = area_i;
                    max_id_face = i;
                }
            }

            if (max_id_face != -1)
            {
                fp.pre_process(det_results[max_id_face].bbox);
                fp.inference();
                fp.post_process(pose_result);
                {
                    ScopedTiming st("osd draw", atoi(argv[9]));
                    fp.draw_result(osd_frame,det_results[max_id_face].bbox,pose_result,false);
                }
                if (std::abs(pose_result.roll) > warning_angle_roll || std::abs(pose_result.yaw) > warning_angle_yaw || std::abs(pose_result.pitch) > warning_angle_pitch)
                {
                    warning_count += 1; 
                }
                else
                {
                    warning_count = 0;
                }
                if (warning_count > warning_amount)
                {
                    cv::putText(osd_frame, " Please look straight ahead ! ", {0, 100}, cv::FONT_HERSHEY_COMPLEX, 2, cv::Scalar(255, 255, 0, 0), 2, 4, 0);
                }
            }
            cv::rotate(osd_frame, osd_frame, cv::ROTATE_90_CLOCKWISE);
        }
        #endif

        {
            ScopedTiming st("osd copy", atoi(argv[9]));
            memcpy(pic_vaddr, osd_frame.data, osd_width * osd_height * 4);
            // 显示通道插入帧
            kd_mpi_vo_chn_insert_frame(osd_id + 3, &vf_info); // K_VO_OSD0
            // printf("kd_mpi_vo_chn_insert_frame success \n");

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
    if (argc != 10)
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