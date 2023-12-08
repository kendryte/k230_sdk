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
#ifndef MY_TRACKER_H
#define MY_TRACKER_H

// #include "opencv2/video/tracking.hpp"
#include "opencv2/highgui/highgui.hpp"

using namespace std;
using namespace cv;

#define StateType Rect_<float>


// This class represents the internel state of individual tracked objects observed as bounding box.
/**
 * @brief 跟踪器
 * 主要为每一个检测框初始化跟踪器并维护
 */
class MyTracker
{
public:

	/**
     * @brief MyTracker构造函数
     * @return None
     */
	MyTracker()
	{
		init_kf(StateType());	 // 使用默认状态初始化滤波器
		m_time_since_update = 0; // 自上次更新以来的时间
		m_hits = 0;				 // 成功匹配次数
		m_hit_streak = 0;		 // 连续成功匹配次数
		m_age = 0;				 // 目标跟踪的帧数
		m_id = kf_count;		 // 目标跟踪器的唯一标识符
	}

	/**
     * @brief MyTracker带参数构造函数
     * @return None
     */
	MyTracker(StateType initRect)
	{
		init_kf(initRect);		 // 使用给定的初始状态初始化卡尔曼滤波器
		m_time_since_update = 0; // 自上次更新以来的时间
		m_hits = 0;				 // 成功匹配次数
		m_hit_streak = 0;		 // 连续成功匹配次数
		m_age = 0;				 // 目标跟踪的帧数
		m_id = kf_count;		 // 目标跟踪器的唯一标识符
		kf_count++;				 
	}

	/**
     * @brief  MyTracker析构函数
     * @return None
     */
	~MyTracker()
	{
		m_history.clear();
	}

	/**
     * @brief  预测目标的状态
     * @return 返回预测目标的状态
     */
	StateType predict();

	/**
     * @brief  更新目标的状态
     * @return None
     */
	void update(StateType stateMat);
	
	/**
     * @brief  获取当前目标的状态
     * @return 返回当前目标的状态
     */
	StateType get_state();

	/**
     * @brief  根据中心点坐标、尺度因子和旋转角度获取状态矩阵
     * @return 返回目标的状态
     */
	StateType get_rect_xysr(float cx, float cy, float s, float r);

	static int kf_count;	 // 统计目标跟踪器的数量
	int m_time_since_update; // 自上次更新以来的时间
	int m_hits;				 // 成功匹配次数
	int m_hit_streak;		 // 连续成功匹配次数
	int m_age;				 // 目标追踪的帧数
	int m_id;				 // 目标追踪器的唯一标识符

private:

	/**
     * @brief  // 初始化滤波器
     * @return None
     */
	void init_kf(StateType stateMat);

	cv::Mat statePost;				  // 跟踪器的状态向量
	std::vector<StateType> m_history; // 目标状态的历史记录
};




#endif