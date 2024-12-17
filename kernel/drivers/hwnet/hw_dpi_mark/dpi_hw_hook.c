

#include "dpi_hw_hook.h"
#include <linux/init.h>
#include <linux/types.h>
#include <linux/string.h>
#include <linux/uaccess.h>
#include <linux/netdevice.h>
#include <linux/netfilter_ipv4.h>
#include <linux/ip.h>
#include <linux/tcp.h>
#include <linux/udp.h>
#include <linux/errno.h>
#include <net/udp.h>
#include <linux/kernel.h> /* add for log */
#include <linux/ctype.h> /* add for tolower */
#include <linux/file.h>
#include <linux/rbtree.h>
#include <linux/inetdevice.h>
#include <linux/netdevice.h>
#include <linux/time.h>
#include <net/sock.h>
#include <hwnet/ipv4/wifipro_tcp_monitor.h>
#include <linux/spinlock.h> /* add for spinlock */
#include <linux/netlink.h> /* add for thread */
#include <uapi/linux/netlink.h> /* add for netlink */
#include <linux/kthread.h> /* add for thread */
#include <linux/version.h>

#define REPORTCMD NETLINK_DOWNLOADEVENT_TO_USER
#define MPLK_SK_STRATEGY_CT (1 << 0)
#define MPLK_SK_STRATEGY_FUTE (1 << 1)
#define MPLK_SK_STRATEGY_FURE (1 << 2)

/* the tag to decide whether to mark or not, default off */
static int g_mark_tag;

/* netlink message format */
struct tag_hw_msg2knl {
	struct nlmsghdr hdr;
	int opt;
	char data[1];
};

/* DPI rule mask format */
typedef struct {
	uint32_t tmgp_uid;
	uint8_t tmgp_tp;
	uint32_t tmgp_mark;
} dpi_rule_mask;

/* DPI rule format */
typedef struct {
	dmr_match_type_t rule_type;
	union {
		uint8_t match_tp_val;
	} rule_body; /* rule body varies according to rule_type */
	uint32_t mark_num;
} dpi_rule_t;

/* dpi_mark_rule for one APP */
typedef struct {
	uid_t dmr_app_uid;
	uint32_t dmr_mplk_netid;
	uint32_t dmr_mplk_strategy;
	dpi_rule_t dmr_rule;
} dpi_mark_rule_t;

static struct sock *g_hw_nlfd;
static unsigned int g_uspa_pid;

#define MPLK_MAX_UID_SOCK_ENTRY_NUM 1024
#define MPLK_NW_BIND_UID_MIN_LIMIT 10000
#define MPLK_SOMARK_SELECTED_OR_VPN_MASK 0x30000
#define MPLK_FWMARK_CONTROL_MASK 0xFFFF0000
#define MPLK_FWMARK_NETID_MASK 0xFFFF
#define MPLK_SOMARK_UNSET 0
#define MPLK_UID_NEED_DELETE 1
#define MPLK_UID_NO_NEED_DELETE 0
#define MPLK_BIND_TO_CELL 1
#define MPLK_UNBIND_TO_CELL 0

/* wait 4s cause game heart beat is 3s */
#define MPLK_TREE_DEL_TIME_WAIT (4 * HZ)

static struct rb_root uid_netid_tree = RB_ROOT;
static int uid_netid_entry_num;
static rwlock_t uid_netid_tree_lock;
static rwlock_t g_dpi_rule_lock;

struct uid_netid {
	struct rb_node uid_netid_node;
	uint32_t netid;
	uid_t uid;
	unsigned long bind_stamp;
	u32 strategy;
	u32 need_delete;
	struct timer_list delete_timer;
};

static struct uid_netid *mplk_uid_netid_tree_search(struct rb_root *root,
	const uid_t uid)
{
	struct rb_node *node = root->rb_node;

	while (node != NULL) {
		struct uid_netid *data = rb_entry(node, struct uid_netid,
			uid_netid_node);
		if (uid < data->uid)
			node = node->rb_left;
		else if (uid > data->uid)
			node = node->rb_right;
		else
			return data;
	}
	return NULL;
}

static void mplk_uid_netid_tree_insert(struct uid_netid *data,
	struct rb_root *root)
{
	struct rb_node **new = &(root->rb_node);
	struct rb_node *parent = NULL;

