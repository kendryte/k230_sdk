// SPDX-License-Identifier: GPL-2.0-or-later
/* Copyright (c) 2022, Canaan Bright Sight Co., Ltd
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
*/

#include <linux/module.h>
#include <linux/init.h>
#include <linux/platform_device.h>
#include <linux/input.h>
#include <linux/of.h>
#include <linux/of_irq.h>
#include <linux/of_device.h>
#include <dt-bindings/interrupt-controller/irq.h>

// k230 pmu regs
#define PMU_LOGS		0x00
#define PMU_STATUS		0x3C
#define INT2OUTPUT0_EN		0x40
#define INT2OUTPUT1_EN		0x44
#define INT2CPU_EN		0x48
#define INT_EN			0x4C
#define INT_TYPE		0x50
#define INT_CLEAR		0x54
#define INT0_0_TRIGGER_VAL	0x58
#define INT8_TRIGGER_VAL	0x5C
#define INT0_1_TRIGGER_VAL	0x60
#define INT0_DEBOUNCE_VAL	0x64
#define INT1_0_EDGE_VAL		0x68
#define INT1_DEBOUNCE_VAL	0x6C
#define INT2_DEBOUNCE_VAL	0x70
#define INT3_DEBOUNCE_VAL	0x74
#define OUTPUT_REG_CTL		0x78
#define OUTPUT_DELAY_CTL	0x7C
#define IO64_CFG		0x80
#define IO65_CFG		0x84
#define IO66_CFG		0x88
#define IO67_CFG		0x8C
#define IO68_CFG		0x90
#define IO69_CFG		0x94
#define IO70_CFG		0x98
#define IO71_CFG		0x9C
#define ISO_STABLE_VAL		0xA0
#define OUTPUT0_CTL		0xA4
#define OUTPUT1_CTL		0xA8
#define INT_STATUS		0xAC

// int2output0, int2output1, int2cpu
#define INT2X_INT0_0_EN_MASK	0x800
#define INT2X_INT0_1_EN_MASK	0x400
#define INT2X_INT0_2_EN_MASK	0x200
#define INT2X_INT1_0_EN_MASK	0x100
#define INT2X_INT1_1_EN_MASK	0x080
#define INT2X_INT2_EN_MASK	0x040
#define INT2X_INT3_EN_MASK	0x020
#define INT2X_INT4_EN_MASK	0x010
#define INT2X_INT5_EN_MASK	0x008
#define INT2X_RTC_ALARM_EN_MASK	0x004
#define INT2X_RTC_TICK_EN_MASK	0x002
#define INT2X_INT8_EN_MASK	0x001

// int en
#define INT8_EN_MASK		0x1000
#define INT0_0_EN_MASK		0x800
#define INT0_1_EN_MASK		0x400
#define INT0_2_EN_MASK		0x200
#define INT1_0_EN_MASK		0x100
#define INT1_1_EN_MASK		0x080
#define INT2_EN_MASK		0x040
#define INT3_EN_MASK		0x020
#define INT4_EN_MASK		0x010
#define INT5_EN_MASK		0x008
#define RTC_ALARM_EN_MASK	0x004
#define RTC_TICK_EN_MASK	0x002
#define INT1_0_TICK_EN_MASK	0x001

// int type
#define INT_TRIGGER_MASK	0x7
#define INT_TRIGGER_TYPE_MASK	0x1
#define INT_TRIGGER_EDGE_MASK	0x2
#define INT_TRIGGER_LEVEL_MASK	0x4
#define INT0_0_LEVEL_MASK	0x100000
#define INT0_1_LEVEL_MASK	0x80000
#define INT0_2_TRIGGER_OFFSET	16
#define INT1_0_EDGE_MASK	0x8000
#define INT1_1_TRIGGER_OFFSET	12
#define INT2_TRIGGER_OFFSET	9
#define INT3_TRIGGER_OFFSET	6
#define INT4_TRIGGER_OFFSET	3
#define INT5_TRIGGER_OFFSET	0

// int clear
#define INT_CLR_ALL		0x3FF
#define INT_CLR_ALL_E8		0x1FF
#define INT_CLR_INT8		0x200
#define INT_CLR_INT0_0		0x100
#define INT_CLR_INT0_1		0x080
#define INT_CLR_INT0_2		0x040
#define INT_CLR_INT1_0		0x020
#define INT_CLR_INT1_1		0x010
#define INT_CLR_INT2		0x008
#define INT_CLR_INT3		0x004
#define INT_CLR_INT4		0x002
#define INT_CLR_INT5		0x001

// pmu attr used
#define PMU_BIN_ATTR_LOGS	0
#define PMU_BIN_ATTR_REGS	1
#define PMU_BIN_ATTR_LOGS_SIZE	0x3C
#define PMU_BIN_ATTR_REGS_SIZE	0x70

#define ALL_WAKEUP_MASK		0x9FE
#define INT0_WAKEUP_MASK	0x800
#define INT1_WAKEUP_MASK	0x180
#define INT2_WAKEUP_MASK	0x040
#define INT3_WAKEUP_MASK	0x020
#define INT4_WAKEUP_MASK	0x010
#define INT5_WAKEUP_MASK	0x008
#define RTC_ALARM_WAKEUP_MASK	0x004
#define RTC_TICK_WAKEUP_MASK	0x002

