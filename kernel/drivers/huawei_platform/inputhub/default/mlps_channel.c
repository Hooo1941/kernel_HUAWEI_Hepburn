/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2020-2020. All rights reserved.
 * Description: mlps channel source file
 * Author: jiangzimu <jiangzimu@huawei.com>
 * Create: 2022-02-16
 */

#include "mlps_channel.h"

#include <linux/fs.h>
#include <linux/init.h>
#include <linux/io.h>
#include <linux/miscdevice.h>
#include <linux/module.h>
#include <linux/of.h>
#include <linux/slab.h>
#include <linux/types.h>
#include <linux/uaccess.h>
#include <linux/ioctl.h>

#include "contexthub_boot.h"
#include "contexthub_ext_log.h"
#include "contexthub_recovery.h"
#include "contexthub_route.h"
#include "sensor_info.h"
#include <securec.h>
#include <platform_include/smart/linux/base/ap/protocol.h>

#include "teek_client_api.h"

#define MLPSHBIO                         0xB1
#define MLPS_IOCTL_MLPS_START          _IOW(MLPSHBIO, 0x01, short)
#define MLPS_IOCTL_MLPS_STOP           _IOW(MLPSHBIO, 0x02, short)
#define MLPS_IOCTL_MLPS_ATTR_START     _IOW(MLPSHBIO, 0x03, short)
#define MLPS_IOCTL_MLPS_ATTR_STOP      _IOW(MLPSHBIO, 0x04, short)
#define MLPS_IOCTL_MLPS_INTERVAL_SET   _IOW(MLPSHBIO, 0x05, short)
#ifndef SUCCESS
#define SUCCESS (0)
#endif
#ifndef FAIL
#define FAIL (-1)
#endif
#define MIN_LEN                  (12)
#define MAX_LEN                  (2048)
#define NORMAL_LEN               (8)
#define BIT_1                    (1)
#define MLPS_BIT_MAP_SIZE        (32)
#define MLPS_FD_DEFAULT          (-1)
#define MLPS_RECOVERY_LOW        0XFF
#define MLPS_RECOVERY_HIGH       0XFF
#define MLPS_RECOVERY_DATA_LEN   8
#define MLPS_RECOVERY_LEN        4
#define FILE_LIMIT_GNSS          0660
#define MLPS_SET_GNSS_ON         1
#define MLPS_SET_GNSS_CLOSE      0
#define MLPS_PRIVACY             0XFFFF
#define MLPS_COMPATIBLE_NAME     "hisilicon,mlps"

#define MLPS_TA_UUID { 0x53933180, 0xc8b0, 0x4a83, \
	{ 0x88, 0xd7, 0x8c, 0x76, 0xd2, 0x11, 0x7d, 0x19 } }

#define SESSION_START_DEFAULT    1
#define MLPS_NOTIFY_TA           5
#define NOTIFY_HAL_READY_CMD_ID  8
int32_t(*mlps_ops_conn_function[MAX_CONN_FUNCTION_NUM])(int32_t uart);
static struct workqueue_struct *g_mlps_gnss_wq = NULL;
struct mlps_gnss_work {
	struct work_struct worker;
	int cmd;
	int service;
	int privacy;
};
static bool g_support_conn_operation = false;
static int g_gnss_status = MLPS_WAIT_GNSS_OPERATION;
typedef enum {
	APP_START = 0,
	APP_HMSCORE,
	APP_SOFTBUS,
	APP_PRIVACY,
	MLPS_APP_MAX,
	MLPS_SENSORHUB_RECOVERY = 0XFFFF,
} mlps_app_type_t;

typedef enum {
	SOFT_MODULE_START = 0,
	HEARTBEAT_ALIVE,
	DECISION_CENTER,
	SOFTBUS_MODULE_MAX,
} softbus_module_type_t;

typedef enum {
	HMS_CORE_MODULE_START = 0,
	MODULE_NEARBY_BEACON = 0x06,
	MODULE_FIND_NETWORK = 0x07,
	HMSCORE_MODULE_MAX,
} hmscore_module_type_t;

