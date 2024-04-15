#ifndef _KD_MP4_FORMAT_H_
#define _KD_MP4_FORMAT_H_

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif // __cplusplus

typedef void* KD_HANDLE;

typedef enum {
    K_MP4_CONFIG_MUXER = 1,
    K_MP4_CONFIG_DEMUXER,
    K_MP4_CONFIG_BUTT
} k_mp4_config_type_e;

typedef enum {
    K_MP4_STREAM_VIDEO = 1,
    K_MP4_STREAM_AUDIO,
    K_MP4_STREAM_BUTT
} k_mp4_track_type_e;

typedef enum {
    K_MP4_CODEC_ID_H264 = 0,
    K_MP4_CODEC_ID_H265,
    K_MP4_CODEC_ID_G711A,
    K_MP4_CODEC_ID_G711U,
    K_MP4_CODEC_ID_BUTT
} k_mp4_codec_id_e;

typedef struct {
    char file_name[128];
    uint8_t fmp4_flag;
} k_mp4_config_muxer_s;

typedef struct {
    char file_name[128];
} k_mp4_config_demuxer_s;

typedef struct {
    k_mp4_config_type_e config_type;
    union {
        k_mp4_config_muxer_s muxer_config;
        k_mp4_config_demuxer_s demuxer_config;
    };
} k_mp4_config_s;

typedef struct {
    uint32_t width;
    uint32_t height;
    uint32_t track_id;
    k_mp4_codec_id_e codec_id;
} k_mp4_video_info_s;

typedef struct {
    uint32_t channels;
    uint32_t sample_rate;
    uint32_t bit_per_sample;
    uint32_t track_id;
    k_mp4_codec_id_e codec_id;
} k_mp4_audio_info_s;

typedef struct {
    k_mp4_track_type_e track_type;
    uint32_t time_scale;
    union {
        k_mp4_video_info_s video_info;
        k_mp4_audio_info_s audio_info;
    };
} k_mp4_track_info_s;

typedef struct {
    uint64_t duration;
    uint32_t track_num;
} k_mp4_file_info_s;

typedef struct {
    k_mp4_codec_id_e codec_id;
    uint64_t time_stamp;
    uint8_t *data;
    uint32_t data_length;
    uint8_t eof;
} k_mp4_frame_data_s;

int kd_mp4_create(KD_HANDLE *mp4_handle, k_mp4_config_s *mp4_cfg);

int kd_mp4_destroy(KD_HANDLE mp4_handle);

int kd_mp4_create_track(KD_HANDLE mp4_handle, KD_HANDLE *track_handle, k_mp4_track_info_s *mp4_track_info);

int kd_mp4_destroy_tracks(KD_HANDLE mp4_handle);

int kd_mp4_write_frame(KD_HANDLE mp4_handle, KD_HANDLE track_handle, k_mp4_frame_data_s *frame_data);

int kd_mp4_get_file_info(KD_HANDLE mp4_handle, k_mp4_file_info_s *file_info);

int kd_mp4_get_track_by_index(KD_HANDLE mp4_handle, uint32_t index, k_mp4_track_info_s *mp4_track_info);

int kd_mp4_get_frame(KD_HANDLE mp4_handle, k_mp4_frame_data_s *frame_data);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif // __cplusplus

#endif // _KD_MP4_FORMAT_H_
