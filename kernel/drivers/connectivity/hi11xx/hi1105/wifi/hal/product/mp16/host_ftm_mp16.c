/*
* Copyright (c) @CompanyNameMagicTag 2021-2023. All rights reserved.
* 功能说明   : FTM功能
* 作者       : wifi
* 创建日期   : 2020年12月8日
*/
#ifdef _PRE_WLAN_FEATURE_FTM
/* 1 头文件包含 */
#include "mac_data.h"
#include "mac_ftm.h"
#include "hmac_resource.h"
#include "hmac_host_tx_data.h"
#include "hmac_tx_data.h"
#include "hmac_config.h"
#include "host_mac_mp16.h"
#include "host_ftm_mp16.h"
#include "host_hal_access_mgmt.h"
#include "hmac_tx_msdu_dscr.h"
#include "hmac_ftm.h"

#include "securec.h"
#include "securectype.h"

#undef THIS_FILE_ID
#define THIS_FILE_ID OAM_FILE_ID_HOST_HAL_FTM_MP16_C

/*
 * 功能描述  : 地址映射
 * 1.日    期  : 2020年11月12日
 *   作    者  : wifi
 *   修改内容  : 新生成函数
 */
static uint32_t mp16_mac_regs_ftm_transfer(hal_host_device_stru *hal_device, uint32_t reg_addr)
{
    uint64_t host_va = 0;
    uint32_t offset  = mp16_regs_addr_get_offset(hal_device->device_id, reg_addr);
    if (oal_pcie_devca_to_hostva(HCC_EP_WIFI_DEV, reg_addr + offset, &host_va) != OAL_SUCC) {
        oam_error_log1(0, OAM_SF_ANY, "{mac_regs_ftm_transfer::regaddr[%x] devca2hostva fail.}", reg_addr);
        return OAL_FAIL;
    }
#ifndef _PRE_LINUX_TEST
    switch (reg_addr) {
        case MP16_MAC_FTM_PROCESS_REG:
            hal_device->ftm_regs.ftm_pro_reg = (uintptr_t)host_va;
            break;
        case MP16_MAC_FTM_INFO_ADDR_MSB_REG:
            hal_device->ftm_regs.ftm_info_msb = (uintptr_t)host_va;
            break;
        case MP16_MAC_FTM_INFO_ADDR_LSB_REG:
            hal_device->ftm_regs.ftm_info_lsb = (uintptr_t)host_va;
            break;
        case MP16_MAC_FTM_BUF_BASE_ADDR_LSB_REG:
            hal_device->ftm_regs.base_lsb = (uintptr_t)host_va;
            break;
        case MP16_MAC_FTM_BUF_BASE_ADDR_MSB_REG:
            hal_device->ftm_regs.base_msb = (uintptr_t)host_va;
            break;
        case MP16_MAC_FTM_WHITELIST_ADDR_LSB_0_REG:
            hal_device->ftm_regs.white_addr_lsb = (uintptr_t)host_va;
            break;
        case MP16_MAC_FTM_WHITELIST_ADDR_MSB_0_REG:
            hal_device->ftm_regs.white_addr_msb = (uintptr_t)host_va;
            break;
        default:
            oam_warning_log1(0, OAM_SF_FTM, "{mac_regs_ftm_transfer::reg addr[%x] not used.}", reg_addr);
            return OAL_FAIL;
    }
#endif
    return OAL_SUCC;
}

