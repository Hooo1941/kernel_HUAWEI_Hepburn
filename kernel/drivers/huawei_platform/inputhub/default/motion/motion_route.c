/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2020-2020. All rights reserved.
 * Description: motion route source file
 * Author: linjianpeng <linjianpeng1@huawei.com>
 * Create: 2020-05-25
 */

#include "motion_route.h"

#include <linux/device.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/of.h>
#include <linux/slab.h>
#include <linux/types.h>

#include <huawei_platform/inputhub/motionhub.h>
#include <log/hiview_hievent.h>
#include <securec.h>
#include "contexthub_recovery.h"
#include "contexthub_route.h"
#include "contexthub_pm.h"


#define SENSOR_FALL_IMONITOR_ID 936005010
#define MOTION_SF_PARA_LEN 16
#define MOTION_TYPE 0
#define SHAKE_RES_LENGTH 2
#define SHAKE_RESULT 1

static int motion_rcv_sf_info(unsigned long value)
{
	int ret;
	uint8_t app_config[MOTION_SF_PARA_LEN] = { 0 };

	app_config[0] = MOTION_TYPE_BACK_TAP;
	app_config[1] = SUB_CMD_MOTION_RCV_STATUS_REQ;
	app_config[2] = (uint8_t)value;

	if ((get_iom3_state() == IOM3_ST_RECOVERY) ||
		(get_iomcu_power_state() == ST_SLEEP))
		ret = send_app_config_cmd(TAG_MOTION, app_config, false);
	else
		ret = send_app_config_cmd(TAG_MOTION, app_config, true);

	if (ret) {
		hwlog_err("send motion %d cfg data to mcu fail,ret=%d\n",
			(int)app_config[0], ret);
		return RET_FAIL;
	}
	hwlog_info("write motion success. sf_rcv_info:%d\n", (int)app_config[2]);
	return RET_SUCC;
}
int send_motion_type_switch(unsigned long type, unsigned long cmd)
{
	int ret;
	uint8_t app_config[MOTION_SF_PARA_LEN] = { 0 };

	app_config[0] = (uint8_t)type;
	app_config[1] = (uint8_t)cmd;

	if ((get_iom3_state() == IOM3_ST_RECOVERY) ||
		(get_iomcu_power_state() == ST_SLEEP))
		ret = send_app_config_cmd(TAG_MOTION, app_config, false);
	else
		ret = send_app_config_cmd(TAG_MOTION, app_config, true);

	if (ret) {
		hwlog_err("send motion %d cfg data to mcu fail,ret=%d\n",
			(int)app_config[0], ret);
		return RET_FAIL;
	}
	hwlog_info("write motion success. motion_ctrl_type:%d %d\n", (int)app_config[0], (int)app_config[1]);
	return RET_SUCC;
}
void motion_speaker_status_change(unsigned long value)
{
	int ret;

	ret = motion_rcv_sf_info(value);
	if (ret)
		hwlog_info("motion_speaker_status_change fail. %d\n", ret);
}

void motion_finger_status_change(unsigned long value)
{
	int ret;

	ret = motion_rcv_sf_info(value);
	if (ret)
		hwlog_info("motion_finger_status_change fail. %d\n", ret);
}

#ifdef CONFIG_HUAWEI_DSM
static void motion_fall_down_report(const struct motion_fall_down_data *info)
{
	int ret;
	unsigned int ret_u = 0;
	struct hiview_hievent *hi_event = NULL;

	if (!info) {
		hwlog_err("%s info NULL\n", __func__);
		return;
	}

	hi_event = hiview_hievent_create(SENSOR_FALL_IMONITOR_ID);
	if (!hi_event) {
		hwlog_err("%s create hievent fail\n", __func__);
		return;
	}

	ret_u |= (unsigned int)hiview_hievent_put_integral(hi_event,
		"fall_status", info->fall_status);
	ret_u |= (unsigned int)hiview_hievent_put_integral(hi_event,
		"impact_force", info->impact_force);
	ret_u |= (unsigned int)hiview_hievent_put_integral(hi_event,
		"fall_height", info->fall_height);
	ret_u |= (unsigned int)hiview_hievent_put_integral(hi_event,
		"still_time", info->still_time);
	ret_u |= (unsigned int)hiview_hievent_put_integral(hi_event,
		"confidence", info->confidence);
	ret_u |= (unsigned int)hiview_hievent_put_integral(hi_event,
		"suspected_cnt", info->suspected_cnt);
	if (ret_u != 0)
		hwlog_err("%s put integral fail ret_put=%d\n", __func__,
			(int)ret_u);

	ret = hiview_hievent_report(hi_event);
	if (ret < 0)
		hwlog_err("%s report fail ret=%d\n", __func__, ret);

	hiview_hievent_destroy(hi_event);

	hwlog_info("%s fall_status=%d impact_force=%d fall_height=%d still_time=%d confidence=%d suspected_cnt=%d\n",
		__func__, info->fall_status, info->impact_force,
		info->fall_height, info->still_time,
		info->confidence, info->suspected_cnt);
}
#endif

