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
#include "face_parse.h"
#include <vector>

cv::Vec3b color_list_for_pixel[] = {
    cv::Vec3b(220, 20, 60),
    cv::Vec3b(0, 0, 142),
    cv::Vec3b(119, 11, 32),
    cv::Vec3b(0, 0, 230),
    cv::Vec3b(106, 0, 228),
    cv::Vec3b(0, 60, 100),
    cv::Vec3b(0, 80, 100),
    cv::Vec3b(0, 0, 70),
    cv::Vec3b(0, 0, 192),
    cv::Vec3b(250, 170, 30),
	cv::Vec3b(100, 170, 30),
	cv::Vec3b(220, 220, 0),
	cv::Vec3b(175, 116, 175),
	cv::Vec3b(250, 0, 30),
	cv::Vec3b(165, 42, 42),
	cv::Vec3b(255, 77, 255),
	cv::Vec3b(0, 226, 252),
	cv::Vec3b(182, 182, 255),
	cv::Vec3b(0, 82, 0)
	};

cv::Vec4b color_list_for_osd_pixel[] = {
    cv::Vec4b(255,220, 20, 60),
    cv::Vec4b(255,0, 0, 142),
    cv::Vec4b(255,119, 11, 32),
    cv::Vec4b(255,0, 0, 230),
    cv::Vec4b(255,106, 0, 228),
    cv::Vec4b(255,0, 60, 100),
    cv::Vec4b(255,0, 80, 100),
    cv::Vec4b(255,0, 0, 70),
    cv::Vec4b(255,0, 0, 192),
    cv::Vec4b(255,250, 170, 30),
	cv::Vec4b(255,100, 170, 30),
	cv::Vec4b(255,220, 220, 0),
	cv::Vec4b(255,175, 116, 175),
	cv::Vec4b(255,250, 0, 30),
	cv::Vec4b(255,165, 42, 42),
	cv::Vec4b(255,255, 77, 255),
	cv::Vec4b(255,0, 226, 252),
	cv::Vec4b(255,182, 182, 255),
	cv::Vec4b(255,0, 82, 0)
};

FaceParse::FaceParse(const char *kmodel_file, const int debug_mode) : AIBase(kmodel_file,"FaceParse",debug_mode)
{
    model_name_ = "FaceParse";
    ai2d_out_tensor_ = get_input_tensor(0);
}

