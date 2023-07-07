#include <rtthread.h>

#ifdef RT_USING_DFS
#include <dfs_fs.h>
#include <dfs_romfs.h>

#define RT_SDCARD_MOUNT_STACK_SIZE 2048
static struct rt_thread sdcard_mount_thread;
static rt_uint8_t sdcard_mount_thread_stack[RT_SDCARD_MOUNT_STACK_SIZE];

void sdcard_mount(void *param)
{
    rt_thread_mdelay(1000);
    if (dfs_mount("sd", "/sdcard", "elm", 0, 0) != 0)
    {
        rt_kprintf("Dir /sdcard mount failed!\n");
    }
}
int mnt_init(void)
{
    rt_err_t ret;

    if (dfs_mount(RT_NULL, "/", "rom", 0, &romfs_root) != 0)
    {
        rt_kprintf("Dir / mount failed!\n");
        return -1;
    }

#ifdef RT_USING_SDIO
    ret = rt_thread_init(&sdcard_mount_thread, "sdcard_mount", sdcard_mount, RT_NULL,
                         &sdcard_mount_thread_stack[0], RT_SDCARD_MOUNT_STACK_SIZE, 0x16, 20);
    if (ret == RT_EOK)
    {
        rt_thread_startup(&sdcard_mount_thread);
    }
#endif
    mkdir("/dev/shm", 0x777);
    if (dfs_mount(RT_NULL, "/dev/shm", "tmp", 0, 0) != 0) {
        rt_kprintf("Dir /dev/shm mount failed!\n");
    }
#ifndef RT_FASTBOOT
    rt_kprintf("/dev/shm file system initialization done!\n");
#endif
    return 0;
}
INIT_ENV_EXPORT(mnt_init);
#endif

