/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 *
 * Description: tof8801 driver
 */

#include <linux/timer.h>
#include <securec.h>
#include "tof8801_app0.h"
#include "tof8801_driver.h"

#define MEASURE_ISLOCK(m) ({ \
		int __rc = (m == 1); \
		__rc; \
	})
#define MEASURE_LOCK(m) ({ \
		m = 1; \
		m; \
	})
#define MEASURE_UNLOCK(m) ({ \
		m = 0; \
		m; \
	})


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

static int tof8801_app0_start_measure_timer(struct tof_sensor_chip *chip, unsigned char cmd);
static int tof8801_app0_stop_measurements(struct tof_sensor_chip *chip);
static int tof8801_app0_osc_trim(struct tof_sensor_chip *chip, int change);
static int tof8801_app0_check_osc(struct tof_sensor_chip *chip);

static void TMF8801_cr_init(TMF8801_cr *cr)
{
	/* start a new recalculation cycle, beginning with the default ratio 0.2 */
	cr->first_host     = 0;
	cr->last_host      = 0;
	cr->first_i2c      = 0;
	cr->last_i2c       = 0;
	cr->ratioQ15       = TOF8801_APP0_EXP_FREQ_RATIO_Q15;
	cr->count          = 0;
	cr->trim_count     = 0;
}

static void TMF8801_cr_recalc(TMF8801_cr *cr)
{
	/* start a new recalculation cycle, but don't lose the latest ratioQ15 */
	cr->count = 0;
}

static void TMF8801_cr_addpair(TMF8801_cr *cr, uint32_t host, uint32_t i2c)
{
	uint32_t num = 0;
	uint32_t den = 0;
	uint32_t denQM15;

	/* add a pair of host and I2C times */
	if (i2c == 0)
		return;
	/* this should not be possible since host count reference is unix_epoch */
	if (host < cr->last_host)
		cr->count = 0;
	if (i2c < cr->last_i2c)
		cr->count = 0;

	/*
	 * Please refer to the Customer tree driver(s) for workarounds of the
	 * issue with the Firmware i2c counter having large jumps due to a race
	 * condition
	 */
	if (cr->count == 0) {
		cr->first_host = host;
		cr->first_i2c = i2c;
	}
	cr->last_host = host;
	cr->last_i2c = i2c;
	cr->count += 1;
	if (cr->count >= TMF8801_MINCOUNT) {
		cr->trim_count += 1;
		num = cr->last_host - cr->first_host;
		den = cr->last_i2c - cr->first_i2c;
		while ((num < TMF8801_CR_WRAPAROUND_DIV2) && (den < TMF8801_CR_WRAPAROUND_DIV2)) {
			num <<= 1;
			den <<= 1;
		};
		denQM15 = ((den + (1 << 14)) >> 15); /* round up, always positive */
		if ((denQM15 == 0) || (num > den))
			cr->count = 0;
		else
			cr->ratioQ15 = (num + (denQM15 >> 1)) / denQM15;
	}
}

static uint32_t TMF8801_cr_map(TMF8801_cr *cr, uint32_t distance)
{
	/* apply the mapping function to calculate a clock-corrected distance */
	return (distance * TMF8801_EXPECTEDRATIO * cr->ratioQ15 + (1 << 14)) >> 15;
}

void tof8801_app0_set_clk_iterations(void *tof_chip, int capture_iterations)
{
	struct tof_sensor_chip *chip = (struct tof_sensor_chip *)tof_chip;
	if (capture_iterations == 0) {
		chip->app0_app.clk_iterations = TOF8801_APP0_DEFAULT_CLK_CNT;
	} else {
		chip->app0_app.clk_iterations = TOF8801_APP0_CLK_CNT_CONSTANT / capture_iterations;
		chip->app0_app.clk_iterations = clamp((int)chip->app0_app.clk_iterations,
			1, TOF8801_APP0_DEFAULT_CLK_CNT);
	}
	if (((uint8_t)chip->app0_app.cap_settings.period) >= 0xFE) {
		// set clock iterations to minimum for large report periods (1/0.5 Hz)
		chip->app0_app.clk_iterations = 1;
	}
	tof8801_infomsg( "Clock trim iterations set to: %u\n",
		chip->app0_app.clk_iterations);
}

int tof8801_app0_is_v2(void *tof_chip)
{
	struct tof_sensor_chip *chip = (struct tof_sensor_chip *)tof_chip;
	/* check for version >= v2 (assume v3+ is backward compatible with v2) */
	return (chip->info_rec.record.app_ver >= TOF8801_APP0_VER_V2);
}

int tof8801_app0_measure_in_progress(void *tof_chip)
{
	struct tof_sensor_chip *chip = (struct tof_sensor_chip *)tof_chip;
	return MEASURE_ISLOCK(chip->app0_app.measure_in_prog);
}

int tof8801_app0_wait_for_idle(void *tof_chip, unsigned long usec_timeout)
{
	int error;
	unsigned long curr = jiffies;
	unsigned char cpu_stat, state;
	struct tof_sensor_chip *chip = (struct tof_sensor_chip *)tof_chip;
	do {
		error = tof8801_get_register(chip->client, TOF8801_STAT, &cpu_stat);
		if (error == 0) {
			error = tof8801_get_register(chip->client, OL_STATE_OFFSET, &state);
			if ((error == 0) && TOF8801_STAT_CPU_READY(cpu_stat) && (state == OL_STATE_IDLE))
				return 0;
		}
	} while ((jiffies - curr) < usecs_to_jiffies(usec_timeout));
	tof8801_errmsg(
		"Error App0 timeout %lu usec waiting on idle; cpu_stat: %#x state: %#x\n",
		usec_timeout, cpu_stat, state);
	return -EIO;
}

