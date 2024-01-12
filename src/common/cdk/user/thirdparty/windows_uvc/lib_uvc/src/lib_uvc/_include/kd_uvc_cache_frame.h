#pragma once
#include "kd_uvc_common.h"
#define  MAX_SNAP_CACHE_FRAME_SIZE  2

struct CACHE_FRAME_YUV_INFO
{
	int frame_number;
	unsigned char        m_cache_pic_data[MAX_YUV_PIC_SIZE];
	int data_size;
	unsigned long pts;
};

struct CACHE_FRAME_DEPTH_INFO
{
	unsigned int frame_number;
	unsigned char        m_cache_pic_data[MAX_DEPTH_PIC_SIZE];
	int data_size;
	unsigned long pts;
};

enum uvc_cache_frame_type
{
	em_uvc_cache_frame_type_yuv = 0,
	em_uvc_cache_frame_type_depth = 1,
};
class kd_uvc_cache_frame
{
public:
	void  cache_frame(uvc_cache_frame_type type, unsigned int frame_number, unsigned long pts,unsigned char* pdata, int len);
	int   select_match_frames(CACHE_FRAME_YUV_INFO& yuv_frame_info, CACHE_FRAME_DEPTH_INFO& depth_frame_info);
	void  reset();
private:
	int                  m_yuv_cache_cnt;
	int                  m_depth_cache_cnt;
	CACHE_FRAME_DEPTH_INFO m_cache_frame_depth[MAX_SNAP_CACHE_FRAME_SIZE];
	CACHE_FRAME_YUV_INFO m_cache_frame_yuv[MAX_SNAP_CACHE_FRAME_SIZE];
};

