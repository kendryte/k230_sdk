#include <stdio.h>
#include <stdio.h>
#include <sys/mman.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <pthread.h>
#include <semaphore.h>
#include <stdlib.h>
#include <errno.h>
#include "player_res.h"
#include "mapi_vdec_api.h"
#include "mapi_sys_api.h"
#include "mapi_ao_api.h"
#include "mapi_adec_api.h"
#include "display_cfg.h"

#define INVALID_AO_HANDEL 0xffff
static k_handle g_ao_hdl = INVALID_AO_HANDEL;
static k_handle g_adec_hdl = 0;
#define AUDIO_PERSEC_DIV_NUM 25

#define VDEC_MAX_WIDTH 1920
#define VDEC_MAX_HEIGHT 1088
#define OUTPUT_BUF_CNT 6
#define INPUT_BUF_CNT 4
#define STREAM_BUF_SIZE VDEC_MAX_WIDTH * VDEC_MAX_HEIGHT
#define FRAME_BUF_SIZE VDEC_MAX_WIDTH *VDEC_MAX_HEIGHT * 2


#define BIND_VO_LAYER 1
static k_u32 g_max_sample_rate = 48000;

static k_audio_stream g_audio_stream;
static k_u32 g_audio_pool_id;
static int g_enc_frame_len = 0;
static k_vdec_stream g_video_stream;

typedef struct
{
    k_pixel_format chn_format;
    k_u32 file_size;
    k_s32 pool_id;
    pthread_t input_tid;
    pthread_t config_vo_tid;
    sem_t sem_vdec_done;
    FILE *input_file;
    k_u32 ch_id;
    char *dump_frame;
    k_u32 dump_frame_size;
    k_bool done;
    k_payload_type type;
    k_vb_blk_handle vb_handle[INPUT_BUF_CNT];
    k_u32 act_width;
    k_u32 act_height;
    k_u32 input_pool_id;
    k_u32 output_pool_id;
} sample_vdec_conf_t;

static sample_vdec_conf_t g_vdec_conf[VDEC_MAX_CHN_NUMS];

static k_s32 _vdec_vb_create_pool(int ch)
{
    k_vb_pool_config pool_config;
    memset(&pool_config, 0, sizeof(pool_config));
    pool_config.blk_cnt = INPUT_BUF_CNT;
    pool_config.blk_size = STREAM_BUF_SIZE;
    pool_config.mode = VB_REMAP_MODE_NOCACHE;
    g_vdec_conf[ch].input_pool_id = kd_mapi_vb_create_pool(&pool_config);
    printf("input_pool_id %d\n", g_vdec_conf[ch].input_pool_id);

    memset(&pool_config, 0, sizeof(pool_config));
    pool_config.blk_cnt = OUTPUT_BUF_CNT;
    pool_config.blk_size = FRAME_BUF_SIZE;
    pool_config.mode = VB_REMAP_MODE_NOCACHE;
    g_vdec_conf[ch].output_pool_id = kd_mapi_vb_create_pool(&pool_config);
    printf("output_pool_id %d\n", g_vdec_conf[ch].output_pool_id);

    return 0;
}

static k_s32 vb_destory_pool(int ch)
{
    //printf("destory_pool input %d \n", g_vdec_conf[ch].input_pool_id);
    kd_mapi_vb_destory_pool(g_vdec_conf[ch].input_pool_id);
    //printf("destory_pool output %d \n", g_vdec_conf[ch].output_pool_id);
    kd_mapi_vb_destory_pool(g_vdec_conf[ch].output_pool_id);

    return 0;
}

k_s32 sys_init(k_bool init_vo)
{
    k_s32 ret;
    k_mapi_media_attr_t media_attr;

    if (init_vo)
    {
        vo_init();
    }

    memset(&media_attr, 0, sizeof(k_mapi_media_attr_t));
    k_vb_config *config = &media_attr.media_config.vb_config;

    memset(config, 0, sizeof(*config));
    config->max_pool_cnt = 4;

    config->comm_pool[0].blk_cnt = 150;
    config->comm_pool[0].blk_size = g_max_sample_rate * 2 * 4 / AUDIO_PERSEC_DIV_NUM;
    config->comm_pool[0].mode =  VB_REMAP_MODE_CACHED ;

    config->comm_pool[1].blk_cnt = 2;
    config->comm_pool[1].blk_size = g_max_sample_rate * 2 * 4 / AUDIO_PERSEC_DIV_NUM * 2; // ao use
    config->comm_pool[1].mode = VB_REMAP_MODE_CACHED ;

    ret = kd_mapi_media_init(&media_attr);
    if (ret != K_SUCCESS)
    {
        printf("kd_mapi_media_init error: %x\n", ret);
        return K_FAILED;
    }



    return K_SUCCESS;
}

