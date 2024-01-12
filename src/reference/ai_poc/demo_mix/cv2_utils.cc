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

#include "cv2_utils.h"

#include <math.h>
#include <cmath>
#include <vector>

#define ORI_WID (720.0)   // 视频分辨率之 宽度
#define ORI_HEI (1280.0)  // 视频分辨率之 高度
#define WID ORI_WID * (1 - 2 * MARGIN) // 初始框之宽度，取视频分辨率的短边
#define HEI WID  // 初始框之高度
#define CEN_X ORI_WID * 0.5  // 图像中心之 X 坐标
#define CEN_Y ORI_HEI * 0.5  // 图像中心之 Y 坐标
#define OUTPUT_1_SIZE 2  // 两个输出，与points变量联系

#define WINDOW_INFLUENCE 0.46  // 框作用范围系数
#define LR  0.34  // rect_size 调整系数

float center[] = {CEN_X, CEN_Y}; // 中心坐标

#define OUTPUT_GRID 16  
#define OUTPUT_GRID_SIZE 256 
float window[OUTPUT_GRID_SIZE];
float points[OUTPUT_GRID_SIZE][OUTPUT_1_SIZE];

float hhanning[] = { 0., 0.04322727, 0.1654347, 0.3454915, 0.55226423, 0.75, 0.9045085, 0.9890738,
               0.9890738 , 0.9045085, 0.75, 0.55226423, 0.3454915, 0.1654347, 0.04322727, 0. }; // 汉宁窗取值

float rect_size[] = {WID, HEI}; // 初始化 rect_size

#define PENALTY_K 0.16 // pscore调整系数

Mat sub_window(Mat img, int size, float len)
{

	Scalar meanValues = mean(img);
	float s_z = len;
	float c = (s_z + 1) / 2;
	int context_xmin = floor(center[0] - c + 0.5);
	int context_xmax = int(context_xmin + s_z - 1);
	int context_ymin = floor(center[1] - c + 0.5);
	int context_ymax = int(context_ymin + s_z - 1);
	int left_pad = int(max(0, -context_xmin));
	int top_pad = int(max(0, -context_ymin));
	int right_pad = int(max(0, context_xmax - img.cols + 1));
	int bottom_pad = int(max(0, context_ymax - img.rows + 1));
	context_xmin = context_xmin + left_pad;
	context_xmax = context_xmax + left_pad;
	context_ymin = context_ymin + top_pad;
	context_ymax = context_ymax + top_pad;
	Mat result;
	if (left_pad != 0 || right_pad != 0 || top_pad != 0 || bottom_pad != 0)
	{
		int r = img.rows + top_pad + bottom_pad;
		int c = img.cols + left_pad + right_pad;
		int k = img.channels();
		Mat tmp(r, c, CV_8UC3, meanValues);
		Rect roi(left_pad, top_pad, img.cols, img.rows);
		img.copyTo(tmp(roi));
		Rect roi1(context_xmin, context_ymin, context_xmax - context_xmin + 1, context_ymax - context_ymin + 1);
		Mat patch = tmp(roi1);
		resize(patch, result, Size(size, size));
		return result;
	}
	else
	{
		Rect roi1(context_xmin, context_ymin, context_xmax - context_xmin + 1, context_ymax - context_ymin + 1);
		Mat patch = img(roi1);
		resize(patch, result, Size(size, size));
		return result;
	}
	return result;
}

void set_han()
{
	int i, j;
	for (i = 0; i < OUTPUT_GRID; i++)
		for (j = 0; j < OUTPUT_GRID; j++)
			window[i * OUTPUT_GRID + j] = hhanning[i] * hhanning[j];

}

void set_points()
{
	int i, j;
	for (i = 0; i < OUTPUT_GRID; i++)
	{
		float x = -128.0;
		for (j = 0; j < OUTPUT_GRID; j++)
		{
			points[i * OUTPUT_GRID + j][0] = x;
			x += OUTPUT_GRID;
		}	
	}
	float y = -128.0;
	for (i = 0; i < OUTPUT_GRID; i++)
	{
		for (j = 0; j < OUTPUT_GRID; j++)
			points[i * OUTPUT_GRID + j][1] = y;
		y += OUTPUT_GRID;
	}
}

Mat crop_img(Mat image, Rect rect)
{
	Mat cropped_img = image(rect);
	return cropped_img;
}

float* get_position(Mat image)
{
	rect_size[0] = rect_size[0] / ORI_WID * image.cols;
	rect_size[1] = rect_size[1] / ORI_HEI * image.rows;
	return rect_size;
}

float* get_center(Mat image)
{
	center[0] = 500;
	center[1] = 250;
	return center;
}

float* get_updated_position()
{
	return rect_size;
}

void init()
{
	set_han();
	set_points();
}

float* convert_bbox(float* box)
{
	int i;
	for (i = 0; i < OUTPUT_GRID_SIZE; i++)
	{
		box[i] = points[i][0] - box[i];
		box[OUTPUT_GRID_SIZE + i] 	 = points[i][1] - box[OUTPUT_GRID_SIZE + i];
		box[OUTPUT_GRID_SIZE * 2 + i] = points[i][0] + box[OUTPUT_GRID_SIZE * 2 + i];
		box[OUTPUT_GRID_SIZE * 3 + i] = points[i][1] + box[OUTPUT_GRID_SIZE * 3 + i];
		corner2center(box[i], box[OUTPUT_GRID_SIZE + i], box[OUTPUT_GRID_SIZE * 2 + i], box[OUTPUT_GRID_SIZE * 3 + i]);
	}		

	return box;
}

