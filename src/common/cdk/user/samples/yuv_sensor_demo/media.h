#ifndef _KD_MEDIA_H
#define _KD_MEDIA_H

#include <unistd.h>
#include <iostream>
#include <memory>
#include <mutex>
#include "mapi_sys_api.h"
#include "mapi_venc_api.h"
#include "mapi_vicap_api.h"
#include "k_vicap_comm.h"

#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <sys/mman.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include "mapi_sys_api.h"
#include "mapi_vicap_api.h"
#include "mapi_vo_api.h"
#include "k_vicap_comm.h"
#include "k_video_comm.h"
#include "k_vo_comm.h"
#include "k_connector_comm.h"
#include "k_nonai_2d_comm.h"
#include "mapi_nonai_2d_api.h"


enum class KdMediaVideoType {
    kVideoTypeH264,
    kVideoTypeH265,
    kVideoTypeMjpeg,
    kVideoTypeButt
};

struct KdMediaInputConfig {
    bool video_valid = false;
    k_vicap_sensor_type sensor_type = IMX335_MIPI_2LANE_RAW12_1920X1080_30FPS_LINEAR;
    int sensor_num = 1;
    KdMediaVideoType video_type = KdMediaVideoType::kVideoTypeH265;
    int venc_width = 1280;
    int venc_height = 720;
    int bitrate_kbps = 2000;
    int session_num = 1;   //   3
    k_connector_type vo = LT9611_MIPI_4LAN_1920X1080_60FPS;
};

class IOnVEncData {
  public:
    virtual ~IOnVEncData() {}
    virtual void OnVEncData(k_u32 chn_id, void *data, size_t size, uint64_t timestamp) = 0;
};

// TODO,
//   At the moment, KdMedia is for bidirection-speech only.
//
class KdMedia {
  public:
    KdMedia() {}
    int Init(const KdMediaInputConfig &config);
    int Deinit();


    int InitVcap(uint32_t chn_id, k_vicap_sensor_type type);
    int InitVenc(uint32_t chn_id,IOnVEncData *on_venc_data);

    int DeinitVcap(uint32_t chn_id);
    int DeinitVenc(uint32_t chn_id);

    int StartVcap(k_vicap_dev vicap_dev);
    int StopVcap(k_vicap_dev vicap_dev);

    int StartVenc(uint32_t chn_id);
    int StopVenc(uint32_t chn_id);

    int InitVO(k_connector_type type);
    int DeinitVO();

    static k_s32 VideoEncCallback(k_u32 chn_num, kd_venc_data_s* stream_data, k_u8* p_private_data);

  private:  // emphasize the following members are private
    KdMedia( const KdMedia& );
    const KdMedia& operator=( const KdMedia& );

  private:
    //
    KdMediaInputConfig config_;
    k_vicap_dev_set_info vcap_dev_info_;
    IOnVEncData *on_venc_data_ {nullptr};
};

#endif // _KD_MEDIA_H
