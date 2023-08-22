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

#include "ui_common.h"

static void scr_signup_close(lv_event_t *e)
{
    lv_scr_load_anim(lv_ui.scr_main, LV_SCR_LOAD_ANIM_NONE, 0, 0, true);
}

static void scr_signup_continue(lv_event_t *e)
{
    lv_obj_t *ta = lv_event_get_user_data(e);
    char *name = lv_textarea_get_text(ta);

    if (strlen(name))
    {
        msg_send_cmd_with_data(MSG_CMD_SIGNUP, name, strlen(name) + 1);
    }
    lv_scr_load_anim(lv_ui.scr_main, LV_SCR_LOAD_ANIM_NONE, 0, 0, true);
}

static void setup_scr_scr_signup(void)
{
    lv_obj_t *obj;

    obj = lv_obj_create(NULL);
    lv_ui.scr_signup = obj;
    lv_obj_remove_style_all(obj);
    lv_obj_set_style_bg_opa(obj, LV_OPA_TRANSP, LV_PART_MAIN);
    lv_obj_clear_flag(obj, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_clear_flag(obj, LV_OBJ_FLAG_SCROLLABLE);

    lv_obj_t *ta = lv_textarea_create(lv_ui.scr_signup);
    lv_obj_set_size(ta, lv_pct(100), LV_SIZE_CONTENT);
    lv_textarea_set_text(ta, "");
    lv_textarea_set_one_line(ta, true);
    lv_textarea_set_max_length(ta, 32);
    lv_textarea_set_align(ta, LV_TEXT_ALIGN_CENTER);
    lv_obj_set_style_text_font(ta, &lv_font_montserrat_48, LV_PART_MAIN);

    lv_obj_t *kb = lv_keyboard_create(lv_ui.scr_signup);
    lv_obj_set_style_text_font(kb, &lv_font_montserrat_40, LV_PART_MAIN);
    lv_obj_set_size(kb, lv_pct(100), lv_pct(30));
    lv_keyboard_set_mode(kb, LV_KEYBOARD_MODE_TEXT_LOWER);
    lv_keyboard_set_textarea(kb, ta);
    lv_obj_add_event_cb(kb, scr_signup_continue, LV_EVENT_READY, ta);
    lv_obj_add_event_cb(kb, scr_signup_close, LV_EVENT_CANCEL, NULL);
    lv_obj_align_to(ta, kb, LV_ALIGN_OUT_TOP_MID, 0, -10);
}

void jump_to_scr_signup(void)
{
    setup_scr_scr_signup();
    lv_scr_load_anim(lv_ui.scr_signup, LV_SCR_LOAD_ANIM_NONE, 0, 0, false);
}
