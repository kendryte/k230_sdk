#include <stdio.h>
#include <sys/mman.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <pthread.h>
#include <stdlib.h>
#include <getopt.h>
#include <signal.h>
#include <pthread.h>
#include <semaphore.h>

#include "kplayer.h"
#include "player_res.h"
#include "k_type.h"
#include "mp4_format.h"

typedef struct TAG_PLAYER_VIDEO_INFO
{
    k_u32 track_id;
    k_payload_type type;
    k_u32 width;
    k_u32 height;

} PLAYER_VIDEO_INFO;

typedef struct TAG_PLAYER_AUDIO_INFO
{
    k_u32 track_id;
    k_payload_type type;
    k_u32 samplerate;
    k_u32 channel_num;
} PLAYER_AUDIO_INFO;
#define INVALID_STREAM_TRACK -1

static K_PLAYER_EVENT_FN g_pfnCallback = NULL;
static void *g_data_context = NULL;
static void *g_mp4_demuxer_handle = NULL;
static k_char g_mp4_filename[256];
static PLAYER_VIDEO_INFO g_player_video_info;
static PLAYER_AUDIO_INFO g_player_audio_info;
static k_s32 g_video_track = INVALID_STREAM_TRACK;
static k_s32 g_audio_track = INVALID_STREAM_TRACK;
static k_bool g_play_start = K_FALSE;
static k_bool g_play_pause = K_FALSE;
static pthread_t g_play_tid = 0;
static K_PLAYER_PROGRESS_INFO g_progress_info;
static int g_connector_type = 0;

static k_payload_type _get_payload_type(k_mp4_codec_id_e codec_id)
{
    if (codec_id == K_MP4_CODEC_ID_H264)
    {
        return K_PT_H264;
    }
    else if (codec_id == K_MP4_CODEC_ID_H265)
    {
        return K_PT_H265;
    }
    else if (codec_id == K_MP4_CODEC_ID_G711A)
    {
        return K_PT_G711A;
    }
    else if (codec_id == K_MP4_CODEC_ID_G711U)
    {
        return K_PT_G711U;
    }

    return K_PT_BUTT;
}

static k_s32 _init_mp4_demuxer(const k_char *filePath)
{
    memset(&g_player_video_info, 0, sizeof(g_player_video_info));
    memset(&g_player_audio_info, 0, sizeof(g_player_audio_info));
    g_video_track = INVALID_STREAM_TRACK;
    g_audio_track = INVALID_STREAM_TRACK;

    k_mp4_config_s mp4_config;
    memset(&mp4_config, 0, sizeof(mp4_config));
    mp4_config.config_type = K_MP4_CONFIG_DEMUXER;
    strcpy(mp4_config.demuxer_config.file_name, filePath);
    mp4_config.muxer_config.fmp4_flag = 0;

    k_s32 ret = kd_mp4_create(&g_mp4_demuxer_handle, &mp4_config);
    if (ret < 0)
    {
        printf("mp4 muxer create failed.\n");
        return -1;
    }

    k_mp4_file_info_s file_info;
    memset(&file_info, 0, sizeof(file_info));
    ret = kd_mp4_get_file_info(g_mp4_demuxer_handle, &file_info);
    if (ret < 0)
    {
        printf("k_mp4_get_file_info: get file info failed.\n");
        kd_mp4_destroy(g_mp4_demuxer_handle);
        return -1;
    }
    //printf("k_mp4_get_file_info duration:%d,track count:%d\n",file_info.duration ,file_info.track_num);
    g_progress_info.total_time = file_info.duration;

    for (int i = 0; i < file_info.track_num; i++)
    {
        k_mp4_track_info_s track_info;
        memset(&track_info, 0, sizeof(track_info));
        ret = kd_mp4_get_track_by_index(g_mp4_demuxer_handle, i, &track_info);
        if (ret < 0)
        {
            printf("k_mp4_get_track_by_index: get track: %d info failed.\n", i);
            kd_mp4_destroy(g_mp4_demuxer_handle);
            return -1;
        }

        if (track_info.track_type == K_MP4_STREAM_VIDEO)
        {
            g_player_video_info.track_id = track_info.video_info.track_id;
            g_player_video_info.width = track_info.video_info.width;
            g_player_video_info.height = track_info.video_info.height;
            g_player_video_info.type = _get_payload_type(track_info.video_info.codec_id);
            g_video_track = track_info.video_info.track_id;

            printf("video track info:type:%d,track_id:%d,width:%d,height:%d\n",
                   g_player_video_info.type, track_info.video_info.track_id,
                   track_info.video_info.width, track_info.video_info.height);
        }
        else if (track_info.track_type == K_MP4_STREAM_AUDIO)
        {
            g_player_audio_info.track_id = track_info.audio_info.track_id;
            g_player_audio_info.type = _get_payload_type(track_info.audio_info.codec_id);
            g_player_audio_info.samplerate = track_info.audio_info.sample_rate;
            g_player_audio_info.channel_num = track_info.audio_info.channels;

            if (g_player_audio_info.type != K_PT_G711A && g_player_audio_info.type != K_PT_G711U)
            {
                continue;
            }
            g_audio_track = track_info.audio_info.track_id;

            printf("audio track info:type:%d,track_id:%d,samplerate:%d,channels:%d\n",
                   g_player_audio_info.type, track_info.audio_info.track_id,
                   track_info.audio_info.sample_rate, track_info.audio_info.channels);
        }
    }

    return 0;
}
k_s32 kd_player_init(k_bool init_vo)
{
    sys_init(init_vo);
    return K_SUCCESS;
}