typedef enum {
	TO_SH_DEVICE_INFO = 0,
	TO_SH_TIMESTAMP_SYNC,
	TO_AP_DEVICE_INFO_LIST = 0x0F,
	TO_AP_DEVICE_INFO,
	TO_AP_BLE_CONFIG,
	TO_AP_BLE_SCAN_RESULT,
	TO_AP_DEVICE_STATUS,
	TO_AP_DECISION_DATA,
} ap_and_sh_type;

typedef enum {
	TEE_PARAM_0 = 0,
	TEE_PARAM_1,
	TEE_PARAM_2,
	TEE_PARAM_3,
} tee_param_t;

static int64_t g_mlps_ref_cnt;
static unsigned int g_mlps_app_bit_map = 0;
static bool g_is_privacy = false;
static bool g_heartbeat_status = false;
static unsigned int g_privacy_cnt = 0;
typedef struct {
	unsigned int subcmd;
	unsigned short mlps_app_type;
	unsigned short app_len;
	unsigned short mlps_module_type;
	unsigned short module_len;
	unsigned short mlps_cmd;
	unsigned short cmd_len;
	unsigned char value[];
} pkt_mlps_detail_data_t;

struct mlps_cmd_map {
	int mlps_ioctl_app_cmd;
	int tag;
	enum obj_cmd cmd;
};

struct mlps_sucmd_map {
	mlps_app_type_t type;
	enum obj_sub_cmd subcmd;
};

static TEEC_Context g_mlps_context = {0};
static TEEC_Session g_mlps_session = {0};
static const u8 *g_mlps_package_name = "/dev/mlpskernel_ca";
static u32 g_uid;

static const struct mlps_cmd_map mlps_cmd_map_tab[] = {
	{ MLPS_IOCTL_MLPS_START, TAG_MLPS, CMD_CMN_OPEN_REQ },
	{ MLPS_IOCTL_MLPS_STOP, TAG_MLPS, CMD_CMN_CLOSE_REQ },
	{ MLPS_IOCTL_MLPS_ATTR_START, TAG_MLPS, CMD_CMN_CONFIG_REQ },
	{ MLPS_IOCTL_MLPS_ATTR_STOP, TAG_MLPS, CMD_CMN_CONFIG_REQ },
	{ MLPS_IOCTL_MLPS_INTERVAL_SET, TAG_MLPS, CMD_CMN_INTERVAL_REQ },
};

static unsigned int bit_check(unsigned int bit)
{
	return ((g_mlps_app_bit_map) & ((unsigned int)BIT_1 << ((bit) % MLPS_BIT_MAP_SIZE)));
}

static void bit_set(unsigned int bit)
{
	g_mlps_app_bit_map |= ((unsigned int)BIT_1 << ((bit) % MLPS_BIT_MAP_SIZE));
	return;
}

static void bit_clear(unsigned int bit)
{
	g_mlps_app_bit_map &= ~((unsigned int)BIT_1 << ((bit) % MLPS_BIT_MAP_SIZE));
	return;
}

int get_mlps_operate_gnss_status(void)
{
	return g_gnss_status;
}

static int mlps_ops_conn_function_echo(int uart)
{
	hwlog_info("%s enter\n", __func__);
	return MLPS_OPERATE_GNSS_FAIL;
}

static void mlps_ops_conn_function_init(void)
{
	int i;

	for (i = 0; i < MAX_CONN_FUNCTION_NUM; i++)
		mlps_ops_conn_function[i] = mlps_ops_conn_function_echo;
}

int mlps_ops_conn_function_register(unsigned int tag, connectivity_callback func)
{
	hwlog_info("%s enter\n", __func__);
	if (func == NULL  || tag >= MAX_CONN_FUNCTION_NUM) {
		hwlog_err("%s,register function is null or tag is error\n", __func__);
		return FAIL;
	}

	if (mlps_ops_conn_function[tag] != mlps_ops_conn_function_echo) {
		hwlog_err("CallbackExist:%u\n", tag);
		return FAIL;
	}

	mlps_ops_conn_function[tag] = func;
	hwlog_info("%s success\n", __func__);
	return SUCCESS;
}

