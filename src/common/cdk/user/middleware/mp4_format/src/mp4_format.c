#include <assert.h>
#include <pthread.h>
#include <string.h>
#include "mp4_format.h"
#include "mov-writer.h"
#include "mov-reader.h"
#include "mpeg4-avc.h"
#include "mpeg4-hevc.h"
#include "mov-format.h"
#include "mp4-writer.h"

#define MAX_INSTANCE_NUM 4
#define MAX_TRACK_NUM 3
static pthread_mutex_t s_mutex = PTHREAD_MUTEX_INITIALIZER;

struct mov_file_cache_t
{
	FILE* fp;
	uint8_t ptr[64 * 1024];
	unsigned int len;
	unsigned int off;
	uint64_t tell;
};

typedef struct kmp4_packet_t {
    int flags;
    uint64_t pts;
    uint64_t dts;
    uint32_t track_id;
    void *ptr;
    size_t bytes;
} k_mp4_packet_t;

typedef struct ktrack_info_s {
    struct mpeg4_avc_t s_avc;
    struct mpeg4_hevc_t s_hevc;
    uint32_t obj_id;
} k_track_info_s;

typedef struct kdemuxer_instance {
    struct mov_reader_t *mov;
    struct mov_file_cache_t file_cache;
    k_track_info_s track_ctx;
} k_demuxer_instance;

typedef struct ktrack_ctx {
    uint8_t s_extra_data[64 * 1024];
    uint32_t time_scale;
    uint32_t width;
    uint32_t height;
    uint32_t pts;
    uint32_t dts;
    int track_flag;
    int add_to_mp4;
    int has_get_idr;
    int has_vps;
    int has_sps;
    int has_pps;
    int has_idr;
    k_mp4_track_type_e track_type;
    struct mpeg4_avc_t avc;
    struct mpeg4_hevc_t hevc;
    k_mp4_audio_info_s audio_info;
    k_mp4_video_info_s video_info;
} k_track_ctx;

typedef struct kmuxer_instance {
    struct mp4_writer_t *mov;
    k_track_ctx *track[MAX_TRACK_NUM];
    FILE *fp;
} k_muxer_instance;

typedef struct kmp4_instance {
    k_mp4_config_type_e instance_type;
    union {
        k_muxer_instance muxer_instance;
        k_demuxer_instance demuxer_instance;
    };
    uint8_t buffer[2 * 1024 * 1024];
    size_t buffer_size;
    uint8_t buffer2[2 * 1024 * 1024];
    size_t buffer2_size;
} k_mp4_instance;

static int mov_file_read(void* fp, void* data, uint64_t bytes)
{
    if (bytes == fread(data, 1, bytes, (FILE*)fp))
        return 0;
	return 0 != ferror((FILE*)fp) ? ferror((FILE*)fp) : -1 /*EOF*/;
}

static int mov_file_write(void* fp, const void* data, uint64_t bytes)
{
	return bytes == fwrite(data, 1, bytes, (FILE*)fp) ? 0 : ferror((FILE*)fp);
}

static int mov_file_seek(void* fp, int64_t offset)
{
	return fseek((FILE*)fp, offset, offset >= 0 ? SEEK_SET : SEEK_END);
}

static int64_t mov_file_tell(void* fp)
{
	return ftell((FILE*)fp);
}

static int mov_file_cache_read(void* fp, void* data, uint64_t bytes)
{
	uint8_t* p = (uint8_t*)data;
	struct mov_file_cache_t* file = (struct mov_file_cache_t*)fp;
	while (bytes > 0)
	{
		assert(file->off <= file->len);
		if (file->off >= file->len)
		{
			if (bytes >= sizeof(file->ptr))
			{
				if (bytes == fread(p, 1, bytes, file->fp))
				{
					file->tell += bytes;
					return 0;
				}
				return 0 != ferror(file->fp) ? ferror(file->fp) : -1 /*EOF*/;
			}
			else
			{
				file->off = 0;
				file->len = (unsigned int)fread(file->ptr, 1, sizeof(file->ptr), file->fp);
				if (file->len < 1)
					return 0 != ferror(file->fp) ? ferror(file->fp) : -1 /*EOF*/;
			}
		}

		if (file->off < file->len)
		{
			unsigned int n = file->len - file->off;
			n = n > bytes ? (unsigned int)bytes : n;
			memcpy(p, file->ptr + file->off, n);
			file->tell += n;
			file->off += n;
			bytes -= n;
			p += n;
		}
	}

	return 0;
}

