#ifndef RT_CONFIG_H__
#define RT_CONFIG_H__

/* Automatically generated file; DO NOT EDIT. */
/* RT-Thread Project Configuration */

/* RT-Thread Kernel */

#define RT_NAME_MAX 20
#define RT_USING_SMART
#define RT_ALIGN_SIZE 8
#define RT_THREAD_PRIORITY_32
#define RT_THREAD_PRIORITY_MAX 32
#define RT_TICK_PER_SECOND 1000
#define RT_USING_OVERFLOW_CHECK
#define RT_USING_HOOK
#define RT_USING_IDLE_HOOK
#define RT_IDLE_HOOK_LIST_SIZE 4
#define IDLE_THREAD_STACK_SIZE 16384
#define RT_USING_TIMER_SOFT
#define RT_TIMER_THREAD_PRIO 4
#define RT_TIMER_THREAD_STACK_SIZE 16384
#define RT_DEBUG

/* Inter-Thread communication */

#define RT_USING_SEMAPHORE
#define RT_USING_MUTEX
#define RT_USING_EVENT
#define RT_USING_MAILBOX
#define RT_USING_MESSAGEQUEUE
#define RT_USING_SIGNALS

/* Memory Management */

#define RT_USING_MEMPOOL
#define RT_USING_MEMHEAP
#define RT_USING_MEMHEAP_AS_HEAP
#define RT_USING_HEAP

/* Kernel Device Object */

#define RT_USING_DEVICE
#define RT_USING_DEVICE_OPS
#define RT_USING_CONSOLE
#define RT_CONSOLEBUF_SIZE 256
#define RT_CONSOLE_DEVICE_NAME "uart"
#define RT_VER_NUM 0x50000
#define ARCH_CPU_64BIT
#define RT_USING_CACHE
#define ARCH_MM_MMU
#define RT_USING_USERSPACE
#define KERNEL_VADDR_START 0x150000000
#define PV_OFFSET 0
#define ARCH_RISCV
#define ARCH_RISCV64

/* Proc Object*/
#define RT_USING_PROC

/* RT-Thread Components */

#define RT_USING_COMPONENTS_INIT
#define RT_USING_USER_MAIN
#define RT_MAIN_THREAD_STACK_SIZE 16384
#define RT_MAIN_THREAD_PRIORITY 10

/* C++ features */

#define RT_USING_MSH
#define RT_USING_FINSH
#define FINSH_USING_MSH
#define FINSH_THREAD_NAME "tshell"
#define FINSH_THREAD_PRIORITY 20
#define FINSH_THREAD_STACK_SIZE 81920
#define FINSH_USING_HISTORY
#define FINSH_HISTORY_LINES 5
#define FINSH_USING_SYMTAB
#define FINSH_CMD_SIZE 1024
#define MSH_USING_BUILT_IN_COMMANDS
#define FINSH_USING_DESCRIPTION
#define FINSH_ARG_MAX 100
#define MSH_WAIT_LWP_FINISH

/* Device virtual file system */

#define RT_USING_DFS
#define RT_USING_DFS_TMPFS
#define RT_USING_DFS_SHAREFS
#define RT_USING_DEV_BUS
#define DFS_USING_WORKDIR
#define DFS_FILESYSTEMS_MAX 6
#define DFS_FILESYSTEM_TYPES_MAX 6
#define DFS_FD_MAX 64
#define RT_USING_DFS_ELMFAT

/* elm-chan's FatFs, Generic FAT Filesystem Module */

#define RT_DFS_ELM_CODE_PAGE 437
#define RT_DFS_ELM_WORD_ACCESS
#define RT_DFS_ELM_USE_LFN_3
#define RT_DFS_ELM_USE_LFN 3
#define RT_DFS_ELM_MAX_LFN 255
#define RT_DFS_ELM_DRIVES 2
#define RT_DFS_ELM_MAX_SECTOR_SIZE 512
#define RT_DFS_ELM_REENTRANT
#define RT_USING_DFS_DEVFS
#define RT_USING_DFS_ROMFS
#define RT_USING_DFS_TMPFS

#define RT_USING_DFS_PROCFS
/* Device Drivers */

