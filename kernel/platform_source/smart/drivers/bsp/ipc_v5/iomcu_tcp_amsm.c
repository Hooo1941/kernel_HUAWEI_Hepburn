/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2021-2022. All rights reserved.
 * Description: Contexthub ipc tcp amsm.
 * Create: 2021-10-14
 */
#include <linux/completion.h>
#include <platform_include/smart/linux/iomcu_status.h>
#include <platform_include/smart/linux/iomcu_dump.h>
#include <platform_include/smart/linux/iomcu_ipc.h>
#include <platform_include/basicplatform/linux/ipc_rproc.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/mutex.h>
#include <linux/semaphore.h>
#include <linux/slab.h>
#include <linux/spinlock.h>
#include <linux/types.h>
#include <linux/workqueue.h>
#include <linux/jiffies.h>
#include <securec.h>
#include <securectype.h>
#include "iomcu_link_ipc.h"
#include "iomcu_link_ipc_shm.h"

#define IOMCU_IPC_RX_WAIT_TIMEOUT     2000
#define NS_TO_MS                      1000000
#define SH_FAULT_IPC_RX_TIMEOUT_CNT   5
#define SH_FAULT_IPC_TX_TIMEOUT_CNT   32

struct iomcu_tcp_amsm_notifier_node {
	int (*notify)(const struct pkt_header *data);
	struct list_head entry;
	uint8_t tag;
	uint8_t cmd;
};

struct iomcu_tcp_amsm_notifier {
	struct list_head head;
	spinlock_t lock;
};

struct iomcu_tcp_amsm_mntn {
	atomic64_t head_max_latency_msecs;
	struct tcp_head_amsm head;
};

struct iomcu_event_waiter {
	struct completion complete;
	struct read_info *rd;
	uint8_t tag;
	uint8_t cmd;
	uint8_t tranid;
	struct list_head entry;
};

static struct mcu_event_wait_list {
	spinlock_t slock;
	struct list_head head;
} mcu_event_wait_list;

static int iomcu_tcp_amsm_recovery_notifier(struct notifier_block *nb, unsigned long foo, void *bar);
static struct iomcu_tcp_amsm_notifier tcp_amsm_notifier = {LIST_HEAD_INIT(tcp_amsm_notifier.head)};
static struct iomcu_tcp_amsm_mntn tcp_amsm_mntm;
static struct notifier_block tcp_amsm_notify = {
	.notifier_call = iomcu_tcp_amsm_recovery_notifier,
	.priority = -1,
};

static BLOCKING_NOTIFIER_HEAD(amsm_send_notifier_list);
static atomic_t g_amsm_tranid = ATOMIC_INIT(0);
static atomic_t g_atomic_recverr = ATOMIC_INIT(0);
static atomic_t g_atomic_senderr = ATOMIC_INIT(0);

static void init_mcu_event_wait_list(void)
{
	INIT_LIST_HEAD(&mcu_event_wait_list.head);
	spin_lock_init(&mcu_event_wait_list.slock);
}

static void init_wait_entry_add_list(struct iomcu_event_waiter *waiter)
{
	unsigned long flags = 0;

	init_completion(&waiter->complete);
	spin_lock_irqsave(&mcu_event_wait_list.slock, flags);
	list_add(&waiter->entry, &mcu_event_wait_list.head);
	spin_unlock_irqrestore(&mcu_event_wait_list.slock, flags);
}

static void list_del_iomcu_event_waiter(struct iomcu_event_waiter *self)
{
	unsigned long flags = 0;

	spin_lock_irqsave(&mcu_event_wait_list.slock, flags);
	list_del(&self->entry);
	spin_unlock_irqrestore(&mcu_event_wait_list.slock, flags);
}

static void iomcu_data_distribution(struct pkt_header *src, struct iomcu_event_waiter *des)
{
	struct pkt_subcmd_resp *head = (struct pkt_subcmd_resp *)src;
	int ret;
	int secfloor_size = (int)(sizeof(head->hd.errno) + sizeof(head->subcmd));

	if (des->rd == NULL)
		return;

	if (des->rd->data == NULL) {
		pr_err("%s : data null\n", __func__);
		return;
	}

	/* even cmds are resp cmd */
	if (0 == (src->cmd & 1)) {
		if (head->hd.length <= MAX_PKT_LENGTH + secfloor_size) {
			des->rd->errno = (int)(head->hd.errno);
			if (head->hd.length < secfloor_size) {
				pr_err("%s head->hd.length %u error\n", __func__, head->hd.length);
				return;
			}

			des->rd->data_length = (int)(head->hd.length) - secfloor_size;

			ret = memcpy_s(des->rd->data, MAX_PKT_LENGTH,
				(char *)head + sizeof(struct pkt_subcmd_resp),
				(size_t)des->rd->data_length);
			if (ret != EOK)
				pr_err("%s memcpy_s error\n", __func__);
		} else {
			des->rd->errno = -EINVAL;
			des->rd->data_length = 0;
			pr_err("data too large from mcu in %s\n", __func__);
		}
	}
}

