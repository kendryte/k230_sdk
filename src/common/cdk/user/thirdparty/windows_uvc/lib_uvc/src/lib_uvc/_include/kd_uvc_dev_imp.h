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
#include "calib_bin_interface.h"

enum UVC_CONTROL_STATUS
{
	em_uvc_control_init = 0,
	em_uvc_control_grabber_start = 1,
	em_uvc_control_grabber_stop  = 2,
	em_uvc_control_snap_start = 3,
	em_uvc_control_snap_stop = 4,

};

typedef struct tag_uvc_grab_frame_buffer
{
	unsigned int frame_number;
	unsigned long long pts;
	unsigned char frame_data[MAX_UVC_FRAME_SIZE];
	int data_len;
	int frame_err_cnt;
	bool grab_one_frame_done;
}uvc_grab_frame_buffer;

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
	virtual kd_uvc_camera_info      kd_uvc_get_camera_information();

	virtual bool      kd_uvc_get_all_uvc_dev_info(bool bfilter_k230, UVC_DEV_INFO_LIST& lst_dev_uvc_info);

	virtual bool      kd_uvc_transfer_file(char* serialNumber, const char* src_filepathname, const char* dst_filepathname, UVC_TRANSFER_FILE_CB uvc_transfer_cb);

	virtual void      kd_uvc_test();

protected:
	static void       _cam_callback(uvc_frame_t* frame, void* ptr);
	int               _do_uvc_frame_data(uvc_frame_t* frame, void* ptr);
	bool              _do_uvc_start_stream(bool bstart);
	int               _update_dev_cfg(const grabber_init_param& pInitParam);
	int               _transfer_cfg();
	int               _decrypt_bin_file(const grabber_init_param& pInitParam);
	int               _kd_uvc_transfer_file_data(char* serialNumber, unsigned char* pdata, int nlen, const char* dst_filepathname, UVC_TRANSFER_FILE_CB uvc_transfer_cb);
	int               _kd_uvc_transfer_cfg_data(char* serialNumber, UVC_TRANSFER_FILE_CB uvc_transfer_cb);
	int               _kd_uvc_transfer_control_cmd(char* serialNumber, UVC_TRANSFER_CONTROL_CMD cmd,bool usb_init);
private:
	grabber_init_param   m_uvc_param;
	bool                 m_brgb_mode_only;
	uvc_grab_init_parameters_ex m_uvc_grab_init_parameters;
	UVC_UTILS_DEVICE     m_uvc_device;

	uvc_grab_frame_buffer m_frame_buffer_rgb;
	uvc_grab_frame_buffer m_frame_buffer_depth;
	uvc_grab_frame_buffer m_frame_buffer_ir;
	uvc_grab_frame_buffer m_frame_buffer_speckle;

	uvc_grab_frame_info   m_grab_frame_info_rgb;
	uvc_grab_frame_info   m_grab_frame_info_depth;
	uvc_grab_frame_info   m_grab_frame_info_ir;
	uvc_grab_frame_info   m_grab_frame_info_speckle;
	uvc_grab_all_frame_info m_grab_all_frame_info;

	bool                 m_bstart_uvc_stream;
	UVC_CONTROL_STATUS   m_uvc_ctrl_status;
	std::mutex           m_mutex;
	std::condition_variable m_cond;

	kd_uvc_cache_frame   m_uvc_cache_frame;
	CACHE_FRAME_UV_INFO m_yuv_frame_info;
	CACHE_FRAME_UV_INFO m_depth_frame_info;

	calib_data_file                 m_calib_data;
	std::unique_ptr<unsigned char[]>  m_decrypt_file_data;

    char*                           m_ref_file_path_name;
	unsigned char*                  m_pref_file_data;
	int                             m_ref_file_data_len;

    char*                           m_conf_file_path_name;
	unsigned char*                  m_pconf_file_data;
	int                             m_conf_file_data_len;

	char                            m_uvc_serialNumber[64];

	kd_uvc_camera_info              m_uvc_camera_info;

};

