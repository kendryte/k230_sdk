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
#ifndef _FACE_DENSE_LANDMARK_H
#define _FACE_DENSE_LANDMARK_H

#include <vector>
#include "utils.h"
#include "ai_base.h"

using std::vector;

/**
 * @brief 人脸密集关键点检测
 * 主要封装了对于每一帧图片，从预处理、运行到后处理给出结果的过程
 */
class FaceDenseLandmark : public AIBase
{
public:
    /**
     * @brief FaceDenseLandmark构造函数，加载kmodel,并初始化kmodel输入、输出(for image)
     * @param kmodel_file kmodel文件路径
     * @param debug_mode  0（不调试）、 1（只显示时间）、2（显示所有打印信息）
     * @return None
     */
    FaceDenseLandmark(const char *kmodel_file, const int debug_mode = 1);

    /**
     * @brief FaceDenseLandmark构造函数，加载kmodel,并初始化kmodel输入、输出和人脸检测阈值(for isp)
     * @param kmodel_file kmodel文件路径
     * @param isp_shape   isp输入大小（chw）
     * @param vaddr       isp对应虚拟地址
     * @param paddr       isp对应物理地址
     * @param debug_mode  0（不调试）、 1（只显示时间）、2（显示所有打印信息）
     * @return None
     */
    FaceDenseLandmark(const char *kmodel_file, FrameCHWSize isp_shape, uintptr_t vaddr, uintptr_t paddr, const int debug_mode);

    /**
     * @brief FaceDenseLandmark析构函数
     * @return None
     */
    ~FaceDenseLandmark();

    /**
     * @brief 图片预处理（ai2d for image）
     * @param ori_img 原始图片
     * @param bbox    原始人脸检测框位置
     * @return None
     */
    void pre_process(cv::Mat ori_img, Bbox &bbox);

    /**
     * @brief 视频流预处理（ai2d for video）
     * @param ori 原始视频流帧数据（或许定义为uint8_t*更加合适）
     * @return None
     */
    void pre_process(Bbox &bbox);

    /**
     * @brief kmodel推理
     * @return None
     */
    void inference();

    /**
     * @brief kmodel推理结果后处理
     * @return None
     */
    void post_process();

    /**
     * @brief 将处理好的轮廓画到原图
     * @param src_img 原图
     * @param pic_mode    ture(原图片)，false(osd)
     * @return None
     */
    void draw_contour(cv::Mat src_img, bool pic_mode = true);

private:
    // img prepocess
    /**
     * @brief 获取旋转矩阵
     * @param bbox 根据检测框获取affine变换矩阵
     * @return None
     */
    void get_affine_matrix(Bbox &bbox);

    std::unique_ptr<ai2d_builder> ai2d_builder_; // ai2d构建器
    runtime_tensor ai2d_in_tensor_;              // ai2d输入tensor
    runtime_tensor ai2d_out_tensor_;             // ai2d输出tensor
    
    uintptr_t vaddr_;                            // isp的虚拟地址
    FrameCHWSize isp_shape_;                     // isp对应的地址大小
    cv::Mat matrix_dst_;                         // affine的变换矩阵
};
#endif