static inline bool tag_cmd_match(uint8_t tag, uint8_t cmd, uint8_t tranid, struct pkt_header *recv)
{
	return (tag == recv->tag) && ((cmd + 1) == recv->cmd) && (tranid == recv->tranid);
}

static void wake_up_iomcu_event_waiter(struct pkt_header *head)
{
	unsigned long flags = 0;
	struct iomcu_event_waiter *pos = NULL;
	struct iomcu_event_waiter *n = NULL;
	unsigned char resp_ret = 0;

	spin_lock_irqsave(&mcu_event_wait_list.slock, flags);
	list_for_each_entry_safe(pos, n, &mcu_event_wait_list.head, entry) {
		if (tag_cmd_match(pos->tag, pos->cmd, pos->tranid, head)) {
			iomcu_data_distribution(head, pos);
			complete(&pos->complete);
			resp_ret++;
		}
	}

	if ((head->tag != TAG_LOG_BUFF) && (head->tag != TAG_SWING_CAM) && (head->tag != 0x3f) &&
		(head->cmd < CMD_DATA_PMU_PROFILE) && ((head->cmd & 1) == 0)) {
		pr_info("[%s]:tag[0x%x] cmd[0x%x] train_id[0x%x] ret[0x%x]\n",
			__func__, head->tag, head->cmd, head->tranid, resp_ret);
		if (resp_ret == 0) {
			list_for_each_entry_safe(pos, n, &mcu_event_wait_list.head, entry)
				pr_warn("[%s] list dump: tag[0x%x] cmd[0x%x] train_id[0x%x]\n",
					__func__, pos->tag, pos->cmd, pos->tranid);
		}
	}
	spin_unlock_irqrestore(&mcu_event_wait_list.slock, flags);
}


static void iomcu_tcp_amsm_latency_info(void)
{
	pr_info("head_max_latency_msecs:tag %x, cmd %x, %llu ms.\n",
		tcp_amsm_mntm.head.tag, tcp_amsm_mntm.head.cmd,
		(u64)atomic64_read(&tcp_amsm_mntm.head_max_latency_msecs));
}

static int iomcu_tcp_amsm_recovery_notifier(struct notifier_block *nb, unsigned long foo, void *bar)
{
	switch (foo) {
	case IOM3_RECOVERY_START:
		iomcu_tcp_amsm_latency_info();
	break;
	default: break;
	}
	return 0;
}

static void iomcu_tcp_amsm_notifier_handler(int *pkt_data, uint16_t pke_len)
{
	struct iomcu_tcp_amsm_notifier_node *pos = NULL;
	struct iomcu_tcp_amsm_notifier_node *n = NULL;
	int (*notify)(const struct pkt_header *data) = NULL;
	unsigned long flags = 0;
	const struct pkt_header *head = (struct pkt_header *)pkt_data;
	u64 entry_jiffies;
	u64 exit_jiffies;
	u64 diff_msecs;

	spin_lock_irqsave(&tcp_amsm_notifier.lock, flags);
	list_for_each_entry_safe(pos, n, &tcp_amsm_notifier.head, entry) {
		if ((pos->tag != head->tag) && (pos->tag != TAG_ALL))
			continue;

		if ((pos->cmd != head->cmd) && (pos->cmd != CMD_ALL))
			continue;

		if (pos->notify == NULL)
			continue;

		notify = pos->notify;

		spin_unlock_irqrestore(&tcp_amsm_notifier.lock, flags);

		entry_jiffies = get_jiffies_64();
		notify((const struct pkt_header *)pkt_data);
		exit_jiffies = get_jiffies_64();

		diff_msecs = jiffies64_to_nsecs(exit_jiffies - entry_jiffies) / NS_TO_MS;
		if (unlikely(diff_msecs > (u64)atomic64_read(&tcp_amsm_mntm.head_max_latency_msecs))) {
			atomic64_set(&tcp_amsm_mntm.head_max_latency_msecs, (s64)diff_msecs);

			tcp_amsm_mntm.head.tag = head->tag;
			tcp_amsm_mntm.head.cmd = head->cmd;
			pr_warn("iomcu head_max_latency_msecs 0x%x 0x%x %llums\n", head->tag, head->cmd, diff_msecs);
		}

		spin_lock_irqsave(&tcp_amsm_notifier.lock, flags);
	}
	spin_unlock_irqrestore(&tcp_amsm_notifier.lock, flags);
}

