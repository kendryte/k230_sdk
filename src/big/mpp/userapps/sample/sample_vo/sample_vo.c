/* Copyright (c) 2023, Canaan Bright Sight Co., Ltd
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 * 1. Redistributions of source code must retain the above copyright
 * notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 * notice, this list of conditions and the following disclaimer in the
 * documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND
 * CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES,
 * INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 * BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/mman.h>
#include <stdbool.h>

#include "k_module.h"
#include "k_type.h"
#include "k_vb_comm.h"
#include "k_video_comm.h"
#include "k_sys_comm.h"
#include "mpi_vb_api.h"
#include "mpi_vvi_api.h"
#include "mpi_sys_api.h"
#include "mpi_vo_api.h"
#include "k_vo_comm.h"

#include "vo_test_case.h"

void display_hardware_init()
{
    // rst display subsystem
    kd_display_reset();
    // set hardware reset;
    kd_display_set_backlight();
    
}

int main(int argc, char *argv[])
{
    int test_case;

    test_case = atoi(argv[1]);

    display_hardware_init();

    usleep(200000);

    switch (test_case)
    {
    case DISPLAY_DSI_LP_MODE_TEST:
        printf("DISPLAY_DSI_LP_MODE_TEST ------------------ \n");
        dwc_dsi_lpmode_test();
        break;

    case DISPLAY_DSI_HS_MODE_TEST:
        printf("DISPLAY_DSI_HS_MODE_TEST ------------------ \n");
        dwc_dsi_hsmode_test();
        break;

    case DISPLAY_DSI_TEST_PATTERN :
        printf("dwc_dsi_init_with_test_pattern ------------------ \n");
        dwc_dsi_init_with_test_pattern();
        break;

    case DISPALY_VO_BACKGROUND_TEST :
        printf("DISPALY_VO_BACKGROUND_TEST ------------------ \n");
        dwc_dsi_init();
        vo_background_init();
        break;

    case DISPALY_VO_OSD0_TEST :
        printf("DISPALY_VO_OSD0_TEST ------------------ \n");
        dwc_dsi_init();
        vo_osd_insert_frame_test();
        break;

    case DISPALY_VO_LAYER_INSERT_FRAME_TEST :
        printf("DISPALY_VO_LAYER_INSERT_FRAME_TEST ------------------ \n");
        dwc_dsi_init();
        vo_layer_insert_frame_test();
        break;

    case DISPALY_VVI_BING_VO_LAYER_TEST :
        printf("DISPALY_VVI_BANG_VO_LAYER_TEST ------------------ \n");
        dwc_dsi_init();
        vo_layer_bind_test();
        break;

    case DISPALY_VVI_BING_VO_OSD_TEST :
        printf("DISPALY_VVI_BANG_VO_OSD_TEST ------------------ \n");
        dwc_dsi_init();
        vo_osd_bind_test();
        break;

    case DISPALY_VO_INSERT_MULTI_FRAME_OSD0_TEST :
        printf("DISPALY_VO_INSERT_MULTI_FRAME_OSD0_TEST ------------------ \n");
        dwc_dsi_init();
        vo_osd_insert_multi_frame_test();
        break;

    case DISPALY_VVI_BING_VO_OSD_DUMP_FRAME_TEST :
        printf("DISPALY_VVI_BANG_VO_OSD_DUMP_FRAME_TEST ------------------ \n");
        dwc_dsi_init();
        vo_osd_bind_dump_frame_test();
        break;
    
    case DISPALY_VO_1LAN_CASE_TEST :
        kd_mpi_vo_set_user_sync_info(65, 1);
        dwc_dsi_1lan_init();
        vo_1lan_background_init();
        break;

    case DISPALY_VO_DSI_READ_ID :
        dwc_dsi_read_hx8399_id();
        break;

    case DISPALY_VO_LAYER0_ROTATION :
        kd_mpi_vo_set_user_sync_info(11, 1);
        dwc_dsi_layer0_init();
        vo_layer0_scaler_test();
        break;
    
    case DISPALY_VO_CONNECTOR_TEST :
        // sample_connector_init();
        sample_connector_osd_install_frame(HX8377_V2_MIPI_4LAN_1080X1920_30FPS);
        break;
    case DISPALY_VO_LT9611_TEST :
        // sample_connector_init();
        sample_connector_osd_install_frame(LT9611_MIPI_4LAN_1920X1080_60FPS);
        break;
    case DISPALY_VO_ST7701_480x854_TEST :
        sample_connector_osd_install_frame(ST7701_V1_MIPI_2LAN_480X854_30FPS);
        break;
    case DISPALY_VO_ST7701_480x800_TEST :
        sample_connector_osd_install_frame(ST7701_V1_MIPI_2LAN_480X800_30FPS);
        break;
    case DISPALY_VO_WRITEBACK_TEST :
        // sample_connector_wbc_dump_frame(HX8377_V2_MIPI_4LAN_1080X1920_30FPS);
        break;
    case DISPALY_VO_ILI9806_480x800_TEST :
        sample_connector_osd_install_frame(ILI9806_MIPI_2LAN_480X800_30FPS);
        break;

    default :
        break;
    }

    return 0;
}
