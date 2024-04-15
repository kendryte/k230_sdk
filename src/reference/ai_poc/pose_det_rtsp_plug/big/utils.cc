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
#include <iostream>
#include "utils.h"


using std::ofstream;
using std::vector;
auto cache = cv::Mat::zeros(1,1,CV_32FC1);

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

void Utils::padding_resize_params(cv::Vec4d& params,FrameCHWSize ori_shape, std::vector<uint8_t> &chw_vec, FrameSize resize_shape, runtime_tensor &ai2d_out_tensor, cv::Scalar padding)
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

    params[0] = ratio;
    params[1] = ratio;

    params[2] = left;
    params[3] = top;

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


void Utils::padding_resize_params(cv::Vec4d& params,FrameCHWSize ori_shape, FrameSize resize_shape, std::unique_ptr<ai2d_builder> &builder, runtime_tensor &ai2d_in_tensor, runtime_tensor &ai2d_out_tensor, const cv::Scalar padding)
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
    // int top = (int)(roundf(dh - 0.1));
    // int bottom = (int)(roundf(dh + 0.1));
    // int left = (int)(roundf(dw - 0.1));
    // int right = (int)(roundf(dw - 0.1));

    int top = (int)(roundf(dh ));
    int bottom = (int)(roundf(dh ));
    int left = (int)(roundf(dw ));
    // int right = (int)(roundf(dw - 0.1));
    int right = (int)(roundf(dw ));

    params[0] = ratio;
    params[1] = ratio;

    params[2] = left;
    params[3] = top;

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

static float sigmoid(float x)
{
    return 1.0f / (1.0f + expf(-x));
}

void Utils::nms(std::vector<BoxInfo> &input_boxes, float NMS_THRESH)
{
    std::sort(input_boxes.begin(), input_boxes.end(), [](BoxInfo a, BoxInfo b) { return a.score > b.score; });
    std::vector<float> vArea(input_boxes.size());
    for (int i = 0; i < int(input_boxes.size()); ++i)
    {
        vArea[i] = (input_boxes.at(i).x2 - input_boxes.at(i).x1 + 1)
            * (input_boxes.at(i).y2 - input_boxes.at(i).y1 + 1);
    }
    for (int i = 0; i < int(input_boxes.size()); ++i)
    {
        for (int j = i + 1; j < int(input_boxes.size());)
        {
            float xx1 = std::max(input_boxes[i].x1, input_boxes[j].x1);
            float yy1 = std::max(input_boxes[i].y1, input_boxes[j].y1);
            float xx2 = std::min(input_boxes[i].x2, input_boxes[j].x2);
            float yy2 = std::min(input_boxes[i].y2, input_boxes[j].y2);
            float w = std::max(float(0), xx2 - xx1 + 1);
            float h = std::max(float(0), yy2 - yy1 + 1);
            float inter = w * h;
            float ovr = inter / (vArea[i] + vArea[j] - inter);
            if (ovr >= NMS_THRESH)
            {
                input_boxes.erase(input_boxes.begin() + j);
                vArea.erase(vArea.begin() + j);
            }
            else
            {
                j++;
            }
        }
    }
}

// for NHWC
std::vector<BoxInfo> Utils::decode_infer(float *data, int net_size, int stride, int num_classes, FrameSize frame_size, float anchors[][2], float threshold)
{
    float ratiow = (float)net_size / frame_size.width;
    float ratioh = (float)net_size / frame_size.height;
    float gain = ratiow < ratioh ? ratiow : ratioh;
    std::vector<BoxInfo> result;
    int grid_size = net_size / stride;
    int one_rsize = num_classes + 5;
    float cx, cy, w, h;
    for (int shift_y = 0; shift_y < grid_size; shift_y++)
    {
        for (int shift_x = 0; shift_x < grid_size; shift_x++)
        {
            int loc = shift_x + shift_y * grid_size;
            for (int i = 0; i < 3; i++)
            {
                float *record = data + (loc * 3 + i) * one_rsize;
                float *cls_ptr = record + 5;
                for (int cls = 0; cls < num_classes; cls++)
                {
                    // float score = sigmoid(cls_ptr[cls]) * sigmoid(record[4]);
                    float score = cls_ptr[cls] * record[4];
                    if (score > threshold)
                    {
                        // cx = (sigmoid(record[0]) * 2.f - 0.5f + (float)shift_x) * (float)stride;
                        // cy = (sigmoid(record[1]) * 2.f - 0.5f + (float)shift_y) * (float)stride;
                        // w = pow(sigmoid(record[2]) * 2.f, 2) * anchors[i][0];
                        // h = pow(sigmoid(record[3]) * 2.f, 2) * anchors[i][1];

                        cx = (record[0] * 2.f - 0.5f + (float)shift_x) * (float)stride;
                        cy = (record[1] * 2.f - 0.5f + (float)shift_y) * (float)stride;
                        w = pow(record[2] * 2.f, 2) * anchors[i][0];
                        h = pow(record[3] * 2.f, 2) * anchors[i][1];

                        cx -= ((net_size - frame_size.width * gain) / 2);
                        cy -= ((net_size - frame_size.height * gain) / 2);
                        cx /= gain;
                        cy /= gain;
                        w /= gain;
                        h /= gain;
                        BoxInfo box;
                        box.x1 = std::max(0, std::min<int>(frame_size.width, int(cx - w / 2.f)));
                        box.y1 = std::max(0, std::min<int>(frame_size.height, int(cy - h / 2.f)));
                        box.x2 = std::max(0, std::min<int>(frame_size.width, int(cx + w / 2.f)));
                        box.y2 = std::max(0, std::min<int>(frame_size.height, int(cy + h / 2.f)));
                        box.score = score;
                        box.label = cls;
                        result.push_back(box);
                    }
                }
            }
        }
    }
    return result;
}

