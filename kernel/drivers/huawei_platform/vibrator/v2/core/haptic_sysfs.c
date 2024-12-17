/*
 * haptic_sysfs.c
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

#include <linux/device.h>

#include "haptic_core.h"

static ssize_t state_show(struct device *dev, struct device_attribute *attr,
			  char *buf)
{
	struct haptic *haptic = dev_get_haptic(dev);

	if (!haptic || !buf)
		return -EINVAL;

	return snprintf(buf, PAGE_SIZE, "state = %d\n", haptic->state);
}

static ssize_t duration_show(struct device *dev, struct device_attribute *attr,
			     char *buf)
{
	struct haptic *haptic = dev_get_haptic(dev);
	ktime_t time_rem;
	s64 time_ms = 0;

	if (!haptic || !buf)
		return -EINVAL;

	if (hrtimer_active(&haptic->timer)) {
		time_rem = hrtimer_get_remaining(&haptic->timer);
		time_ms = ktime_to_ms(time_rem);
	}
	return snprintf(buf, PAGE_SIZE, "duration = %lldms\n", time_ms);
}

static ssize_t duration_store(struct device *dev, struct device_attribute *attr,
			      const char *buf, size_t count)
{
	struct haptic *haptic = dev_get_haptic(dev);
	uint32_t val = 0;
	int ret;

	if (!haptic || !buf)
		return -EINVAL;

	ret = kstrtouint(buf, 0, &val);
	if (ret < 0)
		return ret;

	/* setting 0 on duration is NOP for now */
	if (val <= 0)
		return -EINVAL;

	vib_info("value=%u", val);

	mutex_lock(&haptic->lock);
	haptic->duration = val;
	haptic->cust_boost_on = 0;
	mutex_unlock(&haptic->lock);

	return count;
}

static ssize_t activate_show(struct device *dev, struct device_attribute *attr,
			     char *buf)
{
	struct haptic *haptic = dev_get_haptic(dev);

	if (!haptic || !buf)
		return -EINVAL;

	return snprintf(buf, PAGE_SIZE, "activate = %d\n", haptic->state);
}

static ssize_t activate_store(struct device *dev, struct device_attribute *attr,
			      const char *buf, size_t count)
{
	struct haptic *haptic = dev_get_haptic(dev);
	uint32_t val = 0;
	int ret;

	if (!haptic || !buf)
		return -EINVAL;

	ret = kstrtouint(buf, 0, &val);
	if (ret < 0)
		return ret;

	vib_info("value=%u", val);
	if (!haptic->ram_inited) {
		vib_err("ram init failed, not allow to play!");
		return count;
	}
	mutex_lock(&haptic->lock);
	if (hrtimer_active(&haptic->timer))
		hrtimer_cancel(&haptic->timer);
	haptic->state = val;
	haptic->effect_mode = LONG_VIB_RAM_MODE;
	haptic->index = LONG_VIB_BOOST_EFFECT_ID;
	mutex_unlock(&haptic->lock);
	schedule_work(&haptic->vibrator_work);

	return count;
}

static ssize_t activate_mode_show(struct device *dev,
				  struct device_attribute *attr, char *buf)
{
	struct haptic *haptic = dev_get_haptic(dev);

	if (!haptic || !buf)
		return -EINVAL;

	return snprintf(buf, PAGE_SIZE, "activate_mode = %d\n",
			haptic->activate_mode);
}

static ssize_t activate_mode_store(struct device *dev,
				   struct device_attribute *attr,
				   const char *buf, size_t count)
{
	struct haptic *haptic = dev_get_haptic(dev);
	uint32_t val = 0;
	int ret;

	if (!haptic || !buf)
		return -EINVAL;

	ret = kstrtouint(buf, 0, &val);
	if (ret < 0)
		return ret;
	vib_info("activate_mode = %u", val);
	mutex_lock(&haptic->lock);
	haptic->activate_mode = val;
	mutex_unlock(&haptic->lock);
	return count;
}

