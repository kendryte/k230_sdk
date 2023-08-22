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

#include "msg_proc.h"
#include "ui_common.h"
#include <iostream>
#include <mutex>
#include <queue>
#include <semaphore.h>
#include <sys/fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <iostream>
#include <atomic>
#include <chrono>
#include <pthread.h>
#include "k_ipcmsg.h"
#include "mapi_vo_api.h"
#include "rtsp_client.h"
#include "media.h"
#include "g711_mix_audio.h"
#include "intercom.h"

using namespace std::chrono_literals;

#define WRITE_AUDIO_MIX (0)
#define MP4_MUX         (1)
#define SAVE_FILE_NAME "/nfs/mix.g711u"

std::atomic<bool> g_exit_flag{false};
pthread_t tid_thread_save = NULL;
static std::mutex mix_mutex_;

static bool enable_save = true;

class MyRtspClient : public IOnBackChannel, public IOnAudioData, public IOnVideoData, public IOnAEncData, public IRtspClientEvent
{
public:
    MyRtspClient() {}

    // IOnBackChannel
    virtual void OnBackChannelStart() override
    {
        std::cout << "OnBackChannelStart called" << std::endl;
        media_.StartAiAEnc();
    }
    // // IOnAEncData
    // virtual void OnAEncData(k_u32 chn_id, k_audio_stream* stream_data) override {
    //     // std::cout << "OnAEncData -- size  = " << stream_data->len << ", timestamp = " << stream_data->time_stamp << std::endl;
    //     rtsp_client_.SendAudioData((const uint8_t*)stream_data->stream, stream_data->len, stream_data->time_stamp);
    // }
    // IOnAudioData, received from the server
    virtual void OnAudioStart() override
    {
        std::cout << "OnAudioStart called" << std::endl;
        media_.StartADecAo();
    }

    // IOnAudioData, received from the server
    virtual void OnAudioData(const uint8_t *data, size_t size, uint64_t timestamp) override
    {
        // printf("%s>size %d, timestamp %d\n", __FUNCTION__, size, timestamp);
        if (started_)
        {
            // TODO， to handle jitter and control accumulation
            //   gather data to get complete frame(40ms).
            if (audio_data_size == 0)
            {
                audio_timestamp = timestamp;
            }
            for (size_t i = 0; i < size ; i++)
            {
                g711_buffer_[audio_data_size++] = data[i];
                if (audio_data_size == 320)
                {
                    if (enable_save_ && get_idr_frame_)
                    {
                        mix_mutex_.lock();
                        if (!(mixed_[!mix_pingpong_]))
                        {
                            mix_pingpong_ = !mix_pingpong_;
                            kd_mix_g711u_audio((k_char *)g711_buffer_, (k_char *)mix_src_buffer_[mix_pingpong_], audio_data_size, (k_char *)mix_dst_buffer_);
                            mixed_[mix_pingpong_] = true;
                        }
                        else
                        {
                            kd_mix_g711u_audio((k_char *)g711_buffer_, NULL, audio_data_size, (k_char *)mix_dst_buffer_);
                        }

                        mix_mutex_.unlock();
#if MP4_MUX
                        uint64_t tmp = audio_timestamp; //audio_timestamp+(i/320)*40
                        // printf("MuxAudio timestamp %d\n", tmp);
                        media_.MuxAudioData(mix_dst_buffer_, audio_data_size, tmp);
#endif
#if WRITE_AUDIO_MIX
                        if (!fp_mix_)
                            fp_mix_ = fopen(SAVE_FILE_NAME, "ab");
                        fwrite(mix_dst_buffer_, 1, audio_data_size, fp_mix_);
#endif
                    }
                    // printf("%s>SendAudioData\n", __FUNCTION__);
                    media_.SendAudioData(g711_buffer_, audio_data_size, audio_timestamp);
                    audio_data_size = 0;
                    audio_timestamp = timestamp;
                }
            }
        }
    }

