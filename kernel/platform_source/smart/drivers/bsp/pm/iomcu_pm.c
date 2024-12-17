/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2019. All rights reserved.
 * Description: power manage implement.
 * Create: 2019/11/05
 */

#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/types.h>
#include <linux/gpio.h>
#include <linux/of.h>
#include <linux/of_gpio.h>
#include <linux/rtc.h>
#include <linux/of_device.h>
#include <linux/of_address.h>
#include <linux/of_irq.h>
#include <linux/suspend.h>
#include <linux/fb.h>
#include <linux/pm_wakeup.h>
#include <linux/delay.h>
#ifdef CONFIG_HUAWEI_DSM
#include <dsm/dsm_pub.h>
#endif
#include <securec.h>
#include <platform_include/smart/linux/base/ap/protocol.h>
#include <platform_include/smart/linux/iomcu_dump.h>
#include <platform_include/smart/linux/iomcu_ipc.h>
#include <platform_include/smart/linux/iomcu_pm.h>
#include <platform_include/smart/linux/iomcu_log.h>
#include <platform_include/smart/linux/iomcu_status.h>
#include <platform_include/smart/linux/iomcu_config.h>
#ifdef CONFIG_DKMD_DKSM
#include <platform_include/display/dkmd/dkmd_notify.h>
#include <platform_include/display/dkmd/dkmd_dpu.h>
#endif

#define RESUME_INIT                            0
#define RESUME_MINI                            1
#define RESUME_SKIP                            2

#define SENSORHUB_PM_RESUME_TIMEOUT_MS         1000

#ifdef CONFIG_DKMD_DKSM
#define PM_SCREEN_NUM                          2
static unsigned int g_pm_screen_state = 0;     // default is off.
#endif

static struct iomcu_power_status g_i_power_status;
static int g_resume_skip_flag = RESUME_INIT;
sys_status_t iom3_sr_status = ST_WAKEUP;

static int g_iom3_power_state = ST_POWERON;
static struct completion g_iom3_reboot;
static struct completion g_iom3_resume_mini;
static struct completion g_iom3_resume_all;

struct ipc_debug {
	int event_cnt[TAG_END];
	int pack_error_cnt;
};

static struct ipc_debug g_ipc_debug_info;

DEFINE_MUTEX(g_mutex_pstatus);

static char *g_sys_status_t_str[] = {
	[ST_SCREENON] = "ST_SCREENON",
	[ST_SCREENOFF] = "ST_SCREENOFF",
	[ST_SLEEP] = "ST_SLEEP",
	[ST_WAKEUP] = "ST_WAKEUP",
};

static BLOCKING_NOTIFIER_HEAD(iomcu_pm_notifier_list);

int register_iomcu_pm_notifier(struct notifier_block *nb)
{
	return blocking_notifier_chain_register(&iomcu_pm_notifier_list, nb);
}

int unregister_iomcu_pm_notifier(struct notifier_block *nb)
{
	return blocking_notifier_chain_unregister(&iomcu_pm_notifier_list, nb);
}

void atomic_get_iomcu_power_status(struct iomcu_power_status *p)
{
	size_t len;

	if (p == NULL) {
		ctxhub_err("atomic_get_iomcu_power_status: input pointer NULL\n");
		return;
	}

	len = sizeof(struct iomcu_power_status);

	mutex_lock(&g_mutex_pstatus);
	(void)memcpy_s(p, len, &g_i_power_status, len);
	mutex_unlock(&g_mutex_pstatus);
}

int get_iomcu_power_state(void)
{
	return g_iom3_power_state;
}
EXPORT_SYMBOL(get_iomcu_power_state); //lint !e546 !e580

static inline void clean_ipc_debug_info(void)
{
	(void)memset_s(&g_ipc_debug_info, sizeof(g_ipc_debug_info), 0,
		sizeof(g_ipc_debug_info));
}

