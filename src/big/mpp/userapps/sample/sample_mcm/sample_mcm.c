/* vicap */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/mman.h>
#include <pthread.h>
#include <errno.h>
#include <fcntl.h>
#include <stdint.h>

#include "k_module.h"
#include "k_type.h"
#include "k_vb_comm.h"
#include "k_vicap_comm.h"
#include "k_video_comm.h"
#include "k_sys_comm.h"
#include "mpi_vb_api.h"
#include "mpi_vicap_api.h"
#include "mpi_isp_api.h"
#include "mpi_sys_api.h"
#include "k_vo_comm.h"
#include "mpi_vo_api.h"
#include "vo_test_case.h"
#include "mpi_nonai_2d_api.h"
#include "k_nonai_2d_comm.h"
#include "k_connector_comm.h"
#include "mpi_connector_api.h"
#include "mpi_venc_api.h"
#include "k_venc_comm.h"

#define DUMP_2D_RGB_OUTPUT          0

#define ISP_CHN0_WIDTH              (1280)
#define ISP_CHN0_HEIGHT             (720)
#define VICAP_OUTPUT_BUF_NUM        10
#define VENC_BUF_NUM                6
#define NONAI_2D_BUF_NUM            6

#define TOTAL_ENABLE_2D_CH_NUMS     6
#define NONAI_2D_RGB_CH             4
#define NONAI_2D_BIND_CH_0          0
#define NONAI_2D_BIND_CH_1          1
#define NONAI_2D_BIND_CH_2          2

static k_u32 g_vo_pool_id;
static k_u8 exit_flag = 0;
static k_bool g_enable_2d=K_TRUE;
static k_vo_layer g_vo_layer = K_VO_LAYER1;

typedef struct {
    k_vicap_dev vicap_dev;
} k_mcm_demo;

typedef struct
{
    k_u32 width;
    k_u32 height;
    pthread_t output_tid;
    char out_filename[50];
    FILE *output_file;
    k_u32 stream_buf_size;
} venc_conf_t;

static venc_conf_t g_venc_conf;

static inline void CHECK_RET(k_s32 ret, const char *func, const int line)
{
    if (ret)
        printf("error ret %d, func %s line %d\n", ret, func, line);
}

static int sample_vb_init(void)
{
    k_s32 ret;
    k_vb_config config;
    k_vb_pool_config pool_config;
    k_vb_supplement_config supplement_config;
    k_u32 pool_id;

    memset(&config, 0, sizeof(config));
    memset(&pool_config, 0, sizeof(pool_config));
    config.max_pool_cnt = 64;

    // for vo install plane data
    config.comm_pool[0].blk_cnt = 5;
    config.comm_pool[0].blk_size = PRIVATE_POLL_SZE;          // osd0 - 3 argb 320 x 240
    config.comm_pool[0].mode = VB_REMAP_MODE_NOCACHE;           //VB_REMAP_MODE_NOCACHE;

    k_u16 sride = ISP_CHN0_WIDTH;
    //VB for YUV444 output for dev0
    config.comm_pool[1].blk_cnt = VICAP_OUTPUT_BUF_NUM;
    config.comm_pool[1].mode = VB_REMAP_MODE_NOCACHE;
    config.comm_pool[1].blk_size = VICAP_ALIGN_UP((sride * ISP_CHN0_HEIGHT * 3), 0x1000);

    //VB for YUV444 output for dev1
    config.comm_pool[2].blk_cnt = VICAP_OUTPUT_BUF_NUM;
    config.comm_pool[2].mode = VB_REMAP_MODE_NOCACHE;
    config.comm_pool[2].blk_size = VICAP_ALIGN_UP((sride * ISP_CHN0_HEIGHT * 3 ), 0x1000);

    //VB for YUV444 output for dev2
    config.comm_pool[3].blk_cnt = VICAP_OUTPUT_BUF_NUM;
    config.comm_pool[3].mode = VB_REMAP_MODE_NOCACHE;
    config.comm_pool[3].blk_size = VICAP_ALIGN_UP((sride * ISP_CHN0_HEIGHT * 3 ), 0x1000);

    //VB for nonai_2d
    config.comm_pool[4].blk_cnt = NONAI_2D_BUF_NUM;
    config.comm_pool[4].mode = VB_REMAP_MODE_NOCACHE;
    config.comm_pool[4].blk_size = VICAP_ALIGN_UP((ISP_CHN0_WIDTH * ISP_CHN0_HEIGHT * 3), 0x1000);

    //VB for venc
    config.comm_pool[5].blk_cnt = VENC_BUF_NUM;
    config.comm_pool[5].mode = VB_REMAP_MODE_NOCACHE;
    config.comm_pool[5].blk_size = VICAP_ALIGN_UP((ISP_CHN0_WIDTH * ISP_CHN0_HEIGHT / 2 ), 0x1000);
    g_venc_conf.stream_buf_size = config.comm_pool[5].blk_size;

    ret = kd_mpi_vb_set_config(&config);
    if (ret) {
        printf("vb_set_config failed ret:%d\n", ret);
        return ret;
    }

    memset(&supplement_config, 0, sizeof(supplement_config));
    supplement_config.supplement_config |= VB_SUPPLEMENT_JPEG_MASK;

    ret = kd_mpi_vb_set_supplement_config(&supplement_config);
    if (ret) {
        printf("vb_set_supplement_config failed ret:%d\n", ret);
        return ret;
    }

    ret = kd_mpi_vb_init();
    if (ret) {
        printf("vb_init failed ret:%d\n", ret);
        return ret;
    }

    memset(&pool_config, 0, sizeof(pool_config));
    pool_config.blk_cnt = PRIVATE_POLL_NUM;
    pool_config.blk_size = PRIVATE_POLL_SZE;
    pool_config.mode = VB_REMAP_MODE_NONE;
    pool_id = kd_mpi_vb_create_pool(&pool_config);          // osd0 - 3 argb 320 x 240

    g_vo_pool_id = pool_id;

    return ret;
}


