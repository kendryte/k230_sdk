/* Copyright (c) 2023, Canaan Bright Sight Co., Ltd
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
*/

#ifndef __CANAAN_VO_REGS_H__
#define __CANAAN_VO_REGS_H__

#define VO_SOFT_RST_CTL                             0x00
#define VO_REG_LOAD_CTL                             0x04
#define VO_DMA_SW_CTL                               0x08
#define VO_DMA_RD_CTL_OUT                           0x0c
#define VO_DMA_ARB_MODE                             0x10
#define VO_DMA_WEIGHT_RD0                           0x24
#define VO_DMA_WEIGHT_RD1                           0x28
#define VO_DMA_WEIGHT_RD2                           0x2c
#define VO_DMA_WEIGHT_RD3                           0x30
#define VO_DMA_PRIORITY_RD_0                        0x3c
#define VO_DMA_PRIORITY_RD_1                        0x40
#define VO_DMA_ID_RD_0                              0x4c
#define VO_DMA_ID_RD_1                              0x50

#define VO_LAYER0_LINE0_BD_CTL                      0x80
#define VO_LAYER0_LINE1_BD_CTL                      0x84
#define VO_LAYER0_LINE2_BD_CTL                      0x88
#define VO_LAYER0_LINE3_BD_CTL                      0x8C
#define VO_LAYER1_BD_CTL                            0x90
#define VO_LAYER2_BD_CTL                            0x94
#define VO_LAYER3_BD_CTL                            0x98
#define VO_OSD0_BD_CTL                              0x9c
#define VO_OSD1_BD_CTL                              0xa0
#define VO_OSD2_BD_CTL                              0xa4
#define VO_OSD3_BD_CTL                              0x800
#define VO_OSD4_BD_CTL                              0x804
#define VO_OSD5_BD_CTL                              0x808
#define VO_OSD6_BD_CTL                              0x80c
#define VO_OSD7_BD_CTL                              0x810

#define VO_DISP_IRQ0_CTL                            0x3e0
#define VO_DISP_IRQ1_CTL                            0x3e4
#define VO_DISP_IRQ2_CTL                            0x3e8
#define VO_DISP_IRQ_STATUS                          0x3ec

#define VO_DISP_CTL                                 0x114
#define VO_DISP_ENABLE                              0x118

#define VO_DISP_LAYER0_XCTL                         0x0C8
#define VO_DISP_LAYER0_YCTL                         0x0CC
#define VO_DISP_LAYER1_XCTL                         0x0D0
#define VO_DISP_LAYER1_YCTL                         0x0D4
#define VO_DISP_LAYER2_XCTL                         0x0D8
#define VO_DISP_LAYER2_YCTL                         0x0DC
#define VO_DISP_LAYER3_XCTL                         0x0E0
#define VO_DISP_LAYER3_YCTL                         0x0E4
#define VO_DISP_OSD0_XCTL                           0x0E8
#define VO_DISP_OSD0_YCTL                           0x0EC
#define VO_DISP_OSD1_XCTL                           0x0F0
#define VO_DISP_OSD1_YCTL                           0x0F4
#define VO_DISP_OSD2_XCTL                           0x0F8
#define VO_DISP_OSD2_YCTL                           0x0FC
#define VO_DISP_OSD3_XCTL                           0x820
#define VO_DISP_OSD3_YCTL                           0x824
#define VO_DISP_OSD4_XCTL                           0x828
#define VO_DISP_OSD4_YCTL                           0x82C
#define VO_DISP_OSD5_XCTL                           0x830
#define VO_DISP_OSD5_YCTL                           0x834
#define VO_DISP_OSD6_XCTL                           0x838
#define VO_DISP_OSD6_YCTL                           0x83C
#define VO_DISP_OSD7_XCTL                           0x840
#define VO_DISP_OSD7_YCTL                           0x844

#define VO_LAYER3_OFFSET                            0x240
#define VO_LAYER3_CTL                               0x240
#define VO_LAYER3_Y_ADDR0                           0x244
#define VO_LAYER3_UV_ADDR0                          0x248
#define VO_LAYER3_Y_ADDR1                           0x24C
#define VO_LAYER3_UV_ADDR1                          0x250
#define VO_LAYER3_IMG_IN_OFFSET                     0x254
#define VO_LAYER3_BLENTH                            0x258
#define VO_LAYER3_STRIDE                            0x25C
#define VO_LAYER3_ACT_SIZE                          0x260
#define VO_LAYER3_ADDR_SEL_MODE                     0x264

#define VO_LAYER2_3_CTL_REG_OFFSET                  0x00
#define VO_LAYER2_3_Y_ADDR0_REG_OFFSET              0x04
#define VO_LAYER2_3_UV_ADDR0_REG_OFFSET             0x08
#define VO_LAYER2_3_Y_ADDR1_REG_OFFSET              0x0C
#define VO_LAYER2_3_UV_ADDR1_REG_OFFSET             0x10
#define VO_LAYER2_3_IMG_IN_OFFSET_REG_OFFSET        0x14
#define VO_LAYER2_3_BLENTH_REG_OFFSET               0x18
#define VO_LAYER2_3_STRIDE_REG_OFFSET               0x1C
#define VO_LAYER2_3_ACT_SIZE_REG_OFFSET             0x20
#define VO_LAYER2_3_ADDR_SEL_MODE_REG_OFFSET        0x24

