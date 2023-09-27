/* Copyright (c) 2022, Canaan Bright Sight Co., Ltd
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

#ifndef _BUF_MGT_H_
#define _BUF_MGT_H_

#include <stdint.h>
#include <mutex>
#include <queue>

typedef struct
{
    std::mutex mtx;
    std::queue<void *> valid_q;
    std::queue<void *> idle_q;
    void *pdata_last = (void *) -1;
    void *pdata_last_last = (void *) -1;
} buf_mgt_t;

int buf_mgt_idle_get(buf_mgt_t *pbuf_mgt, void **ppdata);
int buf_mgt_idle_put(buf_mgt_t *pbuf_mgt, void *pdata);
int buf_mgt_valid_get(buf_mgt_t *pbuf_mgt, void **ppdata);
int buf_mgt_valid_put(buf_mgt_t *pbuf_mgt, void *pdata);
int buf_mgt_writer_get(buf_mgt_t *pbuf_mgt, void **ppdata, int level);
int buf_mgt_writer_put(buf_mgt_t *pbuf_mgt, void *pdata);
int buf_mgt_reader_get(buf_mgt_t *pbuf_mgt, void **ppdata, int flag);
int buf_mgt_reader_put(buf_mgt_t *pbuf_mgt, void *pdata);

#endif
