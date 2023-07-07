/*
 * Copyright (c) 2006-2022, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2022-10-10     RT-Thread    the first version
 */

#ifndef __CONTEXT_H__
#define __CONTEXT_H__

void rt_hw_vector_ctx_save(void *buf);
void rt_hw_vector_ctx_restore(void *buf);

void rt_hw_disable_vector();
void rt_hw_enable_vector();

#endif /* __CONTEXT_H__ */