static uint32_t mp16_phy_regs_ftm_transfer(hal_host_device_stru *hal_device, uint32_t reg_addr)
{
    uint64_t host_va = 0;
    uint32_t offset  = mp16_regs_addr_get_offset(hal_device->device_id, reg_addr);
    if (oal_pcie_devca_to_hostva(HCC_EP_WIFI_DEV, reg_addr + offset, &host_va) != OAL_SUCC) {
        oam_error_log1(0, OAM_SF_ANY, "{phy_regs_ftm_transfer::regaddr[%x] devca2hostva fail.}", reg_addr);
        return OAL_FAIL;
    }
#ifndef _PRE_LINUX_TEST
    switch (reg_addr) {
        case MP16_PHY_GLB_REG_BANK_FTM_CFG_REG:
            hal_device->ftm_regs.ftm_cfg_reg = (uintptr_t)host_va;
            break;
        case MP16_PHY_GLB_REG_BANK_PHY_BW_MODE_REG:
            hal_device->ftm_regs.ftm_phy_bw_mode = (uintptr_t)host_va;
            break;
        case MP16_CALI_TEST_REG_BANK_0_FIFO_FORCE_EN_REG:
            hal_device->ftm_regs.fifo_force_reg = (uintptr_t)host_va;
            break;
        case MP16_RX_AGC_REG_BANK_CFG_AGC_WEAK_SIG_EN_REG:
            hal_device->ftm_regs.ftm_gc_weak_sig_en = (uintptr_t)host_va;
            break;
        case MP16_RF_CTRL_REG_BANK_0_CFG_ONLINE_CALI_USE_RF_REG_EN_REG:
            hal_device->ftm_regs.ftm_cali_reg = (uintptr_t)host_va;
            break;
        default:
            oam_warning_log1(0, OAM_SF_FTM, "{phy_regs_ftm_transfer::reg addr[%x] not used.}", reg_addr);
            return OAL_FAIL;
    }
#endif
    return OAL_SUCC;
}
/*
 * 功能描述  : 地址转换
 * 1.日    期  : 2020年11月12日
 *   作    者  : wifi
 *   修改内容  : 新生成函数
 */
uint32_t mp16_host_ftm_reg_init(uint8_t hal_dev_id)
{
    uint32_t idx;
    uint32_t hal_mac_regs[] = {
        MP16_MAC_FTM_PROCESS_REG,
        MP16_MAC_FTM_BUF_BASE_ADDR_LSB_REG,
        MP16_MAC_FTM_BUF_BASE_ADDR_MSB_REG,
        MP16_MAC_FTM_WHITELIST_ADDR_LSB_0_REG,
        MP16_MAC_FTM_WHITELIST_ADDR_MSB_0_REG,
        MP16_MAC_FTM_INFO_ADDR_LSB_REG,
        MP16_MAC_FTM_INFO_ADDR_MSB_REG,
    };
    uint32_t hal_phy_regs[] = {
        MP16_PHY_GLB_REG_BANK_FTM_CFG_REG,
        MP16_RF_CTRL_REG_BANK_0_CFG_ONLINE_CALI_USE_RF_REG_EN_REG,
        MP16_CALI_TEST_REG_BANK_0_FIFO_FORCE_EN_REG,
        MP16_PHY_GLB_REG_BANK_PHY_BW_MODE_REG,
        MP16_RX_AGC_REG_BANK_CFG_AGC_WEAK_SIG_EN_REG,
    };
    hal_host_device_stru *hal_device  = hal_get_host_device(hal_dev_id);
    if (hal_device == NULL) {
        oam_error_log0(0, 0, "host_ftm_reg_init::haldev null!");
        return OAL_FAIL;
    }

    /* 唤醒低功耗内存 */
    hal_pm_try_wakeup_forbid_sleep();
    if (hal_pm_try_wakeup_dev_lock() != OAL_SUCC) {
        hal_pm_try_wakeup_allow_sleep();
        return OAL_FAIL;
    }

    if (!hal_device->ftm_regs.inited) {
        for (idx = 0; idx < sizeof(hal_mac_regs) / sizeof(uint32_t); idx++) {
            if (mp16_mac_regs_ftm_transfer(hal_device, hal_mac_regs[idx]) != OAL_SUCC) {
                hal_pm_try_wakeup_allow_sleep();
                return OAL_FAIL;
            }
        }
        for (idx = 0; idx < sizeof(hal_phy_regs) / sizeof(uint32_t); idx++) {
            if (mp16_phy_regs_ftm_transfer(hal_device, hal_phy_regs[idx]) != OAL_SUCC) {
                hal_pm_try_wakeup_allow_sleep();
                return OAL_FAIL;
            }
        }
    }
    hal_pm_try_wakeup_allow_sleep();
    hal_device->ftm_regs.inited = OAL_TRUE;
    oam_warning_log0(0, OAM_SF_ANY, "{host_ftm_reg_init::regs addr trans succ.}");

    return OAL_SUCC;
}

