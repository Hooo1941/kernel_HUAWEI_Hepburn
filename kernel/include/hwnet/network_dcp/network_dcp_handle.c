

#include "network_dcp_handle.h"
#include <crypto/hash.h>
#include <linux/errno.h>
#include <linux/init.h>
#include <linux/kthread.h>
#include <linux/module.h>
#include <linux/mutex.h>
#include <linux/rtc.h>
#include <linux/sched.h>
#include <linux/semaphore.h>
#include <linux/skbuff.h>
#include <linux/time.h>
#include <linux/types.h>
#include <net/netlink.h>
#include <net/sock.h>
#include <net/tcp.h>
#include <securec.h>

#include <uapi/linux/netlink.h>
#include <uapi/linux/netfilter.h>
#include <uapi/linux/netfilter_ipv4.h>
#ifdef CONFIG_HW_BOOSTER
#include "hwnet/booster/app_proxy.h"
#endif
#include <huawei_platform/log/hw_log.h>

#undef HWLOG_TAG
#define HWLOG_TAG network_dcp
HWLOG_REGIST();
MODULE_LICENSE("GPL");
DEFINE_MUTEX(dcp_receive_sem);

static u16 g_index = 0;
static u32 g_session_id = 10000;
static int g_user_space_pid;
static bool g_enable_proxy = true;
static char g_token_info[LEN_OF_TOKEN];
static char g_auth_key_info[LEN_OF_AUTH_KEY_INFO];
static char g_sign_key_info[LEN_OF_SIGN_KEY_INFO];
static bool g_token_info_valid = false;
static bool g_auth_key_valid = false;
static bool g_sign_key_valid = false;
static spinlock_t g_dcp_send_lock;
static spinlock_t g_proxy_app_list_lock;

static struct sock *g_dcp_nlfd;
static struct semaphore g_dcp_netlink_sema;
static struct semaphore g_sock_destroy_sema;
static struct timer_list g_dcp_push_check_timer;
static struct timer_list g_dcp_proxy_check_timer;
static struct list_head g_proxy_app_info;
static struct sockaddr_in6 g_proxy_dst_v6;
static struct sockaddr_in g_proxy_dst_v4;
const char g_push_keywords1[] = {0x16, 0xf1, 0x04, 0x00};
const char g_push_keywords2[] = {0x17, 0xf1, 0x04, 0x01};

u32 get_transport_hdr_len(const struct sk_buff *skb, u8 proto)
{
	if (proto == IPPROTO_UDP)
		return sizeof(struct udphdr);

	return tcp_hdrlen(skb);
}

bool is_zero_lineare_room(const struct sk_buff *skb, u8 proto)
{
	if (skb_is_nonlinear(skb) &&
		(skb_headlen(skb) == skb_transport_offset(skb)
			+ get_transport_hdr_len(skb, proto)))
		return true;

	return false;
}

u32 dcp_get_parse_len(const struct sk_buff *skb, u8 proto)
{
	u32 parse_len = 0;
	u32 hdr_len = get_transport_hdr_len(skb, proto);
	if (skb->len > skb_transport_offset(skb) + hdr_len)
		parse_len = skb->len - skb_transport_offset(skb) - hdr_len;

	return parse_len;
}

u8 *dcp_get_payload_addr(const struct sk_buff *skb,
	u8 proto, u8 **vaddr, u32 *parse_len)
{
	const skb_frag_t *frag = NULL;
	struct page *page = NULL;
	u8 *payload = NULL;
	if (skb == NULL || vaddr == NULL || parse_len == NULL)
		return payload;

	if (is_zero_lineare_room(skb, proto)) {
		frag = &skb_shinfo(skb)->frags[0];
		page = skb_frag_page(frag);
		*vaddr = kmap_atomic(page);
#if LINUX_VERSION_CODE < KERNEL_VERSION(5, 4, 0)
		payload = *vaddr + frag->page_offset;
#else
		payload = *vaddr + skb_frag_off(frag);
#endif
		*parse_len = skb_frag_size(frag);
	} else {
		payload = skb_transport_header(skb) + get_transport_hdr_len(skb, proto);
		*parse_len = dcp_get_parse_len(skb, proto);
	}
	return payload;
}

void dcp_unmap_vaddr(u8 *vaddr)
{
	if (vaddr)
		kunmap_atomic(vaddr);
}

bool is_full_sock(struct sock *sk)
{
	if (sk != NULL && sk_fullsock(sk))
		return true;

	return false;
}

bool is_ipv6_packet(struct sock *sk)
{
	return ((sk->sk_family == AF_INET6 &&
		!((u32)ipv6_addr_type(&sk->sk_v6_rcv_saddr) &
		IPV6_ADDR_MAPPED)) ? true : false);
}

bool is_valid_content(void)
{
	return g_sign_key_valid &&
		g_auth_key_valid &&
		g_token_info_valid;
}

unsigned long long get_time_stamp(void)
{
	struct timespec64 time = {0};
	unsigned long long time_stamp;
	ktime_get_real_ts64(&time);
	time_stamp = (unsigned long long)time.tv_sec * 1000
		+ (unsigned long long)time.tv_nsec / 1000000;
	return time_stamp;
}

int hmac_sha256(u8 *key, u8 ksize, char *plaintext, u8 psize, u8 *output)
{
	int ret;
	struct crypto_shash *tfm;
	struct shash_desc *shash;
	if (!ksize)
		return -EINVAL;

	tfm = crypto_alloc_shash("hmac(sha256)", 0, 0);
	if (IS_ERR(tfm)) {
		hwlog_info("crypto_alloc_ahash failed.\n");
		return -1;
	}

	ret = crypto_shash_setkey(tfm, key, ksize);
	if (ret) {
		hwlog_info("crypto_ahash_setkey failed.\n");
		goto failed;
	}

	shash = kzalloc(sizeof(*shash) + crypto_shash_descsize(tfm),
			GFP_KERNEL);
	if (!shash) {
		ret = -ENOMEM;
		goto failed;
	}

	shash->tfm = tfm;
	ret = crypto_shash_digest(shash, plaintext, psize, output);
	kfree(shash);

failed:
	crypto_free_shash(tfm);
	return ret;
}

uid_t dcp_get_uid_from_sock(struct sock *sk)
{
	kuid_t kuid;

	if (!sk || !sk_fullsock(sk))
		return overflowuid;
	kuid = sock_net_uid(sock_net(sk), sk);
	return from_kuid_munged(sock_net(sk)->user_ns, kuid);
}

struct proxy_app_info *get_proxy_app_info_by_uid_pid(u32 uid, u32 pid)
{
	struct proxy_app_info *appinfo = NULL;
	struct proxy_app_info *temp = NULL;
	if (!spin_trylock_bh(&g_proxy_app_list_lock))
		return NULL;

	list_for_each_entry_safe(appinfo, temp, &g_proxy_app_info, app_info_list) {
		if (appinfo->uid == uid && appinfo->pids == pid) {
			spin_unlock_bh(&g_proxy_app_list_lock);
			return appinfo;
		}
	}
	spin_unlock_bh(&g_proxy_app_list_lock);
	return NULL;
}

