/*
 * haptic_if.c
 *
 * code for vibrator
 *
 * Copyright (c) 2020 Huawei Technologies Co., Ltd.
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
#include <sound/core.h>
#include <sound/pcm.h>
#include <sound/pcm_params.h>
#include <sound/control.h>
#include <sound/soc.h>

#include "haptic_if.h"
#include "../vibrator_common/vibrator_common.h"
#ifdef AAC_RICHTAP_SUPPORT
#include <linux/mman.h>
#endif

char *cs_ram_name = "aw8692x_ram.bin";
char cs_rtp_name[][CS_RTP_NAME_MAX] = {
	{"haptic_rtp_osc_24K_5s.bin"},
	{"haptic_rtp.bin"},
	{"haptic_rtp_lighthouse.bin"},
	{"haptic_rtp_silk.bin"},
};
struct haptic_container *cs_rtp;
static struct mutex i2c_rw_lock;
static DEFINE_MUTEX(i2c_rw_lock);

static unsigned int boot_power_set;
static struct haptic *cs_haptic_file;
static struct timespec64 pre_ts;
static int64_t gap_time_ms = 0;

static const unsigned char long_amp_again_val[CS_LONG_MAX_AMP_CFG] = {
	0x80, 0x80, 0x70, 0x60, 0x50, 0x40, 0x30, 0x20, 0x10};

static const unsigned char short_amp_again_val[CS_SHORT_MAX_AMP_CFG] = {
	0x80, 0x80, 0x66, 0x4c, 0x33, 0x19};

static char cs_rtp_idx_name[CS_RTP_NAME_MAX] = {0};

int cs_i2c_byte_read(struct haptic *cs_haptic, uint8_t reg_addr, uint8_t * buf, uint32_t len)
{
	int ret = -1;
	int i = CS_I2C_RETRY_TIMES;
	struct i2c_msg msg[2];
	struct i2c_adapter *adapter = cs_haptic->i2c->adapter;

	if (buf == NULL || len == 0) {
		log_err(" input param is invalid");
		return -1;
	}
	/* send reg addr */
	msg[0].addr = cs_haptic->i2c->addr;
	msg[0].flags = 0;
	msg[0].buf = &reg_addr;
	msg[0].len = 1;
	/* read reg */
	msg[1].addr = cs_haptic->i2c->addr;
	msg[1].flags = I2C_M_RD;
	msg[1].buf = buf;
	msg[1].len = len;
	do {
		ret = i2c_transfer(adapter, msg, sizeof(msg) / sizeof(struct i2c_msg));
		if (ret < 0)
			log_err("read failed: %d", ret);
		else
			break;
		i--;
	} while (i > 0);
	return ret;
}

int cs_i2c_byte_write(struct haptic *cs_haptic, uint8_t reg_addr, uint8_t * buf, uint32_t len)
{
	int32_t ret = 0;
	uint8_t *data = NULL;
	int i = CS_I2C_RETRY_TIMES;
	struct i2c_msg msg;
	struct i2c_adapter *adapter = cs_haptic->i2c->adapter;

	if (buf == NULL || len == 0) {
		log_err("input param is invalid");
		return -1;
	}
	data = kmalloc(len + 1, GFP_KERNEL);
	if (data == NULL)
		return -ENOMEM;
	data[0] = reg_addr;
	memcpy(&data[1], buf, len);

	msg.addr = cs_haptic->i2c->addr;
	msg.flags = 0;
	msg.buf = data;
	msg.len = len + 1;
	do {
		ret = i2c_transfer(adapter, &msg, 1);
		if (ret < 0)
			log_err("write failed %d", ret);
		else
			break;
		i--;
	} while (i > 0);
	kfree(data);
	return ret;
}

int cs_i2c_one_byte_write(struct haptic *cs_haptic, uint8_t reg_addr, uint8_t data)
{
	uint8_t temp = 0;

	temp = data;
	return cs_i2c_byte_write(cs_haptic, reg_addr, &temp, 0x01);
}

int cs_write_sram(struct haptic *cs_haptic, uint8_t reg_addr, uint8_t *buf, uint32_t len)
{
	uint8_t *data = NULL;
	int ret = -1;

	if (buf == NULL || len == 0) {
		log_err("input param is invalid");
		return -1;
	}

	data = kmalloc(len + 1, GFP_KERNEL);
	data[0] = reg_addr;
	memcpy(&data[1], buf, len);
	ret = i2c_master_send(cs_haptic->i2c, data, len + 1);
	if (ret < 0)
		log_err("i2c write sram err:%d\n", ret);
	kfree(data);
	return ret;
}

static int parse_dt_gpio(struct device *dev, struct haptic *cs_haptic, struct device_node *np)
{
	int val = 0;

	if (!cs_haptic || !np) {
		log_err ("cs_haptic or np is null");
		return -EINVAL;
	}
	// hardware or software power enable contrl
	val = of_property_read_u32(np, "boost_fw", &cs_haptic->info.boost_fw);
	if (val != 0)
		log_info("boost_fw not found");
	// hardware enable when boost_fw is ture
	if (cs_haptic->info.boost_fw) {
		log_info("boost_en gpio software control %d", cs_haptic->info.boost_en);
		cs_haptic->info.boost_en = of_get_named_gpio(np, "boost_en", 0);
		if (cs_haptic->info.boost_en >= 0)
			log_info("boost en-%d ok\n", cs_haptic->info.boost_en);
		else
			log_err("get boost en gpio fail\n");
		log_info("boost_en gpio provide ok %d", cs_haptic->info.boost_en);
	} else {
		log_info("boost_en gpio hardware control %d", cs_haptic->info.boost_en);
	}
	cs_haptic->reset_gpio = of_get_named_gpio(np, "reset-gpio", 0);
	if (cs_haptic->reset_gpio < 0) {
		log_err("no reset gpio provide");
		return -EPERM;
	}
	log_info("reset gpio provide ok %d", cs_haptic->reset_gpio);
	cs_haptic->irq_gpio = of_get_named_gpio(np, "irq-gpio", 0);
	if (cs_haptic->irq_gpio < 0) {
		log_err("no irq gpio provided");
		return -EINVAL;
	} else {
		log_info("irq gpio provide ok irq = %d", cs_haptic->irq_gpio);
	}
	val = of_property_read_u8(np, "mode", &cs_haptic->info.mode);
	if (val != 0)
		log_info("mode not found");

	val = of_property_read_u32(np, "boost_power_on", &boot_power_set);
	if (val != 0)
		log_info("boot_power_set not found");

	// boost enable or unable contrl
	val = of_property_read_u32(np, "bst_enable", &cs_haptic->bst_enable);
	if (val != 0)
		log_info("bst_enable not found");
	log_info("cs_haptic->bst_enable = %u", cs_haptic->bst_enable);

	return 0;
}

static void hw_reset(struct haptic *cs_haptic)
{
	log_info("enter");
	if (cs_haptic && gpio_is_valid(cs_haptic->reset_gpio)) {
		gpio_set_value_cansleep(cs_haptic->reset_gpio, 0);
		usleep_range(1000, 2000);
		gpio_set_value_cansleep(cs_haptic->reset_gpio, 1);
		usleep_range(8000, 8500);
	} else {
		log_err("failed");
	}
}

static int read_chipid(struct haptic *cs_haptic, uint32_t *reg_val)
{
	uint8_t value = 0;
	int ret = 0;

	log_info("enter");
	/* try the old way of read chip id */
	ret = cs_i2c_byte_read(cs_haptic, CDV2625_REG_CHIPID, &value, CS_I2C_BYTE_ONE);
	if (ret < 0)
		return ret;
	*reg_val = value;
	return ret;
}

static void get_sram_size(struct haptic *cs_haptic)
{
	uint8_t value = 0;
	int ret = 0;

	log_info("enter");
	/* try the old way of read chip id */
	ret = cs_i2c_byte_read(cs_haptic, 0x6f, &value, CS_I2C_BYTE_ONE);
	if (ret < 0) {
		cs_haptic->ram_id = CS_RAM_IDA;
	} else {
		if (value & CDV2625_RAM_TYPE)
			cs_haptic->ram_id = CS_RAM_IDB;
		else
			cs_haptic->ram_id = CS_RAM_IDA;
	}
}

static int parse_chipid(struct haptic *cs_haptic)
{
	int ret = -1;
	uint32_t reg = 0;
	uint8_t cnt = 0;
	uint8_t t_ret_val = 0;

	log_info("enter");
	for (cnt = 0; cnt < CS_READ_CHIPID_RETRIES; cnt++) {
		/* hardware reset */
		hw_reset(cs_haptic);
		ret = read_chipid(cs_haptic, &reg);
		log_info("reg_val = 0x%02X", reg);
		if (ret < 0) {
			log_err("read chip id fail: %d", ret);
			continue;
		}
		switch (reg) {
		case CDV2625_CHIPID:
			cs_haptic->chipid = reg;
			cs_haptic->i2s_config = false;
			ret = cs_i2c_byte_read(cs_haptic, 0x6f, &t_ret_val, CS_I2C_BYTE_ONE);
			if (ret < 0)
				return ret;
			get_sram_size(cs_haptic);
			log_info("detected cdv2625");
			return 0;
		default:
			log_info("unsupport device (0x%02X)", reg);
			break;
		}
		usleep_range(CS_READ_CHIPID_RETRY_DELAY * 1000, CS_READ_CHIPID_RETRY_DELAY * 1000 + 500);
	}
	return -EINVAL;
}

static int ctrl_init(struct haptic *cs_haptic, struct device *dev)
{
	log_info("enter");
	switch (cs_haptic->chipid) {
	case CDV2625_CHIPID:
		cs_haptic->func = cdv2625_func();
		break;
	default:
		log_info("unexpected chipid!");
		return -EINVAL;
	}
	return 0;
}

static void ram_play(struct haptic *cs_haptic, uint8_t mode)
{
	log_info("enter");
	cs_haptic->func->play_mode(cs_haptic, mode);
	cs_haptic->func->play_go(cs_haptic, true);
}

static int get_wave_num(struct haptic *cs_haptic)
{
	uint8_t wave_addr[2] = {0};
	uint32_t first_wave_addr = 0;

	log_info("enter");
	if (!cs_haptic->ram_init) {
		log_err("ram init faild, ram_num = 0!");
		return -EPERM;
	}
	mutex_lock(&cs_haptic->lock);
	/* RAMINIT Enable */

	cs_haptic->func->play_stop(cs_haptic);
	cs_haptic->func->ram_init(cs_haptic, true);
	cs_haptic->func->set_ram_addr(cs_haptic);
	cs_haptic->func->get_first_wave_addr(cs_haptic, wave_addr);
	first_wave_addr = (wave_addr[0] << 8 | wave_addr[1]);
	cs_haptic->ram.ram_num = (first_wave_addr - cs_haptic->ram.base_addr - 1) / 4;
	log_info("first wave addr = 0x%04x", first_wave_addr);
	log_info("ram_num = %u", cs_haptic->ram.ram_num);
	/* RAMINIT Disable */
	cs_haptic->func->ram_init(cs_haptic, false);
	mutex_unlock(&cs_haptic->lock);
	return 0;
}

static void ram_vbat_comp(struct haptic *cs_haptic, bool flag)
{
	log_info("enter");

	if (flag) {
		if (cs_haptic->ram_vbat_comp == CS_RAM_VBAT_COMP_ENABLE) {
			cs_haptic->func->get_vbat(cs_haptic);
			cs_haptic->func->vbat_mode_config(cs_haptic, CS_CONT_VBAT_HW_COMP_MODE);
		} else {
			cs_haptic->func->set_gain(cs_haptic, cs_haptic->gain);
			log_info("%s cs_haptic->gain 1 = %d", __func__, cs_haptic->gain);
		}
	} else {
		cs_haptic->func->set_gain(cs_haptic, cs_haptic->gain);
		log_info("%s cs_haptic->gain 2 = %d", __func__, cs_haptic->gain);
	}
}

int get_trim_range(uint16_t val, uint16_t *index, uint16_t *o_factor, uint8_t *o_offset)
{
	uint16_t cali_factor[] = {F0_CALI_FACTOR00, F0_CALI_FACTOR01, F0_CALI_FACTOR02, F0_CALI_FACTOR03,
		F0_CALI_FACTOR04, F0_CALI_FACTOR05, F0_CALI_FACTOR06, F0_CALI_FACTOR07, F0_CALI_FACTOR08};
	uint8_t cali_offset[] = {F0_CALI_BASE00_OFFSET, F0_CALI_BASE01_OFFSET, F0_CALI_BASE02_OFFSET,
		F0_CALI_BASE03_OFFSET, F0_CALI_BASE04_OFFSET, F0_CALI_BASE05_OFFSET, F0_CALI_BASE06_OFFSET,
		F0_CALI_BASE07_OFFSET, F0_CALI_BASE08_OFFSET};
	uint16_t f0_cali_region;

	f0_cali_region = val / F0_CALI_STEP;
	if (f0_cali_region < F0_CALI_INDEX_MIN || f0_cali_region > F0_CALI_INDEX_MAX) {
		log_info("trim_osc err, value = %u, range = %d - %d", val, F0_CALI_INDEX_MIN * F0_CALI_STEP,
			F0_CALI_INDEX_MAX * F0_CALI_STEP);
		return -1;
	}
	*o_factor = cali_factor[f0_cali_region - F0_CALI_INDEX_MIN];
	*o_offset = cali_offset[f0_cali_region - F0_CALI_INDEX_MIN];
	*index = f0_cali_region;
	return 0;
}

uint16_t trim_positive_compensate(uint16_t cail_val, uint16_t index, uint16_t trim_base, uint16_t factor)
{
	uint16_t f0_cali_region = index;
	uint16_t trim_osc_compensate = cail_val;
	uint16_t cali_factor = factor;
	uint16_t cali_factor2 = 0;
	uint8_t cali_offset2 = 0;
	uint16_t temp_trim_base = trim_base;

	/* out of current range */
	if (cail_val >= (f0_cali_region + 1) * F0_CALI_STEP) {
		if (get_trim_range(trim_osc_compensate, &f0_cali_region, &cali_factor2, &cali_offset2))
			return 0;
		trim_osc_compensate = f0_cali_region * F0_CALI_STEP + cali_offset2 + (trim_osc_compensate -
					f0_cali_region * F0_CALI_STEP + 1) * cali_factor / cali_factor2;
	}
	trim_osc_compensate = trim_osc_compensate - temp_trim_base;
	return trim_osc_compensate;
}

