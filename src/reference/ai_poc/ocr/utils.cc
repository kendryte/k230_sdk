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
// utils.cpp
#include <iostream>
#include "utils.h"

using std::ofstream;
using std::vector;

auto cache = cv::Mat::zeros(1, 1, CV_32FC1);

void Utils::dump_binary_file(const char *file_name, char *data, const size_t size)
{
    // eg:Utils::dump_binary_file(out_name.c_str(),reinterpret_cast<char *>(p_outputs_[i]),each_output_size_by_byte_[i+1]-each_output_size_by_byte_[i]);
    std::ofstream outf;
    outf.open(file_name, std::ofstream::binary);
    outf.write(data, size);
    outf.close();
}

void Utils::dump_gray_image(const char *file_name, const FrameSize &frame_size, unsigned char *data)
{
    cv::Mat gray_image = cv::Mat(frame_size.height, frame_size.width, CV_8UC1, data);
    cv::imwrite(file_name, gray_image);
}

void Utils::dump_color_image(const char *file_name, const FrameSize &frame_size, unsigned char *data)
{
    cv::Mat image_r = cv::Mat(frame_size.height, frame_size.width, CV_8UC1, data);
    cv::Mat image_g = cv::Mat(frame_size.height, frame_size.width, CV_8UC1, data+frame_size.height*frame_size.width);
    cv::Mat image_b = cv::Mat(frame_size.height, frame_size.width, CV_8UC1, data+2*frame_size.height*frame_size.width);
    
    std::vector<cv::Mat> color_vec(3);
    color_vec.clear();
    color_vec.push_back(image_b);
    color_vec.push_back(image_g);
    color_vec.push_back(image_r);

    cv::Mat color_img;
    cv::merge(color_vec, color_img);
    cv::imwrite(file_name, color_img);
}

cv::Mat Utils::padding_resize(const cv::Mat img, const FrameSize &frame_size, const cv::Scalar &padding)
{
    // width:dst_width
    int ori_w = img.cols;
    int ori_h = img.rows;
    float ratiow = (float)frame_size.width / ori_w;
    float ratioh = (float)frame_size.height / ori_h;
    float ratio = ratiow < ratioh ? ratiow : ratioh;
    int new_w = (int)(ratio * ori_w);
    int new_h = (int)(ratio * ori_h);
    float dw = (float)(frame_size.width - new_w) / 2;
    float dh = (float)(frame_size.height - new_h) / 2;
    int top = (int)(roundf(0 - 0.1));
    int bottom = (int)(roundf(dh * 2 + 0.1));
    int left = (int)(roundf(0 - 0.1));
    int right = (int)(roundf(dw * 2 - 0.1));
    cv::Mat cropped_img;
    {
        if ((new_w != frame_size.width) || (new_h != frame_size.height))
        {
            cv::resize(img, cropped_img, cv::Size(new_w, new_h), cv::INTER_AREA);
        }
    }
    {
        // 104, 117, 123,BGR
        cv::copyMakeBorder(cropped_img, cropped_img, top, bottom, left, right, cv::BORDER_CONSTANT, padding);
    }
    return cropped_img;
}

cv::Mat Utils::resize(const cv::Mat img, const FrameSize &frame_size)
{
    cv::Mat cropped_img;
    cv::resize(img, cropped_img, cv::Size(frame_size.width, frame_size.height), cv::INTER_LINEAR);
    return cropped_img;
}

cv::Mat Utils::bgr_to_rgb(cv::Mat ori_img)
{
    cv::Mat rgb_img;
    cv::cvtColor(ori_img, rgb_img, cv::COLOR_BGR2RGB);
    return rgb_img;
}

void Utils::hwc_to_chw(cv::Mat &ori_img, std::vector<uint8_t> &chw_vec)
{
    // for rgb format
    std::vector<cv::Mat> rgbChannels(3);
    cv::split(ori_img, rgbChannels);
    for (auto i = 0; i < rgbChannels.size(); i++)
    {
        std::vector<uint8_t> data = std::vector<uint8_t>(rgbChannels[i].reshape(1, 1));
        chw_vec.insert(chw_vec.end(), data.begin(), data.end());
    }
}

void Utils::bgr2rgb_and_hwc2chw(cv::Mat &ori_img, std::vector<uint8_t> &chw_vec)
{
    // for bgr format
    std::vector<cv::Mat> bgrChannels(3);
    cv::split(ori_img, bgrChannels);
    for (auto i = 2; i > -1; i--)
    {
        std::vector<uint8_t> data = std::vector<uint8_t>(bgrChannels[i].reshape(1, 1));
        chw_vec.insert(chw_vec.end(), data.begin(), data.end());
    }
}

void Utils::resize(FrameCHWSize ori_shape, std::vector<uint8_t> &chw_vec, runtime_tensor &ai2d_out_tensor)
{
    // build ai2d_in_tensor
    dims_t in_shape{1, ori_shape.channel, ori_shape.height, ori_shape.width};
    runtime_tensor ai2d_in_tensor = host_runtime_tensor::create(typecode_t::dt_uint8, in_shape, hrt::pool_shared).expect("cannot create input tensor");

    auto input_buf = ai2d_in_tensor.impl()->to_host().unwrap()->buffer().as_host().unwrap().map(map_access_::map_write).unwrap().buffer();
    memcpy(reinterpret_cast<char *>(input_buf.data()), chw_vec.data(), chw_vec.size());
    hrt::sync(ai2d_in_tensor, sync_op_t::sync_write_back, true).expect("write back input failed");

    // run ai2d
    // ai2d_datatype_t ai2d_dtype{ai2d_format::NCHW_FMT, ai2d_format::NCHW_FMT, typecode_t::dt_uint8, typecode_t::dt_uint8};
    ai2d_datatype_t ai2d_dtype{ai2d_format::NCHW_FMT, ai2d_format::NCHW_FMT, ai2d_in_tensor.datatype(), ai2d_out_tensor.datatype()};
    ai2d_crop_param_t crop_param { false, 30, 20, 400, 600 };
    ai2d_shift_param_t shift_param{false, 0};
    ai2d_pad_param_t pad_param{false, {{0, 0}, {0, 0}, {0, 0}, {0, 0}}, ai2d_pad_mode::constant, {114, 114, 114}};
    ai2d_resize_param_t resize_param{true, ai2d_interp_method::tf_bilinear, ai2d_interp_mode::half_pixel};
    ai2d_affine_param_t affine_param{false, ai2d_interp_method::cv2_bilinear, 0, 0, 127, 1, {0.5, 0.1, 0.0, 0.1, 0.5, 0.0}};

    dims_t out_shape = ai2d_out_tensor.shape();
    ai2d_builder builder { in_shape, out_shape, ai2d_dtype, crop_param, shift_param, pad_param, resize_param, affine_param };
    builder.build_schedule();
    builder.invoke(ai2d_in_tensor,ai2d_out_tensor).expect("error occurred in ai2d running");
}

