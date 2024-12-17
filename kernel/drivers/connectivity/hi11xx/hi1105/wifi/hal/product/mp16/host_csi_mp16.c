/*
* Copyright (c) @CompanyNameMagicTag 2021-2023. All rights reserved.
* 功能说明   : CSI功能
* 作者       : wifi
* 创建日期   : 2020年11月17日
*/
#ifdef _PRE_WLAN_FEATURE_CSI

/* 1 头文件包含 */
#include "mac_data.h"
#include "hmac_resource.h"
#include "hmac_host_tx_data.h"
#include "hmac_tx_data.h"
#include "hmac_config.h"
#include "host_mac_mp16.h"
#include "host_csi_mp16.h"
#include "host_hal_access_mgmt.h"
#include "hmac_tx_msdu_dscr.h"

#include "securec.h"
#include "securectype.h"

#undef THIS_FILE_ID
#define THIS_FILE_ID OAM_FILE_ID_HOST_HAL_CSI_MP16_C

/*
 * 功能描述  : 地址转换
 * 1.日    期  : 2020年11月17日
 *   作    者  : wifi
 *   修改内容  : 新生成函数
 */
static uint32_t mp16_regs_csi_transfer(hal_host_device_stru *hal_device, uint32_t reg_addr)
{
    uint64_t host_va = 0;
    uint32_t offset = mp16_regs_addr_get_offset(hal_device->device_id, reg_addr);
    if (oal_pcie_devca_to_hostva(HCC_EP_WIFI_DEV, reg_addr + offset, &host_va) != OAL_SUCC) {
        oam_error_log1(0, OAM_SF_ANY, "{regs_csi_transfer::regaddr[%x] devca2hostva fail.}", reg_addr);
        return OAL_FAIL;
    }
#ifndef _PRE_LINUX_TEST
    switch (reg_addr) {
        case MP16_MAC_CSI_BUF_BASE_ADDR_LSB_REG:
            hal_device->csi_regs.base_lsb = (uintptr_t)host_va;
            break;
        case MP16_MAC_CSI_BUF_BASE_ADDR_MSB_REG:
            hal_device->csi_regs.base_msb = (uintptr_t)host_va;
            break;
        case MP16_MAC_CSI_BUF_SIZE_REG:
            hal_device->csi_regs.size = (uintptr_t)host_va;
            break;
        case MP16_MAC_CSI_PROCESS_REG:
            hal_device->csi_regs.csi_pro_reg = (uintptr_t)host_va;
            break;
        case MP16_RX_CHN_EST_REG_BANK_CHN_EST_COM_REG:
            hal_device->csi_regs.chn_set = (uintptr_t)host_va;
            break;
        case MP16_MAC_RX_CTRL_REG:
            hal_device->csi_regs.rx_ctrl_reg = (uintptr_t)host_va;
            break;
        case MP16_MAC_CSI_WHITELIST_ADDR_LSB_0_REG:
            hal_device->csi_regs.white_addr_lsb = (uintptr_t)host_va;
            break;
        case MP16_MAC_CSI_WHITELIST_ADDR_MSB_0_REG:
            hal_device->csi_regs.white_addr_msb = (uintptr_t)host_va;
            break;
        case MP16_MAC_LOCATION_INFO_MASK_REG:
            hal_device->csi_regs.location_info = (uintptr_t)host_va;
            break;
        case MP16_MAC_CSI_INFO_ADDR_LSB_REG:
            hal_device->csi_regs.csi_info_lsb = (uintptr_t)host_va;
            break;
        case MP16_MAC_CSI_INFO_ADDR_MSB_REG:
            hal_device->csi_regs.csi_info_msb = (uintptr_t)host_va;
            break;
        default:
            oam_warning_log1(0, OAM_SF_ANY, "{regs_csi_transfer::reg addr[%x] not used.}", reg_addr);
            return OAL_FAIL;
    }
#endif
    return OAL_SUCC;
}

/*
 * 功能描述  : 地址转换
 * 1.日    期  : 2020年11月17日
 *   作    者  : wifi
 *   修改内容  : 新生成函数
 */
void mp16_csi_base_addr_init(hal_host_device_stru *hal_device)
{
    hal_device->csi_regs.devva = 0;
    hal_device->csi_regs.inited = OAL_FALSE;
    hal_device->csi_regs.rbase_dma_addr = 0;
    hal_device->csi_regs.rbase_vaddr = NULL;
}
/*
 * 功能描述  : 地址转换
 * 1.日    期  : 2020年11月17日
 *   作    者  : wifi
 *   修改内容  : 新生成函数
 */
