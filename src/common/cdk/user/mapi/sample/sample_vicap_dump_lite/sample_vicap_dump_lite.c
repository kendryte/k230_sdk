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
#include <stdio.h>
#include <sys/mman.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include "mapi_sys_api.h"
#include "mapi_vicap_api.h"
#include "k_vicap_comm.h"

typedef struct {
    k_u32 page_size;
    k_u64 page_mask;
} kd_ts_sys_paga_vi_cap;

static kd_ts_sys_paga_vi_cap sys_page = {0};
static k_video_frame_info dump_pic_info = {0};
static k_s32 mmap_fd = -1;

void kd_ts_mmap(k_u64 phys_addr, k_u32 size, k_u32 **v_addr)
{
    sys_page.page_size = sysconf(_SC_PAGESIZE);
    sys_page.page_mask = sys_page.page_size - 1;
#ifdef USE_RT_SMART
    *v_addr = (k_u32 *)kd_mpi_sys_mmap(phys_addr, size);
#else
    void *mmap_addr = NULL;
    k_u32 mmap_size = (((size) + (sys_page.page_size) - 1) & ~((sys_page.page_size) - 1));

    if(mmap_fd == -1)
    {
        mmap_fd = open("/dev/mem", O_RDWR | O_SYNC);
    }

    mmap_addr = mmap(NULL, mmap_size, PROT_READ | PROT_WRITE, MAP_SHARED, mmap_fd, phys_addr & ~sys_page.page_mask);

    if(mmap_addr)
    {
        *v_addr = mmap_addr + (phys_addr & sys_page.page_mask);
    }
#endif
    if(*v_addr == NULL)
    {
        printf("mmap error\n");
        return;
    }
    printf("mmap success\n");
    return;
}

int main(void)
{
    k_s32 ret;
    k_video_frame dump_vf_info;
    k_u32 *dump_virt_addr = NULL;
    k_char filename[2048];
    k_u32 num = 0;

    ret = kd_mapi_sys_init();
    if(ret != K_SUCCESS)
    {
        printf("kd_mapi_sys_init error: %x\n", ret);
        goto exit;
    }

    if(mmap_fd == -1)
    {
        mmap_fd = open("/dev/mem", O_RDWR | O_SYNC);
    }

    printf("opne mem success \n");

    while(1)
    {
        printf("Press Enter to continue!!!!\n");
        getchar();
        // ov9732
        ret = kd_mapi_vicap_dump_frame(VICAP_DEV_ID_0, VICAP_CHN_ID_0, VICAP_DUMP_YUV, &dump_pic_info, 100);
        // ov9286
        // ret = kd_mapi_vicap_dump_frame(VICAP_DEV_ID_1, VICAP_CHN_ID_0, VICAP_DUMP_YUV, &dump_pic_info, 100);

        dump_vf_info = dump_pic_info.v_frame;
        printf("ret is %d dump_pic_info phy addr is %x \n", ret, dump_pic_info.v_frame.phys_addr[0]);
        printf("dump_vf_info.width: %d, dump_vf_info.height: %d\n", dump_pic_info.v_frame.width, dump_pic_info.v_frame.height);

        kd_ts_mmap(dump_vf_info.phys_addr[0], dump_vf_info.width * dump_vf_info.height * 3 / 2, &dump_virt_addr); // map to dump_virt_addr

        sprintf(filename, "yuv_%dWx%dH_NV12_%05d.yuv", dump_vf_info.width, dump_vf_info.height, num);
        printf("save yuv file %s\n", filename);
        FILE *fd = fopen(filename, "wb");
        fwrite(dump_virt_addr, 1, dump_vf_info.width * dump_vf_info.height * 3 / 2, fd);
        fclose(fd);

        if(ret >= 0)
        {
            sleep(1);
            // ov9732
            ret = kd_mapi_vicap_release_frame(VICAP_DEV_ID_0, VICAP_CHN_ID_0, &dump_pic_info);
            // ov9286
            // ret = kd_mapi_vicap_release_frame(VICAP_DEV_ID_1, VICAP_CHN_ID_0, &dump_pic_info);
        }
        else
        {
            sleep(1);
        }
        num++;
    }

    printf("Press Enter to exit!!!!\n");
    getchar();
    ret |= kd_mapi_sys_deinit();

    return 0;
exit:
    if(ret)
    {
        printf("sample code run error\n");
    }
    printf("Exit!!!!\n");
    return 0;
}