uint16_t trim_negative_compensate(uint16_t cail_val, uint16_t index, uint16_t trim_base, uint16_t fact, uint16_t offset)
{
	uint16_t f0_cali_region = index;
	uint16_t trim_osc_compensate = cail_val;
	uint16_t cali_factor = fact;
	uint16_t cali_factor2 = 0;
	uint8_t cali_offset = offset;
	uint8_t cali_offset2 = 0;
	uint16_t temp_trim_base = trim_base;

	if (trim_osc_compensate < f0_cali_region * F0_CALI_STEP) {
		if (get_trim_range(trim_osc_compensate, &f0_cali_region, &cali_factor2, &cali_offset2))
			return 0;
		trim_osc_compensate = (f0_cali_region + 1) * F0_CALI_STEP - 1 - ((uint16_t)((f0_cali_region + 1) *
							   F0_CALI_STEP - trim_osc_compensate + cali_offset)*cali_factor/cali_factor2);
	}
	trim_osc_compensate = temp_trim_base - trim_osc_compensate ;
	return trim_osc_compensate;
}

static void f0_cali(struct haptic *cs_haptic)
{
	uint32_t f0_min = 0;
	uint32_t f0_max = 0;
	uint16_t cali_factor = 0;
	uint8_t cali_offset = 0;
	uint16_t trim_osc_compensate = 0;
	uint16_t f0_cali_region;
	int32_t f0_offset = 0;

	log_info("enter");
	f0_min = cs_haptic->info.f0_pre * (100 - cs_haptic->info.f0_cali_percent) / 100;
	f0_max = cs_haptic->info.f0_pre * (100 + cs_haptic->info.f0_cali_percent) / 100;
	cs_haptic->func->upload_lra(cs_haptic, CS_F0_CALI_DEFAULT);
	/* power on f0 check */
	cs_haptic->func->get_f0(cs_haptic);
	if (cs_haptic->f0 < f0_min || cs_haptic->f0 > f0_max) {
		cs_haptic->func->upload_lra(cs_haptic, CS_F0_CALI_LRA);
		log_info("cs_haptic->f0 < f0_min || cs_haptic->f0 > f0_max");
		return;
	}
	f0_offset = cs_haptic->f0 - cs_haptic->info.f0_pre;

	if (get_trim_range(cs_haptic->func->get_trim_osc(cs_haptic), &f0_cali_region, &cali_factor, &cali_offset))
		return;
	if (cs_haptic->f0 == 0 || cali_factor == 0)
		return;
	trim_osc_compensate = (uint16_t)((int32_t)cs_haptic->func->get_trim_osc(cs_haptic) + F0_CALI_OSC_BASE *
		F0_CALI_FLOAT * f0_offset / (int32_t)cs_haptic->f0 / (int32_t)cali_factor);
	log_info("trim_osc_compensate=%u f0_cali_region=%u cali_factor=%u cali_offset=%u",
		trim_osc_compensate, f0_cali_region, cali_factor, cali_offset);
	if (f0_offset > 0) {
		/* out of current range */
		trim_osc_compensate = trim_positive_compensate(trim_osc_compensate, f0_cali_region,
			cs_haptic->func->get_trim_osc(cs_haptic), cali_factor);
	} else {
		/* out of current range */
		trim_osc_compensate = trim_negative_compensate(trim_osc_compensate, f0_cali_region,
			cs_haptic->func->get_trim_osc(cs_haptic), cali_factor, cali_offset);
	}
	log_info("trim_osc_compensate=0x%x", trim_osc_compensate);
	if (f0_offset <= 0)
		trim_osc_compensate |= 0;
	else
		trim_osc_compensate |= 0x400;
	cs_haptic->f0_cali_data = trim_osc_compensate;
	cs_haptic->func->upload_lra(cs_haptic, CS_F0_CALI_LRA);
	return;
}

static int cs_file_open(struct inode *inode, struct file *file)
{
	if (!file)
		return 0;
	if (!try_module_get(THIS_MODULE))
		return -ENODEV;
	file->private_data = (void *)cs_haptic_file;
	return 0;
}

static int cs_file_release(struct inode *inode, struct file *file)
{
	if (!file)
		return 0;
	file->private_data = (void *)NULL;
	module_put(THIS_MODULE);
	return 0;
}

static long cs_file_unlocked_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
{
	struct haptic *cs_haptic = NULL;

	if (!file)
		return 0;
	cs_haptic = (struct haptic *)file->private_data;
	if (!cs_haptic)
		return 0;
	log_info("cmd=0x%x, arg=0x%lx", cmd, arg);

	mutex_lock(&cs_haptic->lock);
	if (_IOC_TYPE(cmd) != CS_HAPTIC_IOCTL_MAGIC) {
		log_err("cmd magic err");
		mutex_unlock(&cs_haptic->lock);
		return -EINVAL;
	}

	switch (cmd) {
	default:
		log_err("unknown cmd");
		break;
	}
	mutex_unlock(&cs_haptic->lock);

	return 0;
}

static ssize_t cs_file_read(struct file *filp, char *buff, size_t len, loff_t *offset)
{
	return len;
}

static void cs_adapt_amp_again(struct haptic *cs_haptic, int haptic_type)
{
	if (!cs_haptic) {
		log_err("haptic is null");
		return ;
	}
	switch (haptic_type) {
	case LONG_VIB_RAM_MODE:
		if (cs_haptic->amplitude >= CS_LONG_MAX_AMP_CFG || cs_haptic->amplitude < 0)
			return;
		cs_haptic->gain = long_amp_again_val[cs_haptic->amplitude];
		log_info("long gain = %u", cs_haptic->gain);
		break;
	case SHORT_VIB_RAM_MODE:
		if (cs_haptic->amplitude >= CS_SHORT_MAX_AMP_CFG || cs_haptic->amplitude < 0)
			return;
		cs_haptic->gain = short_amp_again_val[cs_haptic->amplitude];
		log_info("short gain = %u", cs_haptic->gain);
		break;
	case RTP_VIB_MODE:
		cs_haptic->gain = 0x80;
		break;
	default:
		break;
	}
}

static void cs_special_type_process(struct haptic *cs_haptic, uint64_t type)
{
	switch (type) {
	case HAPTIC_CHARGING_CALIB_ID:
		cs_haptic->f0_is_cali = HAPTIC_CHARGING_IS_CALIB;
		schedule_work(&cs_haptic->vibrator_work);
		break;
	case HAPTIC_NULL_WAVEFORM_ID:
		log_info("null wave id");
		break;
	default:
		break;
	}
}

static void cs_fine_grained_adapt_amp_gain(struct haptic *cs_haptic)
{
	if (cs_haptic->amplitude > HAPTIC_GAIN_MAX_ADAPT_LEVELS)
		cs_haptic->amplitude = HAPTIC_GAIN_MAX_ADAPT_LEVELS;

	if (cs_haptic->amplitude < 0)
		return;

	cs_haptic->gain = HAPTIC_MAX_GAIN_REG * cs_haptic->amplitude / HAPTIC_GAIN_MAX_ADAPT_LEVELS;
	log_info("gain = %u", cs_haptic->gain);
}

static void cs_file_cmd_parse(struct haptic *cs_haptic, uint64_t cmd_type)
{
	int rtp_len = 0;
	uint64_t type = cmd_type;
	log_info("enter");
	if (cs_haptic == NULL) {
		log_info("cs_haptic is null");
		return;
	}
	cs_haptic->duration = 0;
	if (type > FINE_GRAINED_RAM_BASE) { // fine grained, amp is 0-100
		cs_haptic->effect_mode = SHORT_VIB_RAM_MODE;
		cs_haptic->amplitude = type % FINE_GRAINED_AMP_DIV_COFF;
		cs_haptic->index = type / FINE_GRAINED_AMP_DIV_COFF - FINE_GRAINED_RAM_ID_BASE_VAL;
		log_info("fine grained index = %d, amp = %d\n", cs_haptic->index, cs_haptic->amplitude);
		cs_haptic->state = 1;
		cs_fine_grained_adapt_amp_gain(cs_haptic);
		schedule_work(&cs_haptic->vibrator_work);
	} else if (type > LONG_HAPTIC_RTP_MAX_ID && type < FINE_GRAINED_RAM_BASE) {
		cs_haptic->effect_mode = LONG_VIB_RAM_MODE;
		cs_haptic->index = LONG_VIB_EFFECT_ID;
		cs_haptic->duration = type / LONG_TIME_AMP_DIV_COFF;
		cs_haptic->amplitude = type % LONG_TIME_AMP_DIV_COFF;
		cs_haptic->state = 1;
		cs_adapt_amp_again(cs_haptic, LONG_VIB_RAM_MODE);
		__pm_wakeup_event(cs_haptic->ws, cs_haptic->duration + HAPTIC_WAKE_LOCK_GAP);
		log_info("long index = %d amp = %d", cs_haptic->index, cs_haptic->amplitude);
		schedule_work(&cs_haptic->vibrator_work);
	} else if ((type > 0) && (type <= SHORT_HAPTIC_RAM_MAX_ID)) { // short ram Shaptic
		cs_haptic->effect_mode = SHORT_VIB_RAM_MODE;
		cs_haptic->amplitude = type % SHORT_HAPTIC_AMP_DIV_COFF;
		cs_haptic->index = type / SHORT_HAPTIC_AMP_DIV_COFF;
		cs_haptic->duration = SHORT_RAM_DURATION_INIT;
		cs_adapt_amp_again(cs_haptic, SHORT_VIB_RAM_MODE);
		log_info("short index = %d amp = %d", cs_haptic->index, cs_haptic->amplitude);
		cs_haptic->state = 1;
		schedule_work(&cs_haptic->vibrator_work);
	} else {
		cs_haptic->effect_mode = RTP_VIB_MODE;
		cs_haptic->amplitude = type % SHORT_HAPTIC_AMP_DIV_COFF;
		cs_haptic->index = type / SHORT_HAPTIC_AMP_DIV_COFF;
		cs_haptic->func->play_stop(cs_haptic);
		cs_haptic->func->set_rtp_aei(cs_haptic, false);
		cs_haptic->func->irq_clear(cs_haptic);
		cs_adapt_amp_again(cs_haptic, RTP_VIB_MODE);
		cs_haptic->state = 1;
		cs_haptic->rtp_idx = cs_haptic->index - BASE_INDEX;
		rtp_len += snprintf(cs_rtp_idx_name, CS_RTP_NAME_MAX - 1, "cs_%d.bin", cs_haptic->rtp_idx);
		log_info("get rtp name = %s, index = %d, len = %d", cs_rtp_idx_name, cs_haptic->index, rtp_len);
		schedule_work(&cs_haptic->rtp_work);
	}
}

static ssize_t cs_file_write(struct file *filp, const char *buff, size_t len, loff_t *off)
{
	struct haptic *cs_haptic = NULL;
	char write_buff[MAX_WRITE_BUF_LEN] = {0};
	uint64_t type = 0;
	int ret = 0;

	if (!buff || !filp || (len > (MAX_WRITE_BUF_LEN - 1)))
		return -EINVAL;

	log_info("enter");
	cs_haptic = (struct haptic*)filp->private_data;
	if (!cs_haptic) {
		log_err("cs_haptic is null");
		return -EINVAL;
	}
	mutex_lock(&cs_haptic->lock);
	ret = copy_from_user(write_buff, buff, len);
	if (ret) {
		mutex_unlock(&cs_haptic->lock);
		log_err("copy from user fail");
		return -EINVAL;
	}
	if (kstrtoull(write_buff, 10, &type)) {
		log_err("parse value err");
		mutex_unlock(&cs_haptic->lock);
		return -EINVAL;
	}
	log_info("get haptic id = %llu", type);
	cs_haptic->emergency_mode_flag = false;
	cs_haptic->cust_boost_on = CS_SEL_BOOST_CFG_ON;
	cs_haptic->need_notify_duration = false;
	if ((type < HAPTIC_MAX_SPEC_CMD_ID) && (type > HAPTIC_MIN_SPEC_CMD_ID)) {
		cs_special_type_process(cs_haptic, type);
		mutex_unlock(&cs_haptic->lock);
		return len;
	}
	cs_file_cmd_parse(cs_haptic, type);
	mutex_unlock(&cs_haptic->lock);
	return len;
}

#ifdef AAC_RICHTAP_SUPPORT
static void richtap_clean_buf(struct haptic *cs_haptic, int status)
{
	struct mmap_buf_format *opbuf = cs_haptic->start_buf;
	int i = 0;

	for (i = 0; i < RICHTAP_MMAP_BUF_SUM; i++) {
		memset(opbuf->data, 0, RICHTAP_MMAP_BUF_SIZE);
		opbuf->status = status;
		opbuf = opbuf->kernel_next;
	}
}

static void richtap_update_fifo_data(struct haptic *cs_haptic, uint32_t fifo_len)
{
	int32_t samples_left;
	int pos = 0;
	int retry = 3; // set max retry count 3

	do {
		if (cs_haptic->curr_buf->status == MMAP_BUF_DATA_VALID) {
			samples_left = cs_haptic->curr_buf->length - cs_haptic->pos;
			if (samples_left < fifo_len) {
				memcpy(&cs_haptic->rtp_ptr[pos],
					&cs_haptic->curr_buf->data[cs_haptic->pos],
					samples_left);
				pos += samples_left;
				fifo_len -= samples_left;
				cs_haptic->curr_buf->status = MMAP_BUF_DATA_INVALID;
				cs_haptic->curr_buf->length = 0;
				cs_haptic->curr_buf = cs_haptic->curr_buf->kernel_next;
				cs_haptic->pos = 0;
			} else {
				memcpy(&cs_haptic->rtp_ptr[pos],
					&cs_haptic->curr_buf->data[cs_haptic->pos],
					fifo_len);
				cs_haptic->pos += fifo_len;
				pos += fifo_len;
				fifo_len = 0;
			}
		} else if (cs_haptic->curr_buf->status == MMAP_BUF_DATA_FINISHED) {
			break;
		} else {
			if (retry <= 0) {
				log_info("invalid data");
				break;
			} else {
				usleep_range(1000, 1000); // delay 1ms for next
			}
			retry--;
		}
	} while (fifo_len > 0 && atomic_read(&cs_haptic->richtap_rtp_mode));
	log_info("update fifo len %d", pos);
	cs_haptic->func->set_rtp_data(cs_haptic, cs_haptic->rtp_ptr, pos);
}

