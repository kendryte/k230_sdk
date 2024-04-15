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

/****************************************************************************
 * Include
 ****************************************************************************/

#include <ctype.h>
#include <errno.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "mvx_argparse.h"

/****************************************************************************
 * Defines
 ****************************************************************************/

#define min(x, y)               \
    ({                   \
         typeof(x)_min1 = (x);       \
         typeof(y)_min2 = (y);       \
         (void)(&_min1 == &_min2); \
         _min1 < _min2 ? _min1 : _min2; })

/****************************************************************************
 * Types
 ****************************************************************************/

struct mvx_value {
    struct mvx_list_head head;
    char *value;
};

struct mvx_argument {
    struct mvx_list_head head;
    char arg_short;
    const char *arg_long;
    bool optional;
    int nargs;
    enum mvx_argp_action action;
    const char *def;
    const char *help;
    bool is_set;
    struct mvx_list values;
    int min_args;
    int max_args;
};

/****************************************************************************
 * Argument
 ****************************************************************************/

/**
 * Return: 1 on success, 0 if arg can't accept more value, else error code.
 */
static int arg_push(struct mvx_argument *arg,
            const char *s)
{
    size_t size;
    struct mvx_value *value;

    size = mvx_list_size(&arg->values);
    if ((int)size >= arg->max_args)
        return 0;

    value = (mvx_value *)malloc(sizeof(*value));
    if (value == NULL)
        return -EINVAL;

    mvx_list_add_tail(&arg->values, &value->head);
    value->value = strdup(s);

    return 1;
}

static void arg_clear(struct mvx_argument *arg)
{
    struct mvx_value *value;
    struct mvx_list_head *tmp;

    mvx_list_for_each_entry_safe(&arg->values, value, head, tmp) {
        mvx_list_del(&value->head);
        free(value->value);
        free(value);
    }
}

static const char *arg_value(struct mvx_argument *arg,
                 unsigned int index)
{
    struct mvx_value *value;

    mvx_list_for_each_entry(&arg->values, value, head) {
        if (index-- == 0)
            return value->value;
    }

    if (arg->def != NULL)
        return arg->def;

    return NULL;
}

static void arg_open(struct mvx_argument *arg)
{
    if (arg->action == MVX_ARGP_STORE)
        arg_clear(arg);

    arg->is_set = true;
}

static int arg_close(struct mvx_argument *arg)
{
    size_t size;

    if (arg == NULL)
        return 0;

    /* Verify that sufficient arguments have been given. */
    size = mvx_list_size(&arg->values);
    if ((int)size < arg->min_args)
        return -EINVAL;

    return 0;
}

static void arg_construct(struct mvx_argument *arg,
              const char arg_short,
              const char *arg_long,
              bool optional,
              int nargs,
              const char *def,
              const char *help)
{
    arg->arg_short = arg_short;
    arg->arg_long = arg_long;
    arg->optional = optional;
    arg->nargs = nargs;
    arg->action = MVX_ARGP_STORE;
    arg->def = def;
    arg->help = help;
    arg->is_set = false;

    mvx_list_construct(&arg->values);

    switch (arg->nargs) {
    case mvx_argp_ZERO_OR_ONE:
        arg->min_args = 0;
        arg->max_args = 1;
        break;
    case mvx_argp_ZERO_OR_MORE:
        arg->min_args = 0;
        arg->max_args = 1000000;
        break;
    case mvx_argp_ONE_OR_MORE:
        arg->min_args = 1;
        arg->max_args = 1000000;
        break;
    default:
        arg->min_args = arg->nargs;
        arg->max_args = arg->nargs;
    }
}

static struct mvx_argument *arg_new(const char c,
                    const char *s,
                    bool optional,
                    int nargs,
                    const char *def,
                    const char *help)
{
    struct mvx_argument *arg;

    arg = (mvx_argument *)malloc(sizeof(*arg));
    if (arg == NULL)
        return NULL;

    arg_construct(arg, c, s, optional, nargs, def, help);

    return arg;
}

static void arg_destruct(struct mvx_argument *arg)
{
    arg_clear(arg);
}

static void arg_delete(struct mvx_argument *arg)
{
    arg_destruct(arg);
    free(arg);
}

/****************************************************************************
 * Static functions
 ****************************************************************************/

