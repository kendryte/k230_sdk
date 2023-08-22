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

#include "licence_reco.h"

LicenceReco::LicenceReco(const char *kmodel_file, int dict_size, const int debug_mode)
:dict_size(dict_size), AIBase(kmodel_file,"LicenceReco", debug_mode)
{
    model_name_ = "LicenceReco";
    flag = 0;

    input_width = input_shapes_[0][3];
    input_height = input_shapes_[0][2];

    output = new float[input_width * dict_size / 4];

    ai2d_out_tensor_ = this -> get_input_tensor(0);
}

LicenceReco::LicenceReco(const char *kmodel_file, int dict_size, FrameCHWSize isp_shape, uintptr_t vaddr, uintptr_t paddr, const int debug_mode)
:dict_size(dict_size), AIBase(kmodel_file,"LicenceReco", debug_mode)
{
    model_name_ = "LicenceReco";
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

LicenceReco::~LicenceReco()
{
    // delete[] output;
}

void LicenceReco::pre_process(cv::Mat ori_img)
{
    ScopedTiming st(model_name_ + " pre_process image", debug_mode_);
    std::vector<uint8_t> chw_vec;
    Utils::hwc_to_chw(ori_img, chw_vec);
    Utils::resize({ori_img.channels(), ori_img.rows, ori_img.cols}, chw_vec, ai2d_out_tensor_);
}

void LicenceReco::pre_process()
{
    ScopedTiming st(model_name_ + " pre_process video", debug_mode_);
    size_t isp_size = isp_shape_.channel * isp_shape_.height * isp_shape_.width;
    auto buf = ai2d_in_tensor_.impl()->to_host().unwrap()->buffer().as_host().unwrap().map(map_access_::map_write).unwrap().buffer();
    memcpy(reinterpret_cast<char *>(buf.data()), (void *)vaddr_, isp_size);
    hrt::sync(ai2d_in_tensor_, sync_op_t::sync_write_back, true).expect("sync write_back failed");
    ai2d_builder_->invoke(ai2d_in_tensor_, ai2d_out_tensor_).expect("error occurred in ai2d running");
}

void LicenceReco::inference()
{
    // this->set_input_tensor(0, ai2d_out_tensor_);
    this->run();
    this->get_output();
}

void LicenceReco::post_process(vector<unsigned char> &results)
{
    output = p_outputs_[0];

	int size = input_width / 4;

	vector<int> result;
	for (int i = 0; i < size; i++)
	{
		float maxs = -10.f;
		int index = -1;
		for (int j = 0; j < dict_size; j++)
		{
			if (maxs < output[i * dict_size + j])
			{
				index = j;
				maxs = output[i * dict_size + j];
			}
		}
		result.push_back(index);
	}

    // 这里是车牌文字对应的十六进制
    std::vector<unsigned char> vec16dict = {0xb9,0xd2,0xca,0xb9,0xc1,0xec,0xb0,0xc4,0xb8,0xdb,0xcd,0xee,0xbb,0xa6,0xbd,0xf2,0xd3,0xe5,0xbc,0xbd,0xbd,0xfa,0xc3,0xc9,0xc1,0xc9,0xbc,0xaa,0xba,0xda,0xcb,0xd5,0xd5,0xe3,0xbe,0xa9,0xc3,0xf6,0xb8,0xd3,0xc2,0xb3,0xd4,0xa5,0xb6,0xf5,0xcf,0xe6,0xd4,0xc1,0xb9,0xf0,0xc7,0xed,0xb4,0xa8,0xb9,0xf3,0xd4,0xc6,0xb2,0xd8,0xc9,0xc2,0xb8,0xca,0xc7,0xe0,0xc4,0xfe,0xd0,0xc2,0xbe,0xaf,0xd1,0xa7,0x30,0x20,0x31,0x20,0x32,0x20,0x33,0x20,0x34,0x20,0x35,0x20,0x36,0x20,0x37,0x20,0x38,0x20,0x39,0x20,0x41,0x20,0x42,0x20,0x43,0x20,0x44,0x20,0x45,0x20,0x46,0x20,0x47,0x20,0x48,0x20,0x4a,0x20,0x4b,0x20,0x4c,0x20,0x4d,0x20,0x4e,0x20,0x50,0x20,0x51,0x20,0x52,0x20,0x53,0x20,0x54,0x20,0x55,0x20,0x56,0x20,0x57,0x20,0x58,0x20,0x59,0x20,0x5a,0x20,0x5f,0x20,0x2d,0x20};

	for (int i = 0; i < size; i++)
		if (result[i] >= 0 && result[i] != 0 && !(i > 0 && result[i-1] == result[i]))
        {
        	results.push_back(vec16dict[(result[i]-1)*2]);
            results.push_back(vec16dict[(result[i]-1)*2 + 1]);
        }
}