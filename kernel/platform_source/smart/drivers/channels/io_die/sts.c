/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2022-2022. All rights reserved.
 * Description: Contexthub 9030 driver.
 * Create: 2022-11-30
 */

#include <linux/init.h>
#include <linux/notifier.h>
#include <linux/uaccess.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/err.h>
#include <linux/time.h>
#include <linux/slab.h>
#include <linux/mutex.h>
#include <linux/debugfs.h>
#include <linux/delay.h>
#include <linux/jiffies.h>
#include <linux/io.h>
#include "inputhub_api/inputhub_api.h"
#include <platform_include/smart/linux/iomcu_status.h>
#include <platform_include/smart/linux/iomcu_ipc.h>
#include <platform_include/basicplatform/linux/rdr_platform.h>

#include <securec.h>
#include "sts.h"
#include <big_data_channel.h>

#define MODULE_NAME                      "IO_DIE_STS"
#define STS_MAX_PAYLOAD                  (MAX_PKT_LENGTH - (int)sizeof(struct pkt_header))
#define NS_TO_MS                         1000000
#define STS_IPC_RETRY_TIMES              5
#define STS_IPC_RETRY_INTERVAL           200   // ms
#define STS_IPC_RETRY_THREHOLD           2000  // ms

typedef struct {
	atomic64_t max_latency;
	sts_ipc_cmd_enum cmd;
} sts_perf_stat_t;

typedef struct {
	uint8_t vote_count[STS_USER_END];
	uint32_t total;
}sts_vote_t;

typedef struct {
	struct pkt_header hd;
	uint8_t status;
	sts_err_info_t err_info;
}sts_sh_report_status_t;

static sts_sys_status_enum g_sys_st = STS_ST_PRE_INITED;
static sts_vote_t g_sts_vote = {{0}, 0};
static DEFINE_MUTEX(g_sts_st_lock);
static sts_user_func g_sts_user_func_list[STS_USER_END];
static DEFINE_MUTEX(g_sts_notify_lock);
static sts_perf_stat_t g_sts_perf;
static u8 g_support_io_die = 1;
int g_normal_flag = 0;
u64 g_audio_jiffies;
u64 g_tp_jiffies;
u64 g_camera_jiffies;
u64 g_others_jiffies;

typedef struct g_sts_sh_time{
	u32 ap_time;
	u32 modem_time;
	u32 m7_time;
	u32 others_time;
} g_sts_sh_time;

typedef struct g_sts_time{
	u64 audio_time;
	u64 tp_time;
	u64 camera_time;
	u64 others_time;
	u32 ap_time;
	u32 modem_time;
	u32 m7_time;
	u32 coreothers_time;
	u32 coretotal;
} g_sts_time;

static g_sts_time g_sts_update_data;

void* send_sts_data_to_dubai(void)
{
	g_sts_sh_time sts_sh_tmp;

	iomcu_dubai_log_fetch(DUBAI_EVENT_STS_NORMAL_TIME, &sts_sh_tmp, sizeof(g_sts_sh_time));

	g_sts_update_data.ap_time = sts_sh_tmp.ap_time;
	g_sts_update_data.modem_time = sts_sh_tmp.modem_time;
	g_sts_update_data.m7_time = sts_sh_tmp.m7_time;
	g_sts_update_data.coreothers_time = sts_sh_tmp.others_time;
	g_sts_update_data.coretotal = sts_sh_tmp.modem_time + sts_sh_tmp.m7_time +
	                              sts_sh_tmp.others_time;

	pr_info("audio time : %lld, tp time : %lld, camera time : %lld, others time : %lld\n",
	       g_sts_update_data.audio_time, g_sts_update_data.tp_time,
	       g_sts_update_data.camera_time, g_sts_update_data.others_time);
	pr_info("ap time : %d, modem time : %d, m7 time : %d, core others time : %d,\
	       total time : %d\n", g_sts_update_data.ap_time, g_sts_update_data.modem_time,
	       g_sts_update_data.m7_time, g_sts_update_data.coreothers_time,
	       g_sts_update_data.coretotal);

	return (void*)&g_sts_update_data;
}