struct proxy_app_info *get_proxy_app_info_by_uid(u32 uid)
{
	struct proxy_app_info *appinfo = NULL;
	struct proxy_app_info *temp = NULL;
	if (!spin_trylock_bh(&g_proxy_app_list_lock))
		return NULL;

	if (list_empty(&g_proxy_app_info)) {
		spin_unlock_bh(&g_proxy_app_list_lock);
		return NULL;
	}

	list_for_each_entry_safe(appinfo, temp, &g_proxy_app_info, app_info_list) {
		if (appinfo->uid == uid) {
			spin_unlock_bh(&g_proxy_app_list_lock);
			return appinfo;
		}
	}

	spin_unlock_bh(&g_proxy_app_list_lock);
	return NULL;
}

struct proxy_app_info *get_proxy_app_info_by_sock(struct sock *sk)
{
	u32 uid;
	u32 pid;
	struct proxy_app_info *appinfo = NULL;
	if (list_empty(&g_proxy_app_info))
		return NULL;

	uid = dcp_get_uid_from_sock(sk);
	if (uid == 0 || (int)uid == overflowuid)
		return NULL;

#ifdef CONFIG_HUAWEI_KSTATE
	pid = (u32)sk->sk_pid;
	appinfo = get_proxy_app_info_by_uid_pid(uid, pid);
#else
	appinfo = get_proxy_app_info_by_uid(uid);
#endif

	return appinfo;
}

u32 is_push_skb(struct sk_buff *skb, struct tcphdr *tcph)
{
	char *tcp_payload = NULL;
	u8 *vaddr = NULL;
	u32 dlen;
	if (tcph->ack != 1 || tcph->psh != 1)
		return 0;

	tcp_payload = (char *)dcp_get_payload_addr(skb, IPPROTO_TCP, &vaddr, &dlen);
	if (dlen == 0 || tcp_payload == NULL) {
		dcp_unmap_vaddr(vaddr);
		return 0;
	}

	if (dlen > KEY_WORDS_LENGTH &&
		strncmp(tcp_payload, g_push_keywords1, KEY_WORDS_LENGTH - 1) == 0) {
		hwlog_info("%s, s_port: %u, d_port: %u.\n", __func__,
			htons(tcph->source), htons(tcph->dest));
		dcp_unmap_vaddr(vaddr);
		return FIRST_KEY_PKT;
	}

	if (dlen > KEY_WORDS_LENGTH &&
		strncmp(tcp_payload, g_push_keywords2, KEY_WORDS_LENGTH - 1) == 0) {
		hwlog_info("%s, s_port: %u, d_port: %u.\n", __func__,
			htons(tcph->source), htons(tcph->dest));
		dcp_unmap_vaddr(vaddr);
		return SECOND_KEY_PKT;
	}

	dcp_unmap_vaddr(vaddr);
	return 0;
}

struct proxy_socket_dst_info *find_target_sockaddr_info(struct proxy_app_info *appinfo,
	u16 dcp_index)
{
	bool is_found = false;
	struct list_head *list = NULL;
	struct proxy_socket_dst_info *dst_info = NULL;
	struct proxy_socket_dst_info *temp = NULL;
	if (!spin_trylock_bh(&appinfo->dst_info_list_lock))
		return NULL;

	list = &appinfo->dst_info_list;
	if (list_empty(list)) {
		spin_unlock_bh(&appinfo->dst_info_list_lock);
		return NULL;
	}

	list_for_each_entry_safe(dst_info, temp, list, proxy_socket_dst_info_list) {
		if (dst_info->dcp_index == dcp_index) {
			is_found = true;
			break;
		}
	}
	if (!is_found)
		dst_info = NULL;
	spin_unlock_bh(&appinfo->dst_info_list_lock);
	return dst_info;
}

struct sig_forward_req_msg *construct_data_packet(struct proxy_app_info *appinfo,
	u16 src_port, bool is_ipv6_pkt, u16 dcp_index)
{
	errno_t err;
	struct proxy_socket_dst_info *dst_info = NULL;
	struct sig_forward_req_msg *msg = NULL;
	unsigned long long time_stamp;
	dst_info = find_target_sockaddr_info(appinfo, dcp_index);
	if (dst_info == NULL) {
		hwlog_err("%s dst_info NULL.\n", __func__);
		return NULL;
	}
	if (!is_valid_content())
		return NULL;
	msg = kmalloc(sizeof(struct sig_forward_req_msg), GFP_KERNEL);
	if (msg == NULL) {
		hwlog_err("%s kmalloc error!\n", __func__);
		return NULL;
	}
	err = memcpy_s(msg->info.auth_token, LEN_OF_AUTH_KEY_INFO,
		g_auth_key_info, LEN_OF_AUTH_KEY_INFO);
	if (err != EOK) {
		kfree(msg);
		return NULL;
	}
	err = memcpy_s(msg->info.token, LEN_OF_TOKEN, g_token_info, LEN_OF_TOKEN);
	if (err != EOK) {
		kfree(msg);
		return NULL;
	}
	msg->head.version = htons(1);
	msg->head.cmd_type = htons(1);
	time_stamp = get_time_stamp();
	msg->head.time_stamp_high = htonl((u32)(time_stamp >> 32));
	msg->head.time_stamp_low = htonl((u32)(time_stamp));
	msg->head.length = htonl(sizeof(struct sig_forward_req_msg) - sizeof(struct sig_head_info));
	msg->info.session_id = htonl(g_session_id++);
	if (is_ipv6_pkt) {
		msg->orig_dst_address.sin_family = htons(AF_INET6);
		msg->orig_dst_address.v6.sin6_addr = dst_info->dst_info.v6.sin6_addr;
	} else {
		msg->orig_dst_address.sin_family = htons(AF_INET);
		msg->orig_dst_address.v4.sin_addr.s_addr = dst_info->dst_info.v4.sin_addr.s_addr;
	}
	msg->orig_dst_address.sin_port = htons(dst_info->dst_info.sin_port);
	hmac_sha256(g_sign_key_info, LEN_OF_SIGN_KEY_INFO, (char *)msg,
		sizeof(struct sig_forward_req_msg) - LEN_SHA256_HMAC, msg->sha256);
	hwlog_info("%s g_session_id = %u, port = %u, uid = %u.\n",
		__func__, g_session_id - 1, src_port, appinfo->uid);
	return msg;
}

void send_data_packet(struct sock *sk, struct proxy_app_info *appinfo, u16 src_port, bool is_ipv6_pkt)
{
	struct kvec vec;
	struct msghdr msg1;
	struct sig_forward_req_msg *msg = NULL;
	msg = construct_data_packet(appinfo, src_port, is_ipv6_pkt, sk->v6_index);
	if (msg == NULL) {
		hwlog_err("%s: fail, msg is NULL, dcp_index = %u, port = %u.\n",
			__func__, sk->v6_index, src_port);
		return;
	}

	vec.iov_base = msg;
	vec.iov_len = sizeof(struct sig_forward_req_msg);
	(void)memset_s(&msg1, sizeof(msg1), 0, sizeof(msg1));
#if (LINUX_VERSION_CODE < KERNEL_VERSION(5, 4, 0))
	iov_iter_kvec(&msg1.msg_iter, WRITE | ITER_KVEC,
		&vec, 1, sizeof(struct sig_forward_req_msg));
#else
	iov_iter_kvec(&msg1.msg_iter, WRITE,
		&vec, 1, sizeof(struct sig_forward_req_msg));
#endif
	(void)tcp_sendmsg_locked(sk, &msg1, sizeof(struct sig_forward_req_msg));
	kfree(msg);
	appinfo->proxy_socket_num++;
	hwlog_info("%s: success, uid = %u, src_port = %u.\n", __func__, appinfo->uid, src_port);
}

