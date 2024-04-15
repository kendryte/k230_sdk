#ifndef __AUDIO_PCM_32DATA_H__
#define __AUDIO_PCM_32DATA_H__

#ifdef __cplusplus
extern "C" {
#endif /* end of #ifdef __cplusplus */

#include "k_type.h"
#include "k_audio_comm.h"

k_s32    load_wav_info(const char*filename,int* channel,int* samplerate,int* bitpersample);
k_s32    get_pcm_data_from_file(k_u32 *pdata, k_u32 data_len);

#ifdef __cplusplus
}
#endif /* end of #ifdef __cplusplus */

#endif