int tof8801_app0_rw_osc_trim(void *tof_chip, int *trim_parm, int write_op)
{
	int error;
	int save_state;
	int trim_val;
	char fuse3 = 0;
	char fuse6 = 0;
	struct tof_sensor_chip *chip = (struct tof_sensor_chip *)tof_chip;

	if (trim_parm == NULL) {
		tof8801_errmsg("Error: trim value is null pointer");
		return -1;
	}
	/* Save current capture state */
	save_state = !tof8801_app0_stop_measurements(chip);
	error = tof8801_app0_capture(chip, 0);
	if (error) {
		tof8801_errmsg("Error (%d): stopping capture", error);
		return error;
	}

	error = tof8801_set_register(chip->client, OL_CMD_DATA9_OFFSET,
		TOF8801_APP0_OSC_TRIM_CMD);
	if (error) {
		tof8801_errmsg("Error (%d): writing osc_trim command", error);
		return error;
	}
	error = tof8801_set_register(chip->client, TOF8801_STAT, 0x00);
	if (error) {
		tof8801_errmsg("Error (%d): setting PON0", error);
		return error;
	}
	usleep_range(5000, 5500);

	error = tof8801_get_register(chip->client, 0x03, &fuse3);
	if (error) {
		tof8801_errmsg("Error (%d): reading Fuse_3", error);
		return error;
	}
	error = tof8801_get_register(chip->client, 0x06, &fuse6);
	if (error) {
		tof8801_errmsg("Error (%d): reading Fuse_6", error);
		return error;
	}

	if (write_op) {
		trim_val = *trim_parm;
		trim_val &= 0x1FF;
		tof8801_infomsg("%s: new osc_trim: %#x", __func__, trim_val);
		fuse6 &= ~(0x01 << 6);
		fuse6 |= ((trim_val & 0x01) << 6);
		fuse3 = ((trim_val >> 1) & 0xFF);
		error = tof8801_set_register(chip->client, 0x03, (char)fuse3);
		if (error) {
			tof8801_errmsg("Error (%d): writing Fuse_3", error);
			return error;
		}
		error = tof8801_set_register(chip->client, 0x06, (char)fuse6);
		if (error) {
			tof8801_errmsg("Error (%d): writing Fuse_6", error);
			return error;
		}
	} else {
		trim_val = 0;
		trim_val = (((int)fuse3) << 1) | ((fuse6 & (0x01 << 6)) >> 6);
		*trim_parm = trim_val;
		tof8801_infomsg(
			"%s: current osc_trim: %#x, clk_ratio "
			"(Q15): %u",
			__func__, trim_val, chip->app0_app.clk_cr.ratioQ15);
	}

	error = tof8801_set_register(chip->client, TOF8801_STAT, 0x01);
	if (error) {
		tof8801_errmsg("Error (%d): setting PON1", error);
		return error;
	}
	/* wait for app0 to come to idle state */
	error = tof8801_app0_wait_for_idle((void *)chip, 50000);
	if (error) {
		tof8801_errmsg("Error (%d): idle state timeout", error);
		return error;
	}
	/* Restart capture if previously in continuous mode */
	if (save_state) {
		error = tof8801_app0_capture((void *)chip, save_state);
		if (error) {
			tof8801_errmsg("Error (%d): trying to restart capture", error);
			return error;
		}
	}
	chip->saved_clk_trim = trim_val; /* cache trim value */
	return error;
}

static int tof8801_app0_osc_trim(struct tof_sensor_chip *chip, int change)
{
	int error;
	int save_state;
	int trim_val;
	char fuse3 = 0;
	char fuse6 = 0;
	/*
		Oscillator Trimming procedure:
		1. write 0x06: 0x29
		2. write 0xE0 = 0x00 (PON0)
		3. read/write 0x03
		4. read/write 0x06 bit 6
		5. write 0xE0 = 0x01 (PON1)
		6. wait for CPU_READY, error if still 0x00 after timeout (chip reset)
	*/
	/* Save current capture state */
	save_state = !tof8801_app0_stop_measurements(chip);
	error = tof8801_app0_capture(chip, 0);
	if (error) {
		tof8801_errmsg("Error (%d): stopping capture", error);
		return error;
	}

	error = tof8801_set_register(chip->client, OL_CMD_DATA9_OFFSET,
		TOF8801_APP0_OSC_TRIM_CMD);
	if (error) {
		tof8801_errmsg("Error (%d): writing osc_trim command", error);
		return error;
	}
	error = tof8801_set_register(chip->client, TOF8801_STAT, 0x00);
	if (error) {
		tof8801_errmsg("Error (%d): setting PON0", error);
		return error;
	}
	usleep_range(5000, 5500);
	/* Start osc trimming */
	error = tof8801_get_register(chip->client, 0x03, &fuse3);
	if (error) {
		tof8801_errmsg("Error (%d): reading Fuse_3", error);
		return error;
	}

	error = tof8801_get_register(chip->client, 0x06, &fuse6);
	if (error) {
		tof8801_errmsg("Error (%d): reading Fuse_6", error);
		return error;
	}

	trim_val = (((int)fuse3) << 1) | ((fuse6 & (0x01 << 6)) >> 6);
	tof8801_infomsg(
		"%s: current osc_trim: %#x, clk_ratio "
		"(Q15): %u",
		__func__, trim_val, chip->app0_app.clk_cr.ratioQ15);
	trim_val += change;
	trim_val &= 0x1FF;
	tof8801_infomsg("%s: new osc_trim: %#x", __func__, trim_val);
	chip->saved_clk_trim = trim_val; /* cache trim value */
	fuse6 &= ~(0x01 << 6);
	fuse6 |= ((trim_val & 0x01) << 6);
	fuse3 = ((trim_val >> 1) & 0xFF);

	error = tof8801_set_register(chip->client, 0x03, (char)fuse3);
	if (error) {
		tof8801_errmsg("Error (%d): writing Fuse_3", error);
		return error;
	}

	/* End osc trimming */
	error = tof8801_set_register(chip->client, 0x06, (char)fuse6);
	if (error) {
		tof8801_errmsg("Error (%d): writing Fuse_6", error);
		return error;
	}

	error = tof8801_set_register(chip->client, TOF8801_STAT, 0x01);
	if (error) {
		tof8801_errmsg("Error (%d): setting PON1", error);
		return error;
	}
	/* wait for app0 to come to idle state */
	error = tof8801_app0_wait_for_idle((void *)chip, 50000);
	if (error) {
		tof8801_errmsg("Error (%d): idle state timeout", error);
		return error;
	}
	/* Restart capture if previously in continuous mode */
	if (save_state) {
		error = tof8801_app0_capture((void *)chip, save_state);
		if (error) {
			tof8801_errmsg("Error (%d): trying to restart capture", error);
			return error;
		}
	}
	return error;
}

static int tof8801_app0_pll_corrected(struct tof_sensor_chip *chip, unsigned int ratio, int qf)
{
	unsigned int new_ratio = ratio;
	unsigned int k1 = 83090840;
	unsigned int k2 = 495559115;
	int period = chip->app0_app.cap_settings.period;
	int is_prox_only = ((chip->app0_app.cap_settings.v2.alg & 0x3) == 0x01);
	if (period >= 6 && is_prox_only) {
		new_ratio -= ((k1 - (k2 + (period >> 1)) / period) >> 20);
		if (chip->driver_debug)
			tof8801_infomsg(
				"PLL_correction (Q%d): "
				"%u -> %u\n",
				qf, ratio, new_ratio);
	}
	return new_ratio;
}

