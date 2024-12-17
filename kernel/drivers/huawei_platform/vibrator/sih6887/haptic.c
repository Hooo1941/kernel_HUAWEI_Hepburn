/*
  * haptic.c
  *
  * code for vibrator
  *
  * Copyright (c) 2023 Huawei Technologies Co., Ltd.
  *
  * This software is licensed under the terms of the GNU General Public
  * License version 2, as published by the Free Software Foundation, and
  * may be copied, distributed, and modified under those terms.
  *
  * This program is distributed in the hope that it will be useful,
  * but WITHOUT ANY WARRANTY; without even the implied warranty of
  * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
  * GNU General Public License for more details.
  *
 */

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/i2c.h>
#include <linux/of_gpio.h>
#include <linux/delay.h>
#include <linux/device.h>
#include <linux/firmware.h>
#include <linux/slab.h>
#include <linux/version.h>
#include <linux/input.h>
#include <linux/interrupt.h>
#include <linux/debugfs.h>
#include <linux/miscdevice.h>
#include <linux/uaccess.h>
#include <linux/syscalls.h>
#include <linux/power_supply.h>
#include <linux/vmalloc.h>
#include <linux/pm_qos.h>
#include <linux/mm.h>
#include <sound/core.h>
#include <sound/pcm.h>
#include <sound/pcm_params.h>
#include <sound/control.h>
#include <sound/soc.h>
#include <linux/errno.h>
#include <linux/mman.h>

#include "haptic_mid.h"
#include "haptic_regmap.h"
#include "haptic_config.h"
#include "haptic.h"
#include "sih688x_reg.h"

/*****************************************************
 *
 * macro
 *
 *****************************************************/
#define SIH_WAVEFORM_MAX_NUM				20
#define SIH_RAMWAVEFORM_MAX_NUM				3
#define SIH_RTP_DATA_CONT_LEN				(1024 * 1024 * 10)
#define SIH_MEMDEV_MAJOR				0
#define SIH_MEMDEV_NR_DEVS				1
#define SIH_MEMDEV_SIZE					4096
#define SIH_MEMDEV_DATA_BUFFER				1024

#define SIH_DEV_HAPTIC_NAME				"si_haptic"
#define SI_HAPTIC_NAME					"haptics"
/*****************************************************
 *
 * variable
 *
 *****************************************************/
static char si_rtp_idx_name[SI_RTP_NAME_MAX] = {0};
char sih_ram_name[SIH_RAMWAVEFORM_MAX_NUM][SIH_HP_RTP_NAME_MAX] = {
	{"aw8692x_ram.bin"},
};

char sih_rtp_name[SIH_WAVEFORM_MAX_NUM][SIH_HP_RTP_NAME_MAX] = {
	{"haptic_rtp_osc_24K_5s.bin"},
	{"haptic_rtp.bin"},
	{"haptic_rtp_lighthouse.bin"},
	{"haptic_rtp_silk.bin"},
};

static const unsigned char long_amp_again_val[SI_LONG_MAX_AMP_CFG] = {
	0x80, 0x80, 0x70, 0x60, 0x50, 0x40, 0x30, 0x20, 0x10};
static const unsigned char short_amp_again_val[SI_SHORT_MAX_AMP_CFG] = {
	0x80, 0x80, 0x66, 0x4c, 0x33, 0x19};

static haptic_container_t *rtp_data_cont;

typedef struct sih_haptic_ptr {
	unsigned char sih_num;
	sih_haptic_t *g_haptic[SIH_HAPTIC_MAX_DEV_NUM];
} sih_haptic_ptr_t;

/*****************************************************
 *
 * variable
 *
 *****************************************************/
static sih_haptic_ptr_t g_haptic_t;
static sih_haptic_t *s_haptic;

/*****************************************************
 *
 * Functional
 *
 *****************************************************/
static int sih_parse_hw_dts(
	struct device *dev, sih_haptic_t *sih_haptic,
	struct device_node *np)
{
	struct device_node *sih_node = np;

	if (sih_node == NULL) {
		hp_err("%s haptic device node acquire failed\n", __func__);
		return -EINVAL;
	}

	/* acquire reset gpio */
	sih_haptic->chip_attr.reset_gpio =
		of_get_named_gpio(sih_node, "reset-gpio", 0);
	if (sih_haptic->chip_attr.reset_gpio < 0) {
		hp_err("%s: reset gpio acquire failed\n", __func__);
		return -EIO;
	}

	/* acquire irq gpio */
	sih_haptic->chip_attr.irq_gpio =
		of_get_named_gpio(sih_node, "irq-gpio", 0);
	if (sih_haptic->chip_attr.irq_gpio < 0) {
		hp_err("%s: irq gpio acquire failed\n", __func__);
		return -EIO;
	}

	hp_info("reset_gpio = %d, irq_gpio = %d\n",
		sih_haptic->chip_attr.reset_gpio,
		sih_haptic->chip_attr.irq_gpio);

	return 0;
}

static int sih_parse_dts(
	struct device *dev, sih_haptic_t *sih_haptic,
	struct device_node *np)
{
	int ret = -1;

	/* Obtain DTS information and data */
	if (np) {
		ret = sih_parse_hw_dts(dev, sih_haptic, np);
		if (ret) {
			hp_err("%s dts acquire failed\n", __func__);
			return ret;
		}
	} else {
		sih_haptic->chip_attr.reset_gpio = -1;
		sih_haptic->chip_attr.irq_gpio = -1;
	}

	return 0;
}

static void sih_hardware_reset(
	sih_haptic_t *sih_haptic)
{
	/* fpga version not equal chip version */
	if (gpio_is_valid(sih_haptic->chip_attr.reset_gpio)) {
		gpio_set_value(sih_haptic->chip_attr.reset_gpio,
			SIH_HAPTIC_RESET_GPIO_UNINVALID); // 0

		usleep_range(SIH_HAPTIC_RESET_START_TIME,
			SIH_HAPTIC_RESET_END_TIME);

		gpio_set_value(sih_haptic->chip_attr.reset_gpio,
			SIH_HAPTIC_RESET_GPIO_INVALID); // 1

		usleep_range(SIH_HAPTIC_PWR_AWIT_START_TIME,
			SIH_HAPTIC_PWR_AWIT_END_TIME);
	}
}

static int sih_gpio_request_res(
	struct device *dev,
	sih_haptic_t *sih_haptic)
{
	int ret = -1;

	if (gpio_is_valid(sih_haptic->chip_attr.reset_gpio)) {
		ret = devm_gpio_request_one(dev,
			sih_haptic->chip_attr.reset_gpio,
			GPIOF_OUT_INIT_LOW, "sih_haptic_rst");
		if (ret) {
			hp_err("%s: reset gpio request failed\n", __func__);
			return ret;
		}
	}

#ifdef SIH_ENABLE_PIN_CONTROL
	sih_haptic->pinctrl = devm_pinctrl_get(dev);
	if (IS_ERR(sih_haptic->pinctrl)) {
		hp_err("%s Target does not use pinctrl\n", __func__);
		sih_haptic->pinctrl = NULL;
	} else {
		sih_haptic->pinctrl_state_active =
			pinctrl_lookup_state(sih_haptic->pinctrl,
			"sih688x_gpio");
		if (IS_ERR(sih_haptic->pinctrl_state_active)) {
			hp_err("%s cannot find irq_active\n", __func__);
			sih_haptic->pinctrl = NULL;
			sih_haptic->pinctrl_state_active = NULL;
		} else {
			pinctrl_select_state(sih_haptic->pinctrl,
				sih_haptic->pinctrl_state_active);
		}
	}
#endif

	if (gpio_is_valid(sih_haptic->chip_attr.irq_gpio)) {
		ret = devm_gpio_request_one(dev,
			sih_haptic->chip_attr.irq_gpio,
			GPIOF_DIR_IN, "sih_haptic_irq");
		if (ret) {
			devm_gpio_free(dev, sih_haptic->chip_attr.reset_gpio);
			hp_err("%s: irq gpio request failed\n", __func__);
			return ret;
		}
	}

	sih_hardware_reset(sih_haptic);

	return ret;
}

static void sih_change_play_priority(
	sih_haptic_t *sih_haptic,
	sih_broadcast_priority_e play_priority)
{
#ifdef SIH_PRIORITY_OPEN
	sih_haptic->chip_ipara.cur_pri = play_priority;
#endif
}

static void sih_chip_state_recovery(
	sih_haptic_t *sih_haptic)
{
	sih_haptic->chip_ipara.state = SIH_HAPTIC_STANDBY_MODE;
	sih_haptic->chip_ipara.play_mode = SIH_HAPTIC_IDLE_MODE;
	sih_haptic->chip_ipara.action_mode = SIH_HAPTIC_IDLE_MODE;
}

static void sih_rtp_data_play_prehandle(
	sih_haptic_t *sih_haptic)
{
	rtp_data_cont =
		vmalloc(SIH_RTP_DATA_CONT_LEN + sizeof(int));
	if (!rtp_data_cont)
		hp_err("%s: rtp_cont malloc failed\n", __func__);

	rtp_data_cont->len = SIH_RTP_DATA_CONT_LEN;
}

static bool sih_irq_rtp_local_file_handle(sih_haptic_t *sih_haptic, haptic_container_t *rtp_cont)
{
	uint32_t buf_len = 0;
	int ret = 0;
	int cont_len = 0;

	mutex_lock(&sih_haptic->rtp_para.rtp_lock);

	if (!sih_haptic->rtp_para.rtp_cnt) {
		hp_err("%s: rtp_cnt is 0!\n", __func__);
		mutex_unlock(&sih_haptic->rtp_para.rtp_lock);
		sih_change_play_priority(sih_haptic, SIH_BROADCAST_PRI_MIN);
		return false;
	}

	if (!rtp_cont) {
		hp_err("%s: rtp_container is null, break!\n", __func__);
		mutex_unlock(&sih_haptic->rtp_para.rtp_lock);
		sih_change_play_priority(sih_haptic, SIH_BROADCAST_PRI_MIN);
		return false;
	}

	hp_info("%s: rtp_cont->len = %d\n", __func__, rtp_cont->len);

	cont_len = rtp_cont->len;

	if ((cont_len - sih_haptic->rtp_para.rtp_cnt) < (sih_haptic->ram_para.base_addr >> 2))
		buf_len = cont_len - sih_haptic->rtp_para.rtp_cnt;
	else
		buf_len = (sih_haptic->ram_para.base_addr >> 2);

	ret = sih_haptic->hp_func->write_rtp_data(sih_haptic, &rtp_cont->data[sih_haptic->rtp_para.rtp_cnt], buf_len);
	if (ret < 0) {
		sih_haptic->hp_func->play_stop(sih_haptic);
		sih_haptic->hp_func->set_rtp_aei(sih_haptic, false);
		sih_haptic->rtp_para.rtp_init = false;
		sih_change_play_priority(sih_haptic, SIH_BROADCAST_PRI_MIN);
		mutex_unlock(&sih_haptic->rtp_para.rtp_lock);
		hp_err("%s: i2c write rtp data failed\n", __func__);
		return false;
	}

	sih_haptic->rtp_para.rtp_cnt += buf_len;

	hp_info("%s: rtp_para.rtp_cnt %d\n", __func__, sih_haptic->rtp_para.rtp_cnt);

	cont_len = rtp_cont->len;

	if ((sih_haptic->rtp_para.rtp_cnt == cont_len) ||
		sih_haptic->hp_func->if_is_mode(sih_haptic, SIH_HAPTIC_IDLE_MODE)) {
		if (sih_haptic->rtp_para.rtp_cnt != cont_len)
			hp_err("%s: rtp play error suspend!\n", __func__);
		else
			hp_info("%s: rtp update complete!\n", __func__);

		sih_haptic->hp_func->set_rtp_aei(sih_haptic, false);
		sih_haptic->rtp_para.rtp_init = false;
		sih_chip_state_recovery(sih_haptic);
		sih_change_play_priority(sih_haptic, SIH_BROADCAST_PRI_MIN);
		mutex_unlock(&sih_haptic->rtp_para.rtp_lock);
		return false;
	}
	mutex_unlock(&sih_haptic->rtp_para.rtp_lock);

	return true;
}

static irqreturn_t sih_irq_isr(
	int irq,
	void *data)
{
	sih_haptic_t *sih_haptic = data;
	haptic_container_t *rtp_cont = sih_haptic->rtp_para.rtp_cont;

	hp_info("%s enter! interrupt code number is %d\n", __func__, irq);

	if (!sih_haptic->hp_func->get_interrupt_state(sih_haptic) &&
		sih_haptic->rtp_para.rtp_init) {
		while ((sih_haptic->hp_func->if_rtp_fifo_full(sih_haptic) == 0) &&
			(sih_haptic->chip_ipara.play_mode == SIH_HAPTIC_RTP_MODE)) {
			if (!sih_irq_rtp_local_file_handle(sih_haptic, rtp_cont))
				break;
		}
	} else {
		if (!sih_haptic->rtp_para.rtp_init)
			hp_err("%s: rtp init error\n", __func__);
	}

	if (sih_haptic->chip_ipara.play_mode != SIH_HAPTIC_RTP_MODE)
		sih_haptic->hp_func->set_rtp_aei(sih_haptic, false);

	/* detect */
	if ((sih_haptic->chip_ipara.play_mode != SIH_HAPTIC_IDLE_MODE) &&
		((sih_haptic->detect_para.trig_detect_en) |
		(sih_haptic->detect_para.ram_detect_en) |
		(sih_haptic->detect_para.rtp_detect_en) |
		(sih_haptic->detect_para.cont_detect_en)))
			/* detect read fifo */
			sih_haptic->hp_func->read_detect_fifo(sih_haptic);
	hp_info("%s: exit\n", __func__);
	return IRQ_HANDLED;
}

static int sih_gpio_interrupt_config(
	struct device *dev,
	sih_haptic_t *sih_haptic)
{
	int ret = -1;
	int irq_flags = 0;

	/* configure sih6887 chip interrupt register */
	sih_haptic->hp_func->interrupt_state_init(sih_haptic);

	irq_flags = IRQF_TRIGGER_FALLING | IRQF_ONESHOT;

	ret = devm_request_threaded_irq(dev,
		gpio_to_irq(sih_haptic->chip_attr.irq_gpio), NULL,
		sih_irq_isr, irq_flags,
		sih_haptic->soft_frame.vib_dev.name, sih_haptic);
	if (ret != 0) {
		hp_err("%s: irq gpio interrupt request failed\n", __func__);
		return ret;
	}

	return 0;
}

static void sih_vfree_container(
	sih_haptic_t *sih_haptic,
	haptic_container_t *cont)
{
	if (sih_haptic->rtp_para.rtp_pre_mode == SIH_RTP_LOCAL_PLAY)
		vfree(cont);
	else
		sih_haptic->rtp_para.rtp_cont = NULL;
}

static void sih_rtp_play(sih_haptic_t *sih_haptic)
{
	uint32_t buf_len = 0;
	haptic_container_t *rtp_cont = sih_haptic->rtp_para.rtp_cont;
	int cont_len = 0;

	hp_info("%s enter!\n%s: the rtp_cont len is %d\n", __func__, __func__, rtp_cont->len);

#ifdef TIMED_OUTPUT
	pm_qos_add_request(&sih_haptic->pm_qos, PM_QOS_CPU_DMA_LATENCY, SIH_PM_QOS_VALUE_VB);
#endif
	sih_haptic->rtp_para.rtp_cnt = 0;
	mutex_lock(&sih_haptic->rtp_para.rtp_lock);

	while ((!sih_haptic->hp_func->if_rtp_fifo_full(sih_haptic)) &&
		(sih_haptic->chip_ipara.play_mode == SIH_HAPTIC_RTP_MODE)) {
		cont_len = rtp_cont->len;

		if (!rtp_cont) {
			hp_err("%s:container is null, break!\n", __func__);
			sih_change_play_priority(sih_haptic, SIH_BROADCAST_PRI_MIN);
			sih_chip_state_recovery(sih_haptic);
			break;
		}

		if (sih_haptic->rtp_para.rtp_cnt < (sih_haptic->ram_para.base_addr)) {
			if ((cont_len - sih_haptic->rtp_para.rtp_cnt) < (sih_haptic->ram_para.base_addr))
				buf_len = cont_len - sih_haptic->rtp_para.rtp_cnt;
			else
				buf_len = sih_haptic->ram_para.base_addr;
		} else if ((cont_len - sih_haptic->rtp_para.rtp_cnt) < (sih_haptic->ram_para.base_addr >> 2)) {
			buf_len = cont_len - sih_haptic->rtp_para.rtp_cnt;
		} else {
			buf_len = sih_haptic->ram_para.base_addr >> 2;
		}

		hp_info("%s: buf_len = %d, rtp cnt = %d\n", __func__, buf_len, sih_haptic->rtp_para.rtp_cnt);

		sih_haptic->hp_func->write_rtp_data(sih_haptic, &rtp_cont->data[sih_haptic->rtp_para.rtp_cnt], buf_len);

		sih_haptic->rtp_para.rtp_cnt += buf_len;

		if ((sih_haptic->rtp_para.rtp_cnt == cont_len) ||
			sih_haptic->hp_func->if_is_mode(sih_haptic, SIH_HAPTIC_IDLE_MODE)) {
			if (sih_haptic->rtp_para.rtp_cnt != cont_len)
				hp_err("%s: rtp suspend!\n", __func__);
			else
				hp_info("%s: rtp update complete!\n", __func__);
			sih_chip_state_recovery(sih_haptic);
			sih_change_play_priority(sih_haptic, SIH_BROADCAST_PRI_MIN);
			break;
		}
	}
	if (sih_haptic->chip_ipara.play_mode == SIH_HAPTIC_RTP_MODE) {
		hp_info("%s: open rtp empty\n", __func__);
		sih_haptic->hp_func->set_rtp_aei(sih_haptic, true);
	}
	mutex_unlock(&sih_haptic->rtp_para.rtp_lock);
#ifdef TIMED_OUTPUT
	pm_qos_remove_request(&sih_haptic->pm_qos);
#endif
}

