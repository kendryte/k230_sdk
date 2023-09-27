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

#ifndef _POSE_ACTION_
#define _POSE_ACTION_

#include <vector>

#include <vector>
#include <iostream>
#include <fstream>
#include <opencv2/highgui.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/imgproc.hpp>
using namespace cv;

/**
 * @brief 关键点信息
 */
struct KKeyPoint
{
    cv::Point2f p;  // 关键点
    float prob;     // 关键点概率值
};


/** 
 * @brief action帮助信息
 */
struct action_helper {
  bool mark = false;  // 是否标记
  int action_count = 0;  // action计数
  int latency = 0;  // 延迟次数
};


/**
 * @brief PoseAction 类
 * 封装了PoseAction类常用的函数，包括清除动作计数函数、获取动作计数函数、获取xy比例函数、获取xyhigher函数、多种动作检查函数等
 */
class PoseAction
{
    public:

        /** 
        * @brief 获取“深蹲”次数
        * @param kpts_sframe  每帧关键点
        * @param recid  某种动作记录编号
        * @param thres_conf  关键点阈值
        * @return 深蹲次数
        */
        static int check_deep_down(std::vector<KKeyPoint> &kpts_sframe, int recid,float thres_conf);

        /** 
        * @brief 单一动作检查
        * @param results_kpts  关键点结果
        * @param thres_conf  关键点阈值
        * @param actionid  动作编号
        * @param recid  某种动作记录编号
        * @return 动作次数
        */ 
        static int single_action_check(std::vector<KKeyPoint> &results_kpts, float thres_conf, int actionid, int recid);

};

#endif