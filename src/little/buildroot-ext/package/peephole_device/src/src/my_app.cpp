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
#include <cstring>
#include <sys/statfs.h>
#include "my_app.h"
#include "my_muxer.h"
#include "g711_mix_audio.h"
#include "mapi_sys_api.h"

static uint64_t get_avail_space_megabytes(const std::string &dir) {
    struct statfs diskInfo;
    if (statfs(dir.c_str(), &diskInfo) < 0) {
        return 0;
    }

    uint64_t blocksize = diskInfo.f_bsize;
    // uint64_t totalsize = blocksize * diskInfo.f_blocks;
    // uint64_t freeDisk = diskInfo.f_bfree * blocksize;
    uint64_t availableDisk = diskInfo.f_bavail * blocksize;
    return (availableDisk >> 20);
}

// FIXME
static bool flash_boot(void) {
    char buf[1024];
    bool ret = false;
    FILE* fp = popen("cat /proc/cmdline", "r");
    if (fp) {
        while (fgets(buf, 1024, fp) != NULL) {
            std::string str = buf;
            if (str.find("ubifs") != std::string::npos) {
                ret = true;
                break;
            }
        }
        pclose(fp);
    } else {
        std::cout << "popen fail!" << std::endl;
        return false;
    }
    return ret;
}

int MyApp::Init(unsigned short port) {
    printf("peephole_before comminit : %llu\n", perf_get_smodecycles());
    if (client_.Init("peephole", this) < 0) {
        std::cout << "MyApp::Init() client_.init failed" << std::endl;
        return -1;
    }
    printf("peephole_after comminit : %llu\n", perf_get_smodecycles());

    key_detect_thread_exit_flag_.store(false);
    key_detect_thread_ = std::thread([this](){
        key_event_handler();
    });

    if (flash_boot()) {
        storage_ready_ = 0;
        sd_thd_ = std::thread([]() {
            usleep(3000 * 1000); // FIXME, netdrv has higher priority
            std::cout << "install mmc drv... " << std::endl;
            system("insmod /lib/modules/5.10.4/kernel/drivers/mmc/core/mmc_core.ko");
            // system("insmod /lib/modules/5.10.4/kernel/drivers/mmc/core/pwrseq_simple.ko");
            // system("insmod /lib/modules/5.10.4/kernel/drivers/mmc/core/pwrseq_emmc.ko");
            system("insmod /lib/modules/5.10.4/kernel/drivers/mmc/core/mmc_block.ko");
            system("insmod /lib/modules/5.10.4/kernel/drivers/mmc/host/sdhci.ko");
            system("insmod /lib/modules/5.10.4/kernel/drivers/mmc/host/sdhci-pltfm.ko");
            system("insmod /lib/modules/5.10.4/kernel/drivers/mmc/host/sdhci-of-kendryte.ko");
            usleep(1000 * 1000);
            std::cout << "mount sdcard... " << std::endl;
            system("mkdir -p /sharefs");
            system("mount -t vfat /dev/mmcblk1p4 /sharefs");
            system("mkdir -p /sharefs/DCIM/video");
            system("mkdir -p /sharefs/DCIM/snapshot");
            std::cout << "mount sdcard... done" << std::endl;
            storage_ready_ = 1;
        });
    } else {
        storage_ready_ = 1;
    }

    mp4_muxer_.Init(this);
    if (enable_mp4_record_) cmd_start_record();

    jpeg_muxer_.Init(this);

    printf("peephole_after CreateRtspServer : %llu\n", perf_get_smodecycles());
    if (CreateRtspServer() < 0) {
        std::cout << "MyApp::Init() CreateRtspServer failed" << std::endl;
        return - 1;
    }
    printf("peephole_after CreateRtspServer : %llu\n", perf_get_smodecycles());

    request_power_off_.store(false);
    playback_ack_.store(false);
    if (rpc_server_.Init(this, port) < 0) {
        std::cout << "MyApp::Init() rpc_server_.init failed" << std::endl;
        return -1;
    }

    // std::cout << " mp4 available space (MB) : " << get_avail_space_megabytes(mp4_dir_) << std::endl;
    // std::cout << "jpeg available space (MB) : " << get_avail_space_megabytes(jpeg_dir_) << std::endl;

    // init done, notify the comm-server that we are ready
    client_.Start();
    printf("peephole_after comm-client start : %llu\n", perf_get_smodecycles());
    return 0;
}

