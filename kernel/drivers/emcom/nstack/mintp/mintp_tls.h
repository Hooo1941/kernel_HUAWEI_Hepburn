/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2021-2023. All rights reserved.
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * Author: liyouyong songqiubin
 *
 * mtp layer2 Reliable Transmission Protocol.
 */

#ifndef __MINTP_TLS_H__
#define __MINTP_TLS_H__

#include <crypto/aead.h>
#include <net/inet_common.h>
#include <net/inet_sock.h>
#include <linux/tls.h>

/* Maximum data size carried in a TLS record */
#define TLS_MAX_PAYLOAD_SIZE		((size_t)1 << 14)

#define TLS_HEADER_SIZE			5
#define TLS_NONCE_OFFSET		TLS_HEADER_SIZE
#define TLS_RECORD_TYPE_DATA		0x17
#define TLS_AAD_SPACE_SIZE		13
#define MAX_IV_SIZE			16
#define TLS_SG_MAX			(MAX_SKB_FRAGS + 2)

struct mtp_tls_rec {
	struct sk_buff_head skb_queue;
	struct scatterlist sg_pl[TLS_SG_MAX];
	struct scatterlist sg_en[TLS_SG_MAX];
	s16 sg_pl_frag_used;
	s16 sg_pl_frag_iter;
	int sg_pl_len;
	/* AAD | msg_plaintext.sg.data | sg_tag */
	struct scatterlist sg_aead_in[2];
	/* AAD | msg_encrypted.sg.data (data contains overhead for hdr & iv & tag) */
	struct scatterlist sg_aead_out[2];

	char aad_space[TLS_AAD_SPACE_SIZE];
	u16 user_data_offset;
	u16 user_data_len;
	bool last_frag;
	bool error;

	struct aead_request aead_req;
#ifdef ANDROID_KABI_RESERVE
	ANDROID_KABI_RESERVE(1);
#endif
	u8 aead_req_ctx[];
};

struct mtp_cipher_context {
	char *iv;
	char *rec_seq;
};

struct mtp_tls_info {
	u16 version;
	u16 cipher_type;
	u16 prepend_size;
	u16 tag_size;
	u16 overhead_size;
	u16 iv_size;
	u16 salt_size;
	u16 rec_seq_size;
	u16 aad_size;
	u16 tail_size;
	u16 record_size; /* To avoid record spilts, calculate the most appropriate TLS record size based on mss. */
};

struct mtp_tls_aead {
	struct crypto_aead *aead;
	struct crypto_wait async_wait;
	struct mtp_tls_rec *open_rec;
	struct mtp_cipher_context cipher;
	struct tls12_crypto_info_aes_gcm_128 crypto;
};

struct mtp_tls_context {
	struct mtp_tls_info prot_info;
	struct mtp_tls_aead *tx;
	struct mtp_tls_aead *rx;
};

struct mtp_tls_cb {
	int len;
	int offset;
};

static inline struct mtp_tls_cb *skb_tls_cb(struct sk_buff *skb)
{
	return (struct mtp_tls_cb *)((void *)skb->cb + sizeof(struct mtp_skb_cb));
}

extern int mtp_setsockopt_tls(struct sock *sk, int level, int optname, sockptr_t optval,
		   unsigned int optlen);
extern int mtp_getsockopt_tls(struct sock *sk, int optname, char __user *optval, int __user *optlen);
extern int mtp_tls_sendpage_locked(struct sock *sk, struct page *page, int offset, size_t size, int flags);
extern void mtp_tls_close(struct sock *sk);
extern ssize_t mtp_tls_splice_read(struct sock *sk, struct pipe_inode_info *pipe, size_t len, u16 *copied_skb);
void mtp_tls_send_pending_page(struct sock *sk);

#endif /* __MINTP_TLS_H__ */
