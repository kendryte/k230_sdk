/*
 * Copyright (c) 2020-2022 Chengdu Aich Technology Co.,Ltd.All rights reserved.
 * Description: Error code returned by the interface.
 * Author: CompanyName
 * Create: 2021-08-02
 */
#ifndef EXT_ERRNO_H
#define EXT_ERRNO_H

/*****************************************************************************
* 1、通用错误码。注意0x8000 0000最好别用，产生截断误判为成功
*****************************************************************************/
#define  EXT_SUCCESS                                     0
#define  EXT_FAILURE                                     (td_u32)(-1)
#define  EXT_S_FAILURE                                   (-1)
#define  EXT_MALLOC_FAILUE                               0x80000001
#define  EXT_TIMEOUT                                     0x80000002
#define  EXT_RECVING                                     0x80000003
#define  EXT_MEMCPY_S                                    0x80000004
#define  EXT_MEMSET_S                                    0x80000005
#define  EXT_SPRINTF_S                                   0x80000006
#define  EXT_STRCPY_S                                    0x80000007

/*****************************************************************************
* 2、系统适配层:错误码
*****************************************************************************/
/* 任务 */
#define EXT_ERR_TASK_INVALID_PARAM                           0x80000080
#define EXT_ERR_TASK_CREATE_FAIL                             0x80000081
#define EXT_ERR_TASK_DELETE_FAIL                             0x80000082
#define EXT_ERR_TASK_SUPPEND_FAIL                            0x80000083
#define EXT_ERR_TASK_RESUME_FAIL                             0x80000084
#define EXT_ERR_TASK_GET_PRI_FAIL                            0x80000085
#define EXT_ERR_TASK_SET_PRI_FAIL                            0x80000086
#define EXT_ERR_TASK_LOCK_FAIL                               0x80000087
#define EXT_ERR_TASK_UNLOCK_FAIL                             0x80000088
#define EXT_ERR_TASK_DELAY_FAIL                              0x80000089
#define EXT_ERR_TASK_GET_INFO_FAIL                           0x8000008A
#define EXT_ERR_TASK_REGISTER_SCHEDULE_FAIL                  0x8000008B
#define EXT_ERR_TASK_NOT_CREATED                             0x8000008C

/* 中断ISR */
#define EXT_ERR_ISR_INVALID_PARAM                            0x800000C0
#define EXT_ERR_ISR_REQ_IRQ_FAIL                             0x800000C1
#define EXT_ERR_ISR_ADD_JOB_MALLOC_FAIL                      0x800000C2
#define EXT_ERR_ISR_ADD_JOB_SYS_FAIL                         0x800000C3
#define EXT_ERR_ISR_DEL_IRQ_FAIL                             0x800000C4
#define EXT_ERR_ISR_ALREADY_CREATED                          0x800000C5
#define EXT_ERR_ISR_NOT_CREATED                              0x800000C6
#define EXT_ERR_ISR_ENABLE_IRQ_FAIL                          0x800000C7
#define EXT_ERR_ISR_IRQ_ADDR_NOK                             0x800000C8

/* 内存 */
#define EXT_ERR_MEM_INVALID_PARAM                            0x80000100
#define EXT_ERR_MEM_CREAT_POOL_FAIL                          0x80000101
#define EXT_ERR_MEM_CREAT_POOL_NOT_ENOUGH_HANDLE             0x80000102
#define EXT_ERR_MEM_FREE_FAIL                                0x80000103
#define EXT_ERR_MEM_RE_INIT                                  0x80000104
#define EXT_ERR_MEM_NOT_INIT                                 0x80000105
#define EXT_ERR_MEM_CREAT_POOL_MALLOC_FAIL                   0x80000106
#define EXT_ERR_MEM_GET_INFO_FAIL                            0x80000107
#define EXT_ERR_MEM_GET_OS_INFO_NOK                          0x80000108