enum {
	INT8_TRIGGER_VALUE,
	INT0_0_TRIGGER_VALUE,
	INT0_DEBOUNCE_VALUE,
	INT0_1_WAKEUP,
	INT0_1_EVENT,
	INT0_2_TYPE,
	INT0_2_WAKEUP,
	INT0_2_EVENT,
	INT1_DEBOUNCE_VALUE,
	INT1_0_EDGE_VALUE,
	INT1_0_TYPE,
	INT1_0_WAKEUP,
	INT1_0_EVENT,
	INT1_1_TYPE,
	INT1_1_WAKEUP,
	INT1_1_EVENT,
	INT2_DEBOUNCE_VALUE,
	INT2_TYPE,
	INT2_WAKEUP,
	INT2_EVENT,
	INT3_DEBOUNCE_VALUE,
	INT3_TYPE,
	INT3_WAKEUP,
	INT3_EVENT,
	INT4_TYPE,
	INT4_WAKEUP,
	INT4_EVENT,
	INT5_TYPE,
	INT5_WAKEUP,
	INT5_EVENT,
	RTC_ALARM_WAKEUP,
	RTC_ALARM_EVENT,
	RTC_TICK_WAKEUP,
	RTC_TICK_EVENT,
};

enum _pmu_status {
	PMU_RESET,
	SOC_FORCE_PD,
	SOC_NORMAL_PD,
	SOC_RESET,
};

struct _k230_pmu_dev {
	void __iomem *regs;
	struct input_dev *input;
	struct mutex mutex;
	unsigned int pmu_status;
	unsigned int wakeup_source;
};

static struct _k230_pmu_dev *k230_pmu_dev;

static ssize_t pmu_status_show(struct device *dev,
		struct device_attribute *attr, char *buf)
{
	unsigned int pmu_status = k230_pmu_dev->pmu_status;

	return sysfs_emit(buf, "%s\n", pmu_status == PMU_RESET ? "PMU_RESET" :
		pmu_status == SOC_FORCE_PD ? "SOC_FORCE_PD" : 
		pmu_status == SOC_NORMAL_PD ? "SOC_NORMAL_PD" : "SOC_RESET");
}

static ssize_t wakeup_source_show(struct device *dev,
		struct device_attribute *attr, char *buf)
{
	char tmp[64];
	unsigned int wakeup_source = k230_pmu_dev->wakeup_source;

	if (wakeup_source & ALL_WAKEUP_MASK)
		sprintf(tmp, "%s%s%s%s%s%s%s%s",
		wakeup_source & INT0_WAKEUP_MASK ? "INT0," : "",
		wakeup_source & INT1_WAKEUP_MASK ? "INT1," : "",
		wakeup_source & INT2_WAKEUP_MASK ? "INT2," : "",
		wakeup_source & INT3_WAKEUP_MASK ? "INT3," : "",
		wakeup_source & INT4_WAKEUP_MASK ? "INT4," : "",
		wakeup_source & INT5_WAKEUP_MASK ? "INT5," : "",
		wakeup_source & RTC_ALARM_WAKEUP_MASK ? "RTC_ALARM," : "",
		wakeup_source & RTC_TICK_WAKEUP_MASK ? "RTC_TICK," : "");
	else
		sprintf(tmp, ",");

	return sysfs_emit(buf, "%.*s\n", (int)strlen(tmp) - 1, tmp);
}

static ssize_t pmu_bin_read(struct file *filp, struct kobject *kobj,
			struct bin_attribute *attr, char *buf,
			loff_t off, size_t count)
{
	int i = off / 4, cnt = count / 4;
	unsigned int *src = k230_pmu_dev->regs;
	unsigned int *dst = (unsigned int *)buf; // maybe non-aligned
	int bin_attr_type = (int)attr->private;

	mutex_lock(&k230_pmu_dev->mutex);
	if (bin_attr_type == PMU_BIN_ATTR_LOGS) {
		for (; i < PMU_BIN_ATTR_LOGS_SIZE / 4 && cnt; i++, cnt--)
			*dst++ = readl(src++);
	} else if (bin_attr_type == PMU_BIN_ATTR_REGS) {
		src += INT2OUTPUT0_EN / 4;
		for (; i < PMU_BIN_ATTR_REGS_SIZE / 4 && cnt; i++, cnt--)
			*dst++ = readl(src++);
	}
	mutex_unlock(&k230_pmu_dev->mutex);

	return count;
}

static ssize_t pmu_bin_write(struct file *filp, struct kobject *kobj,
			struct bin_attribute *attr, char *buf,
			loff_t off, size_t count)
{
	int i = off / 4, cnt = count / 4;
	unsigned int *src = (unsigned int *)buf; // maybe non-aligned
	unsigned int *dst = k230_pmu_dev->regs;
	int bin_attr_type = (int)attr->private;

	mutex_lock(&k230_pmu_dev->mutex);
	if (bin_attr_type == PMU_BIN_ATTR_LOGS) {
		for (; i < PMU_BIN_ATTR_LOGS_SIZE / 4 && cnt; i++, cnt--)
			writel(*src++, dst++);
	} else if (bin_attr_type == PMU_BIN_ATTR_REGS) {
		dst += INT2OUTPUT0_EN / 4;
		for (; i < PMU_BIN_ATTR_REGS_SIZE / 4 && cnt; i++, cnt--)
			writel(*src++, dst++);
	}
	mutex_unlock(&k230_pmu_dev->mutex);

	return count;
}

