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
 
#include <string>
#include <mutex>
#include "ui_common.h"
#include "my_app.h"

static intercom_status_t g_intercom_status = INTERCOM_STATUS_IDLE;
voice_status_t g_voice_status = VOICE_STATUS_ORIGIN;

lv_ui_t lv_ui;

static std::mutex url_mutex_;
static std::string  s_url_;

void set_url(const char *url) {
    std::unique_lock<std::mutex> lck(url_mutex_);
    s_url_ = url;
}

void scr_main_all_btn_enable(bool enable)
{
    lv_obj_t *objs[] =
    {
        lv_ui.scr_main_btn_intercom.obj,
        lv_ui.scr_main_btn_voice_change.obj,
        lv_ui.scr_main_btn_save.obj,
        lv_ui.scr_main_btn_shutdown.obj,
    };

    for (int i = 0; i < ARRAY_SIZE(objs); i++)
    {
        if (enable)
            lv_obj_clear_state(objs[i], LV_STATE_DISABLED);
        else
            lv_obj_add_state(objs[i], LV_STATE_DISABLED);
    }
}

static void scr_main_btn_intercom_event_handler(lv_event_t *e)
{
    printf("%s>g_intercom_status %d\n", __FUNCTION__, g_intercom_status);
    printf("%s>user_data %s\n", __FUNCTION__, lv_event_get_user_data(e));

    std::unique_lock<std::mutex> lck(url_mutex_);        
    if (s_url_.empty()) {
        printf("url not set yet, please check it\n");
        return;
    }

    if (g_intercom_status == INTERCOM_STATUS_IDLE)
    {
        intercom_start(s_url_.c_str());
        g_intercom_status = INTERCOM_STATUS_INITED;
    }
    else if ((g_intercom_status == INTERCOM_STATUS_INITED) || (g_intercom_status == INTERCOM_STATUS_RUNNING))
    {
        intercom_suspend();
        g_intercom_status = INTERCOM_STATUS_SUSPEND;
    }
    else if (g_intercom_status == INTERCOM_STATUS_SUSPEND)
    {
        intercom_resume(s_url_.c_str());
        g_intercom_status = INTERCOM_STATUS_RUNNING;
    }
    else
    {
        printf("%s>unknown status\n", __FUNCTION__);
    }

    return;
}

static void scr_main_btn_voice_change_event_handler(lv_event_t *e)
{
    printf("%s\n", __FUNCTION__);

    if (g_voice_status == VOICE_STATUS_ORIGIN)
    {
        if(!intercom_enable_pitch_shift())
            g_voice_status = VOICE_STATUS_CHANGED;
    }
    else if (g_voice_status == VOICE_STATUS_CHANGED)
    {
        if (!intercom_disable_pitch_shift())
         g_voice_status = VOICE_STATUS_ORIGIN;
    }
    return;
}

static void scr_main_btn_save_event_handler(lv_event_t *e)
{
    printf("%s\n", __FUNCTION__);
    // intercom_save_control();
}

static void scr_main_btn_shutdown_event_handler(lv_event_t *e)
{
    printf("%s\n", __FUNCTION__);
    MyApp::GetInstance()->DevPowerOff();
}

static int obj_property_init(void)
{
    lv_ui.img_root_path = DATA_FILE_PATH"img/";

    printf("obj_property_init --- img_root_path : %s\n", lv_ui.img_root_path);

    lv_ui.scr_main_btn_intercom.pos.x = 20;
    lv_ui.scr_main_btn_intercom.pos.y = lv_pct(-5);
    lv_ui.scr_main_btn_intercom.align = LV_ALIGN_BOTTOM_LEFT;
    lv_ui.scr_main_btn_intercom.img_path = (char*)"intercom.png";

    lv_ui.scr_main_btn_voice_change.pos.x = lv_pct(33);
    lv_ui.scr_main_btn_voice_change.pos.y = lv_pct(-5);
    lv_ui.scr_main_btn_voice_change.align = LV_ALIGN_BOTTOM_LEFT;
    lv_ui.scr_main_btn_voice_change.img_path = (char*)"voice_change.png";
/*
    lv_ui.scr_main_btn_save.pos.x = lv_pct(50);
    lv_ui.scr_main_btn_save.pos.y = lv_pct(-5);
    lv_ui.scr_main_btn_save.align = LV_ALIGN_BOTTOM_LEFT;
    lv_ui.scr_main_btn_save.img_path = (char*)"record.png";
*/
    lv_ui.scr_main_btn_shutdown.pos.x = lv_pct(66);
    lv_ui.scr_main_btn_shutdown.pos.y = lv_pct(-5);
    lv_ui.scr_main_btn_shutdown.align = LV_ALIGN_BOTTOM_LEFT;
    lv_ui.scr_main_btn_shutdown.img_path = (char*)"shutdown.png";

    return 0;
}