static int tof8801_app0_check_osc(struct tof_sensor_chip *chip)
{
	int error = 0;
	unsigned int ratioQ15 = chip->app0_app.clk_cr.ratioQ15;
	static int stable = 0;
	int trim;

	if (chip->app0_app.clk_cr.trim_count >= chip->app0_app.clk_iterations) {
		/* 4.7 MHz is desired clk freq */
		ratioQ15 = tof8801_app0_pll_corrected(chip, ratioQ15, 15);
		chip->app0_app.clk_cr.ratioQ15 = ratioQ15;
		chip->app0_app.clk_cr.trim_count = 0;
		if (chip->app0_app.clk_trim_enable) {
			trim = TOF8801_APP0_RATIO_TO_TRIM_CNT(ratioQ15);
			if (trim == 0 && stable != 1) {
				tof8801_infomsg(
					"%s: clk_ratio (Q15): %u "
					"is within tolerance\n",
					__func__, chip->app0_app.clk_cr.ratioQ15);
				stable = 1;
			} else if (trim != 0) {
				stable = 0;
				tof8801_infomsg(
					"%s: clk_ratio (Q15): %u "
					"trim clock by: [%d] steps\n",
					__func__, chip->app0_app.clk_cr.ratioQ15, trim);

				error = tof8801_app0_osc_trim(chip, trim);
			}
		}
	}
	return error;
}

void tof8801_app0_report_error(void *tof_chip, char driver_err, char device_err)
{
	struct tof_sensor_chip *chip = (struct tof_sensor_chip *)tof_chip;
	struct tof8801_app0_error *err_str = &chip->app0_app.error_frame.error;
	err_str->header.id = ID_ERROR;
	err_str->header.size_lsb = TOF8801_APP0_ERROR_DATA_SIZE;
	err_str->header.size_msb = 0;
	err_str->driver_err = driver_err;
	err_str->device_err = device_err;
	/*
	 * If we fail to report the error because of a full buffer, there isn't
	 * anything else we can do (possible future enhancement is a signal to
	 * userspace process)
	 */
	(void)tof_queue_frame(chip, chip->app0_app.error_frame.buf,
		APP0_DRV_FRAME_SIZE(chip->app0_app.error_frame.buf));
	MEASURE_UNLOCK(chip->app0_app.measure_in_prog);
}

static int app0_wait_for_tid_change(struct tof_sensor_chip *chip, int num_tries)
{
	int tries = 0;
	int error;
	char new_tid;
	struct tof8801_app0_application *app0 = &chip->app0_app;
	do {
		tries++;
		error = tof8801_get_register(chip->client, OL_TID_OFFSET, &new_tid);
		if (error)
			tof8801_app0_report_error(chip, ERR_COMM, DEV_OK);
		usleep_range(1000, 1050);
	} while ((app0->ctrl_frame.buf[OL_TID_OFFSET] == new_tid) && (tries < num_tries));
	if (tries >= num_tries) {
		tof8801_errmsg("Error waiting on TID");
		tof8801_app0_report_error(chip, ERR_RETRIES, ERR_TID);
		return tries;
	}
	/* keep newest TID */
	app0->ctrl_frame.buf[OL_TID_OFFSET] = new_tid;
	return 0;
}

void tof8801_app0_default_cap_settings(struct tof8801_app0_application *app0_app)
{
	memset(&app0_app->cap_settings, 0, sizeof(app0_app->cap_settings));
	app0_app->cap_settings.cmd = 0;
	app0_app->cap_settings.delay = TOF8801_APP0_DELAY_DEFAULT;
	app0_app->cap_settings.period = TOF8801_APP0_PERIOD_DEFAULT;
	app0_app->cap_settings.noise_thrshld = TOF8801_APP0_NOISE_THRSHLD_DEFAULT;
	app0_app->cap_settings.iterations[0] = TOF8801_APP0_ITERATIONS_DEFAULT & 0xFF;
	app0_app->cap_settings.iterations[1] = (TOF8801_APP0_ITERATIONS_DEFAULT >> 8) & 0xFF;
	if (tof8801_app0_is_v2(container_of(app0_app, struct tof_sensor_chip, app0_app))) {
		app0_app->cap_settings.v2.alg = TOF8801_APP0_ALG_DEFAULT;
		app0_app->cap_settings.v2.gpio = TOF8801_APP0_GPIO_DEFAULT;
		app0_app->cap_settings.v2.data.data = 0;
	}
}

static int tof8801_app0_read_data_set_cmd(struct tof_sensor_chip *chip,
	char *wbuf, char size, char cmd)
{
	int error;
	/* Set read buffer command */
	error = tof8801_set_register(chip->client, OL_COMMAND_OFFSET, cmd);
	if (error) {
		tof8801_errmsg("Error writing cmd \'%x\': %d", cmd, error);
		return error;
	}
	error = app0_wait_for_tid_change(chip, TOF8801_MAX_WAIT_RETRY);
	if (error) {
		tof8801_errmsg("Error waiting on TID for cmd \'%x\': %d", cmd, error);
		return error;
	}
	/* read buffer */
	error = tof_i2c_read(chip->client, TOF8801_APP0_DATA_START, wbuf, size);
	if (error) {
		tof8801_errmsg("Error reading data set buffer: %d", error);
		return error;
	}
	return error;
}

int tof8801_app0_perform_factory_calibration(void *tof_chip)
{
	int error;
	struct tof_sensor_chip *chip = (struct tof_sensor_chip *)tof_chip;
	char fac_calib_cmd = OL_COMMAND_FACTORY_CALIB;

	chip->app0_app.cap_settings.cmd = fac_calib_cmd;
	if (!tof8801_app0_is_v2(chip))
		return 0;
	MEASURE_LOCK(chip->app0_app.measure_in_prog);

	error = tof8801_set_register(chip->client, OL_COMMAND_OFFSET, fac_calib_cmd);
	chip->app0_app.cal_update.dataFactoryConfig = 0;
	/* Start the measure timer */
	if ((error == 0) && (chip->app0_app.cap_settings.period == 0) &&
		(chip->app0_app.diag_state_mask == 0))
		error = mod_timer(&chip->meas_timer, jiffies +
			msecs_to_jiffies(MEASURE_TIMEOUT_MSEC * 4));
	return error;
}

