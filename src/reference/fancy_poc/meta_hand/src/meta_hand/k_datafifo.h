/**
 * @file k_datafifo.h
 * @author  ()
 * @brief
 * @version 1.0
 * @date 2023-06-12
 *
 * @copyright
 * Copyright (c) 2023, Canaan Bright Sight Co., Ltd
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
#ifndef __K_DATAFIFO_H__
#define __K_DATAFIFO_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "k_type.h"

/** \addtogroup     DATAFIFO*/
/** @{ */  /** <!-- [DATAFIFO] */

typedef k_u64 k_datafifo_handle;
#define K_DATAFIFO_INVALID_HANDLE (-1)

/** Datafifo Error number base */
#define K_DATAFIFO_ERRNO_BASE 0x1A00
/** Parameter is invalid */
#define K_DATAFIFO_ERR_EINVAL_PAEAMETER    (K_DATAFIFO_ERRNO_BASE + 1)
/** Null pointer*/
#define K_DATAFIFO_ERR_NULL_PTR            (K_DATAFIFO_ERRNO_BASE + 2)
/** failure caused by malloc memory */
#define K_DATAFIFO_ERR_NOMEM               (K_DATAFIFO_ERRNO_BASE + 3)
/** failure caused by device operation */
#define K_DATAFIFO_ERR_DEV_OPT             (K_DATAFIFO_ERRNO_BASE + 4)
/** operation is not permitted, Reader to write or Writer to read */
#define K_DATAFIFO_ERR_NOT_PERM            (K_DATAFIFO_ERRNO_BASE + 5)
/** data buffer is empty, no data to read*/
#define K_DATAFIFO_ERR_NO_DATA             (K_DATAFIFO_ERRNO_BASE + 6)
/** data buffer is full, no space to write*/
#define K_DATAFIFO_ERR_NO_SPACE            (K_DATAFIFO_ERRNO_BASE + 7)
/** read error*/
#define K_DATAFIFO_ERR_READ                (K_DATAFIFO_ERRNO_BASE + 8)
/** write error*/
#define K_DATAFIFO_ERR_WRITE               (K_DATAFIFO_ERRNO_BASE + 9)


/**
 * @brief Stream release callback. when bDataReleaseByWriter is K_TRUE,
 * The writer should register this function and do release in this function.
 */
typedef void (*K_DATAFIFO_RELEASESTREAM_FN_PTR)(void* pStream);

/** Role of caller*/
typedef enum k_DATAFIFO_OPEN_MODE_E
{
    DATAFIFO_READER,
    DATAFIFO_WRITER
} K_DATAFIFO_OPEN_MODE_E;

/** DATAFIFO parameters */
typedef struct k_DATAFIFO_PARAMS_S
{
    k_u32 u32EntriesNum; /**< The number of items in the ring buffer*/
    k_u32 u32CacheLineSize; /**< Item size*/
    k_bool bDataReleaseByWriter; /**<Whether the data buffer release by writer*/
    K_DATAFIFO_OPEN_MODE_E enOpenMode; /**<READER or WRITER*/
} k_datafifo_params_s;

/** DATAFIFO advanced function */
typedef enum k_DATAFIFO_CMD_E
{
    DATAFIFO_CMD_GET_PHY_ADDR, /**<Get the physic address of ring buffer*/
    DATAFIFO_CMD_READ_DONE, /**<When the read buffer read over, the reader should call this function to notify the writer*/
    DATAFIFO_CMD_WRITE_DONE, /**<When the writer buffer is write done, the writer should call this function*/
    DATAFIFO_CMD_SET_DATA_RELEASE_CALLBACK, /**<When bDataReleaseByWriter is K_TRUE, the writer should call this to register release callback*/
    DATAFIFO_CMD_GET_AVAIL_WRITE_LEN, /**<Get available write length*/
    DATAFIFO_CMD_GET_AVAIL_READ_LEN /**<Get available read length*/
} k_datafifo_cmd_e;

/**
 * @brief This function malloc ring buffer and initialize DATAFIFO module.
 * when one side call this, the other side should call ::kd_datafifo_open_by_addr.
 * @param[out] handle Handle of DATAFIFO.
 * @param[in] pstParams Parameters of DATAFIFO.
 * @return K_SUCCESS Initialize DATAFIFO success.
 * @return K_FAILED Initialize DATAFIFO fail.
 */
k_s32 kd_datafifo_open(k_datafifo_handle* Handle, k_datafifo_params_s* pstParams);

/**
 * @brief This function map the ring buffer physic address to its virtue address and initialize DATAFIFO module.
 * This function should be called after the other side call ::kd_datafifo_open because it need physic address of ring buffer.
 * @param[out] handle Handle of DATAFIFO.
 * @param[in] pstParams Parameters of DATAFIFO.
 * @param[in] u32PhyAddr Physic address of ring buffer. Get it from the other side.
 * @return K_SUCCESS Initialize DATAFIFO success.
 * @return K_FAILED Initialize DATAFIFO fail.
 */
k_s32 kd_datafifo_open_by_addr(k_datafifo_handle* Handle, k_datafifo_params_s* pstParams, k_u64 u64PhyAddr);

/**
 * @brief This function will free or unmap ring buffer and deinitialize DATAFIFO.
 * @param[in] handle Handle of DATAFIFO.
 * @return K_SUCCESS Close success.
 * @return K_FAILED Close fail.
 */
k_s32 kd_datafifo_close(k_datafifo_handle Handle);

/**
 * @brief Read data from ring buffer and save it to ppData.
 * every read buffer size is ::u32CacheLineSize
 * @param[in] handle Handle of DATAFIFO.
 * @param[out] ppData Item read.
 * @return K_SUCCESS Read success.
 * @return K_FAILED Read fail.
 */
k_s32 kd_datafifo_read(k_datafifo_handle Handle, void** ppData);

/**
 * @brief Write data to ring buffer. data size should be ::u32CacheLineSize
 * @param[in] handle Handle of DATAFIFO.
 * @param[in] pData Item to write.
 * @return K_SUCCESS Write success.
 * @return K_FAILED Write fail.
 */
k_s32 kd_datafifo_write(k_datafifo_handle Handle, void* pData);

/**
 * @brief Advanced function. see ::k_datafifo_cmd_e
 * @param[in] handle Handle of DATAFIFO.
 * @param[in] enCMD Command.
 * @param[in,out] arg Input or output argument.
 * @return K_SUCCESS Call function success.
 * @return K_FAILED Call function fail.
 */
k_s32 kd_datafifo_cmd(k_datafifo_handle Handle, k_datafifo_cmd_e enCMD, void* pArg);

/** @}*/  /** <!-- ==== DATAFIFO End ====*/

#ifdef __cplusplus
}
#endif

#endif
