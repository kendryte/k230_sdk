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

#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/ioctl.h>
#include <netinet/in.h>
#include <linux/if.h>
#include "ui_common.h"
#include "intercom.h"
#include "audio_talk.h"
#include "my_app.h"

static intercom_status_t g_intercom_status = INTERCOM_STATUS_IDLE;
voice_status_t g_voice_status;

lv_ui_t lv_ui;

static int get_ipaddr(const char *interface, char *ipaddr){
    int sock_fd;
    if ((sock_fd = socket(AF_INET, SOCK_STREAM, 0)) == -1){
        printf("get_ipaddr() -- socket create failed...!\n");
        return -1;
    }

    struct sockaddr_in *sin;
    struct ifreq ifr_ip;

    memset(&ifr_ip, 0, sizeof(ifr_ip));
    strncpy(ifr_ip.ifr_name, interface, sizeof(ifr_ip.ifr_name) - 1);

    if(ioctl( sock_fd, SIOCGIFADDR, &ifr_ip) < 0 ){
        printf("get_ipaddr() -- SIOCGIFADDR ioctl error\n");
        close(sock_fd);
        return -1;
    }
    sin = (struct sockaddr_in *)&ifr_ip.ifr_addr;
    strcpy(ipaddr,inet_ntoa(sin->sin_addr));
    close(sock_fd);
    return 0;
}

void scr_main_all_btn_enable(bool enable)
{
    lv_obj_t *objs[] =
    {
        lv_ui.scr_main_btn_intercom.obj,
        lv_ui.scr_main_btn_voice_change.obj,
        lv_ui.scr_main_btn_playback.obj,
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
    printf("%s\n", __FUNCTION__);
    char file_path[128];

    if (g_intercom_status == INTERCOM_STATUS_IDLE)
    {
        lv_ui.scr_main_btn_intercom.img_path = "intercom_gray.png";
        g_intercom_status = INTERCOM_STATUS_RUNNING;
    #if !USE_SAMPLE_SYS_INIT
        MyApp::GetInstance()->EnterPlaybackMode(true);
    #endif
        audio_talk(K_TRUE);
    }
    else if ( g_intercom_status == INTERCOM_STATUS_RUNNING)
    {
        lv_ui.scr_main_btn_intercom.img_path = "intercom.png";
        g_intercom_status = INTERCOM_STATUS_IDLE;
    #if !USE_SAMPLE_SYS_INIT
        MyApp::GetInstance()->EnterPlaybackMode(true);
    #endif
        audio_talk(K_FALSE);
    }
    else
    {
        printf("%s>unknown status\n", __FUNCTION__);
    }
    snprintf(file_path, sizeof(file_path), "%s%s", lv_ui.img_root_path,
             lv_ui.scr_main_btn_intercom.img_path);
    lv_img_set_src(lv_ui.scr_main_btn_intercom.obj, file_path);
    return;
}

static void scr_main_btn_voice_change_event_handler(lv_event_t *e)
{
    if (g_intercom_status != INTERCOM_STATUS_RUNNING)
    {
        printf("please start audio talk first\n");
        return ;
    }

    printf("%s\n", __FUNCTION__);
    char file_path[128];

    if (g_voice_status == VOICE_STATUS_ORIGIN)
    {
        lv_ui.scr_main_btn_voice_change.img_path = "voice_change_gray.png";
        g_voice_status = VOICE_STATUS_CHANGED;
        printf("audio_pitch_shift enable before\n");
        audio_pitch_shift(K_TRUE);
        printf("audio_pitch_shift enable end\n");
    }
    else if (g_voice_status == VOICE_STATUS_CHANGED)
    {
        lv_ui.scr_main_btn_voice_change.img_path = "voice_change.png";
        g_voice_status = VOICE_STATUS_ORIGIN;
        printf("audio_pitch_shift disable before\n");
        audio_pitch_shift(K_FALSE);
        printf("audio_pitch_shift disable end\n");
    }
    snprintf(file_path, sizeof(file_path), "%s%s", lv_ui.img_root_path,
             lv_ui.scr_main_btn_voice_change.img_path);
    lv_img_set_src(lv_ui.scr_main_btn_voice_change.obj, file_path);
    return;
}

static void scr_main_btn_playback_event_handler(lv_event_t *e)
{
    printf("%s\n", __FUNCTION__);
    jump_to_scr_playback();
}

static void scr_main_btn_shutdown_event_handler(lv_event_t *e)
{
    printf("%s\n", __FUNCTION__);
    MyApp::GetInstance()->SetPowerOff();
}

static int obj_property_init(void)
{
    lv_ui.img_root_path = DATA_FILE_PATH"img/";

    lv_ui.scr_main_btn_intercom.pos.x = 20;
    lv_ui.scr_main_btn_intercom.pos.y = lv_pct(-5);
    lv_ui.scr_main_btn_intercom.align = LV_ALIGN_BOTTOM_LEFT;
    lv_ui.scr_main_btn_intercom.img_path = "intercom.png";

    lv_ui.scr_main_btn_voice_change.pos.x = lv_pct(25);
    lv_ui.scr_main_btn_voice_change.pos.y = lv_pct(-5);
    lv_ui.scr_main_btn_voice_change.align = LV_ALIGN_BOTTOM_LEFT;
    lv_ui.scr_main_btn_voice_change.img_path = "voice_change.png";

    lv_ui.scr_main_btn_playback.pos.x = lv_pct(50);
    lv_ui.scr_main_btn_playback.pos.y = lv_pct(-5);
    lv_ui.scr_main_btn_playback.align = LV_ALIGN_BOTTOM_LEFT;
    lv_ui.scr_main_btn_playback.img_path = "play.png";

    lv_ui.scr_main_btn_shutdown.pos.x = lv_pct(75);
    lv_ui.scr_main_btn_shutdown.pos.y = lv_pct(-5);
    lv_ui.scr_main_btn_shutdown.align = LV_ALIGN_BOTTOM_LEFT;
    lv_ui.scr_main_btn_shutdown.img_path = "shutdown.png";

    return 0;
}

void setup_scr_scr_main(char *url)
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
                        url);

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

    obj = lv_img_create(lv_ui.scr_main);
    lv_ui.scr_main_btn_playback.obj = obj;
    lv_obj_set_pos(obj, lv_ui.scr_main_btn_playback.pos.x,
                   lv_ui.scr_main_btn_playback.pos.y);
    lv_obj_set_align(obj, lv_ui.scr_main_btn_playback.align);
    lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
    snprintf(file_path, sizeof(file_path), "%s%s", lv_ui.img_root_path,
             lv_ui.scr_main_btn_playback.img_path);
    lv_img_set_src(obj, file_path);
    lv_obj_add_flag(obj, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_add_event_cb(obj, scr_main_btn_playback_event_handler, LV_EVENT_CLICKED,
                        NULL);

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

    std::string net_interface{"eth0"};
    char local_ipaddr[20] = {0};
    get_ipaddr(net_interface.c_str(), (char*)local_ipaddr);
    obj = lv_msgbox_create(lv_ui.scr_main, NULL, local_ipaddr, NULL, false);
    lv_ui.scr_main_msg_url = obj;
    lv_obj_align_to(obj, lv_ui.scr_main_btn_intercom.obj, LV_ALIGN_OUT_TOP_LEFT, 0, -20);
    lv_obj_set_style_text_font(obj, &lv_font_montserrat_32, LV_PART_MAIN);
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
    lv_obj_t **obj = (lv_obj_t**)lv_event_get_user_data(e);

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
