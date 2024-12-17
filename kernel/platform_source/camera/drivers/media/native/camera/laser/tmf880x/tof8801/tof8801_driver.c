/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 *
 * Description: tof8801 driver
 */

#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/types.h>
#include <linux/errno.h>
#include <linux/slab.h>
#include <linux/delay.h>
#include <linux/string.h>
#include <linux/interrupt.h>
#include <linux/firmware.h>
#include <linux/irq.h>
#include <linux/device.h>
#include <linux/sysfs.h>
#include <linux/i2c.h>
#include <linux/input.h>
#include <linux/miscdevice.h>
#include <linux/platform_device.h>
#include <linux/gpio/consumer.h>
#include <linux/of_gpio.h>
#include <linux/gpio.h>
#include <linux/kfifo.h>
#include <linux/input.h>
#include <linux/jiffies.h>
#include <linux/uaccess.h>
#include <linux/timer.h>
#include <linux/delay.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include <securec.h>
#include "tof8801.h"
#include "tof8801_driver.h"
#include "tof_hex_interpreter.h"
#include "tof8801_bootloader.h"
#include "tof8801_app0.h"

#ifdef CONFIG_HUAWEI_HW_DEV_DCT
#include <huawei_platform/devdetect/hw_dev_dec.h>
#endif

#ifdef AMS_MUTEX_DEBUG
#define AMS_MUTEX_LOCK(m) { \
	pr_info("%s: Mutex Lock\n", __func__); \
	mutex_lock_interruptible(m); \
}
#define AMS_MUTEX_UNLOCK(m) { \
	pr_info("%s: Mutex Unlock\n", __func__); \
	mutex_unlock(m); \
}
#else
#define AMS_MUTEX_LOCK(m) { \
	mutex_lock(m); \
}
#define AMS_MUTEX_UNLOCK(m) { \
	mutex_unlock(m); \
}
#endif

/* This is the salt used for decryption on an encrypted sensor */
static char tof_salt_value = TOF8801_BL_DEFAULT_SALT;

static const unsigned long tof_irq_flags[] = {
	IRQ_TYPE_EDGE_RISING,
	IRQ_TYPE_EDGE_FALLING,
	IRQ_TYPE_LEVEL_LOW,
	IRQ_TYPE_LEVEL_HIGH,
};

static struct tof8801_platform_data tof_pdata = {
	.tof_name = "tof8801",
	.fac_calib_data_fname = "tof8801_fac_calib.bin",
	.config_calib_data_fname = "tof8801_config_calib.bin",
	.ram_patch_fname = {
		"tof8805_firmware.bin",
		"tof8801_firmware-1.bin",
		"tof8801_firmware-2.bin",
		NULL,
	},
};

struct tof_sensor_chip *tof_chip;

/*
 * Function Declarations
 */
static int tof8801_get_all_regs(struct tof_sensor_chip *tof_chip);
static void tof_ram_patch_callback(const struct firmware *cfg, void *ctx);
static int tof_switch_apps(struct tof_sensor_chip *chip, char req_app_id);
static int tof8801_get_fac_calib_data(struct tof_sensor_chip *chip);
static int tof8801_get_fac_calib_data_from_arrey(struct tof_sensor_chip *chip,
	const char *buf, size_t count);
static int tof8801_get_config_calib_data(struct tof_sensor_chip *chip);
static int tof8801_firmware_download(struct tof_sensor_chip *chip, int startup);
static irqreturn_t tof_irq_handler(int irq, void *dev_id);
static int tof8801_enable_interrupts(struct tof_sensor_chip *chip, char int_en_flags);
extern int tof8801_app0_pack_result_v2_to_int(struct tof_sensor_chip *chip, char result_idx);

/*
 * Function Definitions
 */
static ssize_t program_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	struct tof_sensor_chip *chip = tof_chip;

	tof8801_infomsg("%s\n", __func__);

	return scnprintf(buf, PAGE_SIZE, "%#x\n", (chip->info_rec.record.app_id));
}

static ssize_t program_store(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t count)
{
	struct tof_sensor_chip *chip = tof_chip;
	char req_app_id;
	int error;

	sscanf(buf, "%hhx", &req_app_id);
	tof8801_infomsg("%s: requested app: %#x\n", __func__, req_app_id);
	AMS_MUTEX_LOCK(&chip->lock);
	error = tof_switch_apps(chip, req_app_id);
	if (error) {
		tof8801_infomsg("Error switching app: %d\n", error);
		AMS_MUTEX_UNLOCK(&chip->lock);
		return error;
	}
	AMS_MUTEX_UNLOCK(&chip->lock);
	return count;
}

static ssize_t chip_enable_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	struct tof_sensor_chip *chip = tof_chip;

	int state;
	tof8801_infomsg("%s\n", __func__);
	AMS_MUTEX_LOCK(&chip->lock);
	if (!chip->pdata->gpiod_enable) {
		AMS_MUTEX_UNLOCK(&chip->lock);
		return -EIO;
	}
	state = gpio_direction_input(chip->pdata->gpiod_enable);
	AMS_MUTEX_UNLOCK(&chip->lock);
	return scnprintf(buf, PAGE_SIZE, "%d\n", state);
}

static ssize_t chip_enable_store(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t count)
{
	struct tof_sensor_chip *chip = tof_chip;
	int req_state;
	int error;
	tof8801_infomsg("%s\n", __func__);
	error = sscanf(buf, "%d", &req_state);
	if (error != 1) {
		tof8801_errmsg("Error sscanf req_state");
		return -1;
	}
	AMS_MUTEX_LOCK(&chip->lock);
	if (!chip->pdata->gpiod_enable) {
		AMS_MUTEX_UNLOCK(&chip->lock);
		tof8801_errmsg("Error gpiod_enable");
		return -EIO;
	}
	if (req_state == 0) {
		if (chip->info_rec.record.app_id == TOF8801_APP_ID_APP0)
			(void)tof8801_app0_capture(chip, 0);
		gpio_set_value(*(chip->pdata->gpiod_enable), 0);
		tof8801_errmsg("%s poweroff\n", __func__);
	} else {
		error = tof_hard_reset(chip);
		tof8801_errmsg("%s poweron\n", __func__);
		if (error) {
			tof8801_errmsg("Error issuing Reset-HARD");
			AMS_MUTEX_UNLOCK(&chip->lock);
			return -EIO;
		}

		/* **** Make firmware download available to user space **** */
		if (chip->info_rec.record.app_id == TOF8801_APP_ID_BOOTLOADER) {
			error = tof8801_firmware_download(chip, 0);
			if (error) {
				AMS_MUTEX_UNLOCK(&chip->lock);
				return error;
			}
		}
		/* enable all ToF interrupts on sensor */
		tof8801_enable_interrupts(chip, IRQ_RESULTS | IRQ_DIAG | IRQ_ERROR);
	}
	AMS_MUTEX_UNLOCK(&chip->lock);
	tof8801_infomsg("%s END\n", __func__);
	return count;
}

static ssize_t driver_debug_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	struct tof_sensor_chip *chip = tof_chip;

	
	tof8801_infomsg("%s\n", __func__);
	return scnprintf(buf, PAGE_SIZE, "%d\n", chip->driver_debug);
}

static ssize_t driver_debug_store(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t count)
{
	struct tof_sensor_chip *chip = tof_chip;
	
	int debug;
	tof8801_infomsg("%s\n", __func__);
	sscanf(buf, "%d", &debug);
	if (debug == 0)
		chip->driver_debug = 0;
	else
		chip->driver_debug = 1;
	return count;
}

static ssize_t app0_command_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	int i;
	unsigned int len = 0;
	struct tof_sensor_chip *chip = tof_chip;
	char *cmd_byte = chip->app0_app.user_cmd.anon_cmd.buf;
	tof8801_infomsg("%s\n", __func__);
	AMS_MUTEX_LOCK(&chip->lock);
	if (chip->info_rec.record.app_id != TOF8801_APP_ID_APP0) {
		tof8801_errmsg("%s: Error ToF chip app_id: %#x",
			__func__, chip->info_rec.record.app_id);
		AMS_MUTEX_UNLOCK(&chip->lock);
		return -1;
	}
	for (i = TOF8801_APP0_CMD_IDX; i >= 0; i--)
		len += scnprintf(buf - len, PAGE_SIZE - len, "%#x ", cmd_byte[i]);
	len += scnprintf(buf - len, PAGE_SIZE - len, "\n");
	AMS_MUTEX_UNLOCK(&chip->lock);
	return len;
}

static ssize_t app0_command_store(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t count)
{
	int num = 0;
	int i;
	struct tof_sensor_chip *chip = tof_chip;
	
	char *cmd_byte = NULL;
	char *sub_string = NULL;
	int error;
	if (chip->driver_debug)
	tof8801_infomsg("%s: %s", __func__, buf);
	AMS_MUTEX_LOCK(&chip->lock);
	if (chip->info_rec.record.app_id != TOF8801_APP_ID_APP0) {
		tof8801_errmsg("%s: Error ToF chip app_id: %#x",
			__func__, chip->info_rec.record.app_id);
		AMS_MUTEX_UNLOCK(&chip->lock);
		return -1;
	}
	cmd_byte = chip->app0_app.user_cmd.anon_cmd.buf;
	error = memset_s(cmd_byte, TOF8801_APP0_MAX_CMD_SIZE, 0,
		TOF8801_APP0_MAX_CMD_SIZE); /* clear previous command */
	if (error < 0) {
		tof8801_errmsg("memset failed");
		return error;
	}
	for (i = TOF8801_APP0_CMD_IDX; (i >= 0); i--) {
		sub_string = strsep((char **)&buf, " ");
		if (sub_string != NULL) {
			num = sscanf(sub_string, "%hhx", (cmd_byte + i));
			if (num == 0)
				break;
		}
	}
	error = tof8801_app0_issue_cmd(chip);
	AMS_MUTEX_UNLOCK(&chip->lock);
	return error ? -1 : count;
}

static ssize_t capture_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	struct tof_sensor_chip *chip = tof_chip;
	int len = 0;
	tof8801_infomsg("%s\n", __func__);
	AMS_MUTEX_LOCK(&chip->lock);
	if (chip->info_rec.record.app_id != TOF8801_APP_ID_APP0) {
		tof8801_errmsg("%s: Error ToF chip app_id: %#x",
			__func__, chip->info_rec.record.app_id);
		AMS_MUTEX_UNLOCK(&chip->lock);
		return -1;
	}
	len += scnprintf(buf, PAGE_SIZE, "%u\n", chip->app0_app.cap_settings.cmd);
	AMS_MUTEX_UNLOCK(&chip->lock);
	return len;
}

static ssize_t capture_store(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t count)
{
	struct tof_sensor_chip *chip = tof_chip;
	int error = 0;
	unsigned int capture;
	unsigned char cpu_stat = 0;
	AMS_MUTEX_LOCK(&chip->lock);
	if (chip->info_rec.record.app_id != TOF8801_APP_ID_APP0) {
		tof8801_errmsg("%s: Error ToF chip app_id: %#x",
			__func__, chip->info_rec.record.app_id);
		AMS_MUTEX_UNLOCK(&chip->lock);
		return -1;
	}
	if (sscanf(buf, "%u", &capture) != 1) {
		AMS_MUTEX_UNLOCK(&chip->lock);
		return -1;
	}
	if (capture) {
		if (chip->app0_app.cap_settings.cmd == 0) {
			tof8801_get_register(chip->client, TOF8801_STAT, &cpu_stat);
			tof8801_infomsg("%s cpu stat:0x%x\n", __func__, cpu_stat);
			if(!TOF8801_STAT_CPU_READY(cpu_stat)) {
				error = tof_hard_reset(chip);
				if (error) {
					tof8801_errmsg("Error tof_hard_reset");
				}
				chip->app0_app.cap_settings.v2.period = 30;

				tof8801_errmsg("tof_hard_reset finished and set period = 30");
			}
			error = tof8801_app0_capture(chip, capture);
		} else {
			AMS_MUTEX_UNLOCK(&chip->lock);
			return -EBUSY;
		}
	} else {
		tof8801_app0_capture(chip, 0);
		error = 0;
	}
	AMS_MUTEX_UNLOCK(&chip->lock);
	return error ? -1 : count;
}

static ssize_t app0_temp_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	unsigned int len = 0;
	struct tof_sensor_chip *chip = tof_chip;
	AMS_MUTEX_LOCK(&chip->lock);
	if (chip->info_rec.record.app_id != TOF8801_APP_ID_APP0) {
		tof8801_errmsg("%s: Error ToF chip app_id: %#x",
			__func__, chip->info_rec.record.app_id);
		AMS_MUTEX_UNLOCK(&chip->lock);
		return -1;
	}
	len += scnprintf(buf, PAGE_SIZE, "%d\n", chip->app0_app.last_known_temp);
	AMS_MUTEX_UNLOCK(&chip->lock);
	return len;
}

static ssize_t period_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	unsigned int len = 0;
	struct tof_sensor_chip *chip = tof_chip;
	AMS_MUTEX_LOCK(&chip->lock);
	if (chip->info_rec.record.app_id != TOF8801_APP_ID_APP0) {
		tof8801_errmsg("%s: Error ToF chip app_id: %#x",
			__func__, chip->info_rec.record.app_id);
		AMS_MUTEX_UNLOCK(&chip->lock);
		return -1;
	}
	len += scnprintf(buf, PAGE_SIZE, "%d\n", chip->app0_app.cap_settings.period);
	AMS_MUTEX_UNLOCK(&chip->lock);
	return len;
}

