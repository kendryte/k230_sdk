#include <atomic>
#include <vector>
#include "streaming_player.h"

using namespace std::chrono_literals;

std::atomic<bool> g_exit_flag{false};

static void sigHandler(int sig_no) {
    g_exit_flag.store(true);
    printf("exit_flag true\n");
}

static void help() {
    std::cout << "Usage: ./rtsp_demo -s 7 -n 2 -t h265 -w 1280 -h 720 -a 0." << std::endl;
    std::cout << "-s: the sensor type: default 7" << std::endl;
    std::cout << "       see camera sensor doc." << std::endl;
    std::cout << "-n: the session number, range: [1, 3]." << std::endl;
    std::cout << "-t: the video encoder type: h264/h265/mjpeg." << std::endl;
    std::cout << "-w: the video encoder width." << std::endl;
    std::cout << "-h: the video encoder height." << std::endl;
    std::cout << "-a: audio input type(0:mic input  1:headphone input):default 0." << std::endl;

}

int main(int argc, char *argv[]) {
    signal(SIGINT, sigHandler);
    signal(SIGPIPE, SIG_IGN);
    g_exit_flag.store(false);

    int ret;

    VideoType video_type = kVideoTypeH265;
    k_vicap_sensor_type sensor_type = IMX335_MIPI_2LANE_RAW12_1920X1080_30FPS_LINEAR;
    int session_num = 1;
    int video_width = 1280;
    int video_height = 720;
    k_i2s_in_mono_channel audio_input_type = KD_I2S_IN_MONO_RIGHT_CHANNEL;//mic input

    if (argc > 1) {
        for (int i = 1; i < argc; i += 2) {
            if (strcmp(argv[i], "-n") == 0) {
                session_num = atoi(argv[i + 1]);
                if (session_num > 3) {
                    std::cout << "chn nummber out of range: [1, 3]." << std::endl;
                    return -1;
                }
            } else if (strcmp(argv[i], "-t") == 0) {
                if (strcmp(argv[i+1], "h264") == 0) {
                    video_type = kVideoTypeH264;
                } else if (strcmp(argv[i+1], "h265") == 0) {
                    video_type = kVideoTypeH265;
                } else if (strcmp(argv[i+1], "mjpeg") == 0) {
                    video_type = kVideoTypeMjpeg;
                } else {
                    std::cout << "the video type is invalid, please check..." << std::endl;
                    help();
                    return -1;
                }
            } else if (strcmp(argv[i], "-w") == 0) {
                video_width = atoi(argv[i+1]);
            } else if (strcmp(argv[i], "-h") == 0) {
                video_height = atoi(argv[i+1]);
            } else if (strcmp(argv[i], "-s") == 0) {
                sensor_type = (k_vicap_sensor_type)atoi(argv[i+1]);
            } else if (strcmp(argv[i], "-a") == 0) {
                audio_input_type = (k_i2s_in_mono_channel)atoi(argv[i+1]);
            }
            else if (strcmp(argv[i], "--help") == 0) {
                help();
                return 0;
            }
        }
    }

    StreamingPlayer *sample_player = new StreamingPlayer(sensor_type, video_width, video_height, session_num);
    if (!sample_player) {
        std::cout << "StreamingPlayer Init failed." << std::endl;
        return -1;
    }

    for (int i = 0; i < session_num; i++) {
        std::string session_name = "session";
        SessionAttr session_attr;
        memset(&session_attr, 0, sizeof(session_attr));
        session_attr.session_idx = i;
        session_attr.video_type = video_type;
        session_attr.video_width = video_width;
        session_attr.video_height = video_height;
        session_attr.session_name = session_name.append(std::to_string(i));
        session_attr.auido_mono_channel_type = audio_input_type;

        ret = sample_player->CreateSession(session_attr);
        if (ret < 0) {
            std::cout << "CreateSession failed." << std::endl;
            goto end;
        }
    }

    sample_player->Start();

    while (!g_exit_flag) {
        std::this_thread::sleep_for(100ms);
    }

    sample_player->Stop();

end:
    for (int i = 0; i < session_num; i++)
        sample_player->DestroySession(i);

    sample_player->DeInit();

    delete sample_player;

    return 0;
}