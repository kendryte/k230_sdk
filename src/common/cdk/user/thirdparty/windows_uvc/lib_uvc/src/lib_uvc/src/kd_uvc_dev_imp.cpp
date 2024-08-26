#include "kd_uvc_dev_imp.h"
#include <iostream>
#include<thread>
using namespace std;
#include <windows.h>
#include <stdio.h>
#include "libuvc/libuvc.h"

extern "C"
{
#include "kd_uvc.h"
}

#include "libusb.h"
#include "kd_uvc_dev_imp.h"

#pragma  comment(lib,"libusb-1.0.lib")
#pragma  comment(lib,"pthread.lib")

#define  K230_UVC_DEVICE_VID 0x29f1
#define  K230_UVC_DEVICE_PID 0x0230

#define  SYNC_K230_TIMESTAMP_DELAY_TIME  50*1000 //50ms

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


kd_uvc_dev_imp::~kd_uvc_dev_imp()
{
	
}

kd_uvc_dev_imp::kd_uvc_dev_imp():m_ref_file_path_name((char*)"/sharefs/H1280W720_ref.bin"), m_conf_file_path_name((char*)"/sharefs/H1280W720_conf.bin")
{
	m_brgb_mode_only = false;
	m_bstart_uvc_stream = false;
	memset(&m_grab_frame_info_rgb, 0, sizeof(m_grab_frame_info_rgb));
	memset(&m_grab_frame_info_depth, 0, sizeof(m_grab_frame_info_depth));
	memset(&m_grab_frame_info_ir, 0, sizeof(m_grab_frame_info_ir));
	memset(&m_grab_frame_info_speckle, 0, sizeof(m_grab_frame_info_speckle));
}

bool kd_uvc_dev_imp::kd_uvc_init(const grabber_init_param& pInitParam)
{
	int ret;

	if (0 != _update_dev_cfg(pInitParam))
	{
		printf("_update_dev_cfg failed\n");
		return false;
	}

	m_uvc_param = pInitParam;
	if (m_uvc_param.init_param.grab_mode == GRAB_IMAGE_MODE_RGB_NONE)
	{
		//m_uvc_param.init_param.grab_mode = GRAB_IMAGE_MODE_RGB_DEPTH;
		m_brgb_mode_only = true;
	}
	else
	{
		m_brgb_mode_only = false;
	}

	m_uvc_device.format = UVC_FRAME_FORMAT_NV12;//UVC_FRAME_FORMAT_NV12
	m_uvc_device.frameWidth = 640;
	m_uvc_device.frameHeight = 360;
	m_uvc_device.targetFps = 30;
	m_uvc_device.pid = K230_UVC_DEVICE_PID;
	m_uvc_device.vid = K230_UVC_DEVICE_VID;
	m_uvc_device.serialNumber = m_uvc_grab_init_parameters.serialNumber;
	m_uvc_device.cb = _cam_callback;
	m_uvc_device.user_ptr = this;

	ret = kd_init_uvc_camera_device(&m_uvc_device);
	if (ret == 0)
	{
		m_uvc_ctrl_status = em_uvc_control_init;
		return true;
	}

	return false;
}
bool kd_uvc_dev_imp::kd_uvc_close()
{
	int ret;
	ret = kd_stop_uvc_camera_device(&m_uvc_device);

	return 0 == ret;
}

bool kd_uvc_dev_imp::kd_uvc_start_grab()
{
	if (m_uvc_ctrl_status == em_uvc_control_grabber_start)
	{
		printf("please stop grab first\n");
		return false;
	}

	if (m_uvc_ctrl_status == em_uvc_control_snap_start)
	{
		printf("please stop snap first\n");
		return false;
	}

	if (!m_bstart_uvc_stream)
	{
		_do_uvc_start_stream(true);
		m_bstart_uvc_stream = true;
	}
	
	m_uvc_ctrl_status = em_uvc_control_grabber_start;
	return true;
}

bool kd_uvc_dev_imp::kd_uvc_stop_grab()
{
	if (m_uvc_ctrl_status != em_uvc_control_grabber_start)
	{
		printf("please start grab first\n");
		return false;
	}
	m_uvc_ctrl_status = em_uvc_control_grabber_stop;
	return true;
}

bool kd_uvc_dev_imp::kd_uvc_snap()
{
	if (m_uvc_ctrl_status == em_uvc_control_grabber_start)
	{
		printf("please stop grab first\n");
		return false;
	}
	else if (m_uvc_ctrl_status == em_uvc_control_snap_start)
	{
		printf("already snap\n");
		return false;
	}

	if (!m_bstart_uvc_stream)
	{
		_do_uvc_start_stream(true);
		m_bstart_uvc_stream = true;
	}

	
	unique_lock<mutex> lck(m_mutex);
	m_uvc_ctrl_status = em_uvc_control_snap_start;
	m_cond.wait(lck);

	m_uvc_ctrl_status = em_uvc_control_snap_stop;

	return true;
}

kd_uvc_camera_info kd_uvc_dev_imp::kd_uvc_get_camera_information()
{
	return m_uvc_camera_info;
}

bool kd_uvc_dev_imp::kd_uvc_get_all_uvc_dev_info(bool bfilter_k230, UVC_DEV_INFO_LIST& lst_dev_uvc_info)
{
	lst_dev_uvc_info.clear();
	int ret;
	uvc_context_t* ctx;
	uvc_device_t** list;
	uvc_device_t* test_dev = 0;
	bool bfind = false;
	int dev_idx;
	ret= uvc_init(&ctx, NULL);
	if (ret < 0)
	{
		printf("uvc_init failed!\n");
		return false;
	}
	ret = uvc_get_device_list(ctx, &list);

	if (ret != UVC_SUCCESS) {
		return ret;
	}

	dev_idx = 0;

	while ((test_dev = list[dev_idx++]) != NULL) {
		uvc_device_descriptor_t* desc;

		if (uvc_get_device_descriptor(test_dev, &desc) != UVC_SUCCESS)
			continue;

		if (bfilter_k230)
		{
			if (desc->idVendor != K230_UVC_DEVICE_VID || desc->idProduct != K230_UVC_DEVICE_PID)
			{
				continue;
			}
		}

		kd_uvc_dev_info dev_info;
		memset(&dev_info, 0, sizeof(dev_info));
		dev_info.pid = desc->idProduct;
		dev_info.vid = desc->idVendor;
		if (desc->serialNumber != nullptr)
		{
			strncpy(dev_info.serialNumber, desc->serialNumber, sizeof(dev_info.serialNumber));
		}
		
		lst_dev_uvc_info.push_back(dev_info);
		bfind = true;
		uvc_free_device_descriptor(desc);
	}

	uvc_free_device_list(list, 1);

	uvc_exit(ctx);
	return bfind;
}

bool kd_uvc_dev_imp::kd_uvc_transfer_file(char* serialNumber, const char* src_filepathname, const char* dst_filepathname, UVC_TRANSFER_FILE_CB uvc_transfer_cb)
{
	FILE* pFile;
	int file_size;
	int ret = 0;
	unsigned char data_buf[MAX_TRANSFER_SIZE] = { 0 };
	unsigned char read_file_buf[MAX_TRANSFER_SIZE] = { 0 };
	int read_len;
	UVC_TRANSFER_FILE_INFO uvc_transfer_file_info;
	memset(&uvc_transfer_file_info, 0, sizeof(UVC_TRANSFER_FILE_INFO));

	//check filename size
	int src_len = strlen(src_filepathname);
	if (src_len > MAX_TRANSFER_PATH_FILE_NAME_SIZE)
	{
		printf("The file name is too long(%d), with a maximum length of %d bytes", src_len, MAX_TRANSFER_PATH_FILE_NAME_SIZE);
		return false;
	}

	//open src file
	pFile = fopen(src_filepathname, "rb");
	if (pFile == nullptr)
	{
		printf("open file:%s failed\n", src_filepathname);
		return false;
	}

	//init uvc transfer with pid vid
	if (0 != kd_uvc_transfer_init(K230_UVC_DEVICE_VID, K230_UVC_DEVICE_PID, serialNumber,true))
	{
		fclose(pFile);
		return false;
	}

	//get file size
	fseek(pFile, 0, SEEK_END);
	file_size = ftell(pFile);
	printf("file:%s size:%d Bytes\n", src_filepathname, file_size);
	rewind(pFile);

	//send transfer start package
	memcpy(uvc_transfer_file_info.dst_filepathname, dst_filepathname, strlen(dst_filepathname));
	UVC_TRANSFER_DATA_HEAD_INFO transfer_head;
	
	memcpy(&transfer_head.data_start_code, UVC_TRANSFER_START_HEAD, strlen(UVC_TRANSFER_START_HEAD));
	transfer_head.tranfer_type = em_uvc_transfer_data_type_file;

	memcpy(data_buf, &transfer_head, sizeof(transfer_head));
	memcpy(data_buf + sizeof(transfer_head), &uvc_transfer_file_info, sizeof(uvc_transfer_file_info));
	ret = kd_uvc_transfer_data(data_buf, MAX_TRANSFER_PATH_FILE_NAME_SIZE + strlen(UVC_TRANSFER_START_HEAD));
	if (ret != 0)
	{
		printf("send transfer start package failed\n");
		return false;
	}
	if (uvc_transfer_cb != NULL)
	{
		uvc_transfer_cb(0, 0, em_uvc_transfer_file_start);
	}

	//send file data package
	int cnt = 0;
	int cur_transfer_size = 0;
	while (!feof(pFile))
	{
		read_len = fread(read_file_buf, 1, MAX_TRANSFER_SIZE, pFile);
		if (read_len > 0)
		{
			memcpy(data_buf, read_file_buf, read_len);

			ret = kd_uvc_transfer_data(data_buf, read_len);
			if (ret != 0)
			{
				printf("==kd_uvc_transfer_file failed\n");
			}
			cnt++;
			cur_transfer_size += read_len;
			if (cnt % 1000 == 0)
			{
				if (uvc_transfer_cb != NULL)
					uvc_transfer_cb(cur_transfer_size, file_size, em_uvc_transfer_file_during);
			}
		}
	}

	//send transfer end package
	memset(data_buf, 0, sizeof(data_buf));
	memcpy(&transfer_head.data_start_code, UVC_TRANSFER_STOP_HEAD, strlen(UVC_TRANSFER_STOP_HEAD));
	transfer_head.tranfer_type = em_uvc_transfer_data_type_file;
	memcpy(data_buf, &transfer_head, sizeof(transfer_head));
	ret = kd_uvc_transfer_data(data_buf, sizeof(transfer_head));
	if (ret != 0)
	{
		printf("==kd_uvc_transfer_file failed\n");
	}

	if (uvc_transfer_cb != NULL)
		uvc_transfer_cb(cur_transfer_size, file_size, em_uvc_transfer_file_stop);

	//deinit uvc
	kd_uvc_transfer_deinit(true);

	return true;
}


