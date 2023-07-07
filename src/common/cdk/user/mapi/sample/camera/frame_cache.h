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
 * The header file about frame cache
 */

#ifndef __FRAME_CACHE_H__
#define __FRAME_CACHE_H__

#include <pthread.h>

typedef struct frame_node_t
{
    unsigned char *mem;
    unsigned int   length;
    unsigned int   used;
    unsigned int   index;
    struct frame_node_t *next;
} frame_node_t;

typedef struct frame_cache_t
{
    struct frame_node_t *head;
    struct frame_node_t *tail;
    unsigned int count;
} frame_cache_t;

typedef struct frame_queue_t
{
    struct frame_cache_t *cache;
    pthread_mutex_t locker;
    pthread_cond_t  waiter;
} frame_queue_t;

typedef struct uvc_cache_t
{
    frame_queue_t *ok_queue;
    frame_queue_t *free_queue;

    unsigned char debug_print;
} uvc_cache_t;

typedef struct uac_cache_t
{
    frame_queue_t *ok_queue;
    frame_queue_t *free_queue;
} uac_cache_t;


int           create_uvc_cache();
void          destroy_uvc_cache();
uvc_cache_t * uvc_cache_get();

int           create_uac_cache();
void          destroy_uac_cache();
uac_cache_t * uac_cache_get();

int           put_node_to_queue(frame_queue_t *q, frame_node_t* node);
int           get_node_from_queue(frame_queue_t *q, frame_node_t** node);
void          clear_uvc_cache();
void          debug_dump_node(frame_node_t *node);

#endif //__FRAME_CACHE_H__
