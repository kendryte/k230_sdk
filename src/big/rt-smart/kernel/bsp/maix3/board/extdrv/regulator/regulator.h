/* Copyright (c) 2023, Canaan Bright Sight Co., Ltd
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 * 1. Redistributions of source code must retain the above copyright
 * notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 * notice, this list of conditions and the following disclaimer in the
 * documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND
 * CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES,
 * INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 * BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef __DRV_REGULATOR_H__
#define __DRV_REGULATOR_H__

#include <stdint.h>
#include <stdbool.h>
#include <rtdevice.h>

struct regulator_ops;

struct regulator_dev {
    struct rt_device parent;
    struct rt_mutex mutex;
    struct regulator_ops *ops;
};

struct regulator_ops {
    int (*init)(struct regulator_dev *pdev);
    int (*deinit)(struct regulator_dev *pdev);
    int (*enable)(struct regulator_dev *pdev, bool enable);
    int (*set_voltage)(struct regulator_dev *pdev, int vol_uv);
    int (*get_voltage)(struct regulator_dev *pdev, int *pvol_uv);
};

int regulator_dev_register(struct regulator_dev *pdev,
        struct regulator_ops *ops, const char *name);
int regulator_enable(struct regulator_dev *pdev, bool enable);
int regulator_set_voltage(struct regulator_dev *pdev, int vol_uv);
int regulator_get_voltage(struct regulator_dev *pdev, int *pvol_uv);

#endif /* __DRV_REGULATOR_H__ */
