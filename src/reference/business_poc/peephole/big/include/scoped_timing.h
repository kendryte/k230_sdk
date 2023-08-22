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

#ifndef _SCOPED_TIMING_H_
#define _SCOPED_TIMING_H_

#include <chrono>
#include <string>
#include <iostream>

class ScopedTiming {
public:
	ScopedTiming(std::string info = "ScopedTiming", int start_timer = 0)
		: m_info(info), m_start_timer(start_timer) {

	}

    void CheckStartTimer() {
        if (!m_start_timer) {
            m_start = std::chrono::steady_clock::now();
            m_start_timer = 1;
        }
    }

    int ElapsedSeconds() {
        if (!m_start_timer) return -1;
        m_stop = std::chrono::steady_clock::now();
        std::chrono::duration<double> time_duration = m_stop - m_start;
        return time_duration.count();
    }

    void TimerStop() {
        m_start_timer = 0;
    }

	~ScopedTiming() { }

private:
	int m_start_timer;
	std::string m_info;
	std::chrono::steady_clock::time_point m_start;
	std::chrono::steady_clock::time_point m_stop;
};

#endif  // _SCOPED_TIMING_H_