void notify_proxy_result(u32 uid, int result)
{
	int ret;
	struct sk_buff *skb = NULL;
	struct nlmsghdr *nlh = NULL;
	struct dcp_unsol_proxy_result *packet = NULL;

	spin_lock_bh(&g_dcp_send_lock);
	if (!g_user_space_pid || !g_dcp_nlfd) {
		hwlog_err("%s: cannot unsol port msg.\n", __func__);
		ret = -1;
		goto end;
	}

	skb = nlmsg_new(sizeof(struct dcp_unsol_proxy_result), GFP_ATOMIC);
	if (skb == NULL) {
		hwlog_err("%s: alloc skb fail.\n", __func__);
		ret = -1;
		goto end;
	}

	nlh = nlmsg_put(skb, 0, 0, 1, sizeof(struct dcp_unsol_proxy_result), 0);
	if (nlh == NULL) {
		kfree_skb(skb);
		skb = NULL;
		ret = -1;
		goto end;
	}

	nlh->nlmsg_type = result;
	packet = nlmsg_data(nlh);
	(void)memset_s(packet, sizeof(struct dcp_unsol_proxy_result),
		0, sizeof(struct dcp_unsol_proxy_result));
	packet->uid = uid;
	packet->result = result;
	ret = netlink_unicast(g_dcp_nlfd, skb, g_user_space_pid, MSG_DONTWAIT);
	goto end;

end:
	spin_unlock_bh(&g_dcp_send_lock);
}

void del_dst_addr_info(struct proxy_app_info *appinfo, u16 src_port, u16 dcp_index)
{
	struct proxy_socket_dst_info *curr = NULL;
	struct proxy_socket_dst_info *temp = NULL;
	struct list_head *list = NULL;
	list = &appinfo->dst_info_list;
	if (list == NULL || list_empty(list) || appinfo->dst_info_num == 0)
		return;

	if (!spin_trylock_bh(&appinfo->dst_info_list_lock))
		return;

	list_for_each_entry_safe(curr, temp, list, proxy_socket_dst_info_list) {
		if (curr->src_port != src_port || curr->dcp_index != dcp_index)
			continue;
		list_del_init(&curr->proxy_socket_dst_info_list);
		kfree(curr);
		appinfo->dst_info_num--;
		break;
	}
	spin_unlock_bh(&appinfo->dst_info_list_lock);
}

void del_push_stream_info(struct proxy_app_info *appinfo, u16 src_port)
{
	struct push_stream_info *stream = NULL;
	struct push_stream_info *temp = NULL;
	struct list_head *list = NULL;
	spinlock_t *lock = NULL;
	if (appinfo == NULL)
		return;

	if (appinfo->proxy_state == DCP_NORMAL_STATE) {
		list = &appinfo->before_proxy_stream_list;
		lock = &appinfo->before_stream_lock;
	} else {
		list = &appinfo->after_proxy_stream_list;
		lock = &appinfo->after_stream_lock;
	}
	if (list_empty(list))
		return;

	spin_lock_bh(lock);
	list_for_each_entry_safe(stream, temp, list, push_stream_info_list) {
		if (stream->src_port == src_port) {
			list_del_init(&stream->push_stream_info_list);
			kfree(stream);
			break;
		}
	}
	spin_unlock_bh(lock);
}

void del_all_push_stream_info(struct list_head *list, spinlock_t *lock)
{
	struct push_stream_info *curr = NULL;
	struct push_stream_info *temp = NULL;
	if (list == NULL || list_empty(list))
		return;

	if (!spin_trylock_bh(lock))
		return;

	list_for_each_entry_safe(curr, temp, list, push_stream_info_list) {
		list_del_init(&curr->push_stream_info_list);
		kfree(curr);
	}
	spin_unlock_bh(lock);
}

void del_all_dst_addr_info(struct proxy_app_info *appinfo, struct list_head *list)
{
	struct proxy_socket_dst_info *curr = NULL;
	struct proxy_socket_dst_info *temp = NULL;
	if (list == NULL || list_empty(list) || appinfo->dst_info_num == 0)
		return;

	if (!spin_trylock_bh(&appinfo->dst_info_list_lock))
		return;

	g_index = 0;
	list_for_each_entry_safe(curr, temp, list, proxy_socket_dst_info_list) {
		list_del_init(&curr->proxy_socket_dst_info_list);
		kfree(curr);
	}
	appinfo->dst_info_num = 0;
	spin_unlock_bh(&appinfo->dst_info_list_lock);
}

bool record_ipv4_proxy_sock_addrinfo(struct proxy_app_info *appinfo,
	u16 dcp_index, struct sockaddr_in *usin)
{
	bool is_found = false;
	struct list_head *addr_list = NULL;
	struct proxy_socket_dst_info *head = NULL;
	struct proxy_socket_dst_info *temp = NULL;
	if (appinfo->dst_info_num >= MAX_DEST_ADDR_COUNT)
		return false;

	if (!spin_trylock_bh(&appinfo->dst_info_list_lock))
		return false;

	addr_list = &appinfo->dst_info_list;
	list_for_each_entry_safe(head, temp, addr_list, proxy_socket_dst_info_list) {
		if (head->dcp_index == dcp_index) {
			spin_unlock_bh(&appinfo->dst_info_list_lock);
			is_found = true;
			return false;
		}
	}

	if (!is_found) {
		head = kmalloc(sizeof(struct proxy_socket_dst_info), GFP_ATOMIC);
		if (head == NULL) {
			spin_unlock_bh(&appinfo->dst_info_list_lock);
			return false;
		}
		INIT_LIST_HEAD(&head->proxy_socket_dst_info_list);
		head->dcp_index = dcp_index;
		head->dst_info.sin_family = AF_INET;
		head->dst_info.sin_port = ntohs(usin->sin_port);
		head->dst_info.v4.sin_addr = usin->sin_addr;
		list_add_rcu(&head->proxy_socket_dst_info_list, addr_list);
		appinfo->dst_info_num++;
		hwlog_info("%s: dcp_index = %u, record address.\n", __func__, dcp_index);
	}
	spin_unlock_bh(&appinfo->dst_info_list_lock);
	return true;
}

