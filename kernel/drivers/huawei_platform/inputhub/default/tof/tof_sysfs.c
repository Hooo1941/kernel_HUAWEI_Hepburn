

#include "tof_sysfs.h"

#include <linux/delay.h>
#include <linux/device.h>
#include <linux/fs.h>
#include <linux/time.h>
#include <linux/jiffies.h>
#include <linux/mtd/hisi_nve_interface.h>
#include <linux/of.h>
#include <linux/of_device.h>
#include <linux/of_gpio.h>
#include <linux/rtc.h>
#include <linux/slab.h>
#include <linux/types.h>
#include <platform_include/basicplatform/linux/fs/vendor_fs_interface.h>

#ifdef CONFIG_HUAWEI_DSM
#include <dsm/dsm_pub.h>
#endif
#include <securec.h>
#include "contexthub_route.h"

#define TOF_SHOW_BUF_SIZE 64
#define TOF_LOG_BUF_SIZE 256

static enum ret_type tof_calibration_res = RET_INIT;       /* tof calibrate result */
static unsigned long g_calib_cmd;
/* pass mark as NA */
static char *cali_error_code_str[] = {
	"NULL", "NA", "EXEC_FAIL", "NV_FAIL", "COMMU_FAIL", "RET_TYPE_MAX"
};

static inline void do_gettimeofday(struct timeval *tv)
{
	struct timespec64 now;

	ktime_get_real_ts64(&now);
	tv->tv_sec = now.tv_sec;
	tv->tv_usec = now.tv_nsec / 1000;
}

static void get_test_time(char *date_str, size_t size)
{
	struct timeval time;
	unsigned long local_time;
	struct rtc_time tm;

	do_gettimeofday(&time);
	/* 60 sec per minute */
	local_time = (u32)(time.tv_sec - (sys_tz.tz_minuteswest * 60));
	rtc_time64_to_tm(local_time, &tm);

	if (snprintf_s(date_str, CLI_TIME_STR_LEN, CLI_TIME_STR_LEN - 1,
		"%04d-%02d-%02d %02d:%02d:%02d",
		tm.tm_year + 1900, tm.tm_mon + 1, /* tm_year start from 1900 */
		tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec) < 0)
		return;
}

static char *get_cali_error_code(int error_code)
{
	if ((error_code > 0) && (error_code < RET_TYPE_MAX))
		return cali_error_code_str[error_code];
	return NULL;
}

static void save_to_file(const char *file_path, const char *content)
{
	static mm_segment_t old_fs;
	struct file *fp = NULL;
	struct file *pfile = NULL;
	int ret;

	old_fs = get_fs();
	set_fs(KERNEL_DS);

	/* mkdir tof data dir */
	ret = (int)vendor_access(TOF_DATA_DIR, 0);
	if (ret) {
		ret = (int)vendor_mkdir(TOF_DATA_DIR, 0770); /* 0770: dir rights */
		if (ret < 0) {
			hwlog_err("create failed, ret = %d\n", TOF_DATA_DIR, ret);
			set_fs(old_fs);
			return;
		}
		hwlog_info("create dir %s successed, %d\n", TOF_DATA_DIR, ret);

		/* chown group */
		pfile = filp_open(TOF_DATA_DIR, O_RDONLY, 0770); /* 0770: dir rights */
		if (IS_ERR_OR_NULL(pfile)) {
			hwlog_err("open failed\n", TOF_DATA_DIR);
			vendor_rmdir(TOF_DATA_DIR);
			set_fs(old_fs);
			return;
		}

		if (vfs_fchown(pfile, ROOT_UID, SYSTEM_GID)) {
			hwlog_err("chown failed\n");
			filp_close(pfile, NULL);
			vendor_rmdir(TOF_DATA_DIR);
			set_fs(old_fs);
			return;
		}
		filp_close(pfile, NULL);
	}

	/* write content */
	fp = filp_open(file_path, O_CREAT | O_WRONLY | O_APPEND, 0644); /* 0644: file rights */
	if (IS_ERR(fp)) {
		hwlog_err("open %s fail\n", file_path);
		set_fs(old_fs);
		return;
	}
	vfs_write(fp, content, strlen(content), &(fp->f_pos));
	filp_close(fp, NULL);

	set_fs(old_fs);
}

