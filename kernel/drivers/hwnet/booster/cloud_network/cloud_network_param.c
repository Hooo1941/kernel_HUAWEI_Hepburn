/*
 * cloud_network_param.c
 *
 * cloud network kernel module implementation
 *
 * Copyright (c) 2023-2023 Huawei Technologies Co., Ltd.
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

#include "cloud_network_param.h"
#include <linux/module.h>
#include <linux/init.h>
#include <linux/netdevice.h>
#include <linux/etherdevice.h>
#include <linux/platform_device.h>
#include <linux/mm.h>
#include <linux/device.h>
#include <linux/kernel.h>
#include <linux/list.h>
#include <linux/skbuff.h>
#include <linux/netlink.h>
#include <linux/moduleparam.h>
#include <net/sock.h>
#include <linux/delay.h>
#include <linux/mutex.h>
#include <linux/spinlock.h>
#include <linux/notifier.h>
#include <linux/reboot.h>
#include <linux/signal.h>
#include <linux/interrupt.h>
#include <securec.h>
#include <linux/sort.h>
#include <linux/version.h>
#include <linux/io.h>
#include <linux/types.h>
#include <linux/timer.h>
#include <linux/gfp.h>
#include <net/xfrm.h>
#include <linux/jhash.h>
#include <linux/rtnetlink.h>
#include <net/netfilter/nf_conntrack.h>
#include <net/netfilter/nf_conntrack_core.h>
#include <net/netfilter/nf_conntrack_helper.h>
#include <net/netfilter/nf_conntrack_seqadj.h>
#include <net/netfilter/nf_conntrack_zones.h>
#include <net/netfilter/nf_nat.h>
#include <net/netfilter/nf_nat_helper.h>
#include <uapi/linux/netfilter/nf_nat.h>

#include "cloud_network_utils.h"
#include "cloud_network.h"
#include "cloud_network_device.h"
#include "cloud_network_rx.h"
#include "cloud_network_signature.h"
#include "cloud_network_detect.h"
#include "cloud_network_session.h"
#include "cloud_network_tx.h"
#include "cloud_network_packet_match.h"
#include "cloud_network_option.h"

#define COMPASS_NORMAL 0
#define COMPASS_ABNORMAL 1
#define COMPASS_CHANGING_MS 5000
struct ims_param {
	struct ims_monitor config;
	bool status;
	s64 timestamp_recv; /* ms */
	spinlock_t lock;
};
struct compass_check {
	spinlock_t lock;
	struct compass_available_monitor config;
	bool front_app_compass_check_status;
	struct hrtimer front_app_compass_check_timer;
	struct work_struct front_app_compass_check_work;
	struct hrtimer unavailable_status_compass_check_timer;
	struct work_struct unavailable_status_compass_check_work;
	struct hrtimer data_flow_compass_check_timer;
	struct work_struct data_flow_compass_check_work;
	struct hrtimer detect_timer;
	struct work_struct detect_timeout_work;
};
static struct hrtimer compass_changing_timer;
static atomic_t compass_changing_state;
static atomic_t compass_available;

enum {
	DETECT_TIMEOUT = 0,
	CHECK_TIMEOUT = 1,
	FAST_CHECK_TIMEOUT = 2,
};

struct ims_param ims_mgr;
static struct compass_check check_mgr;

static atomic_t inited = ATOMIC_INIT(0);
static struct config_param *cfg = NULL; /* cloud network config param */
static struct status_param *status = NULL; /* cloud network status */

static void detect_stop(void);

bool is_compass_changing(void)
{
	return atomic_read(&compass_changing_state) == 1;
}

static enum hrtimer_restart compass_changing_timer_func(struct hrtimer *timer)
{
	atomic_set(&compass_changing_state, 0);
	return HRTIMER_NORESTART;
}

void set_compass_available()
{
	log_info("set compass ip is available");
	atomic_set(&compass_available, 1);
}

void set_compass_unavailable()
{
	log_info("set compass ip is unavailable");
	atomic_set(&compass_available, 0);
}

bool is_compass_available()
{
	return (atomic_read(&compass_available) != 0);
}

void set_data_flow_status_start()
{
	log_info("set data flow status started");
	atomic_set(&status->started, 1);
}

void set_data_flow_status_stop()
{
	log_info("set data flow status stoped");
	atomic_set(&status->started, 0);
}

bool get_data_flow_status()
{
	return (atomic_read(&status->started) != 0);
}

static bool is_compare_config_changed(void)
{
	bool ret;
	read_lock_bh(&(cfg->cfg_lock));
	ret = (cfg->compass.compass_ip != cfg->last_compass.compass_ip) ||
		(cfg->compass.compass_port != cfg->last_compass.compass_port);
	read_unlock_bh(&(cfg->cfg_lock));
	return ret;
}

static int backup_config_info(void)
{
	errno_t s_ret = EOK;

	if (unlikely(cfg == NULL))
		return ERROR;

	write_lock_bh(&(cfg->cfg_lock));
	log_info("set last ip");
	s_ret = memcpy_s(&(cfg->last_compass), sizeof(struct compass_info),
		&(cfg->compass), sizeof(struct compass_info));
	if (unlikely(s_ret != EOK))
		log_err("memcpy_s failed, error");
	write_unlock_bh(&(cfg->cfg_lock));

	return SUCCESS;
}

static int reset_last_config_info(void)
{
	if (unlikely(cfg == NULL))
		return ERROR;

	write_lock_bh(&(cfg->cfg_lock));
	(void)memset_s(&(cfg->last_compass), sizeof(struct compass_info), 0x00,
		sizeof(struct compass_info));
	write_unlock_bh(&(cfg->cfg_lock));

	return SUCCESS;
}

