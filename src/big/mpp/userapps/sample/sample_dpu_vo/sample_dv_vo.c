#include "sample_dpu_vo.h"
#include "vo_test_case.h"

extern k_video_frame_info g_vf_info;

static k_vo_display_resolution hx8399[20] =
{
    // {74250, 445500, 1340, 1080, 20, 20, 220, 1938, 1920, 5, 8, 10},           // display  evblp3
    {74250, 445500, 1240, 1080, 20, 20, 120, 1988, 1920, 5, 8, 55},
};

static k_u32 dv_vo_creat_osd_test(k_vo_osd osd, dv_osd_info *info)
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
    else if(info->format == PIXEL_FORMAT_RGB_MONOCHROME_8BPP)
    {
        info->size = info->act_size.width  * info->act_size.height;
        info->stride  = info->act_size.width / 8;
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


k_s32 sample_dv_vo_init()
{
    k_vo_display_resolution *resolution = NULL;
    int resolution_index = 0;
    resolution = &hx8399[resolution_index];

    k_vo_pub_attr attr;
    k_video_frame_info vf_info;
    dv_osd_info osd;
    k_vo_osd osd_id = K_VO_OSD3;

    memset(&vf_info, 0, sizeof(vf_info));
    memset(&attr, 0, sizeof(attr));
    memset(&osd, 0, sizeof(osd));

    attr.bg_color = 0xffffff;
    attr.intf_sync = K_VO_OUT_1080P30;
    attr.intf_type = K_VO_INTF_MIPI;
    attr.sync_info = resolution;

    osd.act_size.width = 720;// * 2 / 3 ;
    osd.act_size.height = 1280;
    osd.offset.x = 0;
    osd.offset.y = 0;
    osd.global_alptha = 0xff;
    osd.format = PIXEL_FORMAT_RGB_MONOCHROME_8BPP;//PIXEL_FORMAT_RGB_888;//PIXEL_FORMAT_ARGB_4444; //PIXEL_FORMAT_ARGB_1555;//PIXEL_FORMAT_ARGB_8888;

    kd_display_reset();
    // set hardware reset;
    kd_display_set_backlight();

    usleep(200000);
    
    dwc_dsi_init();

    /* vo init */
    kd_mpi_vo_init();

    /* set vo timing */
    kd_mpi_vo_set_dev_param(&attr);

    /* config osd */
    dv_vo_creat_osd_test(osd_id, &osd);

    /* enable vo */
    kd_mpi_vo_enable();

    memset(&g_vf_info, 0, sizeof(k_video_frame_info));
    g_vf_info.v_frame.width = osd.act_size.width;
    g_vf_info.v_frame.height = osd.act_size.height;
    g_vf_info.v_frame.stride[0] = osd.act_size.width;
    g_vf_info.v_frame.pixel_format = osd.format;
    g_vf_info.v_frame.priv_data = K_VO_ONLY_CHANGE_PHYADDR;

    return K_SUCCESS;
}
