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
#ifndef _HAND_DETECTION_H
#define _HAND_DETECTION_H

#include <cmath>
#include <fstream>
#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <stdint.h>

//#include <nncase/runtime/host_runtime_tensor.h>
#include <nncase/runtime/interpreter.h>
#include <nncase/runtime/runtime_tensor.h>

#include "utils.h"
#include "ai_base.h"

using namespace nncase;
using namespace nncase::runtime;
using namespace nncase::runtime::detail;
using namespace std;

/**
 * @brief 手掌检测
 * 主要封装了对于每一帧图片，从预处理、运行到后处理给出结果的过程
 */
class HandDetection:public AIBase
{
public:
    /**
     * @brief HandDetection构造函数，加载kmodel,并初始化kmodel输入、输出
     * @param kmodel_file kmodel文件路径
     * @param obj_thresh 手掌检测阈值，用于过滤roi
     * @param nms_thresh 手掌检测框阈值，用于过滤重复roi
     * @param frame_size 手掌检测输入图片尺寸
     * @param debug_mode  0（不调试）、 1（只显示时间）、2（显示所有打印信息）
     * @return None
     */

    // for image
    HandDetection(const char *kmodel_file, float obj_thresh, float nms_thresh, FrameSize frame_size,const int debug_mode);

    /**
     * @brief HandDetection构造函数，加载kmodel,并初始化kmodel输入、输出和手掌检测阈值
     * @param kmodel_file kmodel文件路径
     * @param obj_thresh 手掌检测阈值，用于过滤roi
     * @param nms_thresh 手掌检测框阈值，用于过滤重复roi
     * @param frame_size 手掌检测输入图片尺寸
     * @param isp_shape   isp输入大小（chw）
     * @param vaddr       isp对应虚拟地址
     * @param paddr       isp对应物理地址
     * @param debug_mode  0（不调试）、 1（只显示时间）、2（显示所有打印信息）
     * @return None
     */

    // for_video
    HandDetection(const char *kmodel_file, float obj_thresh, float nms_thresh, FrameSize frame_size, FrameCHWSize isp_shape, uintptr_t vaddr, uintptr_t paddr, const int debug_mode);

    /**
     * @brief HandDetection析构函数
     * @return None
     */
    ~HandDetection();

    /**
     * @brief 图片预处理
     * @param ori_img 原始图片
     * @return None
     */
    void pre_process(cv::Mat ori_img);

    /**
     * @brief 视频流预处理（ai2d for isp）
     * @return None
     */
    void pre_process();

    /**
     * @brief kmodel推理
     * @return None
     */
    void inference();

    /**
     * @brief kmodel推理结果后处理
     * @param results 后处理之后的基于原始图像的{检测框坐标点、得分和标签}集合
     * @return None
     */
    void post_process(std::vector<BoxInfo> &result);

    std::vector<std::string> labels_ = {"hand"}; //模型输出类别名称


private:

    std::unique_ptr<ai2d_builder> ai2d_builder_; // ai2d构建器
    runtime_tensor ai2d_in_tensor_;              // ai2d输入tensor
    runtime_tensor ai2d_out_tensor_;             // ai2d输出tensor
    uintptr_t vaddr_;                            // isp的虚拟地址
    FrameCHWSize isp_shape_;                     // isp对应的地址大小

    float obj_thresh_;     // 手掌检测阈值
    float nms_thresh_;     // 手掌检测框nms阈值
    FrameSize frame_size_; // 输入图片尺寸
    int classes_num_;      // 模型输出类别数

    float anchors_0[3][2] = { { 26,27 }, { 53,52 }, { 75,71 } };
    float anchors_1[3][2] = { { 80,99 }, { 106,82 }, { 99,134 } };
    float anchors_2[3][2] = { { 140,113 }, { 161,172 }, { 245,276 } };

    /**
     * @brief 非极大值抑制
     * @param input_boxes     后处理之后的基于原始图像的{检测框坐标点、得分和标签}集合
     * @return None
     */
    void nms(std::vector<BoxInfo> &input_boxes);

    /**
     * @brief 预处理模型输出结果
     * @param data              指向模型推理得到的首个roi置信度的指针
     * @param stride            模型推理得到的feature相比模型输入的下采样倍数
     * @param frame_size        原始图像/帧宽高，用于将结果放到原始图像大小
     * @param anchors           模型推理得到的feature对应的anchor
     * @return                  每个feature对应的结果映射回原始图像的{检测框坐标点、得分和标签}集合
     */
    std::vector<BoxInfo> decode_infer(float *data, int stride, FrameSize frame_size, float anchors[][2]);
};
#endif