void Utils::resize(std::unique_ptr<ai2d_builder> &builder, runtime_tensor &ai2d_in_tensor, runtime_tensor &ai2d_out_tensor)
{
    // run ai2d
    ai2d_datatype_t ai2d_dtype{ai2d_format::NCHW_FMT, ai2d_format::NCHW_FMT, ai2d_in_tensor.datatype(), ai2d_out_tensor.datatype()};
    ai2d_crop_param_t crop_param { false, 30, 20, 400, 600 };
    ai2d_shift_param_t shift_param{false, 0};
    ai2d_pad_param_t pad_param{false, {{0, 0}, {0, 0}, {0, 0}, {0, 0}}, ai2d_pad_mode::constant, {114, 114, 114}};
    ai2d_resize_param_t resize_param{true, ai2d_interp_method::tf_bilinear, ai2d_interp_mode::half_pixel};
    ai2d_affine_param_t affine_param{false, ai2d_interp_method::cv2_bilinear, 0, 0, 127, 1, {0.5, 0.1, 0.0, 0.1, 0.5, 0.0}};

    dims_t in_shape = ai2d_in_tensor.shape();
    dims_t out_shape = ai2d_out_tensor.shape();
    builder.reset(new ai2d_builder(in_shape, out_shape, ai2d_dtype, crop_param, shift_param, pad_param, resize_param, affine_param));
    builder->build_schedule();
    builder->invoke(ai2d_in_tensor,ai2d_out_tensor).expect("error occurred in ai2d running");
}

void Utils::crop_resize(FrameCHWSize ori_shape, std::vector<uint8_t> &chw_vec, Bbox &crop_info, runtime_tensor &ai2d_out_tensor)
{
    // build ai2d_in_tensor
    dims_t in_shape{1, ori_shape.channel, ori_shape.height, ori_shape.width};
    runtime_tensor ai2d_in_tensor = host_runtime_tensor::create(typecode_t::dt_uint8, in_shape, hrt::pool_shared).expect("cannot create input tensor");

    auto input_buf = ai2d_in_tensor.impl()->to_host().unwrap()->buffer().as_host().unwrap().map(map_access_::map_write).unwrap().buffer();
    memcpy(reinterpret_cast<char *>(input_buf.data()), chw_vec.data(), chw_vec.size());
    hrt::sync(ai2d_in_tensor, sync_op_t::sync_write_back, true).expect("write back input failed");

    // run ai2d
    ai2d_datatype_t ai2d_dtype{ai2d_format::NCHW_FMT, ai2d_format::NCHW_FMT, ai2d_in_tensor.datatype(), ai2d_out_tensor.datatype()};
    ai2d_crop_param_t crop_param{true, crop_info.x, crop_info.y, crop_info.w, crop_info.h};
    ai2d_shift_param_t shift_param{false, 0};
    ai2d_pad_param_t pad_param{false, {{0, 0}, {0, 0}, {0, 0}, {0, 0}}, ai2d_pad_mode::constant, {114, 114, 114}};
    ai2d_resize_param_t resize_param{true, ai2d_interp_method::tf_bilinear, ai2d_interp_mode::half_pixel};
    ai2d_affine_param_t affine_param{false, ai2d_interp_method::cv2_bilinear, 0, 0, 127, 1, {0.5, 0.1, 0.0, 0.1, 0.5, 0.0}};

    dims_t out_shape = ai2d_out_tensor.shape();
    ai2d_builder builder { in_shape, out_shape, ai2d_dtype, crop_param, shift_param, pad_param, resize_param, affine_param };
    builder.build_schedule();
    builder.invoke(ai2d_in_tensor,ai2d_out_tensor).expect("error occurred in ai2d running");
}

void Utils::crop_resize(Bbox &crop_info, std::unique_ptr<ai2d_builder> &builder, runtime_tensor &ai2d_in_tensor, runtime_tensor &ai2d_out_tensor)
{
    // run ai2d
    ai2d_datatype_t ai2d_dtype{ai2d_format::NCHW_FMT, ai2d_format::NCHW_FMT, ai2d_in_tensor.datatype(), ai2d_out_tensor.datatype()};
    ai2d_crop_param_t crop_param{true, crop_info.x, crop_info.y, crop_info.w, crop_info.h};
    ai2d_shift_param_t shift_param{false, 0};
    ai2d_pad_param_t pad_param{false, {{0, 0}, {0, 0}, {0, 0}, {0, 0}}, ai2d_pad_mode::constant, {114, 114, 114}};
    ai2d_resize_param_t resize_param{true, ai2d_interp_method::tf_bilinear, ai2d_interp_mode::half_pixel};
    ai2d_affine_param_t affine_param{false, ai2d_interp_method::cv2_bilinear, 0, 0, 127, 1, {0.5, 0.1, 0.0, 0.1, 0.5, 0.0}};

    dims_t in_shape = ai2d_in_tensor.shape();
    dims_t out_shape = ai2d_out_tensor.shape();
    builder.reset(new ai2d_builder(in_shape, out_shape, ai2d_dtype, crop_param, shift_param, pad_param, resize_param, affine_param));
    builder->build_schedule();
    builder->invoke(ai2d_in_tensor,ai2d_out_tensor).expect("error occurred in ai2d running");
}

void Utils::padding_resize(FrameCHWSize ori_shape, std::vector<uint8_t> &chw_vec, FrameSize resize_shape, runtime_tensor &ai2d_out_tensor, cv::Scalar padding)
{
    int ori_w = ori_shape.width;
    int ori_h = ori_shape.height;
    int width = resize_shape.width;
    int height = resize_shape.height;
    float ratiow = (float)width / ori_w;
    float ratioh = (float)height / ori_h;
    float ratio = ratiow < ratioh ? ratiow : ratioh;
    int new_w = (int)(ratio * ori_w);
    int new_h = (int)(ratio * ori_h);
    float dw = (float)(width - new_w) / 2;
    float dh = (float)(height - new_h) / 2;
    int top = (int)(roundf(dh - 0.1));
    int bottom = (int)(roundf(dh + 0.1));
    int left = (int)(roundf(dw - 0.1));
    int right = (int)(roundf(dw - 0.1));

    // create input
    dims_t in_shape{1, ori_shape.channel, ori_h, ori_w};
    auto ai2d_in_tensor = host_runtime_tensor::create(typecode_t::dt_uint8, in_shape, hrt::pool_shared).expect("cannot create input tensor");
    auto input_buf = ai2d_in_tensor.impl()->to_host().unwrap()->buffer().as_host().unwrap().map(map_access_::map_write).unwrap().buffer();
    memcpy(reinterpret_cast<char *>(input_buf.data()), chw_vec.data(), chw_vec.size());
    hrt::sync(ai2d_in_tensor, sync_op_t::sync_write_back, true).expect("write back input failed");

    // run ai2d
    ai2d_datatype_t ai2d_dtype{ai2d_format::NCHW_FMT, ai2d_format::NCHW_FMT, ai2d_in_tensor.datatype(), ai2d_out_tensor.datatype()};
    ai2d_crop_param_t crop_param{false, 0, 0, 0, 0};
    ai2d_shift_param_t shift_param{false, 0};
    ai2d_pad_param_t pad_param{true, {{0, 0}, {0, 0}, {top, bottom}, {left, right}}, ai2d_pad_mode::constant, {padding[0], padding[1], padding[2]}};
    ai2d_resize_param_t resize_param{true, ai2d_interp_method::tf_bilinear, ai2d_interp_mode::half_pixel};
    ai2d_affine_param_t affine_param{false, ai2d_interp_method::cv2_bilinear, 0, 0, 127, 1, {0.5, 0.1, 0.0, 0.1, 0.5, 0.0}};

    dims_t out_shape = ai2d_out_tensor.shape();
    ai2d_builder builder { in_shape, out_shape, ai2d_dtype, crop_param, shift_param, pad_param, resize_param, affine_param };
    builder.build_schedule();
    builder.invoke(ai2d_in_tensor,ai2d_out_tensor).expect("error occurred in ai2d running");
}