static bool richtap_rtp_start(struct haptic *cs_haptic)
{
	int cnt = 200; // max wait gls state time
	bool rtp_work_flag = false;
	uint8_t reg_val = 0;

	mutex_lock(&cs_haptic->lock);
	cs_haptic->func->play_mode(cs_haptic, CS_RTP_MODE);
	cs_haptic->func->fifo_reset(cs_haptic);
	cs_haptic->func->play_go(cs_haptic, true);
	udelay(2000);

	while (cnt) {
		reg_val = cs_haptic->func->get_glb_state(cs_haptic);
		if ((reg_val & CS_GLBRD_STATE_MASK) == CS_STATE_RTP) {
			cnt = 0;
			rtp_work_flag = true;
			log_info("RTP_GO! glb_state=0x08");
		} else {
			usleep_range(2000, 2200);
			cnt--;
			log_info("wait for RTP_GO, glb_state=0x%02X\n", reg_val);
		}
	}

	if (rtp_work_flag == false)
		cs_haptic->func->play_stop(cs_haptic);

	mutex_unlock(&cs_haptic->lock);
	return rtp_work_flag;
}

static void update_richtap_play_on_time(int data_len)
{
	if (data_len > MAX_TRANSIENT_RTP_LEN)
		return;
	ktime_get_real_ts64(&pre_ts);
	gap_time_ms = data_len / RTP_LEN_TO_PLAY_TIME_COFF + RTP_PLAY_END_STABLE_TIME; // calc play time
}

static void richtap_rtp_work(struct work_struct *work)
{
	struct haptic *cs_haptic = container_of(work, struct haptic, richtap_rtp_work);
	uint32_t retry = 0;
	uint32_t tmp_len = 0;

	if (!cs_haptic)
		return;
	cs_haptic->curr_buf = cs_haptic->start_buf;
	do {
		retry++;
		if (cs_haptic->curr_buf->status == MMAP_BUF_DATA_VALID) {
			if ((tmp_len + cs_haptic->curr_buf->length) > cs_haptic->ram.base_addr) {
				memcpy(&cs_haptic->rtp_ptr[tmp_len], cs_haptic->curr_buf->data, (cs_haptic->ram.base_addr - tmp_len));
				cs_haptic->pos = cs_haptic->ram.base_addr - tmp_len;
				tmp_len = cs_haptic->ram.base_addr;
			} else {
				memcpy(&cs_haptic->rtp_ptr[tmp_len], cs_haptic->curr_buf->data, cs_haptic->curr_buf->length);
				tmp_len += cs_haptic->curr_buf->length;
				cs_haptic->curr_buf->status = MMAP_BUF_DATA_INVALID;
				cs_haptic->curr_buf->length = 0;
				cs_haptic->pos = 0;
				cs_haptic->curr_buf = cs_haptic->curr_buf->kernel_next;
			}
		} else if (cs_haptic->curr_buf->status == MMAP_BUF_DATA_FINISHED) {
			break;
		} else {
			usleep_range(1000, 1200);
		}
	} while (tmp_len < cs_haptic->ram.base_addr && retry <= 30); // try cont

	// 400K in 20k out
	log_info("rtp tm_len = %d, retry = %d, cs_haptic->ram.base_addr = %d\n",
		tmp_len, retry, cs_haptic->ram.base_addr);
	if (richtap_rtp_start(cs_haptic)) {
		cs_haptic->func->set_rtp_data(cs_haptic, cs_haptic->rtp_ptr, tmp_len);
		update_richtap_play_on_time(tmp_len);
		log_info("set rtp data end");
		cs_haptic->func->set_rtp_aei(cs_haptic, true);
		cs_haptic->func->irq_clear(cs_haptic);
		atomic_set(&cs_haptic->richtap_rtp_mode, true);
	}
}

static void richtap_config_stream_mode(struct haptic *cs_haptic)
{
	richtap_clean_buf(cs_haptic, MMAP_BUF_DATA_INVALID);
	mutex_lock(&cs_haptic->lock);
	cs_haptic->func->irq_clear(cs_haptic);
	cs_haptic->func->play_stop(cs_haptic);
	mutex_unlock(&cs_haptic->lock);
	cs_haptic->func->set_rtp_aei(cs_haptic, false);
	cs_haptic->func->set_pwm(cs_haptic, CS_PWM_12K);
	atomic_set(&cs_haptic->richtap_rtp_mode, false);
	cs_haptic->func->upload_lra(cs_haptic, CS_F0_CALI_LRA);
	cs_haptic->func->set_bst_vol(cs_haptic, cs_haptic->vmax);
	schedule_work(&cs_haptic->richtap_rtp_work);
}

static void richtap_config_stop_mode(struct haptic *cs_haptic)
{
	richtap_clean_buf(cs_haptic, MMAP_BUF_DATA_FINISHED);
	mutex_lock(&cs_haptic->lock);
	cs_haptic->func->irq_clear(cs_haptic);
	cs_haptic->func->play_stop(cs_haptic);
	mutex_unlock(&cs_haptic->lock);
	cs_haptic->func->enable_gain(cs_haptic, 0);
	cs_haptic->func->set_rtp_aei(cs_haptic, false);
	atomic_set(&cs_haptic->richtap_rtp_mode, false);
}

static int richtap_running_status_check()
{
	struct timespec64 now_ts;
	struct timespec64 sub_ts;
	int64_t ns;

	ktime_get_real_ts64(&now_ts);
	sub_ts = timespec64_sub(now_ts, pre_ts);
	ns = timespec64_to_ns(&sub_ts);
	pr_info("%s: gap pre play time = %lu, gap_time_ms = %lu\n", __func__, ns, gap_time_ms);
	if (ns < (gap_time_ms * MS_TIME_TO_NS_TIME))
		return -1;

	return 0;
}

static long richtap_file_unlocked_ioctl(struct file *filp,
	unsigned int cmd, unsigned long arg)
{
	struct haptic *cs_haptic = NULL;
	int ret = 0;
	int tmp;

	if (!filp)
		return -EFAULT;
	cs_haptic = (struct haptic *)filp->private_data;
	if (!cs_haptic)
		return -EFAULT;
	log_info("cmd=0x%x, arg=0x%lx\n", cmd, arg);
	if (cs_haptic->info.is_enabled_richtap_core != true) {
		log_info("do not support richtap core feature");
		return -EFAULT;
	}
	switch (cmd) {
	case RICHTAP_GET_HWINFO:
		tmp = RICHTAP_CDV2625;
		if (copy_to_user((void __user *)arg, &tmp, sizeof(int)))
			ret = -EFAULT;
		break;
	case RICHTAP_RTP_MODE:
		break;
	case RICHTAP_OFF_MODE:
		break;
	case RICHTAP_GET_F0:
		tmp = cs_haptic->f0;
		log_info("get f0 = %d\n", tmp);
		if (copy_to_user((void __user *)arg, &tmp, sizeof(int)))
			ret = -EFAULT;
		break;
	case RICHTAP_SETTING_GAIN:
		if (arg > 0x80)
			arg = 0x80;
		cs_haptic->func->enable_gain(cs_haptic, 1);
		cs_haptic->func->set_gain(cs_haptic, (uint8_t)arg);
		break;
	case RICHTAP_STREAM_MODE:
		__pm_wakeup_event(cs_haptic->ws, HAPTIC_RTP_WAKE_TIME);
		ret = richtap_running_status_check();
		if (!ret)
			richtap_config_stream_mode(cs_haptic);
		break;
	case RICHTAP_STOP_MODE:
		ret = richtap_running_status_check();
		if (!ret)
			richtap_config_stop_mode(cs_haptic);
		break;
	default:
		log_err("unknown cmd");
		break;
	}
	return ret;
}

static int richtap_file_mmap(struct file *filp, struct vm_area_struct *vma)
{
	unsigned long phys;
	struct haptic *cs_haptic = NULL;
	int ret;
	vm_flags_t vm_flags;

	if (!filp || !vma)
		return -EPERM;
	cs_haptic = (struct haptic *)filp->private_data;
	if (!cs_haptic)
		return -EPERM;
#if LINUX_VERSION_CODE > KERNEL_VERSION(4,7,0)
	// only accept PROT_READ, PROT_WRITE and MAP_SHARED from the API of mmap
	vm_flags = calc_vm_prot_bits(PROT_READ|PROT_WRITE, 0) | calc_vm_flag_bits(MAP_SHARED);
	vm_flags |= current->mm->def_flags | VM_MAYREAD | VM_MAYWRITE | VM_MAYEXEC| VM_SHARED | VM_MAYSHARE;
	if (vma && (pgprot_val(vma->vm_page_prot) != pgprot_val(vm_get_page_prot(vm_flags))))
		return -EPERM;

	if (vma && ((vma->vm_end - vma->vm_start) != (PAGE_SIZE << RICHTAP_MMAP_PAGE_ORDER)))
		return -ENOMEM;
#endif
	phys = virt_to_phys(cs_haptic->start_buf);
	ret = remap_pfn_range(vma, vma->vm_start, (phys >> PAGE_SHIFT),
		(vma->vm_end - vma->vm_start), vma->vm_page_prot);
	if (ret) {
		dev_err(cs_haptic->dev, "Error mmap failed\n");
		return ret;
	}

	return ret;
}

static int richtap_rtp_config_init(struct haptic *cs_haptic)
{
	uint32_t i;
	struct mmap_buf_format *temp;

	cs_haptic->rtp_ptr = kmalloc(RICHTAP_MMAP_BUF_SIZE * RICHTAP_MMAP_BUF_SUM,
		GFP_KERNEL);
	if (cs_haptic->rtp_ptr == NULL) {
		log_err("malloc rtp memory failed");
		return -EINVAL;
	}

	cs_haptic->start_buf = (struct mmap_buf_format *)__get_free_pages(GFP_KERNEL, RICHTAP_MMAP_PAGE_ORDER);
	if (cs_haptic->start_buf == NULL) {
		kfree(cs_haptic->rtp_ptr);
		log_err("Error __get_free_pages failed");
		return -EINVAL;
	}
	SetPageReserved(virt_to_page(cs_haptic->start_buf));
	temp = cs_haptic->start_buf;
	for (i = 1; i < RICHTAP_MMAP_BUF_SUM; i++) { // from page 1 start
		temp->kernel_next = (cs_haptic->start_buf + i);
		temp = temp->kernel_next;
	}
	temp->kernel_next = cs_haptic->start_buf;

	INIT_WORK(&cs_haptic->richtap_rtp_work, richtap_rtp_work);
	atomic_set(&cs_haptic->richtap_rtp_mode, false);
	return 0;
}
#endif

static const struct file_operations fops = {
	.owner = THIS_MODULE,
	.read = cs_file_read,
	.write = cs_file_write,
	.unlocked_ioctl = cs_file_unlocked_ioctl,
#ifdef AAC_RICHTAP_SUPPORT
	.unlocked_ioctl = richtap_file_unlocked_ioctl,
	.mmap = richtap_file_mmap,
#endif
	.open = cs_file_open,
	.release = cs_file_release,
};

static struct miscdevice cs_haptic_misc = {
	.minor = MISC_DYNAMIC_MINOR,
	.name = CS_HAPTIC_NAME,
	.fops = &fops,
};

static enum hrtimer_restart vibrator_timer_func(struct hrtimer *timer)
{
	struct haptic *cs_haptic = NULL;

	log_info("enter");
	if (timer == NULL) {
		log_err("timer is null");
		return -EINVAL;
	}
	cs_haptic = container_of(timer, struct haptic, timer);
	cs_haptic->state = 0;
	schedule_work(&cs_haptic->vibrator_work);
	return HRTIMER_NORESTART;
}

static void cs_long_ram_mode_boost_config(struct haptic *cs_haptic, unsigned char waveseq)
{
	int gain_comp = SPECIAL_LONG_RAM_GAIN_COMP;
	log_info("enter");
	if (cs_haptic->cust_boost_on == CS_SEL_BOOST_CFG_ON) {
		cs_haptic->func->set_wav_seq(cs_haptic, 0, 0x14);
		cs_haptic->func->set_wav_loop(cs_haptic, 0, 1);
		cs_haptic->func->set_wav_seq(cs_haptic, 1, waveseq);
		cs_haptic->func->set_wav_loop(cs_haptic, 1, 0x0f);
		cs_haptic->func->set_wav_seq(cs_haptic, 2, 0);
		cs_haptic->func->set_wav_loop(cs_haptic, 2, 0);
		gain_comp += cs_haptic->gain;
		if (gain_comp > MAX_GAIN)
			cs_haptic->func->set_gain(cs_haptic, 0xff);
		else
			cs_haptic->func->set_gain(cs_haptic, gain_comp);
		cs_haptic->special_long_vib = true;
	} else {
		cs_haptic->func->set_wav_seq(cs_haptic, 0, waveseq);
		cs_haptic->func->set_wav_loop(cs_haptic, 0, 0x0f);
		cs_haptic->func->set_wav_seq(cs_haptic, 1, 0);
		cs_haptic->func->set_wav_loop(cs_haptic, 1, 0);
	}
}

static void cs_haptic_ram_config(struct haptic *cs_haptic)
{
	unsigned char waveseq = 0;
	unsigned char waveloop = 0;
	unsigned char seq_idx = 0;

	if (!cs_haptic) {
		log_err("cs_haptic is null");
		return;
	}
	log_info("duration=%d,index = %d, effect_mode = %d", cs_haptic->duration, cs_haptic->index, cs_haptic->effect_mode);
	if (cs_haptic->effect_mode == SHORT_VIB_RAM_MODE) {
		waveseq = cs_haptic->index;
		waveloop = 0;
		cs_haptic->func->set_wav_seq(cs_haptic, 0, waveseq);
		cs_haptic->func->set_wav_loop(cs_haptic, 0, waveloop);
		cs_haptic->func->set_wav_seq(cs_haptic, 1, 0);
		cs_haptic->func->set_wav_loop(cs_haptic, 1, 0);
		log_info ("waveseq = %d", waveseq);
	} else {
		waveseq = cs_haptic->index;
		cs_long_ram_mode_boost_config(cs_haptic, waveseq);
	}
}

static int cs_haptic_play_repeat_seq(struct haptic *cs_haptic, unsigned char flag)
{
	log_info("enter");
	if (!cs_haptic) {
		log_err("cs_haptic is null");
		return -EINVAL;
	}
	if (flag) {
		cs_haptic->func->play_mode(cs_haptic, CS_RAM_LOOP_MODE);
		cs_haptic->func->play_go(cs_haptic, true);
	}
	return 0;
}

static void cs_boost_config(struct haptic *cs_haptic)
{
	if (!cs_haptic->info.is_enabled_lowpower_bst_config) {
		cs_haptic->func->auto_bst_enable(cs_haptic, CDV_MODE_RAM|CDV_MODE_RTP, CS_ENABLE);
		return ;
	}
	if (cs_haptic->emergency_mode_flag)
		cs_haptic->func->auto_bst_enable(cs_haptic, CDV_MODE_RAM|CDV_MODE_RTP, CS_DISABLE);
	else
		cs_haptic->func->auto_bst_enable(cs_haptic, CDV_MODE_RAM|CDV_MODE_RTP, CS_ENABLE);
}

