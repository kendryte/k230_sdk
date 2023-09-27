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
#include "face_dense_landmark.h"
#include <vector>

std::vector<std::vector<int>> dict_kp_seq = {
    {43, 44, 45, 47, 46, 50, 51, 49, 48},                                                                                        // left_eyebrow
    {97, 98, 99, 100, 101, 105, 104, 103, 102},                                                                                  // right_eyebrow
    {35, 36, 33, 37, 39, 42, 40, 41},                                                                                            // left_eye
    {89, 90, 87, 91, 93, 96, 94, 95},                                                                                            // right_eye
    {34, 88},                                                                                                                    // pupil
    {72, 73, 74, 86},                                                                                                            // bridge_nose
    {77, 78, 79, 80, 85, 84, 83},                                                                                                // wing_nose
    {52, 55, 56, 53, 59, 58, 61, 68, 67, 71, 63, 64},                                                                            // out_lip
    {65, 54, 60, 57, 69, 70, 62, 66},                                                                                            // in_lip
    {1, 9, 10, 11, 12, 13, 14, 15, 16, 2, 3, 4, 5, 6, 7, 8, 0, 24, 23, 22, 21, 20, 19, 18, 32, 31, 30, 29, 28, 27, 26, 25, 17}}; // basin

cv::Scalar color_list_for_kp[] = {
    cv::Scalar(0, 255, 0),
    cv::Scalar(0, 255, 0),
    cv::Scalar(255, 0, 255),
    cv::Scalar(255, 0, 255),
    cv::Scalar(0, 0, 255),
    cv::Scalar(0, 170, 255),
    cv::Scalar(0, 255, 255),
    cv::Scalar(255, 255, 0),
    cv::Scalar(50, 220, 255),
    cv::Scalar(255, 30, 30)};

cv::Scalar color_list_for_osd_kp[] = {
    cv::Scalar(255, 0, 255, 0),
    cv::Scalar(255, 0, 255, 0),
    cv::Scalar(255, 255, 0, 255),
    cv::Scalar(255, 255, 0, 255),
    cv::Scalar(255, 255, 0, 0),
    cv::Scalar(255, 255, 170, 0),
    cv::Scalar(255, 255, 255, 0),
    cv::Scalar(255, 0, 255, 255),
    cv::Scalar(255, 255, 220, 50),
    cv::Scalar(255, 30, 30, 255)};

FaceDenseLandmark::FaceDenseLandmark(const char *kmodel_file, const int debug_mode) : AIBase(kmodel_file,"FaceDenseLandmark",debug_mode)
{
    model_name_ = "FaceDenseLandmark";
    ai2d_out_tensor_ = get_input_tensor(0);
}