/* OSTIMER定时器 */
#define EXT_ERR_TIMER_FAILURE                                0x80000140
#define EXT_ERR_TIMER_INVALID_PARAM                          0x80000141
#define EXT_ERR_TIMER_CREATE_HANDLE_FAIL                     0x80000142
#define EXT_ERR_TIMER_START_FAIL                             0x80000143
#define EXT_ERR_TIMER_HANDLE_NOT_CREATE                      0x80000144
#define EXT_ERR_TIMER_HANDLE_INVALID                         0x80000145
#define EXT_ERR_TIMER_STATUS_INVALID                         0x80000146
#define EXT_ERR_TIMER_STATUS_START                           0x80000147
#define EXT_ERR_TIMER_INVALID_MODE                           0x80000148
#define EXT_ERR_TIMER_EXPIRE_INVALID                         0x80000149
#define EXT_ERR_TIMER_FUNCTION_NULL                          0x8000014A
#define EXT_ERR_TIMER_HANDLE_MAXSIZE                         0x8000014B
#define EXT_ERR_TIMER_MALLOC_FAIL                            0x8000014C
#define EXT_ERR_TIMER_NOT_INIT                               0x8000014D

/* 信号量 */
#define EXT_ERR_SEM_INVALID_PARAM                            0x80000180
#define EXT_ERR_SEM_CREATE_FAIL                              0x80000181
#define EXT_ERR_SEM_DELETE_FAIL                              0x80000182
#define EXT_ERR_SEM_WAIT_FAIL                                0x80000183
#define EXT_ERR_SEM_SIG_FAIL                                 0x80000184
#define EXT_ERR_SEM_WAIT_TIME_OUT                            0x80000185

/* 互斥锁 */
#define EXT_ERR_MUX_INVALID_PARAM                            0x800001C0
#define EXT_ERR_MUX_CREATE_FAIL                              0x800001C1
#define EXT_ERR_MUX_DELETE_FAIL                              0x800001C2
#define EXT_ERR_MUX_PEND_FAIL                                0x800001C3
#define EXT_ERR_MUX_POST_FAIL                                0x800001C4

/* 消息 */
#define EXT_ERR_MSG_INVALID_PARAM                            0x80000200
#define EXT_ERR_MSG_CREATE_Q_FAIL                            0x80000201
#define EXT_ERR_MSG_DELETE_Q_FAIL                            0x80000202
#define EXT_ERR_MSG_WAIT_FAIL                                0x80000203
#define EXT_ERR_MSG_SEND_FAIL                                0x80000204
#define EXT_ERR_MSG_GET_Q_INFO_FAIL                          0x80000205
#define EXT_ERR_MSG_Q_DELETE_FAIL                            0x80000206
#define EXT_ERR_MSG_WAIT_TIME_OUT                            0x80000207

/* 事件 */
#define EXT_ERR_EVENT_INVALID_PARAM                          0x80000240
#define EXT_ERR_EVENT_CREATE_NO_HADNLE                       0x80000241
#define EXT_ERR_EVENT_CREATE_SYS_FAIL                        0x80000242
#define EXT_ERR_EVENT_SEND_FAIL                              0x80000243
#define EXT_ERR_EVENT_WAIT_FAIL                              0x80000244
#define EXT_ERR_EVENT_CLEAR_FAIL                             0x80000245
#define EXT_ERR_EVENT_RE_INIT                                0x80000246
#define EXT_ERR_EVENT_NOT_ENOUGH_MEMORY                      0x80000247
#define EXT_ERR_EVENT_NOT_INIT                               0x80000248
#define EXT_ERR_EVENT_DELETE_FAIL                            0x80000249
#define EXT_ERR_EVENT_WAIT_TIME_OUT                          0x8000024A

/* os维测 */
#define EXT_ERR_OSSTAT_INVALID_PARAM                         0x80000280
#define EXT_ERR_OSSTAT_SYSTEM_CALL_ERROR                     0x80000281

/* liteos fpb */
#define EXT_ERR_FPB_COMP_REPEAT                              0x800002C0
#define EXT_ERR_FPB_NO_COMP                                  0x800002C1
#define EXT_ERR_FPB_TYPE                                     0x800002C2
#define EXT_ERR_FPB_NO_FREE_COMP                             0x800002C3
#define EXT_ERR_FPB_ADDR_NOT_ALIGN                           0x800002C4
#define EXT_ERR_FPB_TARGET_ADDR                              0x800002C5
#define EXT_ERR_FPB_BUSY                                     0x800002C6 /* ????????? */
#define EXT_ERR_FPB_ERROR_INPUT                              0x800002C7

