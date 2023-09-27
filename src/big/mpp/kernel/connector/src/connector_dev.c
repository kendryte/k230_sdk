/* Copyright (c) 2023, Canaan Bright Sight Co., Ltd
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 * 1. Redistributions of source code must retain the above copyright
 * notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 * notice, this list of conditions and the following disclaimer in the
 * documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND
 * CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES,
 * INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 * BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#ifdef RT_USING_POSIX
#include <dfs_posix.h>
#include <dfs_poll.h>
#include <posix_termios.h>
#endif

#include "connector_dev.h"
#include "k_connector_comm.h"
#include "sysctl_pwr.h"

struct connector_driver_dev *g_connector_drv[CONNECTOR_NUM_MAX];

static k_s32 connector_dev_open(struct dfs_fd *file)
{
    struct rt_device *device;
	struct connector_driver_dev *pdriver_dev;
	k_s32 ret = 0;

    //rt_kprintf("%s enter, device(%p)\n", __func__, file->fnode->data);

    device = file->fnode->data;
	if (device == NULL) {
		rt_kprintf("%s: device is null\n", __func__);
		return -ENOMEM;
	}
    pdriver_dev = (struct connector_driver_dev *)device;

    sysctl_pwr_up(SYSCTL_PD_DISP);

	//rt_kprintf("%s exit\n", __func__);

	return 0;
}


static k_s32 connector_dev_close(struct dfs_fd *file)
{
    struct rt_device *device;
	struct connector_driver_dev *pdriver_dev;
	k_s32 ret = 0;

    //rt_kprintf("%s enter, device(%p)\n", __func__, file->fnode->data);

    device = file->fnode->data;
	if (device == NULL) {
		rt_kprintf("%s: device is null\n", __func__);
		return -ENOMEM;
	}
    pdriver_dev = (struct connector_driver_dev *)device;

	//rt_kprintf("%s exit\n", __func__);

	return 0;
}


static k_s32 connector_dev_ioctl(struct dfs_fd *file, k_s32 cmd, void *args)
{
    struct rt_device *device;
	struct connector_driver_dev *pdriver_dev;
	k_s32 ret = 0;

    device = file->fnode->data;
	if (device == NULL) {
		rt_kprintf("%s: device is null\n", __func__);
		return -ENOMEM;
	}

    pdriver_dev = (struct connector_driver_dev *)device;

	rt_mutex_take(&pdriver_dev->connector_mutex, RT_WAITING_FOREVER);
	ret = connector_priv_ioctl(pdriver_dev, cmd, args);
    rt_mutex_release(&pdriver_dev->connector_mutex);

    return ret;
}

static const struct dfs_file_ops connector_dev_fops = {
    .open = connector_dev_open,
    .close = connector_dev_close,
    .ioctl = connector_dev_ioctl,
};


k_s32 connector_drv_dev_init(struct connector_driver_dev *pdriver_dev)
{
    struct rt_device *device;
    char dev_name[32];
    k_s32 ret = 0;

    RT_ASSERT(pdriver_dev != NULL)

    device = &pdriver_dev->parent;
    rt_snprintf(dev_name, sizeof(dev_name), "connector_%s", pdriver_dev->connector_name);

    rt_mutex_init(&pdriver_dev->connector_mutex, "connector_mutex", RT_IPC_FLAG_PRIO);

    ret = rt_device_register(device, dev_name, RT_DEVICE_FLAG_RDWR);
    if (ret) {
        rt_kprintf("connector device register fail\n");
        return ret;
    }

    device->fops = &connector_dev_fops;
    device->user_data = pdriver_dev;
    return 0;
}


k_s32 connector_device_init(void)
{
    connector_drv_list_init(connector_drv_list);

    for (k_u32 connector_id = 0; connector_id < CONNECTOR_NUM_MAX; connector_id++) {
        if (g_connector_drv[connector_id] != NULL)
            connector_drv_dev_init(g_connector_drv[connector_id]);
    }

    return 0;
}