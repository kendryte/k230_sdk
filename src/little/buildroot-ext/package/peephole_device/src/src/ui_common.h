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

#ifndef __UI_COMMON_H__
#define __UI_COMMON_H__
#ifdef __cplusplus
extern "C" {
#endif

#include "lvgl.h"
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "k_ipcmsg.h"

#define USE_SAMPLE_SYS_INIT   0 //big core use sample_sys_init.elf
#define ARRAY_SIZE(a) (sizeof(a) / sizeof(a[0]))
#define ISP_WIDTH   (1080)
#define ISP_HEIGHT  (1920)

typedef enum
{
    SCR_MAIN,
    SCR_PLAYBACK,
} scr_status_e;

typedef struct
{
    lv_obj_t *obj;
    lv_point_t pos;
    lv_align_t align;
    char *img_path;
} user_img_obj_t;

typedef struct
{
    scr_status_e scr_status;
    char *img_root_path;
    lv_obj_t *scr_main;
    user_img_obj_t scr_main_btn_intercom;
    user_img_obj_t scr_main_btn_voice_change;
    user_img_obj_t scr_main_btn_save;
    user_img_obj_t scr_main_btn_playback;
    user_img_obj_t scr_main_btn_shutdown;
    lv_obj_t *scr_main_msg_url;
    lv_obj_t *scr_playback;
    lv_obj_t *scr_playback_list;
    lv_obj_t *scr_picture_list;
    lv_obj_t *scr_playback_ctrl;
    lv_obj_t *scr_picture_ctrl;
    lv_obj_t *scr_playback_video;
    user_img_obj_t scr_playback_video_btn_play;
    user_img_obj_t scr_playback_video_btn_stop;
    user_img_obj_t scr_playback_video_btn_return;

} lv_ui_t;

extern lv_ui_t lv_ui;

lv_timer_t *create_oneshot_timer(lv_timer_cb_t timer_xcb, uint32_t period,
                                 void *user_data);
lv_obj_t *create_msgbox(const char *title, const char *txt);
void setup_scr_scr_main(char *url);
void jump_to_scr_main(void);
void jump_to_scr_playback(void);
void scr_main_display_result(int8_t result);
void jump_to_scr_playback_ctrl(uint32_t track_id);
void jump_to_scr_playback_picture_ctrl(uint32_t track_id);
void jump_to_scr_playback_list(void);
void jump_to_scr_picture_list(void);
#ifdef __cplusplus
}
#endif
#endif /* __UI_COMMON_H__ */
