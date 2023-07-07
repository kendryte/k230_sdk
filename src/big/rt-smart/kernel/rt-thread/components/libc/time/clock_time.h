/*
 * Copyright (c) 2006-2018, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2017-12-31     Bernard      the first version
 */

#ifndef CLOCK_TIME_H__
#define CLOCK_TIME_H__

#ifdef __cplusplus
extern "C" {
#endif

/* posix clock and timer */
#define MILLISECOND_PER_SECOND  1000UL
#define MICROSECOND_PER_SECOND  1000000UL
#define NANOSECOND_PER_SECOND   1000000000UL

#define MILLISECOND_PER_TICK    (MILLISECOND_PER_SECOND / RT_TICK_PER_SECOND)
#define MICROSECOND_PER_TICK    (MICROSECOND_PER_SECOND / RT_TICK_PER_SECOND)
#define NANOSECOND_PER_TICK     (NANOSECOND_PER_SECOND  / RT_TICK_PER_SECOND)

#ifndef CLOCK_REALTIME
#define CLOCK_REALTIME      1
#endif

#define CLOCK_CPUTIME_ID    2

#ifndef CLOCK_PROCESS_CPUTIME_ID
#define CLOCK_PROCESS_CPUTIME_ID CLOCK_CPUTIME_ID
#endif
#ifndef CLOCK_THREAD_CPUTIME_ID
#define CLOCK_THREAD_CPUTIME_ID  CLOCK_CPUTIME_ID
#endif

#ifndef CLOCK_MONOTONIC
#define CLOCK_MONOTONIC     4
#endif

#define ACTIVE 1
#define NOT_ACTIVE 0
struct timer_obj
{
    struct rt_timer timer;
    void (*sigev_notify_function)(union sigval val);
    union sigval val;
    struct timespec interval;              /* Reload value */
    struct timespec value;              /* Reload value */
    rt_uint32_t reload;                    /* Reload value in ms */
    rt_uint32_t status;
    int sigev_signo;
    pid_t pid;
};

int clock_getres  (clockid_t clockid, struct timespec *res);
int clock_gettime (clockid_t clockid, struct timespec *tp);
int clock_settime (clockid_t clockid, const struct timespec *tp);

int clock_time_to_tick(const struct timespec *time);

#ifdef __cplusplus
}
#endif

#endif
