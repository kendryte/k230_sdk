#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/mman.h>
#include <errno.h>
#include <fcntl.h>
#include <stdint.h>

#include "k_sensor_comm.h"
#include "mpi_sensor_api.h"
#include "k_vicap_comm.h"
#include "mpi_vicap_api.h"

static void usage(void)
{
    printf("usage: ./sample_sensor_otp.elf -sensor 34 -id zxcvbnmpoiuytre\n");
    printf("Options:\n");
    printf(" -sensor:  ov9286 canmv support :34 ov9286 evb support : 18 \n");
    printf(" -id: serial number for sensor otp \n");
    exit(1);
}


int main(int argc, char *argv[])
{
    char sensor_opt_id[32] = {0};
    k_vicap_sensor_type sensor_type;
    int i = 0;
    size_t len = 0;
    int ret = 0;

    if(argc < 3)
    {
        printf("parameters err \n");
        exit(1);
    }

    for (i = 1; i < argc; i += 2)
    {
        if (strcmp(argv[i], "-help") == 0)
        {
            usage();
        }
        else if (strcmp(argv[i], "-sensor") == 0)
        {
            if ((i + 1) >= argc) {
                printf("sensor parameters missing.\n");
                return -1;
            }

            sensor_type = atoi(argv[i + 1]);

            for (i = i + 2; i < argc; i += 2)
            {
                if (strcmp(argv[i], "-id") == 0)
                {
                    if ((i + 1) >= argc) {
                        printf("id parameters missing.\n");
                        return -1;
                    }

                    len = strlen(argv[i + 1]);
                    if(sensor_type == 34 || sensor_type == 18)
                    {
                        if(len > 15)
                        {
                            printf("ov9286 only suppor 15 serial number \n");
                            exit(1);
                        }
                            
                    }
                    memcpy(sensor_opt_id, argv[i + 1], len);
                }
            }
        }
    }
    printf("sensor_opt_id is %s sensor type is %d len is %ld \n", sensor_opt_id, sensor_type, len);

    k_s32 sensor_fd = -1;
    k_sensor_mode mode;
    k_vicap_sensor_info sensor_info;
    k_sensor_otp_date otp_date;
    k_sensor_otp_date set_otp_date;

    memset(&otp_date, 0, sizeof(k_sensor_otp_date));
    memset(&set_otp_date, 0, sizeof(k_sensor_otp_date));

    ret = kd_mpi_vicap_get_sensor_info(sensor_type, &sensor_info);

    sensor_fd = kd_mpi_sensor_open(sensor_info.sensor_name);
    if (sensor_fd < 0) {
        printf("%s, sensor open failed.\n", __func__);
        return -1;
    }

    memset(&mode, 0 ,sizeof(k_sensor_mode));
    mode.sensor_type = sensor_type;

    ret = kd_mpi_sensor_mode_get(sensor_fd, &mode);
    if (ret) {
        printf("%s, sensor mode get failed.\n", __func__);
        return -1;
    }

    // set mclk
    for(k_s32 idx = 0; idx < SENSOR_MCLK_MAX - 1; idx++)
    {
        if(mode.mclk_setting[idx].mclk_setting_en)
        {
            ret = kd_mpi_vicap_set_mclk(mode.mclk_setting[idx].setting.id, mode.mclk_setting[idx].setting.mclk_sel, mode.mclk_setting[idx].setting.mclk_div, K_TRUE);
        }
    }
    // set sensor power     
    ret = kd_mpi_sensor_power_set(sensor_fd, K_FALSE);
    if (ret) {
        printf("%s, sensor power set failed.\n", __func__);
        return -1;
    }
    usleep(10000);
    ret = kd_mpi_sensor_power_set(sensor_fd, K_TRUE);
    if (ret) {
        printf("%s, sensor power set failed.\n", __func__);
        return -1;
    }

    // set sensor init 
    ret = kd_mpi_sensor_init(sensor_fd, mode);
    if (ret) {
        printf("%s, sensor init failed.\n", __func__);
        return -1;
    }

    // sensor enable
    ret = kd_mpi_sensor_stream_enable(sensor_fd, K_TRUE);
    if (ret) {
        printf("%s, sensor stream on failed.\n", __func__);
        return -1;
    }

    // // get opt 
    // printf("stream on  start  get id \n");
    // getchar();

#define READ_OTP_DATE               1
#define WRITE_OTP_DATE              1


#if WRITE_OTP_DATE
    otp_date.otp_type = 0;
    memcpy(&set_otp_date.otp_date, sensor_opt_id, len);

    ret = kd_mpi_sensor_otpdata_set(sensor_fd, &set_otp_date);
    if (ret == K_ERR_VICAP_OPT_ALREADY_WRITE) {
        printf("%s, sensor opt already  write .\n", __func__);
        kd_mpi_sensor_stream_enable(sensor_fd, K_FALSE);
        return -1;
    }
    else
        printf("WRITE_OTP_DATE  success\n");
    // getchar();
#endif

    sleep(1);

#if READ_OTP_DATE
    otp_date.otp_type = 0;
    ret = kd_mpi_sensor_otpdata_get(sensor_fd, &otp_date);
    if (ret) {
        printf("%s, sensor stream on failed.\n", __func__);
        kd_mpi_sensor_stream_enable(sensor_fd, K_FALSE);
        return -1;
    }
    // for(int i = 0; i < 15; i++)
    //     printf("otp_date date is %x \n", otp_date.otp_date[i]);

    printf("READ_OTP_DATE success otp date is %s \n", otp_date.otp_date);
    // getchar();
    sleep(1);
#endif

    //sensor disable
    kd_mpi_sensor_stream_enable(sensor_fd, K_FALSE);

    return 0;
}