static DEVICE_ATTR_RO(pmu_status);
static DEVICE_ATTR_RO(wakeup_source);

#define PMU_BIN_ATTR(_name, _size, _private)				\
struct bin_attribute bin_attr_##_name = {				\
	.attr = { .name = __stringify(_name), .mode = 0600 },		\
	.read = pmu_bin_read,						\
	.write = pmu_bin_write,						\
	.size = _size,							\
	.private = (void *)_private,					\
}

static PMU_BIN_ATTR(pmu_logs, PMU_BIN_ATTR_LOGS_SIZE, PMU_BIN_ATTR_LOGS);
static PMU_BIN_ATTR(pmu_regs, PMU_BIN_ATTR_REGS_SIZE, PMU_BIN_ATTR_REGS);

static struct attribute *pmu_attrs[] = {
	&dev_attr_pmu_status.attr,
	&dev_attr_wakeup_source.attr,
	NULL,
};
static struct bin_attribute *pmu_bin_attrs[] = {
	&bin_attr_pmu_logs,
	&bin_attr_pmu_regs,
	NULL,
};
static struct attribute_group pmu_group = {
	.name = "pmu",
	.attrs = pmu_attrs,
	.bin_attrs = pmu_bin_attrs,
};

static void _inttype_show(char *buf, void *base, int mask_offset)
{
	unsigned int value;

	value = readl(base + INT_TYPE);
	value >>= mask_offset;
	value &= INT_TRIGGER_MASK;
	sprintf(buf, value & INT_TRIGGER_TYPE_MASK ?
		value & INT_TRIGGER_EDGE_MASK ? "falling" : "rising" :
		value & INT_TRIGGER_LEVEL_MASK ? "low" : "high");
}

static int _inttype_store(char *buf, void *base, int mask_offset)
{
	int ret = 0;
	unsigned int value;

	if (sysfs_streq("rising", buf))
		value = INT_TRIGGER_TYPE_MASK;
	else if (sysfs_streq("falling", buf))
		value = INT_TRIGGER_TYPE_MASK | INT_TRIGGER_EDGE_MASK;
	else if (sysfs_streq("low", buf))
		value = INT_TRIGGER_LEVEL_MASK;
	else if (sysfs_streq("high", buf))
		value = 0;
	else
		ret = -EINVAL;
	if (ret == 0) {
		value <<= mask_offset;
		value |= (readl(base + INT_TYPE) &
			~(INT_TRIGGER_MASK << mask_offset));
		writel(value, base + INT_TYPE);
	}

	return ret;
}

static int _edgetype_store(char *buf, void *base, int mask)
{
	int ret = 0;
	unsigned int value;

	if (sysfs_streq("rising", buf))
		value = 0;
	else if (sysfs_streq("falling", buf))
		value = mask;
	else
		ret = -EINVAL;
	if (ret == 0) {
		value |= (readl(base + INT_TYPE) & ~mask);
		writel(value, base + INT_TYPE);
	}

	return ret;
}

static int _inten_store(char *buf, void *base, int reg, int mask)
{
	int ret = 0;
	unsigned int value;

	if (sysfs_streq("enabled", buf))
		value = mask;
	else if (sysfs_streq("disabled", buf))
		value = 0;
	else
		ret = -EINVAL;
	if (ret == 0) {
		value |= (readl(base + reg) & ~mask);
		writel(value, base + reg);
		value = readl(base + INT_EN) & ~mask;
		value |= (readl(base + INT2OUTPUT0_EN) |
			readl(base + INT2CPU_EN) & mask);
		writel(value, base + INT_EN);
	}

	return ret;
}

#define UINT_SHOW_CASE(_case_value, _reg_offset)			\
	case _case_value:						\
		sprintf(tmp, "%u", readl(base + _reg_offset));		\
	break

#define INTTYPE_SHOW_CASE(_case_value, _mask_offset)			\
	case _case_value:						\
		_inttype_show(tmp, base, _mask_offset);			\
	break

#define EDGETYPE_SHOW_CASE(_case_value, _mask)				\
	case _case_value:						\
		sprintf(tmp, readl(base + INT_TYPE) & _mask ?		\
			"falling" : "rising");				\
	break

#define INTEN_SHOW_CASE(_case_value, _reg_offset, _mask)		\
	case _case_value:						\
		sprintf(tmp, readl(base + _reg_offset) & _mask ?	\
			"enabled" : "disabled");			\
	break

#define UINT_STORE_CASE(_case_value, _reg_offset)			\
	case _case_value:						\
		ret = kstrtouint(buf, 0, &value);			\
		if (ret == 0)						\
			writel(value, base + _reg_offset);		\
	break

#define INTTYPE_STORE_CASE(_case_value, _mask_offset)			\
	case _case_value:						\
		ret = _inttype_store(buf, base, _mask_offset);		\
	break

#define EDGETYPE_STORE_CASE(_case_value, _mask)				\
	case _case_value:						\
		ret = _edgetype_store(buf, base, _mask);		\
	break

#define INTEN_STORE_CASE(_case_value, _reg_offset, _mask)		\
	case _case_value:						\
		ret = _inten_store(buf, base, _reg_offset, _mask);	\
	break

#define to_ext_attr(x) container_of(x, struct dev_ext_attribute, attr)

