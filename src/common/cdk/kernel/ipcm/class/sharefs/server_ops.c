#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <dirent.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/vfs.h>
#include <utime.h>
#include <string.h>

#include "sfs_common.h"
#include "sfs_command.h"

extern int sfs_server_send(int ipc_fd, void *data, int len);

/*
 * the actual sharefs operation
 */
static int sfs_s_open(int ipc_fd, struct sfs_request *req)
{
	int ret;
	int cmd_len;
	struct sfs_request_open *req_open
		= (struct sfs_request_open *)req->request;
	struct sfs_response *resp_cmd;
	struct sfs_response_open *resp_open;

	cmd_len = sizeof(struct sfs_response)
		+ sizeof(struct sfs_response_open);
	resp_cmd = malloc(cmd_len);
	if (!resp_cmd) {
		sfs_error("malloc for cmd[%d:%d] failed",
				req->cmd, req->id);
		return -1;
	}
	resp_cmd->cmd = req->cmd;
	resp_cmd->id = req->id;
	resp_open = (struct sfs_response_open *)resp_cmd->response;
	resp_open->fd = open((const char *)req_open->path,
			req_open->flags, req_open->mode|S_IRWXU|S_IRWXG|S_IRWXO);
	ret = sfs_server_send(ipc_fd, resp_cmd, cmd_len);
	free(resp_cmd);
	return ret;
}

static int sfs_s_close(int ipc_fd, struct sfs_request *req)
{
	int ret;
	int cmd_len;
	struct sfs_request_close *req_close
		= (struct sfs_request_close *)req->request;
	struct sfs_response *resp_cmd;
	struct sfs_response_close *resp_close;

	cmd_len = sizeof(struct sfs_response)
		+ sizeof(struct sfs_response_close);
	resp_cmd = malloc(cmd_len);
	if (!resp_cmd) {
		sfs_error("malloc for cmd[%d:%d] failed",
				req->cmd, req->id);
		return -1;
	}
	resp_cmd->cmd = req->cmd;
	resp_cmd->id = req->id;
	resp_close = (struct sfs_response_close *)resp_cmd->response;
	resp_close->ret = close(req_close->fd);
	ret = sfs_server_send(ipc_fd, resp_cmd, cmd_len);
	free(resp_cmd);
	return ret;
}

extern void *sfs_mmap(unsigned long phys, unsigned long size,
		unsigned long *mapped_addr, unsigned long *mapped_size);
static int sfs_s_read(int ipc_fd, struct sfs_request *req)
{
	int ret;
	int cmd_len;
	struct sfs_request_read *req_read
		= (struct sfs_request_read *)req->request;
	struct sfs_response *resp_cmd;
	struct sfs_response_read *resp_read;
	void *buffer;
	unsigned long mapped_addr = 0;
	unsigned long mapped_size = 0;

	cmd_len = sizeof(struct sfs_response)
		+ sizeof(struct sfs_response_read);
	resp_cmd = malloc(cmd_len);
	if (!resp_cmd) {
		sfs_error("malloc for cmd[%d:%d] failed",
				req->cmd, req->id);
		return -1;
	}
	resp_cmd->cmd = req->cmd;
	resp_cmd->id = req->id;
	resp_read = (struct sfs_response_read *)resp_cmd->response;

	buffer = sfs_mmap(req_read->phys_addr, req_read->phys_size,
			&mapped_addr, &mapped_size);
	if (NULL == buffer) {
		resp_read->size = -1;
	} else {
		resp_read->size = read(req_read->fd, buffer, req_read->read_size);
	}

	if (mapped_addr)
		sfs_munmap(mapped_addr, mapped_size);

	ret = sfs_server_send(ipc_fd, resp_cmd, cmd_len);
	free(resp_cmd);
	return ret;
}

