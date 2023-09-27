#include <linux/ioctl.h>

#ifndef __HI_IPCM_USERDEV_H__
#define __HI_IPCM_USERDEV_H__

#define IPCM_MAX_DEV_NR			8

enum handle_state {
	HANDLE_DISCONNECTED,
	HANDLE_CONNECTING,
	HANDLE_CONNECTED
};

enum handle_priority {
	HANDLE_MSG_NORMAL,
	HANDLE_MSG_PRIORITY
};

struct ipcm_handle_attr {
	int target;
	int port;
	int priority;
	int remote_ids[IPCM_MAX_DEV_NR];
};

#define	HI_IOC_IPCM_BASE  'M'

/* Create a new ipcm handle. A file descriptor is only used*
 * once for one ipcm handle. */
#define HI_IPCM_IOC_CONNECT  \
	_IOW(HI_IOC_IPCM_BASE, 1, struct ipcm_handle_attr)
#define HI_IPCM_IOC_TRY_CONNECT  \
	_IOW(HI_IOC_IPCM_BASE, 2, struct ipcm_handle_attr)

#define HI_IPCM_IOC_CHECK  \
	_IOW(HI_IOC_IPCM_BASE, 3, unsigned long)
#define HI_IPCM_IOC_DISCONNECT  \
	_IOW(HI_IOC_IPCM_BASE, 4, unsigned long)
#define HI_IPCM_IOC_GET_LOCAL_ID \
	_IOW(HI_IOC_IPCM_BASE, 5, unsigned long)
#define HI_IPCM_IOC_GET_REMOTE_ID \
	_IOW(HI_IOC_IPCM_BASE, 6, struct ipcm_handle_attr)
#define HI_IPCM_IOC_GET_REMOTE_STS \
	_IOW(HI_IOC_IPCM_BASE, 7, unsigned long)

#define HI_IPCM_IOC_ATTR_INIT \
	_IOW(HI_IOC_IPCM_BASE, 8, struct ipcm_handle_attr)


#endif  /* __HI_IPCM_H__ */
