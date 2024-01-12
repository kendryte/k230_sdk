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

#include <iostream>
#include <opencv2/core.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/imgproc.hpp>
#include <stdint.h>

using namespace std;
using namespace cv;

#define CONTEXT_AMOUNT 0.5  // rect_size 的宽、高调整系数
#define EXEMPLAR_SIZE 127  // crop 模型输入尺寸
#define INSTANCE_SIZE 255  // src 模型输入尺寸
#define MARGIN  0.4  //   目标框缩放调整系数   


void init();  // 初始化，内部调用了 set_han 和 set_points 两个函数

void set_han();  // 为 featuremap 设置 hanning 值，越往中心，权重值越大

float* get_center(Mat image); // 根据图片，获取图片中心

void set_points(); // 为 featuremap 初始设值

float* get_position(Mat image); // 获取目标初始位置，并更新rect_size

float* get_updated_position();  // 获取目标每次更新之后的位置

Mat sub_window(Mat img, int size, float len); // 界定目标的搜索范围，一般是上一帧目标的外扩范围

void softmax(float* score); // 将score归一化至 0-1

float* convert_bbox(float* box); // box坐标转化，由 <x1,y1,w,h> --> <center_x,center_y,w,h>

float* convert_score(float* score);  // 借助 softmax函数，将score归一化

void corner2center(float& x, float& y, float& w, float& h);  // <x1,y1,w,h> --> <center_x,center_y,w,h>

float change(float r); // 值调整 

float sz(float w, float h); // 对 w、h进行开根号处理

void bbox_clip(float& x, float& y, float& w, float& h, int cols, int rows); // box 裁剪，防止越界

int max_index(float* s); // 获取得分最大的索引

Mat crop_img(Mat image, Rect rect); // 图片裁剪

void track_post_process(float* score, float* box, int cols, int rows, int& box_x, int& box_y, int& box_w, int& box_h, float& best_score); // head 模型后处理

void set_rect_size(float w, float h);

void set_center(float x, float y);

#endif
