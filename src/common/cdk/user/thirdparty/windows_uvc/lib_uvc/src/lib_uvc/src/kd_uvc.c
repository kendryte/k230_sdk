#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include "kd_uvc.h"
#include "libusb.h"
#define CHECK_0_255(V) ( (V>255) ? (255) : (V<0 ? 0 : V ) )
int kd_init_uvc_camera_device(UVC_UTILS_DEVICE * dev)
{
    dev->res = uvc_init(&dev->ctx, NULL); 
    if (dev->res < 0)
    {
        printf("uvc_init failed!\n");
        return -1;
    }
   
    dev->res = uvc_find_device(dev->ctx, &dev->dev, dev->vid, dev->pid,  dev->serialNumber);
    if (dev->res < 0)
    {
        printf("uvc_find_device failed!\n");
        return -1;
    }

    dev->res = uvc_open(dev->dev, &dev->devh);
    if (dev->res < 0)
    {
        printf("uvc_open failed!\n");
        return -1;
    }
    dev->res = uvc_get_stream_ctrl_format_size(dev->devh, &dev->ctrl, dev->format, dev->frameWidth, dev->frameHeight, dev->targetFps );
    if (dev->res < 0)
    {
        printf("uvc_get_stream_ctrl_format_size failed!\n");
        return -1;
    }
        
    printf("init_uvc_camera_device ok!\n");
    return 0;
}
int kd_free_uvc_camera_device(UVC_UTILS_DEVICE * dev)
{
    //uvc_stop_streaming(dev->devh);
    uvc_close(dev->devh);
    uvc_unref_device(dev->dev);
    uvc_exit(dev->ctx);
    printf("free_uvc_camera_device ok!\n");
    return 0;
}
int kd_start_uvc_camera_device(UVC_UTILS_DEVICE * dev)
{
    dev->res = uvc_start_streaming(dev->devh, &dev->ctrl, dev->cb, dev->user_ptr, 0);
    if(dev->res < 0)
    {
        printf("start_uvc_camera_device failed!\n");
        return -1;
    }

    printf("start_uvc_camera_device ok!\n");
    return 0;
}

int kd_stop_uvc_camera_device(UVC_UTILS_DEVICE* dev)
{
	uvc_stop_streaming(dev->devh);
    printf("stop_uvc_camera_device ok!\n");
    return 0;
}


static const int K_UVC_MAX_REQUEST_SIZE = 60;
static const int K_UVC_SET_CUR = 0x01;
static const int K_UVC_VS_PROBE_CONTROL = 0x01;
static const int K_UVC_REQUEST_TYPE = 0x21;
static libusb_device_handle* g_uvc_send_data_hdev = NULL;
static uint8_t g_interfaceNumber = 0x1;//0x1:video stream   0x00:video control

int kd_uvc_transfer_data(const unsigned char* data, int len)
{
	int ret;

	if (len > K_UVC_MAX_REQUEST_SIZE)
	{
		printf("kd_uvc_transfer_data error,buffer size not valid\n");
		return -1;
	}

	/* do the transfer */
	ret = libusb_control_transfer(
        g_uvc_send_data_hdev,
		K_UVC_REQUEST_TYPE,
		K_UVC_SET_CUR,
		K_UVC_VS_PROBE_CONTROL << 8,
        g_interfaceNumber,
		data, len, 0
	);

	if (ret != len)
	{
		printf("libusb_control_transfer error %s", libusb_strerror(ret));
	}

	return 0;
}

static libusb_device_handle* _uvc_find_device_by_serialNumber(int vendor_id, int product_id, const unsigned char* sn)
{
	struct libusb_device** devs;
	struct libusb_device* found = NULL;
	struct libusb_device* dev;
	struct libusb_device_handle* dev_handle = NULL;
	size_t i = 0;
	int r;
	unsigned char buf[64];

	if (libusb_get_device_list(NULL, &devs) < 0)
		return NULL;

	while ((dev = devs[i++]) != NULL) {
		struct libusb_device_descriptor desc;
		r = libusb_get_device_descriptor(dev, &desc);
		if (r < 0)
			goto out;

		found = dev;

		if (desc.idVendor == vendor_id && desc.idProduct == product_id) 
		{
			r = libusb_open(found, &dev_handle);
			if (r < 0)
				continue;

			if (sn == NULL)
			{
				break;//get first uvc device
			}

			int bytes = libusb_get_string_descriptor_ascii(
				dev_handle, desc.iSerialNumber, buf, sizeof(buf));

			if (bytes > 0)
			{
				if (0 == strcmp(buf, sn))
				{
					break; // not close handle
				}
			}

			libusb_close(dev_handle);
		}
	}
	
out:
	libusb_free_device_list(devs, 1);
	return dev_handle;

}


int kd_uvc_transfer_init(int vid, int pid, char* serialNumber,bool usb_init)
{
    int status;
	
	if (usb_init)
	{
		/* open the device using libusb */
		status = libusb_init_context(NULL, NULL, 0);
		if (status < 0) {
			printf("libusb_init() failed: %s\n", libusb_error_name(status));
			return -1;
		}
	}

#if 0
	g_uvc_send_data_hdev = libusb_open_device_with_vid_pid(NULL, vid, pid);
	if (g_uvc_send_data_hdev == NULL) {
		printf("libusb_open() failed\n");
		return -1;
	}
#else

	g_uvc_send_data_hdev = _uvc_find_device_by_serialNumber(vid, pid, serialNumber);
	if (g_uvc_send_data_hdev == NULL) {
		printf("libusb_open() failed\n");
		return -1;
	}
#endif 

	/* We need to claim the first interface */
	libusb_set_auto_detach_kernel_driver(g_uvc_send_data_hdev, 1);
	status = libusb_claim_interface(g_uvc_send_data_hdev, g_interfaceNumber);
	if (status != LIBUSB_SUCCESS) {
		libusb_close(g_uvc_send_data_hdev);
		printf("libusb_claim_interface failed: %s\n", libusb_error_name(status));
		return -1;
	}

	return 0;
}

int kd_uvc_transfer_deinit(bool usb_init)
{
	if (usb_init)
	{
		libusb_release_interface(g_uvc_send_data_hdev, g_interfaceNumber);
	}
	
	libusb_close(g_uvc_send_data_hdev);
	libusb_exit(NULL);

    return 0;
}