static struct mvx_argument *find_arg_long(struct mvx_list *list,
                      const char *arg_long)
{
    struct mvx_argument *arg;

    mvx_list_for_each_entry(list, arg, head) {
        if (strcmp(arg->arg_long, arg_long) == 0)
            return arg;
    }

    return NULL;
}

static struct mvx_argument *find_arg_short(struct mvx_list *list,
                       const char arg_short)
{
    struct mvx_argument *arg;

    mvx_list_for_each_entry(list, arg, head) {
        if (arg->arg_short == arg_short)
            return arg;
    }

    return NULL;
}

static struct mvx_argument *find_arg(struct mvx_argparse *argp,
                     const char c,
                     const char *s)
{
    struct mvx_argument *arg;

    if (isalpha(c) && find_arg_short(&argp->optional, c) != NULL) {
        arg = find_arg_short(&argp->optional, c);
        if (arg != NULL)
            return arg;
    }

    if (s != NULL) {
        arg = find_arg_long(&argp->optional, s);
        if (arg != NULL)
            return arg;

        arg = find_arg_long(&argp->positional, s);
        if (arg != NULL)
            return arg;
    }

    return NULL;
}

static int parse_positional(struct mvx_argparse *argp,
                struct mvx_argument *arg,
                int argc,
                const char **argv)
{
    struct mvx_argument *end;
    int i;

    /* Have all positional arguments been handled? */
    end = mvx_list_end_entry(&argp->positional, typeof(*end), head);
    if (arg == end) {
        if (argc == 0)
            return 0;
        else
            return -EINVAL;
    }

    /*
     * Use greedy algorithm. Current argument consume as many command line
     * arguments as possible.
     */
    for (i = min(arg->max_args, argc); i >= arg->min_args; i--) {
        struct mvx_argument *next = mvx_list_next_entry(arg, head);
        int ret;

        ret = parse_positional(argp, next, argc - i, &argv[i]);
        if (ret == 0) {
            int j;

            for (j = 0; j < i; j++)
                arg_push(arg, argv[j]);

            return 0;
        }
    }

    if (arg->optional != false) {
        struct mvx_argument *next = mvx_list_next_entry(arg, head);
        return parse_positional(argp, next, argc, argv);
    }

    /* No valid combination was found. */
    return -EINVAL;
}

/****************************************************************************
 * Exported functions
 ****************************************************************************/

void mvx_argp_construct(struct mvx_argparse *argp)
{
    mvx_list_construct(&argp->optional);
    mvx_list_construct(&argp->positional);
}

void mvx_argp_destruct(struct mvx_argparse *argp)
{
    struct mvx_argument *arg;
    struct mvx_list_head *tmp;

    mvx_list_for_each_entry_safe(&argp->optional, arg, head, tmp) {
        mvx_list_del(&arg->head);
        arg_delete(arg);
    }

    mvx_list_for_each_entry_safe(&argp->positional, arg, head, tmp) {
        mvx_list_del(&arg->head);
        arg_delete(arg);
    }
}

int mvx_argp_add_opt(struct mvx_argparse *argp,
             const char c,
             const char *s,
             bool optional,
             int nargs,
             const char *def,
             const char *help)
{
    struct mvx_argument *arg;

    if (find_arg(argp, c, s) != NULL) {
        fprintf(stderr,
            "Error: Argument already exists. short='%c', long='%s'.\n",
            c, s);
        return -EINVAL;
    }

    arg = arg_new(c, s, optional, nargs, def, help);
    if (arg == NULL)
        return -EINVAL;

    mvx_list_add_tail(&argp->optional, &arg->head);

    return 0;
}

int mvx_argp_add_pos(struct mvx_argparse *argp,
             const char *s,
             bool optional,
             int nargs,
             const char *def,
             const char *help)
{
    struct mvx_argument *arg;

    if (find_arg(argp, 0, s) != NULL) {
        fprintf(stderr, "Error: Argument already exists. long='%s'.\n",
            s);
        return -EINVAL;
    }

    arg = arg_new(0, s, optional, nargs, def, help);
    if (arg == NULL)
        return -EINVAL;

    mvx_list_add_tail(&argp->positional, &arg->head);

    return 0;
}

