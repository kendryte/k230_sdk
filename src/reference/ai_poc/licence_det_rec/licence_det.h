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

#ifndef _LICENCE_DET_H
#define _LICENCE_DET_H

#include "utils.h"
#include "ai_base.h"

extern float anchors320[4200][4];
extern float anchors640[16800][4];
#define LOC_SIZE  4
#define CONF_SIZE 2
#define LAND_SIZE 8

/**
 * @brief 车牌检测
 * 主要封装了对于每一帧图片，从预处理、运行到后处理给出结果的过程
 */
class LicenceDetect : public AIBase
{
    public:
    /**
        * @brief LicenceDetect构造函数，加载kmodel,并初始化kmodel输入、输出和车牌检测阈值
        * @param kmodel_file kmodel文件路径
        * @param obj_thresh  车牌检测object阈值
        * @param nms_thresh  车牌检测nms阈值
        * @param debug_mode  0（不调试）、 1（只显示时间）、2（显示所有打印信息）
        * @return None
        */
    LicenceDetect(const char *kmodel_file, float obj_thresh, float nms_thresh, const int debug_mode = 1);

    /**
    * @brief LicenceDetect构造函数，加载kmodel,并初始化kmodel输入、输出和车牌检测阈值
    * @param kmodel_file kmodel文件路径
    * @param obj_thresh  车牌检测object阈值
    * @param nms_thresh  车牌检测nms阈值
    * @param isp_shape   isp输入大小（chw）
    * @param vaddr       isp对应虚拟地址
    * @param paddr       isp对应物理地址
    * @param debug_mode  0（不调试）、 1（只显示时间）、2（显示所有打印信息）
    * @return None
    */
    LicenceDetect(const char *kmodel_file, float obj_thresh, float nms_thresh, FrameCHWSize isp_shape, uintptr_t vaddr, uintptr_t paddr,const int debug_mode);

    /**
    * @brief LicenceDetect析构函数
    * @return None
    */
    ~LicenceDetect();

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
    * @param frame_size 原始图像/帧宽高，用于将结果放到原始图像大小
    * @param results 后处理之后的基于原始图像的检测结果集合
    * @return None
    */
    void post_process(FrameSize frame_size, vector<BoxPoint> &results);

    private:

    /**
    * @brief 计算softmax值
    * @param x 
    * @param dx 
    * @param len 
    * @return None
    */
    void local_softmax(float* x, float* dx, uint32_t len);
    
    /**
    * @brief 计算argmax
    * @param x 
    * @param len 
    * @return 返回最大值的索引
    */
    int argmax(float* x, uint32_t len);

    /**
    * @brief 计算confidence
    * @param conf 
    * @param s_probs 
    * @param s 
    * @param size 
    * @param obj_cnt 
    * @return None
    */
    void deal_conf(float* conf, float* s_probs, sortable_obj_t* s, int size, int& obj_cnt);

    /**
    * @brief 计算loc
    * @param loc 
    * @param boxes 
    * @param size 
    * @param obj_cnt 
    * @return None
    */
    void deal_loc(float* loc, float* boxes, int size, int& obj_cnt);

    /**
    * @brief 计算landmark
    * @param landms 
    * @param landmarks 
    * @param size 
    * @param obj_cnt 
    * @return None
    */
    void deal_landms(float* landms, float* landmarks, int size, int& obj_cnt);

    /**
    * @brief 获得初始检测框
    * @param boxes 
    * @param obj_index 
    * @return 检测框集合
    */
    Bbox get_box(float* boxes, int obj_index);

    /**
    * @brief 获得landmark
    * @param landmarks 
    * @param obj_index 
    * @return landmark集合
    */
    landmarks_t get_landmark(float* landmarks, int obj_index);

    /**
    * @brief 计算重合区域
    * @param x1 第一个框的左上角坐标值
    * @param w1 第一个框的w
    * @param x2 第二个框的左上角坐标值
    * @param w2 第二个框的w
    * @return 重叠量
    */
    float overlap(float x1, float w1, float x2, float w2);

    /**
    * @brief 计算两个框的交集
    * @param x1 第一个框
    * @param w1 第二个框
    * @return 交集值
    */
    float box_intersection(Bbox a, Bbox b);

    /**
    * @brief 计算两个框的并集
    * @param x1 第一个框
    * @param w1 第二个框
    * @return 并集值
    */
    float box_union(Bbox a, Bbox b);

    /**
    * @brief 计算两个框的iou
    * @param x1 第一个框
    * @param w1 第二个框
    * @return iou值
    */
    float box_iou(Bbox a, Bbox b);


    std::unique_ptr<ai2d_builder> ai2d_builder_; // ai2d构建器
    runtime_tensor ai2d_in_tensor_;              // ai2d输入tensor
    runtime_tensor ai2d_out_tensor_;             // ai2d输出tensor
    uintptr_t vaddr_;                            // isp的虚拟地址
    FrameCHWSize isp_shape_;                     // isp对应的地址大小

    float obj_thresh;  //车牌检测object阈值
    float nms_thresh;  //车牌检测nms阈值

    float (*anchors)[4];  //车牌检测anchors
    int min_size;         //车牌检测 数量
};
#endif