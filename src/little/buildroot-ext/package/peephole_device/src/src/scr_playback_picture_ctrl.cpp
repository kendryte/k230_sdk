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
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <unistd.h>
#include <pthread.h>
#include <semaphore.h>
#include <signal.h>

#include "kplayer.h"
#include "lv_picture_files.h"

extern "C" void kd_mapi_media_init_workaround(k_bool media_init_flag);

static int g_cur_show_pic_track = -1;

static lv_obj_t * title_obj;
static lv_style_t style_video_title;
static void _picture_init()
{
#if !USE_SAMPLE_SYS_INIT
    kd_mapi_media_init_workaround(K_FALSE);
#endif
    kd_player_init(K_FALSE);
    kd_picture_init();
}

static void _picture_deinit()
{
#if !USE_SAMPLE_SYS_INIT
    kd_mapi_media_init_workaround(K_TRUE);
#endif
    kd_picture_deinit();
    kd_player_deinit(K_FALSE);
}

static lv_obj_t *g_back_label;
static lv_obj_t *g_back_btn;
static void go_back_click_event_cb(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    if (code == LV_EVENT_CLICKED)
    {
        _picture_deinit();
        jump_to_scr_picture_list();
    }
}

static void _lv_back_ctrl_create(lv_obj_t *parent)
{
    g_back_btn = lv_btn_create(parent);
    lv_obj_set_size(g_back_btn,200,80);
    lv_obj_set_align(g_back_btn, LV_ALIGN_BOTTOM_RIGHT);
    lv_obj_add_flag(g_back_btn, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_add_event_cb(g_back_btn, go_back_click_event_cb, LV_EVENT_CLICKED, NULL);
    g_back_label = lv_label_create(g_back_btn);
    lv_label_set_text(g_back_label, LV_SYMBOL_HOME "BACK");
    lv_obj_set_style_text_font(g_back_label, &lv_font_montserrat_40, 0);
    lv_obj_set_style_text_color(g_back_label, lv_color_hex(0xffffff), 0);
    lv_obj_set_align(g_back_label, LV_ALIGN_CENTER);
}

static void _lv_title_ctrl_create(lv_obj_t *parent)
{
    title_obj = lv_label_create(parent);
    lv_obj_set_style_text_color(title_obj, lv_color_hex(0x8a86b8), 0);
    lv_style_init(&style_video_title);
    lv_style_set_text_font(&style_video_title, &lv_font_montserrat_40);
    lv_obj_add_style(title_obj,&style_video_title,0);
    //lv_label_set_text(title_obj, "/clip/video/1.mp4");
    lv_obj_set_align(title_obj,LV_ALIGN_BOTTOM_LEFT);
}

static void _show_picture(uint32_t track_id)
{
    const char *pic_name = _lv_picture_files_get_filepathname(track_id);
    if (pic_name == NULL)
    {
        printf("track_id:%d not valid\n",track_id);
        return ;
    }
    lv_label_set_text(title_obj, pic_name);
    kd_picture_show(pic_name);
    g_cur_show_pic_track = track_id;

}

static void next_click_event_cb(lv_event_t * e)
{
    lv_event_code_t code = lv_event_get_code(e);
    if(code == LV_EVENT_CLICKED)
    {
        printf("lv demo video next\n");
        _show_picture(g_cur_show_pic_track+1);
        //_lv_demo_music_album_next(true);
    }
}

static void prev_click_event_cb(lv_event_t * e)
{
    lv_event_code_t code = lv_event_get_code(e);
    if(code == LV_EVENT_CLICKED)
    {
        printf("lv demo video prev\n");
        _show_picture(g_cur_show_pic_track-1);
    }
}

static lv_obj_t * create_ctrl_box(lv_obj_t * parent)
{
    printf("===========picture create_ctrl_box begin\n");
    /*Create the control box*/
    lv_obj_t * cont = lv_obj_create(parent);
    lv_obj_remove_style_all(cont);
   //lv_obj_set_height(cont, LV_SIZE_CONTENT);
   // lv_obj_set_style_pad_bottom(cont, 17, 0);
   lv_obj_set_size(cont, LV_HOR_RES, 180);
   lv_obj_set_y(cont, LV_VER_RES-260);
   //lv_obj_set_align(cont, LV_ALIGN_BOTTOM_MID);

    static const lv_coord_t grid_col[] = {LV_GRID_FR(2), LV_GRID_FR(3),LV_GRID_FR(5), LV_GRID_FR(5), LV_GRID_FR(5), LV_GRID_FR(3), LV_GRID_FR(2), LV_GRID_TEMPLATE_LAST};
    static const lv_coord_t grid_row[] = {LV_GRID_CONTENT, LV_GRID_CONTENT,LV_GRID_CONTENT, LV_GRID_TEMPLATE_LAST};
    lv_obj_set_grid_dsc_array(cont, grid_col, grid_row);

    LV_IMG_DECLARE(img_lv_btn_pause);
    LV_IMG_DECLARE(img_lv_btn_play);
    LV_IMG_DECLARE(img_lv_btn_next);
    LV_IMG_DECLARE(img_lv_btn_prev);
    LV_IMG_DECLARE(img_lv_btn_loop);

    lv_obj_t * icon;
    icon = lv_img_create(cont);
    lv_img_set_src(icon, &img_lv_btn_prev);
    lv_obj_set_grid_cell(icon, LV_GRID_ALIGN_CENTER, 2, 1, LV_GRID_ALIGN_CENTER, 0, 1);
    lv_obj_add_event_cb(icon, prev_click_event_cb, LV_EVENT_CLICKED, NULL);
    lv_obj_add_flag(icon, LV_OBJ_FLAG_CLICKABLE);

    icon = lv_img_create(cont);
    lv_img_set_src(icon, &img_lv_btn_next);
    lv_obj_set_grid_cell(icon, LV_GRID_ALIGN_CENTER, 4, 1, LV_GRID_ALIGN_CENTER, 0, 1);
    lv_obj_add_event_cb(icon, next_click_event_cb, LV_EVENT_CLICKED, NULL);
    lv_obj_add_flag(icon, LV_OBJ_FLAG_CLICKABLE);

    printf("===========picture create_ctrl_box end\n");
    return cont;

}

static void setup_scr_scr_playback_picture_ctrl(uint32_t track_id)
{
    if (lv_ui.scr_picture_ctrl == NULL)
    {
        lv_obj_t *obj;
        obj = lv_obj_create(NULL);
        lv_ui.scr_picture_ctrl = obj;
        lv_obj_remove_style_all(obj);
        lv_obj_set_style_bg_opa(obj, LV_OPA_TRANSP, LV_PART_MAIN);
        lv_obj_clear_flag(obj, LV_OBJ_FLAG_CLICKABLE);
        lv_obj_clear_flag(obj, LV_OBJ_FLAG_SCROLLABLE);

        //create_ctrl_box(lv_ui.scr_picture_ctrl);
        _lv_back_ctrl_create(lv_ui.scr_picture_ctrl);
        _lv_title_ctrl_create(lv_ui.scr_picture_ctrl);
    }
    g_cur_show_pic_track = -1;
    _picture_init();
    _show_picture(track_id);
}

void jump_to_scr_playback_picture_ctrl(uint32_t track_id)
{
    setup_scr_scr_playback_picture_ctrl(track_id);
    lv_scr_load_anim(lv_ui.scr_picture_ctrl, LV_SCR_LOAD_ANIM_NONE, 0, 0, false);
}
