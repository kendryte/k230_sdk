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
 * 
 * Part of the code is referenced from https://github.com/mcximing/sort-cpp. The orignal code are published under the BSD license.
 */
#ifndef SORT_H
#define SORT_H

#include <vector>
#include <opencv2/opencv.hpp>
#include "my_tracker.h"
#include "utils.h"

//using namespace std;

class Sort
{
public:

    /**
     * @brief 跟踪框信息
     */
    typedef struct TrackingBox{
        int frame;            //视频中的第几帧
        int id;               //跟踪目标的id
        cv::Rect_<float> box; //跟踪框信息：x,y,w,h
    }TrackingBox;

    std::vector<MyTracker> trackers; //跟踪目标信息
    std::vector<std::vector<TrackingBox>> detFrameData; //检测框的信息

    /**
     * @brief      根据检测框对跟踪目标信息进行更新
     * @param bbox 检测框的信息
     * @param fi   当前帧数
     * @return     已更新的跟踪目标信息
     */
    std::vector<TrackingBox> Sortx(std::vector<BoxInfo> bbox, int fi);

    /**
     * @brief 计算两个矩形框的IOU（交并比）
     * @param bb_dr 需要计算IOU的矩形框1
     * @param bb_gt 需要计算IOU的矩形框2
     * @return 计算得到的IOU值
     */
    double GetIOU(cv::Rect_<float> bb_dr, cv::Rect_<float> bb_gt);

};

#endif