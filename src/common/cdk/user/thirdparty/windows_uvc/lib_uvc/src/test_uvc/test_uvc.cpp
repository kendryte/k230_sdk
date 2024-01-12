// test_uvc.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
//
#include <stdio.h>
#include <iostream>
using namespace std;
#include <windows.h>
#include <WinBase.h>
#include "lib_uvc.h"
#include "getopt.h"

#pragma  comment(lib,"lib_uvc.lib")

static const int WEB_CAMERA_VID = 0x1b3f;
static const int WEB_CAMERA_PID = 0x2247;

static const int K230_VID = 0x29f1;
static const int K230_PID = 0x0230;

static int g_save_file = 0;
static int g_work_mode = 0;

static DWORD g_yuv_data_start_time = 0;
static DWORD g_rgb_data_start_time = 0;
static FILE* g_log_fp = nullptr;

static int save_rgb_packet( uint8_t* data, int size)
{
	static int cnt = 0;
	cnt += 1;
	char sFilename[256];
	sprintf(sFilename, "./data/test%d.rgb", cnt);
	FILE* fp = fopen(sFilename, "wb");
	fwrite(data, size, 1, fp);
	fclose(fp);
	return 0;
}

static int save_yuv_packet(uint8_t* data, int size,int cnt)
{
	char sFilename[256];
	sprintf(sFilename, "./data/test%d.yuv", cnt);
	FILE* fp = fopen(sFilename, "wb");
	fwrite(data, size, 1, fp);
	fclose(fp);
	return 0;
}

static int save_depth_packet(uint8_t* data, int size,int cnt)
{
	//DWORD sbegin = GetTickCount();
	char sFilename[256];
	sprintf(sFilename, "./data/test%d.bin", cnt);
	FILE* fp = fopen(sFilename, "wb");
	fwrite(data, size, 1, fp);
	fclose(fp);
	//printf("====save_depth_packet take:%d ms\n", GetTickCount() - sbegin);
	return 0;
}

static void print_data_log(uvc_data_type data_type, unsigned int frame_number, unsigned long pts, int data_len)
{
	static char slog[256];
	if (data_type == KD_UVC_DEPTH_DATA_TYPE)
	{
		if (g_rgb_data_start_time == 0)
		{
			g_rgb_data_start_time = GetTickCount();
		}

		{
			sprintf(slog, "Grab_dep_cnt:%d,PTS:%ld,Data_len:%d\n", frame_number,pts, data_len);
			fwrite(slog, strlen(slog)+1, 1, g_log_fp);
		}


		if (frame_number % 30 == 0)
		{
			printf("Grab_dep_cnt:%d,Average FrameRate = %ld Fps,PTS:%ld,Data_len:%d\n", frame_number, frame_number *1000/ (GetTickCount()- g_rgb_data_start_time), pts, data_len);
		}
		
	}
	else if (data_type == KD_UVC_RGB_DATA_TYPE)
	{
		if (g_yuv_data_start_time == 0)
		{
			g_yuv_data_start_time = GetTickCount();
		}

		{
			sprintf(slog, "Grab_yuv_cnt:%d,PTS:%ld,Data_len:%d\n", frame_number,pts, data_len);
			fwrite(slog, strlen(slog) + 1, 1, g_log_fp);
		}

		if (frame_number % 30 == 0)
		{
			printf("Grab_yuv_cnt:%d,Average FrameRate = %ld Fps,Pts:%ld,Data_len:%d\n", frame_number, frame_number * 1000 / (GetTickCount() - g_yuv_data_start_time), pts, data_len);
		}
	}
}

static void uvc_grab_data_cb(uvc_data_type data_type, unsigned int frame_number, unsigned long pts, unsigned char* uvc_data, int data_len,void*pcontext)
{
	//printf("======uvc_data_cb type:%d,frame_number:%d\n", data_type,frame_number);
	if (data_type == KD_UVC_DEPTH_DATA_TYPE)
	{
		print_data_log(data_type, frame_number, pts, data_len);
		if (data_len != 1280 * 720 * 2)
		{
			char slog[256];
			sprintf(slog,"========error datasize,depth cnt:%d,pts:%ld,data_len:%d(%d)\n", frame_number,pts, data_len, 1280 * 720 * 2);
			fwrite(slog, strlen(slog) + 1, 1, g_log_fp);
			printf(slog);
		}
			
		if (g_save_file)
		{
			save_depth_packet(uvc_data, data_len, frame_number);
		}
		
	}
	else if (data_type == KD_UVC_RGB_DATA_TYPE)
	{
		print_data_log(data_type, frame_number, pts, data_len);
		if (data_len != 1280 * 720 * 3 / 2)
		{
			char slog[256];
			sprintf(slog,"========error datasize,yuv cnt:%d,pts:%ld,data_len:%d(%d)\n", frame_number,pts, data_len, 1280 * 720 * 3 / 2);
			fwrite(slog, strlen(slog) + 1, 1, g_log_fp);
			printf(slog);
		}
			
		if (g_save_file)
		{
			//save_rgb_packet(uvc_data, data_len);
			save_yuv_packet(uvc_data, data_len, frame_number);
		}
	}
}

