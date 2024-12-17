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
 * Author: songqiubin
 *
 * mtp layer2 Reliable Transmission Protocol.
 */
#include "mintp.h"
#include "mintp_output.h"
#include "mintp_conn.h"
#include "mintp_tls.h"

static int mtp_tls_segment(struct sock *sk, struct msghdr *msg, long timeo,
	int *seg, bool *process_backlog);
static int mtp_tls_recv_queue(struct sock *sk, struct msghdr *msg, size_t len, u16 *copied_skb);

#ifdef CONFIG_HW_NSTACK_MINTP_MODULE
/* same as put_page(), huawei has modify the put_page() and made it can't be linked by a kernel module */
static void mtp_put_page(struct page *page)
{
	page = compound_head(page);

	/*
	 * For devmap managed pages we need to catch refcount transition from
	 * 2 to 1, when refcount reach one it means the page is free and we
	 * need to inform the device driver through callback. See
	 * include/linux/memremap.h and HMM for details.
	 */
	if (page_is_devmap_managed(page)) {
		put_devmap_managed_page(page);
		return;
	}

	if (put_page_testzero(page))
		__put_page(page);
}
#else
#define mtp_put_page put_page
#endif

static void mtp_encrypt_free_pl_pages(struct mtp_tls_rec *rec)
{
	int i;
	struct scatterlist *sg;

	for_each_sg(sg_next(rec->sg_aead_in), sg, USHRT_MAX, i) {
		if (!sg)
			break;
		mtp_put_page(sg_page(sg));
	}
}

static void mtp_sg_put_page(struct scatterlist *sg, int num)
{
	int i;
	for (i = 0; i < num; i++)
		mtp_put_page(sg_page(&sg[i]));
}

static int mtp_tls_init_rec(struct mtp_tls_info *prot, struct mtp_tls_aead *tls_aead)
{
	struct mtp_tls_rec *rec;
	int mem_size;

	mem_size = sizeof(struct mtp_tls_rec) + crypto_aead_reqsize(tls_aead->aead);

	rec = kzalloc(mem_size, GFP_KERNEL);
	if (!rec)
		return -ENOMEM;

	skb_queue_head_init(&rec->skb_queue);
	sg_init_table(rec->sg_pl, TLS_SG_MAX);
	sg_init_table(rec->sg_en, TLS_SG_MAX);

	sg_init_table(rec->sg_aead_in, 2);
	sg_set_buf(&rec->sg_aead_in[0], rec->aad_space, prot->aad_size);
	sg_unmark_end(&rec->sg_aead_in[1]);

	sg_init_table(rec->sg_aead_out, 2);
	sg_set_buf(&rec->sg_aead_out[0], rec->aad_space, prot->aad_size);
	sg_unmark_end(&rec->sg_aead_out[1]);

	rec->user_data_len = 0;
	rec->user_data_offset = 0;
	tls_aead->open_rec = rec;
	return 0;
}

static void mtp_tls_set_prot(struct mtp_tls_info *prot, struct tls_crypto_info *crypto_info, u16 mss)
{
	prot->aad_size = TLS_AAD_SPACE_SIZE;
	prot->tail_size = 0;
	prot->version = crypto_info->version;
	prot->cipher_type = crypto_info->cipher_type;
	prot->prepend_size = TLS_HEADER_SIZE + TLS_CIPHER_AES_GCM_128_IV_SIZE;
	prot->tag_size = TLS_CIPHER_AES_GCM_128_TAG_SIZE;
	prot->overhead_size = prot->prepend_size + prot->tag_size + prot->tail_size;
	prot->record_size = rounddown((TLS_MAX_PAYLOAD_SIZE + prot->overhead_size), mss);
	if (prot->record_size > (mss * MAX_SKB_FRAGS))
		prot->record_size = mss * MAX_SKB_FRAGS;
	prot->iv_size = TLS_CIPHER_AES_GCM_128_IV_SIZE;
	prot->salt_size = TLS_CIPHER_AES_GCM_128_SALT_SIZE;
	prot->rec_seq_size = TLS_CIPHER_AES_GCM_128_REC_SEQ_SIZE;
}

static int mtp_tls_set_sw_offload(struct sock *sk, struct mtp_tls_context *ctx, struct mtp_tls_aead *tls_aead)
{
	struct mtp_tls_info *prot = &ctx->prot_info;
	struct tls_crypto_info *crypto_info;
	struct mtp_cipher_context *cctx = &tls_aead->cipher;
	int rc;

	crypto_init_wait(&tls_aead->async_wait);
	crypto_info = &tls_aead->crypto.info;
	mtp_tls_set_prot(prot, crypto_info, mtp_sk(sk)->mss);
	cctx->iv = kmalloc(prot->iv_size + prot->salt_size, GFP_KERNEL);
	if (!cctx->iv) {
		rc = -ENOMEM;
		goto out;
	}

	(void)memcpy_s(cctx->iv, prot->salt_size, tls_aead->crypto.salt, prot->salt_size);
	(void)memcpy_s(cctx->iv + prot->salt_size, prot->iv_size, tls_aead->crypto.iv, prot->iv_size);
	cctx->rec_seq = kmemdup(tls_aead->crypto.rec_seq, prot->rec_seq_size, GFP_KERNEL);
	if (!cctx->rec_seq) {
		rc = -ENOMEM;
		goto free_iv;
	}

	tls_aead->aead = crypto_alloc_aead("gcm(aes)", 0, 0);
	if (IS_ERR(tls_aead->aead)) {
		rc = PTR_ERR(tls_aead->aead);
		tls_aead->aead = NULL;
		goto free_rec_seq;
	}

	rc = crypto_aead_setkey(tls_aead->aead, tls_aead->crypto.key, TLS_CIPHER_AES_GCM_128_KEY_SIZE);
	if (rc)
		goto free_aead;

	rc = crypto_aead_setauthsize(tls_aead->aead, prot->tag_size);
	if (rc)
		goto free_aead;

	rc = mtp_tls_init_rec(prot, tls_aead);
	if (rc)
		goto free_aead;
	goto out;

free_aead:
	crypto_free_aead(tls_aead->aead);
	tls_aead->aead = NULL;
free_rec_seq:
	kfree(cctx->rec_seq);
	cctx->rec_seq = NULL;
free_iv:
	kfree(cctx->iv);
	cctx->iv = NULL;
out:
	return rc;
}

static inline void mtp_tls_ctx_free(struct mtp_sock *msk)
{
	kfree(msk->tls_ctx);
	msk->tls_ctx = NULL;
}

