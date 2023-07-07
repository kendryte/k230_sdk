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

#include <stdlib.h>
#include <iostream>
#include "basic.h"
#include <stdexcept>
#include <limits>
#include <cmath>

using  namespace  std;

/****************************************************************************************\
*        Inverse (or pseudo-inverse) of a matrix, now the inverse cols rows <=3          *
\****************************************************************************************/
#define det2(m)   ((double)m(0,0)*m(1,1) - (double)m(0,1)*m(1,0))
#define det3(m)   (m(0,0)*((double)m(1,1)*m(2,2) - (double)m(1,2)*m(2,1)) -  \
                   m(0,1)*((double)m(1,0)*m(2,2) - (double)m(1,2)*m(2,0)) +  \
                   m(0,2)*((double)m(1,0)*m(2,1) - (double)m(1,1)*m(2,0)))
#define Sf( y, x ) ((float*)(srcdata + y*srcstep))[x]
#define Sd( y, x ) ((double*)(srcdata + y*srcstep))[x]
#define Df( y, x ) ((float*)(dstdata + y*dststep))[x]
#define Dd( y, x ) ((double*)(dstdata + y*dststep))[x]

/****************************************************************************************\
*                     LU & Cholesky implementation for small matrices                    *
\****************************************************************************************/

template<typename _Tp> static inline int
vsLUImpl(_Tp* A, size_t astep, int m, _Tp* b, size_t bstep, int n)
{
    int i, j, k, p = 1;
    astep /= sizeof(A[0]);
    bstep /= sizeof(b[0]);

    for( i = 0; i < m; i++ )
    {
        k = i;
        for( j = i+1; j < m; j++ )
            if( std::abs(A[j*astep + i]) > std::abs(A[k*astep + i]) )
                k = j;
        if( std::abs(A[k*astep + i]) < std::numeric_limits<_Tp>::epsilon() )
            return 0;

        if( k != i )
        {
            for( j = i; j < m; j++ )
                std::swap(A[i*astep + j], A[k*astep + j]);
            if( b )
                for( j = 0; j < n; j++ )
                    std::swap(b[i*bstep + j], b[k*bstep + j]);
            p = -p;
        }
        _Tp d = -1/A[i*astep + i];
        for( j = i+1; j < m; j++ )
        {
            _Tp alpha = A[j*astep + i]*d;
            for( k = i+1; k < m; k++ )
                A[j*astep + k] += alpha*A[i*astep + k];

            if( b )
                for( k = 0; k < n; k++ )
                    b[j*bstep + k] += alpha*b[i*bstep + k];
        }
        A[i*astep + i] = -d;
    }
    if( b )
    {
        for( i = m-1; i >= 0; i-- )
            for( j = 0; j < n; j++ )
            {
                _Tp s = b[i*bstep + j];
                for( k = i+1; k < m; k++ )
                    s -= A[i*astep + k]*b[k*bstep + j];
                b[i*bstep + j] = s*A[i*astep + i];
            }
    }

    return p;
}

int vsLU(float* A, size_t astep, int m, float* b, size_t bstep, int n)
{
    return vsLUImpl(A, astep, m, b, bstep, n);
}

int vsLU(double* A, size_t astep, int m, double* b, size_t bstep, int n)
{
    return vsLUImpl(A, astep, m, b, bstep, n);
}

template<typename _Tp> static inline bool
vsCholImpl(_Tp* A, size_t astep, int m, _Tp* b, size_t bstep, int n)
{
    _Tp* L = A;
    int i, j, k;
    double s;
    astep /= sizeof(A[0]);
    bstep /= sizeof(b[0]);

    for( i = 0; i < m; i++ )
    {
        for( j = 0; j < i; j++ )
        {
            s = A[i*astep + j];
            for( k = 0; k < j; k++ )
                s -= L[i*astep + k]*L[j*astep + k];
            L[i*astep + j] = (_Tp)(s*L[j*astep + j]);
        }
        s = A[i*astep + i];
        for( k = 0; k < j; k++ )
        {
            double t = L[i*astep + k];
            s -= t*t;
        }
        if( s < std::numeric_limits<_Tp>::epsilon() )
            return false;
        L[i*astep + i] = (_Tp)(1./std::sqrt(s));
    }

    if( !b )
        return true;

    // LLt x = b
    // 1: L y = b
    // 2. Lt x = y

    /*
     [ L00             ]  y0   b0
     [ L10 L11         ]  y1 = b1
     [ L20 L21 L22     ]  y2   b2
     [ L30 L31 L32 L33 ]  y3   b3

     [ L00 L10 L20 L30 ]  x0   y0
     [     L11 L21 L31 ]  x1 = y1
     [         L22 L32 ]  x2   y2
     [             L33 ]  x3   y3
    */

    for( i = 0; i < m; i++ )
    {
        for( j = 0; j < n; j++ )
        {
            s = b[i*bstep + j];
            for( k = 0; k < i; k++ )
                s -= L[i*astep + k]*b[k*bstep + j];
            b[i*bstep + j] = (_Tp)(s*L[i*astep + i]);
        }
    }
    for( i = m-1; i >= 0; i-- )
    {
        for( j = 0; j < n; j++ )
        {
            s = b[i*bstep + j];
            for( k = m-1; k > i; k-- )
                s -= L[k*astep + i]*b[k*bstep + j];
            b[i*bstep + j] = (_Tp)(s*L[i*astep + i]);
        }
    }
    return true;
}

