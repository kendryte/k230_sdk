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

#ifndef FASTSPEECH1_H_
#define FASTSPEECH1_H_

#include "ai_base.h"


/**
 * @brief 中文tts encoder部分
 */
class Fastspeech1 : public AIBase
{
    public:
        /**
        * @brief Fastspeech1构造函数，加载kmodel,并初始化kmodel输入、输出
        * @param kmodel_file kmodel文件路径
        * @param debug_mode  0（不调试）、 1（只显示时间）、2（显示所有打印信息）
        * @return None
        */
        Fastspeech1(const char *kmodel_file,  const int debug_mode = 1);
        /**
        * @brief Fastspeech1析构函数
        * @return None
        */
        ~Fastspeech1();

        /**
        * @brief 数据预处理
        * @param speakers 语音发音人数据
        * @param sequence Text对应音素序列数据
        * @return None
        */
        void pre_process(vector<int> speakers,vector<int> sequence);

        /**
        * @brief kmodel推理
        * @return None
        */
        void inference();

        /**
        * @brief kmodel推理结果后处理
        * @param results_zh encoder和添加音高pitch和能量energy的编码
        * @param results_dim 不同音素发音时长的扩展维度[1,3,2,4...]表示第一个音素向量重复扩展一次，第二个音素向量重复扩展3次，以此类推
        * @return None
        */
        void post_process(vector<float> &results_zh,vector<float> &results_dim);

    private:
        float *output; 
        runtime_tensor kmodel_input1 ;    //模型输入，speakers
        runtime_tensor kmodel_input2 ;    //模型输入，sequence
};
#endif