static int sfs_s_write(int ipc_fd, struct sfs_request *req)
{
	int ret;
	int cmd_len;
	struct sfs_request_write *req_write
		= (struct sfs_request_write *)req->request;
	struct sfs_response *resp_cmd;
	struct sfs_response_write *resp_write;
	void *buffer;
	unsigned long mapped_addr = 0;
	unsigned long mapped_size = 0;

	cmd_len = sizeof(struct sfs_response)
		+ sizeof(struct sfs_response_write);
	resp_cmd = malloc(cmd_len);
	if (!resp_cmd) {
		sfs_error("malloc for cmd[%d:%d] failed",
				req->cmd, req->id);
		return -1;
	}
	resp_cmd->cmd = req->cmd;
	resp_cmd->id = req->id;
	resp_write = (struct sfs_response_write *)resp_cmd->response;

	buffer = sfs_mmap(req_write->phys_addr, req_write->phys_size,
			&mapped_addr, &mapped_size);
	if (NULL == buffer) {
		ret = -1;
		goto free_resp_cmd;
	}
	resp_write->size = write(req_write->fd, buffer, req_write->write_size);
	ret = sfs_server_send(ipc_fd, resp_cmd, cmd_len);

free_resp_cmd:
	if (mapped_addr)
		sfs_munmap(mapped_addr, mapped_size);
	free(resp_cmd);
	return ret;
}

static int sfs_s_lseek(int ipc_fd, struct sfs_request *req)
{
	int ret;
	int cmd_len;
	struct sfs_request_lseek *req_lseek
		= (struct sfs_request_lseek *)req->request;
	struct sfs_response *resp_cmd;
	struct sfs_response_lseek *resp_lseek;

	cmd_len = sizeof(struct sfs_response)
		+ sizeof(struct sfs_response_lseek);
	resp_cmd = malloc(cmd_len);
	if (!resp_cmd) {
		sfs_error("malloc for cmd[%d:%d] failed",
				req->cmd, req->id);
		return -1;
	}
	resp_cmd->cmd = req->cmd;
	resp_cmd->id = req->id;
	resp_lseek = (struct sfs_response_lseek *)resp_cmd->response;
	resp_lseek->offset = lseek(req_lseek->fd,
			(off_t)req_lseek->offset, req_lseek->whence);
	ret = sfs_server_send(ipc_fd, resp_cmd, cmd_len);
	free(resp_cmd);
	return ret;
}

static int sfs_s_ioctl(int ipc_fd, struct sfs_request *req)
{
	int ret;
	int cmd_len;
	struct sfs_request_ioctl *req_ioctl
		= (struct sfs_request_ioctl *)req->request;
	struct sfs_response *resp_cmd;
	struct sfs_response_ioctl *resp_ioctl;

	cmd_len = sizeof(struct sfs_response)
		+ sizeof(struct sfs_response_ioctl);
	resp_cmd = malloc(cmd_len);
	if (!resp_cmd) {
		sfs_error("malloc for cmd[%d:%d] failed",
				req->cmd, req->id);
		return -1;
	}
	resp_cmd->cmd = req->cmd;
	resp_cmd->id = req->id;
	resp_ioctl = (struct sfs_response_ioctl *)resp_cmd->response;
	resp_ioctl->ret = ioctl(req_ioctl->fd, req_ioctl->cmd, req_ioctl->args);
	ret = sfs_server_send(ipc_fd, resp_cmd, cmd_len);
	free(resp_cmd);
	return ret;
}

static int sfs_s_sync(int ipc_fd, struct sfs_request *req)
{
	int ret;
	int cmd_len;
	struct sfs_request_sync *req_sync
		= (struct sfs_request_sync *)req->request;
	struct sfs_response *resp_cmd;
	struct sfs_response_sync *resp_sync;

	cmd_len = sizeof(struct sfs_response)
		+ sizeof(struct sfs_response_sync);
	resp_cmd = malloc(cmd_len);
	if (!resp_cmd) {
		sfs_error("malloc for cmd[%d:%d] failed",
				req->cmd, req->id);
		return -1;
	}
	resp_cmd->cmd = req->cmd;
	resp_cmd->id = req->id;
	resp_sync = (struct sfs_response_sync *)resp_cmd->response;
	resp_sync->ret = fsync(req_sync->fd);
	ret = sfs_server_send(ipc_fd, resp_cmd, cmd_len);
	free(resp_cmd);
	return ret;
}

