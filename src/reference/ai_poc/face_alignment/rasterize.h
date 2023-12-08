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

#ifndef _RASTERIZE_H
#define _RASTERIZE_H

#include <algorithm>
#include <cmath>

using std::max;
using std::min;

class Point
{
public:
    float x;
    float y;

public:
    Point() : x(0.f), y(0.f) {}
    Point(float x_, float y_) : x(x_), y(y_) {}
    float dot(Point p)
    {
        return this->x * p.x + this->y * p.y;
    }

    Point operator-(const Point &p)
    {
        Point np;
        np.x = this->x - p.x;
        np.y = this->y - p.y;
        return np;
    }

    Point operator+(const Point &p)
    {
        Point np;
        np.x = this->x + p.x;
        np.y = this->y + p.y;
        return np;
    }

    Point operator*(float s)
    {
        Point np;
        np.x = s * this->x;
        np.y = s * this->y;
        return np;
    }
};

void _get_point_weight(float *weight, Point p, Point p0, Point p1, Point p2);
void _rasterize(unsigned char *image, float *vertices, int *triangles, float *colors, float *depth_buffer,
                int ntri, int h, int w, int c, float alpha, bool reverse);
#endif
