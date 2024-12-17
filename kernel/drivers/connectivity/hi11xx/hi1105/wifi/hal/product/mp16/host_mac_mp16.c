/*
 * Copyright (c) @CompanyNameMagicTag 2021-2023. All rights reserved.
 * 功能说明 : HOST MAC 处理接口
 * 作    者 : wifi
 * 创建日期 : 2012年9月18日
 */

#include "host_mac_mp16.h"
#include "pcie_host.h"
#include "oal_util.h"
#include "oal_ext_if.h"
#include "host_hal_access_mgmt.h"
#include "host_hal_ring.h"
#include "host_hal_device.h"
#include "frw_ext_if.h"
#include "wlan_spec.h"

#undef THIS_FILE_ID
#define THIS_FILE_ID OAM_FILE_ID_HOST_HAL_MAC_MP16_C

void mp16_get_host_phy_int_status(hal_host_device_stru *pst_hal_device, uint32_t *pul_status)
{
    *pul_status = hal_host_readl_irq(pst_hal_device->phy_regs.irq_status_reg);
}

/*
 * 功能描述   : 读中断屏蔽状态寄存器
 * 1.日    期   : 2018年5月20日
 *   作    者   : wifi
 *   修改内容   : 新生成函数
 */
void mp16_get_host_phy_int_mask(hal_host_device_stru *hal_device, uint32_t *mask)
{
    *mask = hal_host_readl_irq(hal_device->phy_regs.irq_status_mask_reg);
}
/*
 * 功能描述   : 读中断屏蔽状态寄存器
 * 1.日    期   : 2018年5月20日
 *   作    者   : wifi
 *   修改内容   : 新生成函数
 */
void mp16_set_host_phy_irq_mask(hal_host_device_stru *hal_device, uint32_t mask)
{
    if (hal_device == NULL) {
        return;
    }
    hal_host_writel(mask, hal_device->phy_regs.irq_status_mask_reg);
}
/*
 * 功能描述   : 清中断状态寄存器
 * 1.日    期   : 2018年5月20日
 *   作    者   : wifi
 *   修改内容   : 新生成函数
 */
void mp16_clear_host_phy_int_status(hal_host_device_stru *hal_device, uint32_t status)
{
    if (hal_device == NULL) {
        return;
    }
    hal_host_writel_irq(status, hal_device->phy_regs.irq_clr_reg);
}


/*
 * 功能描述   : 读中断状态寄存器
 * 1.日    期   : 2018年5月20日
 *   作    者   : wifi
 *   修改内容   : 新生成函数
 */
void mp16_get_host_mac_int_status(hal_host_device_stru *hal_device, uint32_t *status)
{
    *status = hal_host_readl_irq(hal_device->mac_regs.irq_status_reg);
}

/*
 * 功能描述   : 读中断屏蔽状态寄存器
 * 1.日    期   : 2018年5月20日
 *   作    者   : wifi
 *   修改内容   : 新生成函数
 */
void mp16_get_host_mac_int_mask(hal_host_device_stru *hal_device, uint32_t *p_mask)
{
    *p_mask = hal_host_readl_irq(hal_device->mac_regs.irq_status_mask_reg);
}

/* 功能描述 : 刷中断屏蔽irq_status_mask */
void mp16_set_host_mac_irq_mask(hal_host_device_stru *hal_device, uint32_t intr_mask)
{
    if (hal_device == NULL) {
        return;
    }
    hal_host_writel(intr_mask, (uintptr_t)hal_device->mac_regs.irq_status_mask_reg);
}

/*
 * 功能描述   : 清中断状态寄存器
 * 1.日    期   : 2018年5月20日
 *   作    者   : wifi
 *   修改内容   : 新生成函数
 */
void mp16_clear_host_mac_int_status(hal_host_device_stru *pst_hal_device, uint32_t status)
{
    if (pst_hal_device == NULL) {
        return;
    }
    hal_host_writel_irq(status, pst_hal_device->mac_regs.irq_clr_reg);
}

/*
 * 功能描述   : 读状态寄存器
 * 1.日    期   : 2020年8月18日
 *   修改内容   : 新生成函数
 */