static int sfs_s_dup(int ipc_fd, struct sfs_request *req)
{
	int ret;
	int cmd_len;
	struct sfs_request_dup *req_dup
		= (struct sfs_request_dup *)req->request;
	struct sfs_response *resp_cmd;
	struct sfs_response_dup *resp_dup;

	cmd_len = sizeof(struct sfs_response)
		+ sizeof(struct sfs_response_dup);
	resp_cmd = malloc(cmd_len);
	if (!resp_cmd) {
		sfs_error("malloc for cmd[%d:%d] failed",
				req->cmd, req->id);
		return -1;
	}
	resp_cmd->cmd = req->cmd;
	resp_cmd->id = req->id;
	resp_dup = (struct sfs_response_dup *)resp_cmd->response;
	resp_dup->new_fd = dup(req_dup->fd);
	ret = sfs_server_send(ipc_fd, resp_cmd, cmd_len);
	free(resp_cmd);
	return ret;
}

static int sfs_s_opendir(int ipc_fd, struct sfs_request *req)
{
	int ret;
	int cmd_len;
	DIR *p_DIR = NULL;
	struct sfs_request_opendir *req_opendir
		= (struct sfs_request_opendir *)req->request;
	struct sfs_response *resp_cmd;
	struct sfs_response_opendir *resp_opendir;

	cmd_len = sizeof(struct sfs_response)
		+ sizeof(struct sfs_response_opendir);
	resp_cmd = malloc(cmd_len);
	if (!resp_cmd) {
		sfs_error("malloc for cmd[%d:%d] failed",
				req->cmd, req->id);
		return -1;
	}
	resp_cmd->cmd = req->cmd;
	resp_cmd->id = req->id;
	resp_opendir = (struct sfs_response_opendir *)resp_cmd->response;
	p_DIR = opendir((const char *)req_opendir->path);
	resp_opendir->p_DIR = (unsigned long long)p_DIR;
	ret = sfs_server_send(ipc_fd, resp_cmd, cmd_len);
	free(resp_cmd);
	return ret;
}

static int sfs_s_closedir(int ipc_fd, struct sfs_request *req)
{
	int ret;
	int cmd_len;
	struct sfs_request_closedir *req_closedir
		= (struct sfs_request_closedir *)req->request;
	struct sfs_response *resp_cmd;
	struct sfs_response_closedir *resp_closedir;

	cmd_len = sizeof(struct sfs_response)
		+ sizeof(struct sfs_response_closedir);
	resp_cmd = malloc(cmd_len);
	if (!resp_cmd) {
		sfs_error("malloc for cmd[%d:%d] failed",
				req->cmd, req->id);
		return -1;
	}
	resp_cmd->cmd = req->cmd;
	resp_cmd->id = req->id;
	resp_closedir = (struct sfs_response_closedir *)resp_cmd->response;
	resp_closedir->ret = closedir((DIR *)req_closedir->p_DIR);
	ret = sfs_server_send(ipc_fd, resp_cmd, cmd_len);
	free(resp_cmd);
	return ret;
}