static void mtp_tls_enable(struct mtp_sock *msk, struct mtp_tls_aead *tls_aead, int tx)
{
	if (tx) {
		msk->segment = mtp_tls_segment;
		msk->tls_ctx->tx = tls_aead;
		MTP_INC_STATS(MTP_MIB_TLSCURRTXSW);
		MTP_INC_STATS(MTP_MIB_TLSTXSW);
	} else {
		msk->recv_queue = mtp_tls_recv_queue;
		msk->tls_ctx->rx = tls_aead;
		MTP_INC_STATS(MTP_MIB_TLSCURRRXSW);
		MTP_INC_STATS(MTP_MIB_TLSRXSW);
	}
}

static int mtp_setsockopt_tls_conf(struct sock *sk, sockptr_t optval, unsigned int optlen, int tx)
{
	struct tls12_crypto_info_aes_gcm_128 *crypto;
	struct mtp_tls_aead *tls_aead;
	struct mtp_sock *msk = mtp_sk(sk);
	int rc = 0;
	bool tls_ctx_alloc = false;

	if (sockptr_is_null(optval) || (optlen < sizeof(*crypto)))
		return -EINVAL;

	if (!msk->tls_ctx) {
		msk->tls_ctx = kzalloc(sizeof(*msk->tls_ctx), GFP_ATOMIC);
		if (!msk->tls_ctx)
			return -ENOMEM;
		tls_ctx_alloc = true;
	} else if ((tx && msk->tls_ctx->tx) || (!tx && msk->tls_ctx->rx)) {
		/* Currently we don't support set crypto info more than one time */
		return -EBUSY;
	}

	tls_aead = kzalloc(sizeof(*tls_aead), GFP_ATOMIC);
	if (!tls_aead) {
		rc = -ENOMEM;
		goto err_ctx;
	}
	crypto = &tls_aead->crypto;
	rc = copy_from_sockptr(crypto, optval, sizeof(*crypto));
	if (rc) {
		rc = -EFAULT;
		goto err_crypto_info;
	}

	/* check version, Currently we only support tls1.2 */
	if (crypto->info.version != TLS_1_2_VERSION) {
		rc = -EINVAL;
		goto err_crypto_info;
	}

	rc = mtp_tls_set_sw_offload(sk, msk->tls_ctx, tls_aead);
	mtp_info("mtp_tls_set_sw_offload tx %d ret %d\n", tx, rc);
	if (!rc) {
		mtp_tls_enable(msk, tls_aead, tx);
		return 0;
	}
err_crypto_info:
	memzero_explicit(tls_aead, sizeof(*tls_aead));
	kfree(tls_aead);
err_ctx:
	if (tls_ctx_alloc)
		mtp_tls_ctx_free(msk);
	return rc;
}

int mtp_setsockopt_tls(struct sock *sk, int level, int optname, sockptr_t optval, unsigned int optlen)
{
	int rc = 0;
	int tx = (optname == TLS_TX);

	if (!!tx && sk->sk_state != TCP_ESTABLISHED)
		return -ENOTCONN;

	switch (optname) {
	case TLS_TX:
	case TLS_RX:
		lock_sock(sk);
		rc = mtp_setsockopt_tls_conf(sk, optval, optlen, tx);
		release_sock(sk);
		break;
	default:
		rc = -ENOPROTOOPT;
		break;
	}
	return rc;
}

static int mtp_getsockopt_tls_conf(struct sock *sk, char __user *optval,
				   int __user *optlen, int tx)
{
	int rc = 0;
	struct mtp_sock *msk = mtp_sk(sk);
	struct mtp_tls_context *ctx = msk->tls_ctx;
	struct mtp_tls_aead *tls_aead;
	struct mtp_cipher_context *cctx;
	int len;
	struct tls12_crypto_info_aes_gcm_128 *aes_gcm_128;

	if (get_user(len, optlen))
		return -EFAULT;

	if (!optval || (len < sizeof(struct tls_crypto_info))) {
		rc = -EINVAL;
		goto out;
	}

	if (!ctx) {
		rc = -EBUSY;
		goto out;
	}

	/* get user tls aead ctx */
	tls_aead = tx ? ctx->tx : ctx->rx;
	if (!tls_aead) {
		rc = -EBUSY;
		goto out;
	}

	cctx = &tls_aead->cipher;
	aes_gcm_128 = &tls_aead->crypto;
	lock_sock(sk);
	(void)memcpy_s(aes_gcm_128->iv, TLS_CIPHER_AES_GCM_128_IV_SIZE, cctx->iv + TLS_CIPHER_AES_GCM_128_SALT_SIZE,
		TLS_CIPHER_AES_GCM_128_IV_SIZE);
	(void)memcpy_s(aes_gcm_128->rec_seq, TLS_CIPHER_AES_GCM_128_REC_SEQ_SIZE, cctx->rec_seq,
		TLS_CIPHER_AES_GCM_128_REC_SEQ_SIZE);
	release_sock(sk);
	if (copy_to_user(optval, aes_gcm_128, sizeof(*aes_gcm_128)))
		rc = -EFAULT;

out:
	return rc;
}

int mtp_getsockopt_tls(struct sock *sk, int optname, char __user *optval, int __user *optlen)
{
	int rc = 0;

	switch (optname) {
	case TLS_TX:
	case TLS_RX:
		rc = mtp_getsockopt_tls_conf(sk, optval, optlen, optname == TLS_TX);
		break;
	default:
		rc = -ENOPROTOOPT;
		break;
	}
	return rc;
}

static void mtp_tls_close_aead(struct mtp_tls_aead *tls_aead)
{
	kfree(tls_aead->cipher.iv);
	kfree(tls_aead->cipher.rec_seq);
	crypto_free_aead(tls_aead->aead);
	if (tls_aead->open_rec) {
		__skb_queue_purge(&tls_aead->open_rec->skb_queue);
		kfree(tls_aead->open_rec);
	}
	memzero_explicit(tls_aead, sizeof(*tls_aead));
	kfree(tls_aead);
}

void mtp_tls_close(struct sock *sk)
{
	struct mtp_sock *msk = mtp_sk(sk);
	struct mtp_tls_aead *tx = msk->tls_ctx->tx;
	struct mtp_tls_aead *rx = msk->tls_ctx->rx;

	mtp_info("enter\n");
	if (tx) {
		mtp_tls_send_pending_page(sk);
		mtp_tls_close_aead(tx);
		MTP_DEC_STATS(MTP_MIB_TLSCURRTXSW);
	}
	if (rx) {
		mtp_tls_close_aead(rx);
		MTP_DEC_STATS(MTP_MIB_TLSCURRRXSW);
	}

	memzero_explicit(msk->tls_ctx, sizeof(*msk->tls_ctx));
	mtp_tls_ctx_free(msk);
	msk->tls_recv_pend = false;
	sk->sk_shutdown = SHUTDOWN_MASK;
}