static void sih_osc_rtp_play_func(
	sih_haptic_t *sih_haptic)
{
	uint32_t buf_len = 0;
	haptic_container_t *rtp_cont = sih_haptic->rtp_para.rtp_cont;
	int cont_len = 0;
	sih_haptic->osc_para.timeval_flag = 1;

	hp_info("%s enter!\n", __func__);
	sih_haptic->rtp_para.rtp_cnt = 0;

	while (1) {
		if (!sih_haptic->hp_func->if_rtp_fifo_full(sih_haptic)) {
			cont_len = rtp_cont->len;

			if (sih_haptic->rtp_para.rtp_cnt < (sih_haptic->ram_para.base_addr)) {
				if ((cont_len - sih_haptic->rtp_para.rtp_cnt) < (sih_haptic->ram_para.base_addr))
					buf_len = cont_len - sih_haptic->rtp_para.rtp_cnt;
				else
					buf_len = sih_haptic->ram_para.base_addr;
			} else if ((cont_len - sih_haptic->rtp_para.rtp_cnt) < (sih_haptic->ram_para.base_addr >> 2)) {
				buf_len = cont_len - sih_haptic->rtp_para.rtp_cnt;
			} else {
				buf_len = sih_haptic->ram_para.base_addr >> 2;
			}

			if (sih_haptic->rtp_para.rtp_cnt != cont_len) {
				if (sih_haptic->osc_para.timeval_flag == 1) {
					sih_haptic->osc_para.kstart = ktime_get();
					sih_haptic->osc_para.timeval_flag = 0;
				}
				sih_haptic->hp_func->write_rtp_data(sih_haptic,
					&rtp_cont->data[sih_haptic->rtp_para.rtp_cnt], buf_len);

				sih_haptic->rtp_para.rtp_cnt += buf_len;
			}
		}

		if (sih_haptic->hp_func->if_is_mode(sih_haptic, SIH_HAPTIC_IDLE_MODE)) {
			sih_haptic->osc_para.kend = ktime_get();
			hp_err("%s: rtp_cnt:%d cont_cnt:%d\n", __func__, sih_haptic->rtp_para.rtp_cnt, cont_len);
			sih_chip_state_recovery(sih_haptic);
			sih_change_play_priority(sih_haptic, SIH_BROADCAST_PRI_MIN);
			break;
		}
		sih_haptic->osc_para.kend = ktime_get();
		sih_haptic->osc_para.microsecond =
			ktime_to_us(ktime_sub(sih_haptic->osc_para.kend, sih_haptic->osc_para.kstart));
		if (sih_haptic->osc_para.microsecond > 6000000) {
			hp_err("%s:time out osc_rtp_cnt:%d\n", __func__, sih_haptic->rtp_para.rtp_cnt);
			break;
		}
	}
}

static void sih_osc_rtp_play(
	sih_haptic_t *sih_haptic)
{
	hp_info("%s enter!\n", __func__);

#ifdef TIMED_OUTPUT
	pm_qos_add_request(&sih_haptic->pm_qos,
		PM_QOS_CPU_DMA_LATENCY, SIH_PM_QOS_VALUE_VB);
#endif
	mutex_lock(&sih_haptic->rtp_para.rtp_lock);

	sih_osc_rtp_play_func(sih_haptic);

	mutex_unlock(&sih_haptic->rtp_para.rtp_lock);

#ifdef TIMED_OUTPUT
	pm_qos_remove_request(&sih_haptic->pm_qos);
#endif

	sih_haptic->osc_para.microsecond =
		ktime_to_us(ktime_sub(sih_haptic->osc_para.kend, sih_haptic->osc_para.kstart));
	hp_err("%s: microsecond:%d\n", __func__, sih_haptic->osc_para.microsecond);
}

static void sih_rtp_osc_calibration_func(
	sih_haptic_t *sih_haptic)
{
	bool rtp_work_flag = false;
	int cnt = 200;

	while (cnt) {
		if (sih_haptic->hp_func->if_is_mode(sih_haptic, SIH_HAPTIC_RTP_MODE)) {
			cnt = 0;
			rtp_work_flag = true;
			hp_err("%s: rtp_go!\n", __func__);
		} else {
			cnt--;
			hp_err("%s: wait for rtp_go!\n", __func__);
		}
		usleep_range(2000, 2500);
	}
	if (rtp_work_flag) {
		sih_osc_rtp_play(sih_haptic);
	} else {
		/* enter standby mode */
		sih_haptic->hp_func->play_stop(sih_haptic);
		sih_chip_state_recovery(sih_haptic);
		sih_change_play_priority(sih_haptic, SIH_BROADCAST_PRI_MIN);
		hp_err("%s: failed to enter rtp_go status!\n", __func__);
	}
}

static void sih_rtp_osc_calibration(
	sih_haptic_t *sih_haptic)
{
	// bool rtp_work_flag = false;
	// int cnt = 200;
	int ret = -1;
	const struct firmware *rtp_file;

	hp_info("%s enter!\n", __func__);

	mutex_lock(&sih_haptic->rtp_para.rtp_lock);

	sih_haptic->rtp_para.rtp_init = false;
	sih_vfree_container(sih_haptic, sih_haptic->rtp_para.rtp_cont);
	sih_haptic->rtp_para.rtp_pre_mode = SIH_RTP_LOCAL_PLAY;

	ret = request_firmware(&rtp_file, sih_rtp_name[0], sih_haptic->dev);
	if (ret < 0) {
		hp_err("%s: failed to read %s\n", __func__, sih_rtp_name[0]);
		sih_chip_state_recovery(sih_haptic);
		sih_change_play_priority(sih_haptic, SIH_BROADCAST_PRI_MIN);
		mutex_unlock(&sih_haptic->rtp_para.rtp_lock);
		return;
	}

	sih_haptic->rtp_para.rtp_cont = vmalloc(rtp_file->size + sizeof(int));
	if (!sih_haptic->rtp_para.rtp_cont) {
		release_firmware(rtp_file);
		hp_err("%s: error allocating memory\n", __func__);
		sih_chip_state_recovery(sih_haptic);
		sih_change_play_priority(sih_haptic, SIH_BROADCAST_PRI_MIN);
		mutex_unlock(&sih_haptic->rtp_para.rtp_lock);
		return;
	}
	sih_haptic->rtp_para.rtp_cont->len = rtp_file->size;
	sih_haptic->osc_para.osc_rtp_len = rtp_file->size;
	mutex_unlock(&sih_haptic->rtp_para.rtp_lock);
	memcpy(sih_haptic->rtp_para.rtp_cont->data, rtp_file->data, rtp_file->size);

	release_firmware(rtp_file);

	hp_info("%s: rtp data len is %d\n", __func__, sih_haptic->rtp_para.rtp_cont->len);

	sih_haptic->rtp_para.rtp_init = true;
	sih_haptic->hp_func->upload_lra(sih_haptic, SIH_OSC_CALI_LRA);
	sih_haptic->hp_func->set_rtp_aei(sih_haptic, false);
	sih_haptic->hp_func->clear_interrupt_state(sih_haptic);
	sih_haptic->hp_func->play_stop(sih_haptic);

	/* rtp mode config */
	sih_haptic->hp_func->set_play_mode(sih_haptic, SIH_HAPTIC_RTP_MODE);

	/* osc init */
	sih_haptic->hp_func->trim_init(sih_haptic);

	/* disable irq */
	disable_irq(gpio_to_irq(sih_haptic->chip_attr.irq_gpio));

	/* haptic go */
	sih_haptic->hp_func->play_go(sih_haptic, true);
	usleep_range(2000, 2500);

	sih_rtp_osc_calibration_func(sih_haptic);

	/* enable irq */
	enable_irq(gpio_to_irq(sih_haptic->chip_attr.irq_gpio));
}

static void reset_valid_amp(sih_haptic_t *sih_haptic)
{
	if (sih_haptic->set_amplitude_flag != 1)
		return;
	sih_haptic->chip_ipara.gain = long_amp_again_val[THIRD_LONG_HAPTIC_AMP];
	sih_haptic->set_amplitude_flag = 0;
}


static int sih_acquire_prepare_res(
	struct device *dev,
	sih_haptic_t *sih_haptic)
{
	int ret = -1;

	ret = sih_gpio_request_res(dev, sih_haptic);

	return ret;
}

static int sih_acquire_irq_res(
	struct device *dev,
	sih_haptic_t *sih_haptic)
{
	int ret = 0;

	ret = sih_gpio_interrupt_config(dev, sih_haptic);

	return ret;
}

static int get_ram_num(
	sih_haptic_t *sih_haptic)
{
	uint8_t wave_addr[2] = {0};
	uint32_t first_wave_addr = 0;

	hp_info("%s: enter\n", __func__);
	if (!sih_haptic->ram_para.ram_init) {
		hp_err("%s: ram init failed, ram_num = 0!\n", __func__);
		return -EPERM;
	}

	mutex_lock(&sih_haptic->lock);
	/* RAMINIT Enable */
	sih_haptic->hp_func->ram_init(sih_haptic, true);
	sih_haptic->hp_func->play_stop(sih_haptic);
	sih_haptic->hp_func->set_ram_addr(sih_haptic);
	sih_haptic->hp_func->get_first_wave_addr(sih_haptic, wave_addr);
	first_wave_addr = (wave_addr[0] << 8 | wave_addr[1]);
	sih_haptic->ram_para.ram_num =
		(first_wave_addr - sih_haptic->ram_para.base_addr - 1) / 4;

	hp_info("%s: first wave addr = 0x%04x\n", __func__, first_wave_addr);
	hp_info("%s: ram_num = %d\n", __func__, sih_haptic->ram_para.ram_num);

	/* RAMINIT Disable */
	sih_haptic->hp_func->ram_init(sih_haptic, false);
	mutex_unlock(&sih_haptic->lock);

	return 0;
}

static int sih_ram_load_func(
	const struct firmware *cont,
	void *context)
{
	sih_haptic_t *sih_haptic = context;

#ifdef SIH_READ_BINFILE_FLEXBALLY
	static uint8_t load_cont;
	int ram_timer_val = 1000;

	load_cont++;
#endif

	hp_info("%s: enter\n", __func__);

	if (cont) {
		load_cont = 0;
		return 0;
	} else {
		hp_err("%s: failed to read %s\n", __func__,
			sih_ram_name[0]);
		release_firmware(cont);

#ifdef SIH_READ_BINFILE_FLEXBALLY
		if (load_cont <= 2) {
			schedule_delayed_work(
			&sih_haptic->ram_para.ram_delayed_work,
			msecs_to_jiffies(ram_timer_val));
			hp_err("%s:start hrtimer : load_cont%d\n",
				__func__, load_cont);
		}
#endif
		return -1;
	}
}
static void sih_ram_load(
	const struct firmware *cont,
	void *context)
{
	uint16_t check_sum = 0;
	int i;
	int ret = 0;
	sih_haptic_t *sih_haptic = context;
	haptic_container_t *sih_haptic_fw;

	ret = sih_ram_load_func(cont, context);
	if (ret < 0)
		return;

	hp_info("%s: loaded %s - size: %zu\n", __func__,
		sih_ram_name[0],
		cont ? cont->size : 0);

	/* check sum */
	for (i = 2; i < cont->size; i++)
		check_sum += cont->data[i];
	if (check_sum !=
		(uint16_t)((cont->data[0] << 8) |
		(cont->data[1]))) {
		hp_err("%s: check sum err: check_sum=0x%04x\n",
			__func__, check_sum);
		release_firmware(cont);
		return;
	}

	hp_info("%s: check sum pass : 0x%04x\n", __func__, check_sum);

	sih_haptic->ram_para.check_sum = check_sum;

	/* ram update */
	sih_haptic_fw = kzalloc(cont->size + sizeof(int), GFP_KERNEL);
	if (!sih_haptic_fw) {
		release_firmware(cont);
		hp_err("%s: Error allocating memory\n", __func__);
		return;
	}

	sih_haptic_fw->len = cont->size;
	memcpy(sih_haptic_fw->data, cont->data, cont->size);
	release_firmware(cont);
	ret = sih_haptic->hp_func->update_ram_config(sih_haptic, sih_haptic_fw);
	if (ret) {
		hp_err("%s: ram firmware update failed!\n", __func__);
	} else {
		sih_haptic->ram_para.ram_init = true;
		sih_haptic->ram_para.len =
			sih_haptic_fw->len - sih_haptic->ram_para.ram_shift;

		hp_info("%s: ram firmware update complete!\n", __func__);
		get_ram_num(sih_haptic);
	}
	kfree(sih_haptic_fw);
}

static int sih_ram_update(
	sih_haptic_t *sih_haptic)
{
	int len = 0;

	hp_info("%s:enter!\n", __func__);

	sih_haptic->ram_para.ram_init = false;

	len = request_firmware_nowait(THIS_MODULE, FW_ACTION_HOTPLUG,
		sih_ram_name[0],
		sih_haptic->dev, GFP_KERNEL,
		sih_haptic, sih_ram_load);

	return len;
}

static void sih_ram_play(
	sih_haptic_t *sih_haptic,
	uint8_t mode)
{
	hp_info("%s: enter\n", __func__);
	sih_haptic->hp_func->set_play_mode(sih_haptic, mode);
	sih_haptic->hp_func->play_go(sih_haptic, true);
}

#ifdef SIH_PRIORITY_OPEN
static bool __maybe_unused sih_priority_judge(
	sih_haptic_t *sih_haptic,
	sih_broadcast_priority_e desc_prio)
{
	if (!sih_judge_priority(sih_haptic, desc_prio))
		return false;
	sih_haptic->chip_ipara.cur_pri = desc_prio;

	return true;
}
#endif

/*****************************************************
 *
 * vibrator sysfs node
 *
 *****************************************************/

static ssize_t seq_show(
	struct device *dev,
	struct device_attribute *attr,
	char *buf)
{
	ssize_t len = 0;
	cdev_t *cdev = NULL;
	sih_haptic_t *sih_haptic = NULL;
	uint8_t i;

	if (!buf || !dev) {
		hp_err("%s: buf or dev is null!\n", __func__);
		return -EINVAL;
	}
	cdev = dev_get_drvdata(dev);
	if (!cdev) {
		hp_err("%s: cdev is null!\n", __func__);
		return -EINVAL;
	}
	sih_haptic = container_of(cdev, sih_haptic_t, soft_frame.vib_dev);
	if (!sih_haptic) {
		hp_err("%s: sih_haptic is null!\n", __func__);
		return -EINVAL;
	}

	sih_haptic->hp_func->get_wav_seq(sih_haptic, SIH_HAPTIC_SEQUENCER_SIZE);
	for (i = 0; i < SIH_HAPTIC_SEQUENCER_SIZE; i++)
		len += snprintf(buf + len, PAGE_SIZE - len,
			"seq%d = %d\n", i + 1, sih_haptic->ram_para.seq[i]);

	return len;
}

static ssize_t seq_store(
	struct device *dev,
	struct device_attribute *attr,
	const char *buf,
	size_t count)
{
	cdev_t *cdev = NULL;
	sih_haptic_t *sih_haptic = NULL;
	uint32_t databuf[2] = { 0, 0 };

	if (!buf || !dev) {
		hp_err("%s: buf or dev is null!\n", __func__);
		return -EINVAL;
	}
	cdev = dev_get_drvdata(dev);
	if (!cdev) {
		hp_err("%s: cdev is null!\n", __func__);
		return -EINVAL;
	}
	sih_haptic = container_of(cdev, sih_haptic_t, soft_frame.vib_dev);
	if (!sih_haptic) {
		hp_err("%s: sih_haptic is null!\n", __func__);
		return -EINVAL;
	}

	if (sscanf(buf, "%x %x", &databuf[0], &databuf[1]) == 2) {
		if (databuf[0] >= SIH_HAPTIC_SEQUENCER_SIZE) {
			hp_err("%s: input value out of range!\n", __func__);
			return count;
		}
		hp_info("%s: seq%d=0x%02x\n", __func__, databuf[0], databuf[1]);
		mutex_lock(&sih_haptic->lock);
		sih_haptic->ram_para.seq[databuf[0]] = (uint8_t)databuf[1];
		sih_haptic->hp_func->set_wav_seq(sih_haptic,
			(uint8_t)(databuf[0]),
			sih_haptic->ram_para.seq[databuf[0]]);
		mutex_unlock(&sih_haptic->lock);
	}

	return count;
}