/*
 * 功能描述  : 把数字转化成地址
 * 1.日    期  : 2019年08月13日
 *   作    者  : wifi
 *   修改内容  : 新生成函数
 */
static void mp16_ftm_get_dscr_addr(uint64_t value, uint8_t *addr)
{
    addr[MAC_ADDR_5] = (uint8_t)(value & 0xff);                    /* macaddr arr idx 0 */
    addr[MAC_ADDR_4] = (uint8_t)((value >> BIT_OFFSET_8) & 0xff);  /* macaddr arr idx 1 */
    addr[MAC_ADDR_3] = (uint8_t)((value >> BIT_OFFSET_16) & 0xff); /* macaddr arr idx 2 */
    addr[MAC_ADDR_2] = (uint8_t)((value >> BIT_OFFSET_24) & 0xff); /* macaddr arr idx 3 */
    addr[MAC_ADDR_1] = (uint8_t)((value >> BIT_OFFSET_32) & 0xff); /* macaddr arr idx 4 */
    addr[MAC_ADDR_0] = (uint8_t)((value >> BIT_OFFSET_40) & 0xff); /* macaddr arr idx 5 */
}

/*
 * 功能描述   : 根据HAL层VAP标识获取hMAC层VAP信息
 * 1.日    期   : 2020年20月26日
 *   作    者   : wifi
 *   修改内容   : 创建函数
 */
hmac_vap_stru *mp16_host_vap_get_by_hal_vap_id(uint8_t hal_device_id, uint8_t hal_vap_id)
{
    uint8_t mac_vap_id;
    hal_host_device_stru *hal_device = hal_get_host_device(hal_device_id);

    if (hal_device == NULL) {
        oam_error_log0(0, 0, "{host_vap_get_by_hal_vap_id::hal_device null.}");
        return NULL;
    }
    if ((hal_vap_id == MP16_OTHER_BSS_ID) || (hal_vap_id < MP16_HAL_VAP_OFFSET) ||
        (hal_vap_id >= (HAL_MAX_VAP_NUM + MP16_HAL_VAP_OFFSET))) {
        return NULL;
    }

    hal_vap_id -= MP16_HAL_VAP_OFFSET;
    mac_vap_id = hal_device->hal_vap_sts_info[hal_vap_id].mac_vap_id;
    /* 2. 获取hMAC层VAP信息 */
    return (hmac_vap_stru *)mac_res_get_hmac_vap(mac_vap_id);
}

uint32_t mp16_host_ftm_get_info(hal_ftm_info_stru *hal_ftm_info, uint8_t *addr)
{
    uint8_t idx;
    uint32_t *ftm_power_output = NULL;
    volatile uint32_t *ftm_power_input = NULL;
    mp16_ftm_dscr_stru *mp16_ftm_dscr = NULL;

    /* 获取hal_ftm_info的地址 */
    mp16_ftm_dscr = (mp16_ftm_dscr_stru *)addr;
    if (mp16_ftm_dscr == NULL) {
        return OAL_FAIL;
    }

    /* 唤醒低功耗内存 */
    hal_pm_try_wakeup_forbid_sleep();
    if (hal_pm_try_wakeup_dev_lock() != OAL_SUCC) {
        hal_pm_try_wakeup_allow_sleep();
        return OAL_FAIL;
    }

    /* 2. 存储部分结果 */
    hal_ftm_info->ftm_vap_index = mp16_ftm_dscr->word4.bit_vap_index;
    hal_ftm_info->ftm_init_resp_flag = mp16_ftm_dscr->word10.bit_init_resp_flag;
    hal_ftm_info->ftm_intp_time = mp16_ftm_dscr->word10.bit_intp_time;
    hal_ftm_info->ftm_rx_time = mp16_ftm_dscr->word9.rx_time;
    hal_ftm_info->ftm_tx_time = mp16_ftm_dscr->word8.tx_time;
    hal_ftm_info->ftm_dialog_token = mp16_ftm_dscr->word4.bit_dialog_token;
    hal_ftm_info->ftm_timestamp = mp16_ftm_dscr->word7.timestamp;

    mp16_ftm_get_dscr_addr(mp16_ftm_dscr->word5.ra, &hal_ftm_info->ftm_ra[MAC_ADDR_0]);
    mp16_ftm_get_dscr_addr(mp16_ftm_dscr->word6.ta, &hal_ftm_info->ftm_ta[MAC_ADDR_0]);

    for (idx = 0; idx < HAL_HOST_MAX_RF_NUM; idx++) {
        hal_ftm_info->ftm_snr_ant[idx] = mp16_ftm_dscr->word1.snr_ant[idx];
        hal_ftm_info->ftm_rssi_ant[idx] = mp16_ftm_dscr->word1.rssi_hltf[idx];
    }
    hal_ftm_info->ftm_phase_incr_ant = mp16_ftm_dscr->word4.bit_phase_incr;
    ftm_power_input = (uint32_t *)((uint8_t *)mp16_ftm_dscr + sizeof(mp16_ftm_dscr_stru));
    ftm_power_output = &hal_ftm_info->ftm_power[0].reg_value;

     /* 直接存储ftm ppu字段里面的相关峰信息，
       相关峰字段的顺序，位宽是16bit：max_next, max, max_pre1, max_pre2, max_pre3……max_pre62，
       先收的数据放低位，后收到的数据放高位组成一个word */
    for (idx = 0; idx < FTM_MAX_POWER_NUM; idx++) {
        *ftm_power_output++ = *ftm_power_input++;
    }
    hal_pm_try_wakeup_allow_sleep();
    return OAL_SUCC;
}

