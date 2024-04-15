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

#include <stdio.h>
#include <sys/mman.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <pthread.h>
#include <stdlib.h>
#include <getopt.h>
#include <signal.h>
#include "k_type.h"
#include "mp4_format.h"

static k_u8 g_exit = 0;

static void sig_handler(int sig) {
    g_exit = 1;
}

int main(int argc, char *argv[]) {

    signal(SIGINT, sig_handler);

    printf("mp4 demuxer...\n");

    // create mp4 instance
    KD_HANDLE mp4_demuxer = NULL;
    k_mp4_config_s mp4_config;
    memset(&mp4_config, 0, sizeof(mp4_config));
    mp4_config.config_type = K_MP4_CONFIG_DEMUXER;
    strcpy(mp4_config.demuxer_config.file_name, "/sharefs/test.mp4");
    mp4_config.muxer_config.fmp4_flag = 0;

    k_s32 ret = kd_mp4_create(&mp4_demuxer, &mp4_config);
    if (ret < 0) {
        printf("mp4 demuxer create failed.\n");
        return -1;
    }

    k_mp4_file_info_s file_info;
    memset(&file_info, 0, sizeof(file_info));
    ret = kd_mp4_get_file_info(mp4_demuxer, &file_info);
    if (ret < 0) {
        printf("get file info failed.\n");
        goto destroy_mp4;
    }

    printf("file_info: track_num: %d, duration: %lu.\n", file_info.track_num, file_info.duration);

    FILE *video_file_fp = NULL;
    FILE *audio_file_fp = NULL;

    for (int i = 0; i < file_info.track_num; i++) {
        k_mp4_track_info_s track_info;
        memset(&track_info, 0, sizeof(track_info));
        ret = kd_mp4_get_track_by_index(mp4_demuxer, i, &track_info);
        if (ret < 0) {
            printf("get track: %d info failed.\n", i);
            goto destroy_mp4;
        }

        printf("track index: %d info:\n", i);
        printf("    track_type: %d.\n", track_info.track_type);
        printf("    time_scale: %d.\n", track_info.time_scale);
        if (track_info.track_type == K_MP4_STREAM_VIDEO) {
            printf("    codec_id: %d.\n", track_info.video_info.codec_id);
            printf("    track_id: %d.\n", track_info.video_info.track_id);
            printf("    width: %d.\n", track_info.video_info.width);
            printf("    height: %d.\n", track_info.video_info.height);
            if (track_info.video_info.codec_id == K_MP4_CODEC_ID_H264)
                video_file_fp = fopen("/sharefs/test.264", "wb");
            else if (track_info.video_info.codec_id == K_MP4_CODEC_ID_H265)
                video_file_fp = fopen("/sharefs/test.265", "wb");
        } else if (track_info.track_type == K_MP4_STREAM_AUDIO) {
            printf("    codec_id: %d.\n", track_info.audio_info.codec_id);
            printf("    track_id: %d.\n", track_info.audio_info.track_id);
            printf("    channels: %d.\n", track_info.audio_info.channels);
            printf("    sample_rate: %d.\n", track_info.audio_info.sample_rate);
            printf("    bit_per_sample: %d.\n", track_info.audio_info.bit_per_sample);
            if (track_info.audio_info.codec_id == K_MP4_CODEC_ID_G711A)
                audio_file_fp = fopen("/sharefs/test.g711a", "wb");
            else if (track_info.audio_info.codec_id == K_MP4_CODEC_ID_G711U)
                audio_file_fp = fopen("/sharefs/test.g711u", "wb");
        }
    }

    k_mp4_frame_data_s frame_data;
    while (!g_exit) {
        memset(&frame_data, 0, sizeof(frame_data));
        ret = kd_mp4_get_frame(mp4_demuxer, &frame_data);
        if (ret < 0) {
            printf("get frame data failed.\n");
            goto destroy_mp4;
        }

        if (frame_data.eof) {
            printf("demuxer finished.\n");
            break;
        }

        if (frame_data.codec_id == K_MP4_CODEC_ID_H264 || frame_data.codec_id == K_MP4_CODEC_ID_H265) {
            fwrite(frame_data.data, 1, frame_data.data_length, video_file_fp);
        } else if (frame_data.codec_id == K_MP4_CODEC_ID_G711A || frame_data.codec_id == K_MP4_CODEC_ID_G711U) {
            fwrite(frame_data.data, 1, frame_data.data_length, audio_file_fp);
        }
    }

destroy_mp4:
    ret = kd_mp4_destroy(mp4_demuxer);
    if (ret < 0) {
        printf("destroy mp4 failed.\n");
        return -1;
    }

    if (video_file_fp) {
        fclose(video_file_fp);
        video_file_fp = NULL;
    }

    if (audio_file_fp) {
        fclose(audio_file_fp);
        audio_file_fp = NULL;
    }

    printf("mp4 demuxer end...\n");
    return 0;
}
