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
#include "face_alignment_post.h"
#include "k230_math.h"
#include "constant.h"
#include "rasterize.h"

// for image
FaceAlignmentPost::FaceAlignmentPost(const char *kmodel_file, const int debug_mode) : AIBase(kmodel_file,"FaceAlignmentPost", debug_mode)
{
    model_name_ = "FaceAlignmentPost";
}

// ai2d for video
void FaceAlignmentPost::pre_process(vector<float>& param)
{
    ScopedTiming st(model_name_ + " pre_process", debug_mode_);
    int offset = 0;
    for(int i =0;i<input_shapes_.size();++i)
    {
        auto in_tensor = get_input_tensor(i);
        auto buf = in_tensor.impl()->to_host().unwrap()->buffer().as_host().unwrap().map(map_access_::map_write).unwrap().buffer();
        float *buf_data = reinterpret_cast<float *>(buf.data());
        if(i==0)
        {
            int in_shape_row = input_shapes_[0][0],in_shape_col = input_shapes_[0][1];
            for(int j = 0;j<in_shape_row;++j)
            {
                memcpy(buf_data+j*in_shape_col, param.data()+j*4, in_shape_col*sizeof(float));
                offset += in_shape_col;
            }
        }
        else if(i==1)
        {
            int in_shape_row = input_shapes_[1][0],in_shape_col = input_shapes_[1][1];
            for(int j = 0;j<in_shape_row;++j)
            {
                memcpy(buf_data+j*in_shape_col,param.data()+j*4 + input_shapes_[0][1], in_shape_col*sizeof(float));
                offset += in_shape_col;
            }
        }
        else
        {    
            int in_size = input_shapes_[i][0] * input_shapes_[i][1];
            memcpy(buf_data, param.data()+offset, in_size*sizeof(float));
            offset += in_size;
        }
        
        hrt::sync(in_tensor, sync_op_t::sync_write_back, true).expect("sync write_back failed");
    }    
}

void FaceAlignmentPost::inference()
{
    this->run();
    this->get_output();
}

void FaceAlignmentPost::post_process(vector<float>& vertices)
{
    ScopedTiming st(model_name_ + " post_process", debug_mode_);
    int out_size = (each_output_size_by_byte_[1]-each_output_size_by_byte_[0])/sizeof(float);
    vertices.insert(vertices.end(),p_outputs_[0],p_outputs_[0]+out_size);
}

FaceAlignmentPost::~FaceAlignmentPost()
{
}