static ssize_t pmu_attr_show(struct device *dev,
		struct device_attribute *attr, char *buf)
{
	char tmp[16];
	struct dev_ext_attribute *ea = to_ext_attr(attr);
	void *base = k230_pmu_dev->regs;

	mutex_lock(&k230_pmu_dev->mutex);
	switch ((int)ea->var) {
	UINT_SHOW_CASE(INT8_TRIGGER_VALUE, INT8_TRIGGER_VAL);
	UINT_SHOW_CASE(INT0_0_TRIGGER_VALUE, INT0_0_TRIGGER_VAL);
	UINT_SHOW_CASE(INT0_DEBOUNCE_VALUE, INT0_DEBOUNCE_VAL);
	INTTYPE_SHOW_CASE(INT0_2_TYPE, INT0_2_TRIGGER_OFFSET);
	INTEN_SHOW_CASE(INT0_2_WAKEUP, INT2OUTPUT0_EN, INT2X_INT0_2_EN_MASK);
	INTEN_SHOW_CASE(INT0_2_EVENT, INT2CPU_EN, INT2X_INT0_2_EN_MASK);
	UINT_SHOW_CASE(INT1_DEBOUNCE_VALUE, INT1_DEBOUNCE_VAL);
	UINT_SHOW_CASE(INT1_0_EDGE_VALUE, INT1_0_EDGE_VAL);
	EDGETYPE_SHOW_CASE(INT1_0_TYPE, INT1_0_EDGE_MASK);
	INTEN_SHOW_CASE(INT1_0_WAKEUP, INT2OUTPUT0_EN, INT2X_INT1_0_EN_MASK);
	INTEN_SHOW_CASE(INT1_0_EVENT, INT2CPU_EN, INT2X_INT1_0_EN_MASK);
	INTTYPE_SHOW_CASE(INT1_1_TYPE, INT1_1_TRIGGER_OFFSET);
	INTEN_SHOW_CASE(INT1_1_WAKEUP, INT2OUTPUT0_EN, INT2X_INT1_1_EN_MASK);
	INTEN_SHOW_CASE(INT1_1_EVENT, INT2CPU_EN, INT2X_INT1_1_EN_MASK);
	UINT_SHOW_CASE(INT2_DEBOUNCE_VALUE, INT2_DEBOUNCE_VAL);
	INTTYPE_SHOW_CASE(INT2_TYPE, INT2_TRIGGER_OFFSET);
	INTEN_SHOW_CASE(INT2_WAKEUP, INT2OUTPUT0_EN, INT2X_INT2_EN_MASK);
	INTEN_SHOW_CASE(INT2_EVENT, INT2CPU_EN, INT2X_INT2_EN_MASK);
	UINT_SHOW_CASE(INT3_DEBOUNCE_VALUE, INT3_DEBOUNCE_VAL);
	INTTYPE_SHOW_CASE(INT3_TYPE, INT3_TRIGGER_OFFSET);
	INTEN_SHOW_CASE(INT3_WAKEUP, INT2OUTPUT0_EN, INT2X_INT3_EN_MASK);
	INTEN_SHOW_CASE(INT3_EVENT, INT2CPU_EN, INT2X_INT3_EN_MASK);
	INTTYPE_SHOW_CASE(INT4_TYPE, INT4_TRIGGER_OFFSET);
	INTEN_SHOW_CASE(INT4_WAKEUP, INT2OUTPUT0_EN, INT2X_INT4_EN_MASK);
	INTEN_SHOW_CASE(INT4_EVENT, INT2CPU_EN, INT2X_INT4_EN_MASK);
	INTTYPE_SHOW_CASE(INT5_TYPE, INT5_TRIGGER_OFFSET);
	INTEN_SHOW_CASE(INT5_WAKEUP, INT2OUTPUT0_EN, INT2X_INT5_EN_MASK);
	INTEN_SHOW_CASE(INT5_EVENT, INT2CPU_EN, INT2X_INT5_EN_MASK);
	INTEN_SHOW_CASE(RTC_ALARM_WAKEUP, INT2OUTPUT0_EN,
		INT2X_RTC_ALARM_EN_MASK);
	INTEN_SHOW_CASE(RTC_ALARM_EVENT, INT2CPU_EN, INT2X_RTC_ALARM_EN_MASK);
	INTEN_SHOW_CASE(RTC_TICK_WAKEUP, INT2OUTPUT0_EN,
		INT2X_RTC_TICK_EN_MASK);
	INTEN_SHOW_CASE(RTC_TICK_EVENT, INT2CPU_EN, INT2X_RTC_TICK_EN_MASK);
	default:
	break;
	}
	mutex_unlock(&k230_pmu_dev->mutex);

	return sysfs_emit(buf, "%s\n", tmp);
}

static ssize_t pmu_attr_store(struct device *dev,
		struct device_attribute *attr, const char *buf, size_t count)
{
	int ret = 0;
	unsigned int value = 0;
	struct dev_ext_attribute *ea = to_ext_attr(attr);
	void *base = k230_pmu_dev->regs;