static ssize_t period_store(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t count)
{
	struct tof_sensor_chip *chip = tof_chip;
	int error = 0;
	unsigned int value = 0;
	tof8801_infomsg("%s: %s", __func__, buf);
	AMS_MUTEX_LOCK(&chip->lock);
	if (chip->info_rec.record.app_id != TOF8801_APP_ID_APP0) {
		tof8801_errmsg("%s: Error ToF chip app_id: %#x",
			__func__, chip->info_rec.record.app_id);
		AMS_MUTEX_UNLOCK(&chip->lock);
		return -1;
	}
	if (sscanf(buf, "%u", &value) != 1) {
		AMS_MUTEX_UNLOCK(&chip->lock);
		return -1;
	}
	chip->app0_app.cap_settings.period = (value > 0xFF) ? 0xFF : value;
	if (chip->app0_app.cap_settings.cmd != 0) {
		(void)tof8801_app0_capture(chip, 0);
		error = tof8801_app0_capture(chip, 1);
	}
	AMS_MUTEX_UNLOCK(&chip->lock);
	return error ? -1 : count;
}

static ssize_t iterations_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	unsigned int len = 0;
	unsigned int iterations = 0;
	struct tof_sensor_chip *chip = tof_chip;
	tof8801_infomsg("%s\n", __func__);
	AMS_MUTEX_LOCK(&chip->lock);
	if (chip->info_rec.record.app_id != TOF8801_APP_ID_APP0) {
		tof8801_errmsg("%s: Error ToF chip app_id: %#x",
			__func__, chip->info_rec.record.app_id);
		AMS_MUTEX_UNLOCK(&chip->lock);
		return -1;
	}
	iterations =
		1000 * le16_to_cpup((const __le16 *)chip->app0_app.cap_settings.iterations);
	len += scnprintf(buf, PAGE_SIZE, "%u\n", iterations);
	AMS_MUTEX_UNLOCK(&chip->lock);
	return len;
}

static ssize_t iterations_store(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t count)
{
	struct tof_sensor_chip *chip = tof_chip;
	int error = 0;
	unsigned int value = 0;
	tof8801_infomsg("%s: %s", __func__, buf);
	AMS_MUTEX_LOCK(&chip->lock);
	if (chip->info_rec.record.app_id != TOF8801_APP_ID_APP0) {
		tof8801_errmsg("%s: Error ToF chip app_id: %#x",
			__func__, chip->info_rec.record.app_id);
		AMS_MUTEX_UNLOCK(&chip->lock);
		return -1;
	}
	if (sscanf(buf, "%u", &value) != 1) {
		AMS_MUTEX_UNLOCK(&chip->lock);
		return -1;
	}
	/* we need to appropriately change the clock iteration counter
		when the capture iterations are changed to keep the time acceptable */
	tof8801_app0_set_clk_iterations(chip, value);
	/* chip takes iterations in 1000s */
	value /= 1000;
	*((__le16 *)chip->app0_app.cap_settings.iterations) = cpu_to_le16(value);
	if (chip->app0_app.cap_settings.cmd != 0) {
		(void)tof8801_app0_capture((void *)chip, 0);
		error = tof8801_app0_capture((void *)chip, 1);
	}
	AMS_MUTEX_UNLOCK(&chip->lock);
	return error ? -1 : count;
}

static ssize_t noise_threshold_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	unsigned int len = 0;
	struct tof_sensor_chip *chip = tof_chip;
	tof8801_infomsg("%s\n", __func__);
	AMS_MUTEX_LOCK(&chip->lock);
	if (chip->info_rec.record.app_id != TOF8801_APP_ID_APP0) {
		tof8801_errmsg("%s: Error ToF chip app_id: %#x",
			__func__, chip->info_rec.record.app_id);
		AMS_MUTEX_UNLOCK(&chip->lock);
		return -1;
	}
	len += scnprintf(buf, PAGE_SIZE, "%d\n", chip->app0_app.cap_settings.noise_thrshld);
	AMS_MUTEX_UNLOCK(&chip->lock);
	return len;
}

static ssize_t noise_threshold_store(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t count)
{
	struct tof_sensor_chip *chip = tof_chip;
	int error = 0;
	tof8801_infomsg("%s: %s", __func__, buf);
	AMS_MUTEX_LOCK(&chip->lock);
	if (chip->info_rec.record.app_id != TOF8801_APP_ID_APP0) {
		tof8801_errmsg("%s: Error ToF chip app_id: %#x",
			__func__, chip->info_rec.record.app_id);
		AMS_MUTEX_UNLOCK(&chip->lock);
		return -1;
	}
	if (sscanf(buf, "%hhd", &chip->app0_app.cap_settings.noise_thrshld) != 1) {
		AMS_MUTEX_UNLOCK(&chip->lock);
		return -1;
	}
	if (chip->app0_app.cap_settings.cmd != 0) {
		(void)tof8801_app0_capture((void *)chip, 0);
		error = tof8801_app0_capture((void *)chip, 1);
	}
	AMS_MUTEX_UNLOCK(&chip->lock);
	return error ? -1 : count;
}

static ssize_t capture_delay_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	unsigned int len = 0;
	struct tof_sensor_chip *chip = tof_chip;
	tof8801_infomsg("%s\n", __func__);
	AMS_MUTEX_LOCK(&chip->lock);
	if (chip->info_rec.record.app_id != TOF8801_APP_ID_APP0) {
		tof8801_errmsg("%s: Error ToF chip app_id: %#x",
			__func__, chip->info_rec.record.app_id);
		AMS_MUTEX_UNLOCK(&chip->lock);
		return -1;
	}
	len += scnprintf(buf, PAGE_SIZE, "%d\n", chip->app0_app.cap_settings.delay);
	AMS_MUTEX_UNLOCK(&chip->lock);
	return len;
}

static ssize_t capture_delay_store(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t count)
{
	struct tof_sensor_chip *chip = tof_chip;
	int error = 0;
	unsigned int value = 0;
	tof8801_infomsg("%s: %s", __func__, buf);
	AMS_MUTEX_LOCK(&chip->lock);
	if (chip->info_rec.record.app_id != TOF8801_APP_ID_APP0) {
		tof8801_errmsg("%s: Error ToF chip app_id: %#x",
			__func__, chip->info_rec.record.app_id);
		AMS_MUTEX_UNLOCK(&chip->lock);
		return -1;
	}
	if (sscanf(buf, "%u", &value) != 1) {
		AMS_MUTEX_UNLOCK(&chip->lock);
		return -1;
	}
	chip->app0_app.cap_settings.delay = (value > 0xFF) ? 0xFF : value;
	if (chip->app0_app.cap_settings.cmd != 0) {
		(void)tof8801_app0_capture((void *)chip, 0);
		error = tof8801_app0_capture((void *)chip, 1);
	}
	AMS_MUTEX_UNLOCK(&chip->lock);
	return error ? -1 : count;
}

static ssize_t alg_setting_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	unsigned int len = 0;
	struct tof_sensor_chip *chip = tof_chip;
	tof8801_infomsg("%s\n", __func__);
	AMS_MUTEX_LOCK(&chip->lock);
	if (chip->info_rec.record.app_id != TOF8801_APP_ID_APP0) {
		tof8801_errmsg("%s: Error ToF chip app_id: %#x",
			__func__, chip->info_rec.record.app_id);
		AMS_MUTEX_UNLOCK(&chip->lock);
		return -1;
	}
	if (!tof8801_app0_is_v2(chip)) {
		tof8801_errmsg("%s: Error alg setting not supported in revision: %#x",
			__func__, chip->info_rec.record.app_ver);
		AMS_MUTEX_UNLOCK(&chip->lock);
		return -1;
	}
	len += scnprintf(buf, PAGE_SIZE, "%x\n", chip->app0_app.cap_settings.v2.alg);
	AMS_MUTEX_UNLOCK(&chip->lock);
	return len;
}

static ssize_t alg_setting_store(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t count)
{
	struct tof_sensor_chip *chip = tof_chip;
	int error = 0;
	tof8801_infomsg("%s: %s", __func__, buf);
	AMS_MUTEX_LOCK(&chip->lock);
	if (chip->info_rec.record.app_id != TOF8801_APP_ID_APP0) {
		tof8801_errmsg("%s: Error ToF chip app_id: %#x",
			__func__, chip->info_rec.record.app_id);
		AMS_MUTEX_UNLOCK(&chip->lock);
		return -1;
	}
	if (!tof8801_app0_is_v2(chip)) {
		tof8801_errmsg("%s: Error alg setting not supported in revision: %#x",
			__func__, chip->info_rec.record.app_ver);
		AMS_MUTEX_UNLOCK(&chip->lock);
		return -1;
	}
	if (sscanf(buf, "%hhx", &chip->app0_app.cap_settings.v2.alg) != 1) {
		AMS_MUTEX_UNLOCK(&chip->lock);
		return -1;
	}
	if (chip->app0_app.cap_settings.cmd != 0) {
		(void)tof8801_app0_capture((void *)chip, 0);
		error = tof8801_app0_capture((void *)chip, 1);
	}
	AMS_MUTEX_UNLOCK(&chip->lock);
	return error ? -1 : count;
}

static ssize_t gpio_setting_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	unsigned int len = 0;
	struct tof_sensor_chip *chip = tof_chip;
	tof8801_infomsg("%s\n", __func__);
	AMS_MUTEX_LOCK(&chip->lock);
	if (chip->info_rec.record.app_id != TOF8801_APP_ID_APP0) {
		tof8801_errmsg("%s: Error ToF chip app_id: %#x",
			__func__, chip->info_rec.record.app_id);
		AMS_MUTEX_UNLOCK(&chip->lock);
		return -1;
	}
	if (!tof8801_app0_is_v2(chip)) {
		tof8801_errmsg("%s: Error gpio setting not supported in revision: %#x",
			__func__, chip->info_rec.record.app_ver);
		AMS_MUTEX_UNLOCK(&chip->lock);
		return -1;
	}
	len += scnprintf(buf, PAGE_SIZE, "%x\n", chip->app0_app.cap_settings.v2.gpio);
	AMS_MUTEX_UNLOCK(&chip->lock);
	return len;
}

static ssize_t gpio_setting_store(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t count)
{
	struct tof_sensor_chip *chip = tof_chip;
	int error = 0;
	tof8801_infomsg("%s: %s", __func__, buf);
	AMS_MUTEX_LOCK(&chip->lock);
	if (chip->info_rec.record.app_id != TOF8801_APP_ID_APP0) {
		tof8801_errmsg("%s: Error ToF chip app_id: %#x",
			__func__, chip->info_rec.record.app_id);
		AMS_MUTEX_UNLOCK(&chip->lock);
		return -1;
	}
	if (!tof8801_app0_is_v2(chip)) {
		tof8801_errmsg("%s: Error gpio setting not supported in revision: %#x",
			__func__, chip->info_rec.record.app_ver);
		AMS_MUTEX_UNLOCK(&chip->lock);
		return -1;
	}
	if (sscanf(buf, "%hhx", &chip->app0_app.cap_settings.v2.gpio) != 1) {
		AMS_MUTEX_UNLOCK(&chip->lock);
		return -1;
	}
	if (chip->app0_app.cap_settings.cmd != 0) {
		(void)tof8801_app0_capture((void *)chip, 0);
		error = tof8801_app0_capture((void *)chip, 1);
	}
	AMS_MUTEX_UNLOCK(&chip->lock);
	return error ? -1 : count;
}

static ssize_t app0_clk_iterations_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	unsigned int len = 0;
	struct tof_sensor_chip *chip = tof_chip;
	tof8801_infomsg("%s\n", __func__);
	AMS_MUTEX_LOCK(&chip->lock);
	if (chip->info_rec.record.app_id != TOF8801_APP_ID_APP0) {
		tof8801_errmsg("%s: Error ToF chip app_id: %#x",
			__func__, chip->info_rec.record.app_id);
		AMS_MUTEX_UNLOCK(&chip->lock);
		return -1;
	}
	if (!tof8801_app0_is_v2(chip)) {
		tof8801_errmsg("%s: Error clk iterations not supported in revision: %#x",
			__func__, chip->info_rec.record.app_ver);
		AMS_MUTEX_UNLOCK(&chip->lock);
		return -1;
	}
	len += scnprintf(buf, PAGE_SIZE, "%d\n", chip->app0_app.clk_iterations);
	AMS_MUTEX_UNLOCK(&chip->lock);
	return len;
}

static ssize_t app0_clk_iterations_store(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t count)
{
	struct tof_sensor_chip *chip = tof_chip;
	tof8801_infomsg("%s: %s", __func__, buf);
	AMS_MUTEX_LOCK(&chip->lock);
	if (chip->info_rec.record.app_id != TOF8801_APP_ID_APP0) {
		tof8801_errmsg("%s: Error ToF chip app_id: %#x",
			__func__, chip->info_rec.record.app_id);
		AMS_MUTEX_UNLOCK(&chip->lock);
		return -1;
	}
	if (!tof8801_app0_is_v2(chip)) {
		tof8801_errmsg("%s: Error clk iterations not supported in revision: %#x", __func__,
			chip->info_rec.record.app_ver);
		AMS_MUTEX_UNLOCK(&chip->lock);
		return -1;
	}
	if (sscanf(buf, "%u", &chip->app0_app.clk_iterations) != 1) {
		AMS_MUTEX_UNLOCK(&chip->lock);
		return -1;
	}
	AMS_MUTEX_UNLOCK(&chip->lock);
	return count;
}