bool record_ipv6_proxy_sock_addrinfo(struct proxy_app_info *appinfo,
	u16 dcp_index, struct sockaddr_in6 *usin)
{
	bool is_found = false;
	struct list_head *addr_list = NULL;
	struct proxy_socket_dst_info *head = NULL;
	struct proxy_socket_dst_info *temp = NULL;
	if (appinfo->dst_info_num >= MAX_DEST_ADDR_COUNT)
		return false;

	if (!spin_trylock_bh(&appinfo->dst_info_list_lock))
		return false;

	addr_list = &appinfo->dst_info_list;
	list_for_each_entry_safe(head, temp, addr_list, proxy_socket_dst_info_list) {
		if (head->dcp_index == dcp_index) {
			spin_unlock_bh(&appinfo->dst_info_list_lock);
			is_found = true;
			return false;
		}
	}

	if (!is_found) {
		head = kmalloc(sizeof(struct proxy_socket_dst_info), GFP_ATOMIC);
		if (head == NULL) {
			spin_unlock_bh(&appinfo->dst_info_list_lock);
			return false;
		}
		INIT_LIST_HEAD(&head->proxy_socket_dst_info_list);
		head->dcp_index = dcp_index;
		head->dst_info.sin_family = AF_INET6;
		head->dst_info.sin_port = ntohs(usin->sin6_port);
		head->dst_info.v6.sin6_addr = usin->sin6_addr;
		list_add_rcu(&head->proxy_socket_dst_info_list, addr_list);
		appinfo->dst_info_num++;
		hwlog_info("%s: dcp_index = %u, record address.\n", __func__, dcp_index);
	}
	spin_unlock_bh(&appinfo->dst_info_list_lock);
	return true;
}

void record_proxy_stream_info(struct proxy_app_info *appinfo,
	struct tcphdr *tcph, u32 key)
{
	u16 src_port;
	u16 dst_port;
	bool is_found = false;
	struct push_stream_info *head = NULL;
	struct push_stream_info *temp = NULL;
	struct list_head *list = NULL;
	src_port = htons(tcph->dest);
	dst_port = htons(tcph->source);
	if (!spin_trylock_bh(&appinfo->after_stream_lock))
		return;
	list = &appinfo->after_proxy_stream_list;
	list_for_each_entry_safe(head, temp, list, push_stream_info_list) {
		if (head->src_port == src_port && head->dst_port == dst_port) {
			if (key == 1) head->key_words1_num++;
			if (key == 2) head->key_words2_num++;
			spin_unlock_bh(&appinfo->after_stream_lock);
			return;
		}
	}

	if (!is_found) {
		head = kmalloc(sizeof(struct push_stream_info), GFP_ATOMIC);
		(void)memset_s(head, sizeof(struct push_stream_info), 0, sizeof(struct push_stream_info));
		INIT_LIST_HEAD(&head->push_stream_info_list);
		head->src_port = src_port;
		head->dst_port = dst_port;
		head->is_tcp_established = true;
		if (key == 1) head->key_words1_num++;
		if (key == 2) head->key_words2_num++;
		list_add_rcu(&head->push_stream_info_list, list);
	}
	spin_unlock_bh(&appinfo->after_stream_lock);
}

void record_normal_stream_info(struct proxy_app_info *appinfo,
	struct tcphdr *tcph, u32 key)
{
	u16 src_port;
	u16 dst_port;
	bool is_found = false;
	struct list_head *list = NULL;
	struct push_stream_info *head = NULL;
	struct push_stream_info *temp = NULL;
	src_port = htons(tcph->dest);
	dst_port = htons(tcph->source);
	if (!spin_trylock_bh(&appinfo->before_stream_lock))
		return;
	list = &appinfo->before_proxy_stream_list;
	list_for_each_entry_safe(head, temp, list, push_stream_info_list) {
		if (head->src_port == src_port && head->dst_port == dst_port) {
			if (key == 1) head->key_words1_num++;
			if (key == 2) head->key_words2_num++;
			spin_unlock_bh(&appinfo->before_stream_lock);
			is_found = true;
			return;
		}
	}

	if (!is_found) {
		head = kmalloc(sizeof(struct push_stream_info), GFP_ATOMIC);
		(void)memset_s(head, sizeof(struct push_stream_info), 0, sizeof(struct push_stream_info));
		INIT_LIST_HEAD(&head->push_stream_info_list);
		head->src_port = src_port;
		head->dst_port = dst_port;
		head->is_tcp_established = true;
		if (key == 1) head->key_words1_num++;
		if (key == 2) head->key_words2_num++;
		list_add_rcu(&head->push_stream_info_list, list);
	}
	spin_unlock_bh(&appinfo->before_stream_lock);
}

void process_hook_in(struct sock *sk, struct sk_buff *skb, struct tcphdr *tcph)
{
	u8 state;
	u32 key;
	struct proxy_app_info *appinfo = NULL;
	appinfo = get_proxy_app_info_by_sock(sk);
	if (appinfo == NULL)
		return;
	key = is_push_skb(skb, tcph);
	state = appinfo->proxy_state;
	if (state == DCP_PROXY_STATE) {
		if (key > 0 && appinfo->push_socket_state != FOUND_ESTABLISHED)
			record_proxy_stream_info(appinfo, tcph, key);
	} else {
		if (key > 0 && appinfo->push_socket_state != FOUND_ESTABLISHED)
			record_normal_stream_info(appinfo, tcph, key);
	}
}

unsigned int hook_v4_dcp(void *ops,
	struct sk_buff *skb, const struct nf_hook_state *state)
{
	struct sock *sk = NULL;
	struct iphdr *iph = NULL;
	struct tcphdr *tcph = NULL;
	if (list_empty(&g_proxy_app_info) || !g_enable_proxy)
		return NF_ACCEPT;

	if (state == NULL || skb == NULL)
		return NF_ACCEPT;

	sk = skb_to_full_sk(skb);
	if (sk == NULL)
		sk = state->sk;
	if (sk == NULL)
		return NF_ACCEPT;

	iph = ip_hdr(skb);
	if (iph == NULL || iph->protocol != IPPROTO_TCP)
		return NF_ACCEPT;

	tcph = tcp_hdr(skb);
	if (tcph == NULL)
		return NF_ACCEPT;

	process_hook_in(sk, skb, tcph);
	return NF_ACCEPT;
}

unsigned int hook_v6_dcp(void *ops,
	struct sk_buff *skb, const struct nf_hook_state *state)
{
	struct sock *sk = NULL;
	struct tcphdr *tcph = NULL;
	if (list_empty(&g_proxy_app_info) || !g_enable_proxy)
		return NF_ACCEPT;

	if (state == NULL || skb == NULL)
		return NF_ACCEPT;

	sk = skb_to_full_sk(skb);
	if (sk == NULL)
		sk = state->sk;
	if (sk == NULL)
		return NF_ACCEPT;

	if (!is_full_sock(sk))
		return NF_ACCEPT;

	if (sk->sk_type != SOCK_STREAM)
		return NF_ACCEPT;

	tcph = tcp_hdr(skb);
	if (tcph == NULL)
		return NF_ACCEPT;

	process_hook_in(sk, skb, tcph);
	return NF_ACCEPT;
}