FaceDenseLandmark::FaceDenseLandmark(const char *kmodel_file, FrameCHWSize isp_shape, uintptr_t vaddr, uintptr_t paddr, const int debug_mode) : AIBase(kmodel_file,"FaceDenseLandmark", debug_mode)
{
    model_name_ = "FaceDenseLandmark";
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

FaceDenseLandmark::~FaceDenseLandmark()
{
}

// ai2d for image
void FaceDenseLandmark::pre_process(cv::Mat ori_img, Bbox &bbox)
{
    ScopedTiming st(model_name_ + " pre_process image", debug_mode_);
    get_affine_matrix(bbox);

    std::vector<uint8_t> chw_vec;
    Utils::bgr2rgb_and_hwc2chw(ori_img, chw_vec);

    float *matrix_dst_ptr = matrix_dst_.ptr<float>();
    Utils::affine({ori_img.channels(), ori_img.rows, ori_img.cols}, chw_vec, matrix_dst_ptr, ai2d_out_tensor_);
    
    // auto vaddr_out_buf = ai2d_out_tensor_.impl()->to_host().unwrap()->buffer().as_host().unwrap().map(map_access_::map_read).unwrap().buffer();
    // unsigned char *output = reinterpret_cast<unsigned char *>(vaddr_out_buf.data());
    // Utils::dump_color_image("FaceDenseLandmark_input_color.png", {input_shapes_[0][3],input_shapes_[0][2]},output);
}

// ai2d for video
void FaceDenseLandmark::pre_process(Bbox &bbox)
{
    ScopedTiming st(model_name_ + " pre_process_video", debug_mode_);
    get_affine_matrix(bbox);
    float *matrix_dst_ptr = matrix_dst_.ptr<float>();
#if 1
    size_t isp_size = isp_shape_.channel * isp_shape_.height * isp_shape_.width;
    auto buf = ai2d_in_tensor_.impl()->to_host().unwrap()->buffer().as_host().unwrap().map(map_access_::map_write).unwrap().buffer();
    memcpy(reinterpret_cast<char *>(buf.data()), (void *)vaddr_, isp_size);
    hrt::sync(ai2d_in_tensor_, sync_op_t::sync_write_back, true).expect("sync write_back failed");
#endif
    Utils::affine(matrix_dst_ptr, ai2d_builder_, ai2d_in_tensor_, ai2d_out_tensor_);

    // auto vaddr_out_buf = ai2d_out_tensor_.impl()->to_host().unwrap()->buffer().as_host().unwrap().map(map_access_::map_read).unwrap().buffer();
    // unsigned char *output = reinterpret_cast<unsigned char *>(vaddr_out_buf.data());
    // Utils::dump_gray_image("FaceDenseLandmark_input.png",{input_shapes_[0][3],input_shapes_[0][2]},output);
   
}

void FaceDenseLandmark::inference()
{
    this->run();
    this->get_output();
}

void FaceDenseLandmark::post_process()
{
    ScopedTiming st(model_name_ + " post_process", debug_mode_);
    float *pred = p_outputs_[0];

    int half_net_len = input_shapes_[0][2] / 2; // input_shapes_[0][2]==input_shapes_[0][3]
    for (int i = 0; i < output_shapes_[0][1]; ++i)
        pred[i] += (pred[i] + 1) * half_net_len;
    // IM = cv2.invertAffineTransform(self.M)
    cv::Mat matrix_dst_inv;
    cv::invertAffineTransform(matrix_dst_, matrix_dst_inv);
    float *ptr = matrix_dst_inv.ptr<float>();
    int half_out_len = output_shapes_[0][1] / 2; // 212
    for (int kp_id = 0; kp_id < half_out_len; ++kp_id)
    {
        float old_x = pred[kp_id * 2];
        float old_y = pred[kp_id * 2 + 1];
        pred[kp_id * 2] = old_x * ptr[0] + old_y * ptr[1] + 1 * ptr[2];
        pred[kp_id * 2 + 1] = old_x * ptr[3] + old_y * ptr[4] + ptr[5];
    }
}

void FaceDenseLandmark::draw_contour(cv::Mat src_img, bool pic_mode)
{
    ScopedTiming st(model_name_ + " draw_contour", debug_mode_);
    // 定义多边形的顶点坐标
    float *pred = p_outputs_[0];
    int src_width = src_img.cols, src_height = src_img.rows;
    for (int sub_part_index = 0; sub_part_index < dict_kp_seq.size(); ++sub_part_index)
    {
        auto &sub_part = dict_kp_seq[sub_part_index];
        std::vector<std::vector<cv::Point>> face_sub_part_point_set_outline;
        std::vector<cv::Point> face_sub_part_point_set;
        for (int kp_index = 0; kp_index < sub_part.size(); ++kp_index)
        {
            int real_kp_index = sub_part[kp_index];
            float x, y;
            if (pic_mode)
            {
                x = pred[real_kp_index * 2];
                y = pred[real_kp_index * 2 + 1];
            }
            else
            {
                x = pred[real_kp_index * 2] / isp_shape_.width*src_width;
                y = pred[real_kp_index * 2 + 1] / isp_shape_.height*src_height;
            }
            face_sub_part_point_set.push_back({int(x), int(y)});
        }
        face_sub_part_point_set_outline.push_back(face_sub_part_point_set);
        if (sub_part_index == 9 || sub_part_index == 6)
        {
            if (pic_mode)
                cv::polylines(src_img, face_sub_part_point_set_outline, false, color_list_for_kp[sub_part_index], 3);
            else
                cv::polylines(src_img, face_sub_part_point_set_outline, false, color_list_for_osd_kp[sub_part_index], 3);
        }
        else if (sub_part_index == 4)
        {
            for (const auto &kp : face_sub_part_point_set)
            {
                if (pic_mode)
                    cv::circle(src_img, kp, 2, color_list_for_kp[sub_part_index], 3);
                else
                    cv::circle(src_img, kp, 2, color_list_for_osd_kp[sub_part_index], 3);
            }
        }
        else
        {
            if (pic_mode)
                cv::drawContours(src_img, face_sub_part_point_set_outline, -1, color_list_for_kp[sub_part_index], 3);
            else
                cv::drawContours(src_img, face_sub_part_point_set_outline, -1, color_list_for_osd_kp[sub_part_index], 3);
        }
    }
}

void FaceDenseLandmark::get_affine_matrix(Bbox &bbox)
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
