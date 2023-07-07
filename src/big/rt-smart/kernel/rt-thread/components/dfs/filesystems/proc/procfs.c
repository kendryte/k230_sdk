/* Copyright (c) 2022, Canaan Bright Sight Co., Ltd
 *
*/
#include <rthw.h>
#include <rtthread.h>
#include <rtdevice.h>

#include <dfs.h>
#include <dfs_fs.h>
#include <dfs_file.h>

#ifdef RT_USING_DFS_PROCFS
#ifdef RT_USING_PROC

struct proc_dirent
{
    rt_proc_entry_t *entrys;
    rt_uint16_t read_index;
    rt_uint16_t device_count;
};

int dfs_proc_fs_mount(struct dfs_filesystem *fs, unsigned long rwflag, const void *data)
{
    return RT_EOK;
}

int dfs_proc_fs_close(struct dfs_fd *file)
{
    rt_err_t result;
    rt_proc_entry_t entry_id;

    RT_ASSERT(file != RT_NULL);
    RT_ASSERT(file->fnode->ref_count > 0);

    if (file->fnode->ref_count > 1)
    {
        return 0;
    }

    if (file->fnode->type == FT_DIRECTORY)
    {
        struct proc_dirent *root_dirent;

        root_dirent = (struct proc_dirent *)file->fnode->data;
        RT_ASSERT(root_dirent != RT_NULL);

        /* release dirent */
        rt_free(root_dirent);
        return RT_EOK;
    }

    entry_id = (rt_proc_entry_t)file->fnode->data;
    RT_ASSERT(entry_id != RT_NULL);
    file->fnode->data = RT_NULL;

    return RT_EOK;

}

int dfs_proc_fs_open(struct dfs_fd *file)
{
    rt_err_t result;
    rt_proc_entry_t entry;
    rt_base_t level;
    RT_ASSERT(file->fnode->ref_count > 0);

    if (file->fnode->ref_count > 1)
    {
        file->pos = 0;
        return 0;
    }

    /* open root directory */
    if ((file->fnode->path[0] == '/') && (file->fnode->path[1] == '\0') &&
        (file->flags & O_DIRECTORY))
    {
        struct rt_object *object;
        struct rt_list_node *node;
        struct rt_object_information *information;
        struct proc_dirent *root_dirent;
        rt_uint32_t count = 0;

        /* disable interrupt */
        level = rt_hw_interrupt_disable();

        information = rt_object_get_information(RT_Object_Class_Proc);
        RT_ASSERT(information != RT_NULL);
        for (node = information->object_list.next; node != &(information->object_list); node = node->next)
        {
            count ++;
        }

        root_dirent = (struct proc_dirent *)rt_malloc(sizeof(struct proc_dirent) +
                      count * sizeof(rt_proc_entry_t));
        if (root_dirent != RT_NULL)
        {
            root_dirent->entrys = (rt_proc_entry_t *)(root_dirent + 1);
            root_dirent->read_index = 0;
            root_dirent->device_count = count;
            count = 0;
            /* get all proc entry  node */
            for (node = information->object_list.next; node != &(information->object_list); node = node->next)
            {
                object = rt_list_entry(node, struct rt_object, list);
                root_dirent->entrys[count] = (rt_proc_entry_t)object;
                count ++;
            }
        }
        rt_hw_interrupt_enable(level);

        /* set data */
        file->fnode->data = root_dirent;

        return RT_EOK;
    }

    entry = rt_proc_entry_find(&file->fnode->path[1]);
    if (entry == RT_NULL)
    {
        return -ENODEV;
    }

#ifdef RT_USING_POSIX
    if (entry->fops)
    {

        file->fnode->fops = entry->fops;
        file->fnode->data = (void *)entry;

        if (file->fnode->fops->open)
        {
            result = file->fnode->fops->open(file);
            if (result == RT_EOK || result == -RT_ENOSYS)
            {
                file->fnode->type = FT_DEVICE;
                return 0;
            }
        }
    }
#endif

    file->fnode->data = RT_NULL;
    return -EIO;
}

int dfs_proc_fs_stat(struct dfs_filesystem *fs, const char *path, struct stat *st)
{
    /* stat root directory */
    if ((path[0] == '/') && (path[1] == '\0'))
    {
        st->st_dev = 0;

        st->st_mode = S_IFREG | S_IRUSR | S_IRGRP | S_IROTH |
                      S_IWUSR | S_IWGRP | S_IWOTH;
        st->st_mode &= ~S_IFREG;
        st->st_mode |= S_IFDIR | S_IXUSR | S_IXGRP | S_IXOTH;

        st->st_size  = 0;
        st->st_mtime = 0;

        return RT_EOK;
    }
    else
    {
        rt_proc_entry_t entry_id;

        entry_id = rt_proc_entry_find(&path[1]);
        if (entry_id != RT_NULL)
        {
            st->st_dev = 0;

            st->st_mode = S_IRUSR | S_IRGRP | S_IROTH |
                          S_IWUSR | S_IWGRP | S_IWOTH;

            st->st_size  = 0;
            st->st_mtime = 0;
            return RT_EOK;
        }
    }
    return -ENOENT;
}

int dfs_proc_fs_getdents(struct dfs_fd *file, struct dirent *dirp, uint32_t count)
{
    rt_uint32_t index;
    rt_object_t object;
    struct dirent *d;
    struct proc_dirent *root_dirent;

    root_dirent = (struct proc_dirent *)file->fnode->data;
    RT_ASSERT(root_dirent != RT_NULL);

    /* make integer count */
    count = (count / sizeof(struct dirent));
    if (count == 0)
        return -EINVAL;

    for (index = 0; index < count && index + root_dirent->read_index < root_dirent->device_count; 
        index ++)
    {
        object = (rt_object_t)root_dirent->entrys[root_dirent->read_index + index];

        d = dirp + index;
        d->d_type = DT_REG;
        d->d_namlen = RT_NAME_MAX;
        d->d_reclen = (rt_uint16_t)sizeof(struct dirent);
        rt_strncpy(d->d_name, object->name, RT_NAME_MAX);
    }

    root_dirent->read_index += index;

    return index * sizeof(struct dirent);
    return 0;
}

static const struct dfs_file_ops _proc_fops =
{
    dfs_proc_fs_open,
    dfs_proc_fs_close,
    RT_NULL,                    
    RT_NULL,                    /* read */
    RT_NULL,                    /* write */
    RT_NULL,                    /* flush */
    RT_NULL,                    /* lseek */
    dfs_proc_fs_getdents,
    RT_NULL,
};

static const struct dfs_filesystem_ops _proc_fs =
{
    "procfs",
    DFS_FS_FLAG_DEFAULT,
    &_proc_fops,

    dfs_proc_fs_mount,
    RT_NULL,
    RT_NULL,
    RT_NULL,

    RT_NULL,
    dfs_proc_fs_stat,
    RT_NULL,
};

int procfs_init(void)
{
    int ret = 0;
    ret = dfs_register(&_proc_fs);
    return 0;
}
#endif
#endif