/* CPU */
#define EXT_ERR_CPUP_NOT_INIT                                0x80000300
#define EXT_ERR_CPUP_INVALID_PARAM                           0x80000301
#define EXT_ERR_CPUP_CLK_INVALID_PARAM                        0x80000302

/* file system */
#define EXT_ERR_FS_INVALID_PARAM                             0x80000400
#define EXT_ERR_FS_NO_DEVICE                                 0x80000401
#define EXT_ERR_FS_NO_SPACE                                  0x80000402  /* No space left on device */
#define EXT_ERR_FS_BAD_DESCRIPTOR                            0x80000403
#define EXT_ERR_FS_FILE_EXISTS                               0x80000404
#define EXT_ERR_FS_NOT_FOUND                                 0x80000405
#define EXT_ERR_FS_NAME_TOO_LONG                             0x80000406
#define EXT_ERR_FS_READ_ONLY_FS                              0x80000407 /* Read-only file system */
#define EXT_ERR_FS_IO_ERROR                                  0x80000408
#define EXT_ERR_FS_NO_MORE_FILES                             0x80000409

/*****************************************************************************
* 3、驱动:错误码
*****************************************************************************/
/* 串口 */
#define EXT_ERR_UART_INVALID_PARAMETER                       0x80001000
#define EXT_ERR_UART_INVALID_SUSPEND                         0x80001001
#define EXT_ERR_UART_INVALID_PARITY                          0x80001002
#define EXT_ERR_UART_INVALID_DATA_BITS                       0x80001003
#define EXT_ERR_UART_INVALID_STOP_BITS                       0x80001004
#define EXT_ERR_UART_INVALID_BAUD                            0x80001005
#define EXT_ERR_UART_INVALID_COM_PORT                        0x80001006
#define EXT_ERR_UART_NOT_SUPPORT_DMA                         0x80001007

/* gpio */
#define EXT_ERR_GPIO_INVALID_PARAMETER                       0x80001040
#define EXT_ERR_GPIO_REPEAT_INIT                             0x80001041
#define EXT_ERR_GPIO_NOT_INIT                                0x80001042
#define EXT_ERR_GPIO_NOT_SUPPORT                             0x80001043

/* 看门狗 */
#define EXT_ERR_WATCHDOG_PARA_ERROR                          0x80001080

/* Flash */
#define EXT_ERR_FLASH_NOT_INIT                               0x800010C0
#define EXT_ERR_FLASH_INVALID_PARAM                          0x800010C1
#define EXT_ERR_FLASH_INVALID_PARAM_BEYOND_ADDR              0x800010C2
#define EXT_ERR_FLASH_INVALID_PARAM_SIZE_ZERO                0x800010C3
#define EXT_ERR_FLASH_INVALID_PARAM_ERASE_NOT_ALIGN          0x800010C4
#define EXT_ERR_FLASH_INVALID_PARAM_IOCTRL_DATA_NULL         0x800010C5
#define EXT_ERR_FLASH_INVALID_PARAM_DATA_NULL                0x800010C6
#define EXT_ERR_FLASH_INVALID_PARAM_PAD1                     0x800010C7
#define EXT_ERR_FLASH_INVALID_PARAM_PAD2                     0x800010C8
#define EXT_ERR_FLASH_INVALID_PARAM_PAD3                     0x800010C9
#define EXT_ERR_FLASH_INVALID_PARAM_PAD4                     0x800010CA
#define EXT_ERR_FLASH_TIME_OUT_WAIT_READY                    0x800010CB
#define EXT_ERR_FLASH_QUAD_MODE_READ_REG1                    0x800010CC
#define EXT_ERR_FLASH_QUAD_MODE_READ_REG2                    0x800010CD
#define EXT_ERR_FLASH_QUAD_MODE_COMPARE_REG                  0x800010CE
#define EXT_ERR_FLASH_NO_MATCH_FLASH                         0x800010CF
#define EXT_ERR_FLASH_WRITE_ENABLE                           0x800010D0
#define EXT_ERR_FLASH_NO_MATCH_ERASE_SIZE                    0x800010D1
#define EXT_ERR_FLASH_MAX_SPI_OP                             0x800010D2
#define EXT_ERR_FLASH_NOT_SUPPORT_IOCTRL_ID                  0x800010D3
#define EXT_ERR_FLASH_INVALID_CHIP_ID                        0x800010D4
#define EXT_ERR_FLASH_RE_INIT                                0x800010D5
#define EXT_ERR_FLASH_WRITE_NOT_SUPPORT_ERASE                0x800010D6
#define EXT_ERR_FLASH_WRITE_COMPARE_WRONG                    0x800010D7
#define EXT_ERR_FLASH_WAIT_CFG_START_TIME_OUT                0x800010D8
#define EXT_ERR_FLASH_PATITION_INIT_FAIL                     0x800010D9
#define EXT_ERR_FLASH_INITILIZATION                          0x800010DA
#define EXT_ERR_FLASH_ERASE_NOT_4K_ALIGN                     0x800010DB
#define EXT_ERR_FLASH_PROTECT_NOT_SUPPORT                    0x800010DC
#define EXT_ERR_FLASH_PROTECT_NOT_INIT                       0x800010DD
#define EXT_ERR_FLASH_PROTECT_RE_INIT                        0x800010DE
#define EXT_ERR_FLASH_PROTECT_NOT_FIND_CHIP                  0x800010DF
#define EXT_ERR_FLASH_MEMCPY_FAIL                            0x800010E0

