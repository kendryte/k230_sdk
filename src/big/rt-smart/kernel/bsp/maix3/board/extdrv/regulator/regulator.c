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

#include <rtthread.h>
#include "regulator.h"
#define DBG_TAG  "regulator"
#ifdef RT_DEBUG
#define DBG_LVL          DBG_LOG
#else
#define DBG_LVL          DBG_WARNING
#endif
#define DBG_COLOR
#include <rtdbg.h>
#include "encoding.h"

static int regulator_init(struct regulator_dev *pdev)
{
    int ret = 0;

    if (pdev && pdev->ops->init) {
        rt_mutex_take(&pdev->mutex, RT_WAITING_FOREVER);
        ret = pdev->ops->init(pdev);
        rt_mutex_release(&pdev->mutex);
    }

    return ret;
}

static int regulator_deinit(struct regulator_dev *pdev)
{
    int ret = 0;

    if (pdev && pdev->ops->deinit) {
        rt_mutex_take(&pdev->mutex, RT_WAITING_FOREVER);
        ret = pdev->ops->deinit(pdev);
        rt_mutex_release(&pdev->mutex);
    }

    return ret;
}

int regulator_enable(struct regulator_dev *pdev, bool enable)
{
    int ret = 0;

    if (pdev && pdev->ops->enable) {
        rt_mutex_take(&pdev->mutex, RT_WAITING_FOREVER);
        ret = pdev->ops->enable(pdev, enable);
        rt_mutex_release(&pdev->mutex);
    }

    return ret;
}

int regulator_set_voltage(struct regulator_dev *pdev, int vol_uv)
{
    int ret = 0;

    if (pdev && pdev->ops->set_voltage) {
        rt_mutex_take(&pdev->mutex, RT_WAITING_FOREVER);
        ret = pdev->ops->set_voltage(pdev, vol_uv);
        rt_mutex_release(&pdev->mutex);
    }

    return ret;
}

int regulator_get_voltage(struct regulator_dev *pdev, int *pvol_uv)
{
    int ret = 0;

    if (pdev && pdev->ops->get_voltage) {
        rt_mutex_take(&pdev->mutex, RT_WAITING_FOREVER);
        ret = pdev->ops->get_voltage(pdev, pvol_uv);
        rt_mutex_release(&pdev->mutex);
    }

    return ret;
}

int regulator_dev_register(struct regulator_dev *pdev,
        struct regulator_ops *ops, const char *name)
{
    int ret;

    pdev->ops = ops;
    ret = rt_device_register(&pdev->parent, name, RT_DEVICE_FLAG_RDWR);

    if (ret) {
        LOG_E("regulator register fail: %d\n", ret);
    }

    if (ret == RT_EOK)
        ret = rt_mutex_init(&pdev->mutex, name, RT_IPC_FLAG_PRIO);

    ret = regulator_init(pdev);

    return ret;
}

int regulator_debug(int argc, char **argv)
{
    if (argc < 3) {
        rt_kprintf("\nUsage:\t%s dev_name op(e|s|g|t) [data]\n"
            "\tdev_name: regulator name\n"
            "\top      : e(enable) s(set) g(get) t(test)\n"
            "\tdata    : set voltage uV\n\n",
            argv[0]);
        return -1;
    }

    char *dev_name = argv[1];
    char op = argv[2][0];
    int op_data;
    int start_volt, stop_volt, step_volt, delay;

    if (op == 's') {
        if (argc < 4) {
            rt_kprintf("please input voltage\n");
            return -1;
        }
        op_data = strtol(argv[3], 0, 0);
    } else if (op == 'e') {
        if (argc < 4) {
            rt_kprintf("please input 0|1\n");
            return -1;
        }
        op_data = strtol(argv[3], 0, 0);
    } else if (op == 't') {
        if (argc < 7) {
            rt_kprintf("please input start_volt stop_volt step_volt "
                "step_delay\n");
            return -1;
        }
        start_volt = strtol(argv[3], 0, 0);
        stop_volt = strtol(argv[4], 0, 0);
        step_volt = strtol(argv[5], 0, 0);
        delay = strtol(argv[6], 0, 0);
        if (step_volt <= 0) {
            op = 's';
            op_data = start_volt;
        }
    }

    struct regulator_dev *pdev;
    pdev = (struct regulator_dev *)rt_device_find(dev_name);
    if (pdev == RT_NULL) {
        rt_kprintf("Can't find %s device\n", dev_name);
        return -1;
    }

    int ret;

    switch (op) {
        case 'e':
            ret = regulator_enable(pdev, (bool)op_data);
            break;
        case 's':
            ret = regulator_set_voltage(pdev, op_data);
            break;
        case 'g':
            ret = regulator_get_voltage(pdev, &op_data);
            rt_kprintf("current voltage: %d uV\n", op_data);
            break;
        case 't':
            if (start_volt > stop_volt) {
                for (int volt = start_volt; volt >= stop_volt;
                    volt -= step_volt) {
                    regulator_set_voltage(pdev, volt);
                    regulator_get_voltage(pdev, &op_data);
                    rt_kprintf("current voltage: %d uV\n", op_data);
                    rt_thread_mdelay(delay);
                }
            } else {
                for (int volt = start_volt; volt <= stop_volt;
                    volt += step_volt) {
                    regulator_set_voltage(pdev, volt);
                    regulator_get_voltage(pdev, &op_data);
                    rt_kprintf("current voltage: %d uV\n", op_data);
                    rt_thread_mdelay(delay);
                }
            }
            break;
        default:
            rt_kprintf("invalid op '%c'\n", op);
            return -1;
    }

    if (ret != 0) {
        rt_kprintf("op '%c' fail\n", op);
        return -1;
    }

    return 0;
}

MSH_CMD_EXPORT(regulator_debug, regulator debug program);
