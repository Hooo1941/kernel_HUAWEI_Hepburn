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
 * * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS
IS"
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
 *
 */

#ifndef DUMP_GENETLINK_H
#define DUMP_GENETLINK_H

#include <linux/netlink.h>
#include <net/genetlink.h>
#include <linux/skbuff.h>
#include <osl_types.h>
#include <osl_sem.h>
#include <bsp_dump.h>

#undef THIS_MODU
#define THIS_MODU mod_homi

#define DUMP_GENL_NAME "dump_genl"
#define DUMP_GENL_VERSION 0x1

#define DUMP_EAGAIN_RETRY_MSLEEP_ONCE 1

#define DUMP_UPLOAD_PKT_START_MAGIC 0x56786543
#define DUMP_UPLOAD_PKT_END_MAGIC 0xcdefabcd

/* same as kernel dump define */
enum dump_genl_cmd {
    DUMP_GENL_CMD_NONE,
    DUMP_GENL_CMD_UPLOAD,
    DUMP_GENL_CMD_BUTT
};

struct dump_upload_data_pkt {
    uint32_t start_magic;
    uint32_t file_offset;
    uint16_t file_id;
    uint16_t blk_seq;
    uint16_t blk_len;
    uint16_t blk_num;
    uint32_t end_magic;
    char data[0];
};

struct dump_upload_data_hdr {
    uint16_t kernel_msg_type;
    uint16_t op;
    uint16_t seq_id;
    uint16_t data_len;
    char payload[0];
};

struct dump_genl_msg_hdr {
    struct nlmsghdr nlh;
    struct genlmsghdr genlh;
    char payload[0];
};

typedef enum {
    DUMP_ERR_PORT_NULL = 0x100,
    DUMP_ERR_MSG_NEW_FAIL,
    DUMP_ERR_MSG_PUT_FAIL,
    DUMP_ERR_BUTT
} dump_genl_errno_e;

typedef struct {
    /* data hdr info */
    uint16_t msg_type;
    uint16_t op;
    uint16_t msg_seq;
    uint16_t resv;
    /* file pkt hdr info */
    uint32_t file_offset;
    uint16_t file_id;
    uint16_t blk_seq;
    uint16_t blk_len;
    uint16_t blk_num;
    /* data */
    unsigned char *data;
} dump_genl_data_desc_s;

typedef struct {
    struct dump_upload_data_hdr data_hdr;
    struct dump_upload_data_pkt data_pkt;
    unsigned char data[0];
} dump_genl_unicast_msg_s;

typedef struct {
    u16 msg_type;
    u16 op;
    u16 file_id;
    u16 resv;
    u32 file_offset;
    u32 timeout_ms;
} dump_genl_pkt_ctrl_s;

struct dump_conn_info {
    struct net *net;
    u32 port_id;
    osl_sem_id chan_sem;
};

typedef struct {
    struct genl_family family;
    struct dump_conn_info con_info;
} dump_genl_ctrl_s;

#define DUMP_GENL_DATA_SIZE (NLMSG_DEFAULT_SIZE - GENL_HDRLEN - sizeof(dump_genl_unicast_msg_s))
#define DUMP_GENL_UNICAST_MSG_ARGS_SIZE(_m) (sizeof(((dump_genl_unicast_msg_s *)0)->_m))

int dump_genl_init(void);
void dump_genl_set_port_info(struct genl_info *info);
int dump_genl_rcv_para_check(struct sk_buff *skb, struct genl_info *info);
int dump_genl_send_data_advance(u8 *buf, u32 len, dump_genl_pkt_ctrl_s *adv_ctrl);

int dump_genl_rcv_handler(struct sk_buff *skb, struct genl_info *info);
#endif