void setup_scr_scr_main(void)
{
    lv_obj_t *obj;
    char file_path[128];
    g_intercom_status = INTERCOM_STATUS_IDLE;

    obj_property_init();

    obj = lv_obj_create(NULL);
    lv_ui.scr_main = obj;
    lv_obj_remove_style_all(obj);
    lv_obj_set_style_bg_opa(obj, LV_OPA_TRANSP, LV_PART_MAIN);
    lv_obj_clear_flag(obj, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_clear_flag(obj, LV_OBJ_FLAG_SCROLLABLE);

    obj = lv_img_create(lv_ui.scr_main);
    lv_ui.scr_main_btn_intercom.obj = obj;
    lv_obj_set_pos(obj, lv_ui.scr_main_btn_intercom.pos.x,
                   lv_ui.scr_main_btn_intercom.pos.y);
    lv_obj_set_align(obj, lv_ui.scr_main_btn_intercom.align);
    lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
    snprintf(file_path, sizeof(file_path), "%s%s", lv_ui.img_root_path,
             lv_ui.scr_main_btn_intercom.img_path);
    lv_img_set_src(obj, file_path);
    lv_obj_add_flag(obj, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_add_event_cb(obj, scr_main_btn_intercom_event_handler, LV_EVENT_CLICKED,
                        NULL);

    obj = lv_img_create(lv_ui.scr_main);
    lv_ui.scr_main_btn_voice_change.obj = obj;
    lv_obj_set_pos(obj, lv_ui.scr_main_btn_voice_change.pos.x,
                   lv_ui.scr_main_btn_voice_change.pos.y);
    lv_obj_set_align(obj, lv_ui.scr_main_btn_voice_change.align);
    lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
    snprintf(file_path, sizeof(file_path), "%s%s", lv_ui.img_root_path,
             lv_ui.scr_main_btn_voice_change.img_path);
    lv_img_set_src(obj, file_path);
    lv_obj_add_flag(obj, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_add_event_cb(obj, scr_main_btn_voice_change_event_handler, LV_EVENT_CLICKED,
                        NULL);
/*
    obj = lv_img_create(lv_ui.scr_main);
    lv_ui.scr_main_btn_save.obj = obj;
    lv_obj_set_pos(obj, lv_ui.scr_main_btn_save.pos.x,
                   lv_ui.scr_main_btn_save.pos.y);
    lv_obj_set_align(obj, lv_ui.scr_main_btn_save.align);
    lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
    snprintf(file_path, sizeof(file_path), "%s%s", lv_ui.img_root_path,
             lv_ui.scr_main_btn_save.img_path);
    lv_img_set_src(obj, file_path);
    lv_obj_add_flag(obj, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_add_event_cb(obj, scr_main_btn_save_event_handler, LV_EVENT_CLICKED,
                        NULL);
*/
    obj = lv_img_create(lv_ui.scr_main);
    lv_ui.scr_main_btn_shutdown.obj = obj;
    lv_obj_set_pos(obj, lv_ui.scr_main_btn_shutdown.pos.x,
                   lv_ui.scr_main_btn_shutdown.pos.y);
    lv_obj_set_align(obj, lv_ui.scr_main_btn_shutdown.align);
    lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
    snprintf(file_path, sizeof(file_path), "%s%s", lv_ui.img_root_path,
             lv_ui.scr_main_btn_shutdown.img_path);
    lv_img_set_src(obj, file_path);
    lv_obj_add_flag(obj, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_add_event_cb(obj, scr_main_btn_shutdown_event_handler, LV_EVENT_CLICKED,
                        NULL);
}

void jump_to_scr_main(void)
{
    lv_ui.scr_status = SCR_MAIN;
    lv_scr_load(lv_ui.scr_main);
}

lv_timer_t *create_oneshot_timer(lv_timer_cb_t timer_xcb, uint32_t period,
                                 void *user_data)
{
    lv_timer_t *new_timer;

    new_timer = lv_timer_create(timer_xcb, period, user_data);
    if (new_timer == NULL)
        return NULL;
    lv_timer_set_repeat_count(new_timer, 1);

    return new_timer;
}

static void msgbox_close_event_handler(lv_event_t *e)
{
    lv_obj_t **obj = (lv_obj_t **)lv_event_get_user_data(e);

    if (*obj == NULL)
        return;

    lv_timer_del((lv_timer_t *)lv_obj_get_user_data(*obj));
    lv_msgbox_close(*obj);
    *obj = NULL;
}

static void msgbox_timeout_timer_handler(lv_timer_t *timer)
{
    lv_obj_t **obj = (lv_obj_t **)timer->user_data;

    if (*obj == NULL)
        return;

    lv_msgbox_close(*obj);
    *obj = NULL;
}

lv_obj_t *create_msgbox(const char *title, const char *txt)
{
    static lv_obj_t *msgbox = NULL;
    static bool isfirst = true;
    static lv_style_t style;

    if (isfirst)
    {
        isfirst = false;
        lv_style_init(&style);
        lv_style_set_text_font(&style, &lv_font_montserrat_48);
        lv_style_set_text_align(&style, LV_TEXT_ALIGN_CENTER);
    }

    if (msgbox)
        return msgbox;

    msgbox = lv_msgbox_create(NULL, title, txt, NULL, false);
    lv_label_set_recolor(lv_msgbox_get_text(msgbox), true);
    lv_obj_add_style(lv_msgbox_get_text(msgbox), &style, LV_PART_MAIN);
    lv_obj_add_event_cb(lv_obj_get_parent(msgbox), msgbox_close_event_handler,
                        LV_EVENT_CLICKED, (void *)&msgbox);
    lv_obj_center(msgbox);

    lv_timer_t *timer =
        lv_timer_create(msgbox_timeout_timer_handler,
                        1 * 1000, (void *)&msgbox);
    lv_timer_set_repeat_count(timer, 1);
    lv_obj_set_user_data(msgbox, timer);

    return msgbox;
}

void scr_main_display_result(int8_t result)
{
    const char *str = result == 0 ? "#00ff00 ok#" : "#ff0000 fail#";
    create_msgbox("", str);
}