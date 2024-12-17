/*
 * haptic_hv.c
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

#include <linux/delay.h>
#include <linux/firmware.h>
#include <linux/interrupt.h>
#include <linux/list.h>
#include <linux/of_gpio.h>

#include "haptic_core.h"
#include "haptic_misc.h"
#include "haptic_richtap.h"
#include "haptic_sysfs.h"

#ifdef AAC_RICHTAP_SUPPORT
#include <linux/mman.h>
#endif

#ifdef CONFIG_HUAWEI_POWER_EMBEDDED_ISOLATION
#include <linux/power_supply.h>
#include <chipset_common/hwpower/common_module/power_supply_interface.h>
#endif

#ifdef CONFIG_HUAWEI_DUBAI
#include <huawei_platform/log/hwlog_kernel.h>
#endif

#define HAPTIC_NAME "haptic"

struct haptic *dev_get_haptic(struct device *dev)
{
	cdev_t *cdev = NULL;
	struct haptic *haptic = NULL;

	if (!dev)
		return NULL;

	cdev = dev_get_drvdata(dev);
	if (!cdev)
		return NULL;

	haptic = container_of(cdev, struct haptic, vib_dev);

	return haptic;
}

static int parse_dts_gpio(struct haptic *haptic, struct device_node *np)
{
	int ret;
	uint32_t real_i2c_addr;

	// hardware or software power enable contrl
	ret = of_property_read_u32(np, "boost_fw", &haptic->info.boost_fw);
	if (ret != 0)
		vib_info("boost_fw not found");

	// hardware enable when boost_fw is ture
	if (haptic->info.boost_fw) {
		haptic->info.boost_en = of_get_named_gpio(np, "boost_en", 0);
		if (haptic->info.boost_en >= 0) {
			vib_info("boost en-%d ok", haptic->info.boost_en);
		} else {
			vib_err("get boost en gpio fail");
		}
	}

	haptic->reset_gpio = of_get_named_gpio(np, "reset-gpio", 0);
	if (haptic->reset_gpio < 0) {
		vib_err("no reset gpio provide");
		return -EINVAL;
	}
	vib_info("reset gpio provide ok %d", haptic->reset_gpio);

	haptic->irq_gpio = of_get_named_gpio(np, "irq-gpio", 0);
	if (haptic->irq_gpio < 0)
		vib_err("no irq gpio provided");
	else
		vib_info("irq gpio provide ok irq = %d", haptic->irq_gpio);

	ret = of_property_read_u8(np, "mode", &haptic->info.mode);
	if (ret)
		vib_info("mode not found");

	// boost enable or unable contrl
	ret = of_property_read_u32(np, "bst_enable", &haptic->bst_enable);
	if (ret)
		vib_info("bst_enable not found");
	vib_info("haptic->bst_enable = %d", haptic->bst_enable);

	// i2c addr override for multiple devices of same i2c address
	ret = of_property_read_u32(np, "i2c_real_addr", &real_i2c_addr);
	if (ret)
		vib_info("i2c_real_addr is not found");
	if (real_i2c_addr) {
		vib_info("i2c_real_addr is %d", real_i2c_addr);
		haptic->i2c->addr = (uint16_t)real_i2c_addr;
	}
	return 0;
}

static void ram_play(struct haptic *haptic, uint8_t mode)
{
	if (!haptic) {
		vib_err("haptic is null");
		return;
	}
	haptic->hops->play_mode(haptic, mode);
	haptic->hops->play_go(haptic, true);
}

int get_ram_num(struct haptic *haptic)
{
	uint8_t wave_addr[2] = { 0 };
	uint32_t first_wave_addr = 0;

	if (!haptic || !haptic->ram_inited) {
		vib_err("ram init faild, ram_num = 0!");
		return -EPERM;
	}
	mutex_lock(&haptic->lock);
	/* RAMINIT Enable */
	haptic->hops->ram_init(haptic, true);
	haptic->hops->play_stop(haptic);
	haptic->hops->set_ram_addr(haptic);
	haptic->hops->get_first_wave_addr(haptic, wave_addr);
	first_wave_addr = (wave_addr[0] << 8 | wave_addr[1]);
	haptic->ram.ram_num = (first_wave_addr - haptic->ram.base_addr - 1) / 4;
	vib_info("first wave addr = 0x%04x", first_wave_addr);
	vib_info("ram_num = %d", haptic->ram.ram_num);
	/* RAMINIT Disable */
	haptic->hops->ram_init(haptic, false);
	mutex_unlock(&haptic->lock);
	return 0;
}

