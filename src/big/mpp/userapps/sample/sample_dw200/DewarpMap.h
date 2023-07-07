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
#ifndef _DEWARP_HAL_DEWARPMAP_H_
#define _DEWARP_HAL_DEWARPMAP_H_

#ifdef __cplusplus
extern "C" {
#endif

typedef struct tag_grid_map_struct {
    int cols;                // block cols
    int rows;                // block rows
    int block_width;         // grid block width
    int block_height;        // grid block height
    int block_last_w;        // last grid block width
    int block_last_h;        // last grid block height
    int dataType;            // map data type 0: float 1:int
    int line_vertical_up;    // the up vertival line at which block, left or right in block
    int line_vertical_down;  // the down vertival line at which block, left or right in block
    int line_horizontal;     // the horizontal line at which block, up or down in block
    int mapWidth;            // map data cols
    int mapHeight;           // map data rows
    void *pGridmap;          // map buffer
} GridMapS;

int CreateUpdateDewarpMap(unsigned int* warpMap, int widthMap, int heightMap, int bitsMap, int fractionalBitMap,
    double *_cameraMatrix, double *_distCoeffs, int _width, int _height, float _fovRatio, int blockW, int blockH);

void CreateUpdateWarpPolarMap(unsigned int* warpMap, int widthMap, int heightMap, int bitsMap, int fractionalBitMap, int widthSrc, int heightSrc, int widthDst, int heightDst,
	float centerX, float centerY, double maxRadius, int horizon, int verticalUp, int verticalDown, int blockW, int blockH, int wcu,
	double offAngleUL, double offAngleUR, double offAngleDL, double offAngleDR, 
	double fovUL, double fovUR, double fovDL, double fovDR, int panoAtWin, 
	double centerOffsetRatioUL, double centerOffsetRatioUR, double centerOffsetRatioDL, double centerOffsetRatioDR,
	double circleOffsetRatioUL, double circleOffsetRatioUR, double circleOffsetRatioDL, double circleOffsetRatioDR);


int CreateUpdateFisheyeDewarpMap(unsigned int* warpMap, int widthMap, int heightMap, int bitsMap, int fractionalBitMap,
                                double *_cameraMatrix, double *_distCoeffs, int _width, int _height, float _fovRatio,
                                int blockW, int blockH);

void CreateUpdateFisheyeExpandMap(unsigned int* warpMap, int widthMap, int heightMap, int bitsMap, int fractionalBitMap,
                            int widthSrc, int heightSrc, int widthDst, int heightDst, float centerX, float centerY,
                            double maxRadius, int blockW, int blockH);

int CreateUpdatePerspectiveMap(unsigned int* warpMap, int widthMap, int heightMap, int bitsMap, int fractionalBitMap,
                            double *_M0, int _width, int _height, int blockW, int blockH, float scalefx,
                            float scalefy, float shiftx, float shifty);

int computePerspectiveFront(double *pM, int width, int height, double* scale, int *offsetx, int *offsety);

#ifdef __cplusplus
}
#endif

#endif  // _DEWARP_HAL_DEWARPMAP_H_