void sts_data_reset(void)
{
	g_sts_update_data.audio_time = 0;
	g_sts_update_data.tp_time = 0;
	g_sts_update_data.camera_time = 0;
	g_sts_update_data.others_time = 0;
}

static int io_die_sts_ipc(const struct write_info *wr, struct read_info *rd)
{
	int ret;
	uint8_t times = 0;
	u64 entry_jiffies;
	u64 exit_jiffies;
	u64 diff_usecs;

	entry_jiffies = get_jiffies_64();
	ret = write_customize_cmd(wr, rd, true);
	while (ret < 0 && times < STS_IPC_RETRY_TIMES) {
		pr_err("%s: send err, times[%d] ret[%d]\n", __func__, times, ret);
		exit_jiffies = get_jiffies_64();
		diff_usecs = jiffies64_to_nsecs(exit_jiffies - entry_jiffies) / NS_TO_MS;
		if (diff_usecs > STS_IPC_RETRY_THREHOLD) {
			pr_err("%s: timeout, lantency[%llu]ms. \n", __func__, diff_usecs);
			break;
		}
		times++;
		msleep(STS_IPC_RETRY_INTERVAL);
		ret = write_customize_cmd(wr, rd, true);
	}

	return ret;
}

static int do_sts_commu(sts_ipc_cmd_enum cmd, void *data)
{
	int ret;
	struct write_info winfo;
	struct read_info rinfo;

	(void)memset_s(&winfo, sizeof(struct write_info), 0, sizeof(struct write_info));
	(void)memset_s(&rinfo, sizeof(struct read_info), 0, sizeof(struct read_info));

	switch (cmd) {
	case STS_CMD_REQUEST_NORMAL:
	case STS_CMD_RELEASE_NORMAL:
		winfo.wr_len = 0;
		winfo.wr_buf = 0;
		break;
	case STS_CMD_REG_OP:
		winfo.wr_len = sizeof(sts_reg_op_t);
		winfo.wr_buf = data;
		break;
	case STS_CMD_CLK_OUT_CFG:
		winfo.wr_len = sizeof(sts_clk_out_t);;
		winfo.wr_buf = data;
		break;
	case STS_CMD_SPI_CONFIG:
		winfo.wr_len = sizeof(sts_spi_config_t);
		winfo.wr_buf = data;
		break;
	case STS_CMD_SPI_RST:
		winfo.wr_len = 0;
		winfo.wr_buf = 0;
		break;
	default:
		pr_err("%s: invalid cmd [%u].\n", __func__, cmd);
		return -EINVAL;
	}

	winfo.cmd = cmd;
	winfo.tag = TAG_IO_DIE;
	ret = io_die_sts_ipc(&winfo, &rinfo);
	if (ret) {
		pr_err("%s: send err, ret is [%d]\n", __func__, ret);
		return ret;
	}

	pr_info("%s: resp len [%d]\n", __func__, rinfo.data_length);
	if(rinfo.data_length > 0 && STS_CMD_REG_OP == cmd)
		((sts_reg_op_t *)data)->val = *(u32 *)rinfo.data;

	if(rinfo.errno) {
		pr_err("%s: wait resp err, ret is [%d]\n", __func__, rinfo.errno);
		return rinfo.errno;
	}

	return ret;
}