static void ram_vbat_comp(struct haptic *haptic, bool flag)
{
	int temp_gain = 0;

	if (flag) {
		if (haptic->ram_vbat_comp == HAPTIC_RAM_VBAT_COMP_ENABLE) {
			haptic->hops->get_vbat(haptic);
			if (haptic->vbat > HAPTIC_VBAT_REFER) {
				vib_dbg("not need to vbat compensate!");
				return;
			}
			temp_gain =
				haptic->gain * HAPTIC_VBAT_REFER / haptic->vbat;
			if (temp_gain >
			    (128 * HAPTIC_VBAT_REFER / HAPTIC_VBAT_MIN)) {
				temp_gain = 128 * HAPTIC_VBAT_REFER /
					    HAPTIC_VBAT_MIN;
				vib_dbg("gain limit=%d", temp_gain);
			}
			haptic->hops->set_gain(haptic, temp_gain);
			vib_info("ram vbat comp open");
		} else {
			haptic->hops->set_gain(haptic, haptic->gain);
			vib_info(
				"ram vbat comp close(HAPTIC_RAM_VBAT_COMP_ENABLE failed)");
		}
	} else {
		haptic->hops->set_gain(haptic, haptic->gain);
		vib_info("ram vbat comp close(flag false)");
	}
}

void haptic_adapt_amp_again(struct haptic *haptic, int haptic_type)
{
	const unsigned char long_amp_again_val[LONG_MAX_AMP_CFG] = {
		0x80, 0x80, 0x70, 0x60, 0x50, 0x40, 0x30, 0x20, 0x10
	};
	const unsigned char short_amp_again_val[SHORT_MAX_AMP_CFG] = {
		0x80, 0x80, 0x66, 0x4c, 0x33, 0x19
	};

	if (!haptic) {
		vib_err("haptic is null");
		return;
	}
	switch (haptic_type) {
	case LONG_VIB_RAM_MODE:
		if (haptic->amplitude >= LONG_MAX_AMP_CFG ||
		    haptic->amplitude < 0)
			return;
		haptic->gain = long_amp_again_val[haptic->amplitude];
		vib_info("long gain = %u", haptic->gain);
		break;
	case SHORT_VIB_RAM_MODE:
		if (haptic->amplitude >= SHORT_MAX_AMP_CFG ||
		    haptic->amplitude < 0)
			return;
		haptic->gain = short_amp_again_val[haptic->amplitude];
		vib_info("short gain = %u", haptic->gain);
		break;
	case RTP_VIB_MODE:
		haptic->gain = 0x80; // define max amp
		break;
	default:
		break;
	}
}

void haptic_special_type_process(struct haptic *haptic, uint64_t type)
{
	if (!haptic)
		return;

	switch (type) {
	case HAPTIC_CHARGING_CALIB_ID:
		haptic->f0_is_cali = HAPTIC_CHARGING_IS_CALIB;
		schedule_work(&haptic->vibrator_work);
		break;
	case HAPTIC_NULL_WAVEFORM_ID:
		vib_info("null wave id");
		break;
	default:
		break;
	}
}

static enum hrtimer_restart vibrator_timer_func(struct hrtimer *timer)
{
	struct haptic *haptic = container_of(timer, struct haptic, timer);
	haptic->state = 0;
	schedule_work(&haptic->vibrator_work);
	return HRTIMER_NORESTART;
}

static void haptic_long_ram_mode_boost_config(struct haptic *haptic,
					      unsigned char wavseq)
{
	if (haptic->cust_boost_on == HAPTIC_SEL_BOOST_CFG_ON) {
		haptic->hops->set_wav_seq(haptic, 0, 0x14);
		haptic->hops->set_wav_loop(haptic, 0, 0); // loop mode
		haptic->hops->set_wav_seq(haptic, 1, wavseq);
		haptic->hops->set_wav_loop(haptic, 1, 0x0f);
		haptic->hops->set_wav_seq(haptic, 2, 0);
		haptic->hops->set_wav_loop(haptic, 2, 0);
	} else {
		haptic->hops->set_wav_seq(haptic, 0, wavseq);
		haptic->hops->set_wav_loop(haptic, 0, 0x0f);
		haptic->hops->set_wav_seq(haptic, 1, 0);
		haptic->hops->set_wav_loop(haptic, 1, 0);
	}
}

static int haptic_haptic_ram_config(struct haptic *haptic)
{
	unsigned char wavseq = 0;
	unsigned char wavloop = 0;
	unsigned char seq_idx = 0;

	vib_info("duration=%d,index=%d", haptic->duration, haptic->index);
	for (seq_idx = 0; seq_idx < 8; seq_idx++) {
		haptic->hops->set_wav_seq(haptic, seq_idx, 0);
		haptic->hops->set_wav_loop(haptic, seq_idx, 0);
	}
	if (haptic->effect_mode == SHORT_VIB_RAM_MODE) {
		wavseq = haptic->index;
		wavloop = 0;
		haptic->hops->set_wav_seq(haptic, 0, wavseq);
		haptic->hops->set_wav_loop(haptic, 0, wavloop);
		haptic->hops->set_wav_seq(haptic, 1, 0);
		haptic->hops->set_wav_loop(haptic, 1, 0);
		vib_info("waveseq = %s", wavseq);
	} else {
		wavseq = haptic->index;
		haptic_long_ram_mode_boost_config(haptic, wavseq);
	}
	return 0;
}

