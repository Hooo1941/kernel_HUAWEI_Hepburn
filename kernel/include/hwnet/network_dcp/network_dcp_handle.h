

#ifndef _DCP_HANDLER_H
#define _DCP_HANDLER_H

#include <linux/ip.h>
#include <linux/ipv6.h>
#include <linux/list.h>
#include <linux/module.h>
#include <linux/net.h>
#include <linux/netfilter.h>
#include <linux/netlink.h>
#include <linux/types.h>
#include <net/tcp.h>
#include <net/sock.h>

#define LEN_OF_TOKEN 44
#define LEN_OF_AUTH_KEY_INFO 45
#define LEN_OF_SIGN_KEY_INFO 44
#define LEN_SHA256_HMAC 32
#define DCP_PROXY_CHECK (3 * HZ)
#define DCP_PUSH_CHECK (30 * HZ)
#define KEY_WORDS_LENGTH 4
#define FIRST_KEY_PKT 1
#define SECOND_KEY_PKT 2
#define MAX_DEST_ADDR_COUNT 30
#define MAX_SYN_FAIL_NUM 4
#define SYSTEM_UID 1000

enum dcp_cmd_type {
	NL_DCP_MSG_REG,
	NL_DCP_MSG_UNREG,
	NL_DCP_MSG_ENABLE_PXY,
	NL_DCP_MSG_DISABLE_PXY,
	NL_DCP_MSG_SET_APP_INFO,
	NL_DCP_MSG_SET_TOKEN,
	NL_DCP_MSG_PROCESS_DIED,
	NL_DCP_MSG_SET_IP_CONFIG,
	NL_DCP_DESTROY_SOCK,
	NL_DCP_MSG_REQ_BUTT
};

enum dcp_state {
	DCP_INIT_STATE,
	DCP_NORMAL_STATE,
	DCP_PROXY_STATE
};

enum dcp_notify_event {
	DCP_PROXY_SUCC,
	DCP_PROXY_FAIL,
	DCP_PROXY_BYPASS,
	DCP_CLOUD_ENABLE,
	DCP_CLOUD_DISABLE
};

enum dcp_push_socket_state {
	NOT_FOUND,
	FOUND_ESTABLISHED,
	FOUND_CLOSED
};

enum iaware_cmd {
	DESTROY_TCP = 1
};

// 目的地址结构体定义
typedef union sockaddr_un_t {
	struct sockaddr_in6 v6;
	struct sockaddr_in v4;
	struct {
		u16 sin_family;
		u16 sin_port;
	};
} sockaddr_un;

// 新的首包数据结构体定义
struct sig_head_info {
	u16 version;
	u16 cmd_type;
	u32 time_stamp_high;
	u32 time_stamp_low;
	u32 length;
};

// 新的首包数据结构体定义
struct session_info {
	char auth_token[LEN_OF_AUTH_KEY_INFO];
	char resv[3];
	char token[LEN_OF_TOKEN];
	u32 session_id;
};

// 新的首包数据结构体定义
struct sig_forward_req_msg {
	struct sig_head_info head;
	struct session_info info;
	sockaddr_un orig_dst_address;
	u8 sha256[LEN_SHA256_HMAC];
};

// 记录开始导流前的socket信息
struct push_stream_info {
	u16 src_port;
	u16 dst_port;
	u32 key_words1_num;
	u32 key_words2_num;
	bool is_tcp_established;
	struct list_head push_stream_info_list;
};

// 记录开始导流后的目的信息，以及源端口
struct proxy_socket_dst_info {
	u16 src_port; // 源端口
	u16 dcp_index;
	sockaddr_un dst_info; // 目的端口及地址信息
	struct list_head proxy_socket_dst_info_list;
};

// 记录每个进程所有的信息
struct proxy_app_info {
	u32 uid;
	u32 pids;
	u8 proxy_state; // 当前应用状态
	u16 push_port;
	u8 push_socket_state;
	u8 fail_num;
	u8 proxy_socket_num;
	bool is_proxy_enable; // 是否允许导流
	u16 dst_info_num;
	bool is_need_destroy_tcp;
	spinlock_t dst_info_list_lock;
	spinlock_t before_stream_lock;
	spinlock_t after_stream_lock;
	struct list_head app_info_list;
	struct list_head dst_info_list;
	struct list_head before_proxy_stream_list;
	struct list_head after_proxy_stream_list;
};

// 主动上报导流结果
struct dcp_unsol_proxy_result {
	u32 uid;
	int result;
};

// 接收netd下发进程的uid, pid，以及加密token信息
struct dcp_app_info {
	struct nlmsghdr hdr;
	u32 pids;
	u32 uid;
	char token[LEN_OF_TOKEN];
};

// 接收netd下发的代理服务器IP地址信息
struct dcp_sockaddr_info {
	struct nlmsghdr hdr;
	u32 uid;
	sockaddr_un addr;
};

// 函数头定义
void dcp_send_data_packet(struct sock *);
void dcp_tcp_state_change(struct sock *, int);
void dcp_tcp_v6_change_soackaddr(struct sock *, struct sockaddr_in6 *);
void dcp_tcp_v4_change_soackaddr(struct sock *, struct sockaddr_in *);
void process_auth_token_update(char *, int);
void process_sign_token_update(char *, int);
void process_iaware_cmd(int, int, int);
void dcp_close_proxy(bool);

#endif
