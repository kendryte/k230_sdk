#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>


#include "k_connector_comm.h"
#include "k_connector_ioctl.h"
#include "k_vo_comm.h"

#define pr_debug(...) //printf(__VA_ARGS__)
#define pr_info(...) //printf(__VA_ARGS__)
#define pr_warn(...) //printf(__VA_ARGS__)
#define pr_err(...)  printf(__VA_ARGS__)


const k_connector_info connector_info_list[] = {
    {
        "hx8399",
        0, 
        0, 
        BACKGROUND_BLACK_COLOR,
        11,
        K_DSI_4LAN,
        K_BURST_MODE,
        K_VO_LP_MODE,
        {15, 295, 0x17, 0x96},
        {74250, 445500, 1160, 1080, 20, 20, 40, 2134, 1920, 5, 8, 206},
        HX8377_V2_MIPI_4LAN_1080X1920_30FPS,
    },
};


k_s32 kd_mpi_get_connector_info(k_connector_type connector_type, k_connector_info *connector_info)
{
    if(!connector_info) {
        pr_err("%s, connector_info is null\n",__func__);
        return K_ERR_VO_NULL_PTR;
    }

    if (connector_type >= CONNECTOR_NUM_MAX) {
        pr_err("%s, invalid connector_type type.\n", __func__);
        return K_ERR_VO_ILLEGAL_PARAM;
    }
    for(k_s32 i = 0; i < sizeof(connector_info_list)/sizeof(k_connector_info); i++) {
        if (connector_type == connector_info_list[i].type) {
            memcpy(connector_info, &connector_info_list[i], sizeof(k_connector_info));
            return 0;
        }
    }
    return K_ERR_UNEXIST;
}


k_s32 kd_mpi_connector_open(const char *connector_name)
{
    k_s32 fd = 0;
    char dev_name[52];

    if(!connector_name) {
        pr_err("%s, sensor_name is null\n",__func__);
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

k_s32 kd_mpi_connector_id_get(k_s32 fd, k_u32 *sensor_id)
{
    k_s32 ret;

    if(!sensor_id) {
        pr_err("%s, sensor_id is null\n",__func__);
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