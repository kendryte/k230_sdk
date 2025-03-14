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

#include "k_dewarp_comm.h"
#include "mpi_dewarp_api.h"

#include "k_dma_comm.h"
#include "mpi_dma_api.h"

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

#define DW200_CHN0_INPUT_WIDTH 1280
#define DW200_CHN0_INPUT_HEIGHT 720
#define DW200_CHN0_OUTPUT_WIDTH 640 //960
#define DW200_CHN0_OUTPUT_HEIGHT 360//540
#define DW200_CHN0_VB_NUM 4

#define DW200_CHN1_INPUT_WIDTH 1280
#define DW200_CHN1_INPUT_HEIGHT 720
#define DW200_CHN1_OUTPUT_WIDTH 640//960
#define DW200_CHN1_OUTPUT_HEIGHT 360 //540
#define DW200_CHN1_VB_NUM 4

#define DW200_CHN2_INPUT_WIDTH 1280
#define DW200_CHN2_INPUT_HEIGHT 720
#define DW200_CHN2_OUTPUT_WIDTH 640 //960
#define DW200_CHN2_OUTPUT_HEIGHT 360//540
#define DW200_CHN2_VB_NUM 4
#define GDMA_BUF_NUM 6

static k_u32 g_vo_pool_id;
static k_u8 exit_flag = 0;

// static k_vo_layer g_vo_layer = K_VO_LAYER1;

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

    // DW output vb CHN 0 vb mem = 11,059,200
    config.comm_pool[5].blk_cnt = DW200_CHN0_VB_NUM;
    config.comm_pool[5].blk_size = VICAP_ALIGN_UP((DW200_CHN0_OUTPUT_WIDTH * DW200_CHN0_OUTPUT_HEIGHT * 3), 0x1000);
    config.comm_pool[5].mode = VB_REMAP_MODE_NOCACHE;

    config.comm_pool[6].blk_cnt = DW200_CHN1_VB_NUM;
    config.comm_pool[6].blk_size = VICAP_ALIGN_UP((DW200_CHN1_OUTPUT_WIDTH * DW200_CHN1_OUTPUT_HEIGHT * 3), 0x1000);
    config.comm_pool[6].mode = VB_REMAP_MODE_NOCACHE;

    config.comm_pool[7].blk_cnt = DW200_CHN2_VB_NUM;
    config.comm_pool[7].blk_size = VICAP_ALIGN_UP((DW200_CHN1_OUTPUT_WIDTH * DW200_CHN1_OUTPUT_HEIGHT * 3), 0x1000);
    config.comm_pool[7].mode = VB_REMAP_MODE_NOCACHE;

    // for gdma chn0 
    config.comm_pool[8].blk_cnt = GDMA_BUF_NUM;
    config.comm_pool[8].blk_size = VICAP_ALIGN_UP((DW200_CHN0_OUTPUT_WIDTH * DW200_CHN0_OUTPUT_HEIGHT * 3), 0x1000);
    config.comm_pool[8].mode = VB_REMAP_MODE_NOCACHE;

    // for gdma chn 1
    config.comm_pool[9].blk_cnt = GDMA_BUF_NUM;
    config.comm_pool[9].blk_size = VICAP_ALIGN_UP((DW200_CHN1_OUTPUT_WIDTH * DW200_CHN1_OUTPUT_HEIGHT * 3), 0x1000);
    config.comm_pool[9].mode = VB_REMAP_MODE_NOCACHE;

     // for gdma chn 1
    config.comm_pool[10].blk_cnt = GDMA_BUF_NUM;
    config.comm_pool[10].blk_size = VICAP_ALIGN_UP((DW200_CHN2_OUTPUT_WIDTH * DW200_CHN2_OUTPUT_HEIGHT * 3), 0x1000);
    config.comm_pool[10].mode = VB_REMAP_MODE_NOCACHE;


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

// static k_vb_blk_handle sample_vo_insert_frame(k_video_frame_info *vf_info, void **pic_vaddr)
// {
//     k_u64 phys_addr = 0;
//     k_u32 *virt_addr;
//     k_vb_blk_handle handle;
//     k_s32 size = 0;

//     if (vf_info == NULL)
//         return K_FALSE;

