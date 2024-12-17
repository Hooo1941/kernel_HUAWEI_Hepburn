/**
 * Copyright (c) @CompanyNameMagicTag 2021-2023. All rights reserved.
 *
 * Description:pcie chip adaptation
 * Author: @CompanyNameTag
 */

#ifdef _PRE_PLAT_FEATURE_HI110X_PCIE
#define MPXX_LOG_MODULE_NAME "[PCIEC]"
#define HISI_LOG_TAG           "[PCIEC]"

#include "mp16c/host_ctrl_rb_regs.h"
#include "mp16c/pcie_ctrl_rb_regs.h"
#include "mp16c/pcie_pcs_rb_regs.h"

#include "chip/mp16c/pcie_soc_mp16c.h"
#include "pcie_chip.h"
#include "pcie_linux.h"
#include "pcie_pm.h"
#include "pcie_pcs_trace.h"
#include "oam_ext_if.h"

#undef THIS_FILE_ID
#define THIS_FILE_ID OAM_FILE_ID_PCIE_CHIP_MP16C_C

/* mp16c Registers */
#define PA_PCIE_CTRL_BASE_ADDR      PCIE_CTRL_RB_BASE /* 对应PCIE_CTRL页 */
#define PA_PCIE_DBI_BASE_ADDRESS    0x40107000
#define PA_ETE_CTRL_BASE_ADDRESS    HOST_CTRL_RB_BASE
#define PA_GLB_CTL_BASE_ADDR        0x40000000
#define PA_PMU_CMU_CTL_BASE         0x40002000
#define PA_PMU2_CMU_IR_BASE         0x40012000
#define PA_W_CTL_BASE               0x40105000
#define PWR_ON_LABLE_REG            0X40000200
#define DEADBEAF_RESET_VAL                  ((uint16_t)0x5757)
#define DEV_VERSION_CPU_ADDR        0x00000180
#define W_TRACE_CTL_RB_BASE         (0x40024000 + 0xC0000)
#define CPU_TRACE_SAMPLE_MODE_REG   (W_TRACE_CTL_RB_BASE + 0x200) // trace mode
#define CPU_TRACE_MEM_CLK_REG       (W_TRACE_CTL_RB_BASE + 0x10C) // trace clk select
#define CFG_CPU_MONITOR_CLOCK_REG   (W_TRACE_CTL_RB_BASE + 0x108) // trace clk mode
#define CFG_PC_RAM_SEL              0x1
#define CFG_CPU_TRACE_SAMPLE_EN     0x2
#define CPU_TRACE_MEM_CLKEN         0x3

/* mac优化寄存器 */
#define PCI_EXP_LNKCTL2_SPEED_MASK  0xF
#define PCI_EXP_SPEED_CHANGE_MASK   0x20000
#define LINK_RATE_MASK              0x300000
#define LTSSM_STATE_MASK            0x7e00
#define CURRENT_LINK_RATE_OFFSET    20
#define CURRENT_LINK_STATE_OFFSET   9

typedef struct _pcie_phy_adapter_ {
    char *ini_name;
    uint32_t reg;
    uint8_t pos;
    uint8_t bit;
    int32_t value;
} pcie_phy_adapter;

OAL_STATIC int32_t g_pcie_phy_changespeed_adapter_enable = 0x1FFF;
oal_debug_module_param(g_pcie_phy_changespeed_adapter_enable, int, S_IRUGO | S_IWUSR);
OAL_STATIC int32_t g_pcie_phy_ssi_adapter_enable = 0x1FFF;
oal_debug_module_param(g_pcie_phy_ssi_adapter_enable, int, S_IRUGO | S_IWUSR);
pcie_phy_adapter g_pcie_phy_adapter[] = {
    {NULL, MP16C_PCIE_PHY_DA_PLL_ICP_OFF, 23, 5, 0x1F},
    {NULL, MP16C_PCIE_PHY_REG_RX_TST_OFF, 15, 1, 0x1},
    {NULL, MP16C_PCIE_PHY_REG_RX_TST_OFF, 14, 1, 0x1},
    {NULL, MP16C_PCIE_PHY_DA_RX0_CDR_KP_OFF, 9, 3, 0x0},
#ifdef _PRE_PLAT_FEATURE_HI110X_PCIE_DEBUG
    {NULL, MP16C_PCIE_PHY_RG_RX0_EQ_OFF, 4, 2, 0x0},
    {NULL, MP16C_PCIE_PHY_OFSTK_SEL_COUNT_OFF, 16, 2, 0x3},
    {NULL, MP16C_PCIE_PHY_RX_CTLE_OFF, 9, 1, 0x0},
    {"pcie_phy_sel_r", MP16C_PCIE_PHY_RXCTRL_GSA_OFF, 16, 4, 0x9},
    {NULL, MP16C_PCIE_PHY_RX_CTLE_OFF, 4, 1, 0x1},
    {"pcie_phy_sel_att", MP16C_PCIE_PHY_RXCTRL_GSA_OFF, 8, 3, 0x6},
    {NULL, MP16C_PCIE_PHY_RX_CTLE_OFF, 10, 1, 0x0},
    {NULL, MP16C_PCIE_PHY_RXCTRL_GSA_OFF, 12, 4, 0x0},
    {NULL, MP16C_PCIE_PHY_DA_RX0_CDR_KI_OFF, 4, 6, 0x3}
#endif
};

