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
 
#include "DewarpMap.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>

#include "basic.h"
#include "dw200.h"

int computePerspectiveFront(double *pM, int width, int height, double* scale, int *offsetx, int *offsety)
{
    double inx[4] = { 0, width - 1, 0, width - 1};
    double iny[4] = { 0, 0, height - 1, height - 1};
    double outx[4] = { 0.0 };
    double outy[4] = { 0.0 };

    for (int i = 0; i < 4; i++)
    {
        double x0 = pM[0] * inx[i] + pM[1] * iny[i] + pM[2];
        double y0 = pM[3] * inx[i] + pM[4] * iny[i] + pM[5];
        double w0 = pM[6] * inx[i] + pM[7] * iny[i] + pM[8];
        double w = w0 ? 1. / w0 : 0;
        double fx = x0*w;
        double fy = y0*w;
        outx[i] = fx;
        outy[i] = fy;
    }

    if (outx[0] < outx[2])
        *offsetx = (int)outx[2];
    else
        *offsetx = (int)outx[0];

    if (outy[0] < outy[1])
        *offsety = (int)outy[1];
    else
        *offsety = (int)outy[0];

    int endx, endy;
    if (outx[1] < outx[3])
        endx = (int)outx[1];
    else
        endx = (int)outx[3];

    if (outy[2] < outy[3])
        endy = (int)outy[2];
    else
        endy = (int)outy[3];

    double diffw = endx - *offsetx;
    double diffh = endy - *offsety;

    double scalex = diffw / width;
    double scaley = diffh / height;

    if (scalex <= 0.0 || scaley <= 0.0)
        *scale = 1.0;
    else if (scalex < scaley)
        *scale = scalex;
    else
        *scale = scaley;

    return 0;
}

