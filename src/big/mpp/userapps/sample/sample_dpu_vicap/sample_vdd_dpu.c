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

#include "sample_dpu_vicap.h"

#define REF_VOL         (1.8)
#define RESOLUTION      (4096)

#define ADC_CHN_ENABLE  (0)
#define ADC_CHN_DISABLE (1)

#define REF_TEMPERATURE         (36.289)//临时使用固定参数。
#define TEMPERATURE_CX         (640)//临时使用固定参数，实际应该从温度标定参数文件中获得
#define TEMPERATURE_CY         (360)//临时使用固定参数，实际应该从温度标定参数文件中获得
#define TEMPERATURE_KX         (0.00015)//临时使用固定参数，实际应该从温度标定参数文件中获得
#define TEMPERATURE_KY         (0.00015)//临时使用固定参数，实际应该从温度标定参数文件中获得

/* dpu file path define */
#define PARAM_PATH          "/sharefs/H1280W720_conf.bin"
#define REF_PATH            "/sharefs/H1280W720_ref.bin"

k_dpu_init_t dpu_init;
k_dpu_dev_attr_t dpu_dev_attr;
k_dpu_chn_lcn_attr_t lcn_attr;
k_dpu_chn_ir_attr_t ir_attr;

/* dpu global variable */
k_dpu_user_space_t g_temp_space;

int sample_adc(float* temp)
{
	//adc
	int ret = 0;
	unsigned int channel = 2;
	unsigned int reg_value = 0;
	float R_series = 10.2;
	float R_ntc = 1.0;
	float R_t0 = 10.0;
	float Bn = 3380;

	float temperature = 0.0f;
	float vol = 0.0;

	rt_device_t adc_dev;
	printf("adc_driver test\n");

	adc_dev = rt_device_find("adc");
	if (adc_dev == RT_NULL)
	{
		printf("device find error\n");
		return -1;
	}
	printf("find device success\n");

	ret = rt_device_open(adc_dev, RT_DEVICE_OFLAG_RDWR);
	if (ret != RT_EOK)
	{
		printf("adc device open err\n");
		return -1;
	}

	///////////
	//get temperature
	//1read
	uint32_t *p;
	p = (uint32_t *)(intptr_t)channel;
	ret = rt_device_control(adc_dev, ADC_CHN_ENABLE, (void *)p);
	if (ret != RT_EOK)
	{
		printf("adc device control err\n");
		return -1;
	}
	printf("adc device control success. ");
	ret = rt_device_read(adc_dev, channel, (void *)&reg_value, sizeof(unsigned int));

	//2get target
	vol = REF_VOL * reg_value / RESOLUTION;
	R_ntc = vol * R_series / (REF_VOL - vol);
	temperature = Bn / (log(R_ntc / R_t0) + Bn / 298.15) - 273.15;
	printf("channel %d reg_value:0x%04x, voltage:%f, R_ntc:%f, temperature:%f *c\n", channel, reg_value, vol, R_ntc, temperature);

	*temp = temperature;
	return 0;
}

int sample_dv_dpu_update_temp(float temperature_obj)
{
	if (temperature_obj < -50 || temperature_obj>100) {
		printf("obj temperature is invalid!\n");
		return -1;
	}

	printf("start temperature rectify:ref_temp: %f, obj_ref:%f  \n", REF_TEMPERATURE, temperature_obj);
	float diff_temp = temperature_obj - REF_TEMPERATURE;
	//if (diff_temp < 3 && diff_temp > -3)
	//{
	//	printf("temperature is near refence temperature!there is no need to rectify \n");
	//	return 0;
	//}

	k_u16 image_width = dpu_dev_attr.dev_param.spp.width_speckle;
	k_u16 image_height = dpu_dev_attr.dev_param.spp.height_speckle;

	float* row_offset_ = dpu_dev_attr.dev_param.lpp.row_offset;
	float* col_offset_ = dpu_dev_attr.dev_param.lpp.col_offset;

	// 温度补偿量计算
	for (k_u16 r = 0; r < image_height; r++) {
		// 行偏差温度补偿
		float dalte_v_ = 0.f;
		float temp_vertical = (r - TEMPERATURE_CY) * diff_temp * TEMPERATURE_KY;
		float y_temp = temp_vertical + r;
		if (y_temp > 0.5 && y_temp < image_height - 0.5)
			dalte_v_ = temp_vertical;
		row_offset_[r] = dalte_v_;
	}

	for (k_u16 c = 0; c < image_width; c++) {
		// 列偏差温度补偿
		float dalte_u_ = 0.f;
		float temp_horizonal = (c - TEMPERATURE_CX) * diff_temp * TEMPERATURE_KX;
		float x_temp = temp_horizonal + c;
		if (x_temp > 0.5 && x_temp < image_width-0.5)
			dalte_u_ = temp_horizonal;
		col_offset_[c] = dalte_u_;
	}

	kd_mpi_sys_mmz_flush_cache(dpu_dev_attr.dev_param.lpp.row_offset_phys, dpu_dev_attr.dev_param.lpp.row_offset, image_height * sizeof(float));
	kd_mpi_sys_mmz_flush_cache(dpu_dev_attr.dev_param.lpp.col_offset_phys, dpu_dev_attr.dev_param.lpp.col_offset, image_width * sizeof(float));

	
	k_s32 ret = kd_mpi_dpu_set_dev_attr(&dpu_dev_attr);
	if (ret) {
		printf("kd_mpi_dpu_set_dev_attr failed\n");
		printf("rectify failed\n");
		return -1;
		//goto err_dpu_delet;
	}
	printf("kd_mpi_dpu_set_dev_attr success\n");
	printf("rectify success\n");

	return 0;
	
}

