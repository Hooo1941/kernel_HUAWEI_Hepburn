/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2019-2020. All rights reserved.
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
 *
 */
#ifndef __BSP_HOMI_H__
#define __BSP_HOMI_H__

#include <linux/netdevice.h>
#include <linux/module.h>
#include <net/genetlink.h>
#include <osl_types.h>
#include <osl_list.h>
#include <osl_sem.h>

/* removing chan gc time:5s */
#define HOMI_CH_GC_TIME (5000)

struct homi_port;
struct homi_chan;

enum homi_port_type {
    HOMI_PORT_USB = 0,
    HOMI_PORT_NETLINK,
    HOMI_PORT_SOCKET,
    HOMI_PORT_MAX
};

enum homi_chan_type {
    HOMI_CH_USB_BASE = 0,
    HOMI_CH_USB_GPS = HOMI_CH_USB_BASE,
    HOMI_CH_USB_4G,
    HOMI_CH_USB_MPROBE,
    HOMI_CH_USB_END = HOMI_CH_USB_MPROBE,   /* usb chan type end */

    HOMI_CH_SK_BASE = 256,
    HOMI_CH_SK_3000 = HOMI_CH_SK_BASE,   /* diag cfg socket chan */
    HOMI_CH_SK_20249,   /* at socket chan */
    HOMI_CH_SK_20250,   /* cbt socket chan */
    HOMI_CH_SK_20253,   /* diag ind socket chan, bind diag cfg chan */
    HOMI_CH_SK_END = HOMI_CH_SK_20253,   /* socket chan type end */

    HOMI_CH_NL_BASE = 512,
    HOMI_CH_NL_29 = HOMI_CH_NL_BASE,   /* diag ind netlink chan */
    HOMI_CH_NL_31,   /* diag cfg netlink chan */
    HOMI_CH_NL_END = HOMI_CH_NL_31,   /* netlink chan type end */

    HOMI_CH_MAX
};

enum homi_chan_state {
    HOMI_CHSTATE_INITING = 0,
    HOMI_CHSTATE_DISCONNECTED,
    HOMI_CHSTATE_CONNECTED,
    HOMI_CHSTATE_SUSPEND,
    HOMI_CHSTATE_REMOVED,
    HOMI_CHSTATE_MAX
};

struct homi_chan_ops {
    int (*init)(struct homi_chan *chan);
    int (*send)(struct homi_chan *chan, void *data, unsigned int len);
    int (*release)(struct homi_chan *chan);   /* can't sleep in function */
};

/* homi chan callback for user */
struct homi_chan_cb {
    void (*sendcb)(void *data, int len);
    void (*recvcb)(struct homi_chan *chan, void *data, int len);
    void (*eventcb)(enum homi_port_type ptype, enum homi_chan_state state);
};

enum homi_chan_direction {
    HOMI_CHAN_SEND = 0,
    HOMI_CHAN_RECV = 1,
    HOMI_CHAN_SDRV = 2,
    HOMI_CHAN_DIRMAX
};

struct homi_chan {
    int flag;   /* homi chan flag for identify */
    int refcnt;   /* reference count in use */
    spinlock_t ref_lock;
    enum homi_chan_type type;
    enum homi_chan_state state;
    osl_sem_id chsem;
    osl_sem_id rdsem;
    struct list_head list;
    struct homi_port *port;
    struct homi_chan_ops *ops;
    struct homi_chan_cb cb;
    u64 fd;   /* udi/socket */
    u32 acpfd;   /* socket accept fd */
    u32 blength;   /* data buf length */
    u32 bnum;   /* data buf num */
    void *private;
};

struct homi_cfg {
    enum homi_port_type ptype;
    enum homi_chan_type ctype;
    u32 blength;   /* data buf length */
    u32 bnum;   /* data buf num */
    struct homi_chan_cb cb;
};

struct homi_port {
    enum homi_port_type type;
    struct list_head list;   /* chan list */
    spinlock_t lock;   /* for list add */
    struct homi_ctrl *hctrl;
    struct homi_port *next;
    struct homi_chan *(*alloc)(struct homi_port *port, struct homi_cfg *cfg);
    int (*release)(struct homi_port *port, struct homi_chan *chan);
};

#define HOMI_GENL_HDR_LEN (NLMSG_HDRLEN + GENL_HDRLEN)

typedef enum {
    GENL_CMD_UNSPEC,
    GENL_CMD_CHR_RCV_MSG,
    GENL_CMD_BUTT,
} homi_genl_cmd_e;

typedef enum {
    GENL_CHAN_CHR,
    GENL_CHAN_BUTT,
} homi_genl_chan_e;

typedef struct {
    struct net *net;
    u32 port_id;
    osl_sem_id chan_sem;
} genl_chan_ctrl_s;

typedef struct {
    struct genl_family family;
    genl_chan_ctrl_s chan_ctrl_info[GENL_CHAN_BUTT];
} homi_genl_ctrl_s;

typedef int (*homi_genl_doit_handler)(struct sk_buff *skb, struct genl_info *info);

#ifndef CONFIG_HOMI_DISABLE
struct homi_chan *bsp_homi_chan_alloc(struct homi_cfg *hcfg);
int bsp_homi_chan_release(struct homi_chan *chan);
void bsp_homi_genl_set_port_info(homi_genl_chan_e chan_id, struct genl_info *info);
int bsp_homi_genl_rcv_para_check(struct sk_buff *skb, struct genl_info *info);
int bsp_homi_genl_send_data(homi_genl_chan_e chan_id, u8 *buf, u32 len);
int bsp_homi_genl_chan_register(homi_genl_chan_e chan_id, homi_genl_cmd_e cmd, homi_genl_doit_handler doit_handler);
#else
static inline struct homi_chan *bsp_homi_chan_alloc(struct homi_cfg *hcfg)
{
    return NULL;
}

static inline int bsp_homi_chan_release(struct homi_chan *chan)
{
    return 0;
}
#endif

#endif
