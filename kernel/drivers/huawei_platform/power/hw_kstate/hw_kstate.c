/*
 * hw_kstate.c
 *
 * This file use to send kernel state
 *
 * Copyright (c) 2014-2020 Huawei Technologies Co., Ltd.
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

#include <huawei_platform/power/hw_kstate.h>
#include <net/sock.h>
#include <securec.h>
#include <linux/kernel.h>
#include <linux/kfifo.h>
#include <linux/kthread.h>
#include <linux/module.h>
#include <linux/netlink.h>
#include <linux/skbuff.h>
#include <linux/spinlock.h>

#define KSTATE_FIFO_SIZE	4096
#define MAX_LENGTH	(1024 * 1024)
#define KSTATE_THREAD_NAME	"hw_kstate"

static struct task_struct *kstate_thread = NULL;
static struct kfifo kstate_fifo; /* fifo is full of pointer to struct ksmsg */
static spinlock_t kstate_kfifo_lock;
static spinlock_t kstate_socks_lock;
DEFINE_MUTEX(kstate_hook_lock);
static struct list_head kstate_hooks;
static struct list_head kstate_socks;

static DECLARE_WAIT_QUEUE_HEAD(kstate_wait_queue);

struct kstate_sock {
	struct list_head list;
	int user_pid;
	int net_level;
	struct sock *sk;
};

/*
 * check channel valid or not
 * @param channel: channel to check
 * @return false for invalid, true for valid
 */
static inline bool is_channel_valid(int channel)
{
	return ((channel > CHANNEL_ID_NONE) && (channel < CHANNEL_ID_END));
}

/*
 * check packet tag valid or not
 * @param tag: tag to check
 * @return false for invalid, true for valid
 */
static inline bool is_tag_valid(int tag)
{
	return ((tag > PACKET_TAG_NONE) && (tag < PACKET_TAG_END));
}

/*
 * send message to user space and msg
 * @param kmsg: source msg
 * @return -1 for failed, 0 for successed
 */
static int send_to_user(struct kstate_msg *kmsg)
{
	struct sk_buff *skb = NULL;
	struct nlmsghdr *nlh = NULL;
	struct kstate_sock *kstate_sk = NULL;
	int len;
	int ret  = -1;
	if (IS_ERR_OR_NULL(kmsg)) {
		pr_err("hw_kstate %s: kmsg is NULL!\n", __func__);
		return ret;
	}

	len = sizeof(struct ksmsg) + kmsg->msg.length;
	if (len > MAX_LENGTH) { /* the default size */
		pr_err("hw_kstate %s: invalid len: %d!\n", __func__, len);
		return ret;
	}

	/* send up msg */
	spin_lock_bh(&kstate_socks_lock);
	list_for_each_entry(kstate_sk, &kstate_socks, list) {
		if (kstate_sk->user_pid <= 0)
			continue;

		if (kmsg->pid_level != kstate_sk->net_level)
			continue;

		skb = alloc_skb(NLMSG_SPACE(len), GFP_ATOMIC);
		if (IS_ERR_OR_NULL(skb)) {
			pr_err("hw_kstate %s: alloc skb failed!\n", __func__);
			continue;
		}

		nlh = nlmsg_put(skb, 0, 0, 0, len, 0);
		if (memcpy_s(NLMSG_DATA(nlh), len, &kmsg->msg, len) != EOK) {
			pr_err("hw_kstate %s: fail to memcpy_s!\n", __func__);
			kfree_skb(skb);
			continue;
		}

		ret = netlink_unicast(kstate_sk->sk, skb, kstate_sk->user_pid, MSG_DONTWAIT);
		if (ret < 0)
			pr_info("hw_kstate %s unicast fail:%d", __func__, ret);
	}

	spin_unlock_bh(&kstate_socks_lock);
	return ret;
}

/*
 * receive data from module and process it here
 * @param data: source data
 */
static int fifo_out(void *data) {
	struct kstate_msg *msg = NULL;

	while (1) {
		/* wait module data */
		wait_event_interruptible(kstate_wait_queue, !kfifo_is_empty(&kstate_fifo));
		/* fifo has data */
		while (kfifo_out(&kstate_fifo, &msg, sizeof(struct kstate_msg*)) != 0) {
			/* send msg to user space */
			send_to_user(msg);
			kfree(msg);
		}
	}
	return 0;
}

/*
 * receive data from module and save it to fifo
 * @param msg: msg to save
 * @return -1 for failed, 0 for successed
 */
