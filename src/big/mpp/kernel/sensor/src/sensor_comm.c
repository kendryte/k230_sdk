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

#include "sensor_dev.h"
#include "k_sensor_ioctl.h"
#include "k_sensor_comm.h"

extern struct sensor_driver_dev ov9732_sensor_drv;
extern struct sensor_driver_dev ov9286_sensor_drv;
extern struct sensor_driver_dev imx335_sensor_drv;
extern struct sensor_driver_dev sc035hgs_sensor_drv;
extern struct sensor_driver_dev ov5647_sensor_drv;
extern struct sensor_driver_dev sc201cs_sensor_drv;
extern struct sensor_driver_dev ov5647_sensor_csi1_drv;
extern struct sensor_driver_dev ov5647_sensor_csi2_drv;
extern struct sensor_driver_dev xs9950_csi0_sensor_drv;
extern struct sensor_driver_dev xs9950_csi1_sensor_drv;
extern struct sensor_driver_dev xs9950_csi2_sensor_drv;
extern struct sensor_driver_dev gc2053_sensor_drv;

struct sensor_driver_dev *sensor_drv_list[SENSOR_NUM_MAX] = {
    &ov9732_sensor_drv,
    &ov9286_sensor_drv,
    &imx335_sensor_drv,
    &sc035hgs_sensor_drv,
    &ov5647_sensor_drv,
    &sc201cs_sensor_drv,
    &ov5647_sensor_csi1_drv,
    &ov5647_sensor_csi2_drv,
    &xs9950_csi0_sensor_drv,
    &xs9950_csi1_sensor_drv,
    &xs9950_csi2_sensor_drv,
    &gc2053_sensor_drv,
};

void sensor_drv_list_init(struct sensor_driver_dev *drv_list[])
{
    for (k_u32 sensor_id = 0; sensor_id < SENSOR_NUM_MAX; sensor_id++) {
        if (drv_list[sensor_id] != NULL)
            g_sensor_drv[sensor_id] = drv_list[sensor_id];
    }
}

k_s32 sensor_reg_read(k_sensor_i2c_info *i2c_info, k_u16 reg_addr, k_u16 *buf)
{
    struct rt_i2c_msg msg[2];
    k_u8 i2c_reg[2];
    k_u8 i2c_buf[2];

    RT_ASSERT(i2c_info != RT_NULL);

    if (i2c_info->reg_addr_size == SENSOR_REG_VALUE_8BIT) {
        i2c_reg[0] = reg_addr & 0xff;
    } else if (i2c_info->reg_addr_size == SENSOR_REG_VALUE_16BIT) {
        i2c_reg[0] = reg_addr >> 8;
        i2c_reg[1] = reg_addr & 0xff;
    }

    msg[0].addr  = i2c_info->slave_addr;
    msg[0].flags = RT_I2C_WR;
    msg[0].len   = i2c_info->reg_addr_size;
    msg[0].buf   = i2c_reg;

    msg[1].addr  = i2c_info->slave_addr;
    msg[1].flags = RT_I2C_RD;
    msg[1].len   = i2c_info->reg_val_size;
    msg[1].buf   = i2c_buf;

    if (rt_i2c_transfer(i2c_info->i2c_bus, msg, 2) == 2)
    {
        *buf = (i2c_info->reg_val_size == SENSOR_REG_VALUE_8BIT) ? i2c_buf[0] : (i2c_buf[0] << 8) | i2c_buf[1];
        // rt_kprintf("sensor_reg_read: [0x%04x] = [0x%02x] i2c_info->size is %d \n", reg_addr, *buf, i2c_info->size);
        return RT_EOK;
    }
    rt_kprintf("%s err.\n", __func__);

    return RT_ERROR;
}

k_s32 sensor_reg_write(k_sensor_i2c_info *i2c_info, k_u16 reg_addr, k_u16 reg_val)
{
    struct rt_i2c_msg msgs;
    k_u8 buf[4];
    k_u8 len = 0;

    RT_ASSERT(i2c_info != RT_NULL);

    if (i2c_info->reg_addr_size == SENSOR_REG_VALUE_8BIT) {
        buf[len++] = reg_addr & 0xff;
    } else if (i2c_info->reg_addr_size == SENSOR_REG_VALUE_16BIT) {
        buf[len++] = reg_addr >> 8;
        buf[len++] = reg_addr & 0xff;
    }

    if (i2c_info->reg_val_size == SENSOR_REG_VALUE_8BIT) {
        buf[len++] = reg_val & 0xff;
    } else if (i2c_info->reg_val_size == SENSOR_REG_VALUE_16BIT) {
        buf[len++] = reg_val >> 8;
        buf[len++] = reg_val & 0xff;
    }

    msgs.addr = i2c_info->slave_addr;
    msgs.flags = RT_I2C_WR;
    msgs.buf = buf;
    msgs.len = len;

    if (rt_i2c_transfer(i2c_info->i2c_bus, &msgs, 1) == 1)
    {
        //rt_kprintf("sensor_reg_wirte: [0x%04x] = [0x%02x]\n", reg_addr, reg_val);
        return RT_EOK;
    }

    return RT_ERROR;
}

