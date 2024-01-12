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

#include "fastspeech2.h"

Fastspeech2::Fastspeech2(const char *kmodel_file, const int debug_mode)
:AIBase(kmodel_file,"fastspeech2", debug_mode)
{
    model_name_ = "fastspeech2";
    kmodel_input1 = this -> get_input_tensor(0);
}


Fastspeech2::~Fastspeech2()
{
    delete[] output;
}

void Fastspeech2::pre_process(vector<float> vector_zh)
{
    size_t isp_size = vector_zh.size() * sizeof(float);
    auto buf = kmodel_input1.impl()->to_host().unwrap()->buffer().as_host().unwrap().map(map_access_::map_write).unwrap().buffer();
    memcpy(reinterpret_cast<char *>(buf.data()), (void *)vector_zh.data(), isp_size);
}

void Fastspeech2::inference()
{
    this->run();
    this->get_output();
}

void Fastspeech2::post_process(vector<float> &results,int m_pad)
{   
    output = p_outputs_[0];
    //输出为(1,80,600),取出padding部分
    for (int i = 0; i < 80; i++){
        for (int j = 0; j < (600-m_pad); j++) {
                results.push_back(output[i*600+j]);
        }
    }
    
}
    


