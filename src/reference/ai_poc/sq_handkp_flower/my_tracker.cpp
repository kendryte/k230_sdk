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
#include "my_tracker.h"

int MyTracker::kf_count = 0;

void MyTracker::init_kf(StateType stateMat)
{
	statePost = Mat::zeros(4, 1, CV_32F);
	statePost.at<float>(0, 0) = stateMat.x + stateMat.width / 2;
	statePost.at<float>(1, 0) = stateMat.y + stateMat.height / 2;
	statePost.at<float>(2, 0) = stateMat.area();
	statePost.at<float>(3, 0) = stateMat.width / stateMat.height;
}


// Predict the estimated bounding box.
StateType MyTracker::predict()
{
	m_age += 1;

	if (m_time_since_update > 0)
		m_hit_streak = 0;
	m_time_since_update += 1;

	StateType predictBox = get_rect_xysr(statePost.at<float>(0, 0), statePost.at<float>(1, 0), statePost.at<float>(2, 0), statePost.at<float>(3, 0));

	m_history.push_back(predictBox);
	return m_history.back();
}


// Update the state vector with observed bounding box.
void MyTracker::update(StateType stateMat)
{
	m_time_since_update = 0;
	m_history.clear();
	m_hits += 1;
	m_hit_streak += 1;

	statePost.at<float>(0, 0) = stateMat.x + stateMat.width / 2;
	statePost.at<float>(1, 0) = stateMat.y + stateMat.height / 2;
	statePost.at<float>(2, 0) = stateMat.area();
	statePost.at<float>(3, 0) = stateMat.width / stateMat.height;
}

// Return the current state vector
StateType MyTracker::get_state()
{
	return get_rect_xysr(statePost.at<float>(0, 0), statePost.at<float>(1, 0), statePost.at<float>(2, 0), statePost.at<float>(3, 0));
}

// Convert bounding box from [cx,cy,s,r] to [x,y,w,h] style.
StateType MyTracker::get_rect_xysr(float cx, float cy, float s, float r)
{
	float w = sqrt(s * r);
	float h = s / w;
	float x = (cx - w / 2);
	float y = (cy - h / 2);

	if (x < 0 && cx > 0)
		x = 0;
	if (y < 0 && cy > 0)
		y = 0;

	return StateType(x, y, w, h);
}
