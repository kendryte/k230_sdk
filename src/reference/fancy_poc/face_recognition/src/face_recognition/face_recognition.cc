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
#include <dirent.h>
#include <vector>
#include "face_recognition.h"
#include "k_autoconf_comm.h"

FaceRecognition::FaceRecognition(const char *kmodel_file, int max_register_face, float thresh, const int debug_mode) : AIBase(kmodel_file, "FaceRecognition", debug_mode)
{
	model_name_ = "FaceRecognition";
	feature_num_ = output_shapes_[0][1];
	max_register_face_ = max_register_face;
	obj_thresh_ = thresh;
	valid_register_face_ = 0;
	// create_database
	feature_database_ = new float[max_register_face_ * feature_num_];
	ai2d_out_tensor_ = get_input_tensor(0);
}

FaceRecognition::FaceRecognition(const char *kmodel_file, int max_register_face, float thresh, FrameCHWSize isp_shape, uintptr_t vaddr, uintptr_t paddr, const int debug_mode) : AIBase(kmodel_file, "FaceRecognition", debug_mode)
{
	model_name_ = "FaceRecognition";
	feature_num_ = output_shapes_[0][1];
	max_register_face_ = max_register_face;
	obj_thresh_ = thresh;
	valid_register_face_ = 0;
	// create_database
	feature_database_ = new float[max_register_face_ * feature_num_];

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

FaceRecognition::~FaceRecognition()
{
	delete[] feature_database_;
}

// ai2d for image
void FaceRecognition::pre_process(cv::Mat ori_img, float *sparse_points)
{
	ScopedTiming st(model_name_ + " pre_process image", debug_mode_);
	get_affine_matrix(sparse_points);

	std::vector<uint8_t> chw_vec;
	Utils::bgr2rgb_and_hwc2chw(ori_img, chw_vec);
	Utils::affine({ori_img.channels(), ori_img.rows, ori_img.cols}, chw_vec, matrix_dst_, ai2d_out_tensor_);
}

// ai2d for video
void FaceRecognition::pre_process(float *sparse_points)
{
	ScopedTiming st(model_name_ + " pre_process_video", debug_mode_);
	get_affine_matrix(sparse_points);
#if 1
	size_t isp_size = isp_shape_.channel * isp_shape_.height * isp_shape_.width;
	auto buf = ai2d_in_tensor_.impl()->to_host().unwrap()->buffer().as_host().unwrap().map(map_access_::map_write).unwrap().buffer();
	memcpy(reinterpret_cast<char *>(buf.data()), (void *)vaddr_, isp_size);
	hrt::sync(ai2d_in_tensor_, sync_op_t::sync_write_back, true).expect("sync write_back failed");
#endif
	Utils::affine(matrix_dst_, ai2d_builder_, ai2d_in_tensor_, ai2d_out_tensor_);
}

void FaceRecognition::inference()
{
	this->run();
	this->get_output();
}

int FaceRecognition::get_dir_files(const char *path)
{
	DIR *directory = opendir(path);
	vector<string> files;

	if (directory == nullptr)
	{
		std::cerr << "无法打开目录" << std::endl;
		return 0;
	}

	dirent *entry;
	while ((entry = readdir(directory)) != nullptr)
	{
		if (entry->d_type == DT_REG)
		{
			files.push_back(entry->d_name);
			if(debug_mode_>1)
				std::cout << entry->d_name << std::endl;
		}
	}

	closedir(directory);
	return files.size() / 2;
}

void FaceRecognition::database_init(char *db_pth)
{
	
	vector<string> files;
	int file_num = get_dir_files(db_pth);
	if( debug_mode_> 0)
		std::cout<<"found "<< file_num <<" pieces of data in db"<<std::endl;

	valid_register_face_ = 0;
	ScopedTiming st(model_name_ + " database_init", 1);
	for (int i = 1; i <= file_num; ++i)
	{
		int valid_index = valid_register_face_ % max_register_face_;
		std::string fname = string(db_pth) + "/" + std::to_string(i) + ".db";
		vector<float> db_vec = Utils::read_binary_file<float>(fname.c_str());
		memcpy(feature_database_ + valid_index * feature_num_, db_vec.data(), sizeof(float) * feature_num_);

		fname = string(db_pth) + "/" + std::to_string(i) + ".name";
		vector<char> name_vec = Utils::read_binary_file<char>(fname.c_str());
		string current_name(name_vec.begin(), name_vec.end());
		names_.push_back(current_name);
		valid_register_face_ += 1;
		if (valid_register_face_ >= max_register_face_)
		{
			std::cout << "valid_register_face will exceed max_register_face,stop loading redundant databases!" << std::endl;
			break;
		}
	}
	std::cout << "init database Done!" << std::endl;
}

void FaceRecognition::database_search(FaceRecognitionInfo &result)
{
	int i;
	int v_id = -1;
	float v_score;
	float v_score_max = 0.0;
	float basef[feature_num_], testf[feature_num_];
	// current frame
	l2_normalize(p_outputs_[0], testf, feature_num_);
	for (i = 0; i < valid_register_face_; i++)
	{
		l2_normalize(feature_database_ + i * feature_num_, basef, feature_num_);
		v_score = cal_cosine_distance(testf, basef, feature_num_);
		if (v_score > v_score_max)
		{
			v_score_max = v_score;
			v_id = i;
		}
	}
	if (v_id == -1)
	{
		result.id = v_id;
		result.name = "unknown";
		result.score = 0;
	}
	else
	{
		result.id = v_id;
		result.name = names_[v_id];
		result.score = v_score_max;
	}
}

void FaceRecognition::draw_result(cv::Mat &src_img, Bbox &bbox, FaceRecognitionInfo &result, bool pic_mode)
{
	int src_w = src_img.cols;
	int src_h = src_img.rows;
	int max_src_size = std::max(src_w, src_h);
	char text[30];
	if (result.score > obj_thresh_)
	{
		sprintf(text, "%s:%.2f", result.name.c_str(), result.score);
		// sprintf(text, "%s",result.name.c_str());
	}
	else
	{
		sprintf(text, "unknown");
	}

	if (pic_mode)
	{
		cv::rectangle(src_img, cv::Rect(bbox.x, bbox.y, bbox.w, bbox.h), cv::Scalar(255, 255, 255), 2, 2, 0);
		cv::putText(src_img, text, {bbox.x, std::max(int(bbox.y - 10), 0)}, cv::FONT_HERSHEY_COMPLEX, 0.5, cv::Scalar(255, 0, 255), 1, 8, 0);
	}
	else
	{
		int x = bbox.x / isp_shape_.width * src_w;
		int y = bbox.y / isp_shape_.height * src_h;
		int w = bbox.w / isp_shape_.width * src_w;
		int h = bbox.h / isp_shape_.height * src_h;
		cv::rectangle(src_img, cv::Rect(x, y , w, h), cv::Scalar(255,255, 255, 255), 6, 2, 0);
		#if defined(CONFIG_BOARD_K230_CANMV)
			cv::putText(src_img, text, {x, std::max(int(y - 10), 0)}, cv::FONT_HERSHEY_COMPLEX, 1.0, cv::Scalar(255, 255, 0, 255), 1, 8, 0);
		#else
			cv::putText(src_img, text, {x, std::max(int(y - 10), 0)}, cv::FONT_HERSHEY_COMPLEX, 2.0, cv::Scalar(255, 255, 0, 255), 2, 8, 0);
		#endif
		
	}
}

void FaceRecognition::svd22(const float a[4], float u[4], float s[2], float v[4])
{
	s[0] = (sqrtf(powf(a[0] - a[3], 2) + powf(a[1] + a[2], 2)) + sqrtf(powf(a[0] + a[3], 2) + powf(a[1] - a[2], 2))) / 2;
	s[1] = fabsf(s[0] - sqrtf(powf(a[0] - a[3], 2) + powf(a[1] + a[2], 2)));
	v[2] = (s[0] > s[1]) ? sinf((atan2f(2 * (a[0] * a[1] + a[2] * a[3]), a[0] * a[0] - a[1] * a[1] + a[2] * a[2] - a[3] * a[3])) / 2) : 0;
	v[0] = sqrtf(1 - v[2] * v[2]);
	v[1] = -v[2];
	v[3] = v[0];
	u[0] = (s[0] != 0) ? -(a[0] * v[0] + a[1] * v[2]) / s[0] : 1;
	u[2] = (s[0] != 0) ? -(a[2] * v[0] + a[3] * v[2]) / s[0] : 0;
	u[1] = (s[1] != 0) ? (a[0] * v[1] + a[1] * v[3]) / s[1] : -u[2];
	u[3] = (s[1] != 0) ? (a[2] * v[1] + a[3] * v[3]) / s[1] : u[0];
	v[0] = -v[0];
	v[2] = -v[2];
}

static float umeyama_args_112[] =
	{
#define PIC_SIZE 112
		38.2946 * PIC_SIZE / 112, 51.6963 * PIC_SIZE / 112,
		73.5318 * PIC_SIZE / 112, 51.5014 * PIC_SIZE / 112,
		56.0252 * PIC_SIZE / 112, 71.7366 * PIC_SIZE / 112,
		41.5493 * PIC_SIZE / 112, 92.3655 * PIC_SIZE / 112,
		70.7299 * PIC_SIZE / 112, 92.2041 * PIC_SIZE / 112};

void FaceRecognition::image_umeyama_112(float *src, float *dst)
{
#define SRC_NUM 5
#define SRC_DIM 2
	int i, j, k;
	float src_mean[SRC_DIM] = {0.0};
	float dst_mean[SRC_DIM] = {0.0};
	for (i = 0; i < SRC_NUM * 2; i += 2)
	{
		src_mean[0] += src[i];
		src_mean[1] += src[i + 1];
		dst_mean[0] += umeyama_args_112[i];
		dst_mean[1] += umeyama_args_112[i + 1];
	}
	src_mean[0] /= SRC_NUM;
	src_mean[1] /= SRC_NUM;
	dst_mean[0] /= SRC_NUM;
	dst_mean[1] /= SRC_NUM;

	float src_demean[SRC_NUM][2] = {0.0};
	float dst_demean[SRC_NUM][2] = {0.0};

	for (i = 0; i < SRC_NUM; i++)
	{
		src_demean[i][0] = src[2 * i] - src_mean[0];
		src_demean[i][1] = src[2 * i + 1] - src_mean[1];
		dst_demean[i][0] = umeyama_args_112[2 * i] - dst_mean[0];
		dst_demean[i][1] = umeyama_args_112[2 * i + 1] - dst_mean[1];
	}

	float A[SRC_DIM][SRC_DIM] = {0.0};
	for (i = 0; i < SRC_DIM; i++)
	{
		for (k = 0; k < SRC_DIM; k++)
		{
			for (j = 0; j < SRC_NUM; j++)
			{
				A[i][k] += dst_demean[j][i] * src_demean[j][k];
			}
			A[i][k] /= SRC_NUM;
		}
	}

	float(*T)[SRC_DIM + 1] = (float(*)[SRC_DIM + 1]) dst;
	T[0][0] = 1;
	T[0][1] = 0;
	T[0][2] = 0;
	T[1][0] = 0;
	T[1][1] = 1;
	T[1][2] = 0;
	T[2][0] = 0;
	T[2][1] = 0;
	T[2][2] = 1;

	float U[SRC_DIM][SRC_DIM] = {0};
	float S[SRC_DIM] = {0};
	float V[SRC_DIM][SRC_DIM] = {0};
	svd22(&A[0][0], &U[0][0], S, &V[0][0]);

	T[0][0] = U[0][0] * V[0][0] + U[0][1] * V[1][0];
	T[0][1] = U[0][0] * V[0][1] + U[0][1] * V[1][1];
	T[1][0] = U[1][0] * V[0][0] + U[1][1] * V[1][0];
	T[1][1] = U[1][0] * V[0][1] + U[1][1] * V[1][1];

	float scale = 1.0;
	float src_demean_mean[SRC_DIM] = {0.0};
	float src_demean_var[SRC_DIM] = {0.0};
	for (i = 0; i < SRC_NUM; i++)
	{
		src_demean_mean[0] += src_demean[i][0];
		src_demean_mean[1] += src_demean[i][1];
	}
	src_demean_mean[0] /= SRC_NUM;
	src_demean_mean[1] /= SRC_NUM;

	for (i = 0; i < SRC_NUM; i++)
	{
		src_demean_var[0] += (src_demean_mean[0] - src_demean[i][0]) * (src_demean_mean[0] - src_demean[i][0]);
		src_demean_var[1] += (src_demean_mean[1] - src_demean[i][1]) * (src_demean_mean[1] - src_demean[i][1]);
	}
	src_demean_var[0] /= (SRC_NUM);
	src_demean_var[1] /= (SRC_NUM);
	scale = 1.0 / (src_demean_var[0] + src_demean_var[1]) * (S[0] + S[1]);
	T[0][2] = dst_mean[0] - scale * (T[0][0] * src_mean[0] + T[0][1] * src_mean[1]);
	T[1][2] = dst_mean[1] - scale * (T[1][0] * src_mean[0] + T[1][1] * src_mean[1]);
	T[0][0] *= scale;
	T[0][1] *= scale;
	T[1][0] *= scale;
	T[1][1] *= scale;
	float(*TT)[3] = (float(*)[3])T;
}

void FaceRecognition::get_affine_matrix(float *sparse_points)
{
	float matrix_src[5][2];
	for (uint32_t i = 0; i < 5; ++i)
	{
		matrix_src[i][0] = sparse_points[2 * i + 0];
		matrix_src[i][1] = sparse_points[2 * i + 1];
	}
	image_umeyama_112(&matrix_src[0][0], &matrix_dst_[0]);
}

void FaceRecognition::l2_normalize(float *src, float *dst, int len)
{
	float sum = 0;
	for (int i = 0; i < len; ++i)
	{
		sum += src[i] * src[i];
	}
	sum = sqrtf(sum);
	for (int i = 0; i < len; ++i)
	{
		dst[i] = src[i] / sum;
	}
}

float FaceRecognition::cal_cosine_distance(float *feature_0, float *feature_1, int feature_len)
{
	float cosine_distance = 0;
	// calculate the sum square
	for (int i = 0; i < feature_len; ++i)
	{
		float p0 = *(feature_0 + i);
		float p1 = *(feature_1 + i);
		cosine_distance += p0 * p1;
	}
	// cosine distance
	return (0.5 + 0.5 * cosine_distance) * 100;
}