uint32_t mp16_get_dev_rx_filt_mac_status(uint64_t *reg_status)
{
    uint32_t reg_addr;
    uint64_t host_va = 0;

    if (reg_status == NULL) {
        return OAL_FAIL;
    }
    reg_addr = MP16_MAC_RX_FRAMEFILT2_REG;
    if (oal_pcie_devca_to_hostva(HCC_EP_WIFI_DEV, reg_addr, &host_va) != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_RX, "{get_dev_rx_filt_mac_status::regaddr[%x] devca2hostva fail.}",
            reg_addr);
        return OAL_FAIL;
    }
    *reg_status = ((uint64_t)hal_host_readl((uintptr_t)host_va)) << BIT_OFFSET_32;
    reg_addr = MP16_MAC_RX_FRAMEFILT_REG;
    if (oal_pcie_devca_to_hostva(HCC_EP_WIFI_DEV, reg_addr, &host_va) != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_RX, "{get_dev_rx_filt_mac_status::regaddr[%x] devca2hostva fail.}",
            reg_addr);
        return OAL_FAIL;
    }
    *reg_status = (*reg_status | hal_host_readl((uintptr_t)host_va));
    return OAL_SUCC;
}

/*
 * 功能描述 : unmask mac中断屏蔽
 * 1.日 期 : 2019年8月23日
 * 修改内容 : 新生成函数
 */
void  mp16_host_mac_irq_unmask(hal_host_device_stru *hal_device, uint32_t irq)
{
    uint32_t intr_mask;

    intr_mask = hal_host_readl(hal_device->mac_regs.irq_status_mask_reg);
    intr_mask &= (~((uint32_t)1 << irq));

    hal_host_writel(intr_mask, hal_device->mac_regs.irq_status_mask_reg);
}

/*
 * 功能描述 : mask mac中断屏蔽bit
 * 1.日 期 :  2019年8月23日
 * 作 者   :  wifi
 * 修改内容 : 新生成函数
 */
void  mp16_host_mac_irq_mask(hal_host_device_stru *hal_device, uint32_t irq)
{
    uint32_t intr_mask;
    intr_mask = hal_host_readl(hal_device->mac_regs.irq_status_mask_reg);
    if (intr_mask == HAL_HOST_READL_INVALID_VAL) {
        return;
    }
    intr_mask |= (((uint32_t)1 << irq));
    hal_host_writel(intr_mask, hal_device->mac_regs.irq_status_mask_reg);
}

uint32_t mp16_regs_addr_get_offset(uint8_t device_id, uint32_t reg_addr)
{
    uint32_t offset = 0;
    uint32_t mac_reg_offset[WLAN_DEVICE_MAX_NUM_PER_CHIP] = {0, MP16_MAC_BANK_REG_OFFSET};
    uint32_t phy_reg_offset[WLAN_DEVICE_MAX_NUM_PER_CHIP] = {0, MP16_PHY_BANK_REG_OFFSET};
    uint32_t soc_reg_offset[WLAN_DEVICE_MAX_NUM_PER_CHIP] = {0, 0};
#ifndef _PRE_LINUX_TEST
    switch (reg_addr) {
        case MP16_MAC_HOST_INTR_CLR_REG ... MP16_MAC_RPT_SMAC_INTR_CNT_REG:
            offset = mac_reg_offset[device_id];
            break;
        case MP16_DFX_REG_BANK_CFG_INTR_CLR_REG:
        case MP16_DFX_REG_BANK_CFG_INTR_MASK_HOST_REG:
        case MP16_DFX_REG_BANK_PHY_INTR_RPT_REG:
        case MP16_RX_CHN_EST_REG_BANK_CHN_EST_COM_REG:
        case MP16_PHY_GLB_REG_BANK_FTM_CFG_REG:
        case MP16_RF_CTRL_REG_BANK_0_CFG_ONLINE_CALI_USE_RF_REG_EN_REG:
        case MP16_CALI_TEST_REG_BANK_0_FIFO_FORCE_EN_REG:
        case MP16_PHY_GLB_REG_BANK_PHY_BW_MODE_REG:
        case MP16_RX_AGC_REG_BANK_CFG_AGC_WEAK_SIG_EN_REG:
            offset = phy_reg_offset[device_id];
            break;
        /* soc寄存器不区分主辅路,所以不区分主辅路偏移 */
        case MP16_W_SOFT_INT_SET_AON_REG:
            offset = soc_reg_offset[device_id];
            break;
        default:
            oam_error_log1(0, OAM_SF_ANY, "{regs_addr_get_offset::reg addr[%x] not used.}", reg_addr);
    }
#endif
    return offset;
}