static int mlps_privacy_open_ta_session(void)
{
	TEEC_Result result;
	TEEC_Operation op;
	TEEC_UUID uuid = MLPS_TA_UUID;
/* 1. initialize context */
	result = TEEK_InitializeContext(NULL, &g_mlps_context);
	if (result != TEEC_SUCCESS) {
		hwlog_err("%s teec initial failed", __func__);
		return FAIL;
	}
	hwlog_info("%s teec initial done", __func__);

/* 2. open session */
	u32 err_origin = 0;
	if (memset_s(&op, sizeof(op), 0, sizeof(op)) != EOK) {
		hwlog_err("%s memset_s failed", __func__);
		TEEK_FinalizeContext(&g_mlps_context);
		return FAIL;
	}

	// prepare operation parameters
	op.started = SESSION_START_DEFAULT;
	// params[2] is TEEC_MEMREF_TEMP_INPUT
	op.params[TEE_PARAM_2].tmpref.buffer = (void *)&g_uid;
	op.params[TEE_PARAM_2].tmpref.size = sizeof(g_uid);
	// params[3] is TEEC_MEMREF_TEMP_INPUT
	op.params[TEE_PARAM_3].tmpref.buffer = (void *)g_mlps_package_name;
	op.params[TEE_PARAM_3].tmpref.size = strlen(g_mlps_package_name) + 1;
	op.paramTypes = TEEC_PARAM_TYPES(TEEC_NONE, TEEC_NONE,
		TEEC_MEMREF_TEMP_INPUT, TEEC_MEMREF_TEMP_INPUT);
	// Open a session to the TA
	result = TEEK_OpenSession(&g_mlps_context, &g_mlps_session, &uuid,
			TEEC_LOGIN_IDENTIFY, NULL, &op, &err_origin);
	if (result != TEEC_SUCCESS) {
		hwlog_err("%s open session=%d and err_origin is %u",
			__func__, result, err_origin);
		TEEK_FinalizeContext(&g_mlps_context);
		return FAIL;
	}

	return SUCCESS;
}

static void mlps_privacy_invoke_ta(void)
{
	TEEC_Result result;
	TEEC_Operation op_invoke;
	u32 err_origin = 0;
/* 3. set in buffer and out buffer */
	if (memset_s(&op_invoke, sizeof(op_invoke), 0, sizeof(op_invoke)) != EOK) {
		hwlog_err("%s memset_s failed", __func__);
		return;
	}
	op_invoke.paramTypes = TEEC_PARAM_TYPES(TEEC_VALUE_INPUT,
		TEEC_VALUE_OUTPUT, TEEC_NONE, TEEC_NONE);
	op_invoke.started = SESSION_START_DEFAULT;
	op_invoke.params[TEE_PARAM_0].value.a = MLPS_NOTIFY_TA;
/* 4. invoke command to ta */
	result = TEEK_InvokeCommand(&g_mlps_session,
		NOTIFY_HAL_READY_CMD_ID, &op_invoke, &err_origin);
	if (result != TEEC_SUCCESS) {
		hwlog_err("%s TEEK_InvokeCommand failed res=0x%x orig=0x%x \n",
			__func__, result, err_origin);
		return;
	}
	hwlog_info("%s TEEK_InvokeCommand called successfully %d",
		__func__, op_invoke.params[TEE_PARAM_1].value.a);
}

static void mlps_privacy_op_ta(void)
{
	int ret = mlps_privacy_open_ta_session();
	if (ret == FAIL)
		return;

	mlps_privacy_invoke_ta();

	TEEK_CloseSession(&g_mlps_session);
	TEEK_FinalizeContext(&g_mlps_context);
}

static int mlps_operate_gnss(unsigned int cmd)
{
	if (g_is_privacy && cmd == MLPS_SET_GNSS_ON) {
		hwlog_info("%s is in privacy\n", __func__);
		return MLPS_OPERATE_GNSS_FAIL;
	}

	hwlog_info("%s enter\n", __func__);
	int ret = 0;
	int i = 0;
	for (i = 0; i < MAX_CONN_FUNCTION_NUM; i++)
		ret |= mlps_ops_conn_function[i](cmd);

	if (ret != 0) {
		hwlog_err("%s failed, ret is %d\n", __func__, ret);
		return MLPS_OPERATE_GNSS_FAIL;
	}

	hwlog_info("%s, cmd: %d success\n", __func__, cmd);
	return MLPS_OPERATE_GNSS_SUCCESS;
}

