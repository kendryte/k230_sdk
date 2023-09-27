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
//
#include <iostream>
#include <thread>
#include "utils.h"
#include "vi_vo.h"
#include "person_detect.h"
#include "pulc.h"

using std::cerr;
using std::cout;
using std::endl;

std::atomic<bool> isp_stop(false);


void print_usage(const char *name)
{
    cout << "Usage: " << name << "<kmodel> <pd_thresh> <nms_thresh> <input_mode> <attr_kmodel> <pulc_thresh> <glasses_thresh> <hold_thresh> <debug_mode>" << endl
         << "For example: " << endl
         << " [for img] ./person_attr.elf person_attr_yolov5n.kmodel 0.5 0.45 hrnet_demo.jpg person_pulc.kmodel 0.5 0.5 0.5 0" << endl
         << " [for isp] ./person_attr.elf person_attr_yolov5n.kmodel 0.5 0.45 None person_pulc.kmodel 0.5 0.5 0.5 0" << endl
         << "Options:" << endl
         << " 1> kmodel    行人检测kmodel文件路径 \n"
         << " 2> pd_thresh  行人检测阈值\n"
         << " 3> nms_thresh  NMS阈值\n"
         << " 4> input_mode      本地图片(图片路径)/ 摄像头(None) \n"
         << " 5> attr_kmodel 属性识别kmodel文件路径 \n"
         << " 6> pulc_thresh 属性识别阈值 \n"
         << " 7> glasses_thresh 是否配戴眼镜阈值 \n"
         << " 8> hold_thresh 是否持物阈值 \n"
         << " 9> debug_mode      是否需要调试，0、1、2分别表示不调试、简单调试、详细调试\n"
         << "\n"
         << endl;
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

    personDetect pd(argv[1], atof(argv[2]),atof(argv[3]), {SENSOR_CHANNEL, SENSOR_HEIGHT, SENSOR_WIDTH}, reinterpret_cast<uintptr_t>(vaddr), reinterpret_cast<uintptr_t>(paddr), atoi(argv[9]));
    float pulc_thresh = atof(argv[6]);
    float glasses_thresh = atof(argv[7]);
    float hold_thresh = atof(argv[8]);
    Pulc pul(argv[5], {SENSOR_CHANNEL, SENSOR_HEIGHT, SENSOR_WIDTH},pulc_thresh, glasses_thresh, hold_thresh, reinterpret_cast<uintptr_t>(vaddr), reinterpret_cast<uintptr_t>(paddr), atoi(argv[9]));
    vector<BoxInfo> results;

    while (!isp_stop)
    {
        ScopedTiming st("total time", 1);

        {
            ScopedTiming st("read capture", atoi(argv[5]));
            // VICAP_CHN_ID_1 out rgb888p
            memset(&dump_info, 0 , sizeof(k_video_frame_info));
            ret = kd_mpi_vicap_dump_frame(vicap_dev, VICAP_CHN_ID_1, VICAP_DUMP_YUV, &dump_info, 1000);
            if (ret) {
                printf("sample_vicap...kd_mpi_vicap_dump_frame failed.\n");
                continue;
            }
        }
            

        {
            ScopedTiming st("isp copy", atoi(argv[5]));
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
        Bbox crop_box;
        std::string attr_results;

        for (auto r : results)
        {
            ScopedTiming st("draw boxes", atoi(argv[9]));
            text = pd.labels[r.label] + ":" + std::to_string(round(r.score * 100) / 100).substr(0,4);

            int x1 =  r.x1 / SENSOR_WIDTH * osd_width;
            int y1 =  r.y1 / SENSOR_HEIGHT  * osd_height;

            int w = (r.x2-r.x1) / SENSOR_WIDTH * osd_width;
            int h = (r.y2-r.y1) / SENSOR_HEIGHT  * osd_height;

            cv::rectangle(osd_frame, cv::Rect( x1,y1,w,h ), cv::Scalar(255, 255,0, 0), 2, 2, 0); // ARGB
            
            crop_box = { r.x1,r.y1,(r.x2-r.x1),(r.y2-r.y1) };
            pul.pre_process( crop_box );
            pul.inference();
            attr_results = pul.post_process();

            origin.x = r.x1;
            origin.y = r.y1 + 20;

            float fontScale = 2;
            int with = 80;
            string gender = pul.GetGender();
            origin.y += with;
            cv::putText(osd_frame, gender, origin, cv::FONT_HERSHEY_COMPLEX, fontScale, cv::Scalar(255, 0, 0, 255), 1, 8, 0);
            string age = pul.GetAge();
            origin.y += with;
            cv::putText(osd_frame, age, origin, cv::FONT_HERSHEY_COMPLEX, fontScale, cv::Scalar(255, 0, 0, 255), 1, 8, 0);
            string direction = pul.GetDirection();
            origin.y += with;
            cv::putText(osd_frame, direction, origin, cv::FONT_HERSHEY_COMPLEX, fontScale, cv::Scalar(255, 0, 0, 255), 1, 8, 0);
           
        }


        {
            ScopedTiming st("osd copy", atoi(argv[5]));
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
    if (argc != 10)
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
        personDetect pd(argv[1], atof(argv[2]),atof(argv[3]), atoi(argv[9]));
        pd.pre_process(ori_img);
        pd.inference();

        vector<BoxInfo> results;
        pd.post_process({ori_w, ori_h}, results);
        
        float pulc_thresh = atof(argv[6]);
        float glasses_thresh = atof(argv[7]);
        float hold_thresh = atof(argv[8]);
        Pulc pul( argv[5],pulc_thresh,glasses_thresh,hold_thresh,atoi(argv[9]) );
        std::string attr_results;

        
        for (auto r : results)
        {
            ScopedTiming st("draw boxes", atoi(argv[9]));
            std::string text = pd.labels[r.label] + ":" + std::to_string(round(r.score * 100) / 100).substr(0,4);
            std::cout << "text = " << text << std::endl;
            cv::rectangle(ori_img, cv::Rect(r.x1, r.y1, r.x2 - r.x1 + 1, r.y2 - r.y1 + 1), cv::Scalar(0, 0, 255), 2, 2, 0);
            cv::Point origin;

            origin.x = r.x1;
            origin.y = r.y1 - 20;

            cv::putText(ori_img, text, origin, cv::FONT_HERSHEY_COMPLEX, 0.5, cv::Scalar(0, 255, 255), 1, 8, 0);
            pul.pre_process(ori_img, r);
            pul.inference();
            attr_results = pul.post_process();

            float fontScale = 0.5;
            int with = 25;
            string gender = pul.GetGender();
            origin.y += with;
            cv::putText(ori_img, gender, origin, cv::FONT_HERSHEY_COMPLEX, fontScale, cv::Scalar(255, 255, 0, 255), 1, 8, 0);
            string age = pul.GetAge();
            origin.y += with;
            cv::putText(ori_img, age, origin, cv::FONT_HERSHEY_COMPLEX, fontScale, cv::Scalar(255, 255, 0, 255), 1, 8, 0);
            string direction = pul.GetDirection();
            origin.y += with;
            cv::putText(ori_img, direction, origin, cv::FONT_HERSHEY_COMPLEX, fontScale, cv::Scalar(255, 255, 0, 255), 1, 8, 0);
            string glasses = pul.GetGlasses();
            origin.y += with;
            cv::putText(ori_img, glasses, origin, cv::FONT_HERSHEY_COMPLEX, fontScale, cv::Scalar(255, 255, 0, 255), 1, 8, 0);
            string bag = pul.GetBag();
            origin.y += with;
            cv::putText(ori_img, bag, origin, cv::FONT_HERSHEY_COMPLEX, fontScale, cv::Scalar(255, 255, 0, 255), 1, 8, 0);
           

        }

        cv::imwrite("person_attr_result.jpg", ori_img);
    }
    return 0;
}