static void ram_mode_sel_boost_play_cfg(struct haptic *cs_haptic)
{
	uint8_t temp_gain = 0;

	if (cs_haptic->cust_boost_on == CS_SEL_BOOST_CFG_ON) {
		cs_boost_config(cs_haptic);
		if (cs_haptic->special_long_vib == true) {
			temp_gain = cs_haptic->gain;
			cs_haptic->gain = cs_haptic->func->get_gain(cs_haptic);
			ram_vbat_comp(cs_haptic, true);
			cs_haptic->gain = temp_gain;
			cs_haptic->func->auto_bst_enable(cs_haptic, CDV_MODE_RAM, CS_DISABLE);
		} else {
			ram_vbat_comp(cs_haptic, false);
		}
		cs_haptic->special_long_vib = false;
		cs_haptic->func->play_mode(cs_haptic, CS_RAM_MODE);
		cs_haptic->func->play_go(cs_haptic, true);
		return;
	}
	if (cs_haptic->effect_mode == LONG_VIB_RAM_MODE) {
		ram_vbat_comp(cs_haptic, true);
		cs_haptic->func->auto_bst_enable(cs_haptic, CDV_MODE_RAM, CS_DISABLE);
		cs_haptic_play_repeat_seq(cs_haptic, true);
	} else {
		ram_vbat_comp(cs_haptic, false);
		cs_haptic->func->auto_bst_enable(cs_haptic, CDV_MODE_RAM, CS_ENABLE);
		cs_haptic->func->play_mode(cs_haptic, CS_RAM_MODE);
		cs_haptic->func->play_go(cs_haptic, true);
	}
}

static void vibrator_work_routine(struct work_struct *work)
{
	struct haptic *cs_haptic = NULL;

	if (!work) {
		log_err ("work is null");
		return;
	}

	log_info("enter");
	cs_haptic = container_of(work, struct haptic, vibrator_work);
	mutex_lock(&cs_haptic->lock);
	if (cs_haptic->f0_is_cali == HAPTIC_CHARGING_IS_CALIB) {
		cs_haptic->func->get_vbat(cs_haptic);
		f0_cali(cs_haptic);
		cs_haptic->f0_is_cali = 0;
		mutex_unlock(&cs_haptic->lock);
		return;
	}
	/* Enter standby mode */
	cs_haptic->func->play_stop(cs_haptic);
	if (cs_haptic->state) {
		log_info("state is true");
		if (cs_haptic->activate_mode == CS_HAPTIC_ACTIVATE_RAM_MODE) {
			log_info("activate_mode = CS_HAPTIC_ACTIVATE_RAM_MODE");
			cs_haptic_ram_config(cs_haptic);
			if (cs_haptic->duration <= CS_LONG_HAPTIC_RUNNING) { // max long vibra config
				log_info("duration <= CS_LONG_HAPTIC_RUNNINGe");
				ram_mode_sel_boost_play_cfg(cs_haptic);
			} else {
				ram_vbat_comp(cs_haptic, true);
				cs_haptic_play_repeat_seq(cs_haptic, true);
			}
		} else if (cs_haptic->activate_mode == CS_HAPTIC_ACTIVATE_CONT_MODE) {
			log_info("activate_mode == CS_HAPTIC_ACTIVATE_CONT_MODE");
			cs_haptic->func->cont_config(cs_haptic);
		} else {
			log_info("invalid mode, activate_mode: %d", cs_haptic->activate_mode);
		}
		/* run ms timer */
		if (cs_haptic->effect_mode == LONG_VIB_RAM_MODE || cs_haptic->activate_mode == CS_HAPTIC_ACTIVATE_CONT_MODE) {
			log_info("run ms timer! long vib start, duration: %d, gain = 0x%02x", cs_haptic->duration, cs_haptic->gain);
			hrtimer_start(&cs_haptic->timer, ktime_set(cs_haptic->duration / 1000,
						(cs_haptic->duration % 1000) * 1000000), HRTIMER_MODE_REL);
			__pm_wakeup_event(cs_haptic->ws, cs_haptic->duration + HAPTIC_WAKE_LOCK_GAP);
		}
	} else {
		log_info("state is false");
	}
	mutex_unlock(&cs_haptic->lock);
}

static void rtp_play(struct haptic *cs_haptic)
{
	uint8_t glb_state_val = 0;
	uint32_t buf_len = 0;

	log_info("enter");
	cs_haptic->rtp_cnt = 0;
	mutex_lock(&cs_haptic->rtp_lock);
	while ((!cs_haptic->func->rtp_get_fifo_afs(cs_haptic)) && (cs_haptic->play_mode == CS_RTP_MODE)) {
#ifdef CS_ENABLE_RTP_PRINT_LOG
		log_info("rtp cnt = %d", cs_haptic->rtp_cnt);
#endif
		if (!cs_rtp) {
			log_info("rtp is null, break!");
			break;
		}
		if (cs_haptic->rtp_cnt < (cs_haptic->ram.base_addr)) {
			if ((cs_rtp->len - cs_haptic->rtp_cnt) < (cs_haptic->ram.base_addr))
				buf_len = cs_rtp->len - cs_haptic->rtp_cnt;
			else
				buf_len = cs_haptic->ram.base_addr;
		} else if ((cs_rtp->len - cs_haptic->rtp_cnt) < (cs_haptic->ram.base_addr >> 2)) {
			buf_len = cs_rtp->len - cs_haptic->rtp_cnt;
		} else {
			buf_len = cs_haptic->ram.base_addr >> 2;
		}
#ifdef CS_ENABLE_RTP_PRINT_LOG
		log_info("buf_len = %d", buf_len);
#endif
		cs_haptic->func->set_rtp_data(cs_haptic, &cs_rtp->data[cs_haptic->rtp_cnt], buf_len);
		cs_haptic->rtp_cnt += buf_len;
		glb_state_val = cs_haptic->func->get_glb_state(cs_haptic);
		if ((cs_haptic->rtp_cnt == cs_rtp->len)|| (glb_state_val  == CDV_GLB_STATUS_STANDBY)) {
			if (cs_haptic->rtp_cnt != cs_rtp->len)
				log_err("rtp play suspend!");
			else
				log_info("rtp update complete!");
			cs_haptic->rtp_cnt = 0;
			break;
		}
	}
	if (cs_haptic->play_mode == CS_RTP_MODE)
		cs_haptic->func->set_rtp_aei(cs_haptic, true);

	log_info("exit");
	mutex_unlock(&cs_haptic->rtp_lock);
}

static int rtp_work_init(struct haptic *cs_haptic)
{
	int ret = -1;
	const struct firmware *rtp_file;

	log_info("enter");
	if (cs_haptic == NULL) {
		log_err("cs_haptic is null");
		return ret;
	}
	mutex_lock(&cs_haptic->rtp_lock);
	cs_haptic->rtp_routine_on = 1;
	/* fw loaded */
	ret = request_firmware(&rtp_file, cs_rtp_name[cs_haptic->rtp_file_num], cs_haptic->dev);
	if (ret < 0) {
		log_err("failed to read %s", cs_rtp_name[cs_haptic->rtp_file_num]);
		cs_haptic->rtp_routine_on = 0;
		mutex_unlock(&cs_haptic->rtp_lock);
		return ret;
	}
	cs_haptic->rtp_init = false;
	vfree(cs_rtp);
	cs_rtp = vmalloc(rtp_file->size + sizeof(int));
	if (!cs_rtp) {
		release_firmware(rtp_file);
		log_err("error allocating memory");
		cs_haptic->rtp_routine_on = 0;
		mutex_unlock(&cs_haptic->rtp_lock);
		return -1;
	}
	cs_rtp->len = rtp_file->size;
	log_info("rtp file:[%s] size = %dbytes", cs_rtp_name[cs_haptic->rtp_file_num], cs_rtp->len);
	memcpy(cs_rtp->data, rtp_file->data, rtp_file->size);
	mutex_unlock(&cs_haptic->rtp_lock);
	release_firmware(rtp_file);
	cs_haptic->rtp_init = true;
	cs_haptic->func->set_rtp_aei(cs_haptic, false);
	cs_haptic->func->irq_clear(cs_haptic);
	cs_haptic->func->play_stop(cs_haptic);
	/* gain */
	ram_vbat_comp(cs_haptic, false);
	/* boost voltage */
	if (cs_haptic->info.bst_vol_rtp <= cs_haptic->info.max_bst_vol && cs_haptic->info.bst_vol_rtp > 0)
		cs_haptic->func->set_bst_vol(cs_haptic, cs_haptic->info.bst_vol_rtp);
	else
		cs_haptic->func->set_bst_vol(cs_haptic, cs_haptic->vmax);
	return 0;
}

static void rtp_work_routine(struct work_struct *work)
{
	bool rtp_work_flag = false;
	uint8_t reg_val = 0;
	int cnt = 200;
	int ret = -1;
	struct haptic *cs_haptic = NULL;

	log_info("enter");
	if (work == NULL) {
		log_err("work is null");
		return;
	}
	cs_haptic = container_of(work, struct haptic, rtp_work);

	ret = rtp_work_init(cs_haptic);
	if (ret < 0) {
		log_err("rtp_work_init failed");
		return;
	}
	mutex_lock(&cs_haptic->lock);
	/* rtp mode config */
	cs_haptic->func->play_mode(cs_haptic, CS_RTP_MODE);
	cs_haptic->func->play_go(cs_haptic, true);
	usleep_range(2000, 2500);
	while (cnt) {
		reg_val = cs_haptic->func->get_glb_state(cs_haptic);
		if ((reg_val) == CDV_GLB_STATUS_RTP) {
			cnt = 0;
			rtp_work_flag = true;
			log_info("RTP_GO! glb_state=0x08");
		} else {
			cnt--;
			log_info("wait for RTP_GO, glb_state=0x%02X", reg_val);
		}
		usleep_range(2000, 2500);
	}
	if (rtp_work_flag) {
		rtp_play(cs_haptic);
	} else {
		/* enter standby mode */
		cs_haptic->func->play_stop(cs_haptic);
		log_err("failed to enter RTP_GO status!");
	}
	cs_haptic->rtp_routine_on = 0;
	mutex_unlock(&cs_haptic->lock);
}

static irqreturn_t irq_richtap_aei_handle(struct haptic *cs_haptic)
{
	log_info("go richtap_rtp work irq");
	if (!cs_haptic->func->get_irq_state(cs_haptic)) {
		/* modify next line second parameter according to ram.base_addr */
		richtap_update_fifo_data(cs_haptic,
			(cs_haptic->ram.base_addr >> 1)); // 1024
		while (!cs_haptic->func->rtp_get_fifo_afi(cs_haptic) &&
			atomic_read(&cs_haptic->richtap_rtp_mode) &&
			(cs_haptic->curr_buf->status == MMAP_BUF_DATA_VALID))
			richtap_update_fifo_data(cs_haptic,
				(cs_haptic->ram.base_addr >> 2)); // 512
	}
	if (cs_haptic->curr_buf->status == MMAP_BUF_DATA_INVALID) {
		cs_haptic->func->enable_gain(cs_haptic, 0);
		cs_haptic->func->set_rtp_aei(cs_haptic, false);
		atomic_set(&cs_haptic->richtap_rtp_mode, false);
	}

	atomic_set(&cs_haptic->is_process_irq_work, false);
	return IRQ_HANDLED;
}

static void process_rtp_fifo(struct haptic *cs_haptic)
{
	uint32_t buf_len = 0;
	uint8_t glb_state_val = 0;

	// Assume cs_rtp is defined elsewhere
	while ((!cs_haptic->func->rtp_get_fifo_afs(cs_haptic)) && (cs_haptic->play_mode == CS_RTP_MODE)) {
		mutex_lock(&cs_haptic->rtp_lock);
		if (!cs_haptic->rtp_cnt) {
			log_info("cs_haptic->rtp_cnt is 0!");
			mutex_unlock(&cs_haptic->rtp_lock);
			break;
		}
#ifdef CS_ENABLE_RTP_PRINT_LOG
		log_info("rtp mode fifo update, cnt=%d", cs_haptic->rtp_cnt);
#endif
		if (!cs_rtp) {
			log_info("cs_rtp is null, break!");
			mutex_unlock(&cs_haptic->rtp_lock);
			break;
		}
		buf_len = (cs_rtp->len - cs_haptic->rtp_cnt) < (cs_haptic->ram.base_addr >> 2) ?
			(cs_rtp->len - cs_haptic->rtp_cnt) : (cs_haptic->ram.base_addr >> 2);
		cs_haptic->func->set_rtp_data(cs_haptic, &cs_rtp->data[cs_haptic->rtp_cnt], buf_len);
		cs_haptic->rtp_cnt += buf_len;
		glb_state_val = cs_haptic->func->get_glb_state(cs_haptic);
		if ((cs_haptic->rtp_cnt == cs_rtp->len) || ((glb_state_val & CS_GLBRD_STATE_MASK) == CS_STATE_STANDBY)) {
			if (cs_haptic->rtp_cnt != cs_rtp->len)
				log_err("rtp play suspend!");
			else
				log_info("rtp update complete!");
			cs_haptic->rtp_routine_on = 0;
			cs_haptic->func->set_rtp_aei(cs_haptic, false);
			cs_haptic->rtp_cnt = 0;
			cs_haptic->rtp_init = false;
			mutex_unlock(&cs_haptic->rtp_lock);
			break;
		}
		mutex_unlock(&cs_haptic->rtp_lock);
	}
}

static irqreturn_t irq_handle(int irq, void *data)
{
	struct haptic *cs_haptic = (struct haptic *)data;

	if (!cs_haptic) {
		log_err ("data is null");
		return -EINVAL;
	}
	log_info("enter");
	log_info("irq process work");
	atomic_set(&cs_haptic->is_process_irq_work, true);
#ifdef AAC_RICHTAP_SUPPORT
	if (atomic_read(&cs_haptic->richtap_rtp_mode) == true)
		return irq_richtap_aei_handle(cs_haptic);
#endif
	if (!cs_haptic->func->get_irq_state(cs_haptic)) {
		log_info("cs_haptic rtp fifo almost empty");
		if (cs_haptic->rtp_init)
			process_rtp_fifo(cs_haptic);
		else
			log_info("init error");
	}
	if (cs_haptic->play_mode != CS_RTP_MODE)
		cs_haptic->func->set_rtp_aei(cs_haptic, false);
	return IRQ_HANDLED;
}