k_s32 sensor_reg_list_write(k_sensor_i2c_info *i2c_info, const k_sensor_reg *reg_list)
{
	k_s32 ret = 0;
	k_u32 i;

	for (i = 0; reg_list[i].addr != REG_NULL; i++) {
		ret = sensor_reg_write(i2c_info, reg_list[i].addr, reg_list[i].val);
        if (ret) {
            rt_kprintf("%s err, [0x%04x] = [0x%02x]\n", __func__, reg_list[i].addr, reg_list[i].val);
            return -1;
        }
	}
	return ret;
}

k_s32 sensor_reg_list_read(k_sensor_i2c_info *i2c_info, const k_sensor_reg *reg_list)
{
	k_s32 ret = 0;
	k_u16 i, val;

	for (i = 0; reg_list[i].addr != REG_NULL; i++) {
		ret = sensor_reg_read(i2c_info, reg_list[i].addr, &val);
        if (ret) {
            rt_kprintf("%s err, [0x%04x] = [0x%02x]\n", __func__, reg_list[i].addr, reg_list[i].val);
            return -1;
        }
        rt_kprintf("{%04x, %02x}\n", reg_list[i].addr, val);
	}
	return ret;
}

k_s32 sensor_priv_ioctl(struct sensor_driver_dev *dev, k_u32 cmd, void *args)
{
	k_s32 ret = -1;
	if (!dev) {
        rt_kprintf("%s error, dev null\n", __func__);
		return ret;
	}

    //rt_kprintf("[%s:%d]cmd 0x%08x\n", __func__, __LINE__, cmd);
	switch (cmd) {
        case KD_IOC_SENSOR_S_POWER:
        {
			k_s32 power_on;
			if (dev->sensor_func.sensor_power == NULL) {
                rt_kprintf("%s (%s)sensor_power is null\n", __func__, dev->sensor_name);
				return -1;
			}

            if (sizeof(k_s32) != lwp_get_from_user(&power_on, args, sizeof(k_s32))){
                rt_kprintf("%s:%d lwp_get_from_user err\n", __func__, __LINE__);
                return -1;
            }

			ret = dev->sensor_func.sensor_power(dev, power_on);
			break;
		}
        case KD_IOC_SENSOR_S_INIT:
        {
			k_sensor_mode sensor_mode;
			if (dev->sensor_func.sensor_init == NULL) {
                rt_kprintf("%s (%s)sensor_init is null\n", __func__, dev->sensor_name);
				return -1;
			}

            if (sizeof(k_sensor_mode) != lwp_get_from_user(&sensor_mode, args, sizeof(k_sensor_mode))){
                rt_kprintf("%s:%d lwp_get_from_user err\n", __func__, __LINE__);
                return -1;
            }
			ret = dev->sensor_func.sensor_init(dev, sensor_mode);
			break;
		}
        case KD_IOC_SENSOR_G_ID:
        {
			k_u32 chip_id = 0;
			if (dev->sensor_func.sensor_get_chip_id == NULL) {
                rt_kprintf("%s (%s)sensor_get_chip_id is null\n", __func__, dev->sensor_name);
				return -1;
			}
			ret = dev->sensor_func.sensor_get_chip_id(dev, &chip_id);
            if (ret) {
                rt_kprintf("%s (%s)sensor_get_chip_id err\n", __func__, dev->sensor_name);
                return -1;
            }
            if (sizeof(chip_id) != lwp_put_to_user(args, &chip_id, sizeof(chip_id))){
                rt_kprintf("%s:%d lwp_put_to_user err\n", __func__, __LINE__);
                return -1;
            }
			break;
		}
        case KD_IOC_SENSOR_REG_READ:
        {
            k_sensor_reg reg;

            if (sizeof(k_sensor_reg) != lwp_get_from_user(&reg, args, sizeof(k_sensor_reg))){
                rt_kprintf("%s:%d lwp_get_from_user err\n", __func__, __LINE__);
                return -1;
            }

            k_u16 reg_val;
            ret = sensor_reg_read(&dev->i2c_info, reg.addr, &reg_val);
            if (ret) {
                rt_kprintf("%s:%d sensor_reg_read err\n", __func__, __LINE__);
                return -1;
            }
            reg.val = reg_val;

            if (sizeof(k_sensor_reg) != lwp_put_to_user(args, &reg, sizeof(k_sensor_reg))){
                rt_kprintf("%s:%d lwp_put_to_user err\n", __func__, __LINE__);
                return -1;
            }
            break;
        }
        case KD_IOC_SENSOR_REG_WRITE:
        {
            k_sensor_reg reg;

            if (sizeof(k_sensor_reg) != lwp_get_from_user(&reg, args, sizeof(k_sensor_reg))){
                rt_kprintf("%s:%d lwp_get_from_user err\n", __func__, __LINE__);
                return -1;
            }

            ret = sensor_reg_write(&dev->i2c_info, reg.addr, reg.val);
            if (ret) {
                rt_kprintf("%s:%d sensor_reg_write err\n", __func__, __LINE__);
                return -1;
            }
            break;
        }
        case KD_IOC_SENSOR_G_MODE:
        {
            k_sensor_mode mode;
			if (dev->sensor_func.sensor_get_mode == NULL) {
                rt_kprintf("%s (%s)sensor_get_mode is null\n", __func__, dev->sensor_name);
				return -1;
			}

            if (sizeof(k_sensor_mode) != lwp_get_from_user(&mode, args, sizeof(k_sensor_mode))){
                rt_kprintf("%s:%d lwp_get_from_user err\n", __func__, __LINE__);
                return -1;
            }

			ret = dev->sensor_func.sensor_get_mode(dev, &mode);
            if (ret) {
                rt_kprintf("%s (%s)sensor_get_mode err\n", __func__, dev->sensor_name);
                return -1;
            }

            if (sizeof(k_sensor_mode) != lwp_put_to_user(args, &mode, sizeof(k_sensor_mode))){
                rt_kprintf("%s:%d lwp_put_to_user err\n", __func__, __LINE__);
                return -1;
            }

            break;
        }
        case KD_IOC_SENSOR_S_MODE:
        {
            k_sensor_mode mode;

            if (sizeof(k_sensor_mode) != lwp_get_from_user(&mode, args, sizeof(k_sensor_mode))){
                rt_kprintf("%s:%d lwp_get_from_user err\n", __func__, __LINE__);
                return -1;
            }

			if (dev->sensor_func.sensor_set_mode == NULL) {
                rt_kprintf("%s (%s)sensor_set_mode is null\n", __func__, dev->sensor_name);
				return -1;
			}
			ret = dev->sensor_func.sensor_set_mode(dev, mode);
            if (ret) {
                rt_kprintf("%s (%s)sensor_set_mode err\n", __func__, dev->sensor_name);
                return -1;
            }

            break;
        }
        case KD_IOC_SENSOR_ENUM_MODE:
        {
            k_sensor_enum_mode enum_mode;

            if (sizeof(k_sensor_enum_mode) != lwp_get_from_user(&enum_mode, args, sizeof(k_sensor_enum_mode))){
                rt_kprintf("%s:%d lwp_get_from_user err\n", __func__, __LINE__);
                return -1;
            }
            if (dev->sensor_func.sensor_enum_mode == NULL) {
                rt_kprintf("%s (%s)sensor_get_mode is null\n", __func__, dev->sensor_name);
                return -1;
            }
            ret = dev->sensor_func.sensor_enum_mode(dev, &enum_mode);
            if (ret) {
                rt_kprintf("%s (%s)sensor_get_mode err\n", __func__, dev->sensor_name);
                return -1;
            }
            if (sizeof(k_sensor_enum_mode) != lwp_put_to_user(args, &enum_mode, sizeof(k_sensor_enum_mode))){
                rt_kprintf("%s:%d lwp_put_to_user err\n", __func__, __LINE__);
                return -1;
            }

            break;
        }
        case KD_IOC_SENSOR_G_CAPS:
        {
            k_sensor_caps caps;

            if (sizeof(k_sensor_caps) != lwp_get_from_user(&caps, args, sizeof(k_sensor_caps))){
                rt_kprintf("%s:%d lwp_get_from_user err\n", __func__, __LINE__);
                return -1;
            }
            if (dev->sensor_func.sensor_get_caps == NULL) {
                rt_kprintf("%s (%s)sensor_get_caps is null\n", __func__, dev->sensor_name);
                return -1;
            }
            ret = dev->sensor_func.sensor_get_caps(dev, &caps);
            if (ret) {
                rt_kprintf("%s (%s)sensor_get_caps err\n", __func__, dev->sensor_name);
                return -1;
            }
            if (sizeof(k_sensor_caps) != lwp_put_to_user(args, &caps, sizeof(k_sensor_caps))){
                rt_kprintf("%s:%d lwp_put_to_user err\n", __func__, __LINE__);
                return -1;
            }

            break;
        }
        case KD_IOC_SENSOR_CHECK_CONN:
        {
            k_s32 conn = 0;
            if (dev->sensor_func.sensor_conn_check == NULL) {
                rt_kprintf("%s (%s)sensor_get_chip_id is null\n", __func__, dev->sensor_name);
                return -1;
            }
            ret = dev->sensor_func.sensor_conn_check(dev, &conn);
            if (ret) {
                rt_kprintf("%s (%s)sensor_get_chip_id err\n", __func__, dev->sensor_name);
                return -1;
            }
            if (sizeof(k_s32) != lwp_put_to_user(args, &conn, sizeof(k_s32))){
                rt_kprintf("%s:%d lwp_put_to_user err\n", __func__, __LINE__);
                return -1;
            }
            break;
        }
        case KD_IOC_SENSOR_S_STREAM:
        {
            k_s32 enable = 0;
			if (dev->sensor_func.sensor_set_stream == NULL) {
                rt_kprintf("%s (%s)sensor_set_stream is null\n", __func__, dev->sensor_name);
				return -1;
			}

            if (sizeof(k_s32) != lwp_get_from_user(&enable, args, sizeof(k_s32))){
                rt_kprintf("%s:%d lwp_get_from_user err\n", __func__, __LINE__);
                return -1;
            }

			ret = dev->sensor_func.sensor_set_stream(dev, enable);
            if (ret) {
                rt_kprintf("%s (%s)sensor_set_stream err\n", __func__, dev->sensor_name);
                return -1;
            }
            break;
        }
        case KD_IOC_SENSOR_G_AGAIN:
        {
            k_sensor_gain gain;
            if (sizeof(k_sensor_gain) != lwp_get_from_user(&gain, args, sizeof(k_sensor_gain))){
                rt_kprintf("%s:%d lwp_get_from_user err\n", __func__, __LINE__);
                return -1;
            }
            if (dev->sensor_func.sensor_get_again == NULL) {
                rt_kprintf("%s (%s)sensor_get_again is null\n", __func__, dev->sensor_name);
                return -1;
            }
            ret = dev->sensor_func.sensor_get_again(dev, &gain);
            if (ret) {
                rt_kprintf("%s (%s)sensor_get_again err\n", __func__, dev->sensor_name);
                return -1;
            }
            if (sizeof(k_sensor_gain) != lwp_put_to_user(args, &gain, sizeof(k_sensor_gain))){
                rt_kprintf("%s:%d lwp_put_to_user err\n", __func__, __LINE__);
                return -1;
            }

            break;
        }
        case KD_IOC_SENSOR_S_AGAIN:
        {
            k_sensor_gain gain;

            if (sizeof(k_sensor_gain) != lwp_get_from_user(&gain, args, sizeof(k_sensor_gain))){
                rt_kprintf("%s:%d lwp_get_from_user err\n", __func__, __LINE__);
                return -1;
            }

            if (dev->sensor_func.sensor_set_again == NULL) {
                rt_kprintf("%s (%s)sensor_get_again is null\n", __func__, dev->sensor_name);
                return -1;
            }
            ret = dev->sensor_func.sensor_set_again(dev, gain);
            if (ret) {
                rt_kprintf("%s (%s)sensor_get_again err\n", __func__, dev->sensor_name);
                return -1;
            }

            break;
        }
        case KD_IOC_SENSOR_G_DGAIN:
        {
            k_sensor_gain gain;
            if (sizeof(k_sensor_gain) != lwp_get_from_user(&gain, args, sizeof(k_sensor_gain))){
                rt_kprintf("%s:%d lwp_get_from_user err\n", __func__, __LINE__);
                return -1;
            }
            if (dev->sensor_func.sensor_get_dgain == NULL) {
                rt_kprintf("%s (%s)sensor_get_dgain is null\n", __func__, dev->sensor_name);
                return -1;
            }
            ret = dev->sensor_func.sensor_get_dgain(dev, &gain);
            if (ret) {
                rt_kprintf("%s (%s)sensor_get_dgain err\n", __func__, dev->sensor_name);
                return -1;
            }
            if (sizeof(k_sensor_gain) != lwp_put_to_user(args, &gain, sizeof(k_sensor_gain))){
                rt_kprintf("%s:%d lwp_put_to_user err\n", __func__, __LINE__);
                return -1;
            }

            break;
        }
        case KD_IOC_SENSOR_S_DGAIN:
        {
            k_sensor_gain gain;

            if (sizeof(k_sensor_gain) != lwp_get_from_user(&gain, args, sizeof(k_sensor_gain))){
                rt_kprintf("%s:%d lwp_get_from_user err\n", __func__, __LINE__);
                return -1;
            }

            if (dev->sensor_func.sensor_set_dgain == NULL) {
                rt_kprintf("%s (%s)sensor_get_dgain is null\n", __func__, dev->sensor_name);
                return -1;
            }
            ret = dev->sensor_func.sensor_set_dgain(dev, gain);
            if (ret) {
                rt_kprintf("%s (%s)sensor_get_dgain err\n", __func__, dev->sensor_name);
                return -1;
            }

            break;
        }
        case KD_IOC_SENSOR_G_INTG_TIME:
        {
            k_sensor_intg_time time;

            if (dev->sensor_func.sensor_get_dgain == NULL) {
                rt_kprintf("%s (%s)sensor_get_dgain is null\n", __func__, dev->sensor_name);
                return -1;
            }
            if (sizeof(k_sensor_intg_time) != lwp_get_from_user(&time, args, sizeof(k_sensor_intg_time))){
                rt_kprintf("%s:%d lwp_get_from_user err\n", __func__, __LINE__);
                return -1;
            }
            ret = dev->sensor_func.sensor_get_intg_time(dev, &time);
            if (ret) {
                rt_kprintf("%s (%s)k_sensor_intg_time err\n", __func__, dev->sensor_name);
                return -1;
            }
            if (sizeof(k_sensor_intg_time) != lwp_put_to_user(args, &time, sizeof(k_sensor_intg_time))){
                rt_kprintf("%s:%d lwp_put_to_user err\n", __func__, __LINE__);
                return -1;
            }

            break;
        }
        case KD_IOC_SENSOR_S_INTG_TIME:
        {
            k_sensor_intg_time time;

            if (sizeof(k_sensor_intg_time) != lwp_get_from_user(&time, args, sizeof(k_sensor_intg_time))){
                rt_kprintf("%s:%d lwp_get_from_user err\n", __func__, __LINE__);
                return -1;
            }

            if (dev->sensor_func.sensor_set_intg_time == NULL) {
                rt_kprintf("%s (%s)sensor_set_intg_time is null\n", __func__, dev->sensor_name);
                return -1;
            }
            ret = dev->sensor_func.sensor_set_intg_time(dev, time);
            if (ret) {
                rt_kprintf("%s (%s)sensor_set_intg_time err\n", __func__, dev->sensor_name);
                return -1;
            }

            break;
        }
        case KD_IOC_SENSOR_GET_EXP_PRAM:
        {
            k_sensor_exposure_param exp_parm;

            if (dev->sensor_func.sensor_get_exp_parm == NULL) {
                rt_kprintf("%s (%s)sensor_get_dgain is null\n", __func__, dev->sensor_name);
                return -1;
            }
            if (sizeof(k_sensor_exposure_param) != lwp_get_from_user(&exp_parm, args, sizeof(k_sensor_exposure_param))){
                rt_kprintf("%s:%d lwp_get_from_user err\n", __func__, __LINE__);
                return -1;
            }
            ret = dev->sensor_func.sensor_get_exp_parm(dev, &exp_parm);
            if (ret) {
                rt_kprintf("%s (%s)sensor_get_dgain err\n", __func__, dev->sensor_name);
                return -1;
            }
            if (sizeof(k_sensor_exposure_param) != lwp_put_to_user(args, &exp_parm, sizeof(k_sensor_exposure_param))){
                rt_kprintf("%s:%d lwp_put_to_user err\n", __func__, __LINE__);
                return -1;
            }

            break;
        }
        case KD_IOC_SENSOR_SET_EXP_PRAM:
        {
            k_sensor_exposure_param exp_parm;

            if (sizeof(k_sensor_exposure_param) != lwp_get_from_user(&exp_parm, args, sizeof(k_sensor_exposure_param))){
                rt_kprintf("%s:%d lwp_get_from_user err\n", __func__, __LINE__);
                return -1;
            }

            if (dev->sensor_func.sensor_set_exp_parm == NULL) {
                rt_kprintf("%s (%s)sensor_set_exp_parm is null\n", __func__, dev->sensor_name);
                return -1;
            }
            ret = dev->sensor_func.sensor_set_exp_parm(dev, exp_parm);
            if (ret) {
                rt_kprintf("%s (%s)sensor_set_exp_parm err\n", __func__, dev->sensor_name);
                return -1;
            }

            break;
        }
        case KD_IOC_SENSOR_G_FPS:
        {
            k_u32 fps;

            if (dev->sensor_func.sensor_get_fps == NULL) {
                rt_kprintf("%s (%s)sensor_get_fps is null\n", __func__, dev->sensor_name);
                return -1;
            }
            ret = dev->sensor_func.sensor_get_fps(dev, &fps);
            if (ret) {
                rt_kprintf("%s (%s)sensor_get_fps err\n", __func__, dev->sensor_name);
                return -1;
            }
            if (sizeof(k_u32) != lwp_put_to_user(args, &fps, sizeof(k_u32))){
                rt_kprintf("%s:%d lwp_put_to_user err\n", __func__, __LINE__);
                return -1;
            }

            break;
        }
        case KD_IOC_SENSOR_S_FPS:
        {
            k_u32 fps;
            if (sizeof(k_u32) != lwp_get_from_user(&fps, args, sizeof(k_u32))){
                rt_kprintf("%s:%d lwp_get_from_user err\n", __func__, __LINE__);
                return -1;
            }

            if (dev->sensor_func.sensor_set_fps == NULL) {
                rt_kprintf("%s (%s)sensor_set_fps is null\n", __func__, dev->sensor_name);
                return -1;
            }
            ret = dev->sensor_func.sensor_set_fps(dev, fps);
            if (ret) {
                rt_kprintf("%s (%s)sensor_set_fps err\n", __func__, dev->sensor_name);
                return -1;
            }

            break;
        }
        case KD_IOC_SENSOR_G_ISP_STATUS:
        {
            k_sensor_isp_status isp_status;

            if (dev->sensor_func.sensor_get_isp_status == NULL) {
                rt_kprintf("%s (%s)sensor_get_isp_status is null\n", __func__, dev->sensor_name);
                return -1;
            }
            ret = dev->sensor_func.sensor_get_isp_status(dev, &isp_status);
            if (ret) {
                rt_kprintf("%s (%s)sensor_get_fps err\n", __func__, dev->sensor_name);
                return -1;
            }
            if (sizeof(k_sensor_isp_status) != lwp_put_to_user(args, &isp_status, sizeof(k_sensor_isp_status))){
                rt_kprintf("%s:%d lwp_put_to_user err\n", __func__, __LINE__);
                return -1;
            }

            break;
        }
        case KD_IOC_SENSOR_S_BLC:
        {
            k_sensor_blc blc;
            if (sizeof(k_sensor_blc) != lwp_get_from_user(&blc, args, sizeof(k_sensor_blc))){
                rt_kprintf("%s:%d lwp_get_from_user err\n", __func__, __LINE__);
                return -1;
            }

            if (dev->sensor_func.sensor_set_blc == NULL) {
                rt_kprintf("%s (%s)sensor_set_blc is null\n", __func__, dev->sensor_name);
                return -1;
            }
            ret = dev->sensor_func.sensor_set_blc(dev, blc);
            if (ret) {
                rt_kprintf("%s (%s)sensor_set_blc err\n", __func__, dev->sensor_name);
                return -1;
            }

            break;
        }
        case KD_IOC_SENSOR_S_WB:
        {
            k_sensor_white_balance wb;
            if (sizeof(k_sensor_white_balance) != lwp_get_from_user(&wb, args, sizeof(k_sensor_white_balance))){
                rt_kprintf("%s:%d lwp_get_from_user err\n", __func__, __LINE__);
                return -1;
            }

            if (dev->sensor_func.sensor_set_wb == NULL) {
                rt_kprintf("%s (%s)sensor_set_wb is null\n", __func__, dev->sensor_name);
                return -1;
            }
            ret = dev->sensor_func.sensor_set_wb(dev, wb);
            if (ret) {
                rt_kprintf("%s (%s)sensor_set_wb err\n", __func__, dev->sensor_name);
                return -1;
            }

            break;
        }
        case KD_IOC_SENSOR_G_TPG:
        {
            k_sensor_test_pattern tpg;

            if (dev->sensor_func.sensor_get_tpg == NULL) {
                rt_kprintf("%s (%s)sensor_get_tpg is null\n", __func__, dev->sensor_name);
                return -1;
            }
            ret = dev->sensor_func.sensor_get_tpg(dev, &tpg);
            if (ret) {
                rt_kprintf("%s (%s)sensor_get_fps err\n", __func__, dev->sensor_name);
                return -1;
            }
            if (sizeof(k_sensor_test_pattern) != lwp_put_to_user(args, &tpg, sizeof(k_sensor_test_pattern))){
                rt_kprintf("%s:%d lwp_put_to_user err\n", __func__, __LINE__);
                return -1;
            }

            break;
        }
        case KD_IOC_SENSOR_S_TPG:
        {
            k_sensor_test_pattern tpg;
            if (sizeof(k_sensor_test_pattern) != lwp_get_from_user(&tpg, args, sizeof(k_sensor_test_pattern))){
                rt_kprintf("%s:%d lwp_get_from_user err\n", __func__, __LINE__);
                return -1;
            }

            if (dev->sensor_func.sensor_set_tpg == NULL) {
                rt_kprintf("%s (%s)sensor_set_tpg is null\n", __func__, dev->sensor_name);
                return -1;
            }
            ret = dev->sensor_func.sensor_set_tpg(dev, tpg);
            if (ret) {
                rt_kprintf("%s (%s)sensor_set_tpg err\n", __func__, dev->sensor_name);
                return -1;
            }

            break;
        }
        case KD_IOC_SENSOR_G_EXPAND_CURVE:
        {
            k_sensor_compand_curve compand_curve;

            if (dev->sensor_func.sensor_get_expand_curve == NULL) {
                rt_kprintf("%s (%s)sensor_get_expand_curve is null\n", __func__, dev->sensor_name);
                return -1;
            }
            ret = dev->sensor_func.sensor_get_expand_curve(dev, &compand_curve);
            if (ret) {
                rt_kprintf("%s (%s)sensor_get_fps err\n", __func__, dev->sensor_name);
                return -1;
            }
            if (sizeof(k_sensor_compand_curve) != lwp_put_to_user(args, &compand_curve, sizeof(k_sensor_compand_curve))){
                rt_kprintf("%s:%d lwp_put_to_user err\n", __func__, __LINE__);
                return -1;
            }

            break;
        }
        case KD_IOC_SENSOR_G_OTP_DATA:
        {
            k_sensor_otp_date otp_read_val;

            if ((sizeof(k_sensor_otp_date)) != lwp_get_from_user(&otp_read_val, args, sizeof(k_sensor_otp_date))){
                rt_kprintf("%s:%d lwp_get_from_user err\n", __func__, __LINE__);
                return -1;
            }

            if (dev->sensor_func.sensor_get_otp_data == NULL) {
                rt_kprintf("%s (%s)sensor_get_otp_data is null\n", __func__, dev->sensor_name);
                return -1;
            }
            ret = dev->sensor_func.sensor_get_otp_data(dev, &otp_read_val);
            if (ret) {
                rt_kprintf("%s (%s)sensor_get_otp_data err\n", __func__, dev->sensor_name);
                return -1;
            }
            if (sizeof(k_sensor_otp_date) != lwp_put_to_user(args, &otp_read_val, sizeof(k_sensor_otp_date))){
                rt_kprintf("%s:%d lwp_put_to_user err\n", __func__, __LINE__);
                return -1;
            }
            break;
        }
        case KD_IOC_SENSOR_S_OTP_DATA:
        {
            k_sensor_otp_date otp_write_val;

            if ((sizeof(k_sensor_otp_date)) != lwp_get_from_user(&otp_write_val, args, sizeof(k_sensor_otp_date))){
                rt_kprintf("%s:%d lwp_get_from_user err\n", __func__, __LINE__);
                return -1;
            }

            if (dev->sensor_func.sensor_set_otp_data == NULL) {
                rt_kprintf("%s (%s)sensor_set_otp_data is null\n", __func__, dev->sensor_name);
                return -1;
            }
            ret = dev->sensor_func.sensor_set_otp_data(dev, &otp_write_val);
            if (ret) {
                rt_kprintf("%s (%s)sensor_set_otp_data err\n", __func__, dev->sensor_name);
                return ret;
            }
            break;
        }

        case KD_IOC_SENSOR_S_MIRROR :
        {
            k_vicap_mirror_mode mirror;

            if ((sizeof(k_vicap_mirror_mode)) != lwp_get_from_user(&mirror, args, sizeof(k_vicap_mirror_mode))){
                rt_kprintf("%s:%d lwp_get_from_user err\n", __func__, __LINE__);
                return -1;
            }

            if (dev->sensor_func.sensor_mirror_set == NULL) {
                rt_kprintf("%s (%s)sensor_mirror_set is null\n", __func__, dev->sensor_name);
                return -1;
            }
            ret = dev->sensor_func.sensor_mirror_set(dev, mirror);
            if (ret) {
                rt_kprintf("%s (%s)sensor_mirror_set err\n", __func__, dev->sensor_name);
                return ret;
            }
            break;
        }
    	default:
        {
            rt_kprintf("unsupported command 0x%08x\n", cmd);
            break;
        }
    }

	return ret;
}

