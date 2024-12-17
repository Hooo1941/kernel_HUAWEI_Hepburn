/**
 * Copyright (c) @CompanyNameMagicTag 2021-2023. All rights reserved.
 *
 * Description:pcie chip adaptation
 * Author: @CompanyNameTag
 */

#define MPXX_LOG_MODULE_NAME "[PCIEC]"
#define HISI_LOG_TAG           "[PCIEC]"

#include "oal_hcc_host_if.h"
#ifdef _PRE_PLAT_FEATURE_GF6X_PCIE
#include "gf61/host_ctrl_rb_regs.h"
#include "gf61/pcie_ctrl_rb_regs.h"
#include "gf61/pcie_pcs_rb_regs.h"

#include "chip/gf61/pcie_soc_gf61.h"
#include "pcie_gfxx/chip/comm/pcie_pm.h"
#include "pcie_pcs_trace.h"
#include "pcie_pcs_regs.h"
#include "pcie_linux_gfxx.h"
#include "oam_ext_if.h"

/* gf61 Registers */
#define PA_GLB_CTL_BASE_ADDR        0x40000000
#define PA_PCIE_DBI_BASE_ADDRESS    0x40107000
#define PA_PMU_CMU_CTL_BASE         0x40002000
#define PA_PMU2_CMU_IR_BASE         0x40012000
#define PA_W_CTL_BASE               0x40105000

#define PA_GT_PCIE_DBI_BASE_ADDR  0x53107000
#define PA_GT_HOST_CTRL_BASE_ADDR 0x51C4C000
#define PA_GT_PCIE_DEVICE_REG_REG 0x531082D8
#define PA_GT_PCIE_CTRL_BASE_ADDR 0x53108000

#define PWR_ON_LABLE_REG_GT         0x400008d8
#define DEADBEAF_RESET_VAL_GT       ((uint16_t)0xcdcd)
#define PWR_ON_LABLE_REG_WIFI       0x40000200
#define DEADBEAF_RESET_VAL_WIFI     ((uint16_t)0x5757)

#define DEV_VERSION_CPU_ADDR 0x5000003c

/* mac优化寄存器 */
#define PCI_EXP_LNKCTL2_SPEED_MASK  0xF
#define PCI_EXP_SPEED_CHANGE_MASK   0x20000
#define LINK_RATE_MASK              0x300000
#define LTSSM_STATE_MASK            0x7e00
#define CURRENT_LINK_RATE_OFFSET    20
#define CURRENT_LINK_STATE_OFFSET   9

static uintptr_t oal_pcie_get_pcs_paddr(oal_pcie_linux_res *pcie_res, uint32_t dump_type)
{
    int32_t ret;
    pci_addr_map st_map;
    uint32_t phy_paddr;

    if (pcie_res->dev_id == HCC_EP_GT_DEV) {
        phy_paddr = GF61_PCIE_PHY_BASE_ADDRESS_GT;
    } else {
        phy_paddr = GF61_PCIE_PHY_BASE_ADDRESS;
    }
    if (dump_type == PCIE_PCS_DUMP_BY_SSI) {
        return (uintptr_t)phy_paddr;
    }
    ret = oal_pcie_inbound_ca_to_va(pcie_res, phy_paddr, &st_map);
    if (ret != OAL_SUCC) {
        pci_print_log(PCI_LOG_ERR, "get dev address 0x%x failed", phy_paddr);
        return (uintptr_t)NULL;
    }
    /* memport virtual address */
    return (uintptr_t)st_map.va;
}

static void oal_pcie_change_speed_before_pcs(oal_pcie_linux_res *pcie_res, void *pcie_ctrl)
{
    pci_print_log(PCI_LOG_INFO,
                  "before: non_l1_rcvry_cnt:0x%x l1_rcvry_cnt:0x%x stat_monitor1:0x%x stat_monitor2:0x%x",
                  oal_readl(pcie_ctrl + PCIE_CTRL_RB_PCIE_STAT_CNT_RCVRY_OFF),
                  oal_readl(pcie_ctrl + PCIE_CTRL_RB_PCIE_STAT_CNT_L1_RCVRY_OFF),
                  oal_readl(pcie_ctrl + PCIE_CTRL_RB_PCIE_STAT_MONITOR1_OFF),
                  oal_readl(pcie_ctrl + PCIE_CTRL_RB_PCIE_STAT_MONITOR2_OFF));
    oal_writel(0xFFFFFFFF, (pcie_ctrl + PCIE_CTRL_RB_PCIE_STAT_MONITOR2_OFF));

    if (pcs_trace_speedchange_flag() != 0) {
        pcie_pcs_tracer_start(oal_pcie_get_pcs_paddr(pcie_res, PCIE_PCS_DUMP_BY_MEMPORT),
                              PCS_TRACER_SPEED_CHANGE, 0, PCIE_PCS_DUMP_BY_MEMPORT);
    }
}