#define VO_OSD0_OFFSET                              0x280
#define VO_OSD0_INFO                                0x280
#define VO_OSD0_SIZE                                0x284
#define VO_OSD0_VLU_ADDR0                           0x288
#define VO_OSD0_ALP_ADDR0                           0x28C
#define VO_OSD0_VLU_ADDR1                           0x290
#define VO_OSD0_ALP_ADDR1                           0x294
#define VO_OSD0_DMA_CTRL                            0x298
#define VO_OSD0_STRIDE                              0x29C
#define VO_OSD0_ADDR_SEL_MODE                       0x2A0

#define VO_OSD1_OFFSET                              0x2C0
#define VO_OSD1_INFO                                0x2C0
#define VO_OSD1_SIZE                                0x2C4
#define VO_OSD1_VLU_ADDR0                           0x2C8
#define VO_OSD1_ALP_ADDR0                           0x2CC
#define VO_OSD1_VLU_ADDR1                           0x2D0
#define VO_OSD1_ALP_ADDR1                           0x2D4
#define VO_OSD1_DMA_CTRL                            0x2D8
#define VO_OSD1_STRIDE                              0x2DC
#define VO_OSD1_ADDR_SEL_MODE                       0x2E0

#define VO_OSD2_OFFSET                              0x300
#define VO_OSD2_INFO                                0x300
#define VO_OSD2_SIZE                                0x304
#define VO_OSD2_VLU_ADDR0                           0x308
#define VO_OSD2_ALP_ADDR0                           0x30C
#define VO_OSD2_VLU_ADDR1                           0x310
#define VO_OSD2_ALP_ADDR1                           0x314
#define VO_OSD2_DMA_CTRL                            0x318
#define VO_OSD2_STRIDE                              0x31C
#define VO_OSD2_ADDR_SEL_MODE                       0x320

#define VO_OSD3_OFFSET                              0x850
#define VO_OSD3_INFO                                0x850
#define VO_OSD3_SIZE                                0x854
#define VO_OSD3_VLU_ADDR0                           0x858
#define VO_OSD3_ALP_ADDR0                           0x85c
#define VO_OSD3_VLU_ADDR1                           0x860
#define VO_OSD3_ALP_ADDR1                           0x864
#define VO_OSD3_DMA_CTRL                            0x868
#define VO_OSD3_STRIDE                              0x86c
#define VO_OSD3_ADDR_SEL_MODE                       0x870

#define VO_OSD4_OFFSET                              0x880
#define VO_OSD4_INFO                                0x880
#define VO_OSD4_SIZE                                0x884
#define VO_OSD4_VLU_ADDR0                           0x888
#define VO_OSD4_ALP_ADDR0                           0x88c
#define VO_OSD4_VLU_ADDR1                           0x890
#define VO_OSD4_ALP_ADDR1                           0x894
#define VO_OSD4_DMA_CTRL                            0x898
#define VO_OSD4_STRIDE                              0x89c
#define VO_OSD4_ADDR_SEL_MODE                       0x8a0

#define VO_OSD5_OFFSET                              0x8b0
#define VO_OSD5_INFO                                0x8b0
#define VO_OSD5_SIZE                                0x8b4
#define VO_OSD5_VLU_ADDR0                           0x8b8
#define VO_OSD5_ALP_ADDR0                           0x8bc
#define VO_OSD5_VLU_ADDR1                           0x8c0
#define VO_OSD5_ALP_ADDR1                           0x8c4
#define VO_OSD5_DMA_CTRL                            0x8c8
#define VO_OSD5_STRIDE                              0x8cc
#define VO_OSD5_ADDR_SEL_MODE                       0x8d0

#define VO_OSD6_OFFSET                              0x8e0
#define VO_OSD6_INFO                                0x8e0
#define VO_OSD6_SIZE                                0x8e4
#define VO_OSD6_VLU_ADDR0                           0x8e8
#define VO_OSD6_ALP_ADDR0                           0x8ec
#define VO_OSD6_VLU_ADDR1                           0x8f0
#define VO_OSD6_ALP_ADDR1                           0x8f4
#define VO_OSD6_DMA_CTRL                            0x8f8
#define VO_OSD6_STRIDE                              0x8fc
#define VO_OSD6_ADDR_SEL_MODE                       0x900

#define VO_OSD7_OFFSET                              0x910
#define VO_OSD7_INFO                                0x910
#define VO_OSD7_SIZE                                0x914
#define VO_OSD7_VLU_ADDR0                           0x918
#define VO_OSD7_ALP_ADDR0                           0x91c
#define VO_OSD7_VLU_ADDR1                           0x920
#define VO_OSD7_ALP_ADDR1                           0x924
#define VO_OSD7_DMA_CTRL                            0x928
#define VO_OSD7_STRIDE                              0x92c
#define VO_OSD7_ADDR_SEL_MODE                       0x930

#define VO_OSD0_7_INFO_REG_OFFSET                   0x00
#define VO_OSD0_7_SIZE_REG_OFFSET                   0x04
#define VO_OSD0_7_VLU_ADDR0_REG_OFFSET              0x08
#define VO_OSD0_7_ALP_ADDR0_REG_OFFSET              0x0C
#define VO_OSD0_7_VLU_ADDR1_REG_OFFSET              0x10
#define VO_OSD0_7_ALP_ADDR1_REG_OFFSET              0x14
#define VO_OSD0_7_DMA_CTRL_REG_OFFSET               0x18
#define VO_OSD0_7_STRIDE_REG_OFFSET                 0x1C
#define VO_OSD0_7_ADDR_SEL_MODE_REG_OFFSET          0x20

#endif /* __CANAAN_VO_REGS_H__ */
