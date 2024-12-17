/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024. All rights reserved.
 * Description: esim sim det
 */

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wincompatible-pointer-types-discards-qualifiers"

#include "sim_adapt.h"
#include <platform_include/basicplatform/linux/util.h>


#ifdef CONFIG_SIM_ADAPT

static irqreturn_t sim_data_irq_handler(int irq, void *data);
static int sim_data_gpio_irq_reg(struct hw_sim_hardware_info *info);

static irqreturn_t sim_data_irq_handler(int irq, void *data)
{
	struct hw_sim_hardware_info *info = data;
	if (info == NULL) {
		hwsim_loge("info is NULL.\n");
		return IRQ_HANDLED;
	}
	info->sim_card_det = true;
	hwsim_loge("sim_data_irq_handler\n");
	return IRQ_HANDLED;
}

static int sim_data_gpio_irq_reg(struct hw_sim_hardware_info *info)
{
	int ret;
	u32 data_gpio = info->esim_phy_card_det[ESIM_DET_DATA_IO];
	ret = gpio_request(data_gpio, SIM1_DATA_GPIO_STR);
	if (ret < 0) {
		hwsim_loge("gpio request failed, ret = %d\n", ret);
		return ret;
	}

	ret = gpio_direction_input(data_gpio);
	if (ret < 0) {
		hwsim_loge("failed to set gpio_data to input,ret = %d\n",ret);
		return ret;
	}

	info->sim_data_irq = gpio_to_irq(data_gpio);
	ret = request_irq(info->sim_data_irq, sim_data_irq_handler, IRQF_TRIGGER_FALLING, "sim1_data", info);
	if (ret < 0) {
		hwsim_loge("failed to request sim1_data_irq,irq = %d\n",info->sim_data_irq);
		return ret;
	}
	return HW_SIM_OK;
}

static sci_hw_op_result_e phy_card_det_set_iomax_power(struct hw_sim_hardware_info *info)
{
	info->sim_det_vcc_io = devm_regulator_get(info->spmidev, "sim_vcc");
	if (info->sim_det_vcc_io == NULL) {
		hwsim_loge("devm_regulator_get failed to get regulator\n");
		return HW_SIM_ERR;
	}
	if (regulator_set_voltage(info->sim_det_vcc_io, VCC_1_8V, VCC_1_8V) != HW_SIM_OK) {
		hwsim_loge("regulator_set_voltage fail\n");
		return HW_SIM_ERR;
	}

	if (regulator_enable(info->sim_det_vcc_io) != HW_SIM_OK) {
		hwsim_loge("regulator_enable fail\n");
		return HW_SIM_ERR;
	}
	return HW_SIM_OK;
}

static void phy_card_det_free_iomax_power(struct hw_sim_hardware_info *info)
{
	gpio_free(info->esim_phy_card_det[ESIM_DET_RST_IO]);
	gpio_free(info->esim_phy_card_det[ESIM_DET_DATA_IO]);
	if (info->sim_data_irq != 0)
		free_irq(info->sim_data_irq,info);
	info->sim_data_irq = 0;
	if (info->sim_det_vcc_io != NULL) {
		if (regulator_disable(info->sim_det_vcc_io) != 0)
			hwsim_loge("regulator_disable fail\n");
		devm_regulator_put(info->sim_det_vcc_io);
		info->sim_det_vcc_io = NULL;
	}
}

static void phy_sim_simulate_power_on(struct hw_sim_hardware_info *info)
{
	int ret;

	u32 sim1_rst_gpio = info->esim_phy_card_det[ESIM_DET_RST_IO];
	gpio_free(sim1_rst_gpio);
	ret = gpio_request(sim1_rst_gpio,SIM1_RST_GPIO_STR);  
	if (ret < 0) {
		hwsim_loge("failed to request sim1_rst_gpio\n");
		return;
	}

	ret = gpio_direction_output(sim1_rst_gpio, 0);
	if (ret < 0) {
		hwsim_loge("rst gpio output 0 failed\n");
		return;
	}

	ret = sim_data_gpio_irq_reg(info);
	if (ret < 0) {
		hwsim_loge("sim1 data to irq failed\n");
		return;
	}

	udelay(ESIM_RESER_LOW_TIME);
	udelay(ESIM_RESER_LOW_TIME);
	udelay(ESIM_RESER_LOW_TIME);
	info->sim_card_det = false;

	ret = gpio_direction_output(sim1_rst_gpio, 1);
	if (ret < 0) {
		hwsim_loge("rst gpio output 1 failed\n");
		return;
	}

	udelay(ESIM_RESER_HIGH_MAX_STEP);
	int delay_time = 0;
	while (delay_time < ESIM_RESER_HIGH_MAX_TIME) {
		hwsim_logi("phy_sim_simulate_power_on delay_time[%d]\n", delay_time);
		delay_time += ESIM_RESER_HIGH_MAX_STEP;
		udelay(ESIM_RESER_HIGH_MAX_STEP);
		if (info->sim_card_det) {
			free_irq(info->sim_data_irq, info);
			info->sim_data_irq = 0;
			break;
		}
	}

	ret = gpio_direction_output(sim1_rst_gpio, 0);
	if (ret < 0)
		hwsim_loge("rst gpio output 0 failed\n");
}

sci_hw_op_result_e esim_no_sd_phy_card_det(struct hw_sim_hardware_info *info)
{
	int ret;
	u32 sim_id = info->sim_id;
	if (sim_id != 1 || info->sim_esim_type != SIM_TYPE_ESIM || info->sim_card_start_det == true) {
		hwsim_loge("det not allow");
		return SIM_HW_CARD_NO_EXIST_WHILE_ESIM;
	}

	info->sim_card_start_det = true;
	info->sim_card_det = false;

	ret = phy_card_det_set_iomax_power(info);
	if (ret == HW_SIM_OK)
		phy_sim_simulate_power_on(info);

	phy_card_det_free_iomax_power(info);

	info->sim_card_start_det = false;
	if (info->sim_card_det) {
		hwsim_loge("sim1 with a card\n");
		return SIM_HW_CARD_EXIST_WHILE_ESIM;
	} else {
		hwsim_loge("sim1 no card\n");
		return SIM_HW_CARD_NO_EXIST_WHILE_ESIM;
	}
}
#endif
#pragma GCC diagnostic pop
