/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2021-2021. All rights reserved.
 * foss@huawei.com
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

#ifndef PC5_DATA_DEF_H
#define PC5_DATA_DEF_H

#include <linux/skbuff.h>

#ifdef __cplusplus
extern "C" {
#endif

#define PC5_ADDR_LEN 3
#define PC5_MAX_PKT_LEN 8188

struct pc5_tx_data_s {
	unsigned short ver;                    /* version */
	unsigned char  src_addr[PC5_ADDR_LEN]; /* source address */
	unsigned char  dst_addr[PC5_ADDR_LEN]; /* destination address */
	unsigned char  priority;               /* priority: [1~8] */
	unsigned char  pdu_type;               /* packet type, IP(0) or Non-IP(1) */
	unsigned short pdb;                    /* packet delay budget */
	unsigned short traffic_period;         /* traffic period */
	unsigned short data_len;               /* data length */
	unsigned char  data[0];                /* data content */
};

struct pc5_tx_data_v2_s {
	unsigned short ver;                    /* version */
	unsigned char  src_addr[PC5_ADDR_LEN]; /* source address */
	unsigned char  dst_addr[PC5_ADDR_LEN]; /* destination address */
	unsigned char  priority;               /* priority: [1~8] */
	unsigned char  pdu_type;               /* packet type: IP(0) or Non-IP(1) */
	unsigned short pdb;                    /* packet delay budget */
	unsigned short traffic_period;         /* traffic period */
	unsigned char  tx_profile;             /* profile version */
	unsigned char  rsv[3];                 /* reserved */
	unsigned short data_len;               /* data length */
	unsigned char  data[0];                /* data content */
};

struct pc5_rx_data_s {
	unsigned short ver;                    /* version */
	unsigned char  src_addr[PC5_ADDR_LEN]; /* source address */
	unsigned char  dst_addr[PC5_ADDR_LEN]; /* destination address */
	unsigned int   max_data_rate;          /* CCSA: 0-1585200, 0xFFFFFFFF */
	unsigned char  priority;               /* priority: [1~8] */
	unsigned char  cbr;                    /* channel busy rate */
	unsigned short frame_num;              /* frame number */
	unsigned char  subfrm_num;             /* subframe number */
	unsigned char  rsv;                    /* reserved */
	unsigned short data_len;               /* data length */
	unsigned char  data[0];                /* data content */
};

#define PC5_HDR_VER_1 1
#define PC5_HDR_VER_2 2
#define PC5_HDR_VER_LEN sizeof(unsigned short)

static inline unsigned short pc5_get_version(const struct sk_buff *skb)
{
	return *((unsigned short *)skb->data);
}

#define PC5_TXHDR_V1_LEN sizeof(struct pc5_tx_data_s)
#define PC5_TXHDR_V2_LEN sizeof(struct pc5_tx_data_v2_s)

#ifdef __cplusplus
}
#endif

#endif /* PC5_DATA_DEF_H */
