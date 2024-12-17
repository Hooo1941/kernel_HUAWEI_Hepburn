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
/*
 * @brief   module_init模块在拼片平台acore底软内部头文件
 * @file    mdrv_module_init_extra_modem.h
 * @version v1.0
 * @date    2020.11.20
 * <ul><li>v1.0|2020.11.20|创建文件</li></ul>
 * @since
 */

#ifndef __MDRV_ACORE_MODULE_INIT_EXTRA_MODEM_H__
#define __MDRV_ACORE_MODULE_INIT_EXTRA_MODEM_H__

#ifdef __cplusplus
extern "C" {
#endif

extern int rfs_server_init(void);
extern int modem_boot_init(void);
extern int bsp_slice_init(void);
extern int bsp_slice_sync_init(void);
extern int pcie_cdev_platform_rc_init(void);
extern int pmodem_init(void);
extern int bsp_vcom_init(void);
extern int modem_boot_init(void);
extern int pur_init(void);
extern int pcie_nic_init_callback(void);
extern int diag_init(void);
extern int bsp_dump_init(void);
extern int mmc_init(void);
extern s32 bsp_dump_mem_init(void);
extern int modemNv_ProcInit(void);
extern int bsp_version_acore_init(void);
extern int bsp_reset_init(void);
extern int sim_hw_service_init(void);
extern int adc_modem_init(void);
extern int weth_init(void);
extern int bsp_efuse_agent_init(void);
extern int bsp_sysboot_extra_init(void);
extern int bsp_pcie_init_before_boot(void);
extern int trans_report_rc_init(void);
#ifdef CONFIG_MLOADER_HOT_PATCH
extern int bsp_modem_hot_patch_init(void);
#endif

/*
 * @brief modem初始化函数指针类型
 */
typedef int (*modem_drv_module_init_func)(void);

/*
 * @brief modem初始化函数列表
 */
static modem_drv_module_init_func g_modem_drv_module_init_func[] = {
#ifdef ENABLE_BUILD_OM
    bsp_dump_mem_init,
    bsp_dump_init,
#endif

#ifdef CONFIG_WETH
    weth_init,
    trans_report_rc_init,
#endif

    mmc_init,
#ifdef CONFIG_RFS_SERVER
    rfs_server_init,
#endif

#ifdef CONFIG_PCIE_CDEV_RC
    pcie_cdev_platform_rc_init,
#endif

#ifdef CONFIG_PCIE_CDEV_ENG
#ifdef CONFIG_PCIE_MODEM
    pmodem_init,
#endif
#endif
#ifdef CONFIG_APLUSB_PCIE_ICC
    bsp_vcom_init,
#endif
#ifdef CONFIG_MLOADER_HOT_PATCH
    bsp_modem_hot_patch_init,
#endif
#ifdef CONFIG_EXTRA_ONOFF
    bsp_sysboot_extra_init,
#endif
#ifdef CONFIG_MODEM_ADC
    adc_modem_init,
#endif

#ifdef CONFIG_NVIM
    modemNv_ProcInit,
#endif
#ifdef CONFIG_EFUSE
    bsp_efuse_agent_init,
#endif
#ifdef CONFIG_IOT_PCIE_DEV
    bsp_pcie_init_before_boot,
#endif
    bsp_version_acore_init,
    bsp_reset_init,

    modem_boot_init,
#if (defined(CONFIG_EXTRA_TIMERSLICE) && defined(CONFIG_MODULE_TIMER))
    bsp_slice_init,
#endif
#ifdef CONFIG_EXTRA_SLICE_SYNC
    bsp_slice_sync_init,
#endif

#ifdef CONFIG_PCIE_CDEV_RC
    pur_init,
#endif
    diag_init,
};

static modem_drv_module_init_func g_modem_drv_late_init_func[] = {
#ifdef CONFIG_SIM_HOTPLUG_SPMI
    sim_hw_service_init,
#endif
};

#ifdef __cplusplus
}
#endif
#endif