void kd_uvc_dev_imp::kd_uvc_test()
{
	//_transfer_cfg();

	printf("set framerate ...\n");
	UVC_TRANSFER_CONTROL_CMD cmd;
	cmd.type = em_uvc_transfer_control_set_framerate;
	cmd.ctrl_info.frame_rate = 2;
	_kd_uvc_transfer_control_cmd(m_uvc_serialNumber, cmd, true);

	return;
}

void kd_uvc_dev_imp::_cam_callback(uvc_frame_t* frame, void* ptr)
{
	kd_uvc_dev_imp* pthis = (kd_uvc_dev_imp*)ptr;
	unique_lock<mutex> lck(pthis->m_mutex);

	if (pthis->m_uvc_ctrl_status == em_uvc_control_grabber_start || pthis->m_uvc_ctrl_status == em_uvc_control_snap_start)
	{
		pthis->_do_uvc_frame_data(frame, ptr);
	}
}

int kd_uvc_dev_imp::_do_uvc_frame_data(uvc_frame_t* frame, void* ptr)
{
	UVC_PRIVATE_DATA_HEAD_INFO* uvc_data_head = (UVC_PRIVATE_DATA_HEAD_INFO*)frame->data;
	unsigned char* frame_data = (unsigned char*)frame->data + sizeof(UVC_PRIVATE_DATA_HEAD_INFO);

	//printf("=======frame_data,type:%d,frame_number:%d(%d)\n", uvc_data_head->data_start_code == g_uvc_rgb_data_start_code,uvc_data_head->frame_number, uvc_data_head->packet_number_of_frame);
	//img
	if (uvc_data_head->data_start_code == g_uvc_rgb_data_start_code)
	{
		if (1 == uvc_data_head->data_type)
		{
			m_frame_buffer_rgb.data_len = 0;
			if (m_frame_buffer_rgb.data_len + uvc_data_head->data_size > uvc_data_head->frame_size)
			{
				printf("====[%d]error rgb frame size:data_type:%d,cur_index:%d,data_size:%d,frame_size:%d\n", m_frame_buffer_rgb.frame_err_cnt++, uvc_data_head->data_type, m_frame_buffer_rgb.data_len, uvc_data_head->data_size, uvc_data_head->frame_size);
			}
			else
			{
				memcpy(m_frame_buffer_rgb.frame_data + m_frame_buffer_rgb.data_len, frame_data, uvc_data_head->data_size);
				m_frame_buffer_rgb.data_len += uvc_data_head->data_size;
			}

		}
		else if (2 == uvc_data_head->data_type)
		{
			if (m_frame_buffer_rgb.data_len + uvc_data_head->data_size > uvc_data_head->frame_size)
			{
				printf("====[%d]error rgb frame size:data_type:%d,cur_index:%d,data_size:%d,frame_size:%d\n", m_frame_buffer_rgb.frame_err_cnt++, uvc_data_head->data_type, m_frame_buffer_rgb.data_len, uvc_data_head->data_size, uvc_data_head->frame_size);
			}
			else
			{
				memcpy(m_frame_buffer_rgb.frame_data + m_frame_buffer_rgb.data_len, frame_data, uvc_data_head->data_size);
				m_frame_buffer_rgb.data_len += uvc_data_head->data_size;
			}
		}
		else if (3 == uvc_data_head->data_type)
		{
			if (m_frame_buffer_rgb.data_len + uvc_data_head->data_size > uvc_data_head->frame_size)
			{
				printf("====[%d]error rgb frame size:data_type:%d,cur_index:%d,data_size:%d,frame_size:%d\n", m_frame_buffer_rgb.frame_err_cnt++, uvc_data_head->data_type, uvc_data_head->data_size, uvc_data_head->data_size, uvc_data_head->frame_size);
			}
			else if (m_frame_buffer_rgb.data_len + uvc_data_head->data_size == uvc_data_head->frame_size)
			{
				memcpy(m_frame_buffer_rgb.frame_data + m_frame_buffer_rgb.data_len, frame_data, uvc_data_head->data_size);
				m_frame_buffer_rgb.data_len += uvc_data_head->data_size;
				m_frame_buffer_rgb.pts = uvc_data_head->pts;
				m_frame_buffer_rgb.frame_number = uvc_data_head->frame_number;
				if (m_uvc_param.callback_param.uvc_frame_func != nullptr)
				{
					if (m_uvc_ctrl_status == em_uvc_control_snap_start)
					{
						if (GRAB_IMAGE_MODE_RGB_DEPTH == m_uvc_param.init_param.grab_mode)
						{
							if (m_frame_buffer_depth.grab_one_frame_done)
							{
								m_grab_frame_info_rgb.frame_number = m_frame_buffer_rgb.frame_number;
								m_grab_frame_info_rgb.pts = m_frame_buffer_rgb.pts;
								m_grab_frame_info_rgb.data_len = m_frame_buffer_rgb.data_len;
								m_grab_frame_info_rgb.uvc_data = m_frame_buffer_rgb.frame_data;
								m_grab_frame_info_rgb.width = uvc_data_head->width;
								m_grab_frame_info_rgb.height = uvc_data_head->height;

								m_grab_frame_info_depth.frame_number = m_frame_buffer_depth.frame_number;
								m_grab_frame_info_depth.pts = m_frame_buffer_depth.pts;
								m_grab_frame_info_depth.data_len = m_frame_buffer_depth.data_len;
								m_grab_frame_info_depth.uvc_data = m_frame_buffer_depth.frame_data;
								m_grab_frame_info_depth.width = uvc_data_head->width;
								m_grab_frame_info_depth.height = uvc_data_head->height;
								

								m_grab_all_frame_info.frame_depth = m_brgb_mode_only ?nullptr:&m_grab_frame_info_depth;
								m_grab_all_frame_info.frame_rgb = &m_grab_frame_info_rgb;
								m_grab_all_frame_info.frame_speckle = nullptr;
								m_grab_all_frame_info.frame_ir = nullptr;
								m_grab_all_frame_info.serialNumber = m_uvc_grab_init_parameters.serialNumber;
								m_grab_all_frame_info.pcontex = m_uvc_param.callback_param.uvc_data_ptr;
								m_grab_all_frame_info.temperature = uvc_data_head->temperature;

								m_uvc_param.callback_param.uvc_frame_func(&m_grab_all_frame_info);

								m_frame_buffer_depth.grab_one_frame_done = false;
								m_frame_buffer_rgb.data_len = 0;
								m_frame_buffer_depth.data_len = 0;

								m_cond.notify_all();
								m_uvc_ctrl_status == em_uvc_control_snap_stop;
							}
						}
						else if (GRAB_IMAGE_MODE_RGB_IR == m_uvc_param.init_param.grab_mode)
						{
							if (m_frame_buffer_ir.grab_one_frame_done)
							{
								m_grab_frame_info_rgb.frame_number = m_frame_buffer_rgb.frame_number;
								m_grab_frame_info_rgb.pts = m_frame_buffer_rgb.pts;
								m_grab_frame_info_rgb.data_len = m_frame_buffer_rgb.data_len;
								m_grab_frame_info_rgb.uvc_data = m_frame_buffer_rgb.frame_data;
								m_grab_frame_info_rgb.width = uvc_data_head->width;
								m_grab_frame_info_rgb.height = uvc_data_head->height;

								m_grab_frame_info_ir.frame_number = m_frame_buffer_ir.frame_number;
								m_grab_frame_info_ir.pts = m_frame_buffer_ir.pts;
								m_grab_frame_info_ir.data_len = m_frame_buffer_ir.data_len;
								m_grab_frame_info_ir.uvc_data = m_frame_buffer_ir.frame_data;
								m_grab_frame_info_ir.width = uvc_data_head->width;
								m_grab_frame_info_ir.height = uvc_data_head->height;

								m_grab_all_frame_info.frame_depth = nullptr;
								m_grab_all_frame_info.frame_rgb = &m_grab_frame_info_rgb;
								m_grab_all_frame_info.frame_speckle = nullptr;
								m_grab_all_frame_info.frame_ir = &m_grab_frame_info_ir;
								m_grab_all_frame_info.serialNumber = m_uvc_grab_init_parameters.serialNumber;
								m_grab_all_frame_info.pcontex = m_uvc_param.callback_param.uvc_data_ptr;
								m_grab_all_frame_info.temperature = uvc_data_head->temperature;

								m_uvc_param.callback_param.uvc_frame_func(&m_grab_all_frame_info);

								m_frame_buffer_ir.grab_one_frame_done = false;
								m_frame_buffer_rgb.data_len = 0;
								m_frame_buffer_ir.data_len = 0;

								m_cond.notify_all();
								m_uvc_ctrl_status == em_uvc_control_snap_stop;
							}
						}
						else if (GRAB_IMAGE_MODE_RGB_SPECKLE == m_uvc_param.init_param.grab_mode)
						{
							if (m_frame_buffer_speckle.grab_one_frame_done)
							{
								m_grab_frame_info_rgb.frame_number = m_frame_buffer_rgb.frame_number;
								m_grab_frame_info_rgb.pts = m_frame_buffer_rgb.pts;
								m_grab_frame_info_rgb.data_len = m_frame_buffer_rgb.data_len;
								m_grab_frame_info_rgb.uvc_data = m_frame_buffer_rgb.frame_data;
								m_grab_frame_info_rgb.width = uvc_data_head->width;
								m_grab_frame_info_rgb.height = uvc_data_head->height;

								m_grab_frame_info_speckle.frame_number = m_frame_buffer_speckle.frame_number;
								m_grab_frame_info_speckle.pts = m_frame_buffer_speckle.pts;
								m_grab_frame_info_speckle.data_len = m_frame_buffer_speckle.data_len;
								m_grab_frame_info_speckle.uvc_data = m_frame_buffer_speckle.frame_data;
								m_grab_frame_info_speckle.width = uvc_data_head->width;
								m_grab_frame_info_speckle.height = uvc_data_head->height;

								m_grab_all_frame_info.frame_depth = nullptr;
								m_grab_all_frame_info.frame_rgb = &m_grab_frame_info_rgb;
								m_grab_all_frame_info.frame_speckle = &m_grab_frame_info_speckle;
								m_grab_all_frame_info.frame_ir = nullptr;
								m_grab_all_frame_info.serialNumber = m_uvc_grab_init_parameters.serialNumber;
								m_grab_all_frame_info.pcontex = m_uvc_param.callback_param.uvc_data_ptr;
								m_grab_all_frame_info.temperature = uvc_data_head->temperature;

								m_uvc_param.callback_param.uvc_frame_func(&m_grab_all_frame_info);

								m_frame_buffer_ir.grab_one_frame_done = false;
								m_frame_buffer_rgb.data_len = 0;
								m_frame_buffer_ir.data_len = 0;

								m_cond.notify_all();
								m_uvc_ctrl_status == em_uvc_control_snap_stop;
							}
						}
						else
						{
							m_grab_frame_info_rgb.frame_number = m_frame_buffer_rgb.frame_number;
							m_grab_frame_info_rgb.pts = m_frame_buffer_rgb.pts;
							m_grab_frame_info_rgb.data_len = m_frame_buffer_rgb.data_len;
							m_grab_frame_info_rgb.uvc_data = m_frame_buffer_rgb.frame_data;
							m_grab_frame_info_rgb.width = uvc_data_head->width;
							m_grab_frame_info_rgb.height = uvc_data_head->height;

							m_grab_all_frame_info.frame_depth = nullptr;
							m_grab_all_frame_info.frame_rgb = &m_grab_frame_info_rgb;
							m_grab_all_frame_info.frame_speckle = nullptr;
							m_grab_all_frame_info.frame_ir = nullptr;
							m_grab_all_frame_info.serialNumber = m_uvc_grab_init_parameters.serialNumber;
							m_grab_all_frame_info.pcontex = m_uvc_param.callback_param.uvc_data_ptr;
							m_grab_all_frame_info.temperature = uvc_data_head->temperature;

							m_uvc_param.callback_param.uvc_frame_func(&m_grab_all_frame_info);

							m_frame_buffer_rgb.data_len = 0;
							m_cond.notify_all();
							m_uvc_ctrl_status == em_uvc_control_snap_stop;
						}
					}
					else
					{
						if (GRAB_IMAGE_MODE_RGB_DEPTH == m_uvc_param.init_param.grab_mode)
						{
							if (m_frame_buffer_depth.grab_one_frame_done)
							{	
								m_grab_frame_info_rgb.frame_number = m_frame_buffer_rgb.frame_number;
								m_grab_frame_info_rgb.pts = m_frame_buffer_rgb.pts;
								m_grab_frame_info_rgb.data_len = m_frame_buffer_rgb.data_len;
								m_grab_frame_info_rgb.uvc_data = m_frame_buffer_rgb.frame_data;
								m_grab_frame_info_rgb.width = uvc_data_head->width;
								m_grab_frame_info_rgb.height = uvc_data_head->height;

								m_grab_frame_info_depth.frame_number = m_frame_buffer_depth.frame_number;
								m_grab_frame_info_depth.pts = m_frame_buffer_depth.pts;
								m_grab_frame_info_depth.data_len = m_frame_buffer_depth.data_len;
								m_grab_frame_info_depth.uvc_data = m_frame_buffer_depth.frame_data;
								m_grab_frame_info_depth.width = uvc_data_head->width;
								m_grab_frame_info_depth.height = uvc_data_head->height;

								m_grab_all_frame_info.frame_depth = m_brgb_mode_only ? nullptr : &m_grab_frame_info_depth;
								m_grab_all_frame_info.frame_rgb = &m_grab_frame_info_rgb;
								m_grab_all_frame_info.frame_speckle = nullptr;
								m_grab_all_frame_info.frame_ir = nullptr;
								m_grab_all_frame_info.serialNumber = m_uvc_grab_init_parameters.serialNumber;
								m_grab_all_frame_info.pcontex = m_uvc_param.callback_param.uvc_data_ptr;
								m_grab_all_frame_info.temperature = uvc_data_head->temperature;

								m_uvc_param.callback_param.uvc_frame_func(&m_grab_all_frame_info);
								
								m_frame_buffer_depth.grab_one_frame_done = false;
								m_frame_buffer_rgb.data_len = 0;
								m_frame_buffer_depth.data_len = 0;
							}
							else
							{
								m_frame_buffer_rgb.data_len = 0;
								printf("=====lost depth frame,so  discard rgb frame\n");
							}
						}
						else if (GRAB_IMAGE_MODE_RGB_IR == m_uvc_param.init_param.grab_mode)
						{
							if (m_frame_buffer_ir.grab_one_frame_done)
							{
								m_grab_frame_info_rgb.frame_number = m_frame_buffer_rgb.frame_number;
								m_grab_frame_info_rgb.pts = m_frame_buffer_rgb.pts;
								m_grab_frame_info_rgb.data_len = m_frame_buffer_rgb.data_len;
								m_grab_frame_info_rgb.uvc_data = m_frame_buffer_rgb.frame_data;
								m_grab_frame_info_rgb.width = uvc_data_head->width;
								m_grab_frame_info_rgb.height = uvc_data_head->height;

								m_grab_frame_info_ir.frame_number = m_frame_buffer_ir.frame_number;
								m_grab_frame_info_ir.pts = m_frame_buffer_ir.pts;
								m_grab_frame_info_ir.data_len = m_frame_buffer_ir.data_len;
								m_grab_frame_info_ir.uvc_data = m_frame_buffer_ir.frame_data;
								m_grab_frame_info_ir.width = uvc_data_head->width;
								m_grab_frame_info_ir.height = uvc_data_head->height;

								m_grab_all_frame_info.frame_depth = nullptr;
								m_grab_all_frame_info.frame_rgb = &m_grab_frame_info_rgb;
								m_grab_all_frame_info.frame_speckle = nullptr;
								m_grab_all_frame_info.frame_ir = &m_grab_frame_info_ir;
								m_grab_all_frame_info.serialNumber = m_uvc_grab_init_parameters.serialNumber;
								m_grab_all_frame_info.pcontex = m_uvc_param.callback_param.uvc_data_ptr;
								m_grab_all_frame_info.temperature = uvc_data_head->temperature;

								m_uvc_param.callback_param.uvc_frame_func( &m_grab_all_frame_info);

								m_frame_buffer_ir.grab_one_frame_done = false;
								m_frame_buffer_rgb.data_len = 0;
								m_frame_buffer_ir.data_len = 0;
							}
							else
							{
								m_frame_buffer_rgb.data_len = 0;
								printf("=====lost ir frame,so  discard rgb frame\n");
							}
						}
						else if (GRAB_IMAGE_MODE_RGB_SPECKLE == m_uvc_param.init_param.grab_mode)
						{
							if (m_frame_buffer_speckle.grab_one_frame_done)
							{
								m_grab_frame_info_rgb.frame_number = m_frame_buffer_rgb.frame_number;
								m_grab_frame_info_rgb.pts = m_frame_buffer_rgb.pts;
								m_grab_frame_info_rgb.data_len = m_frame_buffer_rgb.data_len;
								m_grab_frame_info_rgb.uvc_data = m_frame_buffer_rgb.frame_data;
								m_grab_frame_info_rgb.width = uvc_data_head->width;
								m_grab_frame_info_rgb.height = uvc_data_head->height;

								m_grab_frame_info_speckle.frame_number = m_frame_buffer_speckle.frame_number;
								m_grab_frame_info_speckle.pts = m_frame_buffer_speckle.pts;
								m_grab_frame_info_speckle.data_len = m_frame_buffer_speckle.data_len;
								m_grab_frame_info_speckle.uvc_data = m_frame_buffer_speckle.frame_data;
								m_grab_frame_info_speckle.width = uvc_data_head->width;
								m_grab_frame_info_speckle.height = uvc_data_head->height;

								m_grab_all_frame_info.frame_depth = nullptr;
								m_grab_all_frame_info.frame_rgb = &m_grab_frame_info_rgb;
								m_grab_all_frame_info.frame_speckle = &m_grab_frame_info_speckle;
								m_grab_all_frame_info.frame_ir = nullptr;
								m_grab_all_frame_info.serialNumber = m_uvc_grab_init_parameters.serialNumber;
								m_grab_all_frame_info.pcontex = m_uvc_param.callback_param.uvc_data_ptr;
								m_grab_all_frame_info.temperature = uvc_data_head->temperature;

								m_uvc_param.callback_param.uvc_frame_func(&m_grab_all_frame_info);

								m_frame_buffer_ir.grab_one_frame_done = false;
								m_frame_buffer_rgb.data_len = 0;
								m_frame_buffer_ir.data_len = 0;
							}
							else
							{
								m_frame_buffer_rgb.data_len = 0;
								printf("=====lost speckle frame,so  discard rgb frame\n");
							}
						}
						else
						{
							m_grab_frame_info_rgb.frame_number = m_frame_buffer_rgb.frame_number;
							m_grab_frame_info_rgb.pts = m_frame_buffer_rgb.pts;
							m_grab_frame_info_rgb.data_len = m_frame_buffer_rgb.data_len;
							m_grab_frame_info_rgb.uvc_data = m_frame_buffer_rgb.frame_data;
							m_grab_frame_info_rgb.width = uvc_data_head->width;
							m_grab_frame_info_rgb.height = uvc_data_head->height;

							m_grab_all_frame_info.frame_depth = nullptr;
							m_grab_all_frame_info.frame_rgb = &m_grab_frame_info_rgb;
							m_grab_all_frame_info.frame_speckle = nullptr;
							m_grab_all_frame_info.frame_ir = nullptr;
							m_grab_all_frame_info.serialNumber = m_uvc_grab_init_parameters.serialNumber;
							m_grab_all_frame_info.pcontex = m_uvc_param.callback_param.uvc_data_ptr;
							m_grab_all_frame_info.temperature = uvc_data_head->temperature;

							m_uvc_param.callback_param.uvc_frame_func(&m_grab_all_frame_info);
							m_frame_buffer_rgb.data_len = 0;
						}
					}
				}
			}
		}
	}
	//depth
	else if (uvc_data_head->data_start_code == g_uvc_depth_data_start_code)
	{
		if (1 == uvc_data_head->data_type)
		{
			m_frame_buffer_depth.grab_one_frame_done = false;
			m_frame_buffer_depth.data_len = 0;
			if (m_frame_buffer_depth.data_len + uvc_data_head->data_size > uvc_data_head->frame_size)
			{
				printf("====[%d]error depth frame size:data_type:%d,cur_index:%d,data_size:%d,frame_size:%d\n", m_frame_buffer_depth.frame_err_cnt++, uvc_data_head->data_type, m_frame_buffer_depth.data_len, uvc_data_head->data_size, uvc_data_head->frame_size);
			}
			else
			{
				memcpy(m_frame_buffer_depth.frame_data + m_frame_buffer_depth.data_len, frame_data, uvc_data_head->data_size);
				m_frame_buffer_depth.data_len += uvc_data_head->data_size;
			}
		}
		else if (2 == uvc_data_head->data_type)
		{
			if (m_frame_buffer_depth.data_len + uvc_data_head->data_size > uvc_data_head->frame_size)
			{
				printf("====[%d]error depth frame size:data_type:%d,cur_index:%d,data_size:%d,frame_size:%d\n", m_frame_buffer_depth.frame_err_cnt++, uvc_data_head->data_type, m_frame_buffer_depth.data_len, uvc_data_head->data_size, uvc_data_head->frame_size);
			}
			else
			{
				memcpy(m_frame_buffer_depth.frame_data + m_frame_buffer_depth.data_len, frame_data, uvc_data_head->data_size);
				m_frame_buffer_depth.data_len += uvc_data_head->data_size;
			}

		}
		else if (3 == uvc_data_head->data_type)
		{
			if (m_frame_buffer_depth.data_len + uvc_data_head->data_size > uvc_data_head->frame_size)
			{
				printf("====[%d]error depth frame size:data_type:%d,cur_index:%d,data_size:%d,frame_size:%d\n", m_frame_buffer_depth.frame_err_cnt++, uvc_data_head->data_type, m_frame_buffer_depth.data_len, uvc_data_head->data_size, uvc_data_head->frame_size);
			}
			else if (m_frame_buffer_depth.data_len + uvc_data_head->data_size == uvc_data_head->frame_size)
			{
				memcpy(m_frame_buffer_depth.frame_data + m_frame_buffer_depth.data_len, frame_data, uvc_data_head->data_size);
				m_frame_buffer_depth.data_len += uvc_data_head->data_size;
				m_frame_buffer_depth.pts = uvc_data_head->pts;
				m_frame_buffer_depth.frame_number = uvc_data_head->frame_number;
				if (m_uvc_param.callback_param.uvc_frame_func != nullptr)
				{
					if (m_uvc_ctrl_status == em_uvc_control_snap_start)
					{
						if (GRAB_IMAGE_MODE_RGB_DEPTH == m_uvc_param.init_param.grab_mode)
						{
							m_frame_buffer_depth.grab_one_frame_done = true;
						}
						else
						{
							m_grab_frame_info_depth.frame_number = m_frame_buffer_depth.frame_number;
							m_grab_frame_info_depth.pts = m_frame_buffer_depth.pts;
							m_grab_frame_info_depth.data_len = m_frame_buffer_depth.data_len;
							m_grab_frame_info_depth.uvc_data = m_frame_buffer_depth.frame_data;
							m_grab_frame_info_depth.width = uvc_data_head->width;
							m_grab_frame_info_depth.height = uvc_data_head->height;

							m_grab_all_frame_info.frame_depth = &m_grab_frame_info_depth;
							m_grab_all_frame_info.frame_rgb = nullptr;
							m_grab_all_frame_info.frame_speckle = nullptr;
							m_grab_all_frame_info.frame_ir = nullptr;
							m_grab_all_frame_info.serialNumber = m_uvc_grab_init_parameters.serialNumber;
							m_grab_all_frame_info.pcontex = m_uvc_param.callback_param.uvc_data_ptr;
							m_grab_all_frame_info.temperature = uvc_data_head->temperature;

							m_uvc_param.callback_param.uvc_frame_func( &m_grab_all_frame_info);

							m_frame_buffer_depth.grab_one_frame_done = false;
							m_frame_buffer_depth.data_len = 0;

							m_cond.notify_all();
							m_uvc_ctrl_status == em_uvc_control_snap_stop;
						}

					}
					else
					{
						if (GRAB_IMAGE_MODE_RGB_DEPTH == m_uvc_param.init_param.grab_mode)
						{
							m_frame_buffer_depth.grab_one_frame_done = true;
						}
						else
						{
							m_frame_buffer_depth.grab_one_frame_done = false;

							m_grab_frame_info_depth.frame_number = m_frame_buffer_depth.frame_number;
							m_grab_frame_info_depth.pts = m_frame_buffer_depth.pts;
							m_grab_frame_info_depth.data_len = m_frame_buffer_depth.data_len;
							m_grab_frame_info_depth.uvc_data = m_frame_buffer_depth.frame_data;
							m_grab_frame_info_depth.width = uvc_data_head->width;
							m_grab_frame_info_depth.height = uvc_data_head->height;

							m_grab_all_frame_info.frame_depth = &m_grab_frame_info_depth;
							m_grab_all_frame_info.frame_rgb = nullptr;
							m_grab_all_frame_info.frame_speckle = nullptr;
							m_grab_all_frame_info.frame_ir = nullptr;
							m_grab_all_frame_info.serialNumber = m_uvc_grab_init_parameters.serialNumber;
							m_grab_all_frame_info.pcontex = m_uvc_param.callback_param.uvc_data_ptr;
							m_grab_all_frame_info.temperature = uvc_data_head->temperature;

							m_uvc_param.callback_param.uvc_frame_func( &m_grab_all_frame_info);
							m_frame_buffer_depth.data_len = 0;

						}
					}
				}
			}
		}
	}
	//ir
	else if (uvc_data_head->data_start_code == g_uvc_ir_data_start_code)
	{
		if (1 == uvc_data_head->data_type)
		{
			m_frame_buffer_ir.grab_one_frame_done = false;
			m_frame_buffer_ir.data_len = 0;
			if (m_frame_buffer_ir.data_len + uvc_data_head->data_size > uvc_data_head->frame_size)
			{
				printf("====[%d]error ir frame size:data_type:%d,cur_index:%d,data_size:%d,frame_size:%d\n", m_frame_buffer_ir.frame_err_cnt++, uvc_data_head->data_type, m_frame_buffer_ir.data_len, uvc_data_head->data_size, uvc_data_head->frame_size);
			}
			else
			{
				memcpy(m_frame_buffer_ir.frame_data + m_frame_buffer_ir.data_len, frame_data, uvc_data_head->data_size);
				m_frame_buffer_ir.data_len += uvc_data_head->data_size;
			}
		}
		else if (2 == uvc_data_head->data_type)
		{
			if (m_frame_buffer_ir.data_len + uvc_data_head->data_size > uvc_data_head->frame_size)
			{
				printf("====[%d]error ir frame size:data_type:%d,cur_index:%d,data_size:%d,frame_size:%d\n", m_frame_buffer_ir.frame_err_cnt++, uvc_data_head->data_type, m_frame_buffer_ir.data_len, uvc_data_head->data_size, uvc_data_head->frame_size);
			}
			else
			{
				memcpy(m_frame_buffer_ir.frame_data + m_frame_buffer_ir.data_len, frame_data, uvc_data_head->data_size);
				m_frame_buffer_ir.data_len += uvc_data_head->data_size;
			}

		}
		else if (3 == uvc_data_head->data_type)
		{
			if (m_frame_buffer_ir.data_len + uvc_data_head->data_size > uvc_data_head->frame_size)
			{
				printf("====[%d]error ir frame size:data_type:%d,cur_index:%d,data_size:%d,frame_size:%d\n", m_frame_buffer_ir.frame_err_cnt++, uvc_data_head->data_type, m_frame_buffer_ir.data_len, uvc_data_head->data_size, uvc_data_head->frame_size);
			}
			else if (m_frame_buffer_ir.data_len + uvc_data_head->data_size == uvc_data_head->frame_size)
			{
				memcpy(m_frame_buffer_ir.frame_data + m_frame_buffer_ir.data_len, frame_data, uvc_data_head->data_size);
				m_frame_buffer_ir.data_len += uvc_data_head->data_size;
				m_frame_buffer_ir.pts = uvc_data_head->pts;
				m_frame_buffer_ir.frame_number = uvc_data_head->frame_number;
				if (m_uvc_param.callback_param.uvc_frame_func != nullptr)
				{
					if (m_uvc_ctrl_status == em_uvc_control_snap_start)
					{
						if (GRAB_IMAGE_MODE_RGB_IR == m_uvc_param.init_param.grab_mode)
						{
							m_frame_buffer_ir.grab_one_frame_done = true;
						}
						else
						{
							m_grab_frame_info_ir.frame_number = m_frame_buffer_ir.frame_number;
							m_grab_frame_info_ir.pts = m_frame_buffer_ir.pts;
							m_grab_frame_info_ir.data_len = m_frame_buffer_ir.data_len;
							m_grab_frame_info_ir.uvc_data = m_frame_buffer_ir.frame_data;
							m_grab_frame_info_ir.width = uvc_data_head->width;
							m_grab_frame_info_ir.height = uvc_data_head->height;

							m_grab_all_frame_info.frame_depth = nullptr;
							m_grab_all_frame_info.frame_rgb = nullptr;
							m_grab_all_frame_info.frame_speckle = nullptr;
							m_grab_all_frame_info.frame_ir = &m_grab_frame_info_ir;
							m_grab_all_frame_info.serialNumber = m_uvc_grab_init_parameters.serialNumber;
							m_grab_all_frame_info.pcontex = m_uvc_param.callback_param.uvc_data_ptr;
							m_grab_all_frame_info.temperature = uvc_data_head->temperature;

							m_uvc_param.callback_param.uvc_frame_func( &m_grab_all_frame_info);

							m_frame_buffer_ir.grab_one_frame_done = false;
							m_frame_buffer_ir.data_len = 0;
							m_cond.notify_all();
							m_uvc_ctrl_status == em_uvc_control_snap_stop;
						}

					}
					else
					{
						if (GRAB_IMAGE_MODE_RGB_IR == m_uvc_param.init_param.grab_mode)
						{
							m_frame_buffer_ir.grab_one_frame_done = true;
						}
						else
						{
							m_frame_buffer_ir.grab_one_frame_done = false;

							m_grab_frame_info_ir.frame_number = m_frame_buffer_ir.frame_number;
							m_grab_frame_info_ir.pts = m_frame_buffer_ir.pts;
							m_grab_frame_info_ir.data_len = m_frame_buffer_ir.data_len;
							m_grab_frame_info_ir.uvc_data = m_frame_buffer_ir.frame_data;
							m_grab_frame_info_ir.width = uvc_data_head->width;
							m_grab_frame_info_ir.height = uvc_data_head->height;

							m_grab_all_frame_info.frame_depth = nullptr;
							m_grab_all_frame_info.frame_rgb = nullptr;
							m_grab_all_frame_info.frame_speckle = nullptr;
							m_grab_all_frame_info.frame_ir = &m_grab_frame_info_ir;
							m_grab_all_frame_info.serialNumber = m_uvc_grab_init_parameters.serialNumber;
							m_grab_all_frame_info.pcontex = m_uvc_param.callback_param.uvc_data_ptr;
							m_grab_all_frame_info.temperature = uvc_data_head->temperature;

							m_uvc_param.callback_param.uvc_frame_func(&m_grab_all_frame_info);
							m_frame_buffer_ir.data_len = 0;

						}
					}
				}
			}
		}
	}
	//speckle
	else if (uvc_data_head->data_start_code == g_uvc_speckle_data_start_code)
	{
		if (1 == uvc_data_head->data_type)
		{
			m_frame_buffer_speckle.grab_one_frame_done = false;
			m_frame_buffer_speckle.data_len = 0;
			if (m_frame_buffer_speckle.data_len + uvc_data_head->data_size > uvc_data_head->frame_size)
			{
				printf("====[%d]error speckle frame size:data_type:%d,cur_index:%d,data_size:%d,frame_size:%d\n", m_frame_buffer_speckle.frame_err_cnt++, uvc_data_head->data_type, m_frame_buffer_speckle.data_len, uvc_data_head->data_size, uvc_data_head->frame_size);
			}
			else
			{
				memcpy(m_frame_buffer_speckle.frame_data + m_frame_buffer_speckle.data_len, frame_data, uvc_data_head->data_size);
				m_frame_buffer_speckle.data_len += uvc_data_head->data_size;
			}
		}
		else if (2 == uvc_data_head->data_type)
		{
			if (m_frame_buffer_speckle.data_len + uvc_data_head->data_size > uvc_data_head->frame_size)
			{
				printf("====[%d]error speckle frame size:data_type:%d,cur_index:%d,data_size:%d,frame_size:%d\n", m_frame_buffer_speckle.frame_err_cnt++, uvc_data_head->data_type, m_frame_buffer_speckle.data_len, uvc_data_head->data_size, uvc_data_head->frame_size);
			}
			else
			{
				memcpy(m_frame_buffer_speckle.frame_data + m_frame_buffer_speckle.data_len, frame_data, uvc_data_head->data_size);
				m_frame_buffer_speckle.data_len += uvc_data_head->data_size;
			}

		}
		else if (3 == uvc_data_head->data_type)
		{
			if (m_frame_buffer_speckle.data_len + uvc_data_head->data_size > uvc_data_head->frame_size)
			{
				printf("====[%d]error depth frame size:data_type:%d,cur_index:%d,data_size:%d,frame_size:%d\n", m_frame_buffer_speckle.frame_err_cnt++, uvc_data_head->data_type, m_frame_buffer_speckle.data_len, uvc_data_head->data_size, uvc_data_head->frame_size);
			}
			else if (m_frame_buffer_speckle.data_len + uvc_data_head->data_size == uvc_data_head->frame_size)
			{
				memcpy(m_frame_buffer_speckle.frame_data + m_frame_buffer_speckle.data_len, frame_data, uvc_data_head->data_size);
				m_frame_buffer_speckle.data_len += uvc_data_head->data_size;
				m_frame_buffer_speckle.pts = uvc_data_head->pts;
				m_frame_buffer_speckle.frame_number = uvc_data_head->frame_number;
				if (m_uvc_param.callback_param.uvc_frame_func != nullptr)
				{
					if (m_uvc_ctrl_status == em_uvc_control_snap_start)
					{
						if (GRAB_IMAGE_MODE_RGB_SPECKLE == m_uvc_param.init_param.grab_mode)
						{
							m_frame_buffer_speckle.grab_one_frame_done = true;
						}
						else if (GRAB_IMAGE_MODE_NONE_SPECKLE == m_uvc_param.init_param.grab_mode)
						{
							m_grab_frame_info_speckle.frame_number = m_frame_buffer_speckle.frame_number;
							m_grab_frame_info_speckle.pts = m_frame_buffer_speckle.pts;
							m_grab_frame_info_speckle.data_len = m_frame_buffer_speckle.data_len;
							m_grab_frame_info_speckle.uvc_data = m_frame_buffer_speckle.frame_data;
							m_grab_frame_info_speckle.width = uvc_data_head->width;
							m_grab_frame_info_speckle.height = uvc_data_head->height;

							m_grab_all_frame_info.frame_depth = nullptr;
							m_grab_all_frame_info.frame_rgb = nullptr;
							m_grab_all_frame_info.frame_speckle = &m_grab_frame_info_speckle;
							m_grab_all_frame_info.frame_ir = nullptr;
							m_grab_all_frame_info.serialNumber = m_uvc_grab_init_parameters.serialNumber;
							m_grab_all_frame_info.pcontex = m_uvc_param.callback_param.uvc_data_ptr;
							m_grab_all_frame_info.temperature = uvc_data_head->temperature;

							m_uvc_param.callback_param.uvc_frame_func(&m_grab_all_frame_info);

							m_frame_buffer_speckle.grab_one_frame_done = false;
							m_frame_buffer_speckle.data_len = 0;
							m_cond.notify_all();
							m_uvc_ctrl_status == em_uvc_control_snap_stop;
						}

					}
					else
					{
						if (GRAB_IMAGE_MODE_RGB_SPECKLE == m_uvc_param.init_param.grab_mode)
						{
							m_frame_buffer_speckle.grab_one_frame_done = true;
						}
						else if (GRAB_IMAGE_MODE_NONE_SPECKLE == m_uvc_param.init_param.grab_mode)
						{
							m_frame_buffer_speckle.grab_one_frame_done = false;

							m_grab_frame_info_speckle.frame_number = m_frame_buffer_speckle.frame_number;
							m_grab_frame_info_speckle.pts = m_frame_buffer_speckle.pts;
							m_grab_frame_info_speckle.data_len = m_frame_buffer_speckle.data_len;
							m_grab_frame_info_speckle.uvc_data = m_frame_buffer_speckle.frame_data;
							m_grab_frame_info_speckle.width = uvc_data_head->width;
							m_grab_frame_info_speckle.height = uvc_data_head->height;

							m_grab_all_frame_info.frame_depth = nullptr;
							m_grab_all_frame_info.frame_rgb = nullptr;
							m_grab_all_frame_info.frame_speckle = &m_grab_frame_info_speckle;
							m_grab_all_frame_info.frame_ir = nullptr;
							m_grab_all_frame_info.serialNumber = m_uvc_grab_init_parameters.serialNumber;
							m_grab_all_frame_info.pcontex = m_uvc_param.callback_param.uvc_data_ptr;
							m_grab_all_frame_info.temperature = uvc_data_head->temperature;

							m_uvc_param.callback_param.uvc_frame_func( &m_grab_all_frame_info);
							m_frame_buffer_speckle.data_len = 0;

						}
					}
				}
			}
		}
	}
	return 0;
}

