#include <iostream>
#include <atomic>
#include <chrono>
#include <unistd.h>
#include <signal.h>
#include <thread>
#include <mutex>
#include <string.h>
#include <stdio.h>
#include "media.h"
#include "mp4_format.h"

using namespace std::chrono_literals;

std::atomic<bool> g_exit_flag{false};
static char g_mp4_pathname[256];

static void sigHandler(int sig_no)
{
    g_exit_flag.store(true);
    printf("exit_flag true\n");
}

class MyMp4muxer : public IOnAEncData, public IOnVEncData
{
public:
    MyMp4muxer() {}

    // IOnAEncData
    virtual void OnAEncData(k_u32 chn_id, k_u8*pdata,size_t size,k_u64 time_stamp) override
    {
        std::unique_lock<std::mutex> lck(frame_muxer_mutex_);
        if (first_frame_time_stamp_ == 0)
        {
            return ;
        }
        if (started_)
        {
            k_mp4_frame_data_s frame_data;
            memset(&frame_data, 0, sizeof(frame_data));
            frame_data.codec_id = K_MP4_CODEC_ID_G711U;
            frame_data.data = pdata;
            frame_data.data_length = size;
            frame_data.time_stamp = time_stamp - first_frame_time_stamp_;
            if (frame_data.time_stamp < 0)
            {
                printf("audio frame_data timestamp <0,return\n");
                return ;
            }
            k_u32 ret = kd_mp4_write_frame(mp4_muxer_, audio_track_handle_, &frame_data);
            if (ret < 0)
            {
                printf("mp4 write audio frame failed.\n");
                return;
            }

        }
    }

    // IOnVEncData
    virtual void OnVEncData(k_u32 chn_id, void *data, size_t size, k_venc_pack_type type, uint64_t first_time_stamp) override
    {
        std::unique_lock<std::mutex> lck(frame_muxer_mutex_);
        if (first_frame_time_stamp_ == 0)
        {
            first_frame_time_stamp_ = first_time_stamp;
        }
        if (started_)
        {
        if (video_stream_count_ % 30 == 0)
            printf("recv count:%d, chn:%d, len:%d\n", video_stream_count_, 0, size);

        k_mp4_frame_data_s frame_data;
        memset(&frame_data, 0, sizeof(frame_data));
        frame_data.codec_id = _get_mp4_codec_type(config_.video_type);
        frame_data.data =  (uint8_t *)data;
        frame_data.data_length = size;
        frame_data.time_stamp = first_time_stamp - first_frame_time_stamp_;

        if (!get_idr_) {
            memcpy(save_idr_ + save_size_, frame_data.data, frame_data.data_length);
            save_size_ += frame_data.data_length;
            first_time_stamp_ = first_time_stamp - first_frame_time_stamp_;
        }

        video_stream_count_++;
        if (video_stream_count_ == 2) {
            get_idr_ = 1;
            memset(&frame_data, 0, sizeof(frame_data));
            frame_data.codec_id = _get_mp4_codec_type(config_.video_type);
            frame_data.data =  save_idr_;
            frame_data.data_length = save_size_;
            frame_data.time_stamp = first_time_stamp_;
        }

        if (get_idr_) {
            k_u32 ret = kd_mp4_write_frame(mp4_muxer_, video_track_handle_, &frame_data);
            if (ret < 0) {
                printf("mp4 write video frame failed.\n");
                return ;
            }
        }

        }
    }

    int Init(const KdMediaInputConfig &config)
    {
        mp4_muxer_init(config);
        if (media_.Init(config) < 0)
            return -1;
        if (media_.CreateAiAEnc(this) < 0)
            return -1;
        if (media_.CreateADecAo() < 0)
            return -1;
        if (config.video_valid && media_.CreateVcapVEnc(this) < 0)
            return -1;

        return 0;
    }
    int DeInit()
    {
        Stop();
        media_.DestroyVcapVEnc();
        media_.DestroyADecAo();
        media_.DestroyAiAEnc();
        media_.Deinit();
        mp4_muxer_deinit();
        return 0;
    }

