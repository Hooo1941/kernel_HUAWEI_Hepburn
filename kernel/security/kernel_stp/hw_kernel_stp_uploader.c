/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2018-2022. All rights reserved.
 * Description: the hw_kernel_stp_uploader.c for kernel data uploading through uevent
 * Create: 2018-03-31
 */

#include "hw_kernel_stp_uploader.h"
#include <linux/rtc.h>
#include <linux/time.h>
#include <securec.h>

#define KSTP_LOG_TAG "kernel_stp_uploader"

#define YEAR_BASE 1900
#define SECONDS_PER_MINUTE 60
#define STP_DATE_LEN 16
#define TIMESTAMP_FORMAT "%04d%02d%02d%02d%02d%02d"

static struct kobject *g_kernel_stp_kobj;
static struct kset *g_kernel_stp_kset;
static DEFINE_MUTEX(upload_mutex);

const struct stp_item_fun g_funcs[] = {
	{KERNEL_STP_UPLOAD, kernel_stp_upload_parse},
	{KERNEL_STP_KSHIELD_UPLOAD, kernel_stp_kshield_upload_parse},
	{KERNEL_STP_REPORT_SECURITY_INFO, kernel_stp_report_security_info_parse},
};

const struct stp_item_info item_info[] = {
	[KCODE]        = { STP_ID_KCODE, STP_NAME_KCODE },
	[SYSCALL]      = { STP_ID_KCODE_SYSCALL, STP_NAME_KCODE_SYSCALL },
	[SE_ENFROCING] = { STP_ID_SE_ENFROCING, STP_NAME_SE_ENFROCING },
	[SE_HOOK]      = { STP_ID_SE_HOOK, STP_NAME_SE_HOOK },
	[ROOT_PROCS]   = { STP_ID_ROOT_PROCS, STP_NAME_ROOT_PROCS },
	[SETIDS]       = { STP_ID_SETIDS, STP_NAME_SETIDS },
	[KEY_FILES]    = { STP_ID_KEY_FILES, STP_NAME_KEY_FILES },
	[USERCOPY]     = { STP_ID_USERCOPY, STP_NAME_USERCOPY },
	[CFI]          = { STP_ID_CFI, STP_NAME_CFI },
	[MOD_SIGN]     = { STP_ID_MOD_SIGN, STP_NAME_MOD_SIGN },
	[PTRACE]       = { STP_ID_PTRACE, STP_NAME_PTRACE },
	[HKIP]         = { STP_ID_HKIP, STP_NAME_HKIP },
	[ITRUSTEE]     = { STP_ID_ITRUSTEE, STP_NAME_ITRUSTEE },
	[DOUBLE_FREE]  = { STP_ID_DOUBLE_FREE, STP_NAME_DOUBLE_FREE },
	[KSHIELD]      = { STP_ID_KSHIELD, STP_NAME_KSHIELD },
};

const struct stp_item_info *get_item_info_by_idx(int idx)
{
	if (idx >= 0 && idx < STP_ITEM_MAX)
		return &item_info[idx];
	else
		return NULL;
};

int kernel_stp_uploader_init(void)
{
	const char *kernel_stp_kobj_name = "hw_kernel_stp_scanner";
	const char *kernel_stp_kset_name = "hw_kernel_stp_kset";
	int ret;

	do {
		g_kernel_stp_kobj = kobject_create_and_add(kernel_stp_kobj_name,
							kernel_kobj);
		if (g_kernel_stp_kobj == NULL) {
			kstp_log_error(KSTP_LOG_TAG, "creat kobject failed");
			ret = KSTP_ERRCODE;
			break;
		}

		g_kernel_stp_kset =  kset_create_and_add(kernel_stp_kset_name,
							NULL, kernel_kobj);
		if (g_kernel_stp_kset == NULL) {
			kstp_log_error(KSTP_LOG_TAG, "creat kset failed");
			ret = KSTP_ERRCODE;
			break;
		}
		g_kernel_stp_kobj->kset = g_kernel_stp_kset;

		ret = kobject_uevent(g_kernel_stp_kobj, KOBJ_ADD);
		if (ret != 0) {
			kstp_log_error(KSTP_LOG_TAG, "kobj_uevent add failed, result is %d", ret);
			break;
		}
	} while (0);

	if (ret != 0) {
		kernel_stp_uploader_exit();
		kstp_log_error(KSTP_LOG_TAG, "kernel stp kobj init failed");
		return ret;
	}

	kstp_log_trace(KSTP_LOG_TAG, "kernel_stp_kobj_init ok!");
	return ret;
}

