/* Copyright (c) 2023, Canaan Bright Sight Co., Ltd
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 * 1. Redistributions of source code must retain the above copyright
 * notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 * notice, this list of conditions and the following disclaimer in the
 * documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND
 * CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES,
 * INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 * BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */
#ifndef __UVC_RECV_FILE_H__
#define __UVC_RECV_FILE_H__

#include <stdbool.h>
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
	int  reverse[10];
	int dma_ro; //bit0~bit7: 0: DEGREE_0, 1: DEGREE_90, 2: DEGREE_180, 3: DEGREE_270
	            //bit8~bit15: adc channel number
	//from file
	char serialNumber[64];
	k_dpu_temperature temperature;
}uvc_grab_init_parameters_ex;

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
	unsigned long long sync_clock;
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

int  recv_uvc_data(unsigned char* pdata,int data_len);

#endif //__UVC_RECV_FILE_H__