static int sample_vicap_init(k_vicap_dev dev_chn, k_vicap_sensor_type type)
{
    k_vicap_dev vicap_dev;
    k_vicap_chn vicap_chn;
    k_vicap_dev_attr dev_attr;
    k_vicap_chn_attr chn_attr;
    k_vicap_sensor_info sensor_info;
    k_vicap_sensor_type sensor_type;
    k_s32 ret = 0;

    memset(&dev_attr, 0 ,sizeof(dev_attr));
    memset(&chn_attr, 0 ,sizeof(chn_attr));
    memset(&sensor_info, 0 ,sizeof(sensor_info));

    sensor_type = type;
    vicap_dev = dev_chn;

    memset(&sensor_info, 0, sizeof(k_vicap_sensor_info));
    ret = kd_mpi_vicap_get_sensor_info(sensor_type, &sensor_info);
    if (ret) {
        printf("sample_vicap, the sensor type not supported!\n");
        return ret;
    }

    memset(&dev_attr, 0, sizeof(k_vicap_dev_attr));
    dev_attr.acq_win.h_start = 0;
    dev_attr.acq_win.v_start = 0;
    dev_attr.acq_win.width = ISP_CHN0_WIDTH;
    dev_attr.acq_win.height = ISP_CHN0_WIDTH;
    dev_attr.mode = VICAP_WORK_ONLY_MCM_MODE;
    dev_attr.buffer_num = VICAP_OUTPUT_BUF_NUM;
    dev_attr.buffer_size = VICAP_ALIGN_UP((ISP_CHN0_WIDTH * ISP_CHN0_HEIGHT * 3), VICAP_ALIGN_1K);
    dev_attr.pipe_ctrl.data = 0xFFFFFFFF;
    dev_attr.pipe_ctrl.bits.af_enable = 0;
    dev_attr.pipe_ctrl.bits.ahdr_enable = 0;
    dev_attr.dw_enable = K_FALSE;

    dev_attr.cpature_frame = 0;
    memcpy(&dev_attr.sensor_info, &sensor_info, sizeof(k_vicap_sensor_info));

    ret = kd_mpi_vicap_set_dev_attr(vicap_dev, dev_attr);
    if (ret) {
        printf("sample_vicap, kd_mpi_vicap_set_dev_attr failed.\n");
        return ret;
    }

    memset(&chn_attr, 0, sizeof(k_vicap_chn_attr));

    //set chn0 output yuv444
    chn_attr.out_win.h_start = 0;
    chn_attr.out_win.v_start = 0;
    chn_attr.out_win.width = ISP_CHN0_WIDTH;
    chn_attr.out_win.height = ISP_CHN0_HEIGHT;
    chn_attr.crop_win = dev_attr.acq_win;
    chn_attr.scale_win = chn_attr.out_win;
    chn_attr.crop_enable = K_FALSE;
    chn_attr.scale_enable = K_FALSE;
    // chn_attr.dw_enable = K_FALSE;
    chn_attr.chn_enable = K_TRUE;

    chn_attr.pix_format = PIXEL_FORMAT_YUV_SEMIPLANAR_444;
    chn_attr.buffer_size = VICAP_ALIGN_UP((ISP_CHN0_WIDTH * ISP_CHN0_HEIGHT * 3), VICAP_ALIGN_1K);


    chn_attr.buffer_num = VICAP_OUTPUT_BUF_NUM;//at least 3 buffers for isp
    vicap_chn = VICAP_CHN_ID_0;

    // printf("sample_vicap ...kd_mpi_vicap_set_chn_attr, buffer_size[%d]\n", chn_attr.buffer_size);
    ret = kd_mpi_vicap_set_chn_attr(vicap_dev, vicap_chn, chn_attr);
    if (ret) {
        printf("sample_vicap, kd_mpi_vicap_set_chn_attr failed.\n");
        return ret;
    }

    // printf("sample_vicap ...kd_mpi_vicap_init\n");
    ret = kd_mpi_vicap_init(vicap_dev);
    if (ret) {
        printf("sample_vicap, kd_mpi_vicap_init failed.\n");
        return ret;
    }

    return ret;
}

