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

#include "ocr_reco.h"

OCRReco::OCRReco(const char *kmodel_file, int dict_size, const int debug_mode)
:dict_size(dict_size), AIBase(kmodel_file,"OCRReco", debug_mode)
{
    model_name_ = "OCRReco";
    flag = 0;

    input_width = input_shapes_[0][3];
    input_height = input_shapes_[0][2];

    output = new float[input_width * dict_size / 4];

    ai2d_out_tensor_ = this -> get_input_tensor(0);
}

OCRReco::OCRReco(const char *kmodel_file, int dict_size, FrameCHWSize isp_shape, uintptr_t vaddr, uintptr_t paddr, const int debug_mode)
:dict_size(dict_size), AIBase(kmodel_file,"OCRReco", debug_mode)
{
    model_name_ = "OCRReco";
    flag = 0;

    input_width = input_shapes_[0][3];
    input_height = input_shapes_[0][2];

    output = new float[input_width * dict_size / 4];

    vaddr_ = vaddr;

    isp_shape_ = isp_shape;
    dims_t in_shape{1, isp_shape.channel, isp_shape.height, isp_shape.width};
    int isp_size = isp_shape.channel * isp_shape.height * isp_shape.width;

    ai2d_in_tensor_ = hrt::create(typecode_t::dt_uint8, in_shape, hrt::pool_shared).expect("create ai2d input tensor failed");

    ai2d_out_tensor_ = this -> get_input_tensor(0);

    Utils::resize(ai2d_builder_, ai2d_in_tensor_, ai2d_out_tensor_);
}

OCRReco::~OCRReco()
{
    delete[] output;
}

void OCRReco::pre_process(cv::Mat ori_img)
{
    ScopedTiming st(model_name_ + " pre_process image", debug_mode_);
    std::vector<uint8_t> chw_vec;

    Utils::bgr2rgb_and_hwc2chw(ori_img, chw_vec);
    Utils::padding_resize_one_side({ori_img.channels(), ori_img.rows, ori_img.cols}, chw_vec, {input_shapes_[0][3], input_shapes_[0][2]}, ai2d_out_tensor_, cv::Scalar(0, 0, 0));
}

void OCRReco::pre_process()
{
    ScopedTiming st(model_name_ + " pre_process video", debug_mode_);
    size_t isp_size = isp_shape_.channel * isp_shape_.height * isp_shape_.width;
    auto buf = ai2d_in_tensor_.impl()->to_host().unwrap()->buffer().as_host().unwrap().map(map_access_::map_write).unwrap().buffer();
    memcpy(reinterpret_cast<char *>(buf.data()), (void *)vaddr_, isp_size);
    hrt::sync(ai2d_in_tensor_, sync_op_t::sync_write_back, true).expect("sync write_back failed");
    ai2d_builder_->invoke(ai2d_in_tensor_, ai2d_out_tensor_).expect("error occurred in ai2d running");
}

void OCRReco::inference()
{
    this->run();
    this->get_output();
}

void OCRReco::post_process(vector<string> &results)
{
    output = p_outputs_[0];

	int size = input_width / 4;

    // std::cout << size << std::endl;

	ifstream dict(DICT);
	vector<string> txt;
	txt.push_back("blank");
	while (!dict.eof())
	{
		string line;
		while (getline(dict, line))
		{
			txt.push_back(line);
		}
	}
	txt.push_back("");


	vector<int> result;
	for (int i = 0; i < size; i++)
	{
		int max_index = std::max_element(output + i * dict_size, output + i * dict_size + dict_size) - (output + i * dict_size);
        // int max_index = std::max_element(pred,pred+output_shapes_[0][3]) - pred;
		result.push_back(max_index);
	}

	for (int i = 0; i < size; i++)
	{
		if (result[i] == 0)
			continue;
		if (i > 0 && result[i - 1] == result[i])
			continue;
		results.push_back(txt[result[i]]);
	}
}