static int config_compass_infor(struct config_param *cfg,
	const struct compass_info *compass)
{
	errno_t s_ret = EOK;

	if (unlikely(cfg == NULL || compass == NULL))
		return ERROR;

	log_info("client id %02X %02X %02X %02X %02X %02X", compass->client_id[0],
		compass->client_id[1], compass->client_id[2], compass->client_id[3],
		compass->client_id[4], compass->client_id[5]);
	log_info("app id %02X %02X %02X", compass->app_id[0], compass->app_id[1],
		compass->app_id[2]);

	write_lock_bh(&(cfg->cfg_lock));
	s_ret = memcpy_s(&(cfg->compass), sizeof(struct compass_info), compass,
		sizeof(struct compass_info));
	write_unlock_bh(&(cfg->cfg_lock));
	if (unlikely(s_ret != EOK))
		return ERROR;

	return SUCCESS;
}

static int config_key_infor(struct config_param *cfg,
	const struct key_info *key)
{
	errno_t s_ret = EOK;

	if (unlikely(cfg == NULL || key == NULL))
		return ERROR;

	log_info("hmac id %d", key->hmac_id);

	write_lock_bh(&(cfg->cfg_lock));
	s_ret = memcpy_s(&(cfg->key), sizeof(struct key_info), key,
		sizeof(struct key_info));
	write_unlock_bh(&(cfg->cfg_lock));
	if (unlikely(s_ret != EOK))
		return ERROR;

	return SUCCESS;
}

static int config_link_infor(struct config_param *cfg,
	const struct link_info *link)
{
	errno_t s_ret = EOK;

	if (unlikely(cfg == NULL || link == NULL))
		return ERROR;

	log_info("pri_modem %02X sec_modem %02X pri_wifi %02X scene %02X",
		link->pri_modem_permit, link->sec_modem_permit, link->pri_wifi_permit,
		link->scene);

	write_lock_bh(&(cfg->cfg_lock));
	s_ret = memcpy_s(&(cfg->link), sizeof(struct link_info), link, sizeof(struct link_info));
	write_unlock_bh(&(cfg->cfg_lock));
	if (unlikely(s_ret != EOK))
		return ERROR;

	return SUCCESS;
}

static struct app_node *app_collect_search(struct config_param *cfg,
	const u32 uid)
{
	struct rb_node *node = NULL;

	if (unlikely(cfg == NULL))
		return NULL;

	node = cfg->app_collect.rb_node;
	while (node != NULL) {
		struct app_node *data = rb_entry(node, struct app_node,
			app_rb_node);
		if (uid < data->app.uid) {
			node = node->rb_left;
		} else if (uid > data->app.uid) {
			node = node->rb_right;
		} else {
			return data;
		}
	}
	return NULL;
}

static void app_collect_delete(struct config_param *cfg,
	const u32 uid)
{
	struct app_node *entry = NULL;

	if (unlikely(cfg == NULL))
		return;

	write_lock_bh(&(cfg->app_collect_lock));
	entry = app_collect_search(cfg, uid);
	if (entry != NULL) {
		rb_erase(&entry->app_rb_node, &(cfg->app_collect));
		kfree(entry);
	}
	write_unlock_bh(&(cfg->app_collect_lock));
	return;
}

static void app_collect_insert(struct app_node *data,
	struct config_param *cfg)
{
	struct rb_node **insert = NULL;
	struct rb_node *parent = NULL;

	if (unlikely(data == NULL || cfg == NULL))
		return;

	write_lock_bh(&(cfg->app_collect_lock));
	insert = &(cfg->app_collect.rb_node);
	while (*insert != NULL) {
		struct app_node *this = rb_entry(*insert, struct app_node,
			app_rb_node);
		parent = *insert;
		if (data->app.uid < this->app.uid) {
			insert = &((*insert)->rb_left);
		} else if (data->app.uid > this->app.uid) {
			insert = &((*insert)->rb_right);
		} else {
			write_unlock_bh(&(cfg->app_collect_lock));
			log_err("uid %d has already inserted, error", data->app.uid);
			return;
		}
	}
	rb_link_node(&data->app_rb_node, parent, insert);
	log_info("app collect insert uid %d", data->app.uid);
	rb_insert_color(&data->app_rb_node, &(cfg->app_collect));
	write_unlock_bh(&(cfg->app_collect_lock));
	return;
}

static void app_collect_clear(struct config_param *cfg)
{
	struct rb_node *node = NULL;
	struct app_node *data = NULL;

	if (unlikely(cfg == NULL))
		return;

	write_lock_bh(&(cfg->app_collect_lock));
	while (node = rb_first(&(cfg->app_collect))) {
		data = rb_entry(node, struct app_node, app_rb_node);
		log_info("erase app uid %p", data->app.uid);
		rb_erase(&data->app_rb_node, &(cfg->app_collect));
		kfree(data);
	}
	cfg->app_collect = RB_ROOT;
	write_unlock_bh(&(cfg->app_collect_lock));
	return;
}

static int config_app_collect_infor(struct config_param *cfg,
	const struct app_collect *app)
{
	errno_t s_ret = EOK;
	u32 app_num = 0;
	struct app_info *app_entry = NULL;
	struct app_node *app_insert = NULL;

	if (unlikely(app == NULL || cfg == NULL))
		return ERROR;

	app_collect_clear(cfg);
	app_num = app->data_len / sizeof(struct app_info);
	if (app_num == 0)
		return SUCCESS;

	for (int i = 0; i < app_num; i++) {
		app_entry = (struct app_info *)(app->data + sizeof(struct app_info) * i);
		if (app_entry->uid < 0)
			continue;
		app_insert = kzalloc(sizeof(struct app_node), GFP_ATOMIC);
		if (app_insert == NULL) {
			log_err("alloc app_entry fail");
			return ERROR;
		}
		s_ret = memcpy_s(&(app_insert->app), sizeof(struct app_info), app_entry,
			sizeof(struct app_info));
		if (s_ret != EOK)
			return ERROR;
		app_collect_insert(app_insert, cfg);
	}
	return SUCCESS;
}