	/* Figure out where to put new node */
	while (*new != NULL) {
		struct uid_netid *this = rb_entry(*new, struct uid_netid,
			uid_netid_node);
		parent = *new;
		if (data->uid < this->uid) {
			new = &((*new)->rb_left);
		} else if (data->uid > this->uid) {
			new = &((*new)->rb_right);
		} else {
			WIFIPRO_WARNING("BUG: dU:%d==tU:%d\n", data->uid,
				this->uid);
			return;
		}
	}

	/* Add new node and rebalance tree. */
	rb_link_node(&data->uid_netid_node, parent, new);
	rb_insert_color(&data->uid_netid_node, root);
	WIFIPRO_VERBOSE("inert a new uid_netid for %u\n", data->uid);
}

static struct uid_netid *mplk_get_uid_netid_nl(const uid_t uid)
{
	return mplk_uid_netid_tree_search(&uid_netid_tree, uid);
}

static void mplk_uid_search_need_delete_uid(unsigned int uid)
{
	struct uid_netid *uid_netid_entry = NULL;

	WIFIPRO_DEBUG("%s: enter", __func__);

	write_lock_bh(&uid_netid_tree_lock);
	uid_netid_entry = mplk_get_uid_netid_nl(uid);
	if (uid_netid_entry != NULL) {
		WIFIPRO_DEBUG("%s: uid need to delete = %d",
			__func__, uid);
		rb_erase(&uid_netid_entry->uid_netid_node, &uid_netid_tree);
		kfree(uid_netid_entry);
		uid_netid_entry_num--;
	}
	write_unlock_bh(&uid_netid_tree_lock);
}
#if LINUX_VERSION_CODE < KERNEL_VERSION(4,15,0)
static void mplk_delete_timer_handle(unsigned long data)
{
	struct uid_netid *netid_entry = (struct uid_netid *)(uintptr_t)data;
	mplk_uid_search_need_delete_uid(netid_entry->uid);
}
#else
static void mplk_delete_timer_handle(struct timer_list *t)
{
	struct uid_netid *netid_entry = from_timer(netid_entry, t, delete_timer);
	mplk_uid_search_need_delete_uid(netid_entry->uid);
}
#endif

static void mplk_setup_uid_delete_timer(struct uid_netid *node)
{
#if LINUX_VERSION_CODE < KERNEL_VERSION(4, 15, 0)
	init_timer(&node->delete_timer);
	node->delete_timer.function = mplk_delete_timer_handle;
	node->delete_timer.data = (unsigned long)(uintptr_t)node;
#else
	timer_setup(&node->delete_timer, mplk_delete_timer_handle, 0);
#endif
	node->delete_timer.expires = jiffies + MPLK_TREE_DEL_TIME_WAIT;

	add_timer(&node->delete_timer);
}

static void mplk_check_need_cancel_delete(struct uid_netid *uid_netid_entry)
{
	if (uid_netid_entry != NULL) {
		if (uid_netid_entry->need_delete) {
			uid_netid_entry->need_delete
				= MPLK_UID_NO_NEED_DELETE;
			del_timer_sync(&uid_netid_entry->delete_timer);
			WIFIPRO_DEBUG("%s: timer deleted",  __func__);
		}
	}
}

void mplk_add_nw_bind(uid_t uid, uint32_t netid)
{
	struct uid_netid *uid_netid_entry = NULL;

	if (uid < MPLK_NW_BIND_UID_MIN_LIMIT)
		return;

	write_lock_bh(&uid_netid_tree_lock);

	if (uid_netid_entry_num > MPLK_MAX_UID_SOCK_ENTRY_NUM) {
		WIFIPRO_WARNING("max entry reached\n");
		write_unlock_bh(&uid_netid_tree_lock);
		return;
	}

	uid_netid_entry = mplk_get_uid_netid_nl(uid);
	mplk_check_need_cancel_delete(uid_netid_entry);
	if (uid_netid_entry == NULL) {
		/* create a new uid_netid_entry. */
		uid_netid_entry = kzalloc(sizeof(*uid_netid_entry), GFP_ATOMIC);
		if (uid_netid_entry == NULL) {
			WIFIPRO_WARNING("alloc uid_netid_entry fail\n");
			write_unlock_bh(&uid_netid_tree_lock);
			return;
		}
		uid_netid_entry_num++;
		uid_netid_entry->uid = uid;
		mplk_uid_netid_tree_insert(uid_netid_entry, &uid_netid_tree);
	}
	if (uid_netid_entry->netid) {
		WIFIPRO_INFO("change %u from %u to %u then num=%d\n",
			uid, uid_netid_entry->netid, netid,
			uid_netid_entry_num);
	} else {
		WIFIPRO_INFO("add a new bind for %u to %u then num=%u\n",
			uid, netid, uid_netid_entry_num);
	}
	if (netid > 0)
		uid_netid_entry->netid = netid;

	/* update them at the time of close sk. */
	uid_netid_entry->strategy = 0;
	uid_netid_entry->bind_stamp = 0;
	uid_netid_entry->need_delete = MPLK_UID_NO_NEED_DELETE;

	write_unlock_bh(&uid_netid_tree_lock);
}

