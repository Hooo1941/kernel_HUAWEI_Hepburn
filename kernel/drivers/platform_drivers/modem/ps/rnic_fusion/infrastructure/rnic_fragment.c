/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2021-2021. All rights reserved.
 *
 * If distributed as part of the Linux kernel, the following license terms
 * apply:
 *
 * * This program is free software; you can redistribute it and/or modify
 * * it under the terms of the GNU General Public License version 2 and
 * * only version 2 as published by the Free Software Foundation.
 * *
 * * This program is distributed in the hope that it will be useful,
 * * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * * GNU General Public License for more details.
 * *
 * * You should have received a copy of the GNU General Public License
 * * along with this program; if not, write to the Free Software
 * * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307, USA
 *
 * Otherwise, the following license terms apply:
 *
 * * Redistribution and use in source and binary forms, with or without
 * * modification, are permitted provided that the following conditions
 * * are met:
 * * 1) Redistributions of source code must retain the above copyright
 * *    notice, this list of conditions and the following disclaimer.
 * * 2) Redistributions in binary form must reproduce the above copyright
 * *    notice, this list of conditions and the following disclaimer in the
 * *    documentation and/or other materials provided with the distribution.
 * * 3) Neither the name of Huawei nor the names of its contributors may
 * *    be used to endorse or promote products derived from this software
 * *    without specific prior written permission.
 *
 * * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

#include <linux/netdevice.h>
#include <linux/skbuff.h>
#include "rnic_data.h"
#include "rnic_fragment_i.h"
#include "rnic_log.h"
#include "rnic_private.h"
#include "rnic_fragment.h"

STATIC LIST_HEAD(rnic_frag_queue_list);
STATIC DEFINE_MUTEX(rnic_frag_queue_lock);

STATIC void rnic_fill_frag_info(struct sk_buff *skb, u32 offset, u32 left, u8 seq)
{
	struct rnic_frag_info_s *finfo = rnic_wan_frag_cb(skb);

	finfo->value = 0;
	finfo->first = !offset;
	finfo->last  = !left;
	finfo->seq   = seq;
}

STATIC struct rnic_frag_queue_s *rnic_frag_find(struct net_device *netdev)
{
	struct rnic_frag_queue_s *fq = NULL;
	struct list_head *pos = NULL;

	list_for_each(pos, &rnic_frag_queue_list) {
		fq = list_entry(pos, struct rnic_frag_queue_s, list);
		if (fq->netdev == netdev)
			return fq;
	}

	return NULL;
}

STATIC int rnic_do_fragment(struct rnic_frag_queue_s *fq, struct sk_buff *skb,
			    int (*output)(struct sk_buff *, struct net_device *))
{
	struct sk_buff *skb2 = NULL;
	unsigned int left = skb->len;
	unsigned int offset = 0;
	unsigned int len;
	u8 seq_id = 0;

	while (left > 0) {
		len = left;
		if (len > RNIC_DEFAULT_MTU)
			len = RNIC_DEFAULT_MTU;

		/* Align the next start on an eight byte boundary. */
		if (len < left)
			len &= ~7U;

		/* Alloc new buffer. */
		skb2 = netdev_alloc_skb_ip_align(NULL, len + RNIC_LL_RES_SPACE);
		if (skb2 == NULL) {
			fq->stats.frag_alloc_errs++;
			goto fail;
		}

		/* Setup reserve space. */
		skb_reserve(skb2, RNIC_LL_RES_SPACE);

		/* Copy data into the new buffer. */
		skb_copy_from_linear_data_offset(skb, (int)offset,
						 skb_put(skb2, len), len);
		skb2->protocol = skb->protocol;

		left -= len;
		rnic_fill_frag_info(skb2, offset, left, seq_id++);
		offset += len;

		output(skb2, fq->netdev);
		fq->stats.frag_oks++;
	}

	dev_consume_skb_any(skb);
	return 0;

fail:
	dev_kfree_skb_any(skb);
	return -ENOMEM;
}

int rnic_fragment(struct sk_buff *skb, struct net_device *netdev,
		  int (*output)(struct sk_buff *, struct net_device *))
{
	if (skb->len > RNIC_DEFAULT_MTU) {
		struct rnic_frag_queue_s *fq = rnic_frag_find(netdev);
		if (fq == NULL)
			goto out;
		return rnic_do_fragment(fq, skb, output);
	}

out:
	rnic_fill_frag_info(skb, 0, 0, 0);
	return output(skb, netdev);
}

STATIC bool rnic_is_fragment(struct sk_buff *skb)
{
	if (!rnic_map_field0_cb(skb)->field1_en)
		return false;

	return !(rnic_map_frag_cb(skb)->first && rnic_map_frag_cb(skb)->last);
}

STATIC void rnic_frag_flush(struct rnic_frag_queue_s *fq)
{
	skb_queue_purge(&fq->frag_list);
	fq->flags = 0;
	fq->seq_next = 0;
}

STATIC int rnic_frag_queue(struct rnic_frag_queue_s *fq, struct sk_buff *skb)
{
	struct rnic_frag_info_s *finfo = rnic_map_frag_cb(skb);

	/* The first fragment? */
	if (finfo->first) {
		if (fq->flags != 0) {
			rnic_frag_flush(fq);
			fq->stats.reasm_order_errs++;
		}

		fq->flags |= RNIC_FRAG_FIRST_IN;
	}

	/* Check out of order. */
	if (!(fq->flags & RNIC_FRAG_FIRST_IN)) {
		fq->stats.reasm_order_errs++;
		goto err;
	}

	/* Check if the sequence number of this fragment is expected. */
	if (finfo->seq != fq->seq_next) {
		fq->stats.reasm_seq_errs++;
		goto err;
	}

	/* The last fragment? */
	if (finfo->last)
		fq->flags |= RNIC_FRAG_LAST_IN;

	/* Insert fragment. */
	skb_queue_tail(&fq->frag_list, skb);
	fq->seq_next++;
	return 0;

err:
	dev_kfree_skb_any(skb);
	return -EINVAL;
}

