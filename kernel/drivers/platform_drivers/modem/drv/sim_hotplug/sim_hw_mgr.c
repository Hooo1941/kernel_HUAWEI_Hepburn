/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2012-2020. All rights reserved.
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
#include "sim_hw_mgr.h"
#include "sim_hotplug.h"
#include "securec.h"
#include <adrv_pmu.h>
#include <adrv_sim.h>
#include <mdrv_sysboot.h>
#include <bsp_print.h>
#include <bsp_reset.h>

#undef THIS_MODU
#define THIS_MODU mod_simhotplug

char g_sci_msg_power_type[SIM_POWER_MSG_BUTT + 1][0x12] = {
        "RESV",
        "POWER_ON_CLASS_C",
        "POWER_ON_CLASS_B",
        "POWER_OFF",
        "UNKOWN"
};

char *sim_msg_power_type(u32 msg_value)
{
    if (msg_value >= SIM_POWER_ON_CLASS_C_REQUEST && msg_value < SIM_POWER_MSG_BUTT) {
        return g_sci_msg_power_type[msg_value];
    } else {
        return g_sci_msg_power_type[SIM_POWER_MSG_BUTT];
    }
}

/* chip connect
 *    pmu --·-------------.------------.
 *    soc     \            \            \
 * ·-------·   \            \        ·-------·
 * |  sci0 | --slaveA--sts--slaveB-- |  SIM0 |
 * ·-------·                         ·-------·
 * |  sci1 | ----------------------- |  SIM1 |
 * ·-------·                         ·-------·
 **/
/**
 * bit0 usim0的IO模式选择
 * 1'b0:3.0V模式
 * 1'b1:1.8V模式
 **/
void sim_pmu_set_bridge_volt(struct sim_platform_info *info, s32 vol)
{
    int ret;
    int chipid;
    sts_reg_op_t bridge_chip_reg = { 0 };

    if (info->bridge_ctrl.enable != 1) {
        return;
    }
    if (info->sim_id != 0) {
        return;
    }

    bridge_chip_reg.reg_addr = info->bridge_ctrl.reg;

    mdelay(1);

    for (chipid = 0; chipid < 0x2; chipid++) {
        bridge_chip_reg.slave_id = chipid;

        bridge_chip_reg.op = STS_SUB_CMD_REG_READ;
        ret = sts_reg_op(&bridge_chip_reg);
        if (ret) {
            sim_log_err("chip%d reg read fail\n", chipid);
            return;
        }
        sim_log_err("chip%d get reg val 0x%x\n", chipid, bridge_chip_reg.val);
        if (vol == SIM_VCC_1P8V) {
            bridge_chip_reg.val |= (1 << info->bridge_ctrl.bit);
        } else {
            bridge_chip_reg.val &= ~(1 << info->bridge_ctrl.bit);
        }
        sim_log_err("chip%d set reg val 0x%x\n", chipid, bridge_chip_reg.val);
        bridge_chip_reg.op = STS_SUB_CMD_REG_WRITE;
        ret = sts_reg_op(&bridge_chip_reg);
        if (ret) {
            sim_log_err("chip%d reg write fail\n", chipid);
            return;
        }
    }
}

void sim_pmu_bridge_init(struct sim_platform_info *info)
{
    int ret;

    ret = of_property_read_u32(info->spmidev->of_node, "sim_bridge_en", (u32 *)&info->bridge_ctrl.enable);
    if (ret == 0) {
        ret = of_property_read_u32_array(info->spmidev->of_node, "sim_bridge_ctrl_reg",
            (u32 *)&info->bridge_ctrl.reg, 0x2);
        if (ret) {
            sim_log_err("skip bridge chip get, ret: %d\n", ret);
            info->bridge_ctrl.reg = 0;
            info->bridge_ctrl.bit = 0;
            info->bridge_ctrl.enable = 0;
        }
    } else {
        sim_log_err("no bridge chip connect\n");
        info->bridge_ctrl.enable = 0;
    }
}

int sim_pmu_ctrl_init(struct sim_platform_info *info)
{
    /* 如果已经get过，不需要重新get */
    if (info->sim_power.sim_pmu_init_flag == 1) {
        return SIM_HW_OP_RESULT_SUCC;
    }

    /* 如果是下电，当前又是关电流程，不需要再get */
    if (info->msgfromsci.msg_type == SIM_MSG_POWER_TYPE && info->msgfromsci.msg_value == SIM_POWER_OFF_REQUEST) {
        return SIM_HW_OP_RESULT_SUCC;
    }

    info->sim_power.sim_pmu = devm_regulator_get(info->spmidev, "sim_vcc");
    if (IS_ERR(info->sim_power.sim_pmu)) {
        sim_log_err("sim%d regulator get fail\n", info->sim_id);
        return SIM_HW_REQUESR_PMU_FAIL;
    }
    info->sim_power.sim_pmu_init_flag = 1;
    sim_log_err("sim%d regulator get done, count:%d\n", info->sim_id, info->regulator_enable_count);
    return SIM_HW_OP_RESULT_SUCC;
}

