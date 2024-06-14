#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>


#include "k_sensor_comm.h"
#include "k_vicap_comm.h"
#include "k_sensor_ioctl.h"


#define pr_debug(...) //printf(__VA_ARGS__)
#define pr_info(...) //printf(__VA_ARGS__)
#define pr_warn(...) //printf(__VA_ARGS__)
#define pr_err(...)  printf(__VA_ARGS__)

static const k_vicap_sensor_type_map sensor_type_map_list[] = {
    {
        "cam-ov9732-mode0",
        OV_OV9732_MIPI_1280X720_30FPS_10BIT_LINEAR
    },
    {
        "cam-ov97286-mode0",
        OV_OV9286_MIPI_1280X720_30FPS_10BIT_LINEAR_IR
    },
    {
        "cam-ov97286-mode1",
        OV_OV9286_MIPI_1280X720_30FPS_10BIT_LINEAR_SPECKLE
    },
    {
        "cam-ov97286-mode2",
        OV_OV9286_MIPI_1280X720_60FPS_10BIT_LINEAR_IR
    },
    {
        "cam-ov97286-mode3",
        OV_OV9286_MIPI_1280X720_60FPS_10BIT_LINEAR_SPECKLE
    },
    {
        "cam-ov97286-mode4",
        OV_OV9286_MIPI_1280X720_30FPS_10BIT_LINEAR_IR_SPECKLE
    },
    {
        "cam-ov97286-mode5",
        OV_OV9286_MIPI_1280X720_60FPS_10BIT_LINEAR_IR_SPECKLE
    },
    {
        "cam-imx335-mode0",
        IMX335_MIPI_2LANE_RAW12_1920X1080_30FPS_LINEAR
    },
    {
        "cam-imx335-mode1",
        IMX335_MIPI_2LANE_RAW12_2592X1944_30FPS_LINEAR
    },
    {
        "cam-imx335-mode2",
        IMX335_MIPI_4LANE_RAW12_2592X1944_30FPS_LINEAR
    },
    {
        "cam-ov5647-mode0",
        OV_OV5647_MIPI_CSI0_1920X1080_30FPS_10BIT_LINEAR
    },
    {
        "cam-ov5647-mode1",
        OV_OV5647_MIPI_CSI1_1920X1080_30FPS_10BIT_LINEAR
    },
    {
        "cam-ov5647-mode2",
        OV_OV5647_MIPI_CSI2_1920X1080_30FPS_10BIT_LINEAR
    }
};

