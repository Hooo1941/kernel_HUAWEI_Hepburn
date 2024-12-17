/*
 * Copyright (c) @CompanyNameMagicTag 2021-2023. All rights reserved.
 * 功能说明   : HOST MAC OPS接口
 * 作者       : wifi
 * 创建日期   : 2012年10月18日
 */

#ifndef HOST_MAC_MP16C_H
#define HOST_MAC_MP16C_H

#include "frw_ext_if.h"
#include "hal_common.h"
#include "host_hal_device.h"
#include "oal_ext_if.h"

/* MP16C MAC寄存器基地址 */
#define MAC_GLB_REG_BASE 0x40020000
#define MAC_RPU_REG_BASE 0x40026000
#define MAC_TMU_REG_BASE 0x40028000
#define MAC_PPU_REG_BASE 0x4002c000

/* MP16C PHY寄存器基地址 */
#define DFX_REG_BANK_BASE 0x40064000
#define RX_CHN_EST_REG_BANK_BASE 0x40072800
#define RF_CTRL_REG_BANK_0_BASE 0x40064800
#define CALI_TEST_REG_BANK_0_BASE 0x40061000
#define PHY_GLB_REG_BANK_BASE 0x40060800
#define MP16C_GLB_CTL_RB_BASE_BASE 0x40000000

/* master: 0x4002_0000~0x4002_ffff slave: 0x4004_0000~0x4004_ffff */
#define MP16C_MAC_BANK_REG_OFFSET 0x20000
/* master: 0x4006_0000~0x          slave: 0x400a_0000~0x */
#define MP16C_PHY_BANK_REG_OFFSET 0x40000

/* PHY 寄存器 */
#define DFX_REG_BANK_PHY_INTR_RPT_REG (DFX_REG_BANK_BASE + 0x110)              /* PHY中断状态上报寄存器 */
#define DFX_REG_BANK_CFG_INTR_MASK_HOST_REG (DFX_REG_BANK_BASE + 0x20)         /* PHY给host中断MASK寄存器 */
#define DFX_REG_BANK_CFG_INTR_CLR_REG (DFX_REG_BANK_BASE + 0x28)               /* PHY中断清除寄存器 */
#define CALI_TEST_REG_BANK_0_FTM_CFG_REG (CALI_TEST_REG_BANK_0_BASE + 0x220)   /* FTM配置寄存器 */
#define RX_CHN_EST_REG_BANK_CHN_EST_COM_REG (RX_CHN_EST_REG_BANK_BASE + 0x1D0) /* 信道估计通用寄存器 */
#define RF_CTRL_REG_BANK_0_CFG_ONLINE_CALI_USE_RF_REG_EN_REG (RF_CTRL_REG_BANK_0_BASE + 0x34)
#define CALI_TEST_REG_BANK_0_FIFO_FORCE_EN_REG (CALI_TEST_REG_BANK_0_BASE + 0x28) /* FIFO使能对应的寄存器配置地址 */
#define PHY_GLB_REG_BANK_PHY_BW_MODE_REG (PHY_GLB_REG_BANK_BASE + 0x4)            /* 频宽模式配置寄存器 */

/* MAC 寄存器 */
#define MAC_GLB_REG_HOST_INTR_STATUS_REG (MAC_GLB_REG_BASE + 0x1004) /* 上报到Host的中断状态寄存器 */
#define MAC_GLB_REG_BA_INFO_BUF_WPTR_REG (MAC_GLB_REG_BASE + 0x105C) /* BA INFO BUFFER的写指针上报 */
#define MAC_RPU_REG_RX_NORM_DATA_FREE_RING_RPTR_REG \
    (MAC_RPU_REG_BASE + 0x1000) /* 接收普通数据帧Free Ring的读指针上报 */
#define MAC_RPU_REG_RX_SMALL_DATA_FREE_RING_RPTR_REG \
    (MAC_RPU_REG_BASE + 0x1004)                                               /* 接收小包数据帧Free Ring的读指针上报 */
