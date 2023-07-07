/*
 * Copyright (C) 2000-2019, Intel Corporation, all rights reserved.
 * Copyright (C) 2009-2011, Willow Garage Inc., all rights reserved.
 * Copyright (C) 2009-2016, NVIDIA Corporation, all rights reserved.
 * Copyright (C) 2010-2013, Advanced Micro Devices, Inc., all rights reserved.
 * Copyright (C) 2015-2016, OpenCV Foundation, all rights reserved.
 * Copyright (C) 2015-2016, Itseez Inc., all rights reserved.
 * Copyright (c) 2019-2020, VeriSilicon Holdings Co., Ltd, all rights reserved.
 * Third party copyrights are property of their respective owners.
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:

 * Redistributions of source code must retain the above copyright notice, this list
 * of conditions and the following disclaimer.
 * Redistributions in binary form must reproduce the above copyright notice, this list
 * of conditions and the following disclaimer in the documentation and/or other
 * materials provided with the distribution.
 * Neither the names of the copyright holders nor the names of the contributors may be
 * used to endorse or promote products derived from this software without specific
 * prior written permission.
 * This software is provided by the copyright holders and contributors "as is" and any
 * express or implied warranties, including, but not limited to, the implied warranties
 * of merchantability and fitness for a particular purpose are disclaimed. In no event
 * shall copyright holders or contributors be liable for any direct, indirect, incidental,
 * special, exemplary, or consequential damages (including, but not limited to, procurement
 * of substitute goods or services; loss of use, data, or profits; or business interruption)
 * however caused and on any theory of liability, whether in contract, strict liability, or
 * tort (including negligence or otherwise) arising in any way out of the use of this
 * software, even if advised of the possibility of such damage.
 */
 
#ifndef _DEWARP_CMODEL_BASIC_H_
#define _DEWARP_CMODEL_BASIC_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

#define VS_32F 1
#define VS_64F 2
#define MAP_16SC2 3
#define MAP_32FC1 4
#define MAP_32FC2 5

#define INTER_BITS 4
#define INTER_TAB_SIZE (1 << INTER_BITS)

#ifndef MIN
#define MIN(a, b) ((a) < (b)) ? (a) : (b)
#endif

#ifndef MAX
#define MAX(a, b) ((a) > (b)) ? (a) : (b)
#endif


#define MAP_BLOCK_WIDTH 16
#define MAP_BLOCK_HEIGHT 16
#define MAP_FRACTIONAL 4   // 4 bit fractional part
#define FLOAT_MAP 0
#define INT_MAP   1
#define RESIZE_FRACTIONAL 7 //7 bit fractional precision

#define RGB24_PLANER_DATA   0
#define YUV422_PLANER_DATA  1
#define YUV420_PLANER_DATA  2


typedef unsigned char uchar;
typedef uint16_t ushort;

enum decomEnum {
    VS_DECOMP_LU = 0,
    VS_DECOMP_SVD,
    VS_DECOMP_EIG,
    VS_DECOMP_CHOLESKY
};

enum DEWARPModel
{
	NORMAL_DEWARP = 0,
	FISHEYE_DEWARP,
	PERSPECTIVE_DEWARP,
	PTZ_DEWARP
};
typedef struct matrix
{
	int rows;
	int cols;
	int step;
	int type;
	void* data;
}VSMatrix;

double vsInvert(VSMatrix *_src, VSMatrix *_dst, int method);

#ifdef __cplusplus
}
#endif

#endif  // _DEWARP_CMODEL_BASIC_H_
