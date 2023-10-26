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
#include "hand_keypoint.h"

HandKeypoint::HandKeypoint(const char *kmodel_file, const int debug_mode)
:AIBase(kmodel_file, "HandKeypoint", debug_mode)
{
    model_name_ = "HandKeypoint";
    ai2d_out_tensor_ = get_input_tensor(0);
}

HandKeypoint::HandKeypoint(const char *kmodel_file, FrameCHWSize isp_shape, uintptr_t vaddr, uintptr_t paddr, const int debug_mode)
: AIBase(kmodel_file,"HandKeypoint", debug_mode)
{
    model_name_ = "HandKeypoint";
    vaddr_ = vaddr;
    isp_shape_ = isp_shape;
    dims_t in_shape{1, isp_shape.channel, isp_shape.height, isp_shape.width};
    int isp_size = isp_shape.channel * isp_shape.height * isp_shape.width;
#if 0
    int in_size = isp_shape.channel * isp_shape.height * isp_shape.width;
    ai2d_in_tensor_ = host_runtime_tensor::create(typecode_t::dt_uint8, in_shape, { (gsl::byte *)vaddr, in_size },
        true, hrt::pool_shared).expect("cannot create input tensor");
#else
    ai2d_in_tensor_ = hrt::create(typecode_t::dt_uint8, in_shape, hrt::pool_shared).expect("create ai2d input tensor failed");
#endif
    ai2d_out_tensor_ = get_input_tensor(0);
}

HandKeypoint::~HandKeypoint()
{
}

void HandKeypoint::pre_process(cv::Mat ori_img, Bbox &bbox)
{
    ScopedTiming st(model_name_ + " pre_process image", debug_mode_);
    std::vector<uint8_t> chw_vec;
    Utils::hwc_to_chw(ori_img, chw_vec);
    Utils::crop_resize({ori_img.channels(), ori_img.rows, ori_img.cols}, chw_vec, bbox, ai2d_out_tensor_);
}

// for video
void HandKeypoint::pre_process(Bbox &bbox)
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

void HandKeypoint::inference()
{
    this->run();
    this->get_output();
}

void HandKeypoint::post_process(Bbox &bbox)
{
    ScopedTiming st(model_name_ + " post_process", debug_mode_);
    float *pred = p_outputs_[0];
    // 绘制关键点像素坐标
    int64_t output_tensor_size = output_shapes_[0][1];// 关键点输出 （x,y）*21= 42
    results.clear();

    for (unsigned i = 0; i < output_tensor_size / 2; i++)
    {
        float x_kp;
        float y_kp;
        x_kp = pred[i * 2] * bbox.w + bbox.x;
        y_kp = pred[i * 2 + 1] * bbox.h + bbox.y;

        results.push_back(static_cast<int>(x_kp));
        results.push_back(static_cast<int>(y_kp));

    }
}

double HandKeypoint::vector_2d_angle(std::vector<double> v1, std::vector<double> v2)
{
    double v1_x = v1[0];
    double v1_y = v1[1];
    double v2_x = v2[0];
    double v2_y = v2[1];
    double v1_norm = std::sqrt(v1_x * v1_x + v1_y * v1_y);
    double v2_norm = std::sqrt(v2_x * v2_x + v2_y * v2_y);
    double dot_product = v1_x * v2_x + v1_y * v2_y;
    double cos_angle = dot_product / (v1_norm * v2_norm);
    double angle = std::acos(cos_angle) * 180 / M_PI;
    if (angle > 180.0)
    {
        return 65535.0;
    }
    return angle;
}

std::vector<double> HandKeypoint::hand_angle()
{
    double angle_;
    std::vector<double> angle_list;
    //---------------------------- thumb 大拇指角度
    angle_ = vector_2d_angle(
        {(results[0] - results[4]), (results[1] - results[5])},
        {(results[6] - results[8]), (results[7] - results[9])}
    );
    angle_list.push_back(angle_);
    //---------------------------- index 食指角度
    angle_ = vector_2d_angle(
        {(results[0] - results[12]), (results[1] - results[13])},
        {(results[14] - results[16]), (results[15] - results[17])}
    );
    angle_list.push_back(angle_);
    //---------------------------- middle 中指角度
    angle_ = vector_2d_angle(
        {(results[0] - results[20]), (results[1] - results[21])},
        {(results[22] - results[24]), (results[23] - results[25])}
    );
    angle_list.push_back(angle_);
    //---------------------------- ring 无名指角度
    angle_ = vector_2d_angle(
        {(results[0] - results[28]), (results[1] - results[29])},
        {(results[30] - results[32]), (results[31] - results[33])}
    );
    angle_list.push_back(angle_);
    //---------------------------- pink 小拇指角度
    angle_ = vector_2d_angle(
        {(results[0] - results[36]), (results[1] - results[37])},
        {(results[38] - results[40]), (results[39] - results[41])}
    );
    angle_list.push_back(angle_);
    return angle_list;
}

std::string HandKeypoint::h_gesture(std::vector<double> angle_list)
{
    int thr_angle = 65;
    int thr_angle_thumb = 53;
    int thr_angle_s = 49;
    std::string gesture_str="other";

    bool present = std::find(angle_list.begin(),angle_list.end(),65535) != angle_list.end();
    if (present)
    {
        std::cout<<"gesture_str:"<<gesture_str<<std::endl;
    }else{
        if (angle_list[0]>thr_angle_thumb && angle_list[1]>thr_angle && angle_list[2]>thr_angle && (angle_list[3]>thr_angle) && (angle_list[4]>thr_angle))
            {gesture_str = "fist";}
        else if ((angle_list[1]<thr_angle_s) && (angle_list[2]<thr_angle_s) && (angle_list[3]<thr_angle_s) && (angle_list[4]<thr_angle_s))
            {gesture_str = "five";}
        else if ((angle_list[0]<thr_angle_s)  && (angle_list[1]<thr_angle_s) && (angle_list[2]>thr_angle) && (angle_list[3]>thr_angle) && (angle_list[4]>thr_angle))
            {gesture_str = "gun";}
        else if ((angle_list[0]<thr_angle_s)  && (angle_list[1]<thr_angle_s) && (angle_list[2]>thr_angle) && (angle_list[3]>thr_angle) && (angle_list[4]<thr_angle_s))
            {gesture_str = "love";}
        else if ((angle_list[0]>5)  && (angle_list[1]<thr_angle_s) && (angle_list[2]>thr_angle) && (angle_list[3]>thr_angle) && (angle_list[4]>thr_angle))
            {gesture_str = "one";}
        else if ((angle_list[0]<thr_angle_s)  && (angle_list[1]>thr_angle) && (angle_list[2]>thr_angle) && (angle_list[3]>thr_angle) && (angle_list[4]<thr_angle_s))
            {gesture_str = "six";}
        else if ((angle_list[0]>thr_angle_thumb)  && (angle_list[1]<thr_angle_s) && (angle_list[2]<thr_angle_s) && (angle_list[3]<thr_angle_s) && (angle_list[4]>thr_angle))
            {gesture_str = "three";}
        else if ((angle_list[0]<thr_angle_s)  && (angle_list[1]>thr_angle) && (angle_list[2]>thr_angle) && (angle_list[3]>thr_angle) && (angle_list[4]>thr_angle))
            {gesture_str = "thumbUp";}
        else if ((angle_list[0]>thr_angle_thumb)  && (angle_list[1]<thr_angle_s) && (angle_list[2]<thr_angle_s) && (angle_list[3]>thr_angle) && (angle_list[4]>thr_angle))
            {gesture_str = "yeah";}
    }
    return gesture_str;
}
