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
#include <string.h>
#include <stdlib.h>
#include <fstream>
using namespace std;
#include "lv_record_files.h"
#include "my_app.h"

LV_IMG_DECLARE(img_lv_btn_list_pause);
LV_IMG_DECLARE(img_lv_btn_list_play);

static lv_obj_t *list;
static const lv_font_t *font_small;
static const lv_font_t *font_medium;
static lv_style_t style_scrollbar;
static lv_style_t style_btn;
static lv_style_t style_btn_pr;
static lv_style_t style_btn_chk;
static lv_style_t style_btn_dis;
static lv_style_t style_title;
static lv_style_t style_artist;
static lv_style_t style_time;

#define LV_DEMO_MUSIC_HANDLE_SIZE 40
static void _lv_video_list_btn_check(uint32_t track_id, bool state)
{
    lv_obj_t *btn = lv_obj_get_child(list, track_id);
    lv_obj_t *icon = lv_obj_get_child(btn, 0);

    if (state)
    {
        lv_obj_add_state(btn, LV_STATE_CHECKED);
        // lv_img_set_src(icon, &img_lv_btn_list_pause);
        lv_img_set_src(icon, &img_lv_btn_list_play);
        lv_obj_scroll_to_view(btn, LV_ANIM_ON);
    }
    else
    {
        lv_obj_clear_state(btn, LV_STATE_CHECKED);
        lv_img_set_src(icon, &img_lv_btn_list_play);
    }
}

static void btn_click_event_cb(lv_event_t *e)
{
    lv_obj_t *btn = lv_event_get_target(e);

    uint32_t idx = lv_obj_get_child_id(btn);
    _lv_video_list_btn_check(idx, true);
    jump_to_scr_playback_ctrl(idx);
    _lv_video_list_btn_check(idx, false);
}

static lv_obj_t *_add_list_btn(lv_obj_t *parent, uint32_t track_id)
{
    float file_size = _lv_record_files_get_size(track_id);
    const char *title = _lv_record_files_get_name(track_id);
    const char *artist = _lv_record_files_get_path(track_id);

    lv_obj_t *btn = lv_obj_create(parent);
    lv_obj_remove_style_all(btn);
    lv_obj_set_size(btn, lv_pct(100), 110);

    lv_obj_add_style(btn, &style_btn, 0);
    lv_obj_add_style(btn, &style_btn_pr, LV_STATE_PRESSED);
    lv_obj_add_style(btn, &style_btn_chk, LV_STATE_CHECKED);
    lv_obj_add_style(btn, &style_btn_dis, LV_STATE_DISABLED);
    lv_obj_add_event_cb(btn, btn_click_event_cb, LV_EVENT_CLICKED, NULL);

#if 0
    if(track_id >= 3) {
        lv_obj_add_state(btn, LV_STATE_DISABLED);
    }
#endif

    lv_obj_t *icon = lv_img_create(btn);
    lv_img_set_src(icon, &img_lv_btn_list_play);
    lv_obj_set_grid_cell(icon, LV_GRID_ALIGN_START, 0, 1, LV_GRID_ALIGN_CENTER, 0, 2);

    lv_obj_t *title_label = lv_label_create(btn);
    lv_label_set_text(title_label, title);
    lv_obj_set_grid_cell(title_label, LV_GRID_ALIGN_START, 1, 1, LV_GRID_ALIGN_CENTER, 0, 1);
    lv_obj_add_style(title_label, &style_title, 0);

    lv_obj_t *artist_label = lv_label_create(btn);
    lv_label_set_text(artist_label, artist);
    lv_obj_add_style(artist_label, &style_artist, 0);
    lv_obj_set_grid_cell(artist_label, LV_GRID_ALIGN_START, 1, 1, LV_GRID_ALIGN_CENTER, 1, 1);

    lv_obj_t *time_label = lv_label_create(btn);
    char filesize_info[256];
    sprintf(filesize_info, "%.2fM", file_size);
    lv_label_set_text(time_label, filesize_info);
    lv_obj_add_style(time_label, &style_time, 0);
    lv_obj_set_grid_cell(time_label, LV_GRID_ALIGN_END, 2, 1, LV_GRID_ALIGN_CENTER, 0, 2);

#if 0
    LV_IMG_DECLARE(img_lv_demo_music_list_border);
    lv_obj_t * border = lv_img_create(btn);
    lv_img_set_src(border, &img_lv_demo_music_list_border);
    lv_obj_set_width(border, lv_pct(120));
    lv_obj_align(border, LV_ALIGN_BOTTOM_MID, 0, 0);
    lv_obj_add_flag(border, LV_OBJ_FLAG_IGNORE_LAYOUT);
#endif

    return btn;
}

