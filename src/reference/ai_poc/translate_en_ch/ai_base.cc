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
#include "ai_base.h"

#include <iostream>
#include <cassert>
#include <fstream>
#include <string>

#include <nncase/runtime/debug.h>

using std::cout;
using std::endl;
using namespace nncase;
using namespace nncase::runtime::detail;

AIBase::AIBase(const char *kmodel_file,const string model_name, const int debug_mode) : debug_mode_(debug_mode),model_name_(model_name)
{
    if (debug_mode > 1)
        cout << "kmodel_file:" << kmodel_file << endl;
    std::ifstream ifs(kmodel_file, std::ios::binary);
    kmodel_interp_.load_model(ifs).expect("Invalid kmodel");
    set_input_init();
    set_output_init();
}

AIBase::~AIBase()
{
}

void AIBase::set_input_init()
{
    ScopedTiming st(model_name_ + " set_input init", debug_mode_);
    int input_total_size = 0;
    each_input_size_by_byte_.push_back(0); // 先补0,为之后做准备
    for (int i = 0; i < kmodel_interp_.inputs_size(); ++i)
    {
        auto desc = kmodel_interp_.input_desc(i);
        auto shape = kmodel_interp_.input_shape(i);
        auto tensor = host_runtime_tensor::create(desc.datatype, shape, hrt::pool_shared).expect("cannot create input tensor");
        kmodel_interp_.input_tensor(i, tensor).expect("cannot set input tensor");
        vector<int> in_shape;
        if (debug_mode_ > 1)
            cout<<"input "<< std::to_string(i) <<" : "<<to_string(desc.datatype)<<",";
        int dsize = 1;
        for (int j = 0; j < shape.size(); ++j)
        {
            in_shape.push_back(shape[j]);
            dsize *= shape[j];
            if (debug_mode_ > 1)
                cout << shape[j] << ",";
        }
        if (debug_mode_ > 1)
            cout << endl;
        input_shapes_.push_back(in_shape);
        // DEFINE_TYPECODE(uint8,      u8,     0x06)
        // DEFINE_TYPECODE(float32,    f32,    0x0B)
        if (desc.datatype == dt_int8 || desc.datatype == dt_uint8)
        {
            input_total_size += dsize;
        }
        else if (desc.datatype == dt_int16 || desc.datatype == dt_uint16 || desc.datatype == dt_float16 || desc.datatype == dt_bfloat16)
        {
            input_total_size += (dsize * 2);
        }
        else if (desc.datatype == dt_int32 || desc.datatype == dt_uint32 || desc.datatype == dt_float32)
        {
            input_total_size += (dsize * 4);
        }
        else if(desc.datatype == dt_int64 || desc.datatype == dt_uint64 || desc.datatype == dt_float64)
        {
            input_total_size += (dsize * 8);
        }
        else
        {
            printf("input data type:%d",desc.datatype);
            assert(("unsupported kmodel output data type", 0));
        }
        each_input_size_by_byte_.push_back(input_total_size);

    }
    each_input_size_by_byte_.push_back(input_total_size); // 最后一个保存总大小
}

runtime_tensor AIBase::get_input_tensor(size_t idx)
{
    return kmodel_interp_.input_tensor(idx).expect("cannot get input tensor");
}

void AIBase::set_output_init()
{
    ScopedTiming st(model_name_ + " set_output_init", debug_mode_);
    each_output_size_by_byte_.clear();
    int output_total_size = 0;
    each_output_size_by_byte_.push_back(0);
    for (size_t i = 0; i < kmodel_interp_.outputs_size(); i++)
    {
        auto desc = kmodel_interp_.output_desc(i);
        auto shape = kmodel_interp_.output_shape(i);
        vector<int> out_shape;
        if (debug_mode_ > 1)
            cout<<"output "<<std::to_string(i)<<" : "<<to_string(desc.datatype)<<",";
        int dsize = 1;
        for (int j = 0; j < shape.size(); ++j)
        {
            out_shape.push_back(shape[j]);
            dsize *= shape[j];
            if (debug_mode_ > 1)
                cout << shape[j] << ",";
        }
        if (debug_mode_ > 1)
            cout << endl;
        output_shapes_.push_back(out_shape);
        if (desc.datatype == dt_int8 || desc.datatype == dt_uint8)
        {
            output_total_size += dsize;
        }
        else if (desc.datatype == dt_int16 || desc.datatype == dt_uint16 || desc.datatype == dt_float16 || desc.datatype == dt_bfloat16)
        {
            output_total_size += (dsize * 2);
        }
        else if (desc.datatype == dt_int32 || desc.datatype == dt_uint32 || desc.datatype == dt_float32)
        {
            output_total_size += (dsize * 4);
        }
        else if(desc.datatype == dt_int64 || desc.datatype == dt_uint64 || desc.datatype == dt_float64)
        {
            output_total_size += (dsize * 8);
        }
        else
        {
            printf("output data type:%d",desc.datatype);
            assert(("unsupported kmodel output data type", 0));
        }

        each_output_size_by_byte_.push_back(output_total_size);
        auto tensor = host_runtime_tensor::create(desc.datatype, shape, hrt::pool_shared).expect("cannot create output tensor");
        kmodel_interp_.output_tensor(i, tensor).expect("cannot set output tensor");
    }
}

void AIBase::run()
{
    ScopedTiming st(model_name_ + " run", debug_mode_);
    kmodel_interp_.run().expect("error occurred in running model");
}

void AIBase::get_output()
{
    ScopedTiming st(model_name_ + " get_output", debug_mode_);
    p_outputs_.clear();
    for (int i = 0; i < kmodel_interp_.outputs_size(); i++)
    {
        auto out = kmodel_interp_.output_tensor(i).expect("cannot get output tensor");
        // auto mapped_buf = std::move(hrt::map(out, map_access_::map_read).unwrap());
        // float *p_out = reinterpret_cast<float *>(mapped_buf.buffer().data());
        auto buf = out.impl()->to_host().unwrap()->buffer().as_host().unwrap().map(map_access_::map_read).unwrap().buffer();
        float *p_out = reinterpret_cast<float *>(buf.data());
        p_outputs_.push_back(p_out);
    }
}