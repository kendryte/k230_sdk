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

#ifndef __MSG_PROC_H__
#define __MSG_PROC_H__

#ifdef __cplusplus
extern "C" {
#endif

/*********************
 *      INCLUDES
 *********************/
#include <stdint.h>

typedef enum {
    MSG_CMD_SIGNUP,
    MSG_CMD_SIGNUP_RESULT,
    MSG_CMD_IMPORT,
    MSG_CMD_IMPORT_RESULT,
    MSG_CMD_DELETE,
    MSG_CMD_DELETE_RESULT,
    MSG_CMD_FEATURE_SAVE,
} msg_cmd_e;

typedef enum {
    UI_CMD_SIGNUP_RESULT,
    UI_CMD_IMPORT_RESULT,
    UI_CMD_DELETE_RESULT,
} ui_cmd_e;

typedef struct {
    uint8_t cmd;
    int8_t result;
    uint8_t reserve[6];
    uint8_t data[0];
} ui_msg_t;

int msg_proc_init(void);
int ui_msg_proc(void);
int msg_send_cmd(uint32_t cmd);
int msg_send_cmd_with_data(uint32_t cmd, void *payload, uint32_t payload_len);

#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif /*__MSG_PROC_H__*/