static const k_vicap_sensor_info sensor_info_list[] = {
    {
        "ov9732",
        1280,
        720,
        VICAP_CSI0,
        VICAP_MIPI_1LANE,
        VICAP_SOURCE_CSI0,
        K_FALSE,
        VICAP_MIPI_PHY_800M,
        VICAP_CSI_DATA_TYPE_RAW10,
        VICAP_LINERA_MODE,
        VICAP_FLASH_DISABLE,
        VICAP_VI_FIRST_FRAME_FS_TR0,
        0,
        OV_OV9732_MIPI_1280X720_30FPS_10BIT_MCLK_16M_LINEAR
    },
    {
        "ov9732",
        1280,
        720,
        VICAP_CSI2,
        VICAP_MIPI_1LANE,
        VICAP_SOURCE_CSI2,
        K_FALSE,
        VICAP_MIPI_PHY_800M,
        VICAP_CSI_DATA_TYPE_RAW10,
        VICAP_LINERA_MODE,
        VICAP_FLASH_DISABLE,
        VICAP_VI_FIRST_FRAME_FS_TR0,
        0,
        OV_OV9732_MIPI_1280X720_30FPS_10BIT_MCLK_16M_LINEAR_V2
    },
    {
        "ov9732",
        1280,
        720,
        VICAP_CSI0,
        VICAP_MIPI_1LANE,
        VICAP_SOURCE_CSI0,
        K_FALSE,
        VICAP_MIPI_PHY_800M,
        VICAP_CSI_DATA_TYPE_RAW10,
        VICAP_LINERA_MODE,
        VICAP_FLASH_DISABLE,
        VICAP_VI_FIRST_FRAME_FS_TR0,
        0,
        OV_OV9732_MIPI_1280X720_30FPS_10BIT_LINEAR
    },
    {
        "ov9286",
        1280,
        720,
        VICAP_CSI1,
        VICAP_MIPI_2LANE,
        VICAP_SOURCE_CSI1_FS_TR1,
        K_TRUE,
        VICAP_MIPI_PHY_800M,
        VICAP_CSI_DATA_TYPE_RAW10,
        VICAP_LINERA_MODE,
        VICAP_FLASH_FOLLOW_STROBE,
        VICAP_VI_FIRST_FRAME_FS_TR0,
        0,
        OV_OV9286_MIPI_1280X720_30FPS_10BIT_MCLK_25M_LINEAR_SPECKLE
    },
    {
        "ov9286",
        1280,
        720,
        VICAP_CSI1,
        VICAP_MIPI_2LANE,
        VICAP_SOURCE_CSI1_FS_TR0,
        K_TRUE,
        VICAP_MIPI_PHY_800M,
        VICAP_CSI_DATA_TYPE_RAW10,
        VICAP_LINERA_MODE,
        VICAP_FLASH_FOLLOW_STROBE,
        VICAP_VI_FIRST_FRAME_FS_TR0,
        0,
        OV_OV9286_MIPI_1280X720_30FPS_10BIT_MCLK_25M_LINEAR_IR
    },
    {
        "ov9286",
        1280,
        720,
        VICAP_CSI1,
        VICAP_MIPI_2LANE,
        VICAP_SOURCE_CSI1_FS_TR1,
        K_TRUE,
        VICAP_MIPI_PHY_800M,
        VICAP_CSI_DATA_TYPE_RAW10,
        VICAP_LINERA_MODE,
        VICAP_FLASH_FOLLOW_STROBE,
        VICAP_VI_FIRST_FRAME_FS_TR0,
        0,
        OV_OV9286_MIPI_1280X720_30FPS_10BIT_MCLK_25M_LINEAR_SPECKLE_V2
    },
    {
        "ov9286",
        1280,
        720,
        VICAP_CSI1,
        VICAP_MIPI_2LANE,
        VICAP_SOURCE_CSI1_FS_TR0, //VICAP_SOURCE_CSI1_FS_TR0,
        K_TRUE,
        VICAP_MIPI_PHY_800M,
        VICAP_CSI_DATA_TYPE_RAW10,
        VICAP_LINERA_MODE,
        VICAP_FLASH_FOLLOW_STROBE,
        VICAP_VI_FIRST_FRAME_FS_TR0,
        0,
        OV_OV9286_MIPI_1280X720_30FPS_10BIT_MCLK_25M_LINEAR_IR_V2
    },
    {
        "ov9286",
        1280,
        720,
        VICAP_CSI1,
        VICAP_MIPI_2LANE,
        VICAP_SOURCE_CSI1_FS_TR0,
        K_TRUE,
        VICAP_MIPI_PHY_800M,
        VICAP_CSI_DATA_TYPE_RAW10,
        VICAP_LINERA_MODE,
        VICAP_FLASH_FOLLOW_STROBE,
        VICAP_VI_FIRST_FRAME_FS_TR0,
        0,
        OV_OV9286_MIPI_1280X720_30FPS_10BIT_LINEAR_IR
    },
    {
        "ov9286",
        1280,
        720,
        VICAP_CSI1,
        VICAP_MIPI_2LANE,
        VICAP_SOURCE_CSI1_FS_TR1,
        K_TRUE,
        VICAP_MIPI_PHY_800M,
        VICAP_CSI_DATA_TYPE_RAW10,
        VICAP_LINERA_MODE,
        VICAP_FLASH_FOLLOW_STROBE,
        VICAP_VI_FIRST_FRAME_FS_TR0,
        0,
        OV_OV9286_MIPI_1280X720_30FPS_10BIT_LINEAR_SPECKLE
    },
    {
        "ov9286",
        1280,
        720,
        VICAP_CSI1,
        VICAP_MIPI_2LANE,
        VICAP_SOURCE_CSI1,
        K_FALSE,
        VICAP_MIPI_PHY_800M,
        VICAP_CSI_DATA_TYPE_RAW10,
        VICAP_LINERA_MODE,
        VICAP_FLASH_DISABLE,
        VICAP_VI_FIRST_FRAME_FS_TR0,
        0,
        OV_OV9286_MIPI_1280X720_30FPS_10BIT_LINEAR_IR_SPECKLE
    },
    {
        "ov9286",
        1280,
        720,
        VICAP_CSI1,
        VICAP_MIPI_2LANE,
        VICAP_SOURCE_CSI1,
        K_FALSE,
        VICAP_MIPI_PHY_800M,
        VICAP_CSI_DATA_TYPE_RAW10,
        VICAP_LINERA_MODE,
        VICAP_FLASH_DISABLE,
        VICAP_VI_FIRST_FRAME_FS_TR0,
        0,
        OV_OV9286_MIPI_1280X720_60FPS_10BIT_LINEAR_IR_SPECKLE
    },
    {
        "imx335",
        1920,
        1080,
        VICAP_CSI0,
        VICAP_MIPI_2LANE,
        VICAP_SOURCE_CSI0,
        K_FALSE,
        VICAP_MIPI_PHY_1200M,
        VICAP_CSI_DATA_TYPE_RAW12,
        VICAP_LINERA_MODE,
        VICAP_FLASH_DISABLE,
        VICAP_VI_FIRST_FRAME_FS_TR0,
        0,
        IMX335_MIPI_2LANE_RAW12_1920X1080_30FPS_LINEAR
    },
    {
        "imx335",
        2592,
        1944,
        VICAP_CSI0,
        VICAP_MIPI_2LANE,
        VICAP_SOURCE_CSI0,
        K_FALSE,
        VICAP_MIPI_PHY_1200M,
        VICAP_CSI_DATA_TYPE_RAW12,
        VICAP_LINERA_MODE,
        VICAP_FLASH_DISABLE,
        VICAP_VI_FIRST_FRAME_FS_TR0,
        0,
        IMX335_MIPI_2LANE_RAW12_2592X1944_30FPS_LINEAR
    },
    {
        "imx335",
        2592,
        1944,
        VICAP_CSI0,
        VICAP_MIPI_4LANE,
        VICAP_SOURCE_CSI0,
        K_FALSE,
        VICAP_MIPI_PHY_1200M,
        VICAP_CSI_DATA_TYPE_RAW12,
        VICAP_LINERA_MODE,
        VICAP_FLASH_DISABLE,
        VICAP_VI_FIRST_FRAME_FS_TR0,
        0,
        IMX335_MIPI_4LANE_RAW12_2592X1944_30FPS_LINEAR,
    },
    {
        "imx335",
        1920,
        1080,
        VICAP_CSI0,
        VICAP_MIPI_2LANE,
        VICAP_SOURCE_CSI0,
        K_FALSE,
        VICAP_MIPI_PHY_1200M,
        VICAP_CSI_DATA_TYPE_RAW12,
        VICAP_LINERA_MODE,
        VICAP_FLASH_DISABLE,
        VICAP_VI_FIRST_FRAME_FS_TR0,
        0,
        IMX335_MIPI_2LANE_RAW12_1920X1080_30FPS_MCLK_7425_LINEAR
    },
    {
        "imx335",
        2592,
        1944,
        VICAP_CSI0,
        VICAP_MIPI_2LANE,
        VICAP_SOURCE_CSI0,
        K_FALSE,
        VICAP_MIPI_PHY_1200M,
        VICAP_CSI_DATA_TYPE_RAW12,
        VICAP_LINERA_MODE,
        VICAP_FLASH_DISABLE,
        VICAP_VI_FIRST_FRAME_FS_TR0,
        0,
        IMX335_MIPI_2LANE_RAW12_2592X1944_30FPS_MCLK_7425_LINEAR
    },
    {
        "imx335",
        2592,
        1944,
        VICAP_CSI0,
        VICAP_MIPI_4LANE,
        VICAP_SOURCE_CSI0,
        K_FALSE,
        VICAP_MIPI_PHY_1200M,
        VICAP_CSI_DATA_TYPE_RAW12,
        VICAP_LINERA_MODE,
        VICAP_FLASH_DISABLE,
        VICAP_VI_FIRST_FRAME_FS_TR0,
        0,
        IMX335_MIPI_4LANE_RAW12_2592X1944_30FPS_MCLK_7425_LINEAR,
    },
    {
        "imx335",
        2592,
        1944,
        VICAP_CSI0,
        VICAP_MIPI_4LANE,
        VICAP_SOURCE_CSI0,
        K_FALSE,
        VICAP_MIPI_PHY_1200M,
        VICAP_CSI_DATA_TYPE_RAW10,
        VICAP_VCID_HDR_2FRAME,
        VICAP_FLASH_DISABLE,
        VICAP_VI_FIRST_FRAME_FS_TR0,
        0,
        IMX335_MIPI_4LANE_RAW10_2XDOL,
    },
    {
        "imx335",
        2592,
        1944,
        VICAP_CSI0,
        VICAP_MIPI_4LANE,
        VICAP_SOURCE_CSI0,
        K_FALSE,
        VICAP_MIPI_PHY_1200M,
        VICAP_CSI_DATA_TYPE_RAW10,
        VICAP_VCID_HDR_3FRAME,
        VICAP_FLASH_DISABLE,
        VICAP_VI_FIRST_FRAME_FS_TR0,
        0,
        IMX335_MIPI_4LANE_RAW10_3XDOL,
    },
    {
        "sc035hgs",
        640,
        480,
        VICAP_CSI2,
        VICAP_MIPI_1LANE,
        VICAP_SOURCE_CSI2,
        K_FALSE,
        VICAP_MIPI_PHY_1200M,
        VICAP_CSI_DATA_TYPE_RAW10,
        VICAP_LINERA_MODE,
        VICAP_FLASH_DISABLE,
        VICAP_VI_FIRST_FRAME_FS_TR0,
        0,
        SC_SC035HGS_MIPI_1LANE_RAW10_640X480_120FPS_LINEAR
    },
    {
        "sc035hgs",
        640,
        480,
        VICAP_CSI2,
        VICAP_MIPI_1LANE,
        VICAP_SOURCE_CSI2,
        K_FALSE,
        VICAP_MIPI_PHY_1200M,
        VICAP_CSI_DATA_TYPE_RAW10,
        VICAP_LINERA_MODE,
        VICAP_FLASH_DISABLE,
        VICAP_VI_FIRST_FRAME_FS_TR0,
        0,
        SC_SC035HGS_MIPI_1LANE_RAW10_640X480_60FPS_LINEAR
    },
    {
        "sc035hgs",
        640,
        480,
        VICAP_CSI2,
        VICAP_MIPI_1LANE,
        VICAP_SOURCE_CSI2,
        K_FALSE,
        VICAP_MIPI_PHY_1200M,
        VICAP_CSI_DATA_TYPE_RAW10,
        VICAP_LINERA_MODE,
        VICAP_FLASH_DISABLE,
        VICAP_VI_FIRST_FRAME_FS_TR0,
        0,
        SC_SC035HGS_MIPI_1LANE_RAW10_640X480_30FPS_LINEAR
    },
    {
        "ov9286",
        1280,
        720,
        VICAP_CSI1,
        VICAP_MIPI_2LANE,
        VICAP_SOURCE_CSI1_FS_TR0,
        K_TRUE,
        VICAP_MIPI_PHY_800M,
        VICAP_CSI_DATA_TYPE_RAW10,
        VICAP_LINERA_MODE,
        VICAP_FLASH_FOLLOW_STROBE,
        VICAP_VI_FIRST_FRAME_FS_TR0,
        0,
        OV_OV9286_MIPI_1280X720_60FPS_10BIT_LINEAR_IR
    },
    {
        "ov9286",
        1280,
        720,
        VICAP_CSI1,
        VICAP_MIPI_2LANE,
        VICAP_SOURCE_CSI1_FS_TR1,
        K_TRUE,
        VICAP_MIPI_PHY_800M,
        VICAP_CSI_DATA_TYPE_RAW10,
        VICAP_LINERA_MODE,
        VICAP_FLASH_FOLLOW_STROBE,
        VICAP_VI_FIRST_FRAME_FS_TR0,
        0,
        OV_OV9286_MIPI_1280X720_60FPS_10BIT_LINEAR_SPECKLE
    },
    {
        "ov5647",
        1920,
        1080,
        VICAP_CSI2,
        VICAP_MIPI_2LANE,
        VICAP_SOURCE_CSI2,
        K_TRUE,
        VICAP_MIPI_PHY_800M,
        VICAP_CSI_DATA_TYPE_RAW10,
        VICAP_LINERA_MODE,
        VICAP_FLASH_DISABLE,
        VICAP_VI_FIRST_FRAME_FS_TR0,
        0,
        OV_OV5647_MIPI_1920X1080_30FPS_10BIT_LINEAR,
    },
    {
        "ov5647",
        2592,
        1944,
        VICAP_CSI0,
        VICAP_MIPI_2LANE,
        VICAP_SOURCE_CSI0,
        K_TRUE,
        VICAP_MIPI_PHY_800M,
        VICAP_CSI_DATA_TYPE_RAW10,
        VICAP_LINERA_MODE,
        VICAP_FLASH_DISABLE,
        VICAP_VI_FIRST_FRAME_FS_TR0,
        0,
        OV_OV5647_MIPI_2592x1944_10FPS_10BIT_LINEAR,
    },
    {
        "ov5647",
        640,
        480,
        VICAP_CSI2,
        VICAP_MIPI_2LANE,
        VICAP_SOURCE_CSI2,
        K_TRUE,
        VICAP_MIPI_PHY_800M,
        VICAP_CSI_DATA_TYPE_RAW10,
        VICAP_LINERA_MODE,
        VICAP_FLASH_DISABLE,
        VICAP_VI_FIRST_FRAME_FS_TR0,
        0,
        OV_OV5647_MIPI_640x480_60FPS_10BIT_LINEAR,
    },
    {
        "ov5647",
        1920,
        1080,
        VICAP_CSI0,
        VICAP_MIPI_2LANE,
        VICAP_SOURCE_CSI0,
        K_TRUE,
        VICAP_MIPI_PHY_800M,
        VICAP_CSI_DATA_TYPE_RAW10,
        VICAP_LINERA_MODE,
        VICAP_FLASH_DISABLE,
        VICAP_VI_FIRST_FRAME_FS_TR0,
        0,
        OV_OV5647_MIPI_CSI0_1920X1080_30FPS_10BIT_LINEAR,
    },
    {
        "sc201cs",
        1600,
        1200,
        VICAP_CSI1,
        VICAP_MIPI_1LANE,
        VICAP_SOURCE_CSI1,
        K_FALSE,
        VICAP_MIPI_PHY_1200M,
        VICAP_CSI_DATA_TYPE_RAW10,
        VICAP_LINERA_MODE,
        VICAP_FLASH_DISABLE,
        VICAP_VI_FIRST_FRAME_FS_TR0,
        0,
        SC_SC201CS_MIPI_1LANE_RAW10_1600X1200_30FPS_LINEAR,
    },
    {
        "sc201cs",
        1600,
        1200,
        VICAP_CSI1,
        VICAP_MIPI_1LANE,
        VICAP_SOURCE_CSI1,
        K_FALSE,
        VICAP_MIPI_PHY_1200M,
        VICAP_CSI_DATA_TYPE_RAW10,
        VICAP_LINERA_MODE,
        VICAP_FLASH_DISABLE,
        VICAP_VI_FIRST_FRAME_FS_TR0,
        0,
        SC_SC201CS_SLAVE_MODE_MIPI_1LANE_RAW10_1600X1200_30FPS_LINEAR,
    },

    {         
        "ov5647_csi2",         
        1920,         
        1080,         
        VICAP_CSI2,         
        VICAP_MIPI_2LANE,        
        VICAP_SOURCE_CSI2,         
        K_TRUE,         
        VICAP_MIPI_PHY_800M,         
        VICAP_CSI_DATA_TYPE_RAW10,         
        VICAP_LINERA_MODE,         
        VICAP_FLASH_DISABLE,         
        VICAP_VI_FIRST_FRAME_FS_TR0,         
        0,         
        OV_OV5647_MIPI_CSI2_1920X1080_30FPS_10BIT_LINEAR,     
    },
    {         
        "ov5647_csi1",         
        1920,         
        1080,         
        VICAP_CSI1,         
        VICAP_MIPI_2LANE,        
        VICAP_SOURCE_CSI1,         
        K_TRUE,         
        VICAP_MIPI_PHY_800M,         
        VICAP_CSI_DATA_TYPE_RAW10,         
        VICAP_LINERA_MODE,         
        VICAP_FLASH_DISABLE,         
        VICAP_VI_FIRST_FRAME_FS_TR0,         
        0,         
        OV_OV5647_MIPI_CSI1_1920X1080_30FPS_10BIT_LINEAR,     
    },
    {
        "ov5647",
        1920,
        1080,
        VICAP_CSI0,
        VICAP_MIPI_2LANE,
        VICAP_SOURCE_CSI0,
        K_TRUE,
        VICAP_MIPI_PHY_800M,
        VICAP_CSI_DATA_TYPE_RAW10,
        VICAP_LINERA_MODE,
        VICAP_FLASH_DISABLE,
        VICAP_VI_FIRST_FRAME_FS_TR0,
        0,
        OV_OV5647_MIPI_CSI0_1920X1080_30FPS_10BIT_LINEAR_V2,
    },
    {
        "ov5647_csi1",
        1920,
        1080,
        VICAP_CSI1,
        VICAP_MIPI_2LANE,
        VICAP_SOURCE_CSI1,
        K_TRUE,
        VICAP_MIPI_PHY_800M,
        VICAP_CSI_DATA_TYPE_RAW10,
        VICAP_LINERA_MODE,
        VICAP_FLASH_DISABLE,
        VICAP_VI_FIRST_FRAME_FS_TR0,
        0,
        OV_OV5647_MIPI_CSI1_1920X1080_30FPS_10BIT_LINEAR_V2,
    },
    {
        "ov5647_csi2",
        1920,
        1080,
        VICAP_CSI2,
        VICAP_MIPI_2LANE,
        VICAP_SOURCE_CSI2,
        K_TRUE,
        VICAP_MIPI_PHY_800M,
        VICAP_CSI_DATA_TYPE_RAW10,
        VICAP_LINERA_MODE,
        VICAP_FLASH_DISABLE,
        VICAP_VI_FIRST_FRAME_FS_TR0,
        0,
        OV_OV5647_MIPI_CSI2_1920X1080_30FPS_10BIT_LINEAR_V2,
    },

    {
        "xs9922b",
        1280,
        720,
        VICAP_CSI0,
        VICAP_MIPI_2LANE,   //VICAP_MIPI_4LANE
        VICAP_SOURCE_CSI0,
        K_FALSE,
        VICAP_MIPI_PHY_1200M,   //VICAP_MIPI_PHY_1200M
        VICAP_CSI_DATA_TYPE_YUV422_8,
        VICAP_LINERA_MODE,//VICAP_VCID_HDR_3FRAME,  VICAP_LINERA_MODE
        VICAP_FLASH_DISABLE,
        VICAP_VI_FIRST_FRAME_FS_TR0,
        0,
        XS9922B_MIPI_CSI0_1280X720_30FPS_YUV422_DOL3,
    },
    {
        "xs9950_csi1",
        1280,
        720,
        VICAP_CSI1,
        VICAP_MIPI_2LANE,   //VICAP_MIPI_4LANE
        VICAP_SOURCE_CSI1,
        K_FALSE,
        VICAP_MIPI_PHY_1200M,   //VICAP_MIPI_PHY_1200M
        VICAP_CSI_DATA_TYPE_YUV422_8,
        VICAP_LINERA_MODE,//VICAP_VCID_HDR_3FRAME,  VICAP_LINERA_MODE
        VICAP_FLASH_DISABLE,
        VICAP_VI_FIRST_FRAME_FS_TR0,
        0,
        XS9950_MIPI_CSI1_1280X720_30FPS_YUV422,
    },
    {
        "xs9950_csi0",
        1280,
        720,
        VICAP_CSI0,
        VICAP_MIPI_2LANE,   //VICAP_MIPI_4LANE
        VICAP_SOURCE_CSI0,
        K_FALSE,
        VICAP_MIPI_PHY_1200M,   //VICAP_MIPI_PHY_1200M
        VICAP_CSI_DATA_TYPE_YUV422_8,
        VICAP_LINERA_MODE,//VICAP_VCID_HDR_3FRAME,  VICAP_LINERA_MODE
        VICAP_FLASH_DISABLE,
        VICAP_VI_FIRST_FRAME_FS_TR0,
        0,
        XS9950_MIPI_CSI0_1280X720_30FPS_YUV422,
    },
    {
        "xs9950_csi2",
        1280,
        720,
        VICAP_CSI2,
        VICAP_MIPI_2LANE,   //VICAP_MIPI_4LANE
        VICAP_SOURCE_CSI2,
        K_FALSE,
        VICAP_MIPI_PHY_1200M,   //VICAP_MIPI_PHY_1200M
        VICAP_CSI_DATA_TYPE_YUV422_8,
        VICAP_LINERA_MODE,//VICAP_VCID_HDR_3FRAME,  VICAP_LINERA_MODE
        VICAP_FLASH_DISABLE,
        VICAP_VI_FIRST_FRAME_FS_TR0,
        0,
        XS9950_MIPI_CSI2_1280X720_30FPS_YUV422,
    },
    {
        "xs9950_csi0",
        1920,
        1080,
        VICAP_CSI0,
        VICAP_MIPI_2LANE,   //VICAP_MIPI_4LANE
        VICAP_SOURCE_CSI0,
        K_FALSE,
        VICAP_MIPI_PHY_1200M,   //VICAP_MIPI_PHY_1200M
        VICAP_CSI_DATA_TYPE_YUV422_8,
        VICAP_LINERA_MODE,//VICAP_VCID_HDR_3FRAME,  VICAP_LINERA_MODE
        VICAP_FLASH_DISABLE,
        VICAP_VI_FIRST_FRAME_FS_TR0,
        0,
        XS9950_MIPI_CSI0_1920X1080_30FPS_YUV422,
    },
    {
        "gc2053",
        1920,
        1080,
        VICAP_CSI0,
        VICAP_MIPI_2LANE,
        VICAP_SOURCE_CSI0,
        K_FALSE,
        VICAP_MIPI_PHY_1200M,
        VICAP_CSI_DATA_TYPE_RAW10,
        VICAP_LINERA_MODE,
        VICAP_FLASH_DISABLE,
        VICAP_VI_FIRST_FRAME_FS_TR0,
        0,
        GC2053_MIPI_CSI0_1920X1080_30FPS_10BIT_LINEAR,
    },
};