static void i2c_read(k_s32 argc, char** argv)
{
    k_sensor_i2c_info i2c_info;
    k_u16 i2c_id;
    k_u16 salve_addr;
    k_u16 reg_addr;
    k_u16 reg_val = 0;

    if(argc < 4) {
        rt_kprintf("USAGE: i2c_read i2c_id salve_addr reg_addr\n");
        return;
    }

    i2c_id = (k_u16) strtoul(argv[1], 0, 0);
    salve_addr = (k_u16) strtoul(argv[2], 0, 0);
    reg_addr = (k_u16) strtoul(argv[3], 0, 0);

    rt_kprintf("i2c_read %d 0x%02x 0x%04x\n", i2c_id, salve_addr, reg_addr);

    switch (i2c_id) {
        case 0:
            i2c_info.i2c_name = "i2c0";
            break;
        case 1:
            i2c_info.i2c_name = "i2c1";
            break;
        case 2:
            i2c_info.i2c_name = "i2c2";
            break;
        case 3:
            i2c_info.i2c_name = "i2c3";
            break;
        default:
            rt_kprintf("i2c id error\n");
            return;
    }
    i2c_info.i2c_bus = rt_i2c_bus_device_find(i2c_info.i2c_name);
    if (i2c_info.i2c_bus == RT_NULL) {
        rt_kprintf("can't find %s deivce", i2c_info.i2c_name);
        return;
    }
    i2c_info.slave_addr = salve_addr;
    i2c_info.reg_val_size = SENSOR_REG_VALUE_8BIT;

    if (reg_addr & 0xff00) {
        i2c_info.reg_addr_size = SENSOR_REG_VALUE_16BIT;
    } else {
        i2c_info.reg_addr_size = SENSOR_REG_VALUE_8BIT;
    }

    if (sensor_reg_read(&i2c_info, reg_addr, &reg_val)) {
        rt_kprintf("i2c read failed\n");
        return;
    }
    rt_kprintf("0x%04x=0x%02x\n", reg_addr, reg_val);
    return;
}
MSH_CMD_EXPORT(i2c_read, i2c register read)

