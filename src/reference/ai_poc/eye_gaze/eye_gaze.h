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
#ifndef _EYE_GAZE_H
#define _EYE_GAZE_H

#include <vector>
#include "utils.h"
#include "ai_base.h"

using std::vector;

/**
 * @brief 注视估计结果
 */
typedef struct EyeGazeInfo
{
    float yaw;                     //eular[1],偏航角,左右移动
	float pitch;                   //eular[0],俯仰角,上下移动
} EyeGazeInfo;

/**
 * @brief 注视估计
 * 主要封装了对于每一帧图片，从预处理、运行到后处理给出结果的过程
 */
class EyeGaze : public AIBase
{
public:
    /**
     * @brief EyeGaze构造函数，加载kmodel,并初始化kmodel输入、输出(for image)
     * @param kmodel_file       kmodel文件路径
     * @param debug_mode        0（不调试）、 1（只显示时间）、2（显示所有打印信息）
     * @return None
     */
    EyeGaze(const char *kmodel_file, const int debug_mode);

    /**
     * @brief EyeGaze构造函数，加载kmodel,并初始化kmodel输入、输出和人脸检测阈值(for isp)
     * @param kmodel_file       kmodel文件路径
     * @param isp_shape         isp输入大小（chw）
     * @param vaddr             isp对应虚拟地址
     * @param paddr             isp对应物理地址
     * @param debug_mode        0（不调试）、 1（只显示时间）、2（显示所有打印信息）
     * @return None
     */
    EyeGaze(const char *kmodel_file, FrameCHWSize isp_shape, uintptr_t vaddr, uintptr_t paddr, const int debug_mode);

    /**
     * @brief EyeGaze析构函数
     * @return None
     */
    ~EyeGaze();

    /**
     * @brief 图片预处理        （ai2d for image）
     * @param ori_img          原始图片
     * @param sparse_points    原始人脸检测框对应的五官点
     * @return None
     */
    void pre_process(cv::Mat ori_img, Bbox &bbox);

    /**
     * @brief 视频流预处理（ai2d for video）
     * @param sparse_points    原始人脸检测框对应的五官点
     * @return None
     */
    void pre_process(Bbox &bbox);

    /**
     * @brief kmodel推理
     * @return None
     */
    void inference();

    /**
     * @brief kmodel后处理
     * @param result            注视估计识别结果
     * @return None
     */
    void post_process(EyeGazeInfo& result);

    /**
     * @brief 将处理好的轮廓画到原图
     * @param src_img     原图
     * @param result      注视估计识别结果
     * @param pic_mode    ture(原图片)，false(osd)
     * @return None
     */
    void draw_result(cv::Mat& src_img,Bbox& bbox,EyeGazeInfo& result, bool pic_mode=true);

private:
    /** 
     * @brief softmax
     * @param input   原始数据
     * @param output  softmax之后的数据
     * @return None     
     */
    void softmax(vector<float>& input,vector<float>& output);

    std::unique_ptr<ai2d_builder> ai2d_builder_; // ai2d构建器
    runtime_tensor ai2d_in_tensor_;              // ai2d输入tensor
    runtime_tensor ai2d_out_tensor_;             // ai2d输出tensor
    
    uintptr_t vaddr_;                            // isp的虚拟地址
    FrameCHWSize isp_shape_;                     // isp对应的地址大小
    float matrix_dst_[10];                       // 人脸affine的变换矩阵
};
#endif