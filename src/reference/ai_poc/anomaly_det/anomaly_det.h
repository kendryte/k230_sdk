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

#ifndef _ANOMALY_DET_H_
#define _ANOMALY_DET_H_

#include "ai_base.h"
#include "utils.h"
#include <Eigen/Dense>
#include <Eigen/Core>
#include <unsupported/Eigen/CXX11/Tensor>

using namespace std;

// ANOMALY_DET
class AnomalyDet:public AIBase
{
public:
	/**
     * @brief AnomalyDet构造函数，加载kmodel,并初始化kmodel输入、输出和异常检测阈值
     * @param kmodel_file kmodel文件路径
     * @param obj_thresh  异常检测阈值
     * @param debug_mode  0（不调试）、 1（只显示时间）、2（显示所有打印信息）
     * @return None
     */
    AnomalyDet(const char *kmodel_file, float obj_thresh, const int debug_mode = 1);
	
	~AnomalyDet(){}
	/**
     * @brief 图片预处理，（ai2d for image）
     * @param ori_img 原始图片
     * @return None
     */
    void pre_process(cv::Mat ori_img);

	/**
     * @brief kmodel推理
     * @return None
     */
    void inference();

	/**
     * @brief kmodel推理结果后处理
     * @param final_score 异常检测得分
     * @return None
     */
    void post_process(vector<anomaly_res> &results);

private:

	/**
     * @brief 计算两个矩阵 x1 和 x2 之间的距离矩阵
     * @param x1 第一个输入矩阵
     * @param x2 第二个输入矩阵
     * @return 距离矩阵
     */
	Eigen::MatrixXf cdist_eigen_matmul(const Eigen::MatrixXf& x1, const Eigen::MatrixXf& x2);

    /**
     * @brief 计算输入矩阵 x 的 softmax 函数
     * @param x 输入矩阵
     * @return softmax 后的矩阵
     */
	Eigen::MatrixXf softmax(Eigen::MatrixXf x);

    /**
     * @brief 在矩阵 mat 中找到前 k 个最大值和它们的索引
     * @param mat 输入矩阵
     * @param k 需要找到的最大值数量
     * @return 一个包含最大值和对应索引的 pair
     */
	std::pair<Eigen::VectorXf, Eigen::VectorXi> find_top_k_values(Eigen::MatrixXf mat, int k);

    /**
     * @brief 在 memory_bank 中找到与 embedding 最近的 k 个近邻
     * @param embedding 输入向量
     * @param memory_bank 包含所有可能近邻矩阵
     * @param k 近邻数量
     * @return 包含 k 个近邻值和对应索引的 pair
     */
	std::pair<Eigen::VectorXf, Eigen::VectorXi> nearest_neighbors(Eigen::MatrixXf embedding, Eigen::MatrixXf memory_bank, int k);

    /**
     * @brief 计算异常分数
     * @param patch_score 特定图像块的分数
     * @param locations 位置信息
     * @param embedding 输入向量
     * @param memory_bank 包含所有可能近邻的矩阵
     * @param k 近邻数量
     * @return 异常分数
     */
	float compute_anomaly_score(Eigen::VectorXf patch_score, Eigen::VectorXi locations, Eigen::MatrixXf embedding, Eigen::MatrixXf memory_bank, int k);

    /**
     * @brief 使用最近邻插值法调整图像大小
     * @param input_image 输入图像（3D Tensor）
     * @param target_size 目标尺寸（宽度, 高度）
     * @return 调整大小后的图像（3D Tensor）
     */
	Eigen::Tensor<float, 3> nearest_neighbor_interpolation(Eigen::Tensor<float, 3> input_image, std::pair<int, int> target_size);

    /**
     * @brief 对输入的3D张量进行平均池化操作
     * @param input 输入3D张量
     * @param poolHeight 池化核的高度
     * @param poolWidth 池化核的宽度
     * @param strideH 垂直步长
     * @param strideW 水平步长
     * @param padH 垂直填充
     * @param padW 水平填充
     * @return 平均池化后的3D张量
     */
	Eigen::Tensor<float, 3> avgPool(const Eigen::Tensor<float, 3>& input, int poolHeight, int poolWidth, int strideH, int strideW, int padH, int padW);

	std::unique_ptr<ai2d_builder> ai2d_builder_; // ai2d构建器
    runtime_tensor ai2d_in_tensor_;              // ai2d输入tensor
    runtime_tensor ai2d_out_tensor_;             // ai2d输出tensor

    float obj_thresh_; // 异常检测阈值
    string labels_;
};

#endif