int tof8801_app0_write_calibration(void *tof_chip)
{
	char calib_buf[0xFF] = {0};
	char calib_len;
	int error = 0;
	struct tof_sensor_chip *chip = (struct tof_sensor_chip *)tof_chip;
	tof_core_data_t *data_0 = &chip->app0_app.cap_settings.v2.data;
	tof_core_data_t *update = &chip->app0_app.cal_update;
	if (!tof8801_app0_is_v2(chip))
		return 0;
	tof8801_dbgmsg("Start calibration upload");
	data_0->dataFactoryConfig = (update->dataFactoryConfig && (chip->ext_calib_data.size != 0));
	/* Algorithm State Data is only valid if there is Factory Calibration data */
	data_0->dataAlgorithmState =
		(data_0->dataFactoryConfig && update->dataAlgorithmState && (chip->alg_info.size != 0));
	data_0->dataConfiguration = (update->dataConfiguration && (chip->config_data.size != 0));
	calib_len = 0;
	if (data_0->dataFactoryConfig) {
		tof8801_infomsg( "Factory Calibration data added");
		memcpy(calib_buf + calib_len, (void *)&chip->ext_calib_data.fac_data,
			chip->ext_calib_data.size);
		calib_len += chip->ext_calib_data.size;
	}
	if (data_0->dataAlgorithmState) {
		tof8801_infomsg( "Algorithm state data added");
		/* zero out alg state except for first 3 Bytes */
		memset(&chip->alg_info.alg_data.data[3], 0, sizeof(chip->alg_info.alg_data) - 3);
		memcpy(calib_buf + calib_len, chip->alg_info.alg_data.data, chip->alg_info.size);
		calib_len += chip->alg_info.size;
	}
	if (data_0->dataConfiguration) {
		tof8801_infomsg( "Configuration data added");
		memcpy(calib_buf + calib_len, (void *)&chip->config_data.cfg_data, chip->config_data.size);
		calib_len += chip->config_data.size;
	}

	if (calib_len == 0) {
		tof8801_infomsg( "No calibration data to upload.");
	} else {
		tof8801_infomsg( "Calibration data uploaded.");
		error = tof_i2c_write(chip->client, OL_FACTORY_CALIB_OFFSET, calib_buf, calib_len);
		if (error) {
			tof8801_errmsg("Error writing calibration data: %d", error);
			return 0;
		}
		/* clear update flags for calibration data */
		update->dataFactoryConfig = 0;
		update->dataAlgorithmState = 0;
		update->dataConfiguration = 0;
	}
	return calib_len;
}

int tof8801_app0_get_version(void *tof_chip, char *str, unsigned int strlen)
{
	struct tof_sensor_chip *chip = (struct tof_sensor_chip *)tof_chip;
	int len = 0;
	if (str != NULL)
		len = scnprintf(str, strlen, "%#x-%#x-%#x-%#x", chip->app0_app.version.maj_rev,
			chip->app0_app.version.min_rev, chip->app0_app.version.patch_rev,
			BYTES_TO_16B(chip->app0_app.version.build_0_rev, chip->app0_app.version.build_1_rev));
	if (chip->driver_debug)
		tof8801_infomsg( "App0 ver_str: \'%s\'", str);
	return len;
}

int tof8801_app0_read_version(struct tof_sensor_chip *chip)
{
	int error;
	chip->app0_app.version.maj_rev = chip->info_rec.record.app_ver;
	error = tof8801_get_register(chip->client, OL_APPREV_MINOR_OFFSET,
		&chip->app0_app.version.min_rev);
	if (error)
		return -EIO;
	error = tof8801_get_register(chip->client, OL_APPREV_PATCH_OFFSET,
		&chip->app0_app.version.patch_rev);
	if (error)
		return -EIO;
	if (tof8801_app0_is_v2(chip)) {
		error = tof8801_get_register(chip->client, OL_APPREV_BUILD_0_OFFSET,
			&chip->app0_app.version.build_0_rev);
		if (error)
			return -EIO;
		error = tof8801_get_register(chip->client, OL_APPREV_BUILD_1_OFFSET,
			&chip->app0_app.version.build_1_rev);
		if (error)
			return -EIO;
	}
	tof8801_infomsg( "App0 version: %d-%d-%d-%d",
		chip->app0_app.version.maj_rev,
		chip->app0_app.version.min_rev, chip->app0_app.version.patch_rev,
		BYTES_TO_16B(chip->app0_app.version.build_0_rev,
			chip->app0_app.version.build_1_rev));
	return 0;
}

void tof8801_app0_init_app(struct tof8801_app0_application *app0)
{
	int iterations = 0;
	struct tof_sensor_chip *chip = container_of(app0, struct tof_sensor_chip, app0_app);
	union tof8801_app0_capture cap_temp;
	MEASURE_UNLOCK(chip->app0_app.measure_in_prog);
	chip->app0_app.cap_settings.cmd = 0;
	memcpy(&cap_temp, &app0->cap_settings, sizeof(cap_temp));
	/* Wipe all local settings for app0, (except app_id) */
	memset(app0, 0, sizeof(struct tof8801_app0_application));
	app0->app_id = TOF8801_APP_ID_APP0;
	memcpy(&app0->cap_settings, &cap_temp, sizeof(app0->cap_settings));
	TMF8801_cr_init(&app0->clk_cr);
	(void)tof8801_app0_read_version(chip);
	app0->cal_update.dataFactoryConfig = 1;
	app0->cal_update.dataAlgorithmState = 1;
	app0->cal_update.dataConfiguration = 1;
	iterations = 1000 * le16_to_cpup((const __le16 *)chip->app0_app.cap_settings.iterations);
	/* we need to appropriately change the clock iteration counter */
	/* when the capture iterations are changed to keep the time acceptable */
	tof8801_app0_set_clk_iterations(chip, iterations);
	app0->clk_trim_enable = 1; /* clock trim default ENABLED */
	if (app0->clk_trim_enable && (chip->saved_clk_trim != UNINITIALIZED_CLK_TRIM_VAL))
		(void)tof8801_app0_rw_osc_trim(chip, &chip->saved_clk_trim, 1); /* use cached trim value */
	tof8801_persistence_filter_initialize(&chip->app0_app.persistence_filter, 24, 8);
}