bool vsCholesky(float* A, size_t astep, int m, float* b, size_t bstep, int n)
{
    return vsCholImpl(A, astep, m, b, bstep, n);
}

bool vsCholesky(double* A, size_t astep, int m, double* b, size_t bstep, int n)
{
    return vsCholImpl(A, astep, m, b, bstep, n);
}

double vsInvert( VSMatrix *_src, VSMatrix *_dst, int method )
{
    bool result = false;
    VSMatrix *src = _src;
    int type = (*src).type;

    if(type != VS_32F && type != VS_64F)
        return result;

    size_t esz = 0;//CV_ELEM_SIZE(type);
    if(type == VS_32F){ esz = sizeof(float);}
    else{ esz = sizeof(double);}

    int m = (*src).rows, n = (*src).cols;

    if( method == VS_DECOMP_SVD )
    {
        //add later
    }
    if( m != n )
        return result;

    if( method == VS_DECOMP_EIG )
    {
        //add later
    }
    //assert( method == VS_DECOMP_LU || method == VS_DECOMP_CHOLESKY );
    VSMatrix *dst = _dst;

    if( n <= 3 )
    {
        uchar* srcdata = (uchar*)(*src).data;
        uchar* dstdata = (uchar*)(*dst).data;
        size_t srcstep = (*src).step;
        size_t dststep = (*dst).step;

        if( n == 2 )
        {
            if( type == VS_32F )
            {
                double d = det2(Sf);
                if( d != 0. )
                {
                    result = true;
                    d = 1./d;
                    {
                        double t0, t1;
                        t0 = Sf(0,0)*d;
                        t1 = Sf(1,1)*d;
                        Df(1,1) = (float)t0;
                        Df(0,0) = (float)t1;
                        t0 = -Sf(0,1)*d;
                        t1 = -Sf(1,0)*d;
                        Df(0,1) = (float)t0;
                        Df(1,0) = (float)t1;
                    }
                }
            }
            else
            {
                double d = det2(Sd);
                if( d != 0. )
                {
                    result = true;
                    d = 1./d;
                    {
                        double t0, t1;
                        t0 = Sd(0,0)*d;
                        t1 = Sd(1,1)*d;
                        Dd(1,1) = t0;
                        Dd(0,0) = t1;
                        t0 = -Sd(0,1)*d;
                        t1 = -Sd(1,0)*d;
                        Dd(0,1) = t0;
                        Dd(1,0) = t1;
                    }
                }
            }
        }
        else if( n == 3 )
        {
            if( type == VS_32F )
            {
                double d = det3(Sf);

                if( d != 0. )
                {
                    double t[12];

                    result = true;
                    d = 1./d;
                    t[0] = (((double)Sf(1,1) * Sf(2,2) - (double)Sf(1,2) * Sf(2,1)) * d);
                    t[1] = (((double)Sf(0,2) * Sf(2,1) - (double)Sf(0,1) * Sf(2,2)) * d);
                    t[2] = (((double)Sf(0,1) * Sf(1,2) - (double)Sf(0,2) * Sf(1,1)) * d);

                    t[3] = (((double)Sf(1,2) * Sf(2,0) - (double)Sf(1,0) * Sf(2,2)) * d);
                    t[4] = (((double)Sf(0,0) * Sf(2,2) - (double)Sf(0,2) * Sf(2,0)) * d);
                    t[5] = (((double)Sf(0,2) * Sf(1,0) - (double)Sf(0,0) * Sf(1,2)) * d);

                    t[6] = (((double)Sf(1,0) * Sf(2,1) - (double)Sf(1,1) * Sf(2,0)) * d);
                    t[7] = (((double)Sf(0,1) * Sf(2,0) - (double)Sf(0,0) * Sf(2,1)) * d);
                    t[8] = (((double)Sf(0,0) * Sf(1,1) - (double)Sf(0,1) * Sf(1,0)) * d);

                    Df(0,0) = (float)t[0]; Df(0,1) = (float)t[1]; Df(0,2) = (float)t[2];
                    Df(1,0) = (float)t[3]; Df(1,1) = (float)t[4]; Df(1,2) = (float)t[5];
                    Df(2,0) = (float)t[6]; Df(2,1) = (float)t[7]; Df(2,2) = (float)t[8];
                }
            }
            else
            {
                double d = det3(Sd);
                if( d != 0. )
                {
                    result = true;
                    d = 1./d;
                    double t[9];

                    t[0] = (Sd(1,1) * Sd(2,2) - Sd(1,2) * Sd(2,1)) * d;
                    t[1] = (Sd(0,2) * Sd(2,1) - Sd(0,1) * Sd(2,2)) * d;
                    t[2] = (Sd(0,1) * Sd(1,2) - Sd(0,2) * Sd(1,1)) * d;

                    t[3] = (Sd(1,2) * Sd(2,0) - Sd(1,0) * Sd(2,2)) * d;
                    t[4] = (Sd(0,0) * Sd(2,2) - Sd(0,2) * Sd(2,0)) * d;
                    t[5] = (Sd(0,2) * Sd(1,0) - Sd(0,0) * Sd(1,2)) * d;

                    t[6] = (Sd(1,0) * Sd(2,1) - Sd(1,1) * Sd(2,0)) * d;
                    t[7] = (Sd(0,1) * Sd(2,0) - Sd(0,0) * Sd(2,1)) * d;
                    t[8] = (Sd(0,0) * Sd(1,1) - Sd(0,1) * Sd(1,0)) * d;

                    Dd(0,0) = t[0]; Dd(0,1) = t[1]; Dd(0,2) = t[2];
                    Dd(1,0) = t[3]; Dd(1,1) = t[4]; Dd(1,2) = t[5];
                    Dd(2,0) = t[6]; Dd(2,1) = t[7]; Dd(2,2) = t[8];
                }
            }
        }
        else
        {
            if( n == 1 )
            {
                if( type == VS_32F )
                {
                    double d = Sf(0,0);
                    if( d != 0. )
                    {
                        result = true;
                        Df(0,0) = (float)(1./d);
                    }
                }else{
                    double d = Sd(0,0);
                    if( d != 0. )
                    {
                        result = true;
                        Dd(0,0) = 1./d;
                    }
                }
            }
        }
        if( !result )
        {
            (*dst).cols = 0;
            (*dst).rows = 0;
            (*dst).data = NULL;
            for(size_t i=0; i<n*n*esz; i++)
            {
                ((uchar*)((*dst).data))[i] = 0;
            }
        }
        return result;
    }

   int elem_size = esz;
   uchar* buf = (uchar*)malloc(n*n*elem_size);
  
   //copy data
   for(int i=0; i<n; i++)
   {
       for(int j=0; j<n; j++)
       {
           if ((*src).type == VS_32F)
           {
               float* src1 = (float*)buf;
               src1[i*n+j] = ((float*)((*src).data))[i*n+j];
           }else
           {
               double* src1 = (double*)buf;
               src1[i*n+j] = ((float*)((*src).data))[i*n+j];
           }
       }
   }
   //setIdentity(dst);
    if( method == VS_DECOMP_LU && type == VS_32F )
        result = vsLU((float*)buf, n*sizeof(float), n, (float*)(*dst).data, n*sizeof(float), n) != 0;
    else if( method == VS_DECOMP_LU && type == VS_64F )
        result = vsLU((double*)buf, n*sizeof(double), n, (double*)(*dst).data, n*sizeof(double), n) != 0;
    else if( method == VS_DECOMP_CHOLESKY && type == VS_32F )
        result = vsCholesky((float*)buf, n*sizeof(float), n, (float*)(*dst).data, n*sizeof(float), n);
    else {
        result = vsCholesky((double*)buf, n*sizeof(double), n, (double*)(*dst).data, n*sizeof(double), n);
    }

    free(buf); buf = NULL;

    if( !result )
    {
        //dst = Scalar(0);
        for(int i=0; i<n*n*elem_size; i++)
        {
            ((uchar*)((*dst).data))[i] = 0;
        }
    }

    return result;
}