static void oal_pcie_change_speed_rc_mac_adapter(oal_pci_dev_stru *pst_rc_dev, int32_t *ep_l1ss_pm)
{
    int32_t is_self_pcie_ip = 0;
    int32_t ret;
    uint32_t lnkctl_val;
    uint32_t val;
    /* debug mac reg before speed-change,bit[19:16]是软件切换后的链路速率 */
    oal_pci_read_config_dword(pst_rc_dev, *ep_l1ss_pm + PCI_EXP_LNKCTL, &lnkctl_val);
    pci_print_log(PCI_LOG_INFO, "before: rc mac addr:0x%x PCI_EXP_LNKCTL:0x%x", PCI_EXP_LNKCTL, lnkctl_val);

    /* 配置TARGET_LINK_SPEED,bit[3:0]是软件切换配置的目标速率,bit[20:17]对应EQ状态 */
    oal_pci_read_config_dword(pst_rc_dev, *ep_l1ss_pm + PCI_EXP_LNKCTL2, &val);
    pci_print_log(PCI_LOG_INFO, "before: rc mac addr:0x%x PCI_EXP_LNKCTL2:0x%x", PCI_EXP_LNKCTL2, val);
    val &= ~PCI_EXP_LNKCTL2_SPEED_MASK;
    val |= (uint32_t)(oal_pcie_get_pcie_speed_change_val() + 1);
    oal_pci_write_config_dword(pst_rc_dev, *ep_l1ss_pm + PCI_EXP_LNKCTL2, val);

    ret = get_cust_conf_int32(INI_MODU_PLAT, "is_self_pcie_ip", &is_self_pcie_ip);
    if ((ret == OAL_SUCC) && (is_self_pcie_ip == 1)) {
        lnkctl_val |= BIT5; /* 自研IP PCI_EXP_LNKCTL BIT5为retrain_link */
        oal_pci_write_config_dword(pst_rc_dev, *ep_l1ss_pm + PCI_EXP_LNKCTL, lnkctl_val);
        pci_print_log(PCI_LOG_INFO, "self pcie ip retrain_link");
    } else {
        /* 配置DIRECT_SPEED_CHANGE,bit[17]直接切速; 不用配置ep进行RETRAIN */
        oal_pci_read_config_dword(pst_rc_dev, *ep_l1ss_pm + PCI_GEN2_CTRL, &val);
        pci_print_log(PCI_LOG_INFO, "before: rc mac addr:0x%x PCI_GEN2_CTRL:0x%x", PCI_GEN2_CTRL, val);
        val &= ~PCI_EXP_SPEED_CHANGE_MASK;
        val |= PCI_EXP_SPEED_CHANGE_MASK;
        oal_pci_write_config_dword(pst_rc_dev, *ep_l1ss_pm + PCI_GEN2_CTRL, val);
    }
}

static void oal_pcie_change_speed_check(oal_pcie_linux_res *pcie_res, const void *pcie_ctrl,
                                        uint8_t *speed_change_succ, uint32_t *link_speed, uint32_t *link_sts)
{
    int32_t pcie_speed_change_ini = oal_pcie_get_pcie_speed_change_val();
    msleep(1);
    *link_speed = ((oal_readl(pcie_ctrl + PCIE_CTRL_RB_PCIE_STATUS2_OFF)) &
                   LINK_RATE_MASK) >> CURRENT_LINK_RATE_OFFSET;
    *link_sts = ((oal_readl(pcie_ctrl + PCIE_CTRL_RB_PCIE_STATUS0_OFF)) &
                 LTSSM_STATE_MASK) >> CURRENT_LINK_STATE_OFFSET;
    if (*link_speed != pcie_speed_change_ini) {
        oam_error_log1(0, OAM_SF_PWR, "speed change fail to %d[0:GEN1 1:GEN2 2:GEN3]", pcie_speed_change_ini);
        pci_print_log(PCI_LOG_ERR, "speed change fail to %d[0:GEN1 1:GEN2 2:GEN3]", pcie_speed_change_ini);
        declare_dft_trace_key_info("pcie_speed_change_fail", OAL_DFT_TRACE_FAIL);
        chr_exception_report(CHR_PLATFORM_EXCEPTION_EVENTID, CHR_SYSTEM_PLAT, CHR_LAYER_DRV,
                             CHR_PLT_DRV_EVENT_PCIE, CHR_PLAT_DRV_ERROR_PCIE_SPEED_CHANGE_FAIL);
#if defined(CONFIG_HUAWEI_OHOS_DSM) || defined(CONFIG_HUAWEI_DSM)
        hw_mpxx_dsm_client_notify(SYSTEM_TYPE_PLATFORM, DSM_WIFI_PCIE_LINKDOWN,
                                  "%s: pcie speed change fail\n", __FUNCTION__);
#endif
        if (pcs_trace_speedchange_flag() & 0x1) {
            pcie_pcs_tracer_dump(oal_pcie_get_pcs_paddr(pcie_res, PCIE_PCS_DUMP_BY_SSI),
                                 0, PCIE_PCS_DUMP_BY_SSI);
        }
    } else {
        *speed_change_succ = 1;
        oam_warning_log1(0, OAM_SF_PWR, "speed change success to %d[0:GEN1 1:GEN2 2:GEN3]", pcie_speed_change_ini);
        pci_print_log(PCI_LOG_INFO, "speed change success to %d[0:GEN1 1:GEN2 2:GEN3]", pcie_speed_change_ini);
        declare_dft_trace_key_info("pcie_speed_change_succ", OAL_DFT_TRACE_SUCC);
        if (pcs_trace_speedchange_flag() & 0x2) {
            pcie_pcs_tracer_dump(oal_pcie_get_pcs_paddr(pcie_res, PCIE_PCS_DUMP_BY_SSI),
                                 0, PCIE_PCS_DUMP_BY_SSI);
        }
    }
}