int tof8801_app0_capture(void *tof_chip, int capture)
{
	struct tof_sensor_chip *chip = (struct tof_sensor_chip *)tof_chip;
	int error;
	if (capture) {
		MEASURE_LOCK(chip->app0_app.measure_in_prog);

		(void)tof8801_app0_write_calibration(chip);
		chip->app0_app.cap_settings.cmd = tof8801_app0_is_v2(chip) ?
			OL_COMMAND_START_EXT_CAL : OL_COMMAND_START;
		error = tof_i2c_write(chip->client, TOF8801_APP0_CMD_START,
			chip->app0_app.cap_settings.buf,
			sizeof(chip->app0_app.cap_settings.buf));
		ktime_get_real_ts64(&chip->app0_app.save_ts);
		/* If this is a single-shot capture, start the measure timer */
		if ((error == 0) && (chip->app0_app.cap_settings.period == 0) &&
			(chip->app0_app.diag_state_mask == 0))
			error = mod_timer(&chip->meas_timer, jiffies + msecs_to_jiffies(MEASURE_TIMEOUT_MSEC));
	} else {
		/* stop measurement */
		error = tof8801_set_register(chip->client, OL_COMMAND_OFFSET, OL_COMMAND_STOP);
		if (error) {
			MEASURE_UNLOCK(chip->app0_app.measure_in_prog);
			tof8801_errmsg("Error sending STOP cmd");
			return error;
		}
		/* clear pending interrupts */
		error = tof8801_set_register(chip->client, TOF8801_INT_STAT, 0);
		if (error) {
			MEASURE_UNLOCK(chip->app0_app.measure_in_prog);
			tof8801_errmsg("Error clearing INT_STAT");
			return error;
		}
		error = tof8801_app0_wait_for_idle(chip, TOF8801_APP0_IDLE_WAIT_TIMEOUT);
		if (error) {
			tof8801_errmsg("Error waiting for IDLE state");
			goto stop_err;
		}
		/* Read out APP0 header info: status, last cmd, TID, register contents, etc */
		error = tof_i2c_read(chip->client, OL_PREVIOUS_OFFSET,
			&chip->app0_app.ctrl_frame.buf[OL_PREVIOUS_OFFSET],
			(sizeof(struct tof8801_app0_control_reg_frame) - OL_PREVIOUS_OFFSET));
		if (error) {
			tof8801_errmsg("Error reading ctrl_reg");
			tof8801_app0_report_error(chip, ERR_COMM, DEV_OK);
			goto stop_err;
		}
		/* keep current correction, but let clock correction re-settle next capture period */
		TMF8801_cr_recalc(&chip->app0_app.clk_cr);
stop_err:
		MEASURE_UNLOCK(chip->app0_app.measure_in_prog);
		chip->app0_app.cap_settings.cmd = 0;
	}
	return error;
}

void tof8801_app0_measure_timer_expiry_callback1(unsigned long data)
{
	/* We have received a timeout event waiting on a response from ToF chip */
	struct tof_sensor_chip *chip = (struct tof_sensor_chip *)data;
	MEASURE_UNLOCK(chip->app0_app.measure_in_prog);
	tof8801_app0_report_error(chip, DRV_OK, ERR_MEASURE_TIMEOUT);
}

void tof8801_app0_measure_timer_expiry_callback(struct timer_list *t)
{
	struct tof_sensor_chip *chip = from_timer(chip, t, meas_timer);
	/* We have received a timeout event waiting on a response from ToF chip */
	MEASURE_UNLOCK(chip->app0_app.measure_in_prog);
	tof8801_app0_report_error(chip, DRV_OK, ERR_MEASURE_TIMEOUT);
}

int tof8801_app0_switch_to_bootloader(void *tof_chip)
{
	int error;
	struct tof_sensor_chip *chip = (struct tof_sensor_chip *)tof_chip;
	error = tof8801_set_register(chip->client, TOF8801_REQ_APP_ID, TOF8801_APP_ID_BOOTLOADER);
	if (error) {
		tof8801_errmsg("Error setting REQ_APP_ID register.\n");
		return error;
	}
	error = tof_wait_for_cpu_ready_timeout(chip->client, 10000);
	if (error)
		return error;
	tof8801_app0_init_app(&chip->app0_app);
	error = tof_init_info_record(chip);
	return (chip->info_rec.record.app_id) == TOF8801_APP_ID_BOOTLOADER ? 0 : -1;
}

char *tof8801_app0_get_results_rcv_buf(
	struct tof8801_app0_application *app0, char frame_size)
{
	struct tof8801_app0_drv_frame_header *header =
		&app0->algo_results_frame.results_frame.header;
	header->id = ID_RESULTS;
	header->size_lsb = LSB(frame_size);
	header->size_msb = SHORT_MSB(frame_size);
	return &app0->algo_results_frame.buf[DRV_FRAME_DATA];
}

int tof8801_app0_pack_result_v2_to_int(struct tof_sensor_chip *chip, char result_idx)
{
	tof_core2_result_t *results =
		&chip->app0_app.algo_results_frame.results_frame.results_v2;
	union result_t {
		struct {
			char byte_0;
			char byte_1;
			char byte_2;
			char byte_3;
		} PACKED;
		int int_result;
	} PACKED;
	union result_t result = { { 0 } };
	result.byte_0 = results->data.distPeak0;
	result.byte_1 = results->data.distPeak1;
	result.byte_2 = results->data.resultInfo.data;
	result.byte_3 = results->data.resultNum;
	return result.int_result;
}

int tof8801_app0_pack_result_to_int(struct tof_sensor_chip *chip, char result_idx)
{
	struct tof8801_app0_algo_results *results =
		&chip->app0_app.algo_results_frame.results_frame.results;
	char num_results = chip->app0_app.ctrl_frame.buf[OL_TARGETS_OFFSET];
	union result_t {
		struct {
			char byte_0;
			char byte_1;
			char byte_2;
			char byte_3;
		} PACKED;
		int int_result;
	} PACKED;
	union result_t result = { { 0 } };
	if ((num_results == 0) || (result_idx > (num_results - 1)))
		return 0;
	result.byte_3 = results->result_num;
	result.byte_2 = results->results[(unsigned int)result_idx].result_info;
	result.byte_1 = results->results[(unsigned int)result_idx].dis_peak_msb;
	result.byte_0 = results->results[(unsigned int)result_idx].dis_peak_lsb;
	return result.int_result;
}

int tof8801_app0_get_v1_results(struct tof_sensor_chip *chip)
{
	char num_results = chip->app0_app.ctrl_frame.buf[OL_TARGETS_OFFSET];
	char results_size = APP0_ALGO_RESULTS_RSP_SIZE(num_results);
	char *rbuf = tof8801_app0_get_results_rcv_buf(&chip->app0_app, results_size);
	int error = 0;
	if (num_results > 0)
		error = tof_i2c_read(chip->client, OL_RESULT_NUMBER_OFFSET, rbuf, results_size);
	return error;
}