	mutex_lock(&k230_pmu_dev->mutex);
	switch ((int)ea->var) {
	UINT_STORE_CASE(INT8_TRIGGER_VALUE, INT8_TRIGGER_VAL);
	UINT_STORE_CASE(INT0_0_TRIGGER_VALUE, INT0_0_TRIGGER_VAL);
	UINT_STORE_CASE(INT0_DEBOUNCE_VALUE, INT0_DEBOUNCE_VAL);
	INTTYPE_STORE_CASE(INT0_2_TYPE, INT0_2_TRIGGER_OFFSET);
	INTEN_STORE_CASE(INT0_2_WAKEUP, INT2OUTPUT0_EN, INT2X_INT0_2_EN_MASK);
	INTEN_STORE_CASE(INT0_2_EVENT, INT2CPU_EN, INT2X_INT0_2_EN_MASK);
	UINT_STORE_CASE(INT1_DEBOUNCE_VALUE, INT1_DEBOUNCE_VAL);
	UINT_STORE_CASE(INT1_0_EDGE_VALUE, INT1_0_EDGE_VAL);
	EDGETYPE_STORE_CASE(INT1_0_TYPE, INT1_0_EDGE_MASK);
	INTEN_STORE_CASE(INT1_0_WAKEUP, INT2OUTPUT0_EN, INT2X_INT1_0_EN_MASK);
	INTEN_STORE_CASE(INT1_0_EVENT, INT2CPU_EN, INT2X_INT1_0_EN_MASK);
	INTTYPE_STORE_CASE(INT1_1_TYPE, INT1_1_TRIGGER_OFFSET);
	INTEN_STORE_CASE(INT1_1_WAKEUP, INT2OUTPUT0_EN, INT2X_INT1_1_EN_MASK);
	INTEN_STORE_CASE(INT1_1_EVENT, INT2CPU_EN, INT2X_INT1_1_EN_MASK);
	UINT_STORE_CASE(INT2_DEBOUNCE_VALUE, INT2_DEBOUNCE_VAL);
	INTTYPE_STORE_CASE(INT2_TYPE, INT2_TRIGGER_OFFSET);
	INTEN_STORE_CASE(INT2_WAKEUP, INT2OUTPUT0_EN, INT2X_INT2_EN_MASK);
	INTEN_STORE_CASE(INT2_EVENT, INT2CPU_EN, INT2X_INT2_EN_MASK);
	UINT_STORE_CASE(INT3_DEBOUNCE_VALUE, INT3_DEBOUNCE_VAL);
	INTTYPE_STORE_CASE(INT3_TYPE, INT3_TRIGGER_OFFSET);
	INTEN_STORE_CASE(INT3_WAKEUP, INT2OUTPUT0_EN, INT2X_INT3_EN_MASK);
	INTEN_STORE_CASE(INT3_EVENT, INT2CPU_EN, INT2X_INT3_EN_MASK);
	INTTYPE_STORE_CASE(INT4_TYPE, INT4_TRIGGER_OFFSET);
	INTEN_STORE_CASE(INT4_WAKEUP, INT2OUTPUT0_EN, INT2X_INT4_EN_MASK);
	INTEN_STORE_CASE(INT4_EVENT, INT2CPU_EN, INT2X_INT4_EN_MASK);
	INTTYPE_STORE_CASE(INT5_TYPE, INT5_TRIGGER_OFFSET);
	INTEN_STORE_CASE(INT5_WAKEUP, INT2OUTPUT0_EN, INT2X_INT5_EN_MASK);
	INTEN_STORE_CASE(INT5_EVENT, INT2CPU_EN, INT2X_INT5_EN_MASK);
	INTEN_STORE_CASE(RTC_ALARM_WAKEUP, INT2OUTPUT0_EN,
		INT2X_RTC_ALARM_EN_MASK);
	INTEN_STORE_CASE(RTC_ALARM_EVENT, INT2CPU_EN, INT2X_RTC_ALARM_EN_MASK);
	INTEN_STORE_CASE(RTC_TICK_WAKEUP, INT2OUTPUT0_EN,
		INT2X_RTC_TICK_EN_MASK);
	INTEN_STORE_CASE(RTC_TICK_EVENT, INT2CPU_EN, INT2X_RTC_TICK_EN_MASK);
	default:
	break;
	}
	mutex_unlock(&k230_pmu_dev->mutex);

	return ret ? ret : count;
}

#define PMU_ATTR(_name, _attr_name, _private)				\
struct dev_ext_attribute dev_attr_##_name = {				\
	__ATTR(_attr_name, 0644, pmu_attr_show, pmu_attr_store),	\
	(void *)_private }

static PMU_ATTR(int0_fpd_val, force_powerdown_value, INT8_TRIGGER_VALUE);
static PMU_ATTR(int0_pe_val, power_event_value, INT0_0_TRIGGER_VALUE);
static PMU_ATTR(int0_deb_val, debounce_value, INT0_DEBOUNCE_VALUE);
static PMU_ATTR(int0_2_type, type, INT0_2_TYPE);
static PMU_ATTR(int0_2_wakeup, wakeup, INT0_2_WAKEUP);
static PMU_ATTR(int0_2_event, event, INT0_2_EVENT);
static struct attribute *int0_attrs[] = {
	&dev_attr_int0_fpd_val.attr.attr,
	&dev_attr_int0_pe_val.attr.attr,
	&dev_attr_int0_deb_val.attr.attr,
	&dev_attr_int0_2_type.attr.attr,
	&dev_attr_int0_2_wakeup.attr.attr,
	&dev_attr_int0_2_event.attr.attr,
	NULL,
};
static struct attribute_group int0_group = {
	.name = "int0",
	.attrs = int0_attrs,
};