//parameter:
//wcu: create or update, 5bits, bit5 1:create, bit3, =1: screen4 update ~bit0,=1: screen0 update; screen num from up to down, left to right
void CreateUpdateWarpPolarMap(unsigned int* warpMap, int widthMap, int heightMap, int bitsMap, int fractionalBitMap, int widthSrc, int heightSrc, int widthDst, int heightDst,
    float centerX, float centerY, double maxRadius, int horizon, int verticalUp, int verticalDown, int blockW, int blockH, int wcu,
    double offAngleUL, double offAngleUR, double offAngleDL, double offAngleDR, double fovUL, double fovUR, double fovDL, double fovDR, int panoAtWin,
	double centerOffsetRatioUL, double centerOffsetRatioUR, double centerOffsetRatioDL, double centerOffsetRatioDR, 
	double circleOffsetRatioUL, double circleOffsetRatioUR, double circleOffsetRatioDL, double circleOffsetRatioDR)
{
    if (!warpMap || !widthDst || !heightDst)
        return;
    double cdUL = 0.1*maxRadius, cdUR = 0.1*maxRadius, cdDL = 0.1*maxRadius, cdDR = 0.1*maxRadius;
    double odUL = 0.1*maxRadius, odUR = 0.1*maxRadius, odDL = 0.1*maxRadius, odDR = 0.1*maxRadius;

    int fracVaule = (1 << fractionalBitMap);
    unsigned int *pMapx = warpMap;
    unsigned int *pMapy = warpMap + widthMap*heightMap;
    int mapvaluex;
    int mapvaluey;
    int mapvalue;
    int splitN = 1;
    float* rhos = (float*)malloc(sizeof(float)*heightDst * 4);
    
        float offThe = 0.9999;
        if (centerOffsetRatioUL<0 || centerOffsetRatioUR<0 || centerOffsetRatioDL<0 || centerOffsetRatioDR<0||
            circleOffsetRatioUL<0 || circleOffsetRatioUR<0 || circleOffsetRatioDL<0 || circleOffsetRatioDR<0||
            centerOffsetRatioUL + circleOffsetRatioUL > offThe || centerOffsetRatioUR + circleOffsetRatioUR > offThe
            || centerOffsetRatioDL + circleOffsetRatioDL > offThe || centerOffsetRatioDR + circleOffsetRatioDR > offThe)
        {
            free(rhos);
            return;
        }
        cdUL = centerOffsetRatioUL*maxRadius;
        cdUR = centerOffsetRatioUR*maxRadius;
        cdDL = centerOffsetRatioDL*maxRadius;
        cdDR = centerOffsetRatioDR*maxRadius;
        odUL = circleOffsetRatioUL*maxRadius;
        odUR = circleOffsetRatioUR*maxRadius;
        odDL = circleOffsetRatioDL*maxRadius;
        odDR = circleOffsetRatioDR*maxRadius;


    if ((horizon > blockH / 2) && (horizon < heightDst))
    {
        if (((verticalUp>blockW / 2) && (verticalUp<widthDst)) && ((verticalDown>blockW / 2) && (verticalDown<widthDst)))
        {
            splitN = 4;
           // dd = 0.3*maxRadius;//offset
        }
        else if ((verticalUp>blockW / 2) && (verticalUp<widthDst))
        {
            splitN = 3;
           // dd = 0.3*maxRadius;//offset
        }
        else if ((verticalDown>blockW / 2) && (verticalDown<widthDst))
        {
            splitN = 3;
            //dd = 0.3*maxRadius;//offset
        }
        else
        {
            splitN = 2;
            //dd = 0.2*maxRadius;//offset
        }
    }
    else
    {
        if (((verticalUp>blockW / 2) && (verticalUp<widthDst)) && ((verticalDown>blockW / 2) && (verticalDown < widthDst)) && (verticalUp == verticalDown))
        {
            splitN = 5;//split left and right
            //dd = 0.2*maxRadius;//offset
        }
    }

    int phi, rho, i, j;
    double Kmag;
    // precalculate scaled rho
    if (1 == splitN)
    {
        double Kangle360 = VS_2PI / widthDst;//src
        float* bufRhos = (float*)(rhos);
       	Kmag = (maxRadius - cdUL - odUL) / heightDst;
        for (rho = 0; rho < heightDst; rho++)
			bufRhos[heightDst - 1 - rho] = (float)(rho * Kmag + cdUL);

        int rows = (heightDst % blockH) ? (heightDst / blockH + 2) : (heightDst / blockH + 1);
        int cols = (widthDst % blockW) ? (widthDst / blockW + 2) : (widthDst / blockW + 1);

        //horizon start
        for (phi = 0, i = 0; i < cols; phi += BLOCK_SIZE, i++)
        {
            if ((cols - 1) == i)
                phi = widthDst - 1;
            double KKy = Kangle360 * phi;
            double cp = cos(KKy);
            double sp = sin(KKy);

            for (rho = 0, j = 0; j < rows; rho += BLOCK_SIZE, j++)
            {
                if ((rows - 1) == j)
                    rho = heightDst - 1;
                double x = bufRhos[rho] * cp + centerX;
                double y = bufRhos[rho] * sp + centerY;

                if ( bitsMap<17)
                {
                    mapvaluex = ((int)(x*fracVaule) & 0xffff);
                    mapvaluey = ((int)(y*fracVaule) & 0xffff);
                    mapvalue = ((mapvaluey << 16) | mapvaluex);
                    pMapx[j*widthMap + i] = (unsigned int)mapvalue;
                }
                else if (bitsMap < 32)
                {
                    mapvaluex = ((int)(x*fracVaule));
                    mapvaluey = ((int)(y*fracVaule));
                    pMapx[j*widthMap + i] = mapvaluex;
                    pMapy[j*widthMap + i] = mapvaluey;
                }
            }
        }
    }
    else if (2 == splitN)
    {
        int lineh = (horizon + blockH / 2) / blockH*blockH;
        int lineh2 = heightDst - lineh;

        float* bufRhosUp = (float*)(rhos);//split up and down parts 
        float* bufRhosDown = (float*)(rhos + heightDst);

        Kmag = (maxRadius - cdUL - odUL) / lineh;
        for (rho = 0; rho < lineh; rho++)
            bufRhosUp[lineh - 1 - rho] = (float)(rho * Kmag + cdUL);

        Kmag = (maxRadius - cdDL - odDL) / lineh2;
        for (rho = 0; rho < lineh2; rho++)
            bufRhosDown[lineh2 - 1 - rho] = (float)(rho*Kmag + cdDL);

		double off_angle_ul = (offAngleUL / 180.0)*VS_PI;// VS_PI;// ((180) / 180.0)*VS_PI;
		double off_angle_dl = (offAngleDL / 180.0)*VS_PI;// 0;// ((0) / 180.0)*VS_PI;
		double fovAngle_ul = fovUL;// 180;
		double fovAngle_dl = fovDL;// 180;
        double Kangleul = ((fovAngle_ul*VS_PI) / 180.0) / widthDst;
        double Kangledl = ((fovAngle_dl*VS_PI) / 180.0) / widthDst;

        int rowsUp = (lineh % blockH) ? (lineh / blockH + 2) : (lineh / blockH + 1);
        int rowsDown = (lineh2 % blockH) ? (lineh2 / blockH + 2) : (lineh2 / blockH + 1);
        int cols = (widthDst % blockW) ? (widthDst / blockW + 2) : (widthDst / blockW + 1);

        if (((wcu >> 5) & 0x1) || (wcu & 0x1))
        {
            for (phi = 0, i = 0; i < cols; phi += blockW, i++)
            {
                if ((cols - 1) == i)
                    phi = widthDst - 1;
                double KKy = Kangleul * phi + off_angle_ul;
                double cp = cos(KKy);
                double sp = sin(KKy);
                //up part
                for (rho = 0, j = 0; j < rowsUp; rho += blockH, j++)
                {
                    if ((rowsUp - 1) == j)
                        rho = lineh - 1;

                    double x = centerX + bufRhosUp[rho] * cp;
                    double y = centerY + bufRhosUp[rho] * sp;

                    if (bitsMap<17)
                    {
                        mapvaluex = ((int)(x*fracVaule) & 0xffff);
                        mapvaluey = ((int)(y*fracVaule) & 0xffff);
                        mapvalue = ((mapvaluey << 16) | mapvaluex);
                        pMapx[j*widthMap + i] = (unsigned int)mapvalue;
                    }
                    else if (bitsMap < 32)
                    {
                        mapvaluex = ((int)(x*fracVaule));
                        mapvaluey = ((int)(y*fracVaule));
                        pMapx[j*widthMap + i] = mapvaluex;
                        pMapy[j*widthMap + i] = mapvaluey;
                    }
                }
            }
        }

        if (((wcu >> 5) & 0x1) || ((wcu>>1) & 0x1))
        {
            for (phi = 0, i = 0; i < cols; phi += blockW, i++)
            {
                if ((cols - 1) == i)
                    phi = widthDst - 1;
                double KKy = Kangledl * phi + off_angle_dl;
                double cp = cos(KKy);
                double sp = sin(KKy);
                //down part
                for (rho = 0, j = 0; j < rowsDown; rho += blockH, j++)
                {
                    if ((rowsDown - 1) == j)
                        rho = lineh2 - 1;
                    double x = centerX + bufRhosDown[rho] * cp;
                    double y = bufRhosDown[rho] * sp + centerY;

                    if (bitsMap<17)
                    {
                        mapvaluex = ((int)(x*fracVaule) & 0xffff);
                        mapvaluey = ((int)(y*fracVaule) & 0xffff);
                        mapvalue = ((mapvaluey << 16) | mapvaluex);
                        pMapx[(j + rowsUp)*widthMap + i] = (unsigned int)mapvalue;
                    }
                    else if (bitsMap < 32)
                    {
                        mapvaluex = ((int)(x*fracVaule));
                        mapvaluey = ((int)(y*fracVaule));
                        pMapx[(j + rowsUp)*widthMap + i] = mapvaluex;
                        pMapy[(j + rowsUp)*widthMap + i] = mapvaluey;
                    }
                }
            }
        }
    }
    else if (3 == splitN)
    {
        int lineh = (horizon + blockH / 2) / blockH*blockH;
        int lineh2 = heightDst - lineh;

        int rowsUp = (lineh % blockH) ? (lineh / blockH + 2) : (lineh / blockH + 1);
        int rowsDown = (lineh2 % blockH) ? (lineh2 / blockH + 2) : (lineh2 / blockH + 1);
        int cols = (widthDst % blockW) ? (widthDst / blockW + 2) : (widthDst / blockW + 1);

		float* bufRhosUpL = (float*)(rhos);//split up and down parts 
		float* bufRhosUpR = (float*)(rhos + heightDst);
		float* bufRhosDownL = (float*)(rhos + 2 * heightDst);
		float* bufRhosDownR = (float*)(rhos + 3 * heightDst);


        if ((verticalUp>blockW / 2) && (verticalUp<widthDst))
        {
			Kmag = (maxRadius - cdUL - odUL) / lineh;
			for (rho = 0; rho < lineh; rho++)
				bufRhosUpL[lineh - 1 - rho] = (float)(rho * Kmag + cdUL);

			Kmag = (maxRadius - cdUR - odUR) / lineh;
			for (rho = 0; rho < lineh; rho++)
				bufRhosUpR[lineh - 1 - rho] = (float)(rho * Kmag + cdUR);

			Kmag = (maxRadius - cdDL -odDL) / lineh2;
			for (rho = 0; rho < lineh2; rho++)
				bufRhosDownL[lineh2 - 1 - rho] = (float)(rho*Kmag + cdDL);

            int linevul = (verticalUp + blockW / 2) / blockW*blockW;//vertical line up, left
            int linevur = widthDst - linevul;//vertical line up, right

            int colsl = (linevul % blockW) ? (linevul / blockW + 2) : (linevul / blockW + 1);
            int colsr = (linevur % blockW) ? (linevur / blockW + 2) : (linevur / blockW + 1);

			double off_angle_ul = (offAngleUL / 180.0)*VS_PI;// ((180 + 15) / 180.0)*VS_PI;
			double off_angle_ur = (offAngleUR / 180.0)*VS_PI;// ((270 + 15) / 180.0)*VS_PI;
			double off_angle_dl = (offAngleDL / 180.0)*VS_PI;// 0;// ((0) / 180.0)*VS_PI;
			double fovAngle_ul = fovUL;// 60;
			double fovAngle_ur = fovUR;// 60;
			double fovAngle_dl = fovDL;// 180;
            double Kangleul = ((fovAngle_ul*VS_PI) / 180.0) / linevul;
            double Kangleur = ((fovAngle_ur*VS_PI) / 180.0) / linevur;
            double Kangledl = ((fovAngle_dl*VS_PI) / 180.0) / widthDst;

            if (((wcu >> 5) & 0x1) || ((wcu)& 0x1))
            {
                for (phi = 0, i = 0; i < colsl; phi += blockW, i++)//width
                {
                    if ((colsl - 1) == i)
                        phi = linevul - 1;
                    double KKy = Kangleul * phi + off_angle_ul;
                    double cp = cos(KKy);
                    double sp = sin(KKy);
                    //up left part
                    for (rho = 0, j = 0; j < rowsUp; rho += blockH, j++)//height
                    {
                        if ((rowsUp - 1) == j)
                            rho = lineh - 1;
                        double x = centerX + bufRhosUpL[rho] * cp;
                        double y = centerY + bufRhosUpL[rho] * sp;

                        if (bitsMap<17)
                        {
                            mapvaluex = ((int)(x*fracVaule) & 0xffff);
                            mapvaluey = ((int)(y*fracVaule) & 0xffff);
                            mapvalue = ((mapvaluey << 16) | mapvaluex);
                            pMapx[j*widthMap + i] = (unsigned int)mapvalue;
                        }
                        else if (bitsMap < 32)
                        {
                            mapvaluex = ((int)(x*fracVaule));
                            mapvaluey = ((int)(y*fracVaule));
                            pMapx[j*widthMap + i] = mapvaluex;
                            pMapy[j*widthMap + i] = mapvaluey;
                        }
                    }
                }

            }

            if (((wcu >> 5) & 0x1) || ((wcu >> 1) & 0x1))
            {
                for (phi = linevul, i = 0; i < colsr; phi += blockW, i++)
                {
                    if ((colsr - 1) == i)
                        phi = widthDst - 1;
                    double KKy = Kangleur * (phi - linevul) + off_angle_ur;
                    double cp = cos(KKy);
                    double sp = sin(KKy);
                    //up right part
                    for (rho = 0, j = 0; j < rowsUp; rho += blockH, j++)//height
                    {
                        if ((rowsUp - 1) == j)
                            rho = lineh - 1;

                        double x = centerX + bufRhosUpR[rho] * cp;
                        double y = centerY + bufRhosUpR[rho] * sp;

                        if (bitsMap<17)
                        {
                            mapvaluex = ((int)(x*fracVaule) & 0xffff);
                            mapvaluey = ((int)(y*fracVaule) & 0xffff);
                            mapvalue = ((mapvaluey << 16) | mapvaluex);
                            pMapx[j*widthMap + i + colsl] = (unsigned int)mapvalue;
                        }
                        else if (bitsMap < 32)
                        {
                            mapvaluex = ((int)(x*fracVaule));
                            mapvaluey = ((int)(y*fracVaule));
                            pMapx[j*widthMap + i + colsl] = mapvaluex;
                            pMapy[j*widthMap + i + colsl] = mapvaluey;
                        }
                    }
                }
            }

            if (((wcu >> 5) & 0x1) || ((wcu >> 2) & 0x1))
            {
                for (phi = 0, i = 0; i < cols; phi += blockW, i++)
                {
                    if ((cols - 1) == i)
                        phi = widthDst - 1;
                    double KKy = Kangledl * phi + off_angle_dl;
                    double cp = cos(KKy);
                    double sp = sin(KKy);
                    //down part
                    for (rho = 0, j = 0; j < rowsDown; rho += blockH, j++)
                    {
                        if ((rowsDown - 1) == j)
                            rho = lineh2 - 1;

                        double x = centerX + bufRhosDownL[rho] * cp;
                        double y = centerY + bufRhosDownL[rho] * sp;

                        if (bitsMap<17)
                        {
                            mapvaluex = ((int)(x*fracVaule) & 0xffff);
                            mapvaluey = ((int)(y*fracVaule) & 0xffff);
                            mapvalue = ((mapvaluey << 16) | mapvaluex);
                            pMapx[(j+rowsUp)*widthMap + i] = (unsigned int)mapvalue;
                        }
                        else if (bitsMap < 32)
                        {
                            mapvaluex = ((int)(x*fracVaule));
                            mapvaluey = ((int)(y*fracVaule));
                            pMapx[(j + rowsUp)*widthMap + i] = mapvaluex;
                            pMapy[(j + rowsUp)*widthMap + i] = mapvaluey;
                        }
                    }
                }
            }

        }
        else if ((verticalDown>blockW / 2) && (verticalDown<widthDst))
        {
			Kmag = (maxRadius - cdUL - odUL) / lineh;
			for (rho = 0; rho < lineh; rho++)
				bufRhosUpL[lineh - 1 - rho] = (float)(rho * Kmag + cdUL);
						
			Kmag = (maxRadius - cdDL - odDL) / lineh2;
			for (rho = 0; rho < lineh2; rho++)
				bufRhosDownL[lineh2 - 1 - rho] = (float)(rho*Kmag + cdDL);

			Kmag = (maxRadius - cdDR - odDR) / lineh2;
			for (rho = 0; rho < lineh2; rho++)
				bufRhosDownR[lineh2 - 1 - rho] = (float)(rho * Kmag + cdDR);

            int linevdl = (verticalDown + blockW / 2) / blockW*blockW;//vertical line down, left
            int linevdr = widthDst - linevdl;////vertical line down, right

            int colsl = (linevdl % blockW) ? (linevdl / blockW + 2) : (linevdl / blockW + 1);
            int colsr = (linevdr % blockW) ? (linevdr / blockW + 2) : (linevdr / blockW + 1);

			double off_angle_ul = (offAngleUL / 180.0)*VS_PI;//((180 + 15) / 180.0)*VS_PI;
			double off_angle_dl = (offAngleDL / 180.0)*VS_PI;//((90 + 15) / 180.0)*VS_PI;
			double off_angle_dr = (offAngleDR / 180.0)*VS_PI;//((0 + 15) / 180.0)*VS_PI;
			double fovAngle_ul = fovUL;// 180;
			double fovAngle_dl = fovDL;//60;
			double fovAngle_dr = fovDR;//60;
            double Kangleul = ((fovAngle_ul*VS_PI) / 180.0) / widthDst;
            double Kangledl = ((fovAngle_dl*VS_PI) / 180.0) / linevdl;
            double Kangledr = ((fovAngle_dr*VS_PI) / 180.0) / linevdr;

            if (((wcu >> 5) & 0x1) || ((wcu)& 0x1))
            {
                for (phi = 0, i = 0; i < cols; phi += blockW, i++)
                {
                    if ((cols - 1) == i)
                        phi = widthDst - 1;
                    double KKy = Kangleul * phi + off_angle_ul;
                    double cp = cos(KKy);
                    double sp = sin(KKy);
                    //up part
                    for (rho = 0, j = 0; j < rowsUp; rho += blockH, j++)
                    {
                        if ((rowsUp - 1) == j)
                            rho = lineh - 1;

                        double x = centerX + bufRhosUpL[rho] * cp;
                        double y = centerY + bufRhosUpL[rho] * sp;

                        if (bitsMap<17)
                        {
                            mapvaluex = ((int)(x*fracVaule) & 0xffff);
                            mapvaluey = ((int)(y*fracVaule) & 0xffff);
                            mapvalue = ((mapvaluey << 16) | mapvaluex);
                            pMapx[j*widthMap + i] = (unsigned int)mapvalue;
                        }
                        else if (bitsMap < 32)
                        {
                            mapvaluex = ((int)(x*fracVaule));
                            mapvaluey = ((int)(y*fracVaule));
                            pMapx[j*widthMap + i] = mapvaluex;
                            pMapy[j*widthMap + i] = mapvaluey;
                        }
                    }
                }
            }

            if (((wcu >> 5) & 0x1) || ((wcu>>1)& 0x1))
            {
                for (phi = 0, i = 0; i < colsl; phi += blockW, i++)//width
                {
                    if ((colsl - 1) == i)
                        phi = linevdl - 1;
                    double KKy = Kangledl * phi + off_angle_dl;
                    double cp = cos(KKy);
                    double sp = sin(KKy);
                    //down left part
                    for (rho = 0, j = 0; j < rowsDown; rho += blockH, j++)
                    {
                        if ((rowsDown - 1) == j)
                            rho = lineh2 - 1;

                        double x = centerX + bufRhosDownL[rho] * cp;
                        double y = centerY + bufRhosDownL[rho] * sp;

                        if (bitsMap<17)
                        {
                            mapvaluex = ((int)(x*fracVaule) & 0xffff);
                            mapvaluey = ((int)(y*fracVaule) & 0xffff);
                            mapvalue = ((mapvaluey << 16) | mapvaluex);
                            pMapx[(j + rowsUp)*widthMap + i] = (unsigned int)mapvalue;
                        }
                        else if (bitsMap < 32)
                        {
                            mapvaluex = ((int)(x*fracVaule));
                            mapvaluey = ((int)(y*fracVaule));
                            pMapx[(j + rowsUp)*widthMap + i] = mapvaluex;
                            pMapy[(j + rowsUp)*widthMap + i] = mapvaluey;
                        }
                    }
                }
            }

            if (((wcu >> 5) & 0x1) || ((wcu >> 2) & 0x1))
            {
                for (phi = linevdl, i = 0; i < colsr; phi += blockW, i++)
                {
                    if ((colsr - 1) == i)
                        phi = widthDst - 1;
                    double KKy = Kangledr * (phi - linevdl) + off_angle_dr;
                    double cp = cos(KKy);
                    double sp = sin(KKy);
                    //down right part
                    for (rho = 0, j = 0; j < rowsDown; rho += blockH, j++)
                    {
                        if ((rowsDown - 1) == j)
                            rho = lineh2 - 1;

                        double x = centerX + bufRhosDownR[rho] * cp;
                        double y = centerY + bufRhosDownR[rho] * sp;

                        if (bitsMap<17)
                        {
                            mapvaluex = ((int)(x*fracVaule) & 0xffff);
                            mapvaluey = ((int)(y*fracVaule) & 0xffff);
                            mapvalue = ((mapvaluey << 16) | mapvaluex);
                            pMapx[(j + rowsUp)*widthMap + i + colsl] = (unsigned int)mapvalue;
                        }
                        else if (bitsMap < 32)
                        {
                            mapvaluex = ((int)(x*fracVaule));
                            mapvaluey = ((int)(y*fracVaule));
                            pMapx[(j + rowsUp)*widthMap + i + colsl] = mapvaluex;
                            pMapy[(j + rowsUp)*widthMap + i + colsl] = mapvaluey;
                        }
                    }
                }
            }
        }
    }
    else if (4 == splitN)
    {
        int lineh = (horizon + blockH / 2) / blockH*blockH;//horizon up
        int lineh2 = heightDst - lineh;//horizon down

        int linevul = (verticalUp + blockW / 2) / blockW*blockW;//vertical line up, left
        int linevur = widthDst - linevul;//vertical line up, right

        int linevdl = (verticalDown + blockW / 2) / blockW*blockW;//vertical line down, left
        int linevdr = widthDst - linevdl;////vertical line down, right

        //add 20190128
        int rowsUp = (lineh % blockH) ? (lineh / blockH + 2) : (lineh / blockH + 1);
        int rowsDown = (lineh2 % blockH) ? (lineh2 / blockH + 2) : (lineh2 / blockH + 1);
        int colsUpLeft = (linevul % blockW) ? (linevul / blockW + 2) : (linevul / blockW + 1);
        int colsUpRight = (linevur % blockW) ? (linevur / blockW + 2) : (linevur / blockW + 1);
        int colsDownLeft = (linevdl % blockW) ? (linevdl / blockW + 2) : (linevdl / blockW + 1);
        int colsDownRight = (linevdr % blockW) ? (linevdr / blockW + 2) : (linevdr / blockW + 1);
        //
		float* bufRhosUpL = (float*)(rhos);//split up and down parts 
		float* bufRhosUpR = (float*)(rhos + heightDst);
		float* bufRhosDownL = (float*)(rhos + 2 * heightDst);
		float* bufRhosDownR = (float*)(rhos + 3 * heightDst);
		Kmag = (maxRadius - cdUL - odUL) / lineh;
		for (rho = 0; rho < lineh; rho++)
			bufRhosUpL[lineh - 1 - rho] = (float)(rho * Kmag + cdUL);

		Kmag = (maxRadius - cdUR - odUR) / lineh;
		for (rho = 0; rho < lineh; rho++)
			bufRhosUpR[lineh - 1 - rho] = (float)(rho * Kmag + cdUR);

		Kmag = (maxRadius - cdDL - odDL) / lineh2;
		for (rho = 0; rho < lineh2; rho++)
			bufRhosDownL[lineh2 - 1 - rho] = (float)(rho*Kmag + cdDL);

		Kmag = (maxRadius - cdDR - odDR) / lineh2;
		for (rho = 0; rho < lineh2; rho++)
			bufRhosDownR[lineh2 - 1 - rho] = (float)(rho * Kmag + cdDR);
		
		//double offAngleUL = (180 + 15), double offAngleUR = (270 + 15), double offAngleDL = (90 + 15), double offAngleDR = 15,
		//double fovUL = 60, double fovUR = 60, double fovDL = 60, double fovDR = 60
		double off_angle_ul = (offAngleUL / 180.0)*VS_PI;
		double off_angle_ur = (offAngleUR / 180.0)*VS_PI;
		double off_angle_dl = (offAngleDL / 180.0)*VS_PI;
		double off_angle_dr = (offAngleDR / 180.0)*VS_PI;

		double fovAngle_ul = fovUL;
		double fovAngle_ur = fovUR;
		double fovAngle_dl = fovDL;
		double fovAngle_dr = fovDR;

        double Kangleul = ((fovAngle_ul*VS_PI) / 180.0) / linevul;
        double Kangleur = ((fovAngle_ur*VS_PI) / 180.0) / linevur;
        double Kangledl = ((fovAngle_dl*VS_PI) / 180.0) / linevdl;
        double Kangledr = ((fovAngle_dr*VS_PI) / 180.0) / linevdr;

        if (((wcu >> 5) & 0x1) || ((wcu) & 0x1))
        {
			if (1 == panoAtWin)
			{
				float scalex = (float)widthSrc /(float) linevul;
				float scaley = (float)heightSrc / (float)lineh;
				
				for (i = 0; i < colsUpLeft; i++)//width
				{					
					//up left part
					double x = i*blockW*scalex;
					for (j = 0; j < rowsUp; j++)//height
					{						
						double y = j*blockH*scaley;

						if (bitsMap<17)
						{
							mapvaluex = ((int)(x*fracVaule) & 0xffff);
							mapvaluey = ((int)(y*fracVaule) & 0xffff);
							mapvalue = ((mapvaluey << 16) | mapvaluex);
							pMapx[j*widthMap + i] = (unsigned int)mapvalue;
						}
						else if (bitsMap < 32)
						{
							mapvaluex = ((int)(x*fracVaule));
							mapvaluey = ((int)(y*fracVaule));
							pMapx[j*widthMap + i] = mapvaluex;
							pMapy[j*widthMap + i] = mapvaluey;
						}
					}
				}
			}
			else{
				for (phi = 0, i = 0; i < colsUpLeft; phi += blockW, i++)//width
				{
					if ((colsUpLeft - 1) == i)
						phi = linevul - 1;
					double KKy = Kangleul * phi + off_angle_ul;
					double cp = cos(KKy);
					double sp = sin(KKy);
					//up left part
					for (rho = 0, j = 0; j < rowsUp; rho += blockH, j++)//height
					{
						if ((rowsUp - 1) == j)
							rho = lineh - 1;

						double x = centerX + bufRhosUpL[rho] * cp;
						double y = centerY + bufRhosUpL[rho] * sp;

						if (bitsMap<17)
						{
							mapvaluex = ((int)(x*fracVaule) & 0xffff);
							mapvaluey = ((int)(y*fracVaule) & 0xffff);
							mapvalue = ((mapvaluey << 16) | mapvaluex);
							pMapx[j*widthMap + i] = (unsigned int)mapvalue;
						}
						else if (bitsMap < 32)
						{
							mapvaluex = ((int)(x*fracVaule));
							mapvaluey = ((int)(y*fracVaule));
							pMapx[j*widthMap + i] = mapvaluex;
							pMapy[j*widthMap + i] = mapvaluey;
						}
					}
				}
			}            
        }


        if (((wcu >> 5) & 0x1) || ((wcu>>2)& 0x1))
        {
            for (phi = 0, i = 0; i < colsDownLeft; phi += blockW, i++)//width
            {
                if ((colsDownLeft - 1) == i)
                    phi = linevdl - 1;
                double KKy = Kangledl * phi + off_angle_dl;
                double cp = cos(KKy);
                double sp = sin(KKy);
                //down left part
                for (rho = 0, j = 0; j < rowsDown; rho += blockH, j++)
                {
                    if ((rowsDown - 1) == j)
                        rho = lineh2 - 1;

                    double x = centerX + bufRhosDownL[rho] * cp;
                    double y = centerY + bufRhosDownL[rho] * sp;

                    if (bitsMap<17)
                    {
                        mapvaluex = ((int)(x*fracVaule) & 0xffff);
                        mapvaluey = ((int)(y*fracVaule) & 0xffff);
                        mapvalue = ((mapvaluey << 16) | mapvaluex);
                        pMapx[(j + rowsUp)*widthMap + i] = (unsigned int)mapvalue;
                    }
                    else if (bitsMap < 32)
                    {
                        mapvaluex = ((int)(x*fracVaule));
                        mapvaluey = ((int)(y*fracVaule));
                        pMapx[(j + rowsUp)*widthMap + i] = mapvaluex;
                        pMapy[(j + rowsUp)*widthMap + i] = mapvaluey;
                    }
                }
            }
        }

        if (((wcu >> 5) & 0x1) || ((wcu >> 1) & 0x1))
        {
            for (phi = linevul, i = 0; i<colsUpRight; phi += blockW, i++)
            {
                if ((colsUpRight - 1) == i)
                    phi = widthDst - 1;

                double KKy = Kangleur * (phi - linevul) + off_angle_ur;
                double cp = cos(KKy);
                double sp = sin(KKy);
                //up right part
                for (rho = 0, j = 0; j<rowsUp; rho += blockH, j++)
                {
                    if ((rowsUp - 1) == j)
                        rho = lineh - 1;

                    double x = centerX + bufRhosUpR[rho] * cp;
                    double y = centerY + bufRhosUpR[rho] * sp;

                    if (bitsMap<17)
                    {
                        mapvaluex = ((int)(x*fracVaule) & 0xffff);
                        mapvaluey = ((int)(y*fracVaule) & 0xffff);
                        mapvalue = ((mapvaluey << 16) | mapvaluex);
                        pMapx[j*widthMap + i + colsUpLeft] = (unsigned int)mapvalue;
                    }
                    else if (bitsMap < 32)
                    {
                        mapvaluex = ((int)(x*fracVaule));
                        mapvaluey = ((int)(y*fracVaule));
                        pMapx[j*widthMap + i + colsUpLeft] = mapvaluex;
                        pMapy[j*widthMap + i + colsUpLeft] = mapvaluey;
                    }
                }
            }
        }

        if (((wcu >> 5) & 0x1) || ((wcu >> 3) & 0x1))
        {
            for (phi = linevdl, i = 0; i<colsDownRight; phi += blockW, i++)
            {
                if ((colsDownRight - 1) == i)
                    phi = widthDst - 1;
                double KKy = Kangledr * (phi - linevdl) + off_angle_dr;
                double cp = cos(KKy);
                double sp = sin(KKy);
                //down right part
                for (rho = 0, j = 0; j < rowsDown; rho += blockH, j++)
                {
                    if ((rowsDown - 1) == j)
                        rho = lineh2 - 1;

                    double x = centerX + bufRhosDownR[rho] * cp;
                    double y = centerY + bufRhosDownR[rho] * sp;

                    if (bitsMap<17)
                    {
                        mapvaluex = ((int)(x*fracVaule) & 0xffff);
                        mapvaluey = ((int)(y*fracVaule) & 0xffff);
                        mapvalue = ((mapvaluey << 16) | mapvaluex);
                        pMapx[(j + rowsUp)*widthMap + i + colsDownLeft] = (unsigned int)mapvalue;
                    }
                    else if (bitsMap < 32)
                    {
                        mapvaluex = ((int)(x*fracVaule));
                        mapvaluey = ((int)(y*fracVaule));
                        pMapx[(j + rowsUp)*widthMap + i + colsDownLeft] = mapvaluex;
                        pMapy[(j + rowsUp)*widthMap + i + colsDownLeft] = mapvaluey;
                    }
                }
            }
        }

    }
    else if (5 == splitN)
    {
        //for RTL test, create map 
        //if (verticalUp == verticalDown)
        {
            int lineh = heightDst;

			float* bufRhosUpL = (float*)(rhos);//split up and down parts 
			float* bufRhosUpR = (float*)(rhos + heightDst);			
			Kmag = (maxRadius - cdUL - odUL) / lineh;
			for (rho = 0; rho < lineh; rho++)
				bufRhosUpL[lineh - 1 - rho] = (float)(rho * Kmag + cdUL);

			Kmag = (maxRadius - cdUR - odUR) / lineh;
			for (rho = 0; rho < lineh; rho++)
				bufRhosUpR[lineh - 1 - rho] = (float)(rho * Kmag + cdUR);			  

            int linevul = (verticalUp + blockW / 2) / blockW*blockW;//vertical line up, left
            int linevur = widthDst - linevul;//vertical line up, right

            int rows = (heightDst % blockH) ? (heightDst / blockH + 2) : (heightDst / blockH + 1);
            int colsL = (linevul % blockW) ? (linevul / blockW + 2) : (linevul / blockW + 1);
            int colsR = (linevur % blockW) ? (linevur / blockW + 2) : (linevur / blockW + 1);

			double off_angle_l = (offAngleUL / 180.0)*VS_PI; //((-90) / 180.0)*VS_PI;
			double off_angle_r = (offAngleUR / 180.0)*VS_PI; //((90) / 180.0)*VS_PI;
			double fovAngle_l = fovUL;// 60;
			double fovAngle_r = fovUR;// 60;

            double Kanglel = ((fovAngle_l*VS_PI) / 180.0) / linevul;
            double Kangler = ((fovAngle_r*VS_PI) / 180.0) / linevur;

            //double KangleFovaul = ((fovAngle*VS_PI) / 180.0) / linevul;
            //double KangleFovaur = ((fovAngle*VS_PI) / 180.0) / linevur;
            if (((wcu >> 5) & 0x1) || ((wcu) & 0x1))
            {
                for (phi = 0, i = 0; i < colsL; phi += blockW, i++)//width
                {
                    if ((colsL - 1) == i)
                        phi = linevul - 1;
                    double KKy = Kanglel * phi + off_angle_l;
                    double cp = cos(KKy);
                    double sp = sin(KKy);
                    //up left part
                    for (rho = 0, j = 0; j < rows; rho += blockH, j++)//height
                    {
                        if ((rows - 1) == j)
                            rho = lineh - 1;

                        double x = centerX + bufRhosUpL[rho] * cp;
                        double y = centerY + bufRhosUpL[rho] * sp;

                        if (bitsMap<17)
                        {
                            mapvaluex = ((int)(x*fracVaule) & 0xffff);
                            mapvaluey = ((int)(y*fracVaule) & 0xffff);
                            mapvalue = ((mapvaluey << 16) | mapvaluex);
                            pMapx[j*widthMap + i] = (unsigned int)mapvalue;
                        }
                        else if (bitsMap < 32)
                        {
                            mapvaluex = ((int)(x*fracVaule));
                            mapvaluey = ((int)(y*fracVaule));
                            pMapx[j*widthMap + i] = mapvaluex;
                            pMapy[j*widthMap + i] = mapvaluey;
                        }
                    }
                }
            }

            if (((wcu >> 5) & 0x1) || ((wcu>>1)& 0x1))
            {
                for (phi = linevul, i = 0; i < colsR; phi += blockW, i++)
                {
                    if ((colsR - 1) == i)
                        phi = widthDst - 1;
                    double KKy = Kangler * (phi - linevul) + off_angle_r;
                    double cp = cos(KKy);
                    double sp = sin(KKy);
                    //up right part
                    for (rho = 0, j = 0; j < rows; rho += blockH, j++)
                    {
                        if ((rows - 1) == j)
                            rho = lineh - 1;

                        double x = centerX + bufRhosUpR[rho] * cp;
                        double y = centerY + bufRhosUpR[rho] * sp;

                        if (bitsMap<17)
                        {
                            mapvaluex = ((int)(x*fracVaule) & 0xffff);
                            mapvaluey = ((int)(y*fracVaule) & 0xffff);
                            mapvalue = ((mapvaluey << 16) | mapvaluex);
                            pMapx[j*widthMap + i + colsL] = (unsigned int)mapvalue;
                        }
                        else if (bitsMap < 32)
                        {
                            mapvaluex = ((int)(x*fracVaule));
                            mapvaluey = ((int)(y*fracVaule));
                            pMapx[j*widthMap + i + colsL] = mapvaluex;
                            pMapy[j*widthMap + i + colsL] = mapvaluey;
                        }
                    }
                }
            }

        }

    }

    free(rhos);
}

