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

#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include "media.h"

int Media::Init() {
    k_vb_config vb_config;
    memset(&vb_config, 0, sizeof(vb_config));
    // vb for audio
    vb_config.max_pool_cnt = 64;
    vb_config.comm_pool[0].blk_cnt = 150;
    vb_config.comm_pool[0].blk_size = audio_sample_rate_ * 2 * 4 / audio_persec_div_num_;
    vb_config.comm_pool[0].mode = VB_REMAP_MODE_NOCACHE;
    vb_config.comm_pool[1].blk_cnt = 10;
    vb_config.comm_pool[1].blk_size = audio_sample_rate_ * 2 * 4 / audio_persec_div_num_ * 2;
    vb_config.comm_pool[1].mode = VB_REMAP_MODE_NOCACHE;

    // vb for video
    k_u64 pic_size = venc_width_ * venc_height_ * 3 / 2;
    k_u64 stream_size = venc_width_ * venc_height_ / 2;
    vb_config.comm_pool[2].blk_cnt = 3;
    vb_config.comm_pool[2].blk_size = ((pic_size + 0xfff) & ~0xfff);
    vb_config.comm_pool[2].mode = VB_REMAP_MODE_NOCACHE;
    vb_config.comm_pool[3].blk_cnt = 3;
    vb_config.comm_pool[3].blk_size = ((stream_size + 0xfff) & ~0xfff);
    vb_config.comm_pool[3].mode = VB_REMAP_MODE_NOCACHE;

    // vb for YUV420SP output
    vb_config.comm_pool[4].blk_cnt = 3;
    vb_config.comm_pool[4].mode = VB_REMAP_MODE_NOCACHE;
    vb_config.comm_pool[4].blk_size = VICAP_ALIGN_UP((venc_width_ * venc_height_ * 3 / 2), 0x1000);
    
    // vb for RGB888 output
    // vb_config.comm_pool[5].blk_cnt = 0;
    // vb_config.comm_pool[5].mode = VB_REMAP_MODE_NOCACHE;
    // vb_config.comm_pool[5].blk_size = VICAP_ALIGN_UP((detect_height_ * detect_width_ * 3 ), 0x1000);

    memset(&sensor_info_, 0, sizeof(sensor_info_));
    kd_mpi_vicap_get_sensor_info(sensor_type_, &sensor_info_);

    vb_config.comm_pool[5].blk_cnt = 3;
    vb_config.comm_pool[5].blk_size = VICAP_ALIGN_UP(sensor_info_.width * sensor_info_.height * 3 / 2, 0x1000);
    vb_config.comm_pool[5].mode = VB_REMAP_MODE_NOCACHE;

    vb_config.comm_pool[6].blk_cnt = 3;
    vb_config.comm_pool[6].blk_size = ((pic_size + 0xfff) & ~0xfff);
    vb_config.comm_pool[6].mode = VB_REMAP_MODE_NOCACHE;
    vb_config.comm_pool[7].blk_cnt = 3;
    vb_config.comm_pool[7].blk_size = ((stream_size + 0xfff) & ~0xfff);
    vb_config.comm_pool[7].mode = VB_REMAP_MODE_NOCACHE;

    k_vb_supplement_config vb_supp;
    memset(&vb_supp, 0, sizeof(vb_supp));
    vb_supp.supplement_config |= VB_SUPPLEMENT_JPEG_MASK;

    /* vb pool config */
    k_s32 ret = kd_mpi_vb_set_config(&vb_config);
    if(ret != K_SUCCESS) {
        return -1;
    }

    /* vb supplenet config */
    ret = kd_mpi_vb_set_supplement_config(&vb_supp);
    if(ret != K_SUCCESS) {
        return -1;
    }

    /* vb pool init */
    ret = kd_mpi_vb_init();
    if(ret != K_SUCCESS) {
        return -1;
    }

    return 0;
}

void Media::DeInit() {
    kd_mpi_vb_exit();
}

int Media::VcapSetDevAttr() {
    k_vicap_dev_attr dev_attr;
    memset(&dev_attr, 0, sizeof(dev_attr));
    dev_attr.acq_win.h_start = 0;
    dev_attr.acq_win.v_start = 0;
    dev_attr.acq_win.width = sensor_info_.width;
    dev_attr.acq_win.height = sensor_info_.height;
    dev_attr.mode = VICAP_WORK_ONLINE_MODE;
    dev_attr.dw_enable = K_TRUE;
    dev_attr.pipe_ctrl.data = 0xFFFFFFFF;
    dev_attr.pipe_ctrl.bits.af_enable = 0;
    dev_attr.pipe_ctrl.bits.ahdr_enable = 0;
    dev_attr.pipe_ctrl.bits.dnr3_enable = 0;
    dev_attr.cpature_frame = 0;
    memcpy(&dev_attr.sensor_info, &sensor_info_, sizeof(k_vicap_sensor_info));

    k_s32 ret = kd_mpi_vicap_set_dev_attr(vicap_dev_, dev_attr);
    if (ret) {
        std::cout << "sample_vicap, kd_mpi_vicap_set_dev_attr failed" << std::endl;
        return ret;
    }

    return 0;
}