static PMU_ATTR(int1_deb_val, debounce_value, INT1_DEBOUNCE_VALUE);
static PMU_ATTR(int1_0_edge_val, edge_cnt_value, INT1_0_EDGE_VALUE);
static PMU_ATTR(int1_0_type, edge_cnt_type, INT1_0_TYPE);
static PMU_ATTR(int1_0_wakeup, edge_cnt_wakeup, INT1_0_WAKEUP);
static PMU_ATTR(int1_0_event, edge_cnt_event, INT1_0_EVENT);
static PMU_ATTR(int1_1_type, type, INT1_1_TYPE);
static PMU_ATTR(int1_1_wakeup, wakeup, INT1_1_WAKEUP);
static PMU_ATTR(int1_1_event, event, INT1_1_EVENT);
static struct attribute *int1_attrs[] = {
	&dev_attr_int1_deb_val.attr.attr,
	&dev_attr_int1_0_edge_val.attr.attr,
	&dev_attr_int1_0_type.attr.attr,
	&dev_attr_int1_0_wakeup.attr.attr,
	&dev_attr_int1_0_event.attr.attr,
	&dev_attr_int1_1_type.attr.attr,
	&dev_attr_int1_1_wakeup.attr.attr,
	&dev_attr_int1_1_event.attr.attr,
	NULL,
};
static struct attribute_group int1_group = {
	.name = "int1",
	.attrs = int1_attrs,
};

static PMU_ATTR(int2_deb_val, debounce_value, INT2_DEBOUNCE_VALUE);
static PMU_ATTR(int2_type, type, INT2_TYPE);
static PMU_ATTR(int2_wakeup, wakeup, INT2_WAKEUP);
static PMU_ATTR(int2_event, event, INT2_EVENT);
static struct attribute *int2_attrs[] = {
	&dev_attr_int2_deb_val.attr.attr,
	&dev_attr_int2_type.attr.attr,
	&dev_attr_int2_wakeup.attr.attr,
	&dev_attr_int2_event.attr.attr,
	NULL,
};
static struct attribute_group int2_group = {
	.name = "int2",
	.attrs = int2_attrs,
};

static PMU_ATTR(int3_deb_val, debounce_value, INT3_DEBOUNCE_VALUE);
static PMU_ATTR(int3_type, type, INT3_TYPE);
static PMU_ATTR(int3_wakeup, wakeup, INT3_WAKEUP);
static PMU_ATTR(int3_event, event, INT3_EVENT);
static struct attribute *int3_attrs[] = {
	&dev_attr_int3_deb_val.attr.attr,
	&dev_attr_int3_type.attr.attr,
	&dev_attr_int3_wakeup.attr.attr,
	&dev_attr_int3_event.attr.attr,
	NULL,
};
static struct attribute_group int3_group = {
	.name = "int3",
	.attrs = int3_attrs,
};

static PMU_ATTR(int4_type, type, INT4_TYPE);
static PMU_ATTR(int4_wakeup, wakeup, INT4_WAKEUP);
static PMU_ATTR(int4_event, event, INT4_EVENT);
static struct attribute *int4_attrs[] = {
	&dev_attr_int4_type.attr.attr,
	&dev_attr_int4_wakeup.attr.attr,
	&dev_attr_int4_event.attr.attr,
	NULL,
};
static struct attribute_group int4_group = {
	.name = "int4",
	.attrs = int4_attrs,
};

static PMU_ATTR(int5_type, type, INT5_TYPE);
static PMU_ATTR(int5_wakeup, wakeup, INT5_WAKEUP);
static PMU_ATTR(int5_event, event, INT5_EVENT);
static struct attribute *int5_attrs[] = {
	&dev_attr_int5_type.attr.attr,
	&dev_attr_int5_wakeup.attr.attr,
	&dev_attr_int5_event.attr.attr,
	NULL,
};
static struct attribute_group int5_group = {
	.name = "int5",
	.attrs = int5_attrs,
};

static PMU_ATTR(rtc_alarm_wakeup, alarm_wakeup, RTC_ALARM_WAKEUP);
static PMU_ATTR(rtc_alarm_event, alarm_event, RTC_ALARM_EVENT);
static PMU_ATTR(rtc_tick_wakeup, tick_wakeup, RTC_TICK_WAKEUP);
static PMU_ATTR(rtc_tick_event, tick_event, RTC_TICK_EVENT);
static struct attribute *rtc_attrs[] = {
	&dev_attr_rtc_alarm_wakeup.attr.attr,
	&dev_attr_rtc_alarm_event.attr.attr,
	&dev_attr_rtc_tick_wakeup.attr.attr,
	&dev_attr_rtc_tick_event.attr.attr,
	NULL,
};
static struct attribute_group rtc_group = {
	.name = "rtc",
	.attrs = rtc_attrs,
};

static const struct attribute_group *dev_groups[] = {
	&pmu_group,
	&int0_group,
	&int1_group,
	&int2_group,
	&int3_group,
	&int4_group,
	&int5_group,
	&rtc_group,
	NULL,
};

