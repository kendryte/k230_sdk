#ifndef __AUDIO_SAVE_FILE_H__
#define __AUDIO_SAVE_FILE_H__

#ifdef __cplusplus
extern "C" {
#endif /* end of #ifdef __cplusplus */

#include "k_type.h"
#include "k_audio_comm.h"

k_s32 audio_save_init(const char* filename,int sample_rate, int channel_count, k_audio_bit_width bit_width, int total_sec); //保存数据初始化
k_s32 audio_save_pcm_memory(k_u8 *data, int size); //保存音频数据到内存



#ifdef __cplusplus
}
#endif /* end of #ifdef __cplusplus */

#endif