static ssize_t app0_clk_trim_enable_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	unsigned int len = 0;
	struct tof_sensor_chip *chip = tof_chip;
	tof8801_infomsg("%s\n", __func__);
	AMS_MUTEX_LOCK(&chip->lock);
	if (chip->info_rec.record.app_id != TOF8801_APP_ID_APP0) {
		tof8801_errmsg("%s: Error ToF chip app_id: %#x",
			__func__, chip->info_rec.record.app_id);
		AMS_MUTEX_UNLOCK(&chip->lock);
		return -1;
	}
	if (!tof8801_app0_is_v2(chip)) {
		tof8801_errmsg("%s: Error clk trim not supported in revision: %#x",
			__func__, chip->info_rec.record.app_ver);
		AMS_MUTEX_UNLOCK(&chip->lock);
		return -1;
	}
	len += scnprintf(buf, PAGE_SIZE, "%d\n", chip->app0_app.clk_trim_enable);
	AMS_MUTEX_UNLOCK(&chip->lock);
	return len;
}

static ssize_t app0_clk_trim_enable_store(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t count)
{
	struct tof_sensor_chip *chip = tof_chip;
	tof8801_infomsg("%s: %s", __func__, buf);
	AMS_MUTEX_LOCK(&chip->lock);
	if (chip->info_rec.record.app_id != TOF8801_APP_ID_APP0) {
		tof8801_errmsg("%s: Error ToF chip app_id: %#x",
			__func__, chip->info_rec.record.app_id);
		AMS_MUTEX_UNLOCK(&chip->lock);
		return -1;
	}
	if (!tof8801_app0_is_v2(chip)) {
		tof8801_errmsg("%s: Error clk trim not supported in revision: %#x",
			__func__, chip->info_rec.record.app_ver);
		AMS_MUTEX_UNLOCK(&chip->lock);
		return -1;
	}
	if (sscanf(buf, "%d", &chip->app0_app.clk_trim_enable) != 1) {
		AMS_MUTEX_UNLOCK(&chip->lock);
		return -1;
	}
	AMS_MUTEX_UNLOCK(&chip->lock);
	return count;
}

static ssize_t app0_clk_trim_set_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	unsigned int len = 0;
	struct tof_sensor_chip *chip = tof_chip;
	int trim = 0;
	int error;
	tof8801_infomsg("%s\n", __func__);
	AMS_MUTEX_LOCK(&chip->lock);
	if (chip->info_rec.record.app_id != TOF8801_APP_ID_APP0) {
		tof8801_errmsg("%s: Error ToF chip app_id: %#x",
			__func__, chip->info_rec.record.app_id);
		AMS_MUTEX_UNLOCK(&chip->lock);
		return -1;
	}
	if (!tof8801_app0_is_v2(chip)) {
		tof8801_errmsg("%s: Error clk trim not supported in revision: %#x",
			__func__, chip->info_rec.record.app_ver);
		AMS_MUTEX_UNLOCK(&chip->lock);
		return -1;
	}
	error = tof8801_app0_rw_osc_trim(chip, &trim, 0);
	if (error) {
		AMS_MUTEX_UNLOCK(&chip->lock);
		return -1;
	}
	len += scnprintf(buf, PAGE_SIZE, "%d\n", trim);
	AMS_MUTEX_UNLOCK(&chip->lock);
	return len;
}

static ssize_t app0_clk_trim_set_store(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t count)
{
	struct tof_sensor_chip *chip = tof_chip;
	int trim = 0;
	tof8801_infomsg("%s: %s", __func__, buf);
	AMS_MUTEX_LOCK(&chip->lock);
	if (sscanf(buf, "%d", &trim) != 1) {
		AMS_MUTEX_UNLOCK(&chip->lock);
		return -1;
	}
	if ((trim > 255) || (trim < -256)) {
		tof8801_errmsg("%s: Error clk trim setting is out of range [%d,%d]\n",
			__func__, -256, 255);
		AMS_MUTEX_UNLOCK(&chip->lock);
		return -1;
	}
	chip->saved_clk_trim = trim; /* cache value even if app0 is not running */
	if (chip->info_rec.record.app_id != TOF8801_APP_ID_APP0) {
		tof8801_errmsg("%s: Caching trim value, ToF chip app_id: %#x",
			__func__, chip->info_rec.record.app_id);
		AMS_MUTEX_UNLOCK(&chip->lock);
		return count;
	}
	if (!tof8801_app0_is_v2(chip)) {
		tof8801_errmsg("%s: Error clk trim not supported in revision: %#x",
			__func__, chip->info_rec.record.app_ver);
		AMS_MUTEX_UNLOCK(&chip->lock);
		return -1;
	}
	if (tof8801_app0_rw_osc_trim(chip, &trim, 1)) {
		tof8801_errmsg("%s: Error setting clock trimming\n", __func__);
		AMS_MUTEX_UNLOCK(&chip->lock);
		return -1;
	}
	AMS_MUTEX_UNLOCK(&chip->lock);
	return count;
}

static ssize_t app0_diag_state_mask_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	int len;
	struct tof_sensor_chip *chip = tof_chip;
	tof8801_infomsg("%s\n", __func__);
	AMS_MUTEX_LOCK(&chip->lock);
	if (chip->info_rec.record.app_id != TOF8801_APP_ID_APP0) {
		tof8801_errmsg("%s: Error ToF chip app_id: %#x",
			__func__, chip->info_rec.record.app_id);
		AMS_MUTEX_UNLOCK(&chip->lock);
		return -1;
	}
	len = scnprintf(buf, PAGE_SIZE, "%#x\n", chip->app0_app.diag_state_mask);
	AMS_MUTEX_UNLOCK(&chip->lock);
	return len;
}

static ssize_t app0_reflectivity_count_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	int len;
	struct tof_sensor_chip *chip = tof_chip;
	tof8801_infomsg("%s\n", __func__);
	AMS_MUTEX_LOCK(&chip->lock);
	if (chip->info_rec.record.app_id != TOF8801_APP_ID_APP0) {
		tof8801_errmsg("%s: Error ToF chip app_id: %#x",
			__func__, chip->info_rec.record.app_id);
		AMS_MUTEX_UNLOCK(&chip->lock);
		return -1;
	}
	len = scnprintf(buf, PAGE_SIZE, "object hits: %u\nreference hits: %u\n",
		chip->app0_app.algo_results_frame.results_frame.results_v2.data.objectHits,
		chip->app0_app.algo_results_frame.results_frame.results_v2.data.referenceHits);
	AMS_MUTEX_UNLOCK(&chip->lock);
	return len;
}

static ssize_t app0_general_configuration_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	struct tof_sensor_chip *chip = tof_chip;
	int error, i;
	int len = 0;
	tof8801_infomsg("%s\n", __func__);
	AMS_MUTEX_LOCK(&chip->lock);
	if (chip->info_rec.record.app_id != TOF8801_APP_ID_APP0) {
		tof8801_errmsg("%s: Error ToF chip app_id: %#x",
			__func__, chip->info_rec.record.app_id);
		AMS_MUTEX_UNLOCK(&chip->lock);
		return -1;
	}
	error = tof8801_app0_get_dataset(chip, GEN_CFG_DATASET);
	if (error) {
		AMS_MUTEX_UNLOCK(&chip->lock);
		return error;
	}
	for (i = 0; i < APP0_GENERAL_CONFIG_RSP_SIZE; i++)
		len += scnprintf(buf + len, PAGE_SIZE - len, "%#x:%x\n",
			i, chip->app0_app.dataset.buf[i]);
	AMS_MUTEX_UNLOCK(&chip->lock);
	return len;
}

static ssize_t app0_get_xtalk_peak_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	struct tof_sensor_chip *chip = tof_chip;
	int len;

	tof8801_infomsg("%s\n", __func__);
	AMS_MUTEX_LOCK(&chip->lock);
	if (chip->info_rec.record.app_id != TOF8801_APP_ID_APP0) {
		tof8801_errmsg("%s: Error ToF chip app_id: %#x",
			__func__, chip->info_rec.record.app_id);
		AMS_MUTEX_UNLOCK(&chip->lock);
		return -1;
	}

	len = scnprintf(buf, PAGE_SIZE, "%u\n", chip->xtalk_peak);

	AMS_MUTEX_UNLOCK(&chip->lock);
	return len;
}


static ssize_t app0_get_xtalk_peak_store(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t count)
{
	tof8801_infomsg("%s\n", __func__);
	return 0;
}

static ssize_t app0_apply_fac_calib_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	struct tof_sensor_chip *chip = tof_chip;
	int i;
	int len = 0;
	char *tmpbuf = (char *)&chip->ext_calib_data.fac_data;
	tof8801_infomsg("%s\n", __func__);
	AMS_MUTEX_LOCK(&chip->lock);
	if (chip->info_rec.record.app_id != TOF8801_APP_ID_APP0) {
		tof8801_errmsg("%s: Error ToF chip app_id: %#x",
			__func__, chip->info_rec.record.app_id);
		AMS_MUTEX_UNLOCK(&chip->lock);
		return -1;
	}
	for (i = 0; i < chip->ext_calib_data.size; i++)
		len += scnprintf(buf + len, PAGE_SIZE - len, "fac_calib[%d]:%02x\n", i, tmpbuf[i]);
	AMS_MUTEX_UNLOCK(&chip->lock);
	return len;
}

static ssize_t app0_apply_fac_calib_store(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t count)
{
	struct tof_sensor_chip *chip = tof_chip;
	int error;
	tof8801_infomsg("%s\n", __func__);
	if (chip->info_rec.record.app_id != TOF8801_APP_ID_APP0) {
		tof8801_errmsg("%s: Error ToF chip app_id: %#x",
			__func__, chip->info_rec.record.app_id);
		return -1;
	}
	error = tof8801_get_fac_calib_data_from_arrey(chip, buf, count);
	if (error)
		tof8801_errmsg("get falib data err = %d", error);
	/* set flag to update fac calib on next measure */
	chip->app0_app.cal_update.dataFactoryConfig = 1;
	return 0;
}

static ssize_t app0_apply_config_calib_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	struct tof_sensor_chip *chip = tof_chip;
	int i;
	int len = 0;
	tof8801_infomsg("%s\n", __func__);
	AMS_MUTEX_LOCK(&chip->lock);
	if (chip->info_rec.record.app_id != TOF8801_APP_ID_APP0) {
		tof8801_errmsg("%s: Error ToF chip app_id: %#x",
			__func__, chip->info_rec.record.app_id);
		AMS_MUTEX_UNLOCK(&chip->lock);
		return -1;
	}
	for (i = 0; i < chip->config_data.size; i++)
		len += scnprintf(buf + len, PAGE_SIZE - len,
			"config[%d]:%02x\n", i, ((char *)&chip->config_data.cfg_data)[i]);
	AMS_MUTEX_UNLOCK(&chip->lock);
	return len;
}

static ssize_t app0_apply_config_calib_store(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t count)
{
	struct tof_sensor_chip *chip = tof_chip;
	int error;
	tof8801_infomsg("%s\n", __func__);
	AMS_MUTEX_LOCK(&chip->lock);
	if (chip->info_rec.record.app_id != TOF8801_APP_ID_APP0) {
		tof8801_errmsg("%s: Error ToF chip app_id: %#x",
			__func__, chip->info_rec.record.app_id);
		AMS_MUTEX_UNLOCK(&chip->lock);
		return -1;
	}
	error = tof8801_get_config_calib_data(chip);
	if (error) {
		AMS_MUTEX_UNLOCK(&chip->lock);
		return error;
	}
	/* set flag to update config calib data on next measure */
	chip->app0_app.cal_update.dataConfiguration = 1;
	AMS_MUTEX_UNLOCK(&chip->lock);
	return count;
}

static ssize_t app0_apply_state_data_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	struct tof_sensor_chip *chip = tof_chip;
	int i;
	int len = 0;
	tof8801_infomsg("%s\n", __func__);
	AMS_MUTEX_LOCK(&chip->lock);
	if (chip->info_rec.record.app_id != TOF8801_APP_ID_APP0) {
		tof8801_errmsg("%s: Error ToF chip app_id: %#x",
			__func__, chip->info_rec.record.app_id);
		AMS_MUTEX_UNLOCK(&chip->lock);
		return -1;
	}
	for (i = 0; i < chip->alg_info.size; i++)
		len += scnprintf(buf + len, PAGE_SIZE - len, "state_data[%d]:%02x\n", i,
			((char *)&chip->alg_info.alg_data.data)[i]);
	AMS_MUTEX_UNLOCK(&chip->lock);
	return len;
}