void MyApp::DeInit() {
    if (enable_mp4_record_) {
        this->EnableRecord(false);
    }
    if (key_detect_thread_.joinable()) {
        key_detect_thread_exit_flag_.store(true);
        key_detect_thread_.join();
    }
    rpc_server_.DeInit();
    // FIXME, stop & destroy rtsp-server may introduce segment-fault at the moment.
#if 1
    rtsp_started_ = false;
#else
    StopRtspServer();
    DestroyRtspServer();
#endif
    client_.Stop();
    client_.DeInit();
    mp4_muxer_.DeInit();
    jpeg_muxer_.DeInit();
    //
    if (flash_boot()) {
        if (sd_thd_.joinable()) {
            sd_thd_.join();
        }
        /*
        system("umount /sharefs");
        system("rmmod sdhci-of-kendryte");
        system("rmmod sdhci-pltfm");
        system("rmmod sdhci");
        system("rmmod mmc_block");
        system("rmmod mmc_core");
        */
    }
    storage_ready_ = 0;
}

static uint64_t get_steady_time_ms();
int MyApp::EnterPlaybackMode(bool sync)
{
    static bool bEnterPlaybackMode = false;
    if (!bEnterPlaybackMode)
    {
        std::cout << "EnterPlaybackMode --- 1 -- " << get_steady_time_ms() << std::endl;
        if (enable_mp4_record_) {
            this->EnableRecord(false);
        }
        // exit all the services except comm
        if (key_detect_thread_.joinable()) {
            key_detect_thread_exit_flag_.store(true);
            key_detect_thread_.join();
        }
        rpc_server_.DeInit();
        std::cout << "EnterPlaybackMode --- 2 -- " << get_steady_time_ms() << std::endl;
        // FIXME, stop & destroy rtsp-server may introduce segment-fault at the moment.
    #if 1
        rtsp_started_ = false;
    #else
        StopRtspServer();
        DestroyRtspServer();
    #endif

        playback_ack_.store(false);
        client_.EnterPlaybackMode();
        std::cout << "EnterPlaybackMode --- 3 -- " << get_steady_time_ms() << std::endl;
        {
            int count = 0;
            while (!playback_ack_.load()) {
                usleep(1000);
                ++count;
                if(count > 5000) {
                    std::cout << "PLAYBACK_ACK timeout" << std::endl;
                    break;
                }
            }
            if (playback_ack_.load()) {
                std::cout << "PLAYBACK_ACK received" << std::endl;
            }
        }
        std::cout << "========waiting for mapi service ready " << get_steady_time_ms() << std::endl;
        int ret = 0;
        while(1) {
            ret = kd_mapi_sys_init();
            if (ret != K_SUCCESS) {
                printf("kd_mapi_sys_init error\n.");
                // return -1;
                usleep(1000 * 10);
            } else {
                break;
            }
        }
        std::cout << "========kd_mapi_sys_init done " << get_steady_time_ms() << std::endl;
        bEnterPlaybackMode = true;

        // stop recording and wait all data written
        mp4_muxer_.DeInit();
        jpeg_muxer_.DeInit();
        std::cout << "EnterPlaybackMode --- Done -- " << get_steady_time_ms() << std::endl;
    }
    return 0;
}

int MyApp::RpcSendEvent(const RpcEventData &event) {
    return rpc_server_.SendEvent(event);
}

// IRpcClientCallback
void MyApp::OnNewClient(int64_t conn_id, const std::string &remote_ip) {
    std::cout << "MyApp::OnNewClient -- id " << (long) conn_id << ", remote_ip : " << remote_ip << std::endl;
    printf("OnNewClient : %llu\n", perf_get_smodecycles());
    StopRtspServer();
    StartRtspServer();
}

void MyApp::OnDisconnect(int64_t conn_id, const std::string &reason) {
    std::cout << "MyApp::OnDisconnect -- id " << (long) conn_id << ", reason : " << reason << std::endl;
    StopRtspServer();
}

void MyApp::OnGetInfo(RpcServerInfo &info) {
    info.has_bidirection_speech = true;
    info.speech_service_port = stream_port_;
    info.speech_stream_name = stream_url_;
}

void MyApp::OnRequest(const RpcRequest &req) {
    switch(req.type) {
    case RpcRequest::Type::POWER_OFF: {
        std::cout << "MyApp::OnRequest -- POWER_OFF" << std::endl;
        request_power_off_.store(true);
        break;
    }
    default: break;
    }
}

