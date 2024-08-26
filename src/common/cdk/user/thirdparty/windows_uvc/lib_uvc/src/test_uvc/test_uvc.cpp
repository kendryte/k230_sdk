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
static int g_frame_rate = 30;
static int g_sensor_type0 = 20;
static int g_sensor_type1 = 19;
static int g_dma_ro = DEGREE_90;

static DWORD g_yuv_data_start_time = 0;
static DWORD g_rgb_data_start_time = 0;
static DWORD g_ir_data_start_time = 0;
static DWORD g_speckle_data_start_time = 0;
static FILE* g_log_fp = nullptr;

static unsigned int g_rgb_frame_number = 0;
static unsigned int g_depth_frame_number = 0;
static unsigned int g_speckle_frame_number = 0;
static unsigned int g_ir_frame_number = 0;
static unsigned long long g_frame_recv_timestamp;

static char* g_cfg_file_path_name = (char*)"./bin/0702/H1280W720.bin";
k_grab_image_mode  g_grab_image_mode = GRAB_IMAGE_MODE_RGB_DEPTH;

static int gettimeofday(struct timeval* tp, void* tzp)
{
	time_t clock;
	struct tm tm;
	SYSTEMTIME wtm;
	GetLocalTime(&wtm);
	tm.tm_year = wtm.wYear - 1900;
	tm.tm_mon = wtm.wMonth - 1;
	tm.tm_mday = wtm.wDay;
	tm.tm_hour = wtm.wHour;
	tm.tm_min = wtm.wMinute;
	tm.tm_sec = wtm.wSecond;
	tm.tm_isdst = -1;
	clock = mktime(&tm);
	tp->tv_sec = clock;
	tp->tv_usec = wtm.wMilliseconds * 1000;
	return 0;
}

static unsigned long long  get_system_time_microsecond()
{
	struct timeval timestamp;
	if (0 == gettimeofday(&timestamp, NULL))
	{
		return (unsigned long long)timestamp.tv_sec * 1000000 + timestamp.tv_usec;
	}

	return 0;
}


enum uvc_data_type
{
	KD_UVC_RGB_DATA_TYPE = 0,
	KD_UVC_DEPTH_DATA_TYPE,
	KD_UVC_IR_DATA_TYPE,
	KD_UVC_SPECKLE_DATA_TYPE,
};


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