static inline void mtp_tls_make_aad(char *buf, size_t size, char *record_sequence)
{
	int ret = memcpy_s(buf, TLS_AAD_SPACE_SIZE, record_sequence,
		TLS_CIPHER_AES_GCM_128_REC_SEQ_SIZE);
	if (unlikely(ret != EOK))
		mtp_err("memcpy_s ret %d\n", ret);
	buf += TLS_CIPHER_AES_GCM_128_REC_SEQ_SIZE;

	buf[0] = TLS_RECORD_TYPE_DATA;
	buf[1] = TLS_1_2_VERSION_MAJOR;
	buf[2] = TLS_1_2_VERSION_MINOR;
	buf[3] = size >> 8;
	buf[4] = size & 0xFF;
}

static inline void mtp_tls_fill_prepend(char *buf, size_t plaintext_len, u8 *iv)
{
	size_t pkt_len = plaintext_len + TLS_CIPHER_AES_GCM_128_TAG_SIZE + TLS_CIPHER_AES_GCM_128_IV_SIZE;
	int ret = memcpy_s(buf + TLS_NONCE_OFFSET, TLS_CIPHER_AES_GCM_128_IV_SIZE,
		iv + TLS_CIPHER_AES_GCM_128_SALT_SIZE, TLS_CIPHER_AES_GCM_128_IV_SIZE);
	if (unlikely(ret != EOK))
		mtp_err("memcpy_s ret %d\n", ret);
	/* we cover nonce explicit here as well, so buf should be of
	 * size KTLS_DTLS_HEADER_SIZE + KTLS_DTLS_NONCE_EXPLICIT_SIZE
	 */
	buf[0] = TLS_RECORD_TYPE_DATA;
	/* Note that VERSION must be TLS_1_2 for both TLS1.2 and TLS1.3 */
	buf[1] = TLS_1_2_VERSION_MINOR;
	buf[2] = TLS_1_2_VERSION_MAJOR;
	/* we can use IV for nonce explicit according to spec */
	buf[3] = pkt_len >> 8;
	buf[4] = pkt_len & 0xFF;
}

static int mtp_sg_from_iter(struct iov_iter *from, struct scatterlist *sg, u32 bytes, s16 *sg_cnt)
{
	int i, maxpages;
	int ret = 0;
	int num_elems = 0;
	s16 sg_idx = 0;
	const int to_max_pages = MAX_SKB_FRAGS;
	struct page *pages[MAX_SKB_FRAGS];
	ssize_t copied, tot_copied, use, offset;

	tot_copied = 0;
	while (bytes > 0) {
		i = 0;
		maxpages = to_max_pages - num_elems;
		if (maxpages <= 0) {
			mtp_err("iov_iter_get_pages maxpages zero, return EFAULT\n");
			ret = -EFAULT;
			goto out;
		}
		copied = iov_iter_get_pages(from, pages, bytes, maxpages, &offset);
		if (copied <= 0) {
			mtp_debug("iov_iter_get_pages failed, return EFAULT\n");
			ret = -EFAULT;
			goto out;
		}

		iov_iter_advance(from, copied);
		tot_copied += copied;
		bytes -= copied;

		while (copied) {
			use = min_t(int, copied, PAGE_SIZE - offset);
			mtp_debug("sg_idx %d use %d offset %zd i %d\n", sg_idx, use, offset, i);
			sg_set_page(&sg[sg_idx], pages[i], use, offset);
			sg_unmark_end(&sg[sg_idx]);

			offset = 0;
			copied -= use;
			sg_idx++;
			num_elems++;
			i++;
		}
	}
	sg_mark_end(&sg[sg_idx - 1]);
	if (sg_cnt)
		*sg_cnt = sg_idx;
out:
	/* Revert iov_iter updates, msg will need to use 'trim' later if it
	 * also needs to be cleared.
	 */
	if (ret) {
		mtp_sg_put_page(sg, sg_idx);
		iov_iter_revert(from, tot_copied);
	}
	return ret;
}

static int mtp_sg_from_iter_skb_copy(struct iov_iter *from, struct mtp_tls_rec *rec, u32 bytes, u16 prepend_size)
{
	int ret = 0;
	s16 sg_idx = 0;
	ssize_t tot_copied = 0;
	struct sk_buff *skb;
	skb_queue_walk(&rec->skb_queue, skb) {
		ssize_t to_copy = sg_idx != 0 ? skb->len : skb->len - prepend_size;
		ssize_t offset = sg_idx != 0 ? 0 : prepend_size;

		to_copy = min_t(int, to_copy, bytes);
		mtp_debug("sg_idx %d skb->len %d to_copy %d offset %d tot_copied %d bytes %u\n",
					sg_idx, skb->len, to_copy, offset, tot_copied, bytes);
		if (skb_copy_datagram_from_iter(skb, offset, from, to_copy) != 0) {
			mtp_err("skb_copy_datagram_from_iter failed, return EFAULT\n");
			ret = -EFAULT;
			goto out;
		}
		if (sg_idx) {
			sg_set_buf(&rec->sg_pl[sg_idx], skb->data, skb->len);
		} else {
			sg_set_buf(&rec->sg_pl[sg_idx], skb->data + prepend_size,
				skb->len - prepend_size);
		}
		sg_unmark_end(&rec->sg_pl[sg_idx]);
		sg_idx++;
		mtp_debug("to_copy %d\n", to_copy);
		tot_copied += to_copy;
		bytes -= to_copy;
		if (bytes <= 0) {
			mtp_debug("bytes %d\n", bytes);
			break;
		}
	}
	sg_mark_end(&rec->sg_pl[sg_idx - 1]);
	rec->sg_pl_frag_used = sg_idx;
	rec->sg_pl_frag_iter = 0;
out:
	/* Revert iov_iter updates, msg will need to use 'trim' later if it
	 * also needs to be cleared.
	 */
	if (ret)
		iov_iter_revert(from, tot_copied);
	return ret;
}

static inline void mtp_tls_err_abort(struct sock *sk, int err)
{
	sk->sk_err = err;
	sk->sk_error_report(sk);
}

static inline void mtp_tls_err_recv(struct sock *sk, struct mtp_tls_rec *rec)
{
	rec->error = true;
	__skb_queue_purge(&rec->skb_queue);
	rec->user_data_offset = 0;
	rec->user_data_len = 0;
	mtp_tls_err_abort(sk, EBADMSG);
}

static inline void mtp_tls_bigint_increment(unsigned char *seq, int len)
{
	int i;

	for (i = len - 1; i >= 0; i--) {
		++seq[i];
		if (seq[i] != 0)
			break;
	}
}

static inline void mtp_tls_advance_record_sn(struct mtp_tls_info *prot, struct mtp_cipher_context *ctx)
{
	mtp_tls_bigint_increment(ctx->rec_seq, prot->rec_seq_size);
	mtp_tls_bigint_increment(ctx->iv + prot->salt_size, prot->iv_size);
}

