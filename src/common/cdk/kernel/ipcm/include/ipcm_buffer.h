#ifndef __IPCM_BUFFER_H__
#define __IPCM_BUFFER_H__

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus*/

#include "ipcm_funcs.h"

/* for linux kernel */
#ifdef __KERNEL__
#include "asm/page.h"
#endif

#define __DATA_ALIGN(end, align) (((unsigned int)end+align-1)&(~(align-1)))
#define __MSG_ALIGNED(end)       __DATA_ALIGN(end, 0x10)
#define __MSG_HEAD_SIZE          sizeof(struct ipcm_transfer_head)
#define __MSG_JUMP_MARK__        0x166

#define MAX_TRANSFER_LEN 1048575
#ifndef PAGE_SHIFT
#define PAGE_SHIFT	12
#endif
#ifndef PAGE_SIZE
#define PAGE_SIZE	0x1000
#endif

#define ZONE_MAGIC	0x89765000

struct ipcm_transfer_head {
    volatile unsigned int target:6;
    volatile unsigned int source:6;
    volatile unsigned int port:10;
    volatile unsigned int type:10;
    volatile unsigned int len:20;
    volatile unsigned int reserve:12;
};

struct ipcm_transfer_handle {
    unsigned int target;
    unsigned int port;
    recv_notifier recv;
    unsigned long data;
    int state;
    unsigned int priority;

    ipcm_atomic_t send_count;
    ipcm_atomic_t recv_count;
    ipcm_atomic_t max_send_len;
    ipcm_atomic_t max_recv_len;
    struct ipcm_lock lock;
    struct ipcm_event connect_event;
};

struct mem_region {
    unsigned long base;
    unsigned int size;
};

struct ipcm_shared_area {
    struct mem_region region;
    volatile char *data;
    unsigned int len;
    volatile unsigned int *rp;
    volatile unsigned int *wp;
    struct ipcm_lock lock;
};

struct ipcm_shared_zone {
    struct mem_region region;
    struct ipcm_shared_area prio_area;
    struct ipcm_shared_area normal_area;
};

int ipcm_send_message(struct ipcm_transfer_handle *handle,const void *buf, unsigned int len, unsigned int flag);
int ipcm_shared_zone_init(int nid, unsigned long base, unsigned long size, int sendbuf);
void ipcm_shared_zone_release(int nid, int sendbuf);
int get_one_message(struct ipcm_node *node, int *port, char *buf, int len);

#ifdef __cplusplus
}
#endif /* __cplusplus*/

#endif