static void oal_pcie_change_speed_after_pcs(oal_pci_dev_stru *pst_rc_dev, void *pcie_ctrl,
                                            int32_t *ep_l1ss_pm, uint32_t *link_speed, uint32_t *link_sts)
{
    uint32_t val;
    pci_print_log(PCI_LOG_INFO,
                  "after: non_l1_rcvry_cnt:0x%x l1_rcvry_cnt:0x%x stat_monitor1:0x%x stat_monitor2:0x%x",
                  oal_readl(pcie_ctrl + PCIE_CTRL_RB_PCIE_STAT_CNT_RCVRY_OFF),
                  oal_readl(pcie_ctrl + PCIE_CTRL_RB_PCIE_STAT_CNT_L1_RCVRY_OFF),
                  oal_readl(pcie_ctrl + PCIE_CTRL_RB_PCIE_STAT_MONITOR1_OFF),
                  oal_readl(pcie_ctrl + PCIE_CTRL_RB_PCIE_STAT_MONITOR2_OFF));
    /* debug mac reg after speed-change */
    oal_pci_read_config_dword(pst_rc_dev, *ep_l1ss_pm + PCI_EXP_LNKCTL, &val);
    pci_print_log(PCI_LOG_INFO, "after: rc mac addr:0x%x PCI_EXP_LNKCTL:0x%x", PCI_EXP_LNKCTL, val);
    oal_pci_read_config_dword(pst_rc_dev, *ep_l1ss_pm + PCI_EXP_LNKCTL2, &val);
    pci_print_log(PCI_LOG_INFO, "after: rc mac addr:0x%x PCI_EXP_LNKCTL2:0x%x", PCI_EXP_LNKCTL2, val);
    oal_pci_read_config_dword(pst_rc_dev, *ep_l1ss_pm + PCI_GEN2_CTRL, &val);
    pci_print_log(PCI_LOG_INFO, "after: rc mac addr:0x%x PCI_GEN2_CTRL:0x%x", PCI_GEN2_CTRL, val);
    pci_print_log(PCI_LOG_INFO, "speed change result: link_sts = 0x%x, link_speed = 0x%x", *link_sts, *link_speed);
}

#define PCIE_PRESET_BIT 20
#define PCIE_PRESET_MASK 0x100000
#define PCIE_TX_SWING_BIT 18
#define PCIE_TX_SWING_MASK 0x40000
int32_t oal_pcie_eye_param_change_gf61(oal_pcie_linux_res *pcie_res)
{
    void *pcie_dbi = pcie_res->ep_res.pst_pci_dbi_base;
    uint32_t preset;
    uint32_t tx_swing;
    uint32_t val;
    uint32_t val_callback;
    uint32_t preset_ini = oal_pcie_get_preset_val();
    uint32_t tx_swing_ini = oal_pcie_get_tx_swing_val();

    if (pcie_dbi == NULL) {
        pci_print_log(PCI_LOG_ERR, "pcie_dbi virtual address is null, not pcie eye param");
        return OAL_FAIL;
    }
    val = oal_readl(pcie_dbi + PCI_GEN2_CTRL);
    preset = oal_get_bits(val, 1, PCIE_PRESET_BIT);
    tx_swing = oal_get_bits(val, 1, PCIE_TX_SWING_BIT);
    if ((preset != preset_ini) || (tx_swing != tx_swing_ini)) {
        val &= ~PCIE_PRESET_MASK;
        val |= ((preset_ini & 0x1) << PCIE_PRESET_BIT);
        val &= ~PCIE_TX_SWING_MASK;
        val |= ((tx_swing_ini & 0x1) << PCIE_TX_SWING_BIT);
        oal_writel(val, pcie_dbi + PCI_GEN2_CTRL);
        val_callback = oal_readl(pcie_dbi + PCI_GEN2_CTRL);
        if (val_callback != val) {
            pci_print_log(PCI_LOG_ERR, "pcie eye param set fail, val 0x%x, callback 0x%x", val, val_callback);
            return OAL_FAIL;
        }
        oal_msleep(1); /* 等待参数生效 */
    }
    return OAL_SUCC;
}