static void fill_content(char *content, union tof_vi5300_calibrate_data *calib_data, char *date_str)
{
	int ret;
	switch (g_calib_cmd) {
	case TOF_CALIBRATE_CMD_XTALK:
		ret = snprintf_s(content, CLI_CONTENT_LEN_MAX, TOF_LOG_BUF_SIZE, TOF_CALI_XTALK_TEST,
			calib_data->cali.xtalk_peak, calib_data->cali.xtalk_calib, calib_data->cali.xtalk_ratio,
			((tof_calibration_res == SUC) ? "pass" : "fail"), 0,
			get_cali_error_code(tof_calibration_res), date_str);
		if (ret < 0) {
			hwlog_err("[%s-%d]:snprintf_s fail, ret:0x%x\n", __func__, __LINE__, ret);
			return;
		}
		break;
	case TOF_CALIBRATE_CMD_OFFSET:
		ret = snprintf_s(content, CLI_CONTENT_LEN_MAX, TOF_LOG_BUF_SIZE, TOF_CALI_OFFSET_TEST,
			calib_data->cali.reftof, calib_data->cali.offset_calib,
			((tof_calibration_res == SUC) ? "pass" : "fail"), 0,
			get_cali_error_code(tof_calibration_res), date_str);
		if (ret < 0) {
			hwlog_err("[%s-%d]:snprintf_s fail, ret:0x%x\n", __func__, __LINE__, ret);
			return;
		}
		break;
	case TOF_CALIBRATE_CMD_NEAR:
		ret = snprintf_s(content, CLI_CONTENT_LEN_MAX, TOF_LOG_BUF_SIZE,
			TOF_CALI_NEAR_TEST, calib_data->cali.near_dist,
			((tof_calibration_res == SUC) ? "pass" : "fail"), 0,
			get_cali_error_code(tof_calibration_res), date_str);
		if (ret < 0) {
			hwlog_err("[%s-%d]:snprintf_s fail, ret:0x%x\n", __func__, __LINE__, ret);
			return;
		}
		break;
	case TOF_CALIBRATE_CMD_FAR:
		ret = snprintf_s(content, CLI_CONTENT_LEN_MAX, TOF_LOG_BUF_SIZE,
			TOF_CALI_FAR_TEST, calib_data->cali.far_dist,
			((tof_calibration_res == SUC) ? "pass" : "fail"), 0,
			get_cali_error_code(tof_calibration_res), date_str);
		if (ret < 0) {
			hwlog_err("[%s-%d]:snprintf_s fail, ret:0x%x\n", __func__, __LINE__, ret);
			return;
		}
		break;
	default:
		hwlog_err("invalid calib cmd %d, please check~\n", g_calib_cmd);
		break;
	}
}

static void tof_calibrate_save_log(union tof_vi5300_calibrate_data *calib_data)
{
	char content[CLI_CONTENT_LEN_MAX] = {0};
	char date_str[CLI_TIME_STR_LEN] = {0};

	get_test_time(date_str, sizeof(date_str));
	fill_content(content, calib_data, date_str);
	save_to_file(TOF_DATA_FILE, content);
}

static int tof_calibrate_save_ext(uint8_t tag, const void *buf, int length)
{
	union tof_vi5300_calibrate_data tmp_calibrate_data;
	union tof_vi5300_calibrate_data calibrate_data;
	union tof_vi5300_calibrate_data *data = &calibrate_data;
	const int *nv_calibrate_data = (const int *)user_info.nv_data;

	if (!buf || length <= 0 || length > TOFP_CALIDATA_NV_SIZE) {
		hwlog_err("[%s-%d]:invalid argument", __func__, __LINE__);
		tof_calibration_res = EXEC_FAIL;
		return -1;
	}
	if (memcpy_s(&tmp_calibrate_data.data, sizeof(tmp_calibrate_data),
		buf, length) != EOK) {
		hwlog_err("[%s-%d]:memcpy_s fail\n", __func__, __LINE__);
		return -1;
	}
	if (read_calibrate_data_from_nv(PS_CALIDATA_NV_NUM,
		sizeof(calibrate_data), "PSENSOR") < 0) {
		hwlog_err("[%s-%d]:read calibrate fail\n", __func__, __LINE__);
		return -1;
	}
	if (memcpy_s(&calibrate_data.data, sizeof(calibrate_data.data),
		nv_calibrate_data, sizeof(union tof_vi5300_calibrate_data)) != EOK) {
		hwlog_err("[%s-%d]:memcpy_s fail\n", __func__, __LINE__);
		return -1;
	}

	if (data && g_calib_cmd == TOF_CALIBRATE_CMD_XTALK) {
		data->cali.xtalk_peak = tmp_calibrate_data.cali.xtalk_peak;
		data->cali.xtalk_calib = tmp_calibrate_data.cali.xtalk_calib;
		data->cali.xtalk_ratio = tmp_calibrate_data.cali.xtalk_ratio;
	}
	if (data && g_calib_cmd == TOF_CALIBRATE_CMD_OFFSET) {
		data->cali.reftof = tmp_calibrate_data.cali.reftof;
		data->cali.offset_calib = tmp_calibrate_data.cali.offset_calib;
	}
	if (write_calibrate_data_to_nv(PS_CALIDATA_NV_NUM, sizeof(calibrate_data.data),
		"PSENSOR", (char *)calibrate_data.data) < 0) {
		hwlog_err("[%s-%d]:write_calibrate_data_to_nv fail\n", __func__, __LINE__);
		return -1;
	}

	tof_calibrate_save_log(&tmp_calibrate_data);
	return 0;
}