bool kd_uvc_dev_imp::_do_uvc_start_stream(bool bstart)
{
	if (bstart)
	{
		int ret;
		ret = kd_start_uvc_camera_device(&m_uvc_device);
		return 0 == ret;
	}
	else
	{
		int ret;
		ret = kd_stop_uvc_camera_device(&m_uvc_device);
		return 0 == ret;
	}

	return true;
}

int kd_uvc_dev_imp::_update_dev_cfg(const grabber_init_param& pInitParam)
{
#if 0
	if (pInitParam.init_param.grab_mode == GRAB_IMAGE_MODE_RGB_NONE)
	{
		m_uvc_grab_init_parameters.grab_mode = GRAB_IMAGE_MODE_RGB_DEPTH;
	}
	else
	{
		m_uvc_grab_init_parameters.grab_mode = pInitParam.init_param.grab_mode;
	}
#else
	m_uvc_grab_init_parameters.grab_mode = pInitParam.init_param.grab_mode;
#endif 
	m_uvc_grab_init_parameters.sensor_type[0] = pInitParam.init_param.sensor_type[0];
	m_uvc_grab_init_parameters.sensor_type[1] = pInitParam.init_param.sensor_type[1];
	m_uvc_grab_init_parameters.adc_enable = pInitParam.init_param.adc_enable;
	m_uvc_grab_init_parameters.overwrite_file = pInitParam.init_param.overwrite_file;


	m_uvc_grab_init_parameters.camera_fps = pInitParam.init_param.camera_fps;
	m_uvc_grab_init_parameters.depth_maximum_distance = pInitParam.init_param.depth_maximum_distance;
	m_uvc_grab_init_parameters.camera_width = pInitParam.init_param.camera_width;
	m_uvc_grab_init_parameters.camera_height = pInitParam.init_param.camera_height;
	m_uvc_grab_init_parameters.dma_ro = pInitParam.init_param.dma_ro;

	//decrypt file
	if (0 != _decrypt_bin_file(pInitParam))
	{
		return -1;
	}

	if (pInitParam.init_param.overwrite_file)
	{
		if (0 != strcmp(m_uvc_serialNumber, pInitParam.init_param.serialNumber))
		{
			printf("serialNumber not match(%s_%s)\n", m_uvc_serialNumber, pInitParam.init_param.serialNumber);
			return -1;
		}
	}

	//update serialnumber and temperature
	memcpy(m_uvc_grab_init_parameters.serialNumber, m_uvc_serialNumber, sizeof(m_uvc_serialNumber));
	m_uvc_grab_init_parameters.temperature.kxppt = m_calib_data.info.kxppt;
	m_uvc_grab_init_parameters.temperature.kyppt = m_calib_data.info.kyppt;
	m_uvc_grab_init_parameters.temperature.temperature_cx = m_calib_data.info.temperature_cx;
	m_uvc_grab_init_parameters.temperature.temperature_cy = m_calib_data.info.temperature_cy;
	m_uvc_grab_init_parameters.temperature.temperature_ref = m_calib_data.info.temperature_ref;
	
	{
		uvc_grab_init_parameters_ex* grab_init_param = &m_uvc_grab_init_parameters;
		printf("grab param:mode:%d,sensor type0:%d,sensor type1:%d,adc_enable:%d,overwrite:%d,serial_num:%s, width:%d,height:%d,fps:%d,distance:%d,adc_enable:%d,t_ref:%f,t_cx:%f,t_cy:%f,kx:%.2f,ky:%.2f\n",
			grab_init_param->grab_mode, grab_init_param->sensor_type[0], grab_init_param->sensor_type[1], grab_init_param->adc_enable,
			grab_init_param->overwrite_file, grab_init_param->serialNumber,
			grab_init_param->camera_width, grab_init_param->camera_height, grab_init_param->camera_fps, grab_init_param->depth_maximum_distance, grab_init_param->adc_enable,
			grab_init_param->temperature.temperature_ref, grab_init_param->temperature.temperature_cx, grab_init_param->temperature.temperature_cy,
			grab_init_param->temperature.kxppt, grab_init_param->temperature.kyppt);
	}

	//get all uvc devices ,match serialNumber
	UVC_DEV_INFO_LIST lst_dev_uvc_info;
	if (!kd_uvc_get_all_uvc_dev_info(true, lst_dev_uvc_info))
	{
		printf("no k230 device\n");
		return -1;
	}



	bool bfind = false;
	for (UVC_DEV_INFO_LIST::iterator itr = lst_dev_uvc_info.begin(); itr != lst_dev_uvc_info.end(); itr++)
	{
		if (0 == strcmp(itr->serialNumber, m_uvc_serialNumber))
		{
			printf("find k230 device serialNumber:%s\n", m_uvc_serialNumber);
			bfind = true;
			break;
		}
	}

	int i = 0;
	for (UVC_DEV_INFO_LIST::iterator itr = lst_dev_uvc_info.begin(); itr != lst_dev_uvc_info.end(); itr++)
	{
		printf("[%d]serialNumber:%s\n", i, itr->serialNumber);
		i++;
	}
	printf("=========k230 cnt:%d,update serialnumber:%s,find:%d,overwrite:%d\n", lst_dev_uvc_info.size(), m_uvc_serialNumber, bfind, pInitParam.init_param.overwrite_file);

	//没有找到当前设备，并且overwrite_file为true，只有当前一个设备时才更新。
	if (!bfind && pInitParam.init_param.overwrite_file && lst_dev_uvc_info.size() >1)
	{
		printf("find k230 device serialNumber:%s failed,update failed\n", m_uvc_serialNumber);
		return -1;
	}
	else if (!bfind && !pInitParam.init_param.overwrite_file && lst_dev_uvc_info.size() > 1)
	{
		printf("find k230 device serialNumber:%s failed,update failed\n", m_uvc_serialNumber);
		return -1;
	}
	else if (!bfind && !pInitParam.init_param.overwrite_file && lst_dev_uvc_info.size() == 1)
	{
		printf("find k230 device serialNumber:%s failed,update failed\n", m_uvc_serialNumber);
		return -1;
	}
	

	//transfer ref and conf file
	//When will it be updated? Update only when no matching UVC serial number is found (currently not updated) or when forced to update.
	bool bupdate_ref_conf_file = !bfind || pInitParam.init_param.overwrite_file;
	if (!bupdate_ref_conf_file)
	{
		printf("no neeed to update ref/conf file\n");
	}
	
	printf("update ref/conf file ... \n");
	if (bupdate_ref_conf_file)
	{
		_kd_uvc_transfer_file_data(bfind ? m_uvc_serialNumber : nullptr, m_pref_file_data, m_ref_file_data_len, m_ref_file_path_name, nullptr);
		_kd_uvc_transfer_file_data(bfind ? m_uvc_serialNumber : nullptr, m_pconf_file_data, m_conf_file_data_len, m_conf_file_path_name, nullptr);
	}
	
	//transfer config
	printf("update cfg ... \n");
	_kd_uvc_transfer_cfg_data(bfind ? m_uvc_serialNumber : nullptr, nullptr);

	//set framerate
	{
		printf("set framerate:%d ...\n", pInitParam.init_param.camera_fps);
		UVC_TRANSFER_CONTROL_CMD cmd;
		cmd.type = em_uvc_transfer_control_set_framerate;
		cmd.ctrl_info.frame_rate = pInitParam.init_param.camera_fps;
		_kd_uvc_transfer_control_cmd(bfind ? m_uvc_serialNumber : nullptr, cmd,true);
	}

	//sync clock
	{
		printf("sync clock ...\n");
		UVC_TRANSFER_CONTROL_CMD cmd;
		cmd.type = em_uvc_transfer_control_sync_clock;

		cmd.ctrl_info.sync_clock = get_system_time_microsecond()+ SYNC_K230_TIMESTAMP_DELAY_TIME;
		_kd_uvc_transfer_control_cmd(bfind ? m_uvc_serialNumber : nullptr, cmd, true);
		printf("sync clock:%llu\n", cmd.ctrl_info.sync_clock);
	}

	return 0;
}

