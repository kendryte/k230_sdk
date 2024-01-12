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

#ifndef _TRACKER_H_
#define _TRACKER_H_

#include <iostream>
#include <vector>
#include "utils.h"
#include "ai_base.h"
#include "cv2_utils.h"

typedef struct Tracker_box
{
    int x;
    int y;
    int w;
    int h; 
    float score;
} Tracker_box;

/**
 * @brief 追踪比较
 * 主要封装了对于crop 和 src model输出，从预处理、运行到后处理给出结果的过程
 */
class Tracker : public AIBase
{
    public:

        /** 
        * @brief Tracker 构造函数，加载kmodel,并初始化kmodel输入、输出
        * @param kmodel_file kmodel文件路径
        * @param debug_mode 0（不调试）、 1（只显示时间）、2（显示所有打印信息）
        * @return None
        */
        Tracker(const char *kmodel_file, float thresh, const int debug_mode);

        /** 
        * @brief  Tracker 析构函数
        * @return None
        */
        ~Tracker();

        /**
         * @brief Tracker 输入预处理
         * @return None
         */
        void pre_process(std::vector<float*> tracker_inputs);

        /**
         * @brief kmodel推理
         * @return None
         */
        void inference();

        /** 
        * @brief postprocess 函数
        * @return None
        */
        void post_process(int cols,int rows,std::vector<Tracker_box>& track_boxes);

        /** 
        * @brief draw_track 函数
        * @return None
        */
        void draw_track(std::vector<Tracker_box> track_boxes,FrameSize sensor_size, FrameSize osd_size, cv::Mat& osd_frame);

    private:

        std::unique_ptr<ai2d_builder> ai2d_builder_; // ai2d构建器
        uintptr_t vaddr_;                            // isp的虚拟地址
        FrameCHWSize isp_shape_;                     // isp对应的地址大小

        int tracker_input_shapes[2];
        float thresh;
        float *output_0;
        float *output_1;
};
#endif