void mplk_add_nw_bind_ex(const char *data, u32 len)
{
	dpi_mark_rule_t *p_dmr = NULL;
	if (data == NULL) {
		pr_info("dpi_hw_hook proc_cmd NETLINK_MPLK_BIND_NETWORK p_dmr is null\n");
		return;
	}
	if (len < sizeof(dpi_mark_rule_t)) {
		pr_info("dpi_hw_hook proc_cmd NETLINK_MPLK_BIND_NETWORK input len is wrong\n");
		return;
	}

	p_dmr = (dpi_mark_rule_t *)data;
	mplk_add_nw_bind(p_dmr->dmr_app_uid, p_dmr->dmr_mplk_netid);
}

void mplk_del_nw_bind(uid_t uid)
{
	struct uid_netid *uid_netid_entry = NULL;

	if (uid < MPLK_NW_BIND_UID_MIN_LIMIT)
		return;

	write_lock_bh(&uid_netid_tree_lock);

	uid_netid_entry = mplk_get_uid_netid_nl(uid);
	if (!uid_netid_entry) {
		WIFIPRO_WARNING("%u not found\n", uid);
		write_unlock_bh(&uid_netid_tree_lock);
		return;
	}

	uid_netid_entry->need_delete = MPLK_UID_NEED_DELETE;
	uid_netid_entry->netid = 0;
	if (!timer_pending(&uid_netid_entry->delete_timer)) {
		mplk_setup_uid_delete_timer(uid_netid_entry);
		WIFIPRO_DEBUG("start timer to close old sk for %u when %lu\n",
			uid, uid_netid_entry->bind_stamp);
	} else {
		WIFIPRO_DEBUG("uid_netid_entry->delete_timer is pending\n");
	}

	write_unlock_bh(&uid_netid_tree_lock);
}

/*
 * Close all sockets of uid.
 *
 * 1, The low 4 bits of strategy, respectively, indicate: FURE-FUTE-CT.
 *       As follows:
 * CT   :close TCP socket by Netd.
 *       (Value in low 4 bit of strategy is 0001, 0x1)
 * FUTE :fake UDP socket TX error.
 *       (Value in low 4 bit of strategy is 0010, 0x2)
 * FURE :fake UDP socket RX error.
 *       (Value in low 4 bit of strategy is 0100, 0x4)
 */
void mplk_close_socket_by_uid(uint32_t strategy, uid_t uid)
{
	struct uid_netid *uid_netid_entry = NULL;

	if (uid < MPLK_NW_BIND_UID_MIN_LIMIT)
		return;

	/* close current uid's udp sock by strategy. */
	write_lock_bh(&uid_netid_tree_lock);

	uid_netid_entry = mplk_get_uid_netid_nl(uid);
	if (uid_netid_entry == NULL) {
		WIFIPRO_WARNING("%u not found\n", uid);
		write_unlock_bh(&uid_netid_tree_lock);
		return;
	}
	uid_netid_entry->strategy = strategy;
	uid_netid_entry->bind_stamp = jiffies;
	WIFIPRO_INFO("close old udp sk for %u when %lu with S:%u\n",
		uid, uid_netid_entry->bind_stamp, strategy);

	write_unlock_bh(&uid_netid_tree_lock);
}

