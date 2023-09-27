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

#ifndef _UTIL_
#define _UTIL_

#include <atomic>
#include <vector>
#include <chrono>
#include <string>
#include <thread>
#include <iostream>
#include <mutex>
#include <queue>
#include <condition_variable>

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
        if (m_start_timer)
            m_start_timer = 0;
    }

	~ScopedTiming() { }

private:
	int m_start_timer;
	std::string m_info;
	std::chrono::steady_clock::time_point m_start;
	std::chrono::steady_clock::time_point m_stop;
};

template <typename T>
class BufThreadQueue {
  public:
    explicit BufThreadQueue(size_t max_size) : max_size_(max_size) {}

    bool put(T& in_res, int tiemout = 300) {
        std::unique_lock<std::mutex> locker(queue_mutex_);
        full_cv_.wait_for(locker, std::chrono::milliseconds(tiemout), [this] {return queue_.size() < max_size_;});
        if (queue_.size() >= max_size_) return false;

        queue_.push(in_res);

        empty_cv_.notify_one();
        return true;
    }

    bool take(T& out_res, int timeout = 300) {
        std::unique_lock<std::mutex> locker(queue_mutex_);
        empty_cv_.wait_for(locker, std::chrono::milliseconds(timeout), [this] {return !queue_.empty();});
        if (queue_.empty()) return false;

        out_res = queue_.front();
        queue_.pop();

        full_cv_.notify_one();
        return true;
    }

    bool empty() {
        std::unique_lock<std::mutex> locker(queue_mutex_);
        return queue_.empty();
    }

    size_t size() {
        std::unique_lock<std::mutex> locker(queue_mutex_);
        return queue_.size();
    }

  private:
    std::mutex queue_mutex_;
    size_t max_size_;
    std::condition_variable empty_cv_;
    std::condition_variable full_cv_;
    std::queue<T> queue_;
};

#endif  // _UTIL_