static int mtp_send_skb_alloc(struct sock *sk, struct mtp_tls_rec *rec, struct mtp_tls_info *prot_info,
	int required_size, char **prepend_buf)
{
	int i = 0;
	struct mtp_sock *msk = mtp_sk(sk);
	unsigned int mss = msk->mss;
	struct sk_buff *skb;

	while (required_size) {
		int skb_size = required_size;
		if (skb_size > mss)
			skb_size = mss;
		/* get a new skb */
		skb = msk_walloc_skb(sk, skb_size, sk->sk_allocation);
		if (skb == NULL) {
			mtp_info("msk_walloc_skb failed\n");
			sg_init_table(rec->sg_en, TLS_SG_MAX);
			__skb_queue_purge(&rec->skb_queue);
			return -ENOMEM;
		}

		__skb_queue_tail(&rec->skb_queue, skb);
		required_size -= skb_size;
		sg_unmark_end(&rec->sg_en[i]);
		skb_put(skb, skb_size);

		if (i) {
			sg_set_buf(&rec->sg_en[i], skb->data, skb->len);
		} else {
			*prepend_buf = skb->data;
			sg_set_buf(&rec->sg_en[i], skb->data + prot_info->prepend_size,
				skb->len - prot_info->prepend_size);
		}

		mtp_debug("i %d: skb_size %d len %d\n", i, skb_size, skb->len);
		if (!required_size)
			sg_mark_end(&rec->sg_en[i]);
		i++;
	}

	return 0;
}

static inline void mtp_tls_fill_sg(struct mtp_tls_rec *rec, struct mtp_tls_aead *aead, int user_size, bool is_enrypt)
{
	mtp_tls_make_aad(rec->aad_space, user_size, aead->cipher.rec_seq);
	sg_unmark_end(&rec->sg_aead_in[1]);
	sg_chain(rec->sg_aead_in, 2, is_enrypt ? rec->sg_pl : rec->sg_en);
	sg_unmark_end(&rec->sg_aead_out[1]);
	sg_chain(rec->sg_aead_out, 2, is_enrypt ? rec->sg_en : rec->sg_pl);
}

static int mtp_encrypt_prepare(struct sock *sk, struct msghdr *msg, struct mtp_tls_aead *tx,
	struct mtp_tls_rec *rec, struct mtp_tls_info *prot_info)
{
	int err;
	int copy = msg_data_left(msg);
	int required_size;
	char *prepend_buf;

	if (copy > (prot_info->record_size - prot_info->overhead_size))
		copy = prot_info->record_size - prot_info->overhead_size;
	required_size = copy + prot_info->overhead_size;
	err = mtp_send_skb_alloc(sk, rec, prot_info, required_size, &prepend_buf);
	if (err)
		return err;

	err = mtp_sg_from_iter(&msg->msg_iter, rec->sg_pl, copy, &rec->sg_pl_frag_iter);
	if (err) {
		int skb_copy_err = mtp_sg_from_iter_skb_copy(&msg->msg_iter, rec, copy, prot_info->prepend_size);
		mtp_debug("mtp_sg_from_iter err %d\n", err);
		if (skb_copy_err) {
			mtp_err("skb_copy_err %d\n", skb_copy_err);
			sg_init_table(rec->sg_en, TLS_SG_MAX);
			__skb_queue_purge(&rec->skb_queue);
			return skb_copy_err;
		}
	} else {
		rec->sg_pl_frag_used = rec->sg_pl_frag_iter;
	}

	rec->sg_pl_len = copy;
	mtp_tls_fill_prepend(prepend_buf, copy, tx->cipher.iv);
	return copy;
}

static inline void mtp_rec_pl_sg_init(struct mtp_tls_rec *rec)
{
	sg_unmark_end(&rec->sg_pl[rec->sg_pl_frag_used - 1]);
	rec->sg_pl_frag_used = 0;
	rec->sg_pl_frag_iter = 0;
	rec->sg_pl_len = 0;
}

static int mtp_encrypt(struct mtp_tls_aead *tx, struct mtp_tls_rec *rec, struct mtp_tls_info *prot_info, int copy)
{
	int err;

	mtp_tls_fill_sg(rec, tx, copy, true);
	aead_request_set_tfm(&rec->aead_req, tx->aead);
	aead_request_set_ad(&rec->aead_req, prot_info->aad_size);
	aead_request_set_crypt(&rec->aead_req, rec->sg_aead_in, rec->sg_aead_out,
			       copy, tx->cipher.iv);
	aead_request_set_callback(&rec->aead_req, CRYPTO_TFM_REQ_MAY_BACKLOG, crypto_req_done, &tx->async_wait);
	err = crypto_aead_encrypt(&rec->aead_req);
	if (err < 0) {
		if (err != -EINPROGRESS) {
			mtp_err("crypto_aead_encrypt fail err %d\n", err);
			return err;
		}
		mtp_info("encrypt in process\n");
		err = crypto_wait_req(err, &tx->async_wait);
		if (err) {
			mtp_err("crypto_wait_req err: %d\n", err);
			return err;
		}
	}
	mtp_debug("encrypt succ, sg_pl_frag_used %d sg_pl_len %d\n", rec->sg_pl_frag_used, rec->sg_pl_len);
	mtp_tls_advance_record_sn(prot_info, &tx->cipher);
	return 0;
}

static void mtp_push_record(struct sock *sk, struct mtp_tls_rec *rec, int *seg, bool msg_end)
{
	struct mtp_sock *msk = mtp_sk(sk);
	bool xmit_once = !mtp_send_head(sk);
	struct sk_buff *skb;

	while ((skb = __skb_dequeue(&rec->skb_queue)) != NULL) {
		struct mtp_skb_cb *dcb;
		skb->ip_summed = CHECKSUM_UNNECESSARY;
		mtp_skb_entail(sk, skb);
		*seg += 1;
		if ((*seg % MTP_SEG_BURST) == 0)
			xmit_once = true;

		msk->write_seq += skb->len;
		dcb = MTP_SKB_CB(skb);
		dcb->end_seq += skb->len;
		msk->cur_snd_msg_size += skb->len;

		if (skb_queue_empty(&rec->skb_queue) && msg_end) {
			msk->stats.out_msgs++;
			dcb->type = MTPHDR_TYPE_MSG_END;
			msk->cur_snd_msg_size = 0;
			/* will call mtp_write_xmit in the end of sendmsg */
			xmit_once = true;
		}
	}
	if (xmit_once)
		mtp_write_xmit(sk, 0, sk->sk_allocation);
}