k_s32 sys_deinit(k_bool deinit_vo)
{
    k_s32 ret;

    ret = kd_mapi_media_deinit();
    if (ret != K_SUCCESS)
    {
        printf("kd_mapi_media_deinit error: %x\n", ret);
        return K_FAILED;
    }

#if 0
    ret = kd_mapi_sys_deinit();
    if (ret != K_SUCCESS)
    {
        printf("kd_mapi_sys_deinit error: %x\n", ret);
        return K_FAILED;
    }
#endif

    if (deinit_vo)
    {
        vo_deinit();
    }

    return K_SUCCESS;
}

static k_s32 _initvo(k_u32 width,k_u32 height,k_u32 vdec_chn)
{
    //init vo
    display_layer_init(width,height);

    if (K_SUCCESS != kd_mapi_vdec_bind_vo(vdec_chn, 0, BIND_VO_LAYER))
    {
        printf("kd_mapi_vdec_bind_vo failed\n");
        return K_FAILED;
    }

    return K_SUCCESS;
}

k_s32 disp_open(k_payload_type video_dec_type,k_u32 width,k_u32 height)
{

    if (_vdec_vb_create_pool(0) != K_SUCCESS)
    {
        printf("_vb_create_pool error\n");
        return K_FAILED;
    }

    int ch = 0;
    k_vdec_chn_attr attr;
    attr.pic_width = VDEC_MAX_WIDTH;
    attr.pic_height = VDEC_MAX_HEIGHT;
    attr.frame_buf_cnt = OUTPUT_BUF_CNT;
    attr.frame_buf_size = FRAME_BUF_SIZE;
    attr.stream_buf_size = STREAM_BUF_SIZE;
    attr.type = video_dec_type;
    attr.frame_buf_pool_id = g_vdec_conf[ch].output_pool_id;

    if (K_SUCCESS != kd_mapi_vdec_init(ch, &attr))
    {
        printf("kd_mapi_ao_init failed\n");
        return K_FAILED;
    }

    if (K_SUCCESS != kd_mapi_vdec_start(ch))
    {
        printf("kd_mapi_vdec_start failed\n");
        return K_FAILED;
    }

    _initvo(width,height,ch);

    return K_SUCCESS;
}


static k_s32 g_mmap_fd_tmp = 0;
static void *_sys_mmap(k_u64 phys_addr, k_u32 size)
{
    void *virt_addr = NULL;
    void *mmap_addr = NULL;
    k_u32 page_size = sysconf(_SC_PAGESIZE);
    k_u64 page_mask = (page_size - 1);
    k_u32 mmap_size = ((size) + (phys_addr & page_mask) + page_mask) & ~(page_mask);

    if (g_mmap_fd_tmp == 0)
    {
        g_mmap_fd_tmp = open("/dev/mem", O_RDWR | O_SYNC);
    }

    mmap_addr = mmap(NULL, mmap_size, PROT_READ | PROT_WRITE, MAP_SHARED, g_mmap_fd_tmp, phys_addr & ~page_mask);

    if (mmap_addr != (void *)-1)
        virt_addr = (void *)((char *)mmap_addr + (phys_addr & page_mask));
    else
    {
        printf("mmap addr error: %d %s.\n", mmap_addr, strerror(errno));
    }

    return virt_addr;
}

static k_s32 _sys_munmap(k_u64 phy_addr, void *virt_addr, k_u32 size)
{
    if (g_mmap_fd_tmp == 0)
    {
        return -1;
    }
    k_u32 ret;

    k_u32 page_size = sysconf(_SC_PAGESIZE);
    k_u64 page_mask = page_size - 1;
    k_u32 mmap_size = ((size) + (phy_addr & page_mask) + page_mask) & ~(page_mask);
    ret = munmap((void *)((k_u64)(virt_addr) & ~page_mask), mmap_size);
    if (ret == -1)
    {
        printf("munmap error.\n");
    }

    return 0;
}