void Utils::padding_resize_one_side(FrameCHWSize ori_shape, std::vector<uint8_t> &chw_vec, FrameSize resize_shape, runtime_tensor &ai2d_out_tensor, cv::Scalar padding)
{
    int ori_w = ori_shape.width;
    int ori_h = ori_shape.height;
    int width = resize_shape.width;
    int height = resize_shape.height;
    float ratiow = (float)width / ori_w;
    float ratioh = (float)height / ori_h;
    float ratio = ratiow < ratioh ? ratiow : ratioh;
    int new_w = (int)(ratio * ori_w);
    int new_h = (int)(ratio * ori_h);
    float dw = (float)(width - new_w) / 2;
    float dh = (float)(height - new_h) / 2;
    int top = (int)(roundf(0));
    int bottom = (int)(roundf(dh * 2 + 0.1));
    int left = (int)(roundf(0));
    int right = (int)(roundf(dw * 2 - 0.1));

    // create input
    dims_t in_shape{1, ori_shape.channel, ori_h, ori_w};
    auto ai2d_in_tensor = host_runtime_tensor::create(typecode_t::dt_uint8, in_shape, hrt::pool_shared).expect("cannot create input tensor");
    auto input_buf = ai2d_in_tensor.impl()->to_host().unwrap()->buffer().as_host().unwrap().map(map_access_::map_write).unwrap().buffer();
    memcpy(reinterpret_cast<char *>(input_buf.data()), chw_vec.data(), chw_vec.size());
    hrt::sync(ai2d_in_tensor, sync_op_t::sync_write_back, true).expect("write back input failed");

    // run ai2d
    ai2d_datatype_t ai2d_dtype{ai2d_format::NCHW_FMT, ai2d_format::NCHW_FMT, ai2d_in_tensor.datatype(), ai2d_out_tensor.datatype()};
    ai2d_crop_param_t crop_param{false, 0, 0, 0, 0};
    ai2d_shift_param_t shift_param{false, 0};
    ai2d_pad_param_t pad_param{true, {{0, 0}, {0, 0}, {top, bottom}, {left, right}}, ai2d_pad_mode::constant, {padding[0], padding[1], padding[2]}};
    ai2d_resize_param_t resize_param{true, ai2d_interp_method::tf_bilinear, ai2d_interp_mode::half_pixel};
    ai2d_affine_param_t affine_param{false, ai2d_interp_method::cv2_bilinear, 0, 0, 127, 1, {0.5, 0.1, 0.0, 0.1, 0.5, 0.0}};

    dims_t out_shape = ai2d_out_tensor.shape();
    ai2d_builder builder { in_shape, out_shape, ai2d_dtype, crop_param, shift_param, pad_param, resize_param, affine_param };
    builder.build_schedule();
    builder.invoke(ai2d_in_tensor,ai2d_out_tensor).expect("error occurred in ai2d running");
}

void Utils::padding_resize(FrameCHWSize ori_shape, FrameSize resize_shape, std::unique_ptr<ai2d_builder> &builder, runtime_tensor &ai2d_in_tensor, runtime_tensor &ai2d_out_tensor, const cv::Scalar padding)
{
    int ori_w = ori_shape.width;
    int ori_h = ori_shape.height;
    int width = resize_shape.width;
    int height = resize_shape.height;
    float ratiow = (float)width / ori_w;
    float ratioh = (float)height / ori_h;
    float ratio = ratiow < ratioh ? ratiow : ratioh;
    int new_w = (int)(ratio * ori_w);
    int new_h = (int)(ratio * ori_h);
    float dw = (float)(width - new_w) / 2;
    float dh = (float)(height - new_h) / 2;
    int top = (int)(roundf(dh - 0.1));
    int bottom = (int)(roundf(dh + 0.1));
    int left = (int)(roundf(dw - 0.1));
    int right = (int)(roundf(dw - 0.1));

    // run ai2d
    ai2d_datatype_t ai2d_dtype{ai2d_format::NCHW_FMT, ai2d_format::NCHW_FMT, ai2d_in_tensor.datatype(), ai2d_out_tensor.datatype()};
    ai2d_crop_param_t crop_param{false, 0, 0, 0, 0};
    ai2d_shift_param_t shift_param{false, 0};
    ai2d_pad_param_t pad_param{true, {{0, 0}, {0, 0}, {top, bottom}, {left, right}}, ai2d_pad_mode::constant, {padding[0], padding[1], padding[2]}};
    ai2d_resize_param_t resize_param{true, ai2d_interp_method::tf_bilinear, ai2d_interp_mode::half_pixel};
    ai2d_affine_param_t affine_param{false, ai2d_interp_method::cv2_bilinear, 0, 0, 127, 1, {0.5, 0.1, 0.0, 0.1, 0.5, 0.0}};

    dims_t in_shape = ai2d_in_tensor.shape();
    dims_t out_shape = ai2d_out_tensor.shape();
    builder.reset(new ai2d_builder(in_shape, out_shape, ai2d_dtype, crop_param, shift_param, pad_param, resize_param, affine_param));
    builder->build_schedule();
    builder->invoke(ai2d_in_tensor,ai2d_out_tensor).expect("error occurred in ai2d running");
}