#define MAC_RPU_REG_RX_DATA_CMP_RING_WPTR_REG (MAC_RPU_REG_BASE + 0x1008)     /* 接收数据帧Complete Ring的写指针上报 */
#define MAC_RPU_REG_RPT_HOST_FREE_RING_STATUS_REG (MAC_RPU_REG_BASE + 0x100C) /* FREE RING上报信息 */
#define MAC_RPU_REG_RX_PPDU_HOST_DESC_FREE_RING_RPTR_REG \
    (MAC_RPU_REG_BASE + 0x1010) /* RX PPDU HOST描述符Free Ring的读指针上报 */
#define MAC_RPU_REG_RPT_PPDU_HOST_FREE_RING_STATUS_REG (MAC_RPU_REG_BASE + 0x1014) /* PPDU HOST FREE RING上报信息 */
#define MAC_GLB_REG_TX_BA_INFO_BUF_DEPTH_REG (MAC_GLB_REG_BASE + 0x3A4)            /* TX BA INFO BUFFER深度配置寄存器 */
#define MAC_GLB_REG_TX_BA_INFO_BUF_ADDR_LSB_REG (MAC_GLB_REG_BASE + 0x3A8) /* TX BA INFO BUFFER基地址配置寄存器 */
#define MAC_GLB_REG_TX_BA_INFO_BUF_ADDR_MSB_REG (MAC_GLB_REG_BASE + 0x3AC) /* TX BA INFO BUFFER基地址配置寄存器 */
#define MAC_GLB_REG_TX_BA_INFO_RPTR_REG (MAC_GLB_REG_BASE + 0x3B0)         /* TX BA INFO RPTR BUFFER基地址配置寄存器 */
#define MAC_GLB_REG_TX_BA_INFO_WPTR_REG (MAC_GLB_REG_BASE + 0x3B4)         /* TX BA INFO WPTR BUFFER基地址配置寄存器 */
#define MAC_RPU_REG_RX_DATA_BUFF_LEN_REG (MAC_RPU_REG_BASE + 0x58)         /* RX数据帧Buffer大小配置寄存器 */

#define MAC_RPU_REG_RX_RING_RESET_CTRL_REG  \
    (MAC_RPU_REG_BASE + 0x54) /* 接收ring复位配置寄存器 */
#define MAC_RPU_REG_RX_NORM_DATA_FREE_RING_ADDR_LSB_REG \
    (MAC_RPU_REG_BASE + 0x5C) /* 接收普通数据帧Free Ring基地址配置寄存器LSB */
#define MAC_RPU_REG_RX_NORM_DATA_FREE_RING_ADDR_MSB_REG \
    (MAC_RPU_REG_BASE + 0x60) /* 接收普通数据帧Free Ring基地址配置寄存器MSB */
#define MAC_RPU_REG_RX_SMALL_DATA_FREE_RING_ADDR_LSB_REG \
    (MAC_RPU_REG_BASE + 0x64) /* 接收小包数据帧Free Ring基地址配置寄存器_LSB */
#define MAC_RPU_REG_RX_SMALL_DATA_FREE_RING_ADDR_MSB_REG \
    (MAC_RPU_REG_BASE + 0x68) /* 接收小包数据帧Free Ring基地址配置寄存器_MSB */
#define MAC_RPU_REG_RX_DATA_CMP_RING_ADDR_LSB_REG \
    (MAC_RPU_REG_BASE + 0x6C) /* 接收数据帧Complete Ring基地址配置寄存器LSB */
#define MAC_RPU_REG_RX_DATA_CMP_RING_ADDR_MSB_REG \
    (MAC_RPU_REG_BASE + 0x70) /* 接收数据帧Complete Ring基地址配置寄存器MSB */