int kd_uvc_dev_imp::_transfer_cfg()
{
	int ret;
	UVC_TRANSFER_DATA_HEAD_INFO transfer_head;

	//init uvc transfer with pid vid
	if (0 != kd_uvc_transfer_init(K230_UVC_DEVICE_VID, K230_UVC_DEVICE_PID, nullptr,true))
	{
		return -1;
	}

	//transfer cfg start
	memcpy(&transfer_head.data_start_code, UVC_TRANSFER_START_HEAD, strlen(UVC_TRANSFER_START_HEAD));
	transfer_head.tranfer_type = em_uvc_transfer_data_type_cfg;
	ret = kd_uvc_transfer_data((unsigned char*)&transfer_head, sizeof(transfer_head));
	if (ret != 0)
	{
		printf("==kd_uvc_transfer_file failed\n");
	}


	
	m_uvc_grab_init_parameters.grab_mode = GRAB_IMAGE_MODE_IR_DEPTH;
	m_uvc_grab_init_parameters.sensor_type[0] = 11;
	m_uvc_grab_init_parameters.sensor_type[1] = 22;
	m_uvc_grab_init_parameters.adc_enable = 1;
	m_uvc_grab_init_parameters.overwrite_file = 1;
	sprintf(m_uvc_grab_init_parameters.serialNumber, "sunxiaopeng233%d", 567);
	m_uvc_grab_init_parameters.camera_width = 1280;
	m_uvc_grab_init_parameters.camera_height = 720;
	m_uvc_grab_init_parameters.camera_fps = 30;
	m_uvc_grab_init_parameters.depth_maximum_distance = 40000;

	m_uvc_grab_init_parameters.temperature.kxppt = 34.554;



	int total_size = sizeof(m_uvc_grab_init_parameters);
	unsigned char* pcfg_data = (unsigned char*)&m_uvc_grab_init_parameters;
	int transfer_cnt = total_size / MAX_TRANSFER_SIZE + (0 != total_size % MAX_TRANSFER_SIZE);
	printf("=====transfer cfg size:%d,transfer cnt:%d\n", total_size,transfer_cnt);
	for (int i = 0; i < transfer_cnt; i++)
	{

		ret = kd_uvc_transfer_data(pcfg_data + MAX_TRANSFER_SIZE*i, (i == transfer_cnt - 1)? (total_size - (transfer_cnt -1)* MAX_TRANSFER_SIZE):MAX_TRANSFER_SIZE);
		if (ret != 0)
		{
			printf("==kd_uvc_transfer_file failed\n");
		}

	}
	

	//transfer cfg stop
	memcpy(&transfer_head.data_start_code, UVC_TRANSFER_STOP_HEAD, strlen(UVC_TRANSFER_STOP_HEAD));
	transfer_head.tranfer_type = em_uvc_transfer_data_type_cfg;
	ret = kd_uvc_transfer_data((unsigned char*)&transfer_head, sizeof(transfer_head));
	if (ret != 0)
	{
		printf("==kd_uvc_transfer_file failed\n");
	}

	kd_uvc_transfer_deinit(true);


	uvc_grab_init_parameters_ex* grab_init_param = &m_uvc_grab_init_parameters;
	printf("grab param:mode:%d,sensor type0:%d,sensor type1:%d,adc_enable:%d,overwrite:%d,serial_num:%s, width:%d,height:%d,fps:%d,distance:%d,adc_enable:%d,t_ref:%f,t_cx:%f,t_cy:%f,kx:%.2f,ky:%.2f\n",
		grab_init_param->grab_mode, grab_init_param->sensor_type[0], grab_init_param->sensor_type[1], grab_init_param->adc_enable,
		grab_init_param->overwrite_file, grab_init_param->serialNumber,
		grab_init_param->camera_width, grab_init_param->camera_height, grab_init_param->depth_maximum_distance, grab_init_param->adc_enable,
		grab_init_param->temperature.temperature_ref, grab_init_param->temperature.temperature_cx, grab_init_param->temperature.temperature_cy,
		grab_init_param->temperature.kxppt, grab_init_param->temperature.kyppt);

	return 0;
}

