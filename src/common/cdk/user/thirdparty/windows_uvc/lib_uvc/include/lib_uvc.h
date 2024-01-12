#pragma once
#ifdef LIBUVC_EXPORTS
#define _DLL_API _declspec(dllexport)
#else
#define _DLL_API _declspec(dllimport)
#endif

enum uvc_data_type
{
	KD_UVC_DEPTH_DATA_TYPE = 0,
	KD_UVC_RGB_DATA_TYPE ,
};
enum uvc_frame_format_type
{
	KD_UVC_FRAME_FORMAT_YUYV = 3,
	KD_UVC_FRAME_FORMAT_MJPEG = 7,
	KD_UVC_FRAME_FORMAT_H264 = 8,
	KD_UVC_FRAME_FORMAT_NV12 = 17,
};

typedef void* UVC_DEV_HANDLE;
typedef void (*UVC_GRAB_DATA_CB)(uvc_data_type data_type, unsigned int frame_number,unsigned long pts, unsigned char*uvc_data, int data_len,void *pcontex);
enum uvc_transfer_file_status
{
	em_uvc_transfer_file_start,   //start transfer
	em_uvc_transfer_file_during,  //transfering
	em_uvc_transfer_file_stop,    //stop transfer
};
typedef void (*UVC_TRANSFER_FILE_CB)(int cur_size,int total_size, uvc_transfer_file_status transfer_status);
typedef struct tag_GrabberInitParam
{
	int uvc_device_pid;
	int uvc_device_vid;
	char* serialNumber;
	uvc_frame_format_type format;
	int frame_width;
	int frame_height;
	int fps;
	UVC_GRAB_DATA_CB uvc_data_func;
	void* uvc_data_ptr;

	tag_GrabberInitParam()
	{
		uvc_data_func = nullptr;
		uvc_data_ptr = nullptr;
		uvc_device_pid = 0x0230;
		uvc_device_vid = 0x29f1;
		serialNumber = nullptr;
		format = KD_UVC_FRAME_FORMAT_NV12;
		frame_width = 1920;
		frame_height = 1080;
		fps = 30;
	}
}grabber_init_param;


class _DLL_API kd_uvc_dev
{
public:
	virtual           ~kd_uvc_dev() {};
	virtual bool      kd_uvc_init(const grabber_init_param& pInitParam) = 0;
	virtual bool      kd_uvc_close() = 0;
	virtual bool      kd_uvc_start_grab() = 0;
	virtual bool      kd_uvc_stop_grab() = 0;

	virtual bool      kd_uvc_snap() = 0;

	virtual bool      kd_uvc_transfer_file(char* serialNumber, const char* src_filepathname, const char* dst_filepathname, UVC_TRANSFER_FILE_CB uvc_transfer_cb) = 0;

	virtual void      kd_uvc_test() = 0;
};

_DLL_API kd_uvc_dev* kd_create_uvc_dev();
_DLL_API void  kd_destroy_uvc_dev(kd_uvc_dev* p_uvc_dev);