static int sfs_s_readdir(int ipc_fd, struct sfs_request *req)
{
	int ret;
	int cmd_len;
	struct dirent *dir;
	struct sfs_request_readdir *req_readdir
		= (struct sfs_request_readdir *)req->request;
	struct sfs_response *resp_cmd;
	struct sfs_response_readdir *resp_readdir;

	cmd_len = sizeof(struct sfs_response)
		+ sizeof(struct sfs_response_readdir)
		/* struct dirent->'char d_name[256];' use it */
		+ 256;
	resp_cmd = malloc(cmd_len);
	if (!resp_cmd) {
		sfs_error("malloc for cmd[%d:%d] failed",
				req->cmd, req->id);
		return -1;
	}
	memset(resp_cmd, 0, cmd_len);
	resp_cmd->cmd = req->cmd;
	resp_cmd->id = req->id;
	resp_readdir = (struct sfs_response_readdir *)resp_cmd->response;
	dir = readdir((DIR *)req_readdir->p_DIR);
	if (dir) {
		resp_readdir->d_ino = dir->d_ino;
		resp_readdir->d_off = dir->d_off;
		resp_readdir->d_reclen = dir->d_reclen;
		resp_readdir->d_type = dir->d_type;
		resp_readdir->d_name_len = strlen(dir->d_name);
		memcpy(resp_readdir->d_name, dir->d_name, strlen(dir->d_name));
		cmd_len = sizeof(struct sfs_response)
			+ sizeof(struct sfs_response_readdir)
			+ resp_readdir->d_name_len;
		resp_readdir->ret = 0;
	} else {
		resp_readdir->ret = -1;
	}
	ret = sfs_server_send(ipc_fd, resp_cmd, cmd_len);
	free(resp_cmd);
	return ret;
}

static int sfs_s_rewinddir(int ipc_fd, struct sfs_request *req)
{
	int ret;
	int cmd_len;
	struct sfs_request_rewinddir *req_rewinddir
		= (struct sfs_request_rewinddir *)req->request;
	struct sfs_response *resp_cmd;
	struct sfs_response_rewinddir *resp_rewinddir;

	cmd_len = sizeof(struct sfs_response)
		+ sizeof(struct sfs_response_rewinddir);
	resp_cmd = malloc(cmd_len);
	if (!resp_cmd) {
		sfs_error("malloc for cmd[%d:%d] failed",
				req->cmd, req->id);
		return -1;
	}
	resp_cmd->cmd = req->cmd;
	resp_cmd->id = req->id;
	resp_rewinddir = (struct sfs_response_rewinddir *)resp_cmd->response;
	rewinddir((DIR *)req_rewinddir->p_DIR);
	resp_rewinddir->ret = 0;
	ret = sfs_server_send(ipc_fd, resp_cmd, cmd_len);
	free(resp_cmd);
	return ret;
}

static int sfs_s_statfs(int ipc_fd, struct sfs_request *req)
{
	int ret;
	int cmd_len;
	struct sfs_request_statfs *req_statfs
		= (struct sfs_request_statfs *)req->request;
	struct sfs_response *resp_cmd;
	struct sfs_response_statfs *resp_statfs;
	struct statfs fs_info;

	cmd_len = sizeof(struct sfs_response)
		+ sizeof(struct sfs_response_statfs);
	resp_cmd = malloc(cmd_len);
	if (!resp_cmd) {
		sfs_error("malloc for cmd[%d:%d] failed",
				req->cmd, req->id);
		return -1;
	}
	resp_cmd->cmd = req->cmd;
	resp_cmd->id = req->id;
	resp_statfs = (struct sfs_response_statfs *)resp_cmd->response;

	resp_statfs->ret = statfs((const char *)req_statfs->path, &fs_info);
	if (!resp_statfs->ret) {
		resp_statfs->f_type         = fs_info.f_type;
		resp_statfs->f_bsize        = fs_info.f_bsize;
		resp_statfs->f_blocks       = fs_info.f_blocks;
		resp_statfs->f_bfree        = fs_info.f_bfree;
		resp_statfs->f_bavail       = fs_info.f_bavail;
		resp_statfs->f_files        = fs_info.f_files;
		resp_statfs->f_ffree        = fs_info.f_ffree;
		resp_statfs->f_fsid__val[0] = fs_info.f_fsid.__val[0];
		resp_statfs->f_fsid__val[1] = fs_info.f_fsid.__val[1];
		resp_statfs->f_namelen      = fs_info.f_namelen;
		resp_statfs->f_frsize       = fs_info.f_frsize;
		resp_statfs->f_spare[0]     = fs_info.f_spare[0];
		resp_statfs->f_spare[1]     = fs_info.f_spare[1];
		resp_statfs->f_spare[2]     = fs_info.f_spare[2];
		resp_statfs->f_spare[3]     = fs_info.f_spare[3];
		//resp_statfs->f_spare[4]     = fs_info.f_spare[4];
	}
	ret = sfs_server_send(ipc_fd, resp_cmd, cmd_len);
	free(resp_cmd);
	return ret;
}

