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

#include "pphumanseg.h"

// for image
SEG::SEG(const char *kmodel_file, const int debug_mode): AIBase(kmodel_file,"pphumanseg", debug_mode)
{

    model_name_ = "pphumanseg";
    
    ai2d_out_tensor_ = get_input_tensor(0);

}


// for video
SEG::SEG(const char *kmodel_file, FrameCHWSize isp_shape, uintptr_t vaddr, uintptr_t paddr, const int debug_mode):AIBase(kmodel_file,"pphumanseg", debug_mode)
{
    model_name_ = "pphumanseg";
    
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

    // ai2d_out_tensor
    ai2d_out_tensor_ = get_input_tensor(0);
    // fixed padding resize param
    Utils::resize(ai2d_builder_, ai2d_in_tensor_, ai2d_out_tensor_);
}


SEG::~SEG()
{

}

// ai2d for image
void SEG::pre_process(cv::Mat ori_img)
{
    ScopedTiming st(model_name_ + " pre_process image", debug_mode_);
    std::vector<uint8_t> chw_vec;
    Utils::hwc_to_chw(ori_img, chw_vec);
    // Utils::padding_resize({ori_img.channels(), ori_img.rows, ori_img.cols}, chw_vec, {input_shapes_[0][3], input_shapes_[0][2]}, ai2d_out_tensor_, cv::Scalar(114, 114, 114));
    Utils::resize({ori_img.channels(), ori_img.rows, ori_img.cols}, chw_vec, ai2d_out_tensor_);
}

// ai2d for video
void SEG::pre_process()
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
    // run ai2d
    #endif
}

void SEG::inference()
{
    ScopedTiming st(model_name_ + " inference", debug_mode_);
    this->run();
    this->get_output();
}

cv::Mat SEG::post_process( )
{

    ScopedTiming st(model_name_ + " post_process", debug_mode_);
    float *output = p_outputs_[0];
    int net_len_w = input_shapes_[0][2];
    int net_len_h = input_shapes_[0][3];
    cv::Mat mask = cv::Mat::ones(net_len_w, net_len_h, CV_8UC1) * 255;

    // {
    //     // for NCHW
	// 	for (int i = 0; i < net_len_h; i++)
	// 	{
	// 		for (int j = 0; j < net_len_w; j++)
	// 		{
	// 			mask.at<uchar>(i, j) = (output[j + net_len_w * i] > output[j + net_len_w * i + net_len_w * net_len_h] ? 255 : 0);
	// 		}
	// 	}
    // }

	{
        // for NHWC
		for (int i = 0; i < net_len_h; i++)
		{
			for (int j = 0; j < net_len_w; j++)
			{
				int idx = j + i * net_len_w;
                mask.at<uchar>(i, j) = (output[2 * (j + net_len_w * i)] > output[2 * (j + net_len_w * i) + 1] ? 255 : 0);
			}
		}
	}

	cv::imwrite("mask.jpg", mask);
	return mask;
}