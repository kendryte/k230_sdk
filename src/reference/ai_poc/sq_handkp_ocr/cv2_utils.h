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
#ifndef _CV2_UTILS
#define _CV2_UTILS

#include <fstream>
#include <iostream>
#include <chrono>
#include <opencv2/core.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/imgproc.hpp>
#include <vector>
#include "utils.h"
#include <map>

#define SHARE_MEMORY_ALLOC          _IOWR('m', 1, unsigned long)
#define SHARE_MEMORY_ALIGN_ALLOC    _IOWR('m', 2, unsigned long)
#define SHARE_MEMORY_FREE           _IOWR('m', 3, unsigned long)
#define SHARE_MEMORY_SHOW           _IOWR('m', 4, unsigned long)
#define SHARE_MEMORY_INVAL_RANGE    _IOWR('m', 5, unsigned long)
#define SHARE_MEMORY_WB_RANGE       _IOWR('m', 6, unsigned long)
#define MEMORY_TEST_BLOCK_SIZE      4096        /* ����������ڴ�ռ��С */
#define MEMORY_TEST_BLOCK_ALIGN     4096        /* �����Ҫmmapӳ��,������Ҫ4K�������� */
#define SHARE_MEMORY_DEV            "/dev/k510-share-memory"
#define MAP_MEMORY_DEV              "/dev/mem"


#define PADDING_R 114
#define PADDING_G 114
#define PADDING_B 114

#define STAGE_NUM 3
#define STRIDE_NUM 3
#define LABELS_NUM 1
#define ANCHORS_NUM 3

using namespace std;
using namespace cv;

cv::Mat ReSize(cv::Mat& dst, cv::Mat image, int w, int h, int& dw, int& dh);
float fast_exp(float x);
float sigmoid(float x);
template <class T>
void draw_detections(cv::Mat& frame, vector<BoxInfo>& detections,vector<string> labels);
void draw_detections(cv::Mat& frame, vector<Boxb>& detections);

std::vector<size_t> sort_indices(const std::vector<Point2f>& vec);
void find_rectangle_vertices(const std::vector<Point2f>& points, Point2f& topLeft, Point2f& topRight, Point2f& bottomRight, Point2f& bottomLeft);
void warppersp(cv::Mat src, cv::Mat& dst, Boxb b);

#endif