/* HRTIMER定时器 */
#define EXT_ERR_HRTIMER_ALREADY_INIT                         0x80001100
#define EXT_ERR_HRTIMER_NOT_INIT                             0x80001101
#define EXT_ERR_HRTIMER_HAVE_NO_AVAILABLE_HANDLE             0x80001102
#define EXT_ERR_HRTIMER_NOT_CREATE_HANDLE                    0x80001103
#define EXT_ERR_HRTIMER_IN_START_STATUS                      0x80001104
#define EXT_ERR_HRTIMER_NOT_START                            0x80001105
#define EXT_ERR_HRTIMER_INVALID_ID                           0x80001106
#define EXT_ERR_HRTIMER_INVALID_PARAMETER                    0x80001107
#define EXT_ERR_HRTIMER_MALLOC_FAILUE                        0x80001108

/* hardware timer */
#define EXT_ERR_HWTIMER_INVALID_PARAMETER                    0x80001140
#define EXT_ERR_HWTIMER_INITILIZATION_ALREADY                0x80001141
#define EXT_ERR_HWTIMER_NO_INIT                              0x80001142

/* i2c */
#define EXT_ERR_I2C_NOT_INIT                                 0x80001180
#define EXT_ERR_I2C_INVALID_PARAMETER                        0x80001181
#define EXT_ERR_I2C_TIMEOUT_START                            0x80001182
#define EXT_ERR_I2C_TIMEOUT_WAIT                             0x80001183
#define EXT_ERR_I2C_TIMEOUT_STOP                             0x80001184
#define EXT_ERR_I2C_TIMEOUT_RCV_BYTE                         0x80001185
#define EXT_ERR_I2C_TIMEOUT_RCV_BYTE_PROC                    0x80001186
#define EXT_ERR_I2C_WAIT_SEM_FAIL                            0x80001187
#define EXT_ERR_I2C_START_ACK_ERR                            0x80001188
#define EXT_ERR_I2C_WAIT_ACK_ERR                             0x80001189

/* spi */
#define EXT_ERR_SPI_NOT_INIT                                 0x800011C0
#define EXT_ERR_SPI_REINIT                                   0x800011C1
#define EXT_ERR_SPI_PARAMETER_WRONG                          0x800011C2
#define EXT_ERR_SPI_BUSY                                     0x800011C3
#define EXT_ERR_SPI_WRITE_TIMEOUT                            0x800011C4
#define EXT_ERR_SPI_READ_TIMEOUT                             0x800011C5
#define EXT_ERR_SPI_NOT_SUPPORT_DMA                          0x800011C6

/* efuse */
#define EXT_ERR_EFUSE_INVALIDATE_ID                          0x80001200
#define EXT_ERR_EFUSE_INVALIDATE_PARA                        0x80001201
#define EXT_ERR_EFUSE_WRITE_ERR                              0x80001202
#define EXT_ERR_EFUSE_INVALIDATE_AUTH                        0x80001203
#define EXT_ERR_EFUSE_BUSY                                   0x80001204
#define EXT_ERR_EFUSE_TIMEOUT                                0x80001205

