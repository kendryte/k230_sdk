/*
 * Copyright (c) 2006-2018, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2012-10-10     aozima       first version.
 */

#ifndef RTC_H__
#define RTC_H__

/* RTC device */
#define RT_DEVICE_CTRL_RTC_GET_TIME     0x40            /**< get time */
#define RT_DEVICE_CTRL_RTC_SET_TIME     0x41            /**< set time */
#define RT_DEVICE_CTRL_RTC_GET_ALARM    0x42            /**< get alarm */
#define RT_DEVICE_CTRL_RTC_SET_ALARM    0x43            /**< set alarm */

#endif /* __RTC_H__ */
