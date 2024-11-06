#if 0
#include "asm/platform.h"
#include "los_tables.h"
#include "los_mux.h"
#include "los_sem.h"
#include "stdio.h"
#include "string.h"
#include "stdlib.h"
#include "sys/mount.h"
#include "fs/fs.h"
#include "fs/dirent_fs.h"
#include "dirent.h"
#include "sys/stat.h"
#include "errno.h"
#endif
#include <rthw.h>
#include <rtthread.h>
#include <rtdevice.h>
#include <dfs.h>
#include <dfs_fs.h>
#include <dfs_file.h>
#include <stdint.h>

#define DBG_TAG    "UART"
#define DBG_LVL    DBG_INFO
#include <rtdbg.h>

#ifdef RT_USING_POSIX
#include <dfs_posix.h>
#include <dfs_poll.h>
#include <posix_termios.h>

//#include "ipcm_ioctl.h"
#include "ipcm_osadapt.h"
#include "ipcm_nodes.h"
#include "ipcm_userdev.h"
#include "ipcm_funcs.h"
#endif

#ifdef RT_USING_USERSPACE
#include <lwp_user_mm.h>
#endif
#include "cache.h"

#include "sfs_common.h"
#include "sfs_command.h"

#define MAX_SHAREFS_MOUNT_LEN    32
/* extern function declaration */

#define PAGE_SIZE_ALIGN          0x1000
#define CACHE_LINE_ALIGN    PAGE_SIZE_ALIGN

extern int sfs_ipc_connected(void);
#define sfs_check(ptr)      if((ptr) == NULL) return (-EINVAL)
#define sfs_ipc_check()     if(!sfs_ipc_connected()) return (-EIO)

struct sfs_mount_point {
	void *handle;
	char mount_path[0];
};
struct sfs_open_point {
	int fd;
};
struct sfs_opendir_point {
	unsigned long long dir;
};

#define ROMFS_DIRENT_FILE   0x00
#define ROMFS_DIRENT_DIR    0x01

struct sharefs_dirent
{
    rt_uint32_t      type;  /* dirent type */

    const char       *name; /* dirent name */
    const rt_uint8_t *data; /* file date ptr */
    rt_size_t        size;  /* file size */
};

#if 0
static sfs_get_mode_by_flags(unsigned int flag)
{
	unsigned char mode;
	mode = FA_READ;
	if (file->flags & O_WRONLY)
	{
		mode |= FA_WRITE;
	}
	if ((file->flags & O_ACCMODE) & O_RDWR)
	{
		mode |= FA_WRITE;
	}
	/* Opens the file, if it is existing. If not, a new file is created. */
	if (file->flags & O_CREAT)
	{
		mode |= FA_OPEN_ALWAYS;
	}
	/* Creates a new file. If the file is existing, it is truncated and overwritten. */
	if (file->flags & O_TRUNC)
	{
		mode |= FA_CREATE_ALWAYS;
	}
	/* Creates a new file. The function fails if the file is already existing. */
	if (file->flags & O_EXCL)
	{
		mode |= FA_CREATE_NEW;
	}
}
#endif

static int dfs_sharefs_lseek_r(struct dfs_fd *fd, off_t offset, int whence);
static struct sfs_response *sfs_client_request(struct sfs_request *req_cmd, int count);
/*
 * operate for share fs
 * */

static int dfs_sharefs_mkdir(struct dfs_fd *fd)
{
	/*2023/3/30 unsport*/
#if 1
	int ret = 0;
	struct sfs_request *req_cmd;
	struct sfs_request_mkdir *req_mkdir;
	struct sfs_response *resp_cmd;
	struct sfs_mount_point *mount_point;
	int cmd_len;
	int path_len;

	sfs_check(fd);
	sfs_ipc_check();

	mount_point = fd->fnode->fs->data;
	path_len = strlen(mount_point->mount_path)
		/* add a '\0' and a '\\' to the path, add two to path_len */
		+ strlen(fd->fnode->path) + 2;
	cmd_len = sizeof(struct sfs_request)
		+ sizeof(struct sfs_request_mkdir)
		+ path_len;
	req_cmd = malloc(cmd_len);
	if (!req_cmd) {
		sfs_error("alloc for mkdir failed\n");
		return -ENOMEM;
	}
	memset(req_cmd, 0, cmd_len);
	req_cmd->cmd = SFS_CMD_MKDIR;
	req_mkdir = (struct sfs_request_mkdir *)req_cmd->request;
	snprintf((char *)req_mkdir->path, path_len, "%s/%s",
			mount_point->mount_path, fd->fnode->path);
	req_mkdir->mode = S_IRWXG;
	resp_cmd = sfs_client_request(req_cmd, cmd_len);
	if (resp_cmd) {
		struct sfs_response_mkdir *resp_mkdir
			= (struct sfs_response_mkdir *)resp_cmd->response;
		if (resp_mkdir->ret) {
			sfs_error("mkdir %s failed\n", req_mkdir->path);
		}
		ret = resp_mkdir->ret;
		free(resp_cmd);
	} else {
		sfs_error("mkdir no response\n");
		ret = -EIO;
	}
	free(req_cmd);
	return ret;
#else
    sfs_error("not support mkdir cmd\n");
	return -1;
#endif
}

