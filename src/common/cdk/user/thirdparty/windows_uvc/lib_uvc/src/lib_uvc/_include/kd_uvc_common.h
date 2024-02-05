#pragma once
#include "lib_uvc.h"
//grab data
static const int g_uvc_rgb_data_start_code = 0xaabbccdd;
static const int g_uvc_depth_data_start_code = 0xddccbbaa;
static const int g_uvc_ir_data_start_code = 0xddbbaacc;
static const int g_uvc_speckle_data_start_code = 0xddaaccbb;

typedef struct {
	int data_start_code;
	int data_size;
	int data_type;//1:start 2:middle  3:last
	int frame_size;
	unsigned int frame_number;
	unsigned int packet_number_of_frame;
	float temperature;
	int width;
	int height;
	int reserve[3];
#ifdef __linux__
	unsigned long pts;
#elif defined(_WIN32)
	unsigned long long pts;
#endif 

}UVC_PRIVATE_DATA_HEAD_INFO;



//transfer data
typedef enum
{
	em_uvc_transfer_data_type_unknown = 0,
	em_uvc_transfer_data_type_file,  //transfer file
	em_uvc_transfer_data_type_cfg,   //transfer cfg
	em_uvc_transfer_data_type_ctl,   //transfer control
}UVC_TRANSFER_DATA_TYPE;

typedef enum
{
	em_uvc_transfer_control_sync_clock = 0, //Synchronous clock
	em_uvc_transfer_control_grab_data,      //start or stop k230 send data to pc
	em_uvc_transfer_control_set_framerate,  //set frame rate
}UVC_TRANSFER_CONTROL_TYPE;

typedef union
{
	unsigned      long long sync_clock;
	bool          start_grab;
	int           frame_rate;
	unsigned long long reverse;
}UVC_TRANSFER_CONTROL_INFO;

typedef struct
{
	UVC_TRANSFER_CONTROL_TYPE type;
	UVC_TRANSFER_CONTROL_INFO ctrl_info;
}UVC_TRANSFER_CONTROL_CMD;

//transfer file
typedef struct
{
	int data_start_code;
	UVC_TRANSFER_DATA_TYPE tranfer_type;
}UVC_TRANSFER_DATA_HEAD_INFO;

#define MAX_TRANSFER_SIZE 60
#define MAX_TRANSFER_PATH_FILE_NAME_SIZE  52 //MAX_TRANSFER_SIZE - head size
static const char* UVC_TRANSFER_START_HEAD = "$$$0";
static const char* UVC_TRANSFER_STOP_HEAD = "$$$2";
typedef struct
{
	char dst_filepathname[MAX_TRANSFER_PATH_FILE_NAME_SIZE];
}UVC_TRANSFER_FILE_INFO;

//transfer cfg
typedef struct tag_uvc_grab_init_parameters_ex
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
	int  reverse[11];
	//from file
	char serialNumber[64];
	k_dpu_temperature temperature;

	tag_uvc_grab_init_parameters_ex()
	{
		camera_fps = 30;
		depth_maximum_distance = 40000;
		camera_width = 1280;
		camera_height = 720;
	}

}uvc_grab_init_parameters_ex;

//frame max buffer size 
#define MAX_UVC_FRAME_SIZE 1920*1080*2