uint32_t mp16_regs_addr_transfer(hal_host_device_stru *hal_device, uint32_t reg_addr)
{
    uint64_t host_va = 0;
    uint32_t offset;

    if (hal_device->device_id >= HAL_DEVICE_ID_BUTT) {
        oam_warning_log1(0, OAM_SF_ANY, "{hal_mac_regs_addr_transfer::device_id[%x]!}", hal_device->device_id);
        return OAL_FAIL;
    }

    offset = mp16_regs_addr_get_offset(hal_device->device_id, reg_addr);
    if (oal_pcie_devca_to_hostva(HCC_EP_WIFI_DEV, reg_addr + offset, &host_va) != OAL_SUCC) {
        oam_error_log1(0, OAM_SF_ANY, "{hal_mac_regs_addr_transfer::regaddr[%x] devca2hostva fail.}", reg_addr);
        return OAL_FAIL;
    }
#ifndef _PRE_LINUX_TEST
    switch (reg_addr) {
        case MP16_MAC_HOST_INTR_CLR_REG:
            hal_device->mac_regs.irq_clr_reg = (uintptr_t)host_va;
            break;
        case MP16_MAC_HOST_INTR_MASK_REG:
            hal_device->mac_regs.irq_status_mask_reg = (uintptr_t)host_va;
            break;
        case MP16_MAC_HOST_INTR_STATUS_REG:
            hal_device->mac_regs.irq_status_reg = (uintptr_t)host_va;
            break;
        case MP16_MAC_VAP_TSFTIMER_RDVALL_STATUS_12_REG:
            hal_device->mac_regs.tsf_lo_reg = (uintptr_t)host_va;
            break;
        case MP16_MAC_SMAC_COMMON_TIMER_STATUS_REG:
            hal_device->mac_regs.timer_status_reg = (uintptr_t)host_va;
            break;
        case MP16_DFX_REG_BANK_CFG_INTR_CLR_REG:
            hal_device->phy_regs.irq_clr_reg = (uintptr_t)host_va;
            break;
        case MP16_DFX_REG_BANK_CFG_INTR_MASK_HOST_REG:
            hal_device->phy_regs.irq_status_mask_reg = (uintptr_t)host_va;
            break;
        case MP16_DFX_REG_BANK_PHY_INTR_RPT_REG:
            hal_device->phy_regs.irq_status_reg = (uintptr_t)host_va;
            break;
        case MP16_W_SOFT_INT_SET_AON_REG:
            hal_device->soc_regs.w_soft_int_set_aon_reg = (uintptr_t)host_va;
            break;
        default:
            oam_warning_log1(0, OAM_SF_ANY, "{hal_mac_regs_addr_transfer::reg addr[%x] not used.}", reg_addr);
            return OAL_FAIL;
    }
#endif
    return OAL_SUCC;
}


int32_t mp16_host_regs_addr_init(hal_host_device_stru *hal_device)
{
    uint32_t idx;
    uint32_t hal_regs[] = {
        MP16_MAC_HOST_INTR_CLR_REG,
        MP16_MAC_HOST_INTR_MASK_REG,
        MP16_MAC_HOST_INTR_STATUS_REG,
        MP16_MAC_VAP_TSFTIMER_RDVALL_STATUS_12_REG,
        MP16_MAC_SMAC_COMMON_TIMER_STATUS_REG,
        MP16_DFX_REG_BANK_CFG_INTR_CLR_REG,
        MP16_DFX_REG_BANK_CFG_INTR_MASK_HOST_REG,
        MP16_DFX_REG_BANK_PHY_INTR_RPT_REG,
        MP16_W_SOFT_INT_SET_AON_REG,
    };

    /* 唤醒低功耗内存 */
    hal_pm_try_wakeup_forbid_sleep();
    if (hal_pm_try_wakeup_dev_lock() != OAL_SUCC) {
        hal_pm_try_wakeup_allow_sleep();
        return OAL_FAIL;
    }

    for (idx = 0; idx < sizeof(hal_regs) / sizeof(uint32_t); idx++) {
        if (OAL_SUCC != mp16_regs_addr_transfer(hal_device, hal_regs[idx])) {
            hal_pm_try_wakeup_allow_sleep();
            return OAL_FAIL;
        }
    }
    hal_pm_try_wakeup_allow_sleep();
    oam_warning_log0(0, OAM_SF_ANY, "{hal_host_regs_addr_init::regs addr trans succ.}");
    return OAL_SUCC;
}