static int haptic_haptic_play_repeat_seq(struct haptic *haptic,
					 unsigned char flag)
{
	vib_info("enter");

	if (flag) {
		haptic->hops->play_mode(haptic, HAPTIC_RAM_LOOP_MODE);
		haptic->hops->play_go(haptic, true);
	}
	return 0;
}

#ifdef CONFIG_HUAWEI_POWER_EMBEDDED_ISOLATION
static void haptic_haptic_emergency_mode(struct haptic *haptic)
{
	int ret;
	int ele = 0;

	ret = power_supply_get_int_property_value(
		"battery", POWER_SUPPLY_PROP_CAPACITY, &ele);
	if (ret < 0) {
		vib_err("get capacity failed");
		return;
	}
	if (ele <= 1) {
		vib_info("enter!");
		haptic->emergency_mode_flag = true;
	}
}
#endif

static void haptic_boost_config(struct haptic *haptic)
{
	if (!haptic->info.is_enabled_lowpower_bst_config) {
		haptic->hops->bst_mode_config(haptic, HAPTIC_BST_BOOST_MODE);
		return;
	}
	if (haptic->emergency_mode_flag)
		haptic->hops->bst_mode_config(haptic, HAPTIC_BST_BYPASS_MODE);
	else
		haptic->hops->bst_mode_config(haptic, HAPTIC_BST_BOOST_MODE);
}

static void ram_mode_sel_boost_play_cfg(struct haptic *haptic)
{
#ifdef CONFIG_HUAWEI_POWER_EMBEDDED_ISOLATION
	if (haptic->info.is_enabled_lowpower_bst_config)
		haptic_haptic_emergency_mode(haptic);
#endif

	if (haptic->cust_boost_on == HAPTIC_SEL_BOOST_CFG_ON) {
		ram_vbat_comp(haptic, false);
		haptic_boost_config(haptic);
		haptic->hops->play_mode(haptic, HAPTIC_RAM_MODE);
		haptic->hops->play_go(haptic, true);
		return;
	}
	if (haptic->effect_mode == LONG_VIB_RAM_MODE) {
		ram_vbat_comp(haptic, true);
		haptic->hops->bst_mode_config(haptic, HAPTIC_BST_BYPASS_MODE);
		haptic_haptic_play_repeat_seq(haptic, true);
	} else {
		ram_vbat_comp(haptic, false);
		haptic->hops->bst_mode_config(haptic, HAPTIC_BST_BOOST_MODE);
		haptic->hops->play_mode(haptic, HAPTIC_RAM_MODE);
		haptic->hops->play_go(haptic, true);
	}
}

static void vibrator_work_routine(struct work_struct *work)
{
	struct haptic *haptic = container_of(work, struct haptic, vibrator_work);

	/* charging mode */
	mutex_lock(&haptic->lock);
	if (haptic->f0_is_cali == HAPTIC_CHARGING_IS_CALIB) {
		haptic->hops->upload_lra(haptic, HAPTIC_F0_CALI_LRA);
		haptic->hops->f0_cali(haptic);
		haptic->f0_is_cali = 0;
		mutex_unlock(&haptic->lock);
		return;
	}

	haptic->hops->upload_lra(haptic, HAPTIC_F0_CALI_LRA);
	/* Enter standby mode */
	haptic->hops->play_stop(haptic);

	if (haptic->state) {
		vib_info("state is true");
		if (haptic->activate_mode == HAPTIC_HAPTIC_ACTIVATE_RAM_MODE) {
			vib_info(
				"activate_mode == HAPTIC_HAPTIC_ACTIVATE_RAM_MODE");
			haptic_haptic_ram_config(haptic);
			if (haptic->duration <=
			    LONG_HAPTIC_RUNNING) { // max long vibra config
				vib_info(
					"duration <= HAPTIC_LONG_HAPTIC_RUNNINGe");
				ram_mode_sel_boost_play_cfg(haptic);
			} else {
				ram_vbat_comp(haptic, true);
				haptic_haptic_play_repeat_seq(haptic, true);
			}
		} else if (haptic->activate_mode ==
			   HAPTIC_HAPTIC_ACTIVATE_CONT_MODE) {
			vib_info(
				"activate_mode == HAPTIC_HAPTIC_ACTIVATE_CONT_MODE");
			haptic->hops->cont_config(haptic);
		} else {
			/* other mode */
			vib_err("set other mode");
		}
		/* run ms timer */
		if (haptic->effect_mode == LONG_VIB_RAM_MODE ||
		    (haptic->activate_mode ==
		     HAPTIC_HAPTIC_ACTIVATE_CONT_MODE)) {
			vib_info("run ms timer");
			hrtimer_start(
				&haptic->timer,
				ktime_set(haptic->duration / 1000,
					  (haptic->duration % 1000) * 1000000),
				HRTIMER_MODE_REL);
			__pm_stay_awake(haptic->ws);
			haptic->wk_lock_flag = 1;
			vib_info("gain = 0x%02X", haptic->gain);
#ifdef CONFIG_HUAWEI_DUBAI
			HWDUBAI_LOGE("DUBAI_TAG_HAPTIC_DURATION",
				     "duration=%d index=%d", haptic->duration,
				     haptic->index);
#endif
		}
	} else {
		vib_info("state is flase");
		if (haptic->wk_lock_flag == 1) {
			__pm_relax(haptic->ws);
			__pm_wakeup_event(haptic->ws, HAPTIC_WAKE_LOCK_GAP);
			haptic->wk_lock_flag = 0;
		}
	}
	mutex_unlock(&haptic->lock);
}

