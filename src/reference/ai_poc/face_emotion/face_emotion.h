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
#ifndef _FACE_EMOTION_H
#define _FACE_EMOTION_H

#include <vector>
#include "utils.h"
#include "ai_base.h"

using std::vector;

typedef struct FaceEmotionInfo
{
    int idx;                     //人脸情感识别结果对应类别id
    float score;                 //人脸情感识别结果对应类别得分
    string label;                //人脸情感识别结果对应类别
} FaceEmotionInfo;

/**
 * @brief 人脸情感识别
 * 主要封装了对于每一帧图片，从预处理、运行到后处理给出结果的过程
 */
class FaceEmotion : public AIBase
{
public:
    /**
     * @brief FaceEmotion构造函数，加载kmodel,并初始化kmodel输入、输出(for image)
     * @param kmodel_file       kmodel文件路径
     * @param debug_mode        0（不调试）、 1（只显示时间）、2（显示所有打印信息）
     * @return None
     */
    FaceEmotion(const char *kmodel_file, const int debug_mode);

    /**
     * @brief FaceEmotion构造函数，加载kmodel,并初始化kmodel输入、输出和人脸检测阈值(for isp)
     * @param kmodel_file       kmodel文件路径
     * @param isp_shape         isp输入大小（chw）
     * @param vaddr             isp对应虚拟地址
     * @param paddr             isp对应物理地址
     * @param debug_mode        0（不调试）、 1（只显示时间）、2（显示所有打印信息）
     * @return None
     */
    FaceEmotion(const char *kmodel_file, FrameCHWSize isp_shape, uintptr_t vaddr, uintptr_t paddr, const int debug_mode);

    /**
     * @brief FaceEmotion析构函数
     * @return None
     */
    ~FaceEmotion();

    /**
     * @brief 图片预处理        （ai2d for image）
     * @param ori_img          原始图片
     * @param sparse_points    原始人脸检测框对应的五官点
     * @return None
     */
    void pre_process(cv::Mat ori_img, float* sparse_points);

    /**
     * @brief 视频流预处理（ai2d for video）
     * @param sparse_points    原始人脸检测框对应的五官点
     * @return None
     */
    void pre_process(float* sparse_points);

    /**
     * @brief kmodel推理
     * @return None
     */
    void inference();

    /**
     * @brief kmodel后处理
     * @param result            后处理结果，包括类别和类别得分
     * @return None
     */
    void post_process(FaceEmotionInfo& result);

    /**
     * @brief 将处理好的轮廓画到原图
     * @param src_img     原图
     * @param bbox        识别人脸的检测框位置
     * @param result      人脸情感识别结果
     * @param pic_mode    ture(原图片)，false(osd)
     * @return None
     */
    void draw_result(cv::Mat& src_img,Bbox& bbox,FaceEmotionInfo& result, bool pic_mode=true);

private:
    /** 
     * @brief svd
     * @param a     原始矩阵
     * @param u     左奇异向量
     * @param s     对角阵
     * @param v     右奇异向量
     * @return None
     */
    void svd22(const float a[4], float u[4], float s[2], float v[4]);
    
    /**
    * @brief 使用Umeyama算法计算仿射变换矩阵
    * @param src  原图像点位置
    * @param dst  目标图像（224*224）点位置。
    */
    void image_umeyama_224(float* src, float* dst);

    /**
    * @brief 获取affine变换矩阵
    * @param sparse_points  原图像人脸五官点位置
    */
    void get_affine_matrix(float* sparse_points);

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
    vector<string> label_list_;                   // 情感分类标签列表
};
#endif