static bool mlps_check_bit_map(void)
{
	int i = 0;
	for (i = 0; i < MODULE_END; i++) {
		if (bit_check(i))
			return true;
	}

	return false;
}

static int mlps_open_gnss_power(unsigned int service)
{
	int ret = MLPS_OPERATE_GNSS_SUCCESS;
	hwlog_info("%s enter, service: %u, g_mlps_app_bit_map: %u\n", __func__, service, g_mlps_app_bit_map);
	if (bit_check(service)) {
		hwlog_warn("%s service %d has opened\n", __func__, service);
		return ret;
	}

	if (mlps_check_bit_map() == false) {
		ret = mlps_operate_gnss(MLPS_SET_GNSS_ON);
		hwlog_warn("%s service %d open success\n", __func__, service);
	}

	if (ret == MLPS_OPERATE_GNSS_FAIL)
		return ret;

	bit_set(service);
	return ret;
}

static int mlps_close_gnss_power(unsigned int service)
{
	int ret = MLPS_OPERATE_GNSS_SUCCESS;
	hwlog_info("%s enter, service: %u, g_mlps_app_bit_map: %u\n", __func__, service, g_mlps_app_bit_map);
	if (!bit_check(service)) {
		hwlog_warn("%s service %d has closed\n", __func__, service);
		return ret;
	}

	bit_clear(service);

	if (mlps_check_bit_map() == false) {
		ret = mlps_operate_gnss(MLPS_SET_GNSS_CLOSE);
		hwlog_warn("%s close success\n", __func__);
	}

	return ret;
}

static void mlps_notify_ta_privacy(unsigned int privacy)
{
	if (privacy == false)
		return;

	if (mlps_check_bit_map() == true) {
		hwlog_info("%s mlps_check_bit_map fail\n", __func__);
		return;
	}

	if (g_is_privacy && g_privacy_cnt == 0)
		mlps_privacy_op_ta();
}

static void mlps_gnss_handle(struct work_struct *work)
{
	int ret = MLPS_OPERATE_GNSS_SUCCESS;
	struct mlps_gnss_work *p = container_of(work, struct mlps_gnss_work, worker);
	if (p->cmd == MLPS_CMD_OPEN) {
		ret = mlps_open_gnss_power(p->service);
	} else if (p->cmd == MLPS_CMD_CLOSE) {
		ret = mlps_close_gnss_power(p->service);
		if (p->privacy == true && g_privacy_cnt > 0)
			g_privacy_cnt--;
		if (ret == MLPS_OPERATE_GNSS_SUCCESS)
			mlps_notify_ta_privacy(p->privacy);
	} else {
		ret = MLPS_NOT_SUPPORT_GNSS_OPERATION;
		hwlog_warn("%s unkonwn cmd %u\n", __func__, p->cmd);
	}
	g_gnss_status = ret;
	kfree(p);
	return;
}

int mlps_operate_gnss_power(unsigned short cmd, unsigned int service)
{
	hwlog_info("%s, cmd: %u, service: %u\n", __func__, cmd, service);
	int ret = MLPS_OPERATE_GNSS_SUCCESS;
	g_gnss_status = MLPS_WAIT_GNSS_OPERATION;
	if (g_support_conn_operation  == false) {
		g_gnss_status = MLPS_NOT_SUPPORT_GNSS_OPERATION;
		ret = MLPS_NOT_SUPPORT_GNSS_OPERATION;
		return ret;
	}

	if (service >= MODULE_END) {
		g_gnss_status = MLPS_NOT_SUPPORT_GNSS_OPERATION;
		ret = MLPS_NOT_SUPPORT_GNSS_OPERATION;
		hwlog_warn("%s unkonwn service %u\n", __func__, service);
		return ret;
	}
	struct mlps_gnss_work  *gnss_work = NULL;
	gnss_work = kzalloc(sizeof(struct mlps_gnss_work), GFP_ATOMIC);
	if (ZERO_OR_NULL_PTR((unsigned long)(uintptr_t)gnss_work)) {
		pr_err("[%s] alloc gnss_work failed.\n", __func__);
		g_gnss_status = MLPS_OPERATE_GNSS_FAIL;
		ret = MLPS_OPERATE_GNSS_FAIL;
		return ret;
	}
	gnss_work->privacy = false;
	if (service == SUPER_PRIVACY) {
		service = SOFTBUS_HEARTBEAT_ALIVE;
		gnss_work->privacy = true;
	}
	gnss_work->cmd = cmd;
	gnss_work->service = service;
	INIT_WORK(&gnss_work->worker, mlps_gnss_handle);
	queue_work(g_mlps_gnss_wq, &gnss_work->worker);
	return ret;
}