static void print_ipc_debug_info(void)
{
	int i;

	for (i = TAG_BEGIN; i < TAG_END; ++i) {
		if (g_ipc_debug_info.event_cnt[i])
			ctxhub_info("event_cnt[%d]: %d\n", i,
				 g_ipc_debug_info.event_cnt[i]);
	}
	if (g_ipc_debug_info.pack_error_cnt)
		ctxhub_err("pack_err_cnt: %d\n", g_ipc_debug_info.pack_error_cnt);
}

int tell_ap_status_to_mcu(int ap_st)
{
	struct read_info pkg_mcu;
	struct write_info winfo;
	pkt_sys_statuschange_req_t pkt;

	if ((ap_st >= ST_BEGIN) && (ap_st < ST_END)) {
		(void)memset_s(&winfo, sizeof(struct write_info), 0, sizeof(struct write_info));
		(void)memset_s(&pkg_mcu, sizeof(struct read_info), 0, sizeof(struct read_info));
		(void)memset_s(&pkt, sizeof(pkt_sys_statuschange_req_t), 0, sizeof(pkt_sys_statuschange_req_t));
		winfo.tag = TAG_SYS;
		winfo.cmd = CMD_SYS_STATUSCHANGE_REQ;
		winfo.wr_len = (int)(sizeof(pkt) - sizeof(pkt.hd));
		pkt.status = (unsigned short)ap_st;
		winfo.wr_buf = &pkt.status;

		if (likely((ap_st >= ST_SCREENON) && (ap_st <= ST_WAKEUP))) {
			ctxhub_info("------------>tell mcu ap in status %s\n",
				 g_sys_status_t_str[ap_st]);
			iom3_sr_status = ap_st;
		} else {
			ctxhub_info("------------>tell mcu ap in status %d\n",
				 ap_st);
		}
		return write_customize_cmd(&winfo,
					(ap_st == ST_SLEEP) ?
					(&pkg_mcu) : NULL,
					true);
	} else {
		ctxhub_err("error status %d in %s\n", ap_st, __func__);
		return -EINVAL;
	}
}

#ifdef CONFIG_CONTEXTHUB_DSS_TRISTATEID
int tell_fold_status_to_mcu(int ap_st)
{
	struct write_info winfo;
	pkt_sys_statuschange_req_t pkt;

	if  (ap_st < PPC_CONFIG_ID_CNT) {
		(void)memset_s(&winfo, sizeof(struct write_info), 0, sizeof(struct write_info));
		(void)memset_s(&pkt, sizeof(pkt_sys_statuschange_req_t), 0, sizeof(pkt_sys_statuschange_req_t));
		winfo.tag = TAG_SYS;
		winfo.cmd = CMD_DSS_STATUSCHANGE_REQ;
		winfo.wr_len = (int)(sizeof(pkt) - sizeof(pkt.hd));
		pkt.status = (unsigned short)ap_st;
		winfo.wr_buf = &pkt.status;
		return write_customize_cmd(&winfo, NULL, true);
	} else {
		ctxhub_err("error status %d in %s\n", ap_st, __func__);
		return -EINVAL;
	}
}
#endif

static int tell_screen_info_to_mcu(unsigned int screen_info)
{
	struct write_info winfo;

	(void)memset_s(&winfo, sizeof(struct write_info), 0, sizeof(struct write_info));

	winfo.tag = TAG_SYS;
	winfo.cmd = CMD_SYS_SCREEN_INFO_REQ;
	winfo.wr_len = (int)sizeof(unsigned int);
	winfo.wr_buf = (void *)(&screen_info);

	ctxhub_info("------------>tell_screen_info_to_mcu %x\n", screen_info);

	return write_customize_cmd(&winfo, NULL, true);
}

sys_status_t get_iom3_sr_status(void)
{
	return iom3_sr_status;
}

