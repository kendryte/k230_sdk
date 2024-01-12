#ifndef __KD_UVC_H__
#define __KD_UVC_H__
#include "libuvc/libuvc.h"
typedef void (*uvc_utils_device_cb)(uvc_frame_t *frame, void *ptr);
typedef struct UVC_UTILS_DEVICE_t
{
    int pid;
    int vid;
    char* serialNumber;
    int format;
    int targetFps;
    int frameWidth;
    int frameHeight;
    uvc_utils_device_cb    cb;
    void*                  user_ptr;
    uvc_context_t          *ctx;
    uvc_error_t            res;
    uvc_device_t           *dev;
    uvc_device_handle_t    *devh;
    uvc_stream_ctrl_t      ctrl;
}UVC_UTILS_DEVICE;
int kd_init_uvc_camera_device(UVC_UTILS_DEVICE * dev);
int kd_free_uvc_camera_device(UVC_UTILS_DEVICE * dev);
int kd_start_uvc_camera_device(UVC_UTILS_DEVICE * dev);
int kd_stop_uvc_camera_device(UVC_UTILS_DEVICE* dev);

int kd_uvc_transfer_init(int vid,int pid, char* serialNumber);
int kd_uvc_transfer_data(const unsigned char* data, int len);
int kd_uvc_transfer_deinit();
#endif //__KD_UVC_H__   