k_s32 kd_player_deinit(k_bool deinit_vo)
{
    sys_deinit(deinit_vo);

    return K_SUCCESS;
}

void kd_player_set_connector_type(int connector_type)
{
    g_connector_type = connector_type;
}

k_s32 kd_player_setdatasource(const k_char *filePath)
{
    k_s32 ret;
    k_bool avsync = K_FALSE;

    memset(g_mp4_filename, 0, sizeof(g_mp4_filename));
    memcpy(g_mp4_filename, filePath, strlen(filePath) + 1);

    ret = _init_mp4_demuxer(g_mp4_filename);
    if (ret != K_SUCCESS)
    {
        printf("_init_mp4_demuxer failed\n");
        return K_FAILED;
    }

    if (g_video_track != INVALID_STREAM_TRACK)
    {
        ret = disp_open(g_player_video_info.type,g_player_video_info.width,g_player_video_info.height,g_connector_type);
        if (ret != K_SUCCESS)
        {
            printf("disp_open failed\n");
            return K_FAILED;
        }
        //avsync = K_TRUE;
    }

    if (g_audio_track != INVALID_STREAM_TRACK)
    {
        ret = ao_open(g_player_audio_info.samplerate, g_player_audio_info.channel_num, g_player_audio_info.type, avsync);
        if (ret != K_SUCCESS)
        {
            printf("ao_open failed\n");
            return K_FAILED;
        }
    }

    return K_SUCCESS;
}

