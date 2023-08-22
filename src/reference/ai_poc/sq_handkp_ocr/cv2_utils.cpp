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
#include <algorithm>
#include <cmath>
#include <typeinfo>
#include "cv2_utils.h"
#include <iostream>
#include <string>
#include <fstream>

float fast_exp(float x)
{
    union {
        uint32_t i;
        float f;
    } v{};
    v.i = (1 << 23) * (1.4426950409 * x + 126.93490512f);
    return v.f;
}

float sigmoid(float x)
{
    return 1.0f / (1.0f + fast_exp(-x));
    //return 1.0f / (1.0f + exp(-x));
}

cv::Mat ReSize(cv::Mat& dst, cv::Mat image, int w, int h, int& dw1, int& dh1) {
    int row = image.rows;
    int col = image.cols;
    float ratio_w = 1.0 * w / col;
    float ratio_h = 1.0 * h / row;
    float ratio = min(ratio_h, ratio_w);
    int unpad_w = (int)round(col * ratio);
    int unpad_h = (int)round(row * ratio);

    float dw = (1.0 * w - unpad_w);
    float dh = (1.0 * h - unpad_h);
    

    int top = int(round((dh - 0.1) / 2));
    int bottom = int(round((dh + 0.1) / 2));
    int left = int(round((dw - 0.1) / 2 ));
    int right = int(round((dw + 0.1) / 2));

    // dw1 = dw /2;
    // dh1 = dh /2;

    dw1 = top;
    dh1 = left;
    // cout << "top = " << top << "   left = " << left << endl;

    cv::Mat tmp;
    cv::resize(image, tmp, cv::Size(unpad_w, unpad_h));
    cv::copyMakeBorder(tmp, dst, top, bottom, left, right, cv::BORDER_CONSTANT);
    // imwrite("resize.jpg", dst);
    return dst;
}

void draw_detections(cv::Mat& frame, vector<BoxInfo>& detections,vector<string> labels)
{
    int thickness = 2;
    for (unsigned int k = 0; k < detections.size(); k++)
    {
        int x = detections[k].x1, y = detections[k].y1;
        int w = detections[k].x2 - detections[k].x1, h = detections[k].y2 - detections[k].y1;
        cv::Rect rect(x, y, w, h);

        cv::rectangle(frame, rect, cv::Scalar(0, 0, 255), thickness);

        cv::Point left_top;
        left_top.x = x;
        left_top.y = y + 25;
        std::string text = labels[detections[k].label] + ":" + std::to_string(round(detections[k].score * 100) / 100.0).substr(0, 4);
        std::cout << "text = " << text << std::endl; 
        cv::putText(frame,text, cv::Point(left_top.x, left_top.y - 5),
            cv::FONT_HERSHEY_SIMPLEX, 0.8, cv::Scalar(255, 255, 0), 2);
    }
}

void draw_detections(cv::Mat& frame, vector<Boxb>& detections)
{
    for(int i = 0; i < detections.size(); i++)
    {   
        vector<Point> vec;
        vec.clear();
        for(int j = 0; j < 4; j++)
        {
            vec.push_back(detections[i].vertices[j]);
        }
        RotatedRect rect = minAreaRect(vec);
        Point2f ver[4];
        rect.points(ver);
        for(int i = 0; i < 4; i++)
            line(frame, ver[i], ver[(i + 1) % 4], Scalar(255, 0, 0), 3);
        std::string text = "score:" + std::to_string(round(detections[i].score * 100) / 100.0).substr(0, 4);

        cv::putText(frame, text, cv::Point(detections[i].meanx, detections[i].meany), 
            cv::FONT_HERSHEY_SIMPLEX, 0.8, cv::Scalar(255, 255, 0), 2);
    }
}


std::vector<size_t> sort_indices(const std::vector<Point2f>& vec) 
{
	std::vector<std::pair<Point2f, size_t>> indexedVec;
	indexedVec.reserve(vec.size());

	// 创建带有索引的副本
	for (size_t i = 0; i < vec.size(); ++i) {
		indexedVec.emplace_back(vec[i], i);
	}

	// 按值对副本进行排序
	std::sort(indexedVec.begin(), indexedVec.end(),
		[](const auto& a, const auto& b) {
		return a.first.x < b.first.x;
	});

	// 提取排序后的索引
	std::vector<size_t> sortedIndices;
	sortedIndices.reserve(vec.size());
	for (const auto& element : indexedVec) {
		sortedIndices.push_back(element.second);
	}

	return sortedIndices;
}