int cloud_network_data_prosess_init(void)
{
	int ret = ERROR;
	struct hw_cloud_network_ops ops;

	ret = cloud_network_rx_init();
	ops.rx_handle = cloud_network_rx;
	ops.tx_handle = cloud_network_tx;
	ret += cloud_network_op_register(&ops);
	cloud_network_register_hook();
	return ret;
}

void cloud_network_data_prosess_deinit(void)
{
	cloud_network_op_unregister();
	cloud_network_unregister_hook();
	cloud_network_data_rx_exit();
	return;
}

void netlink_cmd_cloud_param_key_handle(const struct netlink_msg_head *msg)
{
	struct recv_cloud_param_key *data = NULL;

	if (unlikely(msg == NULL))
		return;

	if (msg->data_len < (sizeof(struct netlink_msg_head)
		+ sizeof(struct recv_cloud_param_key))
		|| msg->data_len > NETLINK_DATA_MSG_LEN) {
		log_err("msg length error");
		return;
	}

	data = (struct recv_cloud_param_key *)(msg->data);
	if (config_key_infor(cfg, &(data->key)) != SUCCESS) {
		log_err("config key infor error");
		return;
	}
	cloud_network_set_key(cfg->key.hmac_key, SIGN_KEY_LEN, cfg->key.hmac_id);
	return;
}

static void start_compass_changing_timer(void)
{
	atomic_set(&compass_changing_state, 1);
	if (!hrtimer_active(&compass_changing_timer))
		hrtimer_start(&compass_changing_timer,
			ms_to_ktime(COMPASS_CHANGING_MS), HRTIMER_MODE_REL);
	all_session_window_reset();
	return;
}

void netlink_cmd_cloud_param_compass_handle(const struct netlink_msg_head *msg)
{
	struct recv_cloud_param_compass *data = NULL;

	if (unlikely(msg == NULL))
		return;

	if (msg->data_len < (sizeof(struct netlink_msg_head)
		+ sizeof(struct recv_cloud_param_compass))
		|| msg->data_len > NETLINK_DATA_MSG_LEN) {
		log_err("msg length error");
		return;
	}

	data = (struct recv_cloud_param_compass *)(msg->data);
	if (backup_config_info() != SUCCESS)
		log_err("backup config compass infor error");

	if (config_compass_infor(cfg, &(data->compass)) != SUCCESS) {
		log_err("config compass infor error");
		atomic_set(&(cfg->enable), CONFIG_DISABLE);
		return;
	}
	if (is_compare_config_changed())
		start_compass_changing_timer();
	set_compass_available();
	compass_check_once();
	return;
}

void netlink_cmd_cloud_param_app_handle(const struct netlink_msg_head *msg)
{
	struct recv_cloud_param_app *data = NULL;

	if (unlikely(msg == NULL))
		return;

	if (msg->data_len < (sizeof(struct netlink_msg_head)
		+ sizeof(struct recv_cloud_param_app))
		|| msg->data_len > NETLINK_DATA_MSG_LEN) {
		log_err("msg length error");
		return;
	}

	data = (struct recv_cloud_param_app *)(msg->data);
	if (config_app_collect_infor(cfg, &(data->app)) != SUCCESS) {
		log_err("config app collect infor error");
		atomic_set(&(cfg->enable), CONFIG_DISABLE);
		return;
	}
	return;
}

void netlink_cmd_cloud_param_link_handle(const struct netlink_msg_head *msg)
{
	struct recv_cloud_param_link *data = NULL;

	if (unlikely(msg == NULL))
		return;

	if (msg->data_len < (sizeof(struct netlink_msg_head)
		+ sizeof(struct recv_cloud_param_link))
		|| msg->data_len > NETLINK_DATA_MSG_LEN) {
		log_err("msg length error");
		return;
	}

	data = (struct recv_cloud_param_link *)(msg->data);
	if (config_link_infor(cfg, &(data->link)) != SUCCESS) {
		log_err("config link infor error");
		atomic_set(&(cfg->enable), CONFIG_DISABLE);
		return;
	}
	return;
}

void netlink_cmd_cloud_ims_monitor_start_handle(const struct netlink_msg_head *msg)
{
	struct recv_ims_monitor_start *data = NULL;

	if (unlikely(msg == NULL))
		return;

	if (unlikely(atomic_read(&(cfg->enable)) == CONFIG_DISABLE)) {
		log_err("cloud network has no inited, ignore this cmd");
		return;
	}

	if (msg->data_len < (sizeof(struct netlink_msg_head)
		+ sizeof(struct recv_ims_monitor_start))
		|| msg->data_len > NETLINK_DATA_MSG_LEN) {
		log_err("msg length error");
		return;
	}

	data = (struct recv_ims_monitor_start *)(msg->data);
	spin_lock_bh(&ims_mgr.lock);
	log_info("uid %d exception_timeout %d", data->ims_monitor_param.uid,
		data->ims_monitor_param.exception_timeout);
	ims_mgr.config.uid = data->ims_monitor_param.uid;
	ims_mgr.config.exception_timeout = data->ims_monitor_param.exception_timeout;
	ims_mgr.timestamp_recv = 0;
	ims_mgr.status = true;
	spin_unlock_bh(&ims_mgr.lock);
	start_gain_detection();
	return;
}