void proxy_check_result(struct proxy_app_info *appinfo)
{
	u32 total = 0;
	struct list_head *list = NULL;
	struct push_stream_info *stream_info = NULL;
	struct push_stream_info *temp = NULL;
	if (appinfo == NULL)
		return;
	if (appinfo->push_socket_state == FOUND_ESTABLISHED)
		return;
	if (appinfo->proxy_state != DCP_PROXY_STATE)
		return;
	if (appinfo->dst_info_num == MAX_DEST_ADDR_COUNT)
		del_all_dst_addr_info(appinfo, &appinfo->dst_info_list);
	list = &appinfo->after_proxy_stream_list;
	list_for_each_entry_safe(stream_info, temp, list, push_stream_info_list) {
		total += stream_info->key_words1_num + stream_info->key_words2_num;
		if ((stream_info->key_words1_num > 3 || stream_info->key_words2_num > 3) &&
			stream_info->is_tcp_established) {
			hwlog_info("%s, found %u push socket %u.\n",
				__func__, appinfo->uid, stream_info->src_port);
			appinfo->fail_num = 0;
			appinfo->push_port = stream_info->src_port;
			appinfo->push_socket_state = FOUND_ESTABLISHED;
			if (appinfo->is_proxy_enable && (
				stream_info->dst_port == htons(g_proxy_dst_v4.sin_port) ||
				stream_info->dst_port == htons(g_proxy_dst_v6.sin6_port))) {
#ifdef CONFIG_HW_BOOSTER
				send_porxy_result(appinfo->uid, DCP_PROXY_SUCC);
#endif
				notify_proxy_result(appinfo->uid, DCP_PROXY_SUCC);
			}
			break;
		}
	}
	if (total == 0 && appinfo->proxy_socket_num > 3 && appinfo->is_proxy_enable) {
		hwlog_err("%s, no push data found in %u.\n", __func__, appinfo->uid);
		appinfo->is_proxy_enable = false;
		notify_proxy_result(appinfo->uid, DCP_PROXY_BYPASS);
	}
}

void dcp_proxy_check_timer(struct timer_list *data)
{
	bool is_need_continue_check = false;
	struct proxy_app_info *appinfo = NULL;
	struct proxy_app_info *temp = NULL;

	spin_lock_bh(&g_proxy_app_list_lock);
	if (list_empty(&g_proxy_app_info)) {
		spin_unlock_bh(&g_proxy_app_list_lock);
		return;
	}

	list_for_each_entry_safe(appinfo, temp, &g_proxy_app_info, app_info_list) {
		proxy_check_result(appinfo);
		if (appinfo->push_socket_state == FOUND_ESTABLISHED)
			continue;
		if (appinfo->proxy_state != DCP_PROXY_STATE)
			continue;
		is_need_continue_check = true;
	}
	spin_unlock_bh(&g_proxy_app_list_lock);

	if (is_need_continue_check) {
		g_dcp_proxy_check_timer.expires = jiffies + DCP_PROXY_CHECK;
		add_timer(&g_dcp_proxy_check_timer);
	}
}

void dcp_push_check_timer(struct timer_list *data)
{
	bool is_need_continue_check = false;
	struct list_head *list = NULL;
	struct proxy_app_info *appinfo = NULL;
	struct proxy_app_info *temp1 = NULL;
	struct push_stream_info *stream_info = NULL;
	struct push_stream_info *temp2 = NULL;
	spin_lock_bh(&g_proxy_app_list_lock);
	if (list_empty(&g_proxy_app_info)) {
		spin_unlock_bh(&g_proxy_app_list_lock);
		return;
	}

	list_for_each_entry_safe(appinfo, temp1, &g_proxy_app_info, app_info_list) {
		if (appinfo->push_socket_state == FOUND_ESTABLISHED)
			continue;
		if (appinfo->proxy_state != DCP_NORMAL_STATE)
			continue;
		list = &appinfo->before_proxy_stream_list;
		list_for_each_entry_safe(stream_info, temp2, list, push_stream_info_list) {
			if ((stream_info->key_words1_num > 3 || stream_info->key_words2_num > 3) &&
				stream_info->is_tcp_established) {
				hwlog_info("%s, found %u push socket %u.\n",
					__func__, appinfo->uid, stream_info->src_port);
				appinfo->push_socket_state = FOUND_ESTABLISHED;
				appinfo->push_port = stream_info->src_port;
				del_all_push_stream_info(list, &appinfo->before_stream_lock);
				break;
			}
		}
	}

	list_for_each_entry_safe(appinfo, temp1, &g_proxy_app_info, app_info_list) {
		if (appinfo->push_socket_state == FOUND_ESTABLISHED)
			continue;
		if (appinfo->proxy_state != DCP_NORMAL_STATE)
			continue;
		is_need_continue_check = true;
	}
	spin_unlock_bh(&g_proxy_app_list_lock);

	if (is_need_continue_check) {
		g_dcp_push_check_timer.expires = jiffies + DCP_PUSH_CHECK;
		add_timer(&g_dcp_push_check_timer);
	}
}

bool destroy_socket(struct sock *sk, struct proxy_app_info *appinfo)
{
	u16 src_port;
	struct inet_sock *inet = NULL;
	inet = inet_sk(sk);
	if (inet == NULL)
		return false;

	src_port = htons(inet->inet_sport);
	if (src_port != appinfo->push_port)
		return false;

	hwlog_info("%s, uid: %u.\n", __func__, appinfo->uid);
	if (sk->sk_state == TCP_ESTABLISHED || sk->sk_state == TCP_SYN_SENT) {
		local_bh_disable();
		bh_lock_sock(sk);
		if (!sock_flag(sk, SOCK_DEAD)) {
			sk->sk_err = ECONNABORTED;
			smp_wmb();
			sk->sk_error_report(sk);
			if (tcp_need_reset(sk->sk_state)) {
				hwlog_info("%s: success.\n", __func__);
				tcp_send_active_reset(sk, sk->sk_allocation);
			}
		}
		bh_unlock_sock(sk);
		local_bh_enable();
	}
	return true;
}

void destroy_push_socket(struct proxy_app_info *appinfo)
{
	u32 bucket;
	struct sock *sk = NULL;
	struct hlist_nulls_node *node = NULL;
	spinlock_t *lock = NULL;
	if (appinfo == NULL || !appinfo->is_proxy_enable)
		return;
	if (appinfo->proxy_state != DCP_PROXY_STATE)
		return;

	hwlog_info("%s, uid: %u.\n", __func__, appinfo->uid);
	for (bucket = 0; bucket <= tcp_hashinfo.ehash_mask; ++bucket) {
		if (hlist_nulls_empty(&tcp_hashinfo.ehash[bucket].chain))
			continue;
		lock = inet_ehash_lockp(&tcp_hashinfo, bucket);
		spin_lock_bh(lock);
		sk_nulls_for_each(sk, node, &tcp_hashinfo.ehash[bucket].chain) {
			if (!sk_fullsock(sk))
				continue;
			if (appinfo->uid != dcp_get_uid_from_sock(sk))
				continue;
#ifdef CONFIG_HUAWEI_KSTATE
			if (appinfo->pids != (u32)sk->sk_pid)
				continue;
#endif
			if (destroy_socket(sk, appinfo))
				break;
		}
		spin_unlock_bh(lock);
	}
}

