
#include <stdio.h>
#include <sys/mman.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <pthread.h>
#include <semaphore.h>
#include <string.h>
#include <time.h>

#define G711_DATA_SRC1  "1_8k.g711a"
#define G711_DATA_SRC2  "2_8k.g711a"
#define G711_DATA_MIX   "mix.g711a"
#include "g711_mix_audio.h"

static FILE *g_fp1 = NULL;
static FILE *g_fp2 = NULL;
static FILE *g_fp3 = NULL;
#define FILE_READ_SIZE  8000/25*2

static k_char g_g711_data1[FILE_READ_SIZE];
static k_char g_g711_data2[FILE_READ_SIZE];
static k_char g_g711_mix_data[FILE_READ_SIZE];
int main(int argc, char *argv[])
{
    int ret = 0;
    g_fp1 = fopen(G711_DATA_SRC1, "rb");
    if (NULL == g_fp1)
    {
        printf("open file:%s failed\n", G711_DATA_SRC1);
        return -1;
    }

    g_fp2 = fopen(G711_DATA_SRC2, "rb");
    if (NULL == g_fp2)
    {
        printf("open file:%s failed\n", G711_DATA_SRC2);
        return -1;
    }

    g_fp3 = fopen(G711_DATA_MIX, "ab");
    if (NULL == g_fp3)
    {
        printf("open file:%s failed\n", G711_DATA_SRC2);
        return -1;
    }

    while(1)
    {
        ret = fread(g_g711_data1, 1, FILE_READ_SIZE, g_fp1);
        if (ret != FILE_READ_SIZE)
        {
            printf("fread size error\n");
            break;
        }

        ret = fread(g_g711_data2, 1, FILE_READ_SIZE, g_fp2);
        if (ret != FILE_READ_SIZE)
        {
            printf("fread size error\n");
            break;
        }

        //ret = kd_mix_g711u_audio(g_g711_data1,g_g711_data2,FILE_READ_SIZE,g_g711_mix_data);
        ret = kd_mix_g711a_audio(g_g711_data1,g_g711_data2,FILE_READ_SIZE,g_g711_mix_data);
        //ret = kd_mix_g711a_audio(NULL,g_g711_data2,FILE_READ_SIZE,g_g711_mix_data);
        //ret = kd_mix_g711a_audio(g_g711_data1,NULL,FILE_READ_SIZE,g_g711_mix_data);
        if (ret != 0)
        {
            printf("mix data error\n");
            break;
        }

        fwrite(g_g711_mix_data,FILE_READ_SIZE,1,g_fp3);

    }

    fclose(g_fp1);
    fclose(g_fp2);
    fclose(g_fp3);

    printf("========sample done\n");

    return 0;
}