int mvx_argp_parse(struct mvx_argparse *argp,
           int argc,
           const char **argv)
{
    struct mvx_argument *arg = NULL;
    int i;
    int ret;

    for (i = 0; i < argc; i++) {
        if (strcmp(argv[i], "--") == 0) {
            i++;
            break;
        } else if (strncmp(argv[i], "--", 2) == 0) {
            size_t len = strlen(&argv[i][2]);
            char __argv[len + 1];
            char *value;

            ret = arg_close(arg);
            if (ret != 0)
                return ret;

            memcpy(__argv, &argv[i][2], len + 1);
            value = strchr(__argv, '=');
            if (value != NULL)
                *value++ = '\0';

            arg = find_arg_long(&argp->optional, __argv);
            if (arg == NULL) {
                fprintf(stderr,
                    "Error: Could not find optional argument '--%s'.\n",
                    &argv[i][2]);
                return -EINVAL;
            }

            arg_open(arg);
            if (value != NULL) {
                ret = arg_push(arg, value);
                if (ret <= 0)
                    return -EINVAL;
            }
        } else if (argv[i][0] == '-' && strlen(argv[i]) > 1) {
            const char *c;

            for (c = &argv[i][1]; *c != '\0'; c++) {
                ret = arg_close(arg);
                if (ret != 0)
                    return ret;

                arg = find_arg_short(&argp->optional, *c);
                if (arg == NULL) {
                    fprintf(stderr,
                        "Error: Could not find optional argument '-%c'.\n",
                        *c);
                    return -EINVAL;
                }

                arg_open(arg);

                /*
                 * If this optional argument takes additional
                 * command line arguments, then the rest of the
                 * argument is treated as a value.
                 */
                if (strlen(&c[1]) > 0) {
                    ret = arg_push(arg, &c[1]);
                    if (ret > 0)
                        break;
                }
            }
        } else if (arg != NULL) {
            ret = arg_push(arg, argv[i]);
            if (ret < 0)
                return ret;
            else if (ret == 0)
                break;
        } else {
            break;
        }
    }

    ret = arg_close(arg);
    if (ret != 0)
        return ret;

    ret = parse_positional(argp,
                   mvx_list_first_entry(&argp->positional,
                            struct mvx_argument,
                            head),
                   argc - i, &argv[i]);
    if (ret != 0)
        return ret;

    mvx_list_for_each_entry(&argp->optional, arg, head) {
        if (arg->optional == false && arg->is_set == false) {
            fprintf(stderr,
                "Error: Mandatory optional argument not set. short='-%c', long='--%s'.\n",
                arg->arg_short, arg->arg_long);
            return -EINVAL;
        }
    }

    return ret;
}

const char *mvx_argp_get(struct mvx_argparse *argp,
             const char *arg_long,
             unsigned int index)
{
    struct mvx_argument *arg;

    arg = find_arg(argp, 0, arg_long);
    if (arg == NULL)
        return NULL;

    return arg_value(arg, index);
}

int mvx_argp_get_int(struct mvx_argparse *argp,
             const char *arg_long,
             unsigned int index)
{
    const char *value;
    int base = 10;

    value = mvx_argp_get(argp, arg_long, index);
    if (value == NULL)
        return 0;

    if (strncmp(value, "0x", 2) == 0 || strncmp(value, "0X", 2) == 0)
        base = 16;
    else if (value[0] == '0')
        base = 8;

    return strtol(value, NULL, base);
}

bool mvx_argp_is_set(struct mvx_argparse *argp,
             const char *arg_long)
{
    struct mvx_argument *arg;

    arg = find_arg(argp, 0, arg_long);
    if (arg == NULL)
        return false;

    return arg->is_set;
}

void mvx_argp_help(struct mvx_argparse *argp,
           const char *exe)
{
    struct mvx_argument *arg;

    printf("usage: %s [optional] [positional]\n\n", exe);

    printf("positional arguments:\n");
    mvx_list_for_each_entry(&argp->positional, arg, head) {
        printf("\t%s\n", arg->arg_long);

        if (arg->help)
            printf("\t\t%s\n", arg->help);
    }

    printf("\noptional arguments:\n");
    mvx_list_for_each_entry(&argp->optional, arg, head) {
        char *delim = (char *)"";

        printf("\t");

        if (arg->arg_short != 0) {
            printf("-%c", arg->arg_short);
            delim = (char *)", ";
        }

        if (arg->arg_long != NULL)
            printf("%s--%s", delim, arg->arg_long);

        printf("\n");

        if (arg->help != NULL)
            printf("\t\t%s\n", arg->help);

        if (arg->def != NULL)
            printf("\t\tDefault: %s\n", arg->def);
    }
}
