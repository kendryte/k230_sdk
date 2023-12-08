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
#ifndef _FACE_ALIGNMENT_POST_H
#define _FACE_ALIGNMENT_POST_H

#include <iostream>
#include <vector>

#include "utils.h"
#include "ai_base.h"

using std::vector;

#define LOC_SIZE 4
#define CONF_SIZE 2
#define LAND_SIZE 10
#define PI 3.1415926

/**
 * @brief 人脸对齐后处理
 * 主要封装了对于每一帧图片，从预处理、运行到后处理给出结果的过程
 */
class FaceAlignmentPost : public AIBase
{
public:
    /**
     * @brief FaceAlignmentPost构造函数
     * @param kmodel_file kmodel文件路径
     * @param debug_mode  0（不调试）、 1（只显示时间）、2（显示所有打印信息）
     * @return None
     */
    FaceAlignmentPost(const char *kmodel_file, const int debug_mode = 1);

    /**
     * @brief FaceAlignmentPost析构函数
     * @return None
     */
    ~FaceAlignmentPost();

    /**
     * @brief 预处理
     * @param param    人脸对齐3d关键点
     * @return None
     */
    void pre_process(vector<float>& param);

    /**
     * @brief kmodel推理
     * @return None
     */
    void inference();

    /**
     * @brief kmodel推理结果后处理
     * @param vertices 人脸3d点
     * @return None
     */
    void post_process(vector<float>& vertices);
private:
    friend class FaceAlignment;
};

#endif