const char *kd_mpi_vicap_get_sensor_string(k_vicap_sensor_type sensor_type)
{
    printf("kd_mpi_vicap_get_sensor_string, sensor_type(%d)\n", sensor_type);

    if (sensor_type >= SENSOR_TYPE_MAX) {
        pr_err("%s, invalid sensor type.\n", __func__);
        return NULL;
    }

    for(k_s32 i = 0; i < sizeof(sensor_type_map_list)/sizeof(k_vicap_sensor_type_map); i++) {
        if (sensor_type_map_list[i].sensor_type == sensor_type) {
            printf("kd_mpi_vicap_get_sensor_string, sensor_string(%s)\n", sensor_type_map_list[i].sensor_string);
            return sensor_type_map_list[i].sensor_string;
        }
    }
    return NULL;
}

k_s32 kd_mpi_vicap_get_sensor_type(k_vicap_sensor_type *sensor_type, const char *sensor_string)
{
    if(!sensor_string) {
        pr_err("%s, type_string is null\n",__func__);
        return K_ERR_VICAP_NULL_PTR;
    }

    for(k_s32 i = 0; i < sizeof(sensor_type_map_list)/sizeof(k_vicap_sensor_type_map); i++) {
        if (!strcmp(sensor_string, sensor_type_map_list[i].sensor_string)) {
            *sensor_type = sensor_type_map_list[i].sensor_type;
            return 0;
        }
    }
    return K_ERR_UNEXIST;
}

