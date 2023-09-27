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
#include "lv_record_files.h"
extern "C" void kd_mapi_media_init_workaround(k_bool media_init_flag);

static lv_obj_t * time_obj;
static lv_obj_t * slider_obj;
static lv_obj_t * play_obj;
static lv_obj_t * title_obj;
static lv_obj_t * time_start_obj;
static lv_obj_t * time_end_obj;
static lv_style_t style_video_title;
static lv_timer_t *  play_timer;

static lv_obj_t * g_ctrl_box = NULL;
#define MAX_SLIDER_RANGE   1000
static K_PLAYER_PROGRESS_INFO g_play_progress_info;
static bool g_play = false;
static sem_t g_stop_player_sem;

static void play_event_click_cb(lv_event_t * e)
{
    lv_obj_t * obj = lv_event_get_target(e);
    if(lv_obj_has_state(obj, LV_STATE_CHECKED))
    {
        kd_player_resume();
    }
    else
    {
        kd_player_pause();
    }
}

static void next_click_event_cb(lv_event_t * e)
{
    lv_event_code_t code = lv_event_get_code(e);
    if(code == LV_EVENT_CLICKED)
    {
        printf("lv demo video next\n");
        //_lv_demo_music_album_next(true);
    }
}

static void prev_click_event_cb(lv_event_t * e)
{
    lv_event_code_t code = lv_event_get_code(e);
    if(code == LV_EVENT_CLICKED)
    {
        printf("lv demo video prev\n");
    }
}

static void _show_cur_play_time()
{
    if (g_play_progress_info.cur_time <= g_play_progress_info.total_time)
    {
        char cur_time[256] = {0};
        int nsec = g_play_progress_info.cur_time/1000;
        sprintf(cur_time,"%02d:%02d",nsec%3600/60,nsec%60);
        lv_label_set_text(time_start_obj, cur_time);
    }
}

static void _show_total_play_time()
{
    char total_time[256] = {0};
    int nsec = g_play_progress_info.total_time/1000;
    sprintf(total_time,"%02d:%02d",nsec%3600/60,nsec%60);
    lv_label_set_text(time_end_obj, total_time);
}

static void play_timer_cb(lv_timer_t * t)
{
    if (g_play_progress_info.total_time != 0)
    {
        lv_slider_set_value(slider_obj, g_play_progress_info.cur_time*MAX_SLIDER_RANGE/g_play_progress_info.total_time, LV_ANIM_ON);
        _show_cur_play_time();
        _show_total_play_time();
    }
}

static void _lv_video_stop()
{
    if (g_play)
    {
        g_play = false;
        kd_player_stop();
        sem_wait(&g_stop_player_sem);

#if !USE_SAMPLE_SYS_INIT
	    kd_mapi_media_init_workaround(K_TRUE);
#endif
        kd_player_deinit(K_FALSE);
        sem_destroy(&g_stop_player_sem);
    }
}

static void _lv_video_next(bool next) // play next or previous
{
}