void netlink_cmd_cloud_ims_monitor_stop_handle(const struct netlink_msg_head *msg)
{
	if (unlikely(msg == NULL))
		return;

	if (unlikely(atomic_read(&(cfg->enable)) == CONFIG_DISABLE)) {
		log_err("cloud network has no inited, ignore this cmd");
		return;
	}
	spin_lock_bh(&ims_mgr.lock);
	ims_mgr.status = false;
	ims_mgr.timestamp_recv = 0;
	spin_unlock_bh(&ims_mgr.lock);
	stop_gain_detection();
}

void netlink_cmd_cloud_compass_monitor_start_handle(const struct netlink_msg_head *msg)
{
	errno_t s_ret = EOK;
	struct recv_compass_available_monitor_start *data = NULL;

	if (unlikely(msg == NULL))
		return;

	if (unlikely(atomic_read(&(cfg->enable)) == CONFIG_DISABLE)) {
		log_err("cloud network has no inited, ignore this cmd");
		return;
	}

	if (msg->data_len < (sizeof(struct netlink_msg_head)
		+ sizeof(struct recv_compass_available_monitor_start))
		|| msg->data_len > NETLINK_DATA_MSG_LEN) {
		log_err("msg length error");
		return;
	}

	data = (struct recv_compass_available_monitor_start *)(msg->data);
	spin_lock_bh(&check_mgr.lock);
	s_ret = memcpy_s(&(check_mgr.config),
		sizeof(struct compass_available_monitor), data,
		sizeof(struct compass_available_monitor));
	if (s_ret != EOK) {
		log_err("memcpy_s failed");
		spin_unlock_bh(&check_mgr.lock);
		return;
	}
	log_info("comfig param %u %u %u %u %u", check_mgr.config.poll_timeout,
		check_mgr.config.detect_interval, check_mgr.config.exception_timeout,
		check_mgr.config.exception_poll_timeout,
		check_mgr.config.data_poll_timeout);
	spin_unlock_bh(&check_mgr.lock);
	front_app_compass_check_start();
}

void netlink_cmd_cloud_compass_monitor_stop_handle(const struct netlink_msg_head *msg)
{
	if (unlikely(msg == NULL))
		return;

	if (unlikely(atomic_read(&(cfg->enable)) == CONFIG_DISABLE)) {
		log_err("cloud network has not inited, ignore this cmd");
		return;
	}
	front_app_compass_check_stop();
}

void netlink_cmd_deinit_param_handle(const struct netlink_msg_head *msg)
{
	if (unlikely(msg == NULL ||
		atomic_read(&inited) == CLOUD_NETWORK_DEINIED ||
		atomic_read(&(cfg->enable)) == CONFIG_DISABLE))
		return;

	atomic_set(&(cfg->enable), CONFIG_DISABLE);
	cloud_network_data_prosess_deinit();

	set_tx_rx_state(TX_RX_STATE_2T2R);
	front_app_compass_check_stop();
	spin_lock_bh(&ims_mgr.lock);
	ims_mgr.status = false;
	ims_mgr.timestamp_recv = 0;
	spin_unlock_bh(&ims_mgr.lock);

	write_lock_bh(&(cfg->cfg_lock));
	(void)memset_s(&(cfg->compass), sizeof(struct compass_info), 0x00,
		sizeof(struct compass_info));
	(void)memset_s(&(cfg->key), sizeof(struct key_info), 0x00,
		sizeof(struct key_info));
	(void)memset_s(&(cfg->link), sizeof(struct link_info), 0x00,
		sizeof(struct link_info));
	write_unlock_bh(&(cfg->cfg_lock));

	app_collect_clear(cfg);
	cloud_network_option_stop();
	cloud_network_session_reset();
	cloud_network_detect_reset();
	ims_check_exit();
	compass_check_exit();
	cloud_network_sign_deinit();
	return;
}

int netlink_cmd_init_param_handle(const struct netlink_msg_head *msg)
{
	struct recv_init *init_data = NULL;
	int ret = ERROR;

	if (unlikely(msg == NULL || atomic_read(&inited) == CLOUD_NETWORK_DEINIED))
		return ERROR;

	if (likely(atomic_read(&(cfg->enable)) != CONFIG_DISABLE)) {
		log_err("cloud network has inited, ignore this init param");
		return SUCCESS;
	}

	if (msg->data_len < (sizeof(struct netlink_msg_head)
		+ sizeof(struct recv_init)) || msg->data_len > NETLINK_DATA_MSG_LEN) {
		log_err("msg length error");
		return ERROR;
	}

	init_data = (struct recv_init *)(msg->data);
	if (reset_last_config_info() != SUCCESS)
		log_err("reset config last compass infor error");

	if (config_compass_infor(cfg, &(init_data->compass)) != SUCCESS) {
		log_err("config compass infor error");
		return ERROR;
	}
	if (config_key_infor(cfg, &(init_data->key)) != SUCCESS) {
		log_err("config key infor error");
		return ERROR;
	}
	if (config_link_infor(cfg, &(init_data->link)) != SUCCESS) {
		log_err("config link infor error");
		return ERROR;
	}
	if (config_app_collect_infor(cfg, &(init_data->app)) != SUCCESS) {
		log_err("config app collect infor error");
		return ERROR;
	}
	set_tx_rx_state(TX_RX_STATE_2T2R);

	ret = ims_check_init();
	ret += compass_check_init();
	send_reset_option();
	cloud_network_sign_init();
	cloud_network_set_key(cfg->key.hmac_key, SIGN_KEY_LEN, cfg->key.hmac_id);
	ret += cloud_network_data_prosess_init();
	atomic_set(&(cfg->enable), CONFIG_ENABLE);
	compass_check_once();
	return ret;
}

