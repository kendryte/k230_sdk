/*
 * The confidential and proprietary information contained in this file may
 * only be used by a person authorised under and to the extent permitted
 * by a subsisting licensing agreement from Arm Technology (China) Co., Ltd.
 *
 *            (C) COPYRIGHT 2021-2021 Arm Technology (China) Co., Ltd.
 *                ALL RIGHTS RESERVED
 *
 * This entire notice must be reproduced on all copies of this file
 * and copies of this file may only be made by a person if such person is
 * permitted to do so under the terms of a subsisting license agreement
 * from Arm Technology (China) Co., Ltd.
 *
 * SPDX-License-Identifier: GPL-2.0-only
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 *
 */

#ifndef __MVX_LIST_H__
#define __MVX_LIST_H__

/****************************************************************************
 * Includes
 ****************************************************************************/

#include <stdbool.h>
#include <stddef.h>

/****************************************************************************
 * Defines
 ****************************************************************************/

#define mvx_list_entry(head, type, member) \
    (type *)((char *)head - offsetof(type, member))

#define mvx_list_first_entry(list, type, member) \
    mvx_list_entry((list)->root.next, type, member)

#define mvx_list_next_entry(ptr, member) \
    mvx_list_entry((ptr)->member.next, typeof(*ptr), member)

#define mvx_list_end_entry(list, type, member) \
    mvx_list_entry(&(list)->root, type, member)

#define mvx_list_for_each_entry(list, ptr, member)             \
    for (ptr = mvx_list_first_entry(list, typeof(*ptr), member); \
         ptr != mvx_list_end_entry(list, typeof(*ptr), member);  \
         ptr = mvx_list_next_entry(ptr, member))

#define mvx_list_for_each_entry_safe(list, ptr, member, tmp)         \
    for (ptr = mvx_list_first_entry(list, typeof(*ptr), member), \
         tmp = (ptr)->member.next;                     \
         ptr != mvx_list_end_entry(list, typeof(*ptr), member);  \
         ptr = mvx_list_entry(tmp, typeof(*ptr), member),         \
         tmp = tmp->next)

/****************************************************************************
 * Types
 ****************************************************************************/

struct mvx_list_head {
    struct mvx_list_head *next;
    struct mvx_list_head *prev;
};

struct mvx_list {
    struct mvx_list_head root;
};

/****************************************************************************
 * Static functions
 ****************************************************************************/

static inline void mvx_list_construct(struct mvx_list *list)
{
    list->root.next = &list->root;
    list->root.prev = &list->root;
}

/**
 * Insert 'add' after 'curr'.
 */
static inline void mvx_list_add(struct mvx_list_head *curr,
                struct mvx_list_head *add)
{
    add->next = curr->next;
    add->prev = curr;
    curr->next->prev = add;
    curr->next = add;
}

static inline void mvx_list_add_tail(struct mvx_list *list,
                     struct mvx_list_head *add)
{
    mvx_list_add(list->root.prev, add);
}

static inline void mvx_list_add_head(struct mvx_list *list,
                     struct mvx_list_head *add)
{
    mvx_list_add(&list->root, add);
}

static inline void mvx_list_del(struct mvx_list_head *head)
{
    head->next->prev = head->prev;
    head->prev->next = head->next;
}

static inline bool mvx_list_empty(struct mvx_list *list)
{
    return list->root.next == &list->root;
}

static inline size_t mvx_list_size(struct mvx_list *list)
{
    size_t size = 0;
    struct mvx_list_head *head;

    for (head = list->root.next; head != &list->root; head = head->next)
        size++;

    return size;
}

#endif /* __MVX_LIST_H__ */