static int mov_file_cache_write(void* fp, const void* data, uint64_t bytes)
{
	struct mov_file_cache_t* file = (struct mov_file_cache_t*)fp;

	file->tell += bytes;

	if (file->off + bytes < sizeof(file->ptr))
	{
		memcpy(file->ptr + file->off, data, bytes);
		file->off += (unsigned int)bytes;
		return 0;
	}

	// write buffer
	if (file->off > 0)
	{
		if (file->off != fwrite(file->ptr, 1, file->off, file->fp))
			return ferror(file->fp);
		file->off = 0; // clear buffer
	}

	// write data;
	return bytes == fwrite(data, 1, bytes, file->fp) ? 0 : ferror(file->fp);
}

static int mov_file_cache_seek(void* fp, int64_t offset)
{
	int r;
	struct mov_file_cache_t* file = (struct mov_file_cache_t*)fp;
	if (offset != file->tell)
	{
		if (file->off > file->len)
		{
			// write bufferred data
			if(file->off != fwrite(file->ptr, 1, file->off, file->fp))
				return ferror(file->fp);
		}

		file->off = file->len = 0;
		r = fseek(file->fp, offset, offset >= 0 ? SEEK_SET : SEEK_END);
		file->tell = ftell(file->fp);
		return r;
	}
	return 0;
}

static int64_t mov_file_cache_tell(void* fp)
{
	struct mov_file_cache_t* file = (struct mov_file_cache_t*)fp;
	if (ftell(file->fp) != (int64_t)(file->tell + (uint64_t)(int)(file->len - file->off)))
		return -1;
	return (int64_t)file->tell;
	//return ftell(file->fp);
}

const struct mov_buffer_t* mov_file_buffer(void)
{
	static struct mov_buffer_t s_io = {
		mov_file_read,
		mov_file_write,
		mov_file_seek,
		mov_file_tell,
	};
	return &s_io;
};

const struct mov_buffer_t* mov_file_cache_buffer(void)
{
	static struct mov_buffer_t s_io = {
		mov_file_cache_read,
		mov_file_cache_write,
		mov_file_cache_seek,
		mov_file_cache_tell,
	};
	return &s_io;
};

static const uint8_t* startcode(const uint8_t *data, size_t bytes)
{
	size_t i;
	for (i = 2; i + 1 < bytes; i++)
	{
		if (0x01 == data[i] && 0x00 == data[i - 1] && 0x00 == data[i - 2])
			return data + i + 1;
	}

	return NULL;
}

static void mov_video_info(void *param, uint32_t track_id, uint32_t time_scale, uint8_t object, int width, int height, const void *extra, size_t bytes) {
    k_mp4_track_info_s *track_info = (k_mp4_track_info_s*)param;
    track_info->track_type = K_MP4_STREAM_VIDEO;
    track_info->time_scale = time_scale;
    if (MOV_OBJECT_H264 == object) {
        // mpeg4_avc_decoder_configuration_record_load((const uint8_t*)extra, bytes, &s_avc);
        track_info->video_info.codec_id = K_MP4_CODEC_ID_H264;
        track_info->video_info.width = width;
        track_info->video_info.height = height;
        track_info->video_info.track_id = track_id;
    } else if (MOV_OBJECT_H265 == object) {
        track_info->video_info.codec_id = K_MP4_CODEC_ID_H265;
        track_info->video_info.width = width;
        track_info->video_info.height = height;
        track_info->video_info.track_id = track_id;
    }

    return;
}