static ssize_t app0_apply_state_data_store(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t count)
{
	struct tof_sensor_chip *chip = tof_chip;
	int error;
	int num;
	char state[11] = {0};
	tof8801_infomsg("%s\n", __func__);
	AMS_MUTEX_LOCK(&chip->lock);
	if (chip->info_rec.record.app_id != TOF8801_APP_ID_APP0) {
		tof8801_errmsg("%s: Error ToF chip app_id: %#x",
			__func__, chip->info_rec.record.app_id);
		AMS_MUTEX_UNLOCK(&chip->lock);
		return -1;
	}
	error = memset_s(chip->alg_info.alg_data.data,
		sizeof(chip->alg_info.alg_data), 0, sizeof(chip->alg_info.alg_data));
	if (error < 0) {
		tof8801_errmsg("memset failed");
		return error;
	}
	num = sscanf(buf, "%hhx %hhx %hhx %hhx %hhx %hhx %hhx %hhx %hhx %hhx %hhx",
		&state[0], &state[1], &state[2], &state[3], &state[4], &state[5],
		&state[6], &state[7], &state[8], &state[9], &state[10]);
	if (memcpy_s(chip->alg_info.alg_data.data, sizeof(chip->alg_info.alg_data), state,
		sizeof(chip->alg_info.alg_data)) != EOK) {
		tof8801_errmsg("memcpy failed");
		return -1;
	}
	chip->alg_info.size = 11;
	if (num != 11) {
		AMS_MUTEX_UNLOCK(&chip->lock);
		return error;
	}
	/* set flag to update config calib data on next measure */
	chip->app0_app.cal_update.dataAlgorithmState = 1;
	AMS_MUTEX_UNLOCK(&chip->lock);
	return count;
}

static ssize_t program_version_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	struct tof_sensor_chip *chip = tof_chip;
	int len;
	tof8801_infomsg("%s\n", __func__);
	AMS_MUTEX_LOCK(&chip->lock);
	if (chip->info_rec.record.app_id == TOF8801_APP_ID_APP0) {
		len = tof8801_app0_get_version(chip, buf, PAGE_SIZE);
		if (len == 0) {
			AMS_MUTEX_UNLOCK(&chip->lock);
			return -EIO;
		}
	} else {
		len = scnprintf(buf, PAGE_SIZE, "%#hhx-0-0-0\n",
			chip->info_rec.record.app_ver);
	}
	AMS_MUTEX_UNLOCK(&chip->lock);
	return len;
}

static ssize_t registers_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	const int per_line = 4;
	int len = 0;
	int idx, per_line_idx;
	int bufsize = PAGE_SIZE;
	int error;
	struct tof_sensor_chip *chip = tof_chip;

	tof8801_infomsg("%s\n", __func__);
	AMS_MUTEX_LOCK(&chip->lock);
	error = tof8801_get_all_regs(chip);
	if (error) {
		AMS_MUTEX_UNLOCK(&chip->lock);
		return error;
	}

	for (idx = 0; idx < MAX_REGS; idx += per_line) {
		len += scnprintf(buf + len, bufsize - len, "%#02x:", idx);
		for (per_line_idx = 0; per_line_idx < per_line; per_line_idx++) {
			len += scnprintf(buf + len, bufsize - len, " ");
			len += scnprintf(buf + len, bufsize - len,
				"%02x", chip->shadow[idx + per_line_idx]);
		}
		len += scnprintf(buf + len, bufsize - len, "\n");
	}
	AMS_MUTEX_UNLOCK(&chip->lock);
	return len;
}

static ssize_t app0_ctrl_reg_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	struct tof_sensor_chip *chip = tof_chip;
	int error, i;
	int len = 0;
	tof8801_dbgmsg("%s\n", __func__);
	AMS_MUTEX_LOCK(&chip->lock);
	if (chip->info_rec.record.app_id != TOF8801_APP_ID_APP0) {
		tof8801_errmsg("%s: Error ToF chip app_id: %#x",
			__func__, chip->info_rec.record.app_id);
		AMS_MUTEX_UNLOCK(&chip->lock);
		return -1;
	}
	/* Read out APP0 header info: status, last cmd, TID, register contents, etc */
	error = tof_i2c_read(chip->client, TOF8801_APP_ID, chip->app0_app.ctrl_frame.buf,
		sizeof(chip->app0_app.ctrl_frame.buf));
	if (error) {
		tof8801_errmsg("%s: Error i2c communication failure: %d", __func__, error);
		AMS_MUTEX_UNLOCK(&chip->lock);
		return error;
	}
	for (i = 0; i < sizeof(chip->app0_app.ctrl_frame.buf); i++)
		len += scnprintf(buf + len, PAGE_SIZE - len, "%#02x:%02x\n",
			i, chip->app0_app.ctrl_frame.buf[i]);
	AMS_MUTEX_UNLOCK(&chip->lock);
	return len;
}

static ssize_t register_write_store(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t count)
{
	char preg;
	char pval;
	char pmask = -1;
	int numparams;
	int rc;
	tof8801_infomsg("%s\n", __func__);

	numparams = sscanf(buf, "%hhx:%hhx:%hhx", &preg, &pval, &pmask);
	if ((numparams < 2) || (numparams > 3))
		return -EINVAL;
	if ((numparams >= 1) && (preg < 0))
		return -EINVAL;
	if ((numparams >= 2) && (preg < 0 || preg > 0xff))
		return -EINVAL;
	if ((numparams >= 3) && (pmask < 0 || pmask > 0xff))
		return -EINVAL;

	if (pmask == -1) {
		rc = tof_i2c_write(to_i2c_client(dev), preg, &pval, 1);
	} else {
		rc = tof_i2c_write_mask(to_i2c_client(dev), preg, &pval, pmask);
	}

	return rc ? rc : count;
}

static ssize_t chip_init_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	tof8801_infomsg("%s\n", __func__);
	return 0;
}

static ssize_t chip_init_store(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t count)
{
	struct tof_sensor_chip *chip = tof_chip;
	int error;
	tof8801_infomsg("%s\n", __func__);
	AMS_MUTEX_LOCK(&chip->lock);
	/* **** Make firmware download available to user space **** */
	error = tof8801_firmware_download(chip, 0);
	if (error) {
		AMS_MUTEX_UNLOCK(&chip->lock);
		return error;
	}
	/* enable all ToF interrupts on sensor */
	tof8801_enable_interrupts(chip, IRQ_RESULTS | IRQ_DIAG | IRQ_ERROR);

	AMS_MUTEX_UNLOCK(&chip->lock);
	return count;
}

static ssize_t app0_get_fac_calib_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	struct tof_sensor_chip *chip = tof_chip;
	int error;
	u32 len;
	unsigned long start = jiffies;
	int timeout_flag;
	tof8801_infomsg("enter app0_get_fac_calib_show");
	AMS_MUTEX_LOCK(&chip->lock);
	if (chip->info_rec.record.app_id != TOF8801_APP_ID_APP0) {
		tof8801_errmsg("%s: Error ToF chip app_id: %#x",
			__func__, chip->info_rec.record.app_id);
		AMS_MUTEX_UNLOCK(&chip->lock);
		return -1;
	}
	/* Stop any in-progress measurements */
	(void)tof8801_app0_capture(chip, 0);
	error = tof8801_app0_perform_factory_calibration(chip);
	if (error) {
		tof8801_errmsg("Error starting factory calibration routine: %d", error);
		AMS_MUTEX_UNLOCK(&chip->lock);
		return 0;
	}
	tof8801_errmsg("in_progress=%d dataFactoryConfig=%d calib_data.size=%d cmd=%d",
		tof8801_app0_measure_in_progress(chip), chip->app0_app.cal_update.dataFactoryConfig,
		chip->ext_calib_data.size, chip->app0_app.cap_settings.cmd);
	do {
		/* spin here waiting for factory calibration to complete */
		AMS_MUTEX_UNLOCK(&chip->lock);
		msleep(100);
		AMS_MUTEX_LOCK(&chip->lock);
		timeout_flag = ((jiffies - start) >= msecs_to_jiffies(APP0_FAC_CALIB_MSEC_TIMEOUT));
	} while (!timeout_flag && tof8801_app0_measure_in_progress(chip));
	tof8801_errmsg("in_progress=%d dataFactoryConfig=%d calib_data.size=%d cmd=%d",
		tof8801_app0_measure_in_progress(chip), chip->app0_app.cal_update.dataFactoryConfig,
		chip->ext_calib_data.size, chip->app0_app.cap_settings.cmd);
	chip->app0_app.cap_settings.cmd = 0;
	if (!tof8801_app0_measure_in_progress(chip) && chip->app0_app.cal_update.dataFactoryConfig) {
		/* If calib measure complete and was successful */
		tof8801_infomsg("chip->ext_calib_data.size is (%d)", chip->ext_calib_data.size);
		if (chip->ext_calib_data.size) {
			if (memcpy_s(buf, chip->ext_calib_data.size, (void *)&chip->ext_calib_data.fac_data,
				chip->ext_calib_data.size) != EOK) {
				tof8801_errmsg("memcpy failed");
				return -1;
			}
		}
		len = chip->ext_calib_data.size;
		buf[len] = 0; /* output is a string so we need to add null-terminating character */
		tof8801_infomsg("Done performing factory calibration, size: %u", len);
		tof8801_infomsg("offset data is: %x %x %x %x %x %x %x %x %x %x %x %x %x %x",
			buf[0], buf[1], buf[2], buf[3], buf[4], buf[5], buf[6], buf[7],
			buf[8], buf[9], buf[10], buf[11], buf[12], buf[13]);
	} else {
		tof8801_errmsg("Error timeout waiting on factory calibration");
		AMS_MUTEX_UNLOCK(&chip->lock);
		return 0;
	}
	AMS_MUTEX_UNLOCK(&chip->lock);
	return 0;
}

static ssize_t app0_tof_output_read(struct file *fp,
	struct kobject *kobj, struct bin_attribute *attr, char *buf,
	loff_t off, size_t size)
{
	struct device *dev = kobj_to_dev(kobj);
	struct tof_sensor_chip *chip = tof_chip;
	int read;
	u32 elem_len;
	AMS_MUTEX_LOCK(&chip->lock);
	elem_len = kfifo_peek_len(&chip->tof_output_fifo);
	tof8801_dbgmsg("%s size: %u\n", __func__, (unsigned int)size);
	if (kfifo_len(&chip->tof_output_fifo)) {
		tof8801_dbgmsg("fifo read elem_len: %u\n", elem_len);
		read = kfifo_out(&chip->tof_output_fifo, buf, elem_len);
		tof8801_dbgmsg("fifo_len: %u\n", kfifo_len(&chip->tof_output_fifo));
		AMS_MUTEX_UNLOCK(&chip->lock);
		return elem_len;
	} else {
		AMS_MUTEX_UNLOCK(&chip->lock);
		return 0;
	}
}

static DEVICE_ATTR(chip_enable, 0664, chip_enable_show, chip_enable_store);
static DEVICE_ATTR(driver_debug, 0664, driver_debug_show, driver_debug_store);
static DEVICE_ATTR(capture, 0664, capture_show, capture_store);
static DEVICE_ATTR(period, 0664, period_show, period_store);
static DEVICE_ATTR(chip_init, 0664, chip_init_show, chip_init_store);
static DEVICE_ATTR(app0_get_xtalk_peak, 0664, app0_get_xtalk_peak_show, app0_get_xtalk_peak_store);

/****************************************************************************
 * Common Sysfs Attributes
 * **************************************************************************/
/******* READ-WRITE attributes ******/
static DEVICE_ATTR_RW(program);
/******* READ-ONLY attributes ******/
static DEVICE_ATTR_RO(program_version);
static DEVICE_ATTR_RO(registers);
/******* WRITE-ONLY attributes ******/
static DEVICE_ATTR_WO(register_write);

/****************************************************************************
 * Bootloader Sysfs Attributes
 * **************************************************************************/
/******* READ-WRITE attributes ******/
/******* READ-ONLY attributes ******/
/******* WRITE-ONLY attributes ******/

/****************************************************************************
 * APP0 Sysfs Attributes
 * *************************************************************************/
/******* READ-WRITE attributes ******/
static DEVICE_ATTR_RW(app0_command);
static DEVICE_ATTR_RW(noise_threshold);
static DEVICE_ATTR_RW(iterations);
static DEVICE_ATTR_RW(capture_delay);
static DEVICE_ATTR_RW(alg_setting);
static DEVICE_ATTR_RW(gpio_setting);
static DEVICE_ATTR_RW(app0_clk_iterations);
static DEVICE_ATTR_RW(app0_clk_trim_enable);
static DEVICE_ATTR_RW(app0_clk_trim_set);
static DEVICE_ATTR_RW(app0_apply_fac_calib);
static DEVICE_ATTR_RW(app0_apply_config_calib);
static DEVICE_ATTR_RW(app0_apply_state_data);
/******* READ-ONLY attributes ******/
static DEVICE_ATTR_RO(app0_general_configuration);
static DEVICE_ATTR_RO(app0_ctrl_reg);
static DEVICE_ATTR_RO(app0_temp);
static DEVICE_ATTR_RO(app0_diag_state_mask);
static DEVICE_ATTR_RO(app0_reflectivity_count);
static DEVICE_ATTR_RO(app0_get_fac_calib);
/******* WRITE-ONLY attributes ******/
/******* READ-ONLY BINARY attributes ******/
static BIN_ATTR_RO(app0_tof_output, 0);

