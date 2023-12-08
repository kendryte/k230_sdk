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
#include <pthread.h>
#include <unistd.h>
#include <signal.h>
#include <termio.h>
#include <atomic>
#include "lv_port.h"
#include "k_type.h"
#include "mapi_sys_api.h"

#include <iostream>
#include "my_app.h"

static std::atomic<bool> g_exit_flag{false};

static int set_priority(void)
{
    int max_prio = sched_get_priority_max(SCHED_RR);
    int min_prio = sched_get_priority_min(SCHED_RR);
    struct sched_param sp = {(max_prio + min_prio) / 4};
    return pthread_setschedparam(pthread_self(), SCHED_RR, &sp);
}

static void sigHandler(int sig_no)
{
    g_exit_flag.store(true);
}


static void Usage() {
    std::cout << "Usage: ./peehole_device [-h] [-r] [-s] [-m <mp4 dir>] [-j <jpeg dir>]"<< std::endl;
    std::cout << "-h: print usage" << std::endl;
    std::cout << "-r: remote features only, default false" << std::endl;
    std::cout << "-s: enable record, default false" << std::endl;
    std::cout << "-m: record directory for mp4, default /sharefs/app " << std::endl;
    std::cout << "-j: record directory for jpg, default /sharefs/app" << std::endl;
    exit(-1);
}

static bool remote_features_only = false;
static bool enable_record = false;
static std::string mp4_dir = "/sharefs/app";
static std::string jpeg_dir = "/sharefs/app";
static bool enable_debug_record = false;
static bool enable_pir_vo = false;
static bool show_perf_cycles = false;

int parse_config(int argc, char *argv[]) {
    int result;
    opterr = 0;
    while ((result = getopt(argc, argv, "hrsvm:j:dt")) != -1) {
        switch(result) {
        case 'r' : {
            remote_features_only = true;
            break;
        }
        case 's' : {
            enable_record = true;
            break;
        }
        case 'v' : {
            enable_pir_vo = true;
            break;
        }
        case 'm' : {
            mp4_dir = optarg;
            break;
        }
        case 'j' : {
            jpeg_dir = optarg;
            break;
        }
        case 'd' : {
            enable_debug_record = true;
            break;
        }
        case 't': {
            printf("perf_get_smodecycles %llu\n", perf_get_smodecycles());
            exit(0);
        }
        case 'h':
        default: Usage(); break;
        }
    }
    return 0;
}

// nonblock getchar
static int my_getchar() {
    system("stty -raw echo -F /dev/tty");
    fd_set rfds;
    struct timeval tv;
    tv.tv_sec = 0;
    tv.tv_usec = 1000 * 10;

    FD_ZERO(&rfds);
    FD_SET(0, &rfds);
    int ret;
    while ((ret = select(1, &rfds, NULL, NULL, &tv)) < 0 && errno == EINTR);
    if (ret > 0 ) {
        return getchar();
    }
    return -1;
}


#if USE_SAMPLE_SYS_INIT
#include "vo_cfg.h"
static int test_main(int argc, char *argv[])
{
    printf("******%s\n",__FUNCTION__);
    int ret = 0;
    ret = kd_mapi_sys_init();
    if (ret != K_SUCCESS)
    {
        std::cout << "kd_mapi_sys_init error." << std::endl;
        return ret;
    }

    // init vo
    vo_layer_init(VO_WIDTH, VO_HEIGHT);
    lv_init();
    lv_port_disp_init();
    lv_port_indev_init();

    setup_scr_scr_main(argv[1]);
    set_priority();

    jump_to_scr_main();
    while (!g_exit_flag)
    {
        lv_timer_handler();
        usleep(40 * 1000);
    }
}
#endif

int main(int argc, char *argv[])
{
#if USE_SAMPLE_SYS_INIT
    parse_config(argc, argv);
    // set mp4&jpeg record-path
    MyApp::GetInstance()->SetRecordDir(mp4_dir, jpeg_dir);
    test_main(argc,argv);
    return 0;
#endif
    k_s32 ret;
    // printf("./peephole_device -h to show usage\n");
    parse_config(argc, argv);

    signal(SIGINT, sigHandler);
    signal(SIGPIPE, SIG_IGN);

    // set mp4&jpeg record-path
    MyApp::GetInstance()->SetRecordDir(mp4_dir, jpeg_dir);
    MyApp::GetInstance()->EnableRecord(enable_record);

    printf("MyApp::GetInstance()->Init() called\n");
    if (MyApp::GetInstance()->Init() < 0) {
        return -1;
    }
    printf("MyApp::GetInstance()->Init() done\n");
    bool gui_init = false;
    if (remote_features_only) {
        // for test, there are no local features (no GUI)
        while(!g_exit_flag) {
            if (MyApp::GetInstance()->RpcPowerOff()) {
                break;
            }
            if (enable_debug_record) {
                int c = my_getchar();
                if(c == 's') {
                    MyApp::GetInstance()->EnableRecord(true);
                    printf("start record ...\n");
                } else if(c == 'p') {
                    MyApp::GetInstance()->EnableRecord(false);
                    printf("record stopping, wait until \"mp4 file generated\" shown\n");
                } else if (c == 'q') {
                    break;
                }
            }
            usleep(40 * 1000);
        }
    } else {
        if (!enable_pir_vo) {
            lv_init();
            lv_port_disp_init();
            lv_port_indev_init();

            setup_scr_scr_main(argv[1]);
            set_priority();

            jump_to_scr_main();
            printf("perf_ after jump_to_scr_main:  %llu\n", perf_get_smodecycles());

            std::thread([](){
                show_ipaddr();
            }).detach();
            while (!g_exit_flag) {
                lv_timer_handler();
                usleep(40 * 1000);

                if (MyApp::GetInstance()->RpcPowerOff()) {
                    break;
                }
                if (enable_debug_record) {
                    int c = my_getchar();
                    if(c == 's') {
                        MyApp::GetInstance()->EnableRecord(true);
                        printf("start record ...\n");
                    } else if(c == 'p') {
                        MyApp::GetInstance()->EnableRecord(false);
                        printf("record stopping, wait until \"mp4 file generated\" shown\n");
                    } else if (c == 'q') {
                        break;
                    }
                }
            }
        } else {
            while (!g_exit_flag) {
                if (MyApp::GetInstance()->RpcPowerOff()) {
                    break;
                }
                int cur_mode = MyApp::GetInstance()->GetCurrentMode();

                if (cur_mode == DOORBELL_MODE) {
                    if (!gui_init) {
                        lv_init();
                        lv_port_disp_init();
                        lv_port_indev_init();

                        setup_scr_scr_main(argv[1]);
                        set_priority();
                        jump_to_scr_main();
                        std::thread([](){
                            show_ipaddr();
                        }).detach();
                       
                        gui_init = true;
                    }
                    lv_timer_handler();
                }

                if (enable_debug_record) {
                    int c = my_getchar();
                    if(c == 's') {
                        MyApp::GetInstance()->EnableRecord(true);
                        printf("start record ...\n");
                    } else if(c == 'p') {
                        MyApp::GetInstance()->EnableRecord(false);
                        printf("record stopping, wait until \"mp4 file generated\" shown\n");
                    } else if (c == 'q') {
                        break;
                    }
                }
                usleep(40 * 1000);
            }
        }
    }

    MyApp::GetInstance()->DeInit();
    // power off, TODO
    std::cout << "Peephole Deivce : Power Off" << std::endl;
    system("poweroff");
    return 0;
}