//2:create fisheye lens expand map
void CreateUpdateFisheyeExpandMap(unsigned int* warpMap, int widthMap, int heightMap, int bitsMap, int fractionalBitMap, int widthSrc, int heightSrc, int widthDst, int heightDst,
    float centerX, float centerY, double maxRadius, int blockW, int blockH)
{
    if (!warpMap || widthDst < 1 || widthSrc < 1 || heightDst < 1 || heightSrc < 1 ||
        widthDst > 0x1fff || widthSrc>0x1fff || heightSrc>0x1fff || heightDst > 0x1fff)
        return;

    unsigned int *pMapx = warpMap;
    unsigned int *pMapy = warpMap + widthMap*heightMap;
    double radius = maxRadius;

    double kmagx = radius * 2 / widthDst;
    double radius2 = radius*radius;

    int gridrows = (heightDst%blockH) ? (heightDst / blockH + 2) : (heightDst / blockH + 1);
    int gridcols = (widthDst%blockW) ? (widthDst / blockW + 2) : (widthDst / blockW + 1);
    int fracVaule = (1 << fractionalBitMap);
    int mapvaluex, mapvaluey, mapvalue;

    for (int j = 0, r = 0; r < gridrows; j += blockH, r++)
    {
        if ((gridrows - 1) == r)
            j = heightDst - 1;
        double diff_y_c = (j - centerY)*(j - centerY);
        //double 
        double dist_y = sqrt(radius2 - diff_y_c);
        for (int i = 0, c = 0; c < gridcols; i += blockW, c++)
        {
            if ((gridcols - 1) == c)
                i = widthDst - 1;
            double diff_i = i*kmagx - radius;
            float x = (float)(diff_i*dist_y / radius + centerX);
            float y = j;
            if (bitsMap<17)
            {
                mapvaluex = ((int)(x*fracVaule) & 0xffff);
                mapvaluey = ((int)(y*fracVaule) & 0xffff);
                mapvalue = ((mapvaluey << 16) | mapvaluex);
                pMapx[r*widthMap + c] = (unsigned int)mapvalue;
            }
            else if (bitsMap < 32)
            {
                mapvaluex = ((int)(x*fracVaule));
                mapvaluey = ((int)(y*fracVaule));
                pMapx[r*widthMap + c] = mapvaluex;
                pMapy[r*widthMap + c] = mapvaluey;
            }
        }
    }
}