static lv_obj_t *g_back_label;
static lv_obj_t *g_back_btn;
static void go_back_click_event_cb(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    if (code == LV_EVENT_CLICKED)
    {
        _lv_video_stop();
        lv_slider_set_value(slider_obj, 0, LV_ANIM_OFF);
        jump_to_scr_playback_list();

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


static lv_obj_t * create_ctrl_box(lv_obj_t * parent)
{
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
    #if 0
    icon = lv_img_create(cont);
    lv_img_set_src(icon, &img_lv_btn_loop);
    lv_obj_set_grid_cell(icon, LV_GRID_ALIGN_END, 5, 1, LV_GRID_ALIGN_CENTER, 0, 1);
    #endif

    icon = lv_img_create(cont);
    lv_img_set_src(icon, &img_lv_btn_prev);
    lv_obj_set_grid_cell(icon, LV_GRID_ALIGN_CENTER, 2, 1, LV_GRID_ALIGN_CENTER, 0, 1);
    lv_obj_add_event_cb(icon, prev_click_event_cb, LV_EVENT_CLICKED, NULL);
    lv_obj_add_flag(icon, LV_OBJ_FLAG_CLICKABLE);

    play_obj = lv_imgbtn_create(cont);
    lv_imgbtn_set_src(play_obj, LV_IMGBTN_STATE_RELEASED, NULL, &img_lv_btn_play, NULL);
    lv_imgbtn_set_src(play_obj, LV_IMGBTN_STATE_CHECKED_RELEASED, NULL, &img_lv_btn_pause, NULL);
    lv_obj_add_flag(play_obj, LV_OBJ_FLAG_CHECKABLE);
    lv_obj_set_grid_cell(play_obj, LV_GRID_ALIGN_CENTER, 3, 1, LV_GRID_ALIGN_CENTER, 0, 1);

    lv_obj_add_event_cb(play_obj, play_event_click_cb, LV_EVENT_CLICKED, NULL);
    lv_obj_add_flag(play_obj, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_set_width(play_obj, img_lv_btn_play.header.w);

    icon = lv_img_create(cont);
    lv_img_set_src(icon, &img_lv_btn_next);
    lv_obj_set_grid_cell(icon, LV_GRID_ALIGN_CENTER, 4, 1, LV_GRID_ALIGN_CENTER, 0, 1);
    lv_obj_add_event_cb(icon, next_click_event_cb, LV_EVENT_CLICKED, NULL);
    lv_obj_add_flag(icon, LV_OBJ_FLAG_CLICKABLE);

    LV_IMG_DECLARE(img_lv_slider_knob);
    slider_obj = lv_slider_create(cont);
    lv_obj_set_style_anim_time(slider_obj, 100, 0);
    lv_obj_add_flag(slider_obj, LV_OBJ_FLAG_CLICKABLE);

    lv_obj_set_height(slider_obj, 6);
    lv_obj_set_grid_cell(slider_obj, LV_GRID_ALIGN_STRETCH, 1, 5, LV_GRID_ALIGN_CENTER, 1, 1);
    lv_obj_set_style_bg_img_src(slider_obj, &img_lv_slider_knob, LV_PART_KNOB);
    lv_obj_set_style_bg_opa(slider_obj, LV_OPA_TRANSP, LV_PART_KNOB);
    lv_obj_set_style_pad_all(slider_obj, 20, LV_PART_KNOB);
    lv_obj_set_style_bg_grad_dir(slider_obj, LV_GRAD_DIR_HOR, LV_PART_INDICATOR);
    lv_obj_set_style_bg_color(slider_obj, lv_color_hex(0x569af8), LV_PART_INDICATOR);
    lv_obj_set_style_bg_grad_color(slider_obj, lv_color_hex(0xa666f1), LV_PART_INDICATOR);
    lv_obj_set_style_outline_width(slider_obj, 0, 0);
    lv_slider_set_range(slider_obj,0,MAX_SLIDER_RANGE);

    time_start_obj = lv_label_create(cont);
    lv_obj_set_style_text_font(time_start_obj, &lv_font_montserrat_22, 0);
    lv_obj_set_style_text_color(time_start_obj, lv_color_hex(0x8a86b8), 0);
    lv_label_set_text(time_start_obj, "00:00");
    lv_obj_set_grid_cell(time_start_obj, LV_GRID_ALIGN_CENTER, 0, 1, LV_GRID_ALIGN_CENTER, 1, 1);

    time_end_obj = lv_label_create(cont);
    lv_obj_set_style_text_font(time_end_obj, &lv_font_montserrat_22, 0);
    lv_obj_set_style_text_color(time_end_obj, lv_color_hex(0x8a86b8), 0);
    lv_label_set_text(time_end_obj, "00:00");
    lv_obj_set_grid_cell(time_end_obj, LV_GRID_ALIGN_END, 6, 1, LV_GRID_ALIGN_CENTER, 1, 1);

    title_obj = lv_label_create(parent);
    lv_obj_set_style_text_color(title_obj, lv_color_hex(0x8a86b8), 0);
    lv_style_init(&style_video_title);
    lv_style_set_text_font(&style_video_title, &lv_font_montserrat_40);
    lv_obj_add_style(title_obj,&style_video_title,0);
    //lv_label_set_text(title_obj, "/clip/video/1.mp4");
    lv_obj_set_align(title_obj,LV_ALIGN_BOTTOM_LEFT);
#if 0
    lv_obj_t * handle_label = lv_label_create(parent);
    //lv_label_set_text(handle_label, LV_SYMBOL_VIDEO"ALL VIDEOS");
    lv_label_set_text(handle_label, LV_SYMBOL_HOME"BACK");
    lv_obj_set_style_text_font(handle_label, &lv_font_montserrat_22, 0);
    lv_obj_set_style_text_color(handle_label, lv_color_hex(0x8a86b8), 0);
    lv_obj_set_align(handle_label,LV_ALIGN_BOTTOM_RIGHT);
    lv_obj_add_flag(handle_label, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_add_event_cb(handle_label, go_videos_click_event_cb, LV_EVENT_CLICKED, NULL);
#endif

    memset(&g_play_progress_info,0,sizeof(g_play_progress_info));
    play_timer = lv_timer_create(play_timer_cb, 100, NULL);
    g_ctrl_box = cont;
    return cont;
}

static k_s32 player_event_cb(K_PLAYER_EVENT_E enEvent, void *pData)
{
    if (enEvent == K_PLAYER_EVENT_EOF)
    {
        sem_post(&g_stop_player_sem);
        if (g_play)
        {
            _lv_video_stop();
            jump_to_scr_playback_list();
        }
    }
    else if (enEvent == K_PLAYER_EVENT_PROGRESS)
    {
        g_play_progress_info = *(K_PLAYER_PROGRESS_INFO*)pData;
    }

    return 0;
}

static void _lv_video_play(uint32_t id)
{
    if (!g_play)
    {
        lv_slider_set_value(slider_obj, 0, LV_ANIM_OFF);
        lv_obj_add_state(play_obj, LV_STATE_CHECKED);

        const char *filename = _lv_record_files_get_filepathname(id);
        lv_label_set_text(title_obj, filename);
        //printf("file pathname:%s\n", filename);
        sem_init(&g_stop_player_sem, 0, 0);
#if !USE_SAMPLE_SYS_INIT
	    kd_mapi_media_init_workaround(K_FALSE);
#endif
        kd_player_init(K_FALSE);
        kd_player_regcallback(player_event_cb, NULL);
        kd_player_setdatasource(filename);
        kd_player_start();

        g_play = true;
    }
}

static void setup_scr_scr_playback_ctrl(uint32_t track_id)
{
    if (lv_ui.scr_playback_ctrl == NULL)
    {
        lv_obj_t *obj;
        obj = lv_obj_create(NULL);
        lv_ui.scr_playback_ctrl = obj;
        lv_obj_remove_style_all(obj);
        lv_obj_set_style_bg_opa(obj, LV_OPA_TRANSP, LV_PART_MAIN);
        lv_obj_clear_flag(obj, LV_OBJ_FLAG_CLICKABLE);
        lv_obj_clear_flag(obj, LV_OBJ_FLAG_SCROLLABLE);

        create_ctrl_box(lv_ui.scr_playback_ctrl);
        _lv_back_ctrl_create(lv_ui.scr_playback_ctrl);
    }

    _lv_video_play(track_id);
}

void jump_to_scr_playback_ctrl(uint32_t track_id)
{
    setup_scr_scr_playback_ctrl(track_id);
    lv_scr_load_anim(lv_ui.scr_playback_ctrl, LV_SCR_LOAD_ANIM_NONE, 0, 0, false);
}