void netlink_cmd_tx_rx_ctl_handle(const struct netlink_msg_head *msg)
{
	struct recv_tx_rx_ctl *tx_rx_ctl = NULL;

	if (unlikely(msg == NULL))
		return;

	if (msg->data_len < (sizeof(struct netlink_msg_head)
		+ sizeof(struct recv_tx_rx_ctl)) ||
		msg->data_len > NETLINK_DATA_MSG_LEN) {
		log_err("msg length error");
		return;
	}

	tx_rx_ctl = (struct recv_tx_rx_ctl *)(msg->data);
	log_info("TX_RX control state %d", tx_rx_ctl->tx_rx.state);
	if (tx_rx_ctl->tx_rx.state == TX_RX_STATE_2T2R)
		start_gain_detection();
	else
		stop_gain_detection();

	set_tx_rx_state(tx_rx_ctl->tx_rx.state);
	return;
}

void netlink_cmd_cloud_param_all_handle(const struct netlink_msg_head *msg)
{
	struct recv_cloud_param_all *data = NULL;

	if (unlikely(msg == NULL))
		return;

	if (msg->data_len < (sizeof(struct netlink_msg_head)
		+ sizeof(struct recv_cloud_param_all)) ||
		msg->data_len > NETLINK_DATA_MSG_LEN) {
		log_err("msg length error");
		return;
	}

	data = (struct recv_cloud_param_all *)(msg->data);

	if (backup_config_info() != SUCCESS)
		log_err("backup config compass infor error");

	if (config_compass_infor(cfg, &(data->compass)) != SUCCESS) {
		log_err("config compass infor error");
		atomic_set(&(cfg->enable), CONFIG_DISABLE);
		return;
	}
	if (config_key_infor(cfg, &(data->key)) != SUCCESS) {
		log_err("config key infor error");
		return;
	}
	if (config_app_collect_infor(cfg, &(data->app)) != SUCCESS) {
		log_err("config app collect infor error");
		atomic_set(&(cfg->enable), CONFIG_DISABLE);
		return;
	}
	if (is_compare_config_changed())
		start_compass_changing_timer();
	set_compass_available();
	cloud_network_set_key(cfg->key.hmac_key, SIGN_KEY_LEN, cfg->key.hmac_id);
	compass_check_once();
	return;
}

bool skb_uid_match(u32 uid, const struct sk_buff *skb)
{
	struct app_node *entry = NULL;
	bool ret = false;

	if (atomic_read(&inited) == CLOUD_NETWORK_DEINIED ||
		atomic_read(&(cfg->enable)) == CONFIG_DISABLE || skb == NULL)
		return false;

	read_lock_bh(&(cfg->app_collect_lock));
	entry = app_collect_search(cfg, uid);
	if (entry != NULL)
		ret = true;
	read_unlock_bh(&(cfg->app_collect_lock));
	return ret;
}

bool is_cloud_network_enable(void)
{
	if (atomic_read(&inited) == CLOUD_NETWORK_DEINIED || atomic_read(&(cfg->enable)) == CONFIG_DISABLE)
		return false;

	return true;
}

int get_protocol_info(struct protocol_info *proto)
{
	errno_t err;

	if (atomic_read(&inited) == CLOUD_NETWORK_DEINIED ||
		atomic_read(&(cfg->enable)) == CONFIG_DISABLE || proto == NULL)
		return ERROR;

	read_lock_bh(&(cfg->cfg_lock));
	proto->compass_ip = cfg->compass.compass_ip;
	proto->compass_port = cfg->compass.compass_port;
	err = memcpy_s(proto->client_id, sizeof(proto->client_id),
		cfg->compass.client_id, sizeof(proto->client_id));
	if (err != EOK)
		return ERROR;

	err = memcpy_s(proto->app_id, sizeof(proto->app_id),
		cfg->compass.app_id, sizeof(proto->app_id));
	if (err != EOK)
		return ERROR;

	proto->hmac_id = cfg->key.hmac_id;
	read_unlock_bh(&(cfg->cfg_lock));
	return SUCCESS;
}

int get_link_info(struct link_info *link_info_data)
{
	if (atomic_read(&inited) == CLOUD_NETWORK_DEINIED ||
		atomic_read(&(cfg->enable)) == CONFIG_DISABLE)
		return ERROR;

	if (unlikely(link_info_data == NULL))
		return ERROR;

	read_lock_bh(&(cfg->cfg_lock));
	link_info_data->pri_modem_permit = cfg->link.pri_modem_permit;
	link_info_data->sec_modem_permit = cfg->link.sec_modem_permit;
	link_info_data->pri_wifi_permit = cfg->link.pri_wifi_permit;
	link_info_data->scene = cfg->link.scene;
	read_unlock_bh(&(cfg->cfg_lock));
	return SUCCESS;
}

bool protocol_info_match(__be32 compass_ip, __be16 compass_port)
{
	if (atomic_read(&inited) == CLOUD_NETWORK_DEINIED ||
		atomic_read(&(cfg->enable)) == CONFIG_DISABLE)
		return false;

	read_lock_bh(&(cfg->cfg_lock));
	if (((cfg->compass.compass_ip == compass_ip) &&
		(cfg->compass.compass_port == ntohs(compass_port))) ||
		((cfg->last_compass.compass_ip == compass_ip) &&
		(cfg->last_compass.compass_port == ntohs(compass_port)))) {
		read_unlock_bh(&(cfg->cfg_lock));
		return true;
	}
	read_unlock_bh(&(cfg->cfg_lock));
	return false;
}

