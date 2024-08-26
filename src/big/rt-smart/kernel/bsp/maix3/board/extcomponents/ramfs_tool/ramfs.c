#include <rtthread.h>
#include <stdlib.h>
#include <stdint.h>
#include <dfs_fs.h>
#include <dfs_ramfs.h>

#define RAMFS_COUNT_MAX 4

static struct ramfs_entry {
    char* path;
    uint8_t* addr;
    struct dfs_ramfs* ramfs;
    uint32_t size;
    uint8_t f_rmdir;
} ramfs_array[RAMFS_COUNT_MAX];

static uint32_t ramfs_count = 0;

static void usage(void)
{
    rt_kprintf("mount: ramfs_tool <mountpoint> <size>\n");
    rt_kprintf("unmount: ramfs_tool <mountpoint>\n");
}

static int find_by_path(char* path)
{
    for (int i = 0; i < RAMFS_COUNT_MAX; i++) {
        if (ramfs_array[i].path && (rt_strcmp(ramfs_array[i].path, path) == 0))
            return i;
    }
    return -1;
}

static int add_entry(struct ramfs_entry* entry)
{
    int idx = -1;

    for (int i = 0; i < RAMFS_COUNT_MAX; i++) {
        if (ramfs_array[i].path == 0) {
            idx = i;
            break;
        }
    }
    if (idx < 0)
        return -1;

    ramfs_array[idx] = *entry;
    ramfs_count++;
    return 0;
}

static int del_entry(char* path)
{
    int idx = find_by_path(path);
    if (idx < 0)
        return -1;

    rt_memheap_detach(&ramfs_array[idx].ramfs->memheap);
    if (ramfs_array[idx].f_rmdir)
        rmdir(ramfs_array[idx].path);
    rt_free(ramfs_array[idx].path);
    ramfs_array[idx].path = NULL;
    rt_free(ramfs_array[idx].addr);
    ramfs_array[idx].addr = 0;
    ramfs_count--;
    return 0;
}

static int ramfs_mount(char* path, uint32_t size)
{
    if (ramfs_count >= RAMFS_COUNT_MAX) {
        rt_kprintf("ramfs mount number is too many\n");
        return -ENOSPC;
    }
    path = dfs_normalize_path(NULL, path);
    if (path == NULL) {
        rt_kprintf("Invalid path\n");
        return -ENOTDIR;
    }
    if (find_by_path(path) >= 0) {
        rt_free(path);
        rt_kprintf("This path has been mounted\n");
        return -EEXIST;
    }
    if (size == 0) {
        rt_free(path);
        rt_kprintf("This size is invalid\n");
        return -EINVAL;
    }
    uint8_t* buf = rt_malloc(size);
    if (buf == NULL) {
        rt_free(path);
        rt_kprintf("ramfs malloc fail\n");
        return -ENOMEM;
    }
    struct dfs_ramfs* ramfs = dfs_ramfs_create(buf, size);
    if (ramfs == NULL) {
        rt_free(buf);
        rt_free(path);
        rt_kprintf("ramfs create fail\n");
        return -ENOMEM;
    }
    int fd = open(path, O_RDONLY | O_DIRECTORY);
    int f_rmdir = 0;
    if (fd < 0) {
        mkdir(path, 0);
        f_rmdir = 1;
    } else {
        close(fd);
    }
    if (dfs_mount(RT_NULL, path, "ram", 0, ramfs) != 0) {
        rt_free(buf);
        rt_free(path);
        rt_kprintf("ramfs mount fail\n");
        return -EPERM;
    }
    struct ramfs_entry entry;
    entry.path = path;
    entry.addr = buf;
    entry.ramfs = ramfs;
    entry.size = size;
    entry.f_rmdir = f_rmdir;
    add_entry(&entry);
    return 0;
}

static int ramfs_unmount(char* path)
{
    path = dfs_normalize_path(NULL, path);
    if (path == NULL) {
        rt_kprintf("Invalid path\n");
        return -ENOTDIR;
    }
    if (find_by_path(path) < 0) {
        rt_free(path);
        rt_kprintf("This path is not mounted\n");
        return -EINVAL;
    }
    if (dfs_unmount(path) != 0) {
        rt_free(path);
        rt_kprintf("ramfs unmount fail\n");
        return -EPERM;
    }
    del_entry(path);
    rt_free(path);
    return 0;
}

int ramfs_tool(int argc, char** argv)
{
    char* path;

    if (argc == 3) {
        path = argv[1];
        uint32_t size = strtoul(argv[2], 0, 0);
        return ramfs_mount(path, size);
    } else if (argc == 2) {
        path = argv[1];
        return ramfs_unmount(path);
    } else {
        usage();
    }
    return 0;
}
MSH_CMD_EXPORT(ramfs_tool, ramfs tool);

#define	RAMFS_CTL_MOUNT     _IOW('R', 0, int)
#define	RAMFS_CTL_UNMOUNT   _IOW('R', 1, int)

struct ramfs_ctl_args {
    char path[64];
    uint32_t size;
};

static struct rt_device ramfs_dev;

static rt_err_t ramfs_open(rt_device_t dev, rt_uint16_t oflag)
{
    return 0;
}

static rt_err_t ramfs_close(rt_device_t dev)
{
    return 0;
}

static rt_err_t ramfs_control(rt_device_t dev, int cmd, void *args)
{
    int ret;
    struct ramfs_ctl_args ctl;

    lwp_get_from_user(&ctl, args, sizeof(ctl));
    if (cmd == RAMFS_CTL_MOUNT) {
        ret = ramfs_mount(ctl.path, ctl.size);
    } else if (cmd == RAMFS_CTL_UNMOUNT) {
        ret = ramfs_unmount(ctl.path);
    } else {
        ret = -EINVAL;
    }
    return ret;
}

static const struct rt_device_ops ramfs_ops = {
    RT_NULL,
    ramfs_open,
    ramfs_close,
    RT_NULL,
    RT_NULL,
    ramfs_control,
};

int ramfs_device_init(void)
{
    ramfs_dev.type = RT_Device_Class_Miscellaneous;
    ramfs_dev.user_data = RT_NULL;

    rt_device_register(&ramfs_dev, "ramfs", RT_DEVICE_FLAG_RDWR);

    ramfs_dev.ops = &ramfs_ops;

    return 0;
}
INIT_DEVICE_EXPORT(ramfs_device_init);