static void *play_thread(void *arg)
{
    k_s32 ret;
    k_mp4_frame_data_s frame_data;
    while (g_play_start)
    {
        if (g_play_pause)
        {
            usleep(100*1000);
            continue;
        }
        memset(&frame_data, 0, sizeof(frame_data));
        ret = kd_mp4_get_frame(g_mp4_demuxer_handle, &frame_data);

        if (ret < 0)
        {
            printf("k_mp4_get_frame: get frame failed.\n");
            return NULL;
        }
        if (frame_data.eof)
        {
            printf("demuxer finished.\n");
            break;
        }

        if (frame_data.codec_id == K_MP4_CODEC_ID_H264 || frame_data.codec_id == K_MP4_CODEC_ID_H265)
        {
            if (g_video_track != INVALID_STREAM_TRACK)
            {
// printf("video data size:%d\n",frame_data.data_length);
#if 0
                printf("video data size:%d,data:0x%x_0x%x_0x%x_0x%x_0x%x_0x%x_0x%x_0x%x_0x%x_0x%x\n",\
                frame_data.data_length,frame_data.data[0],frame_data.data[1],frame_data.data[2],frame_data.data[3],frame_data.data[4],
                frame_data.data[5],frame_data.data[6],frame_data.data[7],frame_data.data[8],frame_data.data[9]);
#endif
                if (frame_data.data_length != 0)
                {
                    disp_play(frame_data.data, frame_data.data_length, frame_data.time_stamp, K_FALSE);
                    g_progress_info.cur_time = frame_data.time_stamp;
                    //printf("video data size:%d,timestamp:%d\n", frame_data.data_length,frame_data.time_stamp);
                    if (g_pfnCallback != NULL)
                    {
                        g_pfnCallback(K_PLAYER_EVENT_PROGRESS, &g_progress_info);
                    }
                }
            }
        }
        else if (frame_data.codec_id == K_MP4_CODEC_ID_G711A || frame_data.codec_id == K_MP4_CODEC_ID_G711U)
        {
            if (g_audio_track != INVALID_STREAM_TRACK)
            {
                //printf("audio data size:%d,timestamp:%d\n", frame_data.data_length,frame_data.time_stamp);
                ao_play(frame_data.data,frame_data.data_length,frame_data.time_stamp);
            }
        }
    }

    //send over frame
    if (g_video_track != INVALID_STREAM_TRACK)
    {
        char end_frame[100] = {0};
        end_frame[0] = 0x0;
        end_frame[1] = 0x0;
        end_frame[2] = 0x0;
        end_frame[3] = 0x1;
        end_frame[4] = 0x41;
        disp_play((k_u8*)end_frame, 100, 0, K_TRUE);
    }

    ret = kd_mp4_destroy(g_mp4_demuxer_handle);
    if (ret < 0)
    {
        printf("destroy mp4 failed.\n");
    }

    if (g_pfnCallback != NULL)
    {
        g_pfnCallback(K_PLAYER_EVENT_EOF, g_data_context);
    }

    return NULL;
}

k_s32 kd_player_start()
{
    if (g_play_start)
    {
        printf("kd_player_start already start\n");
        return K_FAILED;
    }

    g_play_start = K_TRUE;
    pthread_create(&g_play_tid, NULL, play_thread, NULL);

    return K_SUCCESS;
}

k_s32 kd_player_pause()
{
    g_play_pause = K_TRUE;
    return K_SUCCESS;
}

k_s32 kd_player_resume()
{
    g_play_pause = K_FALSE;
    return K_SUCCESS;
}

k_s32 kd_player_regcallback(K_PLAYER_EVENT_FN pfnCallback, void *pData)
{
    g_pfnCallback = pfnCallback;
    g_data_context = pData;
    return 0;
}

k_s32 kd_player_stop()
{
    if (!g_play_start)
    {
        printf("kd_player_start already stop\n");
        return K_FAILED;
    }

    g_play_start = K_FALSE;
    pthread_join(g_play_tid, NULL);
    //wait ao send all data
    sleep(1);

    if (g_audio_track != INVALID_STREAM_TRACK)
    {
        ao_close();
    }
    if (g_video_track != INVALID_STREAM_TRACK)
    {
        disp_close();
    }

    return K_SUCCESS;
}


static int g_pic_width = 1088;
static int g_pic_height = 1920;
static k_u8 g_pic_data[1920*1088*3/2];
static int g_pic_size = 0;


k_s32 kd_picture_init()
{
    k_s32 ret;
    ret = disp_open(K_PT_JPEG,g_pic_width,g_pic_height,g_connector_type);
    if (ret != K_SUCCESS)
    {
        printf("disp_open failed\n");
        return K_FAILED;
    }

    return K_SUCCESS;
}

k_s32 kd_picture_show(const k_char* filePath)
{
    //read jpeg file
    FILE *fp = fopen(filePath, "rb");
    if (NULL == fp)
    {
        printf("open file:%s failed\n", filePath);
        return -1;
    }

    fseek(fp, 0, SEEK_END);
    g_pic_size = ftell(fp);
    fseek(fp, 0, SEEK_SET);
    fread(g_pic_data, g_pic_size, 1, fp);
    fclose(fp);

    disp_play(g_pic_data, g_pic_size, 0, K_FALSE);

    return K_SUCCESS;
}

k_s32 kd_picture_deinit()
{
    disp_close();
    return K_SUCCESS;
}