static ssize_t index_show(struct device *dev, struct device_attribute *attr,
			  char *buf)
{
	struct haptic *haptic = dev_get_haptic(dev);

	if (!haptic || !buf)
		return -EINVAL;

	haptic->hops->get_wav_seq(haptic, 1);
	haptic->index = haptic->seq[0];
	return snprintf(buf, PAGE_SIZE, "index = %d\n", haptic->index);
}

static ssize_t index_store(struct device *dev, struct device_attribute *attr,
			   const char *buf, size_t count)
{
	struct haptic *haptic = dev_get_haptic(dev);
	uint32_t val = 0;
	int ret;

	if (!haptic || !buf)
		return -EINVAL;

	ret = kstrtouint(buf, 0, &val);
	if (ret < 0)
		return ret;
	if (val > haptic->ram.ram_num) {
		vib_err("input value out of range!");
		return count;
	}
	vib_info("value=%u", val);
	mutex_lock(&haptic->lock);
	haptic->index = val;
	haptic->hops->set_repeat_seq(haptic, haptic->index);
	mutex_unlock(&haptic->lock);
	return count;
}

static ssize_t vmax_show(struct device *dev, struct device_attribute *attr,
			 char *buf)
{
	struct haptic *haptic = dev_get_haptic(dev);

	if (!haptic || !buf)
		return -EINVAL;

	return snprintf(buf, PAGE_SIZE, "vmax = 0x%02X\n", haptic->vmax);
}

static ssize_t vmax_store(struct device *dev, struct device_attribute *attr,
			  const char *buf, size_t count)
{
	struct haptic *haptic = dev_get_haptic(dev);
	uint32_t val = 0;
	int ret;

	if (!haptic || !buf)
		return -EINVAL;

	ret = kstrtouint(buf, 0, &val);
	if (ret < 0)
		return ret;
	vib_info("value=%u", val);
	mutex_lock(&haptic->lock);
	haptic->vmax = val;
	haptic->hops->set_bst_vol(haptic, haptic->vmax);
	mutex_unlock(&haptic->lock);
	return count;
}

static ssize_t gain_show(struct device *dev, struct device_attribute *attr,
			 char *buf)
{
	struct haptic *haptic = dev_get_haptic(dev);

	if (!haptic || !buf)
		return -EINVAL;

	return snprintf(buf, PAGE_SIZE, "gain = 0x%02X\n", haptic->gain);
}

static ssize_t gain_store(struct device *dev, struct device_attribute *attr,
			  const char *buf, size_t count)
{
	struct haptic *haptic = dev_get_haptic(dev);
	uint32_t val = 0;
	int ret;

	if (!haptic || !buf)
		return -EINVAL;

	ret = kstrtouint(buf, 0, &val);
	if (ret < 0)
		return ret;
	vib_info("value=0x%02x", val);
	mutex_lock(&haptic->lock);
	haptic->gain = val;
	haptic->hops->set_gain(haptic, haptic->gain);
	mutex_unlock(&haptic->lock);
	return count;
}

static ssize_t seq_show(struct device *dev, struct device_attribute *attr,
			char *buf)
{
	uint8_t i = 0;
	size_t count = 0;
	struct haptic *haptic = dev_get_haptic(dev);

	if (!haptic || !buf)
		return -EINVAL;

	haptic->hops->get_wav_seq(haptic, HAPTIC_SEQUENCER_SIZE);
	for (i = 0; i < HAPTIC_SEQUENCER_SIZE; i++)
		count += snprintf(buf + count, PAGE_SIZE - count,
				  "seq%d = %d\n", i + 1, haptic->seq[i]);
	return count;
}

