/* Copyright (c) 2023, Canaan Bright Sight Co., Ltd
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 * 1. Redistributions of source code must retain the above copyright
 * notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 * notice, this list of conditions and the following disclaimer in the
 * documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND
 * CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES,
 * INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 * BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */
#include <unistd.h>
#include <pthread.h>
#include <stdio.h>
#include "msg_venc.h"
#include "mapi_venc_api.h"
#include "mapi_nonai_2d_api.h"
#include "mapi_venc_comm.h"
#include "mapi_sys_api.h"
#include "send_venc_data.h"
// #include "mpi_vicap_api.h"
#include "mpi_vvi_api.h"
// #include "mpi_isp_api.h"

#include <string.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <stdarg.h>
#include <time.h>


#define  ENABLE_VICAP 0
#if  ENABLE_VICAP
    #include "mpi_vicap_api.h"
    #include "mpi_isp_api.h"
#endif

#define CHECK_MAPI_VENC_NULL_PTR(paraname, ptr)                                 \
    do {                                                                       \
        if ((ptr) == NULL) {                                                   \
            mapi_venc_error_trace("%s is NULL pointer\n", paraname); \
            return K_MAPI_ERR_VENC_NULL_PTR;                                      \
        }                                                                      \
    } while (0)

#define MAPI_VVI_UNUSED(x)    ((x)=(x))


typedef struct
{
    pthread_t output_tid;
    k_u32 venc_chn;
    k_bool is_start;
    k_bool is_end;
    k_s32 frame_cnt;
    k_u32 pic_width;
    k_u32 pic_height;

} venc_output_pthread;

static venc_output_pthread venc_output_arr[VENC_MAX_CHN_NUMS];
static kd_venc_callback_s venc_callback_attr[VENC_MAX_CHN_NUMS];

#if ENABLE_VICAP
#define VI_ALIGN_1K 0x400
#define VI_ALIGN_UP(addr, size) (((addr)+((size)-1U))&(~((size)-1U)))

static k_vicap_dev_attr dev_attr;
static void input_vicap_set_dev_attr()
{
    k_s32 ret;
    k_vicap_dev vicap_dev = VICAP_DEV_ID_0;
    k_vicap_sensor_info sensor_info;

    memset(&dev_attr, 0, sizeof(k_vicap_dev_attr));
    memset(&sensor_info, 0, sizeof(k_vicap_sensor_info));

    sensor_info.sensor_type = IMX335_MIPI_2LANE_RAW12_1920X1080_30FPS_LINEAR;
    ret = kd_mpi_vicap_get_sensor_info(sensor_info.sensor_type, &sensor_info);
    if (ret)
    {
        printf("kd_mpi_vicap_get_sensor_info failed:0x%x\n", ret);
    }

    dev_attr.acq_win.width = sensor_info.width;
    dev_attr.acq_win.height = sensor_info.height;
    printf("dev_attr.acq_win.width:%d dev_attr.acq_win.height:%d \n", dev_attr.acq_win.width, dev_attr.acq_win.height);
    dev_attr.mode = VICAP_WORK_ONLINE_MODE;

    memcpy(&dev_attr.sensor_info, &sensor_info, sizeof(k_vicap_sensor_info));

    printf("kd_mpi_vicap_set_dev_attr \n");
    ret = kd_mpi_vicap_set_dev_attr(vicap_dev, dev_attr);
    if (ret)
    {
        printf("kd_mpi_vicap_set_dev_attr failed:0x%x\n", ret);
    }
}

