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
#include "e3d.h"

E3d::E3d(const char *kmodel_file, const int debug_mode)
:AIBase(kmodel_file, "E3d", debug_mode)
{
    model_name_ = "E3d";
    in_tensor_ = get_input_tensor(0);

	gaussian_data_ = new float[41*41];
    data = new float[input_shapes_[0][0]*input_shapes_[0][1]*input_shapes_[0][2]*input_shapes_[0][3]];

    std::ifstream inF("gaussian.bin",std::ios::binary);
	inF.read(reinterpret_cast<char*>(gaussian_data_),sizeof(float)*(41*41));
	inF.close();

}

E3d::~E3d()
{
    delete[] gaussian_data_;
    delete[] data;
}

void E3d::pre_process(cv::Mat ori_img, std::vector<int> results)
{
    ScopedTiming st(model_name_ + " pre_process image", debug_mode_);

    float ratio,dw,dh;
    cv::Mat image = Utils::letterbox(ori_img, {input_shapes_[0][2], input_shapes_[0][3]}, cv::Scalar(0,0,0),ratio,dw,dh);

    std::vector<cv::Mat> hmChannels = get_heatmap(results, ratio, dw, dh);

    cv::Mat img_fix_size_r;

    image.convertTo(img_fix_size_r, CV_32FC3);  
    img_fix_size_r = (img_fix_size_r - 128.0) / 256.0;

    std::vector<cv::Mat> bgrChannels(3);
    cv::split(img_fix_size_r, bgrChannels);

    for (auto i = 0; i < bgrChannels.size(); i++)
    {
        memcpy(data + i * bgrChannels[i].rows * bgrChannels[i].cols, bgrChannels[i].data, bgrChannels[i].rows * bgrChannels[i].cols*4);
    }

    for (auto i = 0; i < hmChannels.size(); i++)
    {
        memcpy(data + (i + bgrChannels.size()) * hmChannels[i].rows * hmChannels[i].cols, hmChannels[i].data, hmChannels[i].rows * hmChannels[i].cols*4); 
    }

    size_t isp_size = isp_shape_.channel * isp_shape_.height * isp_shape_.width;
    auto buf = in_tensor_.impl()->to_host().unwrap()->buffer().as_host().unwrap().map(map_access_::map_write).unwrap().buffer();
    memcpy(reinterpret_cast<char *>(buf.data()), reinterpret_cast<char*>(data), sizeof(float)*(input_shapes_[0][0]*input_shapes_[0][1]*input_shapes_[0][2]*input_shapes_[0][3]));
	hrt::sync(in_tensor_, sync_op_t::sync_write_back, true).expect("sync write_back failed");
}

void E3d::inference()
{
    this->run();
    this->get_output();
}

void E3d::post_process(int idx, int buffer_size)
{
    ScopedTiming st(model_name_ + " post process ", debug_mode_);

    float *output_0 = p_outputs_[0];

    if(idx < buffer_size)
    {
        Utils::writeBin("ping/" +  std::to_string(idx) +  "_hand.bin",(char* )output_0,  sizeof(float)*(output_shapes_[0][0]*output_shapes_[0][1]));
    }
    else 
    {
        int idx_ = idx % buffer_size;
        Utils::writeBin("pong/" +  std::to_string(idx_) +  "_hand.bin",(char* )output_0,  sizeof(float)*(output_shapes_[0][0]*output_shapes_[0][1]));
    }

    // Utils::writeBin( std::to_string(idx) +  "_hand.bin",(char* )output_0,  sizeof(float)*(output_shapes_[0][0]*output_shapes_[0][1]));
}

void E3d::draw_umich_gaussian(cv::Mat & heatmap, const int &center_x, const int &center_y, int radius, float &ratio, float &dw, float &dh) 
{
    int x = std::round((float)center_x * ratio + dw - 0.1);
    int y = std::round((float)center_y * ratio + dh - 0.1);
    int height = input_shapes_[0][2];
    int width = input_shapes_[0][3];
    int left = std::min(x, radius);
    int right = std::min(width - x, radius + 1);
    int top = std::min(y, radius);
    int bottom = std::min(height - y, radius + 1);

    for (int i = radius - top, k = 0; i < radius + bottom; ++i, ++k) 
    {
        for (int j = radius - left, l = 0; j < radius + right; ++j, ++l) 
        {
            heatmap.at<float>(y - top + k, x - left + l) = gaussian_data_[i * 41 + j];
        }
    }
}

std::vector<cv::Mat> E3d::get_heatmap(const std::vector<int>& handpose_2d, float &ratio, float &dw, float &dh) 
{
    int num_objs = 21;
    int radius = 20;
    std::vector<cv::Mat> hmChannels;
    
    for (int k = 0; k < num_objs; ++k) 
    {
        cv::Mat hm(input_shapes_[0][2], input_shapes_[0][3], CV_32FC1, cv::Scalar(0));
        draw_umich_gaussian(hm, handpose_2d[k*2+0], handpose_2d[k*2+1], radius, ratio, dw, dh);
        hmChannels.push_back(hm);
    }
    return hmChannels;
}