k_s32 kd_mpi_vicap_get_sensor_info(k_vicap_sensor_type sensor_type, k_vicap_sensor_info *sensor_info)
{
    if(!sensor_info) {
        pr_err("%s, sensor_info is null\n",__func__);
        return K_ERR_VICAP_NULL_PTR;
    }

    if (sensor_type >= SENSOR_TYPE_MAX) {
        pr_err("%s, invalid sensor type.\n", __func__);
        return K_ERR_VICAP_ILLEGAL_PARAM;
    }
    for(k_s32 i = 0; i < sizeof(sensor_info_list)/sizeof(k_vicap_sensor_info); i++) {
        if (sensor_type == sensor_info_list[i].sensor_type) {
            memcpy(sensor_info, &sensor_info_list[i], sizeof(k_vicap_sensor_info));
            return 0;
        }
    }
    return K_ERR_UNEXIST;
}

k_s32 kd_mpi_sensor_open(const char *sensor_name)
{
    k_s32 fd = 0;
    char dev_name[32];

    if(!sensor_name) {
        pr_err("%s, sensor_name is null\n",__func__);
        return -K_ERR_VICAP_NULL_PTR;
    }

    snprintf(dev_name, sizeof(dev_name), "/dev/sensor_%s", sensor_name);
    fd = open(dev_name, O_RDWR);
    if (fd < 0) {
        pr_err("%s, failed(%d).\n", __func__, fd);
        return -K_ERR_VICAP_NOTREADY;
    }
    return fd;
}