static ssize_t reg_show(
	struct device *dev,
	struct device_attribute *attr,
	char *buf)
{
	ssize_t len = 0;
	cdev_t *cdev = NULL;
	sih_haptic_t *sih_haptic = NULL;

	if (!buf || !dev) {
		hp_err("%s: buf or dev is null!\n", __func__);
		return -EINVAL;
	}
	cdev = dev_get_drvdata(dev);
	if (!cdev) {
		hp_err("%s: cdev is null!\n", __func__);
		return -EINVAL;
	}
	sih_haptic = container_of(cdev, sih_haptic_t, soft_frame.vib_dev);
	if (!sih_haptic) {
		hp_err("%s: sih_haptic is null!\n", __func__);
		return -EINVAL;
	}

	len = sih_haptic->hp_func->get_all_reg(sih_haptic, len, buf);

	return len;
}

static ssize_t reg_store(
	struct device *dev,
	struct device_attribute *attr,
	const char *buf,
	size_t count)
{
	cdev_t *cdev = NULL;
	sih_haptic_t *sih_haptic = NULL;
	uint8_t val = 0;
	uint32_t databuf[2] = { 0, 0 };

	if (!buf || !dev) {
		hp_err("%s: buf or dev is null!\n", __func__);
		return -EINVAL;
	}
	cdev = dev_get_drvdata(dev);
	if (!cdev) {
		hp_err("%s: cdev is null!\n", __func__);
		return -EINVAL;
	}
	sih_haptic = container_of(cdev, sih_haptic_t, soft_frame.vib_dev);
	if (!sih_haptic) {
		hp_err("%s: sih_haptic is null!\n", __func__);
		return -EINVAL;
	}

	if (sscanf(buf, "%x %x", &databuf[0], &databuf[1]) == 2) {
		val = (uint8_t)databuf[1];
		haptic_regmap_write(sih_haptic->regmapp.regmapping,
			(uint8_t)databuf[0], SIH_I2C_OPERA_BYTE_ONE, &val);
	}

	return count;
}

static ssize_t state_show(
	struct device *dev,
	struct device_attribute *attr,
	char *buf)
{
	ssize_t len = 0;
	cdev_t *cdev = NULL;
	sih_haptic_t *sih_haptic = NULL;

	if (!buf || !dev) {
		hp_err("%s: buf or dev is null!\n", __func__);
		return -EINVAL;
	}
	cdev = dev_get_drvdata(dev);
	if (!cdev) {
		hp_err("%s: cdev is null!\n", __func__);
		return -EINVAL;
	}
	sih_haptic = container_of(cdev, sih_haptic_t, soft_frame.vib_dev);
	if (!sih_haptic) {
		hp_err("%s: sih_haptic is null!\n", __func__);
		return -EINVAL;
	}

	sih_haptic->hp_func->update_chip_state(sih_haptic);

	len = snprintf(buf, PAGE_SIZE, "state = %d\n",
				sih_haptic->chip_ipara.state);
	len += snprintf(buf + len, PAGE_SIZE - len, "play_mode = %d\n",
				sih_haptic->chip_ipara.play_mode);

	return len;
}

static ssize_t state_store(
	struct device *dev,
	struct device_attribute *attr,
	const char *buf,
	size_t count)
{
	cdev_t *cdev = NULL;
	sih_haptic_t *sih_haptic = NULL;
	uint32_t val = 0;
	int ret = 0;

	if (!buf || !dev) {
		hp_err("%s: buf or dev is null!\n", __func__);
		return -EINVAL;
	}
	cdev = dev_get_drvdata(dev);
	if (!cdev) {
		hp_err("%s: cdev is null!\n", __func__);
		return -EINVAL;
	}
	sih_haptic = container_of(cdev, sih_haptic_t, soft_frame.vib_dev);
	if (!sih_haptic) {
		hp_err("%s: sih_haptic is null!\n", __func__);
		return -EINVAL;
	}

	ret = kstrtouint(buf, 0, &val);
	if (ret < 0)
		return ret;

	if (SIH_HAPTIC_STANDBY_MODE == val) {
		sih_haptic->hp_func->play_stop(sih_haptic);
		sih_haptic->chip_ipara.state = SIH_HAPTIC_STANDBY_MODE;
		sih_haptic->chip_ipara.play_mode = SIH_HAPTIC_IDLE_MODE;
		sih_change_play_priority(sih_haptic, SIH_BROADCAST_PRI_MIN);
	}

	return count;
}

static ssize_t gain_show(
	struct device *dev,
	struct device_attribute *attr,
	char *buf)
{
	ssize_t len = 0;
	cdev_t *cdev = NULL;
	sih_haptic_t *sih_haptic = NULL;

	if (!buf || !dev) {
		hp_err("%s: buf or dev is null!\n", __func__);
		return -EINVAL;
	}
	cdev = dev_get_drvdata(dev);
	if (!cdev) {
		hp_err("%s: cdev is null!\n", __func__);
		return -EINVAL;
	}
	sih_haptic = container_of(cdev, sih_haptic_t, soft_frame.vib_dev);
	if (!sih_haptic) {
		hp_err("%s: sih_haptic is null!\n", __func__);
		return -EINVAL;
	}

	len = snprintf(buf, PAGE_SIZE, "gain = 0x%02X\n",
		sih_haptic->chip_ipara.gain);

	return len;
}

static ssize_t gain_store(
	struct device *dev,
	struct device_attribute *attr,
	const char *buf,
	size_t count)
{
	cdev_t *cdev = NULL;
	sih_haptic_t *sih_haptic = NULL;
	uint32_t val = 0;
	int rc = 0;

	if (!buf || !dev) {
		hp_err("%s: buf or dev is null!\n", __func__);
		return -EINVAL;
	}
	cdev = dev_get_drvdata(dev);
	if (!cdev) {
		hp_err("%s: cdev is null!\n", __func__);
		return -EINVAL;
	}
	sih_haptic = container_of(cdev, sih_haptic_t, soft_frame.vib_dev);
	if (!sih_haptic) {
		hp_err("%s: sih_haptic is null!\n", __func__);
		return -EINVAL;
	}

	rc = kstrtouint(buf, 0, &val);
	if (rc < 0)
		return rc;

	if (val > SIH_HAPTIC_MAX_GAIN) {
		hp_err("%s: gain out of range!\n", __func__);
		return count;
	}

	if (val > SIH_HAPTIC_GAIN_LIMIT) {
		hp_err("%s: gain out of limit!\n", __func__);
		val = SIH_HAPTIC_GAIN_LIMIT;
	}

	hp_info("%s: value = 0x%02x\n", __func__, val);

	mutex_lock(&sih_haptic->lock);
	sih_haptic->chip_ipara.gain = val;
	sih_haptic->hp_func->set_gain(sih_haptic, sih_haptic->chip_ipara.gain);
	mutex_unlock(&sih_haptic->lock);

	return count;
}

static ssize_t rtp_show(
	struct device *dev,
	struct device_attribute *attr,
	char *buf)
{
	ssize_t len = 0;
	cdev_t *cdev = NULL;
	sih_haptic_t *sih_haptic = NULL;

	if (!buf || !dev) {
		hp_err("%s: buf or dev is null!\n", __func__);
		return -EINVAL;
	}
	cdev = dev_get_drvdata(dev);
	if (!cdev) {
		hp_err("%s: cdev is null!\n", __func__);
		return -EINVAL;
	}
	sih_haptic = container_of(cdev, sih_haptic_t, soft_frame.vib_dev);
	if (!sih_haptic) {
		hp_err("%s: sih_haptic is null!\n", __func__);
		return -EINVAL;
	}

	len += snprintf(buf + len, PAGE_SIZE - len, "rtp_cnt = %d\n",
		sih_haptic->rtp_para.rtp_cnt);

	return len;
}

static ssize_t rtp_store(
	struct device *dev,
	struct device_attribute *attr,
	const char *buf,
	size_t count)
{
	cdev_t *cdev = NULL;
	sih_haptic_t *sih_haptic = NULL;
	uint32_t val = 0;
	uint32_t rtp_num_max = sizeof(sih_rtp_name) / SIH_RTP_NAME_MAX;
	int rc = 0;

	if (!buf || !dev) {
		hp_err("%s: buf or dev is null!\n", __func__);
		return -EINVAL;
	}
	cdev = dev_get_drvdata(dev);
	if (!cdev) {
		hp_err("%s: cdev is null!\n", __func__);
		return -EINVAL;
	}
	sih_haptic = container_of(cdev, sih_haptic_t, soft_frame.vib_dev);
	if (!sih_haptic) {
		hp_err("%s: sih_haptic is null!\n", __func__);
		return -EINVAL;
	}

	rc = kstrtouint(buf, 0, &val);
	if (rc < 0) {
		hp_err("%s: kstrouint fail\n", __func__);
		return rc;
	}

	hp_info("%s:input number :%d\n", __func__, val);

	mutex_lock(&sih_haptic->lock);

#ifdef SIH_PRIORITY_OPEN
	if (!sih_priority_judge(sih_haptic, sih_haptic->register_pri.rtp)) {
		mutex_unlock(&sih_haptic->lock);
		return count;
	}
#endif

	hp_info("%s pass priority, rtp_num_max = %d\n", __func__, rtp_num_max);

	sih_haptic->hp_func->play_stop(sih_haptic);
	sih_haptic->hp_func->set_rtp_aei(sih_haptic, false);
	sih_haptic->hp_func->clear_interrupt_state(sih_haptic);
	if ((val > 0) && (val <= rtp_num_max)) {
		sih_haptic->rtp_para.rtp_file_num = val - 1;
		hp_info("%s: sih_rtp_name[%d]: %s\n", __func__,
			val, sih_rtp_name[val - 1]);
		sih_haptic->chip_ipara.state = SIH_HAPTIC_ACTIVE_MODE;
		sih_haptic->chip_ipara.action_mode = SIH_HAPTIC_RTP_MODE;
		schedule_work(&sih_haptic->rtp_para.rtp_work);
	} else {
		hp_err("%s: input number err:%d\n", __func__, val);
	}
	mutex_unlock(&sih_haptic->lock);

	return count;
}

static ssize_t cali_show(
	struct device *dev,
	struct device_attribute *attr,
	char *buf)
{
	cdev_t *cdev = NULL;
	sih_haptic_t *sih_haptic = NULL;
	ssize_t len = 0;
	uint32_t cali_rst;

	if (!buf || !dev) {
		hp_err("%s: buf or dev is null!\n", __func__);
		return -EINVAL;
	}
	cdev = dev_get_drvdata(dev);
	if (!cdev) {
		hp_err("%s: cdev is null!\n", __func__);
		return -EINVAL;
	}
	sih_haptic = container_of(cdev, sih_haptic_t, soft_frame.vib_dev);
	if (!sih_haptic) {
		hp_err("%s: sih_haptic is null!\n", __func__);
		return -EINVAL;
	}

	if (sih_haptic->detect_para.tracking_f0 >= SIH_F0_MAX_THRESHOLD ||
		sih_haptic->detect_para.tracking_f0 <= SIH_F0_MIN_THRESHOLD)
		cali_rst = 0;
	else
		cali_rst = 1;

	len += snprintf(buf + len, PAGE_SIZE - len, "%d", cali_rst);

	return len;
}

static ssize_t cali_store(
	struct device *dev,
	struct device_attribute *attr,
	const char *buf,
	size_t count)
{
	cdev_t *cdev = NULL;
	sih_haptic_t *sih_haptic = NULL;
	uint32_t val = 0;
	int rc = 0;
	int i;

	if (!buf || !dev) {
		hp_err("%s: buf or dev is null!\n", __func__);
		return -EINVAL;
	}
	cdev = dev_get_drvdata(dev);
	if (!cdev) {
		hp_err("%s: cdev is null!\n", __func__);
		return -EINVAL;
	}
	sih_haptic = container_of(cdev, sih_haptic_t, soft_frame.vib_dev);
	if (!sih_haptic) {
		hp_err("%s: sih_haptic is null!\n", __func__);
		return -EINVAL;
	}

	rc = kstrtouint(buf, 0, &val);
	if (rc < 0)
		return rc;

	if (val) {
		mutex_lock(&sih_haptic->lock);
		for (i = 0; i < SIH_F0_DETECT_TRY; i++) {
			sih_haptic->hp_func->get_tracking_f0(sih_haptic);
			msleep(200);
			if (sih_haptic->detect_para.tracking_f0 <= SIH_F0_MAX_THRESHOLD &&
				sih_haptic->detect_para.tracking_f0 >= SIH_F0_MIN_THRESHOLD) {
				break;
			}
		}
		mutex_unlock(&sih_haptic->lock);
	}

	return count;
}

static ssize_t auto_pvdd_show(
	struct device *dev,
	struct device_attribute *attr,
	char *buf)
{
	ssize_t len = 0;
	cdev_t *cdev = NULL;
	sih_haptic_t *sih_haptic = NULL;

	if (!buf || !dev) {
		hp_err("%s: buf or dev is null!\n", __func__);
		return -EINVAL;
	}
	cdev = dev_get_drvdata(dev);
	if (!cdev) {
		hp_err("%s: cdev is null!\n", __func__);
		return -EINVAL;
	}
	sih_haptic = container_of(cdev, sih_haptic_t, soft_frame.vib_dev);
	if (!sih_haptic) {
		hp_err("%s: sih_haptic is null!\n", __func__);
		return -EINVAL;
	}

	len += snprintf(buf + len, PAGE_SIZE - len, "auto_pvdd = %d\n",
		sih_haptic->chip_ipara.auto_pvdd_en);

	return len;
}

static ssize_t auto_pvdd_store(
	struct device *dev,
	struct device_attribute *attr,
	const char *buf,
	size_t count)
{
	cdev_t *cdev = NULL;
	sih_haptic_t *sih_haptic = NULL;
	bool val = 0;
	int rc = 0;

	if (!buf || !dev) {
		hp_err("%s: buf or dev is null!\n", __func__);
		return -EINVAL;
	}
	cdev = dev_get_drvdata(dev);
	if (!cdev) {
		hp_err("%s: cdev is null!\n", __func__);
		return -EINVAL;
	}
	sih_haptic = container_of(cdev, sih_haptic_t, soft_frame.vib_dev);
	if (!sih_haptic) {
		hp_err("%s: sih_haptic is null!\n", __func__);
		return -EINVAL;
	}

	rc = strtobool(buf, &val);
	if (rc < 0)
		return rc;

	mutex_lock(&sih_haptic->lock);
	sih_haptic->hp_func->play_stop(sih_haptic);
	sih_haptic->hp_func->set_auto_pvdd(sih_haptic, val);
	mutex_unlock(&sih_haptic->lock);

	return count;
}

static ssize_t duration_show(
	struct device *dev,
	struct device_attribute *attr,
	char *buf)
{
	ssize_t len = 0;
	cdev_t *cdev = NULL;
	sih_haptic_t *sih_haptic = NULL;
	ktime_t time_rem;
	s64 time_ms = 0;

	if (!buf || !dev) {
		hp_err("%s: buf or dev is null!\n", __func__);
		return -EINVAL;
	}
	cdev = dev_get_drvdata(dev);
	if (!cdev) {
		hp_err("%s: cdev is null!\n", __func__);
		return -EINVAL;
	}
	sih_haptic = container_of(cdev, sih_haptic_t, soft_frame.vib_dev);
	if (!sih_haptic) {
		hp_err("%s: sih_haptic is null!\n", __func__);
		return -EINVAL;
	}

	if (hrtimer_active(&sih_haptic->timer)) {
		time_rem = hrtimer_get_remaining(&sih_haptic->timer);
		time_ms = ktime_to_ms(time_rem);
	}

	len = snprintf(buf, PAGE_SIZE, "duration = %lldms\n", time_ms);

	return len;
}

static ssize_t duration_store(
	struct device *dev,
	struct device_attribute *attr,
	const char *buf,
	size_t count)
{
	cdev_t *cdev = NULL;
	sih_haptic_t *sih_haptic = NULL;
	uint32_t val = 0;
	int rc = 0;

	if (!buf || !dev) {
		hp_err("%s: buf or dev is null!\n", __func__);
		return -EINVAL;
	}
	cdev = dev_get_drvdata(dev);
	if (!cdev) {
		hp_err("%s: cdev is null!\n", __func__);
		return -EINVAL;
	}
	sih_haptic = container_of(cdev, sih_haptic_t, soft_frame.vib_dev);
	if (!sih_haptic) {
		hp_err("%s: sih_haptic is null!\n", __func__);
		return -EINVAL;
	}

	rc = kstrtouint(buf, 0, &val);
	if (rc < 0)
		return rc;

	/* duration should not be set to 0 */
	if (val <= 0) {
		hp_err("%s: duration out of range!\n", __func__);
		return count;
	}
	sih_haptic->duration = val;
	sih_haptic->cust_boost_on = 0;
	__pm_wakeup_event(sih_haptic->ws, sih_haptic->duration + HAPTIC_WAKE_LOCK_GAP);
	return count;
}