static int mtp_tls_segment(struct sock *sk, struct msghdr *msg, long timeo,
			int *seg, bool *process_backlog)
{
	struct mtp_sock *msk = mtp_sk(sk);
	int copy;
	int err;
	struct mtp_tls_aead *tx = msk->tls_ctx->tx;
	struct mtp_tls_rec *rec = tx->open_rec;
	struct mtp_tls_info *prot_info = &msk->tls_ctx->prot_info;

	(void)timeo;
	if (*process_backlog && sk_flush_backlog(sk)) {
		*process_backlog = false;
		return -ERESTART;
	}

	/* may be pending page unsend */
	if (unlikely(rec->sg_pl_len))
		mtp_tls_send_pending_page(sk);

	copy = mtp_encrypt_prepare(sk, msg, tx, rec, prot_info);
	if (copy <= 0)
		return copy;

	err = mtp_encrypt(tx, rec, prot_info, copy);
	if (err) {
		mtp_sg_put_page(rec->sg_pl, rec->sg_pl_frag_iter);
		mtp_rec_pl_sg_init(rec);
		iov_iter_revert(&msg->msg_iter, copy);
		__skb_queue_purge(&rec->skb_queue);
		return err;
	}
	mtp_sg_put_page(rec->sg_pl, rec->sg_pl_frag_iter);
	mtp_rec_pl_sg_init(rec);
	mtp_push_record(sk, rec, seg, !msg_data_left(msg));
	*process_backlog = true;
	return copy;
}

static u16 mtp_tls_prepend_buf_parse(struct mtp_tls_aead *rx, struct mtp_tls_info *prot_info,
	unsigned char *buf, u16 len)
{
	u16 record_size;
	int ret;

	if (len < prot_info->prepend_size) {
		mtp_err("len %u small than prepend_size %u\n", len, prot_info->prepend_size);
		return 0;
	}

	if (buf[0] != TLS_RECORD_TYPE_DATA) {
		mtp_err("unsupported record type %u\n", buf[0]);
		return 0;
	}

	if (buf[1] != TLS_1_2_VERSION_MINOR || buf[2] != TLS_1_2_VERSION_MAJOR) {
		mtp_err("unsupported version %x:%x\n", buf[2], buf[1]);
		return 0;
	}
	record_size = (((u16)buf[3]) << 8) | buf[4];
	if (record_size <= (prot_info->tag_size + prot_info->iv_size) ||
		record_size > (prot_info->record_size - TLS_NONCE_OFFSET)) {
		mtp_err("record_size error %u(%u)\n", record_size, prot_info->record_size);
		return 0;
	}
	mtp_debug("get record_size %u\n", record_size);
	ret = memcpy_s(rx->cipher.iv + prot_info->salt_size, prot_info->iv_size,
		buf + TLS_NONCE_OFFSET, prot_info->iv_size);
	if (unlikely(ret != EOK))
		mtp_err("memcpy_s ret %d\n", ret);
	return record_size;
}

static int mtp_tls_recv_offset_error(struct mtp_sock *msk, struct sk_buff *skb, u32 offset)
{
	if (MTP_SKB_CB(skb)->type == MTPHDR_TYPE_FIN)
		return -EPIPE;
	mtp_err("%u:%u offset %u larger than skb len %u\n", msk->src_port,
		msk->dst_port, offset, skb->len);
	return -EBADMSG;
}

static int mtp_tls_recv_skb_append(struct mtp_sock *msk, u16 *record_size, u16 *record_unread,
	int i, struct sk_buff *skb)
{
	struct mtp_tls_aead *rx = msk->tls_ctx->rx;
	struct mtp_tls_rec *rec = rx->open_rec;
	struct mtp_tls_info *prot_info = &msk->tls_ctx->prot_info;
	u32 offset = msk->copied_seq - MTP_SKB_CB(skb)->seq;
	unsigned long used;
	int sg_used;
	struct mtp_tls_cb *tls_cb = skb_tls_cb(skb);

	if (offset >= skb->len)
		return mtp_tls_recv_offset_error(msk, skb, offset);

	used = skb->len - offset;
	sg_unmark_end(&rec->sg_en[i]);
	if (!i) {
		char prepend_buf[TLS_HEADER_SIZE + MAX_IV_SIZE];
		int ret;

		if (offset + prot_info->prepend_size > skb->len) {
			mtp_err("prepend_buf error offset %u len %u data_len %u\n",
				offset, skb->len, skb->data_len);
			return -EBADMSG;
		}

		/* Linearize header to local buffer */
		ret = skb_copy_bits(skb, offset, prepend_buf, prot_info->prepend_size);
		if (ret < 0) {
			mtp_err("skb_copy_bits error offset %u len %u data_len %u\n",
				offset, skb->len, skb->data_len);
			return -EBADMSG;
		}

		*record_size = mtp_tls_prepend_buf_parse(rx, prot_info, prepend_buf, used);
		if (!*record_size)
			return -EBADMSG;

		if (used > (*record_size + TLS_HEADER_SIZE))
			used = *record_size + TLS_HEADER_SIZE;
		sg_used = skb_to_sgvec_nomark(skb, &rec->sg_en[i], offset + prot_info->prepend_size,
					      used - prot_info->prepend_size);
		*record_unread = *record_size - (used - TLS_HEADER_SIZE);
		tls_cb->len = used - prot_info->prepend_size;
		tls_cb->offset = offset + prot_info->prepend_size;
	} else {
		if (used > *record_unread)
			used = *record_unread;
		sg_used = skb_to_sgvec_nomark(skb, &rec->sg_en[i], offset, used);
		*record_unread -= used;
		tls_cb->len = used;
		tls_cb->offset = offset;
	}

	WRITE_ONCE(msk->copied_seq, msk->copied_seq + used);
	return sg_used;
}

static s16 mtp_recv_get_record(struct sock *sk, bool *read_finish, u16 *copied_skb, int *seg)
{
	struct sk_buff *skb, *tmp;
	u16 record_size = 0;
	u16 record_unread = 0;
	int i = 0;
	struct mtp_sock *msk = mtp_sk(sk);
	struct mtp_tls_rec *rec = msk->tls_ctx->rx->open_rec;

	skb_queue_walk_safe(&sk->sk_receive_queue, skb, tmp) {
		__u8 type = MTP_SKB_CB(skb)->type;
		int sg_used = mtp_tls_recv_skb_append(msk, &record_size, &record_unread, i, skb);
		if (sg_used <= 0)
			return (s16)sg_used;

		i += sg_used;
		mtp_debug("i %d record_size %hu sg_used %d record_unread %hu\n",
			i, record_size, sg_used, record_unread);
		if ((msk->copied_seq - MTP_SKB_CB(skb)->seq) < skb->len) {
			mtp_debug("skb is partially read");
			break;
		}
		*copied_skb += 1;
		__skb_unlink(skb, &sk->sk_receive_queue);
		__skb_queue_tail(&rec->skb_queue, skb);

		if (type == MTPHDR_TYPE_FIN)
			WRITE_ONCE(msk->copied_seq, msk->copied_seq + 1);
		if (type == MTPHDR_TYPE_MSG_END || type == MTPHDR_TYPE_FIN) {
			*read_finish = true;
			if (record_unread) {
				mtp_err("record_unread left %u type %u record_size %u\n",
					record_unread, type, record_size);
				goto read_failure;
			}
			break;
		}

		if (!record_unread)
			break;
	}

	if (!i)
		return 0;

	if (record_unread) {
		mtp_err("record_unread left %u\n", record_unread);
read_failure:
		mtp_tls_err_recv(sk, rec);
		return -EBADMSG;
	}

	sg_mark_end(&rec->sg_en[i - 1]);
	*seg = i;
	return (s16)record_size;
}