static int sfs_s_unlink(int ipc_fd, struct sfs_request *req)
{
	int ret;
	int cmd_len;
	struct sfs_request_unlink *req_unlink
		= (struct sfs_request_unlink *)req->request;
	struct sfs_response *resp_cmd;
	struct sfs_response_unlink *resp_unlink;
	cmd_len = sizeof(struct sfs_response)
		+ sizeof(struct sfs_response_unlink);
	resp_cmd = malloc(cmd_len);
	if (!resp_cmd) {
		sfs_error("malloc for cmd[%d:%d] failed",
				req->cmd, req->id);
		return -1;
	}
	resp_cmd->cmd = req->cmd;
	resp_cmd->id = req->id;
	resp_unlink = (struct sfs_response_unlink *)resp_cmd->response;
	resp_unlink->ret = unlink(req_unlink->path);
	ret = sfs_server_send(ipc_fd, resp_cmd, cmd_len);
	free(resp_cmd);
	return ret;
}

static int sfs_s_mkdir(int ipc_fd, struct sfs_request *req)
{
	int ret;
	int cmd_len;
	struct sfs_request_mkdir *req_mkdir
		= (struct sfs_request_mkdir *)req->request;
	struct sfs_response *resp_cmd;
	struct sfs_response_mkdir *resp_mkdir;
	cmd_len = sizeof(struct sfs_response)
		+ sizeof(struct sfs_response_mkdir);
	resp_cmd = malloc(cmd_len + 16);
	if (!resp_cmd) {
		sfs_error("malloc for cmd[%d:%d] failed",
				req->cmd, req->id);
		return -1;
	}
	resp_cmd->cmd = req->cmd;
	resp_cmd->id = req->id;
	resp_mkdir = (struct sfs_response_mkdir *)resp_cmd->response;
	resp_mkdir->ret = mkdir((const char *)req_mkdir->path, req_mkdir->mode);
	ret = sfs_server_send(ipc_fd, resp_cmd, cmd_len);
	free(resp_cmd);
	return ret;
}


static int sfs_s_rmdir(int ipc_fd, struct sfs_request *req)
{
	int ret;
	int cmd_len;
	struct sfs_request_rmdir *req_rmdir
		= (struct sfs_request_rmdir *)req->request;
	struct sfs_response *resp_cmd;
	struct sfs_response_rmdir *resp_rmdir;

	cmd_len = sizeof(struct sfs_response)
		+ sizeof(struct sfs_response_rmdir);
	resp_cmd = malloc(cmd_len);
	if (!resp_cmd) {
		sfs_error("malloc for cmd[%d:%d] failed",
				req->cmd, req->id);
		return -1;
	}
	resp_cmd->cmd = req->cmd;
	resp_cmd->id = req->id;
	resp_rmdir = (struct sfs_response_rmdir *)resp_cmd->response;
	resp_rmdir->ret = rmdir(req_rmdir->path);
	ret = sfs_server_send(ipc_fd, resp_cmd, cmd_len);
	free(resp_cmd);
	return ret;
}

