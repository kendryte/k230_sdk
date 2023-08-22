#include "kplayer.h"
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>
#include <signal.h>

static sem_t g_stop_player_sem;

static void sig_handler(int sig) {
    printf("receive SIGINT \n");
    kd_player_stop();
}

static  k_s32 player_event_cb(K_PLAYER_EVENT_E enEvent, void* pData)
{
    if (enEvent == K_PLAYER_EVENT_EOF)
    {
        sem_post(&g_stop_player_sem);
    }
}

int main(int argc, char *argv[])
{
    signal(SIGINT, sig_handler);

    if (argc < 2) {
        printf("Usage: ./%s <*.mp4> \n",argv[0]);
        return 0;
    }

    printf("file pathname:%s\n",argv[1]);
    sem_init(&g_stop_player_sem,0,0);
    kd_player_init();

    kd_player_regcallback(player_event_cb,NULL);

    kd_player_setdatasource(argv[1]);
    kd_player_start();
    sem_wait(&g_stop_player_sem);
    kd_player_stop();
    kd_player_deinit();
    sem_destroy(&g_stop_player_sem);

    return 0;
}