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
#ifndef _DTRANSLATE_H
#define _DTRANSLATE_H

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
#include <vector>
#include <float.h>

#include <nncase/runtime/interpreter.h>
#include <nncase/runtime/runtime_tensor.h>

#include "sentencepiece_processor.h"
#include "ai_base.h"

using namespace nncase;
using namespace nncase::runtime;
using namespace nncase::runtime::detail;
using namespace std;


/**
 * @brief 翻译
 * 主要封装了解码器对被翻译的句子，从预处理、运行到后处理给出结果的过程
 */
class DTranslate:public AIBase
{
public:
    /**
     * @brief DTranslate构造函数，加载kmodel,并初始化kmodel输入、输出
     * @param kmodel_file kmodel文件路径
     * @param tag_model_file 分词器模型路径
     * @param debug_mode  0（不调试）、 1（只显示时间）、2（显示所有打印信息）
     * @return None
     */
    DTranslate(const char *kmodel_file, const char *tag_model_file, const int debug_mode);

    /**
     * @brief DTranslate析构函数
     * @return None
     */
    ~DTranslate();

    /**
     * @brief 解码器预处理
     * @param index 翻译指定位置
     * @param input_y 解码器输入
     * @param encoder_kv 编码器输出
     * @param src_pad_mask 被翻译句子token掩码
     * @return None
     */
    void pre_process(int index, vector<int64_t> &input_y, float *encoder_kv, int src_token_size);

    /**
     * @brief kmodel推理
     * @return None
     */
    void inference();

    /**
     * @brief 解码器kmodel推理结果后处理
     * @param next_index 翻译后的填写位置
     * @param result 解码器输入
     * @param int_res 翻译结果
     * @param stop 是否停止翻译
     * @return None
     */
    void post_process(int next_index, vector<int64_t> &result, vector<int> &int_res, bool *stop);

    /**
     * @brief 解码器解码
     * @param int_res 翻译结果
     * @return string 翻译后的句子
     */
    string decode_post(vector<int> int_res);

private:

    runtime_tensor in_tensor_0;              // 第1个输入tensor 
    runtime_tensor in_tensor_1;             // 第2个输出tensor
    runtime_tensor in_tensor_2;              // 第3个输入tensor 
    runtime_tensor in_tensor_3;             // 第4个输出tensor

    int maxlen;                             //模型最大输入token长度
    int d_model;                            //模型单个token编码长度
    sentencepiece::SentencePieceProcessor sp_tag; //翻译后语言分词器
    uint8_t *dst_mask;                      //翻译后语言句子掩码
    uint8_t *src_dst_mask;                  //翻译前和翻译后句子结合掩码
    int pad_id;                             //翻译后语言的 token pad 值
    int bos_id;                             //翻译后语言的起始符编号
    int eos_id;                             //翻译后语言的终止符编号
    int vocab_size;                         //翻译后语言字典长度
    float *output;                          //编码器模型输出
};
#endif