OAL_STATIC OAL_INLINE int32_t pcie_write_slave(oal_pcie_res *pst_pci_res, uint32_t cpu_addr, uint32_t val)
{
    pci_addr_map addr_map;

    if (oal_pcie_inbound_ca_to_va(pst_pci_res, cpu_addr,
        &addr_map) != OAL_SUCC) {
        oal_io_print("pcie_write32 0x%8x unmap, read failed!\n", cpu_addr);
        return -OAL_EFAIL;
    }
    oal_writel(val, (void *)addr_map.va);

    return OAL_SUCC;
}


void oal_pcie_device_phy_config_by_ssi(uint32_t base_addr)
{
    uint32_t count;
    pcs_reg_ops reg_ops = { 0 };
    pcie_pcs_set_regs_ops(&reg_ops, PCIE_PCS_DUMP_BY_SSI);

    for (count = 0; count < oal_array_size(g_pcie_phy_adapter); count++) {
        if ((uint32_t)g_pcie_phy_ssi_adapter_enable & (1 << count)) {
            if (g_pcie_phy_adapter[count].ini_name != NULL &&
                get_cust_conf_int32(INI_MODU_PLAT, g_pcie_phy_adapter[count].ini_name,
                &g_pcie_phy_adapter[count].value) == INI_FAILED) {
                pci_print_log(PCI_LOG_INFO, "pcie_phy_adapter[%d].ini_name not set", count);
                return;
            }
            pcie_pcs_set_bits32(&reg_ops, (base_addr + g_pcie_phy_adapter[count].reg), g_pcie_phy_adapter[count].pos,
                                g_pcie_phy_adapter[count].bit, g_pcie_phy_adapter[count].value);
            pci_print_log(PCI_LOG_INFO, "by ssi pcie_phy_adapter[%d] enable", count);
        }
    }
}

void oal_pcie_device_phy_config_by_changespeed(oal_pcie_res *pst_pci_res, uint32_t base_addr)
{
    uint32_t count;

    for (count = 0; count < oal_array_size(g_pcie_phy_adapter); count++) {
        if ((uint32_t)g_pcie_phy_changespeed_adapter_enable & (1 << count)) {
            if (g_pcie_phy_adapter[count].ini_name != NULL &&
                get_cust_conf_int32(INI_MODU_PLAT, g_pcie_phy_adapter[count].ini_name,
                &g_pcie_phy_adapter[count].value) == INI_FAILED) {
                pci_print_log(PCI_LOG_INFO, "pcie_phy_adapter[%d].ini_name not set", count);
                return;
            }
            oal_pcie_device_phy_config_reg(pst_pci_res, (base_addr + g_pcie_phy_adapter[count].reg),
                                           g_pcie_phy_adapter[count].pos, g_pcie_phy_adapter[count].bit,
                                           g_pcie_phy_adapter[count].value);
            pci_print_log(PCI_LOG_INFO, "by changespeed pcie_phy_adapter[%d] enable", count);
        }
    }
}

static uintptr_t oal_pcie_get_pcs_paddr(oal_pci_dev_stru *pst_pci_dev, oal_pcie_res *pst_pci_res, uint32_t dump_type)
{
    int32_t ret;
    pci_addr_map st_map;
    uint32_t phy_paddr = MP16C_PCIE_PHY_BASE_ADDRESS;
    if (dump_type == PCIE_PCS_DUMP_BY_SSI) {
        return (uintptr_t)phy_paddr;
    }
    ret = oal_pcie_inbound_ca_to_va(pst_pci_res, phy_paddr, &st_map);
    if (ret != OAL_SUCC) {
        pci_print_log(PCI_LOG_ERR, "get dev address 0x%x failed", phy_paddr);
        return (uintptr_t)NULL;
    }
    /* memport virtual address */
    return (uintptr_t)st_map.va;
}