static int irq_config(struct device *dev, struct haptic *cs_haptic)
{
	int ret = -1;
	int irq_flags = 0;

	if (gpio_is_valid(cs_haptic->irq_gpio) && !(cs_haptic->flags & CS_FLAG_SKIP_INTERRUPTS)) {
		/* register irq handler */
		cs_haptic->func->interrupt_setup(cs_haptic);
		irq_flags = IRQF_TRIGGER_FALLING | IRQF_ONESHOT;
		ret = devm_request_threaded_irq(dev, gpio_to_irq(cs_haptic->irq_gpio), NULL, irq_handle, irq_flags,
										"cs_haptic", cs_haptic);
		if (ret != 0) {
			log_err("failed to request IRQ %d: %d", gpio_to_irq(cs_haptic->irq_gpio), ret);
			return ret;
		}
	}
	return ret;
}
static int cdv2625_data_check_sum(struct haptic *cs_haptic, const struct firmware *cont)
{
	uint16_t check_sum = 0;
	int i = 0;

	/* check sum */
	for (i = 2; i < cont->size; i++)
		check_sum += cont->data[i];
	if (check_sum != (uint16_t)((cont->data[0] << 8) | (cont->data[1]))) {
		log_err("check sum err: check_sum=0x%04x", check_sum);
		return -1;
	}
	log_info("check sum pass : 0x%04x", check_sum);
	cs_haptic->ram.check_sum = check_sum;
	return 0;
}

static void haptic_base_reg_init(struct haptic *cs_haptic)
{
	cs_haptic->func->play_mode(cs_haptic, CS_STANDBY_MODE);
	cs_haptic->func->set_pwm(cs_haptic, CS_PWM_12K);
	/* misc value init */
	cs_haptic->func->misc_para_init(cs_haptic);
	cs_haptic->func->protect_config(cs_haptic, 0x00, 0x00);
	cs_haptic->func->auto_bst_enable(cs_haptic, CDV_MODE_RAM, cs_haptic->info.is_enabled_auto_bst);
	cs_haptic->func->auto_bst_enable(cs_haptic, CDV_MODE_RTP, cs_haptic->info.is_enabled_auto_bst);

	/* vbat compensation */
	cs_haptic->func->vbat_mode_config(cs_haptic, CS_CONT_VBAT_HW_COMP_MODE);
	f0_cali(cs_haptic);
}

static void cdv2625_ram_update(struct haptic *cs_haptic, const struct firmware *cont)
{
	struct haptic_container *cs_fw;
	int ret = 0;

	/* ram update */
	cs_fw = kzalloc(cont->size + sizeof(int), GFP_KERNEL);
	if (!cs_fw) {
		log_err("Error allocating memory");
		return;
	}
	cs_fw->len = cont->size;
	memcpy(cs_fw->data, cont->data, cont->size);
	ret = cs_haptic->func->container_update(cs_haptic, cs_fw);
	if (ret) {
		log_err("ram firmware update failed!");
		cs_haptic->ramload_fail_cnt++;
		hw_reset(cs_haptic);
		haptic_base_reg_init(cs_haptic);
		schedule_delayed_work(&cs_haptic->ram_work, msecs_to_jiffies(2000));
	} else {
		cs_haptic->ram_init = true;
		cs_haptic->ramload_fail_cnt = 0;
		cs_haptic->ram.len = cs_fw->len - cs_haptic->ram.ram_shift;
		log_info("ram firmware update complete!");
		get_wave_num(cs_haptic);
	}
	kfree(cs_fw);
}

static void ram_load(const struct firmware *cont, void *context)
{
	int ret = 0;
	struct haptic *cs_haptic = NULL;

#ifdef CS_READ_BIN_FLEXBALLY
	static uint8_t load_cont = 0;
	int ram_timer_val = 1000;
	load_cont++;
#endif
	log_info("enter");
	if (context == NULL) {
		log_err("failed to read %s", cs_ram_name);
		return;
	}
	cs_haptic = context;
	if (!cont) {
		log_err("failed to read %s", cs_ram_name);
		release_firmware(cont);
#ifdef CS_READ_BIN_FLEXBALLY
		if (load_cont <= 2) {
			schedule_delayed_work(&cs_haptic->ram_work, msecs_to_jiffies(ram_timer_val));
			log_info("start hrtimer:load_cont%u", load_cont);
		}
#endif
		return;
	}
	log_info("loaded %s - size: %zu", cs_ram_name, cont ? cont->size : 0);
	ret = cdv2625_data_check_sum(cs_haptic, cont);
	if (ret) {
		release_firmware(cont);
		return;
	}
	cdv2625_ram_update(cs_haptic, cont);
	release_firmware(cont);
}

static int ram_update(struct haptic *cs_haptic)
{
	cs_haptic->ram_init = false;
	cs_haptic->rtp_init = false;
	return request_firmware_nowait(THIS_MODULE, FW_ACTION_HOTPLUG, cs_ram_name,
									cs_haptic->dev, GFP_KERNEL, cs_haptic, ram_load);
}

static void ram_work_routine(struct work_struct *work)
{
	struct haptic *cs_haptic = container_of(work, struct haptic, ram_work.work);

	log_info("enter");
	if (cs_haptic->ramload_fail_cnt > CS_I2C_RETRY_TIMES) {
		log_info("ram lib update fail over 3 times");
		return;
	}
	ram_update(cs_haptic);
}

static void ram_work_init(struct haptic *cs_haptic)
{
	int ram_timer_val = CS_RAM_WORK_DELAY_INTERVAL;

	log_info("enter");
	cs_haptic->ram_init = false;
	cs_haptic->rtp_init = false;
	INIT_DELAYED_WORK(&cs_haptic->ram_work, ram_work_routine);
	if (vibrator_get_enter_recovery_mode()) {
		log_info("recovery mode do not need load firmware");
		return;
	}
	schedule_delayed_work(&cs_haptic->ram_work, msecs_to_jiffies(ram_timer_val));
}

/*****************************************************
 *
 * node
 *
 *****************************************************/
static enum led_brightness brightness_get(struct led_classdev *cdev)
{
	struct haptic *cs_haptic = NULL;

	if (cdev == NULL) {
		log_err("cdev is null");
		return -EINVAL;
	}
	cs_haptic = container_of(cdev, struct haptic, vib_dev);
	return cs_haptic->amplitude;
}

static void brightness_set(struct led_classdev *cdev, enum led_brightness level)
{
	struct haptic *cs_haptic = container_of(cdev, struct haptic, vib_dev);

	log_info("enter");
	if (!cs_haptic->ram_init) {
		log_err("ram init failed, not allow to play!");
		return;
	}
	cs_haptic->amplitude = level;
	mutex_lock(&cs_haptic->lock);
	cs_haptic->func->play_stop(cs_haptic);
	if (cs_haptic->amplitude > 0) {
		ram_vbat_comp(cs_haptic, false);
		ram_play(cs_haptic, CS_RAM_MODE);
	}
	mutex_unlock(&cs_haptic->lock);
}

static ssize_t state_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	cdev_t *cdev = NULL;
	struct haptic *cs_haptic = NULL;

	if (!dev || !buf) {
		log_err ("dev or buf is null");
		return -EINVAL;
	}
	cdev = dev_get_drvdata(dev);
	if (!cdev) {
		log_err ("cdev is null");
		return -EINVAL;
	}
	cs_haptic = container_of(cdev, struct haptic, vib_dev);

	return snprintf(buf, PAGE_SIZE, "state = %d\n", cs_haptic->state);
}

static ssize_t state_store(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
	return count;
}

static ssize_t duration_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	cdev_t *cdev = NULL;
	struct haptic *cs_haptic = NULL;
	ktime_t time_rem;
	s64 time_ms = 0;

	if (!dev || !buf) {
		log_err ("dev or buf is null");
		return -EINVAL;
	}
	cdev = dev_get_drvdata(dev);
	if (!cdev) {
		log_err ("cdev is null");
		return -EINVAL;
	}
	cs_haptic = container_of(cdev, struct haptic, vib_dev);
	if (hrtimer_active(&cs_haptic->timer)) {
		time_rem = hrtimer_get_remaining(&cs_haptic->timer);
		time_ms = ktime_to_ms(time_rem);
	}
	return snprintf(buf, PAGE_SIZE, "duration = %lldms\n", time_ms);
}

static ssize_t duration_store(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
	cdev_t *cdev = NULL;
	struct haptic *cs_haptic = NULL;
	uint32_t val = 0;
	int rc = 0;

	if (!dev || !buf) {
		log_err ("dev or buf is null");
		return -EINVAL;
	}
	cdev = dev_get_drvdata(dev);
	if (!cdev) {
		log_err ("cdev is null");
		return -EINVAL;
	}
	cs_haptic = container_of(cdev, struct haptic, vib_dev);

	rc = kstrtouint(buf, 0, &val);
	if (rc < 0)
		return rc;
	/* setting 0 on duration is NOP for now */
	if (val <= 0)
		return count;
	cs_haptic->duration = val;
	cs_haptic->cust_boost_on = 0;
	__pm_wakeup_event(cs_haptic->ws, cs_haptic->duration + HAPTIC_WAKE_LOCK_GAP);
	return count;
}

static ssize_t activate_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	cdev_t *cdev = NULL;
	struct haptic *cs_haptic = NULL;

	if (!dev || !buf) {
		log_err ("dev or buf is null");
		return -EINVAL;
	}
	cdev = dev_get_drvdata(dev);
	if (!cdev) {
		log_err ("cdev is null");
		return -EINVAL;
	}
	cs_haptic = container_of(cdev, struct haptic, vib_dev);

	return snprintf(buf, PAGE_SIZE, "activate = %d\n", cs_haptic->state);
}

static void reset_valid_amp(struct haptic *cs_haptic)
{
	if (cs_haptic->set_amplitude_flag != 1)
		return;

	if (cs_haptic->info.is_enabled_lower_amp)
		cs_haptic->gain = long_amp_again_val[THIRD_LONG_HAPTIC_AMP];

	cs_haptic->set_amplitude_flag = 0;
}

static ssize_t activate_store(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
	cdev_t *cdev = NULL;
	struct haptic *cs_haptic = NULL;
	uint32_t val = 0;
	int rc = 0;

	if (!dev || !buf) {
		log_err ("dev or buf is null");
		return -EINVAL;
	}
	cdev = dev_get_drvdata(dev);
	if (!cdev) {
		log_err ("cdev is null");
		return -EINVAL;
	}
	cs_haptic = container_of(cdev, struct haptic, vib_dev);

	rc = kstrtouint(buf, 0, &val);
	if (rc < 0)
		return rc;
	log_info("value=%u", val);
	if (!cs_haptic->ram_init) {
		log_err("ram init failed, not allow to play!");
		return count;
	}
	mutex_lock(&cs_haptic->lock);
	if (hrtimer_active(&cs_haptic->timer))
		hrtimer_cancel(&cs_haptic->timer);
	cs_haptic->state = val;
	cs_haptic->effect_mode = LONG_VIB_RAM_MODE;
	cs_haptic->index = LONG_VIB_BOOST_EFFECT_ID;
	cs_haptic->gain = long_amp_again_val[MAX_LONG_HAPTIC_AMP];
	reset_valid_amp(cs_haptic);
	mutex_unlock(&cs_haptic->lock);
	schedule_work(&cs_haptic->vibrator_work);

	return count;
}

static ssize_t activate_mode_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	cdev_t *cdev = NULL;
	struct haptic *cs_haptic = NULL;

	if (!dev || !buf) {
		log_err ("dev or buf is null");
		return -EINVAL;
	}
	cdev = dev_get_drvdata(dev);
	if (!cdev) {
		log_err ("cdev is null");
		return -EINVAL;
	}
	cs_haptic = container_of(cdev, struct haptic, vib_dev);

	return snprintf(buf, PAGE_SIZE, "activate_mode = %u\n", cs_haptic->activate_mode);
}

static ssize_t activate_mode_store(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
	cdev_t *cdev = NULL;
	struct haptic *cs_haptic = NULL;
	uint32_t val = 0;
	int rc = 0;

	if (!dev || !buf) {
		log_err ("dev or buf is null");
		return -EINVAL;
	}
	cdev = dev_get_drvdata(dev);
	if (!cdev) {
		log_err ("cdev is null");
		return -EINVAL;
	}
	cs_haptic = container_of(cdev, struct haptic, vib_dev);

	rc = kstrtouint(buf, 0, &val);
	if (rc < 0)
		return rc;
	mutex_lock(&cs_haptic->lock);
	cs_haptic->activate_mode = val;
	mutex_unlock(&cs_haptic->lock);
	return count;
}

static ssize_t index_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	cdev_t *cdev = NULL;
	struct haptic *cs_haptic = NULL;

	if (!dev || !buf) {
		log_err ("dev or buf is null");
		return -EINVAL;
	}
	cdev = dev_get_drvdata(dev);
	if (!cdev) {
		log_err ("cdev is null");
		return -EINVAL;
	}
	cs_haptic = container_of(cdev, struct haptic, vib_dev);

	cs_haptic->func->get_wav_seq(cs_haptic, 1);
	cs_haptic->index = cs_haptic->seq[0];
	return snprintf(buf, PAGE_SIZE, "index = %d\n", cs_haptic->index);
}

static ssize_t index_store(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
	cdev_t *cdev = NULL;
	struct haptic *cs_haptic = NULL;
	uint32_t val = 0;
	int rc = 0;

	if (!dev || !buf) {
		log_err ("dev or buf is null");
		return -EINVAL;
	}
	cdev = dev_get_drvdata(dev);
	if (!cdev) {
		log_err ("cdev is null");
		return -EINVAL;
	}
	cs_haptic = container_of(cdev, struct haptic, vib_dev);

	rc = kstrtouint(buf, 0, &val);
	if (rc < 0)
		return rc;
	if (val > cs_haptic->ram.ram_num) {
		log_err("input value out of range!");
		return count;
	}
	log_info("value=%u", val);
	mutex_lock(&cs_haptic->lock);
	cs_haptic->index = val;
	cs_haptic->func->set_repeat_seq(cs_haptic, cs_haptic->index);
	mutex_unlock(&cs_haptic->lock);
	return count;
}

static ssize_t vmax_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	cdev_t *cdev = NULL;
	struct haptic *cs_haptic = NULL;

	if (!dev || !buf) {
		log_err ("dev or buf is null");
		return -EINVAL;
	}
	cdev = dev_get_drvdata(dev);
	if (!cdev) {
		log_err ("cdev is null");
		return -EINVAL;
	}
	cs_haptic = container_of(cdev, struct haptic, vib_dev);

	return snprintf(buf, PAGE_SIZE, "vmax = 0x%02X\n", cs_haptic->vmax);
}