static int sts_commu(sts_ipc_cmd_enum cmd, void *data)
{
	int ret;
	u64 entry_jiffies;
	u64 exit_jiffies;
	u64 diff_usecs;

	if (!g_support_io_die) {
		pr_info("%s: no io die. \n", __func__);
		return EOK;
	}

	entry_jiffies = get_jiffies_64();
	ret = do_sts_commu(cmd, data);
	exit_jiffies = get_jiffies_64();
	diff_usecs = jiffies64_to_nsecs(exit_jiffies - entry_jiffies) / NS_TO_MS;
	if (unlikely(diff_usecs > (u64)atomic64_read(&g_sts_perf.max_latency))) {
		atomic64_set(&g_sts_perf.max_latency, (s64)diff_usecs);
		g_sts_perf.cmd = cmd;
		pr_info("%s:max_latency, cmd[%u], lantency[%llu]ms\n", __func__, cmd, diff_usecs);
	}

	pr_info("%s: send complete, cmd[%u], lantency[%llu]ms.\n", __func__, cmd, diff_usecs);

	return ret;
}

int sts_notify_register(sts_ap_user_enum user, sts_user_func func)
{
	if (user < STS_USER_BEGIN || user >= STS_USER_END || func == NULL)
		return -EINVAL;

	mutex_lock(&g_sts_notify_lock);
	g_sts_user_func_list[user] = func;
	mutex_unlock(&g_sts_notify_lock);

	return EOK;
}

static int sts_set_status(sts_sys_status_enum st)
{
	if (st < STS_ST_BEGIN || st >= STS_ST_END)
		return -EINVAL;

	mutex_lock(&g_sts_st_lock);
	g_sys_st = st;
	if (st == STS_ST_NON_EXIST)
		g_support_io_die = 0;
	mutex_unlock(&g_sts_st_lock);

	return EOK;
}

sts_sys_status_enum sts_get_status(void)
{
	sts_sys_status_enum st;

	mutex_lock(&g_sts_st_lock);
	st = g_sys_st;
	mutex_unlock(&g_sts_st_lock);

	return st;
}

int sts_normal_request(sts_ap_user_enum src)
{
	uint32_t pre_vote;
	int ret = EOK;

	if (src < STS_USER_BEGIN || src >= STS_USER_END)
		return -EINVAL;

	pr_info("%s: from [%u]. \n", __func__, (uint32_t)src);

	mutex_lock(&g_sts_st_lock);
	if (g_sts_vote.vote_count[src] == 0xFF) {
		pr_err("%s: src [%u] upto max vote. \n", __func__, (uint32_t)src);
		mutex_unlock(&g_sts_st_lock);
		return -EFAULT;
	}

	pre_vote = g_sts_vote.total;
	g_sts_vote.vote_count[src]++;
	g_sts_vote.total++;

	if (pre_vote == 0 && g_sts_vote.total != 0) {
		pr_info("sts ap request normal, form [%u]. \n", (uint32_t)src);
		ret = sts_commu(STS_CMD_REQUEST_NORMAL, NULL);
		if (ret != EOK) {
			pr_info("%s, fail. \n", __func__);
			g_sts_vote.vote_count[src]--;
			g_sts_vote.total--;
		} else {
			pr_info("%s, success. \n", __func__);
			g_normal_flag = 1;
			switch (src) {
			case STS_USER_AUDIO:
				g_audio_jiffies = get_jiffies_64();
				break;
			case STS_USER_TP:
				g_tp_jiffies = get_jiffies_64();
				break;
			case STS_USER_CAMERA:
				g_camera_jiffies = get_jiffies_64();
				break;
			case STS_USER_END:
			default:
				g_others_jiffies = get_jiffies_64();
				break;
			}
		}
	}
	mutex_unlock(&g_sts_st_lock);

	return ret;
}

