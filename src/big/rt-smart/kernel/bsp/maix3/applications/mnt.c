#include <rtthread.h>

#ifdef RT_USING_DFS
#include <dfs_fs.h>
#include <dfs_romfs.h>

int mnt_init(void)
{
    rt_err_t ret;

    if (dfs_mount(RT_NULL, "/", "rom", 0, &romfs_root) != 0)
    {
        rt_kprintf("Dir / mount failed!\n");
        return -1;
    }

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