static void rtp_play(struct haptic *haptic)
{
	uint8_t glb_state_val = 0;
	uint32_t real_len = 10;

	vib_info("enter!");

	haptic->rtp_cnt = 0;
	mutex_lock(&haptic->rtp_lock);
	while ((!haptic->hops->rtp_get_fifo_afs(haptic)) &&
	       (haptic->play_mode == HAPTIC_RTP_MODE)) {
		if (!haptic->rtp) {
			vib_info("haptic->rtp is null, break!");
			break;
		}
		while ((haptic->rtp_cnt < haptic->rtp->len)) {
			if (real_len > (haptic->rtp->len - haptic->rtp_cnt))
				real_len = haptic->rtp->len - haptic->rtp_cnt;
			haptic->hops->set_rtp_data(
				haptic, &haptic->rtp->data[haptic->rtp_cnt],
				real_len);
			haptic->rtp_cnt += real_len;
		}
		glb_state_val = haptic->hops->get_glb_state(haptic);
		if ((haptic->rtp_cnt == haptic->rtp->len) ||
		    ((glb_state_val & HAPTIC_GLBRD_STATE_MASK) ==
		     HAPTIC_STATE_STANDBY)) {
			if (haptic->rtp_cnt != haptic->rtp->len)
				vib_err("rtp play suspend!");
			else
				vib_info("rtp update complete!");
			haptic->rtp_cnt = 0;
			break;
		}
	}
	if (haptic->play_mode == HAPTIC_RTP_MODE)
		haptic->hops->set_rtp_aei(haptic, true);
	mutex_unlock(&haptic->rtp_lock);
}

static void rtp_work_routine(struct work_struct *work)
{
	bool rtp_work_flag = false;
	uint8_t reg_val = 0;
	int cnt = 200;
	int ret = -1;
	const struct firmware *rtp_file = NULL;
	struct haptic *haptic = container_of(work, struct haptic, rtp_work);
	mutex_lock(&haptic->rtp_lock);
	haptic->rtp_routine_on = 1;
	/* fw loaded */
	ret = request_firmware(&rtp_file, haptic->rtp_name, haptic->dev);
	if (ret < 0) {
		vib_err("failed to read %s", haptic->rtp_name);
		haptic->rtp_routine_on = 0;
		mutex_unlock(&haptic->rtp_lock);
		return;
	}
	haptic->rtp_inited = false;
	vfree(haptic->rtp);
	haptic->rtp = vmalloc(rtp_file->size + sizeof(int));
	if (!haptic->rtp) {
		release_firmware(rtp_file);
		vib_err("error allocating memory");
		haptic->rtp_routine_on = 0;
		mutex_unlock(&haptic->rtp_lock);
		return;
	}
	haptic->rtp->len = rtp_file->size;
	vib_info("rtp file:[%s] size = %dbytes", haptic->rtp_name,
		 haptic->rtp->len);
	memcpy(haptic->rtp->data, rtp_file->data, rtp_file->size);
	mutex_unlock(&haptic->rtp_lock);
	release_firmware(rtp_file);
	mutex_lock(&haptic->lock);
	haptic->rtp_inited = true;

	haptic->hops->upload_lra(haptic, HAPTIC_OSC_CALI_LRA);
	haptic->hops->set_rtp_aei(haptic, false);
	haptic->hops->irq_clear(haptic);
	haptic->hops->play_stop(haptic);
	/* gain */
	ram_vbat_comp(haptic, false);
	/* boost voltage */
	if (haptic->info.bst_vol_rtp <= haptic->info.max_bst_vol &&
	    haptic->info.bst_vol_rtp > 0)
		haptic->hops->set_bst_vol(haptic, haptic->info.bst_vol_rtp);
	else
		haptic->hops->set_bst_vol(haptic, haptic->vmax);
	/* rtp mode config */
	haptic->hops->play_mode(haptic, HAPTIC_RTP_MODE);
	/* haptic go */
	haptic->hops->play_go(haptic, true);
	usleep_range(2000, 2500);
	while (cnt) {
		reg_val = haptic->hops->get_glb_state(haptic);
		if ((reg_val & HAPTIC_GLBRD_STATE_MASK) == HAPTIC_STATE_RTP) {
			cnt = 0;
			rtp_work_flag = true;
			vib_info("RTP_GO! glb_state=0x08");
		} else {
			cnt--;
			vib_dbg("wait for RTP_GO, glb_state=0x%02X", reg_val);
		}
		usleep_range(2000, 2500);
	}
	if (rtp_work_flag) {
		vib_info("rtp_work_flag is ture");
		rtp_play(haptic);
	} else {
		/* enter standby mode */
		haptic->hops->play_stop(haptic);
		vib_err("failed to enter RTP_GO status!");
	}
	haptic->rtp_routine_on = 0;
	mutex_unlock(&haptic->lock);
}

