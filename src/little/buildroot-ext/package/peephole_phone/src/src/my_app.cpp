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

#include <iostream>
#include <pthread.h>
#include "my_app.h"
#include "ui_common.h"
#include "lvgl.h"
#include "lv_port.h"
#include "my_app_impl.h"

/*
static int set_priority(void)
{
    int max_prio = sched_get_priority_max(SCHED_RR);
    int min_prio = sched_get_priority_min(SCHED_RR);
    struct sched_param sp = {(max_prio + min_prio) / 4};

    return pthread_setschedparam(pthread_self(), SCHED_RR, &sp);
}
*/
int MyApp::Init() {
    if (intercom_init() < 0) {
        printf("intercom_init failed\n");
        return -1 ;
    }

    lv_init();
    lv_port_disp_init();
    lv_port_indev_init();

    setup_scr_scr_main();
    // set_priority();

    jump_to_scr_main();
    return 0;
}

void MyApp::DeInit() {
    intercom_deinit();
    // lv_deinit(); 
}

void MyApp::Run(volatile int &exit_flag, const char *dev_ip, unsigned short port) {
    if (dev_ip && port) {
        dev_ip_ = dev_ip;
        dev_port_ = port;
    }

    rpc_comm_created_.store(false);
    std::thread thread = std::thread([this, &exit_flag](){
        while(!exit_flag) {
            if (!rpc_comm_created_) {
                if (rpc_client_.Init(this, dev_ip_, dev_port_) < 0) {
                    if (dev_ip_.empty()) usleep(1000 * 100);
                    continue;
                }
                std::cout << "peephole device connected, url : " << url_ << std::endl;
                set_url(url_.c_str());
                
                create_msgbox("Status", "Connected");
                               
                rpc_comm_created_ = true;
            }
            usleep(1000 * 1000);
        }
    });

    while (!exit_flag) {
        lv_timer_handler();
        usleep(1000 * 20);
    }

    if(thread.joinable()) {
        thread.join();
    }

    rpc_client_.DeInit();
}

void MyApp::OnServerInfo(const ServerInfo &info)
{
    url_ = "rtsp://";
    url_ += dev_ip_;
    url_ += ":";
    url_ += std::to_string(info.speech_service_port);
    url_ += "/";
    url_ += info.speech_stream_name;

    // for debug
    // url_ = "rtsp://192.168.5.1:8554/test.264";
}

void MyApp::OnEvent(const UserEventData &event) {
    std::cout << "MyApp::OnEvent() called" << std::endl;
    
    switch(event.type) {
    case UserEventData::EventType::PIR_WAKEUP: create_msgbox("Event", "PIR_WAKEUP"); break;
    case UserEventData::EventType::KEY_WAKEUP: create_msgbox("Event", "KEY_WAKEUP"); break;
    case UserEventData::EventType::STAY_ALARM: create_msgbox("Event", "STAY_ALARM"); break;
    default: create_msgbox("Event", "unknown"); break;
    }
    
}

int intercom_init()
{
    if (PeepholeApp::GetInstance()->Init() < 0) {
        std::cout << "KdRtspClient Init failed." << std::endl;
        return -1;
    }
    return 0;
}

int intercom_start(const char *url)
{
    printf("%s>url %s\n", __FUNCTION__, url);
    return PeepholeApp::GetInstance()->Start(url);
}

int intercom_deinit()
{
    PeepholeApp::GetInstance()->DeInit();
    return 0;
}

int intercom_suspend()
{
    return PeepholeApp::GetInstance()->Suspend();
}

int intercom_resume(const char *url)
{
    return PeepholeApp::GetInstance()->Resume();
}

int intercom_enable_pitch_shift()
{
    return PeepholeApp::GetInstance()->EnablePitchShift();    
}

int intercom_disable_pitch_shift()
{
    return PeepholeApp::GetInstance()->DisablePitchShift();    
}
