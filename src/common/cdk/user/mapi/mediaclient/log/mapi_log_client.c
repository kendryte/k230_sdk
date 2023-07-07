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
#include <stdio.h>
#include <stdarg.h>
#include "mapi_log.h"

static K_ERR_LEVEL_E g_enable_level = K_ERR_LEVEL_ERROR;

static k_char g_module_names[K_MAPI_MOD_BUTT][K_MAPI_MODE_NAME_LEN] =
{
    "SYS_S",
    "VI_S",
    "VPROC_S",
    "VENC_S",
    "VDEC_S",
    "VREC_S",
    "VO_S",
    "AI_S",
    "AENC_S",
    "ADEC_S",
    "AREC_S",
    "AO_S",
    "VVI_S",
    "VVO_S",
};

static inline k_char *mapi_get_mod_name_by_id(k_mapi_mod_id_e mod)
{
    return g_module_names[mod];
}

k_s32 kd_mapi_set_log_level(K_ERR_LEVEL_E level)
{
    g_enable_level = level;
    return K_SUCCESS;
}

k_s32 kendyrte_mapi_log_printf(k_mapi_mod_id_e mod, K_ERR_LEVEL_E level, const char *fmt, ...)
{
    char *name = mapi_get_mod_name_by_id(mod);

    if(level < g_enable_level) {
        return K_SUCCESS;
    }

    va_list args;
    printf("[%s] ", name);
    va_start(args, fmt);
    vprintf(fmt, args);
    va_end(args);

    return 0;
}