static int dfs_sharefs_opendir(struct dfs_fd *fd)
{
	int ret = 0;
	struct sfs_request *req_cmd;
	struct sfs_request_opendir *req_opendir;
	struct sfs_response *resp_cmd;
	struct sfs_response_opendir *resp_opendir;
	struct sfs_mount_point *mount_point;
	int cmd_len;
	int path_len;

	sfs_check(fd);
	sfs_ipc_check();
	mount_point = fd->fnode->fs->data;
	path_len = strlen(mount_point->mount_path)
		/* add a '\0' and a '\\' to the path, add two to path_len */
		+ strlen(fd->fnode->path) + 2;
	cmd_len = sizeof(struct sfs_request)
		+ sizeof(struct sfs_request_opendir)
		+ path_len;
	req_cmd = malloc(cmd_len);
	if (!req_cmd) {
		sfs_error("alloc for opendir %s failed\n", fd->fnode->path);
		return -ENOMEM;
	}
	memset(req_cmd, 0, cmd_len);
	req_cmd->cmd = SFS_CMD_OPENDIR;
	req_opendir = (struct sfs_request_opendir *)req_cmd->request;
	snprintf((char *)req_opendir->path, path_len, "%s/%s",
			mount_point->mount_path, fd->fnode->path);
	resp_cmd = sfs_client_request(req_cmd, cmd_len);
	if (!resp_cmd) {
		sfs_error("opendir %s, no response\n", fd->fnode->path);
		ret = -EIO;
		goto free_req_cmd;
	}
	resp_opendir = (struct sfs_response_opendir *)resp_cmd->response;
	if (resp_opendir->p_DIR) {
		struct sfs_opendir_point *point =
			malloc(sizeof(struct sfs_opendir_point));
		if (point) {
			point->dir = resp_opendir->p_DIR;
			//rt_kprintf("dir:%lx\n", point->dir);
			fd->fnode->type = FT_DIRECTORY;
			fd->data = point;
			ret = 0;
		} else {
			sfs_error("alloc for opendir %s failed\n", fd->fnode->path);
			ret = -ENOMEM;
		}
	} else {
		sfs_error("opendir %s failed\n", fd->fnode->path);
		ret = -EFAULT;
	}
	free(resp_cmd);
free_req_cmd:
	free(req_cmd);
	return ret;
}

int dfs_sharefs_open(struct dfs_fd *fd)
{
	int ret = 0;
	struct sfs_request *req_cmd;
	struct sfs_request_open *req_open;
	struct sfs_response *resp_cmd;
	struct sfs_response_open *resp_open;
	struct sfs_mount_point *mount_point;
	int path_len;
	int cmd_len;
	if ((fd->flags & O_CREAT) && (fd->flags & O_DIRECTORY)) {
		ret =  dfs_sharefs_mkdir(fd);
		if(!ret)
			return dfs_sharefs_opendir(fd);
	} else if (fd->flags & O_DIRECTORY) {
		return dfs_sharefs_opendir(fd);
	}
	mount_point = fd->fnode->fs->data;
	path_len = strlen(mount_point->mount_path) + strlen(fd->fnode->path) + 2;
	cmd_len = sizeof(struct sfs_request)
		+ sizeof(struct sfs_request_open)
		+ path_len;
	req_cmd = malloc(cmd_len);
	if (!req_cmd) {
		sfs_error("alloc for open failed");
		return -ENOMEM;
	}
	memset(req_cmd, 0, cmd_len);
	req_cmd->cmd = SFS_CMD_OPEN;
	req_open = (struct sfs_request_open *)req_cmd->request;
	req_open->flags = fd->flags;
	req_open->mode = 0;
	snprintf((char *)req_open->path, path_len, "%s/%s",
			mount_point->mount_path, fd->fnode->path);

	resp_cmd = sfs_client_request(req_cmd, cmd_len);
	if (!resp_cmd) {
		sfs_error("open %s, no response\n", req_open->path);
		ret = -EIO;
		goto free_req_cmd;
	}
	resp_open = (struct sfs_response_open *)resp_cmd->response;
	if (resp_open->fd < 0) {
		sfs_error("open %s failed\n", req_open->path);
		ret = -EFAULT;
	} else {
		struct sfs_open_point *open_point
			= malloc(sizeof(struct sfs_open_point));
		if (open_point) {
			open_point->fd = resp_open->fd;
			fd->fnode->type = FT_REGULAR;
			fd->data = (void *)open_point;
			ret = 0;
		} else {
			sfs_error("alloc for open failed");
			ret = -ENOMEM;
		}
	}
	if(!ret) {
		fd->fnode->size = dfs_sharefs_lseek_r(fd, 0, SEEK_END);
		if (fd->flags & O_APPEND) {
			fd->pos = fd->fnode->size;
		} else {
			fd->pos = 0;
			dfs_sharefs_lseek_r(fd, 0, SEEK_SET);
		}
	}
	free(resp_cmd);
free_req_cmd:
	free(req_cmd);
	return ret;
}