static void input_vicap_set_chn_attr(k_s32 chn_num)
{
    k_s32 ret;
    k_vicap_dev vicap_dev = VICAP_DEV_ID_0;
    k_vicap_chn vicap_chn = chn_num;
    k_vicap_chn_attr chn_attr;
    k_vicap_sensor_info sensor_info;
    memset(&chn_attr, 0, sizeof(k_vicap_chn_attr));

    chn_attr.out_win.width = venc_output_arr[chn_num].pic_width;
    chn_attr.out_win.height = venc_output_arr[chn_num].pic_height;
    printf("chn_num:%d out_win:%d*%d \n", chn_num, chn_attr.out_win.width, chn_attr.out_win.height);
    memcpy(&chn_attr.crop_win, &dev_attr.acq_win, sizeof(k_vicap_window));
    memcpy(&chn_attr.scale_win, &dev_attr.acq_win, sizeof(k_vicap_window));
    chn_attr.crop_enable = K_FALSE;
    chn_attr.scale_enable = K_FALSE;
    chn_attr.chn_enable = K_TRUE;

    chn_attr.pix_format = PIXEL_FORMAT_YUV_SEMIPLANAR_420;
    chn_attr.buffer_num = 6;
    chn_attr.buffer_size = VI_ALIGN_UP(venc_output_arr[chn_num].pic_width * venc_output_arr[chn_num].pic_height * 3 / 2, VI_ALIGN_1K);
    printf("vicap_dev[%d]vicap_chn[%d]  chn_attr.buffer_size:%d \n",
           vicap_dev, vicap_chn, chn_attr.buffer_size);

    ret = kd_mpi_vicap_set_chn_attr(vicap_dev, vicap_chn, chn_attr);
    if (ret)
    {
        printf("kd_mpi_vicap_set_chn_attr failed:0x%x\n", ret);
    }
}

void input_vicap_start()
{
    printf("input_vicap_start\n");
    k_s32 ret = kd_mpi_vicap_init(0);
    if (ret)
    {
        printf("kd_mpi_vicap_init failed:0x%x\n", ret);
    }

    ret = kd_mpi_vicap_start_stream(VICAP_DEV_ID_0);
    if (ret)
    {
        printf("kd_mpi_vicap_start_stream failed:0x%x\n", ret);
    }
}

void input_vicap_stop()
{
    k_s32 ret;
    printf("kd_mpi_vicap_stop_stream \n");
    ret = kd_mpi_vicap_stop_stream(VICAP_DEV_ID_0);
    printf("kd_mpi_vicap_deinit \n");
    ret = kd_mpi_vicap_deinit(VICAP_DEV_ID_0);
    printf("kd_mpi_vicap_deinit end\n");
}
#endif



static void *output_venc_stream_threads(void *arg)
{
    unsigned int out_frames = 0;
    unsigned int out_frames_fps = 0;
    // pthread_detach(pthread_self());
    int chn = *((int *)arg);
    printf("venc_stream_threads %d start ###\n", chn);
    if (chn >= VENC_MAX_CHN_NUMS)
    {
        printf("venc_stream_threads %d error \n", chn);
        return NULL;
    }

    k_s32 ret;
    k_venc_stream output;
    k_bool first_dump = K_TRUE;
    int i;
    int j;
    k_s32 s32Ret = K_SUCCESS;
    // datafifo_msg msg;
    // memset(&msg,0,sizeof(datafifo_msg));
    int num = 0;
    int numframe = 0;
    int datalen = 0;

    int second_msec =  1000;
    struct timespec ts;
    clock_gettime(CLOCK_REALTIME, &ts);
    printf("%.24s %ld Nanoseconds\n", ctime(&ts.tv_sec), ts.tv_nsec);
    unsigned long long os_start = ts.tv_sec * 1000 + ts.tv_nsec / (1000 * 1000);
    unsigned long long os_end = os_start;
    int sendnum = 0;
    int dataret = 0;
    k_char *buf = malloc(BLOCKLEN * sizeof(k_char));
    memset(buf, 0, BLOCKLEN * sizeof(k_char));
    while (venc_output_arr[chn].is_start)
    {
        k_venc_chn_status status;
        ret = kd_mpi_venc_query_status(chn, &status);
        if (status.cur_packs > 0)
            output.pack_cnt = status.cur_packs;
        else
            output.pack_cnt = 1;

        //printf("venc_stream_threads status.cur_packs:%d ###num:%d \n",status.cur_packs, numframe);
        output.pack = malloc(sizeof(k_venc_pack) * output.pack_cnt);
        ret = kd_mpi_venc_get_stream(chn, &output, 1000);
        if (ret || output.pack_cnt <= 0)
        {
            printf("kd_mpi_venc_get_stream[%d] error ret:%d \n", chn, ret);
            callwriteNULLtoflush(chn);
            free(output.pack);
            usleep(1000);
            continue;
        }

        clock_gettime(CLOCK_REALTIME, &ts);
        os_end =   ts.tv_sec * 1000 + ts.tv_nsec / (1000 * 1000);
        sendnum += output.pack_cnt;
        if ((os_end - os_start)  >= second_msec)
        {
            os_start = os_end;
            //printf("venc[%d]fps:%d \n", chn, sendnum);
            sendnum = 0;
        }
        datafifo_msg *pmsg = (datafifo_msg *)buf;
        pmsg->msg_type = MSG_KEY_TYPE;
        pmsg->chn = chn;
        pmsg->upfunc = venc_callback_attr[chn].pfn_data_cb;
        pmsg->puserdata = venc_callback_attr[chn].p_private_data;
        memcpy(&pmsg->status, &status, sizeof(k_venc_chn_status));
        pmsg->status.cur_packs = output.pack_cnt;

        for (i = 0; i < output.pack_cnt; i++)
        {
            pmsg->pack[i].phys_addr = output.pack[i].phys_addr;
            pmsg->pack[i].len = output.pack[i].len;
            pmsg->pack[i].pts = output.pack[i].pts;
            pmsg->pack[i].type = output.pack[i].type;
            if ((pmsg->pack[i].len > 1024 * 1024 || pmsg->pack[i].len < 0) && sendnum == 0)
                printf("[error] send:%d chn:%d  phys_addr:0x%lx len:%d time[%ld]\n", numframe, pmsg->chn, pmsg->pack[i].phys_addr, pmsg->pack[i].len, get_big_currtime());
            numframe ++;
        }
        dataret = -1;

        dataret = send_venc_data_to_little(chn, buf);

        if (dataret < 0)
            ret = kd_mpi_venc_release_stream(chn, &output);

        free(output.pack);
    }
    free(buf);

    printf("venc_stream_threads %d exit ###\n", chn);
    venc_output_arr[chn].is_start = K_FALSE;
    venc_output_arr[chn].is_end = K_TRUE;

    return NULL;
}