//     if (vf_info->v_frame.pixel_format == PIXEL_FORMAT_ABGR_8888 || vf_info->v_frame.pixel_format == PIXEL_FORMAT_ARGB_8888)
//         size = vf_info->v_frame.height * vf_info->v_frame.width * 4;
//     else if (vf_info->v_frame.pixel_format == PIXEL_FORMAT_RGB_565 || vf_info->v_frame.pixel_format == PIXEL_FORMAT_BGR_565)
//         size = vf_info->v_frame.height * vf_info->v_frame.width * 2;
//     else if (vf_info->v_frame.pixel_format == PIXEL_FORMAT_ABGR_4444 || vf_info->v_frame.pixel_format == PIXEL_FORMAT_ARGB_4444)
//         size = vf_info->v_frame.height * vf_info->v_frame.width * 2;
//     else if (vf_info->v_frame.pixel_format == PIXEL_FORMAT_RGB_888 || vf_info->v_frame.pixel_format == PIXEL_FORMAT_BGR_888)
//         size = vf_info->v_frame.height * vf_info->v_frame.width * 3;
//     else if (vf_info->v_frame.pixel_format == PIXEL_FORMAT_ARGB_1555 || vf_info->v_frame.pixel_format == PIXEL_FORMAT_ABGR_1555)
//         size = vf_info->v_frame.height * vf_info->v_frame.width * 2;
//     else if (vf_info->v_frame.pixel_format == PIXEL_FORMAT_YVU_PLANAR_420)
//         size = vf_info->v_frame.height * vf_info->v_frame.width * 3 / 2;

//     size = size + 4096;         // 强制4K ，后边得删了

//     printf("vb block size is %x \n", size);

//     handle = kd_mpi_vb_get_block(g_vo_pool_id, size, NULL);
//     if (handle == VB_INVALID_HANDLE)
//     {
//         printf("%s get vb block error\n", __func__);
//         return K_FAILED;
//     }

//     phys_addr = kd_mpi_vb_handle_to_phyaddr(handle);
//     if (phys_addr == 0)
//     {
//         printf("%s get phys addr error\n", __func__);
//         return K_FAILED;
//     }

//     virt_addr = (k_u32 *)kd_mpi_sys_mmap(phys_addr, size);
//     // virt_addr = (k_u32 *)kd_mpi_sys_mmap_cached(phys_addr, size);

//     if (virt_addr == NULL)
//     {
//         printf("%s mmap error\n", __func__);
//         return K_FAILED;
//     }

//     vf_info->mod_id = K_ID_VO;
//     vf_info->pool_id = g_vo_pool_id;
//     vf_info->v_frame.phys_addr[0] = phys_addr;
//     if (vf_info->v_frame.pixel_format == PIXEL_FORMAT_YVU_PLANAR_420)
//         vf_info->v_frame.phys_addr[1] = phys_addr + (vf_info->v_frame.height * vf_info->v_frame.stride[0]);
//     *pic_vaddr = virt_addr;

//     printf("phys_addr is %lx \n", phys_addr);

//     return handle;
// }