/* cipher */
#define EXT_ERR_CIPHER_NOT_INIT                              0x80001240
#define EXT_ERR_CIPHER_INVALID_POINT                         0x80001241
#define EXT_ERR_CIPHER_INVALID_PARAMETER                     0x80001242
#define EXT_ERR_CIPHER_NO_AVAILABLE_RNG                      0x80001243
#define EXT_ERR_CIPHER_FAILED_MEM                            0x80001244
#define EXT_ERR_CIPHER_OVERFLOW                              0x80001245
#define EXT_ERR_CIPHER_TIMEOUT                               0x80001246
#define EXT_ERR_CIPHER_UNSUPPORTED                           0x80001247
#define EXT_ERR_CIPHER_REGISTER_IRQ                          0x80001248
#define EXT_ERR_CIPHER_ILLEGAL_KEY                           0x80001249
#define EXT_ERR_CIPHER_INVALID_ADDR                          0x8000124A
#define EXT_ERR_CIPHER_INVALID_LENGTH                        0x8000124B
#define EXT_ERR_CIPHER_ILLEGAL_DATA                          0x8000124C
#define EXT_ERR_CIPHER_RSA_SIGN                              0x8000124D
#define EXT_ERR_CIPHER_RSA_VERIFY                            0x8000124E
#define EXT_ERR_CIPHER_RESULT_WARNING                        0x8000124F
#define EXT_ERR_CIPHER_FLUSH_DCACHE_FAILED                   0x80001250

/* sdio */
#define EXT_ERR_SDIO_INVALID_PARAMETER                       0x80001280

/* tsensor */
#define EXT_ERR_TSENSOR_INVALID_PARAMETER                    0x800012C0

/* adc */
#define EXT_ERR_ADC_PARAMETER_WRONG                          0x80001300
#define EXT_ERR_ADC_INVALID_CHANNEL_ID                       0x80001301
#define EXT_ERR_ADC_TIMEOUT                                  0x80001302
#define EXT_ERR_ADC_NOT_INIT                                 0x80001303

/* pmw */
#define EXT_ERR_PWM_NO_INIT                                  0x80001340
#define EXT_ERR_PWM_INITILIZATION_ALREADY                    0x80001341
#define EXT_ERR_PWM_INVALID_PARAMETER                        0x80001342


/* dma */
#define EXT_ERR_DMA_INVALID_PARA                             0x80001380
#define EXT_ERR_DMA_NOT_INIT                                 0x80001381
#define EXT_ERR_DMA_BUSY                                     0x80001382
#define EXT_ERR_DMA_TRANSFER_FAIL                            0x80001383
#define EXT_ERR_DMA_TRANSFER_TIMEOUT                         0x80001384
#define EXT_ERR_DMA_GET_NOTE_FAIL                            0x80001385
#define EXT_ERR_DMA_LLI_NOT_CREATE                           0x80001386
#define EXT_ERR_DMA_CH_IRQ_ENABLE_FAIL                       0x80001387
/* audio */
#define EXT_ERR_AUDIO_BUSY                                   0x800013C0
#define EXT_ERR_AUDIO_INVALID_PARAMETER                      0x800013C1

/* i2s */
#define EXT_ERR_I2S_INVALID_PARAMETER                        0x80001400
#define EXT_ERR_I2S_WRITE_TIMEOUT                            0x80001401