k_s32 kd_mapi_venc_init(k_u32 chn_num, k_venc_chn_attr *pst_venc_attr)
{
    printf("kd_mapi_venc_init start %d\n", chn_num);
    memset(&venc_output_arr[chn_num], 0, sizeof(venc_output_pthread));
    memset(&venc_callback_attr[chn_num], 0, sizeof(kd_venc_callback_s));
    k_s32 ret;
    CHECK_MAPI_VENC_NULL_PTR("k_venc_chn_attr", pst_venc_attr);
    venc_output_arr[chn_num].pic_width = pst_venc_attr->venc_attr.pic_width;
    venc_output_arr[chn_num].pic_height = pst_venc_attr->venc_attr.pic_height;
    printf("venc[%d] %d*%d size:%d cnt:%d srcfps:%d dstfps:%d rate:%d rc_mode:%d type:%d profile:%d\n", chn_num,
           pst_venc_attr->venc_attr.pic_width,
           pst_venc_attr->venc_attr.pic_height,
           pst_venc_attr->venc_attr.stream_buf_size,
           pst_venc_attr->venc_attr.stream_buf_cnt,
           pst_venc_attr->rc_attr.cbr.src_frame_rate,
           pst_venc_attr->rc_attr.cbr.dst_frame_rate,
           pst_venc_attr->rc_attr.cbr.bit_rate,
           (int)pst_venc_attr->rc_attr.rc_mode,
           (int)pst_venc_attr->venc_attr.type,
           (int)pst_venc_attr->venc_attr.profile);

    ret = kd_mpi_venc_create_chn(chn_num, pst_venc_attr);
    if (ret != K_SUCCESS)
    {
        mapi_venc_error_trace("kd_mpi_venc_create_chn failed:0x%x\n", ret);
        return VENC_RET_MPI_TO_MAPI(ret);
    }

    printf("kd_mapi_venc_init end \n");
    return K_SUCCESS;
}

k_s32 kd_mapi_venc_deinit(k_u32 chn_num)
{
    printf("kd_mapi_venc_deinit start %d\n", chn_num);
    if (chn_num >= VENC_MAX_CHN_NUMS)
    {
        mapi_venc_error_trace("kd_mapi_venc_deinit chn_num:%d error\n", chn_num);
        return -1;
    }
    k_s32 ret = kd_mpi_venc_destroy_chn(chn_num);
    if (ret)
        printf("kd_mpi_venc_destroy_chn error ret:%d \n", ret);

    printf("kd_mapi_venc_deinit end \n");
    return ret;
}