static irqreturn_t irq_handle(int irq, void *data)
{
	uint8_t glb_state_val = 0;
	uint32_t buf_len = 0;
	struct haptic *haptic = NULL;

	if (!data) {
		vib_err("data is null");
		return IRQ_NONE;
	}
	haptic = data;

#ifdef AAC_RICHTAP_SUPPORT
	if (atomic_read(&haptic->richtap_rtp_mode) == true)
		return irq_richtap_aei_handle(haptic);
#endif

	if (!haptic->hops->get_irq_state(haptic)) {
		vib_dbg("haptic rtp fifo almost empty");
		if (haptic->rtp_inited) {
			while ((!haptic->hops->rtp_get_fifo_afs(haptic)) &&
			       (haptic->play_mode == HAPTIC_RTP_MODE)) {
				mutex_lock(&haptic->rtp_lock);
				if (!haptic->rtp_cnt) {
					vib_info("haptic->rtp_cnt is 0!");
					mutex_unlock(&haptic->rtp_lock);
					break;
				}
				if (!haptic->rtp) {
					vib_info("haptic->rtp is null, break!");
					mutex_unlock(&haptic->rtp_lock);
					break;
				}
				if ((haptic->rtp->len - haptic->rtp_cnt) <
				    (haptic->ram.base_addr >> 2)) {
					buf_len = haptic->rtp->len -
						  haptic->rtp_cnt;
				} else {
					buf_len = (haptic->ram.base_addr >> 2);
				}
				haptic->hops->set_rtp_data(
					haptic,
					&haptic->rtp->data[haptic->rtp_cnt],
					buf_len);
				haptic->rtp_cnt += buf_len;
				glb_state_val =
					haptic->hops->get_glb_state(haptic);
				if ((haptic->rtp_cnt == haptic->rtp->len) ||
				    ((glb_state_val & HAPTIC_GLBRD_STATE_MASK) ==
				     HAPTIC_STATE_STANDBY)) {
					if (haptic->rtp_cnt != haptic->rtp->len)
						vib_err("rtp play suspend!");
					else
						vib_info(
							"rtp update complete!");
					haptic->rtp_routine_on = 0;
					haptic->hops->set_rtp_aei(haptic,
								  false);
					haptic->rtp_cnt = 0;
					haptic->rtp_inited = false;
					mutex_unlock(&haptic->rtp_lock);
					break;
				}
				mutex_unlock(&haptic->rtp_lock);
			}
		} else {
			vib_info("init error");
		}
	}
	if (haptic->play_mode != HAPTIC_RTP_MODE)
		haptic->hops->set_rtp_aei(haptic, false);
	return IRQ_HANDLED;
}