//use the camera matrix and dist coeffs compute undistort look up table//
void getDefaultNewCameraMatrixD(double* _newCameraMatrix, double* _cameraMatrix, int imgsize_width, int imgsize_height,
    int centerPrincipalPoint)
{
    double* cameraMatrix = _cameraMatrix;
    double* newCameraMatrix = _newCameraMatrix;

    if (!centerPrincipalPoint)
    {
        for (int i = 0; i<9; i++)
        {
            *newCameraMatrix++ = *cameraMatrix++;
        }
    }
    else
    {
        newCameraMatrix[0] = cameraMatrix[0];
        newCameraMatrix[1] = cameraMatrix[1];
        newCameraMatrix[2] = (imgsize_width - 1)*0.5;
        newCameraMatrix[3] = cameraMatrix[3];
        newCameraMatrix[4] = cameraMatrix[4];
        newCameraMatrix[5] = (imgsize_height - 1)*0.5;
        newCameraMatrix[6] = cameraMatrix[6];
        newCameraMatrix[7] = cameraMatrix[7];
        newCameraMatrix[8] = cameraMatrix[8];
    }
    return;
}

//parameter@
//_cameraMatrix: camera internal parameter matrix, 3x3
// |fx, 0 , u|
// |0 , fy, v|
// |0 , 0 , 1| 
//_distCoeffs: distortion parameter, 1x8 [k1,k2,p1,p2,k3,k4,k5,k6]
//if k3,k4,k5,k6 no value, set 0.
//_matR: eye matrix
//_newCameraMatrix: like _cameraMatrix
//size: image size(width, height)
//m1type: out put map type, fixed point or float point
//_map1: out put undistort look up table
//_map2: out put undistort look up table
void vsInitUndistortRectifyMap(double* _cameraMatrix, double* _distCoeffs,
    double* _matR, double* _newCameraMatrix,
    int size_width, int size_height, int m1type,
    void *_map1, int map1_step, void* _map2, int map2_step)
{
    double *cameraMatrix = _cameraMatrix, *distCoeffs = _distCoeffs;
    double *newCameraMatrix = _newCameraMatrix;
    void *map1 = _map1;
    void *map2 = _map2;

    double A[9] = { 0.0 };
    double Ar[9] = { 0.0 };
    double iR[9] = { 0.0 };
    double L[9] = { 0.0 };
    double U[9] = { 0.0 };
    double invL[9] = { 0.0 };
    double invU[9] = { 0.0 };

    A[0] = cameraMatrix[0];  A[1] = cameraMatrix[1];  A[2] = cameraMatrix[2];
    A[3] = cameraMatrix[3];  A[4] = cameraMatrix[4];  A[5] = cameraMatrix[5];
    A[6] = cameraMatrix[6];  A[7] = cameraMatrix[7];  A[8] = cameraMatrix[8];

    if (newCameraMatrix)
    {
        Ar[0] = newCameraMatrix[0]; Ar[1] = newCameraMatrix[1]; Ar[2] = newCameraMatrix[2];
        Ar[3] = newCameraMatrix[3]; Ar[4] = newCameraMatrix[4]; Ar[5] = newCameraMatrix[5];
        Ar[6] = newCameraMatrix[6]; Ar[7] = newCameraMatrix[7]; Ar[8] = newCameraMatrix[8];
    }
    else
    {
        getDefaultNewCameraMatrixD(Ar, A, size_width, size_height, TRUE);
    }

    L[0] = 1.0;  L[1] = 0.0;  L[2] = 0.0;
    L[4] = 1.0;  L[5] = 0.0;
    L[8] = 1.0;
    U[0] = Ar[0]; U[1] = Ar[1]; U[2] = Ar[2];
    U[3] = 0.0;
    U[6] = 0.0;     U[7] = 0.0;

    L[3] = Ar[3] / U[0];
    L[6] = Ar[6] / U[0];

    U[4] = Ar[4] - L[3] * U[1];
    U[5] = Ar[5] - L[3] * U[2];

    L[7] = (Ar[7] - L[6] * U[1]) / U[4];

    U[8] = Ar[8] - L[6] * U[2] - L[7] * U[5];

    invU[3] = 0.0;
    invU[6] = 0.0;
    invU[7] = 0.0;
    invU[0] = 1.0 / U[0];
    invU[4] = 1.0 / U[4];
    invU[1] = -U[1] * invU[4] / U[0];
    invU[8] = 1.0 / U[8];
    invU[5] = -U[5] * invU[8] / U[4];
    invU[2] = -(U[1] * invU[5] + U[2] * invU[8]) / U[0];

    invL[0] = 1.0;      invL[1] = 0.0;   invL[2] = 0.0;
    invL[3] = -L[3] * invL[0];
    invL[6] = -(L[6] * invL[0] + L[7] * invL[3]);
    invL[4] = 1.0;//1.0 - L[3]*invL[1];
    invL[5] = 0.0;
    invL[7] = -L[7] * invL[4];//-L[7]*invL[4]-L[6]*invL[1];
    invL[8] = 1.0;//1.0-L[6]*invL[2]-L[7]*invL[5];

    iR[0] = invU[0] * invL[0] + invU[1] * invL[3] + invU[2] * invL[6];
    iR[1] = invU[0] * invL[1] + invU[1] * invL[4] + invU[2] * invL[7];
    iR[2] = invU[0] * invL[2] + invU[1] * invL[5] + invU[2] * invL[8];
    iR[3] = invU[3] * invL[0] + invU[4] * invL[3] + invU[5] * invL[6];
    iR[4] = invU[3] * invL[1] + invU[4] * invL[4] + invU[5] * invL[7];
    iR[5] = invU[3] * invL[2] + invU[4] * invL[5] + invU[5] * invL[8];
    iR[6] = invU[6] * invL[0] + invU[7] * invL[3] + invU[8] * invL[6];
    iR[7] = invU[6] * invL[1] + invU[7] * invL[4] + invU[8] * invL[7];
    iR[8] = invU[6] * invL[2] + invU[7] * invL[5] + invU[8] * invL[8];

    double u0 = A[2], v0 = A[5];
    double fx = A[0], fy = A[4];

    double k1 = distCoeffs[0];
    double k2 = distCoeffs[1];
    double p1 = distCoeffs[2];
    double p2 = distCoeffs[3];
    double k3 = distCoeffs[4];
    double k4 = distCoeffs[5];
    double k5 = distCoeffs[6];
    double k6 = distCoeffs[7];

    float* m1f = NULL;
    float* m2f = NULL;
    short* m1 = NULL;
    unsigned short* m2 = NULL;

    for (int i = 0; i < size_height; i++)
    {
        if (m1type == MAP_16SC2){
            m1 = (short*)map1 + map1_step*i;
            m2 = (unsigned short*)map2 + map2_step*i;
        }
        else{
            if (m1type == MAP_32FC1){
                m1f = (float*)map1 + map1_step*i;
                m2f = (float*)map2 + map2_step*i;
            }
            else{
                m1f = (float*)map1 + map1_step*i * 2;
            }
        }

        double _x = i*iR[1] + iR[2], _y = i*iR[4] + iR[5], _w = i*iR[7] + iR[8];

        for (int j = 0; j < size_width; j++, _x += iR[0], _y += iR[3], _w += iR[6])
        {
            double w = 1.0 / _w, x = _x*w, y = _y*w;
            double x2 = x*x, y2 = y*y;
            double r2 = x2 + y2, _2xy = 2 * x*y;
            double kr = (1 + ((k3*r2 + k2)*r2 + k1)*r2) / (1 + ((k6*r2 + k5)*r2 + k4)*r2);
            double u = fx*(x*kr + p1*_2xy + p2*(r2 + 2 * x2)) + u0;
            double v = fy*(y*kr + p1*(r2 + 2 * y2) + p2*_2xy) + v0;
            if (m1type == MAP_16SC2)
            {
                int iu = (int)floor(u*INTER_TAB_SIZE);
                int iv = (int)floor(v*INTER_TAB_SIZE);
                m1[j * 2] = (short)(iu >> INTER_BITS);
                m1[j * 2 + 1] = (short)(iv >> INTER_BITS);
                m2[j] = (unsigned short)((iv & (INTER_TAB_SIZE - 1))*INTER_TAB_SIZE + (iu & (INTER_TAB_SIZE - 1)));
            }
            else if (m1type == MAP_32FC1)
            {
                m1f[j] = (float)u;
                m2f[j] = (float)v;
            }
            else
            {
                m1f[j * 2] = (float)u;
                m1f[j * 2 + 1] = (float)v;
            }
        }
    }
}