// IClientCallback
void MyApp::OnMessage(const UserMessage *message) {
    UserMsgType type = static_cast<UserMsgType>(message->type);
    switch(type) {
    case UserMsgType::SERVER_MSG_BASE: {
        std::cout << "MyApp::OnMessage() -- SERVER_MSG_BASE" << std::endl;
        std::cout << message->data << std::endl; // "hello,world"
        break;
    }
    case UserMsgType::PLAYBACK_ACK : {
        std::cout << "MyApp::OnMessage() -- PLAYBACK_ACK" << std::endl;
        playback_ack_.store(true);
        break;
    }
    case UserMsgType::CURRENT_MODE_TYPE: {
        std::cout << "MyApp::OnMessage() -- CURRENT_MODE_TYPE" << std::endl;
        if (!strcmp(message->data, "doorbell_mode")) {
            current_mode_ = DOORBELL_MODE;
        } else if (!strcmp(message->data, "pir_mode")) {
            current_mode_ = PIR_MODE;
        } else {
            current_mode_ = MODE_BUTT;
        }
        break;
    }
    default: {
        std::cout << "MyApp::OnMessage() -- unknown message : " << message->type << std::endl;
        break;
    }
    }
}

#include <sstream>
static std::string int64_to_string(int64_t value ) {
    std::ostringstream os;
    os << value;
    return os.str();
}

// IJpegMuxerCallback
void MyApp::GetJpegFilename(UserEventData::EventType type, int64_t timestamp_ms, std::string &filename) {
    using DataType = UserEventData::EventType;

    filename = jpeg_dir_ + "/";
    if (type == DataType::PIR_WAKEUP) filename += "pir_wakeup_";
    else if (type == DataType::KEY_WAKEUP) filename += "key_wakeup_";
    else if (type == DataType::STAY_ALARM) filename += "stay_alarm_";
    else {
        std::cout << "unknown event type, please check it" << std::endl;
        filename += "unknown_";
    }
    filename += int64_to_string(timestamp_ms);
    filename += ".jpg";
}

bool MyApp::IsJpegSpaceAvailable() {
    // at least 4M bytes left
    return get_avail_space_megabytes(jpeg_dir_) > 4;
}

void MyApp::OnJpegMuxerError(int err_code) {
    // TODO
    std::cout << "TODO, OnJpegMuxerError -- " << err_code << std::endl;
}

void MyApp::OnEvent(const UserEventData &event) {
    switch(event.type) {
    case UserEventData::EventType::PIR_WAKEUP:{
        std::cout << "MyApp::OnEvent() -- PIR_WAKEUP" << std::endl;
        RpcEventData rpc_event;
        rpc_event.type = RpcEventData::EventType::PIR_WAKEUP;
        rpc_event.timestamp_ms = event.timestamp_ms;
        rpc_event.jpeg = event.jpeg;
        rpc_event.jpeg_size = event.jpeg_size;
        RpcSendEvent(rpc_event);
        break;
    }
    case UserEventData::EventType::KEY_WAKEUP:{
        std::cout << "MyApp::OnEvent() -- KEY_WAKEUP" << std::endl;
        RpcEventData rpc_event;
        rpc_event.type = RpcEventData::EventType::KEY_WAKEUP;
        rpc_event.timestamp_ms = event.timestamp_ms;
        rpc_event.jpeg = event.jpeg;
        rpc_event.jpeg_size = event.jpeg_size;
        RpcSendEvent(rpc_event);
        break;
    }
    case UserEventData::EventType::STAY_ALARM:{
        std::cout << "MyApp::OnEvent() -- STAY_ALARM" << std::endl;
        RpcEventData rpc_event;
        rpc_event.type = RpcEventData::EventType::STAY_ALARM;
        rpc_event.timestamp_ms = event.timestamp_ms;
        rpc_event.jpeg = event.jpeg;
        rpc_event.jpeg_size = event.jpeg_size;
        RpcSendEvent(rpc_event);
        break;
    }
    default: {
        std::cout << "MyApp::OnEvent() -- unknown message : " << static_cast<int>(event.type) << std::endl;
        return;
    }
    }

    if(enable_jpeg_recod_) {
        jpeg_muxer_.Write(event);
    }
}

// IMp4MuxerCallback
void MyApp::GetMp4Filename(std::string &filename) {
    filename = mp4_dir_ + "/peephole.mp4";
}

bool MyApp::IsMp4SpaceAvailable() {
    // at least 4M bytes left
    return get_avail_space_megabytes(mp4_dir_) > 4;
}