static ssize_t osc_cali_show(
	struct device *dev,
	struct device_attribute *attr,
	char *buf)
{
	ssize_t len = 0;
	cdev_t *cdev = NULL;
	sih_haptic_t *sih_haptic = NULL;

	if (!buf || !dev) {
		hp_err("%s: buf or dev is null!\n", __func__);
		return -EINVAL;
	}
	cdev = dev_get_drvdata(dev);
	if (!cdev) {
		hp_err("%s: cdev is null!\n", __func__);
		return -EINVAL;
	}
	sih_haptic = container_of(cdev, sih_haptic_t, soft_frame.vib_dev);
	if (!sih_haptic) {
		hp_err("%s: sih_haptic is null!\n", __func__);
		return -EINVAL;
	}

	len += snprintf(buf + len, PAGE_SIZE - len, "actual = %d theory = %d\n",
		sih_haptic->osc_para.microsecond, sih_haptic->osc_para.theory_time);

	return len;
}

static ssize_t osc_cali_store(
	struct device *dev,
	struct device_attribute *attr,
	const char *buf,
	size_t count)
{
	cdev_t *cdev = NULL;
	sih_haptic_t *sih_haptic = NULL;

	uint32_t val = 0;
	int rc = 0;
	if (!buf || !dev) {
		hp_err("%s: buf or dev is null!\n", __func__);
		return -EINVAL;
	}
	cdev = dev_get_drvdata(dev);
	if (!cdev) {
		hp_err("%s: cdev is null!\n", __func__);
		return -EINVAL;
	}
	sih_haptic = container_of(cdev, sih_haptic_t, soft_frame.vib_dev);
	if (!sih_haptic) {
		hp_err("%s: sih_haptic is null!\n", __func__);
		return -EINVAL;
	}

	rc = kstrtouint(buf, 0, &val);
	if (rc < 0)
		return rc;

	if (val <= 0)
		return count;

	mutex_lock(&sih_haptic->lock);
	sih_haptic->chip_ipara.state = SIH_HAPTIC_ACTIVE_MODE;
	sih_haptic->chip_ipara.action_mode = SIH_HAPTIC_RTP_MODE;
	sih_rtp_osc_calibration(sih_haptic);
	sih_haptic->hp_func->osc_trim_calibration(sih_haptic);
	mutex_unlock(&sih_haptic->lock);

	return count;
}

static ssize_t index_show(
	struct device *dev,
	struct device_attribute *attr,
	char *buf)
{
	ssize_t len = 0;
	cdev_t *cdev = NULL;
	sih_haptic_t *sih_haptic = NULL;

	if (!buf || !dev) {
		hp_err("%s: buf or dev is null!\n", __func__);
		return -EINVAL;
	}
	cdev = dev_get_drvdata(dev);
	if (!cdev) {
		hp_err("%s: cdev is null!\n", __func__);
		return -EINVAL;
	}
	sih_haptic = container_of(cdev, sih_haptic_t, soft_frame.vib_dev);
	if (!sih_haptic) {
		hp_err("%s: sih_haptic is null!\n", __func__);
		return -EINVAL;
	}

	sih_haptic->hp_func->get_wav_seq(sih_haptic, 1);
	sih_haptic->ram_para.index = sih_haptic->ram_para.seq[0];
	len += snprintf(buf, PAGE_SIZE, "index = %d\n",
		sih_haptic->ram_para.index);

	return len;
}

static ssize_t index_store(
	struct device *dev,
	struct device_attribute *attr,
	const char *buf,
	size_t count)
{
	cdev_t *cdev = NULL;
	sih_haptic_t *sih_haptic = NULL;
	uint32_t val = 0;
	int rc = 0;

	if (!buf || !dev) {
		hp_err("%s: buf or dev is null!\n", __func__);
		return -EINVAL;
	}
	cdev = dev_get_drvdata(dev);
	if (!cdev) {
		hp_err("%s: cdev is null!\n", __func__);
		return -EINVAL;
	}
	sih_haptic = container_of(cdev, sih_haptic_t, soft_frame.vib_dev);
	if (!sih_haptic) {
		hp_err("%s: sih_haptic is null!\n", __func__);
		return -EINVAL;
	}

	rc = kstrtouint(buf, 0, &val);
	if (rc < 0)
		return rc;

	if (val > sih_haptic->ram_para.ram_num) {
		hp_err("%s: input value out of range!\n", __func__);
		return count;
	}

	hp_info("%s: value = %d\n", __func__, val);

	mutex_lock(&sih_haptic->lock);
	sih_haptic->ram_para.index = val;
	sih_haptic->hp_func->set_repeat_seq(sih_haptic,
		sih_haptic->ram_para.index);
	mutex_unlock(&sih_haptic->lock);

	return count;
}

static ssize_t ram_num_show(
	struct device *dev,
	struct device_attribute *attr,
	char *buf)
{
	ssize_t len = 0;
	cdev_t *cdev = NULL;
	sih_haptic_t *sih_haptic = NULL;

	if (!buf || !dev) {
		hp_err("%s: buf or dev is null!\n", __func__);
		return -EINVAL;
	}
	cdev = dev_get_drvdata(dev);
	if (!cdev) {
		hp_err("%s: cdev is null!\n", __func__);
		return -EINVAL;
	}
	sih_haptic = container_of(cdev, sih_haptic_t, soft_frame.vib_dev);
	if (!sih_haptic) {
		hp_err("%s: sih_haptic is null!\n", __func__);
		return -EINVAL;
	}

	get_ram_num(sih_haptic);
	len += snprintf(buf + len, PAGE_SIZE - len, "ram_num = %d\n",
		sih_haptic->ram_para.ram_num);

	return len;
}

static ssize_t loop_show(
	struct device *dev,
	struct device_attribute *attr,
	char *buf)
{
	ssize_t len = 0;
	cdev_t *cdev = NULL;
	sih_haptic_t *sih_haptic = NULL;

	if (!buf || !dev) {
		hp_err("%s: buf or dev is null!\n", __func__);
		return -EINVAL;
	}
	cdev = dev_get_drvdata(dev);
	if (!cdev) {
		hp_err("%s: cdev is null!\n", __func__);
		return -EINVAL;
	}
	sih_haptic = container_of(cdev, sih_haptic_t, soft_frame.vib_dev);
	if (!sih_haptic) {
		hp_err("%s: sih_haptic is null!\n", __func__);
		return -EINVAL;
	}

	len = sih_haptic->hp_func->get_wav_internal_loop(sih_haptic, buf);

	return len;
}

static ssize_t loop_store(
	struct device *dev,
	struct device_attribute *attr,
	const char *buf,
	size_t count)
{
	cdev_t *cdev = NULL;
	sih_haptic_t *sih_haptic = NULL;
	uint32_t databuf[2] = { 0, 0 };

	if (!buf || !dev) {
		hp_err("%s: buf or dev is null!\n", __func__);
		return -EINVAL;
	}
	cdev = dev_get_drvdata(dev);
	if (!cdev) {
		hp_err("%s: cdev is null!\n", __func__);
		return -EINVAL;
	}
	sih_haptic = container_of(cdev, sih_haptic_t, soft_frame.vib_dev);
	if (!sih_haptic) {
		hp_err("%s: sih_haptic is null!\n", __func__);
		return -EINVAL;
	}

	if (sscanf(buf, "%x %x", &databuf[0], &databuf[1]) == 2) {
		if ((databuf[0] >= SIH_HAPTIC_SEQUENCER_SIZE) ||
			(databuf[1] > SIH_HAPTIC_REG_SEQLOOP_MAX)) {
			hp_err("%s: input value out of range!\n", __func__);
			return count;
		}
		hp_err("%s: databuf[0] = 0x%02x, databuf[1] = 0x%02x\n",
				__func__, databuf[0], databuf[1]);
		mutex_lock(&sih_haptic->lock);
		sih_haptic->ram_para.loop[databuf[0]] = (uint8_t)databuf[1];
		sih_haptic->hp_func->set_wav_internal_loop(sih_haptic,
			(uint8_t)(databuf[0]),
			sih_haptic->ram_para.loop[databuf[0]]);
		mutex_unlock(&sih_haptic->lock);
	}

	return count;
}

static ssize_t ram_update_show(
	struct device *dev,
	struct device_attribute *attr,
	char *buf)
{
	ssize_t len = 0;
	cdev_t *cdev = NULL;
	sih_haptic_t *sih_haptic = NULL;

	if (!buf || !dev) {
		hp_err("%s: buf or dev is null!\n", __func__);
		return -EINVAL;
	}
	cdev = dev_get_drvdata(dev);
	if (!cdev) {
		hp_err("%s: cdev is null!\n", __func__);
		return -EINVAL;
	}
	sih_haptic = container_of(cdev, sih_haptic_t, soft_frame.vib_dev);
	if (!sih_haptic) {
		hp_err("%s: sih_haptic is null!\n", __func__);
		return -EINVAL;
	}

	/* RAMINIT Enable */
	sih_haptic->hp_func->play_stop(sih_haptic);
	sih_haptic->hp_func->ram_init(sih_haptic, true);
	sih_haptic->hp_func->set_ram_addr(sih_haptic);
	len += snprintf(buf + len, PAGE_SIZE - len, "sih_haptic_ram:\n");
	len += sih_haptic->hp_func->get_ram_data(sih_haptic, buf);

	/* RANINIT Disable */
	sih_haptic->hp_func->ram_init(sih_haptic, false);

	return len;
}

static ssize_t ram_update_store(
	struct device *dev,
	struct device_attribute *attr,
	const char *buf,
	size_t count)
{
	cdev_t *cdev = NULL;
	sih_haptic_t *sih_haptic = NULL;
	uint32_t val = 0;
	int rc = 0;

	if (!buf || !dev) {
		hp_err("%s: buf or dev is null!\n", __func__);
		return -EINVAL;
	}
	cdev = dev_get_drvdata(dev);
	if (!cdev) {
		hp_err("%s: cdev is null!\n", __func__);
		return -EINVAL;
	}
	sih_haptic = container_of(cdev, sih_haptic_t, soft_frame.vib_dev);
	if (!sih_haptic) {
		hp_err("%s: sih_haptic is null!\n", __func__);
		return -EINVAL;
	}

	rc = kstrtouint(buf, 0, &val);
	if (rc < 0)
		return rc;

	hp_info("%s: ram update input is %d\n", __func__, val);

	if (val)
		sih_ram_update(sih_haptic);

	return count;
}

static ssize_t ram_vbat_comp_show(
	struct device *dev,
	struct device_attribute *attr,
	char *buf)
{
	ssize_t len = 0;
	cdev_t *cdev = NULL;
	sih_haptic_t *sih_haptic = NULL;

	if (!buf || !dev) {
		hp_err("%s: buf or dev is null!\n", __func__);
		return -EINVAL;
	}
	cdev = dev_get_drvdata(dev);
	if (!cdev) {
		hp_err("%s: cdev is null!\n", __func__);
		return -EINVAL;
	}
	sih_haptic = container_of(cdev, sih_haptic_t, soft_frame.vib_dev);
	if (!sih_haptic) {
		hp_err("%s: sih_haptic is null!\n", __func__);
		return -EINVAL;
	}

	len += snprintf(buf + len, PAGE_SIZE - len,
		"ram_vbat_comp = %d\n", sih_haptic->ram_para.ram_vbat_comp);

	return len;
}

static ssize_t ram_vbat_comp_store(
	struct device *dev,
	struct device_attribute *attr,
	const char *buf,
	size_t count)
{
	cdev_t *cdev = NULL;
	sih_haptic_t *sih_haptic = NULL;
	uint32_t val = 0;
	int rc = 0;

	if (!buf || !dev) {
		hp_err("%s: buf or dev is null!\n", __func__);
		return -EINVAL;
	}
	cdev = dev_get_drvdata(dev);
	if (!cdev) {
		hp_err("%s: cdev is null!\n", __func__);
		return -EINVAL;
	}
	sih_haptic = container_of(cdev, sih_haptic_t, soft_frame.vib_dev);
	if (!sih_haptic) {
		hp_err("%s: sih_haptic is null!\n", __func__);
		return -EINVAL;
	}

	rc = kstrtouint(buf, 0, &val);
	if (rc < 0)
		return rc;

	mutex_lock(&sih_haptic->lock);

	if (val)
		sih_haptic->ram_para.ram_vbat_comp = SIH_RAM_VBAT_COMP_ENABLE;
	else
		sih_haptic->ram_para.ram_vbat_comp = SIH_RAM_VBAT_COMP_DISABLE;

	mutex_unlock(&sih_haptic->lock);

	return count;
}

static ssize_t cont_show(
	struct device *dev,
	struct device_attribute *attr,
	char *buf)
{
	ssize_t len = 0;
	cdev_t *cdev = NULL;
	sih_haptic_t *sih_haptic = NULL;

	if (!buf || !dev) {
		hp_err("%s: buf or dev is null!\n", __func__);
		return -EINVAL;
	}
	cdev = dev_get_drvdata(dev);
	if (!cdev) {
		hp_err("%s: cdev is null!\n", __func__);
		return -EINVAL;
	}
	sih_haptic = container_of(cdev, sih_haptic_t, soft_frame.vib_dev);
	if (!sih_haptic) {
		hp_err("%s: sih_haptic is null!\n", __func__);
		return -EINVAL;
	}

	len += snprintf(buf + len, PAGE_SIZE - len, "cont_f0 = %d\n",
				sih_haptic->detect_para.tracking_f0);

	return len;
}

static ssize_t cont_store(
	struct device *dev,
	struct device_attribute *attr,
	const char *buf,
	size_t count)
{
	cdev_t *cdev = NULL;
	sih_haptic_t *sih_haptic = NULL;
	uint32_t val = 0;
	uint32_t t_half_num = 0xff;
	uint32_t cont_index = 0;
	int rc = 0;

	hp_info("%s : enter\n", __func__);
	if (!buf || !dev) {
		hp_err("%s: buf or dev is null!\n", __func__);
		return -EINVAL;
	}
	cdev = dev_get_drvdata(dev);
	if (!cdev) {
		hp_err("%s: cdev is null!\n", __func__);
		return -EINVAL;
	}
	sih_haptic = container_of(cdev, sih_haptic_t, soft_frame.vib_dev);
	if (!sih_haptic) {
		hp_err("%s: sih_haptic is null!\n", __func__);
		return -EINVAL;
	}

	rc = kstrtouint(buf, 0, &val);
	if (rc < 0)
		return rc;

	sih_haptic->hp_func->play_stop(sih_haptic);
	if (val) {
		cont_index = 5;
		sih_haptic->hp_func->update_cont_config(sih_haptic, cont_index, t_half_num);
		cont_index = 7;
		sih_haptic->hp_func->update_cont_config(sih_haptic, cont_index, val);
	}

	return count;
}

static ssize_t ram_store(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
	cdev_t *cdev = NULL;
	sih_haptic_t *sih_haptic = NULL;
	uint32_t databuf[2] = {0};

	hp_info("%s : enter\n", __func__);
	if (!buf || !dev) {
		hp_err("%s: buf or dev is null!\n", __func__);
		return -EINVAL;
	}
	cdev = dev_get_drvdata(dev);
	if (!cdev) {
		hp_err("%s: cdev is null!\n", __func__);
		return -EINVAL;
	}
	sih_haptic = container_of(cdev, sih_haptic_t, soft_frame.vib_dev);
	if (!sih_haptic) {
		hp_err("%s: sih_haptic is null!\n", __func__);
		return -EINVAL;
	}

	if (sscanf(buf, "%x %x", &databuf[0], &databuf[1]) != 2) {
		hp_err("%s: input parameter error\n", __func__);
		return count;
	}

	if (!sih_haptic->ram_para.ram_init) {
		hp_err("%s: ram init failed, not allow to play!\n", __func__);
		return count;
	}

	mutex_lock(&sih_haptic->lock);

	/* RAM MODE */
	if (databuf[0] == 1) {
#ifdef SIH_PRIORITY_OPEN
		hp_info("%s:pri.ram = %d, cur_pri = %d\n", __func__, sih_haptic->register_pri.ram,
			sih_haptic->chip_ipara.cur_pri);
		if (!sih_priority_judge(sih_haptic, sih_haptic->register_pri.ram)) {
			mutex_unlock(&sih_haptic->lock);
			hp_err("%s:sih_priority_judge return\n", __func__);
			return count;
		}
#endif
		sih_haptic->hp_func->play_stop(sih_haptic);
		sih_haptic->chip_ipara.action_mode = SIH_HAPTIC_RAM_MODE;
	/* LOOPRAM MODE */
	} else if (databuf[0] == 2) {
#ifdef SIH_PRIORITY_OPEN
		if (!sih_priority_judge(sih_haptic, sih_haptic->register_pri.loopram)) {
			mutex_unlock(&sih_haptic->lock);
			hp_err("%s:pri.ram = %d, cur_pri = %d\n", __func__, sih_haptic->register_pri.loopram,
				sih_haptic->chip_ipara.cur_pri);
			return count;
		}
#endif
		sih_haptic->hp_func->play_stop(sih_haptic);
		sih_haptic->chip_ipara.action_mode = SIH_HAPTIC_RAM_LOOP_MODE;
	} else {
		mutex_unlock(&sih_haptic->lock);
		hp_err("%s mode parameter error\n", __func__);
		return count;
	}

	if (databuf[1] == 1) {
		sih_haptic->chip_ipara.state = SIH_HAPTIC_ACTIVE_MODE;
	} else {
		sih_haptic->hp_func->play_stop(sih_haptic);
		sih_chip_state_recovery(sih_haptic);
		mutex_unlock(&sih_haptic->lock);
		sih_change_play_priority(sih_haptic, SIH_BROADCAST_PRI_MIN);
		return count;
	}

	if (hrtimer_active(&sih_haptic->timer))
		hrtimer_cancel(&sih_haptic->timer);

	mutex_unlock(&sih_haptic->lock);
	schedule_work(&sih_haptic->ram_work);

	return count;
}