/* 功能描述 : 常收维测信息输出 */
void mp16_host_al_rx_fcs_info(hmac_vap_stru *hmac_vap)
{
    hal_host_device_stru *hal_device = hal_get_host_device(hmac_vap->hal_dev_id);
    if (hal_device == NULL) {
        oam_error_log0(0, 0, "{host_al_rx_fcs_info::hal_device null.}");
        return;
    }

    oam_warning_log3(hal_device->device_id, OAM_SF_CFG,
        "host_al_rx_fcs_info:packets info, mpdu succ[%d], ampdu succ[%d],fail[%d]",
        hal_device->st_alrx.rx_normal_mdpu_succ_num,
        hal_device->st_alrx.rx_ampdu_succ_num,
        hal_device->st_alrx.rx_ppdu_fail_num);

    hal_device->st_alrx.rx_normal_mdpu_succ_num = 0;
    hal_device->st_alrx.rx_ampdu_succ_num = 0;
    hal_device->st_alrx.rx_ppdu_fail_num = 0;
    return;
}

/* 功能描述 : 常收维测信息输出 */
void mp16_host_get_rx_pckt_info(hmac_vap_stru *hmac_vap,
    dmac_atcmdsrv_atcmd_response_event *rx_pkcg_event_info)
{
    hal_host_device_stru *hal_device = hal_get_host_device(hmac_vap->hal_dev_id);
    if (hal_device == NULL) {
        oam_error_log0(0, 0, "{host_get_rx_pckt_info::hal_device null.}");
        return;
    }
    if (hal_device->st_alrx.rx_ampdu_succ_num != 0) {
        rx_pkcg_event_info->event_para = hal_device->st_alrx.rx_ampdu_succ_num;
    } else {
        rx_pkcg_event_info->event_para = hal_device->st_alrx.rx_normal_mdpu_succ_num;
    }

    return;
}

static hal_mac_common_timer *g_mac_timer_table[MP16_MAC_MAX_COMMON_TIMER + 1] = {0};
/* 功能描述 : 初始化host通用定时器，填写转换地址 */
int32_t mp16_host_init_common_timer(hal_mac_common_timer *mac_timer)
{
    uint64_t host_va = 0;
    uint8_t timer_id = mac_timer->timer_id;
    uint32_t mask_dev_reg = MP16_MAC_DMAC_COMMON_TIMER_MASK_REG;
    uint32_t timer_ctrl = MP16_MAC_COMMON_TIMER_CTRL_0_REG + MP16_MAC_COMMON_TIMER_OFFSET * timer_id;
    uint32_t timer_val = MP16_MAC_COMMON_TIMER_VAL_0_REG + MP16_MAC_COMMON_TIMER_OFFSET * timer_id;

    if (timer_id > MP16_MAC_MAX_COMMON_TIMER) {
        oam_error_log1(0, OAM_SF_ANY, "{hal_host_init_common_timer::timer id[%d] error.}", timer_id);
        return OAL_FAIL;
    }

    if (oal_pcie_devca_to_hostva(HCC_EP_WIFI_DEV, mask_dev_reg, &host_va) != OAL_SUCC) {
        oam_error_log1(0, OAM_SF_ANY, "{hal_host_init_common_timer::regaddr[%x] devca2hostva fail.}", mask_dev_reg);
        return OAL_FAIL;
    }
    mac_timer->mask_reg_addr = (uintptr_t)host_va;

    if (oal_pcie_devca_to_hostva(HCC_EP_WIFI_DEV, timer_ctrl, &host_va) != OAL_SUCC) {
        oam_error_log1(0, OAM_SF_ANY, "{hal_host_init_common_timer::regaddr[%x] devca2hostva fail.}", timer_ctrl);
        return OAL_FAIL;
    }
    mac_timer->timer_ctrl_addr = (uintptr_t)host_va;

    if (oal_pcie_devca_to_hostva(HCC_EP_WIFI_DEV, timer_val, &host_va) != OAL_SUCC) {
        oam_error_log1(0, OAM_SF_ANY, "{hal_host_init_common_timer::regaddr[%x] devca2hostva fail.}", timer_val);
        return OAL_FAIL;
    }
    mac_timer->timer_val_addr = (uintptr_t)host_va;
    g_mac_timer_table[timer_id] = mac_timer;
    return OAL_SUCC;
}