int cloud_network_param_init(void)
{
	if (likely(cfg == NULL)) {
		log_info("cloud network config param malloc");
		cfg = (struct config_param *)kmalloc(sizeof(struct config_param), GFP_ATOMIC);
		if (unlikely(cfg == NULL)) {
			log_err("kmalloc failed");
			atomic_set(&inited, CLOUD_NETWORK_DEINIED);
			return ERROR;
		}
		atomic_set(&(cfg->enable), CONFIG_DISABLE);
		rwlock_init(&(cfg->cfg_lock));
		rwlock_init(&(cfg->app_collect_lock));
		write_lock_bh(&(cfg->app_collect_lock));
		cfg->app_collect = RB_ROOT;
		write_unlock_bh(&(cfg->app_collect_lock));
	} else {
		log_info("cloud network config param has malloc");
		atomic_set(&(cfg->enable), CONFIG_DISABLE);
		app_collect_clear(cfg);
	}

	write_lock_bh(&(cfg->cfg_lock));
	(void)memset_s(&(cfg->compass), sizeof(struct compass_info), 0x00,
		sizeof(struct compass_info));
	write_unlock_bh(&(cfg->cfg_lock));

	if (likely(status == NULL)) {
		log_info("cloud network status param init");
		status = (struct status_param *)kmalloc(sizeof(struct status_param), GFP_ATOMIC);
		if (unlikely(status == NULL)) {
			log_err("kmalloc failed");
			atomic_set(&inited, CLOUD_NETWORK_DEINIED);
			return ERROR;
		}
	}
	atomic_set(&(status->started), STATUS_STOPED);
	atomic_set(&inited, CLOUD_NETWORK_INITED);
	atomic_set(&compass_changing_state, 0);
	hrtimer_init(&compass_changing_timer, CLOCK_MONOTONIC,
		HRTIMER_MODE_REL);
	compass_changing_timer.function = compass_changing_timer_func;

	return SUCCESS;
}

void cloud_network_param_exit(void)
{
	atomic_set(&inited, CLOUD_NETWORK_DEINIED);
	if (likely(cfg != NULL)) {
		log_err("config param exit");
		app_collect_clear(cfg);
		kfree(cfg);
		cfg = NULL;
	}
	if (likely(status != NULL)) {
		log_err("status param exit");
		kfree(status);
		status = NULL;
	}
	return;
}

static int compass_status_report(int status)
{
	struct netlink_head *msg_rsp = NULL;
	struct netlink_msg_head *msg_header = NULL;
	struct rept_compass_status *msg_rept = NULL;
	struct protocol_info info;
	get_protocol_info(&info);

	msg_rsp = (struct netlink_head *)kmalloc(sizeof(struct netlink_head) +
		sizeof(struct netlink_msg_head) +
		sizeof(struct rept_compass_status), GFP_ATOMIC);
	if (unlikely(msg_rsp == NULL)) {
		log_err("kmalloc failed");
		return ERROR;
	}

	msg_rsp->type = CLOUD_NETWORK_REPORT;
	msg_rsp->msg_len = sizeof(struct netlink_head) +
		sizeof(struct netlink_msg_head) +
		sizeof(struct rept_compass_status);

	msg_header = (struct netlink_msg_head *)(msg_rsp->data);

	msg_header->type = COMPASS_ABNORMAL;
	msg_header->data_len = sizeof(struct netlink_msg_head) +
		sizeof(struct rept_compass_status);

	msg_rept = (struct rept_compass_status *)(msg_header->data);
	msg_rept->status = status;
	msg_rept->compass_ip = info.compass_ip;
	log_info("event report compass status %u", msg_rept->status);
	netlink_report(msg_rsp, WORK_EXEC);
	kfree(msg_rsp);
	return SUCCESS;
}

static void compass_detect_callback(int path, u32 rtt, int average_rtt,
	int loss_rate)
{
	log_info("detect prob RTT %u", rtt);
	if (!is_cloud_network_enable())
		return;

	detect_stop();
	if (!is_compass_available())
		compass_status_report(COMPASS_NORMAL);
	set_compass_available();
	spin_lock_bh(&check_mgr.lock);
	hrtimer_cancel(&check_mgr.detect_timer);
	hrtimer_cancel(&check_mgr.unavailable_status_compass_check_timer);
	if (check_mgr.front_app_compass_check_status &&
		!hrtimer_active(&check_mgr.front_app_compass_check_timer))
		hrtimer_start(&check_mgr.front_app_compass_check_timer,
			ms_to_ktime(check_mgr.config.poll_timeout), HRTIMER_MODE_REL);

	if (get_data_flow_status() &&
		!hrtimer_active(&check_mgr.data_flow_compass_check_timer))
		hrtimer_start(&check_mgr.data_flow_compass_check_timer,
			ms_to_ktime(check_mgr.config.data_poll_timeout), HRTIMER_MODE_REL);
	spin_unlock_bh(&check_mgr.lock);
}

static void detect_start()
{
	log_info("detect start");
	if (unlikely(!is_cloud_network_enable()))
		return;
	detect_module_start(COMPASS_CHECK_ID, check_mgr.config.detect_interval,
		DETECT_PATH_BIT_MODEM_1 | DETECT_PATH_BIT_MODEM_2,
		compass_detect_callback);
	if (!hrtimer_active(&check_mgr.detect_timer))
		hrtimer_start(&check_mgr.detect_timer,
			ms_to_ktime(check_mgr.config.exception_timeout), HRTIMER_MODE_REL);
}

static void detect_stop()
{
	log_info("detect stop");
	detect_module_stop(COMPASS_CHECK_ID);
	return;
}