uint32_t mp16_csi_reg_init(hal_host_device_stru *hal_device)
{
    uint32_t idx;
    uint32_t hal_regs[] = {
        MP16_MAC_CSI_BUF_BASE_ADDR_LSB_REG,
        MP16_MAC_CSI_BUF_BASE_ADDR_MSB_REG,
        MP16_MAC_CSI_BUF_SIZE_REG,
        MP16_MAC_CSI_PROCESS_REG,
        MP16_RX_CHN_EST_REG_BANK_CHN_EST_COM_REG,
        MP16_MAC_RX_CTRL_REG,
        MP16_MAC_CSI_WHITELIST_ADDR_LSB_0_REG,
        MP16_MAC_CSI_WHITELIST_ADDR_MSB_0_REG,
        MP16_MAC_LOCATION_INFO_MASK_REG,
        MP16_MAC_CSI_INFO_ADDR_LSB_REG,
        MP16_MAC_CSI_INFO_ADDR_MSB_REG,
    };

    if (hal_device->device_id >= HAL_DEVICE_ID_BUTT) {
        oam_error_log1(0, OAM_SF_ANY, "{csi_reg_init::device_id[%d].}", hal_device->device_id);
        return OAL_FAIL;
    }

    if (hal_device->csi_regs.inited == OAL_FALSE) {
        for (idx = 0; idx < sizeof(hal_regs) / sizeof(uint32_t); idx++) {
            if (OAL_SUCC != mp16_regs_csi_transfer(hal_device, hal_regs[idx])) {
                hal_device->csi_regs.inited = OAL_FALSE;
                return OAL_FAIL;
            }
        }
        hal_device->csi_regs.inited = OAL_TRUE;
        oam_warning_log0(0, OAM_SF_ANY, "{csi_reg_init::regs addr trans succ.}");
    }
    return OAL_SUCC;
}

/*
 * 函 数 名  : mp16_host_set_csi_buf_base_addr
 * 功能描述  : 设置CSI采样上报片外循环BUF基地址
 * 1.日    期  : 2020年11月17日
 *   作    者  : wifi
 *   修改内容  : 新生成函数
 */
void mp16_host_set_csi_buf_base_addr(hal_host_device_stru *hal_device, uintptr_t devva)
{
    if (devva == 0) {
        oam_error_log0(0, 0, "host_set_csi_buf_base_addr::devva is empty");
        return;
    }
    hal_host_writel(get_low_32_bits(devva), hal_device->csi_regs.base_lsb);
    hal_host_writel(get_high_32_bits(devva), hal_device->csi_regs.base_msb);
}

/*
 * 函 数 名  : mp16_host_set_csi_buf_size
 * 功能描述  : 设置CSI采样上报片外循环BUF大小
 * 1.日    期  : 2020年11月17日
 *   作    者  : wifi
 *   修改内容  : 新生成函数
 */
void mp16_host_set_csi_buf_size(hal_host_device_stru *hal_device, uint16_t cfg_csi_buf_size)
{
    hal_host_writel(cfg_csi_buf_size, (uintptr_t)hal_device->csi_regs.size);
}

/*
 * 函 数 名  : mp16_host_set_csi_en
 * 功能描述  : 开启和关闭CSI
 * 1.日    期  : 2020年11月17日
 *   作    者  : wifi
 *   修改内容  : 新生成函数
 */
void mp16_host_set_csi_en(hal_host_device_stru *hal_device, uint32_t reg_value)
{
    oal_setl_bits((void *)hal_device->csi_regs.csi_pro_reg,
                  MP16_MAC_CFG_CSI_EN_OFFSET,
                  MP16_MAC_CFG_CSI_EN_LEN, reg_value);            /* CSI采集使能 */
    oal_setl_bits((void *)hal_device->csi_regs.csi_pro_reg,
                  MP16_MAC_CFG_CSI_H_MATRIX_RPT_EN_OFFSET,
                  MP16_MAC_CFG_CSI_H_MATRIX_RPT_EN_LEN, 0); /* 上报h矩阵 */
    oal_setl_bits((void *)hal_device->csi_regs.chn_set,
                  MP16_RX_CHN_EST_REG_BANK_CFG_CHN_CONDITION_NUM_RPT_EN_OFFSET,
                  MP16_RX_CHN_EST_REG_BANK_CFG_CHN_CONDITION_NUM_RPT_EN_LEN, reg_value);
}