static k_s32 sample_vicap_stream(k_vicap_dev vicap_dev, k_bool en)
{
    k_s32 ret = 0;
    if(en)
    {
        ret = kd_mpi_vicap_start_stream(vicap_dev);
        if (ret) {
            printf("sample_vicap, kd_mpi_vicap_start_stream failed.\n");
            return ret;
        }
    }
    else
    {
        ret = kd_mpi_vicap_stop_stream(vicap_dev);
        if (ret) {
            printf("sample_vicap, stop stream failed.\n");
            return ret;
        }
        ret = kd_mpi_vicap_deinit(vicap_dev);
        if (ret) {
            printf("sample_vicap, kd_mpi_vicap_deinit failed.\n");
        }
    }
    return ret;
}


static k_s32 sample_connector_init(k_connector_type type)
{
    k_u32 ret = 0;
    k_s32 connector_fd;
    k_connector_type connector_type = type;
	k_connector_info connector_info;

    memset(&connector_info, 0, sizeof(k_connector_info));

    //connector get sensor info
    ret = kd_mpi_get_connector_info(connector_type, &connector_info);
    if (ret) {
        printf("sample_vicap, the sensor type not supported!\n");
        return ret;
    }

    connector_fd = kd_mpi_connector_open(connector_info.connector_name);
    if (connector_fd < 0) {
        printf("%s, connector open failed.\n", __func__);
        return K_ERR_VO_NOTREADY;
    }

    // set connect power
    kd_mpi_connector_power_set(connector_fd, K_TRUE);
    // connector init
    kd_mpi_connector_init(connector_fd, connector_info);

    return 0;
}

static k_u32 sample_vo_creat_osd_test(k_vo_osd osd, osd_info *info)
{
    k_vo_video_osd_attr attr;

    // set attr
    attr.global_alptha = info->global_alptha;

    if (info->format == PIXEL_FORMAT_ABGR_8888 || info->format == PIXEL_FORMAT_ARGB_8888)
    {
        info->size = info->act_size.width  * info->act_size.height * 4;
        info->stride  = info->act_size.width * 4 / 8;
    }
    else if (info->format == PIXEL_FORMAT_RGB_565 || info->format == PIXEL_FORMAT_BGR_565)
    {
        info->size = info->act_size.width  * info->act_size.height * 2;
        info->stride  = info->act_size.width * 2 / 8;
    }
    else if (info->format == PIXEL_FORMAT_RGB_888 || info->format == PIXEL_FORMAT_BGR_888)
    {
        info->size = info->act_size.width  * info->act_size.height * 3;
        info->stride  = info->act_size.width * 3 / 8;
    }
    else if(info->format == PIXEL_FORMAT_ARGB_4444 || info->format == PIXEL_FORMAT_ABGR_4444)
    {
        info->size = info->act_size.width  * info->act_size.height * 2;
        info->stride  = info->act_size.width * 2 / 8;
    }
    else if(info->format == PIXEL_FORMAT_ARGB_1555 || info->format == PIXEL_FORMAT_ABGR_1555)
    {
        info->size = info->act_size.width  * info->act_size.height * 2;
        info->stride  = info->act_size.width * 2 / 8;
    }
    else
    {
        printf("set osd pixel format failed  \n");
    }

    attr.stride = info->stride;
    attr.pixel_format = info->format;
    attr.display_rect = info->offset;
    attr.img_size = info->act_size;
    kd_mpi_vo_set_video_osd_attr(osd, &attr);

    kd_mpi_vo_osd_enable(osd);

    return 0;
}

