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
#include "dynamic_gesture.h"
#include "face_detection.h"
#include "face_pose.h"
#include "crop.h"
#include "src.h"
#include "tracker.h"
#include "cv2_utils.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>

//串口通信设备名和缓冲区大小
#define UART_DEVICE_NAME1 "/dev/uart2"
#define BUF_SIZE (512)

/**
*本项目实现了手势识别、动态手势识别、人脸姿态角估计、人脸单目标跟踪四个任务；四个任务通过手势识别作为控制命令完成切换。
*/
std::atomic<bool> isp_stop(false);
//手势识别模型及参数，包括手掌检测和手势关键点分类模型，关键点为21个
string g_hand_detection_path="hand_det.kmodel";
string g_hand_keypoint_path="handkp_det.kmodel";
float g_obj_thresh=0.15;
float g_nms_thresh=0.4;
//动态手势识别模型及参数，动态手势识别同时需要用到手势识别的两个模型
string g_dynamic_gesture_path="gesture.kmodel";
float g_gesture_obj_thresh=0.4;
float g_gesture_nms_thresh=0.4;
//人脸姿态估计模型及参数，包括人脸检测和脸部姿态估计模型
string g_face_detection_path="face_detection_320.kmodel";
float g_facedet_obj_thresh=0.6;
float g_facedet_nms_thresh=0.2;
string g_face_pose_path="face_pose.kmodel";
//单目标跟踪模型及参数
string g_nanotracker_backbone_crop_path="cropped_test127.kmodel";
string g_nanotracker_backbone_src_path="nanotrack_backbone_sim.kmodel";
string g_nanotracker_head_path="nanotracker_head_calib_k230.kmodel";
int g_template_inputsize=127;
int g_obj_inputsize=255;
int g_head_thresh=0.05;
//任务相关参数
int cur_task_state=0;
string cur_gesture="";
string dg_state="middle";
int debug_mode=0;
//帧头帧尾
int8_t frame_head=0xAA;
int8_t frame_tail=0xBB;

