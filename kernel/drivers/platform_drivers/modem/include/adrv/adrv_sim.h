/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2012-2021. All rights reserved.
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

#ifndef ADRV_SIM_H
#define ADRV_SIM_H

#define STATUS_PLUG_IN 0
#define STATUS_PLUG_OUT 1

#define MODULE_SD 0
#define MODULE_SIM 1

/* ****************************************************************************
 * function     : get_sd2jtag_status
 * description  : get sd2jtag status
 * input        : void
 * output       : void
 * ret value  : true -- sd2jtag enabled
 * false -- sd2jtag disabled
 * *************************************************************************** */
int get_sd2jtag_status(void);

/* ****************************************************************************
 * function     : get_card1_type
 * description  : get card1 type
 * input        : void
 * output       : void
 * ret value  : STATUS_SIM, STATUS_SD, STATUS_NO_CARD, STATUS_SD2JTAG
 * *************************************************************************** */
u8 get_card1_type(void);

/* ****************************************************************************
* function     : sd_sim_detect_run
* description  : run sd sim detect action
* input        : dw_mci_host      mci_host
                    status                in or out
                    current_module  sim or sd
                    need_sleep        true or false
* output       : void
* ret value  : STATUS_PLUG_IN, STATUS_PLUG_OUT
**************************************************************************** */
int sd_sim_detect_run(void *dw_mci_host, int status, int current_module, int need_sleep);

/**
 * @brief sim卡硬件处理类型
 */
typedef enum {
    SIM_HW_CARD_TYPE_INQUIRE = 0x11,           /**< 卡类型查询 */
    SIM_HW_CARD_DET_WHILE_ESIM_INQUIRE = 0x12, /**< ESIM时实卡插入查询 */
    SIM_HW_POWER_ON_CLASS_C_REQUEST = 0x21,    /**< 请求设置1.8V电压 */
    SIM_HW_POWER_ON_CLASS_B_REQUEST = 0x22,    /**< 请求设置3.0V电压 */
    SIM_HW_POWER_OFF_REQUEST = 0x23,           /**< 请求关闭电源 */
    SIM_HW_DEACTIVE_NOTIFIER = 0x31,           /**< 去激活通知 */
    SIM_HW_SET_ESIM_SWITCH = 0x41,             /**< 切换ESIM */
    SIM_HW_SET_USIM_SWITCH = 0x42,             /**< 切换USIM */
    SIM_HW_OP_TYPE_BUTT
} sci_hw_op_type_e;

/**
 * @brief SIM卡硬件处理结果
 */
typedef enum {
    SIM_HW_OP_RESULT_SUCC = 0x0,         /**< 操作成功 */
    SIM_HW_HOTPLUG_DTS_ERR,              /**< 热插拔DTS读取失败 */
    SIM_HW_HOTPLUG_DET_ERR,              /**< 热插拔DET管脚设置失败 */
    SIM_HW_HOTPLUG_HPD_ERR,              /**< 热插拔HPD配置失败 */
    SIM_HW_HOTPLUG_STATE_ERR,            /**< 热插拔状态初始化失败 */
    SIM_HW_HOTPLUG_HPD_IRQ_FAIL,         /**< 热插拔HPD中断注册失败 */
    SIM_HW_INVALID_INQUIRE_MSG = 0x100,  /**< 非法查询命令 */
    SIM_HW_CARD_TYPE_SIM = 0x101,        /**< 卡类型为SIM */
    SIM_HW_CARD_TYPE_MUXSIM,             /**< 卡类型为共卡槽SIM */
    SIM_HW_CARD_TYPE_SD,                 /**< 卡类型为SD */
    SIM_HW_CARD_TYPE_JTAG,               /**< 卡类型为JTAG */
    SIM_HW_CARD_TYPE_FAIL,               /**< 卡类型查询失败 */
    SIM_HW_CARD_EXIST_WHILE_ESIM,        /**< ESIM时存在实体卡 */
    SIM_HW_CARD_NO_EXIST_WHILE_ESIM,     /**< ESIM时不存在实体卡 */
    SIM_HW_SET_GPIO_FAIL,                /**< 设置GPIO失败 */
    SIM_HW_INVALID_POWER_MSG = 0x200,    /**< 非法上电消息 */
    SIM_HW_POWER_ON_SUCC = 0x201,        /**< 打开电源成功 */
    SIM_HW_POWER_ON_FAIL,                /**< 打开电源失败 */
    SIM_HW_POWER_OFF_SUCC,               /**< 关闭电源成功 */
    SIM_HW_POWER_OFF_FAIL,               /**< 关闭电源失败 */
    SIM_HW_REQUESR_PMU_FAIL,             /**< 请求PMU失败 */
    SIM_HW_SET_VOLTAGE_FAIL,             /**< 设置电压失败 */
    SIM_HW_INVALID_NOTIFIER_MSG = 0x300, /**< 非法通知消息 */
    SIM_HW_DEACTIVE_SUCC = 0x301,        /**< 去激活成功 */
    SIM_HW_DEACTIVE_FAIL,                /**< 去激活失败 */
    SIM_HW_INVALID_SWITCH_MSG = 0x400,   /**< 非法切换消息 */
    SIM_HW_SET_ESIM_SUCC = 0x401,        /**< 切换ESIM状态成功 */
    SIM_HW_SET_ESIM_FAIL,                /**< 切换ESIM状态失败 */
    SIM_HW_SET_USIM_SUCC,                /**< 切换USIM状态成功 */
    SIM_HW_SET_USIM_FAIL,                /**< 切换USIM状态失败 */
    SIM_HW_REQUEST_RESULT_BUTT           /**< BUTT */
} sci_hw_op_result_e;