static k_vb_blk_handle sample_vo_insert_frame(k_video_frame_info *vf_info, void **pic_vaddr)
{
    k_u64 phys_addr = 0;
    k_u32 *virt_addr;
    k_vb_blk_handle handle;
    k_s32 size = 0;

    if (vf_info == NULL)
        return K_FALSE;

    if (vf_info->v_frame.pixel_format == PIXEL_FORMAT_ABGR_8888 || vf_info->v_frame.pixel_format == PIXEL_FORMAT_ARGB_8888)
        size = vf_info->v_frame.height * vf_info->v_frame.width * 4;
    else if (vf_info->v_frame.pixel_format == PIXEL_FORMAT_RGB_565 || vf_info->v_frame.pixel_format == PIXEL_FORMAT_BGR_565)
        size = vf_info->v_frame.height * vf_info->v_frame.width * 2;
    else if (vf_info->v_frame.pixel_format == PIXEL_FORMAT_ABGR_4444 || vf_info->v_frame.pixel_format == PIXEL_FORMAT_ARGB_4444)
        size = vf_info->v_frame.height * vf_info->v_frame.width * 2;
    else if (vf_info->v_frame.pixel_format == PIXEL_FORMAT_RGB_888 || vf_info->v_frame.pixel_format == PIXEL_FORMAT_BGR_888)
        size = vf_info->v_frame.height * vf_info->v_frame.width * 3;
    else if (vf_info->v_frame.pixel_format == PIXEL_FORMAT_ARGB_1555 || vf_info->v_frame.pixel_format == PIXEL_FORMAT_ABGR_1555)
        size = vf_info->v_frame.height * vf_info->v_frame.width * 2;
    else if (vf_info->v_frame.pixel_format == PIXEL_FORMAT_YVU_PLANAR_420)
        size = vf_info->v_frame.height * vf_info->v_frame.width * 3 / 2;

    size = size + 4096;         // 强制4K ，后边得删了

    printf("vb block size is %x \n", size);

    handle = kd_mpi_vb_get_block(g_vo_pool_id, size, NULL);
    if (handle == VB_INVALID_HANDLE)
    {
        printf("%s get vb block error\n", __func__);
        return K_FAILED;
    }

    phys_addr = kd_mpi_vb_handle_to_phyaddr(handle);
    if (phys_addr == 0)
    {
        printf("%s get phys addr error\n", __func__);
        return K_FAILED;
    }

    virt_addr = (k_u32 *)kd_mpi_sys_mmap(phys_addr, size);
    // virt_addr = (k_u32 *)kd_mpi_sys_mmap_cached(phys_addr, size);

    if (virt_addr == NULL)
    {
        printf("%s mmap error\n", __func__);
        return K_FAILED;
    }

    vf_info->mod_id = K_ID_VO;
    vf_info->pool_id = g_vo_pool_id;
    vf_info->v_frame.phys_addr[0] = phys_addr;
    if (vf_info->v_frame.pixel_format == PIXEL_FORMAT_YVU_PLANAR_420)
        vf_info->v_frame.phys_addr[1] = phys_addr + (vf_info->v_frame.height * vf_info->v_frame.stride[0]);
    *pic_vaddr = virt_addr;

    printf("phys_addr is %lx \n", phys_addr);

    return handle;
}

static void sample_vo_init(k_connector_type type)
{
    osd_info osd;
    layer_info info;
    k_vo_layer chn_id = K_VO_LAYER1;
    k_vo_osd osd_id = K_VO_OSD0;

    memset(&info, 0, sizeof(info));
    memset(&osd, 0, sizeof(osd));

    sample_connector_init(type);

    if(type == HX8377_V2_MIPI_4LAN_1080X1920_30FPS)
    {
        info.act_size.width = ISP_CHN0_HEIGHT ;//1080;//640;//1080;
        info.act_size.height = ISP_CHN0_WIDTH;//1920;//480;//1920;
        info.format = PIXEL_FORMAT_YVU_PLANAR_420;
        info.func = K_ROTATION_90;////K_ROTATION_90;
        info.global_alptha = 0xff;
        info.offset.x = 0;//(1080-w)/2,
        info.offset.y = 0;//(1920-h)/2;
        vo_creat_layer_test(chn_id, &info);

        osd.act_size.width = ISP_CHN0_WIDTH ;
        osd.act_size.height = ISP_CHN0_HEIGHT;
        osd.offset.x = 0;
        osd.offset.y = 0;
        osd.global_alptha = 0xff;// 0x7f;
        osd.format = PIXEL_FORMAT_RGB_888;
        sample_vo_creat_osd_test(osd_id, &osd);
    }
    else
    {
        info.act_size.width = ISP_CHN0_WIDTH ;//1080;//640;//1080;
        info.act_size.height = ISP_CHN0_HEIGHT;//1920;//480;//1920;
        info.format = PIXEL_FORMAT_YVU_PLANAR_420;
        info.func = 0;////K_ROTATION_90;
        info.global_alptha = 0xff;
        info.offset.x = 0;//(1080-w)/2,
        info.offset.y = 0;//(1920-h)/2;
        vo_creat_layer_test(chn_id, &info);

        osd.act_size.width = ISP_CHN0_WIDTH ;
        osd.act_size.height = ISP_CHN0_HEIGHT;
        osd.offset.x = 0;
        osd.offset.y = 0;
        osd.global_alptha = 0xff;// 0x7f;
        osd.format = PIXEL_FORMAT_RGB_888;
        sample_vo_creat_osd_test(osd_id, &osd);
    }


}