FaceParse::FaceParse(const char *kmodel_file, FrameCHWSize isp_shape, uintptr_t vaddr, uintptr_t paddr, const int debug_mode) : AIBase(kmodel_file,"FaceParse", debug_mode)
{
    model_name_ = "FaceParse";
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

FaceParse::~FaceParse()
{
}

// ai2d for image
void FaceParse::pre_process(cv::Mat ori_img, Bbox &bbox)
{
    ScopedTiming st(model_name_ + " pre_process image", debug_mode_);
    get_affine_matrix(bbox);

    std::vector<uint8_t> chw_vec;
    Utils::bgr2rgb_and_hwc2chw(ori_img, chw_vec);

	float *matrix_dst_ptr = matrix_dst_.ptr<float>();
    Utils::affine({ori_img.channels(), ori_img.rows, ori_img.cols}, chw_vec, matrix_dst_ptr, ai2d_out_tensor_);
    
    // auto vaddr_out_buf = ai2d_out_tensor_.impl()->to_host().unwrap()->buffer().as_host().unwrap().map(map_access_::map_read).unwrap().buffer();
    // unsigned char *output = reinterpret_cast<unsigned char *>(vaddr_out_buf.data());
    // Utils::dump_gray_image("FaceParse_input_gray.png",{input_shapes_[0][3],input_shapes_[0][2]},output);
}

// ai2d for video
void FaceParse::pre_process(Bbox& bbox)
{
    ScopedTiming st(model_name_ + " pre_process_video", debug_mode_);
    get_affine_matrix(bbox);

#if 1
    size_t isp_size = isp_shape_.channel * isp_shape_.height * isp_shape_.width;
    auto buf = ai2d_in_tensor_.impl()->to_host().unwrap()->buffer().as_host().unwrap().map(map_access_::map_write).unwrap().buffer();
    memcpy(reinterpret_cast<char *>(buf.data()), (void *)vaddr_, isp_size);
    hrt::sync(ai2d_in_tensor_, sync_op_t::sync_write_back, true).expect("sync write_back failed");
    // run ai2d
#endif
	float *matrix_dst_ptr = matrix_dst_.ptr<float>();
    Utils::affine(matrix_dst_ptr, ai2d_builder_, ai2d_in_tensor_, ai2d_out_tensor_);

    // auto vaddr_out_buf = ai2d_out_tensor_.impl()->to_host().unwrap()->buffer().as_host().unwrap().map(map_access_::map_read).unwrap().buffer();
    // unsigned char *output = reinterpret_cast<unsigned char *>(vaddr_out_buf.data());
    // Utils::dump_gray_image("FaceParse_input.png",{input_shapes_[0][3],input_shapes_[0][2]},output);
}

void FaceParse::inference()
{
    this->run();
    this->get_output();
}

void FaceParse::post_process(cv::Mat& src_img,Bbox& bbox, bool pic_mode)
{
    ScopedTiming st(model_name_ + " post_process", debug_mode_);

	cv::Mat matrix_dst_inv;
    cv::invertAffineTransform(matrix_dst_, matrix_dst_inv);
    float *ptr = matrix_dst_inv.ptr<float>();
	if(pic_mode)
	{
		cv::Mat images_pred_color = cv::Mat::ones(src_img.rows, src_img.cols, CV_8UC3);
		for (int y = 0; y < output_shapes_[0][1]; ++y)
		{
			for (int x = 0; x < output_shapes_[0][2]; ++x)
			{
				float *pred = p_outputs_[0]+ (y*output_shapes_[0][2]+x)*output_shapes_[0][3];
				int max_index = std::max_element(pred,pred+output_shapes_[0][3]) - pred;
				if(max_index!=0)
				{
					int new_x = x * ptr[0] + y * ptr[1] + 1 * ptr[2];
					int new_y = x * ptr[3] + y * ptr[4] + ptr[5];
					cv::Vec3b& color = images_pred_color.at<cv::Vec3b>(cv::Point(new_x, new_y));
					color = color_list_for_pixel[max_index];
				}
			}
		}
		cv::addWeighted(src_img,0.8,images_pred_color,0.5,0.9,src_img);
	}
	else
	{
		int src_w = src_img.cols;
    	int src_h = src_img.rows;	
		
		//1. get final affine matrix
		//1.1 get affine matrix for osd shape
		bbox.x = float(bbox.x)/isp_shape_.width*src_w;
		bbox.y = float(bbox.y)/isp_shape_.height*src_h;
		bbox.w = float(bbox.w)/isp_shape_.width*src_w;
		bbox.h = float(bbox.h)/isp_shape_.height*src_h;
		get_affine_matrix_for_osd(bbox);
		
		//1.2 get invert affine matrix
		cv::Mat matrix_dst_inv_osd;
		cv::invert(matrix_dst_for_osd_,matrix_dst_inv_osd);
				
		//2.get net_image for output_shapes
		cv::Mat net_image(output_shapes_[0][1], output_shapes_[0][2], CV_8UC4, cv::Scalar(0, 0, 0, 0));
		for (int y = 0; y < output_shapes_[0][1]; ++y)
		{
			for (int x = 0; x < output_shapes_[0][2]; ++x)
			{
				float *pred = p_outputs_[0]+ (y*output_shapes_[0][2]+x)*output_shapes_[0][3];
				int max_index = std::max_element(pred,pred+output_shapes_[0][3]) - pred;
				if(max_index!=0)
					net_image.at<cv::Vec4b>(cv::Point(x, y))=color_list_for_osd_pixel[max_index];	
			}
		}

		//3.affine to osd shape
		cv::Mat matrix_for_warp = matrix_dst_inv_osd(cv::Rect(0,0,3,2));
		cv::Mat mask(src_h, src_w, CV_8UC4, cv::Scalar(0, 0, 0, 0));
		cv::warpAffine(net_image, mask, matrix_for_warp, cv::Size(src_w,src_h));
		src_img = src_img + mask;
	}
}

void FaceParse::get_affine_matrix(Bbox& bbox)
{
    float factor = 2.7;
    float edge_size = input_shapes_[0][2];
	float trans_distance = edge_size / 2.0;
	float height = bbox.h;
	float width = bbox.w;
	float center_x = bbox.x + bbox.w / 2.0 ;
	float center_y = bbox.y + bbox.h / 2.0 ;
	float maximum_edge = factor * (height > width ? height : width);
	float scale = edge_size * 2.0 / maximum_edge;
	float cx = trans_distance - scale * center_x;
	float cy = trans_distance - scale * center_y;
	matrix_dst_ = cv::Mat::zeros(2, 3, CV_32FC1);
    matrix_dst_.at<float>(0, 0) = scale;
    matrix_dst_.at<float>(0, 1) = 0;
    matrix_dst_.at<float>(0, 2) = cx;
    matrix_dst_.at<float>(1, 0) = 0;
    matrix_dst_.at<float>(1, 1) = scale;
    matrix_dst_.at<float>(1, 2) = cy;
}

void FaceParse::get_affine_matrix_for_osd(Bbox& bbox)
{
    float factor = 2.7;
    float edge_size = input_shapes_[0][2];
	float trans_distance = edge_size / 2.0;
	float height = bbox.h;
	float width = bbox.w;
	float center_x = bbox.x + bbox.w / 2.0 ;
	float center_y = bbox.y + bbox.h / 2.0 ;
	float maximum_edge = factor * (height > width ? height : width);
	float scale = edge_size * 2.0 / maximum_edge;
	float cx = trans_distance - scale * center_x;
	float cy = trans_distance - scale * center_y;
	matrix_dst_for_osd_ = cv::Mat::zeros(3, 3, CV_32FC1);
    matrix_dst_for_osd_.at<float>(0, 0) = scale;
    matrix_dst_for_osd_.at<float>(0, 1) = 0;
    matrix_dst_for_osd_.at<float>(0, 2) = cx;
    matrix_dst_for_osd_.at<float>(1, 0) = 0;
    matrix_dst_for_osd_.at<float>(1, 1) = scale;
    matrix_dst_for_osd_.at<float>(1, 2) = cy;
	matrix_dst_for_osd_.at<float>(2, 0) = 0;
	matrix_dst_for_osd_.at<float>(2, 1) = 0;
	matrix_dst_for_osd_.at<float>(2, 2) = 1;
}