static int mtp_decrypt(struct mtp_tls_aead *rx, struct mtp_tls_rec *rec, struct mtp_tls_info *prot_info, u16 user_size)
{
	int err;

	mtp_tls_fill_sg(rec, rx, user_size, false);
	aead_request_set_tfm(&rec->aead_req, rx->aead);
	aead_request_set_ad(&rec->aead_req, prot_info->aad_size);
	aead_request_set_crypt(&rec->aead_req, rec->sg_aead_in, rec->sg_aead_out,
			       user_size + prot_info->tag_size, rx->cipher.iv);
	aead_request_set_callback(&rec->aead_req, CRYPTO_TFM_REQ_MAY_BACKLOG, crypto_req_done, &rx->async_wait);
	err = crypto_aead_decrypt(&rec->aead_req);
	if (err < 0) {
		if (err != -EINPROGRESS) {
			mtp_err("crypto_aead_decrypt fail err %d\n", err);
			MTP_INC_STATS(MTP_MIB_TLSDECRYPTERROR);
			return err;
		}
		mtp_info("decrypt in process\n");
		err = crypto_wait_req(err, &rx->async_wait);
		if (err) {
			mtp_err("crypto_wait_req err: %d\n", err);
			MTP_INC_STATS(MTP_MIB_TLSDECRYPTERROR);
			return err;
		}
	}

	mtp_tls_advance_record_sn(prot_info, &rx->cipher);
	mtp_debug("decrypt succ\n");
	return 0;
}

static int mtp_recv_sgout_skb_append(struct sock *sk, struct mtp_tls_rec *rec, s16 *sg_out_idx, int len)
{
	struct sk_buff *skb;
	int sg_used;
	struct mtp_tls_cb *tls_cb;

	rec->user_data_offset = 0;
	rec->user_data_len = len;

	if (unlikely(*sg_out_idx))
		sg_unmark_end(&rec->sg_pl[*sg_out_idx - 1]);
	skb_queue_walk(&rec->skb_queue, skb) {
		int used;

		tls_cb = skb_tls_cb(skb);
		used = min_t(int, len, tls_cb->len);
		sg_used = skb_to_sgvec_nomark(skb, &rec->sg_pl[*sg_out_idx], tls_cb->offset, used);
		if (sg_used < 0) {
			mtp_err("skb_to_sgvec_nomark error %d\n", sg_used);
			mtp_tls_err_recv(sk, rec);
			return sg_used;
		}
		*sg_out_idx += (s16)sg_used;

		len -= used;
		if (len <= 0)
			goto exit;
	}

	if (len) {
		/* the last frag must in head of sk_receive_queue */
		skb = skb_peek(&sk->sk_receive_queue);
		tls_cb = skb_tls_cb(skb);
		if (len >= tls_cb->len) {
			mtp_err("len %d bigger than tls_cb->len %d\n", len, tls_cb->len);
			mtp_tls_err_recv(sk, rec);
			return -EBADMSG;
		}

		sg_used = skb_to_sgvec_nomark(skb, &rec->sg_pl[*sg_out_idx], tls_cb->offset, len);
		if (sg_used < 0) {
			mtp_err("skb_to_sgvec_nomark error %d\n", sg_used);
			mtp_tls_err_recv(sk, rec);
			return sg_used;
		}
		*sg_out_idx += (s16)sg_used;
	}
	mtp_debug("user_data_len %d sg_out_idx %d\n", rec->user_data_len, *sg_out_idx);
exit:
	sg_mark_end(&rec->sg_pl[*sg_out_idx - 1]);
	return 0;
}

/* return true if skb recv is done */
static bool mtp_recv_skb_eat(struct mtp_tls_rec *rec, struct sk_buff *skb, u16 *copy_len, int *copied, int chunk)
{
	struct mtp_tls_cb *tls_cb = skb_tls_cb(skb);

	*copied += chunk;
	*copy_len -= chunk;
	if (tls_cb->len <= (rec->user_data_offset + chunk)) {
		__skb_unlink(skb, &rec->skb_queue);
		kfree_skb(skb);
		rec->user_data_offset = 0;
	} else {
		rec->user_data_offset += chunk;
		rec->user_data_len -= *copied;
		if (!rec->user_data_len) {
			__skb_queue_purge(&rec->skb_queue);
			rec->user_data_offset = 0;
		}
		return true;
	}
	return false;
}

static int mtp_recv_decrypted_skb(struct sock *sk, struct msghdr *msg, struct mtp_tls_rec *rec, size_t len)
{
	int err;
	u16 copy_len = (u16)min_t(size_t, len, rec->user_data_len);
	struct sk_buff *skb, *tmp;
	int copied = 0;
	struct mtp_tls_cb *tls_cb;

	skb_queue_walk_safe(&rec->skb_queue, skb, tmp) {
		int chunk;

		tls_cb = skb_tls_cb(skb);
		chunk = min_t(int, copy_len, (tls_cb->len - rec->user_data_offset));
		err = skb_copy_datagram_msg(skb, tls_cb->offset + rec->user_data_offset, msg, chunk);
		if (err < 0)
			return err;

		if (mtp_recv_skb_eat(rec, skb, &copy_len, &copied, chunk))
			return copied;
	}

	__skb_queue_purge(&rec->skb_queue);
	if (copy_len) {
		/* the last frag must in head of sk_receive_queue */
		skb = skb_peek(&sk->sk_receive_queue);
		tls_cb = skb_tls_cb(skb);
		if (copy_len >= tls_cb->len) {
			mtp_err("copy_len %d tls_cb->len %d offset %d\n", copy_len, tls_cb->len, tls_cb->offset);
			mtp_tls_err_recv(sk, rec);
			return -EBADMSG;
		}

		err = skb_copy_datagram_msg(skb, tls_cb->offset, msg, copy_len);
		if (err < 0)
			return err;
		copied += copy_len;
		rec->user_data_offset += copy_len;
	}
	rec->user_data_len -= copied;
	return copied;
}

static int copy_iter_from_decrypted_skb(struct sock *sk, struct msghdr *msg, struct mtp_tls_rec *rec, size_t len, s16 user_size)
{
	int decrypted_size = mtp_recv_decrypted_skb(sk, msg, rec, len);
	mtp_debug("len %d decrypted_size %d\n", len, decrypted_size);
	if (decrypted_size < 0) {
		mtp_debug("mtp_recv_decrypted_skb failed %d\n", decrypted_size);
		return -EBADMSG;
	}
	if (user_size <= decrypted_size)
		__skb_queue_purge(&rec->skb_queue);
	return decrypted_size;
}