static void YUV444TONV12(unsigned char *inbuf, unsigned char *outbuf, int w, int h)
{
    unsigned char *srcY = NULL, *srcU = NULL, *srcV = NULL;
    unsigned char *desY = NULL, *desU = NULL;


    srcY = inbuf;//Y
    srcU = srcY + 1;//U
    srcV = srcY + 2;//V

    desY = outbuf;
    desU = desY + w * h;

    for(int i=0; i<w*h; i++)
    {
        *desY = *srcY;
        desY++;
        srcY += 3;
    }

    int half_width = w / 2;
    int half_height = h / 2;

    //UV
    for (int i = 0; i < half_height; i++)
    {
        for (int j = 0; j < half_width; j++)
        {
            *desU = *srcU;
            desU++;
            *desU = *srcV;
            desU++;
            srcU += 2*3;
            srcV += 2*3;
        }
        srcU = srcU + w*3;
        srcV = srcV + w*3;
    }
}

static void *sample_mcm_switch_id_thread(void *arg)
{
    k_char select = 0;
    k_mcm_demo *mcm = (k_mcm_demo *)arg;
    while(1)
    {
        select = (k_char)getchar();
        switch (select)
        {
            case '0':
                mcm->vicap_dev = VICAP_CHN_ID_0;
                break;
            case '1':
                mcm->vicap_dev = VICAP_CHN_ID_1;
                break;
            case '2':
                mcm->vicap_dev = VICAP_CHN_ID_2;
                break;
            case 'q':
                exit_flag = 1;
                return NULL;
            default :
                break;
        }
    }
    return NULL;
}
static void nonai_2d_work(k_video_frame_info *frame)
{
    int ret;
    k_video_frame_info rgb_vf_info;
    static k_u32 dump_cnt=0;
    static FILE *output_file=NULL;

    //yuv420 to rgb
    ret = kd_mpi_nonai_2d_send_frame(NONAI_2D_RGB_CH, frame, 1000);
    if (ret) {
        printf("kd_mpi_nonai_2d_send_frame ch 4 failed. %d\n", ret);
        return;
    }
    ret = kd_mpi_nonai_2d_get_frame(NONAI_2D_RGB_CH, &rgb_vf_info, 1000);
    if (ret) {
        printf("kd_mpi_nonai_2d_get_frame ch 4 failed. %d\n", ret);
        return;
    }

    //dump rgb
    if(ret == K_SUCCESS && dump_cnt == 0)
    {
        k_u8 *pData;
        k_u32 size = ISP_CHN0_WIDTH * ISP_CHN0_HEIGHT * 3;
        pData = (k_u8 *)kd_mpi_sys_mmap(rgb_vf_info.v_frame.phys_addr[0], size);
        output_file = fopen("/sharefs/long.rgb", "wb");
        fwrite(pData, 1, size, output_file);
        kd_mpi_sys_munmap(pData, size);
        fclose(output_file);
        dump_cnt++;
        printf("dump 2D rgb output done\n");
    }

    if(ret == K_SUCCESS)
    {
        ret = kd_mpi_nonai_2d_release_frame(NONAI_2D_RGB_CH, &rgb_vf_info);
        if (ret) {
            printf("kd_mpi_nonai_2d_release_frame ch 4 failed. %d\n", ret);
        }
    }
}