static int dfs_sharefs_closedir(struct dfs_fd *fd)
{
	int ret = 0;
	struct sfs_request *req_cmd;
	struct sfs_request_closedir *req_closedir;
	struct sfs_response *resp_cmd;
	int cmd_len;

	sfs_check(fd);
	sfs_ipc_check();

	cmd_len = sizeof(struct sfs_request)
		+ sizeof(struct sfs_request_closedir);
	req_cmd = malloc(cmd_len);
	if (!req_cmd) {
		sfs_error("alloc for closedir failed\n");
		return -ENOMEM;
	}
	req_cmd->cmd = SFS_CMD_CLOSEDIR;
	req_closedir = (struct sfs_request_closedir *)req_cmd->request;
	req_closedir->p_DIR = ((struct sfs_opendir_point *)(fd->data))->dir;
	resp_cmd = sfs_client_request(req_cmd, cmd_len);
	if (resp_cmd) {
		struct sfs_response_closedir *resp_closedir
			= (struct sfs_response_closedir *)resp_cmd->response;
		if (resp_closedir->ret) {
			sfs_error("closedir failed, ret:%d\n", resp_closedir->ret);
		}
		ret = resp_closedir->ret;
		free(resp_cmd);
	} else {
		sfs_error("closedir no response\n");
		ret = -EIO;
	}
	free(fd->data);
	fd->data = NULL;
	free(req_cmd);
	return ret;
}

int dfs_sharefs_close(struct dfs_fd *fd)
{
	int ret = 0;
	struct sfs_request *req_cmd;
	struct sfs_request_close *req_close;
	struct sfs_response *resp_cmd;
	struct sfs_response_close *resp_close;
	int cmd_len;
	if (fd->fnode->type == FT_DIRECTORY) {
		return dfs_sharefs_closedir(fd);
	}

	sfs_check(fd);
	sfs_ipc_check();
	cmd_len = sizeof(struct sfs_request)
		+ sizeof(struct sfs_request_close);
	req_cmd = malloc(cmd_len);
	if (!req_cmd) {
		sfs_error("alloc for %d failed\n", SFS_CMD_CLOSE);
		return -ENOMEM;
	}
	req_cmd->cmd = SFS_CMD_CLOSE;
	req_close = (struct sfs_request_close *)req_cmd->request;
	req_close->fd = ((struct sfs_open_point *)(fd->data))->fd;
	resp_cmd = sfs_client_request(req_cmd, cmd_len);
	if (!resp_cmd) {
		sfs_error("close %d, no response\n", req_close->fd);
		ret = -EIO;
		goto free_fd;
	}
	resp_close = (struct sfs_response_close *)resp_cmd->response;
	if (resp_close->ret < 0) {
		sfs_error("close %d failed\n", req_close->fd);
	}
	ret = resp_close->ret;
	free(resp_cmd);

free_fd:
	free(fd->data);
	fd->data = NULL;
	free(req_cmd);
	return ret;
}

int dfs_sharefs_ioctl(struct dfs_fd *fd, int cmd, void *args)
{
	/*Not supported*/
	return -1;
}

int dfs_sharefs_read(struct dfs_fd *fd, void *buf, size_t count)
{
	unsigned long align_phys = 0;
	void *virt_addr;
	unsigned long align_len;
	int ret = 0;
	struct sfs_request *req_cmd;
	struct sfs_request_read *req_read;
	struct sfs_response *resp_cmd;
	struct sfs_response_read *resp_read;
	int cmd_len;

	sfs_check(fd);
	sfs_check(buf);
	sfs_ipc_check();
	align_len = __SFS_ALIGN(count, CACHE_LINE_ALIGN);
	/*FIXME: should be physical address */
	virt_addr = rt_pages_alloc(rt_page_bits(align_len));
	align_phys = (unsigned long)virt_addr + PV_OFFSET;
	if (!align_phys) {
		sfs_error("rt_pages_alloc for read buf fail\n");
		return -ENOMEM;
	}
	cmd_len = sizeof(struct sfs_request)
		+ sizeof(struct sfs_request_read);
	req_cmd = malloc(cmd_len);
	if (!req_cmd) {
		sfs_error("alloc for %d failed\n", SFS_CMD_READ);
		ret = -ENOMEM;
		goto free_align_buf;
	}
	req_cmd->cmd = SFS_CMD_READ;
	req_read = (struct sfs_request_read *)req_cmd->request;
	req_read->phys_addr = (unsigned long long)align_phys;
	req_read->phys_size = align_len;
	req_read->read_size = count;
	req_read->fd = ((struct sfs_open_point *)(fd->data))->fd;

	/* need invalidate here to make sure the data be right */
	rt_hw_cpu_dcache_clean_flush(virt_addr, align_len);

	resp_cmd = sfs_client_request(req_cmd, cmd_len);
	if (!resp_cmd) {
		sfs_error("read %d, no response\n", req_read->fd);
		ret = -EIO;
		goto free_req_cmd;
	}
	resp_read = (struct sfs_response_read *)resp_cmd->response;
	if (resp_read->size > 0) {
		if (count > (size_t)resp_read->size)
			count = (size_t)resp_read->size;
		/* need invalidate here to make sure the data be right */
		rt_hw_cpu_dcache_ops(RT_HW_CACHE_INVALIDATE, virt_addr, align_len);
		__memcpy__(buf, (void *)virt_addr, count);
		ret = count;
		fd->pos += count;
	} else if (resp_read->size < 0) {
		sfs_error("read error. ret:%d\n", resp_read->size);
		ret = -EFAULT;
	}
	free(resp_cmd);
free_req_cmd:
	free(req_cmd);
free_align_buf:
	rt_pages_free(virt_addr, rt_page_bits(align_len));
	return ret;
}