static s16 mtp_tls_recv_record(struct sock *sk, struct msghdr *msg, size_t len, u16 *copied_skb, bool *read_finish)
{
	struct mtp_tls_aead *rx = mtp_sk(sk)->tls_ctx->rx;
	struct mtp_tls_rec *rec = rx->open_rec;
	struct mtp_tls_info *prot_info = &mtp_sk(sk)->tls_ctx->prot_info;
	s16 record_size = 0;
	s16 user_size, recv_size;
	int sg_en_cnt = 0;
	int err;

	record_size = mtp_recv_get_record(sk, read_finish, copied_skb, &sg_en_cnt);
	if (record_size <= 0)
		return record_size;

	user_size = record_size - prot_info->tag_size - prot_info->iv_size;
	recv_size = min_t(int, user_size, len);
	err = mtp_sg_from_iter(&msg->msg_iter, rec->sg_pl, recv_size, &rec->sg_pl_frag_iter);
	if (err) {
		mtp_debug("mtp_sg_from_iter err %d\n", err);
		recv_size = 0;
		rec->sg_pl_frag_used = 0;
	} else {
		rec->sg_pl_frag_used = rec->sg_pl_frag_iter;
	}

	if (user_size > recv_size) {
		err = mtp_recv_sgout_skb_append(sk, rec, &rec->sg_pl_frag_used, user_size - recv_size);
		if (err) {
			mtp_err("mtp_recv_sgout_skb_append err %d\n", err);
			mtp_sg_put_page(rec->sg_pl, rec->sg_pl_frag_iter);
			sg_init_table(rec->sg_pl, TLS_SG_MAX);
			mtp_tls_err_recv(sk, rec);
			return -EBADMSG;
		}
		rec->last_frag = *read_finish;
	}

	err = mtp_decrypt(rx, rec, prot_info, user_size);
	mtp_sg_put_page(rec->sg_pl, rec->sg_pl_frag_iter);
	sg_unmark_end(&rec->sg_en[sg_en_cnt - 1]);
	mtp_rec_pl_sg_init(rec);
	if (user_size <= recv_size)
		__skb_queue_purge(&rec->skb_queue);
	if (err) {
		iov_iter_revert(&msg->msg_iter, user_size);
		mtp_tls_err_recv(sk, rec);
		return -EBADMSG;
	}
	if (recv_size == 0)
		recv_size = (s16)copy_iter_from_decrypted_skb(sk, msg, rec, len, user_size);
	return recv_size;
}

int mtp_tls_recv_queue(struct sock *sk, struct msghdr *msg, size_t len, u16 *copied_skb)
{
	int copied = 0;
	bool read_finish = false;
	struct mtp_sock *msk = mtp_sk(sk);
	struct mtp_tls_aead *rx = msk->tls_ctx->rx;
	struct mtp_tls_rec *rec = rx->open_rec;

	if (rec->user_data_len) {
		int copy_len = mtp_recv_decrypted_skb(sk, msg, rec, len);
		if (copy_len < 0)
			return copy_len;
		mtp_err("mtp_recv_decrypted_skb %d\n", copy_len);
		len -= copy_len;
		copied += copy_len;
		if (!rec->user_data_len && rec->last_frag) {
			msk->tls_recv_pend = !!rec->user_data_len;
			return copied;
		}
	}

	while (len > 0 && !read_finish) {
		s16 recv_size = mtp_tls_recv_record(sk, msg, len, copied_skb, &read_finish);
		if (recv_size <= 0) {
			if (!copied)
				return recv_size;
			return copied;
		}
		copied += recv_size;
		if (copied > mtp_sk(sk)->max_msg_size) {
			mtp_err("%u:%u msg size too big, copied %d max_msg_size %u\n", mtp_sk(sk)->src_port,
				msk->dst_port, copied, msk->max_msg_size);
			break;
		}
		len -= recv_size;
	}

	msk->tls_recv_pend = !!rec->user_data_len;
	return copied;
}

void mtp_tls_send_pending_page(struct sock *sk)
{
	struct mtp_sock *msk = mtp_sk(sk);
	int err;
	struct mtp_tls_aead *tx = msk->tls_ctx->tx;
	struct mtp_tls_rec *rec = tx->open_rec;
	struct mtp_tls_info *prot_info = &msk->tls_ctx->prot_info;
	char *prepend_buf;
	int seg = 0;

	if (rec->sg_pl_frag_used <= 0 || rec->sg_pl_len <= 0) {
		struct sk_buff *tskb = mtp_write_queue_tail(sk);
		if (tskb && MTP_SKB_CB(tskb)->type == MTPHDR_TYPE_DEFAULT) {
			msk->cur_snd_msg_size = 0;
			MTP_SKB_CB(tskb)->type = MTPHDR_TYPE_MSG_END;
		}
		return;
	}
	sg_mark_end(&rec->sg_pl[rec->sg_pl_frag_used - 1]);
	if (sk->sk_err || (sk->sk_shutdown & SEND_SHUTDOWN)) {
		rec->sg_pl_frag_used = 0;
		rec->sg_pl_frag_iter = 0;
		rec->sg_pl_len = 0;
		sg_chain(rec->sg_aead_in, 2, rec->sg_pl);
		goto free_pages;
	}
	err = mtp_send_skb_alloc(sk, rec, prot_info, rec->sg_pl_len + prot_info->overhead_size, &prepend_buf);
	if (err) {
		mtp_err("mtp_send_skb_alloc failed\n");
		sg_chain(rec->sg_aead_in, 2, rec->sg_pl);
		goto free_pages;
	}

	mtp_tls_fill_prepend(prepend_buf, rec->sg_pl_len, tx->cipher.iv);
	err = mtp_encrypt(tx, rec, prot_info, rec->sg_pl_len);
	if (err) {
		mtp_err("mtp_encrypt failed\n");
		__skb_queue_purge(&rec->skb_queue);
		goto free_pages;
	}

	mtp_push_record(sk, rec, &seg, true);
free_pages:
	mtp_encrypt_free_pl_pages(rec);
	mtp_rec_pl_sg_init(rec);
}