void Utils::padding_resize_one_side(FrameCHWSize ori_shape, FrameSize resize_shape, std::unique_ptr<ai2d_builder> &builder, runtime_tensor &ai2d_in_tensor, runtime_tensor &ai2d_out_tensor, const cv::Scalar padding)
{
    int ori_w = ori_shape.width;
    int ori_h = ori_shape.height;
    int width = resize_shape.width;
    int height = resize_shape.height;
    float ratiow = (float)width / ori_w;
    float ratioh = (float)height / ori_h;
    float ratio = ratiow < ratioh ? ratiow : ratioh;
    int new_w = (int)(ratio * ori_w);
    int new_h = (int)(ratio * ori_h);
    float dw = (float)(width - new_w) / 2;
    float dh = (float)(height - new_h) / 2;
    int top = (int)(roundf(0));
    int bottom = (int)(roundf(dh * 2 + 0.1));
    int left = (int)(roundf(0));
    int right = (int)(roundf(dw * 2 - 0.1));

    // run ai2d
    ai2d_datatype_t ai2d_dtype{ai2d_format::NCHW_FMT, ai2d_format::NCHW_FMT, ai2d_in_tensor.datatype(), ai2d_out_tensor.datatype()};
    ai2d_crop_param_t crop_param{false, 0, 0, 0, 0};
    ai2d_shift_param_t shift_param{false, 0};
    ai2d_pad_param_t pad_param{true, {{0, 0}, {0, 0}, {top, bottom}, {left, right}}, ai2d_pad_mode::constant, {padding[0], padding[1], padding[2]}};
    ai2d_resize_param_t resize_param{true, ai2d_interp_method::tf_bilinear, ai2d_interp_mode::half_pixel};
    ai2d_affine_param_t affine_param{false, ai2d_interp_method::cv2_bilinear, 0, 0, 127, 1, {0.5, 0.1, 0.0, 0.1, 0.5, 0.0}};

    dims_t in_shape = ai2d_in_tensor.shape();
    dims_t out_shape = ai2d_out_tensor.shape();
    builder.reset(new ai2d_builder(in_shape, out_shape, ai2d_dtype, crop_param, shift_param, pad_param, resize_param, affine_param));
    builder->build_schedule();
    builder->invoke(ai2d_in_tensor,ai2d_out_tensor).expect("error occurred in ai2d running");
}

void Utils::affine(FrameCHWSize ori_shape, std::vector<uint8_t> &ori_data, float *affine_matrix, runtime_tensor &ai2d_out_tensor)
{
    runtime_tensor ai2d_in_tensor;
    // init ai2d in/out
    dims_t in_shape{1, ori_shape.channel, ori_shape.height, ori_shape.width};
    ai2d_in_tensor = host_runtime_tensor::create(typecode_t::dt_uint8, in_shape, hrt::pool_shared).expect("cannot create input tensor");

    // ai2d input
    auto input_buf = ai2d_in_tensor.impl()->to_host().unwrap()->buffer().as_host().unwrap().map(map_access_::map_write).unwrap().buffer();
    memcpy(reinterpret_cast<char *>(input_buf.data()), ori_data.data(), ori_data.size());
    hrt::sync(ai2d_in_tensor, sync_op_t::sync_write_back, true).expect("write back input failed");

    // run ai2d
    
    ai2d_datatype_t ai2d_dtype{ai2d_format::NCHW_FMT, ai2d_format::NCHW_FMT, ai2d_in_tensor.datatype(), ai2d_out_tensor.datatype()};
    ai2d_crop_param_t crop_param{false, 0, 0, 0, 0};
    ai2d_shift_param_t shift_param{false, 0};
    ai2d_pad_param_t pad_param{false, {{0, 0}, {0, 0}, {0, 0}, {10, 0}}, ai2d_pad_mode::constant, {255, 10, 5}};
    ai2d_resize_param_t resize_param{false, ai2d_interp_method::tf_bilinear, ai2d_interp_mode::half_pixel};
    ai2d_affine_param_t affine_param{true, ai2d_interp_method::cv2_bilinear, 0, 0, 127, 1, {affine_matrix[0], affine_matrix[1], affine_matrix[2], affine_matrix[3], affine_matrix[4], affine_matrix[5]}};

    dims_t out_shape = ai2d_out_tensor.shape();
    ai2d_builder builder { in_shape, out_shape, ai2d_dtype, crop_param, shift_param, pad_param, resize_param, affine_param };
    builder.build_schedule();
    builder.invoke(ai2d_in_tensor,ai2d_out_tensor).expect("error occurred in ai2d running");
}

// for video(只算一次即可)
void Utils::affine(float *affine_matrix, std::unique_ptr<ai2d_builder> &builder, runtime_tensor &ai2d_in_tensor, runtime_tensor &ai2d_out_tensor)
{
    // run ai2d
    ai2d_datatype_t ai2d_dtype{ai2d_format::NCHW_FMT, ai2d_format::NCHW_FMT, ai2d_in_tensor.datatype(), ai2d_out_tensor.datatype()};
    ai2d_crop_param_t crop_param{false, 0, 0, 0, 0};
    ai2d_shift_param_t shift_param{false, 0};
    ai2d_pad_param_t pad_param{false, {{0, 0}, {0, 0}, {0, 0}, {10, 0}}, ai2d_pad_mode::constant, {255, 10, 5}};
    ai2d_resize_param_t resize_param{false, ai2d_interp_method::tf_bilinear, ai2d_interp_mode::half_pixel};
    ai2d_affine_param_t affine_param{true, ai2d_interp_method::cv2_bilinear, 0, 0, 127, 1, {affine_matrix[0], affine_matrix[1], affine_matrix[2], affine_matrix[3], affine_matrix[4], affine_matrix[5]}};

    dims_t in_shape = ai2d_in_tensor.shape();
    dims_t out_shape = ai2d_out_tensor.shape();
    builder.reset(new ai2d_builder(in_shape, out_shape, ai2d_dtype, crop_param, shift_param, pad_param, resize_param, affine_param));
    builder->build_schedule();
    builder->invoke(ai2d_in_tensor,ai2d_out_tensor).expect("error occurred in ai2d running");
}


void Utils::chw_rgb2bgr(const FrameSize &frame_size, unsigned char *data, std::vector<uint8_t> &chw_bgr_vec)
{
    chw_bgr_vec.insert(chw_bgr_vec.end(), data+2*frame_size.height*frame_size.width, data+3*frame_size.height*frame_size.width);
    chw_bgr_vec.insert(chw_bgr_vec.end(), data+frame_size.height*frame_size.width, data+2*frame_size.height*frame_size.width);
    chw_bgr_vec.insert(chw_bgr_vec.end(), data, data+frame_size.height*frame_size.width);
}


void Utils::draw_cls_res(cv::Mat& frame, vector<cls_res>& results)
{
    double fontsize = (frame.cols * frame.rows * 1.0) / (300 * 250);
    if (fontsize > 2)
    {
        fontsize = 2;
    }

    for(int i = 0; i < results.size(); i++)
    {   
        std::string text = "class: " + results[i].label + ", score: " + std::to_string(round(results[i].score * 100) / 100.0).substr(0, 4);

        cv::putText(frame, text, cv::Point(1, 40), cv::FONT_HERSHEY_SIMPLEX, fontsize, cv::Scalar(255, 255, 0), 2);

        std::cout << text << std::endl;
    }
}

