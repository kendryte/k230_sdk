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

/*
 * The main logic about frame cache module
 */

#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include <unistd.h>
#include <malloc.h>
#include "frame_cache.h"
#include <pthread.h>
#include "log.h"

extern unsigned int g_imagesize;
extern unsigned int g_cache_count;

static void cache_init(frame_cache_t *cache)
{
    cache->tail  = 0;
    cache->head  = 0;
    cache->count = 0;
}

static void cache_destroy(frame_cache_t *cache)
{
    while (cache->tail != 0)
    {
        frame_node_t* tmp = cache->tail;
        cache->tail = tmp->next;
        free(tmp);
    }

    cache->tail  = 0;
    cache->head  = 0;
    cache->count = 0;
}

static int put_frame_into_cache(frame_cache_t *cache, frame_node_t* node)
{
    node->next = 0;
    cache->count++;

    // first time
    if ((cache->head == 0) && (cache->tail == 0))
    {
        cache->head = node;
        cache->tail = node;
    }
    else
    {
        cache->head->next = node;
        cache->head = node;
    }

    return 0;
}

static int get_frame_from_cache(frame_cache_t *cache, frame_node_t** node)
{
    *node = cache->tail;

    if ((cache->tail != 0) && (cache->tail->next != 0))
    {
        cache->tail = cache->tail->next;
        cache->count--;
    }
    else if (cache->tail == cache->head)
    {
        cache->tail  = 0;
        cache->head  = 0;
        cache->count = 0;
    }

    return (*node != 0) ? 0 : -1;
}

static int free_queue(frame_queue_t *queue)
{
    frame_cache_t *cache = queue->cache;

    if (cache != 0)
    {
        cache_destroy(cache);
        free(cache);
        queue->cache = 0;
    }

    pthread_mutex_destroy(&(queue->locker));
    pthread_cond_destroy(&(queue->waiter));

    return 0;
}

static int free_uvc_cache(uvc_cache_t* uvc_cache)
{
    /* free queue*/
    free_queue(uvc_cache->ok_queue);
    free_queue(uvc_cache->free_queue);

    free(uvc_cache->ok_queue);
    free(uvc_cache->free_queue);

    return 0;
}

static int free_uac_cache(uac_cache_t* uac_cache)
{
    /* free queue*/
    free_queue(uac_cache->ok_queue);
    free_queue(uac_cache->free_queue);

    free(uac_cache->ok_queue);
    free(uac_cache->free_queue);

    return 0;
}


static int init_queue(frame_queue_t **queue)
{
    frame_queue_t *q = *queue;

    if (q == 0)
    {
        goto ERR;
    }

    q->cache = (frame_cache_t*)malloc(sizeof(frame_cache_t));

    if (q->cache == 0)
    {
        LOG("malloc frame cache failure\n");

        goto ERR;
    }

    cache_init(q->cache);

    /*FIXME: recursive locker....*/
    pthread_mutex_init(&(q->locker), 0);
    pthread_cond_init(&(q->waiter), 0);

    return 0;

ERR:
    return -1;
}

static int init_uvc_frame_cache(uvc_cache_t** cache)
{
    uvc_cache_t *uvc_cache = *cache;

    if (uvc_cache == 0)
    {
        LOG("the uvc cache is null\n");

        goto ERR;
    }

    uvc_cache->ok_queue   = 0;
    uvc_cache->free_queue = 0;

    uvc_cache->ok_queue = (frame_queue_t*)malloc(sizeof (frame_queue_t));

    if (uvc_cache->ok_queue == 0)
    {
        LOG("malloc ok_queue failure \n");

        goto ERR;
    }

    uvc_cache->free_queue = (frame_queue_t*)malloc(sizeof (frame_queue_t));

    if (uvc_cache->free_queue == 0)
    {
        LOG("malloc free_queue failure\n");

        goto ERR;
    }

    if (init_queue(&(uvc_cache->ok_queue)) < 0)
    {
        goto ERR;
    }

    if (init_queue(&(uvc_cache->free_queue)) < 0)
    {
        goto ERR;
    }

    return 0;

ERR:
    if (uvc_cache->ok_queue != 0)
    {
        free(uvc_cache->ok_queue);
    }

    if (uvc_cache->free_queue != 0)
    {
        free(uvc_cache->free_queue);
    }

    uvc_cache->ok_queue   = 0;
    uvc_cache->free_queue = 0;

    return -1;
}


static int init_uac_frame_cache(uac_cache_t** cache)
{
    uac_cache_t *uac_cache = *cache;

    if (uac_cache == 0)
    {
        printf("the uac cache is null\n");

        goto ERR;
    }

    uac_cache->ok_queue   = 0;
    uac_cache->free_queue = 0;

    uac_cache->ok_queue = (frame_queue_t*)malloc(sizeof (frame_queue_t));

    if (uac_cache->ok_queue == 0)
    {
        printf("malloc ok_queue failure \n");

        goto ERR;
    }

    uac_cache->free_queue = (frame_queue_t*)malloc(sizeof (frame_queue_t));

    if (uac_cache->free_queue == 0)
    {
        printf("malloc free_queue failure\n");

        goto ERR;
    }

    if (init_queue(&(uac_cache->ok_queue)) < 0)
    {
        goto ERR;
    }

    if (init_queue(&(uac_cache->free_queue)) < 0)
    {
        goto ERR;
    }

    return 0;

ERR:
    if (uac_cache->ok_queue != 0)
    {
        free(uac_cache->ok_queue);
    }

    if (uac_cache->free_queue != 0)
    {
        free(uac_cache->free_queue);
    }

    uac_cache->ok_queue   = 0;
    uac_cache->free_queue = 0;

    return -1;
}