/*
 * 功能描述  : 获取ftm的分频系数，以160M时钟为基准，
 *  mp16 PHY的时钟场景：
 *  PHY_BW_MODE 20M/40M 时， 上报的T值是以320M 时钟计数的，分频系数为2
 *  PHY_BW_MODE 80M/160M 时，上报的T值是以640M 时钟计数的，分频系数为4
 * 1.日    期  : 2020年12月14日
 *   作    者  : wifi
 *   修改内容  : 新生成函数
 */
void mp16_host_ftm_get_divider(hal_host_device_stru *hal_device, uint8_t *divider)
{
    uint32_t reg_value;

    /* 唤醒低功耗内存 */
    hal_pm_try_wakeup_forbid_sleep();
    if (hal_pm_try_wakeup_dev_lock() != OAL_SUCC) {
        hal_pm_try_wakeup_allow_sleep();
        return;
    }

    reg_value = hal_host_readl_irq(hal_device->ftm_regs.ftm_phy_bw_mode);
    hal_pm_try_wakeup_allow_sleep();
    switch (reg_value) {
        case 0x0:  // phymode 20, clk = 4x 160
        case 0x1:  // phymode 40, clk = 4x 160
            *divider = DIVIDER_2CLK; // 分频系数4
            break;

        case 0x2:  // phymode 80, clk = 4x 160
        case 0x3:  // phymode 160, clk = 4x 160
            *divider = DIVIDER_2CLK; // 分频系数4
            break;

        default:
            *divider = DIVIDER_2CLK; // 分频系数4
            break;
    }
}

/*
 * 功能描述  : 打开或关闭phy的ftm采样开关
 * 1.日    期  : 2020年12月14日
 *   作    者  : wifi
 *   修改内容  : 新生成函数
 */
void mp16_host_ftm_set_sample(hal_host_device_stru *hal_device, oal_bool_enum_uint8 ftm_status)
{
    /* 唤醒低功耗内存 */
    hal_pm_try_wakeup_forbid_sleep();
    if (hal_pm_try_wakeup_dev_lock() != OAL_SUCC) {
        hal_pm_try_wakeup_allow_sleep();
        return;
    }

    /* 设置FTM采集使能开关 */
    oal_setl_bits((void *)hal_device->ftm_regs.ftm_pro_reg,
                  MP16_MAC_CFG_FTM_EN_OFFSET, MP16_MAC_CFG_FTM_EN_LEN,
                  ftm_status);
    hal_pm_try_wakeup_allow_sleep();
}

/*
 * 功能描述  : 设置ftm enable
 * 1.日    期  : 2020年12月14日
 *   作    者  : wifi
 *   修改内容  : 新生成函数
 */