void Utils::draw_cls_res(cv::Mat& frame, vector<cls_res>& results, FrameSize osd_frame_size, FrameSize sensor_frame_size)
{
    double fontsize = (frame.cols * frame.rows * 1.0) / (1100 * 1200);

    for(int i = 0; i < results.size(); i++)
    {   
        std::string text = "class: " + results[i].label + ", score: " + std::to_string(round(results[i].score * 100) / 100.0).substr(0, 4);

        cv::putText(frame, text, cv::Point(1, 40), cv::FONT_HERSHEY_SIMPLEX, fontsize, cv::Scalar(255, 255, 255, 0), 2);

        std::cout << text << std::endl;
    }
}

void Utils::draw_ob_det_res(cv::Mat& frame, vector<ob_det_res>& results)
{

    double fontsize = (frame.cols * frame.rows * 1.0) / (300 * 250);
    if (fontsize > 2)
    {
        fontsize = 2;
    }
    
    for (int i = 0; i < results.size(); ++i)
    {
        ob_det_res detection = results[i];

        cv::Rect box = cv::Rect(detection.x1, detection.y1, detection.x2 - detection.x1, detection.y2 - detection.y1);

        cv::Scalar color;
        if (detection.label_index < 80)
        {
            color = color_three[detection.label_index];
        }
        else
        {
            std::random_device rd;
            std::mt19937 gen(rd());
            std::uniform_int_distribution<int> dis(100, 255);
            color = cv::Scalar(dis(gen),dis(gen),dis(gen));
        }
        
        // Detection box
        cv::rectangle(frame, box, color, 2);

        // Detection box text
        std::string classString = detection.label + ' ' + std::to_string(detection.score).substr(0, 4);
        cv::Size textSize = cv::getTextSize(classString, cv::FONT_HERSHEY_DUPLEX, fontsize, 2, 0);
        cv::Rect textBox(box.x, box.y - (textSize.height + 20), textSize.width + 10, textSize.height + 20);

        cv::rectangle(frame, textBox, color, cv::FILLED);
        cv::putText(frame, classString, cv::Point(box.x + 5, box.y - 10), cv::FONT_HERSHEY_DUPLEX, fontsize, cv::Scalar(0,0,0), 2, 0);

        std::cout << classString << std::endl;
    }
}

void Utils::draw_ob_det_res(cv::Mat& frame, vector<ob_det_res>& results, FrameSize osd_frame_size, FrameSize sensor_frame_size)
{
    double fontsize = (frame.cols * frame.rows * 1.0) / (1100 * 1200);

    for (int i = 0; i < results.size(); ++i)
    {
        ob_det_res detection = results[i];

        float rect_x = (float(detection.x1)/sensor_frame_size.width)*osd_frame_size.width;
        float rect_y = (float(detection.y1)/sensor_frame_size.height)*osd_frame_size.height;
        float rect_w = float(detection.x2 - detection.x1) / sensor_frame_size.width * osd_frame_size.width;
        float rect_h = float(detection.y2 - detection.y1) / sensor_frame_size.height * osd_frame_size.height;

        cv::Rect box = cv::Rect(rect_x, rect_y, rect_w, rect_h);

        cv::Scalar color;
        if (detection.label_index < 80)
        {
            color = color_four[detection.label_index];
        }
        else
        {
            std::random_device rd;
            std::mt19937 gen(rd());
            std::uniform_int_distribution<int> dis(100, 255);
            color = cv::Scalar(255,dis(gen),dis(gen),dis(gen));
        }
        
        // Detection box
        cv::rectangle(frame, box, color, 2);

        // Detection box text
        std::string classString = detection.label + ' ' + std::to_string(detection.score).substr(0, 4);
        cv::Size textSize = cv::getTextSize(classString, cv::FONT_HERSHEY_DUPLEX, fontsize, 2, 0);
        cv::Rect textBox(box.x, box.y - (textSize.height + 20), textSize.width + 10, textSize.height + 20);

        cv::rectangle(frame, textBox, color, cv::FILLED);
        cv::putText(frame, classString, cv::Point(box.x + 5, box.y - 10), cv::FONT_HERSHEY_DUPLEX, fontsize, cv::Scalar(255,0,0,0), 2, 0);

        std::cout << classString << std::endl;
    }
}

void Utils::draw_mlcls_res(cv::Mat& frame, vector<multi_lable_res>& results)
{
    double fontsize = (frame.cols * frame.rows * 1.0) / (300 * 250);
    if (fontsize > 2)
    {
        fontsize = 2;
    }

    for(int i = 0; i < results.size(); i++)
    {   
        std::string text;
        int point_x = 15;
        int point_y = 0;
        cv::Scalar color;
        for (int j = 0; j < results[i].labels.size(); j++)
        {
            if (results[i].id_vec[j] == 1)
            {
                text = "class: " + results[i].labels[j] + ", score: " + std::to_string(round(results[i].score_vec[j] * 100) / 100.0).substr(0, 4);
            }
            else
            {
                text = "class: NO " + results[i].labels[j];
            }

            if (j < 80)
            {
                color = color_three[j];
            }
            else
            {
                std::random_device rd;
                std::mt19937 gen(rd());
                std::uniform_int_distribution<int> dis(100, 255);
                color = cv::Scalar(dis(gen),dis(gen),dis(gen));
            }


            cv::Size textSize = cv::getTextSize(text, cv::FONT_HERSHEY_DUPLEX, fontsize, 2, 0);
            if (results[i].id_vec[j] == 1)
            {
                point_y += textSize.height + 10;
                cv::putText(frame, text, cv::Point(point_x, point_y), cv::FONT_HERSHEY_DUPLEX, fontsize, color, 2, 0);
            }

            std::cout << text << std::endl;
        }
    }
}

void Utils::draw_mlcls_res(cv::Mat& frame, vector<multi_lable_res>& results, FrameSize osd_frame_size, FrameSize sensor_frame_size)
{
    double fontsize = (frame.cols * frame.rows * 1.0) / (1100 * 1200);

    for(int i = 0; i < results.size(); i++)
    {   
        std::string text;
        int point_x = 15;
        int point_y = 0;
        cv::Scalar color;
        for (int j = 0; j < results[i].labels.size(); j++)
        {
            if (results[i].id_vec[j] == 1)
            {
                text = "class: " + results[i].labels[j] + ", score: " + std::to_string(round(results[i].score_vec[j] * 100) / 100.0).substr(0, 4);
            }
            else
            {
                text = "class: NO " + results[i].labels[j];
            }

            if (j < 80)
            {
                color = color_four[j];
            }
            else
            {
                std::random_device rd;
                std::mt19937 gen(rd());
                std::uniform_int_distribution<int> dis(100, 255);
                color = cv::Scalar(255,dis(gen),dis(gen),dis(gen));
            }

            cv::Size textSize = cv::getTextSize(text, cv::FONT_HERSHEY_DUPLEX, fontsize, 2, 0);
            if (results[i].id_vec[j] == 1)
            {
                point_y += textSize.height + 10;
                cv::putText(frame, text, cv::Point(point_x, point_y), cv::FONT_HERSHEY_DUPLEX, fontsize, color, 2, 0);
            }
            std::cout << text << std::endl;
        }
    }
}




