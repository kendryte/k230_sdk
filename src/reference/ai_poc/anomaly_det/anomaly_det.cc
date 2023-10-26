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

#include "anomaly_det.h"
#include "utils.h"
#include "scoped_timing.hpp"
#include <opencv2/core.hpp>
#include <opencv2/core/eigen.hpp>
#include <opencv2/imgproc.hpp>

// for image
AnomalyDet::AnomalyDet(const char *kmodel_file, float obj_thresh, const int debug_mode) : obj_thresh_(obj_thresh), AIBase(kmodel_file,"AnomalyDet", debug_mode)
{
    model_name_ = "AnomalyDet";
    ai2d_out_tensor_ = get_input_tensor(0);
}

void AnomalyDet::pre_process(cv::Mat ori_img)
{
	ScopedTiming st(model_name_ + " pre_process image", debug_mode_);
    std::vector<uint8_t> chw_vec;
    Utils::hwc_to_chw(ori_img, chw_vec);
	Utils::resize({ori_img.channels(), ori_img.rows, ori_img.cols}, chw_vec, ai2d_out_tensor_);
}

void AnomalyDet::inference()
{
	ScopedTiming st(model_name_ + " inference_time", debug_mode_);
    this->run();
    this->get_output();
}

void AnomalyDet::post_process(vector<anomaly_res> &results)
{
	ScopedTiming st(model_name_ + " post_process", debug_mode_);

    //1.读取器模型常量
	Eigen::Tensor<float, 2> memory;
	{
		std::vector<float> memory_vec = Utils::read_binary_file<float>("memory.bin");
		Eigen::TensorMap<Eigen::Tensor<float, 2>> eigen_tensor(memory_vec.data(), 40, 2140);//需要知道形状信息。
		Eigen::Tensor<float, 2> memory_1 = eigen_tensor;
		memory = memory_1.shuffle(Eigen::array<int, 2>{1, 0});
	}

	Eigen::Tensor<float, 2> embedding;
	{

		vector<float> out_1(p_outputs_[0] , p_outputs_[0]+ each_output_size_by_byte_[1]); //开始位置和超尾位置
		vector<float> out_2(p_outputs_[1] , p_outputs_[1] + each_output_size_by_byte_[2]-each_output_size_by_byte_[1]);
		
		Eigen::TensorMap<Eigen::Tensor<float, 3>> out_1_tmap(out_1.data(), 32, 32, 16);
		Eigen::TensorMap<Eigen::Tensor<float, 3>> out_2_tmap(out_2.data(), 16, 16, 24);
		Eigen::Tensor<float, 3> out_1_tensor = out_1_tmap;
		Eigen::Tensor<float, 3> out_2_tensor = out_2_tmap;
		Eigen::Tensor<float, 3> l2 = out_1_tensor.shuffle(Eigen::array<int, 3>{2, 1, 0});
		Eigen::Tensor<float, 3> l3 = out_2_tensor.shuffle(Eigen::array<int, 3>{2, 1, 0});
		
		//3 读取bin文件并变形为需要的形状后，下面进入后处理
		//3-1 对l2和l3进行pooling操作
		Eigen::Tensor<float, 3> l2_pooled = avgPool(l2, 3, 3, 1, 1, 1, 1);
		Eigen::Tensor<float, 3> l3_pooled = avgPool(l3, 3, 3, 1, 1, 1, 1);

		// 3-2 对l3_pooled进行最近邻插值
		std::pair<int, int> target_size(l2_pooled.dimension(1), l2_pooled.dimension(2));
		Eigen::Tensor<float, 3> l3_up = nearest_neighbor_interpolation(l3_pooled, target_size);
		
		// 3-3 将l3_up和l2_pooled在channel维度拼接，得到编码embedding。
		Eigen::Tensor<float, 3> embedding_3d = l2_pooled.concatenate(l3_up, 0);
		
		// 3-4 将embedding变形为需要的形状。
		Eigen::Tensor<float, 3> embedding_1 = embedding_3d.shuffle(Eigen::array<int, 3>{2, 1, 0});
		embedding = embedding_1.reshape(Eigen::array<Eigen::Index, 2>{1024, embedding_1.dimension(2)});
		
	}	
	
    // 4 将 Eigen::Tensor 转换为 Eigen::Matrix（embedding和memory的转换）。
	Eigen::MatrixXf embedding_matrix = Eigen::MatrixXf::Map(embedding.data(), embedding.dimension(0), embedding.dimension(1));
	Eigen::MatrixXf memory_matrix = Eigen::MatrixXf::Map(memory.data(), memory.dimension(0), memory.dimension(1));

    // 5 后处理流程
	std::pair<Eigen::VectorXf, Eigen::VectorXi> score_and_locations = nearest_neighbors(embedding_matrix, memory_matrix, 1);
	Eigen::VectorXf patch_scores = score_and_locations.first;
	Eigen::VectorXi locations = score_and_locations.second;
	float final_score;
    final_score = compute_anomaly_score(patch_scores, locations, embedding_matrix, memory_matrix, 9);
	if (final_score > obj_thresh_)
	{
		labels_ = "anomaly";
	}
	else
	{
		labels_ = "nomal";
	}
	results.resize(1);
	results[0].label = labels_;
	results[0].score = final_score;
}