void vsInitUndistortRectifyMap2(double* _cameraMatrix, double* _distCoeffs,
    double* _matR, double* _newCameraMatrix,
    int size_width, int size_height, int m1type,
    void *_map1, int map1_step, void* _map2, int map2_step, int blockW, int blockH)
{
    double *cameraMatrix = _cameraMatrix, *distCoeffs = _distCoeffs;
    double *newCameraMatrix = _newCameraMatrix;
    void *map1 = _map1;
    void *map2 = _map2;

    double A[9] = { 0.0 };
    double Ar[9] = { 0.0 };
    double L[9] = { 0.0 };
    double U[9] = { 0.0 };
    double invL[9] = { 0.0 };
    double invU[9] = { 0.0 };

    A[0] = cameraMatrix[0];  A[1] = cameraMatrix[1];  A[2] = cameraMatrix[2];
    A[3] = cameraMatrix[3];  A[4] = cameraMatrix[4];  A[5] = cameraMatrix[5];
    A[6] = cameraMatrix[6];  A[7] = cameraMatrix[7];  A[8] = cameraMatrix[8];

    if (newCameraMatrix)
    {
        Ar[0] = newCameraMatrix[0]; Ar[1] = newCameraMatrix[1]; Ar[2] = newCameraMatrix[2];
        Ar[3] = newCameraMatrix[3]; Ar[4] = newCameraMatrix[4]; Ar[5] = newCameraMatrix[5];
        Ar[6] = newCameraMatrix[6]; Ar[7] = newCameraMatrix[7]; Ar[8] = newCameraMatrix[8];
    }
    else
    {
        getDefaultNewCameraMatrixD(Ar, A, size_width, size_height, TRUE);
    }

    L[0] = 1.0;  L[1] = 0.0;  L[2] = 0.0;
    L[4] = 1.0;  L[5] = 0.0;
    L[8] = 1.0;
    U[0] = Ar[0]; U[1] = Ar[1]; U[2] = Ar[2];
    U[3] = 0.0;
    U[6] = 0.0;     U[7] = 0.0;

    L[3] = Ar[3] / U[0];
    L[6] = Ar[6] / U[0];

    U[4] = Ar[4] - L[3] * U[1];
    U[5] = Ar[5] - L[3] * U[2];

    L[7] = (Ar[7] - L[6] * U[1]) / U[4];

    U[8] = Ar[8] - L[6] * U[2] - L[7] * U[5];

    invU[3] = 0.0;
    invU[6] = 0.0;
    invU[7] = 0.0;
    invU[0] = 1.0 / U[0];
    invU[4] = 1.0 / U[4];
    invU[1] = -U[1] * invU[4] / U[0];
    invU[8] = 1.0 / U[8];
    invU[5] = -U[5] * invU[8] / U[4];
    invU[2] = -(U[1] * invU[5] + U[2] * invU[8]) / U[0];

    invL[0] = 1.0;      invL[1] = 0.0;   invL[2] = 0.0;
    invL[3] = -L[3] * invL[0];
    invL[6] = -(L[6] * invL[0] + L[7] * invL[3]);
    invL[4] = 1.0;//1.0 - L[3]*invL[1];
    invL[5] = 0.0;
    invL[7] = -L[7] * invL[4];//-L[7]*invL[4]-L[6]*invL[1];
    invL[8] = 1.0;//1.0-L[6]*invL[2]-L[7]*invL[5];

    double u0 = A[2], v0 = A[5];
    double fx = A[0], fy = A[4];

    double k1 = distCoeffs[0];
    double k2 = distCoeffs[1];
    double p1 = distCoeffs[2];
    double p2 = distCoeffs[3];
    double k3 = distCoeffs[4];
    double k4 = distCoeffs[5];
    double k5 = distCoeffs[6];
    double k6 = distCoeffs[7];

    float* m1f = NULL;
    float* m2f = NULL;
    short* m1 = NULL;
    unsigned short* m2 = NULL;

    int gridcols = (size_width%blockW) ? (size_width / blockW + 2) : (size_width / blockW + 1);
    int gridrows = (size_height%blockH) ? (size_height / blockH + 2) : (size_height / blockH + 1);

    double fyinv = 1.0 / Ar[4];
    double fxinv = 1.0 / Ar[0];
    for (int i = 0, r = 0; r < gridrows; i+=blockH, r++)
    {
        if ((gridrows - 1) == r)
            i = size_height - 1;
        if (m1type == MAP_16SC2){
            m1 = (short*)map1 + map1_step*i;
            m2 = (unsigned short*)map2 + map2_step*i;
        }
        else{
            if (m1type == MAP_32FC1){
                m1f = (float*)map1 + map1_step*i;
                m2f = (float*)map2 + map2_step*i;
            }
            else{
                m1f = (float*)map1 + map1_step*i * 2;
            }
        }

        double y = (i - Ar[5])*fyinv;
        for (int j = 0, c = 0; c < gridcols; j+=blockW, c++)
        {
            if ((gridcols - 1) == c)
                j = size_width - 1;
            double x = (j - Ar[2])*fxinv;
            double x2 = x*x, y2 = y*y;
            double r2 = x2 + y2, _2xy = 2 * x*y;
            double kr = (1 + ((k3*r2 + k2)*r2 + k1)*r2) / (1 + ((k6*r2 + k5)*r2 + k4)*r2);
            double u = fx*(x*kr + p1*_2xy + p2*(r2 + 2 * x2)) + u0;
            double v = fy*(y*kr + p1*(r2 + 2 * y2) + p2*_2xy) + v0;
            if (m1type == MAP_16SC2)
            {
                int iu = (int)floor(u*INTER_TAB_SIZE);
                int iv = (int)floor(v*INTER_TAB_SIZE);
                m1[j * 2] = (short)(iu >> INTER_BITS);
                m1[j * 2 + 1] = (short)(iv >> INTER_BITS);
                m2[j] = (unsigned short)((iv & (INTER_TAB_SIZE - 1))*INTER_TAB_SIZE + (iu & (INTER_TAB_SIZE - 1)));
            }
            else if (m1type == MAP_32FC1)
            {
                m1f[j] = (float)u;
                m2f[j] = (float)v;
            }
            else
            {
                m1f[j * 2] = (float)u;
                m1f[j * 2 + 1] = (float)v;
            }
        }
    }
}
//3:create normal lens dewarp map