static ssize_t seq_store(struct device *dev, struct device_attribute *attr,
			 const char *buf, size_t count)
{
	struct haptic *haptic = dev_get_haptic(dev);
	uint32_t databuf[2] = { 0, 0 };

	if (!haptic || !buf)
		return -EINVAL;

	if (sscanf(buf, "%x %x", &databuf[0], &databuf[1]) == 2) {
		if (databuf[0] >= HAPTIC_SEQUENCER_SIZE ||
		    databuf[1] > haptic->ram.ram_num) {
			vib_err("input value out of range!");
			return count;
		}
		vib_info("seq%d=0x%02X", databuf[0], databuf[1]);
		mutex_lock(&haptic->lock);
		haptic->seq[databuf[0]] = (uint8_t)databuf[1];
		haptic->hops->set_wav_seq(haptic, (uint8_t)databuf[0],
					  haptic->seq[databuf[0]]);
		mutex_unlock(&haptic->lock);
	}
	return count;
}

static ssize_t loop_show(struct device *dev, struct device_attribute *attr,
			 char *buf)
{
	size_t count = 0;

	struct haptic *haptic = dev_get_haptic(dev);

	if (!haptic || !buf)
		return -EINVAL;

	count = haptic->hops->get_wav_loop(haptic, buf);
	return count;
}

static ssize_t loop_store(struct device *dev, struct device_attribute *attr,
			  const char *buf, size_t count)
{
	struct haptic *haptic = dev_get_haptic(dev);
	uint32_t databuf[2] = { 0, 0 };

	if (!haptic || !buf)
		return -EINVAL;

	if (sscanf(buf, "%x %x", &databuf[0], &databuf[1]) == 2) {
		vib_info("seq%d loop=0x%02X", databuf[0], databuf[1]);
		mutex_lock(&haptic->lock);
		haptic->loop[databuf[0]] = (uint8_t)databuf[1];
		haptic->hops->set_wav_loop(haptic, (uint8_t)databuf[0],
					   haptic->loop[databuf[0]]);
		mutex_unlock(&haptic->lock);
	}

	return count;
}

static ssize_t reg_show(struct device *dev, struct device_attribute *attr,
			char *buf)
{
	struct haptic *haptic = dev_get_haptic(dev);
	ssize_t len = 0;

	if (!haptic || !buf)
		return -EINVAL;

	len = haptic->hops->get_reg(haptic, len, buf);
	return len;
}

static ssize_t reg_store(struct device *dev, struct device_attribute *attr,
			 const char *buf, size_t count)
{
	uint8_t reg = 0;
	uint8_t val = 0;
	ssize_t len;
	struct haptic *haptic = dev_get_haptic(dev);

	if (!haptic || !buf)
		return -EINVAL;

	/* get input for 2 input: reg addr and value */
	if (sscanf(buf, "%x %x", &reg, &val) == 2) {
		len = haptic->hops->set_reg(haptic, reg, val);
		return len;
	}
	return count;
}

static ssize_t rtp_show(struct device *dev, struct device_attribute *attr,
			char *buf)
{
	struct haptic *haptic = dev_get_haptic(dev);
	ssize_t len = 0;

	if (!haptic || !buf)
		return -EINVAL;

	len += snprintf(buf + len, PAGE_SIZE - len, "rtp_cnt = %d\n",
			haptic->rtp_cnt);
	return len;
}

static ssize_t rtp_sotre(struct device *dev, struct device_attribute *attr,
			 const char *buf, size_t count)
{
	struct haptic *haptic = dev_get_haptic(dev);
	uint32_t val = 0;
	int ret;
	int len = 0;

	if (!haptic || !buf)
		return -EINVAL;

	ret = kstrtouint(buf, 0, &val);
	if (ret < 0) {
		vib_err("kstrtouint fail");
		return ret;
	}
	mutex_lock(&haptic->lock);
	haptic->hops->play_stop(haptic);
	haptic->hops->set_rtp_aei(haptic, false);
	haptic->hops->irq_clear(haptic);

	len += snprintf(haptic->rtp_name, HAPTIC_NAME_MAX - 1, "aw_%u.bin",
			val);
	vib_info(" get rtp name = %s, len = %d", haptic->rtp_name, len);

	schedule_work(&haptic->rtp_work);
	mutex_unlock(&haptic->lock);
	return count;
}