void kernel_stp_uploader_exit(void)
{
	if (g_kernel_stp_kobj != NULL) {
		kobject_put(g_kernel_stp_kobj);
		g_kernel_stp_kobj = NULL;
	}

	if (g_kernel_stp_kset != NULL) {
		kset_unregister(g_kernel_stp_kset);
		g_kernel_stp_kset = NULL;
	}

	kstp_log_trace(KSTP_LOG_TAG, "kernel_stp_kobj_deinit ok!");
}

int kernel_stp_upload_parse(struct stp_item result, const char *addition_info,
			char *upload_info)
{
	if (upload_info == NULL) {
		kstp_log_error(KSTP_LOG_TAG, "input arguments invalid");
		return -EINVAL;
	}
	if (addition_info == NULL)
		snprintf(upload_info, STP_INFO_MAXLEN, "stpinfo=%u:%u:%u:%u:%s",
			result.id, result.status, result.credible,
			result.version, result.name);
	else
		snprintf(upload_info, STP_INFO_MAXLEN, "stpinfo=%u:%u:%u:%u:%s:%s",
			result.id, result.status, result.credible,
			result.version, result.name, addition_info);
	return 0;
}

int kernel_stp_kshield_upload_parse(struct stp_item result, const char *addition_info,
			char *upload_info)
{
	(void)result;
	if (upload_info == NULL) {
		kstp_log_error(KSTP_LOG_TAG, "input arguments invalid");
		return -EINVAL;
	}
	/* kshield has a diffrent tag and info, so deal with it separately */
	if (addition_info == NULL)
		return -EINVAL;
	snprintf(upload_info, STP_INFO_MAXLEN, "kshieldinfo=%s", addition_info);
	return 0;
}

int kernel_stp_report_security_info_parse(struct stp_item result,
			const char *addition_info, char *upload_info)
{
	if (upload_info == NULL || addition_info == NULL) {
		kstp_log_error(KSTP_LOG_TAG, "input arguments invalid");
		return -EINVAL;
	}

	(void)result;
	if (snprintf_s(upload_info, STP_INFO_MAXLEN, STP_INFO_MAXLEN - 1,
		"security_guard=%s", addition_info) < 0) {
		kstp_log_error(KSTP_LOG_TAG, "snprintf_s error");
		return -1;
	}
	return 0;
}

/* concatenate kernel_stp data of type int and type char array */
static int kernel_stp_data_adapter(char **uevent_envp, char *result)
{
	int index = 0;

	if ((uevent_envp == NULL) || (result == NULL)) {
		kstp_log_error(KSTP_LOG_TAG, "input arguments invalid");
		return -EINVAL;
	}

	for (index = 0; index < KERNEL_STP_UEVENT_LEN - 1; index++) {
		uevent_envp[index] = result;
		kstp_log_debug(KSTP_LOG_TAG, "uevent_envp[%d] is %s",
				index, uevent_envp[index]);
	}

	return 0;
}

static int kernel_stp_upload_parse_cmd(struct stp_item result, const char *addition_info,
			char *upload_info, enum stp_item_fun_type cmd)
{
	int i;

	for (i = 0; i < KERNEL_STP_UPLOAD_MAX; i++) {
		if (g_funcs[i].type == cmd)
			return g_funcs[i].FUNC(result, addition_info, upload_info);
	}
	return -EINVAL;
}

