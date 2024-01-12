#include "uvc_recv_file.h"
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include<stdlib.h>
#include <string.h>
#include <unistd.h>

#define MAX_TRANSFER_SIZE 60
#define MAX_TRANSFER_PATH_FILE_NAME_SIZE  56
static const char* UVC_TRANSFER_START_HEAD = "$$$0";
//static const char* UVC_TRANSFER_DURING_HEAD = "$$$1";
static const char* UVC_TRANSFER_STOP_HEAD = "$$$2";
typedef struct
{
	char dst_filepathname[MAX_TRANSFER_PATH_FILE_NAME_SIZE];
}UVC_TRANSFER_INFO;

static int g_fd = -1;
static UVC_TRANSFER_INFO g_transfer_info;

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

int  recv_uvc_data(unsigned char* pdata,int data_len)
{
    //start package:UVC_TRANSFER_START_HEAD + UVC_TRANSFER_INFO
    if (0 == strncmp(pdata,UVC_TRANSFER_START_HEAD,strlen(UVC_TRANSFER_START_HEAD)))
    {
        memset(&g_transfer_info,0,sizeof(g_transfer_info));
        memcpy(&g_transfer_info,pdata+strlen(UVC_TRANSFER_START_HEAD),sizeof(g_transfer_info));

        //if the file exists, delete it
        remove(g_transfer_info.dst_filepathname);
        //open file
        g_fd = open(g_transfer_info.dst_filepathname,O_RDWR | O_CREAT|O_APPEND);
        printf("open file:%s\n",g_transfer_info.dst_filepathname);
    }
    //stop package:UVC_TRANSFER_STOP_HEAD
    else if (0 == strncmp(pdata,UVC_TRANSFER_STOP_HEAD,strlen(UVC_TRANSFER_STOP_HEAD)))
    {
        //close file
        close(g_fd);
        g_fd = -1;
        printf("save file:%s ok\n",g_transfer_info.dst_filepathname);

        //decompress file
        _decompress_file(g_transfer_info.dst_filepathname);
    }
    //file data package:max size MAX_TRANSFER_SIZE
    else
    {
        //write file
        if (g_fd != -1)
        {
            write(g_fd,pdata,data_len);
        }
    }
    return 0;
}