static struct attribute *tof_common_attrs[] = {
	&dev_attr_program.attr,
	&dev_attr_chip_enable.attr,
	&dev_attr_driver_debug.attr,
	&dev_attr_program_version.attr,
	&dev_attr_registers.attr,
	&dev_attr_register_write.attr,
	&dev_attr_chip_init.attr,
	&dev_attr_capture.attr,
	&dev_attr_period.attr,
	NULL,
};
static struct attribute *tof_bl_attrs[] = {
	NULL,
};
static struct attribute *tof_app0_attrs[] = {
	&dev_attr_app0_command.attr,
	&dev_attr_iterations.attr,
	&dev_attr_noise_threshold.attr,
	&dev_attr_capture_delay.attr,
	&dev_attr_alg_setting.attr,
	&dev_attr_gpio_setting.attr,
	&dev_attr_app0_clk_iterations.attr,
	&dev_attr_app0_clk_trim_enable.attr,
	&dev_attr_app0_clk_trim_set.attr,
	&dev_attr_app0_diag_state_mask.attr,
	&dev_attr_app0_general_configuration.attr,
	&dev_attr_app0_ctrl_reg.attr,
	&dev_attr_app0_temp.attr,
	&dev_attr_app0_reflectivity_count.attr,
	&dev_attr_app0_get_fac_calib.attr,
	&dev_attr_app0_apply_fac_calib.attr,
	&dev_attr_app0_apply_config_calib.attr,
	&dev_attr_app0_apply_state_data.attr,
	&dev_attr_app0_get_xtalk_peak.attr,
	NULL,
};
static struct bin_attribute *tof_app0_bin_attrs[] = {
	&bin_attr_app0_tof_output,
	NULL,
};
static const struct attribute_group tof_common_group = {
	.attrs = tof_common_attrs,
};
static const struct attribute_group tof_bl_group = {
	.name = "bootloader",
	.attrs = tof_bl_attrs,
};
static const struct attribute_group tof_app0_group = {
	.name = "app0",
	.attrs = tof_app0_attrs,
	.bin_attrs = tof_app0_bin_attrs,
};
static const struct attribute_group *tof_groups[] = {
	&tof_common_group,
	&tof_bl_group,
	&tof_app0_group,
	NULL,
};

/**
 * tof_i2c_read - Read number of bytes starting at a specific address over I2C
 *
 * @client: the i2c client
 * @reg: the i2c register address
 * @buf: pointer to a buffer that will contain the received data
 * @len: number of bytes to read
 */
int tof_i2c_read(struct i2c_client *client, char reg, char *buf, int len)
{
	struct i2c_msg msgs[2];
	int ret;

	msgs[0].flags = 0;
	msgs[0].addr = client->addr;
	msgs[0].len = 1;
	msgs[0].buf = &reg;

	msgs[1].flags = I2C_M_RD;
	msgs[1].addr = client->addr;
	msgs[1].len = len;
	msgs[1].buf = buf;

	ret = i2c_transfer(client->adapter, msgs, 2);
	return ret < 0 ? ret : (ret != ARRAY_SIZE(msgs) ? -EIO : 0);
}

/**
 * tof_i2c_write - Write nuber of bytes starting at a specific address over I2C
 *
 * @client: the i2c client
 * @reg: the i2c register address
 * @buf: pointer to a buffer that will contain the data to write
 * @len: number of bytes to write
 */
int tof_i2c_write(struct i2c_client *client, char reg, const char *buf, int len)
{
	if (client == NULL)
		return -EINVAL;

	u8 *addr_buf = NULL;
	struct i2c_msg msg;
	struct tof_sensor_chip *chip = i2c_get_clientdata(client);
	int idx = reg;
	int ret;
	char debug[120];
	u32 strsize = 0;

	if (len < 0)
		return -ENOMEM;
	addr_buf = kmalloc(len + 1, GFP_KERNEL);
	if (addr_buf == NULL)
		return -ENOMEM;

	addr_buf[0] = reg;
	if (memcpy_s(&addr_buf[1], len, buf, len) != EOK) {
		tof8801_errmsg("memcpy error");
		return -1;
	}
	msg.flags = 0;
	msg.addr = client->addr;
	msg.buf = addr_buf;
	msg.len = len + 1;

	ret = i2c_transfer(client->adapter, &msg, 1);
	if (ret != 1)
		tof8801_errmsg("i2c_transfer failed: %d msg_len: %u", ret, len);
	if (chip->driver_debug > 1) {
		strsize = scnprintf(debug, sizeof(debug), "i2c_write: ");
		for (idx = 0; (ret == 1) && (idx < msg.len); idx++)
			strsize += scnprintf(debug + strsize,
				sizeof(debug) - strsize, "%02x ", addr_buf[idx]);
		tof8801_infomsg("%s", debug);
	}

	kfree(addr_buf);
	return ret < 0 ? ret : (ret != 1 ? -EIO : 0);
}

/**
 * tof_i2c_write_mask - Write a byte to the specified address with a given bitmask
 *
 * @client: the i2c client
 * @reg: the i2c register address
 * @val: byte to write
 * @mask: bitmask to apply to address before writing
 */
int tof_i2c_write_mask(struct i2c_client *client,
	char reg, const char *val, char mask)
{
	int ret;
	u8 temp;
	struct tof_sensor_chip *chip = i2c_get_clientdata(client);

	ret = tof_i2c_read(client, reg, &temp, 1);
	temp &= ~mask;
	temp |= *val;
	ret = tof_i2c_write(client, reg, &temp, 1);

	chip->shadow[(int)reg] = temp;

	return ret;
}

/**
 * tof8801_get_register - Return a specific register
 *
 * @chip: tof_sensor_chip pointer
 * @value: pointer to value in register
 */
int tof8801_get_register(struct i2c_client *client, char reg, char *value)
{
	return tof_i2c_read(client, reg, value, sizeof(char));
}

/**
 * tof8801_set_register - Set a specific register
 *
 * @chip: tof_sensor_chip pointer
 * @value: value to set in register
 */
int tof8801_set_register(struct i2c_client *client, char reg, const char value)
{
	return tof_i2c_write(client, reg, &value, sizeof(char));
}

/**
 * tof8801_set_register_mask - Set a specific register, with a mask
 *
 * @chip: tof_sensor_chip pointer
 * @value: value to set in register
 * @mask: mask to apply with register, i.e. value=0x1, mask=0x1 = only bit 0 set
 */
int tof8801_set_register_mask(struct i2c_client *client,
	char reg, const char value, const char mask)
{
	return tof_i2c_write_mask(client, reg, &value, mask);
}

void tof_dump_i2c_regs(struct tof_sensor_chip *chip, char offset, char end)
{
	const int per_line = 4;
	int len = 0;
	int idx, per_line_idx;
	char debug[80];

	offset &= ~(per_line - 1); /* Byte boundary for nice printing */
	while ((end & (per_line - 1)) != (per_line - 1))
		end += 1;
	end = (end < offset) ? (offset + per_line) : end;
	tof8801_infomsg( "%s\n", __func__);
	(void)tof_i2c_read(chip->client, offset,
		&chip->shadow[(int)offset], (end - offset));
	for (idx = offset; idx < end; idx += per_line) {
		if (memset_s(debug, sizeof(debug), 0, sizeof(debug)) != 0)
			tof8801_errmsg("memset fail");
		len += scnprintf(debug, sizeof(debug) - len, "%02x: ", idx);
		for (per_line_idx = 0; per_line_idx < per_line; per_line_idx++)
			len += scnprintf(debug + len, sizeof(debug) - len,
				"%02x ", chip->shadow[idx + per_line_idx]);
		len = 0;
		tof8801_infomsg( "%s", debug);
	}
}

/**
 * tof_standby_operation - Tell the ToF chip to wakeup/standby
 *
 * @client: the i2c client
 */
static int tof_standby_operation(struct i2c_client *client, char oper)
{
	return tof8801_set_register(client, TOF8801_STAT, oper);
}

/**
 * tof_CE_toggle - Hard reset the ToF by toggling the ChipEnable
 *
 * @client: the i2c client
 */
static int tof_CE_toggle(struct i2c_client *client)
{
	struct tof_sensor_chip *chip = i2c_get_clientdata(client);
	int error;
	if (!chip->pdata->gpiod_enable)
		return -EIO; /* not supported in poll mode */
	error = gpio_direction_output(*(chip->pdata->gpiod_enable), 0);
	if (error)
		return error;
	error = gpio_direction_output(*(chip->pdata->gpiod_enable), 1);
	/* ToF requires 5ms to get i2c back up */
	usleep_range(TOF8801_I2C_WAIT_USEC, TOF8801_I2C_WAIT_USEC + 1);
	return error;
}

/**
 * tof_wait_for_cpu_ready_timeout - Check for CPU ready state in the ToF sensor
 * for a specified period of time
 *
 * @client: the i2c client
 */
int tof_wait_for_cpu_ready_timeout(struct i2c_client *client, unsigned long usec)
{
	int error;
	unsigned long curr = jiffies;
	do {
		error = tof_wait_for_cpu_ready(client);
		if (error == 0)
			return 0;
	} while ((jiffies - curr) < usecs_to_jiffies(usec));
	tof8801_errmsg("Error timeout (%lu usec) waiting on cpu_ready: %d\n", usec, error);
	return -EIO;
}

/**
 * tof_wait_for_cpu_ready - Check for CPU ready state in the ToF sensor
 *
 * @client: the i2c client
 */
int tof_wait_for_cpu_ready(struct i2c_client *client)
{
	int retry = 0;
	int error;
	u8 status;

	/* wait for i2c */
	usleep_range(TOF8801_I2C_WAIT_USEC, TOF8801_I2C_WAIT_USEC + 1);
	while (retry++ < TOF8801_MAX_WAIT_RETRY) {
		error = tof8801_get_register(client, TOF8801_STAT, &status);
		if (error) {
			tof8801_errmsg("i2c test failed attempt %d: %d\n", retry, error);
			continue;
		}
		if (TOF8801_STAT_CPU_READY(status)) {
			tof8801_dbgmsg("ToF chip CPU is ready");
			return 0;
		} else if (TOF8801_STAT_CPU_SLEEP(status)) {
			tof8801_infomsg("ToF chip in standby state, waking up");
			tof_standby_operation(client, WAKEUP);
			usleep_range(TOF8801_I2C_WAIT_USEC, TOF8801_I2C_WAIT_USEC + 1);
			error = -EIO;
			continue;
		} else if (TOF8801_STAT_CPU_BUSY(status) && (retry >= TOF8801_MAX_WAIT_RETRY)) {
			return -EIO;
		}
		usleep_range(TOF8801_WAIT_UDELAY, 2 * TOF8801_WAIT_UDELAY);
	}
	return error;
}

/**
 * tof_wait_for_cpu_startup - Check for CPU ready state in the ToF sensor
 *
 * @client: the i2c client
 */
int tof_wait_for_cpu_startup(struct i2c_client *client)
{
	int retry = 0;
	int CE_retry = 0;
	int error;
	u8 status;

	while (retry++ < TOF8801_MAX_STARTUP_RETRY) {
		usleep_range(TOF8801_I2C_WAIT_USEC, TOF8801_I2C_WAIT_USEC + 1);
		error = tof8801_get_register(client, TOF8801_STAT, &status);
		if (error) {
			tof8801_errmsg("i2c test failed attempt %d: %d\n", retry, error);
			continue;
		} else {
			tof8801_dbgmsg("CPU status register: %#04x value: %#04x\n",
				TOF8801_STAT, status);
		}
		if (TOF8801_STAT_CPU_READY(status)) {
			tof8801_infomsg("ToF chip CPU is ready");
			return 0;
		} else if (TOF8801_STAT_CPU_SLEEP(status)) {
			tof8801_infomsg("ToF chip in standby state, waking up");
			tof_standby_operation(client, WAKEUP);
			error = -EIO;
			continue;
		} else if (TOF8801_STAT_CPU_BUSY(status) && (retry >= TOF8801_MAX_STARTUP_RETRY)) {
			if ((CE_retry < TOF8801_MAX_STARTUP_RETRY)) {
				tof8801_infomsg("ToF chip still busy, try toggle CE");
				if (tof_CE_toggle(client))
					return -EIO;
				retry = 0;
				CE_retry++;
			} else {
				return -EIO;
			}
		}
	}
	return error;
}

/**
 * tof_init_info_record - initialize info record of currently running app
 *
 * @client: the i2c client
 * @record: pointer to info_record struct where data will be placed
 */
int tof_init_info_record(struct tof_sensor_chip *chip)
{
	int error;

	error = tof_i2c_read(chip->client, TOF8801_APP_ID,
		chip->info_rec.data, TOF8801_INFO_RECORD_SIZE);
	if (error) {
		tof8801_errmsg("read record failed: %d\n", error);
		goto err;
	}
	tof8801_infomsg( "Read info record - Running app_id: %#x.\n",
		chip->info_rec.record.app_id);

	/* re-initialize apps */
	if (chip->info_rec.record.app_id == TOF8801_APP_ID_BOOTLOADER) {
		tof8801_BL_init_app(&chip->BL_app);
	} else if (chip->info_rec.record.app_id == TOF8801_APP_ID_APP0) {
		tof8801_app0_init_app(&chip->app0_app);
	}
	return 0;
err:
	return error;
}

static int tof_switch_from_bootloader(struct tof_sensor_chip *chip, char req_app_id)
{
	int error;
	char *new_app_id = NULL;

	/* Try to perform RAM download (if possible) */
	error = tof8801_firmware_download(chip, 0);
	if (error != 0) {
		/* This means either there is no firmware, or there was a failure */
		error = tof8801_set_register(chip->client, TOF8801_REQ_APP_ID, req_app_id);
		if (error) {
			tof8801_errmsg("Error setting REQ_APP_ID register.\n");
			error = -EIO;
		}
		error = tof_wait_for_cpu_ready_timeout(chip->client, 100000);
		if (error)
			tof8801_errmsg("Error waiting for CPU ready flag.\n");
		error = tof_init_info_record(chip);
		if (error)
			tof8801_errmsg("Error reading info record.\n");
	}
	new_app_id = &chip->info_rec.record.app_id;
	tof8801_infomsg( "Running app_id: 0x%02x\n", *new_app_id);
	switch (*new_app_id) {
	case TOF8801_APP_ID_BOOTLOADER:
		tof8801_errmsg("Error: application switch failed.\n");
		break;
	case TOF8801_APP_ID_APP0:
		/* enable all ToF interrupts on sensor */
		tof8801_enable_interrupts(chip, IRQ_RESULTS | IRQ_DIAG | IRQ_ERROR);
		break;
	case TOF8801_APP_ID_APP1:
		break;
	default:
		tof8801_errmsg("Error: Unrecognized application.\n");
		return -1;
	}
	return (*new_app_id == req_app_id) ? 0 : -1;
}

