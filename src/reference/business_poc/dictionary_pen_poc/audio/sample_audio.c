#include "audio_buf_play.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define  WAV_HEAD_SIZE 44
static void test_play_audio_buffer()
{
    unsigned char *pcm_file_data = NULL;
    FILE *fp = fopen("audio.wav", "rb");
    if (NULL == fp)
    {
        printf("open file:%s failed\n", "audio.wav");
        return ;
    }

    fseek(fp, 0, SEEK_END);
    int size = ftell(fp);
    pcm_file_data = (unsigned char *)malloc(size);
    if (pcm_file_data == NULL)
    {
        printf("malloc size %d failed\n", size);
        return ;
    }

    fseek(fp, 0, SEEK_SET);
    fread(pcm_file_data, size, 1, fp);
    fclose(fp);


    audio_buffer_play(pcm_file_data+WAV_HEAD_SIZE,size-WAV_HEAD_SIZE);
}

int main(int argc, char *argv[])
{
    //init
    audio_buffer_sample_vb_init(K_TRUE,48000);
    audio_buffer_play_init(16000,1);

    //play
    for (int i =0;i < 3;i ++)
    {
        test_play_audio_buffer();
    }

    //deinit
    printf("deinit ...\n");
    sleep(1);
    audio_buffer_play_deinit();
    audio_buffer_sample_vb_destroy();


    return 0;
}