    // IOnVideoData, received from the server
    virtual void OnVideoType(IOnVideoData::VideoType type,  uint8_t *extra_data, size_t extra_data_size)
    {
        video_type_ = type;
        VdecInitParams params;
        if (type == IOnVideoData::VideoTypeH264)
        {
            std::cout << "OnVideoType() called, type H264" << std::endl;
            params.type = KdMediaVideoType::TypeH264;
        }
        if (type == IOnVideoData::VideoTypeH265)
        {
            std::cout << "OnVideoType() called, type H265" << std::endl;
            params.type = KdMediaVideoType::TypeH265;
        }
        media_.CreateVdecVo(params);
        media_.StartVDecVo();
        first_video_frame_ = true;
        get_idr_frame_ = false;
    }
    virtual void OnVideoData(const uint8_t *data, size_t size, uint64_t timestamp, bool keyframe) override
    {
        // printf("%s>size %d, timestamp %d\n", __FUNCTION__, size, timestamp);
        // TODO, to handle jitter and control accumulation
        if (started_)
        {
            // if (video_type_ == IOnVideoData::VideoTypeH264)
            //    std::cout << "OnVideoData() called, type H264, size = " << size << ", timestamp = " << timestamp << std::endl;
            // if (video_type_ == IOnVideoData::VideoTypeH265)
            //    std::cout << "OnVideoData() called, type H265, size = " << size << ", timestamp = " << timestamp << ", keyframe = " << keyframe << std::endl;

            // TODO
            if (first_video_frame_ && !keyframe) return;
            first_video_frame_ = false;
            if (keyframe && !get_idr_frame_)
            {
                get_idr_frame_ = true;
                printf("%s>get_idr_frame_ %d\n", __FUNCTION__, get_idr_frame_);
            }

            // static int test = true;
            // if (!test)  return;
            /*
                        FILE *fp = fopen("video.data", "ab");
                        if (fp) {
                            fwrite(data, 1, size, fp);
                            fclose(fp);
                            printf("saving video data, size = %d\n", size);
                        }
            */
            if (enable_save_ && get_idr_frame_)
            {
#if MP4_MUX
                // printf("MuxVideo timestamp %d\n", timestamp);
                media_.MuxVideoData(data, size, timestamp);
#endif
            }
            // printf("%s>SendVideoData\n", __FUNCTION__);
            media_.SendVideoData(data, size, timestamp);

            // test = false;
        }
    }

    // IOnAEncData
    virtual void OnAEncData(k_u32 chn_id, k_audio_stream *stream_data) override
    {
        // printf("%s>stream_data->len %d, stream_data->time_stamp %d\n", __FUNCTION__, stream_data->len, stream_data->time_stamp);
        tmp_timestamp_ = stream_data->time_stamp;
        if (started_)
        {
            if (enable_save_)
            {
                mix_mutex_.lock();
                if (mix_pingpong_)
                {
                    memcpy(mix_src_buffer_[0], stream_data->stream, stream_data->len);
                    mixed_[0] = false;
                    local_audio_timestamp[0] = stream_data->time_stamp;
                }
                else
                {
                    memcpy(mix_src_buffer_[1], stream_data->stream, stream_data->len);
                    mixed_[1] = false;
                    local_audio_timestamp[1] = stream_data->time_stamp;
                }
                mix_mutex_.unlock();
            }
            // printf("%s>SendAudioData\n", __FUNCTION__);
            rtsp_client_.SendAudioData((const uint8_t *)stream_data->stream, stream_data->len, stream_data->time_stamp);
        }
    }

    // IRtspClientEvent
    virtual void OnRtspClientEvent(int event) override
    {
        // At the moment, there is only one event : shutdown
        printf("OnRtspClientEvent -- shutdown\n");
        g_exit_flag.store(true);
    }

    int Init()
    {
        RtspClientInitParam param;
        param.on_video_data = this;
        param.on_audio_data = this;
        param.on_backchannel = this;
        param.on_event = this;
        if (rtsp_client_.Init(param) < 0) return -1;
        if (media_.Init() < 0) return -1;
        if (media_.CreateAiAEnc(this) < 0) return -1;
        media_.EnablePitchShift();
        if (media_.CreateADecAo() < 0) return -1;
#if MP4_MUX
        media_.CreateMux();
#endif
#if WRITE_AUDIO_MIX
        fp_mix_ = fopen(SAVE_FILE_NAME, "wb");
        if (!fp_mix_)
            printf("open file %s failed\n", SAVE_FILE_NAME);
#endif
        inited_ = true;
        return 0;
    }
    int DeInit()
    {
        Stop();
        if (!inited_) return 0;
        media_.DestroyAiAEnc();
        media_.DestroyADecAo();
        media_.DestroyVDecVo();
        media_.Deinit();
        rtsp_client_.DeInit();
#if MP4_MUX
        StopMux();
        media_.DestroyMux();
#endif
#if WRITE_AUDIO_MIX
        fclose(fp_mix_);
#endif
        inited_ = false;
        return 0;
    }