static void clear_frame_cache(frame_cache_t *c)
{
    cache_destroy(c);
}

static void clear_queue(frame_queue_t *q)
{
    if (0 != pthread_mutex_lock(&(q->locker)))
    {
        LOG("failed to lock frame cache\n");

        goto ERR;
    }

    clear_frame_cache(q->cache);

    pthread_mutex_unlock(&(q->locker));

ERR:
    return;
}

static int create_cache_node_list(frame_queue_t *q,unsigned int buffer_size,int cache_count)
{
    frame_node_t *n = 0;
    int i = 0;
    unsigned int page_size;

    page_size   = getpagesize();

    buffer_size = (buffer_size + page_size - 1) & ~(page_size - 1);

    for (; i < cache_count; ++i)
    {
        n = (frame_node_t*)malloc(sizeof(frame_node_t));

        if (!n)
        {
            LOG("failed to malloc frame_node_t\n");
            goto ERR;
        }

        n->mem = (unsigned char*) memalign (page_size, buffer_size);

        if (!n->mem)
        {
            LOG("failed to malloc frame_node_t->mem field\n");
            goto ERR;
        }

        n->length = buffer_size;
        n->used = 0;
        n->next = 0;
        n->index = i;

        put_node_to_queue(q, n);
    }

    return 0;

ERR:

    if (n)
    {
        free(n);
    }

    return -1;
}

//only one instance for each process.
static uvc_cache_t * g_uvc_cache = 0;
//only one instance for each process.
static uac_cache_t * g_uac_cache = 0;

uvc_cache_t *uvc_cache_get()
{
    return g_uvc_cache;
}

int create_uvc_cache()
{
    unsigned int buffer_size;
    int cache_count;
    g_uvc_cache = (uvc_cache_t*)malloc(sizeof(uvc_cache_t));

    if (g_uvc_cache == 0)
    {
        LOG("malloc uvc_cache failure\n");

        goto ERR;
    }

    if (init_uvc_frame_cache(&g_uvc_cache) < 0)
    {
        LOG("init uvc cache failure\n");

        goto ERR;
    }

    buffer_size = g_imagesize;
    cache_count = g_cache_count;

    create_cache_node_list(g_uvc_cache->free_queue,buffer_size,cache_count);

    return 0;

ERR:
    if (g_uvc_cache != 0)
    {
        free(g_uvc_cache);
    }

    g_uvc_cache = 0;

    return -1;
}

void destroy_uvc_cache()
{
    if (uvc_cache_get() != 0)
    {
        free_uvc_cache(uvc_cache_get());
        free(uvc_cache_get());

        g_uvc_cache = 0;
    }
}

void clear_uvc_cache()
{
    if (uvc_cache_get() != 0)
    {
        uvc_cache_t * uvc_cache = uvc_cache_get();

        clear_queue(uvc_cache->free_queue);
        clear_queue(uvc_cache->ok_queue);
    }
}


uac_cache_t *uac_cache_get()
{
    return g_uac_cache;
}

int create_uac_cache()
{
    unsigned int buffer_size = 640*6;
    int cache_count = 6;

    g_uac_cache = (uac_cache_t*)malloc(sizeof(uac_cache_t));

    if (g_uac_cache == 0)
    {
        printf("malloc uac_cache failure\n");

        goto ERR;
    }

    if (init_uac_frame_cache(&g_uac_cache) < 0)
    {
        printf("init uac cache failure\n");

        goto ERR;
    }

    create_cache_node_list(g_uac_cache->free_queue,buffer_size,cache_count);

    return 0;

ERR:
    if (g_uac_cache != 0)
    {
        free(g_uac_cache);
    }

    g_uac_cache = 0;

    return -1;
}

void destroy_uac_cache()
{
    if (uac_cache_get() != 0)
    {
        free_uac_cache(uac_cache_get());
        free(uac_cache_get());

        g_uac_cache = 0;
    }
}

void clear_uac_cache()
{
    if (uac_cache_get() != 0)
    {
        uac_cache_t * uac_cache = uac_cache_get();

        clear_queue(uac_cache->free_queue);
        clear_queue(uac_cache->ok_queue);
    }
}


int put_node_to_queue(frame_queue_t *q, frame_node_t* node)
{
    if ((q == 0) || (node == 0))
    {
        goto ERR;
    }

    if (0 != pthread_mutex_lock(&(q->locker)))
    {
        LOG("failed to lock frame cache\n");

        goto ERR;
    }

    if (0 != put_frame_into_cache(q->cache, node))
    {
        pthread_mutex_unlock(&(q->locker));

        goto ERR;
    }

    pthread_mutex_unlock(&(q->locker));

    return 0;

ERR:
    return -1;
}

int get_node_from_queue(frame_queue_t *q, frame_node_t** node)
{
    if ((q == 0) || (node == 0))
    {
        goto ERR;
    }

    if (0 != pthread_mutex_lock(&(q->locker)))
    {
        LOG("failed to lock frame cache\n");

        goto ERR;
    }

    if (0 != get_frame_from_cache(q->cache, node))
    {
        pthread_mutex_unlock(&(q->locker));

        goto ERR;
    }

    pthread_mutex_unlock(&(q->locker));

    return 0;
ERR:
    return -1;
}

int wait_queue(frame_queue_t *q)
{
    if (q == 0)
    {
        goto ERR;
    }

ERR:
    return -1;
}

void debug_dump_node(frame_node_t *node)
{
    LOG("0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x\n",
        node->mem[0],
        node->mem[1],
        node->mem[2],
        node->mem[3],
        node->mem[4],
        node->mem[5]);
}
