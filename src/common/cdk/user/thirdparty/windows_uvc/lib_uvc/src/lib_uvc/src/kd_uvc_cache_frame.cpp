#include "kd_uvc_cache_frame.h"
#include<string.h>

void kd_uvc_cache_frame::cache_frame(uvc_cache_frame_type type, unsigned int frame_number, unsigned long pts, unsigned char* pdata, int len)
{
	if (type == em_uvc_cache_frame_type_yuv)
	{
		if (m_yuv_cache_cnt >= MAX_SNAP_CACHE_FRAME_SIZE)
		{
			return;
		}

		memcpy(m_cache_frame_yuv[m_yuv_cache_cnt].m_cache_pic_data, pdata, len);
		m_cache_frame_yuv[m_yuv_cache_cnt].data_size = len;
		m_cache_frame_yuv[m_yuv_cache_cnt].frame_number = frame_number;
		m_cache_frame_yuv[m_yuv_cache_cnt].pts = pts;

		m_yuv_cache_cnt++;

	}
	else if (type == em_uvc_cache_frame_type_depth)
	{
		if (m_depth_cache_cnt >= MAX_SNAP_CACHE_FRAME_SIZE)
		{
			return;
		}

		memcpy(m_cache_frame_depth[m_depth_cache_cnt].m_cache_pic_data, pdata, len);
		m_cache_frame_depth[m_depth_cache_cnt].data_size = len;
		m_cache_frame_depth[m_depth_cache_cnt].frame_number = frame_number;

		m_depth_cache_cnt++;
	}
}

int kd_uvc_cache_frame::select_match_frames(CACHE_FRAME_UV_INFO& yuv_frame_info, CACHE_FRAME_UV_INFO& depth_frame_info)
{
	if (m_depth_cache_cnt == MAX_SNAP_CACHE_FRAME_SIZE && m_yuv_cache_cnt == MAX_SNAP_CACHE_FRAME_SIZE)
	{
		int depth_frame_start_number = m_cache_frame_depth[0].frame_number;
		int yuv_frame_start_number = m_cache_frame_yuv[0].frame_number;

		if (depth_frame_start_number == yuv_frame_start_number)
		{
			yuv_frame_info = m_cache_frame_yuv[0];
			depth_frame_info = m_cache_frame_depth[0];
			return 0;
		}
		else if (depth_frame_start_number > yuv_frame_start_number)
		{
			for (int i = 1; i < MAX_SNAP_CACHE_FRAME_SIZE; i++)
			{
				if (depth_frame_start_number == m_cache_frame_yuv[i].frame_number)
				{
					yuv_frame_info = m_cache_frame_yuv[i];
					depth_frame_info = m_cache_frame_depth[0];
					return 0;
				}
			}

			m_yuv_cache_cnt = 0;
			return -1;
		}
		else
		{
			for (int i = 1; i < MAX_SNAP_CACHE_FRAME_SIZE; i++)
			{
				if (yuv_frame_start_number == m_cache_frame_depth[i].frame_number)
				{
					yuv_frame_info = m_cache_frame_yuv[0];
					depth_frame_info = m_cache_frame_depth[i];
					return 0;
				}
			}

			m_depth_cache_cnt = 0;
			return -1;
		}

	}
	return -1;
}


void kd_uvc_cache_frame::reset()
{
	m_yuv_cache_cnt = 0;
	m_depth_cache_cnt = 0;
}