static void mlps_parse_softbus_module(pkt_mlps_detail_data_t *data)
{
	switch (data->mlps_module_type) {
	case HEARTBEAT_ALIVE:
		mlps_operate_gnss_power(data->mlps_cmd, SOFTBUS_HEARTBEAT_ALIVE);
		if (data->mlps_cmd == MLPS_CMD_OPEN) {
			g_heartbeat_status = true;
			hwlog_info("%s SOFTBUS_HEARTBEAT_ALIVE open\n", __func__);
		} else if (data->mlps_cmd == MLPS_CMD_CLOSE) {
			g_heartbeat_status = false;
			hwlog_info("%s SOFTBUS_HEARTBEAT_ALIVE close\n", __func__);
		}
		break;

	default:
		hwlog_warn("%s unkonwn mlps softbus %d\n", __func__, data->mlps_module_type);
		break;
	}
}

static void mlps_parse_app(pkt_mlps_detail_data_t *data)
{
	switch (data->mlps_app_type) {
	case APP_SOFTBUS:
		mlps_parse_softbus_module(data);
		break;

	default:
		hwlog_warn("%s unkonwn mlps app %d\n", __func__, data->mlps_app_type);
		break;
	}
}

static void mlps_parse_data(pkt_mlps_detail_data_t *data)
{
	if (data->mlps_cmd == MLPS_CMD_OPEN || data->mlps_cmd == MLPS_CMD_CLOSE)
		mlps_parse_app(data);
}

static ssize_t upload_mlps_data(char *data, ssize_t len)
{
	return inputhub_route_write(ROUTE_MLPS_PORT, data, len);
}

static int send_mlps_cmd_internal(int tag, enum obj_cmd cmd,
	enum obj_sub_cmd subcmd, bool use_lock)
{
	interval_param_t interval_param;

	(void)memset_s(&interval_param, sizeof(interval_param), 0,
		sizeof(interval_param));
	if (cmd == CMD_CMN_OPEN_REQ) {
		if (really_do_enable_disable(&g_mlps_ref_cnt, true, 0)) {
			if (use_lock) {
				inputhub_sensor_enable(tag, true);
				inputhub_sensor_setdelay(tag, &interval_param);
			} else {
				inputhub_sensor_enable_nolock(tag, true);
				inputhub_sensor_setdelay_nolock(tag,
					&interval_param);
			}
		}
		hwlog_info("send_mlps_cmd open cmd:%d, mlps_ref_cnt= %x\n", cmd, g_mlps_ref_cnt);
	} else if (cmd == CMD_CMN_CLOSE_REQ) {
		if (really_do_enable_disable(&g_mlps_ref_cnt, false, 0)) {
			if (use_lock)
				inputhub_sensor_enable(tag, false);
			else
				inputhub_sensor_enable_nolock(tag, false);
		}
		hwlog_info("send_mlps_cmd close cmd:%d, mlps_ref_cnt= %x\n", cmd, g_mlps_ref_cnt);
	}
	return 0;
}