static void* oal_pcie_get_ctrl_address(oal_pci_dev_stru *pst_pci_dev, oal_pcie_res *pst_pci_res)
{
    if (oal_pcie_is_master_ip(pst_pci_dev) == OAL_TRUE) {
        /* ep0 */
        return pst_pci_res->pst_pci_ctrl_base;
    } else {
        /* ep1 */
        int32_t ret;
        pci_addr_map st_map;
        ret = oal_pcie_inbound_ca_to_va(pst_pci_res, PCIE_CTRL_RB_BASE, &st_map);
        if (ret != OAL_SUCC) {
            pci_print_log(PCI_LOG_ERR, "get dev address 0x%x failed", PCIE_CTRL_RB_BASE);
            return NULL;
        }
        pst_pci_res->pst_pci_ctrl2_base = (void *)st_map.va;
        return pst_pci_res->pst_pci_ctrl2_base;
    }
}

static void oal_pcie_change_speed_before_pcs(oal_pci_dev_stru *pst_pci_dev, oal_pcie_res *pst_pci_res,
                                             void *pcie_ctrl)
{
    pci_print_log(PCI_LOG_INFO,
                  "before: non_l1_rcvry_cnt:0x%x l1_rcvry_cnt:0x%x stat_monitor1:0x%x stat_monitor2:0x%x",
                  oal_readl(pcie_ctrl + PCIE_CTRL_RB_PCIE_STAT_CNT_RCVRY_OFF),
                  oal_readl(pcie_ctrl + PCIE_CTRL_RB_PCIE_STAT_CNT_L1_RCVRY_OFF),
                  oal_readl(pcie_ctrl + PCIE_CTRL_RB_PCIE_STAT_MONITOR1_OFF),
                  oal_readl(pcie_ctrl + PCIE_CTRL_RB_PCIE_STAT_MONITOR2_OFF));
    oal_writel(0xFFFFFFFF, (pcie_ctrl + PCIE_CTRL_RB_PCIE_STAT_MONITOR2_OFF));

    if (pcs_trace_speedchange_flag() != 0) {
        pcie_pcs_tracer_start(oal_pcie_get_pcs_paddr(pst_pci_dev, pst_pci_res, PCIE_PCS_DUMP_BY_MEMPORT),
                              PCS_TRACER_SPEED_CHANGE, 0, PCIE_PCS_DUMP_BY_MEMPORT);
    }
}

static void oal_pcie_change_speed_rc_mac_adapter(oal_pci_dev_stru *pst_rc_dev, int32_t *ep_l1ss_pm, uint32_t *val)
{
    /* debug mac reg before speed-change,bit[19:16]是软件切换后的链路速率 */
    oal_pci_read_config_dword(pst_rc_dev, *ep_l1ss_pm + PCI_EXP_LNKCTL, val);
    pci_print_log(PCI_LOG_INFO, "before: rc mac addr:0x%x PCI_EXP_LNKCTL:0x%x", PCI_EXP_LNKCTL, *val);

    /* 配置TARGET_LINK_SPEED,bit[3:0]是软件切换配置的目标速率,bit[20:17]对应EQ状态 */
    oal_pci_read_config_dword(pst_rc_dev, *ep_l1ss_pm + PCI_EXP_LNKCTL2, val);
    pci_print_log(PCI_LOG_INFO, "before: rc mac addr:0x%x PCI_EXP_LNKCTL2:0x%x", PCI_EXP_LNKCTL2, *val);
    *val &= ~PCI_EXP_LNKCTL2_SPEED_MASK;
    *val |= (uint32_t)(g_pcie_speed_change + 1);
    oal_pci_write_config_dword(pst_rc_dev, *ep_l1ss_pm + PCI_EXP_LNKCTL2, *val);

    /* 配置DIRECT_SPEED_CHANGE,bit[17]直接切速; 不用配置ep进行RETRAIN */
    oal_pci_read_config_dword(pst_rc_dev, *ep_l1ss_pm + PCI_GEN2_CTRL, val);
    pci_print_log(PCI_LOG_INFO, "before: rc mac addr:0x%x PCI_GEN2_CTRL:0x%x", PCI_GEN2_CTRL, *val);
    *val &= ~PCI_EXP_SPEED_CHANGE_MASK;
    *val |= PCI_EXP_SPEED_CHANGE_MASK;
    oal_pci_write_config_dword(pst_rc_dev, *ep_l1ss_pm + PCI_GEN2_CTRL, *val);
}

