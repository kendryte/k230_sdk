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
#include "face_glasses.h"
#include <vector>

FaceGlasses::FaceGlasses(const char *kmodel_file, const int debug_mode) : AIBase(kmodel_file,"FaceGlasses",debug_mode)
{
    model_name_ = "FaceGlasses";
    ai2d_out_tensor_ = get_input_tensor(0);
}

FaceGlasses::FaceGlasses(const char *kmodel_file, FrameCHWSize isp_shape, uintptr_t vaddr, uintptr_t paddr, const int debug_mode) : AIBase(kmodel_file,"FaceGlasses", debug_mode)
{
    model_name_ = "FaceGlasses";

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

FaceGlasses::~FaceGlasses()
{
}

// ai2d for image
void FaceGlasses::pre_process(cv::Mat ori_img, float* sparse_points)
{
    ScopedTiming st(model_name_ + " pre_process image", debug_mode_);
    get_affine_matrix(sparse_points);

    std::vector<uint8_t> chw_vec;
    Utils::bgr2rgb_and_hwc2chw(ori_img, chw_vec);

    Utils::affine({ori_img.channels(), ori_img.rows, ori_img.cols}, chw_vec, matrix_dst_, ai2d_out_tensor_);
    
    // auto vaddr_out_buf = ai2d_out_tensor_.impl()->to_host().unwrap()->buffer().as_host().unwrap().map(map_access_::map_read).unwrap().buffer();
    // unsigned char *output = reinterpret_cast<unsigned char *>(vaddr_out_buf.data());
    // Utils::dump_gray_image("FaceGlasses_input_gray.png",{input_shapes_[0][3],input_shapes_[0][2]},output);
}

// ai2d for video
void FaceGlasses::pre_process(float* sparse_points)
{
    ScopedTiming st(model_name_ + " pre_process_video", debug_mode_);
    get_affine_matrix(sparse_points);

#if 1
    size_t isp_size = isp_shape_.channel * isp_shape_.height * isp_shape_.width;
    auto buf = ai2d_in_tensor_.impl()->to_host().unwrap()->buffer().as_host().unwrap().map(map_access_::map_write).unwrap().buffer();
    memcpy(reinterpret_cast<char *>(buf.data()), (void *)vaddr_, isp_size);
    hrt::sync(ai2d_in_tensor_, sync_op_t::sync_write_back, true).expect("sync write_back failed");
    // run ai2d
#endif
    Utils::affine(matrix_dst_, ai2d_builder_, ai2d_in_tensor_, ai2d_out_tensor_);

    // auto vaddr_out_buf = ai2d_out_tensor_.impl()->to_host().unwrap()->buffer().as_host().unwrap().map(map_access_::map_read).unwrap().buffer();
    // unsigned char *output = reinterpret_cast<unsigned char *>(vaddr_out_buf.data());
    // Utils::dump_gray_image("FaceGlasses_input.png",{input_shapes_[0][3],input_shapes_[0][2]},output);
}

void FaceGlasses::inference()
{
    this->run();
    this->get_output();
}

void FaceGlasses::post_process(FaceGlassesInfo& result)
{
    ScopedTiming st(model_name_ + " post_process", debug_mode_);
    vector<float> pred_vec(p_outputs_[0],p_outputs_[0] + output_shapes_[0][1]);
    vector<float> softmax_pred_vec;
    softmax(pred_vec,softmax_pred_vec);
    int max_index = argmax(softmax_pred_vec.data(),softmax_pred_vec.size());
    result.score = softmax_pred_vec[max_index];
	
    if(max_index==0 && result.score>0.75)
    {
		result.score = softmax_pred_vec[0];
        result.label = "no glasses";
    }
    else
    {
		result.score = 1-softmax_pred_vec[0];
        result.label = "wear glasses";
    }
}

void FaceGlasses::draw_result(cv::Mat& src_img,Bbox& bbox,FaceGlassesInfo& result, bool pic_mode)
{
    int src_w = src_img.cols;
    int src_h = src_img.rows;
    int max_src_size = std::max(src_w,src_h);

    char text[30];
    //sprintf(text, "%s:%.2f",result.label.c_str(), result.score);
	sprintf(text, "%s",result.label.c_str());

    if(pic_mode)
    {
        cv::rectangle(src_img, cv::Rect(bbox.x, bbox.y , bbox.w, bbox.h), cv::Scalar(255, 255, 255), 2, 2, 0);
        cv::putText(src_img, text , {bbox.x,std::max(int(bbox.y-10),0)}, cv::FONT_HERSHEY_COMPLEX, 0.5, cv::Scalar(255, 0, 0), 1, 8, 0);
    }
    else
    {
	    int x = bbox.x / isp_shape_.width * src_w;
        int y = bbox.y / isp_shape_.height * src_h;
        int w = bbox.w / isp_shape_.width * src_w;
        int h = bbox.h / isp_shape_.height  * src_h;
        cv::rectangle(src_img, cv::Rect(x, y , w, h), cv::Scalar(255,255, 255, 255), 2, 2, 0);
        if(result.label == "no glasses")
			cv::putText(src_img,text,cv::Point(x,std::max(int(y-10),0)),cv::FONT_HERSHEY_COMPLEX,2,cv::Scalar(255,255, 0, 255), 2, 8, 0);
		else
			cv::putText(src_img,text,cv::Point(x,std::max(int(y-10),0)),cv::FONT_HERSHEY_COMPLEX,2,cv::Scalar(255,255, 255, 0), 2, 8, 0);
    }  
}

void FaceGlasses::svd22(const float a[4], float u[4], float s[2], float v[4])
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

static float umeyama_args_224[] =
{
#define PIC_SIZE 224
	38.2946 * PIC_SIZE / 112,  51.6963 * PIC_SIZE / 112,
	73.5318 * PIC_SIZE / 112, 51.5014 * PIC_SIZE / 112,
	56.0252 * PIC_SIZE / 112, 71.7366 * PIC_SIZE / 112,
	41.5493 * PIC_SIZE / 112, 92.3655 * PIC_SIZE / 112,
	70.7299 * PIC_SIZE / 112, 92.2041 * PIC_SIZE / 112
};

void FaceGlasses::image_umeyama_224(float* src, float* dst)
{
#define SRC_NUM 5
#define SRC_DIM 2
	int i, j, k;
	float src_mean[SRC_DIM] = { 0.0 };
	float dst_mean[SRC_DIM] = { 0.0 };
	for (i = 0; i < SRC_NUM * 2; i += 2)
	{
		src_mean[0] += src[i];
		src_mean[1] += src[i + 1];
		dst_mean[0] += umeyama_args_224[i];
		dst_mean[1] += umeyama_args_224[i + 1];
	}
	src_mean[0] /= SRC_NUM;
	src_mean[1] /= SRC_NUM;
	dst_mean[0] /= SRC_NUM;
	dst_mean[1] /= SRC_NUM;

	float src_demean[SRC_NUM][2] = { 0.0 };
	float dst_demean[SRC_NUM][2] = { 0.0 };

	for (i = 0; i < SRC_NUM; i++)
	{
		src_demean[i][0] = src[2 * i] - src_mean[0];
		src_demean[i][1] = src[2 * i + 1] - src_mean[1];
		dst_demean[i][0] = umeyama_args_224[2 * i] - dst_mean[0];
		dst_demean[i][1] = umeyama_args_224[2 * i + 1] - dst_mean[1];
	}

	float A[SRC_DIM][SRC_DIM] = { 0.0 };
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

	float(*T)[SRC_DIM + 1] = (float(*)[SRC_DIM + 1])dst;
	T[0][0] = 1;
	T[0][1] = 0;
	T[0][2] = 0;
	T[1][0] = 0;
	T[1][1] = 1;
	T[1][2] = 0;
	T[2][0] = 0;
	T[2][1] = 0;
	T[2][2] = 1;

	float U[SRC_DIM][SRC_DIM] = { 0 };
	float S[SRC_DIM] = { 0 };
	float V[SRC_DIM][SRC_DIM] = { 0 };
	svd22(&A[0][0], &U[0][0], S, &V[0][0]);

	T[0][0] = U[0][0] * V[0][0] + U[0][1] * V[1][0];
	T[0][1] = U[0][0] * V[0][1] + U[0][1] * V[1][1];
	T[1][0] = U[1][0] * V[0][0] + U[1][1] * V[1][0];
	T[1][1] = U[1][0] * V[0][1] + U[1][1] * V[1][1];

	float scale = 1.0;
	float src_demean_mean[SRC_DIM] = { 0.0 };
	float src_demean_var[SRC_DIM] = { 0.0 };
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

void FaceGlasses::get_affine_matrix(float* sparse_points)
{
    float matrix_src[5][2];
    for (uint32_t i = 0; i < 5; ++i)
    {
        matrix_src[i][0] = sparse_points[2 * i + 0];
		matrix_src[i][1] = sparse_points[2 * i + 1];
    }
    image_umeyama_224(&matrix_src[0][0], &matrix_dst_[0]);
}

int FaceGlasses::argmax(float* data, uint32_t len)
{
	float max_value = data[0];
	int max_index = 0;
	for (uint32_t i = 1; i < len; i++)
	{
		if (max_value < data[i])
		{
			max_value = data[i];
			max_index = i;
		}
	}
	return max_index;
}

void FaceGlasses::softmax(vector<float>& input,vector<float>& output)
{
    //e_x = np.exp(x - np.max(x))
    std::vector<float>::iterator p_input_max = std::max_element(input.begin(), input.end());
    float input_max = *p_input_max;
    float input_total = 0;
    
    for(auto x:input)
	{
		input_total+=exp( x- input_max);
	}

    output.resize(input.size());
	for(int i=0;i<input.size();++i)
	{
		output[i] = exp(input[i] - input_max)/input_total;
	}
}