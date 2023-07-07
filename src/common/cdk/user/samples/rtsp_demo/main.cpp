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
    std::cout << "Usage: ./rtsp_demo -s 0 -n 2 -t h265 -w 1280 -h 720." << std::endl;
    std::cout << "-s: the sensor type:" << std::endl;
    std::cout << "       0: ov9732." << std::endl;
    std::cout << "       1: ov9286 ir." << std::endl;
    std::cout << "       2: ov9286 speckle." << std::endl;
    std::cout << "       3: imx335 2LANE 1920Wx1080H." << std::endl;
    std::cout << "       4: imx335 2LANE 2592Wx1944H." << std::endl;
    std::cout << "       5: imx335 4LANE 2592Wx1944H." << std::endl;
    std::cout << "       6: imx335 2LANE MCLK 7425 1920Wx1080H." << std::endl;
    std::cout << "       7: imx335 2LANE MCLK 7425 2592Wx1944H." << std::endl;
    std::cout << "       8: imx335 4LANE MCLK 7425 2592Wx1944H." << std::endl;
    std::cout << "-n: the session number, range: [1, 3]." << std::endl;
    std::cout << "-t: the video encoder type: h264/h265/mjpeg." << std::endl;
    std::cout << "-w: the video encoder width." << std::endl;
    std::cout << "-h: the video encoder height." << std::endl;
}

static k_vicap_sensor_type get_sensor_type(k_u32 sensor_index)
{
    switch (sensor_index)
    {
    case 0:
        return OV_OV9732_MIPI_1280X720_30FPS_10BIT_LINEAR;
    case 1:
        return OV_OV9286_MIPI_1280X720_30FPS_10BIT_LINEAR_IR;
    case 2:
        return OV_OV9286_MIPI_1280X720_30FPS_10BIT_LINEAR_SPECKLE;
    case 3:
        return IMX335_MIPI_2LANE_RAW12_1920X1080_30FPS_LINEAR;
    case 4:
        return IMX335_MIPI_2LANE_RAW12_2592X1944_30FPS_LINEAR;
    case 5:
        return IMX335_MIPI_4LANE_RAW12_2592X1944_30FPS_LINEAR;
    case 6:
        return IMX335_MIPI_2LANE_RAW12_1920X1080_30FPS_MCLK_7425_LINEAR;
    case 7:
        return IMX335_MIPI_2LANE_RAW12_2592X1944_30FPS_MCLK_7425_LINEAR;
    case 8:
        return IMX335_MIPI_4LANE_RAW12_2592X1944_30FPS_MCLK_7425_LINEAR;
    default:
        printf("unsupport sensor type %d, use default IMX335_MIPI_2LANE_RAW12_1920X1080_30FPS_LINEAR\n", sensor_index);
        return IMX335_MIPI_2LANE_RAW12_1920X1080_30FPS_LINEAR;
    }
    return IMX335_MIPI_2LANE_RAW12_1920X1080_30FPS_LINEAR;
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
                sensor_type = get_sensor_type(atoi(argv[i+1]));
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

    std::string session_name = "session";

    for (int i = 0; i < session_num; i++) {
        SessionAttr session_attr;
        memset(&session_attr, 0, sizeof(session_attr));
        session_attr.session_idx = i;
        session_attr.video_type = video_type;
        session_attr.video_width = video_width;
        session_attr.video_height = video_height;
        session_attr.session_name = session_name.append(std::to_string(i));

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