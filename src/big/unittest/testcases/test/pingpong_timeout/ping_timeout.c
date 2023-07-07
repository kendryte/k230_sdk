#include <stdio.h>
#include <string.h>
#include <lwp_shm.h>
#include <rtthread.h>

#include <utest.h>

#ifdef RT_USING_USERSPACE
/**
 * With separate address spaces, we transfer the id of the shared-memory page
 * which contains the string, instead of the pointer to the string directly.
 */
rt_inline int prepare_data(void *data, size_t len)
{
    int shmid;
    void *shm_vaddr;

    /* use the current thread ID to label the shared memory */
    size_t key = (size_t) rt_thread_self();

    shmid = lwp_shmget(key, len, 1);    /* create a new shm */
    if (shmid == -1)
    {
        LOG_E("Fail to allocate a shared memory!");
        return -1;
    }

    /* get the start address of the shared memory */
    shm_vaddr = lwp_shmat(shmid, NULL);
    if (shm_vaddr == RT_NULL)
    {
        LOG_E("The allocated shared memory doesn't have a valid address!");
        lwp_shmrm(shmid);
        return -1;
    }

    /* put the data into the shared memory */
    memcpy(shm_vaddr, data, len);
    lwp_shmdt(shm_vaddr);

    return shmid;
}
#endif

void ping_timeout(void *result)
{
    int i;
    int pong_ch;

    /* the string to transfer */
    char ping_string[256] = { 0 };
    size_t len = 0;

    /* channel messages to send and return back */
    struct rt_channel_msg ch_msg, ch_msg_ret;

    /* open the IPC channel created by 'pong' */
    pong_ch = rt_channel_open("pong", 0);
    if (pong_ch == -1)
    {
        LOG_E("Error: rt_channel_open: could not find the \'pong\' channel!");
        *((int *)result) = -1;
        return;
    }

    /* try to communicate through the IPC channel */
    for (i = 0; i < 100; i++)
    {
        /* initialize the message to send */
        ch_msg.type = RT_CHANNEL_RAW;
        snprintf(ping_string, 255, "count = %d", i);
        len = strlen(ping_string) + 1;
        ping_string[len] = '\0';

#ifdef RT_USING_USERSPACE
        int shmid = prepare_data(ping_string, len);
        if (shmid < 0)
        {
            LOG_E("Ping: fail to prepare the ping message.");
            continue;
        }
        ch_msg.u.d = (void *)(size_t)shmid;
#else
        ch_msg.u.d = ping_string;
#endif

        rt_channel_send(pong_ch, &ch_msg);

        LOG_I("Ping: send %s", ping_string);
        rt_channel_send_recv_timeout(pong_ch, &ch_msg, &ch_msg_ret, 100);
        LOG_I("Ping: receive the reply %d", (int)(size_t)ch_msg_ret.u.d);

#ifdef RT_USING_USERSPACE
        lwp_shmrm(shmid);
#endif
    }

    /* get rid of the channel and the shared memory */
    rt_channel_close(pong_ch);

    *((int *)result) = 0;
}
