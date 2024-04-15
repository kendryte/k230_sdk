#include "pcm_data.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include<wav_ctrl.h>

static k_u32         *g_pcm_file_data = NULL;
static k_u32           g_pcm_file_len = 0;
static k_u32           g_pcm_file_index = 0;


static int g_bitpersample = 0;
static int g_wav_headlen = 0;
static char g_wav_filename[256];



static k_s32  _load_file(const char*filename)
{
    unsigned char *pcm_file_data = NULL;
    FILE *fp = fopen(filename, "rb");
    if (NULL == fp)
    {
        printf("open file:%s failed\n", filename);
        return -1;
    }

    fseek(fp, 0, SEEK_END);
    int size = ftell(fp);
    pcm_file_data = (unsigned char *)malloc(size);
    if (pcm_file_data == NULL)
    {
        printf("malloc size %d failed\n", size);
        return -1;
    }

    fseek(fp, 0, SEEK_SET);
    fread(pcm_file_data, size, 1, fp);
    fclose(fp);
    g_pcm_file_len = size - g_wav_headlen;

    g_pcm_file_data = (k_u32 *)(pcm_file_data + g_wav_headlen);

    printf("open file:%s ok,file size:%d,data size:%d,wav header size:%d\n", filename, size,g_pcm_file_len,(int)g_wav_headlen);

    return 0;
}


static k_s32    _get_bit_pcm_data_from_file(const char*filename,k_u32 *pdata, k_u32 data_len)
{
    if (g_pcm_file_index + data_len > g_pcm_file_len/4)
    {
        g_pcm_file_index = 0;
        
        printf("read file over \n");
        return -1;
        
    }

    for (unsigned int i = 0; i < data_len; i ++)
    {
        /*if (g_pcm_file_index == 0)
        {
            printf("========0x%x_0x%x_0x%x_0x%x_0x%x_0x%x_0x%x_0x%x_0x%x_0x%x\n",
            g_pcm_file_data[0],g_pcm_file_data[1],g_pcm_file_data[2],g_pcm_file_data[3],g_pcm_file_data[4],
            g_pcm_file_data[5],g_pcm_file_data[6],g_pcm_file_data[7],g_pcm_file_data[8],g_pcm_file_data[9]);
        }*/
        pdata[i] = g_pcm_file_data[g_pcm_file_index++];
        if (g_pcm_file_index >= g_pcm_file_len / 4)
        {
            g_pcm_file_index = 0;
            
            printf("read file over\n");
            return -1;
            
        }
    }
    return 0;
}


k_s32    load_wav_info(const char*filename,int* channel,int* samplerate,int* bitpersample)
{
    int header_len = 0;
    int data_len = 0;
    read_wav_header(filename,&header_len,&data_len,channel,samplerate,bitpersample);
    printf("========read_wav_header:headerlen:%d,channel:%d,samplerate:%d,bitpersample:%d\n",\
    header_len,*channel,*samplerate,*bitpersample);

    g_wav_headlen = header_len;
    g_bitpersample = *bitpersample;
    memcpy(g_wav_filename,filename,strlen(filename)+1);

    if (0 != _load_file(filename))
    {
        return -1;
    }
    return 0;
}

k_s32    get_pcm_data_from_file(k_u32 *pdata, k_u32 data_len)
{
    return _get_bit_pcm_data_from_file(g_wav_filename,pdata,data_len);
    return -1;
}