static void oal_pcie_change_speed_check(oal_pci_dev_stru *pst_pci_dev, oal_pcie_res *pst_pci_res, const void *pcie_ctrl,
                                        uint8_t *speed_change_succ, uint32_t *link_speed, uint32_t *link_sts)
{
    msleep(1);
    *link_speed = ((oal_readl(pcie_ctrl + PCIE_CTRL_RB_PCIE_STATUS2_OFF)) &
                   LINK_RATE_MASK) >> CURRENT_LINK_RATE_OFFSET;
    *link_sts = ((oal_readl(pcie_ctrl + PCIE_CTRL_RB_PCIE_STATUS0_OFF)) &
                 LTSSM_STATE_MASK) >> CURRENT_LINK_STATE_OFFSET;
    if (*link_speed != g_pcie_speed_change) {
        oam_error_log1(0, OAM_SF_PWR, "speed change fail to %d[0:GEN1 1:GEN2 2:GEN3]", g_pcie_speed_change);
        pci_print_log(PCI_LOG_ERR, "speed change fail to %d[0:GEN1 1:GEN2 2:GEN3]", g_pcie_speed_change);
        declare_dft_trace_key_info("pcie_speed_change_fail", OAL_DFT_TRACE_FAIL);
        chr_exception_report(CHR_PLATFORM_EXCEPTION_EVENTID, CHR_SYSTEM_PLAT, CHR_LAYER_DRV,
                             CHR_PLT_DRV_EVENT_PCIE, CHR_PLAT_DRV_ERROR_PCIE_SPEED_CHANGE_FAIL);
#if defined(CONFIG_HUAWEI_OHOS_DSM) || defined(CONFIG_HUAWEI_DSM)
        hw_mpxx_dsm_client_notify(SYSTEM_TYPE_PLATFORM, DSM_WIFI_PCIE_LINKDOWN,
                                  "%s: pcie speed change fail\n", __FUNCTION__);
#endif
        if (pcs_trace_speedchange_flag() & 0x1) {
            pcie_pcs_tracer_dump(oal_pcie_get_pcs_paddr(pst_pci_dev, pst_pci_res, PCIE_PCS_DUMP_BY_SSI),
                                 0, PCIE_PCS_DUMP_BY_SSI);
        }
    } else {
        *speed_change_succ = 1;
        oam_warning_log1(0, OAM_SF_PWR, "speed change success to %d[0:GEN1 1:GEN2 2:GEN3]", g_pcie_speed_change);
        pci_print_log(PCI_LOG_INFO, "speed change success to %d[0:GEN1 1:GEN2 2:GEN3]", g_pcie_speed_change);
        declare_dft_trace_key_info("pcie_speed_change_succ", OAL_DFT_TRACE_SUCC);
        if (pcs_trace_speedchange_flag() & 0x2) {
            pcie_pcs_tracer_dump(oal_pcie_get_pcs_paddr(pst_pci_dev, pst_pci_res, PCIE_PCS_DUMP_BY_SSI),
                                 0, PCIE_PCS_DUMP_BY_SSI);
        }
    }
}