int Media::VcapSetChnAttr(k_vicap_chn vicap_chn, const VcapChnAttr &vcap_attr) {
    k_vicap_chn_attr vicap_chn_attr;
    memset(&vicap_chn_attr, 0,sizeof(vicap_chn_attr));
    vicap_chn_attr.out_win.h_start = 0;
    vicap_chn_attr.out_win.v_start = 0;
    vicap_chn_attr.out_win.width = vcap_attr.output_width;
    vicap_chn_attr.out_win.height = vcap_attr.output_height;
    vicap_chn_attr.crop_win.h_start = 768;
    vicap_chn_attr.crop_win.v_start = 16;
    vicap_chn_attr.crop_win.width = vcap_attr.crop_width;
    vicap_chn_attr.crop_win.height = vcap_attr.crop_height;
    vicap_chn_attr.scale_win = vicap_chn_attr.out_win;
    vicap_chn_attr.crop_enable = K_TRUE;
    vicap_chn_attr.scale_enable = K_FALSE;
    vicap_chn_attr.chn_enable = K_TRUE;
    vicap_chn_attr.alignment = 12;
    vicap_chn_attr.pix_format = vcap_attr.pixel_format;
    vicap_chn_attr.buffer_num = 3;
    vicap_chn_attr.buffer_size = VICAP_ALIGN_UP(sensor_info_.width * sensor_info_.height * 3 / 2, 0x1000);

    k_s32 ret = kd_mpi_vicap_set_chn_attr(vicap_dev_, vicap_chn, vicap_chn_attr);
    if (ret) {
        std::cout << "sample_vicap, kd_mpi_vicap_set_chn_attr failed." << std::endl;
        return ret;
    }

    return 0;
}

int Media::VcapInit() {
    k_s32 ret;
    ret = kd_mpi_vicap_set_database_parse_mode(vicap_dev_, VICAP_DATABASE_PARSE_HEADER);
    if (ret) {
        printf("sample_vicap, kd_mpi_vicap_set_database_parse_mode failed.\n");
        return ret;
    }
    ret = kd_mpi_vicap_init(vicap_dev_);
    if (ret) {
        std::cout << "sample_vicap, kd_mpi_vicap_init failed." << std::endl;
        return ret;
    }
    return 0;
}

int Media::VcapStart() {
    k_s32 ret = kd_mpi_vicap_start_stream(vicap_dev_);
    if (ret) {
        std::cout << "sample_vicap, kd_mpi_vicap_start_stream failed." << std::endl;
        return ret;
    }

    return 0;
}

int Media::VcapStop() {
    k_s32 ret = kd_mpi_vicap_stop_stream(vicap_dev_);
    if (ret) {
        std::cout  << "sample_vicap, kd_mpi_vicap_stop_stream failed." << std::endl;
        return ret;
    }

    ret = kd_mpi_vicap_deinit(vicap_dev_);
    if (ret) {
        std::cout << "sample_vicap, kd_mpi_vicap_deinit failed." << std::endl;
        return ret;
    }

    return 0;
}

int Media::VcapGetDumpFrame(k_vicap_chn vicap_chn, k_video_frame_info *dump_frame, void *vap_ai_vaddr) {
    k_s32 ret = kd_mpi_vicap_dump_frame(vicap_dev_, vicap_chn, VICAP_DUMP_YUV, dump_frame, 1000);
    if (ret) {
        std::cout << "VcapGetDumpFrame failed." << std::endl;
        return -1;
    }

    k_u32 size = 3 * detect_width_ * detect_height_;
    void *vbvaddr = kd_mpi_sys_mmap_cached(dump_frame->v_frame.phys_addr[0], size);
    memcpy(vap_ai_vaddr, (void *)vbvaddr, size);
    kd_mpi_sys_munmap(vbvaddr, size);

    return 0;
}

int Media::VcapReleaseDumpFrame(k_vicap_chn vicap_chn, const k_video_frame_info *dump_frame) {
    k_s32 ret = kd_mpi_vicap_dump_release(vicap_dev_, vicap_chn, dump_frame);
    if (ret) {
        std::cout << "VcapReleaseDumpFrame failed." << std::endl;
        return -1;
    }

    return 0;
}

int Media::MediaSysAlloc(k_u64 *phy_addr, void **virt_addr, k_u32 len) {
    k_s32 ret = kd_mpi_sys_mmz_alloc_cached(phy_addr, virt_addr, "allocate", "anonymous", len);
    if (ret) {
        std::cout << "MediaSysAlloc failed." << std::endl;
        return -1;
    }

    return 0;
}

int Media::MediaSysFree(k_u64 phy_addr, void *virt_addr) {
    k_s32 ret = kd_mpi_sys_mmz_free(phy_addr, virt_addr);
    if (ret) {
        std::cout << "MediaSysFree failed." << std::endl;
        return -1;
    }

    return 0;
}