void MyApp::OnMp4MuxerError(int err_code) {
    // TODO
    std::cout << "TODO, OnMp4MuxerError -- " << err_code << std::endl;
}

#include <sys/time.h>
#include <time.h>
static uint64_t get_steady_time_ms() {
    struct timespec ts{};
    clock_gettime(CLOCK_MONOTONIC, &ts);
    uint64_t ms = (ts.tv_sec * 1000);
    ms += ts.tv_nsec/1000000;
    return ms;
}

// TODO, optimze data copy
void MyApp::OnVEncFrameData(const AVEncFrameData &data) {
    // std::cout << "MyApp::OnVEncFrameData() -- addr = " << (long)data.data << ", size = " << data.size << std::endl;
    std::unique_lock<std::mutex> rtsp_lck(rtsp_mutex_);
    if (rtsp_started_) {
        if(!video_data_realtime_flag_) {
            uint64_t curr = get_steady_time_ms();
            if (!previous_video_timestamp_ms_  || curr - previous_video_timestamp_ms_ < 30) {
                previous_video_timestamp_ms_ = curr;
            } else {
                video_data_realtime_flag_ = true;
                previous_video_timestamp_ms_ = 0;
            }
        }
        if (video_data_realtime_flag_) {
            // std::cout << "MyApp::OnVEncFrameData() -- addr = " << (long)data.data << ", size = " << data.size << std::endl;
            rtsp_server_.SendVideoData(stream_url_, data.data, data.size, data.timestamp_ms);
        }
    }
    rtsp_lck.unlock();

    if (enable_mp4_record_) {
        mp4_muxer_.Write(data);
    }
}

void MyApp::OnAEncFrameData(const AVEncFrameData &data) {
    // std::cout << "MyApp::OnAEncFrameData() -- addr = " << (long)data.data << ", size = " << data.size << std::endl;
    std::unique_lock<std::mutex> rtsp_lck(rtsp_mutex_);
    if (rtsp_started_) {
        if(!audio_data_realtime_flag_) {
            uint64_t curr = get_steady_time_ms();
            if (!previous_audio_timestamp_ms_  || curr - previous_audio_timestamp_ms_ < 40) {
                previous_audio_timestamp_ms_ = curr;
            } else {
                audio_data_realtime_flag_ = true;
                previous_audio_timestamp_ms_ = 0;
            }
        }
        if (audio_data_realtime_flag_) {
            // std::cout << "MyApp::OnAEncFrameData() -- addr = " << (long)data.data << ", size = " << data.size << std::endl;
            rtsp_server_.SendAudioData(stream_url_, data.data, data.size, data.timestamp_ms);
        }
    }
    rtsp_lck.unlock();

    if (enable_mp4_record_) {
        // mix the audio data, FIXME
        std::unique_lock<std::mutex> lck(aq_mutex_);
        if (!audio_queue_.empty()) {
            auto back_data = audio_queue_.front();
            audio_queue_.pop();
            lck.unlock();
            if (back_data.size != data.size) {
                std::cout << "audio frame size not the same" << std::endl;
            }
            else if (audio_data_realtime_flag_) {
                kd_mix_g711u_audio((k_char *)data.data, (k_char *)back_data.data.get(), data.size, (k_char *)data.data);
            }
        } else {
            lck.unlock();
        }
        mp4_muxer_.Write(data);
    }
}

struct InterruptCb : public IInterruptCallback {
    explicit InterruptCb(std::atomic<bool> &flag) : flag_(flag) {}
    virtual bool Exit() override { return !flag_; }
  private:
    std::atomic<bool> &flag_;
};
void MyApp::OnBackChannelData(std::string &session_name, const uint8_t *data, size_t size, uint64_t timestamp) {
    //  TODO， need to queue data to handle jitter and control accumulation
    //   gather data to get complete frame(40ms).
    if (backchannel_data_size == 0) {
        timestamp_backchanel = timestamp;
    }
    for (size_t i = 0; i < size ;i++) {
        g711_buffer_backchannel[backchannel_data_size++] = data[i];
        if (backchannel_data_size == 320) {
            InterruptCb cb(rtsp_started_);
            AVEncFrameData data;
            data.type = AVEncFrameData::Type::PCMU;
            data.data = g711_buffer_backchannel;
            data.size = backchannel_data_size;
            data.timestamp_ms = timestamp_backchanel;
            data.keyframe = true;
            data.sequence = 0; // not used
            data.flags = 0;  // not used
            this->SendAudioData(data, &cb);

            if (enable_mp4_record_) {
                BackchannelData audio_data;
                audio_data.size = 320;
                std::shared_ptr<uint8_t> p(new uint8_t[audio_data.size], std::default_delete<uint8_t[]>());
                memcpy(p.get(), g711_buffer_backchannel, audio_data.size);
                std::unique_lock<std::mutex> lck(aq_mutex_);
                if (audio_queue_.size() > 2) audio_queue_.pop();
                audio_queue_.push(audio_data);
            }

            backchannel_data_size = 0;
            timestamp_backchanel = timestamp;
        }
    }
}

