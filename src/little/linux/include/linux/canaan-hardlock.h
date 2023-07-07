/* Copyright (c) 2023, Canaan Bright Sight Co., Ltd
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
*/
#ifndef __CANAAN_HARDLOCK_H__
#define __CANAAN_HARDLOCK_H__
#define HARDLOCK_MAX    128
int hardlock_lock(int num);
int hardlock_unlock(int num);
int request_lock(int num);
#endif