int kd_uvc_dev_imp::_decrypt_bin_file(const grabber_init_param& pInitParam)
{
	if (pInitParam.init_param.cfg_file_path_name == nullptr)
	{
		return -1;
	}
	printf("decrypt bin file:%s\n", pInitParam.init_param.cfg_file_path_name);

	FILE* pFile;
	int file_size;
	int ret = 0;
	int data_index = 0;
	
	//open src file
	pFile = fopen(pInitParam.init_param.cfg_file_path_name, "rb");
	if (pFile == nullptr)
	{
		printf("open file:%s failed\n", pInitParam.init_param.cfg_file_path_name);
		return -1;
	}
	
	//get file size
	fseek(pFile, 0, SEEK_END);
	file_size = ftell(pFile);
	printf("file:%s size:%d Bytes\n", pInitParam.init_param.cfg_file_path_name, file_size);
	rewind(pFile);
	
	//load file data
	if (m_decrypt_file_data != nullptr)
	{
		m_decrypt_file_data.reset();
	}
	m_decrypt_file_data = make_unique<unsigned char[]>(file_size);

	unsigned char* pfile_data = m_decrypt_file_data.get();
	fread(pfile_data, 1, file_size, pFile);
	fclose(pFile);

	//decrypt file data
	for (int i = 0; i < file_size; i++)
	{
		pfile_data[i] = decrypt_(pfile_data[i]);
	}

	data_index = sizeof(m_calib_data.param_bin_name) + sizeof(m_calib_data.info) + sizeof(m_calib_data.ref_size_);
	memcpy(&m_calib_data, pfile_data, data_index);

	//printf("=====param_bin_name:%s\n", data_file_info.param_bin_name);
	//printf("=====ref_size_:%d\n", data_file_info.ref_size_);
	memset(m_uvc_serialNumber, 0, sizeof(m_uvc_serialNumber));
	if (pInitParam.init_param.overwrite_file)
	{
		strncpy(m_uvc_serialNumber, m_calib_data.param_bin_name, sizeof(m_uvc_serialNumber));
	}
	else
	{
		strncpy(m_uvc_serialNumber, pInitParam.init_param.serialNumber, sizeof(m_uvc_serialNumber));
	}
	
	
	//printf("=====serialNumber:%s\n", m_uvc_serialNumber);

	m_pref_file_data = pfile_data + data_index;
	m_ref_file_data_len = m_calib_data.ref_size_;

	m_pconf_file_data = pfile_data + data_index + m_calib_data.ref_size_;
	m_conf_file_data_len = file_size - data_index - m_calib_data.ref_size_;

	//get camera info
	unsigned char* p_buffer2_binparams = m_pconf_file_data;
	//get ushort height width
	unsigned char* p_buffer2_binparams_ushort = p_buffer2_binparams + 52;
	unsigned short* p_params_ushort = reinterpret_cast<unsigned short*>(p_buffer2_binparams_ushort);

	m_uvc_camera_info.ir_width = p_params_ushort[0];
	m_uvc_camera_info.ir_height = p_params_ushort[1];
	m_uvc_camera_info.rgb_width = p_params_ushort[2];
	m_uvc_camera_info.rgb_height = p_params_ushort[3];

	//get float - cx cy
	m_uvc_camera_info.ir_fx = m_calib_data.info.reserve[0];
	m_uvc_camera_info.ir_fy = m_calib_data.info.reserve[1];
	m_uvc_camera_info.ir_cx = m_calib_data.info.reserve[2];
	m_uvc_camera_info.ir_cy = m_calib_data.info.reserve[3];

	unsigned char* p_buffer2_binparams_float = p_buffer2_binparams + 72;
	float* p_params_float = reinterpret_cast<float*>(p_buffer2_binparams_float);
	m_uvc_camera_info.rgb_fx = p_params_float[18];//rgb
	m_uvc_camera_info.rgb_fy = p_params_float[19];
	m_uvc_camera_info.rgb_cx = p_params_float[20];
	m_uvc_camera_info.rgb_cy = p_params_float[21];
	m_uvc_camera_info.depth_precision = p_params_float[24];

	printf("bin file:%s param info,ir_width:%d,ir_height:%d,rgb_width:%d,rgb_height:%d,ir_fx:%f,ir_fy:%f,ir_cx:%f,ir_cy:%f,rgb_fx:%f,rgb_fy:%f,rgb_cx:%f,rgb_cy:%f,depth_precision:%f\n", \
		pInitParam.init_param.cfg_file_path_name, m_uvc_camera_info.ir_width, m_uvc_camera_info.ir_height, m_uvc_camera_info.rgb_width, m_uvc_camera_info.rgb_height, \
		m_uvc_camera_info.ir_fx, m_uvc_camera_info.ir_fy, m_uvc_camera_info.ir_cx, m_uvc_camera_info.ir_cy, m_uvc_camera_info.rgb_fx, m_uvc_camera_info.rgb_fy, \
		m_uvc_camera_info.rgb_cx, m_uvc_camera_info.rgb_cy, m_uvc_camera_info.depth_precision);

	return 0;
}