int CreateUpdateDewarpMap(unsigned int* warpMap, int widthMap, int heightMap, int bitsMap, int fractionalBitMap, double *_cameraMatrix, double *_distCoeffs, int _width, int _height, float _fovRatio, int blockW, int blockH)
{
    if (!_cameraMatrix || !_distCoeffs)
        return -1;
    double *cameraMatrix = _cameraMatrix;
    double *distCoeffs = _distCoeffs;

    double newCameraMatrix[9];
    float fovRatio = 1.0f;
    if (_fovRatio > 0)
        fovRatio = _fovRatio;
    //copy camera matrix data to new camera matrix
    newCameraMatrix[0] = cameraMatrix[0] * fovRatio;
    newCameraMatrix[1] = cameraMatrix[1];
    newCameraMatrix[2] = cameraMatrix[2];
    newCameraMatrix[3] = cameraMatrix[3];
    newCameraMatrix[4] = cameraMatrix[4] * fovRatio;
    newCameraMatrix[5] = cameraMatrix[5];
    newCameraMatrix[6] = cameraMatrix[6];
    newCameraMatrix[7] = cameraMatrix[7];
    newCameraMatrix[8] = cameraMatrix[8];

    unsigned int *map1_part = warpMap;//x 
    unsigned int *map2_part = warpMap + widthMap*heightMap;//y

    double u0 = cameraMatrix[2], v0 = cameraMatrix[5];
    double fx = cameraMatrix[0], fy = cameraMatrix[4];

    double k1 = distCoeffs[0];
    double k2 = distCoeffs[1];
    double p1 = distCoeffs[2];
    double p2 = distCoeffs[3];
    double k3 = distCoeffs[4];
    double k4 = distCoeffs[5];
    double k5 = distCoeffs[6];
    double k6 = distCoeffs[7];

    int size_width = _width;
    int size_height = _height;
    int fracVaule = (1 << fractionalBitMap);
    int mapvaluex, mapvaluey, mapvalue;

    int gridcols = (size_width%blockW) ? (size_width / blockW + 2) : (size_width / blockW + 1);
    int gridrows = (size_height%blockH) ? (size_height / blockH + 2) : (size_height / blockH + 1);

    double u0_new = newCameraMatrix[2], v0_new = newCameraMatrix[5];
    double fx_new = newCameraMatrix[0], fy_new = newCameraMatrix[4];
    double fxinv = 1.0 / fx_new;
    double fyinv = 1.0 / fy_new;

    for (int i = 0, r = 0; r < gridrows; i += blockH, r++)
    {
        if ((gridrows - 1) == r)
            i = size_height - 1;

        double y = (i - v0_new)*fyinv;
        for (int j = 0, c = 0; c < gridcols; j += blockW, c++)
        {
            if ((gridcols - 1) == c)
                j = size_width - 1;
            double x = (j - u0_new)*fxinv;
            double x2 = x*x, y2 = y*y;
            double r2 = x2 + y2, _2xy = 2 * x*y;
            double kr = (1 + ((k3*r2 + k2)*r2 + k1)*r2) / (1 + ((k6*r2 + k5)*r2 + k4)*r2);
            double u = fx*(x*kr + p1*_2xy + p2*(r2 + 2 * x2)) + u0;
            double v = fy*(y*kr + p1*(r2 + 2 * y2) + p2*_2xy) + v0;
            if((int)(u*fracVaule) < 0)
                u=0;
            if((int)(v*fracVaule) < 0)
                v=0;
            if (bitsMap<17)
            {
                mapvaluex = ((int)(u*fracVaule) & 0xffff);
                mapvaluey = ((int)(v*fracVaule) & 0xffff);
                mapvalue = ((mapvaluey << 16) | mapvaluex);
                map1_part[r*widthMap + c] = (unsigned int)mapvalue;
            }
            else if (bitsMap < 32)
            {
                mapvaluex = ((int)(u*fracVaule));
                mapvaluey = ((int)(v*fracVaule));
                map1_part[r*widthMap + c] = mapvaluex;
                map2_part[r*widthMap + c] = mapvaluey;
            }
        }
    }
    return 0;
}

void fisheyeInitUndistortRectifyMap(double* camMat, double* distV, double* R, double* newcamMat,
    int size_width, int size_height, int m1type, float* map1, float* map2)
{
    double f[2] = {0};
    double c[2] = {0};
    if (camMat)
    {
        f[0] = camMat[0];
        f[1] = camMat[4];
        c[0] = camMat[2];
        c[1] = camMat[5];
    }

    double k[4] = { 0.0 };
    if (distV)
    {
        k[0] = distV[0];
        k[1] = distV[1];
        k[2] = distV[2];
        k[3] = distV[3];
    }

    double PP[9] = { 1.0, 0.0, 0.0, 0.0, 1.0, 0.0, 0.0, 0.0, 1.0 };
    if (newcamMat)
    {
        PP[0] = newcamMat[0];    PP[1] = newcamMat[1];    PP[2] = newcamMat[2];
        PP[3] = newcamMat[3];    PP[4] = newcamMat[4];    PP[5] = newcamMat[5];
        PP[6] = newcamMat[6];    PP[7] = newcamMat[7];    PP[8] = newcamMat[8];
    }

    double fxinv = 1.0 / PP[0];// 1/fx
    double fyinv = 1.0 / PP[4];// 1/fy

    for (int i = 0; i < size_height; ++i)
    {
        float* m1f = map1 + i*size_width;
        float* m2f = map2 + i*size_width;
        short*  m1 = (short*)m1f;
        ushort* m2 = (ushort*)m2f;

        /*double _x = i*iR(0, 1) + iR(0, 2),
        _y = i*iR(1, 1) + iR(1, 2),
        _w = i*iR(2, 1) + iR(2, 2);*/

        double _y = (i - PP[5])*fyinv;

        for (int j = 0; j < size_width; ++j)
        {
            //double x = _x / _w, y = _y / _w;
            double _x = (j - PP[2])*fxinv;

            double x = _x, y = _y;

            double r = sqrt(x*x + y*y);
            double theta = atan(r);

            double theta2 = theta*theta, theta4 = theta2*theta2, theta6 = theta4*theta2, theta8 = theta4*theta4;
            double theta_d = theta * (1 + k[0] * theta2 + k[1] * theta4 + k[2] * theta6 + k[3] * theta8);

            double scale = (r == 0) ? 1.0 : theta_d / r;
            double u = f[0] * x*scale + c[0];
            double v = f[1] * y*scale + c[1];

            if (m1type == MAP_16SC2)
            {
                int iu = (int)(u*INTER_TAB_SIZE);
                int iv = (int)(v*INTER_TAB_SIZE);
                m1[j * 2 + 0] = (short)(iu >> INTER_BITS);
                m1[j * 2 + 1] = (short)(iv >> INTER_BITS);
                m2[j] = (ushort)((iv & (INTER_TAB_SIZE - 1))*INTER_TAB_SIZE + (iu & (INTER_TAB_SIZE - 1)));
            }
            else if (m1type == MAP_32FC1)
            {
                m1f[j] = (float)u;
                m2f[j] = (float)v;
            }
            //_x += iR(0, 0);
            //_y += iR(1, 0);
            //_w += iR(2, 0);
        }
    }
}
void fisheyeInitUndistortRectifyMap2(double* camMat, double* distV, double* R, double* newcamMat,
    int size_width, int size_height, int m1type, float* map1, float* map2, int blockW, int blockH)
{
    double f[2] = {0};
    double c[2] = {0};
    if (camMat)
    {
        f[0] = camMat[0];
        f[1] = camMat[4];
        c[0] = camMat[2];
        c[1] = camMat[5];
    }

    double k[4] = { 0.0 };
    if (distV)
    {
        k[0] = distV[0];
        k[1] = distV[1];
        k[2] = distV[2];
        k[3] = distV[3];
    }

    double PP[9] = { 1.0, 0.0, 0.0, 0.0, 1.0, 0.0, 0.0, 0.0, 1.0 };
    if (newcamMat)
    {
        PP[0] = newcamMat[0];    PP[1] = newcamMat[1];    PP[2] = newcamMat[2];
        PP[3] = newcamMat[3];    PP[4] = newcamMat[4];    PP[5] = newcamMat[5];
        PP[6] = newcamMat[6];    PP[7] = newcamMat[7];    PP[8] = newcamMat[8];
    }

    double fxinv = 1.0 / PP[0];// 1/fx
    double fyinv = 1.0 / PP[4];// 1/fy

    int cols = (size_width%blockW) ? (size_width / blockW + 2) : (size_width / blockW + 1);
    int rows = (size_height%blockH) ? (size_height / blockH + 2) : (size_height / blockH + 1);

    for (int i = 0, rr = 0; rr < rows;  i+=blockH, rr++)
    {
        if ((rows - 1) == rr)
            i = size_height - 1;
        float* m1f = map1 + i*size_width;
        float* m2f = map2 + i*size_width;
        short*  m1 = (short*)m1f;
        ushort* m2 = (ushort*)m2f;

        /*double _x = i*iR(0, 1) + iR(0, 2),
        _y = i*iR(1, 1) + iR(1, 2),
        _w = i*iR(2, 1) + iR(2, 2);*/

        double _y = (i - PP[5])*fyinv;

        for (int j = 0, cc = 0; cc < cols; j+=blockW, cc++ )
        {
            if ((cols - 1) == cc)
                j = size_width - 1;
            //double x = _x / _w, y = _y / _w;
            double _x = (j - PP[2])*fxinv;

            double x = _x, y = _y;

            double r = sqrt(x*x + y*y);
            double theta = atan(r);

            double theta2 = theta*theta, theta4 = theta2*theta2, theta6 = theta4*theta2, theta8 = theta4*theta4;
            double theta_d = theta * (1 + k[0] * theta2 + k[1] * theta4 + k[2] * theta6 + k[3] * theta8);

            double scale = (r == 0) ? 1.0 : theta_d / r;
            double u = f[0] * x*scale + c[0];
            double v = f[1] * y*scale + c[1];

            if (m1type == MAP_16SC2)
            {
                int iu = (int)(u*INTER_TAB_SIZE);
                int iv = (int)(v*INTER_TAB_SIZE);
                m1[j * 2 + 0] = (short)(iu >> INTER_BITS);
                m1[j * 2 + 1] = (short)(iv >> INTER_BITS);
                m2[j] = (ushort)((iv & (INTER_TAB_SIZE - 1))*INTER_TAB_SIZE + (iu & (INTER_TAB_SIZE - 1)));
            }
            else if (m1type == MAP_32FC1)
            {
                m1f[j] = (float)u;
                m2f[j] = (float)v;
            }
            //_x += iR(0, 0);
            //_y += iR(1, 0);
            //_w += iR(2, 0);
        }
    }
}
//4:create fisheye lens mode map