#define MAC_RPU_REG_RX_NORM_DATA_FREE_RING_SIZE_REG \
    (MAC_RPU_REG_BASE + 0x74) /* 接收普通数据帧Free Ring的大小配置寄存器 */
#define MAC_RPU_REG_RX_SMALL_DATA_FREE_RING_SIZE_REG \
    (MAC_RPU_REG_BASE + 0x78)                                           /* 接收小包数据帧Free Ring的大小配置寄存器 */
#define MAC_RPU_REG_RX_DATA_CMP_RING_SIZE_REG (MAC_RPU_REG_BASE + 0x7C) /* 接收数据帧Complete Ring的大小配置寄存器 */
#define MAC_RPU_REG_RX_NORM_DATA_FREE_RING_WPTR_REG \
    (MAC_RPU_REG_BASE + 0x80) /* 接收普通数据帧Free Ring的写指针配置寄存器 */
#define MAC_RPU_REG_RX_SMALL_DATA_FREE_RING_WPTR_REG \
    (MAC_RPU_REG_BASE + 0x84)                                           /* 接收小包数据帧Free Ring的写指针配置寄存器 */
#define MAC_RPU_REG_RX_DATA_CMP_RING_RPTR_REG (MAC_RPU_REG_BASE + 0x88) /* 接收数据帧Complete Ring的读指针配置寄存器 \
                                                                         */
#define MAC_RPU_REG_RX_PPDU_HOST_DESC_FREE_RING_ADDR_LSB_REG \
    (MAC_RPU_REG_BASE + 0x98) /* RX PPDU HOST描述符Free Ring基地址LSB */
#define MAC_RPU_REG_RX_PPDU_HOST_DESC_FREE_RING_ADDR_MSB_REG \
    (MAC_RPU_REG_BASE + 0x9C) /* RX PPDU HOST描述符Free Ring基地址MSB */
#define MAC_RPU_REG_RX_PPDU_HOST_DESC_FREE_RING_SIZE_REG \
    (MAC_RPU_REG_BASE + 0xA0) /* RX PPDU HOST描述符Free Ring的大小 */
#define MAC_RPU_REG_RX_PPDU_HOST_DESC_FREE_RING_WPTR_REG \
    (MAC_RPU_REG_BASE + 0xA4) /* RX PPDU HOST描述符Free Ring的写指针 */
#define MAC_RPU_REG_RX_PPDU_HOST_DESC_FREE_RING_RPTR_CFG_REG \
    (MAC_RPU_REG_BASE + 0xA8)                                     /* RX PPDU HOST描述符的Free Ring的读指针 */
#define MAC_GLB_REG_HOST_INTR_CLR_REG (MAC_GLB_REG_BASE + 0x35C)  /* Host中断清除寄存器 */
#define MAC_GLB_REG_HOST_INTR_MASK_REG (MAC_GLB_REG_BASE + 0x360) /* Host中断屏蔽寄存器 */
#define MAC_RPU_REG_RX_DATA_CMP_RING_WPTR_CFG_REG \
    (MAC_RPU_REG_BASE + 0x8C) /* 接收数据帧Complete Ring的写指针配置寄存器 */
#define MAC_RPU_REG_RX_NORM_DATA_FREE_RING_RPTR_CFG_REG \
    (MAC_RPU_REG_BASE + 0x90) /* 接收普通数据帧Free Ring的读指针配置寄存器 */
#define MAC_RPU_REG_RX_SMALL_DATA_FREE_RING_RPTR_CFG_REG \
    (MAC_RPU_REG_BASE + 0x94)                                  /* 接收小包数据帧的Free Ring的读指针 */
#define MAC_RPU_REG_RX_FRAMEFILT_REG (MAC_RPU_REG_BASE + 0x4)  /* RX过滤控制寄存器 */
#define MAC_RPU_REG_RX_FRAMEFILT2_REG (MAC_RPU_REG_BASE + 0x8) /* RX过滤控制寄存器2 */

