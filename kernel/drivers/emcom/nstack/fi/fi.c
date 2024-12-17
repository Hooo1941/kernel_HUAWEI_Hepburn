/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2019-2023. All rights reserved.
 * Description: This module is to collect IP package parameters
 * Author: songqiubin songqiubin@huawei.com
 * Create: 2019-09-18
 */

#include "fi.h"
#include <linux/highmem.h>
#include <linux/in.h>
#include <linux/ip.h>
#include <linux/ipv6.h>
#include <linux/netdevice.h>
#include <linux/tcp.h>
#include <linux/timer.h>
#include <linux/udp.h>
#include <linux/version.h>
#include <net/net_namespace.h>
#include <net/sock.h>
#include <net/tcp.h>
#include <net/udp.h>
#include "emcom/emcom_xengine.h"

#define FI_PAYLOAD_MAX_LINEARE_LEN	600

struct fi_ctx g_fi_ctx;
static struct fi_app_info_table g_fi_app_info;
static uint64_t g_fi_iface_bytes[FI_DEV_NUM];

void fi_qos_pre_update(const struct sk_buff *skb, struct fi_flow_node *flow);
void fi_filesize_pre_update(const struct sk_buff *skb, struct fi_flow_node *flow);
void fi_iface_qos_update_report(void);
static int fi_get_http_rsp_code(const char *buf, unsigned int buf_len,
	unsigned char *out_buf, unsigned int out_len);

static unsigned int fi_ipv4_hook_out(void *priv, struct sk_buff *skb,
	const struct nf_hook_state *state);

static unsigned int fi_ipv4_hook_in(void *priv, struct sk_buff *skb,
	const struct nf_hook_state *state);

#if IS_ENABLED(CONFIG_IPV6)
static unsigned int fi_ipv6_hook_out(void *priv, struct sk_buff *skb,
	const struct nf_hook_state *state);

static unsigned int fi_ipv6_hook_in(void *priv, struct sk_buff *skb,
	const struct nf_hook_state *state);
#endif