static ssize_t ram_update_show(struct device *dev,
			       struct device_attribute *attr, char *buf)
{
	struct haptic *haptic = dev_get_haptic(dev);
	ssize_t len = 0;

	if (!haptic || !buf)
		return -EINVAL;

	/* RAMINIT Enable */
	haptic->hops->ram_init(haptic, true);
	haptic->hops->play_stop(haptic);
	haptic->hops->set_ram_addr(haptic);
	len += snprintf(buf + len, PAGE_SIZE - len, "aw_haptic_ram:\n");
	len += haptic->hops->get_ram_data(haptic, buf);
	len += snprintf(buf + len, PAGE_SIZE - len, "\n");
	/* RAMINIT Disable */
	haptic->hops->ram_init(haptic, false);
	return len;
}

static ssize_t ram_update_store(struct device *dev,
				struct device_attribute *attr, const char *buf,
				size_t count)
{
	struct haptic *haptic = dev_get_haptic(dev);
	uint32_t val = 0;
	int ret;

	if (!haptic || !buf)
		return -EINVAL;

	ret = kstrtouint(buf, 0, &val);
	if (ret < 0)
		return ret;
	if (val)
		ram_update(haptic);
	return count;
}

static ssize_t ram_num_show(struct device *dev, struct device_attribute *attr,
			    char *buf)
{
	struct haptic *haptic = dev_get_haptic(dev);
	ssize_t len = 0;

	if (!haptic || !buf)
		return -EINVAL;

	get_ram_num(haptic);
	len += snprintf(buf + len, PAGE_SIZE - len, "ram_num = %d\n",
			haptic->ram.ram_num);
	return len;
}

static ssize_t f0_show(struct device *dev, struct device_attribute *attr,
		       char *buf)
{
	struct haptic *haptic = dev_get_haptic(dev);
	ssize_t len = 0;

	if (!haptic || !buf)
		return -EINVAL;

	mutex_lock(&haptic->lock);
	haptic->hops->upload_lra(haptic, HAPTIC_WRITE_ZERO);
	haptic->hops->get_f0(haptic);
	haptic->hops->upload_lra(haptic, HAPTIC_F0_CALI_LRA);
	mutex_unlock(&haptic->lock);
	len += snprintf(buf + len, PAGE_SIZE - len,
			"lra_f0 = %d cont_f0 = %d\n", haptic->f0,
			haptic->cont_f0);
	return len;
}

static ssize_t cali_val_show(struct device *dev, struct device_attribute *attr,
			     char *buf)
{
	struct haptic *haptic = dev_get_haptic(dev);
	ssize_t len = 0;

	if (!haptic || !buf)
		return -EINVAL;
	vib_info("f0 val = %u", haptic->f0);

	if (haptic->f0_val_err < 0)
		haptic->f0 = 0;
	vib_info("cal f0 val = %u", haptic->f0);

	len += snprintf(buf + len, PAGE_SIZE - len, "%d", haptic->f0);
	return len;
}

static ssize_t cali_show(struct device *dev, struct device_attribute *attr,
			 char *buf)
{
	struct haptic *haptic = dev_get_haptic(dev);
	ssize_t len = 0;
	int cal_ret;

	if (!haptic || !buf)
		return -EINVAL;

	if (haptic->f0 > HAPTIC_F0_CALI_THER_H ||
	    haptic->f0 < HAPTIC_F0_CALI_THER_L)
		cal_ret = 0;
	else
		cal_ret = 1;

	if (haptic->f0_val_err < 0) {
		cal_ret = 0;
		vib_info("cal err");
	}
	len += snprintf(buf + len, PAGE_SIZE - len, "%d", cal_ret);
	return len;
}

static ssize_t cali_store(struct device *dev, struct device_attribute *attr,
			  const char *buf, size_t count)
{
	struct haptic *haptic = dev_get_haptic(dev);
	uint32_t val = 0;
	int ret;

	if (!haptic || !buf)
		return -EINVAL;

	ret = kstrtouint(buf, 0, &val);
	if (ret < 0)
		return ret;
	if (val) {
		haptic->f0_val_err = 0;
		mutex_lock(&haptic->lock);
		haptic->hops->f0_cali(haptic);
		mutex_unlock(&haptic->lock);
	}
	return count;
}

