#include "msg_proc.h"
#include "ui_common.h"
// #include <sys/fcntl.h>
// #include <sys/stat.h>
// #include <sys/types.h>
// #include <chrono>
#include "k_ipcmsg.h"
// #include "rtsp_client.h"
// #include "media.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum
{
    INTERCOM_STATUS_INITED,
    INTERCOM_STATUS_SUSPEND,
    INTERCOM_STATUS_RUNNING,
    INTERCOM_STATUS_IDLE
} intercom_status_t;

typedef enum
{
    VOICE_STATUS_ORIGIN,
    VOICE_STATUS_CHANGED,
    VOICE_STATUS_BUTT
} voice_status_t;

k_s32 intercom_init(char *url);
k_s32 intercom_start(char *url);
k_s32 intercom_suspend();
k_s32 intercom_recover(char *url);
k_s32 intercom_enable_pitch_shift();
k_s32 intercom_disable_pitch_shift();
k_s32 intercom_save_control();

#ifdef __cplusplus
}
#endif