#pragma once

//tranfer frame data
static const int g_uvc_rgb_data_start_code = 0xaabbccdd;
static const int g_uvc_depth_data_start_code = 0xddccbbaa;

typedef struct {
	int data_start_code;
	int data_size;
	int data_type;//1:start 2:middle  3:last
	int frame_size;
	unsigned int frame_number;
	unsigned int packet_number_of_frame;
	int reserve[2];
#ifdef __linux__
	unsigned long pts;
#elif defined(_WIN32)
	unsigned long long pts;
#endif 

}UVC_PRIVATE_DATA_HEAD_INFO;




//transfer file
#define MAX_TRANSFER_SIZE 60
#define MAX_TRANSFER_PATH_FILE_NAME_SIZE  56 //MAX_TRANSFER_SIZE - head size
static const char* UVC_TRANSFER_START_HEAD = "$$$0";
//static const char* UVC_TRANSFER_DURING_HEAD = "$$$1";
static const char* UVC_TRANSFER_STOP_HEAD = "$$$2";
typedef struct
{
	char dst_filepathname[MAX_TRANSFER_PATH_FILE_NAME_SIZE];
}UVC_TRANSFER_INFO;


#define MAX_YUV_PIC_SIZE 1920*1080*3/2
#define MAX_DEPTH_PIC_SIZE 1920*1080*2