int kd_uvc_dev_imp::_kd_uvc_transfer_file_data(char* serialNumber, unsigned char* pdata, int nlen, const char* dst_filepathname, UVC_TRANSFER_FILE_CB uvc_transfer_cb)
{
	FILE* pFile;
	int file_size;
	int ret = 0;
	unsigned char data_buf[MAX_TRANSFER_SIZE] = { 0 };
	unsigned char read_file_buf[MAX_TRANSFER_SIZE] = { 0 };
	int read_len;
	UVC_TRANSFER_FILE_INFO uvc_transfer_file_info;
	memset(&uvc_transfer_file_info, 0, sizeof(UVC_TRANSFER_FILE_INFO));

	//init uvc transfer with pid vid
	if (0 != kd_uvc_transfer_init(K230_UVC_DEVICE_VID, K230_UVC_DEVICE_PID, serialNumber,true))
	{
		return -1;
	}

	file_size = nlen;


	//send transfer start package
	memcpy(uvc_transfer_file_info.dst_filepathname, dst_filepathname, strlen(dst_filepathname));
	UVC_TRANSFER_DATA_HEAD_INFO transfer_head;

	memcpy(&transfer_head.data_start_code, UVC_TRANSFER_START_HEAD, strlen(UVC_TRANSFER_START_HEAD));
	transfer_head.tranfer_type = em_uvc_transfer_data_type_file;

	memcpy(data_buf, &transfer_head, sizeof(transfer_head));
	memcpy(data_buf + sizeof(transfer_head), &uvc_transfer_file_info, sizeof(uvc_transfer_file_info));
	ret = kd_uvc_transfer_data(data_buf, MAX_TRANSFER_PATH_FILE_NAME_SIZE + strlen(UVC_TRANSFER_START_HEAD));
	if (ret != 0)
	{
		printf("send transfer start package failed\n");
		return false;
	}
	if (uvc_transfer_cb != NULL)
	{
		uvc_transfer_cb(0, 0, em_uvc_transfer_file_start);
	}

	//send file data package
	int cnt = 0;
	int cur_transfer_size = 0;
	int transfer_cnt = file_size / MAX_TRANSFER_SIZE + (0 != file_size % MAX_TRANSFER_SIZE);
	int transfer_len = 0;
	for (int i =0; i < transfer_cnt;i ++)
	{
		if (i == transfer_cnt - 1)
		{
			transfer_len = file_size - MAX_TRANSFER_SIZE * (transfer_cnt - 1);
		}
		else
		{
			transfer_len = MAX_TRANSFER_SIZE;
		}
		memcpy(data_buf, pdata + MAX_TRANSFER_SIZE * i, transfer_len);

		ret = kd_uvc_transfer_data(data_buf, transfer_len);
		if (ret != 0)
		{
			printf("==kd_uvc_transfer_file failed\n");
		}

		
		cnt++;
		cur_transfer_size += transfer_len;
		if (cnt % 1000 == 0)
		{
			if (uvc_transfer_cb != NULL)
				uvc_transfer_cb(cur_transfer_size, file_size, em_uvc_transfer_file_during);
		}
	}

	//send transfer end package
	memset(data_buf, 0, sizeof(data_buf));
	memcpy(&transfer_head.data_start_code, UVC_TRANSFER_STOP_HEAD, strlen(UVC_TRANSFER_STOP_HEAD));
	transfer_head.tranfer_type = em_uvc_transfer_data_type_file;
	memcpy(data_buf, &transfer_head, sizeof(transfer_head));
	ret = kd_uvc_transfer_data(data_buf, sizeof(transfer_head));
	if (ret != 0)
	{
		printf("==kd_uvc_transfer_file failed\n");
	}

	if (uvc_transfer_cb != NULL)
		uvc_transfer_cb(cur_transfer_size, file_size, em_uvc_transfer_file_stop);

	//deinit uvc
	kd_uvc_transfer_deinit(true);

	return 0;
}