int CreateUpdateFisheyeDewarpMap(unsigned int* warpMap, int widthMap, int heightMap, int bitsMap, int fractionalBitMap, double *_cameraMatrix, double *_distCoeffs, int _width, int _height, float _fovRatio, int blockW, int blockH)
{
    if (!_cameraMatrix || !_distCoeffs)
        return -1;
    int img_width = _width;
    int img_height = _height;
    double *cameraMatrix = _cameraMatrix;
    double *distCoeffs = _distCoeffs;
    double newcameraMatrix[9] = { 1.0, 0.0, 0.0, 0.0, 1.0, 0.0, 0.0, 0.0, 1.0 };
    double fovRatio = 1.0;
    int fracVaule = (1 << fractionalBitMap);
    int mapvaluex, mapvaluey, mapvalue;

    if (_fovRatio > 0.0f)
        fovRatio = _fovRatio;
    //copy camera matrix to new camera matrix
    for (int i = 0; i < 9; i++)
    {
        newcameraMatrix[i] = cameraMatrix[i];
    }
    newcameraMatrix[0] *= fovRatio;//(0,0)
    newcameraMatrix[4] *= fovRatio;//(1,1)
    newcameraMatrix[2] = 0.5*img_width;//(0,2)
    newcameraMatrix[5] = 0.5*img_height;//(1,2)

    unsigned int* pfxMap = warpMap;
    unsigned int* pfyMap = warpMap + widthMap*heightMap;
    double f[2], c[2];
    f[0] = cameraMatrix[0];
    f[1] = cameraMatrix[4];
    c[0] = cameraMatrix[2];
    c[1] = cameraMatrix[5];
    double k[4] = { 0.0 };
    k[0] = distCoeffs[0];
    k[1] = distCoeffs[1];
    k[2] = distCoeffs[2];
    k[3] = distCoeffs[3];
    double PP[9] = { 1.0, 0.0, 0.0, 0.0, 1.0, 0.0, 0.0, 0.0, 1.0 };
    PP[0] = newcameraMatrix[0];    PP[1] = newcameraMatrix[1];    PP[2] = newcameraMatrix[2];
    PP[3] = newcameraMatrix[3];    PP[4] = newcameraMatrix[4];    PP[5] = newcameraMatrix[5];
    PP[6] = newcameraMatrix[6];    PP[7] = newcameraMatrix[7];    PP[8] = newcameraMatrix[8];

    double fxinv = 1.0 / PP[0];// 1/fx
    double fyinv = 1.0 / PP[4];// 1/fy

    int cols = (img_width%blockW) ? (img_width / blockW + 2) : (img_width / blockW + 1);
    int rows = (img_height%blockH) ? (img_height / blockH + 2) : (img_height / blockH + 1);

    for (int i = 0, rr = 0; rr < rows; i += blockH, rr++)
    {
        if ((rows - 1) == rr)
            i = img_height - 1;

        double _y = (i - PP[5])*fyinv;
        for (int j = 0, cc = 0; cc < cols; j += blockW, cc++)
        {
            if ((cols - 1) == cc)
                j = img_width - 1;
            //double x = _x / _w, y = _y / _w;
            double _x = (j - PP[2])*fxinv;
            double x = _x, y = _y;

            double r = sqrt(x*x + y*y);
            double theta = atan(r);

            double theta2 = theta*theta, theta4 = theta2*theta2, theta6 = theta4*theta2, theta8 = theta4*theta4;
            double theta_d = theta * (1 + k[0] * theta2 + k[1] * theta4 + k[2] * theta6 + k[3] * theta8);

            double scale = (r == 0) ? 1.0 : theta_d / r;
            double u = f[0] * x*scale + c[0];
            double v = f[1] * y*scale + c[1];

            if (bitsMap<17)
            {
                mapvaluex = ((int)(u*fracVaule) & 0xffff);
                mapvaluey = ((int)(v*fracVaule) & 0xffff);
                mapvalue = ((mapvaluey << 16) | mapvaluex);
                pfxMap[rr*widthMap + cc] = (unsigned int)mapvalue;
            }
            else if (bitsMap < 32)
            {
                mapvaluex = ((int)(u*fracVaule));
                mapvaluey = ((int)(v*fracVaule));
                pfxMap[rr*widthMap + cc] = mapvaluex;
                pfyMap[rr*widthMap + cc] = mapvaluey;
            }
        }
    }
    return 0;
}

