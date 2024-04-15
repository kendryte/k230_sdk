#ifndef __AUDIO_WAV_CTRL_H__
#define __AUDIO_WAV_CTRL_H__

#ifdef __cplusplus
extern "C" {
#endif /* end of #ifdef __cplusplus */

#include "k_type.h"
#include "k_audio_comm.h"

typedef struct WAV_RIFF
{
    k_s8 RIFF_id[4];
    k_u32 RIFF_size;
    k_s8 RIFF_type[4];
} RIFF_t;

typedef struct WAV_FMT
{
    k_s8 FMT_id[4];
    k_u32 FMT_size;
    k_u16 FMT_auioFormat;
    k_u16 FMT_numChannels;
    k_u32 FMT_sampleRate;
    k_u32 FMT_byteRate;
    k_u16 FMT_blockAlign;
    k_u16 FMT_bitsPerSample;
} FMT_t;

typedef struct WAV_data
{
    k_s8 DATA_id[4];
    k_u32 DATA_size;
} Data_t;


typedef struct WAV_fotmat
{
    RIFF_t riff;
    FMT_t fmt;
    Data_t data;
} WAV;

void  set_wav_head(int channel,int samplerate,int bitpersample,int total_sample_size,k_u8  *audio_header);
k_s32 load_wav_head(const char* filename,int* channel,int* samplerate,int* bitpersample,int* total_sample_size);
int   read_wav_header(const char *filename, int *header_len, int *data_len,int* channel,int* samplerate,int* bitpersample);
#ifdef __cplusplus
}
#endif /* end of #ifdef __cplusplus */

#endif