void Utils::nms_pose(std::vector<BoxInfo> &input_boxes, float NMS_THRESH,vector<int> &nms_result)
{
    std::sort(input_boxes.begin(), input_boxes.end(), [](BoxInfo a, BoxInfo b) { return a.score > b.score; });
    std::vector<float> vArea(input_boxes.size());
    for (int i = 0; i < int(input_boxes.size()); ++i)
    {
        vArea[i] = (input_boxes.at(i).x2 - input_boxes.at(i).x1 + 1)
            * (input_boxes.at(i).y2 - input_boxes.at(i).y1 + 1);
    }
    for (int i = 0; i < int(input_boxes.size()); ++i)
    {
        for (int j = i + 1; j < int(input_boxes.size());)
        {
            float xx1 = std::max(input_boxes[i].x1, input_boxes[j].x1);
            float yy1 = std::max(input_boxes[i].y1, input_boxes[j].y1);
            float xx2 = std::min(input_boxes[i].x2, input_boxes[j].x2);
            float yy2 = std::min(input_boxes[i].y2, input_boxes[j].y2);
            float w = std::max(float(0), xx2 - xx1 + 1);
            float h = std::max(float(0), yy2 - yy1 + 1);
            float inter = w * h;
            float ovr = inter / (vArea[i] + vArea[j] - inter);
            if (ovr >= NMS_THRESH)
            {
                input_boxes.erase(input_boxes.begin() + j);
                vArea.erase(vArea.begin() + j);
            }
            else
            {
                j++;
            }
        }
    }
}

