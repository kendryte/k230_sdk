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
#include "face_pose.h"
#include <vector>

#define PI (3.1415926)

FacePose::FacePose(const char *kmodel_file, const int debug_mode) : AIBase(kmodel_file,"FacePose",debug_mode)
{
    model_name_ = "FacePose";
    ai2d_out_tensor_ = get_input_tensor(0);
}

FacePose::FacePose(const char *kmodel_file, FrameCHWSize isp_shape, uintptr_t vaddr, uintptr_t paddr, const int debug_mode) : AIBase(kmodel_file,"FacePose", debug_mode)
{
    model_name_ = "FacePose";
	
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

FacePose::~FacePose()
{
}

// ai2d for image
void FacePose::pre_process(cv::Mat ori_img, Bbox& bbox)
{
    ScopedTiming st(model_name_ + " pre_process image", debug_mode_);
    get_affine_matrix(bbox);

    std::vector<uint8_t> chw_vec;
    Utils::bgr2rgb_and_hwc2chw(ori_img, chw_vec);
    Utils::affine({ori_img.channels(), ori_img.rows, ori_img.cols}, chw_vec, affine_matrix_, ai2d_out_tensor_);
    
    // auto vaddr_out_buf = ai2d_out_tensor_.impl()->to_host().unwrap()->buffer().as_host().unwrap().map(map_access_::map_read).unwrap().buffer();
    // unsigned char *output = reinterpret_cast<unsigned char *>(vaddr_out_buf.data());
    // Utils::dump_gray_image("FacePose_input_gray.png",{input_shapes_[0][3],input_shapes_[0][2]},output);
}

// ai2d for video
void FacePose::pre_process(Bbox& bbox)
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
    Utils::affine(affine_matrix_, ai2d_builder_, ai2d_in_tensor_, ai2d_out_tensor_);

    // auto vaddr_out_buf = ai2d_out_tensor_.impl()->to_host().unwrap()->buffer().as_host().unwrap().map(map_access_::map_read).unwrap().buffer();
    // unsigned char *output = reinterpret_cast<unsigned char *>(vaddr_out_buf.data());
    // Utils::dump_gray_image("FacePose_input.png",{input_shapes_[0][3],input_shapes_[0][2]},output);
}

void FacePose::inference()
{
    this->run();
    this->get_output();
}

void FacePose::post_process(FacePoseInfo& result)
{
    ScopedTiming st(model_name_ + " post_process", debug_mode_);
	get_euler(p_outputs_[0], result);
}

void FacePose::draw_result(cv::Mat& src_img,Bbox& bbox,FacePoseInfo& result, bool pic_mode)
{
	int src_width = src_img.cols;
    int src_height = src_img.rows;

	float height = bbox.h;
	float width = bbox.w;
	float center_x = bbox.x + bbox.w / 2.0 ;
	float center_y = bbox.y + bbox.h / 2.0 ;
	float projections[8][3] = { 0.0 };
	float radius = 0.5 * (height > width ? height : width);
	build_projection_matrix(radius, &projections[0][0]);

	std::vector<cv::Point> first_points;
	std::vector<cv::Point> second_points;
	for (uint32_t pp = 0; pp < 8; pp++)
	{
		cv::Point point;
		float sum_x = 0.0, sum_y = 0.0;
		int x, y;
		for (uint32_t cc = 0; cc < 3; cc++)
		{
			sum_x += projections[pp][cc] * R[cc][0];
			sum_y += projections[pp][cc] * (-R[cc][1]);
		}
		if(pic_mode)
		{
			x = static_cast<int>(sum_x + center_x);
			y = static_cast<int>(sum_y + center_y);
		}
		else
		{
			x = (sum_x + center_x)/isp_shape_.width*src_width;
            y = (sum_y + center_y)/isp_shape_.height*src_height;
		}
		//check
		point.x = std::max(0, std::min(x, src_width));
		point.y = std::max(0, std::min(y, src_height));
		if(pp<4)
			first_points.push_back(point);
		else
			second_points.push_back(point);
	}

	if(pic_mode)
	{
		cv::polylines(src_img, first_points, true, cv::Scalar(0, 0, 255), 2, 8, 0);
		cv::polylines(src_img, second_points, true, cv::Scalar(0, 0, 255), 2, 8, 0);
		for (uint32_t ll = 0; ll < 4; ll++)
		{
			cv::line(src_img, first_points[ll], second_points[ll], cv::Scalar(0, 0, 255), 5, 8, 0);
		}
		char text[50]={0};
    	sprintf(text, "roll:%.2f,yaw:%.2f,pitch:%.2f.",result.roll,result.yaw,result.pitch);
		int x = bbox.x;
        int y = bbox.y;
		cv::putText(src_img, text, {std::max(int(x-10),0), std::max(int(y-10),0)}, cv::FONT_HERSHEY_COMPLEX, 0.8, cv::Scalar(255, 0, 0), 2, 4, 0);
	}
	else
	{
		cv::polylines(src_img, first_points, true, cv::Scalar(255, 0, 255, 255), 5, 8, 0);
		cv::polylines(src_img, second_points, true, cv::Scalar(255, 0, 255, 255), 5, 8, 0);
		for (uint32_t ll = 0; ll < 4; ll++)
		{
			cv::line(src_img, first_points[ll], second_points[ll], cv::Scalar(255, 0, 255, 255), 5, 8, 0);
		}
	}   
}

void FacePose::get_affine_matrix(Bbox& bbox)
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
	affine_matrix_[0] = scale;
	affine_matrix_[1] = 0;
	affine_matrix_[2] = cx;
	affine_matrix_[3] = 0;
	affine_matrix_[4] = scale;
	affine_matrix_[5] = cy;
}

void FacePose::rotation_matrix_to_euler_angles(float (*R)[3], FacePoseInfo& eular)
{
	float sy = sqrtf(powf(R[0][0], 2) + powf(R[1][0], 2));
	if (sy < 1e-6)
	{
		eular.pitch = atan2f(-R[1][2], R[1][1]) * 180 / PI;
		eular.yaw = atan2f(-R[2][0], sy) * 180 / PI;
		eular.roll = 0;
	}
	else
	{
		eular.pitch = atan2f(R[2][1], R[2][2]) * 180 / PI;
		eular.yaw = atan2f(-R[2][0], sy) * 180 / PI;
		eular.roll = atan2f(R[1][0], R[0][0]) * 180 / PI;
	}
}

void FacePose::build_projection_matrix(float rear_size, float *projections)
{
	float rear_depth = 0;
	float factor = sqrtf(2.0);
	float front_size = factor * rear_size;
	float front_depth = factor * rear_size;
	float temp[8][3] = { {-rear_size, -rear_size, rear_depth},
				{-rear_size, rear_size, rear_depth},
				{rear_size, rear_size, rear_depth},
				{rear_size, -rear_size, rear_depth},
				{-front_size, -front_size, front_depth},
				{-front_size, front_size, front_depth},
				{front_size, front_size, front_depth},
				{front_size, -front_size, front_depth},
			};		
	memcpy(projections, temp, 8 * 3 * sizeof(float));
}

void FacePose::get_euler(float *data, FacePoseInfo& eular)
{
	for(uint32_t hh = 0; hh < 3; hh++)
	{
		memcpy(&R[hh][0], data + hh * 4, 3 * sizeof(float));
	}
	rotation_matrix_to_euler_angles(R, eular);
}