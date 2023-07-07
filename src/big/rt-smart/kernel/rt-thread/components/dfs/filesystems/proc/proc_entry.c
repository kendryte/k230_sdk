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

#include <rtthread.h>
#include <rthw.h>

#ifdef RT_USING_MODULE
#include <dlmodule.h>
#endif

#ifdef RT_USING_LWP
#include <lwp.h>
#endif


#ifdef RT_USING_PROC

rt_proc_entry_t rt_proc_entry_find(const char *name)
{
    struct rt_object *object;
    struct rt_list_node *node;
    struct rt_object_information *information;

    /* enter critical */
    if (rt_thread_self() != RT_NULL)
        rt_enter_critical();

    information = rt_object_get_information(RT_Object_Class_Proc);
    RT_ASSERT(information != RT_NULL);
    for (node  = information->object_list.next;
         node != &(information->object_list);
         node  = node->next)
    {
        object = rt_list_entry(node, struct rt_object, list);
        if (rt_strncmp(object->name, name, RT_NAME_MAX) == 0)
        {
            /* leave critical */
            if (rt_thread_self() != RT_NULL)
                rt_exit_critical();

            return (rt_proc_entry_t)object;
        }
    }

    /* leave critical */
    if (rt_thread_self() != RT_NULL)
        rt_exit_critical();

    /* not found */
    return RT_NULL;
}

rt_err_t rt_proc_entry_register(rt_proc_entry_t entry, const char *name)
{
    if (entry == RT_NULL)
        return -RT_ERROR;

    if (rt_proc_entry_find(name) != RT_NULL)
        return -RT_ERROR;

    rt_object_init(&(entry->parent), RT_Object_Class_Proc, name);

#if defined(RT_USING_POSIX)
    entry->fops = NULL;
#endif

    return RT_EOK;
}

rt_err_t rt_proc_entry_unregister(rt_proc_entry_t entry)
{
    RT_ASSERT(entry != RT_NULL);
    RT_ASSERT(rt_object_get_type(&entry->parent) == RT_Object_Class_Proc);
    RT_ASSERT(rt_object_is_systemobject(&entry->parent));

    rt_object_detach(&(entry->parent));

    return RT_EOK;
}

 rt_proc_entry_t rt_proc_entry_create(int type, int attach_size)
{
    int size;
    rt_proc_entry_t entry;

    size = RT_ALIGN(sizeof(struct rt_proc_entry), RT_ALIGN_SIZE);
    attach_size = RT_ALIGN(attach_size, RT_ALIGN_SIZE);
    /* use the total size */
    size += attach_size;

    entry = (rt_proc_entry_t)rt_malloc(size);
    if (entry)
    {
        rt_memset(entry, 0x0, sizeof(struct rt_proc_entry));
        entry->type = (enum rt_proc_class_type)type;
    }

    return entry;
}

void rt_proc_entry_destory(rt_proc_entry_t entry)
{
    RT_ASSERT(entry != RT_NULL);
    RT_ASSERT(rt_object_get_type(&entry->parent) == RT_Object_Class_Proc);
    RT_ASSERT(rt_object_is_systemobject(&entry->parent) == RT_FALSE);

    rt_object_detach(&(entry->parent));

    rt_free(entry);
}
#endif