int dfs_sharefs_write(struct dfs_fd *fd, const void *buf, size_t count)
{
	void *align_phys = NULL;
	void *virt_addr;
	unsigned long  align_len;
	int ret = 0;
	struct sfs_request *req_cmd;
	struct sfs_request_write *req_write;
	struct sfs_response *resp_cmd;
	struct sfs_response_write *resp_write;
	int cmd_len;

	sfs_check(fd);
	sfs_check(buf);
	sfs_ipc_check();

	align_len = __SFS_ALIGN(count, CACHE_LINE_ALIGN);
	/*FIXME: should be physical address */
	virt_addr = rt_pages_alloc(rt_page_bits(align_len));
	align_phys = virt_addr + PV_OFFSET;
	if (!align_phys) {
		sfs_error("rt_pages_alloc for write buf fail\n");
		return -ENOMEM;
	}
	cmd_len = sizeof(struct sfs_request)
		+ sizeof(struct sfs_request_read);
	req_cmd = malloc(cmd_len);
	if (!req_cmd) {
		sfs_error("alloc for write failed");
		ret = -ENOMEM;
		goto free_align_buf;
	}
	req_cmd->cmd = SFS_CMD_WRITE;
	req_write = (struct sfs_request_write *)req_cmd->request;
	req_write->phys_addr = (unsigned long long)align_phys;
	req_write->phys_size = align_len;
	req_write->write_size = count;
	req_write->fd = ((struct sfs_open_point *)(fd->data))->fd;

	__memcpy__(virt_addr, buf, count);
	rt_hw_cpu_dcache_ops(RT_HW_CACHE_FLUSH, virt_addr, align_len);
	resp_cmd = sfs_client_request(req_cmd, cmd_len);
	if (!resp_cmd) {
		sfs_error("write %d, no response\n", req_write->fd);
		ret = -EIO;
		goto free_req_cmd;
	}
	resp_write = (struct sfs_response_write *)resp_cmd->response;
	if (resp_write->size > 0) {
		if (count > (size_t)resp_write->size)
			count = (size_t)resp_write->size;
		ret = count;
		fd->pos += count;
	} else if (resp_write->size < 0) {
		sfs_error("write error. ret:%d\n", resp_write->size);
		ret = -EFAULT;
	}
	free(resp_cmd);
free_req_cmd:
	free(req_cmd);
free_align_buf:
	rt_pages_free(virt_addr, rt_page_bits(align_len));
	return ret;
}
int dfs_sharefs_flush(struct dfs_fd *fd)
{
	int ret = 0;
	struct sfs_request *req_cmd;
	struct sfs_request_sync *req_sync;
	struct sfs_response *resp_cmd;
	int cmd_len;

	sfs_check(fd);
	sfs_ipc_check();
	cmd_len = sizeof(struct sfs_request)
		+ sizeof(struct sfs_request_sync);
	req_cmd = malloc(cmd_len);
	if (!req_cmd) {
		sfs_error("alloc for sync failed\n");
		return -ENOMEM;
	}
	req_cmd->cmd = SFS_CMD_SYNC;
	req_sync = (struct sfs_request_sync *)req_cmd->request;
	req_sync->fd = ((struct sfs_open_point *)(fd->data))->fd;

	resp_cmd = sfs_client_request(req_cmd, cmd_len);
	if (resp_cmd) {
		struct sfs_response_sync *resp_sync
			= (struct sfs_response_sync *)resp_cmd->response;
		ret = resp_sync->ret;
		free(resp_cmd);
	} else {
		sfs_error("sync %d, no response\n", req_sync->fd);
		ret = -EIO;
	}
	free(req_cmd);
	return ret;
}
static int dfs_sharefs_rewinddir(struct dfs_fd *fd)
{
	int ret = 0;
	struct sfs_request *req_cmd;
	struct sfs_request_rewinddir *req_rewinddir;
	struct sfs_response *resp_cmd;
	int cmd_len;

	sfs_check(fd);
	sfs_ipc_check();
	cmd_len = sizeof(struct sfs_request)
		+ sizeof(struct sfs_request_rewinddir);
	req_cmd = malloc(cmd_len);
	if (!req_cmd) {
		sfs_error("alloc for readdir failed\n");
		return -ENOMEM;
	}
	req_cmd->cmd = SFS_CMD_REWINDDIR;
	req_rewinddir = (struct sfs_request_rewinddir *)req_cmd->request;
	req_rewinddir->p_DIR = ((struct sfs_opendir_point *)(fd->data))->dir;
	resp_cmd = sfs_client_request(req_cmd, cmd_len);
	if (resp_cmd) {
		struct sfs_response_rewinddir *resp_rewinddir
			= (struct sfs_response_rewinddir *)resp_cmd->response;
		ret = resp_rewinddir->ret;
		free(resp_cmd);
	} else {
		sfs_error("rewinddir no response\n");
		ret = -EIO;
	}
	free(req_cmd);
	return ret;
}