void mp16_host_ftm_set_enable(hal_host_device_stru *hal_device, oal_bool_enum_uint8 ftm_status)
{
    /* 唤醒低功耗内存 */
    hal_pm_try_wakeup_forbid_sleep();
    if (hal_pm_try_wakeup_dev_lock() != OAL_SUCC) {
        hal_pm_try_wakeup_allow_sleep();
        return;
    }

     /* 置位 MAC FTM enable位 */
    oal_setl_bits((void *)hal_device->ftm_regs.ftm_pro_reg,
                  MP16_MAC_CFG_FTM_EN_OFFSET, MP16_MAC_CFG_FTM_EN_LEN,
                  ftm_status);
    /* 置位 关闭弱干扰位 */
    if (ftm_status == OAL_TRUE) {
        oal_setl_bits((void *)hal_device->ftm_regs.ftm_gc_weak_sig_en,
                      MP16_RX_AGC_REG_BANK_CFG_WEAK_SIG_IMMUNITY_EN_OFFSET,
                      MP16_RX_AGC_REG_BANK_CFG_WEAK_SIG_IMMUNITY_EN_LEN,
                      OAL_FALSE);
    }

    /* 置位 PHY FTM enable位 */
    oal_setl_bits((void *)hal_device->ftm_regs.ftm_cfg_reg,
                  MP16_PHY_GLB_REG_BANK_CFG_FTM_EN_OFFSET, MP16_PHY_GLB_REG_BANK_CFG_FTM_EN_LEN,
                  ftm_status);

    /* 置位 PHY FTM cali enable位 */
    oal_setl_bits((void *)hal_device->ftm_regs.ftm_cali_reg,
                  MP16_RF_CTRL_REG_BANK_0_CFG_FTM_CALI_WR_RF_REG_EN_OFFSET,
                  MP16_RF_CTRL_REG_BANK_0_CFG_FTM_CALI_WR_RF_REG_EN_LEN,
                  ftm_status);
    hal_pm_try_wakeup_allow_sleep();
}

/*
 * 功能描述  : 设置ftm PHY接收通路
 * 1.日    期  : 2020年12月14日
 *   作    者  : wifi
 *   修改内容  : 新生成函数
 */
void mp16_host_ftm_set_m2s_phy(hal_host_device_stru *hal_device, uint8_t tx_chain_selection,
    uint8_t tx_rssi_selection)
{
    uint8_t tx_selection;
    uint32_t ftm_cap = mac_ftm_get_cap();

    tx_selection = ((ftm_cap & FTM_SWITCH_CAP) == 0 ? tx_chain_selection : tx_rssi_selection);
    if (tx_selection >= HAL_HOST_MAX_RF_NUM) {  /* 最大4通道 */
        return;
    }

    /* 唤醒低功耗内存 */
    hal_pm_try_wakeup_forbid_sleep();
    if (hal_pm_try_wakeup_dev_lock() != OAL_SUCC) {
        hal_pm_try_wakeup_allow_sleep();
        return;
    }
    oal_setl_bits((void *)hal_device->ftm_regs.ftm_cfg_reg,
                  MP16_PHY_GLB_REG_BANK_CFG_FTM_CALI_CH_SEL_OFFSET,
                  MP16_PHY_GLB_REG_BANK_CFG_FTM_CALI_CH_SEL_LEN,
                  tx_selection);
    oal_setl_bits((void *)hal_device->ftm_regs.fifo_force_reg,
                  MP16_CALI_TEST_REG_BANK_0_CFG_REG_TX_FIFO_EN_OFFSET,
                  MP16_CALI_TEST_REG_BANK_0_CFG_REG_TX_FIFO_EN_LEN, BIT(tx_selection));
    hal_pm_try_wakeup_allow_sleep();
    oam_warning_log1(0, OAM_SF_ANY,
                     "set_ftm_m2s_phy:: tx_selection=[%d]", tx_selection);
}

/*
 * 功能描述  : 设置提供给芯片使用的ftm缓冲区基址
 * 1.日    期  : 2020年12月14日
 *   作    者  : wifi
 *   修改内容  : 新生成函数
 */
