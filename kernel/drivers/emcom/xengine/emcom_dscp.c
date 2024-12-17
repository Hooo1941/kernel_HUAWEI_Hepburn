/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
 */
#include <net/sock.h>
#include <net/ip.h>
#include <linux/socket.h>
#include <linux/in.h>
#include <linux/in6.h>
#include <linux/netdevice.h>
#include <linux/spinlock.h>
#include <linux/sort.h>
#include <linux/bsearch.h>

#include "../emcom_netlink.h"
#include "../emcom_utils.h"
#include "emcom/emcom_xengine.h"
#include "securec.h"
#include "emcom/emcom_dscp.h"

#undef HWLOG_TAG
#define HWLOG_TAG emcom_xengine_dscp
HWLOG_REGIST();

static dscp_all_app_info g_dscp_all_app_info;
static spinlock_t g_dscp_all_app_info_lock;

static inline bool invalid_uid(uid_t uid)
{
	return (uid < UID_APP);
}

static inline uid_t get_root_uid(uid_t uid)
{
	return uid % ROOT_UID_MASK;
}

void emcom_xengine_dscp_init(void)
{
	emcom_logd("emcom dscp init");
	
	spin_lock_init(&g_dscp_all_app_info_lock);
	
	emcom_xengine_dscp_clear();

	return;
}

void emcom_xengine_dscp_clear(void)
{
	emcom_logd("emcom dscp clear");
	
	spin_lock_bh(&g_dscp_all_app_info_lock);

	g_dscp_all_app_info.app_sum = 0;
	for (int i = 0; i < DSCP_TOP_APP_MAX_NUM; i++) {
		g_dscp_all_app_info.allapp[i].uid = UID_APP;
		g_dscp_all_app_info.allapp[i].val = DSCP_INVALID_VALUE;
	}

	spin_unlock_bh(&g_dscp_all_app_info_lock);

	return;
}

static inline int cmp_func(const void *a, const void *b)
{
	return (((dscp_app_info*)a)->uid - ((dscp_app_info*)b)->uid);
}

void emcom_xengine_dscp_config_start(const char *pdata, uint16_t len)
{
	emcom_logd("emcom dscp config start");

	dscp_parse_start_info *startinfo = NULL;

	if (!pdata || len <= sizeof(dscp_parse_start_info)) {
		emcom_loge("dscp_config start data or length: %hu is error", len);
		return;
	}

	startinfo = (dscp_parse_start_info*)pdata;
	if (startinfo->app_sum > DSCP_TOP_APP_MAX_NUM) {
		emcom_loge("dscp_config start uid too many, app_sum = %u", startinfo->app_sum);
		return;
	}

	if (len < startinfo->app_sum * sizeof(dscp_app_info) + sizeof(uint32_t)) {
		emcom_loge("dscp_config start app_sum = %u length: %hu is error", startinfo->app_sum, len);
		return;
	}

	spin_lock_bh(&g_dscp_all_app_info_lock);
	
	dscp_app_info *appinfo = NULL;
	appinfo = (dscp_app_info*)startinfo->data;
	int cur = 0;
	int i = 0;
	
	while (i < DSCP_TOP_APP_MAX_NUM && cur < startinfo->app_sum) {
		uid_t uid = get_root_uid(appinfo[cur].uid);
		if (invalid_uid(uid)) {
			cur++;
			continue;
		}

		g_dscp_all_app_info.allapp[i].uid = uid;
		g_dscp_all_app_info.allapp[i].val = appinfo[cur].val;
		i++;
		cur++;
		g_dscp_all_app_info.app_sum++;
	}

	sort(g_dscp_all_app_info.allapp, g_dscp_all_app_info.app_sum, sizeof(dscp_app_info), cmp_func, NULL);

	spin_unlock_bh(&g_dscp_all_app_info_lock);

	return;
}