#define MAC_GLB_REG_DMAC_INTR_MASK_REG (MAC_GLB_REG_BASE + 0x368)           /* DMAC中断屏蔽寄存器 */
#define MAC_RPU_REG_RX_CTRL_REG (MAC_RPU_REG_BASE + 0x0)                    /* 接收控制寄存器 */
#define MAC_PPU_REG_CSI_PROCESS_REG (MAC_PPU_REG_BASE + 0x4)                /* CSI采样上报配置寄存器 */
#define MAC_PPU_REG_CSI_BUF_BASE_ADDR_LSB_REG (MAC_PPU_REG_BASE + 0x14)     /* CSI采样上报片外循环BUF基地址LSB */
#define MAC_PPU_REG_CSI_BUF_BASE_ADDR_MSB_REG (MAC_PPU_REG_BASE + 0x18)     /* CSI采样上报片外循环BUF基地址MSB */
#define MAC_PPU_REG_CSI_BUF_SIZE_REG (MAC_PPU_REG_BASE + 0x1C)              /* CSI采样上报片外循环BUF大小 */
#define MAC_PPU_REG_CSI_WHITELIST_ADDR_LSB_0_REG (MAC_PPU_REG_BASE + 0x128) /* CSI采样白名单低32位 */
#define MAC_PPU_REG_CSI_WHITELIST_ADDR_MSB_0_REG (MAC_PPU_REG_BASE + 0x1A8) /* CSI采样白名单高16位 */

#define MAC_PPU_REG_LOCATION_INFO_MASK_REG (MAC_PPU_REG_BASE + 0x1004) /* 定位信息上报 */
#define MAC_PPU_REG_CSI_INFO_ADDR_MSB_REG (MAC_PPU_REG_BASE + 0x1008)  /* CSI上报地址高32bit */
#define MAC_PPU_REG_CSI_INFO_ADDR_LSB_REG (MAC_PPU_REG_BASE + 0x100C)  /* CSI上报地址低32bit */
#define MAC_PPU_REG_FTM_INFO_ADDR_MSB_REG (MAC_PPU_REG_BASE + 0x1010)  /* FTM上报地址高32bit */
#define MAC_PPU_REG_FTM_INFO_ADDR_LSB_REG (MAC_PPU_REG_BASE + 0x1014)  /* FTM上报地址低32bit */

#define MAC_PPU_REG_FTM_PROCESS_REG (MAC_PPU_REG_BASE + 0x8)               /* FTM采样上报配置寄存器 */
#define MAC_PPU_REG_FTM_BUF_BASE_ADDR_LSB_REG (MAC_PPU_REG_BASE + 0xC)     /* FTM采样上报片外循环BUF基地址LSB */
#define MAC_PPU_REG_FTM_BUF_BASE_ADDR_MSB_REG (MAC_PPU_REG_BASE + 0x10)    /* FTM采样上报片外循环BUF基地址MSB */
#define MAC_PPU_REG_FTM_WHITELIST_ADDR_LSB_0_REG (MAC_PPU_REG_BASE + 0x28) /* FTM采样白名单低32位 */
#define MAC_PPU_REG_FTM_WHITELIST_ADDR_MSB_0_REG (MAC_PPU_REG_BASE + 0xA8) /* FTM采样白名单高16位 */

#define MAC_GLB_REG_DMAC_COMMON_TIMER_MASK_REG (MAC_GLB_REG_BASE + 0x384)         /* DMAC通用计数器中断屏蔽寄存器 */
#define MAC_TMU_REG_COMMON_TIMER_CTRL_0_REG (MAC_TMU_REG_BASE + 0xE8)             /* 通用定时器相关参数配置 */
#define MAC_TMU_REG_COMMON_TIMER_VAL_0_REG (MAC_TMU_REG_BASE + 0x168)             /* 通用定时器定时值配置 */
#define MAC_TMU_REG_VAP_TSFTIMER_RDVALL_STATUS_12_REG (MAC_TMU_REG_BASE + 0x1134) /* VAP tsf(低32bit), 单位us */
#define MP16C_W_SOFT_INT_SET_AON_REG (MP16C_GLB_CTL_RB_BASE_BASE + 0x134)