static int fifo_in(struct kstate_msg *msg)
{
	int ret = -1;

	if (IS_ERR_OR_NULL(msg)) {
		pr_err("hw_kstate %s: the msg point is err!\n", __func__);
		goto err;
	}

	/* put data in kfifo */
	spin_lock_bh(&kstate_kfifo_lock);
	if (kfifo_in(&kstate_fifo, &msg, sizeof(struct kstate_msg*)) == 0) {
		/* put data failed */
		if (kfifo_is_full(&kstate_fifo)) {
			if (send_to_user(msg) < 0)
				pr_err("hw_kstate %s: send msg to user failed!\n", __func__);
			kfree(msg);
		} else {
			pr_err("hw_kstate %s: kfifo in error!\n", __func__);
			kfree(msg);
		}
	} else {
		/* put data successfully */
		ret = 0;
		wake_up_interruptible(&kstate_wait_queue);
	}

	spin_unlock_bh(&kstate_kfifo_lock);

err:
	return ret;
}

/*
 * receive data from user space and process it here
 * @param msg: source msg
 */
static void process_us_data(struct ksmsg *msg)
{
	struct kstate_opt *elem = NULL;
	struct kstate_msg tmp_msg;
	if (IS_ERR_OR_NULL(msg)) {
		pr_err("hw_kstate %s: msg is NULL!\n", __func__);
		return;
	}

	if (PACKET_TAG_VALIDATE_CHANNEL == msg->tag) {
		/* confirm connection is still online */
		tmp_msg.msg.tag = PACKET_TAG_VALIDATE_CHANNEL;
		tmp_msg.msg.src = CHANNEL_ID_NETLINK;
		tmp_msg.msg.dst = CHANNEL_ID_NONE;
		tmp_msg.msg.length = 0;
		tmp_msg.pid_level = (current != NULL && current->thread_pid != NULL) ? current->thread_pid->level : 0;
		send_to_user(&tmp_msg);
	} else {
		/* receive msg from user */
		if (CHANNEL_ID_NETLINK != msg->dst) {
			pr_err("hw_kstate %s: channel=%u is not netlink!\n", __func__, msg->dst);
			return;
		}
		/* calling corresponding hook func */
		mutex_lock(&kstate_hook_lock);
		list_for_each_entry(elem, &kstate_hooks, list) {
			if (elem->tag == msg->tag) {
				if (!IS_ERR_OR_NULL(elem->hook))
					elem->hook(msg->src, msg->tag, msg->buffer, msg->length);
			}
		}
		mutex_unlock(&kstate_hook_lock);
	}
}

static void update_user_pid(struct sk_buff *skb, int pid)
{
	struct kstate_sock *kstate_sk = NULL;
	if (pid <= 0 || IS_ERR_OR_NULL(skb) || IS_ERR_OR_NULL(skb->sk))
		return;

	spin_lock_bh(&kstate_socks_lock);
	list_for_each_entry(kstate_sk, &kstate_socks, list) {
		if (sock_net(kstate_sk->sk) == sock_net(skb->sk)) {
			kstate_sk->user_pid = pid;
			break;
		}
	}
	spin_unlock_bh(&kstate_socks_lock);
}

/*
 * receive command from user calling process_us_data() to process it
 * @param skb: source command
 */
static void recv_from_user(struct sk_buff *skb)
{
	struct sk_buff *tmp_skb = NULL;
	struct nlmsghdr *nlh = NULL;
	struct ksmsg *msg = NULL;
	struct ksmsg *data = NULL;
	int len = 0;

	if (IS_ERR_OR_NULL(skb)) {
		pr_err("hw_kstate %s: skb is NULL!\n", __func__);
		return;
	}
	tmp_skb = skb;
	if (tmp_skb->len >= NLMSG_SPACE(0)) {
		nlh = nlmsg_hdr(tmp_skb);
		if (nlh->nlmsg_pid > 0)
			update_user_pid(skb, nlh->nlmsg_pid);

		len = NLMSG_PAYLOAD(nlh, 0);
		if (nlh->nlmsg_len < NLMSG_LENGTH(sizeof(struct ksmsg))) {
			pr_err("hw_kstate %s: invalid msg length!\n", __func__);
			return;
		}
		data = (struct ksmsg*)NLMSG_DATA(nlh);
		if ((len >= (sizeof(struct ksmsg) + data->length))
			&& (len < MAX_LENGTH) /* default size */
			&& (len > 0)
			&& (tmp_skb->len >= nlh->nlmsg_len)) {
			msg = (struct ksmsg*)kmalloc(len, GFP_ATOMIC);
			if (IS_ERR_OR_NULL(msg)) {
				pr_err("hw_kstate %s: msg is NULL!\n", __func__);
				return;
			}

			if (memcpy_s(msg, len, data, len) != EOK) {
				pr_err("hw_kstate %s: failed to memcpy_s!\n", __func__);
				kfree(msg);
				return;
			}

			process_us_data(msg);
			kfree(msg);
		} else {
			pr_err("hw_kstate %s: length err, skb len:%d nlmsg_len:%u length in ksmsg: %u\n",\
					__func__, skb->len, nlh->nlmsg_len, data->length);
		}
	}
}

