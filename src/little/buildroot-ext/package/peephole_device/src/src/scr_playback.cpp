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
#include "my_app.h"

static lv_obj_t *rec_lst_obj;
static void playback_video()
{
    //printf("=======%s\n",__FUNCTION__);
#if !USE_SAMPLE_SYS_INIT
    MyApp::GetInstance()->EnterPlaybackMode(true);
#endif
    jump_to_scr_playback_list();
}

static void playback_pic()
{
    printf("%s\n", __FUNCTION__);
#if !USE_SAMPLE_SYS_INIT
    MyApp::GetInstance()->EnterPlaybackMode(true);
#endif
    jump_to_scr_picture_list();
    // jump_to_scr_main();
}

static void playback_event_handler(lv_event_t *e)
{
    printf("%s\n", __FUNCTION__);
    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t *obj = lv_event_get_target(e);
    uint32_t id = lv_btnmatrix_get_selected_btn(obj);
    if (id == 0) // video
    {
        printf("%s>vidoe selected\n", __FUNCTION__);
        playback_video();
    }
    else if (id == 1) // picture
    {
        printf("%s>picture selected\n", __FUNCTION__);
        playback_pic();
    }
    else if (id == 2) // return
    {
        printf("%s>return selected\n", __FUNCTION__);
        jump_to_scr_main();
    }
}

static const char *btnm_map[] = {"video", "\n",
                                 "picture", "\n",
                                 "return", ""};

static void setup_scr_scr_playback(void)
{
    printf("%s\n", __FUNCTION__);

    if (lv_ui.scr_playback == NULL)
    {
        lv_obj_t *obj;
        obj = lv_obj_create(NULL);
        lv_ui.scr_playback = obj;
        lv_obj_remove_style_all(obj);
        lv_obj_set_style_bg_opa(obj, LV_OPA_TRANSP, LV_PART_MAIN);
        lv_obj_clear_flag(obj, LV_OBJ_FLAG_CLICKABLE);
        lv_obj_clear_flag(obj, LV_OBJ_FLAG_SCROLLABLE);

        lv_obj_t *bm = lv_btnmatrix_create(lv_ui.scr_playback);
        lv_btnmatrix_set_map(bm, btnm_map);
        lv_obj_align(bm, LV_ALIGN_CENTER, 0, 0);
        lv_obj_set_size(bm, lv_pct(50), lv_pct(30));
        lv_obj_set_style_text_font(bm, &lv_font_montserrat_40, LV_PART_MAIN);
        lv_obj_add_event_cb(bm, playback_event_handler, LV_EVENT_CLICKED, NULL);
    }
}

void jump_to_scr_playback(void)
{
    printf("%s\n", __FUNCTION__);
    setup_scr_scr_playback();
    lv_scr_load_anim(lv_ui.scr_playback, LV_SCR_LOAD_ANIM_NONE, 0, 0, false);
}
