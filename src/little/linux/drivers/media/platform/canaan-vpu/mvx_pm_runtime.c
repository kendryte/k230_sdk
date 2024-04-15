/*
 * The confidential and proprietary information contained in this file may
 * only be used by a person authorised under and to the extent permitted
 * by a subsisting licensing agreement from Arm Technology (China) Co., Ltd.
 *
 *            (C) COPYRIGHT 2021-2021 Arm Technology (China) Co., Ltd.
 *                ALL RIGHTS RESERVED
 *
 * This entire notice must be reproduced on all copies of this file
 * and copies of this file may only be made by a person if such person is
 * permitted to do so under the terms of a subsisting license agreement
 * from Arm Technology (China) Co., Ltd.
 *
 * SPDX-License-Identifier: GPL-2.0-only
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 *
 */

#include <linux/pm_runtime.h>
#include "mvx_log_group.h"

int mvx_pm_runtime_get_sync(struct device *dev)
{
#ifdef ENABLE_PM_CLK
    int ret;

    ret = pm_runtime_get_sync(dev);
    if (ret < 0)
        MVX_LOG_PRINT(&mvx_log_dev, MVX_LOG_WARNING,
                  "PM runtime get sync failed! ret=%d", ret);

    return ret;
#else /* !CONFIG_PM  */
    return 1;
#endif /* CONFIG_PM */
}

int mvx_pm_runtime_put_sync(struct device *dev)
{
#ifdef ENABLE_PM_CLK
    int ret;

    ret = pm_runtime_put_sync(dev);
    if (ret < 0)
        MVX_LOG_PRINT(&mvx_log_dev, MVX_LOG_WARNING,
                  "PM runtime put sync failed! ret=%d", ret);

    return ret;
#else /* !CONFIG_PM  */
    return 0;
#endif /* CONFIG_PM */
}
