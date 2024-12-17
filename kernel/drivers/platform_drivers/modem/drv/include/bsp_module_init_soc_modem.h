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
 * @brief   module_init模块在soc平台acore上底软内部头文件
 * @file    bsp_module_init_soc_modem.h
 * @version v1.0
 * @date    2020.11.20
 * <ul><li>v1.0|2020.11.20|创建文件</li></ul>
 * @since
 */
#ifndef __BSP_MODULE_INIT_SOC_MODEM_H__
#define __BSP_MODULE_INIT_SOC_MODEM_H__

#ifdef __cplusplus
extern "C" {
#endif

/* declare function for drv begin */
extern int bsp_load_image_init(void);
extern int sysctrl_init(void);
extern int bsp_slice_init(void);
extern int bsp_timer_init(void);
extern int bsp_softtimer_init(void);
extern int bsp_ipcmsg_init(void);
extern int bsp_ipc_init(void);
extern int bsp_fipc_init(void);
extern int bsp_dump_mem_init(void);
extern int syslog_logmem_init(void);
extern int bsp_udi_init(void);
extern int bsp_reset_init(void);
#ifdef ECDC_INIT_ENABLE
extern int ecdc_genetlink_init(void);
extern int llt_shell_verify_init(void);
#endif

#ifdef CONFIG_MDMPM_DEBUG
extern int mdmpm_debug_init(void);
#endif
extern int odt_init(void);
extern int bsp_dump_init(void);
extern int mmc_init(void);
extern int nv_init_dev(void);
extern int om_socket_init(void);
extern int bsp_version_acore_init(void);
extern int ppm_init_phy_port(void);
extern int cpm_port_associate_init(void);
extern int scm_init(void);
extern int bsp_trng_seed_init(void);
extern int bsp_dump_resave_modem_log(void);
extern int dump_mdmap_subsys_exc_stub_init(void);
extern int modem_rfile_init(void);

#ifdef CONFIG_MODEM_PINCTRL
extern int bsp_modem_pinctrl_dts_parse(void);
#endif

#if defined(CONFIG_KERNEL_DTS_DECOUPLED)
extern int bsp_dt_init(void);
#endif


#ifdef CONFIG_AVS_TEST
extern int avs_test_init(void);
#endif

extern int bsp_modem_boot_init(void);
extern int modem_efuse_agent_init(void);
extern int virtshl_init(void);
extern int cshell_port_init(void);
extern int modem_log_init(void);
extern int pmic_modem_ocp_init(void);
#ifdef CONFIG_MODEM_CORESIGHT
extern int coresight_trace_init(void);
extern int mdmcp_coresight_init(void);
#endif
#ifdef CONFIG_SYSVIEW
extern int bsp_sysview_init(void);
#endif
extern int modem_power_on_init(void);
extern int sim_hw_service_init(void);
extern int bastet_modem_driver_init(void);
#ifndef CONFIG_HOMI_DISABLE
extern int homi_init(void);
extern int homi_usb_init(void);
extern int homi_socket_init(void);
extern int homi_netlink_init(void);
#endif

extern int diag_init(void);

#ifdef CONFIG_MPROBE
extern int mprobe_init(void);
#endif
#ifdef CONFIG_DRA
extern int dra_init(void);
#endif
#ifdef CONFIG_SC_GID_CTRL
extern int gid_ctrl_init(void);
#endif
extern int aximem_init(void);
#ifdef CONFIG_NMI
extern int nmi_init(void);
#endif
extern int mloader_verifier_init(void);
extern int mloader_loader_init(void);
extern int mloader_driver_init(void);
extern int lpmcu_modem_init(void);
extern int bsp_pfa_module_init(void);
extern int pfa_filter_init(void);
extern int trans_report_init(void);
#ifdef CONFIG_MLOADER_HOT_PATCH
extern int bsp_modem_hot_patch_b_init(void);
#endif
extern int bsp_memory_init(void);
extern int bsp_memory_layout_init(void);
extern int bsp_memory_report_init(void);
extern int bsp_fw_dt_init(void);
extern int bsp_msg_init(void);
#ifdef CONFIG_BBPDS
extern int bbpds_init(void);
#endif
extern int ccpudebug_init(void);
#ifdef CONFIG_VDEV
extern int bsp_vdev_init(void);
#endif
extern int bsp_usb_relay_init(void);

extern int sysboot_load_modem_teeos_early_init(void);
extern int sysboot_load_modem_teeos_later_init(void);
#ifdef CONFIG_CHR_FS
extern int chr_init(void);
#endif

#ifdef FEATURE_NVA_ON
extern int nva_init(void);
#endif

extern int hmi_init(void);

#ifdef CONFIG_SYSBUS
extern int sys_bus_init(void);
#endif

#ifdef CONFIG_AP_QIC
extern int qic_dump_register_init(void);
#endif

#ifdef CONFIG_AP_NOC
extern int noc_dump_register_init(void);
#endif

#ifdef CONFIG_MODEM_VBAT
extern int bsp_vbat_init(void);
#endif
#ifdef CONFIG_DPA_OM
extern int bsp_diag_dpa_msg_init(void);
#endif
/* declare function for drv end */
/*
 * @brief modem初始化函数指针类型
 */
typedef int (*modem_drv_module_init_func)(void);

/*
 * @brief modem初始化函数列表
 */
static modem_drv_module_init_func g_modem_drv_module_init_func[] = {
/* pure_initcall */
/* 融合架构平台依赖memory layout的模块初始化需要放在
   sysboot_load_modem_teeos_early_init与sysboot_load_modem_teeos_later_init之间
*/
    sysboot_load_modem_teeos_early_init,
    mloader_verifier_init,
    bsp_fw_dt_init,
    sysboot_load_modem_teeos_later_init,
#if defined(CONFIG_KERNEL_DTS_DECOUPLED)
    bsp_dt_init,
#endif
#ifdef FEATURE_NVA_ON
    nva_init,
#endif
#ifdef CONFIG_SHARED_MEMORY
    bsp_memory_init,
#endif
#ifdef CONFIG_MEMORY_LAYOUT
    bsp_memory_layout_init,
#endif
    /* core_initcall */
    bsp_dump_resave_modem_log,
    sysctrl_init,
    bsp_slice_init,
    bsp_dump_mem_init,
    syslog_logmem_init,
#ifdef CONFIG_AP_QIC
    qic_dump_register_init,
#endif
#ifdef CONFIG_AP_NOC
    noc_dump_register_init,
#endif

#ifdef CONFIG_MODULE_IPCMSG
    bsp_ipcmsg_init,
#endif
    bsp_ipc_init,
    mloader_loader_init,
    bsp_dump_init,
/* arch_initcall */
#ifdef CONFIG_NMI
    nmi_init,
#endif
    bsp_timer_init,
#ifdef CONFIG_FIPC_V200
    bsp_fipc_init,
#endif
    bsp_udi_init,

    /* arch_initcall_sync */
#ifdef CONFIG_MODEM_MSG
    bsp_msg_init,
#endif
#ifdef CONFIG_EFUSE
#ifdef CONFIG_EFUSE_MODEM_AGENT
    modem_efuse_agent_init,
#endif
#endif
    lpmcu_modem_init,
#ifdef CONFIG_SC_GID_CTRL
    gid_ctrl_init,
#endif
#ifdef CONFIG_DRA
    dra_init,
#endif
    /* subsys_initcall */
#ifdef CONFIG_AVS_TEST
    avs_test_init,
#endif
    /* fs_initcall */
    mmc_init,

    /* module_init */
    bsp_reset_init,
    dump_mdmap_subsys_exc_stub_init,

#ifdef ECDC_INIT_ENABLE
    ecdc_genetlink_init,
    llt_shell_verify_init,
#endif

    odt_init,

#ifdef CONFIG_BBPDS
    bbpds_init,
#endif

#ifdef CONFIG_MLOADER_HOT_PATCH
    bsp_modem_hot_patch_b_init,
#endif
#if (defined(CONFIG_OM_SOCKET) || (defined(FEATURE_SVLSOCKET)))
    om_socket_init,
#endif
    bsp_version_acore_init,

#ifndef CONFIG_HOMI_DISABLE
    homi_init,
    homi_usb_init,
    homi_socket_init,
    homi_netlink_init,
#endif
    diag_init,
#ifdef CONFIG_MPROBE
    mprobe_init,
#endif
#ifdef CONFIG_PFA
    bsp_pfa_module_init,
    pfa_filter_init,
#endif
    bsp_modem_boot_init,
#ifdef CONFIG_CSHELL
    virtshl_init,
    cshell_port_init,
#endif
    modem_log_init,
#ifdef CONFIG_MDMPM_DEBUG
    mdmpm_debug_init,
#endif
#ifdef CONFIG_PMU_OCP
    pmic_modem_ocp_init,
#endif
#ifdef CONFIG_VDEV
    bsp_vdev_init,
#endif
    bsp_usb_relay_init,
#ifdef CONFIG_NET_OM
    trans_report_init,
#endif

#ifdef FEATURE_BASTET
#if (FEATURE_BASTET == FEATURE_ON)
    bastet_modem_driver_init,
#endif
#endif

#ifdef CONFIG_TRNG_SEED
    bsp_trng_seed_init,
#endif
#ifdef CONFIG_CHR_FS
    chr_init,
#endif
    modem_rfile_init,
#ifdef CONFIG_MODEM_VBAT
    bsp_vbat_init,
#endif
};

static modem_drv_module_init_func g_modem_drv_late_init_func[] = {
    /* late_initcall */
#ifdef CONFIG_MODEM_CORESIGHT
    mdmcp_coresight_init,
    coresight_trace_init,
#endif
    modem_power_on_init,
#ifdef CONFIG_CCPUDEBUG
    ccpudebug_init,
#endif
#ifdef CONFIG_SYSVIEW
    bsp_sysview_init,
#endif
#ifdef CONFIG_SIM_HOTPLUG_SPMI
    sim_hw_service_init,
#endif

#ifdef CONFIG_MODEM_PINCTRL
    bsp_modem_pinctrl_dts_parse,
#endif

    mloader_driver_init,

    bsp_memory_report_init,
#ifdef CONFIG_SYSBUS
    sys_bus_init,
#endif
#ifdef CONFIG_DPA_OM
    bsp_diag_dpa_msg_init,
#endif
};


#ifdef __cplusplus
}
#endif
#endif