int sts_normal_release(sts_ap_user_enum src)
{
	int ret = EOK;
	u64 release_jiffies = 0;

	if (src < STS_USER_BEGIN || src >= STS_USER_END)
		return -EINVAL;

	pr_info("%s: from [%u]. \n", __func__, (uint32_t)src);

	mutex_lock(&g_sts_st_lock);
	if (g_sts_vote.vote_count[src] == 0) {
		pr_err("%s: src [%u] has no vote. \n", __func__, (uint32_t)src);
		mutex_unlock(&g_sts_st_lock);
		return -EFAULT;
	}

	g_sts_vote.vote_count[src]--;
	g_sts_vote.total--;
	if (g_sts_vote.total == 0) {
		pr_info("sts ap release normal, form [%u] \n", (uint32_t)src);
		ret = sts_commu(STS_CMD_RELEASE_NORMAL, NULL);
		if (ret != EOK) {
			pr_info("%s, fail \n", __func__);
			g_sts_vote.vote_count[src]++;
			g_sts_vote.total++;
		} else {
			pr_info("%s, success \n", __func__);
			if (g_normal_flag == 1) {
				g_normal_flag = 0;
				release_jiffies = get_jiffies_64();
				switch (src) {
				case STS_USER_AUDIO:
					g_sts_update_data.audio_time +=
					jiffies64_to_msecs(release_jiffies - g_audio_jiffies);
					break;
				case STS_USER_TP:
					g_sts_update_data.tp_time +=
					jiffies64_to_msecs(release_jiffies - g_tp_jiffies);
					break;
				case STS_USER_CAMERA:
					g_sts_update_data.camera_time +=
					jiffies64_to_msecs(release_jiffies - g_camera_jiffies);
					break;
				case STS_USER_END:
				default:
					g_sts_update_data.others_time +=
					jiffies64_to_msecs(release_jiffies - g_others_jiffies);
					break;
				}
			}
		}
	}
	mutex_unlock(&g_sts_st_lock);

	return ret;
}

/* sync ipc */
int sts_do_reg_op(sts_reg_op_t *data)
{
	if (data == NULL) {
		pr_err("sts_reg_op: data is null\n");
		return -EINVAL;
	}

	pr_info("%s: op[%x], slave_id[%x], reg_addr[%x], val[%x]\n", __func__, data->op, data->slave_id,
		data->reg_addr, data->val);

	return sts_commu(STS_CMD_REG_OP, data);
}

int sts_spi_rst(sts_ap_user_enum src, enum ioide_sts_err_code err_code)
{
	u32 vote;

	mutex_lock(&g_sts_st_lock);
	vote = g_sts_vote.vote_count[STS_USER_TP];
	if((src != STS_USER_TP) || (vote == 0) || (err_code != STS_ESD_ERR)) {
		pr_err("%s fail, src %u, vote %u\n", __func__, src, vote);
		mutex_unlock(&g_sts_st_lock);
		return -EINVAL;
	}
	mutex_unlock(&g_sts_st_lock);
	pr_err("%s start\n", __func__);

	return sts_commu(STS_CMD_SPI_RST, NULL);
}

int sts_clk_out_sys(sts_clk_out_t *data)
{
	if (data == NULL) {
		pr_err("sts_clk_out_sys: data is null\n");
		return -EINVAL;
	}

	return sts_commu(STS_CMD_CLK_OUT_CFG, data);
}

int sts_spi_config(sts_spi_config_t *data)
{
	if (data == NULL) {
		pr_err("sts_spi_config: data is null\n");
		return -EINVAL;
	}

	pr_info("%s: speed[%x]\n", __func__, data->speed);

	return sts_commu(STS_CMD_SPI_CONFIG, data);
}

