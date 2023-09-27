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

#include "buf_mgt.hpp"

using namespace std;

int buf_mgt_idle_get(buf_mgt_t *pbuf_mgt, void **ppdata)
{
    int ret = 0;

    pbuf_mgt->mtx.lock();
    if (pbuf_mgt->idle_q.empty())
    {
        ret = -1;
    }
    else
    {
        *ppdata = pbuf_mgt->idle_q.front();
        pbuf_mgt->idle_q.pop();
    }
    pbuf_mgt->mtx.unlock();

    return ret;
}

int buf_mgt_idle_put(buf_mgt_t *pbuf_mgt, void *pdata)
{
    int ret = 0;

    pbuf_mgt->mtx.lock();
    pbuf_mgt->idle_q.push(pdata);
    pbuf_mgt->mtx.unlock();

    return ret;
}

int buf_mgt_valid_get(buf_mgt_t *pbuf_mgt, void **ppdata)
{
    int ret = 0;

    pbuf_mgt->mtx.lock();
    if (pbuf_mgt->valid_q.empty())
    {
        ret = -1;
    }
    else
    {
        *ppdata = pbuf_mgt->valid_q.front();
        pbuf_mgt->valid_q.pop();
    }
    pbuf_mgt->mtx.unlock();

    return ret;
}

int buf_mgt_valid_put(buf_mgt_t *pbuf_mgt, void *pdata)
{
    int ret = 0;

    pbuf_mgt->mtx.lock();
    pbuf_mgt->valid_q.push(pdata);
    pbuf_mgt->mtx.unlock();

    return ret;
}

int buf_mgt_writer_get(buf_mgt_t *pbuf_mgt, void **ppdata, int level)
{
    int ret = 0;

    pbuf_mgt->mtx.lock();
    if (pbuf_mgt->idle_q.empty())
    {
        if (pbuf_mgt->valid_q.size() <= level)
        {
            ret = -1;
        }
        else
        {
            *ppdata = pbuf_mgt->valid_q.front();
            pbuf_mgt->valid_q.pop();
            ret = 1;
        }
    }
    else
    {
        *ppdata = pbuf_mgt->idle_q.front();
        pbuf_mgt->idle_q.pop();
    }
    pbuf_mgt->mtx.unlock();

    return ret;
}

int buf_mgt_writer_put(buf_mgt_t *pbuf_mgt, void *pdata)
{
    return buf_mgt_valid_put(pbuf_mgt, pdata);
}

int buf_mgt_reader_get(buf_mgt_t *pbuf_mgt, void **ppdata, int flag)
{
    int ret = 0;

    pbuf_mgt->mtx.lock();
    if (pbuf_mgt->valid_q.empty())
    {
        *ppdata = flag ? pbuf_mgt->pdata_last : (void *) -1;
        if (*ppdata == (void *) -1)
            ret = -1;
        else
            ret = 1;
    }
    else
    {
        *ppdata = pbuf_mgt->valid_q.front();
        pbuf_mgt->valid_q.pop();
    }
    if (pbuf_mgt->pdata_last_last != pbuf_mgt->pdata_last)
    {
        if (pbuf_mgt->pdata_last_last != (void *) -1)
            pbuf_mgt->idle_q.push(pbuf_mgt->pdata_last_last);
    }
    pbuf_mgt->pdata_last_last = pbuf_mgt->pdata_last;
    pbuf_mgt->pdata_last = *ppdata;
    pbuf_mgt->mtx.unlock();

    return ret;
}

int buf_mgt_reader_put(buf_mgt_t *pbuf_mgt, void *pdata)
{
    return buf_mgt_idle_put(pbuf_mgt, pdata);
}