static int mplk_rebind_sock(struct sock *sk, int is_bind, uint32_t netid)
{
	if (sk == NULL)
		return 0;

	lock_sock(sk);
	inet_sk(sk)->inet_rcv_saddr = 0;
	inet_sk(sk)->inet_saddr = 0;
	sk->sk_userlocks &= ~SOCK_BINDADDR_LOCK;
	sk->sk_prot->rehash(sk);

	if (is_bind) {
		sk->sk_mark = (sk->sk_mark & MPLK_FWMARK_CONTROL_MASK) |
			(netid & MPLK_FWMARK_NETID_MASK);
		WIFIPRO_DEBUG("bind to cell, set sk->sk_mark = %d\n",
			sk->sk_mark);
	} else {
		sk->sk_mark = MPLK_SOMARK_UNSET;
		WIFIPRO_DEBUG("unbind to cell, set sk->sk_mark to 0\n");
	}

	sk_dst_reset(sk);

	if (inet_sk(sk)->inet_saddr)
		inet_sk(sk)->inet_saddr = 0;

	release_sock(sk);
	return 0;
}

static int mplk_trigger_socket_reroute(struct sock *sk, int is_bind,
	uint32_t netid)
{
	int err;

	if (sk == NULL)
		return 0;

	if (sk->sk_protocol == IPPROTO_TCP) {
		err = -EPIPE;
		WIFIPRO_DEBUG("trigger tcp err for %u\n", sk->sk_uid.val);
	} else {
		err = mplk_rebind_sock(sk, is_bind, netid);
		WIFIPRO_DEBUG("rebind for uid = %u\n", sk->sk_uid.val);
	}
	return err;
}

static int mplk_interrupt_data_transfer(struct sock *sk, unsigned int strategy)
{
	uid_t uid;
	struct uid_netid *uid_netid_entry = NULL;
	struct uid_netid uid_entry;

	if (sk == NULL)
		return 0;

	read_lock_bh(&uid_netid_tree_lock);

	uid = sk->sk_uid.val;
	uid_netid_entry = mplk_get_uid_netid_nl(uid);
	if (!uid_netid_entry) {
		read_unlock_bh(&uid_netid_tree_lock);
		return 0;
	}
	uid_entry = *uid_netid_entry;
	read_unlock_bh(&uid_netid_tree_lock);

	if (uid_entry.need_delete == MPLK_UID_NEED_DELETE) {
		mplk_trigger_socket_reroute(sk, MPLK_UNBIND_TO_CELL,
			uid_entry.netid);
	} else if (time_before(sk->sk_born_stamp, uid_entry.bind_stamp)) {
		if (uid_entry.strategy & strategy)
			mplk_trigger_socket_reroute(sk, MPLK_BIND_TO_CELL,
				uid_entry.netid);
	}

	return 0;
}

void mplk_try_nw_bind(struct sock *sk)
{
	struct uid_netid *uid_netid_entry = NULL;
	uid_t uid;

	if (sk == NULL)
		return;
	if ((sk->sk_protocol != IPPROTO_TCP) &&
		(sk->sk_protocol != IPPROTO_UDP) &&
		(sk->sk_protocol != IPPROTO_UDPLITE))
		return;

	uid = sk->sk_uid.val;
	if (uid < MPLK_NW_BIND_UID_MIN_LIMIT)
		return;

	read_lock_bh(&uid_netid_tree_lock);
	uid_netid_entry = mplk_get_uid_netid_nl(uid);
	if (uid_netid_entry && uid_netid_entry->netid > 0) {
		if (uid_netid_entry->need_delete == MPLK_UID_NEED_DELETE) {
			WIFIPRO_DEBUG("%u is to delete,use default mark = %d\n",
				 uid, sk->sk_mark);
			read_unlock_bh(&uid_netid_tree_lock);
			return;
		}
		if (sk->sk_mark == MPLK_SOMARK_UNSET) {
			sk->sk_mark = uid_netid_entry->netid;
			WIFIPRO_DEBUG("%u bound to %u", uid,
				uid_netid_entry->netid);
		} else {
			WIFIPRO_DEBUG("sk_mark has set to:%u", sk->sk_mark);
		}
	}
	read_unlock_bh(&uid_netid_tree_lock);
}
EXPORT_SYMBOL(mplk_try_nw_bind);

