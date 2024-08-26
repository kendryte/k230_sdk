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


#include "vi_vo.h"
#include "person_detect.h"
#include "utils.h"
#include "BYTETracker.h"


using std::cerr;
using std::cout;
using std::endl;

using namespace std;

const float color_list[80][3] =
{
    {0.000, 0.447, 0.741},
    {0.850, 0.325, 0.098},
    {0.929, 0.694, 0.125},
    {0.494, 0.184, 0.556},
    {0.466, 0.674, 0.188},
    {0.301, 0.745, 0.933},
    {0.635, 0.078, 0.184},
    {0.300, 0.300, 0.300},
    {0.600, 0.600, 0.600},
    {1.000, 0.000, 0.000},
    {1.000, 0.500, 0.000},
    {0.749, 0.749, 0.000},
    {0.000, 1.000, 0.000},
    {0.000, 0.000, 1.000},
    {0.667, 0.000, 1.000},
    {0.333, 0.333, 0.000},
    {0.333, 0.667, 0.000},
    {0.333, 1.000, 0.000},
    {0.667, 0.333, 0.000},
    {0.667, 0.667, 0.000},
    {0.667, 1.000, 0.000},
    {1.000, 0.333, 0.000},
    {1.000, 0.667, 0.000},
    {1.000, 1.000, 0.000},
    {0.000, 0.333, 0.500},
    {0.000, 0.667, 0.500},
    {0.000, 1.000, 0.500},
    {0.333, 0.000, 0.500},
    {0.333, 0.333, 0.500},
    {0.333, 0.667, 0.500},
    {0.333, 1.000, 0.500},
    {0.667, 0.000, 0.500},
    {0.667, 0.333, 0.500},
    {0.667, 0.667, 0.500},
    {0.667, 1.000, 0.500},
    {1.000, 0.000, 0.500},
    {1.000, 0.333, 0.500},
    {1.000, 0.667, 0.500},
    {1.000, 1.000, 0.500},
    {0.000, 0.333, 1.000},
    {0.000, 0.667, 1.000},
    {0.000, 1.000, 1.000},
    {0.333, 0.000, 1.000},
    {0.333, 0.333, 1.000},
    {0.333, 0.667, 1.000},
    {0.333, 1.000, 1.000},
    {0.667, 0.000, 1.000},
    {0.667, 0.333, 1.000},
    {0.667, 0.667, 1.000},
    {0.667, 1.000, 1.000},
    {1.000, 0.000, 1.000},
    {1.000, 0.333, 1.000},
    {1.000, 0.667, 1.000},
    {0.333, 0.000, 0.000},
    {0.500, 0.000, 0.000},
    {0.667, 0.000, 0.000},
    {0.833, 0.000, 0.000},
    {1.000, 0.000, 0.000},
    {0.000, 0.167, 0.000},
    {0.000, 0.333, 0.000},
    {0.000, 0.500, 0.000},
    {0.000, 0.667, 0.000},
    {0.000, 0.833, 0.000},
    {0.000, 1.000, 0.000},
    {0.000, 0.000, 0.167},
    {0.000, 0.000, 0.333},
    {0.000, 0.000, 0.500},
    {0.000, 0.000, 0.667},
    {0.000, 0.000, 0.833},
    {0.000, 0.000, 1.000},
    {0.000, 0.000, 0.000},
    {0.143, 0.143, 0.143},
    {0.286, 0.286, 0.286},
    {0.429, 0.429, 0.429},
    {0.571, 0.571, 0.571},
    {0.714, 0.714, 0.714},
    {0.857, 0.857, 0.857},
    {0.000, 0.447, 0.741},
    {0.314, 0.717, 0.741},
    {0.50, 0.5, 0}
};

std::atomic<bool> isp_stop(false);