void Utils::DrawPred(cv::Mat& img, std::vector<OutputPose>& results,
              const std::vector<std::vector<unsigned int>> &SKELLTON,
              const std::vector<std::vector<unsigned int>> &KPS_COLORS,
              const std::vector<std::vector<unsigned int>> &LIMB_COLORS)
{
    const int num_point =17;
    for (auto &result:results){
        int  left,top,width, height;
        left = result.box.x;
        top = result.box.y;
        width = result.box.width;
        height = result.box.height;

        std::string label = "person:  " + std::to_string(result.confidence).substr(0, 4);
        int baseLine;
        cv::Size labelSize = cv::getTextSize(label, cv::FONT_HERSHEY_SIMPLEX, 0.5, 1, &baseLine);
        top = std::max(top, labelSize.height) -10 ;
        putText(img, label, cv::Point(left, top - 30), cv::FONT_HERSHEY_SIMPLEX, 3, cv::Scalar(0,0,255), 5);
        cv::rectangle(img, cv::Point(left, top), cv::Point(left+width, top+height), cv::Scalar(0, 0, 255), 10);
 
        // 连线
        auto &kps = result.kps;
        for (int k=0; k<num_point+2; k++){// 不要设置为>0.5f ,>0.0f显示效果比较好
            // 关键点绘制
            if (k<num_point){
                int kps_x = std::round(kps[k*3]);
                int kps_y = std::round(kps[k*3 + 1]);
                float kps_s = kps[k*3 + 2];
 
                if (kps_s > 0.0f){
                    cv::Scalar kps_color = cv::Scalar(KPS_COLORS[k][0],KPS_COLORS[k][1],KPS_COLORS[k][2]);
                    cv::circle(img, {kps_x, kps_y}, 5, kps_color, 15);
                }
            }
 
            auto &ske = SKELLTON[k];
            int pos1_x = std::round(kps[(ske[0] -1) * 3]);
            int pos1_y = std::round(kps[(ske[0] -1) * 3 + 1]);
 
            int pos2_x = std::round(kps[(ske[1] -1) * 3]);
            int pos2_y = std::round(kps[(ske[1] -1) * 3 + 1]);
 
            float pos1_s = kps[(ske[0] -1) * 3 + 2];
            float pos2_s = kps[(ske[1] -1) * 3 + 2];
 
            if (pos1_s > 0.0f && pos2_s >0.0f){// 不要设置为>0.5f ,>0.0f显示效果比较好
                cv::Scalar limb_color = cv::Scalar(LIMB_COLORS[k][0], LIMB_COLORS[k][1], LIMB_COLORS[k][3]);
                cv::line(img, {pos1_x, pos1_y}, {pos2_x, pos2_y}, limb_color, 10);
            }
 
        // 跌倒检测
            // float pt5_x = kps[5*3];
            // float pt5_y = kps[5*3 + 1];
            // float pt6_x = kps[6*3];
            // float pt6_y = kps[6*3+1];
            // float center_up_x = (pt5_x + pt6_x) /2.0f ;
            // float center_up_y = (pt5_y + pt6_y) / 2.0f;
            // cv::Point center_up = cv::Point((int)center_up_x, (int)center_up_y);
 
            // float pt11_x = kps[11*3];
            // float pt11_y = kps[11*3 + 1];
            // float pt12_x = kps[12*3];
            // float pt12_y = kps[12*3 + 1];
            // float center_down_x = (pt11_x + pt12_x) / 2.0f;
            // float center_down_y = (pt11_y + pt12_y) / 2.0f;
            // cv::Point center_down = cv::Point((int)center_down_x, (int)center_down_y);
 
 
            // float right_angle_point_x = center_down_x;
            // float righ_angle_point_y = center_up_y;
            // cv::Point right_angl_point = cv::Point((int)right_angle_point_x, (int)righ_angle_point_y);
 
 
            // float a = abs(right_angle_point_x - center_up_x);
            // float b = abs(center_down_y - righ_angle_point_y);
 
            // float tan_value = a / b;
            // float Pi = acos(-1);
            // float angle = atan(tan_value) * 180.0f/ Pi;
            // //string angel_label = "angle: " + to_string(angle);
            // std::string angel_label = "";
            // putText(img, angel_label, cv::Point(left, top-40), cv::FONT_HERSHEY_SIMPLEX, 0.5, cv::Scalar(0,0,255), 2);
 
            // if (angle > 60.0f || center_down_y <= center_up_y || (double)width/ height > 5.0f/3.0f) // 宽高比小于0.6为站立，大于5/3为跌倒
            // {
            //     std::string fall_down_label = "person fall down!!!!";
            //     putText(img, fall_down_label , cv::Point(left, top-20), cv::FONT_HERSHEY_SIMPLEX, 0.5, cv::Scalar(0,0,255), 2);
 
            //     printf("angel:%f width/height:%f\n",angle, (double)width/ height );
            // }
 
 
        }
    }
}