int tof8801_app0_clock_skew_correct_results(struct tof_sensor_chip *chip)
{
	int error;
	int cr_dist;
	struct timespec64 end_ts = { 0 };
	static struct timespec64 min_ts = { 0 };
	int diff;
	u64 usec_epoch;
	tof_core2_result_t *result = &chip->app0_app.algo_results_frame.results_frame.results_v2;
	u32 tof_clk_cnt = result->data.sysClock;
	ktime_get_real_ts64(&end_ts);
	usec_epoch = (end_ts.tv_sec * 1000000) + ((end_ts.tv_nsec + 500) / 1000);
	if (timespec64_sub(end_ts, min_ts).tv_sec >= 60) {
		TMF8801_cr_recalc(&chip->app0_app.clk_cr);
		min_ts = end_ts;
	}
	TMF8801_cr_addpair(&chip->app0_app.clk_cr, (u32)usec_epoch, tof_clk_cnt);
	error = tof8801_app0_check_osc(chip);
	cr_dist = TMF8801_cr_map(&chip->app0_app.clk_cr, result->data.distPeak);
	if (chip->driver_debug)
		tof8801_infomsg(
			"clk_skew: host_ts: %u (us) dev_ts: %u (5MHz cnt) "
			"old_dist: %u new_dist: %u confidence: %d\n",
			(u32)usec_epoch, tof_clk_cnt, result->data.distPeak,
			cr_dist, result->data.resultInfo.reliabilityE);

	/* Special Situation: when driver receive this special num, it means dis is less than 10cm */

	chip->distance = cr_dist;
	chip->confidence = result->data.resultInfo.reliabilityE;

	result->data.distPeak = cr_dist;

	if (chip->driver_debug)
		tof8801_infomsg(
				"before clk_skew: host__ts: %u (us) conf: %d "
				"old__dist: %u new__dist: %u\n", (u32)usec_epoch, result->data.resultInfo.reliabilityE,
				chip->app0_app.distPeak_previous, result->data.distPeak);

	/* apply filter */
	if ((result->data.resultInfo.reliabilityE != 10) && (result->data.resultInfo.reliabilityE < 20) &&
		(result->data.resultInfo.reliabilityE > 0)) {
		/* 10% tolerance */
		if (result->data.distPeak > chip->app0_app.distPeak_previous)
			diff = (result->data.distPeak - chip->app0_app.distPeak_previous) * 10;
		else
			diff = (chip->app0_app.distPeak_previous - result->data.distPeak) * 10;
		/* out of tolerance range */
		if (diff > chip->app0_app.distPeak_previous)
			/* set cr_dist to remove detection, and wait for next measurement */
			cr_dist = TMF8801_MAX_DIST_MM + 1;
	}
	if (cr_dist > TMF8801_MAX_DIST_MM) {
		/* if corrected distance is out of range, remove the object detection */
		result->data.distPeak = 0;
		result->data.resultInfo.reliabilityE = 0;
	}

	chip->app0_app.distPeak_previous = result->data.distPeak;
	chip->app0_app.reliabilityE_previous = result->data.resultInfo.reliabilityE;
	if (chip->driver_debug)
		tof8801_infomsg(
				"after clk_skew: host__ts: %u (us) dev__ts: %u (5MHz cnt) "
				"old__dist: %u new__dist: %u\n", (u32)usec_epoch, tof_clk_cnt,
				chip->app0_app.distPeak_previous, result->data.distPeak);

	/* Save latest timestamp for periodic capture mode */
	memcpy(&chip->app0_app.save_ts, &end_ts, sizeof(chip->app0_app.save_ts));
	return error;
}

int tof8801_app0_get_v2_results(struct tof_sensor_chip *chip)
{
	char results_size;
	struct tof8801_app0_drv_frame_header *header =
		&chip->app0_app.algo_results_frame.results_frame.header;
	tof_core2_result_t *result =
		&chip->app0_app.algo_results_frame.results_frame.results_v2;
	int error;
	header->id = ID_RESULTS_V2;
	error = tof_i2c_read(chip->client, OL_STATUS_OFFSET, (char *)result,
		sizeof(tof_core2_result_status_regs_t) + offsetof(tof_core2_result_data_t, addTargets));
	if (error == 0) {
		chip->xtalk_peak = ((result->data.stateData.data[8] << 8) | result->data.stateData.data[9]);
		if (chip->driver_debug)
			tof8801_infomsg( "xtalk peak: %u\n", chip->xtalk_peak);
	}
	/* RESULTS_V2 contains clock skew info to correct results, though */
	/* it is required that the host time the measure to apply correction */
	if (error == 0)
		error = tof8801_app0_clock_skew_correct_results(chip);
	if (error == 0) {  
		/* Filter non-persistent objects. */
		tof8801_persistence_filter_update(&chip->app0_app.persistence_filter, result);
	}
	/*
	 * Only read out first object + add'l data right now. v2_results response
	 * size is fixed for one object. This will need to be updated if we
	 * report multi object through this method
	 */
	results_size = APP0_ALGO_RESULTS_V2_RSP_SIZE;
	header->size_lsb = LSB(results_size);
	header->size_msb = SHORT_MSB(results_size);
	if (result->data.numAddTargets > 0) {
		/* not supported reading multi-target yet */
	}
	/* Save cache data */
	memcpy(chip->alg_info.alg_data.data, &result->data.stateData.data, OL_RES2_STATE_DATA_SIZE);
	chip->alg_info.size = OL_RES2_STATE_DATA_SIZE;
	/* temperature is stored in last slot of state data */
	chip->app0_app.last_known_temp = chip->alg_info.alg_data.data[chip->alg_info.size - 1];
	return error;
}

int tof8801_app0_read_results(struct tof_sensor_chip *chip)
{
	char results_avail = chip->app0_app.ctrl_frame.buf[OL_REGISTER_CONTENTS_OFFSET];
	int error;
	int first_obj_dis;
	uint32_t distance;
	uint32_t confidence;
	uint32_t status = 0;
	uint8_t NumberOfObjects = 1;
	unsigned int iterations = 0;

	memset(&chip->app0_app.algo_results_frame.results_frame.results, 0,
		sizeof(chip->app0_app.algo_results_frame.results_frame.results));
	if (results_avail == OL_COMMAND_RD_RESULT) {
		error = tof8801_app0_get_v1_results(chip);
		first_obj_dis = tof8801_app0_pack_result_to_int(chip, 0);
	} else {
		error = tof8801_app0_get_v2_results(chip);
		first_obj_dis = tof8801_app0_pack_result_v2_to_int(chip, 0);
	}
	if (error) {
		tof8801_errmsg("IRQ: Error reading result from device");
		tof8801_app0_report_error(chip, ERR_COMM,
			chip->app0_app.ctrl_frame.buf[OL_STATUS_OFFSET]);
	}

	distance = (first_obj_dis & 0x0000FFFF) >> 0;
	confidence = (first_obj_dis & 0x003F0000) >> 16;

	if (distance < DISTANCE_THRESHOLD && confidence >= CONFIDENCE_THRESHOLD_LOW) {
		status = 0; /* status 0: data is confident */
	} else if (distance == DISTANCE_THRESHOLD_INFINITY &&
		confidence == CONFIDENCE_THRESHOLD_INFINITY) {
		status = 255;
		NumberOfObjects = 0;
	} else if (distance >= DISTANCE_THRESHOLD) {
		if (confidence >= CONFIDENCE_THRESHOLD_HIGH) {
			status = 0;
		} else if (confidence >= CONFIDENCE_THRESHOLD_MID) {
			status = 6; /* status 6: data is semiconfident */
		}

		/* Special Situation: when driver receive this special num,
		it means that dis is less than 10cm, then raw report 3cm to algo */
		if (distance == NEAREST_DISTANCE_STATUS) {
			distance = 30;
			status = 21; /* 21 is special status only for AMS to solve 3~10cm situation */
			tof8801_infomsg( "10cm occured! distance(%u), confidence(%u), status(%u)",
				distance, confidence, status);
		}
	} else {
		status = 7; /* status 7: data is not confident */
	}

	if (chip->driver_debug)
		tof8801_infomsg( "distance(%u), confidence(%u), status(%u)",
			distance, confidence, status);

	/* send result data to fifo */
	error = tof_queue_frame(chip, chip->app0_app.algo_results_frame.buf,
		APP0_DRV_FRAME_SIZE(chip->app0_app.algo_results_frame.buf));
	if (error)
		tof8801_errmsg("IRQ: Error adding result frame to fifo");

	return error;
}

