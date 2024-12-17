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
 * @brief   飞龙架构2.0 usb模块对外头文件
 * @file    mdrv_bmi_usb.h
 * @version v1.0
 * @date    2023.07.05
 * @note    该文件增删改需通过接口变更评审
 * <ul><li>v1.0|2023.07.05|创建文件</li></ul>
 * @since
 */

#ifndef _MDRV_BMI_USB_H_
#define _MDRV_BMI_USB_H_

#include <linux/usb/ch9.h>
#include <linux/if_ether.h>
#include <linux/usb/composite.h>

/**
 * @brief usb hook点枚举值
 *
 * @par 描述:
 * usb hook 点通过枚举值记录跟踪
 *
 * @attention
 * 添加usb hook 需评审，然后分配枚举值
 */
typedef enum {
    HOOK_ACM_SET_INTF_DESC,
    HOOK_NCM_SET_INTF_DESC,
    HOOK_NCM_SET_CTRL_STRING,
    HOOK_ECM_SET_INTF_DESC,
    HOOK_ACM_SETUP_HANDLER,     /* acm 配置交互流程中添加hook */
    HOOK_ETH_ECM_RESUME,        /* ecm 低功耗唤醒流程中添加hook */
    HOOK_ETH_TX_DONE,           /* 下行发送完成处理流程中添加hook */
    HOOK_ETH_TX_RET,            /* 下行填充usb request描述符流程中添加hook */
    HOOK_ETH_DISCONNECT,        /* 网卡拔出流程中添加hook */
    HOOK_ETH_CONNECT,           /* 网卡插入流程中添加hook*/
    HOOK_USB_WAKEUP_FAIL,       /* usb唤醒流程中添加hook*/
    HOOK_USB_ADP_STATUS_CHANGE, /* usb插拔事件处理流程中添加hook*/
    HOOK_USB_MODE_INIT,         /* usb识别插拔状态流程中添加hook*/
} hook_usb_index;

/**
 * @brief usb hook点的入参格式
 *
 * @par 描述:
 * usb hook点注册的终端函数的入参采用结构体封装
 *
 * @attention
 * 每个usb hook点注册的终端函数参数结构体单独添加，添加需评审，评审后不再更改
 */

/* customer function pointer */
typedef void (*cust_func)(void *);

/**
 * @brief acm_setup定制处理hook参数
 * @par func，内核结构体指针
 * @par ctrl，内核结构体指针
 * @par[out] value，输出参数
 */
typedef struct {
    struct usb_function *func;
    const struct usb_ctrlrequest *ctrl;
    int *value;
} hook_acm_setup_handler_param;

/**
 * @brief ecm resume定制化hook参数
 * @par func，内核结构体指针
 */
typedef struct {
    struct usb_function *func;
} hook_ecm_resume_param;

/**
 * @brief usb状态定制处理hook参数
 * @par usb_status，usb插拔状态
 */
typedef struct {
    int usb_status;
} hook_usb_adp_status_change_param;

/**
 * @brief usb mode init定制处理hook参数
 * @par[out] usb_status，usb插拔状态
 */
typedef struct {
    int *usb_status;
} hook_usb_mode_init_param;

/**
 * @brief acm intf desc数据、
 * @par intf_desc 接口描述符
 * @par port_num 端口号
 */
typedef struct {
    struct usb_interface_descriptor *intf_desc;
    unsigned char port_num;
} hook_acm_set_intf_desc;

/**
 * @brief ncm intf desc数据
 * @par is_mbim_mode，当前工作模式是否为mbim模式
 * @par ctrl_intf，控制接口描述符
 * @par data_intf，数据接口描述符
 */
typedef struct {
    bool is_mbim_mode;
    struct usb_interface_descriptor *ctrl_intf;
    struct usb_interface_descriptor *data_intf;
} hook_ncm_set_intf_desc;

/**
 * @brief ncm ctrl string控制接口字符串
 * @par ctrl_string，控制接口字符串
 */
typedef struct {
    struct usb_string *ctrl_string;
} hook_ncm_set_string;

/**
 * @brief ecm intf desc数据
 * @par ctrl_intf，描述控制接口字符串
 * @par data_intf，描述控制接口字符串
 * @par data_nop_intf，描述控制接口字符串
 */
