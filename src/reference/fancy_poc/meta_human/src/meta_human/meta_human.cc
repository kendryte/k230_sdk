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
#include "meta_human.h"


void writeBin0(std::string path, char *buf, int size)
{
    std::ofstream outfile(path, std::ifstream::binary);
    outfile.write((char *)(buf), size);
    outfile.close();
}

void writeBinInt(std::string path, int num)
{
    std::ofstream file(path, std::ios::binary);
    string str = std::to_string(num);
    int len = str.length();
    file.write((char*)&len, sizeof(len));
    file.write(str.c_str(), len);
    file.close();
}

std::string readBinInt(std::string path)
{
    ifstream file2(path, std::ios::binary);
    int len2 = 0;
    file2.read((char*)&len2, sizeof(len2));
    char* buffer = new char[len2 + 1];
    file2.read(buffer, len2);
    buffer[len2] = '\0';
    string str2 = buffer;
    delete[] buffer;
    file2.close();

    return str2;
}

// for image
Meta_Human::Meta_Human(const char *kmodel_file,const int debug_mode):AIBase(kmodel_file,"Meta_Human", debug_mode)
{
    model_name_ = "Meta_Human";
    ai2d_out_tensor_ = get_input_tensor(0);
}

// for video
Meta_Human::Meta_Human(const char *kmodel_file,  FrameCHWSize isp_shape, uintptr_t vaddr, uintptr_t paddr, const int debug_mode):AIBase(kmodel_file,"Meta_Human", debug_mode)
{
    model_name_ = "Meta_Human";
    
    vaddr_ = vaddr;

    isp_shape_ = isp_shape;
    dims_t in_shape{1, isp_shape_.channel, isp_shape_.height, isp_shape_.width};
    int isp_size = isp_shape_.channel * isp_shape_.height * isp_shape_.width;
    #if 0
    ai2d_in_tensor_ = host_runtime_tensor::create(typecode_t::dt_uint8, in_shape, { (gsl::byte *)vaddr, isp_size },
        true, hrt::pool_shared).expect("cannot create input tensor");
    #else
    ai2d_in_tensor_ = hrt::create(typecode_t::dt_uint8, in_shape, hrt::pool_shared).expect("create ai2d input tensor failed");
    #endif

    ai2d_out_tensor_ = get_input_tensor(0);
    Utils::padding_resize(isp_shape_, {input_shapes_[0][3], input_shapes_[0][2]}, ai2d_builder_, ai2d_in_tensor_, ai2d_out_tensor_, cv::Scalar(104, 117, 123));
}

Meta_Human::~Meta_Human()
{
}

// ai2d for image
void Meta_Human::pre_process(cv::Mat ori_img)
{
    ScopedTiming st(model_name_ + " pre_process image", debug_mode_);
    std::vector<uint8_t> chw_vec;
    Utils::hwc_to_chw(ori_img, chw_vec);
    Utils::padding_resize({ori_img.channels(), ori_img.rows, ori_img.cols}, chw_vec, {input_shapes_[0][3], input_shapes_[0][2]}, ai2d_out_tensor_, cv::Scalar(104, 117, 123));
}

// ai2d for video
void Meta_Human::pre_process()
{
    ScopedTiming st(model_name_ + " pre_process video", debug_mode_);
    #if 0
    ai2d_builder_->invoke().expect("error occurred in ai2d running");
    #else
    size_t isp_size = isp_shape_.channel * isp_shape_.height * isp_shape_.width;
    auto buf = ai2d_in_tensor_.impl()->to_host().unwrap()->buffer().as_host().unwrap().map(map_access_::map_write).unwrap().buffer();
    memcpy(reinterpret_cast<char *>(buf.data()), (void *)vaddr_, isp_size);
    hrt::sync(ai2d_in_tensor_, sync_op_t::sync_write_back, true).expect("sync write_back failed");
    ai2d_builder_->invoke(ai2d_in_tensor_,ai2d_out_tensor_).expect("error occurred in ai2d running");
    #endif
}

void Meta_Human::inference()
{
    this->run();
    this->get_output();
}

void Meta_Human::post_process(FrameSize frame_size,int idx,int num_storage)
{
    ScopedTiming st(model_name_ + " post process ", debug_mode_);
    int net_len = input_shapes_[0][2];

    {
        float *output_0 = p_outputs_[0];
        float *output_1 = p_outputs_[1];

        if(idx < num_storage)
        {
            writeBin0(  "ping/" + std::to_string(idx) +  "_0.bin",(char* )output_0, 1  * net_len/8 * net_len/8  * 1 * 4);
            writeBin0(  "ping/" + std::to_string(idx) +  "_1.bin",(char *)output_1, 1  * net_len/8 * net_len/8 * 145 * 4);
        }
        else 
        {
            int idx_ = idx % num_storage;
            writeBin0(  "pong/" + std::to_string(idx_) +  "_0.bin",(char* )output_0, 1  * net_len/8 * net_len/8  * 1 * 4);
            writeBin0(  "pong/" + std::to_string(idx_) +  "_1.bin",(char *)output_1, 1  * net_len/8 * net_len/8 * 145 * 4);
        }
    }
}