/*
 * kernel information entry
 * @param dst: user to send
 * @param tag: packet tag
 * @param data: source data
 * @param len: data len
 * @return -1 for failed, 0 for successed
 */
int kstate(channel_id dst, packet_tag tag, const char *data, size_t len)
{
	struct kstate_msg *kmsg = NULL;
	int msg_len;

	if (!is_channel_valid(dst)) {
		pr_err("hw_kstate %s: channel is invalid!\n", __func__);
		return -1;
	}

	if (!is_tag_valid(tag)) {
		pr_err("hw_kstate %s: tag is invalid!\n", __func__);
		return -1;
	}

	if (IS_ERR_OR_NULL(data)) {
		pr_err("hw_kstate %s: data is NULL!\n", __func__);
		return -1;
	}

	/* kmsg will be freed in fifo_in() or send_to_user() */
	msg_len = sizeof(struct kstate_msg) + len;
	kmsg = (struct kstate_msg*)kmalloc(msg_len, GFP_ATOMIC);
	if (IS_ERR_OR_NULL(kmsg)) {
		pr_err("hw_kstate %s: kmsg is NULL!\n", __func__);
		return -1;
	}

	if (memset_s(kmsg, msg_len, 0, msg_len) != EOK) {
		pr_err("hw_kstate %s: fail to memset_s!\n", __func__);
		kfree(kmsg);
		return -1;
	}

	kmsg->pid_level = (current != NULL && current->thread_pid != NULL) ? current->thread_pid->level : 0;
	kmsg->msg.tag = tag;
	kmsg->msg.src = CHANNEL_ID_NETLINK;
	kmsg->msg.dst = dst;
	kmsg->msg.length = len;
	if (memcpy_s(kmsg->msg.buffer, len, data, len) != EOK) {
		pr_err("hw_kstate %s: fail to memcpy_s!\n", __func__);
		kfree(kmsg);
		return -1;
	}

	if (fifo_in(kmsg) < 0) {
		pr_err("hw_kstate %s: save kmsg from module failed!\n", __func__);
		return -1;
	}

	return 0;
}

/*
 * register hook function to kstate
 * @param opt: kstate_opt struct to register
 * @return -1 for failed, 0 for successed
 */
int kstate_register_hook(struct kstate_opt *opt)
{
	struct kstate_opt *elem = NULL;
	int ret = -1;

	if (IS_ERR_OR_NULL(opt)) {
		pr_err("hw_kstate %s: kstate_opt is NULL!\n", __func__);
		goto err;
	}

	if (!is_channel_valid(opt->dst)) {
		pr_err("hw_kstate %s: kstate channel=%u is invalid!\n", __func__, opt->dst);
		goto err;
	}

	if (!is_tag_valid(opt->tag)) {
		pr_err("hw_kstate %s: kstate tag=%u is invalid!\n", __func__, opt->tag);
		goto err;
	}

	mutex_lock(&kstate_hook_lock);
	list_for_each_entry(elem, &kstate_hooks, list) {
		if (elem->tag == opt->tag) {
			ret = 0;
			goto out;
		}
	}
	list_add_tail(&opt->list, &kstate_hooks);
	ret = 0;

out:
	mutex_unlock(&kstate_hook_lock);

err:
	return ret;
}

/*
 * unregister hook function from kstate
 * @param opt: kstate_opt struct to unregister
 */