static void oal_pcie_change_speed_after_pcs(oal_pci_dev_stru *pst_rc_dev, void *pcie_ctrl, int32_t *ep_l1ss_pm,
                                            uint32_t *link_speed, uint32_t *link_sts, uint32_t *val)
{
    pci_print_log(PCI_LOG_INFO,
                  "after: non_l1_rcvry_cnt:0x%x l1_rcvry_cnt:0x%x stat_monitor1:0x%x stat_monitor2:0x%x",
                  oal_readl(pcie_ctrl + PCIE_CTRL_RB_PCIE_STAT_CNT_RCVRY_OFF),
                  oal_readl(pcie_ctrl + PCIE_CTRL_RB_PCIE_STAT_CNT_L1_RCVRY_OFF),
                  oal_readl(pcie_ctrl + PCIE_CTRL_RB_PCIE_STAT_MONITOR1_OFF),
                  oal_readl(pcie_ctrl + PCIE_CTRL_RB_PCIE_STAT_MONITOR2_OFF));
    /* debug mac reg after speed-change */
    oal_pci_read_config_dword(pst_rc_dev, *ep_l1ss_pm + PCI_EXP_LNKCTL, val);
    pci_print_log(PCI_LOG_INFO, "after: rc mac addr:0x%x PCI_EXP_LNKCTL:0x%x", PCI_EXP_LNKCTL, *val);
    oal_pci_read_config_dword(pst_rc_dev, *ep_l1ss_pm + PCI_EXP_LNKCTL2, val);
    pci_print_log(PCI_LOG_INFO, "after: rc mac addr:0x%x PCI_EXP_LNKCTL2:0x%x", PCI_EXP_LNKCTL2, *val);
    oal_pci_read_config_dword(pst_rc_dev, *ep_l1ss_pm + PCI_GEN2_CTRL, val);
    pci_print_log(PCI_LOG_INFO, "after: rc mac addr:0x%x PCI_GEN2_CTRL:0x%x", PCI_GEN2_CTRL, *val);
    pci_print_log(PCI_LOG_INFO, "speed change result: link_sts = 0x%x, link_speed = 0x%x", *link_sts, *link_speed);
}