static ssize_t cont_show(struct device *dev, struct device_attribute *attr,
			 char *buf)
{
	struct haptic *haptic = dev_get_haptic(dev);
	ssize_t len = 0;

	if (!haptic || !buf)
		return -EINVAL;

	haptic->hops->read_f0(haptic);
	len += snprintf(buf + len, PAGE_SIZE - len, "cont_f0 = %d\n",
			haptic->cont_f0);
	return len;
}

static ssize_t cont_store(struct device *dev, struct device_attribute *attr,
			  const char *buf, size_t count)
{
	struct haptic *haptic = dev_get_haptic(dev);
	uint32_t val = 0;
	int ret;

	if (!haptic || !buf)
		return -EINVAL;

	ret = kstrtouint(buf, 0, &val);
	if (ret < 0)
		return ret;

	haptic->hops->play_stop(haptic);
	if (val) {
		haptic->hops->upload_lra(haptic, HAPTIC_F0_CALI_LRA);
		haptic->hops->cont_config(haptic);
	}
	return count;
}

static ssize_t vbat_monitor_show(struct device *dev,
				 struct device_attribute *attr, char *buf)
{
	struct haptic *haptic = dev_get_haptic(dev);
	ssize_t len = 0;

	if (!haptic || !buf)
		return -EINVAL;

	mutex_lock(&haptic->lock);
	haptic->hops->get_vbat(haptic);
	len += snprintf(buf + len, PAGE_SIZE - len, "vbat_monitor = %d\n",
			haptic->vbat);
	mutex_unlock(&haptic->lock);

	return len;
}

static ssize_t lra_resistance_show(struct device *dev,
				   struct device_attribute *attr, char *buf)
{
	struct haptic *haptic = dev_get_haptic(dev);
	ssize_t len = 0;

	if (!haptic || !buf)
		return -EINVAL;

	haptic->hops->get_lra_resistance(haptic);
	len += snprintf(buf + len, PAGE_SIZE - len, "lra_resistance = %d\n",
			haptic->lra);
	return len;
}

static ssize_t auto_boost_show(struct device *dev,
			       struct device_attribute *attr, char *buf)
{
	struct haptic *haptic = dev_get_haptic(dev);
	ssize_t len = 0;

	if (!haptic || !buf)
		return -EINVAL;

	len += snprintf(buf + len, PAGE_SIZE - len, "auto_boost = %d\n",
			haptic->auto_boost);

	return len;
}

static ssize_t auto_boost_store(struct device *dev,
				struct device_attribute *attr, const char *buf,
				size_t count)
{
	struct haptic *haptic = dev_get_haptic(dev);
	uint32_t val = 0;
	int ret;

	if (!haptic || !buf)
		return -EINVAL;

	ret = kstrtouint(buf, 0, &val);
	if (ret < 0)
		return ret;

	mutex_lock(&haptic->lock);
	haptic->hops->play_stop(haptic);
	haptic->hops->auto_bst_enable(haptic, val);
	mutex_unlock(&haptic->lock);

	return count;
}

static ssize_t prct_mode_show(struct device *dev, struct device_attribute *attr,
			      char *buf)
{
	struct haptic *haptic = dev_get_haptic(dev);
	ssize_t len = 0;
	uint8_t reg_val = 0;

	if (!haptic || !buf)
		return -EINVAL;

	reg_val = haptic->hops->get_prctmode(haptic);
	len += snprintf(buf + len, PAGE_SIZE - len, "prctmode = %d\n", reg_val);
	return len;
}