#define LINK_SPEED_CHANGE_MAX_ROUND 2
void oal_pcie_link_speed_change_gf61(oal_pcie_linux_res *pcie_res)
{
    void *pcie_ctrl = NULL;
    oal_pci_dev_stru *pst_rc_dev = NULL;
    int32_t ep_l1ss_pm = 0;
    uint8_t speed_change_round = 1;
    uint8_t speed_change_succ = 0;
    uint32_t link_speed, link_sts;

    pcie_ctrl = pcie_res->ep_res.pst_pci_ctrl_base;
    if (pcie_ctrl == NULL) {
        pci_print_log(PCI_LOG_ERR, "pcie_ctrl virtual address is null, not change link speed");
        return;
    }

    while ((speed_change_succ == 0) && (speed_change_round <= LINK_SPEED_CHANGE_MAX_ROUND)) {
        pci_print_log(PCI_LOG_INFO, "pcie link speed change, round:%d", speed_change_round);
        speed_change_round++;

        /* Step1: 如果当前速率不等于GEN1, 或目标速率等于GEN1, 不切速 */
        link_speed = ((oal_readl(pcie_ctrl + PCIE_CTRL_RB_PCIE_STATUS2_OFF)) &
                      LINK_RATE_MASK) >> CURRENT_LINK_RATE_OFFSET;
        if (link_speed != PCIE_GEN1) {
            oam_error_log0(0, OAM_SF_PWR, "pcie link speed change error, current speed is not GEN1");
            declare_dft_trace_key_info("pcie_speed_change_error", OAL_DFT_TRACE_FAIL);
            return;
        }
        if (oal_pcie_get_pcie_speed_change_val() == PCIE_GEN1) {
            pci_print_log(PCI_LOG_WARN, "pcie link speed change bypass, target speed is GEN1");
            continue;
        }

        /* Step2: 如果链路状态bit[14:9]非L0, 不切速 */
        link_sts = ((oal_readl(pcie_ctrl + PCIE_CTRL_RB_PCIE_STATUS0_OFF)) &
                    LTSSM_STATE_MASK) >> CURRENT_LINK_STATE_OFFSET;
        if (link_sts != 0x11) {
            pci_print_log(PCI_LOG_ERR, "pcie link speed change bypass, link stats is not L0");
            continue;
        }

        /* Step3: 链路状态机数采维测 */
        oal_pcie_change_speed_before_pcs(pcie_res, pcie_ctrl);

        /* Step4: 配置rc侧mac寄存器 */
        pst_rc_dev = pci_upstream_bridge(pcie_res->comm_res->pcie_dev);
        if (pst_rc_dev != NULL) {
            ep_l1ss_pm = oal_pci_pcie_cap(pst_rc_dev);
            if (!ep_l1ss_pm) {
                pci_print_log(PCI_LOG_ERR, "pcie link speed change fail, cannot get rc addr");
                continue;
            }
            oal_pcie_change_speed_rc_mac_adapter(pst_rc_dev, &ep_l1ss_pm);
        }

        /* Step5: 固定delay 1ms, EDA仿真不超过300us;并确认链路速度,bit[21:20] */
        oal_pcie_change_speed_check(pcie_res, pcie_ctrl, &speed_change_succ, &link_speed, &link_sts);

        /* Step6: 链路状态机数采维测，预期non_l1_rcvry_cnt=0x3, l1_rcvry_cnt=0x0 */
        oal_pcie_change_speed_after_pcs(pst_rc_dev, pcie_ctrl, &ep_l1ss_pm, &link_speed, &link_sts);
    }
}

int32_t oal_pcie_get_bar_region_info_gf61(oal_pcie_linux_res *pcie_res, oal_pcie_region **, uint32_t *);
int32_t oal_pcie_set_outbound_membar_gf61(oal_pcie_linux_res *pcie_res, oal_pcie_iatu_bar* pst_iatu_bar);

static uintptr_t oal_pcie_get_test_ram_address(void)
{
    return 0x2000000; // pkt mem for test, by acp port
}

static int32_t oal_pcie_voltage_bias_init_gf61_int(oal_pcie_linux_res *pcie_res)
{
    oal_print_mpxx_log(MPXX_LOG_ERR, "oal_pcie_voltage_bias_init_gf61_int is not implement!");
    return OAL_SUCC;
}

#define GT_CTL_RB_BASE                   0x51c42000
#define GT_CTL_RB_GSOC_CLKEN_CFG_REG_OFF 0x130
#define GT_CTL_RB_GSOC_SRST_CFG_REG_OFF  0x140

typedef union {
    struct {
        unsigned int rf_ctl_clken : 1;
        unsigned int uart_clken : 1;
        unsigned int i2c0_clken : 1;
        unsigned int i2c1_clken : 1;
        unsigned int i2c2_clken : 1;
        unsigned int wdt0_clken : 1;
        unsigned int wdt1_clken : 1;
        unsigned int wdt2_clken : 1;
        unsigned int timer0_clken : 1;
        unsigned int timer1_clken : 1;
        unsigned int timer2_clken : 1;
        unsigned int timer3_clken : 1;
        unsigned int i2s_clken : 1;
        unsigned int edma_clken : 1;
        unsigned int ete_clken : 1;
        unsigned int gram_clken : 1;
        unsigned int pkt_mem_clken : 1;
        unsigned int fmc_clken : 1;
        unsigned int monitor_clken : 1;
        unsigned int reserved0 : 1;
        unsigned int ipc_clken : 1;
        unsigned int audio_clken : 1;
        unsigned int reserved1 : 10;
    } bits;
    unsigned int as_dword;
} hreg_gsoc_clken_cfg;