#define LINK_SPEED_CHANGE_MAX_ROUND 2
void oal_pcie_link_speed_change_mp16c(oal_pci_dev_stru *pst_pci_dev, oal_pcie_res *pst_pci_res)
{
    void *pcie_ctrl = NULL;
    oal_pci_dev_stru *pst_rc_dev = NULL;
    uint32_t val = 0;
    int32_t ep_l1ss_pm = 0;
    uint8_t speed_change_round = 1;
    uint8_t speed_change_succ = 0;
    uint32_t link_speed, link_sts;

    pcie_ctrl = oal_pcie_get_ctrl_address(pst_pci_dev, pst_pci_res);
    if (pcie_ctrl == NULL) {
        pci_print_log(PCI_LOG_ERR, "pcie_ctrl virtual address is null");
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
        if (g_pcie_speed_change == PCIE_GEN1) {
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
        oal_pcie_change_speed_before_pcs(pst_pci_dev, pst_pci_res, pcie_ctrl);

        /* Step4: 配置rc侧mac寄存器 */
        pst_rc_dev = pci_upstream_bridge(pst_pci_dev);
        if (pst_rc_dev != NULL) {
            ep_l1ss_pm = oal_pci_pcie_cap(pst_rc_dev);
            if (!ep_l1ss_pm) {
                pci_print_log(PCI_LOG_ERR, "pcie link speed change fail, cannot get rc addr");
                continue;
            }
            oal_pcie_change_speed_rc_mac_adapter(pst_rc_dev, &ep_l1ss_pm, &val);
        }

        /* Step5: 固定delay 1ms, EDA仿真不超过300us;并确认链路速度,bit[21:20] */
        oal_pcie_change_speed_check(pst_pci_dev, pst_pci_res, pcie_ctrl, &speed_change_succ, &link_speed, &link_sts);

        /* Step6: 链路状态机数采维测，预期non_l1_rcvry_cnt=0x3, l1_rcvry_cnt=0x0 */
        oal_pcie_change_speed_after_pcs(pst_rc_dev, pcie_ctrl, &ep_l1ss_pm, &link_speed, &link_sts, &val);
    }
}

int32_t oal_pcie_get_bar_region_info_mp16c(oal_pcie_res *pst_pci_res, oal_pcie_region **, uint32_t *);
int32_t oal_pcie_set_outbound_membar_mp16c(oal_pcie_res *pst_pci_res, oal_pcie_iatu_bar* pst_iatu_bar);

static uintptr_t oal_pcie_get_test_ram_address(void)
{
    return 0x2000000; // pkt mem for test, by acp port
}

static int32_t oal_pcie_voltage_bias_init_mp16c(oal_pcie_res *pst_pci_res)
{
    oal_print_mpxx_log(MPXX_LOG_ERR, "oal_pcie_chip_info_init_mp16c is not implement!");
    return OAL_SUCC;
}

static int32_t oal_pcie_host_slave_address_switch_mp16c(oal_pcie_res *pst_pci_res, uint64_t src_addr,
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

/* device intx/msi init */
static int32_t oal_ete_intr_init(oal_pcie_res *pst_pci_res)
{
    hreg_host_intr_mask intr_mask;
#ifdef _PRE_COMMENT_CODE_
    hreg_pcie_ctrl_ete_ch_dr_empty_intr_mask host_dre_mask;
#endif

    /* enable interrupts */
    if (pst_pci_res->pst_pci_ctrl_base == NULL) {
        oal_print_mpxx_log(MPXX_LOG_INFO, "pst_pci_ctrl_base is null");
        return -OAL_ENODEV;
    }

    hreg_get_val(intr_mask, pst_pci_res->pst_pci_ctrl_base + PCIE_CTRL_RB_HOST_INTR_MASK_OFF);

    intr_mask.bits.host2device_tx_intr_mask = 0;
    intr_mask.bits.device2host_tx_intr_mask = 0;
    intr_mask.bits.device2host_rx_intr_mask = 0;
    intr_mask.bits.device2host_intr_mask = 0;

    /* WiFi中断需要在WiFi回调注册后Unmask */
    intr_mask.bits.mac_n1_intr_mask = 0;
    intr_mask.bits.mac_n2_intr_mask = 0;
    intr_mask.bits.phy_n1_intr_mask = 0;
    intr_mask.bits.phy_n2_intr_mask = 0;

    hreg_set_val(intr_mask, pst_pci_res->pst_pci_ctrl_base + PCIE_CTRL_RB_HOST_INTR_MASK_OFF);
    hreg_get_val(intr_mask, pst_pci_res->pst_pci_ctrl_base + PCIE_CTRL_RB_HOST_INTR_MASK_OFF);
    oal_print_mpxx_log(MPXX_LOG_INFO, "pcie intr mask=0x%x", intr_mask.as_dword);

    /* mask:1 for mask, 0 for unmask */
#ifdef _PRE_COMMENT_CODE_
    hreg_get_val(host_dre_mask, pst_pci_res->pst_pci_ctrl_base + PCIE_CTRL_RB_PCIE_CTRL_ETE_CH_DR_EMPTY_INTR_MASK_OFF);
    host_dre_mask.bits.host_rx_ch_dr_empty_intr_mask = 0; /* unmask rx dre int */
    hreg_set_val(host_dre_mask,  pst_pci_res->pst_pci_ctrl_base +
                 PCIE_CTRL_RB_PCIE_CTRL_ETE_CH_DR_EMPTY_INTR_MASK_OFF);
#endif

    /* unmask all ete host int */
    oal_writel(0x0, pst_pci_res->ete_info.reg.ete_intr_mask_addr);

    return OAL_SUCC;
}

static void oal_ete_res_init(oal_pcie_res *pst_pci_res)
{
    oal_ete_reg *reg = &pst_pci_res->ete_info.reg;

    reg->host_intr_sts_addr = pst_pci_res->pst_pci_ctrl_base +
                              PCIE_CTRL_RB_HOST_INTR_STATUS_OFF;
    reg->host_intr_clr_addr = pst_pci_res->pst_pci_ctrl_base +
                              PCIE_CTRL_RB_HOST_INTR_CLR_OFF;
    reg->ete_intr_sts_addr = pst_pci_res->pst_ete_base +
                             HOST_CTRL_RB_PCIE_CTL_ETE_INTR_STS_OFF;
    reg->ete_intr_clr_addr = pst_pci_res->pst_ete_base +
                             HOST_CTRL_RB_PCIE_CTL_ETE_INTR_CLR_OFF;
    reg->ete_intr_mask_addr = pst_pci_res->pst_ete_base +
                              HOST_CTRL_RB_PCIE_CTL_ETE_INTR_MASK_OFF;
    reg->ete_dr_empty_sts_addr = pst_pci_res->pst_ete_base +
                                     HOST_CTRL_RB_PCIE_CTRL_ETE_CH_DR_EMPTY_INTR_STS_OFF;
    reg->ete_dr_empty_clr_addr = pst_pci_res->pst_ete_base +
                                 HOST_CTRL_RB_PCIE_CTRL_ETE_CH_DR_EMPTY_INTR_CLR_OFF;
    reg->h2d_intr_addr = pst_pci_res->pst_pci_ctrl_base + PCIE_CTRL_RB_HOST2DEVICE_INTR_SET_OFF;
    reg->host_intr_mask_addr = pst_pci_res->pst_pci_ctrl_base + PCIE_CTRL_RB_HOST_INTR_MASK_OFF;
}

static int32_t oal_pcie_poweroff_pre(oal_pcie_res *pst_pci_res)
{
    hreg_host_intr_mask host_intr_mask;
    hreg_host_intr_status host_intr_status;

    hreg_get_val(host_intr_status, pst_pci_res->pst_pci_ctrl_base + PCIE_CTRL_RB_HOST_INTR_STATUS_OFF);
    pci_print_log(PCI_LOG_INFO, "host_intr_status = 0x%x", host_intr_status.as_dword);

    hreg_get_val(host_intr_mask, pst_pci_res->pst_pci_ctrl_base + PCIE_CTRL_RB_HOST_INTR_MASK_OFF);
    host_intr_mask.bits.mac_n2_intr_mask = 1;
    host_intr_mask.bits.mac_n1_intr_mask = 1;
    host_intr_mask.bits.phy_n2_intr_mask = 1;
    host_intr_mask.bits.phy_n1_intr_mask = 1;
    hreg_set_val(host_intr_mask, pst_pci_res->pst_pci_ctrl_base + PCIE_CTRL_RB_HOST_INTR_MASK_OFF);

    hreg_get_val(host_intr_mask, pst_pci_res->pst_pci_ctrl_base + PCIE_CTRL_RB_HOST_INTR_MASK_OFF);
    if (host_intr_mask.bits.mac_n2_intr_mask != 1 ||
        host_intr_mask.bits.mac_n1_intr_mask != 1 ||
        host_intr_mask.bits.phy_n2_intr_mask != 1 ||
        host_intr_mask.bits.phy_n1_intr_mask != 1) {
            pci_print_log(PCI_LOG_ERR, "host_intr_mask = 0x%x", host_intr_mask.as_dword);
            return -OAL_EFAIL;
    }

    return OAL_SUCC;
}

static int32_t oal_pcie_poweroff_complete(oal_pcie_res *pst_pci_res)
{
    pci_addr_map st_map; /* DEVICE power_label地址 */
    uint16_t value;
    int32_t ret;

    ret = oal_pcie_inbound_ca_to_va(pst_pci_res, PWR_ON_LABLE_REG, &st_map);
    if (ret != OAL_SUCC) {
        pci_print_log(PCI_LOG_ERR, "invalid device address 0x%x, map failed\n", PWR_ON_LABLE_REG);
        return -OAL_ENOMEM;
    }
    value = oal_pcie_read_mem16(st_map.va);
    pci_print_log(PCI_LOG_WARN, "power_label 0x%x = 0x%x", PWR_ON_LABLE_REG, value);
    if (value == DEADBEAF_RESET_VAL) {
        return OAL_SUCC;
    } else if (value == 0xffff) {
        // check pcie link state
        if (oal_pcie_check_link_state(pst_pci_res) == OAL_FALSE) {
            pci_print_log(PCI_LOG_ERR, "pcie_detect_linkdown");
            return -OAL_ENODEV;
        }

        // retry & check device power_on_label
        value = oal_pcie_read_mem16(st_map.va);
        pci_print_log(PCI_LOG_WARN, "[retry]power_label 0x%x = 0x%x", PWR_ON_LABLE_REG, value);
        return -OAL_ENODEV;
    }
    return -OAL_EFAIL;
}

static int32_t oal_pcie_dyn_mp16c(oal_pcie_res *pst_pci_res, uint32_t request_speed)
{
    oal_udelay(50); // delay 50 us
    return OAL_SUCC;
}

static int32_t oal_pcie_config_slave_mp16c(oal_pcie_res *pst_pci_res, oal_pci_slave_config_type slave_type)
{
    int32_t ret;

    if (slave_type == PCI_CPUTRACE) {
        // clear sample enable
        ret = pcie_write_slave(pst_pci_res, CPU_TRACE_SAMPLE_MODE_REG,
                               CFG_CPU_TRACE_SAMPLE_EN);
        if (ret != OAL_SUCC) {
            oal_print_mpxx_log(MPXX_LOG_ERR, "pcie write trace en fail\n");
            return ret;
        }
        // select trace mem clk source
        ret = pcie_write_slave(pst_pci_res, CPU_TRACE_MEM_CLK_REG,
                               CFG_PC_RAM_SEL);
        if (ret != OAL_SUCC) {
            oal_print_mpxx_log(MPXX_LOG_ERR, "pcie write trace ram sel fail\n");
            return ret;
        }
        // trace memory clk enable
        ret = pcie_write_slave(pst_pci_res, CFG_CPU_MONITOR_CLOCK_REG,
                               CPU_TRACE_MEM_CLKEN);
        if (ret != OAL_SUCC) {
            oal_print_mpxx_log(MPXX_LOG_ERR, "pcie write trace clk en fail\n");
            return ret;
        }
    }

    return OAL_SUCC;
}

static void oal_pcie_chip_info_cb_init(pcie_chip_cb *cb, int32_t device_id)
{
    cb->get_test_ram_address = oal_pcie_get_test_ram_address;
    cb->pcie_config_slave = oal_pcie_config_slave_mp16c;
    cb->pcie_voltage_bias_init = oal_pcie_voltage_bias_init_mp16c;
    cb->pcie_get_bar_region_info = oal_pcie_get_bar_region_info_mp16c;
    cb->pcie_set_outbound_membar = oal_pcie_set_outbound_membar_mp16c;
    cb->pcie_host_slave_address_switch = oal_pcie_host_slave_address_switch_mp16c;
    cb->pcie_poweroff_pre = oal_pcie_poweroff_pre;
    cb->pcie_poweroff_complete = oal_pcie_poweroff_complete;
    cb->ete_address_init = oal_ete_res_init;
    cb->ete_intr_init = oal_ete_intr_init;
    cb->pcie_dyn_change_linkspeed = oal_pcie_dyn_mp16c;
}

int32_t oal_pcie_chip_info_init_mp16c(oal_pcie_res *pst_pci_res, int32_t device_id)
{
    oal_print_mpxx_log(MPXX_LOG_INFO, "oal_pcie_chip_info_init_mp16c");
    pst_pci_res->chip_info.dual_pci_support = OAL_FALSE;
    pst_pci_res->chip_info.ete_support = OAL_TRUE;
    pst_pci_res->chip_info.membar_support = OAL_TRUE;
    pst_pci_res->chip_info.addr_info.pcie_ctrl = PA_PCIE_CTRL_BASE_ADDR;
    pst_pci_res->chip_info.addr_info.dbi = PA_PCIE_DBI_BASE_ADDRESS;
    pst_pci_res->chip_info.addr_info.ete_ctrl = PA_ETE_CTRL_BASE_ADDRESS;
    pst_pci_res->chip_info.addr_info.glb_ctrl = PA_GLB_CTL_BASE_ADDR;
    pst_pci_res->chip_info.addr_info.pmu_ctrl = PA_PMU_CMU_CTL_BASE;
    pst_pci_res->chip_info.addr_info.pmu2_ctrl = PA_PMU2_CMU_IR_BASE;
    pst_pci_res->chip_info.addr_info.boot_version = DEV_VERSION_CPU_ADDR;
    pst_pci_res->chip_info.addr_info.sharemem_addr = PCIE_CTRL_RB_HOST_DEVICE_REG1_REG;
    oal_pcie_chip_info_cb_init(&pst_pci_res->chip_info.cb, device_id);
    return OAL_SUCC;
}

static int32_t oal_pcie_linkup_prepare_fixup(int32_t ep_idx)
{
#ifdef _PRE_PLAT_FEATURE_HI110X_PCIE_DEBUG
    oal_pcie_device_phy_config_by_ssi(MP16C_PCIE_PHY_BASE_ADDRESS);
#endif

    return OAL_SUCC;
}

static int32_t oal_pcie_linkup_fail_fixup(int32_t ep_idx)
{
    chr_exception_report(CHR_PLATFORM_EXCEPTION_EVENTID, CHR_SYSTEM_PLAT, CHR_LAYER_DRV,
                         CHR_PLT_DRV_EVENT_PCIE, CHR_PLAT_DRV_ERROR_PCIE_POWERON_LINK_FAIL);
    return OAL_SUCC;
}

static int32_t oal_pcie_linkup_succ_fixup(int32_t ep_idx)
{
    return OAL_SUCC;
}

int32_t oal_pcie_board_init_mp16c(linkup_fixup_ops *ops)
{
    ops->link_prepare_fixup = oal_pcie_linkup_prepare_fixup;
    ops->link_fail_fixup = oal_pcie_linkup_fail_fixup;
    ops->link_succ_fixup = oal_pcie_linkup_succ_fixup;
    return OAL_SUCC;
}

void oal_ete_default_intr_cb(void *fun)
{
}

#endif