static int sfs_s_rename(int ipc_fd, struct sfs_request *req)
{
	int ret;
	int cmd_len;
	struct sfs_request_rename *req_rename
		= (struct sfs_request_rename *)req->request;
	struct sfs_response *resp_cmd;
	struct sfs_response_rename *resp_rename;
	char *old_path;
	char *new_path;
	cmd_len = sizeof(struct sfs_response)
		+ sizeof(struct sfs_response_rename);
	resp_cmd = malloc(cmd_len);
	if (!resp_cmd) {
		sfs_error("malloc for cmd[%d:%d] failed",
				req->cmd, req->id);
		return -1;
	}
	resp_cmd->cmd = req->cmd;
	resp_cmd->id = req->id;
	resp_rename = (struct sfs_response_rename *)resp_cmd->response;

	old_path = (char *)req_rename->old_path;
	/* there has a '\0' at the end of old_path */
	new_path = (char *)(old_path + strlen(old_path) + 1);
	resp_rename->ret = rename(old_path, new_path);
	ret = sfs_server_send(ipc_fd, resp_cmd, cmd_len);
	free(resp_cmd);
	return ret;
}

static int sfs_s_stat(int ipc_fd, struct sfs_request *req)
{
	int ret;
	int cmd_len;
	struct sfs_request_stat *req_stat
		= (struct sfs_request_stat *)req->request;
	struct sfs_response *resp_cmd;
	struct sfs_response_stat *resp_stat;
	struct stat buf;

	cmd_len = sizeof(struct sfs_response)
		+ sizeof(struct sfs_response_stat);
	resp_cmd = malloc(cmd_len);
	if (!resp_cmd) {
		sfs_error("malloc for cmd[%d:%d] failed",
				req->cmd, req->id);
		return -1;
	}
	resp_cmd->cmd = req->cmd;
	resp_cmd->id = req->id;
	resp_stat = (struct sfs_response_stat *)resp_cmd->response;
	resp_stat->ret = stat((const char *)req_stat->path, &buf);
	if (!resp_stat->ret) {
		resp_stat->st_dev = buf.st_dev;
		resp_stat->st_ino = buf.st_ino;
		resp_stat->st_mode = buf.st_mode;
		resp_stat->st_nlink = buf.st_nlink;
		resp_stat->st_uid = buf.st_uid;
		resp_stat->st_gid = buf.st_gid;
		resp_stat->st_rdev = buf.st_rdev;
		resp_stat->st_size = buf.st_size;
		resp_stat->st_blksize = buf.st_blksize;
		resp_stat->st_blocks = buf.st_blocks;

		resp_stat->st_atim.tv_sec  = buf.st_atim.tv_sec;
		resp_stat->st_atim.tv_nsec = buf.st_atim.tv_nsec;
		resp_stat->st_mtim.tv_sec  = buf.st_mtim.tv_sec;
		resp_stat->st_mtim.tv_nsec = buf.st_mtim.tv_nsec;
		resp_stat->st_ctim.tv_sec  = buf.st_ctim.tv_sec;
		resp_stat->st_ctim.tv_nsec = buf.st_ctim.tv_nsec;
		//resp_stat->__glibc_reserved[0] = buf.__glibc_reserved[0];
		//resp_stat->__glibc_reserved[1] = buf.__glibc_reserved[1];
	}
	ret = sfs_server_send(ipc_fd, resp_cmd, cmd_len);
	free(resp_cmd);
	return ret;
}

static int sfs_s_utime(int ipc_fd, struct sfs_request *req)
{
	int ret;
	int cmd_len;
	struct sfs_request_utime *req_utime
		= (struct sfs_request_utime *)req->request;
	struct sfs_response *resp_cmd;
	struct sfs_response_utime *resp_utime;
	struct utimbuf buf;

	cmd_len = sizeof(struct sfs_response)
		+ sizeof(struct sfs_response_utime);
	resp_cmd = malloc(cmd_len);
	if (!resp_cmd) {
		sfs_error("malloc for cmd[%d:%d] failed",
				req->cmd, req->id);
		return -1;
	}
	resp_cmd->cmd = req->cmd;
	resp_cmd->id = req->id;
	resp_utime = (struct sfs_response_utime *)resp_cmd->response;

	buf.modtime = req_utime->modtime;
	buf.actime = req_utime->actime;
	resp_utime->ret = utime((const char *)req_utime->path, &buf);
	ret = sfs_server_send(ipc_fd, resp_cmd, cmd_len);
	free(resp_cmd);
	return ret;
}