static int iomc_tcp_amsm_recv_handler(const void *buf, const uint16_t len)
{
	const struct tcp_head_amsm *head = (const struct tcp_head_amsm *)buf;
	struct pkt_header *pkt_head = NULL;
	struct iomcu_tcp_amsm_notifier_node *pos = NULL;
	struct iomcu_tcp_amsm_notifier_node *n = NULL;
	unsigned long flags = 0;
	uint32_t pkt_len;
	bool is_notifier = false;

	if (head == NULL || len < sizeof(struct tcp_head_amsm)) {
		pr_err("[%s] para err len %x.\n", __func__, len);
		return 0;
	}

	if (head->tag == TAG_SYS)
		pr_info("[%s]tag[0x%x]cmd[0x%x]len[%u]\n", __func__, head->tag, head->cmd, len);

	spin_lock_irqsave(&tcp_amsm_notifier.lock, flags);
	list_for_each_entry_safe(pos, n, &tcp_amsm_notifier.head, entry) {
		if ((pos->tag != head->tag) && (pos->tag != TAG_ALL))
			continue;

		if ((pos->cmd != head->cmd) && (pos->cmd != CMD_ALL))
			continue;

		is_notifier = true;
		break;
	}
	spin_unlock_irqrestore(&tcp_amsm_notifier.lock, flags);

	pkt_len = (uint32_t)(len + sizeof(struct pkt_header) - sizeof(struct tcp_head_amsm));
	pkt_head = (struct pkt_header *)kzalloc(pkt_len, GFP_ATOMIC);
	if (ZERO_OR_NULL_PTR((unsigned long)(uintptr_t)pkt_head)) {
		pr_err("[%s] kzalloc notifier error\n", __func__);
		return 0;
	}

	// set pkt_header to adpater legacy app.
	pkt_head->cmd = head->cmd;
	pkt_head->tag = head->tag;
	pkt_head->length = len - (uint16_t)sizeof(struct tcp_head_amsm);
	pkt_head->tranid = head->tranid;

	// copy app-payload.
	if (len > sizeof(struct tcp_head_amsm))
		(void)memcpy_s((uint8_t *)(&pkt_head[1]),
			       len - sizeof(struct tcp_head_amsm),
			       (void *)&head[1], len - sizeof(struct tcp_head_amsm));

	wake_up_iomcu_event_waiter(pkt_head);

	if (!is_notifier) {
		pr_err("[%s] unregistered tag %x, cmd %x.\n", __func__, head->tag, head->cmd);
		kfree((void *)pkt_head);
		pkt_head = NULL;
		return 0;
	}

	iomcu_tcp_amsm_notifier_handler((void *)pkt_head, (uint16_t)pkt_len);

	kfree((void *)pkt_head);
	pkt_head = NULL;
	return 0;
}

static int write_customize_cmd_para_check(const struct write_info *wr)
{
	if (wr == NULL) {
		pr_err("NULL pointer in %s\n", __func__);
		return -EINVAL;
	}

	if (wr->tag < TAG_BEGIN || wr->tag >= TAG_END || wr->wr_len < 0) {
		pr_err("[%s]tag[0x%x]orLen[0x%x]err\n", __func__, wr->tag, wr->wr_len);
		return -EINVAL;
	}

	if (wr->wr_len != 0 && wr->wr_buf == NULL) {
		pr_err("tag = %d, wr_len %d  error in %s\n", wr->tag, wr->wr_len, __func__);
		return -EINVAL;
	}

	if (wr->wr_len > ipc_shm_get_capacity()) {
		pr_err("[%s] wr_len %d  Max Len %d error.\n", __func__, wr->wr_len, ipc_shm_get_capacity());
		return -EINVAL;
	}

	return 0;
}