static int send_mlps_cmd(enum obj_cmd cmd, unsigned long arg, ssize_t len)
{
	void __user *argp = (void __user *)(uintptr_t)arg;
	int argvalue = 0;
	int i;
	int loop_num;

	loop_num = sizeof(mlps_cmd_map_tab) /
		sizeof(mlps_cmd_map_tab[0]);
	for (i = 0; i < loop_num; i++) {
		if (mlps_cmd_map_tab[i].mlps_ioctl_app_cmd == cmd)
			break;
	}

	if (i == loop_num) {
		hwlog_err("%s unknown cmd %d in mlps\n", __func__, cmd);
		return -EFAULT;
	}

	if (copy_from_user(&argvalue, argp, sizeof(argvalue)))
		hwlog_err("%s copy_from_user fail or arg is NULL\n", __func__);

	return send_mlps_cmd_internal(mlps_cmd_map_tab[i].tag,
		mlps_cmd_map_tab[i].cmd, argvalue, true);
}

static int send_mlps_cmd_ext(void *data, ssize_t len,
	struct read_info *pkg_mcu, bool use_lock)
{
	struct write_info pkg_ap;
	int ret = 0;

	if (!(data) || (len <= 0)) {
		hwlog_err("%s para failed\n", __func__);
		return -1;
	}

	if (memset_s(&pkg_ap, sizeof(pkg_ap), 0, sizeof(pkg_ap))) {
		hwlog_err("%s memset data failed\n", __func__);
		return -1;
	}

	pkg_ap.cmd = CMD_CMN_CONFIG_REQ;
	pkg_ap.tag = TAG_MLPS;
	pkg_ap.wr_buf = data;
	pkg_ap.wr_len = SUBCMD_LEN + len;

	ret = write_customize_cmd(&pkg_ap, NULL, use_lock);
	if (ret < 0) {
		hwlog_err("%s: err. write cmd\n", __func__);
		return -1;
	}
	return ret;
}

static int process_mlps_data_from_mcu(const struct pkt_header *head)
{
	unsigned char *data = (unsigned char *)head + sizeof(struct pkt_header);
	upload_mlps_data(data, head->length);
	hwlog_err("%s: upload mlps data\n", __func__);
	return 0;
}

/* read /dev/mlpshub */
static ssize_t mlps_read(struct file *file, char __user *data, size_t count,
			loff_t *pos)
{
	return inputhub_route_read(ROUTE_MLPS_PORT, data, count);
}

static int check_mlps_privacy_mode(pkt_mlps_detail_data_t *data)
{
	if (data->mlps_app_type != APP_PRIVACY || data->app_len != NORMAL_LEN)
		return 0;

	if (data->mlps_module_type != MLPS_PRIVACY || data->module_len != MLPS_RECOVERY_LEN)
		return 0;

	if (data->mlps_cmd == MLPS_CMD_OPEN) {
		g_is_privacy = true;
		g_privacy_cnt++;
		(void)mlps_operate_gnss_power(MLPS_CMD_CLOSE, SUPER_PRIVACY);
		hwlog_info("%s: privacy mode in cnt = %d\n", __func__, g_privacy_cnt);
	} else if (data->mlps_cmd == MLPS_CMD_CLOSE) {
		g_is_privacy = false;
		if (g_heartbeat_status)
			(void)mlps_operate_gnss_power(MLPS_CMD_OPEN, SUPER_PRIVACY);
		hwlog_info("%s: privacy mode out cnt = %d\n", __func__, g_privacy_cnt);
	}

	return -1;
}

static int check_mlps_data(pkt_mlps_detail_data_t *data)
{
	if (data->mlps_app_type <= APP_START || data->mlps_app_type >= MLPS_APP_MAX) {
		hwlog_err("%s: app fail\n", __func__);
		return -1;
	}

	if (data->mlps_cmd < MLPS_CMD_OPEN || data->mlps_cmd >= MLPS_CMD_UPLOAD) {
		hwlog_err("%s: cmd fail\n", __func__);
		return -1;
	}

	return check_mlps_privacy_mode(data);
}

