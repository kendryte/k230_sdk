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
 * 
 * Part of the code is referenced from https://github.com/mcximing/sort-cpp. The orignal code are published under the BSD license.
 */
#ifndef HUNGARIAN_H
#define HUNGARIAN_H

#include <iostream>
#include <vector>

using namespace std;


/**
 * @brief 匈牙利算法
 * 主要封装了对于跟踪框和检测框的匹配过程
 */
class HungarianAlgorithm
{
public:

	/**
     * @brief HungarianAlgorithm构造函数
     * @return None
     */
	HungarianAlgorithm();

	/**
     * @brief HungarianAlgorithm析构函数
     * @return None
     */
	~HungarianAlgorithm();

	/**
     * @brief 检测框和跟踪框的匹配问题
	 * @param DistMatrix 表示检测框和跟踪框之间的成对距离的代价/距离矩阵
	 * @param Assignment 将检测框与跟踪框匹配的结果
     * @return 最优化匹配的代价/距离
     */
	double Solve(vector<vector<double>>& DistMatrix, vector<int>& Assignment);

private:
	/**
     * @brief 对给定的代价/距离矩阵进行最优分配
	 * @param cost       保存最小代价/距离
	 * @param distMatrix 表示检测框和跟踪框之间的代价/距离矩阵
	 * @param nOfRows    行数
	 * @param nOfColumns 列数
	 * @return None
     */
	void assignmentoptimal(int *assignment, double *cost, double *distMatrix, int nOfRows, int nOfColumns);

	/**
     * @brief 根据星标矩阵构建最终的分配方案
	 * @param assignment 最终的分配方案
	 * @param starMatrix 保存最优分配方案，即每个行的分配结果对应的列索引
	 * @param nOfRows	 行数
	 * @param nOfColumns 列数
     * @return None
     */
	void buildassignmentvector(int *assignment, bool *starMatrix, int nOfRows, int nOfColumns);

	/**
     * @brief 基于分配方案计算最终的代价，即总距离
	 * @param assignment 最终的分配方案
	 * @param cost       保存最小代价/距离
	 * @param distMatrix 表示检测框和跟踪框之间的代价/距离矩阵
	 * @param nOfRows	 行数
     * @return None
     */
	void computeassignmentcost(int *assignment, double *cost, double *distMatrix, int nOfRows);

	/**
     * @brief 执行步骤2a，更新星标矩阵，对于没有被覆盖的0元素，对应位置设为星号
	 * @param assignment	 当前的分配方案
	 * @param distMatrix	 表示检测框和跟踪框之间的代价/距离矩阵
	 * @param starMatrix	 星标矩阵，表示已经分配的分配对
	 * @param newStarMatrix	 新的星标矩阵，用于更新星标矩阵
	 * @param primeMatrix	 素数矩阵，用于辅助计算
	 * @param coveredColumns 被覆盖的列标记
	 * @param coveredRows	 被覆盖的行标记
	 * @param nOfRows		 行数
	 * @param nOfColumns	 列数
	 * @param minDim		 行数和列数的最小值
     * @return None
     */
	void step2a(int *assignment, double *distMatrix, bool *starMatrix, bool *newStarMatrix, bool *primeMatrix, bool *coveredColumns, bool *coveredRows, int nOfRows, int nOfColumns, int minDim);

	/**
     * @brief 执行步骤2b，检查星号位置的列是否已经被覆盖，如果已覆盖则转移到步骤3，如果未覆盖则转移到步骤3
	 * @param assignment	 当前的分配方案
	 * @param distMatrix	 表示检测框和跟踪框之间的代价/距离矩阵
	 * @param starMatrix	 星标矩阵，表示已经分配的分配对
	 * @param newStarMatrix	 新的星标矩阵，用于更新星标矩阵
	 * @param primeMatrix	 素数矩阵，用于辅助计算
	 * @param coveredColumns 被覆盖的列标记
	 * @param coveredRows	 被覆盖的行标记
	 * @param nOfRows		 行数
	 * @param nOfColumns	 列数
	 * @param minDim		 行数和列数的最小值
     * @return None
     */
	void step2b(int *assignment, double *distMatrix, bool *starMatrix, bool *newStarMatrix, bool *primeMatrix, bool *coveredColumns, bool *coveredRows, int nOfRows, int nOfColumns, int minDim);
	
	/**
     * @brief 执行步骤3，更新代价矩阵和覆盖的行列
	 * @param assignment	 当前的分配方案
	 * @param distMatrix	 表示检测框和跟踪框之间的代价/距离矩阵
	 * @param starMatrix	 星标矩阵，表示已经分配的分配对
	 * @param newStarMatrix	 新的星标矩阵，用于更新星标矩阵
	 * @param primeMatrix	 素数矩阵，用于辅助计算
	 * @param coveredColumns 被覆盖的列标记
	 * @param coveredRows	 被覆盖的行标记
	 * @param nOfRows		 行数
	 * @param nOfColumns	 列数
	 * @param minDim		 行数和列数的最小值
     * @return None
     */
	void step3(int *assignment, double *distMatrix, bool *starMatrix, bool *newStarMatrix, bool *primeMatrix, bool *coveredColumns, bool *coveredRows, int nOfRows, int nOfColumns, int minDim);
	
	/**
     * @brief 执行步骤4，根据当前处理的位置(row, col)，对星号和素数矩阵进行相应操作
	 * @param assignment	 当前的分配方案
	 * @param distMatrix	 表示检测框和跟踪框之间的代价/距离矩阵
	 * @param starMatrix	 星标矩阵，表示已经分配的分配对
	 * @param newStarMatrix	 新的星标矩阵，用于更新星标矩阵
	 * @param primeMatrix	 素数矩阵，用于辅助计算
	 * @param coveredColumns 被覆盖的列标记
	 * @param coveredRows	 被覆盖的行标记
	 * @param nOfRows		 行数
	 * @param nOfColumns	 列数
	 * @param minDim		 行数和列数的最小值
	 * @param row			 当前处理的行索引
	 * @param col			 当前处理的列索引
     * @return None
     */
	void step4(int *assignment, double *distMatrix, bool *starMatrix, bool *newStarMatrix, bool *primeMatrix, bool *coveredColumns, bool *coveredRows, int nOfRows, int nOfColumns, int minDim, int row, int col);
	
	/**
     * @brief 执行步骤5，根据当前矩阵状态进行相应操作，直到结束或找到无覆盖0元素
	 * @param assignment	 当前的分配方案
	 * @param distMatrix	 表示检测框和跟踪框之间的代价/距离矩阵
	 * @param starMatrix	 星标矩阵，表示已经分配的分配对
	 * @param newStarMatrix	 新的星标矩阵，用于更新星标矩阵
	 * @param primeMatrix	 素数矩阵，用于辅助计算
	 * @param coveredColumns 被覆盖的列标记
	 * @param coveredRows	 被覆盖的行标记
	 * @param nOfRows		 行数
	 * @param nOfColumns	 列数
	 * @param minDim		 行数和列数的最小值
     * @return None
     */
	void step5(int *assignment, double *distMatrix, bool *starMatrix, bool *newStarMatrix, bool *primeMatrix, bool *coveredColumns, bool *coveredRows, int nOfRows, int nOfColumns, int minDim);
};
#endif