int mplk_sendmsg(struct sock *sk)
{
	return mplk_interrupt_data_transfer(sk, MPLK_SK_STRATEGY_FUTE);
}
EXPORT_SYMBOL(mplk_sendmsg);

int mplk_recvmsg(struct sock *sk)
{
	return mplk_interrupt_data_transfer(sk, MPLK_SK_STRATEGY_FURE);
}
EXPORT_SYMBOL(mplk_recvmsg);

int get_mplk_somark(struct sock *sk, int val)
{
	struct uid_netid *uid_netid_entry = NULL;
	uid_t uid;
	int mark;

	if (sk == NULL)
		return val;

	/* if bit16 or bit17 are set, do not change somark */
	if ((unsigned int)val & MPLK_SOMARK_SELECTED_OR_VPN_MASK) {
		WIFIPRO_DEBUG("mplk explicitlySelected or protectedFromVpn");
		return val;
	}

	uid = sk->sk_uid.val;
	mark = val;
	read_lock_bh(&uid_netid_tree_lock);
	uid_netid_entry = mplk_get_uid_netid_nl(uid);
	if (uid_netid_entry &&
		uid_netid_entry->need_delete != MPLK_UID_NEED_DELETE) {
		/* only replace val's lowest 16bits with new netid */
		mark = ((u32)mark & MPLK_FWMARK_CONTROL_MASK) |
			(uid_netid_entry->netid & MPLK_FWMARK_NETID_MASK);
		WIFIPRO_DEBUG("get new mark = %d\n", mark);
	} else {
		WIFIPRO_DEBUG("mark not change , mark still = %d\n", mark);
	}
	read_unlock_bh(&uid_netid_tree_lock);
	return mark;
}
EXPORT_SYMBOL(get_mplk_somark);

static dpi_rule_mask g_dpi_rule[DPI_RULE_MAX_SIZE];
static int g_dpi_rule_next_index = 0;

static dpi_rule_mask g_dpi_rules;
static int g_dpi_rules_num = 0;

/* Add dpi rules to the hash list */
void add_dpi_rule(const char *data, u32 len)
{
	dpi_rule_mask *mask = NULL;
	dpi_mark_rule_t *p_dmr = NULL;
	if (data == NULL || len < sizeof(dpi_mark_rule_t)) {
		printk("add_dpi_rule inputlength invalid\n");
		return;
	}
	write_lock_bh(&g_dpi_rule_lock);
	if (g_dpi_rule_next_index < 0 || g_dpi_rule_next_index >= DPI_RULE_MAX_SIZE) {
		write_unlock_bh(&g_dpi_rule_lock);
		return;
	}
	mask = &g_dpi_rule[g_dpi_rule_next_index];
	g_dpi_rule_next_index = (g_dpi_rule_next_index + 1) % DPI_RULE_MAX_SIZE;
	write_unlock_bh(&g_dpi_rule_lock);
	p_dmr = (dpi_mark_rule_t *)data;

	/*
	 * Only support transportion protocol dpi for wangzherongyao
	 * till 2017/06/24, TODO: add other dpi rules
	 */
	switch (p_dmr->dmr_rule.rule_type) {
	case DMR_MT_TP:
		mask->tmgp_tp = p_dmr->dmr_rule.rule_body.match_tp_val;
		mask->tmgp_uid = p_dmr->dmr_app_uid;
		mask->tmgp_mark = p_dmr->dmr_rule.mark_num;
		break;
	default:
		break;
	}
}

void add_dpi_rule_ex(const char *data, u32 len)
{
	pr_info("add_dpi_rule_ex enter\n");
	dpi_mark_rule_t *p_dmr = NULL;
	if (g_dpi_rules_num < 0) {
		printk("add_dpi_rule_ex failed g_dpi_rules_num\n");
		return;
	}

	if (data == NULL || len < sizeof(dpi_mark_rule_t))
		return;

	p_dmr = (dpi_mark_rule_t *)data;

	g_dpi_rules.tmgp_tp = p_dmr->dmr_rule.rule_body.match_tp_val;
	g_dpi_rules.tmgp_uid = p_dmr->dmr_app_uid;
	g_dpi_rules.tmgp_mark = p_dmr->dmr_rule.mark_num;
	g_dpi_rules_num = 1;
}