Eigen::MatrixXf AnomalyDet::cdist_eigen_matmul(const Eigen::MatrixXf& x1, const Eigen::MatrixXf& x2) 
{
	

 	Eigen::MatrixXf x1_norm = x1.rowwise().squaredNorm();
    Eigen::MatrixXf x2_norm = x2.rowwise().squaredNorm();
    Eigen::MatrixXf x1x2 = x1 * x2.transpose();
    Eigen::MatrixXf dists = (x1_norm.rowwise().replicate(x2.rows()) + x2_norm.transpose().colwise().replicate(x1.rows()) - 2 * x1x2).array().max(0.0).sqrt();

    return dists;

}

Eigen::MatrixXf AnomalyDet::softmax(Eigen::MatrixXf x) 
{
	// 按行取最大值，返回一个行向量
	Eigen::VectorXf max_x = x.rowwise().maxCoeff();

	// 按行做减法，每一行都减去该行的最大值
	x = (x.array().colwise() - max_x.array()).matrix();

	// 对每一行做exp操作
	Eigen::MatrixXf exp_x = x.array().exp();

	// 对每一行做除法操作
	Eigen::VectorXf sum_exp_x = exp_x.rowwise().sum();
	Eigen::MatrixXf result = (exp_x.array().colwise() / sum_exp_x.array()).matrix();
	return result;
}

std::pair<Eigen::VectorXf, Eigen::VectorXi> AnomalyDet::find_top_k_values(Eigen::MatrixXf mat, int k) 
{

	std::vector<float> values_vec;
	std::vector<int> indices_vec;
	for (int i = 0; i < mat.rows(); i++) {
		Eigen::RowVectorXf row = mat.row(i);
		std::vector<std::pair<float, int>> elements;
		for (int j = 0; j < row.size(); j++) {
			elements.push_back(std::make_pair(row(j), j));
		}
		std::partial_sort(elements.begin(), elements.begin() + k, elements.end(), std::less<>());
		for (int j = 0; j < k; j++) {
			values_vec.push_back(elements[j].first);
			indices_vec.push_back(elements[j].second);
		}
	}
	Eigen::VectorXf values(k * mat.rows());
	Eigen::VectorXi indices(k * mat.rows());
	for (int i = 0; i < values_vec.size(); i++) {
		values(i) = values_vec[i];
		indices(i) = indices_vec[i];
	}
	return std::make_pair(values, indices);

}

std::pair<Eigen::VectorXf, Eigen::VectorXi> AnomalyDet::nearest_neighbors(Eigen::MatrixXf embedding, Eigen::MatrixXf memory_bank, int k) 
{

	Eigen::MatrixXf distance = cdist_eigen_matmul(embedding, memory_bank);
	
	if (k == 1) {

		//转换为cv::Mat加速计算
		cv::Mat distance_cv;
		cv::eigen2cv(distance, distance_cv);
		cv::Mat patch_scores_cv, locations_cv;
		auto m_start = std::chrono::steady_clock::now();
		cv::reduce(distance_cv, patch_scores_cv, 1, cv::REDUCE_MIN, CV_32F);
		cv::Mat sorted_indices;
		cv::sortIdx(distance_cv, sorted_indices, cv::SORT_EVERY_ROW + cv::SORT_ASCENDING);
		locations_cv = sorted_indices.col(0);
		Eigen::VectorXf patch_scores;
		Eigen::VectorXi locations;
		cv::cv2eigen(patch_scores_cv, patch_scores);
		cv::cv2eigen(locations_cv, locations);
		return {patch_scores, locations};
	}
	else {
		return find_top_k_values(distance, k);
	}
	
}