static void *sample_mcm_dump_screen_thread(void *arg)
{
    k_s32 ret = 0;
    void *pic_vaddr = NULL;
    void *pic_vaddr3 = NULL;
    k_vo_layer chn_id = K_VO_LAYER1;
    k_video_frame_info vf_info, vf_info3;
    k_vb_blk_handle block, block3;
    k_mcm_demo *mcm = (k_mcm_demo *)arg;

    // default duump ch0 mcm
    mcm->vicap_dev = VICAP_DEV_ID_0;

    memset(&vf_info, 0, sizeof(vf_info));
    memset(&vf_info3, 0, sizeof(vf_info3));

    vf_info.v_frame.width = ISP_CHN0_WIDTH;
    vf_info.v_frame.height = ISP_CHN0_HEIGHT;
    vf_info.v_frame.stride[0] = vf_info.v_frame.width;
    vf_info.v_frame.pixel_format = PIXEL_FORMAT_YVU_PLANAR_420;
    block = sample_vo_insert_frame(&vf_info, &pic_vaddr);

    vf_info3.v_frame.width = ISP_CHN0_WIDTH;
    vf_info3.v_frame.height = ISP_CHN0_HEIGHT;
    vf_info3.v_frame.stride[0] = vf_info.v_frame.width;
    vf_info3.v_frame.pixel_format = PIXEL_FORMAT_YVU_PLANAR_420;
    block3 = sample_vo_insert_frame(&vf_info3, &pic_vaddr3);

    int yuv444_bufLen = ISP_CHN0_WIDTH * ISP_CHN0_HEIGHT * 3;
    unsigned char *yuv444_buff = malloc(yuv444_bufLen * sizeof(unsigned char));

    int vo_frame_cut = 0;
    k_video_frame_info dumm_vf_info;
    k_u8 *dump_virt_addr = NULL;

    printf("start dum mcm buff mcm->vicap_dev is %d \n", mcm->vicap_dev);

    while(exit_flag != 1)
    {
        if(vo_frame_cut == 0)
        {
            ret = kd_mpi_vicap_dump_frame(mcm->vicap_dev, VICAP_CHN_ID_0, VICAP_DUMP_YUV444, &dumm_vf_info, 1000);
            if (ret) {
                printf("sample_vicap, chn(%d) dump frame failed.\n", mcm->vicap_dev);
                continue;
            }

            if(g_enable_2d)
            {
                nonai_2d_work(&dumm_vf_info);
            }
            else
            {
                dump_virt_addr = kd_mpi_sys_mmap(dumm_vf_info.v_frame.phys_addr[0], yuv444_bufLen);

                memcpy(yuv444_buff, dump_virt_addr, yuv444_bufLen);
                YUV444TONV12(yuv444_buff, (unsigned char *)pic_vaddr, ISP_CHN0_WIDTH, ISP_CHN0_HEIGHT);
                kd_mpi_vo_chn_insert_frame(chn_id, &vf_info);  //K_VO_OSD0

                kd_mpi_sys_munmap(dump_virt_addr, yuv444_bufLen);
            }

            kd_mpi_vicap_dump_release(mcm->vicap_dev, VICAP_CHN_ID_0, &dumm_vf_info);
            vo_frame_cut = 1;
        }
        else
        {
            ret = kd_mpi_vicap_dump_frame(mcm->vicap_dev, VICAP_CHN_ID_0, VICAP_DUMP_YUV444, &dumm_vf_info, 1000);
            if (ret) {
                printf("sample_vicap, chn(%d) dump frame failed.\n", mcm->vicap_dev);
                continue;
            }

            if(g_enable_2d)
            {
                nonai_2d_work(&dumm_vf_info);
            }
            else
            {
                dump_virt_addr = kd_mpi_sys_mmap(dumm_vf_info.v_frame.phys_addr[0], yuv444_bufLen);
                memcpy(yuv444_buff, dump_virt_addr, yuv444_bufLen);
                YUV444TONV12(yuv444_buff, (unsigned char *)pic_vaddr3, ISP_CHN0_WIDTH, ISP_CHN0_HEIGHT);
                kd_mpi_vo_chn_insert_frame(chn_id, &vf_info3);  //K_VO_OSD0

                kd_mpi_sys_munmap(dump_virt_addr, yuv444_bufLen);
            }

            kd_mpi_vicap_dump_release(mcm->vicap_dev, VICAP_CHN_ID_0, &dumm_vf_info);
            vo_frame_cut = 0;
        }
    }

    kd_mpi_vb_release_block(block3);
    kd_mpi_vb_release_block(block);
    printf("dump exut success \n");
    return 0 ;
}

static k_s32 nonai_2d_start()
{
    int i, ret;
    k_nonai_2d_chn_attr attr_2d;

    if(!g_enable_2d) return K_SUCCESS;

    for(i = 0; i < TOTAL_ENABLE_2D_CH_NUMS; i++)
    {
        attr_2d.mode = K_NONAI_2D_CALC_MODE_CSC;
        if(i == NONAI_2D_RGB_CH)
        {
            attr_2d.dst_fmt = PIXEL_FORMAT_RGB_888_PLANAR;
        }
        else
        {
            attr_2d.dst_fmt = PIXEL_FORMAT_YUV_SEMIPLANAR_420;
        }
        ret = kd_mpi_nonai_2d_create_chn(i, &attr_2d);
        CHECK_RET(ret, __func__, __LINE__);

        ret = kd_mpi_nonai_2d_start_chn(i);
        CHECK_RET(ret, __func__, __LINE__);
    }

    return K_SUCCESS;
}

static k_s32 nonai_2d_exit()
{
    int ret = 0;
    int i;

    for(i = 0; i < TOTAL_ENABLE_2D_CH_NUMS; i++)
    {
        kd_mpi_nonai_2d_stop_chn(i);
        kd_mpi_nonai_2d_destroy_chn(i);
    }

    ret = kd_mpi_nonai_2d_close();
    CHECK_RET(ret, __func__, __LINE__);

    return K_SUCCESS;
}

