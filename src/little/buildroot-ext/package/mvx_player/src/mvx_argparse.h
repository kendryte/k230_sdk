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

#ifndef __mvx_argp_H__
#define __mvx_argp_H__

#ifdef __cplusplus
extern "C" {
#endif

/****************************************************************************
 * Includes
 ****************************************************************************/

#include "mvx_list.h"

/****************************************************************************
 * Defines
 ****************************************************************************/

#define mvx_argp_ZERO_OR_ONE    -1
#define mvx_argp_ZERO_OR_MORE   -2
#define mvx_argp_ONE_OR_MORE    -3

/****************************************************************************
 * Types
 ****************************************************************************/

enum mvx_argp_action {
    MVX_ARGP_STORE,
    MVX_ARGP_APPEND
};

struct mvx_argparse {
    struct mvx_list optional;
    struct mvx_list positional;
};

/****************************************************************************
 * Exported functions
 ****************************************************************************/

/**
 * mvx_argp_construct() - Construct argparse.
 */
void mvx_argp_construct(struct mvx_argparse *argp);

/**
 * mvx_argp_destruct() - Destruct argparse.
 */
void mvx_argp_destruct(struct mvx_argparse *argp);

/**
 * mvx_argp_add_opt() - Add optional argument.
 */
int mvx_argp_add_opt(struct mvx_argparse *argp,
             const char arg_short,
             const char *arg_long,
             bool optional,
             int nargs,
             const char *def,
             const char *help);

/**
 * mvx_argp_add_pos() - Add positional argument.
 */
int mvx_argp_add_pos(struct mvx_argparse *argp,
             const char *arg_long,
             bool optional,
             int nargs,
             const char *def,
             const char *help);

/**
 * mvx_argp_parse() - Parse command line arguments.
 */
int mvx_argp_parse(struct mvx_argparse *argp,
           int argc,
           const char **argv);

/**
 * mvx_argp_parse() - Parse command line arguments.
 */
const char *mvx_argp_get(struct mvx_argparse *argp,
             const char *arg_long,
             unsigned int index);

/**
 * mvx_argp_get_int() - Parse command line arguments.
 */
int mvx_argp_get_int(struct mvx_argparse *argp,
             const char *arg_long,
             unsigned int index);

/**
 * mvx_argp_is_set() - Return if argument has been set.
 */
bool mvx_argp_is_set(struct mvx_argparse *argp,
             const char *arg_long);

/**
 * mvx_argp_help() - Print help message.
 */
void mvx_argp_help(struct mvx_argparse *argp,
           const char *exe);

#ifdef __cplusplus
}
#endif

#endif /* __mvx_argp_H__ */