static ssize_t lra_resistance_show(
	struct device *dev,
	struct device_attribute *attr,
	char *buf)
{
	ssize_t len = 0;
	cdev_t *cdev = NULL;
	sih_haptic_t *sih_haptic = NULL;

	if (!buf || !dev) {
		hp_err("%s: buf or dev is null!\n", __func__);
		return -EINVAL;
	}
	cdev = dev_get_drvdata(dev);
	if (!cdev) {
		hp_err("%s: cdev is null!\n", __func__);
		return -EINVAL;
	}
	sih_haptic = container_of(cdev, sih_haptic_t, soft_frame.vib_dev);
	if (!sih_haptic) {
		hp_err("%s: sih_haptic is null!\n", __func__);
		return -EINVAL;
	}

	mutex_lock(&sih_haptic->lock);
	sih_haptic->hp_func->get_lra_resistance(sih_haptic);
	mutex_unlock(&sih_haptic->lock);

	len += snprintf(buf + len, PAGE_SIZE - len, "rl = %d\n",
				sih_haptic->detect_para.detect_rl);
	return len;
}

static ssize_t lra_resistance_store(
	struct device *dev,
	struct device_attribute *attr,
	const char *buf,
	size_t count)
{
	uint32_t val = 0;
	int ret = 0;
	cdev_t *cdev = NULL;
	sih_haptic_t *sih_haptic = NULL;

	if (!buf || !dev) {
		hp_err("%s: buf or dev is null!\n", __func__);
		return -EINVAL;
	}
	cdev = dev_get_drvdata(dev);
	if (!cdev) {
		hp_err("%s: cdev is null!\n", __func__);
		return -EINVAL;
	}
	sih_haptic = container_of(cdev, sih_haptic_t, soft_frame.vib_dev);
	if (!sih_haptic) {
		hp_err("%s: sih_haptic is null!\n", __func__);
		return -EINVAL;
	}

	ret = kstrtoint(buf, 0, &val);
	if (ret < 0)
		return ret;

	sih_haptic->detect_para.detect_rl_offset = val;

	return count;
}

static ssize_t trig_para_show(
	struct device *dev,
	struct device_attribute *attr,
	char *buf)
{
	ssize_t len = 0;
	cdev_t *cdev = NULL;
	sih_haptic_t *sih_haptic = NULL;

	if (!buf || !dev) {
		hp_err("%s: buf or dev is null!\n", __func__);
		return -EINVAL;
	}
	cdev = dev_get_drvdata(dev);
	if (!cdev) {
		hp_err("%s: cdev is null!\n", __func__);
		return -EINVAL;
	}
	sih_haptic = container_of(cdev, sih_haptic_t, soft_frame.vib_dev);
	if (!sih_haptic) {
		hp_err("%s: sih_haptic is null!\n", __func__);
		return -EINVAL;
	}

	len += sih_haptic->hp_func->get_trig_para(sih_haptic, buf);

	return len;
}

static ssize_t trig_para_store(
	struct device *dev,
	struct device_attribute *attr,
	const char *buf,
	size_t count)
{
	cdev_t *cdev = NULL;
	sih_haptic_t *sih_haptic = NULL;
	uint32_t databuf[7] = { 0, 0, 0, 0, 0, 0, 0};

	if (!buf || !dev) {
		hp_err("%s: buf or dev is null!\n", __func__);
		return -EINVAL;
	}
	cdev = dev_get_drvdata(dev);
	if (!cdev) {
		hp_err("%s: cdev is null!\n", __func__);
		return -EINVAL;
	}
	sih_haptic = container_of(cdev, sih_haptic_t, soft_frame.vib_dev);
	if (!sih_haptic) {
		hp_err("%s: sih_haptic is null!\n", __func__);
		return -EINVAL;
	}

	if (sscanf(buf, "%x %x %x %x %x %x %x",
		&databuf[0], &databuf[1], &databuf[2], &databuf[3],
		&databuf[4], &databuf[5], &databuf[6]) == 7) {
		mutex_lock(&sih_haptic->lock);
		sih_haptic->hp_func->set_trig_para(sih_haptic, databuf);
		mutex_unlock(&sih_haptic->lock);
	}

	return count;
}

static ssize_t low_power_show(
	struct device *dev,
	struct device_attribute *attr,
	char *buf)
{
	cdev_t *cdev = NULL;
	sih_haptic_t *sih_haptic = NULL;
	ssize_t len = 0;

	if (!buf || !dev) {
		hp_err("%s: buf or dev is null!\n", __func__);
		return -EINVAL;
	}
	cdev = dev_get_drvdata(dev);
	if (!cdev) {
		hp_err("%s: cdev is null!\n", __func__);
		return -EINVAL;
	}
	sih_haptic = container_of(cdev, sih_haptic_t, soft_frame.vib_dev);
	if (!sih_haptic) {
		hp_err("%s: sih_haptic is null!\n", __func__);
		return -EINVAL;
	}

	len += snprintf(buf + len, PAGE_SIZE - len, "low_power_mode = %d\n",
		(uint8_t)sih_haptic->chip_ipara.low_power);

	return len;
}

static ssize_t low_power_store(
	struct device *dev,
	struct device_attribute *attr,
	const char *buf,
	size_t count)
{
	cdev_t *cdev = NULL;
	sih_haptic_t *sih_haptic = NULL;
	uint32_t val = 0;
	int ret = 0;

	if (!buf || !dev) {
		hp_err("%s: buf or dev is null!\n", __func__);
		return -EINVAL;
	}
	cdev = dev_get_drvdata(dev);
	if (!cdev) {
		hp_err("%s: cdev is null!\n", __func__);
		return -EINVAL;
	}
	sih_haptic = container_of(cdev, sih_haptic_t, soft_frame.vib_dev);
	if (!sih_haptic) {
		hp_err("%s: sih_haptic is null!\n", __func__);
		return -EINVAL;
	}

	ret = kstrtouint(buf, 0, &val);
	if (ret < 0)
		return ret;

	mutex_lock(&sih_haptic->lock);
	sih_haptic->hp_func->set_low_power_mode(sih_haptic, val);
	mutex_unlock(&sih_haptic->lock);

	return count;
}

static ssize_t activate_show(
	struct device *dev,
	struct device_attribute *attr,
	char *buf)
{
	cdev_t *cdev = NULL;
	sih_haptic_t *sih_haptic = NULL;
	ssize_t len = 0;

	if (!buf || !dev) {
		hp_err("%s: buf or dev is null!\n", __func__);
		return -EINVAL;
	}
	cdev = dev_get_drvdata(dev);
	if (!cdev) {
		hp_err("%s: cdev is null!\n", __func__);
		return -EINVAL;
	}
	sih_haptic = container_of(cdev, sih_haptic_t, soft_frame.vib_dev);
	if (!sih_haptic) {
		hp_err("%s: sih_haptic is null!\n", __func__);
		return -EINVAL;
	}

	len += snprintf(buf, PAGE_SIZE, "activate = %d\n",
		sih_haptic->chip_ipara.state);

	return len;
}

static ssize_t activate_store(
	struct device *dev,
	struct device_attribute *attr,
	const char *buf,
	size_t count)
{
	cdev_t *cdev = NULL;
	sih_haptic_t *sih_haptic = NULL;
	uint32_t val = 0;
	int rc = 0;

	if (!buf || !dev) {
		hp_err("%s: buf or dev is null!\n", __func__);
		return -EINVAL;
	}
	cdev = dev_get_drvdata(dev);
	if (!cdev) {
		hp_err("%s: cdev is null!\n", __func__);
		return -EINVAL;
	}
	sih_haptic = container_of(cdev, sih_haptic_t, soft_frame.vib_dev);
	if (!sih_haptic) {
		hp_err("%s: sih_haptic is null!\n", __func__);
		return -EINVAL;
	}

	rc = kstrtouint(buf, 0, &val);
	if (rc < 0)
		return rc;

	if (val < 0)
		return count;

	hp_info("%s: value = %d\n", __func__, val);

	if (!sih_haptic->ram_para.ram_init) {
		hp_err("%s: ram init failed\n", __func__);
		return count;
	}
	mutex_lock(&sih_haptic->lock);
#ifdef SIH_PRIORITY_OPEN
		hp_info("%s:sih_haptic->register_pri.ram = %d current priority = %d\n",
			__func__, sih_haptic->register_pri.ram,
			sih_haptic->chip_ipara.cur_pri);
		if (!sih_priority_judge(sih_haptic,
			sih_haptic->register_pri.ram)) {
			mutex_unlock(&sih_haptic->lock);
			hp_err("%s:sih_priority_judge return\n", __func__);
			return count;
		}
#endif
	if (hrtimer_active(&sih_haptic->timer))
		hrtimer_cancel(&sih_haptic->timer);
	sih_haptic->chip_ipara.state = val;
	sih_haptic->chip_ipara.action_mode = SIH_HAPTIC_RAM_MODE;
	sih_haptic->effect_mode = LONG_VIB_RAM_MODE;
	sih_haptic->index = LONG_VIB_BOOST_EFFECT_ID;
	sih_haptic->chip_ipara.gain = long_amp_again_val[MAX_LONG_HAPTIC_AMP];
	reset_valid_amp(sih_haptic);

	mutex_unlock(&sih_haptic->lock);
	schedule_work(&sih_haptic->vibrator_work);

	return count;
}

static ssize_t vmax_show(
	struct device *dev,
	struct device_attribute *attr,
	char *buf)
{
	cdev_t *cdev = NULL;
	sih_haptic_t *sih_haptic = NULL;
	ssize_t len = 0;

	if (!buf || !dev) {
		hp_err("%s: buf or dev is null!\n", __func__);
		return -EINVAL;
	}
	cdev = dev_get_drvdata(dev);
	if (!cdev) {
		hp_err("%s: cdev is null!\n", __func__);
		return -EINVAL;
	}
	sih_haptic = container_of(cdev, sih_haptic_t, soft_frame.vib_dev);
	if (!sih_haptic) {
		hp_err("%s: sih_haptic is null!\n", __func__);
		return -EINVAL;
	}

	len += snprintf(buf, PAGE_SIZE, "drv_vboost = %d\n",
		sih_haptic->chip_ipara.drv_vboost);

	return len;
}

static ssize_t vmax_store(
	struct device *dev,
	struct device_attribute *attr,
	const char *buf,
	size_t count)
{
	cdev_t *cdev = NULL;
	sih_haptic_t *sih_haptic = NULL;

	uint32_t val = 0;
	int rc = 0;

	if (!buf || !dev) {
		hp_err("%s: buf or dev is null!\n", __func__);
		return -EINVAL;
	}
	cdev = dev_get_drvdata(dev);
	if (!cdev) {
		hp_err("%s: cdev is null!\n", __func__);
		return -EINVAL;
	}
	sih_haptic = container_of(cdev, sih_haptic_t, soft_frame.vib_dev);
	if (!sih_haptic) {
		hp_err("%s: sih_haptic is null!\n", __func__);
		return -EINVAL;
	}

	rc = kstrtouint(buf, 0, &val);
	if (rc < 0)
		return rc;

	hp_info("%s: value=%d\n", __func__, val);

	mutex_lock(&sih_haptic->lock);
	sih_haptic->chip_ipara.drv_vboost = val * SIH_6887_DRV_VBOOST_COEFFICIENT;
	if (val < SIH_6887_DRV_VBOOST_MIN) {
		hp_info("%s: drv_vboost is too low,set to 60:%d",
			__func__, val);
		sih_haptic->chip_ipara.drv_vboost = SIH_6887_DRV_VBOOST_MIN
			* SIH_6887_DRV_VBOOST_COEFFICIENT;
	} else if (val > SIH_6887_DRV_VBOOST_MAX) {
		hp_info("%s: drv_vboost is too high,set to 110:%d",
			__func__, val);
		sih_haptic->chip_ipara.drv_vboost = SIH_6887_DRV_VBOOST_MAX
			* SIH_6887_DRV_VBOOST_COEFFICIENT;
	}
	sih_haptic->hp_func->set_drv_bst_vol(sih_haptic,
		sih_haptic->chip_ipara.drv_vboost);
	mutex_unlock(&sih_haptic->lock);

	return count;
}

static ssize_t vbat_monitor_show(
	struct device *dev,
	struct device_attribute *attr,
	char *buf)
{
	cdev_t *cdev = NULL;
	sih_haptic_t *sih_haptic = NULL;
	ssize_t len = 0;

	if (!buf || !dev) {
		hp_err("%s: buf or dev is null!\n", __func__);
		return -EINVAL;
	}
	cdev = dev_get_drvdata(dev);
	if (!cdev) {
		hp_err("%s: cdev is null!\n", __func__);
		return -EINVAL;
	}
	sih_haptic = container_of(cdev, sih_haptic_t, soft_frame.vib_dev);
	if (!sih_haptic) {
		hp_err("%s: sih_haptic is null!\n", __func__);
		return -EINVAL;
	}

	mutex_lock(&sih_haptic->lock);
	sih_haptic->hp_func->get_vbat(sih_haptic);
	len += snprintf(buf + len, PAGE_SIZE - len, "vbat_monitor = %d\n",
		sih_haptic->detect_para.detect_vbat);
	mutex_unlock(&sih_haptic->lock);

	return len;
}

static ssize_t main_loop_show(
	struct device *dev,
	struct device_attribute *attr,
	char *buf)
{
	ssize_t len = 0;
	cdev_t *cdev = NULL;
	sih_haptic_t *sih_haptic = NULL;

	if (!buf || !dev) {
		hp_err("%s: buf or dev is null!\n", __func__);
		return -EINVAL;
	}
	cdev = dev_get_drvdata(dev);
	if (!cdev) {
		hp_err("%s: cdev is null!\n", __func__);
		return -EINVAL;
	}
	sih_haptic = container_of(cdev, sih_haptic_t, soft_frame.vib_dev);
	if (!sih_haptic) {
		hp_err("%s: sih_haptic is null!\n", __func__);
		return -EINVAL;
	}

	len = sih_haptic->hp_func->get_wav_external_loop(sih_haptic, buf);

	return len;
}

static ssize_t main_loop_store(
	struct device *dev,
	struct device_attribute *attr,
	const char *buf,
	size_t count)
{
	cdev_t *cdev = NULL;
	sih_haptic_t *sih_haptic = NULL;
	uint8_t val = 0;
	int rc = 0;

	if (!buf || !dev) {
		hp_err("%s: buf or dev is null!\n", __func__);
		return -EINVAL;
	}
	cdev = dev_get_drvdata(dev);
	if (!cdev) {
		hp_err("%s: cdev is null!\n", __func__);
		return -EINVAL;
	}
	sih_haptic = container_of(cdev, sih_haptic_t, soft_frame.vib_dev);
	if (!sih_haptic) {
		hp_err("%s: sih_haptic is null!\n", __func__);
		return -EINVAL;
	}

	rc = kstrtou8(buf, 0, &val);
	if (rc < 0)
		return rc;

	if (val > SIH_RAM_MAIN_LOOP_MAX_TIME) {
		hp_err("%s: input value out of range!\n", __func__);
		return count;
	}

	hp_info("%s: main_loop = %d\n", __func__, val);

	mutex_lock(&sih_haptic->lock);
	sih_haptic->ram_para.external_loop = val;
	sih_haptic->hp_func->set_wav_external_loop(sih_haptic, val);
	mutex_unlock(&sih_haptic->lock);

	return count;
}

static ssize_t seq_gain_show(
	struct device *dev,
	struct device_attribute *attr,
	char *buf)
{
	ssize_t len = 0;
	cdev_t *cdev = NULL;
	sih_haptic_t *sih_haptic = NULL;

	if (!buf || !dev) {
		hp_err("%s: buf or dev is null!\n", __func__);
		return -EINVAL;
	}
	cdev = dev_get_drvdata(dev);
	if (!cdev) {
		hp_err("%s: cdev is null!\n", __func__);
		return -EINVAL;
	}
	sih_haptic = container_of(cdev, sih_haptic_t, soft_frame.vib_dev);
	if (!sih_haptic) {
		hp_err("%s: sih_haptic is null!\n", __func__);
		return -EINVAL;
	}

	len = sih_haptic->hp_func->get_ram_seq_gain(sih_haptic, buf);

	return len;
}