static void sample_bind()
{
    k_s32 ret;
    k_mpp_chn vi_mpp_chn;
    k_mpp_chn nonai_2d_mpp_chn;
    k_mpp_chn venc_mpp_chn;

    vi_mpp_chn.mod_id = K_ID_VI;
    vi_mpp_chn.dev_id = 0;
    vi_mpp_chn.chn_id = 0;
    nonai_2d_mpp_chn.mod_id = K_ID_NONAI_2D;
    nonai_2d_mpp_chn.dev_id = 0;
    nonai_2d_mpp_chn.chn_id = NONAI_2D_BIND_CH_0;
    venc_mpp_chn.mod_id = K_ID_VENC;
    venc_mpp_chn.dev_id = 0;
    venc_mpp_chn.chn_id = 0;


    ret = kd_mpi_sys_bind(&vi_mpp_chn, &nonai_2d_mpp_chn);
    CHECK_RET(ret, __func__, __LINE__);

    ret = kd_mpi_sys_bind(&nonai_2d_mpp_chn, &venc_mpp_chn);
    CHECK_RET(ret, __func__, __LINE__);

    k_mpp_chn vo_mpp_chn;
    vo_mpp_chn.mod_id = K_ID_VO;
    vo_mpp_chn.dev_id = 0;
    vo_mpp_chn.chn_id = g_vo_layer;
    ret = kd_mpi_sys_bind(&nonai_2d_mpp_chn, &vo_mpp_chn);
    CHECK_RET(ret, __func__, __LINE__);

    return;
}

static void sample_unbind()
{
    k_s32 ret;
    k_mpp_chn vi_mpp_chn;
    k_mpp_chn nonai_2d_mpp_chn;
    k_mpp_chn venc_mpp_chn;

    vi_mpp_chn.mod_id = K_ID_VI;
    vi_mpp_chn.dev_id = 0;
    vi_mpp_chn.chn_id = 0;
    nonai_2d_mpp_chn.mod_id = K_ID_NONAI_2D;
    nonai_2d_mpp_chn.dev_id = 0;
    nonai_2d_mpp_chn.chn_id = NONAI_2D_BIND_CH_0;
    venc_mpp_chn.mod_id = K_ID_VENC;
    venc_mpp_chn.dev_id = 0;
    venc_mpp_chn.chn_id = 0;

    ret = kd_mpi_sys_unbind(&vi_mpp_chn, &nonai_2d_mpp_chn);
    CHECK_RET(ret, __func__, __LINE__);

    ret = kd_mpi_sys_unbind(&nonai_2d_mpp_chn, &venc_mpp_chn);
    CHECK_RET(ret, __func__, __LINE__);

    k_mpp_chn vo_mpp_chn;
    vo_mpp_chn.mod_id = K_ID_VO;
    vo_mpp_chn.dev_id = 0;
    vo_mpp_chn.chn_id = g_vo_layer;
    ret = kd_mpi_sys_unbind(&nonai_2d_mpp_chn, &vo_mpp_chn);
    CHECK_RET(ret, __func__, __LINE__);
    return;
}

static void *output_thread(void *arg)
{
    k_venc_stream output;
    int out_cnt, out_frames;
    k_s32 ret;
    int i;
    k_u32 total_len = 0;

    out_cnt = 0;
    out_frames = 0;

    while (1)
    {
        k_venc_chn_status status;

        ret = kd_mpi_venc_query_status(0, &status);
        CHECK_RET(ret, __func__, __LINE__);

        if (status.cur_packs > 0)
            output.pack_cnt = status.cur_packs;
        else
            output.pack_cnt = 1;

        output.pack = malloc(sizeof(k_venc_pack) * output.pack_cnt);

        ret = kd_mpi_venc_get_stream(0, &output, -1);
        CHECK_RET(ret, __func__, __LINE__);

        out_cnt += output.pack_cnt;
        for (i = 0; i < output.pack_cnt; i++)
        {
            if (output.pack[i].type != K_VENC_HEADER)
            {
                out_frames++;
            }

            k_u8 *pData;
            pData = (k_u8 *)kd_mpi_sys_mmap(output.pack[i].phys_addr, output.pack[i].len);
            if (g_venc_conf.output_file)
                fwrite(pData, 1, output.pack[i].len, g_venc_conf.output_file);
            kd_mpi_sys_munmap(pData, output.pack[i].len);

            total_len += output.pack[i].len;
        }

        ret = kd_mpi_venc_release_stream(0, &output);
        CHECK_RET(ret, __func__, __LINE__);

        free(output.pack);

    }
    printf("%s>done, out_frames %d, size %d bits\n", __func__, out_frames, total_len * 8);
    return arg;
}