int Media::VoInit() {
    VoLyaerVdssBindVoConfig();

    k_mpp_chn srcModuleVi;
    k_mpp_chn dstModuleVo;
    memset(&srcModuleVi, 0, sizeof(srcModuleVi));
    memset(&dstModuleVo, 0, sizeof(dstModuleVo));
    srcModuleVi.mod_id = K_ID_VI;
    srcModuleVi.dev_id = VICAP_DEV_ID_0;
    srcModuleVi.chn_id = VICAP_CHN_ID_0;
    dstModuleVo.mod_id = K_ID_VO;
    dstModuleVo.dev_id = K_VO_DISPLAY_DEV_ID;
    dstModuleVo.chn_id = K_VO_DISPLAY_CHN_ID1;
    k_s32 ret = kd_mpi_sys_bind(&srcModuleVi, &dstModuleVo);
    if (ret) {
        std::cout << "kd_mpi_sys_bind failed, bind vi to vo" << std::endl;
        return ret;
    }

    return 0;
}

int Media::VoConnectorInit() {
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
    // connector init
    kd_mpi_connector_init(connector_fd, connector_info);

    return 0;
}

int Media::VoInsertFrame(k_video_frame_info *vf_info) {
    int ret = kd_mpi_vo_chn_insert_frame(K_VO_OSD3 + 3, vf_info);
    if (ret) {
        printf("VoInsertFrame failed.\n");
        return -1;
    }

    return 0;
}

uint32_t Media::VoGetInserFrame(k_video_frame_info *vf_info, void **pic_vaddr) {
    k_u64 phys_addr = 0;
    k_u32 *virt_addr;
    k_vb_blk_handle handle;
    k_s32 size;

    k_vo_video_osd_attr attr;
    attr.global_alptha = 0xff;
    attr.stride = osd_width_ * 4 / 8;
    attr.pixel_format = PIXEL_FORMAT_ARGB_8888;
    attr.display_rect.x = 0;
    attr.display_rect.y = 0;
    attr.img_size.width = osd_width_;
    attr.img_size.height = osd_height_;
    kd_mpi_vo_set_video_osd_attr(K_VO_OSD3, &attr);
    kd_mpi_vo_osd_enable(K_VO_OSD3);

    k_vb_pool_config pool_config;
    memset(&pool_config, 0, sizeof(pool_config));
    pool_config.blk_size = VICAP_ALIGN_UP((osd_width_ * osd_height_ * 4 * 2), 0x1000);
    pool_config.blk_cnt = 1;
    pool_config.mode = VB_REMAP_MODE_NOCACHE;
    osd_pool_id_ = kd_mpi_vb_create_pool(&pool_config);

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

    size = size + 4096;

    handle = kd_mpi_vb_get_block(osd_pool_id_, size, NULL);
    if (handle == VB_INVALID_HANDLE) {
        printf("%s get vb block error\n", __func__);
        return K_FAILED;
    }

    phys_addr = kd_mpi_vb_handle_to_phyaddr(handle);
    if (phys_addr == 0) {
        printf("%s get phys addr error\n", __func__);
        return K_FAILED;
    }

    virt_addr = (k_u32 *)kd_mpi_sys_mmap(phys_addr, size);
    if (virt_addr == NULL) {
        printf("%s mmap error\n", __func__);
        return K_FAILED;
    }

    vf_info->mod_id = K_ID_VO;
    vf_info->pool_id = osd_pool_id_;
    vf_info->v_frame.phys_addr[0] = phys_addr;
    if (vf_info->v_frame.pixel_format == PIXEL_FORMAT_YVU_PLANAR_420)
        vf_info->v_frame.phys_addr[1] = phys_addr + (vf_info->v_frame.height * vf_info->v_frame.stride[0]);
    *pic_vaddr = virt_addr;

    return handle;
}

int Media::VoReleaseInserFrame(uint32_t inser_handle) {
    kd_mpi_vo_osd_disable(K_VO_OSD3);
    kd_mpi_vb_release_block(k_vb_blk_handle(inser_handle));
    int ret = kd_mpi_vb_destory_pool(osd_pool_id_);
    if (ret)
        printf("vb destory pool ret = %d, osd_pool_id = %d...\n", ret, osd_pool_id_);

    return 0;
}

int Media::VoDeInit() {
    kd_mpi_vo_disable_video_layer(K_VO_LAYER1);

    k_mpp_chn srcModuleVi;
    k_mpp_chn dstModuleVo;
    memset(&srcModuleVi, 0, sizeof(srcModuleVi));
    memset(&dstModuleVo, 0, sizeof(dstModuleVo));
    srcModuleVi.mod_id = K_ID_VI;
    srcModuleVi.dev_id = VICAP_DEV_ID_0;
    srcModuleVi.chn_id = VICAP_CHN_ID_0;
    dstModuleVo.mod_id = K_ID_VO;
    dstModuleVo.dev_id = K_VO_DISPLAY_DEV_ID;
    dstModuleVo.chn_id = K_VO_DISPLAY_CHN_ID1;
    k_s32 ret = kd_mpi_sys_unbind(&srcModuleVi, &dstModuleVo);
    if (ret) {
        std::cout << "kd_mpi_sys_unbind failed, bind venc snap to vicap chn 2" << std::endl;
        return ret;
    }

    return 0;
}

