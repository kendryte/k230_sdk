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
#include<ctime>
#include<iostream>

#include"rasterize.h"

void _get_point_weight(float *weight, Point p, Point p0, Point p1, Point p2) 
{
    // vectors
    Point v0, v1, v2;
    v0 = p2 - p0;
    v1 = p1 - p0;
    v2 = p - p0;

    // dot products
    float dot00 = v0.dot(v0); //v0.x * v0.x + v0.y * v0.y //np.dot(v0.T, v0)
    float dot01 = v0.dot(v1); //v0.x * v1.x + v0.y * v1.y //np.dot(v0.T, v1)
    float dot02 = v0.dot(v2); //v0.x * v2.x + v0.y * v2.y //np.dot(v0.T, v2)
    float dot11 = v1.dot(v1); //v1.x * v1.x + v1.y * v1.y //np.dot(v1.T, v1)
    float dot12 = v1.dot(v2); //v1.x * v2.x + v1.y * v2.y//np.dot(v1.T, v2)

    // barycentric coordinates
    float inverDeno;
    if (dot00 * dot11 - dot01 * dot01 == 0)
        inverDeno = 0;
    else
        inverDeno = 1 / (dot00 * dot11 - dot01 * dot01);

    float u = (dot11 * dot02 - dot01 * dot12) * inverDeno;
    float v = (dot00 * dot12 - dot01 * dot02) * inverDeno;

    // weight
    weight[0] = 1 - u - v;
    weight[1] = v;
    weight[2] = u;
}

void _rasterize(
	unsigned char *image, float *vertices, int *triangles, float *colors, float *depth_buffer,
	int ntri, int h, int w, int c, float alpha, bool reverse) {
	int x, y, k;
	int tri_p0_ind, tri_p1_ind, tri_p2_ind;
	Point p0, p1, p2, p;
	int x_min, x_max, y_min, y_max;
	float p_depth, p0_depth, p1_depth, p2_depth;
	float p_color, p0_color, p1_color, p2_color;
	float weight[3];

	for (int i = 0; i < ntri; i++) {
		tri_p0_ind = triangles[3 * i];
		tri_p1_ind = triangles[3 * i + 1];
		tri_p2_ind = triangles[3 * i + 2];

		p0.x = vertices[3 * tri_p0_ind];
		p0.y = vertices[3 * tri_p0_ind + 1];
		p0_depth = vertices[3 * tri_p0_ind + 2];
		p1.x = vertices[3 * tri_p1_ind];
		p1.y = vertices[3 * tri_p1_ind + 1];
		p1_depth = vertices[3 * tri_p1_ind + 2];
		p2.x = vertices[3 * tri_p2_ind];
		p2.y = vertices[3 * tri_p2_ind + 1];
		p2_depth = vertices[3 * tri_p2_ind + 2];

		x_min = max((int)ceil(min(p0.x, min(p1.x, p2.x))), 0);
		x_max = min((int)floor(max(p0.x, max(p1.x, p2.x))), w - 1);

		y_min = max((int)ceil(min(p0.y, min(p1.y, p2.y))), 0);
		y_max = min((int)floor(max(p0.y, max(p1.y, p2.y))), h - 1);

		if (x_max < x_min || y_max < y_min) {
			continue;
		}

		for (y = y_min; y <= y_max; y++) {
			for (x = x_min; x <= x_max; x++) {
				p.x = float(x);
				p.y = float(y);

				// call get_point_weight function once
				_get_point_weight(weight, p, p0, p1, p2);

				// and judge is_point_in_tri by below line of code
				if (weight[2] > 0 && weight[1] > 0 && weight[0] > 0) {
					_get_point_weight(weight, p, p0, p1, p2);
					p_depth = weight[0] * p0_depth + weight[1] * p1_depth + weight[2] * p2_depth;

					if ((p_depth > depth_buffer[y * w + x])) {
						//for (k = 0; k < c; k++) {
						for (k = 0; k < 3; k++) {
							p0_color = colors[3 * tri_p0_ind + k];
							p1_color = colors[3 * tri_p1_ind + k];
							p2_color = colors[3 * tri_p2_ind + k];

							p_color = weight[0] * p0_color + weight[1] * p1_color + weight[2] * p2_color;
							if (reverse) {
								if (c == 3)    //bgr
								{
									image[(h - 1 - y) * w * c + x * c + k] = (unsigned char)(
										(1 - alpha) * image[(h - 1 - y) * w * c + x * c + k] + alpha * 255 * p_color);
									//                                image[(h - 1 - y) * w * c + x * c + k] = (unsigned char) (255 * p_color);
								}
								else          //c==4 ,argb
								{
									image[(h - 1 - y) * w * c + x * c + k + 1] = (unsigned char)(
										(1 - alpha) * image[(h - 1 - y) * w * c + x * c + k + 1] + alpha * 255 * p_color);
								}

							}
							else {
								if (c == 3)  //bgr
								{
									image[y * w * c + x * c + k] = (unsigned char)(
										(1 - alpha) * image[y * w * c + x * c + k] + alpha * 255 * p_color);

								}
								else			//c==4 ,argb
								{
									image[y * w * c + x * c] = 255;
									image[y * w * c + x * c + k + 1] = (unsigned char)(
										(1 - alpha) * image[y * w * c + x * c + k] + alpha * 255 * p_color);
								}

								
							}
						}

						depth_buffer[y * w + x] = p_depth;
					}
				}
			}
		}
	}
}