ssize_t attr_tof_calibrate_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	union tof_vi5300_calibrate_data tmp_calibrate_data;
	const int *calibrate_data = (const int *)user_info.nv_data;

	if (!buf) {
		hwlog_err("[%s-%d]:buf is null ptr\n", __func__, __LINE__);
		return -1;
	}

	if (read_calibrate_data_from_nv(PS_CALIDATA_NV_NUM,
		sizeof(tmp_calibrate_data), "PSENSOR") < 0) {
		hwlog_err("[%s-%d]:read calibrate fail\n", __func__, __LINE__);
		return -1;
	}

	if (memcpy_s(&tmp_calibrate_data.data, sizeof(tmp_calibrate_data.data),
		calibrate_data, sizeof(union tof_vi5300_calibrate_data)) != EOK) {
		hwlog_err("[%s-%d]:memcpy_s fail\n", __func__, __LINE__);
		return -1;
	}

	return snprintf_s(buf, PAGE_SIZE, TOF_SHOW_BUF_SIZE, "%d,%d,%d,%d,%d\n",
		tmp_calibrate_data.cali.xtalk_peak, tmp_calibrate_data.cali.xtalk_calib, tmp_calibrate_data.cali.xtalk_ratio,
		tmp_calibrate_data.cali.reftof, tmp_calibrate_data.cali.offset_calib);
}

static int send_tof_calibrate_cmd(unsigned long calibrate_cmd,
	uint8_t *tag_ptr, struct read_info *pkg_mcu_ptr)
{
	struct tof_device_info *chip_info = NULL;
	struct read_info tmp_pkg_mcu;

	if (!tag_ptr || !pkg_mcu_ptr) {
		hwlog_err("[%s-%d]:params is null ptr\n", __func__, __LINE__);
		return -1;
	}

	chip_info = tof_get_device_info(*tag_ptr);
	if (!chip_info || chip_info->tof_enable_flag == 0) {
		hwlog_err("[%s-%d]:is not valid tof device\n", __func__, __LINE__);
		return -1;
	}
	hwlog_info("[%s-%d]:calibrate cmd type:%d\n", __func__, __LINE__,
		calibrate_cmd);
	tmp_pkg_mcu = send_calibrate_cmd(*tag_ptr, calibrate_cmd, &tof_calibration_res);
	if (tof_calibration_res == COMMU_FAIL || tof_calibration_res == EXEC_FAIL ||
		tmp_pkg_mcu.errno != 0)
		return -1;

	if (memcpy_s(pkg_mcu_ptr, sizeof(tmp_pkg_mcu), &tmp_pkg_mcu,
		sizeof(struct read_info)) != EOK) {
		hwlog_err("[%s-%d]:memcpy_s fail\n", __func__, __LINE__);
		return -1;
	}

	return 0;
}

ssize_t attr_tof_calibrate_store(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t count)
{
	int ret;
	unsigned long calibrate_cmd = 0;
	union tof_vi5300_calibrate_data calib_data;
	struct read_info pkg_mcu;
	uint8_t tag = TAG_TOF;

	if (!buf) {
		hwlog_err("[%s-%d]:buf is null ptr\n", __func__, __LINE__);
		return -1;
	}
	if (kstrtoul(buf, TO_DECIMALISM, &calibrate_cmd))
		return -EINVAL;

	if (calibrate_cmd < TOF_CALIBRATE_CMD_XTALK || calibrate_cmd > TOF_CALIBRATE_CMD_FAR)
		return count;

	g_calib_cmd = calibrate_cmd;
	ret = send_tof_calibrate_cmd(calibrate_cmd, &tag, &pkg_mcu);
	if (ret < 0) {
		hwlog_err("[%s-%d]:send_tof_calibrate_cmd fail, ret:%d\n", __func__, __LINE__, ret);
		goto tof_calibrate_show_data;
	}

	tof_calibrate_save_ext(tag, pkg_mcu.data, pkg_mcu.data_length);
	hwlog_info("tof_calibrate success\n");
tof_calibrate_show_data:
	if (memcpy_s(&calib_data.data, sizeof(calib_data.data), pkg_mcu.data, pkg_mcu.data_length) != EOK) {
		hwlog_err("[%s-%d]:memcpy_s fail\n", __func__, __LINE__);
		return -1;
	}
	switch (g_calib_cmd) {
	case TOF_CALIBRATE_CMD_XTALK:
		hwlog_info("[%s-%d]:xtalk_peak:%d, xtalk_calib:%d, xtalk_ratio:%d\n",
			__func__, __LINE__, calib_data.cali.xtalk_peak, calib_data.cali.xtalk_calib, calib_data.cali.xtalk_ratio);
		break;
	case TOF_CALIBRATE_CMD_OFFSET:
		hwlog_info("[%s-%d]:offset_reftof:%d, offset_calib:%d\n",
		__func__, __LINE__, calib_data.cali.reftof, calib_data.cali.offset_calib);
		break;
	case TOF_CALIBRATE_CMD_NEAR:
		hwlog_info("[%s-%d]:near_distacnce:%d\n", __func__, __LINE__, calib_data.cali.near_dist);
		break;
	case TOF_CALIBRATE_CMD_FAR:
		hwlog_info("[%s-%d]:far_distance:%d\n", __func__, __LINE__, calib_data.cali.far_dist);
		break;
	default:
		hwlog_err("invalid calib cmd %d, please check~\n", g_calib_cmd);
		break;
	}
	return ret < 0 ? ret : count;
}
