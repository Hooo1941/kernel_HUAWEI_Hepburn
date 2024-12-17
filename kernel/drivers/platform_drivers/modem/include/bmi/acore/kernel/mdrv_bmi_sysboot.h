/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2012-2023. All rights reserved.
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
 * @brief   飞龙架构2.0 sysboot模块对外头文件
 * @file    mdrv_bmi_adc.h
 * @version v1.0
 * @date    2023.07.10
 * @note    修改记录(版本号|修订日期|说明)
 * <ul><li>v1.0|xxx.xx.xx|创建文件</li></ul>
 * @since
 */
#ifndef __MDRV_BMI_SYSBOOT_H__
#define __MDRV_BMI_SYSBOOT_H__

#include <linux/list.h>
#include <linux/rtc.h>

/*****************************************************************************
     *                           Attention                           *
 * Description : Driver for sysboot
 * Core        : Acore、Ccore
 * Header File : the following head files need to be modified at the same time
 * : ccore/mree/plat/include/fusion/bsp_onoff.h
 * ：include/bmi/acore/kernel/mdrv_bmi_sysboot.h
 * : include/bmi/hiboot/mdrv_bmi_sysboot.h
 ******************************************************************************/

#define BOOT_FROM_A_PART_FLAG 0x1  // 主分区
#define BOOT_FROM_B_PART_FLAG 0x2  // 备份分区

#define DIRECT_SWITCH 0  // A/A分区切换类型
#define REBOOT_SWITCH 1  // A/B分区切换类型

/**
 * @brief 关机原因
 */
typedef enum DRV_SHUTDOWN_REASON_tag_s {
    DRV_SHUTDOWN_POWER_KEY,           /**< 长按 Power 键关机 */
    DRV_SHUTDOWN_BATTERY_ERROR,       /**< 电池异常 */
    DRV_SHUTDOWN_LOW_BATTERY,         /**< 电池电量低 */
    DRV_SHUTDOWN_TEMPERATURE_PROTECT, /**< 过温保护关机 */
    DRV_SHUTDOWN_CHARGE_REMOVE,       /**< 关机充电模式下，拔除充电器 */
    DRV_SHUTDOWN_UPDATE,              /**< 关机并进入升级模式 */
    DRV_SHUTDOWN_RESET,               /**< 系统软复位 */
    DRV_SHUTDOWN_BUTT
} drv_shutdown_reason_e;

/**
 * @brief 下电重启标记
 */
typedef enum {
    POWER_OFF_REASON_NORMAL = 0x504F535E,
    POWER_OFF_REASON_BAD_BATTERY = 0x504F4242,
    POWER_OFF_REASON_LOW_BATTERY = 0x504F424C,
    POWER_OFF_REASON_OVER_TEMP = 0x504F4F54,
    POWER_OFF_REASON_RM_CHARGE = 0x504F4235,
    POWER_OFF_REASON_UPDATE = 0x504F4F55,
    POWER_OFF_REASON_RESET = 0x504F4D56,
    POWER_OFF_REASON_BY_CHARGE = 0x5046424C, /* reset and entry power off charging */
    POWER_OFF_REASON_INVALID = 0xFFFFFFFF
} power_off_reboot_flag_e;

/*
 * @brief 重启原因枚举
 */
typedef enum {
    POWER_REBOOT_CMD_WARMRESET = 0x52454348,
    POWER_REBOOT_CMD_BOOTLOADER = 0x5245424F,
    POWER_REBOOT_CMD_RECOVERY = 0x52455245,
    POWER_REBOOT_CMD_FACTORY = 0x52455D51,
    POWER_REBOOT_CMD_UPDATE = 0x52455344,
    POWER_REBOOT_CMD_BUTT = 0xFFFFFFFF
} power_reboot_cmd_e;

/**
 * @brief 开机原因枚举值定义
 */
typedef enum {
    POWER_ON_REASON_START = 0x45462310,             /**< start magic number */
    POWER_ON_REASON_CHARGE = POWER_ON_REASON_START, /**< code boot: include USB/DC */
    POWER_ON_REASON_POWER_KEY = 0x45462311,         /**< code boot: power key */
    POWER_ON_REASON_NORMAL_REBOOT = 0x45462312,     /**< warm boot: normal reboot */
    POWER_ON_REASON_ACORE_WDT = 0x45462313,         /**< warm boot: A core watch dog */
    POWER_ON_REASON_CCORE_WDT = 0x45462314,         /**< warm boot: C core watch dog */
    POWER_ON_REASON_MCORE_WDT = 0x45462315,         /**< warm boot: M core watch dog */
    POWER_ON_REASON_ACORE_EXCEPTION = 0x45462316,   /**< warm boot: A core Other anomalies */
    POWER_ON_REASON_CCORE_EXCEPTION = 0x45462317,   /**< warm boot: C core Other anomalies */
    POWER_ON_REASON_MCORE_EXCEPTION = 0x45462318,   /**< warm boot: M core Other anomalies */
    POWER_ON_REASON_UNKNOW_EXCEPTION = 0x45462319,  /**< unkwon reason, such as Hardware reset */
    POWER_ON_REASON_BUTT = 0x4546231a
} power_on_start_reason_e;