int write_customize_cmd(const struct write_info *wr, struct read_info *rd, bool is_lock)
{
	int ret;
	uint16_t send_length;
	struct tcp_head_amsm *tcp_head = NULL;
	unsigned long tcp_addr;
	bool resp = (rd != NULL);
	struct iomcu_event_waiter waiter;
	struct pkt_header *head = NULL;
	union dump_info info;

	if (get_contexthub_dts_status() != 0) {
		pr_err("[%s]do not send ipc as sensorhub is disabled\n", __func__);
		return -1;
	}

	ret = write_customize_cmd_para_check(wr);
	if (ret != 0)
		return ret;
	ret = iomcu_check_dump_status();
	if (ret != 0)
		return ret;

	pr_info("[%s] tag[0x%x] cmd[0x%x] len[%d]\n", __func__, wr->tag, wr->cmd, wr->wr_len);

	send_length =(uint16_t)(wr->wr_len + sizeof(struct tcp_head_amsm));
	tcp_addr = iomcu_link_ipc_get_buffer(send_length, (wr->cmd << 8) | wr->tag);
	if (tcp_addr == 0) {
		pr_err("[%s] get link buffer failed.\n", __func__);
		return -EINVAL;
	}

	tcp_head = (struct tcp_head_amsm*)(uintptr_t)tcp_addr;
	tcp_head->cmd = (uint8_t)wr->cmd;
	tcp_head->tag = (uint8_t)wr->tag;
	tcp_head->tranid = (uint8_t)atomic_inc_return(&g_amsm_tranid);

	if (resp) {
		waiter.rd = rd;
		waiter.tag = (uint8_t)wr->tag;
		waiter.cmd = (uint8_t)wr->cmd;
		waiter.tranid = tcp_head->tranid;
		init_wait_entry_add_list(&waiter);
	}

	(void)memcpy_s((void *)(tcp_head + 1), (size_t)wr->wr_len, wr->wr_buf, (size_t)wr->wr_len);

	ret = iomcu_link_ipc_send(1, TCP_AMSM, (void *)(uintptr_t)tcp_addr, send_length, resp);
	if (get_iom3_rec_state() == IOM3_RECOVERY_IDLE && (ret != 0)) {
		pr_warn("%s iom3 is recoverying [%d]\n", __func__, atomic_read(&g_atomic_senderr));
		if (atomic_inc_return(&g_atomic_senderr) > SH_FAULT_IPC_TX_TIMEOUT_CNT) {
			pr_err("SH_FAULT_IPC_TX_TIMEOUT tag[%u]cmd[%u]\n", wr->tag, wr->cmd);
			info.reg.tag = (uint8_t)wr->tag;
			info.reg.cmd = (uint8_t)wr->cmd;
			info.modid = SENSORHUB_MODID;
			save_dump_info_to_history(info);
			iom3_need_recovery(SENSORHUB_MODID, SH_FAULT_IPC_TX_TIMEOUT);
		}
	} else {
		atomic_set(&g_atomic_senderr, 0);
	}

	if (ret != 0) {
		pr_err("[%s] send tag[0x%x]cmd[0x%x] failed.\n", __func__, tcp_head->tag, tcp_head->cmd);
		(void)iomcu_link_ipc_put_buffer(tcp_addr, true);
		if (resp)
			list_del_iomcu_event_waiter(&waiter);
		return ret;
	}

	if (resp) {
		ret = (int)(wait_for_completion_timeout(&waiter.complete, msecs_to_jiffies(IOMCU_IPC_RX_WAIT_TIMEOUT)));
		if (ret == 0) {
			ret = -ETIME;
			pr_warn("[%s][repoTIMEOUT]tag[0x%x]cmd[0x%x]resp[%u]len[%u]\n",
				__func__, wr->tag, wr->cmd, resp, send_length);
			if (atomic_inc_return(&g_atomic_recverr) >
				SH_FAULT_IPC_RX_TIMEOUT_CNT) {
				pr_err("SH_FAULT_IPC_RX_TIMEOUT tag[%u]cmd[%u]\n", wr->tag, wr->cmd);
				info.reg.tag = (uint8_t)wr->tag;
				info.reg.cmd = (uint8_t)wr->cmd;
				save_dump_info_to_history(info);
				iom3_need_recovery(SENSORHUB_MODID, SH_FAULT_IPC_RX_TIMEOUT);
			}
		} else {
			ret = 0;
			atomic_set(&g_atomic_recverr, 0);
		}
	}

	head = (struct pkt_header *)(uintptr_t)tcp_addr;
	head->tag = (uint8_t)wr->tag;
	head->cmd = (uint8_t)wr->cmd;

	blocking_notifier_call_chain(&amsm_send_notifier_list, IOMCU_IPC_SEND, (void *)head);

	iomcu_link_ipc_put_buffer((unsigned long)tcp_addr, false);

	if (resp)
		list_del_iomcu_event_waiter(&waiter);

	if (wr->tag == TAG_AOD)
		return ret;
	else
		return 0;
}
EXPORT_SYMBOL(write_customize_cmd);