void video_proc()
{
    //打开串口
    int fd1;
    char send[BUF_SIZE];
    fd1 = open(UART_DEVICE_NAME1, O_RDWR);
    if (fd1 < 0) {
        perror("Failed to open UART device");
        return;
    }

    vivcap_start();
    // 设置osd参数
    k_video_frame_info vf_info;
    void *pic_vaddr = NULL;
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

    //手势识别和动态手势识别初始化
    HandDetection hd(g_hand_detection_path.c_str(), g_gesture_obj_thresh, g_gesture_nms_thresh, {SENSOR_WIDTH, SENSOR_HEIGHT}, {SENSOR_CHANNEL, SENSOR_HEIGHT, SENSOR_WIDTH}, reinterpret_cast<uintptr_t>(vaddr), reinterpret_cast<uintptr_t>(paddr), debug_mode);
    HandKeypoint hk(g_hand_keypoint_path.c_str(), {SENSOR_CHANNEL, SENSOR_HEIGHT, SENSOR_WIDTH}, reinterpret_cast<uintptr_t>(vaddr), reinterpret_cast<uintptr_t>(paddr), debug_mode);
    DynamicGesture Dag(g_dynamic_gesture_path.c_str(), debug_mode);
    enum state {TRIGGER,UP,RIGHT,DOWN,LEFT,MIDDLE} cur_state_ = TRIGGER, pre_state_ = TRIGGER, draw_state_ = TRIGGER;
    int idx_ = 0;
    int idx = 0;
    std::vector<int> history ={2};
    std::vector<vector<float>> history_logit ;
    std::vector<int> vec_flag;
    std::chrono::steady_clock::time_point m_start; // 计时开始时间
	std::chrono::steady_clock::time_point m_stop;  // 计时结束时间
    std::chrono::steady_clock::time_point s_start; // 结果显示计时开始时间
    std::chrono::steady_clock::time_point s_stop;  // 结果显示计时结束时间
    int bin_width = 150;
    int bin_height = 216;
    //读入左上角提示图标
    cv::Mat shang_argb;
    Utils::bin_2_mat("shang.bin", bin_width, bin_height, shang_argb);
    cv::Mat xia_argb;
    Utils::bin_2_mat("xia.bin", bin_width, bin_height, xia_argb);
    cv::Mat zuo_argb;
    Utils::bin_2_mat("zuo.bin", bin_height, bin_width, zuo_argb);
    cv::Mat you_argb;
    Utils::bin_2_mat("you.bin", bin_height, bin_width, you_argb);
    cv::Mat trigger_argb;
    cv::Mat up_argb;
    cv::Mat down_argb;
    cv::Mat left_argb;
    cv::Mat right_argb;
    cv::Mat middle_argb;
    std::vector<BoxInfo> results;
    //人脸姿态估计初始化
    FaceDetection fd(g_face_detection_path.c_str(), g_facedet_obj_thresh,g_facedet_nms_thresh, {SENSOR_CHANNEL, SENSOR_HEIGHT, SENSOR_WIDTH}, reinterpret_cast<uintptr_t>(vaddr), reinterpret_cast<uintptr_t>(paddr), debug_mode);
    FacePose fp(g_face_pose_path.c_str(), {SENSOR_CHANNEL, SENSOR_HEIGHT, SENSOR_WIDTH}, reinterpret_cast<uintptr_t>(vaddr), reinterpret_cast<uintptr_t>(paddr), debug_mode);
    std::vector<FaceDetectionInfo> det_results;
    FacePoseInfo pose_result;
    //单目标跟踪初始化
    init();
    Crop crop(g_nanotracker_backbone_crop_path.c_str(), debug_mode);
    Crop crop_video(g_nanotracker_backbone_crop_path.c_str(),{SENSOR_CHANNEL, SENSOR_HEIGHT, SENSOR_WIDTH}, reinterpret_cast<uintptr_t>(vaddr), reinterpret_cast<uintptr_t>(paddr), debug_mode);
    Src src(g_nanotracker_backbone_src_path.c_str(), debug_mode);
    Src src_video(g_nanotracker_backbone_src_path.c_str() ,{SENSOR_CHANNEL, SENSOR_HEIGHT, SENSOR_WIDTH}, reinterpret_cast<uintptr_t>(vaddr), reinterpret_cast<uintptr_t>(paddr), debug_mode);
    Tracker track(g_nanotracker_head_path.c_str(),g_head_thresh, debug_mode);
    int track_x1;
    int track_y1;
    int track_x2;
    int track_y2;
    float track_mean_x;
    float track_mean_y;
    int draw_mean_x;
    int draw_mean_y;
    float track_w;
    float track_h;
    int draw_mean_w;
    int draw_mean_h;
    bool enter_init = true;
    int seconds = 1;  //倒计时时长，单位秒
    time_t endtime;  //倒计时结束时间
    cv::Mat src_img;
    cv::Mat src_input;
    int flag=0;

    while (!isp_stop)
    {
        ScopedTiming st("total time", 1);
        {
            ScopedTiming st("read capture", debug_mode);
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
            ScopedTiming st("isp copy", debug_mode);
            auto vbvaddr = kd_mpi_sys_mmap_cached(dump_info.v_frame.phys_addr[0], size);
            memcpy(vaddr, (void *)vbvaddr, SENSOR_HEIGHT * SENSOR_WIDTH * 3); 
            kd_mpi_sys_munmap(vbvaddr, size);
        }
        cv::Mat osd_frame(osd_height, osd_width, CV_8UC4, cv::Scalar(0, 0, 0, 0));

        //首先进入手势识别
        if(cur_task_state==0){
            results.clear();
            hd.pre_process();
            hd.inference();
            hd.post_process(results);
            
            for (auto r: results)
            {
                if(results.size()>1){
                    break;
                }
                std::string text = hd.labels_[r.label] + ":" + std::to_string(round(r.score * 100) / 100.0);
                //坐标转换
                int w = r.x2 - r.x1 + 1;
                int h = r.y2 - r.y1 + 1;
                int rect_x = r.x1/ SENSOR_WIDTH * osd_width;
                int rect_y = r.y1/ SENSOR_HEIGHT * osd_height;
                int rect_w = (float)w / SENSOR_WIDTH * osd_width;
                int rect_h = (float)h / SENSOR_HEIGHT  * osd_height;
                //绘制手掌检测框
                cv::rectangle(osd_frame, cv::Rect(rect_x, rect_y , rect_w, rect_h), cv::Scalar( 255,255, 255, 255), 2, 2, 0);
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
                //手掌21个关键点识别
                hk.pre_process(bbox);
                hk.inference();
                hk.post_process(bbox);
                {
                    ScopedTiming st("osd draw keypoints", debug_mode);
                    hk.draw_keypoints(osd_frame, text, bbox, false);
                }
                //识别手势
                std::vector<double> angle_list = hk.hand_angle();
                std::string gesture = hk.h_gesture(angle_list);
                std::string text1 = "Gesture: " + gesture;
                cv::putText(osd_frame, text1, cv::Point(rect_x,rect_y),cv::FONT_HERSHEY_COMPLEX, 1, cv::Scalar(255, 0, 150, 255), 2);
                //如果是“one”手势进入动态手势识别，“yeah”手势进入人脸姿态估计，“three”手势进入人脸单目标跟踪
                if(gesture=="one"){
                    cur_task_state=1;
                    dg_state="middle";
                    continue;
                }
                else if(gesture=="yeah"){
                    cur_task_state=2;
                    continue;
                }
                else if(gesture=="three"){
                    cur_task_state=3;
                    flag=1;
                    enter_init=true;
                    continue;
                }
                
            }
        }
        //动态手势识别逻辑
        else if(cur_task_state==1)
        {
            //如果是等待触发状态，进行正常的手势识别，“love”手势退出动态手势识别，不同方向的“five”手势通过计算角度判断
            //进入哪个方向的已触发状态
            if (cur_state_== TRIGGER)
            {
                ScopedTiming st("trigger time", debug_mode);
                results.clear();
                hd.pre_process();
                hd.inference();
                hd.post_process(results);
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

                    std::vector<double> angle_list = hk.hand_angle();
                    std::string gesture = hk.h_gesture(angle_list);

                    if(gesture=="love"){
                        cur_task_state=0;
                        std::vector<int8_t> frame_data;
                        frame_data.push_back(frame_head);
                        frame_data.push_back(0x02);
                        frame_data.push_back(0x02);
                        frame_data.push_back(0x02);
                        frame_data.push_back(0x02);
                        frame_data.push_back(frame_tail);
                        write(fd1,frame_data.data(), frame_data.size());
                        break;
                    }

                    if (gesture == "five")
                    {
                        double v1_x = hk.results[24] - hk.results[0];
                        double v1_y = hk.results[25] - hk.results[1];
                        double v2_x = 1.0; 
                        double v2_y = 0.0;
                        // 掌根到中指指尖向量和（1，0）向量的夹角
                        double v1_norm = std::sqrt(v1_x * v1_x + v1_y * v1_y);
                        double v2_norm = std::sqrt(v2_x * v2_x + v2_y * v2_y);
                        double dot_product = v1_x * v2_x + v1_y * v2_y;
                        double cos_angle = dot_product / (v1_norm * v2_norm);
                        double angle = std::acos(cos_angle) * 180 / M_PI;

                        if (v1_y>0)
                        {
                            angle = 360-angle;
                        }

                        if ((70.0<=angle) && (angle<110.0))
                        {
                            if ((pre_state_ != UP) || (pre_state_ != MIDDLE))
                            {
                                vec_flag.push_back(pre_state_);
                            }
                            if ((vec_flag.size()>10)||(pre_state_ == UP) || (pre_state_ == MIDDLE) ||(pre_state_ == TRIGGER))
                            {
                                cv::Mat copy_image = osd_frame(cv::Rect(0,0,bin_width,bin_height));
                                shang_argb.copyTo(copy_image); 
                                cur_state_ = UP;
                            }
                        }
                        else if ((110.0<=angle) && (angle<225.0))
                        {
                            // 中指向右(实际方向)
                            if (pre_state_ != RIGHT)
                            {
                                vec_flag.push_back(pre_state_);
                            }
                            if ((vec_flag.size()>10)||(pre_state_ == RIGHT)||(pre_state_ == TRIGGER))
                            {
                                cv::Mat copy_image = osd_frame(cv::Rect(0,0,bin_height,bin_width));
                                you_argb.copyTo(copy_image); 
                                cur_state_ = RIGHT;
                            }
                        }
                        else if((225.0<=angle) && (angle<315.0))
                        {
                            if (pre_state_ != DOWN)
                            {
                                vec_flag.push_back(pre_state_);
                            }
                            if ((vec_flag.size()>10)||(pre_state_ == DOWN)||(pre_state_ == TRIGGER))
                            {
                                cv::Mat copy_image = osd_frame(cv::Rect(0,0,bin_width,bin_height));
                                xia_argb.copyTo(copy_image); 
                                cur_state_ = DOWN;
                            }
                        }
                        else
                        {
                            if (pre_state_ != LEFT)
                            {
                                vec_flag.push_back(pre_state_);
                            }
                            if ((vec_flag.size()>10)||(pre_state_ == LEFT)||(pre_state_ == TRIGGER))
                            {
                                cv::Mat copy_image = osd_frame(cv::Rect(0,0,bin_height,bin_width));
                                zuo_argb.copyTo(copy_image);
                                cur_state_ = LEFT;
                            }
                        }
                        m_start = std::chrono::steady_clock::now();
                    }
                }
                //如果已退出动态手势识别，在循环里要将这一帧插入Display
                if(cur_task_state==0){
                {
                    ScopedTiming st("osd copy", debug_mode);
                    memcpy(pic_vaddr, osd_frame.data, osd_width * osd_height * 4);
                    // 显示通道插入帧
                    kd_mpi_vo_chn_insert_frame(osd_id + 3, &vf_info); 
                    ret = kd_mpi_vicap_dump_release(vicap_dev, VICAP_CHN_ID_1, &dump_info);
                    if (ret)
                    {
                        printf("sample_vicap...kd_mpi_vicap_dump_release failed.\n");
                    }
                
                }
                continue;
            }
                history_logit.clear();
            }
            //如果已进入触发状态，进行动态手势识别，判断向上下左右哪个方向挥手，或者是否五指尖集中于一点表示中间
            else if (cur_state_ != TRIGGER)
            {
                {
                    int matsize = SENSOR_WIDTH * SENSOR_HEIGHT;
                    cv::Mat ori_img;
                    {
                        cv::Mat ori_img_R = cv::Mat(SENSOR_HEIGHT, SENSOR_WIDTH, CV_8UC1, vaddr);
                        cv::Mat ori_img_G = cv::Mat(SENSOR_HEIGHT, SENSOR_WIDTH, CV_8UC1, vaddr + 1 * matsize);
                        cv::Mat ori_img_B = cv::Mat(SENSOR_HEIGHT, SENSOR_WIDTH, CV_8UC1, vaddr + 2 * matsize);
                        std::vector<cv::Mat> sensor_rgb;
                        sensor_rgb.push_back(ori_img_B);
                        sensor_rgb.push_back(ori_img_G);
                        sensor_rgb.push_back(ori_img_R); 
                        cv::merge(sensor_rgb, ori_img);
                    }
                    Dag.pre_process(ori_img);
                    Dag.inference();
                    Dag.post_process();
                    vector<float> avg_logit;
                    {
                        vector<float> output;
                        Dag.get_out(output);
                        history_logit.push_back(output);

                        for(int j=0;j<27;j++)
                        {
                            float sum = 0.0;
                            for (int i=0;i<history_logit.size();i++)
                            {
                                sum += history_logit[i][j];
                            }
                            avg_logit.push_back(sum / history_logit.size());
                        }
                        idx_ = std::distance(avg_logit.begin(), std::max_element(avg_logit.begin(), avg_logit.end()));
                    }

                    idx = Dag.process_output(idx_, history);
                    if (idx!=idx_)
                    {
                        vector<float> history_logit_last = history_logit.back();
                        history_logit.clear();
                        history_logit.push_back(history_logit_last);
                    }
                    if (cur_state_ == UP)
                    {
                        cv::Mat copy_image = osd_frame(cv::Rect(0,0,bin_width,bin_height));
                        shang_argb.copyTo(copy_image); 
                        if ((idx==15) || (idx==10))
                        {
                            vec_flag.clear();
                            if (((avg_logit[idx] >= 0.7) && (history_logit.size() >= 2)) || ((avg_logit[idx] >= 0.3) && (history_logit.size() >= 4)))
                            {
                                s_start = std::chrono::steady_clock::now();
                                cur_state_ = TRIGGER;
                                draw_state_ = DOWN;
                                history.clear();
                            }
                            pre_state_ = UP;
                        }else if ((idx==25)||(idx==26)) 
                        {
                            vec_flag.clear();
                            if (((avg_logit[idx] >= 0.4) && (history_logit.size() >= 2)) || ((avg_logit[idx] >= 0.3) && (history_logit.size() >= 3)))
                            {
                                s_start = std::chrono::steady_clock::now();
                                cur_state_ = TRIGGER;
                                draw_state_ = MIDDLE;
                                history.clear();
                            }
                            pre_state_ = MIDDLE;
                        }else
                        {
                            history_logit.clear();
                        }
                    }
                    else if (cur_state_ == RIGHT)
                    {
                        cv::Mat copy_image = osd_frame(cv::Rect(0,0,bin_height,bin_width));
                        you_argb.copyTo(copy_image); 
                        if  ((idx==16)||(idx==11)) 
                        {
                            vec_flag.clear();
                            if (((avg_logit[idx] >= 0.4) && (history_logit.size() >= 2)) || ((avg_logit[idx] >= 0.3) && (history_logit.size() >= 3)))
                            {
                                s_start = std::chrono::steady_clock::now();
                                cur_state_ = TRIGGER;
                                draw_state_ = RIGHT;
                                history.clear();
                            }
                            pre_state_ = RIGHT;
                        }else
                        {
                            history_logit.clear();
                        }
                    }
                    else if (cur_state_ == DOWN)
                    {
                        cv::Mat copy_image = osd_frame(cv::Rect(0,0,bin_width,bin_height));
                        xia_argb.copyTo(copy_image); 
                        if  ((idx==18)||(idx==13))
                        {
                            vec_flag.clear();
                            if (((avg_logit[idx] >= 0.4) && (history_logit.size() >= 2)) || ((avg_logit[idx] >= 0.3) && (history_logit.size() >= 3)))
                            {
                                s_start = std::chrono::steady_clock::now();
                                cur_state_ = TRIGGER;
                                draw_state_ = UP;
                                history.clear();
                            }
                            pre_state_ = DOWN;
                        }else
                        {
                            history_logit.clear();
                        }
                    }
                    else if (cur_state_ == LEFT)
                    {
                        cv::Mat copy_image = osd_frame(cv::Rect(0,0,bin_height,bin_width));
                        zuo_argb.copyTo(copy_image);
                        if ((idx==17)||(idx==12))
                        {
                            vec_flag.clear();
                            if (((avg_logit[idx] >= 0.4) && (history_logit.size() >= 2)) || ((avg_logit[idx] >= 0.3) && (history_logit.size() >= 3)))
                            {
                                s_start = std::chrono::steady_clock::now();
                                cur_state_ = TRIGGER;
                                draw_state_ = LEFT;
                                history.clear();
                            }
                            pre_state_ = LEFT;
                        }else
                        {
                            history_logit.clear();
                        }
                    }
                }
                m_stop = std::chrono::steady_clock::now();
                double elapsed_ms = std::chrono::duration<double, std::milli>(m_stop - m_start).count();

                if ((cur_state_ != TRIGGER) &&(elapsed_ms>2000))
                {
                    cur_state_ = TRIGGER;
                    pre_state_ = TRIGGER;
                }
            }
            s_stop = std::chrono::steady_clock::now();
            double elapsed_ms_show = std::chrono::duration<double, std::milli>(s_stop - s_start).count();

            //持续一段时间的显示动态手势识别的结果，同时按照识别结果发送帧数据
            if (elapsed_ms_show<1000)
            {
               int c_x=osd_width/2-50;
               int c_y=osd_height/2;
               if (draw_state_ == UP)
                {
                    cv::putText(osd_frame, "up", cv::Point(c_x,c_y),cv::FONT_HERSHEY_COMPLEX, 5, cv::Scalar(255, 255, 0, 0), 2);
                    //如果当前状态是“middel”才发送一帧“up”数据
                    if(dg_state=="middle"){
                        std::vector<int8_t> frame_data;
                        int8_t deviceNumber = 0;
                        int8_t commandNumber = 0;
                        int8_t dataLength = 1; 
                        int8_t data=0;
                        frame_data.push_back(frame_head);
                        frame_data.push_back(deviceNumber);
                        frame_data.push_back(commandNumber);
                        frame_data.push_back(dataLength);
                        frame_data.push_back(data);
                        frame_data.push_back(frame_tail);
                        write(fd1,frame_data.data(), frame_data.size());
                    }
                    dg_state="up";
                } else if (draw_state_ == RIGHT)
                {
                    cv::putText(osd_frame, "left", cv::Point(c_x,c_y),cv::FONT_HERSHEY_COMPLEX, 5, cv::Scalar(255, 255, 0, 0), 2);
                    //如果当前状态是“middel”才发送一帧“left”数据
                    if(dg_state=="middle"){
                        std::vector<int8_t> frame_data;
                        int8_t deviceNumber = 0;
                        int8_t commandNumber = 1;
                        int8_t dataLength = 1; 
                        int8_t data=1;
                        frame_data.push_back(frame_head);
                        frame_data.push_back(deviceNumber);
                        frame_data.push_back(commandNumber);
                        frame_data.push_back(dataLength);
                        frame_data.push_back(data);
                        frame_data.push_back(frame_tail);
                        write(fd1,frame_data.data(), frame_data.size());
                    }
                    dg_state="left";
                }else if (draw_state_ == DOWN)
                {
                    cv::putText(osd_frame, "down", cv::Point(c_x,c_y),cv::FONT_HERSHEY_COMPLEX, 5, cv::Scalar(255, 255, 0, 0), 2);
                    //如果当前状态是“middel”才发送一帧“down”数据
                    if(dg_state=="middle"){
                        std::vector<int8_t> frame_data;
                        int8_t deviceNumber = 0;
                        int8_t commandNumber = 2;
                        int8_t dataLength = 1; 
                        int8_t data=2;
                        frame_data.push_back(frame_head);
                        frame_data.push_back(deviceNumber);
                        frame_data.push_back(commandNumber);
                        frame_data.push_back(dataLength);
                        frame_data.push_back(data);
                        frame_data.push_back(frame_tail);
                        write(fd1,frame_data.data(), frame_data.size());
                    }
                    dg_state="down";
                }else if (draw_state_ == LEFT)
                {
                    cv::putText(osd_frame, "right", cv::Point(c_x,c_y),cv::FONT_HERSHEY_COMPLEX, 5, cv::Scalar(255, 255, 0, 0), 2);
                    //如果当前状态是“middel”才发送一帧“right”数据
                    if(dg_state=="middle"){
                        std::vector<int8_t> frame_data;
                        int8_t deviceNumber = 0;
                        int8_t commandNumber = 3;
                        int8_t dataLength = 1; 
                        int8_t data=3;
                        frame_data.push_back(frame_head);
                        frame_data.push_back(deviceNumber);
                        frame_data.push_back(commandNumber);
                        frame_data.push_back(dataLength);
                        frame_data.push_back(data);
                        frame_data.push_back(frame_tail);
                        write(fd1,frame_data.data(), frame_data.size());
                    }
                    dg_state="right";
                }else if (draw_state_ == MIDDLE)
                {
                    cv::putText(osd_frame, "middle", cv::Point(c_x,c_y),cv::FONT_HERSHEY_COMPLEX, 5, cv::Scalar(255, 255, 0, 0), 2);
                    //如果当前状态不是“middel”发送一帧“middle”数据
                    if(dg_state!="middle"){
                        std::vector<int8_t> frame_data;
                        int8_t deviceNumber = 0;
                        int8_t commandNumber = 4;
                        int8_t dataLength = 1; 
                        int8_t data=4;
                        frame_data.push_back(frame_head);
                        frame_data.push_back(deviceNumber);
                        frame_data.push_back(commandNumber);
                        frame_data.push_back(dataLength);
                        frame_data.push_back(data);
                        frame_data.push_back(frame_tail);
                        write(fd1,frame_data.data(), frame_data.size());
                    }
                    dg_state="middle";
                }

            }else
            {
                draw_state_ = TRIGGER;
            }
        }
        //人脸姿态角估计的逻辑
        else if(cur_task_state==2)
        {
            //先进行手势识别
            results.clear();
            hd.pre_process();
            hd.inference();
            hd.post_process(results);

            for (auto r: results)
            {
                std::string text = hd.labels_[r.label] + ":" + std::to_string(round(r.score * 100) / 100.0);
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

                std::vector<double> angle_list = hk.hand_angle();
                std::string gesture = hk.h_gesture(angle_list);
                std::string text1 = "Gesture: " + gesture;
               //如果识别到“love”手势，退出当前任务
                if(gesture=="love"){
                    cur_task_state=0;
                    std::vector<int8_t> frame_data;
                    frame_data.push_back(frame_head);
                    frame_data.push_back(0x02);
                    frame_data.push_back(0x02);
                    frame_data.push_back(0x02);
                    frame_data.push_back(0x02);
                    frame_data.push_back(frame_tail);
                    write(fd1,frame_data.data(), frame_data.size());
                    break;
                }
            }
            if(cur_task_state==0){
                {
                    ScopedTiming st("osd copy", debug_mode);
                    memcpy(pic_vaddr, osd_frame.data, osd_width * osd_height * 4);
                    // 显示通道插入帧
                    kd_mpi_vo_chn_insert_frame(osd_id + 3, &vf_info); // K_VO_OSD0
                    ret = kd_mpi_vicap_dump_release(vicap_dev, VICAP_CHN_ID_1, &dump_info);
                    if (ret)
                    {
                        printf("sample_vicap...kd_mpi_vicap_dump_release failed.\n");
                    }
                }
                continue;
            }

            //首先进行人脸检测
            det_results.clear();
            fd.pre_process();
            fd.inference();
            fd.post_process({SENSOR_WIDTH, SENSOR_HEIGHT}, det_results);
            //如果识别到人脸，选择距离display中心点最近的人做姿态估计
            if(det_results.size()>=1){
                int center_x=osd_width/2;
                int center_y=osd_height/2;
                int distance=center_x*center_x+center_y*center_y;
                int idx=0;
                for(int i=0;i<det_results.size();i++){
                    int x = det_results[i].bbox.x / SENSOR_WIDTH * osd_width;
                    int y = det_results[i].bbox.y / SENSOR_HEIGHT * osd_height;
                    int w = det_results[i].bbox.w / SENSOR_WIDTH * osd_width;
                    int h = det_results[i].bbox.h / SENSOR_HEIGHT * osd_height;
                    int c_x=x+w/2;
                    int c_y=y+h/2;
                    int c_w=std::abs(center_x-c_x);
                    int c_h=std::abs(center_y-c_y);
                    int dis_idx=c_w*c_w+c_h*c_h;
                    if(dis_idx<distance){
                        distance=dis_idx;
                        idx=i;
                    }

                }
                //进行人脸姿态估计
                fp.pre_process(det_results[idx].bbox);
                fp.inference();
                fp.post_process(pose_result);
                {
                    ScopedTiming st("osd draw", debug_mode);
                    string pitch_text="pitch:"+std::to_string(pose_result.pitch);
                    string roll_text="roll:"+std::to_string(pose_result.roll);
                    string yaw_text="yaw:"+std::to_string(pose_result.yaw);
                    cv::putText(osd_frame, pitch_text, cv::Point(50,50),cv::FONT_HERSHEY_COMPLEX, 2, cv::Scalar(255, 255, 0, 0), 2);
                    cv::putText(osd_frame, roll_text, cv::Point(50,100),cv::FONT_HERSHEY_COMPLEX, 2, cv::Scalar(255, 255, 0, 0), 2);
                    cv::putText(osd_frame, yaw_text, cv::Point(50,150),cv::FONT_HERSHEY_COMPLEX, 2, cv::Scalar(255, 255, 0, 0), 2);
                    fp.draw_result(osd_frame,det_results[idx].bbox,pose_result,false);
                    //发送帧数据，包括相对三轴正方向的夹角
                    std::vector<int8_t> frame_data;
                    int8_t deviceNumber = 1;
                    int8_t commandNumber = 0;
                    int8_t dataLength = 3; 
                    frame_data.push_back(frame_head);
                    frame_data.push_back(deviceNumber);
                    frame_data.push_back(commandNumber);
                    frame_data.push_back(dataLength);
                    frame_data.push_back(static_cast<int8_t>(pose_result.pitch));
                    frame_data.push_back(static_cast<int8_t>(pose_result.roll));
                    frame_data.push_back(static_cast<int8_t>(pose_result.yaw));
                    frame_data.push_back(frame_tail);
                    write(fd1,frame_data.data(), frame_data.size());
                }
            }
        }
        //单目标人脸跟踪逻辑
        else if(cur_task_state==3)
        {
            //先进行手势识别，如果识别到“love”手势，退出当前任务
            results.clear();
            hd.pre_process();
            hd.inference();
            hd.post_process(results);
            for (auto r: results)
            {
                std::string text = hd.labels_[r.label] + ":" + std::to_string(round(r.score * 100) / 100.0);
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
                std::vector<double> angle_list = hk.hand_angle();
                std::string gesture = hk.h_gesture(angle_list);
                std::string text1 = "Gesture: " + gesture;
                //发送退出帧数据
                if(gesture=="love"){
                    cur_task_state=0;
                    std::vector<int8_t> frame_data;
                        frame_data.push_back(frame_head);
                        frame_data.push_back(0x02);
                        frame_data.push_back(0x02);
                        frame_data.push_back(0x02);
                        frame_data.push_back(0x02);
                        frame_data.push_back(frame_tail);
                        write(fd1,frame_data.data(), frame_data.size());
                        break;
                }
            }
            if(cur_task_state==0){
                {
                    ScopedTiming st("osd copy", debug_mode);
                    memcpy(pic_vaddr, osd_frame.data, osd_width * osd_height * 4);
                    kd_mpi_vo_chn_insert_frame(osd_id + 3, &vf_info);
                    ret = kd_mpi_vicap_dump_release(vicap_dev, VICAP_CHN_ID_1, &dump_info);
                    if (ret)
                    {
                        printf("sample_vicap...kd_mpi_vicap_dump_release failed.\n");
                    }
                
                }
                continue;
            }
            //如果是刚完成任务切换，需要检测距离display中心点最近的人脸，作为跟踪目标
            if(flag==1){
                det_results.clear();
                fd.pre_process();
                fd.inference();
                fd.post_process({SENSOR_WIDTH, SENSOR_HEIGHT}, det_results);
                if(det_results.size()>=1){
                    int center_x=osd_width/2;
                    int center_y=osd_height/2;
                    int distance=center_x*center_x+center_y*center_y;
                    int idx=-1;
                    for(int i=0;i<det_results.size();i++){
                        int x = det_results[i].bbox.x / SENSOR_WIDTH * osd_width;
                        int y = det_results[i].bbox.y / SENSOR_HEIGHT * osd_height;
                        int w = det_results[i].bbox.w / SENSOR_WIDTH * osd_width;
                        int h = det_results[i].bbox.h / SENSOR_HEIGHT * osd_height;
                        int c_x=x+w/2;
                        int c_y=y+h/2;
                        int c_w=std::abs(center_x-c_x);
                        int c_h=std::abs(center_y-c_y);
                        int dis_idx=c_w*c_w+c_h*c_h;
                        if(dis_idx<distance){
                            distance=dis_idx;
                            idx=i;
                        }

                    }
                    track_x1=det_results[idx].bbox.x;
                    track_y1=det_results[idx].bbox.y;
                    track_x2=det_results[idx].bbox.x+det_results[0].bbox.w;
                    track_y2=det_results[idx].bbox.y+det_results[0].bbox.h;
                    track_mean_x = (track_x2 + track_x1) / 2.0;
                    track_mean_y = (track_y1 + track_y2) / 2.0;
                    track_w = float(track_x2 - track_x1);
                    track_h = float(track_y2 - track_y1);
                    draw_mean_w = int(track_w / SENSOR_WIDTH * osd_width);
                    draw_mean_h = int(track_h / SENSOR_HEIGHT * osd_height);
                    draw_mean_x = int(track_mean_x / SENSOR_WIDTH * osd_width - draw_mean_w / 2.0);
                    draw_mean_y = int(track_mean_y / SENSOR_HEIGHT * osd_height - draw_mean_h / 2.0);
                    set_center(track_mean_x, track_mean_y);
                    set_rect_size(track_w, track_h);
                    endtime = time(NULL) + seconds;  
                    flag=0;
                }
                else{
                    {
                        ScopedTiming st("osd copy", debug_mode);
                        memcpy(pic_vaddr, osd_frame.data, osd_width * osd_height * 4);
                        kd_mpi_vo_chn_insert_frame(osd_id + 3, &vf_info); 
                        ret = kd_mpi_vicap_dump_release(vicap_dev, VICAP_CHN_ID_1, &dump_info);
                        if (ret)
                        {
                            printf("sample_vicap...kd_mpi_vicap_dump_release failed.\n");
                        }
                    }
                    continue;
                }
            }
            
            //首先对整帧图像做预处理
            float w_z, h_z, s_z;
            float *pos = get_updated_position();
            w_z = pos[0] + CONTEXT_AMOUNT * (pos[0] + pos[1]);
            h_z = pos[1] + CONTEXT_AMOUNT * (pos[0] + pos[1]);
            s_z = round(sqrt(w_z * h_z));
            src_video.pre_process_video(src_img);
            src_input = sub_window(src_img, INSTANCE_SIZE, round(s_z * INSTANCE_SIZE / EXEMPLAR_SIZE));
            time_t nowtime = time(NULL);
            //在跟踪框初始化阶段，从图像中crop出待跟踪目标，经backbone处理成featuremap，等待后续跟踪使用
            if (enter_init && nowtime <= endtime)
            {
                cv::rectangle(osd_frame, cv::Rect( draw_mean_x,draw_mean_y,draw_mean_w,draw_mean_h ), cv::Scalar(255, 0,255, 0), 8, 2, 0); // ARGB
                cv::Mat crop_img;
                crop_video.pre_process_video(crop_img);
                cv::Mat crop_input = sub_window(crop_img, EXEMPLAR_SIZE, round(s_z));
                crop.pre_process(crop_input);
                crop.inference();
                crop.post_process();
                if (nowtime > endtime)
                {
                    enter_init = false;
                }
            }
            //跟踪阶段
            else
            {
                
               //对每一帧图像经过backbone处理成featuremap
                src.pre_process(src_input);
                src.inference();
                src.post_process();
                //将两个输出的featuremap输入到对比模型中，从回归结果中得到跟踪框
                std::vector<float*> inputs;
                inputs.clear();
                inputs.push_back(crop.output);
                inputs.push_back(src.output);
                track.pre_process(inputs);
                track.inference();
                std::vector<Tracker_box> track_boxes;
                track.post_process(SENSOR_WIDTH, SENSOR_HEIGHT,track_boxes);
                track.draw_track(track_boxes,{SENSOR_WIDTH, SENSOR_HEIGHT}, {osd_width,osd_height}, osd_frame);        
                int c_x=osd_width/2;
                int c_y=osd_height/2;
                int rec_s=osd_height*0.5;
                int x_flag=0;
                int y_flag=0;
                string res_track="";
                std::vector<int8_t> frame_data;
                int8_t deviceNumber = 1;
                int8_t commandNumber = 1;
                int8_t dataLength = 2; 
                int8_t data_1=0;
                int8_t data_2=0;
                
                //根据跟踪框偏离边长为0.5*osd_height方框的相对位置，组织帧数据发送
                if (track_boxes.size())
                {

                    int r_x1 = track_boxes[0].x;
                    int r_y1 = track_boxes[0].y;
                    int r_x2 = (r_x1+track_boxes[0].w);
                    int r_y2 = (r_y1+track_boxes[0].h);
                    int x1 =   r_x1*1.0 / SENSOR_WIDTH * osd_width;
                    int y1 =    r_y1*1.0 / SENSOR_HEIGHT * osd_height;
                    int w = (r_x2-r_x1)*1.0 / SENSOR_WIDTH * osd_width;
                    int h = (r_y2-r_y1)*1.0 / SENSOR_HEIGHT * osd_height;

                    int b_x = (x1+w/2);
                    int b_y = (y1+h/2);
                    if(b_x>(c_x+rec_s/2)){
                        data_1=1;//向左
                    }
                    if(b_x<(c_x-rec_s/2)){
                        data_1=2;//向右
                    }
                    if(b_y>(c_y+rec_s/2)){
                        data_2=1;//向下
                    }
                    if(b_y<(c_y-rec_s/2)){
                        data_2=2;//向上
                    }
                }
                frame_data.push_back(frame_head);
                frame_data.push_back(deviceNumber);
                frame_data.push_back(commandNumber);
                frame_data.push_back(dataLength);
                frame_data.push_back(data_1);
                frame_data.push_back(data_2);
                frame_data.push_back(frame_tail);
                write(fd1,frame_data.data(), frame_data.size());
                
            }
        }

        {
            ScopedTiming st("osd copy", debug_mode);
            memcpy(pic_vaddr, osd_frame.data, osd_width * osd_height * 4);
            kd_mpi_vo_chn_insert_frame(osd_id + 3, &vf_info);
            ret = kd_mpi_vicap_dump_release(vicap_dev, VICAP_CHN_ID_1, &dump_info);
            if (ret)
            {
                printf("sample_vicap...kd_mpi_vicap_dump_release failed.\n");
            }
        
        }
    }

    close(fd1);
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
    std::thread thread_isp(video_proc);
    while (getchar() != 'q')
    {
        usleep(10000);
    }
    isp_stop = true;
    thread_isp.join();
    return 0;
}




