/* Copyright (c) 2022, Canaan Bright Sight Co., Ltd
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
#ifndef _DYNAMIC_GESTURE_H
#define _DYNAMIC_GESTURE_H

#include <iostream>
#include <vector>

#include "utils.h"
#include "ai_base.h"

/**
 * @brief 手势识别
 * 主要封装了对于每一帧图片，从预处理、运行到后处理给出结果的过程
 */
class DynamicGesture : public AIBase
{
public:

    /**
     * @brief DynamicGesture构造函数，加载kmodel,并初始化kmodel输入、输出
     * @param kmodel_file kmodel文件路径
     * @param debug_mode  0（不调试）、 1（只显示时间）、2（显示所有打印信息）
     * @return None
     */
    DynamicGesture(const char *kmodel_file, const int debug_mode = 1);

    /**
     * @brief DynamicGesture析构函数
     * @return None
     */
    ~DynamicGesture();

    /**
     * @brief 图片预处理
     * @param ori_img 原始图片
     * @return None
     */
    void pre_process(cv::Mat ori_img);

    /**
     * @brief kmodel推理
     * @return None
     */
    void inference();

    /**
     * @brief 当前帧kmodel推理结果传入下一帧
     * @return None
     */
    void post_process();

    /**
     * @brief 返回kmodel推理结果后处理
     * @return 识别结果
     */
    void get_out(vector<float> &output);

    void softmax(float* x, float* dx, uint32_t len);

    std::vector<std::string> labels
    {
        "Doing other things",  // 0
        "Drumming Fingers",  // 1
        "No gesture",  // 2
        "Pulling Hand In",  // 3
        "Pulling Two Fingers In",  // 4
        "Pushing Hand Away",  // 5
        "Pushing Two Fingers Away",  // 6
        "Rolling Hand Backward",  // 7
        "Rolling Hand Forward",  // 8
        "Shaking Hand",  // 9
        "Sliding Two Fingers Down",  // 10
        "Sliding Two Fingers Left",  // 11
        "Sliding Two Fingers Right",  // 12
        "Sliding Two Fingers Up",  // 13
        "Stop Sign",  // 14
        "Swiping Down",  // 15
        "Swiping Left",  // 16
        "Swiping Right",  // 17
        "Swiping Up",  // 18
        "Thumb Down",  // 19
        "Thumb Up",  // 20
        "Turning Hand Clockwise",  // 21
        "Turning Hand Counterclockwise",  // 22
        "Zooming In",  // 23
        "Zooming In With Two Fingers",  // 24
        "Zooming Out",  // 25
        "Zooming Out With Two Fingers"  // 26
    };

    /**
     * @brief 返回kmodel推理结果后处理
     * @param pred    kmodel推理类别
     * @param history 多帧kmodel推理结果
     * @return        最终的识别类别
     */
    int process_output(int pred, std::vector<int>& history);

private:

    std::vector<runtime_tensor> in_tensors_;    //输入tensor
    std::vector<float*> input_bins;             //kmodel的输入和输出数据

    const int max_hist_len = 20;                //最多存储多少帧的结果
};
#endif