static void mov_audio_info(void *param, uint32_t track_id, uint32_t time_scale, uint8_t object, int channel_count, int bit_per_sample, int sample_rate, const void *extra, size_t bytes) {
    k_mp4_track_info_s *track_info = (k_mp4_track_info_s*)param;
    track_info->track_type = K_MP4_STREAM_AUDIO;
    track_info->time_scale = time_scale;
    if (MOV_OBJECT_G711a == object || MOV_OBJECT_G711u == object) {
        track_info->audio_info.channels = channel_count;
        if (MOV_OBJECT_G711a == object)
            track_info->audio_info.codec_id =  K_MP4_CODEC_ID_G711A;
        else if (MOV_OBJECT_G711u == object)
            track_info->audio_info.codec_id =  K_MP4_CODEC_ID_G711U;
        track_info->audio_info.sample_rate = sample_rate;
        track_info->audio_info.bit_per_sample =  bit_per_sample;
        track_info->audio_info.track_id = track_id;
    }

    return;
}

static void* onalloc(void* param, uint32_t track, size_t bytes, int64_t pts, int64_t dts, int flags)
{
	k_mp4_packet_t* pkt = (k_mp4_packet_t*)param;
	if (pkt->bytes < bytes)
		return NULL;
	pkt->flags = flags;
	pkt->pts = pts;
	pkt->dts = dts;
	pkt->track_id = track;
	pkt->bytes = bytes;
	return pkt->ptr;
}

int kd_mp4_create(KD_HANDLE *mp4_handle, k_mp4_config_s *mp4_cfg) {
    if (!mp4_cfg) {
        printf("kd_mp4_create: mp4 config is null.\n");
        return -1;
    }

    k_mp4_instance *mp4_instance = NULL;
    mp4_instance = (k_mp4_instance *)calloc(1, sizeof(k_mp4_instance));
    if (!mp4_instance) {
        printf("kd_mp4_create: create mp4 instance failed.\n");
        return -1;
    }
    mp4_instance->buffer_size = 2 * 1024 * 1024;
    mp4_instance->buffer2_size = 2 * 1024 * 1024;

    mp4_instance->instance_type = mp4_cfg->config_type;

    switch (mp4_cfg->config_type) {
        case K_MP4_CONFIG_MUXER: {
            FILE *fp = fopen(mp4_cfg->muxer_config.file_name, "wb+");
            if (!fp) {
                printf("kd_mp4_create: output file %s open failed.\n", mp4_cfg->muxer_config.file_name);
                return -1;
            }

            // remove "MOV_FLAG_SEGMENT", in order to obtain fmp4-duration.. TODO
            mp4_instance->muxer_instance.mov = mp4_writer_create(mp4_cfg->muxer_config.fmp4_flag, mov_file_buffer(), fp, MOV_FLAG_FASTSTART /*| MOV_FLAG_SEGMENT*/);
            if (!mp4_instance->muxer_instance.mov) {
                printf("kd_mp4_create: create mp4 writer failed.\n");
                return -1;
            }
            mp4_instance->muxer_instance.fp = fp;
            break;
        }
        case K_MP4_CONFIG_DEMUXER: {
            struct mov_file_cache_t *file_cache = &(mp4_instance->demuxer_instance.file_cache);
            file_cache->fp = fopen(mp4_cfg->demuxer_config.file_name, "rb");
            if (!file_cache->fp) {
                printf("input file %s open failed.\n", mp4_cfg->demuxer_config.file_name);
                return -1;
            }
            mp4_instance->demuxer_instance.mov = mov_reader_create(mov_file_cache_buffer(), file_cache);
            if (!mp4_instance->demuxer_instance.mov) {
                printf("kd_mp4_create: create mp4 reader failed.\n");
                return -1;
            }

            k_demuxer_instance *demuxer = &mp4_instance->demuxer_instance;
            uint8_t extra_data[1024];
            int32_t extra_data_size = 0;
            uint8_t obj_id;
            mov_reader_video_entry_info(demuxer->mov, &obj_id, extra_data, &extra_data_size);
            if (extra_data_size) {
                if (obj_id == MOV_OBJECT_H264)
                    mpeg4_avc_decoder_configuration_record_load((const uint8_t*)extra_data, extra_data_size, &demuxer->track_ctx.s_avc);
                else if (obj_id == MOV_OBJECT_HEVC) {
                    mpeg4_hevc_decoder_configuration_record_load((const uint8_t*)extra_data, extra_data_size, &demuxer->track_ctx.s_hevc);
                }
            }

            break;
        }
        default : {
            printf("kd_mp4_create: mp4 config type not support.\n");
            return -1;
        }
    }

    *mp4_handle = (KD_HANDLE)mp4_instance;

    return 0;
}