static void sample_vo_init(k_connector_type type)
{
    osd_info osd;
    layer_info info;

    memset(&info, 0, sizeof(info));
    memset(&osd, 0, sizeof(osd));

    sample_connector_init(type);

#if  DW_DEV0_USE_RGB
         // osd0 init
        osd.act_size.width = DW200_CHN0_OUTPUT_WIDTH ;
        osd.act_size.height = DW200_CHN0_OUTPUT_HEIGHT;
        osd.offset.x = 0;
        osd.offset.y = 0;
        osd.global_alptha = 0xff;// 0x7f;
        osd.format = PIXEL_FORMAT_RGB_888;
        sample_vo_creat_osd_test(K_VO_OSD2, &osd);
#else
        info.act_size.width = DW200_CHN0_OUTPUT_HEIGHT;//DW200_CHN0_OUTPUT_WIDTH ;
        info.act_size.height = DW200_CHN0_OUTPUT_WIDTH;//DW200_CHN0_OUTPUT_HEIGHT;
        info.format = PIXEL_FORMAT_YVU_PLANAR_420;
        info.func = 0;////K_ROTATION_90;
        info.global_alptha = 0xff;
        info.offset.x = 0;//(1080-w)/2,
        info.offset.y = 0;//(1920-h)/2;
        vo_creat_layer_test(K_VO_LAYER1, &info);
#endif
        
#if DW_DEV1_USE_RGB
         // osd0 init
        osd.act_size.width = DW200_CHN1_OUTPUT_WIDTH ;
        osd.act_size.height = DW200_CHN1_OUTPUT_HEIGHT;
        osd.offset.x = DW200_CHN1_OUTPUT_WIDTH ;//960;
        osd.offset.y = 0;
        osd.global_alptha = 0xff;// 0x7f;
        osd.format = PIXEL_FORMAT_RGB_888;
        sample_vo_creat_osd_test(K_VO_OSD1, &osd);
#else
        // layer2 init
        info.act_size.width = DW200_CHN1_OUTPUT_HEIGHT;//DW200_CHN1_OUTPUT_WIDTH ;
        info.act_size.height = DW200_CHN1_OUTPUT_WIDTH; //DW200_CHN1_OUTPUT_HEIGHT;
        info.format = PIXEL_FORMAT_YVU_PLANAR_420;
        info.func = 0;////K_ROTATION_90;
        info.global_alptha = 0xff;
        info.offset.x = DW200_CHN1_OUTPUT_HEIGHT + 50;//(1080-w)/2,
        info.offset.y = 0;//(1920-h)/2;
        vo_creat_layer_test(K_VO_LAYER2, &info);
#endif

         // osd0 init
        osd.act_size.width = DW200_CHN2_OUTPUT_HEIGHT ;
        osd.act_size.height = DW200_CHN2_OUTPUT_WIDTH;
        osd.offset.x = 0;
        osd.offset.y = DW200_CHN2_OUTPUT_WIDTH;//540;
        osd.global_alptha = 0xff;// 0x7f;
        osd.format = PIXEL_FORMAT_RGB_888;
        sample_vo_creat_osd_test(K_VO_OSD0, &osd);

}

