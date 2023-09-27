#include "stdio.h"
#include <rtthread.h>
#include <rthw.h>

extern int sfs_ipc_init(void *arg);
extern void sfs_ipc_deinit(void);
extern int sfs_ipc_connected(void);
extern int sfs_client_init(const char *mount_path);
extern void sfs_client_deinit(const char *mount_path);

int sharefs_client_init(const char *path)
{
	int ret;
	unsigned int id;
	rt_thread_t sharefs_thread;
	sharefs_thread = rt_thread_create("sharefs_client", sfs_ipc_init, RT_NULL, 0x6000, 5, 5);
	if(sharefs_thread == RT_NULL)
	{
		rt_kprintf("create sharefs thread failed\n");
	}

	ret = rt_thread_startup(sharefs_thread);
	if(ret)
	{
		rt_kprintf("sharefs thread startup failed\n");
	}

	return sfs_client_init(path);
}

void sharefs_client_deinit(const char *path)
{
	sfs_client_deinit(path);
	sfs_ipc_deinit();
}

int sharefs_client_connected(void)
{
	return sfs_ipc_connected();
}