k_s32 disp_play(k_u8*pdata,k_u32 len,k_u64 timestamp,k_bool end_stream)
{
    k_s32 ret;
    sample_vdec_conf_t *vdec_conf;
    k_vdec_stream stream;
    vdec_conf = &g_vdec_conf[0];
    int poolid = vdec_conf->input_pool_id;
    k_u64 phys_addr = 0;
    k_u8 *virt_addr;
    k_u32 blk_size;

    blk_size = STREAM_BUF_SIZE;

    while(1)
    {
        ret = kd_mapi_sys_get_vb_block_from_pool_id(poolid, &phys_addr, blk_size, NULL);
        if (K_SUCCESS != ret)
        {
            usleep(30000);
            continue;
        }
        break;
    }

    vdec_conf->pool_id = poolid;
    virt_addr = (unsigned char*)_sys_mmap(phys_addr, blk_size);

    memcpy(virt_addr,pdata,len);
    stream.phy_addr = phys_addr;
    stream.len = len;
    stream.pts = timestamp;
    stream.end_of_stream = end_stream;

    ret = kd_mapi_vdec_send_stream(0, &stream, -1);
    if (K_SUCCESS != ret)
    {
        printf("kd_mapi_vdec_send_stream failed\n");
        return K_FAILED;
    }
    _sys_munmap(phys_addr, virt_addr, blk_size);
    ret = kd_mapi_sys_release_vb_block(phys_addr, blk_size);
    if (K_SUCCESS != ret)
    {
        printf("kd_mapi_sys_release_vb_block failed\n");
        return K_FAILED;
    }

    if(end_stream)
    {
        // check vdec over
        k_vdec_chn_status status;
        while (1)
        {
            ret = kd_mapi_vdec_query_status(vdec_conf->ch_id, &status);
            if (K_SUCCESS != ret)
            {
                printf("kd_mapi_vdec_query_status failed\n");
                break;
            }

            if (status.end_of_stream)
            {
                printf("%s>ch %d, receive eos\n", __func__, vdec_conf->ch_id);
                break;
            }
            usleep(100);
        }
    }

    return K_SUCCESS;
}

k_s32 disp_close()
{
    int ch = 0;
    if (K_SUCCESS != kd_mapi_vdec_unbind_vo(ch, 0, BIND_VO_LAYER))
    {
        printf("kd_mapi_vdec_unbind_vo failed\n");
        return K_FAILED;
    }

    if (K_SUCCESS != kd_mapi_vdec_stop(ch))
    {
        printf("kd_mapi_vdec_stop failed\n");
        return K_FAILED;
    }

    if (K_SUCCESS != kd_mapi_vdec_deinit(ch))
    {
        printf("kd_mapi_vdec_stop failed\n");
        return K_FAILED;
    }

    display_layer_deinit();

    if (vb_destory_pool(0) != K_SUCCESS)
    {
        printf("vb_destory_pool error");
        return K_FAILED;
    }

    return K_SUCCESS;
}

