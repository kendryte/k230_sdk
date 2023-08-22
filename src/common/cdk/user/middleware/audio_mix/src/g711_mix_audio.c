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

#include "g711_mix_audio.h"

#include <stdio.h>
#include <string.h>
#include "g711codec.h"

#define MAX_SRC_DATA_LEN 48000 * 2 * 4
#define AUDIO_DATA_VALUE_MAX 32767 // 2^15（short）
#define AUDIO_DATA_VALUE_MIN -32768
static k_char g_dst_data1[MAX_SRC_DATA_LEN];
static k_char g_dst_data2[MAX_SRC_DATA_LEN];
static k_char g_dst_mix_data[MAX_SRC_DATA_LEN];

#define ENABLE_MIX_AUDIO_DEBUG 1

#ifdef ENABLE_MIX_AUDIO_DEBUG
#define mix_audio_debug printf
#else
#define mix_audio_debug(ARGS...)
#endif

static k_s32 _mix_g711_data(k_char *src_data1, k_char *src_data2, k_u32 data_len, k_char *dst_mix_data, unsigned char type)
{
	//decode:data_len->data_len*2
	if (-1 == decode(src_data1, g_dst_data1, data_len, type))
	{
		mix_audio_debug("%s decode data1 failed\n", __FUNCTION__);
		return -1;
	}

	if (-1 == decode(src_data2, g_dst_data2, data_len, type))
	{
		mix_audio_debug("%s decode data2 failed\n", __FUNCTION__);
		return -1;
	}

	//mix data
	short *pSrc1 = (short *)g_dst_data1;
	short *pSrc2 = (short *)g_dst_data2;
	short *pDst = (short *)g_dst_mix_data;
	int nSum = 0;
	int nMixLen = data_len; // 16bit
	for (int i = 0; i < nMixLen; i++)
	{
		nSum = 0;
		nSum += pSrc1[i];
		nSum += pSrc2[i];
		// overflow
		if (nSum > AUDIO_DATA_VALUE_MAX)
			nSum = AUDIO_DATA_VALUE_MAX;
		else if (nSum < AUDIO_DATA_VALUE_MIN)
			nSum = AUDIO_DATA_VALUE_MIN;

		pDst[i] = (short)nSum;
	}

	//encode
	if (-1 == encode(g_dst_mix_data,dst_mix_data,data_len*2,type))
	{
		mix_audio_debug("%s encode data failed\n", __FUNCTION__);
		return -1;
	}

	return 0;
}

k_s32 kd_mix_g711a_audio(k_char *src_data1, k_char *src_data2, k_u32 data_len, k_char *dst_mix_data)
{
	if (data_len > MAX_SRC_DATA_LEN)
	{
		mix_audio_debug("%s data_len(%d) not valid\n", __FUNCTION__, data_len);
		return -1;
	}

	if (src_data1 == NULL && src_data2 == NULL)
	{
		return -1;
	}

	if (src_data1 != NULL && src_data2 == NULL)
	{
		memcpy(dst_mix_data, src_data1, data_len);
	}
	else if (src_data1 == NULL && src_data2 != NULL)
	{
		memcpy(dst_mix_data, src_data2, data_len);
	}
	else if (src_data1 != NULL && src_data2 != NULL)
	{
		return _mix_g711_data(src_data1,src_data2,data_len,dst_mix_data,G711_A_LAW);
	}

	return 0;
}

k_s32 kd_mix_g711u_audio(k_char *src_data1, k_char *src_data2, k_u32 data_len, k_char *dst_mix_data)
{
	if (data_len > MAX_SRC_DATA_LEN)
	{
		mix_audio_debug("%s data_len(%d) not valid\n", __FUNCTION__, data_len);
		return -1;
	}

	if (src_data1 == NULL && src_data2 == NULL)
	{
		return -1;
	}

	if (src_data1 != NULL && src_data2 == NULL)
	{
		memcpy(dst_mix_data, src_data1, data_len);
	}
	else if (src_data1 == NULL && src_data2 != NULL)
	{
		memcpy(dst_mix_data, src_data2, data_len);
	}
	else if (src_data1 != NULL && src_data2 != NULL)
	{
		return _mix_g711_data(src_data1,src_data2,data_len,dst_mix_data,G711_MU_LAW);
	}

	return 0;
}