void expandRectangle(Point2f& topLeft, Point2f& topRight, Point2f& bottomRight, Point2f& bottomLeft, float scaleFactor_width,float scaleFactor_high, int maxWidth, int maxHeight) {
    // Calculate the center of the original rectangle

    Point2f newTopLeft(scaleFactor_width * topLeft.x - (scaleFactor_width-1)*topRight.x, scaleFactor_high * topLeft.y - (scaleFactor_high-1)*bottomLeft.y);
    Point2f newTopRight(scaleFactor_width * topRight.x - (scaleFactor_width-1)*topLeft.x, scaleFactor_high * topRight.y - (scaleFactor_high-1)*bottomRight.y);
    Point2f newBottomRight(scaleFactor_width * bottomRight.x - (scaleFactor_width-1)*bottomLeft.x, scaleFactor_high * bottomRight.y - (scaleFactor_high-1)*topRight.y);
    Point2f newBottomLeft(scaleFactor_width * bottomLeft.x - (scaleFactor_width-1)*bottomRight.x, scaleFactor_high * bottomLeft.y - (scaleFactor_high-1)*topLeft.y);

    // Check and adjust vertices to stay within specified boundaries
    if (newTopLeft.x < 0) newTopLeft.x = 0;
    if (newTopLeft.y < 0) newTopLeft.y = 0;
    if (newTopRight.x > maxWidth) newTopRight.x = maxWidth;
    if (newTopRight.y < 0) newTopRight.y = 0;
    if (newBottomRight.x > maxWidth) newBottomRight.x = maxWidth;
    if (newBottomRight.y > maxHeight) newBottomRight.y = maxHeight;
    if (newBottomLeft.x < 0) newBottomLeft.x = 0;
    if (newBottomLeft.y > maxHeight) newBottomLeft.y = maxHeight;

    // Update the input points with the adjusted vertices
    topLeft = newTopLeft;
    topRight = newTopRight;
    bottomRight = newBottomRight;
    bottomLeft = newBottomLeft;
    
}
std::vector<size_t> sort_indices(const std::vector<Point2f>& vec) 
{
	std::vector<std::pair<Point2f, size_t>> indexedVec;
	indexedVec.reserve(vec.size());

	// 创建带有索引的副本
	for (size_t i = 0; i < vec.size(); ++i) {
		indexedVec.emplace_back(vec[i], i);
	}

	// 按值对副本进行排序
	std::sort(indexedVec.begin(), indexedVec.end(),
		[](const auto& a, const auto& b) {
		return a.first.x < b.first.x;
	});

	// 提取排序后的索引
	std::vector<size_t> sortedIndices;
	sortedIndices.reserve(vec.size());
	for (const auto& element : indexedVec) {
		sortedIndices.push_back(element.second);
	}

	return sortedIndices;
}
void find_rectangle_vertices(const std::vector<Point2f>& points, Point2f& topLeft, Point2f& topRight, Point2f& bottomRight, Point2f& bottomLeft) 
{
    //先按照x排序,比较左右，再按照y比较上下
	auto sorted_x_id = sort_indices(points);

	if (points[sorted_x_id[0]].y < points[sorted_x_id[1]].y)
	{
		topLeft = points[sorted_x_id[0]];
		bottomLeft = points[sorted_x_id[1]];
	}
	else
	{
		topLeft = points[sorted_x_id[1]];
		bottomLeft = points[sorted_x_id[0]];
	}

	if (points[sorted_x_id[2]].y < points[sorted_x_id[3]].y)
	{
        bottomRight = points[sorted_x_id[3]];
		topRight = points[sorted_x_id[2]];

	}
	else
	{ 
        bottomRight = points[sorted_x_id[2]];
		topRight = points[sorted_x_id[3]];
	}
	
}
void Utils::draw_ocr_det_res(cv::Mat& frame, vector<ocr_det_res>& results)
{
    double fontsize = (frame.cols * frame.rows * 1.0) / (1100 * 1200);
    
    for(int i = 0; i < results.size(); i++)
    {   
        std::vector<cv::Point> vec;
        vec.clear();
        for(int j = 0; j < 4; j++)
        {
            vec.push_back(results[i].vertices[j]);
        }
        cv::RotatedRect rect = minAreaRect(vec);
        std::vector<Point2f> ver(4),vtd(4);
        rect.points(ver.data());
        int maxWidth = frame.cols;
        int maxHeight = frame.rows;
        float scaleFactor_width = 1.025;
        float scaleFactor_higt = 1.15;
        
        find_rectangle_vertices(ver, vtd[0], vtd[1], vtd[2], vtd[3]);
        expandRectangle(vtd[0], vtd[1], vtd[2], vtd[3],scaleFactor_width,scaleFactor_higt,maxWidth,maxHeight);

        for(int i = 0; i < 4; i++)
            line(frame, ver[i], ver[(i + 1) % 4], Scalar(255, 0, 0), 3);
        std::string text = "score:" + std::to_string(round(results[i].score * 100) / 100.0).substr(0, 4);
        cv::putText(frame, text, cv::Point(results[i].meanx, results[i].meany), 
        cv::FONT_HERSHEY_SIMPLEX, fontsize, cv::Scalar(255, 255, 0), 2);
    }
}
void Utils::draw_ocr_det_res(cv::Mat& frame, vector<ocr_det_res>& results, FrameSize osd_frame_size, FrameSize sensor_frame_size)
{
    for(int i = 0; i < results.size(); i++)
    {   
        std::vector<cv::Point> vec;
        vec.clear();
        for(int j = 0; j < 4; j++)
        {
            cv::Point tmp = results[i].vertices[j];
            tmp.x = (float(tmp.x)/sensor_frame_size.width)*osd_frame_size.width;
            tmp.y = (float(tmp.y)/sensor_frame_size.height)*osd_frame_size.height;
            vec.push_back(tmp);
        }
        cv::RotatedRect rect = minAreaRect(vec);
        std::vector<Point2f> ver(4),vtd(4);
        rect.points(ver.data());
        int maxWidth = frame.cols;
        int maxHeight = frame.rows;
        float scaleFactor_width = 1.025;
        float scaleFactor_higt = 1.15;
        
        find_rectangle_vertices(ver, vtd[0], vtd[1], vtd[2], vtd[3]);
        expandRectangle(vtd[0], vtd[1], vtd[2], vtd[3],scaleFactor_width,scaleFactor_higt,maxWidth,maxHeight);

        for(int i = 0; i < 4; i++)
            line(frame, vtd[i], vtd[(i + 1) % 4], Scalar(255,255, 0, 0), 3);
    }
}