void mp16_host_ftm_set_buf_base_addr(hal_host_device_stru *hal_device, uintptr_t devva)
{
    if (devva == 0) {
        oam_error_log0(0, 0, "host_ftm_set_buf_base_addr::devva is empty");
        return;
    }
    /* 唤醒低功耗内存 */
    hal_pm_try_wakeup_forbid_sleep();
    if (hal_pm_try_wakeup_dev_lock() != OAL_SUCC) {
        hal_pm_try_wakeup_allow_sleep();
        return;
    }
    hal_host_writel(get_low_32_bits(devva), hal_device->ftm_regs.base_lsb);
    hal_host_writel(get_high_32_bits(devva), hal_device->ftm_regs.base_msb);
    hal_pm_try_wakeup_allow_sleep();
}

/*
 * 功能描述  : 设置ftm缓冲区size
 * 1.日    期  : 2020年12月14日
 *   作    者  : wifi
 *   修改内容  : 新生成函数
 */
void mp16_host_ftm_set_buf_size(hal_host_device_stru *hal_device, uint16_t cfg_ftm_buf_size)
{
    /* 唤醒低功耗内存 */
    hal_pm_try_wakeup_forbid_sleep();
    if (hal_pm_try_wakeup_dev_lock() != OAL_SUCC) {
        hal_pm_try_wakeup_allow_sleep();
        return;
    }
    /* 设置FTM采样上报片外循环BUF深度 */
    oal_setl_bits((void *)hal_device->ftm_regs.ftm_pro_reg,
                  MP16_MAC_CFG_FTM_BUF_SIZE_OFFSET,
                  MP16_MAC_CFG_FTM_BUF_SIZE_LEN, cfg_ftm_buf_size);
    hal_pm_try_wakeup_allow_sleep();
}

/*
 * 功能描述  : 设置ftm白名单
 * 1.日    期  : 2020年12月14日
 *   作    者  : wifi
 *   修改内容  : 新生成函数
 */
uint32_t mp16_host_ftm_set_white_list(hal_host_device_stru *hal_device, uint8_t idx,
    uint8_t *mac_addr)
{
    uint32_t reg_value;
    uint32_t addr_lsb = 0;
    uint32_t addr_msb = 0;

    if (idx >= FTM_WHITELIST_ADDR_SIZE) { /* 偏移31个以上寄存器地址，超出了ftm寄存器地址偏移范围 */
        return OAL_FAIL;
    }
    /* 唤醒低功耗内存 */
    hal_pm_try_wakeup_forbid_sleep();
    if (hal_pm_try_wakeup_dev_lock() != OAL_SUCC) {
        hal_pm_try_wakeup_allow_sleep();
        return OAL_FAIL;
    }

    reg_value = (uint32_t)mac_addr[MAC_ADDR_5];
    addr_lsb |= (reg_value & 0x000000FF);
    reg_value = (uint32_t)mac_addr[MAC_ADDR_4];
    addr_lsb |= ((reg_value & 0x000000FF) << BIT_OFFSET_8);
    reg_value = (uint32_t)mac_addr[MAC_ADDR_3];
    addr_lsb |= ((reg_value & 0x000000FF) << BIT_OFFSET_16);
    reg_value = (uint32_t)mac_addr[MAC_ADDR_2];
    addr_lsb |= ((reg_value & 0x000000FF) << BIT_OFFSET_24);
    reg_value = (uint32_t)mac_addr[MAC_ADDR_1];
    addr_msb |= (reg_value & 0x000000FF);
    reg_value = (uint32_t)mac_addr[MAC_ADDR_0];
    addr_msb |= ((reg_value & 0x000000FF) << BIT_OFFSET_8);

    /* 以FTM采样白名单低位寄存器起始地址为基地址，偏移8 * idx个字节 */
    hal_host_writel(addr_lsb, hal_device->ftm_regs.white_addr_lsb + 8 * idx);

    /* 以FTM采样白名单高位寄存器起始地址为基地址，偏移8 * idx个字节 */
    hal_host_writel(addr_msb, hal_device->ftm_regs.white_addr_msb + 8 * idx);
    hal_pm_try_wakeup_allow_sleep();
    return OAL_SUCC;
}

