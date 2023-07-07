/**
 * @file usage.h
 * @author  ()
 * @brief
 * @version 1.0
 * @date 2022-11-18
 *
 * @copyright Copyright (c) 2022 Canaan Inc.
 *
 */

#ifndef __USAGE_H__
#define __USAGE_H__

#include <rtthread.h>

rt_uint8_t sys_cpu_usage(rt_uint8_t cpu_id);
rt_uint8_t sys_thread_usage(rt_thread_t thread);
rt_uint8_t sys_process_usage(int pid);
rt_err_t sys_set_usage_period(int mill_sec);

#endif /* __DRV_UART_H__ */
