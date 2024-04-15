#include "kplayer.h"
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>
#include <signal.h>
#include <unistd.h>

static sem_t g_stop_player_sem;

static void sig_handler(int sig)
{
    printf("receive SIGINT \n");
    kd_player_stop();
}

static k_s32 player_event_cb(K_PLAYER_EVENT_E enEvent, void *pData)
{
    if (enEvent == K_PLAYER_EVENT_EOF)
    {
        sem_post(&g_stop_player_sem);
    }
    return 0;
}

#if 0
static void showpicture(const char* pic_path)
{
    kd_player_init(K_TRUE);
    kd_picture_init();
    kd_picture_show(pic_path);
    sleep(5);
    kd_picture_deinit();
    kd_player_deinit(K_TRUE);
}
#endif

int main(int argc, char *argv[])
{
#if 1
    signal(SIGINT, sig_handler);

    if (argc < 3) {
        printf("Usage: ./%s <*.mp4> <type(connect type,see vo doc)> \n",argv[0]);
        return 0;
    }

    printf("file pathname:%s\n",argv[1]);
    int connect_type = atoi(argv[2]);
    printf("connector type:%d\n",connect_type);

    sem_init(&g_stop_player_sem,0,0);
    kd_player_init(K_TRUE);

    kd_player_set_connector_type(connect_type);

    kd_player_regcallback(player_event_cb,NULL);

    kd_player_setdatasource(argv[1]);
    kd_player_start();
    sem_wait(&g_stop_player_sem);
    kd_player_stop();
    kd_player_deinit(K_TRUE);
    sem_destroy(&g_stop_player_sem);
#else
    if (argc < 2) {
        printf("Usage: ./%s <*.mp4> \n",argv[0]);
        return 0;
    }

    #if 1
    kd_player_init(K_TRUE);
    kd_picture_init();

    printf("==========show next \n");
    kd_picture_show("/clip/snapshot/stay_0.jpg");
    sleep(5);
    printf("==========show next \n");
    kd_picture_show("/clip/snapshot/stay_1.jpg");
    sleep(5);
    printf("==========show next \n");
    kd_picture_show("/clip/snapshot/stay_2.jpg");
    sleep(5);
    //kd_picture_show("/clip/snapshot/1.jpg");
    sleep(1000);
    #else

    showpicture("/clip/snapshot/stay_0.jpg");
    showpicture("/clip/snapshot/stay_1.jpg");
    showpicture("/clip/snapshot/stay_2.jpg");
    #endif
#endif

    return 0;
}
