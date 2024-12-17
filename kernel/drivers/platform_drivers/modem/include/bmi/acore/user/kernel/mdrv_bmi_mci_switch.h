/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
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
 * @brief   MCI AT拦截功能对外头文件
 * @file    mdrv_bmi_mci_switch.h
 * @version v1.0
 * @date    2023.09.20
 * @note    该文件增删改需通过接口变更评审
 * <ul><li>v1.0|2023.09.20|创建文件</li></ul>
 * @since
 */

#ifndef __MDRV_BMI_MCI_SWITCH_H__
#define __MDRV_BMI_MCI_SWITCH_H__

/**
 * @brief MCI AT拦截功能设备节点名称
 */
#define MCI_SWITCH_TE "/dev/mci_te"
#define MCI_SWITCH_MT "/dev/mci_mt"

/**
* @brief 拦截机制支持通道ID
*/
enum mci_switch_chan_id {
    MCI_CHN_APP_0,
    MCI_CHN_APP_1,
    MCI_CHN_APP_2,
    MCI_CHN_APP_3,
    MCI_CHN_APP_4,
    MCI_CHN_APP_5,
    MCI_CHN_APP_6,
    MCI_CHN_APP_7,
    MCI_CHN_APP_8,
    MCI_CHN_APP_9,
    MCI_CHN_APP_10,
    MCI_CHN_APP_11,
    MCI_CHN_APP_12,
    MCI_CHN_APP_13,
    MCI_CHN_APP_14,
    MCI_CHN_APP_15,
    MCI_CHN_APP_16,
    MCI_CHN_APP_17,
    MCI_CHN_APP_18,
    MCI_CHN_APP_19,
    MCI_CHN_APP_20,
    MCI_CHN_APP_21,
    MCI_CHN_APP_22,
    MCI_CHN_APP_23,
    MCI_CHN_APP_24,
    MCI_CHN_APP_25,
    MCI_CHN_APP_26,
    MCI_CHN_APP_27,
    MCI_CHN_APP_28,
    MCI_CHN_APP_29,
    MCI_CHN_APP_30,
    MCI_CHN_APP_31,
    MCI_CHN_APP_32,
    MCI_CHN_APP_33,
    MCI_CHN_APP_34,
    MCI_CHN_APP_35,
    MCI_CHN_APP_36,
    MCI_CHN_APP_37,
    MCI_CHN_APP_38,
    MCI_CHN_APP_39,
    MCI_CHN_APP_40,
    MCI_CHN_APP_41,
    MCI_CHN_APP_42,
    MCI_CHN_APP_43,
    MCI_CHN_APP_44,
    MCI_CHN_APP_45,
    MCI_CHN_APP_46,
    MCI_CHN_APP_47,
    MCI_CHN_APP_48,
    MCI_CHN_APP_49,
    MCI_CHN_APP_50,
    MCI_CHN_APP_51,
    MCI_CHN_APP_52,
    MCI_CHN_APP_53,
    MCI_CHN_APP_54,
    MCI_CHN_APP_55,
    MCI_CHN_APP_56,
    MCI_CHN_APP_57,
    MCI_CHN_APP_58,
    MCI_CHN_APP_59,
    MCI_CHN_APP_60,
    MCI_CHN_APP_61,
    MCI_CHN_APP_62,
    MCI_CHN_APP_63,
    MCI_CHN_APP_ACT,
    MCI_CHN_APP_NMCTRL,
    MCI_CHN_APP_BASTET,
    MCI_CHN_BRIDGE,     // 透传通道
    MCI_CHN_BRIDGE1,     // 透传通道
    MCI_CHN_ATP,
    MCI_CHN_APP_TEST,
    MCI_CHN_ACM_CTRL,
    MCI_CHN_ACM_AT,
    MCI_CHN_ACM_SHELL,
    MCI_CHN_ACM_LTE_DIAG,
    MCI_CHN_ACM_OM,
    MCI_CHN_ACM_MODEM,
    MCI_CHN_ACM_GPS,
    MCI_CHN_ACM_3G_GPS,
    MCI_CHN_ACM_3G_PCVOICE,
    MCI_CHN_ACM_PCVOICE,
    MCI_CHN_ACM_SKYTONE,
    MCI_CHN_ACM_CDMA_LOG,
    MCI_CHN_HSUART,
    MCI_CHN_SOCK_AT,
    MCI_CHN_SOCKET_OM,
    MCI_CHN_PCIE_AT,
    MCI_CHN_UNET_AT,
    MCI_CHN_UNET_CTRL,
    MCI_CHN_PNET_CTRL,
    MCI_CHN_CBT,
    MCI_CHN_CMUX0,
    MCI_CHN_CMUX1,
    MCI_CHN_CMUX2,
    MCI_CHN_CMUX3,
    MCI_CHN_BOTTOM,
};

