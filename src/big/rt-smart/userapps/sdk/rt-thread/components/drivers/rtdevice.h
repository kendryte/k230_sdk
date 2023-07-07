#ifndef RT_DEVICE_H___
#define RT_DEVICE_H___

#include <rtthread.h>
#include "include/drivers/rtc.h"

struct rt_completion
{
    rt_uint32_t flag;

    struct rt_event *wait_event;
};

void rt_completion_init(struct rt_completion *completion);
rt_err_t rt_completion_wait(struct rt_completion *completion, rt_int32_t timeout);
void rt_completion_done(struct rt_completion *completion);

#endif
