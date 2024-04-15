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
#include "e3d.h"

#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include "k_datafifo.h"

#include <stdlib.h>
#include <time.h>

std::atomic<bool> isp_stop(false);

#define READER_INDEX    0
#define WRITER_INDEX    1

static k_s32 g_s32Index = 0;
static k_datafifo_handle hDataFifo[2] = {(k_datafifo_handle)K_DATAFIFO_INVALID_HANDLE, (k_datafifo_handle)K_DATAFIFO_INVALID_HANDLE};
static const k_s32 BLOCK_LEN = 252;  //输出内容大小

static void release(void* pStream)
{
}

static int datafifo_init()
{
    k_s32 s32Ret = K_SUCCESS;

    k_datafifo_params_s writer_params = {10, BLOCK_LEN, K_TRUE, DATAFIFO_WRITER};

    s32Ret = kd_datafifo_open(&hDataFifo[WRITER_INDEX], &writer_params);

    if (K_SUCCESS != s32Ret)
    {
        printf("open datafifo error:%x\n", s32Ret);
        return -1;
    }

    k_u64 phyAddr = 0;
    s32Ret = kd_datafifo_cmd(hDataFifo[WRITER_INDEX], DATAFIFO_CMD_GET_PHY_ADDR, &phyAddr);

    if (K_SUCCESS != s32Ret)
    {
        printf("get datafifo phy addr error:%x\n", s32Ret);
        return -1;
    }

    printf("PhyAddr: %lx\n", phyAddr);

    s32Ret = kd_datafifo_cmd(hDataFifo[WRITER_INDEX], DATAFIFO_CMD_SET_DATA_RELEASE_CALLBACK, (void *)release);

    if (K_SUCCESS != s32Ret)
    {
        printf("set release func callback error:%x\n", s32Ret);
        return -1;
    }

    printf("datafifo_init finish\n");

    return 0;
}

void datafifo_deinit(void)
{
    k_s32 s32Ret = K_SUCCESS;
    // call write NULL to flush and release stream buffer.
    s32Ret = kd_datafifo_write(hDataFifo[WRITER_INDEX], NULL);
    if (K_SUCCESS != s32Ret)
    {
        printf("write error:%x\n", s32Ret);
    }
    printf(" kd_datafifo_close %lx\n", hDataFifo[WRITER_INDEX]);
    kd_datafifo_close(hDataFifo[WRITER_INDEX]);
}

void print_usage(const char *name)
{
	cout << "Usage: " << name << "<kmodel_det> <obj_thresh> <nms_thresh> <kmodel_kp> <kmodel_e3d> <debug_mode>" << endl
		 << "Options:" << endl
		 << " 1> kmodel_det      手掌检测kmodel路径\n"
         << " 2> obj_thresh      手掌检测阈值\n"
         << " 3> nms_thresh      手掌检测非极大值抑制阈值\n"
		 << " 4> kmodel_kp       手势关键点检测kmodel路径\n"
         << " 5> kmodel_e3d      e3d检测kmodel路径\n"
		 << " 6> debug_mode      是否需要调试, 0、1、2分别表示不调试、简单调试、详细调试\n"
		 << "\n"
		 << endl;
}