static ssize_t seq_gain_store(
	struct device *dev,
	struct device_attribute *attr,
	const char *buf,
	size_t count)
{
	cdev_t *cdev = NULL;
	sih_haptic_t *sih_haptic = NULL;
	uint32_t databuf[2] = { 0, 0 };

	if (!buf || !dev) {
		hp_err("%s: buf or dev is null!\n", __func__);
		return -EINVAL;
	}
	cdev = dev_get_drvdata(dev);
	if (!cdev) {
		hp_err("%s: cdev is null!\n", __func__);
		return -EINVAL;
	}
	sih_haptic = container_of(cdev, sih_haptic_t, soft_frame.vib_dev);
	if (!sih_haptic) {
		hp_err("%s: sih_haptic is null!\n", __func__);
		return -EINVAL;
	}

	if (sscanf(buf, "%x %x", &databuf[0], &databuf[1]) == 2) {
		if (databuf[0] >= SIH_HAPTIC_SEQUENCER_SIZE) {
			hp_err("%s: input value out of range!\n", __func__);
			return count;
		}

		if (databuf[1] > SIH_HAPTIC_GAIN_LIMIT) {
			hp_err("%s: gain out of limit!\n", __func__);
			databuf[1] = SIH_HAPTIC_GAIN_LIMIT;
		}

		hp_info("%s: seq%d gain = 0x%02x\n",
			__func__, databuf[0], databuf[1]);
		mutex_lock(&sih_haptic->lock);
		sih_haptic->ram_para.gain[databuf[0]] = (uint8_t)databuf[1];
		sih_haptic->hp_func->set_ram_seq_gain(sih_haptic,
			(uint8_t)databuf[0],
			sih_haptic->ram_para.gain[databuf[0]]);
		mutex_unlock(&sih_haptic->lock);
	}

	return count;
}

static ssize_t activate_mode_show(struct device *dev,
				struct device_attribute *attr, char *buf)
{
	cdev_t *cdev = NULL;
	sih_haptic_t *sih_haptic = NULL;

	if (!buf || !dev) {
		hp_err("%s: buf or dev is null!\n", __func__);
		return -EINVAL;
	}
	cdev = dev_get_drvdata(dev);
	if (!cdev) {
		hp_err("%s: cdev is null!\n", __func__);
		return -EINVAL;
	}
	sih_haptic = container_of(cdev, sih_haptic_t, soft_frame.vib_dev);
	if (!sih_haptic) {
		hp_err("%s: sih_haptic is null!\n", __func__);
		return -EINVAL;
	}

	return snprintf(buf, PAGE_SIZE, "activate_mode = %d\n",
			sih_haptic->activate_mode);
}

static ssize_t activate_mode_store(struct device *dev,
						struct device_attribute *attr,
						const char *buf, size_t count)
{
	cdev_t *cdev = NULL;
	sih_haptic_t *sih_haptic = NULL;
	uint32_t val = 0;
	int rc = 0;

	if (!buf || !dev) {
		hp_err("%s: buf or dev is null!\n", __func__);
		return -EINVAL;
	}
	cdev = dev_get_drvdata(dev);
	if (!cdev) {
		hp_err("%s: cdev is null!\n", __func__);
		return -EINVAL;
	}
	sih_haptic = container_of(cdev, sih_haptic_t, soft_frame.vib_dev);
	if (!sih_haptic) {
		hp_err("%s: sih_haptic is null!\n", __func__);
		return -EINVAL;
	}

	rc = kstrtouint(buf, 0, &val);
	hp_info("activate_mode = %d", val);
	if (rc < 0)
		return rc;
	mutex_lock(&sih_haptic->lock);
	sih_haptic->activate_mode = val;
	mutex_unlock(&sih_haptic->lock);
	return count;
}

static ssize_t set_amplitude_store(struct device *dev,
						struct device_attribute *attr,
						const char *buf, size_t count)
{
	cdev_t *cdev = NULL;
	sih_haptic_t *sih_haptic = NULL;
	uint32_t val = 0;
	int rc = 0;

	if (!buf || !dev) {
		hp_err("%s: buf or dev is null!\n", __func__);
		return -EINVAL;
	}
	cdev = dev_get_drvdata(dev);
	if (!cdev) {
		hp_err("%s: cdev is null!\n", __func__);
		return -EINVAL;
	}
	sih_haptic = container_of(cdev, sih_haptic_t, soft_frame.vib_dev);
	if (!sih_haptic) {
		hp_err("%s: sih_haptic is null!\n", __func__);
		return -EINVAL;
	}

	rc = kstrtouint(buf, 0, &val);
	if (rc < 0)
		return rc;
	hp_info("value=%d", val);
	if (val == ADJUST_GAIN_CAM_RESONANCES)
		sih_haptic->set_amplitude_flag = 1;
	else
		sih_haptic->set_amplitude_flag = 0;
	return count;
}

static ssize_t cali_val_show(struct device *dev,
				struct device_attribute *attr, char *buf)
{
	ssize_t len = 0;
	cdev_t *cdev = NULL;
	sih_haptic_t *sih_haptic = NULL;

	if (!buf || !dev) {
		hp_err("%s: buf or dev is null!\n", __func__);
		return -EINVAL;
	}
	cdev = dev_get_drvdata(dev);
	if (!cdev) {
		hp_err("%s: cdev is null!\n", __func__);
		return -EINVAL;
	}
	sih_haptic = container_of(cdev, sih_haptic_t, soft_frame.vib_dev);
	if (!sih_haptic) {
		hp_err("%s: sih_haptic is null!\n", __func__);
		return -EINVAL;
	}

	len += snprintf(buf + len, PAGE_SIZE - len, "%d",
		sih_haptic->detect_para.tracking_f0);

	return len;
}

static ssize_t tracking_f0_show(
	struct device *dev,
	struct device_attribute *attr,
	char *buf)
{
	ssize_t len = 0;
	uint8_t i;
	cdev_t *cdev = NULL;
	sih_haptic_t *sih_haptic = NULL;

	if (!buf || !dev) {
		hp_err("%s: buf or dev is null!\n", __func__);
		return -EINVAL;
	}
	cdev = dev_get_drvdata(dev);
	if (!cdev) {
		hp_err("%s: cdev is null!\n", __func__);
		return -EINVAL;
	}
	sih_haptic = container_of(cdev, sih_haptic_t, soft_frame.vib_dev);
	if (!sih_haptic) {
		hp_err("%s: sih_haptic is null!\n", __func__);
		return -EINVAL;
	}

	mutex_lock(&sih_haptic->lock);
	for (i = 0; i < SIH_F0_DETECT_TRY; i++) {
		sih_haptic->hp_func->get_tracking_f0(sih_haptic);
		msleep(200);
		if (sih_haptic->detect_para.tracking_f0 <= SIH_F0_MAX_THRESHOLD &&
			sih_haptic->detect_para.tracking_f0 >= SIH_F0_MIN_THRESHOLD) {
			break;
		}
	}
	mutex_unlock(&sih_haptic->lock);

	len += snprintf(buf, PAGE_SIZE, "tracking_f0 = %d\n", sih_haptic->detect_para.tracking_f0);

	return len;
}

static DEVICE_ATTR(f0, S_IWUSR | S_IRUGO,
	tracking_f0_show, NULL);
static DEVICE_ATTR(seq, S_IWUSR | S_IRUGO,
	seq_show, seq_store);
static DEVICE_ATTR(reg, S_IWUSR | S_IRUGO,
	reg_show, reg_store);
static DEVICE_ATTR(gain, S_IWUSR | S_IRUGO,
	gain_show, gain_store);
static DEVICE_ATTR(state, S_IWUSR | S_IRUGO,
	state_show, state_store);
static DEVICE_ATTR(loop, S_IWUSR | S_IRUGO,
	loop_show, loop_store);
static DEVICE_ATTR(rtp, S_IWUSR | S_IRUGO,
	rtp_show, rtp_store);
static DEVICE_ATTR(vibrator_calib, S_IWUSR | S_IRUGO,
	cali_show, cali_store);
static DEVICE_ATTR(cont, S_IWUSR | S_IRUGO,
	cont_show, cont_store);
static DEVICE_ATTR(ram, S_IWUSR | S_IRUGO,
	NULL, ram_store);
static DEVICE_ATTR(index, S_IWUSR | S_IRUGO,
	index_show, index_store);
static DEVICE_ATTR(ram_num, S_IWUSR | S_IRUGO,
	ram_num_show, NULL);
static DEVICE_ATTR(duration, S_IWUSR | S_IRUGO,
	duration_show, duration_store);
static DEVICE_ATTR(osc_cali, S_IWUSR | S_IRUGO,
	osc_cali_show, osc_cali_store);
static DEVICE_ATTR(auto_pvdd, S_IWUSR | S_IRUGO,
	auto_pvdd_show, auto_pvdd_store);
static DEVICE_ATTR(ram_update, S_IWUSR | S_IRUGO,
	ram_update_show, ram_update_store);
static DEVICE_ATTR(ram_vbat_comp, S_IWUSR | S_IRUGO,
	ram_vbat_comp_show, ram_vbat_comp_store);
static DEVICE_ATTR(lra_resistance, S_IWUSR | S_IRUGO,
	lra_resistance_show, lra_resistance_store);
static DEVICE_ATTR(trig_para, S_IWUSR | S_IRUGO,
	trig_para_show, trig_para_store);
static DEVICE_ATTR(low_power, S_IWUSR | S_IRUGO,
	low_power_show, low_power_store);
static DEVICE_ATTR(activate, S_IWUSR | S_IRUGO,
	activate_show, activate_store);
static DEVICE_ATTR(vmax, S_IWUSR | S_IRUGO,
	vmax_show, vmax_store);
static DEVICE_ATTR(vbat_monitor, S_IWUSR | S_IRUGO,
	vbat_monitor_show, NULL);
static DEVICE_ATTR(main_loop, S_IWUSR | S_IRUGO,
	main_loop_show, main_loop_store);
static DEVICE_ATTR(seq_gain, S_IWUSR | S_IRUGO,
	seq_gain_show, seq_gain_store);
static DEVICE_ATTR(activate_mode, S_IWUSR | S_IRUGO,
	activate_mode_show, activate_mode_store);
static DEVICE_ATTR(set_amplitude, S_IWUSR | S_IRUGO,
	 NULL, set_amplitude_store);
static DEVICE_ATTR(cali_val, S_IWUSR | S_IRUGO,
	 cali_val_show, NULL);
static struct attribute *sih_vibra_attribute[] = {
		&dev_attr_f0.attr,
		&dev_attr_seq.attr,
		&dev_attr_reg.attr,
		&dev_attr_gain.attr,
		&dev_attr_state.attr,
		&dev_attr_loop.attr,
		&dev_attr_rtp.attr,
		&dev_attr_vibrator_calib.attr,
		&dev_attr_cont.attr,
		&dev_attr_ram.attr,
		&dev_attr_index.attr,
		&dev_attr_ram_num.attr,
		&dev_attr_duration.attr,
		&dev_attr_osc_cali.attr,
		&dev_attr_auto_pvdd.attr,
		&dev_attr_ram_update.attr,
		&dev_attr_ram_vbat_comp.attr,
		&dev_attr_lra_resistance.attr,
		&dev_attr_trig_para.attr,
		&dev_attr_low_power.attr,
		&dev_attr_activate.attr,
		&dev_attr_vmax.attr,
		&dev_attr_vbat_monitor.attr,
		&dev_attr_main_loop.attr,
		&dev_attr_seq_gain.attr,
		&dev_attr_activate_mode.attr,
		&dev_attr_set_amplitude.attr,
		&dev_attr_cali_val.attr,
		NULL,
};

static struct attribute_group sih_vibra_attribute_group = {
	.attrs = sih_vibra_attribute,
};

#ifdef TIMED_OUTPUT
static int  sih_vibra_get_time(
	struct timed_output_dev *dev)
{
	sih_haptic_t *sih_haptic = container_of(dev,
		sih_haptic_t, soft_frame.vib_dev);
	ktime_t remain_time;

	if (hrtimer_active(&sih_haptic->timer)) {
		remain_time = hrtimer_get_remaining(&sih_haptic->timer);
		return ktime_to_ms(remain_time);
	}
	return 0;
}

static void sih_vibra_enable(
	struct timed_output_dev *dev,
	int value)
{
	hp_err("%s:enter!\n", __func__);
	sih_haptic_t *sih_haptic = container_of(dev,
		sih_haptic_t, soft_frame.vib_dev);

	if (!sih_haptic->ram_para.ram_init) {
		hp_err("%s:ram init error\n", __func__);
		return;
	}

	mutex_lock(&sih_haptic->lock);

	if (value > 0) {
#ifdef SIH_PRIORITY_OPEN
		hp_info("%s:sih_haptic->register_pri.ram = %d current priority = %d\n",
			__func__, sih_haptic->register_pri.ram,
			sih_haptic->chip_ipara.cur_pri);
		if (!sih_priority_judge(sih_haptic,
			sih_haptic->register_pri.ram)) {
			mutex_unlock(&sih_haptic->lock);
			hp_err("%s:sih_priority_judge return\n", __func__);
			return;
		}
#endif
		sih_haptic->hp_func->play_stop(sih_haptic);
		sih_haptic->chip_ipara.action_mode = SIH_HAPTIC_RAM_MODE;
		sih_haptic->chip_ipara.state = SIH_HAPTIC_ACTIVE_MODE;

		sih_ram_play(sih_haptic, sih_haptic->chip_ipara.action_mode);
	}
	mutex_unlock(&sih_haptic->lock);
}
#else
static enum led_brightness sih_vibra_brightness_get(
	struct led_classdev *cdev)
{
	return LED_OFF;
}

static void sih_vibra_brightness_set(
	struct led_classdev *cdev,
	enum led_brightness level)
{
	sih_haptic_t *sih_haptic = container_of(cdev,
		sih_haptic_t, soft_frame.vib_dev);

	hp_err("%s:enter!\n", __func__);

	if (!sih_haptic->ram_para.ram_init) {
		hp_err("%s:ram init error\n", __func__);
		return;
	}

	mutex_lock(&sih_haptic->lock);
	if (level > 0) {
#ifdef SIH_PRIORITY_OPEN
		hp_info("%s:pri.ram = %d,cur_pri = %d\n",
			__func__, sih_haptic->register_pri.ram,
			sih_haptic->chip_ipara.cur_pri);
		if (!sih_priority_judge(sih_haptic,
			sih_haptic->register_pri.ram)) {
			mutex_unlock(&sih_haptic->lock);
			hp_err("%s:sih_priority_judge return\n", __func__);
			return;
		}
#endif
		sih_haptic->hp_func->play_stop(sih_haptic);
		sih_haptic->chip_ipara.action_mode = SIH_HAPTIC_RAM_MODE;
		sih_haptic->chip_ipara.state = SIH_HAPTIC_ACTIVE_MODE;

		sih_ram_play(sih_haptic, sih_haptic->chip_ipara.action_mode);
	}
	mutex_unlock(&sih_haptic->lock);
}
#endif

static int __maybe_unused sih_add_node(
	struct device *dev,
	sih_haptic_t *sih_haptic)
{
	int ret = -1;

#ifdef TIMED_OUTPUT
	sih_haptic->soft_frame.vib_dev.name = SIH_HAPTIC_NAME;
	sih_haptic->soft_frame.vib_dev.get_time = sih_vibra_get_time;
	sih_haptic->soft_frame.vib_dev.enable = sih_vibra_enable;

	/* time output frame register */
	ret = timed_output_dev_register(&(sih_haptic->soft_frame.vib_dev));
	if (ret < 0) {
		hp_err("%s: timed out frame register failed return value =%d\n",
			__func__, ret);
		return ret;
	}

	/* vibrator sysfs node create */
	ret = sysfs_create_group(&sih_haptic->soft_frame.vib_dev.dev->kobj,
		&sih_vibra_attribute_group);
	if (ret < 0) {
		hp_err("%s: vibrator sysfs node create failed\n ", __func__);
		return ret;
	}
#else
	sih_haptic->soft_frame.vib_dev.name = SIH_HAPTIC_NAME;
	sih_haptic->soft_frame.vib_dev.brightness_get =
		sih_vibra_brightness_get;
	sih_haptic->soft_frame.vib_dev.brightness_set =
		sih_vibra_brightness_set;
	sih_haptic->soft_frame.vib_dev.default_trigger =
		"hw_vb_trigger";

	/* led sub system register */
	ret = devm_led_classdev_register(sih_haptic->dev,
		&sih_haptic->soft_frame.vib_dev);
	if (ret < 0) {
		hp_err("%s: led sub system register failed\n", __func__);
		return ret;
	}

	/* vibrator sysfs node create */
	ret = sysfs_create_group(&sih_haptic->soft_frame.vib_dev.dev->kobj,
		&sih_vibra_attribute_group);
	if (ret < 0) {
		hp_err("vibrator sysfs node create failed\n ", __func__);
		return ret;
	}
#endif
	return 0;
}

static enum hrtimer_restart haptic_timer_func(
	struct hrtimer *timer)
{
	struct sih_haptic *sih_haptic = container_of(timer,
					struct sih_haptic, timer);

	hp_info("%s enter!\n", __func__);
	sih_chip_state_recovery(sih_haptic);
	schedule_work(&sih_haptic->vibrator_work);
	return HRTIMER_NORESTART;
}