/**
 * @brief 下电行为枚举值
 */
typedef enum {
    DO_POWER_OFF = 0,
    DO_SOFT_POWER_OFF,
    DO_REBOOT,
    DO_NOTHING,
    DO_INVALID,
} shutdown_result_action_e;

/**
 * @brief 系统启动模式枚举
 */
typedef enum {
    SYSBOOT_BOOT_MODE_ERROR = -1,
    SYSBOOT_BOOT_MODE_STUB = 0,
    SYSBOOT_BOOT_MODE_FMC = 0,
    SYSBOOT_BOOT_MODE_EMMC0,
    SYSBOOT_BOOT_MODE_FMC_DOUBLEBOOT,
    SYSBOOT_BOOT_MODE_AP_PCIE,
    SYSBOOT_BOOT_MODE_AP_UART,
    SYSBOOT_BOOT_MODE_AP_PCIE1,
    SYSBOOT_BOOT_MODE_FMC_PAD,
} sysboot_boot_mode_e;

/**
 * @brief 开机模式枚举值定义
 * 注意：该枚举定义与mbb上层应用保持一致，为保证上层应用的前向兼容，各模式枚举值不能变更
 */
typedef enum {
    DRV_START_MODE_EXCEPTION,                     /**< 异常开机模式，此时大部分业务都不启动 */
    DRV_START_MODE_NORMAL,                        /**< 正常开机模式，所有业务启动 */
    DRV_START_MODE_CHARGING,                      /**< 充电开机模式, ps 等业务不启动 */
    DRV_START_MODE_NOAPP,                         /**< SD卡升级第二次重启后不启动APP */
    DRV_START_MODE_UPDATE = DRV_START_MODE_NOAPP, /**< 升级模式 */
    DRV_START_MODE_MODEM,
    DRV_START_MODE_BUTT
} drv_start_mode_e;

/**
 * @brief modem id索引枚举值
 */
typedef enum {
    MODEM_ID0 = 0,
    MODEM_ID1,
    MODEM_ID2,
    MODEM_ID_INVALID,
} modem_idx_e;

/**
 * @brief modem状态枚举值
 */
typedef enum {
    MODEM_NOT_READY = 0,
    MODEM_READY,
    MODEM_INIT_OK,
    MODEM_STATE_INVALID,
} modem_state_e;

/**
 * @brief 系统所支持的启动状态枚举值定义
 */
typedef enum {
    MODEM_INIT_DONE = 0, /* modem ccore module init初始化完成 */
    MODEM_BOOT_DONE, /* modem 启动完成，MBB上存在该状态，手机上不涉及 */
    MODEM_BOOT_FAIL, /* modem启动失败，MBB上存在该状态，手机上不涉及 */
    AP_BOOT_FAIL, /* AP启动失败，MBB上存在该状态，手机上不涉及 */
    SYSBOOT_STAGE_MAX,
} modem_sysboot_stage_e;

/**
 * @brief 系统启动状态通知结构信息，在modem_sysboot_stage_e的每一个启动阶段都会调用一次func
 */
typedef void (*modem_sysboot_notifier_func)(modem_sysboot_stage_e stage);
typedef struct {
    struct list_head list;
    modem_sysboot_notifier_func func;
} modem_sysboot_notifier_s;

/*****************************************************************************
     *                           Attention                           *
 *****************************************************************************/

/**
 * @brief shutdown_judge_table 输入条件结构
 */
typedef struct {
    unsigned long shutdown_reason;  /**< 下电原因，取值见drv_shutdown_reason_e */
    unsigned long the_charger_state; /**< 是否处于充电状态：1--是，0--否 */
    unsigned long is_power_off_charging; /**< 是否为关机充电模式：1--是，0--否 */
} input_condition_s;

/**
 * @brief shutdown_judge_table 输出信息结构
 */
typedef struct {
    unsigned long result; /**< 下电原因，取值见shutdown_result_action_e */
    unsigned long reboot_reason; /**< 下电重启原因，取值见power_off_reboot_flag_e */
} output_condition_s;

/**
 * @brief shutdown_judge_table 结构
 */
typedef struct {
    /* input condition */
    input_condition_s factor;
    /* output information */
    output_condition_s out;
} shutdown_class_s;

/**
 * @brief 复位Modem子系统。
 *
 * @par 描述:
 * 复位Modem子系统。
 *
 * @attention:
 * <ul><li>1. P在phone形态下，该函数实现对Modem子系统（C核）的单独复位；MBB形态下，则是对全系统复位。</li></ul>
 * <ul><li>2. ccore和acore都可以调用，最终结果一致，内部函数实现有差异。</li></ul>
 * <ul><li>3. 异常情况下，如果需要重启modem时，请使用system_error接口；严禁直接调用该接口。</li></ul>
 *
 * @par 依赖:
 * <ul><li>mdrv_bmi_sysboot.h：该接口声明所在的头文件。</li></ul>
 */
void mdrv_sysboot_restart(void);