int kd_uvc_dev_imp::_kd_uvc_transfer_cfg_data(char* serialNumber, UVC_TRANSFER_FILE_CB uvc_transfer_cb)
{
	int ret;
	UVC_TRANSFER_DATA_HEAD_INFO transfer_head;

	//init uvc transfer with pid vid
	if (0 != kd_uvc_transfer_init(K230_UVC_DEVICE_VID, K230_UVC_DEVICE_PID, serialNumber,true))
	{
		return -1;
	}

	//transfer cfg start
	memcpy(&transfer_head.data_start_code, UVC_TRANSFER_START_HEAD, strlen(UVC_TRANSFER_START_HEAD));
	transfer_head.tranfer_type = em_uvc_transfer_data_type_cfg;
	ret = kd_uvc_transfer_data((unsigned char*)&transfer_head, sizeof(transfer_head));
	if (ret != 0)
	{
		printf("==kd_uvc_transfer_file failed\n");
	}

	int total_size = sizeof(m_uvc_grab_init_parameters);
	unsigned char* pcfg_data = (unsigned char*)&m_uvc_grab_init_parameters;
	int transfer_cnt = total_size / MAX_TRANSFER_SIZE + (0 != total_size % MAX_TRANSFER_SIZE);
	for (int i = 0; i < transfer_cnt; i++)
	{

		ret = kd_uvc_transfer_data(pcfg_data + MAX_TRANSFER_SIZE * i, (i == transfer_cnt - 1) ? (total_size - (transfer_cnt - 1) * MAX_TRANSFER_SIZE) : MAX_TRANSFER_SIZE);
		if (ret != 0)
		{
			printf("==kd_uvc_transfer_file failed\n");
		}

	}


	//transfer cfg stop
	memcpy(&transfer_head.data_start_code, UVC_TRANSFER_STOP_HEAD, strlen(UVC_TRANSFER_STOP_HEAD));
	transfer_head.tranfer_type = em_uvc_transfer_data_type_cfg;
	ret = kd_uvc_transfer_data((unsigned char*)&transfer_head, sizeof(transfer_head));
	if (ret != 0)
	{
		printf("==kd_uvc_transfer_file failed\n");
	}

	kd_uvc_transfer_deinit(true);
	return 0;
}