void delete_dpi_rule(const char *data, u32 len)
{
	pr_info("delete_dpi_rule enter\n");
	if (g_dpi_rules_num <= 0) {
		printk("delete_dpi_rule failed g_dpi_rules_num\n");
		return;
	}

	if (data == NULL) {
		printk("delete_dpi_rule failed p_dmr == NULL\n");
		return;
	}

	if (len < sizeof(dpi_mark_rule_t)) {
		printk("add_dpi_rule_ex failed len is abnormal\n");
		return;
	}

	g_dpi_rules.tmgp_tp = 0;
	g_dpi_rules.tmgp_mark = 0;
	g_dpi_rules.tmgp_uid = 0;
	g_dpi_rules_num = 0;
}

static rwlock_t g_tcp_recover_lock;
static uid_t g_tcp_recover_white_list[TCP_RECOVER_RULE_MAX_SIZE];
static int g_tcp_recover_next_index = 0;
int get_tcp_recover_somark(struct sock *sk)
{
	uid_t uid = sk->sk_uid.val;
	int is_tcp_recover_scene = 0;
	for (int this_uid_index = 0; this_uid_index < g_tcp_recover_next_index; ++this_uid_index) {
		if (g_tcp_recover_white_list[this_uid_index] == uid) {
			is_tcp_recover_scene = 1;
			break;
		}
	}
	return is_tcp_recover_scene;
}

void delete_tcp_recover_rule(uid_t uid)
{
	write_lock_bh(&g_tcp_recover_lock);
	int temp_next_index = 0;
	for (int this_uid_index = 0; this_uid_index < g_tcp_recover_next_index; ++this_uid_index) {
		if (g_tcp_recover_white_list[this_uid_index] != uid) {
			g_tcp_recover_white_list[temp_next_index] = g_tcp_recover_white_list[this_uid_index];
			temp_next_index++;
		}
	}
	g_tcp_recover_next_index = temp_next_index;
	write_unlock_bh(&g_tcp_recover_lock);
}

void add_tcp_recover_rule(uid_t uid, uint32_t is_tcp_recover_scene)
{
	if (g_tcp_recover_next_index < 0)
		return;
	for (int this_uid_index = 0; this_uid_index < g_tcp_recover_next_index; ++this_uid_index) {
		if (g_tcp_recover_white_list[this_uid_index] == uid) {
			if (!is_tcp_recover_scene)
				delete_tcp_recover_rule(uid);
			return;
		}
	}
	write_lock_bh(&g_tcp_recover_lock);
	if (is_tcp_recover_scene && g_tcp_recover_next_index < TCP_RECOVER_RULE_MAX_SIZE) {
		g_tcp_recover_white_list[g_tcp_recover_next_index] = uid;
		g_tcp_recover_next_index++;
		pr_info("add_tcp_recover_rule uid=%d is_tcp_recover_scene=%d \n", uid, is_tcp_recover_scene);
	}
	write_unlock_bh(&g_tcp_recover_lock);
}

static bool _proc_cmd_dmr(int cmd, dpi_mark_rule_t *p_dmr)
{
	if (p_dmr == NULL) {
		pr_info("dpi_hw_hook proc_cmd %d p_dmr is null\n", cmd);
		return false;
	}
	return true;
}

