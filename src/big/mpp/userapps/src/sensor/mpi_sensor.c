#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>


#include "k_sensor_comm.h"
#include "k_vicap_comm.h"
#include "k_sensor_ioctl.h"

#include "mpi_vicap_api.h"


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
        "ov9732-1280x720",
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
        30,
        OV_OV9732_MIPI_1280X720_30FPS_10BIT_MCLK_16M_LINEAR
    },
    {
        "ov9732",
        "ov9732-1280x720",
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
        30,
        OV_OV9732_MIPI_1280X720_30FPS_10BIT_MCLK_16M_LINEAR_V2
    },
    {
        "ov9732",
        "ov9732-1280x720",
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
        30,
        OV_OV9732_MIPI_1280X720_30FPS_10BIT_LINEAR
    },
    {
        "ov9286",
        "ov9286-1280x720",
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
        30,
        OV_OV9286_MIPI_1280X720_30FPS_10BIT_MCLK_25M_LINEAR_SPECKLE
    },
    {
        "ov9286",
        "ov9286-1280x720",
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
        30,
        OV_OV9286_MIPI_1280X720_30FPS_10BIT_MCLK_25M_LINEAR_IR
    },
    {
        "ov9286",
        "ov9286-1280x720",
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
        30,
        OV_OV9286_MIPI_1280X720_30FPS_10BIT_MCLK_25M_LINEAR_SPECKLE_V2
    },
    {
        "ov9286",
        "ov9286-1280x720",
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
        30,
        OV_OV9286_MIPI_1280X720_30FPS_10BIT_MCLK_25M_LINEAR_IR_V2
    },
    {
        "ov9286",
        "ov9286-1280x720",
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
        30,
        OV_OV9286_MIPI_1280X720_30FPS_10BIT_LINEAR_IR
    },
    {
        "ov9286",
        "ov9286-1280x720",
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
        30,
        OV_OV9286_MIPI_1280X720_30FPS_10BIT_LINEAR_SPECKLE
    },
    {
        "ov9286",
        "ov9286-1280x720",
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
        30,
        OV_OV9286_MIPI_1280X720_30FPS_10BIT_LINEAR_IR_SPECKLE
    },
    {
        "ov9286",
        "ov9286-1280x720",
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
        60,
        OV_OV9286_MIPI_1280X720_60FPS_10BIT_LINEAR_IR_SPECKLE
    },
    {
        "imx335",
        "imx335-1920x1080",
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
        30,
        IMX335_MIPI_2LANE_RAW12_1920X1080_30FPS_LINEAR
    },
    {
        "imx335",
        "imx335-2592x1944",
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
        30,
        IMX335_MIPI_2LANE_RAW12_2592X1944_30FPS_LINEAR
    },
    {
        "imx335",
        "imx335-2592x1944",
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
        30,
        IMX335_MIPI_4LANE_RAW12_2592X1944_30FPS_LINEAR,
    },
    {
        "imx335",
        "imx335-1920x1080",
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
        30,
        IMX335_MIPI_2LANE_RAW12_1920X1080_30FPS_MCLK_7425_LINEAR
    },
    {
        "imx335",
        "imx335-2592x1944",
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
        30,
        IMX335_MIPI_2LANE_RAW12_2592X1944_30FPS_MCLK_7425_LINEAR
    },
    {
        "imx335",
        "imx335-2592x1944",
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
        30,
        IMX335_MIPI_4LANE_RAW12_2592X1944_30FPS_MCLK_7425_LINEAR,
    },
    {
        "imx335",
        "imx335-2592x1944",
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
        15,
        IMX335_MIPI_4LANE_RAW10_2XDOL,
    },
    {
        "imx335",
        "imx335-2592x1944",
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
        15,
        IMX335_MIPI_4LANE_RAW10_3XDOL,
    },
    {
        "sc035hgs",
        "sc035hgs-640x480",
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
        120,
        SC_SC035HGS_MIPI_1LANE_RAW10_640X480_120FPS_LINEAR
    },
    {
        "sc035hgs",
        "sc035hgs-640x480",
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
        60,
        SC_SC035HGS_MIPI_1LANE_RAW10_640X480_60FPS_LINEAR
    },
    {
        "sc035hgs",
        "sc035hgs-640x480",
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
        30,
        SC_SC035HGS_MIPI_1LANE_RAW10_640X480_30FPS_LINEAR
    },
    {
        "ov9286",
        "ov9286-1280x720",
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
        60,
        OV_OV9286_MIPI_1280X720_60FPS_10BIT_LINEAR_IR
    },
    {
        "ov9286",
        "ov9286-1280x720",
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
        60,
        OV_OV9286_MIPI_1280X720_60FPS_10BIT_LINEAR_SPECKLE
    },
    {
        "ov5647_csi2",
        "ov5647-1920x1080",
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
        30,
        OV_OV5647_MIPI_1920X1080_30FPS_10BIT_LINEAR,
    },
    {
        "ov5647",
        "ov5647-2592x1944",
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
        15,
        OV_OV5647_MIPI_2592x1944_10FPS_10BIT_LINEAR,
    },
    {
        "ov5647",
        "ov5647-640x480",
        640,
        480,
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
        90,
        OV_OV5647_MIPI_640x480_90FPS_10BIT_LINEAR,
    },
    {
        "ov5647",
        "ov5647-1920x1080",
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
        30,
        OV_OV5647_MIPI_CSI0_1920X1080_30FPS_10BIT_LINEAR,
    },
    {
        "sc201cs",
        "sc201cs-1600x1200",
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
        30,
        SC_SC201CS_MIPI_1LANE_RAW10_1600X1200_30FPS_LINEAR,
    },
    {
        "sc201cs",
        "sc201cs-1600x1200",
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
        30,
        SC_SC201CS_SLAVE_MODE_MIPI_1LANE_RAW10_1600X1200_30FPS_LINEAR,
    },

    {         
        "ov5647_csi2",         
        "ov5647-1920x1080",
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
        30,
        OV_OV5647_MIPI_CSI2_1920X1080_30FPS_10BIT_LINEAR,     
    },
    {         
        "ov5647_csi1",        
        "ov5647-1920x1080", 
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
        30,
        OV_OV5647_MIPI_CSI1_1920X1080_30FPS_10BIT_LINEAR,     
    },
    {
        "ov5647",
        "ov5647-1920x1080",
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
        30,
        OV_OV5647_MIPI_CSI0_1920X1080_30FPS_10BIT_LINEAR_V2,
    },
    {
        "ov5647",
        "ov5647-1280x720",
        1280,
        720,
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
        60,
        OV_OV5647_MIPI_CSI0_1280X720_60FPS_10BIT_LINEAR,
    },
    {
        "ov5647",
        "ov5647-1280x960",
        1280,
        960,
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
        45,
        OV_OV5647_MIPI_CSI0_1280X960_45FPS_10BIT_LINEAR,
    },
    {
        "ov5647_csi1",
        "ov5647-1920x1080", 
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
        30,
        OV_OV5647_MIPI_CSI1_1920X1080_30FPS_10BIT_LINEAR_V2,
    },
    {
        "ov5647_csi1",
        "ov5647-640x480", 
        640,
        480,
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
        90,
        OV_OV5647_MIPI_CSI1_640x480_90FPS_10BIT_LINEAR,
    },
    {
        "ov5647_csi1",
        "ov5647-1280x720",
        1280,
        720,
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
        60,
        OV_OV5647_MIPI_CSI1_1280X720_60FPS_10BIT_LINEAR,
    },
    {
        "ov5647_csi1",
        "ov5647-1280x960",
        1280,
        960,
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
        45,
        OV_OV5647_MIPI_CSI1_1280X960_45FPS_10BIT_LINEAR,
    },
    {
        "ov5647_csi2",
        "ov5647-1920x1080", 
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
        30,
        OV_OV5647_MIPI_CSI2_1920X1080_30FPS_10BIT_LINEAR_V2,
    },
    {
        "ov5647_csi2",
        "ov5647-640x480", 
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
        90,
        OV_OV5647_MIPI_CSI2_640x480_90FPS_10BIT_LINEAR,
    },
    {
        "ov5647_csi2",
        "ov5647-1280x720",
        1280,
        720,
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
        60,
        OV_OV5647_MIPI_CSI2_1280X720_60FPS_10BIT_LINEAR,
    },
    {
        "ov5647_csi2",
        "ov5647-1280x960",
        1280,
        960,
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
        45,
        OV_OV5647_MIPI_CSI2_1280X960_45FPS_10BIT_LINEAR,
    },
    // {
    //     "xs9922b",
    //     1280,
    //     720,
    //     VICAP_CSI0,
    //     VICAP_MIPI_2LANE,   //VICAP_MIPI_4LANE
    //     VICAP_SOURCE_CSI0,
    //     K_FALSE,
    //     VICAP_MIPI_PHY_1200M,   //VICAP_MIPI_PHY_1200M
    //     VICAP_CSI_DATA_TYPE_YUV422_8,
    //     VICAP_LINERA_MODE,//VICAP_VCID_HDR_3FRAME,  VICAP_LINERA_MODE
    //     VICAP_FLASH_DISABLE,
    //     VICAP_VI_FIRST_FRAME_FS_TR0,
    //     0,
    //     XS9922B_MIPI_CSI0_1280X720_30FPS_YUV422_DOL3,
    // },
    {
        "xs9950_csi1",
        "xs9950-1280x720",
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
        30,
        XS9950_MIPI_CSI1_1280X720_30FPS_YUV422,
    },
    {
        "xs9950_csi0",
        "xs9950-1280x720",
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
        30,
        XS9950_MIPI_CSI0_1280X720_30FPS_YUV422,
    },
    {
        "xs9950_csi2",
        "xs9950-1280x720",
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
        30,
        XS9950_MIPI_CSI2_1280X720_30FPS_YUV422,
    },
    {
        "xs9950_csi0",
        "xs9950-1280x720",
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
        30,
        XS9950_MIPI_CSI0_1920X1080_30FPS_YUV422,
    },
    {
        "gc2053",
        "gc2053-1920x1080",
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
        30,
        GC2053_MIPI_CSI0_1920X1080_30FPS_10BIT_LINEAR,
    },
    {
        "gc2053",
        "gc2053-1920x1080",
        1920,
        1080,
        VICAP_CSI2,
        VICAP_MIPI_2LANE,
        VICAP_SOURCE_CSI2,
        K_FALSE,
        VICAP_MIPI_PHY_1200M,
        VICAP_CSI_DATA_TYPE_RAW10,
        VICAP_LINERA_MODE,
        VICAP_FLASH_DISABLE,
        VICAP_VI_FIRST_FRAME_FS_TR0,
        0,
        30,
        GC2053_MIPI_CSI2_1920X1080_30FPS_10BIT_LINEAR,
    },
    {
        "gc2053",
        "gc2053-1280x960",
        1280,
        960,
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
        50,
        GC2053_MIPI_CSI0_1280X960_50FPS_10BIT_LINEAR,
    },
    {
        "gc2053",
        "gc2053-1280x960",
        1280,
        960,
        VICAP_CSI2,
        VICAP_MIPI_2LANE,
        VICAP_SOURCE_CSI2,
        K_FALSE,
        VICAP_MIPI_PHY_1200M,
        VICAP_CSI_DATA_TYPE_RAW10,
        VICAP_LINERA_MODE,
        VICAP_FLASH_DISABLE,
        VICAP_VI_FIRST_FRAME_FS_TR0,
        0,
        50,
        GC2053_MIPI_CSI2_1280X960_50FPS_10BIT_LINEAR,
    },
    {
        "gc2053",
        "gc2053-1280x720",
        1280,
        720,
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
        60,
        GC2053_MIPI_CSI0_1280X720_60FPS_10BIT_LINEAR,
    },
    {
        "gc2053",
        "gc2053-1280x720",
        1280,
        720,
        VICAP_CSI2,
        VICAP_MIPI_2LANE,
        VICAP_SOURCE_CSI2,
        K_FALSE,
        VICAP_MIPI_PHY_1200M,
        VICAP_CSI_DATA_TYPE_RAW10,
        VICAP_LINERA_MODE,
        VICAP_FLASH_DISABLE,
        VICAP_VI_FIRST_FRAME_FS_TR0,
        0,
        60,
        GC2053_MIPI_CSI2_1280X720_60FPS_10BIT_LINEAR,
    },
    {
        "gc2093",
        "gc2093-1920x1080",
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
        30,
        GC2093_MIPI_CSI0_1920X1080_30FPS_10BIT_LINEAR,
    },
    {
        "gc2093",
        "gc2093-1920x1080",
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
        60,
        GC2093_MIPI_CSI0_1920X1080_60FPS_10BIT_LINEAR,
    },
    {
        "gc2093",
        "gc2093-1280x720",
        1280,
        720,
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
        60,
        GC2093_MIPI_CSI0_1280X720_90FPS_10BIT_LINEAR,
    },
    {
        "gc2093",
        "gc2093-1280x960",
        1280,
        960,
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
        60,
        GC2093_MIPI_CSI0_1280X960_90FPS_10BIT_LINEAR,
    },
    {
        "gc2093_csi1",
        "gc2093-1920x1080",
        1920,
        1080,
        VICAP_CSI1,
        VICAP_MIPI_2LANE,
        VICAP_SOURCE_CSI1,
        K_FALSE,
        VICAP_MIPI_PHY_1200M,
        VICAP_CSI_DATA_TYPE_RAW10,
        VICAP_LINERA_MODE,
        VICAP_FLASH_DISABLE,
        VICAP_VI_FIRST_FRAME_FS_TR0,
        0,
        30,
        GC2093_MIPI_CSI1_1920X1080_30FPS_10BIT_LINEAR,
    },
    {
        "gc2093_csi1",
        "gc2093-1920x1080",
        1920,
        1080,
        VICAP_CSI1,
        VICAP_MIPI_2LANE,
        VICAP_SOURCE_CSI1,
        K_FALSE,
        VICAP_MIPI_PHY_1200M,
        VICAP_CSI_DATA_TYPE_RAW10,
        VICAP_LINERA_MODE,
        VICAP_FLASH_DISABLE,
        VICAP_VI_FIRST_FRAME_FS_TR0,
        0,
        60,
        GC2093_MIPI_CSI1_1920X1080_60FPS_10BIT_LINEAR,
    },
    {
        "gc2093_csi1",
        "gc2093-1280x720",
        1280,
        720,
        VICAP_CSI1,
        VICAP_MIPI_2LANE,
        VICAP_SOURCE_CSI1,
        K_FALSE,
        VICAP_MIPI_PHY_1200M,
        VICAP_CSI_DATA_TYPE_RAW10,
        VICAP_LINERA_MODE,
        VICAP_FLASH_DISABLE,
        VICAP_VI_FIRST_FRAME_FS_TR0,
        0,
        60,
        GC2093_MIPI_CSI1_1280X720_90FPS_10BIT_LINEAR,
    },
    {
        "gc2093_csi1",
        "gc2093-1280x960",
        1280,
        960,
        VICAP_CSI1,
        VICAP_MIPI_2LANE,
        VICAP_SOURCE_CSI1,
        K_FALSE,
        VICAP_MIPI_PHY_1200M,
        VICAP_CSI_DATA_TYPE_RAW10,
        VICAP_LINERA_MODE,
        VICAP_FLASH_DISABLE,
        VICAP_VI_FIRST_FRAME_FS_TR0,
        0,
        60,
        GC2093_MIPI_CSI1_1280X960_90FPS_10BIT_LINEAR,
    },
    {
        "gc2093_csi2",
        "gc2093-1920x1080",
        1920,
        1080,
        VICAP_CSI2,
        VICAP_MIPI_2LANE,
        VICAP_SOURCE_CSI2,
        K_FALSE,
        VICAP_MIPI_PHY_1200M,
        VICAP_CSI_DATA_TYPE_RAW10,
        VICAP_LINERA_MODE,
        VICAP_FLASH_DISABLE,
        VICAP_VI_FIRST_FRAME_FS_TR0,
        0,
        60,
        GC2093_MIPI_CSI2_1920X1080_60FPS_10BIT_LINEAR,
    },
    {
        "gc2093_csi2",
        "gc2093-1920x1080",
        1920,
        1080,
        VICAP_CSI2,
        VICAP_MIPI_2LANE,
        VICAP_SOURCE_CSI2,
        K_FALSE,
        VICAP_MIPI_PHY_1200M,
        VICAP_CSI_DATA_TYPE_RAW10,
        VICAP_LINERA_MODE,
        VICAP_FLASH_DISABLE,
        VICAP_VI_FIRST_FRAME_FS_TR0,
        0,
        30,
        GC2093_MIPI_CSI2_1920X1080_30FPS_10BIT_LINEAR,
    },
    {
        "gc2093_csi2",
        "gc2093-1280x960",
        1280,
        960,
        VICAP_CSI2,
        VICAP_MIPI_2LANE,
        VICAP_SOURCE_CSI2,
        K_FALSE,
        VICAP_MIPI_PHY_1200M,
        VICAP_CSI_DATA_TYPE_RAW10,
        VICAP_LINERA_MODE,
        VICAP_FLASH_DISABLE,
        VICAP_VI_FIRST_FRAME_FS_TR0,
        0,
        90,
        GC2093_MIPI_CSI2_1280X960_90FPS_10BIT_LINEAR,
    },
    {
        "gc2093_csi2",
        "gc2093-1280x720",
        1280,
        720,
        VICAP_CSI2,
        VICAP_MIPI_2LANE,
        VICAP_SOURCE_CSI2,
        K_FALSE,
        VICAP_MIPI_PHY_1200M,
        VICAP_CSI_DATA_TYPE_RAW10,
        VICAP_LINERA_MODE,
        VICAP_FLASH_DISABLE,
        VICAP_VI_FIRST_FRAME_FS_TR0,
        0,
        90,
        GC2093_MIPI_CSI2_1280X720_90FPS_10BIT_LINEAR,
    },
    {
        "os08a20",
        "os08a20-3840x2160",
        3840,
        2160,
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
        30,
        OS08A20_MIPI_CSI0_3840X2160_30FPS_10BIT_LINEAR,
    },
    {
        "sc132gs",
        "sc132gs-1080x1280",
        1080,
        1280,
        VICAP_CSI2,
        VICAP_MIPI_2LANE,
        VICAP_SOURCE_CSI2,
        K_FALSE,
        VICAP_MIPI_PHY_1200M,
        VICAP_CSI_DATA_TYPE_RAW10,
        VICAP_LINERA_MODE,
        VICAP_FLASH_DISABLE,
        VICAP_VI_FIRST_FRAME_FS_TR0,
        0,
        60,
        SC132GS_MIPI_CSI2_1080X1200_30FPS_10BIT_LINEAR,
    },
    {
        "sc132gs",
        "sc132gs-640x480",
        640,
        480,
        VICAP_CSI2,
        VICAP_MIPI_2LANE,
        VICAP_SOURCE_CSI2,
        K_FALSE,
        VICAP_MIPI_PHY_800M,
        VICAP_CSI_DATA_TYPE_RAW10,
        VICAP_LINERA_MODE,
        VICAP_FLASH_DISABLE,
        VICAP_VI_FIRST_FRAME_FS_TR0,
        0,
        60,
        SC132GS_MIPI_CSI2_640X480_30FPS_10BIT_LINEAR,
    },
        {
        "tys3238",
        "bf3238-1920x1080", //"tys3238-1928x1088",
        1920,//1928,
        1080, //1088,
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
        30,
        BY3238_MIPI_CSI2_1920X1080_30FPS_10BIT_LINEAR,
    },

    {
        "tys3238",
        "gc2093-1280x960", //"tys3238-1928x1088",
        1280,//1928,
        960, //1088,
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
        30,
        BY3238_MIPI_CSI2_1280X960_30FPS_10BIT_LINEAR,
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
        // pr_err("%s, error(%d)\n", __func__, ret);
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


k_s32 kd_mpi_adapt_sensor_get(k_vicap_adapt_id *csi0_adapt_id, k_vicap_adapt_id *csi1_adapt_id, k_vicap_adapt_id *csi2_adapt_id)
{
    k_s32 i = 0, j = 0;
    k_s32 ret = 0;
    k_s32 sensor_fd = -1;
    k_u32 chip_id;
    k_vicap_sensor_info sensor_info;
    k_sensor_mode mode;

    csi0_adapt_id->adapt_len = 0;
    csi1_adapt_id->adapt_len = 0;
    csi2_adapt_id->adapt_len = 0;

    for (i = 0; i != SENSOR_TYPE_MAX; i++) 
    {
       memset(&sensor_info, 0, sizeof(k_vicap_sensor_info));
       memset(&mode, 0, sizeof(k_sensor_mode));

       ret = kd_mpi_vicap_get_sensor_info((k_vicap_sensor_type)i , &sensor_info);
       if (ret) {
            printf("kd_mpi_vicap_adapt_config, the sensor type not supported! i is %d \n", i);
            continue;
       }    
       // open sensor 
        sensor_fd = kd_mpi_sensor_open(sensor_info.sensor_name);
        if (sensor_fd < 0) {
            pr_err("%s, sensor open failed.\n", __func__);
        }

        mode.sensor_type = sensor_info.sensor_type;
        ret = kd_mpi_sensor_mode_get(sensor_fd, &mode);
        if (ret) {
            pr_err("%s, sensor mode get failed. i is %d \n", __func__, i);
        }

        // check sensor need mclk
        for(k_s32 idx = 0; idx < SENSOR_MCLK_MAX - 1; idx++)
        {
            if(mode.mclk_setting[idx].mclk_setting_en)
            {
                ret = kd_mpi_vicap_sensor_set_mclk(mode.mclk_setting[idx].setting);
            }
            else
            {
                ret = kd_mpi_vicap_sensor_disable_mclk(mode.mclk_setting[idx].setting);
            }
        }

        // ret = kd_mpi_sensor_power_set(sensor_fd, K_TRUE);
        ret = kd_mpi_sensor_id_get(sensor_fd, &chip_id);
        if(ret == 0)
        {
            // chip ok
            switch(sensor_info.csi_num)
            {
                case VICAP_CSI0 :
                    csi0_adapt_id->adapt_id[csi0_adapt_id->adapt_len] = i;
                    csi0_adapt_id->adapt_len = csi0_adapt_id->adapt_len + 1;
                    break;
                case VICAP_CSI1 :
                    csi1_adapt_id->adapt_id[csi1_adapt_id->adapt_len] = i;
                    csi1_adapt_id->adapt_len = csi1_adapt_id->adapt_len + 1;
                    break;
                case VICAP_CSI2 :
                    csi2_adapt_id->adapt_id[csi2_adapt_id->adapt_len] = i;
                    csi2_adapt_id->adapt_len = csi2_adapt_id->adapt_len + 1;
                    break;
                default : 
                    printf("csi num err \n");
                    break;
            }
        }

        kd_mpi_sensor_close(sensor_fd);
    }

    return 0;
}


/*
Sensor Name: Sensor1, FPS: 60, Width: 1920, Height: 1080
Sensor Name: Sensor2, FPS: 60, Width: 1280, Height: 720
Sensor Name: Sensor3, FPS: 30, Width: 1920, Height: 1080
Sensor Name: Sensor4, FPS: 30, Width: 1280, Height: 720
*/
static int compare_sensor_info(const void *a, const void *b) {
    k_vicap_sensor_info *sensorA = (k_vicap_sensor_info *)a;
    k_vicap_sensor_info *sensorB = (k_vicap_sensor_info *)b;

    // Compare fps
    if (sensorA->fps != sensorB->fps) {
        return sensorB->fps - sensorA->fps;
    }
    // If fps is the same, compare width
    if (sensorA->width != sensorB->width) {
        return sensorB->width - sensorA->width;
    }
    // If width is the same, compare height
    return sensorB->height - sensorA->height;
}

extern k_u32 get_mirror_by_sensor_type(k_vicap_sensor_type type);

k_s32 kd_mpi_sensor_adapt_get(k_vicap_probe_config *config, k_vicap_sensor_info *info)
{
#define MAX_SENSOR_COUNT (16)

    k_s32 ret = 0;
    k_s32 sensor_fd = -1;
    k_u32 chip_id;
    k_sensor_mode mode;

    k_u32 sensor_info_count = 0;
    k_vicap_sensor_info sensor_info, *p_sensor_info = NULL;
    k_vicap_sensor_info sensor_info_list[MAX_SENSOR_COUNT];

    if (((void *)0 == config) || ((void *)0 == info))
    {
        return 2;
    }
    sensor_info_count = 0;
    memset(info, 0, sizeof(k_vicap_sensor_info));
    memset(&sensor_info_list[0], 0, sizeof(sensor_info_list));

    for (int sensor_idx = 0; sensor_idx != SENSOR_TYPE_MAX; sensor_idx++)
    {
        if (sensor_info_count >= MAX_SENSOR_COUNT)
        {
            printf("kd_mpi_sensor_adapt_get, too may ids\n");
            break;
        }

        memset(&mode, 0, sizeof(k_sensor_mode));
        memset(&sensor_info, 0, sizeof(k_vicap_sensor_info));

        ret = kd_mpi_vicap_get_sensor_info((k_vicap_sensor_type)sensor_idx, &sensor_info);
        if (ret)
        {
            // printf("kd_mpi_vicap_adapt_config, the sensor type not supported! index is %d \n", sensor_idx);
            continue;
        }

        if (config->csi_num != sensor_info.csi_num)
        {
            continue;
        }

        // open sensor
        sensor_fd = kd_mpi_sensor_open(sensor_info.sensor_name);
        if (sensor_fd < 0)
        {
            printf("%s, sensor open failed.\n", __func__);
            continue;
        }

        mode.sensor_type = sensor_info.sensor_type;
        ret = kd_mpi_sensor_mode_get(sensor_fd, &mode);
        if (ret)
        {
            printf("%s, sensor mode get failed. i is %d \n", __func__, sensor_idx);
            continue;
        }

        // check sensor need mclk
        for (k_s32 idx = 0; idx < SENSOR_MCLK_MAX - 1; idx++)
        {
            if (mode.mclk_setting[idx].mclk_setting_en)
            {
                ret = kd_mpi_vicap_sensor_set_mclk(mode.mclk_setting[idx].setting);
            }
            else
            {
                ret = kd_mpi_vicap_sensor_disable_mclk(mode.mclk_setting[idx].setting);
            }
        }

        // ret = kd_mpi_sensor_power_set(sensor_fd, K_TRUE);
        ret = kd_mpi_sensor_id_get(sensor_fd, &chip_id);
        if (ret == 0)
        {
            memcpy(&sensor_info_list[sensor_info_count], &sensor_info, sizeof(k_vicap_sensor_info));
            sensor_info_count++;
        }

        kd_mpi_sensor_close(sensor_fd);
    }

    if (sensor_info_count)
    {
        qsort(&sensor_info_list[0], sensor_info_count, sizeof(k_vicap_sensor_info), compare_sensor_info);

        /* first find wanted fps */
        if(0x00 != config->fps)
        {
            for (int i = 0; i < sensor_info_count; i++)
            {
                p_sensor_info = &sensor_info_list[i];

                if ((config->width == p_sensor_info->width) && (config->height == p_sensor_info->height) && (config->fps == p_sensor_info->fps))
                {
                    memcpy(info, p_sensor_info, sizeof(k_vicap_sensor_info));
                    goto _on_success;
                }
            }
        }

        /* find same resolution */
        for (int i = 0; i < sensor_info_count; i++)
        {
            p_sensor_info = &sensor_info_list[i];

            if ((config->width == p_sensor_info->width) && (config->height == p_sensor_info->height) /* && (config->fps == p_sensor_info->fps) */)
            {
                memcpy(info, p_sensor_info, sizeof(k_vicap_sensor_info));
                goto _on_success;
            }
        }

        /* find a bigger resolution */
        for (int i = 0; i < sensor_info_count; i++)
        {
            p_sensor_info = &sensor_info_list[i];

            if ((config->width <= p_sensor_info->width) && (config->height <= p_sensor_info->height) /* && (config->fps == p_sensor_info->fps) */)
            {
                memcpy(info, p_sensor_info, sizeof(k_vicap_sensor_info));
                goto _on_success;
            }
        }
    }

    return 1;

_on_success:

    strncpy(config->sensor_name, info->sensor_name, sizeof(config->sensor_name));
    config->mirror = get_mirror_by_sensor_type(info->sensor_type);

    printf("probe sensor type %d, mirror %d\n", info->sensor_type, config->mirror);

    return 0;

#undef MAX_SENSOR_COUNT
}