/*****************************************************************************
* 4、中间应用:错误码
*****************************************************************************/
/* NV */
#define EXT_ERR_NV_FILE_ERR                                  0x80003000
#define EXT_ERR_NV_MEMCPY_FAIL                               0x80003001
#define EXT_ERR_NV_WRITE_FILE_FAIL                           0x80003002
#define EXT_ERR_NV_UPDATA_DATA_FAIL                          0x80003003
#define EXT_ERR_NV_UPDATA_FILE_FAIL                          0x80003004
#define EXT_ERR_NV_NOT_SUPPORT_WRITE                         0x80003005
#define EXT_ERR_NV_FSEC_TOTAL_NUM_INVALID                    0x80003006 /* 工厂NV项个数非法 */
#define EXT_ERR_NV_FAIL_N_TIMES                              0x80003007
#define EXT_ERR_NV_SEM_FAIL                                  0x80003008
#define EXT_ERR_NV_LEN_ERR                                   0x80003009
#define EXT_ERR_NV_NOT_FOUND                                 0x8000300A
#define EXT_ERR_NV_FULL                                      0x8000300B
#define EXT_ERR_NV_NOT_ENOUGH_MEMORY                         0x8000300C
#define EXT_ERR_NV_NOT_SUPPORT                               0x8000300D
#define EXT_ERR_NV_NOT_SUPPORT_ID                            0x8000300E
#define EXT_ERR_NV_BAD_DATA                                  0x8000300F
#define EXT_ERR_NV_INVALID_TYPE                              0x80003010
/* NV读取失败" "Read NVIM Failure" */
#define EXT_ERR_NV_ERROR_READ                                0x80003011
/* NV写失败，长度过长""Write Error for Length Overflow" */
#define EXT_ERR_NV_NOT_SUPPORT_LENTH                         0x80003012
/* NV写失败,Flash坏块" "Write Error for Flash Bad Block" */
#define EXT_ERR_NV_BAD_BLOCK                                 0x80003013
/* NV写失败,其他错误" "Write Error for Unknown Reason" */
#define EXT_ERR_NV_ERROR_WRITE                               0x80003014
#define EXT_ERR_NV_INITILIZATION                             0x80003015
#define EXT_ERR_NV_INVALID_PARAMETER                         0x80003016

/* 低功耗 */
#define EXT_ERR_LOWPOWER_INVALID_PARAMETER                   0x80003040

/* upgrade common error */
#define EXT_ERR_UPG_COMMON                                   0x80003060
#define EXT_ERR_UPG_NULL_POINTER                             (EXT_ERR_UPG_COMMON + 0x0)
#define EXT_ERR_UPG_PARAMETER                                (EXT_ERR_UPG_COMMON + 0x1)
#define EXT_ERR_UPG_BACKUP_ADDR                              (EXT_ERR_UPG_COMMON + 0x2)
#define EXT_ERR_UPG_BUSY                                     (EXT_ERR_UPG_COMMON + 0x3)
#define EXT_ERR_UPG_FLASH_BAD                                (EXT_ERR_UPG_COMMON + 0x4)
#define EXT_ERR_UPG_START_ADDR                               (EXT_ERR_UPG_COMMON + 0x5)
#define EXT_ERR_UPG_INITILIZATION_ALREADY                    (EXT_ERR_UPG_COMMON + 0x6)
#define EXT_ERR_UPG_FILE_LEN                                 (EXT_ERR_UPG_COMMON + 0x7)
#define EXT_ERR_UPG_NOT_START                                (EXT_ERR_UPG_COMMON + 0x8)
#define EXT_ERR_UPG_MALLOC_FAIL                              (EXT_ERR_UPG_COMMON + 0x9)
#define EXT_ERR_UPG_GET_SECTION_HEAD                         (EXT_ERR_UPG_COMMON + 0xA)
#define EXT_ERR_UPG_BUF_LEN                                  (EXT_ERR_UPG_COMMON + 0xB)
#define EXT_ERR_UPG_FLASH_SIZE                               (EXT_ERR_UPG_COMMON + 0xC)
#define EXT_ERR_UPG_NV_SIZE                                  (EXT_ERR_UPG_COMMON + 0xD)
#define EXT_ERR_UPG_ALREADY_FINISH                           (EXT_ERR_UPG_COMMON + 0xE)
#define EXT_ERR_UPG_RSA_KEY_ADDR                             (EXT_ERR_UPG_COMMON + 0xF)
#define EXT_ERR_UPG_ECC_KEY_ADDR                             (EXT_ERR_UPG_COMMON + 0x10)
#define EXT_ERR_UPG_FILE_LEN_OVER                            (EXT_ERR_UPG_COMMON + 0x11)
#define EXT_ERR_UPG_STOP                                     (EXT_ERR_UPG_COMMON + 0x12)
#define EXT_ERR_UPG_LOW_FIRMWARE_VER                         (EXT_ERR_UPG_COMMON + 0x13)
#define EXT_ERR_UPG_FULL_FIRMWARE_VER                        (EXT_ERR_UPG_COMMON + 0x14)
#define EXT_ERR_UPG_LOW_BOOT_VER                             (EXT_ERR_UPG_COMMON + 0x15)
#define EXT_ERR_UPG_FULL_BOOT_VER                            (EXT_ERR_UPG_COMMON + 0x16)
#define EXT_ERR_UPG_FIRST_PACKET_OFFSET                      (EXT_ERR_UPG_COMMON + 0x17)
#define EXT_ERR_UPG_MEMCPY_FAIL                              (EXT_ERR_UPG_COMMON + 0x18)