k_s32 kd_mpi_sensor_close(k_s32 fd)
{
    close(fd);
}

k_s32 kd_mpi_sensor_power_set(k_s32 fd, k_bool on)
{
    k_s32 ret;

    ret = ioctl(fd, KD_IOC_SENSOR_S_POWER, &on);
    if (ret != 0) {
        pr_err("%s, error(%d)\n", __func__, ret);
        return K_ERR_VICAP_NOT_SUPPORT;
    }

    return ret;
}

k_s32 kd_mpi_sensor_id_get(k_s32 fd, k_u32 *sensor_id)
{
    k_s32 ret;

    if(!sensor_id) {
        pr_err("%s, sensor_id is null\n",__func__);
        return K_ERR_VICAP_NULL_PTR;
    }

    ret = ioctl(fd, KD_IOC_SENSOR_G_ID, sensor_id);
    if (ret != 0) {
        pr_err("%s, error(%d)\n", __func__, ret);
        return K_ERR_VICAP_NOT_SUPPORT;
    }

    return ret;
}

k_s32 kd_mpi_sensor_init(k_s32 fd, k_sensor_mode mode)
{
    k_s32 ret;

    ret = ioctl(fd, KD_IOC_SENSOR_S_INIT, mode);
    if (ret != 0) {
        pr_err("%s, error(%d)\n", __func__, ret);
        return K_ERR_VICAP_NOT_SUPPORT;
    }

    return ret;
}

