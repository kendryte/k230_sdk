#ifndef CYW43_INCLUDED_CONFIGPORT_H
#define CYW43_INCLUDED_CONFIGPORT_H

#include <stdint.h>

#define CYW43_CHIPSET_FIRMWARE_INCLUDE_FILE "firmware/fw_bcm43438a1.h"
#define CYW43_WIFI_NVRAM_INCLUDE_FILE       "firmware/nvram.h"
#define CYW43_CLEAR_SDIO_INT                (1)
#define CYW43_RESOURCE_VERIFY_DOWNLOAD      (1)
#define CYW43_LWIP                          (0)
#define CYW43_NETUTILS                      (0)
#define CYW43_USE_OTP_MAC                   (1)

#define CYW43_PIN_WL_REG_ON                 (1)
#define CYW43_PIN_WL_SDIO_1                 (1)
#define NDEBUG

#define MIN(a, b)                           ((a) <= (b) ? (a) : (b))
#undef static_assert
#define static_assert(expr, msg)
#define CYW43_ARRAY_SIZE(a)                 (sizeof(a) / sizeof((a)[0]))

#define CYW43_EPERM                         (1)
#define CYW43_EIO                           (5)
#define CYW43_EINVAL                        (22)
#define CYW43_ETIMEDOUT                     (110)

void cyw43_thread_enter(void);
void cyw43_thread_exit(void);
void cyw43_thread_lock_check(void);

#define CYW43_THREAD_ENTER cyw43_thread_enter();
#define CYW43_THREAD_EXIT cyw43_thread_exit();
#define CYW43_THREAD_LOCK_CHECK cyw43_thread_lock_check();

#define CYW43_SDPCM_SEND_COMMON_WAIT        do { } while (0)
#define CYW43_DO_IOCTL_WAIT                 do { } while (0)

#define CYW43_HAL_PIN_MODE_INPUT            (1)
#define CYW43_HAL_PIN_MODE_OUTPUT           (0)
#define CYW43_HAL_PIN_PULL_NONE             (0)
#define CYW43_HAL_MAC_WLAN0                 (0)

uint64_t cyw43_hal_ticks_us(void);
uint64_t cyw43_hal_ticks_ms(void);
void cyw43_delay_us(uint64_t us);
void cyw43_delay_ms(uint64_t ms);
void cyw43_hal_get_mac(int interface, uint8_t mac[6]);
void cyw43_hal_pin_config(int pin, int mode, int pull, int alt);
void cyw43_hal_pin_config_irq_falling(int pin, int enable);
int cyw43_hal_pin_read(int pin);
void cyw43_hal_pin_low(int pin);
void cyw43_hal_pin_high(int pin);
void cyw43_schedule_internal_poll_dispatch(void (*func)(void));
#endif // CYW43_INCLUDED_CONFIGPORT_H