typedef struct {
    struct usb_interface_descriptor *ctrl_intf;
    struct usb_interface_descriptor *data_intf;
    struct usb_interface_descriptor *data_nop_intf;
} hook_ecm_set_intf_desc;

/**
 * @brief 设置acm定制接口字符串描述符指针
 *
 * @par 描述:acm接口描述符
 * @retval =0，表示函数执行成功。
 * @retval !=0，表示执行失败。
 * @attention 函数只能调用一次
 */
int mdrv_usb_acm_set_vendor_desc(struct usb_descriptor_header *desc);

/**
 * @brief 设置ncm定制接口字符串描述符指针
 *
 * @par 描述:ncm接口描述符
 * @retval =0，表示函数执行成功。
 * @retval !=0，表示执行失败。
 */
int mdrv_usb_ncm_set_vendor_desc(struct usb_descriptor_header *desc);

/**
 * @brief 获取usb charger类型
 *
 * @retval 0 无效类型
 * @retval 1 充电
 * @retval 2 不充电
 *
 * @par 依赖:
 * <ul><li>mdrv_bmi_usb.h：该接口声明所在的头文件。</li></ul>
 */
int mdrv_usb_get_charger_type(void);

/**
 * @brief 网卡类型枚举值
 */
typedef enum usb_network_card {
    USB_NETWORK_CARD_NCM,
    USB_NETWORK_CARD_RNDIS,
    USB_NETWORK_CARD_ECM,
    USB_NETWORK_CARD_FORCE_NCM16,
    USB_NETWORK_CARD_MBIM,
    USB_NETWORK_CARD_ERR
} usb_network_card_t;

/**
 * @brief 设置网卡类型接口
 *
 * @param[in] type，网卡类型，取值见usb_network_card_t
 *
 * @par 依赖:
 * <ul><li>mdrv_bmi_usb.h：该接口声明所在的头文件。</li></ul>
 */
void mdrv_usb_network_card_type_set(usb_network_card_t type);

/**
 * @brief 获取网卡类型接口
 *
 * @retval 网卡类型，取值见usb_network_card_t
 *
 * @par 依赖:
 * <ul><li>mdrv_bmi_usb.h：该接口声明所在的头文件。</li></ul>
 */
usb_network_card_t mdrv_usb_network_card_type_get(void);

/**
 * @brief usb网卡设备绑定配置接口
 *
 * @param[in] c，usb配置实体
 * @param[in] ethaddr[ETH_ALEN]，网卡主机端mac地址，ETH_ALEN内核宏
 * @param[in] vendor_id，网卡产品id
 * @param[in] manufacturer，网卡产品名称
 * @param[in] type，网卡类型，取值见usb_network_card_t
 *
 * @retval < 0 ,绑定失败
 * @retval >= 0 ,绑定成功，返回网卡设备ID
 *
 * @par 依赖:
 * <ul><li>mdrv_bmi_usb.h：该接口声明所在的头文件。</li></ul>
 */
int mdrv_gether_bind_config(struct usb_configuration *c, unsigned char ethaddr[ETH_ALEN], unsigned int vendor_id,
    const char *manufacturer, usb_network_card_t type);

/**
 * @brief usb网卡设备解除绑定配置接口
 *
 * @param[in] id，网卡设备id
 *
 * @par 依赖:
 * <ul><li>mdrv_bmi_usb.h：该接口声明所在的头文件。</li></ul>
 */
void mdrv_gether_unbind_config(int id);

/**
 * @brief usb插拔类型枚举值
 */
enum usb_plug_type {
    IOT_USB_DEVICE_REMOVE = 0,
    IOT_USB_DEVICE_INSERT = 1,
    IOT_USB_PERIP_INSERT = 2,
    IOT_USB_PERIP_REMOVE = 3,
    IOT_USB_DEVICE_UNKNOW = 0xFF
};

/**
 * @brief usb插拔检测初始化接口
 *
 * @retval < 0，插拔检测初始化失败
 * @retval = 0，插拔检测初始化成功
 *
 * @par 依赖:
 * <ul><li>mdrv_bmi_usb.h：该接口声明所在的头文件。</li></ul>
 */