#define MOTION_SHAKE_PATH                   "/sys/bus/platform/drivers/huawei,camcfgdev/get_motion_data"
int write_buf_to_node(void *buf, u32 len)
{
	int ret = 0;
	int flags;
	struct file *fp = NULL;
	mm_segment_t old_fs;
	char shake_temp_file[PATH_MAXLEN] = MOTION_SHAKE_PATH;

	old_fs = get_fs();
	set_fs(KERNEL_DS);

	flags = O_CREAT | O_RDWR | O_TRUNC;
	fp = filp_open(shake_temp_file, flags, FILE_LIMIT);
	if (IS_ERR(fp)) {
		hwlog_err("%s():open file %s err, err=%d\n", __func__, shake_temp_file, PTR_ERR(fp));
		ret = -1;
		goto file_err;
	}

	ret = vfs_write(fp, buf, len, &(fp->f_pos));
	if (ret != len) {
		hwlog_err("%s:write file %s exception with ret %d\n",
			__func__, shake_temp_file, ret);
		ret = -1;
	}
	filp_close(fp, NULL);
file_err:
	set_fs(old_fs);
	return ret;
}

int inputhub_process_motion_report(const struct pkt_header *head)
{
	char *motion_data = (char *)head + sizeof(pkt_common_data_t);

	if (motion_data[MOTION_TYPE] == MOTIONHUB_TYPE_SHAKE) {
		char shake_res[SHAKE_RES_LENGTH];
		sprintf_s(shake_res, sizeof(shake_res), "%d", SHAKE_RESULT);
		write_buf_to_node(shake_res, sizeof(shake_res));
	}

	if ((((int)motion_data[0]) == MOTIONHUB_TYPE_TAKE_OFF) ||
		(((int)motion_data[0]) == MOTIONHUB_TYPE_PICKUP) ||
		(((int)motion_data[0]) == MOTION_TYPE_REMOVE)) {
		inputhub_route_pm_wakeup_event(jiffies_to_msecs(HZ));
		hwlog_err("%s weaklock HZ motiontype=%d\n", __func__,
			motion_data[0]);
	}

	hwlog_info("%s : motiontype = %d motion_result = %d motion_status = %d\n",
		/* 2 is motion_status */
		__func__, motion_data[0], motion_data[1], motion_data[2]);

	return inputhub_route_write(ROUTE_MOTION_PORT, motion_data,
		head->length -
		(sizeof(pkt_common_data_t) - sizeof(struct pkt_header)));
}

bool is_motion_data_report(const struct pkt_header *head)
{
	/* all sensors report data with command CMD_PRIVATE */
	return (head->tag == TAG_MOTION) && (head->cmd == CMD_DATA_REQ);
}

void process_step_counter_report(const struct pkt_header *head)
{
	uint16_t cnt;

	inputhub_route_pm_wakeup_event(jiffies_to_msecs(HZ));

	cnt = ((struct pkt_step_counter_motion_req *)head)->data_hd.cnt;
	if (cnt == 1) {
		struct pkt_motion_data *mt = NULL;

		hwlog_info("Kernel get pedometer motion\n");
		mt = &(((struct pkt_step_counter_motion_req *)head)->mt);
#ifdef CONFIG_HUAWEI_DSM
		if (mt->motion_type == MOTION_TYPE_FALL_DOWN)
			motion_fall_down_report(
		 		(const struct motion_fall_down_data *)mt->data);
#endif
		return;
	}

	if (cnt == 0)
		hwlog_info("Kernel get pedometer event\n");
	else
		hwlog_info("Kernel get pedometer event cnt=%u\n", cnt);

	step_counter_data_process((pkt_step_counter_data_req_t *)head);
	report_sensor_event(head->tag,
		(int *)(&((pkt_step_counter_data_req_t *) head)->step_count),
		head->length);
}