STATIC struct sk_buff *rnic_frag_reasm(struct rnic_frag_queue_s *fq)
{
	struct sk_buff *skb = NULL;
	struct sk_buff *skb2 = NULL;
	int total_len = 0;
	int space;

	/* Could optimize this by using skb_try_coalesce? */
	skb2 = __skb_dequeue(&fq->frag_list);

	/* Alloc new buffer. */
	skb_queue_walk(&fq->frag_list, skb)
		total_len += (int)skb->len;

	space = total_len - skb_tailroom(skb2);
	if (space > 0 && pskb_expand_head(skb2, 0, space, GFP_ATOMIC)) {
		__skb_queue_head(&fq->frag_list, skb2);
		fq->stats.reasm_alloc_errs++;
		return NULL;
	}

	/* Copy data into the new buffer. */
	while ((skb = __skb_dequeue(&fq->frag_list))) {
		skb_copy_from_linear_data(skb, skb_put(skb2, skb->len),
					  skb->len);
		dev_kfree_skb_any(skb);
	}

	fq->flags = 0;
	fq->seq_next = 0;
	fq->stats.reasm_oks++;
	return skb2;
}

STATIC int rnic_frag_rcv(struct rnic_frag_queue_s *fq, struct sk_buff *skb,
			 int (*input)(struct sk_buff *, struct net_device *))
{
	int ret;

	ret = rnic_frag_queue(fq, skb);
	if (ret)
		goto err;

	if (fq->flags == (RNIC_FRAG_FIRST_IN | RNIC_FRAG_LAST_IN)) {
		struct sk_buff *skb2 = rnic_frag_reasm(fq);
		if (skb2 == NULL) {
			ret = -ENOMEM;
			goto err;
		}

		return input(skb2, fq->netdev);
	}

	return 0;

err:
	rnic_frag_flush(fq);
	return ret;
}

int rnic_defrag(struct sk_buff *skb, struct net_device *netdev,
		int (*input)(struct sk_buff *, struct net_device *))
{
	if (rnic_is_fragment(skb)) {
		struct rnic_frag_queue_s *fq = rnic_frag_find(netdev);
		if (fq == NULL)
			goto out;
		return rnic_frag_rcv(fq, skb, input);
	}

out:
	return input(skb, netdev);
}

int rnic_frag_create(struct net_device *netdev)
{
	struct rnic_frag_queue_s *fq = NULL;
	struct list_head *pos = NULL;
	int ret = 0;

	mutex_lock(&rnic_frag_queue_lock);
	list_for_each(pos, &rnic_frag_queue_list) {
		fq = list_entry(pos, struct rnic_frag_queue_s, list);
		if (fq->netdev == netdev)
			goto out;
	}

	fq = kzalloc(sizeof(*fq), GFP_KERNEL);
	if (fq == NULL) {
		ret = -ENOMEM;
		goto out;
	}

	fq->netdev = netdev;
	skb_queue_head_init(&fq->frag_list);
	list_add_tail(&fq->list, &rnic_frag_queue_list);

out:
	mutex_unlock(&rnic_frag_queue_lock);
	return ret;
}

void rnic_frag_destory(struct net_device *netdev)
{
	struct rnic_frag_queue_s *fq = NULL;
	struct list_head *pos = NULL;

	mutex_lock(&rnic_frag_queue_lock);
	list_for_each(pos, &rnic_frag_queue_list) {
		fq = list_entry(pos, struct rnic_frag_queue_s, list);
		if (fq->netdev == netdev)
			goto out_found;
	}

	RNIC_LOGE("netdev %s does not exist!\n", netdev->name);
	goto out;

out_found:
	skb_queue_purge(&fq->frag_list);
	list_del(&fq->list);
	kfree(fq);
out:
	mutex_unlock(&rnic_frag_queue_lock);
}

void rnic_frag_report_stats(struct net_device *netdev)
{
	struct rnic_frag_queue_s *fq = NULL;
	struct rnic_frag_pkt_stats_rpt_s rpt_info = {{0}};

	fq = rnic_frag_find(netdev);
	if (fq == NULL)
		return;

	/* version 101 */
	rpt_info.hdr.ver = 101;
	scnprintf(rpt_info.name, IFNAMSIZ, "%s", netdev->name);
	rpt_info.stats = fq->stats;
	rnic_trans_report(ID_DIAG_RNIC_FRAG_PKT_STATS, &rpt_info, sizeof(rpt_info));
}

void rnic_frag_show_stats(void)
{
	struct rnic_frag_stats_s *stats = NULL;
	struct rnic_frag_queue_s *fq = NULL;
	struct list_head *pos = NULL;

	list_for_each(pos, &rnic_frag_queue_list) {
		fq = list_entry(pos, struct rnic_frag_queue_s, list);
		pr_err("netdev %s frag stats:\n", fq->netdev->name);

		stats = &fq->stats;
		pr_err("frag_oks           %u\n", stats->frag_oks);
		pr_err("frag_alloc_errors  %u\n", stats->frag_alloc_errs);
		pr_err("reasm_oks          %u\n", stats->reasm_oks);
		pr_err("reasm_seq_errors   %u\n", stats->reasm_seq_errs);
		pr_err("reasm_order_errors %u\n", stats->reasm_order_errs);
		pr_err("reasm_alloc_errors %u\n", stats->reasm_alloc_errs);
	}
}