k_s32 kd_mapi_venc_start(k_s32 chn_num, k_s32 s32_frame_cnt)
{
    mapi_venc_error_trace("start chn_num:%d s32_frame_cnt:%d \n", chn_num, s32_frame_cnt);
    k_s32 ret;
    if (chn_num >= VENC_MAX_CHN_NUMS)
    {
        mapi_venc_error_trace("kd_mapi_venc_start chn_num:%d error\n", chn_num);
        return -1;
    }
    ret = kd_mpi_venc_start_chn(chn_num);
    if (ret != K_SUCCESS)
    {
        mapi_venc_error_trace("kd_mapi_venc_start failed:0x%x\n", ret);
        return VENC_RET_MPI_TO_MAPI(ret);
    }

    venc_output_arr[chn_num].is_start = K_TRUE;
    venc_output_arr[chn_num].is_end = K_FALSE;
    venc_output_arr[chn_num].venc_chn = chn_num;
    venc_output_arr[chn_num].frame_cnt = s32_frame_cnt;
    pthread_create(&venc_output_arr[chn_num].output_tid, NULL, output_venc_stream_threads, &venc_output_arr[chn_num].venc_chn);

    mapi_venc_error_trace("end chn_num[%d].output_tid:%d  \n", chn_num, venc_output_arr[chn_num].output_tid);
    return K_SUCCESS;
}

k_s32 kd_mapi_venc_stop(k_s32 chn_num)
{
    mapi_venc_error_trace("start chn_num:%d \n", chn_num);
    if (chn_num >= VENC_MAX_CHN_NUMS)
    {
        mapi_venc_error_trace("kd_mapi_venc_stop chn_num:%d error\n", chn_num);
        return -1;
    }

    mapi_venc_error_trace("output_tid:%d \n", venc_output_arr[chn_num].output_tid);

    if (venc_output_arr[chn_num].output_tid != 0)
    {
        usleep(1000 * 1000); //vicap stop and have stream
        venc_output_arr[chn_num].is_start = K_FALSE;

        while (venc_output_arr[chn_num].is_end)
        {
            usleep(500);
        }
        pthread_join(venc_output_arr[chn_num].output_tid, NULL);


        venc_output_arr[chn_num].output_tid = 0;
    }

    k_s32 ret = kd_mpi_venc_stop_chn(chn_num);
    if (ret != K_SUCCESS)
    {
        mapi_venc_error_trace("kd_mpi_venc_stop_chn failed:0x%x\n", ret);
        return VENC_RET_MPI_TO_MAPI(ret);
    }
    mapi_venc_error_trace("end \n");
    return ret;
}

k_s32 kd_mapi_venc_bind_vi(k_s32 src_dev, k_s32 src_chn, k_s32 chn_num)
{
    mapi_venc_error_trace("start %d\n", chn_num);
    if (chn_num >= VENC_MAX_CHN_NUMS)
    {
        mapi_venc_error_trace("kd_mapi_venc_bind_vi chn_num:%d error\n", chn_num);
        return -1;
    }
#if ENABLE_VICAP
    if (chn_num == 0)
    {
        input_vicap_set_dev_attr();
    }
    input_vicap_set_chn_attr(chn_num);
#endif

    k_mpp_chn vi_mpp_chn;
    k_mpp_chn venc_mpp_chn;


    vi_mpp_chn.mod_id = K_ID_VI;
    vi_mpp_chn.dev_id = src_dev;
    vi_mpp_chn.chn_id = src_chn;
    venc_mpp_chn.mod_id = K_ID_VENC;
    venc_mpp_chn.dev_id = 0;
    venc_mpp_chn.chn_id = chn_num;
    k_s32 ret = kd_mpi_sys_bind(&vi_mpp_chn, &venc_mpp_chn);

#if ENABLE_VICAP
    int i = 0;
    int last = VENC_MAX_CHN_NUMS;
    for (i = 0; i < VENC_MAX_CHN_NUMS; i++)
    {
        if (venc_callback_attr[i].pfn_data_cb != NULL)
        {
            last = i;
        }
    }
    printf("start vi last:%d \n", last);
    if (last == chn_num)
    {
        input_vicap_start();
    }
#endif
    mapi_venc_error_trace("end \n");
    return ret;
}