void sim_pmu_ctrl_deinit(struct sim_platform_info *info)
{
    if (info->sim_power.sim_pmu_init_flag != 1) {
        sim_log_err("sim%d regulator not get, skip put, count:%d\n", info->sim_id, info->regulator_enable_count);
        return;
    }

    if (regulator_is_enabled(info->sim_power.sim_pmu)) {
        sim_log_err("sim%d current work, skip put, count:%d\n", info->sim_id, info->regulator_enable_count);
        return;
    }

    devm_regulator_put(info->sim_power.sim_pmu);
    info->sim_power.sim_pmu = NULL;
    info->sim_power.sim_pmu_init_flag = 0;
    sim_log_err("sim%d regulator put done, count:%d\n", info->sim_id, info->regulator_enable_count);
}

int sim_pmu_power_on(struct sim_platform_info *info, s32 vol)
{
    int ret;
    u32 pmic_reg_value;

    ret = sim_pmu_ctrl_init(info);
    if (ret != SIMHP_OK) {
        return ret;
    }

    /* 不管硬件是否下电状态，但是上次还未走软件下电流程（硬件是快速下电，软件不感知），需要先走下软件下电流程 */
    if (info->regulator_enable_count >= 1) {
        sim_log_err("sim%d try power off first, conut:%d\n", info->sim_id, info->regulator_enable_count);
        info->regulator_enable_count--;
        ret = regulator_disable(info->sim_power.sim_pmu);
        if (ret) {
            sim_log_err("disable sim_vcc failed, ret:%d\n", ret);
        }
    }

    ret = regulator_set_voltage(info->sim_power.sim_pmu, vol, vol);
    if (ret != 0) {
        sim_log_err("set sim_pmu failed.\n");
        return SIM_HW_SET_VOLTAGE_FAIL;
    }

    if (info->regulator_enable_count != 0) {
        sim_log_err("sim%d unbalance count:%d, back to 0\n", info->sim_id, info->regulator_enable_count);
        info->regulator_enable_count = 0;
    }

    ret = regulator_enable(info->sim_power.sim_pmu);
    if (ret != 0) {
        sim_log_err("enable sim_pmu failed.\n");
        return SIM_POWER_ON_FAIL;
    }
    info->regulator_enable_count++;

    if (info->sim_power.sim_lvshift_ctrl.func) {
        pmic_reg_value = sub_pmic_reg_read(info->sim_power.sim_lvshift_ctrl.reg_off);
        pmic_reg_value &= (~0x4);
        sub_pmic_reg_write(info->sim_power.sim_lvshift_ctrl.reg_off, pmic_reg_value);
        udelay(1000); // delay 1000 us
        pmic_reg_value = sub_pmic_reg_read(info->sim_power.sim_lvshift_ctrl.reg_off);
        pmic_reg_value |= 0x1;
        sub_pmic_reg_write(info->sim_power.sim_lvshift_ctrl.reg_off, pmic_reg_value);
        sim_log_err("write lvshift offset %d, value %d.\n", info->sim_power.sim_lvshift_ctrl.reg_off, pmic_reg_value);
    }

    ret = regulator_is_enabled(info->sim_power.sim_pmu);
    if (ret == 0) {
        sim_log_err("sim%d power on fail\n", info->sim_id);
    }

    sim_pmu_set_bridge_volt(info, vol);

    sim_log_err("sim%d power on done, count:%d\n", info->sim_id, info->regulator_enable_count);

    return SIM_POWER_ON_SUCC;
}

int sim_pmu_power_off(struct sim_platform_info *info)
{
    int ret;
    u32 pmic_reg_value;
    if (info->sim_power.sim_lvshift_ctrl.func) {
        pmic_reg_value = sub_pmic_reg_read(info->sim_power.sim_lvshift_ctrl.reg_off);
        pmic_reg_value &= (~0x1);
        sub_pmic_reg_write(info->sim_power.sim_lvshift_ctrl.reg_off, pmic_reg_value);
        pmic_reg_value = sub_pmic_reg_read(info->sim_power.sim_lvshift_ctrl.reg_off);
        pmic_reg_value |= 0x4;
        sub_pmic_reg_write(info->sim_power.sim_lvshift_ctrl.reg_off, pmic_reg_value);
        sim_log_err("write lvshift offset %d, value %d.\n", info->sim_power.sim_lvshift_ctrl.reg_off, pmic_reg_value);
    }

    /* 没有 get 过的情况下，不能做 regulator 动作 */
    if (info->sim_power.sim_pmu_init_flag == 0) {
        sim_log_err("sim%d skip\n", info->sim_id);
        return SIM_POWER_OFF_SUCC;
    }

    /* 判断是否硬件做过下电，如果没有硬件下电，走软件下电流程 */
    ret = regulator_is_enabled(info->sim_power.sim_pmu);
    if (ret) {
        /* 未硬下电，走软件下电 */
        regulator_disable(info->sim_power.sim_pmu);
        info->regulator_enable_count--;
        sim_log_err("sim%d disabled, count:%d\n", info->sim_id, info->regulator_enable_count);
    } else {
        /* 如果硬件已经下过电后，走下regulator软件流程，保障软件状态一致 */
        sim_log_err("sim%d hardware disabled firstly, conut:%d\n", info->sim_id, info->regulator_enable_count);
        if (info->regulator_enable_count == 1) {
            sim_log_err("sim%d try soft disable, count:%d\n", info->sim_id, info->regulator_enable_count);
            regulator_disable(info->sim_power.sim_pmu);
            info->regulator_enable_count--;
        }
    }

    if (info->regulator_enable_count != 0) {
       sim_log_err("sim%d unbalance count:%d, back to 0\n", info->sim_id, info->regulator_enable_count);
       info->regulator_enable_count = 0;
    }

    /* sim bridge chip set power default 3V mode */
    sim_pmu_set_bridge_volt(info, SIM_VCC_3V);

    sim_log_err("sim%d power off done, count:%d\n", info->sim_id, info->regulator_enable_count);

    sim_pmu_ctrl_deinit(info);
    return SIM_POWER_OFF_SUCC;
}

