

#ifndef _CHR_NETLINK_H
#define _CHR_NETLINK_H

#include "wbc_hw_hook.h"
#include <linux/netlink.h>

#define CHR_SPEED_SLOW_EVENT 8
#define CHR_NETLINK_INIT 1
#define CHR_NETLINK_EXIT 0
#define DIFF_SRC_IP_ADDR_MAX 2
#define DIFF_DST_IP_ADDR_MAX 3
#define TIMER_3_MINUTES (180 * HZ)
#define TIMER_60_MINUTES (180 * 20 * HZ)

#define CHR_TIMER_3 1
#define CHR_TIMER_60 2
#define RNT_SIZE 2

/*
 * enum - netlink chr report message
 * @NETLINK_CHR_REG: send from apk to register the PID for netlink kernel socket
 * @NETLINK_CHR_KER_MSG: send from kernel to apk
 * @NETLINK_CHR_UNREG: when apk exit send this type message to unregister
 * @NETLINK_CHR_SET_APP_UID: send from apk to kernel, set top app's uid
 */
enum {
	NETLINK_CHR_REG = 0,
	NETLINK_CHR_KER_MSG,
	NETLINK_CHR_UNREG,
	NETLINK_CHR_SET_APP_UID,
};

struct chr_nl_packet_msg {
	int chr_event;
	unsigned int src_addr;
	struct http_return rtn_stat[RNT_SIZE];
};

struct chr_net_iface_struct {
	unsigned int dst_addr_idx;
	unsigned int dst_addr[DIFF_DST_IP_ADDR_MAX];
	unsigned int src_addr;
};

struct tag_chr_msg_to_knl {
	struct nlmsghdr hdr;
	int index;
	unsigned int uid;
};

void notify_chr_thread_to_send_msg(unsigned int dst_addr,
	unsigned int src_addr, struct sock *sk);
int chr_notify_event(int event, int pid, unsigned int src_addr,
	struct http_return *prtn);
unsigned int get_user_space_pid(void);
#endif /* _CHR_NETLINK_H */