/**
* @brief 拦截机制支持通道类型
*/
enum mci_switch_chan_type {
    MCI_CHAN_TYPE_ACM,
    MCI_CHAN_TYPE_HSUART,
    MCI_CHAN_TYPE_SOCK,
    MCI_CHAN_TYPE_NETIF_CTRL,
    MCI_CHAN_TYPE_UNET_AT,
    MCI_CHAN_TYPE_PCIE_VDEV,
    MCI_CHAN_TYPE_APP,
    MCI_CHAN_TYPE_ACK,
    MCI_CHAN_TYPE_NORTIFY,
    MCI_CHAN_TYPE_MAX,
};

/**
* @brief 拦截机制命令类型
*/
enum mci_switch_cmd_type {
    SWITCH_QUERY_READY,
    SWITCH_NOTIFY_READY, /**< 参数类型为unsigned int, 0: unready, 1: ready */
    SWITCH_TRANS_DATA,
    SWITCH_WRITE_MSC,
    SWITCH_WRITE_FLOW_CTRL,
    SWITCH_WRITE_WLEN,
    SWITCH_WRITE_STP,
    SWITCH_WRITE_EPS,
    SWITCH_WRITE_BAUD,
    SWITCH_CLEAR_WRITE_BUFF,
    SWITCH_NOTIFY_WATER_LINE,
    SWITCH_NOTIFY_ESCAPE,
    SWITCH_ENABLE_ESCAPE_DETECT, /**< 参数类型为unsigned int, 0: 不使能, 1: 使能 */
    SWITCH_SET_TRANS_MODE, /**< 参数类型为unsigned int, 0: 命令模式, 1: 数据模式 */
    SWITCH_SET_PPP_INFO,
    SWITCH_ENABLE_CMUX, /**< 参数类型为unsigned int, 0: 不使能, 1: 使能 */
    SWITCH_SET_CMUX_INFO,
    SWITCH_SET_CMUX_DLC,
    SWITCH_TYPE_MAX,
};

/**
* @brief 拦截机制消息格式
*/
struct mci_switch_trans_msg {
    enum mci_switch_chan_type channel_type;
    enum mci_switch_chan_id channel_id;
    enum mci_switch_cmd_type cmd_type;
    unsigned int reserved; /* 自定义字段，便于用户使用 */
    unsigned int buffer_len;
    unsigned char buffer[0];
};

/**
* @brief 拦截节点ioctl参数类型
*/
struct mci_switch_ioctl_msg {
    enum mci_switch_chan_type channel_type;
    enum mci_switch_chan_id channel_id;
};

#define MCI_SWITCH_MAGIC 'S'
#define MCI_SWITCH_ENABLE_CMD _IOW(MCI_SWITCH_MAGIC, 0, struct mci_switch_ioctl_msg)
#define MCI_SWITCH_DISABLE_CMD _IOW(MCI_SWITCH_MAGIC, 1, struct mci_switch_ioctl_msg)

#endif