int Media::VencSnapChnCreate(int chn, int width, int height) {
    k_venc_chn_attr attr;
    memset(&attr, 0, sizeof(attr));
    attr.venc_attr.pic_width = width;
    attr.venc_attr.pic_height = height;
    k_u64 stream_size = width * height / 2;
    attr.venc_attr.stream_buf_size = VICAP_ALIGN_UP(stream_size, 0x1000);
    attr.venc_attr.stream_buf_cnt = 3;
    attr.rc_attr.rc_mode = K_VENC_RC_MODE_MJPEG_FIXQP;
    attr.rc_attr.mjpeg_fixqp.src_frame_rate = 30;
    attr.rc_attr.mjpeg_fixqp.dst_frame_rate = 30;
    attr.rc_attr.mjpeg_fixqp.q_factor = 45;
    attr.venc_attr.type = K_PT_JPEG;
    k_s32 ret = kd_mpi_venc_create_chn(chn, &attr);
    if (ret) {
        std::cout << "kd_mpi_venc_create_chn failed, chn: " << chn << std::endl;
        return ret;
    }
    venc_snap_chn_ = chn;
    return 0;
}

int Media::VencSnapChnStart() {
    k_s32 ret = kd_mpi_venc_start_chn(venc_snap_chn_);
    if (ret) {
        std::cout << "VencSnapChnStart failed, chn: " << venc_snap_chn_ << std::endl;
        return ret;
    }

    k_mpp_chn srcModuleVi;
    k_mpp_chn dstModuleVencSnap;

    srcModuleVi.mod_id = K_ID_VI;
    srcModuleVi.dev_id = VICAP_DEV_ID_0;
    srcModuleVi.chn_id = VICAP_CHN_ID_2;
    dstModuleVencSnap.mod_id = K_ID_VENC;
    dstModuleVencSnap.dev_id = 0;
    dstModuleVencSnap.chn_id = venc_snap_chn_;

    ret = kd_mpi_sys_bind(&srcModuleVi, &dstModuleVencSnap);
    if (ret) {
        std::cout << "kd_mpi_sys_bind failed, bind venc snap to vicap chn 2" << std::endl;
        return ret;
    }
    return 0;
}

int Media::VencSnapChnStop() {
    k_mpp_chn srcModuleVi;
    k_mpp_chn dstModuleVencSnap;

    srcModuleVi.mod_id = K_ID_VI;
    srcModuleVi.dev_id = VICAP_DEV_ID_0;
    srcModuleVi.chn_id = VICAP_CHN_ID_2;
    dstModuleVencSnap.mod_id = K_ID_VENC;
    dstModuleVencSnap.dev_id = 0;
    dstModuleVencSnap.chn_id = venc_snap_chn_;
    k_s32 ret = kd_mpi_sys_unbind(&srcModuleVi, &dstModuleVencSnap);
    if (ret) {
        std::cout << "kd_mpi_sys_unbind failed, bind venc snap to vicap chn 2" << std::endl;
        return ret;
    }

    ret = kd_mpi_venc_stop_chn(venc_snap_chn_);
    if (ret) {
        std::cout << "VencSnapChnStop failed, chn: " << venc_snap_chn_ << std::endl;
        return ret;
    }

    return 0;
}

int Media::VencSnapChnDestory() {
    k_s32 ret = kd_mpi_venc_destroy_chn(venc_snap_chn_);
    if (ret) {
        std::cout << "ViVencSnapChnDestory failed, chn: " << venc_snap_chn_ << std::endl;
        return ret;
    }

    return 0;
}

int Media::VencChnCreate(int chn, int width, int height) {
    k_venc_chn_attr attr;
    memset(&attr, 0, sizeof(attr));
    attr.venc_attr.pic_width = width;
    attr.venc_attr.pic_height = height;
    k_u64 stream_size = width * height / 2;
    attr.venc_attr.stream_buf_size = VICAP_ALIGN_UP(stream_size, 0x1000);
    attr.venc_attr.stream_buf_cnt = 3;
    attr.venc_attr.type = K_PT_H265;
    attr.venc_attr.profile = VENC_PROFILE_H265_MAIN;
    attr.rc_attr.rc_mode = K_VENC_RC_MODE_CBR;
    attr.rc_attr.cbr.src_frame_rate = 30;
    attr.rc_attr.cbr.dst_frame_rate = 30;
    attr.rc_attr.cbr.gop = 30;
    attr.rc_attr.cbr.bit_rate = 2000;

    k_s32 ret = kd_mpi_venc_create_chn(chn, &attr);
    if (ret) {
        std::cout << "kd_mpi_venc_create_chn failed, chn: " << chn << std::endl;
        return ret;
    }
    venc_chn_ = chn;
    kd_mpi_venc_enable_idr(venc_chn_, K_TRUE);

    return 0;
}

int Media::VencChnStart() {
    k_s32 ret = kd_mpi_venc_start_chn(venc_chn_);
    if (ret) {
        std::cout << "VencChnStart failed, chn: " << venc_chn_ << std::endl;
        return ret;
    }

    k_mpp_chn srcModuleVi;
    k_mpp_chn dstModuleVenc;
    srcModuleVi.mod_id = K_ID_VI;
    srcModuleVi.dev_id = VICAP_DEV_ID_0;
    srcModuleVi.chn_id = VICAP_CHN_ID_1;
    dstModuleVenc.mod_id = K_ID_VENC;
    dstModuleVenc.dev_id = 0;
    dstModuleVenc.chn_id = venc_chn_;

    ret = kd_mpi_sys_bind(&srcModuleVi, &dstModuleVenc);
    if (ret) {
        std::cout << "kd_mpi_sys_bind failed, unbind venc snap to vicap chn 0" << std::endl;
        return ret;
    }

    return 0;
}