typedef union {
    struct {
        unsigned int srst_rf_ctl_n : 1;
        unsigned int srst_uart_n : 1;
        unsigned int srst_i2c0_n : 1;
        unsigned int srst_i2c1_n : 1;
        unsigned int srst_i2c2_n : 1;
        unsigned int reserved0 : 3;
        unsigned int srst_timer0_n : 1;
        unsigned int srst_timer1_n : 1;
        unsigned int srst_timer2_n : 1;
        unsigned int srst_timer3_n : 1;
        unsigned int srst_i2s_n : 1;
        unsigned int srst_edma_n : 1;
        unsigned int srst_ete_n : 1;
        unsigned int srst_gram_n : 1;
        unsigned int srst_pktmem_n : 1;
        unsigned int srst_fmc_n : 1;
        unsigned int srst_monitor_n : 1;
        unsigned int reserved1 : 1;
        unsigned int srst_ipc_n : 1;
        unsigned int srst_meter_apb_n : 1;
        unsigned int srst_meter_n : 1;
        unsigned int srst_fmc_crgdiv_n : 1;
        unsigned int reserved2 : 8;
    } bits;
    unsigned int as_dword;
} hreg_gsoc_srst_cfg;
static int32_t oal_pcie_gt_ete_clk_en(uintptr_t base_addr, uint32_t enable)
{
    hreg_gsoc_clken_cfg value;
    hreg_gsoc_clken_cfg read;
    uintptr_t offset = base_addr + GT_CTL_RB_GSOC_CLKEN_CFG_REG_OFF;
    hreg_get_val(value, offset);
    value.bits.ete_clken = !!enable;
    hreg_set_val(value, offset);
    hreg_get_val(read, offset);
    if (read.bits.ete_clken != value.bits.ete_clken) {
        return -OAL_EFAIL;
    } else {
        return OAL_SUCC;
    }
}

static int32_t oal_pcie_gt_ete_srst_en(uintptr_t base_addr, uint32_t enable)
{
    hreg_gsoc_srst_cfg value;
    hreg_gsoc_srst_cfg read;
    uintptr_t offset = base_addr + GT_CTL_RB_GSOC_CLKEN_CFG_REG_OFF;
    hreg_get_val(value, offset);
    value.bits.srst_ete_n = !!enable;
    hreg_set_val(value, offset);
    hreg_get_val(read, offset);
    if (read.bits.srst_ete_n != value.bits.srst_ete_n) {
        return -OAL_EFAIL;
    } else {
        return OAL_SUCC;
    }
}

static void oal_pcie_gt_ete_dereset(oal_pcie_linux_res *pcie_res)
{
    int32_t ret;
    pci_addr_map st_map;
    ret = oal_pcie_inbound_ca_to_va(pcie_res, GT_CTL_RB_BASE, &st_map);
    if (ret != OAL_SUCC) {
        pci_print_log(PCI_LOG_ERR, "get dev address 0x%x failed", GT_CTL_RB_BASE);
        return;
    }
    ret = oal_pcie_gt_ete_clk_en(st_map.va, OAL_TRUE);
    if (ret != OAL_SUCC) {
        pci_print_log(PCI_LOG_ERR, "enable gt ete clk failed");
        return;
    }
    ret = oal_pcie_gt_ete_srst_en(st_map.va, OAL_TRUE);
    if (ret != OAL_SUCC) {
        pci_print_log(PCI_LOG_ERR, "enable gt ete srst failed");
        return;
    }
}

#define WIFI_ETE_CTRL_BASE 0x40000054
typedef union {
    struct {
        unsigned int soft_pcie0_n : 1;
        unsigned int soft_rst_pcie0_ctrl_n : 1;
        unsigned int soft_rst_dbi0_n : 1;
        unsigned int soft_rst_pcs0_n : 1;
        unsigned int reserved0 : 4;
        unsigned int soft_rst_ete_n : 1;
        unsigned int soft_rst_ete_ctrl_n : 1;
        unsigned int srst_rst_pcie0_itf_n : 1;
        unsigned int reserved1 : 5;
    } bits;
    unsigned int as_dword;
} aon_pcie_soft_rst_cfg;

static void oal_pcie_wifi_ete_dereset(oal_pcie_linux_res *pcie_res)
{
    int32_t ret;
    pci_addr_map st_map;
    aon_pcie_soft_rst_cfg value;
    aon_pcie_soft_rst_cfg read;
    ret = oal_pcie_inbound_ca_to_va(pcie_res, WIFI_ETE_CTRL_BASE, &st_map);
    if (ret != OAL_SUCC) {
        pci_print_log(PCI_LOG_ERR, "get dev address 0x%x failed", WIFI_ETE_CTRL_BASE);
        return;
    }

    hreg_get_val(value, st_map.va);
    value.bits.soft_rst_ete_n = 0x1;
    value.bits.soft_rst_ete_ctrl_n = 0x1;
    hreg_set_val(value, st_map.va);
    hreg_get_val(read, st_map.va);
    if ((read.bits.soft_rst_ete_n != value.bits.soft_rst_ete_n) ||
        (read.bits.soft_rst_ete_ctrl_n != value.bits.soft_rst_ete_ctrl_n)) {
        pci_print_log(PCI_LOG_ERR, "wifi ete derest failed");
    }
}