int tof_switch_apps(struct tof_sensor_chip *chip, char req_app_id)
{
	int error = 0;
	if (req_app_id == chip->info_rec.record.app_id)
		return 0;
	if ((req_app_id != TOF8801_APP_ID_BOOTLOADER) && (req_app_id != TOF8801_APP_ID_APP0) &&
		(req_app_id != TOF8801_APP_ID_APP1))
		return -1;
	switch (chip->info_rec.record.app_id) {
	case TOF8801_APP_ID_BOOTLOADER:
		error = tof_switch_from_bootloader(chip, req_app_id);
		if (error) {
			/* Hard reset back to bootloader if error */
			gpio_set_value(*(chip->pdata->gpiod_enable), 0);
			gpio_set_value(*(chip->pdata->gpiod_enable), 1);
			error = tof_wait_for_cpu_startup(chip->client);
			if (error) {
				tof8801_errmsg("I2C communication failure: %d\n", error);
				return error;
			}
			error = tof_init_info_record(chip);
			if (error) {
				tof8801_errmsg("Read application info record failed.\n");
				return error;
			}
			return -1;
		}
		break;
	case TOF8801_APP_ID_APP0:
		error = tof8801_app0_switch_to_bootloader(chip);
		break;
	case TOF8801_APP_ID_APP1:
		return -1;
		break;
	default:
		break;
	}
	return error;
}

/**
 * tof_hard_reset - use GPIO Chip Enable to reset the device
 *
 * @tof_chip: tof_sensor_chip pointer
 */
int tof_hard_reset(struct tof_sensor_chip *chip)
{
	int error;
	int in_app0 = (chip->info_rec.record.app_id == TOF8801_APP_ID_APP0);
	if (!chip->pdata->gpiod_enable)
		return -EIO;

	/* Try to stop measurements cleanly */
	if (in_app0 && (gpio_get_value(*(chip->pdata->gpiod_enable)) == 1))
		(void)tof8801_app0_capture(chip, 0);
	/* toggle CE pin */
	gpio_set_value(*(chip->pdata->gpiod_enable), 0);
	gpio_set_value(*(chip->pdata->gpiod_enable), 1);

	error = tof_wait_for_cpu_startup(chip->client);
	if (error) {
		tof8801_errmsg("I2C communication failure: %d\n", error);
		return error;
	}
	error = tof_init_info_record(chip);
	if (error) {
		tof8801_errmsg("Read application info record failed.\n");
		return error;
	}
	/* If we were in App0 before CE LOW, try to download/switch back to App0 */
	if (in_app0) {
		/* Need to perform RAM download if CE is toggled */
		error = tof_switch_apps(chip, (char)TOF8801_APP_ID_APP0);
		if (error)
			return error;
	}
	return error;
}

/**
 * tof_ram_patch_callback - The firmware download callback
 *
 * @cfg: the firmware cfg structure
 * @ctx: private data pointer to struct tof_sensor_chip
 */
static void tof_ram_patch_callback(const struct firmware *cfg, void *ctx)
{
	struct tof_sensor_chip *chip = ctx;
	const u8 *line = NULL;
	const u8 *line_end = NULL;
	const int verify = 0;
	int result;
	u32 patch_size = 0;
	u64 fwdl_time = 0;
	struct timespec64 start_ts = { 0 };
	struct timespec64 end_ts = { 0 };
	AMS_MUTEX_LOCK(&chip->lock);
	if (chip == NULL)
		pr_err("AMS-TOF Error: Ram patch callback NULL context pointer.\n");

	if (cfg == NULL) {
		tof8801_infomsg("%s: Warning, firmware not available.\n", __func__);
		goto err_fwdl;
	}
	tof8801_infomsg( "%s: Ram patch in progress...\n", __func__);
	/* Assuming you can only perform ram download while in BL application */
	/* switch back to BL app to perform RAM download */
	if (chip->info_rec.record.app_id != TOF8801_APP_ID_BOOTLOADER) {
		tof8801_infomsg( "Current app_id: %hhx - Switching to bootloader for RAM download",
			chip->info_rec.record.app_id);
		result = tof_switch_apps(chip, (char)TOF8801_APP_ID_BOOTLOADER);
		if (result) {
			tof8801_infomsg( "Error changing to bootloader app: \'%d\'", result);
			goto err_fwdl;
		}
	}
	/* Start fwdl timer */
	ktime_get_real_ts64(&start_ts);
	/* setup encryption salt */
	result = tof8801_BL_upload_init(chip->client, &chip->BL_app, tof_salt_value);
	if (result) {
		tof8801_infomsg( "Error setting upload salt: \'%d\'", result);
		goto err_fwdl;
	}
	/* assume we have mutex already */
	intelHexInterpreterInitialise();
	line = cfg->data;
	line_end = line;
	while ((line_end - cfg->data) < cfg->size) {
		line_end = strchrnul(line, '\n');
		patch_size +=
			((line_end - line) > INTEL_HEX_MIN_RECORD_SIZE) ?
			((line_end - line - INTEL_HEX_MIN_RECORD_SIZE) / 2) : 0;
		result = intelHexHandleRecord(chip->client, &chip->BL_app,
			line_end - line, line, verify);
		if (result) {
			tof8801_errmsg("%s: Ram patch failed: %d\n", __func__, result);
			goto err_fwdl;
		}
		line = ++line_end;
	}
	/* Stop fwdl timer */
	ktime_get_real_ts64(&end_ts);
	fwdl_time = timespec64_sub(end_ts, start_ts).tv_nsec / 1000000; /* 1000000 : time in ms */
	tof8801_infomsg( "%s: Ram patch complete, patch size: %uK, dl time: %llu ms\n",
		__func__, ((patch_size >> 10) + 1), fwdl_time);
	/* wait for i2c */
	usleep_range(TOF8801_I2C_WAIT_USEC, TOF8801_I2C_WAIT_USEC + 1);
	/* resync our info record since we just switched apps */
	tof_init_info_record(chip);
err_fwdl:
	release_firmware(cfg);
	complete_all(&chip->ram_patch_in_progress);
	AMS_MUTEX_UNLOCK(&chip->lock);
}

int tof_queue_frame(struct tof_sensor_chip *chip, void *buf, int size)
{
	unsigned int fifo_len;
	unsigned int frame_size;
	int result = kfifo_in(&chip->tof_output_fifo, buf, size);
	if (result == 0) {
		if (chip->driver_debug == 1)
			tof8801_errmsg("Error: Frame buffer is full, clearing buffer.\n");
		kfifo_reset(&chip->tof_output_fifo);
		tof8801_app0_report_error(chip, ERR_BUF_OVERFLOW, DEV_OK);
		result = kfifo_in(&chip->tof_output_fifo, buf, size);
		if (result == 0)
			tof8801_errmsg("Error: queueing ToF output frame.\n");
	}
	if (chip->driver_debug == 2) {
		fifo_len = kfifo_len(&chip->tof_output_fifo);
		frame_size = ((char *)buf)[DRV_FRAME_SIZE_LSB] |
			(((char *)buf)[DRV_FRAME_SIZE_MSB] << 8);
		tof8801_infomsg( "Add frame_id: 0x%x, data_size: %u\n",
			((char *)buf)[DRV_FRAME_ID], frame_size);
		tof8801_infomsg( "New fifo len: %u, fifo utilization: %u%%\n",
			fifo_len, (1000 * fifo_len / kfifo_size(&chip->tof_output_fifo)) / 10);
	}
	return (result == size) ? 0 : -1;
}

/**
 * tof_irq_handler - The IRQ handler
 *
 * @irq: interrupt number.
 * @dev_id: private data pointer.
 */
static irqreturn_t tof_irq_handler(int irq, void *dev_id)
{
	struct tof_sensor_chip *tof_chip = (struct tof_sensor_chip *)dev_id;
	char int_stat = 0;
	char appid;
	int error;
	AMS_MUTEX_LOCK(&tof_chip->lock);
	/* Go to appropriate IRQ handler depending on the app running */
	appid = tof_chip->info_rec.record.app_id;
	switch (appid) {
	case TOF8801_APP_ID_BOOTLOADER:
		goto irq_handled;
	case TOF8801_APP_ID_APP0:
		(void)tof8801_get_register(tof_chip->client, TOF8801_INT_STAT, &int_stat);
		if (tof_chip->driver_debug)
			tof8801_infomsg("IRQ stat: %#x\n", int_stat);
		if (int_stat != 0) {
			/* Clear interrupt on ToF chip */
			error = tof8801_set_register(tof_chip->client, TOF8801_INT_STAT, int_stat);
			if (error)
				tof8801_app0_report_error(tof_chip, ERR_COMM, DEV_OK);
			tof8801_app0_process_irq(tof_chip, int_stat);
			/* Alert user space of changes */
			sysfs_notify(&tof_chip->client->dev.kobj,
				tof_app0_group.name, bin_attr_app0_tof_output.attr.name);
		}
		break;
	case TOF8801_APP_ID_APP1:
		goto irq_handled;
	default:
		break;
	}
irq_handled:
	AMS_MUTEX_UNLOCK(&tof_chip->lock);
	return IRQ_HANDLED;
}

int tof8801_app0_poll_irq_thread(void *tof_chip)
{
	struct tof_sensor_chip *chip = (struct tof_sensor_chip *)tof_chip;
	char meas_cmd = 0;
	int us_sleep;
	AMS_MUTEX_LOCK(&chip->lock);

	us_sleep = chip->poll_period * 100; /* Poll period is interpreted in units of 100 usec */
	tof8801_infomsg(
		"Starting ToF irq polling thread, period: %u us\n", us_sleep);
	AMS_MUTEX_UNLOCK(&chip->lock);
	while (!kthread_should_stop()) {
		AMS_MUTEX_LOCK(&chip->lock);
		meas_cmd = chip->app0_app.cap_settings.cmd;
		AMS_MUTEX_UNLOCK(&chip->lock);
		if (meas_cmd)
			(void)tof_irq_handler(0, tof_chip);
		usleep_range(us_sleep, us_sleep + us_sleep / 10); /* 10 us interval */
	}
	return 0;
}

/**
 * tof_request_irq - request IRQ for given gpio
 *
 * @tof_chip: tof_sensor_chip pointer
 */
static int tof_request_irq(struct tof_sensor_chip *tof_chip)
{
	int rc;
	rc = request_threaded_irq(tof_chip->irq, NULL,
		tof_irq_handler, IRQF_TRIGGER_FALLING | IRQF_ONESHOT,
		"tof8801_interrupt", (void *)tof_chip);
	if (rc)
		tof8801_errmsg("fail to req threaded irq, rc=%d\n", rc);
	else
		tof8801_infomsg("irq %d now handled\n", tof_chip->irq);
	return rc;
}

/**
 * tof8801_get_all_regs - read all addressable I2C registers from device
 *
 * @tof_chip: tof_sensor_chip pointer
 */
static int tof8801_get_all_regs(struct tof_sensor_chip *tof_chip)
{
	int error;

	error = memset_s(tof_chip->shadow, MAX_REGS, 0, MAX_REGS);
	if (error < 0) {
		tof8801_errmsg("memset failed");
		return error;
	}

	error = tof_i2c_read(tof_chip->client,
		TOF8801_APP_ID, tof_chip->shadow, MAX_REGS);
	if (error < 0) {
		tof8801_errmsg("Read all registers failed: %d\n", error);
		return error;
	}
	return 0;
}

/**
 * tof8801_enable_interrupts - enable specified interrutps
 *
 * @tof_chip: tof_sensor_chip pointer
 * @int_en_flags: OR'd flags of interrupts to enable
 */
static int tof8801_enable_interrupts(
	struct tof_sensor_chip *chip, char int_en_flags)
{
	char flags;
	int error = tof8801_get_register(chip->client, TOF8801_INT_EN, &flags);
	flags &= TOF8801_INT_MASK;
	flags |= int_en_flags;
	if (error)
		return error;
	return tof8801_set_register(chip->client, TOF8801_INT_EN, flags);
}

static int tof8801_get_config_calib_data(struct tof_sensor_chip *chip)
{
	int error;
	const struct firmware *config_fw = NULL;
	/* Set current configuration calibration data size to 0 */
	chip->config_data.size = 0;
	/* Check for available fac_calib to read */
	error = request_firmware_direct(&config_fw,
		chip->pdata->config_calib_data_fname, &chip->client->dev);
	if (error || config_fw == NULL) {
		tof8801_errmsg("configuration calibration data not available \'%s\': %d\n",
			chip->pdata->config_calib_data_fname, error);
		return 0;
	} else {
		tof8801_infomsg( "Read in config_calib file: \'%s\'.\n",
			chip->pdata->config_calib_data_fname);
	}
	if (config_fw->size > sizeof(chip->config_data.cfg_data)) {
		tof8801_errmsg(
			"Error: config calibration data size too large %u > %u (MAX)\n",
			config_fw->size,
			sizeof(chip->config_data.cfg_data));
		return 1;
	}
	if (memcpy_s((void *)&chip->config_data.cfg_data, config_fw->size,
		config_fw->data, config_fw->size) != EOK) {
		tof8801_errmsg("memcpy failed");
		return -1;
	}
	chip->config_data.size = config_fw->size;
	release_firmware(config_fw);
	return 0;
}