static ssize_t vmax_store(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
	cdev_t *cdev = NULL;
	struct haptic *cs_haptic = NULL;
	uint32_t val = 0;
	int rc = 0;

	if (!dev || !buf) {
		log_err ("dev or buf is null");
		return -EINVAL;
	}
	cdev = dev_get_drvdata(dev);
	if (!cdev) {
		log_err ("cdev is null");
		return -EINVAL;
	}
	cs_haptic = container_of(cdev, struct haptic, vib_dev);

	rc = kstrtouint(buf, 0, &val);
	if (rc < 0)
		return rc;
	log_info("value=%u", val);
	mutex_lock(&cs_haptic->lock);
	cs_haptic->vmax = val;
	cs_haptic->func->set_bst_vol(cs_haptic, cs_haptic->vmax);
	mutex_unlock(&cs_haptic->lock);
	return count;
}

static ssize_t gain_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	cdev_t *cdev = NULL;
	struct haptic *cs_haptic = NULL;

	if (!dev || !buf) {
		log_err ("dev or buf is null");
		return -EINVAL;
	}
	cdev = dev_get_drvdata(dev);
	if (!cdev) {
		log_err ("cdev is null");
		return -EINVAL;
	}
	cs_haptic = container_of(cdev, struct haptic, vib_dev);

	return snprintf(buf, PAGE_SIZE, "gain = 0x%02X\n", cs_haptic->gain);
}

static ssize_t gain_store(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
	cdev_t *cdev = NULL;
	struct haptic *cs_haptic = NULL;
	uint32_t val = 0;
	int rc = 0;

	if (!dev || !buf) {
		log_err ("dev or buf is null");
		return -EINVAL;
	}
	cdev = dev_get_drvdata(dev);
	if (!cdev) {
		log_err ("cdev is null");
		return -EINVAL;
	}
	cs_haptic = container_of(cdev, struct haptic, vib_dev);

	rc = kstrtouint(buf, 0, &val);
	if (rc < 0)
		return rc;
	log_info("value=0x%02x", val);
	mutex_lock(&cs_haptic->lock);
	cs_haptic->gain = val;
	cs_haptic->func->set_gain(cs_haptic, cs_haptic->gain);
	mutex_unlock(&cs_haptic->lock);
	return count;
}

static ssize_t seq_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	uint8_t i = 0;
	size_t count = 0;
	cdev_t *cdev = NULL;
	struct haptic *cs_haptic = NULL;

	if (!dev || !buf) {
		log_err ("dev or buf is null");
		return -EINVAL;
	}
	cdev = dev_get_drvdata(dev);
	if (!cdev) {
		log_err ("cdev is null");
		return -EINVAL;
	}
	cs_haptic = container_of(cdev, struct haptic, vib_dev);

	cs_haptic->func->get_wav_seq(cs_haptic, CS_SEQUENCER_SIZE);
	for (i = 0; i < CS_SEQUENCER_SIZE; i++)
		count += snprintf(buf + count, PAGE_SIZE - count, "seq%u = %u\n", i + 1, cs_haptic->seq[i]);
	return count;
}

static ssize_t seq_store(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
	cdev_t *cdev = NULL;
	struct haptic *cs_haptic = NULL;
	uint32_t databuf[2] = { 0, 0 };

	if (!dev || !buf) {
		log_err ("dev or buf is null");
		return -EINVAL;
	}
	cdev = dev_get_drvdata(dev);
	if (!cdev) {
		log_err ("cdev is null");
		return -EINVAL;
	}
	cs_haptic = container_of(cdev, struct haptic, vib_dev);

	if (sscanf(buf, "%x %x", &databuf[0], &databuf[1]) == 2) {
		if (databuf[0] >= CS_SEQUENCER_SIZE ||
			databuf[1] > cs_haptic->ram.ram_num) {
			log_err("input value out of range!");
			return count;
		}
		log_info("seq%d=0x%02X", databuf[0], databuf[1]);
		mutex_lock(&cs_haptic->lock);
		cs_haptic->seq[databuf[0]] = (uint8_t)databuf[1];
		cs_haptic->func->set_wav_seq(cs_haptic, (uint8_t)databuf[0], cs_haptic->seq[databuf[0]]);
		mutex_unlock(&cs_haptic->lock);
	}
	return count;
}

static ssize_t reg_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	cdev_t *cdev = NULL;
	struct haptic *cs_haptic = NULL;
	ssize_t len = 0;

	if (!dev || !buf) {
		log_err ("dev or buf is null");
		return -EINVAL;
	}
	cdev = dev_get_drvdata(dev);
	if (!cdev) {
		log_err ("cdev is null");
		return -EINVAL;
	}
	cs_haptic = container_of(cdev, struct haptic, vib_dev);

	len = cs_haptic->func->get_reg(cs_haptic, len, buf);
	return len;
}

static ssize_t reg_store(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
	uint8_t val = 0;
	cdev_t *cdev = NULL;
	struct haptic *cs_haptic = NULL;
	uint32_t databuf[2] = { 0, 0 };

	if (!dev || !buf) {
		log_err ("dev or buf is null");
		return -EINVAL;
	}
	cdev = dev_get_drvdata(dev);
	if (!cdev) {
		log_err ("cdev is null");
		return -EINVAL;
	}
	cs_haptic = container_of(cdev, struct haptic, vib_dev);

	if (sscanf(buf, "%x %x", &databuf[0], &databuf[1]) == 2) {
		val = (uint8_t)databuf[1];
		cs_i2c_byte_write(cs_haptic, (uint8_t)databuf[0], &val, CS_I2C_BYTE_ONE);
	}
	return count;
}

static ssize_t rtp_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	cdev_t *cdev = NULL;
	struct haptic *cs_haptic = NULL;
	ssize_t len = 0;

	if (!dev || !buf) {
		log_err ("dev or buf is null");
		return -EINVAL;
	}
	cdev = dev_get_drvdata(dev);
	if (!cdev) {
		log_err ("cdev is null");
		return -EINVAL;
	}
	cs_haptic = container_of(cdev, struct haptic, vib_dev);

	len += snprintf(buf + len, PAGE_SIZE - len, "rtp_cnt = %d\n", cs_haptic->rtp_cnt);
	return len;
}

static ssize_t cs_rtp_store(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
	cdev_t *cdev = NULL;
	struct haptic *cs_haptic = NULL;
	uint32_t val = 0;
	int rc = 0;
	int len = 0;

	if (!dev || !buf) {
		log_err ("dev or buf is null");
		return -EINVAL;
	}
	cdev = dev_get_drvdata(dev);
	if (!cdev) {
		log_err ("cdev is null");
		return -EINVAL;
	}
	cs_haptic = container_of(cdev, struct haptic, vib_dev);

	rc = kstrtouint(buf, 0, &val);
	if (rc < 0) {
		log_err("kstrtouint fail");
		return rc;
	}
	mutex_lock(&cs_haptic->lock);
	cs_haptic->func->play_stop(cs_haptic);
	cs_haptic->func->set_rtp_aei(cs_haptic, false);
	cs_haptic->func->irq_clear(cs_haptic);
	len += snprintf(cs_rtp_idx_name, CS_RTP_NAME_MAX - 1, "cs_%u.bin", val);
	log_info(" get rtp name = %s, len = %d", cs_rtp_idx_name, len);
	schedule_work(&cs_haptic->rtp_work);

	mutex_unlock(&cs_haptic->lock);
	return count;
}

static ssize_t ram_update_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	cdev_t *cdev = NULL;
	struct haptic *cs_haptic = NULL;
	ssize_t len = 0;

	if (!dev || !buf) {
		log_err ("dev or buf is null");
		return -EINVAL;
	}
	cdev = dev_get_drvdata(dev);
	if (!cdev) {
		log_err ("cdev is null");
		return -EINVAL;
	}
	cs_haptic = container_of(cdev, struct haptic, vib_dev);

	/* RAMINIT Enable */
	cs_haptic->func->ram_init(cs_haptic, true);
	cs_haptic->func->play_stop(cs_haptic);
	cs_haptic->func->set_ram_addr(cs_haptic);
	len += snprintf(buf + len, PAGE_SIZE - len, "cs_haptic_ram:\n");
	len += cs_haptic->func->get_ram_data(cs_haptic, buf);
	len += snprintf(buf + len, PAGE_SIZE - len, "\n");
	/* RAMINIT Disable */
	cs_haptic->func->ram_init(cs_haptic, false);
	return len;
}

static ssize_t ram_update_store(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
	cdev_t *cdev = NULL;
	struct haptic *cs_haptic = NULL;
	uint32_t val = 0;
	int rc = 0;

	if (!dev || !buf) {
		log_err ("dev or buf is null");
		return -EINVAL;
	}
	cdev = dev_get_drvdata(dev);
	if (!cdev) {
		log_err ("cdev is null");
		return -EINVAL;
	}
	cs_haptic = container_of(cdev, struct haptic, vib_dev);

	rc = kstrtouint(buf, 0, &val);
	if (rc < 0)
		return rc;
	if (val)
		ram_update(cs_haptic);
	return count;
}

static ssize_t ram_num_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	cdev_t *cdev = NULL;
	struct haptic *cs_haptic = NULL;
	ssize_t len = 0;

	if (!dev || !buf) {
		log_err ("dev or buf is null");
		return -EINVAL;
	}
	cdev = dev_get_drvdata(dev);
	if (!cdev) {
		log_err ("cdev is null");
		return -EINVAL;
	}
	cs_haptic = container_of(cdev, struct haptic, vib_dev);

	get_wave_num(cs_haptic);
	len += snprintf(buf + len, PAGE_SIZE - len, "ram_num = %u\n", cs_haptic->ram.ram_num);
	return len;
}

static ssize_t f0_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	cdev_t *cdev = NULL;
	struct haptic *cs_haptic = NULL;
	ssize_t len = 0;

	if (!dev || !buf) {
		log_err ("dev or buf is null");
		return -EINVAL;
	}
	cdev = dev_get_drvdata(dev);
	if (!cdev) {
		log_err ("cdev is null");
		return -EINVAL;
	}
	cs_haptic = container_of(cdev, struct haptic, vib_dev);

	mutex_lock(&cs_haptic->lock);
	cs_haptic->func->get_f0(cs_haptic);
	mutex_unlock(&cs_haptic->lock);
	len += snprintf(buf + len, PAGE_SIZE - len, "lra_f0 = %u cont_f0 = %u\n", cs_haptic->f0, cs_haptic->cont_f0);
	return len;
}

static ssize_t cali_val_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	cdev_t *cdev = NULL;
	struct haptic *cs_haptic = NULL;
	ssize_t len = 0;

	if (!dev || !buf) {
		log_err ("dev or buf is null");
		return -EINVAL;
	}
	cdev = dev_get_drvdata(dev);
	if (!cdev) {
		log_err ("cdev is null");
		return -EINVAL;
	}
	cs_haptic = container_of(cdev, struct haptic, vib_dev);

	if (!buf || !cs_haptic)
		return 0;
	log_info("f0 val = %u", cs_haptic->f0);

#ifdef F0_CONT_F0
	len += snprintf(buf + len, PAGE_SIZE - len, "cs_haptic f0 = %u\n", cs_haptic->f0);
#endif
	len += snprintf(buf + len, PAGE_SIZE - len, "%u", cs_haptic->f0);
	return len;
}

static ssize_t cali_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	cdev_t *cdev = NULL;
	struct haptic *cs_haptic = NULL;
	ssize_t len = 0;
	int cal_rst;
	uint32_t f0_min = 0;
	uint32_t f0_max = 0;

	if (!dev || !buf) {
		log_err ("dev or buf is null");
		return -EINVAL;
	}
	cdev = dev_get_drvdata(dev);
	if (!cdev) {
		log_err ("cdev is null");
		return -EINVAL;
	}
	cs_haptic = container_of(cdev, struct haptic, vib_dev);
	f0_min = cs_haptic->info.f0_pre * (100 - cs_haptic->info.f0_cali_percent) / 100;
	f0_max = cs_haptic->info.f0_pre * (100 + cs_haptic->info.f0_cali_percent) / 100;

	if (cs_haptic->f0 < f0_min || cs_haptic->f0 > f0_max)
		cal_rst = 0;
	else
		cal_rst = 1;

	len += snprintf(buf + len, PAGE_SIZE - len, "%d", cal_rst);
	return len;
}

static ssize_t cali_store(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
	cdev_t *cdev = NULL;
	struct haptic *cs_haptic = NULL;
	uint32_t val = 0;
	int rc = 0;

	if (!dev || !buf) {
		log_err ("dev or buf is null");
		return -EINVAL;
	}
	cdev = dev_get_drvdata(dev);
	if (!cdev) {
		log_err ("cdev is null");
		return -EINVAL;
	}
	cs_haptic = container_of(cdev, struct haptic, vib_dev);

	rc = kstrtouint(buf, 0, &val);
	if (rc < 0)
		return rc;
	if (val) {
		mutex_lock(&cs_haptic->lock);
		f0_cali(cs_haptic);
		mutex_unlock(&cs_haptic->lock);
	}
	return count;
}

static ssize_t cont_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	cdev_t *cdev = NULL;
	struct haptic *cs_haptic = NULL;
	ssize_t len = 0;

	if (!dev || !buf) {
		log_err ("dev or buf is null");
		return -EINVAL;
	}
	cdev = dev_get_drvdata(dev);
	if (!cdev) {
		log_err ("cdev is null");
		return -EINVAL;
	}
	cs_haptic = container_of(cdev, struct haptic, vib_dev);

	cs_haptic->func->read_f0(cs_haptic);
	len += snprintf(buf + len, PAGE_SIZE - len, "cont_f0 = %u\n", cs_haptic->cont_f0);
	return len;
}

static ssize_t cont_store(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
	cdev_t *cdev = NULL;
	struct haptic *cs_haptic = NULL;
	uint32_t val = 0;
	int rc = 0;

	if (!dev || !buf) {
		log_err ("dev or buf is null");
		return -EINVAL;
	}
	cdev = dev_get_drvdata(dev);
	if (!cdev) {
		log_err ("cdev is null");
		return -EINVAL;
	}
	cs_haptic = container_of(cdev, struct haptic, vib_dev);

	rc = kstrtouint(buf, 0, &val);
	if (rc < 0)
		return rc;

	cs_haptic->func->play_stop(cs_haptic);
	if (val)
		cs_haptic->func->cont_config(cs_haptic);
	return count;
}