static int kernel_stp_upload_cmd(struct stp_item result, const char *addition_info,
			enum stp_item_fun_type cmd)
{
	int ret;
	char *upload_info = NULL;
	char *uevent_envp[KERNEL_STP_UEVENT_LEN] = { NULL };

	if (g_kernel_stp_kobj == NULL) {
		kstp_log_debug(KSTP_LOG_TAG, "kernel stp kobj no creat");
		return KSTP_ERRCODE;
	}

	do {
		upload_info = kzalloc(STP_INFO_MAXLEN, GFP_KERNEL);
		if (upload_info == NULL) {
			kstp_log_error(KSTP_LOG_TAG, "failed to alloc upload_info");
			return -EINVAL;
		}

		ret = kernel_stp_upload_parse_cmd(result, addition_info, upload_info, cmd);
		if (ret != 0) {
			kstp_log_error(KSTP_LOG_TAG, "data parse failed, ret is %d", ret);
			break;
		}

		ret = kernel_stp_data_adapter(uevent_envp, upload_info);
		if (ret != 0) {
			kstp_log_error(KSTP_LOG_TAG, "data adpter failed, ret is %d", ret);
			break;
		}

		ret = kobject_uevent_env(g_kernel_stp_kobj, KOBJ_CHANGE,
					uevent_envp);
		if (ret != 0) {
			kstp_log_error(KSTP_LOG_TAG, "kobj upload failed, ret is %d", ret);
			break;
		}

		kstp_log_trace(KSTP_LOG_TAG, "event upload finished. result: %d", ret);
	} while (0);

	if (upload_info != NULL)
		kfree(upload_info);

	return ret;
}

static void get_current_time(char *buf, int length)
{
	struct rtc_time tm;
	struct timespec64 tv;

	ktime_get_real_ts64(&tv);
	tv.tv_sec -= sys_tz.tz_minuteswest * SECONDS_PER_MINUTE;
	rtc_time64_to_tm(tv.tv_sec, &tm);

	(void)snprintf_s(buf, length, length - 1,
		TIMESTAMP_FORMAT, tm.tm_year + YEAR_BASE, tm.tm_mon + 1,
		tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec);
}

static int event_info_to_json(const struct event_info *info, char *output, uint32_t length)
{
	uint8_t date[STP_DATE_LEN] = {0};

	get_current_time(date, sizeof(date));
	return snprintf_s(output, length, length - 1,
		"{\"eventId\":%llu,\"date\":\"%s\",\"version\":\"%s\",\"eventContent\":%s}",
		info->id, date, info->version, info->content);
}

int kernel_stp_upload(struct stp_item result, const char *addition_info)
{
	return kernel_stp_upload_cmd(result, addition_info, KERNEL_STP_UPLOAD);
}

int kernel_stp_kshield_upload(struct stp_item result, const char *addition_info)
{
	return kernel_stp_upload_cmd(result, addition_info, KERNEL_STP_KSHIELD_UPLOAD);
}

int kernel_stp_report_security_info(const struct event_info *info)
{
	int ret;
	struct stp_item result = {0};
	char *addition_info = NULL;

	if (info == NULL) {
		kstp_log_error(KSTP_LOG_TAG, "input arguments invalid");
		return -EINVAL;
	}

	addition_info = kzalloc(STP_INFO_MAXLEN, GFP_KERNEL);
	if (addition_info == NULL) {
		kstp_log_error(KSTP_LOG_TAG, "failed to alloc addition_info");
		return -EINVAL;
	}

	ret = event_info_to_json(info, addition_info, STP_INFO_MAXLEN);
	if (ret < 0) {
		kstp_log_error(KSTP_LOG_TAG, "event_info_to_json failed");
		kfree(addition_info);
		return ret;
	}

	ret = kernel_stp_upload_cmd(result, addition_info, KERNEL_STP_REPORT_SECURITY_INFO);
	kfree(addition_info);

	return ret;
}