/* upgrade file check error */
#define EXT_ERR_UPG_CHECK                                     0x80003080
#define EXT_ERR_UPG_IMAGE_ID                                 (EXT_ERR_UPG_CHECK + 0x0)
#define EXT_ERR_UPG_FILE_TYPE                                (EXT_ERR_UPG_CHECK + 0x1)
#define EXT_ERR_UPG_HEAD_LEN                                 (EXT_ERR_UPG_CHECK + 0x2)
#define EXT_ERR_UPG_SIGN_ALG                                 (EXT_ERR_UPG_CHECK + 0x3)
#define EXT_ERR_UPG_RSA_KEY_LEN                              (EXT_ERR_UPG_CHECK + 0x4)
#define EXT_ERR_UPG_RSA_HEAD_SIGN                            (EXT_ERR_UPG_CHECK + 0x5)
#define EXT_ERR_UPG_ECC_KEY_LEN                              (EXT_ERR_UPG_CHECK + 0x6)
#define EXT_ERR_UPG_ECC_HEAD_SIGN                            (EXT_ERR_UPG_CHECK + 0x7)
#define EXT_ERR_UPG_COMMON_SHA256                            (EXT_ERR_UPG_CHECK + 0x8)
#define EXT_ERR_UPG_SECTION_SHA256                           (EXT_ERR_UPG_CHECK + 0x9)
#define EXT_ERR_UPG_FIRMWARE_VER                             (EXT_ERR_UPG_CHECK + 0xA)
#define EXT_ERR_UPG_BOOT_VER                                 (EXT_ERR_UPG_CHECK + 0xB)
#define EXT_ERR_UPG_BOOT_HEAD                                (EXT_ERR_UPG_CHECK + 0xC)
#define EXT_ERR_UPG_BOOT_LEN                                 (EXT_ERR_UPG_CHECK + 0xD)
#define EXT_ERR_UPG_BOOT_ROOT_KEY                            (EXT_ERR_UPG_CHECK + 0xE)
#define EXT_ERR_UPG_BOOT_ROOT_KEY_LEN                        (EXT_ERR_UPG_CHECK + 0xF)
#define EXT_ERR_UPG_BOOT_KEY_ID                              (EXT_ERR_UPG_CHECK + 0x10)
#define EXT_ERR_UPG_BOOT_SIGN_ALG                            (EXT_ERR_UPG_CHECK + 0x11)
#define EXT_ERR_UPG_BOOT_SUB_KEY                             (EXT_ERR_UPG_CHECK + 0x12)
#define EXT_ERR_UPG_BOOT_SUB_KEY_CAT                         (EXT_ERR_UPG_CHECK + 0x13)
#define EXT_ERR_UPG_BOOT_SUB_KEY_RSIM                        (EXT_ERR_UPG_CHECK + 0x14)
#define EXT_ERR_UPG_BOOT_DIE_ID                              (EXT_ERR_UPG_CHECK + 0x15)
#define EXT_ERR_UPG_BOOT_HASH                                (EXT_ERR_UPG_CHECK + 0x16)
#define EXT_ERR_UPG_BOOT_SUB_KEY_LEN                         (EXT_ERR_UPG_CHECK + 0x17)