void emcom_xengine_dscp_add_app(uint32_t uid, int val)
{
	emcom_logd("emcom dscp update of add app");
	
	dscp_app_info key = {
		.uid = uid,
		.val = val
	};

	if (g_dscp_all_app_info.app_sum >= DSCP_TOP_APP_MAX_NUM) {
		emcom_loge("space full");
		return;
	}

	if (bsearch(&key, g_dscp_all_app_info.allapp, g_dscp_all_app_info.app_sum, sizeof(dscp_app_info), cmp_func)) {
		emcom_loge("duplicated uid %u", uid);
		return;
	}

	g_dscp_all_app_info.allapp[g_dscp_all_app_info.app_sum].uid = uid;
	g_dscp_all_app_info.allapp[g_dscp_all_app_info.app_sum].val = val;
	g_dscp_all_app_info.app_sum++;
	
	return;
}

void emcom_xengine_dscp_del_app(uid_t uid)
{
	emcom_logd("emcom dscp update of delete app");
	dscp_app_info key = {
		.uid = uid,
		.val = DSCP_INVALID_VALUE
	};

	dscp_app_info *rlt = bsearch(&key, g_dscp_all_app_info.allapp,
		g_dscp_all_app_info.app_sum, sizeof(dscp_app_info), cmp_func);

	if (rlt == NULL) {
		emcom_loge("uid not found:%u", uid);
		return;
	}
	
	int cur = rlt - g_dscp_all_app_info.allapp;
	int i;
	for (i = cur; i < g_dscp_all_app_info.app_sum -1; ++i) {
		g_dscp_all_app_info.allapp[i].uid = g_dscp_all_app_info.allapp[i + 1].uid;
		g_dscp_all_app_info.allapp[i].val = g_dscp_all_app_info.allapp[i + 1].val;
	}

	g_dscp_all_app_info.allapp[i].uid = UID_APP;
	g_dscp_all_app_info.allapp[i].val = DSCP_INVALID_VALUE;
	g_dscp_all_app_info.app_sum--;

	return;
}

void emcom_xengine_dscp_update_app_info(const char *pdata, uint16_t len)
{
	dscp_parse_update_app_info *update_app_info = NULL;
	uid_t uid = 0;

	emcom_logd("emcom dscp update app info");
	
	if (!pdata || len < sizeof(dscp_parse_update_app_info)) {
		emcom_loge("data or length %hu is error", len);
		return;
	}
	
	update_app_info = (dscp_parse_update_app_info*) pdata;
	uid = get_root_uid(update_app_info->uid);
	if (invalid_uid(uid)) {
		emcom_loge("invalid uid %u", update_app_info->uid);
		return;
	}

	spin_lock_bh(&g_dscp_all_app_info_lock);

	switch (update_app_info->operation) {
	case DSCP_APP_ADD:
		emcom_xengine_dscp_add_app(uid, update_app_info->val);
		break;

	case DSCP_APP_DEL:
		emcom_xengine_dscp_del_app(uid);
		break;

	default:
		break;
	}

	spin_unlock_bh(&g_dscp_all_app_info_lock);

	return;
}

void emcom_xengine_dscp_set_iptos(struct sock *sk)
{
	uid_t uid;

	if (g_dscp_all_app_info.app_sum == 0)
		return;

	uid = get_root_uid(sock_i_uid(sk).val);
	if (uid < UID_APP) {
		ip_sock_set_tos(sk, DSCP_SYS_TOS_VALUE);
		return;
	}

	if (uid < g_dscp_all_app_info.allapp[0].uid ||
		uid > g_dscp_all_app_info.allapp[g_dscp_all_app_info.app_sum - 1].uid)
		return;

	dscp_app_info key = {
		.uid = uid,
		.val = DSCP_INVALID_VALUE
	};
	dscp_app_info *rlt = (dscp_app_info*)bsearch(&key, g_dscp_all_app_info.allapp,
		g_dscp_all_app_info.app_sum, sizeof(dscp_app_info), cmp_func);
	
	if (rlt)
		ip_sock_set_tos(sk, rlt->val);

	return;
}

MODULE_LICENSE("GPL v2");
MODULE_DESCRIPTION("xengine module driver");
