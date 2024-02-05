#include "uvc_recv_file.h"
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define MAX_TRANSFER_SIZE 60
#define MAX_TRANSFER_PATH_FILE_NAME_SIZE  52 //MAX_TRANSFER_SIZE - head size
static const char* UVC_TRANSFER_START_HEAD = "$$$0";
static const char* UVC_TRANSFER_STOP_HEAD = "$$$2";
typedef struct
{
	char dst_filepathname[MAX_TRANSFER_PATH_FILE_NAME_SIZE];
}UVC_TRANSFER_FILE_INFO;

static int g_fd = -1;
static UVC_TRANSFER_FILE_INFO g_transfer_info;
static UVC_TRANSFER_DATA_TYPE g_cur_transfer_type = em_uvc_transfer_data_type_unknown;
uvc_grab_init_parameters_ex g_grab_init_param;

static char g_cfg[1024];
static int  g_cur_cfg_len = 0;

extern int do_dpu_ctrol_cmd(UVC_TRANSFER_CONTROL_CMD cmd_ctrl);

static int _decompress_file(const char* pfilename)
{
    char*suffix_name = strrchr((char*)pfilename, '.');

    if (strlen(suffix_name) == 4)
    {
        if (0 == strncmp(suffix_name,".zip",4))
        {
            char*p = strrchr((char*)pfilename, '/');
            char filepath[256];
            memset(filepath,0,sizeof(filepath));
            strncpy(filepath, pfilename, (p - pfilename)+1);

            printf("decompress file:%s ...\n",pfilename);
            char cmd[256];
            snprintf(cmd,sizeof(cmd),"unzip -o %s -d %s",pfilename,filepath);
            int ret = system(cmd);
            if (0 == ret)
            {
                remove(pfilename);
                printf("%s ok\n",cmd);
                system("sync");
            }
            else
            {
                printf("%s failed\n",cmd);
            }

        }
    }
    return 0;
}

static int _update_uvc_serial_number()
{
    //update current gadget
    char sCmd[256];
    sprintf(sCmd,"echo %s > /sys/kernel/config/usb_gadget/g1/strings/0x409/serialnumber",g_grab_init_param.serialNumber);
    system(sCmd);
    printf(sCmd);
    printf("\n");

    //modify serial_number frome canaan-camera-dpu.sh
    sprintf(sCmd,"sed -i 's/SERIAL=.*/SERIAL=\"%s\"/g' /mnt/canaan-camera-dpu.sh",g_grab_init_param.serialNumber);
    system(sCmd);
    printf(sCmd);
    printf("\n");

    system("sync");
    return 0;
}

static int _do_cfg(char* pdata, int len)
{
    if (len != sizeof(g_grab_init_param))
    {
        printf("_do_cfg datasize:%d,uvc_grab_init_parameters_ex size:%d\n",len,sizeof(g_grab_init_param));
        return -1;
    }
    memcpy(&g_grab_init_param,pdata,sizeof(g_grab_init_param));

    uvc_grab_init_parameters_ex * grab_init_param = &g_grab_init_param;
	printf("grab param:mode:%d,sensor type0:%d,sensor type1:%d,adc_enable:%d,overwrite:%d,serial_num:%s, width:%d,height:%d,fps:%d,distance:%d,adc_enable:%d,t_ref:%f,t_cx:%f,t_cy:%f,kx:%.2f,ky:%.2f\n",
			grab_init_param->grab_mode, grab_init_param->sensor_type[0], grab_init_param->sensor_type[1], grab_init_param->adc_enable,
			grab_init_param->overwrite_file, grab_init_param->serialNumber,
			grab_init_param->camera_width, grab_init_param->camera_height, grab_init_param->camera_fps, grab_init_param->depth_maximum_distance, grab_init_param->adc_enable,
			grab_init_param->temperature.temperature_ref, grab_init_param->temperature.temperature_cx, grab_init_param->temperature.temperature_cy,
			grab_init_param->temperature.kxppt, grab_init_param->temperature.kyppt);


    //update serial_number
    _update_uvc_serial_number();

    return 0;
}