int MyApp::CreateRtspServer() {
    std::unique_lock<std::mutex> rtsp_lck(rtsp_mutex_);
    if (rtsp_server_.Init(stream_port_, this) < 0) {
        return -1;
    }
    std::cout << "MyApp::CreateRtspServer() done" << std::endl;
    rtsp_started_ = false;
    return 0;
}

int MyApp::DestroyRtspServer() {
    std::unique_lock<std::mutex> rtsp_lck(rtsp_mutex_);
    if (rtsp_started_) return -1;
    rtsp_server_.DeInit();
    std::cout << "MyApp::DestroyRtspServer() done" << std::endl;
    return 0;
}

int MyApp::StartRtspServer() {
    std::unique_lock<std::mutex> rtsp_lck(rtsp_mutex_);
    if(rtsp_started_) return 0;
    SessionAttr session_attr;
    session_attr.with_audio = true;
    session_attr.with_audio_backchannel = true; // TODO, depends on work-mode
    session_attr.with_video = true;
    session_attr.video_type = VideoType::kVideoTypeH265;
    if (rtsp_server_.CreateSession(stream_url_, session_attr) < 0) {
        std::cout << "MyApp::StartRtspServer() failed to create sessions" << std::endl;
        return -1;
    }
    rtsp_server_.Start();
    backchannel_data_size = 0;
    video_data_realtime_flag_.store(false);
    previous_video_timestamp_ms_ = 0;
    audio_data_realtime_flag_.store(false);
    previous_audio_timestamp_ms_ = 0;
    rtsp_started_ = true;
    std::cout << "MyApp::StartRtspServer() done" << std::endl;
    return 0;
}

int MyApp::StopRtspServer() {
    std::unique_lock<std::mutex> rtsp_lck(rtsp_mutex_);
    if (!rtsp_started_) return 0;
    rtsp_server_.Stop();
    rtsp_started_ = false;
    std::cout << "MyApp::StopRtspServer() done" << std::endl;
    return 0;
}

/**
 * NOTE: KEY_WAKEUP is defined in "linux/input.h",
 * So we have to place the codes below at the bottom of this file to avoid name polution.
*/
#include <fcntl.h>
#include <sys/select.h>
#include <linux/input.h>
static int kbhit (int fd) {
    struct timeval tv;
    tv.tv_sec = 0;
    tv.tv_usec = 10000;

    fd_set rdfs;
    FD_ZERO(&rdfs);
    FD_SET (fd, &rdfs);

    while (select(fd+1, &rdfs,  NULL, NULL, &tv) < 0 && errno == EINTR);
    return FD_ISSET(fd, &rdfs);
}

void MyApp::key_event_handler() {
    struct input_event ev[64];
    fd_set rdfs;

    const char *filename = "/dev/input/event0";
    int fd = open(filename, O_RDONLY);
    if (fd < 0) {
        printf("Can not open %s\n", filename);
        return;
    }

    FD_ZERO(&rdfs);
    FD_SET(fd, &rdfs);
    while (!key_detect_thread_exit_flag_) {
        if (!kbhit(fd)) {
            continue;
        }
        int rd = read(fd, ev, sizeof(ev));
        if (rd < (int) sizeof(struct input_event)) {
            printf("expected %d bytes, got %d\n", (int) sizeof(struct input_event), rd);
            perror("\nkey: error reading");
            return;
        }
        for (int i = 0; i < rd / sizeof(struct input_event); i++) {
            if (ev[i].type == EV_KEY) {
                // printf("type %d (%s), code %d (%s), ",type, typename(type),
                // 	code, codename(type, code));
                if(ev[i].value) {
                    if(i == 0) {
                        printf("send ipcmsg: get doorbell\n");
                        KeyPressed();
                    }
                }
            }
        }
    }
    ioctl(fd, EVIOCGRAB, (void*)0);
    close(fd);
}