float* convert_score(float* score)
{
	softmax(score);
	return score;
}

void softmax(float* score)
{
	float sum0 = 0.0, sum1 = 0.0;
	for (int i = 0; i < OUTPUT_GRID_SIZE; i++)
		sum1 += exp(score[OUTPUT_GRID_SIZE + i]);
	for (int i = 0; i < OUTPUT_GRID_SIZE; i++)
		score[i + OUTPUT_GRID_SIZE] = exp(score[OUTPUT_GRID_SIZE + i]) / sum1;
}

void corner2center(float& x, float& y, float& w, float& h)
{
	float x1 = x;
	float y1 = y;
	float x2 = w;
	float y2 = h;
	x = (x1 + x2) * 0.5;
	y = (y1 + y2) * 0.5;
	w = x2 - x1;
	h = y2 - y1;
}

float change(float r)
{
	return max(r, (float)(1.0 / r));
}

float sz(float w, float h)
{
	float pad = (w + h) * 0.5;
	return sqrt((w + pad) * (h * pad));
}

int max_index(float* s)
{
	int index = 0;
	float max = s[0];
	for (int i = 1; i < OUTPUT_GRID_SIZE; i++)
		if (max < s[i])
		{
			max = s[i];
			index = i;
		}
	return index;
}

void bbox_clip(float& x, float& y, float& w, float& h, int cols, int rows)
{
	float cx = x, cy = y, cw = w, ch = h;
	x = max((float)0.0,  (float)min(cx, (float)(cols * 1.0)));
	y = max((float)0.0,  (float)min(cy, (float)(rows * 1.0)));
	w = max((float)10.0, (float)min(cw, (float)(cols * 1.0)));
	h = max((float)10.0, (float)min(ch, (float)(rows * 1.0)));
}

void track_post_process(float* score, float* box, int cols,int rows, int& box_x, int& box_y, int& box_w, int& box_h, float& best_score)
{

	float w_z = rect_size[0] + CONTEXT_AMOUNT * (rect_size[0] + rect_size[1]);
	float h_z = rect_size[1] + CONTEXT_AMOUNT * (rect_size[0] + rect_size[1]);
	float s_z = sqrt(w_z * h_z);
	float scale_z = EXEMPLAR_SIZE / s_z;
	score = convert_score(score);
	box = convert_bbox(box);
	float w[OUTPUT_GRID_SIZE];
	float h[OUTPUT_GRID_SIZE];
	float sc[OUTPUT_GRID_SIZE], rc[OUTPUT_GRID_SIZE];
	float penalty[OUTPUT_GRID_SIZE], pscore[OUTPUT_GRID_SIZE];
	for (int i = 0; i < OUTPUT_GRID_SIZE; i++) 
	{
		w[i] = box[OUTPUT_GRID_SIZE * 2 + i];
		h[i] = box[OUTPUT_GRID_SIZE * 3 + i];
	}

	for (int i = 0; i < OUTPUT_GRID_SIZE; i++)
	{
		float tmps = sz(w[i], h[i]) / sz(rect_size[0] * scale_z, rect_size[1] * scale_z);
		sc[i] = change(tmps);
		float tmpr = (rect_size[0] / rect_size[1]) / (w[i] / h[i]);
		rc[i] = change(tmpr);
		penalty[i] = exp(-(rc[i] * sc[i] - 1) * PENALTY_K);
		pscore[i] = penalty[i] * score[OUTPUT_GRID_SIZE + i];
		pscore[i] = pscore[i] * (1 - WINDOW_INFLUENCE) + window[i] * WINDOW_INFLUENCE;

	}
	int best_index = max_index(pscore);
	float cx = box[best_index] / scale_z;
	float cy = box[best_index + OUTPUT_GRID_SIZE] / scale_z;
	float cw = box[best_index + OUTPUT_GRID_SIZE * 2] / scale_z;
	float ch = box[best_index + OUTPUT_GRID_SIZE * 3] / scale_z;
	float lr = penalty[best_index] * score[OUTPUT_GRID_SIZE + best_index] * LR;

	cx = cx + center[0];
	cy = cy + center[1];
	cw = rect_size[0] * (1 - lr) + cw * lr;
	ch = rect_size[1] * (1 - lr) + ch * lr;
	bbox_clip(cx, cy, cw, ch, cols, rows);
	center[0] = cx;
	center[1] = cy;
	rect_size[0] = cw;
	rect_size[1] = ch;
	best_score = score[OUTPUT_GRID_SIZE+best_index];
	box_x = max(0, int(cx - cw / 2));
	box_y = max(0, int(cy - ch / 2));
	box_w = int(cw);
	box_h = int(ch);
}

void set_rect_size(float w, float h)
{
	rect_size[0] = w;
	rect_size[1] = h;
}

void set_center(float x, float y)
{	
	center[0] = x;
	center[1] = y;
}