int kd_mp4_destroy(KD_HANDLE mp4_handle) {
    if (mp4_handle == NULL) {
        printf("kd_mp4_destroy: mp4 handle is null.\n");
        return -1;
    }

    k_mp4_instance *mp4_instance = (k_mp4_instance *)mp4_handle;

    switch (mp4_instance->instance_type) {
        case K_MP4_CONFIG_MUXER: {
            if (mp4_instance->muxer_instance.mov) {
                mp4_writer_destroy(mp4_instance->muxer_instance.mov);
                mp4_instance->muxer_instance.mov = NULL;
            }
            if (mp4_instance->muxer_instance.fp) {
                fclose(mp4_instance->muxer_instance.fp);
                mp4_instance->muxer_instance.fp = NULL;
            }
            break;
        }
        case K_MP4_CONFIG_DEMUXER: {
            if (mp4_instance->demuxer_instance.mov) {
                mov_reader_destroy(mp4_instance->demuxer_instance.mov);
                mp4_instance->demuxer_instance.mov = NULL;
                if (mp4_instance->demuxer_instance.file_cache.fp) {
                    fclose(mp4_instance->demuxer_instance.file_cache.fp);
                    mp4_instance->demuxer_instance.file_cache.fp = NULL;
                }
            }
            break;
        }
        default : {
            printf("kd_mp4_destroy: mp4 config type not support.\n");
            return -1;
        }
    }

    if (mp4_instance) {
        free(mp4_instance);
        mp4_instance = NULL;
    }

    return 0;
}

int kd_mp4_create_track(KD_HANDLE mp4_handle, KD_HANDLE *track_handle, k_mp4_track_info_s *mp4_track_info) {
    if (mp4_handle == NULL) {
        printf("kd_mp4_create_track: mp4 handl is null.\n");
        return -1;
    }

    k_mp4_instance *mp4_instance = (k_mp4_instance *)mp4_handle;
    k_track_ctx *track = NULL;

    if (mp4_track_info->track_type == K_MP4_STREAM_VIDEO) {
        track = (k_track_ctx *)calloc(1, sizeof(k_track_ctx));
        if (!track) {
            printf("kd_mp4_create_track: create track failed.\n");
            return -1;
        }

        track->track_type = K_MP4_STREAM_VIDEO;
        track->time_scale = mp4_track_info->time_scale;
        track->pts = 0;
        track->dts = 0;
        track->width = mp4_track_info->video_info.width;
        track->height = mp4_track_info->video_info.height;
        track->track_flag = 1;
        track->add_to_mp4 = -1;
        track->has_get_idr = 0;
        track->has_vps = 0;
        track->has_sps = 0;
        track->has_pps = 0;
        track->has_idr = 0;
        memcpy(&track->video_info, &(mp4_track_info->video_info), sizeof(k_mp4_video_info_s));
    } else if (mp4_track_info->track_type == K_MP4_STREAM_AUDIO) {
        track = (k_track_ctx *)calloc(1, sizeof(k_track_ctx));
        if (!track) {
            printf("kd_mp4_create_track: create track failed.\n");
            return -1;
        }

        track->track_type = K_MP4_STREAM_AUDIO;
        track->pts = 0;
        track->dts = 0;
        track->track_flag = 1;
        track->add_to_mp4 = -1;
        memcpy(&track->audio_info, &(mp4_track_info->audio_info), sizeof(k_mp4_audio_info_s));
    } else {
        printf("kd_mp4_create_track: the track type is invalid.\n");
        return -1;
    }

    *track_handle = (KD_HANDLE)track;

    int track_set_to_mp4 = 0;
    pthread_mutex_lock(&s_mutex);
    for (int i = 0; i < MAX_TRACK_NUM; i++) {
        if (mp4_instance->muxer_instance.track[i] == NULL) {
            mp4_instance->muxer_instance.track[i] = track;
            track_set_to_mp4 = 1;
            break;
        }
    }
    pthread_mutex_unlock(&s_mutex);

    if (!track_set_to_mp4) {
        printf("kd_mp4_create_track: mp4 already cannot creat new track.\n");
        return -1;
    }

    return 0;
}

