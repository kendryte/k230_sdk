/**
 * @file k_type.h
 * @author
 * @brief
 * @version 1.0
 * @date 2022-08-31
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
 */
#ifndef __K_TYPE_H__
#define __K_TYPE_H__

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/** \addtogroup     SYSTEM_CTRL */
/** @{ */ /** <!-- [SYSTEM_CTRL] */

typedef unsigned char           k_u8;
typedef unsigned short          k_u16;
typedef unsigned int            k_u32;

typedef signed char             k_s8;
typedef short                   k_s16;
typedef int                     k_s32;

typedef unsigned long           k_u64;
typedef signed long             k_s64;

typedef char                    k_char;

typedef enum
{
    K_FALSE = 0,
    K_TRUE  = 1,
} k_bool;

typedef k_u32 k_handle;

#define K_SUCCESS               (0)         /**< no error*/
#define K_FAILED                (-1)        /**< A generic error happens*/

/** @} */ /** <!-- ==== SYSTEM_CTRL End ==== */

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __K_TYPE_H__ */
