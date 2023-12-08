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
#include "dtranslate.h"


DTranslate::DTranslate(const char *kmodel_file, const char *tag_model_file, const int debug_mode)
:AIBase(kmodel_file, "DTranslate", debug_mode)
{
    const auto tag_status = sp_tag.Load(tag_model_file);
    if (!tag_status.ok()) {
        std::cerr << tag_status.ToString() << std::endl;
    }
    pad_id = sp_tag.pad_id();
    bos_id = sp_tag.bos_id();
    eos_id = sp_tag.eos_id();

    in_tensor_0 = get_input_tensor(0);
    in_tensor_1 = get_input_tensor(1);
    in_tensor_2 = get_input_tensor(2);
    in_tensor_3 = get_input_tensor(3);
}

DTranslate::~DTranslate()
{
    delete[] dst_mask;
    delete[] src_dst_mask;
}

void DTranslate::pre_process(int index, vector<int64_t> &input_y, float *encoder_kv, int src_token_size)
{
    ScopedTiming st("Decoder preprocess", debug_mode_);
    if (index == 0)
    {
        maxlen = input_shapes_[0][1];
        d_model = input_shapes_[1][2];
        vocab_size = output_shapes_[0][2];
        input_y = vector<int64_t>(maxlen,pad_id);
        input_y[0] = bos_id;
        dst_mask = new uint8_t[maxlen*maxlen];
        src_dst_mask = new uint8_t[maxlen*maxlen];

        for (int i = 0; i < maxlen; i++)
        {
            for (int j = 0; j < maxlen; j++)
            {
                if (i < 1 and j < 1)
                {
                    dst_mask[i*maxlen + j] = 0;
                }
                else
                {
                    dst_mask[i*maxlen + j] = 1;
                }
            }
        }

        for (int i = 0; i < maxlen; i++)
        {
            for (int j = 0; j < maxlen; j++)
            {
                if (i < 1 and j < src_token_size)
                {
                    src_dst_mask[i*maxlen + j] = 0;
                }
                else
                {
                    src_dst_mask[i*maxlen + j] = 1;
                }
            }
        }
    }
    else
    {
        for (int i = 0; i < index + 1; i++)
        {
            dst_mask[i*maxlen + index] = 0;
        }
        for (int j = 0; j < index + 1; j++)
        {
            dst_mask[index*maxlen + j] = 0;
        }

        for (int j = 0; j < src_token_size; j++)
        {
            src_dst_mask[index*maxlen + j] = 0;
        }
    }
    
    size_t in_tensor_0_size = maxlen;
    size_t in_tensor_1_size = maxlen*d_model;
    size_t in_tensor_2_size = maxlen*maxlen;
    size_t in_tensor_3_size = maxlen*maxlen;
    auto buf_0 = in_tensor_0.impl()->to_host().unwrap()->buffer().as_host().unwrap().map(map_access_::map_write).unwrap().buffer();
    auto buf_1 = in_tensor_1.impl()->to_host().unwrap()->buffer().as_host().unwrap().map(map_access_::map_write).unwrap().buffer();
    auto buf_2 = in_tensor_2.impl()->to_host().unwrap()->buffer().as_host().unwrap().map(map_access_::map_write).unwrap().buffer();
    auto buf_3 = in_tensor_3.impl()->to_host().unwrap()->buffer().as_host().unwrap().map(map_access_::map_write).unwrap().buffer();
    memcpy(reinterpret_cast<char *>(buf_0.data()), input_y.data(), in_tensor_0_size*8);
    memcpy(reinterpret_cast<char *>(buf_1.data()), encoder_kv, in_tensor_1_size*4);
    memcpy(reinterpret_cast<char *>(buf_2.data()), dst_mask, in_tensor_2_size);
    memcpy(reinterpret_cast<char *>(buf_3.data()), src_dst_mask, in_tensor_3_size);
    hrt::sync(in_tensor_0, sync_op_t::sync_write_back, true).expect("sync write_back failed");
    hrt::sync(in_tensor_1, sync_op_t::sync_write_back, true).expect("sync write_back failed");
    hrt::sync(in_tensor_2, sync_op_t::sync_write_back, true).expect("sync write_back failed");
    hrt::sync(in_tensor_3, sync_op_t::sync_write_back, true).expect("sync write_back failed");
}


void DTranslate::inference()
{
    this->run();
    this->get_output();
}

void DTranslate::post_process(int next_index, vector<int64_t> &result,vector<int> &int_res, bool *stop)
{
    ScopedTiming st("Decoder postprocess", debug_mode_);
    output = p_outputs_[0];
    int max_index = 0;
    float max_value = FLT_MIN;

    for(int i = 0; i < vocab_size; i++)
    {
        if(output[(next_index-1)*vocab_size + i] > max_value)
        {
            max_value = output[(next_index-1)*vocab_size + i];
            max_index = i;
        }
    }

    result[next_index] = max_index;
    int_res.push_back(max_index);
    if (max_index == eos_id)
    {
        *stop = true;
    }
}

string DTranslate::decode_post(vector<int> int_res)
{
    ScopedTiming st("Decoder decode", debug_mode_);
    string result;
    sp_tag.Decode(int_res, &result);
    return result;
}