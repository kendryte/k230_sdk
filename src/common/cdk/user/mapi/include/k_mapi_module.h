/**
 * @file k_mapi_module.h
 * @author  ()
 * @brief
 * @version 1.0
 * @date 2023-06-12
 *
* @copyright
 * Copyright (c) 2023, Canaan Bright Sight Co., Ltd
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
 *
 */
#ifndef __K_MAPI_MODULE_H__
#define __K_MAPI_MODULE_H__

#ifdef __cplusplus
extern "C"{
#endif /* End of #ifdef __cplusplus */

#define K_MAPI_MODE_NAME_LEN (8)

typedef enum
{
    K_MAPI_MOD_SYS = 0,

    K_MAPI_MOD_VI,
    K_MAPI_MOD_VPROC,
    K_MAPI_MOD_VENC,
    K_MAPI_MOD_VDEC,
    K_MAPI_MOD_VREC,
    K_MAPI_MOD_VO,

    K_MAPI_MOD_AI,
    K_MAPI_MOD_AENC,
    K_MAPI_MOD_ADEC,
    K_MAPI_MOD_AREC,
    K_MAPI_MOD_AO,

    K_MAPI_MOD_VVI,
    K_MAPI_MOD_VVO,

    K_MAPI_MOD_DPU,
    K_MAPI_MOD_VICAP,
    K_MAPI_MOD_SENSOR,
    K_MAPI_MOD_ISP,

    K_MAPI_MOD_BUTT,
} k_mapi_mod_id_e;

#ifdef __cplusplus
}
#endif /* End of #ifdef __cplusplus */
#endif

