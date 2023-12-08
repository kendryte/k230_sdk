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
#ifndef _ETRANSLATE_H
#define _ETRANSLATE_H

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

#include <nncase/runtime/interpreter.h>
#include <nncase/runtime/runtime_tensor.h>

#include "sentencepiece_processor.h"
#include "ai_base.h"

using namespace nncase;
using namespace nncase::runtime;
using namespace nncase::runtime::detail;
using namespace std;

/**
 * @brief Encoder 输出
 */
typedef struct EncoderStruct
{
    float *encoder_kv; // 经过编码器的编码结果
    int src_token_size; //被翻译句子token长度
} EncoderStruct;


/**
 * @brief 翻译
 * 主要封装了编码器对被翻译的句子，从预处理、运行到后处理给出结果的过程
 */
class ETranslate:public AIBase
{
public:
    /**
     * @brief ETranslate构造函数，加载kmodel,并初始化kmodel输入、输出
     * @param kmodel_file kmodel文件路径
     * @param src_model_file 分词器模型路径
     * @param debug_mode  0（不调试）、 1（只显示时间）、2（显示所有打印信息）
     * @return None
     */
    ETranslate(const char *kmodel_file, const char *src_model_file, const int debug_mode);

    /**
     * @brief ETranslate析构函数
     * @return None
     */
    ~ETranslate();

    /**
     * @brief 被翻译句子预处理
     * @param ori_sen 被翻译句子
     * @return None
     */
    void pre_process(string ori_sen);

    /**
     * @brief kmodel推理
     * @return None
     */
    void inference();

    /**
     * @brief kmodel推理结果后处理
     * @param result 编码器输出结果
     * @return None
     */
    void post_process(vector<EncoderStruct> &result);

    /**
     * @brief 获取最大token长度
     * @return int 返回最大token长度
     */
    int get_maxlen();

private:

    runtime_tensor in_tensor_0;              // 第1个输入tensor 
    runtime_tensor in_tensor_1;             // 第2个输出tensor

    int maxlen;                             //模型最大输入token长度
    int src_token_size;                     //被翻译句子的token长度
    sentencepiece::SentencePieceProcessor sp_src;  //被翻译语言的分词器
    uint8_t *src_mask;                      //被翻译句子的掩码
    int64_t *source_seq;                    //被翻译句子token列表
    int bos_id;                             //被翻译语言的起始符编号
    int eos_id;                             //被翻译语言的终止符编号
    int pad_id;                             //被翻译语言的token pad值
};
#endif