//5: create perspective map
int CreateUpdatePerspectiveMap(unsigned int* warpMap, int widthMap, int heightMap, int bitsMap, int fractionalBitMap, double *_M0,
        int _width, int _height, int blockW, int blockH, float scalefx, float scalefy, float shiftx, float shifty)
{
    if (!_M0 || !warpMap)
        return 1;

    int width = _width;
    int height = _height;
    double M0[9] = { 0 };
    double invM[9] = { 1, 0, 0, 0, 1, 0, 0, 0, 1 };
    VSMatrix matM, invmatM;
    M0[0] = _M0[0];
    M0[1] = _M0[1];
    M0[2] = _M0[2];
    M0[3] = _M0[3];
    M0[4] = _M0[4];
    M0[5] = _M0[5];
    M0[6] = _M0[6];
    M0[7] = _M0[7];
    M0[8] = _M0[8];

    matM.cols = 3;
    matM.rows = 3;
    matM.type = VS_64F;
    matM.step = matM.cols * sizeof(double);
    matM.data = M0;

    invmatM.cols = 3;
    invmatM.rows = 3;
    invmatM.type = VS_64F;
    invmatM.step = invmatM.cols * sizeof(double);
    invmatM.data = invM;

    vsInvert(&matM, &invmatM, VS_DECOMP_LU);

    int mapvaluex, mapvaluey, mapvalue;
    int fracVaule = (1 << fractionalBitMap);

    unsigned int *pfxMap = warpMap;
    unsigned int *pfyMap = (warpMap + widthMap*heightMap);

    int cols = (width%blockW) ? (width / blockW + 2) : (width / blockW + 1);
    int rows = (height%blockH) ? (height / blockH + 2) : (height / blockH + 1);

    //create perspective map
    for (int y = 0, j = 0; j < rows; y += blockH, j++)
    {
        if ((rows - 1) == j)
            y = height - 1;
        for (int x = 0, i = 0; i < cols; x += blockW, i++)
        {
            if ((cols - 1) == i)
                x = width - 1;

            double x0 = invM[0] * (x*scalefx + shiftx) + invM[1] * (y*scalefy + shifty) + invM[2];
            double y0 = invM[3] * (x*scalefx + shiftx) + invM[4] * (y*scalefy + shifty) + invM[5];
            double w0 = invM[6] * (x*scalefx + shiftx) + invM[7] * (y*scalefy + shifty) + invM[8];
            double w = w0 ? 1. / w0 : 0;
            double fx = x0*w;
            double fy = y0*w;

            if (bitsMap<17)
            {
                mapvaluex = ((int)(fx*fracVaule) & 0xffff);
                mapvaluey = ((int)(fy*fracVaule) & 0xffff);
                mapvalue = ((mapvaluey << 16) | mapvaluex);
                pfxMap[j*widthMap + i] = (unsigned int)mapvalue;
            }
            else if (bitsMap < 32)
            {
                mapvaluex = ((int)(fx*fracVaule));
                mapvaluey = ((int)(fy*fracVaule));
                pfxMap[j*widthMap + i] = mapvaluex;
                pfyMap[j*widthMap + i] = mapvaluey;
            }
        }
    }

    return 0;
}
int clipSizef(float* fin, float fmin, float fmax)
{
    if (fin)
    {
        if (*fin < fmin)
            *fin = fmin;
        if (*fin > fmax)
            *fin = fmax;
    }
    return 0;
}
int CreateGridMapInt(float* srcMapx, float* srcMapy, GridMapS* gridMap, int width, int height, int blockWidth, int blockHeight, int fractionalBit,
    int splitS, int lineH, int lineVU, int lineVD, int widthSrc, int heightSrc)
{
    if (!srcMapx || !srcMapy || !gridMap)
    {
        return -1;
    }

    if ((width < 1) || (height < 1) || (blockWidth < 1) || (blockHeight < 1) || (width<blockWidth) || (height<blockHeight))
        return -1;

    int isClipValue = 0;

    int gridCols = width / blockWidth;
    int gridRows = height / blockHeight;

    int offsetw = width%blockWidth;
    int offseth = height%blockHeight;

    if (offsetw)
        gridCols += 1;
    if (offseth)
        gridRows += 1;

    int mapWidth = gridCols + 1;
    int mapHeight = gridRows + 1;
    if (splitS)
    {
        mapWidth++;
        mapHeight++;
    }

    gridMap->block_width = blockWidth;
    gridMap->block_height = blockHeight;
    gridMap->block_last_w = (offsetw ? offsetw : blockWidth);
    gridMap->block_last_h = (offseth ? offseth : blockHeight);
    gridMap->cols = gridCols;//block num cols
    gridMap->rows = gridRows;//block num rows
    gridMap->dataType = INT_MAP;
    gridMap->pGridmap = (void *)malloc(sizeof(int)*(mapWidth)*(mapHeight)* 2);
    gridMap->line_horizontal = 0;
    gridMap->line_vertical_up = 0;
    gridMap->line_vertical_down = 0;
    gridMap->mapWidth = mapWidth;
    gridMap->mapHeight = mapHeight;

    int line_h = 0;
    int line_v1 = 0;
    int line_v2 = 0;
    int half_width = width / 2;
    int half_height = height / 2;
    int half_bw = blockWidth / 2;
    int half_bh = blockHeight / 2;
    int blockd = (half_height + half_bh) / blockHeight;
    int blockr = (half_width + half_bw) / blockWidth;
    //v1 black line at image resolution center
    line_h = blockd*blockHeight - 4;
    line_v1 = blockr*blockWidth - 4;
    line_v2 = line_v1;

    //v2 black line at image every where
    int lineH_at_block = (lineH + half_bh) / blockHeight;
    int lineVU_at_block = (lineVU + half_bw) / blockWidth;
    int lineVD_at_block = (lineVD + half_bw) / blockWidth;
    line_h = lineH_at_block*blockHeight - 4;
    line_v1 = lineVU_at_block*blockWidth - 4;
    line_v2 = lineVD_at_block*blockWidth - 4;

    if (splitS)
    {
        gridMap->line_horizontal = (line_h>0 ? line_h : 0);
        gridMap->line_vertical_up = (line_v1>0 ? line_v1 : 0);
        gridMap->line_vertical_down = (line_v2>0 ? line_v2 : 0);
    }
    else
    {
        gridMap->line_horizontal = 0xffff;
        gridMap->line_vertical_up = 0xffff;
        gridMap->line_vertical_down = 0xffff;
    }

    int fracVaule = (1 << fractionalBit);

    float fx, fy;
    int ix, iy;
    int sumw, sumh;

    int* pgridmapx = (int *)gridMap->pGridmap;
    int* pgridmapy = (int *)gridMap->pGridmap + mapWidth*mapHeight;
    //for (int c = 0; c<gridChannels; c++)
    int x = 0;
    int y = 0;
    int i = 0;
    int j = 0;
    if (!splitS)
    {
        sumh = 0;
        for (y = 0; y<gridRows; y++)
        {
            //int bh = ((y == (gridRows - 1)) ? (offseth ? offseth : blockHeight) : blockHeight);
            //sumh += (y>0 ? bh : 0);
            sumh += (y>0 ? blockHeight : 0);
            sumw = 0;
            for (x = 0; x<gridCols; x++)
            {
                //int bw = ((x == (gridCols - 1)) ? (offsetw ? offsetw : blockWidth) : blockWidth);
                //sumw += (x>0 ? bw : 0);
                sumw += (x>0 ? blockWidth : 0);
                fx = *(srcMapx + sumh*width + sumw);
                fy = *(srcMapy + sumh*width + sumw);

                if (isClipValue)
                {
                    if (fx < 0)    fx = 0.0f;
                    if (fx >(widthSrc - 1))    fx = widthSrc - 1;
                    if (fy < 0)    fy = 0.0f;
                    if (fy >(heightSrc - 1)) fy = heightSrc - 1;
                }

                ix = (int)(fx*fracVaule);
                iy = (int)(fy*fracVaule);

                pgridmapx[y*mapWidth + x] = ix;
                pgridmapy[y*mapWidth + x] = iy;
            }

            fx = *(srcMapx + sumh*width + width - 1);
            fy = *(srcMapy + sumh*width + width - 1);
            if (isClipValue)
            {
                if (fx < 0)    fx = 0.0f;
                if (fx >(widthSrc - 1))    fx = widthSrc - 1;
                if (fy < 0)    fy = 0.0f;
                if (fy >(heightSrc - 1)) fy = heightSrc - 1;
            }
            ix = (int)(fx*fracVaule);
            iy = (int)(fy*fracVaule);
            pgridmapx[y*mapWidth + mapWidth - 1] = ix;
            pgridmapy[y*mapWidth + mapWidth - 1] = iy;
        }

        sumw = 0;
        for (x = 0; x<gridCols; x++)
        {
            sumw += (x>0 ? blockWidth : 0);
            fx = *(srcMapx + (height - 1)*width + sumw);
            fy = *(srcMapy + (height - 1)*width + sumw);
            if (isClipValue)
            {
                if (fx < 0)    fx = 0.0f;
                if (fx >(widthSrc - 1))    fx = widthSrc - 1;
                if (fy < 0)    fy = 0.0f;
                if (fy >(heightSrc - 1)) fy = heightSrc - 1;
            }
            ix = (int)(fx*fracVaule);
            iy = (int)(fy*fracVaule);
            pgridmapx[gridRows*mapWidth + x] = ix;
            pgridmapy[gridRows*mapWidth + x] = iy;
        }

        fx = *(srcMapx + (height - 1)*width + width - 1);
        fy = *(srcMapy + (height - 1)*width + width - 1);
        if (isClipValue)
        {
            if (fx < 0)    fx = 0.0f;
            if (fx >(widthSrc - 1))    fx = widthSrc - 1;
            if (fy < 0)    fy = 0.0f;
            if (fy >(heightSrc - 1)) fy = heightSrc - 1;
        }
        ix = (int)(fx*fracVaule);
        iy = (int)(fy*fracVaule);
        pgridmapx[gridRows*mapWidth + mapWidth - 1] = ix;
        pgridmapy[gridRows*mapWidth + mapWidth - 1] = iy;
    }
    else
    {
        sumh = 0;
        for (y = 0, j = 0; y < gridRows; y++, j++)
        {
            sumh += (y > 0 ? blockHeight : 0);
            if (y == lineH_at_block)
            {
                sumw = 0;
                for (x = 0, i = 0; x < gridCols; x++, i++)
                {
                    sumw += (x > 0 ? blockWidth : 0);
                    if (x == lineVU_at_block)
                    {
                        fx = *(srcMapx + (sumh - 1)*width + sumw - 1);
                        fy = *(srcMapy + (sumh - 1)*width + sumw - 1);
                        clipSizef(&fx, 0.0f, widthSrc - 1);
                        clipSizef(&fy, 0.0f, heightSrc - 1);
                        ix = (int)(fx*fracVaule);
                        iy = (int)(fy*fracVaule);
                        pgridmapx[j*mapWidth + i] = ix;
                        pgridmapy[j*mapWidth + i] = iy;

                        i++;
                    }
                    fx = *(srcMapx + (sumh - 1)*width + sumw);
                    fy = *(srcMapy + (sumh - 1)*width + sumw);
                    clipSizef(&fx, 0.0f, widthSrc - 1);
                    clipSizef(&fy, 0.0f, heightSrc - 1);
                    ix = (int)(fx*fracVaule);
                    iy = (int)(fy*fracVaule);
                    pgridmapx[j*mapWidth + i] = ix;
                    pgridmapy[j*mapWidth + i] = iy;
                }

                fx = *(srcMapx + (sumh - 1)*width + width - 1);
                fy = *(srcMapy + (sumh - 1)*width + width - 1);
                clipSizef(&fx, 0.0f, widthSrc - 1);
                clipSizef(&fy, 0.0f, heightSrc - 1);
                ix = (int)(fx*fracVaule);
                iy = (int)(fy*fracVaule);
                pgridmapx[j*mapWidth + i] = ix;
                pgridmapy[j*mapWidth + i] = iy;

                j++;
                sumw = 0;
                for (x = 0, i = 0; x < gridCols; x++, i++)
                {
                    sumw += (x > 0 ? blockWidth : 0);
                    if (x == lineVD_at_block)
                    {
                        fx = *(srcMapx + sumh*width + sumw - 1);
                        fy = *(srcMapy + sumh*width + sumw - 1);
                        clipSizef(&fx, 0.0f, widthSrc - 1);
                        clipSizef(&fy, 0.0f, heightSrc - 1);
                        ix = (int)(fx*fracVaule);
                        iy = (int)(fy*fracVaule);
                        pgridmapx[j*mapWidth + i] = ix;
                        pgridmapy[j*mapWidth + i] = iy;

                        i++;
                    }
                    fx = *(srcMapx + sumh*width + sumw);
                    fy = *(srcMapy + sumh*width + sumw);
                    clipSizef(&fx, 0.0f, widthSrc - 1);
                    clipSizef(&fy, 0.0f, heightSrc - 1);
                    ix = (int)(fx*fracVaule);
                    iy = (int)(fy*fracVaule);
                    pgridmapx[j*mapWidth + i] = ix;
                    pgridmapy[j*mapWidth + i] = iy;
                }
            }
            else if (y < lineH_at_block)
            {
                sumw = 0;
                for (x = 0, i = 0; x < gridCols; x++, i++)
                {
                    sumw += (x > 0 ? blockWidth : 0);
                    if (x == lineVU_at_block)
                    {
                        fx = *(srcMapx + sumh*width + sumw - 1);
                        fy = *(srcMapy + sumh*width + sumw - 1);
                        clipSizef(&fx, 0.0f, widthSrc - 1);
                        clipSizef(&fy, 0.0f, heightSrc - 1);
                        ix = (int)(fx*fracVaule);
                        iy = (int)(fy*fracVaule);
                        pgridmapx[j*mapWidth + i] = ix;
                        pgridmapy[j*mapWidth + i] = iy;

                        i++;
                    }
                    fx = *(srcMapx + sumh*width + sumw);
                    fy = *(srcMapy + sumh*width + sumw);

                    clipSizef(&fx, 0.0f, widthSrc - 1);
                    clipSizef(&fy, 0.0f, heightSrc - 1);
                    ix = (int)(fx*fracVaule);
                    iy = (int)(fy*fracVaule);
                    pgridmapx[j*mapWidth + i] = ix;
                    pgridmapy[j*mapWidth + i] = iy;
                }
            }
            else if (y > lineH_at_block)
            {
                sumw = 0;
                for (x = 0, i = 0; x < gridCols; x++, i++)
                {
                    sumw += (x > 0 ? blockWidth : 0);
                    if (x == lineVD_at_block)
                    {
                        fx = *(srcMapx + sumh*width + sumw - 1);
                        fy = *(srcMapy + sumh*width + sumw - 1);
                        clipSizef(&fx, 0.0f, widthSrc - 1);
                        clipSizef(&fy, 0.0f, heightSrc - 1);
                        ix = (int)(fx*fracVaule);
                        iy = (int)(fy*fracVaule);
                        pgridmapx[j*mapWidth + i] = ix;
                        pgridmapy[j*mapWidth + i] = iy;

                        i++;
                    }
                    fx = *(srcMapx + sumh*width + sumw);
                    fy = *(srcMapy + sumh*width + sumw);
                    clipSizef(&fx, 0.0f, widthSrc - 1);
                    clipSizef(&fy, 0.0f, heightSrc - 1);
                    ix = (int)(fx*fracVaule);
                    iy = (int)(fy*fracVaule);
                    pgridmapx[j*mapWidth + i] = ix;
                    pgridmapy[j*mapWidth + i] = iy;
                }
            }

            fx = *(srcMapx + sumh*width + width - 1);
            fy = *(srcMapy + sumh*width + width - 1);
            clipSizef(&fx, 0.0f, widthSrc - 1);
            clipSizef(&fy, 0.0f, heightSrc - 1);
            ix = (int)(fx*fracVaule);
            iy = (int)(fy*fracVaule);
            pgridmapx[j*mapWidth + i] = ix;
            pgridmapy[j*mapWidth + i] = iy;
        }

        sumw = 0;
        for (x = 0, i = 0; x < gridCols; x++, i++)
        {
            sumw += (x > 0 ? blockWidth : 0);
            if (x == lineVD_at_block)
            {
                fx = *(srcMapx + (height - 1)*width + sumw - 1);
                fy = *(srcMapy + (height - 1)*width + sumw - 1);
                clipSizef(&fx, 0.0f, widthSrc - 1);
                clipSizef(&fy, 0.0f, heightSrc - 1);
                ix = (int)(fx*fracVaule);
                iy = (int)(fy*fracVaule);
                pgridmapx[j*mapWidth + i] = ix;
                pgridmapy[j*mapWidth + i] = iy;

                i++;
            }
            fx = *(srcMapx + (height - 1)*width + sumw);
            fy = *(srcMapy + (height - 1)*width + sumw);
            clipSizef(&fx, 0.0f, widthSrc - 1);
            clipSizef(&fy, 0.0f, heightSrc - 1);
            ix = (int)(fx*fracVaule);
            iy = (int)(fy*fracVaule);
            pgridmapx[j*mapWidth + i] = ix;
            pgridmapy[j*mapWidth + i] = iy;
        }
        fx = *(srcMapx + (height - 1)*width + width - 1);
        fy = *(srcMapy + (height - 1)*width + width - 1);
        clipSizef(&fx, 0.0f, widthSrc - 1);
        clipSizef(&fy, 0.0f, heightSrc - 1);
        ix = (int)(fx*fracVaule);
        iy = (int)(fy*fracVaule);
        pgridmapx[j*mapWidth + i] = ix;
        pgridmapy[j*mapWidth + i] = iy;
    }

    return 0;
}

void ReleaseGridMap(GridMapS* gridMap)
{
    if (gridMap) {
        gridMap->cols = 0;
        gridMap->rows = 0;
        gridMap->block_width = 0;
        gridMap->block_height = 0;
        gridMap->block_last_w = 0;
        gridMap->block_last_h = 0;
        gridMap->dataType = 0;
        gridMap->line_vertical_up = 0;
        gridMap->line_vertical_down = 0;
        gridMap->line_horizontal = 0;
        if (gridMap->pGridmap)
        {
            free(gridMap->pGridmap);
            gridMap->pGridmap = NULL;
        }
    }
}