/*
 * 函 数 名  : mp16_host_set_phy_info_en
 * 功能描述  : 设置phy info上报使能
 * 1.日    期  : 2020年11月17日
 *   作    者  : wifi
 *   修改内容  : 新生成函数
 */
void mp16_host_set_phy_info_en(hal_host_device_stru *hal_device, uint32_t reg_value)
{
    oal_setl_bits((void *)(uintptr_t)hal_device->csi_regs.rx_ctrl_reg,
                  MP16_MAC_CFG_RX_PPDU_DESC_MODE_OFFSET, MP16_MAC_CFG_RX_PPDU_DESC_MODE_LEN, reg_value);
}

/*
 * 函 数 名  : mp16_host_set_csi_bw
 * 功能描述  : 设置CSI采集的带宽
 * 1.日    期  : 2020年11月17日
 *   作    者  : wifi
 *   修改内容  : 新生成函数
 */
void mp16_host_set_csi_bw(hal_host_device_stru *hal_device, uint32_t reg_value)
{
    oal_setl_bits((void *)hal_device->csi_regs.csi_pro_reg,
                  MP16_MAC_CFG_CSI_BAND_WIDTH_MASK_OFFSET, MP16_MAC_CFG_CSI_BAND_WIDTH_MASK_LEN,
                  reg_value);
}

/*
 * 函 数 名  : mp16_host_set_csi_frame_type
 * 功能描述  : 针对以下帧类型进行CSI采集上报
 * 1.日    期  : 2020年11月17日
 *   作    者  : wifi
 *   修改内容  : 新生成函数
 */
void mp16_host_set_csi_frame_type(hal_host_device_stru *hal_device, uint32_t reg_value)
{
    oal_setl_bits((void *)hal_device->csi_regs.csi_pro_reg,
                  MP16_MAC_CFG_CSI_FRAME_TYPE_MASK_OFFSET, MP16_MAC_CFG_CSI_FRAME_TYPE_MASK_LEN,
                  reg_value);
}
/*
 * 函 数 名  : mp16_host_set_csi_extrat_en
 * 功能描述  : 设置CSI抽取,extra_en置一时,只有160M会抽取
 * 1.日    期  : 2020年11月17日
 *   作    者  : wifi
 *   修改内容  : 新生成函数
 */
void mp16_host_set_csi_extrat_en(hal_host_device_stru *hal_device, uint32_t reg_value)
{
    oal_setl_bits((void *)hal_device->csi_regs.csi_pro_reg,
                  MP16_MAC_CFG_CSI_SAMPLE_PERIOD_OFFSET, MP16_MAC_CFG_CSI_SAMPLE_PERIOD_LEN,
                  reg_value);
}

/*
 * 函 数 名  : mp16_host_set_csi_sample_period
 * 功能描述  : 设置CSI采样周期
 * 1.日    期  : 2020年11月17日
 *   作    者  : wifi
 *   修改内容  : 新生成函数
 */
void mp16_host_set_csi_sample_period(hal_host_device_stru *hal_device, uint32_t reg_value)
{
    oal_setl_bits((void *)hal_device->csi_regs.csi_pro_reg,
                  MP16_MAC_CFG_CSI_SAMPLE_PERIOD_OFFSET, MP16_MAC_CFG_CSI_SAMPLE_PERIOD_LEN,
                  reg_value);
}

/*
 * 函 数 名  : mp16_host_set_csi_ta
 * 功能描述  : 设置csi上报帧的TA，TA匹配时才上报CSI信息
 * 1.日    期  : 2020年11月17日
 *   作    者  : wifi
 *   修改内容  : 新生成函数
 */
/* 当前只配置到csi_num=0的寄存器中，总共有32对TA寄存器 */
void mp16_host_set_csi_ta(hal_host_device_stru *hal_device, uint8_t *mac_addr)
{
    uint32_t bssid_msb; /* 高16位 */
    uint32_t bssid_lsb; /* 低32位 */

    bssid_msb = mac_addr[MAC_ADDR_0];
    bssid_msb = (bssid_msb << BIT_OFFSET_8) | mac_addr[MAC_ADDR_1];

    bssid_lsb = mac_addr[MAC_ADDR_2];
    bssid_lsb = (bssid_lsb << BIT_OFFSET_8) | mac_addr[MAC_ADDR_3];
    bssid_lsb = (bssid_lsb << BIT_OFFSET_8) | mac_addr[MAC_ADDR_4];
    bssid_lsb = (bssid_lsb << BIT_OFFSET_8) | mac_addr[MAC_ADDR_5];

    hal_host_writel(bssid_msb, hal_device->csi_regs.white_addr_msb);
    hal_host_writel(bssid_lsb, hal_device->csi_regs.white_addr_lsb);
}


