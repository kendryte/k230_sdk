#pragma once
#include "lib_uvc.h"
extern "C"
{
	#include "kd_uvc.h"
}
#include "kd_uvc_common.h"
#include <thread>
#include <mutex>
#include <condition_variable>
#include "kd_uvc_cache_frame.h"

enum UVC_CONTROL_STATUS
{
	em_uvc_control_init = 0,
	em_uvc_control_grabber_start = 1,
	em_uvc_control_grabber_stop  = 2,
	em_uvc_control_snap_start = 3,
	em_uvc_control_snap_stop = 4,

};

class kd_uvc_dev_imp:public kd_uvc_dev
{
public:
	~ kd_uvc_dev_imp();
	kd_uvc_dev_imp();
	virtual bool      kd_uvc_init(const grabber_init_param& pInitParam);
	virtual bool      kd_uvc_close();
	virtual bool      kd_uvc_start_grab();
	virtual bool      kd_uvc_stop_grab();

	virtual bool      kd_uvc_snap();

	virtual bool      kd_uvc_transfer_file(char* serialNumber, const char* src_filepathname, const char* dst_filepathname, UVC_TRANSFER_FILE_CB uvc_transfer_cb);

	virtual void      kd_uvc_test();

protected:
	static void       _cam_callback(uvc_frame_t* frame, void* ptr);
	int               _do_uvc_frame_data(uvc_frame_t* frame, void* ptr);
	bool              _do_uvc_start_stream(bool bstart);
	int               _do_snap_match_frame();

private:
	grabber_init_param   m_uvc_param;
	UVC_UTILS_DEVICE     m_uvc_device;


	unsigned char        m_yuv_pic_data[MAX_YUV_PIC_SIZE];
	int                  m_yuv_cur_index ;
	int                  m_yuv_err_cnt;

	unsigned char        m_depth_pic_data[MAX_DEPTH_PIC_SIZE];
	int                  m_depth_cur_index;
	int                  m_depth_err_cnt;


	bool                 m_bstart_uvc_stream;
	UVC_CONTROL_STATUS   m_uvc_ctrl_status;
	std::mutex           m_mutex;
	std::condition_variable m_cond;

	kd_uvc_cache_frame   m_uvc_cache_frame;
	CACHE_FRAME_YUV_INFO m_yuv_frame_info;
	CACHE_FRAME_DEPTH_INFO m_depth_frame_info;
};

