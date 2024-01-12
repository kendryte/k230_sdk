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

#include "fastspeech1.h"

Fastspeech1::Fastspeech1(const char *kmodel_file, const int debug_mode)
:AIBase(kmodel_file,"fastspeech1", debug_mode)
{
    model_name_ = "fastspeech1";
    kmodel_input1 = this -> get_input_tensor(0);
    kmodel_input2 = this -> get_input_tensor(1);
}


Fastspeech1::~Fastspeech1()
{
    delete[] output;
}

void Fastspeech1::pre_process(vector<int> speakers,vector<int> sequence)
{
    size_t isp_size = sequence.size() * sizeof(int);
    auto buf = kmodel_input2.impl()->to_host().unwrap()->buffer().as_host().unwrap().map(map_access_::map_write).unwrap().buffer();
    memcpy(reinterpret_cast<char *>(buf.data()), (void *)sequence.data(), isp_size);
    size_t isp_size1 = 1 * sizeof(int);
    auto buf1 = kmodel_input1.impl()->to_host().unwrap()->buffer().as_host().unwrap().map(map_access_::map_write).unwrap().buffer();
    memcpy(reinterpret_cast<char *>(buf1.data()), (void *)speakers.data(), isp_size1);
}

void Fastspeech1::inference()
{
    this->run();
    this->get_output();
}

void Fastspeech1::post_process(vector<float> &results_zh,vector<float> &results_dim)
{   
    output = p_outputs_[0];
    for (int j = 0; j < output_shapes_[0][0] * output_shapes_[0][1] * output_shapes_[0][2]; j++) {
        results_zh.push_back(output[j]);
    }
    output = p_outputs_[1];

    for (int j = 0; j < output_shapes_[1][0] * output_shapes_[1][1] ; j++) {
        results_dim.push_back(output[j]);
    }
}


