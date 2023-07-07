/*
 * Copyright (c) 2020-2022 Chengdu Aich Technology Co.,Ltd.All rights reserved.
 * Description: oam main implementation.(non-rom).
 * Author: CompanyName
 * Create: 2021-08-04
 */

/* 头文件包含 */
#include "oam_main.h"
#include "soc_types_base.h"
#include "securec.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

/* 全局变量定义 */
static struct kobject *g_sysfs_soc_oam = TD_NULL;
/* new fix: */
//td_u32 g_level_log = 2;
td_u32 g_level_log = OAM_LOG_LEVEL_ERROR;

/* 函数实现 */
td_u32 oam_get_log_level(td_void)
{
    return g_level_log;
}

/*****************************************************************************
 功能描述  : OAM模块初始化总入口，包含OAM模块内部所有特性的初始化。
 返 回 值  : 初始化返回值，成功或失败原因
*****************************************************************************/
static ssize_t log_level_show(struct kobject *kobj,
                              struct kobj_attribute *attr, char *buf)
{
    if (buf == TD_NULL) {
        return -EXT_FAILURE;
    }

    return snprintf_s(buf, PAGE_SIZE, PAGE_SIZE - 1, "loglevel:             \n"
                     " 0    close log           \n"
                     " 1    ERROR               \n"
                     " 2    WARN                \n"
                     " 3    INFO                \n");
}

STATIC ssize_t store_log_level_set(void *dev, struct kobj_attribute *attr, const char *buf, size_t count)
{
    td_s32 input;
    if (buf == TD_NULL) {
        return -EXT_FAILURE;
    }

    input = oal_atoi(buf);
    if (input < 0 || input > 5) {    /* input must range [0 5] */
        return -EXT_FAILURE;
    }

    g_level_log = (td_u32)input;
    return count;
}

STATIC struct kobj_attribute g_oam_host_log_attr =
__ATTR(loglevel, 0664, (void *)log_level_show, (void *)store_log_level_set);    /* mode 0664 */

static struct attribute *g_oam_log_attrs[] = {
    &g_oam_host_log_attr.attr,
#ifdef _SDIO_TEST
    &oam_sdio_test_attr.attr,
#endif
    NULL
};

static struct attribute_group g_oam_state_group = {
    .attrs = g_oam_log_attrs,
};

td_s32 oam_user_ctrl_init(void)
{
    td_s32 ret;
    g_sysfs_soc_oam = kobject_create_and_add("AIW4201_debug", TD_NULL);
    if (g_sysfs_soc_oam == TD_NULL) {
        printk("kobject_create_and_add fail!ret=%d", -ENOMEM);
        return -ENOMEM;
    }

    ret = sysfs_create_group(g_sysfs_soc_oam, &g_oam_state_group);
    if (ret) {
        printk("sysfs_create_group fail!ret=%d", ret);
    }
    return ret;
}

static td_s32 oam_user_ctrl_exit(td_void)
{
    if (g_sysfs_soc_oam) {
        sysfs_remove_group(g_sysfs_soc_oam, &g_oam_state_group);
        kobject_put(g_sysfs_soc_oam);
    }
    return EXT_SUCCESS;
}

td_s32 oam_main_init(td_void)
{
    td_s32 ret = oam_user_ctrl_init();
    if (ret != EXT_SUCCESS) {
        return ret;
    }
    oam_warning_log0("oam_main_init SUCCESSFULLY");
    return EXT_SUCCESS;
}

/*****************************************************************************
 功能描述  : OAM模块卸载
 返 回 值  : 模块卸载返回值，成功或失败原因
*****************************************************************************/
td_void oam_main_exit(td_void)
{
    td_s32 ret = oam_user_ctrl_exit();
    if (ret != EXT_SUCCESS) {
        oam_warning_log0("oam_main_exit:: oam_user_ctrl_exit fail!");
    }
    return;
}

#ifdef __cplusplus
#if __cplusplus
    }
#endif
#endif
