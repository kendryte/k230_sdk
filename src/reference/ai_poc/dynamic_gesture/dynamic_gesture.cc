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
#include "dynamic_gesture.h"
#include "utils.h"

DynamicGesture::DynamicGesture(const char *kmodel_file, const int debug_mode) : AIBase(kmodel_file,"DynamicGesture", debug_mode)
{
    model_name_ = "DynamicGesture";
   
    for (int i=0;i<input_shapes_.size();i++)
    {
        runtime_tensor in_tensor_= get_input_tensor(i);
        in_tensors_.push_back(in_tensor_);
    }

    for (int i=0;i<input_shapes_.size();i++)
    {
        float* data_ = new float[input_shapes_[i][0]*input_shapes_[i][1]*input_shapes_[i][2]*input_shapes_[i][3]];
        memset(data_, 0, sizeof(float) * input_shapes_[i][0]*input_shapes_[i][1]*input_shapes_[i][2]*input_shapes_[i][3]);
        input_bins.push_back(data_);
    }
}

DynamicGesture::~DynamicGesture()
{
    for (int i=0;i<input_shapes_.size();i++)
    {
        delete[] input_bins[i];
    }
}

void DynamicGesture::pre_process(cv::Mat ori_img)
{
    ScopedTiming st(model_name_ + " pre_process_video", debug_mode_);

    // shape上的变化，RGB
    cv::Mat image = Utils::resize_centercrop(ori_img, 256, input_shapes_[0][2]);

    // 归一化，标准化，CHW
    std::vector<cv::Mat> image_channels(3);
    cv::split(image, image_channels); //分割三通道图像

    std::vector<float> mean_value{0.485, 0.456, 0.406};
    std::vector<float> std_value{0.229, 0.224, 0.225};
    for (auto i = 0; i < image_channels.size(); i++)
    {
        for(int hh = 0; hh < input_shapes_[0][2]; hh++)
        {
            for(int ww = 0; ww < input_shapes_[0][3]; ww++)
            {
                input_bins[0][i * input_shapes_[0][2] * input_shapes_[0][3] + hh * input_shapes_[0][3] + ww] = (image_channels[i].data[hh * input_shapes_[0][3] + ww] / 255.0 - mean_value[i]) / std_value[i];
            }
        }
    }

    for (int i = 0; i<input_shapes_.size(); i++)
    {
        auto buf = in_tensors_[i].impl()->to_host().unwrap()->buffer().as_host().unwrap().map(map_access_::map_write).unwrap().buffer();
        memcpy(reinterpret_cast<char *>(buf.data()), reinterpret_cast<char*>(input_bins[i]), sizeof(float)*(input_shapes_[i][0]*input_shapes_[i][1]*input_shapes_[i][2]*input_shapes_[i][3]));
        hrt::sync(in_tensors_[i], sync_op_t::sync_write_back, true).expect("sync write_back failed");
    }
}

void DynamicGesture::inference()
{
    this->run();
    this->get_output();
}

void DynamicGesture::post_process()
{
    ScopedTiming st(model_name_ + " post_process", debug_mode_);
    float *output = p_outputs_[0];

    for (int i=0;i<10;i++)
    {
        memcpy(reinterpret_cast<char*>(input_bins[i + 1]), reinterpret_cast<char*>(p_outputs_[i+1]), sizeof(float)*(input_shapes_[i + 1][0]*input_shapes_[i+1][1]*input_shapes_[i+1][2]*input_shapes_[i+1][3]));
    }
}

void DynamicGesture::get_out(vector<float> &output)
{
    softmax(p_outputs_[0], p_outputs_[0], output_shapes_[0][1] );
    output.resize(output_shapes_[0][1]);

    for (int i=0;i<output_shapes_[0][1];i++)
    {
        output[i] = p_outputs_[0][i];
    }
}

void DynamicGesture::softmax(float* x, float* dx, uint32_t len)
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

int DynamicGesture::process_output(int pred, std::vector<int>& history) {
    if (pred == 7 || pred == 8 || pred == 21 || pred == 22 || pred == 3 ) {
        pred = history.back();
    }
    if (pred == 0 || pred == 4 || pred == 6 || pred == 9|| pred == 14 || pred == 1|| pred == 19|| pred == 20|| pred == 23|| pred == 24) 
    {
        pred = history.back();
    }

    if (pred == 0) {
        pred = 2;
    }
    if (pred != history.back()) {
        if (history.size() >= 2) {
            if (!(history.back() == history[history.size() - 2])) {
                pred = history.back();
            }
        }
    }
    history.push_back(pred);
    if (history.size() > max_hist_len) {
        history.erase(history.begin(), history.begin() + (history.size() - max_hist_len));
    }
    
    return history.back();
}