void oal_pcie_ete_dereset(oal_pcie_linux_res *pcie_res)
{
    if (pcie_res->pst_bus->dev_id == HCC_EP_GT_DEV) {
        oal_pcie_gt_ete_dereset(pcie_res);
    } else {
        oal_pcie_wifi_ete_dereset(pcie_res);
    }
}

static int32_t oal_pcie_host_slave_address_switch_gf61_int(oal_pcie_linux_res *pcie_res, uint64_t src_addr,
                                                           uint64_t* dst_addr, int32_t is_host_iova)
{
    if (is_host_iova == OAL_TRUE) {
        if (oal_likely((src_addr < (HISI_PCIE_MASTER_END_ADDRESS)))) {
            *dst_addr = src_addr + HISI_PCIE_IP_REGION_OFFSET;
            pci_print_log(PCI_LOG_DBG, "pcie_if_hostca_to_devva ok, hostca=0x%llx\n", *dst_addr);
            return OAL_SUCC;
        }
    } else {
        if (oal_likely((((src_addr >= HISI_PCIE_SLAVE_START_ADDRESS)
                       && (src_addr < (HISI_PCIE_SLAVE_END_ADDRESS)))))) {
            *dst_addr = src_addr - HISI_PCIE_IP_REGION_OFFSET;
            pci_print_log(PCI_LOG_DBG, "pcie_if_devva_to_hostca ok, devva=0x%llx\n", *dst_addr);
            return OAL_SUCC;
        }
    }

    pci_print_log(PCI_LOG_ERR, "pcie_slave_address_switch %s failed, src_addr=0x%llx\n",
                  (is_host_iova == OAL_TRUE) ? "iova->slave" : "slave->iova", src_addr);
    return -OAL_EFAIL;
}

static int32_t oal_ete_intr_init(oal_pcie_linux_res *pcie_res)
{
    hreg_host_intr_mask intr_mask;
    hreg_gt_host_intr_mask gt_intr_mask;
#ifdef _PRE_COMMENT_CODE_
    hreg_pcie_ctrl_ete_ch_dr_empty_intr_mask host_dre_mask;
#endif
    oal_ete_reg *reg = NULL;
    oal_pcie_ep_res *ep_res = &pcie_res->ep_res;

    /* enable interrupts */
    if (ep_res->pst_pci_ctrl_base == NULL) {
        oal_print_mpxx_log(MPXX_LOG_INFO, "pst_pci_ctrl_base is null");
        return -OAL_ENODEV;
    }

    reg = &ep_res->ete_info.reg;
    if (pcie_res->dev_id == HCC_EP_GT_DEV) {
        hreg_get_val(gt_intr_mask, (void *)reg->host_intr_mask_addr);
        gt_intr_mask.bits.device2host_tx_intr_mask = 0;
        gt_intr_mask.bits.device2host_rx_intr_mask = 0;
        gt_intr_mask.bits.device2host_intr_mask = 0;
        gt_intr_mask.bits.host_ete_tx_intr_mask = 0;
        gt_intr_mask.bits.host_ete_rx_intr_mask = 0;
        gt_intr_mask.bits.rx_ch_dr_empty_intr_mask = 0;
        gt_intr_mask.bits.tx_ch_dr_empty_intr_mask = 0;
        hreg_set_val(gt_intr_mask, (void *)reg->host_intr_mask_addr);
    } else {
        hreg_get_val(intr_mask, (void *)reg->host_intr_mask_addr);
        intr_mask.bits.host2device_tx_intr_mask = 0;
        intr_mask.bits.device2host_tx_intr_mask = 0;
        intr_mask.bits.device2host_rx_intr_mask = 0;
        intr_mask.bits.device2host_intr_mask = 0;

        /* WiFi中断需要在WiFi回调注册后Unmask */
        intr_mask.bits.mac_n1_intr_mask = 0;
        intr_mask.bits.mac_n2_intr_mask = 0;
        intr_mask.bits.phy_n1_intr_mask = 0;
        intr_mask.bits.phy_n2_intr_mask = 0;
        hreg_set_val(intr_mask, (void *)reg->host_intr_mask_addr);
    }
    hreg_get_val(intr_mask, (void *)reg->host_intr_mask_addr);
    oal_print_mpxx_log(MPXX_LOG_INFO, "pcie intr mask=0x%x", intr_mask.as_dword);

    /* mask:1 for mask, 0 for unmask */
#ifdef _PRE_COMMENT_CODE_
    hreg_get_val(host_dre_mask, ep_res->pst_pci_ctrl_base + PCIE_CTRL_RB_PCIE_CTRL_ETE_CH_DR_EMPTY_INTR_MASK_OFF);
    host_dre_mask.bits.host_rx_ch_dr_empty_intr_mask = 0; /* unmask rx dre int */
    hreg_set_val(host_dre_mask,  ep_res->pst_pci_ctrl_base +
                 PCIE_CTRL_RB_PCIE_CTRL_ETE_CH_DR_EMPTY_INTR_MASK_OFF);
#endif

    /* unmask all ete host int */
    oal_writel(0x0, ep_res->pst_ete_base + HOST_CTRL_RB_PCIE_CTL_ETE_INTR_MASK_OFF);

    return OAL_SUCC;
}