static int tof8801_get_fac_calib_data(struct tof_sensor_chip *chip)
{
	int error;
	const struct firmware *calib_fw = NULL;
	/* Set current factory calibration data size to 0 */
	chip->ext_calib_data.size = 0;
	/* Alg info is only valid with factory cal, so clear it as well */
	chip->alg_info.size = 0;
	/* Check for available fac_calib to read */
	error = request_firmware_direct(&calib_fw,
		chip->pdata->fac_calib_data_fname, &chip->client->dev);
	if (error || calib_fw == NULL) {
		tof8801_infomsg("factory calibration data not available \'%s\': %d\n",
			chip->pdata->fac_calib_data_fname, error);
		return 0;
	} else {
		tof8801_infomsg( "Read in fac_calib file: \'%s\'.\n",
			chip->pdata->fac_calib_data_fname);
	}
	if (calib_fw->size > sizeof(chip->ext_calib_data.fac_data)) {
		tof8801_errmsg(
			"Error: factory calibration data size too large %u > %u (MAX)\n",
			calib_fw->size,
			sizeof(chip->ext_calib_data.fac_data));
		return 1;
	}
	if (memcpy_s((void *)&chip->ext_calib_data.fac_data, calib_fw->size,
		calib_fw->data, calib_fw->size) != EOK) {
		tof8801_errmsg("memcpy error\n");
		return -1;
	}
	chip->ext_calib_data.size = calib_fw->size;
	release_firmware(calib_fw);
	return 0;
}

static int tof8801_get_fac_calib_data_from_arrey(
	struct tof_sensor_chip *chip, const char *buf, size_t count)
{
	uint8_t *pdata = NULL;

	/* Set current factory calibration data size to 0 */
	chip->ext_calib_data.size = 0;
	/* Alg info is only valid with factory cal, so clear it as well */
	chip->alg_info.size = 0;

	memcpy((void *)&chip->ext_calib_data.fac_data, buf, count);

	chip->ext_calib_data.size = count;
	pdata = &chip->ext_calib_data.fac_data.data[0];

	tof8801_infomsg(
		"%s: %d: %x %x %x %x %x %x %x %x %x %x %x %x %x %x", __func__, count,
		pdata[0], pdata[1], pdata[2], pdata[3], pdata[4], pdata[5], pdata[6],
		pdata[7], pdata[8], pdata[9], pdata[10], pdata[11], pdata[12], pdata[13]);

	return 0;
}

static int tof8801_firmware_download(struct tof_sensor_chip *chip, int startup)
{
	int error;
	struct timespec64 start_ts = { 0 };
	struct timespec64 end_ts = { 0 };
	int mutex_locked = mutex_is_locked(&chip->lock);
	int file_idx;
	ktime_get_real_ts64(&start_ts);
	/* Iterate through all Firmware(s) to find one that works. 'Works' here is
	 * defined as running APP0 after FWDL
	 */
	for (file_idx = 0; chip->pdata->ram_patch_fname[file_idx] != NULL; file_idx++) {
		/* ** reset completion event that FWDL is starting ** */
		reinit_completion(&chip->ram_patch_in_progress);
		if (mutex_locked) {
			AMS_MUTEX_UNLOCK(&chip->lock);
		}
		tof8801_infomsg( "Trying firmware: \'%s\'...\n",
			chip->pdata->ram_patch_fname[file_idx]);
		/* **** Check for available firmware to load **** */
		error = request_firmware_nowait(THIS_MODULE, true,
			chip->pdata->ram_patch_fname[file_idx], &chip->client->dev,
			GFP_KERNEL, chip, tof_ram_patch_callback);
		if (error)
			tof8801_infomsg("Firmware not available \'%s\': %d\n",
				chip->pdata->ram_patch_fname[file_idx], error);
		if (!startup && !wait_for_completion_interruptible_timeout(&chip->ram_patch_in_progress,
			msecs_to_jiffies(TOF_FWDL_TIMEOUT_MSEC)))
			tof8801_errmsg("Timeout waiting for Ram Patch \'%s\' Complete",
				chip->pdata->ram_patch_fname[file_idx]);
		if (mutex_locked) {
			AMS_MUTEX_LOCK(&chip->lock);
		}
		if (chip->info_rec.record.app_id == TOF8801_APP_ID_APP0) {
			/* assume we are done if APP0 is running */
			break;
		}
	}
	ktime_get_real_ts64(&end_ts);
	tof8801_dbgmsg("FWDL callback %lu ms to finish",
		(timespec64_sub(end_ts, start_ts).tv_nsec / 1000000)); /* 1000000 : ms */
	/* error if App0 is not running (fwdl failed) */
	return (chip->info_rec.record.app_id != TOF8801_APP_ID_APP0) ? -EIO : 0;
}

static int ctrl_perform_calibration(struct tof_sensor_chip *chip, void __user *p)
{
	int rc = 0;
	hwlaser_ioctl_perform_calibration_t *calib =
			(hwlaser_ioctl_perform_calibration_t *)p;

	switch (calib->calibration_type) {
	case HWLASER_CALIBRATION_CROSSTALK:
		tof8801_infomsg( "HWLASER_CALIBRATION_CROSSTALK");
		capture_store(&tof_chip->client->dev, 0, "1", 2);
		msleep(200);
		capture_store(&tof_chip->client->dev, 0, "0", 2);
		tof_chip->xtalk = tof_chip->xtalk_peak;
		tof8801_infomsg("xtalk_peak = %d", tof_chip->xtalk);
		break;
	case HWLASER_CALIBRATION_OFFSET_SIMPLE:
		tof8801_infomsg( "HWLASER_CALIBRATION_OFFSET_SIMPLE");
		app0_get_fac_calib_show(&chip->client->dev, 0, &chip->offset_data[0]);
		app0_apply_fac_calib_store(&chip->client->dev, 0, &chip->offset_data[0], 14);
		break;
	default:
		tof8801_errmsg("unsupport calibration type");
		rc = -EINVAL;
		break;
	}

done:
	return rc;
}

static int tof8801_laser_get_set_cal_data(
	struct tof_sensor_chip *chip, hwlaser_calibration_data_t *p)
{
	int ret;

	hwlaser_calibration_data_t *cal_data = NULL;
	mutex_lock(&chip->lock);
	cal_data = (hwlaser_calibration_data_t *)p;
	if (cal_data->is_read) {
		tof8801_infomsg( "hal get calibration data from driver");
		memset(&(cal_data->u.dataL4), 0, sizeof(hwlaser_calibration_data_L4));

		/* send offset_data to hal */
		ret = memcpy_s(&(cal_data->u.dataL4.offset_data[0]),
			14, &(chip->offset_data[0]), 14);
		if (ret) {
			tof8801_errmsg("offset data memcpy fail");
			goto done;
		}
		memcpy(&(p->u.dataL4.offset_data[0]),
			&(cal_data->u.dataL4.offset_data[0]),
			sizeof(p->u.dataL4.offset_data));
		/* send xtalk to hal */
		ret = memcpy_s(&(cal_data->u.dataL4.xtalk), sizeof(unsigned int),
			&chip->xtalk, sizeof(unsigned int));
		if (ret) {
			tof8801_errmsg("xtalk memcpy fail");
			goto done;
		}
		memcpy(&(p->u.dataL4.xtalk),
			&(cal_data->u.dataL4.xtalk),
			sizeof(unsigned int));
	} else {
		tof8801_infomsg(
			"hal set calibration data to driver, driver only need offset data.");
		/* calib data set to driver */
		ret = app0_apply_fac_calib_store(&chip->client->dev, 0,
			&(cal_data->u.dataL4.offset_data[0]), 14);
	}

done:
	mutex_unlock(&chip->lock);
	return ret;
}

static int tof8801_laser_get_data(struct tof_sensor_chip *chip, void *p)
{
	int ret;
	uint32_t distance;
	uint32_t confidence;
	uint32_t status = 0;
	hwlaser_RangingData_t* udata = NULL;
    udata = (hwlaser_RangingData_t*) p;

	distance = chip->distance;
	confidence = chip->confidence;

	if (distance < DISTANCE_THRESHOLD && confidence >= CONFIDENCE_THRESHOLD_LOW) {
		status = RANGESTATUS_CONFIDENT;
	} else if (distance >= DISTANCE_THRESHOLD) {
		if (confidence >= CONFIDENCE_THRESHOLD_HIGH) {
			status = RANGESTATUS_CONFIDENT;
		} else if (confidence >= CONFIDENCE_THRESHOLD_MID) {
			status = RANGESTATUS_SEMI_CONFIDENT;
		}
	} else {
		status = RANGESTATUS_NOT_CONFIDENCT;
	}

	chip->udata.u.data_1.distance = distance;
	chip->udata.u.data_1.status = status;

	udata->u.data_1.distance = distance;
	udata->u.data_1.status = status;
	tof8801_infomsg("distance = %u status = %u", distance, status);

	return ret;
}

static void Chip_init(struct tof_sensor_chip *chip)
{
	int error = 0;
	AMS_MUTEX_LOCK(&chip->lock);
	/***** Make firmware download available to user space *****/
	error = tof8801_firmware_download(chip, 0);
	if (error) {
		AMS_MUTEX_UNLOCK(&chip->lock);
		return;
	}
	tof8801_infomsg("firmware download success");
	/* enable all ToF interrupts on sensor */
	tof8801_enable_interrupts(chip, IRQ_RESULTS | IRQ_DIAG | IRQ_ERROR);

	AMS_MUTEX_UNLOCK(&chip->lock);
	return;
}

long tof_ioctl(void *hw_data, unsigned int cmd, void *p)
{
	int rc = 0;
	const char *capture_on = "1";
	const char *capture_off = "0";
	const char *period = "50"; /* fix period to 50 when in ioctl mode */
	hwlaser_calibration_data_t *cal_data = NULL;
	hwlaser_info_t *pinfo = NULL;

	if (tof_chip == NULL) {
		return -EINVAL;
	}

	switch (cmd) {
	case HWLASER_IOCTL_GET_INFO:
		tof8801_infomsg("HWLASER_IOCTL_GET_INFO");
		if (strlen(DEV_NAME) < HWLASER_NAME_SIZE)
			strncpy_s(tof_chip->pinfo.name,
				HWLASER_NAME_SIZE, DEV_NAME, strlen(DEV_NAME));
		tof_chip->pinfo.i2c_idx = 2;
		tof_chip->pinfo.valid = 1;
		tof_chip->pinfo.version = HWLASER_AMS_I0_VERSION;
		tof_chip->pinfo.ap_pos = HWLASER_POS_AP;
		pinfo = (hwlaser_info_t *)p;
		if (strncpy_s(pinfo->name, HWLASER_NAME_SIZE - 1 ,"tof8801", HWLASER_NAME_SIZE - 1))
			tof8801_errmsg("strncpy_s failed %d", __LINE__);
		if (strncpy_s(pinfo->product_name, HWLASER_NAME_SIZE - 1 ,"LASER", HWLASER_NAME_SIZE - 1))
			tof8801_errmsg("strncpy_s failed %d", __LINE__);
		pinfo->version = HWLASER_AMS_I0_VERSION;
		pinfo->ap_pos = HWLASER_POS_AP;
		break;
	case HWLASER_IOCTL_PERFORM_CALIBRATION:
		tof8801_infomsg("HWLASER_IOCTL_PERFORM_CALIBRATION");
		rc = ctrl_perform_calibration(tof_chip, p);
		break;
	case HWLASER_IOCTL_CALIBRATION_DATA:
		tof8801_infomsg("HWLASER_IOCTL_CALIBRATION_DATA");
		cal_data = (hwlaser_calibration_data_t *)p;
		rc = tof8801_laser_get_set_cal_data(tof_chip, cal_data);
		break;
	case HWLASER_IOCTL_STOP:
		tof8801_infomsg("HWLASER_IOCTL_STOP");
		capture_store(&tof_chip->client->dev, 0, capture_off, 2);
		break;
	case HWLASER_IOCTL_START:
		tof8801_infomsg("HWLASER_IOCTL_START");
		period_store(&tof_chip->client->dev, 0, period, 3);
		capture_store(&tof_chip->client->dev, 0, capture_on, 2);
		msleep(100);
		break;
	case HWLASER_IOCTL_MZ_DATA:
		tof8801_infomsg("HWLASER_IOCTL_MZ_DATA");
		rc = tof8801_laser_get_data(tof_chip, p);
		break;
	case HWLASER_IOCTL_POWERON:
		tof8801_infomsg("HWLASER_IOCTL_POWERON");
		Chip_init(tof_chip);
		break;
	case HWLASER_IOCTL_MATCHID:
		tof8801_infomsg("HWLASER_IOCTL_MATCHID");
		break;
	case HWLASER_IOCTL_INIT:
		tof8801_infomsg("HWLASER_IOCTL_INIT");
		break;
	default:
		tof8801_errmsg("unsupport cmd");
		rc = -EINVAL;
		break;
	}
	return rc;
}
EXPORT_SYMBOL(tof_ioctl);

static int tof_open(struct inode *inode, struct file *filep)
{
	tof8801_infomsg("tof open");

	return 0;
}

