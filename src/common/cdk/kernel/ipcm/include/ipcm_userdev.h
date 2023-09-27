#ifndef __IPCM_USERDEV_H__
#define __IPCM_USERDEV_H__

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus*/

#if defined(RT_USING_MUSL)
#include <sys/ioctl.h>
/* ioctl command encoding: 32 bits total, command in lower 16 bits,
 * size of the parameter structure in the lower 14 bits of the
 * upper 16 bits.
 * Encoding the size of the parameter structure in the ioctl request
 * is useful for catching programs compiled with old versions
 * and to avoid overwriting user space outside the user buffer area.
 * The highest 2 bits are reserved for indicating the ``access mode''.
 * NOTE: This limits the max parameter size to 16kB -1 !
 */

/*
 * The following is for compatibility across the various Linux
 * platforms.  The generic ioctl numbering scheme doesn't really enforce
 * a type field.  De facto, however, the top 8 bits of the lower 16
 * bits are indeed used as a type field, so we might just as well make
 * this explicit here.  Please be sure to use the decoding macros
 * below from now on.
 */
#define _IOC_NRBITS	8
#define _IOC_TYPEBITS	8

/*
 * Let any architecture override either of the following before
 * including this file.
 */

#ifndef _IOC_SIZEBITS
# define _IOC_SIZEBITS	14
#endif

#ifndef _IOC_DIRBITS
# define _IOC_DIRBITS	2
#endif

#define _IOC_NRMASK	((1 << _IOC_NRBITS)-1)
#define _IOC_TYPEMASK	((1 << _IOC_TYPEBITS)-1)
#define _IOC_SIZEMASK	((1 << _IOC_SIZEBITS)-1)
#define _IOC_DIRMASK	((1 << _IOC_DIRBITS)-1)

#define _IOC_NRSHIFT	0
#define _IOC_TYPESHIFT	(_IOC_NRSHIFT+_IOC_NRBITS)
#define _IOC_SIZESHIFT	(_IOC_TYPESHIFT+_IOC_TYPEBITS)
#define _IOC_DIRSHIFT	(_IOC_SIZESHIFT+_IOC_SIZEBITS)

/*
 * Direction bits, which any architecture can choose to override
 * before including this file.
 *
 * NOTE: _IOC_WRITE means userland is writing and kernel is
 * reading. _IOC_READ means userland is reading and kernel is writing.
 */

#ifndef _IOC_NONE
# define _IOC_NONE	0U
#endif

#ifndef _IOC_WRITE
# define _IOC_WRITE	1U
#endif

#ifndef _IOC_READ
# define _IOC_READ	2U
#endif

#include <libc/libc_ioctl.h>

#define _IOC_TYPECHECK(t) (sizeof(t))

/*
 * Used to create numbers.
 *
 * NOTE: _IOW means userland is writing and kernel is reading. _IOR
 * means userland is reading and kernel is writing.
 */
#define __IO(type,nr)		_IOC(_IOC_NONE,(type),(nr),0)
#define __IOR(type,nr,size)	_IOC(_IOC_READ,(type),(nr),(_IOC_TYPECHECK(size)))
#define __IOW(type,nr,size)	_IOC(_IOC_WRITE,(type),(nr),(_IOC_TYPECHECK(size)))
#define __IOWR(type,nr,size)	_IOC(_IOC_READ|_IOC_WRITE,(type),(nr),(_IOC_TYPECHECK(size)))
#define __IOR_BAD(type,nr,size)	_IOC(_IOC_READ,(type),(nr),sizeof(size))
#define __IOW_BAD(type,nr,size)	_IOC(_IOC_WRITE,(type),(nr),sizeof(size))
#define __IOWR_BAD(type,nr,size)	_IOC(_IOC_READ|_IOC_WRITE,(type),(nr),sizeof(size))

/* used to decode ioctl numbers.. */
#define _IOC_DIR(nr)		(((nr) >> _IOC_DIRSHIFT) & _IOC_DIRMASK)
#define _IOC_TYPE(nr)		(((nr) >> _IOC_TYPESHIFT) & _IOC_TYPEMASK)
#define _IOC_NR(nr)		(((nr) >> _IOC_NRSHIFT) & _IOC_NRMASK)
#define _IOC_SIZE(nr)		(((nr) >> _IOC_SIZESHIFT) & _IOC_SIZEMASK)

/* ...and for the drivers/sound files... */

#define IOC_IN		(_IOC_WRITE << _IOC_DIRSHIFT)
#define IOC_OUT		(_IOC_READ << _IOC_DIRSHIFT)
#define IOC_INOUT	((_IOC_WRITE|_IOC_READ) << _IOC_DIRSHIFT)
#define IOCSIZE_MASK	(_IOC_SIZEMASK << _IOC_SIZESHIFT)
#define IOCSIZE_SHIFT	(_IOC_SIZESHIFT)
#else
#include <linux/ioctl.h>
#endif

#define IPCM_MAX_DEV_NR			8

enum handle_state {
	HANDLE_DISCONNECTED,
	HANDLE_CONNECTING,
	HANDLE_CONNECTED
};

enum handle_priority {
	HANDLE_MSG_NORMAL,
	HANDLE_MSG_PRIORITY
};

struct ipcm_handle_attr {
	int target;
	int port;
	int priority;
	int remote_ids[IPCM_MAX_DEV_NR];
};

struct ipcm_node_reset {
	int target;
	int timeout;
};

#define	K_IOC_IPCM_BASE  'M'

/* Create a new ipcm handle. A file descriptor is only used*
 * once for one ipcm handle. */
#define K_IPCM_IOC_CONNECT  \
	_IOW(K_IOC_IPCM_BASE, 1, struct ipcm_handle_attr)
#define K_IPCM_IOC_TRY_CONNECT  \
	_IOW(K_IOC_IPCM_BASE, 2, struct ipcm_handle_attr)

#define K_IPCM_IOC_CHECK  \
	_IOW(K_IOC_IPCM_BASE, 3, unsigned long)
#define K_IPCM_IOC_DISCONNECT  \
	_IOW(K_IOC_IPCM_BASE, 4, unsigned long)
#define K_IPCM_IOC_GET_LOCAL_ID \
	_IOW(K_IOC_IPCM_BASE, 5, unsigned long)
#define K_IPCM_IOC_GET_REMOTE_ID \
	_IOW(K_IOC_IPCM_BASE, 6, struct ipcm_handle_attr)
#define K_IPCM_IOC_GET_REMOTE_STS \
	_IOW(K_IOC_IPCM_BASE, 7, unsigned long)

#define K_IPCM_IOC_ATTR_INIT \
	_IOW(K_IOC_IPCM_BASE, 8, struct ipcm_handle_attr)

#define K_IPCM_IOC_NODE_RESET \
	_IOW(K_IOC_IPCM_BASE, 9, struct ipcm_node_reset)

#ifdef __cplusplus
}
#endif /* __cplusplus*/

#endif  /* __IPCM_USERDEV_H__ */
