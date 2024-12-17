
/*
 * hw_laser_dmd.c
 *
 * laser dmd driver source file
 *
 * Copyright (C) 2024-2024 Huawei Technology Co., Ltd.
 *
 * This program is free software; you may redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; version 2 of the License.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS
 * BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
 * ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 *
 */
#include <linux/init.h>
#include <linux/module.h>
#include <linux/device.h>
#include <linux/kernel.h>
#include <linux/kobject.h>
#include <linux/slab.h>
#include <log/hiview_hievent.h>
#include <securec.h>
#include "cam_log.h"
#include "hw_laser_dmd.h"

#define HWLOG_TAG laser_dsm
#define LASER_DSM_BUF_SIZE_1024 1024
#define LASER_DMD_INFO_LENGTH 50
#define PFX "[laser_dmd]"
#define log_inf(fmt, args...) pr_info(PFX "%s line:%d " fmt, __func__, __LINE__, ##args)
#define log_err(fmt, args...) pr_err(PFX "%s line:%d " fmt, __func__, __LINE__, ##args)

typedef struct errno_string_map
{
	int32_t error_no;
	char *name;
} errno_string_map;

typedef struct laser_probe_info {
	char* name;
	probe_state state;
} laser_probe_info;

static struct dsm_client *g_laser_dsm_client;

static struct laser_probe_info g_state_table[] = {
	[HWLASER_VL53L1] = { "vl53l1 ", PROBE_STATE_MAX },
	[HWLASER_VI5300] = { "vi5300 ", PROBE_STATE_MAX },
	[HWLASER_VL53L3] = { "vl53l3 ", PROBE_STATE_MAX },
};

void report_laser_probe_state(device_type dev_type, probe_state state)
{
	unsigned int i;
	bool all_probe_fail = true;

	if (dev_type < 0 || dev_type >= HWLASER_TYPE_MAX) {
		log_err("laser type err\n");
		return;
	}
	g_state_table[dev_type].state = state;

	for (i = 0; i < HWLASER_TYPE_MAX; ++ i) {
		if (g_state_table[i].state != PROBE_FAIL) {
			all_probe_fail = false;
			break;
		}
	}

	if (all_probe_fail)
		laser_dsm_report_dmd(HWLASER_TYPE_MAX, DSM_LASER_PROBE_ERROR_NO);

	return;
}

static int get_errno_str(int err_no, char info[])
{
	int i;
	int ret;
	struct errno_string_map err_to_str_map[] = {
		{ DSM_LASER_PROBE_ERROR_NO, "probe fail" },
		{ DSM_LASER_I2C_ERROR_NO, "i2c transfer fail" },
		{ DSM_LASER_FW_DOWNLOAD_ERROR_NO, "firmware download fail" },
		{ DSM_LASER_MEASURE_DISTANCE_ERROR_NO, "negative mersurement" },
	};
	uint32_t size = sizeof(err_to_str_map) / sizeof(errno_string_map);

	for (i = 0; i < size; ++i) {
		if (err_no == err_to_str_map[i].error_no)
			break;
	}
	if (i == size) {
		log_err("error type error\n");
		return -EPERM;
	}

	ret = strncat_s(info, LASER_DMD_INFO_LENGTH,
		err_to_str_map[i].name, LASER_DMD_INFO_LENGTH - strlen(info) - 1);
	return ret;
}

static int laser_dsm_get_info(int dev_type, int err_no, char info[])
{
	int ret;

	if (dev_type < 0 || dev_type >= HWLASER_TYPE_MAX) {
		log_err("device type error\n");
		return -EPERM;
	}
	ret = strncat_s(info, LASER_DMD_INFO_LENGTH,
		g_state_table[dev_type].name, LASER_DMD_INFO_LENGTH - strlen(info) - 1);

	if (get_errno_str(err_no, info))
		return -EPERM;

	return 0;
}

int laser_dsm_report_dmd(int dev_type, int err_no)
{
	int ret;
	char info[LASER_DMD_INFO_LENGTH] = {0};
	struct dsm_client *client = g_laser_dsm_client;

	if (!client) {
		log_err("client is null\n");
		return -EPERM;
	}

	if (dev_type != HWLASER_TYPE_MAX &&
		g_state_table[dev_type].state == PROBE_FAIL) {
		log_err("not working laser");
		return 0;
	}

	ret = laser_dsm_get_info(dev_type, err_no, info);
	if (ret) {
		log_err("get info failed\n");
		return -EPERM;
	}

	if (!dsm_client_ocuppy(client)) {
		dsm_client_record(client, "%s", info);
		dsm_client_notify(client, err_no);
		log_inf("report device type:%d, err_no:%d\n", dev_type, err_no);
		return 0;
	}

	log_err("laser dsm client is busy\n");
	return -EPERM;
}

static int __init laser_dsm_init(void)
{
	struct dsm_dev laser_dsm_dev_laser = {
		.name = "dsm_laser",
		.device_name = NULL,
		.ic_name = NULL,
		.module_name = NULL,
		.fops = NULL,
		.buff_size = LASER_DSM_BUF_SIZE_1024,
	};

	g_laser_dsm_client = dsm_register_client(&laser_dsm_dev_laser);

	if (!g_laser_dsm_client)
		log_inf("%s dsm register fail\n", g_laser_dsm_client->client_name);
	else
		log_inf("%s dsm register success\n", g_laser_dsm_client->client_name);

	return 0;
}

static void __exit laser_dsm_exit(void)
{
	g_laser_dsm_client = NULL;

	return;
}

subsys_initcall_sync(laser_dsm_init);
module_exit(laser_dsm_exit);

MODULE_DESCRIPTION("dsm for laser module driver");
MODULE_AUTHOR("Huawei Technologies Co., Ltd.");