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
#include <chrono>
#include <string>
#include <iostream>

/**
 * @brief 计时类
 * 统计在该类实例生命周期内的耗时
 */
class ScopedTiming
{
public:
	/**
	 * @brief ScopedTiming构造函数,初始化计时对象名称并开始计时
	 * @param info 			 计时对象名称
	 * @param enable_profile 是否开始计时
	 * @return None
	 */
	ScopedTiming(std::string info = "ScopedTiming", int enable_profile = 1)
		: m_info(info), enable_profile(enable_profile)
	{
		if (enable_profile)
		{
			m_start = std::chrono::steady_clock::now();
		}
	}

	/**
	 * @brief ScopedTiming析构,结束计时，并打印耗时
	 * @return None
	 */
	~ScopedTiming()
	{
		if (enable_profile)
		{
			m_stop = std::chrono::steady_clock::now();
			double elapsed_ms = std::chrono::duration<double, std::milli>(m_stop - m_start).count();
			std::cout << m_info << " took " << elapsed_ms << " ms" << std::endl;
		}
	}

private:
	int enable_profile;							   // 是否统计时间
	std::string m_info;							   // 计时对象名称
	std::chrono::steady_clock::time_point m_start; // 计时开始时间
	std::chrono::steady_clock::time_point m_stop;  // 计时结束时间
};