static k_s32 nonai_2d_init()
{
    int i;
    k_s32 ret = 0;
    k_nonai_2d_chn_attr attr_2d;

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
        // kd_mpi_nonai_2d_init(i, &attr_2d);
        ret = kd_mpi_nonai_2d_create_chn(i, &attr_2d);
        if(ret != 0 )
            printf("kd_mpi_nonai_2d_create_chn failed \n");
        // kd_mpi_nonai_2d_start(i);
        ret = kd_mpi_nonai_2d_start_chn(i);
        if(ret != 0 )
            printf("kd_mpi_nonai_2d_start_chn failed \n");
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




#define DEWARP_DEV_ID 0

#define DW_DEV1_USE_RGB         0
#define DW_DEV0_USE_RGB         0


static k_s32 sample_dw200_init(void)
{
    k_s32 ret = 0;
    struct k_dw_settings dw0_settings;
    struct k_dw_settings dw1_settings;
    struct k_dw_settings dw2_settings;

    memset(&dw0_settings, 0, sizeof(struct k_dw_settings));
    memset(&dw1_settings, 0, sizeof(struct k_dw_settings));
    memset(&dw2_settings, 0, sizeof(struct k_dw_settings));

#if DW_DEV0_USE_RGB
    dw0_settings.vdev_id = DEWARP_DEV_ID;
    dw0_settings.input.width = DW200_CHN0_INPUT_WIDTH;
    dw0_settings.input.height = DW200_CHN0_INPUT_HEIGHT;
    dw0_settings.input.format = K_DW_PIX_YUV420SP;
    dw0_settings.input.bit10 = K_FALSE;
    dw0_settings.input.alignment = K_FALSE;
    dw0_settings.output_enable_mask = 1;

    dw0_settings.output[0].width = DW200_CHN0_OUTPUT_WIDTH;
    dw0_settings.output[0].height = DW200_CHN0_OUTPUT_HEIGHT;
    dw0_settings.output[0].format = K_DW_PIX_RGB888;
    dw0_settings.output[0].alignment = 0;
    dw0_settings.output[0].bit10= K_FALSE;
    dw0_settings.crop[0].bottom = 0;
    dw0_settings.crop[0].left = 0;
    dw0_settings.crop[0].right = 0;
    dw0_settings.crop[0].top = 0;
#else
    dw0_settings.vdev_id = DEWARP_DEV_ID;
    dw0_settings.input.width = DW200_CHN0_INPUT_WIDTH;
    dw0_settings.input.height = DW200_CHN0_INPUT_HEIGHT;
    dw0_settings.input.format = K_DW_PIX_YUV420SP;
    dw0_settings.input.bit10 = K_FALSE;
    dw0_settings.input.alignment = K_FALSE;
    dw0_settings.output_enable_mask = 1;

    dw0_settings.output[0].width = DW200_CHN0_OUTPUT_WIDTH;
    dw0_settings.output[0].height = DW200_CHN0_OUTPUT_HEIGHT;
    dw0_settings.output[0].format = K_DW_PIX_YUV420SP;
    dw0_settings.output[0].alignment = 0;
    dw0_settings.output[0].bit10= K_FALSE;
    dw0_settings.crop[0].bottom = 0;
    dw0_settings.crop[0].left = 0;
    dw0_settings.crop[0].right = 0;
    dw0_settings.crop[0].top = 0;
#endif

#if DW_DEV1_USE_RGB
    dw1_settings.vdev_id = DEWARP_DEV_ID + 1;
    dw1_settings.input.width = DW200_CHN1_INPUT_WIDTH;
    dw1_settings.input.height = DW200_CHN1_INPUT_HEIGHT;
    dw1_settings.input.format = K_DW_PIX_YUV420SP;
    dw1_settings.input.bit10 = K_FALSE;
    dw1_settings.input.alignment = K_FALSE;
    dw1_settings.output_enable_mask = 1;

    dw1_settings.output[0].width = DW200_CHN1_OUTPUT_WIDTH;
    dw1_settings.output[0].height = DW200_CHN1_OUTPUT_HEIGHT;
    dw1_settings.output[0].format = K_DW_PIX_RGB888;
    dw1_settings.output[0].alignment = 0;
    dw1_settings.output[0].bit10= K_FALSE;
    dw1_settings.crop[0].bottom = 0;
    dw1_settings.crop[0].left = 0;
    dw1_settings.crop[0].right = 0;
    dw1_settings.crop[0].top = 0;
#else
    dw1_settings.vdev_id = DEWARP_DEV_ID + 1;
    dw1_settings.input.width = DW200_CHN1_INPUT_WIDTH;
    dw1_settings.input.height = DW200_CHN1_INPUT_HEIGHT;
    dw1_settings.input.format = K_DW_PIX_YUV420SP;
    dw1_settings.input.bit10 = K_FALSE;
    dw1_settings.input.alignment = K_FALSE;
    dw1_settings.output_enable_mask = 1;

    dw1_settings.output[0].width = DW200_CHN1_OUTPUT_WIDTH;
    dw1_settings.output[0].height = DW200_CHN1_OUTPUT_HEIGHT;
    dw1_settings.output[0].format = K_DW_PIX_YUV420SP;
    dw1_settings.output[0].alignment = 0;
    dw1_settings.output[0].bit10= K_FALSE;
    dw1_settings.crop[0].bottom = 0;
    dw1_settings.crop[0].left = 0;
    dw1_settings.crop[0].right = 0;
    dw1_settings.crop[0].top = 0;
#endif

    dw2_settings.vdev_id = DEWARP_DEV_ID + 2;
    dw2_settings.input.width = DW200_CHN2_INPUT_WIDTH;
    dw2_settings.input.height = DW200_CHN2_INPUT_HEIGHT;
    dw2_settings.input.format = K_DW_PIX_YUV420SP;
    dw2_settings.input.bit10 = K_FALSE;
    dw2_settings.input.alignment = K_FALSE;
    dw2_settings.output_enable_mask = 1;

    dw2_settings.output[0].width = DW200_CHN2_OUTPUT_WIDTH;
    dw2_settings.output[0].height = DW200_CHN2_OUTPUT_HEIGHT;
    dw2_settings.output[0].format = K_DW_PIX_RGB888;
    dw2_settings.output[0].alignment = 0;
    dw2_settings.output[0].bit10= K_FALSE;
    dw2_settings.crop[0].bottom = 0;
    dw2_settings.crop[0].left = 0;
    dw2_settings.crop[0].right = 0;
    dw2_settings.crop[0].top = 0;

    ret = kd_mpi_dw_init(&dw0_settings);
    if(ret)
    {
        printf("kd_mpi_dw_init init o failed \n");
    }

    ret = kd_mpi_dw_init(&dw1_settings);
    if(ret)
    {
        printf("kd_mpi_dw_init init o failed \n");
    }

    ret = kd_mpi_dw_init(&dw2_settings);
    if(ret)
    {
        printf("kd_mpi_dw_init init o failed \n");
    }

    return ret ;
}

static void dw_exit(void)
{
    kd_mpi_dw_exit(DEWARP_DEV_ID);
    kd_mpi_dw_exit(DEWARP_DEV_ID + 1);
	kd_mpi_dw_exit(DEWARP_DEV_ID + 2);
}


static k_s32 dma_dev_attr_init(void)
{
    k_dma_dev_attr_t dev_attr;

    dev_attr.burst_len = 0;
    dev_attr.ckg_bypass = 0xff;
    dev_attr.outstanding = 7;

    int ret = kd_mpi_dma_set_dev_attr(&dev_attr);
    if (ret != K_SUCCESS)
    {
        printf("set dma dev attr error\r\n");
        return ret;
    }

    ret = kd_mpi_dma_start_dev();
    if (ret != K_SUCCESS)
    {
        printf("start dev error\r\n");
        return ret;
    }

    return ret;
}


static void gdma_init(k_u8 chn, k_u8 rot, k_pixel_format pix, k_u32 width, k_u32 height)
{
    k_u8 gdma_rotation = rot;
    k_pixel_format pix_format = pix;//PIXEL_FORMAT_YVU_SEMIPLANAR_420;
    k_dma_chn_attr_u gdma_attr;
    k_u8 ret = 0;

    memset(&gdma_attr, 0, sizeof(gdma_attr));
    gdma_attr.gdma_attr.buffer_num = 3;//GDMA_BUF_NUM;
    gdma_attr.gdma_attr.rotation = gdma_rotation;
    gdma_attr.gdma_attr.x_mirror = K_FALSE;
    gdma_attr.gdma_attr.y_mirror = K_FALSE;
    gdma_attr.gdma_attr.width = width;
    gdma_attr.gdma_attr.height = height;
    gdma_attr.gdma_attr.work_mode = DMA_BIND;
    gdma_attr.gdma_attr.src_stride[0] = width;
    if (gdma_rotation == DEGREE_180) {
        gdma_attr.gdma_attr.dst_stride[0] = width;
    } else {
        gdma_attr.gdma_attr.dst_stride[0] = height;
    }
    if (pix_format == PIXEL_FORMAT_RGB_888) {
        gdma_attr.gdma_attr.pixel_format = DMA_PIXEL_FORMAT_RGB_888;
        gdma_attr.gdma_attr.src_stride[0] *= 3;
        gdma_attr.gdma_attr.dst_stride[0] *= 3;
    } else {
        gdma_attr.gdma_attr.pixel_format = DMA_PIXEL_FORMAT_YUV_SEMIPLANAR_420_8BIT;
        gdma_attr.gdma_attr.src_stride[1] = gdma_attr.gdma_attr.src_stride[0];
        gdma_attr.gdma_attr.dst_stride[1] = gdma_attr.gdma_attr.dst_stride[0];
    }

    ret = kd_mpi_dma_set_chn_attr(chn, &gdma_attr);
    if (ret != K_SUCCESS) {
        printf("set chn attr error\r\n");
    }
    ret = kd_mpi_dma_start_chn(chn);
    if (ret != K_SUCCESS) {
        printf("start chn error\r\n");
    }

}

static void gdma_exit(k_u8 gdma_chn)
{
    k_u8 ret = 0 ;
    ret = kd_mpi_dma_stop_chn(gdma_chn);
    if (ret != K_SUCCESS) {
        printf("stop chn error\r\n");
    }

}

static void sample_bind()
{
    k_s32 ret;
    k_mpp_chn vi_mpp_chn;
    k_mpp_chn nonai_2d_mpp_chn;
    k_mpp_chn dw_mpp_chn;
    k_mpp_chn vo_mpp_chn;
    k_mpp_chn gdma;

    // pipe 1 
    vi_mpp_chn.mod_id = K_ID_VI;
    vi_mpp_chn.dev_id = 0;
    vi_mpp_chn.chn_id = 0;
    nonai_2d_mpp_chn.mod_id = K_ID_NONAI_2D;
    nonai_2d_mpp_chn.dev_id = 0;
    nonai_2d_mpp_chn.chn_id = NONAI_2D_BIND_CH_0;
    ret = kd_mpi_sys_bind(&vi_mpp_chn, &nonai_2d_mpp_chn);
    CHECK_RET(ret, __func__, __LINE__);

    dw_mpp_chn.mod_id = K_ID_DW200;
    dw_mpp_chn.dev_id = 0;
    dw_mpp_chn.chn_id = 0;
    ret = kd_mpi_sys_bind(&nonai_2d_mpp_chn, &dw_mpp_chn);
    CHECK_RET(ret, __func__, __LINE__);

    gdma.mod_id = K_ID_DMA;
    gdma.dev_id = 0;
    gdma.chn_id = 0;
    ret = kd_mpi_sys_bind(&dw_mpp_chn, &gdma);
    CHECK_RET(ret, __func__, __LINE__);

    vo_mpp_chn.mod_id = K_ID_VO;
    vo_mpp_chn.dev_id = 0;
    vo_mpp_chn.chn_id = K_VO_LAYER1;
    ret = kd_mpi_sys_bind(&gdma, &vo_mpp_chn);
    CHECK_RET(ret, __func__, __LINE__);

    // pipe 2 
    vi_mpp_chn.mod_id = K_ID_VI;
    vi_mpp_chn.dev_id = 1;
    vi_mpp_chn.chn_id = 0;
    nonai_2d_mpp_chn.mod_id = K_ID_NONAI_2D;
    nonai_2d_mpp_chn.dev_id = 0;
    nonai_2d_mpp_chn.chn_id = 1;
    ret = kd_mpi_sys_bind(&vi_mpp_chn, &nonai_2d_mpp_chn);
    CHECK_RET(ret, __func__, __LINE__);

    dw_mpp_chn.mod_id = K_ID_DW200;
    dw_mpp_chn.dev_id = 1;
    dw_mpp_chn.chn_id = 0;
    ret = kd_mpi_sys_bind(&nonai_2d_mpp_chn, &dw_mpp_chn);
    CHECK_RET(ret, __func__, __LINE__);

    gdma.mod_id = K_ID_DMA;
    gdma.dev_id = 0;
    gdma.chn_id = 1;
    ret = kd_mpi_sys_bind(&dw_mpp_chn, &gdma);
    CHECK_RET(ret, __func__, __LINE__);

    vo_mpp_chn.mod_id = K_ID_VO;
    vo_mpp_chn.dev_id = 0;
    vo_mpp_chn.chn_id = K_VO_LAYER2;
    ret = kd_mpi_sys_bind(&gdma, &vo_mpp_chn);
    CHECK_RET(ret, __func__, __LINE__);

     // pipe 3
    vi_mpp_chn.mod_id = K_ID_VI;
    vi_mpp_chn.dev_id = 2;
    vi_mpp_chn.chn_id = 0;
    nonai_2d_mpp_chn.mod_id = K_ID_NONAI_2D;
    nonai_2d_mpp_chn.dev_id = 0;
    nonai_2d_mpp_chn.chn_id = 2;
    ret = kd_mpi_sys_bind(&vi_mpp_chn, &nonai_2d_mpp_chn);
    CHECK_RET(ret, __func__, __LINE__);

    dw_mpp_chn.mod_id = K_ID_DW200;
    dw_mpp_chn.dev_id = 2;
    dw_mpp_chn.chn_id = 0;
    ret = kd_mpi_sys_bind(&nonai_2d_mpp_chn, &dw_mpp_chn);
    CHECK_RET(ret, __func__, __LINE__);

    gdma.mod_id = K_ID_DMA;
    gdma.dev_id = 0;
    gdma.chn_id = 2;
    ret = kd_mpi_sys_bind(&dw_mpp_chn, &gdma);
    CHECK_RET(ret, __func__, __LINE__);

    vo_mpp_chn.mod_id = K_ID_VO;
    vo_mpp_chn.dev_id = 0;
    vo_mpp_chn.chn_id = 3;
    ret = kd_mpi_sys_bind(&gdma, &vo_mpp_chn);
    CHECK_RET(ret, __func__, __LINE__);

    return;
}

static void sample_unbind()
{
        k_s32 ret;
    k_mpp_chn vi_mpp_chn;
    k_mpp_chn nonai_2d_mpp_chn;
    k_mpp_chn dw_mpp_chn;
    k_mpp_chn vo_mpp_chn;
    k_mpp_chn gdma;

    // pipe 1 
    vi_mpp_chn.mod_id = K_ID_VI;
    vi_mpp_chn.dev_id = 0;
    vi_mpp_chn.chn_id = 0;

    nonai_2d_mpp_chn.mod_id = K_ID_NONAI_2D;
    nonai_2d_mpp_chn.dev_id = 0;
    nonai_2d_mpp_chn.chn_id = NONAI_2D_BIND_CH_0;
    ret = kd_mpi_sys_unbind(&vi_mpp_chn, &nonai_2d_mpp_chn);
    CHECK_RET(ret, __func__, __LINE__);

    dw_mpp_chn.mod_id = K_ID_DW200;
    dw_mpp_chn.dev_id = 0;
    dw_mpp_chn.chn_id = 0;
    ret = kd_mpi_sys_unbind(&nonai_2d_mpp_chn, &dw_mpp_chn);
    CHECK_RET(ret, __func__, __LINE__);

    gdma.mod_id = K_ID_DMA;
    gdma.dev_id = 0;
    gdma.chn_id = 0;
    ret = kd_mpi_sys_unbind(&dw_mpp_chn, &gdma);
    CHECK_RET(ret, __func__, __LINE__);

    vo_mpp_chn.mod_id = K_ID_VO;
    vo_mpp_chn.dev_id = 0;
    vo_mpp_chn.chn_id = K_VO_LAYER1;
    ret = kd_mpi_sys_unbind(&gdma, &vo_mpp_chn);
    CHECK_RET(ret, __func__, __LINE__);

    // pipe 2 
    vi_mpp_chn.mod_id = K_ID_VI;
    vi_mpp_chn.dev_id = 1;
    vi_mpp_chn.chn_id = 0;
    nonai_2d_mpp_chn.mod_id = K_ID_NONAI_2D;
    nonai_2d_mpp_chn.dev_id = 0;
    nonai_2d_mpp_chn.chn_id = 1;
    ret = kd_mpi_sys_unbind(&vi_mpp_chn, &nonai_2d_mpp_chn);
    CHECK_RET(ret, __func__, __LINE__);

    dw_mpp_chn.mod_id = K_ID_DW200;
    dw_mpp_chn.dev_id = 1;
    dw_mpp_chn.chn_id = 0;
    ret = kd_mpi_sys_unbind(&nonai_2d_mpp_chn, &dw_mpp_chn);
    CHECK_RET(ret, __func__, __LINE__);

    gdma.mod_id = K_ID_DMA;
    gdma.dev_id = 0;
    gdma.chn_id = 1;
    ret = kd_mpi_sys_unbind(&dw_mpp_chn, &gdma);
    CHECK_RET(ret, __func__, __LINE__);

    vo_mpp_chn.mod_id = K_ID_VO;
    vo_mpp_chn.dev_id = 0;
    vo_mpp_chn.chn_id = K_VO_LAYER2;
    ret = kd_mpi_sys_unbind(&gdma, &vo_mpp_chn);
    CHECK_RET(ret, __func__, __LINE__);

     // pipe 3
    vi_mpp_chn.mod_id = K_ID_VI;
    vi_mpp_chn.dev_id = 2;
    vi_mpp_chn.chn_id = 0;
    
    nonai_2d_mpp_chn.mod_id = K_ID_NONAI_2D;
    nonai_2d_mpp_chn.dev_id = 0;
    nonai_2d_mpp_chn.chn_id = 2;
    ret = kd_mpi_sys_unbind(&vi_mpp_chn, &nonai_2d_mpp_chn);
    CHECK_RET(ret, __func__, __LINE__);

    dw_mpp_chn.mod_id = K_ID_DW200;
    dw_mpp_chn.dev_id = 2;
    dw_mpp_chn.chn_id = 0;
    ret = kd_mpi_sys_unbind(&nonai_2d_mpp_chn, &dw_mpp_chn);
    CHECK_RET(ret, __func__, __LINE__);

    gdma.mod_id = K_ID_DMA;
    gdma.dev_id = 0;
    gdma.chn_id = 2;
    ret = kd_mpi_sys_unbind(&dw_mpp_chn, &gdma);
    CHECK_RET(ret, __func__, __LINE__);

    vo_mpp_chn.mod_id = K_ID_VO;
    vo_mpp_chn.dev_id = 0;
    vo_mpp_chn.chn_id = 3;
    ret = kd_mpi_sys_unbind(&gdma, &vo_mpp_chn);
    CHECK_RET(ret, __func__, __LINE__);

    return;
    return;
}


int main(int argc, char *argv[])
{
    int ret;
    k_video_frame_info dump_info;
    k_video_frame_info rgb_vf_info;

    // vb init
    ret = sample_vb_init();
    if(ret) {
        goto vb_init_error;
    }

    // vo init
    sample_vo_init(LT9611_MIPI_4LAN_1280X720_30FPS);

    // init 2d 
    nonai_2d_init();

    // init bind 
    sample_bind();

    dma_dev_attr_init();
    gdma_init(0, DEGREE_90, PIXEL_FORMAT_YVU_SEMIPLANAR_420, DW200_CHN0_OUTPUT_WIDTH, DW200_CHN0_OUTPUT_HEIGHT);
    gdma_init(1, DEGREE_90, PIXEL_FORMAT_YVU_SEMIPLANAR_420, DW200_CHN0_OUTPUT_WIDTH, DW200_CHN0_OUTPUT_HEIGHT);
     gdma_init(2, DEGREE_90, PIXEL_FORMAT_RGB_888, DW200_CHN0_OUTPUT_WIDTH, DW200_CHN0_OUTPUT_HEIGHT);

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

    sample_dw200_init();

    sample_vicap_stream(VICAP_DEV_ID_0, K_TRUE);
    sample_vicap_stream(VICAP_DEV_ID_1, K_TRUE);
    sample_vicap_stream(VICAP_DEV_ID_2, K_TRUE);


    while(exit_flag != 1)
    {
        memset(&dump_info, 0 , sizeof(k_video_frame_info));
        memset(&rgb_vf_info, 0 , sizeof(k_video_frame_info));
        ret = kd_mpi_vicap_dump_frame(VICAP_DEV_ID_0, VICAP_CHN_ID_0, VICAP_DUMP_YUV444, &dump_info, 300);
        if (ret)
        {
            printf("sample_vicap...kd_mpi_vicap_dump_frame failed.\n");
            continue;
        }

        ret = kd_mpi_nonai_2d_send_frame(NONAI_2D_RGB_CH, &dump_info, 1000);
        if (ret)
        {
            printf("sensor 0：kd_mpi_nonai_2d_send_frame ch 4 failed. %d\n", ret);
            ret = kd_mpi_vicap_dump_release(VICAP_DEV_ID_0, VICAP_CHN_ID_0, &dump_info);
            if (ret)
            {
                printf("sensor 0：sample_vicap...kd_mpi_vicap_dump_release failed.\n");
                continue;
            }
        }


        ret = kd_mpi_nonai_2d_get_frame(NONAI_2D_RGB_CH, &rgb_vf_info, 1000);
        if (ret)
        {
            printf("sensor 0：kd_mpi_nonai_2d_get_frame ch 4 failed. %d\n", ret);
            // goto vicap_release;
            ret = kd_mpi_vicap_dump_release(VICAP_DEV_ID_0, VICAP_CHN_ID_0, &dump_info);
            if (ret)
            {
                printf("sensor 0：sample_vicap...kd_mpi_vicap_dump_release failed.\n");
                continue;
            }
        }

        // todo ai 

        ret = kd_mpi_nonai_2d_release_frame(NONAI_2D_RGB_CH, &rgb_vf_info);
        if (ret) {
            printf("sensor 0：kd_mpi_nonai_2d_release_frame ch 4 failed. %d\n", ret);
        }

        ret = kd_mpi_vicap_dump_release(VICAP_DEV_ID_0, VICAP_CHN_ID_0, &dump_info);
        if (ret) {
            printf("sample_vicap...kd_mpi_vicap_dump_release failed.\n");
        }
    }


    sample_vicap_stream(VICAP_DEV_ID_0 , K_FALSE);
    sample_vicap_stream(VICAP_DEV_ID_1 , K_FALSE);
    sample_vicap_stream(VICAP_DEV_ID_2 , K_FALSE);


    usleep(1000 * 34);


    nonai_2d_exit();

    dw_exit();

    gdma_exit(0);
    gdma_exit(1);

    kd_mpi_vo_disable_video_layer(K_VO_LAYER1);
    kd_mpi_vo_disable_video_layer(K_VO_LAYER2);

    sample_unbind();

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