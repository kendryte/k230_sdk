#pragma once
#include <list>
#include <functional>

#ifdef LIBUVC_EXPORTS
#define _DLL_API _declspec(dllexport)
#else
#define _DLL_API _declspec(dllimport)
#endif

typedef enum
{
	GRAB_IMAGE_MODE_RGB_DEPTH = 0,  /*sensor 0: RGB,  sensor 1: speckle*/
	GRAB_IMAGE_MODE_RGB_IR,       /*sensor 0: RGB,  sensor 1: IR*/
	GRAB_IMAGE_MODE_IR_DEPTH,     /*sensor 0: IR,   sensor 1: speckle*/
	GRAB_IMAGE_MODE_NONE_SPECKLE, /*sensor 0: none, sensor 1: speckle*/
	GRAB_IMAGE_MODE_NONE_IR,      /*sensor 0: none, sensor 1: IR*/
	GRAB_IMAGE_MODE_NONE_DEPTH,   /*sensor 0: none, sensor 1: speckle*/
	GRAB_IMAGE_MODE_RGB_SPECKLE,  /*sensor 0: RGB,  sensor 1: speckle*/
	GRAB_IMAGE_MODE_RGB_NONE,     /*sensor 0: RGB,  sensor 1: none*/
	GRAB_IMAGE_MODE_BUTT,
} k_grab_image_mode;

typedef struct
{
	float temperature_ref;
	float temperature_cx;
	float temperature_cy;
	float kxppt;
	float kyppt;
	float reserve[11];
}k_dpu_temperature;


typedef enum
{
	DEGREE_0,       /**< Rotate 0 degrees */
	DEGREE_90,      /**< Rotate 90 degrees */
	DEGREE_180,     /**< Rotate 180 degrees */
	DEGREE_270,     /**< Rotate 270 degrees */
} k_gdma_rotation_e;

typedef struct tag_uvc_grab_frame_info
{
	unsigned int frame_number;
	unsigned long long pts; 
	unsigned char* uvc_data;
	int data_len;
	int width;
	int height;
}uvc_grab_frame_info;

typedef struct tag_uvc_grab_all_frame_info
{
	char* serialNumber;
	float temperature;
	uvc_grab_frame_info* frame_rgb;
	uvc_grab_frame_info* frame_depth;
	uvc_grab_frame_info* frame_ir;
	uvc_grab_frame_info* frame_speckle;
	void* pcontex;
}uvc_grab_all_frame_info;

enum uvc_transfer_file_status
{
	em_uvc_transfer_file_start,   //start transfer
	em_uvc_transfer_file_during,  //transfering
	em_uvc_transfer_file_stop,    //stop transfer
};
typedef void (*UVC_TRANSFER_FILE_CB)(int cur_size,int total_size, uvc_transfer_file_status transfer_status);

typedef struct tag_uvc_grab_init_parameters
{
	//default 
	int camera_fps;
	int depth_maximum_distance;
	int camera_width;
	int camera_height;

	k_grab_image_mode grab_mode;
	int sensor_type[2];
	bool adc_enable;
	bool overwrite_file;
	char serialNumber[64];
	char cfg_file_path_name[256];
	int dma_ro;

	tag_uvc_grab_init_parameters()
	{
		camera_fps = 30;
		depth_maximum_distance = 40000;
		camera_width = 1280;
		camera_height = 720;

		memset(cfg_file_path_name, 0, sizeof(cfg_file_path_name));
		grab_mode = GRAB_IMAGE_MODE_RGB_DEPTH;
		adc_enable = true;
		overwrite_file = false;

		sensor_type[0] = 20;
		sensor_type[1] = 19;
		dma_ro = DEGREE_90;
	}

}uvc_grab_init_parameters;

typedef struct tag_uvc_grab_callback_param
{
	std::function<void(uvc_grab_all_frame_info* pframe_info)> uvc_frame_func;
	void* uvc_data_ptr;
	tag_uvc_grab_callback_param()
	{
		uvc_frame_func = nullptr;
		uvc_data_ptr = nullptr;
	}
}uvc_grab_callback_param;

typedef struct tag_GrabberInitParam
{
	uvc_grab_init_parameters init_param;
	uvc_grab_callback_param  callback_param;
}grabber_init_param;

typedef struct tag_uvc_dev_info
{
	int vid;
	int pid;
	char serialNumber[64];
}kd_uvc_dev_info;
typedef std::list<kd_uvc_dev_info>  UVC_DEV_INFO_LIST;

typedef struct tag_camera_info {
	//sensor resolution
	unsigned short  ir_width;
	unsigned short  ir_height;
	unsigned short  rgb_width;
	unsigned short  rgb_height;
	//intrinsic parameters
	float ir_fx;
	float ir_fy;
	float ir_cx;
	float ir_cy;
	float rgb_fx;
	float rgb_fy;
	float rgb_cx;
	float rgb_cy;
	//depth accuracy unit : mm
	float depth_precision;
}kd_uvc_camera_info;

class _DLL_API kd_uvc_dev
{
public:
	virtual           ~kd_uvc_dev() {};
	virtual bool      kd_uvc_init(const grabber_init_param& pInitParam) = 0;
	virtual bool      kd_uvc_close() = 0;
	virtual bool      kd_uvc_start_grab() = 0;
	virtual bool      kd_uvc_stop_grab() = 0;
	virtual bool      kd_uvc_snap() = 0;
	virtual kd_uvc_camera_info      kd_uvc_get_camera_information() = 0;


	virtual bool      kd_uvc_get_all_uvc_dev_info(bool bfilter_k230, UVC_DEV_INFO_LIST&lst_dev_uvc_info) = 0;
	virtual bool      kd_uvc_transfer_file(char* serialNumber, const char* src_filepathname, const char* dst_filepathname, UVC_TRANSFER_FILE_CB uvc_transfer_cb) = 0;
	virtual void      kd_uvc_test() = 0;
};

_DLL_API kd_uvc_dev* kd_create_uvc_dev();
_DLL_API void  kd_destroy_uvc_dev(kd_uvc_dev* p_uvc_dev);