/* write to /dev/motionhub, when need tansfer big data ,use this channel */
static ssize_t mlps_write(struct file *file, const char __user *user_data,
			 size_t len, loff_t *ppos)
{
	if (!(user_data) || (len < MIN_LEN) || (len > MAX_LEN)) {
		hwlog_err("%s para failed\n", __func__);
		return len;
	}

	char *data = (char *)kzalloc(len + SUBCMD_LEN, GFP_KERNEL);
	if (data == NULL) {
		hwlog_err("%s user data kzalloc fail\n", __func__);
		return len;
	}

	pkt_mlps_detail_data_t *temp = (pkt_mlps_detail_data_t *)data;
	temp->subcmd = SUB_CMD_MLPS_CONFIG_REQ;
	if (copy_from_user(&temp->mlps_app_type, user_data, len)) {
		hwlog_err("%s copy from user fail\n", __func__);
		if (data != NULL) {
			kfree(data);
		}
		return len;
	}

	int ret = check_mlps_data((pkt_mlps_detail_data_t *)data);
	if (ret == 0) {
#ifdef CONFIG_MLPS_OPERATE_CONN
		mlps_parse_data((pkt_mlps_detail_data_t *)data);
#endif
		ret = send_mlps_cmd_ext(data, len, NULL, true);
		if (ret != 0)
			hwlog_err("mlps_write work error\n");
	}

	if (data != NULL) {
		kfree(data);
		data = NULL;
	}

	return len;
}

static void notify_ap_mlps_recovery(void)
{
	char mlps_recovery_data[MLPS_RECOVERY_DATA_LEN] = {MLPS_RECOVERY_HIGH, MLPS_RECOVERY_LOW,
		MLPS_RECOVERY_LEN, 0, 0, 0, 0, 0};
	upload_mlps_data((char *)mlps_recovery_data, MLPS_RECOVERY_DATA_LEN);
}

/*
 * Description: ioctl function to /dev/mlpshub, do open, do config/debug
 *              cmd to mlpshub
 * Return:      result of ioctrl command, 0 successed, -ENOTTY failed
 */
static long mlps_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
{
	switch (cmd) {
	case MLPS_IOCTL_MLPS_START:
		hwlog_info("mlps: %s  cmd: MLPS_IOCTL_MLPS_START\n", __func__);
		break;

	case MLPS_IOCTL_MLPS_STOP:
		hwlog_info("mlps: %s  cmd: MLPS_IOCTL_MLPS_STOP\n", __func__);
		break;

	case MLPS_IOCTL_MLPS_ATTR_START:
		hwlog_info("mlps: %s  cmd: MLPS_IOCTL_MLPS_ATTR_START\n", __func__);
		break;

	case MLPS_IOCTL_MLPS_ATTR_STOP:
		hwlog_info("mlps: %s  cmd: MLPS_IOCTL_MLPS_ATTR_STOP\n", __func__);
		break;

	case MLPS_IOCTL_MLPS_INTERVAL_SET:
		hwlog_info("mlps: %s  cmd: MLPS_IOCTL_MLPS_INTERVAL_SET\n", __func__);
		break;

	default:
		hwlog_err("mlps: %s unknown cmd : %u\n", __func__, cmd);
		return -ENOTTY;
	}

	return send_mlps_cmd(cmd, arg, sizeof(arg));
}

/* open to /dev/mlpshub, do nothing now */
static int mlps_open(struct inode *inode, struct file *file)
{
	hwlog_info("mlps: enter %s\n", __func__);
	return 0;
}

/* release to /dev/mlpshub, do nothing now */
static int mlps_release(struct inode *inode, struct file *file)
{
	hwlog_info("mlps: %s ok\n", __func__);
	return 0;
}

static void enable_mlps_when_recovery_iom3(void)
{
	g_mlps_ref_cnt = 0;

	send_mlps_cmd_internal(TAG_MLPS, CMD_CMN_OPEN_REQ,
		SUB_CMD_NULL_REQ, false);
}

static int mlps_recovery_notify(struct notifier_block *nb,
	unsigned long foo, void *bar)
{
	/* prevent access the emmc now: */
	hwlog_info("%s (%lu) +\n", __func__, foo);
	switch (foo) {
	case IOM3_RECOVERY_IDLE:
		notify_ap_mlps_recovery();
		break;
	case IOM3_RECOVERY_START:
	case IOM3_RECOVERY_MINISYS:
	case IOM3_RECOVERY_3RD_DOING:
	case IOM3_RECOVERY_FAILED:
		break;
	case IOM3_RECOVERY_DOING:
		save_step_count();
		enable_mlps_when_recovery_iom3();
		break;
	default:
		hwlog_err("%s -unknow state %ld\n", __func__, foo);
		break;
	}
	hwlog_info("%s -\n", __func__);
	return 0;
}