static void update_current_app_status(u8 tag, u8 cmd)
{
	if (tag >= TAG_END)
		return;

	mutex_lock(&g_mutex_pstatus);
	if ((cmd == CMD_CMN_OPEN_REQ) || (cmd == CMD_CMN_INTERVAL_REQ))
		g_i_power_status.app_status[tag] = 1;
	else if (cmd == CMD_CMN_CLOSE_REQ)
		g_i_power_status.app_status[tag] = 0;
	mutex_unlock(&g_mutex_pstatus);
}

int sensorhub_pm_s4_entry(void)
{
	int ret = RET_SUCC;

	ctxhub_info("%s, iom3_sr_status:%u\n", __func__, iom3_sr_status);
	if (iom3_sr_status != ST_POWEROFF) {
		ret = tell_ap_status_to_mcu(ST_POWEROFF);
		if (ret != RET_SUCC)
			ctxhub_warn("tell_ap_status_to_mcu err:%d in %s\n", ret, __func__);
		g_iom3_power_state = ST_POWEROFF;
		clean_ipc_debug_info();
	}

	return ret;
}

static void iomcu_pm_send_cmd_notifier(const struct pkt_header * head)
{
	if (head == NULL)
		return;

	update_current_app_status(head->tag, head->cmd);
}

static void check_current_app(void)
{
	int i = 0;
	int flag = 0;

	mutex_lock(&g_mutex_pstatus);
	for (i = 0; i < TAG_END; i++) {
		if (g_i_power_status.app_status[i])
			flag++;
	}
	if (flag > 0) {
		ctxhub_info("total %d app running after ap suspend\n", flag);
		g_i_power_status.power_status = SUB_POWER_ON;
	} else {
		ctxhub_info("iomcu will power off after ap suspend\n");
		g_i_power_status.power_status = SUB_POWER_OFF;
	}
	mutex_unlock(&g_mutex_pstatus);
}

static int sensorhub_pm_suspend(struct device *dev)
{
	int ret = RET_SUCC;

	ctxhub_info("%s+\n", __func__);
	if (iom3_sr_status != ST_SLEEP) {
		ret = tell_ap_status_to_mcu(ST_SLEEP);
		g_iom3_power_state = ST_SLEEP;
		check_current_app();
		clean_ipc_debug_info();
	}
	ctxhub_info("%s-\n", __func__);
	return ret;
}

static int wait_all_ready(void)
{
	if (wait_for_completion_timeout(&g_iom3_resume_all,
					 msecs_to_jiffies(1000)) == 0) {
		ctxhub_err("RESUME :wait for ALL timeout\n");
		return RET_FAIL;
	} else if (g_resume_skip_flag != RESUME_SKIP) {
		ctxhub_info("RESUME mcu all ready!\n");
		blocking_notifier_call_chain(&iomcu_pm_notifier_list, ST_MCUREADY, NULL);
	} else {
		ctxhub_err("RESUME skip ALL\n");
	}
	return RET_SUCC;
}

