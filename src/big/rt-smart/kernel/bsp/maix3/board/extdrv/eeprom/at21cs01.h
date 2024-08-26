#ifndef __DRV_EEPROM__
#define __DRV_EEPROM__

#include <rtdef.h>

rt_err_t eeprom_dev_open(rt_device_t dev, rt_uint16_t oflag);
rt_size_t eeprom_dev_read(rt_device_t dev, rt_off_t pos, void *buffer, rt_size_t size);
rt_size_t eeprom_dev_write(rt_device_t dev, rt_off_t pos, const void *buffer, rt_size_t size);
rt_err_t eeprom_dev_close(rt_device_t dev);

#endif