#if (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)
/*
 * 函 数 名  : hmac_config_csi_buffer_addr
 * 功能描述  : 设置csi报文地址
 * 1.日    期  : 2020年11月17日
 *   作    者  : wifi
 *   修改内容  : 新生成函数
 */
static uint32_t mp16_config_csi_buffer_addr(hal_host_device_stru *hal_device, uint32_t cfg_csi_on)
{
    int32_t ret;
    oal_pci_dev_stru *pcie_dev = oal_get_wifi_pcie_dev();

    if (cfg_csi_on == OAL_TRUE) {
        if (hal_device->csi_regs.rbase_vaddr == NULL) {
            hal_device->csi_regs.rbase_vaddr =
                dma_alloc_coherent(&pcie_dev->dev, HMAC_CSI_BUFF_SIZE,
                                   &hal_device->csi_regs.rbase_dma_addr, GFP_KERNEL);

            if (hal_device->csi_regs.rbase_vaddr == NULL) {
                return OAL_FAIL;
            }
            memset_s(hal_device->csi_regs.rbase_vaddr, HMAC_CSI_BUFF_SIZE,
                     0, HMAC_CSI_BUFF_SIZE);

            ret = pcie_if_hostca_to_devva(HCC_EP_WIFI_DEV, (uint64_t)hal_device->csi_regs.rbase_dma_addr,
                                          &hal_device->csi_regs.devva);
            if (ret != OAL_SUCC) {
                oam_error_log0(0, 0, "hmac_config_csi_buffer_addr alloc pcie_if_hostca_to_devva fail.");
                dma_free_coherent(&pcie_dev->dev,
                                  HMAC_CSI_BUFF_SIZE,
                                  hal_device->csi_regs.rbase_vaddr,
                                  (dma_addr_t)hal_device->csi_regs.rbase_dma_addr);
                hal_device->csi_regs.rbase_vaddr = NULL;
                return OAL_FAIL;
            }

            mp16_host_set_csi_buf_base_addr(hal_device, hal_device->csi_regs.devva);
            mp16_host_set_csi_buf_size(hal_device, HMAC_CSI_BUFF_SIZE);
        }
    } else {
        if (hal_device->csi_regs.rbase_vaddr != NULL) {
            dma_free_coherent(&pcie_dev->dev,
                              HMAC_CSI_BUFF_SIZE,
                              hal_device->csi_regs.rbase_vaddr,
                              (dma_addr_t)hal_device->csi_regs.rbase_dma_addr);
            hal_device->csi_regs.rbase_vaddr = NULL;
        }
    }
    return OAL_SUCC;
}
#else
static uint32_t mp16_config_csi_buffer_addr(hal_host_device_stru *hal_device, uint32_t cfg_csi_on)
{
    int32_t ret;
    oal_pci_dev_stru  *pcie_dev = oal_get_wifi_pcie_dev();

    if (cfg_csi_on == OAL_TRUE) {
        if (hal_device->csi_regs.rbase_vaddr == NULL) {
            hal_device->csi_regs.rbase_vaddr = oal_mem_alloc_m(OAL_MEM_POOL_ID_LOCAL, HMAC_CSI_BUFF_SIZE, OAL_FALSE);

            if (hal_device->csi_regs.rbase_vaddr == NULL) {
                oam_error_log2(0, 0, "hmac_config_csi_buffer_addr:dma_alloc fail[%p]size[%d]",
                               &pcie_dev->dev, HMAC_CSI_BUFF_SIZE);
                return OAL_FAIL;
            }
            memset_s(hal_device->csi_regs.rbase_vaddr, HMAC_CSI_BUFF_SIZE,
                     0, HMAC_CSI_BUFF_SIZE);

            ret = pcie_if_hostca_to_devva(HCC_EP_WIFI_DEV, (uint64_t)hal_device->csi_regs.rbase_dma_addr,
                                          &hal_device->csi_regs.devva);
            if (ret != OAL_SUCC) {
                oam_error_log0(0, 0, "hmac_config_csi_buffer_addr alloc pcie_if_hostca_to_devva fail.");
                oal_mem_free_m(hal_device->csi_regs.rbase_vaddr, OAL_TRUE);
                hal_device->csi_regs.rbase_vaddr = NULL;
                return OAL_FAIL;
            }

            mp16_host_set_csi_buf_base_addr(hal_device, hal_device->csi_regs.devva);
            mp16_host_set_csi_buf_size(hal_device, HMAC_CSI_BUFF_SIZE);
        }
    } else {
        if (hal_device->csi_regs.rbase_vaddr != NULL) {
            oal_mem_free_m(hal_device->csi_regs.rbase_vaddr, OAL_TRUE);
            hal_device->csi_regs.rbase_vaddr = NULL;
        }
    }
    return OAL_SUCC;
}
#endif
/*
 * 功能描述  : mp16 获取ftm/csi中断上报信息
 * 1.日    期  : 2020年11月18日
 *   作    者  : wifi
 *   修改内容  : 新生成函数
 */
