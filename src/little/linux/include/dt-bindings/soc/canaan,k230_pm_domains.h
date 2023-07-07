/* Copyright (c) 2023, Canaan Bright Sight Co., Ltd
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
*/

#ifndef _DT_BINDINGS_SOC_K230_PM_DOMAINS_H
#define _DT_BINDINGS_SOC_K230_PM_DOMAINS_H

#define K230_PM_DOMAIN_CPU1       0       //CPU1 subsystem
#define K230_PM_DOMAIN_AI         1       //AI Subsystem
#define K230_PM_DOMAIN_DISP       2       //display, 2.5D Engine
#define K230_PM_DOMAIN_VPU        3       //video subsystem
#define K230_PM_DOMAIN_DPU        4       //3D-struct Engine
// #define K230_PM_DOMAIN_DDR      5       //DDRC+DDR-PHY
// #define K230_PM_DOMAIN_NORMAL   6       //others: ISP, STOR(SHRM), HS, LS, SEC
#define K230_PM_DOMAIN_MAX        5

#endif /* _DT_BINDINGS_SOC_K230_PM_DOMAINS_H */
