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

#ifndef _POSE_DETECT
#define _POSE_DETECT

#include <iostream>
#include <vector>
#include "utils.h"
#include "ai_base.h"


/**
 * @brief 人体关键点检测任务
 * 主要封装了对于每一帧图片，从预处理、运行到后处理给出结果的过程
 */
class poseDetect: public AIBase
{
    public:

        /** 
        * for image
        * @brief poseDetect 构造函数，加载kmodel,并初始化kmodel输入、输出、类阈值和NMS阈值
        * @param kmodel_file kmodel文件路径
        * @param obj_thresh 检测框阈值
        * @param nms_thresh NMS阈值
        * @param debug_mode 0（不调试）、 1（只显示时间）、2（显示所有打印信息）
        * @return None
        */
        poseDetect(const char *kmodel_file, float obj_thresh,float nms_thresh,  const int debug_mode);
        /** 
        * for video
        * @brief poseDetect 构造函数，加载kmodel,并初始化kmodel输入、输出、类阈值和NMS阈值
        * @param kmodel_file kmodel文件路径
        * @param obj_thresh 检测框阈值
        * @param nms_thresh NMS阈值
        * @param isp_shape   isp输入大小（chw）
        * @param vaddr       isp对应虚拟地址
        * @param paddr       isp对应物理地址
        * @param debug_mode 0（不调试）、 1（只显示时间）、2（显示所有打印信息）
        * @return None
        */
        poseDetect(const char *kmodel_file, float obj_thresh,float nms_thresh, FrameCHWSize isp_shape, uintptr_t vaddr, uintptr_t paddr, const int debug_mode);
        /** 
        * @brief  poseDetect 析构函数
        * @return None
        */
        ~poseDetect();

        /**
         * @brief 图片预处理（ai2d for image）
         * @param ori_img 原始图片
         * @return None
         */
        void pre_process(cv::Mat ori_img);

        /**
         * @brief 视频流预处理（ai2d for video）
         * @return None
         */
        void pre_process();

        /**
         * @brief kmodel推理
         * @return None
         */
        void inference();


        /** 
        * @brief  Detect函数
        * @param  all_data      模型输出
        * @param  output        检测输出结果
        * @param  params        长宽伸缩比以及x、y方向padding长度
        * @return true/false    是否检测得到
        */
        bool Detect(float* all_data, std::vector<OutputPose> &output,cv::Vec4d params);

        /** 
        * @brief  BatchDetect函数
        * @param  all_data      模型输出
        * @param  output        检测输出结果
        * @param  params        长宽伸缩比以及x、y方向padding长度
        * @return true/false    是否检测得到
        */
        bool BatchDetect(float* all_data, std::vector<std::vector<OutputPose>>& output,cv::Vec4d params);

        /** 
        * @brief postprocess 函数，对输出解码后的结果，进行NMS处理
        * @param  output        检测输出结果
        * @param  params        长宽伸缩比以及x、y方向padding长度
        * @return None
        */
        bool post_process( std::vector<OutputPose> &output,cv::Vec4d params);

        std::vector<std::string> labels { "person" }; // 类别标签

        cv::Vec4d params;  // 长宽伸缩比以及x、y方向padding长度

    private:
        float obj_thresh_;  // 检测框阈值
        float nms_thresh_;  // NMS阈值
        
        int _anchorLength;// pose一个框的信息56个数
    
        std::vector<int64_t> _outputTensorShape; // 输出Tensor Shape

        float *foutput_0;   // 输出

        std::unique_ptr<ai2d_builder> ai2d_builder_; // ai2d构建器
        runtime_tensor ai2d_in_tensor_;              // ai2d输入tensor
        runtime_tensor ai2d_out_tensor_;             // ai2d输出tensor
        uintptr_t vaddr_;                            // isp的虚拟地址
        FrameCHWSize isp_shape_;                     // isp对应的地址大小

};
#endif