static void ram_load(const struct firmware *cont, void *context)
{
	uint16_t check_sum = 0;
	int i = 0;
	int ret = 0;
	struct haptic *haptic = NULL;
	struct haptic_container *haptic_fw;

#ifdef HAPTIC_READ_BIN_FLEXBALLY
	static uint8_t load_cont;
	int ram_timer_val = 1000;

	load_cont++;
#endif
	if (!context) {
		vib_err("context is null");
		return;
	}
	haptic = context;
	if (!cont) {
		vib_err("failed to read %s", haptic->hops->get_ram_name());
		release_firmware(cont);
#ifdef HAPTIC_READ_BIN_FLEXBALLY
		if (load_cont <= 1) {
			schedule_delayed_work(&haptic->ram_work,
					      msecs_to_jiffies(ram_timer_val));
			vib_info("start hrtimer:load_cont%d", load_cont);
		}
#endif
		return;
	}
	vib_info("loaded %s - size: %zu", haptic->hops->get_ram_name(),
		 cont ? cont->size : 0);
	/* check sum */
	for (i = 2; i < cont->size; i++)
		check_sum += cont->data[i];
	if (check_sum != (uint16_t)((cont->data[0] << 8) | (cont->data[1]))) {
		vib_err("check sum err: check_sum=0x%04x", check_sum);
		release_firmware(cont);
		return;
	}
	vib_info("check sum pass : 0x%04x", check_sum);
	haptic->ram.check_sum = check_sum;

	/* aw ram update */
	vib_info("befor kzalloc");
	haptic_fw = kzalloc(cont->size + sizeof(int), GFP_KERNEL);
	if (!haptic_fw) {
		release_firmware(cont);
		vib_err("Error allocating memory");
		return;
	}
	vib_info("kzalloc success");
	haptic_fw->len = cont->size;
	vib_info("ready to memcpy");
	memcpy(haptic_fw->data, cont->data, cont->size);
	vib_info("ready to release_firmware");
	release_firmware(cont);
	vib_info("ready to container_update");
	ret = haptic->hops->container_update(haptic, haptic_fw);
	if (ret) {
		vib_err("ram firmware update failed!");
	} else {
		haptic->ram_inited = true;
		haptic->ram.len = haptic_fw->len - haptic->ram.ram_shift;
		haptic->hops->trig_init(haptic);
		vib_info("ram firmware update complete!");
		get_ram_num(haptic);
	}
	kfree(haptic_fw);
}

int ram_update(struct haptic *haptic)
{
	int ret;
	if (!haptic) {
		vib_err("haptic is null");
		return -EINVAL;
	}
	haptic->ram_inited = false;
	haptic->rtp_inited = false;
	ret = request_firmware_nowait(THIS_MODULE, FW_ACTION_HOTPLUG,
				      haptic->hops->get_ram_name(), haptic->dev,
				      GFP_KERNEL, haptic, ram_load);
	if (ret < 0)
		vib_info("request_firmware_nowait failed, ret = %d", ret);
	return ret;
}

static void ram_work_routine(struct work_struct *work)
{
	struct haptic *haptic = container_of(work, struct haptic, ram_work.work);

	ram_update(haptic);
}

static void ram_work_init(struct haptic *haptic)
{
	int ram_timer_val = HAPTIC_RAM_WORK_DELAY_INTERVAL;

	INIT_DELAYED_WORK(&haptic->ram_work, ram_work_routine);
	schedule_delayed_work(&haptic->ram_work,
			      msecs_to_jiffies(ram_timer_val));
}

int haptic_play_effect_type(struct haptic *haptic, int type)
{
	int rtp_len = 0;

	if (!haptic)
		return -EINVAL;

	mutex_lock(&haptic->lock);

	haptic->emergency_mode_flag = false;
	haptic->cust_boost_on = HAPTIC_SEL_BOOST_CFG_ON;
	if ((type < HAPTIC_MAX_SPEC_CMD_ID) &&
	    (type > HAPTIC_MIN_SPEC_CMD_ID)) {
		haptic_special_type_process(haptic, type);
		mutex_unlock(&haptic->lock);
		return -1;
	}

	haptic->state = 1;
	if (type > LONG_HAPTIC_RTP_MAX_ID) { // long time
		haptic->effect_mode = LONG_VIB_RAM_MODE;
		haptic->index = LONG_VIB_EFFECT_ID;
		haptic->duration = type / LONG_TIME_AMP_DIV_COFF;
		haptic->amplitude = type % LONG_TIME_AMP_DIV_COFF;

		haptic_adapt_amp_again(haptic, LONG_VIB_RAM_MODE);
		vib_info("long index = %d, amp = %d", haptic->index,
			 haptic->amplitude);
		schedule_work(&haptic->vibrator_work);
	} else if ((type > 0) &&
		   (type <= SHORT_HAPTIC_RAM_MAX_ID)) { // short ram haptic
		haptic->effect_mode = SHORT_VIB_RAM_MODE;
		haptic->amplitude = type % SHORT_HAPTIC_AMP_DIV_COFF;
		haptic->index = type / SHORT_HAPTIC_AMP_DIV_COFF;
		haptic_adapt_amp_again(haptic, SHORT_VIB_RAM_MODE);
		vib_info("short index = %d, amp = %d", haptic->index,
			 haptic->amplitude);
		schedule_work(&haptic->vibrator_work);
	} else { // long and short rtp haptic
		haptic->effect_mode = RTP_VIB_MODE;
		haptic->amplitude = type % SHORT_HAPTIC_AMP_DIV_COFF;
		haptic->index = type / SHORT_HAPTIC_AMP_DIV_COFF;
		haptic->hops->play_stop(haptic);
		haptic->hops->set_rtp_aei(haptic, false);
		haptic->hops->irq_clear(haptic);
		haptic_adapt_amp_again(haptic, RTP_VIB_MODE);
		haptic->rtp_idx = haptic->index - BASE_INDEX;
		rtp_len += snprintf(haptic->rtp_name, HAPTIC_NAME_MAX - 1,
				    "haptic_%d.bin", haptic->rtp_idx);
		vib_info("get rtp name = %s, index = %d, len = %d",
			 haptic->rtp_name, haptic->rtp_idx, rtp_len);
		schedule_work(&haptic->rtp_work);
	}

	mutex_unlock(&haptic->lock);
	return 0;
}