#define RT_USING_GNNE
#define RT_USING_DEVMEM2
#define RT_USING_USAGE
#define RT_USING_DEVICE_IPC
#define RT_UNAMED_PIPE_NUMBER 64
#define RT_PIPE_BUFSZ 512
#define RT_USING_SERIAL
#define RT_USING_SERIAL_V1
#define RT_SERIAL_USING_DMA
#define RT_SERIAL_RB_BUFSZ 64
#define RT_USING_TTY
#define RT_USING_CPUTIME
#define RT_USING_CPUTIME_RISCV
#define RT_USING_NULL
#define RT_USING_ZERO
#define RT_USING_RANDOM
#define RT_USING_DEVMEM
#define RT_USING_RTC
// #define RT_USING_RTC_PMU
#define RT_USING_I2C
#define RT_USING_I2C0
#define RT_USING_I2C1
#define RT_USING_I2C3
#define RT_USING_I2C4
#define RT_USING_I2C_DUMP
#define RT_USING_ADC
#define RT_USING_OTP
#define RT_USING_TS
#define RT_USING_HWHASH
#define RT_USING_AES
#define RT_USING_SM4
#define RT_USING_HARDLOCK
#define RT_USING_GPIO
#define RT_USING_SPI
#define RT_USING_WDT
#define RT_USING_PWM
#define RT_USING_HWTIMER
#define RT_USING_HW_TIMER0
#define RT_USING_HW_TIMER1
#define RT_USING_HW_TIMER2
#define RT_SDIO_STACK_SIZE 512
#define RT_SDIO_THREAD_PRIORITY 15
#define RT_MMCSD_STACK_SIZE 4096
#define RT_MMCSD_THREAD_PREORITY 22
#define RT_MMCSD_MAX_PARTITION 16
#define RT_USING_DEV_BUS
#define RT_USING_REGULATOR
/* canaan uart driver for uart1 uart2 uart4 */
// #define RT_USING_CANAAN_UART
// #define RT_USING_UART1
// #define RT_USING_UART2
// #define RT_USING_UART4
#define UART_BUFFER_SIZE     (1024 * 1024)
#define POLLIN_SIZE          (UART_BUFFER_SIZE / 2)
#define UART_TIMEOUT         (1000)

//#define PKG_USING_CHERRYUSB
//#define CONFIG_ENABLE_USB_DEVICE 1
//#define PKG_CHERRYUSB_DEVICE_DWC2
//#define PKG_CHERRYUSB_DEVICE_DWC2_PORT_HS
//#define PKG_CHERRYUSB_DEVICE
//#define PKG_CHERRYUSB_DEVICE_CDC
//#define PKG_CHERRYUSB_DEVICE_MSC
/* Using USB */

/*Using fast boot*/
#define RT_FASTBOOT

/* POSIX layer and C standard library */

#define RT_USING_LIBC
#define RT_USING_MUSL
#define RT_USING_POSIX
#define RT_USING_POSIX_CLOCKTIME

/* Network */

/* Socket abstraction layer */


/* Network interface device */


/* light weight TCP/IP stack */


/* AT commands */


/* VBUS(Virtual Software BUS) */


/* Utilities */

#define RT_USING_LWP
#define RT_LWP_MAX_NR 30
#define LWP_TASK_STACK_SIZE 16384
#define RT_CH_MSG_MAX_NR 1024
#define LWP_CONSOLE_INPUT_BUFFER_SIZE 1024
#define LWP_TID_MAX_NR 64
#define RT_LWP_SHM_MAX_NR 64

/* RT-Thread online packages */

/* IoT - internet of things */


/* Wi-Fi */

/* Marvell WiFi */


/* Wiced WiFi */


/* IoT Cloud */


/* security packages */


/* language packages */

/* JSON: JavaScript Object Notation, a lightweight data-interchange format */


/* XML: Extensible Markup Language */


/* multimedia packages */

/* LVGL: powerful and easy-to-use embedded GUI library */


/* u8g2: a monochrome graphic library */


/* PainterEngine: A cross-platform graphics application framework written in C language */


/* tools packages */


/* system packages */

/* enhanced kernel services */


/* acceleration: Assembly language or algorithmic acceleration packages */


/* CMSIS: ARM Cortex-M Microcontroller Software Interface Standard */


/* Micrium: Micrium software products porting for RT-Thread */


/* peripheral libraries and drivers */

/* sensors drivers */


/* touch drivers */


/* Kendryte SDK */


/* AI packages */


/* Signal Processing and Control Algorithm Packages */


/* miscellaneous packages */

/* project laboratory */

/* samples: kernel and components samples */


/* entertainment: terminal games and other interesting software packages */


/* Arduino libraries */


/* Projects */


/* Sensors */


/* Display */


/* Timing */


/* Data Processing */


/* Data Storage */

/* Communication */


/* Device Control */


/* Other */

/* Signal IO */


/* Uncategorized */

/* Kernel Testcase */

#define UTEST_MEMHEAP_TC
#define UTEST_IRQ_TC
#define UTEST_SEMAPHORE_TC
#define UTEST_EVENT_TC
#define UTEST_TIMER_TC
#define UTEST_MESSAGEQUEUE_TC
#define UTEST_MUTEX_TC
#define UTEST_MAILBOX_TC
#define UTEST_THREAD_TC
#define UTEST_MMU_TC
#define UTEST_ADC
#define UTEST_SPI_NAND
#define UTEST_OTP
#define UTEST_WDT
#define UTEST_TS
// #define UTEST_RTC
// #define UTEST_HWTIMER
// #define UTEST_PWM
//#define UTEST_I2C_OV9282
//#define UTEST_I2C_TMP103
// #define UTEST_GPIO
#define BOARD_fpgac908
#define ENABLE_FPU
#define ENABLE_VECTOR
#define __STACKSIZE__ 65536

#define RT_USING_MPP
#define RT_USING_IPCM
#endif