uint32_t mp16_get_host_ftm_csi_locationinfo(hal_host_device_stru *hal_device)
{
    return hal_host_readl_irq(hal_device->csi_regs.location_info);
}

uint32_t mp16_csi_config(mac_vap_stru *mac_vap, uint16_t len, uint8_t *param)
{
    mac_cfg_csi_param_stru *cfg_csi_param = (mac_cfg_csi_param_stru *)param;
    hmac_vap_stru *hmac_vap = NULL;
    hal_host_device_stru *hal_device = NULL;
    oal_bool_enum_uint8 cfg_csi_on;

    hmac_vap = mac_res_get_hmac_vap(mac_vap->uc_vap_id);
    if (hmac_vap == NULL) {
        return OAL_FAIL;
    }
    hal_device = hal_get_host_device(hmac_vap->hal_dev_id);
    if (oal_any_null_ptr2(cfg_csi_param, hal_device)) {
        oam_error_log0(0, OAM_SF_CFG, "{csi_config::cfg_csi_param and hal_device null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    cfg_csi_on = cfg_csi_param->en_cfg_csi_on;

    if (mp16_csi_reg_init(hal_device) != OAL_SUCC) {
        oam_error_log0(0, OAM_SF_CFG, "{csi_config::csi init regs failed.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    /* 唤醒低功耗内存 */
    hal_pm_try_wakeup_forbid_sleep();
    if (hal_pm_try_wakeup_dev_lock() != OAL_SUCC) {
        hal_pm_try_wakeup_allow_sleep();
        return OAL_FAIL;
    }

    mp16_host_mac_irq_unmask(hal_device, MP16_MAC_RPT_HOST_INTR_LOCATION_COMPLETE_OFFSET);
    mp16_host_set_csi_en(hal_device, OAL_FALSE);
    mp16_host_set_phy_info_en(hal_device, OAL_FALSE);

    /* 设置带宽 */
    mp16_host_set_csi_bw(hal_device, (uint32_t)cfg_csi_param->uc_csi_bw);

    /* 设置CSI采集的帧类型 */
    mp16_host_set_csi_frame_type(hal_device, (uint32_t)cfg_csi_param->uc_csi_frame_type);

    /* 设置CSI采样周期 */
    mp16_host_set_csi_sample_period(hal_device, (uint32_t)cfg_csi_param->uc_csi_sample_period);

    if (!mac_addr_is_zero(cfg_csi_param->auc_mac_addr)) {
        /* 设置上报csi帧的TA mac地址 */
        mp16_host_set_csi_ta(hal_device, cfg_csi_param->auc_mac_addr);
    }

    if (mp16_config_csi_buffer_addr(hal_device, !!cfg_csi_on) != OAL_SUCC) {
        hal_pm_try_wakeup_allow_sleep();
        return OAL_FAIL;
    }

    mp16_host_set_csi_en(hal_device, !!cfg_csi_on); /* !! 非零传入1, 0传入0 */
    mp16_host_set_phy_info_en(hal_device, !!cfg_csi_on);
    hal_pm_try_wakeup_allow_sleep();
    oam_warning_log4(0, OAM_SF_CFG,
        "{csi_config::cfg_csi_on:%d, bw:%d, csi_frame_type:%d, csi_sample_period:%d.}",
        cfg_csi_on, cfg_csi_param->uc_csi_bw, cfg_csi_param->uc_csi_frame_type, cfg_csi_param->uc_csi_sample_period);

    return OAL_SUCC;
}

uint32_t mp16_get_csi_info(hmac_csi_info_stru *hmac_csi_info, uint8_t *addr)
{
    mp16_csi_ppu_stru *csi_ppu = NULL;
    uint32_t timestamp;
    csi_ppu = (mp16_csi_ppu_stru *)addr;
    timestamp = csi_ppu->word7.timestamp;

    // timestamp
    hmac_csi_info->csi_data[BYTE_OFFSET_0] = (uint8_t)((timestamp & 0xFF000000) >> BIT_OFFSET_24);
    hmac_csi_info->csi_data[BYTE_OFFSET_1] = (uint8_t)((timestamp & 0xFF0000) >> BIT_OFFSET_16);
    hmac_csi_info->csi_data[BYTE_OFFSET_2] = (uint8_t)((timestamp & 0xFF00) >> BIT_OFFSET_8);
    hmac_csi_info->csi_data[BYTE_OFFSET_3] = (uint8_t)(timestamp & 0xFF);
    // bw mcs protocol
    hmac_csi_info->csi_data[BYTE_OFFSET_4] = (uint8_t)(csi_ppu->word0.bit_freq_bw);
    hmac_csi_info->csi_data[BYTE_OFFSET_5] = (uint8_t)(csi_ppu->word4.bit_nss_mcs_rate);
    hmac_csi_info->csi_data[BYTE_OFFSET_6] = (uint8_t)(csi_ppu->word4.bit_protocol_mode);
    // rssi snr
    hmac_csi_info->csi_data[BYTE_OFFSET_7] = (uint8_t)(csi_ppu->word0.rssi_lltf_ch[WLAN_SINGLE_STREAM_0]);
    hmac_csi_info->csi_data[BYTE_OFFSET_8] = (uint8_t)(csi_ppu->word0.rssi_lltf_ch[WLAN_SINGLE_STREAM_1]);
    hmac_csi_info->csi_data[BYTE_OFFSET_9] = (uint8_t)(csi_ppu->word0.rssi_lltf_ch[WLAN_SINGLE_STREAM_2]);
    hmac_csi_info->csi_data[BYTE_OFFSET_10] = (uint8_t)(csi_ppu->word0.rssi_lltf_ch[WLAN_SINGLE_STREAM_3]);
    hmac_csi_info->csi_data[BYTE_OFFSET_11] = (uint8_t)(csi_ppu->word1.rssi_hltf[WLAN_SINGLE_STREAM_0]);
    hmac_csi_info->csi_data[BYTE_OFFSET_12] = (uint8_t)(csi_ppu->word1.rssi_hltf[WLAN_SINGLE_STREAM_1]);
    hmac_csi_info->csi_data[BYTE_OFFSET_13] = (uint8_t)(csi_ppu->word1.rssi_hltf[WLAN_SINGLE_STREAM_2]);
    hmac_csi_info->csi_data[BYTE_OFFSET_14] = (uint8_t)(csi_ppu->word1.rssi_hltf[WLAN_SINGLE_STREAM_3]);
    hmac_csi_info->csi_data[BYTE_OFFSET_15] = (uint8_t)(csi_ppu->word1.snr_ant[WLAN_SINGLE_STREAM_0]);
    hmac_csi_info->csi_data[BYTE_OFFSET_16] = (uint8_t)(csi_ppu->word1.snr_ant[WLAN_SINGLE_STREAM_1]);
    hmac_csi_info->csi_data[BYTE_OFFSET_17] = (uint8_t)(csi_ppu->word1.snr_ant[WLAN_SINGLE_STREAM_2]);
    hmac_csi_info->csi_data[BYTE_OFFSET_18] = (uint8_t)(csi_ppu->word1.snr_ant[WLAN_SINGLE_STREAM_3]);
    hmac_csi_info->rpt_info_len = csi_ppu->word0.bit_rpt_info_len;
    hmac_csi_info->vap_index = csi_ppu->word4.bit_vap_index;
    return OAL_SUCC;
}

uint32_t mp16_host_ftm_csi_init(hal_host_device_stru *hal_device)
{
    mp16_csi_base_addr_init(hal_device);

    return OAL_SUCC;
}

#endif