k_s32 kd_mpi_sensor_reg_read(k_s32 fd, k_u32 reg_addr, k_u32 *reg_val)
{
    k_s32 ret;

    if(!reg_val) {
        pr_err("%s, reg_val is null\n",__func__);
        return K_ERR_VICAP_NULL_PTR;
    }

    k_sensor_reg reg;
    reg.addr = reg_addr;
    reg.val = 0;

    ret = ioctl(fd, KD_IOC_SENSOR_REG_READ, &reg);
    if (ret != 0) {
        pr_err("%s, error(%d)\n", __func__, ret);
        return K_ERR_VICAP_NOT_SUPPORT;
    }
    *reg_val = reg.val;

    return ret;
}

k_s32 kd_mpi_sensor_reg_write(k_s32 fd, k_u32 reg_addr, k_u32 reg_val)
{
    k_s32 ret;

    k_sensor_reg reg;
    reg.addr = reg_addr;
    reg.val = reg_val;

    ret = ioctl(fd, KD_IOC_SENSOR_REG_WRITE, &reg);
    if (ret != 0) {
        pr_err("%s, error(%d)\n", __func__, ret);
        return K_ERR_VICAP_NOT_SUPPORT;
    }

    return ret;
}

k_s32 kd_mpi_sensor_mode_get(k_s32 fd, k_sensor_mode *mode)
{
    k_s32 ret;

    if(!mode) {
        pr_err("%s, mode is null\n",__func__);
        return K_ERR_VICAP_NULL_PTR;
    }

    ret = ioctl(fd, KD_IOC_SENSOR_G_MODE, mode);
    if (ret != 0) {
        pr_err("%s, error(%d)\n", __func__, ret);
        return K_ERR_VICAP_NOT_SUPPORT;
    }

    return ret;
}