int mdrv_usb_plug_init(void);

/**
 * @brief usb插拔处理接口
 *
 * @param[in]  type, usb插拔类型，取值见usb_plug_type
 *
 * @retval < 0，USB插拔处理失败
 * @retval = 0，USB插拔处理成功
 *
 * @par 依赖:
 * <ul><li>mdrv_bmi_usb.h：该接口声明所在的头文件。</li></ul>
 */
int mdrv_usb_detect_process(enum usb_plug_type type);

/**
 * @brief acm设备类型枚举值
 *
 * @par 依赖:
 * <ul><li>mdrv_bmi_usb.h：该接口声明所在的头文件。</li></ul>
 */
enum acm_class_type {
    ACM_CLASS_CDEV,
    ACM_CLASS_TTY,
    ACM_CLASS_MODEM,
    ACM_CLASS_MAX /* last item */
};

/**
 * @brief acm端口申请接口
 *
 * @param[in] type，acm设备类型，取值见acm_class_type
 * @param[out] port_num，端口号
 *
 * @retval < 0，申请失败
 * @retval = 0，申请成功
 *
 * @par 依赖:
 * <ul><li>mdrv_bmi_usb.h：该接口声明所在的头文件。</li></ul>
 */
int mdrv_acm_setup(enum acm_class_type type, unsigned char *port_num);

/**
 * @brief acm端口释放接口
 *
 * @param[in] type，acm设备类型，取值见acm_class_type
 * @param[in] port_num，端口号
 *
 * @retval < 0，释放失败
 * @retval = 0，释放成功
 *
 * @par 依赖:
 * <ul><li>mdrv_bmi_usb.h：该接口声明所在的头文件。</li></ul>
 */
int mdrv_acm_cleanup(enum acm_class_type type, unsigned char port_num);

/**
 * @brief acm绑定配置接口
 *
 * @param[in] port_num，端口号
 * @param[in] type，acm设备类型，取值见acm_class_type
 *
 * @retval struct usb_function类型指针
 *
 * @par 依赖:
 * <ul><li>mdrv_bmi_usb.h：该接口声明所在的头文件。</li></ul>
 */
struct usb_function *mdrv_acm_bind_config(unsigned int port_num, enum acm_class_type type);

/**
 * @brief ecm上报连接状态接口
 *
 * @param[in] f，struct usb_function内核结构体
 *
 * @retval < 0，上报失败
 * @retval = 0，上报成功
 *
 * @par 依赖:
 * <ul><li>mdrv_bmi_usb.h：该接口声明所在的头文件。</li></ul>
 */
int mdrv_ecm_notify(const struct usb_function *f);

/**
 * @brief USB枚举状态上报通知链注册接口
 *
 * @param[in] nb，struct notifier_block内核结构体
 *
 * @retval < 0，注册失败
 * @retval = 0，注册成功
 *
 * @par 依赖:
 * struct notifier_block中notifier_call回调中第二个参数存在两个值：
 * USB_IOT_ENUM_DONE值为3, 表示枚举完成
 * USB_IOT_DISABLE值为0xF1, 表示设备拔出
 *
 * <ul><li>mdrv_bmi_usb.h：该接口声明所在的头文件。</li></ul>
 */
int mdrv_usb_register_notifier(struct notifier_block *nb);

/**
 * @brief USB枚举状态上报通知链去注册接口
 *
 * @param[in] nb，struct notifier_block内核结构体
 *
 * @retval < 0，注册失败
 * @retval = 0，注册成功
 *
 * @par 依赖:
 * struct notifier_block中notifier_call回调中第二个参数存在两个值：
 * USB_IOT_ENUM_DONE值为3, 表示枚举完成
 * USB_IOT_DISABLE值为0xF1, 表示设备拔出
 *
 * <ul><li>mdrv_bmi_usb.h：该接口声明所在的头文件。</li></ul>
 */
int mdrv_usb_unregister_notifier(struct notifier_block *nb);

/* usb hook register */
int mdrv_usb_hook_register(hook_usb_index idx, cust_func p_cust_func);

/* usb hook register */
int mdrv_usb_hook_unregister(hook_usb_index idx);
#endif