int dfs_sharefs_lseek(struct dfs_fd *fd, off_t offset)
{
	off_t ret = 0;
	struct sfs_request *req_cmd;
	struct sfs_request_lseek *req_lseek;
	struct sfs_response *resp_cmd;
	struct sfs_response_lseek *resp_lseek;
	int cmd_len;

	sfs_check(fd);
	sfs_ipc_check();
	if(fd->fnode->type == FT_DIRECTORY) {
		return dfs_sharefs_rewinddir(fd);
	}
	cmd_len = sizeof(struct sfs_request)
		+ sizeof(struct sfs_request_lseek);
	req_cmd = malloc(cmd_len);
	if (!req_cmd) {
		sfs_error("alloc for %d failed\n", SFS_CMD_LSEEK);
		return -ENOMEM;
	}
	req_cmd->cmd = SFS_CMD_LSEEK;
	req_lseek = (struct sfs_request_lseek *)req_cmd->request;
	req_lseek->fd = ((struct sfs_open_point *)(fd->data))->fd;
	req_lseek->offset = offset;
	req_lseek->whence = SEEK_SET;
	resp_cmd = sfs_client_request(req_cmd, cmd_len);
	if (!resp_cmd) {
		sfs_error("lseek %d, no response\n", req_lseek->fd);
		ret = -EIO;
		goto free_req_cmd;
	}
	resp_lseek = (struct sfs_response_lseek *)resp_cmd->response;
	if (resp_lseek->offset < 0) {
		sfs_error("lseek %d failed\n", req_lseek->fd);
	}
	ret = (off_t)resp_lseek->offset;
	fd->pos = ret;
	free(resp_cmd);
free_req_cmd:
	free(req_cmd);
	return ret;
}

static int dfs_sharefs_lseek_r(struct dfs_fd *fd, off_t offset, int whence)
{
	off_t ret = 0;
	struct sfs_request *req_cmd;
	struct sfs_request_lseek *req_lseek;
	struct sfs_response *resp_cmd;
	struct sfs_response_lseek *resp_lseek;
	int cmd_len;

	sfs_check(fd);
	sfs_ipc_check();
	cmd_len = sizeof(struct sfs_request)
		+ sizeof(struct sfs_request_lseek);
	req_cmd = malloc(cmd_len);
	if (!req_cmd) {
		sfs_error("alloc for %d failed\n", SFS_CMD_LSEEK);
		return -ENOMEM;
	}
	req_cmd->cmd = SFS_CMD_LSEEK;
	req_lseek = (struct sfs_request_lseek *)req_cmd->request;
	req_lseek->fd = ((struct sfs_open_point *)(fd->data))->fd;
	req_lseek->offset = offset;
	req_lseek->whence = whence;
	resp_cmd = sfs_client_request(req_cmd, cmd_len);
	if (!resp_cmd) {
		sfs_error("lseek %d, no response\n", req_lseek->fd);
		ret = -EIO;
		goto free_req_cmd;
	}
	resp_lseek = (struct sfs_response_lseek *)resp_cmd->response;
	if (resp_lseek->offset < 0) {
		sfs_error("lseek %d failed\n", req_lseek->fd);
	}
	ret = (off_t)resp_lseek->offset;
	free(resp_cmd);
free_req_cmd:
	free(req_cmd);
	return ret;
}

#define MAX_DIR_NAME_LEN    256
int dfs_sharefs_getdents(struct dfs_fd *fd, struct dirent *dir, uint32_t count)
{
    int ret = 0;
    struct sfs_request *req_cmd;
    struct sfs_request_readdir *req_readdir;
    struct sfs_response *resp_cmd;
    struct sfs_response_readdir *resp_readdir;
    struct dirent *d;
    int cmd_len;
    sfs_check(dir);
    sfs_ipc_check();
    d =  dir;
    cmd_len = sizeof(struct sfs_request)
        + sizeof(struct sfs_request_readdir);
    req_cmd = malloc(cmd_len);
    if (!req_cmd) {
        sfs_error("alloc for readdir failed\n");
        return -ENOMEM;
    }
    req_cmd->cmd = SFS_CMD_READDIR;
    req_readdir = (struct sfs_request_readdir *)req_cmd->request;
    req_readdir->p_DIR = ((struct sfs_opendir_point *)(fd->data))->dir;
	//rt_kprintf("dir:%lx\n", req_readdir->p_DIR);
	resp_cmd = sfs_client_request(req_cmd, cmd_len);
	if (!resp_cmd) {
		sfs_error("readdir no response\n");
		ret = -EIO;
		goto free_req_cmd;
	}
	resp_readdir = (struct sfs_response_readdir *)resp_cmd->response;
	if (!resp_readdir->ret) {
		if (resp_readdir->d_name_len >= MAX_DIR_NAME_LEN)
			resp_readdir->d_name_len = MAX_DIR_NAME_LEN - 1;
		__memcpy__(d->d_name, resp_readdir->d_name, resp_readdir->d_name_len);
		d->d_name[resp_readdir->d_name_len] = '\0';
		d->d_type = resp_readdir->d_type;
		d->d_namlen = resp_readdir->d_name_len;
		ret = sizeof(struct dirent);
		d->d_reclen = ret;
	} else {
		ret = 0;
	}
    free(resp_cmd);
free_req_cmd:
    free(req_cmd);
	//rt_kprintf("ret:%d\n", ret);
    return ret;
}

static const struct dfs_file_ops _sharefs_fops =
{
    dfs_sharefs_open,
    dfs_sharefs_close,
    dfs_sharefs_ioctl,
    dfs_sharefs_read,
    dfs_sharefs_write,
    dfs_sharefs_flush, /* flush */
    dfs_sharefs_lseek,
    dfs_sharefs_getdents,
};

int dfs_sharefs_mount(struct dfs_filesystem *fs, unsigned long rwflag, const void *data)
{
	struct sfs_mount_point *point = malloc(sizeof(struct sfs_mount_point) + 16);
	fs->data = point;
	strcpy(point->mount_path, "/sharefs");
	return 0;
}

int dfs_sharefs_unmount(struct dfs_filesystem *fs)
{
	struct sfs_mount_point *point = fs->data		;
	if(point) {
		free(point);
	}
	return 0;
}