void Utils::warppersp(cv::Mat src, cv::Mat& dst, ocr_det_res b, std::vector<Point2f>& vtd)
{
    Mat rotation;
    vector<Point> con;
    for(auto i : b.vertices)
        con.push_back(i);

    RotatedRect minrect = minAreaRect(con);
    std::vector<Point2f> vtx(4),vt(4);
    minrect.points(vtx.data());

    find_rectangle_vertices(vtx, vtd[0], vtd[1], vtd[2], vtd[3]);

    int maxWidth = src.cols;
    int maxHeight = src.rows;
    float scaleFactor_width = 1.025;
    float scaleFactor_higt = 1.15;
    expandRectangle(vtd[0], vtd[1], vtd[2], vtd[3],scaleFactor_width,scaleFactor_higt,maxWidth,maxHeight);

    
    float tmp_w = cv::norm(vtd[1]-vtd[0]);
    float tmp_h = cv::norm(vtd[2]-vtd[1]);
    float w = std::max(tmp_w,tmp_h);
    float h = std::min(tmp_w,tmp_h);

    vt[0].x = 0;
    vt[0].y = 0;
    vt[1].x = w;//w
    vt[1].y = 0;
    vt[2].x = w;
    vt[2].y = h;
    vt[3].x = 0;
    vt[3].y = h;//h
    rotation = getPerspectiveTransform(vtd, vt);

    warpPerspective(src, dst, rotation, Size(w, h));
}

void Utils::paint_ascii(cv::Mat& image,int x_offset,int y_offset,unsigned long offset)
{
    Point p;
	p.x = x_offset;
	p.y = y_offset;
	//存放ascii字膜
	char buff[16];           
	//打开ascii字库文件
	FILE *ASCII;
	if ((ASCII = fopen(WORD_ENCODE, "rb")) == NULL){
        printf("Can't open ascii.zf,Please check the path!");
		//getch();
		exit(0);
	}
	fseek(ASCII, offset, SEEK_SET);
	fread(buff, 16, 1, ASCII);
    fclose(ASCII);
	int i, j;
	Point p1 = p;
	for (i = 0; i<16; i++)                  //十六个char
	{
		p.x = x_offset;
		for (j = 0; j < 8; j++)              //一个char八个bit
		{
			p1 = p;
			if (buff[i] & (0x80 >> j))    /*测试当前位是否为1*/
			{
				/*
					由于原本ascii字膜是8*16的，不够大，
					所以原本的一个像素点用4个像素点替换，
					替换后就有16*32个像素点
					ps：感觉这样写代码多余了，但目前暂时只想到了这种方法
				*/
				circle(image, p1, 0, Scalar(0, 0, 255), -1);
				p1.x++;
				circle(image, p1, 0, Scalar(0, 0, 255), -1);
				p1.y++;
				circle(image, p1, 0, Scalar(0, 0, 255), -1);
				p1.x--;
				circle(image, p1, 0, Scalar(0, 0, 255), -1);
			}						
			p.x+=2;            //原来的一个像素点变为四个像素点，所以x和y都应该+2
		}
		p.y+=2;
	}
}

void Utils::paint_ascii_video(cv::Mat& image,int x_offset,int y_offset,unsigned long offset)
{
    Point p;
	p.x = x_offset;
	p.y = y_offset;
	//存放ascii字膜
	char buff[16];           
	//打开ascii字库文件
	FILE *ASCII;
	if ((ASCII = fopen(WORD_ENCODE, "rb")) == NULL){
        printf("Can't open ascii.zf,Please check the path!");
		//getch();
		exit(0);
	}
	fseek(ASCII, offset, SEEK_SET);
	fread(buff, 16, 1, ASCII);
    fclose(ASCII);
	int i, j;
	Point p1 = p;
	for (i = 0; i<16; i++)                  //十六个char
	{
		p.x = x_offset;
		for (j = 0; j < 8; j++)              //一个char八个bit
		{
			p1 = p;
			if (buff[i] & (0x80 >> j))    /*测试当前位是否为1*/
			{
				/*
					由于原本ascii字膜是8*16的，不够大，
					所以原本的一个像素点用4个像素点替换，
					替换后就有16*32个像素点
					ps：感觉这样写代码多余了，但目前暂时只想到了这种方法
				*/
				circle(image, p1, 0, Scalar(255, 0, 0, 255), -1);
				p1.x--;
				circle(image, p1, 0, Scalar(255, 0, 0, 255), -1);
				p1.y--;
				circle(image, p1, 0, Scalar(255, 0, 0, 255), -1);
				p1.x++;
				circle(image, p1, 0, Scalar(255, 0, 0, 255), -1);
			}						
			p.x+=2;            //原来的一个像素点变为四个像素点，所以x和y都应该+2
		}
		p.y+=2;
	}
}

void Utils::paint_chinese(cv::Mat& image,int x_offset,int y_offset,unsigned long offset)
{
    Point p;
    p.x=x_offset;
    p.y=y_offset;
    FILE *HZK;
    char buff[72];//72个字节，用来存放汉字的
   if((HZK=fopen(CHINESE_ENCODE,"rb"))==NULL){
        printf("Can't open HZKf2424.hz,Please check the path!");
        exit(0);//退出
    }
    fseek(HZK, offset, SEEK_SET);/*将文件指针移动到偏移量的位置*/
    fread(buff, 72, 1, HZK);/*从偏移量的位置读取72个字节，每个汉字占72个字节*/
    fclose(HZK);
    bool mat[24][24];//定义一个新的矩阵存放转置后的文字字膜
    int i,j,k;
    for (i = 0; i<24; i++)                 /*24x24点阵汉字，一共有24行*/
	{
		for (j = 0; j<3; j++)                /*横向有3个字节，循环判断每个字节的*/
			for (k = 0; k<8; k++)              /*每个字节有8位，循环判断每位是否为1*/
				if (buff[i * 3 + j] & (0x80 >> k))    /*测试当前位是否为1*/
				{
					mat[j * 8 + k][i] = true;          /*为1的存入新的字膜中*/
				}
				else {
					mat[j * 8 + k][i] = false;
				}
	}
    for (i = 0; i < 24; i++)
	{
		p.x = x_offset;
		for (j = 0; j < 24; j++)
		{		
			if (mat[i][j])
				circle(image, p, 1, Scalar(255, 0, 0), -1);		  //写(替换)像素点
			p.x++;                                                //右移一个像素点
		}
		p.y++;                                                    //下移一个像素点
	}
}