void process_proxy_enable(u32 uid)
{
	struct proxy_app_info *appinfo = NULL;
	appinfo = get_proxy_app_info_by_uid(uid);
	if (appinfo == NULL) {
		hwlog_err("%s, %u not in proxy_app_list.\n", __func__, uid);
		return;
	}

	if (appinfo->is_proxy_enable &&
		appinfo->proxy_state == DCP_PROXY_STATE &&
		appinfo->push_socket_state == FOUND_ESTABLISHED)
		notify_proxy_result(appinfo->uid, DCP_PROXY_SUCC);
	appinfo->is_proxy_enable = true;
	appinfo->fail_num = 0;
	hwlog_info("%s: uid: %u.\n", __func__, uid);
}

void process_proxy_disable(u32 uid)
{
	struct proxy_app_info *appinfo = NULL;
	appinfo = get_proxy_app_info_by_uid(uid);
	if (appinfo == NULL) {
		hwlog_err("%s, %u not in proxy_app_list.\n", __func__, uid);
		return;
	}

	appinfo->is_proxy_enable = false;
	hwlog_info("%s: uid: %u.\n", __func__, uid);
}

void process_app_update(u32 uid, u32 pid)
{
	struct proxy_app_info *appinfo = NULL;
	appinfo = get_proxy_app_info_by_uid(uid);
	if (appinfo != NULL) {
		hwlog_err("%s: failed. uid: %u, already exist.\n", __func__, uid);
		return;
	}

	appinfo = kmalloc(sizeof(struct proxy_app_info), GFP_ATOMIC);
	if (appinfo == NULL)
		return;
	INIT_LIST_HEAD(&appinfo->app_info_list);
	INIT_LIST_HEAD(&appinfo->after_proxy_stream_list);
	INIT_LIST_HEAD(&appinfo->before_proxy_stream_list);
	INIT_LIST_HEAD(&appinfo->dst_info_list);
	appinfo->proxy_state = DCP_NORMAL_STATE;
	appinfo->uid = uid;
	appinfo->pids = pid;
	appinfo->push_port = 0;
	appinfo->fail_num = 0;
	appinfo->proxy_socket_num = 0;
	appinfo->push_socket_state = NOT_FOUND;
	appinfo->is_proxy_enable = false;
	appinfo->dst_info_num = 0;
	appinfo->is_need_destroy_tcp = false;
	spin_lock_init(&appinfo->before_stream_lock);
	spin_lock_init(&appinfo->after_stream_lock);
	spin_lock_init(&appinfo->dst_info_list_lock);

	spin_lock_bh(&g_proxy_app_list_lock);
	list_add_rcu(&appinfo->app_info_list, &g_proxy_app_info);
	spin_unlock_bh(&g_proxy_app_list_lock);

	if (timer_pending(&g_dcp_push_check_timer))
		del_timer(&g_dcp_push_check_timer);
	g_dcp_push_check_timer.expires = jiffies + DCP_PUSH_CHECK;
	add_timer(&g_dcp_push_check_timer);
	hwlog_info("%s: success. uid: %u, pid: %u.\n", __func__, uid, pid);
}

void process_app_crash(u32 uid)
{
	struct proxy_app_info *appinfo = NULL;
	struct proxy_app_info *temp = NULL;
	spin_lock_bh(&g_proxy_app_list_lock);
	list_for_each_entry_safe(appinfo, temp, &g_proxy_app_info, app_info_list) {
		if (appinfo->uid == uid) {
			del_all_push_stream_info(&appinfo->before_proxy_stream_list, &appinfo->before_stream_lock);
			del_all_push_stream_info(&appinfo->after_proxy_stream_list, &appinfo->after_stream_lock);
			del_all_dst_addr_info(appinfo, &appinfo->dst_info_list);
			list_del_init(&appinfo->app_info_list);
			kfree(appinfo);
			hwlog_info("%s: success del %u info.\n", __func__, uid);
			break;
		}
	}
	spin_unlock_bh(&g_proxy_app_list_lock);
}

void process_token_update(char *token)
{
	errno_t err;
	if (token == NULL || strlen(token) < LEN_OF_TOKEN) {
		hwlog_err("%s error.\n", __func__);
		return;
	}

	g_token_info_valid = true;
	err = memcpy_s(g_token_info, LEN_OF_TOKEN, token, LEN_OF_TOKEN);
	if (err != EOK)
		hwlog_err("%s: memcpy_s token fail.\n", __func__);
}

void process_auth_token_update(char *token, int length)
{
	errno_t err;
	if (token == NULL || strlen(token) != length || strlen(token) < LEN_OF_AUTH_KEY_INFO) {
		hwlog_err("%s error.\n", __func__);
		return;
	}

	g_auth_key_valid = true;
	err = memcpy_s(g_auth_key_info, LEN_OF_AUTH_KEY_INFO, token, LEN_OF_AUTH_KEY_INFO);
	if (err != EOK)
		hwlog_err("%s: memcpy_s token fail.\n", __func__);
}

void process_sign_token_update(char *token, int length)
{
	errno_t err;
	if (token == NULL || strlen(token) != length || strlen(token) < LEN_OF_SIGN_KEY_INFO) {
		hwlog_err("%s error.\n", __func__);
		return;
	}

	g_sign_key_valid = true;
	err = memcpy_s(g_sign_key_info, LEN_OF_SIGN_KEY_INFO, token, LEN_OF_SIGN_KEY_INFO);
	if (err != EOK)
		hwlog_err("%s: memcpy_s token fail.\n", __func__);
}

void process_iaware_cmd(int uid, int pids, int type)
{
	struct proxy_app_info *appinfo = NULL;
	if (type != DESTROY_TCP)
		return;
	appinfo = get_proxy_app_info_by_uid_pid(uid, pids);
	if (appinfo == NULL)
		return;

	appinfo->is_need_destroy_tcp = true;
	up(&g_sock_destroy_sema);
	hwlog_info("%s: uid: %u, pid: %u.\n", __func__, uid, pids);
}

void process_ip_config(struct nlmsghdr *nlh)
{
	struct dcp_sockaddr_info *hmsg = NULL;
	hmsg = (struct dcp_sockaddr_info *)nlh;
	if (hmsg == NULL)
		return;
	if (hmsg->addr.sin_family == AF_INET6) {
		g_proxy_dst_v6.sin6_family = AF_INET6;
		g_proxy_dst_v6.sin6_port = hmsg->addr.v6.sin6_port;
		g_proxy_dst_v6.sin6_addr = hmsg->addr.v6.sin6_addr;
		hwlog_info("%s: sin_family: IPV6, port: %u.\n", __func__,
			htons(hmsg->addr.v6.sin6_port));
	} else if (hmsg->addr.sin_family == AF_INET) {
		g_proxy_dst_v4.sin_family = AF_INET;
		g_proxy_dst_v4.sin_port = hmsg->addr.v4.sin_port;
		g_proxy_dst_v4.sin_addr = hmsg->addr.v4.sin_addr;
		hwlog_info("%s: sin_family: IPV4, port: %u.\n", __func__,
			htons(hmsg->addr.v4.sin_port));
	} else {
		hwlog_info("%s: sin_family error.\n", __func__);
	}
}