static void si_long_ram_mode_boost_config(sih_haptic_t *sih_haptic,
	unsigned char wavseq)
{
	if (sih_haptic->cust_boost_on == SI_SEL_BOOST_CFG_ON) {
		sih_haptic->hp_func->set_wav_seq(sih_haptic, 0, 0x14);
		sih_haptic->hp_func->set_wav_internal_loop(sih_haptic, 0, 0); // loop mode
		sih_haptic->hp_func->set_wav_seq(sih_haptic, 1, wavseq);
		sih_haptic->hp_func->set_wav_internal_loop(sih_haptic, 1, 0x0f);
		sih_haptic->hp_func->set_wav_seq(sih_haptic, 2, 0);
		sih_haptic->hp_func->set_wav_internal_loop(sih_haptic, 2, 0);
	} else {
		sih_haptic->hp_func->set_wav_seq(sih_haptic, 0, wavseq);
		sih_haptic->hp_func->set_wav_internal_loop(sih_haptic, 0, 0x0f);
		sih_haptic->hp_func->set_wav_seq(sih_haptic, 1, 0);
		sih_haptic->hp_func->set_wav_internal_loop(sih_haptic, 1, 0);
	}
}

static int si_haptic_play_repeat_seq(sih_haptic_t *sih_haptic,
	unsigned char flag)
{
	hp_info("%s enter!\n", __func__);

	if (!sih_haptic) {
		hp_err("si_haptic is null");
		return -EINVAL;
	}
	if (flag) {
		sih_haptic->hp_func->set_play_mode(sih_haptic, SIH_HAPTIC_RAM_LOOP_MODE);
		sih_haptic->hp_func->play_go(sih_haptic, true);
	}
	return 0;
}

static int si_haptic_ram_config(
	sih_haptic_t *sih_haptic)
{
	unsigned char wavseq = 0;
	unsigned char wavloop = 0;
	unsigned char seq_idx = 0;
	// unsigned int total_wavloop = 0;

	if (!sih_haptic) {
		hp_err("si_haptic is null");
		return -EINVAL;
	}
	hp_info("duration=%d,index = %d\n", sih_haptic->duration, sih_haptic->index);
	for (seq_idx = 0; seq_idx < 8; seq_idx++) {
		sih_haptic->hp_func->set_wav_seq(sih_haptic, seq_idx, 0);
		sih_haptic->hp_func->set_wav_internal_loop(sih_haptic, seq_idx, 0);
	}
	if (sih_haptic->effect_mode == SHORT_VIB_RAM_MODE) {
		wavseq = sih_haptic->index;
		wavloop = 0;
		sih_haptic->hp_func->set_wav_seq(sih_haptic, 0, wavseq);
		sih_haptic->hp_func->set_wav_internal_loop(sih_haptic, 0, wavloop);
		sih_haptic->hp_func->set_wav_seq(sih_haptic, 1, 0);
		sih_haptic->hp_func->set_wav_internal_loop(sih_haptic, 1, 0);
		hp_info ("waveseq = %s\n", wavseq);
	} else {
		wavseq = sih_haptic->index;
		si_long_ram_mode_boost_config(sih_haptic, wavseq);
	}
	return 0;
}

static void ram_mode_sel_boost_play_cfg(
	sih_haptic_t *sih_haptic)
{
	if (sih_haptic->cust_boost_on == SI_SEL_BOOST_CFG_ON) {
		sih_haptic->hp_func->set_play_mode(sih_haptic, SIH_HAPTIC_RAM_MODE);
		sih_haptic->hp_func->play_go(sih_haptic, true);
		return;
	}
	if (sih_haptic->effect_mode == LONG_VIB_RAM_MODE) {
		si_haptic_play_repeat_seq(sih_haptic, true);
	} else {
		sih_haptic->hp_func->set_play_mode(sih_haptic, SIH_HAPTIC_RAM_MODE);
		sih_haptic->hp_func->play_go(sih_haptic, true);
	}
}

static void si_cont_config(
	sih_haptic_t *sih_haptic)
{
	uint32_t t_half_num = 0xff;
	uint32_t cont_index = 0;
	uint32_t val = 1;

	hp_info("%s : enter\n", __func__);
	sih_haptic->hp_func->play_stop(sih_haptic);
	cont_index = 5;
	sih_haptic->hp_func->update_cont_config(sih_haptic, cont_index, t_half_num);
	cont_index = 7;
	sih_haptic->hp_func->update_cont_config(sih_haptic, cont_index, val);
}

static void vibrator_work_routine(struct work_struct *work)
{
	sih_haptic_t *sih_haptic = NULL;

	if (!work) {
		hp_err("work is null");
		return;
	}
	sih_haptic = container_of(work, sih_haptic_t, vibrator_work);

	/* charging mode */
	mutex_lock(&sih_haptic->lock);

	if (sih_haptic->f0_is_cali == HAPTIC_CHARGING_IS_CALIB) {
		sih_haptic->hp_func->get_tracking_f0(sih_haptic);
		sih_haptic->f0_is_cali = 0;
		mutex_unlock(&sih_haptic->lock);
		return;
	}

	/* Enter standby mode */
	sih_haptic->hp_func->play_stop(sih_haptic);
	if (sih_haptic->chip_ipara.state) {
		hp_info("state is true");
		if (sih_haptic->activate_mode == SI_HAPTIC_ACTIVATE_RAM_MODE) {
			hp_info("activate_mode == SI_HAPTIC_ACTIVATE_RAM_MODE");
			si_haptic_ram_config(sih_haptic);
			if (sih_haptic->duration <= SI_LONG_HAPTIC_RUNNING) { // max long vibra config
				hp_info("duration <= SI_LONG_HAPTIC_RUNNING");
				ram_mode_sel_boost_play_cfg(sih_haptic);
			} else {
				si_haptic_play_repeat_seq(sih_haptic, true);
			}
		} else if (sih_haptic->activate_mode == SI_HAPTIC_ACTIVATE_CONT_MODE) {
			hp_info("activate_mode == SI_HAPTIC_ACTIVATE_CONT_MODE");
			si_cont_config(sih_haptic);
		} else {
			/* other mode */
			hp_err("set other mode\n");
		}
		/* run ms timer */
		if (sih_haptic->effect_mode == LONG_VIB_RAM_MODE ||
			(sih_haptic->activate_mode == SI_HAPTIC_ACTIVATE_CONT_MODE)) {
			hp_info("run ms timer");
			hrtimer_start(&sih_haptic->timer, ktime_set(sih_haptic->duration / 1000,
				(sih_haptic->duration % 1000) * 1000000), HRTIMER_MODE_REL);
			hp_info("%s gain = 0x%02X\n",  __func__, sih_haptic->chip_ipara.gain);
		}
	}
	mutex_unlock(&sih_haptic->lock);
}

static void ram_work_func(
	struct work_struct *work)
{
	sih_haptic_t *sih_haptic = container_of(work, sih_haptic_t, ram_work);

	hp_info("%s enter!\n", __func__);
	mutex_lock(&sih_haptic->lock);
	/* Enter standby mode */
	sih_haptic->hp_func->play_stop(sih_haptic);
	if (sih_haptic->chip_ipara.state == SIH_HAPTIC_ACTIVE_MODE) {
		switch (sih_haptic->chip_ipara.action_mode) {
		case SIH_HAPTIC_RAM_MODE:
			sih_ram_play(sih_haptic, SIH_HAPTIC_RAM_MODE);
			break;
		case SIH_HAPTIC_RAM_LOOP_MODE:
			sih_ram_play(sih_haptic, SIH_HAPTIC_RAM_LOOP_MODE);
			/* run ms timer */
			hrtimer_start(&sih_haptic->timer,
				ktime_set(sih_haptic->chip_ipara.duration /
				1000,
				(sih_haptic->chip_ipara.duration % 1000) *
				1000000), HRTIMER_MODE_REL);
			break;
		default:
			hp_err("%s: err state = %d\n",
				__func__, sih_haptic->chip_ipara.state);
			break;
		}
	}
	mutex_unlock(&sih_haptic->lock);
}

static int rtp_work_func_load_firmware(
	sih_haptic_t *sih_haptic)
{
	int ret = -1;
	const struct firmware *rtp_file;
	hp_info("%s enter!\n", __func__);
	mutex_lock(&sih_haptic->rtp_para.rtp_lock);

	sih_haptic->rtp_para.rtp_init = false;
	sih_vfree_container(sih_haptic, sih_haptic->rtp_para.rtp_cont);
	sih_haptic->rtp_para.rtp_pre_mode = SIH_RTP_LOCAL_PLAY;

	ret = request_firmware(&rtp_file,
		sih_rtp_name[sih_haptic->rtp_para.rtp_file_num],
		sih_haptic->dev);
	if (ret < 0) {
		hp_err("%s: failed to read %s\n", __func__,
			sih_rtp_name[sih_haptic->rtp_para.rtp_file_num]);
		sih_chip_state_recovery(sih_haptic);
		sih_change_play_priority(sih_haptic, SIH_BROADCAST_PRI_MIN);
		mutex_unlock(&sih_haptic->rtp_para.rtp_lock);
		return ret;
	}

	sih_haptic->rtp_para.rtp_cont = vmalloc(rtp_file->size + sizeof(int));
	if (!sih_haptic->rtp_para.rtp_cont) {
		release_firmware(rtp_file);
		hp_err("%s: error allocating memory\n", __func__);
		sih_chip_state_recovery(sih_haptic);
		sih_change_play_priority(sih_haptic, SIH_BROADCAST_PRI_MIN);
		mutex_unlock(&sih_haptic->rtp_para.rtp_lock);
		return -1;
	}
	sih_haptic->rtp_para.rtp_cont->len = rtp_file->size;

	memcpy(sih_haptic->rtp_para.rtp_cont->data,
				rtp_file->data, rtp_file->size);

	release_firmware(rtp_file);

	mutex_unlock(&sih_haptic->rtp_para.rtp_lock);

	hp_info("%s: rtp data len is %d\n", __func__, sih_haptic->rtp_para.rtp_cont->len);
	return 0;
}

static void rtp_work_func(
	struct work_struct *work)
{
	bool rtp_work_flag = false;
	int cnt = 200;
	int ret;

	sih_haptic_t *sih_haptic = container_of(work,
		sih_haptic_t, rtp_para.rtp_work);

	hp_info("%s enter!\n", __func__);

	ret = rtp_work_func_load_firmware(sih_haptic);
	if (ret < 0)
		return;

	mutex_lock(&sih_haptic->lock);

	sih_haptic->rtp_para.rtp_init = true;

	sih_haptic->hp_func->upload_lra(sih_haptic, SIH_OSC_CALI_LRA);
	sih_haptic->hp_func->set_rtp_aei(sih_haptic, false);
	sih_haptic->hp_func->clear_interrupt_state(sih_haptic);
	sih_haptic->hp_func->play_stop(sih_haptic);
	/* gain */

	/* boost voltage */

	/* rtp mode config */
	sih_haptic->hp_func->set_play_mode(sih_haptic, SIH_HAPTIC_RTP_MODE);

	/* haptic go */
	sih_haptic->hp_func->play_go(sih_haptic, true);
	usleep_range(2000, 2500);
	while (cnt) {
		if (sih_haptic->hp_func->if_is_mode(sih_haptic,
				SIH_HAPTIC_RTP_MODE)) {
			cnt = 0;
			rtp_work_flag = true;
			hp_err("%s: rtp_go!\n", __func__);
		} else {
			cnt--;
			hp_err("%s: wait for rtp_go!\n", __func__);
		}
		usleep_range(2000, 2500);
	}
	if (rtp_work_flag) {
		hp_info("%s: go sih_rtp_play !\n", __func__);
		sih_rtp_play(sih_haptic);
	} else {
		/* enter standby mode */
		sih_haptic->hp_func->play_stop(sih_haptic);
		sih_chip_state_recovery(sih_haptic);
		sih_change_play_priority(sih_haptic, SIH_BROADCAST_PRI_MIN);
		hp_err("%s: failed to enter rtp_go status!\n", __func__);
	}

	mutex_unlock(&sih_haptic->lock);
}

static void rtp_data_work_func(
	struct work_struct *work)
{
	bool rtp_work_flag = false;
	int cnt = 200;

	sih_haptic_t *sih_haptic = container_of(work,
		sih_haptic_t, rtp_para.rtp_data_work);

	hp_info("%s enter!\n", __func__);

	mutex_lock(&sih_haptic->rtp_para.rtp_lock);
	sih_haptic->rtp_para.rtp_init = false;
	mutex_unlock(&sih_haptic->rtp_para.rtp_lock);

	mutex_lock(&sih_haptic->lock);
	sih_haptic->rtp_para.rtp_init = true;

	sih_haptic->hp_func->upload_lra(sih_haptic, SIH_OSC_CALI_LRA);
	sih_haptic->hp_func->set_rtp_aei(sih_haptic, false);

	sih_haptic->hp_func->play_stop(sih_haptic);

	sih_haptic->hp_func->clear_interrupt_state(sih_haptic);
	/* rtp mode config */
	sih_haptic->hp_func->set_play_mode(sih_haptic, SIH_HAPTIC_RTP_MODE);
	/* haptic go */
	sih_haptic->hp_func->play_go(sih_haptic, true);
	usleep_range(2000, 2500);
	while (cnt) {
		if (sih_haptic->hp_func->if_is_mode(sih_haptic,
			SIH_HAPTIC_RTP_MODE)) {
			cnt = 0;
			rtp_work_flag = true;
			hp_info("%s: rtp_go!\n", __func__);
		} else {
			cnt--;
			hp_info("%s: wait for rtp_go!\n", __func__);
		}
		usleep_range(2000, 2500);
	}

	if (rtp_work_flag) {
		sih_rtp_play(sih_haptic);
	} else {
		/* enter standby mode */
		sih_haptic->hp_func->play_stop(sih_haptic);
		sih_chip_state_recovery(sih_haptic);
		hp_err("%s: failed to enter rtp_go status!\n", __func__);
	}
	/* enable fifo empty interrupt */
	mutex_unlock(&sih_haptic->lock);
}

static void delay_init_work_func(
	struct work_struct *work)
{
	sih_haptic_t *sih_haptic = container_of(work,
		sih_haptic_t, delay_init_work.work);

	sih_haptic->hp_func->set_low_power_mode(sih_haptic, 1);
	sih_haptic->hp_func->play_stop(sih_haptic);
	sih_haptic->hp_func->efuse_check(sih_haptic);
	sih_chip_state_recovery(sih_haptic);
	sih_change_play_priority(sih_haptic, SIH_BROADCAST_PRI_MIN);

	sih_haptic->hp_func->init_chip_parameter(sih_haptic);
	sih_haptic->hp_func->get_lra_resistance(sih_haptic);
	sih_haptic->hp_func->get_vbat(sih_haptic);
	sih_chip_config(sih_haptic);
	sih_haptic->hp_func->get_tracking_f0(sih_haptic);
	sih_rtp_data_play_prehandle(sih_haptic);

	hp_info("%s: end\n", __func__);
}

static void vibrator_parameter_init(
	sih_haptic_t *sih_haptic)
{
	/* normal driving voltage */
	sih_haptic->chip_attr.com_drv_viol = SIH_HAPTIC_COM_DRV_VIOL;

	/* exceed driving voltage */
	sih_haptic->chip_attr.exceed_drv_val = SIH_HAPTIC_EXCEED_DRV_VAL;

	/* ram wave init */
	sih_haptic->chip_ipara.ram_wave = 0;

	/* f0 pre init */
	sih_haptic->detect_para.detect_f0 = SIH_F0_PRE_VALUE;
	sih_haptic->detect_para.tracking_f0 = SIH_TRACKING_F0_PRE_VALUE;
	sih_haptic->detect_para.cali_target_value = SIH_F0_PRE_VALUE;

	/* rl pre init */
	sih_haptic->detect_para.detect_rl = 0;
	sih_haptic->detect_para.detect_rl_rawdata = 0;
	sih_haptic->detect_para.detect_rl_offset = 0;
	/* vbat init */
	sih_haptic->detect_para.detect_vbat = 0;
	sih_haptic->detect_para.detect_vbat_rawdata = 0;
	sih_haptic->activate_mode = 0;
	/* rtp start thres init */
	sih_haptic->rtp_para.rtp_start_thres = SIH_RTP_START_DEFAULT_THRES;
}

static void si_adapt_amp_again(sih_haptic_t *sih_haptic, int haptic_type)
{
	if (!sih_haptic) {
		hp_err("hp_haptic is null");
		return;
	}
	switch (haptic_type) {
	case LONG_VIB_RAM_MODE:
		if (sih_haptic->amplitude >= SI_LONG_MAX_AMP_CFG || sih_haptic->amplitude < 0)
			return;
		sih_haptic->chip_ipara.gain = long_amp_again_val[sih_haptic->amplitude];
		hp_info("long gain = %u\n", sih_haptic->chip_ipara.gain);
		break;
	case SHORT_VIB_RAM_MODE:
		if (sih_haptic->amplitude >= SI_SHORT_MAX_AMP_CFG || sih_haptic->amplitude < 0)
			return;
		sih_haptic->chip_ipara.gain = short_amp_again_val[sih_haptic->amplitude];
		hp_info("short gain = %u\n", sih_haptic->chip_ipara.gain);
		break;
	case RTP_VIB_MODE:
		sih_haptic->chip_ipara.gain = 0x80; // define max amp
		break;
	default:
		break;
	}
}

