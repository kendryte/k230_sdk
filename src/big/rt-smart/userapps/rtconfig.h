#ifndef RT_CONFIG_H__
#define RT_CONFIG_H__

/* DO NOT EDIT. */
/* RT-Thread Project Configuration */

/* RT-Thread Kernel */
#define RT_USING_LWP
#define RT_NAME_MAX 20
#define RT_ALIGN_SIZE 8
#define RT_THREAD_PRIORITY_MAX 32
#define RT_TICK_PER_SECOND 100
#define RT_DEBUG
#define RT_DEBUG_COLOR

/* Inter-Thread communication */
#define RT_USING_SEMAPHORE
#define RT_USING_MUTEX
#define RT_USING_EVENT
#define RT_USING_MAILBOX
#define RT_USING_MESSAGEQUEUE
//#define RT_USING_SIGNALS

/* Memory Management */
#define RT_USING_MEMPOOL
#define RT_USING_MEMHEAP
#define RT_USING_SMALL_MEM
#define RT_USING_MEMTRACE
#define RT_USING_HEAP

/* Kernel Device Object */
#define RT_USING_DEVICE
#define RT_USING_DEVICE_OPS
#define RT_USING_USERSPACE
#define RT_USING_CACHE
#define RT_USING_CONSOLE

/* Command shell */

#define RT_USING_FINSH
#define FINSH_THREAD_NAME "tshell"
#define FINSH_USING_HISTORY
#define FINSH_HISTORY_LINES 5
#define FINSH_USING_SYMTAB
#define FINSH_USING_DESCRIPTION
#define FINSH_THREAD_PRIORITY 20
#define FINSH_THREAD_STACK_SIZE 16384
#define FINSH_CMD_SIZE 80
#define FINSH_USING_MSH
#define FINSH_USING_MSH_DEFAULT
#define FINSH_USING_MSH_ONLY
#define FINSH_ARG_MAX 10

/* Device virtual file system */

#define RT_USING_DFS
#define DFS_USING_WORKDIR
#define RT_USING_DFS_DEVFS
#define DFS_FD_MAX  16

/* Device Drivers */

#define RT_USING_DEVICE_IPC

/* POSIX layer and C standard library */

#define RT_USING_LIBC
#define RT_USING_MUSL
#define RT_USING_MUSL_LIBC
#define RT_USING_POSIX

/* Network */
#define RT_USING_NETDEV

/* Socket abstraction layer */

#define RT_USING_SAL

/* protocol stack implement */

#define SAL_USING_UNET
#define SAL_USING_POSIX

#endif