static int _do_cmd(char* pdata, int len)
{
    if (sizeof(UVC_TRANSFER_CONTROL_CMD) != len)
    {
        printf("receive uvc control cmd error size:%d(%d)\n",len,sizeof(UVC_TRANSFER_CONTROL_CMD));
        return -1;
    }

    UVC_TRANSFER_CONTROL_CMD ctl_cmd;
    memcpy(&ctl_cmd,pdata,len);

    do_dpu_ctrol_cmd(ctl_cmd);
    return 0;
}

int  recv_uvc_data(unsigned char* pdata,int data_len)
{
    UVC_TRANSFER_DATA_HEAD_INFO transfer_data_head;
    if (data_len >= sizeof(UVC_TRANSFER_DATA_HEAD_INFO))
    {
        memcpy(&transfer_data_head,pdata,sizeof(transfer_data_head));

        if (0 ==  strncmp((char*)&transfer_data_head.data_start_code,UVC_TRANSFER_START_HEAD,strlen(UVC_TRANSFER_START_HEAD)))
        {
            //start package:UVC_TRANSFER_HEAD + UVC_TRANSFER_FILE_INFO
            if (transfer_data_head.tranfer_type == em_uvc_transfer_data_type_file)
            {
                memset(&g_transfer_info,0,sizeof(g_transfer_info));
                memcpy(&g_transfer_info,pdata+sizeof(transfer_data_head),sizeof(g_transfer_info));

                //if the file exists, delete it
                remove(g_transfer_info.dst_filepathname);
                //open file
                g_fd = open(g_transfer_info.dst_filepathname,O_RDWR | O_CREAT|O_APPEND);
                printf("open file:%s\n",g_transfer_info.dst_filepathname);
                g_cur_transfer_type = em_uvc_transfer_data_type_file;
                return 0;
            }
            else if (transfer_data_head.tranfer_type == em_uvc_transfer_data_type_cfg)
            {
                g_cur_transfer_type = em_uvc_transfer_data_type_cfg;
                return 0;
            }
            else if (transfer_data_head.tranfer_type == em_uvc_transfer_data_type_ctl)
            {
                g_cur_transfer_type = em_uvc_transfer_data_type_ctl;
                return 0;
            }

        }
        else if (0 ==  strncmp((char*)&transfer_data_head.data_start_code,UVC_TRANSFER_STOP_HEAD,strlen(UVC_TRANSFER_STOP_HEAD)))
        {
            //stop package:UVC_TRANSFER_HEAD
            if (transfer_data_head.tranfer_type == em_uvc_transfer_data_type_file)
            {
                //close file
                close(g_fd);
                g_fd = -1;
                printf("save file:%s ok\n",g_transfer_info.dst_filepathname);

                //decompress file
                _decompress_file(g_transfer_info.dst_filepathname);
                g_cur_transfer_type = em_uvc_transfer_data_type_unknown;
                return 0;
            }
            else if (transfer_data_head.tranfer_type == em_uvc_transfer_data_type_cfg)
            {
                _do_cfg(g_cfg,g_cur_cfg_len);
                g_cur_transfer_type = em_uvc_transfer_data_type_unknown;
                g_cur_cfg_len = 0;
                return 0;
            }
            else if (transfer_data_head.tranfer_type == em_uvc_transfer_data_type_ctl)
            {
                _do_cmd(g_cfg,g_cur_cfg_len);
                g_cur_transfer_type = em_uvc_transfer_data_type_unknown;
                g_cur_cfg_len = 0;
                return 0;
            }
        }
    }

    //get transfer data
    if (g_cur_transfer_type == em_uvc_transfer_data_type_cfg)
    {
        memcpy(g_cfg+g_cur_cfg_len,pdata,data_len);
        g_cur_cfg_len += data_len;
    }
    else if (g_cur_transfer_type == em_uvc_transfer_data_type_file)
    {
        //write file
        if (g_fd != -1)
        {
            write(g_fd,pdata,data_len);
        }
    }
    else if (g_cur_transfer_type == em_uvc_transfer_data_type_ctl)
    {
        memcpy(g_cfg+g_cur_cfg_len,pdata,data_len);
        g_cur_cfg_len += data_len;
    }

    return 0;
}