static void si_special_type_process(sih_haptic_t *sih_haptic,
	uint64_t type)
{
	switch (type) {
	case HAPTIC_CHARGING_CALIB_ID:
		sih_haptic->f0_is_cali = HAPTIC_CHARGING_IS_CALIB;
		schedule_work(&sih_haptic->vibrator_work);
		break;
	case HAPTIC_NULL_WAVEFORM_ID:
		hp_err("null wave id");
		break;
	default:
		break;
	}
}

static int si_file_open(struct inode *inode, struct file *file)
{
	if (!file)
		return 0;

	if (!try_module_get(THIS_MODULE))
		return -ENODEV;

	file->private_data = (void *)s_haptic;

	return 0;
}

static int si_file_release(struct inode *inode, struct file *file)
{
	if (!file)
		return 0;
	file->private_data = (void *)NULL;

	module_put(THIS_MODULE);
	return 0;
}

static long si_file_unlocked_ioctl(struct file *file,
	unsigned int cmd, unsigned long arg)
{
	sih_haptic_t *sih_haptic = NULL;

	hp_info("si file ioctl \n");

	if (!file) {
		hp_err("file is null\n", __func__);
		return -EINVAL;
	}
	sih_haptic = (sih_haptic_t *)file->private_data;
	if (!sih_haptic) {
		hp_err("sih_haptic is null\n", __func__);
		return -EINVAL;
	}
	hp_info("cmd=0x%x, arg=0x%lx\n", cmd, arg);

	mutex_lock(&sih_haptic->lock);
	if (_IOC_TYPE(cmd) != AW_HAPTIC_IOCTL_MAGIC) {
		hp_err("%s: cmd magic err\n", __func__);
		mutex_unlock(&sih_haptic->lock);
		return -EINVAL;
	}

	switch (cmd) {
	default:
		hp_err("%s, unknown cmd\n", __func__);
		break;
	}
	mutex_unlock(&sih_haptic->lock);
	return 0;
}

static ssize_t si_file_read(struct file *filp,
	char *buff, size_t len, loff_t *offset)
{
	sih_haptic_t *sih_haptic = NULL;
	int ret = 0;
	int i;
	unsigned char reg_val = 0;
	unsigned char *pbuff = NULL;

	hp_info("si file read \n");

	if (!filp || !buff) {
		hp_err("filp or buff is null\n", __func__);
		return -EINVAL;
	}

	sih_haptic = (sih_haptic_t *)filp->private_data;

	if (!sih_haptic) {
		hp_err("sih_haptic is null\n", __func__);
		return -EINVAL;
	}

	mutex_lock(&sih_haptic->lock);
	hp_info("len=%d\n", len);
	switch (sih_haptic->fileops.cmd) {
	case SI_HAPTIC_CMD_READ_REG:
		pbuff = kzalloc(len, GFP_KERNEL);
		if (pbuff != NULL) {
			for (i = 0; i < len; i++) {
				i2c_read_bytes(sih_haptic,
						(uint8_t)sih_haptic->fileops.reg + i,
						(uint8_t *)&reg_val, SIH_I2C_OPERA_BYTE_ONE);
				pbuff[i] = reg_val;
			}
			ret = copy_to_user(buff, pbuff, len);
			if (ret)
				hp_err("copy to user fail\n");
			kfree(pbuff);
		} else {
			hp_err("alloc memory fail\n");
		}
		break;
	default:
		hp_err("unknown cmd %d\n", sih_haptic->fileops.cmd);
		break;
	}

	mutex_unlock(&sih_haptic->lock);

	for (i = 0; i < len; i++)
		hp_info("buff[%d]=0x%02x\n", i, buff[i]);

	return len;
}

static void si_parse_haptic_type(
	sih_haptic_t *sih_haptic, uint64_t type)
{
	int rtp_len = 0;

	hp_info("si get haptic id = %llu\n", type);

	if ((type < HAPTIC_MAX_SPEC_CMD_ID) && (type > HAPTIC_MIN_SPEC_CMD_ID)) {
		si_special_type_process(sih_haptic, type);
		return;
	}

	if (type > LONG_HAPTIC_RTP_MAX_ID) { // long time
		sih_haptic->effect_mode = LONG_VIB_RAM_MODE;
		sih_haptic->index = LONG_VIB_EFFECT_ID;
		sih_haptic->duration = type / LONG_TIME_AMP_DIV_COFF;
		sih_haptic->amplitude = type % LONG_TIME_AMP_DIV_COFF;
		sih_haptic->chip_ipara.state = 1;
		si_adapt_amp_again(sih_haptic, LONG_VIB_RAM_MODE);
		__pm_wakeup_event(sih_haptic->ws, sih_haptic->duration + HAPTIC_WAKE_LOCK_GAP);
		hp_info("long index = %d, amp = %d\n", sih_haptic->index, sih_haptic->amplitude);
		schedule_work(&sih_haptic->vibrator_work);
	} else if ((type > 0) && (type <= SHORT_HAPTIC_RAM_MAX_ID)) { // short ram haptic
		sih_haptic->effect_mode = SHORT_VIB_RAM_MODE;
		sih_haptic->amplitude = type % SHORT_HAPTIC_AMP_DIV_COFF;
		sih_haptic->index = type / SHORT_HAPTIC_AMP_DIV_COFF;
		si_adapt_amp_again(sih_haptic, SHORT_VIB_RAM_MODE);
		hp_info("short index = %d, amp = %d\n", sih_haptic->index, sih_haptic->amplitude);
		sih_haptic->chip_ipara.state = 1;
		schedule_work(&sih_haptic->vibrator_work);
	} else { // long and short rtp haptic
		sih_haptic->effect_mode = RTP_VIB_MODE;
		sih_haptic->amplitude = type % SHORT_HAPTIC_AMP_DIV_COFF;
		sih_haptic->index = type / SHORT_HAPTIC_AMP_DIV_COFF;
		sih_haptic->hp_func->play_stop(sih_haptic);
		sih_haptic->hp_func->set_rtp_aei(sih_haptic, false);
		sih_haptic->hp_func->clear_interrupt_state(sih_haptic);
		si_adapt_amp_again(sih_haptic, RTP_VIB_MODE);
		sih_haptic->chip_ipara.state = 1;
		sih_haptic->rtp_idx = sih_haptic->index - BASE_INDEX;
		rtp_len += snprintf(si_rtp_idx_name, SI_RTP_NAME_MAX - 1,
							"si_%d.bin", sih_haptic->rtp_idx);
		hp_info(" get rtp name = %s, index = %d, len = %d\n",
			si_rtp_idx_name, sih_haptic->rtp_idx, rtp_len);
		schedule_work(&sih_haptic->rtp_para.rtp_work);
	}
}

static ssize_t si_file_write(struct file *filp,
	const char *buff, size_t len, loff_t *off)
{
	sih_haptic_t *sih_haptic = NULL;
	char write_buf[MAX_WRITE_BUF_LEN] = {0};
	uint64_t type = 0;

	hp_info("si file write \n");

	if (!buff || !filp || (len > (MAX_WRITE_BUF_LEN - 1)))
		return len;

	sih_haptic = (sih_haptic_t *)filp->private_data;
	if (!sih_haptic)
		return len;
	mutex_lock(&sih_haptic->lock);
	if (copy_from_user(write_buf, buff, len)) {
		hp_err("[si_haptics_write] copy_from_user failed\n");
		mutex_unlock(&sih_haptic->lock);
		return len;
	}
	if (kstrtoull(write_buf, 10, &type)) {
		hp_err("[si_haptics_write] read value error\n");
		mutex_unlock(&sih_haptic->lock);
		return len;
	}

	sih_haptic->cust_boost_on = SI_SEL_BOOST_CFG_ON;

	si_parse_haptic_type(sih_haptic, type);

	mutex_unlock(&sih_haptic->lock);

	return len;
}

static const struct file_operations fops = {
	.owner = THIS_MODULE,
	.read = si_file_read,
	.write = si_file_write,
	.unlocked_ioctl = si_file_unlocked_ioctl,
	.open = si_file_open,
	.release = si_file_release,
};

static struct miscdevice si_haptic_misc = {
	.minor = MISC_DYNAMIC_MINOR,
	.name = SI_HAPTIC_NAME,
	.fops = &fops,
};

static void vibrator_init(
	sih_haptic_t *sih_haptic)
{
	hrtimer_init(&sih_haptic->timer, CLOCK_MONOTONIC, HRTIMER_MODE_REL);
	sih_haptic->timer.function = haptic_timer_func;

	INIT_WORK(&sih_haptic->ram_work, ram_work_func);
	INIT_WORK(&sih_haptic->rtp_para.rtp_work, rtp_work_func);
	INIT_WORK(&sih_haptic->vibrator_work, vibrator_work_routine);
	INIT_WORK(&sih_haptic->rtp_para.rtp_data_work,
		rtp_data_work_func);

	mutex_init(&sih_haptic->lock);
	mutex_init(&sih_haptic->rtp_para.rtp_lock);

	INIT_DELAYED_WORK(&sih_haptic->delay_init_work, delay_init_work_func);
}

static void ram_delayed_work_func(
	struct work_struct *work)
{
	sih_haptic_t *sih_haptic = container_of(work,
		sih_haptic_t, ram_para.ram_delayed_work.work);

	hp_info("%s enter\n", __func__);

	sih_ram_update(sih_haptic);
}

static void ram_delayed_work_init(
	sih_haptic_t *sih_haptic)
{
	int ram_time_interval = SIH_RAM_TIME_DELAU_INTERVAL_MS * 1000;

	sih_haptic->sih_wq = create_singlethread_workqueue("sih_wq");

	INIT_DELAYED_WORK(&sih_haptic->ram_para.ram_delayed_work,
		ram_delayed_work_func);

	queue_delayed_work(sih_haptic->sih_wq, &sih_haptic->ram_para.ram_delayed_work,
		msecs_to_jiffies(ram_time_interval));

	hp_info("%s end\n", __func__);
}

static int sih_i2c_probe(
	struct i2c_client *i2c,
	const struct i2c_device_id *id)
{
	sih_haptic_t *sih_haptic;
	struct device_node *np = i2c->dev.of_node;
	int ret = -1;

	hp_info("%s:haptic i2c probe enter\n", __func__);
	/* I2C Adapter capability detection */
	if (!i2c_check_functionality(i2c->adapter, I2C_FUNC_I2C)) {
		hp_err("i2c algorithm ability detect failed\n");
		return -EIO;
	}

	/* Allocate cache space for haptic device */
	sih_haptic = devm_kzalloc(&i2c->dev, sizeof(sih_haptic_t), GFP_KERNEL);
	if (sih_haptic == NULL)
		return -ENOMEM;

	sih_haptic->dev = &i2c->dev;
	sih_haptic->i2c = i2c;
	i2c_set_clientdata(i2c, sih_haptic);

	/* matching dts */
	if (sih_parse_dts(&i2c->dev, sih_haptic, np))
		goto err_parse_dts;

	/* acquire gpio resources and read id code */
	if (sih_acquire_prepare_res(&i2c->dev, sih_haptic))
		goto err_prepare_res;

#ifdef SIH_PRIORITY_OPEN
	/* acquire_prioity */
	sih_register_priority(sih_haptic, np);
#endif

	/* Registers chip manipulation functions */
	if (sih_register_func(sih_haptic))
		goto err_id;

	/* registers regmap */
	sih_haptic->regmapp.regmapping = haptic_regmap_init(i2c,
		sih_haptic->regmapp.config);
	if (sih_haptic->regmapp.regmapping == NULL)
		goto err_regmap;

	/* handle gpio irq */
	if (sih_acquire_irq_res(&i2c->dev, sih_haptic))
		goto err_irq;

	g_haptic_t.sih_num = 1;

	g_haptic_t.g_haptic[SIH_HAPTIC_MMAP_DEV_INDEX] = sih_haptic;
	s_haptic = sih_haptic;

	/* vibrator parameter init */
	vibrator_parameter_init(sih_haptic);

	/* vibrator init */
	vibrator_init(sih_haptic);

	/* Schedule work queues and initialize haptic chip parameters */
	schedule_delayed_work(&sih_haptic->delay_init_work,
		msecs_to_jiffies(20));

	/* ram work struct schedule */
	ram_delayed_work_init(sih_haptic);

	/* add sysfs node */
	ret = sih_add_node(&i2c->dev, sih_haptic);
	if (ret)
		goto err_dev_sysfs;

	ret = misc_register(&si_haptic_misc);
	if (ret)
		pr_err("%s:misc fail:%d\n", __func__, ret);

	hp_info("%s end\n", __func__);
	return ret;

err_dev_sysfs:
	devm_free_irq(&i2c->dev, gpio_to_irq(sih_haptic->chip_attr.irq_gpio),
				sih_haptic);
err_irq:
err_regmap:
err_id:
	devm_gpio_free(&i2c->dev, sih_haptic->chip_attr.irq_gpio);
	devm_gpio_free(&i2c->dev, sih_haptic->chip_attr.reset_gpio);
err_prepare_res:
err_parse_dts:
	devm_kfree(&i2c->dev, sih_haptic);

	sih_haptic = NULL;

	return -1;
}

static int sih_i2c_remove(
	struct i2c_client *i2c)
{
	int ret = 0;

	sih_haptic_t *sih_haptic = i2c_get_clientdata(i2c);

	/* delayed_work release */
	cancel_delayed_work_sync(&sih_haptic->delay_init_work);
	cancel_delayed_work_sync(&sih_haptic->ram_para.ram_delayed_work);

	/* work_struct release */
	cancel_work_sync(&sih_haptic->ram_work);
	cancel_work_sync(&sih_haptic->rtp_para.rtp_work);
	cancel_work_sync(&sih_haptic->rtp_para.rtp_data_work);

	/*work_queue release*/
	destroy_workqueue(sih_haptic->sih_wq);

	/* hrtimer release */
	hrtimer_cancel(&sih_haptic->timer);

	/* mutex release */
	mutex_destroy(&sih_haptic->lock);
	mutex_destroy(&sih_haptic->rtp_para.rtp_lock);

	/* timed_output release */
#ifdef TIMED_OUTPUT
	timed_output_dev_unregister(&sih_haptic->soft_frame.vib_dev);
#endif

	/* irq release */
	devm_free_irq(&i2c->dev,
		gpio_to_irq(sih_haptic->chip_attr.irq_gpio), sih_haptic);

	/* gpio release */
	if (gpio_is_valid(sih_haptic->chip_attr.irq_gpio))
		devm_gpio_free(&i2c->dev, sih_haptic->chip_attr.irq_gpio);
	if (gpio_is_valid(sih_haptic->chip_attr.reset_gpio))
		devm_gpio_free(&i2c->dev, sih_haptic->chip_attr.reset_gpio);

	/* regmap exit */
	haptic_regmap_remove(sih_haptic->regmapp.regmapping);

	/* waveform release */

	/* container release */
	sih_vfree_container(sih_haptic, sih_haptic->rtp_para.rtp_cont);

	kfree(sih_haptic->detect_para.detect_fifo);

	kfree(sih_haptic->detect_para.detect_bemf_value);
	kfree(sih_haptic->detect_para.detect_f0_value);
	/* reg addr release */
	if (sih_haptic->chip_reg.reg_addr != NULL)
		kfree(sih_haptic->chip_reg.reg_addr);

	misc_deregister(&si_haptic_misc);
	return ret;
}


static int sih_suspend(
	struct device *dev)
{
	int ret = 0;
	sih_haptic_t *sih_haptic = dev_get_drvdata(dev);

	mutex_lock(&sih_haptic->lock);
	/* chip stop */
	mutex_unlock(&sih_haptic->lock);

	return ret;
}

static int sih_resume(
	struct device *dev)
{
	int ret = 0;

	pr_info("%s extected\n", __func__);

	return ret;
}

static SIMPLE_DEV_PM_OPS(sih_pm_ops, sih_suspend, sih_resume);

static const struct i2c_device_id sih_i2c_id[] = {
	{ SIH_HAPTIC_NAME_688X, 0 },
	{ }
};

static struct of_device_id sih_dt_match[] = {
	{ .compatible = SIH_HAPTIC_COMPAT_688X },
	{ },
};

struct i2c_driver sih_i2c_driver = {
	.driver = {
		.name = SIH_HAPTIC_NAME_688X,
		.owner = THIS_MODULE,
		.of_match_table = of_match_ptr(sih_dt_match),
		.pm = &sih_pm_ops,
	},
	.probe = sih_i2c_probe,
	.remove = sih_i2c_remove,
	.id_table = sih_i2c_id,
};

static int __init sih_i2c_init(void)
{
	int ret = 0;

	ret = i2c_add_driver(&sih_i2c_driver);

	pr_info("i2c_add_driver___add memo,ret = %d\n", ret);

	if (ret) {
		pr_err("fail to add haptic device into i2c\n");
		return ret;
	}
	return 0;
}

static void __exit sih_i2c_exit(
	void)
{
	i2c_del_driver(&sih_i2c_driver);
}

module_init(sih_i2c_init);
module_exit(sih_i2c_exit);

EXPORT_SYMBOL(sih_i2c_driver);
MODULE_DESCRIPTION("Haptic Driver V1.0.0.9762");
MODULE_LICENSE("GPL v2");
