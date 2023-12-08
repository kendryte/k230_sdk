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
#include "etranslate.h"


ETranslate::ETranslate(const char *kmodel_file, const char *src_model_file, const int debug_mode)
:AIBase(kmodel_file, "ETranslate", debug_mode)
{
    const auto src_status = sp_src.Load(src_model_file);
    if (!src_status.ok()) {
        std::cerr << src_status.ToString() << std::endl;
    }

    bos_id = sp_src.bos_id();
    eos_id = sp_src.eos_id();
    pad_id = sp_src.pad_id();

    in_tensor_0 = get_input_tensor(0);
    in_tensor_1 = get_input_tensor(1);
}

ETranslate::~ETranslate()
{
}

void ETranslate::pre_process(string ori_sen)
{
    ScopedTiming st("Encoder preprocess", debug_mode_);
    maxlen = input_shapes_[0][1];
    std::vector<int> ids;
    sp_src.Encode(ori_sen, &ids);

    source_seq = new int64_t[maxlen];
    source_seq[0] = bos_id;
    for(int i = 1; i < ids.size()+1; i++)
    {
        source_seq[i] = ids[i-1];
    }
    source_seq[ids.size()+1] = eos_id;
    src_token_size = ids.size()+2;
    
    if (maxlen < ids.size()+2) {
        std::cerr << " 输入的句子词数超出上限 " << std::endl;
    }

    for(int i = 0; i < maxlen-ids.size()-2; i++)
    {
        source_seq[ids.size()+2 + i] = pad_id;
    }

    src_mask = new uint8_t[maxlen*maxlen];
    for (int i = 0; i < maxlen; i++)
    {
        for (int j = 0; j < maxlen; j++)
        {
            if (i < ids.size()+2 and j < ids.size()+2)
            {
                src_mask[i*maxlen + j] = 0;
            }
            else
            {
                src_mask[i*maxlen + j] = 1;
            }
        }
    }

    size_t in_tensor_0_size = maxlen;
    size_t in_tensor_1_size = maxlen*maxlen;
    auto buf_0 = in_tensor_0.impl()->to_host().unwrap()->buffer().as_host().unwrap().map(map_access_::map_write).unwrap().buffer();
    auto buf_1 = in_tensor_1.impl()->to_host().unwrap()->buffer().as_host().unwrap().map(map_access_::map_write).unwrap().buffer();
    memcpy(reinterpret_cast<char *>(buf_0.data()), source_seq, in_tensor_0_size*8);
    memcpy(reinterpret_cast<char *>(buf_1.data()), src_mask, in_tensor_1_size);
    hrt::sync(in_tensor_0, sync_op_t::sync_write_back, true).expect("sync write_back failed");
    hrt::sync(in_tensor_1, sync_op_t::sync_write_back, true).expect("sync write_back failed");

    delete[] src_mask;
    delete[] source_seq;
}


void ETranslate::inference()
{
    this->run();
    this->get_output();
}

void ETranslate::post_process(vector<EncoderStruct> &result)
{
    ScopedTiming st("Encoder postprocess", debug_mode_);
    EncoderStruct eresult;
    eresult.encoder_kv = p_outputs_[0];
    eresult.src_token_size = src_token_size;
    result.push_back(eresult);
}

int ETranslate::get_maxlen()
{
    return maxlen;
}