int register_mcu_send_cmd_notifier(struct notifier_block *nb)
{
	return blocking_notifier_chain_register(&amsm_send_notifier_list, nb);
}

int unregister_mcu_send_cmd_notifier(struct notifier_block *nb)
{
	return blocking_notifier_chain_unregister(&amsm_send_notifier_list, nb);
}

int register_mcu_event_notifier(int tag, int cmd, int (*notify)(const struct pkt_header *head))
{
	struct iomcu_tcp_amsm_notifier_node *pnode = NULL;
	int ret = 0;
	unsigned long flags = 0;

	if ((!(tag >= TAG_BEGIN && tag <= TAG_END)) || notify == NULL) {
		pr_err("%s: ++ invalid tag %x \n", __func__, tag);
		return -EINVAL;
	}

	spin_lock_irqsave(&tcp_amsm_notifier.lock, flags);
	list_for_each_entry(pnode, &tcp_amsm_notifier.head, entry) {
		if ((tag == pnode->tag) && (cmd == pnode->cmd) && (notify == pnode->notify)) {
				pr_warn("[%s] tag[0x%x]cmd[0x%x] already registed\n!", __func__, tag, cmd);
				goto out;
			}
	}

	pnode = kzalloc(sizeof(struct iomcu_tcp_amsm_notifier_node), GFP_ATOMIC);
	if (ZERO_OR_NULL_PTR((unsigned long)(uintptr_t)pnode)) {
		ret = -ENOMEM;
		goto out;
	}
	pnode->tag = (uint8_t)tag;
	pnode->cmd = (uint8_t)cmd;
	pnode->notify = notify;

	list_add(&pnode->entry, &tcp_amsm_notifier.head);
out:
	spin_unlock_irqrestore(&tcp_amsm_notifier.lock, flags);
	return ret;
}
EXPORT_SYMBOL(register_mcu_event_notifier);

int unregister_mcu_event_notifier(int tag, int cmd, int (*notify)(const struct pkt_header *head))
{
	struct iomcu_tcp_amsm_notifier_node *pos = NULL;
	struct iomcu_tcp_amsm_notifier_node *n = NULL;
	unsigned long flags = 0;

	if ((!(tag >= TAG_BEGIN && tag <= TAG_END)) || notify == NULL)
		return -EINVAL;

	spin_lock_irqsave(&tcp_amsm_notifier.lock, flags);
	list_for_each_entry_safe(pos, n, &tcp_amsm_notifier.head, entry) {
		if ((tag == pos->tag) && (cmd == pos->cmd) && (notify == pos->notify)) {
			list_del(&pos->entry);
			kfree(pos);
			pos = NULL;
			break;
		}
	}
	spin_unlock_irqrestore(&tcp_amsm_notifier.lock, flags);
	return 0;
}
EXPORT_SYMBOL(unregister_mcu_event_notifier);

static int iomcu_tcp_amsm_init(void)
{
	int ret;

	(void)memset_s(&tcp_amsm_mntm, sizeof(struct iomcu_tcp_amsm_mntn), 0, sizeof(struct iomcu_tcp_amsm_mntn));

	INIT_LIST_HEAD(&tcp_amsm_notifier.head);
	spin_lock_init(&tcp_amsm_notifier.lock);

	init_mcu_event_wait_list();

	ret = iomcu_link_ipc_recv_register(TCP_AMSM, iomc_tcp_amsm_recv_handler);
	if (ret != 0)
		pr_err("[%s] iomcu_link_ipc_recv_register TCP_AMSM faild\n", __func__);

	ret = register_iom3_recovery_notifier(&tcp_amsm_notify);
	if (ret != 0)
		pr_err("[%s] register_iom3_recovery_notifier faild\n", __func__);

	return 0;
}

static void __exit iomcu_tcp_amsm_exit(void)
{
	iomcu_link_ipc_recv_unregister(TCP_AMSM, iomc_tcp_amsm_recv_handler);

	unregister_iom3_recovery_notifier(&tcp_amsm_notify);
}

device_initcall_sync(iomcu_tcp_amsm_init);
module_exit(iomcu_tcp_amsm_exit);

MODULE_DESCRIPTION("iomcu tcp layer - amsm");
MODULE_LICENSE("GPL");