int Media::VencChnStop() {
    k_mpp_chn srcModuleVi;
    k_mpp_chn dstModuleVenc;
    srcModuleVi.mod_id = K_ID_VI;
    srcModuleVi.dev_id = VICAP_DEV_ID_0;
    srcModuleVi.chn_id = VICAP_CHN_ID_1;
    dstModuleVenc.mod_id = K_ID_VENC;
    dstModuleVenc.dev_id = 0;
    dstModuleVenc.chn_id = venc_chn_;

    k_s32 ret = kd_mpi_sys_unbind(&srcModuleVi, &dstModuleVenc);
    if (ret) {
        std::cout << "kd_mpi_sys_unbind failed, unbind venc to vicap chn 1" << std::endl;
        return ret;
    }

    ret = kd_mpi_venc_stop_chn(venc_chn_);
    if (ret) {
        std::cout << "VencChnStop failed, chn: " << venc_chn_ << std::endl;
        return ret;
    }

    return 0;
}

int Media::VencChnDestory() {
    k_s32 ret = kd_mpi_venc_destroy_chn(venc_chn_);
    if (ret) {
        std::cout << "ViVencChnDestory failed, chn: " << venc_chn_ << std::endl;
        return ret;
    }

    return 0;
}

int Media::AiAEncCreate() {
    k_audio_dev ai_dev = 0;
    k_ai_chn ai_chn = 0;
    int aenc_chn = 0;
    k_aenc_chn_attr aenc_chn_attr;
    memset(&aenc_chn_attr, 0, sizeof(aenc_chn_attr));

    aenc_chn_attr.type = K_PT_G711U;
    aenc_chn_attr.buf_size = audio_persec_div_num_;
    aenc_chn_attr.point_num_per_frame = audio_sample_rate_ / aenc_chn_attr.buf_size;
    k_s32 ret = kd_mpi_aenc_create_chn(aenc_chn, &aenc_chn_attr);
    if (ret != K_SUCCESS) {
        std::cout << "kd_mpi_aenc_create_chn faild." << std::endl;
        return -1;
    }

    k_aio_dev_attr aio_dev_attr;
    memset(&aio_dev_attr, 0, sizeof(aio_dev_attr));
    aio_dev_attr.audio_type = KD_AUDIO_INPUT_TYPE_I2S;
    aio_dev_attr.kd_audio_attr.i2s_attr.sample_rate = audio_sample_rate_;
    aio_dev_attr.kd_audio_attr.i2s_attr.bit_width = KD_AUDIO_BIT_WIDTH_16;
    aio_dev_attr.kd_audio_attr.i2s_attr.chn_cnt = 2;
    aio_dev_attr.kd_audio_attr.i2s_attr.i2s_mode = K_RIGHT_JUSTIFYING_MODE;
    aio_dev_attr.kd_audio_attr.i2s_attr.snd_mode = KD_AUDIO_SOUND_MODE_MONO;
    aio_dev_attr.kd_audio_attr.i2s_attr.frame_num = audio_persec_div_num_;
    aio_dev_attr.kd_audio_attr.i2s_attr.point_num_per_frame = audio_sample_rate_ / aio_dev_attr.kd_audio_attr.i2s_attr.frame_num;
    aio_dev_attr.kd_audio_attr.i2s_attr.i2s_type = K_AIO_I2STYPE_INNERCODEC;
    ret = kd_mpi_ai_set_pub_attr(ai_dev, &aio_dev_attr);
    if (ret != K_SUCCESS) {
        std::cout << "kd_mpi_ai_set_pub_attr failed." << std::endl;
        return -1;
    }

    return 0;
}

int Media::AiAEncStart() {
    k_audio_dev ai_dev = 0;
    k_ai_chn ai_chn = 0;
    int aenc_chn = 0;
    kd_mpi_ai_enable(ai_dev);
    kd_mpi_ai_enable_chn(ai_dev, ai_chn);

    k_mpp_chn ai_mpp_chn;
    k_mpp_chn aenc_mpp_chn;
    ai_mpp_chn.mod_id = K_ID_AI;
    ai_mpp_chn.dev_id = ai_dev;
    ai_mpp_chn.chn_id = ai_chn;
    aenc_mpp_chn.mod_id = K_ID_AENC;
    aenc_mpp_chn.dev_id = 0;
    aenc_mpp_chn.chn_id = aenc_chn;
    k_s32 ret = kd_mpi_sys_bind(&ai_mpp_chn, &aenc_mpp_chn);
    if (ret != K_SUCCESS) {
        std::cout << "kd_mpi_sys_bind failed." << std::endl;
        return -1;
    }

    return 0;
}

int Media::AEncGetStream(k_audio_stream *aenc_stream) {
    k_s32 ret = kd_mpi_aenc_get_stream(0, aenc_stream, 1000);
    if (ret != K_SUCCESS) {
        std::cout << "kd_mpi_aenc_get_stream failed." << std::endl;
        return -1;
    }

    return 0;
}