int  kd_uvc_dev_imp::_kd_uvc_transfer_control_cmd(char* serialNumber, UVC_TRANSFER_CONTROL_CMD cmd, bool usb_init)
{
	int ret;
	UVC_TRANSFER_DATA_HEAD_INFO transfer_head;

	//init uvc transfer with pid vid
	if (0 != kd_uvc_transfer_init(K230_UVC_DEVICE_VID, K230_UVC_DEVICE_PID, serialNumber,usb_init))
	{
		return -1;
	}

	memcpy(&transfer_head.data_start_code, UVC_TRANSFER_START_HEAD, strlen(UVC_TRANSFER_START_HEAD));
	transfer_head.tranfer_type = em_uvc_transfer_data_type_ctl;
	ret = kd_uvc_transfer_data((unsigned char*)&transfer_head, sizeof(transfer_head));
	if (ret != 0)
	{
		printf("==kd_uvc_transfer_file failed\n");
	}

	ret = kd_uvc_transfer_data((unsigned char*)&cmd, sizeof(cmd));
	if (ret != 0)
	{
		printf("==kd_uvc_transfer_file failed\n");
	}

	//transfer cfg stop
	memcpy(&transfer_head.data_start_code, UVC_TRANSFER_STOP_HEAD, strlen(UVC_TRANSFER_STOP_HEAD));
	transfer_head.tranfer_type = em_uvc_transfer_data_type_ctl;
	ret = kd_uvc_transfer_data((unsigned char*)&transfer_head, sizeof(transfer_head));
	if (ret != 0)
	{
		printf("==kd_uvc_transfer_file failed\n");
	}

	kd_uvc_transfer_deinit(usb_init);

	return 0;
}
