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

#include "pulc.h"
#include <vector>
#include "utils.h"


// for image
Pulc::Pulc(const char *kmodel_file,float color_thresh, float type_thresh, const int debug_mode)
:color_thresh_(color_thresh),type_thresh_(type_thresh), AIBase(kmodel_file,"vehicleAttr",debug_mode)
{
    model_name_ = "vehicleAttr";
    ai2d_out_tensor_ = get_input_tensor(0);

}

// for video
Pulc::Pulc(const char *kmodel_file, FrameCHWSize isp_shape,float color_thresh, float type_thresh,  uintptr_t vaddr, uintptr_t paddr, const int debug_mode)
: color_thresh_(color_thresh),type_thresh_(type_thresh),AIBase(kmodel_file,"personAttr", debug_mode)
{

    model_name_ = "vehicleAttr";
    
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
    ai2d_out_tensor_ = get_input_tensor(0);

}

// ai2d for image
void Pulc::pre_process(cv::Mat ori_img, BoxInfo &bbox)
{
    ScopedTiming st(model_name_ + " pre_process image", debug_mode_);
    // get_affine_matrix(bbox);

    std::vector<uint8_t> chw_vec;
    cv::Mat cropped_image = ori_img(cv::Rect((int)(bbox.x1) ,(int)(bbox.y1),(int)(bbox.x2 - bbox.x1),(int)(bbox.y2 - bbox.y1)));

    // add
    cropped_image = Utils::bgr_to_rgb(cropped_image);

    Utils::hwc_to_chw(cropped_image, chw_vec);
    Utils::padding_resize({cropped_image.channels(), cropped_image.rows, cropped_image.cols}, chw_vec, {input_shapes_[0][3], input_shapes_[0][2]}, ai2d_out_tensor_, cv::Scalar(114, 114, 114));


}

// for video
void Pulc::pre_process(Bbox &bbox)
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

void Pulc::inference()
{
    this->run();
    this->get_output();
}

string Pulc::post_process()
{
    output_0 = p_outputs_[0];

    string result = "";
    string color = "Color: " + (string)(output_0[0] > color_thresh_ ? "Yellow" : (output_0[1] > color_thresh_ ? "orange" : (output_0[2] > color_thresh_ ? "green" : (output_0[3] > color_thresh_ ? "gray" : 
                    (output_0[4] > color_thresh_ ? "read" : (output_0[5] > color_thresh_ ? "blue" : (output_0[6] > color_thresh_ ? "white" : (output_0[7] > color_thresh_ ? "golden": 
                    (output_0[8] > color_thresh_ ? "brown" : (output_0[9] > color_thresh_ ? "black" : "Color unknown"))))))))));
    

    string type = "Type: " + (string)(output_0[10] > type_thresh_ ? "sedan" : (output_0[11] > type_thresh_ ? "suv" : (output_0[12] > type_thresh_ ? "van" : (output_0[13] > type_thresh_ ? "hatchback" : 
                    (output_0[14] > type_thresh_ ? "mpv" : (output_0[15] > type_thresh_ ? "pickup" : (output_0[16] > type_thresh_ ? "bus" : (output_0[17] > type_thresh_ ? "truck" :
                    (output_0[18] > type_thresh_ ? "estate" : "Type unknown")))))))));;

    result = color + "   " + type;
    return result;
}

string Pulc::GetColor()
{
    output_0 = p_outputs_[0];

    return "Color: " + (string)(output_0[0] > color_thresh_ ? "yellow" : (output_0[1] > color_thresh_ ? "orange" : (output_0[2] > color_thresh_ ? "green" : (output_0[3] > color_thresh_ ? "gray" : 
                    (output_0[4] > color_thresh_ ? "red" : (output_0[5] > color_thresh_ ? "blue" : (output_0[6] > color_thresh_ ? "white" : (output_0[7] > color_thresh_ ? "golden": 
                    (output_0[8] > color_thresh_ ? "brown" : (output_0[9] > color_thresh_ ? "black" : "Color unknown"))))))))));
}

string Pulc::GetType()
{
    output_0 = p_outputs_[0];
    return "Type: " + (string)(output_0[10] > type_thresh_ ? "sedan" : (output_0[11] > type_thresh_ ? "suv" : (output_0[12] > type_thresh_ ? "van" : (output_0[13] > type_thresh_ ? "hatchback" : 
                    (output_0[14] > type_thresh_ ? "mpv" : (output_0[15] > type_thresh_ ? "pickup" : (output_0[16] > type_thresh_ ? "bus" : (output_0[17] > type_thresh_ ? "truck" :
                    (output_0[18] > type_thresh_ ? "estate" : "Type unknown")))))))));;
}


Pulc::~Pulc()
{

}

void Pulc::get_affine_matrix(Bbox &bbox)
{
    int w = bbox.w;
    int h = bbox.h;
    float scale_ratio = float(input_shapes_[0][2]) / (std::max(w, h) * 1.5); // input_shapes_[0][2]==input_shapes_[0][3]
    float cx = (bbox.x + w / 2) * scale_ratio;
    float cy = (bbox.y + h / 2) * scale_ratio;
    int half_net_len = input_shapes_[0][2] / 2; // input_shapes_[0][2]==input_shapes_[0][3]
    // # [scale,0,96-333.42172012893894],[0,scale,96-141.95230854588053],[0,0,1]
    matrix_dst_ = cv::Mat::zeros(2, 3, CV_32FC1);
    matrix_dst_.at<float>(0, 0) = scale_ratio;
    matrix_dst_.at<float>(0, 1) = 0;
    matrix_dst_.at<float>(0, 2) = half_net_len - cx;
    matrix_dst_.at<float>(1, 0) = 0;
    matrix_dst_.at<float>(1, 1) = scale_ratio;
    matrix_dst_.at<float>(1, 2) = half_net_len - cy;
}