static int app0_get_histogram(struct tof_sensor_chip *chip, int size)
{
	struct tof8801_app0_application *app0 = &chip->app0_app;
	char *rbuf = app0->hist_frame.hist.buf;
	uint32_t num_chunks = APP0_HIST_SIZE_TO_NUM_CHUNKS(size);
	int error;
	int i;
	/* Read histogram in quartets */
	for (i = 0; i < num_chunks; i++) {
		error = tof_i2c_read(chip->client, TOF8801_APP0_DATA_START,
			&rbuf[i * TOF8801_APP0_HIST_CHUNK_SIZE],
			TOF8801_APP0_HIST_CHUNK_SIZE);
		if (error) {
			tof8801_app0_report_error(chip, ERR_COMM, DEV_OK);
			return error;
		}
	}
	return 0;
}

static int app0_get_histograms(struct tof_sensor_chip *chip,
	tof_core1_diag_info_t diag_info)
{
	struct tof8801_app0_application *app0 = &chip->app0_app;
	int tdc = diag_info.tdcAvail;
	int error;
	int hist_size = APP0_DIAG_SIZE_TO_HIST_SIZE(diag_info.size);
	int id = APP0_DIAG_STATE_TO_HIST_ID(diag_info.diagNum);
	app0->hist_frame.hist.header.size_lsb = LSB(hist_size);
	app0->hist_frame.hist.header.size_msb = SHORT_MSB(hist_size);
	if (tdc == tdcAvailAll5) {
		for (tdc = 0; tdc < TOF8801_APP0_NUM_TDC; tdc += 1) {
			app0->hist_frame.hist.header.id = id + tdc;
			error = app0_get_histogram(chip, hist_size);
			if (error)
				return error;
			/* place one full histogram frame into fifo */
			(void)tof_queue_frame(chip, app0->hist_frame.buf,
				APP0_DRV_FRAME_SIZE(app0->hist_frame.buf));
		}
	} else {
		if (tdc == tdcAvailSum)
			app0->hist_frame.hist.header.id = id;
		else
			app0->hist_frame.hist.header.id = id + (tdc - 1);
		error = app0_get_histogram(chip, hist_size);
		if (error)
			return error;
		/* place one full histogram frame into fifo */
		(void)tof_queue_frame(chip, app0->hist_frame.buf,
			APP0_DRV_FRAME_SIZE(app0->hist_frame.buf));
	}
	return 0;
}

static int tof8801_app0_stop_measurements(struct tof_sensor_chip *chip)
{
	char period = 0;
	switch (chip->app0_app.cap_settings.cmd) {
	case OL_COMMAND_START:
		period = chip->app0_app.cap_settings.v1.period;
		break;
	case OL_COMMAND_START_EXT:
	case OL_COMMAND_START_EXT_CAL:
		period = chip->app0_app.cap_settings.v2.period;
		break;
	default:
		break;
	}
	return (chip->app0_app.cap_settings.cmd == 0) || (period == 0);
}

int tof8801_app0_get_histograms(struct tof_sensor_chip *chip,
	tof_core1_diag_info_t diag_info)
{
	int error;
	if (diag_info.tdcAvail == tdcAvailNone)
		return 0;
	/* Send 'READ HISTOGRAMS' CMD */
	error = tof8801_set_register(chip->client, OL_COMMAND_OFFSET, OL_COMMAND_RD_HISTO);
	if (error)
		tof8801_app0_report_error(chip, ERR_COMM, DEV_OK);
	/* app0 sometimes needs a moment to get histograms ready */
	usleep_range(1000, 1050);
	error = app0_wait_for_tid_change(chip, TOF8801_MAX_WAIT_RETRY);
	if (error)
		return error;
	error = app0_get_histograms(chip, diag_info);
	return error;
}

static void tof8801_app0_read_fac_calib(struct tof_sensor_chip *chip)
{
	uint8_t *buf = &chip->ext_calib_data.fac_data.data[0];
	int error = tof_i2c_read(chip->client, OL_FACTORY_CALIB_OFFSET,
		(char *)&chip->ext_calib_data.fac_data,
		sizeof(chip->ext_calib_data.fac_data));
	tof8801_errmsg("enter tof8801_app0_read_fac_calib");
	if (!error)
		chip->ext_calib_data.size = sizeof(chip->ext_calib_data.fac_data);
	else
		chip->ext_calib_data.size = 0;
	tof8801_infomsg( "tof8801_app0_read_fac_calib from Reg, \
			size: %u\n",
		chip->ext_calib_data.size);
	tof8801_infomsg( "%x %x %x %x %x %x %x %x %x %x %x %x %x %x\n",
		buf[0], buf[1], buf[2], buf[3], buf[4],
		buf[5], buf[6], buf[7], buf[8], buf[9], buf[10], buf[11], buf[12], buf[13]);
	MEASURE_UNLOCK(chip->app0_app.measure_in_prog);
	del_timer_sync(&chip->meas_timer);
	/* Set update flag for factory config */
	chip->app0_app.cal_update.dataFactoryConfig = 1;
	tof8801_errmsg("chip->ext_calib_data.size is (%d), \
			chip->app0_app.cal_update.dataFactoryConfig is (%d)",
		chip->ext_calib_data.size, chip->app0_app.cal_update.dataFactoryConfig);
	return;
}