static void oal_ete_res_init(oal_pcie_linux_res *pcie_res)
{
    oal_ete_reg *reg = &pcie_res->ep_res.ete_info.reg;

    if (pcie_res->pst_bus->dev_id == HCC_EP_WIFI_DEV) {
        reg->host_intr_sts_addr = pcie_res->ep_res.pst_pci_ctrl_base +
                                PCIE_CTRL_RB_HOST_INTR_STATUS_OFF;
        reg->host_intr_clr_addr = pcie_res->ep_res.pst_pci_ctrl_base +
                                PCIE_CTRL_RB_HOST_INTR_CLR_OFF;
        reg->host_intr_mask_addr = pcie_res->ep_res.pst_pci_ctrl_base +
                                 PCIE_CTRL_RB_HOST_INTR_MASK_OFF;
        reg->h2d_intr_addr = pcie_res->ep_res.pst_pci_ctrl_base +
                           PCIE_CTRL_RB_HOST2DEVICE_INTR_SET_OFF;
    } else if (pcie_res->pst_bus->dev_id == HCC_EP_GT_DEV) {
        reg->host_intr_sts_addr = pcie_res->ep_res.pst_pci_ctrl_base +
                                GT_HOST_INTR_STATUS_OFF;
        reg->host_intr_clr_addr = pcie_res->ep_res.pst_pci_ctrl_base +
                                GT_HOST_INTR_CLR_OFF;
        reg->host_intr_mask_addr = pcie_res->ep_res.pst_pci_ctrl_base +
                                 GT_HOST_INTR_MASK_OFF;
        reg->h2d_intr_addr = pcie_res->ep_res.pst_pci_ctrl_base +
                           GT_HOST2DEVICE_INTR_SET_OFF;
    }
    reg->ete_intr_sts_addr = pcie_res->ep_res.pst_ete_base +
                             HOST_CTRL_RB_PCIE_CTL_ETE_INTR_STS_OFF;
    reg->ete_intr_clr_addr = pcie_res->ep_res.pst_ete_base +
                             HOST_CTRL_RB_PCIE_CTL_ETE_INTR_CLR_OFF;
    reg->ete_intr_mask_addr = pcie_res->ep_res.pst_ete_base +
                              HOST_CTRL_RB_PCIE_CTL_ETE_INTR_MASK_OFF;
    reg->ete_dr_empty_sts_addr = pcie_res->ep_res.pst_ete_base +
                                     HOST_CTRL_RB_PCIE_CTRL_ETE_CH_DR_EMPTY_INTR_STS_OFF;
    reg->ete_dr_empty_clr_addr = pcie_res->ep_res.pst_ete_base +
                                 HOST_CTRL_RB_PCIE_CTRL_ETE_CH_DR_EMPTY_INTR_CLR_OFF;
}

static int32_t oal_pcie_poweroff_pre(oal_pcie_linux_res *pst_pci_lres)
{
    hreg_host_intr_mask host_intr_mask;
    hreg_host_intr_status host_intr_status;

    if (pst_pci_lres->dev_id == HCC_EP_GT_DEV) {
        return OAL_SUCC;
    }

    hreg_get_val(host_intr_status, pst_pci_lres->ep_res.pst_pci_ctrl_base + PCIE_CTRL_RB_HOST_INTR_STATUS_OFF);
    pci_print_log(PCI_LOG_INFO, "host_intr_status = 0x%x", host_intr_status.as_dword);

    hreg_get_val(host_intr_mask, pst_pci_lres->ep_res.pst_pci_ctrl_base + PCIE_CTRL_RB_HOST_INTR_MASK_OFF);
    host_intr_mask.bits.mac_n2_intr_mask = 1;
    host_intr_mask.bits.mac_n1_intr_mask = 1;
    host_intr_mask.bits.phy_n2_intr_mask = 1;
    host_intr_mask.bits.phy_n1_intr_mask = 1;
    hreg_set_val(host_intr_mask, pst_pci_lres->ep_res.pst_pci_ctrl_base + PCIE_CTRL_RB_HOST_INTR_MASK_OFF);

    hreg_get_val(host_intr_mask, pst_pci_lres->ep_res.pst_pci_ctrl_base + PCIE_CTRL_RB_HOST_INTR_MASK_OFF);
    if (host_intr_mask.bits.mac_n2_intr_mask != 1 ||
        host_intr_mask.bits.mac_n1_intr_mask != 1 ||
        host_intr_mask.bits.phy_n2_intr_mask != 1 ||
        host_intr_mask.bits.phy_n1_intr_mask != 1) {
            pci_print_log(PCI_LOG_ERR, "host_intr_mask = 0x%x", host_intr_mask.as_dword);
            return -OAL_EFAIL;
    }

    return OAL_SUCC;
}

