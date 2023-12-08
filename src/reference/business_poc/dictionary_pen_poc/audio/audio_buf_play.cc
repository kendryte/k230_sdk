#include "audio_buf_play.h"
#include <stdio.h>
#include <pthread.h>
#include <time.h>
#include <string.h>
#include "k_vb_comm.h"
#include "k_sys_comm.h"
#include "mpi_vb_api.h"
#include "mpi_sys_api.h"
#include "mpi_ao_api.h"

/* According to earlier standards */
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>

#define AUDIO_PERSEC_DIV_NUM 25

static k_vb_blk_handle g_audio_handle;
static k_s32 _get_audio_frame(k_audio_frame *audio_frame, int nSize)
{
    g_audio_handle = kd_mpi_vb_get_block(VB_INVALID_POOLID, nSize, NULL);
    if (g_audio_handle == VB_INVALID_HANDLE)
    {
        printf("%s get vb block error\n", __func__);
        return K_FAILED;
    }
    audio_frame->len = nSize;
    audio_frame->pool_id = kd_mpi_vb_handle_to_pool_id(g_audio_handle);
    audio_frame->phys_addr = kd_mpi_vb_handle_to_phyaddr(g_audio_handle);
    audio_frame->virt_addr = kd_mpi_sys_mmap(audio_frame->phys_addr, nSize);
    printf("=======_get_audio_frame virt_addr:%p\n", audio_frame->virt_addr);

    return K_SUCCESS;
}

static k_s32 _release_audio_frame()
{
    kd_mpi_vb_release_block(g_audio_handle);
    return K_SUCCESS;
}

// static k_bool g_vb_init = K_FALSE;
// k_s32 audio_buffer_sample_vb_init(k_bool enable_cache, k_u32 sample_rate)
// {
//     if (g_vb_init)
//     {
//         return K_SUCCESS;
//     }
//     k_s32 ret;
//     k_vb_config config;

//     memset(&config, 0, sizeof(config));
//     config.max_pool_cnt = 64;

//     config.comm_pool[0].blk_cnt = 50;
//     config.comm_pool[0].blk_size = sample_rate * 2 * 4 / AUDIO_PERSEC_DIV_NUM;
//     config.comm_pool[0].mode = enable_cache ? VB_REMAP_MODE_CACHED : VB_REMAP_MODE_NOCACHE;

//     config.comm_pool[1].blk_cnt = 2;
//     config.comm_pool[1].blk_size = sample_rate * 2 * 4 / AUDIO_PERSEC_DIV_NUM * 2; // ao use
//     config.comm_pool[1].mode = enable_cache ? VB_REMAP_MODE_CACHED : VB_REMAP_MODE_NOCACHE;

//     int blk_total_size = 0;
//     for (int i = 0; i < 2; i++)
//     {
//         blk_total_size += config.comm_pool[i].blk_cnt * config.comm_pool[i].blk_size;
//     }
//     printf("mmz blk total size:%.2f MB\n", blk_total_size / 1024 / 1024.0);

//     ret = kd_mpi_vb_set_config(&config);
//     if (ret)
//     {
//         printf("vb_set_config failed ret:%d\n", ret);
//         return ret;
//     }
//     else
//     {
//         printf("vb_set_config ok\n");
//     }

//     ret = kd_mpi_vb_init();

//     if (ret)
//         printf("vb_init failed ret:%d\n", ret);
//     else
//         g_vb_init = K_TRUE;

//     return ret;
// }

k_s32 audio_buffer_sample_vb_destroy()
{
    // if (!g_vb_init)
    // {
    //     return K_FAILED;
    // }
    // g_vb_init = K_FALSE;

    k_s32 ret;
    ret = kd_mpi_vb_exit();
    if (ret)
        printf("vb_exit failed ret:%d\n", ret);
    return ret;
}

static k_audio_frame g_play_audio_frame;
static k_u32 g_channel_count;
static k_u32 g_sample_rate;

k_s32 audio_buffer_play_init(k_u32 sample_rate,k_u32 channel_count)
{

    _get_audio_frame(&g_play_audio_frame, sample_rate * 2 * 2 / AUDIO_PERSEC_DIV_NUM);
    g_channel_count = channel_count;
    g_sample_rate = sample_rate;

    k_aio_dev_attr ao_dev_attr;
    ao_dev_attr.audio_type = KD_AUDIO_OUTPUT_TYPE_I2S;
    ao_dev_attr.kd_audio_attr.i2s_attr.sample_rate = sample_rate;
    ao_dev_attr.kd_audio_attr.i2s_attr.bit_width = KD_AUDIO_BIT_WIDTH_16;
    ao_dev_attr.kd_audio_attr.i2s_attr.chn_cnt = 2;
    ao_dev_attr.kd_audio_attr.i2s_attr.i2s_mode = K_STANDARD_MODE;
    ao_dev_attr.kd_audio_attr.i2s_attr.snd_mode = (1 == channel_count) ? KD_AUDIO_SOUND_MODE_MONO : KD_AUDIO_SOUND_MODE_STEREO;
    ao_dev_attr.kd_audio_attr.i2s_attr.frame_num = AUDIO_PERSEC_DIV_NUM;
    ao_dev_attr.kd_audio_attr.i2s_attr.point_num_per_frame = ao_dev_attr.kd_audio_attr.i2s_attr.sample_rate / ao_dev_attr.kd_audio_attr.i2s_attr.frame_num;
    ao_dev_attr.kd_audio_attr.i2s_attr.i2s_type = K_AIO_I2STYPE_INNERCODEC;

    kd_mpi_ao_set_pub_attr(0, &ao_dev_attr);

    kd_mpi_ao_enable(0);
    kd_mpi_ao_enable_chn(0, 0);

    return 0;
}

k_s32 audio_buffer_play(k_u8 *pdata, k_u32 data_len)
{
    k_u8 *pDataBuf = (k_u8 *)g_play_audio_frame.virt_addr;
    int frame_size = g_sample_rate*g_channel_count*2/AUDIO_PERSEC_DIV_NUM;//16 bit
    g_play_audio_frame.len = frame_size;
    int frame_count = data_len / frame_size;
    for (int i=0;i < frame_count; i ++)
    {
        memcpy(pDataBuf,pdata+i*frame_size,frame_size);
        if (0 != kd_mpi_ao_send_frame(0, 0, &g_play_audio_frame, 1000))
        {
            printf("kd_mpi_ao_send_frame failed\n");
            return -1;
        }
    }

    return 0;
}

k_s32 audio_buffer_play_deinit()
{
    kd_mpi_ao_disable_chn(0, 0);
    kd_mpi_ao_disable(0);
    _release_audio_frame();
    return 0;
}



#define  WAV_HEAD_SIZE 44
void audio_test_file(void)
{
    unsigned char *pcm_file_data = NULL;
    FILE *fp = fopen("audio_test.wav", "rb");
    if (NULL == fp)
    {
        printf("open file:%s failed\n", "audio.wav");
        return ;
    }

    fseek(fp, 0, SEEK_END);
    int size = ftell(fp);
    pcm_file_data = (unsigned char *)malloc(size);
    if (pcm_file_data == NULL)
    {
        printf("malloc size %d failed\n", size);
        return ;
    }

    fseek(fp, 0, SEEK_SET);
    fread(pcm_file_data, size, 1, fp);
    fclose(fp);


    audio_buffer_play(pcm_file_data+WAV_HEAD_SIZE,size-WAV_HEAD_SIZE);
}