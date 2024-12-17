/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2020-2021. All rights reserved.
 * License terms: GNU General Public License (GPL)
 * Author: songqiubin
 *
 * MinTP layer2 Reliable Transmission Protocol.
 * Part of the code refers to the Linux tcpip stack
 */

#ifndef _MINTP_CONN_H
#define _MINTP_CONN_H

#include "mintp.h"
#include "net/snmp.h"

#define MTP_SYNCOOKIE_AGE	3
#define MTP_SYNCOOKIE_PERIOD	(10 * HZ)

static inline u32 mtp_cookie_time(void)
{
	u64 val = get_jiffies_64();

	do_div(val, MTP_SYNCOOKIE_PERIOD);
	return val;
}

void mtp_hashinfo_init(void);
void mtp_hashinfo_exit(void);
int mtp_get_port(struct sock *sk, unsigned short snum);
void mtp_put_port(struct sock *sk);
int mtp_lib_hash(struct sock *sk);
void mtp_lib_unhash(struct sock *sk);
void mtp_device_down(struct net_device *dev);
void mtp_set_keepalive(struct sock *sk, int val);
int dev_get_mac_by_inetaddr(__be32 ifa_local, unsigned char *mac);
int mtp_d2d_enable(struct mtp_sock *msk);
void mtp_d2d_disable(struct mtp_sock *msk);
void mtp_dev_update_bytes(void *ptr, int bytes);
struct sock *mtp_lookup_skb(const struct sk_buff *skb, u16 dport, u16 sport);
u32 mtp_create_syn_cookie(const struct ethhdr *eth, const struct mtphdr *mtph);
struct sock *mtp_get_cookie_sock(struct sock *sk, const struct sk_buff *skb);
struct net_device *dev_get_by_mac(unsigned char *addr);

#ifdef CONFIG_PROC_FS
int mtp_proc_init(void);
void mtp_proc_exit(void);
#endif /* CONFIG_PROC_FS */

/* MIB */
enum {
	MTP_MIB_NUM = 0,
	MTP_MIB_RTOALGORITHM, /* RtoAlgorithm */
	MTP_MIB_RTOMIN, /* RtoMin */
	MTP_MIB_RTOMAX, /* RtoMax */
	MTP_MIB_MAXCONN, /* MaxConn */
	MTP_MIB_ACTIVEOPENS, /* ActiveOpens */
	MTP_MIB_PASSIVEOPENS, /* PassiveOpens */
	MTP_MIB_ATTEMPTFAILS, /* AttemptFails */
	MTP_MIB_ESTABRESETS, /* EstabResets */
	MTP_MIB_CURRESTAB, /* CurrEstab */
	MTP_MIB_INSEGS, /* InSegs */
	MTP_MIB_OUTSEGS, /* OutSegs */
	MTP_MIB_RETRANSSEGS, /* RetransSegs */
	MTP_MIB_INERRS, /* InErrs */
	MTP_MIB_OUTRSTS, /* OutRsts */
	MTP_MIB_TLSCURRTXSW, /* TlsCurrTxSw */
	MTP_MIB_TLSCURRRXSW, /* TlsCurrRxSw */
	MTP_MIB_TLSTXSW, /* TlsTxSw */
	MTP_MIB_TLSRXSW, /* TlsRxSw */
	MTP_MIB_TLSDECRYPTERROR, /* TlsDecryptError */
	__MTP_MIB_MAX
};

#define MTP_MIB_MAX	__MTP_MIB_MAX
struct mtp_mib {
	unsigned long	mibs[MTP_MIB_MAX];
};

extern struct mtp_mib __percpu *mtp_statistics;

#define MTP_ADD_STATS(field, val) SNMP_ADD_STATS(mtp_statistics, field, val)
#define MTP_INC_STATS(field) SNMP_INC_STATS(mtp_statistics, field)
#define __MTP_INC_STATS(field) __SNMP_INC_STATS(mtp_statistics, field)
#define MTP_DEC_STATS(field) SNMP_DEC_STATS(mtp_statistics, field)

int mtp_mib_init(void);
void mtp_mib_exit(void);

#endif /* _MINTP_CONN_H */