k_s32 kd_mpi_sensor_mode_set(k_s32 fd, k_sensor_mode mode)
{
    k_s32 ret;

    ret = ioctl(fd, KD_IOC_SENSOR_S_MODE, &mode);
    if (ret != 0) {
        pr_err("%s, error(%d)\n", __func__, ret);
        return K_ERR_VICAP_NOT_SUPPORT;
    }

    return ret;
}

k_s32 kd_mpi_sensor_mode_enum(k_s32 fd, k_sensor_enum_mode *enum_mode)
{
    k_s32 ret;

    if(!enum_mode) {
        pr_err("%s, enum_mode is null\n",__func__);
        return K_ERR_VICAP_NULL_PTR;
    }

    ret = ioctl(fd, KD_IOC_SENSOR_ENUM_MODE, enum_mode);
    if (ret != 0) {
        pr_err("%s, error(%d)\n", __func__, ret);
        return K_ERR_VICAP_NOT_SUPPORT;
    }

    return ret;
}

k_s32 kd_mpi_sensor_caps_get(k_s32 fd, k_sensor_caps *caps)
{
    k_s32 ret;

    if(!caps) {
        pr_err("%s, caps is null\n",__func__);
        return K_ERR_VICAP_NULL_PTR;
    }

    ret = ioctl(fd, KD_IOC_SENSOR_G_CAPS, caps);
    if (ret != 0) {
        pr_err("%s, error(%d)\n", __func__, ret);
        return K_ERR_VICAP_NOT_SUPPORT;
    }

    return ret;
}

k_s32 kd_mpi_sensor_connection_check(k_s32 fd, k_s32 *connection)
{
    k_s32 ret;

    if(!connection) {
        pr_err("%s, connection is null\n",__func__);
        return K_ERR_VICAP_NULL_PTR;
    }

    ret = ioctl(fd, KD_IOC_SENSOR_CHECK_CONN, connection);
    if (ret != 0) {
        pr_err("%s, error(%d)\n", __func__, ret);
        return K_ERR_VICAP_NOT_SUPPORT;
    }

    return ret;
}

k_s32 kd_mpi_sensor_stream_enable(k_s32 fd, k_s32 enable)
{
    k_s32 ret;

    ret = ioctl(fd, KD_IOC_SENSOR_S_STREAM, &enable);
    if (ret != 0) {
        pr_err("%s, error(%d)\n", __func__, ret);
        return K_ERR_VICAP_NOT_SUPPORT;
    }

    return ret;
}

k_s32 kd_mpi_sensor_again_set(k_s32 fd, k_sensor_gain gain)
{
    k_s32 ret;

    ret = ioctl(fd, KD_IOC_SENSOR_S_AGAIN, &gain);
    if (ret != 0) {
        pr_err("%s, error(%d)\n", __func__, ret);
        return K_ERR_VICAP_NOT_SUPPORT;
    }

    return ret;
}

k_s32 kd_mpi_sensor_again_get(k_s32 fd, k_sensor_gain *gain)
{
    k_s32 ret;

    if(!gain) {
        pr_err("%s, gain is null\n",__func__);
        return K_ERR_VICAP_NULL_PTR;
    }

    ret = ioctl(fd, KD_IOC_SENSOR_G_AGAIN, gain);
    if (ret != 0) {
        pr_err("%s, error(%d)\n", __func__, ret);
        return K_ERR_VICAP_NOT_SUPPORT;
    }

    return ret;
}

k_s32 kd_mpi_sensor_dgain_set(k_s32 fd, k_sensor_gain gain)
{
    k_s32 ret;

    ret = ioctl(fd, KD_IOC_SENSOR_S_DGAIN, &gain);
    if (ret != 0) {
        pr_err("%s, error(%d)\n", __func__, ret);
        return K_ERR_VICAP_NOT_SUPPORT;
    }

    return ret;
}

k_s32 kd_mpi_sensor_dgain_get(k_s32 fd, k_sensor_gain *gain)
{
    k_s32 ret;

    if(!gain) {
        pr_err("%s, gain is null\n",__func__);
        return K_ERR_VICAP_NULL_PTR;
    }

    ret = ioctl(fd, KD_IOC_SENSOR_G_DGAIN, gain);
    if (ret != 0) {
        pr_err("%s, error(%d)\n", __func__, ret);
        return K_ERR_VICAP_NOT_SUPPORT;
    }

    return ret;
}

k_s32 kd_mpi_sensor_intg_time_set(k_s32 fd, k_sensor_intg_time time)//intigr
{
    k_s32 ret;

    ret = ioctl(fd, KD_IOC_SENSOR_S_INTG_TIME, &time);
    if (ret != 0) {
        pr_err("%s, error(%d)\n", __func__, ret);
        return K_ERR_VICAP_NOT_SUPPORT;
    }

    return ret;
}

k_s32 kd_mpi_sensor_intg_time_get(k_s32 fd, k_sensor_intg_time *time)
{
    k_s32 ret;

    if(!time) {
        pr_err("%s, gain is null\n",__func__);
        return K_ERR_VICAP_NULL_PTR;
    }

    ret = ioctl(fd, KD_IOC_SENSOR_G_INTG_TIME, time);
    if (ret != 0) {
        pr_err("%s, error(%d)\n", __func__, ret);
        return K_ERR_VICAP_NOT_SUPPORT;
    }

    return ret;
}

