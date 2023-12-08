#include "k_module.h"
#include "k_type.h"
#include "k_vb_comm.h"
#include "k_video_comm.h"
#include "k_sys_comm.h"
#include "mpi_vb_api.h"
#include "mpi_sys_api.h"
#include "mpi_vo_api.h"
#include "k_vo_comm.h"

#include "stdio.h"
#include "string.h"
#include "stdlib.h"


#include "k_connector_comm.h"
#include "mpi_connector_api.h"
#include "vo.h"

#include "opencv2/opencv.hpp"
#include "opencv2/core.hpp"
#include "opencv2/highgui.hpp"
#include "opencv2/imgcodecs.hpp"
#include "opencv2/imgproc.hpp"

#include "CvxText.h"



k_vicap_dev_attr dev_attr;
k_vicap_chn_attr chn_attr;
k_vicap_sensor_info sensor_info;
k_vicap_sensor_type sensor_type;


k_s32 sample_connector_init(k_connector_type type)
{
    k_u32 ret = 0;
    k_s32 connector_fd;
    k_u32 chip_id = 0x00;
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
    // set connect get id
    kd_mpi_connector_id_get(connector_fd, &chip_id);
    // connector init
    kd_mpi_connector_init(connector_fd, connector_info);

    return 0;
}

k_u32 sample_vo_creat_osd(k_vo_osd osd, osd_info *info)
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

int vo_creat_layer_test(k_vo_layer chn_id, layer_info *info)
{
    k_vo_video_layer_attr attr;

    // check layer
    if ((chn_id >= K_MAX_VO_LAYER_NUM) || ((info->func & K_VO_SCALER_ENABLE) && (chn_id != K_VO_LAYER0))
            || ((info->func != 0) && (chn_id == K_VO_LAYER2)))
    {
        printf("input layer num failed \n");
        return -1 ;
    }

    memset(&attr, 0, sizeof(attr));

    // set offset
    attr.display_rect = info->offset;
    // set act
    attr.img_size = info->act_size;
    // sget size
    info->size = info->act_size.height * info->act_size.width * 3 / 2;
    //set pixel format
    attr.pixel_format = info->format;
    if (info->format != PIXEL_FORMAT_YVU_PLANAR_420)
    {
        printf("input pix format failed \n");
        return -1;
    }
    // set stride
    attr.stride = (info->act_size.width / 8 - 1) + ((info->act_size.height - 1) << 16);
    // set function
    attr.func = info->func;
    // set scaler attr
    attr.scaler_attr = info->attr;

    // set video layer atrr
    kd_mpi_vo_set_video_layer_attr(chn_id, &attr);

    // enable layer
    kd_mpi_vo_enable_video_layer(chn_id);

    return 0;
}


k_vb_blk_handle sample_vo_insert_frame(k_video_frame_info *vf_info, void **pic_vaddr)
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

    handle = kd_mpi_vb_get_block(cidianbi.plane.g_pool_id, size, NULL);
    if (handle == VB_INVALID_HANDLE)
    {
        printf("%s cidianbi.plane.g_pool_id is %d get vb block error\n", __func__, cidianbi.plane.g_pool_id);
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
    vf_info->pool_id = cidianbi.plane.g_pool_id;
    vf_info->v_frame.phys_addr[0] = phys_addr;
    if (vf_info->v_frame.pixel_format == PIXEL_FORMAT_YVU_PLANAR_420)
        vf_info->v_frame.phys_addr[1] = phys_addr + (vf_info->v_frame.height * vf_info->v_frame.stride[0]);
    *pic_vaddr = virt_addr;

    printf("phys_addr is %lx \n", phys_addr);

    return handle;
}

inline wstring to_width_string(const string input)
{
    wstring_convert<codecvt_utf8<wchar_t>> converter;

    return converter.from_bytes(input);
}

