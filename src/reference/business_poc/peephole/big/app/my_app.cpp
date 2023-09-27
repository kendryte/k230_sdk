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

#include <thread>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <fstream>
#include "my_app.h"

struct InterrruptCb : public IInterruptCallback {
    explicit InterrruptCb(std::atomic<bool> &exit_flag) : exit_flag_(exit_flag) {}
    virtual bool Exit() override { return exit_flag_; }
  private:
    std::atomic<bool> &exit_flag_;
};

int MyApp::Init() {
    media_ = new Media();
    media_->Init();
    comm_server_ = new MyCommServer();
    comm_server_->Init();
    ReceiveMessageThread();
    mode_switch_ = current_mode_;
    return 0;
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

int MyApp::DeInit(bool send_msg) {
    recevie_msg_exit_flag_.store(true);
    if (receive_message_thread_.joinable()) {
        receive_message_thread_.join();
    }

    // deinit vb before PLAYBACK_ACK sent
    if (media_) {
        media_->DeInit();
        delete media_;
        media_ = nullptr;
        std::cout << "media_ deinit, " << get_steady_time_ms() << std::endl;
    }

    if (send_msg && comm_server_) {
        // TODO, use async message
        UserMessage msg;
        msg.type = static_cast<int>(UserMsgType::PLAYBACK_ACK);
        msg.len = 0;
        comm_server_->SendMessage(&msg, sizeof(msg));
        std::cout << "PLAYBACK_ACK sent, " << get_steady_time_ms() << std::endl;
    }

    if (comm_server_) {
        comm_server_->DeInit();
        delete comm_server_;
        comm_server_ = nullptr;
        std::cout << "comm_server_ deinit, " << get_steady_time_ms() << std::endl;
    }
    return 0;
}

int MyApp::EnterPlaybackMode() {
    current_mode_ = PLAYBACK_MODE;

    std::cout << "Enter Playback Mode..." << get_steady_time_ms() << std::endl;
    DeInit(true);
    std::cout << "DeInit done..." << get_steady_time_ms() << std::endl;

    while (getchar() != 'q') {
        usleep(10000);
    }

    std::cout << "Exit Playback Mode..." << std::endl;
    return 0;
}

TrigerMode MyApp::EnterPirMode() {
    current_mode_ = PIR_MODE;
    kmodel_path_ = "person_detect_yolov5n.kmodel";

    media_->VcapSetDevAttr();
    // for ai
    {
        VcapChnAttr vcap_chn_attr;
        k_vicap_chn vi_detect_chn = VICAP_CHN_ID_0;
        vcap_chn_attr.output_width = 720;
        vcap_chn_attr.output_height = 1280;
        vcap_chn_attr.crop_width = 1088;
        vcap_chn_attr.crop_height = 1920;
        vcap_chn_attr.pixel_format = PIXEL_FORMAT_BGR_888_PLANAR;
        media_->VcapSetChnAttr(vi_detect_chn, vcap_chn_attr);
    }
    // for snapshot
    {
        VcapChnAttr vcap_chn_attr;
        memset(&vcap_chn_attr, 0, sizeof(vcap_chn_attr));
        vcap_chn_attr.output_width = 1088;
        vcap_chn_attr.output_height = 1920;
        vcap_chn_attr.crop_width = 1088;
        vcap_chn_attr.crop_height = 1920;
        vcap_chn_attr.pixel_format = PIXEL_FORMAT_YUV_SEMIPLANAR_420;
        media_->VcapSetChnAttr(VICAP_CHN_ID_2, vcap_chn_attr);
    }
    media_->VcapInit();

    int venc_snap_chn_ = 0;
    media_->VencSnapChnCreate(venc_snap_chn_, 1088, 1920);
    media_->VencSnapChnStart();
    media_->VcapStart();

    ai_exit_flag_.store(false);
    venc_exit_flag_.store(false);
    send_event_exit_flag_.store(false);
    exit_msg_.store(false);
    StartPirSnapThread();
    StartPersonDetectThread();
    StartSendEventThread();

    while (!ai_exit_flag_ && !venc_exit_flag_) {
        usleep(10000);
    }
    media_->VcapStop();
    media_->VencSnapChnStop();
    if (person_detect_thread_.joinable())
        person_detect_thread_.join();

    if (venc_thread_.joinable())
        venc_thread_.join();

    if (send_event_thread_.joinable())
        send_event_thread_.join();

    media_->VencSnapChnDestory();

    std::cout << "PIR mode exit" << std::endl;
    return mode_switch_;
}

void MyApp::StartPersonDetectThread() {
    person_detect_thread_ = std::thread([this]() {
        vector<BoxInfo> results;
        ScopedTiming st;
        k_u64 paddr = 0;
        void *vaddr = nullptr;
        k_u32 size = 3 * detect_width_ * detect_height_;
        media_->MediaSysAlloc(&paddr, &vaddr, size);

        person_detect_ = new personDetect(kmodel_path_.c_str(), 0.5, 0.45, {3, detect_height_, detect_width_}, reinterpret_cast<uintptr_t>(vaddr), reinterpret_cast<uintptr_t>(paddr), 0);
        while (!ai_exit_flag_) {
            k_video_frame_info dump_frame;
            k_s32 ret = media_->VcapGetDumpFrame(VICAP_CHN_ID_0, &dump_frame, vaddr);
            if (ret != K_SUCCESS) {
                continue;
            }

            results.clear();
            person_detect_->pre_process();
            person_detect_->inference();
            person_detect_->post_process({detect_width_, detect_height_}, results);
            if (results.size() > 0) {
                if (!start_detect_) {
                    st.CheckStartTimer();
                    start_detect_.store(true);
                } else {
                    int elapsed = st.ElapsedSeconds();
                    if (elapsed >= 10) {
                        printf("stay aram...\n");
                        check_stay_.store(true);
                        start_detect_.store(false);
                        st.TimerStop();
                    }
                }
            } else {
                st.TimerStop();
                start_detect_.store(false);
            }

            media_->VcapReleaseDumpFrame(VICAP_CHN_ID_0, &dump_frame);
        }
        delete person_detect_;
        media_->MediaSysFree(paddr, vaddr);
    });
}

void MyApp::StartSendEventThread() {
    int index = 0;
    send_event_thread_ = std::thread([this]() {
        InterrruptCb cb(send_event_exit_flag_);
        while (!send_event_exit_flag_) {
            if (exit_msg_) {
                send_event_exit_flag_.store(true);
                continue;
            }
            if (current_mode_ == PIR_MODE) {
                if (client_ready_ ) {
                    if (pir_event_queue_.empty()) {
                        std::this_thread::sleep_for(std::chrono::milliseconds(20));
                        continue;
                    }
                    UserEventData event;
                    pir_event_queue_.take(event);
                    comm_server_->SendEvent(event, &cb);
                    delete[] event.jpeg;
                }
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(20));
        }
    });
}

void MyApp::StartPirSnapThread() {
    venc_thread_ = std::thread([this](){
        int snap_chn = 0;
        while (!venc_exit_flag_) {
            VencChnStatus status;
            k_venc_stream venc_stream;
            memset(&venc_stream, 0, sizeof(venc_stream));

            media_->VencChnQueryStatus(snap_chn, &status);
            if (status.cur_packs > 0) {
                venc_stream.pack_cnt = status.cur_packs;
            } else {
                venc_stream.pack_cnt = 1;
            }

            venc_stream.pack = new k_venc_pack[venc_stream.pack_cnt];

            k_s32 ret = media_->VencChnGetStream(snap_chn, &venc_stream);
            if (ret != K_SUCCESS) {
                delete[] venc_stream.pack;
                continue;
            }

            if (first_snap_) {
                for (int i = 0; i < venc_stream.pack_cnt; i++) {
                    k_u8 *pData;
                    pData = (k_u8 *)kd_mpi_sys_mmap(venc_stream.pack[i].phys_addr, venc_stream.pack[i].len);

                    k_u8 *data_temp;
                    data_temp = new k_u8[venc_stream.pack[i].len];
                    memcpy(data_temp, pData, venc_stream.pack[i].len);
                    printf("snap first data len = %d...\n", venc_stream.pack[i].len);

                    UserEventData user_event;
                    user_event.type = UserEventData::EventType::PIR_WAKEUP;
                    user_event.jpeg = data_temp;
                    user_event.jpeg_size = venc_stream.pack[i].len;
                    user_event.timestamp_ms = venc_stream.pack[i].pts;

                    kd_mpi_sys_munmap(pData, venc_stream.pack[i].len);

                    pir_event_queue_.put(user_event);
                }

                first_snap_.store(false);
            } else if (check_stay_) {
                for (int i = 0; i < venc_stream.pack_cnt; i++) {
                    k_u8 *pData;
                    pData = (k_u8 *)kd_mpi_sys_mmap(venc_stream.pack[i].phys_addr, venc_stream.pack[i].len);

                    k_u8 *data_temp = new k_u8[venc_stream.pack[i].len];
                    memcpy(data_temp, pData, venc_stream.pack[i].len);

                    UserEventData user_event;
                    user_event.type = UserEventData::EventType::STAY_ALARM;
                    user_event.jpeg = data_temp;
                    user_event.jpeg_size = venc_stream.pack[i].len;
                    user_event.timestamp_ms = venc_stream.pack[i].pts;

                    kd_mpi_sys_munmap(pData, venc_stream.pack[i].len);

                    pir_event_queue_.put(user_event);
                }

                check_stay_.store(false);
            }
            media_->VencChnReleaseStream(snap_chn, &venc_stream);

            delete[] venc_stream.pack;
        }
    });
}

TrigerMode MyApp::EnterDoorBellMode() {
    current_mode_ = DOORBELL_MODE;
    key_pressed_.store(true);

    media_->ADecAoCreate();
    media_->ADecAoInnerStart();
    media_->ADecAoExternStart();
    media_->AiAEncCreate();
    media_->AiAEncStart();

    int venc_snap_chn_ = 0;
    int venc_chn_ = 1;
    media_->VencChnCreate(venc_chn_, 1088, 1920);
    media_->VencSnapChnCreate(venc_snap_chn_, 1088, 1920);
    media_->VcapSetDevAttr();
    // for vo
    {
        VcapChnAttr vcap_chn_attr;
        memset(&vcap_chn_attr, 0, sizeof(vcap_chn_attr));
        vcap_chn_attr.output_width = 1088;
        vcap_chn_attr.output_height = 1920;
        vcap_chn_attr.crop_width = 1088;
        vcap_chn_attr.crop_height = 1920;
        vcap_chn_attr.pixel_format = PIXEL_FORMAT_YUV_SEMIPLANAR_420;
        media_->VcapSetChnAttr(VICAP_CHN_ID_0, vcap_chn_attr);
    }
    // for h26x encode
    {
        VcapChnAttr vcap_chn_attr;
        memset(&vcap_chn_attr, 0, sizeof(vcap_chn_attr));
        vcap_chn_attr.output_width = 1088;
        vcap_chn_attr.output_height = 1920;
        vcap_chn_attr.crop_width = 1088;
        vcap_chn_attr.crop_height = 1920;
        vcap_chn_attr.pixel_format = PIXEL_FORMAT_YUV_SEMIPLANAR_420;
        media_->VcapSetChnAttr(VICAP_CHN_ID_1, vcap_chn_attr);
    }

    // for snapshot
    {
        VcapChnAttr vcap_chn_attr;
        memset(&vcap_chn_attr, 0, sizeof(vcap_chn_attr));
        vcap_chn_attr.output_width = 1088;
        vcap_chn_attr.output_height = 1920;
        vcap_chn_attr.crop_width = 1088;
        vcap_chn_attr.crop_height = 1920;
        vcap_chn_attr.pixel_format = PIXEL_FORMAT_YUV_SEMIPLANAR_420;
        media_->VcapSetChnAttr(VICAP_CHN_ID_2, vcap_chn_attr);
    }
    media_->VcapInit();
    media_->VoInit();
    media_->VencChnStart();
    media_->VencSnapChnStart();
    media_->VcapStart();

    doorbell_palyaudio_exit_flag_.store(false);
    doorbell_venc_exit_flag_.store(false);
    doorbell_aenc_exit_flag_.store(false);

    StartPlayAudioThread();
    StartAencThread();

    StartVencThread();
    StartVencSnapThread();

    StartAencSendThread();
    StartVencSendThread();

    while(!doorbell_venc_exit_flag_
         &&!doorbell_aenc_exit_flag_
         &&!doorbell_palyaudio_exit_flag_) {
        usleep(10000);
    }

    media_->VcapStop();
    media_->VencSnapChnStop();
    media_->VencChnStop();
    media_->AiAencStop();
    media_->ADecAoInnerStop();
    media_->ADecAoExternStop();

    if (doorbell_venc_thread_.joinable())
        doorbell_venc_thread_.join();

    if (venc_send_thread_.joinable())
        venc_send_thread_.join();

    if (doorbell_snap_thread_.joinable())
        doorbell_snap_thread_.join();

    if (doorbell_aenc_thread_.joinable())
        doorbell_aenc_thread_.join();

    if (aenc_send_thread_.joinable())
        aenc_send_thread_.join();

    if (play_audio_thread_.joinable())
        play_audio_thread_.join();

    media_->VencSnapChnDestory();
    media_->VencChnDestory();
    media_->VoDeInit();
    media_->AiAEncDestory();
    media_->ADecAoDestroy();
    std::cout << "Doorbell mode exit" << std::endl;
    return mode_switch_;
}

void MyApp::StartAencThread() {
    doorbell_aenc_thread_ = std::thread([this]() {
        k_u32 aenc_sequence{0};
        while (!doorbell_aenc_exit_flag_) {
            k_audio_stream audio_stream;
            k_s32 ret = media_->AEncGetStream(&audio_stream);
            if (ret != K_SUCCESS) {
                usleep(1000);
                continue;
            }
            k_u8 *pData = media_->SysMmap(audio_stream.phys_addr, audio_stream.len);
            k_u8 *data_temp = new k_u8[audio_stream.len];
            memcpy(data_temp, pData, audio_stream.len);
            media_->SynMunmap(pData, audio_stream.len);
            media_->AEncReleaseStream(&audio_stream);
            {
                AVEncFrameData aframe;
                aframe.data = data_temp;
                aframe.size = audio_stream.len;
                aframe.type = AVEncFrameData::Type::PCMU;
                aframe.timestamp_ms = audio_stream.time_stamp / 1000;
                aframe.sequence = aenc_sequence++;
                aframe.flags = client_ready_;
                if (aenc_stream_queue_.put(aframe) == false) {
                    delete []data_temp;
                }
            }
        }
        aenc_senc_exit_.store(true);
        printf("StartAencThread  stop...\n");
    });
}

void MyApp::StartAencSendThread() {
    aenc_send_thread_ = std::thread([this]() {
        InterrruptCb cb(aenc_senc_exit_);
        while (!aenc_senc_exit_) {
            if (!client_ready_) {
                usleep(10000);
                continue;
            }

            if (!aenc_stream_queue_.empty()) {
                AVEncFrameData aframe;
                aenc_stream_queue_.take(aframe);
                int ret = comm_server_->SendAudioData(aframe, &cb);
                if (ret < 0) {
                    std::cout << "send audio data to client faield." << std::endl;
                }
                delete[] aframe.data;
                usleep(5000);
            } else {
                usleep(10000);
            }
        }

        while (!aenc_stream_queue_.empty()) {
            AVEncFrameData aframe;
            aenc_stream_queue_.take(aframe);
            delete[] aframe.data;
        }
        printf("StartAencSendThread  stop...\n");
    });
}

void MyApp::StartVencSendThread() {
    venc_send_thread_ = std::thread([this]() {
        int ret = 0;
        InterrruptCb cb(venc_senc_exit_);
        while (!venc_senc_exit_) {
            if (!client_ready_) {
                usleep(10000);
                continue;
            }

            if (!send_video_event_data_) {
                if (!doorbell_event_vec_.empty()) {
                    ret = comm_server_->SendEvent(doorbell_event_vec_[0], &cb);
                    if (ret < 0) {
                        std::cout << "send doorbell event to client faield." << std::endl;
                    }
                    delete[] doorbell_event_vec_[0].jpeg;
                    send_video_event_data_.store(true);
                    doorbell_event_vec_.clear();
                }
            }

            if (!venc_stream_queue_.empty()) {
                AVEncFrameData vframe;
                venc_stream_queue_.take(vframe);
                ret = comm_server_->SendVideoData(vframe, &cb);
                if (ret < 0) {
                    std::cout << "send video data to client faield." << std::endl;
                }
                delete[] vframe.data;
                usleep(5000);
            } else {
                usleep(10000);
            }
        }

        while (!venc_stream_queue_.empty()) {
            AVEncFrameData vframe;
            venc_stream_queue_.take(vframe);
            delete[] vframe.data;
        }

        printf("StartVencSendThread stop...\n");
    });
}

void MyApp::StartVencThread() {
    doorbell_venc_thread_ = std::thread([this]() {
        int ret;
        int venc_chn = 1;
        bool get_header {false};
        bool is_idr {false};
        k_u32 venc_sequence {0};

        while (!doorbell_venc_exit_flag_) {
            VencChnStatus status;
            k_venc_stream venc_stream;
            memset(&venc_stream, 0, sizeof(venc_stream));
            if (media_->VencChnQueryStatus(venc_chn, &status)) {
                usleep(1000);
                continue;
            }
            if (status.cur_packs > 0) {
                venc_stream.pack_cnt = status.cur_packs;
            } else {
                venc_stream.pack_cnt = 1;
            }

            venc_stream.pack = new k_venc_pack[venc_stream.pack_cnt];
            ret = media_->VencChnGetStream(venc_chn, &venc_stream);
            if (ret != K_SUCCESS) {
                delete[] venc_stream.pack;
                continue;
            }

            for (int i = 0; i < venc_stream.pack_cnt; i++) {
                k_u8 *pData = (k_u8 *)kd_mpi_sys_mmap(venc_stream.pack[i].phys_addr, venc_stream.pack[i].len);
                // printf("-----venc_stream.pack[%d].pts = %lld, len = %d\n", i, venc_stream.pack[i].pts, venc_stream.pack[i].len);
                if (venc_stream.pack[i].type == K_VENC_HEADER) {
                    memcpy(venc_header_, pData, venc_stream.pack[i].len);
                    venc_header_size_ = venc_stream.pack[i].len;
                    kd_mpi_sys_munmap(pData, venc_stream.pack[i].len);
                    get_header = true;
                    continue;
                }
                k_u8 *frame_data = nullptr;
                k_u32 frame_size;
                if (venc_stream.pack[i].type == K_VENC_I_FRAME && get_header) {
                    frame_data = new k_u8[venc_stream.pack[i].len + venc_header_size_];
                    memcpy(frame_data, venc_header_, venc_header_size_);
                    memcpy(frame_data + venc_header_size_, pData, venc_stream.pack[i].len);
                    frame_size = venc_stream.pack[i].len + venc_header_size_;
                    get_header = false;
                    is_idr = true;
                } else {
                    frame_data = new k_u8[venc_stream.pack[i].len];
                    memcpy(frame_data, pData, venc_stream.pack[i].len);
                    frame_size = venc_stream.pack[i].len;
                    is_idr = false;
                }
                kd_mpi_sys_munmap(pData, venc_stream.pack[i].len);
                {
                    AVEncFrameData vframe;
                    vframe.data = frame_data;
                    vframe.size = frame_size;
                    vframe.type = AVEncFrameData::Type::H265;
                    vframe.timestamp_ms = venc_stream.pack[i].pts / 1000;
                    vframe.keyframe = is_idr;
                    vframe.sequence = venc_sequence++;
                    vframe.flags = client_ready_;
                    if (venc_stream_queue_.put(vframe) == false) {
                        delete []frame_data;
                    }
                }
            }
            media_->VencChnReleaseStream(venc_chn, &venc_stream);
            delete[] venc_stream.pack;
        }
        venc_senc_exit_.store(true);
        printf("StartVencThread stop...\n");
    });
}

void MyApp::StartVencSnapThread() {
    doorbell_snap_thread_ = std::thread([this]() {
        int venc_snap_chn = 0;
        while (!doorbell_venc_exit_flag_) {
            VencChnStatus status;
            k_venc_stream venc_stream;
            memset(&venc_stream, 0, sizeof(venc_stream));

            media_->VencChnQueryStatus(venc_snap_chn, &status);
            if (status.cur_packs > 0) {
                venc_stream.pack_cnt = status.cur_packs;
            } else {
                venc_stream.pack_cnt = 1;
            }

            venc_stream.pack = new k_venc_pack[venc_stream.pack_cnt];
            k_s32 ret = media_->VencChnGetStream(venc_snap_chn, &venc_stream);
            if (ret != K_SUCCESS) {
                delete[] venc_stream.pack;
                continue;
            }

            if (doorbell_first_snap_) {
                for (int i = 0; i < venc_stream.pack_cnt; i++) {
                    k_u8 *pData;
                    pData = (k_u8 *)kd_mpi_sys_mmap(venc_stream.pack[i].phys_addr, venc_stream.pack[i].len);

                    k_u8 *data_temp = new k_u8[venc_stream.pack[i].len];
                    memcpy(data_temp, pData, venc_stream.pack[i].len);
                    printf("doorbell_first_snap first data len = %d...\n", venc_stream.pack[i].len);
                    UserEventData user_event;
                    user_event.type = UserEventData::EventType::KEY_WAKEUP;
                    user_event.jpeg = data_temp;
                    user_event.jpeg_size = venc_stream.pack[i].len;
                    user_event.timestamp_ms = venc_stream.pack[i].pts;

                    kd_mpi_sys_munmap(pData, venc_stream.pack[i].len);

                    doorbell_event_vec_.push_back(user_event);
                }

                doorbell_first_snap_.store(false);
            }

            media_->VencChnReleaseStream(venc_snap_chn, &venc_stream);

            delete[] venc_stream.pack;

        }
        printf("StartVencSnapThread stop...\n");
    });
}

void MyApp::StartPlayAudioThread() {
    play_audio_thread_ = std::thread([this]() {
        std::fstream audio_file;
        k_u32 music_data_size_ {0};
        k_u8 *music_data_ {nullptr};
        audio_file.open("./doorbell.g711u", std::ios::in | std::ios::binary);
        if (audio_file.is_open()) {
            audio_file.seekg(0, std::ios::end);
            music_data_size_ = audio_file.tellg();
            audio_file.seekg(0, std::ios::beg);
            music_data_ = new k_u8[music_data_size_];
            audio_file.read((char *)music_data_, music_data_size_);
            audio_file.close();
        } else {
            music_data_ = nullptr;
            music_data_size_ = 0;
            std::cout << "failed to open doorbell.g711u" << std::endl;
        }

        int audio_sample_rate = 8000;
        int audio_persec_dir_sum = 25;
        int stream_len = audio_sample_rate * 2 / audio_persec_dir_sum / 2;
        int cur_data_index = 0;
        int play_audio_num = 0;
        k_u64 pts = 0;

        while (!doorbell_palyaudio_exit_flag_) {
            if (key_pressed_ && music_data_ && music_data_size_ >= 320) {
                if (cur_data_index + stream_len > music_data_size_) {
                    cur_data_index = 0;
                    play_audio_num++;
                }

                if (play_audio_num >= 5) {
                    printf("play audio stop...\n");
                    key_pressed_.store(false);
                    pts = 0;
                    cur_data_index = 0;
                    play_audio_num = 0;
                    continue;
                }

                media_->ADecSendData((const k_u8*)(music_data_ + cur_data_index), stream_len, pts, false);
                pts += 40;

                cur_data_index += stream_len;
            }

            AVEncFrameData aframe;
            if (comm_server_->GetAudioFrame(aframe)) {
                media_->ADecSendData((const k_u8*)(aframe.data), aframe.size, aframe.timestamp_ms, true);
                delete[] aframe.data;
            } else if (!key_pressed_) {
                usleep(3000);
            }
        }
        AVEncFrameData aframe;
        while (comm_server_->GetAudioFrame(aframe)) {
            delete[] aframe.data;
            usleep(10000);
        }

        delete []music_data_;
        printf("StartPlayAudioThread stop...\n");
        return;
    });
}

void MyApp::ReceiveMessageThread() {
    receive_message_thread_ = std::thread([this]() {
        while (!recevie_msg_exit_flag_) {
            if (comm_server_->ClientReady()) {
                client_ready_.store(true);
                exit_msg_.store(false);
            }

            if (comm_server_->GetKeyPressed() && !key_pressed_) {
                key_pressed_.store(true);
            }

            if (comm_server_->GetExitMsg()) {
                exit_msg_.store(true);
                recevie_msg_exit_flag_.store(true);
                playback_.store(false);
                break;
            }

            if (comm_server_->GetPlayback()) {
                exit_msg_.store(true);
                playback_.store(true);
            }

            if (current_mode_ == PIR_MODE && playback_) {
                mode_switch_ = PLAYBACK_MODE;
                ai_exit_flag_.store(true);
                venc_exit_flag_.store(true);
                send_event_exit_flag_.store(true);
                recevie_msg_exit_flag_.store(true);
                current_mode_ = PLAYBACK_MODE;
            }

            if (current_mode_ == DOORBELL_MODE && playback_) {
                mode_switch_ = PLAYBACK_MODE;
                doorbell_venc_exit_flag_.store(true);
                doorbell_aenc_exit_flag_.store(true);
                doorbell_palyaudio_exit_flag_.store(true);
                recevie_msg_exit_flag_.store(true);
                current_mode_ = PLAYBACK_MODE;
            }

            if (current_mode_ == PIR_MODE && key_pressed_) {
                printf("key pressed...\n");
                mode_switch_ = DOORBELL_MODE;
                ai_exit_flag_.store(true);
                venc_exit_flag_.store(true);
                send_event_exit_flag_.store(true);
                current_mode_ = DOORBELL_MODE;
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(20));
        }

        if (current_mode_ == PIR_MODE) {
            ai_exit_flag_.store(true);
            venc_exit_flag_.store(true);
        } else if (current_mode_ == DOORBELL_MODE) {
            doorbell_venc_exit_flag_.store(true);
            doorbell_aenc_exit_flag_.store(true);
            doorbell_palyaudio_exit_flag_.store(true);
        }
    });

    return;
}

TrigerMode MyApp::GetCurrentMode() {
    int mem_fd = open("/dev/mem", O_RDWR | O_SYNC);
    if (mem_fd < 0)
    {
        std::cout << "open /dev/mem fail." << std::endl;
        return BOTTOM_MODE;
    }

    void *pmu = mmap(NULL, 0x1000, PROT_READ | PROT_WRITE, MAP_SHARED, mem_fd, 0x91103000);
    if (pmu == NULL)
    {
        std::cout << "mmap fail." << std::endl;
        return BOTTOM_MODE;
    }

    *(volatile unsigned int *)(pmu+0x158) = 0;

    void *pmu1 = mmap(NULL, 0x1000, PROT_READ | PROT_WRITE, MAP_SHARED, mem_fd, 0x91000000);
    if (pmu1 == NULL)
    {
        std::cout << "mmap fail." << std::endl;
        return BOTTOM_MODE;
    }

    unsigned int reg = *(volatile unsigned int *)(pmu1 + 0xac);

    int reg12 = (reg >> 11) & 0x01;
    int reg5 = (reg >> 4) & 0x01;

    if (reg5) {  // kai ji jian
        current_mode_ = PIR_MODE;
    } else if (reg12) {  // chang an jian
        current_mode_ = DOORBELL_MODE;
    }

    return current_mode_; // pir mode
}