static ssize_t vbat_monitor_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	cdev_t *cdev = NULL;
	struct haptic *cs_haptic = NULL;
	ssize_t len = 0;

	if (!dev || !buf) {
		log_err ("dev or buf is null");
		return -EINVAL;
	}
	cdev = dev_get_drvdata(dev);
	if (!cdev) {
		log_err ("cdev is null");
		return -EINVAL;
	}
	cs_haptic = container_of(cdev, struct haptic, vib_dev);

	mutex_lock(&cs_haptic->lock);
	cs_haptic->func->get_vbat(cs_haptic);
	len += snprintf(buf + len, PAGE_SIZE - len, "vbat_monitor = %u\n", cs_haptic->vbat);
	mutex_unlock(&cs_haptic->lock);

	return len;
}

static ssize_t lra_resistance_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	cdev_t *cdev = NULL;
	struct haptic *cs_haptic = NULL;
	ssize_t len = 0;

	if (!dev || !buf) {
		log_err ("dev or buf is null");
		return -EINVAL;
	}
	cdev = dev_get_drvdata(dev);
	if (!cdev) {
		log_err ("cdev is null");
		return -EINVAL;
	}
	cs_haptic = container_of(cdev, struct haptic, vib_dev);

	cs_haptic->func->get_lra_resistance(cs_haptic);
	len += snprintf(buf + len, PAGE_SIZE - len, "lra_resistance = %u\n", cs_haptic->lra);
	return len;
}

static ssize_t auto_boost_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	cdev_t *cdev = NULL;
	struct haptic *cs_haptic = NULL;
	ssize_t len = 0;

	if (!dev || !buf) {
		log_err ("dev or buf is null");
		return -EINVAL;
	}
	cdev = dev_get_drvdata(dev);
	if (!cdev) {
		log_err ("cdev is null");
		return -EINVAL;
	}
	cs_haptic = container_of(cdev, struct haptic, vib_dev);

	len += snprintf(buf + len, PAGE_SIZE - len, "auto_boost = %u\n", cs_haptic->auto_boost);

	return len;
}

static ssize_t auto_boost_store(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
	cdev_t *cdev = NULL;
	struct haptic *cs_haptic = NULL;
	uint32_t val = 0;
	int rc = 0;

	if (!dev || !buf) {
		log_err ("dev or buf is null");
		return -EINVAL;
	}
	cdev = dev_get_drvdata(dev);
	if (!cdev) {
		log_err ("cdev is null");
		return -EINVAL;
	}
	cs_haptic = container_of(cdev, struct haptic, vib_dev);

	rc = kstrtouint(buf, 0, &val);
	if (rc < 0)
		return rc;

	mutex_lock(&cs_haptic->lock);
	cs_haptic->func->play_stop(cs_haptic);
	cs_haptic->func->auto_bst_enable(cs_haptic, CDV_MODE_RAM, val);
	cs_haptic->func->auto_bst_enable(cs_haptic, CDV_MODE_RTP, val);
	mutex_unlock(&cs_haptic->lock);

	return count;
}

static ssize_t prct_mode_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	cdev_t *cdev = NULL;
	struct haptic *cs_haptic = NULL;
	ssize_t len = 0;
	uint8_t reg_val = 0;

	if (!dev || !buf) {
		log_err ("dev or buf is null");
		return -EINVAL;
	}
	cdev = dev_get_drvdata(dev);
	if (!cdev) {
		log_err ("cdev is null");
		return -EINVAL;
	}
	cs_haptic = container_of(cdev, struct haptic, vib_dev);

	reg_val = cs_haptic->func->get_prctmode(cs_haptic);
	len += snprintf(buf + len, PAGE_SIZE - len, "prctmode = %u\n", reg_val);
	return len;
}

static ssize_t prct_mode_store(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
	cdev_t *cdev = NULL;
	struct haptic *cs_haptic = NULL;
	uint32_t databuf[2] = { 0, 0 };
	uint32_t addr = 0;
	uint32_t val = 0;

	if (!dev || !buf) {
		log_err ("dev or buf is null");
		return -EINVAL;
	}
	cdev = dev_get_drvdata(dev);
	if (!cdev) {
		log_err ("cdev is null");
		return -EINVAL;
	}
	cs_haptic = container_of(cdev, struct haptic, vib_dev);

	if (sscanf(buf, "%x %x", &databuf[0], &databuf[1]) == 2) {
		addr = databuf[0];
		val = databuf[1];
		mutex_lock(&cs_haptic->lock);
		cs_haptic->func->protect_config(cs_haptic, addr, val);
		mutex_unlock(&cs_haptic->lock);
	}
	return count;
}

static ssize_t ram_vbat_comp_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	cdev_t *cdev = NULL;
	struct haptic *cs_haptic = NULL;
	ssize_t len = 0;

	if (!dev || !buf) {
		log_err ("dev or buf is null");
		return -EINVAL;
	}
	cdev = dev_get_drvdata(dev);
	if (!cdev) {
		log_err ("cdev is null");
		return -EINVAL;
	}
	cs_haptic = container_of(cdev, struct haptic, vib_dev);

	len += snprintf(buf + len, PAGE_SIZE - len, "ram_vbat_comp = %u\n", cs_haptic->ram_vbat_comp);

	return len;
}

static ssize_t ram_vbat_comp_store(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
	cdev_t *cdev = NULL;
	struct haptic *cs_haptic = NULL;
	uint32_t val = 0;
	int rc = 0;

	if (!dev || !buf) {
		log_err ("dev or buf is null");
		return -EINVAL;
	}
	cdev = dev_get_drvdata(dev);
	if (!cdev) {
		log_err ("cdev is null");
		return -EINVAL;
	}
	cs_haptic = container_of(cdev, struct haptic, vib_dev);

	rc = kstrtouint(buf, 0, &val);
	if (rc < 0)
		return rc;
	mutex_lock(&cs_haptic->lock);
	if (val)
		cs_haptic->ram_vbat_comp = CS_RAM_VBAT_COMP_ENABLE;
	else
		cs_haptic->ram_vbat_comp = CS_RAM_VBAT_COMP_DISABLE;
	mutex_unlock(&cs_haptic->lock);

	return count;
}

static ssize_t osc_cali_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	cdev_t *cdev = NULL;
	struct haptic *cs_haptic = NULL;
	ssize_t len = 0;

	if (!dev || !buf) {
		log_err ("dev or buf is null");
		return -EINVAL;
	}
	cdev = dev_get_drvdata(dev);
	if (!cdev) {
		log_err ("cdev is null");
		return -EINVAL;
	}
	cs_haptic = container_of(cdev, struct haptic, vib_dev);

	len += snprintf(buf + len, PAGE_SIZE - len, "osc_cali_data = 0x%02X\n", cs_haptic->osc_cali_data);

	return len;
}

static ssize_t osc_cali_store(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
	cdev_t *cdev = NULL;
	struct haptic *cs_haptic = NULL;
	uint32_t val = 0;
	int rc = 0;

	if (!dev || !buf) {
		log_err ("dev or buf is null");
		return -EINVAL;
	}
	cdev = dev_get_drvdata(dev);
	if (!cdev) {
		log_err ("cdev is null");
		return -EINVAL;
	}
	cs_haptic = container_of(cdev, struct haptic, vib_dev);

	rc = kstrtouint(buf, 0, &val);
	if (rc < 0)
		return rc;

	return count;
}

static ssize_t gun_type_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	cdev_t *cdev = NULL;
	struct haptic *cs_haptic = NULL;

	if (!dev || !buf) {
		log_err ("dev or buf is null");
		return -EINVAL;
	}
	cdev = dev_get_drvdata(dev);
	if (!cdev) {
		log_err ("cdev is null");
		return -EINVAL;
	}
	cs_haptic = container_of(cdev, struct haptic, vib_dev);

	return snprintf(buf, PAGE_SIZE, "0x%02x\n", cs_haptic->gun_type);
}

static ssize_t gun_type_store(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
	cdev_t *cdev = NULL;
	struct haptic *cs_haptic = NULL;
	uint32_t val = 0;
	int rc = 0;

	if (!dev || !buf) {
		log_err ("dev or buf is null");
		return -EINVAL;
	}
	cdev = dev_get_drvdata(dev);
	if (!cdev) {
		log_err ("cdev is null");
		return -EINVAL;
	}
	cs_haptic = container_of(cdev, struct haptic, vib_dev);

	rc = kstrtouint(buf, 0, &val);
	if (rc < 0)
		return rc;

	log_info("value=%u", val);

	mutex_lock(&cs_haptic->lock);
	cs_haptic->gun_type = val;
	mutex_unlock(&cs_haptic->lock);
	return count;
}

static ssize_t bullet_nr_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	cdev_t *cdev = NULL;
	struct haptic *cs_haptic = NULL;

	if (!dev || !buf) {
		log_err ("dev or buf is null");
		return -EINVAL;
	}
	cdev = dev_get_drvdata(dev);
	if (!cdev) {
		log_err ("cdev is null");
		return -EINVAL;
	}
	cs_haptic = container_of(cdev, struct haptic, vib_dev);

	return snprintf(buf, PAGE_SIZE, "0x%02x\n", cs_haptic->bullet_nr);
}

static ssize_t bullet_nr_store(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
	cdev_t *cdev = NULL;
	struct haptic *cs_haptic = NULL;
	uint32_t val = 0;
	int rc = 0;

	if (!dev || !buf) {
		log_err ("dev or buf is null");
		return -EINVAL;
	}
	cdev = dev_get_drvdata(dev);
	if (!cdev) {
		log_err ("cdev is null");
		return -EINVAL;
	}
	cs_haptic = container_of(cdev, struct haptic, vib_dev);

	rc = kstrtouint(buf, 0, &val);
	if (rc < 0)
		return rc;
	log_info("value=%u", val);
	mutex_lock(&cs_haptic->lock);
	cs_haptic->bullet_nr = val;
	mutex_unlock(&cs_haptic->lock);
	return count;
}

static int rw_cmd_parse(struct haptic *cs_haptic, const char *buf, uint32_t flag, uint32_t reg_num, uint32_t reg_addr)
{
	int i = 0;
	int rc = 0;
	char data_buf[5] = { 0 };
	uint8_t value = 0;

	if (flag == CS_SEQ_WRITE) {
		if ((reg_num * 5) != (strlen(buf) - 3 * 5)) {
			log_err("param error");
			return -ERANGE;
		}
		for (i = 0; i < reg_num; i++) {
			memcpy(data_buf, &buf[15 + i * 5], 4);
			data_buf[4] = '\0';
			rc = kstrtou8(data_buf, 0, &value);
			if (rc < 0) {
				log_err("param error");
				return -ERANGE;
			}
			cs_haptic->i2c_info.reg_data[i] = value;
		}
		cs_i2c_byte_write(cs_haptic, (uint8_t)reg_addr, cs_haptic->i2c_info.reg_data, reg_num);
	} else if (flag == CS_SEQ_READ) {
		cs_i2c_byte_read(cs_haptic, reg_addr, cs_haptic->i2c_info.reg_data, reg_num);
	}
	return 0;
}

static ssize_t f0_save_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	ssize_t len = 0;
	cdev_t *cdev = NULL;
	struct haptic *cs_haptic = NULL;

	if (!dev || !buf) {
		log_err ("dev or buf is null");
		return -EINVAL;
	}
	cdev = dev_get_drvdata(dev);
	if (!cdev) {
		log_err ("cdev is null");
		return -EINVAL;
	}
	cs_haptic = container_of(cdev, struct haptic, vib_dev);

	len += snprintf(buf + len, PAGE_SIZE - len, "f0_cali_data = 0x%02X\n", cs_haptic->f0_cali_data);

	return len;
}

static ssize_t f0_save_store(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
	uint32_t val = 0;
	int rc = 0;
	cdev_t *cdev = NULL;
	struct haptic *cs_haptic = NULL;

	if (!dev || !buf) {
		log_err ("dev or buf is null");
		return -EINVAL;
	}
	cdev = dev_get_drvdata(dev);
	if (!cdev) {
		log_err ("cdev is null");
		return -EINVAL;
	}
	cs_haptic = container_of(cdev, struct haptic, vib_dev);

	rc = kstrtouint(buf, 0, &val);
	if (rc < 0)
		return rc;
	cs_haptic->f0_cali_data = val;
	return count;
}


static ssize_t osc_save_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	ssize_t len = 0;
	cdev_t *cdev = NULL;
	struct haptic *cs_haptic = NULL;

	if (!dev || !buf) {
		log_err ("dev or buf is null");
		return -EINVAL;
	}
	cdev = dev_get_drvdata(dev);
	if (!cdev) {
		log_err ("cdev is null");
		return -EINVAL;
	}
	cs_haptic = container_of(cdev, struct haptic, vib_dev);

	len += snprintf(buf + len, PAGE_SIZE - len, "osc_cali_data = 0x%02X\n", cs_haptic->osc_cali_data);

	return len;
}

static ssize_t osc_save_store(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
	uint32_t val = 0;
	int rc = 0;
	cdev_t *cdev = NULL;
	struct haptic *cs_haptic = NULL;

	if (!dev || !buf) {
		log_err ("dev or buf is null");
		return -EINVAL;
	}
	cdev = dev_get_drvdata(dev);
	if (!cdev) {
		log_err ("cdev is null");
		return -EINVAL;
	}
	cs_haptic = container_of(cdev, struct haptic, vib_dev);

	rc = kstrtouint(buf, 0, &val);
	if (rc < 0)
		return rc;
	cs_haptic->osc_cali_data = val;
	return count;
}
static ssize_t set_amplitude_store(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
	cdev_t *cdev = NULL;
	struct haptic *cs_haptic = NULL;
	uint32_t val = 0;
	int rc = 0;

	if (!dev || !buf) {
		log_err ("dev or buf is null");
		return -EINVAL;
	}
	cdev = dev_get_drvdata(dev);
	if (!cdev) {
		log_err ("cdev is null");
		return -EINVAL;
	}
	cs_haptic = container_of(cdev, struct haptic, vib_dev);

	rc = kstrtouint(buf, 0, &val);
	if (rc < 0)
		return rc;
	log_info("value=%u", val);
	if (cs_haptic->info.is_enabled_lower_amp) {
		if (val == ADJUST_GAIN_CAM_RESONANCES)
			cs_haptic->set_amplitude_flag = 1;
		else
			cs_haptic->set_amplitude_flag = 0;
	}
	return count;
}