void print_usage(const char *name)
{
    cout << "Usage: " << name << " <kmodel> <pd_thresh> <nms_thresh> <input_mode> <debug_mode> <fps> <buffer>" << endl
         << "For example: " << endl
         << " [for isp] ./bytetrack.elf bytetrack_yolov5n.kmodel 0.5 0.45 None 0 24 30" << endl
         << " [for img] ./bytetrack.elf bytetrack_yolov5n.kmodel 0.5 0.45 277 0 24 30" << endl
         << "Options:" << endl
         << " 1> kmodel    bytetrack行人检测kmodel文件路径 \n"
         << " 2> pd_thresh  行人检测阈值 \n"
         << " 3> nms_thresh  NMS阈值 \n"
         << " 4> input_mode       图像 (Number) or 摄像头(None) \n"
         << " 5> debug_mode      是否需要调试，0、1、2分别表示不调试、简单调试、详细调试 \n"
         << " 6> fps         帧率 \n" 
         << " 7> buffer      容忍帧数，即超过多少帧之后无法匹配上某个track，就认为该track丢失 \n"
         << "\n"
         << endl;
}


void video_proc_v1(char *argv[])
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

    personDetect pd(argv[1], atof(argv[2]),atof(argv[3]), {SENSOR_CHANNEL, SENSOR_HEIGHT, SENSOR_WIDTH}, reinterpret_cast<uintptr_t>(vaddr), reinterpret_cast<uintptr_t>(paddr), atoi(argv[5]));

    std::vector<BoxInfo> results;
    std::vector<Object> objects;

    // int fps = 24;
    int fps = atoi(argv[6]);
    int buffer = atoi(argv[7]);
    BYTETracker tracker(fps, buffer);

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

        {
            ScopedTiming st("results and objects clear ", atoi(argv[5]));
            results.clear();
            objects.clear();
        }
        
        pd.pre_process();
        pd.inference();
        pd.post_process({SENSOR_WIDTH, SENSOR_HEIGHT}, results);

        cv::Mat osd_frame(osd_height, osd_width, CV_8UC4, cv::Scalar(0, 0, 0, 0));
        std::string text;
        cv::Point origin;

        for (auto res : results)
        {
            ScopedTiming st("results transfer ", atoi(argv[5]));
            
            Object obj{ {res.x1,res.y1,res.x2- res.x1,res.y2- res.y1},res.label,res.score };
            objects.push_back(obj);

        }
        
        std::vector<STrack> output_stracks;
        {
            ScopedTiming st("tracker.update ", atoi(argv[5]));
            output_stracks = tracker.update(objects);
        }

        for (int i = 0; i < output_stracks.size(); i++)
        {
            ScopedTiming st("draw boxes", atoi(argv[5]));
            std::vector<float> tlwh = output_stracks[i].tlwh;
            bool vertical = tlwh[2] / tlwh[3] > 1.6;
            if (tlwh[2] * tlwh[3] > 20 && !vertical)
            {
                Scalar s = tracker.get_color(output_stracks[i].track_id);

                int x1 =  tlwh[0] / SENSOR_WIDTH * osd_width;
                int y1 =  tlwh[1] / SENSOR_HEIGHT  * osd_height;

                int w = tlwh[2] / SENSOR_WIDTH * osd_width;
                int h = tlwh[3] / SENSOR_HEIGHT  * osd_height;
                
                cv::putText(osd_frame, format("%d", output_stracks[i].track_id), Point(x1, y1 + 5), 0, 2, Scalar(255,255, 0, 0), 2, LINE_AA);
                cv::rectangle(osd_frame, Rect(x1,y1,w,h), s, 2);

                
            }
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

void video_proc_v2(char *argv[])
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

    personDetect pd(argv[1], atof(argv[2]),atof(argv[3]), {SENSOR_CHANNEL, SENSOR_HEIGHT, SENSOR_WIDTH}, reinterpret_cast<uintptr_t>(vaddr), reinterpret_cast<uintptr_t>(paddr), atoi(argv[5]));

    std::vector<BoxInfo> results;
    std::vector<Object> objects;

    // int fps = 24;
    int fps = atoi(argv[6]);
    int buffer = atoi(argv[7]);
    BYTETracker tracker(fps, buffer);

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

        {
            ScopedTiming st("results and objects clear ", atoi(argv[5]));
            results.clear();
            objects.clear();
        }
        
        pd.pre_process();
        pd.inference();
        pd.post_process({SENSOR_WIDTH, SENSOR_HEIGHT}, results);

        cv::Mat osd_frame(osd_height, osd_width, CV_8UC4, cv::Scalar(0, 0, 0, 0));
        std::string text;
        cv::Point origin;

        for (auto res : results)
        {
            ScopedTiming st("results transfer ", atoi(argv[5]));
            
            Object obj{ {res.x1,res.y1,res.x2- res.x1,res.y2- res.y1},res.label,res.score };
            objects.push_back(obj);

        }
        
        std::vector<STrack> output_stracks;
        {
            ScopedTiming st("tracker.update ", atoi(argv[5]));
            output_stracks = tracker.update(objects);
        }

        for (int i = 0; i < output_stracks.size(); i++)
        {
            ScopedTiming st("draw boxes", atoi(argv[5]));
            std::vector<float> tlwh = output_stracks[i].tlwh;
            bool vertical = tlwh[2] / tlwh[3] > 1.6;
            if (tlwh[2] * tlwh[3] > 20 && !vertical)
            {
                Scalar s = tracker.get_color(output_stracks[i].track_id);

                int x1 =  tlwh[0] / SENSOR_WIDTH * osd_width;
                int y1 =  tlwh[1] / SENSOR_HEIGHT  * osd_height;

                int w = tlwh[2] / SENSOR_WIDTH * osd_width;
                int h = tlwh[3] / SENSOR_HEIGHT  * osd_height;
                
                cv::putText(osd_frame, format("%d", output_stracks[i].track_id), Point(x1, y1 + 5), 0, 2, Scalar(255,255, 0, 0), 2, LINE_AA);
                cv::rectangle(osd_frame, Rect(x1,y1,w,h), s, 2);

                
            }
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

void video_proc_k230d(char *argv[])
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

    personDetect pd(argv[1], atof(argv[2]),atof(argv[3]), {SENSOR_CHANNEL, SENSOR_HEIGHT, SENSOR_WIDTH}, reinterpret_cast<uintptr_t>(vaddr), reinterpret_cast<uintptr_t>(paddr), atoi(argv[5]));

    std::vector<BoxInfo> results;
    std::vector<Object> objects;

    // int fps = 24;
    int fps = atoi(argv[6]);
    int buffer = atoi(argv[7]);
    BYTETracker tracker(fps, buffer);

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

        {
            ScopedTiming st("results and objects clear ", atoi(argv[5]));
            results.clear();
            objects.clear();
        }
        
        pd.pre_process();
        pd.inference();
        pd.post_process({SENSOR_WIDTH, SENSOR_HEIGHT}, results);

        cv::Mat osd_frame(osd_height, osd_width, CV_8UC4, cv::Scalar(0, 0, 0, 0));
        cv::rotate(osd_frame, osd_frame, cv::ROTATE_90_COUNTERCLOCKWISE);
        std::string text;
        cv::Point origin;

        for (auto res : results)
        {
            ScopedTiming st("results transfer ", atoi(argv[5]));
            
            Object obj{ {res.x1,res.y1,res.x2- res.x1,res.y2- res.y1},res.label,res.score };
            objects.push_back(obj);

        }
        
        std::vector<STrack> output_stracks;
        {
            ScopedTiming st("tracker.update ", atoi(argv[5]));
            output_stracks = tracker.update(objects);
        }

        for (int i = 0; i < output_stracks.size(); i++)
        {
            ScopedTiming st("draw boxes", atoi(argv[5]));
            std::vector<float> tlwh = output_stracks[i].tlwh;
            bool vertical = tlwh[2] / tlwh[3] > 1.6;
            if (tlwh[2] * tlwh[3] > 20 && !vertical)
            {
                Scalar s = tracker.get_color(output_stracks[i].track_id);

                int x1 =  tlwh[0] / SENSOR_WIDTH * osd_frame.cols;
                int y1 =  tlwh[1] / SENSOR_HEIGHT  * osd_frame.rows;

                int w = tlwh[2] / SENSOR_WIDTH * osd_frame.cols;
                int h = tlwh[3] / SENSOR_HEIGHT  * osd_frame.rows;
                
                cv::putText(osd_frame, format("%d", output_stracks[i].track_id), Point(x1, y1 + 5), 0, 2, Scalar(255,255, 0, 0), 2, LINE_AA);
                cv::rectangle(osd_frame, Rect(x1,y1,w,h), s, 2);

                
            }
        }
        cv::rotate(osd_frame, osd_frame, cv::ROTATE_90_CLOCKWISE);

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

void video_proc_01(char *argv[])
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

    personDetect pd(argv[1], atof(argv[2]),atof(argv[3]), {SENSOR_CHANNEL, SENSOR_HEIGHT, SENSOR_WIDTH}, reinterpret_cast<uintptr_t>(vaddr), reinterpret_cast<uintptr_t>(paddr), atoi(argv[5]));

    std::vector<BoxInfo> results;
    std::vector<Object> objects;

    // int fps = 24;
    int fps = atoi(argv[6]);
    int buffer = atoi(argv[7]);
    BYTETracker tracker(fps, buffer);

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

        {
            ScopedTiming st("results and objects clear ", atoi(argv[5]));
            results.clear();
            objects.clear();
        }
        
        pd.pre_process();
        pd.inference();
        pd.post_process({SENSOR_WIDTH, SENSOR_HEIGHT}, results);

        cv::Mat osd_frame(osd_height, osd_width, CV_8UC4, cv::Scalar(0, 0, 0, 0));



        #if defined(STUDIO_HDMI)
        {
            std::string text;
            cv::Point origin;

            for (auto res : results)
            {
                ScopedTiming st("results transfer ", atoi(argv[5]));
                
                Object obj{ {res.x1,res.y1,res.x2- res.x1,res.y2- res.y1},res.label,res.score };
                objects.push_back(obj);

            }
            
            std::vector<STrack> output_stracks;
            {
                ScopedTiming st("tracker.update ", atoi(argv[5]));
                output_stracks = tracker.update(objects);
            }

            for (int i = 0; i < output_stracks.size(); i++)
            {
                ScopedTiming st("draw boxes", atoi(argv[5]));
                std::vector<float> tlwh = output_stracks[i].tlwh;
                bool vertical = tlwh[2] / tlwh[3] > 1.6;
                if (tlwh[2] * tlwh[3] > 20 && !vertical)
                {
                    Scalar s = tracker.get_color(output_stracks[i].track_id);

                    int x1 =  tlwh[0] / SENSOR_WIDTH * osd_frame.cols;
                    int y1 =  tlwh[1] / SENSOR_HEIGHT  * osd_frame.rows;

                    int w = tlwh[2] / SENSOR_WIDTH * osd_frame.cols;
                    int h = tlwh[3] / SENSOR_HEIGHT  * osd_frame.rows;
                    
                    cv::putText(osd_frame, format("%d", output_stracks[i].track_id), Point(x1, y1 + 5), 0, 2, Scalar(255,255, 0, 0), 2, LINE_AA);
                    cv::rectangle(osd_frame, Rect(x1,y1,w,h), s, 2);

                    
                }
            }
        }
        #else
        {
            cv::rotate(osd_frame, osd_frame, cv::ROTATE_90_COUNTERCLOCKWISE);
            std::string text;
            cv::Point origin;

            for (auto res : results)
            {
                ScopedTiming st("results transfer ", atoi(argv[5]));
                
                Object obj{ {res.x1,res.y1,res.x2- res.x1,res.y2- res.y1},res.label,res.score };
                objects.push_back(obj);

            }
            
            std::vector<STrack> output_stracks;
            {
                ScopedTiming st("tracker.update ", atoi(argv[5]));
                output_stracks = tracker.update(objects);
            }

            for (int i = 0; i < output_stracks.size(); i++)
            {
                ScopedTiming st("draw boxes", atoi(argv[5]));
                std::vector<float> tlwh = output_stracks[i].tlwh;
                bool vertical = tlwh[2] / tlwh[3] > 1.6;
                if (tlwh[2] * tlwh[3] > 20 && !vertical)
                {
                    Scalar s = tracker.get_color(output_stracks[i].track_id);

                    int x1 =  tlwh[0] / SENSOR_WIDTH * osd_frame.cols;
                    int y1 =  tlwh[1] / SENSOR_HEIGHT  * osd_frame.rows;

                    int w = tlwh[2] / SENSOR_WIDTH * osd_frame.cols;
                    int h = tlwh[3] / SENSOR_HEIGHT  * osd_frame.rows;
                    
                    cv::putText(osd_frame, format("%d", output_stracks[i].track_id), Point(x1, y1 + 5), 0, 2, Scalar(255,255, 0, 0), 2, LINE_AA);
                    cv::rectangle(osd_frame, Rect(x1,y1,w,h), s, 2);

                    
                }
            }
            cv::rotate(osd_frame, osd_frame, cv::ROTATE_90_CLOCKWISE);
        }
        #endif

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
    if (argc != 8)
    {
        print_usage(argv[0]);
        return -1;
    }

    if (strcmp(argv[4], "None") == 0)
    {
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
    }
    else
    {
        personDetect pd(argv[1], atof(argv[2]),atof(argv[3]),  atoi(argv[5]));
        std::vector<BoxInfo> results;
        std::vector<Object> objects;

        // int fps = 24;
        int fps = atoi(argv[6]);
        int buffer = atoi(argv[7]);
        BYTETracker tracker(fps, buffer);

        int num_frames = 0;
        int total_ms = 0;

        int fileNum = atoi(argv[4]);

        while(true)
        {

            objects.clear();
            results.clear();
            if (num_frames > fileNum)
                break;

            std::string frame_pth =  "bytetrack_data/images/" + std::to_string(num_frames) + ".jpg";

            cv::Mat img = cv::imread(frame_pth);

            pd.pre_process(img);
            pd.inference();
            pd.post_process({img.cols, img.rows}, results);

            cv::Point origin;

            for (auto res : results)
            {
                ScopedTiming st("results transfer ", atoi(argv[5]));
                
                Object obj{ {res.x1,res.y1,res.x2- res.x1,res.y2- res.y1},res.label,res.score };
                objects.push_back(obj);

            }

            std::vector<STrack> output_stracks = tracker.update(objects);
            {
                ScopedTiming st("tracker.update ", atoi(argv[5]));
                output_stracks = tracker.update(objects);
            }

            for (int i = 0; i < output_stracks.size(); i++)
            {
                ScopedTiming st("draw boxes", atoi(argv[5]));
                std::vector<float> tlwh = output_stracks[i].tlwh;
                bool vertical = tlwh[2] / tlwh[3] > 1.6;
                if (tlwh[2] * tlwh[3] > 20 && !vertical)
                {
                    Scalar s = tracker.get_color_img(output_stracks[i].track_id);

                    putText(img, format("%d", output_stracks[i].track_id), Point(tlwh[0], tlwh[1] - 5), 0, 0.6, Scalar(0, 0, 255), 2, LINE_AA);
                    rectangle(img, Rect(tlwh[0], tlwh[1], tlwh[2], tlwh[3]), s, 2);
                }
            }
            putText(img, format("frame: %d  num: %d", num_frames, output_stracks.size()), 
                    Point(0, 30), 0, 0.6, Scalar(0, 0, 255), 2, LINE_AA);
            
            std::string out_path = "bytetrack_data/output/" + std::to_string(num_frames) + ".jpg";

            cv::imwrite(out_path, img);

            num_frames++;
        }

    }
    
    return 0;
}