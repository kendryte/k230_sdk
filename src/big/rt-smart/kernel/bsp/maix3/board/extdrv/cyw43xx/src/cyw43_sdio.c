/*
 * This file is part of the cyw43-driver
 *
 * Copyright (C) 2019-2022 George Robotics Pty Ltd
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 * 3. Any redistribution, use, or modification in source or binary form is done
 *    solely for personal benefit and not for any commercial purpose or for
 *    monetary gain.
 *
 * THIS SOFTWARE IS PROVIDED BY THE LICENSOR AND COPYRIGHT OWNER "AS IS" AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE LICENSOR OR COPYRIGHT OWNER BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * This software is also available for use with certain devices under different
 * terms, as set out in the top level LICENSE file.  For commercial licensing
 * options please email contact@georgerobotics.com.au.
 */

#include <stdbool.h>

#include "cyw43.h"
#include "cyw43_internal.h"
#include "cyw43_sdio.h"

#if !CYW43_USE_SPI

// Performs an SDIO CMD52 transaction.
// On success returns the response byte (0-255).
// On error returns a negative errno code.
static int cyw43_sdio_cmd52(bool write, uint32_t fn, uint32_t addr, uint32_t val) {
    return cyw43_sdio_transfer_cmd52(fn, write, addr, val);
}

static int cyw43_sdio_cmd53(bool write, uint32_t fn, uint32_t addr, size_t len, uint8_t *buf) {
    return cyw43_sdio_transfer_cmd53(fn, write, addr, 1, buf, len);
}

int cyw43_read_bytes(cyw43_int_t *self, uint32_t fn, uint32_t addr, size_t len, uint8_t *buf) {
    return cyw43_sdio_cmd53(false, fn, addr, len, buf);
}

int cyw43_write_bytes(cyw43_int_t *self, uint32_t fn, uint32_t addr, size_t len, const uint8_t *buf) {
    return cyw43_sdio_cmd53(true, fn, addr, len, (uint8_t *)buf);
}

int cyw43_read_reg_u8(cyw43_int_t *self, uint32_t fn, uint32_t reg) {
    return cyw43_sdio_cmd52(false, fn, reg, 0);
}

uint32_t cyw43_read_reg_u32(cyw43_int_t *self, uint32_t fn, uint32_t reg) {
    uint32_t val = 0;
    cyw43_sdio_cmd53(false, fn, reg, 4, (void *)&val);
    return val;
}

int cyw43_write_reg_u8(cyw43_int_t *self, uint32_t function, uint32_t reg, uint32_t val) {
    return cyw43_sdio_cmd52(true, function, reg, val);
}

int cyw43_write_reg_u32(cyw43_int_t *self, uint32_t function, uint32_t reg, uint32_t val) {
    return cyw43_sdio_cmd53(true, function, reg, 4, (void *)&val);
}

#endif