int Media::AEncReleaseStream(k_audio_stream *aenc_stream) {
    k_s32 ret = kd_mpi_aenc_release_stream(0, aenc_stream);
    if (ret != K_SUCCESS) {
        std::cout << "kd_mpi_aenc_release_stream failed." << std::endl;
        return -1;
    }

    return 0;
}

int Media::AiAencStop() {
    k_audio_dev ai_dev = 0;
    k_ai_chn ai_chn = 0;
    int aenc_chn = 0;

    k_mpp_chn ai_mpp_chn;
    k_mpp_chn aenc_mpp_chn;
    ai_mpp_chn.mod_id = K_ID_AI;
    ai_mpp_chn.dev_id = ai_dev;
    ai_mpp_chn.chn_id = ai_chn;
    aenc_mpp_chn.mod_id = K_ID_AENC;
    aenc_mpp_chn.dev_id = 0;
    aenc_mpp_chn.chn_id = aenc_chn;
    kd_mpi_sys_unbind(&ai_mpp_chn, &aenc_mpp_chn);

    kd_mpi_ai_disable_chn(ai_dev, ai_chn);
    kd_mpi_ai_disable(ai_dev);

    return 0;
}

int Media::AiAEncDestory() {
    int aenc_chn = 0;
    
    kd_mpi_aenc_destroy_chn(aenc_chn);

    return 0;
}

int Media::ADecAoCreate() {
    k_adec_chn adec_inner_chn = 0;
    k_adec_chn adec_external_chn = 1;
    k_audio_dev ao_dev = 0;
    k_adec_chn_attr adec_chn_attr;
    memset(&adec_chn_attr, 0, sizeof(adec_chn_attr));

    adec_chn_attr.type = K_PT_G711U;
    adec_chn_attr.buf_size = audio_persec_div_num_;
    adec_chn_attr.point_num_per_frame = audio_sample_rate_ / adec_chn_attr.buf_size;

    k_s32 ret = kd_mpi_adec_create_chn(adec_inner_chn, &adec_chn_attr);
    if (ret != K_SUCCESS) {
        std::cout << "kd_mpi_adec_create_chn inner faild." << std::endl;
        return -1;
    }

    ret = kd_mpi_adec_create_chn(adec_external_chn, &adec_chn_attr);
    if (ret != K_SUCCESS) {
        std::cout << "kd_mpi_adec_create_chn inner faild." << std::endl;
        return -1;
    }

    k_aio_dev_attr aio_dev_attr;
    memset(&aio_dev_attr, 0, sizeof(aio_dev_attr));
    aio_dev_attr.audio_type = KD_AUDIO_OUTPUT_TYPE_I2S;
    aio_dev_attr.kd_audio_attr.i2s_attr.sample_rate = audio_sample_rate_;
    aio_dev_attr.kd_audio_attr.i2s_attr.bit_width = KD_AUDIO_BIT_WIDTH_16;
    aio_dev_attr.kd_audio_attr.i2s_attr.chn_cnt = 2;
    aio_dev_attr.kd_audio_attr.i2s_attr.i2s_mode = K_RIGHT_JUSTIFYING_MODE;
    aio_dev_attr.kd_audio_attr.i2s_attr.snd_mode = KD_AUDIO_SOUND_MODE_MONO;
    aio_dev_attr.kd_audio_attr.i2s_attr.frame_num = audio_persec_div_num_;
    aio_dev_attr.kd_audio_attr.i2s_attr.point_num_per_frame = audio_sample_rate_ / aio_dev_attr.kd_audio_attr.i2s_attr.frame_num;
    aio_dev_attr.kd_audio_attr.i2s_attr.i2s_type = K_AIO_I2STYPE_INNERCODEC;
    ret = kd_mpi_ao_set_pub_attr(ao_dev, &aio_dev_attr);
    if (ret != K_SUCCESS) {
        std::cout << "kd_mpi_ao_set_pub_attr failed." << std::endl;
        return -1;
    }

    kd_mpi_ao_enable(ao_dev);

    return 0;
}

int Media::ADecAoInnerStart() {
    k_adec_chn adec_inner_chn = 0;
    k_audio_dev ao_dev = 0;
    k_ao_chn ao_inner_chn = 0;

    kd_mpi_ao_enable_chn(ao_dev, ao_inner_chn);
    kd_mpi_adec_clr_chn_buf(adec_inner_chn);

    k_mpp_chn ao_mpp_chn;
    k_mpp_chn adec_mpp_chn;
    memset(&ao_mpp_chn, 0, sizeof(ao_mpp_chn));
    memset(&adec_mpp_chn, 0, sizeof(adec_mpp_chn));
    adec_mpp_chn.mod_id = K_ID_ADEC;
    adec_mpp_chn.dev_id = 0;
    adec_mpp_chn.chn_id = adec_inner_chn;
    ao_mpp_chn.mod_id = K_ID_AO;
    ao_mpp_chn.dev_id = ao_dev;
    ao_mpp_chn.chn_id = ao_inner_chn;
    k_s32 ret = kd_mpi_sys_bind(&adec_mpp_chn, &ao_mpp_chn);
    if (ret != K_SUCCESS) {
        std::cout << "kd_mpi_sys_bind failed." << std::endl;
        return -1;
    }

    return 0;
}