int kd_mp4_destroy_tracks(KD_HANDLE mp4_handle) {
    if (mp4_handle == NULL) {
        printf("kd_mp4_destroy_tracks: mp4 handle is null.\n");
        return -1;
    }

    k_mp4_instance *mp4_instance = (k_mp4_instance *)mp4_handle;
    if (mp4_instance->instance_type == K_MP4_CONFIG_DEMUXER) {
        printf("kd_mp4_destroy_tracks: this interface not support demuxer type.\n");
        return -1;
    }

    for (int i = 0; i < MAX_TRACK_NUM; i++) {
        if (mp4_instance->muxer_instance.track[i]) {
            free(mp4_instance->muxer_instance.track[i]);
            mp4_instance->muxer_instance.track[i] = NULL;
        }
    }

    return 0;
}

static k_track_ctx * get_audio_track(KD_HANDLE mp4_handle) {
    if (mp4_handle == NULL) {
        return -1;
    }

    k_mp4_instance *mp4_instance = (k_mp4_instance *)mp4_handle;
    if (mp4_instance->instance_type == K_MP4_CONFIG_DEMUXER) {
        return -1;
    }

    for (int i = 0; i < MAX_TRACK_NUM; i++) {
        k_track_ctx *track = mp4_instance->muxer_instance.track[i];
        if (track && track->track_type == K_MP4_STREAM_AUDIO) {
            return track;
        }
    }
    return NULL;
}