int mtp_tls_sendpage_locked(struct sock *sk, struct page *page, int offset, size_t size, int flags)
{
	struct mtp_sock *msk = mtp_sk(sk);
	int err;
	int eor = !(flags & MSG_SENDPAGE_NOTLAST) && (!msk->sendpage_msg_size);
	struct mtp_tls_aead *tx = msk->tls_ctx->tx;
	struct mtp_tls_rec *rec = tx->open_rec;
	struct mtp_tls_info *prot_info = &msk->tls_ctx->prot_info;
	char *prepend_buf;
	int seg = 0;
	int skb_len;

	if (size > (prot_info->record_size - prot_info->overhead_size - rec->sg_pl_len))
		size = prot_info->record_size - prot_info->overhead_size - rec->sg_pl_len;
	get_page(page);
	sg_unmark_end(&rec->sg_pl[rec->sg_pl_frag_used]);
	sg_set_page(&rec->sg_pl[rec->sg_pl_frag_used], page, size, offset);
	rec->sg_pl_frag_used++;
	rec->sg_pl_len += (int)size;
	if (!eor && (rec->sg_pl_frag_used < MAX_SKB_FRAGS) &&
	    (rec->sg_pl_len < (prot_info->record_size - prot_info->overhead_size))) {
		mtp_debug("wait more data flags %x tot_len %d, used %d ret %zu\n",
			flags, rec->sg_pl_len, rec->sg_pl_frag_used, size);
		return size;
	}
	sg_mark_end(&rec->sg_pl[rec->sg_pl_frag_used - 1]);
	skb_len = rec->sg_pl_len + prot_info->overhead_size;
	err = mtp_send_skb_alloc(sk, rec, prot_info, skb_len, &prepend_buf);
	if (err) {
		mtp_err("mtp_send_skb_alloc failed\n");
		goto error;
	}

	mtp_tls_fill_prepend(prepend_buf, rec->sg_pl_len, tx->cipher.iv);
	err = mtp_encrypt(tx, rec, prot_info, rec->sg_pl_len);
	if (err) {
		mtp_err("mtp_encrypt failed\n");
		__skb_queue_purge(&rec->skb_queue);
error:
		rec->sg_pl_len -= size;
		rec->sg_pl_frag_used--;
		mtp_put_page(page);
		return err;
	}

	mtp_encrypt_free_pl_pages(rec);
	mtp_rec_pl_sg_init(rec);
	if (!msk->sendpage_msg_size || (msk->cur_snd_msg_size + skb_len) >= msk->sendpage_msg_size)
		eor = 1;
	mtp_push_record(sk, rec, &seg, !!eor);
	mtp_debug("send segments %d ret %zu\n", seg, size);
	return size;
}

static int mtp_splice_decrypted_skb(struct sock *sk, struct pipe_inode_info *pipe, struct mtp_tls_rec *rec,
	size_t len, bool *read_finish)
{
	u16 copy_len = (u16)min_t(size_t, len, rec->user_data_len);
	struct sk_buff *skb, *tmp;
	int copied = 0;
	struct mtp_tls_cb *tls_cb;

	skb_queue_walk_safe(&rec->skb_queue, skb, tmp) {
		int chunk;

		if (!copy_len) {
			rec->user_data_len -= copied;
			return copied;
		}

		tls_cb = skb_tls_cb(skb);
		chunk = min_t(int, copy_len, (tls_cb->len - rec->user_data_offset));

		chunk = skb_splice_bits(skb, sk, tls_cb->offset + rec->user_data_offset, pipe, chunk, 0);
		if (chunk < 0) {
			mtp_err("skb_queue splice return %d\n", chunk);
			rec->user_data_len -= copied;
			*read_finish = true;
			return copied ? copied : chunk;
		}
		if (mtp_recv_skb_eat(rec, skb, &copy_len, &copied, chunk))
			return copied;
	}

	__skb_queue_purge(&rec->skb_queue);
	if (copy_len) {
		/* the last frag must in head of sk_receive_queue */
		skb = skb_peek(&sk->sk_receive_queue);
		if (unlikely(!skb)) {
			mtp_err("skb is NULL, left copy_len %d\n", copy_len);
			goto err_abort;
		}
		tls_cb = skb_tls_cb(skb);
		if (copy_len >= (tls_cb->len - rec->user_data_offset)) {
			mtp_err("copy_len %d tls_cb->len %d offset %d\n", copy_len, tls_cb->len, rec->user_data_offset);
err_abort:
			mtp_tls_err_recv(sk, rec);
			return copied ? copied : -EBADMSG;
		}

		copy_len = skb_splice_bits(skb, sk, tls_cb->offset + rec->user_data_offset, pipe, copy_len, 0);
		if (copy_len < 0) {
			mtp_err("sk_receive_queue splice return %d\n", copy_len);
			rec->user_data_len -= copied;
			*read_finish = true;
			return copied ? copied : copy_len;
		}
		copied += copy_len;
		rec->user_data_offset += copy_len;
	}
	rec->user_data_len -= copied;
	return copied;
}

static bool mtp_pipe_space_is_low(struct pipe_inode_info *pipe)
{
	unsigned int p_occupancy;
	p_occupancy = pipe_occupancy(pipe->head, pipe->tail);
	if (p_occupancy >= pipe->max_usage || (pipe->max_usage - p_occupancy) < MAX_SKB_FRAGS)
		return true;
	return false;
}

ssize_t mtp_tls_splice_read(struct sock *sk, struct pipe_inode_info *pipe, size_t len, u16 *copied_skb)
{
	int copied = 0;
	int err;
	bool read_finish = false;
	struct mtp_sock *msk = mtp_sk(sk);
	struct mtp_tls_aead *rx = msk->tls_ctx->rx;
	struct mtp_tls_rec *rec = rx->open_rec;
	struct mtp_tls_info *prot_info = &msk->tls_ctx->prot_info;

	if (rec->user_data_len)
		goto direct_read;

	while (len > 0 && !read_finish) {
		s16 record_size = 0;
		u16 user_size;
		int sg_en_cnt;
		int copy_len;

		record_size = mtp_recv_get_record(sk, &read_finish, copied_skb, &sg_en_cnt);
		if (record_size <= 0) {
			if (!copied)
				copied = record_size;
			break;
		}
		user_size = (u16)record_size - prot_info->tag_size - prot_info->iv_size;
		err = mtp_recv_sgout_skb_append(sk, rec, &rec->sg_pl_frag_used, user_size);
		if (err)
			goto err_out;
		rec->last_frag = read_finish;

		err = mtp_decrypt(rx, rec, prot_info, user_size);
		sg_unmark_end(&rec->sg_en[sg_en_cnt - 1]);
		mtp_rec_pl_sg_init(rec);
		if (err) {
			mtp_tls_err_recv(sk, rec);
err_out:
			if (!copied)
				copied = err;
			break;
		}

direct_read:
		copy_len = mtp_splice_decrypted_skb(sk, pipe, rec, len, &read_finish);
		if (copy_len < 0)
			break;
		mtp_debug("mtp_recv_decrypted_skb %u\n", copy_len);
		len -= copy_len;
		copied += copy_len;

		if (rec->error || mtp_pipe_space_is_low(pipe))
			break;
	}

	msk->tls_recv_pend = !!rec->user_data_len;
	return copied;
}
