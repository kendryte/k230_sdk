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

#include "mvx_argparse.h"
#include "mvx_player.hpp"
#include <fstream>

using namespace std;

int main(int argc, const char *argv[])
{
    int ret;
    mvx_argparse argp;

    mvx_argp_construct(&argp);
    mvx_argp_add_opt(&argp, '\0', "dev", true, 1, "/dev/video0", "Device.");
    mvx_argp_add_opt(&argp, 'f', "formats", true, 0, "0", "List supported formats.");

    ret = mvx_argp_parse(&argp, argc - 1, &argv[1]);
    if (ret != 0)
    {
        mvx_argp_help(&argp, argv[0]);
        return 1;
    }

    Info info(mvx_argp_get(&argp, "dev", 0));
    if (mvx_argp_is_set(&argp, "formats"))
    {
        cout << "Supported formats:" << endl;
        info.enumerateFormats();
    }

    return 0;
}