int kd_mp4_write_frame(KD_HANDLE mp4_handle, KD_HANDLE track_handle, k_mp4_frame_data_s *frame_data) {
    if (mp4_handle == NULL || track_handle == NULL) {
        printf("k_mp4_write_frame: mp4 handle or track handle is null.\n");
        return -1;
    }

    k_mp4_instance *mp4_instance = (k_mp4_instance *)mp4_handle;
    if (mp4_instance->instance_type == K_MP4_CONFIG_DEMUXER) {
        printf("kd_mp4_write_frame: this interface not support demuxer.\n");
        return -1;
    }

    k_track_ctx *track = (k_track_ctx *)track_handle;

    if (track->track_type == K_MP4_STREAM_VIDEO) {
        const uint8_t *ptr, *nalu, *end;
        uint32_t ptr_len;

        ptr = (const uint8_t *)frame_data->data;
        ptr_len = frame_data->data_length;
        end = ptr + ptr_len;
        const uint8_t *p = ptr;

        if (!track->has_get_idr) {
            if (frame_data->codec_id == K_MP4_CODEC_ID_H264) {
                while (1) {
                    nalu = startcode(p, (int)(end - p));
                    if (!nalu) {
                        break;
                    }
                    int nalu_type = nalu[0] & 0x1F;
                    if (nalu_type == 7) {
                        track->has_sps = 1;
                    } else if (nalu_type == 8) {
                        track->has_pps = 1;
                    } else if (nalu_type == 5) {
                        track->has_idr = 1;
                    }
                    p = nalu;
                }
                if (track->has_sps && track->has_pps && track->has_idr) {
                    track->has_get_idr = 1;
                } else {
                    printf("kd_mp4_write_frame: mp4 muxer write should start with key frame (header + idr).\n");
                    return -1;
                }
            } else if (frame_data->codec_id == K_MP4_CODEC_ID_H265) {
                while (1) {
                    nalu = startcode(p, (int)(end - p));
                    if (!nalu) {
                        break;
                    }
                    int nalu_type = (nalu[0] >> 1) &0x3f;
                    if (nalu_type == 32) {
                        track->has_vps = 1;
                    } else if (nalu_type == 33) {
                        track->has_sps = 1;
                    } else if (nalu_type == 34) {
                        track->has_pps = 1;
                    } else if (nalu_type == 19) {
                        track->has_idr = 1;
                    }
                    p = nalu;
                }
                if (track->has_vps && track->has_sps && track->has_pps && track->has_idr) {
                    track->has_get_idr = 1;
                } else {
                    printf("kd_mp4_write_frame: mp4 muxer write should start with key frame (header + idr).\n");
                    return -1;
                }
            }
        }

        int vcl = 0;
        int update = 0;
        uint8_t *s_buffer = mp4_instance->buffer;
        size_t s_buffer_size = mp4_instance->buffer_size;
        if (frame_data->codec_id == K_MP4_CODEC_ID_H264) {
            int n = h264_annexbtomp4(&track->avc, ptr, ptr_len, s_buffer, s_buffer_size, &vcl, &update);
            if (track->add_to_mp4 < 0) {
                if (track->avc.nb_sps < 1 || track->avc.nb_pps < 1) {
                    return -2;
                }

                int extra_data_size = mpeg4_avc_decoder_configuration_record_save(&track->avc, track->s_extra_data, sizeof(track->s_extra_data));
                if (extra_data_size <= 0) {
                    return -1;
                }

                track->add_to_mp4 = mp4_writer_add_video(mp4_instance->muxer_instance.mov, MOV_OBJECT_H264, track->width, track->height, track->s_extra_data, extra_data_size);
                if (track->add_to_mp4 < 0)
                    return -1;

                // FIXME, code-refactor expected
                k_track_ctx *track_audio = get_audio_track(mp4_handle);
                if (track_audio && track_audio->add_to_mp4 < 0) {
                    track_audio->add_to_mp4 = mp4_writer_add_audio(mp4_instance->muxer_instance.mov, MOV_OBJECT_G711u, 1, 16, 8000, NULL, 0);
                }
                mp4_writer_init_segment(mp4_instance->muxer_instance.mov);
            }

            track->pts = frame_data->time_stamp / 1000;
            mp4_writer_write(mp4_instance->muxer_instance.mov, track->add_to_mp4, s_buffer, n, track->pts, track->pts, vcl == 1 ? MOV_AV_FLAG_KEYFREAME : 0);
        } else if (frame_data->codec_id == K_MP4_CODEC_ID_H265) {
            int n = h265_annexbtomp4(&track->hevc, ptr, ptr_len, s_buffer, s_buffer_size, &vcl, &update);
            if (track->add_to_mp4 < 0) {
                if (track->hevc.numOfArrays < 1) {
                    return -2;
                }

                int extra_data_size = mpeg4_hevc_decoder_configuration_record_save(&track->hevc, track->s_extra_data, sizeof(track->s_extra_data));
                if (extra_data_size <= 0) {
                    return -1;
                }

                track->add_to_mp4 = mp4_writer_add_video(mp4_instance->muxer_instance.mov, MOV_OBJECT_HEVC, track->width, track->height, track->s_extra_data, extra_data_size);
                if (track->add_to_mp4 < 0) {
                    return -1;
                }

                // FIXME, code-refactor expected
                k_track_ctx *track_audio = get_audio_track(mp4_handle);
                if (track_audio && track_audio->add_to_mp4 < 0) {
                    track_audio->add_to_mp4 = mp4_writer_add_audio(mp4_instance->muxer_instance.mov, MOV_OBJECT_G711u, 1, 16, 8000, NULL, 0);
                }
                mp4_writer_init_segment(mp4_instance->muxer_instance.mov);
            }
            track->pts = frame_data->time_stamp / 1000;
            mp4_writer_write(mp4_instance->muxer_instance.mov, track->add_to_mp4, s_buffer, n, track->pts, track->pts, 1 == vcl ? MOV_AV_FLAG_KEYFREAME : 0);
        }
    } else if (track->track_type == K_MP4_STREAM_AUDIO) {
        if (frame_data->codec_id == K_MP4_CODEC_ID_G711A) {
            if (track->add_to_mp4 < 0) {
                int channels = track->audio_info.channels;
                int sample_rate = track->audio_info.sample_rate;
                int bit_per_sample = track->audio_info.bit_per_sample;
                track->add_to_mp4 = mp4_writer_add_audio(mp4_instance->muxer_instance.mov, MOV_OBJECT_G711a, channels, bit_per_sample, sample_rate, NULL, 0);
            }

            track->pts = frame_data->time_stamp / 1000;
            mp4_writer_write(mp4_instance->muxer_instance.mov, track->add_to_mp4, frame_data->data, frame_data->data_length, track->pts, track->pts, 0);
        } else if (frame_data->codec_id == K_MP4_CODEC_ID_G711U) {
            if (track->add_to_mp4 < 0) {
                int channels = track->audio_info.channels;
                int sample_rate = track->audio_info.sample_rate;
                int bit_per_sample = track->audio_info.bit_per_sample;
                track->add_to_mp4 = mp4_writer_add_audio(mp4_instance->muxer_instance.mov, MOV_OBJECT_G711u, channels, bit_per_sample, sample_rate, NULL, 0);
            }

            track->pts = frame_data->time_stamp / 1000;
            mp4_writer_write(mp4_instance->muxer_instance.mov, track->add_to_mp4, frame_data->data, frame_data->data_length, track->pts, track->pts, 0);
        }
    }

    return 0;
}

