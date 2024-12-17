#ifndef __SOC_SYSQIC_INTERFACE_H__
#define __SOC_SYSQIC_INTERFACE_H__ 
#ifdef __cplusplus
#if __cplusplus
    extern "C" {
#endif
#endif
#ifndef __SOC_H_FOR_ASM__
#define SOC_SYSQIC_IB_RTL_VER_ADDR(base) ((base) + (0x0000UL))
#define SOC_SYSQIC_IB_RTL_INFO_ADDR(base) ((base) + (0x0004UL))
#define SOC_SYSQIC_IB_RTL_INF1_ADDR(base) ((base) + (0x0008UL))
#define SOC_SYSQIC_IB_RTL_INF2_ADDR(base) ((base) + (0x000CUL))
#define SOC_SYSQIC_IB_CKG_CTRL_ADDR(base) ((base) + (0x0100UL))
#define SOC_SYSQIC_IB_RAM_TMOD_ADDR(base) ((base) + (0x0104UL))
#define SOC_SYSQIC_IB_RSV_RW_ADDR(base) ((base) + (0x010CUL))
#define SOC_SYSQIC_IB_CS_CTRL_ADDR(base) ((base) + (0x0110UL))
#define SOC_SYSQIC_IB_WA_CTRL_ADDR(base) ((base) + (0x0114UL))
#define SOC_SYSQIC_IB_CR_CTRL_ADDR(base) ((base) + (0x0118UL))
#define SOC_SYSQIC_IB_CT_CTRL_ADDR(base) ((base) + (0x011CUL))
#define SOC_SYSQIC_IB_TIMEOUT_CTRL_ADDR(base) ((base) + (0x0130UL))
#define SOC_SYSQIC_IB_ECC_CTRL_ADDR(base) ((base) + (0x0134UL))
#define SOC_SYSQIC_IB_PARITY_CTRL_ADDR(base) ((base) + (0x0138UL))
#define SOC_SYSQIC_IB_QOS_LMTR0_CTRL_ADDR(base) ((base) + (0x0140UL))
#define SOC_SYSQIC_IB_QOS_LMTR1_CTRL_ADDR(base) ((base) + (0x0144UL))
#define SOC_SYSQIC_IB_QOS_LMTR2_CTRL_ADDR(base) ((base) + (0x0148UL))
#define SOC_SYSQIC_IB_QOS_LMTR3_CTRL_ADDR(base) ((base) + (0x014CUL))
#define SOC_SYSQIC_IB_QOS_OSTD0_CTRL_ADDR(base) ((base) + (0x0150UL))
#define SOC_SYSQIC_IB_QOS_OSTD1_CTRL_ADDR(base) ((base) + (0x0154UL))
#define SOC_SYSQIC_IB_QOS_OSTD2_CTRL_ADDR(base) ((base) + (0x0158UL))
#define SOC_SYSQIC_IB_QOS_OSTD3_CTRL_ADDR(base) ((base) + (0x015CUL))
#define SOC_SYSQIC_IB_QOS_RDQOS_CTRL_ADDR(base) ((base) + (0x0160UL))
#define SOC_SYSQIC_IB_QOS_WRQOS_CTRL_ADDR(base) ((base) + (0x0164UL))
#define SOC_SYSQIC_IB_QOS_RDPRI_CTRL_ADDR(base) ((base) + (0x0168UL))
#define SOC_SYSQIC_IB_QOS_WRPRI_CTRL_ADDR(base) ((base) + (0x016CUL))
#define SOC_SYSQIC_IB_QOS_PUSH_CTRL_ADDR(base) ((base) + (0x0170UL))
#define SOC_SYSQIC_IB_QOS_LMTR_LITE_CTRL_ADDR(base) ((base) + (0x0174UL))
#define SOC_SYSQIC_IB_QOS_RGLR0_CTRL_ADDR(base) ((base) + (0x0180UL))
#define SOC_SYSQIC_IB_QOS_RGLR1_CTRL_ADDR(base) ((base) + (0x0184UL))
#define SOC_SYSQIC_IB_PROBE_WREQ_CFG0_ADDR(base) ((base) + (0x0200UL))
#define SOC_SYSQIC_IB_PROBE_WREQ_CFG1_ADDR(base) ((base) + (0x0204UL))
#define SOC_SYSQIC_IB_PROBE_WREQ_FILTER0_ADDR(base) ((base) + (0x0210UL))
#define SOC_SYSQIC_IB_PROBE_WREQ_FILTER1_ADDR(base) ((base) + (0x0214UL))
#define SOC_SYSQIC_IB_PROBE_WREQ_FILTER2_ADDR(base) ((base) + (0x0218UL))
#define SOC_SYSQIC_IB_PROBE_WREQ_FILTER3_ADDR(base) ((base) + (0x021CUL))
#define SOC_SYSQIC_IB_PROBE_WREQ_FILTER4_0_ADDR(base) ((base) + (0x0220UL))
#define SOC_SYSQIC_IB_PROBE_WREQ_FILTER4_1_ADDR(base) ((base) + (0x0224UL))
#define SOC_SYSQIC_IB_PROBE_WREQ_FILTER4_2_ADDR(base) ((base) + (0x0228UL))
#define SOC_SYSQIC_IB_PROBE_WREQ_FILTER4_3_ADDR(base) ((base) + (0x022CUL))
#define SOC_SYSQIC_IB_PROBE_WREQ_FILTER5_0_ADDR(base) ((base) + (0x0230UL))
#define SOC_SYSQIC_IB_PROBE_WREQ_FILTER5_1_ADDR(base) ((base) + (0x0234UL))
#define SOC_SYSQIC_IB_PROBE_WREQ_FILTER5_2_ADDR(base) ((base) + (0x0238UL))
#define SOC_SYSQIC_IB_PROBE_WREQ_FILTER5_3_ADDR(base) ((base) + (0x023CUL))
#define SOC_SYSQIC_IB_PROBE_WREQ_FILTER6_ADDR(base) ((base) + (0x0240UL))
#define SOC_SYSQIC_IB_PROBE_WREQ_FILTER7_ADDR(base) ((base) + (0x0244UL))
#define SOC_SYSQIC_IB_PROBE_WREQ_FILTER8_ADDR(base) ((base) + (0x0248UL))
#define SOC_SYSQIC_IB_PROBE_WREQ_FILTER9_ADDR(base) ((base) + (0x024CUL))
#define SOC_SYSQIC_IB_PROBE_WREQ_INT_STAUS_ADDR(base) ((base) + (0x0250UL))
#define SOC_SYSQIC_IB_PROBE_WREQ_INT_CLR_ADDR(base) ((base) + (0x0254UL))
#define SOC_SYSQIC_IB_PROBE_WREQ_CNT_ADDR(base,cnt_reg_num) ((base) + (0x0280+0x4*(cnt_reg_num)))
#define SOC_SYSQIC_IB_PROBE_WREQ_INT_CFG_ADDR(base,cnt_reg_num) ((base) + (0x02C0+0x4*(cnt_reg_num)))
#define SOC_SYSQIC_IB_PROBE_RREQ_CFG0_ADDR(base) ((base) + (0x0300UL))
#define SOC_SYSQIC_IB_PROBE_RREQ_CFG1_ADDR(base) ((base) + (0x0304UL))
#define SOC_SYSQIC_IB_PROBE_RREQ_FILTER0_ADDR(base) ((base) + (0x0310UL))
#define SOC_SYSQIC_IB_PROBE_RREQ_FILTER1_ADDR(base) ((base) + (0x0314UL))
#define SOC_SYSQIC_IB_PROBE_RREQ_FILTER2_ADDR(base) ((base) + (0x0318UL))
#define SOC_SYSQIC_IB_PROBE_RREQ_FILTER3_ADDR(base) ((base) + (0x031CUL))
#define SOC_SYSQIC_IB_PROBE_RREQ_FILTER4_0_ADDR(base) ((base) + (0x0320UL))
#define SOC_SYSQIC_IB_PROBE_RREQ_FILTER4_1_ADDR(base) ((base) + (0x0324UL))
#define SOC_SYSQIC_IB_PROBE_RREQ_FILTER4_2_ADDR(base) ((base) + (0x0328UL))
#define SOC_SYSQIC_IB_PROBE_RREQ_FILTER4_3_ADDR(base) ((base) + (0x032CUL))
#define SOC_SYSQIC_IB_PROBE_RREQ_FILTER5_0_ADDR(base) ((base) + (0x0330UL))
#define SOC_SYSQIC_IB_PROBE_RREQ_FILTER5_1_ADDR(base) ((base) + (0x0334UL))
#define SOC_SYSQIC_IB_PROBE_RREQ_FILTER5_2_ADDR(base) ((base) + (0x0338UL))
#define SOC_SYSQIC_IB_PROBE_RREQ_FILTER5_3_ADDR(base) ((base) + (0x033CUL))
#define SOC_SYSQIC_IB_PROBE_RREQ_FILTER6_ADDR(base) ((base) + (0x0340UL))
#define SOC_SYSQIC_IB_PROBE_RREQ_FILTER7_ADDR(base) ((base) + (0x0344UL))
#define SOC_SYSQIC_IB_PROBE_RREQ_FILTER8_ADDR(base) ((base) + (0x0348UL))
#define SOC_SYSQIC_IB_PROBE_RREQ_FILTER9_ADDR(base) ((base) + (0x034CUL))
#define SOC_SYSQIC_IB_PROBE_RREQ_INT_STATUS_ADDR(base) ((base) + (0x0350UL))
#define SOC_SYSQIC_IB_PROBE_RREQ_INT_CLR_ADDR(base) ((base) + (0x0354UL))
#define SOC_SYSQIC_IB_PROBE_RREQ_CNT_ADDR(base,cnt_reg_num) ((base) + (0x0380+0x4*(cnt_reg_num)))
#define SOC_SYSQIC_IB_PROBE_RREQ_INT_CFG_ADDR(base,cnt_reg_num) ((base) + (0x03C0+0x4*(cnt_reg_num)))
#define SOC_SYSQIC_IB_DFX_INT_CTRL0_ADDR(base) ((base) + (0x0600UL))
#define SOC_SYSQIC_IB_DFX_INT_CTRL1_ADDR(base) ((base) + (0x0604UL))
#define SOC_SYSQIC_IB_DFX_INT_STATUS_ADDR(base) ((base) + (0x0608UL))
#define SOC_SYSQIC_IB_DFX_INT_CLR_ADDR(base) ((base) + (0x060CUL))
#define SOC_SYSQIC_IB_DFX_ERR_INF0_ADDR(base) ((base) + (0x0610UL))
#define SOC_SYSQIC_IB_DFX_ERR_INF1_ADDR(base) ((base) + (0x0614UL))
#define SOC_SYSQIC_IB_DFX_ERR_INF2_ADDR(base) ((base) + (0x0618UL))
#define SOC_SYSQIC_IB_DFX_ERR_INF3_ADDR(base) ((base) + (0x061CUL))
#define SOC_SYSQIC_IB_DFX_ORG_INT0_ADDR(base) ((base) + (0x0620UL))
#define SOC_SYSQIC_IB_DFX_ORG_INT1_ADDR(base) ((base) + (0x0624UL))
#define SOC_SYSQIC_IB_DFX_SAFETY_ORG_INT_ADDR(base) ((base) + (0x0628UL))
#define SOC_SYSQIC_IB_DFX_SAFETY_INF0_ADDR(base) ((base) + (0x0640UL))
#define SOC_SYSQIC_IB_DFX_SAFETY_INF1_ADDR(base) ((base) + (0x0644UL))
#define SOC_SYSQIC_IB_DFX_SAFETY_INF2_ADDR(base) ((base) + (0x0648UL))
#define SOC_SYSQIC_IB_DFX_SAFETY_INF3_ADDR(base) ((base) + (0x064CUL))
#define SOC_SYSQIC_IB_DFX_TABLE_CTRL_ADDR(base) ((base) + (0x0680UL))
#define SOC_SYSQIC_IB_DFX_TABLE_INF0_ADDR(base) ((base) + (0x0690UL))
#define SOC_SYSQIC_IB_DFX_TABLE_INF1_ADDR(base) ((base) + (0x0694UL))
#define SOC_SYSQIC_IB_DFX_TABLE_INF2_ADDR(base) ((base) + (0x0698UL))
#define SOC_SYSQIC_IB_DFX_TABLE_INF3_ADDR(base) ((base) + (0x069CUL))
#define SOC_SYSQIC_IB_SOFT_LPC_CTRL_ADDR(base) ((base) + (0x06C0UL))
#define SOC_SYSQIC_IB_DFX_MODULE_ADDR(base) ((base) + (0x0700UL))
#define SOC_SYSQIC_IB_DFX_BP_ADDR(base) ((base) + (0x0704UL))
#define SOC_SYSQIC_IB_DFX_OT_ADDR(base) ((base) + (0x0708UL))
#define SOC_SYSQIC_IB_DFX_BUF0_ADDR(base) ((base) + (0x0710UL))
#define SOC_SYSQIC_IB_DFX_BUF1_ADDR(base) ((base) + (0x0714UL))
#define SOC_SYSQIC_IB_DFX_LPC_ADDR(base) ((base) + (0x0718UL))
#define SOC_SYSQIC_IB_DFX_RREQ_TX0_ADDR(base) ((base) + (0x0720UL))
#define SOC_SYSQIC_IB_DFX_RREQ_TX1_ADDR(base) ((base) + (0x0724UL))
#define SOC_SYSQIC_IB_DFX_RREQ_TX2_ADDR(base) ((base) + (0x0728UL))
#define SOC_SYSQIC_IB_DFX_RREQ_TX3_ADDR(base) ((base) + (0x072CUL))
#define SOC_SYSQIC_IB_DFX_WREQ_TX0_ADDR(base) ((base) + (0x0730UL))
#define SOC_SYSQIC_IB_DFX_WREQ_TX1_ADDR(base) ((base) + (0x0734UL))
#define SOC_SYSQIC_IB_DFX_WREQ_TX2_ADDR(base) ((base) + (0x0738UL))
#define SOC_SYSQIC_IB_DFX_WREQ_TX3_ADDR(base) ((base) + (0x073CUL))
#define SOC_SYSQIC_IB_DFX_RSP_RX_ADDR(base) ((base) + (0x0740UL))
#define SOC_SYSQIC_IB_DFX_CFG_PORT_ADDR(base) ((base) + (0x0744UL))
#define SOC_SYSQIC_IB_DFX_RCT_TIMEOUT0_ADDR(base) ((base) + (0x0750UL))
#define SOC_SYSQIC_IB_DFX_RCT_TIMEOUT1_ADDR(base) ((base) + (0x0754UL))
#define SOC_SYSQIC_IB_DFX_RCT_TIMEOUT2_ADDR(base) ((base) + (0x0758UL))
#define SOC_SYSQIC_IB_DFX_RCT_TIMEOUT3_ADDR(base) ((base) + (0x075CUL))
#define SOC_SYSQIC_IB_DFX_WCT_TIMEOUT0_ADDR(base) ((base) + (0x0760UL))
#define SOC_SYSQIC_IB_DFX_WCT_TIMEOUT1_ADDR(base) ((base) + (0x0764UL))
#define SOC_SYSQIC_IB_DFX_WCT_TIMEOUT2_ADDR(base) ((base) + (0x0768UL))
#define SOC_SYSQIC_IB_DFX_WCT_TIMEOUT3_ADDR(base) ((base) + (0x076CUL))
#define SOC_SYSQIC_IB_DFX_RCT_LOCK0_ADDR(base) ((base) + (0x0770UL))
#define SOC_SYSQIC_IB_DFX_RCT_LOCK1_ADDR(base) ((base) + (0x0774UL))
#define SOC_SYSQIC_IB_DFX_RCT_LOCK2_ADDR(base) ((base) + (0x0778UL))
#define SOC_SYSQIC_IB_DFX_RCT_LOCK3_ADDR(base) ((base) + (0x077CUL))
#define SOC_SYSQIC_IB_DFX_WCT_LOCK0_ADDR(base) ((base) + (0x0780UL))
#define SOC_SYSQIC_IB_DFX_WCT_LOCK1_ADDR(base) ((base) + (0x0784UL))
#define SOC_SYSQIC_IB_DFX_WCT_LOCK2_ADDR(base) ((base) + (0x0788UL))
#define SOC_SYSQIC_IB_DFX_WCT_LOCK3_ADDR(base) ((base) + (0x078CUL))
#define SOC_SYSQIC_IB_DFX_ECC_ADDR(base) ((base) + (0x0790UL))
#define SOC_SYSQIC_IB_DFX_FIFO0_ADDR(base) ((base) + (0x0794UL))
#define SOC_SYSQIC_IB_DFX_EXT_CTRL_ADDR(base) ((base) + (0x0798UL))
#else
#define SOC_SYSQIC_IB_RTL_VER_ADDR(base) ((base) + (0x0000))
#define SOC_SYSQIC_IB_RTL_INFO_ADDR(base) ((base) + (0x0004))
#define SOC_SYSQIC_IB_RTL_INF1_ADDR(base) ((base) + (0x0008))
#define SOC_SYSQIC_IB_RTL_INF2_ADDR(base) ((base) + (0x000C))
#define SOC_SYSQIC_IB_CKG_CTRL_ADDR(base) ((base) + (0x0100))
#define SOC_SYSQIC_IB_RAM_TMOD_ADDR(base) ((base) + (0x0104))
#define SOC_SYSQIC_IB_RSV_RW_ADDR(base) ((base) + (0x010C))
#define SOC_SYSQIC_IB_CS_CTRL_ADDR(base) ((base) + (0x0110))
#define SOC_SYSQIC_IB_WA_CTRL_ADDR(base) ((base) + (0x0114))
#define SOC_SYSQIC_IB_CR_CTRL_ADDR(base) ((base) + (0x0118))
#define SOC_SYSQIC_IB_CT_CTRL_ADDR(base) ((base) + (0x011C))
#define SOC_SYSQIC_IB_TIMEOUT_CTRL_ADDR(base) ((base) + (0x0130))
#define SOC_SYSQIC_IB_ECC_CTRL_ADDR(base) ((base) + (0x0134))
#define SOC_SYSQIC_IB_PARITY_CTRL_ADDR(base) ((base) + (0x0138))
#define SOC_SYSQIC_IB_QOS_LMTR0_CTRL_ADDR(base) ((base) + (0x0140))
#define SOC_SYSQIC_IB_QOS_LMTR1_CTRL_ADDR(base) ((base) + (0x0144))
#define SOC_SYSQIC_IB_QOS_LMTR2_CTRL_ADDR(base) ((base) + (0x0148))
#define SOC_SYSQIC_IB_QOS_LMTR3_CTRL_ADDR(base) ((base) + (0x014C))
#define SOC_SYSQIC_IB_QOS_OSTD0_CTRL_ADDR(base) ((base) + (0x0150))
#define SOC_SYSQIC_IB_QOS_OSTD1_CTRL_ADDR(base) ((base) + (0x0154))
#define SOC_SYSQIC_IB_QOS_OSTD2_CTRL_ADDR(base) ((base) + (0x0158))
#define SOC_SYSQIC_IB_QOS_OSTD3_CTRL_ADDR(base) ((base) + (0x015C))
#define SOC_SYSQIC_IB_QOS_RDQOS_CTRL_ADDR(base) ((base) + (0x0160))
#define SOC_SYSQIC_IB_QOS_WRQOS_CTRL_ADDR(base) ((base) + (0x0164))
#define SOC_SYSQIC_IB_QOS_RDPRI_CTRL_ADDR(base) ((base) + (0x0168))
#define SOC_SYSQIC_IB_QOS_WRPRI_CTRL_ADDR(base) ((base) + (0x016C))
#define SOC_SYSQIC_IB_QOS_PUSH_CTRL_ADDR(base) ((base) + (0x0170))
#define SOC_SYSQIC_IB_QOS_LMTR_LITE_CTRL_ADDR(base) ((base) + (0x0174))
#define SOC_SYSQIC_IB_QOS_RGLR0_CTRL_ADDR(base) ((base) + (0x0180))
#define SOC_SYSQIC_IB_QOS_RGLR1_CTRL_ADDR(base) ((base) + (0x0184))
#define SOC_SYSQIC_IB_PROBE_WREQ_CFG0_ADDR(base) ((base) + (0x0200))
#define SOC_SYSQIC_IB_PROBE_WREQ_CFG1_ADDR(base) ((base) + (0x0204))
#define SOC_SYSQIC_IB_PROBE_WREQ_FILTER0_ADDR(base) ((base) + (0x0210))
#define SOC_SYSQIC_IB_PROBE_WREQ_FILTER1_ADDR(base) ((base) + (0x0214))
#define SOC_SYSQIC_IB_PROBE_WREQ_FILTER2_ADDR(base) ((base) + (0x0218))
#define SOC_SYSQIC_IB_PROBE_WREQ_FILTER3_ADDR(base) ((base) + (0x021C))
#define SOC_SYSQIC_IB_PROBE_WREQ_FILTER4_0_ADDR(base) ((base) + (0x0220))
#define SOC_SYSQIC_IB_PROBE_WREQ_FILTER4_1_ADDR(base) ((base) + (0x0224))
#define SOC_SYSQIC_IB_PROBE_WREQ_FILTER4_2_ADDR(base) ((base) + (0x0228))
#define SOC_SYSQIC_IB_PROBE_WREQ_FILTER4_3_ADDR(base) ((base) + (0x022C))
#define SOC_SYSQIC_IB_PROBE_WREQ_FILTER5_0_ADDR(base) ((base) + (0x0230))
#define SOC_SYSQIC_IB_PROBE_WREQ_FILTER5_1_ADDR(base) ((base) + (0x0234))
#define SOC_SYSQIC_IB_PROBE_WREQ_FILTER5_2_ADDR(base) ((base) + (0x0238))
#define SOC_SYSQIC_IB_PROBE_WREQ_FILTER5_3_ADDR(base) ((base) + (0x023C))
#define SOC_SYSQIC_IB_PROBE_WREQ_FILTER6_ADDR(base) ((base) + (0x0240))
#define SOC_SYSQIC_IB_PROBE_WREQ_FILTER7_ADDR(base) ((base) + (0x0244))
#define SOC_SYSQIC_IB_PROBE_WREQ_FILTER8_ADDR(base) ((base) + (0x0248))
#define SOC_SYSQIC_IB_PROBE_WREQ_FILTER9_ADDR(base) ((base) + (0x024C))
#define SOC_SYSQIC_IB_PROBE_WREQ_INT_STAUS_ADDR(base) ((base) + (0x0250))
#define SOC_SYSQIC_IB_PROBE_WREQ_INT_CLR_ADDR(base) ((base) + (0x0254))
#define SOC_SYSQIC_IB_PROBE_WREQ_CNT_ADDR(base,cnt_reg_num) ((base) + (0x0280+0x4*(cnt_reg_num)))
#define SOC_SYSQIC_IB_PROBE_WREQ_INT_CFG_ADDR(base,cnt_reg_num) ((base) + (0x02C0+0x4*(cnt_reg_num)))
#define SOC_SYSQIC_IB_PROBE_RREQ_CFG0_ADDR(base) ((base) + (0x0300))
#define SOC_SYSQIC_IB_PROBE_RREQ_CFG1_ADDR(base) ((base) + (0x0304))
#define SOC_SYSQIC_IB_PROBE_RREQ_FILTER0_ADDR(base) ((base) + (0x0310))
#define SOC_SYSQIC_IB_PROBE_RREQ_FILTER1_ADDR(base) ((base) + (0x0314))
#define SOC_SYSQIC_IB_PROBE_RREQ_FILTER2_ADDR(base) ((base) + (0x0318))
#define SOC_SYSQIC_IB_PROBE_RREQ_FILTER3_ADDR(base) ((base) + (0x031C))
#define SOC_SYSQIC_IB_PROBE_RREQ_FILTER4_0_ADDR(base) ((base) + (0x0320))
#define SOC_SYSQIC_IB_PROBE_RREQ_FILTER4_1_ADDR(base) ((base) + (0x0324))
#define SOC_SYSQIC_IB_PROBE_RREQ_FILTER4_2_ADDR(base) ((base) + (0x0328))
#define SOC_SYSQIC_IB_PROBE_RREQ_FILTER4_3_ADDR(base) ((base) + (0x032C))
#define SOC_SYSQIC_IB_PROBE_RREQ_FILTER5_0_ADDR(base) ((base) + (0x0330))
#define SOC_SYSQIC_IB_PROBE_RREQ_FILTER5_1_ADDR(base) ((base) + (0x0334))
#define SOC_SYSQIC_IB_PROBE_RREQ_FILTER5_2_ADDR(base) ((base) + (0x0338))
#define SOC_SYSQIC_IB_PROBE_RREQ_FILTER5_3_ADDR(base) ((base) + (0x033C))
#define SOC_SYSQIC_IB_PROBE_RREQ_FILTER6_ADDR(base) ((base) + (0x0340))
#define SOC_SYSQIC_IB_PROBE_RREQ_FILTER7_ADDR(base) ((base) + (0x0344))
#define SOC_SYSQIC_IB_PROBE_RREQ_FILTER8_ADDR(base) ((base) + (0x0348))
#define SOC_SYSQIC_IB_PROBE_RREQ_FILTER9_ADDR(base) ((base) + (0x034C))
#define SOC_SYSQIC_IB_PROBE_RREQ_INT_STATUS_ADDR(base) ((base) + (0x0350))
#define SOC_SYSQIC_IB_PROBE_RREQ_INT_CLR_ADDR(base) ((base) + (0x0354))
#define SOC_SYSQIC_IB_PROBE_RREQ_CNT_ADDR(base,cnt_reg_num) ((base) + (0x0380+0x4*(cnt_reg_num)))
#define SOC_SYSQIC_IB_PROBE_RREQ_INT_CFG_ADDR(base,cnt_reg_num) ((base) + (0x03C0+0x4*(cnt_reg_num)))
#define SOC_SYSQIC_IB_DFX_INT_CTRL0_ADDR(base) ((base) + (0x0600))
#define SOC_SYSQIC_IB_DFX_INT_CTRL1_ADDR(base) ((base) + (0x0604))
#define SOC_SYSQIC_IB_DFX_INT_STATUS_ADDR(base) ((base) + (0x0608))
#define SOC_SYSQIC_IB_DFX_INT_CLR_ADDR(base) ((base) + (0x060C))
#define SOC_SYSQIC_IB_DFX_ERR_INF0_ADDR(base) ((base) + (0x0610))
#define SOC_SYSQIC_IB_DFX_ERR_INF1_ADDR(base) ((base) + (0x0614))
#define SOC_SYSQIC_IB_DFX_ERR_INF2_ADDR(base) ((base) + (0x0618))
#define SOC_SYSQIC_IB_DFX_ERR_INF3_ADDR(base) ((base) + (0x061C))
#define SOC_SYSQIC_IB_DFX_ORG_INT0_ADDR(base) ((base) + (0x0620))
#define SOC_SYSQIC_IB_DFX_ORG_INT1_ADDR(base) ((base) + (0x0624))
#define SOC_SYSQIC_IB_DFX_SAFETY_ORG_INT_ADDR(base) ((base) + (0x0628))
#define SOC_SYSQIC_IB_DFX_SAFETY_INF0_ADDR(base) ((base) + (0x0640))
#define SOC_SYSQIC_IB_DFX_SAFETY_INF1_ADDR(base) ((base) + (0x0644))
#define SOC_SYSQIC_IB_DFX_SAFETY_INF2_ADDR(base) ((base) + (0x0648))
#define SOC_SYSQIC_IB_DFX_SAFETY_INF3_ADDR(base) ((base) + (0x064C))
#define SOC_SYSQIC_IB_DFX_TABLE_CTRL_ADDR(base) ((base) + (0x0680))
#define SOC_SYSQIC_IB_DFX_TABLE_INF0_ADDR(base) ((base) + (0x0690))
#define SOC_SYSQIC_IB_DFX_TABLE_INF1_ADDR(base) ((base) + (0x0694))
#define SOC_SYSQIC_IB_DFX_TABLE_INF2_ADDR(base) ((base) + (0x0698))
#define SOC_SYSQIC_IB_DFX_TABLE_INF3_ADDR(base) ((base) + (0x069C))
#define SOC_SYSQIC_IB_SOFT_LPC_CTRL_ADDR(base) ((base) + (0x06C0))
#define SOC_SYSQIC_IB_DFX_MODULE_ADDR(base) ((base) + (0x0700))
#define SOC_SYSQIC_IB_DFX_BP_ADDR(base) ((base) + (0x0704))
#define SOC_SYSQIC_IB_DFX_OT_ADDR(base) ((base) + (0x0708))
#define SOC_SYSQIC_IB_DFX_BUF0_ADDR(base) ((base) + (0x0710))
#define SOC_SYSQIC_IB_DFX_BUF1_ADDR(base) ((base) + (0x0714))
#define SOC_SYSQIC_IB_DFX_LPC_ADDR(base) ((base) + (0x0718))
#define SOC_SYSQIC_IB_DFX_RREQ_TX0_ADDR(base) ((base) + (0x0720))
#define SOC_SYSQIC_IB_DFX_RREQ_TX1_ADDR(base) ((base) + (0x0724))
#define SOC_SYSQIC_IB_DFX_RREQ_TX2_ADDR(base) ((base) + (0x0728))
#define SOC_SYSQIC_IB_DFX_RREQ_TX3_ADDR(base) ((base) + (0x072C))
#define SOC_SYSQIC_IB_DFX_WREQ_TX0_ADDR(base) ((base) + (0x0730))
#define SOC_SYSQIC_IB_DFX_WREQ_TX1_ADDR(base) ((base) + (0x0734))
#define SOC_SYSQIC_IB_DFX_WREQ_TX2_ADDR(base) ((base) + (0x0738))
#define SOC_SYSQIC_IB_DFX_WREQ_TX3_ADDR(base) ((base) + (0x073C))
#define SOC_SYSQIC_IB_DFX_RSP_RX_ADDR(base) ((base) + (0x0740))
#define SOC_SYSQIC_IB_DFX_CFG_PORT_ADDR(base) ((base) + (0x0744))
#define SOC_SYSQIC_IB_DFX_RCT_TIMEOUT0_ADDR(base) ((base) + (0x0750))
#define SOC_SYSQIC_IB_DFX_RCT_TIMEOUT1_ADDR(base) ((base) + (0x0754))
#define SOC_SYSQIC_IB_DFX_RCT_TIMEOUT2_ADDR(base) ((base) + (0x0758))
#define SOC_SYSQIC_IB_DFX_RCT_TIMEOUT3_ADDR(base) ((base) + (0x075C))
#define SOC_SYSQIC_IB_DFX_WCT_TIMEOUT0_ADDR(base) ((base) + (0x0760))
#define SOC_SYSQIC_IB_DFX_WCT_TIMEOUT1_ADDR(base) ((base) + (0x0764))
#define SOC_SYSQIC_IB_DFX_WCT_TIMEOUT2_ADDR(base) ((base) + (0x0768))
#define SOC_SYSQIC_IB_DFX_WCT_TIMEOUT3_ADDR(base) ((base) + (0x076C))
#define SOC_SYSQIC_IB_DFX_RCT_LOCK0_ADDR(base) ((base) + (0x0770))
#define SOC_SYSQIC_IB_DFX_RCT_LOCK1_ADDR(base) ((base) + (0x0774))
#define SOC_SYSQIC_IB_DFX_RCT_LOCK2_ADDR(base) ((base) + (0x0778))
#define SOC_SYSQIC_IB_DFX_RCT_LOCK3_ADDR(base) ((base) + (0x077C))
#define SOC_SYSQIC_IB_DFX_WCT_LOCK0_ADDR(base) ((base) + (0x0780))
#define SOC_SYSQIC_IB_DFX_WCT_LOCK1_ADDR(base) ((base) + (0x0784))
#define SOC_SYSQIC_IB_DFX_WCT_LOCK2_ADDR(base) ((base) + (0x0788))
#define SOC_SYSQIC_IB_DFX_WCT_LOCK3_ADDR(base) ((base) + (0x078C))
#define SOC_SYSQIC_IB_DFX_ECC_ADDR(base) ((base) + (0x0790))
#define SOC_SYSQIC_IB_DFX_FIFO0_ADDR(base) ((base) + (0x0794))
#define SOC_SYSQIC_IB_DFX_EXT_CTRL_ADDR(base) ((base) + (0x0798))
#endif
#ifndef __SOC_H_FOR_ASM__
#define SOC_SYSQIC_RT_INFO_ADDR(base) ((base) + (0x0000UL))
#define SOC_SYSQIC_RT_RESERVED_RW_ADDR(base) ((base) + (0x0004UL))
#define SOC_SYSQIC_RT_RESERVED_RO_ADDR(base) ((base) + (0x0008UL))
#define SOC_SYSQIC_RT_CKG_BYPS_ADDR(base) ((base) + (0x000CUL))
#define SOC_SYSQIC_RT_GLB_ST_ADDR(base) ((base) + (0x0010UL))
#define SOC_SYSQIC_RT_GLB_INT_ST_ADDR(base) ((base) + (0x0014UL))
#define SOC_SYSQIC_RT_TIMEOUT_CTRL_ADDR(base) ((base) + (0x0020UL))
#define SOC_SYSQIC_RT_MAX_LAT_CFG_ADDR(base) ((base) + (0x0024UL))
#define SOC_SYSQIC_RT_MAX_LAT_ST_ADDR(base) ((base) + (0x0028UL))
#define SOC_SYSQIC_RT_LPC_CTRL_ADDR(base) ((base) + (0x002CUL))
#define SOC_SYSQIC_RT_LPC_ST_ADDR(base) ((base) + (0x0030UL))
#define SOC_SYSQIC_RT_FUC_BYPS_ADDR(base) ((base) + (0x0034UL))
#define SOC_SYSQIC_RT_INPORT_VC_CTRL_ADDR(base,inp_idx) ((base) + (0x0100+(inp_idx)*0x0040UL))
#define SOC_SYSQIC_RT_INPORT_DFX_ST0_ADDR(base,inp_idx) ((base) + (0x0104+(inp_idx)*0x0040UL))
#define SOC_SYSQIC_RT_INPORT_DFX_ST1_ADDR(base,inp_idx) ((base) + (0x0108+(inp_idx)*0x0040UL))
#define SOC_SYSQIC_RT_INPORT_DFX_ST2_ADDR(base,inp_idx) ((base) + (0x010C+(inp_idx)*0x0040UL))
#define SOC_SYSQIC_RT_INPORT_DFX_ST3_ADDR(base,inp_idx) ((base) + (0x0110+(inp_idx)*0x0040UL))
#define SOC_SYSQIC_RT_INPORT_INT_ST_ADDR(base,inp_idx) ((base) + (0x0114+(inp_idx)*0x0040UL))
#define SOC_SYSQIC_RT_INPORT_INT_INI_ADDR(base,inp_idx) ((base) + (0x0118+(inp_idx)*0x0040UL))
#define SOC_SYSQIC_RT_INPORT_INT_CLEAR_ADDR(base,inp_idx) ((base) + (0x011C+(inp_idx)*0x0040UL))
#define SOC_SYSQIC_RT_INPORT_INT_EN_ADDR(base,inp_idx) ((base) + (0x0120+(inp_idx)*0x0040UL))
#define SOC_SYSQIC_RT_INPORT_DFX_ST4_ADDR(base,inp_idx) ((base) + (0x0124+(inp_idx)*0x0040UL))
#define SOC_SYSQIC_RT_INPORT_INT_SNP_ADDR(base,inp_idx) ((base) + (0x0128+(inp_idx)*0x0040UL))
#define SOC_SYSQIC_RT_OUTPORT_CFC_CTRL_ADDR(base,outp_idx) ((base) + (0x0400+(outp_idx)*0x0040UL))
#define SOC_SYSQIC_RT_OUTPORT_DFX_ST0_ADDR(base,outp_idx) ((base) + (0x0404+(outp_idx)*0x0040UL))
#define SOC_SYSQIC_RT_OUTPORT_DFX_ST1_ADDR(base,outp_idx) ((base) + (0x0408+(outp_idx)*0x0040UL))
#else
#define SOC_SYSQIC_RT_INFO_ADDR(base) ((base) + (0x0000))
#define SOC_SYSQIC_RT_RESERVED_RW_ADDR(base) ((base) + (0x0004))
#define SOC_SYSQIC_RT_RESERVED_RO_ADDR(base) ((base) + (0x0008))
#define SOC_SYSQIC_RT_CKG_BYPS_ADDR(base) ((base) + (0x000C))
#define SOC_SYSQIC_RT_GLB_ST_ADDR(base) ((base) + (0x0010))
#define SOC_SYSQIC_RT_GLB_INT_ST_ADDR(base) ((base) + (0x0014))
#define SOC_SYSQIC_RT_TIMEOUT_CTRL_ADDR(base) ((base) + (0x0020))
#define SOC_SYSQIC_RT_MAX_LAT_CFG_ADDR(base) ((base) + (0x0024))
#define SOC_SYSQIC_RT_MAX_LAT_ST_ADDR(base) ((base) + (0x0028))
#define SOC_SYSQIC_RT_LPC_CTRL_ADDR(base) ((base) + (0x002C))
#define SOC_SYSQIC_RT_LPC_ST_ADDR(base) ((base) + (0x0030))
#define SOC_SYSQIC_RT_FUC_BYPS_ADDR(base) ((base) + (0x0034))
#define SOC_SYSQIC_RT_INPORT_VC_CTRL_ADDR(base,inp_idx) ((base) + (0x0100+(inp_idx)*0x0040))
#define SOC_SYSQIC_RT_INPORT_DFX_ST0_ADDR(base,inp_idx) ((base) + (0x0104+(inp_idx)*0x0040))
#define SOC_SYSQIC_RT_INPORT_DFX_ST1_ADDR(base,inp_idx) ((base) + (0x0108+(inp_idx)*0x0040))
#define SOC_SYSQIC_RT_INPORT_DFX_ST2_ADDR(base,inp_idx) ((base) + (0x010C+(inp_idx)*0x0040))
#define SOC_SYSQIC_RT_INPORT_DFX_ST3_ADDR(base,inp_idx) ((base) + (0x0110+(inp_idx)*0x0040))
#define SOC_SYSQIC_RT_INPORT_INT_ST_ADDR(base,inp_idx) ((base) + (0x0114+(inp_idx)*0x0040))
#define SOC_SYSQIC_RT_INPORT_INT_INI_ADDR(base,inp_idx) ((base) + (0x0118+(inp_idx)*0x0040))
#define SOC_SYSQIC_RT_INPORT_INT_CLEAR_ADDR(base,inp_idx) ((base) + (0x011C+(inp_idx)*0x0040))
#define SOC_SYSQIC_RT_INPORT_INT_EN_ADDR(base,inp_idx) ((base) + (0x0120+(inp_idx)*0x0040))
#define SOC_SYSQIC_RT_INPORT_DFX_ST4_ADDR(base,inp_idx) ((base) + (0x0124+(inp_idx)*0x0040))
#define SOC_SYSQIC_RT_INPORT_INT_SNP_ADDR(base,inp_idx) ((base) + (0x0128+(inp_idx)*0x0040))
#define SOC_SYSQIC_RT_OUTPORT_CFC_CTRL_ADDR(base,outp_idx) ((base) + (0x0400+(outp_idx)*0x0040))
#define SOC_SYSQIC_RT_OUTPORT_DFX_ST0_ADDR(base,outp_idx) ((base) + (0x0404+(outp_idx)*0x0040))
#define SOC_SYSQIC_RT_OUTPORT_DFX_ST1_ADDR(base,outp_idx) ((base) + (0x0408+(outp_idx)*0x0040))
#endif
#ifndef __SOC_H_FOR_ASM__
#define SOC_SYSQIC_WA_INFO_ADDR(base) ((base) + (0x0000UL))
#define SOC_SYSQIC_WA_RESERVED_RW_ADDR(base) ((base) + (0x0004UL))
#define SOC_SYSQIC_WA_RESERVED_RO_ADDR(base) ((base) + (0x0008UL))
#define SOC_SYSQIC_WA_CKG_BYPS_ADDR(base) ((base) + (0x000CUL))
#define SOC_SYSQIC_WA_GLB_CTRL_ADDR(base) ((base) + (0x0030UL))
#define SOC_SYSQIC_WA_ST_ADDR(base) ((base) + (0x0010UL))
#define SOC_SYSQIC_WA_INI_ST_ADDR(base) ((base) + (0x0014UL))
#define SOC_SYSQIC_WA_INT_INI_ADDR(base) ((base) + (0x0018UL))
#define SOC_SYSQIC_WA_INT_CLEAR_ADDR(base) ((base) + (0x001CUL))
#define SOC_SYSQIC_WA_INT_EN_ADDR(base) ((base) + (0x0020UL))
#define SOC_SYSQIC_WA_MAX_LAT_CFG_ADDR(base) ((base) + (0x0024UL))
#define SOC_SYSQIC_WA_MAX_LAT_ST_ADDR(base) ((base) + (0x0028UL))
#define SOC_SYSQIC_WA_LPC_CTRL_ADDR(base) ((base) + (0x002CUL))
#define SOC_SYSQIC_WA_INT_SNAP_ADDR(base) ((base) + (0x0038UL))
#define SOC_SYSQIC_WA_LPC_ST_ADDR(base) ((base) + (0x0034UL))
#define SOC_SYSQIC_WA_SFW_CTRL_ADDR(base) ((base) + (0x0100UL))
#define SOC_SYSQIC_WA_INPORT_DFX_ST0_ADDR(base) ((base) + (0x0104UL))
#define SOC_SYSQIC_WA_OUTPORT_DFX_ST0_ADDR(base) ((base) + (0x0404UL))
#define SOC_SYSQIC_WA_OUTPORT_DFX_ST1_ADDR(base) ((base) + (0x0408UL))
#else
#define SOC_SYSQIC_WA_INFO_ADDR(base) ((base) + (0x0000))
#define SOC_SYSQIC_WA_RESERVED_RW_ADDR(base) ((base) + (0x0004))
#define SOC_SYSQIC_WA_RESERVED_RO_ADDR(base) ((base) + (0x0008))
#define SOC_SYSQIC_WA_CKG_BYPS_ADDR(base) ((base) + (0x000C))
#define SOC_SYSQIC_WA_GLB_CTRL_ADDR(base) ((base) + (0x0030))
#define SOC_SYSQIC_WA_ST_ADDR(base) ((base) + (0x0010))
#define SOC_SYSQIC_WA_INI_ST_ADDR(base) ((base) + (0x0014))
#define SOC_SYSQIC_WA_INT_INI_ADDR(base) ((base) + (0x0018))
#define SOC_SYSQIC_WA_INT_CLEAR_ADDR(base) ((base) + (0x001C))
#define SOC_SYSQIC_WA_INT_EN_ADDR(base) ((base) + (0x0020))
#define SOC_SYSQIC_WA_MAX_LAT_CFG_ADDR(base) ((base) + (0x0024))
#define SOC_SYSQIC_WA_MAX_LAT_ST_ADDR(base) ((base) + (0x0028))
#define SOC_SYSQIC_WA_LPC_CTRL_ADDR(base) ((base) + (0x002C))
#define SOC_SYSQIC_WA_INT_SNAP_ADDR(base) ((base) + (0x0038))
#define SOC_SYSQIC_WA_LPC_ST_ADDR(base) ((base) + (0x0034))
#define SOC_SYSQIC_WA_SFW_CTRL_ADDR(base) ((base) + (0x0100))
#define SOC_SYSQIC_WA_INPORT_DFX_ST0_ADDR(base) ((base) + (0x0104))
#define SOC_SYSQIC_WA_OUTPORT_DFX_ST0_ADDR(base) ((base) + (0x0404))
#define SOC_SYSQIC_WA_OUTPORT_DFX_ST1_ADDR(base) ((base) + (0x0408))
#endif
#ifndef __SOC_H_FOR_ASM__
#define SOC_SYSQIC_TB_RTL_VER_ADDR(base) ((base) + (0x0000UL))
#define SOC_SYSQIC_TB_RTL_INFO_ADDR(base) ((base) + (0x0004UL))
#define SOC_SYSQIC_TB_RTL_INF1_ADDR(base) ((base) + (0x0008UL))
#define SOC_SYSQIC_TB_RTL_INF2_ADDR(base) ((base) + (0x000CUL))
#define SOC_SYSQIC_TB_CKG_BYP_ADDR(base) ((base) + (0x0100UL))
#define SOC_SYSQIC_TB_PARITY_CTRL_ADDR(base) ((base) + (0x0104UL))
#define SOC_SYSQIC_TB_VC_CTRL_ADDR(base,vc_num) ((base) + (0x0110+0x4*(vc_num)))
#define SOC_SYSQIC_TB_QOS_CFC0_ADDR(base,vc_num) ((base) + (0x0200+0x4*(vc_num)))
#define SOC_SYSQIC_TB_QOS_CFC1_ADDR(base,vc_num) ((base) + (0x0220+0x4*(vc_num)))
#define SOC_SYSQIC_TB_QOS_CFC2_ADDR(base,vc_num) ((base) + (0x0240+0x4*(vc_num)))
#define SOC_SYSQIC_TB_QOS_CFC3_ADDR(base,vc_num) ((base) + (0x0260+0x4*(vc_num)))
#define SOC_SYSQIC_TB_QOS_CFC4_ADDR(base,vc_num) ((base) + (0x0280+0x4*(vc_num)))
#define SOC_SYSQIC_TB_QOS_CFC5_ADDR(base,vc_num) ((base) + (0x0300+0x4*(vc_num)))
#define SOC_SYSQIC_TB_QOS_PRESS_ADDR(base) ((base) + (0x0320UL))
#define SOC_SYSQIC_TB_INT_EN_ADDR(base) ((base) + (0x0400UL))
#define SOC_SYSQIC_TB_INT_CLR_ADDR(base) ((base) + (0x0404UL))
#define SOC_SYSQIC_TB_INT_STATUS_ADDR(base) ((base) + (0x0408UL))
#define SOC_SYSQIC_TB_INT_TYPE_ADDR(base) ((base) + (0x040cUL))
#define SOC_SYSQIC_TB_INT_WREQ_INF0_ADDR(base,info_num) ((base) + (0x0410+0x4*(info_num)))
#define SOC_SYSQIC_TB_INT_WREQ_INF1_ADDR(base,info_num) ((base) + (0x040+0x4*(info_num)))
#define SOC_SYSQIC_TB_INT_RREQ_INF0_ADDR(base,info_num) ((base) + (0x0470+0x4*(info_num)))
#define SOC_SYSQIC_TB_INT_RREQ_INF1_ADDR(base,info_num) ((base) + (0x04a0+0x4*(info_num)))
#define SOC_SYSQIC_TB_ERR_CMD_NOTE0_ADDR(base) ((base) + (0x0600UL))
#define SOC_SYSQIC_TB_ERR_CMD_MASK_ADDR(base) ((base) + (0x0608UL))
#define SOC_SYSQIC_TB_LPC_CTRL_ADDR(base) ((base) + (0x0610UL))
#define SOC_SYSQIC_TB_CT_CTRL_ADDR(base) ((base) + (0x0614UL))
#define SOC_SYSQIC_DFX_TB_MODULE_ADDR(base) ((base) + (0x0800UL))
#define SOC_SYSQIC_DFX_TB_BP_ADDR(base) ((base) + (0x0804UL))
#define SOC_SYSQIC_DFX_TB_OT_ADDR(base) ((base) + (0x0808UL))
#define SOC_SYSQIC_DFX_TB_AWCT_BUF0_ADDR(base) ((base) + (0x0810UL))
#define SOC_SYSQIC_DFX_TB_AWCT_BUF1_ADDR(base) ((base) + (0x0814UL))
#define SOC_SYSQIC_DFX_TB_ARCT_BUF0_ADDR(base) ((base) + (0x0818UL))
#define SOC_SYSQIC_DFX_TB_ARCT_BUF1_ADDR(base) ((base) + (0x081cUL))
#define SOC_SYSQIC_DFX_TB_ARCT_BUF2_ADDR(base) ((base) + (0x0820UL))
#define SOC_SYSQIC_DFX_TB_ARCT_BUF3_ADDR(base) ((base) + (0x0824UL))
#define SOC_SYSQIC_DFX_TB_DATA_BUF_ADDR(base) ((base) + (0x0828UL))
#define SOC_SYSQIC_DFX_TB_FIFO_ADDR(base) ((base) + (0x0830UL))
#define SOC_SYSQIC_DFX_TB_TX_CRDT_ADDR(base) ((base) + (0x0834UL))
#define SOC_SYSQIC_DFX_TB_OBSERVE_CTRL_ADDR(base) ((base) + (0x0840UL))
#define SOC_SYSQIC_DFX_TB_OBSERVE_INFO_ADDR(base,info_num) ((base) + (0x0850+0x4*(info_num)))
#define SOC_SYSQIC_DFX_TB_CFC_OSTD_NUM_ADDR(base,vc_num) ((base) + (0x0880+0x4*(vc_num)))
#define SOC_SYSQIC_DFX_TB_LPC_ST_ADDR(base) ((base) + (0x08b0UL))
#define SOC_SYSQIC_DFX_TB_ARB_ST_ADDR(base) ((base) + (0x08b4UL))
#define SOC_SYSQIC_DFX_TB_ID_ST0_ADDR(base) ((base) + (0x08b8UL))
#define SOC_SYSQIC_DFX_TB_ID_ST1_ADDR(base) ((base) + (0x08bcUL))
#define SOC_SYSQIC_DFX_TB_POISON_MSK_ADDR(base) ((base) + (0x08c0UL))
#define SOC_SYSQIC_DFX_TB_POISON_CLR_ADDR(base) ((base) + (0x08c4UL))
#define SOC_SYSQIC_DFX_TB_POISON_ST_ADDR(base) ((base) + (0x08c8UL))
#define SOC_SYSQIC_TB_RESERVED_RW_ADDR(base) ((base) + (0x0900UL))
#define SOC_SYSQIC_TB_RESERVED_RO_ADDR(base) ((base) + (0x0904UL))
#else
#define SOC_SYSQIC_TB_RTL_VER_ADDR(base) ((base) + (0x0000))
#define SOC_SYSQIC_TB_RTL_INFO_ADDR(base) ((base) + (0x0004))
#define SOC_SYSQIC_TB_RTL_INF1_ADDR(base) ((base) + (0x0008))
#define SOC_SYSQIC_TB_RTL_INF2_ADDR(base) ((base) + (0x000C))
#define SOC_SYSQIC_TB_CKG_BYP_ADDR(base) ((base) + (0x0100))
#define SOC_SYSQIC_TB_PARITY_CTRL_ADDR(base) ((base) + (0x0104))
#define SOC_SYSQIC_TB_VC_CTRL_ADDR(base,vc_num) ((base) + (0x0110+0x4*(vc_num)))
#define SOC_SYSQIC_TB_QOS_CFC0_ADDR(base,vc_num) ((base) + (0x0200+0x4*(vc_num)))
#define SOC_SYSQIC_TB_QOS_CFC1_ADDR(base,vc_num) ((base) + (0x0220+0x4*(vc_num)))
#define SOC_SYSQIC_TB_QOS_CFC2_ADDR(base,vc_num) ((base) + (0x0240+0x4*(vc_num)))
#define SOC_SYSQIC_TB_QOS_CFC3_ADDR(base,vc_num) ((base) + (0x0260+0x4*(vc_num)))
#define SOC_SYSQIC_TB_QOS_CFC4_ADDR(base,vc_num) ((base) + (0x0280+0x4*(vc_num)))
#define SOC_SYSQIC_TB_QOS_CFC5_ADDR(base,vc_num) ((base) + (0x0300+0x4*(vc_num)))
#define SOC_SYSQIC_TB_QOS_PRESS_ADDR(base) ((base) + (0x0320))
#define SOC_SYSQIC_TB_INT_EN_ADDR(base) ((base) + (0x0400))
#define SOC_SYSQIC_TB_INT_CLR_ADDR(base) ((base) + (0x0404))
#define SOC_SYSQIC_TB_INT_STATUS_ADDR(base) ((base) + (0x0408))
#define SOC_SYSQIC_TB_INT_TYPE_ADDR(base) ((base) + (0x040c))
#define SOC_SYSQIC_TB_INT_WREQ_INF0_ADDR(base,info_num) ((base) + (0x0410+0x4*(info_num)))
#define SOC_SYSQIC_TB_INT_WREQ_INF1_ADDR(base,info_num) ((base) + (0x040+0x4*(info_num)))
#define SOC_SYSQIC_TB_INT_RREQ_INF0_ADDR(base,info_num) ((base) + (0x0470+0x4*(info_num)))
#define SOC_SYSQIC_TB_INT_RREQ_INF1_ADDR(base,info_num) ((base) + (0x04a0+0x4*(info_num)))
#define SOC_SYSQIC_TB_ERR_CMD_NOTE0_ADDR(base) ((base) + (0x0600))
#define SOC_SYSQIC_TB_ERR_CMD_MASK_ADDR(base) ((base) + (0x0608))
#define SOC_SYSQIC_TB_LPC_CTRL_ADDR(base) ((base) + (0x0610))
#define SOC_SYSQIC_TB_CT_CTRL_ADDR(base) ((base) + (0x0614))
#define SOC_SYSQIC_DFX_TB_MODULE_ADDR(base) ((base) + (0x0800))
#define SOC_SYSQIC_DFX_TB_BP_ADDR(base) ((base) + (0x0804))
#define SOC_SYSQIC_DFX_TB_OT_ADDR(base) ((base) + (0x0808))
#define SOC_SYSQIC_DFX_TB_AWCT_BUF0_ADDR(base) ((base) + (0x0810))
#define SOC_SYSQIC_DFX_TB_AWCT_BUF1_ADDR(base) ((base) + (0x0814))
#define SOC_SYSQIC_DFX_TB_ARCT_BUF0_ADDR(base) ((base) + (0x0818))
#define SOC_SYSQIC_DFX_TB_ARCT_BUF1_ADDR(base) ((base) + (0x081c))
#define SOC_SYSQIC_DFX_TB_ARCT_BUF2_ADDR(base) ((base) + (0x0820))
#define SOC_SYSQIC_DFX_TB_ARCT_BUF3_ADDR(base) ((base) + (0x0824))
#define SOC_SYSQIC_DFX_TB_DATA_BUF_ADDR(base) ((base) + (0x0828))
#define SOC_SYSQIC_DFX_TB_FIFO_ADDR(base) ((base) + (0x0830))
#define SOC_SYSQIC_DFX_TB_TX_CRDT_ADDR(base) ((base) + (0x0834))
#define SOC_SYSQIC_DFX_TB_OBSERVE_CTRL_ADDR(base) ((base) + (0x0840))
#define SOC_SYSQIC_DFX_TB_OBSERVE_INFO_ADDR(base,info_num) ((base) + (0x0850+0x4*(info_num)))
#define SOC_SYSQIC_DFX_TB_CFC_OSTD_NUM_ADDR(base,vc_num) ((base) + (0x0880+0x4*(vc_num)))
#define SOC_SYSQIC_DFX_TB_LPC_ST_ADDR(base) ((base) + (0x08b0))
#define SOC_SYSQIC_DFX_TB_ARB_ST_ADDR(base) ((base) + (0x08b4))
#define SOC_SYSQIC_DFX_TB_ID_ST0_ADDR(base) ((base) + (0x08b8))
#define SOC_SYSQIC_DFX_TB_ID_ST1_ADDR(base) ((base) + (0x08bc))
#define SOC_SYSQIC_DFX_TB_POISON_MSK_ADDR(base) ((base) + (0x08c0))
#define SOC_SYSQIC_DFX_TB_POISON_CLR_ADDR(base) ((base) + (0x08c4))
#define SOC_SYSQIC_DFX_TB_POISON_ST_ADDR(base) ((base) + (0x08c8))
#define SOC_SYSQIC_TB_RESERVED_RW_ADDR(base) ((base) + (0x0900))
#define SOC_SYSQIC_TB_RESERVED_RO_ADDR(base) ((base) + (0x0904))
#endif
#ifndef __SOC_H_FOR_ASM__
#define SOC_SYSQIC_TB1_CTRL0_ADDR(base) ((base) + (0x0000UL))
#define SOC_SYSQIC_TB1_FC_IN_STAT0_ADDR(base) ((base) + (0x0004UL))
#define SOC_SYSQIC_TB1_QTP_DEC_STAT0_ADDR(base) ((base) + (0x0008UL))
#define SOC_SYSQIC_TB1_QTP_ENC_STAT0_ADDR(base) ((base) + (0x000CUL))
#define SOC_SYSQIC_TB1_FC_OUT_STAT0_ADDR(base) ((base) + (0x0010UL))
#define SOC_SYSQIC_TB1_GM_STAT0_ADDR(base) ((base) + (0x0014UL))
#define SOC_SYSQIC_TB1_GM_STAT1_ADDR(base) ((base) + (0x0018UL))
#define SOC_SYSQIC_TB1_GM_STAT2_ADDR(base) ((base) + (0x001CUL))
#define SOC_SYSQIC_TB1_GM_STAT3_ADDR(base) ((base) + (0x0020UL))
#define SOC_SYSQIC_TB1_GM_STAT4_ADDR(base) ((base) + (0x0024UL))
#define SOC_SYSQIC_TB1_GM_STAT5_ADDR(base) ((base) + (0x0028UL))
#define SOC_SYSQIC_TB1_DFX_CTRL0_ADDR(base) ((base) + (0x002CUL))
#define SOC_SYSQIC_TB1_DFX_STAT0_ADDR(base) ((base) + (0x0030UL))
#define SOC_SYSQIC_TB1_DFX_STAT1_ADDR(base) ((base) + (0x0034UL))
#define SOC_SYSQIC_TB1_DFX_STAT2_ADDR(base) ((base) + (0x0038UL))
#define SOC_SYSQIC_TB1_DFX_STAT3_ADDR(base) ((base) + (0x003CUL))
#define SOC_SYSQIC_TB1_DFX_STAT4_ADDR(base) ((base) + (0x0040UL))
#define SOC_SYSQIC_TB1_DFX_STAT5_ADDR(base) ((base) + (0x0044UL))
#define SOC_SYSQIC_TB1_CG_CTRL0_ADDR(base) ((base) + (0x0048UL))
#define SOC_SYSQIC_TB1_RESERVED_RW_ADDR(base) ((base) + (0x004CUL))
#define SOC_SYSQIC_TB1_RESERVED_RO_ADDR(base) ((base) + (0x0050UL))
#define SOC_SYSQIC_TB1_INT_EN_ADDR(base) ((base) + (0x0100UL))
#define SOC_SYSQIC_TB1_INT_CLR_ADDR(base) ((base) + (0x0104UL))
#define SOC_SYSQIC_TB1_INT_STATUS_ADDR(base) ((base) + (0x0108UL))
#define SOC_SYSQIC_TB1_CTRL1_ADDR(base) ((base) + (0x0120UL))
#else
#define SOC_SYSQIC_TB1_CTRL0_ADDR(base) ((base) + (0x0000))
#define SOC_SYSQIC_TB1_FC_IN_STAT0_ADDR(base) ((base) + (0x0004))
#define SOC_SYSQIC_TB1_QTP_DEC_STAT0_ADDR(base) ((base) + (0x0008))
#define SOC_SYSQIC_TB1_QTP_ENC_STAT0_ADDR(base) ((base) + (0x000C))
#define SOC_SYSQIC_TB1_FC_OUT_STAT0_ADDR(base) ((base) + (0x0010))
#define SOC_SYSQIC_TB1_GM_STAT0_ADDR(base) ((base) + (0x0014))
#define SOC_SYSQIC_TB1_GM_STAT1_ADDR(base) ((base) + (0x0018))
#define SOC_SYSQIC_TB1_GM_STAT2_ADDR(base) ((base) + (0x001C))
#define SOC_SYSQIC_TB1_GM_STAT3_ADDR(base) ((base) + (0x0020))
#define SOC_SYSQIC_TB1_GM_STAT4_ADDR(base) ((base) + (0x0024))
#define SOC_SYSQIC_TB1_GM_STAT5_ADDR(base) ((base) + (0x0028))
#define SOC_SYSQIC_TB1_DFX_CTRL0_ADDR(base) ((base) + (0x002C))
#define SOC_SYSQIC_TB1_DFX_STAT0_ADDR(base) ((base) + (0x0030))
#define SOC_SYSQIC_TB1_DFX_STAT1_ADDR(base) ((base) + (0x0034))
#define SOC_SYSQIC_TB1_DFX_STAT2_ADDR(base) ((base) + (0x0038))
#define SOC_SYSQIC_TB1_DFX_STAT3_ADDR(base) ((base) + (0x003C))
#define SOC_SYSQIC_TB1_DFX_STAT4_ADDR(base) ((base) + (0x0040))
#define SOC_SYSQIC_TB1_DFX_STAT5_ADDR(base) ((base) + (0x0044))
#define SOC_SYSQIC_TB1_CG_CTRL0_ADDR(base) ((base) + (0x0048))
#define SOC_SYSQIC_TB1_RESERVED_RW_ADDR(base) ((base) + (0x004C))
#define SOC_SYSQIC_TB1_RESERVED_RO_ADDR(base) ((base) + (0x0050))
#define SOC_SYSQIC_TB1_INT_EN_ADDR(base) ((base) + (0x0100))
#define SOC_SYSQIC_TB1_INT_CLR_ADDR(base) ((base) + (0x0104))
#define SOC_SYSQIC_TB1_INT_STATUS_ADDR(base) ((base) + (0x0108))
#define SOC_SYSQIC_TB1_CTRL1_ADDR(base) ((base) + (0x0120))
#endif
#ifndef __SOC_H_FOR_ASM__
#define SOC_SYSQIC_LPC_CTRL0_ADDR(base) ((base) + (0x000UL))
#define SOC_SYSQIC_LPC_CTRL1_ADDR(base) ((base) + (0x004UL))
#define SOC_SYSQIC_LPC_ERR_STAT_ADDR(base) ((base) + (0x008UL))
#define SOC_SYSQIC_LPC_ERRSTAT_CLR_ADDR(base) ((base) + (0x00CUL))
#define SOC_SYSQIC_LPC_STAT1_ADDR(base) ((base) + (0x010UL))
#define SOC_SYSQIC_LPC_STAT2_ADDR(base) ((base) + (0x014UL))
#define SOC_SYSQIC_LPC_STAT3_ADDR(base) ((base) + (0x018UL))
#define SOC_SYSQIC_LPC_STAT4_ADDR(base) ((base) + (0x01CUL))
#define SOC_SYSQIC_LPC_STAT5_ADDR(base) ((base) + (0x020UL))
#define SOC_SYSQIC_LPC_REVISION_ADDR(base) ((base) + (0x024UL))
#define SOC_SYSQIC_LPC_CTRL3_ADDR(base) ((base) + (0x030UL))
#define SOC_SYSQIC_LPC_INT_EN_ADDR(base) ((base) + (0x040UL))
#define SOC_SYSQIC_LPC_INT_CLR_ADDR(base) ((base) + (0x044UL))
#define SOC_SYSQIC_LPC_INT_STATUS_ADDR(base) ((base) + (0x048UL))
#else
#define SOC_SYSQIC_LPC_CTRL0_ADDR(base) ((base) + (0x000))
#define SOC_SYSQIC_LPC_CTRL1_ADDR(base) ((base) + (0x004))
#define SOC_SYSQIC_LPC_ERR_STAT_ADDR(base) ((base) + (0x008))
#define SOC_SYSQIC_LPC_ERRSTAT_CLR_ADDR(base) ((base) + (0x00C))
#define SOC_SYSQIC_LPC_STAT1_ADDR(base) ((base) + (0x010))
#define SOC_SYSQIC_LPC_STAT2_ADDR(base) ((base) + (0x014))
#define SOC_SYSQIC_LPC_STAT3_ADDR(base) ((base) + (0x018))
#define SOC_SYSQIC_LPC_STAT4_ADDR(base) ((base) + (0x01C))
#define SOC_SYSQIC_LPC_STAT5_ADDR(base) ((base) + (0x020))
#define SOC_SYSQIC_LPC_REVISION_ADDR(base) ((base) + (0x024))
#define SOC_SYSQIC_LPC_CTRL3_ADDR(base) ((base) + (0x030))
#define SOC_SYSQIC_LPC_INT_EN_ADDR(base) ((base) + (0x040))
#define SOC_SYSQIC_LPC_INT_CLR_ADDR(base) ((base) + (0x044))
#define SOC_SYSQIC_LPC_INT_STATUS_ADDR(base) ((base) + (0x048))
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int rtl_tag : 8;
        unsigned int reserved_0: 8;
        unsigned int version : 12;
        unsigned int reserved_1: 4;
    } reg;
} SOC_SYSQIC_IB_RTL_VER_UNION;
#endif
#define SOC_SYSQIC_IB_RTL_VER_rtl_tag_START (0)
#define SOC_SYSQIC_IB_RTL_VER_rtl_tag_END (7)
#define SOC_SYSQIC_IB_RTL_VER_version_START (16)
#define SOC_SYSQIC_IB_RTL_VER_version_END (27)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int rtl_info0 : 32;
    } reg;
} SOC_SYSQIC_IB_RTL_INFO_UNION;
#endif
#define SOC_SYSQIC_IB_RTL_INFO_rtl_info0_START (0)
#define SOC_SYSQIC_IB_RTL_INFO_rtl_info0_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int rtl_info1 : 32;
    } reg;
} SOC_SYSQIC_IB_RTL_INF1_UNION;
#endif
#define SOC_SYSQIC_IB_RTL_INF1_rtl_info1_START (0)
#define SOC_SYSQIC_IB_RTL_INF1_rtl_info1_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int rtl_info2 : 32;
    } reg;
} SOC_SYSQIC_IB_RTL_INF2_UNION;
#endif
#define SOC_SYSQIC_IB_RTL_INF2_rtl_info2_START (0)
#define SOC_SYSQIC_IB_RTL_INF2_rtl_info2_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int ckg_byp_ib : 1;
        unsigned int ckg_byp_cfg : 1;
        unsigned int ckg_byp_rcmd : 1;
        unsigned int ckg_byp_wcmd : 1;
        unsigned int ckg_byp_rct : 1;
        unsigned int ckg_byp_wct : 1;
        unsigned int ckg_byp_rrsp : 1;
        unsigned int ckg_byp_wrsp : 1;
        unsigned int ckg_byp_qos : 1;
        unsigned int ckg_byp_cfc : 1;
        unsigned int ckg_byp_ram : 1;
        unsigned int ckg_byp_cfg_cmd : 1;
        unsigned int ckg_byp_misc : 1;
        unsigned int ckg_byp_prob : 1;
        unsigned int ckg_byp_rs : 1;
        unsigned int reserved : 17;
    } reg;
} SOC_SYSQIC_IB_CKG_CTRL_UNION;
#endif
#define SOC_SYSQIC_IB_CKG_CTRL_ckg_byp_ib_START (0)
#define SOC_SYSQIC_IB_CKG_CTRL_ckg_byp_ib_END (0)
#define SOC_SYSQIC_IB_CKG_CTRL_ckg_byp_cfg_START (1)
#define SOC_SYSQIC_IB_CKG_CTRL_ckg_byp_cfg_END (1)
#define SOC_SYSQIC_IB_CKG_CTRL_ckg_byp_rcmd_START (2)
#define SOC_SYSQIC_IB_CKG_CTRL_ckg_byp_rcmd_END (2)
#define SOC_SYSQIC_IB_CKG_CTRL_ckg_byp_wcmd_START (3)
#define SOC_SYSQIC_IB_CKG_CTRL_ckg_byp_wcmd_END (3)
#define SOC_SYSQIC_IB_CKG_CTRL_ckg_byp_rct_START (4)
#define SOC_SYSQIC_IB_CKG_CTRL_ckg_byp_rct_END (4)
#define SOC_SYSQIC_IB_CKG_CTRL_ckg_byp_wct_START (5)
#define SOC_SYSQIC_IB_CKG_CTRL_ckg_byp_wct_END (5)
#define SOC_SYSQIC_IB_CKG_CTRL_ckg_byp_rrsp_START (6)
#define SOC_SYSQIC_IB_CKG_CTRL_ckg_byp_rrsp_END (6)
#define SOC_SYSQIC_IB_CKG_CTRL_ckg_byp_wrsp_START (7)
#define SOC_SYSQIC_IB_CKG_CTRL_ckg_byp_wrsp_END (7)
#define SOC_SYSQIC_IB_CKG_CTRL_ckg_byp_qos_START (8)
#define SOC_SYSQIC_IB_CKG_CTRL_ckg_byp_qos_END (8)
#define SOC_SYSQIC_IB_CKG_CTRL_ckg_byp_cfc_START (9)
#define SOC_SYSQIC_IB_CKG_CTRL_ckg_byp_cfc_END (9)
#define SOC_SYSQIC_IB_CKG_CTRL_ckg_byp_ram_START (10)
#define SOC_SYSQIC_IB_CKG_CTRL_ckg_byp_ram_END (10)
#define SOC_SYSQIC_IB_CKG_CTRL_ckg_byp_cfg_cmd_START (11)
#define SOC_SYSQIC_IB_CKG_CTRL_ckg_byp_cfg_cmd_END (11)
#define SOC_SYSQIC_IB_CKG_CTRL_ckg_byp_misc_START (12)
#define SOC_SYSQIC_IB_CKG_CTRL_ckg_byp_misc_END (12)
#define SOC_SYSQIC_IB_CKG_CTRL_ckg_byp_prob_START (13)
#define SOC_SYSQIC_IB_CKG_CTRL_ckg_byp_prob_END (13)
#define SOC_SYSQIC_IB_CKG_CTRL_ckg_byp_rs_START (14)
#define SOC_SYSQIC_IB_CKG_CTRL_ckg_byp_rs_END (14)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int ram_tmod : 16;
        unsigned int reserved : 16;
    } reg;
} SOC_SYSQIC_IB_RAM_TMOD_UNION;
#endif
#define SOC_SYSQIC_IB_RAM_TMOD_ram_tmod_START (0)
#define SOC_SYSQIC_IB_RAM_TMOD_ram_tmod_END (15)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int rsv_rw : 32;
    } reg;
} SOC_SYSQIC_IB_RSV_RW_UNION;
#endif
#define SOC_SYSQIC_IB_RSV_RW_rsv_rw_START (0)
#define SOC_SYSQIC_IB_RSV_RW_rsv_rw_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int arcs_byp_dis : 1;
        unsigned int awcs_byp_dis : 1;
        unsigned int arcs_narrow_single : 1;
        unsigned int awcs_narrow_single : 1;
        unsigned int reserved : 28;
    } reg;
} SOC_SYSQIC_IB_CS_CTRL_UNION;
#endif
#define SOC_SYSQIC_IB_CS_CTRL_arcs_byp_dis_START (0)
#define SOC_SYSQIC_IB_CS_CTRL_arcs_byp_dis_END (0)
#define SOC_SYSQIC_IB_CS_CTRL_awcs_byp_dis_START (1)
#define SOC_SYSQIC_IB_CS_CTRL_awcs_byp_dis_END (1)
#define SOC_SYSQIC_IB_CS_CTRL_arcs_narrow_single_START (2)
#define SOC_SYSQIC_IB_CS_CTRL_arcs_narrow_single_END (2)
#define SOC_SYSQIC_IB_CS_CTRL_awcs_narrow_single_START (3)
#define SOC_SYSQIC_IB_CS_CTRL_awcs_narrow_single_END (3)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int wa_en : 1;
        unsigned int reserved_0 : 3;
        unsigned int wa_trsfr_wl : 4;
        unsigned int reserved_1 : 24;
    } reg;
} SOC_SYSQIC_IB_WA_CTRL_UNION;
#endif
#define SOC_SYSQIC_IB_WA_CTRL_wa_en_START (0)
#define SOC_SYSQIC_IB_WA_CTRL_wa_en_END (0)
#define SOC_SYSQIC_IB_WA_CTRL_wa_trsfr_wl_START (4)
#define SOC_SYSQIC_IB_WA_CTRL_wa_trsfr_wl_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int sc_en : 1;
        unsigned int reserved_0 : 3;
        unsigned int sc_gid0_byp : 2;
        unsigned int reserved_1 : 26;
    } reg;
} SOC_SYSQIC_IB_CR_CTRL_UNION;
#endif
#define SOC_SYSQIC_IB_CR_CTRL_sc_en_START (0)
#define SOC_SYSQIC_IB_CR_CTRL_sc_en_END (0)
#define SOC_SYSQIC_IB_CR_CTRL_sc_gid0_byp_START (4)
#define SOC_SYSQIC_IB_CR_CTRL_sc_gid0_byp_END (5)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int head_byp_dis : 1;
        unsigned int rlast_arb : 1;
        unsigned int free_disable : 1;
        unsigned int reserved : 13;
        unsigned int rct_ost_wl : 8;
        unsigned int wct_ost_wl : 8;
    } reg;
} SOC_SYSQIC_IB_CT_CTRL_UNION;
#endif
#define SOC_SYSQIC_IB_CT_CTRL_head_byp_dis_START (0)
#define SOC_SYSQIC_IB_CT_CTRL_head_byp_dis_END (0)
#define SOC_SYSQIC_IB_CT_CTRL_rlast_arb_START (1)
#define SOC_SYSQIC_IB_CT_CTRL_rlast_arb_END (1)
#define SOC_SYSQIC_IB_CT_CTRL_free_disable_START (2)
#define SOC_SYSQIC_IB_CT_CTRL_free_disable_END (2)
#define SOC_SYSQIC_IB_CT_CTRL_rct_ost_wl_START (16)
#define SOC_SYSQIC_IB_CT_CTRL_rct_ost_wl_END (23)
#define SOC_SYSQIC_IB_CT_CTRL_wct_ost_wl_START (24)
#define SOC_SYSQIC_IB_CT_CTRL_wct_ost_wl_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int timeout_dis : 1;
        unsigned int reserved_0 : 3;
        unsigned int timeout_base_sel : 4;
        unsigned int reserved_1 : 24;
    } reg;
} SOC_SYSQIC_IB_TIMEOUT_CTRL_UNION;
#endif
#define SOC_SYSQIC_IB_TIMEOUT_CTRL_timeout_dis_START (0)
#define SOC_SYSQIC_IB_TIMEOUT_CTRL_timeout_dis_END (0)
#define SOC_SYSQIC_IB_TIMEOUT_CTRL_timeout_base_sel_START (4)
#define SOC_SYSQIC_IB_TIMEOUT_CTRL_timeout_base_sel_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int ecc_dis : 1;
        unsigned int reserved_0 : 3;
        unsigned int eccerr_waterline : 8;
        unsigned int reserved_1 : 20;
    } reg;
} SOC_SYSQIC_IB_ECC_CTRL_UNION;
#endif
#define SOC_SYSQIC_IB_ECC_CTRL_ecc_dis_START (0)
#define SOC_SYSQIC_IB_ECC_CTRL_ecc_dis_END (0)
#define SOC_SYSQIC_IB_ECC_CTRL_eccerr_waterline_START (4)
#define SOC_SYSQIC_IB_ECC_CTRL_eccerr_waterline_END (11)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int parity_en : 1;
        unsigned int reserved : 31;
    } reg;
} SOC_SYSQIC_IB_PARITY_CTRL_UNION;
#endif
#define SOC_SYSQIC_IB_PARITY_CTRL_parity_en_START (0)
#define SOC_SYSQIC_IB_PARITY_CTRL_parity_en_END (0)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int cfc0_bandwidth : 8;
        unsigned int cfc0_saturation : 8;
        unsigned int reserved_0 : 8;
        unsigned int cfc0_lp : 2;
        unsigned int reserved_1 : 4;
        unsigned int cfc0_type : 1;
        unsigned int cfc0_en : 1;
    } reg;
} SOC_SYSQIC_IB_QOS_LMTR0_CTRL_UNION;
#endif
#define SOC_SYSQIC_IB_QOS_LMTR0_CTRL_cfc0_bandwidth_START (0)
#define SOC_SYSQIC_IB_QOS_LMTR0_CTRL_cfc0_bandwidth_END (7)
#define SOC_SYSQIC_IB_QOS_LMTR0_CTRL_cfc0_saturation_START (8)
#define SOC_SYSQIC_IB_QOS_LMTR0_CTRL_cfc0_saturation_END (15)
#define SOC_SYSQIC_IB_QOS_LMTR0_CTRL_cfc0_lp_START (24)
#define SOC_SYSQIC_IB_QOS_LMTR0_CTRL_cfc0_lp_END (25)
#define SOC_SYSQIC_IB_QOS_LMTR0_CTRL_cfc0_type_START (30)
#define SOC_SYSQIC_IB_QOS_LMTR0_CTRL_cfc0_type_END (30)
#define SOC_SYSQIC_IB_QOS_LMTR0_CTRL_cfc0_en_START (31)
#define SOC_SYSQIC_IB_QOS_LMTR0_CTRL_cfc0_en_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int cfc1_bandwidth : 8;
        unsigned int cfc1_saturation : 8;
        unsigned int reserved_0 : 8;
        unsigned int cfc1_lp : 2;
        unsigned int reserved_1 : 5;
        unsigned int cfc1_en : 1;
    } reg;
} SOC_SYSQIC_IB_QOS_LMTR1_CTRL_UNION;
#endif
#define SOC_SYSQIC_IB_QOS_LMTR1_CTRL_cfc1_bandwidth_START (0)
#define SOC_SYSQIC_IB_QOS_LMTR1_CTRL_cfc1_bandwidth_END (7)
#define SOC_SYSQIC_IB_QOS_LMTR1_CTRL_cfc1_saturation_START (8)
#define SOC_SYSQIC_IB_QOS_LMTR1_CTRL_cfc1_saturation_END (15)
#define SOC_SYSQIC_IB_QOS_LMTR1_CTRL_cfc1_lp_START (24)
#define SOC_SYSQIC_IB_QOS_LMTR1_CTRL_cfc1_lp_END (25)
#define SOC_SYSQIC_IB_QOS_LMTR1_CTRL_cfc1_en_START (31)
#define SOC_SYSQIC_IB_QOS_LMTR1_CTRL_cfc1_en_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int cfc2_bandwidth : 8;
        unsigned int cfc2_saturation : 8;
        unsigned int reserved_0 : 8;
        unsigned int cfc2_lp : 2;
        unsigned int reserved_1 : 4;
        unsigned int cfc2_type : 1;
        unsigned int cfc2_en : 1;
    } reg;
} SOC_SYSQIC_IB_QOS_LMTR2_CTRL_UNION;
#endif
#define SOC_SYSQIC_IB_QOS_LMTR2_CTRL_cfc2_bandwidth_START (0)
#define SOC_SYSQIC_IB_QOS_LMTR2_CTRL_cfc2_bandwidth_END (7)
#define SOC_SYSQIC_IB_QOS_LMTR2_CTRL_cfc2_saturation_START (8)
#define SOC_SYSQIC_IB_QOS_LMTR2_CTRL_cfc2_saturation_END (15)
#define SOC_SYSQIC_IB_QOS_LMTR2_CTRL_cfc2_lp_START (24)
#define SOC_SYSQIC_IB_QOS_LMTR2_CTRL_cfc2_lp_END (25)
#define SOC_SYSQIC_IB_QOS_LMTR2_CTRL_cfc2_type_START (30)
#define SOC_SYSQIC_IB_QOS_LMTR2_CTRL_cfc2_type_END (30)
#define SOC_SYSQIC_IB_QOS_LMTR2_CTRL_cfc2_en_START (31)
#define SOC_SYSQIC_IB_QOS_LMTR2_CTRL_cfc2_en_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int cfc3_bandwidth : 8;
        unsigned int cfc3_saturation : 8;
        unsigned int reserved_0 : 8;
        unsigned int cfc3_lp : 2;
        unsigned int reserved_1 : 5;
        unsigned int cfc3_en : 1;
    } reg;
} SOC_SYSQIC_IB_QOS_LMTR3_CTRL_UNION;
#endif
#define SOC_SYSQIC_IB_QOS_LMTR3_CTRL_cfc3_bandwidth_START (0)
#define SOC_SYSQIC_IB_QOS_LMTR3_CTRL_cfc3_bandwidth_END (7)
#define SOC_SYSQIC_IB_QOS_LMTR3_CTRL_cfc3_saturation_START (8)
#define SOC_SYSQIC_IB_QOS_LMTR3_CTRL_cfc3_saturation_END (15)
#define SOC_SYSQIC_IB_QOS_LMTR3_CTRL_cfc3_lp_START (24)
#define SOC_SYSQIC_IB_QOS_LMTR3_CTRL_cfc3_lp_END (25)
#define SOC_SYSQIC_IB_QOS_LMTR3_CTRL_cfc3_en_START (31)
#define SOC_SYSQIC_IB_QOS_LMTR3_CTRL_cfc3_en_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int cfc0_ostd_lvl : 8;
        unsigned int reserved : 24;
    } reg;
} SOC_SYSQIC_IB_QOS_OSTD0_CTRL_UNION;
#endif
#define SOC_SYSQIC_IB_QOS_OSTD0_CTRL_cfc0_ostd_lvl_START (0)
#define SOC_SYSQIC_IB_QOS_OSTD0_CTRL_cfc0_ostd_lvl_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int cfc1_ostd_lvl : 8;
        unsigned int reserved : 24;
    } reg;
} SOC_SYSQIC_IB_QOS_OSTD1_CTRL_UNION;
#endif
#define SOC_SYSQIC_IB_QOS_OSTD1_CTRL_cfc1_ostd_lvl_START (0)
#define SOC_SYSQIC_IB_QOS_OSTD1_CTRL_cfc1_ostd_lvl_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int cfc2_ostd_lvl : 8;
        unsigned int reserved : 24;
    } reg;
} SOC_SYSQIC_IB_QOS_OSTD2_CTRL_UNION;
#endif
#define SOC_SYSQIC_IB_QOS_OSTD2_CTRL_cfc2_ostd_lvl_START (0)
#define SOC_SYSQIC_IB_QOS_OSTD2_CTRL_cfc2_ostd_lvl_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int cfc3_ostd_lvl : 8;
        unsigned int reserved : 24;
    } reg;
} SOC_SYSQIC_IB_QOS_OSTD3_CTRL_UNION;
#endif
#define SOC_SYSQIC_IB_QOS_OSTD3_CTRL_cfc3_ostd_lvl_START (0)
#define SOC_SYSQIC_IB_QOS_OSTD3_CTRL_cfc3_ostd_lvl_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int rd_qos0 : 3;
        unsigned int reserved_0: 1;
        unsigned int rd_qos1 : 3;
        unsigned int reserved_1: 1;
        unsigned int rd_qos2 : 3;
        unsigned int reserved_2: 1;
        unsigned int rd_qos3 : 3;
        unsigned int reserved_3: 1;
        unsigned int rd_qos4 : 3;
        unsigned int reserved_4: 1;
        unsigned int rd_qos5 : 3;
        unsigned int reserved_5: 1;
        unsigned int rd_qos6 : 3;
        unsigned int reserved_6: 1;
        unsigned int rd_qos7 : 3;
        unsigned int reserved_7: 1;
    } reg;
} SOC_SYSQIC_IB_QOS_RDQOS_CTRL_UNION;
#endif
#define SOC_SYSQIC_IB_QOS_RDQOS_CTRL_rd_qos0_START (0)
#define SOC_SYSQIC_IB_QOS_RDQOS_CTRL_rd_qos0_END (2)
#define SOC_SYSQIC_IB_QOS_RDQOS_CTRL_rd_qos1_START (4)
#define SOC_SYSQIC_IB_QOS_RDQOS_CTRL_rd_qos1_END (6)
#define SOC_SYSQIC_IB_QOS_RDQOS_CTRL_rd_qos2_START (8)
#define SOC_SYSQIC_IB_QOS_RDQOS_CTRL_rd_qos2_END (10)
#define SOC_SYSQIC_IB_QOS_RDQOS_CTRL_rd_qos3_START (12)
#define SOC_SYSQIC_IB_QOS_RDQOS_CTRL_rd_qos3_END (14)
#define SOC_SYSQIC_IB_QOS_RDQOS_CTRL_rd_qos4_START (16)
#define SOC_SYSQIC_IB_QOS_RDQOS_CTRL_rd_qos4_END (18)
#define SOC_SYSQIC_IB_QOS_RDQOS_CTRL_rd_qos5_START (20)
#define SOC_SYSQIC_IB_QOS_RDQOS_CTRL_rd_qos5_END (22)
#define SOC_SYSQIC_IB_QOS_RDQOS_CTRL_rd_qos6_START (24)
#define SOC_SYSQIC_IB_QOS_RDQOS_CTRL_rd_qos6_END (26)
#define SOC_SYSQIC_IB_QOS_RDQOS_CTRL_rd_qos7_START (28)
#define SOC_SYSQIC_IB_QOS_RDQOS_CTRL_rd_qos7_END (30)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int wr_qos0 : 3;
        unsigned int reserved_0: 1;
        unsigned int wr_qos1 : 3;
        unsigned int reserved_1: 1;
        unsigned int wr_qos2 : 3;
        unsigned int reserved_2: 1;
        unsigned int wr_qos3 : 3;
        unsigned int reserved_3: 1;
        unsigned int wr_qos4 : 3;
        unsigned int reserved_4: 1;
        unsigned int wr_qos5 : 3;
        unsigned int reserved_5: 1;
        unsigned int wr_qos6 : 3;
        unsigned int reserved_6: 1;
        unsigned int wr_qos7 : 3;
        unsigned int reserved_7: 1;
    } reg;
} SOC_SYSQIC_IB_QOS_WRQOS_CTRL_UNION;
#endif
#define SOC_SYSQIC_IB_QOS_WRQOS_CTRL_wr_qos0_START (0)
#define SOC_SYSQIC_IB_QOS_WRQOS_CTRL_wr_qos0_END (2)
#define SOC_SYSQIC_IB_QOS_WRQOS_CTRL_wr_qos1_START (4)
#define SOC_SYSQIC_IB_QOS_WRQOS_CTRL_wr_qos1_END (6)
#define SOC_SYSQIC_IB_QOS_WRQOS_CTRL_wr_qos2_START (8)
#define SOC_SYSQIC_IB_QOS_WRQOS_CTRL_wr_qos2_END (10)
#define SOC_SYSQIC_IB_QOS_WRQOS_CTRL_wr_qos3_START (12)
#define SOC_SYSQIC_IB_QOS_WRQOS_CTRL_wr_qos3_END (14)
#define SOC_SYSQIC_IB_QOS_WRQOS_CTRL_wr_qos4_START (16)
#define SOC_SYSQIC_IB_QOS_WRQOS_CTRL_wr_qos4_END (18)
#define SOC_SYSQIC_IB_QOS_WRQOS_CTRL_wr_qos5_START (20)
#define SOC_SYSQIC_IB_QOS_WRQOS_CTRL_wr_qos5_END (22)
#define SOC_SYSQIC_IB_QOS_WRQOS_CTRL_wr_qos6_START (24)
#define SOC_SYSQIC_IB_QOS_WRQOS_CTRL_wr_qos6_END (26)
#define SOC_SYSQIC_IB_QOS_WRQOS_CTRL_wr_qos7_START (28)
#define SOC_SYSQIC_IB_QOS_WRQOS_CTRL_wr_qos7_END (30)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int rd_pri0 : 2;
        unsigned int reserved_0: 2;
        unsigned int rd_pri1 : 2;
        unsigned int reserved_1: 2;
        unsigned int rd_pri2 : 2;
        unsigned int reserved_2: 2;
        unsigned int rd_pri3 : 2;
        unsigned int reserved_3: 2;
        unsigned int rd_pri4 : 2;
        unsigned int reserved_4: 2;
        unsigned int rd_pri5 : 2;
        unsigned int reserved_5: 2;
        unsigned int rd_pri6 : 2;
        unsigned int reserved_6: 2;
        unsigned int rd_pri7 : 2;
        unsigned int reserved_7: 2;
    } reg;
} SOC_SYSQIC_IB_QOS_RDPRI_CTRL_UNION;
#endif
#define SOC_SYSQIC_IB_QOS_RDPRI_CTRL_rd_pri0_START (0)
#define SOC_SYSQIC_IB_QOS_RDPRI_CTRL_rd_pri0_END (1)
#define SOC_SYSQIC_IB_QOS_RDPRI_CTRL_rd_pri1_START (4)
#define SOC_SYSQIC_IB_QOS_RDPRI_CTRL_rd_pri1_END (5)
#define SOC_SYSQIC_IB_QOS_RDPRI_CTRL_rd_pri2_START (8)
#define SOC_SYSQIC_IB_QOS_RDPRI_CTRL_rd_pri2_END (9)
#define SOC_SYSQIC_IB_QOS_RDPRI_CTRL_rd_pri3_START (12)
#define SOC_SYSQIC_IB_QOS_RDPRI_CTRL_rd_pri3_END (13)
#define SOC_SYSQIC_IB_QOS_RDPRI_CTRL_rd_pri4_START (16)
#define SOC_SYSQIC_IB_QOS_RDPRI_CTRL_rd_pri4_END (17)
#define SOC_SYSQIC_IB_QOS_RDPRI_CTRL_rd_pri5_START (20)
#define SOC_SYSQIC_IB_QOS_RDPRI_CTRL_rd_pri5_END (21)
#define SOC_SYSQIC_IB_QOS_RDPRI_CTRL_rd_pri6_START (24)
#define SOC_SYSQIC_IB_QOS_RDPRI_CTRL_rd_pri6_END (25)
#define SOC_SYSQIC_IB_QOS_RDPRI_CTRL_rd_pri7_START (28)
#define SOC_SYSQIC_IB_QOS_RDPRI_CTRL_rd_pri7_END (29)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int wr_pri0 : 2;
        unsigned int reserved_0: 2;
        unsigned int wr_pri1 : 2;
        unsigned int reserved_1: 2;
        unsigned int wr_pri2 : 2;
        unsigned int reserved_2: 2;
        unsigned int wr_pri3 : 2;
        unsigned int reserved_3: 2;
        unsigned int wr_pri4 : 2;
        unsigned int reserved_4: 2;
        unsigned int wr_pri5 : 2;
        unsigned int reserved_5: 2;
        unsigned int wr_pri6 : 2;
        unsigned int reserved_6: 2;
        unsigned int wr_pri7 : 2;
        unsigned int reserved_7: 2;
    } reg;
} SOC_SYSQIC_IB_QOS_WRPRI_CTRL_UNION;
#endif
#define SOC_SYSQIC_IB_QOS_WRPRI_CTRL_wr_pri0_START (0)
#define SOC_SYSQIC_IB_QOS_WRPRI_CTRL_wr_pri0_END (1)
#define SOC_SYSQIC_IB_QOS_WRPRI_CTRL_wr_pri1_START (4)
#define SOC_SYSQIC_IB_QOS_WRPRI_CTRL_wr_pri1_END (5)
#define SOC_SYSQIC_IB_QOS_WRPRI_CTRL_wr_pri2_START (8)
#define SOC_SYSQIC_IB_QOS_WRPRI_CTRL_wr_pri2_END (9)
#define SOC_SYSQIC_IB_QOS_WRPRI_CTRL_wr_pri3_START (12)
#define SOC_SYSQIC_IB_QOS_WRPRI_CTRL_wr_pri3_END (13)
#define SOC_SYSQIC_IB_QOS_WRPRI_CTRL_wr_pri4_START (16)
#define SOC_SYSQIC_IB_QOS_WRPRI_CTRL_wr_pri4_END (17)
#define SOC_SYSQIC_IB_QOS_WRPRI_CTRL_wr_pri5_START (20)
#define SOC_SYSQIC_IB_QOS_WRPRI_CTRL_wr_pri5_END (21)
#define SOC_SYSQIC_IB_QOS_WRPRI_CTRL_wr_pri6_START (24)
#define SOC_SYSQIC_IB_QOS_WRPRI_CTRL_wr_pri6_END (25)
#define SOC_SYSQIC_IB_QOS_WRPRI_CTRL_wr_pri7_START (28)
#define SOC_SYSQIC_IB_QOS_WRPRI_CTRL_wr_pri7_END (29)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int arpush_dis : 1;
        unsigned int awpush_dis : 1;
        unsigned int reserved : 30;
    } reg;
} SOC_SYSQIC_IB_QOS_PUSH_CTRL_UNION;
#endif
#define SOC_SYSQIC_IB_QOS_PUSH_CTRL_arpush_dis_START (0)
#define SOC_SYSQIC_IB_QOS_PUSH_CTRL_arpush_dis_END (0)
#define SOC_SYSQIC_IB_QOS_PUSH_CTRL_awpush_dis_START (1)
#define SOC_SYSQIC_IB_QOS_PUSH_CTRL_awpush_dis_END (1)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int reserved_0 : 8;
        unsigned int cfc_lite_saturation : 5;
        unsigned int reserved_1 : 11;
        unsigned int cfc_lite_lp : 2;
        unsigned int reserved_2 : 5;
        unsigned int cfc_lite_en : 1;
    } reg;
} SOC_SYSQIC_IB_QOS_LMTR_LITE_CTRL_UNION;
#endif
#define SOC_SYSQIC_IB_QOS_LMTR_LITE_CTRL_cfc_lite_saturation_START (8)
#define SOC_SYSQIC_IB_QOS_LMTR_LITE_CTRL_cfc_lite_saturation_END (12)
#define SOC_SYSQIC_IB_QOS_LMTR_LITE_CTRL_cfc_lite_lp_START (24)
#define SOC_SYSQIC_IB_QOS_LMTR_LITE_CTRL_cfc_lite_lp_END (25)
#define SOC_SYSQIC_IB_QOS_LMTR_LITE_CTRL_cfc_lite_en_START (31)
#define SOC_SYSQIC_IB_QOS_LMTR_LITE_CTRL_cfc_lite_en_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int rglr0_bandwidth : 8;
        unsigned int rglr0_saturation : 8;
        unsigned int rglr0_pri_low : 3;
        unsigned int reserved_0 : 1;
        unsigned int rglr0_pri_high : 3;
        unsigned int reserved_1 : 1;
        unsigned int rglr0_lp : 2;
        unsigned int reserved_2 : 4;
        unsigned int rglr0_type : 1;
        unsigned int rglr0_en : 1;
    } reg;
} SOC_SYSQIC_IB_QOS_RGLR0_CTRL_UNION;
#endif
#define SOC_SYSQIC_IB_QOS_RGLR0_CTRL_rglr0_bandwidth_START (0)
#define SOC_SYSQIC_IB_QOS_RGLR0_CTRL_rglr0_bandwidth_END (7)
#define SOC_SYSQIC_IB_QOS_RGLR0_CTRL_rglr0_saturation_START (8)
#define SOC_SYSQIC_IB_QOS_RGLR0_CTRL_rglr0_saturation_END (15)
#define SOC_SYSQIC_IB_QOS_RGLR0_CTRL_rglr0_pri_low_START (16)
#define SOC_SYSQIC_IB_QOS_RGLR0_CTRL_rglr0_pri_low_END (18)
#define SOC_SYSQIC_IB_QOS_RGLR0_CTRL_rglr0_pri_high_START (20)
#define SOC_SYSQIC_IB_QOS_RGLR0_CTRL_rglr0_pri_high_END (22)
#define SOC_SYSQIC_IB_QOS_RGLR0_CTRL_rglr0_lp_START (24)
#define SOC_SYSQIC_IB_QOS_RGLR0_CTRL_rglr0_lp_END (25)
#define SOC_SYSQIC_IB_QOS_RGLR0_CTRL_rglr0_type_START (30)
#define SOC_SYSQIC_IB_QOS_RGLR0_CTRL_rglr0_type_END (30)
#define SOC_SYSQIC_IB_QOS_RGLR0_CTRL_rglr0_en_START (31)
#define SOC_SYSQIC_IB_QOS_RGLR0_CTRL_rglr0_en_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int rglr1_bandwidth : 8;
        unsigned int rglr1_saturation : 8;
        unsigned int rglr1_pri_low : 3;
        unsigned int reserved_0 : 1;
        unsigned int rglr1_pri_high : 3;
        unsigned int reserved_1 : 1;
        unsigned int rglr1_lp : 2;
        unsigned int reserved_2 : 5;
        unsigned int rglr1_en : 1;
    } reg;
} SOC_SYSQIC_IB_QOS_RGLR1_CTRL_UNION;
#endif
#define SOC_SYSQIC_IB_QOS_RGLR1_CTRL_rglr1_bandwidth_START (0)
#define SOC_SYSQIC_IB_QOS_RGLR1_CTRL_rglr1_bandwidth_END (7)
#define SOC_SYSQIC_IB_QOS_RGLR1_CTRL_rglr1_saturation_START (8)
#define SOC_SYSQIC_IB_QOS_RGLR1_CTRL_rglr1_saturation_END (15)
#define SOC_SYSQIC_IB_QOS_RGLR1_CTRL_rglr1_pri_low_START (16)
#define SOC_SYSQIC_IB_QOS_RGLR1_CTRL_rglr1_pri_low_END (18)
#define SOC_SYSQIC_IB_QOS_RGLR1_CTRL_rglr1_pri_high_START (20)
#define SOC_SYSQIC_IB_QOS_RGLR1_CTRL_rglr1_pri_high_END (22)
#define SOC_SYSQIC_IB_QOS_RGLR1_CTRL_rglr1_lp_START (24)
#define SOC_SYSQIC_IB_QOS_RGLR1_CTRL_rglr1_lp_END (25)
#define SOC_SYSQIC_IB_QOS_RGLR1_CTRL_rglr1_en_START (31)
#define SOC_SYSQIC_IB_QOS_RGLR1_CTRL_rglr1_en_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int wreq_stat_en : 1;
        unsigned int reserved_0 : 3;
        unsigned int wreq_stat_mode : 2;
        unsigned int reserved_1 : 2;
        unsigned int wreq_stat_cnt_mode : 24;
    } reg;
} SOC_SYSQIC_IB_PROBE_WREQ_CFG0_UNION;
#endif
#define SOC_SYSQIC_IB_PROBE_WREQ_CFG0_wreq_stat_en_START (0)
#define SOC_SYSQIC_IB_PROBE_WREQ_CFG0_wreq_stat_en_END (0)
#define SOC_SYSQIC_IB_PROBE_WREQ_CFG0_wreq_stat_mode_START (4)
#define SOC_SYSQIC_IB_PROBE_WREQ_CFG0_wreq_stat_mode_END (5)
#define SOC_SYSQIC_IB_PROBE_WREQ_CFG0_wreq_stat_cnt_mode_START (8)
#define SOC_SYSQIC_IB_PROBE_WREQ_CFG0_wreq_stat_cnt_mode_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int wreq_latcy_mode : 1;
        unsigned int reserved_0 : 3;
        unsigned int wreq_latcy_prescaler : 10;
        unsigned int reserved_1 : 2;
        unsigned int wreq_pfm_wreq_align_mode : 1;
        unsigned int wreq_pfm_wreq_bin4_cmd_type : 1;
        unsigned int reserved_2 : 14;
    } reg;
} SOC_SYSQIC_IB_PROBE_WREQ_CFG1_UNION;
#endif
#define SOC_SYSQIC_IB_PROBE_WREQ_CFG1_wreq_latcy_mode_START (0)
#define SOC_SYSQIC_IB_PROBE_WREQ_CFG1_wreq_latcy_mode_END (0)
#define SOC_SYSQIC_IB_PROBE_WREQ_CFG1_wreq_latcy_prescaler_START (4)
#define SOC_SYSQIC_IB_PROBE_WREQ_CFG1_wreq_latcy_prescaler_END (13)
#define SOC_SYSQIC_IB_PROBE_WREQ_CFG1_wreq_pfm_wreq_align_mode_START (16)
#define SOC_SYSQIC_IB_PROBE_WREQ_CFG1_wreq_pfm_wreq_align_mode_END (16)
#define SOC_SYSQIC_IB_PROBE_WREQ_CFG1_wreq_pfm_wreq_bin4_cmd_type_START (17)
#define SOC_SYSQIC_IB_PROBE_WREQ_CFG1_wreq_pfm_wreq_bin4_cmd_type_END (17)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int wreq_addr_s0 : 32;
    } reg;
} SOC_SYSQIC_IB_PROBE_WREQ_FILTER0_UNION;
#endif
#define SOC_SYSQIC_IB_PROBE_WREQ_FILTER0_wreq_addr_s0_START (0)
#define SOC_SYSQIC_IB_PROBE_WREQ_FILTER0_wreq_addr_s0_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int wreq_addr_s1 : 32;
    } reg;
} SOC_SYSQIC_IB_PROBE_WREQ_FILTER1_UNION;
#endif
#define SOC_SYSQIC_IB_PROBE_WREQ_FILTER1_wreq_addr_s1_START (0)
#define SOC_SYSQIC_IB_PROBE_WREQ_FILTER1_wreq_addr_s1_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int wreq_addr_e0 : 32;
    } reg;
} SOC_SYSQIC_IB_PROBE_WREQ_FILTER2_UNION;
#endif
#define SOC_SYSQIC_IB_PROBE_WREQ_FILTER2_wreq_addr_e0_START (0)
#define SOC_SYSQIC_IB_PROBE_WREQ_FILTER2_wreq_addr_e0_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int wreq_addr_e1 : 32;
    } reg;
} SOC_SYSQIC_IB_PROBE_WREQ_FILTER3_UNION;
#endif
#define SOC_SYSQIC_IB_PROBE_WREQ_FILTER3_wreq_addr_e1_START (0)
#define SOC_SYSQIC_IB_PROBE_WREQ_FILTER3_wreq_addr_e1_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int wreq_user0 : 32;
    } reg;
} SOC_SYSQIC_IB_PROBE_WREQ_FILTER4_0_UNION;
#endif
#define SOC_SYSQIC_IB_PROBE_WREQ_FILTER4_0_wreq_user0_START (0)
#define SOC_SYSQIC_IB_PROBE_WREQ_FILTER4_0_wreq_user0_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int wreq_user1 : 32;
    } reg;
} SOC_SYSQIC_IB_PROBE_WREQ_FILTER4_1_UNION;
#endif
#define SOC_SYSQIC_IB_PROBE_WREQ_FILTER4_1_wreq_user1_START (0)
#define SOC_SYSQIC_IB_PROBE_WREQ_FILTER4_1_wreq_user1_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int wreq_user2 : 32;
    } reg;
} SOC_SYSQIC_IB_PROBE_WREQ_FILTER4_2_UNION;
#endif
#define SOC_SYSQIC_IB_PROBE_WREQ_FILTER4_2_wreq_user2_START (0)
#define SOC_SYSQIC_IB_PROBE_WREQ_FILTER4_2_wreq_user2_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int wreq_user3 : 32;
    } reg;
} SOC_SYSQIC_IB_PROBE_WREQ_FILTER4_3_UNION;
#endif
#define SOC_SYSQIC_IB_PROBE_WREQ_FILTER4_3_wreq_user3_START (0)
#define SOC_SYSQIC_IB_PROBE_WREQ_FILTER4_3_wreq_user3_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int wreq_user_msk0 : 32;
    } reg;
} SOC_SYSQIC_IB_PROBE_WREQ_FILTER5_0_UNION;
#endif
#define SOC_SYSQIC_IB_PROBE_WREQ_FILTER5_0_wreq_user_msk0_START (0)
#define SOC_SYSQIC_IB_PROBE_WREQ_FILTER5_0_wreq_user_msk0_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int wreq_user_msk1 : 32;
    } reg;
} SOC_SYSQIC_IB_PROBE_WREQ_FILTER5_1_UNION;
#endif
#define SOC_SYSQIC_IB_PROBE_WREQ_FILTER5_1_wreq_user_msk1_START (0)
#define SOC_SYSQIC_IB_PROBE_WREQ_FILTER5_1_wreq_user_msk1_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int wreq_user_msk2 : 32;
    } reg;
} SOC_SYSQIC_IB_PROBE_WREQ_FILTER5_2_UNION;
#endif
#define SOC_SYSQIC_IB_PROBE_WREQ_FILTER5_2_wreq_user_msk2_START (0)
#define SOC_SYSQIC_IB_PROBE_WREQ_FILTER5_2_wreq_user_msk2_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int wreq_user_msk3 : 32;
    } reg;
} SOC_SYSQIC_IB_PROBE_WREQ_FILTER5_3_UNION;
#endif
#define SOC_SYSQIC_IB_PROBE_WREQ_FILTER5_3_wreq_user_msk3_START (0)
#define SOC_SYSQIC_IB_PROBE_WREQ_FILTER5_3_wreq_user_msk3_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int wreq_exclid : 8;
        unsigned int wreq_exclid_msk : 8;
        unsigned int wreq_sf : 8;
        unsigned int wreq_sf_msk : 8;
    } reg;
} SOC_SYSQIC_IB_PROBE_WREQ_FILTER6_UNION;
#endif
#define SOC_SYSQIC_IB_PROBE_WREQ_FILTER6_wreq_exclid_START (0)
#define SOC_SYSQIC_IB_PROBE_WREQ_FILTER6_wreq_exclid_END (7)
#define SOC_SYSQIC_IB_PROBE_WREQ_FILTER6_wreq_exclid_msk_START (8)
#define SOC_SYSQIC_IB_PROBE_WREQ_FILTER6_wreq_exclid_msk_END (15)
#define SOC_SYSQIC_IB_PROBE_WREQ_FILTER6_wreq_sf_START (16)
#define SOC_SYSQIC_IB_PROBE_WREQ_FILTER6_wreq_sf_END (23)
#define SOC_SYSQIC_IB_PROBE_WREQ_FILTER6_wreq_sf_msk_START (24)
#define SOC_SYSQIC_IB_PROBE_WREQ_FILTER6_wreq_sf_msk_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int wreq_qos : 8;
        unsigned int wreq_tid : 8;
        unsigned int wreq_tid_msk : 8;
        unsigned int wreq_lck : 4;
        unsigned int wreq_lck_msk : 4;
    } reg;
} SOC_SYSQIC_IB_PROBE_WREQ_FILTER7_UNION;
#endif
#define SOC_SYSQIC_IB_PROBE_WREQ_FILTER7_wreq_qos_START (0)
#define SOC_SYSQIC_IB_PROBE_WREQ_FILTER7_wreq_qos_END (7)
#define SOC_SYSQIC_IB_PROBE_WREQ_FILTER7_wreq_tid_START (8)
#define SOC_SYSQIC_IB_PROBE_WREQ_FILTER7_wreq_tid_END (15)
#define SOC_SYSQIC_IB_PROBE_WREQ_FILTER7_wreq_tid_msk_START (16)
#define SOC_SYSQIC_IB_PROBE_WREQ_FILTER7_wreq_tid_msk_END (23)
#define SOC_SYSQIC_IB_PROBE_WREQ_FILTER7_wreq_lck_START (24)
#define SOC_SYSQIC_IB_PROBE_WREQ_FILTER7_wreq_lck_END (27)
#define SOC_SYSQIC_IB_PROBE_WREQ_FILTER7_wreq_lck_msk_START (28)
#define SOC_SYSQIC_IB_PROBE_WREQ_FILTER7_wreq_lck_msk_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int wreq_tsize_min : 16;
        unsigned int wreq_tsize_max : 16;
    } reg;
} SOC_SYSQIC_IB_PROBE_WREQ_FILTER8_UNION;
#endif
#define SOC_SYSQIC_IB_PROBE_WREQ_FILTER8_wreq_tsize_min_START (0)
#define SOC_SYSQIC_IB_PROBE_WREQ_FILTER8_wreq_tsize_min_END (15)
#define SOC_SYSQIC_IB_PROBE_WREQ_FILTER8_wreq_tsize_max_START (16)
#define SOC_SYSQIC_IB_PROBE_WREQ_FILTER8_wreq_tsize_max_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int wreq_dstid : 8;
        unsigned int wreq_dstid_msk : 8;
        unsigned int wreq_vcnum : 4;
        unsigned int wreq_vcnum_msk : 4;
        unsigned int wreq_burst : 1;
        unsigned int wreq_burst_msk : 1;
        unsigned int wreq_wr : 1;
        unsigned int wreq_wr_msk : 1;
        unsigned int wreq_splt_cmd : 1;
        unsigned int wreq_splt_cmd_msk : 1;
        unsigned int reserved : 2;
    } reg;
} SOC_SYSQIC_IB_PROBE_WREQ_FILTER9_UNION;
#endif
#define SOC_SYSQIC_IB_PROBE_WREQ_FILTER9_wreq_dstid_START (0)
#define SOC_SYSQIC_IB_PROBE_WREQ_FILTER9_wreq_dstid_END (7)
#define SOC_SYSQIC_IB_PROBE_WREQ_FILTER9_wreq_dstid_msk_START (8)
#define SOC_SYSQIC_IB_PROBE_WREQ_FILTER9_wreq_dstid_msk_END (15)
#define SOC_SYSQIC_IB_PROBE_WREQ_FILTER9_wreq_vcnum_START (16)
#define SOC_SYSQIC_IB_PROBE_WREQ_FILTER9_wreq_vcnum_END (19)
#define SOC_SYSQIC_IB_PROBE_WREQ_FILTER9_wreq_vcnum_msk_START (20)
#define SOC_SYSQIC_IB_PROBE_WREQ_FILTER9_wreq_vcnum_msk_END (23)
#define SOC_SYSQIC_IB_PROBE_WREQ_FILTER9_wreq_burst_START (24)
#define SOC_SYSQIC_IB_PROBE_WREQ_FILTER9_wreq_burst_END (24)
#define SOC_SYSQIC_IB_PROBE_WREQ_FILTER9_wreq_burst_msk_START (25)
#define SOC_SYSQIC_IB_PROBE_WREQ_FILTER9_wreq_burst_msk_END (25)
#define SOC_SYSQIC_IB_PROBE_WREQ_FILTER9_wreq_wr_START (26)
#define SOC_SYSQIC_IB_PROBE_WREQ_FILTER9_wreq_wr_END (26)
#define SOC_SYSQIC_IB_PROBE_WREQ_FILTER9_wreq_wr_msk_START (27)
#define SOC_SYSQIC_IB_PROBE_WREQ_FILTER9_wreq_wr_msk_END (27)
#define SOC_SYSQIC_IB_PROBE_WREQ_FILTER9_wreq_splt_cmd_START (28)
#define SOC_SYSQIC_IB_PROBE_WREQ_FILTER9_wreq_splt_cmd_END (28)
#define SOC_SYSQIC_IB_PROBE_WREQ_FILTER9_wreq_splt_cmd_msk_START (29)
#define SOC_SYSQIC_IB_PROBE_WREQ_FILTER9_wreq_splt_cmd_msk_END (29)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int wreq_stat_cnt_int : 8;
        unsigned int reserved : 24;
    } reg;
} SOC_SYSQIC_IB_PROBE_WREQ_INT_STAUS_UNION;
#endif
#define SOC_SYSQIC_IB_PROBE_WREQ_INT_STAUS_wreq_stat_cnt_int_START (0)
#define SOC_SYSQIC_IB_PROBE_WREQ_INT_STAUS_wreq_stat_cnt_int_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int wreq_stat_cnt_int_clr : 8;
        unsigned int reserved : 24;
    } reg;
} SOC_SYSQIC_IB_PROBE_WREQ_INT_CLR_UNION;
#endif
#define SOC_SYSQIC_IB_PROBE_WREQ_INT_CLR_wreq_stat_cnt_int_clr_START (0)
#define SOC_SYSQIC_IB_PROBE_WREQ_INT_CLR_wreq_stat_cnt_int_clr_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int wreq_stat_cnt : 32;
    } reg;
} SOC_SYSQIC_IB_PROBE_WREQ_CNT_UNION;
#endif
#define SOC_SYSQIC_IB_PROBE_WREQ_CNT_wreq_stat_cnt_START (0)
#define SOC_SYSQIC_IB_PROBE_WREQ_CNT_wreq_stat_cnt_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int wreq_stat_cnt_int_th : 32;
    } reg;
} SOC_SYSQIC_IB_PROBE_WREQ_INT_CFG_UNION;
#endif
#define SOC_SYSQIC_IB_PROBE_WREQ_INT_CFG_wreq_stat_cnt_int_th_START (0)
#define SOC_SYSQIC_IB_PROBE_WREQ_INT_CFG_wreq_stat_cnt_int_th_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int rreq_stat_en : 1;
        unsigned int reserved_0 : 3;
        unsigned int rreq_stat_mode : 2;
        unsigned int reserved_1 : 2;
        unsigned int rreq_stat_cnt_mode : 24;
    } reg;
} SOC_SYSQIC_IB_PROBE_RREQ_CFG0_UNION;
#endif
#define SOC_SYSQIC_IB_PROBE_RREQ_CFG0_rreq_stat_en_START (0)
#define SOC_SYSQIC_IB_PROBE_RREQ_CFG0_rreq_stat_en_END (0)
#define SOC_SYSQIC_IB_PROBE_RREQ_CFG0_rreq_stat_mode_START (4)
#define SOC_SYSQIC_IB_PROBE_RREQ_CFG0_rreq_stat_mode_END (5)
#define SOC_SYSQIC_IB_PROBE_RREQ_CFG0_rreq_stat_cnt_mode_START (8)
#define SOC_SYSQIC_IB_PROBE_RREQ_CFG0_rreq_stat_cnt_mode_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int rreq_latcy_mode : 1;
        unsigned int reserved_0 : 3;
        unsigned int rreq_latcy_prescaler : 10;
        unsigned int reserved_1 : 2;
        unsigned int rreq_pfm_rreq_align_mode : 1;
        unsigned int rreq_pfm_rreq_bin4_cmd_type : 1;
        unsigned int reserved_2 : 14;
    } reg;
} SOC_SYSQIC_IB_PROBE_RREQ_CFG1_UNION;
#endif
#define SOC_SYSQIC_IB_PROBE_RREQ_CFG1_rreq_latcy_mode_START (0)
#define SOC_SYSQIC_IB_PROBE_RREQ_CFG1_rreq_latcy_mode_END (0)
#define SOC_SYSQIC_IB_PROBE_RREQ_CFG1_rreq_latcy_prescaler_START (4)
#define SOC_SYSQIC_IB_PROBE_RREQ_CFG1_rreq_latcy_prescaler_END (13)
#define SOC_SYSQIC_IB_PROBE_RREQ_CFG1_rreq_pfm_rreq_align_mode_START (16)
#define SOC_SYSQIC_IB_PROBE_RREQ_CFG1_rreq_pfm_rreq_align_mode_END (16)
#define SOC_SYSQIC_IB_PROBE_RREQ_CFG1_rreq_pfm_rreq_bin4_cmd_type_START (17)
#define SOC_SYSQIC_IB_PROBE_RREQ_CFG1_rreq_pfm_rreq_bin4_cmd_type_END (17)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int rreq_addr_s0 : 32;
    } reg;
} SOC_SYSQIC_IB_PROBE_RREQ_FILTER0_UNION;
#endif
#define SOC_SYSQIC_IB_PROBE_RREQ_FILTER0_rreq_addr_s0_START (0)
#define SOC_SYSQIC_IB_PROBE_RREQ_FILTER0_rreq_addr_s0_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int rreq_addr_s1 : 32;
    } reg;
} SOC_SYSQIC_IB_PROBE_RREQ_FILTER1_UNION;
#endif
#define SOC_SYSQIC_IB_PROBE_RREQ_FILTER1_rreq_addr_s1_START (0)
#define SOC_SYSQIC_IB_PROBE_RREQ_FILTER1_rreq_addr_s1_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int rreq_addr_e0 : 32;
    } reg;
} SOC_SYSQIC_IB_PROBE_RREQ_FILTER2_UNION;
#endif
#define SOC_SYSQIC_IB_PROBE_RREQ_FILTER2_rreq_addr_e0_START (0)
#define SOC_SYSQIC_IB_PROBE_RREQ_FILTER2_rreq_addr_e0_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int rreq_addr_e1 : 32;
    } reg;
} SOC_SYSQIC_IB_PROBE_RREQ_FILTER3_UNION;
#endif
#define SOC_SYSQIC_IB_PROBE_RREQ_FILTER3_rreq_addr_e1_START (0)
#define SOC_SYSQIC_IB_PROBE_RREQ_FILTER3_rreq_addr_e1_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int rreq_user0 : 32;
    } reg;
} SOC_SYSQIC_IB_PROBE_RREQ_FILTER4_0_UNION;
#endif
#define SOC_SYSQIC_IB_PROBE_RREQ_FILTER4_0_rreq_user0_START (0)
#define SOC_SYSQIC_IB_PROBE_RREQ_FILTER4_0_rreq_user0_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int rreq_user1 : 32;
    } reg;
} SOC_SYSQIC_IB_PROBE_RREQ_FILTER4_1_UNION;
#endif
#define SOC_SYSQIC_IB_PROBE_RREQ_FILTER4_1_rreq_user1_START (0)
#define SOC_SYSQIC_IB_PROBE_RREQ_FILTER4_1_rreq_user1_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int rreq_user2 : 32;
    } reg;
} SOC_SYSQIC_IB_PROBE_RREQ_FILTER4_2_UNION;
#endif
#define SOC_SYSQIC_IB_PROBE_RREQ_FILTER4_2_rreq_user2_START (0)
#define SOC_SYSQIC_IB_PROBE_RREQ_FILTER4_2_rreq_user2_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int rreq_user3 : 32;
    } reg;
} SOC_SYSQIC_IB_PROBE_RREQ_FILTER4_3_UNION;
#endif
#define SOC_SYSQIC_IB_PROBE_RREQ_FILTER4_3_rreq_user3_START (0)
#define SOC_SYSQIC_IB_PROBE_RREQ_FILTER4_3_rreq_user3_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int rreq_user_msk0 : 32;
    } reg;
} SOC_SYSQIC_IB_PROBE_RREQ_FILTER5_0_UNION;
#endif
#define SOC_SYSQIC_IB_PROBE_RREQ_FILTER5_0_rreq_user_msk0_START (0)
#define SOC_SYSQIC_IB_PROBE_RREQ_FILTER5_0_rreq_user_msk0_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int rreq_user_msk1 : 32;
    } reg;
} SOC_SYSQIC_IB_PROBE_RREQ_FILTER5_1_UNION;
#endif
#define SOC_SYSQIC_IB_PROBE_RREQ_FILTER5_1_rreq_user_msk1_START (0)
#define SOC_SYSQIC_IB_PROBE_RREQ_FILTER5_1_rreq_user_msk1_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int rreq_user_msk2 : 32;
    } reg;
} SOC_SYSQIC_IB_PROBE_RREQ_FILTER5_2_UNION;
#endif
#define SOC_SYSQIC_IB_PROBE_RREQ_FILTER5_2_rreq_user_msk2_START (0)
#define SOC_SYSQIC_IB_PROBE_RREQ_FILTER5_2_rreq_user_msk2_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int rreq_user_msk3 : 32;
    } reg;
} SOC_SYSQIC_IB_PROBE_RREQ_FILTER5_3_UNION;
#endif
#define SOC_SYSQIC_IB_PROBE_RREQ_FILTER5_3_rreq_user_msk3_START (0)
#define SOC_SYSQIC_IB_PROBE_RREQ_FILTER5_3_rreq_user_msk3_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int rreq_exclid : 8;
        unsigned int rreq_exclid_msk : 8;
        unsigned int rreq_sf : 8;
        unsigned int rreq_sf_msk : 8;
    } reg;
} SOC_SYSQIC_IB_PROBE_RREQ_FILTER6_UNION;
#endif
#define SOC_SYSQIC_IB_PROBE_RREQ_FILTER6_rreq_exclid_START (0)
#define SOC_SYSQIC_IB_PROBE_RREQ_FILTER6_rreq_exclid_END (7)
#define SOC_SYSQIC_IB_PROBE_RREQ_FILTER6_rreq_exclid_msk_START (8)
#define SOC_SYSQIC_IB_PROBE_RREQ_FILTER6_rreq_exclid_msk_END (15)
#define SOC_SYSQIC_IB_PROBE_RREQ_FILTER6_rreq_sf_START (16)
#define SOC_SYSQIC_IB_PROBE_RREQ_FILTER6_rreq_sf_END (23)
#define SOC_SYSQIC_IB_PROBE_RREQ_FILTER6_rreq_sf_msk_START (24)
#define SOC_SYSQIC_IB_PROBE_RREQ_FILTER6_rreq_sf_msk_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int rreq_qos : 8;
        unsigned int rreq_tid : 8;
        unsigned int rreq_tid_msk : 8;
        unsigned int rreq_lck : 4;
        unsigned int rreq_lck_msk : 4;
    } reg;
} SOC_SYSQIC_IB_PROBE_RREQ_FILTER7_UNION;
#endif
#define SOC_SYSQIC_IB_PROBE_RREQ_FILTER7_rreq_qos_START (0)
#define SOC_SYSQIC_IB_PROBE_RREQ_FILTER7_rreq_qos_END (7)
#define SOC_SYSQIC_IB_PROBE_RREQ_FILTER7_rreq_tid_START (8)
#define SOC_SYSQIC_IB_PROBE_RREQ_FILTER7_rreq_tid_END (15)
#define SOC_SYSQIC_IB_PROBE_RREQ_FILTER7_rreq_tid_msk_START (16)
#define SOC_SYSQIC_IB_PROBE_RREQ_FILTER7_rreq_tid_msk_END (23)
#define SOC_SYSQIC_IB_PROBE_RREQ_FILTER7_rreq_lck_START (24)
#define SOC_SYSQIC_IB_PROBE_RREQ_FILTER7_rreq_lck_END (27)
#define SOC_SYSQIC_IB_PROBE_RREQ_FILTER7_rreq_lck_msk_START (28)
#define SOC_SYSQIC_IB_PROBE_RREQ_FILTER7_rreq_lck_msk_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int rreq_tsize_min : 16;
        unsigned int rreq_tsize_max : 16;
    } reg;
} SOC_SYSQIC_IB_PROBE_RREQ_FILTER8_UNION;
#endif
#define SOC_SYSQIC_IB_PROBE_RREQ_FILTER8_rreq_tsize_min_START (0)
#define SOC_SYSQIC_IB_PROBE_RREQ_FILTER8_rreq_tsize_min_END (15)
#define SOC_SYSQIC_IB_PROBE_RREQ_FILTER8_rreq_tsize_max_START (16)
#define SOC_SYSQIC_IB_PROBE_RREQ_FILTER8_rreq_tsize_max_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int rreq_dstid : 8;
        unsigned int rreq_dstid_msk : 8;
        unsigned int rreq_vcnum : 4;
        unsigned int rreq_vcnum_msk : 4;
        unsigned int rreq_burst : 1;
        unsigned int rreq_burst_msk : 1;
        unsigned int rreq_rd : 1;
        unsigned int rreq_rd_msk : 1;
        unsigned int rreq_splt_cmd : 1;
        unsigned int rreq_splt_cmd_msk : 1;
        unsigned int reserved : 2;
    } reg;
} SOC_SYSQIC_IB_PROBE_RREQ_FILTER9_UNION;
#endif
#define SOC_SYSQIC_IB_PROBE_RREQ_FILTER9_rreq_dstid_START (0)
#define SOC_SYSQIC_IB_PROBE_RREQ_FILTER9_rreq_dstid_END (7)
#define SOC_SYSQIC_IB_PROBE_RREQ_FILTER9_rreq_dstid_msk_START (8)
#define SOC_SYSQIC_IB_PROBE_RREQ_FILTER9_rreq_dstid_msk_END (15)
#define SOC_SYSQIC_IB_PROBE_RREQ_FILTER9_rreq_vcnum_START (16)
#define SOC_SYSQIC_IB_PROBE_RREQ_FILTER9_rreq_vcnum_END (19)
#define SOC_SYSQIC_IB_PROBE_RREQ_FILTER9_rreq_vcnum_msk_START (20)
#define SOC_SYSQIC_IB_PROBE_RREQ_FILTER9_rreq_vcnum_msk_END (23)
#define SOC_SYSQIC_IB_PROBE_RREQ_FILTER9_rreq_burst_START (24)
#define SOC_SYSQIC_IB_PROBE_RREQ_FILTER9_rreq_burst_END (24)
#define SOC_SYSQIC_IB_PROBE_RREQ_FILTER9_rreq_burst_msk_START (25)
#define SOC_SYSQIC_IB_PROBE_RREQ_FILTER9_rreq_burst_msk_END (25)
#define SOC_SYSQIC_IB_PROBE_RREQ_FILTER9_rreq_rd_START (26)
#define SOC_SYSQIC_IB_PROBE_RREQ_FILTER9_rreq_rd_END (26)
#define SOC_SYSQIC_IB_PROBE_RREQ_FILTER9_rreq_rd_msk_START (27)
#define SOC_SYSQIC_IB_PROBE_RREQ_FILTER9_rreq_rd_msk_END (27)
#define SOC_SYSQIC_IB_PROBE_RREQ_FILTER9_rreq_splt_cmd_START (28)
#define SOC_SYSQIC_IB_PROBE_RREQ_FILTER9_rreq_splt_cmd_END (28)
#define SOC_SYSQIC_IB_PROBE_RREQ_FILTER9_rreq_splt_cmd_msk_START (29)
#define SOC_SYSQIC_IB_PROBE_RREQ_FILTER9_rreq_splt_cmd_msk_END (29)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int rreq_stat_cnt_int : 8;
        unsigned int reserved : 24;
    } reg;
} SOC_SYSQIC_IB_PROBE_RREQ_INT_STATUS_UNION;
#endif
#define SOC_SYSQIC_IB_PROBE_RREQ_INT_STATUS_rreq_stat_cnt_int_START (0)
#define SOC_SYSQIC_IB_PROBE_RREQ_INT_STATUS_rreq_stat_cnt_int_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int rreq_stat_cnt_int_clr : 8;
        unsigned int reserved : 24;
    } reg;
} SOC_SYSQIC_IB_PROBE_RREQ_INT_CLR_UNION;
#endif
#define SOC_SYSQIC_IB_PROBE_RREQ_INT_CLR_rreq_stat_cnt_int_clr_START (0)
#define SOC_SYSQIC_IB_PROBE_RREQ_INT_CLR_rreq_stat_cnt_int_clr_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int rreq_stat_cnt : 32;
    } reg;
} SOC_SYSQIC_IB_PROBE_RREQ_CNT_UNION;
#endif
#define SOC_SYSQIC_IB_PROBE_RREQ_CNT_rreq_stat_cnt_START (0)
#define SOC_SYSQIC_IB_PROBE_RREQ_CNT_rreq_stat_cnt_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int rreq_stat_cnt_int_th : 32;
    } reg;
} SOC_SYSQIC_IB_PROBE_RREQ_INT_CFG_UNION;
#endif
#define SOC_SYSQIC_IB_PROBE_RREQ_INT_CFG_rreq_stat_cnt_int_th_START (0)
#define SOC_SYSQIC_IB_PROBE_RREQ_INT_CFG_rreq_stat_cnt_int_th_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int dfx_int_dis : 1;
        unsigned int dfx_int_safety_dis : 1;
        unsigned int safety_r_type_dis : 6;
        unsigned int dfx_int_r_type_dis : 24;
    } reg;
} SOC_SYSQIC_IB_DFX_INT_CTRL0_UNION;
#endif
#define SOC_SYSQIC_IB_DFX_INT_CTRL0_dfx_int_dis_START (0)
#define SOC_SYSQIC_IB_DFX_INT_CTRL0_dfx_int_dis_END (0)
#define SOC_SYSQIC_IB_DFX_INT_CTRL0_dfx_int_safety_dis_START (1)
#define SOC_SYSQIC_IB_DFX_INT_CTRL0_dfx_int_safety_dis_END (1)
#define SOC_SYSQIC_IB_DFX_INT_CTRL0_safety_r_type_dis_START (2)
#define SOC_SYSQIC_IB_DFX_INT_CTRL0_safety_r_type_dis_END (7)
#define SOC_SYSQIC_IB_DFX_INT_CTRL0_dfx_int_r_type_dis_START (8)
#define SOC_SYSQIC_IB_DFX_INT_CTRL0_dfx_int_r_type_dis_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int safety_int_err_dis : 1;
        unsigned int reserved : 1;
        unsigned int safety_w_type_dis : 6;
        unsigned int dfx_int_w_type_dis : 24;
    } reg;
} SOC_SYSQIC_IB_DFX_INT_CTRL1_UNION;
#endif
#define SOC_SYSQIC_IB_DFX_INT_CTRL1_safety_int_err_dis_START (0)
#define SOC_SYSQIC_IB_DFX_INT_CTRL1_safety_int_err_dis_END (0)
#define SOC_SYSQIC_IB_DFX_INT_CTRL1_safety_w_type_dis_START (2)
#define SOC_SYSQIC_IB_DFX_INT_CTRL1_safety_w_type_dis_END (7)
#define SOC_SYSQIC_IB_DFX_INT_CTRL1_dfx_int_w_type_dis_START (8)
#define SOC_SYSQIC_IB_DFX_INT_CTRL1_dfx_int_w_type_dis_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int dfx_int_status : 1;
        unsigned int dfx_int_safety_status : 1;
        unsigned int reserved_0 : 2;
        unsigned int dfx_int_safety_type : 4;
        unsigned int dfx_int_type : 8;
        unsigned int reserved_1 : 16;
    } reg;
} SOC_SYSQIC_IB_DFX_INT_STATUS_UNION;
#endif
#define SOC_SYSQIC_IB_DFX_INT_STATUS_dfx_int_status_START (0)
#define SOC_SYSQIC_IB_DFX_INT_STATUS_dfx_int_status_END (0)
#define SOC_SYSQIC_IB_DFX_INT_STATUS_dfx_int_safety_status_START (1)
#define SOC_SYSQIC_IB_DFX_INT_STATUS_dfx_int_safety_status_END (1)
#define SOC_SYSQIC_IB_DFX_INT_STATUS_dfx_int_safety_type_START (4)
#define SOC_SYSQIC_IB_DFX_INT_STATUS_dfx_int_safety_type_END (7)
#define SOC_SYSQIC_IB_DFX_INT_STATUS_dfx_int_type_START (8)
#define SOC_SYSQIC_IB_DFX_INT_STATUS_dfx_int_type_END (15)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int dfx_int_clr : 1;
        unsigned int dfx_int_safety_clr : 1;
        unsigned int reserved : 30;
    } reg;
} SOC_SYSQIC_IB_DFX_INT_CLR_UNION;
#endif
#define SOC_SYSQIC_IB_DFX_INT_CLR_dfx_int_clr_START (0)
#define SOC_SYSQIC_IB_DFX_INT_CLR_dfx_int_clr_END (0)
#define SOC_SYSQIC_IB_DFX_INT_CLR_dfx_int_safety_clr_START (1)
#define SOC_SYSQIC_IB_DFX_INT_CLR_dfx_int_safety_clr_END (1)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int dfx_err_info0 : 32;
    } reg;
} SOC_SYSQIC_IB_DFX_ERR_INF0_UNION;
#endif
#define SOC_SYSQIC_IB_DFX_ERR_INF0_dfx_err_info0_START (0)
#define SOC_SYSQIC_IB_DFX_ERR_INF0_dfx_err_info0_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int dfx_err_info1 : 32;
    } reg;
} SOC_SYSQIC_IB_DFX_ERR_INF1_UNION;
#endif
#define SOC_SYSQIC_IB_DFX_ERR_INF1_dfx_err_info1_START (0)
#define SOC_SYSQIC_IB_DFX_ERR_INF1_dfx_err_info1_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int dfx_err_info2 : 32;
    } reg;
} SOC_SYSQIC_IB_DFX_ERR_INF2_UNION;
#endif
#define SOC_SYSQIC_IB_DFX_ERR_INF2_dfx_err_info2_START (0)
#define SOC_SYSQIC_IB_DFX_ERR_INF2_dfx_err_info2_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int dfx_err_info3 : 32;
    } reg;
} SOC_SYSQIC_IB_DFX_ERR_INF3_UNION;
#endif
#define SOC_SYSQIC_IB_DFX_ERR_INF3_dfx_err_info3_START (0)
#define SOC_SYSQIC_IB_DFX_ERR_INF3_dfx_err_info3_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int dfx_int_org0 : 32;
    } reg;
} SOC_SYSQIC_IB_DFX_ORG_INT0_UNION;
#endif
#define SOC_SYSQIC_IB_DFX_ORG_INT0_dfx_int_org0_START (0)
#define SOC_SYSQIC_IB_DFX_ORG_INT0_dfx_int_org0_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int dfx_int_org1 : 32;
    } reg;
} SOC_SYSQIC_IB_DFX_ORG_INT1_UNION;
#endif
#define SOC_SYSQIC_IB_DFX_ORG_INT1_dfx_int_org1_START (0)
#define SOC_SYSQIC_IB_DFX_ORG_INT1_dfx_int_org1_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int dfx_safety_int_org : 32;
    } reg;
} SOC_SYSQIC_IB_DFX_SAFETY_ORG_INT_UNION;
#endif
#define SOC_SYSQIC_IB_DFX_SAFETY_ORG_INT_dfx_safety_int_org_START (0)
#define SOC_SYSQIC_IB_DFX_SAFETY_ORG_INT_dfx_safety_int_org_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int dfx_safety_info0 : 32;
    } reg;
} SOC_SYSQIC_IB_DFX_SAFETY_INF0_UNION;
#endif
#define SOC_SYSQIC_IB_DFX_SAFETY_INF0_dfx_safety_info0_START (0)
#define SOC_SYSQIC_IB_DFX_SAFETY_INF0_dfx_safety_info0_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int dfx_safety_info1 : 32;
    } reg;
} SOC_SYSQIC_IB_DFX_SAFETY_INF1_UNION;
#endif
#define SOC_SYSQIC_IB_DFX_SAFETY_INF1_dfx_safety_info1_START (0)
#define SOC_SYSQIC_IB_DFX_SAFETY_INF1_dfx_safety_info1_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int dfx_safety_info2 : 32;
    } reg;
} SOC_SYSQIC_IB_DFX_SAFETY_INF2_UNION;
#endif
#define SOC_SYSQIC_IB_DFX_SAFETY_INF2_dfx_safety_info2_START (0)
#define SOC_SYSQIC_IB_DFX_SAFETY_INF2_dfx_safety_info2_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int dfx_safety_info3 : 32;
    } reg;
} SOC_SYSQIC_IB_DFX_SAFETY_INF3_UNION;
#endif
#define SOC_SYSQIC_IB_DFX_SAFETY_INF3_dfx_safety_info3_START (0)
#define SOC_SYSQIC_IB_DFX_SAFETY_INF3_dfx_safety_info3_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int table_snapshot_src : 2;
        unsigned int reserved_0 : 6;
        unsigned int table_snapshot_sel : 8;
        unsigned int reserved_1 : 16;
    } reg;
} SOC_SYSQIC_IB_DFX_TABLE_CTRL_UNION;
#endif
#define SOC_SYSQIC_IB_DFX_TABLE_CTRL_table_snapshot_src_START (0)
#define SOC_SYSQIC_IB_DFX_TABLE_CTRL_table_snapshot_src_END (1)
#define SOC_SYSQIC_IB_DFX_TABLE_CTRL_table_snapshot_sel_START (8)
#define SOC_SYSQIC_IB_DFX_TABLE_CTRL_table_snapshot_sel_END (15)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int dfx_snapshot_info0 : 32;
    } reg;
} SOC_SYSQIC_IB_DFX_TABLE_INF0_UNION;
#endif
#define SOC_SYSQIC_IB_DFX_TABLE_INF0_dfx_snapshot_info0_START (0)
#define SOC_SYSQIC_IB_DFX_TABLE_INF0_dfx_snapshot_info0_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int dfx_snapshot_info1 : 32;
    } reg;
} SOC_SYSQIC_IB_DFX_TABLE_INF1_UNION;
#endif
#define SOC_SYSQIC_IB_DFX_TABLE_INF1_dfx_snapshot_info1_START (0)
#define SOC_SYSQIC_IB_DFX_TABLE_INF1_dfx_snapshot_info1_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int dfx_snapshot_info2 : 32;
    } reg;
} SOC_SYSQIC_IB_DFX_TABLE_INF2_UNION;
#endif
#define SOC_SYSQIC_IB_DFX_TABLE_INF2_dfx_snapshot_info2_START (0)
#define SOC_SYSQIC_IB_DFX_TABLE_INF2_dfx_snapshot_info2_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int dfx_snapshot_info3 : 32;
    } reg;
} SOC_SYSQIC_IB_DFX_TABLE_INF3_UNION;
#endif
#define SOC_SYSQIC_IB_DFX_TABLE_INF3_dfx_snapshot_info3_START (0)
#define SOC_SYSQIC_IB_DFX_TABLE_INF3_dfx_snapshot_info3_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int soft_lpc_port_sel : 4;
        unsigned int soft_cfg_lpc_sel : 1;
        unsigned int reserved_0 : 3;
        unsigned int soft_cfg_lpc_req_req : 1;
        unsigned int soft_cfg_lpc_rsp_req : 1;
        unsigned int reserved_1 : 6;
        unsigned int soft_lpc_rreq_req : 4;
        unsigned int soft_lpc_wreq_req : 4;
        unsigned int soft_lpc_rrsp_req : 4;
        unsigned int soft_lpc_wrsp_req : 4;
    } reg;
} SOC_SYSQIC_IB_SOFT_LPC_CTRL_UNION;
#endif
#define SOC_SYSQIC_IB_SOFT_LPC_CTRL_soft_lpc_port_sel_START (0)
#define SOC_SYSQIC_IB_SOFT_LPC_CTRL_soft_lpc_port_sel_END (3)
#define SOC_SYSQIC_IB_SOFT_LPC_CTRL_soft_cfg_lpc_sel_START (4)
#define SOC_SYSQIC_IB_SOFT_LPC_CTRL_soft_cfg_lpc_sel_END (4)
#define SOC_SYSQIC_IB_SOFT_LPC_CTRL_soft_cfg_lpc_req_req_START (8)
#define SOC_SYSQIC_IB_SOFT_LPC_CTRL_soft_cfg_lpc_req_req_END (8)
#define SOC_SYSQIC_IB_SOFT_LPC_CTRL_soft_cfg_lpc_rsp_req_START (9)
#define SOC_SYSQIC_IB_SOFT_LPC_CTRL_soft_cfg_lpc_rsp_req_END (9)
#define SOC_SYSQIC_IB_SOFT_LPC_CTRL_soft_lpc_rreq_req_START (16)
#define SOC_SYSQIC_IB_SOFT_LPC_CTRL_soft_lpc_rreq_req_END (19)
#define SOC_SYSQIC_IB_SOFT_LPC_CTRL_soft_lpc_wreq_req_START (20)
#define SOC_SYSQIC_IB_SOFT_LPC_CTRL_soft_lpc_wreq_req_END (23)
#define SOC_SYSQIC_IB_SOFT_LPC_CTRL_soft_lpc_rrsp_req_START (24)
#define SOC_SYSQIC_IB_SOFT_LPC_CTRL_soft_lpc_rrsp_req_END (27)
#define SOC_SYSQIC_IB_SOFT_LPC_CTRL_soft_lpc_wrsp_req_START (28)
#define SOC_SYSQIC_IB_SOFT_LPC_CTRL_soft_lpc_wrsp_req_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int dfx_module_busy : 32;
    } reg;
} SOC_SYSQIC_IB_DFX_MODULE_UNION;
#endif
#define SOC_SYSQIC_IB_DFX_MODULE_dfx_module_busy_START (0)
#define SOC_SYSQIC_IB_DFX_MODULE_dfx_module_busy_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int dfx_bp_st : 32;
    } reg;
} SOC_SYSQIC_IB_DFX_BP_UNION;
#endif
#define SOC_SYSQIC_IB_DFX_BP_dfx_bp_st_START (0)
#define SOC_SYSQIC_IB_DFX_BP_dfx_bp_st_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int dfx_wr_ostd_cnt : 8;
        unsigned int dfx_rd_ostd_cnt : 8;
        unsigned int reserved : 16;
    } reg;
} SOC_SYSQIC_IB_DFX_OT_UNION;
#endif
#define SOC_SYSQIC_IB_DFX_OT_dfx_wr_ostd_cnt_START (0)
#define SOC_SYSQIC_IB_DFX_OT_dfx_wr_ostd_cnt_END (7)
#define SOC_SYSQIC_IB_DFX_OT_dfx_rd_ostd_cnt_START (8)
#define SOC_SYSQIC_IB_DFX_OT_dfx_rd_ostd_cnt_END (15)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int dfx_rct_cmd_cnt : 8;
        unsigned int dfx_rct_buf_cnt : 8;
        unsigned int dfx_wct_cmd_cnt : 8;
        unsigned int dfx_wct_buf_cnt : 8;
    } reg;
} SOC_SYSQIC_IB_DFX_BUF0_UNION;
#endif
#define SOC_SYSQIC_IB_DFX_BUF0_dfx_rct_cmd_cnt_START (0)
#define SOC_SYSQIC_IB_DFX_BUF0_dfx_rct_cmd_cnt_END (7)
#define SOC_SYSQIC_IB_DFX_BUF0_dfx_rct_buf_cnt_START (8)
#define SOC_SYSQIC_IB_DFX_BUF0_dfx_rct_buf_cnt_END (15)
#define SOC_SYSQIC_IB_DFX_BUF0_dfx_wct_cmd_cnt_START (16)
#define SOC_SYSQIC_IB_DFX_BUF0_dfx_wct_cmd_cnt_END (23)
#define SOC_SYSQIC_IB_DFX_BUF0_dfx_wct_buf_cnt_START (24)
#define SOC_SYSQIC_IB_DFX_BUF0_dfx_wct_buf_cnt_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int dfx_rct_dt_buf_cnt : 8;
        unsigned int reserved : 24;
    } reg;
} SOC_SYSQIC_IB_DFX_BUF1_UNION;
#endif
#define SOC_SYSQIC_IB_DFX_BUF1_dfx_rct_dt_buf_cnt_START (0)
#define SOC_SYSQIC_IB_DFX_BUF1_dfx_rct_dt_buf_cnt_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int dfx_lpc_st : 32;
    } reg;
} SOC_SYSQIC_IB_DFX_LPC_UNION;
#endif
#define SOC_SYSQIC_IB_DFX_LPC_dfx_lpc_st_START (0)
#define SOC_SYSQIC_IB_DFX_LPC_dfx_lpc_st_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int dfx_rreq0_vc0_crd_cnt : 8;
        unsigned int dfx_rreq0_vc1_crd_cnt : 8;
        unsigned int dfx_rreq0_vc2_crd_cnt : 8;
        unsigned int dfx_rreq0_vc3_crd_cnt : 8;
    } reg;
} SOC_SYSQIC_IB_DFX_RREQ_TX0_UNION;
#endif
#define SOC_SYSQIC_IB_DFX_RREQ_TX0_dfx_rreq0_vc0_crd_cnt_START (0)
#define SOC_SYSQIC_IB_DFX_RREQ_TX0_dfx_rreq0_vc0_crd_cnt_END (7)
#define SOC_SYSQIC_IB_DFX_RREQ_TX0_dfx_rreq0_vc1_crd_cnt_START (8)
#define SOC_SYSQIC_IB_DFX_RREQ_TX0_dfx_rreq0_vc1_crd_cnt_END (15)
#define SOC_SYSQIC_IB_DFX_RREQ_TX0_dfx_rreq0_vc2_crd_cnt_START (16)
#define SOC_SYSQIC_IB_DFX_RREQ_TX0_dfx_rreq0_vc2_crd_cnt_END (23)
#define SOC_SYSQIC_IB_DFX_RREQ_TX0_dfx_rreq0_vc3_crd_cnt_START (24)
#define SOC_SYSQIC_IB_DFX_RREQ_TX0_dfx_rreq0_vc3_crd_cnt_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int dfx_rreq1_vc0_crd_cnt : 8;
        unsigned int dfx_rreq1_vc1_crd_cnt : 8;
        unsigned int dfx_rreq1_vc2_crd_cnt : 8;
        unsigned int dfx_rreq1_vc3_crd_cnt : 8;
    } reg;
} SOC_SYSQIC_IB_DFX_RREQ_TX1_UNION;
#endif
#define SOC_SYSQIC_IB_DFX_RREQ_TX1_dfx_rreq1_vc0_crd_cnt_START (0)
#define SOC_SYSQIC_IB_DFX_RREQ_TX1_dfx_rreq1_vc0_crd_cnt_END (7)
#define SOC_SYSQIC_IB_DFX_RREQ_TX1_dfx_rreq1_vc1_crd_cnt_START (8)
#define SOC_SYSQIC_IB_DFX_RREQ_TX1_dfx_rreq1_vc1_crd_cnt_END (15)
#define SOC_SYSQIC_IB_DFX_RREQ_TX1_dfx_rreq1_vc2_crd_cnt_START (16)
#define SOC_SYSQIC_IB_DFX_RREQ_TX1_dfx_rreq1_vc2_crd_cnt_END (23)
#define SOC_SYSQIC_IB_DFX_RREQ_TX1_dfx_rreq1_vc3_crd_cnt_START (24)
#define SOC_SYSQIC_IB_DFX_RREQ_TX1_dfx_rreq1_vc3_crd_cnt_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int dfx_rreq2_vc0_crd_cnt : 8;
        unsigned int dfx_rreq2_vc1_crd_cnt : 8;
        unsigned int dfx_rreq2_vc2_crd_cnt : 8;
        unsigned int dfx_rreq2_vc3_crd_cnt : 8;
    } reg;
} SOC_SYSQIC_IB_DFX_RREQ_TX2_UNION;
#endif
#define SOC_SYSQIC_IB_DFX_RREQ_TX2_dfx_rreq2_vc0_crd_cnt_START (0)
#define SOC_SYSQIC_IB_DFX_RREQ_TX2_dfx_rreq2_vc0_crd_cnt_END (7)
#define SOC_SYSQIC_IB_DFX_RREQ_TX2_dfx_rreq2_vc1_crd_cnt_START (8)
#define SOC_SYSQIC_IB_DFX_RREQ_TX2_dfx_rreq2_vc1_crd_cnt_END (15)
#define SOC_SYSQIC_IB_DFX_RREQ_TX2_dfx_rreq2_vc2_crd_cnt_START (16)
#define SOC_SYSQIC_IB_DFX_RREQ_TX2_dfx_rreq2_vc2_crd_cnt_END (23)
#define SOC_SYSQIC_IB_DFX_RREQ_TX2_dfx_rreq2_vc3_crd_cnt_START (24)
#define SOC_SYSQIC_IB_DFX_RREQ_TX2_dfx_rreq2_vc3_crd_cnt_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int dfx_rreq3_vc0_crd_cnt : 8;
        unsigned int dfx_rreq3_vc1_crd_cnt : 8;
        unsigned int dfx_rreq3_vc2_crd_cnt : 8;
        unsigned int dfx_rreq3_vc3_crd_cnt : 8;
    } reg;
} SOC_SYSQIC_IB_DFX_RREQ_TX3_UNION;
#endif
#define SOC_SYSQIC_IB_DFX_RREQ_TX3_dfx_rreq3_vc0_crd_cnt_START (0)
#define SOC_SYSQIC_IB_DFX_RREQ_TX3_dfx_rreq3_vc0_crd_cnt_END (7)
#define SOC_SYSQIC_IB_DFX_RREQ_TX3_dfx_rreq3_vc1_crd_cnt_START (8)
#define SOC_SYSQIC_IB_DFX_RREQ_TX3_dfx_rreq3_vc1_crd_cnt_END (15)
#define SOC_SYSQIC_IB_DFX_RREQ_TX3_dfx_rreq3_vc2_crd_cnt_START (16)
#define SOC_SYSQIC_IB_DFX_RREQ_TX3_dfx_rreq3_vc2_crd_cnt_END (23)
#define SOC_SYSQIC_IB_DFX_RREQ_TX3_dfx_rreq3_vc3_crd_cnt_START (24)
#define SOC_SYSQIC_IB_DFX_RREQ_TX3_dfx_rreq3_vc3_crd_cnt_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int dfx_wreq0_vc0_crd_cnt : 8;
        unsigned int dfx_wreq0_vc1_crd_cnt : 8;
        unsigned int dfx_wreq0_vc2_crd_cnt : 8;
        unsigned int dfx_wreq0_vc3_crd_cnt : 8;
    } reg;
} SOC_SYSQIC_IB_DFX_WREQ_TX0_UNION;
#endif
#define SOC_SYSQIC_IB_DFX_WREQ_TX0_dfx_wreq0_vc0_crd_cnt_START (0)
#define SOC_SYSQIC_IB_DFX_WREQ_TX0_dfx_wreq0_vc0_crd_cnt_END (7)
#define SOC_SYSQIC_IB_DFX_WREQ_TX0_dfx_wreq0_vc1_crd_cnt_START (8)
#define SOC_SYSQIC_IB_DFX_WREQ_TX0_dfx_wreq0_vc1_crd_cnt_END (15)
#define SOC_SYSQIC_IB_DFX_WREQ_TX0_dfx_wreq0_vc2_crd_cnt_START (16)
#define SOC_SYSQIC_IB_DFX_WREQ_TX0_dfx_wreq0_vc2_crd_cnt_END (23)
#define SOC_SYSQIC_IB_DFX_WREQ_TX0_dfx_wreq0_vc3_crd_cnt_START (24)
#define SOC_SYSQIC_IB_DFX_WREQ_TX0_dfx_wreq0_vc3_crd_cnt_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int dfx_wreq1_vc0_crd_cnt : 8;
        unsigned int dfx_wreq1_vc1_crd_cnt : 8;
        unsigned int dfx_wreq1_vc2_crd_cnt : 8;
        unsigned int dfx_wreq1_vc3_crd_cnt : 8;
    } reg;
} SOC_SYSQIC_IB_DFX_WREQ_TX1_UNION;
#endif
#define SOC_SYSQIC_IB_DFX_WREQ_TX1_dfx_wreq1_vc0_crd_cnt_START (0)
#define SOC_SYSQIC_IB_DFX_WREQ_TX1_dfx_wreq1_vc0_crd_cnt_END (7)
#define SOC_SYSQIC_IB_DFX_WREQ_TX1_dfx_wreq1_vc1_crd_cnt_START (8)
#define SOC_SYSQIC_IB_DFX_WREQ_TX1_dfx_wreq1_vc1_crd_cnt_END (15)
#define SOC_SYSQIC_IB_DFX_WREQ_TX1_dfx_wreq1_vc2_crd_cnt_START (16)
#define SOC_SYSQIC_IB_DFX_WREQ_TX1_dfx_wreq1_vc2_crd_cnt_END (23)
#define SOC_SYSQIC_IB_DFX_WREQ_TX1_dfx_wreq1_vc3_crd_cnt_START (24)
#define SOC_SYSQIC_IB_DFX_WREQ_TX1_dfx_wreq1_vc3_crd_cnt_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int dfx_wreq2_vc0_crd_cnt : 8;
        unsigned int dfx_wreq2_vc1_crd_cnt : 8;
        unsigned int dfx_wreq2_vc2_crd_cnt : 8;
        unsigned int dfx_wreq2_vc3_crd_cnt : 8;
    } reg;
} SOC_SYSQIC_IB_DFX_WREQ_TX2_UNION;
#endif
#define SOC_SYSQIC_IB_DFX_WREQ_TX2_dfx_wreq2_vc0_crd_cnt_START (0)
#define SOC_SYSQIC_IB_DFX_WREQ_TX2_dfx_wreq2_vc0_crd_cnt_END (7)
#define SOC_SYSQIC_IB_DFX_WREQ_TX2_dfx_wreq2_vc1_crd_cnt_START (8)
#define SOC_SYSQIC_IB_DFX_WREQ_TX2_dfx_wreq2_vc1_crd_cnt_END (15)
#define SOC_SYSQIC_IB_DFX_WREQ_TX2_dfx_wreq2_vc2_crd_cnt_START (16)
#define SOC_SYSQIC_IB_DFX_WREQ_TX2_dfx_wreq2_vc2_crd_cnt_END (23)
#define SOC_SYSQIC_IB_DFX_WREQ_TX2_dfx_wreq2_vc3_crd_cnt_START (24)
#define SOC_SYSQIC_IB_DFX_WREQ_TX2_dfx_wreq2_vc3_crd_cnt_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int dfx_wreq3_vc0_crd_cnt : 8;
        unsigned int dfx_wreq3_vc1_crd_cnt : 8;
        unsigned int dfx_wreq3_vc2_crd_cnt : 8;
        unsigned int dfx_wreq3_vc3_crd_cnt : 8;
    } reg;
} SOC_SYSQIC_IB_DFX_WREQ_TX3_UNION;
#endif
#define SOC_SYSQIC_IB_DFX_WREQ_TX3_dfx_wreq3_vc0_crd_cnt_START (0)
#define SOC_SYSQIC_IB_DFX_WREQ_TX3_dfx_wreq3_vc0_crd_cnt_END (7)
#define SOC_SYSQIC_IB_DFX_WREQ_TX3_dfx_wreq3_vc1_crd_cnt_START (8)
#define SOC_SYSQIC_IB_DFX_WREQ_TX3_dfx_wreq3_vc1_crd_cnt_END (15)
#define SOC_SYSQIC_IB_DFX_WREQ_TX3_dfx_wreq3_vc2_crd_cnt_START (16)
#define SOC_SYSQIC_IB_DFX_WREQ_TX3_dfx_wreq3_vc2_crd_cnt_END (23)
#define SOC_SYSQIC_IB_DFX_WREQ_TX3_dfx_wreq3_vc3_crd_cnt_START (24)
#define SOC_SYSQIC_IB_DFX_WREQ_TX3_dfx_wreq3_vc3_crd_cnt_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int dfx_rrsp0_fifo_st : 2;
        unsigned int dfx_wrsp0_fifo_st : 2;
        unsigned int dfx_rrsp1_fifo_st : 2;
        unsigned int dfx_wrsp1_fifo_st : 2;
        unsigned int dfx_rrsp2_fifo_st : 2;
        unsigned int dfx_wrsp2_fifo_st : 2;
        unsigned int dfx_rrsp3_fifo_st : 2;
        unsigned int dfx_wrsp3_fifo_st : 2;
        unsigned int reserved : 16;
    } reg;
} SOC_SYSQIC_IB_DFX_RSP_RX_UNION;
#endif
#define SOC_SYSQIC_IB_DFX_RSP_RX_dfx_rrsp0_fifo_st_START (0)
#define SOC_SYSQIC_IB_DFX_RSP_RX_dfx_rrsp0_fifo_st_END (1)
#define SOC_SYSQIC_IB_DFX_RSP_RX_dfx_wrsp0_fifo_st_START (2)
#define SOC_SYSQIC_IB_DFX_RSP_RX_dfx_wrsp0_fifo_st_END (3)
#define SOC_SYSQIC_IB_DFX_RSP_RX_dfx_rrsp1_fifo_st_START (4)
#define SOC_SYSQIC_IB_DFX_RSP_RX_dfx_rrsp1_fifo_st_END (5)
#define SOC_SYSQIC_IB_DFX_RSP_RX_dfx_wrsp1_fifo_st_START (6)
#define SOC_SYSQIC_IB_DFX_RSP_RX_dfx_wrsp1_fifo_st_END (7)
#define SOC_SYSQIC_IB_DFX_RSP_RX_dfx_rrsp2_fifo_st_START (8)
#define SOC_SYSQIC_IB_DFX_RSP_RX_dfx_rrsp2_fifo_st_END (9)
#define SOC_SYSQIC_IB_DFX_RSP_RX_dfx_wrsp2_fifo_st_START (10)
#define SOC_SYSQIC_IB_DFX_RSP_RX_dfx_wrsp2_fifo_st_END (11)
#define SOC_SYSQIC_IB_DFX_RSP_RX_dfx_rrsp3_fifo_st_START (12)
#define SOC_SYSQIC_IB_DFX_RSP_RX_dfx_rrsp3_fifo_st_END (13)
#define SOC_SYSQIC_IB_DFX_RSP_RX_dfx_wrsp3_fifo_st_START (14)
#define SOC_SYSQIC_IB_DFX_RSP_RX_dfx_wrsp3_fifo_st_END (15)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int dfx_cfg_req_crd_cnt : 8;
        unsigned int reserved_0 : 8;
        unsigned int dfx_cfg_rsp_fifo_st : 2;
        unsigned int reserved_1 : 14;
    } reg;
} SOC_SYSQIC_IB_DFX_CFG_PORT_UNION;
#endif
#define SOC_SYSQIC_IB_DFX_CFG_PORT_dfx_cfg_req_crd_cnt_START (0)
#define SOC_SYSQIC_IB_DFX_CFG_PORT_dfx_cfg_req_crd_cnt_END (7)
#define SOC_SYSQIC_IB_DFX_CFG_PORT_dfx_cfg_rsp_fifo_st_START (16)
#define SOC_SYSQIC_IB_DFX_CFG_PORT_dfx_cfg_rsp_fifo_st_END (17)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int dfx_rct_timeout0 : 32;
    } reg;
} SOC_SYSQIC_IB_DFX_RCT_TIMEOUT0_UNION;
#endif
#define SOC_SYSQIC_IB_DFX_RCT_TIMEOUT0_dfx_rct_timeout0_START (0)
#define SOC_SYSQIC_IB_DFX_RCT_TIMEOUT0_dfx_rct_timeout0_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int dfx_rct_timeout1 : 32;
    } reg;
} SOC_SYSQIC_IB_DFX_RCT_TIMEOUT1_UNION;
#endif
#define SOC_SYSQIC_IB_DFX_RCT_TIMEOUT1_dfx_rct_timeout1_START (0)
#define SOC_SYSQIC_IB_DFX_RCT_TIMEOUT1_dfx_rct_timeout1_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int dfx_rct_timeout2 : 32;
    } reg;
} SOC_SYSQIC_IB_DFX_RCT_TIMEOUT2_UNION;
#endif
#define SOC_SYSQIC_IB_DFX_RCT_TIMEOUT2_dfx_rct_timeout2_START (0)
#define SOC_SYSQIC_IB_DFX_RCT_TIMEOUT2_dfx_rct_timeout2_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int dfx_rct_timeout3 : 32;
    } reg;
} SOC_SYSQIC_IB_DFX_RCT_TIMEOUT3_UNION;
#endif
#define SOC_SYSQIC_IB_DFX_RCT_TIMEOUT3_dfx_rct_timeout3_START (0)
#define SOC_SYSQIC_IB_DFX_RCT_TIMEOUT3_dfx_rct_timeout3_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int dfx_wct_timeout0 : 32;
    } reg;
} SOC_SYSQIC_IB_DFX_WCT_TIMEOUT0_UNION;
#endif
#define SOC_SYSQIC_IB_DFX_WCT_TIMEOUT0_dfx_wct_timeout0_START (0)
#define SOC_SYSQIC_IB_DFX_WCT_TIMEOUT0_dfx_wct_timeout0_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int dfx_wct_timeout1 : 32;
    } reg;
} SOC_SYSQIC_IB_DFX_WCT_TIMEOUT1_UNION;
#endif
#define SOC_SYSQIC_IB_DFX_WCT_TIMEOUT1_dfx_wct_timeout1_START (0)
#define SOC_SYSQIC_IB_DFX_WCT_TIMEOUT1_dfx_wct_timeout1_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int dfx_wct_timeout2 : 32;
    } reg;
} SOC_SYSQIC_IB_DFX_WCT_TIMEOUT2_UNION;
#endif
#define SOC_SYSQIC_IB_DFX_WCT_TIMEOUT2_dfx_wct_timeout2_START (0)
#define SOC_SYSQIC_IB_DFX_WCT_TIMEOUT2_dfx_wct_timeout2_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int dfx_wct_timeout3 : 32;
    } reg;
} SOC_SYSQIC_IB_DFX_WCT_TIMEOUT3_UNION;
#endif
#define SOC_SYSQIC_IB_DFX_WCT_TIMEOUT3_dfx_wct_timeout3_START (0)
#define SOC_SYSQIC_IB_DFX_WCT_TIMEOUT3_dfx_wct_timeout3_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int dfx_rct_lock0 : 32;
    } reg;
} SOC_SYSQIC_IB_DFX_RCT_LOCK0_UNION;
#endif
#define SOC_SYSQIC_IB_DFX_RCT_LOCK0_dfx_rct_lock0_START (0)
#define SOC_SYSQIC_IB_DFX_RCT_LOCK0_dfx_rct_lock0_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int dfx_rct_lock1 : 32;
    } reg;
} SOC_SYSQIC_IB_DFX_RCT_LOCK1_UNION;
#endif
#define SOC_SYSQIC_IB_DFX_RCT_LOCK1_dfx_rct_lock1_START (0)
#define SOC_SYSQIC_IB_DFX_RCT_LOCK1_dfx_rct_lock1_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int dfx_rct_lock2 : 32;
    } reg;
} SOC_SYSQIC_IB_DFX_RCT_LOCK2_UNION;
#endif
#define SOC_SYSQIC_IB_DFX_RCT_LOCK2_dfx_rct_lock2_START (0)
#define SOC_SYSQIC_IB_DFX_RCT_LOCK2_dfx_rct_lock2_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int dfx_rct_lock3 : 32;
    } reg;
} SOC_SYSQIC_IB_DFX_RCT_LOCK3_UNION;
#endif
#define SOC_SYSQIC_IB_DFX_RCT_LOCK3_dfx_rct_lock3_START (0)
#define SOC_SYSQIC_IB_DFX_RCT_LOCK3_dfx_rct_lock3_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int dfx_wct_lock0 : 32;
    } reg;
} SOC_SYSQIC_IB_DFX_WCT_LOCK0_UNION;
#endif
#define SOC_SYSQIC_IB_DFX_WCT_LOCK0_dfx_wct_lock0_START (0)
#define SOC_SYSQIC_IB_DFX_WCT_LOCK0_dfx_wct_lock0_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int dfx_wct_lock1 : 32;
    } reg;
} SOC_SYSQIC_IB_DFX_WCT_LOCK1_UNION;
#endif
#define SOC_SYSQIC_IB_DFX_WCT_LOCK1_dfx_wct_lock1_START (0)
#define SOC_SYSQIC_IB_DFX_WCT_LOCK1_dfx_wct_lock1_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int dfx_wct_lock2 : 32;
    } reg;
} SOC_SYSQIC_IB_DFX_WCT_LOCK2_UNION;
#endif
#define SOC_SYSQIC_IB_DFX_WCT_LOCK2_dfx_wct_lock2_START (0)
#define SOC_SYSQIC_IB_DFX_WCT_LOCK2_dfx_wct_lock2_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int dfx_wct_lock3 : 32;
    } reg;
} SOC_SYSQIC_IB_DFX_WCT_LOCK3_UNION;
#endif
#define SOC_SYSQIC_IB_DFX_WCT_LOCK3_dfx_wct_lock3_START (0)
#define SOC_SYSQIC_IB_DFX_WCT_LOCK3_dfx_wct_lock3_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int dfx_ecc_cnt : 8;
        unsigned int reserved : 24;
    } reg;
} SOC_SYSQIC_IB_DFX_ECC_UNION;
#endif
#define SOC_SYSQIC_IB_DFX_ECC_dfx_ecc_cnt_START (0)
#define SOC_SYSQIC_IB_DFX_ECC_dfx_ecc_cnt_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int dfx_fifo_st0 : 32;
    } reg;
} SOC_SYSQIC_IB_DFX_FIFO0_UNION;
#endif
#define SOC_SYSQIC_IB_DFX_FIFO0_dfx_fifo_st0_START (0)
#define SOC_SYSQIC_IB_DFX_FIFO0_dfx_fifo_st0_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int dfx_ext_ctrl_st : 32;
    } reg;
} SOC_SYSQIC_IB_DFX_EXT_CTRL_UNION;
#endif
#define SOC_SYSQIC_IB_DFX_EXT_CTRL_dfx_ext_ctrl_st_START (0)
#define SOC_SYSQIC_IB_DFX_EXT_CTRL_dfx_ext_ctrl_st_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int rt_info : 32;
    } reg;
} SOC_SYSQIC_RT_INFO_UNION;
#endif
#define SOC_SYSQIC_RT_INFO_rt_info_START (0)
#define SOC_SYSQIC_RT_INFO_rt_info_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int rt_reserved_rw : 32;
    } reg;
} SOC_SYSQIC_RT_RESERVED_RW_UNION;
#endif
#define SOC_SYSQIC_RT_RESERVED_RW_rt_reserved_rw_START (0)
#define SOC_SYSQIC_RT_RESERVED_RW_rt_reserved_rw_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int rt_reserved_ro : 32;
    } reg;
} SOC_SYSQIC_RT_RESERVED_RO_UNION;
#endif
#define SOC_SYSQIC_RT_RESERVED_RO_rt_reserved_ro_START (0)
#define SOC_SYSQIC_RT_RESERVED_RO_rt_reserved_ro_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int ckg_byps_inp : 8;
        unsigned int ckg_byps_outp : 8;
        unsigned int ckg_byps_nmg : 1;
        unsigned int ckg_byps_cfc : 1;
        unsigned int reserved : 14;
    } reg;
} SOC_SYSQIC_RT_CKG_BYPS_UNION;
#endif
#define SOC_SYSQIC_RT_CKG_BYPS_ckg_byps_inp_START (0)
#define SOC_SYSQIC_RT_CKG_BYPS_ckg_byps_inp_END (7)
#define SOC_SYSQIC_RT_CKG_BYPS_ckg_byps_outp_START (8)
#define SOC_SYSQIC_RT_CKG_BYPS_ckg_byps_outp_END (15)
#define SOC_SYSQIC_RT_CKG_BYPS_ckg_byps_nmg_START (16)
#define SOC_SYSQIC_RT_CKG_BYPS_ckg_byps_nmg_END (16)
#define SOC_SYSQIC_RT_CKG_BYPS_ckg_byps_cfc_START (17)
#define SOC_SYSQIC_RT_CKG_BYPS_ckg_byps_cfc_END (17)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int dfx_inblk_busy : 8;
        unsigned int dfx_outblk_busy : 8;
        unsigned int dfx_rt_busy : 1;
        unsigned int reserved : 15;
    } reg;
} SOC_SYSQIC_RT_GLB_ST_UNION;
#endif
#define SOC_SYSQIC_RT_GLB_ST_dfx_inblk_busy_START (0)
#define SOC_SYSQIC_RT_GLB_ST_dfx_inblk_busy_END (7)
#define SOC_SYSQIC_RT_GLB_ST_dfx_outblk_busy_START (8)
#define SOC_SYSQIC_RT_GLB_ST_dfx_outblk_busy_END (15)
#define SOC_SYSQIC_RT_GLB_ST_dfx_rt_busy_START (16)
#define SOC_SYSQIC_RT_GLB_ST_dfx_rt_busy_END (16)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int dfx_inp_err_st : 8;
        unsigned int reserved : 24;
    } reg;
} SOC_SYSQIC_RT_GLB_INT_ST_UNION;
#endif
#define SOC_SYSQIC_RT_GLB_INT_ST_dfx_inp_err_st_START (0)
#define SOC_SYSQIC_RT_GLB_INT_ST_dfx_inp_err_st_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int buf_urg_lvl : 8;
        unsigned int sch_switch_lvl : 8;
        unsigned int reserved : 16;
    } reg;
} SOC_SYSQIC_RT_TIMEOUT_CTRL_UNION;
#endif
#define SOC_SYSQIC_RT_TIMEOUT_CTRL_buf_urg_lvl_START (0)
#define SOC_SYSQIC_RT_TIMEOUT_CTRL_buf_urg_lvl_END (7)
#define SOC_SYSQIC_RT_TIMEOUT_CTRL_sch_switch_lvl_START (8)
#define SOC_SYSQIC_RT_TIMEOUT_CTRL_sch_switch_lvl_END (15)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int prescaler : 3;
        unsigned int reserved_0 : 5;
        unsigned int sat_en : 1;
        unsigned int sat_clr : 1;
        unsigned int reserved_1 : 2;
        unsigned int max_lat_vc : 4;
        unsigned int max_lat_inp : 4;
        unsigned int max_lat_outp : 4;
        unsigned int reserved_2 : 8;
    } reg;
} SOC_SYSQIC_RT_MAX_LAT_CFG_UNION;
#endif
#define SOC_SYSQIC_RT_MAX_LAT_CFG_prescaler_START (0)
#define SOC_SYSQIC_RT_MAX_LAT_CFG_prescaler_END (2)
#define SOC_SYSQIC_RT_MAX_LAT_CFG_sat_en_START (8)
#define SOC_SYSQIC_RT_MAX_LAT_CFG_sat_en_END (8)
#define SOC_SYSQIC_RT_MAX_LAT_CFG_sat_clr_START (9)
#define SOC_SYSQIC_RT_MAX_LAT_CFG_sat_clr_END (9)
#define SOC_SYSQIC_RT_MAX_LAT_CFG_max_lat_vc_START (12)
#define SOC_SYSQIC_RT_MAX_LAT_CFG_max_lat_vc_END (15)
#define SOC_SYSQIC_RT_MAX_LAT_CFG_max_lat_inp_START (16)
#define SOC_SYSQIC_RT_MAX_LAT_CFG_max_lat_inp_END (19)
#define SOC_SYSQIC_RT_MAX_LAT_CFG_max_lat_outp_START (20)
#define SOC_SYSQIC_RT_MAX_LAT_CFG_max_lat_outp_END (23)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int dfx_bp_cnt : 16;
        unsigned int dfx_sat_cnt : 16;
    } reg;
} SOC_SYSQIC_RT_MAX_LAT_ST_UNION;
#endif
#define SOC_SYSQIC_RT_MAX_LAT_ST_dfx_bp_cnt_START (0)
#define SOC_SYSQIC_RT_MAX_LAT_ST_dfx_bp_cnt_END (15)
#define SOC_SYSQIC_RT_MAX_LAT_ST_dfx_sat_cnt_START (16)
#define SOC_SYSQIC_RT_MAX_LAT_ST_dfx_sat_cnt_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int inp_lpc_ctrl : 8;
        unsigned int inp_lpc_req_sft : 8;
        unsigned int outp_lpc_ctrl : 8;
        unsigned int outp_lpc_req_sft : 8;
    } reg;
} SOC_SYSQIC_RT_LPC_CTRL_UNION;
#endif
#define SOC_SYSQIC_RT_LPC_CTRL_inp_lpc_ctrl_START (0)
#define SOC_SYSQIC_RT_LPC_CTRL_inp_lpc_ctrl_END (7)
#define SOC_SYSQIC_RT_LPC_CTRL_inp_lpc_req_sft_START (8)
#define SOC_SYSQIC_RT_LPC_CTRL_inp_lpc_req_sft_END (15)
#define SOC_SYSQIC_RT_LPC_CTRL_outp_lpc_ctrl_START (16)
#define SOC_SYSQIC_RT_LPC_CTRL_outp_lpc_ctrl_END (23)
#define SOC_SYSQIC_RT_LPC_CTRL_outp_lpc_req_sft_START (24)
#define SOC_SYSQIC_RT_LPC_CTRL_outp_lpc_req_sft_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int dfx_inp_lpc_req : 8;
        unsigned int dfx_inp_lpc_ack : 8;
        unsigned int dfx_outp_lpc_req : 8;
        unsigned int dfx_outp_lpc_ack : 8;
    } reg;
} SOC_SYSQIC_RT_LPC_ST_UNION;
#endif
#define SOC_SYSQIC_RT_LPC_ST_dfx_inp_lpc_req_START (0)
#define SOC_SYSQIC_RT_LPC_ST_dfx_inp_lpc_req_END (7)
#define SOC_SYSQIC_RT_LPC_ST_dfx_inp_lpc_ack_START (8)
#define SOC_SYSQIC_RT_LPC_ST_dfx_inp_lpc_ack_END (15)
#define SOC_SYSQIC_RT_LPC_ST_dfx_outp_lpc_req_START (16)
#define SOC_SYSQIC_RT_LPC_ST_dfx_outp_lpc_req_END (23)
#define SOC_SYSQIC_RT_LPC_ST_dfx_outp_lpc_ack_START (24)
#define SOC_SYSQIC_RT_LPC_ST_dfx_outp_lpc_ack_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int inp_byps_en : 1;
        unsigned int reserved : 31;
    } reg;
} SOC_SYSQIC_RT_FUC_BYPS_UNION;
#endif
#define SOC_SYSQIC_RT_FUC_BYPS_inp_byps_en_START (0)
#define SOC_SYSQIC_RT_FUC_BYPS_inp_byps_en_END (0)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int inp_vc_stfw_en : 4;
        unsigned int reserved : 28;
    } reg;
} SOC_SYSQIC_RT_INPORT_VC_CTRL_UNION;
#endif
#define SOC_SYSQIC_RT_INPORT_VC_CTRL_inp_vc_stfw_en_START (0)
#define SOC_SYSQIC_RT_INPORT_VC_CTRL_inp_vc_stfw_en_END (3)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int dfx_vcfifo_st : 8;
        unsigned int reserved : 24;
    } reg;
} SOC_SYSQIC_RT_INPORT_DFX_ST0_UNION;
#endif
#define SOC_SYSQIC_RT_INPORT_DFX_ST0_dfx_vcfifo_st_START (0)
#define SOC_SYSQIC_RT_INPORT_DFX_ST0_dfx_vcfifo_st_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int dfx_vc_acq : 32;
    } reg;
} SOC_SYSQIC_RT_INPORT_DFX_ST1_UNION;
#endif
#define SOC_SYSQIC_RT_INPORT_DFX_ST1_dfx_vc_acq_START (0)
#define SOC_SYSQIC_RT_INPORT_DFX_ST1_dfx_vc_acq_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int dfx_outp0_arbfifo_st : 8;
        unsigned int dfx_outp1_arbfifo_st : 8;
        unsigned int dfx_outp2_arbfifo_st : 8;
        unsigned int dfx_outp3_arbfifo_st : 8;
    } reg;
} SOC_SYSQIC_RT_INPORT_DFX_ST2_UNION;
#endif
#define SOC_SYSQIC_RT_INPORT_DFX_ST2_dfx_outp0_arbfifo_st_START (0)
#define SOC_SYSQIC_RT_INPORT_DFX_ST2_dfx_outp0_arbfifo_st_END (7)
#define SOC_SYSQIC_RT_INPORT_DFX_ST2_dfx_outp1_arbfifo_st_START (8)
#define SOC_SYSQIC_RT_INPORT_DFX_ST2_dfx_outp1_arbfifo_st_END (15)
#define SOC_SYSQIC_RT_INPORT_DFX_ST2_dfx_outp2_arbfifo_st_START (16)
#define SOC_SYSQIC_RT_INPORT_DFX_ST2_dfx_outp2_arbfifo_st_END (23)
#define SOC_SYSQIC_RT_INPORT_DFX_ST2_dfx_outp3_arbfifo_st_START (24)
#define SOC_SYSQIC_RT_INPORT_DFX_ST2_dfx_outp3_arbfifo_st_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int dfx_outp4_arbfifo_st : 8;
        unsigned int dfx_outp5_arbfifo_st : 8;
        unsigned int dfx_outp6_arbfifo_st : 8;
        unsigned int dfx_outp7_arbfifo_st : 8;
    } reg;
} SOC_SYSQIC_RT_INPORT_DFX_ST3_UNION;
#endif
#define SOC_SYSQIC_RT_INPORT_DFX_ST3_dfx_outp4_arbfifo_st_START (0)
#define SOC_SYSQIC_RT_INPORT_DFX_ST3_dfx_outp4_arbfifo_st_END (7)
#define SOC_SYSQIC_RT_INPORT_DFX_ST3_dfx_outp5_arbfifo_st_START (8)
#define SOC_SYSQIC_RT_INPORT_DFX_ST3_dfx_outp5_arbfifo_st_END (15)
#define SOC_SYSQIC_RT_INPORT_DFX_ST3_dfx_outp6_arbfifo_st_START (16)
#define SOC_SYSQIC_RT_INPORT_DFX_ST3_dfx_outp6_arbfifo_st_END (23)
#define SOC_SYSQIC_RT_INPORT_DFX_ST3_dfx_outp7_arbfifo_st_START (24)
#define SOC_SYSQIC_RT_INPORT_DFX_ST3_dfx_outp7_arbfifo_st_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int dfx_rop_unq_err_st : 1;
        unsigned int dfx_did_unq_err_st : 1;
        unsigned int dfx_i2o_access_err_st : 1;
        unsigned int dfx_vc_access_err_st : 1;
        unsigned int dfx_vc_hop_err_st : 1;
        unsigned int dfx_vc_top_err_st : 1;
        unsigned int dfx_vc_hop_top_err_st : 1;
        unsigned int dfx_did_ovfl_st : 1;
        unsigned int dfx_rop_ovfl_st : 1;
        unsigned int reserved : 23;
    } reg;
} SOC_SYSQIC_RT_INPORT_INT_ST_UNION;
#endif
#define SOC_SYSQIC_RT_INPORT_INT_ST_dfx_rop_unq_err_st_START (0)
#define SOC_SYSQIC_RT_INPORT_INT_ST_dfx_rop_unq_err_st_END (0)
#define SOC_SYSQIC_RT_INPORT_INT_ST_dfx_did_unq_err_st_START (1)
#define SOC_SYSQIC_RT_INPORT_INT_ST_dfx_did_unq_err_st_END (1)
#define SOC_SYSQIC_RT_INPORT_INT_ST_dfx_i2o_access_err_st_START (2)
#define SOC_SYSQIC_RT_INPORT_INT_ST_dfx_i2o_access_err_st_END (2)
#define SOC_SYSQIC_RT_INPORT_INT_ST_dfx_vc_access_err_st_START (3)
#define SOC_SYSQIC_RT_INPORT_INT_ST_dfx_vc_access_err_st_END (3)
#define SOC_SYSQIC_RT_INPORT_INT_ST_dfx_vc_hop_err_st_START (4)
#define SOC_SYSQIC_RT_INPORT_INT_ST_dfx_vc_hop_err_st_END (4)
#define SOC_SYSQIC_RT_INPORT_INT_ST_dfx_vc_top_err_st_START (5)
#define SOC_SYSQIC_RT_INPORT_INT_ST_dfx_vc_top_err_st_END (5)
#define SOC_SYSQIC_RT_INPORT_INT_ST_dfx_vc_hop_top_err_st_START (6)
#define SOC_SYSQIC_RT_INPORT_INT_ST_dfx_vc_hop_top_err_st_END (6)
#define SOC_SYSQIC_RT_INPORT_INT_ST_dfx_did_ovfl_st_START (7)
#define SOC_SYSQIC_RT_INPORT_INT_ST_dfx_did_ovfl_st_END (7)
#define SOC_SYSQIC_RT_INPORT_INT_ST_dfx_rop_ovfl_st_START (8)
#define SOC_SYSQIC_RT_INPORT_INT_ST_dfx_rop_ovfl_st_END (8)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int dfx_rop_unq_err_ini : 1;
        unsigned int dfx_did_unq_err_ini : 1;
        unsigned int dfx_i2o_access_err_ini : 1;
        unsigned int dfx_vc_access_err_ini : 1;
        unsigned int dfx_vc_hop_err_ini : 1;
        unsigned int dfx_vc_top_err_ini : 1;
        unsigned int dfx_vc_hop_top_err_ini : 1;
        unsigned int dfx_did_ovfl_ini : 1;
        unsigned int dfx_rop_ovfl_ini : 1;
        unsigned int reserved : 23;
    } reg;
} SOC_SYSQIC_RT_INPORT_INT_INI_UNION;
#endif
#define SOC_SYSQIC_RT_INPORT_INT_INI_dfx_rop_unq_err_ini_START (0)
#define SOC_SYSQIC_RT_INPORT_INT_INI_dfx_rop_unq_err_ini_END (0)
#define SOC_SYSQIC_RT_INPORT_INT_INI_dfx_did_unq_err_ini_START (1)
#define SOC_SYSQIC_RT_INPORT_INT_INI_dfx_did_unq_err_ini_END (1)
#define SOC_SYSQIC_RT_INPORT_INT_INI_dfx_i2o_access_err_ini_START (2)
#define SOC_SYSQIC_RT_INPORT_INT_INI_dfx_i2o_access_err_ini_END (2)
#define SOC_SYSQIC_RT_INPORT_INT_INI_dfx_vc_access_err_ini_START (3)
#define SOC_SYSQIC_RT_INPORT_INT_INI_dfx_vc_access_err_ini_END (3)
#define SOC_SYSQIC_RT_INPORT_INT_INI_dfx_vc_hop_err_ini_START (4)
#define SOC_SYSQIC_RT_INPORT_INT_INI_dfx_vc_hop_err_ini_END (4)
#define SOC_SYSQIC_RT_INPORT_INT_INI_dfx_vc_top_err_ini_START (5)
#define SOC_SYSQIC_RT_INPORT_INT_INI_dfx_vc_top_err_ini_END (5)
#define SOC_SYSQIC_RT_INPORT_INT_INI_dfx_vc_hop_top_err_ini_START (6)
#define SOC_SYSQIC_RT_INPORT_INT_INI_dfx_vc_hop_top_err_ini_END (6)
#define SOC_SYSQIC_RT_INPORT_INT_INI_dfx_did_ovfl_ini_START (7)
#define SOC_SYSQIC_RT_INPORT_INT_INI_dfx_did_ovfl_ini_END (7)
#define SOC_SYSQIC_RT_INPORT_INT_INI_dfx_rop_ovfl_ini_START (8)
#define SOC_SYSQIC_RT_INPORT_INT_INI_dfx_rop_ovfl_ini_END (8)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int dfx_err_clr : 1;
        unsigned int reserved : 31;
    } reg;
} SOC_SYSQIC_RT_INPORT_INT_CLEAR_UNION;
#endif
#define SOC_SYSQIC_RT_INPORT_INT_CLEAR_dfx_err_clr_START (0)
#define SOC_SYSQIC_RT_INPORT_INT_CLEAR_dfx_err_clr_END (0)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int dfx_rop_unq_err_en : 1;
        unsigned int dfx_did_unq_err_en : 1;
        unsigned int dfx_i2o_access_err_en : 1;
        unsigned int dfx_vc_access_err_en : 1;
        unsigned int dfx_vc_hop_err_en : 1;
        unsigned int dfx_vc_top_err_en : 1;
        unsigned int dfx_vc_hop_top_err_en : 1;
        unsigned int dfx_did_ovfl_en : 1;
        unsigned int dfx_rop_ovfl_en : 1;
        unsigned int reserved : 23;
    } reg;
} SOC_SYSQIC_RT_INPORT_INT_EN_UNION;
#endif
#define SOC_SYSQIC_RT_INPORT_INT_EN_dfx_rop_unq_err_en_START (0)
#define SOC_SYSQIC_RT_INPORT_INT_EN_dfx_rop_unq_err_en_END (0)
#define SOC_SYSQIC_RT_INPORT_INT_EN_dfx_did_unq_err_en_START (1)
#define SOC_SYSQIC_RT_INPORT_INT_EN_dfx_did_unq_err_en_END (1)
#define SOC_SYSQIC_RT_INPORT_INT_EN_dfx_i2o_access_err_en_START (2)
#define SOC_SYSQIC_RT_INPORT_INT_EN_dfx_i2o_access_err_en_END (2)
#define SOC_SYSQIC_RT_INPORT_INT_EN_dfx_vc_access_err_en_START (3)
#define SOC_SYSQIC_RT_INPORT_INT_EN_dfx_vc_access_err_en_END (3)
#define SOC_SYSQIC_RT_INPORT_INT_EN_dfx_vc_hop_err_en_START (4)
#define SOC_SYSQIC_RT_INPORT_INT_EN_dfx_vc_hop_err_en_END (4)
#define SOC_SYSQIC_RT_INPORT_INT_EN_dfx_vc_top_err_en_START (5)
#define SOC_SYSQIC_RT_INPORT_INT_EN_dfx_vc_top_err_en_END (5)
#define SOC_SYSQIC_RT_INPORT_INT_EN_dfx_vc_hop_top_err_en_START (6)
#define SOC_SYSQIC_RT_INPORT_INT_EN_dfx_vc_hop_top_err_en_END (6)
#define SOC_SYSQIC_RT_INPORT_INT_EN_dfx_did_ovfl_en_START (7)
#define SOC_SYSQIC_RT_INPORT_INT_EN_dfx_did_ovfl_en_END (7)
#define SOC_SYSQIC_RT_INPORT_INT_EN_dfx_rop_ovfl_en_START (8)
#define SOC_SYSQIC_RT_INPORT_INT_EN_dfx_rop_ovfl_en_END (8)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int dfx_vc_req : 32;
    } reg;
} SOC_SYSQIC_RT_INPORT_DFX_ST4_UNION;
#endif
#define SOC_SYSQIC_RT_INPORT_DFX_ST4_dfx_vc_req_START (0)
#define SOC_SYSQIC_RT_INPORT_DFX_ST4_dfx_vc_req_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int dfx_rop_unq_err_snap : 1;
        unsigned int dfx_did_unq_err_snap : 1;
        unsigned int dfx_i2o_access_err_snap : 1;
        unsigned int dfx_vc_access_err_snap : 1;
        unsigned int dfx_vc_hop_err_snap : 1;
        unsigned int dfx_vc_top_err_snap : 1;
        unsigned int dfx_vc_hop_top_err_snap : 1;
        unsigned int dfx_did_ovfl_snap : 1;
        unsigned int dfx_rop_ovfl_snap : 1;
        unsigned int reserved : 23;
    } reg;
} SOC_SYSQIC_RT_INPORT_INT_SNP_UNION;
#endif
#define SOC_SYSQIC_RT_INPORT_INT_SNP_dfx_rop_unq_err_snap_START (0)
#define SOC_SYSQIC_RT_INPORT_INT_SNP_dfx_rop_unq_err_snap_END (0)
#define SOC_SYSQIC_RT_INPORT_INT_SNP_dfx_did_unq_err_snap_START (1)
#define SOC_SYSQIC_RT_INPORT_INT_SNP_dfx_did_unq_err_snap_END (1)
#define SOC_SYSQIC_RT_INPORT_INT_SNP_dfx_i2o_access_err_snap_START (2)
#define SOC_SYSQIC_RT_INPORT_INT_SNP_dfx_i2o_access_err_snap_END (2)
#define SOC_SYSQIC_RT_INPORT_INT_SNP_dfx_vc_access_err_snap_START (3)
#define SOC_SYSQIC_RT_INPORT_INT_SNP_dfx_vc_access_err_snap_END (3)
#define SOC_SYSQIC_RT_INPORT_INT_SNP_dfx_vc_hop_err_snap_START (4)
#define SOC_SYSQIC_RT_INPORT_INT_SNP_dfx_vc_hop_err_snap_END (4)
#define SOC_SYSQIC_RT_INPORT_INT_SNP_dfx_vc_top_err_snap_START (5)
#define SOC_SYSQIC_RT_INPORT_INT_SNP_dfx_vc_top_err_snap_END (5)
#define SOC_SYSQIC_RT_INPORT_INT_SNP_dfx_vc_hop_top_err_snap_START (6)
#define SOC_SYSQIC_RT_INPORT_INT_SNP_dfx_vc_hop_top_err_snap_END (6)
#define SOC_SYSQIC_RT_INPORT_INT_SNP_dfx_did_ovfl_snap_START (7)
#define SOC_SYSQIC_RT_INPORT_INT_SNP_dfx_did_ovfl_snap_END (7)
#define SOC_SYSQIC_RT_INPORT_INT_SNP_dfx_rop_ovfl_snap_START (8)
#define SOC_SYSQIC_RT_INPORT_INT_SNP_dfx_rop_ovfl_snap_END (8)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int cfc0_bandwidth : 8;
        unsigned int cfc0_saturation : 8;
        unsigned int reserved_0 : 8;
        unsigned int cfc0_lp : 2;
        unsigned int reserved_1 : 5;
        unsigned int cfc0_en : 1;
    } reg;
} SOC_SYSQIC_RT_OUTPORT_CFC_CTRL_UNION;
#endif
#define SOC_SYSQIC_RT_OUTPORT_CFC_CTRL_cfc0_bandwidth_START (0)
#define SOC_SYSQIC_RT_OUTPORT_CFC_CTRL_cfc0_bandwidth_END (7)
#define SOC_SYSQIC_RT_OUTPORT_CFC_CTRL_cfc0_saturation_START (8)
#define SOC_SYSQIC_RT_OUTPORT_CFC_CTRL_cfc0_saturation_END (15)
#define SOC_SYSQIC_RT_OUTPORT_CFC_CTRL_cfc0_lp_START (24)
#define SOC_SYSQIC_RT_OUTPORT_CFC_CTRL_cfc0_lp_END (25)
#define SOC_SYSQIC_RT_OUTPORT_CFC_CTRL_cfc0_en_START (31)
#define SOC_SYSQIC_RT_OUTPORT_CFC_CTRL_cfc0_en_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int dfx_vc0_crdt_cnt : 8;
        unsigned int dfx_vc1_crdt_cnt : 8;
        unsigned int dfx_vc2_crdt_cnt : 8;
        unsigned int dfx_vc3_crdt_cnt : 8;
    } reg;
} SOC_SYSQIC_RT_OUTPORT_DFX_ST0_UNION;
#endif
#define SOC_SYSQIC_RT_OUTPORT_DFX_ST0_dfx_vc0_crdt_cnt_START (0)
#define SOC_SYSQIC_RT_OUTPORT_DFX_ST0_dfx_vc0_crdt_cnt_END (7)
#define SOC_SYSQIC_RT_OUTPORT_DFX_ST0_dfx_vc1_crdt_cnt_START (8)
#define SOC_SYSQIC_RT_OUTPORT_DFX_ST0_dfx_vc1_crdt_cnt_END (15)
#define SOC_SYSQIC_RT_OUTPORT_DFX_ST0_dfx_vc2_crdt_cnt_START (16)
#define SOC_SYSQIC_RT_OUTPORT_DFX_ST0_dfx_vc2_crdt_cnt_END (23)
#define SOC_SYSQIC_RT_OUTPORT_DFX_ST0_dfx_vc3_crdt_cnt_START (24)
#define SOC_SYSQIC_RT_OUTPORT_DFX_ST0_dfx_vc3_crdt_cnt_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int dfx_vccrdt_vld : 4;
        unsigned int reserved : 28;
    } reg;
} SOC_SYSQIC_RT_OUTPORT_DFX_ST1_UNION;
#endif
#define SOC_SYSQIC_RT_OUTPORT_DFX_ST1_dfx_vccrdt_vld_START (0)
#define SOC_SYSQIC_RT_OUTPORT_DFX_ST1_dfx_vccrdt_vld_END (3)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int wa_info : 32;
    } reg;
} SOC_SYSQIC_WA_INFO_UNION;
#endif
#define SOC_SYSQIC_WA_INFO_wa_info_START (0)
#define SOC_SYSQIC_WA_INFO_wa_info_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int wa_reserved_rw : 32;
    } reg;
} SOC_SYSQIC_WA_RESERVED_RW_UNION;
#endif
#define SOC_SYSQIC_WA_RESERVED_RW_wa_reserved_rw_START (0)
#define SOC_SYSQIC_WA_RESERVED_RW_wa_reserved_rw_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int wa_reserved_ro : 32;
    } reg;
} SOC_SYSQIC_WA_RESERVED_RO_UNION;
#endif
#define SOC_SYSQIC_WA_RESERVED_RO_wa_reserved_ro_START (0)
#define SOC_SYSQIC_WA_RESERVED_RO_wa_reserved_ro_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int ckg_byps_inp : 1;
        unsigned int ckg_byps_outp : 1;
        unsigned int ckg_byps_nmg : 1;
        unsigned int ckg_byps_rs : 1;
        unsigned int ckg_byps_txfifo : 1;
        unsigned int reserved : 27;
    } reg;
} SOC_SYSQIC_WA_CKG_BYPS_UNION;
#endif
#define SOC_SYSQIC_WA_CKG_BYPS_ckg_byps_inp_START (0)
#define SOC_SYSQIC_WA_CKG_BYPS_ckg_byps_inp_END (0)
#define SOC_SYSQIC_WA_CKG_BYPS_ckg_byps_outp_START (1)
#define SOC_SYSQIC_WA_CKG_BYPS_ckg_byps_outp_END (1)
#define SOC_SYSQIC_WA_CKG_BYPS_ckg_byps_nmg_START (2)
#define SOC_SYSQIC_WA_CKG_BYPS_ckg_byps_nmg_END (2)
#define SOC_SYSQIC_WA_CKG_BYPS_ckg_byps_rs_START (3)
#define SOC_SYSQIC_WA_CKG_BYPS_ckg_byps_rs_END (3)
#define SOC_SYSQIC_WA_CKG_BYPS_ckg_byps_txfifo_START (4)
#define SOC_SYSQIC_WA_CKG_BYPS_ckg_byps_txfifo_END (4)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int sync_ctrl_push : 1;
        unsigned int reserved : 31;
    } reg;
} SOC_SYSQIC_WA_GLB_CTRL_UNION;
#endif
#define SOC_SYSQIC_WA_GLB_CTRL_sync_ctrl_push_START (0)
#define SOC_SYSQIC_WA_GLB_CTRL_sync_ctrl_push_END (0)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int dfx_inblk_busy : 1;
        unsigned int dfx_outblk_busy : 1;
        unsigned int sync_vld : 1;
        unsigned int dfx_nopend_st : 1;
        unsigned int dfx_wa_sub_busy : 4;
        unsigned int dfx_two2one_busy : 4;
        unsigned int dfx_one2zero_busy : 4;
        unsigned int dfx_wdtr_busy : 4;
        unsigned int dfx_zero2one_busy : 4;
        unsigned int dfx_one2two_busy : 4;
        unsigned int dfx_rs_bp : 4;
    } reg;
} SOC_SYSQIC_WA_ST_UNION;
#endif
#define SOC_SYSQIC_WA_ST_dfx_inblk_busy_START (0)
#define SOC_SYSQIC_WA_ST_dfx_inblk_busy_END (0)
#define SOC_SYSQIC_WA_ST_dfx_outblk_busy_START (1)
#define SOC_SYSQIC_WA_ST_dfx_outblk_busy_END (1)
#define SOC_SYSQIC_WA_ST_sync_vld_START (2)
#define SOC_SYSQIC_WA_ST_sync_vld_END (2)
#define SOC_SYSQIC_WA_ST_dfx_nopend_st_START (3)
#define SOC_SYSQIC_WA_ST_dfx_nopend_st_END (3)
#define SOC_SYSQIC_WA_ST_dfx_wa_sub_busy_START (4)
#define SOC_SYSQIC_WA_ST_dfx_wa_sub_busy_END (7)
#define SOC_SYSQIC_WA_ST_dfx_two2one_busy_START (8)
#define SOC_SYSQIC_WA_ST_dfx_two2one_busy_END (11)
#define SOC_SYSQIC_WA_ST_dfx_one2zero_busy_START (12)
#define SOC_SYSQIC_WA_ST_dfx_one2zero_busy_END (15)
#define SOC_SYSQIC_WA_ST_dfx_wdtr_busy_START (16)
#define SOC_SYSQIC_WA_ST_dfx_wdtr_busy_END (19)
#define SOC_SYSQIC_WA_ST_dfx_zero2one_busy_START (20)
#define SOC_SYSQIC_WA_ST_dfx_zero2one_busy_END (23)
#define SOC_SYSQIC_WA_ST_dfx_one2two_busy_START (24)
#define SOC_SYSQIC_WA_ST_dfx_one2two_busy_END (27)
#define SOC_SYSQIC_WA_ST_dfx_rs_bp_START (28)
#define SOC_SYSQIC_WA_ST_dfx_rs_bp_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int dfx_rop_unq_err_st : 1;
        unsigned int dfx_did_unq_err_st : 1;
        unsigned int dfx_vc_access_err_st : 1;
        unsigned int dfx_vc_hop_err_st : 1;
        unsigned int dfx_vc_top_err_st : 1;
        unsigned int dfx_vc_hop_top_err_st : 1;
        unsigned int reserved : 26;
    } reg;
} SOC_SYSQIC_WA_INI_ST_UNION;
#endif
#define SOC_SYSQIC_WA_INI_ST_dfx_rop_unq_err_st_START (0)
#define SOC_SYSQIC_WA_INI_ST_dfx_rop_unq_err_st_END (0)
#define SOC_SYSQIC_WA_INI_ST_dfx_did_unq_err_st_START (1)
#define SOC_SYSQIC_WA_INI_ST_dfx_did_unq_err_st_END (1)
#define SOC_SYSQIC_WA_INI_ST_dfx_vc_access_err_st_START (2)
#define SOC_SYSQIC_WA_INI_ST_dfx_vc_access_err_st_END (2)
#define SOC_SYSQIC_WA_INI_ST_dfx_vc_hop_err_st_START (3)
#define SOC_SYSQIC_WA_INI_ST_dfx_vc_hop_err_st_END (3)
#define SOC_SYSQIC_WA_INI_ST_dfx_vc_top_err_st_START (4)
#define SOC_SYSQIC_WA_INI_ST_dfx_vc_top_err_st_END (4)
#define SOC_SYSQIC_WA_INI_ST_dfx_vc_hop_top_err_st_START (5)
#define SOC_SYSQIC_WA_INI_ST_dfx_vc_hop_top_err_st_END (5)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int dfx_rop_unq_err_ini : 1;
        unsigned int dfx_did_unq_err_ini : 1;
        unsigned int dfx_vc_access_err_ini : 1;
        unsigned int dfx_vc_hop_err_ini : 1;
        unsigned int dfx_vc_top_err_ini : 1;
        unsigned int dfx_vc_hop_top_err_ini : 1;
        unsigned int reserved : 26;
    } reg;
} SOC_SYSQIC_WA_INT_INI_UNION;
#endif
#define SOC_SYSQIC_WA_INT_INI_dfx_rop_unq_err_ini_START (0)
#define SOC_SYSQIC_WA_INT_INI_dfx_rop_unq_err_ini_END (0)
#define SOC_SYSQIC_WA_INT_INI_dfx_did_unq_err_ini_START (1)
#define SOC_SYSQIC_WA_INT_INI_dfx_did_unq_err_ini_END (1)
#define SOC_SYSQIC_WA_INT_INI_dfx_vc_access_err_ini_START (2)
#define SOC_SYSQIC_WA_INT_INI_dfx_vc_access_err_ini_END (2)
#define SOC_SYSQIC_WA_INT_INI_dfx_vc_hop_err_ini_START (3)
#define SOC_SYSQIC_WA_INT_INI_dfx_vc_hop_err_ini_END (3)
#define SOC_SYSQIC_WA_INT_INI_dfx_vc_top_err_ini_START (4)
#define SOC_SYSQIC_WA_INT_INI_dfx_vc_top_err_ini_END (4)
#define SOC_SYSQIC_WA_INT_INI_dfx_vc_hop_top_err_ini_START (5)
#define SOC_SYSQIC_WA_INT_INI_dfx_vc_hop_top_err_ini_END (5)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int dfx_err_clr : 1;
        unsigned int reserved : 31;
    } reg;
} SOC_SYSQIC_WA_INT_CLEAR_UNION;
#endif
#define SOC_SYSQIC_WA_INT_CLEAR_dfx_err_clr_START (0)
#define SOC_SYSQIC_WA_INT_CLEAR_dfx_err_clr_END (0)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int dfx_rop_unq_err_en : 1;
        unsigned int dfx_did_unq_err_en : 1;
        unsigned int dfx_vc_access_err_en : 1;
        unsigned int dfx_vc_hop_err_en : 1;
        unsigned int dfx_vc_top_err_en : 1;
        unsigned int dfx_vc_hop_top_err_en : 1;
        unsigned int reserved : 26;
    } reg;
} SOC_SYSQIC_WA_INT_EN_UNION;
#endif
#define SOC_SYSQIC_WA_INT_EN_dfx_rop_unq_err_en_START (0)
#define SOC_SYSQIC_WA_INT_EN_dfx_rop_unq_err_en_END (0)
#define SOC_SYSQIC_WA_INT_EN_dfx_did_unq_err_en_START (1)
#define SOC_SYSQIC_WA_INT_EN_dfx_did_unq_err_en_END (1)
#define SOC_SYSQIC_WA_INT_EN_dfx_vc_access_err_en_START (2)
#define SOC_SYSQIC_WA_INT_EN_dfx_vc_access_err_en_END (2)
#define SOC_SYSQIC_WA_INT_EN_dfx_vc_hop_err_en_START (3)
#define SOC_SYSQIC_WA_INT_EN_dfx_vc_hop_err_en_END (3)
#define SOC_SYSQIC_WA_INT_EN_dfx_vc_top_err_en_START (4)
#define SOC_SYSQIC_WA_INT_EN_dfx_vc_top_err_en_END (4)
#define SOC_SYSQIC_WA_INT_EN_dfx_vc_hop_top_err_en_START (5)
#define SOC_SYSQIC_WA_INT_EN_dfx_vc_hop_top_err_en_END (5)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int prescaler : 3;
        unsigned int reserved_0 : 5;
        unsigned int sat_en : 1;
        unsigned int sat_clr : 1;
        unsigned int reserved_1 : 2;
        unsigned int max_lat_vc : 4;
        unsigned int reserved_2 : 16;
    } reg;
} SOC_SYSQIC_WA_MAX_LAT_CFG_UNION;
#endif
#define SOC_SYSQIC_WA_MAX_LAT_CFG_prescaler_START (0)
#define SOC_SYSQIC_WA_MAX_LAT_CFG_prescaler_END (2)
#define SOC_SYSQIC_WA_MAX_LAT_CFG_sat_en_START (8)
#define SOC_SYSQIC_WA_MAX_LAT_CFG_sat_en_END (8)
#define SOC_SYSQIC_WA_MAX_LAT_CFG_sat_clr_START (9)
#define SOC_SYSQIC_WA_MAX_LAT_CFG_sat_clr_END (9)
#define SOC_SYSQIC_WA_MAX_LAT_CFG_max_lat_vc_START (12)
#define SOC_SYSQIC_WA_MAX_LAT_CFG_max_lat_vc_END (15)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int dfx_bp_cnt : 16;
        unsigned int dfx_sat_cnt : 16;
    } reg;
} SOC_SYSQIC_WA_MAX_LAT_ST_UNION;
#endif
#define SOC_SYSQIC_WA_MAX_LAT_ST_dfx_bp_cnt_START (0)
#define SOC_SYSQIC_WA_MAX_LAT_ST_dfx_bp_cnt_END (15)
#define SOC_SYSQIC_WA_MAX_LAT_ST_dfx_sat_cnt_START (16)
#define SOC_SYSQIC_WA_MAX_LAT_ST_dfx_sat_cnt_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int inp_lpc_ctrl : 1;
        unsigned int inp_lpc_req_sft : 1;
        unsigned int outp_lpc_ctrl : 1;
        unsigned int outp_lpc_req_sft : 1;
        unsigned int reserved : 28;
    } reg;
} SOC_SYSQIC_WA_LPC_CTRL_UNION;
#endif
#define SOC_SYSQIC_WA_LPC_CTRL_inp_lpc_ctrl_START (0)
#define SOC_SYSQIC_WA_LPC_CTRL_inp_lpc_ctrl_END (0)
#define SOC_SYSQIC_WA_LPC_CTRL_inp_lpc_req_sft_START (1)
#define SOC_SYSQIC_WA_LPC_CTRL_inp_lpc_req_sft_END (1)
#define SOC_SYSQIC_WA_LPC_CTRL_outp_lpc_ctrl_START (2)
#define SOC_SYSQIC_WA_LPC_CTRL_outp_lpc_ctrl_END (2)
#define SOC_SYSQIC_WA_LPC_CTRL_outp_lpc_req_sft_START (3)
#define SOC_SYSQIC_WA_LPC_CTRL_outp_lpc_req_sft_END (3)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int dfx_rop_unq_err_snap : 1;
        unsigned int dfx_did_unq_err_snap : 1;
        unsigned int dfx_vc_access_err_snap : 1;
        unsigned int dfx_vc_hop_err_snap : 1;
        unsigned int dfx_vc_top_err_snap : 1;
        unsigned int dfx_vc_hop_top_err_snap : 1;
        unsigned int reserved : 26;
    } reg;
} SOC_SYSQIC_WA_INT_SNAP_UNION;
#endif
#define SOC_SYSQIC_WA_INT_SNAP_dfx_rop_unq_err_snap_START (0)
#define SOC_SYSQIC_WA_INT_SNAP_dfx_rop_unq_err_snap_END (0)
#define SOC_SYSQIC_WA_INT_SNAP_dfx_did_unq_err_snap_START (1)
#define SOC_SYSQIC_WA_INT_SNAP_dfx_did_unq_err_snap_END (1)
#define SOC_SYSQIC_WA_INT_SNAP_dfx_vc_access_err_snap_START (2)
#define SOC_SYSQIC_WA_INT_SNAP_dfx_vc_access_err_snap_END (2)
#define SOC_SYSQIC_WA_INT_SNAP_dfx_vc_hop_err_snap_START (3)
#define SOC_SYSQIC_WA_INT_SNAP_dfx_vc_hop_err_snap_END (3)
#define SOC_SYSQIC_WA_INT_SNAP_dfx_vc_top_err_snap_START (4)
#define SOC_SYSQIC_WA_INT_SNAP_dfx_vc_top_err_snap_END (4)
#define SOC_SYSQIC_WA_INT_SNAP_dfx_vc_hop_top_err_snap_START (5)
#define SOC_SYSQIC_WA_INT_SNAP_dfx_vc_hop_top_err_snap_END (5)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int dfx_inp_lpc_req : 1;
        unsigned int dfx_inp_lpc_ack : 1;
        unsigned int dfx_outp_lpc_req : 1;
        unsigned int dfx_outp_lpc_ack : 1;
        unsigned int reserved : 28;
    } reg;
} SOC_SYSQIC_WA_LPC_ST_UNION;
#endif
#define SOC_SYSQIC_WA_LPC_ST_dfx_inp_lpc_req_START (0)
#define SOC_SYSQIC_WA_LPC_ST_dfx_inp_lpc_req_END (0)
#define SOC_SYSQIC_WA_LPC_ST_dfx_inp_lpc_ack_START (1)
#define SOC_SYSQIC_WA_LPC_ST_dfx_inp_lpc_ack_END (1)
#define SOC_SYSQIC_WA_LPC_ST_dfx_outp_lpc_req_START (2)
#define SOC_SYSQIC_WA_LPC_ST_dfx_outp_lpc_req_END (2)
#define SOC_SYSQIC_WA_LPC_ST_dfx_outp_lpc_ack_START (3)
#define SOC_SYSQIC_WA_LPC_ST_dfx_outp_lpc_ack_END (3)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int fifo_sfw_en : 4;
        unsigned int reserved : 28;
    } reg;
} SOC_SYSQIC_WA_SFW_CTRL_UNION;
#endif
#define SOC_SYSQIC_WA_SFW_CTRL_fifo_sfw_en_START (0)
#define SOC_SYSQIC_WA_SFW_CTRL_fifo_sfw_en_END (3)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int dfx_vcfifo_st : 8;
        unsigned int reserved : 24;
    } reg;
} SOC_SYSQIC_WA_INPORT_DFX_ST0_UNION;
#endif
#define SOC_SYSQIC_WA_INPORT_DFX_ST0_dfx_vcfifo_st_START (0)
#define SOC_SYSQIC_WA_INPORT_DFX_ST0_dfx_vcfifo_st_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int dfx_vc0_crdt_cnt : 8;
        unsigned int dfx_vc1_crdt_cnt : 8;
        unsigned int dfx_vc2_crdt_cnt : 8;
        unsigned int dfx_vc3_crdt_cnt : 8;
    } reg;
} SOC_SYSQIC_WA_OUTPORT_DFX_ST0_UNION;
#endif
#define SOC_SYSQIC_WA_OUTPORT_DFX_ST0_dfx_vc0_crdt_cnt_START (0)
#define SOC_SYSQIC_WA_OUTPORT_DFX_ST0_dfx_vc0_crdt_cnt_END (7)
#define SOC_SYSQIC_WA_OUTPORT_DFX_ST0_dfx_vc1_crdt_cnt_START (8)
#define SOC_SYSQIC_WA_OUTPORT_DFX_ST0_dfx_vc1_crdt_cnt_END (15)
#define SOC_SYSQIC_WA_OUTPORT_DFX_ST0_dfx_vc2_crdt_cnt_START (16)
#define SOC_SYSQIC_WA_OUTPORT_DFX_ST0_dfx_vc2_crdt_cnt_END (23)
#define SOC_SYSQIC_WA_OUTPORT_DFX_ST0_dfx_vc3_crdt_cnt_START (24)
#define SOC_SYSQIC_WA_OUTPORT_DFX_ST0_dfx_vc3_crdt_cnt_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int dfx_vccrdt_vld : 4;
        unsigned int dfx_sfwfifo_st : 8;
        unsigned int reserved : 20;
    } reg;
} SOC_SYSQIC_WA_OUTPORT_DFX_ST1_UNION;
#endif
#define SOC_SYSQIC_WA_OUTPORT_DFX_ST1_dfx_vccrdt_vld_START (0)
#define SOC_SYSQIC_WA_OUTPORT_DFX_ST1_dfx_vccrdt_vld_END (3)
#define SOC_SYSQIC_WA_OUTPORT_DFX_ST1_dfx_sfwfifo_st_START (4)
#define SOC_SYSQIC_WA_OUTPORT_DFX_ST1_dfx_sfwfifo_st_END (11)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int rtl_tag : 16;
        unsigned int rtl_version : 12;
        unsigned int reserved : 4;
    } reg;
} SOC_SYSQIC_TB_RTL_VER_UNION;
#endif
#define SOC_SYSQIC_TB_RTL_VER_rtl_tag_START (0)
#define SOC_SYSQIC_TB_RTL_VER_rtl_tag_END (15)
#define SOC_SYSQIC_TB_RTL_VER_rtl_version_START (16)
#define SOC_SYSQIC_TB_RTL_VER_rtl_version_END (27)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int rtl_info0 : 32;
    } reg;
} SOC_SYSQIC_TB_RTL_INFO_UNION;
#endif
#define SOC_SYSQIC_TB_RTL_INFO_rtl_info0_START (0)
#define SOC_SYSQIC_TB_RTL_INFO_rtl_info0_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int rtl_info1 : 32;
    } reg;
} SOC_SYSQIC_TB_RTL_INF1_UNION;
#endif
#define SOC_SYSQIC_TB_RTL_INF1_rtl_info1_START (0)
#define SOC_SYSQIC_TB_RTL_INF1_rtl_info1_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int rtl_info2 : 32;
    } reg;
} SOC_SYSQIC_TB_RTL_INF2_UNION;
#endif
#define SOC_SYSQIC_TB_RTL_INF2_rtl_info2_START (0)
#define SOC_SYSQIC_TB_RTL_INF2_rtl_info2_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int tb_ckg_byp : 1;
        unsigned int awct_ckg_byp : 1;
        unsigned int arct_ckg_byp : 1;
        unsigned int rreq_ckg_byp : 1;
        unsigned int rrsp_ckg_byp : 1;
        unsigned int wreq_ckg_byp : 1;
        unsigned int wrsp_ckg_byp : 1;
        unsigned int cfc_ckg_byp : 1;
        unsigned int wprob_ckg_byp : 1;
        unsigned int rprob_ckg_byp : 1;
        unsigned int tb0_cfg_ckg_byp : 1;
        unsigned int rs_ckg_byp : 1;
        unsigned int vc_ckg_byp : 1;
        unsigned int wr_dfltslv_ckg_byp : 1;
        unsigned int rd_dfltslv_ckg_byp : 1;
        unsigned int wr_fwl_ckg_byp : 1;
        unsigned int rd_fwl_ckg_byp : 1;
        unsigned int reserved : 15;
    } reg;
} SOC_SYSQIC_TB_CKG_BYP_UNION;
#endif
#define SOC_SYSQIC_TB_CKG_BYP_tb_ckg_byp_START (0)
#define SOC_SYSQIC_TB_CKG_BYP_tb_ckg_byp_END (0)
#define SOC_SYSQIC_TB_CKG_BYP_awct_ckg_byp_START (1)
#define SOC_SYSQIC_TB_CKG_BYP_awct_ckg_byp_END (1)
#define SOC_SYSQIC_TB_CKG_BYP_arct_ckg_byp_START (2)
#define SOC_SYSQIC_TB_CKG_BYP_arct_ckg_byp_END (2)
#define SOC_SYSQIC_TB_CKG_BYP_rreq_ckg_byp_START (3)
#define SOC_SYSQIC_TB_CKG_BYP_rreq_ckg_byp_END (3)
#define SOC_SYSQIC_TB_CKG_BYP_rrsp_ckg_byp_START (4)
#define SOC_SYSQIC_TB_CKG_BYP_rrsp_ckg_byp_END (4)
#define SOC_SYSQIC_TB_CKG_BYP_wreq_ckg_byp_START (5)
#define SOC_SYSQIC_TB_CKG_BYP_wreq_ckg_byp_END (5)
#define SOC_SYSQIC_TB_CKG_BYP_wrsp_ckg_byp_START (6)
#define SOC_SYSQIC_TB_CKG_BYP_wrsp_ckg_byp_END (6)
#define SOC_SYSQIC_TB_CKG_BYP_cfc_ckg_byp_START (7)
#define SOC_SYSQIC_TB_CKG_BYP_cfc_ckg_byp_END (7)
#define SOC_SYSQIC_TB_CKG_BYP_wprob_ckg_byp_START (8)
#define SOC_SYSQIC_TB_CKG_BYP_wprob_ckg_byp_END (8)
#define SOC_SYSQIC_TB_CKG_BYP_rprob_ckg_byp_START (9)
#define SOC_SYSQIC_TB_CKG_BYP_rprob_ckg_byp_END (9)
#define SOC_SYSQIC_TB_CKG_BYP_tb0_cfg_ckg_byp_START (10)
#define SOC_SYSQIC_TB_CKG_BYP_tb0_cfg_ckg_byp_END (10)
#define SOC_SYSQIC_TB_CKG_BYP_rs_ckg_byp_START (11)
#define SOC_SYSQIC_TB_CKG_BYP_rs_ckg_byp_END (11)
#define SOC_SYSQIC_TB_CKG_BYP_vc_ckg_byp_START (12)
#define SOC_SYSQIC_TB_CKG_BYP_vc_ckg_byp_END (12)
#define SOC_SYSQIC_TB_CKG_BYP_wr_dfltslv_ckg_byp_START (13)
#define SOC_SYSQIC_TB_CKG_BYP_wr_dfltslv_ckg_byp_END (13)
#define SOC_SYSQIC_TB_CKG_BYP_rd_dfltslv_ckg_byp_START (14)
#define SOC_SYSQIC_TB_CKG_BYP_rd_dfltslv_ckg_byp_END (14)
#define SOC_SYSQIC_TB_CKG_BYP_wr_fwl_ckg_byp_START (15)
#define SOC_SYSQIC_TB_CKG_BYP_wr_fwl_ckg_byp_END (15)
#define SOC_SYSQIC_TB_CKG_BYP_rd_fwl_ckg_byp_START (16)
#define SOC_SYSQIC_TB_CKG_BYP_rd_fwl_ckg_byp_END (16)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int parity_chk_en : 1;
        unsigned int reserved : 31;
    } reg;
} SOC_SYSQIC_TB_PARITY_CTRL_UNION;
#endif
#define SOC_SYSQIC_TB_PARITY_CTRL_parity_chk_en_START (0)
#define SOC_SYSQIC_TB_PARITY_CTRL_parity_chk_en_END (0)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int wreq_fifo_stfwd_lvl : 8;
        unsigned int reserved : 24;
    } reg;
} SOC_SYSQIC_TB_VC_CTRL_UNION;
#endif
#define SOC_SYSQIC_TB_VC_CTRL_wreq_fifo_stfwd_lvl_START (0)
#define SOC_SYSQIC_TB_VC_CTRL_wreq_fifo_stfwd_lvl_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int cfc0_ostd_lvl : 8;
        unsigned int reserved : 23;
        unsigned int cfc0_en : 1;
    } reg;
} SOC_SYSQIC_TB_QOS_CFC0_UNION;
#endif
#define SOC_SYSQIC_TB_QOS_CFC0_cfc0_ostd_lvl_START (0)
#define SOC_SYSQIC_TB_QOS_CFC0_cfc0_ostd_lvl_END (7)
#define SOC_SYSQIC_TB_QOS_CFC0_cfc0_en_START (31)
#define SOC_SYSQIC_TB_QOS_CFC0_cfc0_en_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int cfc1_ostd_lvl : 8;
        unsigned int reserved : 23;
        unsigned int cfc1_en : 1;
    } reg;
} SOC_SYSQIC_TB_QOS_CFC1_UNION;
#endif
#define SOC_SYSQIC_TB_QOS_CFC1_cfc1_ostd_lvl_START (0)
#define SOC_SYSQIC_TB_QOS_CFC1_cfc1_ostd_lvl_END (7)
#define SOC_SYSQIC_TB_QOS_CFC1_cfc1_en_START (31)
#define SOC_SYSQIC_TB_QOS_CFC1_cfc1_en_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int cfc2_ostd_lvl : 8;
        unsigned int reserved : 23;
        unsigned int cfc2_en : 1;
    } reg;
} SOC_SYSQIC_TB_QOS_CFC2_UNION;
#endif
#define SOC_SYSQIC_TB_QOS_CFC2_cfc2_ostd_lvl_START (0)
#define SOC_SYSQIC_TB_QOS_CFC2_cfc2_ostd_lvl_END (7)
#define SOC_SYSQIC_TB_QOS_CFC2_cfc2_en_START (31)
#define SOC_SYSQIC_TB_QOS_CFC2_cfc2_en_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int cfc3_ostd_lvl : 8;
        unsigned int reserved : 23;
        unsigned int cfc3_en : 1;
    } reg;
} SOC_SYSQIC_TB_QOS_CFC3_UNION;
#endif
#define SOC_SYSQIC_TB_QOS_CFC3_cfc3_ostd_lvl_START (0)
#define SOC_SYSQIC_TB_QOS_CFC3_cfc3_ostd_lvl_END (7)
#define SOC_SYSQIC_TB_QOS_CFC3_cfc3_en_START (31)
#define SOC_SYSQIC_TB_QOS_CFC3_cfc3_en_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int cfc4_bandwidth : 8;
        unsigned int cfc4_saturation : 8;
        unsigned int reserved_0 : 8;
        unsigned int cfc4_lp : 2;
        unsigned int reserved_1 : 4;
        unsigned int cfc4_cmd_type : 1;
        unsigned int cfc4_en : 1;
    } reg;
} SOC_SYSQIC_TB_QOS_CFC4_UNION;
#endif
#define SOC_SYSQIC_TB_QOS_CFC4_cfc4_bandwidth_START (0)
#define SOC_SYSQIC_TB_QOS_CFC4_cfc4_bandwidth_END (7)
#define SOC_SYSQIC_TB_QOS_CFC4_cfc4_saturation_START (8)
#define SOC_SYSQIC_TB_QOS_CFC4_cfc4_saturation_END (15)
#define SOC_SYSQIC_TB_QOS_CFC4_cfc4_lp_START (24)
#define SOC_SYSQIC_TB_QOS_CFC4_cfc4_lp_END (25)
#define SOC_SYSQIC_TB_QOS_CFC4_cfc4_cmd_type_START (30)
#define SOC_SYSQIC_TB_QOS_CFC4_cfc4_cmd_type_END (30)
#define SOC_SYSQIC_TB_QOS_CFC4_cfc4_en_START (31)
#define SOC_SYSQIC_TB_QOS_CFC4_cfc4_en_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int cfc5_bandwidth : 8;
        unsigned int cfc5_saturation : 8;
        unsigned int reserved_0 : 8;
        unsigned int cfc5_lp : 2;
        unsigned int reserved_1 : 5;
        unsigned int cfc5_en : 1;
    } reg;
} SOC_SYSQIC_TB_QOS_CFC5_UNION;
#endif
#define SOC_SYSQIC_TB_QOS_CFC5_cfc5_bandwidth_START (0)
#define SOC_SYSQIC_TB_QOS_CFC5_cfc5_bandwidth_END (7)
#define SOC_SYSQIC_TB_QOS_CFC5_cfc5_saturation_START (8)
#define SOC_SYSQIC_TB_QOS_CFC5_cfc5_saturation_END (15)
#define SOC_SYSQIC_TB_QOS_CFC5_cfc5_lp_START (24)
#define SOC_SYSQIC_TB_QOS_CFC5_cfc5_lp_END (25)
#define SOC_SYSQIC_TB_QOS_CFC5_cfc5_en_START (31)
#define SOC_SYSQIC_TB_QOS_CFC5_cfc5_en_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int press_mask : 1;
        unsigned int reserved : 31;
    } reg;
} SOC_SYSQIC_TB_QOS_PRESS_UNION;
#endif
#define SOC_SYSQIC_TB_QOS_PRESS_press_mask_START (0)
#define SOC_SYSQIC_TB_QOS_PRESS_press_mask_END (0)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int int_mask : 16;
        unsigned int int_en : 16;
    } reg;
} SOC_SYSQIC_TB_INT_EN_UNION;
#endif
#define SOC_SYSQIC_TB_INT_EN_int_mask_START (0)
#define SOC_SYSQIC_TB_INT_EN_int_mask_END (15)
#define SOC_SYSQIC_TB_INT_EN_int_en_START (16)
#define SOC_SYSQIC_TB_INT_EN_int_en_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int int_clr : 16;
        unsigned int reserved : 16;
    } reg;
} SOC_SYSQIC_TB_INT_CLR_UNION;
#endif
#define SOC_SYSQIC_TB_INT_CLR_int_clr_START (0)
#define SOC_SYSQIC_TB_INT_CLR_int_clr_END (15)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int int_st : 16;
        unsigned int int_st_lock : 16;
    } reg;
} SOC_SYSQIC_TB_INT_STATUS_UNION;
#endif
#define SOC_SYSQIC_TB_INT_STATUS_int_st_START (0)
#define SOC_SYSQIC_TB_INT_STATUS_int_st_END (15)
#define SOC_SYSQIC_TB_INT_STATUS_int_st_lock_START (16)
#define SOC_SYSQIC_TB_INT_STATUS_int_st_lock_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int wreq_opcerr_type : 2;
        unsigned int reserved_0 : 2;
        unsigned int wreq_tsizeerr_type : 4;
        unsigned int reserved_1 : 8;
        unsigned int rreq_opcerr_type : 2;
        unsigned int reserved_2 : 2;
        unsigned int rreq_tsizeerr_type : 4;
        unsigned int wreq_tmo_type : 3;
        unsigned int reserved_3 : 1;
        unsigned int rreq_tmo_type : 3;
        unsigned int reserved_4 : 1;
    } reg;
} SOC_SYSQIC_TB_INT_TYPE_UNION;
#endif
#define SOC_SYSQIC_TB_INT_TYPE_wreq_opcerr_type_START (0)
#define SOC_SYSQIC_TB_INT_TYPE_wreq_opcerr_type_END (1)
#define SOC_SYSQIC_TB_INT_TYPE_wreq_tsizeerr_type_START (4)
#define SOC_SYSQIC_TB_INT_TYPE_wreq_tsizeerr_type_END (7)
#define SOC_SYSQIC_TB_INT_TYPE_rreq_opcerr_type_START (16)
#define SOC_SYSQIC_TB_INT_TYPE_rreq_opcerr_type_END (17)
#define SOC_SYSQIC_TB_INT_TYPE_rreq_tsizeerr_type_START (20)
#define SOC_SYSQIC_TB_INT_TYPE_rreq_tsizeerr_type_END (23)
#define SOC_SYSQIC_TB_INT_TYPE_wreq_tmo_type_START (24)
#define SOC_SYSQIC_TB_INT_TYPE_wreq_tmo_type_END (26)
#define SOC_SYSQIC_TB_INT_TYPE_rreq_tmo_type_START (28)
#define SOC_SYSQIC_TB_INT_TYPE_rreq_tmo_type_END (30)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int wreq_inf0 : 32;
    } reg;
} SOC_SYSQIC_TB_INT_WREQ_INF0_UNION;
#endif
#define SOC_SYSQIC_TB_INT_WREQ_INF0_wreq_inf0_START (0)
#define SOC_SYSQIC_TB_INT_WREQ_INF0_wreq_inf0_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int wreq_inf1 : 32;
    } reg;
} SOC_SYSQIC_TB_INT_WREQ_INF1_UNION;
#endif
#define SOC_SYSQIC_TB_INT_WREQ_INF1_wreq_inf1_START (0)
#define SOC_SYSQIC_TB_INT_WREQ_INF1_wreq_inf1_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int rreq_inf0 : 32;
    } reg;
} SOC_SYSQIC_TB_INT_RREQ_INF0_UNION;
#endif
#define SOC_SYSQIC_TB_INT_RREQ_INF0_rreq_inf0_START (0)
#define SOC_SYSQIC_TB_INT_RREQ_INF0_rreq_inf0_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int rreq_inf1 : 32;
    } reg;
} SOC_SYSQIC_TB_INT_RREQ_INF1_UNION;
#endif
#define SOC_SYSQIC_TB_INT_RREQ_INF1_rreq_inf1_START (0)
#define SOC_SYSQIC_TB_INT_RREQ_INF1_rreq_inf1_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int wreq_signerr_type : 3;
        unsigned int reserved_0 : 1;
        unsigned int wreq_diderr_type : 2;
        unsigned int reserved_1 : 2;
        unsigned int rreq_signerr_type : 3;
        unsigned int reserved_2 : 1;
        unsigned int rreq_diderr_type : 2;
        unsigned int reserved_3 : 2;
        unsigned int cfg_req_signerr_type : 3;
        unsigned int reserved_4 : 1;
        unsigned int cfg_req_diderr_type : 2;
        unsigned int reserved_5 : 10;
    } reg;
} SOC_SYSQIC_TB_ERR_CMD_NOTE0_UNION;
#endif
#define SOC_SYSQIC_TB_ERR_CMD_NOTE0_wreq_signerr_type_START (0)
#define SOC_SYSQIC_TB_ERR_CMD_NOTE0_wreq_signerr_type_END (2)
#define SOC_SYSQIC_TB_ERR_CMD_NOTE0_wreq_diderr_type_START (4)
#define SOC_SYSQIC_TB_ERR_CMD_NOTE0_wreq_diderr_type_END (5)
#define SOC_SYSQIC_TB_ERR_CMD_NOTE0_rreq_signerr_type_START (8)
#define SOC_SYSQIC_TB_ERR_CMD_NOTE0_rreq_signerr_type_END (10)
#define SOC_SYSQIC_TB_ERR_CMD_NOTE0_rreq_diderr_type_START (12)
#define SOC_SYSQIC_TB_ERR_CMD_NOTE0_rreq_diderr_type_END (13)
#define SOC_SYSQIC_TB_ERR_CMD_NOTE0_cfg_req_signerr_type_START (16)
#define SOC_SYSQIC_TB_ERR_CMD_NOTE0_cfg_req_signerr_type_END (18)
#define SOC_SYSQIC_TB_ERR_CMD_NOTE0_cfg_req_diderr_type_START (20)
#define SOC_SYSQIC_TB_ERR_CMD_NOTE0_cfg_req_diderr_type_END (21)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int wreq_signerr_mask : 1;
        unsigned int wreq_diderr_mask : 1;
        unsigned int rreq_signerr_mask : 1;
        unsigned int rreq_diderr_mask : 1;
        unsigned int cfg_req_signerr_mask : 1;
        unsigned int cfg_req_diderr_mask : 1;
        unsigned int reserved : 26;
    } reg;
} SOC_SYSQIC_TB_ERR_CMD_MASK_UNION;
#endif
#define SOC_SYSQIC_TB_ERR_CMD_MASK_wreq_signerr_mask_START (0)
#define SOC_SYSQIC_TB_ERR_CMD_MASK_wreq_signerr_mask_END (0)
#define SOC_SYSQIC_TB_ERR_CMD_MASK_wreq_diderr_mask_START (1)
#define SOC_SYSQIC_TB_ERR_CMD_MASK_wreq_diderr_mask_END (1)
#define SOC_SYSQIC_TB_ERR_CMD_MASK_rreq_signerr_mask_START (2)
#define SOC_SYSQIC_TB_ERR_CMD_MASK_rreq_signerr_mask_END (2)
#define SOC_SYSQIC_TB_ERR_CMD_MASK_rreq_diderr_mask_START (3)
#define SOC_SYSQIC_TB_ERR_CMD_MASK_rreq_diderr_mask_END (3)
#define SOC_SYSQIC_TB_ERR_CMD_MASK_cfg_req_signerr_mask_START (4)
#define SOC_SYSQIC_TB_ERR_CMD_MASK_cfg_req_signerr_mask_END (4)
#define SOC_SYSQIC_TB_ERR_CMD_MASK_cfg_req_diderr_mask_START (5)
#define SOC_SYSQIC_TB_ERR_CMD_MASK_cfg_req_diderr_mask_END (5)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int lpc_en : 1;
        unsigned int lpc_wreq_req : 1;
        unsigned int lpc_wrsp_req : 1;
        unsigned int lpc_rreq_req : 1;
        unsigned int lpc_rrsp_req : 1;
        unsigned int lpc_cfg_req_req : 1;
        unsigned int lpc_cfg_rsp_req : 1;
        unsigned int reserved : 25;
    } reg;
} SOC_SYSQIC_TB_LPC_CTRL_UNION;
#endif
#define SOC_SYSQIC_TB_LPC_CTRL_lpc_en_START (0)
#define SOC_SYSQIC_TB_LPC_CTRL_lpc_en_END (0)
#define SOC_SYSQIC_TB_LPC_CTRL_lpc_wreq_req_START (1)
#define SOC_SYSQIC_TB_LPC_CTRL_lpc_wreq_req_END (1)
#define SOC_SYSQIC_TB_LPC_CTRL_lpc_wrsp_req_START (2)
#define SOC_SYSQIC_TB_LPC_CTRL_lpc_wrsp_req_END (2)
#define SOC_SYSQIC_TB_LPC_CTRL_lpc_rreq_req_START (3)
#define SOC_SYSQIC_TB_LPC_CTRL_lpc_rreq_req_END (3)
#define SOC_SYSQIC_TB_LPC_CTRL_lpc_rrsp_req_START (4)
#define SOC_SYSQIC_TB_LPC_CTRL_lpc_rrsp_req_END (4)
#define SOC_SYSQIC_TB_LPC_CTRL_lpc_cfg_req_req_START (5)
#define SOC_SYSQIC_TB_LPC_CTRL_lpc_cfg_req_req_END (5)
#define SOC_SYSQIC_TB_LPC_CTRL_lpc_cfg_rsp_req_START (6)
#define SOC_SYSQIC_TB_LPC_CTRL_lpc_cfg_rsp_req_END (6)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int arct_depth : 8;
        unsigned int awct_depth : 8;
        unsigned int reserved : 16;
    } reg;
} SOC_SYSQIC_TB_CT_CTRL_UNION;
#endif
#define SOC_SYSQIC_TB_CT_CTRL_arct_depth_START (0)
#define SOC_SYSQIC_TB_CT_CTRL_arct_depth_END (7)
#define SOC_SYSQIC_TB_CT_CTRL_awct_depth_START (8)
#define SOC_SYSQIC_TB_CT_CTRL_awct_depth_END (15)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int dfx_module_busy : 32;
    } reg;
} SOC_SYSQIC_DFX_TB_MODULE_UNION;
#endif
#define SOC_SYSQIC_DFX_TB_MODULE_dfx_module_busy_START (0)
#define SOC_SYSQIC_DFX_TB_MODULE_dfx_module_busy_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int dfx_bp_st : 32;
    } reg;
} SOC_SYSQIC_DFX_TB_BP_UNION;
#endif
#define SOC_SYSQIC_DFX_TB_BP_dfx_bp_st_START (0)
#define SOC_SYSQIC_DFX_TB_BP_dfx_bp_st_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int dfx_rd_ot : 8;
        unsigned int dfx_wr_ot : 8;
        unsigned int reserved : 16;
    } reg;
} SOC_SYSQIC_DFX_TB_OT_UNION;
#endif
#define SOC_SYSQIC_DFX_TB_OT_dfx_rd_ot_START (0)
#define SOC_SYSQIC_DFX_TB_OT_dfx_rd_ot_END (7)
#define SOC_SYSQIC_DFX_TB_OT_dfx_wr_ot_START (8)
#define SOC_SYSQIC_DFX_TB_OT_dfx_wr_ot_END (15)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int dfx_awct_buf_st0 : 32;
    } reg;
} SOC_SYSQIC_DFX_TB_AWCT_BUF0_UNION;
#endif
#define SOC_SYSQIC_DFX_TB_AWCT_BUF0_dfx_awct_buf_st0_START (0)
#define SOC_SYSQIC_DFX_TB_AWCT_BUF0_dfx_awct_buf_st0_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int dfx_awct_buf_st1 : 32;
    } reg;
} SOC_SYSQIC_DFX_TB_AWCT_BUF1_UNION;
#endif
#define SOC_SYSQIC_DFX_TB_AWCT_BUF1_dfx_awct_buf_st1_START (0)
#define SOC_SYSQIC_DFX_TB_AWCT_BUF1_dfx_awct_buf_st1_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int dfx_arct_buf_st0 : 32;
    } reg;
} SOC_SYSQIC_DFX_TB_ARCT_BUF0_UNION;
#endif
#define SOC_SYSQIC_DFX_TB_ARCT_BUF0_dfx_arct_buf_st0_START (0)
#define SOC_SYSQIC_DFX_TB_ARCT_BUF0_dfx_arct_buf_st0_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int dfx_arct_buf_st1 : 32;
    } reg;
} SOC_SYSQIC_DFX_TB_ARCT_BUF1_UNION;
#endif
#define SOC_SYSQIC_DFX_TB_ARCT_BUF1_dfx_arct_buf_st1_START (0)
#define SOC_SYSQIC_DFX_TB_ARCT_BUF1_dfx_arct_buf_st1_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int dfx_arct_buf_st2 : 32;
    } reg;
} SOC_SYSQIC_DFX_TB_ARCT_BUF2_UNION;
#endif
#define SOC_SYSQIC_DFX_TB_ARCT_BUF2_dfx_arct_buf_st2_START (0)
#define SOC_SYSQIC_DFX_TB_ARCT_BUF2_dfx_arct_buf_st2_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int dfx_arct_buf_st3 : 32;
    } reg;
} SOC_SYSQIC_DFX_TB_ARCT_BUF3_UNION;
#endif
#define SOC_SYSQIC_DFX_TB_ARCT_BUF3_dfx_arct_buf_st3_START (0)
#define SOC_SYSQIC_DFX_TB_ARCT_BUF3_dfx_arct_buf_st3_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int dfx_data_buf_st : 32;
    } reg;
} SOC_SYSQIC_DFX_TB_DATA_BUF_UNION;
#endif
#define SOC_SYSQIC_DFX_TB_DATA_BUF_dfx_data_buf_st_START (0)
#define SOC_SYSQIC_DFX_TB_DATA_BUF_dfx_data_buf_st_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int dfx_fifo_st : 32;
    } reg;
} SOC_SYSQIC_DFX_TB_FIFO_UNION;
#endif
#define SOC_SYSQIC_DFX_TB_FIFO_dfx_fifo_st_START (0)
#define SOC_SYSQIC_DFX_TB_FIFO_dfx_fifo_st_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int dfx_wrsp_crdt_st : 4;
        unsigned int dfx_rrsp_crdt_st : 4;
        unsigned int dfx_cfg_rsp_crdt_st : 4;
        unsigned int reserved : 20;
    } reg;
} SOC_SYSQIC_DFX_TB_TX_CRDT_UNION;
#endif
#define SOC_SYSQIC_DFX_TB_TX_CRDT_dfx_wrsp_crdt_st_START (0)
#define SOC_SYSQIC_DFX_TB_TX_CRDT_dfx_wrsp_crdt_st_END (3)
#define SOC_SYSQIC_DFX_TB_TX_CRDT_dfx_rrsp_crdt_st_START (4)
#define SOC_SYSQIC_DFX_TB_TX_CRDT_dfx_rrsp_crdt_st_END (7)
#define SOC_SYSQIC_DFX_TB_TX_CRDT_dfx_cfg_rsp_crdt_st_START (8)
#define SOC_SYSQIC_DFX_TB_TX_CRDT_dfx_cfg_rsp_crdt_st_END (11)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int dfx_obsv_idx : 16;
        unsigned int dfx_obsv_type : 1;
        unsigned int reserved : 15;
    } reg;
} SOC_SYSQIC_DFX_TB_OBSERVE_CTRL_UNION;
#endif
#define SOC_SYSQIC_DFX_TB_OBSERVE_CTRL_dfx_obsv_idx_START (0)
#define SOC_SYSQIC_DFX_TB_OBSERVE_CTRL_dfx_obsv_idx_END (15)
#define SOC_SYSQIC_DFX_TB_OBSERVE_CTRL_dfx_obsv_type_START (16)
#define SOC_SYSQIC_DFX_TB_OBSERVE_CTRL_dfx_obsv_type_END (16)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int dfx_ct_info : 32;
    } reg;
} SOC_SYSQIC_DFX_TB_OBSERVE_INFO_UNION;
#endif
#define SOC_SYSQIC_DFX_TB_OBSERVE_INFO_dfx_ct_info_START (0)
#define SOC_SYSQIC_DFX_TB_OBSERVE_INFO_dfx_ct_info_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int dfx_cfc0_ostd_num : 8;
        unsigned int dfx_cfc1_ostd_num : 8;
        unsigned int dfx_cfc2_ostd_num : 8;
        unsigned int dfx_cfc3_ostd_num : 8;
    } reg;
} SOC_SYSQIC_DFX_TB_CFC_OSTD_NUM_UNION;
#endif
#define SOC_SYSQIC_DFX_TB_CFC_OSTD_NUM_dfx_cfc0_ostd_num_START (0)
#define SOC_SYSQIC_DFX_TB_CFC_OSTD_NUM_dfx_cfc0_ostd_num_END (7)
#define SOC_SYSQIC_DFX_TB_CFC_OSTD_NUM_dfx_cfc1_ostd_num_START (8)
#define SOC_SYSQIC_DFX_TB_CFC_OSTD_NUM_dfx_cfc1_ostd_num_END (15)
#define SOC_SYSQIC_DFX_TB_CFC_OSTD_NUM_dfx_cfc2_ostd_num_START (16)
#define SOC_SYSQIC_DFX_TB_CFC_OSTD_NUM_dfx_cfc2_ostd_num_END (23)
#define SOC_SYSQIC_DFX_TB_CFC_OSTD_NUM_dfx_cfc3_ostd_num_START (24)
#define SOC_SYSQIC_DFX_TB_CFC_OSTD_NUM_dfx_cfc3_ostd_num_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int dfx_lpc_st : 32;
    } reg;
} SOC_SYSQIC_DFX_TB_LPC_ST_UNION;
#endif
#define SOC_SYSQIC_DFX_TB_LPC_ST_dfx_lpc_st_START (0)
#define SOC_SYSQIC_DFX_TB_LPC_ST_dfx_lpc_st_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int dfx_wreq_arb_st : 4;
        unsigned int dfx_rreq_arb_st : 4;
        unsigned int dfx_cfg_rsp_arb_st : 2;
        unsigned int reserved : 22;
    } reg;
} SOC_SYSQIC_DFX_TB_ARB_ST_UNION;
#endif
#define SOC_SYSQIC_DFX_TB_ARB_ST_dfx_wreq_arb_st_START (0)
#define SOC_SYSQIC_DFX_TB_ARB_ST_dfx_wreq_arb_st_END (3)
#define SOC_SYSQIC_DFX_TB_ARB_ST_dfx_rreq_arb_st_START (4)
#define SOC_SYSQIC_DFX_TB_ARB_ST_dfx_rreq_arb_st_END (7)
#define SOC_SYSQIC_DFX_TB_ARB_ST_dfx_cfg_rsp_arb_st_START (8)
#define SOC_SYSQIC_DFX_TB_ARB_ST_dfx_cfg_rsp_arb_st_END (9)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int dfx_awid : 8;
        unsigned int dfx_wid : 8;
        unsigned int dfx_wrsp_iqry_id : 8;
        unsigned int reserved : 8;
    } reg;
} SOC_SYSQIC_DFX_TB_ID_ST0_UNION;
#endif
#define SOC_SYSQIC_DFX_TB_ID_ST0_dfx_awid_START (0)
#define SOC_SYSQIC_DFX_TB_ID_ST0_dfx_awid_END (7)
#define SOC_SYSQIC_DFX_TB_ID_ST0_dfx_wid_START (8)
#define SOC_SYSQIC_DFX_TB_ID_ST0_dfx_wid_END (15)
#define SOC_SYSQIC_DFX_TB_ID_ST0_dfx_wrsp_iqry_id_START (16)
#define SOC_SYSQIC_DFX_TB_ID_ST0_dfx_wrsp_iqry_id_END (23)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int dfx_arid : 16;
        unsigned int dfx_rrsp_iqry_id : 16;
    } reg;
} SOC_SYSQIC_DFX_TB_ID_ST1_UNION;
#endif
#define SOC_SYSQIC_DFX_TB_ID_ST1_dfx_arid_START (0)
#define SOC_SYSQIC_DFX_TB_ID_ST1_dfx_arid_END (15)
#define SOC_SYSQIC_DFX_TB_ID_ST1_dfx_rrsp_iqry_id_START (16)
#define SOC_SYSQIC_DFX_TB_ID_ST1_dfx_rrsp_iqry_id_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int poison_msk : 1;
        unsigned int reserved : 31;
    } reg;
} SOC_SYSQIC_DFX_TB_POISON_MSK_UNION;
#endif
#define SOC_SYSQIC_DFX_TB_POISON_MSK_poison_msk_START (0)
#define SOC_SYSQIC_DFX_TB_POISON_MSK_poison_msk_END (0)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int poison_clr : 1;
        unsigned int reserved : 31;
    } reg;
} SOC_SYSQIC_DFX_TB_POISON_CLR_UNION;
#endif
#define SOC_SYSQIC_DFX_TB_POISON_CLR_poison_clr_START (0)
#define SOC_SYSQIC_DFX_TB_POISON_CLR_poison_clr_END (0)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int poison_st : 1;
        unsigned int reserved : 31;
    } reg;
} SOC_SYSQIC_DFX_TB_POISON_ST_UNION;
#endif
#define SOC_SYSQIC_DFX_TB_POISON_ST_poison_st_START (0)
#define SOC_SYSQIC_DFX_TB_POISON_ST_poison_st_END (0)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int reserved : 32;
    } reg;
} SOC_SYSQIC_TB_RESERVED_RW_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int reserved : 32;
    } reg;
} SOC_SYSQIC_TB_RESERVED_RO_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int burst_en : 1;
        unsigned int pwrds_xreq_req_sel : 1;
        unsigned int pwrds_xreq_req_sw_val : 1;
        unsigned int pwrds_xrsp_req_sel : 1;
        unsigned int pwrds_xrsp_req_sw_val : 1;
        unsigned int reserved : 27;
    } reg;
} SOC_SYSQIC_TB1_CTRL0_UNION;
#endif
#define SOC_SYSQIC_TB1_CTRL0_burst_en_START (0)
#define SOC_SYSQIC_TB1_CTRL0_burst_en_END (0)
#define SOC_SYSQIC_TB1_CTRL0_pwrds_xreq_req_sel_START (1)
#define SOC_SYSQIC_TB1_CTRL0_pwrds_xreq_req_sel_END (1)
#define SOC_SYSQIC_TB1_CTRL0_pwrds_xreq_req_sw_val_START (2)
#define SOC_SYSQIC_TB1_CTRL0_pwrds_xreq_req_sw_val_END (2)
#define SOC_SYSQIC_TB1_CTRL0_pwrds_xrsp_req_sel_START (3)
#define SOC_SYSQIC_TB1_CTRL0_pwrds_xrsp_req_sel_END (3)
#define SOC_SYSQIC_TB1_CTRL0_pwrds_xrsp_req_sw_val_START (4)
#define SOC_SYSQIC_TB1_CTRL0_pwrds_xrsp_req_sw_val_END (4)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int fc_in_credit_cnt : 8;
        unsigned int fc_in_fifo_full : 1;
        unsigned int fc_in_fifo_empty_n : 1;
        unsigned int fc_in_cmd_press : 1;
        unsigned int fc_in_active : 1;
        unsigned int pwrds_xreq_req : 1;
        unsigned int pwrds_xreq_ack : 1;
        unsigned int reserved : 18;
    } reg;
} SOC_SYSQIC_TB1_FC_IN_STAT0_UNION;
#endif
#define SOC_SYSQIC_TB1_FC_IN_STAT0_fc_in_credit_cnt_START (0)
#define SOC_SYSQIC_TB1_FC_IN_STAT0_fc_in_credit_cnt_END (7)
#define SOC_SYSQIC_TB1_FC_IN_STAT0_fc_in_fifo_full_START (8)
#define SOC_SYSQIC_TB1_FC_IN_STAT0_fc_in_fifo_full_END (8)
#define SOC_SYSQIC_TB1_FC_IN_STAT0_fc_in_fifo_empty_n_START (9)
#define SOC_SYSQIC_TB1_FC_IN_STAT0_fc_in_fifo_empty_n_END (9)
#define SOC_SYSQIC_TB1_FC_IN_STAT0_fc_in_cmd_press_START (10)
#define SOC_SYSQIC_TB1_FC_IN_STAT0_fc_in_cmd_press_END (10)
#define SOC_SYSQIC_TB1_FC_IN_STAT0_fc_in_active_START (11)
#define SOC_SYSQIC_TB1_FC_IN_STAT0_fc_in_active_END (11)
#define SOC_SYSQIC_TB1_FC_IN_STAT0_pwrds_xreq_req_START (12)
#define SOC_SYSQIC_TB1_FC_IN_STAT0_pwrds_xreq_req_END (12)
#define SOC_SYSQIC_TB1_FC_IN_STAT0_pwrds_xreq_ack_START (13)
#define SOC_SYSQIC_TB1_FC_IN_STAT0_pwrds_xreq_ack_END (13)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int qtp_dec_cmd_press0 : 1;
        unsigned int qtp_dec_cmd_press1 : 1;
        unsigned int qtp_dec_cmd_press2 : 1;
        unsigned int qtp_dec_cmd_press3 : 1;
        unsigned int qtp_dec_cmd_press4 : 1;
        unsigned int qtp_dec_cmd_press5 : 1;
        unsigned int qtp_dec_cmd_press6 : 1;
        unsigned int qtp_dec_cmd_press7 : 1;
        unsigned int qtp_dec_defslv_press : 1;
        unsigned int qtp_dec_active : 1;
        unsigned int reserved : 22;
    } reg;
} SOC_SYSQIC_TB1_QTP_DEC_STAT0_UNION;
#endif
#define SOC_SYSQIC_TB1_QTP_DEC_STAT0_qtp_dec_cmd_press0_START (0)
#define SOC_SYSQIC_TB1_QTP_DEC_STAT0_qtp_dec_cmd_press0_END (0)
#define SOC_SYSQIC_TB1_QTP_DEC_STAT0_qtp_dec_cmd_press1_START (1)
#define SOC_SYSQIC_TB1_QTP_DEC_STAT0_qtp_dec_cmd_press1_END (1)
#define SOC_SYSQIC_TB1_QTP_DEC_STAT0_qtp_dec_cmd_press2_START (2)
#define SOC_SYSQIC_TB1_QTP_DEC_STAT0_qtp_dec_cmd_press2_END (2)
#define SOC_SYSQIC_TB1_QTP_DEC_STAT0_qtp_dec_cmd_press3_START (3)
#define SOC_SYSQIC_TB1_QTP_DEC_STAT0_qtp_dec_cmd_press3_END (3)
#define SOC_SYSQIC_TB1_QTP_DEC_STAT0_qtp_dec_cmd_press4_START (4)
#define SOC_SYSQIC_TB1_QTP_DEC_STAT0_qtp_dec_cmd_press4_END (4)
#define SOC_SYSQIC_TB1_QTP_DEC_STAT0_qtp_dec_cmd_press5_START (5)
#define SOC_SYSQIC_TB1_QTP_DEC_STAT0_qtp_dec_cmd_press5_END (5)
#define SOC_SYSQIC_TB1_QTP_DEC_STAT0_qtp_dec_cmd_press6_START (6)
#define SOC_SYSQIC_TB1_QTP_DEC_STAT0_qtp_dec_cmd_press6_END (6)
#define SOC_SYSQIC_TB1_QTP_DEC_STAT0_qtp_dec_cmd_press7_START (7)
#define SOC_SYSQIC_TB1_QTP_DEC_STAT0_qtp_dec_cmd_press7_END (7)
#define SOC_SYSQIC_TB1_QTP_DEC_STAT0_qtp_dec_defslv_press_START (8)
#define SOC_SYSQIC_TB1_QTP_DEC_STAT0_qtp_dec_defslv_press_END (8)
#define SOC_SYSQIC_TB1_QTP_DEC_STAT0_qtp_dec_active_START (9)
#define SOC_SYSQIC_TB1_QTP_DEC_STAT0_qtp_dec_active_END (9)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int qtp_enc_rsp_press0 : 1;
        unsigned int qtp_enc_rsp_press1 : 1;
        unsigned int qtp_enc_rsp_press2 : 1;
        unsigned int qtp_enc_rsp_press3 : 1;
        unsigned int qtp_enc_rsp_press4 : 1;
        unsigned int qtp_enc_rsp_press5 : 1;
        unsigned int qtp_enc_rsp_press6 : 1;
        unsigned int qtp_enc_rsp_press7 : 1;
        unsigned int qtp_enc_defslv_press : 1;
        unsigned int qtp_enc_active : 1;
        unsigned int reserved : 22;
    } reg;
} SOC_SYSQIC_TB1_QTP_ENC_STAT0_UNION;
#endif
#define SOC_SYSQIC_TB1_QTP_ENC_STAT0_qtp_enc_rsp_press0_START (0)
#define SOC_SYSQIC_TB1_QTP_ENC_STAT0_qtp_enc_rsp_press0_END (0)
#define SOC_SYSQIC_TB1_QTP_ENC_STAT0_qtp_enc_rsp_press1_START (1)
#define SOC_SYSQIC_TB1_QTP_ENC_STAT0_qtp_enc_rsp_press1_END (1)
#define SOC_SYSQIC_TB1_QTP_ENC_STAT0_qtp_enc_rsp_press2_START (2)
#define SOC_SYSQIC_TB1_QTP_ENC_STAT0_qtp_enc_rsp_press2_END (2)
#define SOC_SYSQIC_TB1_QTP_ENC_STAT0_qtp_enc_rsp_press3_START (3)
#define SOC_SYSQIC_TB1_QTP_ENC_STAT0_qtp_enc_rsp_press3_END (3)
#define SOC_SYSQIC_TB1_QTP_ENC_STAT0_qtp_enc_rsp_press4_START (4)
#define SOC_SYSQIC_TB1_QTP_ENC_STAT0_qtp_enc_rsp_press4_END (4)
#define SOC_SYSQIC_TB1_QTP_ENC_STAT0_qtp_enc_rsp_press5_START (5)
#define SOC_SYSQIC_TB1_QTP_ENC_STAT0_qtp_enc_rsp_press5_END (5)
#define SOC_SYSQIC_TB1_QTP_ENC_STAT0_qtp_enc_rsp_press6_START (6)
#define SOC_SYSQIC_TB1_QTP_ENC_STAT0_qtp_enc_rsp_press6_END (6)
#define SOC_SYSQIC_TB1_QTP_ENC_STAT0_qtp_enc_rsp_press7_START (7)
#define SOC_SYSQIC_TB1_QTP_ENC_STAT0_qtp_enc_rsp_press7_END (7)
#define SOC_SYSQIC_TB1_QTP_ENC_STAT0_qtp_enc_defslv_press_START (8)
#define SOC_SYSQIC_TB1_QTP_ENC_STAT0_qtp_enc_defslv_press_END (8)
#define SOC_SYSQIC_TB1_QTP_ENC_STAT0_qtp_enc_active_START (9)
#define SOC_SYSQIC_TB1_QTP_ENC_STAT0_qtp_enc_active_END (9)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int fc_out_credit_cnt : 8;
        unsigned int fc_out_fifo_full : 1;
        unsigned int fc_out_fifo_empty_n : 1;
        unsigned int fc_out_press : 1;
        unsigned int fc_out_active : 1;
        unsigned int pwrds_xrsp_req : 1;
        unsigned int pwrds_xrsp_ack : 1;
        unsigned int reserved : 18;
    } reg;
} SOC_SYSQIC_TB1_FC_OUT_STAT0_UNION;
#endif
#define SOC_SYSQIC_TB1_FC_OUT_STAT0_fc_out_credit_cnt_START (0)
#define SOC_SYSQIC_TB1_FC_OUT_STAT0_fc_out_credit_cnt_END (7)
#define SOC_SYSQIC_TB1_FC_OUT_STAT0_fc_out_fifo_full_START (8)
#define SOC_SYSQIC_TB1_FC_OUT_STAT0_fc_out_fifo_full_END (8)
#define SOC_SYSQIC_TB1_FC_OUT_STAT0_fc_out_fifo_empty_n_START (9)
#define SOC_SYSQIC_TB1_FC_OUT_STAT0_fc_out_fifo_empty_n_END (9)
#define SOC_SYSQIC_TB1_FC_OUT_STAT0_fc_out_press_START (10)
#define SOC_SYSQIC_TB1_FC_OUT_STAT0_fc_out_press_END (10)
#define SOC_SYSQIC_TB1_FC_OUT_STAT0_fc_out_active_START (11)
#define SOC_SYSQIC_TB1_FC_OUT_STAT0_fc_out_active_END (11)
#define SOC_SYSQIC_TB1_FC_OUT_STAT0_pwrds_xrsp_req_START (12)
#define SOC_SYSQIC_TB1_FC_OUT_STAT0_pwrds_xrsp_req_END (12)
#define SOC_SYSQIC_TB1_FC_OUT_STAT0_pwrds_xrsp_ack_START (13)
#define SOC_SYSQIC_TB1_FC_OUT_STAT0_pwrds_xrsp_ack_END (13)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int tsize_left : 8;
        unsigned int cur_state : 2;
        unsigned int split_in_proc : 1;
        unsigned int reserved_0 : 1;
        unsigned int qtp_cnt : 5;
        unsigned int reserved_1 : 3;
        unsigned int flit_cnt : 5;
        unsigned int flit_fifo_empty_n : 1;
        unsigned int flit_fifo_full : 1;
        unsigned int tsize_fifo_empty_n : 1;
        unsigned int tsize_fifo_full : 1;
        unsigned int gm_active : 1;
        unsigned int reserved_2 : 2;
    } reg;
} SOC_SYSQIC_TB1_GM_STAT0_UNION;
#endif
#define SOC_SYSQIC_TB1_GM_STAT0_tsize_left_START (0)
#define SOC_SYSQIC_TB1_GM_STAT0_tsize_left_END (7)
#define SOC_SYSQIC_TB1_GM_STAT0_cur_state_START (8)
#define SOC_SYSQIC_TB1_GM_STAT0_cur_state_END (9)
#define SOC_SYSQIC_TB1_GM_STAT0_split_in_proc_START (10)
#define SOC_SYSQIC_TB1_GM_STAT0_split_in_proc_END (10)
#define SOC_SYSQIC_TB1_GM_STAT0_qtp_cnt_START (12)
#define SOC_SYSQIC_TB1_GM_STAT0_qtp_cnt_END (16)
#define SOC_SYSQIC_TB1_GM_STAT0_flit_cnt_START (20)
#define SOC_SYSQIC_TB1_GM_STAT0_flit_cnt_END (24)
#define SOC_SYSQIC_TB1_GM_STAT0_flit_fifo_empty_n_START (25)
#define SOC_SYSQIC_TB1_GM_STAT0_flit_fifo_empty_n_END (25)
#define SOC_SYSQIC_TB1_GM_STAT0_flit_fifo_full_START (26)
#define SOC_SYSQIC_TB1_GM_STAT0_flit_fifo_full_END (26)
#define SOC_SYSQIC_TB1_GM_STAT0_tsize_fifo_empty_n_START (27)
#define SOC_SYSQIC_TB1_GM_STAT0_tsize_fifo_empty_n_END (27)
#define SOC_SYSQIC_TB1_GM_STAT0_tsize_fifo_full_START (28)
#define SOC_SYSQIC_TB1_GM_STAT0_tsize_fifo_full_END (28)
#define SOC_SYSQIC_TB1_GM_STAT0_gm_active_START (29)
#define SOC_SYSQIC_TB1_GM_STAT0_gm_active_END (29)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int qic_cmd_press : 1;
        unsigned int split_cmd_press : 1;
        unsigned int amba_rsp_press : 1;
        unsigned int qic_rsp_press : 1;
        unsigned int rsp_ibid : 8;
        unsigned int rsp_pktid : 7;
        unsigned int rsp_pending : 1;
        unsigned int reserved : 12;
    } reg;
} SOC_SYSQIC_TB1_GM_STAT1_UNION;
#endif
#define SOC_SYSQIC_TB1_GM_STAT1_qic_cmd_press_START (0)
#define SOC_SYSQIC_TB1_GM_STAT1_qic_cmd_press_END (0)
#define SOC_SYSQIC_TB1_GM_STAT1_split_cmd_press_START (1)
#define SOC_SYSQIC_TB1_GM_STAT1_split_cmd_press_END (1)
#define SOC_SYSQIC_TB1_GM_STAT1_amba_rsp_press_START (2)
#define SOC_SYSQIC_TB1_GM_STAT1_amba_rsp_press_END (2)
#define SOC_SYSQIC_TB1_GM_STAT1_qic_rsp_press_START (3)
#define SOC_SYSQIC_TB1_GM_STAT1_qic_rsp_press_END (3)
#define SOC_SYSQIC_TB1_GM_STAT1_rsp_ibid_START (4)
#define SOC_SYSQIC_TB1_GM_STAT1_rsp_ibid_END (11)
#define SOC_SYSQIC_TB1_GM_STAT1_rsp_pktid_START (12)
#define SOC_SYSQIC_TB1_GM_STAT1_rsp_pktid_END (18)
#define SOC_SYSQIC_TB1_GM_STAT1_rsp_pending_START (19)
#define SOC_SYSQIC_TB1_GM_STAT1_rsp_pending_END (19)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int pending_cmd0 : 32;
    } reg;
} SOC_SYSQIC_TB1_GM_STAT2_UNION;
#endif
#define SOC_SYSQIC_TB1_GM_STAT2_pending_cmd0_START (0)
#define SOC_SYSQIC_TB1_GM_STAT2_pending_cmd0_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int pending_cmd1 : 32;
    } reg;
} SOC_SYSQIC_TB1_GM_STAT3_UNION;
#endif
#define SOC_SYSQIC_TB1_GM_STAT3_pending_cmd1_START (0)
#define SOC_SYSQIC_TB1_GM_STAT3_pending_cmd1_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int pending_cmd2 : 32;
    } reg;
} SOC_SYSQIC_TB1_GM_STAT4_UNION;
#endif
#define SOC_SYSQIC_TB1_GM_STAT4_pending_cmd2_START (0)
#define SOC_SYSQIC_TB1_GM_STAT4_pending_cmd2_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int pending_cmd3 : 32;
    } reg;
} SOC_SYSQIC_TB1_GM_STAT5_UNION;
#endif
#define SOC_SYSQIC_TB1_GM_STAT5_pending_cmd3_START (0)
#define SOC_SYSQIC_TB1_GM_STAT5_pending_cmd3_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int timeout_mask : 1;
        unsigned int hop_miss_mask : 1;
        unsigned int hop_b2b_mask : 1;
        unsigned int unsupport_opc_mask : 1;
        unsigned int unsupport_tbid_mask : 1;
        unsigned int unsupport_ibid_mask : 1;
        unsigned int wrap_lt32b_mask : 1;
        unsigned int wrap_unaligned_mask : 1;
        unsigned int wrap_n2pow_mask : 1;
        unsigned int wrap_narrow_mask : 1;
        unsigned int slverror_mask : 1;
        unsigned int tsize_top_unmatch_mask : 1;
        unsigned int trans_over_burst16_mask : 1;
        unsigned int reserved : 16;
        unsigned int flow_sel : 3;
    } reg;
} SOC_SYSQIC_TB1_DFX_CTRL0_UNION;
#endif
#define SOC_SYSQIC_TB1_DFX_CTRL0_timeout_mask_START (0)
#define SOC_SYSQIC_TB1_DFX_CTRL0_timeout_mask_END (0)
#define SOC_SYSQIC_TB1_DFX_CTRL0_hop_miss_mask_START (1)
#define SOC_SYSQIC_TB1_DFX_CTRL0_hop_miss_mask_END (1)
#define SOC_SYSQIC_TB1_DFX_CTRL0_hop_b2b_mask_START (2)
#define SOC_SYSQIC_TB1_DFX_CTRL0_hop_b2b_mask_END (2)
#define SOC_SYSQIC_TB1_DFX_CTRL0_unsupport_opc_mask_START (3)
#define SOC_SYSQIC_TB1_DFX_CTRL0_unsupport_opc_mask_END (3)
#define SOC_SYSQIC_TB1_DFX_CTRL0_unsupport_tbid_mask_START (4)
#define SOC_SYSQIC_TB1_DFX_CTRL0_unsupport_tbid_mask_END (4)
#define SOC_SYSQIC_TB1_DFX_CTRL0_unsupport_ibid_mask_START (5)
#define SOC_SYSQIC_TB1_DFX_CTRL0_unsupport_ibid_mask_END (5)
#define SOC_SYSQIC_TB1_DFX_CTRL0_wrap_lt32b_mask_START (6)
#define SOC_SYSQIC_TB1_DFX_CTRL0_wrap_lt32b_mask_END (6)
#define SOC_SYSQIC_TB1_DFX_CTRL0_wrap_unaligned_mask_START (7)
#define SOC_SYSQIC_TB1_DFX_CTRL0_wrap_unaligned_mask_END (7)
#define SOC_SYSQIC_TB1_DFX_CTRL0_wrap_n2pow_mask_START (8)
#define SOC_SYSQIC_TB1_DFX_CTRL0_wrap_n2pow_mask_END (8)
#define SOC_SYSQIC_TB1_DFX_CTRL0_wrap_narrow_mask_START (9)
#define SOC_SYSQIC_TB1_DFX_CTRL0_wrap_narrow_mask_END (9)
#define SOC_SYSQIC_TB1_DFX_CTRL0_slverror_mask_START (10)
#define SOC_SYSQIC_TB1_DFX_CTRL0_slverror_mask_END (10)
#define SOC_SYSQIC_TB1_DFX_CTRL0_tsize_top_unmatch_mask_START (11)
#define SOC_SYSQIC_TB1_DFX_CTRL0_tsize_top_unmatch_mask_END (11)
#define SOC_SYSQIC_TB1_DFX_CTRL0_trans_over_burst16_mask_START (12)
#define SOC_SYSQIC_TB1_DFX_CTRL0_trans_over_burst16_mask_END (12)
#define SOC_SYSQIC_TB1_DFX_CTRL0_flow_sel_START (29)
#define SOC_SYSQIC_TB1_DFX_CTRL0_flow_sel_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int timeout_detected : 1;
        unsigned int hop_miss_detected : 1;
        unsigned int hop_b2b_detected : 1;
        unsigned int unsupport_opc_detected : 1;
        unsigned int unsupport_tbid_detected : 1;
        unsigned int unsupport_ibid_detected : 1;
        unsigned int wrap_lt32b_detected : 1;
        unsigned int wrap_unaligned_detected : 1;
        unsigned int wrap_n2pow_detected : 1;
        unsigned int wrap_narrow_detected : 1;
        unsigned int slverror_detected : 1;
        unsigned int tsize_top_unmatch_detected : 1;
        unsigned int trans_over_burst16_detected : 1;
        unsigned int reserved : 19;
    } reg;
} SOC_SYSQIC_TB1_DFX_STAT0_UNION;
#endif
#define SOC_SYSQIC_TB1_DFX_STAT0_timeout_detected_START (0)
#define SOC_SYSQIC_TB1_DFX_STAT0_timeout_detected_END (0)
#define SOC_SYSQIC_TB1_DFX_STAT0_hop_miss_detected_START (1)
#define SOC_SYSQIC_TB1_DFX_STAT0_hop_miss_detected_END (1)
#define SOC_SYSQIC_TB1_DFX_STAT0_hop_b2b_detected_START (2)
#define SOC_SYSQIC_TB1_DFX_STAT0_hop_b2b_detected_END (2)
#define SOC_SYSQIC_TB1_DFX_STAT0_unsupport_opc_detected_START (3)
#define SOC_SYSQIC_TB1_DFX_STAT0_unsupport_opc_detected_END (3)
#define SOC_SYSQIC_TB1_DFX_STAT0_unsupport_tbid_detected_START (4)
#define SOC_SYSQIC_TB1_DFX_STAT0_unsupport_tbid_detected_END (4)
#define SOC_SYSQIC_TB1_DFX_STAT0_unsupport_ibid_detected_START (5)
#define SOC_SYSQIC_TB1_DFX_STAT0_unsupport_ibid_detected_END (5)
#define SOC_SYSQIC_TB1_DFX_STAT0_wrap_lt32b_detected_START (6)
#define SOC_SYSQIC_TB1_DFX_STAT0_wrap_lt32b_detected_END (6)
#define SOC_SYSQIC_TB1_DFX_STAT0_wrap_unaligned_detected_START (7)
#define SOC_SYSQIC_TB1_DFX_STAT0_wrap_unaligned_detected_END (7)
#define SOC_SYSQIC_TB1_DFX_STAT0_wrap_n2pow_detected_START (8)
#define SOC_SYSQIC_TB1_DFX_STAT0_wrap_n2pow_detected_END (8)
#define SOC_SYSQIC_TB1_DFX_STAT0_wrap_narrow_detected_START (9)
#define SOC_SYSQIC_TB1_DFX_STAT0_wrap_narrow_detected_END (9)
#define SOC_SYSQIC_TB1_DFX_STAT0_slverror_detected_START (10)
#define SOC_SYSQIC_TB1_DFX_STAT0_slverror_detected_END (10)
#define SOC_SYSQIC_TB1_DFX_STAT0_tsize_top_unmatch_detected_START (11)
#define SOC_SYSQIC_TB1_DFX_STAT0_tsize_top_unmatch_detected_END (11)
#define SOC_SYSQIC_TB1_DFX_STAT0_trans_over_burst16_detected_START (12)
#define SOC_SYSQIC_TB1_DFX_STAT0_trans_over_burst16_detected_END (12)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int pending_rd_trans : 1;
        unsigned int rd_outstanding : 5;
        unsigned int pending_wr_trans : 1;
        unsigned int wr_outstanding : 5;
        unsigned int reserved : 20;
    } reg;
} SOC_SYSQIC_TB1_DFX_STAT1_UNION;
#endif
#define SOC_SYSQIC_TB1_DFX_STAT1_pending_rd_trans_START (0)
#define SOC_SYSQIC_TB1_DFX_STAT1_pending_rd_trans_END (0)
#define SOC_SYSQIC_TB1_DFX_STAT1_rd_outstanding_START (1)
#define SOC_SYSQIC_TB1_DFX_STAT1_rd_outstanding_END (5)
#define SOC_SYSQIC_TB1_DFX_STAT1_pending_wr_trans_START (6)
#define SOC_SYSQIC_TB1_DFX_STAT1_pending_wr_trans_END (6)
#define SOC_SYSQIC_TB1_DFX_STAT1_wr_outstanding_START (7)
#define SOC_SYSQIC_TB1_DFX_STAT1_wr_outstanding_END (11)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int error_cmd0 : 32;
    } reg;
} SOC_SYSQIC_TB1_DFX_STAT2_UNION;
#endif
#define SOC_SYSQIC_TB1_DFX_STAT2_error_cmd0_START (0)
#define SOC_SYSQIC_TB1_DFX_STAT2_error_cmd0_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int error_cmd1 : 32;
    } reg;
} SOC_SYSQIC_TB1_DFX_STAT3_UNION;
#endif
#define SOC_SYSQIC_TB1_DFX_STAT3_error_cmd1_START (0)
#define SOC_SYSQIC_TB1_DFX_STAT3_error_cmd1_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int error_cmd2 : 32;
    } reg;
} SOC_SYSQIC_TB1_DFX_STAT4_UNION;
#endif
#define SOC_SYSQIC_TB1_DFX_STAT4_error_cmd2_START (0)
#define SOC_SYSQIC_TB1_DFX_STAT4_error_cmd2_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int error_cmd3 : 32;
    } reg;
} SOC_SYSQIC_TB1_DFX_STAT5_UNION;
#endif
#define SOC_SYSQIC_TB1_DFX_STAT5_error_cmd3_START (0)
#define SOC_SYSQIC_TB1_DFX_STAT5_error_cmd3_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int clk_sel_fc_in : 1;
        unsigned int clk_sel_qtp_dec : 1;
        unsigned int clk_sel_gm : 1;
        unsigned int clk_sel_qtp_enc : 1;
        unsigned int clk_sel_fc_out : 1;
        unsigned int clk_sel_defslv : 1;
        unsigned int clk_sel_qcr : 1;
        unsigned int clk_sel_tb : 1;
        unsigned int reserved : 24;
    } reg;
} SOC_SYSQIC_TB1_CG_CTRL0_UNION;
#endif
#define SOC_SYSQIC_TB1_CG_CTRL0_clk_sel_fc_in_START (0)
#define SOC_SYSQIC_TB1_CG_CTRL0_clk_sel_fc_in_END (0)
#define SOC_SYSQIC_TB1_CG_CTRL0_clk_sel_qtp_dec_START (1)
#define SOC_SYSQIC_TB1_CG_CTRL0_clk_sel_qtp_dec_END (1)
#define SOC_SYSQIC_TB1_CG_CTRL0_clk_sel_gm_START (2)
#define SOC_SYSQIC_TB1_CG_CTRL0_clk_sel_gm_END (2)
#define SOC_SYSQIC_TB1_CG_CTRL0_clk_sel_qtp_enc_START (3)
#define SOC_SYSQIC_TB1_CG_CTRL0_clk_sel_qtp_enc_END (3)
#define SOC_SYSQIC_TB1_CG_CTRL0_clk_sel_fc_out_START (4)
#define SOC_SYSQIC_TB1_CG_CTRL0_clk_sel_fc_out_END (4)
#define SOC_SYSQIC_TB1_CG_CTRL0_clk_sel_defslv_START (5)
#define SOC_SYSQIC_TB1_CG_CTRL0_clk_sel_defslv_END (5)
#define SOC_SYSQIC_TB1_CG_CTRL0_clk_sel_qcr_START (6)
#define SOC_SYSQIC_TB1_CG_CTRL0_clk_sel_qcr_END (6)
#define SOC_SYSQIC_TB1_CG_CTRL0_clk_sel_tb_START (7)
#define SOC_SYSQIC_TB1_CG_CTRL0_clk_sel_tb_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int reserved : 32;
    } reg;
} SOC_SYSQIC_TB1_RESERVED_RW_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int reserved : 32;
    } reg;
} SOC_SYSQIC_TB1_RESERVED_RO_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int int_en : 16;
        unsigned int reserved : 16;
    } reg;
} SOC_SYSQIC_TB1_INT_EN_UNION;
#endif
#define SOC_SYSQIC_TB1_INT_EN_int_en_START (0)
#define SOC_SYSQIC_TB1_INT_EN_int_en_END (15)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int int_clr : 16;
        unsigned int reserved : 16;
    } reg;
} SOC_SYSQIC_TB1_INT_CLR_UNION;
#endif
#define SOC_SYSQIC_TB1_INT_CLR_int_clr_START (0)
#define SOC_SYSQIC_TB1_INT_CLR_int_clr_END (15)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int int_st : 16;
        unsigned int int_st_lock : 16;
    } reg;
} SOC_SYSQIC_TB1_INT_STATUS_UNION;
#endif
#define SOC_SYSQIC_TB1_INT_STATUS_int_st_START (0)
#define SOC_SYSQIC_TB1_INT_STATUS_int_st_END (15)
#define SOC_SYSQIC_TB1_INT_STATUS_int_st_lock_START (16)
#define SOC_SYSQIC_TB1_INT_STATUS_int_st_lock_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int parity_chk_en : 1;
        unsigned int reserved : 31;
    } reg;
} SOC_SYSQIC_TB1_CTRL1_UNION;
#endif
#define SOC_SYSQIC_TB1_CTRL1_parity_chk_en_START (0)
#define SOC_SYSQIC_TB1_CTRL1_parity_chk_en_END (0)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int fencing_cmd_latency : 4;
        unsigned int lp_sig_latency : 4;
        unsigned int bypass_clkgt_defslv : 1;
        unsigned int bypass_clkgt_lpcmon : 1;
        unsigned int soft_link_up : 1;
        unsigned int reserved : 21;
    } reg;
} SOC_SYSQIC_LPC_CTRL0_UNION;
#endif
#define SOC_SYSQIC_LPC_CTRL0_fencing_cmd_latency_START (0)
#define SOC_SYSQIC_LPC_CTRL0_fencing_cmd_latency_END (3)
#define SOC_SYSQIC_LPC_CTRL0_lp_sig_latency_START (4)
#define SOC_SYSQIC_LPC_CTRL0_lp_sig_latency_END (7)
#define SOC_SYSQIC_LPC_CTRL0_bypass_clkgt_defslv_START (8)
#define SOC_SYSQIC_LPC_CTRL0_bypass_clkgt_defslv_END (8)
#define SOC_SYSQIC_LPC_CTRL0_bypass_clkgt_lpcmon_START (9)
#define SOC_SYSQIC_LPC_CTRL0_bypass_clkgt_lpcmon_END (9)
#define SOC_SYSQIC_LPC_CTRL0_soft_link_up_START (10)
#define SOC_SYSQIC_LPC_CTRL0_soft_link_up_END (10)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int lpcm_ctrl_revd : 32;
    } reg;
} SOC_SYSQIC_LPC_CTRL1_UNION;
#endif
#define SOC_SYSQIC_LPC_CTRL1_lpcm_ctrl_revd_START (0)
#define SOC_SYSQIC_LPC_CTRL1_lpcm_ctrl_revd_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int rd_errstat_cmd_nohop : 1;
        unsigned int rd_errstat_cmd_notop : 1;
        unsigned int rd_errstat_unknow_srcid : 1;
        unsigned int rd_errstat_unknow_opc : 1;
        unsigned int wr_errstat_cmd_nohop : 1;
        unsigned int wr_errstat_cmd_notop : 1;
        unsigned int wr_errstat_unknow_srcid : 1;
        unsigned int wr_errstat_unknow_opc : 1;
        unsigned int cfg_errstat_cmd_nohop : 1;
        unsigned int cfg_errstat_cmd_notop : 1;
        unsigned int cfg_errstat_unknow_srcid : 1;
        unsigned int cfg_errstat_unknow_opc : 1;
        unsigned int cfg_cmd_aft_fencing : 1;
        unsigned int rd_cmd_aft_fencing : 1;
        unsigned int wr_cmd_aft_fencing : 1;
        unsigned int reserved : 17;
    } reg;
} SOC_SYSQIC_LPC_ERR_STAT_UNION;
#endif
#define SOC_SYSQIC_LPC_ERR_STAT_rd_errstat_cmd_nohop_START (0)
#define SOC_SYSQIC_LPC_ERR_STAT_rd_errstat_cmd_nohop_END (0)
#define SOC_SYSQIC_LPC_ERR_STAT_rd_errstat_cmd_notop_START (1)
#define SOC_SYSQIC_LPC_ERR_STAT_rd_errstat_cmd_notop_END (1)
#define SOC_SYSQIC_LPC_ERR_STAT_rd_errstat_unknow_srcid_START (2)
#define SOC_SYSQIC_LPC_ERR_STAT_rd_errstat_unknow_srcid_END (2)
#define SOC_SYSQIC_LPC_ERR_STAT_rd_errstat_unknow_opc_START (3)
#define SOC_SYSQIC_LPC_ERR_STAT_rd_errstat_unknow_opc_END (3)
#define SOC_SYSQIC_LPC_ERR_STAT_wr_errstat_cmd_nohop_START (4)
#define SOC_SYSQIC_LPC_ERR_STAT_wr_errstat_cmd_nohop_END (4)
#define SOC_SYSQIC_LPC_ERR_STAT_wr_errstat_cmd_notop_START (5)
#define SOC_SYSQIC_LPC_ERR_STAT_wr_errstat_cmd_notop_END (5)
#define SOC_SYSQIC_LPC_ERR_STAT_wr_errstat_unknow_srcid_START (6)
#define SOC_SYSQIC_LPC_ERR_STAT_wr_errstat_unknow_srcid_END (6)
#define SOC_SYSQIC_LPC_ERR_STAT_wr_errstat_unknow_opc_START (7)
#define SOC_SYSQIC_LPC_ERR_STAT_wr_errstat_unknow_opc_END (7)
#define SOC_SYSQIC_LPC_ERR_STAT_cfg_errstat_cmd_nohop_START (8)
#define SOC_SYSQIC_LPC_ERR_STAT_cfg_errstat_cmd_nohop_END (8)
#define SOC_SYSQIC_LPC_ERR_STAT_cfg_errstat_cmd_notop_START (9)
#define SOC_SYSQIC_LPC_ERR_STAT_cfg_errstat_cmd_notop_END (9)
#define SOC_SYSQIC_LPC_ERR_STAT_cfg_errstat_unknow_srcid_START (10)
#define SOC_SYSQIC_LPC_ERR_STAT_cfg_errstat_unknow_srcid_END (10)
#define SOC_SYSQIC_LPC_ERR_STAT_cfg_errstat_unknow_opc_START (11)
#define SOC_SYSQIC_LPC_ERR_STAT_cfg_errstat_unknow_opc_END (11)
#define SOC_SYSQIC_LPC_ERR_STAT_cfg_cmd_aft_fencing_START (12)
#define SOC_SYSQIC_LPC_ERR_STAT_cfg_cmd_aft_fencing_END (12)
#define SOC_SYSQIC_LPC_ERR_STAT_rd_cmd_aft_fencing_START (13)
#define SOC_SYSQIC_LPC_ERR_STAT_rd_cmd_aft_fencing_END (13)
#define SOC_SYSQIC_LPC_ERR_STAT_wr_cmd_aft_fencing_START (14)
#define SOC_SYSQIC_LPC_ERR_STAT_wr_cmd_aft_fencing_END (14)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int rd_err_stat_clr : 4;
        unsigned int wr_err_stat_clr : 4;
        unsigned int cfg_err_stat_clr : 4;
        unsigned int fencing_err_stat_clr : 3;
        unsigned int reserved : 17;
    } reg;
} SOC_SYSQIC_LPC_ERRSTAT_CLR_UNION;
#endif
#define SOC_SYSQIC_LPC_ERRSTAT_CLR_rd_err_stat_clr_START (0)
#define SOC_SYSQIC_LPC_ERRSTAT_CLR_rd_err_stat_clr_END (3)
#define SOC_SYSQIC_LPC_ERRSTAT_CLR_wr_err_stat_clr_START (4)
#define SOC_SYSQIC_LPC_ERRSTAT_CLR_wr_err_stat_clr_END (7)
#define SOC_SYSQIC_LPC_ERRSTAT_CLR_cfg_err_stat_clr_START (8)
#define SOC_SYSQIC_LPC_ERRSTAT_CLR_cfg_err_stat_clr_END (11)
#define SOC_SYSQIC_LPC_ERRSTAT_CLR_fencing_err_stat_clr_START (12)
#define SOC_SYSQIC_LPC_ERRSTAT_CLR_fencing_err_stat_clr_END (14)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int dfx_rd_trans_pending : 1;
        unsigned int dfx_wr_trans_pending : 1;
        unsigned int dfx_trans_pending : 1;
        unsigned int dfx_defslv_nosel : 1;
        unsigned int pw_stat : 5;
        unsigned int busy_lpc_mon : 1;
        unsigned int dfx_lpc_main_busy : 1;
        unsigned int reserved : 21;
    } reg;
} SOC_SYSQIC_LPC_STAT1_UNION;
#endif
#define SOC_SYSQIC_LPC_STAT1_dfx_rd_trans_pending_START (0)
#define SOC_SYSQIC_LPC_STAT1_dfx_rd_trans_pending_END (0)
#define SOC_SYSQIC_LPC_STAT1_dfx_wr_trans_pending_START (1)
#define SOC_SYSQIC_LPC_STAT1_dfx_wr_trans_pending_END (1)
#define SOC_SYSQIC_LPC_STAT1_dfx_trans_pending_START (2)
#define SOC_SYSQIC_LPC_STAT1_dfx_trans_pending_END (2)
#define SOC_SYSQIC_LPC_STAT1_dfx_defslv_nosel_START (3)
#define SOC_SYSQIC_LPC_STAT1_dfx_defslv_nosel_END (3)
#define SOC_SYSQIC_LPC_STAT1_pw_stat_START (4)
#define SOC_SYSQIC_LPC_STAT1_pw_stat_END (8)
#define SOC_SYSQIC_LPC_STAT1_busy_lpc_mon_START (9)
#define SOC_SYSQIC_LPC_STAT1_busy_lpc_mon_END (9)
#define SOC_SYSQIC_LPC_STAT1_dfx_lpc_main_busy_START (10)
#define SOC_SYSQIC_LPC_STAT1_dfx_lpc_main_busy_END (10)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int dfx_rreq_fifo_st : 8;
        unsigned int dfx_wreq_fifo_st : 8;
        unsigned int dfx_rrsp_fifo_st : 2;
        unsigned int dfx_wrsp_fifo_st : 2;
        unsigned int dfx_cfg_req_fifo_st : 2;
        unsigned int dfx_cfg_rsp_fifo_st : 2;
        unsigned int dfx_rreq_backpress : 1;
        unsigned int dfx_wreq_backpress : 1;
        unsigned int dfx_rrsp_backpress : 1;
        unsigned int dfx_wrsp_backpress : 1;
        unsigned int dfx_cfg_req_backpress : 1;
        unsigned int dfx_cfg_rsp_backpress : 1;
        unsigned int reserved : 2;
    } reg;
} SOC_SYSQIC_LPC_STAT2_UNION;
#endif
#define SOC_SYSQIC_LPC_STAT2_dfx_rreq_fifo_st_START (0)
#define SOC_SYSQIC_LPC_STAT2_dfx_rreq_fifo_st_END (7)
#define SOC_SYSQIC_LPC_STAT2_dfx_wreq_fifo_st_START (8)
#define SOC_SYSQIC_LPC_STAT2_dfx_wreq_fifo_st_END (15)
#define SOC_SYSQIC_LPC_STAT2_dfx_rrsp_fifo_st_START (16)
#define SOC_SYSQIC_LPC_STAT2_dfx_rrsp_fifo_st_END (17)
#define SOC_SYSQIC_LPC_STAT2_dfx_wrsp_fifo_st_START (18)
#define SOC_SYSQIC_LPC_STAT2_dfx_wrsp_fifo_st_END (19)
#define SOC_SYSQIC_LPC_STAT2_dfx_cfg_req_fifo_st_START (20)
#define SOC_SYSQIC_LPC_STAT2_dfx_cfg_req_fifo_st_END (21)
#define SOC_SYSQIC_LPC_STAT2_dfx_cfg_rsp_fifo_st_START (22)
#define SOC_SYSQIC_LPC_STAT2_dfx_cfg_rsp_fifo_st_END (23)
#define SOC_SYSQIC_LPC_STAT2_dfx_rreq_backpress_START (24)
#define SOC_SYSQIC_LPC_STAT2_dfx_rreq_backpress_END (24)
#define SOC_SYSQIC_LPC_STAT2_dfx_wreq_backpress_START (25)
#define SOC_SYSQIC_LPC_STAT2_dfx_wreq_backpress_END (25)
#define SOC_SYSQIC_LPC_STAT2_dfx_rrsp_backpress_START (26)
#define SOC_SYSQIC_LPC_STAT2_dfx_rrsp_backpress_END (26)
#define SOC_SYSQIC_LPC_STAT2_dfx_wrsp_backpress_START (27)
#define SOC_SYSQIC_LPC_STAT2_dfx_wrsp_backpress_END (27)
#define SOC_SYSQIC_LPC_STAT2_dfx_cfg_req_backpress_START (28)
#define SOC_SYSQIC_LPC_STAT2_dfx_cfg_req_backpress_END (28)
#define SOC_SYSQIC_LPC_STAT2_dfx_cfg_rsp_backpress_START (29)
#define SOC_SYSQIC_LPC_STAT2_dfx_cfg_rsp_backpress_END (29)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int pwrdm_rreq_req : 1;
        unsigned int pwrdm_rreq_ack : 1;
        unsigned int pwrdm_wreq_req : 1;
        unsigned int pwrdm_wreq_ack : 1;
        unsigned int pwrdm_rrsp_req : 1;
        unsigned int pwrdm_rrsp_ack : 1;
        unsigned int pwrdm_wrsp_req : 1;
        unsigned int pwrdm_wrsp_ack : 1;
        unsigned int pwrdm_cfgreq_req : 1;
        unsigned int pwrdm_cfgreq_ack : 1;
        unsigned int pwrdm_cfgrsp_req : 1;
        unsigned int pwrdm_cfgrsp_ack : 1;
        unsigned int qtp_conn_req : 1;
        unsigned int qtp_conn_ack : 1;
        unsigned int qtp_slv_pwron : 1;
        unsigned int qtp_mst_pwron : 1;
        unsigned int reserved : 16;
    } reg;
} SOC_SYSQIC_LPC_STAT3_UNION;
#endif
#define SOC_SYSQIC_LPC_STAT3_pwrdm_rreq_req_START (0)
#define SOC_SYSQIC_LPC_STAT3_pwrdm_rreq_req_END (0)
#define SOC_SYSQIC_LPC_STAT3_pwrdm_rreq_ack_START (1)
#define SOC_SYSQIC_LPC_STAT3_pwrdm_rreq_ack_END (1)
#define SOC_SYSQIC_LPC_STAT3_pwrdm_wreq_req_START (2)
#define SOC_SYSQIC_LPC_STAT3_pwrdm_wreq_req_END (2)
#define SOC_SYSQIC_LPC_STAT3_pwrdm_wreq_ack_START (3)
#define SOC_SYSQIC_LPC_STAT3_pwrdm_wreq_ack_END (3)
#define SOC_SYSQIC_LPC_STAT3_pwrdm_rrsp_req_START (4)
#define SOC_SYSQIC_LPC_STAT3_pwrdm_rrsp_req_END (4)
#define SOC_SYSQIC_LPC_STAT3_pwrdm_rrsp_ack_START (5)
#define SOC_SYSQIC_LPC_STAT3_pwrdm_rrsp_ack_END (5)
#define SOC_SYSQIC_LPC_STAT3_pwrdm_wrsp_req_START (6)
#define SOC_SYSQIC_LPC_STAT3_pwrdm_wrsp_req_END (6)
#define SOC_SYSQIC_LPC_STAT3_pwrdm_wrsp_ack_START (7)
#define SOC_SYSQIC_LPC_STAT3_pwrdm_wrsp_ack_END (7)
#define SOC_SYSQIC_LPC_STAT3_pwrdm_cfgreq_req_START (8)
#define SOC_SYSQIC_LPC_STAT3_pwrdm_cfgreq_req_END (8)
#define SOC_SYSQIC_LPC_STAT3_pwrdm_cfgreq_ack_START (9)
#define SOC_SYSQIC_LPC_STAT3_pwrdm_cfgreq_ack_END (9)
#define SOC_SYSQIC_LPC_STAT3_pwrdm_cfgrsp_req_START (10)
#define SOC_SYSQIC_LPC_STAT3_pwrdm_cfgrsp_req_END (10)
#define SOC_SYSQIC_LPC_STAT3_pwrdm_cfgrsp_ack_START (11)
#define SOC_SYSQIC_LPC_STAT3_pwrdm_cfgrsp_ack_END (11)
#define SOC_SYSQIC_LPC_STAT3_qtp_conn_req_START (12)
#define SOC_SYSQIC_LPC_STAT3_qtp_conn_req_END (12)
#define SOC_SYSQIC_LPC_STAT3_qtp_conn_ack_START (13)
#define SOC_SYSQIC_LPC_STAT3_qtp_conn_ack_END (13)
#define SOC_SYSQIC_LPC_STAT3_qtp_slv_pwron_START (14)
#define SOC_SYSQIC_LPC_STAT3_qtp_slv_pwron_END (14)
#define SOC_SYSQIC_LPC_STAT3_qtp_mst_pwron_START (15)
#define SOC_SYSQIC_LPC_STAT3_qtp_mst_pwron_END (15)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int dfx_wrsp_crd_cnt : 8;
        unsigned int dfx_rrsp_crd_cnt : 8;
        unsigned int dfx_cfg_rsp_crd_cnt : 8;
        unsigned int reserved : 8;
    } reg;
} SOC_SYSQIC_LPC_STAT4_UNION;
#endif
#define SOC_SYSQIC_LPC_STAT4_dfx_wrsp_crd_cnt_START (0)
#define SOC_SYSQIC_LPC_STAT4_dfx_wrsp_crd_cnt_END (7)
#define SOC_SYSQIC_LPC_STAT4_dfx_rrsp_crd_cnt_START (8)
#define SOC_SYSQIC_LPC_STAT4_dfx_rrsp_crd_cnt_END (15)
#define SOC_SYSQIC_LPC_STAT4_dfx_cfg_rsp_crd_cnt_START (16)
#define SOC_SYSQIC_LPC_STAT4_dfx_cfg_rsp_crd_cnt_END (23)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int wr_opc_aft_fencing : 6;
        unsigned int reserved_0 : 2;
        unsigned int rd_opc_aft_fencing : 6;
        unsigned int reserved_1 : 2;
        unsigned int cfg_opc_aft_fencing : 6;
        unsigned int reserved_2 : 2;
        unsigned int reserved_3 : 8;
    } reg;
} SOC_SYSQIC_LPC_STAT5_UNION;
#endif
#define SOC_SYSQIC_LPC_STAT5_wr_opc_aft_fencing_START (0)
#define SOC_SYSQIC_LPC_STAT5_wr_opc_aft_fencing_END (5)
#define SOC_SYSQIC_LPC_STAT5_rd_opc_aft_fencing_START (8)
#define SOC_SYSQIC_LPC_STAT5_rd_opc_aft_fencing_END (13)
#define SOC_SYSQIC_LPC_STAT5_cfg_opc_aft_fencing_START (16)
#define SOC_SYSQIC_LPC_STAT5_cfg_opc_aft_fencing_END (21)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int lpc_revision_ro : 32;
    } reg;
} SOC_SYSQIC_LPC_REVISION_UNION;
#endif
#define SOC_SYSQIC_LPC_REVISION_lpc_revision_ro_START (0)
#define SOC_SYSQIC_LPC_REVISION_lpc_revision_ro_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int parity_chk_en : 1;
        unsigned int reserved : 31;
    } reg;
} SOC_SYSQIC_LPC_CTRL3_UNION;
#endif
#define SOC_SYSQIC_LPC_CTRL3_parity_chk_en_START (0)
#define SOC_SYSQIC_LPC_CTRL3_parity_chk_en_END (0)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int int_en : 8;
        unsigned int reserved : 24;
    } reg;
} SOC_SYSQIC_LPC_INT_EN_UNION;
#endif
#define SOC_SYSQIC_LPC_INT_EN_int_en_START (0)
#define SOC_SYSQIC_LPC_INT_EN_int_en_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int int_clr : 8;
        unsigned int reserved : 24;
    } reg;
} SOC_SYSQIC_LPC_INT_CLR_UNION;
#endif
#define SOC_SYSQIC_LPC_INT_CLR_int_clr_START (0)
#define SOC_SYSQIC_LPC_INT_CLR_int_clr_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int int_st : 8;
        unsigned int int_st_lock : 8;
        unsigned int reserved : 16;
    } reg;
} SOC_SYSQIC_LPC_INT_STATUS_UNION;
#endif
#define SOC_SYSQIC_LPC_INT_STATUS_int_st_START (0)
#define SOC_SYSQIC_LPC_INT_STATUS_int_st_END (7)
#define SOC_SYSQIC_LPC_INT_STATUS_int_st_lock_START (8)
#define SOC_SYSQIC_LPC_INT_STATUS_int_st_lock_END (15)
#ifdef __cplusplus
#if __cplusplus
    }
#endif
#endif
#endif