#define PICTURE         "result_nmt.txt"
// test picture 
void sample_vo_filling_color(void *pic_vaddr, string in, string out, bool is_en)
{
    int i = 0;
    void *read_addr = NULL;
    FILE *fd;
    int ret = 0;
    cv::Mat img;

    k_u32 *temp_addr = (k_u32 *)pic_vaddr;
    k_u32 read_size = 64;

    CvxText cv_text_("./kmodel/simsun.ttc");

    // cv::Scalar size(15, 1.0, 0.1, 0);
    // float p = 1.f;
    // cv_text_.setFont(NULL, &size, NULL, &p);

    cv_text_.restoreFont();

    // memset(pic_vaddr, &data, cidianbi.plane.osd.act_size.width * cidianbi.plane.osd.act_size.height * 4);
    memset(pic_vaddr, 0xffffffff, cidianbi.plane.osd.act_size.width * cidianbi.plane.osd.act_size.height * 4);

    img = cv::Mat(cidianbi.plane.osd.act_size.height, cidianbi.plane.osd.act_size.width, CV_8UC(4), pic_vaddr);

    wstring w_str_in = to_width_string(in);
    wstring w_str_out = to_width_string(out);
    
    cv_text_.putText(img, w_str_in.c_str(),cv::Point(10, 80),cv::Scalar(0, 0, 255));
    cv_text_.putText(img, w_str_out.c_str(),cv::Point(10, 140),cv::Scalar(0, 0, 255));
    
    cout << w_str_in.c_str() << "\n" << w_str_out.c_str() << endl;
}


k_s32 sample_vo_layer_config(void)
{
    layer_info info;
    k_vo_layer chn_id = K_VO_LAYER1;

    memset(&info, 0, sizeof(info));

    info.act_size.width = ISP_CHN1_WIDTH;//1080;//640;//1080;
    info.act_size.height = ISP_CHN1_HEIGHT;//1920;//480;//1920;
    info.format = PIXEL_FORMAT_YVU_PLANAR_420;
    info.func = K_ROTATION_0;////K_ROTATION_90;
    info.global_alptha = 0xff;
    info.offset.x = 0;//(1080-w)/2,
    info.offset.y = 0;//(1920-h)/2;
    vo_creat_layer_test(chn_id, &info);

    return 0;
}

int sample_sys_bind_init(void)
{
    k_s32 ret = 0;
    k_mpp_chn vicap_mpp_chn;
    k_mpp_chn vo_mpp_chn;
    vicap_mpp_chn.mod_id = K_ID_VI;
    vicap_mpp_chn.dev_id = cidianbi.vicap_dev;
    vicap_mpp_chn.chn_id = VICAP_CHN_ID_1;

    vo_mpp_chn.mod_id = K_ID_VO;
    vo_mpp_chn.dev_id = K_VO_DISPLAY_DEV_ID;
    vo_mpp_chn.chn_id = K_VO_DISPLAY_CHN_ID1;

    ret = kd_mpi_sys_bind(&vicap_mpp_chn, &vo_mpp_chn);
    if (ret) {
        printf("kd_mpi_sys_unbind failed:0x%x\n", ret);
    }
    return ret;
}

void sample_vicap_unbind_vo(void)
{
    k_s32 ret = 0;
    k_mpp_chn vicap_mpp_chn;
    k_mpp_chn vo_mpp_chn;

    vicap_mpp_chn.mod_id = K_ID_VI;
    vicap_mpp_chn.dev_id = cidianbi.vicap_dev;
    vicap_mpp_chn.chn_id = VICAP_CHN_ID_1;

    vo_mpp_chn.mod_id = K_ID_VO;
    vo_mpp_chn.dev_id = K_VO_DISPLAY_DEV_ID;
    vo_mpp_chn.chn_id = K_VO_DISPLAY_CHN_ID1;

    ret = kd_mpi_sys_unbind(&vicap_mpp_chn, &vo_mpp_chn);
    if (ret) {
        printf("kd_mpi_sys_unbind failed:0x%x\n", ret);
    }
    return;
}