int sim_pmu_ctrl(struct sim_platform_info *info)
{
    int ret;

    mutex_lock(&info->sim_hotplug_lock);

    sim_log_err("sim%d %s \n", info->sim_id, sim_msg_power_type(info->msgfromsci.msg_value));

    switch (info->msgfromsci.msg_value) {
        case SIM_POWER_ON_CLASS_C_REQUEST:
            ret = sim_pmu_power_on(info, SIM_VCC_1P8V);
            break;

        case SIM_POWER_ON_CLASS_B_REQUEST:
            ret = sim_pmu_power_on(info, SIM_VCC_3V);
            break;

        case SIM_POWER_OFF_REQUEST:
            ret = sim_pmu_power_off(info);
            break;

        default:
            sim_log_err("invalid cmd.\n");
            ret = SIM_HW_SET_VOLTAGE_FAIL;
            break;
    }

    mutex_unlock(&info->sim_hotplug_lock);

    sim_log_err("pmu ctrl done. ret = %d.\n", ret);
    return ret;
}

int sim_pmu_shutdown(drv_reset_cb_moment_e eparam, int userdata)
{
    int ret = 0;
    u32 sim_id, conv_msg;
    struct sim_platform_info *info = NULL;

    UNUSED(userdata);
    if (eparam != MDRV_RESET_CB_BEFORE) {
        return BSP_OK;
    }

    for (sim_id = 0; sim_id < MAXSIMHPNUM; sim_id++) {
        info = sim_get_plat_info(sim_id);
        if (info == NULL) {
            sim_log_err("sim%d info is NULL\n", sim_id);
            continue;
        }

        info->msgfromsci.msg_type = SIM_MSG_POWER_TYPE;
        info->msgfromsci.msg_value = SIM_POWER_OFF_REQUEST;
        if (info->is_udp) {
            ret = sim_pmu_ctrl(info);
        } else {
            conv_msg = info->msgfromsci.msg_type * SIM_HW_OPS_PER_TYPE + info->msgfromsci.msg_value;
            sim_log_err("hw_adp handle for sim_id %d with msg_type 0x%x.\n", info->sim_id, conv_msg);
            ret = sim_hwadp_handler(info->sim_id, conv_msg);
        }
        sim_log_err("sim%d SIM_POWER_OFF_REQUEST done, ret:%d\n", sim_id, ret);
    }

    return BSP_OK;
}

int sim_pmu_shutdown_cb_init(u32 sim_id)
{
    int ret;

    /* no params identify sim0 or sim1, only register one process cb func */
    if (sim_id > 0) {
        return BSP_OK;
    }

    ret = bsp_reset_cb_func_register("SCI", sim_pmu_shutdown, MDRV_RESET_CB_BEFORE, DRV_RESET_CB_PIOR_SCI);
    if (ret) {
        sim_log_err("modem self reset register fail. ret = %d.\n", ret);
    }
    sim_log_err("init ok\n");
    return ret;
}

int sim_hw_mgr_init(struct sim_platform_info *info)
{
    int ret = 0;
    if (info->is_udp) {
        ret = of_property_read_u32_array(info->spmidev->of_node, "sim_lvshift_ctrl",
            (u32 *)&info->sim_power.sim_lvshift_ctrl, 0x3);
        if (ret < 0) {
            sim_log_err("failed to read sim_lvshift_ctrl.\n");
            info->sim_power.sim_lvshift_ctrl.func = 0;
        }

        sim_pmu_bridge_init(info);
    } else {
        sim_hwadp_init(info->sim_id, info->spmidev);
    }

    ret = sim_pmu_shutdown_cb_init(info->sim_id);
    return ret;
}