static unsigned int typevalue_to_inttype(int type, int offset)
{
	unsigned int value;

	if (type & IRQ_TYPE_EDGE_RISING)
		value = INT_TRIGGER_TYPE_MASK;
	else if (type & IRQ_TYPE_EDGE_FALLING)
		value = INT_TRIGGER_TYPE_MASK | INT_TRIGGER_EDGE_MASK;
	else if (type & IRQ_TYPE_LEVEL_LOW)
		value = INT_TRIGGER_LEVEL_MASK;
	else if (type & IRQ_TYPE_LEVEL_HIGH)
		value = 0;
	else
		value = 0;

	return value << offset;
}

static int k230_pmu_config(struct device_node *node)
{
	void *base = k230_pmu_dev->regs;
	struct device_node *np;
	unsigned int value;
	unsigned int int2out0_en, int2cpu_en, int_en, int_type;

	int2out0_en = INT2X_INT0_0_EN_MASK;
	int2cpu_en = INT2X_INT0_0_EN_MASK;
	int_en = INT8_EN_MASK;
	int_type = 0;

	np = of_get_child_by_name(node, "int0");
	if (np) {
		if (!of_property_read_u32(np, "force-powerdown-value", &value))
			writel(value, base + INT8_TRIGGER_VAL);
		if (!of_property_read_u32(np, "power-event-value", &value))
			writel(value, base + INT0_0_TRIGGER_VAL);
		if (!of_property_read_u32(np, "debounce-value", &value))
			writel(value, base + INT0_DEBOUNCE_VAL);
		if (!of_property_read_u32(np, "type", &value))
			int_type |= typevalue_to_inttype(value,
				INT0_2_TRIGGER_OFFSET);
		if (of_property_read_bool(np, "wakeup"))
			int2out0_en |= INT2X_INT0_2_EN_MASK;
		if (of_property_read_bool(np, "event"))
			int2cpu_en |= INT2X_INT0_2_EN_MASK;
	}

	np = of_get_child_by_name(node, "int1");
	if (np) {
		if (!of_property_read_u32(np, "debounce-value", &value))
			writel(value, base + INT1_DEBOUNCE_VAL);
		if (!of_property_read_u32(np, "edge-cnt-value", &value))
			writel(value, base + INT1_0_EDGE_VAL);
		if (!of_property_read_u32(np, "edge-cnt-type", &value))
			int_type |= (value & IRQ_TYPE_EDGE_RISING ? 0 :
				INT1_0_EDGE_MASK);
		if (of_property_read_bool(np, "edge-cnt-wakeup"))
			int2out0_en |= INT2X_INT1_0_EN_MASK;
		if (of_property_read_bool(np, "edge-cnt-event"))
			int2cpu_en |= INT2X_INT1_0_EN_MASK;
		if (!of_property_read_u32(np, "type", &value))
			int_type |= typevalue_to_inttype(value,
				INT1_1_TRIGGER_OFFSET);
		if (of_property_read_bool(np, "wakeup"))
			int2out0_en |= INT2X_INT1_1_EN_MASK;
		if (of_property_read_bool(np, "event"))
			int2cpu_en |= INT2X_INT1_1_EN_MASK;
	}

	np = of_get_child_by_name(node, "int2");
	if (np) {
		if (!of_property_read_u32(np, "debounce-value", &value))
			writel(value, base + INT2_DEBOUNCE_VAL);
		if (!of_property_read_u32(np, "type", &value))
			int_type |= typevalue_to_inttype(value,
				INT2_TRIGGER_OFFSET);
		if (of_property_read_bool(np, "wakeup"))
			int2out0_en |= INT2X_INT2_EN_MASK;
		if (of_property_read_bool(np, "event"))
			int2cpu_en |= INT2X_INT2_EN_MASK;
	}

	np = of_get_child_by_name(node, "int3");
	if (np) {
		if (!of_property_read_u32(np, "debounce-value", &value))
			writel(value, base + INT3_DEBOUNCE_VAL);
		if (!of_property_read_u32(np, "type", &value))
			int_type |= typevalue_to_inttype(value,
				INT3_TRIGGER_OFFSET);
		if (of_property_read_bool(np, "wakeup"))
			int2out0_en |= INT2X_INT3_EN_MASK;
		if (of_property_read_bool(np, "event"))
			int2cpu_en |= INT2X_INT3_EN_MASK;
	}

	np = of_get_child_by_name(node, "int4");
	if (np) {
		if (!of_property_read_u32(np, "type", &value))
			int_type |= typevalue_to_inttype(value,
				INT4_TRIGGER_OFFSET);
		if (of_property_read_bool(np, "wakeup"))
			int2out0_en |= INT2X_INT4_EN_MASK;
		if (of_property_read_bool(np, "event"))
			int2cpu_en |= INT2X_INT4_EN_MASK;
	}

	np = of_get_child_by_name(node, "int5");
	if (np) {
		if (!of_property_read_u32(np, "type", &value))
			int_type |= typevalue_to_inttype(value,
				INT5_TRIGGER_OFFSET);
		if (of_property_read_bool(np, "wakeup"))
			int2out0_en |= INT2X_INT5_EN_MASK;
		if (of_property_read_bool(np, "event"))
			int2cpu_en |= INT2X_INT5_EN_MASK;
	}

	np = of_get_child_by_name(node, "rtc");
	if (np) {
		if (of_property_read_bool(np, "alarm-wakeup"))
			int2out0_en |= INT2X_RTC_ALARM_EN_MASK;
		if (of_property_read_bool(np, "alarm-event"))
			int2cpu_en |= INT2X_RTC_ALARM_EN_MASK;
		if (of_property_read_bool(np, "tick-wakeup"))
			int2out0_en |= INT2X_RTC_TICK_EN_MASK;
		if (of_property_read_bool(np, "tick-event"))
			int2cpu_en |= INT2X_RTC_TICK_EN_MASK;
	}

	int_en |= int2out0_en | int2cpu_en;
	writel(0, base + INT_EN);
	writel(INT_CLR_ALL, base + INT_CLEAR);
	writel(int_type, base + INT_TYPE);
	writel(int_en, base + INT_EN);
	writel(int2out0_en, base + INT2OUTPUT0_EN);
	writel(int2cpu_en, base + INT2CPU_EN);

	return 0;
}

