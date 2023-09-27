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
#include <mutex>
#include <queue>
#include <unistd.h>
#include <atomic>
#include <chrono>
#include <cstring>
#include "my_app_impl.h"

using namespace std::chrono_literals;

// IRtspClientEvent
void PeepholeApp::OnRtspClientEvent(int event)
{
    // At the moment, there is only one event : shutdown
    printf("OnRtspClientEvent -- shutdown\n");
    rtsp_disconnected_ = true;
}

// IOnBackChannel
void PeepholeApp::OnBackChannelStart()
{
    std::cout << "OnBackChannelStart called" << std::endl;
    media_.StartAiAEnc();
}
// IOnAudioData, received from the server
void PeepholeApp::OnAudioStart()
{
    std::cout << "OnAudioStart called" << std::endl;
    media_.StartADecAo();
}

// IOnAudioData, received from the server
void PeepholeApp::OnAudioData(const uint8_t *data, size_t size, uint64_t timestamp)
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
                media_.SendAudioData(g711_buffer_, audio_data_size, audio_timestamp);
                audio_data_size = 0;
                audio_timestamp = timestamp;
            }
        }
    }
}

// IOnAEncData
void PeepholeApp::OnAEncData(k_u32 chn_id, k_audio_stream *stream_data)
{
    // printf("%s>stream_data->len %d, stream_data->time_stamp %d\n", __FUNCTION__, stream_data->len, stream_data->time_stamp);
    tmp_timestamp_ = stream_data->time_stamp;
    if (started_)
    {
        rtsp_client_.SendAudioData((const uint8_t *)stream_data->stream, stream_data->len, stream_data->time_stamp);
    }
}

 // IOnVideoData, received from the server
 void PeepholeApp::OnVideoType(IOnVideoData::VideoType type,  uint8_t *extra_data, size_t extra_data_size)
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

void PeepholeApp::OnVideoData(const uint8_t *data, size_t size, uint64_t timestamp, bool keyframe)
{
    // TODO, to handle jitter and control accumulation
    if (started_)
    {
        if (first_video_frame_ && !keyframe) return;
        first_video_frame_ = false;

        // printf("%s>SendVideoData\n", __FUNCTION__);
        media_.SendVideoData(data, size, timestamp);
    }
}


int PeepholeApp::Init(int flag)
{
    RtspClientInitParam param;
    param.on_video_data = this;
    param.on_audio_data = this;
    param.on_backchannel = this;
    param.on_event = this;
    if (rtsp_client_.Init(param) < 0) return -1;
    if (!flag && media_.Init() < 0) return -1;
    if (media_.CreateAiAEnc(this) < 0) return -1;
    // media_.EnablePitchShift();
    if (media_.CreateADecAo() < 0) return -1;
    inited_ = true;
    return 0;
}
void PeepholeApp::DeInit(int flag)
{
    Stop();
    if (!inited_) return;
    media_.DestroyAiAEnc();
    media_.DestroyADecAo();
    media_.DestroyVDecVo();
    if(!flag) media_.Deinit();
    rtsp_client_.DeInit();
    inited_ = false;    
}

int PeepholeApp::Start(const char *url)
{
    if (started_) return 0;
    // media_.StartADecAo();
    rtsp_url_ = url;
    rtsp_client_.Open(url);
    // media_.StartAiAEnc();
    started_ = true;
    return 0;
}

int PeepholeApp::Stop()
{
    if (!started_) return 0;
    started_ = false;
    rtsp_client_.Close();
    media_.StopAiAEnc();
    media_.StopADecAo();
    media_.StopVDecVo();
    media_.DestroyVDecVo();
    return 0;
}

int PeepholeApp::Suspend()
{
    DeInit(1);
    return 0;
}

int PeepholeApp::Resume()
{
    Init(1);
    Start(rtsp_url_.c_str());
    return 0;
}

int PeepholeApp::EnablePitchShift()
{
    if (!started_) return -1;
    return media_.EnablePitchShift();
}

int PeepholeApp::DisablePitchShift()
{
    if (!started_) return -1;
    return media_.DisablePitchShift();
}