void dcp_process_msg(struct nlmsghdr *nlh)
{
	struct dcp_app_info *hmsg = NULL;
	hmsg = (struct dcp_app_info *)nlh;
	if (hmsg == NULL)
		return;
	if (!g_enable_proxy)
		return;
	hwlog_info("%s: type = %d.\n", __func__, nlh->nlmsg_type);
	switch (nlh->nlmsg_type) {
	case NL_DCP_MSG_REG:
		g_user_space_pid = nlh->nlmsg_pid;
		break;
	case NL_DCP_MSG_UNREG:
		g_user_space_pid = 0;
		break;
	case NL_DCP_MSG_ENABLE_PXY:
		process_proxy_enable(hmsg->uid);
		break;
	case NL_DCP_MSG_DISABLE_PXY:
		process_proxy_disable(hmsg->uid);
		break;
	case NL_DCP_MSG_SET_APP_INFO:
		process_app_update(hmsg->uid, hmsg->pids);
		break;
	case NL_DCP_MSG_SET_TOKEN:
		process_token_update(hmsg->token);
		break;
	case NL_DCP_MSG_PROCESS_DIED:
		process_app_crash(hmsg->uid);
		break;
	case NL_DCP_MSG_SET_IP_CONFIG:
		process_ip_config(nlh);
		break;
	default:
		break;
	}
}

void dcp_netlink_receive(struct sk_buff *__skb)
{
	struct nlmsghdr *nlh = NULL;
	struct sk_buff *skb = NULL;
	if (__skb == NULL)
		return;
	skb = skb_get(__skb);
	if (skb == NULL)
		return;
	mutex_lock(&dcp_receive_sem);
	if (skb->len < NLMSG_HDRLEN)
		goto skb_free;

	nlh = nlmsg_hdr(skb);
	if (nlh == NULL)
		goto skb_free;
	if ((nlh->nlmsg_len < sizeof(struct nlmsghdr)) ||
		(skb->len < nlh->nlmsg_len))
		goto skb_free;
	dcp_process_msg(nlh);

skb_free:
	kfree_skb(skb);
	mutex_unlock(&dcp_receive_sem);
}

static int tcp_destroy_thread(void *data)
{
	struct proxy_app_info *appinfo = NULL;
	struct proxy_app_info *temp = NULL;
	while (!kthread_should_stop()) {
		down(&g_sock_destroy_sema);
		spin_lock_bh(&g_proxy_app_list_lock);
		list_for_each_entry_safe(appinfo, temp, &g_proxy_app_info, app_info_list) {
			if (appinfo->is_need_destroy_tcp) {
				destroy_push_socket(appinfo);
				appinfo->is_need_destroy_tcp = false;
				break;
			}
		}
		spin_unlock_bh(&g_proxy_app_list_lock);
	}
	return 0;
}

void dcp_send_data_packet(struct sock *sk)
{
	u16 src_port;
	bool is_ipv6_pkt = false;
	struct inet_sock *inet = NULL;
	struct proxy_app_info *appinfo = NULL;
	if (sk == NULL || sk->v6_index == 0)
		return;
	if (sk->is_first_packet != 0)
		return;

	sk->is_first_packet = 1;
	appinfo = get_proxy_app_info_by_sock(sk);
	if (appinfo == NULL)
		return;
	is_ipv6_pkt = is_ipv6_packet(sk);
	inet = inet_sk(sk);
	src_port = htons(inet->inet_sport);
	send_data_packet(sk, appinfo, src_port, is_ipv6_pkt);
}

void set_socket_state(struct proxy_app_info *appinfo, u16 src_port, u16 dst_port)
{
	struct list_head *list = NULL;
	spinlock_t *lock = NULL;
	struct push_stream_info *head = NULL;
	struct push_stream_info *temp = NULL;
	if (appinfo->proxy_state == DCP_NORMAL_STATE) {
		list = &appinfo->before_proxy_stream_list;
		lock = &appinfo->before_stream_lock;
	} else {
		list = &appinfo->after_proxy_stream_list;
		lock = &appinfo->after_stream_lock;
	}

	if (!spin_trylock_bh(lock))
		return;
	list_for_each_entry_safe(head, temp, list, push_stream_info_list) {
		if (head->src_port == src_port && head->dst_port == dst_port) {
			head->is_tcp_established = false;
			hwlog_info("%s, uid: %u socket %u closed.\n", __func__, appinfo->uid, src_port);
			break;
		}
	}
	spin_unlock_bh(lock);
}

void tcp_state_change_handle(struct proxy_app_info *appinfo, u16 src_port, u16 dst_port)
{
	struct list_head *list = NULL;
	spinlock_t *lock = NULL;
	if (appinfo->push_port != src_port ||
		appinfo->push_socket_state != FOUND_ESTABLISHED)
		return;

	if (appinfo->proxy_state == DCP_NORMAL_STATE) {
		list = &appinfo->before_proxy_stream_list;
		lock = &appinfo->before_stream_lock;
	} else {
		list = &appinfo->after_proxy_stream_list;
		lock = &appinfo->after_stream_lock;
		if (appinfo->is_proxy_enable && (
			dst_port == htons(g_proxy_dst_v4.sin_port) ||
			dst_port == htons(g_proxy_dst_v6.sin6_port))) {
#ifdef CONFIG_HW_BOOSTER
			send_porxy_result(appinfo->uid, DCP_PROXY_FAIL);
#endif
			notify_proxy_result(appinfo->uid, DCP_PROXY_FAIL);
		}
	}

	hwlog_info("%s, uid: %u push socket %u closed.\n", __func__, appinfo->uid, src_port);
	appinfo->proxy_state = DCP_PROXY_STATE;
	del_all_push_stream_info(list, lock);
	del_all_dst_addr_info(appinfo, &appinfo->dst_info_list);
	appinfo->push_socket_state = FOUND_CLOSED;
	appinfo->proxy_socket_num = 0;
	if (timer_pending(&g_dcp_proxy_check_timer))
		del_timer(&g_dcp_proxy_check_timer);
	g_dcp_proxy_check_timer.expires = jiffies + DCP_PROXY_CHECK;
	add_timer(&g_dcp_proxy_check_timer);
}

void dcp_close_proxy(bool result)
{
	struct proxy_app_info *appinfo = NULL;
	struct proxy_app_info *temp = NULL;
	g_enable_proxy = result;
	notify_proxy_result(SYSTEM_UID, result ? DCP_CLOUD_ENABLE : DCP_CLOUD_DISABLE);
	hwlog_info("%s: result = %u.\n", __func__,
		result? DCP_CLOUD_ENABLE : DCP_CLOUD_DISABLE);
	if (list_empty(&g_proxy_app_info))
		return;
	if (!result) {
		spin_lock_bh(&g_proxy_app_list_lock);
		list_for_each_entry_safe(appinfo, temp, &g_proxy_app_info, app_info_list) {
			del_all_push_stream_info(&appinfo->before_proxy_stream_list, &appinfo->before_stream_lock);
			del_all_push_stream_info(&appinfo->after_proxy_stream_list, &appinfo->after_stream_lock);
			del_all_dst_addr_info(appinfo, &appinfo->dst_info_list);
			list_del_init(&appinfo->app_info_list);
			kfree(appinfo);
			hwlog_err("%s: success del app info.\n", __func__);
		}
		spin_unlock_bh(&g_proxy_app_list_lock);
	}
}