k_s32 kd_mpi_sensor_fps_get(k_s32 fd, k_u32 *fps)
{
    k_s32 ret;

    if(!fps) {
        pr_err("%s, fps is null\n",__func__);
        return K_ERR_VICAP_NULL_PTR;
    }

    ret = ioctl(fd, KD_IOC_SENSOR_G_FPS, fps);
    if (ret != 0) {
        pr_err("%s, error(%d)\n", __func__, ret);
        return K_ERR_VICAP_NOT_SUPPORT;
    }

    return ret;
}

k_s32 kd_mpi_sensor_fps_set(k_s32 fd, k_u32 fps)
{
    k_s32 ret;

    ret = ioctl(fd, KD_IOC_SENSOR_S_FPS, &fps);
    if (ret != 0) {
        pr_err("%s, error(%d)\n", __func__, ret);
        return K_ERR_VICAP_NOT_SUPPORT;
    }

    return ret;
}

k_s32 kd_mpi_sensor_isp_status_get(k_s32 fd, k_sensor_isp_status *isp_status)
{
    k_s32 ret;

    if(!isp_status) {
        pr_err("%s, isp_status is null\n",__func__);
        return K_ERR_VICAP_NULL_PTR;
    }

    ret = ioctl(fd, KD_IOC_SENSOR_G_ISP_STATUS, isp_status);
    if (ret != 0) {
        pr_err("%s, error(%d)\n", __func__, ret);
        return K_ERR_VICAP_NOT_SUPPORT;
    }

    return ret;
}

k_s32 kd_mpi_sensor_blc_set(k_s32 fd, k_sensor_blc blc)
{
    k_s32 ret;

    ret = ioctl(fd, KD_IOC_SENSOR_S_BLC, &blc);
    if (ret != 0) {
        pr_err("%s, error(%d)\n", __func__, ret);
        return K_ERR_VICAP_NOT_SUPPORT;
    }

    return ret;
}

k_s32 kd_mpi_sensor_wb_set(k_s32 fd, k_sensor_white_balance wb)
{
    k_s32 ret;

    ret = ioctl(fd, KD_IOC_SENSOR_S_WB, &wb);
    if (ret != 0) {
        pr_err("%s, error(%d)\n", __func__, ret);
        return K_ERR_VICAP_NOT_SUPPORT;
    }

    return ret;
}


k_s32 kd_mpi_sensor_tpg_get(k_s32 fd, k_sensor_test_pattern *tpg)
{
    k_s32 ret;

    if(!tpg) {
        pr_err("%s, tpg is null\n",__func__);
        return K_ERR_VICAP_NULL_PTR;
    }

    ret = ioctl(fd, KD_IOC_SENSOR_G_TPG, tpg);
    if (ret != 0) {
        pr_err("%s, error(%d)\n", __func__, ret);
        return K_ERR_VICAP_NOT_SUPPORT;
    }

    return ret;
}

k_s32 kd_mpi_sensor_tpg_set(k_s32 fd, k_sensor_test_pattern tpg)
{
    k_s32 ret;

    ret = ioctl(fd, KD_IOC_SENSOR_S_TPG, &tpg);
    if (ret != 0) {
        pr_err("%s, error(%d)\n", __func__, ret);
        return K_ERR_VICAP_NOT_SUPPORT;
    }

    return ret;
}

k_s32 kd_mpi_sensor_expand_curve_get(k_s32 fd, k_sensor_compand_curve *curve)
{
    k_s32 ret;

    if(!curve) {
        pr_err("%s, curve is null\n",__func__);
        return K_ERR_VICAP_NULL_PTR;
    }

    ret = ioctl(fd, KD_IOC_SENSOR_G_EXPAND_CURVE, curve);
    if (ret != 0) {
        pr_err("%s, error(%d)\n", __func__, ret);
        return K_ERR_VICAP_NOT_SUPPORT;
    }

    return ret;
}

k_s32 kd_mpi_sensor_otpdata_get(k_s32 fd, void *ota_data)
{
    k_s32 ret;

    if(!ota_data) {
        pr_err("%s, ota_data is null\n",__func__);
        return K_ERR_VICAP_NULL_PTR;
    }

    ret = ioctl(fd, KD_IOC_SENSOR_G_OTP_DATA, ota_data);
    if (ret != 0) {
        pr_err("%s, error(%d)\n", __func__, ret);
        return K_ERR_VICAP_NOT_SUPPORT;
    }

    return ret;
}

k_s32 kd_mpi_sensor_otpdata_set(k_s32 fd, void *ota_data)
{
    k_s32 ret;

    if(!ota_data) {
        pr_err("%s, ota_data is null\n",__func__);
        return K_ERR_VICAP_NULL_PTR;
    }

    ret = ioctl(fd, KD_IOC_SENSOR_S_OTP_DATA, ota_data);
    if (ret != 0) {
        pr_err("%s, error(%d)\n", __func__, ret);
        return ret;
    }

    return ret;
}


k_s32 kd_mpi_sensor_mirror_set(k_s32 fd, k_vicap_mirror_mode mirror)
{
    k_s32 ret;

    ret = ioctl(fd, KD_IOC_SENSOR_S_MIRROR, &mirror);
    if (ret != 0) {
        pr_err("%s, error(%d)\n", __func__, ret);
        return ret;
    }

    return ret;
}


