#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>

#include "k_connector_comm.h"
#include "k_connector_ioctl.h"
#include "k_vo_comm.h"
#include "mpi_connector_api.h"

#define pr_debug(...) // printf(__VA_ARGS__)
#define pr_info(...) // printf(__VA_ARGS__)
#define pr_warn(...) // printf(__VA_ARGS__)
#define pr_err(...) printf(__VA_ARGS__)

k_connector_info connector_info_list[] = {
    {
        "hx8399",
        0,
        0,
        BACKGROUND_BLACK_COLOR,
        11,
        7,
        K_DSI_4LAN,
        K_BURST_MODE,
        K_VO_LP_MODE,
        { 15, 295, 0x17, 0x96 },
        { 74250, 445500, 1160, 1080, 20, 20, 40, 2134, 1920, 5, 8, 206 },
        HX8377_V2_MIPI_4LAN_1080X1920_30FPS,
    },
    {
        "lt9611",
        0,
        0,
        BACKGROUND_BLACK_COLOR,
        10,
        0,
        K_DSI_4LAN,
        K_BURST_MODE,
        K_VO_LP_MODE,
        { 0, 0, 0, 0 },
        { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
        LT9611_MIPI_ADAPT_RESOLUTION,
    },
    {
        "lt9611",
        0,
        0,
        BACKGROUND_BLACK_COLOR,
        10,
        3,
        K_DSI_4LAN,
        K_BURST_MODE,
        K_VO_LP_MODE,
        { 15, 295, 0x09, 0x96 },
        { 148500, 891000, 2200, 1920, 44, 148, 88, 1125, 1080, 5, 4, 36 },
        LT9611_MIPI_4LAN_1920X1080_60FPS,
    },
    {
        "lt9611",
        0,
        0,
        BACKGROUND_BLACK_COLOR,
        10,
        3,
        K_DSI_4LAN,
        K_BURST_MODE,
        K_VO_LP_MODE,
        { 15, 295, 0x09, 0x96 },
        { 148500, 891000, 2200, 1920, 44, 148, 88, 1125, 1080, 5, 4, 36 },
        LT9611_MIPI_4LAN_1920X1080_30FPS,
    },
    // {
    //     "lt9611",
    //     0,
    //     0,
    //     BACKGROUND_BLACK_COLOR,
    //     10,
    //     7,
    //     K_DSI_4LAN,
    //     K_BURST_MODE,
    //     K_VO_LP_MODE,
    //     { 15, 295, 0x19, 0x96 },
    //     { 74250, 445500, 2200, 1920, 44, 148, 88, 1125, 1080, 5, 4, 36 },
    //     LT9611_MIPI_4LAN_1920X1080_30FPS,
    // },
    {
        "lt9611",
        0,
        0,
        BACKGROUND_BLACK_COLOR,
        9,
        7,
        K_DSI_4LAN,
        K_BURST_MODE,
        K_VO_LP_MODE,
        { 15, 295, 0x19, 0x96 },
        { 74250, 445500, 1616, 1280, 40, 220, 76, 765, 720, 5, 4, 36 },
        LT9611_MIPI_4LAN_1280X720_60FPS,
    },
    {
        "lt9611",
        0,
        0,
        BACKGROUND_BLACK_COLOR,
        9,
        7,
        K_DSI_4LAN,
        K_BURST_MODE,
        K_VO_LP_MODE,
        { 15, 295, 0x19, 0x96 },
        { 74250, 445500, 1940, 1280, 40, 220, 400, 765, 720, 5, 4, 36 },
        LT9611_MIPI_4LAN_1280X720_50FPS,
    },
    {
        "lt9611",
        0,
        0,
        BACKGROUND_BLACK_COLOR,
        9,
        7,
        K_DSI_4LAN,
        K_BURST_MODE,
        K_VO_LP_MODE,
        { 15, 295, 0x19, 0x96 },
        { 74250, 445500, 3232, 1280, 40, 220, 1692, 765, 720, 5, 4, 36 },
        LT9611_MIPI_4LAN_1280X720_30FPS,
    },
    {
        "lt9611",
        0,
        0,
        BACKGROUND_BLACK_COLOR,
        9,
        23,
        K_DSI_4LAN,
        K_BURST_MODE,
        K_VO_LP_MODE,
        { 15, 394, 0x39, 0x96 },
        { 24750, 148500, 800, 640, 96, 48, 16, 525, 480, 5, 4, 36 },
        LT9611_MIPI_4LAN_640X480_60FPS,
    },
    {
        "lt9611",
        0,
        0,
        BACKGROUND_BLACK_COLOR,
        8,
        43,
        K_DSI_4LAN,
        K_BURST_MODE,
        K_VO_LP_MODE,
        { 15, 214, 0x3f, 0x96 },
        { 13500, 81000, 752, 320, 96, 48, 288, 300, 240, 10, 14, 36 },
        OTHER_MIPI_4LAN_320X240_60FPS,
    },
    // {
    //     "st7701",
    //     0,
    //     0,
    //     BACKGROUND_BLACK_COLOR,
    //     10,
    //     24,
    //     K_DSI_2LAN,
    //     K_BURST_MODE,
    //     K_VO_LP_MODE,
    //     { 10, 259, 0x27, 0x84 },
    //     { 23760, 285120, 660, 480, 10, 20, 150, 1200, 800, 10, 20, 370 },
    //     ST7701_V1_MIPI_2LAN_480X800_30FPS,
    // },
    {
        "st7701",
        0,
        0,
        BACKGROUND_BLACK_COLOR,
        10,
        14,
        K_DSI_2LAN,
        K_BURST_MODE,
        K_VO_LP_MODE,
        { 9, 196, 0x17, 0xa3},  //0x96
        { 39600, 475200, 600, 480, 20, 20, 80, 1100, 800, 10, 70, 220},
        ST7701_V1_MIPI_2LAN_480X800_30FPS,
    },

    {
        "st7701",
        0,
        0,
        BACKGROUND_PINK_COLOR,
        10,
        24,
        K_DSI_2LAN,
        K_BURST_MODE,
        K_VO_LP_MODE,
        { 10, 259, 0x27, 0x84 },
        { 23760, 285120, 660, 480, 10, 20, 150, 1200, 854, 10, 20, 316 },
        ST7701_V1_MIPI_2LAN_480X854_30FPS,
    },
    // {
    //     "ili9806",
    //     0,
    //     0,
    //     m,
    //     9,
    //     44,
    //     K_DSI_2LAN,
    //     K_BURST_MODE,
    //     K_VO_LP_MODE,
    //     { 9, 262, 0x37, 0x82},
    //     { 13200, 158400, 500, 480, 5, 10, 5, 880, 800, 5, 20, 55},
    //     ILI9806_MIPI_2LAN_480X800_30FPS,
    // },
    {
        "ili9806",
        0,
        0,
        BACKGROUND_PINK_COLOR,
        10,
        24,
        K_DSI_2LAN,
        K_BURST_MODE,
        K_VO_LP_MODE,
        { 10, 259, 0x27, 0x84 },
        { 23760, 285120, 660, 480, 10, 20, 150, 1200, 800, 10, 20, 370 },
        ILI9806_MIPI_2LAN_480X800_30FPS,
    },
};

k_s32 kd_mpi_get_connector_info(k_connector_type connector_type, k_connector_info* connector_info)
{
    if (!connector_info) {
        pr_err("%s, connector_info is null\n", __func__);
        return K_ERR_VO_NULL_PTR;
    }

    for (k_s32 i = 0; i < sizeof(connector_info_list) / sizeof(k_connector_info); i++) {
        if (connector_type == connector_info_list[i].type) {
            memcpy(connector_info, &connector_info_list[i], sizeof(k_connector_info));
            return 0;
        }
    }
    return K_ERR_UNEXIST;
}

k_s32 kd_mpi_connector_adapt_resolution(k_connector_type type, k_connector_negotiated_data* negotiated_data)
{
    k_u32 ret = 0;
    k_s32 connector_fd;
    k_connector_type connector_type;
    k_connector_info connector_info;

    // step 1
    // read HDMI monitor EDID and negotiate the resolution
    memset(&connector_info, 0, sizeof(k_connector_info));
    connector_type = type;
    ret = kd_mpi_get_connector_info(connector_type, &connector_info);
    if (ret) {
        printf("%s get connector info failed for connector_type %d\n", __func__, connector_type);
        return ret;
    }

    connector_fd = kd_mpi_connector_open(connector_info.connector_name);
    if (connector_fd < 0) {
        printf("%s connector open %s failed \n", __func__, connector_info.connector_name);
        return K_ERR_VO_NOTREADY;
    }
    kd_mpi_connector_get_negotiated_data(connector_fd, negotiated_data);
    kd_mpi_connector_close(connector_fd);

    // step 2
    // according to the negotiated resolution and then set the prefered resolution
    memset(&connector_info, 0, sizeof(k_connector_info));
    connector_type = negotiated_data->negotiated_types[0];
    ret = kd_mpi_get_connector_info(connector_type, &connector_info);
    if (ret) {
        printf("%s get connector info failed for connector_type %d\n", __func__, connector_type);
        return ret;
    }

    connector_fd = kd_mpi_connector_open(connector_info.connector_name);
    if (connector_fd < 0) {
        printf("%s connector open %s failed \n", __func__, connector_info.connector_name);
        return K_ERR_VO_NOTREADY;
    }
    kd_mpi_connector_power_set(connector_fd, 1);
    kd_mpi_connector_init(connector_fd, connector_info);

    return 0;
}

k_s32 kd_mpi_connector_open(const char* connector_name)
{
    k_s32 fd = 0;
    char dev_name[52];

    if (!connector_name) {
        pr_err("%s, sensor_name is null\n", __func__);
        return -K_ERR_VO_NULL_PTR;
    }

    snprintf(dev_name, sizeof(dev_name), "/dev/connector_%s", connector_name);
    fd = open(dev_name, O_RDWR);
    if (fd < 0) {
        pr_err("%s, failed(%d).\n", __func__, fd);
        return -K_ERR_VO_NOTREADY;
    }
    return fd;
}

k_s32 kd_mpi_connector_close(k_s32 fd)
{
    close(fd);
}

k_s32 kd_mpi_connector_power_set(k_s32 fd, k_bool on)
{
    k_s32 ret;

    ret = ioctl(fd, KD_IOC_CONNECTOR_S_POWER, &on);
    if (ret != 0) {
        pr_err("%s, error(%d)\n", __func__, ret);
        return K_ERR_VO_NOT_SUPPORT;
    }

    return ret;
}

k_s32 kd_mpi_connector_id_get(k_s32 fd, k_u32* sensor_id)
{
    k_s32 ret;

    if (!sensor_id) {
        pr_err("%s, sensor_id is null\n", __func__);
        return K_ERR_VO_NULL_PTR;
    }

    ret = ioctl(fd, KD_IOC_CONNECTOR_G_ID, sensor_id);
    if (ret != 0) {
        pr_err("%s, error(%d)\n", __func__, ret);
        return K_ERR_VO_NOT_SUPPORT;
    }

    return ret;
}

k_s32 kd_mpi_connector_init(k_s32 fd, k_connector_info info)
{
    k_s32 ret;

    ret = ioctl(fd, KD_IOC_CONNECTOR_S_INIT, info);
    if (ret != 0) {
        pr_err("%s, error(%d)\n", __func__, ret);
        return K_ERR_VO_NOT_SUPPORT;
    }

    return ret;
}

k_s32 kd_mpi_connector_get_negotiated_data(k_s32 fd, k_connector_negotiated_data* negotiated_data)
{
    k_s32 ret;

    ret = ioctl(fd, KD_IOC_CONNECTOR_G_NEG_DATA, negotiated_data);
    if (ret == -1) {
        pr_err("%s, error(%d)\n", __func__, ret);
        return K_ERR_VO_NOT_SUPPORT;
    }

    return ret;
}

k_s32 kd_mpi_connector_set_mirror(k_s32 fd, k_connector_mirror mirror)
{
    k_s32 ret;

    ret = ioctl(fd, KD_IOC_CONNECTOR_S_MIRROR, &mirror);
    if (ret != 0) {
        pr_err("%s, error(%d)\n", __func__, ret);
        return K_ERR_VO_NOT_SUPPORT;
    }

    return ret;
}