static struct notifier_block g_mlps_recovery_notify = {
	.notifier_call = mlps_recovery_notify,
	.priority = -1,
};

/* file_operations to mlps */
static const struct file_operations mlps_fops = {
	.owner = THIS_MODULE,
	.llseek = no_llseek,
	.read = mlps_read,
	.write = mlps_write,
	.unlocked_ioctl = mlps_ioctl,
#ifdef CONFIG_COMPAT
	.compat_ioctl = mlps_ioctl,
#endif
	.open = mlps_open,
	.release = mlps_release,
};

/* miscdevice to motion */
static struct miscdevice mlpshub_miscdev = {
	.minor = MISC_DYNAMIC_MINOR,
	.name = "mlpshub",
	.fops = &mlps_fops,
};

static int is_mlps_supported(void)
{
	int len = 0;
	struct device_node *mlps_node = NULL;
	const char *mlps_status = NULL;
	const char *mlps_operate_conn = NULL;
	static int ret = SUCCESS;

	mlps_node = of_find_compatible_node(NULL, NULL, MLPS_COMPATIBLE_NAME);
	if (!mlps_node) {
		hwlog_err("%s, can not find node mlps_status\n", __func__);
		return FAIL;
	}

	mlps_status = of_get_property(mlps_node, "status", &len);
	if (!mlps_status) {
		hwlog_err("%s, can't find property status\n", __func__);
		return FAIL;
	}

	if (strstr(mlps_status, "ok")) {
		hwlog_info("%s, mlps enabled!\n", __func__);
		ret = SUCCESS;
		if (of_property_read_string_index(mlps_node, "operate_conn", 0, &mlps_operate_conn) != 0)
			hwlog_err("%s, fail to read mlps operate_conn\n", __func__);
		if (strstr(mlps_operate_conn, "ok")) {
			g_support_conn_operation = true;
			hwlog_info("%s, mlps needs to operate conn!\n", __func__);
		}
	} else {
		hwlog_err("%s, mlps disabled!\n", __func__);
		ret = FAIL;
	}
	return ret;
}

/*
* Description:	apply kernel buffer, register mlps_miscdev
* Return: 		result of function, 0 successed, else false
*/
static int __init mlpshub_init(void)
{
	int ret;

	if (is_sensorhub_disabled())
		return -1;

	ret = is_mlps_supported();
	if (ret != 0)
	{
		hwlog_info("%s, not support mlps!\n", __func__);
		return ret;
	}

	mlps_ops_conn_function_init();

	g_mlps_gnss_wq = create_singlethread_workqueue("mlps_gnss_workqueue");
	if (g_mlps_gnss_wq == NULL) {
		hwlog_err("create_singlethread_workqueue fail\n");
		return -EFAULT;
	}

	ret = inputhub_route_open(ROUTE_MLPS_PORT);
	if (ret != 0) {
		hwlog_err("mlps: %s cannot open inputhub route err=%d\n",
			__func__, ret);
		return ret;
	}

	ret = misc_register(&mlpshub_miscdev);
	if (ret != 0) {
		hwlog_err("%s cannot register miscdev err=%d\n", __func__, ret);
		inputhub_route_close(ROUTE_MLPS_PORT);
		return ret;
	}
	register_mcu_event_notifier(TAG_MLPS, CMD_DATA_REQ, process_mlps_data_from_mcu);
	register_iom3_recovery_notifier(&g_mlps_recovery_notify);
	hwlog_info("%s ok\n", __func__);
	return ret;
}

/* release kernel buffer, deregister mlpshub_ud_miscdev */
static void __exit mlpshub_exit(void)
{
	inputhub_route_close(ROUTE_MLPS_PORT);
	misc_deregister(&mlpshub_miscdev);
	hwlog_info("exit %s\n", __func__);
}

late_initcall_sync(mlpshub_init);
module_exit(mlpshub_exit);

MODULE_LICENSE("GPL v2");
MODULE_DESCRIPTION("MLPSHub driver");
MODULE_AUTHOR("Huawei Technologies Co., Ltd.");