    int Start()
    {
        if (started_)
            return 0;
        media_.StartADecAo();
        media_.StartAiAEnc();
        media_.StartVcapVEnc();
        started_ = true;
        return 0;
    }
    int Stop()
    {
        if (!started_)
            return 0;
        started_ = false;
        media_.StopVcapVEnc();
        media_.StopADecAo();
        media_.StopAiAEnc();
        return 0;
    }

protected:
    int mp4_muxer_init(const KdMediaInputConfig &config)
    {
        printf("mp4 config audio channel:%d,samplerate:%d\n",config.audio_channel_cnt,config.audio_samplerate);

        config_ = config;
        k_s32 ret;
        k_mp4_config_s mp4_config;
        memset(&mp4_config, 0, sizeof(mp4_config));
        mp4_config.config_type = K_MP4_CONFIG_MUXER;
        //strcpy(mp4_config.muxer_config.file_name, "/sharefs/test.mp4");
        strcpy(mp4_config.muxer_config.file_name,g_mp4_pathname);
        mp4_config.muxer_config.fmp4_flag = 0;

        ret = kd_mp4_create(&mp4_muxer_, &mp4_config);
        if (ret < 0)
        {
            printf("mp4 muxer create failed.\n");
            return -1;
        }

        // create video track
        k_mp4_track_info_s video_track_info;
        memset(&video_track_info, 0, sizeof(video_track_info));
        video_track_info.track_type = K_MP4_STREAM_VIDEO;
        video_track_info.time_scale = 1000;
        video_track_info.video_info.width = config.venc_width;
        video_track_info.video_info.height = config.venc_height;
        ret = kd_mp4_create_track(mp4_muxer_, &video_track_handle_, &video_track_info);
        if (ret < 0)
        {
            printf("create video track failed.\n");
            return -1;
        }

        // create audio track
        k_mp4_track_info_s audio_track_info;
        memset(&audio_track_info, 0, sizeof(audio_track_info));
        audio_track_info.track_type = K_MP4_STREAM_AUDIO;
        audio_track_info.time_scale = 1000;
        audio_track_info.audio_info.channels = config.audio_channel_cnt;
        audio_track_info.audio_info.codec_id = K_MP4_CODEC_ID_G711U;
        audio_track_info.audio_info.sample_rate = config.audio_samplerate;
        audio_track_info.audio_info.bit_per_sample = 16;
        ret = kd_mp4_create_track(mp4_muxer_, &audio_track_handle_, &audio_track_info);
        if (ret < 0)
        {
            printf("create video track failed.\n");
            return -1;
        }
        first_save_idr_frame_ = false;
        video_header_len_ = 0;

        return 0;
    }

    int mp4_muxer_deinit()
    {
        k_s32 ret;
        ret = kd_mp4_destroy_tracks(mp4_muxer_);
        if (ret < 0)
        {
            printf("destroy mp4 tracks failed.\n");
            return -1;
        }

        ret = kd_mp4_destroy(mp4_muxer_);
        if (ret < 0)
        {
            printf("destroy mp4 failed.\n");
            return -1;
        }

        return 0;
    }

    k_mp4_codec_id_e _get_mp4_codec_type(KdMediaVideoType video_type)
    {
        if (video_type == KdMediaVideoType::kVideoTypeH265)
        {
            return K_MP4_CODEC_ID_H265;
        }
        else if (video_type == KdMediaVideoType::kVideoTypeH264)
        {
            return K_MP4_CODEC_ID_H264;
        }

        return K_MP4_CODEC_ID_H265;
    }

private:
    KD_HANDLE mp4_muxer_{NULL};
    KD_HANDLE video_track_handle_{NULL};
    KD_HANDLE audio_track_handle_{NULL};
    KdMedia media_;
    std::string stream_url_;
    std::atomic<bool> started_{false};
    std::atomic<bool> first_save_idr_frame_{false};
    KdMediaInputConfig config_;
    uint8_t save_idr_[1920 * 1080 * 3 / 2];
    uint32_t video_header_len_{0};

    uint32_t video_stream_count_{0};
    bool  get_idr_{false};
    uint32_t save_size_{0};
    uint64_t first_time_stamp_{0};

    std::mutex frame_muxer_mutex_;

    std::atomic<uint64_t> first_frame_time_stamp_{0};
    uint64_t video_first_time_stamp_{0};
};

static void show_usage(char* pname)
{
    printf("Usage: ./%s -o *.mp4  -s 7\n",pname);
    printf("-s: the sensor type: default 7\n");
    printf("     see camera sensor doc.\n");
    printf("-o: save mp4 path file name\n");
}

int main(int argc, char *argv[])
{
    signal(SIGINT, sigHandler);

    KdMediaInputConfig config = {
        .video_valid = true,
        .sensor_type = IMX335_MIPI_2LANE_RAW12_1920X1080_30FPS_LINEAR,
        .sensor_num = 1,
        .video_type = KdMediaVideoType::kVideoTypeH265,
        .venc_width = 1280,
        .venc_height = 720,
        .bitrate_kbps = 4000,

        .audio_samplerate = 44100,
        .audio_channel_cnt = 2,
        .pitch_shift_semitones = 0};

    if (argc > 1)
    {
        for (int i = 1; i < argc; i += 2)
        {
            if (strcmp(argv[i], "-s") == 0)
            {
                config.sensor_type = (k_vicap_sensor_type)atoi(argv[i + 1]);
            }
            if (strcmp(argv[i], "-o") == 0)
            {
                memset(g_mp4_pathname,0,sizeof(g_mp4_pathname));
                strncpy(g_mp4_pathname,argv[i + 1],sizeof(g_mp4_pathname));
            }
            else if (strcmp(argv[i], "-h") == 0)
            {
                show_usage(argv[0]);
                return 0;
            }
        }
    }
    else
    {
        show_usage(argv[0]);
        return 0;
    }

    printf("mp4 muxer...\n");

    MyMp4muxer *mp4muxer = new MyMp4muxer();
    if (!mp4muxer || mp4muxer->Init(config) < 0)
    {
        std::cout << "mp4muxer Init failed." << std::endl;
        return -1;
    }
    mp4muxer->Start();

    while (!g_exit_flag)
    {
        std::this_thread::sleep_for(100ms);
    }
    mp4muxer->Stop();
    mp4muxer->DeInit();
    delete mp4muxer;
    return 0;
}