int Media::ADecAoExternStart() {
    k_adec_chn adec_external_chn = 1;
    k_audio_dev ao_dev = 0;
    k_ao_chn ao_external_chn = 1;

    kd_mpi_ao_enable_chn(ao_dev, ao_external_chn);
    kd_mpi_adec_clr_chn_buf(adec_external_chn);

    k_mpp_chn ao_mpp_chn;
    k_mpp_chn adec_mpp_chn;
    memset(&ao_mpp_chn, 0, sizeof(ao_mpp_chn));
    memset(&adec_mpp_chn, 0, sizeof(adec_mpp_chn));
    adec_mpp_chn.mod_id = K_ID_ADEC;
    adec_mpp_chn.dev_id = 0;
    adec_mpp_chn.chn_id = adec_external_chn;
    ao_mpp_chn.mod_id = K_ID_AO;
    ao_mpp_chn.dev_id = ao_dev;
    ao_mpp_chn.chn_id = ao_external_chn;
    k_s32 ret = kd_mpi_sys_bind(&adec_mpp_chn, &ao_mpp_chn);
    if (ret != K_SUCCESS) {
        std::cout << "kd_mpi_sys_bind failed." << std::endl;
        return -1;
    }

    return 0;
}

int Media::ADecAoInnerStop() {
    k_adec_chn adec_inner_chn = 0;
    k_audio_dev ao_dev = 0;
    k_ao_chn ao_inner_chn = 0;

    k_mpp_chn ao_mpp_chn;
    k_mpp_chn adec_mpp_chn;
    memset(&ao_mpp_chn, 0, sizeof(ao_mpp_chn));
    memset(&adec_mpp_chn, 0, sizeof(adec_mpp_chn));
    adec_mpp_chn.mod_id = K_ID_ADEC;
    adec_mpp_chn.dev_id = 0;
    adec_mpp_chn.chn_id = adec_inner_chn;
    ao_mpp_chn.mod_id = K_ID_AO;
    ao_mpp_chn.dev_id = ao_dev;
    ao_mpp_chn.chn_id = ao_inner_chn;
    kd_mpi_sys_unbind(&adec_mpp_chn, &ao_mpp_chn);
    kd_mpi_ao_disable_chn(ao_dev, ao_inner_chn);

    return 0;
}

int Media::ADecAoExternStop() {
    k_adec_chn adec_external_chn = 1;
    k_audio_dev ao_dev = 0;
    k_ao_chn ao_external_chn = 1;

    k_mpp_chn ao_mpp_chn;
    k_mpp_chn adec_mpp_chn;
    memset(&ao_mpp_chn, 0, sizeof(ao_mpp_chn));
    memset(&adec_mpp_chn, 0, sizeof(adec_mpp_chn));
    adec_mpp_chn.mod_id = K_ID_ADEC;
    adec_mpp_chn.dev_id = 0;
    adec_mpp_chn.chn_id = adec_external_chn;
    ao_mpp_chn.mod_id = K_ID_AO;
    ao_mpp_chn.dev_id = ao_dev;
    ao_mpp_chn.chn_id = ao_external_chn;
    kd_mpi_sys_unbind(&adec_mpp_chn, &ao_mpp_chn);
    kd_mpi_ao_disable_chn(ao_dev, ao_external_chn);

    return 0;
}

uint8_t *Media::SysMmap(k_u64 phy_addr, k_u32 size) {
    void *vir_addr = kd_mpi_sys_mmap(phy_addr, size);

    return (uint8_t *)vir_addr;
}

void Media::SynMunmap(void *vir_addr, k_u32 size) {
    kd_mpi_sys_munmap(vir_addr, size);

    return;
}

int Media::ADecSendData(const k_u8 *data, k_u32 size, k_u64 timestamp_ms, bool inner) {
    k_adec_chn adec_inner_chn = 0;
    k_adec_chn adec_external_chn = 1;
    if (inner) {
        if (audio_stream_.audio_stream.len != size) {
            if (audio_stream_.audio_stream.len) {
                ReleaseVbBuffer(&audio_stream_.blk_handle);
                SynMunmap(audio_stream_.audio_stream.stream, audio_stream_.audio_stream.len);
                memset(&audio_stream_, 0, sizeof(audio_stream_));
            }

            GetVbBuffer(&audio_stream_.audio_stream.phys_addr, size, &audio_stream_.blk_handle);
            audio_stream_.audio_stream.stream = SysMmap(audio_stream_.audio_stream.phys_addr, size);
            audio_stream_.audio_stream.len = size;
        }

        memcpy(audio_stream_.audio_stream.stream, data, size);
        audio_stream_.audio_stream.time_stamp = timestamp_ms;
        audio_stream_.audio_stream.seq++;

        k_s32 ret = kd_mpi_adec_send_stream(adec_inner_chn, &audio_stream_.audio_stream, K_TRUE);
        if (ret != K_SUCCESS) {
            std::cout << "kd_mpi_adec_send_stream inner failed." << std::endl;
            return -1;
        }
        
    } else {
        if (audio_innercom_stream_.audio_stream.len != size) {
            if (audio_innercom_stream_.audio_stream.len) {
                ReleaseVbBuffer(&audio_innercom_stream_.blk_handle);
                SynMunmap(audio_innercom_stream_.audio_stream.stream, audio_innercom_stream_.audio_stream.len);
                memset(&audio_innercom_stream_, 0, sizeof(audio_innercom_stream_));
            }

            GetVbBuffer(&audio_innercom_stream_.audio_stream.phys_addr, size, &audio_innercom_stream_.blk_handle);
            audio_innercom_stream_.audio_stream.stream = SysMmap(audio_innercom_stream_.audio_stream.phys_addr, size);
            audio_innercom_stream_.audio_stream.len = size;
        }

        memcpy(audio_innercom_stream_.audio_stream.stream, data, size);
        audio_innercom_stream_.audio_stream.time_stamp = timestamp_ms;
        audio_innercom_stream_.audio_stream.seq++;

        k_s32 ret = kd_mpi_adec_send_stream(adec_external_chn, &audio_innercom_stream_.audio_stream, K_TRUE);
        if (ret != K_SUCCESS) {
            std::cout << "kd_mpi_adec_send_stream inner failed." << std::endl;
            return -1;
        }
    }

    return 0;
}