/**
 * @brief 获取开机模式,该接口只在mbb平台生效，phone平台接口打桩。
 *
 * @par 描述:
 * 获取开机模式信息。
 *
 * @retval drv_start_mode_e,开机模式枚举值
 *
 * @par 依赖:
 * <ul><li>mdrv_bmi_sysboot.h：该接口声明所在的头文件。</li></ul>
 */
drv_start_mode_e mdrv_sysboot_get_bootmode(void);

/**
 * @brief power_off回调注册，该接口只在mbb平台生效，phone平台接口打桩。
 *
 * @par 描述:
 * 用于注册软关机处理的回调函数
 * 1. 供MBB模块执行软关机处理流程
 * 2. 注册的回调函数在锁中断条件下执行
 * 3. 注册的回调函数原型必须是void (*hook)(shutdown_result_action_e)
 *
 * @retval void
 *
 * @par 依赖:
 * <ul><li>mdrv_bmi_sysboot.h：该接口声明所在的头文件。</li></ul>
 *
 * @see shutdown_result_action_e
 */
void mdrv_sysboot_power_off_cb_register(void (*hook)(shutdown_result_action_e));

/**
 * @brief 获取指定modem_id对应的状态
 *
 * @param[in]  modem_id, modem_id编号
 *
 * @retval >=0，执行成功，表示modem_id对应的状态
 * @retval < 0,  执行失败
 *
 * @see modem状态见：modem_state_e、modem_idx_e
 */
int mdrv_sysboot_get_modem_state(modem_idx_e modem_id);

/**
 * @brief 获取power_on时间, 该接口只在mbb平台生效，phone平台接口打桩。
 *
 * @param[out]  time, 返回power_on时间
 * @retval 0，执行成功
 * @retval != 0,  执行失败
 */
int mdrv_sysboot_get_power_on_time(struct rtc_time *time);

/**
 * @brief 注册系统启动状态通知接口
 *
 * @attention
 * <ul><li>入参notifier的生命周期在系统运行过程一直有效，否则功能会异常
 * <ul><li>不能阻塞流程往下执行
 *
 * @param[in]  notifier, 启动状态通知接口信息
 *
 * @retval 0, 执行成功
 * @retval 非0, 执行失败
 *
 * @see modem_sysboot_notifier_s、modem_sysboot_stage_e
 */
int mdrv_sysboot_notifier_register(modem_sysboot_notifier_s *notifier);

/**
 * @brief 设置重启原因
 *
 * @param[in]  cmd, 重启原因，取值见power_reboot_cmd_e；
 * @par 依赖:
 * <ul><li>mdrv_bmi_sysboot.h：该接口声明所在的头文件。</li></ul>
 *
 * @retval 0，执行成功。
 * @retval 非0，执行失败。
 *
 * @see table结构体见：power_reboot_cmd_e
 */
int mdrv_sysboot_set_power_reboot_cmd(power_reboot_cmd_e cmd);

/**
 * @brief 注册shutdown_judge_table
 *
 * @par 描述:
 * 该接口只支持一次，不支持重复注册；
 *
 * @attention:
 * <ul><li>参数table为全局变量或从堆区申请空间，生命周期在整个系统器件都有效。</li></ul>
 *
 * @param[in]  table, shutdown_judge_table buf地址；
 *             table_array_size, table大小，单位为字节；
 *
 * @retval 0，写成功。
 * @retval 非0，写失败。
 *
 * @par 依赖:
 * <ul><li>mdrv_bmi_sysboot.h：该接口声明所在的头文件。</li></ul>
 *
 *
 * @see table结构体见：shutdown_class_s
 */
int mdrv_sysboot_register_shutdown_judge_table(shutdown_class_s *table, unsigned int table_array_size);

/**
 * @brief 获取系统启动模式
 *
 * @par 依赖:
 * <ul><li>mdrv_bmi_sysboot.h：该接口声明所在的头文件。</li></ul>
 *
 * @see table结构体见：sysboot_boot_mode_e
 */
sysboot_boot_mode_e mdrv_sysboot_get_boot_mode(void);

/**
 * @brief 系统关机处理
 *
 * @par 描述:
 * 立刻执行关机处理或延时指定时间后执行关机处理；
 *
 * @param[in]  en_reason, 下电原因，取值见drv_shutdown_reason_e；
 *             delay_in_ms, 执行关机流程的延时时间；
 *
 * @retval 0，执行成功。
 * @retval 非0，执行失败。
 *
 * @see 下电原因见：drv_shutdown_reason_e
 */
int mdrv_sysboot_shut_down(drv_shutdown_reason_e en_reason, unsigned int delay_in_ms);

/**
 * @brief 获取启动分区,
 *
 * @retval 1，主分区。
 * @retval 2，备份分区。
 *
 * @see BOOT_FROM_A_PART_FLAG, BOOT_FROM_B_PART_FLAG
 */
unsigned int mdrv_sysboot_get_boot_part(void);

/**
 * @brief 获取分区切换类型,
 *
 * @retval 0，直接切换分区（A/A）
 * @retval 1，重启切换分区（A/B）
 *
 * @see DIRECT_SWITCH, REBOOT_SWITCH
 */
unsigned int mdrv_sysboot_get_switch_type(void);

#endif