static int sfs_s_chattr(int ipc_fd, struct sfs_request *req)
{
	int ret;
	int cmd_len;
	struct sfs_request_chattr *req_chattr
		= (struct sfs_request_chattr *)req->request;
	struct sfs_response *resp_cmd;
	struct sfs_response_chattr *resp_chattr;

	cmd_len = sizeof(struct sfs_response)
		+ sizeof(struct sfs_response_chattr);
	resp_cmd = malloc(cmd_len);
	if (!resp_cmd) {
		sfs_error("malloc for cmd[%d:%d] failed",
				req->cmd, req->id);
		return -1;
	}
	resp_cmd->cmd = req->cmd;
	resp_cmd->id = req->id;
	resp_chattr = (struct sfs_response_chattr *)resp_cmd->response;
	resp_chattr->ret = chmod((const char *)req_chattr->path, req_chattr->mode);
	ret = sfs_server_send(ipc_fd, resp_cmd, cmd_len);
	free(resp_cmd);
	return ret;
}

int sfs_server_dispatch_cmd(int fd, void *data, unsigned int count)
{
	struct sfs_request *req = data;

    //sfs_error("send, cmd:%d, id:%d, count:%d", req->cmd, req->id, count);
	switch (req->cmd) {
		case SFS_CMD_OPEN: 		/* open */
			return sfs_s_open(fd, req);
		case SFS_CMD_CLOSE:		/* close */
			return sfs_s_close(fd, req);
		case SFS_CMD_READ: 		/* read */
			return sfs_s_read(fd, req);
		case SFS_CMD_WRITE: 		/* write */
			return sfs_s_write(fd, req);
		case SFS_CMD_LSEEK: 		/* seek */
			return sfs_s_lseek(fd, req);
		case SFS_CMD_IOCTL:		/* ioctl */
			return sfs_s_ioctl(fd, req);
		case SFS_CMD_SYNC: 		/* sync */
			return sfs_s_sync(fd, req);
		case SFS_CMD_DUP:			/* dup */
			return sfs_s_dup(fd, req);
		case SFS_CMD_OPENDIR:		/* opendir */
			return sfs_s_opendir(fd, req);
		case SFS_CMD_CLOSEDIR: 	/* closedir */
			return sfs_s_closedir(fd, req);
		case SFS_CMD_READDIR:		/* readdir */
			return sfs_s_readdir(fd, req);
		case SFS_CMD_REWINDDIR:	/* rewinddir */
			return sfs_s_rewinddir(fd, req);
		case SFS_CMD_BIND: 		/* bind */
			return -1;
		case SFS_CMD_UNBIND:		/* unbind */
			return -1;
		case SFS_CMD_STATFS:		/* statfs */
			return sfs_s_statfs(fd, req);
		case SFS_CMD_UNLINK:		/* unlink */
			return sfs_s_unlink(fd, req);
		case SFS_CMD_MKDIR:		/* mkdir */
			return sfs_s_mkdir(fd, req);
		case SFS_CMD_RMDIR:		/* rmdir */
			return sfs_s_rmdir(fd, req);
		case SFS_CMD_RENAME:		/* rename */
			return sfs_s_rename(fd, req);
		case SFS_CMD_STAT: 		/* stat */
			return sfs_s_stat(fd, req);
		case SFS_CMD_UTIME:		/* for utime*/
			return sfs_s_utime(fd, req);
		case SFS_CMD_CHATTR:		/* for chattr*/
			return sfs_s_chattr(fd, req);
		default:
			sfs_error("not supported CMD: %d\n", req->cmd);
			return -1;
	}
}

