#include "sample_dpu_vo.h"
#include "vo_test_case.h"

#include "k_connector_comm.h"
#include "mpi_connector_api.h"

extern k_video_frame_info g_vf_info;


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

static k_u32 sample_vicap_vo_init(k_bool mirror)
{
    k_u32 ret = 0;
    k_s32 connector_fd;
    k_connector_type connector_type = HX8377_V2_MIPI_4LAN_1080X1920_30FPS;
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

    if(mirror)
        kd_mpi_connector_set_mirror(connector_fd, K_CONNECTOR_MIRROR_VER);
    // connector init
    kd_mpi_connector_init(connector_fd, connector_info);

    return 0;
}

k_s32 sample_dv_vo_init(k_bool mirror)
{
    k_video_frame_info vf_info;
    dv_osd_info osd;
    k_vo_osd osd_id = K_VO_OSD3;

    memset(&vf_info, 0, sizeof(vf_info));
    memset(&osd, 0, sizeof(osd));

    sample_vicap_vo_init(mirror);

    osd.act_size.width = 720;// * 2 / 3 ;
    osd.act_size.height = 1280;
    osd.offset.x = (1080-osd.act_size.width)/2;
    osd.offset.y = (1920-osd.act_size.height)/2;
    osd.global_alptha = 0xff;
    osd.format = PIXEL_FORMAT_RGB_MONOCHROME_8BPP;//PIXEL_FORMAT_RGB_888;//PIXEL_FORMAT_ARGB_4444; //PIXEL_FORMAT_ARGB_1555;//PIXEL_FORMAT_ARGB_8888;

    /* config osd */
    dv_vo_creat_osd_test(osd_id, &osd);

    memset(&g_vf_info, 0, sizeof(k_video_frame_info));
    g_vf_info.v_frame.width = osd.act_size.width;
    g_vf_info.v_frame.height = osd.act_size.height;
    g_vf_info.v_frame.stride[0] = osd.act_size.width;
    g_vf_info.v_frame.pixel_format = osd.format;
    g_vf_info.v_frame.priv_data = K_VO_ONLY_CHANGE_PHYADDR;

    return K_SUCCESS;
}