k_s32 kd_mapi_venc_unbind_vi(k_s32 src_dev, k_s32 src_chn, k_s32 chn_num)
{
    mapi_venc_error_trace("start vicap stop %d\n", chn_num);
#if ENABLE_VICAP
    int i = 0;
    int last = VENC_MAX_CHN_NUMS;
    for (i = 0; i < VENC_MAX_CHN_NUMS; i++)
    {
        if (venc_callback_attr[i].pfn_data_cb != NULL)
        {
            last = i;
        }
    }
    printf("stop vi last:%d \n", last);
    if (last == chn_num)
    {
        input_vicap_stop();
    }
#endif
    k_mpp_chn vi_mpp_chn;
    k_mpp_chn venc_mpp_chn;

    vi_mpp_chn.mod_id = K_ID_VI;
    vi_mpp_chn.dev_id = src_dev;
    vi_mpp_chn.chn_id = src_chn;

    venc_mpp_chn.mod_id = K_ID_VENC;
    venc_mpp_chn.dev_id = 0;
    venc_mpp_chn.chn_id = chn_num;
    k_s32 ret = kd_mpi_sys_unbind(&vi_mpp_chn, &venc_mpp_chn);

    mapi_venc_error_trace("vicap stop end %d\n", chn_num);

    return  ret;
}



k_s32 kd_mapi_venc_registercallback(k_u32 chn_num, kd_venc_callback_s *pst_venc_cb)
{
    mapi_venc_error_trace("%d ###\n", chn_num);
    if (chn_num >= VENC_MAX_CHN_NUMS)
    {
        mapi_venc_error_trace("venc chn %d error\n", chn_num);
        return -1;
    }
    CHECK_MAPI_VENC_NULL_PTR("pst_venc_cb", pst_venc_cb);

    venc_callback_attr[chn_num].pfn_data_cb = pst_venc_cb->pfn_data_cb;
    venc_callback_attr[chn_num].p_private_data = pst_venc_cb->p_private_data;

    return K_SUCCESS;
}



k_s32 kd_mapi_venc_unregistercallback(k_u32 chn_num, kd_venc_callback_s *pst_venc_cb)
{
    mapi_venc_error_trace("%d ###\n", chn_num);
    if (chn_num >= VENC_MAX_CHN_NUMS)
    {
        mapi_venc_error_trace("venc chn %d error\n", chn_num);
        return -1;
    }
    CHECK_MAPI_VENC_NULL_PTR("pst_venc_cb", pst_venc_cb);

    venc_callback_attr[chn_num].pfn_data_cb = NULL;
    venc_callback_attr[chn_num].p_private_data = 0;

    return K_SUCCESS;
}

k_s32 kd_mapi_venc_enable_idr(k_s32 chn_num, k_bool idr_enable)
{
    printf("%s\n", __FUNCTION__);
    mapi_venc_error_trace("chn_num:%d idr_enable:%d \n", chn_num, (int)idr_enable);
    return kd_mpi_venc_enable_idr(chn_num, idr_enable);
}

k_s32 kd_mapi_venc_request_idr(k_s32 chn_num)
{
    printf("%s\n", __FUNCTION__);
    return kd_mpi_venc_request_idr(chn_num);
}

k_s32 kd_mapi_nonai_2d_init(k_u32 chn_num, k_nonai_2d_chn_attr *attr)
{
    printf("%s\n", __FUNCTION__);
    return kd_mpi_nonai_2d_create_chn(chn_num, attr);
}

k_s32 kd_mapi_nonai_2d_deinit(k_u32 chn_num)
{
    printf("%s\n", __FUNCTION__);
    return kd_mpi_nonai_2d_destroy_chn(chn_num);
}

k_s32 kd_mapi_nonai_2d_start(k_u32 chn_num)
{
    printf("%s\n", __FUNCTION__);
    return kd_mpi_nonai_2d_start_chn(chn_num);
}

k_s32 kd_mapi_nonai_2d_stop(k_u32 chn_num)
{
    printf("%s\n", __FUNCTION__);
    return kd_mpi_nonai_2d_stop_chn(chn_num);
}