#if LINUX_VERSION_CODE >= KERNEL_VERSION(4, 14, 1)
	static const struct nf_hook_ops fi_nfhooks[] = {
#else
	static struct nf_hook_ops fi_nfhooks[] = {
#endif
	{
		.hook = fi_ipv4_hook_out,
		.pf = PF_INET,
		.hooknum = NF_INET_LOCAL_OUT,
		.priority = NF_IP_PRI_FILTER + 1,
	},
	{
		.hook = fi_ipv4_hook_in,
		.pf = PF_INET,
		.hooknum = NF_INET_LOCAL_IN,
		.priority = NF_IP_PRI_FILTER + 1,
	},
#if IS_ENABLED(CONFIG_IPV6)
	{
		.hook = fi_ipv6_hook_out,
		.pf = PF_INET6,
		.hooknum = NF_INET_LOCAL_OUT,
		.priority = NF_IP_PRI_FILTER + 1,
	},
	{
		.hook = fi_ipv6_hook_in,
		.pf = PF_INET6,
		.hooknum = NF_INET_LOCAL_IN,
		.priority = NF_IP_PRI_FILTER + 1,
	},
#endif
};

void fi_timer_start(void)
{
	if (!g_fi_ctx.is_tm_start && !timer_pending(&(g_fi_ctx.tm))) {
		g_fi_ctx.tm.expires = jiffies + HZ / FI_TIMER_INTERVAL;
		add_timer(&(g_fi_ctx.tm));
		g_fi_ctx.is_tm_start = true;
	} else {
		fi_loge("timer_pending, not need to add again");
	}
}

bool fi_check_periodic_rpt_enable(void)
{
	struct fi_app_info_node *app = NULL;
	unsigned int i, j;
	for (i = 0; i < FI_LAUNCH_APP_MAX; i++) {
		spin_lock_bh(&(g_fi_app_info.app[i].lock));
		app = &(g_fi_app_info.app[i]);
		if (app->msg.status == FI_INVALID_APP_STATUS) {
			spin_unlock_bh(&(g_fi_app_info.app[i].lock));
			continue;
		}

		for (j = 0; j < FI_FLOW_IDENTIFY_MAX; j++) {
			if (app->msg.flow_identify_cfg[j].periodic[FI_DIR_UP].report_type != FI_RPT_OFF ||
				app->msg.flow_identify_cfg[j].periodic[FI_DIR_DOWN].report_type != FI_RPT_OFF) {
				spin_unlock_bh(&(g_fi_app_info.app[i].lock));
				return true;
			}
		}
		spin_unlock_bh(&(g_fi_app_info.app[i].lock));
	}
	return false;
}

void fi_del_timer_sync_if_no_periodic_rpt(bool is_periodic_rpt_enabled)
{
	if (g_fi_ctx.is_tm_start && !is_periodic_rpt_enabled) {
		del_timer_sync(&(g_fi_ctx.tm));
		g_fi_ctx.is_tm_start = false;
	}
}

void fi_register_nfhook(bool periodic_rpt)
{
	int ret;
	int i;
	struct net_device *dev = NULL;
	char *iface = NULL;
	struct rtnl_link_stats64 stats;

	mutex_lock(&(g_fi_ctx.nf_mutex));
	if (g_fi_ctx.nf_exist) {
		if (periodic_rpt)
			fi_timer_start();
		else
			fi_del_timer_sync_if_no_periodic_rpt(false);
		mutex_unlock(&(g_fi_ctx.nf_mutex));
		return;
	}
	local_irq_disable();
#if LINUX_VERSION_CODE >= KERNEL_VERSION(4, 14, 1)
	ret = nf_register_net_hooks(&init_net, fi_nfhooks, ARRAY_SIZE(fi_nfhooks));
#else
	ret = nf_register_hooks(fi_nfhooks, ARRAY_SIZE(fi_nfhooks));
#endif
	local_irq_enable();
	if (ret == 0) {
		g_fi_ctx.nf_exist = true;
		if (periodic_rpt)
			fi_timer_start();
	}
	mutex_unlock(&(g_fi_ctx.nf_mutex));
	for (i = 0; i < FI_DEV_NUM; i++) {
		iface = emcom_xengine_get_network_iface_name(i);
		dev = dev_get_by_name(&init_net, iface);
		if (dev == NULL) {
			/* clear g_fi_iface_bytes if dev is off */
			g_fi_iface_bytes[i] = 0;
			fi_logi(" can't find dev %s", iface);
			continue;
		}
		read_lock(&dev_base_lock);
		dev_get_stats(dev, &stats);
		read_unlock(&dev_base_lock);
		dev_put(dev);
		g_fi_iface_bytes[i] = stats.rx_bytes;
		fi_logi("init ifname: %s, total_byes: %lu", iface, stats.rx_bytes);
	}
	if (ret == 0)
		fi_logi("FI register nf hooks successfully");
	else
		fi_loge("FI register nf hooks failed, ret:%d", ret);
}

void fi_unregister_nfhook(void)
{
	mutex_lock(&(g_fi_ctx.nf_mutex));
	if (!g_fi_ctx.nf_exist) {
		mutex_unlock(&(g_fi_ctx.nf_mutex));
		return;
	}

	local_irq_disable();
#if LINUX_VERSION_CODE >= KERNEL_VERSION(4, 14, 1)
	nf_unregister_net_hooks(&init_net, fi_nfhooks, ARRAY_SIZE(fi_nfhooks));
#else
	nf_unregister_hooks(fi_nfhooks, ARRAY_SIZE(fi_nfhooks));
#endif
	local_irq_enable();

	g_fi_ctx.nf_exist = false;
	del_timer_sync(&(g_fi_ctx.tm));
	fi_ipv4_flow_clear();
#if IS_ENABLED(CONFIG_IPV6)
	fi_ipv6_flow_clear();
#endif
	mutex_unlock(&(g_fi_ctx.nf_mutex));
	fi_logi("FI unregister nf hooks successfully");
}

#if LINUX_VERSION_CODE < KERNEL_VERSION(4, 19, 0)
void fi_timer(unsigned long arg)
#else
void fi_timer(struct timer_list *t)
#endif
{
	fi_flow_stat_update_report();
	fi_flow_qos_update_report();
	fi_iface_qos_update_report();
	fi_ipv4_flow_age();
#if IS_ENABLED(CONFIG_IPV6)
	fi_ipv6_flow_age();
#endif
	mod_timer(&(g_fi_ctx.tm), jiffies + HZ / FI_TIMER_INTERVAL);
}

static inline bool fi_payloadlen_parse(struct sk_buff *skb, struct fi_pkt_parse *pktinfo);
static inline bool fi_tcpseq_parse(struct sk_buff *skb, struct fi_pkt_parse *pktinfo);
static inline bool fi_tcpack_parse(struct sk_buff *skb, struct fi_pkt_parse *pktinfo);
static inline bool fi_tcpflags_parse(struct sk_buff *skb, struct fi_pkt_parse *pktinfo);
static inline bool fi_tcpwindowsize_parse(struct sk_buff *skb, struct fi_pkt_parse *pktinfo);
static inline bool fi_tcpoptions_parse(struct sk_buff *skb, struct fi_pkt_parse *pktinfo);
static inline bool fi_payload_segment_parse(struct sk_buff *skb, struct fi_pkt_parse *pktinfo);

static struct fi_pkt_parse_f g_fi_pkt_parse_f[] = {
	{FI_PAYLOADLEN_PARSE,		fi_payloadlen_parse},
	{FI_PAYLOAD_SEGMENT_PARSE,	fi_payload_segment_parse},
	{FI_ECHO_DELAY,				NULL},
	{FI_TCPSEQ_PARSE,			fi_tcpseq_parse},
	{FI_TCPACK_PARSE,			fi_tcpack_parse},
	{FI_TCPFLAGS_PARSE,			fi_tcpflags_parse},
	{FI_TCPWINDOWSIZE_PARSE,	fi_tcpwindowsize_parse},
	{FI_TCPOPTIONS_PARSE,		fi_tcpoptions_parse},
	{FI_END_PARSE,				NULL},
};

static uint fi_get_hdr_len(const struct sk_buff *skb, uint8_t proto)
{
	if (proto == IPPROTO_UDP)
		return sizeof(struct udphdr);

	return tcp_hdrlen(skb);
}

static uint32_t fi_get_payload_parse_len(fi_flow_identify_cfg *flow_cfg,
	uint32_t stat_stop, enum fi_dir dir)
{
	uint32_t pkt_report_len = 0;
	uint32_t period_report_len = 0;

	if (!(stat_stop & FI_FLOW_PKT_REPORT_STOP))
		pkt_report_len = flow_cfg->pkt[dir].seg_end + 1;

	if (!(stat_stop & FI_FLOW_PERIODIC_REPORT_STOP))
		period_report_len = flow_cfg->periodic[dir].seg_end + 1;

	return max(pkt_report_len, period_report_len);
}

static uint32_t fi_copy_payload_by_offset(char *dst, uint32_t dst_len, char *src, uint32_t copy_len)
{
	if (unlikely(src == NULL)) {
		fi_loge("src address is null");
		return 0;
	}

	if (dst_len < copy_len)
		copy_len = dst_len;

	if (memcpy_s(dst, dst_len, src, copy_len) != EOK) {
		fi_loge("copy payload error");
		return 0;
	}
	return copy_len;
}

static uint16_t fi_get_pkt_report_len(fi_flow_identify_cfg *flow_cfg, uint32_t parse_len, enum fi_dir dir)
{
	if (parse_len == 0 ||
		flow_cfg->pkt[dir].seg_end <= flow_cfg->pkt[dir].seg_begin ||
		flow_cfg->pkt[dir].seg_begin >= parse_len)
		return 0;

	return min(flow_cfg->pkt[dir].seg_end - flow_cfg->pkt[dir].seg_begin + 1, parse_len);
}

static bool fi_payloadlen_parse(struct sk_buff *skb, struct fi_pkt_parse *pktinfo)
{
	if (pktinfo->msg.flow_msg.l4proto == IPPROTO_UDP) {
		if (skb->len > skb_transport_offset(skb) + sizeof(struct udphdr))
			pktinfo->msg.payload_len = skb->len -
				skb_transport_offset(skb) - sizeof(struct udphdr);
		else
			pktinfo->msg.payload_len = 0;
	} else {
		if (skb->len > skb_transport_offset(skb) + tcp_hdrlen(skb))
			pktinfo->msg.payload_len = skb->len - skb_transport_offset(skb) - tcp_hdrlen(skb);
		else
			pktinfo->msg.payload_len = 0;
	}

	pktinfo->msg.total_len = skb->len;
	return true;
}

static bool fi_tcpseq_parse(struct sk_buff *skb, struct fi_pkt_parse *pktinfo)
{
	struct tcphdr *tcph = NULL;

	if (pktinfo->msg.flow_msg.l4proto == IPPROTO_TCP) {
		tcph = tcp_hdr(skb);
		pktinfo->msg.tcp_seq = ntohl(tcph->seq);
		return true;
	}
	return true;
}

static bool fi_tcpack_parse(struct sk_buff *skb, struct fi_pkt_parse *pktinfo)
{
	struct tcphdr *tcph = NULL;

	if (pktinfo->msg.flow_msg.l4proto == IPPROTO_TCP) {
		tcph = tcp_hdr(skb);
		pktinfo->msg.tcp_ack = ntohl(tcph->ack);
		return true;
	}
	return true;
}

static bool fi_tcpflags_parse(struct sk_buff *skb, struct fi_pkt_parse *pktinfo)
{
	struct tcphdr *tcph = NULL;

	if (pktinfo->msg.flow_msg.l4proto == IPPROTO_TCP) {
		tcph = tcp_hdr(skb);
		pktinfo->msg.tcp_flags |= tcph->fin;
		pktinfo->msg.tcp_flags |= tcph->syn << TCPHDR_OFFSET_SYN;
		pktinfo->msg.tcp_flags |= tcph->rst << TCPHDR_OFFSET_RST;
		pktinfo->msg.tcp_flags |= tcph->psh << TCPHDR_OFFSET_PSH;
		pktinfo->msg.tcp_flags |= tcph->ack << TCPHDR_OFFSET_ACK;
		pktinfo->msg.tcp_flags |= tcph->urg << TCPHDR_OFFSET_URG;
		pktinfo->msg.tcp_flags |= tcph->ece << TCPHDR_OFFSET_ECE;
		pktinfo->msg.tcp_flags |= tcph->cwr << TCPHDR_OFFSET_CWR;
		return true;
	}
	return true;
}

static bool fi_tcpwindowsize_parse(struct sk_buff *skb, struct fi_pkt_parse *pktinfo)
{
	struct tcphdr *tcph = NULL;

	if (pktinfo->msg.flow_msg.l4proto == IPPROTO_TCP) {
		tcph = tcp_hdr(skb);
		pktinfo->msg.tcp_window = ntohs(tcph->window);
		return true;
	}
	return true;
}

static bool fi_tcpoptions_parse(struct sk_buff *skb, struct fi_pkt_parse *pktinfo)
{
	unsigned int optionslen;
	int ret;

	if (pktinfo->msg.flow_msg.l4proto != IPPROTO_TCP)
		return true;
	if (tcp_hdrlen(skb) <= sizeof(struct tcphdr))
		return true;
	optionslen = tcp_hdrlen(skb) - sizeof(struct tcphdr);
	if (optionslen > FI_TCP_OPTIONS_LEN)
		return false;
	ret = memcpy_s(pktinfo->msg.tcp_options, FI_TCP_OPTIONS_LEN,
		(char *)(skb_transport_header(skb) + sizeof(struct tcphdr)), optionslen);
	if (ret) {
		fi_loge("memset_s failed ret=%d", ret);
		return false;
	}

	return true;
}

static bool fi_payload_segment_parse(struct sk_buff *skb, struct fi_pkt_parse *pktinfo)
{
	const skb_frag_t *frag = NULL;
	uint transport_len = 0;
	uint payload_len = 0;
	char *payload = NULL;
	uint i = 0;
	uint32_t offset = 0;

	pktinfo->parse_len = min((uint32_t)pktinfo->msg.payload_len, pktinfo->parse_len);
	if (pktinfo->parse_len == 0)
		return false;

	transport_len = fi_get_hdr_len(skb, pktinfo->msg.flow_msg.l4proto);
	/* linear area process */
	if (!skb_is_nonlinear(skb)) {
		pktinfo->payload = skb_transport_header(skb) + transport_len;
		return true;
	}

	/* nonlinear area process */
	/* copy from the linear region */
	if (skb_headlen(skb) > skb_transport_offset(skb) + transport_len) {
		payload_len = skb_headlen(skb) - skb_transport_offset(skb) - transport_len;
		payload = skb_transport_header(skb) + transport_len;
		if (payload_len >= pktinfo->parse_len) {
			pktinfo->payload = payload;
			return true;
		}
		pktinfo->payload = kzalloc(pktinfo->parse_len, GFP_ATOMIC);
		pktinfo->is_malloc_buf = true;
		offset += fi_copy_payload_by_offset(pktinfo->payload + offset,
			pktinfo->parse_len - offset, payload, payload_len);
	}

	/* copy from the frags */
	for (i = 0; i < skb_shinfo(skb)->nr_frags && i < MAX_SKB_FRAGS; i++) {
		frag = (const skb_frag_t *)(&skb_shinfo(skb)->frags[i]);
		payload_len = skb_frag_size(frag);
		payload = skb_frag_address_safe(frag);
		if (i == 0 && !pktinfo->is_malloc_buf) {
			if (payload_len >= pktinfo->parse_len) {
				pktinfo->payload = payload;
				return true;
			}
			pktinfo->payload = kzalloc(pktinfo->parse_len, GFP_ATOMIC);
			pktinfo->is_malloc_buf = true;
		}
		offset += fi_copy_payload_by_offset(pktinfo->payload + offset,
			pktinfo->parse_len - offset, payload, payload_len);
		if (offset >= pktinfo->parse_len)
			return true;
	}

	pktinfo->parse_len = offset;
	/* now we do not copy from frag_list, because linear area and frags is enough */
	if (pktinfo->payload != NULL)
		return true;
	return false;
}

static void fi_payload_update_frags(struct fi_pkt_parse *pktinfo)
{
	fi_rpt_cfg *periodic_cfg = NULL;

	pktinfo->raw_frag_num++;
	periodic_cfg = &pktinfo->flow_cb.flow_identify_cfg->periodic[pktinfo->dir]; // only use periodic_cfg!!
	if (fi_payload_len_filter_check(pktinfo->msg.payload_len,
			periodic_cfg->filter_pkt_size_start, periodic_cfg->filter_pkt_size_stop) &&
		fi_payload_str_filter_check(pktinfo, periodic_cfg)) {
		pktinfo->filter_frag_num++;
		pktinfo->filter_frag_payload_len = pktinfo->frag_payload_len;
		pktinfo->filter_frag_payload = pktinfo->frag_payload;
	}
}

static void fi_payload_frag_meta_frags_parse(struct fi_pkt_parse *pktinfo, const struct sk_buff *skb)
{
	int tmp_num = 1;
	int i = 0;
	int j = 0;
	unsigned short gso_size = 0;
	const skb_frag_t *frag = NULL;
	char *frag_addr = NULL;
	if (skb_shinfo(skb)->nr_frags == 0)
		return;

	gso_size = skb_shinfo(skb)->gso_size;
	for (i = 0; i < skb_shinfo(skb)->nr_frags && i < MAX_SKB_FRAGS; i++) {
		frag = (const skb_frag_t *)(&skb_shinfo(skb)->frags[i]);
		frag_addr = skb_frag_address_safe(frag);
		if (!skb_is_gso(skb)) {
			pktinfo->is_multi_frag = true;
			pktinfo->frag_payload_len = skb_frag_size(frag);
			pktinfo->frag_payload = frag_addr;
			fi_payload_update_frags(pktinfo);
		} else if (skb_frag_size(frag) >= gso_size) {
			pktinfo->is_multi_frag = true;
			tmp_num = skb_frag_size(frag) / gso_size;
			for (j = 0; j < tmp_num; j++) {
				pktinfo->frag_payload_len = gso_size;
				pktinfo->frag_payload = (frag_addr != NULL ? frag_addr + j * gso_size : NULL);
				fi_payload_update_frags(pktinfo);
			}
			if (skb_frag_size(frag) % gso_size == 0)
				continue;
			pktinfo->frag_payload_len = skb_frag_size(frag) % gso_size;
			pktinfo->frag_payload = (frag_addr != NULL ? frag_addr + tmp_num * gso_size : NULL);
			if (frag_addr != NULL && pktinfo->frag_payload - frag_addr + pktinfo->frag_payload_len >
				skb_frag_size(frag)) {
				pktinfo->frag_payload_len = skb_frag_size(frag);
				pktinfo->frag_payload = frag_addr;
			}
			fi_payload_update_frags(pktinfo);
		}
	}
}

static void fi_payload_frag_meta_parse(struct fi_pkt_parse *pktinfo, const struct sk_buff *skb)
{
	int transport_hdr_len = 0;
	uint16_t linear_payload_len = 0;

	/* step1: parse the linear area. linear area may have no payload or have only payload! */
	if (skb_transport_header_was_set(skb)) {
		transport_hdr_len = fi_get_hdr_len(skb, pktinfo->msg.flow_msg.l4proto);
		if (skb_headlen(skb) > skb_transport_offset(skb) + transport_hdr_len)
			linear_payload_len = skb_headlen(skb) - skb_transport_offset(skb) - transport_hdr_len;
	} else {
		linear_payload_len = skb_headlen(skb);
	}
	if (linear_payload_len > 0) { // linear area has payload
		pktinfo->frag_payload_len = linear_payload_len;
		pktinfo->frag_payload = skb_transport_header_was_set(skb) ?
			skb_transport_header(skb) + transport_hdr_len : skb->data;
		fi_payload_update_frags(pktinfo);
	}

	/* step2: parse the frags area */
	fi_payload_frag_meta_frags_parse(pktinfo, skb);
}

static void fi_payload_frag_parse(struct fi_pkt_parse *pktinfo, const struct sk_buff *skb)
{
	struct sk_buff *frag_list = NULL;
	pktinfo->is_multi_frag = false;
	if (skb_is_gso(skb))
		pktinfo->is_multi_frag = true;

	/* step1: parse the linear and frags area */
	fi_payload_frag_meta_parse(pktinfo, skb);

	/* step2: parse the frag_list area, no recursive */
	if (skb_shinfo(skb)->frag_list != NULL) {
		frag_list = skb_shinfo(skb)->frag_list;
		pktinfo->is_multi_frag = true;
		while (frag_list && pktinfo->raw_frag_num < FI_SKB_MAX_FRAG_LIST_NUM) {
			fi_payload_frag_meta_parse(pktinfo, frag_list);
			frag_list = frag_list->next;
		}
	}

	if (pktinfo->is_multi_frag &&
		pktinfo->filter_frag_payload != NULL && pktinfo->filter_frag_payload_len != 0) {
		pktinfo->frag_payload = pktinfo->filter_frag_payload;
		pktinfo->frag_payload_len = pktinfo->filter_frag_payload_len;
	}
}

static bool is_tcp_protocol(const struct sk_buff *skb)
{
	struct iphdr *iphdr = NULL;
	struct ipv6hdr *ipv6hdr = NULL;
	int thoff = 0;

	if (ntohs(skb->protocol) == ETH_P_IP) {
		iphdr = ip_hdr(skb);
		return iphdr != NULL && iphdr->protocol == IPPROTO_TCP;
	} else {
		ipv6hdr = ipv6_hdr(skb);
		return ipv6hdr != NULL && ipv6_find_hdr(skb, &thoff, -1, NULL, NULL) == IPPROTO_TCP;
	}
}

/**
 * parse http dl packet and return the http response code
 * return 0: may try parse next time; -1: no need parse next time; other: the http real rsp code
 */
int fi_parse_http_rsp_code(const struct sk_buff *skb, char *payload, uint32_t parse_len)
{
	uint16_t port;
	unsigned char rsp_code[FI_HTTP_RSP_CODE_MAX_LEN + 1] = {0};
	int code = -1;
	struct tcphdr *tcphdr = NULL;

	if (skb == NULL)
		return 0;

	if (!is_tcp_protocol(skb))
		return -1;

	tcphdr = tcp_hdr(skb);
	port = ntohs(tcphdr->source);
	if (port != EMCOM_MPFLOW_FI_PORT_80 || tcphdr->rst || tcphdr->fin)
		return -1;

	if (tcphdr->syn)
		return 0; /* deal next time */

	if (parse_len == 0 || payload == NULL) /* may be an ack */
		return 0; // deal next time

	if (parse_len < FI_MIN_HTTP_LEN) /* first dl pkt lenth samll */
		return -1;

	if (fi_get_http_rsp_code(payload, FI_MIN_HTTP_LEN, rsp_code, FI_HTTP_RSP_CODE_MAX_LEN) != 0)
		return -1;

	if (kstrtoint(rsp_code, FI_DECIMAL_BASE, &code) != 0)
		return -1;
	fi_logd("src port:%u, dst port:%u, rsp_code:%d", port, ntohs(tcphdr->dest), code);
	return (code <= 0 ? -1 : code);
}

static int fi_get_http_rsp_code(const char *buf, unsigned int buf_len,
	unsigned char *out_buf, unsigned int out_len)
{
	unsigned int i;
	unsigned int code_begin;
	unsigned int code_end;
	unsigned int code_len;
	bool has_end = false;
	const unsigned char *http_rsp_hdr = FI_HTTP_VERSION_STR;
	const unsigned int http_rsp_hdr_len = strlen(FI_HTTP_VERSION_STR);

	for (i = 0; i < http_rsp_hdr_len; i++) {
		if (buf[i] != http_rsp_hdr[i])
			return -1;
	}
	code_begin = i;
	for (i = 0; i < FI_HTTP_RSP_CODE_MAX_LEN &&
		*(buf + code_begin + i) != 0 && *(buf + code_begin + i) != '\t' &&
		*(buf + code_begin + i) != '\r' && *(buf + code_begin + i) != '\n'; i++) {
		if (*(buf + code_begin + i) == ' ') {
			code_end = code_begin + i - 1;
			has_end = true;
			break;
		}
	}
	if (!has_end)
		return -1;

	code_len = code_end - code_begin + 1;
	if (code_len == 0 || code_len > out_len)
		return -1;

	if (memcpy_s(out_buf, FI_HTTP_RSP_CODE_MAX_LEN, buf + code_begin, code_len) != EOK)
		return -1;
	return 0;
}

static void fi_reset(void)
{
	int i;

	fi_logi("fi need reset when netlink pid change]free_cnt=%d", atomic_read(&(g_fi_app_info.free_cnt)));
	if (atomic_read(&(g_fi_app_info.free_cnt)) >= FI_LAUNCH_APP_MAX)
		return;
	for (i = 0; i < FI_LAUNCH_APP_MAX; i++) {
		spin_lock_bh(&(g_fi_app_info.app[i].lock));
		if (g_fi_app_info.app[i].msg.status != FI_INVALID_APP_STATUS) {
			struct fi_app_info_node *app = &(g_fi_app_info.app[i]);

			app->rand_num = 0;
			app->msg.status = FI_INVALID_APP_STATUS;
		}
		spin_unlock_bh(&(g_fi_app_info.app[i].lock));
	}
	atomic_set(&(g_fi_app_info.free_cnt), FI_LAUNCH_APP_MAX);
	fi_unregister_nfhook();
}

static void fi_set_nl_pid(const struct fi_cfg_head *cfg)
{
	uint8_t service = FI_MPFLOW_SERVICE;
	if (cfg->len < sizeof(struct fi_cfg_head) + sizeof(uint8_t)+ sizeof(uint32_t)) {
		fi_loge(" received a message without enough FI app info, len %d, min_len %zu\n",
			cfg->len, sizeof(struct fi_cfg_head) + sizeof(uint32_t));
		return;
	}
	/* fi need reset when netlink pid change, in case of daemon process restart */
	fi_reset();

	service = *((char *)cfg + sizeof(struct fi_cfg_head));
	if (service == FI_MPROUTE_SERVICE)
		g_fi_ctx.mproute_pid = *(uint32_t *)((char *)cfg + sizeof(uint8_t) + sizeof(struct fi_cfg_head));
	else
		g_fi_ctx.nl_pid = *(uint32_t *)((char *)cfg + sizeof(uint8_t) + sizeof(struct fi_cfg_head));

	fi_logi("set netlink pid from daemon, service:%u, netlink pid:%u", service,
		(service == FI_MPROUTE_SERVICE) ? g_fi_ctx.mproute_pid : g_fi_ctx.nl_pid);
	return;
}

static void fi_iface_qos_rpt_update(void)
{
	int i;

	g_fi_ctx.iface_qos_rpt_enable = 0;
	for (i = 0; i < FI_LAUNCH_APP_MAX; i++) {
		spin_lock_bh(&(g_fi_app_info.app[i].lock));
		if (g_fi_app_info.app[i].msg.uid != 0 &&
			g_fi_app_info.app[i].msg.status == FI_COLLECT_RUN &&
			g_fi_app_info.app[i].msg.qos_rpt_enable) {
			g_fi_ctx.iface_qos_rpt_enable = 1;
			spin_unlock_bh(&(g_fi_app_info.app[i].lock));
			break;
		}
		spin_unlock_bh(&(g_fi_app_info.app[i].lock));
	}
}

static bool fi_app_info_update_status_check_invalid(const enum fi_app_status status)
{
	if ((status <= FI_INVALID_APP_STATUS) || (status >= FI_MAX_APP_STATUS)) {
		fi_loge("received an app with illegal status %d", status);
		return true;
	} else {
		return false;
	}
}

static bool fi_app_info_update_same_app_check(struct fi_app_update_msg *update_msg,
	struct fi_app_info_node *app)
{
	if (app->msg.uid == update_msg->uid)
		return true;

	return false;
}

static int protocol_trans(int proto)
{
	switch (proto) {
	case FI_TCP:
		return IPPROTO_TCP;
	case FI_UDP:
		return IPPROTO_UDP;
	default:
		break;
	}
	return proto;
}

static struct fi_app_info_node *fi_app_info_update_get_app(struct fi_app_update_msg *update_msg)
{
	struct fi_app_info_node *app = NULL;
	struct fi_app_info_node *new_app = NULL;
	int i, ret;

	for (i = 0; i < FI_LAUNCH_APP_MAX; i++) {
		spin_lock_bh(&(g_fi_app_info.app[i].lock));
		if (fi_app_info_update_same_app_check(update_msg, &g_fi_app_info.app[i]) &&
			g_fi_app_info.app[i].msg.status != FI_INVALID_APP_STATUS) {
			app = &(g_fi_app_info.app[i]);
			break;
		}
		if (new_app == NULL && g_fi_app_info.app[i].msg.status ==
			FI_INVALID_APP_STATUS) {
			new_app = &(g_fi_app_info.app[i]);
			continue;
		}
		spin_unlock_bh(&(g_fi_app_info.app[i].lock));
	}
	if (app == NULL && new_app == NULL) {
		fi_loge("reach max app launch size %d", FI_LAUNCH_APP_MAX);
		return NULL;
	} else if (app == NULL) {
		app = new_app;
		atomic_dec(&(g_fi_app_info.free_cnt));
		if (update_msg->service == FI_MPFLOW_SERVICE)
			g_fi_ctx.mpflow_app_num++;
		else
			g_fi_ctx.mproute_app_num++;
	} else if (new_app != NULL) {
		spin_unlock_bh(&(new_app->lock));
	}

	ret = memcpy_s(&(app->msg), sizeof(struct fi_app_update_msg),
		update_msg, sizeof(struct fi_app_update_msg));
	if (ret) {
		fi_loge("memcpy_s failed ret=%d", ret);
		spin_unlock_bh(&(app->lock));
		return NULL;
	}

	for (i = 0; i < FI_FLOW_IDENTIFY_MAX; i++)
		app->msg.flow_identify_cfg[i].proto = protocol_trans(app->msg.flow_identify_cfg[i].proto);

	app->rand_num++;
	spin_unlock_bh(&(app->lock));

	return app;
}

static void fi_app_info_update_proc(struct fi_app_info_node *app)
{
	fi_rpt_cfg *periodic = NULL;
	fi_rpt_cfg *pkt = NULL;
	int i;

	for (i = 0; i < FI_DIR_BOTH; i++) {
		periodic = &(app->msg.flow_identify_cfg[0].periodic[i]);
		pkt = &(app->msg.flow_identify_cfg[0].pkt[i]);
		fi_logi("Config received.app %s[%d], service:%u, status=%d,proto=%d,dport=%d,"
			"periodic[dir=%d roiType=%d roiStart=%d roiStart=%d],"
			"immediate[dir=%d roiType=%d roiStart=%d roiStart=%d] version:%s",
			app->msg.app_name, app->msg.uid, app->msg.service, app->msg.status,
			app->msg.flow_identify_cfg[0].proto, app->msg.flow_identify_cfg[0].dport[0],
			i, periodic->report_type, periodic->roi_start, periodic->roi_stop,
			i, pkt->report_type, pkt->roi_start, pkt->roi_stop, FI_VER);
	}
	fi_register_nfhook(fi_check_periodic_rpt_enable());
	fi_iface_qos_rpt_update();
}

static void fi_app_info_update(const struct fi_cfg_head *cfg)
{
	struct fi_app_update_msg *update_msg = NULL;
	struct fi_app_info_node *app = NULL;
	int i, ret;

	if (cfg->len != sizeof(struct fi_cfg_head) + sizeof(struct fi_app_update_msg)) {
		fi_loge("received message without enough FI app info, len %d, min_len %zu",
			cfg->len, sizeof(struct fi_cfg_head) + sizeof(struct fi_app_update_msg));
		return;
	}
	update_msg = (struct fi_app_update_msg *)((char *)cfg + sizeof(struct fi_cfg_head));
	if (fi_app_info_update_status_check_invalid(update_msg->status) ||
		(update_msg->status == FI_COLLECT_STOP)) {
		fi_loge("received an app with illegal status %d",
			update_msg->status);
		return;
	}

	app = fi_app_info_update_get_app(update_msg);
	if (app == NULL)
		return;

	fi_app_info_update_proc(app);
	return;
}

static struct fi_app_info_node *fi_app_status_update_and_get_app(struct fi_app_update_msg *update_msg)
{
	struct fi_app_info_node *app = NULL;
	int i;

	for (i = 0; i < FI_LAUNCH_APP_MAX; i++) {
		spin_lock_bh(&(g_fi_app_info.app[i].lock));
		if (g_fi_app_info.app[i].msg.uid == update_msg->uid &&
			g_fi_app_info.app[i].msg.status != FI_INVALID_APP_STATUS) {
			app = &(g_fi_app_info.app[i]);
			break;
		}
		spin_unlock_bh(&(g_fi_app_info.app[i].lock));
	}
	if (app == NULL) {
		fi_loge("app %s, uid %d not exist", update_msg->app_name, update_msg->uid);
		return NULL;
	}

	if (update_msg->status == FI_COLLECT_STOP) {
		app->rand_num = 0;
		app->msg.status = FI_INVALID_APP_STATUS;
		spin_unlock_bh(&(app->lock));
		fi_ipv4_flow_clear_by_uid(update_msg->uid);
#if IS_ENABLED(CONFIG_IPV6)
		fi_ipv6_flow_clear_by_uid(update_msg->uid);
#endif
		if (update_msg->service == FI_MPFLOW_SERVICE) {
			if (g_fi_ctx.mpflow_app_num > 0)
				g_fi_ctx.mpflow_app_num--;
		} else {
			if (g_fi_ctx.mproute_app_num > 0)
				g_fi_ctx.mproute_app_num--;
		}
		atomic_inc(&(g_fi_app_info.free_cnt));
	} else {
		app->msg.status = update_msg->status;
		app->rand_num++;
		spin_unlock_bh(&(app->lock));
	}

	return app;
}

static void fi_app_status_update(const struct fi_cfg_head *cfg)
{
	struct fi_app_update_msg *update_msg = NULL;
	struct fi_app_info_node *app = NULL;

	if (cfg->len != sizeof(struct fi_cfg_head) + sizeof(struct fi_app_update_msg)) {
		fi_loge("receive a message without enough FI app info, len %d, min_len %zu",
			cfg->len, sizeof(struct fi_cfg_head) + sizeof(struct fi_app_update_msg));
		return;
	}
	update_msg = (struct fi_app_update_msg *)((char *)cfg +
		sizeof(struct fi_cfg_head));
	if (fi_app_info_update_status_check_invalid(update_msg->status))
		return;

	app = fi_app_status_update_and_get_app(update_msg);
	if (app == NULL)
		return;
	if (atomic_read(&(g_fi_app_info.free_cnt)) >= FI_LAUNCH_APP_MAX)
		fi_unregister_nfhook();
	else {
		mutex_lock(&(g_fi_ctx.nf_mutex));
		fi_del_timer_sync_if_no_periodic_rpt(fi_check_periodic_rpt_enable());
		mutex_unlock(&(g_fi_ctx.nf_mutex));
	}

	fi_iface_qos_rpt_update();
	fi_logi("app %s status update, uid:%d, service:%d, status:%d, rand_num:%lu, free_cnt:%d",
		app->msg.app_name, app->msg.uid, app->msg.service, app->msg.status,
		app->rand_num, atomic_read(&(g_fi_app_info.free_cnt)));

	return;
}

static void fi_ipv4_stop_flow_pkt_report(const struct fi_flow_msg *flow_msg)
{
	struct fi_pkt_parse pktinfo;
	uint32_t hash;
	struct fi_flow_head *head = NULL;
	struct fi_flow_node *node = NULL;

	if (memset_s(&pktinfo, sizeof(pktinfo), 0, sizeof(struct fi_pkt_parse))) {
		fi_loge("memset_s failed");
		return;
	}

	pktinfo.msg.flow_msg.ipv4_sip = flow_msg->ipv4_sip;
	pktinfo.msg.flow_msg.ipv4_dip = flow_msg->ipv4_dip;
	pktinfo.msg.flow_msg.sport = flow_msg->sport;
	pktinfo.msg.flow_msg.dport = flow_msg->dport;

	hash = fi_ipv4_flow_hash(&pktinfo, FI_FLOW_LEN + 1, false);
	if (strcmp(pktinfo.msg.flow_msg.flow, flow_msg->flow) != 0) {
		fi_loge("flow:%s not equal input param:%s", pktinfo.msg.flow_msg.flow, flow_msg->flow);
		return;
	}

	head = fi_ipv4_flow_header(hash);
	if (head == NULL) {
		fi_loge("flow:%s not find, hash head is null", pktinfo.msg.flow_msg.flow);
		return;
	}

	fi_ipv4_flow_lock(hash);
	node = fi_ipv4_flow_find(head, flow_msg->ipv4_sip, flow_msg->sport, flow_msg->ipv4_dip, flow_msg->dport);
	if (node != NULL) {
		node->flow_ctx.stat_stop |= FI_FLOW_PKT_REPORT_STOP;
		fi_logi("flow:%s stop pkt report", pktinfo.msg.flow_msg.flow);
	} else {
		fi_loge("flow:%s not find", pktinfo.msg.flow_msg.flow);
	}
	fi_ipv4_flow_unlock(hash);

	return;
}

static void fi_ipv6_stop_flow_pkt_report(const struct fi_flow_msg *flow_msg);
static void fi_stop_flow_pkt_report(const struct fi_cfg_head *cfg)
{
	struct fi_flow_msg *flow_msg = NULL;
	if (cfg->len != sizeof(struct fi_cfg_head) + sizeof(struct fi_flow_msg)) {
		fi_loge("receive a message without enough flow keyinfo, len %d, min_len %zu",
			cfg->len, sizeof(struct fi_cfg_head) + sizeof(struct fi_flow_msg));
		return;
	}

	flow_msg = (struct fi_flow_msg *)((char *)cfg + sizeof(struct fi_cfg_head));
	if (flow_msg->l3proto == ETH_P_IP)
		fi_ipv4_stop_flow_pkt_report(flow_msg);
#if IS_ENABLED(CONFIG_IPV6)
	else
		fi_ipv6_stop_flow_pkt_report(flow_msg);
#endif
}

void fi_cfg_process(const struct fi_cfg_head *cfg)
{
	if (!g_fi_ctx.running) {
		fi_loge("FI not initialized yet, cfg type:%d", cfg->type);
		return;
	}
	if (cfg == NULL || cfg->len == 0)
		return;

	switch (cfg->type) {
	case FI_SET_NL_PID_CMD:
		fi_set_nl_pid(cfg);
		break;
	case FI_COLLECT_START_CMD:
	case FI_COLLECT_INFO_UPDATE_CMD:
		fi_app_info_update(cfg);
		break;
	case FI_COLLECT_STATUS_UPDATE_CMD:
		fi_app_status_update(cfg);
		break;
	case FI_STOP_FLOW_PKT_REPORT_CMD:
		fi_stop_flow_pkt_report(cfg);
		break;
	default:
		fi_loge("receive a not FI message, type:%d", cfg->type);
		break;
	}
	return;
}
EXPORT_SYMBOL(fi_cfg_process); /*lint !e580*/

static bool fi_copy_pkt_report_payload(const struct fi_pkt_parse *pktinfo,
	char *dst_buff, uint32_t dst_buff_len)
{
	int ret;
	bool is_udp_multi_frag = (pktinfo->msg.flow_msg.l4proto == IPPROTO_UDP && pktinfo->is_multi_frag);
	fi_flow_identify_cfg *flow_cfg = pktinfo->flow_cb.flow_identify_cfg;
	if (pktinfo->msg.report_len == 0)
		return true;

	if (pktinfo->payload != NULL) {
		ret = memcpy_s(dst_buff, dst_buff_len,
			pktinfo->payload + flow_cfg->pkt[pktinfo->dir].seg_begin, pktinfo->msg.report_len);
	} else {
		fi_loge("payload null");
		return false;
	}

	if (ret) {
		fi_loge("memcpy_s failed ret=%d, parse_len=%u, report_len:%u",
			ret, pktinfo->parse_len, pktinfo->msg.report_len);
		return false;
	}

	fi_logd("flow:%s udp_multi_frag:%d, get payload_seg payload_len:%u, parse_len:%u, report_len:%u. begin=%u end=%u",
		pktinfo->msg.flow_msg.flow, is_udp_multi_frag,
		pktinfo->msg.payload_len, pktinfo->parse_len, pktinfo->msg.report_len,
		flow_cfg->pkt[pktinfo->dir].seg_begin, flow_cfg->pkt[pktinfo->dir].seg_end);
	return true;
}

static void fi_pkt_msg_report(struct fi_pkt_parse *pktinfo, int type)
{
	uint32_t size;
	char *data = NULL;
	int ret;
	struct sk_buff *pskb = NULL;
	spin_lock_bh(&(pktinfo->flow_cb.app_info->lock));
	pktinfo->msg.report_len = fi_get_pkt_report_len(pktinfo->flow_cb.flow_identify_cfg,
			pktinfo->parse_len, pktinfo->dir);
	if (pktinfo->rand_num == pktinfo->flow_cb.app_info->rand_num) {
		spin_unlock_bh(&(pktinfo->flow_cb.app_info->lock));
		size = sizeof(struct fi_pkt_msg) + pktinfo->msg.report_len;
		pskb = fi_get_netlink_skb(type, size, &data);
		if (pskb == NULL) {
			fi_loge("failed to malloc memory for data, size:%u, report_len:%u",
				size, pktinfo->parse_len);
			return;
		}

		ret = memcpy_s(data, sizeof(struct fi_pkt_msg), &(pktinfo->msg), sizeof(struct fi_pkt_msg));
		if (ret) {
			fi_loge("memcpy_s failed ret:%d", ret);
			kfree_skb(pskb);
			return;
		}

		if (!fi_copy_pkt_report_payload(pktinfo, data + offsetof(struct fi_pkt_msg, payload_seg),
			size - offsetof(struct fi_pkt_msg, payload_seg))) {
			kfree_skb(pskb);
			return;
		}

		fi_enqueue_netlink_skb(pskb);
		fi_logd("finish to send a message to native, uid:%d, flow:%s, parse_len:%u, report_len:%u",
			pktinfo->msg.flow_msg.uid, pktinfo->msg.flow_msg.flow, pktinfo->parse_len, pktinfo->msg.report_len);
	} else {
		spin_unlock_bh(&(pktinfo->flow_cb.app_info->lock));
	}
	return;
}

static bool fi_check_ip_addrss(struct fi_pkt_parse *pktinfo, int32_t uid)
{
	struct sockaddr_in6 addr = {0};
	if (pktinfo->msg.flow_msg.l3proto == ETH_P_IP) {
		struct sockaddr_in *usin = (struct sockaddr_in *)&addr;
		usin->sin_family = AF_INET;
		usin->sin_addr.s_addr = pktinfo->msg.flow_msg.ipv4_dip;
	} else {
		struct sockaddr_in6 *usin6 = (struct sockaddr_in6 *)&addr;
		usin6->sin6_family = AF_INET6;
		usin6->sin6_addr = pktinfo->msg.flow_msg.ipv6_dip;
	}
	if (!emcom_xengine_check_ip_addrss((struct sockaddr *)&addr) ||
		emcom_xengine_check_ip_is_private((struct sockaddr *)&addr)) {
		fi_logd("invalid addr. uid: %d", uid);
		return false;
	}
	return true;
}

static bool fi_app_info_get(struct fi_pkt_parse *pktinfo,
				int32_t uid, enum fi_app_status status)
{
	struct fi_app_info_node *app = NULL;
	int i, j;
	bool match = false;

	if (pktinfo == NULL || !fi_check_ip_addrss(pktinfo, uid))
		return false;

	for (i = 0; i < FI_LAUNCH_APP_MAX; i++) {
		spin_lock_bh(&(g_fi_app_info.app[i].lock));
		if (g_fi_app_info.app[i].msg.uid == uid &&
			g_fi_app_info.app[i].msg.status == status) {
			app = &(g_fi_app_info.app[i]);
			break;
		}
		spin_unlock_bh(&(g_fi_app_info.app[i].lock));
	}
	if (app == NULL)
		return false;
	pktinfo->flow_cb.app_info = app;
	pktinfo->rand_num = pktinfo->flow_cb.app_info->rand_num;
	pktinfo->msg.flow_msg.uid = uid;
	for (i = 0; i < FI_FLOW_IDENTIFY_MAX; i++) {
		int proto = app->msg.flow_identify_cfg[i].proto;
		if (proto == FI_PROTO_ANY || proto == pktinfo->msg.flow_msg.l4proto) {
			for (j = 0; j < FI_DPORT_MAX; j++) {
				if (app->msg.flow_identify_cfg[i].dport[j] == FI_PORT_ANY ||
					pktinfo->msg.flow_msg.dport == app->msg.flow_identify_cfg[i].dport[j]) {
					match = true;
					break;
				}
			}
		}
		if (match)
			break;
	}
	spin_unlock_bh(&(app->lock));
	if (!match)
		return false;
	pktinfo->flow_cb.flow_identify_cfg = &app->msg.flow_identify_cfg[i];
	return true;
}

static bool fi_ipv4_pkt_check(struct sk_buff *skb)
{
	struct iphdr *iph = ip_hdr(skb);
	if (iph == NULL)
		return false;
	if (ipv4_is_loopback(iph->saddr) || ipv4_is_loopback(iph->daddr))
		return false;
	if (iph->protocol == IPPROTO_UDP) {
		if (skb->len >= skb_transport_offset(skb) + sizeof(struct udphdr))
			return true;
	} else if (iph->protocol == IPPROTO_TCP) {
		if (skb->len >= skb_transport_offset(skb) + tcp_hdrlen(skb))
			return true;
	}
	return false;
}

static void fi_ip_save_5tuple_info(struct fi_pkt_parse *pktinfo, uint16_t protocol, const struct sk_buff *skb)
{
	struct udphdr *udph = NULL;
	struct tcphdr *tcph = NULL;
	if (protocol == IPPROTO_UDP) {
		udph = udp_hdr(skb);
		pktinfo->msg.flow_msg.l4proto = IPPROTO_UDP;
		pktinfo->msg.flow_msg.sport = ntohs(udph->source);
		pktinfo->msg.flow_msg.dport = ntohs(udph->dest);
	} else {
		tcph = tcp_hdr(skb);
		pktinfo->msg.flow_msg.l4proto = IPPROTO_TCP;
		pktinfo->msg.flow_msg.sport = ntohs(tcph->source);
		pktinfo->msg.flow_msg.dport = ntohs(tcph->dest);
	}
}

static void fi_ipv4_5tuple_parse(struct fi_pkt_parse *pktinfo, const struct sk_buff *skb,
				enum fi_dir dir)
{
	struct iphdr *iph = NULL;

#if LINUX_VERSION_CODE < KERNEL_VERSION(5, 4, 0)
	struct timeval ts;
	do_gettimeofday(&ts);
	pktinfo->msg.tstamp.tv_sec = ts.tv_sec;
	pktinfo->msg.tstamp.tv_usec = ts.tv_usec;
#else
	struct timespec64 ts;
	ktime_get_real_ts64(&ts);
	pktinfo->msg.tstamp.tv_sec = ts.tv_sec;
	pktinfo->msg.tstamp.tv_usec = ts.tv_nsec / 1000;
#endif
	pktinfo->msg.flow_msg.l3proto = ntohs(skb->protocol);
	pktinfo->msg.dir = dir;
	iph = ip_hdr(skb);
	pktinfo->msg.flow_msg.ipv4_sip = iph->saddr;
	pktinfo->msg.flow_msg.ipv4_dip = iph->daddr;
	fi_ip_save_5tuple_info(pktinfo, iph->protocol, skb);
	return;
}

static void fi_ipv4_pkt_parse(struct fi_pkt_parse *pktinfo, struct sk_buff *skb)
{
	int i;

	spin_lock_bh(&(pktinfo->flow_cb.app_info->lock));
	if (pktinfo->msg.flow_msg.l4proto == IPPROTO_UDP)
		fi_payload_frag_parse(pktinfo, skb);

	for (i = 0; i < FI_END_PARSE; i++) {
		if ((pktinfo->flow_cb.flow_identify_cfg->pkt[pktinfo->dir].opt_enable[i] ||
			pktinfo->flow_cb.flow_identify_cfg->periodic[pktinfo->dir].opt_enable[i]) &&
			(g_fi_pkt_parse_f[i].opt_val_parse != NULL)) {
			fi_logd("parse target:%d function", i);
			(void)(g_fi_pkt_parse_f[i].opt_val_parse(skb, pktinfo));
		}
	}
	spin_unlock_bh(&(pktinfo->flow_cb.app_info->lock));
	return;
}

static void fi_flow_get_dev(struct fi_flow_msg *flow_msg, const struct net_device *dev)
{
	int ret = memcpy_s(flow_msg->dev, IFNAMSIZ, dev->name, strlen(dev->name) + 1);
	if (ret)
		fi_loge("memcpy_s failed ret=%d, dev %s", ret, dev->name);
	return;
}

bool fi_is_blocked_rsp_code(int rsp_code)
{
	if (rsp_code == FI_HTTP_RESPONSE_MOVED || rsp_code == FI_HTTP_RESPONSE_REDIRECT ||
		rsp_code == FI_HTTP_RESPONSE_OTHER_LOCATION || rsp_code == FI_HTTP_RESPONSE_USE_GATEWAY ||
		rsp_code == FI_HTTP_RESPONSE_BAD_REQUEST || rsp_code == FI_HTTP_RESPONSE_SERVER_INTERNAL_ERROR ||
		rsp_code == F_HTTP_RESPONSE_GATEWAY_ERROR)
		return true;
	return false;
}

static struct sock *fi_get_sock_by_dir(struct sk_buff *skb, enum fi_dir dir)
{
	if (dir == FI_DIR_UP)
		return skb->sk;
	return NULL;
}

static bool fi_up_pkt_check(struct sk_buff *skb, struct fi_pkt_parse *pktinfo,
	kuid_t *kuid, enum fi_dir dir)
{
	struct sock *sk = NULL;

	if (dir != FI_DIR_UP)
		return true;

	sk = skb->sk;
	if (sk == NULL)
		return false;
	/* If sk state is SYN_RECV, sk is actually a request_sock, can not call sock_i_uid */
	if (pktinfo->msg.flow_msg.l4proto == IPPROTO_TCP &&
		(sk->sk_state == TCP_NEW_SYN_RECV || sk->sk_state == TCP_SYN_RECV)) {
		fi_logd("Ignore SYN_RECV sk, sk_state=%d l4proto=%d port[%hu--%hu]",
			sk->sk_state, pktinfo->msg.flow_msg.l4proto, pktinfo->msg.flow_msg.sport, pktinfo->msg.flow_msg.dport);
		return false;
	}
	*kuid = sock_i_uid(sk);
	if (!fi_app_info_get(pktinfo, kuid->val, FI_COLLECT_RUN))
		return false;
	pktinfo->msg.flow_msg.dev_if = sk->sk_bound_dev_if;
	return true;
}

static void fi_flow_get_mark(struct sock *sk, struct fi_flow_msg *flow_msg)
{
	if (flow_msg && sk)
		flow_msg->sk_mark = sk->sk_bound_dev_if;
}

static void fi_update_fault_type(enum fi_dir dir, int last_tcp_state, struct fi_pkt_parse *pktinfo, const kuid_t *kuid)
{
	if (dir == FI_DIR_DOWN && pktinfo->msg.flow_msg.l4proto == IPPROTO_TCP && last_tcp_state < TCP_FIN_WAIT1 &&
		(pktinfo->msg.tcp_flags & (TCPHDR_FIN | TCPHDR_RST))) {
		fi_logi("rcv active tcp fin/rst for sk port:%u, uid:%u", pktinfo->msg.flow_msg.dport, kuid->val);
		pktinfo->msg.flow_msg.flow_fault_type = FI_FLOW_FAULT_CLOSE_BY_NW;
	}
}

static void fi_update_last_tcp_state(int *last_tcp_state, struct fi_flow_node *flow)
{
	*last_tcp_state = flow->flow_ctx.tcp_state;
	if (flow->sk)
		flow->flow_ctx.tcp_state = flow->sk->sk_state;
}

static bool fi_is_flow_already_closed(const struct fi_pkt_parse *pktinfo, const struct fi_flow_node *flow,
	int last_tcp_state)
{
	return pktinfo->msg.flow_msg.l4proto == IPPROTO_TCP && flow->sk && flow->sk->sk_state == TCP_CLOSE &&
		last_tcp_state == TCP_CLOSE;
}

static bool fi_is_all_stat_report_stopped(const struct fi_pkt_parse *pktinfo, const struct fi_flow_node *flow)
{
	return (flow->flow_ctx.stat_stop & FI_FLOW_ALL_REPORT_STOP) == FI_FLOW_ALL_REPORT_STOP &&
		!flow->flow_cb.flow_identify_cfg->basic_rpt_en &&
		(pktinfo->msg.flow_msg.flow_fault_type == FI_FLOW_FAULT_NONE);
}

static bool fi_ipv4_pktinfo_update(struct sk_buff *skb, struct fi_pkt_parse *pktinfo, kuid_t *kuid, enum fi_dir dir)
{
	if (unlikely(skb == NULL))
		return false;
	if (!fi_ipv4_pkt_check(skb))
		return false;

	if (memset_s(pktinfo, sizeof(struct fi_pkt_parse), 0, sizeof(struct fi_pkt_parse))) {
		fi_loge("ipv4 hook memset_s failed");
		return false;
	}

	fi_ipv4_5tuple_parse(pktinfo, skb, dir);
	return fi_up_pkt_check(skb, pktinfo, kuid, dir);
}

static bool fi_check_flow_already_closed(int *last_tcp_state, struct fi_flow_node *flow,
				struct fi_pkt_parse *pktinfo)
{
	fi_flow_get_mark(flow->sk, &(flow->flow_ctx.flow_msg));

	fi_update_last_tcp_state(last_tcp_state, flow);

	if (fi_is_flow_already_closed(pktinfo, flow, *last_tcp_state)) {
		fi_logd("Ignore flow:l4proto=%d,sk=%pK, sk_state=%d port[%hu--%hu]",
			pktinfo->msg.flow_msg.l4proto, flow->sk, flow->sk->sk_state,
			pktinfo->msg.flow_msg.sport, pktinfo->msg.flow_msg.dport);
		return true;
	}
	return false;
}

static bool fi_flow_qos_and_report_pre_check(struct fi_pkt_parse *pktinfo, const struct nf_hook_state *state,
				struct fi_flow_node *flow, struct sk_buff *skb, enum fi_dir dir)
{
	if (pktinfo->msg.flow_msg.new_flow == true) {
		if (state->out) {
			fi_flow_get_dev(&pktinfo->msg.flow_msg, state->out);
			fi_flow_get_dev(&flow->flow_ctx.flow_msg, state->out);
		} else {
			fi_logi("new flow dev is null");
		}
	}
	if (fi_flow_qos_rpt_enable(flow) && pktinfo->msg.flow_msg.l4proto == IPPROTO_TCP) {
		if (dir == FI_DIR_UP)
			fi_filesize_pre_update(skb, flow);
		else
			fi_qos_pre_update(skb, flow);
	}

	/* if all stat report have stopped, bypass this packet */
	return fi_is_all_stat_report_stopped(pktinfo, flow);
}

static void fi_set_pktinfo_from_flow(struct fi_pkt_parse *pktinfo, struct fi_flow_node *flow, enum fi_dir dir)
{
	if (dir == FI_DIR_DOWN) {
		pktinfo->flow_cb = flow->flow_cb;
		pktinfo->rand_num = pktinfo->flow_cb.app_info->rand_num;
		pktinfo->msg.flow_msg.uid = flow->uid;
		pktinfo->msg.flow_msg.dev_if = flow->flow_ctx.flow_msg.dev_if;
	}
	/* if the payload is segmented, copy payload to pktinfo->payload */
	pktinfo->dir = dir;
	pktinfo->is_malloc_buf = false;
	pktinfo->parse_len = fi_get_payload_parse_len(flow->flow_cb.flow_identify_cfg, flow->flow_ctx.stat_stop, dir);
}

static void fi_ipv4_hook(struct sk_buff *ipv4_skb, const struct nf_hook_state *state, enum fi_dir dir)
{
	kuid_t kuid = {0};
	struct fi_pkt_parse pktinfo;
	uint32_t hash;
	bool enable = false;
	struct fi_flow_head *head = NULL;
	struct fi_flow_node *flow = NULL;
	struct sk_buff *skb = ipv4_skb;
	int last_tcp_state = 0;

	if (!fi_ipv4_pktinfo_update(skb, &pktinfo, &kuid, dir))
		return;

	hash = fi_ipv4_flow_hash(&pktinfo, FI_FLOW_LEN + 1,
		((dir == FI_DIR_DOWN) ? true : false));
	head = fi_ipv4_flow_header(hash);
	fi_ipv4_flow_lock(hash);
	flow = fi_ipv4_flow_get(head, kuid.val, &pktinfo,
		((dir == FI_DIR_UP) ? true : false), fi_get_sock_by_dir(skb, dir));
	if (flow == NULL || fi_check_flow_already_closed(&last_tcp_state, flow, &pktinfo) ||
		fi_flow_qos_and_report_pre_check(&pktinfo, state, flow, skb, dir)) {
		fi_ipv4_flow_unlock(hash);
		return;
	}

	fi_set_pktinfo_from_flow(&pktinfo, flow, dir);
	fi_ipv4_pkt_parse(&pktinfo, skb);

	if (dir == FI_DIR_DOWN && flow->flow_ctx.key_info_check_rst == FI_RPT_NULL) {
		int http_rsp_code = fi_parse_http_rsp_code(skb, pktinfo.payload, pktinfo.parse_len);
		if (http_rsp_code > 0 && fi_is_blocked_rsp_code(http_rsp_code))
			flow->flow_ctx.key_info_check_rst = FI_RPT_BLOCK;
		else if (http_rsp_code > 0 || http_rsp_code == -1)
			flow->flow_ctx.key_info_check_rst = FI_RPT_GO;
	}

	fi_update_fault_type(dir, last_tcp_state, &pktinfo, &kuid);

	fi_flow_statistics(flow, &pktinfo, dir);
	enable = (fi_pkt_report_check(flow, &pktinfo, dir) &&
		flow->flow_ctx.key_info_check_rst != FI_RPT_BLOCK) ||
		(pktinfo.msg.flow_msg.flow_fault_type > FI_FLOW_FAULT_NONE);
	if (enable) {
		int ret = strcpy_s(pktinfo.msg.flow_msg.dev, IFNAMSIZ, flow->flow_ctx.flow_msg.dev);
		if (ret)
			fi_loge("strcpy_s failed ret=%d, dev=%s", ret, flow->flow_ctx.flow_msg.dev);
	}
	fi_ipv4_flow_unlock(hash);

	if (enable)
		fi_pkt_msg_report(&pktinfo, FI_PKT_MSG_RPT);

	if (pktinfo.is_malloc_buf)
		kfree(pktinfo.payload);
}

static unsigned int fi_ipv4_hook_out(void *priv, struct sk_buff *skb,
	const struct nf_hook_state *state)
{
	fi_ipv4_hook(skb, state, FI_DIR_UP);
	return NF_ACCEPT;
}

static unsigned int fi_ipv4_hook_in(void *priv, struct sk_buff *skb,
	const struct nf_hook_state *state)
{
	fi_ipv4_hook(skb, state, FI_DIR_DOWN);
	return NF_ACCEPT;
}

#if IS_ENABLED(CONFIG_IPV6)
static void fi_ipv6_stop_flow_pkt_report(const struct fi_flow_msg *flow_msg)
{
	struct fi_pkt_parse pktinfo;
	uint32_t hash;
	struct fi_flow_head *head = NULL;
	struct fi_flow_node *node = NULL;

	if (memset_s(&pktinfo, sizeof(pktinfo), 0, sizeof(struct fi_pkt_parse))) {
		fi_loge("memset_s failed");
		return;
	}

	pktinfo.msg.flow_msg.ipv6_sip = flow_msg->ipv6_sip;
	pktinfo.msg.flow_msg.ipv6_dip = flow_msg->ipv6_dip;
	pktinfo.msg.flow_msg.sport = flow_msg->sport;
	pktinfo.msg.flow_msg.dport = flow_msg->dport;

	hash = fi_ipv6_flow_hash(&pktinfo, FI_FLOW_LEN + 1, false);
	if (strcmp(pktinfo.msg.flow_msg.flow, flow_msg->flow) != 0) {
		fi_loge("flow:%s not equal input param:%s", pktinfo.msg.flow_msg.flow, flow_msg->flow);
		return;
	}

	head = fi_ipv6_flow_header(hash);
	if (head == NULL) {
		fi_loge("flow:%s not find, hash head is null", pktinfo.msg.flow_msg.flow);
		return;
	}

	fi_ipv6_flow_lock(hash);
	node = fi_ipv6_flow_find(head, &flow_msg->ipv6_sip, flow_msg->sport, &flow_msg->ipv6_dip, flow_msg->dport);
	if (node != NULL) {
		node->flow_ctx.stat_stop |= FI_FLOW_PKT_REPORT_STOP;
		fi_logi("flow:%s stop pkt report", pktinfo.msg.flow_msg.flow);
	} else {
		fi_loge("flow:%s not find", pktinfo.msg.flow_msg.flow);
	}
	fi_ipv6_flow_unlock(hash);
}

static bool fi_ipv6_pkt_check(struct sk_buff *skb)
{
	int thoff = 0;
	int tproto;
	struct ipv6hdr *iph = ipv6_hdr(skb);
	if (iph == NULL)
		return false;
	if (ipv6_addr_loopback(&(iph->saddr)) ||
		ipv6_addr_loopback(&(iph->daddr)))
		return false;
	tproto = ipv6_find_hdr(skb, &thoff, -1, NULL, NULL);
	if (tproto == IPPROTO_UDP) {
		if (skb->len >= skb_transport_offset(skb) + sizeof(struct udphdr))
			return true;
	} else if (tproto == IPPROTO_TCP) {
		if (skb->len >= skb_transport_offset(skb) + tcp_hdrlen(skb))
			return true;
	}
	return false;
}

static void fi_ipv6_5tuple_parse(struct fi_pkt_parse *pktinfo, struct sk_buff *skb,
				enum fi_dir dir)
{
	struct ipv6hdr *iph = NULL;
	int thoff = 0;
	int tproto;
	int ret;

#if LINUX_VERSION_CODE < KERNEL_VERSION(5, 4, 0)
	struct timeval ts;
	do_gettimeofday(&ts);
	pktinfo->msg.tstamp.tv_sec = ts.tv_sec;
	pktinfo->msg.tstamp.tv_usec = ts.tv_usec;
#else
	struct timespec64 ts;
	ktime_get_real_ts64(&ts);
	pktinfo->msg.tstamp.tv_sec = ts.tv_sec;
	pktinfo->msg.tstamp.tv_usec = ts.tv_nsec / 1000;
#endif
	pktinfo->msg.flow_msg.l3proto = ntohs(skb->protocol);
	pktinfo->msg.dir = dir;
	iph = ipv6_hdr(skb);
	ret = memcpy_s(&(pktinfo->msg.flow_msg.ipv6_sip), sizeof(pktinfo->msg.flow_msg.ipv6_sip),
		&(iph->saddr),
		sizeof(pktinfo->msg.flow_msg.ipv6_sip));
	if (ret) {
		fi_loge("memset_s failed ret=%d", ret);
		return;
	}

	ret = memcpy_s(&(pktinfo->msg.flow_msg.ipv6_dip), sizeof(pktinfo->msg.flow_msg.ipv6_dip),
		&(iph->daddr),
		sizeof(pktinfo->msg.flow_msg.ipv6_dip));
	if (ret) {
		fi_loge("memset_s failed ret=%d", ret);
		return;
	}
	tproto = ipv6_find_hdr(skb, &thoff, -1, NULL, NULL);
	fi_ip_save_5tuple_info(pktinfo, tproto, skb);
	return;
}

static void fi_ipv6_pkt_parse(struct fi_pkt_parse *pktinfo, struct sk_buff *skb)
{
	int i;

	spin_lock_bh(&(pktinfo->flow_cb.app_info->lock));
	if (pktinfo->msg.flow_msg.l4proto == IPPROTO_UDP)
		fi_payload_frag_parse(pktinfo, skb);

	for (i = 0; i < FI_END_PARSE; i++) {
		if ((pktinfo->flow_cb.flow_identify_cfg->pkt[pktinfo->dir].opt_enable[i] ||
			pktinfo->flow_cb.flow_identify_cfg->periodic[pktinfo->dir].opt_enable[i]) &&
			g_fi_pkt_parse_f[i].opt_val_parse != NULL) {
			fi_logd("parse target:%d function", i);
			(void)(g_fi_pkt_parse_f[i].opt_val_parse(skb, pktinfo));
		}
	}
	spin_unlock_bh(&(pktinfo->flow_cb.app_info->lock));
	return;
}

static bool fi_ipv6_pktinfo_update(struct sk_buff *skb, struct fi_pkt_parse *pktinfo, kuid_t *kuid, enum fi_dir dir)
{
	if (unlikely(skb == NULL))
		return false;

	if (!fi_ipv6_pkt_check(skb))
		return false;

	if (memset_s(pktinfo, sizeof(struct fi_pkt_parse), 0, sizeof(struct fi_pkt_parse))) {
		fi_loge("ipv6 hook memset_s failed");
		return false;
	}

	fi_ipv6_5tuple_parse(pktinfo, skb, dir);

	return fi_up_pkt_check(skb, pktinfo, kuid, dir);
}

static void fi_ipv6_hook(struct sk_buff *ipv6_skb, const struct nf_hook_state *state, enum fi_dir dir)
{
	kuid_t kuid = {0};
	struct fi_pkt_parse pktinfo;
	uint32_t hash;
	bool enable = false;
	struct fi_flow_head *head = NULL;
	struct fi_flow_node *flow = NULL;
	struct sk_buff *skb = ipv6_skb;
	int last_tcp_state = 0;

	if (!fi_ipv6_pktinfo_update(skb, &pktinfo, &kuid, dir))
		return;

	hash = fi_ipv6_flow_hash(&pktinfo, FI_FLOW_LEN + 1,
		((dir == FI_DIR_DOWN) ? true : false));
	head = fi_ipv6_flow_header(hash);
	fi_ipv6_flow_lock(hash);
	flow = fi_ipv6_flow_get(head, kuid.val, &pktinfo,
		((dir == FI_DIR_UP) ? true : false), fi_get_sock_by_dir(skb, dir));
	if (flow == NULL || fi_check_flow_already_closed(&last_tcp_state, flow, &pktinfo) ||
		fi_flow_qos_and_report_pre_check(&pktinfo, state, flow, skb, dir)) {
		fi_ipv6_flow_unlock(hash);
		return;
	}

	fi_set_pktinfo_from_flow(&pktinfo, flow, dir);
	fi_ipv6_pkt_parse(&pktinfo, skb);

	if (dir == FI_DIR_DOWN && flow->flow_ctx.key_info_check_rst == FI_RPT_NULL) {
		int http_rsp_code = fi_parse_http_rsp_code(skb, pktinfo.payload, pktinfo.parse_len);
		if (http_rsp_code > 0 && fi_is_blocked_rsp_code(http_rsp_code))
			flow->flow_ctx.key_info_check_rst = FI_RPT_BLOCK;
		else if (http_rsp_code > 0 || http_rsp_code == -1)
			flow->flow_ctx.key_info_check_rst = FI_RPT_GO;
	}

	fi_update_fault_type(dir, last_tcp_state, &pktinfo, &kuid);

	fi_flow_statistics(flow, &pktinfo, dir);
	enable = (fi_pkt_report_check(flow, &pktinfo, dir) && flow->flow_ctx.key_info_check_rst != FI_RPT_BLOCK) ||
		(pktinfo.msg.flow_msg.flow_fault_type > FI_FLOW_FAULT_NONE);
	if (enable) {
		int ret = strcpy_s(pktinfo.msg.flow_msg.dev, IFNAMSIZ, flow->flow_ctx.flow_msg.dev);
		if (ret)
			fi_loge("strcpy_s failed ret=%d, dev=%s", ret, flow->flow_ctx.flow_msg.dev);
	}
	fi_ipv6_flow_unlock(hash);

	if (enable)
		fi_pkt_msg_report(&pktinfo, FI_PKT_MSG_RPT);

	if (pktinfo.is_malloc_buf)
		kfree(pktinfo.payload);
}

static unsigned int fi_ipv6_hook_out(void *priv, struct sk_buff *skb,
	const struct nf_hook_state *state)
{
	fi_ipv6_hook(skb, state, FI_DIR_UP);
	return NF_ACCEPT;
}

static unsigned int fi_ipv6_hook_in(void *priv, struct sk_buff *skb,
	const struct nf_hook_state *state)
{
	fi_ipv6_hook(skb, state, FI_DIR_DOWN);
	return NF_ACCEPT;
}
#endif

void fi_init(struct sock *nlfd)
{
	int ret;
	int i;

	ret = memset_s(&g_fi_app_info, sizeof(g_fi_app_info), 0, sizeof(g_fi_app_info));
	if (ret) {
		fi_loge("memset_s failed ret=%d", ret);
		return;
	}

	for (i = 0; i < FI_LAUNCH_APP_MAX; i++)
		spin_lock_init(&(g_fi_app_info.app[i].lock));
	atomic_set(&(g_fi_app_info.free_cnt), FI_LAUNCH_APP_MAX);
	fi_logd("app_info init success");

	fi_ipv4_flow_init();
	fi_logd("IPV4 flow table init success");
#if IS_ENABLED(CONFIG_IPV6)
	fi_ipv6_flow_init();
	fi_logd("IPV6 flow table init success");
#endif

	ret = memset_s(&g_fi_ctx, sizeof(g_fi_ctx), 0, sizeof(g_fi_ctx));
	if (ret) {
		fi_loge("memset_s failed ret=%d", ret);
		return;
	}

	mutex_init(&g_fi_ctx.nf_mutex);
#if LINUX_VERSION_CODE < KERNEL_VERSION(4, 19, 0)
	init_timer(&(g_fi_ctx.tm));
	g_fi_ctx.tm.function = fi_timer;
#else
	timer_setup(&(g_fi_ctx.tm), fi_timer, 0);
#endif
	g_fi_ctx.is_tm_start = false;
	g_fi_ctx.nlfd = nlfd;
	g_fi_ctx.running = true;
	skb_queue_head_init(&g_fi_ctx.skb_queue);
	sema_init(&g_fi_ctx.netlink_sync_sema, 0);
	g_fi_ctx.netlink_task = kthread_run(fi_netlink_thread, NULL, "fi_netlink_thread");
	fi_logi("init success");
	return;
}
EXPORT_SYMBOL(fi_init); /*lint !e580*/

void fi_deinit(void)
{
	int ret;

	fi_unregister_nfhook();

	ret = memset_s(&g_fi_app_info, sizeof(g_fi_app_info), 0, sizeof(g_fi_app_info));
	if (ret) {
		fi_loge("memset_s failed ret=%d", ret);
		return;
	}
	fi_logd("app_info deinit success");

	fi_ipv4_flow_clear();
#if IS_ENABLED(CONFIG_IPV6)
	fi_ipv6_flow_clear();
#endif
	fi_logd("flow table deinit success");

	g_fi_ctx.running = false;
	if (g_fi_ctx.netlink_task != NULL) {
		kthread_stop(g_fi_ctx.netlink_task);
		g_fi_ctx.netlink_task = NULL;
	}
	fi_empty_netlink_skb_queue();

	fi_logi("deinit success");
	return;
}
EXPORT_SYMBOL(fi_deinit); /*lint !e580*/
/**
 * fi_mstamp_get - get current timestamp
 * @cl: place to store timestamps
 */
static void fi_mstamp_get(struct fi_mstamp *cl)
{
	u64 val = local_clock();
	do_div(val, NSEC_PER_USEC);
	cl->stamp_us = (u32)val;
	cl->stamp_jiffies = (u32)jiffies;
}

/**
 * fi_mstamp_delta - compute the difference in usec between two fi_mstamp
 * @t1: pointer to newest sample
 * @t0: pointer to oldest sample
 */
static u32 fi_mstamp_us_delta(struct fi_mstamp *t1, struct fi_mstamp *t0)
{
	s32 delta_us = t1->stamp_us - t0->stamp_us;
	u32 delta_jiffies = t1->stamp_jiffies - t0->stamp_jiffies;

	/* If delta_us is negative, this might be because interval is too big,
	 * or local_clock() drift is too big : fallback using jiffies.
	 */
	if ((delta_us <= 0) || (delta_jiffies >= (INT_MAX / (USEC_PER_SEC / HZ))))
		delta_us = jiffies_to_usecs(delta_jiffies);

	return delta_us;
}

void fi_bw_calc(struct fi_flow_node *flow)
{
	uint32_t delta_us;

	if ((flow->rcv_bw.first_rcv_time.stamp_jiffies == 0) && (flow->rcv_bw.first_rcv_time.stamp_us == 0))
		return;
	delta_us = fi_mstamp_us_delta(&(flow->rcv_bw.last_rcv_time), &(flow->rcv_bw.first_rcv_time));
	if (flow->rcv_bw.total_bytes == 0) {
		if (flow->rcv_bw.burst_bytes < FI_MIN_BURST_SIZE || delta_us * FI_TIMER_INTERVAL < USEC_PER_SEC)
			return;
	}
	if (flow->rcv_bw.burst_bytes >= FI_MIN_BURST_SIZE) {
		flow->rcv_bw.total_bytes += flow->rcv_bw.burst_bytes;
		flow->rcv_bw.total_time += delta_us;
	}
	if (flow->rcv_bw.total_time < FI_MIN_BURST_DURATION)
		flow->qos.bw_est = (uint64_t)flow->rcv_bw.total_bytes * USEC_PER_SEC / FI_MIN_BURST_DURATION;
	else
		flow->qos.bw_est = (uint64_t)flow->rcv_bw.total_bytes * USEC_PER_SEC / flow->rcv_bw.total_time;
	fi_logi("bw: uid %u, sport: %u, total_bytes: %u, total_time: %u, bw_est: %u",
			flow->uid, flow->sport, flow->rcv_bw.total_bytes, flow->rcv_bw.total_time, flow->qos.bw_est);
	flow->rcv_bw.total_bytes = 0;
	flow->rcv_bw.total_time = 0;
}

void fi_bw_update(struct fi_flow_node *flow)
{
	if (flow->rcv_bw.burst_bytes > FI_MIN_BURST_SIZE) {
		uint32_t delta_us = fi_mstamp_us_delta(&(flow->rcv_bw.last_rcv_time), &(flow->rcv_bw.first_rcv_time));
		fi_logd("burst: uid %u, sport: %u, burst_bytes: %u, delta_us: %u",
			flow->uid, flow->sport, flow->rcv_bw.burst_bytes, delta_us);
		delta_us = (delta_us > FI_MIN_BURST_DURATION) ? delta_us : FI_MIN_BURST_DURATION;
		flow->rcv_bw.total_bytes += flow->rcv_bw.burst_bytes;
		flow->rcv_bw.total_time += delta_us;
	}
}

void fi_rtt_update(struct fi_flow_node *flow)
{
	unsigned long now_jiffies = jiffies;
	u64 now_mstamp = tcp_clock_us();
	struct tcp_sock *tp = NULL;
	uint32_t srtt;
	uint32_t rcv_rtt;
	unsigned long srtt_time;
	u64 rcv_rtt_time;
	u32 delta_us;

	if ((flow->sk == NULL) || (flow->sk->sk_protocol != IPPROTO_TCP)
		|| (flow->sk->sk_state != TCP_ESTABLISHED && flow->sk->sk_state != TCP_SYN_SENT))
		return;

	tp = tcp_sk(flow->sk);
	srtt = tp->srtt_us >> FI_RTT_SHIFT;
	srtt_time = tp->rcv_tstamp;
	rcv_rtt = tp->rcv_rtt_est.rtt_us >> FI_RTT_SHIFT;
	rcv_rtt_time = tp->rcv_rtt_est.time;
	fi_logd("sport %u now %lu srtt_time %lu srtt %u now_mstamp %ju rcv_rtt_time %ju rcv_rtt %u",
			flow->sk->sk_num, now_jiffies, srtt_time, srtt, now_mstamp, rcv_rtt_time, rcv_rtt);
	fi_logd("updatetime %u tcp_mstamp %ju mdev_max_us %u rttvar_us %u total_retrans %u retrans_stamp %u",
			flow->updatetime, tp->tcp_mstamp, tp->mdev_us, tp->rttvar_us, tp->total_retrans, tp->retrans_stamp);
	/* SYN TIMEOUT */
	if (flow->sk->sk_state == TCP_SYN_SENT && tp->total_retrans > 0) {
		if (!strncmp(FI_CELL0_IFNAME, flow->flow_ctx.flow_msg.dev, strlen(flow->flow_ctx.flow_msg.dev)) ||
			!strncmp(FI_CELL1_IFNAME, flow->flow_ctx.flow_msg.dev, strlen(flow->flow_ctx.flow_msg.dev)))
			flow->qos.rtt = tp->total_retrans * USEC_PER_SEC;
		else
			flow->qos.rtt = 0;
		fi_logi("syn time out, set rtt = %u", flow->qos.rtt);
		return;
	}
	if (rcv_rtt == 0) {
		flow->qos.rtt = srtt;
		return;
	}
	delta_us = tcp_stamp_us_delta(now_mstamp, rcv_rtt_time);
	/*lint -e666*/
	if (time_before(now_jiffies, srtt_time + msecs_to_jiffies(MSEC_PER_SEC)) ||
			time_before(now_jiffies, srtt_time + usecs_to_jiffies(delta_us)) || srtt < tp->rcv_rtt_est.rtt_us)
		flow->qos.rtt = srtt;
	else
		flow->qos.rtt = rcv_rtt;
	/*lint +e666*/
}

void fi_qos_pre_update(const struct sk_buff *skb, struct fi_flow_node *flow)
{
	uint32_t max_delta;
	uint32_t delta_time_last;
	uint32_t delta_time_first;
	struct fi_mstamp now;
	fi_mstamp_get(&now);
	if (unlikely(flow->qos.rtt == 0))
		fi_rtt_update(flow);
	if (flow->sk == NULL || flow->sk->sk_protocol != IPPROTO_TCP)
		return;
	max_delta = flow->qos.rtt >> 1;
	delta_time_last = fi_mstamp_us_delta(&now, &(flow->rcv_bw.last_rcv_time));
	delta_time_first = fi_mstamp_us_delta(&now, &(flow->rcv_bw.first_rcv_time));
	fi_logd("dev %s, sport %u, jiffies %u us %u seq %u len %u", flow->flow_ctx.flow_msg.dev, flow->sk->sk_num,
		now.stamp_jiffies, now.stamp_us, TCP_SKB_CB(skb)->seq, skb->len);
	if (delta_time_last <= max_delta || delta_time_last <= FI_MIN_DELTA_US || delta_time_first <= flow->qos.rtt) {
		flow->rcv_bw.burst_bytes += skb->len;
	} else {
		fi_bw_update(flow);
		flow->rcv_bw.first_rcv_time = now;
		flow->rcv_bw.burst_bytes = 0;
	}
	flow->rcv_bw.last_rcv_time = now;
	flow->qos.rcv_bytes += skb->len;
}

void fi_filesize_pre_update(const struct sk_buff *skb, struct fi_flow_node *flow)
{
	uint32_t download_bytes;
	flow->flow_ctx.up_pkt_num++;
	if (skb->len <= skb_transport_offset(skb) + tcp_hdrlen(skb) ||
		(flow->sk == NULL || flow->sk->sk_protocol != IPPROTO_TCP)) {
		return;
	} else {
		struct tcp_sock *tp = tcp_sk(flow->sk);
		if (unlikely(flow->file_size.last_rcv_bytes == 0)) {
			flow->file_size.last_rcv_bytes = tp->bytes_received;
			return;
		}

		if (tp->bytes_received < flow->file_size.last_rcv_bytes)
			return;

		download_bytes = tp->bytes_received - flow->file_size.last_rcv_bytes;
		flow->file_size.last_rcv_bytes = tp->bytes_received;
		if (download_bytes > flow->file_size.size) {
			flow->file_size.size = download_bytes;
			fi_logd("sport %u filesize %u", flow->sk->sk_num, flow->file_size.size);
		}
	}
}

int fi_iface_msg_by_index(char *data, int index, uint32_t size)
{
	struct fi_iface_msg iface_msg;
	struct net_device *dev = NULL;
	struct rtnl_link_stats64 stats;
	char *iface = NULL;
	int ret;
	static int report_cnt = FI_IFACE_PRINT_LOG_INTERVAL;
	iface = emcom_xengine_get_network_iface_name(index);
	ret = strncpy_s(iface_msg.dev, IFNAMSIZ, iface, strlen(iface));
	if (ret != EOK) {
		fi_loge("strncpy_s failed ret: %d, iface: %s", ret, iface);
		return -1;
	}

	dev = dev_get_by_name(&init_net, iface);
	if (dev) {
		read_lock(&dev_base_lock);
		dev_get_stats(dev, &stats);
		read_unlock(&dev_base_lock);
		dev_put(dev);
		if (stats.rx_bytes > g_fi_iface_bytes[index])
			iface_msg.rcv_bytes = stats.rx_bytes - g_fi_iface_bytes[index];
		else
			iface_msg.rcv_bytes = 0;
		g_fi_iface_bytes[index] = stats.rx_bytes;
	} else {
		iface_msg.rcv_bytes = 0;
		/* clear g_fi_iface_bytes if dev is off */
		g_fi_iface_bytes[index] = 0;
		iface_msg.dev[0] = '\0';
	}

	if (dev && report_cnt == FI_IFACE_PRINT_LOG_INTERVAL)
		fi_logi("Periodic report. iface: %s, total_byes: %lu", iface, g_fi_iface_bytes[index]);

	ret = memcpy_s(data + index * sizeof(struct fi_iface_msg), size - index * sizeof(struct fi_iface_msg),
		&iface_msg, sizeof(struct fi_iface_msg));
	if (ret != EOK) {
		fi_loge("memcpy_s failed ret: %d, size: %u", ret, size);
		return -1;
	}

	if (dev && report_cnt == FI_IFACE_PRINT_LOG_INTERVAL)
		fi_logi("iface %s message rcv_bytes: %u", iface_msg.dev, iface_msg.rcv_bytes);

	report_cnt--;
	if (report_cnt <= 0)
		report_cnt = FI_IFACE_PRINT_LOG_INTERVAL;

	return 0;
}

void fi_iface_qos_update_report(void)
{
	int i;
	uint32_t size = sizeof(struct fi_iface_msg) * FI_DEV_NUM;
	char *data = NULL;
	struct sk_buff *pskb = NULL;

	if (!g_fi_ctx.iface_qos_rpt_enable)
		return;

	pskb = fi_get_netlink_skb(FI_IFACE_MSG_RPT, size, &data);
	if (pskb == NULL) {
		fi_loge("failed to malloc memory for data");
		return;
	}
	for (i = 0; i < FI_DEV_NUM; i++) {
		if (fi_iface_msg_by_index(data, i, size) != 0) {
			kfree_skb(pskb);
			return;
		}
	}
	fi_enqueue_netlink_skb(pskb);

	return;
}