static ssize_t prct_mode_store(struct device *dev,
			       struct device_attribute *attr, const char *buf,
			       size_t count)
{
	struct haptic *haptic = dev_get_haptic(dev);
	uint32_t databuf[2] = { 0, 0 };
	uint32_t addr = 0;
	uint32_t val = 0;

	if (!haptic || !buf)
		return -EINVAL;

	if (sscanf(buf, "%x %x", &databuf[0], &databuf[1]) == 2) {
		addr = databuf[0];
		val = databuf[1];
		mutex_lock(&haptic->lock);
		haptic->hops->protect_config(haptic, addr, val);
		mutex_unlock(&haptic->lock);
	}
	return count;
}

static ssize_t ram_vbat_comp_show(struct device *dev,
				  struct device_attribute *attr, char *buf)
{
	struct haptic *haptic = dev_get_haptic(dev);
	ssize_t len = 0;

	if (!haptic || !buf)
		return -EINVAL;

	len += snprintf(buf + len, PAGE_SIZE - len, "ram_vbat_comp = %d\n",
			haptic->ram_vbat_comp);

	return len;
}

static ssize_t ram_vbat_comp_store(struct device *dev,
				   struct device_attribute *attr,
				   const char *buf, size_t count)
{
	struct haptic *haptic = dev_get_haptic(dev);
	uint32_t val = 0;
	int ret;

	if (!haptic || !buf)
		return -EINVAL;

	ret = kstrtouint(buf, 0, &val);
	if (ret < 0)
		return ret;
	mutex_lock(&haptic->lock);
	if (val)
		haptic->ram_vbat_comp = HAPTIC_RAM_VBAT_COMP_ENABLE;
	else
		haptic->ram_vbat_comp = HAPTIC_RAM_VBAT_COMP_DISABLE;
	mutex_unlock(&haptic->lock);

	return count;
}

static ssize_t f0_save_show(struct device *dev, struct device_attribute *attr,
			    char *buf)
{
	ssize_t len = 0;

	struct haptic *haptic = dev_get_haptic(dev);

	if (!haptic || !buf)
		return -EINVAL;

	len += snprintf(buf + len, PAGE_SIZE - len, "f0_cali_data = 0x%02X\n",
			haptic->f0_cali_data);

	return len;
}

static ssize_t f0_save_store(struct device *dev, struct device_attribute *attr,
			     const char *buf, size_t count)
{
	uint32_t val = 0;
	int ret = 0;

	struct haptic *haptic = dev_get_haptic(dev);

	if (!haptic || !buf)
		return -EINVAL;

	ret = kstrtouint(buf, 0, &val);
	if (ret < 0)
		return ret;

	mutex_lock(&haptic->lock);
	haptic->f0_cali_data = val;
	mutex_unlock(&haptic->lock);
	return count;
}

static ssize_t osc_save_show(struct device *dev, struct device_attribute *attr,
			     char *buf)
{
	ssize_t len = 0;

	struct haptic *haptic = dev_get_haptic(dev);

	if (!haptic || !buf)
		return -EINVAL;

	len += snprintf(buf + len, PAGE_SIZE - len, "osc_cali_data = 0x%02X\n",
			haptic->osc_cali_data);

	return len;
}

static ssize_t osc_save_store(struct device *dev, struct device_attribute *attr,
			      const char *buf, size_t count)
{
	uint32_t val = 0;
	int ret;
	struct haptic *haptic = dev_get_haptic(dev);

	if (!haptic || !buf)
		return -EINVAL;

	ret = kstrtouint(buf, 0, &val);
	if (ret < 0)
		return ret;
	haptic->osc_cali_data = val;
	return count;
}

