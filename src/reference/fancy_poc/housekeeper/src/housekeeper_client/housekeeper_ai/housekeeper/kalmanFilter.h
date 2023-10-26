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
 *	Part of the code is referenced from https://github.com/1079863482/yolov8_ByteTrack_TensorRT
 *
 */

#ifndef KALMANFILTER_H
#define KALMANFILTER_H

#include "dataType.h"

namespace byte_kalman
{
	class KalmanFilter
	{
	public:
		static const double chi2inv95[10];
		KalmanFilter();
		KAL_DATA initiate(const DETECTBOX& measurement);
		void predict(KAL_MEAN& mean, KAL_COVA& covariance);
		KAL_HDATA project(const KAL_MEAN& mean, const KAL_COVA& covariance);
		KAL_DATA update(const KAL_MEAN& mean,
			const KAL_COVA& covariance,
			const DETECTBOX& measurement);

		Eigen::Matrix<float, 1, -1> gating_distance(
			const KAL_MEAN& mean,
			const KAL_COVA& covariance,
			const std::vector<DETECTBOX>& measurements,
			bool only_position = false);

	private:
		Eigen::Matrix<float, 8, 8, Eigen::RowMajor> _motion_mat;
		Eigen::Matrix<float, 4, 8, Eigen::RowMajor> _update_mat;
		float _std_weight_position;
		float _std_weight_velocity;
	};
}

#endif