static int sensorhub_pm_resume(struct device *dev)
{
	int ret = RET_SUCC;

	ctxhub_info("%s+\n", __func__);
	print_ipc_debug_info();
	g_resume_skip_flag = RESUME_INIT;

	reinit_completion(&g_iom3_reboot);
	reinit_completion(&g_iom3_resume_mini);
	reinit_completion(&g_iom3_resume_all);
	barrier();
	write_timestamp_base_to_sharemem();
	tell_ap_status_to_mcu(ST_WAKEUP);
	/* wait mini ready */
	if (wait_for_completion_timeout(&g_iom3_resume_mini,
			msecs_to_jiffies(SENSORHUB_PM_RESUME_TIMEOUT_MS)) == 0) {
		ctxhub_err("RESUME :wait for MINI timeout\n");
		goto resume_err;
	} else if (g_resume_skip_flag != RESUME_SKIP) {
		blocking_notifier_call_chain(&iomcu_pm_notifier_list, ST_MINSYSREADY, NULL);
	} else {
		ctxhub_err("RESUME skip MINI\n");
	}

	/* wait all ready */
	ret = wait_all_ready();
	if (ret != RET_SUCC)
		goto resume_err;

	if (wait_for_completion_timeout(&g_iom3_reboot,
			msecs_to_jiffies(SENSORHUB_PM_RESUME_TIMEOUT_MS)) == 0) {
		ctxhub_err("resume :wait for response timeout\n");
		goto resume_err;
	}

	// not support print_err_info()
	goto done;
resume_err:
	/* always ret 0 */
	iom3_need_recovery(SENSORHUB_MODID, SH_FAULT_RESUME);
done:
	g_iom3_power_state = ST_WAKEUP;
	ctxhub_info("%s-\n", __func__);
	return RET_SUCC;
}
#ifdef CONFIG_DKMD_DKSM
static void sensorhub_fb_event_blank_status_to_mcu(int blank, struct dkmd_event *event)
{
	if (event == NULL) {
		ctxhub_err("%s:event is null\n", __func__);
		return;
	}
	switch (blank) {
		case FB_BLANK_UNBLANK:
		// first screen is power on, tell muc screen on.
			if (g_pm_screen_state == 0)
				tell_ap_status_to_mcu(ST_SCREENON);
			g_pm_screen_state |= (1 << (unsigned char)event->panel_id);
			ctxhub_info("%s: panel %x power on %x.\n", __func__, event->panel_id, g_pm_screen_state);
			break;
		case FB_BLANK_POWERDOWN:
			g_pm_screen_state &= ~(1 << (unsigned char)event->panel_id);
			// all screen is power donw, tell mcu screen off.
			if (g_pm_screen_state == 0)
				tell_ap_status_to_mcu(ST_SCREENOFF);
			ctxhub_info("%s: panel %x power off %x.\n", __func__, event->panel_id, g_pm_screen_state);
			break;
		default:
			ctxhub_err("unknown---> lcd unknown in %s\n", __func__);
			break;
	}
}

static void sensorhub_status_to_mcu(unsigned long action, int blank,
				struct dkmd_event *event)
{
	if (event == NULL) {
		ctxhub_err("%s:event is null\n", __func__);
		return;
	}
	switch (action) {
	/* change finished */
		case DKMD_EVENT_BLANK:
			sensorhub_fb_event_blank_status_to_mcu(blank, event);
			break;
#ifdef CONFIG_CONTEXTHUB_DSS_TRISTATEID
		case DKMD_EVENT_SET_ACTIVE_RECT:
			tell_fold_status_to_mcu(blank);
			ctxhub_info("%s: blank: %u\n", __func__, blank);
			break;
#endif
		default:
			break;
	}
}

static int sensorhub_fb_notifier(struct notifier_block *nb, unsigned long action, void *data)
{
	struct dkmd_event *event = NULL;
	int blank = 0;
	unsigned int old_screen_state = 0;

	if (!data)
		return NOTIFY_OK;
	event = (struct dkmd_event *)data;
	blank = event->value;
	if (event->panel_id >= PM_SCREEN_NUM) {
		ctxhub_err("[%s] panel_id not match.\n", __func__);
		return NOTIFY_OK;
	}
	old_screen_state = g_pm_screen_state;
	sensorhub_status_to_mcu(action, blank, event);

	if (g_pm_screen_state != old_screen_state)
		tell_screen_info_to_mcu(g_pm_screen_state);

	return NOTIFY_OK;
}
#else
static int sensorhub_fb_notifier(struct notifier_block *nb,
				 unsigned long action, void *data)
{
	if (!data)
		return NOTIFY_OK;
	switch (action) {
	/* change finished */
	case FB_EVENT_BLANK:
		{
			struct fb_event *event = data;
			int *blank = event->data;

			if (!blank) {
				ctxhub_err("%s, blank is null, return\n",
					__func__);
				return NOTIFY_OK;
			}
			/* only main screen on/off info send to hub */
			if (registered_fb[0] != event->info) {
				ctxhub_err("%s, not main screen info, just return\n",
					__func__);
				return NOTIFY_OK;
			}
			switch (*blank) {
			/* screen on */
			case FB_BLANK_UNBLANK:
				tell_ap_status_to_mcu(ST_SCREENON);
				break;
			/* screen off */
			case FB_BLANK_POWERDOWN:
				tell_ap_status_to_mcu(ST_SCREENOFF);
				break;

			default:
				ctxhub_err("unknown---> lcd unknown in %s\n",
					__func__);
				break;
			}
			break;
		}
	default:
		break;
	}

	return NOTIFY_OK;
}
#endif