static void mp16_host_ftm_copy_device_stru_to_host_stru(hal_ftm_info_device_stru *hal_ftm_info_device,
    hal_ftm_info_stru *hal_ftm_info)
{
    int i;

    memset_s(hal_ftm_info, sizeof(hal_ftm_info_stru), sizeof(hal_ftm_info_stru), 0);
    hal_ftm_info->device_id = hal_ftm_info_device->device_id;
    hal_ftm_info->ftm_init_resp_flag = hal_ftm_info_device->ftm_init_resp_flag;
    hal_ftm_info->ftm_intp_time = hal_ftm_info_device->ftm_intp_time;
    hal_ftm_info->ftm_frame_type = hal_ftm_info_device->ftm_frame_type;
    hal_ftm_info->ftm_vap_index = hal_ftm_info_device->ftm_vap_index;
    hal_ftm_info->ftm_dialog_token = hal_ftm_info_device->ftm_dialog_token;

    for (i = 0; i < WLAN_MAC_ADDR_LEN; i++) {
        hal_ftm_info->ftm_ra[i] = hal_ftm_info_device->ftm_ra[i];
        hal_ftm_info->ftm_ta[i] = hal_ftm_info_device->ftm_ta[i];
    }

    for (i = 0; i < HAL_HOST_MP16_RF_NUM; i++) {
        hal_ftm_info->ftm_snr_ant[i] = hal_ftm_info_device->ftm_snr_ant[i];
        hal_ftm_info->ftm_rssi_ant[i] = hal_ftm_info_device->ftm_rssi_ant[i];
    }

    hal_ftm_info->ftm_info_len = hal_ftm_info_device->ftm_info_len;
    hal_ftm_info->ftm_phase_incr_ant = hal_ftm_info_device->ftm_phase_incr_ant;
    hal_ftm_info->ftm_rx_time = hal_ftm_info_device->ftm_rx_time;
    hal_ftm_info->ftm_tx_time = hal_ftm_info_device->ftm_tx_time;
    hal_ftm_info->ftm_timestamp = hal_ftm_info_device->ftm_timestamp;

    for (i = 0; i < FTM_MAX_POWER_NUM; i++) {
        hal_ftm_info->ftm_power[i].reg_value = hal_ftm_info_device->ftm_power[i].reg_value;
    }
}

/*
 * 功能描述  : 将dmac传上来的ftm结构体转换为hmac格式
 * 1.日    期  : 2022年7月14日
 *   修改内容  : 新生成函数
 */
void mp16_host_ftm_transfer_device_stru_to_host_stru(uint8_t *device_param, uint8_t *host_param)
{
    hal_ftm_info_device_stru *hal_ftm_info_device = (hal_ftm_info_device_stru *)device_param;
    hal_ftm_info_stru *hal_ftm_info = (hal_ftm_info_stru *)host_param;

    mp16_host_ftm_copy_device_stru_to_host_stru(hal_ftm_info_device, hal_ftm_info);
}

/*
 * 功能描述  : 根据dmac传上来的ftm参数，找到最大的rssi
 * 1.日    期  : 2022年7月14日
 *   修改内容  : 新生成函数
 */
void mp16_host_ftm_get_rssi_selection(uint8_t *hmac_ftm, uint8_t *device_param)
{
    uint8_t chan_idx;
    int8_t rssi_value = OAL_RSSI_INIT_VALUE;
    hal_ftm_info_device_stru *hal_ftm_info_device = (hal_ftm_info_device_stru *)device_param;
    hmac_ftm_stru *hmac_ftm_temp = (hmac_ftm_stru *)hmac_ftm;

    hmac_ftm_temp->ftm_rssi_selection = 0;

    for (chan_idx = 0; chan_idx < HAL_HOST_MP16_RF_NUM; ++chan_idx) {
        if (hal_ftm_info_device->ftm_rssi_ant[chan_idx] > rssi_value) {
            rssi_value = hal_ftm_info_device->ftm_rssi_ant[chan_idx];
            hmac_ftm_temp->ftm_rssi_selection = chan_idx;
        }
    }
}
#endif