/* MAC 通用定时器 */
#define MP16C_MAC_COMMON_TIMER_OFFSET 0x4
#define MP16C_MAC_MAX_COMMON_TIMER 31 /* 通用定时器个数 */
#define MP16C_MAC_GLB_REG_RPT_HOST_INTR_DATA_RING_OVERRUN_OFFSET  5
#define MP16C_MAC_GLB_REG_RPT_HOST_INTR_DATA_RING_OVERRUN_MASK    0x20
#define MP16C_MAC_GLB_REG_RPT_HOST_INTR_SMALL_RING_EMPTY_OFFSET   4
#define MP16C_MAC_GLB_REG_RPT_HOST_INTR_SMALL_RING_EMPTY_MASK     0x10
#define MP16C_MAC_GLB_REG_RPT_HOST_INTR_NORM_RING_EMPTY_OFFSET    3
#define MP16C_MAC_GLB_REG_RPT_HOST_INTR_NORM_RING_EMPTY_MASK      0x8
#define MP16C_MAC_GLB_REG_RPT_HOST_INTR_RX_COMPLETE_OFFSET        2
#define MP16C_MAC_GLB_REG_RPT_HOST_INTR_RX_COMPLETE_MASK          0x4

void mp16c_host_mac_irq_mask(hal_host_device_stru *hal_device, uint32_t irq);
void mp16c_host_mac_irq_unmask(hal_host_device_stru *hal_device, uint32_t irq);
void mp16c_clear_host_mac_int_status(hal_host_device_stru *hal_device, uint32_t status);
void mp16c_get_host_mac_int_mask(hal_host_device_stru *hal_device, uint32_t *p_mask);
void mp16c_get_host_mac_int_status(hal_host_device_stru *hal_device, uint32_t *p_status);
void mp16c_set_host_mac_irq_mask(hal_host_device_stru *hal_device, uint32_t intr_mask);
uint32_t mp16c_regs_addr_transfer(hal_host_device_stru *hal_device, uint32_t reg_addr);
void mp16c_clear_host_phy_int_status(hal_host_device_stru *hal_device, uint32_t status);
void mp16c_set_host_phy_int_mask(hal_host_device_stru *hal_device, uint32_t mask);
void mp16c_get_host_phy_int_mask(hal_host_device_stru *hal_device, uint32_t *p_mask);
void mp16c_get_host_phy_int_status(hal_host_device_stru *hal_device, uint32_t *p_status);
int32_t mp16c_host_regs_addr_init(hal_host_device_stru *hal_device);
uint32_t mp16c_get_dev_rx_filt_mac_status(uintptr_t *reg_status);
uint32_t mp16c_regs_addr_get_offset(uint8_t device_id, uint32_t reg_addr);

void mp16c_host_al_rx_fcs_info(hmac_vap_stru *hmac_vap);
void mp16c_host_get_rx_pckt_info(hmac_vap_stru *hmac_vap, dmac_atcmdsrv_atcmd_response_event *rx_pkcg_event_info);
int32_t mp16c_host_init_common_timer(hal_mac_common_timer *mac_timer);
void mp16c_host_set_mac_common_timer(hal_mac_common_timer *mac_common_timer);
uint32_t mp16c_host_get_tsf_lo(hal_host_device_stru *hal_device, uint8_t hal_vap_id, uint32_t *tsf);
hal_mac_common_timer *mp16c_get_host_mac_common_timer_ptr(uint8_t id);
void mp16c_clear_host_mac_irq_mask(void);
void mp16c_recover_host_mac_irq_mask(void);
void mp16c_host_mac_phy_irq_mask(void);
#endif