/* process the cmd, opt not used currently */
static void _proc_cmd(int cmd, int opt, const char *data, u32 len)
{
	dpi_mark_rule_t *p_dmr = NULL;

	if (len >= sizeof(dpi_mark_rule_t))
		p_dmr = (dpi_mark_rule_t *)data;

	switch (cmd) {
	case NETLINK_SET_RULE_TO_KERNEL:
		add_dpi_rule(data, len);
		break;
	case NETLINK_START_MARK:
		g_mark_tag = 1;
		break;
	case NETLINK_STOP_MARK:
		write_lock_bh(&g_dpi_rule_lock);
		g_dpi_rule_next_index = 0;
		write_unlock_bh(&g_dpi_rule_lock);
		g_mark_tag = 0;
		break;
	case NETLINK_SET_RULE_TO_KERNEL_EX:
		add_dpi_rule_ex(data, len);
		break;
	case NETLINK_DEL_RULE_TO_KERNEL:
		delete_dpi_rule(data, len);
		break;
	case NETLINK_MPLK_BIND_NETWORK:
		mplk_add_nw_bind_ex(data, len);
		break;
	case NETLINK_MPLK_UNBIND_NETWORK:
		if (!_proc_cmd_dmr(cmd, p_dmr))
			return;
		mplk_del_nw_bind(p_dmr->dmr_app_uid);
		break;
	case NETLINK_MPLK_RESET_SOCKET:
		pr_info("mplk not support reset command now\n");
		break;
	case NETLINK_MPLK_CLOSE_SOCKET:
		if (!_proc_cmd_dmr(cmd, p_dmr))
			return;
		mplk_close_socket_by_uid(p_dmr->dmr_mplk_strategy,
			p_dmr->dmr_app_uid);
		break;
	case NETLINK_SET_TCP_RECOVER_TO_KERNEL:
		if (!_proc_cmd_dmr(cmd, p_dmr))
			return;
		add_tcp_recover_rule(p_dmr->dmr_app_uid, p_dmr->dmr_mplk_strategy);
		break;
	default:
		pr_info("hwdpi:kernel_hw_receive cmd=%d is wrong\n", cmd);
	}
}

/* receive cmd for DPI netlink message */
static void kernel_ntl_receive(struct sk_buff *__skb)
{
	struct nlmsghdr *nlh = NULL;
	struct tag_hw_msg2knl *hmsg = NULL;
	struct sk_buff *skb = NULL;

	if (__skb == NULL)
		return;

	skb = skb_get(__skb);
	if (skb != NULL && (skb->len >= (unsigned int)NLMSG_HDRLEN)) {
		nlh = nlmsg_hdr(skb);
		if ((nlh->nlmsg_len >= sizeof(struct nlmsghdr)) &&
				(skb->len >= nlh->nlmsg_len)) {
			if (nlh->nlmsg_type == NETLINK_REG_TO_KERNEL) {
				g_uspa_pid = nlh->nlmsg_pid;
			} else if (nlh->nlmsg_type == NETLINK_UNREG_TO_KERNEL) {
				g_uspa_pid = 0;
			} else if (nlh->nlmsg_len < sizeof(struct nlmsghdr) + sizeof(int) + sizeof(char)) {
				pr_info("dpi_hw_hook kernel_ntl_receive nlh->nlmsg_len=%d is too short\n", nlh->nlmsg_len);
				kfree_skb(skb);
				return;
			} else {
				hmsg = (struct tag_hw_msg2knl *)nlh;
				_proc_cmd(nlh->nlmsg_type, hmsg->opt,
					(char *)&(hmsg->data[0]), nlh->nlmsg_len - sizeof(struct nlmsghdr) - sizeof(int));
			}
		}
	}
	kfree_skb(skb);
}

/* Initialize netlink socket, add hook function for netlink message receiving */
static void netlink_init(void)
{
	struct netlink_kernel_cfg hwcfg = {
		.input = kernel_ntl_receive,
	};
	g_hw_nlfd = netlink_kernel_create(&init_net, NETLINK_HW_DPI, &hwcfg);
	if (g_hw_nlfd == NULL)
		pr_info("netlink_init failed NETLINK_HW_DPI\n");
}

/* mark the skb if it matched the rules */
void mark_skb_by_rule_ex(struct sk_buff *skb)
{
	struct sock *sk = sk_to_full_sk(skb->sk);
	struct iphdr *iph = ip_hdr(skb);

	// invalidation no deal to avoid to lead to time.
	if (sk->sk_hwdpi_mark & MR_MULTIP_UNMARKED)
		return;

	if (sk->sk_hwdpi_mark & MR_TMGP_4) {
	    skb->mark = g_dpi_rules.tmgp_mark;
	    return;
	}

	if (iph != NULL && (g_dpi_rules.tmgp_tp & iph->protocol)) {
	    if (g_dpi_rules.tmgp_uid == sk->sk_uid.val) {
	    	skb->mark = g_dpi_rules.tmgp_mark;
	    	sk->sk_hwdpi_mark |= MR_TMGP_4;
	    	printk("mark_skb_by_rule_ex skb_uid=%d skb->mark = %d sk->sk_hwdpi_mark= %d",
	    		sk->sk_uid, skb->mark, sk->sk_hwdpi_mark);
	    	return;
	    }
	}

	sk->sk_hwdpi_mark |= MR_MULTIP_UNMARKED;
}

