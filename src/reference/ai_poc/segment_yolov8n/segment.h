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

#ifndef _SEG_H
#define _SEG_H

#include "utils.h"
#include "ai_base.h"


/**
 * @brief 多目标分割
 * 主要封装了对于每一帧图片，从预处理、运行到后处理给出结果的过程
 */
class Seg : public AIBase
{
    public:
        /**
        * @brief Seg构造函数，加载kmodel,并初始化kmodel输入、输出和多目标分割阈值
        * @param kmodel_file kmodel文件路径
        * @param conf_thres  分割框的置信度阈值
        * @param nms_thres   分割非极大值抑制阈值
        * @param mask_thres  分割掩码阈值
        * @param debug_mode  0（不调试）、 1（只显示时间）、2（显示所有打印信息）
        * @return None
        */
        Seg(const char *kmodel_file, float conf_thres, float nms_thres, float mask_thres, const int debug_mode = 1);
        
        /**
        * @brief Seg构造函数，加载kmodel,并初始化kmodel输入、输出和多目标分割阈值
        * @param kmodel_file kmodel文件路径
        * @param conf_thres  分割框的置信度阈值
        * @param nms_thres   分割非极大值抑制阈值
        * @param mask_thres  分割掩码阈值
        * @param isp_shape   isp输入大小（chw）
        * @param vaddr       isp对应虚拟地址
        * @param paddr       isp对应物理地址
        * @param debug_mode  0（不调试）、 1（只显示时间）、2（显示所有打印信息）
        * @return None
        */    
        Seg(const char *kmodel_file, float conf_thres, float nms_thres, float mask_thres, FrameCHWSize isp_shape, uintptr_t vaddr, uintptr_t paddr,const int debug_mode);
        
        /**
        * @brief Seg析构函数
        * @return None
        */
        ~Seg();

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
        * @param results 后处理之后的基于原始图像的分割结果集合
        * @return None
        */
        void post_process(FrameSize frame_size, vector<OutputSeg> &results);

    private:

        /**
        * @brief nms 非极大值抑制
        * @param boxes  模型初始预测的检测框
        * @param confidences 模型初始预测检测框对应的置信度
        * @param confThreshold 置信度阈值
        * @param nmsThreshold 非极大值抑制阈值
        * @param indices 非极大值抑制后的检测框索引
        * @return None
        */
        void nms_boxes(vector<Rect> &boxes, vector<float> &confidences, float confThreshold, float nmsThreshold, vector<int> &indices);
        
        /**
        * @brief 计算 iou 
        * @param rect1  检测框1
        * @param rect2  检测框2
        * @return float iou值
        */
        float get_iou_value(Rect rect1, Rect rect2);
        
        std::unique_ptr<ai2d_builder> ai2d_builder_; // ai2d构建器
        runtime_tensor ai2d_in_tensor_;              // ai2d输入tensor
        runtime_tensor ai2d_out_tensor_;             // ai2d输出tensor
        uintptr_t vaddr_;                            // isp的虚拟地址
        FrameCHWSize isp_shape_;                     // isp对应的地址大小

        //类别名称
        std::vector<std::string> classes{"person", "bicycle", "car", "motorcycle", "airplane", "bus", "train", "truck", "boat", "traffic light", "fire hydrant", "stop sign", "parking meter", "bench", "bird", "cat", "dog", "horse", "sheep", "cow", "elephant", "bear", "zebra", "giraffe", "backpack", "umbrella", "handbag", "tie", "suitcase", "frisbee", "skis", "snowboard", "sports ball", "kite", "baseball bat", "baseball glove", "skateboard", "surfboard", "tennis racket", "bottle", "wine glass", "cup", "fork", "knife", "spoon", "bowl", "banana", "apple", "sandwich", "orange", "broccoli", "carrot", "hot dog", "pizza", "donut", "cake", "chair", "couch", "potted plant", "bed", "dining table", "toilet", "tv", "laptop", "mouse", "remote", "keyboard", "cell phone", "microwave", "oven", "toaster", "sink", "refrigerator", "book", "clock", "vase", "scissors", "teddy bear", "hair drier", "toothbrush"};

        float conf_thres; //置信度阈值
        float nms_thres; //非极大值抑制阈值
        float mask_thres; //掩码阈值
        int classes_count; //类别数
        int segChannels; //分割通道数
        int segWidth; //分割宽
        int segHeight; //分割高
        int Num_box;  //检测的框数

        float *output_0; //kmodel输出0
        float *output_1; //kmodel输出1

};
#endif