static struct notifier_block fb_notify = {
	.notifier_call = sensorhub_fb_notifier,
};


static void set_pm_notifier(void)
{
	init_completion(&g_iom3_resume_mini);
	init_completion(&g_iom3_resume_all);
	init_completion(&g_iom3_reboot);
#ifdef CONFIG_DKMD_DKSM
	dkmd_register_client(&fb_notify);
#else
	fb_register_client(&fb_notify);
#endif
}

static int mcu_power_log_process(const struct pkt_header *head)
{
	ctxhub_info("%s in\n", __func__);
	if (head == NULL) {
		ctxhub_err("%s input head is null\n", __func__);
		return RET_FAIL;
	}

	mutex_lock(&g_mutex_pstatus);
	g_i_power_status.idle_time = ((pkt_power_log_report_req_t *)head)->idle_time;
	g_i_power_status.active_app_during_suspend =
		((pkt_power_log_report_req_t *)head)->current_app_mask;
	mutex_unlock(&g_mutex_pstatus);

	ctxhub_info("last suspend iomcu idle time is %d , active apps high is  0x%x, low is  0x%x\n",
		g_i_power_status.idle_time,
		(uint32_t)((g_i_power_status.active_app_during_suspend >> 32) & 0xffffffff),
		(uint32_t)(g_i_power_status.active_app_during_suspend & 0xffffffff));
	return 0;
}

static int iomcu_power_info_init(void)
{
	int ret;

	(void)memset_s(&g_i_power_status, sizeof(struct iomcu_power_status),
			0, sizeof(struct iomcu_power_status));
	ret = register_mcu_event_notifier(TAG_LOG, CMD_LOG_POWER_REQ,
		mcu_power_log_process);
	if (ret != 0) {
		ctxhub_err("[%s], register power log failed!\n", __func__);
		return ret;
	}
	return 0;
}

static bool is_mcu_wakeup(const struct pkt_header *head)
{
	if ((head->tag == TAG_SYS) && (head->cmd == CMD_SYS_STATUSCHANGE_REQ) &&
		(((pkt_sys_statuschange_req_t *)head)->status == ST_WAKEUP))
		return true;
	return false;
}

static bool is_mcu_resume_mini(const struct pkt_header *head)
{
	if ((head->tag == TAG_SYS) && (head->cmd == CMD_SYS_STATUSCHANGE_REQ) &&
		((pkt_sys_statuschange_req_t *)head)->status == ST_MINSYSREADY &&
		get_iom3_rec_state() != IOM3_RECOVERY_MINISYS)
		return true;
	return false;
}

static bool is_mcu_resume_all(const struct pkt_header *head)
{
	if ((head->tag == TAG_SYS) && (head->cmd == CMD_SYS_STATUSCHANGE_REQ) &&
		((pkt_sys_statuschange_req_t *)head)->status == ST_MCUREADY &&
		get_iom3_rec_state() != IOM3_RECOVERY_MINISYS)
		return true;
	return false;
}

static int mcu_reboot_callback(const struct pkt_header *head)
{
	ctxhub_info("%s\n", __func__);
	complete(&g_iom3_reboot);
	return 0;
}

