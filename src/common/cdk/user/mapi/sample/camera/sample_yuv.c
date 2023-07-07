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
#include <sys/mman.h>
#include <fcntl.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <sys/ioctl.h>
#include <errno.h>
#include <inttypes.h>

#include "log.h"
#include "frame_cache.h"
#include "sample_yuv.h"
#include "mapi_sys_api.h"
#include "mapi_vicap_api.h"
#include "k_vicap_comm.h"

typedef struct sample_getyuv_s
{
    k_bool thread_start;
    k_u32 dev_num;
    k_u32 chn_num;
} sample_getyuv_para_s;

static sample_getyuv_para_s g_para;
static pthread_t g_loop_yuv_frame_pid;

static k_s32 mmap_fd = -1;

static k_video_frame_info dump_pic_info = {0};

static FILE *g_pfd = NULL;

long long timediff(struct timeval end, struct timeval start)
{
	signed long l;

	end.tv_sec -= start.tv_sec;
	l = (signed long) end.tv_usec - (signed long) start.tv_usec;
	if (l < 0) {
		end.tv_sec--;
		l = 1000000+l;
	}
	return ((long long)end.tv_sec * (long long)1000000) + (long long)l;
}

static void sample_yuv_get_buf_size(const k_video_frame *v_frame, k_u32 *size)
{
    k_pixel_format p_format = v_frame->pixel_format;

    if(PIXEL_FORMAT_YUV_SEMIPLANAR_420 == v_frame->pixel_format)
    {
        *size = v_frame->width * v_frame->height * 3 / 2;
    }
    else
    {
        printf("%s:%d format not support! \n", __func__, p_format);
    }

    return;
}

static void sample_yuv_dump(const k_video_frame *v_frame, FILE *g_pfd)
{
    k_pixel_format p_format = v_frame->pixel_format;

    uvc_cache_t *uvc_cache = NULL;
    frame_node_t *fnode = NULL;
    k_u32 size;
    k_u32 mmap_size;
    void *mmap_addr = NULL;
    k_u32 * temp_virt_addr = NULL;
    k_u32 page_size = sysconf(_SC_PAGESIZE);
    k_u64 page_mask = (page_size - 1);

    if(0 == v_frame->phys_addr[0])
        return;

    sample_yuv_get_buf_size(v_frame, &size);
    mmap_size = ((size) + page_size - 1) & (~page_mask);

    if(mmap_fd == -1)
        mmap_fd = open("/dev/mem", O_RDWR | O_SYNC);
    mmap_addr = mmap(NULL, mmap_size, PROT_READ | PROT_WRITE, MAP_SHARED,
                    mmap_fd, v_frame->phys_addr[0] & ~page_mask);
    if(!mmap_addr)
    {
        goto err_exit;
    }
    temp_virt_addr = mmap_addr + (v_frame->phys_addr[0] & page_mask);

    //get free cache node
    uvc_cache = uvc_cache_get();
    if (uvc_cache)
    {
        get_node_from_queue(uvc_cache->free_queue, &fnode);
    }

    if (!fnode)
    {
        goto err_exit;
    }

    k_u32 *node_ptr = (k_u32 *)fnode->mem;
    fnode->used = 0;

    memcpy(node_ptr, temp_virt_addr, size);

    fnode->used = size;

#if (1 == UVC_SAVE_FILE)
    fwrite(temp_virt_addr, size, 1, g_pfd);
    fflush(g_pfd);
#endif

err_exit:
    if (fnode)
    {
        put_node_to_queue(uvc_cache->ok_queue, fnode);
    }
    if (mmap_addr)
    {
        munmap(mmap_addr, mmap_size);
        mmap_addr = NULL;
    }
    return;
}

void* loop_yuv_frame_thread(void *p)
{
    k_s32 ret;

     sample_getyuv_para_s *para = (sample_getyuv_para_s*)p;

#if (1 == UVC_SAVE_FILE)
    char yuv_name[128];
    snprintf(yuv_name, 128, "output/dev%d_chn%d.yuv", para->dev_num, para->chn_num);
    g_pfd = fopen(yuv_name, "wb");
    if (NULL == g_pfd)
    {
        printf("open file %s err!\n", yuv_name);
        return NULL;
    }
#endif

    while (para->thread_start)
    {
        ret = kd_mapi_vicap_dump_frame(para->dev_num,
                                        para->chn_num,
                                        VICAP_DUMP_YUV,
                                        &dump_pic_info,
                                        100);

        // printf("get frame phys addr:%lx ret i %d \n", dump_pic_info.v_frame.phys_addr[0], ret);
        if(ret >= 0)
        {
            sample_yuv_dump(&dump_pic_info.v_frame, g_pfd);
            ret = kd_mapi_vicap_release_frame(para->dev_num, para->chn_num, &dump_pic_info);
        }
    }

    if (NULL != g_pfd)
    {
        fclose(g_pfd);
        g_pfd = NULL;
    }

    return NULL;
}

k_s32 start_get_yuv(k_u32 dev_num, k_u32 chn_num)
{
    g_para.thread_start = K_TRUE;
    g_para.dev_num = dev_num;
    g_para.chn_num = chn_num;
    return pthread_create(&g_loop_yuv_frame_pid, NULL, loop_yuv_frame_thread, (void *)&g_para);
}

k_s32 stop_get_yuv(void)
{
    if (K_TRUE == g_para.thread_start)
    {
        g_para.thread_start = K_FALSE;
        pthread_join(g_loop_yuv_frame_pid, NULL);
    }
    return K_SUCCESS;
}