int sample_dv_dpu_init()
{
    k_s32 ret;

    /************************************************************
     * This part is the demo that actually starts to use DPU
     ***********************************************************/
    /* dpu init */
    dpu_init.start_num = 0;
    dpu_init.buffer_num = 3;
    ret = kd_mpi_dpu_init(&dpu_init);
    if (ret) {
        printf("kd_mpi_dpu_init failed\n");
        goto err_return;
    }

    /* parse file */
    ret = kd_mpi_dpu_parse_file(PARAM_PATH,
                                &dpu_dev_attr.dev_param,
                                &lcn_attr.lcn_param,
                                &ir_attr.ir_param,
                                &g_temp_space);
    // printf("g_temp_space.virt_addr:%p, g_temp_space.phys_addr:%lx\n",
    //     g_temp_space.virt_addr, g_temp_space.phys_addr);
    if (g_temp_space.virt_addr == NULL) {
        printf("g_temp_space.virt_addr is NULL\n");
        goto err_return;
    }
    if (ret) {
        printf("kd_mpi_dpu_parse_file failed\n");
        goto err_return;
    }

    /* set device attribute */
    dpu_dev_attr.mode = DPU_UNBIND;
    dpu_dev_attr.tytz_temp_recfg = K_TRUE;
    dpu_dev_attr.align_depth_recfg = K_TRUE;
    dpu_dev_attr.param_valid = 123;
    // dpu_dev_attr.dev_param.spp.flag_align = K_FALSE;

    ret = kd_mpi_dpu_set_dev_attr(&dpu_dev_attr);
    if (ret) {
        printf("kd_mpi_dpu_set_dev_attr failed\n");
        goto err_dpu_delet;
    }
    printf("kd_mpi_dpu_set_dev_attr success\n");

    /* set reference image */
    ret = kd_mpi_dpu_set_ref_image(REF_PATH);
    if (ret) {
        printf("kd_mpi_dpu_set_ref_image failed\n");
        goto err_dpu_delet;
    }
    printf("kd_mpi_dpu_set_ref_image success\n");

    /* set template image */
    ret = kd_mpi_dpu_set_template_image(&g_temp_space);
    if (ret) {
        printf("kd_mpi_dpu_set_template_image failed\n");
        goto err_dpu_delet;
    }
    printf("kd_mpi_dpu_set_template_image success\n");

    /* start dev */
    ret = kd_mpi_dpu_start_dev();
    if (ret) {
        printf("kd_mpi_dpu_start_dev failed\n");
        goto err_dpu_delet;
    }
    printf("kd_mpi_dpu_start_dev success\n");

    /* set chn attr */
    lcn_attr.chn_num = 0;
    lcn_attr.param_valid = 0;
    ir_attr.chn_num = 1;
    ir_attr.param_valid = 0;
    ret = kd_mpi_dpu_set_chn_attr(&lcn_attr, &ir_attr);
    if (ret) {
        printf("kd_mpi_dpu_set_chn_attr failed\n");
        goto err_dpu_dev;
    }
    printf("kd_mpi_dpu_set_chn_attr success\n");

    /* start channel 0 */
    ret = kd_mpi_dpu_start_chn(0);
    if (ret) {
        printf("kd_mpi_dpu_start_chn 0 failed\n");
        goto err_dpu_dev;
    }
    printf("kd_mpi_dpu_start_chn lcn success\n");

    /* start channel 1 */
    ret = kd_mpi_dpu_start_chn(1);
    if (ret) {
        printf("kd_mpi_dpu_start_chn 1 failed\n");
        goto err_dpu_dev;
    }
    printf("kd_mpi_dpu_start_chn ir success\n");

    return K_SUCCESS;

    /************************************************************
     * This part is used to stop the DPU
     ***********************************************************/
    ret = kd_mpi_dpu_stop_chn(0);
    if (ret) {
        printf("kd_mpi_dpu_stop_chn lcn failed\n");
        return 0;
    }
    ret = kd_mpi_dpu_stop_chn(1);
    if (ret) {
        printf("kd_mpi_dpu_stop_chn ir failed\n");
        return 0;
    }
    printf("kd_mpi_dpu_stop_chn success\n");

err_dpu_dev:
    ret = kd_mpi_dpu_stop_dev();
    if (ret) {
        printf("kd_mpi_dpu_stop_dev failed\n");
        return 0;
    }
    printf("kd_mpi_dpu_stop_dev success\n");

err_dpu_delet:
    kd_mpi_dpu_delete();

err_return:
    return K_FAILED;
}

int sample_dv_dpu_delete()
{
    k_s32 ret;

    ret = kd_mpi_dpu_stop_chn(0);
    if (ret) {
        printf("kd_mpi_dpu_stop_chn lcn failed\n");
        return 0;
    }
    ret = kd_mpi_dpu_stop_chn(1);
    if (ret) {
        printf("kd_mpi_dpu_stop_chn ir failed\n");
        return 0;
    }
    printf("kd_mpi_dpu_stop_chn success\n");

    ret = kd_mpi_dpu_stop_dev();
    if (ret) {
        printf("kd_mpi_dpu_stop_dev failed\n");
        return 0;
    }
    printf("kd_mpi_dpu_stop_dev success\n");

    kd_mpi_dpu_delete();

    return K_SUCCESS;
}