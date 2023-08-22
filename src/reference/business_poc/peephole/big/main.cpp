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
#include "mapi_sys_api.h"

using std::cerr;
using std::cout;
using std::endl;

std::atomic<bool> isp_stop(false);

void snapshot_save(const std::string &snapshot_img_name, unsigned char *rgb_data, uint32_t pic_width, uint32_t pic_height) {
    cv::Mat img_bgr, img_rgb;

    uint32_t sigle_chn_size = pic_width * pic_height;
    cv::Mat img_bgr_b(pic_height, pic_width, CV_8UC1, rgb_data);
    cv::Mat img_bgr_g(pic_height, pic_width, CV_8UC1, rgb_data + sigle_chn_size);
    cv::Mat img_bgr_r(pic_height, pic_width, CV_8UC1, rgb_data + sigle_chn_size * 2);
    std::vector<cv::Mat> mat_vec;
    mat_vec.push_back(img_bgr_b);
    mat_vec.push_back(img_bgr_g);
    mat_vec.push_back(img_bgr_r);

    cv::merge(mat_vec, img_bgr);
    cv::cvtColor(img_bgr, img_rgb, cv::COLOR_BGR2RGB);
    cv::imwrite(snapshot_img_name, img_rgb);
}

std::string GetTimestampStr() {
    std::time_t timestamp = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
    struct tm *time = localtime(&timestamp);
    char time_value[128];
    printf("time year = %d, month = %d...\n", time->tm_year, time->tm_mon);
    sprintf(time_value, "%lu%02lu%02lu%02lu%02lu%02lu", (time->tm_year + 1900), (time->tm_mon + 1), time->tm_mday, time->tm_hour, time->tm_min, time->tm_sec);
    return std::string(time_value);
}

void video_proc(char *argv[]) {
    vivcap_start();

    k_video_frame_info vf_info;
    memset(&vf_info, 0, sizeof(vf_info));

    vf_info.v_frame.width = osd_width;
    vf_info.v_frame.height = osd_height;
    vf_info.v_frame.stride[0] = osd_width;
    vf_info.v_frame.pixel_format = PIXEL_FORMAT_ARGB_8888;

    void *pic_vaddr = NULL;       //osd
    block = get_vo_insert_frame(&vf_info, &pic_vaddr);

    // alloc memory
    size_t paddr = 0;
    void *vaddr = nullptr;
    size_t size = SENSOR_CHANNEL * SENSOR_HEIGHT * SENSOR_WIDTH;
    int ret = kd_mpi_sys_mmz_alloc_cached(&paddr, &vaddr, "allocate", "anonymous", size);
    if (ret) {
        std::cerr << "physical_memory_block::allocate failed: ret = " << ret << ", errno = " << strerror(errno) << std::endl;
        std::abort();
    }

    personDetect pd(argv[1], 0.5, 0.45, {SENSOR_CHANNEL, SENSOR_HEIGHT, SENSOR_WIDTH}, reinterpret_cast<uintptr_t>(vaddr), reinterpret_cast<uintptr_t>(paddr), 0);

    vector<BoxInfo> results;
    ScopedTiming st;
    bool is_first_detect = false;
    bool has_saved = false;

    while (!isp_stop) {
        memset(&dump_info, 0 , sizeof(k_video_frame_info));
        ret = kd_mpi_vicap_dump_frame(vicap_dev, VICAP_CHN_ID_1, VICAP_DUMP_YUV, &dump_info, 1000);
        if (ret) {
            printf("sample_vicap, ret = %d...kd_mpi_vicap_dump_frame failed.\n", ret);
            continue;
        }

        // 从vivcap中读取一帧图像到dump_info
        auto vbvaddr = kd_mpi_sys_mmap_cached(dump_info.v_frame.phys_addr[0], size);
        memcpy(vaddr, (void *)vbvaddr, SENSOR_HEIGHT * SENSOR_WIDTH * 3);  // 这里以后可以去掉，不用copy
        kd_mpi_sys_munmap(vbvaddr, size);

        results.clear();

        pd.pre_process();
        pd.inference();
        pd.post_process({SENSOR_WIDTH, SENSOR_HEIGHT}, results);
        if (results.size() > 0) {
            if (!is_first_detect) {
                st.CheckStartTimer();
                is_first_detect = true;
            } else {
                int elapsed = st.ElapsedSeconds();
                if (elapsed >= 10) {
                    std::string time_stamp = GetTimestampStr();
                    std::string snapshot_img_name = "save_" + time_stamp + ".jpg";
                    snapshot_save(snapshot_img_name, (unsigned char*)vaddr, SENSOR_WIDTH, SENSOR_HEIGHT);
                    st.TimerStop();
                    has_saved = true;
                    is_first_detect = false;
                }
            }
        } else {
            st.TimerStop();
            is_first_detect = false;
        }

        cv::Mat osd_frame(osd_height, osd_width, CV_8UC4, cv::Scalar(0, 0, 0, 0));

        if (has_saved) {
            for (auto r : results) {
                int x1 =  osd_width - r.x2 / SENSOR_WIDTH * osd_width;
                int y1 = osd_height -  r.y2 / SENSOR_HEIGHT  * osd_height;

                int w = (r.x2-r.x1) / SENSOR_WIDTH * osd_width;
                int h = (r.y2-r.y1) / SENSOR_HEIGHT  * osd_height;

                cv::rectangle(osd_frame, cv::Rect( x1,y1,w,h ), cv::Scalar(255, 255,0, 0), 6, 2, 0); // ARGB
            }
            has_saved = false;
        }

        memcpy(pic_vaddr, osd_frame.data, osd_width * osd_height * 4);
        kd_mpi_vo_chn_insert_frame(osd_id+3, &vf_info);  //K_VO_OSD0

        ret = kd_mpi_vicap_dump_release(vicap_dev, VICAP_CHN_ID_1, &dump_info);
        if (ret) {
            printf("sample_vicap...kd_mpi_vicap_dump_release failed.\n");
        }
    }

    vo_osd_release_block();
    vivcap_stop();


    // free memory
    ret = kd_mpi_sys_mmz_free(paddr, vaddr);
    if (ret) {
        std::cerr << "free failed: ret = " << ret << ", errno = " << strerror(errno) << std::endl;
        std::abort();
    }
}


int main(int argc, char *argv[]) {
    std::cout << "person_snap sample begin..." << std::endl;

    k_s32 ret = kd_mapi_sys_init();
    if(ret != K_SUCCESS) {
        printf("kd_mapi_sys_init error:%d\n", ret);
        return ret;
    }

    std::thread thread_isp(video_proc, argv);
    while (getchar() != 'q') {
        usleep(10000);
    }

    isp_stop = true;
    if (thread_isp.joinable())
        thread_isp.join();

    ret = kd_mapi_sys_deinit();
    if(ret != K_SUCCESS) {
        printf("kd_mapi_sys_deinit error:%d\n", ret);
        return ret;
    }

    std::cout << "person_snap sample end..." << std::endl;

    return 0;
}