/* 本接口用于SIM卡桥接芯片供电电源1.8V和3.0V切换配置 */
typedef enum {
    STS_SUB_CMD_REG_WRITE,
    STS_SUB_CMD_REG_READ,
} sts_reg_sub_cmd_enum;

typedef struct {
    u32 op;        /* write or read, see sts_reg_sub_cmd_enum */
    u32 slave_id;  /* 0: slave A, 1: slave B */
    u32 reg_addr;
    u32 val;       /* write vlaue, if read, ignore it */
} sts_reg_op_t;

/**
 * @brief 本接口用于SIM卡桥接芯片供电电源配置
 *
 * @par 描述:
 * 本接口用于SIM卡桥接芯片供电电源配置
 *
 * @attention
 * <ul><li>NA</li></ul>
 *
 * @param[in]  data 操作寄存器的信息
 *
 * @retval 0:  操作成功。
 * @retval 非0：操作失败。
 *
 * @par 依赖:
 * <ul><li>NA</li></ul>
 *
 * @see
 */
int sts_reg_op(sts_reg_op_t *data);

/**
 * @brief 本接口用于SIM卡硬件适配初始化
 *
 * @par 描述:
 * 本接口用于SIM卡硬件适配初始化
 *
 * @attention
 * <ul><li>NA</li></ul>
 *
 * @param[in]  sim_id 卡槽ID
 * @param[in]  dev 当前使用的设备
 *
 * @retval 0:  初始化成功。
 * @retval 非0：初始化失败。
 *
 * @par 依赖:
 * <ul><li>NA</li></ul>
 *
 * @see
 */
int sim_hwadp_init(u32 sim_id, struct device *dev);

/**
 * @brief 本接口用于SIM卡硬件适配消息处理
 *
 * @par 描述:
 * 本接口用于SIM卡硬件适配消息处理
 *
 * @attention
 * <ul><li>NA</li></ul>
 *
 * @param[in]  sim_id 卡槽ID
 * @param[in]  msg_id 消息类型，sci_hw_op_type_e
 *
 * @retval 消息处理结果，sci_hw_op_result_e
 *
 * @par 依赖:
 * <ul><li>NA</li></ul>
 *
 * @see sci_hw_op_type_e, sci_hw_op_result_e
 */
sci_hw_op_result_e sim_hwadp_handler(u32 sim_id, sci_hw_op_type_e msg_id);

/**
 * @brief sim卡硬件调试信息
 */
struct sim_hw_debug_info {
    /* 3: for reset, clock and data pins */
    u32 iomux_status[3]; /**< 当前iomux状态 */
    u32 power_status;    /**< 当前电源状态 */
    u32 event_len;       /**< 事件队列深度 */
    u32 data[];          /**< 事件队列 */
};

/**
 * @brief 本接口用于获取SIM卡硬件调试信息
 *
 * @par 描述:
 * 本接口用于获取SIM卡硬件调试信息
 *
 * @attention
 * <ul><li>NA</li></ul>
 *
 * @param[in]  sim_id 卡槽ID
 * @param[out]  dbg_info sim卡硬件调试信息
 *
 * @retval 0:  获取成功。
 * @retval 非0：获取失败。
 *
 * @par 依赖:
 * <ul><li>NA</li></ul>
 *
 * @see
 */
int sim_get_hw_debug_info(u32 sim_id, struct sim_hw_debug_info *dbg_info);

#endif /* ADRV_H */
