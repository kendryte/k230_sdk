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

#include "hifigan.h"

HiFiGan::HiFiGan(const char *kmodel_file, const int debug_mode)
:AIBase(kmodel_file,"hifigan", debug_mode)
{
    model_name_ = "hifigan";
    kmodel_input = this -> get_input_tensor(0);
}


HiFiGan::~HiFiGan()
{
    delete[] output;
}

void HiFiGan::pre_process(vector<float> tensor)
{
    size_t isp_size = tensor.size();
    auto buf = kmodel_input.impl()->to_host().unwrap()->buffer().as_host().unwrap().map(map_access_::map_write).unwrap().buffer();
    memcpy(reinterpret_cast<char *>(buf.data()), (void *)tensor.data(), isp_size*4);
    hrt::sync(kmodel_input, sync_op_t::sync_write_back, true).expect("sync write_back failed");
}

void HiFiGan::inference()
{
    this->run();
    this->get_output();
}

void HiFiGan::post_process(vector<float> &results)
{   
    output = p_outputs_[0];
    for (int j = 0; j < output_shapes_[0][0] * output_shapes_[0][1] * output_shapes_[0][2]; j++) {
        results.push_back(output[j]);
    }
}