void compass_check_once()
{
	log_info("compass check once");
	spin_lock_bh(&check_mgr.lock);
	detect_start();
	spin_unlock_bh(&check_mgr.lock);
}

void front_app_compass_check_start()
{
	log_info("front app compass check start");
	spin_lock_bh(&check_mgr.lock);
	check_mgr.front_app_compass_check_status = true;
	if (!hrtimer_active(&check_mgr.front_app_compass_check_timer))
		hrtimer_start(&check_mgr.front_app_compass_check_timer,
			ms_to_ktime(check_mgr.config.poll_timeout), HRTIMER_MODE_REL);

	detect_start();
	spin_unlock_bh(&check_mgr.lock);
}

void front_app_compass_check_stop()
{
	log_info("front app compass check stop");
	spin_lock_bh(&check_mgr.lock);
	check_mgr.front_app_compass_check_status = false;
	hrtimer_cancel(&check_mgr.front_app_compass_check_timer);
	spin_unlock_bh(&check_mgr.lock);
}

void data_flow_compass_check_start()
{
	log_info("data flow compass check start");
	spin_lock_bh(&check_mgr.lock);
	if (!hrtimer_active(&check_mgr.data_flow_compass_check_timer))
		hrtimer_start(&check_mgr.data_flow_compass_check_timer,
			ms_to_ktime(check_mgr.config.data_poll_timeout), HRTIMER_MODE_REL);
	spin_unlock_bh(&check_mgr.lock);
}

static enum hrtimer_restart front_app_compass_check_timeout(struct hrtimer *timer)
{
	if (is_cloud_network_enable())
		schedule_work(&check_mgr.front_app_compass_check_work);
	return HRTIMER_NORESTART;
}

static enum hrtimer_restart unavailable_status_compass_check_timeout(struct hrtimer *timer)
{
	if (is_cloud_network_enable())
		schedule_work(&check_mgr.unavailable_status_compass_check_work);
	return HRTIMER_NORESTART;
}

static enum hrtimer_restart data_flow_compass_check_timer_timeout(struct hrtimer *timer)
{
	if (is_cloud_network_enable())
		schedule_work(&check_mgr.data_flow_compass_check_work);
	return HRTIMER_NORESTART;
}

static enum hrtimer_restart compass_detect_timeout(struct hrtimer *timer)
{
	if (is_cloud_network_enable())
		schedule_work(&check_mgr.detect_timeout_work);
	return HRTIMER_NORESTART;
}

static void front_app_compass_check_handle(struct work_struct *work)
{
	log_info("front app compass check timer handle");
	if (!is_cloud_network_enable())
		return;
	spin_lock_bh(&check_mgr.lock);
	detect_start();
	spin_unlock_bh(&check_mgr.lock);
}

static void unavailable_status_compass_check_handle(struct work_struct *work)
{
	log_info("unavailable status compass check timer handle");
	if (!is_cloud_network_enable())
		return;
	spin_lock_bh(&check_mgr.lock);
	detect_start();
	spin_unlock_bh(&check_mgr.lock);
}

static void data_flow_compass_check_timer_handle(struct work_struct *work)
{
	log_info("data flow compass check timer handle");
	if (!is_cloud_network_enable())
		return;
	spin_lock_bh(&check_mgr.lock);
	detect_start();
	spin_unlock_bh(&check_mgr.lock);
}

static bool compass_unavailable_condition(void)
{
	struct link_info link_info_data;

	if (get_link_info(&link_info_data) != SUCCESS)
		return false;
	if ((link_info_data.scene == SCENE_MOEM_MODEM) && /* modem + modem scene */
		((get_link_path() & (PATH_MODEM_1 | PATH_MODEM_2)) == 0))
		return false;

	return true;
}

static void compass_detect_timeout_handle(struct work_struct *work)
{
	log_info("compass detect timeout, compass unavailable handle");
	if (!is_cloud_network_enable())
		return;
	if (compass_unavailable_condition() && is_compass_available()) {
		set_compass_unavailable();
		cloud_network_session_reset();
		compass_status_report(COMPASS_ABNORMAL);
	}
	spin_lock_bh(&check_mgr.lock);
	detect_stop();
	if (!hrtimer_active(&check_mgr.unavailable_status_compass_check_timer))
		hrtimer_start(&check_mgr.unavailable_status_compass_check_timer,
			ms_to_ktime(check_mgr.config.exception_poll_timeout),
				HRTIMER_MODE_REL);
	spin_unlock_bh(&check_mgr.lock);
	ims_check_reset();
}

void stop_all_compass_detect(void)
{
	log_info("stop all compass deetect");
	cancel_work_sync(&(check_mgr.front_app_compass_check_work));
	cancel_work_sync(&(check_mgr.unavailable_status_compass_check_work));
	cancel_work_sync(&(check_mgr.data_flow_compass_check_work));
	cancel_work_sync(&(check_mgr.detect_timeout_work));
	hrtimer_cancel(&check_mgr.front_app_compass_check_timer);
	hrtimer_cancel(&check_mgr.unavailable_status_compass_check_timer);
	hrtimer_cancel(&check_mgr.data_flow_compass_check_timer);
	hrtimer_cancel(&check_mgr.detect_timer);
	return;
}

void start_compass_detect_on_demand(void)
{
	log_info("start compass detect on demand");
	if (check_mgr.front_app_compass_check_status &&
		!hrtimer_active(&check_mgr.front_app_compass_check_timer))
		hrtimer_start(&check_mgr.front_app_compass_check_timer,
			ms_to_ktime(check_mgr.config.poll_timeout), HRTIMER_MODE_REL);

	if (get_data_flow_status() &&
		!hrtimer_active(&check_mgr.data_flow_compass_check_timer))
		hrtimer_start(&check_mgr.data_flow_compass_check_timer,
			ms_to_ktime(check_mgr.config.data_poll_timeout), HRTIMER_MODE_REL);
	return;
}