int Media::ADecAoDestroy() {
    k_audio_dev ao_dev = 0;
    k_adec_chn adec_inner_chn = 0;
    k_adec_chn adec_external_chn = 1;
    kd_mpi_ao_disable(ao_dev);
    kd_mpi_adec_destroy_chn(adec_inner_chn);
    kd_mpi_adec_destroy_chn(adec_external_chn);

    if (audio_stream_.audio_stream.len) {
        ReleaseVbBuffer(&audio_stream_.blk_handle);
        SynMunmap(audio_stream_.audio_stream.stream, audio_stream_.audio_stream.len);
        memset(&audio_stream_, 0, sizeof(audio_stream_));
    }

    if (audio_innercom_stream_.audio_stream.len) {
        ReleaseVbBuffer(&audio_innercom_stream_.blk_handle);
        SynMunmap(audio_innercom_stream_.audio_stream.stream, audio_innercom_stream_.audio_stream.len);
        memset(&audio_innercom_stream_, 0, sizeof(audio_innercom_stream_));
    }

    return 0;
}

void Media::GetVbBuffer(k_u64 *phy_addr, k_u64 blk_size, k_vb_blk_handle *handle) {
    *handle = kd_mpi_vb_get_block(VB_INVALID_POOLID, blk_size, NULL);
    if (*handle == VB_INVALID_HANDLE) {
        std::cout << "%s get vb block error." << std::endl;
        return;
    }

    *phy_addr = kd_mpi_vb_handle_to_phyaddr(*handle);

    return;
}

int Media::ReleaseVbBuffer(k_vb_blk_handle *handle) {
    kd_mpi_vb_release_block(*handle);

    return 0;
}

int Media::VencChnQueryStatus(int chn, VencChnStatus *chn_status) {
    k_venc_chn_status status;
    memset(&status, 0, sizeof(status));
    k_s32 ret = kd_mpi_venc_query_status(chn, &status);
    if (ret) {
        std::cout << "VencChnQueryStatus failed." << std::endl;
        return ret;
    }

    memcpy(chn_status, &status, sizeof(status));

    return 0;
}

int Media::VencChnGetStream(int chn, k_venc_stream *stream) {
    k_s32 ret = kd_mpi_venc_get_stream(chn, stream, 300);
    if (ret) {
        std::cout << "VencChnGetStream failed." << std::endl;
        return ret;
    }

    return 0;
}

int Media::VencChnReleaseStream(int chn, k_venc_stream *stream) {
    k_s32 ret = kd_mpi_venc_release_stream(chn, stream);
    if (ret) {
        std::cout << "VencChnReleaseStream failed." << std::endl;
        return ret;
    }

    return 0;
}

void Media::VoLyaerVdssBindVoConfig() {
    layer_info info;

    k_vo_layer chn_id = K_VO_LAYER1;
    memset(&info, 0, sizeof(info));

    VoConnectorInit();
    info.act_size.width = venc_width_;
    info.act_size.height = venc_height_;
    info.format = PIXEL_FORMAT_YVU_PLANAR_420;
    info.func = 0;//K_ROTATION_180;////K_ROTATION_90;
    info.global_alptha = 0xff;
    info.offset.x = 0;
    info.offset.y = 0;
    VoCreateLayer(chn_id, &info);

    return;
}

int Media::VoCreateLayer(k_vo_layer chn_id, layer_info *info) {
    k_vo_video_layer_attr attr;

    // check layer
    if ((chn_id >= K_MAX_VO_LAYER_NUM) || ((info->func & K_VO_SCALER_ENABLE) && (chn_id != K_VO_LAYER0))
            || ((info->func != 0) && (chn_id == K_VO_LAYER2))) {
        printf("input layer num failed \n");
        return -1 ;
    }

    // check scaler

    // set offset
    attr.display_rect = info->offset;
    // set act
    attr.img_size = info->act_size;
    // sget size
    info->size = info->act_size.height * info->act_size.width * 3 / 2;
    //set pixel format
    attr.pixel_format = info->format;
    if (info->format != PIXEL_FORMAT_YVU_PLANAR_420) {
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