int dfs_sharefs_mkfs(rt_device_t devid)
{
	return 0;
}

int dfs_sharefs_statfs(struct dfs_filesystem *fs, struct statfs *buf)
{
	int ret = 0;
	struct sfs_request *req_cmd;
	struct sfs_request_statfs *req_statfs;
	struct sfs_response *resp_cmd;
	struct sfs_mount_point *mount_point;
	int cmd_len;
	//rt_kprintf("%s %d\n", __func__, __LINE__);
	sfs_check(buf);
	sfs_ipc_check();

	mount_point = fs->data;
	cmd_len = sizeof(struct sfs_request)
		+ sizeof(struct sfs_request_statfs)
		/* add a '\0' to the path */
		+ strlen(mount_point->mount_path) + 1;
	req_cmd = malloc(cmd_len);
	if (!req_cmd) {
		sfs_error("alloc for statfs failed\n");
		return -ENOMEM;
	}
	memset(req_cmd, 0, cmd_len);
	req_cmd->cmd = SFS_CMD_STATFS;
	req_statfs = (struct sfs_request_statfs *)req_cmd->request;
	__memcpy__(req_statfs->path, mount_point->mount_path,
			strlen(mount_point->mount_path));
	resp_cmd = sfs_client_request(req_cmd, cmd_len);
	if (resp_cmd) {
		struct sfs_response_statfs *resp_statfs
			= (struct sfs_response_statfs *)resp_cmd->response;
		if (!resp_statfs->ret) {
			buf->f_bsize = resp_statfs->f_bsize;
			buf->f_blocks = resp_statfs->f_blocks;
			buf->f_bfree = resp_statfs->f_bfree;
		}
		ret = resp_statfs->ret;
		free(resp_cmd);
	} else {
		sfs_error("statfs no response\n");
		ret = -EIO;
	}
	free(req_cmd);
	return ret;
}


static int dfs_sharefs_rmdir(struct dfs_filesystem *fs, const char *pathname)
{
	int ret = 0;
	struct sfs_request *req_cmd;
	struct sfs_request_rmdir *req_rmdir;
	struct sfs_response *resp_cmd;
	struct sfs_mount_point *mount_point;
	int cmd_len;
	int path_len;

	sfs_check(fs);
	sfs_check(pathname);
	sfs_ipc_check();
	path_len = strlen(pathname);
	cmd_len = sizeof(struct sfs_request)
		+ sizeof(struct sfs_request_rmdir)
		+ path_len;
	req_cmd = malloc(cmd_len);
	if (!req_cmd) {
		sfs_error("alloc for rmdir failed\n");
		return -ENOMEM;
	}
	memset(req_cmd, 0, cmd_len);
	req_cmd->cmd = SFS_CMD_RMDIR;
	req_rmdir = (struct sfs_request_rmdir *)req_cmd->request;
	snprintf((char *)req_rmdir->path, path_len, "%s", pathname);
	resp_cmd = sfs_client_request(req_cmd, cmd_len);
	if (resp_cmd) {
		struct sfs_response_rmdir *resp_rmdir
			= (struct sfs_response_rmdir *)resp_cmd->response;
		if (resp_rmdir->ret) {
			sfs_error("rmdir %s failed\n", req_rmdir->path);
		}
		ret = resp_rmdir->ret;
		free(resp_cmd);
	} else {
		sfs_error("rmdir no response\n");
		ret = -EIO;
	}
	free(req_cmd);
	return ret;
}


int dfs_sharefs_unlink(struct dfs_filesystem *fs, const char *pathname)
{
	int ret = 0;
	struct sfs_request *req_cmd;
	struct sfs_request_unlink *req_unlink;
	struct sfs_response *resp_cmd;
	struct sfs_mount_point *mount_point;
	int cmd_len;
	int path_len;

	sfs_check(fs);
	sfs_check(pathname);
	sfs_ipc_check();

	mount_point = fs->data;
	path_len = strlen(mount_point->mount_path)
		/* add a '\0' and a '\\' to the path, add two to path_len */
		+ strlen(pathname) + 2;
	cmd_len = sizeof(struct sfs_request)
		+ sizeof(struct sfs_request_statfs)
		+ path_len;
	req_cmd = malloc(cmd_len);
	if (!req_cmd) {
		sfs_error("alloc for unlink failed\n");
		return -ENOMEM;
	}
	memset(req_cmd, 0, cmd_len);
	req_cmd->cmd = SFS_CMD_UNLINK;
	req_unlink = (struct sfs_request_unlink *)req_cmd->request;
	snprintf((char *)req_unlink->path, path_len, "%s/%s",
			mount_point->mount_path, pathname);
	resp_cmd = sfs_client_request(req_cmd, cmd_len);
	if (resp_cmd) {
		struct sfs_response_unlink *resp_unlink
			= (struct sfs_response_unlink *)resp_cmd->response;
		if (resp_unlink->ret) {
			if(dfs_sharefs_rmdir(fs, req_unlink->path))
				sfs_error("unlink %s failed\n", req_unlink->path);
		}
		ret = resp_unlink->ret;
		free(resp_cmd);
	} else {
		sfs_error("unlink no response\n");
		ret = -EIO;
	}
	free(req_cmd);
	return ret;
}