int sample_vivcap_init(void)
{
    k_s32 ret = 0;

    sensor_type =  SC_SC035HGS_MIPI_1LANE_RAW10_640X480_60FPS_LINEAR;
    cidianbi.vicap_dev = VICAP_DEV_ID_0;

    memset(&sensor_info, 0, sizeof(k_vicap_sensor_info));
    ret = kd_mpi_vicap_get_sensor_info(sensor_type, &sensor_info);
    if (ret) {
        printf("sample_vicap, the sensor type not supported!\n");
        return ret;
    }

    memset(&dev_attr, 0, sizeof(k_vicap_dev_attr));
    dev_attr.acq_win.h_start = 0;
    dev_attr.acq_win.v_start = 0;
    dev_attr.acq_win.width = ISP_INPUT_WIDTH;
    dev_attr.acq_win.height = ISP_INPUT_HEIGHT;
    dev_attr.mode = VICAP_WORK_ONLINE_MODE;

    dev_attr.pipe_ctrl.data = 0xFFFFFFFF;
    dev_attr.pipe_ctrl.bits.af_enable = 0;
    dev_attr.pipe_ctrl.bits.ahdr_enable = 0;


    dev_attr.cpature_frame = 0;
    memcpy(&dev_attr.sensor_info, &sensor_info, sizeof(k_vicap_sensor_info));

    ret = kd_mpi_vicap_set_dev_attr(cidianbi.vicap_dev, dev_attr);
    if (ret) {
        printf("sample_vicap, kd_mpi_vicap_set_dev_attr failed.\n");
        return ret;
    }

    memset(&chn_attr, 0, sizeof(k_vicap_chn_attr));

    //set chn0 output yuv420sp
    // chn_attr.out_win = dev_attr.acq_win;
    // chn_attr.crop_win = chn_attr.out_win;
    chn_attr.out_win.h_start = 0;
    chn_attr.out_win.v_start = 0;
    chn_attr.out_win.width = ISP_CHN0_WIDTH;
    chn_attr.out_win.height = ISP_CHN0_HEIGHT;

    // chn_attr.crop_win = dev_attr.acq_win;
    chn_attr.crop_win.h_start = ISP_CROP_W_OFFSET;
    chn_attr.crop_win.v_start = ISP_CROP_H_OFFSET;
    chn_attr.crop_win.width = ISP_CHN0_WIDTH;
    chn_attr.crop_win.height = ISP_CHN0_HEIGHT;

    chn_attr.scale_win = chn_attr.out_win;
    chn_attr.crop_enable = K_FALSE;
    chn_attr.scale_enable = K_FALSE;
    // chn_attr.dw_enable = K_FALSE;
    chn_attr.chn_enable = K_TRUE;
    chn_attr.pix_format = PIXEL_FORMAT_YVU_PLANAR_420;
    chn_attr.buffer_num = VICAP_MAX_FRAME_COUNT;//at least 3 buffers for isp
    chn_attr.buffer_size = VICAP_ALIGN_UP((ISP_CHN0_WIDTH * ISP_CHN0_HEIGHT * 3 / 2), VICAP_ALIGN_1K);;
    cidianbi.vicap_chn = VICAP_CHN_ID_0;

    // printf("sample_vicap ...kd_mpi_vicap_set_chn_attr, buffer_size[%d]\n", chn_attr.buffer_size);
    ret = kd_mpi_vicap_set_chn_attr(cidianbi.vicap_dev, cidianbi.vicap_chn, chn_attr);
    if (ret) {
        printf("sample_vicap, kd_mpi_vicap_set_chn_attr failed.\n");
        return ret;
    }

#if VICAP_VO_TESET
    // config bind 
    sample_sys_bind_init();
    //set chn1 output rgb888p
    chn_attr.out_win.h_start = 0;
    chn_attr.out_win.v_start = 0;
    chn_attr.out_win.width = ISP_CHN1_WIDTH ;
    chn_attr.out_win.height = ISP_CHN1_HEIGHT;

    chn_attr.crop_win.h_start = ISP_CROP_W_OFFSET;
    chn_attr.crop_win.v_start = ISP_CROP_H_OFFSET;
    chn_attr.crop_win.width = ISP_CHN0_WIDTH;
    chn_attr.crop_win.height = ISP_CHN0_HEIGHT;

    chn_attr.scale_win = chn_attr.out_win;
    chn_attr.crop_enable = K_FALSE;
    chn_attr.scale_enable = K_FALSE;
    chn_attr.chn_enable = K_TRUE;
    chn_attr.pix_format = PIXEL_FORMAT_YVU_PLANAR_420;
    chn_attr.buffer_num = VICAP_MAX_FRAME_COUNT;//at least 3 buffers for isp
    chn_attr.buffer_size = VICAP_ALIGN_UP((ISP_CHN1_HEIGHT * ISP_CHN1_WIDTH * 3 ), VICAP_ALIGN_1K);
    chn_attr.fps = 30;

    // printf("sample_vicap ...kd_mpi_vicap_set_chn_attr, buffer_size[%d]\n", chn_attr.buffer_size);
    ret = kd_mpi_vicap_set_chn_attr(cidianbi.vicap_dev, VICAP_CHN_ID_1, chn_attr);
    if (ret) {
        printf("sample_vicap, kd_mpi_vicap_set_chn_attr failed.\n");
        return ret;
    }
#endif
    // set to header file database parse mode
    ret = kd_mpi_vicap_set_database_parse_mode(cidianbi.vicap_dev, VICAP_DATABASE_PARSE_HEADER);
    if (ret) {
        printf("sample_vicap, kd_mpi_vicap_set_database_parse_mode failed.\n");
        return ret;
    }

    // printf("sample_vicap ...kd_mpi_vicap_init\n");
    ret = kd_mpi_vicap_init(cidianbi.vicap_dev);
    if (ret) {
        printf("sample_vicap, kd_mpi_vicap_init failed.\n");
        return ret;
    }
    ret = kd_mpi_vicap_start_stream(cidianbi.vicap_dev);
    if (ret) {
        printf("sample_vicap, kd_mpi_vicap_start_stream failed.\n");
        return ret;
    }
    
    printf("vicap init success -------------------\n");

    return ret;
}