k_s32 ao_open(k_s32 s32SampleRate, k_s32 s32ChanNum,k_payload_type audio_dec_type, k_bool avsync)
{
    if (g_ao_hdl != INVALID_AO_HANDEL)
    {
        printf("ao already open\n");
        return K_FAILED;
    }
    k_aio_dev_attr aio_dev_attr;
    aio_dev_attr.audio_type = KD_AUDIO_OUTPUT_TYPE_I2S;
    aio_dev_attr.avsync = avsync;
    aio_dev_attr.kd_audio_attr.i2s_attr.sample_rate = s32SampleRate;
    aio_dev_attr.kd_audio_attr.i2s_attr.bit_width = KD_AUDIO_BIT_WIDTH_16;
    aio_dev_attr.kd_audio_attr.i2s_attr.chn_cnt = 2;
    aio_dev_attr.kd_audio_attr.i2s_attr.snd_mode = (1==s32ChanNum)?KD_AUDIO_SOUND_MODE_MONO:KD_AUDIO_SOUND_MODE_STEREO;
    aio_dev_attr.kd_audio_attr.i2s_attr.i2s_mode = K_RIGHT_JUSTIFYING_MODE;
    aio_dev_attr.kd_audio_attr.i2s_attr.frame_num = 5;//AUDIO_PERSEC_DIV_NUM;
    aio_dev_attr.kd_audio_attr.i2s_attr.point_num_per_frame = s32SampleRate / AUDIO_PERSEC_DIV_NUM;
    aio_dev_attr.kd_audio_attr.i2s_attr.i2s_type =  K_AIO_I2STYPE_INNERCODEC;

    if (K_SUCCESS != kd_mapi_ao_init(0,0, &aio_dev_attr,&g_ao_hdl))
    {
        printf("kd_mapi_ao_init failed\n");
        return K_FAILED;
    }

    if (K_SUCCESS != kd_mapi_ao_start(g_ao_hdl))
    {
        printf("kd_mapi_ao_start failed\n");
        return K_FAILED;
    }

    k_adec_chn_attr adec_chn_attr;
    adec_chn_attr.type = audio_dec_type;
    adec_chn_attr.buf_size = AUDIO_PERSEC_DIV_NUM;
    adec_chn_attr.point_num_per_frame = s32SampleRate / adec_chn_attr.buf_size;
    adec_chn_attr.mode = K_ADEC_MODE_PACK;

    if (K_SUCCESS != kd_mapi_adec_init(g_adec_hdl, &adec_chn_attr))
    {
        printf("kd_mapi_adec_init faild\n");
        return K_FAILED;
    }

    if (K_SUCCESS != kd_mapi_adec_start(g_adec_hdl))
    {
        printf("kd_mapi_adec_start faild\n");
        return K_FAILED;
    }

    if(K_SUCCESS != kd_mapi_adec_bind_ao(g_ao_hdl,g_adec_hdl)) {
        printf("kd_mapi_adec_bind_ao failed\n");
        return K_FAILED;
    }

    g_enc_frame_len = s32SampleRate * 2 * 2 / AUDIO_PERSEC_DIV_NUM / 2;
    kd_mapi_sys_get_vb_block(&g_audio_pool_id, &g_audio_stream.phys_addr, g_enc_frame_len, NULL);
    g_audio_stream.stream = _sys_mmap(g_audio_stream.phys_addr,g_enc_frame_len);

    return K_SUCCESS;
}

k_s32 ao_play(k_u8*pdata,k_u32 len,k_u64 timestamp)
{
    memcpy(g_audio_stream.stream, pdata, len);
    g_audio_stream.seq++;
    g_audio_stream.time_stamp = timestamp;
    g_audio_stream.len = len;

    return kd_mapi_adec_send_stream(g_adec_hdl,&g_audio_stream);
}

k_s32 ao_close()
{
    if (g_ao_hdl == INVALID_AO_HANDEL)
    {
        printf("ao already close\n");
        return K_FAILED;
    }

    if (K_SUCCESS !=kd_mapi_ao_stop(g_ao_hdl))
    {
        printf("kd_mapi_ao_stop failed\n");
        return K_FAILED;
    }

    if (K_SUCCESS !=kd_mapi_adec_stop(g_adec_hdl))
    {
        printf("kd_mapi_adec_stop failed\n");
        return K_FAILED;
    }

    if(K_SUCCESS != kd_mapi_adec_unbind_ao(g_ao_hdl,g_adec_hdl)) {
        printf("kd_mapi_adec_unbind_ao failed\n");
        return K_FAILED;
    }

    if (K_SUCCESS != kd_mapi_ao_deinit(g_ao_hdl))
    {
        printf("kd_mapi_ao_deinit failed\n");
        return K_FAILED;
    }

    if (K_SUCCESS != kd_mapi_adec_deinit(g_adec_hdl))
    {
        printf("kd_mapi_adec_deinit failed\n");
        return K_FAILED;
    }

    kd_mapi_sys_release_vb_block(g_audio_stream.phys_addr, g_enc_frame_len);
    _sys_munmap(g_audio_stream.phys_addr,g_audio_stream.stream,g_enc_frame_len);

    g_ao_hdl = INVALID_AO_HANDEL;
    return K_SUCCESS;
}

