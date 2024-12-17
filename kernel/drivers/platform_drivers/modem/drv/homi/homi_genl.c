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

#include <securec.h>
#include <bsp_print.h>
#include <osl_malloc.h>
#include <bsp_chr.h>
#include "homi_genl.h"

struct genl_ops g_homi_genl_ops[] = {
    {
        .cmd = GENL_CMD_CHR_RCV_MSG,
        .doit = bsp_chr_genl_rcv_handler,
    },
};

homi_genl_ctrl_s g_homi_ctrl_info;

homi_genl_ctrl_s *homi_genl_get_ctrl_info(void)
{
    return &g_homi_ctrl_info;
}

void bsp_homi_genl_set_port_info(homi_genl_chan_e chan_id, struct genl_info *info)
{
    if (chan_id >= GENL_CHAN_BUTT) {
        return;
    }

    g_homi_ctrl_info.chan_ctrl_info[chan_id].net = genl_info_net(info);
    g_homi_ctrl_info.chan_ctrl_info[chan_id].port_id = info->snd_portid;
}

int bsp_homi_genl_rcv_para_check(struct sk_buff *skb, struct genl_info *info)
{
    struct nlmsghdr *nlhdr = NULL;

    if (skb == NULL || info == NULL) {
        bsp_err("parameter null\n");
        return -1;
    }

    nlhdr = nlmsg_hdr(skb);
    if (nlhdr == NULL) {
        bsp_err("nlhdr is null\n");
        return -1;
    }

    if (skb->len < HOMI_GENL_HDR_LEN) {
        bsp_err("skb_len invalid, skb_len=0x%x\n", skb->len);
        return -1;
    }

    return 0;
}

static int homi_genl_send_fragment_data(homi_genl_chan_e chan_id, homi_genl_data_desc_s *data_desc)
{
        homi_genl_ctrl_s *ctrl_info = &g_homi_ctrl_info;
        struct sk_buff *genl_skb = NULL;
        char *payload_hdr = NULL;
        homi_genl_unicast_msg_s *unicast_msg = NULL;
        int ret;
        unsigned int payload_len = NLMSG_ALIGN(sizeof(homi_genl_unicast_msg_s) + data_desc->len);

        if (ctrl_info->chan_ctrl_info[chan_id].net == NULL) {
            bsp_info("homi genl chan net is null, chan_id=0x%x\n", chan_id);
            return HOMI_ERR_PORT_NULL;
        }
        
        genl_skb = genlmsg_new(payload_len, GFP_KERNEL);
        if (genl_skb == NULL) {
            bsp_err("nlmsg_new fail\n");
            return HOMI_ERR_MSG_NEW_FAIL;
        }

        payload_hdr = genlmsg_put(genl_skb, ctrl_info->chan_ctrl_info[chan_id].port_id,
            0, &ctrl_info->family, 0, GENL_CMD_CHR_RCV_MSG);
        if (payload_hdr == NULL) {
            ret = HOMI_ERR_MSG_PUT_FAIL;
            goto err;
        }

        unicast_msg = (homi_genl_unicast_msg_s *)payload_hdr;
        unicast_msg->msg_type = 0;
        unicast_msg->op = 0;
        unicast_msg->args_length = HOMI_GENL_UNICAST_MSG_ARGS_SIZE(send_data) + data_desc->len;
        unicast_msg->send_data.msg_seq = data_desc->msg_seq;
        unicast_msg->send_data.blk_num = data_desc->blk_num;
        unicast_msg->send_data.blk_seq = data_desc->blk_seq;
        unicast_msg->send_data.reserved = 0;
        unicast_msg->send_data.length = data_desc->len;

        ret = memcpy_s(unicast_msg->send_data.data,unicast_msg->send_data.length,
            data_desc->data, data_desc->len);
        if (ret) {
            bsp_err("memcpy_s unicast send data fail, ret=0x%x\n", ret);
            goto err;
        }

        skb_put(genl_skb, payload_len);
        genlmsg_end(genl_skb, payload_hdr);
        
        ret = genlmsg_unicast(ctrl_info->chan_ctrl_info[chan_id].net, genl_skb,
                        ctrl_info->chan_ctrl_info[chan_id].port_id);
        if (ret < 0) {
            bsp_info("genl unicast fail, ret=%d, chan_id = 0x%x", ret, chan_id);
            return ret;
        }
        return 0;

    err:
        dev_kfree_skb_any(genl_skb);
        return ret;

}

int bsp_homi_genl_send_data(homi_genl_chan_e chan_id,u8 *buf, u32 len)
{
        homi_genl_data_desc_s data_desc;
        unsigned int block_num;
        unsigned int tail_size;
        unsigned int i;

        if (chan_id >= GENL_CHAN_BUTT) {
            return -1;
        }

        osl_sem_down(&g_homi_ctrl_info.chan_ctrl_info[chan_id].chan_sem);

        block_num = len / HOMI_GENL_DATA_SIZE;
        tail_size = len % HOMI_GENL_DATA_SIZE;

        (void)memset_s(&data_desc, sizeof(homi_genl_data_desc_s), 0, sizeof(data_desc));

        data_desc.blk_num = block_num;
        data_desc.reserved = 0;
        data_desc.data = buf;
        data_desc.len = HOMI_GENL_DATA_SIZE;

        for (i = 0; i < block_num; i++) {
            homi_genl_send_fragment_data(chan_id, &data_desc);
            data_desc.data += HOMI_GENL_DATA_SIZE;
            data_desc.blk_seq++;
        }

        if (tail_size) {
          data_desc.len = tail_size;
          homi_genl_send_fragment_data(chan_id, &data_desc); 
        }

        osl_sem_up(&g_homi_ctrl_info.chan_ctrl_info[chan_id].chan_sem);
        return 0;
}

int homi_genl_init(void)
{
    int ret;
    int i;

    g_homi_ctrl_info.family.hdrsize = 0;
    g_homi_ctrl_info.family.version = HOMI_GENL_VERSION;
    g_homi_ctrl_info.family.ops = g_homi_genl_ops;
    g_homi_ctrl_info.family.n_ops = ARRAY_SIZE(g_homi_genl_ops);
    ret = strncpy_s(g_homi_ctrl_info.family.name, sizeof(g_homi_ctrl_info.family.name),
        HOMI_GENL_NAME, sizeof(HOMI_GENL_NAME));
    if (ret) {
        bsp_err("strcpy genl name failed, ret=0x%x\n", ret);
        return ret;
    }

    ret = genl_register_family(&g_homi_ctrl_info.family);
    if (ret) {
        bsp_err("genl_register_family failed, ret=0x%x\n", ret);
    }

    for (i = GENL_CHAN_CHR; i < GENL_CHAN_BUTT; i++) {
        osl_sem_init(1, &g_homi_ctrl_info.chan_ctrl_info[i].chan_sem);
    }

    return ret;
}