/* DIAG */
#define EXT_ERR_DIAG_NOT_FOUND                               0x800030C0
#define EXT_ERR_DIAG_INVALID_ID                              0x800030C1
#define EXT_ERR_DIAG_FULL                                    0x800030C2
#define EXT_ERR_DIAG_CONSUMED                                0x800030C3
#define EXT_ERR_DIAG_CONTINUE                                0x800030C4
#define EXT_ERR_DIAG_TOO_SMALL_BUFFER                        0x800030C5
#define EXT_ERR_DIAG_NO_MORE_DATA                            0x800030C6
#define EXT_ERR_DIAG_NOT_ENOUGH_MEMORY                       0x800030C7
#define EXT_ERR_DIAG_INVALID_HEAP_ADDR                       0x800030C8
#define EXT_ERR_DIAG_NOT_CONNECT                             0x800030C9
#define EXT_ERR_DIAG_BUSY                                    0x800030CA
#define EXT_ERR_DIAG_TOO_LARGE_FRAME                         0x800030CB
#define EXT_ERR_DIAG_RAM_ALIGN                               0x800030CC
#define EXT_ERR_DIAG_NOT_SUPPORT                             0x800030CD
#define EXT_ERR_DIAG_UNAVAILABLE                             0x800030CE
#define EXT_ERR_DIAG_CFG_NOT_ALLOW                           0x800030CF
#define EXT_ERR_DIAG_INVALID_CODE_ADDR                       0x800030D0
#define EXT_ERR_DIAG_OBJ_NOT_FOUND                           0x800030D1
#define EXT_ERR_DIAG_QUEUE_FULL                              0x800030D2
#define EXT_ERR_DIAG_NO_MORE_MEMORY                          0x800030D3
#define EXT_ERR_DIAG_SYSTEM_CALL_ERROR                       0x800030D4
#define EXT_ERR_DIAG_NO_INIT                                 0x800030D5
#define EXT_ERR_DIAG_INVALID_PARAMETER                       0x800030D6
#define EXT_ERR_DIAG_STAT_NOT_SUPPORT                        0x800030D7
#define EXT_ERR_DIAG_ID_OR_CALLBACK_ALREADY_REGISTERED       0x800030D8
#define EXT_ERR_DIAG_SET_CONN_ACK_INFO                       0x800030D9
#define EXT_ERR_DIAG_CMD_NUM_EXCEED_UPPER_LIMIT              0x800030DA
#define EXT_ERR_DIAG_MEMCPY_FAIL                             0x800030DB

/* reset 复位 */
#define EXT_ERR_RESET_TOO_LARGE_DATA                         0x80003100
#define EXT_ERR_RESET_INVALID_PARAMETER                      0x80003101

/* syserror */
#define EXT_ERR_SYSERROR_NOT_FOUND                           0x80003140
#define EXT_ERR_SYSERROR_INVALID_PARAMETER                   0x80003141


/* APP */
#define EXT_ERR_APP_INITILIZATION_ALREADY                    0x80003180
#define EXT_ERR_APP_INVALID_PARAMETER                        0x80003181

/* CRC */
#define EXT_ERR_CRC_INVALID_PARAMETER                        0x800031C0


/* sigma */
#define EXT_ERR_SIGMA_INVALID_PARAMETER                      0x80003200

/* data collect */
#define EXT_ERR_DATACOLLECT_INVALID_PARAMETER                0x80003240
#define EXT_ERR_DATACOLLECT_BUSY                             0x80003241

/* AT */
#define EXT_ERR_AT_NAME_OR_FUNC_REPEAT_REGISTERED           0x80003280
#define EXT_ERR_AT_INVALID_PARAMETER                        0x80003281

/*****************************************************************************
* 5、协议栈:错误码
*****************************************************************************/
/* wifi */
/* 说明:wifi错误码ext_err_code_enum 将base基值修改为起始值，其他错误码一次延顺 */
/* dmac */
#define EXT_ERR_WIFI_DMAC_NOT_SUPPORT                        0x80004000

/* hmac */
#define EXT_ERR_WIFI_HMAC_INVALID_PARAMETER                  0x80004040

/* wal */
#define EXT_ERR_WIFI_WAL_MALLOC_FAIL                         0x80004080
#define EXT_ERR_WIFI_WAL_FAILURE                             0x80004081
#define EXT_ERR_WIFI_WAL_BUSY                                0x80004082
#define EXT_ERR_WIFI_WAL_INVALID_PARAMETER                   0x80004083

/*****************************************************************************
* 6、保留 unit128个
* 7、客户使用预留 unit64个
*****************************************************************************/
#endif /* EXT_ERRNO_H */