static void i2c_write(k_s32 argc, char** argv)
{
    k_sensor_i2c_info i2c_info;
    k_u16 i2c_id;
    k_u16 salve_addr;
    k_u16 reg_addr;
    k_u16 reg_val;

    if(argc < 5) {
        rt_kprintf("USAGE: i2c_write i2c_id salve_addr reg_addr reg_val\n");
        return;
    }

    i2c_id = (k_u16) strtoul(argv[1], 0, 0);
    salve_addr = (k_u16) strtoul(argv[2], 0, 0);
    reg_addr = (k_u16) strtoul(argv[3], 0, 0);
    reg_val = (k_u16) strtoul(argv[4], 0, 0);

    rt_kprintf("i2c_write %d 0x%02x 0x%04x 0x%02x\n", i2c_id, salve_addr, reg_addr, reg_val);
    switch (i2c_id) {
        case 0:
            i2c_info.i2c_name = "i2c0";
            break;
        case 1:
            i2c_info.i2c_name = "i2c1";
            break;
        case 2:
            i2c_info.i2c_name = "i2c2";
            break;
        case 3:
            i2c_info.i2c_name = "i2c3";
            break;
        default:
            rt_kprintf("i2c id error\n");
            return;
    }
    i2c_info.i2c_bus = rt_i2c_bus_device_find(i2c_info.i2c_name);
    if (i2c_info.i2c_bus == RT_NULL) {
        rt_kprintf("can't find %s deivce", i2c_info.i2c_name);
        return;
    }
    i2c_info.slave_addr = salve_addr;
    i2c_info.reg_val_size = SENSOR_REG_VALUE_8BIT;

    if (reg_addr & 0xff00) {
        i2c_info.reg_addr_size = SENSOR_REG_VALUE_16BIT;
    } else {
        i2c_info.reg_addr_size = SENSOR_REG_VALUE_8BIT;
    }

    if (sensor_reg_write(&i2c_info, reg_addr, reg_val)) {
        rt_kprintf("i2c write failed\n");
        return;
    }
    return;
}
MSH_CMD_EXPORT(i2c_write, i2c register write)