int compass_check_init()
{
	check_mgr.config.poll_timeout = FRONT_APP_COMPASS_CHECK_INTERVAL;
	check_mgr.config.detect_interval = COMPASS_DETECT_INTERVAL;
	check_mgr.config.exception_timeout = COMPASS_DETECT_TIMEOUT;
	check_mgr.config.exception_poll_timeout =
		UNAVAILABLE_STATUS_COMPASS_CHECK_INTERVAL;
	check_mgr.config.data_poll_timeout = DATA_FLOW_COMPASS_CHECK_INTERVAL;

	check_mgr.front_app_compass_check_status = false;
	set_data_flow_status_stop();
	set_compass_available();

	INIT_WORK(&(check_mgr.front_app_compass_check_work),
		front_app_compass_check_handle);
	INIT_WORK(&(check_mgr.unavailable_status_compass_check_work),
		unavailable_status_compass_check_handle);
	INIT_WORK(&(check_mgr.data_flow_compass_check_work),
		data_flow_compass_check_timer_handle);
	INIT_WORK(&(check_mgr.detect_timeout_work), compass_detect_timeout_handle);

	hrtimer_init(&check_mgr.front_app_compass_check_timer, CLOCK_MONOTONIC,
		HRTIMER_MODE_REL);
	check_mgr.front_app_compass_check_timer.function =
		front_app_compass_check_timeout;
	hrtimer_init(&check_mgr.unavailable_status_compass_check_timer,
		CLOCK_MONOTONIC, HRTIMER_MODE_REL);
	check_mgr.unavailable_status_compass_check_timer.function =
		unavailable_status_compass_check_timeout;
	hrtimer_init(&check_mgr.data_flow_compass_check_timer,
		CLOCK_MONOTONIC, HRTIMER_MODE_REL);
	check_mgr.data_flow_compass_check_timer.function =
		data_flow_compass_check_timer_timeout;
	hrtimer_init(&check_mgr.detect_timer, CLOCK_MONOTONIC, HRTIMER_MODE_REL);
	check_mgr.detect_timer.function = compass_detect_timeout;
	spin_lock_init(&check_mgr.lock);
	return SUCCESS;
}

void compass_check_exit()
{
	spin_lock_bh(&check_mgr.lock);
	check_mgr.front_app_compass_check_status = false;
	spin_unlock_bh(&check_mgr.lock);
	set_data_flow_status_stop();
	cancel_work_sync(&(check_mgr.front_app_compass_check_work));
	cancel_work_sync(&(check_mgr.unavailable_status_compass_check_work));
	cancel_work_sync(&(check_mgr.data_flow_compass_check_work));
	cancel_work_sync(&(check_mgr.detect_timeout_work));
	hrtimer_cancel(&check_mgr.front_app_compass_check_timer);
	hrtimer_cancel(&check_mgr.unavailable_status_compass_check_timer);
	hrtimer_cancel(&check_mgr.data_flow_compass_check_timer);
	hrtimer_cancel(&check_mgr.detect_timer);
	return;
}

int ims_check_init()
{
	ims_mgr.timestamp_recv = 0;
	ims_mgr.config.exception_timeout = 0;
	ims_mgr.config.uid = 0;
	ims_mgr.status = false;
	spin_lock_init(&ims_mgr.lock);
	return SUCCESS;
}

void ims_check_exit()
{
	spin_lock_bh(&ims_mgr.lock);
	ims_mgr.timestamp_recv = 0;
	ims_mgr.config.exception_timeout = 0;
	ims_mgr.config.uid = 0;
	ims_mgr.status = false;
	spin_unlock_bh(&ims_mgr.lock);
}

void ims_check_reset()
{
	spin_lock_bh(&ims_mgr.lock);
	ims_mgr.timestamp_recv = 0;
	spin_unlock_bh(&ims_mgr.lock);
	return;
}

bool get_ims_status()
{
	bool ret = false;
	spin_lock_bh(&ims_mgr.lock);
	ret = ims_mgr.status;
	spin_unlock_bh(&ims_mgr.lock);
	return ret;
}

bool is_ims_timeout(s64 timestamp_send)
{
	spin_lock_bh(&ims_mgr.lock);
	if (!ims_mgr.status) {
		spin_unlock_bh(&ims_mgr.lock);
		return false;
	}

	if (ims_mgr.timestamp_recv == 0) {
		ims_mgr.timestamp_recv = timestamp_send;
		spin_unlock_bh(&ims_mgr.lock);
		return false;
	}

	if (timestamp_send - ims_mgr.timestamp_recv >
		(s64)ims_mgr.config.exception_timeout) {
		spin_unlock_bh(&ims_mgr.lock);
		return true;
	}
	spin_unlock_bh(&ims_mgr.lock);
	return false;
}

bool is_ims_uid(u32 uid)
{
	spin_lock_bh(&ims_mgr.lock);
	if (ims_mgr.config.uid == uid) {
		spin_unlock_bh(&ims_mgr.lock);
		return true;
	}
	spin_unlock_bh(&ims_mgr.lock);
	return false;
}

void update_ims_recv_timestamp(u64 timestamp_recv)
{
	spin_lock_bh(&ims_mgr.lock);
	if (ims_mgr.status)
		ims_mgr.timestamp_recv = timestamp_recv;
	spin_unlock_bh(&ims_mgr.lock);
}