static int inputhub_pm_callback(const struct pkt_header *head)
{
	if ((head->tag == TAG_SYS) && (head->cmd == CMD_SYS_STATUSCHANGE_REQ))
		ctxhub_info("%s recv. status %x, skip_flag %x, rec %x\n",
			    __func__, ((pkt_sys_statuschange_req_t *)head)->status,
			    g_resume_skip_flag, get_iom3_rec_state());

	if (is_mcu_resume_mini(head)) {
		ctxhub_warn("%s MINI ready\n", __func__);
		g_resume_skip_flag = RESUME_MINI;
		barrier();
		complete(&g_iom3_resume_mini);
	}
	if (is_mcu_resume_all(head)) {
		ctxhub_warn("%s ALL ready\n", __func__);
		complete(&g_iom3_resume_all);
	}
	if (is_mcu_wakeup(head)) {
		if (g_resume_skip_flag != RESUME_MINI) {
			g_resume_skip_flag = RESUME_SKIP;
			barrier();
			complete(&g_iom3_resume_mini);
			complete(&g_iom3_resume_all);
		}
		mcu_reboot_callback(head);
	}

	return 0;
}

static int iomcu_ipc_notifier(struct notifier_block *nb, unsigned long event, void *para)
{
	switch (event) {
	case IOMCU_IPC_SEND:
		iomcu_pm_send_cmd_notifier((struct pkt_header *)para);
		break;

	default:
		break;
	}
	return NOTIFY_OK;
}

static struct notifier_block iomcu_ipc_notif_block = {
	.notifier_call = iomcu_ipc_notifier,
};

/* probe() function for platform driver */
static int iomcu_probe(struct platform_device *pdev)
{
	return 0;
}

/* remove() function for platform driver */
static int __exit iomcu_remove(struct platform_device *pdev)
{
	return 0;
}

const struct dev_pm_ops iomcu_pm_ops = {
	.suspend = sensorhub_pm_suspend,
	.resume  = sensorhub_pm_resume,
};

#define IOMCU_DEV_COMPAT_STR "hisilicon,contexthub_status"

static const struct of_device_id iomcu_match_table[] = {
	{ .compatible = IOMCU_DEV_COMPAT_STR, },
	{},
};

static struct platform_driver iomcu_platdev = {
	.driver = {
		.name = "iomcu_dev",
		.owner = THIS_MODULE,
		.of_match_table = of_match_ptr(iomcu_match_table),
		.pm = &iomcu_pm_ops,
	},
	.probe  = iomcu_probe,
	.remove = iomcu_remove,
};

static int iomcu_pm_init(void)
{
	int ret;

	ctxhub_info("[%s] ++", __func__);
	if (get_contexthub_dts_status())
		return -EACCES;

	ret = iomcu_power_info_init();
	if (ret != 0)
		return ret;

	set_pm_notifier();

	ret = register_mcu_event_notifier(TAG_ALL, CMD_ALL, inputhub_pm_callback);
	if (ret != 0)
		ctxhub_err("[%s], register mcu_event_notifier failed!\n", __func__);

	ret = register_mcu_send_cmd_notifier(&iomcu_ipc_notif_block);
	if (ret != 0)
		ctxhub_err("[%s], register iomcu_pm_send_cmd_notifier failed\n", __func__);

	ctxhub_info("[%s] --", __func__);

	return platform_driver_register(&iomcu_platdev);
}

static void iomcu_pm_exit(void)
{
	(void)unregister_mcu_event_notifier(TAG_ALL, CMD_ALL, inputhub_pm_callback);
	(void)unregister_mcu_send_cmd_notifier(&iomcu_ipc_notif_block);

	ctxhub_info("%s...\n", __func__);
}

late_initcall(iomcu_pm_init)
module_exit(iomcu_pm_exit);