static DEVICE_ATTR(f0, S_IWUSR | S_IRUGO, f0_show, NULL);
static DEVICE_ATTR(seq, S_IWUSR | S_IRUGO, seq_show, seq_store);
static DEVICE_ATTR(reg, S_IWUSR | S_IRUGO, reg_show, reg_store);
static DEVICE_ATTR(vmax, S_IWUSR | S_IRUGO, vmax_show, vmax_store);
static DEVICE_ATTR(gain, S_IWUSR | S_IRUGO, gain_show, gain_store);
static DEVICE_ATTR(loop, S_IWUSR | S_IRUGO, loop_show, loop_store);
static DEVICE_ATTR(rtp, S_IWUSR | S_IRUGO, rtp_show, rtp_sotre);
static DEVICE_ATTR(vibrator_calib, S_IWUSR | S_IRUGO, cali_show, cali_store);
static DEVICE_ATTR(cali_val, S_IWUSR | S_IRUGO, cali_val_show, NULL);
static DEVICE_ATTR(cont, S_IWUSR | S_IRUGO, cont_show, cont_store);
static DEVICE_ATTR(state, S_IWUSR | S_IRUGO, state_show, NULL);
static DEVICE_ATTR(index, S_IWUSR | S_IRUGO, index_show, index_store);
static DEVICE_ATTR(ram_num, S_IWUSR | S_IRUGO, ram_num_show, NULL);
static DEVICE_ATTR(duration, S_IWUSR | S_IRUGO, duration_show, duration_store);
static DEVICE_ATTR(activate, S_IWUSR | S_IRUGO, activate_show, activate_store);
static DEVICE_ATTR(prctmode, S_IWUSR | S_IRUGO, prct_mode_show,
		   prct_mode_store);
static DEVICE_ATTR(auto_boost, S_IWUSR | S_IRUGO, auto_boost_show,
		   auto_boost_store);
static DEVICE_ATTR(ram_update, S_IWUSR | S_IRUGO, ram_update_show,
		   ram_update_store);
static DEVICE_ATTR(vbat_monitor, S_IWUSR | S_IRUGO, vbat_monitor_show, NULL);
static DEVICE_ATTR(activate_mode, S_IWUSR | S_IRUGO, activate_mode_show,
		   activate_mode_store);
static DEVICE_ATTR(ram_vbat_comp, S_IWUSR | S_IRUGO, ram_vbat_comp_show,
		   ram_vbat_comp_store);
static DEVICE_ATTR(lra_resistance, S_IWUSR | S_IRUGO, lra_resistance_show,
		   NULL);
static DEVICE_ATTR(osc_save, S_IWUSR | S_IRUGO, osc_save_show, osc_save_store);
static DEVICE_ATTR(f0_save, S_IWUSR | S_IRUGO, f0_save_show, f0_save_store);

static struct attribute *vibrator_attributes[] = {
	&dev_attr_state.attr,
	&dev_attr_duration.attr,
	&dev_attr_activate.attr,
	&dev_attr_activate_mode.attr,
	&dev_attr_index.attr,
	&dev_attr_vmax.attr,
	&dev_attr_gain.attr,
	&dev_attr_seq.attr,
	&dev_attr_loop.attr,
	&dev_attr_reg.attr,
	&dev_attr_rtp.attr,
	&dev_attr_ram_update.attr,
	&dev_attr_ram_num.attr,
	&dev_attr_f0.attr,
	&dev_attr_f0_save.attr,
	&dev_attr_vibrator_calib.attr,
	&dev_attr_cali_val.attr,
	&dev_attr_cont.attr,
	&dev_attr_vbat_monitor.attr,
	&dev_attr_lra_resistance.attr,
	&dev_attr_auto_boost.attr,
	&dev_attr_prctmode.attr,
	&dev_attr_ram_vbat_comp.attr,
	&dev_attr_osc_save.attr,
	NULL
};

static struct attribute_group vibrator_attribute_group = {
	.attrs = vibrator_attributes
};

int haptic_create_sysfs_nodes(struct haptic *haptic)
{
	int ret;

	if (!haptic)
		return -EINVAL;

	ret = sysfs_create_group(&haptic->vib_dev.dev->kobj,
				 &vibrator_attribute_group);
	if (ret < 0) {
		vib_err("error creating sysfs attr files");
		return ret;
	}

	return 0;
}

void haptic_remove_sysfs_nodes(struct haptic *haptic)
{
	if (!haptic)
		return;

	sysfs_remove_group(&haptic->vib_dev.dev->kobj,
			   &vibrator_attribute_group);
}