void dcp_tcp_state_change(struct sock *sk, int curr_tcp_state)
{
	u8 proxy_state;
	u16 src_port;
	u16 dst_port;
	int last_tcp_state;
	struct inet_sock *inet = NULL;
	struct proxy_app_info *appinfo = NULL;
	if (!is_full_sock(sk))
		return;
	inet = inet_sk(sk);
	if (inet == NULL)
		return;
	last_tcp_state = sk->sk_state;
	if (last_tcp_state == curr_tcp_state)
		return;
	appinfo = get_proxy_app_info_by_sock(sk);
	if (appinfo == NULL)
		return;
	src_port = htons(inet->inet_sport);
	dst_port = htons(inet->inet_dport);
	proxy_state = appinfo->proxy_state;
	if (proxy_state == DCP_PROXY_STATE &&
		appinfo->push_socket_state == FOUND_CLOSED) {
		if (last_tcp_state == TCP_SYN_SENT &&
			curr_tcp_state != TCP_ESTABLISHED &&
			sk->v6_index > 0)
			appinfo->fail_num++;
	}

	if (appinfo->fail_num == MAX_SYN_FAIL_NUM && appinfo->is_proxy_enable) {
		hwlog_err("%s, %u reached max syn fail num.\n", __func__, appinfo->uid);
		appinfo->is_proxy_enable = false;
		notify_proxy_result(appinfo->uid, DCP_PROXY_BYPASS);
	}

	if (last_tcp_state == TCP_ESTABLISHED && curr_tcp_state != TCP_ESTABLISHED) {
		set_socket_state(appinfo, src_port, dst_port);
		tcp_state_change_handle(appinfo, src_port, dst_port);
	}
}

void dcp_tcp_v6_change_soackaddr(struct sock *sk, struct sockaddr_in6 *usin)
{
	struct proxy_app_info *appinfo = NULL;
	if (g_proxy_dst_v6.sin6_port == 0)
		return;
	if (usin == NULL)
		return;
	appinfo = get_proxy_app_info_by_sock(sk);
	if (appinfo == NULL)
		return;
	if (appinfo->proxy_state != DCP_PROXY_STATE)
		return;
	if (appinfo->push_socket_state != FOUND_CLOSED)
		return;
	if (!appinfo->is_proxy_enable)
		return;
	if (appinfo->proxy_socket_num == MAX_DEST_ADDR_COUNT) {
		hwlog_err("%s, uid: %u proxy_socket_num reached %d.\n",
			__func__, appinfo->uid, MAX_DEST_ADDR_COUNT);
		return;
	}

	g_index++;
	if (record_ipv6_proxy_sock_addrinfo(appinfo, g_index, usin)) {
		sk->v6_index = g_index;
		usin->sin6_port = g_proxy_dst_v6.sin6_port;
		usin->sin6_addr = g_proxy_dst_v6.sin6_addr;
	}
}

void dcp_tcp_v4_change_soackaddr(struct sock *sk, struct sockaddr_in *usin)
{
	struct proxy_app_info *appinfo = NULL;
	if (g_proxy_dst_v4.sin_port == 0)
		return;
	if (usin == NULL)
		return;
	appinfo = get_proxy_app_info_by_sock(sk);
	if (appinfo == NULL)
		return;
	if (appinfo->proxy_state != DCP_PROXY_STATE)
		return;
	if (appinfo->push_socket_state != FOUND_CLOSED)
		return;
	if (!appinfo->is_proxy_enable)
		return;
	if (appinfo->proxy_socket_num == MAX_DEST_ADDR_COUNT) {
		hwlog_err("%s, uid: %u proxy_socket_num reached %d.\n",
			__func__, appinfo->uid, MAX_DEST_ADDR_COUNT);
		return;
	}

	g_index++;
	if (record_ipv4_proxy_sock_addrinfo(appinfo, g_index, usin)) {
		sk->v6_index = g_index;
		usin->sin_port = g_proxy_dst_v4.sin_port;
		usin->sin_addr = g_proxy_dst_v4.sin_addr;
	}
}

struct nf_hook_ops net_hooks[] = {
	{
		.hook = hook_v4_dcp,
		.pf = AF_INET,
		.hooknum = NF_INET_LOCAL_IN,
		.priority = NF_IP_PRI_FILTER + 1,
	},
	{
		.hook = hook_v6_dcp,
		.pf = AF_INET6,
		.hooknum = NF_INET_LOCAL_IN,
		.priority = NF_IP_PRI_FILTER + 1,
	}
};

void dcp_context_init(void)
{
	INIT_LIST_HEAD(&g_proxy_app_info);
	(void)memset_s(g_token_info, LEN_OF_TOKEN, 0, LEN_OF_TOKEN);
	(void)memset_s(g_auth_key_info, LEN_OF_AUTH_KEY_INFO, 0, LEN_OF_AUTH_KEY_INFO);
	(void)memset_s(g_sign_key_info, LEN_OF_SIGN_KEY_INFO, 0, LEN_OF_SIGN_KEY_INFO);
	(void)memset_s(&g_proxy_dst_v6, sizeof(g_proxy_dst_v6), 0, sizeof(g_proxy_dst_v6));
	(void)memset_s(&g_proxy_dst_v4, sizeof(g_proxy_dst_v4), 0, sizeof(g_proxy_dst_v4));
	sema_init(&g_dcp_netlink_sema, 0);
	sema_init(&g_sock_destroy_sema, 0);
	spin_lock_init(&g_dcp_send_lock);
	spin_lock_init(&g_proxy_app_list_lock);
	timer_setup(&g_dcp_push_check_timer, dcp_push_check_timer, 0);
	timer_setup(&g_dcp_proxy_check_timer, dcp_proxy_check_timer, 0);
}

bool dcp_netlink_init(void)
{
	struct netlink_kernel_cfg dcp_nl_cfg = {
		.input = dcp_netlink_receive,
	};

	g_dcp_nlfd = netlink_kernel_create(&init_net,
		NETLINK_DCP_EVENT_NL, &dcp_nl_cfg);
	if (!g_dcp_nlfd)
		return false;

	return true;
}

int __init dcp_handle_module_init(void)
{
	u32 ret;
	struct task_struct *task = NULL;
	if (!dcp_netlink_init()) {
		hwlog_err("%s fail to create netlink fd.\n", __func__);
		return -1;
	}

	dcp_context_init();
	task = kthread_run(tcp_destroy_thread, NULL, "tcp_destroy_thread");
	if (IS_ERR(task)) {
		hwlog_err("%s: failed to create thread", __func__);
		task = NULL;
		return -EINVAL;
	}

	ret = nf_register_net_hooks(&init_net, net_hooks, ARRAY_SIZE(net_hooks));
	hwlog_info("dcp_handle_module_init ret = %u.", ret);
	return 0;
}

void __exit dcp_handle_module_exit(void)
{
	if (g_dcp_nlfd && g_dcp_nlfd->sk_socket) {
		sock_release(g_dcp_nlfd->sk_socket);
		g_dcp_nlfd = NULL;
	}
	nf_unregister_net_hooks(&init_net, net_hooks, ARRAY_SIZE(net_hooks));
}

module_init(dcp_handle_module_init);
module_exit(dcp_handle_module_exit);