static lv_obj_t *g_back_label;
static lv_obj_t *g_back_btn;
static void go_back_click_event_cb(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    if (code == LV_EVENT_CLICKED)
    {
        jump_to_scr_main();
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

lv_obj_t *_lv_rec_video_list_create(lv_obj_t *parent)
{
    std::string mp4_dir;
    std::string jpeg_dir;
    MyApp::GetInstance()->GetRecordDir(mp4_dir,jpeg_dir);
    printf("****mp4 dir:%s\n",mp4_dir.c_str());

    if (0 != _lv_record_files_init((char*)mp4_dir.c_str()))
    {
        printf("_lv_record_files_init failed\n");
        return NULL;
    }

    font_small = &lv_font_montserrat_16;
    font_medium = &lv_font_montserrat_22;

    lv_style_init(&style_scrollbar);
    lv_style_set_width(&style_scrollbar, 4);
    lv_style_set_bg_opa(&style_scrollbar, LV_OPA_COVER);
    lv_style_set_bg_color(&style_scrollbar, lv_color_hex3(0xeee));
    lv_style_set_radius(&style_scrollbar, LV_RADIUS_CIRCLE);
    lv_style_set_pad_right(&style_scrollbar, 4);

    static const lv_coord_t grid_cols[] = {LV_GRID_CONTENT, LV_GRID_FR(1), LV_GRID_CONTENT, LV_GRID_TEMPLATE_LAST};
    static const lv_coord_t grid_rows[] = {35, 30, LV_GRID_TEMPLATE_LAST};

    lv_style_init(&style_btn);
    lv_style_set_bg_opa(&style_btn, LV_OPA_TRANSP);
    lv_style_set_grid_column_dsc_array(&style_btn, grid_cols);
    lv_style_set_grid_row_dsc_array(&style_btn, grid_rows);
    lv_style_set_grid_row_align(&style_btn, LV_GRID_ALIGN_CENTER);
    lv_style_set_layout(&style_btn, LV_LAYOUT_GRID);
    lv_style_set_pad_right(&style_btn, 30);

    lv_style_init(&style_btn_pr);
    lv_style_set_bg_opa(&style_btn_pr, LV_OPA_COVER);
    lv_style_set_bg_color(&style_btn_pr, lv_color_hex(0x4c4965));
    // lv_style_set_bg_color(&style_btn_pr,  lv_color_hex(0x0));

    lv_style_init(&style_btn_chk);
    lv_style_set_bg_opa(&style_btn_chk, LV_OPA_COVER);
    lv_style_set_bg_color(&style_btn_chk, lv_color_hex(0x4c4965));
    // lv_style_set_bg_color(&style_btn_chk, lv_color_hex(0x0));

    lv_style_init(&style_btn_dis);
    lv_style_set_text_opa(&style_btn_dis, LV_OPA_40);
    lv_style_set_img_opa(&style_btn_dis, LV_OPA_40);

    lv_style_init(&style_title);
    lv_style_set_text_font(&style_title, font_medium);
    lv_style_set_text_color(&style_title, lv_color_hex(0xffffff));
    // lv_style_set_text_color(&style_title, lv_color_hex(0x0));

    lv_style_init(&style_artist);
    lv_style_set_text_font(&style_artist, font_small);
    lv_style_set_text_color(&style_artist, lv_color_hex(0xb1b0be));

    lv_style_init(&style_time);
    lv_style_set_text_font(&style_time, font_medium);
    lv_style_set_text_color(&style_time, lv_color_hex(0xffffff));
    // lv_style_set_text_color(&style_time, lv_color_hex(0x0));

    /*Create an empty transparent container*/
    list = lv_obj_create(parent);
    lv_obj_remove_style_all(list);
    lv_obj_set_size(list, LV_HOR_RES, LV_VER_RES - LV_DEMO_MUSIC_HANDLE_SIZE);
    lv_obj_set_y(list, LV_DEMO_MUSIC_HANDLE_SIZE);
    lv_obj_add_style(list, &style_scrollbar, LV_PART_SCROLLBAR);
    lv_obj_set_flex_flow(list, LV_FLEX_FLOW_COLUMN);

    uint32_t track_id;
    for (track_id = 0; _lv_record_files_get_name(track_id); track_id++)
    {
        _add_list_btn(list, track_id);
    }
    //_lv_video_list_btn_check(0, true);
    return list;
}

static void setup_scr_scr_playback_list(void)
{
    if (lv_ui.scr_playback_list == NULL)
    {
        lv_obj_t *obj;
        obj = lv_obj_create(NULL);
        lv_ui.scr_playback_list = obj;
        lv_obj_remove_style_all(obj);
        lv_obj_set_style_bg_opa(obj, LV_OPA_TRANSP, LV_PART_MAIN);
        lv_obj_clear_flag(obj, LV_OBJ_FLAG_CLICKABLE);
        lv_obj_clear_flag(obj, LV_OBJ_FLAG_SCROLLABLE);

        _lv_rec_video_list_create(lv_ui.scr_playback_list);
        _lv_back_ctrl_create(lv_ui.scr_playback_list);
    }
}

void jump_to_scr_playback_list(void)
{
    setup_scr_scr_playback_list();
    lv_scr_load_anim(lv_ui.scr_playback_list, LV_SCR_LOAD_ANIM_NONE, 0, 0, false);
}