static DEVICE_ATTR(f0, S_IWUSR | S_IRUGO, f0_show, NULL);
static DEVICE_ATTR(cali_val, S_IWUSR | S_IRUGO, cali_val_show, NULL);
static DEVICE_ATTR(seq, S_IWUSR | S_IRUGO, seq_show, seq_store);
static DEVICE_ATTR(reg, S_IWUSR | S_IRUGO, reg_show, reg_store);
static DEVICE_ATTR(vmax, S_IWUSR | S_IRUGO, vmax_show, vmax_store);
static DEVICE_ATTR(gain, S_IWUSR | S_IRUGO, gain_show, gain_store);
static DEVICE_ATTR(rtp, S_IWUSR | S_IRUGO, rtp_show, cs_rtp_store);
static DEVICE_ATTR(vibrator_calib, S_IWUSR | S_IRUGO, cali_show, cali_store);
static DEVICE_ATTR(cont, S_IWUSR | S_IRUGO, cont_show, cont_store);
static DEVICE_ATTR(state, S_IWUSR | S_IRUGO, state_show, state_store);
static DEVICE_ATTR(index, S_IWUSR | S_IRUGO, index_show, index_store);
static DEVICE_ATTR(ram_num, S_IWUSR | S_IRUGO, ram_num_show, NULL);
static DEVICE_ATTR(duration, S_IWUSR | S_IRUGO, duration_show, duration_store);
static DEVICE_ATTR(activate, S_IWUSR | S_IRUGO, activate_show, activate_store);
static DEVICE_ATTR(osc_cali, S_IWUSR | S_IRUGO, osc_cali_show, osc_cali_store);
static DEVICE_ATTR(gun_type, S_IWUSR | S_IRUGO, gun_type_show, gun_type_store);
static DEVICE_ATTR(prctmode, S_IWUSR | S_IRUGO, prct_mode_show, prct_mode_store);
static DEVICE_ATTR(bullet_nr, S_IWUSR | S_IRUGO, bullet_nr_show, bullet_nr_store);
static DEVICE_ATTR(auto_boost, S_IWUSR | S_IRUGO, auto_boost_show, auto_boost_store);
static DEVICE_ATTR(ram_update, S_IWUSR | S_IRUGO, ram_update_show, ram_update_store);
static DEVICE_ATTR(vbat_monitor, S_IWUSR | S_IRUGO, vbat_monitor_show, NULL);
static DEVICE_ATTR(activate_mode, S_IWUSR | S_IRUGO, activate_mode_show, activate_mode_store);
static DEVICE_ATTR(ram_vbat_comp, S_IWUSR | S_IRUGO, ram_vbat_comp_show, ram_vbat_comp_store);
static DEVICE_ATTR(lra_resistance, S_IWUSR | S_IRUGO, lra_resistance_show, NULL);
static DEVICE_ATTR(osc_save, S_IWUSR | S_IRUGO, osc_save_show, osc_save_store);
static DEVICE_ATTR(f0_save, S_IWUSR | S_IRUGO, f0_save_show, f0_save_store);
static DEVICE_ATTR(set_amplitude, S_IWUSR | S_IRUGO, NULL, set_amplitude_store);

static struct attribute *vibrator_attributes[] = {
	&dev_attr_state.attr,
	&dev_attr_cali_val.attr,
	&dev_attr_duration.attr,
	&dev_attr_activate.attr,
	&dev_attr_activate_mode.attr,
	&dev_attr_index.attr,
	&dev_attr_vmax.attr,
	&dev_attr_gain.attr,
	&dev_attr_seq.attr,
	&dev_attr_reg.attr,
	&dev_attr_rtp.attr,
	&dev_attr_ram_update.attr,
	&dev_attr_ram_num.attr,
	&dev_attr_f0.attr,
	&dev_attr_f0_save.attr,
	&dev_attr_vibrator_calib.attr,
	&dev_attr_cont.attr,
	&dev_attr_vbat_monitor.attr,
	&dev_attr_lra_resistance.attr,
	&dev_attr_auto_boost.attr,
	&dev_attr_prctmode.attr,
	&dev_attr_ram_vbat_comp.attr,
	&dev_attr_osc_cali.attr,
	&dev_attr_osc_save.attr,
	&dev_attr_gun_type.attr,
	&dev_attr_bullet_nr.attr,
	&dev_attr_set_amplitude.attr,
	NULL
};

static struct attribute_group vibrator_attribute_group = {
	.attrs = vibrator_attributes
};

static int vibrator_init(struct haptic *cs_haptic)
{
	int ret = 0;

	log_info("enter");
	log_info("loaded in leds_cdev framework!");
	cs_haptic->vib_dev.name = "vibrator";
	cs_haptic->vib_dev.brightness_get = brightness_get;
	cs_haptic->vib_dev.brightness_set = brightness_set;
	cs_haptic->vib_dev.default_trigger = "hw_vb_trigger";
	ret = devm_led_classdev_register(&cs_haptic->i2c->dev, &cs_haptic->vib_dev);
	if (ret < 0) {
		log_err("fail to create led dev");
		return ret;
	}
	ret = sysfs_create_group(&cs_haptic->vib_dev.dev->kobj, &vibrator_attribute_group);
	if (ret < 0) {
		log_err("error creating sysfs attr files");
		return ret;
	}
	hrtimer_init(&cs_haptic->timer, CLOCK_MONOTONIC, HRTIMER_MODE_REL);
	cs_haptic->timer.function = vibrator_timer_func;
	INIT_WORK(&cs_haptic->vibrator_work, vibrator_work_routine);
	INIT_WORK(&cs_haptic->rtp_work, rtp_work_routine);
	mutex_init(&cs_haptic->lock);
	mutex_init(&cs_haptic->rtp_lock);

	return 0;
}

static void haptic_init(struct haptic *cs_haptic)
{
	int ret = 0;

	log_info("enter");
	if (!cs_haptic) {
		log_err("cs_haptic is null");
		return;
	}
	ret = misc_register(&cs_haptic_misc);
	if (ret) {
		log_err("misc fail");
		return;
	}

	/* haptic init */
	mutex_lock(&cs_haptic->lock);
	cs_haptic->bullet_nr = 0;
	cs_haptic->gun_type = 0xff;
	cs_haptic->rtp_routine_on = 0;
	cs_haptic->activate_mode = cs_haptic->info.mode;
	cs_haptic->special_long_vib = false;

	cs_haptic->ram_vbat_comp = CS_RAM_VBAT_COMP_ENABLE;

	/* f0 calibration */
	cs_haptic->f0_cali_data = 0;
	haptic_base_reg_init(cs_haptic);
	mutex_unlock(&cs_haptic->lock);
}

static int cs_haptic_gpio_request(struct i2c_client *i2c, struct haptic *cs_haptic)
{
	int ret = -1;

	if (cs_haptic->info.boost_fw) {
		if (gpio_is_valid(cs_haptic->info.boost_en)) {
			ret = devm_gpio_request_one(&i2c->dev, cs_haptic->info.boost_en, GPIOF_OUT_INIT_HIGH, "boost_en");
			if (ret) {
				log_err("boost en gpio request failed");
				goto err_boost_en_request;
			}
			gpio_set_value_cansleep(cs_haptic->info.boost_en, 1);
			usleep_range(3500, 4000);
		}
	}

	if (gpio_is_valid(cs_haptic->reset_gpio)) {
		ret = devm_gpio_request_one(&i2c->dev, cs_haptic->reset_gpio, GPIOF_OUT_INIT_LOW, "cs_rst");
		if (ret) {
			log_err("rst request failed");
			goto err_reset_gpio_request;
		}
	}

	if (gpio_is_valid(cs_haptic->irq_gpio)) {
		ret = devm_gpio_request_one(&i2c->dev, cs_haptic->irq_gpio, GPIOF_DIR_IN, "cs_int");
		if (ret) {
			log_err("int request failed");
			goto err_irq_gpio_request;
		}
	}

	return 0;
err_irq_gpio_request:
err_reset_gpio_request:
err_boost_en_request:
	return ret;
}

static int cs_i2c_probe(struct i2c_client *i2c, const struct i2c_device_id *id)
{
	int ret = 0;
	struct haptic *cs_haptic;
	struct device_node *np = NULL;

	log_info("enter");
	if (i2c == NULL) {
		log_info("i2c client is null");
		return -ENOMEM;
	}
	np = i2c->dev.of_node;
	if (!i2c_check_functionality(i2c->adapter, I2C_FUNC_I2C)) {
		log_err("check_functionality failed");
		return -EIO;
	}
	cs_haptic = devm_kzalloc(&i2c->dev, sizeof(struct haptic), GFP_KERNEL);
	if (cs_haptic == NULL)
		return -ENOMEM;

	cs_haptic->dev = &i2c->dev;
	cs_haptic->i2c = i2c;

	i2c_set_clientdata(i2c, cs_haptic);
	dev_set_drvdata(&i2c->dev, cs_haptic);

	/* cs_haptic rst & int */
	if (np) {
		ret = parse_dt_gpio(&i2c->dev, cs_haptic, np);
		if (ret) {
			log_err("failed to parse gpio");
			goto err_parse_dt;
		}
	} else {
		cs_haptic->reset_gpio = -1;
		cs_haptic->irq_gpio = -1;
	}

	ret = cs_haptic_gpio_request(i2c, cs_haptic);
	if (ret < 0) {
		log_err("gpio request failed ret=%d", ret);
		goto err_gpio_request;
	}

	/* cs_haptic chip id */
	ret = parse_chipid(cs_haptic);
	if (ret < 0) {
		log_err("read_chipid failed ret=%d", ret);
		goto err_id;
	}
	ret = ctrl_init(cs_haptic, &i2c->dev);
	if (ret < 0) {
		log_err("ctrl_init failed ret=%d", ret);
		goto err_ctrl_init;
	}
	cs_haptic->ws = wakeup_source_register(cs_haptic->dev, "cs_haptic");
	if (!cs_haptic->ws) {
		log_err("wakeup_source_register failed ret=%d", ret);
		goto err_ws_register;
	}
#ifdef CS_CHECK_QUALIFY
	ret = cs_haptic->func->check_qualify(cs_haptic);
	if (ret < 0) {
		log_err("qualify check failed ret=%d", ret);
		goto err_check_qualify;
	}
#endif
	cs_haptic->func->parse_dt(&i2c->dev, cs_haptic, np);

#ifdef CS_SND_SOC_CODEC
	cs_haptic->func->snd_soc_init(&i2c->dev);
#endif

	/* cs_haptic irq */
	ret = irq_config(&i2c->dev, cs_haptic);
	if (ret != 0) {
		log_err("irq_config failed ret=%d", ret);
		goto err_irq_config;
	}
	atomic_set(&cs_haptic->is_process_irq_work, false);
#ifdef AAC_RICHTAP_SUPPORT
	ret = richtap_rtp_config_init(cs_haptic);
	if (ret < 0)
		goto richtap_err;
#endif
	cs_haptic_file = cs_haptic;
	vibrator_init(cs_haptic);
	haptic_init(cs_haptic);
	ram_work_init(cs_haptic);
	log_info("probe completed successfully!");

	return 0;

#ifdef AAC_RICHTAP_SUPPORT
richtap_err:
#endif
err_irq_config:
err_check_qualify:
	wakeup_source_unregister(cs_haptic->ws);
err_ws_register:
err_ctrl_init:
err_id:
err_gpio_request:
err_parse_dt:
	return ret;
}

static int cs_i2c_remove(struct i2c_client *i2c)
{
	struct haptic *cs_haptic = i2c_get_clientdata(i2c);

	log_info("enter!");
	cancel_delayed_work_sync(&cs_haptic->ram_work);
	cancel_work_sync(&cs_haptic->rtp_work);
	cancel_work_sync(&cs_haptic->vibrator_work);
	hrtimer_cancel(&cs_haptic->timer);
	mutex_destroy(&cs_haptic->lock);
	mutex_destroy(&cs_haptic->rtp_lock);
	devm_free_irq(&i2c->dev, gpio_to_irq(cs_haptic->irq_gpio), cs_haptic);
	if (gpio_is_valid(cs_haptic->irq_gpio))
		devm_gpio_free(&i2c->dev, cs_haptic->irq_gpio);
#ifdef CS_SND_SOC_CODEC
	snd_soc_unregister_codec(&i2c->dev);
#endif
	if (gpio_is_valid(cs_haptic->reset_gpio))
		devm_gpio_free(&i2c->dev, cs_haptic->reset_gpio);
	misc_deregister(&cs_haptic_misc);
	log_info("secc!");
	return 0;
}

static const struct i2c_device_id cs_i2c_id[] = {
	{CS_I2C_NAME, 0},
	{}
};
MODULE_DEVICE_TABLE(i2c, cs_i2c_id);

static int cs_i2c_suspend(struct device *dev)
{
	int ret = 0;
	struct haptic *cs_haptic = dev_get_drvdata(dev);
	unsigned int cnt = HAPTIC_SR_MAX_WAIT_CNT;

	log_info("enter");
#ifdef AAC_RICHTAP_SUPPORT
	while (cnt && atomic_read(&cs_haptic->is_process_irq_work)) {
		usleep_range(2000, 2500);
		cnt--;
	}
	log_info("wait suspend time enter");
	if (atomic_read(&cs_haptic->richtap_rtp_mode)) {
		richtap_config_stop_mode(cs_haptic);
		log_info("stop richtap rtp mode");
		return 0;
	}
#endif

	mutex_lock(&cs_haptic->lock);
	cs_haptic->func->play_stop(cs_haptic);
	mutex_unlock(&cs_haptic->lock);

	return ret;
}

static int cs_i2c_resume(struct device *dev)
{
	int ret = 0;
	return ret;
}

static SIMPLE_DEV_PM_OPS(cs_pm_ops, cs_i2c_suspend, cs_i2c_resume);

static const struct of_device_id cs_dt_match[] = {
	{.compatible = "chipsea, cs_haptic"},
	{},
};

struct i2c_driver cs_i2c_driver = {
	.driver = {
		.name = CS_I2C_NAME,
		.owner = THIS_MODULE,
		.of_match_table = of_match_ptr(cs_dt_match),
		},
	.probe = cs_i2c_probe,
	.remove = cs_i2c_remove,
	.id_table = cs_i2c_id,
};

static int __init cs_i2c_init(void)
{
	int ret = 0;

	log_info("cs_haptic driver version %s", CS_HAPTIC_DRIVER_VERSION);
	ret = i2c_add_driver(&cs_i2c_driver);
	if (ret) {
		log_err("fail to add cs_haptic device into i2c");
		return ret;
	}
	return 0;
}
module_init(cs_i2c_init);

static void __exit cs_i2c_exit(void)
{
	i2c_del_driver(&cs_i2c_driver);
}
module_exit(cs_i2c_exit);

MODULE_DESCRIPTION("CDV2625 Haptic Driver");
MODULE_LICENSE("GPL v2");