void video_proc(char *argv[])
{
    k_char cmd[64];
    k_s32 s32Ret = K_SUCCESS;
    pthread_t sendThread;
    pthread_t readThread;

    s32Ret = datafifo_init();
    if (0 != s32Ret)
    {
        return;
    }

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

    HandDetection hd(argv[1], atof(argv[2]), atof(argv[3]), {SENSOR_WIDTH, SENSOR_HEIGHT}, {SENSOR_CHANNEL, SENSOR_HEIGHT, SENSOR_WIDTH}, reinterpret_cast<uintptr_t>(vaddr), reinterpret_cast<uintptr_t>(paddr), atoi(argv[6]));

    HandKeypoint hk(argv[4], {SENSOR_CHANNEL, SENSOR_HEIGHT, SENSOR_WIDTH}, reinterpret_cast<uintptr_t>(vaddr), reinterpret_cast<uintptr_t>(paddr), atoi(argv[6]));

    E3d e3d(argv[5], atoi(argv[6]));

    std::vector<BoxInfo> results;

    vector<float> result_e3d;

    while (!isp_stop)
    {
        ScopedTiming st("total time", 1);

        // datafifo
        k_u32 availWriteLen = 0;

        // call write NULL to flush
        s32Ret = kd_datafifo_write(hDataFifo[WRITER_INDEX], NULL);
        if (K_SUCCESS != s32Ret)
        {
            printf("write error:%x\n", s32Ret);
        }

        s32Ret = kd_datafifo_cmd(hDataFifo[WRITER_INDEX], DATAFIFO_CMD_GET_AVAIL_WRITE_LEN, &availWriteLen);
        if (K_SUCCESS != s32Ret)
        {
            printf("get available write len error:%x\n", s32Ret);
            break;
        }

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
        result_e3d.clear();

        hd.pre_process();
        hd.inference();
        hd.post_process(results);

        cv::Mat osd_frame(osd_height, osd_width, CV_8UC4, cv::Scalar(0, 0, 0, 0));
        cv::Mat cropped_img;

        if ((results.size() < 2))
        {
            for (auto r: results)
            {
                int w = r.x2 - r.x1 + 1;
                int h = r.y2 - r.y1 + 1;
                
                int length = std::max(w,h)/2;
                int cx = (r.x1+r.x2)/2;
                int cy = (r.y1+r.y2)/2;
                int ratio_num = 1.6*length;

                int x1_1 = std::max(0,cx-ratio_num);
                int y1_1 = std::max(0,cy-ratio_num);
                int x2_1 = std::min(SENSOR_WIDTH-1, cx+ratio_num);
                int y2_1 = std::min(SENSOR_HEIGHT-1, cy+ratio_num);
                int w_1 = x2_1 - x1_1 + 1;
                int h_1 = y2_1 - y1_1 + 1;
                
                Bbox bbox = {x1_1,y1_1,w_1,h_1};
                hk.pre_process(bbox);
                hk.inference();
                hk.post_process(bbox);
                {
                    int matsize = SENSOR_WIDTH * SENSOR_HEIGHT;
                    
                    cv::Mat ori_img;
                    cv::Mat ori_img_R = cv::Mat(SENSOR_HEIGHT, SENSOR_WIDTH, CV_8UC1, vaddr);
                    cv::Mat ori_img_G = cv::Mat(SENSOR_HEIGHT, SENSOR_WIDTH, CV_8UC1, vaddr + 1 * matsize);
                    cv::Mat ori_img_B = cv::Mat(SENSOR_HEIGHT, SENSOR_WIDTH, CV_8UC1, vaddr + 2 * matsize);
                    std::vector<cv::Mat> sensor_rgb;
                    sensor_rgb.push_back(ori_img_B);
                    sensor_rgb.push_back(ori_img_G);
                    sensor_rgb.push_back(ori_img_R); 
                    cv::merge(sensor_rgb, ori_img);

                    cropped_img = Utils::crop(ori_img, bbox);
                }

                e3d.pre_process(cropped_img, hk.results);
                e3d.inference();
                e3d.post_process(result_e3d);

                if (availWriteLen >= BLOCK_LEN)
                {
                    ScopedTiming st("datafifo send ", atoi(argv[6]));
                    s32Ret = kd_datafifo_write(hDataFifo[WRITER_INDEX], (k_char*)result_e3d.data());
                    if (K_SUCCESS != s32Ret)
                    {
                        printf("write error:%x\n", s32Ret);
                        break;
                    }

                    s32Ret = kd_datafifo_cmd(hDataFifo[WRITER_INDEX], DATAFIFO_CMD_WRITE_DONE, NULL);
                    if (K_SUCCESS != s32Ret)
                    {
                        printf("write done error:%x\n", s32Ret);
                        break;
                    }
                    g_s32Index++;
                }
                else
                {
                    printf("no free space: %d\n", availWriteLen);
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


int main(int argc, char *argv[])
{
    std::cout << "case " << argv[0] << " built at " << __DATE__ << " " << __TIME__ << std::endl;
    if (argc != 7)
    {
        print_usage(argv[0]); 
        return -1;
    }

    std::thread thread_isp(video_proc, argv);
    while (getchar() != 'q')
    {
        usleep(10000);
    }

    isp_stop = true;
    thread_isp.join();
    
    return 0;
}
