#include <rthw.h>
#include <rtthread.h>
#include <rtdevice.h>

#ifdef RT_USING_DFS
#include <dfs_fs.h>
#endif

int __attribute__((weak)) sharefs_client_init(const char *path)
{
    rt_kprintf("no share fs library!\n");
    return RT_EOK;
}

extern int sharefs_client_init(const char *path);
rt_int32_t sharefs_module_init(void)
{
    return sharefs_client_init("/sharefs");
}

INIT_APP_EXPORT(sharefs_module_init);