static irqreturn_t pmu_isr(int irq, void *dev_id)
{
#define EVENT_COUNT 9
	static unsigned int mask_array[EVENT_COUNT] = {
		INT0_0_EN_MASK, INT0_2_EN_MASK,
		INT1_0_EN_MASK | INT1_1_EN_MASK,
		INT2_EN_MASK, INT3_EN_MASK,
		INT4_EN_MASK, INT5_EN_MASK,
		RTC_ALARM_EN_MASK, RTC_TICK_EN_MASK
	};
	static unsigned int keycode_array[EVENT_COUNT] = {
		KEY_POWER, BTN_0, BTN_1, BTN_2, BTN_3,
		BTN_4, BTN_5, BTN_6, BTN_7
	};
	struct _k230_pmu_dev *pmu = dev_id;
	void *base = pmu->regs;
	struct input_dev *input = pmu->input;
	unsigned int int_status;
	int i;

	int_status = readl(base + INT_STATUS) & readl(base + INT2CPU_EN);
	writel(int_status >> 3, base + INT_CLEAR);

	for (i = 0; i < EVENT_COUNT; i++) {
		if (int_status & mask_array[i]) {
			input_report_key(input, keycode_array[i], 1);
			input_report_key(input, keycode_array[i], 0);
		}
	}
	if (int_status)
		input_sync(input);

	return IRQ_HANDLED;
}

static int k230_pmu_probe(struct platform_device *pdev)
{
	struct device *dev = &pdev->dev;
	struct _k230_pmu_dev *pmu;
	struct input_dev *input;
	void __iomem *pmu_iso;
	unsigned int pmu_status;
	unsigned int int_status;
	int irq, ret;

	pmu = devm_kzalloc(dev, sizeof(*pmu), GFP_KERNEL);
	if (!pmu)
		return -ENOMEM;
	k230_pmu_dev = pmu;

	pmu->regs = devm_platform_ioremap_resource(pdev, 0);
	if (IS_ERR(pmu->regs))
		return PTR_ERR(pmu->regs);

	pmu_iso = ioremap(0x91103158, 4);
	writel(0, pmu_iso);
	iounmap(pmu_iso);

	pmu_status = readl(pmu->regs + PMU_STATUS);
	int_status = readl(pmu->regs + INT_STATUS);
	if (pmu_status == SOC_FORCE_PD && ((int_status & 1) == 0))
		pmu_status = SOC_RESET;
	int_status &= readl(pmu->regs + INT2OUTPUT0_EN) & ALL_WAKEUP_MASK;
	writel(SOC_FORCE_PD, pmu->regs + PMU_STATUS);
	pmu->pmu_status = pmu_status;
	pmu->wakeup_source = int_status;
	mutex_init(&pmu->mutex);

	k230_pmu_config(dev->of_node);

	input = devm_input_allocate_device(dev);
	if (!input) {
		dev_err(dev, "failed to allocate input device\n");
		return -ENOMEM;
	}

	pmu->input = input;
	input->name = "pmu_input";
	input_set_capability(input, EV_KEY, KEY_POWER);
	input_set_capability(input, EV_KEY, BTN_0);
	input_set_capability(input, EV_KEY, BTN_1);
	input_set_capability(input, EV_KEY, BTN_2);
	input_set_capability(input, EV_KEY, BTN_3);
	input_set_capability(input, EV_KEY, BTN_4);
	input_set_capability(input, EV_KEY, BTN_5);
	input_set_capability(input, EV_KEY, BTN_6);
	input_set_capability(input, EV_KEY, BTN_7);

	ret = input_register_device(input);
	if (ret) {
		dev_err(dev, "register input device error: %d\n", ret);
		return ret;
	}

	irq = platform_get_irq(pdev, 0);
	if (irq < 0) {
		dev_err(dev, "no set irq\n");
		return -EINVAL;
	}

	ret = devm_request_irq(dev, irq, pmu_isr, 0, "pmu_irq", pmu);
	if (ret < 0) {
		dev_err(dev, "Unable to claim irq\n");
		return ret;
	}

	ret = sysfs_create_groups(&dev->kobj, dev_groups);
	if (ret) {
		dev_err(dev, "create groups error\n");
		return ret;
	}

	return ret;
}

static const struct of_device_id k230_pmu_of_matches[] = {
	{ .compatible = "kendryte, k230-pmu" },
	{}
};

static struct platform_driver k230_pmu_driver = {
	.driver = {
		.name = "k230-pmu",
		.of_match_table = k230_pmu_of_matches,
	},
	.probe = k230_pmu_probe,
};

module_platform_driver(k230_pmu_driver);

MODULE_DESCRIPTION("K230 PMU driver");
MODULE_LICENSE("GPL v2");