void Utils::DrawPred_video(cv::Mat& img,FrameSize frame_size, std::vector<OutputPose>& results,
              const std::vector<std::vector<unsigned int>> &SKELLTON,
              const std::vector<std::vector<unsigned int>> &KPS_COLORS,
              const std::vector<std::vector<unsigned int>> &LIMB_COLORS)
{
    int osd_width= img.cols;
    int osd_height = img.rows;

    int SENSOR_HEIGHT = frame_size.height;
    int SENSOR_WIDTH = frame_size.width;

    const int num_point =17;
    for (auto &result:results){
        int  left,top,width, height;
        left = result.box.x;
        top = result.box.y;
        width = result.box.width;
        height = result.box.height;

        std::string label = "person:  " + std::to_string(result.confidence).substr(0, 4);
        int baseLine;
        cv::Size labelSize = cv::getTextSize(label, cv::FONT_HERSHEY_SIMPLEX, 0.5, 1, &baseLine);
        top = std::max(top, labelSize.height) -10 ;

        // 连线
        auto &kps = result.kps;
        for (int k=0; k<num_point+2; k++){// 不要设置为>0.5f ,>0.0f显示效果比较好
            // 关键点绘制
            if (k<num_point){
                int kps_x = std::round(kps[k*3]);
                int kps_y = std::round(kps[k*3 + 1]);
                float kps_s = kps[k*3 + 2];
 
                int kps_x1 =  (float)kps_x / SENSOR_WIDTH * osd_width;
                int kps_y1 =  (float)kps_y / SENSOR_HEIGHT  * osd_height;

                if (kps_s > 0.0f){
                    cv::Scalar kps_color = cv::Scalar(255,KPS_COLORS[k][0],KPS_COLORS[k][1],KPS_COLORS[k][2]);
                    cv::circle(img, {kps_x1, kps_y1}, 5, kps_color, -3);
                }
            }
 
            auto &ske = SKELLTON[k];
            int pos1_x = std::round(kps[(ske[0] -1) * 3]);
            int pos1_y = std::round(kps[(ske[0] -1) * 3 + 1]);

            int pos1_x_ =  (float)pos1_x / SENSOR_WIDTH * osd_width;
            int pos1_y_ =  (float)pos1_y / SENSOR_HEIGHT  * osd_height;

            int pos2_x = std::round(kps[(ske[1] -1) * 3]);
            int pos2_y = std::round(kps[(ske[1] -1) * 3 + 1]);

            int pos2_x_ = (float)pos2_x / SENSOR_WIDTH * osd_width;
            int pos2_y_ =  (float)pos2_y / SENSOR_HEIGHT  * osd_height;
 
            float pos1_s = kps[(ske[0] -1) * 3 + 2];
            float pos2_s = kps[(ske[1] -1) * 3 + 2];
 
            if (pos1_s > 0.0f && pos2_s >0.0f){// 不要设置为>0.5f ,>0.0f显示效果比较好
                cv::Scalar limb_color = cv::Scalar(255,LIMB_COLORS[k][0], LIMB_COLORS[k][1], LIMB_COLORS[k][3]);
                cv::line(img, {pos1_x_, pos1_y_}, {pos2_x_, pos2_y_}, limb_color,3);
            }

        // // 跌倒检测
        //     float pt5_x = kps[5*3];
        //     float pt5_y = kps[5*3 + 1];
        //     float pt6_x = kps[6*3];
        //     float pt6_y = kps[6*3+1];
        //     float center_up_x = (pt5_x + pt6_x) /2.0f ;
        //     float center_up_y = (pt5_y + pt6_y) / 2.0f;
        //     cv::Point center_up = cv::Point((int)center_up_x, (int)center_up_y);
 
        //     float pt11_x = kps[11*3];
        //     float pt11_y = kps[11*3 + 1];
        //     float pt12_x = kps[12*3];
        //     float pt12_y = kps[12*3 + 1];
        //     float center_down_x = (pt11_x + pt12_x) / 2.0f;
        //     float center_down_y = (pt11_y + pt12_y) / 2.0f;
        //     cv::Point center_down = cv::Point((int)center_down_x, (int)center_down_y);
 
 
        //     float right_angle_point_x = center_down_x;
        //     float righ_angle_point_y = center_up_y;
        //     cv::Point right_angl_point = cv::Point((int)right_angle_point_x, (int)righ_angle_point_y);
 
 
        //     float a = abs(right_angle_point_x - center_up_x);
        //     float b = abs(center_down_y - righ_angle_point_y);
 
        //     float tan_value = a / b;
        //     float Pi = acos(-1);
        //     float angle = atan(tan_value) * 180.0f/ Pi;
        //     std::string angel_label = "";
 
        //     if (angle > 60.0f || center_down_y <= center_up_y || (double)width/ height > 5.0f/3.0f) // 宽高比小于0.6为站立，大于5/3为跌倒
        //     {
        //         std::string fall_down_label = "person fall down!!!!";
        //     }
 
 
        }

        int plot_x1 = int(left * 1.0 / SENSOR_WIDTH * osd_width);
        int plot_y1 = int((top - 30) * 1.0 / SENSOR_HEIGHT  * osd_height);
        int plot_x2 = int((left+width) * 1.0 / SENSOR_WIDTH * osd_width);
        int plot_y2 = (top + height) > kps[kps.size() - 2] ? int((top+height) * 1.0 / SENSOR_HEIGHT  * osd_height) : int(kps[kps.size() - 2] * 1.0 / SENSOR_HEIGHT  * osd_height);

        // std::cout << top + height << " " << kps[kps.size() - 2] << std::endl;
        // putText(img, label, cv::Point(plot_x1, plot_y1), cv::FONT_HERSHEY_SIMPLEX, 2, cv::Scalar(255,255,0,0), 4);
        cv::rectangle(img, cv::Point(plot_x1, plot_y1), cv::Point(plot_x2, plot_y2+20), cv::Scalar(255, 255, 0, 0), 4);
    }
}