hal_mac_common_timer *mp16_get_host_mac_common_timer_ptr(uint8_t id)
{
    if (id > MP16_MAC_MAX_COMMON_TIMER) {
        return NULL;
    }
    return g_mac_timer_table[id];
}

uint32_t mp16_get_host_mac_common_timer_status(hal_host_device_stru *hal_device)
{
    return hal_host_readl(hal_device->mac_regs.timer_status_reg);
}

/*
 * 函 数 名  : mp16_host_set_mac_common_timer
 * 功能描述  : 配置通用MAC定时器
 * 1.日    期  : 2020年12月22日
 *   作    者  : wifi
 *   修改内容  : 新生成函数
 */
void mp16_host_set_mac_common_timer(hal_mac_common_timer *mac_common_timer)
{
    uint8_t timer_id = mac_common_timer->timer_id;
    uint32_t timer_mask;
    uint32_t timer_ctrl;

    if (timer_id > MP16_MAC_MAX_COMMON_TIMER) {
        oam_error_log1(0, OAM_SF_ANY, "host_set_mac_common_timer :: timer[%d] id error.", timer_id);
        return;
    }
    hal_pm_try_wakeup_forbid_sleep();
    if (hal_pm_try_wakeup_dev_lock() != OAL_SUCC) {
        hal_pm_try_wakeup_allow_sleep();
        return;
    }

    if (mac_common_timer->timer_en == OAL_FALSE) {
        hal_host_writel(0, mac_common_timer->timer_ctrl_addr);
        hal_pm_try_wakeup_allow_sleep();
        return;
    }
    timer_mask = hal_host_readl(mac_common_timer->mask_reg_addr);
    timer_mask = timer_mask & (~BIT(timer_id)); /* 打开timer_id对应的mask */
    hal_host_writel(timer_mask, mac_common_timer->mask_reg_addr); /* 打开中断处理 */

    hal_host_writel(mac_common_timer->common_timer_val, mac_common_timer->timer_val_addr);

    timer_ctrl = ((mac_common_timer->timer_en) | (mac_common_timer->timer_mode << 1) |
        (mac_common_timer->timer_unit << 2)); /* bit0: timer en bit1: timer_mode bit2~3: unit */
    hal_host_writel(timer_ctrl, mac_common_timer->timer_ctrl_addr);
    hal_pm_try_wakeup_allow_sleep();
}

uint32_t mp16_host_get_tsf_lo(hal_host_device_stru *hal_device, uint8_t hal_vap_id, uint32_t *tsf)
{
    uintptr_t reg_addr;
    if (hal_vap_id >= HAL_MAX_VAP_NUM || hal_device->mac_regs.tsf_lo_reg == 0) {
        return OAL_FAIL;
    }
    hal_pm_try_wakeup_forbid_sleep();
    if (hal_pm_try_wakeup_dev_lock() != OAL_SUCC) {
        hal_pm_try_wakeup_allow_sleep();
        return OAL_FAIL;
    }

    reg_addr = hal_device->mac_regs.tsf_lo_reg + (hal_vap_id * 4); // 4: 每个vap一个tsf寄存器（4字节
    *tsf = hal_host_readl(reg_addr);
    hal_pm_try_wakeup_allow_sleep();
    return OAL_SUCC;
}
