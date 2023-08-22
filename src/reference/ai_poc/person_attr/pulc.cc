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

// for image
Pulc::Pulc(const char *kmodel_file,float pulc_thresh, float glasses_thresh, float hold_thresh, const int debug_mode)
:pulc_thresh_(pulc_thresh),glasses_thresh_(glasses_thresh),hold_thresh_(hold_thresh), AIBase(kmodel_file,"personAttr",debug_mode)
{
    model_name_ = "personAttr";
    ai2d_out_tensor_ = get_input_tensor(0);

}

// for video
Pulc::Pulc(const char *kmodel_file, FrameCHWSize isp_shape,float pulc_thresh, float glasses_thresh, float hold_thresh, uintptr_t vaddr, uintptr_t paddr, const int debug_mode)
: pulc_thresh_(pulc_thresh),glasses_thresh_(glasses_thresh),hold_thresh_(hold_thresh),AIBase(kmodel_file,"personAttr", debug_mode)
{

    model_name_ = "personAttr";
    
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

    std::vector<uint8_t> chw_vec;
    cv::Mat cropped_image = ori_img(cv::Rect((int)(bbox.x1) ,(int)(bbox.y1),(int)(bbox.x2 - bbox.x1),(int)(bbox.y2 - bbox.y1)));
    // cv::imwrite("cropped_image.jpg", cropped_image);

    // add
    cropped_image = Utils::bgr_to_rgb(cropped_image);

    Utils::hwc_to_chw(cropped_image, chw_vec);
    Utils::padding_resize({cropped_image.channels(), cropped_image.rows, cropped_image.cols}, chw_vec, {input_shapes_[0][3], input_shapes_[0][2]}, ai2d_out_tensor_, cv::Scalar(114, 114, 114));

    // auto vaddr_out_buf = ai2d_out_tensor_.impl()->to_host().unwrap()->buffer().as_host().unwrap().map(map_access_::map_read).unwrap().buffer();
    // unsigned char *output0 = reinterpret_cast<unsigned char *>(vaddr_out_buf.data());
    // Utils::dump_color_image("color_image.jpg",{input_shapes_[0][3],input_shapes_[0][2]},output0);

}


// ai2d for video
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

    // auto vaddr_out_buf = ai2d_out_tensor_.impl()->to_host().unwrap()->buffer().as_host().unwrap().map(map_access_::map_read).unwrap().buffer();
    // unsigned char *output0 = reinterpret_cast<unsigned char *>(vaddr_out_buf.data());
    // Utils::dump_color_image("color_video.jpg",{input_shapes_[0][3],input_shapes_[0][2]},output0);

}


void Pulc::inference()
{
    this->run();
    this->get_output();
}

string Pulc::post_process()
{

    ScopedTiming st(model_name_ + " post_process", debug_mode_);
    output_0 = p_outputs_[0];

    string result = "";
    string gender = (string)(output_0[22] < pulc_thresh_ ? "Female" : "Male") + "\n";
    string age = (string)(output_0[19] > output_0[20] ? "AgeLess18" : (output_0[20] > output_0[21] ? "Age18-60" : "AgeOver60")) + "\n";
    string direction = (string)(output_0[23] > output_0[24] ? "Front" : (output_0[24] > output_0[25] ? "Side" : "Back")) + "\n";
    string glasses = (string)"Glasses: " + (output_0[1] > glasses_thresh_ ? "True" : "False") + "\n";
    string hat = "Hat: " + (string)(output_0[0] > pulc_thresh_ ? "True" : "False") + "\n";
    string hold_obj = "HoldObjectsInFront: " + (string)(output_0[18] > hold_thresh_ ? "True" : "False") + "\n";
    string bag = (string)(output_0[15] > output_0[16] ? (output_0[15] > pulc_thresh_ ? "HandBag" : "No bag") : 
                (output_0[16] > output_0[17] ? (output_0[17] > pulc_thresh_ ? "ShoulderBag" : "No bag") :
                (output_0[17] > pulc_thresh_ ? "Backpack" : "No bag"))) + "\n";
    string upper = "Upper: " + (string)(output_0[3] > output_0[2] ? "LongSleeve " : "ShortSleeve ") + 
                    (output_0[4] > output_0[5] ? "UpperStride" : (output_0[5] > output_0[6] ? "UpperLogo" :
                    (output_0[6] > output_0[7] ? "UpperPlaid" : "UpperSplice"))) + "\n";
    string lower = "Lower: " + (string)(output_0[8] > pulc_thresh_ ? "LowerStripe" : (output_0[9] > pulc_thresh_ ? "LowerPattern" :
                               (output_0[10] > pulc_thresh_ ? "LongCoat" : (output_0[11] > pulc_thresh_ ? "Trousers" : 
                               (output_0[12] > pulc_thresh_ ? "Shorts" : "Skirt&Dress"))))) + "\n";
    string shoe = (string)(output_0[14] > pulc_thresh_ ? "Boots" : "No boots") + "\n";
    result = gender + age + direction + glasses + hat + hold_obj + bag + upper + lower + shoe;
    return result;
}

string Pulc::GetGender()
{
    return (string)(output_0[22] > pulc_thresh_ ? "Female" : "Male");
}

string Pulc::GetAge()
{
    return (string)(output_0[19] > output_0[20] ? "AgeLess18" : (output_0[20] > output_0[21] ? "Age18-60" : "AgeOver60"));
}

string Pulc::GetDirection()
{
    return (string)(output_0[23] > output_0[24] ? "Front" : (output_0[24] > output_0[25] ? "Side" : "Back"));
}

string Pulc::GetGlasses()
{
    return (string)"Glasses: " + (output_0[1] > glasses_thresh_ ? "True" : "False");
}

string Pulc::GetHat()
{
    return "Hat: " + (string)(output_0[0] > pulc_thresh_ ? "True" : "False");
}

string Pulc::GetHoldObj()
{
    return "HoldObjectsInFront: " + (string)(output_0[18] > hold_thresh_ ? "True" : "False");
}

string Pulc::GetBag()
{
    return (string)(output_0[15] > output_0[16] ? (output_0[15] > pulc_thresh_ ? "HandBag" : "No bag") : 
                (output_0[16] > output_0[17] ? (output_0[17] > pulc_thresh_ ? "ShoulderBag" : "No bag") :
                (output_0[17] > pulc_thresh_ ? "Backpack" : "No bag")));
}

string Pulc::GetUpper()
{
    return "Upper: " + (string)(output_0[3] > output_0[2] ? "LongSleeve " : "ShortSleeve ") + 
                    (output_0[4] > output_0[5] ? "UpperStride" : (output_0[5] > output_0[6] ? "UpperLogo" :
                    (output_0[6] > output_0[7] ? "UpperPlaid" : "UpperSplice")));
}

string Pulc::GetLower()
{
    return "Lower: " + (string)(output_0[8] > pulc_thresh_ ? "LowerStripe" : (output_0[9] > pulc_thresh_ ? "LowerPattern" :
                               (output_0[10] > pulc_thresh_ ? "LongCoat" : (output_0[11] > pulc_thresh_ ? "Trousers" : 
                               (output_0[12] > pulc_thresh_ ? "Shorts" : "Skirt&Dress")))));
}

string Pulc::GetShoe()
{
    return (string)(output_0[14] > pulc_thresh_ ? "Boots" : "No boots");
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