void find_rectangle_vertices(const std::vector<Point2f>& points, Point2f& topLeft, Point2f& topRight, Point2f& bottomRight, Point2f& bottomLeft) 
{
    //先按照x排序,比较左右，再按照y比较上下
	auto sorted_x_id = sort_indices(points);

	if (points[sorted_x_id[0]].y < points[sorted_x_id[1]].y)
	{
		topLeft = points[sorted_x_id[0]];
		bottomLeft = points[sorted_x_id[1]];
	}
	else
	{
		topLeft = points[sorted_x_id[1]];
		bottomLeft = points[sorted_x_id[0]];
	}

	if (points[sorted_x_id[2]].y < points[sorted_x_id[3]].y)
	{
        bottomRight = points[sorted_x_id[3]];
		topRight = points[sorted_x_id[2]];

	}
	else
	{ 
        bottomRight = points[sorted_x_id[2]];
		topRight = points[sorted_x_id[3]];
	}
	
}

void warppersp(cv::Mat src, cv::Mat& dst, Boxb b)
{
    Mat rotation;
    vector<Point> con;
    for(auto i : b.ver_src)
        con.push_back(i);

    RotatedRect minrect = minAreaRect(con);
    std::vector<Point2f> vtx(4),vtd(4),vt(4);
    minrect.points(vtx.data());

    find_rectangle_vertices(vtx, vtd[0], vtd[1], vtd[2], vtd[3]);
    
    //w,h tmp_w=dist(p1,p0),tmp_h=dist(p1,p2)
    float tmp_w = cv::norm(vtd[1]-vtd[0]);
    float tmp_h = cv::norm(vtd[2]-vtd[1]);
    float w = std::max(tmp_w,tmp_h);
    float h = std::min(tmp_w,tmp_h);

    std::cout << vtd[0].x << vtd[0].y << endl;
    std::cout << vtd[1].x << vtd[1].y << endl;
    std::cout << vtd[2].x << vtd[2].y << endl;
    std::cout << vtd[3].x << vtd[3].y << endl;

    vt[0].x = 0;
    vt[0].y = 0;
    vt[1].x = w;//w
    vt[1].y = 0;
    vt[2].x = w;
    vt[2].y = h;
    vt[3].x = 0;
    vt[3].y = h;//h
    rotation = getPerspectiveTransform(vtd, vt);

    warpPerspective(src, dst, rotation, Size(w, h));

}

// void warppersp(cv::Mat src, cv::Mat& dst, Boxb b)
// {
//     Mat rotation;
//     vector<Point> con;
//     for(auto i : b.ver_src)
//         con.push_back(i);

//     RotatedRect minrect = minAreaRect(con);

//     Point2f vtx[4], vt[4];
//     minrect.points(vtx);

//     float xmin = 1.0 * src.cols, ymin = 1.0 * src.rows, xmax = 0., ymax = 0.;
//     for(auto i : vtx)
//     {
//         xmin = i.x < xmin ? i.x : xmin;
//         xmax = i.x > xmax ? i.x : xmax;
//         ymin = i.y < ymin ? i.y : ymin;
//         ymax = i.y > ymax ? i.y : ymax;
//     }
//     vt[0].x = 0;
//     vt[0].y = 0;
//     vt[1].x = xmax - xmin;
//     vt[1].y = 0;
//     vt[2].x = xmax - xmin;
//     vt[2].y = ymax - ymin;
//     vt[3].x = 0;
//     vt[3].y = ymax - ymin;
//     rotation = getPerspectiveTransform(vtx, vt);

//     warpPerspective(src, dst, rotation, Size(xmax-xmin, ymax-ymin));

// }

// template <class T>
// std::vector<T> read_binary_file(const char *file_name)
// {
//     std::ifstream ifs(file_name, std::ios::binary);
//     ifs.seekg(0, ifs.end);
//     size_t len = ifs.tellg();
//     std::vector<T> vec(len / sizeof(T), 0);
//     ifs.seekg(0, ifs.beg);
//     ifs.read(reinterpret_cast<char *>(vec.data()), len);
//     ifs.close();
//     return vec;
// }