int kd_mp4_get_file_info(KD_HANDLE mp4_handle, k_mp4_file_info_s *file_info) {
    if (file_info) {
        file_info->duration = 0;
        file_info->track_num = 0;
    }

    if (mp4_handle == NULL || file_info == NULL) {
        printf("kd_mp4_get_file_info: mp4 handle or file_info is null.\n");
        return -1;
    }

    k_mp4_instance *mp4_instance = (k_mp4_instance *)mp4_handle;

    if (mp4_instance->instance_type != K_MP4_CONFIG_DEMUXER) {
        printf("kd_mp4_get_file_info: this interface is not support muxer.\n");
        return -1;
    }

    mov_reader_t *reader = mp4_instance->demuxer_instance.mov;
    file_info->duration = mov_reader_getduration(reader);
    file_info->track_num = mov_reader_gettrackcount(reader);

    return 0;
}

int kd_mp4_get_track_by_index(KD_HANDLE mp4_handle, uint32_t index, k_mp4_track_info_s *mp4_track_info) {
    if (mp4_handle == NULL || mp4_track_info == NULL) {
        printf("kd_mp4_get_track_by_index: mp4 handle or mp4_track_info is null.\n");
        return -1;
    }

    k_mp4_instance *mp4_instance = (k_mp4_instance *)mp4_handle;

    struct mov_reader_t *reader = mp4_instance->demuxer_instance.mov;

    uint32_t track_num = mov_reader_gettrackcount(reader);
    if (index < 0 || index > track_num) {
        printf("k_mp4_get_track_by_index: the track index: %d is invalid.\n", index);
        return -1;
    }

    struct mov_reader_trackinfo_t info = {mov_video_info, mov_audio_info, NULL};
    mov_reader_getinfo(reader, index, &info, mp4_track_info);

    return 0;
}

