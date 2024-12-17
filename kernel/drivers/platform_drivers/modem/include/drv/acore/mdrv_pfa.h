/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2012-2015. All rights reserved.
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

/**
 *  @brief   acore对外头文件，融合架构不对外提供。
 *  @file    mdrv_pfa.h
 *  @version v1.0
 *  @date    2020.03.13
 *  @note    修改记录(版本号|修订日期|说明)
 *  <ul><li>v1.0|2020.03.13|增加说明：该头文件融合架构不对外提供</li></ul>
 *  @since
 */
#ifndef __MDRV_PFA_H__
#define __MDRV_PFA_H__

#include <linux/skbuff.h>
#include <linux/if_ether.h>

/**
 * @brief 关闭PFA硬转功能 *
 * @par 描述:
 * 该接口提供给协议栈使用，用于关闭PFA硬转功能
 * @attention
 * <ul><li> NA </li></ul>
 * @param[in] void
 * @retval void
 * @par 依赖:
 * <ul><li>mdrv_pfa.h：该接口声明所在的头文件。</li></ul>
 * @see NA
 */
void mdrv_pfa_transfer_pause(void);

/**
 * @brief 打开PFA硬转功能 *
 * @par 描述:
 * 该接口提供给协议栈使用，用于打开PFA硬转功能
 * @attention
 * <ul><li> NA </li></ul>
 * @param[in] void
 * @retval void
 * @par 依赖:
 * <ul><li>mdrv_pfa.h：该接口声明所在的头文件。</li></ul>
 * @see NA
 */
void mdrv_pfa_transfer_restart(void);

union pfa_tft_desc_attr_u {
    struct {
        unsigned int int_en : 1;
        unsigned int mode : 2;
        unsigned int cd_en : 1;
        unsigned int dl_data_sec_n : 1;
        unsigned int dl_rqi : 1;
        unsigned int reserved0 : 2;
        unsigned int fc_head : 4;
        unsigned int irq_en : 1;
        unsigned int push_en : 1;
        unsigned int dl_qos_flowid7 : 1;
        unsigned int high_pri_flag : 1;
        unsigned int pkt_len : 16;
    } bits;
    unsigned int u32;
};

/* IDs region of basic descriptor */
union pfa_tft_desc_id_u {
    struct {
        unsigned int pdu_session_id : 8;
        unsigned int drb_id : 8;
        unsigned int modem_id : 2;
        unsigned int rat_id : 3;
        unsigned int refl_fc_head : 4;
        unsigned int dl_qos_flow_id : 7;
    } bits;
    unsigned int u32;
};

enum pfa_tft_fchead_e {
    PFA_TFT_MODEM0_ULFC = 0,
    PFA_TFT_MODEM1_ULFC,
    PFA_TFT_MODEM2_ULFC,
    PFA_TFT_MODEM0_DLFC,
    PFA_TFT_MODEM1_DLFC,
    PFA_TFT_MODEM2_DLFC,
    PFA_TFT_LTEV_DLFC,
    PFA_TFT_LTEV_ULFC,
    PFA_TFT_ETH_ULFC,
    PFA_TFT_ETH_DLFC,
    PFA_TFT_FCHEAD_BOTTOM,
};

#define WAN_DRA_OWN BIT(0)
#define WAN_PFA_L2ADDR_FROM_CB BIT(3)

enum pfa_td_mode {
    PFA_TD_MOD_COPY_AND_FILTER = 0,
    PFA_TD_MOD_FILTER_ONLY,
    PFA_TD_MOD_COPY_ONLY,
    PFA_TD_MOD_BOTTOM,
};
/* PFA_TFT info for PFA BD  */
struct pfa_tft_info_s {
    unsigned int pdu_session_id:8;
    unsigned int fc_head:4;
    unsigned int modem_id:2;
    unsigned int higi_pri_flag:1;
    unsigned int parse_en:1;
    unsigned int td_mode_en:1;
    enum pfa_td_mode td_mode:2;
    unsigned int v2x_ind:1;
    unsigned int pkt_ind:1; // 1’b0 IP;  1‘b1 eth
    unsigned int with_mac_pdu:1; // bypass to ipf, whether match mac_ursp table:1’b0 no;1‘b1 yes
    unsigned int tft_filter_ind:1; // V200 tft filter enable
    unsigned int copy_mode:1; // V200 sne copy enalbe
    unsigned int bypass_dst_pkt_ind:2; // 1’b0 eth;  1‘b1 ip; ohter
    unsigned int reserve:6;
};