static int32_t oal_pcie_poweroff_complete(oal_pcie_linux_res *pst_pci_res)
{
    pci_addr_map st_map; /* DEVICE power_label地址 */
    uint16_t value;
    int32_t ret;
    uint32_t label_reg;
    uint32_t default_val;

    if (pst_pci_res->dev_id == HCC_EP_GT_DEV) {
        label_reg = PWR_ON_LABLE_REG_GT;
        default_val = DEADBEAF_RESET_VAL_GT;
    } else {
        label_reg = PWR_ON_LABLE_REG_WIFI;
        default_val = DEADBEAF_RESET_VAL_WIFI;
    }

    ret = oal_pcie_inbound_ca_to_va(pst_pci_res, label_reg, &st_map);
    if (ret != OAL_SUCC) {
        pci_print_log(PCI_LOG_ERR, "invalid device address 0x%x, map failed\n", label_reg);
        return -OAL_ENOMEM;
    }
    value = oal_pcie_read_mem16(st_map.va);
    pci_print_log(PCI_LOG_WARN, "power_label 0x%x = 0x%x", label_reg, value);
    if (value == default_val) {
        return OAL_SUCC;
    } else if (value == 0xffff) {
        // check pcie link state
        if (oal_pcie_check_link_state(pst_pci_res) == OAL_FALSE) {
            pci_print_log(PCI_LOG_ERR, "pcie_detect_linkdown");
            return -OAL_ENODEV;
        }

        // retry & check device power_on_label
        value = oal_pcie_read_mem16(st_map.va);
        pci_print_log(PCI_LOG_WARN, "[retry]power_label 0x%x = 0x%x", label_reg, value);
        return -OAL_ENODEV;
    }
    return -OAL_EFAIL;
}

static void oal_pcie_chip_info_cb_init(pcie_chip_cb *cb, int32_t device_id)
{
    cb->get_test_ram_address = oal_pcie_get_test_ram_address;
    cb->pcie_voltage_bias_init = oal_pcie_voltage_bias_init_gf61_int;
    cb->pcie_get_bar_region_info = oal_pcie_get_bar_region_info_gf61;
    cb->pcie_set_outbound_membar = oal_pcie_set_outbound_membar_gf61;
    cb->pcie_host_slave_address_switch = oal_pcie_host_slave_address_switch_gf61_int;
    cb->ete_address_init = oal_ete_res_init;
    cb->ete_intr_init = oal_ete_intr_init;
    cb->pcie_poweroff_pre = oal_pcie_poweroff_pre;
    cb->pcie_poweroff_complete = oal_pcie_poweroff_complete;
}

int32_t oal_pcie_chip_info_init_gf61(oal_pcie_linux_res *pcie_res, int32_t device_id)
{
    oal_pcie_ep_res *ep_res = &pcie_res->ep_res;
    oal_print_mpxx_log(MPXX_LOG_INFO, "oal_pcie_chip_info_init_gf61");
    ep_res->chip_info.dual_pci_support = OAL_FALSE;
    ep_res->chip_info.ete_support = OAL_TRUE;
    ep_res->chip_info.membar_support = OAL_TRUE;
    if (pcie_res->dev_id == HCC_EP_GT_DEV) {
        ep_res->chip_info.addr_info.pcie_ctrl = PA_GT_PCIE_CTRL_BASE_ADDR;
        ep_res->chip_info.addr_info.dbi = PA_GT_PCIE_DBI_BASE_ADDR;
        ep_res->chip_info.addr_info.ete_ctrl = PA_GT_HOST_CTRL_BASE_ADDR;
        ep_res->chip_info.addr_info.sharemem_addr = PA_GT_PCIE_DEVICE_REG_REG;
    } else {
        ep_res->chip_info.addr_info.pcie_ctrl = PCIE_CTRL_RB_BASE;
        ep_res->chip_info.addr_info.dbi = PA_PCIE_DBI_BASE_ADDRESS;
        ep_res->chip_info.addr_info.ete_ctrl = HOST_CTRL_RB_BASE;
        ep_res->chip_info.addr_info.sharemem_addr = PCIE_CTRL_RB_HOST_DEVICE_REG1_REG;
    }
    ep_res->chip_info.addr_info.glb_ctrl = PA_GLB_CTL_BASE_ADDR;
    ep_res->chip_info.addr_info.pmu_ctrl = PA_PMU_CMU_CTL_BASE;
    ep_res->chip_info.addr_info.pmu2_ctrl = PA_PMU2_CMU_IR_BASE;
    ep_res->chip_info.addr_info.boot_version = DEV_VERSION_CPU_ADDR;
    oal_pcie_chip_info_cb_init(&ep_res->chip_info.cb, device_id);
    return OAL_SUCC;
}

void oal_ete_default_intr_cb(void *fun)
{
}

int32_t oal_pcie_slave_soc_config(oal_pcie_linux_res *pcie_lres, oal_pci_slave_config_type type)
{
    if (pcie_lres->ep_res.chip_info.cb.pcie_config_slave != NULL) {
        return pcie_lres->ep_res.chip_info.cb.pcie_config_slave(pcie_lres, type);
    }
    return -OAL_ENODEV;
}
#else
int32_t oal_pcie_slave_soc_config(oal_pcie_linux_res *pcie_lres, oal_pci_slave_config_type type)
{
    return -OAL_ENODEV;
}
#endif
