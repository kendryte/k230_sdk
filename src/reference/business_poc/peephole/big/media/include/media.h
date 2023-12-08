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

#ifndef _MEDIA_H_
#define _MEDIA_H_

#include <atomic>
#include <stdio.h>
#include <iostream>
#include "k_type.h"
#include "mpi_venc_api.h"
#include "mpi_vicap_api.h"
#include "mpi_vo_api.h"
#include "mapi_sys_api.h"
#include "mpi_sys_api.h"
#include "mpi_vb_api.h"
#include "mpi_ai_api.h"
#include "mpi_ao_api.h"
#include "mpi_aenc_api.h"
#include "mpi_adec_api.h"
#include "vo_test_case.h"
#include "k_connector_comm.h"
#include "mpi_connector_api.h"

struct VcapChnAttr {
    k_u16 output_width;
    k_u16 output_height;
    k_u16 crop_width;
    k_u16 crop_height;
    k_pixel_format pixel_format;
};

struct VencChnAttr {
    k_u32 pic_width;
    k_u32 pic_height;
    k_venc_rc_mode rc_mode;
    k_payload_type payload_type;
};

struct VencChnStatus {
    k_u32 cur_packs;
    k_u64 release_pic_pts;
    k_bool end_of_stream;
    k_venc_stream_info stream_info;
};

struct ModuleChn {
    k_mod_id module_id;
    k_s32 dev_id;
    k_s32 chn_id;
};

struct AudioStream {
    k_audio_stream audio_stream;
    k_vb_blk_handle blk_handle;
};



class Media {
  public:
    ~Media() { }

    int Init();
    void DeInit();

    int MediaSysAlloc(k_u64 *phy_addr, void **virt_addr, k_u32 len);
    int MediaSysFree(k_u64 phy_addr, void *virt_addr);

    int VcapSetDevAttr();
    int VcapSetChnAttr(k_vicap_chn vicap_chn, const VcapChnAttr &vcap_attr);
    int VcapInit();
    int VcapStart();
    int VcapGetDumpFrame(k_vicap_chn vicap_chn, k_video_frame_info *dump_frame, void *vap_ai_vaddr);
    int VcapReleaseDumpFrame(k_vicap_chn vicap_chn, const k_video_frame_info *dump_frame);
    int VcapStop();

    int VoInit();
    uint32_t VoGetInserFrame(k_video_frame_info *vf_info, void **pic_vaddr);
    int VoInsertFrame(k_video_frame_info *vf_info);
    int VoReleaseInserFrame(uint32_t inser_handle);
    int VoDeInit();

    int VencChnCreate(int chn, int width, int height);
    int VencChnStart();
    int VencChnStop();
    int VencChnDestory();

    int VencChnQueryStatus(int chn, VencChnStatus *chn_status);
    int VencChnGetStream(int chn, k_venc_stream *stream);
    int VencChnReleaseStream(int chn, k_venc_stream *stream);

    int VencSnapChnCreate(int chn, int width, int height);
    int VencSnapChnStart();
    int VencSnapChnStop();
    int VencSnapChnDestory();

    uint8_t *SysMmap(k_u64 phy_addr, k_u32 size);
    void SynMunmap(void *vir_addr, k_u32 size);

    int AiAEncCreate();
    int AiAEncStart();
    int AEncGetStream(k_audio_stream *aenc_stream);
    int AEncReleaseStream(k_audio_stream *aenc_stream);
    int AiAencStop();
    int AiAEncDestory();
    void GetVbBuffer(k_u64 *phy_addr, k_u64 blk_size, k_vb_blk_handle *handle);
    int ReleaseVbBuffer(k_vb_blk_handle *handle);

    int ADecAoCreate();
    int ADecAoInnerStart();
    int ADecAoExternStart();
    int ADecSendData(const k_u8 *data, k_u32 size, k_u64 timestamp_ms, bool inner);
    int ADecAoInnerStop();
    int ADecAoExternStop();
    int ADecAoDestroy();

  private:
    int VoConnectorInit();
    void VoLyaerVdssBindVoConfig();
    int VoCreateLayer(k_vo_layer chn_id, layer_info *info);

  private:
    k_vicap_sensor_info sensor_info_;
    k_vicap_sensor_type sensor_type_ {IMX335_MIPI_2LANE_RAW12_2592X1944_30FPS_LINEAR};
    k_vicap_dev vicap_dev_ {VICAP_DEV_ID_0};
    k_u64 audio_sample_rate_ {8000};
    k_u64 audio_persec_div_num_ {25};
    AudioStream audio_stream_;
    AudioStream audio_innercom_stream_;
    k_u64 venc_width_ {1088};
    k_u64 venc_height_ {1920};
    k_u32 osd_pool_id_ {0};
    k_u64 osd_width_ {1088};
    k_u64 osd_height_ {1920};
    k_u8 venc_snap_chn_ {0};
    k_u8 venc_chn_ {1};
    k_u64 detect_width_ {720};
    k_u64 detect_height_ {1280};
    k_u64 vcap_ai_paddr_ {0};
    void *vcap_ai_vaddr_ {nullptr};
};

#endif // _MEDIA_H_