static enum led_brightness brightness_get(struct led_classdev *cdev)
{
	struct haptic *haptic = container_of(cdev, struct haptic, vib_dev);

	return haptic->amplitude;
}

static void brightness_set(struct led_classdev *cdev, enum led_brightness level)
{
	struct haptic *haptic = container_of(cdev, struct haptic, vib_dev);

	vib_info("enter");
	if (!haptic->ram_inited) {
		vib_err("ram init failed, not allow to play!");
		return;
	}
	haptic->amplitude = level;
	mutex_lock(&haptic->lock);
	haptic->hops->play_stop(haptic);
	if (haptic->amplitude > 0) {
		haptic->hops->upload_lra(haptic, HAPTIC_F0_CALI_LRA);
		ram_vbat_comp(haptic, false);
		ram_play(haptic, HAPTIC_RAM_MODE);
	}
	mutex_unlock(&haptic->lock);
}

static int vibrator_init(struct haptic *haptic)
{
	int ret;

	vib_info("loaded in leds_cdev framework!");
	haptic->vib_dev.name = "vibrator";
	haptic->vib_dev.brightness_get = brightness_get;
	haptic->vib_dev.brightness_set = brightness_set;
	haptic->vib_dev.default_trigger = "hw_vb_trigger";
	ret = devm_led_classdev_register(&haptic->i2c->dev, &haptic->vib_dev);
	if (ret < 0) {
		vib_err("fail to create led dev");
		return ret;
	}

	ret = haptic_create_sysfs_nodes(haptic);
	if (ret < 0) {
		vib_err("fail to create sysfs nodes");
		return ret;
	}

	hrtimer_init(&haptic->timer, CLOCK_MONOTONIC, HRTIMER_MODE_REL);
	haptic->timer.function = vibrator_timer_func;
	INIT_WORK(&haptic->vibrator_work, vibrator_work_routine);
	INIT_WORK(&haptic->rtp_work, rtp_work_routine);
	mutex_init(&haptic->lock);
	mutex_init(&haptic->rtp_lock);

	return 0;
}

static int haptic_init(struct haptic *haptic)
{
	int ret;

	vib_info("enter");
	ret = haptic_register_misc_dev(haptic);
	if (ret) {
		vib_err("reg misc dev fail");
		return ret;
	}

	/* haptic init */
	mutex_lock(&haptic->lock);
	haptic->rtp_routine_on = 0;
	haptic->activate_mode = haptic->info.mode;
	haptic->hops->play_mode(haptic, HAPTIC_STANDBY_MODE);
	haptic->hops->set_pwm(haptic, HAPTIC_PWM_12K);
	/* misc value init */
	haptic->hops->misc_para_init(haptic);

	haptic->hops->set_bst_peak_cur(haptic);
	haptic->hops->set_bst_vol(haptic, haptic->vmax);
	haptic->hops->protect_config(haptic, 0x01, 0x00);
	haptic->hops->auto_bst_enable(haptic, haptic->info.is_enabled_auto_bst);
	haptic->hops->offset_cali(haptic);
	/* vbat compensation */
	haptic->hops->vbat_mode_config(haptic, HAPTIC_CONT_VBAT_HW_COMP_MODE);
	haptic->ram_vbat_comp = HAPTIC_RAM_VBAT_COMP_ENABLE;

	mutex_unlock(&haptic->lock);

	/* f0 calibration */
	mutex_lock(&haptic->lock);
	haptic->hops->f0_cali(haptic);
	mutex_unlock(&haptic->lock);

	return 0;
}

static LIST_HEAD(haptic_ops_list);
int register_haptic_ops(struct haptic_ops *hops)
{
	if (!hops) {
		vib_err("hops is null");
		return -EINVAL;
	}
	list_add_tail(&hops->list, &haptic_ops_list);
	return 0;
}
EXPORT_SYMBOL(register_haptic_ops);

static int detect_chip(struct haptic *haptic)
{
	struct haptic_ops *hops;

	list_for_each_entry (hops, &haptic_ops_list, list) {
		if (!hops->read_chipid)
			continue;
		if (hops->read_chipid(haptic) != 0)
			continue;
		haptic->hops = hops;
		vib_info("detect chip:%x", haptic->chipid);
		return 0;
	}
	vib_err("failed to detect chip");
	return -ENODEV;
}