union pfa_result_u {
    struct {
        unsigned int acpu_pkt_rsn:5;
        unsigned int rsv:27;
    } bits;
    unsigned int u32;
};

/* map of skb->CB in wan_tx */
struct wan_info_s {
    struct pfa_tft_info_s info;
    unsigned int userfield0;
    unsigned int userfield1;
    unsigned int userfield2;
};

union pfa_tft_result_u {
    struct {
        unsigned int bid_qosid          :8;
        unsigned int pf_type            :2;
        unsigned int ff_type            :1;
        unsigned int version_err        :1;
        unsigned int head_len_err       :1;
        unsigned int bd_pkt_noeq        :1;
        unsigned int pkt_parse_err      :1;
        unsigned int bd_cd_noeq         :1;
        unsigned int pkt_len_err        :1;
        unsigned int modem_id           :2;
        unsigned int reserved           :1;
        unsigned int dl_special_flag    :1;
        unsigned int no_tcp_udp         :1;
        unsigned int ip_type            :1;
        unsigned int to_acpu_flag       :1;
        unsigned int pdu_session_id     :8;
    } bits;
    struct {
        unsigned int bid_qosid          :7;
        unsigned int rdq_sel          :2;
        unsigned int pf_type            :2;
        unsigned int ff_type            :1;
        unsigned int version_err        :1;
        unsigned int head_len_err       :1;
        unsigned int bd_pkt_noeq        :1;
        unsigned int pkt_parse_err      :1;
        unsigned int bd_cd_noeq         :1;
        unsigned int pkt_len_err        :1;
        unsigned int modem_id           :2;
        unsigned int dl_special_flag    :1;
        unsigned int no_tcp_udp         :1;
        unsigned int ip_type            :1;
        unsigned int to_acpu_flag       :1;
        unsigned int pdu_session_id     :8;
    } bits_v100;
    unsigned int u32;
};

union packet_info_u {
    struct {
        unsigned int unmapped        :8;
        unsigned int l4_proto           :8;
        unsigned int ip_proto           :4;
        unsigned int l2_hdr_offeset  :5;
        unsigned int is_accable        :1;
        unsigned int net_id          :6;
    } bits;
    unsigned int u32;
};

/* map of skb->CB in wan_rx */
struct rx_cb_map_s {
    union pfa_tft_result_u pfa_tft_result;
    unsigned int userfield0;
    unsigned int userfield1;
    unsigned int userfield2;
    union packet_info_u packet_info;
    unsigned int pkt_id;
    union pfa_result_u pfa_result;
    unsigned long long dra_org;
    unsigned long long dra_l2;
};

enum wan_waterlevel_e {
    HIGH_ALERT,
    RESUME_OK,
};

/* wan callbacks */
struct wan_callback_s {
    /**
     * @brief 上送报文给ADS接口
     *
     * @par 描述:
     * 将下行到Host的报文上送到ADS模块
     *
     * @param[in]  struct sk_buff* skb，数据报文
     *
     */
    int unmap_en;
    void (*rx)(struct sk_buff* skb);
    void (*rx_complete)(void);
};

/**
 * @brief 维测打点位置
 *
 */
enum fw_pos {
    /*上行A核枚举*/
    HIMAC_UP_RCV = 0,
    USB_UP_RCV,
    PCIE_UP_RCV,
    PFA_UP_RCV,
    PFA_LAN_UP_RCV,
    RNIC_UP_RCV,
    USB_UP_FORWARD,

    /*下行A核枚举*/
    PFA_DOWN_RCV = 1024,
    RNIC_DOWN_RCV,
    PFA_LAN_DOWN_RCV,
    HIMAC_DOWN_RCV,
    USB_DOWN_RCV,
    PCIE_DOWN_RCV,
    USB_DOWN_COMP,
};