int kd_mp4_get_frame(KD_HANDLE mp4_handle, k_mp4_frame_data_s *frame_data) {
    if (mp4_handle == NULL || frame_data == NULL) {
        printf("kd_mp4_get_frame: mp4 handle or frame_data is null.\n");
        return -1;
    }

    k_mp4_instance *mp4_instance = (k_mp4_instance *)mp4_handle;
    if (mp4_instance->instance_type == K_MP4_CONFIG_MUXER) {
        printf("kd_mp4_get_frame: this interface is not support muxer.\n");
        return -1;
    }

    uint8_t *s_buffer = mp4_instance->buffer;
    size_t s_buffer_size = mp4_instance->buffer_size;
    k_mp4_packet_t pkt;
    memset(&pkt, 0, sizeof(pkt));
    pkt.ptr = s_buffer;
    pkt.bytes = s_buffer_size;
    int ret = mov_reader_read2(mp4_instance->demuxer_instance.mov, onalloc, &pkt);
    if (ret < 0) {
        printf("kd_mp4_get_frame: get packet failed.\n");
        return -1;
    }
    if (ret == 0) {
        printf("kd_mp4_get_frame: demuxer has finished.\n");
        frame_data->eof = 1;
        return ret;
    }

    uint8_t *s_packet = mp4_instance->buffer2;
    size_t s_packet_size = mp4_instance->buffer2_size;
    uint32_t object_id = mov_reader_objectid(mp4_instance->demuxer_instance.mov, pkt.track_id);
    if (object_id == MOV_OBJECT_H264) {
        assert(h264_is_new_access_unit((const uint8_t*)pkt.ptr + 4, pkt.bytes - 4));
        uint32_t n = h264_mp4toannexb(&mp4_instance->demuxer_instance.track_ctx.s_avc, pkt.ptr, pkt.bytes, s_packet, s_packet_size);
        frame_data->data = s_packet;
        frame_data->data_length = n;
        frame_data->time_stamp = pkt.pts;
        frame_data->codec_id = K_MP4_CODEC_ID_H264;
    } else if (object_id == MOV_OBJECT_H265) {
        assert(h265_is_new_access_unit((const uint8_t*)pkt.ptr + 4, pkt.bytes - 4));
        uint32_t n = h265_mp4toannexb(&mp4_instance->demuxer_instance.track_ctx.s_hevc, pkt.ptr, pkt.bytes, s_packet, s_packet_size);
        frame_data->data = s_packet;
        frame_data->data_length = n;
        frame_data->time_stamp = pkt.pts;
        frame_data->codec_id = K_MP4_CODEC_ID_H265;
    } else if (object_id == MOV_OBJECT_G711a) {
        frame_data->codec_id = K_MP4_CODEC_ID_G711A;
    #if 0
        frame_data->data = (uint8_t*)pkt.ptr + 2;
        frame_data->data_length = pkt.bytes - 2;
    #else
        frame_data->data = (uint8_t*)pkt.ptr;
        frame_data->data_length = pkt.bytes;
    #endif
        frame_data->time_stamp = pkt.pts;
    } else if (object_id == MOV_OBJECT_G711u) {
        frame_data->codec_id = K_MP4_CODEC_ID_G711U;
    #if 0
        frame_data->data = (uint8_t*)pkt.ptr + 2;
        frame_data->data_length = pkt.bytes - 2;
    #else
        frame_data->data = (uint8_t*)pkt.ptr;
        frame_data->data_length = pkt.bytes;
    #endif
        frame_data->time_stamp = pkt.pts;
    }

    return 0;
}