    int RecoverVdec()
    {
        VdecInitParams params;
        if (video_type_ == IOnVideoData::VideoTypeH264)
        {
            std::cout << "RecoverVdec() called, type H264" << std::endl;
            params.type = KdMediaVideoType::TypeH264;
        }
        if (video_type_ == IOnVideoData::VideoTypeH265)
        {
            std::cout << "RecoverVdec() called, type H265" << std::endl;
            params.type = KdMediaVideoType::TypeH265;
        }
        media_.CreateVdecVo(params);
        media_.StartVDecVo();
        first_video_frame_ = true;
        get_idr_frame_ = false;
        return 0;
    }

    int Start(const char *url)
    {
        if (started_) return 0;
        // media_.StartADecAo();
        rtsp_client_.Open(url);
        // media_.StartAiAEnc();
        started_ = true;
        return 0;
    }
    int Stop()
    {
        if (!started_) return 0;
        started_ = false;
        rtsp_client_.Close();
        media_.StopAiAEnc();
        media_.StopADecAo();
        media_.StopVDecVo();
        return 0;
    }
    int Suspend()
    {
        if (!started_) return 0;
        started_ = false;
        media_.StopAiAEnc();
        media_.StopADecAo();
        media_.StopVDecVo();
        media_.DestroyVDecVo();
        get_idr_frame_ = false;
        first_video_frame_ = true;
        return 0;
    }
    int Recover()
    {
        if (started_) return 0;
        media_.StartADecAo();
        media_.StartAiAEnc();
        RecoverVdec();
        started_ = true;
        return 0;
    }

    int EnablePitchShift()
    {
        if (!started_) return 0;
        media_.EnablePitchShift();
        return 0;
    }

    int DisablePitchShift()
    {
        if (!started_) return 0;
        media_.DisablePitchShift();
        return 0;
    }

    int CreateMux()
    {
        if (!started_) return 0;
        media_.CreateMux();
    }

    int DetoryMux()
    {
        if (!started_) return 0;
        media_.DestroyMux();
    }

    int StartMux()
    {
        if (!started_) return 0;
        enable_save_ = true;
        get_idr_frame_ = false;
    }

    int StopMux()
    {
        if (!started_) return 0;
        enable_save_ = false;
    }

    int SwitchMux()
    {
        if (!started_) return 0;
        if (enable_save_)
        {
            printf("Stop save\n");
            StopMux();
        }
        else
        {
            printf("StartMux\n");
            StartMux();
        }
        printf("%s done\n", __FUNCTION__);
    }

private:
    KdRtspClient rtsp_client_;
    KdMedia media_;
    std::atomic<bool> started_{false};
    std::atomic<bool> inited_{false};
    uint8_t g711_buffer_[320] = {0};
    uint8_t mix_src_buffer_[2][320] = {0};
    uint8_t mix_dst_buffer_[320] = {0};
    volatile bool mixed_[2] = {false};
    volatile bool mix_pingpong_ = true;
    size_t audio_data_size = 0;
    uint64_t audio_timestamp;
    uint64_t tmp_timestamp_;
    uint64_t local_audio_timestamp[2] = {0};
    IOnVideoData::VideoType video_type_{IOnVideoData::VideoTypeInvalid};
    bool first_video_frame_{true};
    bool enable_save_ = true;
    bool get_idr_frame_{false};
#if WRITE_AUDIO_MIX
    FILE *fp_mix_ = NULL;
#endif
};

MyRtspClient *client = NULL;

#ifdef __cplusplus
extern "C" {
#endif

k_s32 intercom_init(char *url)
{
    printf("%s>url %s\n", __FUNCTION__, url);
    client = new MyRtspClient();
    if (!client || client->Init() < 0)
    {
        std::cout << "KdRtspClient Init failed." << std::endl;
        return -1;
    }
    client->Start(url);
    return 0;
}

k_s32 intercom_start(char *url)
{
    client->Start(url);
    return 0;
}

k_s32 intercom_deinit()
{
    if (client)
    {
        client->DeInit();
        delete client;
    }
    return 0;
}

k_s32 intercom_suspend()
{
    client->Suspend();
    // client->Stop();
    return 0;
}

k_s32 intercom_recover(char *url)
{
    client->Recover();
    // client->Start(url);
    return 0;
}

k_s32 intercom_enable_pitch_shift()
{
    client->EnablePitchShift();
    return 0;
}

k_s32 intercom_disable_pitch_shift()
{
    client->DisablePitchShift();
    return 0;
}

k_s32 intercom_save_control()
{
    printf("%s\n", __FUNCTION__);
    if (enable_save)
    {
        client->StopMux();
        enable_save = false;
    }
    else
    {
        client->StartMux();
        enable_save = true;
    }
    // client->SwitchMux();
    return 0;
}

#ifdef __cplusplus
}
#endif