int dfs_sharefs_stat(struct dfs_filesystem *fs, const char *filename, struct stat *buf)
{
	int ret = 0;
	struct sfs_request *req_cmd;
	struct sfs_request_stat *req_stat;
	struct sfs_response *resp_cmd;
	struct sfs_mount_point *mount_point;
	int path_len;
	int cmd_len;

	//rt_kprintf("%s %d\n", __func__, __LINE__);
	sfs_check(buf);
	sfs_ipc_check();
	mount_point = fs->data;
	path_len = strlen(mount_point->mount_path) + strlen(filename) + 2;
		/* add a '\0' and a '\\' to the path, add two to path_len */
	cmd_len = sizeof(struct sfs_request)
		+ sizeof(struct sfs_request_stat)
		+ path_len;
	req_cmd = malloc(cmd_len);
	if (!req_cmd) {
		sfs_error("alloc for stat failed\n");
		return -ENOMEM;
	}
	memset(req_cmd, 0, cmd_len);
	req_cmd->cmd = SFS_CMD_STAT;
	req_stat = (struct sfs_request_stat *)req_cmd->request;
	snprintf((char *)req_stat->path, path_len, "%s/%s",
			mount_point->mount_path, filename);
	resp_cmd = sfs_client_request(req_cmd, cmd_len);
	if (resp_cmd) {
		struct sfs_response_stat *resp_stat
			= (struct sfs_response_stat *)resp_cmd->response;
		if (!resp_stat->ret) {
			buf->st_dev    = resp_stat->st_dev;
			buf->st_ino    = resp_stat->st_ino;
			buf->st_mode   = resp_stat->st_mode;
			buf->st_nlink  = resp_stat->st_nlink;
			buf->st_uid    = resp_stat->st_uid;
			buf->st_gid    = resp_stat->st_gid;
			buf->st_rdev   = resp_stat->st_rdev;
			buf->st_size   = resp_stat->st_size;
			buf->st_blksize = resp_stat->st_blksize;
			buf->st_blocks  = resp_stat->st_blocks;
			buf->st_atim.tv_sec  = resp_stat->st_atim.tv_sec;
			buf->st_atim.tv_nsec = resp_stat->st_atim.tv_nsec;
			buf->st_mtim.tv_sec  = resp_stat->st_mtim.tv_sec;
			buf->st_mtim.tv_nsec = resp_stat->st_mtim.tv_nsec;
			buf->st_ctim.tv_sec  = resp_stat->st_ctim.tv_sec;
			buf->st_ctim.tv_nsec = resp_stat->st_ctim.tv_nsec;
			//buf->__unused4 = resp_stat->__glibc_reserved[0];
			//buf->__unused5 = resp_stat->__glibc_reserved[1];
		}
		ret = resp_stat->ret;
		free(resp_cmd);
	} else {
		sfs_error("stat no response\n");
		ret = -EIO;
	}
	free(req_cmd);
	return ret;
}
int dfs_sharefs_rename(struct dfs_filesystem *fs, const char *oldpath, const char *newpath)
{
	int ret = 0;
	struct sfs_request *req_cmd;
	struct sfs_request_rename *req_rename;
	struct sfs_response *resp_cmd;
	struct sfs_mount_point *mount_point;
	int cmd_len;
	int old_path_len;
	int new_path_len;

	sfs_check(fs);
	sfs_check(oldpath);
	sfs_check(newpath);
	sfs_ipc_check();
	mount_point = fs->data;
	old_path_len = strlen(mount_point->mount_path)
		/* add a '\0' and a '\\' to the path, add two to path_len */
		+ strlen(oldpath) + 2;
	new_path_len = strlen(mount_point->mount_path)
		/* add a '\0' and a '\\' to the path, add two to path_len */
		+ strlen(newpath) + 2;
	cmd_len = sizeof(struct sfs_request)
		+ sizeof(struct sfs_request_rename)
		+ old_path_len + new_path_len;
	req_cmd = malloc(cmd_len);
	if (!req_cmd) {
		sfs_error("alloc for rename failed\n");
		return -ENOMEM;
	}
	memset(req_cmd, 0, cmd_len);
	req_cmd->cmd = SFS_CMD_RENAME;
	req_rename = (struct sfs_request_rename *)req_cmd->request;
	snprintf((char *)req_rename->old_path,
			old_path_len, "%s/%s",
			mount_point->mount_path, oldpath);
	snprintf((char *)((unsigned long)req_rename->old_path + old_path_len),
			new_path_len, "%s/%s",
			mount_point->mount_path, newpath);
	//rt_kprintf("old:%s new:%s \n", req_rename->old_path, &req_rename->old_path + old_path_len);
	resp_cmd = sfs_client_request(req_cmd, cmd_len);
	if (resp_cmd) {
		struct sfs_response_rename *resp_rename
			= (struct sfs_response_rename *)resp_cmd->response;
		if (resp_rename->ret) {
			sfs_error("rename %s failed\n", req_rename->old_path);
		}
		ret = resp_rename->ret;
		free(resp_cmd);
	} else {
		sfs_error("rename no response\n");
		ret = -EIO;
	}
	free(req_cmd);
	return ret;
}
static const struct dfs_filesystem_ops _sharefs =
{
    "share",
    DFS_FS_FLAG_DEFAULT,
    &_sharefs_fops,

    dfs_sharefs_mount,
    dfs_sharefs_unmount,
	NULL,
	dfs_sharefs_statfs,
	dfs_sharefs_unlink,
	dfs_sharefs_stat,
	dfs_sharefs_rename,
};