void kstate_unregister_hook(struct kstate_opt *opt)
{
	struct kstate_opt *elem = NULL;
	struct kstate_opt *n = NULL;

	if (IS_ERR_OR_NULL(opt)) {
		pr_err("hw_kstate %s: kstate_opt is NULL!\n", __func__);
		return;
	}

	mutex_lock(&kstate_hook_lock);
	list_for_each_entry_safe(elem, n, &kstate_hooks, list) {
		if (opt == elem) {
			list_del(&elem->list);
			break;
		}
	}
	mutex_unlock(&kstate_hook_lock);
}

/*
 * release netlink sock from skb queue
 */
static void release_netlink_sock(void)
{
	struct kstate_sock *kstate_sk = NULL;
	spin_lock_bh(&kstate_socks_lock);
	list_for_each_entry(kstate_sk, &kstate_socks, list) {
		pr_info("hw_kstate %s net:%d", __func__, kstate_sk->net_level);
		list_del(&kstate_sk->list);
		netlink_kernel_release(kstate_sk->sk);
		kfree(kstate_sk);
	}
	spin_unlock_bh(&kstate_socks_lock);
}

static void __net_exit kstate_net_exit(struct net *net)
{
	struct kstate_sock *kstate_sk = NULL;
	spin_lock_bh(&kstate_socks_lock);
	list_for_each_entry(kstate_sk, &kstate_socks, list) {
		if (sock_net(kstate_sk->sk) == net)
			goto found;
	}
	spin_unlock_bh(&kstate_socks_lock);
	return;

found:
	pr_info("hw_kstate %s net:%d", __func__, kstate_sk->net_level);
	list_del(&kstate_sk->list);
	spin_unlock_bh(&kstate_socks_lock);
	netlink_kernel_release(kstate_sk->sk);
	kfree(kstate_sk);
}

static int __net_init kstate_net_init(struct net *net)
{
	struct netlink_kernel_cfg cfg = {
		.groups = 0,
		.input = recv_from_user,
	};

	struct kstate_sock *kstate_sk;
	kstate_sk = kzalloc(sizeof(*kstate_sk), GFP_ATOMIC);
	if (!kstate_sk)
		return -ENOMEM;

	kstate_sk->sk = netlink_kernel_create(net, NETLINK_HW_KSTATE, &cfg);
	if (kstate_sk->sk == NULL) {
		pr_info("%s netlink create error", __func__);
		return -ENOMEM;
	}
	kstate_sk->user_pid = -1;
	kstate_sk->net_level = (net == &init_net) ? 0: 1;

	spin_lock_bh(&kstate_socks_lock);
	pr_info("hw_kstate %s net level:%d, add list", __func__, kstate_sk->net_level);
	list_add_tail(&kstate_sk->list, &kstate_socks);
	spin_unlock_bh(&kstate_socks_lock);

	return 0;
}

static struct pernet_operations kstate_net_ops = {
	.init = kstate_net_init,
	.exit = kstate_net_exit,
};

static int __init kstate_init(void)
{
	int ret = -1;

	INIT_LIST_HEAD(&kstate_hooks);
	INIT_LIST_HEAD(&kstate_socks);

	spin_lock_init(&kstate_kfifo_lock);
	spin_lock_init(&kstate_socks_lock);

	/* create netlink sock to receive command from user space */
	register_pernet_subsys(&kstate_net_ops);

	/* init fifo */
	ret = kfifo_alloc(&kstate_fifo, KSTATE_FIFO_SIZE , GFP_ATOMIC);
	if (0 != ret) {
		pr_err("hw_kstate %s: error kfifo_alloc!\n", __func__);
		goto err_alloc_kfifo;
	}

	/* run thread */
	kstate_thread = kthread_run(fifo_out, NULL, KSTATE_THREAD_NAME);
	if (IS_ERR_OR_NULL(kstate_thread)) {
		pr_err("hw_kstate %s: error create thread!\n", __func__);
		goto err_create_thread;
	}

	pr_info("hw_kstate %s: inited!\n", __func__);

	return 0;

err_create_thread:
	kfifo_free(&kstate_fifo);

err_alloc_kfifo:
	release_netlink_sock();

err_create_netlink:
	return ret;
}

static void __exit kstate_exit(void)
{
	if (!IS_ERR_OR_NULL(kstate_thread)) {
		kthread_stop(kstate_thread);
	}
	kfifo_free(&kstate_fifo);

	release_netlink_sock();
}

module_init(kstate_init);
module_exit(kstate_exit);
MODULE_LICENSE("GPL v2");
MODULE_DESCRIPTION("use to send kernel state");
MODULE_AUTHOR("Huawei Technologies Co., Ltd.");