void tof8801_app0_process_irq(void *tof_chip, char int_stat)
{
	int error;
	struct tof_sensor_chip *chip = (struct tof_sensor_chip *)tof_chip;
	struct tof8801_app0_application *app0 = &chip->app0_app;
	tof_core1_diag_info_t diag_info;
	/* Read out APP0 header info: status, last cmd, TID, register contents, etc */
	error = tof_i2c_read(chip->client, OL_PREVIOUS_OFFSET,
		&app0->ctrl_frame.buf[OL_PREVIOUS_OFFSET],
		(sizeof(struct tof8801_app0_control_reg_frame) - OL_PREVIOUS_OFFSET));
	if (error) {
		tof8801_errmsg("IRQ: Error reading ctrl_reg");
		tof8801_app0_report_error(chip, ERR_COMM, DEV_OK);
		/* Nothing else to do if we cant read the control registers... */
		return;
	}
	if (int_stat & IRQ_RESULTS) {
		if (chip->driver_debug)
			tof8801_infomsg( "IRQ: Received RESULTS interrupt");
		if (app0->ctrl_frame.buf[OL_REGISTER_CONTENTS_OFFSET] == OL_COMMAND_FACTORY_CALIB) {
			tof8801_app0_read_fac_calib(chip); /* read out fac calib data, 14 Bytes */
			tof8801_errmsg("tof8801_app0_process_irq, IRQ: OL_COMMAND_FACTORY_CALIB");
			return;
		} else {
			tof8801_app0_read_results(chip); /* read out measure result data  4 Bytes */
			if (tof8801_app0_stop_measurements(chip)) {
				MEASURE_UNLOCK(app0->measure_in_prog);
				del_timer_sync(&chip->meas_timer);
				chip->app0_app.cap_settings.cmd = 0;
			}
		}
		return;
	}
	if (int_stat & IRQ_DIAG) {
		/* if we are trying to STOP, exit early */
		if (chip->app0_app.ctrl_frame.buf[OL_PREVIOUS_OFFSET] == OL_COMMAND_STOP)
			return;
		/* Look at OL_DIAG_INFO to see which histograms are available */
		diag_info.data_0 = app0->ctrl_frame.buf[OL_DIAG_INFO_0_OFFSET];
		diag_info.data_1 = app0->ctrl_frame.buf[OL_DIAG_INFO_1_OFFSET];
		if (chip->driver_debug) {
			tof8801_infomsg( "IRQ: Received DIAG interrupt");
			tof8801_infomsg( "IRQ: STATE: %#x diagNum: %#hhx diagHistSize: %#hhx tdcAvail: %#hhx",
				chip->app0_app.ctrl_frame.buf[OL_STATE_OFFSET], diag_info.diagNum,
				diag_info.size, diag_info.tdcAvail);
		}
		if ((chip->app0_app.diag_state_mask & (1 << diag_info.diagNum)) == 0)
			tof8801_app0_report_error(chip, DEV_OK, ERR_DIAG_INFO);
		(void)tof8801_app0_get_histograms(chip, diag_info);
		tof8801_set_register(chip->client, OL_COMMAND_OFFSET, OL_COMMAND_CONTINUE);
	}
}

static int tof8801_app0_issue_gen_cmd(unsigned char cmd, struct tof_sensor_chip *chip)
{
	int error;
	switch (cmd) {
	case OL_COMMAND_SET_DIAG_MASK:
		chip->app0_app.diag_state_mask =
			(chip->app0_app.user_cmd.diag_mask_cmd.diag_0 |
			(chip->app0_app.user_cmd.diag_mask_cmd.diag_1 << 8) |
			(chip->app0_app.user_cmd.diag_mask_cmd.diag_2 << 16) |
			(chip->app0_app.user_cmd.diag_mask_cmd.diag_3 << 24));
		break;
	case OL_COMMAND_ENTER_BOOTLOADER:
		MEASURE_UNLOCK(chip->app0_app.measure_in_prog);
		return tof8801_app0_switch_to_bootloader(chip);
	default:
		break;
	}
	error = tof8801_app0_start_measure_timer(chip, cmd);
	if (error) {
		tof8801_errmsg("Error starting measure timer: %d", error);
		return error;
	}
	error = tof_i2c_write(chip->client, TOF8801_APP0_CMD_START,
		chip->app0_app.user_cmd.anon_cmd.buf,
		TOF8801_APP0_MAX_CMD_SIZE);
	return error;
}

static int tof8801_app0_start_measure_timer(
	struct tof_sensor_chip *chip, unsigned char cmd)
{
	int error = 0;
	/* Sync measure command with capture settings */
	if (TOF8801_APP0_IS_MEAS_CMD(cmd))
		memcpy(&chip->app0_app.cap_settings,
			((union tof8801_app0_capture *)(chip->app0_app.user_cmd.anon_cmd.buf)),
			sizeof(chip->app0_app.cap_settings));
	/* If taking a measure command, set a flag (not diagnostic) */
	if (TOF8801_APP0_IS_MEAS_CMD(cmd) && (chip->app0_app.diag_state_mask == 0)) {
		if (tof8801_app0_measure_in_progress(chip)) {
			error = -EIO;
		} else {
			/* start msec timeout timer */
			MEASURE_LOCK(chip->app0_app.measure_in_prog);
			error = mod_timer(&chip->meas_timer,
				jiffies + msecs_to_jiffies(MEASURE_TIMEOUT_MSEC));
		}
	}
	return error;
}

static int tof8801_app0_issue_force_cmd(
	unsigned char cmd, struct tof_sensor_chip *chip)
{
	int error;
	chip->app0_app.cap_settings.cmd = 0;
	MEASURE_UNLOCK(chip->app0_app.measure_in_prog);
	error = tof_i2c_write(chip->client, TOF8801_APP0_CMD_START,
		chip->app0_app.user_cmd.anon_cmd.buf,
		TOF8801_APP0_MAX_CMD_SIZE);
	return error;
}

int tof8801_app0_issue_cmd(void *tof_chip)
{
	struct tof_sensor_chip *chip = (struct tof_sensor_chip *)tof_chip;
	unsigned char command = chip->app0_app.user_cmd.anon_cmd.buf[TOF8801_APP0_CMD_IDX];
	int error;
	if (command < TOF8801_APP0_FORCE_COMMANDS) {
		error = tof8801_app0_issue_gen_cmd(command, chip);
	} else {
		/* The command is a force/preemptive command; STOP, RESET, etc. */
		error = tof8801_app0_issue_force_cmd(command, chip);
	}
	/* If not a measure command we can wait for Idle state */
	if ((error == 0) && !TOF8801_APP0_IS_MEAS_CMD(command))
		error = tof8801_app0_wait_for_idle(chip, TOF8801_APP0_IDLE_WAIT_TIMEOUT);
	return error;
}

int tof8801_app0_get_dataset(void *tof_chip, int dataset_type)
{
	struct tof_sensor_chip *chip = (struct tof_sensor_chip *)tof_chip;
	int error;
	char cmd = 0;
	char size = 0;
	switch (dataset_type) {
	case GEN_CFG_DATASET:
		cmd = OL_COMMAND_RD_GEN_CONFIG;
		size = APP0_GENERAL_CONFIG_RSP_SIZE;
		break;
	default:
		tof8801_errmsg("Error unsupported dataset type \'%d\'", dataset_type);
	}
	error = tof8801_get_register(chip->client, OL_TID_OFFSET,
		&chip->app0_app.ctrl_frame.buf[OL_TID_OFFSET]);
	if (error == 0)
		error = tof8801_app0_read_data_set_cmd(chip, chip->app0_app.dataset.buf, size, cmd);
	return error;
}