float AnomalyDet::compute_anomaly_score(Eigen::VectorXf patch_score, Eigen::VectorXi locations, Eigen::MatrixXf embedding, Eigen::MatrixXf memory_bank, int k) 
{
	int max_patch;
	auto score = patch_score.maxCoeff(&max_patch);
	Eigen::MatrixXf max_patch_features = embedding.row(max_patch);
	int nn_index = locations(max_patch);
	Eigen::MatrixXf nn_sample = memory_bank.row(nn_index);
	Eigen::VectorXi support_samples = nearest_neighbors(nn_sample, memory_bank, k).second;
	Eigen::MatrixXf support_memory(support_samples.size(), memory_bank.cols());
	for (int i = 0; i < support_samples.size(); ++i) {
		support_memory.row(i) = memory_bank.row(support_samples(i));
	}
	Eigen::MatrixXf distances = cdist_eigen_matmul(max_patch_features, support_memory);
	float weight = 1.0 - softmax(distances)(0, 0);
	float final_score = weight * score;

	return final_score;
}

Eigen::Tensor<float, 3> AnomalyDet::nearest_neighbor_interpolation(Eigen::Tensor<float, 3> input_image, std::pair<int, int> target_size) 
{
	// 计算每个目标像素对应的输入像素的位置
	Eigen::VectorXd row_indices = Eigen::VectorXd::LinSpaced(target_size.first, 0, target_size.first - 1) * input_image.dimension(1) / target_size.first;
	Eigen::VectorXd col_indices = Eigen::VectorXd::LinSpaced(target_size.second, 0, target_size.second - 1) * input_image.dimension(2) / target_size.second;

	// 使用数组索引和广播机制进行最近邻插值
	Eigen::Tensor<float, 3> output_image(input_image.dimension(0), target_size.first, target_size.second);

	for (int c = 0; c < input_image.dimension(0); ++c) {
		for (int i = 0; i < target_size.first; ++i) {
			for (int j = 0; j < target_size.second; ++j) {
				int row_index = floor(row_indices(i));
				int col_index = floor(col_indices(j));
				output_image(c, i, j) = input_image(c, row_index, col_index);
			}
		}
	}

	return output_image;
}

Eigen::Tensor<float, 3> AnomalyDet::avgPool(const Eigen::Tensor<float, 3>& input, int poolHeight, int poolWidth, int strideH, int strideW, int padH, int padW)
{
	// 计算输出的大小
	int outHeight = ((input.dimension(1) - poolHeight + 2 * padH) / strideH) + 1;
	int outWidth = ((input.dimension(2) - poolWidth + 2 * padW) / strideW) + 1;

	// 创建输出矩阵并初始化为零
	Eigen::Tensor<float, 3> output(input.dimension(0), outHeight, outWidth);
	int channels = input.dimension(0);
	int input_rows = input.dimension(1);
	int input_cols = input.dimension(2);
	int poolArea = poolHeight * poolWidth;

	// 执行平均池化
	for (int c = 0; c < channels; ++c) {
		for (int h = 0; h < outHeight; ++h) {
			for (int w = 0; w < outWidth; ++w) {
				int startH = h * strideH - padH;
				int startW = w * strideW - padW;
				int endH = std::min(startH + poolHeight, input_rows);
				int endW = std::min(startW + poolWidth, input_cols);
				startH = std::max(startH, 0);
				startW = std::max(startW, 0);
				float poolSum = 0.0;
				for (int ph = startH; ph < endH; ++ph)
				{
					for (int pw = startW; pw < endW; ++pw)
					{
						poolSum += input(c, ph, pw);
						//++poolArea;
					}
				}
				output(c, h, w) = poolSum / poolArea;
			}
		}
	}
	return output;
}