static int save_yuv_packet(uvc_data_type type,uint8_t* data, unsigned long long pts, int size,int cnt)
{
	char sFilename[256];
	if (type == KD_UVC_IR_DATA_TYPE)
	{
		sprintf(sFilename, "./data/test%d_ir_%lld.yuv", cnt,pts);
	}
	else if (type == KD_UVC_RGB_DATA_TYPE)
	{
		sprintf(sFilename, "./data/test%d_rgb_%lld.yuv", cnt,pts);
	}
	else if (type == KD_UVC_SPECKLE_DATA_TYPE)
	{
		sprintf(sFilename, "./data/test%d_speckle_%lld.yuv", cnt,pts);
	}
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

static void print_data_log(uvc_data_type data_type, unsigned int frame_number, unsigned long long pts, int data_len,float temperature)
{
	static char slog[256];
	if (data_type == KD_UVC_DEPTH_DATA_TYPE)
	{
		if (g_rgb_data_start_time == 0)
		{
			g_rgb_data_start_time = GetTickCount();
		}

		{
			sprintf(slog, "Grab_dep_cnt:%d,PTS:%lld(%llu_%d),Data_len:%d,temperature:%f\n", frame_number,pts, g_frame_recv_timestamp, g_frame_recv_timestamp - pts, data_len, temperature);
			//sprintf(slog, "Grab_dep_cnt:%d,PTS:%llu,Data_len:%d,temperature:%f\n", frame_number, pts, data_len, temperature);
			fwrite(slog, strlen(slog)+1, 1, g_log_fp);
		}	

		if (frame_number != 0)
		{
			if (frame_number % 30 == 0)
			{
				printf("Grab_dep_cnt:%d,Average FrameRate = %.2f Fps,PTS:%lld(%llu_%d),Data_len:%d,temperature:%f\n", frame_number, frame_number * 1000.0 / (GetTickCount() - g_rgb_data_start_time), pts, g_frame_recv_timestamp, g_frame_recv_timestamp -pts, data_len, temperature);
				//printf("Grab_dep_cnt:%d,Average FrameRate = %.2f Fps,PTS:%llu,Data_len:%d,temperature:%f\n", frame_number, frame_number * 1000.0 / (GetTickCount() - g_rgb_data_start_time), pts,  data_len, temperature);
			}
		}

	}
	else if (data_type == KD_UVC_RGB_DATA_TYPE)
	{
		if (g_yuv_data_start_time == 0)
		{
			g_yuv_data_start_time = GetTickCount();
		}

		{
			sprintf(slog, "Grab_yuv_cnt:%d,PTS:%lld(%llu_%d),Data_len:%d,temperature:%f\n", frame_number,pts, g_frame_recv_timestamp, g_frame_recv_timestamp - pts, data_len, temperature);
			//sprintf(slog, "Grab_yuv_cnt:%d,PTS:%llu,Data_len:%d,temperature:%f\n", frame_number, pts,data_len, temperature);
			fwrite(slog, strlen(slog) + 1, 1, g_log_fp);
		}

		if (frame_number != 0)
		{
			if (frame_number % 30 == 0)
			{
				printf("Grab_yuv_cnt:%d,Average FrameRate = %.2f Fps,PTS:%lld(%llu_%d),Data_len:%d,temperature:%f\n", frame_number, frame_number * 1000.0 / (GetTickCount() - g_yuv_data_start_time), pts, g_frame_recv_timestamp, g_frame_recv_timestamp - pts, data_len, temperature);
				//printf("Grab_yuv_cnt:%d,Average FrameRate = %.2f Fps,PTS:%llu,Data_len:%d,temperature:%f\n", frame_number, frame_number * 1000.0 / (GetTickCount() - g_yuv_data_start_time), pts,  data_len, temperature);
			}
		}
	}
	else if (data_type == KD_UVC_IR_DATA_TYPE)
	{
		if (g_ir_data_start_time == 0)
		{
			g_ir_data_start_time = GetTickCount();
		}

		{
			sprintf(slog, "Grab_ir _cnt:%d,PTS:%lld(%llu_%d),Data_len:%d,temperature:%f\n", frame_number, pts, g_frame_recv_timestamp, g_frame_recv_timestamp - pts, data_len, temperature);
			//sprintf(slog, "Grab_ir_cnt:%d,PTS:%llu,Data_len:%d,temperature:%f\n", frame_number, pts, data_len, temperature);
			fwrite(slog, strlen(slog) + 1, 1, g_log_fp);
		}

		if (frame_number != 0)
		{
			if (frame_number % 30 == 0)
			{
				//printf("Grab_ir_cnt:%d,Average FrameRate = %.2f Fps,PTS:%llu,Data_len:%d,temperature:%f\n", frame_number, frame_number * 1000.0 / (GetTickCount() - g_ir_data_start_time), pts, data_len, temperature);
				printf("Grab_ir_cnt:%d,Average FrameRate = %.2f Fps,PTS:%lld(%llu_%d),Data_len:%d,temperature:%f\n", frame_number, frame_number * 1000.0 / (GetTickCount() - g_ir_data_start_time), pts, g_frame_recv_timestamp, g_frame_recv_timestamp - pts, data_len, temperature);
			}
		}

	}
	else if (data_type == KD_UVC_SPECKLE_DATA_TYPE)
	{
		if (g_speckle_data_start_time == 0)
		{
			g_speckle_data_start_time = GetTickCount();
		}

		{
			//sprintf(slog, "Grab_speckle_cnt:%d,PTS:%llu,Data_len:%d,temperature:%f\n", frame_number, pts, data_len, temperature);
			sprintf(slog, "Grab_speckle_cnt:%d,PTS:%lld(%llu_%d),Data_len:%d,temperature:%f\n", frame_number, pts, g_frame_recv_timestamp, g_frame_recv_timestamp - pts, data_len, temperature);

			fwrite(slog, strlen(slog) + 1, 1, g_log_fp);
		}

		if (frame_number != 0)
		{
			if (frame_number % 30 == 0)
			{
				//printf("Grab_speckle_cnt:%d,Average FrameRate = %.2f Fps,PTS:%llu,Data_len:%d,temperature:%f\n", frame_number, frame_number * 1000.0 / (GetTickCount() - g_speckle_data_start_time), pts, data_len, temperature);
				printf("Grab_speckle_cnt:%d,Average FrameRate = %.2f Fps,PTS:%lld(%llu_%d),Data_len:%d,temperature:%f\n", frame_number, frame_number * 1000.0 / (GetTickCount() - g_speckle_data_start_time), pts, g_frame_recv_timestamp, g_frame_recv_timestamp - pts, data_len, temperature);
			}
		}
	}
}

#if 1
static void uvc_grab_frame_cb(uvc_grab_all_frame_info* pframe_info)
{
	g_frame_recv_timestamp = get_system_time_microsecond();
	//depth
	if (pframe_info->frame_depth != nullptr)
	{
		
		print_data_log(KD_UVC_DEPTH_DATA_TYPE, g_depth_frame_number++/*pframe_info->frame_depth->frame_number*/, pframe_info->frame_depth->pts, pframe_info->frame_depth->data_len, pframe_info->temperature);
		if (g_save_file)
		{
			save_depth_packet(pframe_info->frame_depth->uvc_data, pframe_info->frame_depth->data_len, pframe_info->frame_depth->frame_number);
		}

		//printf("depth frame:width:%d,height:%d,tx:%.2f\n", pframe_info->frame_depth->width, pframe_info->frame_depth->height, pframe_info->temperature);
	}


	//image
	if (pframe_info->frame_rgb != nullptr)
	{
		print_data_log(KD_UVC_RGB_DATA_TYPE, g_rgb_frame_number++/*pframe_info->frame_rgb->frame_number*/, pframe_info->frame_rgb->pts, pframe_info->frame_rgb->data_len, pframe_info->temperature);

		if (g_save_file)
		{
			//save_rgb_packet(uvc_data, data_len);
			save_yuv_packet(KD_UVC_RGB_DATA_TYPE, pframe_info->frame_rgb->uvc_data, pframe_info->frame_rgb->pts, pframe_info->frame_rgb->data_len, pframe_info->frame_rgb->frame_number);
		}

		//printf("rgb frame:width:%d,height:%d,tx:%.2f\n", pframe_info->frame_rgb->width, pframe_info->frame_rgb->height, pframe_info->temperature);
	}


	//ir
	if (pframe_info->frame_ir != nullptr)
	{
		print_data_log(KD_UVC_IR_DATA_TYPE, g_ir_frame_number++/*pframe_info->frame_ir->frame_number*/, pframe_info->frame_ir->pts, pframe_info->frame_ir->data_len, pframe_info->temperature);

		if (g_save_file)
		{
			//save_rgb_packet(uvc_data, data_len);
			save_yuv_packet(KD_UVC_IR_DATA_TYPE,pframe_info->frame_ir->uvc_data, pframe_info->frame_ir->pts, pframe_info->frame_ir->data_len, pframe_info->frame_ir->frame_number);
		}
		//printf("ir frame:width:%d,height:%d,tx:%.2f\n", pframe_info->frame_ir->width, pframe_info->frame_ir->height, pframe_info->temperature);
	}

	//speckle
	if (pframe_info->frame_speckle != nullptr)
	{
		print_data_log(KD_UVC_SPECKLE_DATA_TYPE, g_speckle_frame_number++/*pframe_info->frame_speckle->frame_number*/, pframe_info->frame_speckle->pts, pframe_info->frame_speckle->data_len, pframe_info->temperature);

		if (g_save_file)
		{
			//save_rgb_packet(uvc_data, data_len);
			save_yuv_packet(KD_UVC_SPECKLE_DATA_TYPE,pframe_info->frame_speckle->uvc_data, pframe_info->frame_speckle->pts, pframe_info->frame_speckle->data_len, pframe_info->frame_speckle->frame_number);
		}
		//printf("speckle frame:width:%d,height:%d,tx:%.2f\n", pframe_info->frame_speckle->width, pframe_info->frame_speckle->height, pframe_info->temperature);
	}
}
#else
static void uvc_grab_frame_cb(uvc_grab_all_frame_info* pframe_info)
{
	static char slog[256];
	if (pframe_info->frame_rgb != nullptr && pframe_info->frame_depth != nullptr)
	{
		snprintf(slog, 256, "cnt:%-5d ,%s%-20lld,  %s%-20lld, %s%-10d\n", pframe_info->frame_rgb->frame_number, "yuv:", pframe_info->frame_rgb->pts, "depth:", pframe_info->frame_depth->pts, "diff:", pframe_info->frame_rgb->pts - pframe_info->frame_depth->pts);
		fwrite(slog, strlen(slog) + 1, 1, g_log_fp);
	}
	else if (pframe_info->frame_rgb != nullptr && pframe_info->frame_ir != nullptr)
	{
		snprintf(slog, 256, "cnt:%-5d ,%s%-20lld,  %s%-20lld, %s%-10d\n", pframe_info->frame_rgb->frame_number, "yuv:", pframe_info->frame_rgb->pts, "ir:", pframe_info->frame_ir->pts, "diff:", pframe_info->frame_rgb->pts - pframe_info->frame_ir->pts);
		fwrite(slog, strlen(slog) + 1, 1, g_log_fp);
	}
	else if (pframe_info->frame_rgb != nullptr && pframe_info->frame_speckle != nullptr)
	{
		snprintf(slog, 256, "cnt:%-5d ,%s%-20lld,  %s%-20lld, %s%-10d\n", pframe_info->frame_rgb->frame_number, "yuv:", pframe_info->frame_rgb->pts, "speckle:", pframe_info->frame_speckle->pts, "diff:", pframe_info->frame_rgb->pts - pframe_info->frame_speckle->pts);
		fwrite(slog, strlen(slog) + 1, 1, g_log_fp);
	}

}
#endif 

static void uvc_snap_frame_cb(uvc_grab_all_frame_info* pframe_info)
{
	if (pframe_info->frame_depth != nullptr)
	{
		if (g_save_file)
		{
			save_depth_packet(pframe_info->frame_depth->uvc_data, pframe_info->frame_depth->data_len, pframe_info->frame_depth->frame_number);
		}
	}

	if (pframe_info->frame_rgb != nullptr)
	{
		if (g_save_file)
		{
			save_yuv_packet(KD_UVC_RGB_DATA_TYPE,pframe_info->frame_rgb->uvc_data, pframe_info->frame_rgb->pts, pframe_info->frame_rgb->data_len, pframe_info->frame_rgb->frame_number);
		}
	}

	if (pframe_info->frame_ir != nullptr)
	{
		if (g_save_file)
		{
			save_yuv_packet(KD_UVC_IR_DATA_TYPE, pframe_info->frame_ir->uvc_data, pframe_info->frame_ir->pts, pframe_info->frame_ir->data_len, pframe_info->frame_ir->frame_number);
		}
	}

	if (pframe_info->frame_speckle != nullptr)
	{
		if (g_save_file)
		{
			save_yuv_packet(KD_UVC_SPECKLE_DATA_TYPE, pframe_info->frame_speckle->uvc_data, pframe_info->frame_speckle->pts, pframe_info->frame_speckle->data_len, pframe_info->frame_speckle->frame_number);
		}
	}
}


static int _grab_uvc_data(int frame_rate,char* serialNumber)
{
	char sFilename[256];
	sprintf(sFilename, "./data/data.txt");
	g_log_fp = fopen(sFilename, "wb");

	kd_uvc_dev* p_uvc_dev = kd_create_uvc_dev();
	grabber_init_param init_param;
	init_param.init_param.camera_fps = frame_rate;
	init_param.init_param.overwrite_file = true;
	init_param.init_param.sensor_type[0] = g_sensor_type0;
	init_param.init_param.sensor_type[1] = g_sensor_type1;
	init_param.init_param.grab_mode = g_grab_image_mode; 
	init_param.init_param.dma_ro = g_dma_ro;

	if (serialNumber != nullptr)
	{
		sprintf(init_param.init_param.serialNumber, "%s", serialNumber);//modify
	}

	sprintf(init_param.init_param.cfg_file_path_name, "%s", g_cfg_file_path_name);

	init_param.callback_param.uvc_frame_func = uvc_grab_frame_cb;

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

static int _snap_uvc_data(int frame_rate)
{
	kd_uvc_dev* p_uvc_dev = kd_create_uvc_dev();
	grabber_init_param init_param;
	init_param.init_param.camera_fps = frame_rate;
	init_param.init_param.overwrite_file = false;
	init_param.init_param.sensor_type[0] = g_sensor_type0;
	init_param.init_param.sensor_type[1] = g_sensor_type1;
	init_param.init_param.grab_mode = g_grab_image_mode;
	sprintf(init_param.init_param.serialNumber, "%s", "0702");

	sprintf(init_param.init_param.cfg_file_path_name, "%s", g_cfg_file_path_name);

	init_param.callback_param.uvc_frame_func = uvc_snap_frame_cb;

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

static void get_all_uvc_dev_info()
{

	kd_uvc_dev* p_uvc_dev = kd_create_uvc_dev();
	UVC_DEV_INFO_LIST lst_dev_uvc_info;
	p_uvc_dev->kd_uvc_get_all_uvc_dev_info(true, lst_dev_uvc_info);
	printf("======uvc dev size:%d\n", lst_dev_uvc_info.size());
	for (UVC_DEV_INFO_LIST::iterator itr = lst_dev_uvc_info.begin(); itr != lst_dev_uvc_info.end(); itr++)
	{
		printf("device pid:%d,vid:%d,serialNumber:%s\n", itr->pid, itr->vid, itr->serialNumber);
	}

	kd_destroy_uvc_dev(p_uvc_dev);
	p_uvc_dev = nullptr;

}

static void usage(const char* argv0)
{
	printf("grab stream frame Usage: %s -s 0 -m 0 -f ./bin/0702/H1280W720.bin -t 0.\n", argv0);
	printf("snap frame  Usage: %s -s 1 -m 1 -f ./bin/0702/H1280W720.bin -t 0.\n", argv0);
	printf("transfer file Usage: %s -m 2 -i ./bin/H1280W720_conf.bin -o /sharefs/H1280W720_conf.bin.\n", argv0);
	printf("-s <local filepathname>: save frame data to local file\n");
	printf("-m <work mode>: work mode. 0:grab data,  1:snap data, 2:tranfer file\n");
	printf("-i <local filepathname>: transfer local filename.\n");
	printf("-o <remote filepathname>: k230 receive filename.\n");
	printf("-f <ref/cfg filepathname>: k230 update ref/cfg file.\n");
	printf("-t <dpu image mode>: dpu image mode.\n");
	printf("-r <fps>: set fps.\n");
	printf("-a <sensor type0>:sensor type0,default is 20.\n");
	printf("-b <sensor type1>:sensor type1,default is 19.\n");
	printf("-n <init serialnumber>:init seialnumber,default is 0701.\n");
	printf("-d <gdma ro>:0: Rotate 0, 1: Rotate 90, 2: Rotate 180, 3:Rotate 270\n");
}

extern int test_file_decrpt();
int main(int argc, char* argv[])
{
	if (1 == argc)
	{
		usage(argv[0]);
		return 0;
	}

	_wmkdir(L"data");
	int opt;
	char* transfer_filename = nullptr;
	char* dst_filename = nullptr;
	char* serialNumber = (char*)"0701";
	
	while ((opt = getopt(argc, argv, (char*)"hs:m:i:o:f:t:r:a:b:n:d:")) != -1)
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
			case 'f':
				g_cfg_file_path_name = optarg;
				break;
			case 't':
				g_grab_image_mode = (k_grab_image_mode)atoi(optarg);
				break;
			case 'r':
				g_frame_rate = atoi(optarg);
				break;
			case 'a':
				g_sensor_type0 = atoi(optarg);
				break;
			case 'b':
				g_sensor_type1 = atoi(optarg);
				break;
			case 'n':
				serialNumber = optarg;
				break;
			case 'd':
				g_dma_ro = atoi(optarg);
				break;
			default:
				printf("Invalid option '-%c'\n", opt);
				usage(argv[0]);
				return 1;
		}
	}

	if (g_work_mode == 0)
	{
		printf("work mode:%d,save file %d,ref/cfg file:%s,grab image mode:%d,framerate:%d,sensor type0:%d,sensor type1:%d,serialNumber:%s\n", g_work_mode, g_save_file, g_cfg_file_path_name, g_grab_image_mode,g_frame_rate, g_sensor_type0, g_sensor_type1, serialNumber);
		_grab_uvc_data(g_frame_rate, serialNumber);
	}
	else if (g_work_mode == 1)
	{
		printf("work mode:%d,save file %d,ref/cfg file:%s,grab image mode:%d,framerate:%d,sensor type0:%d,sensor type1:%d\n", g_work_mode, g_save_file, g_cfg_file_path_name, g_grab_image_mode,g_frame_rate, g_sensor_type0, g_sensor_type1);
		_snap_uvc_data(g_frame_rate);
	}
	else if (g_work_mode == 2)
	{
		kd_uvc_dev* p_uvc_dev = kd_create_uvc_dev();
		printf("work mode:%d,transfer local filename:%s,dst filename:%s\n", g_work_mode, transfer_filename,dst_filename);
		p_uvc_dev->kd_uvc_transfer_file(nullptr, transfer_filename, dst_filename, uvc_transfer_file_callback);
	}
	else if (g_work_mode == 3)
	{
		get_all_uvc_dev_info();
	}
	
	return 0;
}
