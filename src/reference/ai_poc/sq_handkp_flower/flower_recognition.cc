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
#include "flower_recognition.h"
#include "utils.h"

FlowerRecognition::FlowerRecognition(const char *kmodel_file, const int debug_mode) : AIBase(kmodel_file,"FlowerRecognition",debug_mode)
{
    model_name_ = "FlowerRecognition";
    ai2d_out_tensor_ = get_input_tensor(0);
}

FlowerRecognition::FlowerRecognition(const char *kmodel_file, FrameCHWSize isp_shape, uintptr_t vaddr, uintptr_t paddr, const int debug_mode) : AIBase(kmodel_file,"FlowerRecognition", debug_mode)
{
    model_name_ = "FlowerRecognition";
    // input->isp（Fixed size）
    vaddr_ = vaddr;
    isp_shape_ = isp_shape;
    dims_t in_shape{1, isp_shape.channel, isp_shape.height, isp_shape.width};
#if 0
    int in_size = isp_shape.channel * isp_shape.height * isp_shape.width;
    ai2d_in_tensor_ = host_runtime_tensor::create(typecode_t::dt_uint8, in_shape, { (gsl::byte *)vaddr, in_size },
        true, hrt::pool_shared).expect("cannot create input tensor");
#else
    ai2d_in_tensor_ = hrt::create(typecode_t::dt_uint8, in_shape, hrt::pool_shared).expect("create ai2d input tensor failed");
#endif
    // ouput（Fixed size）
    dims_t out_shape{1, input_shapes_[0][1], input_shapes_[0][2], input_shapes_[0][3]};
    ai2d_out_tensor_ = get_input_tensor(0);
}

FlowerRecognition::~FlowerRecognition()
{
}

void FlowerRecognition::pre_process(cv::Mat ori_img, Bbox &bbox)
{
    ScopedTiming st(model_name_ + " pre_process image", debug_mode_);

    std::vector<uint8_t> chw_vec;
    Utils::bgr2rgb_and_hwc2chw(ori_img, chw_vec);
    Utils::crop_resize({ori_img.channels(), ori_img.rows, ori_img.cols}, chw_vec, bbox, ai2d_out_tensor_);
}

void FlowerRecognition::pre_process(Bbox &bbox)
{
    ScopedTiming st(model_name_ + " pre_process_video", debug_mode_);
#if 1
    size_t isp_size = isp_shape_.channel * isp_shape_.height * isp_shape_.width;
    auto buf = ai2d_in_tensor_.impl()->to_host().unwrap()->buffer().as_host().unwrap().map(map_access_::map_write).unwrap().buffer();
    memcpy(reinterpret_cast<char *>(buf.data()), (void *)vaddr_, isp_size);
    hrt::sync(ai2d_in_tensor_, sync_op_t::sync_write_back, true).expect("sync write_back failed");
#endif
    Utils::crop_resize(bbox, ai2d_builder_, ai2d_in_tensor_, ai2d_out_tensor_);
}

void FlowerRecognition::inference()
{
    this->run();
    this->get_output();
}

std::string FlowerRecognition::post_process()
{
    ScopedTiming st(model_name_ + " post_process", debug_mode_);
    float *output = p_outputs_[0];
    float pred[102] = {0};
    softmax(output, pred, output_shapes_[0][1] );
    auto it = std::max_element(pred, pred + output_shapes_[0][1]);
    size_t idx = it - pred;
    std::string text = labels[idx] + ":" + std::to_string(round(*it * 100) / 100.0);
    std::cout<<"predicted: "<<labels[idx]<<":"<<std::to_string(round(*it * 100) / 100.0)<<std::endl;

    return text;
}

void FlowerRecognition::softmax(float* x, float* dx, uint32_t len)
{
    float max_value = x[0];
    for (uint32_t i = 0; i < len; i++)
    {
        if (max_value < x[i])
        {
            max_value = x[i];
        }
    }
    for (uint32_t i = 0; i < len; i++)
    {
        x[i] -= max_value;
        x[i] = expf(x[i]);
    }
    float sum_value = 0.0f;
    for (uint32_t i = 0; i < len; i++)
    {
        sum_value += x[i];
    }
    for (uint32_t i = 0; i < len; i++)
    {
        dx[i] = x[i] / sum_value;
    }
}
