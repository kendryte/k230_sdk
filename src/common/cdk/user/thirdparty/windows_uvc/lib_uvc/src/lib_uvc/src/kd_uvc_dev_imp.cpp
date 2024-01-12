#include "kd_uvc_dev_imp.h"
#include <iostream>
#include<thread>
using namespace std;
#include <windows.h>
#include <stdio.h>
#include "calib_bin_interface.h"

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

kd_uvc_dev_imp::~kd_uvc_dev_imp()
{
	
}

kd_uvc_dev_imp::kd_uvc_dev_imp()
{
	m_bstart_uvc_stream = false;

}

bool kd_uvc_dev_imp::kd_uvc_init(const grabber_init_param& pInitParam)
{
	int ret;

	m_uvc_param = pInitParam;
	m_uvc_device.format = pInitParam.format;//UVC_FRAME_FORMAT_NV12
	m_uvc_device.frameWidth = pInitParam.frame_width;
	m_uvc_device.frameHeight = pInitParam.frame_height;
	m_uvc_device.targetFps = pInitParam.fps;
	m_uvc_device.pid = pInitParam.uvc_device_pid;
	m_uvc_device.vid = pInitParam.uvc_device_vid;
	m_uvc_device.serialNumber = pInitParam.serialNumber;
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

bool kd_uvc_dev_imp::kd_uvc_transfer_file(char* serialNumber, const char* src_filepathname, const char* dst_filepathname, UVC_TRANSFER_FILE_CB uvc_transfer_cb)
{
	FILE* pFile;
	int file_size;
	int ret = 0;
	unsigned char data_buf[MAX_TRANSFER_SIZE] = { 0 };
	unsigned char read_file_buf[MAX_TRANSFER_SIZE] = { 0 };
	int read_len;
	UVC_TRANSFER_INFO uvc_transfer_info;
	memset(&uvc_transfer_info, 0, sizeof(UVC_TRANSFER_INFO));

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
	if (0 != kd_uvc_transfer_init(K230_UVC_DEVICE_VID, K230_UVC_DEVICE_PID, serialNumber))
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
	memcpy(uvc_transfer_info.dst_filepathname, dst_filepathname, strlen(dst_filepathname));
	memcpy(data_buf, UVC_TRANSFER_START_HEAD, strlen(UVC_TRANSFER_START_HEAD));
	memcpy(data_buf + strlen(UVC_TRANSFER_START_HEAD), &uvc_transfer_info, sizeof(uvc_transfer_info));
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
	memcpy(data_buf, UVC_TRANSFER_STOP_HEAD, strlen(UVC_TRANSFER_STOP_HEAD));
	ret = kd_uvc_transfer_data(data_buf, strlen(UVC_TRANSFER_STOP_HEAD));
	if (ret != 0)
	{
		printf("==kd_uvc_transfer_file failed\n");
	}

	if (uvc_transfer_cb != NULL)
		uvc_transfer_cb(cur_transfer_size, file_size, em_uvc_transfer_file_stop);

	//deinit uvc
	kd_uvc_transfer_deinit();

	return true;
}


void kd_uvc_dev_imp::kd_uvc_test()
{
	FILE* pFile;
	int file_size = 0;
	//open src file
	const char* src_filepathname = "C:/clip/H1280W720.bin";
	pFile = fopen(src_filepathname, "rb");
	if (pFile == nullptr)
	{
		printf("open file:%s failed\n", src_filepathname);
		return ;
	}

	//get file size
	fseek(pFile, 0, SEEK_END);
	file_size = ftell(pFile);
	printf("file:%s size:%d Bytes\n", src_filepathname, file_size);
	rewind(pFile);


	char* pfiledata = new char[file_size];
	int data_index = 0;
	fread(pfiledata, 1, file_size,pFile);
	fclose(pFile);

	for (int i = 0; i < file_size; i++)
	{
		pfiledata[i] = decrypt_(pfiledata[i]);
	}
	

	calib_data_file data_file_info;
	data_index = sizeof(data_file_info.param_bin_name) + sizeof(data_file_info.info) + sizeof(data_file_info.ref_size_);
	memcpy(&data_file_info, pfiledata, data_index);
	printf("=====param_bin_name:%s\n", data_file_info.param_bin_name);
	printf("=====ref_size_:%d\n", data_file_info.ref_size_);

	printf("===temperature_ref:%.10f,temperature_cx:%10f,temperature_cy:%.10f,kxppt:%.10f,kyppt:%.10f\n", data_file_info.info.temperature_ref, data_file_info.info.temperature_cx, data_file_info.info.temperature_cy, data_file_info.info.kxppt, data_file_info.info.kyppt);

	char* P_ref_ = pfiledata + data_index;
	int   P_ref_len = data_file_info.ref_size_;
	//save ref
	{
		FILE* fp = fopen("./a.bin", "wb");
		fwrite(P_ref_, P_ref_len, 1, fp);
		fclose(fp);
	}

	char* P_bin_params = pfiledata + data_index + data_file_info.ref_size_;
	int   P_bin_params_len = file_size - data_index - P_ref_len;

	printf("=======P_ref_len:%d,P_bin_params_len:%d\n", P_ref_len, P_bin_params_len);
	//save bin_params
	{
		FILE* fp = fopen("./b.bin", "wb");
		fwrite(P_bin_params, P_bin_params_len, 1, fp);
		fclose(fp);
	}


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
	if (uvc_data_head->data_start_code == g_uvc_rgb_data_start_code)
	{
		if (1 == uvc_data_head->data_type)
		{
			m_yuv_cur_index = 0;
			if (m_yuv_cur_index + uvc_data_head->data_size > uvc_data_head->frame_size)
			{
				printf("====[%d]error yuv frame size:data_type:%d,cur_index:%d,data_size:%d,frame_size:%d\n", m_yuv_err_cnt++, uvc_data_head->data_type, m_yuv_cur_index, uvc_data_head->data_size, uvc_data_head->frame_size);
			}
			else
			{
				memcpy(m_yuv_pic_data + m_yuv_cur_index, frame_data, uvc_data_head->data_size);
				m_yuv_cur_index += uvc_data_head->data_size;
			}

		}
		else if (2 == uvc_data_head->data_type)
		{
			if (m_yuv_cur_index + uvc_data_head->data_size > uvc_data_head->frame_size)
			{
				printf("====[%d]error yuv frame size:data_type:%d,cur_index:%d,data_size:%d,frame_size:%d\n", m_yuv_err_cnt++, uvc_data_head->data_type, m_yuv_cur_index, uvc_data_head->data_size, uvc_data_head->frame_size);
			}
			else
			{
				memcpy(m_yuv_pic_data + m_yuv_cur_index, frame_data, uvc_data_head->data_size);
				m_yuv_cur_index += uvc_data_head->data_size;
			}
		}
		else if (3 == uvc_data_head->data_type)
		{
			if (m_yuv_cur_index + uvc_data_head->data_size > uvc_data_head->frame_size)
			{
				printf("====[%d]error yuv frame size:data_type:%d,cur_index:%d,data_size:%d,frame_size:%d\n", m_yuv_err_cnt++, uvc_data_head->data_type, m_yuv_cur_index, uvc_data_head->data_size, uvc_data_head->frame_size);
			}
			else if (m_yuv_cur_index + uvc_data_head->data_size == uvc_data_head->frame_size)
			{
				memcpy(m_yuv_pic_data + m_yuv_cur_index, frame_data, uvc_data_head->data_size);
				m_yuv_cur_index += uvc_data_head->data_size;
				if (m_uvc_param.uvc_data_func != nullptr)
				{
					if (m_uvc_ctrl_status == em_uvc_control_snap_start)
					{
						m_uvc_cache_frame.cache_frame(em_uvc_cache_frame_type_yuv, uvc_data_head->frame_number, uvc_data_head->pts, m_yuv_pic_data, m_yuv_cur_index);
						_do_snap_match_frame();
					}
					else
					{
						m_uvc_param.uvc_data_func(KD_UVC_RGB_DATA_TYPE, uvc_data_head->frame_number, uvc_data_head->pts, m_yuv_pic_data, m_yuv_cur_index, m_uvc_param.uvc_data_ptr);
					}
				}
			}
			m_yuv_cur_index = 0;
		}
	}
	else if (uvc_data_head->data_start_code == g_uvc_depth_data_start_code)
	{
		if (1 == uvc_data_head->data_type)
		{
			m_depth_cur_index = 0;
			if (m_depth_cur_index + uvc_data_head->data_size > uvc_data_head->frame_size)
			{
				printf("====[%d]error depth frame size:data_type:%d,cur_index:%d,data_size:%d,frame_size:%d\n", m_depth_err_cnt++, uvc_data_head->data_type, m_depth_cur_index, uvc_data_head->data_size, uvc_data_head->frame_size);
			}
			else
			{
				memcpy(m_depth_pic_data + m_depth_cur_index, frame_data, uvc_data_head->data_size);
				m_depth_cur_index += uvc_data_head->data_size;
			}
		}
		else if (2 == uvc_data_head->data_type)
		{
			if (m_depth_cur_index + uvc_data_head->data_size > uvc_data_head->frame_size)
			{
				printf("====[%d]error depth frame size:data_type:%d,cur_index:%d,data_size:%d,frame_size:%d\n", m_depth_err_cnt++, uvc_data_head->data_type, m_depth_cur_index, uvc_data_head->data_size, uvc_data_head->frame_size);
			}
			else
			{
				memcpy(m_depth_pic_data + m_depth_cur_index, frame_data, uvc_data_head->data_size);
				m_depth_cur_index += uvc_data_head->data_size;
			}

		}
		else if (3 == uvc_data_head->data_type)
		{
			if (m_depth_cur_index + uvc_data_head->data_size > uvc_data_head->frame_size)
			{
				printf("====[%d]error depth frame size:data_type:%d,cur_index:%d,data_size:%d,frame_size:%d\n", m_depth_err_cnt++, uvc_data_head->data_type, m_depth_cur_index, uvc_data_head->data_size, uvc_data_head->frame_size);
			}
			else if (m_depth_cur_index + uvc_data_head->data_size == uvc_data_head->frame_size)
			{
				memcpy(m_depth_pic_data + m_depth_cur_index, frame_data, uvc_data_head->data_size);
				m_depth_cur_index += uvc_data_head->data_size;
				if (m_uvc_param.uvc_data_func != nullptr)
				{
					if (m_uvc_ctrl_status == em_uvc_control_snap_start)
					{
						m_uvc_cache_frame.cache_frame(em_uvc_cache_frame_type_depth, uvc_data_head->frame_number, uvc_data_head->pts, m_depth_pic_data, m_depth_cur_index);
						_do_snap_match_frame();
					}
					else
					{
						m_uvc_param.uvc_data_func(KD_UVC_DEPTH_DATA_TYPE, uvc_data_head->frame_number, uvc_data_head->pts, m_depth_pic_data, m_depth_cur_index, m_uvc_param.uvc_data_ptr);
					}
				}
			}
			m_depth_cur_index = 0;
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

int kd_uvc_dev_imp::_do_snap_match_frame()
{
	if (0 == m_uvc_cache_frame.select_match_frames(m_yuv_frame_info, m_depth_frame_info))
	{
		if (m_uvc_param.uvc_data_func != nullptr)
		{
			m_uvc_param.uvc_data_func(KD_UVC_DEPTH_DATA_TYPE, m_depth_frame_info.frame_number, m_depth_frame_info.pts, m_depth_frame_info.m_cache_pic_data,m_depth_frame_info.data_size, m_uvc_param.uvc_data_ptr);
			m_uvc_param.uvc_data_func(KD_UVC_RGB_DATA_TYPE, m_yuv_frame_info.frame_number, m_yuv_frame_info.pts, m_yuv_frame_info.m_cache_pic_data, m_yuv_frame_info.data_size, m_uvc_param.uvc_data_ptr);
		}
		m_uvc_cache_frame.reset();

		m_cond.notify_all();
		m_uvc_ctrl_status == em_uvc_control_snap_stop;
	}
	return -1;
}