void Utils::paint_chinese_video(cv::Mat& image,int x_offset,int y_offset,unsigned long offset)
{
    Point p;
    p.x=x_offset;
    p.y=y_offset;
    FILE *HZK;
    char buff[72];//72个字节，用来存放汉字的
   if((HZK=fopen(CHINESE_ENCODE,"rb"))==NULL){
        printf("Can't open HZKf2424.hz,Please check the path!");
        exit(0);//退出
    }
    fseek(HZK, offset, SEEK_SET);/*将文件指针移动到偏移量的位置*/
    fread(buff, 72, 1, HZK);/*从偏移量的位置读取72个字节，每个汉字占72个字节*/
    fclose(HZK);
    bool mat[24][24];//定义一个新的矩阵存放转置后的文字字膜
    int i,j,k;
    for (i = 0; i<24; i++)                 /*24x24点阵汉字，一共有24行*/
	{
		for (j = 0; j<3; j++)                /*横向有3个字节，循环判断每个字节的*/
			for (k = 0; k<8; k++)              /*每个字节有8位，循环判断每位是否为1*/
				if (buff[i * 3 + j] & (0x80 >> k))    /*测试当前位是否为1*/
				{
					mat[j * 8 + k][i] = true;          /*为1的存入新的字膜中*/
				}
				else {
					mat[j * 8 + k][i] = false;
				}
	}
    for (i = 0; i < 24; i++)
	{
		p.x = x_offset;
		for (j = 0; j < 24; j++)
		{		
			if (mat[i][j])
				circle(image, p, 1, Scalar(255, 255, 0, 0), -1);		  //写(替换)像素点
			p.x++;                                                //右移一个像素点
		}
		p.y++;                                                    //下移一个像素点
	}
}

void Utils::draw_ocr_rec_res(cv::Mat& image, vector<unsigned char>& vec16)
{

    int x_offset = 2;
    int y_offset = 2;
    //x和y就是第一个字在图片上的起始坐标
    //通过图片路径获取图片
    unsigned char qh,wh;      //定义区号，位号
    unsigned long offset;           //偏移量

    int text_length=vec16.size();  
    std::vector<unsigned char> hexcode = vec16;

    int x =x_offset,y = y_offset;//x,y:在图片上绘制文字的起始坐标
    for(int m=0;m<text_length;){
        if(hexcode[m]==0x23){
            break;//读到#号时结束
        }
        else if(hexcode[m]>0xaf){
            qh=hexcode[m]-0xaf;//使用的字库里是以汉字啊开头，而不是以汉字符号开头
            wh=hexcode[m+1] - 0xa0;//计算位码
            offset=(94*(qh-1)+(wh-1))*72L;
            paint_chinese(image,x,y,offset);
            /*
            计算在汉字库中的偏移量
            对于每个汉字，使用24*24的点阵来表示的
            一行有三个字节，一共24行，所以需要72个字节来表示
            */
            m=m+2;//一个汉字的机内码占两个字节，
            x+=24;//一个汉字为24*24个像素点，由于是水平放置，所以是向右移动24个像素点
        }
        else{//当读取的字符为ASCII码时
            wh=hexcode[m];
            offset=wh*16l;//计算英文字符的偏移量
            paint_ascii(image,x,y,offset);
            m++;//英文字符在文件里表示只占一个字节，所以往后移一位就行了
            x+=16;
        }
    }
}

void Utils::draw_ocr_text(int x_offset,int y_offset,cv::Mat& image,vector<unsigned char> vec16)
{
    //x和y就是第一个字在图片上的起始坐标
    //通过图片路径获取图片
    unsigned char qh,wh;      //定义区号，位号
    unsigned long offset;           //偏移量

    int text_length=vec16.size();  
    std::vector<unsigned char> hexcode = vec16;

    int x =x_offset,y = y_offset;//x,y:在图片上绘制文字的起始坐标
    for(int m=0;m<text_length;){
        if(hexcode[m]==0x23){
            break;//读到#号时结束
        }
        else if(hexcode[m]>0xaf){
            qh=hexcode[m]-0xaf;//使用的字库里是以汉字啊开头，而不是以汉字符号开头
            wh=hexcode[m+1] - 0xa0;//计算位码
            offset=(94*(qh-1)+(wh-1))*72L;
            paint_chinese(image,x,y,offset);
            /*
            计算在汉字库中的偏移量
            对于每个汉字，使用24*24的点阵来表示的
            一行有三个字节，一共24行，所以需要72个字节来表示
            */
            m=m+2;//一个汉字的机内码占两个字节，
            x+=24;//一个汉字为24*24个像素点，由于是水平放置，所以是向右移动24个像素点
        }
        else{//当读取的字符为ASCII码时
            wh=hexcode[m];
            offset=wh*16l;//计算英文字符的偏移量
            paint_ascii(image,x,y,offset);
            m++;//英文字符在文件里表示只占一个字节，所以往后移一位就行了
            x+=16;
        }
    }
}

void Utils::draw_ocr_text(float x_offset,float y_offset,cv::Mat& image,vector<unsigned char> vec16, FrameSize osd_frame_size, FrameSize sensor_frame_size)
{
    //x和y就是第一个字在图片上的起始坐标
    //通过图片路径获取图片
    unsigned char qh,wh;      //定义区号，位号
    unsigned long offset;           //偏移量

    int text_length=vec16.size();  
    std::vector<unsigned char> hexcode = vec16;

    int x = int ((x_offset/sensor_frame_size.width) * osd_frame_size.width);
    int y = int ((y_offset/sensor_frame_size.height) * osd_frame_size.height);//x,y:在图片上绘制文字的起始坐标
    for(int m=0;m<text_length;){
        if(hexcode[m]==0x23){
            break;//读到#号时结束
        }
        else if(hexcode[m]>0xaf){
            qh=hexcode[m]-0xaf;//使用的字库里是以汉字啊开头，而不是以汉字符号开头
            wh=hexcode[m+1] - 0xa0;//计算位码
            offset=(94*(qh-1)+(wh-1))*72L;
            paint_chinese_video(image,x,y,offset);
            /*
            计算在汉字库中的偏移量
            对于每个汉字，使用24*24的点阵来表示的
            一行有三个字节，一共24行，所以需要72个字节来表示
            */
            m=m+2;//一个汉字的机内码占两个字节，
            x+=24;//一个汉字为24*24个像素点，由于是水平放置，所以是向右移动24个像素点
        }
        else{//当读取的字符为ASCII码时
            wh=hexcode[m];
            offset=wh*16l;//计算英文字符的偏移量
            paint_ascii_video(image,x,y,offset);
            m++;//英文字符在文件里表示只占一个字节，所以往后移一位就行了
            x+=16;
        }
    }
}