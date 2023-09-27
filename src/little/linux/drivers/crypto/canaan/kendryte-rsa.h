/* Copyright (c) 2023, Canaan Bright Sight Co., Ltd
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
*/

#ifndef __KENDRYTE_RSA_T__
#define __KENDRYTE_RSA_T__

#define PKC_BUFFER_SIZE                     512
#define ECP_MPMAC_SIZE                      16
#define ECP_MPROG_SIZE                      256

//----------------------RSA REGISTER OFFSET---------------------------------------//
#define ECP_CTRL_OFFSET                     0x8
#define ECP_STATUS_OFFSET                   0xc
#define ECP_EC_OFFSET                       0x100
#define ECP_E_SHORT_OFFSET                  0x110
#define ECP_MAC_OFFSET                      0x1f0
#define ECP_PROGRAM_OFFSET                  0x200
#define ECP_DATA_OFFSET                     0x300

//----------------------RSA REGISTER BITS---------------------------------------//
#define ECP_ECP_EC_FIELD_BITS               8
//----------------------RSA REGISTER BIT MASKS----------------------------------//
#define ECP_STATUS_BUSY_MASK                0x00000001
#define ECP_STATUS_MPROG_MASK               0x00000002


/**
 * @brief Status code
 */
typedef enum {
    SUCCESS,     ///< Success.
    E_ALIGN,     ///< Address alignment mismatch.
    E_OVERFLOW,  ///< Space overflow.
    E_UNDERFLOW, ///< Size too small.
    E_INVALID,   ///< Invalid argument.
    E_BUSY,      ///< Resource is occupied.
    E_UNAVAIL,   ///< Resource is unavailable.
    E_FIRMWARE,  ///< Firmware error.
    E_VERFAIL,   ///< Invalid public key or digital signature.
    E_ECMPROG,   ///< Invalid ECC microprogram.
    E_DENY,      ///< Access denied.
    E_UNSUPPORT, ///< Not support.
    E_INFINITY,  ///< Point at infinity.
    E_ERROR,     ///< Unspecific error.
} kendryte_status_t;

/**
 * @brief CMAC of RSA micro programs
 */
typedef struct
{
    const void *prk;
    const void *puk;
} kendryte_rsa_mprog_cmac_st;

/**
 * @brief Functions of RSA micro programs
 */
typedef struct
{
    const void *prk;
    const void *puk;
} kendryte_rsa_mprog_func_st;

/**
 * @brief RSA micro programs
 */
 typedef struct
{
    kendryte_rsa_mprog_cmac_st **cmac;
    kendryte_rsa_mprog_func_st *func;
} kendryte_rsa_mprog_st;

typedef enum {
    P1024 = 0x85,
    P2048,
    P3072,
    P4096,
    N_ECP_FIELD_T,
} kendryte_ecp_field_t;

typedef enum {
    RSA1024,      ///< RSA-1024
    RSA2048,      ///< RSA-2048
    RSA3072,      ///< RSA-3072
    RSA4096,      ///< RSA-4096
    N_RSA_TYPE_T, // keep in the last one
} kendryte_rsa_type_t;

typedef enum {
    ECP_UNSUPPORTED = -1,
    ECP_ECF39303,
    ECP_ECF09303,
} kendryte_ecp_version_t;

typedef enum {
    MP_UNSUPPORTED = -1,
    MP_00000000,
    MP_3EE2DE76,
} kendryte_mp_version_t;

struct kendryte_rsa_dev {
    void __iomem *base;
    struct device *dev;
    struct clk *clk;
    struct reset_control *rst;
};

struct kendryte_rsa_ctx {
    struct kendryte_rsa_dev *dev;

    uint8_t n[512];
    uint32_t e;
    uint8_t d[512];
    uint8_t key[2048];
    uint32_t key_sz;

    kendryte_rsa_type_t rsatype;
};

#endif  /* __KENDRYTE_RSA_T__ */