static void uvc_snap_data_cb(uvc_data_type data_type, unsigned int frame_number, unsigned long pts, unsigned char* uvc_data, int data_len, void* pcontext)
{
	printf("======uvc_snap_data_cb type:%d,frame_number:%d,data_len:%d\n", data_type,frame_number, data_len);
	if (data_type == KD_UVC_DEPTH_DATA_TYPE)
	{
		if (g_save_file)
		{
			save_depth_packet(uvc_data, data_len, frame_number);
		}
	}
	else  if (data_type == KD_UVC_RGB_DATA_TYPE)
	{
		if (g_save_file)
		{
			save_yuv_packet(uvc_data, data_len, frame_number);
		}
	}
}


static int _grab_uvc_data()
{
	char sFilename[256];
	sprintf(sFilename, "./data/data.txt");
	g_log_fp = fopen(sFilename, "wb");

	kd_uvc_dev* p_uvc_dev = kd_create_uvc_dev();
	grabber_init_param init_param;
	UVC_DEV_HANDLE uvc_handle;

	init_param.uvc_data_func = uvc_grab_data_cb;

	init_param.uvc_device_vid = K230_VID;
	init_param.uvc_device_pid = K230_PID;
	init_param.format = KD_UVC_FRAME_FORMAT_NV12;
	init_param.frame_width = 640;
	init_param.frame_height = 360;

	if (!p_uvc_dev->kd_uvc_init(init_param))
	{
		return -1;
	}

	if (!p_uvc_dev->kd_uvc_start_grab())
	{
		printf("start grab failed\n");
		return -1;
	}

	Sleep(1000 * 1);//delay by 1 second, otherwise the grab data will not continue
	while (getchar() == 'q')
	{
		break;
	}

	p_uvc_dev->kd_uvc_stop_grab();
	p_uvc_dev->kd_uvc_close();

	fclose(g_log_fp);

	return 0;
}

static int _snap_uvc_data()
{
	kd_uvc_dev* p_uvc_dev = kd_create_uvc_dev();
	grabber_init_param init_param;
	UVC_DEV_HANDLE uvc_handle;

	init_param.uvc_data_func = uvc_snap_data_cb;

	init_param.uvc_device_vid = K230_VID;
	init_param.uvc_device_pid = K230_PID;
	init_param.format = KD_UVC_FRAME_FORMAT_NV12;
	init_param.frame_width = 640;
	init_param.frame_height = 360;

	if (!p_uvc_dev->kd_uvc_init(init_param))
	{
		return -1;
	}

	for (int i = 0; i < 1000; i++)
	{
		p_uvc_dev->kd_uvc_snap();
		Sleep(1000 * 1);
	}

	Sleep(1000 * 1);//delay by 1 second, otherwise the grab data will not continue
	while (getchar() == 'q')
	{
		break;
	}

	p_uvc_dev->kd_uvc_stop_grab();
	p_uvc_dev->kd_uvc_close();

	return 0;
}

static void uvc_transfer_file_callback(int cur_size, int total_size, uvc_transfer_file_status transfer_status)
{
	if (em_uvc_transfer_file_start == transfer_status)
	{
		printf("transfer start package ok\n");
	}
	else if (em_uvc_transfer_file_during == transfer_status)
	{
		printf("\rtransfer file package %d/%d Bytes", cur_size, total_size);
	}
	else if (em_uvc_transfer_file_stop == transfer_status)
	{
		printf("\rtransfer file package %d/%d Bytes\n", cur_size, total_size);
		printf("transfer end package ok\n");
	}
}


static void usage(const char* argv0)
{
	printf("Usage: %s -s 0 -m 0 -f H1280W720_ref.bin.\n", argv0);
	printf("-s: grab data to file\n");
	printf("-m: work mode. 0:grab data  1:snap data 2:tranfer file\n");
	printf("-i: transfer local filename.\n");
	printf("-o: k230 receive filename.\n");

}

extern int test_file_decrpt();
int main(int argc, char* argv[])
{
	_wmkdir(L"data");
	int opt;
	char* transfer_filename = nullptr;
	char* dst_filename = nullptr;
	
	while ((opt = getopt(argc, argv, (char*)"hs:m:i:o:")) != -1)
	{
		switch (opt)
		{
			case 's':
				g_save_file = atoi(optarg);
				break;
			case 'm':
				g_work_mode = atoi(optarg);
				break;
			case 'h':
				usage(argv[0]);
				return 0;
			case 'i':
				transfer_filename = optarg;
				break;
			case 'o':
				dst_filename = optarg;
				break;
			default:
				printf("Invalid option '-%c'\n", opt);
				usage(argv[0]);
				return 1;
		}
	}

	if (g_work_mode == 0)
	{
		printf("work mode:%d,save file %d\n", g_work_mode, g_save_file);
		_grab_uvc_data();
	}
	else if (g_work_mode == 1)
	{
		printf("work mode:%d,save file %d\n", g_work_mode, g_save_file);
		_snap_uvc_data();
	}
	else if (g_work_mode == 2)
	{
		kd_uvc_dev* p_uvc_dev = kd_create_uvc_dev();
		printf("work mode:%d,transfer local filename:%s,dst filename:%s\n", g_work_mode, transfer_filename,dst_filename);
		p_uvc_dev->kd_uvc_transfer_file(NULL, transfer_filename, dst_filename, uvc_transfer_file_callback);
	}
	
	return 0;
}