long tof_file_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
{
	return 0;
}

static struct file_operations tmf8801_ops = {
	.owner = THIS_MODULE,
	.unlocked_ioctl = tof_file_ioctl,
	.open = tof_open,
	.release = NULL,
};

static int get_xsdn(struct tof_sensor_chip *tof_chip)
{
	int rc;

	if (tof_chip == NULL) {
		tof8801_errmsg("tof8801 tof_chip is null");
		return -1;
	}

	if (tof_chip->gpio_enable == -1) {
		tof8801_errmsg("tof8801 EN gpio is required");
		rc = -ENODEV;
		return rc;
	}

	tof8801_infomsg("tof8801 request EN gpio %d", tof_chip->gpio_enable);
	rc = gpio_request(tof_chip->gpio_enable, "tof8801 EN");
	if (rc) {
		tof8801_errmsg("tof8801 fail to acquire EN gpio %d", rc);
		return rc;
	}

	return rc;
}

static void put_xsdn(struct tof_sensor_chip *tof_chip)
{
	if (tof_chip == NULL) {
		tof8801_errmsg("tof8801 tof_chip is null");
		return;
	}

	tof8801_infomsg("tof8801 release EN gpio %d", tof_chip->gpio_enable);
	if (tof_chip->gpio_enable != -1) {
		gpio_free(tof_chip->gpio_enable);
		tof_chip->gpio_enable = -1;
	}
}

static void put_intr(struct tof_sensor_chip *tof_chip)
{
	if (tof_chip == NULL) {
		tof8801_errmsg("tof8801 tof_chip is null");
		return;
	}

	tof8801_infomsg("tof8801 release intr gpio %d", tof_chip->gpio_interrupt);
	if (tof_chip->gpio_interrupt != -1) {
		gpio_free(tof_chip->gpio_interrupt);
		tof_chip->gpio_interrupt = -1;
	}
}

static int get_intr(struct tof_sensor_chip *tof_chip)
{
	int rc = 0;

	if (tof_chip == NULL) {
		tof8801_errmsg("tof8801 tof_chip is null");
		return -1;
	}

	if (tof_chip->gpio_interrupt == -1) {
		tof8801_errmsg("tof8801 no interrupt gpio");
		goto request_failed;
	}

	tof8801_infomsg("tof8801 request intr_gpio %d", tof_chip->gpio_interrupt);
	rc = gpio_request(tof_chip->gpio_interrupt, "tof8801 intr");
	if (rc) {
		tof8801_errmsg("tof8801 fail to acquire intr gpio %d", rc);
		goto request_failed;
	}

	rc = gpio_direction_input(tof_chip->gpio_interrupt);
	if (rc) {
		tof8801_errmsg("tof8801 to configure intr as input %d", rc);
		goto direction_failed;
	}

	tof_chip->irq = gpio_to_irq(tof_chip->gpio_interrupt);
	if (tof_chip->irq < 0) {
		tof8801_errmsg("tof8801 fail to map GPIO: %d to interrupt:%d",
			tof_chip->gpio_interrupt, tof_chip->irq);
		goto direction_failed;
	}

	return rc;

direction_failed:
	put_intr(tof_chip);
request_failed:
	return rc;
}

ssize_t tof8801_not_change_iterations(struct tof_sensor_chip *tof_chip)
{
	return 0;
}

static ssize_t tof8801_parse_iterations(struct tof_sensor_chip *tof_chip)
{
	return 0;
}

static int tof8801_parse_tree(
	struct device_node *node,
	struct tof_sensor_chip *tof_chip)
{
	int rc;
	enum of_gpio_flags flags;
	if (tof_chip == NULL) {
		tof8801_errmsg("tof8801 invalid args");
		return -1;
	}

	tof_chip->gpio_enable = of_get_named_gpio_flags(node, "tof8801,en-gpio", 0, &flags);
	if (tof_chip->gpio_enable < 0) {
		tof8801_errmsg("tof8801 parse gpio-enable failed");
		return -ENODEV;
	}

	tof_chip->gpio_interrupt = of_get_named_gpio_flags(node, "tof8801,intr-gpio", 0, &flags);;
	if (tof_chip->gpio_enable < 0) {
		tof8801_errmsg("tof8801 parse gpio-interrupt failed");
		return -ENODEV;
	}

	/* configure gpios */
	rc = get_xsdn(tof_chip);
	if (rc)
		goto no_xsdn;
	rc = get_intr(tof_chip);
	if (rc)
		goto no_intr;

	return rc;

no_intr:
	put_xsdn(tof_chip);
no_xsdn:
	return rc;
}

static void tof8801_release_gpios(struct tof_sensor_chip *tof_chip)
{
	if (tof_chip == NULL) {
		tof8801_errmsg("tof8801 tof_chip is null");
		return;
	}

	put_xsdn(tof_chip);
	put_intr(tof_chip);
}

int tof_probe(struct i2c_client *client, const struct i2c_device_id *idp)
{
	int error;
	void *poll_prop_ptr = NULL;
	u8 status = 0;
	struct device *dev = &client->dev;
	struct device_node *node = NULL;
	hw_laser_ctrl_t *ctrl = NULL;
	int gpio_status = 0;

	tof8801_infomsg("enter tof8801 tof_probe");
	if (!i2c_check_functionality(client->adapter, I2C_FUNC_I2C)) {
		tof8801_errmsg("tof8801 i2c check functionality failed.");
		return -ENXIO;
	}

	tof_chip = kzalloc(sizeof(struct tof_sensor_chip), GFP_KERNEL);
	if (tof_chip == NULL) {
		return -ENOMEM;
	}
	if (!dev->of_node) {
		kfree(tof_chip);
		return -EINVAL;
	}
	/* Setup data structures */
	mutex_init(&tof_chip->lock);
	client->dev.platform_data = (void *)&tof_pdata;
	tof_chip->client = client;
	tof_chip->pdata = &tof_pdata;
	tof_chip->clsdev = dev;
	tof_chip->ctrl = ctrl;
	i2c_set_clientdata(client, tof_chip);
	ctrl = (hw_laser_ctrl_t *)idp->driver_data;
	ctrl->data = (void*)tof_chip;
	node = dev->of_node;

	/* Firmware sync structure initialization */
	init_completion(&tof_chip->ram_patch_in_progress);
	/* initialize kfifo for frame output */
	INIT_KFIFO(tof_chip->tof_output_fifo);
	/* Setup measure timer */
	timer_setup(&tof_chip->meas_timer,
		tof8801_app0_measure_timer_expiry_callback, 0);

	/* parse device tree */
	error = tof8801_parse_tree(node, tof_chip);
	if (error)
		goto parse_tree_err;

	/* Set ChipEnable HIGH */
	tof_chip->pdata->gpiod_enable = &tof_chip->gpio_enable;
	tof_chip->pdata->gpiod_interrupt = &tof_chip->gpio_interrupt;
	if (tof_chip->pdata->gpiod_enable) {
		error = gpio_direction_output(*(tof_chip->pdata->gpiod_enable), 1);
		if (error) {
			tof8801_errmsg("tof8801 gpio direction output failed.");
			goto gpio_err;
		}
		gpio_status = gpio_get_value(*(tof_chip->pdata->gpiod_enable));
		tof8801_infomsg("tof8801 EN gpio_status is %d", gpio_status);
		error = tof_wait_for_cpu_startup(tof_chip->client);
		if (error) {
			tof8801_errmsg("tof8801 I2C communication failure: %d", error);
			goto gpio_err;
		}
	} else {
		tof8801_errmsg("tof8801 gpiod_enable is nullptr");
		goto parse_tree_err;
	}

	/* get chipid */
	error = tof8801_get_register(client, TOF8801_ID, &status);
	if (error) {
		tof8801_errmsg("tof8801 get chip-id register failed %d", error);
		goto gpio_err;
	} else {
		if (status != 0x07) {
			tof8801_errmsg("tof8801 registerid value is: %#04x", status);
			goto gpio_err;
		}
		tof8801_infomsg("tof8801 match id susccess");
	}

	/* use pooling mode or interupt mode */
	poll_prop_ptr = (void *)of_get_property(tof_chip->client->dev.of_node,
		TOF_PROP_NAME_POLLIO, NULL);
	tof_chip->poll_period = poll_prop_ptr ? be32_to_cpup(poll_prop_ptr) : 0;
	if (tof_chip->poll_period == 0) {
		/* Use Interrupt I/O instead of polled */
		/* Setup GPIO IRQ handler */
		if (tof_chip->pdata->gpiod_interrupt) {
			error = tof_request_irq(tof_chip);
			if (error) {
				tof8801_errmsg("tof8801 interrupt request failed.");
				goto gen_err;
			}
			tof8801_infomsg("tof8801 interrupt mode start.");
		}
	} else {
		/* Use Polled I/O instead of interrupt */
		tof_chip->app0_poll_irq = kthread_run(tof8801_app0_poll_irq_thread,
			(void *)tof_chip, "tof-irq_poll");
		if (IS_ERR(tof_chip->app0_poll_irq)) {
			tof8801_errmsg("tof8801 error starting IRQ polling thread.");
			error = PTR_ERR(tof_chip->app0_poll_irq);
			goto gen_err;
		}
		tof8801_infomsg("tof8801 polled I/O mode start.");
	}

	/* ioctrl interface */
	tof_chip->major = register_chrdev(0, DEV_NAME, &tmf8801_ops);
	if (tof_chip->major < 0) {
		tof8801_errmsg("tof8801 register_chrdev failed.");
		goto gen_err;
	}

	/* class dev create */
	tof_chip->cls = class_create(THIS_MODULE, DEV_NAME);
	if (IS_ERR(tof_chip->cls)) {
		tof8801_errmsg("tof8801 class_create failed.");
		goto chrdev_create_err;
	}
	tof_chip->clsdev = device_create(tof_chip->cls, NULL,
		MKDEV(tof_chip->major, 0), NULL, DEV_NAME);
	if (IS_ERR(tof_chip->clsdev)) {
		tof8801_errmsg("tof8801 device_create failed.");
		goto device_create_err;
	}

	/* Wait until ToF is ready for commands */
	error = tof_wait_for_cpu_startup(client);
	if (error) {
		tof8801_errmsg("tof8801 i2c communication failure: %d", error);
		goto device_create_err;
	}

	tof_chip->saved_clk_trim = UNINITIALIZED_CLK_TRIM_VAL;
	/* read external (manufacturer) configuration data */
	error = tof8801_get_config_calib_data(tof_chip);
	if (error)
		tof8801_errmsg("tof8801 error reading config data: %d", error);

	/* read external (manufacturer) factory calibration data */
	error = tof8801_get_fac_calib_data(tof_chip);
	if (error)
		tof8801_errmsg("tof8801 error reading fac_calib data: %d", error);

	tof8801_app0_default_cap_settings(&tof_chip->app0_app);

	error = tof_init_info_record(tof_chip);
	if (error) {
		tof8801_errmsg("tof8801 read application info record failed.");
		goto device_create_err;
	}

	tof_chip->app0_app.distPeak_previous = 0;
	tof_chip->app0_app.reliabilityE_previous = 0;

	/* enable all ToF interrupts on sensor */
	tof8801_enable_interrupts(tof_chip, IRQ_RESULTS | IRQ_DIAG | IRQ_ERROR);


#ifdef CONFIG_HUAWEI_HW_DEV_DCT
	set_hw_dev_flag(DEV_I2C_LASER);
#endif


	tof8801_infomsg("tof8801 probe ok.");

	/* match id succeed, then set devcheck flag */

	return 0;

device_create_err:
	class_destroy(tof_chip->cls);
chrdev_create_err:
	unregister_chrdev(tof_chip->major, DEV_NAME);
gen_err:
	if (tof_chip->poll_period != 0)
		(void)kthread_stop(tof_chip->app0_poll_irq);
gpio_err:
	if (tof_chip->pdata->gpiod_enable)
		(void)gpio_direction_output(*(tof_chip->pdata->gpiod_enable), 0);
	tof8801_release_gpios(tof_chip);
parse_tree_err:
	tof8801_errmsg("tof8801 probe failed.");
	kfree(tof_chip);
	return error;
}
EXPORT_SYMBOL(tof_probe);

int tof_remove(struct i2c_client *client)
{
	struct tof_sensor_chip *chip = i2c_get_clientdata(client);
	char int_stat = 0;
	if (chip->info_rec.record.app_id == TOF8801_APP_ID_APP0) {
		/* Stop any current measurements */
		tof8801_app0_capture(chip, 0);
		(void)tof8801_get_register(chip->client, TOF8801_INT_STAT, &int_stat);
		/* Clear any interrupt status */
		if (int_stat != 0)
			(void)tof8801_set_register(chip->client, TOF8801_INT_STAT, int_stat);
	}
	if (chip->pdata->gpiod_interrupt)
		devm_free_irq(&client->dev, client->irq, chip);
	if (chip->poll_period != 0)
		(void)kthread_stop(chip->app0_poll_irq);
	/* disable the chip */
	if (chip->pdata->gpiod_enable)
		(void)gpio_direction_output(*(chip->pdata->gpiod_enable), 0);
	del_timer_sync(&chip->meas_timer); /* delete measure timer */
	/* ioctrl interface */
	device_destroy(chip->cls, MKDEV(chip->major, 0));
	class_destroy(chip->cls);
	unregister_chrdev(chip->major, DEV_NAME);

	return 0;
}
EXPORT_SYMBOL(tof_remove);