k_s32 sample_venc_start()
{
    k_u32 bitrate   = 4000;   //kbps
    int width       = 1280;
    int height      = 720;
    k_venc_rc_mode rc_mode  = K_VENC_RC_MODE_CBR;
    k_payload_type type     = K_PT_H265;
    k_venc_profile profile  = VENC_PROFILE_H265_MAIN;
    int ret = 0;

    k_venc_chn_attr attr;
    memset(&attr, 0, sizeof(attr));
    attr.venc_attr.pic_width = width;
    attr.venc_attr.pic_height = height;
    attr.venc_attr.stream_buf_size = g_venc_conf.stream_buf_size;
    attr.venc_attr.stream_buf_cnt = VENC_BUF_NUM;

    attr.rc_attr.rc_mode = rc_mode;
    attr.rc_attr.cbr.src_frame_rate = 30;
    attr.rc_attr.cbr.dst_frame_rate = 30;
    attr.rc_attr.cbr.bit_rate = bitrate;

    attr.venc_attr.type = type;
    attr.venc_attr.profile = profile;
    printf("payload type is H265\n");

    ret = kd_mpi_venc_create_chn(0, &attr);
    CHECK_RET(ret, __func__, __LINE__);

    ret = kd_mpi_venc_start_chn(0);
    CHECK_RET(ret, __func__, __LINE__);

    sample_bind();

    pthread_create(&g_venc_conf.output_tid, NULL, output_thread, NULL);

    return K_SUCCESS;
}

k_s32 sample_venc_exit()
{
    int ret = 0;

    ret = kd_mpi_venc_stop_chn(0);
    CHECK_RET(ret, __func__, __LINE__);

    ret = kd_mpi_venc_destroy_chn(0);
    CHECK_RET(ret, __func__, __LINE__);

    sample_unbind();

    pthread_cancel(g_venc_conf.output_tid);
    pthread_join(g_venc_conf.output_tid, NULL);

    if (g_venc_conf.output_file)
        fclose(g_venc_conf.output_file);

    return K_SUCCESS;
}


int main(int argc, char *argv[])
{
    int ret;
    k_mcm_demo mcm;
    pthread_t mcm_dump_demo;
    pthread_t mcm_switch_id;

    memset(&g_venc_conf, 0, sizeof(venc_conf_t));

    for (int i = 1; i < argc; i++)
    {
        if (strcmp(argv[i], "-o") == 0)
        {
            strcpy(g_venc_conf.out_filename, argv[i + 1]);
            if ((g_venc_conf.output_file = fopen(g_venc_conf.out_filename, "wb")) == NULL)
            {
                printf("Cannot open output file\n");
            }
            printf("out_filename: %s\n", g_venc_conf.out_filename);
        }
    }

    // vb init
    ret = sample_vb_init();
    if(ret) {
        goto vb_init_error;
    }

    // vo init
    sample_vo_init(HX8377_V2_MIPI_4LAN_1080X1920_30FPS);

    // three mcm init
    ret = sample_vicap_init(VICAP_DEV_ID_0, XS9950_MIPI_CSI0_1280X720_30FPS_YUV422);
    if(ret < 0)
    {
        printf("vicap VICAP_DEV_ID_0 init failed \n");
        goto vicap_init_error;
    }
    ret = sample_vicap_init(VICAP_DEV_ID_1, XS9950_MIPI_CSI1_1280X720_30FPS_YUV422);
    if(ret < 0)
    {
        printf("vicap VICAP_DEV_ID_1 init failed \n");
        goto vicap_init_error;
    }
    ret = sample_vicap_init(VICAP_DEV_ID_2, XS9950_MIPI_CSI2_1280X720_30FPS_YUV422);
    if(ret < 0)
    {
        printf("vicap VICAP_DEV_ID_2 init failed \n");
        goto vicap_init_error;
    }

    sample_vicap_stream(VICAP_DEV_ID_0, K_TRUE);
    sample_vicap_stream(VICAP_DEV_ID_1, K_TRUE);
    sample_vicap_stream(VICAP_DEV_ID_2, K_TRUE);

    if(g_enable_2d)
    {
        nonai_2d_start();
        sample_venc_start();
    }

    usleep(200000);
    pthread_create(&mcm_dump_demo, NULL, sample_mcm_dump_screen_thread, &mcm);
    pthread_create(&mcm_switch_id, NULL, sample_mcm_switch_id_thread, &mcm);

    while(exit_flag != 1)
    {
        usleep(100000);
    }

    pthread_join(mcm_dump_demo, NULL);
    pthread_join(mcm_switch_id, NULL);

    for (int dev_num = 0; dev_num < VICAP_DEV_ID_MAX; dev_num++) {
        sample_vicap_stream(VICAP_DEV_ID_0 + dev_num, K_FALSE);
    }

    usleep(1000 * 34);
    kd_mpi_vo_disable_video_layer(g_vo_layer);

    if(g_enable_2d)
    {
        nonai_2d_exit();
        sample_venc_exit();
    }

    ret = kd_mpi_vb_exit();
    if (ret) {
        printf("fastboot_app, kd_mpi_vb_exit failed.\n");
        return ret;
    }

    return 0;

vicap_init_error:
    for(int i = 0; i < VICAP_DEV_ID_MAX; i++)
    {
        sample_vicap_stream(VICAP_DEV_ID_0 + i, K_FALSE);
    }

vb_init_error:
    return 0;
}