static unsigned int g_cmd_count;
static rt_sem_t g_sfs_sem;
static rt_mutex_t g_cmd_list_mutex;
static unsigned int g_sfs_mount_count;

struct sfs_cmd_list {
	struct sfs_cmd_list *prev;
	struct sfs_cmd_list *next;
	int resp_len;
	unsigned long response[0];
};
struct sfs_cmd_list g_recv_cmd;

static void init_cmd_list(struct sfs_cmd_list *head)
{
	head->prev = head;
	head->next = head;
}

static void free_cmd_list(struct sfs_cmd_list *head)
{
	struct sfs_cmd_list *cmd, *next;

	while (head->next != head) {
		cmd = head->next;
		next = cmd->next;
		head->next = next;
		next->prev = head;
	}
}

static void insert_cmd_list(struct sfs_cmd_list *node,
		struct sfs_cmd_list *head)
{
	struct sfs_cmd_list *prev = head->prev;
	node->next = head;
	node->prev = prev;
	prev->next = node;
	head->prev = node;
}

static void delect_cmd_list(struct sfs_cmd_list *node)
{
	struct sfs_cmd_list *prev = node->prev;
	struct sfs_cmd_list *next = node->next;
	prev->next = next;
	next->prev = prev;
	node->prev = NULL;
	node->next = NULL;
}

#define list_for_each_cmd(cur, head) \
	for (cur = (head)->next; cur != (head); cur = cur->next)

#define REQUEST_TIMEOUT    1000
extern int sfs_client_send(void *buf, int count);
static struct sfs_response *sfs_client_request(struct sfs_request *req_cmd, int count)
{
	struct sfs_cmd_list *cursor;
	struct sfs_response *resp_cmd;
	unsigned int ret;

	rt_mutex_take(g_cmd_list_mutex, RT_WAITING_FOREVER);
	g_cmd_count++;
	req_cmd->id = g_cmd_count;
	rt_mutex_release(g_cmd_list_mutex);

	sfs_client_send(req_cmd, count);
retry:
	rt_sem_take(g_sfs_sem, RT_WAITING_FOREVER);
	rt_mutex_take(g_cmd_list_mutex, RT_WAITING_FOREVER);
	list_for_each_cmd(cursor, &g_recv_cmd) {
		resp_cmd = (struct sfs_response *)cursor->response;
		if ((req_cmd->cmd == resp_cmd->cmd)
				&& (req_cmd->id == resp_cmd->id)) {
			delect_cmd_list(cursor);
			break;
		}
	}
	rt_mutex_release(g_cmd_list_mutex);

	if (cursor != &g_recv_cmd) {
		resp_cmd = malloc(cursor->resp_len);
		if (resp_cmd) {
			__memcpy__(resp_cmd, cursor->response, cursor->resp_len);
		} else {
			sfs_error("no memory for request:[%d,%lu]",
					req_cmd->cmd, req_cmd->id);
		}
		free(cursor);
		return resp_cmd;
	} else {
		rt_sem_release(g_sfs_sem);
		goto retry;
	}
}

void sfs_client_response(void *buf, int count)
{
	struct sfs_cmd_list *insert;

	insert = malloc(sizeof(struct sfs_cmd_list) + count);
	if (!insert) {
		sfs_error("maloc for response failed");
		return;
	}
	__memcpy__(insert->response, buf, count);
	insert->resp_len = count;

	rt_mutex_take(g_cmd_list_mutex, RT_WAITING_FOREVER);
	insert_cmd_list(insert, &g_recv_cmd);
	rt_mutex_release(g_cmd_list_mutex);

	/* wakeup the operation */
	(void)rt_sem_release(g_sfs_sem);
}

int sfs_client_init(const char *mount_path)
{
	int ret = -1;

	if (!mount_path) {
		sfs_error("invalid path for sharefs mount point!\n");
		return -EINVAL;
	}
	if (strlen(mount_path) >= MAX_SHAREFS_MOUNT_LEN) {
		sfs_error("mount point path len too long!");
		return -EINVAL;
	}

	dfs_register(&_sharefs);

    ret = dfs_mount(RT_NULL, mount_path, "share", 0, NULL) != 0;
    if(ret) {
        sfs_error("Dir %s mount failed!\n", mount_path);
        return -1;
    }
	init_cmd_list(&g_recv_cmd);
	g_cmd_count = 0;
	g_sfs_sem = rt_sem_create("sfs_sem", 0, RT_IPC_FLAG_PRIO);
	if(g_sfs_sem == RT_NULL)
	{
		sfs_error("sharefs create sem failed\n");
		return ret;
	}
	g_cmd_list_mutex = rt_mutex_create("sfs_cmd_list_mutex", RT_IPC_FLAG_PRIO);
	if(g_cmd_list_mutex == RT_NULL)
	{
		sfs_error("sfs_cmd_list_mutex create faied\n");
		rt_sem_delete(g_sfs_sem);
		return ret;
	}
	g_sfs_mount_count++;

	return ret;
}

void sfs_client_deinit(const char *mount_path)
{
	g_sfs_mount_count--;
	if (!g_sfs_mount_count) {
		rt_mutex_delete(g_cmd_list_mutex);
		rt_sem_delete(g_sfs_sem);
		g_cmd_count = 0;
	}
#if 0
	if (umount(mount_path))
		sfs_error("umount path %s failed", mount_path);
#endif
}