static int sts_notify_handler(const struct pkt_header *pkt)
{
	int i, ret;
	sts_err_info_t *err_info = NULL;
	sts_sh_report_status_t *data = (sts_sh_report_status_t *)pkt;

	if (pkt == NULL || pkt->tag != TAG_IO_DIE || pkt->length < STS_STATUS_LEN) {
		pr_err("%s: invalid para\n", __func__);
		return -EINVAL;
	}
	pr_info("%s: tag:%x, cmd:%x, len:%x, status:%x\n", __func__, pkt->tag, pkt->cmd, pkt->length, data->status);
	ret = sts_set_status(data->status);
	if (ret != EOK)
		return ret;

	pr_info("%s: curr status:%x\n", __func__, sts_get_status());

	if(data->status == STS_ST_ERROR)
		err_info = (sts_err_info_t *)&(data->err_info);

	if (pkt->cmd == STS_CMD_STATUS_NOTIFY) {
		mutex_lock(&g_sts_notify_lock);
		for (i = STS_USER_BEGIN; i < STS_USER_END; i++)
			if (g_sts_user_func_list[i] != NULL)
				g_sts_user_func_list[i](data->status, err_info);
		mutex_unlock(&g_sts_notify_lock);
	}

	if(data->status == STS_ST_ERROR) {
		/* record error msg to RDR and trigger system error */
		pr_err("%s: io die sts error. slave A:%x, slave B:%x\n", __func__, *(uint32_t *)(err_info->a_side),
			*(uint32_t *)(err_info->b_side));
		pr_err("iomcu restart.\n");
		iom3_need_recovery(SENSORHUB_USER_MODID, SH_FAULT_IO_DIE);
		rdr_syserr_process_for_ap(MODID_AP_S_PANIC_IO_DIE_STS, *(uint64_t *)(err_info->a_side),
			*(uint64_t *)(err_info->b_side));
		return -EFAULT;
	}

	return EOK;
}

static void sts_recovery_iom3_handler(void)
{
	int ret;

	pr_info("%s: sts status [%d]", __func__, sts_get_status());
	if (sts_get_status() != STS_ST_INITED)
		return;

	mutex_lock(&g_sts_st_lock);
	pr_info("%s: total vote [%d].\n",__func__, g_sts_vote.total);
	pr_info("%s: audio vote [%d].\n",__func__, g_sts_vote.vote_count[STS_USER_AUDIO]);
	pr_info("%s: tp vote [%d].\n",__func__, g_sts_vote.vote_count[STS_USER_TP]);
	pr_info("%s: carmera vote [%d].\n",__func__, g_sts_vote.vote_count[STS_USER_CAMERA]);

	/* release the defalut vote generated by sensorhub inited */
	if (g_sts_vote.total == 0) {
		pr_info("%s: sts recovery ap vote!", __func__);
		ret = sts_commu(STS_CMD_RELEASE_NORMAL, NULL);
		if (ret < 0)
			pr_err("%s: sts recovery vote fail", __func__);
	}
	mutex_unlock(&g_sts_st_lock);
}

static int sts_recovery_notifier(struct notifier_block *nb,
	unsigned long foo, void *bar)
{
	pr_info("%s %lu +\n", __func__, foo);
	switch (foo) {
	case IOM3_RECOVERY_IDLE:
	case IOM3_RECOVERY_START:
	case IOM3_RECOVERY_MINISYS:
	case IOM3_RECOVERY_3RD_DOING:
	case IOM3_RECOVERY_FAILED:
		break;
	case IOM3_RECOVERY_DOING:
		sts_recovery_iom3_handler();
		break;
	default:
		pr_err("%s -unknow state %ld\n", __func__, foo);
		break;
	}
	pr_info("%s -\n", __func__);
	return 0;
}

static struct notifier_block sts_recovery_notify = {
	.notifier_call = sts_recovery_notifier,
	.priority = -1,
};

static int __init sts_init(void)
{
	int ret;

	ret = register_mcu_event_notifier(TAG_IO_DIE, STS_CMD_STATUS_NOTIFY, sts_notify_handler);
	if (ret != 0) {
		pr_err( "register mcu event failed =%d\n", ret);
		return ret;
	}
	register_iom3_recovery_notifier(&sts_recovery_notify);

	g_sts_vote.vote_count[STS_USER_TP]++;
	g_sts_vote.total++;
	pr_info("%s: tp vote [%d].\n",__func__, g_sts_vote.vote_count[STS_USER_TP]);

	pr_info("%s ok\n", __func__);

	sts_data_reset();

	return 0;
}

static void __exit sts_exit(void)
{
	unregister_mcu_event_notifier(TAG_IO_DIE, STS_CMD_STATUS_NOTIFY, sts_notify_handler);
	pr_info("exit %s\n", __func__);
}

late_initcall_sync(sts_init);
module_exit(sts_exit);