static int parse_dts(struct haptic *haptic)
{
	int ret;
	struct device_node *np = haptic->i2c->dev.of_node;

	if (!np)
		return -EINVAL;

	ret = parse_dts_gpio(haptic, np);
	if (ret) {
		vib_err("failed to parse gpio");
		return ret;
	}

	haptic->hops->parse_dt(haptic, np);

	vib_info("parse dts finished");
	return 0;
}

static int config_gpio_irq(struct haptic *haptic)
{
	int ret = -1;
	int irq_flags = 0;

	if (haptic->info.boost_fw) {
		if (gpio_is_valid(haptic->info.boost_en)) {
			ret = devm_gpio_request_one(&haptic->i2c->dev,
						    haptic->info.boost_en,
						    GPIOF_OUT_INIT_HIGH,
						    "haptic_boost_en");
			if (ret) {
				vib_err("boost en gpio request failed");
				return ret;
			}
			gpio_set_value_cansleep(haptic->info.boost_en, 1);
			usleep_range(3500, 4000);
		}
	}

	if (gpio_is_valid(haptic->reset_gpio)) {
		ret = devm_gpio_request_one(&haptic->i2c->dev,
					    haptic->reset_gpio,
					    GPIOF_OUT_INIT_LOW, "haptic_rst");
		if (ret) {
			vib_err("rst request failed");
			return ret;
		}
	}

	if (gpio_is_valid(haptic->irq_gpio)) {
		ret = devm_gpio_request_one(&haptic->i2c->dev, haptic->irq_gpio,
					    GPIOF_DIR_IN, "haptic_int");
		if (ret) {
			vib_err("int request failed");
			return ret;
		}

		/* register irq handler */
		haptic->hops->interrupt_setup(haptic);
		irq_flags = IRQF_TRIGGER_FALLING | IRQF_ONESHOT;
		ret = devm_request_threaded_irq(haptic->dev,
						gpio_to_irq(haptic->irq_gpio),
						NULL, irq_handle, irq_flags,
						"haptic", haptic);
		if (ret) {
			vib_err("failed to request IRQ %d: %d",
				gpio_to_irq(haptic->irq_gpio), ret);
			return ret;
		}
	}
	return 0;
}

static int register_vibrator_wakeup_source(struct haptic *haptic)
{
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(5, 4, 0))
	haptic->ws = wakeup_source_register(haptic->dev, "haptic");
#else
	haptic->ws = wakeup_source_register("haptic");
#endif

	if (!haptic->ws) {
		vib_err("register wakeup source failed");
		return -EINVAL;
	}

	vib_info("register haptic as wakeup source");
	return 0;
}

static int chip_rest_init(struct haptic *haptic)
{
	int ret;

#ifdef HAPTIC_CHECK_QUALIFY
	ret = haptic->hops->check_qualify(haptic);
	if (ret < 0) {
		vib_err("qualify check failed ret=%d", ret);
		return ret;
	}
#endif

	haptic->hops->creat_node(haptic);

	if (!haptic->hops->get_ram_name && !haptic->hops->get_ram_name()) {
		vib_err("chip ram name not config");
		return -EFAULT;
	}

	return 0;
}

int hwvibrator_init(struct haptic *haptic)
{
	int ret;

	if (!haptic)
		return -EINVAL;

	ret = detect_chip(haptic);
	if (ret)
		return ret;

	ret = parse_dts(haptic);
	if (ret)
		return ret;

	ret = config_gpio_irq(haptic);
	if (ret)
		return ret;

	ret = vibrator_init(haptic);
	if (ret)
		goto error_vibrator;

	ret = haptic_init(haptic);
	if (ret)
		goto error_haptic;

#ifdef AAC_RICHTAP_SUPPORT
	ret = richtap_rtp_config_init(haptic);
	if (ret)
		return ret;
#endif

	ret = register_vibrator_wakeup_source(haptic);
	if (ret) {
		goto error_wakeup_source;
	}

	ret = chip_rest_init(haptic);
	if (ret) {
		goto error_chip;
	}

	ram_work_init(haptic);
	return 0;

error_wakeup_source:
error_chip:
	haptic_unregister_misc_dev();
error_haptic:
	haptic_remove_sysfs_nodes(haptic);
error_vibrator:
	return -ENODEV;
}

int hwvibrator_deinit(struct haptic *haptic)
{
	if (!haptic)
		return -EINVAL;

	cancel_delayed_work_sync(&haptic->ram_work);
	cancel_work_sync(&haptic->rtp_work);
	cancel_work_sync(&haptic->vibrator_work);

	hrtimer_cancel(&haptic->timer);

	mutex_destroy(&haptic->lock);
	mutex_destroy(&haptic->rtp_lock);

	haptic_unregister_misc_dev();

	return 0;
}