/**
 * @brief 报文唯一标识
 *
 */
struct packet_id {
    unsigned int iface_id : 4;    // HIMAC:1; USB:2; PCIE:3; SPE:4;
    unsigned int id       : 28;
};

/**
 * @brief 报文时间戳
 *
 */
struct packet_time_stamp {
    struct packet_id pkt_id;
    enum fw_pos pos;
    unsigned int time_stamp;
};

struct packet_pdcp_cnt {
    struct packet_id pkt_id;
    unsigned int pdcp_cnt;
};

typedef unsigned int (*wan_get_handle_cb)(struct sk_buff* skb);
static inline unsigned int wan_get_devid(struct sk_buff *skb)
{
    struct rx_cb_map_s *cb_map =  (struct rx_cb_map_s *)&skb->cb;
    return cb_map->userfield0 & 0xffff;
}

typedef enum pfa_version {
    PFA_VERSION_V0 = 0,
    PFA_VERSION_V100,
    PFA_VERSION_V200,
    PFA_VER_ERR,
}pfa_version_type;
/**
 * @brief 获取pfa version
 * @par 描述:
 * 无
 * @retval 芯片version。
 */
pfa_version_type mdrv_pfa_get_version(void);

int mdrv_wan_tx(struct sk_buff* skb, unsigned int dra_flag);
int mdrv_wan_callback_register(unsigned int handle, struct wan_callback_s* cb);
int mdrv_wan_get_handle_cb_register(wan_get_handle_cb cb);

void mdrv_wan_del_direct_fw_dev(unsigned int devid);
int mdrv_lan_tx(struct sk_buff *skb, unsigned int dra_flag);
int mdrv_lan_add_netdev(struct net_device *ndev, int (*lan_rx)(struct sk_buff* skb), void (*lan_rx_complete)(void));
int mdrv_lan_rm_netdev(struct net_device *ndev);

enum pfa_port_type {
    PFA_WWAN_RELAY,
    PFA_WWAN_RELAY_MAX = 7,
};
struct mdrv_pfa_port_ops {
    void (*rx)(struct sk_buff *skb, unsigned int len, void* para, unsigned int flag);
    void (*tx)(struct sk_buff *skb, void *para);
    void (*rx_complete)(void *para);
    void *para;
};
struct mdrv_pfa_port_info_s {
    struct net_device *dev;
    unsigned int port_type; /*从业务角度看到的端口需求类型*/
    struct mdrv_pfa_port_ops ops; /**/
};
/*
	申请分配端口
	1、端口分配，成功正数，返回端口号，负数失败
	2、动态注册，只有拨号成功ifconfig 网卡up的时候才注册/down去注册
	3、参数包括：RD回调函数/以及回调参数，回调函数需要自行判断回调上下文
	4、驱动实现：多核软件上下文上送，单核 软中断上下文，在调用CB之前需要保证数据一致性invalid & clean Cache.
	5、参数包括：端口信息使用pfa_port_type
*/
int mdrv_pfa_open_port(struct mdrv_pfa_port_info_s* info);

/*
1、网卡down时候关闭
*/
int mdrv_pfa_close_port(int port);

/*
    1、转发表项参数，刷新行为：根据valid字段刷新，valid有效，更新，无效不变化
	2、fw_type: ETH_TYPE IPV4_TYPE IPV6_TYPE

	1）添加表项的时候调用
	2) 和open函数只能二选一，dev和pfa_port关联。只关联ipf端口，中继调用open函数，非中继用该接口
*/
enum pfa_fw_type {
    PFA_FW_TYPE_IPV4,
    PFA_FW_TYPE_IPV6,
    PFA_FW_TYPE_MAC,
    PFA_FW_TYPE_END
};
struct wan_dev_info_s {
    struct net_device* dev;
    void (*get_fw_info)(struct net_device* dev, int fw_type, struct wan_info_s* info);
};
int mdrv_wan_dev_info_register(struct wan_dev_info_s *dev_info);

int mdrv_wan_dev_info_deregister(struct wan_dev_info_s *dev_info);

#endif
