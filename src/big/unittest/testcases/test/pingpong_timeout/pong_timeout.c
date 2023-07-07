#include <stdio.h>
#include <lwp_shm.h>
#include <rtthread.h>

#include <utest.h>

void pong_timeout(void *result)
{
    int i;
    int pong_ch;
    struct rt_channel_msg msg_text;
    char *str;

#ifdef RT_USING_USERSPACE
    int shmid;
#endif

    pong_ch = rt_channel_open("pong", 0);
    if (pong_ch < 0)
    {
        LOG_I("Cannot open Pong channel ret %d", pong_ch);
        LOG_I("Now create Pong channel");
        /* create the IPC channel for 'pong' */
        pong_ch = rt_channel_open("pong", O_CREAT);
        if (pong_ch == -1) {
            LOG_E("Error: rt_channel_open: fail to create the IPC channel for pong!");
            *((int *)result) = -1;
            return;
        }
    }
    LOG_I("Pong: wait on the IPC channel: %d", pong_ch);

    /* respond to the the test messages from 'ping' */
    for (i = 0; i < 100; i++)
    {
        int ret;

        ret = rt_channel_recv_timeout(pong_ch, &msg_text, 5000);
        if (ret != RT_EOK)
        {
            if (ret == -RT_ETIMEOUT)
            {
                LOG_I("timeout!");
            }
            LOG_I("ret err:%x", ret);
            continue;
        }

        rt_channel_recv_timeout(pong_ch, &msg_text, 100);
#ifdef RT_USING_USERSPACE
        shmid = (int)(size_t)msg_text.u.d;
        if (shmid < 0 || !(str = (char *)lwp_shmat(shmid, NULL)))
        {
            msg_text.u.d = (void *)-1;
            LOG_E("Pong: receive an invalid shared-memory page.");
            rt_channel_reply(pong_ch, &msg_text);   /* send back -1 */
            continue;
        }

        LOG_I("Pong: receive %s", str);
        lwp_shmdt(str);
#else
        str = (char *)msg_test.u.d;
        LOG_I("Pong: receive %s", str);
#endif

        /* prepare the reply message */
        LOG_I("Pong: reply count = %d", i);
        msg_text.type = RT_CHANNEL_RAW;
        msg_text.u.d = (void *)(size_t)i;
        rt_thread_mdelay(i);
        rt_channel_reply(pong_ch, &msg_text);
    }

    *((int *)result) = 0;
    rt_channel_close(pong_ch);
}