/* mark the skb if it matched the rules */
void mark_skb_by_rule_legacy(struct sk_buff *skb)
{
	struct sock *sk = sk_to_full_sk(skb->sk);
	int i;
	struct iphdr *iph = ip_hdr(skb);
	if (sk->sk_hwdpi_mark & MR_MARKED)
			return;

	for (i = 0; i < g_dpi_rule_next_index; i++) {
		dpi_rule_mask *mask = &g_dpi_rule[i];
		if (mask->tmgp_uid == 0)
			return;

		if (sk->sk_hwdpi_mark & MR_TMGP_2) {
			skb->mark = mask->tmgp_mark;
			return;
		}
		if (iph != NULL && iph->protocol == mask->tmgp_tp) {
			if (sk->sk_uid.val == mask->tmgp_uid) {
				sk->sk_hwdpi_mark = sk->sk_hwdpi_mark | MR_TMGP_2;
				skb->mark = mask->tmgp_mark;
				return;
			}
		}
	}

	/* This socket is not tmgp gaming udp */
	sk->sk_hwdpi_mark = sk->sk_hwdpi_mark | MR_MARKED;
}

/* mark the skb if it matched the rules */
void mark_skb_by_rule(struct sk_buff *skb, int tag)
{
	struct sock *sk = skb->sk;

	if (sk == NULL)
		return;

	if ((g_dpi_rule_next_index == 0)
		&& (g_dpi_rules_num == 0))
		return;

	if (g_dpi_rules_num == 0 && tag == 0) {
		sk->sk_hwdpi_mark = sk->sk_hwdpi_mark & MR_RESET;
		return;
	}

	skb->mark = 0;
	if (g_dpi_rules_num != 0)
		mark_skb_by_rule_ex(skb);

	if (tag != 0)
		mark_skb_by_rule_legacy(skb);
}

static unsigned int dpimark_hook_localout(void *ops, struct sk_buff *skb,
	const struct nf_hook_state *state)
{
	/* match the packet for optimization */
	if (skb != NULL && skb->sk != NULL && skb->sk->sk_state > 0)
		mark_skb_by_rule(skb, g_mark_tag);
	return NF_ACCEPT;
}

static struct nf_hook_ops net_hooks[] = {
	{
		.hook = dpimark_hook_localout,
#if (KERNEL_VERSION(4, 4, 0) > LINUX_VERSION_CODE)
		.owner = THIS_MODULE,
#endif
		.pf = PF_INET,
		.hooknum = NF_INET_POST_ROUTING,
		.priority = NF_IP_PRI_FILTER - 1,
	},
};

static int __init nf_init(void)
{
	int ret = 0;

	/*
	 * Add a hook point on NF_INET_LOCAL_OUT,
	 * where we can get all the packets generated by local APP
	 */
#if (KERNEL_VERSION(4, 14, 0) > LINUX_VERSION_CODE)
	ret = nf_register_hooks(net_hooks, ARRAY_SIZE(net_hooks));
#else
	ret = nf_register_net_hooks(&init_net, net_hooks,
		ARRAY_SIZE(net_hooks));
#endif
	if (ret) {
		pr_info("nf_init ret=%d  ", ret);
		return -1;
	}

	/* Initialize the netlink connection */
	netlink_init();

	rwlock_init(&uid_netid_tree_lock);
	rwlock_init(&g_dpi_rule_lock);

	pr_info("dpi_hw_hook_init success\n");

	return 0;
}

static void __exit nf_exit(void)
{
#if (KERNEL_VERSION(4, 14, 0) > LINUX_VERSION_CODE)
	nf_unregister_hooks(net_hooks, ARRAY_SIZE(net_hooks));
#else
	nf_unregister_net_hooks(&init_net, net_hooks, ARRAY_SIZE(net_hooks));
#endif
}

module_init(nf_init);
module_exit(nf_exit);

MODULE_LICENSE("Dual BSD");
MODULE_AUTHOR("s00399850");
MODULE_DESCRIPTION("HW DPI MARK NF_HOOK");
MODULE_VERSION("1.0.1");
MODULE_ALIAS("HW LWDPI 01");
