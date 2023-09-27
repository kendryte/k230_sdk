#ifndef  __SHARE_FS_COMMAND_H__
#define  __SHARE_FS_COMMAND_H__

#define __SFS_ALIGN(length, align)    (((unsigned long)length+align-1)&(~(align-1)))

enum __command
{
	SFS_CMD_INVALID = 0,
	SFS_CMD_OPEN,         /* 1.open */
	SFS_CMD_CLOSE,        /* 2.close */
	SFS_CMD_READ,         /* 3.read */
	SFS_CMD_WRITE,        /* 4.write */
	SFS_CMD_LSEEK,        /* 5.seek */
	SFS_CMD_IOCTL,        /* 6.ioctl */
	SFS_CMD_SYNC,         /* 7.sync */
	SFS_CMD_DUP,          /* 8.dup */
	SFS_CMD_OPENDIR,      /* 9.opendir */
	SFS_CMD_CLOSEDIR,     /* 10.closedir */
	SFS_CMD_READDIR,      /* 11.readdir */
	SFS_CMD_REWINDDIR,    /* 12.rewinddir */
	SFS_CMD_BIND,         /* 13.bind */
	SFS_CMD_UNBIND,       /* 14.unbind */
	SFS_CMD_STATFS,       /* 15.statfs */
	SFS_CMD_UNLINK,       /* 16.unlink */
	SFS_CMD_MKDIR,        /* 17.mkdir */
	SFS_CMD_RMDIR,        /* 18.rmdir */
	SFS_CMD_RENAME,       /* 19.rename */
	SFS_CMD_STAT,         /* 20.stat */
	SFS_CMD_UTIME,        /* 21.utime */
	SFS_CMD_CHATTR,       /* 22.chattr : chmod */
	SFS_CMD_NUM
};

/* sfs_command */
struct sfs_request {
	int cmd;
	unsigned int id;
	unsigned long long request[0];
};

struct sfs_response {
	int cmd;
	unsigned int id;
	unsigned long long response[0];
};
/* open */
struct sfs_request_open
{
	unsigned int flags;
	unsigned int mode;
	unsigned long long path[0];
};
struct sfs_response_open
{
	int fd;
};

/* close */
struct sfs_request_close
{
	int fd;
};
struct sfs_response_close
{
	int ret;
};

/* read */
struct sfs_request_read
{
	int fd;
	unsigned long long phys_addr;
	unsigned long phys_size;
	unsigned long read_size;
};
struct sfs_response_read
{
	long long size;
};

/* write */
struct sfs_request_write
{
	int fd;
	unsigned long long phys_addr;
	unsigned long phys_size;
	unsigned long write_size;
};
struct sfs_response_write
{
	long long size;
};

/* lseek */
struct sfs_request_lseek
{
	int fd;
	int whence;
	long long offset;
};
struct sfs_response_lseek
{
	long long offset;
};

/* ioctl */
struct sfs_request_ioctl
{
	int fd;
	long long cmd ;
	unsigned long long args;
};
struct sfs_response_ioctl
{
	int ret;
};

/* sync */
struct sfs_request_sync
{
	int fd;
};
struct sfs_response_sync
{
	int ret;
};

/* dup */
struct sfs_request_dup
{
	int fd;
};
struct sfs_response_dup
{
	int new_fd;
};

/* opendir */
struct sfs_request_opendir
{
	unsigned long long path[0];
};
struct sfs_response_opendir
{
	unsigned long long p_DIR;
};

/* closedir */
struct sfs_request_closedir
{
	unsigned long long p_DIR;
};
struct sfs_response_closedir
{
	int ret;
};

/* readdir */
struct sfs_request_readdir
{
	unsigned long long p_DIR;
};
struct sfs_response_readdir
{
	int ret;
	unsigned long long  d_ino;
	long long	d_off;
	int d_reclen;
	unsigned char d_type;
	int d_name_len;
	unsigned long long d_name[0];
};

/* rewinddir */
struct sfs_request_rewinddir
{
	unsigned long long p_DIR;
};
struct sfs_response_rewinddir
{
	int ret;
};

/* statfs */
struct sfs_request_statfs
{
	unsigned long long path[0];
};
struct sfs_response_statfs
{
	int ret;

	unsigned long long f_blocks;
	unsigned long long f_bfree;
	unsigned long long f_bavail;
	unsigned long long f_files;
	unsigned long long f_ffree;
	unsigned int f_type;
	int f_bsize;
	int f_fsid__val[2];
	int f_namelen;
	int f_frsize;
	int f_flags;
	int f_spare[4];
};

/* unlink */
struct sfs_request_unlink
{
	unsigned long long path[0];
};
struct sfs_response_unlink
{
	int ret;
};

/* mkdir */
struct sfs_request_mkdir
{
	unsigned int mode;
	unsigned long long path[0];
};
struct sfs_response_mkdir
{
	int ret;
};

/* rmdir */
struct sfs_request_rmdir
{
	unsigned long long path[0];
};
struct sfs_response_rmdir
{
	int ret;
};

/* rename */
struct sfs_request_rename
{
	unsigned long long old_path[0];
};
struct sfs_response_rename
{
	int ret;
};

/* stat */
struct sfs_request_stat
{
	unsigned long long path[0];
};
struct timespec__ {
	long long  tv_sec;
	unsigned long long tv_nsec;
};
struct sfs_response_stat
{
	int ret;

	unsigned long long st_dev;		/* Device.  */
	unsigned long long st_ino;		/* File serial number. */
	unsigned int st_mode;			/* File mode.  */
	unsigned int st_nlink;			/* Link count.  */
	unsigned int st_uid;			/* User ID of the file's owner. */
	unsigned int st_gid;			/* Group ID of the file's group.*/
	unsigned long long st_rdev;     /* Device number, if device.  */
	unsigned long long st_size;		/* Size of file, in bytes. */
	unsigned int  st_blksize;		/* Optimal block size for I/O.  */
	unsigned long long st_blocks;	/* 512-byte blocks */

	struct timespec__ st_atim;      /* Time of last access.  */
	struct timespec__ st_mtim;      /* Time of last modification.  */
	struct timespec__ st_ctim;      /* Time of last status change.  */
	//int __glibc_reserved[2];
};

/* utime */
struct sfs_request_utime
{
	unsigned long long modtime;
	unsigned long long actime;
	unsigned long long path[0];
};
struct sfs_response_utime
{
	int ret;
};

/* chattr */
struct sfs_request_chattr
{
	unsigned long long mode;
	unsigned long long path[0];
};
struct sfs_response_chattr
{
	int ret;
};

#endif