int sample_vivcap_deinit(void)
{
    k_s32 ret;

    ret = kd_mpi_vicap_stop_stream(cidianbi.vicap_dev);
    if (ret) {
        printf("sample_vicap, stop stream failed.\n");
    }
    ret = kd_mpi_vicap_deinit(cidianbi.vicap_dev);
    if (ret) {
        printf("sample_vicap, kd_mpi_vicap_deinit failed.\n");
        return ret;
    }

#if VICAP_VO_TESET
    sample_vicap_unbind_vo();
#endif

    return 0;

}

int sample_gpio_init(void)
{
    int ret;
    int gpio_fd = -1;
    pin_mode_t key0;
    pin_mode_t key1;

    pin_mode_t led0;
    pin_mode_t led1;

    gpio_fd = open("/dev/gpio", O_RDWR);
    if (gpio_fd < 0)
    {
        printf("open /dev/pin err\n");
        return -1;
    }

    key0.pin = KEY_PIN_NUM1;
    key1.pin = KEY_PIN_NUM2;

    led0.pin = LED_PIN_NUM1;
    led1.pin = LED_PIN_NUM2;

    ret = ioctl(gpio_fd, GPIO_DM_INPUT, &key0);
    if (ret)
    {
        printf("ioctl /dev/pin err\n");
        return -1;
    }

    ret = ioctl(gpio_fd, GPIO_DM_INPUT, &key1);
    if (ret)
    {
        printf("ioctl /dev/pin err\n");
        return -1;
    }

    ret = ioctl(gpio_fd, GPIO_DM_OUTPUT, &led0);
    if (ret)
    {
        printf("ioctl /dev/pin err\n");
        return -1;
    }


    ret = ioctl(gpio_fd, GPIO_DM_OUTPUT, &led1);
    if (ret)
    {
        printf("ioctl /dev/pin err\n");
        return -1;
    }

    return gpio_fd;
}
