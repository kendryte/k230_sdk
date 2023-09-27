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
#ifndef _E3D_H
#define _E3D_H

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

#include <nncase/runtime/interpreter.h>
#include <nncase/runtime/runtime_tensor.h>

#include "utils.h"
#include "ai_base.h"


using namespace nncase;
using namespace nncase::runtime;
using namespace nncase::runtime::detail;
using namespace std;

/**
 * @brief 手部3D关键点检测
 * 主要封装了对于每一帧图片，从预处理、运行到后处理给出结果的过程
 */
class E3d: public AIBase
{
public:
    /**
     * @brief E3d构造函数，加载kmodel,并初始化kmodel输入、输出
     * @param kmodel_file kmodel文件路径
     * @param debug_mode  0（不调试）、 1（只显示时间）、2（显示所有打印信息）
     * @return None
     */
    E3d(const char *kmodel_file,const int debug_mode=0);

    /**
     * @brief E3d析构函数
     * @return None
     */
    ~E3d();

    /**
     * @brief 图片预处理
     * @param ori_img 原始图片
     * @param results 2D关键点在输入图像上的位置坐标
     * @return None
     */
    void pre_process(cv::Mat ori_img, std::vector<int> results);

    /**
     * @brief kmodel推理
     * @return None
     */
    void inference();

    /**
     * @brief 将3D关键点坐标保存为bin文件
     * @param idx 当前帧数
     * @return None
     */
    void post_process(int idx, int buffer_size);

    /**
     * @brief 根据单个2D关键点位置坐标和高斯核生成单张heatmap
     * @param heatmap  heatmap
     * @param center_x 单个关键点x坐标
     * @param center_y 单个关键点y坐标
     * @param radius   高斯核半径
     * @param ratio    输入图像预处理时的缩放比例
     * @param dw       输入图像预处理时列方向的左右padding长度
     * @param dh       输入图像预处理时行方向的上下padding长度
     * @return None
     */
    void draw_umich_gaussian(cv::Mat & heatmap, const int &center_x, const int &center_y, int radius, float &ratio, float &dw, float &dh);
    
    /**
     * @brief 根据2D关键点位置坐标和高斯核生成heatmap
     * @param handpose_2d 2D关键点在输入图像上的位置坐标
     * @param ratio       输入图像预处理时的缩放比例
     * @param dw          输入图像预处理时列方向的左右padding长度
     * @param dh          输入图像预处理时行方向的上下padding长度
     * @return heatmaps
     */
    std::vector<cv::Mat> get_heatmap(const std::vector<int>& handpose_2d, float &ratio, float &dw, float &dh);

private:
    runtime_tensor in_tensor_;                   // 输入tensor
    FrameCHWSize isp_shape_;                     // isp对应的地址大小

    float* gaussian_data_;   //高斯核
    float* data;             //模型输入数据
};
#endif
