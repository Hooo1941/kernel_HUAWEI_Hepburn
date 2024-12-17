#ifndef __SOC_DPU_INTERFACE_H__
#define __SOC_DPU_INTERFACE_H__ 
#ifdef __cplusplus
    #if __cplusplus
        extern "C" {
    #endif
#endif
#define DPU_DM_CMDLIST_ADDR_H_ADDR(base) ((base) + (0x0000))
#define DPU_DM_CMDLIST_ADDR0_ADDR(base) ((base) + (0x0004))
#define DPU_DM_CMDLIST_ADDR1_ADDR(base) ((base) + (0x0008))
#define DPU_DM_CMDLIST_ADDR2_ADDR(base) ((base) + (0x000C))
#define DPU_DM_CMDLIST_ADDR3_ADDR(base) ((base) + (0x0010))
#define DPU_DM_CMDLIST_ADDR4_ADDR(base) ((base) + (0x0014))
#define DPU_DM_CMDLIST_ADDR5_ADDR(base) ((base) + (0x0018))
#define DPU_DM_CMDLIST_ADDR6_ADDR(base) ((base) + (0x001C))
#define DPU_DM_CMDLIST_ADDR7_ADDR(base) ((base) + (0x0020))
#define DPU_DM_CMDLIST_ADDR8_ADDR(base) ((base) + (0x0024))
#define DPU_DM_CMDLIST_ADDR9_ADDR(base) ((base) + (0x0028))
#define DPU_DM_CMDLIST_ADDR10_ADDR(base) ((base) + (0x002C))
#define DPU_DM_CMDLIST_ADDR11_ADDR(base) ((base) + (0x0030))
#define DPU_DM_CMDLIST_ADDR12_ADDR(base) ((base) + (0x0034))
#define DPU_DM_CMDLIST_ADDR13_ADDR(base) ((base) + (0x0038))
#define DPU_DM_CMDLIST_ADDR14_ADDR(base) ((base) + (0x003C))
#define DPU_DM_CMDLIST_ADDR15_ADDR(base) ((base) + (0x0040))
#define DPU_DM_SROT_NUMBER_ADDR(base) ((base) + (0x0050))
#define DPU_DM_DDIC_NUMBER_ADDR(base) ((base) + (0x0054))
#define DPU_DM_LAYER_NUMBER_ADDR(base) ((base) + (0x0058))
#define DPU_DM_SCENE_RESERVED_ADDR(base) ((base) + (0x005C))
#define DPU_DM_OV_IMG_WIDTH_ADDR(base) ((base) + (0x0060))
#define DPU_DM_OV_BLAYER_ENDX_ADDR(base) ((base) + (0x0064))
#define DPU_DM_OV_BLAYER_ENDY_ADDR(base) ((base) + (0x0068))
#define DPU_DM_OV_BG_COLOR_RGB_ADDR(base) ((base) + (0x006C))
#define DPU_DM_OV_ORDER0_ADDR(base) ((base) + (0x0070))
#define DPU_DM_OV_RESERVED_0_ADDR(base) ((base) + (0x0074))
#define DPU_DM_OV_RESERVED_1_ADDR(base) ((base) + (0x0078))
#define DPU_DM_OV_RESERVED_2_ADDR(base) ((base) + (0x007C))
#define DPU_DM_HDR_INPUT_IMG_WIDTH_ADDR(base) ((base) + (0x0080))
#define DPU_DM_HDR_OUTPUT_IMG_WIDTH_ADDR(base) ((base) + (0x0084))
#define DPU_DM_HDR_INPUT_FMT_ADDR(base) ((base) + (0x0088))
#define DPU_DM_HDR_RESERVED_ADDR(base) ((base) + (0x008C))
#define DPU_DM_UVUP_INPUT_IMG_WIDTH_ADDR(base) ((base) + (0x0090))
#define DPU_DM_UVUP_OUTPUT_IMG_WIDTH_ADDR(base) ((base) + (0x0094))
#define DPU_DM_UVUP_INPUT_FMT_ADDR(base) ((base) + (0x0098))
#define DPU_DM_UVUP_THRESHOLD_ADDR(base) ((base) + (0x009C))
#define DPU_DM_SROT0_INPUT_IMG_WIDTH_ADDR(base) ((base) + (0x00A0))
#define DPU_DM_SROT0_OUTPUT_IMG_WIDTH_ADDR(base) ((base) + (0x00A4))
#define DPU_DM_SROT0_INPUT_FMT_ADDR(base) ((base) + (0x00A8))
#define DPU_DM_SROT0_RESERVED_ADDR(base) ((base) + (0x00AC))
#define DPU_DM_SROT1_INPUT_IMG_WIDTH_ADDR(base) ((base) + (0x00B0))
#define DPU_DM_SROT1_OUTPUT_IMG_WIDTH_ADDR(base) ((base) + (0x00B4))
#define DPU_DM_SROT1_INPUT_FMT_ADDR(base) ((base) + (0x00B8))
#define DPU_DM_SROT1_RESERVED_ADDR(base) ((base) + (0x00BC))
#define DPU_DM_CLD0_INPUT_IMG_WIDTH_ADDR(base) ((base) + (0x00C0))
#define DPU_DM_CLD0_OUTPUT_IMG_WIDTH_ADDR(base) ((base) + (0x00C4))
#define DPU_DM_CLD0_INPUT_FMT_ADDR(base) ((base) + (0x00C8))
#define DPU_DM_CLD0_RESERVED_ADDR(base) ((base) + (0x00CC))
#define DPU_DM_CLD1_INPUT_IMG_WIDTH_ADDR(base) ((base) + (0x00D0))
#define DPU_DM_CLD1_OUTPUT_IMG_WIDTH_ADDR(base) ((base) + (0x00D4))
#define DPU_DM_CLD1_INPUT_FMT_ADDR(base) ((base) + (0x00D8))
#define DPU_DM_CLD1_RESERVED_ADDR(base) ((base) + (0x00DC))
#define DPU_DM_HEMCD_INPUT_PLD_SIZE_ADDR(base) ((base) + (0x00E0))
#define DPU_DM_HEMCD_OUTPUT_IMG_HEIGHT_ADDR(base) ((base) + (0x00E4))
#define DPU_DM_HEMCD_INPUT_FMT_ADDR(base) ((base) + (0x00E8))
#define DPU_DM_HEMCD_RESERVED_ADDR(base) ((base) + (0x00EC))
#define DPU_DM_SCL0_INPUT_IMG_WIDTH_ADDR(base) ((base) + (0x00F0))
#define DPU_DM_SCL0_OUTPUT_IMG_WIDTH_ADDR(base) ((base) + (0x00F4))
#define DPU_DM_SCL0_TYPE_ADDR(base) ((base) + (0x00F8))
#define DPU_DM_SCL0_THRESHOLD_ADDR(base) ((base) + (0x00FC))
#define DPU_DM_SCL1_INPUT_IMG_WIDTH_ADDR(base) ((base) + (0x0100))
#define DPU_DM_SCL1_OUTPUT_IMG_WIDTH_ADDR(base) ((base) + (0x0104))
#define DPU_DM_SCL1_TYPE_ADDR(base) ((base) + (0x0108))
#define DPU_DM_SCL1_THRESHOLD_ADDR(base) ((base) + (0x010C))
#define DPU_DM_SCL2_INPUT_IMG_WIDTH_ADDR(base) ((base) + (0x0110))
#define DPU_DM_SCL2_OUTPUT_IMG_WIDTH_ADDR(base) ((base) + (0x0114))
#define DPU_DM_SCL2_TYPE_ADDR(base) ((base) + (0x0118))
#define DPU_DM_SCL2_THRESHOLD_ADDR(base) ((base) + (0x011C))
#define DPU_DM_SCL3_INPUT_IMG_WIDTH_ADDR(base) ((base) + (0x0120))
#define DPU_DM_SCL3_OUTPUT_IMG_WIDTH_ADDR(base) ((base) + (0x0124))
#define DPU_DM_SCL3_TYPE_ADDR(base) ((base) + (0x0128))
#define DPU_DM_SCL3_THRESHOLD_ADDR(base) ((base) + (0x012C))
#define DPU_DM_SCL4_INPUT_IMG_WIDTH_ADDR(base) ((base) + (0x0130))
#define DPU_DM_SCL4_OUTPUT_IMG_WIDTH_ADDR(base) ((base) + (0x0134))
#define DPU_DM_SCL4_TYPE_ADDR(base) ((base) + (0x0138))
#define DPU_DM_SCL4_THRESHOLD_ADDR(base) ((base) + (0x013C))
#define DPU_DM_DPP_INPUT_IMG_WIDTH_ADDR(base) ((base) + (0x0140))
#define DPU_DM_DPP_OUTPUT_IMG_WIDTH_ADDR(base) ((base) + (0x0144))
#define DPU_DM_DPP_SEL_ADDR(base) ((base) + (0x0148))
#define DPU_DM_DPP_RESERVED_ADDR(base) ((base) + (0x014C))
#define DPU_DM_DDIC_INPUT_IMG_WIDTH_ADDR(base) ((base) + (0x0150))
#define DPU_DM_DDIC_OUTPUT_IMG_WIDTH_ADDR(base) ((base) + (0x0154))
#define DPU_DM_DDIC_INPUT_FMT_ADDR(base) ((base) + (0x0158))
#define DPU_DM_DDIC_DEBURNIN_WB_EN_ADDR(base) ((base) + (0x015C))
#define DPU_DM_DDIC_WB_OUTPUT_IMG_WIDTH_ADDR(base) ((base) + (0x0160))
#define DPU_DM_DDIC_DEMURA_FMT_ADDR(base) ((base) + (0x0164))
#define DPU_DM_DDIC_DEMURA_LUT_WIDTH_ADDR(base) ((base) + (0x0168))
#define DPU_DM_DDIC_RESERVED_2_ADDR(base) ((base) + (0x016C))
#define DPU_DM_DSC_INPUT_IMG_WIDTH_ADDR(base) ((base) + (0x0170))
#define DPU_DM_DSC_OUTPUT_IMG_WIDTH_ADDR(base) ((base) + (0x0174))
#define DPU_DM_DSC_SEL_ADDR(base) ((base) + (0x0178))
#define DPU_DM_DSC_RESERVED_ADDR(base) ((base) + (0x017C))
#define DPU_DM_WCH_INPUT_IMG_WIDTH_ADDR(base) ((base) + (0x0180))
#define DPU_DM_WCH_INPUT_IMG_STRY_ADDR(base) ((base) + (0x0184))
#define DPU_DM_WCH_RESERVED_0_ADDR(base) ((base) + (0x0188))
#define DPU_DM_WCH_RESERVED_1_ADDR(base) ((base) + (0x018C))
#define DPU_DM_ITF_INPUT_IMG_WIDTH_ADDR(base) ((base) + (0x0190))
#define DPU_DM_ITF_INPUT_FMT_ADDR(base) ((base) + (0x0194))
#define DPU_DM_ITF_RESERVED_0_ADDR(base) ((base) + (0x0198))
#define DPU_DM_ITF_RESERVED_1_ADDR(base) ((base) + (0x019C))
#define DPU_DM_LAYER_HEIGHT_ADDR(base,ln) ((base) + (0x01A0+0x80*(ln)))
#define DPU_DM_LAYER_MASK_Y0_ADDR(base,ln) ((base) + (0x01A4+0x80*(ln)))
#define DPU_DM_LAYER_MASK_Y1_ADDR(base,ln) ((base) + (0x01A8+0x80*(ln)))
#define DPU_DM_LAYER_DMA_SEL_ADDR(base,ln) ((base) + (0x01AC+0x80*(ln)))
#define DPU_DM_LAYER_SBLK_TYPE_ADDR(base,ln) ((base) + (0x01B0+0x80*(ln)))
#define DPU_DM_LAYER_BOT_CROP_ADDR(base,ln) ((base) + (0x01B4+0x80*(ln)))
#define DPU_DM_LAYER_OV_CLIP_LEFT_ADDR(base,ln) ((base) + (0x01B8+0x80*(ln)))
#define DPU_DM_LAYER_OV_DFC_CFG_ADDR(base,ln) ((base) + (0x01BC+0x80*(ln)))
#define DPU_DM_LAYER_OV_STARTY_ADDR(base,ln) ((base) + (0x01C0+0x80*(ln)))
#define DPU_DM_LAYER_OV_ENDY_ADDR(base,ln) ((base) + (0x01C4+0x80*(ln)))
#define DPU_DM_LAYER_OV_PATTERN_A_ADDR(base,ln) ((base) + (0x01C8+0x80*(ln)))
#define DPU_DM_LAYER_OV_PATTERN_RGB_ADDR(base,ln) ((base) + (0x01CC+0x80*(ln)))
#define DPU_DM_LAYER_OV_MODE_ADDR(base,ln) ((base) + (0x01D0+0x80*(ln)))
#define DPU_DM_LAYER_OV_ALPHA_ADDR(base,ln) ((base) + (0x01D4+0x80*(ln)))
#define DPU_DM_LAYER_STRETCH3_LINE_ADDR(base,ln) ((base) + (0x01D8+0x80*(ln)))
#define DPU_DM_LAYER_START_ADDR3_H_ADDR(base,ln) ((base) + (0x01DC+0x80*(ln)))
#define DPU_DM_LAYER_START_ADDR0_L_ADDR(base,ln) ((base) + (0x01E0+0x80*(ln)))
#define DPU_DM_LAYER_START_ADDR1_L_ADDR(base,ln) ((base) + (0x01E4+0x80*(ln)))
#define DPU_DM_LAYER_START_ADDR2_L_ADDR(base,ln) ((base) + (0x01E8+0x80*(ln)))
#define DPU_DM_LAYER_START_ADDR3_L_ADDR(base,ln) ((base) + (0x01EC+0x80*(ln)))
#define DPU_DM_LAYER_STRIDE0_ADDR(base,ln) ((base) + (0x01F0+0x80*(ln)))
#define DPU_DM_LAYER_STRIDE1_ADDR(base,ln) ((base) + (0x01F4+0x80*(ln)))
#define DPU_DM_LAYER_STRIDE2_ADDR(base,ln) ((base) + (0x01F8+0x80*(ln)))
#define DPU_DM_LAYER_RSV0_ADDR(base,ln) ((base) + (0x01FC+0x80*(ln)))
#define DPU_DM_LAYER_RSV1_ADDR(base,ln) ((base) + (0x0200+0x80*(ln)))
#define DPU_DM_LAYER_RSV2_ADDR(base,ln) ((base) + (0x0204+0x80*(ln)))
#define DPU_DM_LAYER_RSV3_ADDR(base,ln) ((base) + (0x0208+0x80*(ln)))
#define DPU_DM_LAYER_RSV4_ADDR(base,ln) ((base) + (0x020C+0x80*(ln)))
#define DPU_DM_LAYER_RSV5_ADDR(base,ln) ((base) + (0x0210+0x80*(ln)))
#define DPU_DM_LAYER_RSV6_ADDR(base,ln) ((base) + (0x0214+0x80*(ln)))
#define DPU_DM_LAYER_RSV7_ADDR(base,ln) ((base) + (0x0218+0x80*(ln)))
#define DPU_DM_LAYER_RSV8_ADDR(base,ln) ((base) + (0x021C+0x80*(ln)))
#define DPU_CMD_CMDLIST_CH_PENDING_CLR_ADDR(base,i) ((base) + (0x0000+0x40*(i)))
#define DPU_CMD_CMDLIST_CH_CTRL_ADDR(base,i) ((base) + (0x0004+0x40*(i)))
#define DPU_CMD_CMDLIST_CH_STATUS_ADDR(base,i) ((base) + (0x0008+0x40*(i)))
#define DPU_CMD_CMDLIST_CH_STAAD_ADDR(base,i) ((base) + (0x000C+0x40*(i)))
#define DPU_CMD_CMDLIST_CH_CURAD_ADDR(base,i) ((base) + (0x0010+0x40*(i)))
#define DPU_CMD_CMDLIST_CH_INTE_ADDR(base,i) ((base) + (0x0014+0x40*(i)))
#define DPU_CMD_CMDLIST_CH_INTC_ADDR(base,i) ((base) + (0x0018+0x40*(i)))
#define DPU_CMD_CMDLIST_CH_INTS_ADDR(base,i) ((base) + (0x001C+0x40*(i)))
#define DPU_CMD_CMDLIST_CH_DBG_ADDR(base,i) ((base) + (0x0028+0x40*(i)))
#define DPU_CMD_CMDLIST_DBG_ADDR(base) ((base) + (0x0700))
#define DPU_CMD_BUF_DBG_EN_ADDR(base) ((base) + (0x0704))
#define DPU_CMD_BUF_DBG_CNT_CLR_ADDR(base) ((base) + (0x0708))
#define DPU_CMD_BUF_DBG_CNT_ADDR(base) ((base) + (0x070C))
#define DPU_CMD_CMDLIST_TIMEOUT_TH_ADDR(base) ((base) + (0x0710))
#define DPU_CMD_CMDLIST_START_ADDR(base) ((base) + (0x0714))
#define DPU_CMD_CMDLIST_ADDR_MASK_EN_ADDR(base) ((base) + (0x0718))
#define DPU_CMD_CMDLIST_ADDR_MASK_DIS_ADDR(base) ((base) + (0x071C))
#define DPU_CMD_CMDLIST_ADDR_MASK_STATUS_ADDR(base) ((base) + (0x0720))
#define DPU_CMD_CMDLIST_TASK_CONTINUE_ADDR(base) ((base) + (0x0724))
#define DPU_CMD_CMDLIST_TASK_STATUS_ADDR(base) ((base) + (0x0728))
#define DPU_CMD_CMDLIST_CTRL_ADDR(base) ((base) + (0x072C))
#define DPU_CMD_CMDLIST_SECU_ADDR(base) ((base) + (0x0730))
#define DPU_CMD_CMDLIST_INTS_ADDR(base) ((base) + (0x0734))
#define DPU_CMD_CMDLIST_SWRST_ADDR(base) ((base) + (0x0738))
#define DPU_CMD_MEM_CTRL_ADDR(base) ((base) + (0x073C))
#define DPU_CMD_CLK_SEL_ADDR(base) ((base) + (0x0740))
#define DPU_CMD_CLK_EN_ADDR(base) ((base) + (0x0744))
#define DPU_CMD_SW_RELOAD_PROT_ADDR(base) ((base) + (0x0748))
#define DPU_CMD_SYNC_MODE_ADDR(base) ((base) + (0x074C))
#define DPU_CMD_CFG_FLAG_ADDR(base) ((base) + (0x0750))
#define DPU_CMD_SYNC_DBG_ADDR(base) ((base) + (0x0754))
#define DPU_CMD_CH0_CFG_TIME_EN_ADDR(base) ((base) + (0x0758))
#define DPU_CMD_CH0_CFG_TIME_CNT_ADDR(base) ((base) + (0x077C))
#define DPU_CMD_SCE0_CFG_SIGNAL_ADDR(base) ((base) + (0x0780))
#define DPU_CMD_SCE1_CFG_SIGNAL_ADDR(base) ((base) + (0x0784))
#define DPU_CMD_SCE2_CFG_SIGNAL_ADDR(base) ((base) + (0x0788))
#define DPU_CMD_SCE3_CFG_SIGNAL_ADDR(base) ((base) + (0x078c))
#define DPU_CMD_SCE4_CFG_SIGNAL_ADDR(base) ((base) + (0x0790))
#define DPU_CMD_SCE5_CFG_SIGNAL_ADDR(base) ((base) + (0x0794))
#define DPU_CMD_SCE6_CFG_SIGNAL_ADDR(base) ((base) + (0x0798))
#define DPU_CMD_LBUSIF_MSTATE_ADDR(base) ((base) + (0x079c))
#define DPU_CMD_TWO_SAME_FRAME_BYPASS_ADDR(base) ((base) + (0x07a0))
#define DPU_GLB_TAG_ADDR(base) ((base) + (0x0000))
#define DPU_GLB_CPU_CMD_FORCE_ADDR(base) ((base) + (0x0004))
#define DPU_GLB_JTAG_MODE_ADDR(base) ((base) + (0x0008))
#define DPU_GLB_DACC_TZPC_ADDR(base) ((base) + (0x000C))
#define DPU_GLB_MODULE_CLK_SEL_ADDR(base) ((base) + (0x0100))
#define DPU_GLB_MODULE_CLK_EN_ADDR(base) ((base) + (0x104))
#define DPU_GLB_PM_CTRL_ADDR(base) ((base) + (0x0108))
#define DPU_GLB_DSLP_EN_ADDR(base) ((base) + (0x010C))
#define DPU_GLB_SPRAM_CTRL_S_ADDR(base) ((base) + (0x0110))
#define DPU_GLB_TPRAM_CTRL_TP_ADDR(base) ((base) + (0x0114))
#define DPU_GLB_SPRAM_CTRL_SPC_ADDR(base) ((base) + (0x0118))
#define DPU_GLB_TPRAM_CTRL_TPL_ADDR(base) ((base) + (0x011C))
#define DPU_GLB_RS_CLEAR_ADDR(base) ((base) + (0x0120))
#define DPU_GLB_DISP_CH0_MODULE_CLK_SEL_ADDR(base) ((base) + (0x0124))
#define DPU_GLB_DISP_CH0_MODULE_CLK_EN_ADDR(base) ((base) + (0x0128))
#define DPU_GLB_DISP_CH1_MODULE_CLK_SEL_ADDR(base) ((base) + (0x012C))
#define DPU_GLB_DISP_CH1_MODULE_CLK_EN_ADDR(base) ((base) + (0x0130))
#define DPU_GLB_DISP_AVHR_MODULE_CLK_SEL_ADDR(base) ((base) + (0x0134))
#define DPU_GLB_DISP_AVHR_MODULE_CLK_EN_ADDR(base) ((base) + (0x138))
#define DPU_GLB_DISP_MODULE_CLK_SEL_ADDR(base) ((base) + (0x013C))
#define DPU_GLB_DISP_MODULE_CLK_EN_ADDR(base) ((base) + (0x140))
#define DPU_GLB_RST_EN_ADDR(base) ((base) + (0x144))
#define DPU_GLB_GMP_MEM_CTRL_ADDR(base) ((base) + (0x01A0))
#define DPU_GLB_LB_PART_MEM_CTRL_ADDR(base) ((base) + (0x01A4))
#define DPU_GLB_DACC_MEM_CTRL_ADDR(base) ((base) + (0x01A8))
#define DPU_GLB_SDMA0_MEM_CTRL_ADDR(base) ((base) + (0x01AC))
#define DPU_GLB_SDMA1_MEM_CTRL_ADDR(base) ((base) + (0x01B0))
#define DPU_GLB_SDMA2_MEM_CTRL_ADDR(base) ((base) + (0x01B4))
#define DPU_GLB_NS_DFS_OK_TO_LPMCU_O_ADDR(base) ((base) + (0x200))
#define DPU_GLB_NS_DFS_OK_TO_LPMCU_MSK_ADDR(base) ((base) + (0x204))
#define DPU_GLB_NS_EM_O_ADDR(base) ((base) + (0x208))
#define DPU_GLB_NS_EM_MSK_ADDR(base) ((base) + (0x20C))
#define DPU_GLB_NS_TO_NPU_O_ADDR(base) ((base) + (0x0210))
#define DPU_GLB_NS_TO_NPU_MASK_ADDR(base) ((base) + (0x0214))
#define DPU_GLB_S_TO_NPU_O_ADDR(base) ((base) + (0x0218))
#define DPU_GLB_S_TO_NPU_MASK_ADDR(base) ((base) + (0x021C))
#define DPU_GLB_NS_DP0_TO_GIC_O_ADDR(base) ((base) + (0x0240))
#define DPU_GLB_NS_DP0_TO_GIC_MSK_ADDR(base) ((base) + (0x0244))
#define DPU_GLB_NS_DP1_TO_GIC_O_ADDR(base) ((base) + (0x0248))
#define DPU_GLB_NS_DP1_TO_GIC_MSK_ADDR(base) ((base) + (0x024C))
#define DPU_GLB_NS_MDP_TO_GIC_O_ADDR(base) ((base) + (0x0250))
#define DPU_GLB_NS_MDP_TO_GIC_MSK_ADDR(base) ((base) + (0x0254))
#define DPU_GLB_NS_SDP_TO_GIC_O_ADDR(base) ((base) + (0x0258))
#define DPU_GLB_NS_SDP_TO_GIC_MSK_ADDR(base) ((base) + (0x025C))
#define DPU_GLB_NS_OFFLINE0_TO_GIC_O_ADDR(base) ((base) + (0x0260))
#define DPU_GLB_NS_OFFLINE0_TO_GIC_MSK_ADDR(base) ((base) + (0x0264))
#define DPU_GLB_NS_OFFLINE1_TO_GIC_O_ADDR(base) ((base) + (0x0268))
#define DPU_GLB_NS_OFFLINE1_TO_GIC_MSK_ADDR(base) ((base) + (0x026C))
#define DPU_GLB_NS_OFFLINE2_TO_GIC_O_ADDR(base) ((base) + (0x0270))
#define DPU_GLB_NS_OFFLINE2_TO_GIC_MSK_ADDR(base) ((base) + (0x0274))
#define DPU_GLB_NS_DP0_TO_IOMCU_O_ADDR(base) ((base) + (0x02A0))
#define DPU_GLB_NS_DP0_TO_IOMCU_MSK_ADDR(base) ((base) + (0x02A4))
#define DPU_GLB_NS_DP1_TO_IOMCU_O_ADDR(base) ((base) + (0x02A8))
#define DPU_GLB_NS_DP1_TO_IOMCU_MSK_ADDR(base) ((base) + (0x02AC))
#define DPU_GLB_NS_MDP_TO_IOMCU_O_ADDR(base) ((base) + (0x02B0))
#define DPU_GLB_NS_MDP_TO_IOMCU_MSK_ADDR(base) ((base) + (0x02B4))
#define DPU_GLB_NS_SDP_TO_IOMCU_O_ADDR(base) ((base) + (0x02B8))
#define DPU_GLB_NS_SDP_TO_IOMCU_MSK_ADDR(base) ((base) + (0x02BC))
#define DPU_GLB_NS_OFFLINE0_TO_IOMCU_O_ADDR(base) ((base) + (0x02C0))
#define DPU_GLB_NS_OFFLINE0_TO_IOMCU_MSK_ADDR(base) ((base) + (0x02C4))
#define DPU_GLB_NS_OFFLINE1_TO_IOMCU_O_ADDR(base) ((base) + (0x02C8))
#define DPU_GLB_NS_OFFLINE1_TO_IOMCU_MSK_ADDR(base) ((base) + (0x02CC))
#define DPU_GLB_NS_OFFLINE2_TO_IOMCU_O_ADDR(base) ((base) + (0x02D0))
#define DPU_GLB_NS_OFFLINE2_TO_IOMCU_MSK_ADDR(base) ((base) + (0x02D4))
#define DPU_GLB_S_DP0_TO_GIC_O_ADDR(base) ((base) + (0x0300))
#define DPU_GLB_S_DP0_TO_GIC_MSK_ADDR(base) ((base) + (0x0304))
#define DPU_GLB_S_DP1_TO_GIC_O_ADDR(base) ((base) + (0x0308))
#define DPU_GLB_S_DP1_TO_GIC_MSK_ADDR(base) ((base) + (0x030C))
#define DPU_GLB_S_MDP_TO_GIC_O_ADDR(base) ((base) + (0x0310))
#define DPU_GLB_S_MDP_TO_GIC_MSK_ADDR(base) ((base) + (0x0314))
#define DPU_GLB_S_SDP_TO_GIC_O_ADDR(base) ((base) + (0x0318))
#define DPU_GLB_S_SDP_TO_GIC_MSK_ADDR(base) ((base) + (0x031C))
#define DPU_GLB_OFFLINE0_TO_GIC_O_ADDR(base) ((base) + (0x0320))
#define DPU_GLB_S_OFFLINE0_TO_GIC_MSK_ADDR(base) ((base) + (0x0324))
#define DPU_GLB_S_OFFLINE1_TO_GIC_O_ADDR(base) ((base) + (0x0328))
#define DPU_GLB_S_OFFLINE1_TO_GIC_MSK_ADDR(base) ((base) + (0x032C))
#define DPU_GLB_S_OFFLINE2_TO_GIC_O_ADDR(base) ((base) + (0x0330))
#define DPU_GLB_S_OFFLINE2_TO_GIC_MSK_ADDR(base) ((base) + (0x0334))
#define DPU_GLB_S_DP0_TO_LPMCU_O_ADDR(base) ((base) + (0x0380))
#define DPU_GLB_S_DP0_TO_LPMCU_MSK_ADDR(base) ((base) + (0x0384))
#define DPU_GLB_S_DP1_TO_LPMCU_O_ADDR(base) ((base) + (0x0388))
#define DPU_GLB_S_DP1_TO_LPMCU_MSK_ADDR(base) ((base) + (0x038C))
#define DPU_GLB_S_MDP_TO_LPMCU_O_ADDR(base) ((base) + (0x0390))
#define DPU_GLB_S_MDP_TO_LPMCU_MSK_ADDR(base) ((base) + (0x0394))
#define DPU_GLB_S_SDP_TO_LPMCU_O_ADDR(base) ((base) + (0x0398))
#define DPU_GLB_S_SDP_TO_LPMCU_MSK_ADDR(base) ((base) + (0x039C))
#define DPU_GLB_S_OFFLINE0_TO_LPMCU_O_ADDR(base) ((base) + (0x03A0))
#define DPU_GLB_S_OFFLINE0_TO_LPMCU_MSK_ADDR(base) ((base) + (0x03A4))
#define DPU_GLB_S_OFFLINE1_TO_LPMCU_O_ADDR(base) ((base) + (0x03A8))
#define DPU_GLB_S_OFFLINE1_TO_LPMCU_MSK_ADDR(base) ((base) + (0x03AC))
#define DPU_GLB_S_OFFLINE2_TO_LPMCU_O_ADDR(base) ((base) + (0x03B0))
#define DPU_GLB_S_OFFLINE2_TO_LPMCU_MSK_ADDR(base) ((base) + (0x03B4))
#define DPU_GLB_DBG_IRQ_NS_GIC_ADDR(base) ((base) + (0x0400))
#define DPU_GLB_DBG_IRQ_S_GIC_ADDR(base) ((base) + (0x0404))
#define DPU_GLB_DBG_IRQ_NS_IOMCU_ADDR(base) ((base) + (0x0408))
#define DPU_GLB_DBG_IRQ_S_LPMCU_ADDR(base) ((base) + (0x040C))
#define DPU_GLB_DBG_IRQ_NS_NPU_ADDR(base) ((base) + (0x0410))
#define DPU_GLB_DBG_IRQ_S_NPU_ADDR(base) ((base) + (0x0414))
#define DPU_GLB_CMD_NS_INT_O_ADDR(base) ((base) + (0x0440))
#define DPU_GLB_CMD_NS_INT_MSK_ADDR(base) ((base) + (0x0444))
#define DPU_GLB_SMART_DMA0_NS_INT_O_ADDR(base) ((base) + (0x0448))
#define DPU_GLB_SMART_DMA0_NS_INT_MSK_ADDR(base) ((base) + (0x044C))
#define DPU_GLB_SMART_DMA1_NS_INT_O_ADDR(base) ((base) + (0x0450))
#define DPU_GLB_SMART_DMA1_NS_INT_MSK_ADDR(base) ((base) + (0x0454))
#define DPU_GLB_SMART_DMA2_NS_INT_O_ADDR(base) ((base) + (0x0458))
#define DPU_GLB_SMART_DMA2_NS_INT_MSK_ADDR(base) ((base) + (0x045C))
#define DPU_GLB_SMART_DMA3_NS_INT_O_ADDR(base) ((base) + (0x0460))
#define DPU_GLB_SMART_DMA3_NS_INT_MSK_ADDR(base) ((base) + (0x0464))
#define DPU_GLB_WCH0_NS_INT_O_ADDR(base) ((base) + (0x0468))
#define DPU_GLB_WCH0_NS_INT_MSK_ADDR(base) ((base) + (0x046C))
#define DPU_GLB_WCH1_NS_INT_O_ADDR(base) ((base) + (0x0470))
#define DPU_GLB_WCH1_NS_INT_MSK_ADDR(base) ((base) + (0x0474))
#define DPU_GLB_WCH2_NS_INT_O_ADDR(base) ((base) + (0x0478))
#define DPU_GLB_WCH2_NS_INT_MSK_ADDR(base) ((base) + (0x047C))
#define DPU_GLB_DACC_NS_INT_O_ADDR(base) ((base) + (0x0480))
#define DPU_GLB_DACC_NS_INT_MSK_ADDR(base) ((base) + (0x0484))
#define DPU_GLB_HEMCD0_NS_INT_O_ADDR(base) ((base) + (0x0488))
#define DPU_GLB_HEMCD0_NS_INT_MSK_ADDR(base) ((base) + (0x048C))
#define DPU_GLB_DDE_NS_INT_O_ADDR(base) ((base) + (0x0490))
#define DPU_GLB_DDE_NS_INT_MSK_ADDR(base) ((base) + (0x0494))
#define DPU_GLB_HEMCD1_NS_INT_O_ADDR(base) ((base) + (0x0498))
#define DPU_GLB_HEMCD1_NS_INT_MSK_ADDR(base) ((base) + (0x049C))
#define DPU_GLB_DACC_S_INT_O_ADDR(base) ((base) + (0x04A0))
#define DPU_GLB_DACC_S_INT_MSK_ADDR(base) ((base) + (0x04A4))
#define DPU_GLB_SDMA_CTRL0_ADDR(base,sdn) ((base) + (0x0500+0x40*(sdn)))
#define DPU_GLB_SDMA_CTRL1_ADDR(base,sdn) ((base) + (0x0504+0x40*(sdn)))
#define DPU_GLB_SDMA_CTRL2_ADDR(base,sdn) ((base) + (0x0508+0x40*(sdn)))
#define DPU_GLB_SDMA_DBG_RESERVED0_ADDR(base,sdn) ((base) + (0x050c+0x40*(sdn)))
#define DPU_GLB_SDMA_DBG_OUT0_ADDR(base,sdn) ((base) + (0x0510+0x40*(sdn)))
#define DPU_GLB_SDMA_DBG_OUT1_ADDR(base,sdn) ((base) + (0x0514+0x40*(sdn)))
#define DPU_GLB_SDMA_DBG_MONITOR0_ADDR(base,sdn) ((base) + (0x0518+0x40*(sdn)))
#define DPU_GLB_SDMA_DBG_MONITOR1_ADDR(base,sdn) ((base) + (0x051c+0x40*(sdn)))
#define DPU_GLB_SDMA_DBG_MONITOR2_ADDR(base,sdn) ((base) + (0x0520+0x40*(sdn)))
#define DPU_GLB_SDMA_DBG_MONITOR3_ADDR(base,sdn) ((base) + (0x0524+0x40*(sdn)))
#define DPU_GLB_SDMA_DBG_MONITOR4_ADDR(base,sdn) ((base) + (0x0528+0x40*(sdn)))
#define DPU_GLB_SDMA_DBG_MONITOR5_ADDR(base,sdn) ((base) + (0x052c+0x40*(sdn)))
#define DPU_GLB_SDMA_CTRL3_ADDR(base,sdn) ((base) + (0x0530+0x40*(sdn)))
#define DPU_GLB_SDMA_DBG_OUT2_ADDR(base,sdn) ((base) + (0x0534+0x40*(sdn)))
#define DPU_GLB_SDMA_DBG_RESERVED1_ADDR(base,sdn) ((base) + (0x0538+0x40*(sdn)))
#define DPU_GLB_SDMA_W_DBG0_ADDR(base,sdn) ((base) + (0x0900+0x40*(sdn)))
#define DPU_GLB_SDMA_W_DBG1_ADDR(base,sdn) ((base) + (0x0904+0x40*(sdn)))
#define DPU_GLB_SDMA_W_DBG2_ADDR(base,sdn) ((base) + (0x0908+0x40*(sdn)))
#define DPU_GLB_SDMA_W_DBG3_ADDR(base,sdn) ((base) + (0x090C+0x40*(sdn)))
#define DPU_GLB_SROT_DBG_OUT0_ADDR(base,srn) ((base) + (0x0600+0x40*(srn)))
#define DPU_GLB_SROT_DBG_OUT1_ADDR(base,srn) ((base) + (0x0604+0x40*(srn)))
#define DPU_GLB_SROT_DBG_OUT2_ADDR(base,srn) ((base) + (0x0608+0x40*(srn)))
#define DPU_GLB_SROT_DBG_OUT3_ADDR(base,srn) ((base) + (0x060C+0x40*(srn)))
#define DPU_GLB_SROT_DBG_OUT4_ADDR(base,srn) ((base) + (0x0610+0x40*(srn)))
#define DPU_GLB_SROT_DBG_OUT5_ADDR(base,srn) ((base) + (0x0614+0x40*(srn)))
#define DPU_GLB_SROT_DBG_OUT6_ADDR(base,srn) ((base) + (0x0618+0x40*(srn)))
#define DPU_GLB_SROT_DBG_OUT7_ADDR(base,srn) ((base) + (0x061C+0x40*(srn)))
#define DPU_GLB_SROT_DBG_RESERVED_ADDR(base,srn) ((base) + (0x0620+0x40*(srn)))
#define DPU_GLB_SROT_CTRL0_ADDR(base,srn) ((base) + (0x0624+0x40*(srn)))
#define DPU_GLB_REG_DVFS_CTRL_ADDR(base) ((base) + (0x06A0))
#define DPU_GLB_REG_PERI_DVFS_YCNT0_ADDR(base) ((base) + (0x06A4))
#define DPU_GLB_REG_PERI_DVFS_YCNT2_ADDR(base) ((base) + (0x06A8))
#define DPU_GLB_REG_DDR_DVFS_YCNT0_ADDR(base) ((base) + (0x06AC))
#define DPU_GLB_REG_DDR_DVFS_YCNT2_ADDR(base) ((base) + (0x06B0))
#define DPU_GLB_REG_DVFS_OV_CNT_ADDR(base) ((base) + (0x06B4))
#define DPU_GLB_CMD_ADDR_ADDR(base) ((base) + (0x06B8))
#define DPU_GLB_REG_DVFS_CTRL1_ADDR(base) ((base) + (0x06BC))
#define DPU_GLB_DBG_CTRL0_ADDR(base) ((base) + (0x0700))
#define DPU_GLB_DBG_IP_CYCLE_CNT_ADDR(base) ((base) + (0x0704))
#define DPU_GLB_DBG_IP_STATE0_ADDR(base) ((base) + (0x0708))
#define DPU_GLB_DBG_IP_STATE1_ADDR(base) ((base) + (0x070C))
#define DPU_GLB_DBG_CTRL1_ADDR(base) ((base) + (0x0710))
#define DPU_GLB_DBG_OV_YCNT0_ADDR(base) ((base) + (0x0714))
#define DPU_GLB_DBG_OV_YCNT1_ADDR(base) ((base) + (0x0718))
#define DPU_GLB_DBG_OV_YCNT2_ADDR(base) ((base) + (0x071C))
#define DPU_GLB_DBG_OV_YCNT3_ADDR(base) ((base) + (0x0720))
#define DPU_GLB_DBG_CFG_CTRL_ADDR(base) ((base) + (0x0740))
#define DPU_GLB_DBG_CFG_STATE_ADDR(base) ((base) + (0x0744))
#define DPU_GLB_DBG_CFG_ADDR_ADDR(base) ((base) + (0x0748))
#define DPU_GLB_DBG_DCPT_CTRL_ADDR(base) ((base) + (0x0760))
#define DPU_GLB_DBG_DCPT_DATA0_ADDR(base) ((base) + (0x0764))
#define DPU_GLB_DBG_DCPT_DATA1_ADDR(base) ((base) + (0x0768))
#define DPU_GLB_DBG_DATA_SEL_ADDR(base) ((base) + (0x0770))
#define DPU_GLB_DBG_DATA0_ADDR(base) ((base) + (0x0774))
#define DPU_GLB_DBG_DATA1_ADDR(base) ((base) + (0x0778))
#define DPU_GLB_DBG_DATA2_ADDR(base) ((base) + (0x077C))
#define DPU_GLB_DBG_DATA3_ADDR(base) ((base) + (0x0780))
#define DPU_GLB_DBG_DATA4_ADDR(base) ((base) + (0x0784))
#define DPU_GLB_DBG_FLUSH_EN_ADDR(base) ((base) + (0x0788))
#define DPU_GLB_DBG_FLUSH_CLR_ADDR(base) ((base) + (0x078C))
#define DPU_GLB_DBG_FLUSH_ST_ADDR(base) ((base) + (0x0790))
#define DPU_GLB_DBG_DATA5_ADDR(base) ((base) + (0x0794))
#define DPU_GLB_DBG_DATA6_ADDR(base) ((base) + (0x0798))
#define DPU_GLB_DBG_DATA7_ADDR(base) ((base) + (0x079C))
#define DPU_GLB_MASK_CTRL0_ADDR(base) ((base) + (0x0800))
#define DPU_GLB_MASK_CTRL1_ADDR(base) ((base) + (0x0804))
#define DPU_GLB_MASK_CTRL2_ADDR(base) ((base) + (0x0808))
#define DPU_GLB_CRC_CTRL_ADDR(base) ((base) + (0x0840))
#define DPU_GLB_CRC_RESULT0_ADDR(base) ((base) + (0x0844))
#define DPU_GLB_CRC_FRAME_CNT0_ADDR(base) ((base) + (0x0848))
#define DPU_GLB_CRC_RESULT1_ADDR(base) ((base) + (0x084C))
#define DPU_GLB_CRC_FRAME_CNT1_ADDR(base) ((base) + (0x0850))
#define DPU_GLB_CRC_RESULT2_ADDR(base) ((base) + (0x0854))
#define DPU_GLB_CRC_FRAME_CNT2_ADDR(base) ((base) + (0x0858))
#define DPU_GLB_CRC_RESULT3_ADDR(base) ((base) + (0x085C))
#define DPU_GLB_CRC_FRAME_CNT3_ADDR(base) ((base) + (0x0860))
#define DPU_GLB_CRC_INT3_ADDR(base) ((base) + (0x0864))
#define DPU_GLB_MCLEAR_CTRL_ADDR(base) ((base) + (0x08A0))
#define DPU_GLB_MCLEAR_CLEAR_ST_ADDR(base) ((base) + (0x08A4))
#define DPU_GLB_MCLEAR_CLEAR_IP_ST_ADDR(base) ((base) + (0x08A8))
#define DPU_WCH_DMA_CTRL0_ADDR(base) ((base) + (0x0000))
#define DPU_WCH_DMA_CTRL1_ADDR(base) ((base) + (0x0004))
#define DPU_WCH_DMA_IMG_SIZE_ADDR(base) ((base) + (0x0008))
#define DPU_WCH_DMA_PTR0_ADDR(base) ((base) + (0x000C))
#define DPU_WCH_DMA_STRIDE0_ADDR(base) ((base) + (0x0010))
#define DPU_WCH_DMA_PTR1_ADDR(base) ((base) + (0x0014))
#define DPU_WCH_DMA_STRIDE1_ADDR(base) ((base) + (0x0018))
#define DPU_WCH_DMA_PTR2_ADDR(base) ((base) + (0x001C))
#define DPU_WCH_DMA_STRIDE2_ADDR(base) ((base) + (0x0020))
#define DPU_WCH_DMA_PTR3_ADDR(base) ((base) + (0x0024))
#define DPU_WCH_DMA_STRIDE3_ADDR(base) ((base) + (0x0028))
#define DPU_WCH_DMA_CMP_HDR_PTR_OFT_ADDR(base) ((base) + (0x002C))
#define DPU_WCH_DMA_CH_SEGMENT_CTRL_ADDR(base) ((base) + (0x0030))
#define DPU_WCH_DMA_CMP_CTRL_ADDR(base) ((base) + (0x0034))
#define DPU_WCH_DMA_CMP_PIC_BLKS_ADDR(base) ((base) + (0x0038))
#define DPU_WCH_DMA_CMP_MEM_CTRL0_ADDR(base) ((base) + (0x003C))
#define DPU_WCH_DMA_CMP_MEM_CTRL1_ADDR(base) ((base) + (0x0040))
#define DPU_WCH_DMA_CMP_RESEVED_ADDR(base) ((base) + (0x0044))
#define DPU_WCH_DMA_CMP_INFO_OUT0_ADDR(base) ((base) + (0x0048))
#define DPU_WCH_DMA_CMP_INFO_OUT1_ADDR(base) ((base) + (0x004C))
#define DPU_WCH_DMA_INTR_ADDR(base) ((base) + (0x0050))
#define DPU_WCH_DMA_RSV_IN0_ADDR(base) ((base) + (0x0054))
#define DPU_WCH_DMA_RSV_IN1_ADDR(base) ((base) + (0x0058))
#define DPU_WCH_DMA_RSV_IN2_ADDR(base) ((base) + (0x005C))
#define DPU_WCH_DMA_RSV_IN3_ADDR(base) ((base) + (0x0060))
#define DPU_WCH_DMA_MONITOR_OUT0_ADDR(base) ((base) + (0x0064))
#define DPU_WCH_DMA_MONITOR_OUT1_ADDR(base) ((base) + (0x0068))
#define DPU_WCH_DMA_MONITOR_OUT2_ADDR(base) ((base) + (0x006C))
#define DPU_WCH_DMA_MONITOR_OUT3_ADDR(base) ((base) + (0x0070))
#define DPU_WCH_DMA_MONITOR_OUT4_ADDR(base) ((base) + (0x0074))
#define DPU_WCH_DMA_MONITOR_OUT5_ADDR(base) ((base) + (0x0078))
#define DPU_WCH_WTL_CTRL_ADDR(base) ((base) + (0x00A0))
#define DPU_WCH_WLT_LINE_THRESHOLD1_ADDR(base) ((base) + (0x00A4))
#define DPU_WCH_WLT_LINE_THRESHOLD3_ADDR(base) ((base) + (0x00A8))
#define DPU_WCH_WLT_BASE_LINE_OFFSET_ADDR(base) ((base) + (0x00AC))
#define DPU_WCH_WLT_DMA_ADDR_ADDR(base) ((base) + (0x00B0))
#define DPU_WCH_WLT_DMA_QOS_ADDR(base) ((base) + (0x00B4))
#define DPU_WCH_WLT_C_LINE_NUM_ADDR(base) ((base) + (0x00B8))
#define DPU_WCH_WLT_C_SLICE_NUM_ADDR(base) ((base) + (0x00BC))
#define DPU_WCH_REG_DEFAULT_ADDR(base) ((base) + (0x00D0))
#define DPU_WCH_CH_RD_SHADOW_ADDR(base) ((base) + (0x00D4))
#define DPU_WCH_CH_CTL_ADDR(base) ((base) + (0x00D8))
#define DPU_WCH_CH_SECU_EN_ADDR(base) ((base) + (0x00DC))
#define DPU_WCH_CH_SW_END_REQ_ADDR(base) ((base) + (0x00E0))
#define DPU_WCH_CH_CLK_SEL_ADDR(base) ((base) + (0x00E4))
#define DPU_WCH_CH_CLK_EN_ADDR(base) ((base) + (0x00E8))
#define DPU_WCH_CH_DBG0_ADDR(base) ((base) + (0x00EC))
#define DPU_WCH_CH_DBG1_ADDR(base) ((base) + (0x00F0))
#define DPU_WCH_CH_DBG2_ADDR(base) ((base) + (0x00F4))
#define DPU_WCH_CH_DBG3_ADDR(base) ((base) + (0x00F8))
#define DPU_WCH_CH_DBG4_ADDR(base) ((base) + (0x00FC))
#define DPU_WCH_CH_DBG5_ADDR(base) ((base) + (0x00C0))
#define DPU_WCH_FRM_END_WCH_DELAY_ADDR(base) ((base) + (0x00C4))
#define DPU_WCH_WB_ERR_CTRL_ADDR(base) ((base) + (0x00C8))
#define DPU_WCH_CH_DBG6_ADDR(base) ((base) + (0x00CC))
#define DPU_WCH_DFC_DISP_SIZE_ADDR(base) ((base) + (0x0100))
#define DPU_WCH_DFC_PIX_IN_NUM_ADDR(base) ((base) + (0x0104))
#define DPU_WCH_DFC_GLB_ALPHA01_ADDR(base) ((base) + (0x0108))
#define DPU_WCH_DFC_DISP_FMT_ADDR(base) ((base) + (0x010C))
#define DPU_WCH_DFC_CLIP_CTL_HRZ_ADDR(base) ((base) + (0x0110))
#define DPU_WCH_DFC_CLIP_CTL_VRZ_ADDR(base) ((base) + (0x0114))
#define DPU_WCH_DFC_CTL_CLIP_EN_ADDR(base) ((base) + (0x0118))
#define DPU_WCH_DFC_ICG_MODULE_ADDR(base) ((base) + (0x011C))
#define DPU_WCH_DFC_DITHER_ENABLE_ADDR(base) ((base) + (0x0120))
#define DPU_WCH_DFC_PADDING_CTL_ADDR(base) ((base) + (0x0124))
#define DPU_WCH_DFC_GLB_ALPHA23_ADDR(base) ((base) + (0x0128))
#define DPU_WCH_DFC_GLB_ALPHA_ADDR(base) ((base) + (0x012C))
#define DPU_WCH_DFC_ALPHA_CTL_ADDR(base) ((base) + (0x0130))
#define DPU_WCH_DFC_ALPHA_THD_ADDR(base) ((base) + (0x0134))
#define DPU_WCH_BITEXT_CTL_ADDR(base) ((base) + (0x0140))
#define DPU_WCH_BITEXT_REG_INI0_0_ADDR(base) ((base) + (0x0144))
#define DPU_WCH_BITEXT_REG_INI0_1_ADDR(base) ((base) + (0x0148))
#define DPU_WCH_BITEXT_REG_INI0_2_ADDR(base) ((base) + (0x014C))
#define DPU_WCH_BITEXT_REG_INI0_3_ADDR(base) ((base) + (0x0150))
#define DPU_WCH_BITEXT_REG_INI1_0_ADDR(base) ((base) + (0x0154))
#define DPU_WCH_BITEXT_REG_INI1_1_ADDR(base) ((base) + (0x0158))
#define DPU_WCH_BITEXT_REG_INI1_2_ADDR(base) ((base) + (0x015C))
#define DPU_WCH_BITEXT_REG_INI1_3_ADDR(base) ((base) + (0x0160))
#define DPU_WCH_BITEXT_REG_INI2_0_ADDR(base) ((base) + (0x0164))
#define DPU_WCH_BITEXT_REG_INI2_1_ADDR(base) ((base) + (0x0168))
#define DPU_WCH_BITEXT_REG_INI2_2_ADDR(base) ((base) + (0x016C))
#define DPU_WCH_BITEXT_REG_INI2_3_ADDR(base) ((base) + (0x0170))
#define DPU_WCH_BITEXT_POWER_CTRL_C_ADDR(base) ((base) + (0x0174))
#define DPU_WCH_BITEXT_POWER_CTRL_SHIFT_ADDR(base) ((base) + (0x0178))
#define DPU_WCH_BITEXT_FILT_00_ADDR(base) ((base) + (0x017C))
#define DPU_WCH_BITEXT_FILT_01_ADDR(base) ((base) + (0x0180))
#define DPU_WCH_BITEXT_FILT_02_ADDR(base) ((base) + (0x0184))
#define DPU_WCH_BITEXT_FILT_10_ADDR(base) ((base) + (0x0188))
#define DPU_WCH_BITEXT_FILT_11_ADDR(base) ((base) + (0x018C))
#define DPU_WCH_BITEXT_FILT_12_ADDR(base) ((base) + (0x0190))
#define DPU_WCH_BITEXT_FILT_20_ADDR(base) ((base) + (0x0194))
#define DPU_WCH_BITEXT_FILT_21_ADDR(base) ((base) + (0x01A0))
#define DPU_WCH_BITEXT_FILT_22_ADDR(base) ((base) + (0x01A4))
#define DPU_WCH_BITEXT_THD_R0_ADDR(base) ((base) + (0x01A8))
#define DPU_WCH_BITEXT_THD_R1_ADDR(base) ((base) + (0x01AC))
#define DPU_WCH_BITEXT_THD_G0_ADDR(base) ((base) + (0x01B0))
#define DPU_WCH_BITEXT_THD_G1_ADDR(base) ((base) + (0x01B4))
#define DPU_WCH_BITEXT_THD_B0_ADDR(base) ((base) + (0x01B8))
#define DPU_WCH_BITEXT_THD_B1_ADDR(base) ((base) + (0x01BC))
#define DPU_WCH_BITEXT0_DBG0_ADDR(base) ((base) + (0x01C0))
#define DPU_WCH_DITHER_CTL1_ADDR(base) ((base) + (0x01D0))
#define DPU_WCH_DITHER_TRI_THD10_ADDR(base) ((base) + (0x01DC))
#define DPU_WCH_DITHER_TRI_THD10_UNI_ADDR(base) ((base) + (0x01E8))
#define DPU_WCH_BAYER_CTL_ADDR(base) ((base) + (0x01EC))
#define DPU_WCH_BAYER_MATRIX_PART1_ADDR(base) ((base) + (0x01F4))
#define DPU_WCH_BAYER_MATRIX_PART0_ADDR(base) ((base) + (0x01F8))
#define DPU_WCH_SCF_EN_HSCL_STR_ADDR(base) ((base) + (0x0200))
#define DPU_WCH_SCF_EN_VSCL_STR_ADDR(base) ((base) + (0x0204))
#define DPU_WCH_SCF_H_V_ORDER_ADDR(base) ((base) + (0x0208))
#define DPU_WCH_SCF_CH_CORE_GT_ADDR(base) ((base) + (0x020C))
#define DPU_WCH_SCF_INPUT_WIDTH_HEIGHT_ADDR(base) ((base) + (0x0210))
#define DPU_WCH_SCF_OUTPUT_WIDTH_HEIGHT_ADDR(base) ((base) + (0x0214))
#define DPU_WCH_SCF_COEF_MEM_CTRL_ADDR(base) ((base) + (0x0218))
#define DPU_WCH_SCF_EN_HSCL_ADDR(base) ((base) + (0x021C))
#define DPU_WCH_SCF_EN_VSCL_ADDR(base) ((base) + (0x0220))
#define DPU_WCH_SCF_ACC_HSCL_ADDR(base) ((base) + (0x0224))
#define DPU_WCH_SCF_ACC_HSCL1_ADDR(base) ((base) + (0x0228))
#define DPU_WCH_SCF_INC_HSCL_ADDR(base) ((base) + (0x0234))
#define DPU_WCH_SCF_ACC_VSCL_ADDR(base) ((base) + (0x0238))
#define DPU_WCH_SCF_ACC_VSCL1_ADDR(base) ((base) + (0x023C))
#define DPU_WCH_SCF_INC_VSCL_ADDR(base) ((base) + (0x0248))
#define DPU_WCH_SCF_EN_NONLINEAR_ADDR(base) ((base) + (0x024C))
#define DPU_WCH_SCF_EN_MMP_ADDR(base) ((base) + (0x027C))
#define DPU_WCH_SCF_DB_H0_ADDR(base) ((base) + (0x0280))
#define DPU_WCH_SCF_DB_H1_ADDR(base) ((base) + (0x0284))
#define DPU_WCH_SCF_DB_V0_ADDR(base) ((base) + (0x0288))
#define DPU_WCH_SCF_DB_V1_ADDR(base) ((base) + (0x028C))
#define DPU_WCH_SCF_LB_MEM_CTRL_ADDR(base) ((base) + (0x0290))
#define DPU_WCH_PCSC_IDC0_ADDR(base) ((base) + (0x0400))
#define DPU_WCH_PCSC_IDC2_ADDR(base) ((base) + (0x0404))
#define DPU_WCH_PCSC_ODC0_ADDR(base) ((base) + (0x0408))
#define DPU_WCH_PCSC_ODC2_ADDR(base) ((base) + (0x040C))
#define DPU_WCH_PCSC_P00_ADDR(base) ((base) + (0x0410))
#define DPU_WCH_PCSC_P01_ADDR(base) ((base) + (0x0414))
#define DPU_WCH_PCSC_P02_ADDR(base) ((base) + (0x0418))
#define DPU_WCH_PCSC_P10_ADDR(base) ((base) + (0x041C))
#define DPU_WCH_PCSC_P11_ADDR(base) ((base) + (0x0420))
#define DPU_WCH_PCSC_P12_ADDR(base) ((base) + (0x0424))
#define DPU_WCH_PCSC_P20_ADDR(base) ((base) + (0x0428))
#define DPU_WCH_PCSC_P21_ADDR(base) ((base) + (0x042C))
#define DPU_WCH_PCSC_P22_ADDR(base) ((base) + (0x0430))
#define DPU_WCH_PCSC_ICG_MODULE_ADDR(base) ((base) + (0x0434))
#define DPU_WCH_POST_CLIP_DISP_SIZE_ADDR(base) ((base) + (0x0480))
#define DPU_WCH_POST_CLIP_CTL_HRZ_ADDR(base) ((base) + (0x0484))
#define DPU_WCH_POST_CLIP_CTL_VRZ_ADDR(base) ((base) + (0x0488))
#define DPU_WCH_POST_CLIP_EN_ADDR(base) ((base) + (0x048C))
#define DPU_WCH_CSC_IDC0_ADDR(base) ((base) + (0x0500))
#define DPU_WCH_CSC_IDC2_ADDR(base) ((base) + (0x0504))
#define DPU_WCH_CSC_ODC0_ADDR(base) ((base) + (0x0508))
#define DPU_WCH_CSC_ODC2_ADDR(base) ((base) + (0x050C))
#define DPU_WCH_CSC_P00_ADDR(base) ((base) + (0x0510))
#define DPU_WCH_CSC_P01_ADDR(base) ((base) + (0x0514))
#define DPU_WCH_CSC_P02_ADDR(base) ((base) + (0x0518))
#define DPU_WCH_CSC_P10_ADDR(base) ((base) + (0x051C))
#define DPU_WCH_CSC_P11_ADDR(base) ((base) + (0x0520))
#define DPU_WCH_CSC_P12_ADDR(base) ((base) + (0x0524))
#define DPU_WCH_CSC_P20_ADDR(base) ((base) + (0x0528))
#define DPU_WCH_CSC_P21_ADDR(base) ((base) + (0x052C))
#define DPU_WCH_CSC_P22_ADDR(base) ((base) + (0x0530))
#define DPU_WCH_CSC_ICG_MODULE_ADDR(base) ((base) + (0x0534))
#define DPU_WCH_ROT_FIRST_LNS_ADDR(base) ((base) + (0x0580))
#define DPU_WCH_ROT_STATE_ADDR(base) ((base) + (0x0584))
#define DPU_WCH_ROT_MEM_CTRL_ADDR(base) ((base) + (0x0588))
#define DPU_WCH_ROT_SIZE_ADDR(base) ((base) + (0x058C))
#define DPU_WCH_ROT_422_MODE_ADDR(base) ((base) + (0x0590))
#define DPU_WCH_CH_DEBUG_SEL_ADDR(base) ((base) + (0x600))
#define DPU_WCH_DMA_ADDR_EXT_ADDR(base) ((base) + (0x604))
#define DPU_WCH_REG_CTRL_ADDR(base) ((base) + (0x608))
#define DPU_WCH_REG_CTRL_FLUSH_EN_ADDR(base) ((base) + (0x60C))
#define DPU_WCH_REG_CTRL_DEBUG_ADDR(base) ((base) + (0x610))
#define DPU_WCH_REG_CTRL_STATE_ADDR(base) ((base) + (0x614))
#define DPU_WCH_R_LB_DEBUG0_ADDR(base) ((base) + (0x620))
#define DPU_WCH_R_LB_DEBUG1_ADDR(base) ((base) + (0x624))
#define DPU_WCH_R_LB_DEBUG2_ADDR(base) ((base) + (0x628))
#define DPU_WCH_R_LB_DEBUG3_ADDR(base) ((base) + (0x62C))
#define DPU_WCH_REG_CTRL_LAYER_ID_ADDR(base) ((base) + (0x630))
#define DPU_WCH_V0_SCF_VIDEO_6TAP_COEF_ADDR(base,l) ((base) + (0x1000+0x4*(l)))
#define DPU_WCH_V0_SCF_VIDEO_5TAP_COEF_ADDR(base,m) ((base) + (0x3000+0x4*(m)))
#define DPU_WCH_V0_SCF_VIDEO_4TAP_COEF_ADDR(base,n) ((base) + (0x3800+0x4*(n)))
#define DPU_DBCU_SMARTDMA_0_AXI_SEL_ADDR(base) ((base) + (0x0000))
#define DPU_DBCU_SMARTDMA_1_AXI_SEL_ADDR(base) ((base) + (0x0004))
#define DPU_DBCU_SMARTDMA_2_AXI_SEL_ADDR(base) ((base) + (0x0008))
#define DPU_DBCU_SMARTDMA_3_AXI_SEL_ADDR(base) ((base) + (0x000C))
#define DPU_DBCU_WCH_0_AXI_SEL_ADDR(base) ((base) + (0x0010))
#define DPU_DBCU_WCH_1_AXI_SEL_ADDR(base) ((base) + (0x0014))
#define DPU_DBCU_WCH_2_AXI_SEL_ADDR(base) ((base) + (0x0018))
#define DPU_DBCU_CMDLIST_AXI_SEL_ADDR(base) ((base) + (0x001C))
#define DPU_DBCU_ARCACH0_ADDR_THRH_ADDR(base) ((base) + (0x0020))
#define DPU_DBCU_ARCACH0_ADDR_THRL_ADDR(base) ((base) + (0x0024))
#define DPU_DBCU_ARCACH0_VALUE0_ADDR(base) ((base) + (0x0028))
#define DPU_DBCU_ARCACH0_VALUE1_ADDR(base) ((base) + (0x002C))
#define DPU_DBCU_ARCACH1_ADDR_THRH_ADDR(base) ((base) + (0x0030))
#define DPU_DBCU_ARCACH1_ADDR_THRL_ADDR(base) ((base) + (0x0034))
#define DPU_DBCU_ARCACH1_VALUE0_ADDR(base) ((base) + (0x0038))
#define DPU_DBCU_ARCACH1_VALUE1_ADDR(base) ((base) + (0x003C))
#define DPU_DBCU_AWCACH0_ADDR_THRH_ADDR(base) ((base) + (0x0040))
#define DPU_DBCU_AWCACH0_ADDR_THRL_ADDR(base) ((base) + (0x0044))
#define DPU_DBCU_AWCACH0_VALUE0_ADDR(base) ((base) + (0x0048))
#define DPU_DBCU_AWCACH0_VALUE1_ADDR(base) ((base) + (0x004C))
#define DPU_DBCU_ARCACH2_ADDR_THRH_ADDR(base) ((base) + (0x0050))
#define DPU_DBCU_ARCACH2_ADDR_THRL_ADDR(base) ((base) + (0x0054))
#define DPU_DBCU_ARCACH2_VALUE0_ADDR(base) ((base) + (0x0058))
#define DPU_DBCU_ARCACH2_VALUE1_ADDR(base) ((base) + (0x005C))
#define DPU_DBCU_AWCACH2_ADDR_THRH_ADDR(base) ((base) + (0x0060))
#define DPU_DBCU_AWCACH2_ADDR_THRL_ADDR(base) ((base) + (0x0064))
#define DPU_DBCU_AWCACH2_VALUE0_ADDR(base) ((base) + (0x0068))
#define DPU_DBCU_AWCACH2_VALUE1_ADDR(base) ((base) + (0x006C))
#define DPU_DBCU_ARCACH3_ADDR_THRH_ADDR(base) ((base) + (0x0070))
#define DPU_DBCU_ARCACH3_ADDR_THRL_ADDR(base) ((base) + (0x0074))
#define DPU_DBCU_ARCACH3_VALUE0_ADDR(base) ((base) + (0x0078))
#define DPU_DBCU_ARCACH3_VALUE1_ADDR(base) ((base) + (0x007C))
#define DPU_DBCU_SMARTDMA_0_MID_ADDR(base) ((base) + (0x0400))
#define DPU_DBCU_SMARTDMA_1_MID_ADDR(base) ((base) + (0x0404))
#define DPU_DBCU_SMARTDMA_2_MID_ADDR(base) ((base) + (0x0408))
#define DPU_DBCU_SMARTDMA_3_MID_ADDR(base) ((base) + (0x040C))
#define DPU_DBCU_WCH_0_MID_ADDR(base) ((base) + (0x0410))
#define DPU_DBCU_WCH_1_MID_ADDR(base) ((base) + (0x0414))
#define DPU_DBCU_WCH_2_MID_ADDR(base) ((base) + (0x0418))
#define DPU_DBCU_CMDLIST_MID_ADDR(base) ((base) + (0x041C))
#define DPU_DBCU_MMU_ID_ATTR_0_ADDR(base) ((base) + (0x0420))
#define DPU_DBCU_MMU_ID_ATTR_1_ADDR(base) ((base) + (0x0424))
#define DPU_DBCU_MMU_ID_ATTR_2_ADDR(base) ((base) + (0x0428))
#define DPU_DBCU_MMU_ID_ATTR_3_ADDR(base) ((base) + (0x042C))
#define DPU_DBCU_MMU_ID_ATTR_4_ADDR(base) ((base) + (0x0430))
#define DPU_DBCU_MMU_ID_ATTR_5_ADDR(base) ((base) + (0x0434))
#define DPU_DBCU_MMU_ID_ATTR_6_ADDR(base) ((base) + (0x0438))
#define DPU_DBCU_MMU_ID_ATTR_7_ADDR(base) ((base) + (0x043C))
#define DPU_DBCU_MMU_ID_ATTR_8_ADDR(base) ((base) + (0x0440))
#define DPU_DBCU_MMU_ID_ATTR_9_ADDR(base) ((base) + (0x0444))
#define DPU_DBCU_MMU_ID_ATTR_10_ADDR(base) ((base) + (0x0448))
#define DPU_DBCU_MMU_ID_ATTR_11_ADDR(base) ((base) + (0x044C))
#define DPU_DBCU_MMU_ID_ATTR_12_ADDR(base) ((base) + (0x0450))
#define DPU_DBCU_MMU_ID_ATTR_13_ADDR(base) ((base) + (0x0454))
#define DPU_DBCU_MMU_ID_ATTR_14_ADDR(base) ((base) + (0x0458))
#define DPU_DBCU_MMU_ID_ATTR_15_ADDR(base) ((base) + (0x045C))
#define DPU_DBCU_MMU_ID_ATTR_16_ADDR(base) ((base) + (0x0460))
#define DPU_DBCU_MMU_ID_ATTR_17_ADDR(base) ((base) + (0x0464))
#define DPU_DBCU_MMU_ID_ATTR_18_ADDR(base) ((base) + (0x0468))
#define DPU_DBCU_MMU_ID_ATTR_19_ADDR(base) ((base) + (0x046C))
#define DPU_DBCU_MMU_ID_ATTR_20_ADDR(base) ((base) + (0x0470))
#define DPU_DBCU_MMU_ID_ATTR_21_ADDR(base) ((base) + (0x0474))
#define DPU_DBCU_MMU_ID_ATTR_22_ADDR(base) ((base) + (0x0478))
#define DPU_DBCU_MMU_ID_ATTR_23_ADDR(base) ((base) + (0x047C))
#define DPU_DBCU_MMU_ID_ATTR_24_ADDR(base) ((base) + (0x0480))
#define DPU_DBCU_MMU_ID_ATTR_25_ADDR(base) ((base) + (0x0484))
#define DPU_DBCU_MMU_ID_ATTR_26_ADDR(base) ((base) + (0x0488))
#define DPU_DBCU_MMU_ID_ATTR_27_ADDR(base) ((base) + (0x048C))
#define DPU_DBCU_MMU_ID_ATTR_28_ADDR(base) ((base) + (0x0490))
#define DPU_DBCU_MMU_ID_ATTR_29_ADDR(base) ((base) + (0x0494))
#define DPU_DBCU_MMU_ID_ATTR_30_ADDR(base) ((base) + (0x0498))
#define DPU_DBCU_MMU_ID_ATTR_31_ADDR(base) ((base) + (0x049C))
#define DPU_DBCU_MMU_ID_ATTR_32_ADDR(base) ((base) + (0x04A0))
#define DPU_DBCU_MMU_ID_ATTR_33_ADDR(base) ((base) + (0x04A4))
#define DPU_DBCU_MMU_ID_ATTR_34_ADDR(base) ((base) + (0x04A8))
#define DPU_DBCU_MMU_ID_ATTR_35_ADDR(base) ((base) + (0x04AC))
#define DPU_DBCU_MMU_ID_ATTR_36_ADDR(base) ((base) + (0x04B0))
#define DPU_DBCU_MMU_ID_ATTR_37_ADDR(base) ((base) + (0x04B4))
#define DPU_DBCU_MMU_ID_ATTR_38_ADDR(base) ((base) + (0x04B8))
#define DPU_DBCU_MMU_ID_ATTR_39_ADDR(base) ((base) + (0x04BC))
#define DPU_DBCU_MMU_ID_ATTR_40_ADDR(base) ((base) + (0x04C0))
#define DPU_DBCU_MMU_ID_ATTR_41_ADDR(base) ((base) + (0x04C4))
#define DPU_DBCU_MMU_ID_ATTR_42_ADDR(base) ((base) + (0x04C8))
#define DPU_DBCU_MMU_ID_ATTR_43_ADDR(base) ((base) + (0x04CC))
#define DPU_DBCU_MMU_ID_ATTR_44_ADDR(base) ((base) + (0x04D0))
#define DPU_DBCU_MMU_ID_ATTR_45_ADDR(base) ((base) + (0x04D4))
#define DPU_DBCU_MMU_ID_ATTR_46_ADDR(base) ((base) + (0x04D8))
#define DPU_DBCU_MMU_ID_ATTR_47_ADDR(base) ((base) + (0x04DC))
#define DPU_DBCU_MMU_ID_ATTR_48_ADDR(base) ((base) + (0x04E0))
#define DPU_DBCU_MMU_ID_ATTR_49_ADDR(base) ((base) + (0x04E4))
#define DPU_DBCU_MMU_ID_ATTR_50_ADDR(base) ((base) + (0x04E8))
#define DPU_DBCU_MMU_ID_ATTR_51_ADDR(base) ((base) + (0x04EC))
#define DPU_DBCU_MMU_ID_ATTR_52_ADDR(base) ((base) + (0x04F0))
#define DPU_DBCU_MMU_ID_ATTR_53_ADDR(base) ((base) + (0x04F4))
#define DPU_DBCU_MMU_ID_ATTR_54_ADDR(base) ((base) + (0x04F8))
#define DPU_DBCU_MMU_ID_ATTR_55_ADDR(base) ((base) + (0x04FC))
#define DPU_DBCU_MMU_ID_ATTR_56_ADDR(base) ((base) + (0x0500))
#define DPU_DBCU_MMU_ID_ATTR_57_ADDR(base) ((base) + (0x0504))
#define DPU_DBCU_MMU_ID_ATTR_58_ADDR(base) ((base) + (0x0508))
#define DPU_DBCU_MMU_ID_ATTR_59_ADDR(base) ((base) + (0x050C))
#define DPU_DBCU_MMU_ID_ATTR_60_ADDR(base) ((base) + (0x0510))
#define DPU_DBCU_MMU_ID_ATTR_61_ADDR(base) ((base) + (0x0514))
#define DPU_DBCU_MMU_ID_ATTR_62_ADDR(base) ((base) + (0x0518))
#define DPU_DBCU_MMU_ID_ATTR_63_ADDR(base) ((base) + (0x051C))
#define DPU_DBCU_MMU_ID_ATTR_64_ADDR(base) ((base) + (0x0520))
#define DPU_DBCU_MMU_ID_ATTR_65_ADDR(base) ((base) + (0x0524))
#define DPU_DBCU_MMU_ID_ATTR_66_ADDR(base) ((base) + (0x0528))
#define DPU_DBCU_MMU_ID_ATTR_67_ADDR(base) ((base) + (0x052C))
#define DPU_DBCU_MMU_ID_ATTR_68_ADDR(base) ((base) + (0x0530))
#define DPU_DBCU_MMU_ID_ATTR_69_ADDR(base) ((base) + (0x0534))
#define DPU_DBCU_MMU_ID_ATTR_70_ADDR(base) ((base) + (0x0538))
#define DPU_DBCU_MMU_ID_ATTR_71_ADDR(base) ((base) + (0x053C))
#define DPU_DBCU_MMU_ID_ATTR_72_ADDR(base) ((base) + (0x0540))
#define DPU_DBCU_MMU_ID_ATTR_73_ADDR(base) ((base) + (0x0544))
#define DPU_DBCU_MMU_ID_ATTR_74_ADDR(base) ((base) + (0x0548))
#define DPU_DBCU_MMU_ID_ATTR_75_ADDR(base) ((base) + (0x054C))
#define DPU_DBCU_MMU_ID_ATTR_76_ADDR(base) ((base) + (0x0550))
#define DPU_DBCU_MMU_ID_ATTR_77_ADDR(base) ((base) + (0x0554))
#define DPU_DBCU_MMU_ID_ATTR_78_ADDR(base) ((base) + (0x0558))
#define DPU_DBCU_MMU_ID_ATTR_79_ADDR(base) ((base) + (0x055C))
#define DPU_DBCU_MMU_ID_ATTR_NS_0_ADDR(base) ((base) + (0x0560))
#define DPU_DBCU_MMU_ID_ATTR_NS_1_ADDR(base) ((base) + (0x0564))
#define DPU_DBCU_MMU_ID_ATTR_NS_2_ADDR(base) ((base) + (0x0568))
#define DPU_DBCU_MMU_ID_ATTR_NS_3_ADDR(base) ((base) + (0x056C))
#define DPU_DBCU_MMU_ID_ATTR_NS_4_ADDR(base) ((base) + (0x0570))
#define DPU_DBCU_MMU_ID_ATTR_NS_5_ADDR(base) ((base) + (0x0574))
#define DPU_DBCU_MMU_ID_ATTR_NS_6_ADDR(base) ((base) + (0x0578))
#define DPU_DBCU_MMU_ID_ATTR_NS_7_ADDR(base) ((base) + (0x057C))
#define DPU_DBCU_MMU_ID_ATTR_NS_8_ADDR(base) ((base) + (0x0580))
#define DPU_DBCU_MMU_ID_ATTR_NS_9_ADDR(base) ((base) + (0x0584))
#define DPU_DBCU_MMU_ID_ATTR_NS_10_ADDR(base) ((base) + (0x0588))
#define DPU_DBCU_MMU_ID_ATTR_NS_11_ADDR(base) ((base) + (0x058C))
#define DPU_DBCU_MMU_ID_ATTR_NS_12_ADDR(base) ((base) + (0x0590))
#define DPU_DBCU_MMU_ID_ATTR_NS_13_ADDR(base) ((base) + (0x0594))
#define DPU_DBCU_MMU_ID_ATTR_NS_14_ADDR(base) ((base) + (0x0598))
#define DPU_DBCU_MMU_ID_ATTR_NS_15_ADDR(base) ((base) + (0x059C))
#define DPU_DBCU_MMU_ID_ATTR_NS_16_ADDR(base) ((base) + (0x05A0))
#define DPU_DBCU_MMU_ID_ATTR_NS_17_ADDR(base) ((base) + (0x05A4))
#define DPU_DBCU_MMU_ID_ATTR_NS_18_ADDR(base) ((base) + (0x05A8))
#define DPU_DBCU_MMU_ID_ATTR_NS_19_ADDR(base) ((base) + (0x05AC))
#define DPU_DBCU_MMU_ID_ATTR_NS_20_ADDR(base) ((base) + (0x05B0))
#define DPU_DBCU_MMU_ID_ATTR_NS_21_ADDR(base) ((base) + (0x05B4))
#define DPU_DBCU_MMU_ID_ATTR_NS_22_ADDR(base) ((base) + (0x05B8))
#define DPU_DBCU_MMU_ID_ATTR_NS_23_ADDR(base) ((base) + (0x05BC))
#define DPU_DBCU_MMU_ID_ATTR_NS_24_ADDR(base) ((base) + (0x05C0))
#define DPU_DBCU_MMU_ID_ATTR_NS_25_ADDR(base) ((base) + (0x05C4))
#define DPU_DBCU_MMU_ID_ATTR_NS_26_ADDR(base) ((base) + (0x05C8))
#define DPU_DBCU_MMU_ID_ATTR_NS_27_ADDR(base) ((base) + (0x05CC))
#define DPU_DBCU_MMU_ID_ATTR_NS_28_ADDR(base) ((base) + (0x05D0))
#define DPU_DBCU_MMU_ID_ATTR_NS_29_ADDR(base) ((base) + (0x05D4))
#define DPU_DBCU_MMU_ID_ATTR_NS_30_ADDR(base) ((base) + (0x05D8))
#define DPU_DBCU_MMU_ID_ATTR_NS_31_ADDR(base) ((base) + (0x05DC))
#define DPU_DBCU_MMU_ID_ATTR_NS_32_ADDR(base) ((base) + (0x05E0))
#define DPU_DBCU_MMU_ID_ATTR_NS_33_ADDR(base) ((base) + (0x05E4))
#define DPU_DBCU_MMU_ID_ATTR_NS_34_ADDR(base) ((base) + (0x05E8))
#define DPU_DBCU_MMU_ID_ATTR_NS_35_ADDR(base) ((base) + (0x05EC))
#define DPU_DBCU_MMU_ID_ATTR_NS_36_ADDR(base) ((base) + (0x05F0))
#define DPU_DBCU_MMU_ID_ATTR_NS_37_ADDR(base) ((base) + (0x05F4))
#define DPU_DBCU_MMU_ID_ATTR_NS_38_ADDR(base) ((base) + (0x05F8))
#define DPU_DBCU_MMU_ID_ATTR_NS_39_ADDR(base) ((base) + (0x05FC))
#define DPU_DBCU_MMU_ID_ATTR_NS_40_ADDR(base) ((base) + (0x0600))
#define DPU_DBCU_MMU_ID_ATTR_NS_41_ADDR(base) ((base) + (0x0604))
#define DPU_DBCU_MMU_ID_ATTR_NS_42_ADDR(base) ((base) + (0x0608))
#define DPU_DBCU_MMU_ID_ATTR_NS_43_ADDR(base) ((base) + (0x060C))
#define DPU_DBCU_MMU_ID_ATTR_NS_44_ADDR(base) ((base) + (0x0610))
#define DPU_DBCU_MMU_ID_ATTR_NS_45_ADDR(base) ((base) + (0x0614))
#define DPU_DBCU_MMU_ID_ATTR_NS_46_ADDR(base) ((base) + (0x0618))
#define DPU_DBCU_MMU_ID_ATTR_NS_47_ADDR(base) ((base) + (0x061C))
#define DPU_DBCU_MMU_ID_ATTR_NS_48_ADDR(base) ((base) + (0x0620))
#define DPU_DBCU_MMU_ID_ATTR_NS_49_ADDR(base) ((base) + (0x0624))
#define DPU_DBCU_MMU_ID_ATTR_NS_50_ADDR(base) ((base) + (0x0628))
#define DPU_DBCU_MMU_ID_ATTR_NS_51_ADDR(base) ((base) + (0x062C))
#define DPU_DBCU_MMU_ID_ATTR_NS_52_ADDR(base) ((base) + (0x0630))
#define DPU_DBCU_MMU_ID_ATTR_NS_53_ADDR(base) ((base) + (0x0634))
#define DPU_DBCU_MMU_ID_ATTR_NS_54_ADDR(base) ((base) + (0x0638))
#define DPU_DBCU_MMU_ID_ATTR_NS_55_ADDR(base) ((base) + (0x063C))
#define DPU_DBCU_MMU_ID_ATTR_NS_56_ADDR(base) ((base) + (0x0640))
#define DPU_DBCU_MMU_ID_ATTR_NS_57_ADDR(base) ((base) + (0x0644))
#define DPU_DBCU_MMU_ID_ATTR_NS_58_ADDR(base) ((base) + (0x0648))
#define DPU_DBCU_MMU_ID_ATTR_NS_59_ADDR(base) ((base) + (0x064C))
#define DPU_DBCU_MMU_ID_ATTR_NS_60_ADDR(base) ((base) + (0x0650))
#define DPU_DBCU_MMU_ID_ATTR_NS_61_ADDR(base) ((base) + (0x0654))
#define DPU_DBCU_MMU_ID_ATTR_NS_62_ADDR(base) ((base) + (0x0658))
#define DPU_DBCU_MMU_ID_ATTR_NS_63_ADDR(base) ((base) + (0x065C))
#define DPU_DBCU_MMU_ID_ATTR_NS_64_ADDR(base) ((base) + (0x0660))
#define DPU_DBCU_MMU_ID_ATTR_NS_65_ADDR(base) ((base) + (0x0664))
#define DPU_DBCU_MMU_ID_ATTR_NS_66_ADDR(base) ((base) + (0x0668))
#define DPU_DBCU_MMU_ID_ATTR_NS_67_ADDR(base) ((base) + (0x066C))
#define DPU_DBCU_MMU_ID_ATTR_NS_68_ADDR(base) ((base) + (0x0670))
#define DPU_DBCU_MMU_ID_ATTR_NS_69_ADDR(base) ((base) + (0x0674))
#define DPU_DBCU_MMU_ID_ATTR_NS_70_ADDR(base) ((base) + (0x0678))
#define DPU_DBCU_MMU_ID_ATTR_NS_71_ADDR(base) ((base) + (0x067C))
#define DPU_DBCU_MMU_ID_ATTR_NS_72_ADDR(base) ((base) + (0x0680))
#define DPU_DBCU_MMU_ID_ATTR_NS_73_ADDR(base) ((base) + (0x0684))
#define DPU_DBCU_MMU_ID_ATTR_NS_74_ADDR(base) ((base) + (0x0688))
#define DPU_DBCU_MMU_ID_ATTR_NS_75_ADDR(base) ((base) + (0x068C))
#define DPU_DBCU_MMU_ID_ATTR_NS_76_ADDR(base) ((base) + (0x0690))
#define DPU_DBCU_MMU_ID_ATTR_NS_77_ADDR(base) ((base) + (0x0694))
#define DPU_DBCU_MMU_ID_ATTR_NS_78_ADDR(base) ((base) + (0x0698))
#define DPU_DBCU_MMU_ID_ATTR_NS_79_ADDR(base) ((base) + (0x069C))
#define DPU_DBCU_AXI2_RID_MSK0_ADDR(base) ((base) + (0x0720))
#define DPU_DBCU_AXI2_RID_MSK1_ADDR(base) ((base) + (0x0724))
#define DPU_DBCU_AXI2_WID_MSK_ADDR(base) ((base) + (0x0728))
#define DPU_DBCU_AXI2_R_QOS_MAP_ADDR(base) ((base) + (0x072C))
#define DPU_DBCU_AXI3_RID_MSK0_ADDR(base) ((base) + (0x0730))
#define DPU_DBCU_AXI3_RID_MSK1_ADDR(base) ((base) + (0x0734))
#define DPU_DBCU_AXI3_R_QOS_MAP_ADDR(base) ((base) + (0x0738))
#define DPU_DBCU_AXI0_RID_MSK0_ADDR(base) ((base) + (0x073C))
#define DPU_DBCU_AXI0_RID_MSK1_ADDR(base) ((base) + (0x0740))
#define DPU_DBCU_AXI0_WID_MSK_ADDR(base) ((base) + (0x0744))
#define DPU_DBCU_AXI0_R_QOS_MAP_ADDR(base) ((base) + (0x0748))
#define DPU_DBCU_AXI1_RID_MSK0_ADDR(base) ((base) + (0x074C))
#define DPU_DBCU_AXI1_RID_MSK1_ADDR(base) ((base) + (0x0750))
#define DPU_DBCU_AXI1_R_QOS_MAP_ADDR(base) ((base) + (0x0754))
#define DPU_DBCU_AXI0_READ_FLUX_HIGH_CFG_ADDR(base) ((base) + (0x0758))
#define DPU_DBCU_AXI0_READ_FLUX_LOW_CFG_ADDR(base) ((base) + (0x075C))
#define DPU_DBCU_AXI0_WRITE_FLUX_HIGH_CFG_ADDR(base) ((base) + (0x0760))
#define DPU_DBCU_AXI0_WRITE_FLUX_LOW_CFG_ADDR(base) ((base) + (0x0764))
#define DPU_DBCU_AXI1_READ_FLUX_HIGH_CFG_ADDR(base) ((base) + (0x0768))
#define DPU_DBCU_AXI1_READ_FLUX_LOW_CFG_ADDR(base) ((base) + (0x076C))
#define DPU_DBCU_AXI2_READ_FLUX_HIGH_CFG_ADDR(base) ((base) + (0x0770))
#define DPU_DBCU_AXI2_READ_FLUX_LOW_CFG_ADDR(base) ((base) + (0x0774))
#define DPU_DBCU_AXI2_WRITE_FLUX_HIGH_CFG_ADDR(base) ((base) + (0x0778))
#define DPU_DBCU_AXI2_WRITE_FLUX_LOW_CFG_ADDR(base) ((base) + (0x077C))
#define DPU_DBCU_AXI3_READ_FLUX_HIGH_CFG_ADDR(base) ((base) + (0x0780))
#define DPU_DBCU_AXI3_READ_FLUX_LOW_CFG_ADDR(base) ((base) + (0x0784))
#define DPU_DBCU_CLK_SEL_0_ADDR(base) ((base) + (0x0788))
#define DPU_DBCU_CLK_SEL_1_ADDR(base) ((base) + (0x078C))
#define DPU_DBCU_CLK_EN_0_ADDR(base) ((base) + (0x0790))
#define DPU_DBCU_CLK_EN_1_ADDR(base) ((base) + (0x0794))
#define DPU_DBCU_MONITOR_TIMER_INI_ADDR(base) ((base) + (0x0798))
#define DPU_DBCU_DEBUG_BUF_BASE_ADDR(base) ((base) + (0x079C))
#define DPU_DBCU_DEBUG_CTRL_ADDR(base) ((base) + (0x07A0))
#define DPU_DBCU_AIF_SHADOW_READ_ADDR(base) ((base) + (0x07A4))
#define DPU_DBCU_MONITOR_EN_ADDR(base) ((base) + (0x07A8))
#define DPU_DBCU_MONITOR_CTRL_ADDR(base) ((base) + (0x07AC))
#define DPU_DBCU_MONITOR_SAMPLE_MUN_ADDR(base) ((base) + (0x07B0))
#define DPU_DBCU_MONITOR_SAMPLE_TIME_ADDR(base) ((base) + (0x07B4))
#define DPU_DBCU_MONITOR_SAMPLE_FLOW_ADDR(base) ((base) + (0x07B8))
#define DPU_DBCU_AXI_DOMAIN_CFG_ADDR(base) ((base) + (0x07BC))
#define DPU_DBCU_MONITOR_WR_ST_0_ADDR(base) ((base) + (0x0870))
#define DPU_DBCU_MONITOR_WR_ST_1_ADDR(base) ((base) + (0x0880))
#define DPU_DBCU_MONITOR_WR_ST_2_ADDR(base) ((base) + (0x0884))
#define DPU_DBCU_MONITOR_WR_ST_3_ADDR(base) ((base) + (0x0888))
#define DPU_DBCU_MONITOR_WR_ST_4_ADDR(base) ((base) + (0x088C))
#define DPU_DBCU_MONITOR_WR_ST_5_ADDR(base) ((base) + (0x0890))
#define DPU_DBCU_MONITOR_WR_ST_6_ADDR(base) ((base) + (0x0894))
#define DPU_DBCU_MONITOR_WR_ST_7_ADDR(base) ((base) + (0x0898))
#define DPU_DBCU_MONITOR_WR_ST_8_ADDR(base) ((base) + (0x089C))
#define DPU_DBCU_MONITOR_RD_ST_0_ADDR(base) ((base) + (0x08A0))
#define DPU_DBCU_MONITOR_RD_ST_1_ADDR(base) ((base) + (0x08B0))
#define DPU_DBCU_MONITOR_RD_ST_2_ADDR(base) ((base) + (0x08B4))
#define DPU_DBCU_MONITOR_RD_ST_3_ADDR(base) ((base) + (0x08B8))
#define DPU_DBCU_MONITOR_RD_ST_4_ADDR(base) ((base) + (0x08BC))
#define DPU_DBCU_MONITOR_RD_ST_5_ADDR(base) ((base) + (0x08C0))
#define DPU_DBCU_MONITOR_RD_ST_6_ADDR(base) ((base) + (0x08C4))
#define DPU_DBCU_MONITOR_RD_ST_7_ADDR(base) ((base) + (0x08C8))
#define DPU_DBCU_MONITOR_RD_ST_8_ADDR(base) ((base) + (0x08CC))
#define DPU_DBCU_MONITOR_OS_R0_ADDR(base) ((base) + (0x08D0))
#define DPU_DBCU_MONITOR_OS_R1_ADDR(base) ((base) + (0x08E0))
#define DPU_DBCU_MONITOR_OS_R2_ADDR(base) ((base) + (0x08E4))
#define DPU_DBCU_MONITOR_OS_R3_ADDR(base) ((base) + (0x08E8))
#define DPU_DBCU_MONITOR_OS_R4_ADDR(base) ((base) + (0x08EC))
#define DPU_DBCU_MONITOR_OS_W0_ADDR(base) ((base) + (0x08F0))
#define DPU_DBCU_MONITOR_OS_W1_ADDR(base) ((base) + (0x0900))
#define DPU_DBCU_MONITOR_OS_W2_ADDR(base) ((base) + (0x0904))
#define DPU_DBCU_MONITOR_OS_W3_ADDR(base) ((base) + (0x090C))
#define DPU_DBCU_AIF_STAT_0_ADDR(base) ((base) + (0x0910))
#define DPU_DBCU_AIF_STAT_1_ADDR(base) ((base) + (0x0914))
#define DPU_DBCU_AIF_STAT_2_ADDR(base) ((base) + (0x0918))
#define DPU_DBCU_AIF_STAT_3_ADDR(base) ((base) + (0x091C))
#define DPU_DBCU_AIF_STAT_4_ADDR(base) ((base) + (0x0920))
#define DPU_DBCU_AIF_STAT_5_ADDR(base) ((base) + (0x0924))
#define DPU_DBCU_AIF_STAT_6_ADDR(base) ((base) + (0x0928))
#define DPU_DBCU_AIF_STAT_7_ADDR(base) ((base) + (0x092C))
#define DPU_DBCU_AIF_STAT_8_ADDR(base) ((base) + (0x0930))
#define DPU_DBCU_AIF_STAT_9_ADDR(base) ((base) + (0x0934))
#define DPU_DBCU_AIF_STAT_10_ADDR(base) ((base) + (0x0938))
#define DPU_DBCU_AIF_STAT_11_ADDR(base) ((base) + (0x093C))
#define DPU_DBCU_AIF_STAT_12_ADDR(base) ((base) + (0x0940))
#define DPU_DBCU_AIF_STAT_13_ADDR(base) ((base) + (0x0944))
#define DPU_DBCU_AIF_STAT_14_ADDR(base) ((base) + (0x0948))
#define DPU_DBCU_AIF_STAT_15_ADDR(base) ((base) + (0x094C))
#define DPU_DBCU_MIF_SHADOW_READ_ADDR(base) ((base) + (0x0B08))
#define DPU_DBCU_AIF_CMD_RELOAD_ADDR(base) ((base) + (0x0B0C))
#define DPU_DBCU_MIF_CTRL_SMARTDMA_0_ADDR(base) ((base) + (0x0B10))
#define DPU_DBCU_MIF_LEAD_SMARTDMA_0_ADDR(base) ((base) + (0x0B14))
#define DPU_DBCU_MIF_OS_SMARTDMA_0_ADDR(base) ((base) + (0x0B18))
#define DPU_DBCU_MIF_STAT_SMARTDMA_0_ADDR(base) ((base) + (0x0B1C))
#define DPU_DBCU_MIF_CTRL_SMARTDMA_1_ADDR(base) ((base) + (0x0B20))
#define DPU_DBCU_MIF_LEAD_SMARTDMA_1_ADDR(base) ((base) + (0x0B24))
#define DPU_DBCU_MIF_OS_SMARTDMA_1_ADDR(base) ((base) + (0x0B28))
#define DPU_DBCU_MIF_STAT_SMARTDMA_1_ADDR(base) ((base) + (0x0B2C))
#define DPU_DBCU_MIF_CTRL_SMARTDMA_2_ADDR(base) ((base) + (0x0B30))
#define DPU_DBCU_MIF_LEAD_SMARTDMA_2_ADDR(base) ((base) + (0x0B34))
#define DPU_DBCU_MIF_OS_SMARTDMA_2_ADDR(base) ((base) + (0x0B38))
#define DPU_DBCU_MIF_STAT_SMARTDMA_2_ADDR(base) ((base) + (0x0B3C))
#define DPU_DBCU_MIF_CTRL_SMARTDMA_3_ADDR(base) ((base) + (0x0B40))
#define DPU_DBCU_MIF_LEAD_SMARTDMA_3_ADDR(base) ((base) + (0x0B44))
#define DPU_DBCU_MIF_OS_SMARTDMA_3_ADDR(base) ((base) + (0x0B48))
#define DPU_DBCU_MIF_STAT_SMARTDMA_3_ADDR(base) ((base) + (0x0B4C))
#define DPU_DBCU_MIF_CTRL_WCH0_ADDR(base) ((base) + (0x0B50))
#define DPU_DBCU_MIF_LEAD_CTRL_WCH0_ADDR(base) ((base) + (0x0B54))
#define DPU_DBCU_MIF_OS_CTRL_WCH0_ADDR(base) ((base) + (0x0B58))
#define DPU_DBCU_MIF_STAT_WCH0_ADDR(base) ((base) + (0x0B5C))
#define DPU_DBCU_MIF_CTRL_WCH1_ADDR(base) ((base) + (0x0B60))
#define DPU_DBCU_MIF_LEAD_CTRL_WCH1_ADDR(base) ((base) + (0x0B64))
#define DPU_DBCU_MIF_OS_CTRL_WCH1_ADDR(base) ((base) + (0x0B68))
#define DPU_DBCU_MIF_STAT_WCH1_ADDR(base) ((base) + (0x0B6C))
#define DPU_DBCU_MIF_CTRL_WCH2_ADDR(base) ((base) + (0x0B70))
#define DPU_DBCU_MIF_LEAD_CTRL_WCH2_ADDR(base) ((base) + (0x0B74))
#define DPU_DBCU_MIF_OS_CTRL_WCH2_ADDR(base) ((base) + (0x0B78))
#define DPU_DBCU_MIF_STAT_WCH2_ADDR(base) ((base) + (0x0B7C))
#define DPU_LBUF_CTL_CLK_SEL_ADDR(base) ((base) + (0x0000))
#define DPU_LBUF_CTL_CLK_EN_ADDR(base) ((base) + (0x0004))
#define DPU_LBUF_PART_CLK_SEL_ADDR(base) ((base) + (0x0008))
#define DPU_LBUF_PART_CLK_EN_ADDR(base) ((base) + (0x000C))
#define DPU_LBUF_MEM_CTRL_ADDR(base) ((base) + (0x0010))
#define DPU_LBUF_MEM_DSLP_CTRL_ADDR(base) ((base) + (0x0100))
#define DPU_LBUF_SOUR_AUTO_CTRL_ADDR(base) ((base) + (0x0104))
#define DPU_LBUF_DEBUG_CTRL_ADDR(base) ((base) + (0x0180))
#define DPU_LBUF_DBUF0_CTRL_ADDR(base) ((base) + (0x0200))
#define DPU_LBUF_DBUF0_STATE_ADDR(base) ((base) + (0x0204))
#define DPU_LBUF_DBUF0_THD_RQOS_ADDR(base) ((base) + (0x0210))
#define DPU_LBUF_DBUF0_THD_DFS_OK_ADDR(base) ((base) + (0x0220))
#define DPU_LBUF_DBUF0_THD_FLUX_REQ_BEF_ADDR(base) ((base) + (0x0230))
#define DPU_LBUF_DBUF0_THD_FLUX_REQ_AFT_ADDR(base) ((base) + (0x0234))
#define DPU_LBUF_DBUF0_ONLINE_FILL_LEVEL_ADDR(base) ((base) + (0x0240))
#define DPU_LBUF_DBUF0_THD_FILL_LEV0_ADDR(base) ((base) + (0x0250))
#define DPU_LBUF_DBUF0_PRS_SCN0_ADDR(base) ((base) + (0x0254))
#define DPU_LBUF_DBUF1_CTRL_ADDR(base) ((base) + (0x0300))
#define DPU_LBUF_DBUF1_STATE_ADDR(base) ((base) + (0x0304))
#define DPU_LBUF_DBUF1_THD_RQOS_ADDR(base) ((base) + (0x0310))
#define DPU_LBUF_DBUF1_THD_DFS_OK_ADDR(base) ((base) + (0x0320))
#define DPU_LBUF_DBUF1_THD_FLUX_REQ_BEF_ADDR(base) ((base) + (0x0330))
#define DPU_LBUF_DBUF1_THD_FLUX_REQ_AFT_ADDR(base) ((base) + (0x0334))
#define DPU_LBUF_DBUF1_ONLINE_FILL_LEVEL_ADDR(base) ((base) + (0x0340))
#define DPU_LBUF_DBUF1_THD_FILL_LEV0_ADDR(base) ((base) + (0x0350))
#define DPU_LBUF_DBUF1_PRS_SCN1_ADDR(base) ((base) + (0x0354))
#define DPU_LBUF_DBUF2_CTRL_ADDR(base) ((base) + (0x0400))
#define DPU_LBUF_DBUF2_STATE_ADDR(base) ((base) + (0x0404))
#define DPU_LBUF_DBUF2_THD_RQOS_ADDR(base) ((base) + (0x0410))
#define DPU_LBUF_DBUF2_ONLINE_FILL_LEVEL_ADDR(base) ((base) + (0x0440))
#define DPU_LBUF_DBUF2_THD_FILL_LEV0_ADDR(base) ((base) + (0x0450))
#define DPU_LBUF_DBUF2_PRS_SCN2_ADDR(base) ((base) + (0x0454))
#define DPU_LBUF_DBUF3_CTRL_ADDR(base) ((base) + (0x0500))
#define DPU_LBUF_DBUF3_STATE_ADDR(base) ((base) + (0x0504))
#define DPU_LBUF_DBUF3_THD_RQOS_ADDR(base) ((base) + (0x0510))
#define DPU_LBUF_DBUF3_ONLINE_FILL_LEVEL_ADDR(base) ((base) + (0x0540))
#define DPU_LBUF_DBUF3_THD_FILL_LEV0_ADDR(base) ((base) + (0x0550))
#define DPU_LBUF_DBUF3_PRS_SCN3_ADDR(base) ((base) + (0x0554))
#define DPU_LBUF_SCN0_FLUX_CTRL_ADDR(base) ((base) + (0x0600))
#define DPU_LBUF_SCN0_FLUX_TH_ADDR(base) ((base) + (0x0604))
#define DPU_LBUF_SCN1_FLUX_CTRL_ADDR(base) ((base) + (0x0610))
#define DPU_LBUF_SCN1_FLUX_TH_ADDR(base) ((base) + (0x0614))
#define DPU_LBUF_SCN2_FLUX_CTRL_ADDR(base) ((base) + (0x0620))
#define DPU_LBUF_SCN2_FLUX_TH_ADDR(base) ((base) + (0x0624))
#define DPU_LBUF_SCN3_FLUX_CTRL_ADDR(base) ((base) + (0x0630))
#define DPU_LBUF_SCN3_FLUX_TH_ADDR(base) ((base) + (0x0634))
#define DPU_LBUF_SCN4_FLUX_CTRL_ADDR(base) ((base) + (0x0640))
#define DPU_LBUF_SCN4_FLUX_TH_ADDR(base) ((base) + (0x0644))
#define DPU_LBUF_SCN5_FLUX_CTRL_ADDR(base) ((base) + (0x0650))
#define DPU_LBUF_SCN5_FLUX_TH_ADDR(base) ((base) + (0x0654))
#define DPU_LBUF_SCN6_FLUX_CTRL_ADDR(base) ((base) + (0x0660))
#define DPU_LBUF_SCN6_FLUX_TH_ADDR(base) ((base) + (0x0664))
#define DPU_LBUF_DCPT_STATE_ADDR(base) ((base) + (0x0800))
#define DPU_LBUF_DCPT_DMA_STATE_ADDR(base) ((base) + (0x0804))
#define DPU_LBUF_CTL_STATE_ADDR(base) ((base) + (0x0808))
#define DPU_LBUF_RF_STATE_ADDR(base) ((base) + (0x0810))
#define DPU_LBUF_PRE_USE_STATE_ADDR(base) ((base) + (0x0820))
#define DPU_LBUF_PPST_USE_STATE_ADDR(base) ((base) + (0x0824))
#define DPU_LBUF_PRE_CHECK_STATE_ADDR(base) ((base) + (0x0828))
#define DPU_LBUF_PPST_CHECK_STATE_ADDR(base) ((base) + (0x082C))
#define DPU_LBUF_SCN_STATE_ADDR(base) ((base) + (0x0830))
#define DPU_LBUF_SCN_DMA_STATE_ADDR(base) ((base) + (0x0834))
#define DPU_LBUF_RUN_STATE_ADDR(base) ((base) + (0x0840))
#define DPU_LBUF_RUN_MST_STATE_ADDR(base) ((base) + (0x0844))
#define DPU_LBUF_RUN_CTL_STATE_ADDR(base) ((base) + (0x0848))
#define DPU_LBUF_RUN_WR_STATE_ADDR(base) ((base) + (0x0850))
#define DPU_LBUF_RUN_RD_STATE_ADDR(base) ((base) + (0x0854))
#define DPU_LBUF_RUN_RD2_STATE_ADDR(base) ((base) + (0x0858))
#define DPU_LBUF_PART0_RMST_PASS_ADDR(base) ((base) + (0x0900))
#define DPU_LBUF_PART0_WMST_PASS_ADDR(base) ((base) + (0x0904))
#define DPU_LBUF_PART0_IDX_DATA_ADDR(base) ((base) + (0x0908))
#define DPU_LBUF_PART0_RUN_DAT_ADDR(base) ((base) + (0x090C))
#define DPU_LBUF_PART1_RMST_PASS_ADDR(base) ((base) + (0x0940))
#define DPU_LBUF_PART1_WMST_PASS_ADDR(base) ((base) + (0x0944))
#define DPU_LBUF_PART1_IDX_DATA_ADDR(base) ((base) + (0x0948))
#define DPU_LBUF_PART1_RUN_DATA_ADDR(base) ((base) + (0x094C))
#define DPU_LBUF_PART2_RMST_PASS_ADDR(base) ((base) + (0x0980))
#define DPU_LBUF_PART2_WMST_PASS_ADDR(base) ((base) + (0x0984))
#define DPU_LBUF_PART2_IDX_DATA_ADDR(base) ((base) + (0x0988))
#define DPU_LBUF_PART2_RUN_DATA_ADDR(base) ((base) + (0x098C))
#define DPU_LBUF_DBG_CTRL0_ADDR(base) ((base) + (0x09A0))
#define DPU_LBUF_DBG_STATE0_ADDR(base) ((base) + (0x09A4))
#define DPU_LBUF_DBG_STATE1_ADDR(base) ((base) + (0x09A8))
#define DPU_ARSR_RD_SHADOW_ADDR(base) ((base) + (0x0000))
#define DPU_ARSR_REG_DEFAULT_ADDR(base) ((base) + (0x0004))
#define DPU_ARSR_TOP_CLK_SEL_ADDR(base) ((base) + (0x0008))
#define DPU_ARSR_TOP_CLK_EN_ADDR(base) ((base) + (0x000C))
#define DPU_ARSR_REG_CTRL_ADDR(base) ((base) + (0x0010))
#define DPU_ARSR_REG_CTRL_FLUSH_EN_ADDR(base) ((base) + (0x0014))
#define DPU_ARSR_REG_CTRL_DEBUG_ADDR(base) ((base) + (0x0018))
#define DPU_ARSR_REG_CTRL_STATE_ADDR(base) ((base) + (0x001C))
#define DPU_ARSR_R_LB_DEBUG0_ADDR(base) ((base) + (0x0030))
#define DPU_ARSR_R_LB_DEBUG1_ADDR(base) ((base) + (0x0034))
#define DPU_ARSR_R_LB_DEBUG2_ADDR(base) ((base) + (0x0038))
#define DPU_ARSR_R_LB_DEBUG3_ADDR(base) ((base) + (0x003C))
#define DPU_ARSR_W_LB_DEBUG0_ADDR(base) ((base) + (0x0040))
#define DPU_ARSR_W_LB_DEBUG1_ADDR(base) ((base) + (0x0044))
#define DPU_ARSR_W_LB_DEBUG2_ADDR(base) ((base) + (0x0048))
#define DPU_ARSR_W_LB_DEBUG3_ADDR(base) ((base) + (0x004C))
#define DPU_ARSR_REG_CTRL_LAYER_ID_ADDR(base) ((base) + (0x0050))
#define DPU_ARSR_DFC_DISP_SIZE_ADDR(base) ((base) + (0x0100))
#define DPU_ARSR_DFC_PIX_IN_NUM_ADDR(base) ((base) + (0x0104))
#define DPU_ARSR_DFC_GLB_ALPHA01_ADDR(base) ((base) + (0x0108))
#define DPU_ARSR_DFC_DISP_FMT_ADDR(base) ((base) + (0x010C))
#define DPU_ARSR_DFC_CLIP_CTL_HRZ_ADDR(base) ((base) + (0x0110))
#define DPU_ARSR_DFC_CLIP_CTL_VRZ_ADDR(base) ((base) + (0x0114))
#define DPU_ARSR_DFC_CTL_CLIP_EN_ADDR(base) ((base) + (0x0118))
#define DPU_ARSR_DFC_ICG_MODULE_ADDR(base) ((base) + (0x011C))
#define DPU_ARSR_DFC_DITHER_ENABLE_ADDR(base) ((base) + (0x0120))
#define DPU_ARSR_DFC_PADDING_CTL_ADDR(base) ((base) + (0x0124))
#define DPU_ARSR_DFC_GLB_ALPHA23_ADDR(base) ((base) + (0x0128))
#define DPU_ARSR_BITEXT_CTL_ADDR(base) ((base) + (0x0140))
#define DPU_ARSR_BITEXT_REG_INI0_0_ADDR(base) ((base) + (0x0144))
#define DPU_ARSR_BITEXT_REG_INI0_1_ADDR(base) ((base) + (0x0148))
#define DPU_ARSR_BITEXT_REG_INI0_2_ADDR(base) ((base) + (0x014C))
#define DPU_ARSR_BITEXT_REG_INI0_3_ADDR(base) ((base) + (0x0150))
#define DPU_ARSR_BITEXT_REG_INI1_0_ADDR(base) ((base) + (0x0154))
#define DPU_ARSR_BITEXT_REG_INI1_1_ADDR(base) ((base) + (0x0158))
#define DPU_ARSR_BITEXT_REG_INI1_2_ADDR(base) ((base) + (0x015C))
#define DPU_ARSR_BITEXT_REG_INI1_3_ADDR(base) ((base) + (0x0160))
#define DPU_ARSR_BITEXT_REG_INI2_0_ADDR(base) ((base) + (0x0164))
#define DPU_ARSR_BITEXT_REG_INI2_1_ADDR(base) ((base) + (0x0168))
#define DPU_ARSR_BITEXT_REG_INI2_2_ADDR(base) ((base) + (0x016C))
#define DPU_ARSR_BITEXT_REG_INI2_3_ADDR(base) ((base) + (0x0170))
#define DPU_ARSR_BITEXT_POWER_CTRL_C_ADDR(base) ((base) + (0x0174))
#define DPU_ARSR_BITEXT_POWER_CTRL_SHIFT_ADDR(base) ((base) + (0x0178))
#define DPU_ARSR_BITEXT_FILT_00_ADDR(base) ((base) + (0x017C))
#define DPU_ARSR_BITEXT_FILT_01_ADDR(base) ((base) + (0x0180))
#define DPU_ARSR_BITEXT_FILT_02_ADDR(base) ((base) + (0x0184))
#define DPU_ARSR_BITEXT_FILT_10_ADDR(base) ((base) + (0x0188))
#define DPU_ARSR_BITEXT_FILT_11_ADDR(base) ((base) + (0x018C))
#define DPU_ARSR_BITEXT_FILT_12_ADDR(base) ((base) + (0x0190))
#define DPU_ARSR_BITEXT_FILT_20_ADDR(base) ((base) + (0x0194))
#define DPU_ARSR_BITEXT_FILT_21_ADDR(base) ((base) + (0x01A0))
#define DPU_ARSR_BITEXT_FILT_22_ADDR(base) ((base) + (0x01A4))
#define DPU_ARSR_BITEXT_THD_R0_ADDR(base) ((base) + (0x01A8))
#define DPU_ARSR_BITEXT_THD_R1_ADDR(base) ((base) + (0x01AC))
#define DPU_ARSR_BITEXT_THD_G0_ADDR(base) ((base) + (0x01B0))
#define DPU_ARSR_BITEXT_THD_G1_ADDR(base) ((base) + (0x01B4))
#define DPU_ARSR_BITEXT_THD_B0_ADDR(base) ((base) + (0x01B8))
#define DPU_ARSR_BITEXT_THD_B1_ADDR(base) ((base) + (0x01BC))
#define DPU_ARSR_BITEXT0_DBG0_ADDR(base) ((base) + (0x01C0))
#define DPU_ARSR_CSC_IDC0_ADDR(base) ((base) + (0x01C4))
#define DPU_ARSR_CSC_IDC2_ADDR(base) ((base) + (0x01C8))
#define DPU_ARSR_CSC_ODC0_ADDR(base) ((base) + (0x01CC))
#define DPU_ARSR_CSC_ODC2_ADDR(base) ((base) + (0x01D0))
#define DPU_ARSR_CSC_P00_ADDR(base) ((base) + (0x01D4))
#define DPU_ARSR_CSC_P01_ADDR(base) ((base) + (0x01D8))
#define DPU_ARSR_CSC_P02_ADDR(base) ((base) + (0x01DC))
#define DPU_ARSR_CSC_P10_ADDR(base) ((base) + (0x01E0))
#define DPU_ARSR_CSC_P11_ADDR(base) ((base) + (0x01E4))
#define DPU_ARSR_CSC_P12_ADDR(base) ((base) + (0x01E8))
#define DPU_ARSR_CSC_P20_ADDR(base) ((base) + (0x01EC))
#define DPU_ARSR_CSC_P21_ADDR(base) ((base) + (0x01F0))
#define DPU_ARSR_CSC_P22_ADDR(base) ((base) + (0x01F4))
#define DPU_ARSR_CSC_ICG_MODULE_ADDR(base) ((base) + (0x01F8))
#define DPU_ARSR_CSC_MPREC_ADDR(base) ((base) + (0x01FC))
#define DPU_ARSR_SCF_EN_HSCL_STR_ADDR(base) ((base) + (0x0200))
#define DPU_ARSR_SCF_EN_VSCL_STR_ADDR(base) ((base) + (0x0204))
#define DPU_ARSR_SCF_H_V_ORDER_ADDR(base) ((base) + (0x0208))
#define DPU_ARSR_SCF_CH_CORE_GT_ADDR(base) ((base) + (0x020C))
#define DPU_ARSR_SCF_INPUT_WIDTH_HEIGHT_ADDR(base) ((base) + (0x0210))
#define DPU_ARSR_SCF_OUTPUT_WIDTH_HEIGHT_ADDR(base) ((base) + (0x0214))
#define DPU_ARSR_SCF_COEF_MEM_CTRL_ADDR(base) ((base) + (0x0218))
#define DPU_ARSR_SCF_EN_HSCL_ADDR(base) ((base) + (0x021C))
#define DPU_ARSR_SCF_EN_VSCL_ADDR(base) ((base) + (0x0220))
#define DPU_ARSR_SCF_ACC_HSCL_ADDR(base) ((base) + (0x0224))
#define DPU_ARSR_SCF_ACC_HSCL1_ADDR(base) ((base) + (0x0228))
#define DPU_ARSR_SCF_INC_HSCL_ADDR(base) ((base) + (0x0234))
#define DPU_ARSR_SCF_ACC_VSCL_ADDR(base) ((base) + (0x0238))
#define DPU_ARSR_SCF_ACC_VSCL1_ADDR(base) ((base) + (0x023C))
#define DPU_ARSR_SCF_INC_VSCL_ADDR(base) ((base) + (0x0248))
#define DPU_ARSR_SCF_EN_NONLINEAR_ADDR(base) ((base) + (0x024C))
#define DPU_ARSR_SCF_EN_MMP_ADDR(base) ((base) + (0x027C))
#define DPU_ARSR_SCF_DB_H0_ADDR(base) ((base) + (0x0280))
#define DPU_ARSR_SCF_DB_H1_ADDR(base) ((base) + (0x0284))
#define DPU_ARSR_SCF_DB_V0_ADDR(base) ((base) + (0x0288))
#define DPU_ARSR_SCF_DB_V1_ADDR(base) ((base) + (0x028C))
#define DPU_ARSR_SCF_LB_MEM_CTRL_ADDR(base) ((base) + (0x0290))
#define DPU_ARSR_INPUT_WIDTH_HEIGHT_ADDR(base) ((base) + (0x0300))
#define DPU_ARSR_OUTPUT_WIDTH_HEIGHT_ADDR(base) ((base) + (0x0304))
#define DPU_ARSR_IHLEFT_ADDR(base) ((base) + (0x0308))
#define DPU_ARSR_IHLEFT1_ADDR(base) ((base) + (0x030C))
#define DPU_ARSR_IHRIGHT_ADDR(base) ((base) + (0x0310))
#define DPU_ARSR_IHRIGHT1_ADDR(base) ((base) + (0x0314))
#define DPU_ARSR_IVTOP_ADDR(base) ((base) + (0x0318))
#define DPU_ARSR_IVBOTTOM_ADDR(base) ((base) + (0x031C))
#define DPU_ARSR_IVBOTTOM1_ADDR(base) ((base) + (0x0320))
#define DPU_ARSR_IHINC_ADDR(base) ((base) + (0x0324))
#define DPU_ARSR_IVINC_ADDR(base) ((base) + (0x0328))
#define DPU_ARSR_OFFSET_ADDR(base) ((base) + (0x032C))
#define DPU_ARSR_MODE_ADDR(base) ((base) + (0x0330))
#define DPU_ARSR_SKIN_THRES_Y_ADDR(base) ((base) + (0x0334))
#define DPU_ARSR_SKIN_THRES_U_ADDR(base) ((base) + (0x0338))
#define DPU_ARSR_SKIN_THRES_V_ADDR(base) ((base) + (0x033C))
#define DPU_ARSR_SKIN_CFG0_ADDR(base) ((base) + (0x0340))
#define DPU_ARSR_SKIN_CFG1_ADDR(base) ((base) + (0x0344))
#define DPU_ARSR_SKIN_CFG2_ADDR(base) ((base) + (0x0348))
#define DPU_ARSR_SHOOT_CFG1_ADDR(base) ((base) + (0x034C))
#define DPU_ARSR_SHOOT_CFG2_ADDR(base) ((base) + (0x0350))
#define DPU_ARSR_SHOOT_CFG3_ADDR(base) ((base) + (0x0354))
#define DPU_ARSR_SHARP_CFG3_ADDR(base) ((base) + (0x0358))
#define DPU_ARSR_SHARP_CFG4_ADDR(base) ((base) + (0x035C))
#define DPU_ARSR_SHARP_CFG5_ADDR(base) ((base) + (0x0360))
#define DPU_ARSR_SHARP_CFG6_ADDR(base) ((base) + (0x0364))
#define DPU_ARSR_SHARP_CFG7_ADDR(base) ((base) + (0x0368))
#define DPU_ARSR_SHARP_CFG8_ADDR(base) ((base) + (0x036C))
#define DPU_ARSR_SHARPLEVEL_ADDR(base) ((base) + (0x0370))
#define DPU_ARSR_SHPGAIN_LOW_ADDR(base) ((base) + (0x0374))
#define DPU_ARSR_SHPGAIN_MID_ADDR(base) ((base) + (0x0378))
#define DPU_ARSR_SHPGAIN_HIGH_ADDR(base) ((base) + (0x037C))
#define DPU_ARSR_GAINCTRLSLOPH_MF_ADDR(base) ((base) + (0x0380))
#define DPU_ARSR_GAINCTRLSLOPL_MF_ADDR(base) ((base) + (0x0384))
#define DPU_ARSR_GAINCTRLSLOPH_HF_ADDR(base) ((base) + (0x0388))
#define DPU_ARSR_GAINCTRLSLOPL_HF_ADDR(base) ((base) + (0x038C))
#define DPU_ARSR_MF_LMT_ADDR(base) ((base) + (0x0390))
#define DPU_ARSR_GAIN_MF_ADDR(base) ((base) + (0x0394))
#define DPU_ARSR_MF_B_ADDR(base) ((base) + (0x0398))
#define DPU_ARSR_HF_LMT_ADDR(base) ((base) + (0x039C))
#define DPU_ARSR_GAIN_HF_ADDR(base) ((base) + (0x03A0))
#define DPU_ARSR_HF_B_ADDR(base) ((base) + (0x03A4))
#define DPU_ARSR_LF_CTRL_ADDR(base) ((base) + (0x03A8))
#define DPU_ARSR_LF_VAR_ADDR(base) ((base) + (0x03AC))
#define DPU_ARSR_LF_CTRL_SLOP_ADDR(base) ((base) + (0x03B0))
#define DPU_ARSR_HF_SELECT_ADDR(base) ((base) + (0x03B4))
#define DPU_ARSR_SHARP_CFG2_H_ADDR(base) ((base) + (0x03B8))
#define DPU_ARSR_SHARP_CFG2_L_ADDR(base) ((base) + (0x03BC))
#define DPU_ARSR_TEXTURW_ANALYSTS_ADDR(base) ((base) + (0x03D0))
#define DPU_ARSR_INTPLSHOOTCTRL_ADDR(base) ((base) + (0x03D4))
#define DPU_ARSR_ARSR2P_DEBUG0_ADDR(base) ((base) + (0x03D8))
#define DPU_ARSR_ARSR2P_DEBUG1_ADDR(base) ((base) + (0x03DC))
#define DPU_ARSR_ARSR2P_DEBUG2_ADDR(base) ((base) + (0x03E0))
#define DPU_ARSR_ARSR2P_DEBUG3_ADDR(base) ((base) + (0x03E4))
#define DPU_ARSR_ARSR2P_LB_MEM_CTRL_ADDR(base) ((base) + (0x03E8))
#define DPU_ARSR_COEF_MEM_CTRL_ADDR(base) ((base) + (0x03EC))
#define DPU_ARSR_POST_CSC_IDC0_ADDR(base) ((base) + (0x0400))
#define DPU_ARSR_POST_CSC_IDC2_ADDR(base) ((base) + (0x0404))
#define DPU_ARSR_POST_CSC_ODC0_ADDR(base) ((base) + (0x0408))
#define DPU_ARSR_POST_CSC_ODC2_ADDR(base) ((base) + (0x040C))
#define DPU_ARSR_POST_CSC_P00_ADDR(base) ((base) + (0x0410))
#define DPU_ARSR_POST_CSC_P01_ADDR(base) ((base) + (0x0414))
#define DPU_ARSR_POST_CSC_P02_ADDR(base) ((base) + (0x0418))
#define DPU_ARSR_POST_CSC_P10_ADDR(base) ((base) + (0x041C))
#define DPU_ARSR_POST_CSC_P11_ADDR(base) ((base) + (0x0420))
#define DPU_ARSR_POST_CSC_P12_ADDR(base) ((base) + (0x0424))
#define DPU_ARSR_POST_CSC_P20_ADDR(base) ((base) + (0x0428))
#define DPU_ARSR_POST_CSC_P21_ADDR(base) ((base) + (0x042C))
#define DPU_ARSR_POST_CSC_P22_ADDR(base) ((base) + (0x0430))
#define DPU_ARSR_POST_CSC_MODULE_EN_ADDR(base) ((base) + (0x0434))
#define DPU_ARSR_POST_CSC_MPREC_ADDR(base) ((base) + (0x0438))
#define DPU_ARSR_POST_CLIP_DISP_SIZE_ADDR(base) ((base) + (0x0480))
#define DPU_ARSR_POST_CLIP_CTL_HRZ_ADDR(base) ((base) + (0x0484))
#define DPU_ARSR_POST_CLIP_CTL_VRZ_ADDR(base) ((base) + (0x0488))
#define DPU_ARSR_POST_CLIP_EN_ADDR(base) ((base) + (0x048C))
#define DPU_ARSR_V0_SCF_VIDEO_6TAP_COEF_ADDR(base,l) ((base) + (0x1000+0x4*(l)))
#define DPU_ARSR_V0_SCF_VIDEO_5TAP_COEF_ADDR(base,m) ((base) + (0x3000+0x4*(m)))
#define DPU_ARSR_V0_SCF_VIDEO_4TAP_COEF_ADDR(base,n) ((base) + (0x3800+0x4*(n)))
#define DPU_ARSR_COEFY_V_ADDR(base,o) ((base) + (0x5000+0x4*(o)))
#define DPU_ARSR_COEFY_H_ADDR(base,p) ((base) + (0x5100+0x4*(p)))
#define DPU_ARSR_COEFA_V_ADDR(base,o) ((base) + (0x5300+0x4*(o)))
#define DPU_ARSR_COEFA_H_ADDR(base,p) ((base) + (0x5400+0x4*(p)))
#define DPU_ARSR_COEFUV_V_ADDR(base,o) ((base) + (0x5600+0x4*(o)))
#define DPU_ARSR_COEFUV_H_ADDR(base,p) ((base) + (0x5700+0x4*(p)))
#define DPU_VSCF_RD_SHADOW_ADDR(base) ((base) + (0x0000))
#define DPU_VSCF_REG_DEFAULT_ADDR(base) ((base) + (0x0004))
#define DPU_VSCF_TOP_CLK_SEL_ADDR(base) ((base) + (0x0008))
#define DPU_VSCF_TOP_CLK_EN_ADDR(base) ((base) + (0x000C))
#define DPU_VSCF_REG_CTRL_ADDR(base) ((base) + (0x0010))
#define DPU_VSCF_REG_CTRL_FLUSH_EN_ADDR(base) ((base) + (0x0014))
#define DPU_VSCF_REG_CTRL_DEBUG_ADDR(base) ((base) + (0x0018))
#define DPU_VSCF_REG_CTRL_STATE_ADDR(base) ((base) + (0x001C))
#define DPU_VSCF_R_LB_DEBUG0_ADDR(base) ((base) + (0x0030))
#define DPU_VSCF_R_LB_DEBUG1_ADDR(base) ((base) + (0x0034))
#define DPU_VSCF_R_LB_DEBUG2_ADDR(base) ((base) + (0x0038))
#define DPU_VSCF_R_LB_DEBUG3_ADDR(base) ((base) + (0x003C))
#define DPU_VSCF_W_LB_DEBUG0_ADDR(base) ((base) + (0x0040))
#define DPU_VSCF_W_LB_DEBUG1_ADDR(base) ((base) + (0x0044))
#define DPU_VSCF_W_LB_DEBUG2_ADDR(base) ((base) + (0x0048))
#define DPU_VSCF_W_LB_DEBUG3_ADDR(base) ((base) + (0x004C))
#define DPU_VSCF_REG_CTRL_LAYER_ID_ADDR(base) ((base) + (0x0050))
#define DPU_VSCF_DFC_DISP_SIZE_ADDR(base) ((base) + (0x0100))
#define DPU_VSCF_DFC_PIX_IN_NUM_ADDR(base) ((base) + (0x0104))
#define DPU_VSCF_DFC_GLB_ALPHA01_ADDR(base) ((base) + (0x0108))
#define DPU_VSCF_DFC_DISP_FMT_ADDR(base) ((base) + (0x010C))
#define DPU_VSCF_DFC_CLIP_CTL_HRZ_ADDR(base) ((base) + (0x0110))
#define DPU_VSCF_DFC_CLIP_CTL_VRZ_ADDR(base) ((base) + (0x0114))
#define DPU_VSCF_DFC_CTL_CLIP_EN_ADDR(base) ((base) + (0x0118))
#define DPU_VSCF_DFC_ICG_MODULE_ADDR(base) ((base) + (0x011C))
#define DPU_VSCF_DFC_DITHER_ENABLE_ADDR(base) ((base) + (0x0120))
#define DPU_VSCF_DFC_PADDING_CTL_ADDR(base) ((base) + (0x0124))
#define DPU_VSCF_DFC_GLB_ALPHA23_ADDR(base) ((base) + (0x0128))
#define DPU_VSCF_BITEXT_CTL_ADDR(base) ((base) + (0x0140))
#define DPU_VSCF_BITEXT_REG_INI0_0_ADDR(base) ((base) + (0x0144))
#define DPU_VSCF_BITEXT_REG_INI0_1_ADDR(base) ((base) + (0x0148))
#define DPU_VSCF_BITEXT_REG_INI0_2_ADDR(base) ((base) + (0x014C))
#define DPU_VSCF_BITEXT_REG_INI0_3_ADDR(base) ((base) + (0x0150))
#define DPU_VSCF_BITEXT_REG_INI1_0_ADDR(base) ((base) + (0x0154))
#define DPU_VSCF_BITEXT_REG_INI1_1_ADDR(base) ((base) + (0x0158))
#define DPU_VSCF_BITEXT_REG_INI1_2_ADDR(base) ((base) + (0x015C))
#define DPU_VSCF_BITEXT_REG_INI1_3_ADDR(base) ((base) + (0x0160))
#define DPU_VSCF_BITEXT_REG_INI2_0_ADDR(base) ((base) + (0x0164))
#define DPU_VSCF_BITEXT_REG_INI2_1_ADDR(base) ((base) + (0x0168))
#define DPU_VSCF_BITEXT_REG_INI2_2_ADDR(base) ((base) + (0x016C))
#define DPU_VSCF_BITEXT_REG_INI2_3_ADDR(base) ((base) + (0x0170))
#define DPU_VSCF_BITEXT_POWER_CTRL_C_ADDR(base) ((base) + (0x0174))
#define DPU_VSCF_BITEXT_POWER_CTRL_SHIFT_ADDR(base) ((base) + (0x0178))
#define DPU_VSCF_BITEXT_FILT_00_ADDR(base) ((base) + (0x017C))
#define DPU_VSCF_BITEXT_FILT_01_ADDR(base) ((base) + (0x0180))
#define DPU_VSCF_BITEXT_FILT_02_ADDR(base) ((base) + (0x0184))
#define DPU_VSCF_BITEXT_FILT_10_ADDR(base) ((base) + (0x0188))
#define DPU_VSCF_BITEXT_FILT_11_ADDR(base) ((base) + (0x018C))
#define DPU_VSCF_BITEXT_FILT_12_ADDR(base) ((base) + (0x0190))
#define DPU_VSCF_BITEXT_FILT_20_ADDR(base) ((base) + (0x0194))
#define DPU_VSCF_BITEXT_FILT_21_ADDR(base) ((base) + (0x01A0))
#define DPU_VSCF_BITEXT_FILT_22_ADDR(base) ((base) + (0x01A4))
#define DPU_VSCF_BITEXT_THD_R0_ADDR(base) ((base) + (0x01A8))
#define DPU_VSCF_BITEXT_THD_R1_ADDR(base) ((base) + (0x01AC))
#define DPU_VSCF_BITEXT_THD_G0_ADDR(base) ((base) + (0x01B0))
#define DPU_VSCF_BITEXT_THD_G1_ADDR(base) ((base) + (0x01B4))
#define DPU_VSCF_BITEXT_THD_B0_ADDR(base) ((base) + (0x01B8))
#define DPU_VSCF_BITEXT_THD_B1_ADDR(base) ((base) + (0x01BC))
#define DPU_VSCF_BITEXT0_DBG0_ADDR(base) ((base) + (0x01C0))
#define DPU_VSCF_CSC_IDC0_ADDR(base) ((base) + (0x01C4))
#define DPU_VSCF_CSC_IDC2_ADDR(base) ((base) + (0x01C8))
#define DPU_VSCF_CSC_ODC0_ADDR(base) ((base) + (0x01CC))
#define DPU_VSCF_CSC_ODC2_ADDR(base) ((base) + (0x01D0))
#define DPU_VSCF_CSC_P00_ADDR(base) ((base) + (0x01D4))
#define DPU_VSCF_CSC_P01_ADDR(base) ((base) + (0x01D8))
#define DPU_VSCF_CSC_P02_ADDR(base) ((base) + (0x01DC))
#define DPU_VSCF_CSC_P10_ADDR(base) ((base) + (0x01E0))
#define DPU_VSCF_CSC_P11_ADDR(base) ((base) + (0x01E4))
#define DPU_VSCF_CSC_P12_ADDR(base) ((base) + (0x01E8))
#define DPU_VSCF_CSC_P20_ADDR(base) ((base) + (0x01EC))
#define DPU_VSCF_CSC_P21_ADDR(base) ((base) + (0x01F0))
#define DPU_VSCF_CSC_P22_ADDR(base) ((base) + (0x01F4))
#define DPU_VSCF_CSC_ICG_MODULE_ADDR(base) ((base) + (0x01F8))
#define DPU_VSCF_CSC_MPREC_ADDR(base) ((base) + (0x01FC))
#define DPU_VSCF_SCF_EN_HSCL_STR_ADDR(base) ((base) + (0x0200))
#define DPU_VSCF_SCF_EN_VSCL_STR_ADDR(base) ((base) + (0x0204))
#define DPU_VSCF_SCF_H_V_ORDER_ADDR(base) ((base) + (0x0208))
#define DPU_VSCF_SCF_CH_CORE_GT_ADDR(base) ((base) + (0x020C))
#define DPU_VSCF_SCF_INPUT_WIDTH_HEIGHT_ADDR(base) ((base) + (0x0210))
#define DPU_VSCF_SCF_OUTPUT_WIDTH_HEIGHT_ADDR(base) ((base) + (0x0214))
#define DPU_VSCF_SCF_COEF_MEM_CTRL_ADDR(base) ((base) + (0x0218))
#define DPU_VSCF_SCF_EN_HSCL_ADDR(base) ((base) + (0x021C))
#define DPU_VSCF_SCF_EN_VSCL_ADDR(base) ((base) + (0x0220))
#define DPU_VSCF_SCF_ACC_HSCL_ADDR(base) ((base) + (0x0224))
#define DPU_VSCF_SCF_ACC_HSCL1_ADDR(base) ((base) + (0x0228))
#define DPU_VSCF_SCF_INC_HSCL_ADDR(base) ((base) + (0x0234))
#define DPU_VSCF_SCF_ACC_VSCL_ADDR(base) ((base) + (0x0238))
#define DPU_VSCF_SCF_ACC_VSCL1_ADDR(base) ((base) + (0x023C))
#define DPU_VSCF_SCF_INC_VSCL_ADDR(base) ((base) + (0x0248))
#define DPU_VSCF_SCF_EN_NONLINEAR_ADDR(base) ((base) + (0x024C))
#define DPU_VSCF_SCF_EN_MMP_ADDR(base) ((base) + (0x027C))
#define DPU_VSCF_SCF_DB_H0_ADDR(base) ((base) + (0x0280))
#define DPU_VSCF_SCF_DB_H1_ADDR(base) ((base) + (0x0284))
#define DPU_VSCF_SCF_DB_V0_ADDR(base) ((base) + (0x0288))
#define DPU_VSCF_SCF_DB_V1_ADDR(base) ((base) + (0x028C))
#define DPU_VSCF_SCF_LB_MEM_CTRL_ADDR(base) ((base) + (0x0290))
#define DPU_VSCF_POST_CLIP_DISP_SIZE_ADDR(base) ((base) + (0x0480))
#define DPU_VSCF_POST_CLIP_CTL_HRZ_ADDR(base) ((base) + (0x0484))
#define DPU_VSCF_POST_CLIP_CTL_VRZ_ADDR(base) ((base) + (0x0488))
#define DPU_VSCF_POST_CLIP_EN_ADDR(base) ((base) + (0x048C))
#define DPU_VSCF_V0_SCF_VIDEO_6TAP_COEF_ADDR(base,l) ((base) + (0x1000+0x4*(l)))
#define DPU_VSCF_V0_SCF_VIDEO_5TAP_COEF_ADDR(base,m) ((base) + (0x3000+0x4*(m)))
#define DPU_VSCF_V0_SCF_VIDEO_4TAP_COEF_ADDR(base,n) ((base) + (0x3800+0x4*(n)))
#define DPU_HDR_RD_SHADOW_ADDR(base) ((base) + (0x0000))
#define DPU_HDR_REG_DEFAULT_ADDR(base) ((base) + (0x0004))
#define DPU_HDR_TOP_CLK_SEL_ADDR(base) ((base) + (0x0008))
#define DPU_HDR_TOP_CLK_EN_ADDR(base) ((base) + (0x000C))
#define DPU_HDR_REG_CTRL_ADDR(base) ((base) + (0x0010))
#define DPU_HDR_REG_CTRL_FLUSH_EN_ADDR(base) ((base) + (0x0014))
#define DPU_HDR_REG_CTRL_DEBUG_ADDR(base) ((base) + (0x0018))
#define DPU_HDR_REG_CTRL_STATE_ADDR(base) ((base) + (0x001C))
#define DPU_HDR_R_LB_DEBUG0_ADDR(base) ((base) + (0x0030))
#define DPU_HDR_R_LB_DEBUG1_ADDR(base) ((base) + (0x0034))
#define DPU_HDR_R_LB_DEBUG2_ADDR(base) ((base) + (0x0038))
#define DPU_HDR_R_LB_DEBUG3_ADDR(base) ((base) + (0x003C))
#define DPU_HDR_W_LB_DEBUG0_ADDR(base) ((base) + (0x0040))
#define DPU_HDR_W_LB_DEBUG1_ADDR(base) ((base) + (0x0044))
#define DPU_HDR_W_LB_DEBUG2_ADDR(base) ((base) + (0x0048))
#define DPU_HDR_W_LB_DEBUG3_ADDR(base) ((base) + (0x004C))
#define DPU_HDR_REG_CTRL_LAYER_ID_ADDR(base) ((base) + (0x0050))
#define DPU_HDR_DFC_DISP_SIZE_ADDR(base) ((base) + (0x0100))
#define DPU_HDR_DFC_PIX_IN_NUM_ADDR(base) ((base) + (0x0104))
#define DPU_HDR_DFC_GLB_ALPHA01_ADDR(base) ((base) + (0x0108))
#define DPU_HDR_DFC_DISP_FMT_ADDR(base) ((base) + (0x010C))
#define DPU_HDR_DFC_CLIP_CTL_HRZ_ADDR(base) ((base) + (0x0110))
#define DPU_HDR_DFC_CLIP_CTL_VRZ_ADDR(base) ((base) + (0x0114))
#define DPU_HDR_DFC_CTL_CLIP_EN_ADDR(base) ((base) + (0x0118))
#define DPU_HDR_DFC_ICG_MODULE_ADDR(base) ((base) + (0x011C))
#define DPU_HDR_DFC_DITHER_ENABLE_ADDR(base) ((base) + (0x0120))
#define DPU_HDR_DFC_PADDING_CTL_ADDR(base) ((base) + (0x0124))
#define DPU_HDR_DFC_GLB_ALPHA23_ADDR(base) ((base) + (0x0128))
#define DPU_HDR_BITEXT_CTL_ADDR(base) ((base) + (0x0140))
#define DPU_HDR_BITEXT_REG_INI0_0_ADDR(base) ((base) + (0x0144))
#define DPU_HDR_BITEXT_REG_INI0_1_ADDR(base) ((base) + (0x0148))
#define DPU_HDR_BITEXT_REG_INI0_2_ADDR(base) ((base) + (0x014C))
#define DPU_HDR_BITEXT_REG_INI0_3_ADDR(base) ((base) + (0x0150))
#define DPU_HDR_BITEXT_REG_INI1_0_ADDR(base) ((base) + (0x0154))
#define DPU_HDR_BITEXT_REG_INI1_1_ADDR(base) ((base) + (0x0158))
#define DPU_HDR_BITEXT_REG_INI1_2_ADDR(base) ((base) + (0x015C))
#define DPU_HDR_BITEXT_REG_INI1_3_ADDR(base) ((base) + (0x0160))
#define DPU_HDR_BITEXT_REG_INI2_0_ADDR(base) ((base) + (0x0164))
#define DPU_HDR_BITEXT_REG_INI2_1_ADDR(base) ((base) + (0x0168))
#define DPU_HDR_BITEXT_REG_INI2_2_ADDR(base) ((base) + (0x016C))
#define DPU_HDR_BITEXT_REG_INI2_3_ADDR(base) ((base) + (0x0170))
#define DPU_HDR_BITEXT_POWER_CTRL_C_ADDR(base) ((base) + (0x0174))
#define DPU_HDR_BITEXT_POWER_CTRL_SHIFT_ADDR(base) ((base) + (0x0178))
#define DPU_HDR_BITEXT_FILT_00_ADDR(base) ((base) + (0x017C))
#define DPU_HDR_BITEXT_FILT_01_ADDR(base) ((base) + (0x0180))
#define DPU_HDR_BITEXT_FILT_02_ADDR(base) ((base) + (0x0184))
#define DPU_HDR_BITEXT_FILT_10_ADDR(base) ((base) + (0x0188))
#define DPU_HDR_BITEXT_FILT_11_ADDR(base) ((base) + (0x018C))
#define DPU_HDR_BITEXT_FILT_12_ADDR(base) ((base) + (0x0190))
#define DPU_HDR_BITEXT_FILT_20_ADDR(base) ((base) + (0x0194))
#define DPU_HDR_BITEXT_FILT_21_ADDR(base) ((base) + (0x01A0))
#define DPU_HDR_BITEXT_FILT_22_ADDR(base) ((base) + (0x01A4))
#define DPU_HDR_BITEXT_THD_R0_ADDR(base) ((base) + (0x01A8))
#define DPU_HDR_BITEXT_THD_R1_ADDR(base) ((base) + (0x01AC))
#define DPU_HDR_BITEXT_THD_G0_ADDR(base) ((base) + (0x01B0))
#define DPU_HDR_BITEXT_THD_G1_ADDR(base) ((base) + (0x01B4))
#define DPU_HDR_BITEXT_THD_B0_ADDR(base) ((base) + (0x01B8))
#define DPU_HDR_BITEXT_THD_B1_ADDR(base) ((base) + (0x01BC))
#define DPU_HDR_BITEXT0_DBG0_ADDR(base) ((base) + (0x01C0))
#define DPU_HDR_MEM_CTRL_ADDR(base) ((base) + (0x0200))
#define DPU_HDR_CTRL_ADDR(base) ((base) + (0x0204))
#define DPU_HDR_SIZE_ADDR(base) ((base) + (0x0208))
#define DPU_HDR_INV_SIZE_ADDR(base) ((base) + (0x020C))
#define DPU_HDR_MEAN_STT_ADDR(base) ((base) + (0x0210))
#define DPU_HDR_CSC_CTRL_ADDR(base) ((base) + (0x0214))
#define DPU_HDR_CSC_IDC0_ADDR(base) ((base) + (0x0218))
#define DPU_HDR_CSC_IDC2_ADDR(base) ((base) + (0x021C))
#define DPU_HDR_CSC_ODC0_ADDR(base) ((base) + (0x0220))
#define DPU_HDR_CSC_ODC2_ADDR(base) ((base) + (0x0224))
#define DPU_HDR_CSC_P00_ADDR(base) ((base) + (0x0228))
#define DPU_HDR_CSC_P01_ADDR(base) ((base) + (0x022C))
#define DPU_HDR_CSC_P02_ADDR(base) ((base) + (0x0230))
#define DPU_HDR_CSC_P10_ADDR(base) ((base) + (0x0234))
#define DPU_HDR_CSC_P11_ADDR(base) ((base) + (0x0238))
#define DPU_HDR_CSC_P12_ADDR(base) ((base) + (0x023C))
#define DPU_HDR_CSC_P20_ADDR(base) ((base) + (0x0240))
#define DPU_HDR_CSC_P21_ADDR(base) ((base) + (0x0244))
#define DPU_HDR_CSC_P22_ADDR(base) ((base) + (0x0248))
#define DPU_HDR_GTM_CTRL_ADDR(base) ((base) + (0x024C))
#define DPU_HDR_GTM_SLF_LCPROC_MIN_E_ADDR(base) ((base) + (0x0250))
#define DPU_HDR_GTM_SLF_LCPROC_CLIP1_ADDR(base) ((base) + (0x0254))
#define DPU_HDR_GTM_SLF_LCPROC_CLIP2_ADDR(base) ((base) + (0x0258))
#define DPU_HDR_GTM_SLF_LCPROC_SCALE_ADDR(base) ((base) + (0x025C))
#define DPU_HDR_GTM_SLF_LCPROC_THRESHOLD_ADDR(base) ((base) + (0x0260))
#define DPU_HDR_GTM_SLF_LCPROC_Y_TRANSFM0_ADDR(base) ((base) + (0x0264))
#define DPU_HDR_GTM_SLF_LCPROC_Y_TRANSFM1_ADDR(base) ((base) + (0x0268))
#define DPU_HDR_GTM_SLF_LCPROC_OUT_BIT_ADDR(base) ((base) + (0x026C))
#define DPU_HDR_GTM_SLF_LCPROC_CRATIOCLIP_ADDR(base) ((base) + (0x0270))
#define DPU_HDR_GTM_PQ2LINEAR_COEF_ADDR(base,q) ((base) + (0x0274+0x4*(q)))
#define DPU_HDR_GTM_LINEAR2PQ_COEF_ADDR(base,q) ((base) + (0x0578+0x4*(q)))
#define DPU_HDR_GTM_LUMA0_LUT_ADDR(base,y1) ((base) + (0x077C+0x4*(y1)))
#define DPU_HDR_HDR_GTM_CHROMA_LUT_ADDR(base,y0) ((base) + (0x07FC+0x4*(y0)))
#define DPU_HDR_DBG_ADDR(base) ((base) + (0x081C))
#define DPU_HDR_GTM_SLF_LCPROC_MIN_O_ADDR(base) ((base) + (0x0820))
#define DPU_HDR_GTM_PQ2LINEAR_STEP1_ADDR(base) ((base) + (0x0824))
#define DPU_HDR_GTM_PQ2LINEAR_STEP2_ADDR(base) ((base) + (0x0828))
#define DPU_HDR_GTM_PQ2LINEAR_POS1_ADDR(base) ((base) + (0x082C))
#define DPU_HDR_GTM_PQ2LINEAR_POS2_ADDR(base) ((base) + (0x0830))
#define DPU_HDR_GTM_PQ2LINEAR_POS3_ADDR(base) ((base) + (0x0834))
#define DPU_HDR_GTM_PQ2LINEAR_POS4_ADDR(base) ((base) + (0x0838))
#define DPU_HDR_GTM_PQ2LINEAR_POS5_ADDR(base) ((base) + (0x083C))
#define DPU_HDR_GTM_PQ2LINEAR_POS6_ADDR(base) ((base) + (0x0840))
#define DPU_HDR_GTM_PQ2LINEAR_POS7_ADDR(base) ((base) + (0x0844))
#define DPU_HDR_GTM_PQ2LINEAR_POS8_ADDR(base) ((base) + (0x0848))
#define DPU_HDR_GTM_PQ2LINEAR_NUM1_ADDR(base) ((base) + (0x084C))
#define DPU_HDR_GTM_PQ2LINEAR_NUM2_ADDR(base) ((base) + (0x0850))
#define DPU_HDR_GTM_PQ2LINEAR_NUM3_ADDR(base) ((base) + (0x0854))
#define DPU_HDR_GTM_PQ2LINEAR_NUM4_ADDR(base) ((base) + (0x0858))
#define DPU_HDR_GTM_LINEAR2PQ_STEP1_ADDR(base) ((base) + (0x085C))
#define DPU_HDR_GTM_LINEAR2PQ_STEP2_ADDR(base) ((base) + (0x0860))
#define DPU_HDR_GTM_LINEAR2PQ_POS1_ADDR(base) ((base) + (0x0864))
#define DPU_HDR_GTM_LINEAR2PQ_POS2_ADDR(base) ((base) + (0x0868))
#define DPU_HDR_GTM_LINEAR2PQ_POS3_ADDR(base) ((base) + (0x086C))
#define DPU_HDR_GTM_LINEAR2PQ_POS4_ADDR(base) ((base) + (0x0870))
#define DPU_HDR_GTM_LINEAR2PQ_POS5_ADDR(base) ((base) + (0x0874))
#define DPU_HDR_GTM_LINEAR2PQ_POS6_ADDR(base) ((base) + (0x0878))
#define DPU_HDR_GTM_LINEAR2PQ_POS7_ADDR(base) ((base) + (0x087C))
#define DPU_HDR_GTM_LINEAR2PQ_POS8_ADDR(base) ((base) + (0x0880))
#define DPU_HDR_GTM_LINEAR2PQ_NUM1_ADDR(base) ((base) + (0x0884))
#define DPU_HDR_GTM_LINEAR2PQ_NUM2_ADDR(base) ((base) + (0x0888))
#define DPU_HDR_GTM_LINEAR2PQ_NUM3_ADDR(base) ((base) + (0x088C))
#define DPU_HDR_GTM_LINEAR2PQ_NUM4_ADDR(base) ((base) + (0x0890))
#define DPU_HDR_GTM_RGB2Y_ADDR(base) ((base) + (0x0894))
#define DPU_HDR_CSC1_IDC0_ADDR(base) ((base) + (0x0898))
#define DPU_HDR_CSC1_IDC2_ADDR(base) ((base) + (0x089C))
#define DPU_HDR_CSC1_ODC0_ADDR(base) ((base) + (0x08A0))
#define DPU_HDR_CSC1_ODC2_ADDR(base) ((base) + (0x08A4))
#define DPU_HDR_CSC1_P00_ADDR(base) ((base) + (0x08A8))
#define DPU_HDR_CSC1_P01_ADDR(base) ((base) + (0x08AC))
#define DPU_HDR_CSC1_P02_ADDR(base) ((base) + (0x08B0))
#define DPU_HDR_CSC1_P10_ADDR(base) ((base) + (0x08B4))
#define DPU_HDR_CSC1_P11_ADDR(base) ((base) + (0x08B8))
#define DPU_HDR_CSC1_P12_ADDR(base) ((base) + (0x08BC))
#define DPU_HDR_CSC1_P20_ADDR(base) ((base) + (0x08C0))
#define DPU_HDR_CSC1_P21_ADDR(base) ((base) + (0x08C4))
#define DPU_HDR_CSC1_P22_ADDR(base) ((base) + (0x08C8))
#define DPU_HDR_CSC2_IDC0_ADDR(base) ((base) + (0x08CC))
#define DPU_HDR_CSC2_IDC2_ADDR(base) ((base) + (0x08D0))
#define DPU_HDR_CSC2_ODC0_ADDR(base) ((base) + (0x08D4))
#define DPU_HDR_CSC2_ODC2_ADDR(base) ((base) + (0x08D8))
#define DPU_HDR_CSC2_P00_ADDR(base) ((base) + (0x08DC))
#define DPU_HDR_CSC2_P01_ADDR(base) ((base) + (0x08E0))
#define DPU_HDR_CSC2_P02_ADDR(base) ((base) + (0x08E4))
#define DPU_HDR_CSC2_P10_ADDR(base) ((base) + (0x08E8))
#define DPU_HDR_CSC2_P11_ADDR(base) ((base) + (0x08EC))
#define DPU_HDR_CSC2_P12_ADDR(base) ((base) + (0x08F0))
#define DPU_HDR_CSC2_P20_ADDR(base) ((base) + (0x08F4))
#define DPU_HDR_CSC2_P21_ADDR(base) ((base) + (0x08F8))
#define DPU_HDR_CSC2_P22_ADDR(base) ((base) + (0x08FC))
#define DPU_HDR_DEGAMMA_EN_ADDR(base) ((base) + (0x0A00))
#define DPU_HDR_DEGAMMA_MEM_CTRL_ADDR(base) ((base) + (0x0A04))
#define DPU_HDR_DEGAMMA_LUT_SEL_ADDR(base) ((base) + (0x0A08))
#define DPU_HDR_DEGAMMA_DBG0_ADDR(base) ((base) + (0x0A0C))
#define DPU_HDR_DEGAMMA_DBG1_ADDR(base) ((base) + (0x0A10))
#define DPU_HDR_GMP_EN_ADDR(base) ((base) + (0x0A14))
#define DPU_HDR_GMP_MEM_CTRL_ADDR(base) ((base) + (0x0A18))
#define DPU_HDR_GMP_LUT_SEL_ADDR(base) ((base) + (0x0A1C))
#define DPU_HDR_GMP_DBG_W0_ADDR(base) ((base) + (0x0A20))
#define DPU_HDR_GMP_DBG_R0_ADDR(base) ((base) + (0x0A24))
#define DPU_HDR_GMP_DBG_R1_ADDR(base) ((base) + (0x0A28))
#define DPU_HDR_GMP_DBG_R2_ADDR(base) ((base) + (0x0A2C))
#define DPU_HDR_GAMMA_EN_ADDR(base) ((base) + (0x0A30))
#define DPU_HDR_GAMMA_MEM_CTRL_ADDR(base) ((base) + (0x0A34))
#define DPU_HDR_GAMMA_LUT_SEL_ADDR(base) ((base) + (0x0A38))
#define DPU_HDR_GAMMA_DBG0_ADDR(base) ((base) + (0x0A3C))
#define DPU_HDR_GAMMA_DBG1_ADDR(base) ((base) + (0x0A40))
#define DPU_HDR_GTM_LUMA_LUT_ADDR(base,y3) ((base) + (0x1000+0x4*(y3)))
#define DPU_HDR_GTM_CHROMA_LUT_ADDR(base,y3) ((base) + (0x1200+0x4*(y3)))
#define DPU_HDR_GTM_CHROMA0_LUT_ADDR(base,y3) ((base) + (0x1400+0x4*(y3)))
#define DPU_HDR_GTM_CHROMA1_LUT_ADDR(base,y3) ((base) + (0x1600+0x4*(y3)))
#define DPU_HDR_GTM_CHROMA2_LUT_ADDR(base,y3) ((base) + (0x1800+0x4*(y3)))
#define DPU_HDR_DEGAMMA_R_COEF_ADDR(base,q) ((base) + (0x2000+0x4*(q)))
#define DPU_HDR_DEGAMMA_G_COEF_ADDR(base,q) ((base) + (0x2400+0x4*(q)))
#define DPU_HDR_DEGAMMA_B_COEF_ADDR(base,q) ((base) + (0x2800+0x4*(q)))
#define DPU_HDR_GAMMA_R_COEF_ADDR(base,q) ((base) + (0x3000+0x4*(q)))
#define DPU_HDR_GAMMA_G_COEF_ADDR(base,q) ((base) + (0x3400+0x4*(q)))
#define DPU_HDR_GAMMA_B_COEF_ADDR(base,q) ((base) + (0x3800+0x4*(q)))
#define DPU_HDR_GMP_LUT_GMP_COEF_ADDR(base,u1) ((base) + (0x4*(u1)))
#define DPU_CLD_RGB_ADDR(base) ((base) + (0x0000))
#define DPU_CLD_CLK_EN_ADDR(base) ((base) + (0x0004))
#define DPU_CLD_CLK_SEL_ADDR(base) ((base) + (0x0008))
#define DPU_CLD_DBG_ADDR(base) ((base) + (0x000C))
#define DPU_CLD_DBG1_ADDR(base) ((base) + (0x0010))
#define DPU_CLD_DBG2_ADDR(base) ((base) + (0x0014))
#define DPU_CLD_DBG3_ADDR(base) ((base) + (0x0018))
#define DPU_CLD_DBG4_ADDR(base) ((base) + (0x001C))
#define DPU_CLD_DBG5_ADDR(base) ((base) + (0x0020))
#define DPU_CLD_DBG6_ADDR(base) ((base) + (0x0024))
#define DPU_CLD_DBG7_ADDR(base) ((base) + (0x0028))
#define DPU_CLD_EN_ADDR(base) ((base) + (0x002C))
#define DPU_CLD_SIZE_ADDR(base) ((base) + (0x0030))
#define DPU_CLD_REG_CTRL_ADDR(base) ((base) + (0x0040))
#define DPU_CLD_REG_CTRL_FLUSH_EN_ADDR(base) ((base) + (0x0044))
#define DPU_CLD_REG_CTRL_DEBUG_ADDR(base) ((base) + (0x0048))
#define DPU_CLD_REG_CTRL_STATE_ADDR(base) ((base) + (0x004C))
#define DPU_CLD_REG_CTRL_LAYER_ID_ADDR(base) ((base) + (0x0050))
#define DPU_CLD_REG_RD_SHADOW_ADDR(base) ((base) + (0x0090))
#define DPU_CLD_REG_DEFAULT_ADDR(base) ((base) + (0x0094))
#define DPU_CLD_R_LB_DEBUG0_ADDR(base) ((base) + (0x0100))
#define DPU_CLD_R_LB_DEBUG1_ADDR(base) ((base) + (0x0104))
#define DPU_CLD_R_LB_DEBUG2_ADDR(base) ((base) + (0x0108))
#define DPU_CLD_R_LB_DEBUG3_ADDR(base) ((base) + (0x010C))
#define DPU_CLD_W_LB_DEBUG0_ADDR(base) ((base) + (0x0110))
#define DPU_CLD_W_LB_DEBUG1_ADDR(base) ((base) + (0x0114))
#define DPU_CLD_W_LB_DEBUG2_ADDR(base) ((base) + (0x0118))
#define DPU_CLD_W_LB_DEBUG3_ADDR(base) ((base) + (0x011C))
#define DPU_RCH_OV_RCH_OV0_TOP_CLK_SEL_ADDR(base) ((base) + (0x0000))
#define DPU_RCH_OV_RCH_OV0_TOP_CLK_EN_ADDR(base) ((base) + (0x0004))
#define DPU_RCH_OV_RCH_OV1_TOP_CLK_SEL_ADDR(base) ((base) + (0x0008))
#define DPU_RCH_OV_RCH_OV1_TOP_CLK_EN_ADDR(base) ((base) + (0x000C))
#define DPU_RCH_OV_RCH_OV2_TOP_CLK_SEL_ADDR(base) ((base) + (0x0010))
#define DPU_RCH_OV_RCH_OV2_TOP_CLK_EN_ADDR(base) ((base) + (0x0014))
#define DPU_RCH_OV_RCH_OV3_TOP_CLK_SEL_ADDR(base) ((base) + (0x0018))
#define DPU_RCH_OV_RCH_OV3_TOP_CLK_EN_ADDR(base) ((base) + (0x001C))
#define DPU_RCH_OV_WRAP_CLK_SEL_ADDR(base) ((base) + (0x0020))
#define DPU_RCH_OV_WRAP_CLK_EN_ADDR(base) ((base) + (0x0024))
#define DPU_RCH_OV_RCH_DBG0_ADDR(base) ((base) + (0x0030))
#define DPU_RCH_OV_RCH_DBG1_ADDR(base) ((base) + (0x0034))
#define DPU_RCH_OV_RCH_DBG2_ADDR(base) ((base) + (0x0038))
#define DPU_RCH_OV_RCH_DBG3_ADDR(base) ((base) + (0x003C))
#define DPU_RCH_OV_RCH_DBG4_ADDR(base) ((base) + (0x0040))
#define DPU_RCH_OV_RCH_DBG5_ADDR(base) ((base) + (0x0044))
#define DPU_RCH_OV_RCH_DBG6_ADDR(base) ((base) + (0x0048))
#define DPU_RCH_OV_RCH_DBG7_ADDR(base) ((base) + (0x004C))
#define DPU_RCH_OV_RCH_DBG8_ADDR(base) ((base) + (0x0050))
#define DPU_RCH_OV_RCH_DBG9_ADDR(base) ((base) + (0x0054))
#define DPU_RCH_OV_RCH_DBG10_ADDR(base) ((base) + (0x0058))
#define DPU_RCH_OV_RCH_DBG11_ADDR(base) ((base) + (0x005C))
#define DPU_RCH_OV_OV_MEM_CTRL_0_ADDR(base) ((base) + (0x0060))
#define DPU_RCH_OV_OV_MEM_CTRL_1_ADDR(base) ((base) + (0x0064))
#define DPU_RCH_OV_OV_MEM_CTRL_2_ADDR(base) ((base) + (0x0068))
#define DPU_RCH_OV_OV_MEM_CTRL_3_ADDR(base) ((base) + (0x006C))
#define DPU_RCH_OV_RCH_DBG12_ADDR(base) ((base) + (0x0070))
#define DPU_RCH_OV_RCH_DBG13_ADDR(base) ((base) + (0x0074))
#define DPU_RCH_OV_RCH_DBG14_ADDR(base) ((base) + (0x0078))
#define DPU_RCH_OV_RCH_DBG15_ADDR(base) ((base) + (0x007C))
#define DPU_RCH_OV_RCH_DBG16_ADDR(base) ((base) + (0x0080))
#define DPU_RCH_OV_RCH_DBG17_ADDR(base) ((base) + (0x0084))
#define DPU_RCH_OV_RCH_DBG18_ADDR(base) ((base) + (0x0088))
#define DPU_RCH_OV_RCH_DBG19_ADDR(base) ((base) + (0x008C))
#define DPU_RCH_OV_BITEXT_CTL_ADDR(base,i0) ((base) + (0x0100+0x100*(i0)))
#define DPU_RCH_OV_BITEXT_REG_INI0_0_ADDR(base,i0) ((base) + (0x0104+0x100*(i0)))
#define DPU_RCH_OV_BITEXT_REG_INI0_1_ADDR(base,i0) ((base) + (0x0108+0x100*(i0)))
#define DPU_RCH_OV_BITEXT_REG_INI0_2_ADDR(base,i0) ((base) + (0x010C+0x100*(i0)))
#define DPU_RCH_OV_BITEXT_REG_INI0_3_ADDR(base,i0) ((base) + (0x0110+0x100*(i0)))
#define DPU_RCH_OV_BITEXT_REG_INI1_0_ADDR(base,i0) ((base) + (0x0114+0x100*(i0)))
#define DPU_RCH_OV_BITEXT_REG_INI1_1_ADDR(base,i0) ((base) + (0x0118+0x100*(i0)))
#define DPU_RCH_OV_BITEXT_REG_INI1_2_ADDR(base,i0) ((base) + (0x011C+0x100*(i0)))
#define DPU_RCH_OV_BITEXT_REG_INI1_3_ADDR(base,i0) ((base) + (0x0120+0x100*(i0)))
#define DPU_RCH_OV_BITEXT_REG_INI2_0_ADDR(base,i0) ((base) + (0x0124+0x100*(i0)))
#define DPU_RCH_OV_BITEXT_REG_INI2_1_ADDR(base,i0) ((base) + (0x0128+0x100*(i0)))
#define DPU_RCH_OV_BITEXT_REG_INI2_2_ADDR(base,i0) ((base) + (0x012C+0x100*(i0)))
#define DPU_RCH_OV_BITEXT_REG_INI2_3_ADDR(base,i0) ((base) + (0x0130+0x100*(i0)))
#define DPU_RCH_OV_BITEXT_POWER_CTRL_C_ADDR(base,i0) ((base) + (0x0134+0x100*(i0)))
#define DPU_RCH_OV_BITEXT_POWER_CTRL_SHIFT_ADDR(base,i0) ((base) + (0x0138+0x100*(i0)))
#define DPU_RCH_OV_BITEXT_FILT_00_ADDR(base,i0) ((base) + (0x013C+0x100*(i0)))
#define DPU_RCH_OV_BITEXT_FILT_01_ADDR(base,i0) ((base) + (0x0140+0x100*(i0)))
#define DPU_RCH_OV_BITEXT_FILT_02_ADDR(base,i0) ((base) + (0x0144+0x100*(i0)))
#define DPU_RCH_OV_BITEXT_FILT_10_ADDR(base,i0) ((base) + (0x0148+0x100*(i0)))
#define DPU_RCH_OV_BITEXT_FILT_11_ADDR(base,i0) ((base) + (0x014C+0x100*(i0)))
#define DPU_RCH_OV_BITEXT_FILT_12_ADDR(base,i0) ((base) + (0x0150+0x100*(i0)))
#define DPU_RCH_OV_BITEXT_FILT_20_ADDR(base,i0) ((base) + (0x0154+0x100*(i0)))
#define DPU_RCH_OV_BITEXT_FILT_21_ADDR(base,i0) ((base) + (0x0158+0x100*(i0)))
#define DPU_RCH_OV_BITEXT_FILT_22_ADDR(base,i0) ((base) + (0x015C+0x100*(i0)))
#define DPU_RCH_OV_BITEXT_THD_R0_ADDR(base,i0) ((base) + (0x0160+0x100*(i0)))
#define DPU_RCH_OV_BITEXT_THD_R1_ADDR(base,i0) ((base) + (0x0164+0x100*(i0)))
#define DPU_RCH_OV_BITEXT_THD_G0_ADDR(base,i0) ((base) + (0x0168+0x100*(i0)))
#define DPU_RCH_OV_BITEXT_THD_G1_ADDR(base,i0) ((base) + (0x016C+0x100*(i0)))
#define DPU_RCH_OV_BITEXT_THD_B0_ADDR(base,i0) ((base) + (0x0170+0x100*(i0)))
#define DPU_RCH_OV_BITEXT_THD_B1_ADDR(base,i0) ((base) + (0x0174+0x100*(i0)))
#define DPU_RCH_OV_DFC_GLB_ALPHA23_ADDR(base,i0) ((base) + (0x0178+0x100*(i0)))
#define DPU_RCH_OV_BITEXT0_DBG0_ADDR(base,i0) ((base) + (0x017C+0x100*(i0)))
#define DPU_RCH_OV_CSC_IDC0_ADDR(base,j0) ((base) + (0x0300+0x80*(j0)))
#define DPU_RCH_OV_CSC_IDC2_ADDR(base,j0) ((base) + (0x0304+0x80*(j0)))
#define DPU_RCH_OV_CSC_ODC0_ADDR(base,j0) ((base) + (0x0308+0x80*(j0)))
#define DPU_RCH_OV_CSC_ODC2_ADDR(base,j0) ((base) + (0x030C+0x80*(j0)))
#define DPU_RCH_OV_CSC_P00_ADDR(base,j0) ((base) + (0x0310+0x80*(j0)))
#define DPU_RCH_OV_CSC_P01_ADDR(base,j0) ((base) + (0x0314+0x80*(j0)))
#define DPU_RCH_OV_CSC_P02_ADDR(base,j0) ((base) + (0x0318+0x80*(j0)))
#define DPU_RCH_OV_CSC_P10_ADDR(base,j0) ((base) + (0x031C+0x80*(j0)))
#define DPU_RCH_OV_CSC_P11_ADDR(base,j0) ((base) + (0x0320+0x80*(j0)))
#define DPU_RCH_OV_CSC_P12_ADDR(base,j0) ((base) + (0x0324+0x80*(j0)))
#define DPU_RCH_OV_CSC_P20_ADDR(base,j0) ((base) + (0x0328+0x80*(j0)))
#define DPU_RCH_OV_CSC_P21_ADDR(base,j0) ((base) + (0x032C+0x80*(j0)))
#define DPU_RCH_OV_CSC_P22_ADDR(base,j0) ((base) + (0x0330+0x80*(j0)))
#define DPU_RCH_OV_CSC_MPREC_ADDR(base,j0) ((base) + (0x0338+0x80*(j0)))
#define DPU_RCH_OV_MITM_CTRL_ADDR(base,k0) ((base) + (0x0810+0x200*(k0)))
#define DPU_RCH_OV_MITM_SLF_DEGAMMA_STEP1_ADDR(base,k0) ((base) + (0x0814+0x200*(k0)))
#define DPU_RCH_OV_MITM_SLF_DEGAMMA_STEP2_ADDR(base,k0) ((base) + (0x0818+0x200*(k0)))
#define DPU_RCH_OV_MITM_SLF_DEGAMMA_POS1_ADDR(base,k0) ((base) + (0x081C+0x200*(k0)))
#define DPU_RCH_OV_MITM_SLF_DEGAMMA_POS2_ADDR(base,k0) ((base) + (0x0820+0x200*(k0)))
#define DPU_RCH_OV_MITM_SLF_DEGAMMA_POS3_ADDR(base,k0) ((base) + (0x0824+0x200*(k0)))
#define DPU_RCH_OV_MITM_SLF_DEGAMMA_POS4_ADDR(base,k0) ((base) + (0x0828+0x200*(k0)))
#define DPU_RCH_OV_MITM_SLF_DEGAMMA_NUM1_ADDR(base,k0) ((base) + (0x082C+0x200*(k0)))
#define DPU_RCH_OV_MITM_SLF_DEGAMMA_NUM2_ADDR(base,k0) ((base) + (0x0830+0x200*(k0)))
#define DPU_RCH_OV_MITM_SLF_GAMUT_COEF00_0_ADDR(base,k0) ((base) + (0x0834+0x200*(k0)))
#define DPU_RCH_OV_MITM_SLF_GAMUT_COEF01_0_ADDR(base,k0) ((base) + (0x0838+0x200*(k0)))
#define DPU_RCH_OV_MITM_SLF_GAMUT_COEF02_0_ADDR(base,k0) ((base) + (0x083C+0x200*(k0)))
#define DPU_RCH_OV_MITM_SLF_GAMUT_COEF10_0_ADDR(base,k0) ((base) + (0x0840+0x200*(k0)))
#define DPU_RCH_OV_MITM_SLF_GAMUT_COEF11_0_ADDR(base,k0) ((base) + (0x0844+0x200*(k0)))
#define DPU_RCH_OV_MITM_SLF_GAMUT_COEF12_0_ADDR(base,k0) ((base) + (0x0848+0x200*(k0)))
#define DPU_RCH_OV_MITM_SLF_GAMUT_COEF20_0_ADDR(base,k0) ((base) + (0x084C+0x200*(k0)))
#define DPU_RCH_OV_MITM_SLF_GAMUT_COEF21_0_ADDR(base,k0) ((base) + (0x0850+0x200*(k0)))
#define DPU_RCH_OV_MITM_SLF_GAMUT_COEF22_0_ADDR(base,k0) ((base) + (0x0854+0x200*(k0)))
#define DPU_RCH_OV_MITM_SLF_GAMUT_SCALE_ADDR(base,k0) ((base) + (0x0858+0x200*(k0)))
#define DPU_RCH_OV_MITM_SLF_GAMUT_CLIP_MAX_ADDR(base,k0) ((base) + (0x085C+0x200*(k0)))
#define DPU_RCH_OV_MITM_SLF_GAMMA_STEP1_ADDR(base,k0) ((base) + (0x0860+0x200*(k0)))
#define DPU_RCH_OV_MITM_SLF_GAMMA_STEP2_ADDR(base,k0) ((base) + (0x0864+0x200*(k0)))
#define DPU_RCH_OV_MITM_SLF_GAMMA_POS1_ADDR(base,k0) ((base) + (0x0868+0x200*(k0)))
#define DPU_RCH_OV_MITM_SLF_GAMMA_POS2_ADDR(base,k0) ((base) + (0x086C+0x200*(k0)))
#define DPU_RCH_OV_MITM_SLF_GAMMA_POS3_ADDR(base,k0) ((base) + (0x0870+0x200*(k0)))
#define DPU_RCH_OV_MITM_SLF_GAMMA_POS4_ADDR(base,k0) ((base) + (0x0874+0x200*(k0)))
#define DPU_RCH_OV_MITM_SLF_GAMMA_POS5_ADDR(base,k0) ((base) + (0x0878+0x200*(k0)))
#define DPU_RCH_OV_MITM_SLF_GAMMA_POS6_ADDR(base,k0) ((base) + (0x087C+0x200*(k0)))
#define DPU_RCH_OV_MITM_SLF_GAMMA_POS7_ADDR(base,k0) ((base) + (0x0880+0x200*(k0)))
#define DPU_RCH_OV_MITM_SLF_GAMMA_POS8_ADDR(base,k0) ((base) + (0x0884+0x200*(k0)))
#define DPU_RCH_OV_MITM_SLF_GAMMA_NUM1_ADDR(base,k0) ((base) + (0x0888+0x200*(k0)))
#define DPU_RCH_OV_MITM_SLF_GAMMA_NUM2_ADDR(base,k0) ((base) + (0x088C+0x200*(k0)))
#define DPU_RCH_OV_MITM_ALPHA_ADDR(base,k0) ((base) + (0x0890+0x200*(k0)))
#define DPU_RCH_OV_MITM_DEGAMMA_LUT_ADDR(base,y1,k0) ((base) + ((0x0894+0x200*(k0)) + 0x4*(y1)))
#define DPU_RCH_OV_MITM_GAMMA_LUT_ADDR(base,y1,k0) ((base) + ((0x0914+0x200*(k0)) + 0x4*(y1)))
#define DPU_RCH_OV_MITM_DBG0_ADDR(base,k0) ((base) + (0x0994+0x200*(k0)))
#define DPU_RCH_OV_MITM_DBG1_ADDR(base,k0) ((base) + (0x0998+0x200*(k0)))
#define DPU_RCH_OV_LAYER_SRCLOKEY_ADDR(base) ((base) + (0x0c00))
#define DPU_RCH_OV_LAYER_SRCHIKEY_ADDR(base) ((base) + (0x0c04))
#define DPU_RCH_OV_LAYER_DSTLOKEY_ADDR(base) ((base) + (0x0c08))
#define DPU_RCH_OV_LAYER_DSTHIKEY_ADDR(base) ((base) + (0x0c0c))
#define DPU_RCH_OV_LAYER_ALPHA_MODE_ADDR(base) ((base) + (0x0c10))
#define DPU_RCH_OV_OV0_DBG0_ADDR(base) ((base) + (0x0c14))
#define DPU_RCH_OV_OV0_DBG1_ADDR(base) ((base) + (0x0c18))
#define DPU_RCH_OV_OV0_DBG2_ADDR(base) ((base) + (0x0c1c))
#define DPU_RCH_OV_OV0_DBG3_ADDR(base) ((base) + (0x0c20))
#define DPU_RCH_OV_OV1_DBG0_ADDR(base) ((base) + (0x0c24))
#define DPU_RCH_OV_OV1_DBG1_ADDR(base) ((base) + (0x0c28))
#define DPU_RCH_OV_OV1_DBG2_ADDR(base) ((base) + (0x0c2c))
#define DPU_RCH_OV_OV1_DBG3_ADDR(base) ((base) + (0x0c30))
#define DPU_RCH_OV_OV2_DBG0_ADDR(base) ((base) + (0x0c34))
#define DPU_RCH_OV_OV2_DBG1_ADDR(base) ((base) + (0x0c38))
#define DPU_RCH_OV_OV2_DBG2_ADDR(base) ((base) + (0x0c3c))
#define DPU_RCH_OV_OV2_DBG3_ADDR(base) ((base) + (0x0c40))
#define DPU_RCH_OV_OV3_DBG0_ADDR(base) ((base) + (0x0c44))
#define DPU_RCH_OV_OV3_DBG1_ADDR(base) ((base) + (0x0c48))
#define DPU_RCH_OV_OV3_DBG2_ADDR(base) ((base) + (0x0c4c))
#define DPU_RCH_OV_OV3_DBG3_ADDR(base) ((base) + (0x0c50))
#define DPU_RCH_OV_RXX0_DBG0_ADDR(base) ((base) + (0x0d00))
#define DPU_RCH_OV_RXX0_DBG1_ADDR(base) ((base) + (0x0d04))
#define DPU_RCH_OV_RXX0_DBG2_ADDR(base) ((base) + (0x0d08))
#define DPU_RCH_OV_RXX0_DBG3_ADDR(base) ((base) + (0x0d0c))
#define DPU_RCH_OV_RXX1_DBG0_ADDR(base) ((base) + (0x0d10))
#define DPU_RCH_OV_RXX1_DBG1_ADDR(base) ((base) + (0x0d14))
#define DPU_RCH_OV_RXX1_DBG2_ADDR(base) ((base) + (0x0d18))
#define DPU_RCH_OV_RXX1_DBG3_ADDR(base) ((base) + (0x0d1c))
#define DPU_RCH_OV_RXX2_DBG0_ADDR(base) ((base) + (0x0d20))
#define DPU_RCH_OV_RXX2_DBG1_ADDR(base) ((base) + (0x0d24))
#define DPU_RCH_OV_RXX2_DBG2_ADDR(base) ((base) + (0x0d28))
#define DPU_RCH_OV_RXX2_DBG3_ADDR(base) ((base) + (0x0d2c))
#define DPU_RCH_OV_RXX3_DBG0_ADDR(base) ((base) + (0x0d30))
#define DPU_RCH_OV_RXX3_DBG1_ADDR(base) ((base) + (0x0d34))
#define DPU_RCH_OV_RXX3_DBG2_ADDR(base) ((base) + (0x0d38))
#define DPU_RCH_OV_RXX3_DBG3_ADDR(base) ((base) + (0x0d3c))
#define DPU_RCH_OV_WXX0_DBG0_ADDR(base) ((base) + (0x0d40))
#define DPU_RCH_OV_WXX0_DBG1_ADDR(base) ((base) + (0x0d44))
#define DPU_RCH_OV_WXX0_DBG2_ADDR(base) ((base) + (0x0d48))
#define DPU_RCH_OV_WXX0_DBG3_ADDR(base) ((base) + (0x0d4c))
#define DPU_RCH_OV_WXX1_DBG0_ADDR(base) ((base) + (0x0d50))
#define DPU_RCH_OV_WXX1_DBG1_ADDR(base) ((base) + (0x0d54))
#define DPU_RCH_OV_WXX1_DBG2_ADDR(base) ((base) + (0x0d58))
#define DPU_RCH_OV_WXX1_DBG3_ADDR(base) ((base) + (0x0d5c))
#define DPU_RCH_OV_WXX2_DBG0_ADDR(base) ((base) + (0x0d60))
#define DPU_RCH_OV_WXX2_DBG1_ADDR(base) ((base) + (0x0d64))
#define DPU_RCH_OV_WXX2_DBG2_ADDR(base) ((base) + (0x0d68))
#define DPU_RCH_OV_WXX2_DBG3_ADDR(base) ((base) + (0x0d6c))
#define DPU_RCH_OV_WXX3_DBG0_ADDR(base) ((base) + (0x0d70))
#define DPU_RCH_OV_WXX3_DBG1_ADDR(base) ((base) + (0x0d74))
#define DPU_RCH_OV_WXX3_DBG2_ADDR(base) ((base) + (0x0d78))
#define DPU_RCH_OV_WXX3_DBG3_ADDR(base) ((base) + (0x0d7c))
#define DPU_DPP_INTS_ADDR(base) ((base) + (0x0000))
#define DPU_DPP_INT_MSK_ADDR(base) ((base) + (0x0004))
#define DPU_DPP_CH_CLK_SEL_ADDR(base) ((base) + (0x0008))
#define DPU_DPP_CH_CLK_EN_ADDR(base) ((base) + (0x000C))
#define DPU_DPP_CLRBAR_IMG_SIZE_ADDR(base) ((base) + (0x0010))
#define DPU_DPP_CLRBAR_CTRL_ADDR(base) ((base) + (0x0014))
#define DPU_DPP_CLRBAR_1ST_CLR_ADDR(base) ((base) + (0x0018))
#define DPU_DPP_CLRBAR_2ND_CLR_ADDR(base) ((base) + (0x001C))
#define DPU_DPP_CLRBAR_3RD_CLR_ADDR(base) ((base) + (0x0020))
#define DPU_DPP_CRC_CFG_EN_ADDR(base) ((base) + (0x0030))
#define DPU_DPP_CRC_FRM_CNT_ADDR(base) ((base) + (0x0034))
#define DPU_DPP_CRC_RESULT_ADDR(base) ((base) + (0x0038))
#define DPU_DPP_CH_RD_SHADOW_SEL_ADDR(base) ((base) + (0x003C))
#define DPU_DPP_REG_DEFAULT_ADDR(base) ((base) + (0x0040))
#define DPU_DPP_CH_DBG_CNT_ADDR(base) ((base) + (0x002C))
#define DPU_DPP_CH_RO_ADDR(base) ((base) + (0x0044))
#define DPU_DPP_CH_RW1_ADDR(base) ((base) + (0x0048))
#define DPU_DPP_CH_RW2_ADDR(base) ((base) + (0x004C))
#define DPU_DPP_REG_CTRL_ADDR(base) ((base) + (0x0058))
#define DPU_DPP_REG_CTRL_FLUSH_EN_ADDR(base) ((base) + (0x0060))
#define DPU_DPP_REG_CTRL_DEBUG_ADDR(base) ((base) + (0x0064))
#define DPU_DPP_REG_CTRL_STATE_ADDR(base) ((base) + (0x0068))
#define DPU_DPP_REG_CTRL_LAYER_ID_ADDR(base) ((base) + (0x006C))
#define DPU_DPP_R_LB_DEBUG0_ADDR(base) ((base) + (0x0080))
#define DPU_DPP_R_LB_DEBUG1_ADDR(base) ((base) + (0x0084))
#define DPU_DPP_R_LB_DEBUG2_ADDR(base) ((base) + (0x0088))
#define DPU_DPP_R_LB_DEBUG3_ADDR(base) ((base) + (0x008C))
#define DPU_DPP_W_LB_DEBUG0_ADDR(base) ((base) + (0x0090))
#define DPU_DPP_W_LB_DEBUG1_ADDR(base) ((base) + (0x0094))
#define DPU_DPP_W_LB_DEBUG2_ADDR(base) ((base) + (0x0098))
#define DPU_DPP_W_LB_DEBUG3_ADDR(base) ((base) + (0x009C))
#define DPU_DPP_PADD_EN_ADDR(base) ((base) + (0x0350))
#define DPU_DPP_PADD_MODE_ADDR(base) ((base) + (0x0354))
#define DPU_DPP_PADD_DATA_ADDR(base) ((base) + (0x0358))
#define DPU_DPP_PADD_LEFT_ADDR(base) ((base) + (0x035C))
#define DPU_DPP_WIDTH_AFT_PADD_ADDR(base) ((base) + (0x0360))
#define DPU_DPP_PADD_PIC_SIZE_ADDR(base) ((base) + (0x0364))
#define DPU_DPP_RGB_HIST_EN_ADDR(base) ((base) + (0x0404))
#define DPU_DPP_R0_HIST_ADDR(base) ((base) + (0x0408))
#define DPU_DPP_R1_HIST_ADDR(base) ((base) + (0x040C))
#define DPU_DPP_R2_HIST_ADDR(base) ((base) + (0x0410))
#define DPU_DPP_R3_HIST_ADDR(base) ((base) + (0x0414))
#define DPU_DPP_R4_HIST_ADDR(base) ((base) + (0x0418))
#define DPU_DPP_R5_HIST_ADDR(base) ((base) + (0x041C))
#define DPU_DPP_R6_HIST_ADDR(base) ((base) + (0x0420))
#define DPU_DPP_R7_HIST_ADDR(base) ((base) + (0x0424))
#define DPU_DPP_R8_HIST_ADDR(base) ((base) + (0x0428))
#define DPU_DPP_R9_HIST_ADDR(base) ((base) + (0x042C))
#define DPU_DPP_R10_HIST_ADDR(base) ((base) + (0x0430))
#define DPU_DPP_R11_HIST_ADDR(base) ((base) + (0x0434))
#define DPU_DPP_R12_HIST_ADDR(base) ((base) + (0x0438))
#define DPU_DPP_R13_HIST_ADDR(base) ((base) + (0x043C))
#define DPU_DPP_R14_HIST_ADDR(base) ((base) + (0x0440))
#define DPU_DPP_R15_HIST_ADDR(base) ((base) + (0x0444))
#define DPU_DPP_G0_HIST_ADDR(base) ((base) + (0x0448))
#define DPU_DPP_G1_HIST_ADDR(base) ((base) + (0x044C))
#define DPU_DPP_G2_HIST_ADDR(base) ((base) + (0x0450))
#define DPU_DPP_G3_HIST_ADDR(base) ((base) + (0x0454))
#define DPU_DPP_G4_HIST_ADDR(base) ((base) + (0x0458))
#define DPU_DPP_G5_HIST_ADDR(base) ((base) + (0x045C))
#define DPU_DPP_G6_HIST_ADDR(base) ((base) + (0x0460))
#define DPU_DPP_G7_HIST_ADDR(base) ((base) + (0x0464))
#define DPU_DPP_G8_HIST_ADDR(base) ((base) + (0x0468))
#define DPU_DPP_G9_HIST_ADDR(base) ((base) + (0x046C))
#define DPU_DPP_G10_HIST_ADDR(base) ((base) + (0x0470))
#define DPU_DPP_G11_HIST_ADDR(base) ((base) + (0x0474))
#define DPU_DPP_G12_HIST_ADDR(base) ((base) + (0x0478))
#define DPU_DPP_G13_HIST_ADDR(base) ((base) + (0x047C))
#define DPU_DPP_G14_HIST_ADDR(base) ((base) + (0x0480))
#define DPU_DPP_G15_HIST_ADDR(base) ((base) + (0x0484))
#define DPU_DPP_B0_HIST_ADDR(base) ((base) + (0x0488))
#define DPU_DPP_B1_HIST_ADDR(base) ((base) + (0x048C))
#define DPU_DPP_B2_HIST_ADDR(base) ((base) + (0x0490))
#define DPU_DPP_B3_HIST_ADDR(base) ((base) + (0x0494))
#define DPU_DPP_B4_HIST_ADDR(base) ((base) + (0x0498))
#define DPU_DPP_B5_HIST_ADDR(base) ((base) + (0x049C))
#define DPU_DPP_B6_HIST_ADDR(base) ((base) + (0x04A0))
#define DPU_DPP_B7_HIST_ADDR(base) ((base) + (0x04A4))
#define DPU_DPP_B8_HIST_ADDR(base) ((base) + (0x04A8))
#define DPU_DPP_B9_HIST_ADDR(base) ((base) + (0x04AC))
#define DPU_DPP_B10_HIST_ADDR(base) ((base) + (0x04B0))
#define DPU_DPP_B11_HIST_ADDR(base) ((base) + (0x04B4))
#define DPU_DPP_B12_HIST_ADDR(base) ((base) + (0x04B8))
#define DPU_DPP_B13_HIST_ADDR(base) ((base) + (0x04BC))
#define DPU_DPP_B14_HIST_ADDR(base) ((base) + (0x04C0))
#define DPU_DPP_B15_HIST_ADDR(base) ((base) + (0x04C4))
#define DPU_DPP_SPR_MEM_CTRL_ADDR(base) ((base) + (0x0500))
#define DPU_DPP_SPR_SIZE_ADDR(base) ((base) + (0x0504))
#define DPU_DPP_SPR_CTRL_ADDR(base) ((base) + (0x0508))
#define DPU_DPP_SPR_PIX_EVEN_COEF_SEL_ADDR(base) ((base) + (0x050C))
#define DPU_DPP_SPR_PIX_ODD_COEF_SEL_ADDR(base) ((base) + (0x0510))
#define DPU_DPP_SPR_PIX_PANEL_ARRANGE_SEL_ADDR(base) ((base) + (0x0514))
#define DPU_DPP_SPR_COEFF_V0H0_R0_ADDR(base) ((base) + (0x0518))
#define DPU_DPP_SPR_COEFF_V0H0_R1_ADDR(base) ((base) + (0x051C))
#define DPU_DPP_SPR_COEFF_V0H1_R0_ADDR(base) ((base) + (0x0520))
#define DPU_DPP_SPR_COEFF_V0H1_R1_ADDR(base) ((base) + (0x0524))
#define DPU_DPP_SPR_COEFF_V1H0_R0_ADDR(base) ((base) + (0x0528))
#define DPU_DPP_SPR_COEFF_V1H0_R1_ADDR(base) ((base) + (0x052C))
#define DPU_DPP_SPR_COEFF_V1H1_R0_ADDR(base) ((base) + (0x0530))
#define DPU_DPP_SPR_COEFF_V1H1_R1_ADDR(base) ((base) + (0x0534))
#define DPU_DPP_SPR_COEFF_V0H0_G0_ADDR(base) ((base) + (0x0538))
#define DPU_DPP_SPR_COEFF_V0H0_G1_ADDR(base) ((base) + (0x053C))
#define DPU_DPP_SPR_COEFF_V0H1_G0_ADDR(base) ((base) + (0x0540))
#define DPU_DPP_SPR_COEFF_V0H1_G1_ADDR(base) ((base) + (0x0544))
#define DPU_DPP_SPR_COEFF_V1H0_G0_ADDR(base) ((base) + (0x0548))
#define DPU_DPP_SPR_COEFF_V1H0_G1_ADDR(base) ((base) + (0x054C))
#define DPU_DPP_SPR_COEFF_V1H1_G0_ADDR(base) ((base) + (0x0550))
#define DPU_DPP_SPR_COEFF_V1H1_G1_ADDR(base) ((base) + (0x0554))
#define DPU_DPP_SPR_COEFF_V0H0_B0_ADDR(base) ((base) + (0x0558))
#define DPU_DPP_SPR_COEFF_V0H0_B1_ADDR(base) ((base) + (0x055C))
#define DPU_DPP_SPR_COEFF_V0H1_B0_ADDR(base) ((base) + (0x0560))
#define DPU_DPP_SPR_COEFF_V0H1_B1_ADDR(base) ((base) + (0x0564))
#define DPU_DPP_SPR_COEFF_V1H0_B0_ADDR(base) ((base) + (0x0568))
#define DPU_DPP_SPR_COEFF_V1H0_B1_ADDR(base) ((base) + (0x056C))
#define DPU_DPP_SPR_COEFF_V1H1_B0_ADDR(base) ((base) + (0x0570))
#define DPU_DPP_SPR_COEFF_V1H1_B1_ADDR(base) ((base) + (0x0574))
#define DPU_DPP_SPR_LAREA_START_ADDR(base) ((base) + (0x0578))
#define DPU_DPP_SPR_LAREA_END_ADDR(base) ((base) + (0x057C))
#define DPU_DPP_SPR_LAREA_OFFSET_ADDR(base) ((base) + (0x0580))
#define DPU_DPP_SPR_LAREA_GAIN_ADDR(base) ((base) + (0x0584))
#define DPU_DPP_SPR_LAREA_BORDER_GAIN_R_ADDR(base) ((base) + (0x0588))
#define DPU_DPP_SPR_LAREA_BORDER_GAIN_G_ADDR(base) ((base) + (0x058C))
#define DPU_DPP_SPR_LAREA_BORDER_GAIN_B_ADDR(base) ((base) + (0x0590))
#define DPU_DPP_SPR_R_BORDERLR_REG_ADDR(base) ((base) + (0x0594))
#define DPU_DPP_SPR_R_BORDERTB_REG_ADDR(base) ((base) + (0x0598))
#define DPU_DPP_SPR_G_BORDERLR_REG_ADDR(base) ((base) + (0x059C))
#define DPU_DPP_SPR_G_BORDERTB_REG_ADDR(base) ((base) + (0x05A0))
#define DPU_DPP_SPR_B_BORDERLR_REG_ADDR(base) ((base) + (0x05A4))
#define DPU_DPP_SPR_B_BORDERTB_REG_ADDR(base) ((base) + (0x05A8))
#define DPU_DPP_SPR_PIXGAIN_REG_ADDR(base) ((base) + (0x05AC))
#define DPU_DPP_SPR_PIXGAIN_REG1_ADDR(base) ((base) + (0x05B0))
#define DPU_DPP_SPR_BORDER_POSITION0_ADDR(base) ((base) + (0x05B4))
#define DPU_DPP_SPR_BORDER_POSITION1_ADDR(base) ((base) + (0x05B8))
#define DPU_DPP_SPR_BORDER_POSITION2_ADDR(base) ((base) + (0x05BC))
#define DPU_DPP_SPR_BORDER_POSITION3_ADDR(base) ((base) + (0x05C0))
#define DPU_DPP_SPR_BLEND_ADDR(base) ((base) + (0x05C4))
#define DPU_DPP_SPR_WEIGHT_ADDR(base) ((base) + (0x05C8))
#define DPU_DPP_SPR_EDGESTR_R_ADDR(base) ((base) + (0x05CC))
#define DPU_DPP_SPR_EDGESTR_G_ADDR(base) ((base) + (0x05D0))
#define DPU_DPP_SPR_EDGESTR_B_ADDR(base) ((base) + (0x05D4))
#define DPU_DPP_SPR_DIRWEIBLDMIN_ADDR(base) ((base) + (0x05D8))
#define DPU_DPP_SPR_DIRWEIBLDMAX_ADDR(base) ((base) + (0x05DC))
#define DPU_DPP_SPR_DIFFDIRGAIN_R0_ADDR(base) ((base) + (0x05E0))
#define DPU_DPP_SPR_DIFFDIRGAIN_R1_ADDR(base) ((base) + (0x05E4))
#define DPU_DPP_SPR_DIFFDIRGAIN_G0_ADDR(base) ((base) + (0x05E8))
#define DPU_DPP_SPR_DIFFDIRGAIN_G1_ADDR(base) ((base) + (0x05EC))
#define DPU_DPP_SPR_DIFFDIRGAIN_B0_ADDR(base) ((base) + (0x05F0))
#define DPU_DPP_SPR_DIFFDIRGAIN_B1_ADDR(base) ((base) + (0x05F4))
#define DPU_DPP_SPR_HORZGRADBLEND_ADDR(base) ((base) + (0x05F8))
#define DPU_DPP_SPR_HORZBDBLD_ADDR(base) ((base) + (0x05FC))
#define DPU_DPP_SPR_HORZBDWEIGHT_ADDR(base) ((base) + (0x0600))
#define DPU_DPP_SPR_VERTBDBLD_ADDR(base) ((base) + (0x0604))
#define DPU_DPP_SPR_VERTBDWEIGHT_ADDR(base) ((base) + (0x0608))
#define DPU_DPP_SPR_VERTBD_GAIN_R_ADDR(base) ((base) + (0x060C))
#define DPU_DPP_SPR_VERTBD_GAIN_G_ADDR(base) ((base) + (0x0610))
#define DPU_DPP_SPR_VERTBD_GAIN_B_ADDR(base) ((base) + (0x0614))
#define DPU_DPP_SPR_DEBUG_ADDR(base) ((base) + (0x0618))
#define DPU_DPP_PTG_CTRL_ADDR(base) ((base) + (0x0650))
#define DPU_DPP_PTG_SIZE_ADDR(base) ((base) + (0x0654))
#define DPU_DPP_PTG_POSITION_START_ADDR(base) ((base) + (0x0658))
#define DPU_DPP_PTG_POSITION_END_ADDR(base) ((base) + (0x065C))
#define DPU_DPP_PTG_PIX0_ADDR(base) ((base) + (0x0660))
#define DPU_DPP_PTG_PIX1_ADDR(base) ((base) + (0x0664))
#define DPU_DPP_PTG_DEBUG_ADDR(base) ((base) + (0x0668))
#define DPU_DPP_DEGAMA_EN_ADDR(base) ((base) + (0x0700))
#define DPU_DPP_DEGAMA_MEM_CTRL_ADDR(base) ((base) + (0x0704))
#define DPU_DPP_DEGAMA_LUT_SEL_ADDR(base) ((base) + (0x0708))
#define DPU_DPP_DEGAMA_DBG0_ADDR(base) ((base) + (0x070C))
#define DPU_DPP_DEGAMA_DBG1_ADDR(base) ((base) + (0x0710))
#define DPU_DPP_ROIGAMA0_EN_ADDR(base) ((base) + (0x0800))
#define DPU_DPP_ROIGAMA0_MEM_CTRL_ADDR(base) ((base) + (0x0804))
#define DPU_DPP_ROIGAMA0_LUT_SEL_ADDR(base) ((base) + (0x0808))
#define DPU_DPP_ROIGAMA0_DBG0_ADDR(base) ((base) + (0x080C))
#define DPU_DPP_ROIGAMA0_DBG1_ADDR(base) ((base) + (0x0810))
#define DPU_DPP_ROIGAMA1_EN_ADDR(base) ((base) + (0x0900))
#define DPU_DPP_ROIGAMA1_MEM_CTRL_ADDR(base) ((base) + (0x0904))
#define DPU_DPP_ROIGAMA1_LUT_SEL_ADDR(base) ((base) + (0x0908))
#define DPU_DPP_ROIGAMA1_DBG0_ADDR(base) ((base) + (0x090C))
#define DPU_DPP_ROIGAMA1_DBG1_ADDR(base) ((base) + (0x0910))
#define DPU_DPP_GAMA_SIZE_ADDR(base) ((base) + (0x0914))
#define DPU_DPP_GAMA_ROI_ADDR(base) ((base) + (0x0918))
#define DPU_DPP_GAMA_ROI0_START_ADDR(base) ((base) + (0x091C))
#define DPU_DPP_GAMA_ROI0_END_ADDR(base) ((base) + (0x0920))
#define DPU_DPP_GAMA_ROI1_START_ADDR(base) ((base) + (0x0924))
#define DPU_DPP_GAMA_ROI1_END_ADDR(base) ((base) + (0x0928))
#define DPU_DPP_GMP_EN_ADDR(base) ((base) + (0x9A0))
#define DPU_DPP_GMP_MEM_CTRL_ADDR(base) ((base) + (0x9A4))
#define DPU_DPP_GMP_LUT_SEL_ADDR(base) ((base) + (0x9A8))
#define DPU_DPP_GMP_DBG_W0_ADDR(base) ((base) + (0x9AC))
#define DPU_DPP_GMP_DBG_R0_ADDR(base) ((base) + (0x9B0))
#define DPU_DPP_GMP_DBG_R1_ADDR(base) ((base) + (0x9B4))
#define DPU_DPP_GMP_DBG_R2_ADDR(base) ((base) + (0x9B8))
#define DPU_DPP_GMP_SIZE_ADDR(base) ((base) + (0x9BC))
#define DPU_DPP_GMP_ROI_START_ADDR(base) ((base) + (0x9C0))
#define DPU_DPP_GMP_ROI_END_ADDR(base) ((base) + (0x9C4))
#define DPU_DPP_DITHER_CTL1_ADDR(base) ((base) + (0x1500))
#define DPU_DPP_DITHER_CTL0_ADDR(base) ((base) + (0x1504))
#define DPU_DPP_DITHER_TRI_THD12_0_ADDR(base) ((base) + (0x1508))
#define DPU_DPP_DITHER_TRI_THD12_1_ADDR(base) ((base) + (0x150C))
#define DPU_DPP_DITHER_TRI_THD10_ADDR(base) ((base) + (0x1510))
#define DPU_DPP_DITHER_TRI_THD12_UNI_0_ADDR(base) ((base) + (0x1514))
#define DPU_DPP_DITHER_TRI_THD12_UNI_1_ADDR(base) ((base) + (0x1518))
#define DPU_DPP_DITHER_TRI_THD10_UNI_ADDR(base) ((base) + (0x151C))
#define DPU_DPP_BAYER_CTL_ADDR(base) ((base) + (0x1520))
#define DPU_DPP_BAYER_ALPHA_THD_ADDR(base) ((base) + (0x1524))
#define DPU_DPP_BAYER_MATRIX_PART1_ADDR(base) ((base) + (0x1528))
#define DPU_DPP_BAYER_MATRIX_PART0_ADDR(base) ((base) + (0x152C))
#define DPU_DPP_HIFREQ_REG_INI_CFG_EN_ADDR(base) ((base) + (0x1530))
#define DPU_DPP_HIFREQ_REG_INI0_0_ADDR(base) ((base) + (0x1534))
#define DPU_DPP_HIFREQ_REG_INI0_1_ADDR(base) ((base) + (0x1538))
#define DPU_DPP_HIFREQ_REG_INI0_2_ADDR(base) ((base) + (0x153C))
#define DPU_DPP_HIFREQ_REG_INI0_3_ADDR(base) ((base) + (0x1540))
#define DPU_DPP_HIFREQ_REG_INI1_0_ADDR(base) ((base) + (0x1544))
#define DPU_DPP_HIFREQ_REG_INI1_1_ADDR(base) ((base) + (0x1548))
#define DPU_DPP_HIFREQ_REG_INI1_2_ADDR(base) ((base) + (0x154C))
#define DPU_DPP_HIFREQ_REG_INI1_3_ADDR(base) ((base) + (0x1550))
#define DPU_DPP_HIFREQ_REG_INI2_0_ADDR(base) ((base) + (0x1554))
#define DPU_DPP_HIFREQ_REG_INI2_1_ADDR(base) ((base) + (0x1558))
#define DPU_DPP_HIFREQ_REG_INI2_2_ADDR(base) ((base) + (0x155C))
#define DPU_DPP_HIFREQ_REG_INI2_3_ADDR(base) ((base) + (0x1560))
#define DPU_DPP_HIFREQ_POWER_CTRL_ADDR(base) ((base) + (0x1564))
#define DPU_DPP_HIFREQ_FILT_0_ADDR(base) ((base) + (0x1568))
#define DPU_DPP_HIFREQ_FILT_1_ADDR(base) ((base) + (0x156C))
#define DPU_DPP_HIFREQ_FILT_2_ADDR(base) ((base) + (0x1570))
#define DPU_DPP_HIFREQ_THD_R0_ADDR(base) ((base) + (0x1574))
#define DPU_DPP_HIFREQ_THD_R1_ADDR(base) ((base) + (0x1578))
#define DPU_DPP_HIFREQ_THD_G0_ADDR(base) ((base) + (0x157C))
#define DPU_DPP_HIFREQ_THD_G1_ADDR(base) ((base) + (0x1580))
#define DPU_DPP_HIFREQ_THD_B0_ADDR(base) ((base) + (0x1584))
#define DPU_DPP_HIFREQ_THD_B1_ADDR(base) ((base) + (0x1588))
#define DPU_DPP_HIFREQ_DBG0_ADDR(base) ((base) + (0x158C))
#define DPU_DPP_HIFREQ_DBG1_ADDR(base) ((base) + (0x1590))
#define DPU_DPP_HIFREQ_DBG2_ADDR(base) ((base) + (0x1594))
#define DPU_DPP_ERRDIFF_CTL_ADDR(base) ((base) + (0x1598))
#define DPU_DPP_ERRDIFF_WEIGHT_ADDR(base) ((base) + (0x159C))
#define DPU_DPP_DITHER_FRC_CTL_ADDR(base) ((base) + (0x15A0))
#define DPU_DPP_FRC_01_PART1_ADDR(base) ((base) + (0x15A4))
#define DPU_DPP_FRC_01_PART0_ADDR(base) ((base) + (0x15A8))
#define DPU_DPP_FRC_10_PART1_ADDR(base) ((base) + (0x15AC))
#define DPU_DPP_FRC_10_PART0_ADDR(base) ((base) + (0x15B0))
#define DPU_DPP_FRC_11_PART1_ADDR(base) ((base) + (0x15B4))
#define DPU_DPP_FRC_11_PART0_ADDR(base) ((base) + (0x15B8))
#define DPU_DPP_DITHER_MEM_CTRL_ADDR(base) ((base) + (0x15BC))
#define DPU_DPP_DITHER_DBG0_ADDR(base) ((base) + (0x15C0))
#define DPU_DPP_DITHER_DBG1_ADDR(base) ((base) + (0x15C4))
#define DPU_DPP_DITHER_DBG2_ADDR(base) ((base) + (0x15C8))
#define DPU_DPP_DITHER_CTRL2_ADDR(base) ((base) + (0x15CC))
#define DPU_DPP_DITHER_IMG_SIZE_ADDR(base) ((base) + (0x15D0))
#define DPU_DPP_CLIP_TOP_ADDR(base) ((base) + (0x1600))
#define DPU_DPP_CLIP_BOTTOM_ADDR(base) ((base) + (0x1604))
#define DPU_DPP_CLIP_LEFT_ADDR(base) ((base) + (0x1608))
#define DPU_DPP_CLIP_RIGHT_ADDR(base) ((base) + (0x160C))
#define DPU_DPP_CLIP_EN_ADDR(base) ((base) + (0x1610))
#define DPU_DPP_CLIP_DBG_ADDR(base) ((base) + (0x1614))
#define DPU_DPP_CLIP_IMG_SIZE_ADDR(base) ((base) + (0x1618))
#define DPU_DPP_NLXCC_COEF_00_ADDR(base) ((base) + (0x1700))
#define DPU_DPP_NLXCC_COEF_01_ADDR(base) ((base) + (0x1704))
#define DPU_DPP_NLXCC_COEF_02_ADDR(base) ((base) + (0x1708))
#define DPU_DPP_NLXCC_COEF_03_ADDR(base) ((base) + (0x170C))
#define DPU_DPP_NLXCC_COEF_10_ADDR(base) ((base) + (0x1710))
#define DPU_DPP_NLXCC_COEF_11_ADDR(base) ((base) + (0x1714))
#define DPU_DPP_NLXCC_COEF_12_ADDR(base) ((base) + (0x1718))
#define DPU_DPP_NLXCC_COEF_13_ADDR(base) ((base) + (0x171C))
#define DPU_DPP_NLXCC_COEF_20_ADDR(base) ((base) + (0x1720))
#define DPU_DPP_NLXCC_COEF_21_ADDR(base) ((base) + (0x1724))
#define DPU_DPP_NLXCC_COEF_22_ADDR(base) ((base) + (0x1728))
#define DPU_DPP_NLXCC_COEF_23_ADDR(base) ((base) + (0x172C))
#define DPU_DPP_NLXCC_EN_ADDR(base) ((base) + (0x1730))
#define DPU_DPP_NLXCC_DUAL_PXL_EN_ADDR(base) ((base) + (0x1734))
#define DPU_DPP_NLXCC_DEBUG_ADDR(base) ((base) + (0x1738))
#define DPU_DPP_NLXCC_SIZE_ADDR(base) ((base) + (0x173C))
#define DPU_DPP_NLXCC_ROI_START_ADDR(base) ((base) + (0x1740))
#define DPU_DPP_NLXCC_ROI_END_ADDR(base) ((base) + (0x1744))
#define DPU_DPP_XCC_COEF_00_ADDR(base) ((base) + (0x1750))
#define DPU_DPP_XCC_COEF_01_ADDR(base) ((base) + (0x1754))
#define DPU_DPP_XCC_COEF_02_ADDR(base) ((base) + (0x1758))
#define DPU_DPP_XCC_COEF_03_ADDR(base) ((base) + (0x175C))
#define DPU_DPP_XCC_COEF_10_ADDR(base) ((base) + (0x1760))
#define DPU_DPP_XCC_COEF_11_ADDR(base) ((base) + (0x1764))
#define DPU_DPP_XCC_COEF_12_ADDR(base) ((base) + (0x1768))
#define DPU_DPP_XCC_COEF_13_ADDR(base) ((base) + (0x176C))
#define DPU_DPP_XCC_COEF_20_ADDR(base) ((base) + (0x1770))
#define DPU_DPP_XCC_COEF_21_ADDR(base) ((base) + (0x1774))
#define DPU_DPP_XCC_COEF_22_ADDR(base) ((base) + (0x1778))
#define DPU_DPP_XCC_COEF_23_ADDR(base) ((base) + (0x177C))
#define DPU_DPP_XCC_EN_ADDR(base) ((base) + (0x1780))
#define DPU_DPP_XCC_SIZE_ADDR(base) ((base) + (0x1784))
#define DPU_DPP_XCC_ROI_START_ADDR(base) ((base) + (0x1788))
#define DPU_DPP_XCC_ROI_END_ADDR(base) ((base) + (0x178C))
#define DPU_DPP_HIGHLIGHT_ADDR(base) ((base) + (0x1800))
#define DPU_DPP_HL_HUE_GAIN_0_ADDR(base) ((base) + (0x1804))
#define DPU_DPP_HL_HUE_GAIN_1_ADDR(base) ((base) + (0x1808))
#define DPU_DPP_HL_HUE_GAIN_2_ADDR(base) ((base) + (0x180C))
#define DPU_DPP_HL_HUE_GAIN_3_ADDR(base) ((base) + (0x1810))
#define DPU_DPP_HL_HUE_GAIN_4_ADDR(base) ((base) + (0x1814))
#define DPU_DPP_HL_HUE_GAIN_5_ADDR(base) ((base) + (0x1818))
#define DPU_DPP_HL_SAT_ADDR(base) ((base) + (0x181C))
#define DPU_DPP_HL_YOUT_MAXTH_ADDR(base) ((base) + (0x1820))
#define DPU_DPP_HL_YOUT_ADDR(base) ((base) + (0x1824))
#define DPU_DPP_HL_DIFF_ADDR(base) ((base) + (0x1828))
#define DPU_DPP_SKIN_COUNT_ADDR(base) ((base) + (0x182C))
#define DPU_DPP_LRE_SAT_ADDR(base) ((base) + (0x1830))
#define DPU_DPP_LOCAL_REFRESH_H_ADDR(base) ((base) + (0x1834))
#define DPU_DPP_LOCAL_REFRESH_V_ADDR(base) ((base) + (0x1838))
#define DPU_DPP_GLOBAL_HIST_EN_ADDR(base) ((base) + (0x183C))
#define DPU_DPP_GLOBAL_HIST_START_ADDR(base) ((base) + (0x1840))
#define DPU_DPP_GLOBAL_HIST_SIZE_ADDR(base) ((base) + (0x1844))
#define DPU_DPP_SCREEN_MODE_ADDR(base) ((base) + (0x1848))
#define DPU_DPP_SEPARATOR_ADDR(base) ((base) + (0x184C))
#define DPU_DPP_WEIGHT_MIN_MAX_ADDR(base) ((base) + (0x1850))
#define DPU_DPP_FNA_EN_ADDR(base) ((base) + (0x1854))
#define DPU_DPP_DBG_HIACE_ADDR(base) ((base) + (0x1858))
#define DPU_DPP_SPR_GAMA_EN_ADDR(base) ((base) + (0x18A0))
#define DPU_DPP_SPR_GAMA_MEM_CTRL_ADDR(base) ((base) + (0x18A4))
#define DPU_DPP_SPR_GAMA_LUT_SEL_ADDR(base) ((base) + (0x18A8))
#define DPU_DPP_SPR_GAMA_DBG0_ADDR(base) ((base) + (0x18AC))
#define DPU_DPP_SPR_GAMA_DBG1_ADDR(base) ((base) + (0x18B0))
#define DPU_DPP_SPR_GAMA_SHIFTEN_ADDR(base) ((base) + (0x18B4))
#define DPU_DPP_ALSC_DEGAMMA_EN_ADDR(base) ((base) + (0x1900))
#define DPU_DPP_ALSC_DEGAMMA_MEM_CTRL_ADDR(base) ((base) + (0x1904))
#define DPU_DPP_ALSC_DEGAMMA_LUT_SEL_ADDR(base) ((base) + (0x1908))
#define DPU_DPP_ALSC_DEGAMMA_DBG0_ADDR(base) ((base) + (0x190C))
#define DPU_DPP_ALSC_DEGAMMA_DBG1_ADDR(base) ((base) + (0x1910))
#define DPU_DPP_ALSC_EN_ADDR(base) ((base) + (0x1914))
#define DPU_DPP_ALSC_PPRO_BYPASS_ADDR(base) ((base) + (0x1918))
#define DPU_DPP_SENSOR_CHANNEL_ADDR(base) ((base) + (0x191C))
#define DPU_DPP_PIC_SIZE_ADDR(base) ((base) + (0x1920))
#define DPU_DPP_ALSC_ADDR_ADDR(base) ((base) + (0x1924))
#define DPU_DPP_ALSC_SIZE_ADDR(base) ((base) + (0x1928))
#define DPU_DPP_NOISE1_ADDR(base) ((base) + (0x192C))
#define DPU_DPP_NOISE2_ADDR(base) ((base) + (0x1930))
#define DPU_DPP_NOISE3_ADDR(base) ((base) + (0x1934))
#define DPU_DPP_NOISE4_ADDR(base) ((base) + (0x1938))
#define DPU_DPP_COEF_R2R_ADDR(base) ((base) + (0x193C))
#define DPU_DPP_COEF_R2G_ADDR(base) ((base) + (0x1940))
#define DPU_DPP_COEF_R2B_ADDR(base) ((base) + (0x1944))
#define DPU_DPP_COEF_R2C_ADDR(base) ((base) + (0x1948))
#define DPU_DPP_COEF_G2R_ADDR(base) ((base) + (0x194C))
#define DPU_DPP_COEF_G2G_ADDR(base) ((base) + (0x1950))
#define DPU_DPP_COEF_G2B_ADDR(base) ((base) + (0x1954))
#define DPU_DPP_COEF_G2C_ADDR(base) ((base) + (0x1958))
#define DPU_DPP_COEF_B2R_ADDR(base) ((base) + (0x195C))
#define DPU_DPP_COEF_B2G_ADDR(base) ((base) + (0x1960))
#define DPU_DPP_COEF_B2B_ADDR(base) ((base) + (0x1964))
#define DPU_DPP_COEF_B2C_ADDR(base) ((base) + (0x1968))
#define DPU_DPP_TIMESTAMP_L_ADDR(base) ((base) + (0x196C))
#define DPU_DPP_TIMESTAMP_H_ADDR(base) ((base) + (0x1970))
#define DPU_DPP_ALSC_DBG0_ADDR(base) ((base) + (0x1974))
#define DPU_DPP_ALSC_DBG1_ADDR(base) ((base) + (0x1978))
#define DPU_DPP_ADDR_BLOCK_ADDR(base,n30) ((base) + (0x1A00+0x4*(n30)))
#define DPU_DPP_AVE_BLOCK_ADDR(base,n30) ((base) + (0x1A80+0x4*(n30)))
#define DPU_DPP_COEF_BLOCK_R_ADDR(base,n30) ((base) + (0x1B00+0x4*(n30)))
#define DPU_DPP_COEF_BLOCK_G_ADDR(base,n30) ((base) + (0x1B80+0x4*(n30)))
#define DPU_DPP_COEF_BLOCK_B_ADDR(base,n30) ((base) + (0x1C00+0x4*(n30)))
#define DPU_DPP_COEF_BLOCK_C_ADDR(base,n30) ((base) + (0x1C80+0x4*(n30)))
#define DPU_DPP_U_DEGAMA_R_COEF_ADDR(base,q) ((base) + (0x5000+0x4*(q)))
#define DPU_DPP_U_DEGAMA_G_COEF_ADDR(base,q) ((base) + (0x5400+0x4*(q)))
#define DPU_DPP_U_DEGAMA_B_COEF_ADDR(base,q) ((base) + (0x5800+0x4*(q)))
#define DPU_DPP_U_ROIGAMA0_R_COEF_ADDR(base,q) ((base) + (0x6000+0x4*(q)))
#define DPU_DPP_U_ROIGAMA0_G_COEF_ADDR(base,q) ((base) + (0x6400+0x4*(q)))
#define DPU_DPP_U_ROIGAMA0_B_COEF_ADDR(base,q) ((base) + (0x6800+0x4*(q)))
#define DPU_DPP_U_ROIGAMA1_R_COEF_ADDR(base,q) ((base) + (0x7000+0x4*(q)))
#define DPU_DPP_U_ROIGAMA1_G_COEF_ADDR(base,q) ((base) + (0x7400+0x4*(q)))
#define DPU_DPP_U_ROIGAMA1_B_COEF_ADDR(base,q) ((base) + (0x7800+0x4*(q)))
#define DPU_DPP_LOCAL_HIST_ADDR(base,q) ((base) + (0x8000+0x4*(q)))
#define DPU_DPP_LOCAL_GAMMA_ADDR(base,q) ((base) + (0x9000+0x4*(q)))
#define DPU_DPP_DETAIL_WEIGHT_ADDR(base,r) ((base) + (0x9500+0x4*(r)))
#define DPU_DPP_LOG_LUM_ADDR(base,s) ((base) + (0x9600+0x4*(s)))
#define DPU_DPP_LUMA_GAMMA_ADDR(base,t) ((base) + (0x9700+0x4*(t)))
#define DPU_DPP_GLB_HIST_ADDR(base,q) ((base) + (0x9800+0x4*(q)))
#define DPU_DPP_GLB_SAT_ADDR(base,q) ((base) + (0x9900+0x4*(q)))
#define DPU_DPP_FNA_ADDR(base,q) ((base) + (0x9A00+0x4*(q)))
#define DPU_DPP_SKINCOUNT_ADDR(base,z) ((base) + (0x9B00+0x4*(z)))
#define DPU_DPP_U_GAMA_R_COEF_ADDR(base,q) ((base) + (0xA000+0x4*(q)))
#define DPU_DPP_U_GAMA_G_COEF_ADDR(base,q) ((base) + (0xA400+0x4*(q)))
#define DPU_DPP_U_GAMA_B_COEF_ADDR(base,q) ((base) + (0xA800+0x4*(q)))
#define DPU_DPP_U_GMP_COEF_ADDR(base,u) ((base) + (0xB000+0x4*(u)))
#define DPU_DPP_U_ALSC_DEGAMMA_R_COEF_ADDR(base,q) ((base) + (0x16000+0x4*(q)))
#define DPU_DPP_U_ALSC_DEGAMM_G_COEF_ADDR(base,q) ((base) + (0x16400+0x4*(q)))
#define DPU_DPP_U_ALSC_DEGAMM_B_COEF_ADDR(base,q) ((base) + (0x16800+0x4*(q)))
#define DPU_HIACE_ACE_INT_STAT_ADDR(base) ((base) + (0x0000))
#define DPU_HIACE_ACE_INT_UNMASK_ADDR(base) ((base) + (0x0004))
#define DPU_HIACE_BYPASS_ACE_ADDR(base) ((base) + (0x0008))
#define DPU_HIACE_BYPASS_ACE_STAT_ADDR(base) ((base) + (0x000C))
#define DPU_HIACE_UPDATE_LOCAL_ADDR(base) ((base) + (0x0010))
#define DPU_HIACE_LOCAL_VALID_ADDR(base) ((base) + (0x0014))
#define DPU_HIACE_GAMMA_AB_SHADOW_ADDR(base) ((base) + (0x0018))
#define DPU_HIACE_GAMMA_AB_WORK_ADDR(base) ((base) + (0x001C))
#define DPU_HIACE_GLOBAL_HIST_AB_SHADOW_ADDR(base) ((base) + (0x0020))
#define DPU_HIACE_GLOBAL_HIST_AB_WORK_ADDR(base) ((base) + (0x0024))
#define DPU_HIACE_IMAGE_INFO_ADDR(base) ((base) + (0x0030))
#define DPU_HIACE_HALF_BLOCK_INFO_ADDR(base) ((base) + (0x0034))
#define DPU_HIACE_XYWEIGHT_ADDR(base) ((base) + (0x0038))
#define DPU_HIACE_LHIST_SFT_ADDR(base) ((base) + (0x003C))
#define DPU_HIACE_ROI_START_POINT_ADDR(base) ((base) + (0x0040))
#define DPU_HIACE_ROI_WIDTH_HIGH_ADDR(base) ((base) + (0x0044))
#define DPU_HIACE_ROI_MODE_CTRL_ADDR(base) ((base) + (0x0048))
#define DPU_HIACE_ROI_HIST_STAT_MODE_ADDR(base) ((base) + (0x004C))
#define DPU_HIACE_HUE_ADDR(base) ((base) + (0x0050))
#define DPU_HIACE_SATURATION_ADDR(base) ((base) + (0x0054))
#define DPU_HIACE_VALUE_ADDR(base) ((base) + (0x0058))
#define DPU_HIACE_SKIN_GAIN_ADDR(base) ((base) + (0x005C))
#define DPU_HIACE_UP_LOW_TH_ADDR(base) ((base) + (0x0060))
#define DPU_HIACE_RGB_BLEND_WEIGHT_ADDR(base) ((base) + (0x0064))
#define DPU_HIACE_FNA_STATISTIC_ADDR(base) ((base) + (0x0068))
#define DPU_HIACE_UP_CNT_ADDR(base) ((base) + (0x0070))
#define DPU_HIACE_LOW_CNT_ADDR(base) ((base) + (0x0074))
#define DPU_HIACE_SUM_SATURATION_ADDR(base) ((base) + (0x0078))
#define DPU_HIACE_GLOBAL_HIST_0_ADDR(base) ((base) + (0x0080))
#define DPU_HIACE_GLOBAL_HIST_1_ADDR(base) ((base) + (0x0084))
#define DPU_HIACE_GLOBAL_HIST_2_ADDR(base) ((base) + (0x0088))
#define DPU_HIACE_GLOBAL_HIST_3_ADDR(base) ((base) + (0x008C))
#define DPU_HIACE_GLOBAL_HIST_4_ADDR(base) ((base) + (0x0090))
#define DPU_HIACE_GLOBAL_HIST_5_ADDR(base) ((base) + (0x0094))
#define DPU_HIACE_GLOBAL_HIST_6_ADDR(base) ((base) + (0x0098))
#define DPU_HIACE_GLOBAL_HIST_7_ADDR(base) ((base) + (0x009C))
#define DPU_HIACE_GLOBAL_HIST_8_ADDR(base) ((base) + (0x00A0))
#define DPU_HIACE_GLOBAL_HIST_9_ADDR(base) ((base) + (0x00A4))
#define DPU_HIACE_GLOBAL_HIST_10_ADDR(base) ((base) + (0x00A8))
#define DPU_HIACE_GLOBAL_HIST_11_ADDR(base) ((base) + (0x00AC))
#define DPU_HIACE_GLOBAL_HIST_12_ADDR(base) ((base) + (0x00B0))
#define DPU_HIACE_GLOBAL_HIST_13_ADDR(base) ((base) + (0x00B4))
#define DPU_HIACE_GLOBAL_HIST_14_ADDR(base) ((base) + (0x00B8))
#define DPU_HIACE_GLOBAL_HIST_15_ADDR(base) ((base) + (0x00BC))
#define DPU_HIACE_GLOBAL_HIST_16_ADDR(base) ((base) + (0x00C0))
#define DPU_HIACE_GLOBAL_HIST_17_ADDR(base) ((base) + (0x00C4))
#define DPU_HIACE_GLOBAL_HIST_18_ADDR(base) ((base) + (0x00C8))
#define DPU_HIACE_GLOBAL_HIST_19_ADDR(base) ((base) + (0x00CC))
#define DPU_HIACE_GLOBAL_HIST_20_ADDR(base) ((base) + (0x00D0))
#define DPU_HIACE_GLOBAL_HIST_21_ADDR(base) ((base) + (0x00D4))
#define DPU_HIACE_GLOBAL_HIST_22_ADDR(base) ((base) + (0x00D8))
#define DPU_HIACE_GLOBAL_HIST_23_ADDR(base) ((base) + (0x00DC))
#define DPU_HIACE_GLOBAL_HIST_24_ADDR(base) ((base) + (0x00E0))
#define DPU_HIACE_GLOBAL_HIST_25_ADDR(base) ((base) + (0x00E4))
#define DPU_HIACE_GLOBAL_HIST_26_ADDR(base) ((base) + (0x00E8))
#define DPU_HIACE_GLOBAL_HIST_27_ADDR(base) ((base) + (0x00EC))
#define DPU_HIACE_GLOBAL_HIST_28_ADDR(base) ((base) + (0x00F0))
#define DPU_HIACE_GLOBAL_HIST_29_ADDR(base) ((base) + (0x00F4))
#define DPU_HIACE_GLOBAL_HIST_30_ADDR(base) ((base) + (0x00F8))
#define DPU_HIACE_GLOBAL_HIST_31_ADDR(base) ((base) + (0x00FC))
#define DPU_HIACE_LHIST_EN_ADDR(base) ((base) + (0x0100))
#define DPU_HIACE_LOCAL_HIST_ADDR(base) ((base) + (0x0104))
#define DPU_HIACE_GAMMA_W_ADDR(base) ((base) + (0x0108))
#define DPU_HIACE_GAMMA_VXHY_W_ADDR(base) ((base) + (0x010C))
#define DPU_HIACE_GAMMA_R_ADDR(base) ((base) + (0x0110))
#define DPU_HIACE_GAMMA_VXHY_R_ADDR(base) ((base) + (0x0114))
#define DPU_HIACE_INIT_GAMMA_ADDR(base) ((base) + (0x0120))
#define DPU_HIACE_MANUAL_RELOAD_ADDR(base) ((base) + (0x0124))
#define DPU_HIACE_RAMCLK_FUNC_ADDR(base) ((base) + (0x0128))
#define DPU_HIACE_CLK_GATE_ADDR(base) ((base) + (0x012C))
#define DPU_HIACE_GAMMA_RAM_A_CFG_MEM_CTRL_ADDR(base) ((base) + (0x0130))
#define DPU_HIACE_GAMMA_RAM_B_CFG_MEM_CTRL_ADDR(base) ((base) + (0x0134))
#define DPU_HIACE_LHIST_RAM_CFG_MEM_CTRL_ADDR(base) ((base) + (0x0138))
#define DPU_HIACE_GAMMA_RAM_A_CFG_PM_CTRL_ADDR(base) ((base) + (0x0140))
#define DPU_HIACE_GAMMA_RAM_B_CFG_PM_CTRL_ADDR(base) ((base) + (0x0144))
#define DPU_HIACE_LHIST_RAM_CFG_PM_CTRL_ADDR(base) ((base) + (0x0148))
#define DPU_HIACE_SAT_GLOBAL_HIST_0_ADDR(base) ((base) + (0x0180))
#define DPU_HIACE_SAT_GLOBAL_HIST_1_ADDR(base) ((base) + (0x0184))
#define DPU_HIACE_SAT_GLOBAL_HIST_2_ADDR(base) ((base) + (0x0188))
#define DPU_HIACE_SAT_GLOBAL_HIST_3_ADDR(base) ((base) + (0x018C))
#define DPU_HIACE_SAT_GLOBAL_HIST_4_ADDR(base) ((base) + (0x0190))
#define DPU_HIACE_SAT_GLOBAL_HIST_5_ADDR(base) ((base) + (0x0194))
#define DPU_HIACE_SAT_GLOBAL_HIST_6_ADDR(base) ((base) + (0x0198))
#define DPU_HIACE_SAT_GLOBAL_HIST_7_ADDR(base) ((base) + (0x019C))
#define DPU_HIACE_SAT_GLOBAL_HIST_8_ADDR(base) ((base) + (0x01A0))
#define DPU_HIACE_SAT_GLOBAL_HIST_9_ADDR(base) ((base) + (0x01A4))
#define DPU_HIACE_SAT_GLOBAL_HIST_10_ADDR(base) ((base) + (0x01A8))
#define DPU_HIACE_SAT_GLOBAL_HIST_11_ADDR(base) ((base) + (0x01AC))
#define DPU_HIACE_SAT_GLOBAL_HIST_12_ADDR(base) ((base) + (0x01B0))
#define DPU_HIACE_SAT_GLOBAL_HIST_13_ADDR(base) ((base) + (0x01B4))
#define DPU_HIACE_SAT_GLOBAL_HIST_14_ADDR(base) ((base) + (0x01B8))
#define DPU_HIACE_SAT_GLOBAL_HIST_15_ADDR(base) ((base) + (0x01BC))
#define DPU_HIACE_SAT_GLOBAL_HIST_16_ADDR(base) ((base) + (0x01C0))
#define DPU_HIACE_SAT_GLOBAL_HIST_17_ADDR(base) ((base) + (0x01C4))
#define DPU_HIACE_SAT_GLOBAL_HIST_18_ADDR(base) ((base) + (0x01C8))
#define DPU_HIACE_SAT_GLOBAL_HIST_19_ADDR(base) ((base) + (0x01CC))
#define DPU_HIACE_SAT_GLOBAL_HIST_20_ADDR(base) ((base) + (0x01D0))
#define DPU_HIACE_SAT_GLOBAL_HIST_21_ADDR(base) ((base) + (0x01D4))
#define DPU_HIACE_SAT_GLOBAL_HIST_22_ADDR(base) ((base) + (0x01D8))
#define DPU_HIACE_SAT_GLOBAL_HIST_23_ADDR(base) ((base) + (0x01DC))
#define DPU_HIACE_SAT_GLOBAL_HIST_24_ADDR(base) ((base) + (0x01E0))
#define DPU_HIACE_SAT_GLOBAL_HIST_25_ADDR(base) ((base) + (0x01E4))
#define DPU_HIACE_SAT_GLOBAL_HIST_26_ADDR(base) ((base) + (0x01E8))
#define DPU_HIACE_SAT_GLOBAL_HIST_27_ADDR(base) ((base) + (0x01EC))
#define DPU_HIACE_SAT_GLOBAL_HIST_28_ADDR(base) ((base) + (0x01F0))
#define DPU_HIACE_SAT_GLOBAL_HIST_29_ADDR(base) ((base) + (0x01F4))
#define DPU_HIACE_SAT_GLOBAL_HIST_30_ADDR(base) ((base) + (0x01F8))
#define DPU_HIACE_SAT_GLOBAL_HIST_31_ADDR(base) ((base) + (0x01FC))
#define DPU_HIACE_FNA_ADDR_ADDR(base) ((base) + (0x0200))
#define DPU_HIACE_FNA_DATA_ADDR(base) ((base) + (0x0204))
#define DPU_HIACE_UPDATE_FNA_ADDR(base) ((base) + (0x0208))
#define DPU_HIACE_FNA_VALID_ADDR(base) ((base) + (0x0210))
#define DPU_HIACE_DB_PIPE_CFG_ADDR(base) ((base) + (0x0220))
#define DPU_HIACE_DB_PIPE_EXT_WIDTH_ADDR(base) ((base) + (0x0224))
#define DPU_HIACE_DB_PIPE_FULL_IMG_WIDTH_ADDR(base) ((base) + (0x0228))
#define DPU_HIACE_HDR_EN_ADDR(base) ((base) + (0x0230))
#define DPU_HIACE_LOGLUM_MAX_ADDR(base) ((base) + (0x0234))
#define DPU_HIACE_HIST_MODEV_ADDR(base) ((base) + (0x0238))
#define DPU_HIACE_END_POINT_ADDR(base) ((base) + (0x023C))
#define DPU_HIACE_ACE_DBG0_ADDR(base) ((base) + (0x0300))
#define DPU_HIACE_ACE_DBG1_ADDR(base) ((base) + (0x0304))
#define DPU_HIACE_ACE_DBG2_ADDR(base) ((base) + (0x0308))
#define DPU_HIACE_ACE_DBG_CLK_ADDR(base) ((base) + (0x030C))
#define DPU_HIACE_BYPASS_NR_ADDR(base) ((base) + (0x0400))
#define DPU_HIACE_S3_SOME_BRIGHTNESS01_ADDR(base) ((base) + (0x0410))
#define DPU_HIACE_S3_SOME_BRIGHTNESS23_ADDR(base) ((base) + (0x0414))
#define DPU_HIACE_S3_SOME_BRIGHTNESS4_ADDR(base) ((base) + (0x0418))
#define DPU_HIACE_S3_MIN_MAX_SIGMA_ADDR(base) ((base) + (0x0420))
#define DPU_HIACE_S3_GREEN_SIGMA03_ADDR(base) ((base) + (0x0430))
#define DPU_HIACE_S3_GREEN_SIGMA45_ADDR(base) ((base) + (0x0434))
#define DPU_HIACE_S3_RED_SIGMA03_ADDR(base) ((base) + (0x0440))
#define DPU_HIACE_S3_RED_SIGMA45_ADDR(base) ((base) + (0x0444))
#define DPU_HIACE_S3_BLUE_SIGMA03_ADDR(base) ((base) + (0x0450))
#define DPU_HIACE_S3_BLUE_SIGMA45_ADDR(base) ((base) + (0x0454))
#define DPU_HIACE_S3_WHITE_SIGMA03_ADDR(base) ((base) + (0x0460))
#define DPU_HIACE_S3_WHITE_SIGMA45_ADDR(base) ((base) + (0x0464))
#define DPU_HIACE_S3_FILTER_LEVEL_ADDR(base) ((base) + (0x0470))
#define DPU_HIACE_S3_SIMILARITY_COEFF_ADDR(base) ((base) + (0x0474))
#define DPU_HIACE_S3_V_FILTER_WEIGHT_ADJ_ADDR(base) ((base) + (0x0478))
#define DPU_HIACE_S3_HUE_ADDR(base) ((base) + (0x0480))
#define DPU_HIACE_S3_SATURATION_ADDR(base) ((base) + (0x0484))
#define DPU_HIACE_S3_VALUE_ADDR(base) ((base) + (0x0488))
#define DPU_HIACE_S3_SKIN_GAIN_ADDR(base) ((base) + (0x048C))
#define DPU_HIACE_NR_RAMCLK_FUNC_ADDR(base) ((base) + (0x0490))
#define DPU_HIACE_NR_CLK_GATE_ADDR(base) ((base) + (0x0494))
#define DPU_HIACE_NR_RAM_A_CFG_MEM_CTRL_ADDR(base) ((base) + (0x0498))
#define DPU_HIACE_NR_RAM_A_CFG_PM_CTRL_ADDR(base) ((base) + (0x049C))
#define DPU_DSC_VERSION_ADDR(base) ((base) + (0x0000))
#define DPU_DSC_PPS_IDENTIFIER_ADDR(base) ((base) + (0x0004))
#define DPU_DSC_EN_ADDR(base) ((base) + (0x0008))
#define DPU_DSC_CTRL_ADDR(base) ((base) + (0x000C))
#define DPU_DSC_PIC_SIZE_ADDR(base) ((base) + (0x0010))
#define DPU_DSC_SLICE_SIZE_ADDR(base) ((base) + (0x0014))
#define DPU_DSC_CHUNK_SIZE_ADDR(base) ((base) + (0x0018))
#define DPU_DSC_INITIAL_DELAY_ADDR(base) ((base) + (0x001C))
#define DPU_DSC_RC_PARAM0_ADDR(base) ((base) + (0x0020))
#define DPU_DSC_RC_PARAM1_ADDR(base) ((base) + (0x0024))
#define DPU_DSC_RC_PARAM2_ADDR(base) ((base) + (0x0028))
#define DPU_DSC_RC_PARAM3_ADDR(base) ((base) + (0x002C))
#define DPU_DSC_FLATNESS_PARAM_ADDR(base) ((base) + (0x0030))
#define DPU_DSC_RC_PARAM4_ADDR(base) ((base) + (0x0034))
#define DPU_DSC_RC_PARAM5_ADDR(base) ((base) + (0x0038))
#define DPU_DSC_RC_BUF_THRESH0_ADDR(base) ((base) + (0x003C))
#define DPU_DSC_RC_BUF_THRESH1_ADDR(base) ((base) + (0x0040))
#define DPU_DSC_RC_BUF_THRESH2_ADDR(base) ((base) + (0x0044))
#define DPU_DSC_RC_BUF_THRESH3_ADDR(base) ((base) + (0x0048))
#define DPU_DSC_RC_RANGE_PARAM0_ADDR(base) ((base) + (0x004C))
#define DPU_DSC_RC_RANGE_PARAM1_ADDR(base) ((base) + (0x0050))
#define DPU_DSC_RC_RANGE_PARAM2_ADDR(base) ((base) + (0x0054))
#define DPU_DSC_RC_RANGE_PARAM3_ADDR(base) ((base) + (0x0058))
#define DPU_DSC_RC_RANGE_PARAM4_ADDR(base) ((base) + (0x005C))
#define DPU_DSC_RC_RANGE_PARAM5_ADDR(base) ((base) + (0x0060))
#define DPU_DSC_RC_RANGE_PARAM6_ADDR(base) ((base) + (0x0064))
#define DPU_DSC_RC_RANGE_PARAM7_ADDR(base) ((base) + (0x0068))
#define DPU_DSC_ADJUSTMENT_BITS_ADDR(base) ((base) + (0x006C))
#define DPU_DSC_BITS_PER_GRP_ADDR(base) ((base) + (0x0070))
#define DPU_DSC_MULTI_SLICE_CTL_ADDR(base) ((base) + (0x0074))
#define DPU_DSC_OUT_CTRL_ADDR(base) ((base) + (0x0078))
#define DPU_DSC_CLK_SEL_ADDR(base) ((base) + (0x007C))
#define DPU_DSC_CLK_EN_ADDR(base) ((base) + (0x0080))
#define DPU_DSC_MEM_CTRL_ADDR(base) ((base) + (0x0084))
#define DPU_DSC_ST_DATAIN_ADDR(base) ((base) + (0x0088))
#define DPU_DSC_ST_DATAOUT_ADDR(base) ((base) + (0x008C))
#define DPU_DSC_ST_SLC_POS_ADDR(base) ((base) + (0x0090))
#define DPU_DSC_ST_PIC_POS_ADDR(base) ((base) + (0x0098))
#define DPU_DSC_ST_FIFO_ADDR(base) ((base) + (0x00A0))
#define DPU_DSC_ST_LINEBUF_ADDR(base) ((base) + (0x00A8))
#define DPU_DSC_ST_ITFC_ADDR(base) ((base) + (0x00B0))
#define DPU_DSC_REG_DEBUG_ADDR(base) ((base) + (0x00C0))
#define DPU_DSC_12_ADDR(base) ((base) + (0x00C4))
#define DPU_DSC_CSC_IDC0_ADDR(base) ((base) + (0x00D0))
#define DPU_DSC_CSC_IDC2_ADDR(base) ((base) + (0x00D4))
#define DPU_DSC_CSC_ODC0_ADDR(base) ((base) + (0x00D8))
#define DPU_DSC_CSC_ODC2_ADDR(base) ((base) + (0x00DC))
#define DPU_DSC_CSC_P00_ADDR(base) ((base) + (0x00E0))
#define DPU_DSC_CSC_P01_ADDR(base) ((base) + (0x00E4))
#define DPU_DSC_CSC_P02_ADDR(base) ((base) + (0x00E8))
#define DPU_DSC_CSC_P10_ADDR(base) ((base) + (0x00EC))
#define DPU_DSC_CSC_P11_ADDR(base) ((base) + (0x00F0))
#define DPU_DSC_CSC_P12_ADDR(base) ((base) + (0x00F4))
#define DPU_DSC_CSC_P20_ADDR(base) ((base) + (0x00F8))
#define DPU_DSC_CSC_P21_ADDR(base) ((base) + (0x00FC))
#define DPU_DSC_CSC_P22_ADDR(base) ((base) + (0x0100))
#define DPU_DSC_CSC_MPREC_ADDR(base) ((base) + (0x0104))
#define DPU_DSC_PADD_CTRL_ADDR(base) ((base) + (0x180))
#define DPU_DSC_PADD_SIZE_ADDR(base) ((base) + (0x184))
#define DPU_DSC_PADD_WIDTH_ADDR(base) ((base) + (0x188))
#define DPU_DSC_PADD_DATA_ADDR(base) ((base) + (0x18C))
#define DPU_DSC_TXIP_CTRL_ADDR(base) ((base) + (0x01B0))
#define DPU_DSC_TXIP_SIZE_ADDR(base) ((base) + (0x01B4))
#define DPU_DSC_TXIP_COEF0_ADDR(base) ((base) + (0x01B8))
#define DPU_DSC_TXIP_COEF1_ADDR(base) ((base) + (0x01BC))
#define DPU_DSC_TXIP_COEF2_ADDR(base) ((base) + (0x01C0))
#define DPU_DSC_TXIP_COEF3_ADDR(base) ((base) + (0x01C4))
#define DPU_DSC_TXIP_OFFSET0_ADDR(base) ((base) + (0x01C8))
#define DPU_DSC_TXIP_OFFSET1_ADDR(base) ((base) + (0x01CC))
#define DPU_DSC_TXIP_CLIP_ADDR(base) ((base) + (0x01D0))
#define DPU_DSC_DATAPACK_CTRL_ADDR(base) ((base) + (0x0200))
#define DPU_DSC_DATAPACK_SIZE_ADDR(base) ((base) + (0x0204))
#define DPU_DSC_RD_SHADOW_ADDR(base) ((base) + (0x0240))
#define DPU_DSC_REG_DEFAULT_ADDR(base) ((base) + (0x0244))
#define DPU_DSC_TOP_CLK_SEL_ADDR(base) ((base) + (0x0248))
#define DPU_DSC_TOP_CLK_EN_ADDR(base) ((base) + (0x024C))
#define DPU_DSC_REG_CTRL_ADDR(base) ((base) + (0x0250))
#define DPU_DSC_REG_CTRL_FLUSH_EN_ADDR(base) ((base) + (0x0254))
#define DPU_DSC_REG_CTRL_DEBUG_ADDR(base) ((base) + (0x0258))
#define DPU_DSC_REG_CTRL_STATE_ADDR(base) ((base) + (0x025C))
#define DPU_DSC_R_LB_DEBUG0_ADDR(base) ((base) + (0x0260))
#define DPU_DSC_R_LB_DEBUG1_ADDR(base) ((base) + (0x0264))
#define DPU_DSC_R_LB_DEBUG2_ADDR(base) ((base) + (0x0268))
#define DPU_DSC_R_LB_DEBUG3_ADDR(base) ((base) + (0x026C))
#define DPU_DSC_W_LB_DEBUG0_ADDR(base) ((base) + (0x0270))
#define DPU_DSC_W_LB_DEBUG1_ADDR(base) ((base) + (0x0274))
#define DPU_DSC_W_LB_DEBUG2_ADDR(base) ((base) + (0x0278))
#define DPU_DSC_W_LB_DEBUG3_ADDR(base) ((base) + (0x027C))
#define DPU_DSC_REG_CTRL_LAYER_ID_ADDR(base) ((base) + (0x0280))
#define DPU_ITF_CH_IMG_SIZE_ADDR(base) ((base) + (0x0000))
#define DPU_ITF_CH_DPP_CLIP_TOP_ADDR(base) ((base) + (0x0004))
#define DPU_ITF_CH_DPP_CLIP_BOTTOM_ADDR(base) ((base) + (0x0008))
#define DPU_ITF_CH_DPP_CLIP_LEFT_ADDR(base) ((base) + (0x000C))
#define DPU_ITF_CH_DPP_CLIP_RIGHT_ADDR(base) ((base) + (0x0010))
#define DPU_ITF_CH_DPP_CLIP_EN_ADDR(base) ((base) + (0x0014))
#define DPU_ITF_CH_DPP_CLIP_DBG_ADDR(base) ((base) + (0x0018))
#define DPU_ITF_CH_WB_UNPACK_POST_CLIP_DISP_SIZE_ADDR(base) ((base) + (0x001C))
#define DPU_ITF_CH_WB_UNPACK_POST_CLIP_CTL_HRZ_ADDR(base) ((base) + (0x0020))
#define DPU_ITF_CH_WB_UNPACK_EN_ADDR(base) ((base) + (0x0024))
#define DPU_ITF_CH_WB_UNPACK_SIZE_ADDR(base) ((base) + (0x0028))
#define DPU_ITF_CH_WB_UNPACK_IMG_FMT_ADDR(base) ((base) + (0x002C))
#define DPU_ITF_CH_DPP_CLRBAR_CTRL_ADDR(base) ((base) + (0x0030))
#define DPU_ITF_CH_DPP_CLRBAR_1ST_CLR_ADDR(base) ((base) + (0x0034))
#define DPU_ITF_CH_DPP_CLRBAR_2ND_CLR_ADDR(base) ((base) + (0x0038))
#define DPU_ITF_CH_DPP_CLRBAR_3RD_CLR_ADDR(base) ((base) + (0x003C))
#define DPU_ITF_CH_ITFSW_RD_SHADOW_ADDR(base) ((base) + (0x0040))
#define DPU_ITF_CH_HIDIC_MODE_ADDR(base) ((base) + (0x0044))
#define DPU_ITF_CH_HIDIC_DIMENSION_ADDR(base) ((base) + (0x0048))
#define DPU_ITF_CH_HIDIC_CMP_RESERVED_ADDR(base) ((base) + (0x004C))
#define DPU_ITF_CH_HIDIC_DBG_OUT_HI_ADDR(base) ((base) + (0x0050))
#define DPU_ITF_CH_HIDIC_DBG_OUT_LO_ADDR(base) ((base) + (0x0054))
#define DPU_ITF_CH_REG_CTRL_ADDR(base) ((base) + (0x0058))
#define DPU_ITF_CH_REG_CTRL_FLUSH_EN_ADDR(base) ((base) + (0x005C))
#define DPU_ITF_CH_REG_CTRL_DEBUG_ADDR(base) ((base) + (0x0060))
#define DPU_ITF_CH_REG_CTRL_STATE_ADDR(base) ((base) + (0x0064))
#define DPU_ITF_CH_ITFSW_DATA_SEL_ADDR(base) ((base) + (0x0068))
#define DPU_ITF_CH_CLRBAR_START_ADDR(base) ((base) + (0x006C))
#define DPU_ITF_CH_ITFCH_CLK_CTL_ADDR(base) ((base) + (0x0070))
#define DPU_ITF_CH_VSYNC_DELAY_CNT_CTRL_ADDR(base) ((base) + (0x0074))
#define DPU_ITF_CH_VSYNC_DELAY_CNT_ADDR(base) ((base) + (0x0078))
#define DPU_ITF_CH_R_LB_DEBUG0_ADDR(base) ((base) + (0x0080))
#define DPU_ITF_CH_R_LB_DEBUG1_ADDR(base) ((base) + (0x0084))
#define DPU_ITF_CH_R_LB_DEBUG2_ADDR(base) ((base) + (0x0088))
#define DPU_ITF_CH_R_LB_DEBUG3_ADDR(base) ((base) + (0x008C))
#define DPU_ITF_CH_REG_DEFAULT_ADDR(base) ((base) + (0x0090))
#define DPU_ITF_CH_REG_CTRL_LAYER_ID_ADDR(base) ((base) + (0x0094))
#define DPU_ITF_CH_VSYNC_TIMESTAMP_H_ADDR(base) ((base) + (0x0098))
#define DPU_ITF_CH_VSYNC_TIMESTAMP_L_ADDR(base) ((base) + (0x009C))
#define DPU_PIPE_SW_SIG_CTRL_0_ADDR(base) ((base) + (0x0000))
#define DPU_PIPE_SW_SW_POS_CTRL_SIG_EN_0_ADDR(base) ((base) + (0x0004))
#define DPU_PIPE_SW_DAT_CTRL_0_ADDR(base) ((base) + (0x0008))
#define DPU_PIPE_SW_SW_POS_CTRL_DAT_EN_0_ADDR(base) ((base) + (0x000C))
#define DPU_PIPE_SW_NXT_SW_NO_PR_0_ADDR(base) ((base) + (0x0010))
#define DPU_PIPE_SW_CLK_SEL_0_ADDR(base) ((base) + (0x0014))
#define DPU_PIPE_SW_SECU_GATE_0_ADDR(base) ((base) + (0x0018))
#define DPU_PIPE_SW_SIG_CTRL_1_ADDR(base) ((base) + (0x001C))
#define DPU_PIPE_SW_SW_POS_CTRL_SIG_EN_1_ADDR(base) ((base) + (0x0020))
#define DPU_PIPE_SW_DAT_CTRL_1_ADDR(base) ((base) + (0x0024))
#define DPU_PIPE_SW_SW_POS_CTRL_DAT_EN_1_ADDR(base) ((base) + (0x0028))
#define DPU_PIPE_SW_NXT_SW_NO_PR_1_ADDR(base) ((base) + (0x002C))
#define DPU_PIPE_SW_CLK_SEL_1_ADDR(base) ((base) + (0x0030))
#define DPU_PIPE_SW_SECU_GATE_1_ADDR(base) ((base) + (0x0034))
#define DPU_PIPE_SW_SIG_CTRL_2_ADDR(base) ((base) + (0x0038))
#define DPU_PIPE_SW_SW_POS_CTRL_SIG_EN_2_ADDR(base) ((base) + (0x003C))
#define DPU_PIPE_SW_DAT_CTRL_2_ADDR(base) ((base) + (0x0040))
#define DPU_PIPE_SW_SW_POS_CTRL_DAT_EN_2_ADDR(base) ((base) + (0x0044))
#define DPU_PIPE_SW_NXT_SW_NO_PR_2_ADDR(base) ((base) + (0x0048))
#define DPU_PIPE_SW_CLK_SEL_2_ADDR(base) ((base) + (0x004C))
#define DPU_PIPE_SW_SECU_GATE_2_ADDR(base) ((base) + (0x0050))
#define DPU_PIPE_SW_SIG_CTRL_3_ADDR(base) ((base) + (0x0054))
#define DPU_PIPE_SW_SW_POS_CTRL_SIG_EN_3_ADDR(base) ((base) + (0x0058))
#define DPU_PIPE_SW_DAT_CTRL_3_ADDR(base) ((base) + (0x005C))
#define DPU_PIPE_SW_SW_POS_CTRL_DAT_EN_3_ADDR(base) ((base) + (0x0060))
#define DPU_PIPE_SW_NXT_SW_NO_PR_3_ADDR(base) ((base) + (0x0064))
#define DPU_PIPE_SW_CLK_SEL_3_ADDR(base) ((base) + (0x0068))
#define DPU_PIPE_SW_SECU_GATE_3_ADDR(base) ((base) + (0x006C))
#define DPU_PIPE_SW_SIG_CTRL_4_ADDR(base) ((base) + (0x0070))
#define DPU_PIPE_SW_SW_POS_CTRL_SIG_EN_4_ADDR(base) ((base) + (0x0074))
#define DPU_PIPE_SW_DAT_CTRL_4_ADDR(base) ((base) + (0x0078))
#define DPU_PIPE_SW_SW_POS_CTRL_DAT_EN_4_ADDR(base) ((base) + (0x007C))
#define DPU_PIPE_SW_NXT_SW_NO_PR_4_ADDR(base) ((base) + (0x0080))
#define DPU_PIPE_SW_CLK_SEL_4_ADDR(base) ((base) + (0x0084))
#define DPU_PIPE_SW_SECU_GATE_4_ADDR(base) ((base) + (0x0088))
#define DPU_PIPE_SW_SPLIT_HSIZE_ADDR(base) ((base) + (0x008C))
#define DPU_PIPE_SW_SPLIT_CTL_ADDR(base) ((base) + (0x0090))
#define DPU_PIPE_SW_TVPACK_SIZE_IN_0_ADDR(base) ((base) + (0x0094))
#define DPU_PIPE_SW_TVPACK_SIZE_OUT_0_ADDR(base) ((base) + (0x0098))
#define DPU_PIPE_SW_TVPACK_CTL_0_ADDR(base) ((base) + (0x009C))
#define DPU_PIPE_SW_TVPACK_SIZE_IN_2_ADDR(base) ((base) + (0x00A0))
#define DPU_PIPE_SW_TVPACK_SIZE_OUT_2_ADDR(base) ((base) + (0x00A4))
#define DPU_PIPE_SW_TVPACK_CTL_2_ADDR(base) ((base) + (0x00A8))
#define DPU_PIPE_SW_TVPACK_SIZE_IN_3_ADDR(base) ((base) + (0x00AC))
#define DPU_PIPE_SW_TVPACK_SIZE_OUT_3_ADDR(base) ((base) + (0x00B0))
#define DPU_PIPE_SW_TVPACK_CTL_3_ADDR(base) ((base) + (0x00B4))
#define DPU_PIPE_SW_TVPACK_SIZE_IN_4_ADDR(base) ((base) + (0x00B8))
#define DPU_PIPE_SW_TVPACK_SIZE_OUT_4_ADDR(base) ((base) + (0x00BC))
#define DPU_PIPE_SW_TVPACK_CTL_4_ADDR(base) ((base) + (0x00C0))
#define DPU_PIPE_SW_INT_EM_CTL_ADDR(base) ((base) + (0x00C4))
#define DPU_PIPE_SW_INT_EM_TO_IPP_ADDR(base) ((base) + (0x00C8))
#define DPU_PIPE_SW_INT_EM_TO_IPP_MASK_ADDR(base) ((base) + (0x00D0))
#define DPU_PIPE_SW_INT_EM_TO_ISP_ADDR(base) ((base) + (0x00D4))
#define DPU_PIPE_SW_INT_EM_TO_ISP_MASK_ADDR(base) ((base) + (0x00D8))
#define DPU_PIPE_SW_INT_EM_TO_IVP_ADDR(base) ((base) + (0x00DC))
#define DPU_PIPE_SW_INT_EM_TO_IVP_MASK_ADDR(base) ((base) + (0x00E0))
#define DPU_PIPE_SW_INT_EM_TO_SH_ADDR(base) ((base) + (0x00E4))
#define DPU_PIPE_SW_INT_EM_TO_SH_MASK_ADDR(base) ((base) + (0x00E8))
#define DPU_PIPE_SW_RD_SHADOW_ADDR(base) ((base) + (0x00EC))
#define DPU_PIPE_SW_OVERLAP_SIZE_ADDR(base) ((base) + (0x00F0))
#define DPU_PIPE_SW_CLK_SEL_ADDR(base) ((base) + (0x00F4))
#define DPU_PIPE_SW_CLK_EN_ADDR(base) ((base) + (0x00F8))
#define DPU_PIPE_SW_REG_DEFAULT_ADDR(base) ((base) + (0x00FC))
#define DPU_UVSAMP_RD_SHADOW_ADDR(base) ((base) + (0x0000))
#define DPU_UVSAMP_REG_DEFAULT_ADDR(base) ((base) + (0x0004))
#define DPU_UVSAMP_TOP_CLK_SEL_ADDR(base) ((base) + (0x0008))
#define DPU_UVSAMP_TOP_CLK_EN_ADDR(base) ((base) + (0x000C))
#define DPU_UVSAMP_REG_CTRL_ADDR(base) ((base) + (0x0010))
#define DPU_UVSAMP_REG_CTRL_FLUSH_EN_ADDR(base) ((base) + (0x0014))
#define DPU_UVSAMP_REG_CTRL_DEBUG_ADDR(base) ((base) + (0x0018))
#define DPU_UVSAMP_REG_CTRL_STATE_ADDR(base) ((base) + (0x001C))
#define DPU_UVSAMP_REG_CTRL_LAYER_ID_ADDR(base) ((base) + (0x0080))
#define DPU_UVSAMP_R_LB_DEBUG0_ADDR(base) ((base) + (0x0090))
#define DPU_UVSAMP_R_LB_DEBUG1_ADDR(base) ((base) + (0x0094))
#define DPU_UVSAMP_R_LB_DEBUG2_ADDR(base) ((base) + (0x0098))
#define DPU_UVSAMP_R_LB_DEBUG3_ADDR(base) ((base) + (0x009C))
#define DPU_UVSAMP_W_LB_DEBUG0_ADDR(base) ((base) + (0x00A0))
#define DPU_UVSAMP_W_LB_DEBUG1_ADDR(base) ((base) + (0x00A4))
#define DPU_UVSAMP_W_LB_DEBUG2_ADDR(base) ((base) + (0x00A8))
#define DPU_UVSAMP_W_LB_DEBUG3_ADDR(base) ((base) + (0x00AC))
#define DPU_UVSAMP_CSC_IDC0_ADDR(base) ((base) + (0x0020))
#define DPU_UVSAMP_CSC_IDC2_ADDR(base) ((base) + (0x0024))
#define DPU_UVSAMP_CSC_ODC0_ADDR(base) ((base) + (0x0028))
#define DPU_UVSAMP_CSC_ODC2_ADDR(base) ((base) + (0x002C))
#define DPU_UVSAMP_CSC_P00_ADDR(base) ((base) + (0x0030))
#define DPU_UVSAMP_CSC_P01_ADDR(base) ((base) + (0x0034))
#define DPU_UVSAMP_CSC_P02_ADDR(base) ((base) + (0x0038))
#define DPU_UVSAMP_CSC_P10_ADDR(base) ((base) + (0x003C))
#define DPU_UVSAMP_CSC_P11_ADDR(base) ((base) + (0x0040))
#define DPU_UVSAMP_CSC_P12_ADDR(base) ((base) + (0x0044))
#define DPU_UVSAMP_CSC_P20_ADDR(base) ((base) + (0x0048))
#define DPU_UVSAMP_CSC_P21_ADDR(base) ((base) + (0x004C))
#define DPU_UVSAMP_CSC_P22_ADDR(base) ((base) + (0x0050))
#define DPU_UVSAMP_CSC_ICG_MODULE_ADDR(base) ((base) + (0x0054))
#define DPU_UVSAMP_CSC_MPREC_ADDR(base) ((base) + (0x0058))
#define DPU_UVSAMP_MEM_CTRL_ADDR(base) ((base) + (0x0060))
#define DPU_UVSAMP_SIZE_ADDR(base) ((base) + (0x0064))
#define DPU_UVSAMP_CTRL_ADDR(base) ((base) + (0x0068))
#define DPU_UVSAMP_COEF_0_ADDR(base) ((base) + (0x006C))
#define DPU_UVSAMP_COEF_1_ADDR(base) ((base) + (0x0070))
#define DPU_UVSAMP_COEF_2_ADDR(base) ((base) + (0x0074))
#define DPU_UVSAMP_COEF_3_ADDR(base) ((base) + (0x0078))
#define DPU_UVSAMP_DEBUG_ADDR(base) ((base) + (0x007C))
#define DPU_DSI_PERIP_CHAR_CTRL_ADDR(base) ((base) + (0x0000))
#define DPU_DSI_APB_READ_VCID_ADDR(base) ((base) + (0x0004))
#define DPU_DSI_MODE_CTRL_ADDR(base) ((base) + (0x0008))
#define DPU_DSI_APB_WR_LP_HDR_ADDR(base) ((base) + (0x000C))
#define DPU_DSI_APB_WR_LP_PLD_DATA_ADDR(base) ((base) + (0x0010))
#define DPU_DSI_CMD_PLD_BUF_STATUS_ADDR(base) ((base) + (0x0014))
#define DPU_DSI_VID_3D_CTRL_ADDR(base) ((base) + (0x0018))
#define DPU_DSI_VER_ADDR(base) ((base) + (0x001C))
#define DPU_DSI_POR_CTRL_ADDR(base) ((base) + (0x0020))
#define DPU_DSI_CLK_DIV_CTRL_ADDR(base) ((base) + (0x0024))
#define DPU_DSI_VIDEO_VCID_ADDR(base) ((base) + (0x0030))
#define DPU_DSI_VIDEO_COLOR_FORMAT_ADDR(base) ((base) + (0x0034))
#define DPU_DSI_VIDEO_POL_CTRL_ADDR(base) ((base) + (0x0038))
#define DPU_DSI_VIDEO_MODE_LP_CMD_TIMING_ADDR(base) ((base) + (0x003C))
#define DPU_DSI_EDPI_CMD_SIZE_ADDR(base) ((base) + (0x0040))
#define DPU_DSI_CMD_MODE_CTRL_ADDR(base) ((base) + (0x0044))
#define DPU_DSI_VIDEO_MODE_CTRL_ADDR(base) ((base) + (0x0050))
#define DPU_DSI_VIDEO_PKT_LEN_ADDR(base) ((base) + (0x0054))
#define DPU_DSI_VIDEO_CHUNK_NUM_ADDR(base) ((base) + (0x0058))
#define DPU_DSI_VIDEO_NULL_SIZE_ADDR(base) ((base) + (0x005C))
#define DPU_DSI_VIDEO_HSA_NUM_ADDR(base) ((base) + (0x0060))
#define DPU_DSI_VIDEO_HBP_NUM_ADDR(base) ((base) + (0x0064))
#define DPU_DSI_VIDEO_HLINE_NUM_ADDR(base) ((base) + (0x0068))
#define DPU_DSI_VIDEO_VSA_NUM_ADDR(base) ((base) + (0x006C))
#define DPU_DSI_VIDEO_VBP_NUM_ADDR(base) ((base) + (0x0070))
#define DPU_DSI_VIDEO_VFP_NUM_ADDR(base) ((base) + (0x0074))
#define DPU_DSI_VIDEO_VACT_NUM_ADDR(base) ((base) + (0x0078))
#define DPU_DSI_LP_CLK_CTRL_ADDR(base) ((base) + (0x0080))
#define DPU_DSI_CLKLANE_TRANS_TIME_ADDR(base) ((base) + (0x0084))
#define DPU_DSI_DATALANE_TRNAS_TIME_ADDR(base) ((base) + (0x0088))
#define DPU_DSI_CDPHY_RST_CTRL_ADDR(base) ((base) + (0x008C))
#define DPU_DSI_CDPHY_LANE_NUM_ADDR(base) ((base) + (0x0090))
#define DPU_DSI_CDPHY_ULPS_CTRL_ADDR(base) ((base) + (0x0094))
#define DPU_DSI_CDPHY_TX_TRIG_ADDR(base) ((base) + (0x0098))
#define DPU_DSI_CDPHY_STATUS_ADDR(base) ((base) + (0x009C))
#define DPU_DSI_CDPHY_TEST_CTRL_0_ADDR(base) ((base) + (0x00A0))
#define DPU_DSI_CDPHY_TEST_CTRL_1_ADDR(base) ((base) + (0x00A4))
#define DPU_DSI_ERROR_STATUS_0_ADDR(base) ((base) + (0x00A8))
#define DPU_DSI_ERROR_STATUS_1_ADDR(base) ((base) + (0x00AC))
#define DPU_DSI_ERROR_MASK_0_ADDR(base) ((base) + (0x00B0))
#define DPU_DSI_ERROR_MASK_1_ADDR(base) ((base) + (0x00B4))
#define DPU_DSI_CDPHY_CAL_EN_ADDR(base) ((base) + (0x00B8))
#define DPU_DSI_INT_ERROR_FORCE0_ADDR(base) ((base) + (0x00BC))
#define DPU_DSI_INT_ERROR_FORCE1_ADDR(base) ((base) + (0x00C0))
#define DPU_DSI_AUTO_ULPS_MODE_ADDR(base) ((base) + (0x00C4))
#define DPU_DSI_AUTO_ULPS_ENTRY_DELAY_ADDR(base) ((base) + (0x00C8))
#define DPU_DSI_AUTO_ULPS_WAKEUP_TIME_ADDR(base) ((base) + (0x00CC))
#define DPU_DSI_DSC_PARA_SET_ADDR(base) ((base) + (0x00D0))
#define DPU_DSI_CDPHY_MAX_RD_TIME_ADDR(base) ((base) + (0x00D4))
#define DPU_DSI_AUTO_ULPS_MIN_TIME_ADDR(base) ((base) + (0x00D8))
#define DPU_DSI_CPHY_OR_DPHY_ADDR(base) ((base) + (0x00DC))
#define DPU_DSI_SHADOW_REG_CTRL_ADDR(base) ((base) + (0x00E0))
#define DPU_DSI_VIDEO_VCID_ACT_ADDR(base) ((base) + (0x00E4))
#define DPU_DSI_VIDEO_COLOR_FORMAT_ACT_ADDR(base) ((base) + (0x00E8))
#define DPU_DSI_VIDEO_MODE_LP_CMD_TIMING_ACT_ADDR(base) ((base) + (0x00EC))
#define DPU_DSI_TO_TIME_CTRL_ADDR(base) ((base) + (0x00F0))
#define DPU_DSI_HS_RD_TO_TIME_CTRL_ADDR(base) ((base) + (0x00F4))
#define DPU_DSI_LP_RD_TO_TIME_CTRL_ADDR(base) ((base) + (0x00F8))
#define DPU_DSI_HS_WR_TO_TIME_CTRL_ADDR(base) ((base) + (0x00FC))
#define DPU_DSI_LP_WR_TO_TIME_CTRL_ADDR(base) ((base) + (0x0100))
#define DPU_DSI_BTA_TO_TIME_CTRL_ADDR(base) ((base) + (0x0104))
#define DPU_DSI_VIDEO_MODE_CTRL_ACT_ADDR(base) ((base) + (0x0110))
#define DPU_DSI_VIDEO_PKT_LEN_ACT_ADDR(base) ((base) + (0x0114))
#define DPU_DSI_VIDEO_CHUNK_NUM_ACT_ADDR(base) ((base) + (0x0118))
#define DPU_DSI_VIDEO_NULL_SIZE_ACT_ADDR(base) ((base) + (0x011C))
#define DPU_DSI_VIDEO_HSA_NUM_ACT_ADDR(base) ((base) + (0x0120))
#define DPU_DSI_VIDEO_HBP_NUM_ACT_ADDR(base) ((base) + (0x0124))
#define DPU_DSI_VIDEO_HLINE_NUM_ACT_ADDR(base) ((base) + (0x0128))
#define DPU_DSI_VIDEO_VSA_NUM_ACT_ADDR(base) ((base) + (0x012C))
#define DPU_DSI_VIDEO_VBP_NUM_ACT_ADDR(base) ((base) + (0x0130))
#define DPU_DSI_VIDEO_VFP_NUM_ACT_ADDR(base) ((base) + (0x0134))
#define DPU_DSI_VIDEO_VACT_NUM_ACT_ADDR(base) ((base) + (0x0138))
#define DPU_DSI_VIDEO_3D_CTRL_ACT_ADDR(base) ((base) + (0x013C))
#define DPU_DSI_PPROT_CFG_ADDR(base) ((base) + (0x0140))
#define DPU_DSI_MEM_CTRL_ADDR(base) ((base) + (0x0194))
#define DPU_DSI_PM_CTRL_ADDR(base) ((base) + (0x0198))
#define DPU_DSI_DBG0_ADDR(base) ((base) + (0x019C))
#define DPU_DSI_PHY_PLL_START_TIME_ADDR(base) ((base) + (0x01A0))
#define DPU_DSI_DBG_CRC_CTRL_ADDR(base) ((base) + (0x01A4))
#define DPU_DSI_DBG_CRC_VAL_ADDR(base) ((base) + (0x01A8))
#define DPU_DSI_SECU_CFG_EN_ADDR(base) ((base) + (0x01AC))
#define DPU_DSI_LDI_PLR_CTRL_ADDR(base) ((base) + (0x01B0))
#define DPU_DSI_LDI_3D_CTRL_ADDR(base) ((base) + (0x01B4))
#define DPU_DSI_LDI_CTRL_ADDR(base) ((base) + (0x01B8))
#define DPU_DSI_LDI_DE_SPACE_LOW_ADDR(base) ((base) + (0x01BC))
#define DPU_DSI_CMD_MOD_CTRL_ADDR(base) ((base) + (0x01C0))
#define DPU_DSI_TE_CTRL_ADDR(base) ((base) + (0x01C4))
#define DPU_DSI_TE_HS_NUM_ADDR(base) ((base) + (0x01C8))
#define DPU_DSI_TE_HS_WD_ADDR(base) ((base) + (0x01CC))
#define DPU_DSI_TE_VS_WD_ADDR(base) ((base) + (0x01D0))
#define DPU_DSI_LDI_FRM_MSK_ADDR(base) ((base) + (0x01D4))
#define DPU_DSI_FRM_VALID_DBG_ADDR(base) ((base) + (0x01D8))
#define DPU_DSI_LDI_VINACT_CNT_ADDR(base) ((base) + (0x01DC))
#define DPU_DSI_VSYNC_DELAY_CTRL_ADDR(base) ((base) + (0x01E0))
#define DPU_DSI_VSYNC_DELAY_TIME_ADDR(base) ((base) + (0x01E4))
#define DPU_DSI_RGB_PAT_ADDR(base) ((base) + (0x01E8))
#define DPU_DSI_LDI_DPI0_HRZ_CTRL2_ADDR(base) ((base) + (0x01EC))
#define DPU_DSI_LDI_VRT_CTRL2_ADDR(base) ((base) + (0x01F0))
#define DPU_DSI_LDI_DPI0_HRZ_CTRL3_ADDR(base) ((base) + (0x01F4))
#define DPU_DSI_DSS_SH_MASK_INT_ADDR(base) ((base) + (0x01F8))
#define DPU_DSI_VSTATE_ADDR(base) ((base) + (0x01FC))
#define DPU_DSI_DPI0_HSTATE_ADDR(base) ((base) + (0x0200))
#define DPU_DSI_VCOUNT_ADDR(base) ((base) + (0x0204))
#define DPU_DSI_DPI0_HCOUNT_ADDR(base) ((base) + (0x0208))
#define DPU_DSI_LDI_VINACT_CNT_ACT_ADDR(base) ((base) + (0x020C))
#define DPU_DSI_VSYNC_DELAY_TIME_ACT_ADDR(base) ((base) + (0x0210))
#define DPU_DSI_LDI_DPI0_HRZ_CTRL2_ACT_ADDR(base) ((base) + (0x0214))
#define DPU_DSI_LDI_VRT_CTRL2_ACT_ADDR(base) ((base) + (0x0218))
#define DPU_DSI_LDI_DPI0_HRZ_CTRL3_ACT_ADDR(base) ((base) + (0x021C))
#define DPU_DSI_RGB_PAT_ACT_ADDR(base) ((base) + (0x0220))
#define DPU_DSI_DPHYTX_STOPSNT_ADDR(base) ((base) + (0x224))
#define DPU_DSI_DPHYTX_CTRL_ADDR(base) ((base) + (0x228))
#define DPU_DSI_DPHYTX_TRSTOP_FLAG_ADDR(base) ((base) + (0x22C))
#define DPU_DSI_DPHYTX_STATUS_ADDR(base) ((base) + (0x230))
#define DPU_DSI_LDI_CMD_EVENT_SEL_ADDR(base) ((base) + (0x0234))
#define DPU_DSI_LDI_FRM_MSK_UP_ADDR(base) ((base) + (0x0238))
#define DPU_DSI_MCU_ITF_INTS_ADDR(base) ((base) + (0x240))
#define DPU_DSI_MCU_ITF_INT_MSK_ADDR(base) ((base) + (0x244))
#define DPU_DSI_CPU_ITF_INTS_ADDR(base) ((base) + (0x0248))
#define DPU_DSI_CPU_ITF_INT_MSK_ADDR(base) ((base) + (0x024C))
#define DPU_DSI_PMM_ITF_INTS_ADDR(base) ((base) + (0x0250))
#define DPU_DSI_PMM_ITF_INT_MSK_ADDR(base) ((base) + (0x0254))
#define DPU_DSI_RGB_PAT2_ADDR(base) ((base) + (0x0258))
#define DPU_DSI_RGB_PAT2_ACT_ADDR(base) ((base) + (0x025C))
#define DPU_DSI_VSYNC_DELAY_MODE2_CTRL_ADDR(base) ((base) + (0x0260))
#define DPU_DSI_VSYNC_DELAY_MODE2_DELAY_LINE_ADDR(base) ((base) + (0x0264))
#define DPU_DSI_VSYNC_DELAY_MODE2_DELAY_LINE_ACT_ADDR(base) ((base) + (0x0268))
#define DPU_DSI_VSYNC_DELAY_MODE3_CTRL_ADDR(base) ((base) + (0x026C))
#define DPU_DSI_VSYNC_DELAY_MODE3_LINE_NUM_ADDR(base) ((base) + (0x0270))
#define DPU_DSI_VSYNC_DELAY_MODE3_LINE_NUM_ACT_ADDR(base) ((base) + (0x0274))
#define DPU_DSI_VSYNC_DELAY_MODE4_CTRL_ADDR(base) ((base) + (0x0278))
#define DPU_DSI_MODE3_EM_SYNC_FSM_CTRL_ADDR(base) ((base) + (0x027C))
#define DPU_DSI_MODE3_EM_SYNC_QUIT_CTRL_ADDR(base) ((base) + (0x0280))
#define DPU_DSI_EM_CYCLE_CNT_ADDR(base) ((base) + (0x0284))
#define DPU_DSI_EM_CYCLE_CNT_REMAINDER_ADDR(base) ((base) + (0x0288))
#define DPU_DSI_SOC_WAKE_PRE_CNT_ADDR(base) ((base) + (0x028C))
#define DPU_DSI_EM_QUIT_CYCLE_NUM_0_ADDR(base) ((base) + (0x0290))
#define DPU_DSI_EM_QUIT_CYCLE_NUM_1_ADDR(base) ((base) + (0x0294))
#define DPU_DSI_EM_QUIT_CYCLE_NUM_2_ADDR(base) ((base) + (0x0298))
#define DPU_DSI_EM_QUIT_CYCLE_NUM_3_ADDR(base) ((base) + (0x029C))
#define DPU_DSI_EM_QUIT_CYCLE_NUM_4_ADDR(base) ((base) + (0x02A0))
#define DPU_DSI_EM_QUIT_CYCLE_NUM_5_ADDR(base) ((base) + (0x02A4))
#define DPU_DSI_EM_QUIT_CYCLE_NUM_6_ADDR(base) ((base) + (0x02A8))
#define DPU_DSI_EM_QUIT_CYCLE_NUM_7_ADDR(base) ((base) + (0x02AC))
#define DPU_DSI_EM_QUIT_VALID_CTRL_ADDR(base) ((base) + (0x02B0))
#define DPU_DSI_EM_CYCLE_CNT_ACT_ADDR(base) ((base) + (0x02B4))
#define DPU_DSI_EM_CYCLE_CNT_REMAINDER_ACT_ADDR(base) ((base) + (0x02B8))
#define DPU_DSI_SOC_WAKE_PRE_CNT_ACT_ADDR(base) ((base) + (0x02BC))
#define DPU_DSI_EM_QUIT_CYCLE_NUM_0_ACT_ADDR(base) ((base) + (0x02C0))
#define DPU_DSI_EM_QUIT_CYCLE_NUM_1_ACT_ADDR(base) ((base) + (0x02C4))
#define DPU_DSI_EM_QUIT_CYCLE_NUM_2_ACT_ADDR(base) ((base) + (0x02C8))
#define DPU_DSI_EM_QUIT_CYCLE_NUM_3_ACT_ADDR(base) ((base) + (0x02CC))
#define DPU_DSI_EM_QUIT_CYCLE_NUM_4_ACT_ADDR(base) ((base) + (0x02D0))
#define DPU_DSI_EM_QUIT_CYCLE_NUM_5_ACT_ADDR(base) ((base) + (0x02D4))
#define DPU_DSI_EM_QUIT_CYCLE_NUM_6_ACT_ADDR(base) ((base) + (0x02D8))
#define DPU_DSI_EM_QUIT_CYCLE_NUM_7_ACT_ADDR(base) ((base) + (0x02DC))
#define DPU_DSI_EM_QUIT_VALID_CTRL_ACT_ADDR(base) ((base) + (0x02E0))
#define DPU_DSI_EM_QUIT_CNT_ADDR(base) ((base) + (0x02E4))
#define DPU_DSI_EM_SYNC_REG_CTRL_ADDR(base) ((base) + (0x02E8))
#define DPU_DSI_MODE3_EM_SYNC_FSM_CTRL_ACT_ADDR(base) ((base) + (0x02EC))
#define DPU_DSI_VSYNC_DELAY_MODE4_LINE_NUM_ADDR(base) ((base) + (0x02F0))
#define DPU_DSI_VSYNC_DELAY_MODE4_LINE_NUM_ACT_ADDR(base) ((base) + (0x02F4))
#define DPU_DSI_EM_SYNC_STATE_CUR_ADDR(base) ((base) + (0x02F8))
#define DPU_DSI_EM_SYNC_CNT_ADDR(base) ((base) + (0x02FC))
#define DPU_DSI_EM_SYNC_CNT_LT_ADDR(base) ((base) + (0x0300))
#define DPU_DSI_EM_QUIT_SRC_SEL_ADDR(base) ((base) + (0x0304))
#define DPU_DSI_CDPHY_SHUTDOWN_TIMING_CTRL_ADDR(base) ((base) + (0x0308))
#define DPU_DSI_CDPHY_WAKEUP_TIMING_CTRL_ADDR(base) ((base) + (0x030C))
#define DPU_DSI_CDPHY_TIMING_CTRL_EN_ADDR(base) ((base) + (0x0310))
#define DPU_DSI_GEN_HP_HDR_ADDR(base) ((base) + (0x0314))
#define DPU_DSI_GEN_HP_PLD_DATA_ADDR(base) ((base) + (0x0318))
#define DPU_DSI_HP_CMD_FIFO_CTRL_ADDR(base) ((base) + (0x031C))
#define DPU_DSI_HP_CMD_FIFO_STATUS_ADDR(base) ((base) + (0x0320))
#define DPU_DSI_HP_PLD_FIFO_STATUS_ADDR(base) ((base) + (0x0324))
#define DPU_DSI_LP_CMD_FIFO_CTRL_ADDR(base) ((base) + (0x0328))
#define DPU_DSI_LP_CMD_FIFO_STATUS_ADDR(base) ((base) + (0x032C))
#define DPU_DSI_LP_PLD_FIFO_STATUS_ADDR(base) ((base) + (0x0330))
#define DPU_DSI_FRM_VALID_DBG2_ADDR(base) ((base) + (0x0334))
#define DPU_DSI_IOMCU_ITF_INT_ADDR(base) ((base) + (0x338))
#define DPU_DSI_IOMCU_ITF_INT_MASK_ADDR(base) ((base) + (0x33C))
#define DPU_DSI_ULPS_CLKLANE_DELAY_CNT_ADDR(base) ((base) + (0x0340))
#define DPU_DSI_TE_CTRL_2_ADDR(base) ((base) + (0x0344))
#define DPU_DSI_TE_HS_NUM_2_ADDR(base) ((base) + (0x0348))
#define DPU_DSI_TE_HS_WD_2_ADDR(base) ((base) + (0x034C))
#define DPU_DSI_TE_VS_WD_2_ADDR(base) ((base) + (0x0350))
#define DPU_DSI_MODE3_EXTRA_CTRL_ADDR(base) ((base) + (0x0354))
#define DPU_DSI_HP_CMD_PKT_STATUS_ADDR(base) ((base) + (0x0358))
#define DPU_DSI_VBP_LINE_LEFT_SETTING_ADDR(base) ((base) + (0x035C))
#define DPU_DSI_VFP_LINE_LEFT_SETTING_ADDR(base) ((base) + (0x0360))
#define DPU_DSI_CMD_HS_PKT_DISCONTIN_CTRL_ADDR(base) ((base) + (0x0364))
#define DPU_DSI_TE_CTRL_3_ADDR(base) ((base) + (0x0368))
#define DPU_DSI_TE_HS_NUM_3_ADDR(base) ((base) + (0x036C))
#define DPU_DSI_TE_HS_WD_3_ADDR(base) ((base) + (0x0370))
#define DPU_DSI_TE_VS_WD_3_ADDR(base) ((base) + (0x0374))
#define DPU_DSI_GLB_DDSP_CTRL_ADDR(base) ((base) + (0x0000))
#define DPU_DSI_GLB_LANE_CTRL_ADDR(base) ((base) + (0x0004))
#define DPU_DSI_GLB_DSI_RESET_EN_ADDR(base) ((base) + (0x0008))
#define DPU_DSI_GLB_DOZE_DSI_SEL_ADDR(base) ((base) + (0x000C))
#define DPU_DSI_GLB_INTR_SLEEP_IDLE_LPM3_ADDR(base) ((base) + (0x0010))
#define DPU_DSI_GLB_INTR_SLEEP_IDLE_LPM3_MASK_ADDR(base) ((base) + (0x0014))
#define DPU_DSI_GLB_TP_SEL_ADDR(base) ((base) + (0x0018))
#define DPU_DSI_GLB_GEN_DUAL_HDR_ADDR(base) ((base) + (0x001C))
#define DPU_DSI_GLB_GEN_DUAL_PLD_DATA_ADDR(base) ((base) + (0x0020))
#define DPU_DSI_GLB_DUAL_REG_WR_SEL_ADDR(base) ((base) + (0x0024))
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int cmdlist_addr_h : 4;
        unsigned int reserved_0 : 4;
        unsigned int cmdlist_next_en : 1;
        unsigned int reserved_1 : 23;
    } reg;
} DPU_DM_CMDLIST_ADDR_H_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int cmdlist_addr0 : 32;
    } reg;
} DPU_DM_CMDLIST_ADDR0_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int cmdlist_addr1 : 32;
    } reg;
} DPU_DM_CMDLIST_ADDR1_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int cmdlist_addr2 : 32;
    } reg;
} DPU_DM_CMDLIST_ADDR2_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int cmdlist_addr3 : 32;
    } reg;
} DPU_DM_CMDLIST_ADDR3_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int cmdlist_addr4 : 32;
    } reg;
} DPU_DM_CMDLIST_ADDR4_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int cmdlist_addr5 : 32;
    } reg;
} DPU_DM_CMDLIST_ADDR5_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int cmdlist_addr6 : 32;
    } reg;
} DPU_DM_CMDLIST_ADDR6_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int cmdlist_addr7 : 32;
    } reg;
} DPU_DM_CMDLIST_ADDR7_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int cmdlist_addr8 : 32;
    } reg;
} DPU_DM_CMDLIST_ADDR8_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int cmdlist_addr9 : 32;
    } reg;
} DPU_DM_CMDLIST_ADDR9_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int cmdlist_addr10 : 32;
    } reg;
} DPU_DM_CMDLIST_ADDR10_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int cmdlist_addr11 : 32;
    } reg;
} DPU_DM_CMDLIST_ADDR11_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int cmdlist_addr12 : 32;
    } reg;
} DPU_DM_CMDLIST_ADDR12_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int cmdlist_addr13 : 32;
    } reg;
} DPU_DM_CMDLIST_ADDR13_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int cmdlist_addr14 : 32;
    } reg;
} DPU_DM_CMDLIST_ADDR14_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int cmdlist_addr15 : 32;
    } reg;
} DPU_DM_CMDLIST_ADDR15_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int scene_id : 8;
        unsigned int hdr_number : 8;
        unsigned int uvup_number : 8;
        unsigned int srot_number : 8;
    } reg;
} DPU_DM_SROT_NUMBER_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int cld_number : 8;
        unsigned int scl_number : 8;
        unsigned int dpp_number : 8;
        unsigned int ddic_number : 8;
    } reg;
} DPU_DM_DDIC_NUMBER_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int dsc_number : 8;
        unsigned int wch_number : 8;
        unsigned int itf_number : 8;
        unsigned int layer_number : 8;
    } reg;
} DPU_DM_LAYER_NUMBER_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int hemcd_number : 8;
        unsigned int scene_mode : 8;
        unsigned int scene_reserved : 16;
    } reg;
} DPU_DM_SCENE_RESERVED_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int ov_img_height : 16;
        unsigned int ov_img_width : 16;
    } reg;
} DPU_DM_OV_IMG_WIDTH_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int ov_blayer_startx : 16;
        unsigned int ov_blayer_endx : 16;
    } reg;
} DPU_DM_OV_BLAYER_ENDX_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int ov_blayer_starty : 16;
        unsigned int ov_blayer_endy : 16;
    } reg;
} DPU_DM_OV_BLAYER_ENDY_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int ov_bg_color_rgb : 32;
    } reg;
} DPU_DM_OV_BG_COLOR_RGB_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int ov_bg_color_cfg : 16;
        unsigned int ov_sel : 8;
        unsigned int ov_order0 : 8;
    } reg;
} DPU_DM_OV_ORDER0_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int ov_order1 : 8;
        unsigned int ov_reserved_0 : 24;
    } reg;
} DPU_DM_OV_RESERVED_0_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int ov_reserved_1 : 32;
    } reg;
} DPU_DM_OV_RESERVED_1_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int ov_reserved_2 : 32;
    } reg;
} DPU_DM_OV_RESERVED_2_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int hdr_input_img_height : 16;
        unsigned int hdr_input_img_width : 16;
    } reg;
} DPU_DM_HDR_INPUT_IMG_WIDTH_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int hdr_output_img_height : 16;
        unsigned int hdr_output_img_width : 16;
    } reg;
} DPU_DM_HDR_OUTPUT_IMG_WIDTH_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int hdr_layer_id : 8;
        unsigned int hdr_order0 : 8;
        unsigned int hdr_sel : 8;
        unsigned int hdr_input_fmt : 8;
    } reg;
} DPU_DM_HDR_INPUT_FMT_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int hdr_output_fmt : 8;
        unsigned int hdr_reserved : 24;
    } reg;
} DPU_DM_HDR_RESERVED_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int uvup_input_img_height : 16;
        unsigned int uvup_input_img_width : 16;
    } reg;
} DPU_DM_UVUP_INPUT_IMG_WIDTH_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int uvup_output_img_height : 16;
        unsigned int uvup_output_img_width : 16;
    } reg;
} DPU_DM_UVUP_OUTPUT_IMG_WIDTH_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int uvup_layer_id : 8;
        unsigned int uvup_order0 : 8;
        unsigned int uvup_sel : 8;
        unsigned int uvup_input_fmt : 8;
    } reg;
} DPU_DM_UVUP_INPUT_FMT_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int uvup_output_fmt : 8;
        unsigned int reserved : 16;
        unsigned int uvup_threshold : 8;
    } reg;
} DPU_DM_UVUP_THRESHOLD_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int srot0_input_img_height : 16;
        unsigned int srot0_input_img_width : 16;
    } reg;
} DPU_DM_SROT0_INPUT_IMG_WIDTH_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int srot0_output_img_height : 16;
        unsigned int srot0_output_img_width : 16;
    } reg;
} DPU_DM_SROT0_OUTPUT_IMG_WIDTH_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int srot0_layer_id : 8;
        unsigned int srot0_order0 : 8;
        unsigned int srot0_sel : 8;
        unsigned int srot0_input_fmt : 8;
    } reg;
} DPU_DM_SROT0_INPUT_FMT_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int srot0_output_fmt : 8;
        unsigned int srot0_reserved : 24;
    } reg;
} DPU_DM_SROT0_RESERVED_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int srot1_input_img_height : 16;
        unsigned int srot1_input_img_width : 16;
    } reg;
} DPU_DM_SROT1_INPUT_IMG_WIDTH_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int srot1_output_img_height : 16;
        unsigned int srot1_output_img_width : 16;
    } reg;
} DPU_DM_SROT1_OUTPUT_IMG_WIDTH_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int srot1_layer_id : 8;
        unsigned int srot1_order0 : 8;
        unsigned int srot1_sel : 8;
        unsigned int srot1_input_fmt : 8;
    } reg;
} DPU_DM_SROT1_INPUT_FMT_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int srot1_output_fmt : 8;
        unsigned int srot1_reserved : 24;
    } reg;
} DPU_DM_SROT1_RESERVED_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int cld0_input_img_height : 16;
        unsigned int cld0_input_img_width : 16;
    } reg;
} DPU_DM_CLD0_INPUT_IMG_WIDTH_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int cld0_output_img_height : 16;
        unsigned int cld0_output_img_width : 16;
    } reg;
} DPU_DM_CLD0_OUTPUT_IMG_WIDTH_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int cld0_layer_id : 8;
        unsigned int cld0_order0 : 8;
        unsigned int cld0_sel : 8;
        unsigned int cld0_input_fmt : 8;
    } reg;
} DPU_DM_CLD0_INPUT_FMT_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int cld0_output_fmt : 8;
        unsigned int cld0_reserved : 24;
    } reg;
} DPU_DM_CLD0_RESERVED_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int cld1_input_img_height : 16;
        unsigned int cld1_input_img_width : 16;
    } reg;
} DPU_DM_CLD1_INPUT_IMG_WIDTH_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int cld1_output_img_height : 16;
        unsigned int cld1_output_img_width : 16;
    } reg;
} DPU_DM_CLD1_OUTPUT_IMG_WIDTH_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int cld1_layer_id : 8;
        unsigned int cld1_order0 : 8;
        unsigned int cld1_sel : 8;
        unsigned int cld1_input_fmt : 8;
    } reg;
} DPU_DM_CLD1_INPUT_FMT_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int cld1_output_fmt : 8;
        unsigned int cld1_reserved : 24;
    } reg;
} DPU_DM_CLD1_RESERVED_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int hemcd_input_pld_size : 32;
    } reg;
} DPU_DM_HEMCD_INPUT_PLD_SIZE_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int hemcd_output_img_width : 16;
        unsigned int hemcd_output_img_height : 16;
    } reg;
} DPU_DM_HEMCD_OUTPUT_IMG_HEIGHT_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int hemcd_layer_id : 8;
        unsigned int hemcd_order0 : 8;
        unsigned int hemcd_sel : 8;
        unsigned int hemcd_input_fmt : 8;
    } reg;
} DPU_DM_HEMCD_INPUT_FMT_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int hemcd_output_fmt : 8;
        unsigned int hemcd_reserved : 24;
    } reg;
} DPU_DM_HEMCD_RESERVED_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int scl0_input_img_height : 16;
        unsigned int scl0_input_img_width : 16;
    } reg;
} DPU_DM_SCL0_INPUT_IMG_WIDTH_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int scl0_output_img_height : 16;
        unsigned int scl0_output_img_width : 16;
    } reg;
} DPU_DM_SCL0_OUTPUT_IMG_WIDTH_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int scl0_layer_id : 8;
        unsigned int scl0_order0 : 8;
        unsigned int scl0_sel : 8;
        unsigned int scl0_type : 8;
    } reg;
} DPU_DM_SCL0_TYPE_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int scl0_pre_post_sel : 8;
        unsigned int scl0_input_fmt : 8;
        unsigned int scl0_output_fmt : 8;
        unsigned int scl0_threshold : 8;
    } reg;
} DPU_DM_SCL0_THRESHOLD_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int scl1_input_img_height : 16;
        unsigned int scl1_input_img_width : 16;
    } reg;
} DPU_DM_SCL1_INPUT_IMG_WIDTH_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int scl1_output_img_height : 16;
        unsigned int scl1_output_img_width : 16;
    } reg;
} DPU_DM_SCL1_OUTPUT_IMG_WIDTH_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int scl1_layer_id : 8;
        unsigned int scl1_order0 : 8;
        unsigned int scl1_sel : 8;
        unsigned int scl1_type : 8;
    } reg;
} DPU_DM_SCL1_TYPE_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int scl1_pre_post_sel : 8;
        unsigned int scl1_input_fmt : 8;
        unsigned int scl1_output_fmt : 8;
        unsigned int scl1_threshold : 8;
    } reg;
} DPU_DM_SCL1_THRESHOLD_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int scl2_input_img_height : 16;
        unsigned int scl2_input_img_width : 16;
    } reg;
} DPU_DM_SCL2_INPUT_IMG_WIDTH_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int scl2_output_img_height : 16;
        unsigned int scl2_output_img_width : 16;
    } reg;
} DPU_DM_SCL2_OUTPUT_IMG_WIDTH_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int scl2_layer_id : 8;
        unsigned int scl2_order0 : 8;
        unsigned int scl2_sel : 8;
        unsigned int scl2_type : 8;
    } reg;
} DPU_DM_SCL2_TYPE_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int scl2_pre_post_sel : 8;
        unsigned int scl2_input_fmt : 8;
        unsigned int scl2_output_fmt : 8;
        unsigned int scl2_threshold : 8;
    } reg;
} DPU_DM_SCL2_THRESHOLD_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int scl3_input_img_height : 16;
        unsigned int scl3_input_img_width : 16;
    } reg;
} DPU_DM_SCL3_INPUT_IMG_WIDTH_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int scl3_output_img_height : 16;
        unsigned int scl3_output_img_width : 16;
    } reg;
} DPU_DM_SCL3_OUTPUT_IMG_WIDTH_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int scl3_layer_id : 8;
        unsigned int scl3_order0 : 8;
        unsigned int scl3_sel : 8;
        unsigned int scl3_type : 8;
    } reg;
} DPU_DM_SCL3_TYPE_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int scl3_pre_post_sel : 8;
        unsigned int scl3_input_fmt : 8;
        unsigned int scl3_output_fmt : 8;
        unsigned int scl3_threshold : 8;
    } reg;
} DPU_DM_SCL3_THRESHOLD_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int scl4_input_img_height : 16;
        unsigned int scl4_input_img_width : 16;
    } reg;
} DPU_DM_SCL4_INPUT_IMG_WIDTH_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int scl4_output_img_height : 16;
        unsigned int scl4_output_img_width : 16;
    } reg;
} DPU_DM_SCL4_OUTPUT_IMG_WIDTH_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int scl4_layer_id : 8;
        unsigned int scl4_order0 : 8;
        unsigned int scl4_sel : 8;
        unsigned int scl4_type : 8;
    } reg;
} DPU_DM_SCL4_TYPE_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int scl4_pre_post_sel : 8;
        unsigned int scl4_input_fmt : 8;
        unsigned int scl4_output_fmt : 8;
        unsigned int scl4_threshold : 8;
    } reg;
} DPU_DM_SCL4_THRESHOLD_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int dpp_input_img_height : 16;
        unsigned int dpp_input_img_width : 16;
    } reg;
} DPU_DM_DPP_INPUT_IMG_WIDTH_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int dpp_output_img_height : 16;
        unsigned int dpp_output_img_width : 16;
    } reg;
} DPU_DM_DPP_OUTPUT_IMG_WIDTH_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int dpp_layer_id : 8;
        unsigned int dpp_order0 : 8;
        unsigned int dpp_order1 : 8;
        unsigned int dpp_sel : 8;
    } reg;
} DPU_DM_DPP_SEL_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int dpp_input_fmt : 8;
        unsigned int dpp_output_fmt : 8;
        unsigned int dpp_reserved : 16;
    } reg;
} DPU_DM_DPP_RESERVED_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int ddic_input_img_height : 16;
        unsigned int ddic_input_img_width : 16;
    } reg;
} DPU_DM_DDIC_INPUT_IMG_WIDTH_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int ddic_output_img_height : 16;
        unsigned int ddic_output_img_width : 16;
    } reg;
} DPU_DM_DDIC_OUTPUT_IMG_WIDTH_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int ddic_layer_id : 8;
        unsigned int ddic_order0 : 8;
        unsigned int ddic_sel : 8;
        unsigned int ddic_input_fmt : 8;
    } reg;
} DPU_DM_DDIC_INPUT_FMT_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int ddic_demura_type : 8;
        unsigned int reserved : 8;
        unsigned int ddic_idle_wb_en : 8;
        unsigned int ddic_deburnin_wb_en : 8;
    } reg;
} DPU_DM_DDIC_DEBURNIN_WB_EN_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int ddic_wb_output_img_height : 16;
        unsigned int ddic_wb_output_img_width : 16;
    } reg;
} DPU_DM_DDIC_WB_OUTPUT_IMG_WIDTH_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int ddic_wb_output_fmt : 8;
        unsigned int ddic_wb_nxt_order : 8;
        unsigned int ddic_output_fmt : 8;
        unsigned int ddic_demura_fmt : 8;
    } reg;
} DPU_DM_DDIC_DEMURA_FMT_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int ddic_demura_lut_height : 16;
        unsigned int ddic_demura_lut_width : 16;
    } reg;
} DPU_DM_DDIC_DEMURA_LUT_WIDTH_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int ddic_demura_lut_layer_id0 : 8;
        unsigned int ddic_demura_lut_layer_id1 : 8;
        unsigned int ddic_reserved_2 : 16;
    } reg;
} DPU_DM_DDIC_RESERVED_2_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int dsc_input_img_height : 16;
        unsigned int dsc_input_img_width : 16;
    } reg;
} DPU_DM_DSC_INPUT_IMG_WIDTH_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int dsc_output_img_height : 16;
        unsigned int dsc_output_img_width : 16;
    } reg;
} DPU_DM_DSC_OUTPUT_IMG_WIDTH_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int dsc_layer_id : 8;
        unsigned int dsc_order0 : 8;
        unsigned int dsc_order1 : 8;
        unsigned int dsc_sel : 8;
    } reg;
} DPU_DM_DSC_SEL_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int dsc_input_fmt : 8;
        unsigned int dsc_output_fmt : 8;
        unsigned int dsc_reserved : 16;
    } reg;
} DPU_DM_DSC_RESERVED_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int wch_input_img_height : 16;
        unsigned int wch_input_img_width : 16;
    } reg;
} DPU_DM_WCH_INPUT_IMG_WIDTH_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int wch_input_img_strx : 16;
        unsigned int wch_input_img_stry : 16;
    } reg;
} DPU_DM_WCH_INPUT_IMG_STRY_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int wch_layer_id : 8;
        unsigned int wch_sel : 8;
        unsigned int wch_input_fmt : 8;
        unsigned int wch_reserved_0 : 8;
    } reg;
} DPU_DM_WCH_RESERVED_0_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int wch_reserved_1 : 32;
    } reg;
} DPU_DM_WCH_RESERVED_1_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int itf_input_img_height : 16;
        unsigned int itf_input_img_width : 16;
    } reg;
} DPU_DM_ITF_INPUT_IMG_WIDTH_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int itf_layer_id : 8;
        unsigned int itf_hidic_en : 8;
        unsigned int itf_sel : 8;
        unsigned int itf_input_fmt : 8;
    } reg;
} DPU_DM_ITF_INPUT_FMT_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int itf_reserved_0 : 32;
    } reg;
} DPU_DM_ITF_RESERVED_0_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int itf_reserved_1 : 32;
    } reg;
} DPU_DM_ITF_RESERVED_1_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int layer_width : 16;
        unsigned int layer_height : 16;
    } reg;
} DPU_DM_LAYER_HEIGHT_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int layer_mask_x0 : 16;
        unsigned int layer_mask_y0 : 16;
    } reg;
} DPU_DM_LAYER_MASK_Y0_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int layer_mask_x1 : 16;
        unsigned int layer_mask_y1 : 16;
    } reg;
} DPU_DM_LAYER_MASK_Y1_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int layer_mask_en : 8;
        unsigned int layer_dma_fmt : 8;
        unsigned int layer_fbc_type : 8;
        unsigned int layer_dma_sel : 8;
    } reg;
} DPU_DM_LAYER_DMA_SEL_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int layer_id : 8;
        unsigned int layer_stretch_en : 8;
        unsigned int layer_scramble_mode : 2;
        unsigned int layer_tag_mode_en : 1;
        unsigned int layer_yuv_trans : 1;
        unsigned int reserved : 4;
        unsigned int layer_sblk_type : 8;
    } reg;
} DPU_DM_LAYER_SBLK_TYPE_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int layer_ov_fmt : 8;
        unsigned int layer_nxt_order : 8;
        unsigned int layer_top_crop : 8;
        unsigned int layer_bot_crop : 8;
    } reg;
} DPU_DM_LAYER_BOT_CROP_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int layer_vmirr_en : 8;
        unsigned int layer_hmirr_en : 8;
        unsigned int layer_ov_pattern_type : 8;
        unsigned int layer_ov_clip_left : 8;
    } reg;
} DPU_DM_LAYER_OV_CLIP_LEFT_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int layer_ov_dfc_ax_swap : 1;
        unsigned int layer_ov_dfc_uv_swap : 1;
        unsigned int layer_ov_dfc_rb_swap : 1;
        unsigned int layer_ov_dfc_glb_alpha1 : 10;
        unsigned int layer_ov_dfc_glb_alpha0 : 10;
        unsigned int layer_ov_clip_right : 6;
        unsigned int reserved : 2;
        unsigned int layer_ov_mitm_degamma_reg_sel : 1;
    } reg;
} DPU_DM_LAYER_OV_DFC_CFG_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int layer_ov_startx : 16;
        unsigned int layer_ov_starty : 16;
    } reg;
} DPU_DM_LAYER_OV_STARTY_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int layer_ov_endx : 16;
        unsigned int layer_ov_endy : 16;
    } reg;
} DPU_DM_LAYER_OV_ENDY_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int reserved_0 : 1;
        unsigned int layer_ov_csc_reg_sel : 4;
        unsigned int layer_ov_bitext_reg_reg_sel : 1;
        unsigned int layer_ov_mitm_en : 6;
        unsigned int layer_ov_csc_en : 1;
        unsigned int layer_ov_dfc_en : 1;
        unsigned int layer_ov_mitm_gamut_reg_sel : 1;
        unsigned int layer_ov_mitm_gama_reg_sel : 1;
        unsigned int layer_ov_pattern_a : 10;
        unsigned int layer_ov_enable : 1;
        unsigned int layer_rop_en : 1;
        unsigned int layer_trop_en : 1;
        unsigned int reserved_1 : 3;
    } reg;
} DPU_DM_LAYER_OV_PATTERN_A_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int layer_ov_pattern_rgb : 30;
        unsigned int reserved : 2;
    } reg;
} DPU_DM_LAYER_OV_PATTERN_RGB_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int layer_ov_alpha_smode : 1;
        unsigned int layer_ov_fix_mode : 1;
        unsigned int layer_ov_dst_pmode : 1;
        unsigned int layer_ov_dst_gmode : 2;
        unsigned int layer_ov_dst_amode : 2;
        unsigned int layer_ov_src_mmode : 1;
        unsigned int layer_ov_src_pmode : 1;
        unsigned int layer_ov_src_gmode : 2;
        unsigned int layer_ov_src_amode : 2;
        unsigned int layer_ov_auto_nosrc : 1;
        unsigned int layer_ov_src_cfg : 1;
        unsigned int layer_ov_trop_code : 4;
        unsigned int layer_ov_rop_code : 8;
        unsigned int layer_clip_en : 1;
        unsigned int layer_bitext_en : 3;
        unsigned int reserved : 1;
    } reg;
} DPU_DM_LAYER_OV_MODE_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int layer_ov_dst_global_alpha : 10;
        unsigned int layer_ov_src_global_alpha : 10;
        unsigned int layer_ov_alpha_offdst : 2;
        unsigned int layer_ov_alpha_offsrc : 1;
        unsigned int layer_ov_alpha : 9;
    } reg;
} DPU_DM_LAYER_OV_ALPHA_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int layer_stretch0_line : 8;
        unsigned int layer_stretch1_line : 8;
        unsigned int layer_stretch2_line : 8;
        unsigned int layer_stretch3_line : 8;
    } reg;
} DPU_DM_LAYER_STRETCH3_LINE_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int layer_start_addr0_h : 8;
        unsigned int layer_start_addr1_h : 8;
        unsigned int layer_start_addr2_h : 8;
        unsigned int layer_start_addr3_h : 8;
    } reg;
} DPU_DM_LAYER_START_ADDR3_H_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int layer_start_addr0_l : 32;
    } reg;
} DPU_DM_LAYER_START_ADDR0_L_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int layer_start_addr1_l : 32;
    } reg;
} DPU_DM_LAYER_START_ADDR1_L_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int layer_start_addr2_l : 32;
    } reg;
} DPU_DM_LAYER_START_ADDR2_L_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int layer_start_addr3_l : 32;
    } reg;
} DPU_DM_LAYER_START_ADDR3_L_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int layer_stride0 : 32;
    } reg;
} DPU_DM_LAYER_STRIDE0_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int layer_stride1 : 32;
    } reg;
} DPU_DM_LAYER_STRIDE1_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int layer_stride2 : 32;
    } reg;
} DPU_DM_LAYER_STRIDE2_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int layer_stride3 : 16;
        unsigned int layer_rsv0 : 16;
    } reg;
} DPU_DM_LAYER_RSV0_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int layer_rsv1 : 32;
    } reg;
} DPU_DM_LAYER_RSV1_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int layer_rsv2 : 32;
    } reg;
} DPU_DM_LAYER_RSV2_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int layer_rsv3 : 32;
    } reg;
} DPU_DM_LAYER_RSV3_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int layer_rsv4 : 32;
    } reg;
} DPU_DM_LAYER_RSV4_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int layer_rsv5 : 32;
    } reg;
} DPU_DM_LAYER_RSV5_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int layer_rsv6 : 32;
    } reg;
} DPU_DM_LAYER_RSV6_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int layer_rsv7 : 32;
    } reg;
} DPU_DM_LAYER_RSV7_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int layer_rsv8 : 32;
    } reg;
} DPU_DM_LAYER_RSV8_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int ch_pending_clr : 1;
        unsigned int reserved : 31;
    } reg;
} DPU_CMD_CMDLIST_CH_PENDING_CLR_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int ch_enable : 1;
        unsigned int ch_auto_single : 1;
        unsigned int ch_start_sel : 3;
        unsigned int ch_pause : 1;
        unsigned int ch_scene_sel : 1;
        unsigned int reserved : 25;
    } reg;
} DPU_CMD_CMDLIST_CH_CTRL_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int ch_ctl_fsm : 4;
        unsigned int reserved : 4;
        unsigned int ch_dma_fsm : 2;
        unsigned int ch_idle : 1;
        unsigned int oa_idle : 1;
        unsigned int ch_cmd_done_id : 10;
        unsigned int ch_cmd_id : 10;
    } reg;
} DPU_CMD_CMDLIST_CH_STATUS_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int reserved : 4;
        unsigned int ch_start_addr : 28;
    } reg;
} DPU_CMD_CMDLIST_CH_STAAD_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int ch_cnt : 4;
        unsigned int ch_curr_addr : 28;
    } reg;
} DPU_CMD_CMDLIST_CH_CURAD_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int ch_onedone_inte : 1;
        unsigned int ch_alldone_inte : 1;
        unsigned int ch_axi_err_inte : 1;
        unsigned int ch_pending_inte : 1;
        unsigned int ch_start_inte : 1;
        unsigned int ch_badcmd_inte : 1;
        unsigned int ch_timeout_inte : 1;
        unsigned int ch_taskdone_inte : 1;
        unsigned int reserved : 24;
    } reg;
} DPU_CMD_CMDLIST_CH_INTE_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int ch_onedone_intc : 1;
        unsigned int ch_alldone_intc : 1;
        unsigned int ch_axi_err_intc : 1;
        unsigned int ch_pending_intc : 1;
        unsigned int ch_start_intc : 1;
        unsigned int ch_badcmd_intc : 1;
        unsigned int ch_timeout_intc : 1;
        unsigned int ch_taskdone_intc : 1;
        unsigned int reserved : 24;
    } reg;
} DPU_CMD_CMDLIST_CH_INTC_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int ch_onedone_ints : 1;
        unsigned int ch_alldone_ints : 1;
        unsigned int ch_axi_err_ints : 1;
        unsigned int ch_pending_ints : 1;
        unsigned int ch_start_ints : 1;
        unsigned int ch_badcmd_ints : 1;
        unsigned int ch_timeout_ints : 1;
        unsigned int ch_taskdone_ints : 1;
        unsigned int reserved : 24;
    } reg;
} DPU_CMD_CMDLIST_CH_INTS_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int cmd_dbg0 : 1;
        unsigned int cmd_dbg1 : 1;
        unsigned int cmd_dbg2 : 1;
        unsigned int cmd_dbg3 : 1;
        unsigned int cmd_dbg4 : 1;
        unsigned int cmd_dbg5 : 1;
        unsigned int cmd_dbg6 : 1;
        unsigned int cmd_dbg7 : 1;
        unsigned int cmd_dbg8 : 1;
        unsigned int cmd_dbg9 : 1;
        unsigned int cmd_dbg10 : 1;
        unsigned int cmd_dbg11 : 1;
        unsigned int reserved : 20;
    } reg;
} DPU_CMD_CMDLIST_CH_DBG_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int cmd_axi_dbg : 32;
    } reg;
} DPU_CMD_CMDLIST_DBG_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int buf_dbg_en : 1;
        unsigned int reserved : 31;
    } reg;
} DPU_CMD_BUF_DBG_EN_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int buf_dbg_cnt_clr : 1;
        unsigned int reserved : 31;
    } reg;
} DPU_CMD_BUF_DBG_CNT_CLR_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int buf_dbg_cnt_th : 9;
        unsigned int reserved : 23;
    } reg;
} DPU_CMD_BUF_DBG_CNT_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int timeout_th : 32;
    } reg;
} DPU_CMD_CMDLIST_TIMEOUT_TH_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int start : 9;
        unsigned int reserved : 23;
    } reg;
} DPU_CMD_CMDLIST_START_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int addr_mask_en : 9;
        unsigned int reserved : 23;
    } reg;
} DPU_CMD_CMDLIST_ADDR_MASK_EN_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int addr_mask_dis : 9;
        unsigned int reserved : 23;
    } reg;
} DPU_CMD_CMDLIST_ADDR_MASK_DIS_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int addr_mask_status : 9;
        unsigned int reserved : 23;
    } reg;
} DPU_CMD_CMDLIST_ADDR_MASK_STATUS_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int task_continue : 9;
        unsigned int reserved : 23;
    } reg;
} DPU_CMD_CMDLIST_TASK_CONTINUE_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int task_status : 9;
        unsigned int reserved : 23;
    } reg;
} DPU_CMD_CMDLIST_TASK_STATUS_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int axi_mid : 4;
        unsigned int axi_qos : 2;
        unsigned int axi_mainpress : 2;
        unsigned int axi_outstd_depth : 4;
        unsigned int buf0_outstd_depth : 2;
        unsigned int buf1_outstd_depth : 2;
        unsigned int buf2_outstd_depth : 2;
        unsigned int buf3_outstd_depth : 2;
        unsigned int buf4_outstd_depth : 2;
        unsigned int buf5_outstd_depth : 2;
        unsigned int buf6_outstd_depth : 2;
        unsigned int reserved : 6;
    } reg;
} DPU_CMD_CMDLIST_CTRL_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int ch_secu : 9;
        unsigned int reserved : 23;
    } reg;
} DPU_CMD_CMDLIST_SECU_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int ch_ints : 9;
        unsigned int reserved : 23;
    } reg;
} DPU_CMD_CMDLIST_INTS_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int ch_swreset : 9;
        unsigned int reserved : 23;
    } reg;
} DPU_CMD_CMDLIST_SWRST_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int cmd_mem_ctrl : 4;
        unsigned int reserved : 28;
    } reg;
} DPU_CMD_MEM_CTRL_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int cmd_clk_sel : 32;
    } reg;
} DPU_CMD_CLK_SEL_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int cmd_clk_en : 32;
    } reg;
} DPU_CMD_CLK_EN_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int cmd_sw_reload_prot : 1;
        unsigned int reserved : 31;
    } reg;
} DPU_CMD_SW_RELOAD_PROT_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int cmd_sync_mode : 1;
        unsigned int reserved : 31;
    } reg;
} DPU_CMD_SYNC_MODE_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int cmd_cfg_flag : 1;
        unsigned int reserved : 31;
    } reg;
} DPU_CMD_CFG_FLAG_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int cmd_sync_dbg : 1;
        unsigned int reserved : 31;
    } reg;
} DPU_CMD_SYNC_DBG_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int ch_cfg_time_en0 : 1;
        unsigned int ch_cfg_time_en1 : 1;
        unsigned int ch_cfg_time_en2 : 1;
        unsigned int ch_cfg_ch_id : 4;
        unsigned int reserved_0 : 1;
        unsigned int ch_cfg_time_node_id : 10;
        unsigned int reserved_1 : 14;
    } reg;
} DPU_CMD_CH0_CFG_TIME_EN_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int ch_cfg_time_cnt : 32;
    } reg;
} DPU_CMD_CH0_CFG_TIME_CNT_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int sce0_scene_fsm_cs : 4;
        unsigned int sce0_dma_fsm_cs : 3;
        unsigned int reserved_0 : 1;
        unsigned int sce0_fifo_empty : 1;
        unsigned int reserved_1 : 3;
        unsigned int sce0_buf_empty : 1;
        unsigned int reserved_2 : 3;
        unsigned int sce0_dcpt_fsm : 2;
        unsigned int reserved_3 : 2;
        unsigned int sce0_dcpt_fifo_empty : 1;
        unsigned int reserved_4 : 11;
    } reg;
} DPU_CMD_SCE0_CFG_SIGNAL_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int sce1_scene_fsm_cs : 4;
        unsigned int sce1_dma_fsm_cs : 3;
        unsigned int reserved_0 : 1;
        unsigned int sce1_fifo_empty : 1;
        unsigned int reserved_1 : 3;
        unsigned int sce1_buf_empty : 1;
        unsigned int reserved_2 : 3;
        unsigned int sce1_dcpt_fsm : 2;
        unsigned int reserved_3 : 2;
        unsigned int sce1_dcpt_fifo_empty : 1;
        unsigned int reserved_4 : 11;
    } reg;
} DPU_CMD_SCE1_CFG_SIGNAL_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int sce2_scene_fsm_cs : 4;
        unsigned int sce2_dma_fsm_cs : 3;
        unsigned int reserved_0 : 1;
        unsigned int sce2_fifo_empty : 1;
        unsigned int reserved_1 : 3;
        unsigned int sce2_buf_empty : 1;
        unsigned int reserved_2 : 3;
        unsigned int sce2_dcpt_fsm : 2;
        unsigned int reserved_3 : 2;
        unsigned int sce2_dcpt_fifo_empty : 1;
        unsigned int reserved_4 : 11;
    } reg;
} DPU_CMD_SCE2_CFG_SIGNAL_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int sce3_scene_fsm_cs : 4;
        unsigned int sce3_dma_fsm_cs : 3;
        unsigned int reserved_0 : 1;
        unsigned int sce3_fifo_empty : 1;
        unsigned int reserved_1 : 3;
        unsigned int sce3_buf_empty : 1;
        unsigned int reserved_2 : 3;
        unsigned int sce3_dcpt_fsm : 2;
        unsigned int reserved_3 : 2;
        unsigned int sce3_dcpt_fifo_empty : 1;
        unsigned int reserved_4 : 11;
    } reg;
} DPU_CMD_SCE3_CFG_SIGNAL_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int sce4_scene_fsm_cs : 4;
        unsigned int sce4_dma_fsm_cs : 3;
        unsigned int reserved_0 : 1;
        unsigned int sce4_fifo_empty : 1;
        unsigned int reserved_1 : 3;
        unsigned int sce4_buf_empty : 1;
        unsigned int reserved_2 : 3;
        unsigned int sce4_dcpt_fsm : 2;
        unsigned int reserved_3 : 2;
        unsigned int sce4_dcpt_fifo_empty : 1;
        unsigned int reserved_4 : 11;
    } reg;
} DPU_CMD_SCE4_CFG_SIGNAL_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int sce5_scene_fsm_cs : 4;
        unsigned int sce5_dma_fsm_cs : 3;
        unsigned int reserved_0 : 1;
        unsigned int sce5_fifo_empty : 1;
        unsigned int reserved_1 : 3;
        unsigned int sce5_buf_empty : 1;
        unsigned int reserved_2 : 3;
        unsigned int sce5_dcpt_fsm : 2;
        unsigned int reserved_3 : 2;
        unsigned int sce5_dcpt_fifo_empty : 1;
        unsigned int reserved_4 : 11;
    } reg;
} DPU_CMD_SCE5_CFG_SIGNAL_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int sce6_scene_fsm_cs : 4;
        unsigned int sce6_dma_fsm_cs : 3;
        unsigned int reserved_0 : 1;
        unsigned int sce6_fifo_empty : 1;
        unsigned int reserved_1 : 3;
        unsigned int sce6_buf_empty : 1;
        unsigned int reserved_2 : 3;
        unsigned int sce6_dcpt_fsm : 2;
        unsigned int reserved_3 : 2;
        unsigned int sce6_dcpt_fifo_empty : 1;
        unsigned int reserved_4 : 11;
    } reg;
} DPU_CMD_SCE6_CFG_SIGNAL_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int lbusif_mstate : 4;
        unsigned int reserved : 28;
    } reg;
} DPU_CMD_LBUSIF_MSTATE_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int two_same_frame_bypass : 1;
        unsigned int scene_dacc_int_ctrl : 7;
        unsigned int scene_int_delay_cycle : 6;
        unsigned int reserved : 18;
    } reg;
} DPU_CMD_TWO_SAME_FRAME_BYPASS_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int dss_tag : 32;
    } reg;
} DPU_GLB_TAG_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int cpu_cmd_force : 2;
        unsigned int reserved : 30;
    } reg;
} DPU_GLB_CPU_CMD_FORCE_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int dss_jtag_mode : 1;
        unsigned int reserved : 31;
    } reg;
} DPU_GLB_JTAG_MODE_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int dss_dacc_im_s : 1;
        unsigned int dss_dacc_dm_s : 1;
        unsigned int dss_dacc_wdt_s : 1;
        unsigned int reserved : 29;
    } reg;
} DPU_GLB_DACC_TZPC_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int clk_dss_core_preload_sel : 1;
        unsigned int reserved_0 : 1;
        unsigned int clk_dss_core_cld0_sel : 1;
        unsigned int clk_dss_core_cld1_sel : 1;
        unsigned int clk_dss_core_cmd_sel : 1;
        unsigned int clk_dss_core_dbcu_sel : 1;
        unsigned int reserved_1 : 1;
        unsigned int clk_dss_core_itfsw_sel : 1;
        unsigned int clk_dss_core_lbuf_ctrl_sel : 1;
        unsigned int clk_dss_core_lbuf_part0_sel : 1;
        unsigned int clk_dss_core_part_lbus_sel : 1;
        unsigned int clk_dss_core_smartdma0_sel : 1;
        unsigned int clk_dss_core_smartdma1_sel : 1;
        unsigned int clk_dss_core_smartdma2_sel : 1;
        unsigned int clk_dss_core_smartdma3_sel : 1;
        unsigned int clk_dss_core_uvup_sel : 1;
        unsigned int clk_dss_core_dbg_sel : 1;
        unsigned int clk_dss_core_glb_cfg_sel : 1;
        unsigned int clk_dss_core_wch0_sel : 1;
        unsigned int clk_dss_core_wch1_sel : 1;
        unsigned int clk_dss_core_wch2_sel : 1;
        unsigned int clk_dss_core_srot0_sel : 1;
        unsigned int clk_dss_core_srot1_sel : 1;
        unsigned int clk_dss_core_avhr_sel : 1;
        unsigned int clk_dss_core_clear_sel : 1;
        unsigned int clk_dss_core_dacc_sel : 1;
        unsigned int reserved_2 : 1;
        unsigned int reserved_3 : 1;
        unsigned int clk_dss_core_hemcd0_sel : 1;
        unsigned int clk_dss_core_hemcd1_sel : 1;
        unsigned int reserved_4 : 2;
    } reg;
} DPU_GLB_MODULE_CLK_SEL_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int clk_dss_core_preload_en : 1;
        unsigned int reserved_0 : 1;
        unsigned int clk_dss_core_cld0_en : 1;
        unsigned int clk_dss_core_cld1_en : 1;
        unsigned int clk_dss_core_cmd_en : 1;
        unsigned int clk_dss_core_dbcu_en : 1;
        unsigned int reserved_1 : 1;
        unsigned int clk_dss_core_itfsw_en : 1;
        unsigned int clk_dss_core_lbuf_ctrl_en : 1;
        unsigned int clk_dss_core_lbuf_part0_en : 1;
        unsigned int clk_dss_core_part_lbus_en : 1;
        unsigned int clk_dss_core_smartdma0_en : 1;
        unsigned int clk_dss_core_smartdma1_en : 1;
        unsigned int clk_dss_core_smartdma2_en : 1;
        unsigned int clk_dss_core_smartdma3_en : 1;
        unsigned int clk_dss_core_uvup_en : 1;
        unsigned int clk_dss_core_dbg_en : 1;
        unsigned int clk_dss_core_glb_cfg_en : 1;
        unsigned int clk_dss_core_wch0_en : 1;
        unsigned int clk_dss_core_wch1_en : 1;
        unsigned int clk_dss_core_wch2_en : 1;
        unsigned int clk_dss_core_srot0_en : 1;
        unsigned int clk_dss_core_srot1_en : 1;
        unsigned int clk_dss_core_avhr_en : 1;
        unsigned int clk_dss_core_clear_en : 1;
        unsigned int clk_dss_core_dacc_en : 1;
        unsigned int reserved_2 : 1;
        unsigned int reserved_3 : 1;
        unsigned int clk_dss_core_hemcd0_en : 1;
        unsigned int clk_dss_core_hemcd1_en : 1;
        unsigned int reserved_4 : 2;
    } reg;
} DPU_GLB_MODULE_CLK_EN_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int dss_pm_ctrl : 32;
    } reg;
} DPU_GLB_PM_CTRL_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int glb_dslp_en : 1;
        unsigned int reserved : 31;
    } reg;
} DPU_GLB_DSLP_EN_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int dss_mem_ctrl : 32;
    } reg;
} DPU_GLB_SPRAM_CTRL_S_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int dss_mem_ctrl_tp : 32;
    } reg;
} DPU_GLB_TPRAM_CTRL_TP_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int dss_mem_ctrl_spc : 32;
    } reg;
} DPU_GLB_SPRAM_CTRL_SPC_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int dss_mem_ctrl_tpl : 32;
    } reg;
} DPU_GLB_TPRAM_CTRL_TPL_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int dsi0_srst : 1;
        unsigned int dsi1_srst : 1;
        unsigned int dsi2_srst : 1;
        unsigned int dpi0_srst : 1;
        unsigned int dpi1_srst : 1;
        unsigned int reserved : 27;
    } reg;
} DPU_GLB_RS_CLEAR_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int dss_disp_ch0_clk_sel : 32;
    } reg;
} DPU_GLB_DISP_CH0_MODULE_CLK_SEL_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int dss_disp_ch0_clk_en : 32;
    } reg;
} DPU_GLB_DISP_CH0_MODULE_CLK_EN_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int dss_disp_ch1_clk_sel : 32;
    } reg;
} DPU_GLB_DISP_CH1_MODULE_CLK_SEL_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int dss_disp_ch1_clk_en : 32;
    } reg;
} DPU_GLB_DISP_CH1_MODULE_CLK_EN_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int dss_avhr_clk_sel : 32;
    } reg;
} DPU_GLB_DISP_AVHR_MODULE_CLK_SEL_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int dss_avhr_clk_en : 32;
    } reg;
} DPU_GLB_DISP_AVHR_MODULE_CLK_EN_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int reserved_0 : 1;
        unsigned int clk_dss_core_dpi_sel : 1;
        unsigned int clk_dss_core_dsi2_sel : 1;
        unsigned int clk_dss_core_dsi1_sel : 1;
        unsigned int clk_dss_core_dsi0_sel : 1;
        unsigned int reserved_1 : 27;
    } reg;
} DPU_GLB_DISP_MODULE_CLK_SEL_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int reserved_0 : 1;
        unsigned int clk_dss_core_dpi_en : 1;
        unsigned int clk_dss_core_dsi2_en : 1;
        unsigned int clk_dss_core_dsi1_en : 1;
        unsigned int clk_dss_core_dsi0_en : 1;
        unsigned int reserved_1 : 27;
    } reg;
} DPU_GLB_DISP_MODULE_CLK_EN_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int reserved_0 : 1;
        unsigned int rst_dpi_en : 1;
        unsigned int rst_dsi2_en : 1;
        unsigned int rst_dsi1_en : 1;
        unsigned int rst_dsi0_en : 1;
        unsigned int reserved_1 : 27;
    } reg;
} DPU_GLB_RST_EN_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int gmp_mem_ctrl : 4;
        unsigned int reserved : 28;
    } reg;
} DPU_GLB_GMP_MEM_CTRL_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int lb_part_mem_ctrl : 4;
        unsigned int reserved : 28;
    } reg;
} DPU_GLB_LB_PART_MEM_CTRL_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int dacc_mem_ctrl : 4;
        unsigned int reserved : 28;
    } reg;
} DPU_GLB_DACC_MEM_CTRL_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int sdma0_mem_ctrl : 4;
        unsigned int reserved : 28;
    } reg;
} DPU_GLB_SDMA0_MEM_CTRL_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int sdma1_mem_ctrl : 4;
        unsigned int reserved : 28;
    } reg;
} DPU_GLB_SDMA1_MEM_CTRL_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int sdma2_mem_ctrl : 4;
        unsigned int reserved : 28;
    } reg;
} DPU_GLB_SDMA2_MEM_CTRL_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int dss_ns_dfs_ok_to_lpmcu_o : 1;
        unsigned int reserved : 31;
    } reg;
} DPU_GLB_NS_DFS_OK_TO_LPMCU_O_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int dss_ns_dfs_ok_to_lpmcu_msk : 1;
        unsigned int reserved : 31;
    } reg;
} DPU_GLB_NS_DFS_OK_TO_LPMCU_MSK_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int dss_ns_em_sensorhub_int_o : 1;
        unsigned int dss_ns_em_ivp_int_o : 1;
        unsigned int dss_ns_em_isp_int_o : 1;
        unsigned int dss_ns_em_ipp_int_o : 1;
        unsigned int reserved : 28;
    } reg;
} DPU_GLB_NS_EM_O_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int dss_ns_em_sensorhub_to_iomcu_msk : 1;
        unsigned int dss_ns_em_ivp_msk : 1;
        unsigned int dss_ns_em_isp_msk : 1;
        unsigned int dss_ns_em_ipp_msk : 1;
        unsigned int reserved : 28;
    } reg;
} DPU_GLB_NS_EM_MSK_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int dss_ns_to_npu_o : 18;
        unsigned int reserved : 14;
    } reg;
} DPU_GLB_NS_TO_NPU_O_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int dss_ns_to_npu_msk : 18;
        unsigned int reserved : 14;
    } reg;
} DPU_GLB_NS_TO_NPU_MASK_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int dss_s_to_npu_o : 18;
        unsigned int reserved : 14;
    } reg;
} DPU_GLB_S_TO_NPU_O_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int dss_s_to_npu_msk : 18;
        unsigned int reserved : 14;
    } reg;
} DPU_GLB_S_TO_NPU_MASK_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int dss_ns_dp0_to_gic_o : 18;
        unsigned int reserved : 14;
    } reg;
} DPU_GLB_NS_DP0_TO_GIC_O_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int dss_ns_dp0_to_gic_msk : 18;
        unsigned int reserved : 14;
    } reg;
} DPU_GLB_NS_DP0_TO_GIC_MSK_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int dss_ns_dp1_to_gic_o : 18;
        unsigned int reserved : 14;
    } reg;
} DPU_GLB_NS_DP1_TO_GIC_O_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int dss_ns_dp1_to_gic_msk : 18;
        unsigned int reserved : 14;
    } reg;
} DPU_GLB_NS_DP1_TO_GIC_MSK_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int dss_ns_mdp_to_gic_o : 18;
        unsigned int reserved : 14;
    } reg;
} DPU_GLB_NS_MDP_TO_GIC_O_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int dss_ns_mdp_to_gic_msk : 18;
        unsigned int reserved : 14;
    } reg;
} DPU_GLB_NS_MDP_TO_GIC_MSK_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int dss_ns_sdp_to_gic_o : 18;
        unsigned int reserved : 14;
    } reg;
} DPU_GLB_NS_SDP_TO_GIC_O_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int dss_ns_sdp_to_gic_msk : 18;
        unsigned int reserved : 14;
    } reg;
} DPU_GLB_NS_SDP_TO_GIC_MSK_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int dss_ns_offline0_to_gic_o : 18;
        unsigned int reserved : 14;
    } reg;
} DPU_GLB_NS_OFFLINE0_TO_GIC_O_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int dss_ns_offline0_to_gic_msk : 18;
        unsigned int reserved : 14;
    } reg;
} DPU_GLB_NS_OFFLINE0_TO_GIC_MSK_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int dss_ns_offline1_to_gic_o : 18;
        unsigned int reserved : 14;
    } reg;
} DPU_GLB_NS_OFFLINE1_TO_GIC_O_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int dss_ns_offline1_to_gic_msk : 18;
        unsigned int reserved : 14;
    } reg;
} DPU_GLB_NS_OFFLINE1_TO_GIC_MSK_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int dss_ns_offline2_to_gic_o : 18;
        unsigned int reserved : 14;
    } reg;
} DPU_GLB_NS_OFFLINE2_TO_GIC_O_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int dss_ns_offline2_to_gic_msk : 18;
        unsigned int reserved : 14;
    } reg;
} DPU_GLB_NS_OFFLINE2_TO_GIC_MSK_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int dss_ns_dp0_to_iomcu_o : 18;
        unsigned int reserved : 14;
    } reg;
} DPU_GLB_NS_DP0_TO_IOMCU_O_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int dss_ns_dp0_to_iomcu_msk : 18;
        unsigned int reserved : 14;
    } reg;
} DPU_GLB_NS_DP0_TO_IOMCU_MSK_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int dss_ns_dp1_to_iomcu_o : 18;
        unsigned int reserved : 14;
    } reg;
} DPU_GLB_NS_DP1_TO_IOMCU_O_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int dss_ns_dp1_to_iomcu_msk : 18;
        unsigned int reserved : 14;
    } reg;
} DPU_GLB_NS_DP1_TO_IOMCU_MSK_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int dss_ns_mdp_to_iomcu_o : 18;
        unsigned int reserved : 14;
    } reg;
} DPU_GLB_NS_MDP_TO_IOMCU_O_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int dss_ns_mdp_to_iomcu_msk : 18;
        unsigned int reserved : 14;
    } reg;
} DPU_GLB_NS_MDP_TO_IOMCU_MSK_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int dss_ns_sdp_to_iomcu_o : 18;
        unsigned int reserved : 14;
    } reg;
} DPU_GLB_NS_SDP_TO_IOMCU_O_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int dss_ns_sdp_to_iomcu_msk : 18;
        unsigned int reserved : 14;
    } reg;
} DPU_GLB_NS_SDP_TO_IOMCU_MSK_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int dss_ns_offline0_to_iomcu_o : 18;
        unsigned int reserved : 14;
    } reg;
} DPU_GLB_NS_OFFLINE0_TO_IOMCU_O_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int dss_ns_offline0_to_iomcu_msk : 18;
        unsigned int reserved : 14;
    } reg;
} DPU_GLB_NS_OFFLINE0_TO_IOMCU_MSK_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int dss_ns_offline1_to_iomcu_o : 18;
        unsigned int reserved : 14;
    } reg;
} DPU_GLB_NS_OFFLINE1_TO_IOMCU_O_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int dss_ns_offline1_to_iomcu_msk : 18;
        unsigned int reserved : 14;
    } reg;
} DPU_GLB_NS_OFFLINE1_TO_IOMCU_MSK_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int dss_ns_offline2_to_iomcu_o : 18;
        unsigned int reserved : 14;
    } reg;
} DPU_GLB_NS_OFFLINE2_TO_IOMCU_O_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int dss_ns_offline2_to_iomcu_msk : 18;
        unsigned int reserved : 14;
    } reg;
} DPU_GLB_NS_OFFLINE2_TO_IOMCU_MSK_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int dss_s_dp0_to_gic_o : 18;
        unsigned int reserved : 14;
    } reg;
} DPU_GLB_S_DP0_TO_GIC_O_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int dss_s_dp0_to_gic_msk : 18;
        unsigned int reserved : 14;
    } reg;
} DPU_GLB_S_DP0_TO_GIC_MSK_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int dss_s_dp1_to_gic_o : 18;
        unsigned int reserved : 14;
    } reg;
} DPU_GLB_S_DP1_TO_GIC_O_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int dss_s_dp1_to_gic_msk : 18;
        unsigned int reserved : 14;
    } reg;
} DPU_GLB_S_DP1_TO_GIC_MSK_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int dss_s_mdp_to_gic_o : 18;
        unsigned int reserved : 14;
    } reg;
} DPU_GLB_S_MDP_TO_GIC_O_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int dss_s_mdp_to_gic_msk : 18;
        unsigned int reserved : 14;
    } reg;
} DPU_GLB_S_MDP_TO_GIC_MSK_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int dss_s_sdp_to_gic_o : 18;
        unsigned int reserved : 14;
    } reg;
} DPU_GLB_S_SDP_TO_GIC_O_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int dss_s_sdp_to_gic_msk : 18;
        unsigned int reserved : 14;
    } reg;
} DPU_GLB_S_SDP_TO_GIC_MSK_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int dss_offline0_to_gic_o : 18;
        unsigned int reserved : 14;
    } reg;
} DPU_GLB_OFFLINE0_TO_GIC_O_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int dss_s_offline0_to_gic_msk : 18;
        unsigned int reserved : 14;
    } reg;
} DPU_GLB_S_OFFLINE0_TO_GIC_MSK_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int dss_s_offline1_to_gic_o : 18;
        unsigned int reserved : 14;
    } reg;
} DPU_GLB_S_OFFLINE1_TO_GIC_O_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int dss_s_offline1_to_gic_msk : 18;
        unsigned int reserved : 14;
    } reg;
} DPU_GLB_S_OFFLINE1_TO_GIC_MSK_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int dss_s_offline2_to_gic_o : 18;
        unsigned int reserved : 14;
    } reg;
} DPU_GLB_S_OFFLINE2_TO_GIC_O_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int dss_s_offline2_to_gic_msk : 18;
        unsigned int reserved : 14;
    } reg;
} DPU_GLB_S_OFFLINE2_TO_GIC_MSK_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int dss_s_dp0_to_lpmcu_o : 18;
        unsigned int reserved : 14;
    } reg;
} DPU_GLB_S_DP0_TO_LPMCU_O_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int dss_s_dp0_to_lpmcu_msk : 18;
        unsigned int reserved : 14;
    } reg;
} DPU_GLB_S_DP0_TO_LPMCU_MSK_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int dss_s_dp1_to_lpmcu_o : 18;
        unsigned int reserved : 14;
    } reg;
} DPU_GLB_S_DP1_TO_LPMCU_O_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int dss_s_dp1_to_lpmcu_msk : 18;
        unsigned int reserved : 14;
    } reg;
} DPU_GLB_S_DP1_TO_LPMCU_MSK_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int dss_s_mdp_to_lpmcu_o : 18;
        unsigned int reserved : 14;
    } reg;
} DPU_GLB_S_MDP_TO_LPMCU_O_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int dss_s_mdp_to_lpmcu_msk : 18;
        unsigned int reserved : 14;
    } reg;
} DPU_GLB_S_MDP_TO_LPMCU_MSK_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int dss_s_sdp_to_lpmcu_o : 18;
        unsigned int reserved : 14;
    } reg;
} DPU_GLB_S_SDP_TO_LPMCU_O_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int dss_s_sdp_to_lpmcu_msk : 18;
        unsigned int reserved : 14;
    } reg;
} DPU_GLB_S_SDP_TO_LPMCU_MSK_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int dss_s_offline0_to_lpmcu_o : 18;
        unsigned int reserved : 14;
    } reg;
} DPU_GLB_S_OFFLINE0_TO_LPMCU_O_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int dss_s_offline0_to_lpmcu_msk : 18;
        unsigned int reserved : 14;
    } reg;
} DPU_GLB_S_OFFLINE0_TO_LPMCU_MSK_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int dss_s_offline1_to_lpmcu_o : 18;
        unsigned int reserved : 14;
    } reg;
} DPU_GLB_S_OFFLINE1_TO_LPMCU_O_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int dss_s_offline1_to_lpmcu_msk : 18;
        unsigned int reserved : 14;
    } reg;
} DPU_GLB_S_OFFLINE1_TO_LPMCU_MSK_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int dss_s_offline2_to_lpmcu_o : 18;
        unsigned int reserved : 14;
    } reg;
} DPU_GLB_S_OFFLINE2_TO_LPMCU_O_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int dss_s_offline2_to_lpmcu_msk : 18;
        unsigned int reserved : 14;
    } reg;
} DPU_GLB_S_OFFLINE2_TO_LPMCU_MSK_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int dbg_irq_not_safe_gic : 7;
        unsigned int reserved : 25;
    } reg;
} DPU_GLB_DBG_IRQ_NS_GIC_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int dbg_irq_safe_gic : 7;
        unsigned int reserved : 25;
    } reg;
} DPU_GLB_DBG_IRQ_S_GIC_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int dbg_irq_not_safe_iomcu : 7;
        unsigned int reserved : 25;
    } reg;
} DPU_GLB_DBG_IRQ_NS_IOMCU_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int dbg_irq_safe_lpmcu : 7;
        unsigned int reserved : 25;
    } reg;
} DPU_GLB_DBG_IRQ_S_LPMCU_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int dbg_irq_not_safe_npu : 1;
        unsigned int reserved : 31;
    } reg;
} DPU_GLB_DBG_IRQ_NS_NPU_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int dbg_irq_safe_npu : 1;
        unsigned int reserved : 31;
    } reg;
} DPU_GLB_DBG_IRQ_S_NPU_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int cmd_ns_int_o : 9;
        unsigned int reserved : 23;
    } reg;
} DPU_GLB_CMD_NS_INT_O_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int cmd_ns_int_msk : 9;
        unsigned int reserved : 23;
    } reg;
} DPU_GLB_CMD_NS_INT_MSK_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int smart_dma0_ns_int_o : 14;
        unsigned int reserved : 18;
    } reg;
} DPU_GLB_SMART_DMA0_NS_INT_O_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int smart_dma0_ns_int_msk : 14;
        unsigned int reserved : 18;
    } reg;
} DPU_GLB_SMART_DMA0_NS_INT_MSK_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int smart_dma1_ns_int_o : 14;
        unsigned int reserved : 18;
    } reg;
} DPU_GLB_SMART_DMA1_NS_INT_O_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int smart_dma1_ns_int_msk : 14;
        unsigned int reserved : 18;
    } reg;
} DPU_GLB_SMART_DMA1_NS_INT_MSK_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int smart_dma2_ns_int_o : 14;
        unsigned int reserved : 18;
    } reg;
} DPU_GLB_SMART_DMA2_NS_INT_O_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int smart_dma2_ns_int_msk : 14;
        unsigned int reserved : 18;
    } reg;
} DPU_GLB_SMART_DMA2_NS_INT_MSK_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int smart_dma3_ns_int_o : 14;
        unsigned int reserved : 18;
    } reg;
} DPU_GLB_SMART_DMA3_NS_INT_O_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int smart_dma3_ns_int_msk : 14;
        unsigned int reserved : 18;
    } reg;
} DPU_GLB_SMART_DMA3_NS_INT_MSK_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int wch0_ns_int_o : 14;
        unsigned int reserved : 18;
    } reg;
} DPU_GLB_WCH0_NS_INT_O_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int wch0_ns_int_msk : 14;
        unsigned int reserved : 18;
    } reg;
} DPU_GLB_WCH0_NS_INT_MSK_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int wch1_ns_int_o : 15;
        unsigned int reserved : 17;
    } reg;
} DPU_GLB_WCH1_NS_INT_O_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int wch1_ns_int_msk : 15;
        unsigned int reserved : 17;
    } reg;
} DPU_GLB_WCH1_NS_INT_MSK_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int wch2_ns_int_o : 14;
        unsigned int reserved : 18;
    } reg;
} DPU_GLB_WCH2_NS_INT_O_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int wch2_ns_int_msk : 14;
        unsigned int reserved : 18;
    } reg;
} DPU_GLB_WCH2_NS_INT_MSK_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int dacc_ns_int_o : 10;
        unsigned int reserved : 22;
    } reg;
} DPU_GLB_DACC_NS_INT_O_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int dacc_ns_int_msk : 10;
        unsigned int reserved : 22;
    } reg;
} DPU_GLB_DACC_NS_INT_MSK_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int hemcd0_ns_int_o : 8;
        unsigned int reserved : 24;
    } reg;
} DPU_GLB_HEMCD0_NS_INT_O_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int hemcd0_ns_int_msk : 8;
        unsigned int reserved : 24;
    } reg;
} DPU_GLB_HEMCD0_NS_INT_MSK_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int dde_ns_int_o : 8;
        unsigned int reserved : 24;
    } reg;
} DPU_GLB_DDE_NS_INT_O_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int dde_ns_int_msk : 8;
        unsigned int reserved : 24;
    } reg;
} DPU_GLB_DDE_NS_INT_MSK_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int hemcd1_ns_int_o : 8;
        unsigned int reserved : 24;
    } reg;
} DPU_GLB_HEMCD1_NS_INT_O_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int hemcd1_ns_int_msk : 8;
        unsigned int reserved : 24;
    } reg;
} DPU_GLB_HEMCD1_NS_INT_MSK_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int dacc_s_int_o : 7;
        unsigned int reserved : 25;
    } reg;
} DPU_GLB_DACC_S_INT_O_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int dacc_s_int_msk : 7;
        unsigned int reserved : 25;
    } reg;
} DPU_GLB_DACC_S_INT_MSK_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int dma_ddrr_os_pld : 6;
        unsigned int dma_ddrr_os_hdr : 6;
        unsigned int dma_va_en : 1;
        unsigned int reserved : 19;
    } reg;
} DPU_GLB_SDMA_CTRL0_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int dma_hebc_raw_mode_en : 1;
        unsigned int dma_err_int_en : 1;
        unsigned int dma_check_en : 1;
        unsigned int reserved : 29;
    } reg;
} DPU_GLB_SDMA_CTRL1_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int dma_clkgate_off : 1;
        unsigned int dma_mem_sd_off : 1;
        unsigned int dma_auto_clk_gt_en : 1;
        unsigned int reserved : 29;
    } reg;
} DPU_GLB_SDMA_CTRL2_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int dma_dbg_reserved0 : 32;
    } reg;
} DPU_GLB_SDMA_DBG_RESERVED0_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int dma_dbg_out0 : 32;
    } reg;
} DPU_GLB_SDMA_DBG_OUT0_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int dma_dbg_out1 : 32;
    } reg;
} DPU_GLB_SDMA_DBG_OUT1_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int dma_dbg_monitor0 : 32;
    } reg;
} DPU_GLB_SDMA_DBG_MONITOR0_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int dma_dbg_monitor1 : 32;
    } reg;
} DPU_GLB_SDMA_DBG_MONITOR1_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int dma_dbg_monitor2 : 32;
    } reg;
} DPU_GLB_SDMA_DBG_MONITOR2_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int dma_dbg_monitor3 : 32;
    } reg;
} DPU_GLB_SDMA_DBG_MONITOR3_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int dma_dbg_monitor4 : 32;
    } reg;
} DPU_GLB_SDMA_DBG_MONITOR4_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int dma_dbg_monitor5 : 32;
    } reg;
} DPU_GLB_SDMA_DBG_MONITOR5_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int sdma_ctrl3 : 1;
        unsigned int reserved : 31;
    } reg;
} DPU_GLB_SDMA_CTRL3_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int sdma_dbg_out2 : 1;
        unsigned int reserved : 31;
    } reg;
} DPU_GLB_SDMA_DBG_OUT2_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int dma_dbg_reserved1 : 32;
    } reg;
} DPU_GLB_SDMA_DBG_RESERVED1_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int sdma_w_dbg0 : 32;
    } reg;
} DPU_GLB_SDMA_W_DBG0_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int sdma_w_dbg1 : 32;
    } reg;
} DPU_GLB_SDMA_W_DBG1_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int sdma_w_dbg2 : 32;
    } reg;
} DPU_GLB_SDMA_W_DBG2_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int sdma_w_dbg3 : 32;
    } reg;
} DPU_GLB_SDMA_W_DBG3_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int srot_dbg_out0 : 32;
    } reg;
} DPU_GLB_SROT_DBG_OUT0_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int srot_dbg_out1 : 32;
    } reg;
} DPU_GLB_SROT_DBG_OUT1_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int srot_dbg_out2 : 32;
    } reg;
} DPU_GLB_SROT_DBG_OUT2_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int srot_dbg_out3 : 32;
    } reg;
} DPU_GLB_SROT_DBG_OUT3_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int srot_dbg_out4 : 32;
    } reg;
} DPU_GLB_SROT_DBG_OUT4_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int srot_dbg_out5 : 32;
    } reg;
} DPU_GLB_SROT_DBG_OUT5_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int srot_dbg_out6 : 32;
    } reg;
} DPU_GLB_SROT_DBG_OUT6_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int srot_dbg_out7 : 32;
    } reg;
} DPU_GLB_SROT_DBG_OUT7_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int srot_dbg_reserved : 32;
    } reg;
} DPU_GLB_SROT_DBG_RESERVED_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int srot_ctl0 : 1;
        unsigned int reserved : 31;
    } reg;
} DPU_GLB_SROT_CTRL0_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int reserved_0 : 1;
        unsigned int reg_dvfs_scene_id : 3;
        unsigned int reg_dvfs_vsync_sel : 3;
        unsigned int reg_peri_dvfs_en : 1;
        unsigned int reg_ddr_dvfs_en : 1;
        unsigned int reserved_1 : 23;
    } reg;
} DPU_GLB_REG_DVFS_CTRL_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int reg_peri_dvfs_ycnt0 : 16;
        unsigned int reg_peri_dvfs_ycnt1 : 16;
    } reg;
} DPU_GLB_REG_PERI_DVFS_YCNT0_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int reg_peri_dvfs_ycnt2 : 16;
        unsigned int reg_peri_dvfs_ycnt3 : 16;
    } reg;
} DPU_GLB_REG_PERI_DVFS_YCNT2_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int reg_ddr_dvfs_ycnt0 : 16;
        unsigned int reg_ddr_dvfs_ycnt1 : 16;
    } reg;
} DPU_GLB_REG_DDR_DVFS_YCNT0_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int reg_ddr_dvfs_ycnt2 : 16;
        unsigned int reg_ddr_dvfs_ycnt3 : 16;
    } reg;
} DPU_GLB_REG_DDR_DVFS_YCNT2_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int reg_dvfs_ov_ycnt : 16;
        unsigned int reserved : 16;
    } reg;
} DPU_GLB_REG_DVFS_OV_CNT_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int cmd_addr : 32;
    } reg;
} DPU_GLB_CMD_ADDR_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int reg_dvfs_flush_en : 1;
        unsigned int reserved : 31;
    } reg;
} DPU_GLB_REG_DVFS_CTRL1_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int dbg_ip_sel : 8;
        unsigned int dbg_ip_vsync_sel : 3;
        unsigned int dbg_ip_en : 1;
        unsigned int reserved : 20;
    } reg;
} DPU_GLB_DBG_CTRL0_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int dbg_ip_cycle_cnt0 : 16;
        unsigned int dbg_ip_cycle_cnt1 : 16;
    } reg;
} DPU_GLB_DBG_IP_CYCLE_CNT_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int dbg_ip_state0 : 32;
    } reg;
} DPU_GLB_DBG_IP_STATE0_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int dbg_ip_state1 : 32;
    } reg;
} DPU_GLB_DBG_IP_STATE1_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int dbg_ov_ycnt_en : 1;
        unsigned int reserved : 31;
    } reg;
} DPU_GLB_DBG_CTRL1_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int dbg_ov_scene_id0_ycnt : 16;
        unsigned int dbg_ov_scene_id1_ycnt : 16;
    } reg;
} DPU_GLB_DBG_OV_YCNT0_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int dbg_ov_scene_id2_ycnt : 16;
        unsigned int dbg_ov_scene_id3_ycnt : 16;
    } reg;
} DPU_GLB_DBG_OV_YCNT1_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int dbg_ov_scene_id4_ycnt : 16;
        unsigned int dbg_ov_scene_id5_ycnt : 16;
    } reg;
} DPU_GLB_DBG_OV_YCNT2_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int dbg_ov_scene_id6_ycnt : 16;
        unsigned int reserved : 16;
    } reg;
} DPU_GLB_DBG_OV_YCNT3_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int dbg_cfg_ctrl : 2;
        unsigned int reserved : 30;
    } reg;
} DPU_GLB_DBG_CFG_CTRL_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int dbg_cfg_state : 1;
        unsigned int reserved : 31;
    } reg;
} DPU_GLB_DBG_CFG_STATE_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int dbg_cfg_addr : 32;
    } reg;
} DPU_GLB_DBG_CFG_ADDR_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int dbg_dcpt_sel : 4;
        unsigned int dbg_dcpt_en : 1;
        unsigned int reserved : 27;
    } reg;
} DPU_GLB_DBG_DCPT_CTRL_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int dbg_dcpt_data0 : 32;
    } reg;
} DPU_GLB_DBG_DCPT_DATA0_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int dbg_dcpt_data1 : 32;
    } reg;
} DPU_GLB_DBG_DCPT_DATA1_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int dbg_data_sel : 32;
    } reg;
} DPU_GLB_DBG_DATA_SEL_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int dbg_data0 : 32;
    } reg;
} DPU_GLB_DBG_DATA0_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int dbg_data1 : 32;
    } reg;
} DPU_GLB_DBG_DATA1_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int dbg_data2 : 32;
    } reg;
} DPU_GLB_DBG_DATA2_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int dbg_data3 : 32;
    } reg;
} DPU_GLB_DBG_DATA3_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int dbg_data4 : 32;
    } reg;
} DPU_GLB_DBG_DATA4_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int dbg_flush_en : 4;
        unsigned int reserved : 28;
    } reg;
} DPU_GLB_DBG_FLUSH_EN_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int dbg_flush_clr : 4;
        unsigned int reserved : 28;
    } reg;
} DPU_GLB_DBG_FLUSH_CLR_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int dbg_flush_st : 4;
        unsigned int reserved : 28;
    } reg;
} DPU_GLB_DBG_FLUSH_ST_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int dbg_data5 : 32;
    } reg;
} DPU_GLB_DBG_DATA5_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int dbg_data6 : 32;
    } reg;
} DPU_GLB_DBG_DATA6_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int dbg_data7 : 32;
    } reg;
} DPU_GLB_DBG_DATA7_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int mask_sta_en : 1;
        unsigned int reserved_0 : 14;
        unsigned int mask_sta_layer_id : 5;
        unsigned int mask_sta_scene_id : 3;
        unsigned int mask_sta_format : 4;
        unsigned int mask_sta_vsync_sel : 3;
        unsigned int reserved_1 : 2;
    } reg;
} DPU_GLB_MASK_CTRL0_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int mask_sta_height : 14;
        unsigned int mask_sta_width : 14;
        unsigned int reserved : 4;
    } reg;
} DPU_GLB_MASK_CTRL1_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int mask_flush_en : 1;
        unsigned int reserved : 31;
    } reg;
} DPU_GLB_MASK_CTRL2_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int crc_cfg_en0 : 1;
        unsigned int crc_cfg_en1 : 1;
        unsigned int crc_cfg_en2 : 1;
        unsigned int crc_cfg_en3 : 1;
        unsigned int crc_cfg_sel0 : 3;
        unsigned int crc_cfg_sel1 : 3;
        unsigned int crc_cfg_sel2 : 2;
        unsigned int crc_cfg_sel3 : 2;
        unsigned int crc_vsync_sel2 : 3;
        unsigned int crc_vsync_sel3 : 3;
        unsigned int reserved : 12;
    } reg;
} DPU_GLB_CRC_CTRL_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int crc_result0 : 32;
    } reg;
} DPU_GLB_CRC_RESULT0_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int crc_frame_cnt0 : 32;
    } reg;
} DPU_GLB_CRC_FRAME_CNT0_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int crc_result1 : 32;
    } reg;
} DPU_GLB_CRC_RESULT1_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int crc_frame_cnt1 : 32;
    } reg;
} DPU_GLB_CRC_FRAME_CNT1_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int crc_result2 : 32;
    } reg;
} DPU_GLB_CRC_RESULT2_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int crc_frame_cnt2 : 32;
    } reg;
} DPU_GLB_CRC_FRAME_CNT2_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int crc_result3 : 32;
    } reg;
} DPU_GLB_CRC_RESULT3_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int crc_frame_cnt3 : 32;
    } reg;
} DPU_GLB_CRC_FRAME_CNT3_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int crc_int0 : 1;
        unsigned int crc_int1 : 1;
        unsigned int crc_int2 : 1;
        unsigned int crc_int3 : 1;
        unsigned int reserved : 28;
    } reg;
} DPU_GLB_CRC_INT3_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int mclear_ctrl : 29;
        unsigned int dbuf_clr_en : 3;
    } reg;
} DPU_GLB_MCLEAR_CTRL_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int mclear_clear_st : 32;
    } reg;
} DPU_GLB_MCLEAR_CLEAR_ST_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int mclear_clear_ip_st : 32;
    } reg;
} DPU_GLB_MCLEAR_CLEAR_IP_ST_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int dma_mode : 3;
        unsigned int dma_va_en : 1;
        unsigned int dma_rot_en : 1;
        unsigned int dma_hmirr_en : 1;
        unsigned int dma_vmirr_en : 1;
        unsigned int reserved : 25;
    } reg;
} DPU_WCH_DMA_CTRL0_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int dma_pix_fmt : 6;
        unsigned int reserved_0 : 1;
        unsigned int dma_os : 6;
        unsigned int dma_threshold : 3;
        unsigned int dma_qos : 3;
        unsigned int reserved_1 : 13;
    } reg;
} DPU_WCH_DMA_CTRL1_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int dma_img_width : 16;
        unsigned int dma_img_height : 16;
    } reg;
} DPU_WCH_DMA_IMG_SIZE_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int dma_ptr0 : 32;
    } reg;
} DPU_WCH_DMA_PTR0_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int dma_stride0 : 24;
        unsigned int reserved : 8;
    } reg;
} DPU_WCH_DMA_STRIDE0_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int dma_ptr1 : 32;
    } reg;
} DPU_WCH_DMA_PTR1_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int dma_stride1 : 24;
        unsigned int reserved : 8;
    } reg;
} DPU_WCH_DMA_STRIDE1_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int dma_ptr2 : 32;
    } reg;
} DPU_WCH_DMA_PTR2_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int dma_stride2 : 24;
        unsigned int reserved : 8;
    } reg;
} DPU_WCH_DMA_STRIDE2_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int dma_ptr3 : 32;
    } reg;
} DPU_WCH_DMA_PTR3_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int dma_stride3 : 24;
        unsigned int reserved : 8;
    } reg;
} DPU_WCH_DMA_STRIDE3_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int dma_cmp_hdr_ptr_oft : 32;
    } reg;
} DPU_WCH_DMA_CMP_HDR_PTR_OFT_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int dma_ch_segment : 16;
        unsigned int reserved_0 : 4;
        unsigned int dma_ch_segment_en : 1;
        unsigned int reserved_1 : 11;
    } reg;
} DPU_WCH_DMA_CH_SEGMENT_CTRL_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int dma_cmp_yuv_trans : 1;
        unsigned int dma_cmp_tag_en : 1;
        unsigned int dma_cmp_raw_en : 1;
        unsigned int dma_cmp_spblk_compact_num : 5;
        unsigned int dma_cmp_spblk_layout : 2;
        unsigned int dma_cmp_spblk_layout_switch_en : 1;
        unsigned int dma_cmp_scramble_mode : 4;
        unsigned int reserved : 17;
    } reg;
} DPU_WCH_DMA_CMP_CTRL_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int dma_cmp_pic_blks : 24;
        unsigned int reserved : 8;
    } reg;
} DPU_WCH_DMA_CMP_PIC_BLKS_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int dma_cmp_mem_ctrl0 : 32;
    } reg;
} DPU_WCH_DMA_CMP_MEM_CTRL0_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int dma_cmp_mem_ctrl1 : 32;
    } reg;
} DPU_WCH_DMA_CMP_MEM_CTRL1_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int dma_cmp_reserved : 32;
    } reg;
} DPU_WCH_DMA_CMP_RESEVED_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int dma_cmp_info_out0 : 32;
    } reg;
} DPU_WCH_DMA_CMP_INFO_OUT0_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int dma_cmp_info_out1 : 32;
    } reg;
} DPU_WCH_DMA_CMP_INFO_OUT1_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int dma_intr : 8;
        unsigned int wch_intr : 8;
        unsigned int reserved : 16;
    } reg;
} DPU_WCH_DMA_INTR_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int dma_rsv_in0 : 32;
    } reg;
} DPU_WCH_DMA_RSV_IN0_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int dma_rsv_in1 : 32;
    } reg;
} DPU_WCH_DMA_RSV_IN1_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int dma_rsv_in2 : 32;
    } reg;
} DPU_WCH_DMA_RSV_IN2_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int dma_rsv_in3 : 32;
    } reg;
} DPU_WCH_DMA_RSV_IN3_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int dma_monitor_out0 : 32;
    } reg;
} DPU_WCH_DMA_MONITOR_OUT0_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int dma_monitor_out1 : 32;
    } reg;
} DPU_WCH_DMA_MONITOR_OUT1_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int dma_monitor_out2 : 32;
    } reg;
} DPU_WCH_DMA_MONITOR_OUT2_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int dma_monitor_out3 : 32;
    } reg;
} DPU_WCH_DMA_MONITOR_OUT3_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int dma_monitor_out4 : 32;
    } reg;
} DPU_WCH_DMA_MONITOR_OUT4_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int dma_monitor_out5 : 32;
    } reg;
} DPU_WCH_DMA_MONITOR_OUT5_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int wlt_en : 1;
        unsigned int reserved : 31;
    } reg;
} DPU_WCH_WTL_CTRL_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int wlt_line_threshold0 : 16;
        unsigned int wlt_line_threshold1 : 16;
    } reg;
} DPU_WCH_WLT_LINE_THRESHOLD1_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int wlt_line_threshold2 : 16;
        unsigned int wlt_line_threshold3 : 16;
    } reg;
} DPU_WCH_WLT_LINE_THRESHOLD3_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int wlt_base_line_offset : 16;
        unsigned int reserved : 16;
    } reg;
} DPU_WCH_WLT_BASE_LINE_OFFSET_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int wlt_dma_addr : 32;
    } reg;
} DPU_WCH_WLT_DMA_ADDR_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int wlt_dma_qos : 3;
        unsigned int reserved : 29;
    } reg;
} DPU_WCH_WLT_DMA_QOS_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int wlt_y_line_num : 16;
        unsigned int wlt_c_line_num : 16;
    } reg;
} DPU_WCH_WLT_C_LINE_NUM_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int wlt_y_slice_num : 3;
        unsigned int wlt_c_slice_num : 3;
        unsigned int reserved : 26;
    } reg;
} DPU_WCH_WLT_C_SLICE_NUM_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int reg_default : 1;
        unsigned int reserved : 31;
    } reg;
} DPU_WCH_REG_DEFAULT_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int rwch_rd_shadow : 1;
        unsigned int reserved : 31;
    } reg;
} DPU_WCH_CH_RD_SHADOW_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int ch_en : 1;
        unsigned int ch_version_sel : 1;
        unsigned int reserved_0 : 1;
        unsigned int ch_block_en : 1;
        unsigned int ch_last_block : 1;
        unsigned int reserved_1 : 2;
        unsigned int reserved_2 : 1;
        unsigned int ch_dmac_en : 1;
        unsigned int reserved_3 : 7;
        unsigned int reserved_4 : 16;
    } reg;
} DPU_WCH_CH_CTL_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int ch_secu_en : 1;
        unsigned int reserved : 31;
    } reg;
} DPU_WCH_CH_SECU_EN_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int ch_sw_end_req : 1;
        unsigned int reserved : 31;
    } reg;
} DPU_WCH_CH_SW_END_REQ_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int ch_clk_sel : 32;
    } reg;
} DPU_WCH_CH_CLK_SEL_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int ch_clk_en : 32;
    } reg;
} DPU_WCH_CH_CLK_EN_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int ch_dbg0 : 32;
    } reg;
} DPU_WCH_CH_DBG0_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int ch_dbg1 : 32;
    } reg;
} DPU_WCH_CH_DBG1_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int ch_dbg2 : 32;
    } reg;
} DPU_WCH_CH_DBG2_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int ch_dbg3 : 32;
    } reg;
} DPU_WCH_CH_DBG3_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int ch_dbg4 : 32;
    } reg;
} DPU_WCH_CH_DBG4_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int ch_dbg5 : 32;
    } reg;
} DPU_WCH_CH_DBG5_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int frm_end_wch_delay : 32;
    } reg;
} DPU_WCH_FRM_END_WCH_DELAY_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int wb_err_cycle_threshold : 30;
        unsigned int wb_err_ints_unmask : 1;
        unsigned int wb_err_bypass : 1;
    } reg;
} DPU_WCH_WB_ERR_CTRL_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int ch_dbg6 : 32;
    } reg;
} DPU_WCH_CH_DBG6_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int dfc_size_vrt : 13;
        unsigned int reserved_0 : 3;
        unsigned int dfc_size_hrz : 13;
        unsigned int reserved_1 : 3;
    } reg;
} DPU_WCH_DFC_DISP_SIZE_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int dfc_pix_in_num : 1;
        unsigned int reserved : 31;
    } reg;
} DPU_WCH_DFC_PIX_IN_NUM_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int dfc_glb_alpha1 : 10;
        unsigned int reserved_0 : 6;
        unsigned int dfc_glb_alpha0 : 10;
        unsigned int reserved_1 : 6;
    } reg;
} DPU_WCH_DFC_GLB_ALPHA01_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int dfc_ax_swap : 1;
        unsigned int dfc_img_fmt : 5;
        unsigned int dfc_uv_swap : 1;
        unsigned int dfc_rb_swap : 1;
        unsigned int reserved : 24;
    } reg;
} DPU_WCH_DFC_DISP_FMT_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int reserved_0: 6;
        unsigned int reserved_1: 10;
        unsigned int reserved_2: 6;
        unsigned int reserved_3: 10;
    } reg;
} DPU_WCH_DFC_CLIP_CTL_HRZ_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int reserved_0: 6;
        unsigned int reserved_1: 10;
        unsigned int reserved_2: 6;
        unsigned int reserved_3: 10;
    } reg;
} DPU_WCH_DFC_CLIP_CTL_VRZ_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int reserved_0: 1;
        unsigned int reserved_1: 31;
    } reg;
} DPU_WCH_DFC_CTL_CLIP_EN_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int dfc_module_en : 1;
        unsigned int reserved : 31;
    } reg;
} DPU_WCH_DFC_ICG_MODULE_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int dfc_dither_en : 1;
        unsigned int reserved : 31;
    } reg;
} DPU_WCH_DFC_DITHER_ENABLE_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int dfc_left_pad : 6;
        unsigned int reserved_0 : 2;
        unsigned int dfc_right_pad : 6;
        unsigned int reserved_1 : 2;
        unsigned int dfc_top_pad : 6;
        unsigned int reserved_2 : 2;
        unsigned int dfc_bot_pad : 6;
        unsigned int dfc_pad_pre_bypass : 1;
        unsigned int dfc_ctl_pad_enable : 1;
    } reg;
} DPU_WCH_DFC_PADDING_CTL_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int dfc_glb_alpha3 : 10;
        unsigned int reserved_0 : 6;
        unsigned int dfc_glb_alpha2 : 10;
        unsigned int reserved_1 : 6;
    } reg;
} DPU_WCH_DFC_GLB_ALPHA23_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int reserved_0 : 16;
        unsigned int dfc_glb_alpha : 10;
        unsigned int reserved_1 : 6;
    } reg;
} DPU_WCH_DFC_GLB_ALPHA_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int bayer_alpha_handle_mode : 2;
        unsigned int bayer_alpha_shift_mode : 2;
        unsigned int reserved : 28;
    } reg;
} DPU_WCH_DFC_ALPHA_CTL_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int bayer_alpha_thd : 30;
        unsigned int reserved : 2;
    } reg;
} DPU_WCH_DFC_ALPHA_THD_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int reserved_0 : 1;
        unsigned int reserved_1 : 1;
        unsigned int reserved_2 : 1;
        unsigned int bitext_reg_ini_cfg_en : 1;
        unsigned int reserved_3 : 28;
    } reg;
} DPU_WCH_BITEXT_CTL_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int bitext_reg_ini0_0 : 32;
    } reg;
} DPU_WCH_BITEXT_REG_INI0_0_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int bitext_reg_ini0_1 : 32;
    } reg;
} DPU_WCH_BITEXT_REG_INI0_1_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int bitext_reg_ini0_2 : 32;
    } reg;
} DPU_WCH_BITEXT_REG_INI0_2_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int bitext_reg_ini0_3 : 31;
        unsigned int reserved : 1;
    } reg;
} DPU_WCH_BITEXT_REG_INI0_3_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int bitext_reg_ini1_0 : 32;
    } reg;
} DPU_WCH_BITEXT_REG_INI1_0_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int bitext_reg_ini1_1 : 32;
    } reg;
} DPU_WCH_BITEXT_REG_INI1_1_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int bitext_reg_ini1_2 : 32;
    } reg;
} DPU_WCH_BITEXT_REG_INI1_2_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int bitext_reg_ini1_3 : 31;
        unsigned int reserved : 1;
    } reg;
} DPU_WCH_BITEXT_REG_INI1_3_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int bitext_reg_ini2_0 : 32;
    } reg;
} DPU_WCH_BITEXT_REG_INI2_0_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int bitext_reg_ini2_1 : 32;
    } reg;
} DPU_WCH_BITEXT_REG_INI2_1_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int bitext_reg_ini2_2 : 32;
    } reg;
} DPU_WCH_BITEXT_REG_INI2_2_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int bitext_reg_ini2_3 : 31;
        unsigned int reserved : 1;
    } reg;
} DPU_WCH_BITEXT_REG_INI2_3_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int bitext_power_ctrl_c : 2;
        unsigned int reserved : 30;
    } reg;
} DPU_WCH_BITEXT_POWER_CTRL_C_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int bitext_power_ctrl_shift : 2;
        unsigned int reserved : 30;
    } reg;
} DPU_WCH_BITEXT_POWER_CTRL_SHIFT_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int bitext_filt_00 : 5;
        unsigned int reserved : 27;
    } reg;
} DPU_WCH_BITEXT_FILT_00_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int bitext_filt_01 : 5;
        unsigned int reserved : 27;
    } reg;
} DPU_WCH_BITEXT_FILT_01_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int bitext_filt_02 : 5;
        unsigned int reserved : 27;
    } reg;
} DPU_WCH_BITEXT_FILT_02_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int bitext_filt_10 : 5;
        unsigned int reserved : 27;
    } reg;
} DPU_WCH_BITEXT_FILT_10_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int bitext_filt_11 : 5;
        unsigned int reserved : 27;
    } reg;
} DPU_WCH_BITEXT_FILT_11_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int bitext_filt_12 : 5;
        unsigned int reserved : 27;
    } reg;
} DPU_WCH_BITEXT_FILT_12_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int bitext_filt_20 : 5;
        unsigned int reserved : 27;
    } reg;
} DPU_WCH_BITEXT_FILT_20_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int bitext_filt_21 : 5;
        unsigned int reserved : 27;
    } reg;
} DPU_WCH_BITEXT_FILT_21_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int bitext_filt_22 : 5;
        unsigned int reserved : 27;
    } reg;
} DPU_WCH_BITEXT_FILT_22_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int bitext_thd_r0 : 32;
    } reg;
} DPU_WCH_BITEXT_THD_R0_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int bitext_thd_r1 : 16;
        unsigned int reserved : 16;
    } reg;
} DPU_WCH_BITEXT_THD_R1_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int bitext_thd_g0 : 32;
    } reg;
} DPU_WCH_BITEXT_THD_G0_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int bitext_thd_g1 : 16;
        unsigned int reserved : 16;
    } reg;
} DPU_WCH_BITEXT_THD_G1_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int bitext_thd_b0 : 32;
    } reg;
} DPU_WCH_BITEXT_THD_B0_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int bitext_thd_b1 : 16;
        unsigned int reserved : 16;
    } reg;
} DPU_WCH_BITEXT_THD_B1_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int bitext_dbg0 : 32;
    } reg;
} DPU_WCH_BITEXT0_DBG0_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int dither_en : 1;
        unsigned int dither_hifreq_noise_mode : 2;
        unsigned int dither_rgb_shift_mode : 1;
        unsigned int dither_unifrom_en : 1;
        unsigned int bayer_rgb_en : 1;
        unsigned int reserved : 26;
    } reg;
} DPU_WCH_DITHER_CTL1_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int dither_tri_thd10_b : 10;
        unsigned int dither_tri_thd10_g : 10;
        unsigned int dither_tri_thd10_r : 10;
        unsigned int reserved : 2;
    } reg;
} DPU_WCH_DITHER_TRI_THD10_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int dither_tri_thd10_uni_b : 10;
        unsigned int dither_tri_thd10_uni_g : 10;
        unsigned int dither_tri_thd10_uni_r : 10;
        unsigned int reserved : 2;
    } reg;
} DPU_WCH_DITHER_TRI_THD10_UNI_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int reserved_0 : 16;
        unsigned int bayer_offset : 12;
        unsigned int reserved_1 : 4;
    } reg;
} DPU_WCH_BAYER_CTL_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int bayer_matrix_part1 : 32;
    } reg;
} DPU_WCH_BAYER_MATRIX_PART1_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int bayer_matrix_part0 : 32;
    } reg;
} DPU_WCH_BAYER_MATRIX_PART0_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int scf_en_hscl_str : 1;
        unsigned int scf_en_hscl_str_a : 1;
        unsigned int reserved : 30;
    } reg;
} DPU_WCH_SCF_EN_HSCL_STR_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int scf_en_vscl_str : 1;
        unsigned int scf_en_vscl_str_a : 1;
        unsigned int reserved : 30;
    } reg;
} DPU_WCH_SCF_EN_VSCL_STR_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int scf_h_v_order : 1;
        unsigned int reserved : 31;
    } reg;
} DPU_WCH_SCF_H_V_ORDER_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int reserved_0: 1;
        unsigned int reserved_1: 1;
        unsigned int reserved_2: 1;
        unsigned int reserved_3: 1;
        unsigned int reserved_4: 28;
    } reg;
} DPU_WCH_SCF_CH_CORE_GT_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int scf_input_height : 13;
        unsigned int reserved_0 : 3;
        unsigned int scf_input_width : 13;
        unsigned int reserved_1 : 3;
    } reg;
} DPU_WCH_SCF_INPUT_WIDTH_HEIGHT_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int scf_output_height : 13;
        unsigned int reserved_0 : 3;
        unsigned int scf_output_width : 13;
        unsigned int reserved_1 : 3;
    } reg;
} DPU_WCH_SCF_OUTPUT_WIDTH_HEIGHT_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int scf_hcoef_mem_ctrl : 4;
        unsigned int scf_vcoef_mem_ctrl : 4;
        unsigned int reserved : 24;
    } reg;
} DPU_WCH_SCF_COEF_MEM_CTRL_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int scf_en_hscl_en : 1;
        unsigned int reserved : 31;
    } reg;
} DPU_WCH_SCF_EN_HSCL_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int scf_en_vscl_en : 1;
        unsigned int scf_out_buffer_en : 1;
        unsigned int reserved : 30;
    } reg;
} DPU_WCH_SCF_EN_VSCL_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int scf_acc_hscl : 30;
        unsigned int reserved : 2;
    } reg;
} DPU_WCH_SCF_ACC_HSCL_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int scf_acc_hscl1 : 19;
        unsigned int reserved : 13;
    } reg;
} DPU_WCH_SCF_ACC_HSCL1_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int scf_inc_hscl : 24;
        unsigned int reserved : 8;
    } reg;
} DPU_WCH_SCF_INC_HSCL_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int scf_acc_vscl : 30;
        unsigned int reserved : 2;
    } reg;
} DPU_WCH_SCF_ACC_VSCL_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int scf_acc_vscl1 : 19;
        unsigned int reserved : 13;
    } reg;
} DPU_WCH_SCF_ACC_VSCL1_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int scf_inc_vscl : 24;
        unsigned int reserved : 8;
    } reg;
} DPU_WCH_SCF_INC_VSCL_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int scf_sw_rst : 1;
        unsigned int reserved : 31;
    } reg;
} DPU_WCH_SCF_EN_NONLINEAR_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int scf_en_mmp : 1;
        unsigned int reserved : 31;
    } reg;
} DPU_WCH_SCF_EN_MMP_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int scf_debug_h0 : 32;
    } reg;
} DPU_WCH_SCF_DB_H0_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int scf_debug_h1 : 32;
    } reg;
} DPU_WCH_SCF_DB_H1_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int scf_debug_v0 : 32;
    } reg;
} DPU_WCH_SCF_DB_V0_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int scf_debug_v1 : 32;
    } reg;
} DPU_WCH_SCF_DB_V1_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int scf_lb_mem_ctrl : 4;
        unsigned int reserved : 28;
    } reg;
} DPU_WCH_SCF_LB_MEM_CTRL_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int pcsc_idc0 : 11;
        unsigned int reserved_0: 5;
        unsigned int pcsc_idc1 : 11;
        unsigned int reserved_1: 5;
    } reg;
} DPU_WCH_PCSC_IDC0_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int pcsc_idc2 : 11;
        unsigned int reserved : 21;
    } reg;
} DPU_WCH_PCSC_IDC2_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int pcsc_odc0 : 11;
        unsigned int reserved_0: 5;
        unsigned int pcsc_odc1 : 11;
        unsigned int reserved_1: 5;
    } reg;
} DPU_WCH_PCSC_ODC0_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int pcsc_odc2 : 11;
        unsigned int reserved : 21;
    } reg;
} DPU_WCH_PCSC_ODC2_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int pcsc_p00 : 17;
        unsigned int reserved : 15;
    } reg;
} DPU_WCH_PCSC_P00_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int pcsc_p01 : 17;
        unsigned int reserved : 15;
    } reg;
} DPU_WCH_PCSC_P01_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int pcsc_p02 : 17;
        unsigned int reserved : 15;
    } reg;
} DPU_WCH_PCSC_P02_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int pcsc_p10 : 17;
        unsigned int reserved : 15;
    } reg;
} DPU_WCH_PCSC_P10_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int pcsc_p11 : 17;
        unsigned int reserved : 15;
    } reg;
} DPU_WCH_PCSC_P11_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int pcsc_p12 : 17;
        unsigned int reserved : 15;
    } reg;
} DPU_WCH_PCSC_P12_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int pcsc_p20 : 17;
        unsigned int reserved : 15;
    } reg;
} DPU_WCH_PCSC_P20_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int pcsc_p21 : 17;
        unsigned int reserved : 15;
    } reg;
} DPU_WCH_PCSC_P21_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int pcsc_p22 : 17;
        unsigned int reserved : 15;
    } reg;
} DPU_WCH_PCSC_P22_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int pcsc_module_en : 1;
        unsigned int reserved : 31;
    } reg;
} DPU_WCH_PCSC_ICG_MODULE_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int post_clip_size_vrt : 13;
        unsigned int reserved_0 : 3;
        unsigned int post_clip_size_hrz : 13;
        unsigned int reserved_1 : 3;
    } reg;
} DPU_WCH_POST_CLIP_DISP_SIZE_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int post_clip_right : 6;
        unsigned int reserved_0 : 10;
        unsigned int post_clip_left : 6;
        unsigned int reserved_1 : 10;
    } reg;
} DPU_WCH_POST_CLIP_CTL_HRZ_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int post_clip_bot : 6;
        unsigned int reserved_0 : 10;
        unsigned int post_clip_top : 6;
        unsigned int reserved_1 : 10;
    } reg;
} DPU_WCH_POST_CLIP_CTL_VRZ_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int post_clip_enable : 1;
        unsigned int reserved : 31;
    } reg;
} DPU_WCH_POST_CLIP_EN_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int csc_idc0 : 11;
        unsigned int reserved_0: 5;
        unsigned int csc_idc1 : 11;
        unsigned int reserved_1: 5;
    } reg;
} DPU_WCH_CSC_IDC0_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int csc_idc2 : 11;
        unsigned int reserved : 21;
    } reg;
} DPU_WCH_CSC_IDC2_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int csc_odc0 : 11;
        unsigned int reserved_0: 5;
        unsigned int csc_odc1 : 11;
        unsigned int reserved_1: 5;
    } reg;
} DPU_WCH_CSC_ODC0_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int csc_odc2 : 11;
        unsigned int reserved : 21;
    } reg;
} DPU_WCH_CSC_ODC2_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int csc_p00 : 17;
        unsigned int reserved : 15;
    } reg;
} DPU_WCH_CSC_P00_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int csc_p01 : 17;
        unsigned int reserved : 15;
    } reg;
} DPU_WCH_CSC_P01_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int csc_p02 : 17;
        unsigned int reserved : 15;
    } reg;
} DPU_WCH_CSC_P02_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int csc_p10 : 17;
        unsigned int reserved : 15;
    } reg;
} DPU_WCH_CSC_P10_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int csc_p11 : 17;
        unsigned int reserved : 15;
    } reg;
} DPU_WCH_CSC_P11_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int csc_p12 : 17;
        unsigned int reserved : 15;
    } reg;
} DPU_WCH_CSC_P12_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int csc_p20 : 17;
        unsigned int reserved : 15;
    } reg;
} DPU_WCH_CSC_P20_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int csc_p21 : 17;
        unsigned int reserved : 15;
    } reg;
} DPU_WCH_CSC_P21_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int csc_p22 : 17;
        unsigned int reserved : 15;
    } reg;
} DPU_WCH_CSC_P22_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int csc_module_en : 1;
        unsigned int reserved : 31;
    } reg;
} DPU_WCH_CSC_ICG_MODULE_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int rot_first_lns_en : 1;
        unsigned int rot_first_lns : 5;
        unsigned int reserved : 26;
    } reg;
} DPU_WCH_ROT_FIRST_LNS_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int rot_state : 32;
    } reg;
} DPU_WCH_ROT_STATE_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int rot_mem_ctrl : 4;
        unsigned int rot_auto_lp_en : 1;
        unsigned int reserved : 27;
    } reg;
} DPU_WCH_ROT_MEM_CTRL_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int rot_img_width : 13;
        unsigned int reserved_0 : 3;
        unsigned int rot_img_height : 13;
        unsigned int reserved_1 : 3;
    } reg;
} DPU_WCH_ROT_SIZE_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int rot_yuv422_mode : 1;
        unsigned int reserved : 31;
    } reg;
} DPU_WCH_ROT_422_MODE_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int ch_debug_sel : 4;
        unsigned int reserved : 28;
    } reg;
} DPU_WCH_CH_DEBUG_SEL_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int dma_addr_ext0 : 4;
        unsigned int reserved : 28;
    } reg;
} DPU_WCH_DMA_ADDR_EXT_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int reg_ctrl_scene_id : 3;
        unsigned int reserved : 29;
    } reg;
} DPU_WCH_REG_CTRL_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int reg_ctrl_flush_en : 1;
        unsigned int reserved : 31;
    } reg;
} DPU_WCH_REG_CTRL_FLUSH_EN_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int reg_ctrl_dbg : 32;
    } reg;
} DPU_WCH_REG_CTRL_DEBUG_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int reg_ctrl_state : 32;
    } reg;
} DPU_WCH_REG_CTRL_STATE_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int r_lb_debug0 : 32;
    } reg;
} DPU_WCH_R_LB_DEBUG0_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int r_lb_debug1 : 32;
    } reg;
} DPU_WCH_R_LB_DEBUG1_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int r_lb_debug2 : 32;
    } reg;
} DPU_WCH_R_LB_DEBUG2_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int r_lb_debug3 : 32;
    } reg;
} DPU_WCH_R_LB_DEBUG3_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int reg_ctrl_layer_id : 5;
        unsigned int reserved : 27;
    } reg;
} DPU_WCH_REG_CTRL_LAYER_ID_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int scf_video_6tap_coef : 32;
    } reg;
} DPU_WCH_V0_SCF_VIDEO_6TAP_COEF_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int scf_video_5tap_coef : 32;
    } reg;
} DPU_WCH_V0_SCF_VIDEO_5TAP_COEF_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int scf_video_4tap_coef : 32;
    } reg;
} DPU_WCH_V0_SCF_VIDEO_4TAP_COEF_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int axi_sel_0 : 2;
        unsigned int reserved : 30;
    } reg;
} DPU_DBCU_SMARTDMA_0_AXI_SEL_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int axi_sel_1 : 2;
        unsigned int reserved : 30;
    } reg;
} DPU_DBCU_SMARTDMA_1_AXI_SEL_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int axi_sel_2 : 2;
        unsigned int reserved : 30;
    } reg;
} DPU_DBCU_SMARTDMA_2_AXI_SEL_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int axi_sel_3 : 2;
        unsigned int reserved : 30;
    } reg;
} DPU_DBCU_SMARTDMA_3_AXI_SEL_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int axi_sel_4 : 2;
        unsigned int reserved : 30;
    } reg;
} DPU_DBCU_WCH_0_AXI_SEL_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int axi_sel_5 : 2;
        unsigned int reserved : 30;
    } reg;
} DPU_DBCU_WCH_1_AXI_SEL_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int axi_sel_6 : 2;
        unsigned int reserved : 30;
    } reg;
} DPU_DBCU_WCH_2_AXI_SEL_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int axi_sel_7 : 2;
        unsigned int reserved : 30;
    } reg;
} DPU_DBCU_CMDLIST_AXI_SEL_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int arcache0_addr_thrh : 32;
    } reg;
} DPU_DBCU_ARCACH0_ADDR_THRH_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int arcache0_addr_thrl : 32;
    } reg;
} DPU_DBCU_ARCACH0_ADDR_THRL_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int arcache0_value0 : 4;
        unsigned int arcache0_addr_thrh_h4bit : 4;
        unsigned int reserved : 24;
    } reg;
} DPU_DBCU_ARCACH0_VALUE0_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int arcache0_value1 : 4;
        unsigned int arcache0_addr_thrl_h4bit : 4;
        unsigned int reserved : 24;
    } reg;
} DPU_DBCU_ARCACH0_VALUE1_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int arcache1_addr_thrh : 32;
    } reg;
} DPU_DBCU_ARCACH1_ADDR_THRH_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int arcache1_addr_thrl : 32;
    } reg;
} DPU_DBCU_ARCACH1_ADDR_THRL_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int arcache1_value0 : 4;
        unsigned int arcache1_addr_thrh_h4bit : 4;
        unsigned int reserved : 24;
    } reg;
} DPU_DBCU_ARCACH1_VALUE0_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int arcache1_value1 : 4;
        unsigned int arcache1_addr_thrl_h4bit : 4;
        unsigned int reserved : 24;
    } reg;
} DPU_DBCU_ARCACH1_VALUE1_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int awcache0_addr_thrh : 32;
    } reg;
} DPU_DBCU_AWCACH0_ADDR_THRH_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int awcache0_addr_thrl : 32;
    } reg;
} DPU_DBCU_AWCACH0_ADDR_THRL_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int awcache0_value0 : 4;
        unsigned int awcache0_addr_thrh_h4bit : 4;
        unsigned int reserved : 24;
    } reg;
} DPU_DBCU_AWCACH0_VALUE0_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int awcache0_value1 : 4;
        unsigned int awcache0_addr_thrl_h4bit : 4;
        unsigned int reserved : 24;
    } reg;
} DPU_DBCU_AWCACH0_VALUE1_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int arcache2_addr_thrh : 32;
    } reg;
} DPU_DBCU_ARCACH2_ADDR_THRH_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int arcache2_addr_thrl : 32;
    } reg;
} DPU_DBCU_ARCACH2_ADDR_THRL_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int arcache2_value0 : 4;
        unsigned int arcache2_addr_thrh_h4bit : 4;
        unsigned int reserved : 24;
    } reg;
} DPU_DBCU_ARCACH2_VALUE0_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int arcache2_value1 : 4;
        unsigned int arcache2_addr_thrl_h4bit : 4;
        unsigned int reserved : 24;
    } reg;
} DPU_DBCU_ARCACH2_VALUE1_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int awcache2_addr_thrh : 32;
    } reg;
} DPU_DBCU_AWCACH2_ADDR_THRH_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int awcache2_addr_thrl : 32;
    } reg;
} DPU_DBCU_AWCACH2_ADDR_THRL_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int awcache2_value0 : 4;
        unsigned int awcache2_addr_thrh_h4bit : 4;
        unsigned int reserved : 24;
    } reg;
} DPU_DBCU_AWCACH2_VALUE0_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int awcache2_value1 : 4;
        unsigned int awcache2_addr_thrl_h4bit : 4;
        unsigned int reserved : 24;
    } reg;
} DPU_DBCU_AWCACH2_VALUE1_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int arcache3_addr_thrh : 32;
    } reg;
} DPU_DBCU_ARCACH3_ADDR_THRH_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int arcache3_addr_thrl : 32;
    } reg;
} DPU_DBCU_ARCACH3_ADDR_THRL_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int arcache3_value0 : 4;
        unsigned int arcache3_addr_thrh_h4bit : 4;
        unsigned int reserved : 24;
    } reg;
} DPU_DBCU_ARCACH3_VALUE0_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int arcache3_value1 : 4;
        unsigned int arcache3_addr_thrl_h4bit : 4;
        unsigned int reserved : 24;
    } reg;
} DPU_DBCU_ARCACH3_VALUE1_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int mid_0 : 8;
        unsigned int reserved : 24;
    } reg;
} DPU_DBCU_SMARTDMA_0_MID_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int mid_1 : 8;
        unsigned int reserved : 24;
    } reg;
} DPU_DBCU_SMARTDMA_1_MID_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int mid_2 : 8;
        unsigned int reserved : 24;
    } reg;
} DPU_DBCU_SMARTDMA_2_MID_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int mid_3 : 8;
        unsigned int reserved : 24;
    } reg;
} DPU_DBCU_SMARTDMA_3_MID_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int mid_4 : 8;
        unsigned int reserved : 24;
    } reg;
} DPU_DBCU_WCH_0_MID_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int mid_5 : 8;
        unsigned int reserved : 24;
    } reg;
} DPU_DBCU_WCH_1_MID_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int mid_6 : 8;
        unsigned int reserved : 24;
    } reg;
} DPU_DBCU_WCH_2_MID_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int mid_7 : 8;
        unsigned int reserved : 24;
    } reg;
} DPU_DBCU_CMDLIST_MID_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int ch_sid_0 : 8;
        unsigned int ch_secsid_0 : 1;
        unsigned int reserved_0 : 2;
        unsigned int ch_ssid_0 : 8;
        unsigned int ch_ssidv_0 : 1;
        unsigned int ch_sc_hint_0 : 4;
        unsigned int ch_gid_0 : 4;
        unsigned int ch_gid_en_0 : 1;
        unsigned int ch_ptl_as_ful_0 : 1;
        unsigned int reserved_1 : 2;
    } reg;
} DPU_DBCU_MMU_ID_ATTR_0_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int ch_sid_1 : 8;
        unsigned int ch_secsid_1 : 1;
        unsigned int reserved_0 : 2;
        unsigned int ch_ssid_1 : 8;
        unsigned int ch_ssidv_1 : 1;
        unsigned int ch_sc_hint_1 : 4;
        unsigned int ch_gid_1 : 4;
        unsigned int ch_gid_en_1 : 1;
        unsigned int ch_ptl_as_ful_1 : 1;
        unsigned int reserved_1 : 2;
    } reg;
} DPU_DBCU_MMU_ID_ATTR_1_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int ch_sid_2 : 8;
        unsigned int ch_secsid_2 : 1;
        unsigned int reserved_0 : 2;
        unsigned int ch_ssid_2 : 8;
        unsigned int ch_ssidv_2 : 1;
        unsigned int ch_sc_hint_2 : 4;
        unsigned int ch_gid_2 : 4;
        unsigned int ch_gid_en_2 : 1;
        unsigned int ch_ptl_as_ful_2 : 1;
        unsigned int reserved_1 : 2;
    } reg;
} DPU_DBCU_MMU_ID_ATTR_2_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int ch_sid_3 : 8;
        unsigned int ch_secsid_3 : 1;
        unsigned int reserved_0 : 2;
        unsigned int ch_ssid_3 : 8;
        unsigned int ch_ssidv_3 : 1;
        unsigned int ch_sc_hint_3 : 4;
        unsigned int ch_gid_3 : 4;
        unsigned int ch_gid_en_3 : 1;
        unsigned int ch_ptl_as_ful_3 : 1;
        unsigned int reserved_1 : 2;
    } reg;
} DPU_DBCU_MMU_ID_ATTR_3_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int ch_sid_4 : 8;
        unsigned int ch_secsid_4 : 1;
        unsigned int reserved_0 : 2;
        unsigned int ch_ssid_4 : 8;
        unsigned int ch_ssidv_4 : 1;
        unsigned int ch_sc_hint_4 : 4;
        unsigned int ch_gid_4 : 4;
        unsigned int ch_gid_en_4 : 1;
        unsigned int ch_ptl_as_ful_4 : 1;
        unsigned int reserved_1 : 2;
    } reg;
} DPU_DBCU_MMU_ID_ATTR_4_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int ch_sid_5 : 8;
        unsigned int ch_secsid_5 : 1;
        unsigned int reserved_0 : 2;
        unsigned int ch_ssid_5 : 8;
        unsigned int ch_ssidv_5 : 1;
        unsigned int ch_sc_hint_5 : 4;
        unsigned int ch_gid_5 : 4;
        unsigned int ch_gid_en_5 : 1;
        unsigned int ch_ptl_as_ful_5 : 1;
        unsigned int reserved_1 : 2;
    } reg;
} DPU_DBCU_MMU_ID_ATTR_5_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int ch_sid_6 : 8;
        unsigned int ch_secsid_6 : 1;
        unsigned int reserved_0 : 2;
        unsigned int ch_ssid_6 : 8;
        unsigned int ch_ssidv_6 : 1;
        unsigned int ch_sc_hint_6 : 4;
        unsigned int ch_gid_6 : 4;
        unsigned int ch_gid_en_6 : 1;
        unsigned int ch_ptl_as_ful_6 : 1;
        unsigned int reserved_1 : 2;
    } reg;
} DPU_DBCU_MMU_ID_ATTR_6_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int ch_sid_7 : 8;
        unsigned int ch_secsid_7 : 1;
        unsigned int reserved_0 : 2;
        unsigned int ch_ssid_7 : 8;
        unsigned int ch_ssidv_7 : 1;
        unsigned int ch_sc_hint_7 : 4;
        unsigned int ch_gid_7 : 4;
        unsigned int ch_gid_en_7 : 1;
        unsigned int ch_ptl_as_ful_7 : 1;
        unsigned int reserved_1 : 2;
    } reg;
} DPU_DBCU_MMU_ID_ATTR_7_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int ch_sid_8 : 8;
        unsigned int ch_secsid_8 : 1;
        unsigned int reserved_0 : 2;
        unsigned int ch_ssid_8 : 8;
        unsigned int ch_ssidv_8 : 1;
        unsigned int ch_sc_hint_8 : 4;
        unsigned int ch_gid_8 : 4;
        unsigned int ch_gid_en_8 : 1;
        unsigned int ch_ptl_as_ful_8 : 1;
        unsigned int reserved_1 : 2;
    } reg;
} DPU_DBCU_MMU_ID_ATTR_8_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int ch_sid_9 : 8;
        unsigned int ch_secsid_9 : 1;
        unsigned int reserved_0 : 2;
        unsigned int ch_ssid_9 : 8;
        unsigned int ch_ssidv_9 : 1;
        unsigned int ch_sc_hint_9 : 4;
        unsigned int ch_gid_9 : 4;
        unsigned int ch_gid_en_9 : 1;
        unsigned int ch_ptl_as_ful_9 : 1;
        unsigned int reserved_1 : 2;
    } reg;
} DPU_DBCU_MMU_ID_ATTR_9_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int ch_sid_10 : 8;
        unsigned int ch_secsid_10 : 1;
        unsigned int reserved_0 : 2;
        unsigned int ch_ssid_10 : 8;
        unsigned int ch_ssidv_10 : 1;
        unsigned int ch_sc_hint_10 : 4;
        unsigned int ch_gid_10 : 4;
        unsigned int ch_gid_en_10 : 1;
        unsigned int ch_ptl_as_ful_10 : 1;
        unsigned int reserved_1 : 2;
    } reg;
} DPU_DBCU_MMU_ID_ATTR_10_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int ch_sid_11 : 8;
        unsigned int ch_secsid_11 : 1;
        unsigned int reserved_0 : 2;
        unsigned int ch_ssid_11 : 8;
        unsigned int ch_ssidv_11 : 1;
        unsigned int ch_sc_hint_11 : 4;
        unsigned int ch_gid_11 : 4;
        unsigned int ch_gid_en_11 : 1;
        unsigned int ch_ptl_as_ful_11 : 1;
        unsigned int reserved_1 : 2;
    } reg;
} DPU_DBCU_MMU_ID_ATTR_11_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int ch_sid_12 : 8;
        unsigned int ch_secsid_12 : 1;
        unsigned int reserved_0 : 2;
        unsigned int ch_ssid_12 : 8;
        unsigned int ch_ssidv_12 : 1;
        unsigned int ch_sc_hint_12 : 4;
        unsigned int ch_gid_12 : 4;
        unsigned int ch_gid_en_12 : 1;
        unsigned int ch_ptl_as_ful_12 : 1;
        unsigned int reserved_1 : 2;
    } reg;
} DPU_DBCU_MMU_ID_ATTR_12_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int ch_sid_13 : 8;
        unsigned int ch_secsid_13 : 1;
        unsigned int reserved_0 : 2;
        unsigned int ch_ssid_13 : 8;
        unsigned int ch_ssidv_13 : 1;
        unsigned int ch_sc_hint_13 : 4;
        unsigned int ch_gid_13 : 4;
        unsigned int ch_gid_en_13 : 1;
        unsigned int ch_ptl_as_ful_13 : 1;
        unsigned int reserved_1 : 2;
    } reg;
} DPU_DBCU_MMU_ID_ATTR_13_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int ch_sid_14 : 8;
        unsigned int ch_secsid_14 : 1;
        unsigned int reserved_0 : 2;
        unsigned int ch_ssid_14 : 8;
        unsigned int ch_ssidv_14 : 1;
        unsigned int ch_sc_hint_14 : 4;
        unsigned int ch_gid_14 : 4;
        unsigned int ch_gid_en_14 : 1;
        unsigned int ch_ptl_as_ful_14 : 1;
        unsigned int reserved_1 : 2;
    } reg;
} DPU_DBCU_MMU_ID_ATTR_14_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int ch_sid_15 : 8;
        unsigned int ch_secsid_15 : 1;
        unsigned int reserved_0 : 2;
        unsigned int ch_ssid_15 : 8;
        unsigned int ch_ssidv_15 : 1;
        unsigned int ch_sc_hint_15 : 4;
        unsigned int ch_gid_15 : 4;
        unsigned int ch_gid_en_15 : 1;
        unsigned int ch_ptl_as_ful_15 : 1;
        unsigned int reserved_1 : 2;
    } reg;
} DPU_DBCU_MMU_ID_ATTR_15_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int ch_sid_16 : 8;
        unsigned int ch_secsid_16 : 1;
        unsigned int reserved_0 : 2;
        unsigned int ch_ssid_16 : 8;
        unsigned int ch_ssidv_16 : 1;
        unsigned int ch_sc_hint_16 : 4;
        unsigned int ch_gid_16 : 4;
        unsigned int ch_gid_en_16 : 1;
        unsigned int ch_ptl_as_ful_16 : 1;
        unsigned int reserved_1 : 2;
    } reg;
} DPU_DBCU_MMU_ID_ATTR_16_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int ch_sid_17 : 8;
        unsigned int ch_secsid_17 : 1;
        unsigned int reserved_0 : 2;
        unsigned int ch_ssid_17 : 8;
        unsigned int ch_ssidv_17 : 1;
        unsigned int ch_sc_hint_17 : 4;
        unsigned int ch_gid_17 : 4;
        unsigned int ch_gid_en_17 : 1;
        unsigned int ch_ptl_as_ful_17 : 1;
        unsigned int reserved_1 : 2;
    } reg;
} DPU_DBCU_MMU_ID_ATTR_17_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int ch_sid_18 : 8;
        unsigned int ch_secsid_18 : 1;
        unsigned int reserved_0 : 2;
        unsigned int ch_ssid_18 : 8;
        unsigned int ch_ssidv_18 : 1;
        unsigned int ch_sc_hint_18 : 4;
        unsigned int ch_gid_18 : 4;
        unsigned int ch_gid_en_18 : 1;
        unsigned int ch_ptl_as_ful_18 : 1;
        unsigned int reserved_1 : 2;
    } reg;
} DPU_DBCU_MMU_ID_ATTR_18_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int ch_sid_19 : 8;
        unsigned int ch_secsid_19 : 1;
        unsigned int reserved_0 : 2;
        unsigned int ch_ssid_19 : 8;
        unsigned int ch_ssidv_19 : 1;
        unsigned int ch_sc_hint_19 : 4;
        unsigned int ch_gid_19 : 4;
        unsigned int ch_gid_en_19 : 1;
        unsigned int ch_ptl_as_ful_19 : 1;
        unsigned int reserved_1 : 2;
    } reg;
} DPU_DBCU_MMU_ID_ATTR_19_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int ch_sid_20 : 8;
        unsigned int ch_secsid_20 : 1;
        unsigned int reserved_0 : 2;
        unsigned int ch_ssid_20 : 8;
        unsigned int ch_ssidv_20 : 1;
        unsigned int ch_sc_hint_20 : 4;
        unsigned int ch_gid_20 : 4;
        unsigned int ch_gid_en_20 : 1;
        unsigned int ch_ptl_as_ful_20 : 1;
        unsigned int reserved_1 : 2;
    } reg;
} DPU_DBCU_MMU_ID_ATTR_20_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int ch_sid_21 : 8;
        unsigned int ch_secsid_21 : 1;
        unsigned int reserved_0 : 2;
        unsigned int ch_ssid_21 : 8;
        unsigned int ch_ssidv_21 : 1;
        unsigned int ch_sc_hint_21 : 4;
        unsigned int ch_gid_21 : 4;
        unsigned int ch_gid_en_21 : 1;
        unsigned int ch_ptl_as_ful_21 : 1;
        unsigned int reserved_1 : 2;
    } reg;
} DPU_DBCU_MMU_ID_ATTR_21_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int ch_sid_22 : 8;
        unsigned int ch_secsid_22 : 1;
        unsigned int reserved_0 : 2;
        unsigned int ch_ssid_22 : 8;
        unsigned int ch_ssidv_22 : 1;
        unsigned int ch_sc_hint_22 : 4;
        unsigned int ch_gid_22 : 4;
        unsigned int ch_gid_en_22 : 1;
        unsigned int ch_ptl_as_ful_22 : 1;
        unsigned int reserved_1 : 2;
    } reg;
} DPU_DBCU_MMU_ID_ATTR_22_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int ch_sid_23 : 8;
        unsigned int ch_secsid_23 : 1;
        unsigned int reserved_0 : 2;
        unsigned int ch_ssid_23 : 8;
        unsigned int ch_ssidv_23 : 1;
        unsigned int ch_sc_hint_23 : 4;
        unsigned int ch_gid_23 : 4;
        unsigned int ch_gid_en_23 : 1;
        unsigned int ch_ptl_as_ful_23 : 1;
        unsigned int reserved_1 : 2;
    } reg;
} DPU_DBCU_MMU_ID_ATTR_23_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int ch_sid_24 : 8;
        unsigned int ch_secsid_24 : 1;
        unsigned int reserved_0 : 2;
        unsigned int ch_ssid_24 : 8;
        unsigned int ch_ssidv_24 : 1;
        unsigned int ch_sc_hint_24 : 4;
        unsigned int ch_gid_24 : 4;
        unsigned int ch_gid_en_24 : 1;
        unsigned int ch_ptl_as_ful_24 : 1;
        unsigned int reserved_1 : 2;
    } reg;
} DPU_DBCU_MMU_ID_ATTR_24_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int ch_sid_25 : 8;
        unsigned int ch_secsid_25 : 1;
        unsigned int reserved_0 : 2;
        unsigned int ch_ssid_25 : 8;
        unsigned int ch_ssidv_25 : 1;
        unsigned int ch_sc_hint_25 : 4;
        unsigned int ch_gid_25 : 4;
        unsigned int ch_gid_en_25 : 1;
        unsigned int ch_ptl_as_ful_25 : 1;
        unsigned int reserved_1 : 2;
    } reg;
} DPU_DBCU_MMU_ID_ATTR_25_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int ch_sid_26 : 8;
        unsigned int ch_secsid_26 : 1;
        unsigned int reserved_0 : 2;
        unsigned int ch_ssid_26 : 8;
        unsigned int ch_ssidv_26 : 1;
        unsigned int ch_sc_hint_26 : 4;
        unsigned int ch_gid_26 : 4;
        unsigned int ch_gid_en_26 : 1;
        unsigned int ch_ptl_as_ful_26 : 1;
        unsigned int reserved_1 : 2;
    } reg;
} DPU_DBCU_MMU_ID_ATTR_26_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int ch_sid_27 : 8;
        unsigned int ch_secsid_27 : 1;
        unsigned int reserved_0 : 2;
        unsigned int ch_ssid_27 : 8;
        unsigned int ch_ssidv_27 : 1;
        unsigned int ch_sc_hint_27 : 4;
        unsigned int ch_gid_27 : 4;
        unsigned int ch_gid_en_27 : 1;
        unsigned int ch_ptl_as_ful_27 : 1;
        unsigned int reserved_1 : 2;
    } reg;
} DPU_DBCU_MMU_ID_ATTR_27_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int ch_sid_28 : 8;
        unsigned int ch_secsid_28 : 1;
        unsigned int reserved_0 : 2;
        unsigned int ch_ssid_28 : 8;
        unsigned int ch_ssidv_28 : 1;
        unsigned int ch_sc_hint_28 : 4;
        unsigned int ch_gid_28 : 4;
        unsigned int ch_gid_en_28 : 1;
        unsigned int ch_ptl_as_ful_28 : 1;
        unsigned int reserved_1 : 2;
    } reg;
} DPU_DBCU_MMU_ID_ATTR_28_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int ch_sid_29 : 8;
        unsigned int ch_secsid_29 : 1;
        unsigned int reserved_0 : 2;
        unsigned int ch_ssid_29 : 8;
        unsigned int ch_ssidv_29 : 1;
        unsigned int ch_sc_hint_29 : 4;
        unsigned int ch_gid_29 : 4;
        unsigned int ch_gid_en_29 : 1;
        unsigned int ch_ptl_as_ful_29 : 1;
        unsigned int reserved_1 : 2;
    } reg;
} DPU_DBCU_MMU_ID_ATTR_29_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int ch_sid_30 : 8;
        unsigned int ch_secsid_30 : 1;
        unsigned int reserved_0 : 2;
        unsigned int ch_ssid_30 : 8;
        unsigned int ch_ssidv_30 : 1;
        unsigned int ch_sc_hint_30 : 4;
        unsigned int ch_gid_30 : 4;
        unsigned int ch_gid_en_30 : 1;
        unsigned int ch_ptl_as_ful_30 : 1;
        unsigned int reserved_1 : 2;
    } reg;
} DPU_DBCU_MMU_ID_ATTR_30_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int ch_sid_31 : 8;
        unsigned int ch_secsid_31 : 1;
        unsigned int reserved_0 : 2;
        unsigned int ch_ssid_31 : 8;
        unsigned int ch_ssidv_31 : 1;
        unsigned int ch_sc_hint_31 : 4;
        unsigned int ch_gid_31 : 4;
        unsigned int ch_gid_en_31 : 1;
        unsigned int ch_ptl_as_ful_31 : 1;
        unsigned int reserved_1 : 2;
    } reg;
} DPU_DBCU_MMU_ID_ATTR_31_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int ch_sid_32 : 8;
        unsigned int ch_secsid_32 : 1;
        unsigned int reserved_0 : 2;
        unsigned int ch_ssid_32 : 8;
        unsigned int ch_ssidv_32 : 1;
        unsigned int ch_sc_hint_32 : 4;
        unsigned int ch_gid_32 : 4;
        unsigned int ch_gid_en_32 : 1;
        unsigned int ch_ptl_as_ful_32 : 1;
        unsigned int reserved_1 : 2;
    } reg;
} DPU_DBCU_MMU_ID_ATTR_32_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int ch_sid_33 : 8;
        unsigned int ch_secsid_33 : 1;
        unsigned int reserved_0 : 2;
        unsigned int ch_ssid_33 : 8;
        unsigned int ch_ssidv_33 : 1;
        unsigned int ch_sc_hint_33 : 4;
        unsigned int ch_gid_33 : 4;
        unsigned int ch_gid_en_33 : 1;
        unsigned int ch_ptl_as_ful_33 : 1;
        unsigned int reserved_1 : 2;
    } reg;
} DPU_DBCU_MMU_ID_ATTR_33_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int ch_sid_34 : 8;
        unsigned int ch_secsid_34 : 1;
        unsigned int reserved_0 : 2;
        unsigned int ch_ssid_34 : 8;
        unsigned int ch_ssidv_34 : 1;
        unsigned int ch_sc_hint_34 : 4;
        unsigned int ch_gid_34 : 4;
        unsigned int ch_gid_en_34 : 1;
        unsigned int ch_ptl_as_ful_34 : 1;
        unsigned int reserved_1 : 2;
    } reg;
} DPU_DBCU_MMU_ID_ATTR_34_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int ch_sid_35 : 8;
        unsigned int ch_secsid_35 : 1;
        unsigned int reserved_0 : 2;
        unsigned int ch_ssid_35 : 8;
        unsigned int ch_ssidv_35 : 1;
        unsigned int ch_sc_hint_35 : 4;
        unsigned int ch_gid_35 : 4;
        unsigned int ch_gid_en_35 : 1;
        unsigned int ch_ptl_as_ful_35 : 1;
        unsigned int reserved_1 : 2;
    } reg;
} DPU_DBCU_MMU_ID_ATTR_35_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int ch_sid_36 : 8;
        unsigned int ch_secsid_36 : 1;
        unsigned int reserved_0 : 2;
        unsigned int ch_ssid_36 : 8;
        unsigned int ch_ssidv_36 : 1;
        unsigned int ch_sc_hint_36 : 4;
        unsigned int ch_gid_36 : 4;
        unsigned int ch_gid_en_36 : 1;
        unsigned int ch_ptl_as_ful_36 : 1;
        unsigned int reserved_1 : 2;
    } reg;
} DPU_DBCU_MMU_ID_ATTR_36_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int ch_sid_37 : 8;
        unsigned int ch_secsid_37 : 1;
        unsigned int reserved_0 : 2;
        unsigned int ch_ssid_37 : 8;
        unsigned int ch_ssidv_37 : 1;
        unsigned int ch_sc_hint_37 : 4;
        unsigned int ch_gid_37 : 4;
        unsigned int ch_gid_en_37 : 1;
        unsigned int ch_ptl_as_ful_37 : 1;
        unsigned int reserved_1 : 2;
    } reg;
} DPU_DBCU_MMU_ID_ATTR_37_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int ch_sid_38 : 8;
        unsigned int ch_secsid_38 : 1;
        unsigned int reserved_0 : 2;
        unsigned int ch_ssid_38 : 8;
        unsigned int ch_ssidv_38 : 1;
        unsigned int ch_sc_hint_38 : 4;
        unsigned int ch_gid_38 : 4;
        unsigned int ch_gid_en_38 : 1;
        unsigned int ch_ptl_as_ful_38 : 1;
        unsigned int reserved_1 : 2;
    } reg;
} DPU_DBCU_MMU_ID_ATTR_38_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int ch_sid_39 : 8;
        unsigned int ch_secsid_39 : 1;
        unsigned int reserved_0 : 2;
        unsigned int ch_ssid_39 : 8;
        unsigned int ch_ssidv_39 : 1;
        unsigned int ch_sc_hint_39 : 4;
        unsigned int ch_gid_39 : 4;
        unsigned int ch_gid_en_39 : 1;
        unsigned int ch_ptl_as_ful_39 : 1;
        unsigned int reserved_1 : 2;
    } reg;
} DPU_DBCU_MMU_ID_ATTR_39_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int ch_sid_40 : 8;
        unsigned int ch_secsid_40 : 1;
        unsigned int reserved_0 : 2;
        unsigned int ch_ssid_40 : 8;
        unsigned int ch_ssidv_40 : 1;
        unsigned int ch_sc_hint_40 : 4;
        unsigned int ch_gid_40 : 4;
        unsigned int ch_gid_en_40 : 1;
        unsigned int ch_ptl_as_ful_40 : 1;
        unsigned int reserved_1 : 2;
    } reg;
} DPU_DBCU_MMU_ID_ATTR_40_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int ch_sid_41 : 8;
        unsigned int ch_secsid_41 : 1;
        unsigned int reserved_0 : 2;
        unsigned int ch_ssid_41 : 8;
        unsigned int ch_ssidv_41 : 1;
        unsigned int ch_sc_hint_41 : 4;
        unsigned int ch_gid_41 : 4;
        unsigned int ch_gid_en_41 : 1;
        unsigned int ch_ptl_as_ful_41 : 1;
        unsigned int reserved_1 : 2;
    } reg;
} DPU_DBCU_MMU_ID_ATTR_41_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int ch_sid_42 : 8;
        unsigned int ch_secsid_42 : 1;
        unsigned int reserved_0 : 2;
        unsigned int ch_ssid_42 : 8;
        unsigned int ch_ssidv_42 : 1;
        unsigned int ch_sc_hint_42 : 4;
        unsigned int ch_gid_42 : 4;
        unsigned int ch_gid_en_42 : 1;
        unsigned int ch_ptl_as_ful_42 : 1;
        unsigned int reserved_1 : 2;
    } reg;
} DPU_DBCU_MMU_ID_ATTR_42_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int ch_sid_43 : 8;
        unsigned int ch_secsid_43 : 1;
        unsigned int reserved_0 : 2;
        unsigned int ch_ssid_43 : 8;
        unsigned int ch_ssidv_43 : 1;
        unsigned int ch_sc_hint_43 : 4;
        unsigned int ch_gid_43 : 4;
        unsigned int ch_gid_en_43 : 1;
        unsigned int ch_ptl_as_ful_43 : 1;
        unsigned int reserved_1 : 2;
    } reg;
} DPU_DBCU_MMU_ID_ATTR_43_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int ch_sid_44 : 8;
        unsigned int ch_secsid_44 : 1;
        unsigned int reserved_0 : 2;
        unsigned int ch_ssid_44 : 8;
        unsigned int ch_ssidv_44 : 1;
        unsigned int ch_sc_hint_44 : 4;
        unsigned int ch_gid_44 : 4;
        unsigned int ch_gid_en_44 : 1;
        unsigned int ch_ptl_as_ful_44 : 1;
        unsigned int reserved_1 : 2;
    } reg;
} DPU_DBCU_MMU_ID_ATTR_44_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int ch_sid_45 : 8;
        unsigned int ch_secsid_45 : 1;
        unsigned int reserved_0 : 2;
        unsigned int ch_ssid_45 : 8;
        unsigned int ch_ssidv_45 : 1;
        unsigned int ch_sc_hint_45 : 4;
        unsigned int ch_gid_45 : 4;
        unsigned int ch_gid_en_45 : 1;
        unsigned int ch_ptl_as_ful_45 : 1;
        unsigned int reserved_1 : 2;
    } reg;
} DPU_DBCU_MMU_ID_ATTR_45_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int ch_sid_46 : 8;
        unsigned int ch_secsid_46 : 1;
        unsigned int reserved_0 : 2;
        unsigned int ch_ssid_46 : 8;
        unsigned int ch_ssidv_46 : 1;
        unsigned int ch_sc_hint_46 : 4;
        unsigned int ch_gid_46 : 4;
        unsigned int ch_gid_en_46 : 1;
        unsigned int ch_ptl_as_ful_46 : 1;
        unsigned int reserved_1 : 2;
    } reg;
} DPU_DBCU_MMU_ID_ATTR_46_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int ch_sid_47 : 8;
        unsigned int ch_secsid_47 : 1;
        unsigned int reserved_0 : 2;
        unsigned int ch_ssid_47 : 8;
        unsigned int ch_ssidv_47 : 1;
        unsigned int ch_sc_hint_47 : 4;
        unsigned int ch_gid_47 : 4;
        unsigned int ch_gid_en_47 : 1;
        unsigned int ch_ptl_as_ful_47 : 1;
        unsigned int reserved_1 : 2;
    } reg;
} DPU_DBCU_MMU_ID_ATTR_47_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int ch_sid_48 : 8;
        unsigned int ch_secsid_48 : 1;
        unsigned int reserved_0 : 2;
        unsigned int ch_ssid_48 : 8;
        unsigned int ch_ssidv_48 : 1;
        unsigned int ch_sc_hint_48 : 4;
        unsigned int ch_gid_48 : 4;
        unsigned int ch_gid_en_48 : 1;
        unsigned int ch_ptl_as_ful_48 : 1;
        unsigned int reserved_1 : 2;
    } reg;
} DPU_DBCU_MMU_ID_ATTR_48_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int ch_sid_49 : 8;
        unsigned int ch_secsid_49 : 1;
        unsigned int reserved_0 : 2;
        unsigned int ch_ssid_49 : 8;
        unsigned int ch_ssidv_49 : 1;
        unsigned int ch_sc_hint_49 : 4;
        unsigned int ch_gid_49 : 4;
        unsigned int ch_gid_en_49 : 1;
        unsigned int ch_ptl_as_ful_49 : 1;
        unsigned int reserved_1 : 2;
    } reg;
} DPU_DBCU_MMU_ID_ATTR_49_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int ch_sid_50 : 8;
        unsigned int ch_secsid_50 : 1;
        unsigned int reserved_0 : 2;
        unsigned int ch_ssid_50 : 8;
        unsigned int ch_ssidv_50 : 1;
        unsigned int ch_sc_hint_50 : 4;
        unsigned int ch_gid_50 : 4;
        unsigned int ch_gid_en_50 : 1;
        unsigned int ch_ptl_as_ful_50 : 1;
        unsigned int reserved_1 : 2;
    } reg;
} DPU_DBCU_MMU_ID_ATTR_50_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int ch_sid_51 : 8;
        unsigned int ch_secsid_51 : 1;
        unsigned int reserved_0 : 2;
        unsigned int ch_ssid_51 : 8;
        unsigned int ch_ssidv_51 : 1;
        unsigned int ch_sc_hint_51 : 4;
        unsigned int ch_gid_51 : 4;
        unsigned int ch_gid_en_51 : 1;
        unsigned int ch_ptl_as_ful_51 : 1;
        unsigned int reserved_1 : 2;
    } reg;
} DPU_DBCU_MMU_ID_ATTR_51_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int ch_sid_52 : 8;
        unsigned int ch_secsid_52 : 1;
        unsigned int reserved_0 : 2;
        unsigned int ch_ssid_52 : 8;
        unsigned int ch_ssidv_52 : 1;
        unsigned int ch_sc_hint_52 : 4;
        unsigned int ch_gid_52 : 4;
        unsigned int ch_gid_en_52 : 1;
        unsigned int ch_ptl_as_ful_52 : 1;
        unsigned int reserved_1 : 2;
    } reg;
} DPU_DBCU_MMU_ID_ATTR_52_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int ch_sid_53 : 8;
        unsigned int ch_secsid_53 : 1;
        unsigned int reserved_0 : 2;
        unsigned int ch_ssid_53 : 8;
        unsigned int ch_ssidv_53 : 1;
        unsigned int ch_sc_hint_53 : 4;
        unsigned int ch_gid_53 : 4;
        unsigned int ch_gid_en_53 : 1;
        unsigned int ch_ptl_as_ful_53 : 1;
        unsigned int reserved_1 : 2;
    } reg;
} DPU_DBCU_MMU_ID_ATTR_53_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int ch_sid_54 : 8;
        unsigned int ch_secsid_54 : 1;
        unsigned int reserved_0 : 2;
        unsigned int ch_ssid_54 : 8;
        unsigned int ch_ssidv_54 : 1;
        unsigned int ch_sc_hint_54 : 4;
        unsigned int ch_gid_54 : 4;
        unsigned int ch_gid_en_54 : 1;
        unsigned int ch_ptl_as_ful_54 : 1;
        unsigned int reserved_1 : 2;
    } reg;
} DPU_DBCU_MMU_ID_ATTR_54_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int ch_sid_55 : 8;
        unsigned int ch_secsid_55 : 1;
        unsigned int reserved_0 : 2;
        unsigned int ch_ssid_55 : 8;
        unsigned int ch_ssidv_55 : 1;
        unsigned int ch_sc_hint_55 : 4;
        unsigned int ch_gid_55 : 4;
        unsigned int ch_gid_en_55 : 1;
        unsigned int ch_ptl_as_ful_55 : 1;
        unsigned int reserved_1 : 2;
    } reg;
} DPU_DBCU_MMU_ID_ATTR_55_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int ch_sid_56 : 8;
        unsigned int ch_secsid_56 : 1;
        unsigned int reserved_0 : 2;
        unsigned int ch_ssid_56 : 8;
        unsigned int ch_ssidv_56 : 1;
        unsigned int ch_sc_hint_56 : 4;
        unsigned int ch_gid_56 : 4;
        unsigned int ch_gid_en_56 : 1;
        unsigned int ch_ptl_as_ful_56 : 1;
        unsigned int reserved_1 : 2;
    } reg;
} DPU_DBCU_MMU_ID_ATTR_56_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int ch_sid_57 : 8;
        unsigned int ch_secsid_57 : 1;
        unsigned int reserved_0 : 2;
        unsigned int ch_ssid_57 : 8;
        unsigned int ch_ssidv_57 : 1;
        unsigned int ch_sc_hint_57 : 4;
        unsigned int ch_gid_57 : 4;
        unsigned int ch_gid_en_57 : 1;
        unsigned int ch_ptl_as_ful_57 : 1;
        unsigned int reserved_1 : 2;
    } reg;
} DPU_DBCU_MMU_ID_ATTR_57_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int ch_sid_58 : 8;
        unsigned int ch_secsid_58 : 1;
        unsigned int reserved_0 : 2;
        unsigned int ch_ssid_58 : 8;
        unsigned int ch_ssidv_58 : 1;
        unsigned int ch_sc_hint_58 : 4;
        unsigned int ch_gid_58 : 4;
        unsigned int ch_gid_en_58 : 1;
        unsigned int ch_ptl_as_ful_58 : 1;
        unsigned int reserved_1 : 2;
    } reg;
} DPU_DBCU_MMU_ID_ATTR_58_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int ch_sid_59 : 8;
        unsigned int ch_secsid_59 : 1;
        unsigned int reserved_0 : 2;
        unsigned int ch_ssid_59 : 8;
        unsigned int ch_ssidv_59 : 1;
        unsigned int ch_sc_hint_59 : 4;
        unsigned int ch_gid_59 : 4;
        unsigned int ch_gid_en_59 : 1;
        unsigned int ch_ptl_as_ful_59 : 1;
        unsigned int reserved_1 : 2;
    } reg;
} DPU_DBCU_MMU_ID_ATTR_59_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int ch_sid_60 : 8;
        unsigned int ch_secsid_60 : 1;
        unsigned int reserved_0 : 2;
        unsigned int ch_ssid_60 : 8;
        unsigned int ch_ssidv_60 : 1;
        unsigned int ch_sc_hint_60 : 4;
        unsigned int ch_gid_60 : 4;
        unsigned int ch_gid_en_60 : 1;
        unsigned int ch_ptl_as_ful_60 : 1;
        unsigned int reserved_1 : 2;
    } reg;
} DPU_DBCU_MMU_ID_ATTR_60_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int ch_sid_61 : 8;
        unsigned int ch_secsid_61 : 1;
        unsigned int reserved_0 : 2;
        unsigned int ch_ssid_61 : 8;
        unsigned int ch_ssidv_61 : 1;
        unsigned int ch_sc_hint_61 : 4;
        unsigned int ch_gid_61 : 4;
        unsigned int ch_gid_en_61 : 1;
        unsigned int ch_ptl_as_ful_61 : 1;
        unsigned int reserved_1 : 2;
    } reg;
} DPU_DBCU_MMU_ID_ATTR_61_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int ch_sid_62 : 8;
        unsigned int ch_secsid_62 : 1;
        unsigned int reserved_0 : 2;
        unsigned int ch_ssid_62 : 8;
        unsigned int ch_ssidv_62 : 1;
        unsigned int ch_sc_hint_62 : 4;
        unsigned int ch_gid_62 : 4;
        unsigned int ch_gid_en_62 : 1;
        unsigned int ch_ptl_as_ful_62 : 1;
        unsigned int reserved_1 : 2;
    } reg;
} DPU_DBCU_MMU_ID_ATTR_62_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int ch_sid_63 : 8;
        unsigned int ch_secsid_63 : 1;
        unsigned int reserved_0 : 2;
        unsigned int ch_ssid_63 : 8;
        unsigned int ch_ssidv_63 : 1;
        unsigned int ch_sc_hint_63 : 4;
        unsigned int ch_gid_63 : 4;
        unsigned int ch_gid_en_63 : 1;
        unsigned int ch_ptl_as_ful_63 : 1;
        unsigned int reserved_1 : 2;
    } reg;
} DPU_DBCU_MMU_ID_ATTR_63_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int ch_sid_64 : 8;
        unsigned int ch_secsid_64 : 1;
        unsigned int reserved_0 : 2;
        unsigned int ch_ssid_64 : 8;
        unsigned int ch_ssidv_64 : 1;
        unsigned int ch_sc_hint_64 : 4;
        unsigned int ch_gid_64 : 4;
        unsigned int ch_gid_en_64 : 1;
        unsigned int ch_ptl_as_ful_64 : 1;
        unsigned int reserved_1 : 2;
    } reg;
} DPU_DBCU_MMU_ID_ATTR_64_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int ch_sid_65 : 8;
        unsigned int ch_secsid_65 : 1;
        unsigned int reserved_0 : 2;
        unsigned int ch_ssid_65 : 8;
        unsigned int ch_ssidv_65 : 1;
        unsigned int ch_sc_hint_65 : 4;
        unsigned int ch_gid_65 : 4;
        unsigned int ch_gid_en_65 : 1;
        unsigned int ch_ptl_as_ful_65 : 1;
        unsigned int reserved_1 : 2;
    } reg;
} DPU_DBCU_MMU_ID_ATTR_65_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int ch_sid_66 : 8;
        unsigned int ch_secsid_66 : 1;
        unsigned int reserved_0 : 2;
        unsigned int ch_ssid_66 : 8;
        unsigned int ch_ssidv_66 : 1;
        unsigned int ch_sc_hint_66 : 4;
        unsigned int ch_gid_66 : 4;
        unsigned int ch_gid_en_66 : 1;
        unsigned int ch_ptl_as_ful_66 : 1;
        unsigned int reserved_1 : 2;
    } reg;
} DPU_DBCU_MMU_ID_ATTR_66_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int ch_sid_67 : 8;
        unsigned int ch_secsid_67 : 1;
        unsigned int reserved_0 : 2;
        unsigned int ch_ssid_67 : 8;
        unsigned int ch_ssidv_67 : 1;
        unsigned int ch_sc_hint_67 : 4;
        unsigned int ch_gid_67 : 4;
        unsigned int ch_gid_en_67 : 1;
        unsigned int ch_ptl_as_ful_67 : 1;
        unsigned int reserved_1 : 2;
    } reg;
} DPU_DBCU_MMU_ID_ATTR_67_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int ch_sid_68 : 8;
        unsigned int ch_secsid_68 : 1;
        unsigned int reserved_0 : 2;
        unsigned int ch_ssid_68 : 8;
        unsigned int ch_ssidv_68 : 1;
        unsigned int ch_sc_hint_68 : 4;
        unsigned int ch_gid_68 : 4;
        unsigned int ch_gid_en_68 : 1;
        unsigned int ch_ptl_as_ful_68 : 1;
        unsigned int reserved_1 : 2;
    } reg;
} DPU_DBCU_MMU_ID_ATTR_68_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int ch_sid_69 : 8;
        unsigned int ch_secsid_69 : 1;
        unsigned int reserved_0 : 2;
        unsigned int ch_ssid_69 : 8;
        unsigned int ch_ssidv_69 : 1;
        unsigned int ch_sc_hint_69 : 4;
        unsigned int ch_gid_69 : 4;
        unsigned int ch_gid_en_69 : 1;
        unsigned int ch_ptl_as_ful_69 : 1;
        unsigned int reserved_1 : 2;
    } reg;
} DPU_DBCU_MMU_ID_ATTR_69_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int ch_sid_70 : 8;
        unsigned int ch_secsid_70 : 1;
        unsigned int reserved_0 : 2;
        unsigned int ch_ssid_70 : 8;
        unsigned int ch_ssidv_70 : 1;
        unsigned int ch_sc_hint_70 : 4;
        unsigned int ch_gid_70 : 4;
        unsigned int ch_gid_en_70 : 1;
        unsigned int ch_ptl_as_ful_70 : 1;
        unsigned int reserved_1 : 2;
    } reg;
} DPU_DBCU_MMU_ID_ATTR_70_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int ch_sid_71 : 8;
        unsigned int ch_secsid_71 : 1;
        unsigned int reserved_0 : 2;
        unsigned int ch_ssid_71 : 8;
        unsigned int ch_ssidv_71 : 1;
        unsigned int ch_sc_hint_71 : 4;
        unsigned int ch_gid_71 : 4;
        unsigned int ch_gid_en_71 : 1;
        unsigned int ch_ptl_as_ful_71 : 1;
        unsigned int reserved_1 : 2;
    } reg;
} DPU_DBCU_MMU_ID_ATTR_71_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int ch_sid_72 : 8;
        unsigned int ch_secsid_72 : 1;
        unsigned int reserved_0 : 2;
        unsigned int ch_ssid_72 : 8;
        unsigned int ch_ssidv_72 : 1;
        unsigned int ch_sc_hint_72 : 4;
        unsigned int ch_gid_72 : 4;
        unsigned int ch_gid_en_72 : 1;
        unsigned int ch_ptl_as_ful_72 : 1;
        unsigned int reserved_1 : 2;
    } reg;
} DPU_DBCU_MMU_ID_ATTR_72_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int ch_sid_73 : 8;
        unsigned int ch_secsid_73 : 1;
        unsigned int reserved_0 : 2;
        unsigned int ch_ssid_73 : 8;
        unsigned int ch_ssidv_73 : 1;
        unsigned int ch_sc_hint_73 : 4;
        unsigned int ch_gid_73 : 4;
        unsigned int ch_gid_en_73 : 1;
        unsigned int ch_ptl_as_ful_73 : 1;
        unsigned int reserved_1 : 2;
    } reg;
} DPU_DBCU_MMU_ID_ATTR_73_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int ch_sid_74 : 8;
        unsigned int ch_secsid_74 : 1;
        unsigned int reserved_0 : 2;
        unsigned int ch_ssid_74 : 8;
        unsigned int ch_ssidv_74 : 1;
        unsigned int ch_sc_hint_74 : 4;
        unsigned int ch_gid_74 : 4;
        unsigned int ch_gid_en_74 : 1;
        unsigned int ch_ptl_as_ful_74 : 1;
        unsigned int reserved_1 : 2;
    } reg;
} DPU_DBCU_MMU_ID_ATTR_74_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int ch_sid_75 : 8;
        unsigned int ch_secsid_75 : 1;
        unsigned int reserved_0 : 2;
        unsigned int ch_ssid_75 : 8;
        unsigned int ch_ssidv_75 : 1;
        unsigned int ch_sc_hint_75 : 4;
        unsigned int ch_gid_75 : 4;
        unsigned int ch_gid_en_75 : 1;
        unsigned int ch_ptl_as_ful_75 : 1;
        unsigned int reserved_1 : 2;
    } reg;
} DPU_DBCU_MMU_ID_ATTR_75_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int ch_sid_76 : 8;
        unsigned int ch_secsid_76 : 1;
        unsigned int reserved_0 : 2;
        unsigned int ch_ssid_76 : 8;
        unsigned int ch_ssidv_76 : 1;
        unsigned int ch_sc_hint_76 : 4;
        unsigned int ch_gid_76 : 4;
        unsigned int ch_gid_en_76 : 1;
        unsigned int ch_ptl_as_ful_76 : 1;
        unsigned int reserved_1 : 2;
    } reg;
} DPU_DBCU_MMU_ID_ATTR_76_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int ch_sid_77 : 8;
        unsigned int ch_secsid_77 : 1;
        unsigned int reserved_0 : 2;
        unsigned int ch_ssid_77 : 8;
        unsigned int ch_ssidv_77 : 1;
        unsigned int ch_sc_hint_77 : 4;
        unsigned int ch_gid_77 : 4;
        unsigned int ch_gid_en_77 : 1;
        unsigned int ch_ptl_as_ful_77 : 1;
        unsigned int reserved_1 : 2;
    } reg;
} DPU_DBCU_MMU_ID_ATTR_77_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int ch_sid_78 : 8;
        unsigned int ch_secsid_78 : 1;
        unsigned int reserved_0 : 2;
        unsigned int ch_ssid_78 : 8;
        unsigned int ch_ssidv_78 : 1;
        unsigned int ch_sc_hint_78 : 4;
        unsigned int ch_gid_78 : 4;
        unsigned int ch_gid_en_78 : 1;
        unsigned int ch_ptl_as_ful_78 : 1;
        unsigned int reserved_1 : 2;
    } reg;
} DPU_DBCU_MMU_ID_ATTR_78_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int ch_sid_79 : 8;
        unsigned int ch_secsid_79 : 1;
        unsigned int reserved_0 : 2;
        unsigned int ch_ssid_79 : 8;
        unsigned int ch_ssidv_79 : 1;
        unsigned int ch_sc_hint_79 : 4;
        unsigned int ch_gid_79 : 4;
        unsigned int ch_gid_en_79 : 1;
        unsigned int ch_ptl_as_ful_79 : 1;
        unsigned int reserved_1 : 2;
    } reg;
} DPU_DBCU_MMU_ID_ATTR_79_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int ch_sid_ns_0 : 8;
        unsigned int reserved_0 : 3;
        unsigned int ch_ssid_ns_0 : 8;
        unsigned int ch_ssidv_ns_0 : 1;
        unsigned int ch_sc_hint_ns_0 : 4;
        unsigned int ch_gid_ns_0 : 4;
        unsigned int ch_gid_en_ns_0 : 1;
        unsigned int ch_ptl_as_ful_ns_0 : 1;
        unsigned int reserved_1 : 2;
    } reg;
} DPU_DBCU_MMU_ID_ATTR_NS_0_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int ch_sid_ns_1 : 8;
        unsigned int reserved_0 : 3;
        unsigned int ch_ssid_ns_1 : 8;
        unsigned int ch_ssidv_ns_1 : 1;
        unsigned int ch_sc_hint_ns_1 : 4;
        unsigned int ch_gid_ns_1 : 4;
        unsigned int ch_gid_en_ns_1 : 1;
        unsigned int ch_ptl_as_ful_ns_1 : 1;
        unsigned int reserved_1 : 2;
    } reg;
} DPU_DBCU_MMU_ID_ATTR_NS_1_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int ch_sid_ns_2 : 8;
        unsigned int reserved_0 : 3;
        unsigned int ch_ssid_ns_2 : 8;
        unsigned int ch_ssidv_ns_2 : 1;
        unsigned int ch_sc_hint_ns_2 : 4;
        unsigned int ch_gid_ns_2 : 4;
        unsigned int ch_gid_en_ns_2 : 1;
        unsigned int ch_ptl_as_ful_ns_2 : 1;
        unsigned int reserved_1 : 2;
    } reg;
} DPU_DBCU_MMU_ID_ATTR_NS_2_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int ch_sid_ns_3 : 8;
        unsigned int reserved_0 : 3;
        unsigned int ch_ssid_ns_3 : 8;
        unsigned int ch_ssidv_ns_3 : 1;
        unsigned int ch_sc_hint_ns_3 : 4;
        unsigned int ch_gid_ns_3 : 4;
        unsigned int ch_gid_en_ns_3 : 1;
        unsigned int ch_ptl_as_ful_ns_3 : 1;
        unsigned int reserved_1 : 2;
    } reg;
} DPU_DBCU_MMU_ID_ATTR_NS_3_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int ch_sid_ns_4 : 8;
        unsigned int reserved_0 : 3;
        unsigned int ch_ssid_ns_4 : 8;
        unsigned int ch_ssidv_ns_4 : 1;
        unsigned int ch_sc_hint_ns_4 : 4;
        unsigned int ch_gid_ns_4 : 4;
        unsigned int ch_gid_en_ns_4 : 1;
        unsigned int ch_ptl_as_ful_ns_4 : 1;
        unsigned int reserved_1 : 2;
    } reg;
} DPU_DBCU_MMU_ID_ATTR_NS_4_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int ch_sid_ns_5 : 8;
        unsigned int reserved_0 : 3;
        unsigned int ch_ssid_ns_5 : 8;
        unsigned int ch_ssidv_ns_5 : 1;
        unsigned int ch_sc_hint_ns_5 : 4;
        unsigned int ch_gid_ns_5 : 4;
        unsigned int ch_gid_en_ns_5 : 1;
        unsigned int ch_ptl_as_ful_ns_5 : 1;
        unsigned int reserved_1 : 2;
    } reg;
} DPU_DBCU_MMU_ID_ATTR_NS_5_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int ch_sid_ns_6 : 8;
        unsigned int reserved_0 : 3;
        unsigned int ch_ssid_ns_6 : 8;
        unsigned int ch_ssidv_ns_6 : 1;
        unsigned int ch_sc_hint_ns_6 : 4;
        unsigned int ch_gid_ns_6 : 4;
        unsigned int ch_gid_en_ns_6 : 1;
        unsigned int ch_ptl_as_ful_ns_6 : 1;
        unsigned int reserved_1 : 2;
    } reg;
} DPU_DBCU_MMU_ID_ATTR_NS_6_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int ch_sid_ns_7 : 8;
        unsigned int reserved_0 : 3;
        unsigned int ch_ssid_ns_7 : 8;
        unsigned int ch_ssidv_ns_7 : 1;
        unsigned int ch_sc_hint_ns_7 : 4;
        unsigned int ch_gid_ns_7 : 4;
        unsigned int ch_gid_en_ns_7 : 1;
        unsigned int ch_ptl_as_ful_ns_7 : 1;
        unsigned int reserved_1 : 2;
    } reg;
} DPU_DBCU_MMU_ID_ATTR_NS_7_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int ch_sid_ns_8 : 8;
        unsigned int reserved_0 : 3;
        unsigned int ch_ssid_ns_8 : 8;
        unsigned int ch_ssidv_ns_8 : 1;
        unsigned int ch_sc_hint_ns_8 : 4;
        unsigned int ch_gid_ns_8 : 4;
        unsigned int ch_gid_en_ns_8 : 1;
        unsigned int ch_ptl_as_ful_ns_8 : 1;
        unsigned int reserved_1 : 2;
    } reg;
} DPU_DBCU_MMU_ID_ATTR_NS_8_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int ch_sid_ns_9 : 8;
        unsigned int reserved_0 : 3;
        unsigned int ch_ssid_ns_9 : 8;
        unsigned int ch_ssidv_ns_9 : 1;
        unsigned int ch_sc_hint_ns_9 : 4;
        unsigned int ch_gid_ns_9 : 4;
        unsigned int ch_gid_en_ns_9 : 1;
        unsigned int ch_ptl_as_ful_ns_9 : 1;
        unsigned int reserved_1 : 2;
    } reg;
} DPU_DBCU_MMU_ID_ATTR_NS_9_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int ch_sid_ns_10 : 8;
        unsigned int reserved_0 : 3;
        unsigned int ch_ssid_ns_10 : 8;
        unsigned int ch_ssidv_ns_10 : 1;
        unsigned int ch_sc_hint_ns_10 : 4;
        unsigned int ch_gid_ns_10 : 4;
        unsigned int ch_gid_en_ns_10 : 1;
        unsigned int ch_ptl_as_ful_ns_10 : 1;
        unsigned int reserved_1 : 2;
    } reg;
} DPU_DBCU_MMU_ID_ATTR_NS_10_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int ch_sid_ns_11 : 8;
        unsigned int reserved_0 : 3;
        unsigned int ch_ssid_ns_11 : 8;
        unsigned int ch_ssidv_ns_11 : 1;
        unsigned int ch_sc_hint_ns_11 : 4;
        unsigned int ch_gid_ns_11 : 4;
        unsigned int ch_gid_en_ns_11 : 1;
        unsigned int ch_ptl_as_ful_ns_11 : 1;
        unsigned int reserved_1 : 2;
    } reg;
} DPU_DBCU_MMU_ID_ATTR_NS_11_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int ch_sid_ns_12 : 8;
        unsigned int reserved_0 : 3;
        unsigned int ch_ssid_ns_12 : 8;
        unsigned int ch_ssidv_ns_12 : 1;
        unsigned int ch_sc_hint_ns_12 : 4;
        unsigned int ch_gid_ns_12 : 4;
        unsigned int ch_gid_en_ns_12 : 1;
        unsigned int ch_ptl_as_ful_ns_12 : 1;
        unsigned int reserved_1 : 2;
    } reg;
} DPU_DBCU_MMU_ID_ATTR_NS_12_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int ch_sid_ns_13 : 8;
        unsigned int reserved_0 : 3;
        unsigned int ch_ssid_ns_13 : 8;
        unsigned int ch_ssidv_ns_13 : 1;
        unsigned int ch_sc_hint_ns_13 : 4;
        unsigned int ch_gid_ns_13 : 4;
        unsigned int ch_gid_en_ns_13 : 1;
        unsigned int ch_ptl_as_ful_ns_13 : 1;
        unsigned int reserved_1 : 2;
    } reg;
} DPU_DBCU_MMU_ID_ATTR_NS_13_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int ch_sid_ns_14 : 8;
        unsigned int reserved_0 : 3;
        unsigned int ch_ssid_ns_14 : 8;
        unsigned int ch_ssidv_ns_14 : 1;
        unsigned int ch_sc_hint_ns_14 : 4;
        unsigned int ch_gid_ns_14 : 4;
        unsigned int ch_gid_en_ns_14 : 1;
        unsigned int ch_ptl_as_ful_ns_14 : 1;
        unsigned int reserved_1 : 2;
    } reg;
} DPU_DBCU_MMU_ID_ATTR_NS_14_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int ch_sid_ns_15 : 8;
        unsigned int reserved_0 : 3;
        unsigned int ch_ssid_ns_15 : 8;
        unsigned int ch_ssidv_ns_15 : 1;
        unsigned int ch_sc_hint_ns_15 : 4;
        unsigned int ch_gid_ns_15 : 4;
        unsigned int ch_gid_en_ns_15 : 1;
        unsigned int ch_ptl_as_ful_ns_15 : 1;
        unsigned int reserved_1 : 2;
    } reg;
} DPU_DBCU_MMU_ID_ATTR_NS_15_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int ch_sid_ns_16 : 8;
        unsigned int reserved_0 : 3;
        unsigned int ch_ssid_ns_16 : 8;
        unsigned int ch_ssidv_ns_16 : 1;
        unsigned int ch_sc_hint_ns_16 : 4;
        unsigned int ch_gid_ns_16 : 4;
        unsigned int ch_gid_en_ns_16 : 1;
        unsigned int ch_ptl_as_ful_ns_16 : 1;
        unsigned int reserved_1 : 2;
    } reg;
} DPU_DBCU_MMU_ID_ATTR_NS_16_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int ch_sid_ns_17 : 8;
        unsigned int reserved_0 : 3;
        unsigned int ch_ssid_ns_17 : 8;
        unsigned int ch_ssidv_ns_17 : 1;
        unsigned int ch_sc_hint_ns_17 : 4;
        unsigned int ch_gid_ns_17 : 4;
        unsigned int ch_gid_en_ns_17 : 1;
        unsigned int ch_ptl_as_ful_ns_17 : 1;
        unsigned int reserved_1 : 2;
    } reg;
} DPU_DBCU_MMU_ID_ATTR_NS_17_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int ch_sid_ns_18 : 8;
        unsigned int reserved_0 : 3;
        unsigned int ch_ssid_ns_18 : 8;
        unsigned int ch_ssidv_ns_18 : 1;
        unsigned int ch_sc_hint_ns_18 : 4;
        unsigned int ch_gid_ns_18 : 4;
        unsigned int ch_gid_en_ns_18 : 1;
        unsigned int ch_ptl_as_ful_ns_18 : 1;
        unsigned int reserved_1 : 2;
    } reg;
} DPU_DBCU_MMU_ID_ATTR_NS_18_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int ch_sid_ns_19 : 8;
        unsigned int reserved_0 : 3;
        unsigned int ch_ssid_ns_19 : 8;
        unsigned int ch_ssidv_ns_19 : 1;
        unsigned int ch_sc_hint_ns_19 : 4;
        unsigned int ch_gid_ns_19 : 4;
        unsigned int ch_gid_en_ns_19 : 1;
        unsigned int ch_ptl_as_ful_ns_19 : 1;
        unsigned int reserved_1 : 2;
    } reg;
} DPU_DBCU_MMU_ID_ATTR_NS_19_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int ch_sid_ns_20 : 8;
        unsigned int reserved_0 : 3;
        unsigned int ch_ssid_ns_20 : 8;
        unsigned int ch_ssidv_ns_20 : 1;
        unsigned int ch_sc_hint_ns_20 : 4;
        unsigned int ch_gid_ns_20 : 4;
        unsigned int ch_gid_en_ns_20 : 1;
        unsigned int ch_ptl_as_ful_ns_20 : 1;
        unsigned int reserved_1 : 2;
    } reg;
} DPU_DBCU_MMU_ID_ATTR_NS_20_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int ch_sid_ns_21 : 8;
        unsigned int reserved_0 : 3;
        unsigned int ch_ssid_ns_21 : 8;
        unsigned int ch_ssidv_ns_21 : 1;
        unsigned int ch_sc_hint_ns_21 : 4;
        unsigned int ch_gid_ns_21 : 4;
        unsigned int ch_gid_en_ns_21 : 1;
        unsigned int ch_ptl_as_ful_ns_21 : 1;
        unsigned int reserved_1 : 2;
    } reg;
} DPU_DBCU_MMU_ID_ATTR_NS_21_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int ch_sid_ns_22 : 8;
        unsigned int reserved_0 : 3;
        unsigned int ch_ssid_ns_22 : 8;
        unsigned int ch_ssidv_ns_22 : 1;
        unsigned int ch_sc_hint_ns_22 : 4;
        unsigned int ch_gid_ns_22 : 4;
        unsigned int ch_gid_en_ns_22 : 1;
        unsigned int ch_ptl_as_ful_ns_22 : 1;
        unsigned int reserved_1 : 2;
    } reg;
} DPU_DBCU_MMU_ID_ATTR_NS_22_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int ch_sid_ns_23 : 8;
        unsigned int reserved_0 : 3;
        unsigned int ch_ssid_ns_23 : 8;
        unsigned int ch_ssidv_ns_23 : 1;
        unsigned int ch_sc_hint_ns_23 : 4;
        unsigned int ch_gid_ns_23 : 4;
        unsigned int ch_gid_en_ns_23 : 1;
        unsigned int ch_ptl_as_ful_ns_23 : 1;
        unsigned int reserved_1 : 2;
    } reg;
} DPU_DBCU_MMU_ID_ATTR_NS_23_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int ch_sid_ns_24 : 8;
        unsigned int reserved_0 : 3;
        unsigned int ch_ssid_ns_24 : 8;
        unsigned int ch_ssidv_ns_24 : 1;
        unsigned int ch_sc_hint_ns_24 : 4;
        unsigned int ch_gid_ns_24 : 4;
        unsigned int ch_gid_en_ns_24 : 1;
        unsigned int ch_ptl_as_ful_ns_24 : 1;
        unsigned int reserved_1 : 2;
    } reg;
} DPU_DBCU_MMU_ID_ATTR_NS_24_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int ch_sid_ns_25 : 8;
        unsigned int reserved_0 : 3;
        unsigned int ch_ssid_ns_25 : 8;
        unsigned int ch_ssidv_ns_25 : 1;
        unsigned int ch_sc_hint_ns_25 : 4;
        unsigned int ch_gid_ns_25 : 4;
        unsigned int ch_gid_en_ns_25 : 1;
        unsigned int ch_ptl_as_ful_ns_25 : 1;
        unsigned int reserved_1 : 2;
    } reg;
} DPU_DBCU_MMU_ID_ATTR_NS_25_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int ch_sid_ns_26 : 8;
        unsigned int reserved_0 : 3;
        unsigned int ch_ssid_ns_26 : 8;
        unsigned int ch_ssidv_ns_26 : 1;
        unsigned int ch_sc_hint_ns_26 : 4;
        unsigned int ch_gid_ns_26 : 4;
        unsigned int ch_gid_en_ns_26 : 1;
        unsigned int ch_ptl_as_ful_ns_26 : 1;
        unsigned int reserved_1 : 2;
    } reg;
} DPU_DBCU_MMU_ID_ATTR_NS_26_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int ch_sid_ns_27 : 8;
        unsigned int reserved_0 : 3;
        unsigned int ch_ssid_ns_27 : 8;
        unsigned int ch_ssidv_ns_27 : 1;
        unsigned int ch_sc_hint_ns_27 : 4;
        unsigned int ch_gid_ns_27 : 4;
        unsigned int ch_gid_en_ns_27 : 1;
        unsigned int ch_ptl_as_ful_ns_27 : 1;
        unsigned int reserved_1 : 2;
    } reg;
} DPU_DBCU_MMU_ID_ATTR_NS_27_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int ch_sid_ns_28 : 8;
        unsigned int reserved_0 : 3;
        unsigned int ch_ssid_ns_28 : 8;
        unsigned int ch_ssidv_ns_28 : 1;
        unsigned int ch_sc_hint_ns_28 : 4;
        unsigned int ch_gid_ns_28 : 4;
        unsigned int ch_gid_en_ns_28 : 1;
        unsigned int ch_ptl_as_ful_ns_28 : 1;
        unsigned int reserved_1 : 2;
    } reg;
} DPU_DBCU_MMU_ID_ATTR_NS_28_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int ch_sid_ns_29 : 8;
        unsigned int reserved_0 : 3;
        unsigned int ch_ssid_ns_29 : 8;
        unsigned int ch_ssidv_ns_29 : 1;
        unsigned int ch_sc_hint_ns_29 : 4;
        unsigned int ch_gid_ns_29 : 4;
        unsigned int ch_gid_en_ns_29 : 1;
        unsigned int ch_ptl_as_ful_ns_29 : 1;
        unsigned int reserved_1 : 2;
    } reg;
} DPU_DBCU_MMU_ID_ATTR_NS_29_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int ch_sid_ns_30 : 8;
        unsigned int reserved_0 : 3;
        unsigned int ch_ssid_ns_30 : 8;
        unsigned int ch_ssidv_ns_30 : 1;
        unsigned int ch_sc_hint_ns_30 : 4;
        unsigned int ch_gid_ns_30 : 4;
        unsigned int ch_gid_en_ns_30 : 1;
        unsigned int ch_ptl_as_ful_ns_30 : 1;
        unsigned int reserved_1 : 2;
    } reg;
} DPU_DBCU_MMU_ID_ATTR_NS_30_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int ch_sid_ns_31 : 8;
        unsigned int reserved_0 : 3;
        unsigned int ch_ssid_ns_31 : 8;
        unsigned int ch_ssidv_ns_31 : 1;
        unsigned int ch_sc_hint_ns_31 : 4;
        unsigned int ch_gid_ns_31 : 4;
        unsigned int ch_gid_en_ns_31 : 1;
        unsigned int ch_ptl_as_ful_ns_31 : 1;
        unsigned int reserved_1 : 2;
    } reg;
} DPU_DBCU_MMU_ID_ATTR_NS_31_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int ch_sid_ns_32 : 8;
        unsigned int reserved_0 : 3;
        unsigned int ch_ssid_ns_32 : 8;
        unsigned int ch_ssidv_ns_32 : 1;
        unsigned int ch_sc_hint_ns_32 : 4;
        unsigned int ch_gid_ns_32 : 4;
        unsigned int ch_gid_en_ns_32 : 1;
        unsigned int ch_ptl_as_ful_ns_32 : 1;
        unsigned int reserved_1 : 2;
    } reg;
} DPU_DBCU_MMU_ID_ATTR_NS_32_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int ch_sid_ns_33 : 8;
        unsigned int reserved_0 : 3;
        unsigned int ch_ssid_ns_33 : 8;
        unsigned int ch_ssidv_ns_33 : 1;
        unsigned int ch_sc_hint_ns_33 : 4;
        unsigned int ch_gid_ns_33 : 4;
        unsigned int ch_gid_en_ns_33 : 1;
        unsigned int ch_ptl_as_ful_ns_33 : 1;
        unsigned int reserved_1 : 2;
    } reg;
} DPU_DBCU_MMU_ID_ATTR_NS_33_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int ch_sid_ns_34 : 8;
        unsigned int reserved_0 : 3;
        unsigned int ch_ssid_ns_34 : 8;
        unsigned int ch_ssidv_ns_34 : 1;
        unsigned int ch_sc_hint_ns_34 : 4;
        unsigned int ch_gid_ns_34 : 4;
        unsigned int ch_gid_en_ns_34 : 1;
        unsigned int ch_ptl_as_ful_ns_34 : 1;
        unsigned int reserved_1 : 2;
    } reg;
} DPU_DBCU_MMU_ID_ATTR_NS_34_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int ch_sid_ns_35 : 8;
        unsigned int reserved_0 : 3;
        unsigned int ch_ssid_ns_35 : 8;
        unsigned int ch_ssidv_ns_35 : 1;
        unsigned int ch_sc_hint_ns_35 : 4;
        unsigned int ch_gid_ns_35 : 4;
        unsigned int ch_gid_en_ns_35 : 1;
        unsigned int ch_ptl_as_ful_ns_35 : 1;
        unsigned int reserved_1 : 2;
    } reg;
} DPU_DBCU_MMU_ID_ATTR_NS_35_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int ch_sid_ns_36 : 8;
        unsigned int reserved_0 : 3;
        unsigned int ch_ssid_ns_36 : 8;
        unsigned int ch_ssidv_ns_36 : 1;
        unsigned int ch_sc_hint_ns_36 : 4;
        unsigned int ch_gid_ns_36 : 4;
        unsigned int ch_gid_en_ns_36 : 1;
        unsigned int ch_ptl_as_ful_ns_36 : 1;
        unsigned int reserved_1 : 2;
    } reg;
} DPU_DBCU_MMU_ID_ATTR_NS_36_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int ch_sid_ns_37 : 8;
        unsigned int reserved_0 : 3;
        unsigned int ch_ssid_ns_37 : 8;
        unsigned int ch_ssidv_ns_37 : 1;
        unsigned int ch_sc_hint_ns_37 : 4;
        unsigned int ch_gid_ns_37 : 4;
        unsigned int ch_gid_en_ns_37 : 1;
        unsigned int ch_ptl_as_ful_ns_37 : 1;
        unsigned int reserved_1 : 2;
    } reg;
} DPU_DBCU_MMU_ID_ATTR_NS_37_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int ch_sid_ns_38 : 8;
        unsigned int reserved_0 : 3;
        unsigned int ch_ssid_ns_38 : 8;
        unsigned int ch_ssidv_ns_38 : 1;
        unsigned int ch_sc_hint_ns_38 : 4;
        unsigned int ch_gid_ns_38 : 4;
        unsigned int ch_gid_en_ns_38 : 1;
        unsigned int ch_ptl_as_ful_ns_38 : 1;
        unsigned int reserved_1 : 2;
    } reg;
} DPU_DBCU_MMU_ID_ATTR_NS_38_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int ch_sid_ns_39 : 8;
        unsigned int reserved_0 : 3;
        unsigned int ch_ssid_ns_39 : 8;
        unsigned int ch_ssidv_ns_39 : 1;
        unsigned int ch_sc_hint_ns_39 : 4;
        unsigned int ch_gid_ns_39 : 4;
        unsigned int ch_gid_en_ns_39 : 1;
        unsigned int ch_ptl_as_ful_ns_39 : 1;
        unsigned int reserved_1 : 2;
    } reg;
} DPU_DBCU_MMU_ID_ATTR_NS_39_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int ch_sid_ns_40 : 8;
        unsigned int reserved_0 : 3;
        unsigned int ch_ssid_ns_40 : 8;
        unsigned int ch_ssidv_ns_40 : 1;
        unsigned int ch_sc_hint_ns_40 : 4;
        unsigned int ch_gid_ns_40 : 4;
        unsigned int ch_gid_en_ns_40 : 1;
        unsigned int ch_ptl_as_ful_ns_40 : 1;
        unsigned int reserved_1 : 2;
    } reg;
} DPU_DBCU_MMU_ID_ATTR_NS_40_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int ch_sid_ns_41 : 8;
        unsigned int reserved_0 : 3;
        unsigned int ch_ssid_ns_41 : 8;
        unsigned int ch_ssidv_ns_41 : 1;
        unsigned int ch_sc_hint_ns_41 : 4;
        unsigned int ch_gid_ns_41 : 4;
        unsigned int ch_gid_en_ns_41 : 1;
        unsigned int ch_ptl_as_ful_ns_41 : 1;
        unsigned int reserved_1 : 2;
    } reg;
} DPU_DBCU_MMU_ID_ATTR_NS_41_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int ch_sid_ns_42 : 8;
        unsigned int reserved_0 : 3;
        unsigned int ch_ssid_ns_42 : 8;
        unsigned int ch_ssidv_ns_42 : 1;
        unsigned int ch_sc_hint_ns_42 : 4;
        unsigned int ch_gid_ns_42 : 4;
        unsigned int ch_gid_en_ns_42 : 1;
        unsigned int ch_ptl_as_ful_ns_42 : 1;
        unsigned int reserved_1 : 2;
    } reg;
} DPU_DBCU_MMU_ID_ATTR_NS_42_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int ch_sid_ns_43 : 8;
        unsigned int reserved_0 : 3;
        unsigned int ch_ssid_ns_43 : 8;
        unsigned int ch_ssidv_ns_43 : 1;
        unsigned int ch_sc_hint_ns_43 : 4;
        unsigned int ch_gid_ns_43 : 4;
        unsigned int ch_gid_en_ns_43 : 1;
        unsigned int ch_ptl_as_ful_ns_43 : 1;
        unsigned int reserved_1 : 2;
    } reg;
} DPU_DBCU_MMU_ID_ATTR_NS_43_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int ch_sid_ns_44 : 8;
        unsigned int reserved_0 : 3;
        unsigned int ch_ssid_ns_44 : 8;
        unsigned int ch_ssidv_ns_44 : 1;
        unsigned int ch_sc_hint_ns_44 : 4;
        unsigned int ch_gid_ns_44 : 4;
        unsigned int ch_gid_en_ns_44 : 1;
        unsigned int ch_ptl_as_ful_ns_44 : 1;
        unsigned int reserved_1 : 2;
    } reg;
} DPU_DBCU_MMU_ID_ATTR_NS_44_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int ch_sid_ns_45 : 8;
        unsigned int reserved_0 : 3;
        unsigned int ch_ssid_ns_45 : 8;
        unsigned int ch_ssidv_ns_45 : 1;
        unsigned int ch_sc_hint_ns_45 : 4;
        unsigned int ch_gid_ns_45 : 4;
        unsigned int ch_gid_en_ns_45 : 1;
        unsigned int ch_ptl_as_ful_ns_45 : 1;
        unsigned int reserved_1 : 2;
    } reg;
} DPU_DBCU_MMU_ID_ATTR_NS_45_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int ch_sid_ns_46 : 8;
        unsigned int reserved_0 : 3;
        unsigned int ch_ssid_ns_46 : 8;
        unsigned int ch_ssidv_ns_46 : 1;
        unsigned int ch_sc_hint_ns_46 : 4;
        unsigned int ch_gid_ns_46 : 4;
        unsigned int ch_gid_en_ns_46 : 1;
        unsigned int ch_ptl_as_ful_ns_46 : 1;
        unsigned int reserved_1 : 2;
    } reg;
} DPU_DBCU_MMU_ID_ATTR_NS_46_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int ch_sid_ns_47 : 8;
        unsigned int reserved_0 : 3;
        unsigned int ch_ssid_ns_47 : 8;
        unsigned int ch_ssidv_ns_47 : 1;
        unsigned int ch_sc_hint_ns_47 : 4;
        unsigned int ch_gid_ns_47 : 4;
        unsigned int ch_gid_en_ns_47 : 1;
        unsigned int ch_ptl_as_ful_ns_47 : 1;
        unsigned int reserved_1 : 2;
    } reg;
} DPU_DBCU_MMU_ID_ATTR_NS_47_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int ch_sid_ns_48 : 8;
        unsigned int reserved_0 : 3;
        unsigned int ch_ssid_ns_48 : 8;
        unsigned int ch_ssidv_ns_48 : 1;
        unsigned int ch_sc_hint_ns_48 : 4;
        unsigned int ch_gid_ns_48 : 4;
        unsigned int ch_gid_en_ns_48 : 1;
        unsigned int ch_ptl_as_ful_ns_48 : 1;
        unsigned int reserved_1 : 2;
    } reg;
} DPU_DBCU_MMU_ID_ATTR_NS_48_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int ch_sid_ns_49 : 8;
        unsigned int reserved_0 : 3;
        unsigned int ch_ssid_ns_49 : 8;
        unsigned int ch_ssidv_ns_49 : 1;
        unsigned int ch_sc_hint_ns_49 : 4;
        unsigned int ch_gid_ns_49 : 4;
        unsigned int ch_gid_en_ns_49 : 1;
        unsigned int ch_ptl_as_ful_ns_49 : 1;
        unsigned int reserved_1 : 2;
    } reg;
} DPU_DBCU_MMU_ID_ATTR_NS_49_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int ch_sid_ns_50 : 8;
        unsigned int reserved_0 : 3;
        unsigned int ch_ssid_ns_50 : 8;
        unsigned int ch_ssidv_ns_50 : 1;
        unsigned int ch_sc_hint_ns_50 : 4;
        unsigned int ch_gid_ns_50 : 4;
        unsigned int ch_gid_en_ns_50 : 1;
        unsigned int ch_ptl_as_ful_ns_50 : 1;
        unsigned int reserved_1 : 2;
    } reg;
} DPU_DBCU_MMU_ID_ATTR_NS_50_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int ch_sid_ns_51 : 8;
        unsigned int reserved_0 : 3;
        unsigned int ch_ssid_ns_51 : 8;
        unsigned int ch_ssidv_ns_51 : 1;
        unsigned int ch_sc_hint_ns_51 : 4;
        unsigned int ch_gid_ns_51 : 4;
        unsigned int ch_gid_en_ns_51 : 1;
        unsigned int ch_ptl_as_ful_ns_51 : 1;
        unsigned int reserved_1 : 2;
    } reg;
} DPU_DBCU_MMU_ID_ATTR_NS_51_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int ch_sid_ns_52 : 8;
        unsigned int reserved_0 : 3;
        unsigned int ch_ssid_ns_52 : 8;
        unsigned int ch_ssidv_ns_52 : 1;
        unsigned int ch_sc_hint_ns_52 : 4;
        unsigned int ch_gid_ns_52 : 4;
        unsigned int ch_gid_en_ns_52 : 1;
        unsigned int ch_ptl_as_ful_ns_52 : 1;
        unsigned int reserved_1 : 2;
    } reg;
} DPU_DBCU_MMU_ID_ATTR_NS_52_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int ch_sid_ns_53 : 8;
        unsigned int reserved_0 : 3;
        unsigned int ch_ssid_ns_53 : 8;
        unsigned int ch_ssidv_ns_53 : 1;
        unsigned int ch_sc_hint_ns_53 : 4;
        unsigned int ch_gid_ns_53 : 4;
        unsigned int ch_gid_en_ns_53 : 1;
        unsigned int ch_ptl_as_ful_ns_53 : 1;
        unsigned int reserved_1 : 2;
    } reg;
} DPU_DBCU_MMU_ID_ATTR_NS_53_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int ch_sid_ns_54 : 8;
        unsigned int reserved_0 : 3;
        unsigned int ch_ssid_ns_54 : 8;
        unsigned int ch_ssidv_ns_54 : 1;
        unsigned int ch_sc_hint_ns_54 : 4;
        unsigned int ch_gid_ns_54 : 4;
        unsigned int ch_gid_en_ns_54 : 1;
        unsigned int ch_ptl_as_ful_ns_54 : 1;
        unsigned int reserved_1 : 2;
    } reg;
} DPU_DBCU_MMU_ID_ATTR_NS_54_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int ch_sid_ns_55 : 8;
        unsigned int reserved_0 : 3;
        unsigned int ch_ssid_ns_55 : 8;
        unsigned int ch_ssidv_ns_55 : 1;
        unsigned int ch_sc_hint_ns_55 : 4;
        unsigned int ch_gid_ns_55 : 4;
        unsigned int ch_gid_en_ns_55 : 1;
        unsigned int ch_ptl_as_ful_ns_55 : 1;
        unsigned int reserved_1 : 2;
    } reg;
} DPU_DBCU_MMU_ID_ATTR_NS_55_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int ch_sid_ns_56 : 8;
        unsigned int reserved_0 : 3;
        unsigned int ch_ssid_ns_56 : 8;
        unsigned int ch_ssidv_ns_56 : 1;
        unsigned int ch_sc_hint_ns_56 : 4;
        unsigned int ch_gid_ns_56 : 4;
        unsigned int ch_gid_en_ns_56 : 1;
        unsigned int ch_ptl_as_ful_ns_56 : 1;
        unsigned int reserved_1 : 2;
    } reg;
} DPU_DBCU_MMU_ID_ATTR_NS_56_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int ch_sid_ns_57 : 8;
        unsigned int reserved_0 : 3;
        unsigned int ch_ssid_ns_57 : 8;
        unsigned int ch_ssidv_ns_57 : 1;
        unsigned int ch_sc_hint_ns_57 : 4;
        unsigned int ch_gid_ns_57 : 4;
        unsigned int ch_gid_en_ns_57 : 1;
        unsigned int ch_ptl_as_ful_ns_57 : 1;
        unsigned int reserved_1 : 2;
    } reg;
} DPU_DBCU_MMU_ID_ATTR_NS_57_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int ch_sid_ns_58 : 8;
        unsigned int reserved_0 : 3;
        unsigned int ch_ssid_ns_58 : 8;
        unsigned int ch_ssidv_ns_58 : 1;
        unsigned int ch_sc_hint_ns_58 : 4;
        unsigned int ch_gid_ns_58 : 4;
        unsigned int ch_gid_en_ns_58 : 1;
        unsigned int ch_ptl_as_ful_ns_58 : 1;
        unsigned int reserved_1 : 2;
    } reg;
} DPU_DBCU_MMU_ID_ATTR_NS_58_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int ch_sid_ns_59 : 8;
        unsigned int reserved_0 : 3;
        unsigned int ch_ssid_ns_59 : 8;
        unsigned int ch_ssidv_ns_59 : 1;
        unsigned int ch_sc_hint_ns_59 : 4;
        unsigned int ch_gid_ns_59 : 4;
        unsigned int ch_gid_en_ns_59 : 1;
        unsigned int ch_ptl_as_ful_ns_59 : 1;
        unsigned int reserved_1 : 2;
    } reg;
} DPU_DBCU_MMU_ID_ATTR_NS_59_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int ch_sid_ns_60 : 8;
        unsigned int reserved_0 : 3;
        unsigned int ch_ssid_ns_60 : 8;
        unsigned int ch_ssidv_ns_60 : 1;
        unsigned int ch_sc_hint_ns_60 : 4;
        unsigned int ch_gid_ns_60 : 4;
        unsigned int ch_gid_en_ns_60 : 1;
        unsigned int ch_ptl_as_ful_ns_60 : 1;
        unsigned int reserved_1 : 2;
    } reg;
} DPU_DBCU_MMU_ID_ATTR_NS_60_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int ch_sid_ns_61 : 8;
        unsigned int reserved_0 : 3;
        unsigned int ch_ssid_ns_61 : 8;
        unsigned int ch_ssidv_ns_61 : 1;
        unsigned int ch_sc_hint_ns_61 : 4;
        unsigned int ch_gid_ns_61 : 4;
        unsigned int ch_gid_en_ns_61 : 1;
        unsigned int ch_ptl_as_ful_ns_61 : 1;
        unsigned int reserved_1 : 2;
    } reg;
} DPU_DBCU_MMU_ID_ATTR_NS_61_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int ch_sid_ns_62 : 8;
        unsigned int reserved_0 : 3;
        unsigned int ch_ssid_ns_62 : 8;
        unsigned int ch_ssidv_ns_62 : 1;
        unsigned int ch_sc_hint_ns_62 : 4;
        unsigned int ch_gid_ns_62 : 4;
        unsigned int ch_gid_en_ns_62 : 1;
        unsigned int ch_ptl_as_ful_ns_62 : 1;
        unsigned int reserved_1 : 2;
    } reg;
} DPU_DBCU_MMU_ID_ATTR_NS_62_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int ch_sid_ns_63 : 8;
        unsigned int reserved_0 : 3;
        unsigned int ch_ssid_ns_63 : 8;
        unsigned int ch_ssidv_ns_63 : 1;
        unsigned int ch_sc_hint_ns_63 : 4;
        unsigned int ch_gid_ns_63 : 4;
        unsigned int ch_gid_en_ns_63 : 1;
        unsigned int ch_ptl_as_ful_ns_63 : 1;
        unsigned int reserved_1 : 2;
    } reg;
} DPU_DBCU_MMU_ID_ATTR_NS_63_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int ch_sid_ns_64 : 8;
        unsigned int reserved_0 : 3;
        unsigned int ch_ssid_ns_64 : 8;
        unsigned int ch_ssidv_ns_64 : 1;
        unsigned int ch_sc_hint_ns_64 : 4;
        unsigned int ch_gid_ns_64 : 4;
        unsigned int ch_gid_en_ns_64 : 1;
        unsigned int ch_ptl_as_ful_ns_64 : 1;
        unsigned int reserved_1 : 2;
    } reg;
} DPU_DBCU_MMU_ID_ATTR_NS_64_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int ch_sid_ns_65 : 8;
        unsigned int reserved_0 : 3;
        unsigned int ch_ssid_ns_65 : 8;
        unsigned int ch_ssidv_ns_65 : 1;
        unsigned int ch_sc_hint_ns_65 : 4;
        unsigned int ch_gid_ns_65 : 4;
        unsigned int ch_gid_en_ns_65 : 1;
        unsigned int ch_ptl_as_ful_ns_65 : 1;
        unsigned int reserved_1 : 2;
    } reg;
} DPU_DBCU_MMU_ID_ATTR_NS_65_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int ch_sid_ns_66 : 8;
        unsigned int reserved_0 : 3;
        unsigned int ch_ssid_ns_66 : 8;
        unsigned int ch_ssidv_ns_66 : 1;
        unsigned int ch_sc_hint_ns_66 : 4;
        unsigned int ch_gid_ns_66 : 4;
        unsigned int ch_gid_en_ns_66 : 1;
        unsigned int ch_ptl_as_ful_ns_66 : 1;
        unsigned int reserved_1 : 2;
    } reg;
} DPU_DBCU_MMU_ID_ATTR_NS_66_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int ch_sid_ns_67 : 8;
        unsigned int reserved_0 : 3;
        unsigned int ch_ssid_ns_67 : 8;
        unsigned int ch_ssidv_ns_67 : 1;
        unsigned int ch_sc_hint_ns_67 : 4;
        unsigned int ch_gid_ns_67 : 4;
        unsigned int ch_gid_en_ns_67 : 1;
        unsigned int ch_ptl_as_ful_ns_67 : 1;
        unsigned int reserved_1 : 2;
    } reg;
} DPU_DBCU_MMU_ID_ATTR_NS_67_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int ch_sid_ns_68 : 8;
        unsigned int reserved_0 : 3;
        unsigned int ch_ssid_ns_68 : 8;
        unsigned int ch_ssidv_ns_68 : 1;
        unsigned int ch_sc_hint_ns_68 : 4;
        unsigned int ch_gid_ns_68 : 4;
        unsigned int ch_gid_en_ns_68 : 1;
        unsigned int ch_ptl_as_ful_ns_68 : 1;
        unsigned int reserved_1 : 2;
    } reg;
} DPU_DBCU_MMU_ID_ATTR_NS_68_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int ch_sid_ns_69 : 8;
        unsigned int reserved_0 : 3;
        unsigned int ch_ssid_ns_69 : 8;
        unsigned int ch_ssidv_ns_69 : 1;
        unsigned int ch_sc_hint_ns_69 : 4;
        unsigned int ch_gid_ns_69 : 4;
        unsigned int ch_gid_en_ns_69 : 1;
        unsigned int ch_ptl_as_ful_ns_69 : 1;
        unsigned int reserved_1 : 2;
    } reg;
} DPU_DBCU_MMU_ID_ATTR_NS_69_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int ch_sid_ns_70 : 8;
        unsigned int reserved_0 : 3;
        unsigned int ch_ssid_ns_70 : 8;
        unsigned int ch_ssidv_ns_70 : 1;
        unsigned int ch_sc_hint_ns_70 : 4;
        unsigned int ch_gid_ns_70 : 4;
        unsigned int ch_gid_en_ns_70 : 1;
        unsigned int ch_ptl_as_ful_ns_70 : 1;
        unsigned int reserved_1 : 2;
    } reg;
} DPU_DBCU_MMU_ID_ATTR_NS_70_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int ch_sid_ns_71 : 8;
        unsigned int reserved_0 : 3;
        unsigned int ch_ssid_ns_71 : 8;
        unsigned int ch_ssidv_ns_71 : 1;
        unsigned int ch_sc_hint_ns_71 : 4;
        unsigned int ch_gid_ns_71 : 4;
        unsigned int ch_gid_en_ns_71 : 1;
        unsigned int ch_ptl_as_ful_ns_71 : 1;
        unsigned int reserved_1 : 2;
    } reg;
} DPU_DBCU_MMU_ID_ATTR_NS_71_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int ch_sid_ns_72 : 8;
        unsigned int reserved_0 : 3;
        unsigned int ch_ssid_ns_72 : 8;
        unsigned int ch_ssidv_ns_72 : 1;
        unsigned int ch_sc_hint_ns_72 : 4;
        unsigned int ch_gid_ns_72 : 4;
        unsigned int ch_gid_en_ns_72 : 1;
        unsigned int ch_ptl_as_ful_ns_72 : 1;
        unsigned int reserved_1 : 2;
    } reg;
} DPU_DBCU_MMU_ID_ATTR_NS_72_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int ch_sid_ns_73 : 8;
        unsigned int reserved_0 : 3;
        unsigned int ch_ssid_ns_73 : 8;
        unsigned int ch_ssidv_ns_73 : 1;
        unsigned int ch_sc_hint_ns_73 : 4;
        unsigned int ch_gid_ns_73 : 4;
        unsigned int ch_gid_en_ns_73 : 1;
        unsigned int ch_ptl_as_ful_ns_73 : 1;
        unsigned int reserved_1 : 2;
    } reg;
} DPU_DBCU_MMU_ID_ATTR_NS_73_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int ch_sid_ns_74 : 8;
        unsigned int reserved_0 : 3;
        unsigned int ch_ssid_ns_74 : 8;
        unsigned int ch_ssidv_ns_74 : 1;
        unsigned int ch_sc_hint_ns_74 : 4;
        unsigned int ch_gid_ns_74 : 4;
        unsigned int ch_gid_en_ns_74 : 1;
        unsigned int ch_ptl_as_ful_ns_74 : 1;
        unsigned int reserved_1 : 2;
    } reg;
} DPU_DBCU_MMU_ID_ATTR_NS_74_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int ch_sid_ns_75 : 8;
        unsigned int reserved_0 : 3;
        unsigned int ch_ssid_ns_75 : 8;
        unsigned int ch_ssidv_ns_75 : 1;
        unsigned int ch_sc_hint_ns_75 : 4;
        unsigned int ch_gid_ns_75 : 4;
        unsigned int ch_gid_en_ns_75 : 1;
        unsigned int ch_ptl_as_ful_ns_75 : 1;
        unsigned int reserved_1 : 2;
    } reg;
} DPU_DBCU_MMU_ID_ATTR_NS_75_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int ch_sid_ns_76 : 8;
        unsigned int reserved_0 : 3;
        unsigned int ch_ssid_ns_76 : 8;
        unsigned int ch_ssidv_ns_76 : 1;
        unsigned int ch_sc_hint_ns_76 : 4;
        unsigned int ch_gid_ns_76 : 4;
        unsigned int ch_gid_en_ns_76 : 1;
        unsigned int ch_ptl_as_ful_ns_76 : 1;
        unsigned int reserved_1 : 2;
    } reg;
} DPU_DBCU_MMU_ID_ATTR_NS_76_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int ch_sid_ns_77 : 8;
        unsigned int reserved_0 : 3;
        unsigned int ch_ssid_ns_77 : 8;
        unsigned int ch_ssidv_ns_77 : 1;
        unsigned int ch_sc_hint_ns_77 : 4;
        unsigned int ch_gid_ns_77 : 4;
        unsigned int ch_gid_en_ns_77 : 1;
        unsigned int ch_ptl_as_ful_ns_77 : 1;
        unsigned int reserved_1 : 2;
    } reg;
} DPU_DBCU_MMU_ID_ATTR_NS_77_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int ch_sid_ns_78 : 8;
        unsigned int reserved_0 : 3;
        unsigned int ch_ssid_ns_78 : 8;
        unsigned int ch_ssidv_ns_78 : 1;
        unsigned int ch_sc_hint_ns_78 : 4;
        unsigned int ch_gid_ns_78 : 4;
        unsigned int ch_gid_en_ns_78 : 1;
        unsigned int ch_ptl_as_ful_ns_78 : 1;
        unsigned int reserved_1 : 2;
    } reg;
} DPU_DBCU_MMU_ID_ATTR_NS_78_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int ch_sid_ns_79 : 8;
        unsigned int reserved_0 : 3;
        unsigned int ch_ssid_ns_79 : 8;
        unsigned int ch_ssidv_ns_79 : 1;
        unsigned int ch_sc_hint_ns_79 : 4;
        unsigned int ch_gid_ns_79 : 4;
        unsigned int ch_gid_en_ns_79 : 1;
        unsigned int ch_ptl_as_ful_ns_79 : 1;
        unsigned int reserved_1 : 2;
    } reg;
} DPU_DBCU_MMU_ID_ATTR_NS_79_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int axi2_rid_msk0 : 32;
    } reg;
} DPU_DBCU_AXI2_RID_MSK0_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int axi2_rid_msk1 : 32;
    } reg;
} DPU_DBCU_AXI2_RID_MSK1_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int axi2_wid_msk : 32;
    } reg;
} DPU_DBCU_AXI2_WID_MSK_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int axi2_rd_qos_map : 8;
        unsigned int axi2_wr_qos_map : 8;
        unsigned int axi2_rd_prs_map : 8;
        unsigned int axi2_wr_prs_map : 8;
    } reg;
} DPU_DBCU_AXI2_R_QOS_MAP_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int axi3_rid_msk0 : 32;
    } reg;
} DPU_DBCU_AXI3_RID_MSK0_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int axi3_rid_msk1 : 32;
    } reg;
} DPU_DBCU_AXI3_RID_MSK1_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int axi3_rd_qos_map : 8;
        unsigned int reserved_0 : 8;
        unsigned int axi3_rd_prs_map : 8;
        unsigned int reserved_1 : 8;
    } reg;
} DPU_DBCU_AXI3_R_QOS_MAP_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int axi0_rid_msk0 : 32;
    } reg;
} DPU_DBCU_AXI0_RID_MSK0_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int axi0_rid_msk1 : 32;
    } reg;
} DPU_DBCU_AXI0_RID_MSK1_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int axi0_wid_msk : 32;
    } reg;
} DPU_DBCU_AXI0_WID_MSK_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int axi0_rd_qos_map : 8;
        unsigned int axi0_wr_qos_map : 8;
        unsigned int axi0_rd_prs_map : 8;
        unsigned int axi0_wr_prs_map : 8;
    } reg;
} DPU_DBCU_AXI0_R_QOS_MAP_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int axi1_rid_msk0 : 32;
    } reg;
} DPU_DBCU_AXI1_RID_MSK0_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int axi1_rid_msk1 : 32;
    } reg;
} DPU_DBCU_AXI1_RID_MSK1_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int axi1_rd_qos_map : 8;
        unsigned int reserved_0 : 8;
        unsigned int axi1_rd_prs_map : 8;
        unsigned int reserved_1 : 8;
    } reg;
} DPU_DBCU_AXI1_R_QOS_MAP_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int axi0_bwc_en_high_r : 1;
        unsigned int axi0_saturation_high_r : 14;
        unsigned int axi0_bandwidth_high_r : 13;
        unsigned int reserved : 4;
    } reg;
} DPU_DBCU_AXI0_READ_FLUX_HIGH_CFG_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int axi0_bwc_en_low_r : 1;
        unsigned int axi0_saturation_low_r : 14;
        unsigned int axi0_bandwidth_low_r : 13;
        unsigned int reserved : 4;
    } reg;
} DPU_DBCU_AXI0_READ_FLUX_LOW_CFG_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int axi0_bwc_en_high_w : 1;
        unsigned int axi0_saturation_high_w : 14;
        unsigned int axi0_bandwidth_high_w : 13;
        unsigned int reserved : 4;
    } reg;
} DPU_DBCU_AXI0_WRITE_FLUX_HIGH_CFG_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int axi0_bwc_en_low_w : 1;
        unsigned int axi0_saturation_low_w : 14;
        unsigned int axi0_bandwidth_low_w : 13;
        unsigned int reserved : 4;
    } reg;
} DPU_DBCU_AXI0_WRITE_FLUX_LOW_CFG_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int axi1_bwc_en_high_r : 1;
        unsigned int axi1_saturation_high_r : 14;
        unsigned int axi1_bandwidth_high_r : 13;
        unsigned int reserved : 4;
    } reg;
} DPU_DBCU_AXI1_READ_FLUX_HIGH_CFG_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int axi1_bwc_en_low_r : 1;
        unsigned int axi1_saturation_low_r : 14;
        unsigned int axi1_bandwidth_low_r : 13;
        unsigned int reserved : 4;
    } reg;
} DPU_DBCU_AXI1_READ_FLUX_LOW_CFG_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int axi2_bwc_en_high_r : 1;
        unsigned int axi2_saturation_high_r : 14;
        unsigned int axi2_bandwidth_high_r : 13;
        unsigned int reserved : 4;
    } reg;
} DPU_DBCU_AXI2_READ_FLUX_HIGH_CFG_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int axi2_bwc_en_low_r : 1;
        unsigned int axi2_saturation_low_r : 14;
        unsigned int axi2_bandwidth_low_r : 13;
        unsigned int reserved : 4;
    } reg;
} DPU_DBCU_AXI2_READ_FLUX_LOW_CFG_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int axi2_bwc_en_high_w : 1;
        unsigned int axi2_saturation_high_w : 14;
        unsigned int axi2_bandwidth_high_w : 13;
        unsigned int reserved : 4;
    } reg;
} DPU_DBCU_AXI2_WRITE_FLUX_HIGH_CFG_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int axi2_bwc_en_low_w : 1;
        unsigned int axi2_saturation_low_w : 14;
        unsigned int axi2_bandwidth_low_w : 13;
        unsigned int reserved : 4;
    } reg;
} DPU_DBCU_AXI2_WRITE_FLUX_LOW_CFG_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int axi3_bwc_en_high_r : 1;
        unsigned int axi3_saturation_high_r : 14;
        unsigned int axi3_bandwidth_high_r : 13;
        unsigned int reserved : 4;
    } reg;
} DPU_DBCU_AXI3_READ_FLUX_HIGH_CFG_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int axi3_bwc_en_low_r : 1;
        unsigned int axi3_saturation_low_r : 14;
        unsigned int axi3_bandwidth_low_r : 13;
        unsigned int reserved : 4;
    } reg;
} DPU_DBCU_AXI3_READ_FLUX_LOW_CFG_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int dbcu_clk_sel_0 : 32;
    } reg;
} DPU_DBCU_CLK_SEL_0_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int dbcu_clk_sel_1 : 32;
    } reg;
} DPU_DBCU_CLK_SEL_1_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int dbcu_clk_en_0 : 32;
    } reg;
} DPU_DBCU_CLK_EN_0_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int dbcu_clk_en_1 : 32;
    } reg;
} DPU_DBCU_CLK_EN_1_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int monitor_ini : 32;
    } reg;
} DPU_DBCU_MONITOR_TIMER_INI_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int reserved : 4;
        unsigned int debug_base : 28;
    } reg;
} DPU_DBCU_DEBUG_BUF_BASE_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int flux_mode : 1;
        unsigned int addr_protect_en : 1;
        unsigned int reserved : 14;
        unsigned int debug_depth : 16;
    } reg;
} DPU_DBCU_DEBUG_CTRL_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int aif_rd_shadow : 1;
        unsigned int reserved : 31;
    } reg;
} DPU_DBCU_AIF_SHADOW_READ_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int monitor_en : 1;
        unsigned int reserved : 31;
    } reg;
} DPU_DBCU_MONITOR_EN_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int mon_axi_sel_w : 2;
        unsigned int mon_axi_sel_r : 2;
        unsigned int sample_mode : 2;
        unsigned int reserved_0 : 2;
        unsigned int sample_cmd_type : 2;
        unsigned int sample_mid : 9;
        unsigned int mon_sel : 3;
        unsigned int reserved_1 : 10;
    } reg;
} DPU_DBCU_MONITOR_CTRL_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int window_end_num : 16;
        unsigned int reserved : 16;
    } reg;
} DPU_DBCU_MONITOR_SAMPLE_MUN_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int window_end_time : 32;
    } reg;
} DPU_DBCU_MONITOR_SAMPLE_TIME_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int window_end_flow : 32;
    } reg;
} DPU_DBCU_MONITOR_SAMPLE_FLOW_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int axi0_domain_r : 2;
        unsigned int axi0_domain_w : 2;
        unsigned int axi1_domain_r : 2;
        unsigned int axi2_domain_r : 2;
        unsigned int axi2_domain_w : 2;
        unsigned int axi3_domain_r : 2;
        unsigned int reserved : 20;
    } reg;
} DPU_DBCU_AXI_DOMAIN_CFG_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int real_cycle_w : 32;
    } reg;
} DPU_DBCU_MONITOR_WR_ST_0_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int window_cycle_w : 32;
    } reg;
} DPU_DBCU_MONITOR_WR_ST_1_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int writedata : 32;
    } reg;
} DPU_DBCU_MONITOR_WR_ST_2_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int wr_cmd_cnt : 32;
    } reg;
} DPU_DBCU_MONITOR_WR_ST_3_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int wr_done_cnt : 32;
    } reg;
} DPU_DBCU_MONITOR_WR_ST_4_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int wburst_cnt : 16;
        unsigned int reserved : 16;
    } reg;
} DPU_DBCU_MONITOR_WR_ST_5_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int avr_wrcycle_cnt : 32;
    } reg;
} DPU_DBCU_MONITOR_WR_ST_6_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int min_wrcycle_cnt : 16;
        unsigned int reserved : 16;
    } reg;
} DPU_DBCU_MONITOR_WR_ST_7_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int max_wrcycle_cnt : 16;
        unsigned int reserved : 16;
    } reg;
} DPU_DBCU_MONITOR_WR_ST_8_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int real_cycle_r : 32;
    } reg;
} DPU_DBCU_MONITOR_RD_ST_0_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int window_cycle_r : 32;
    } reg;
} DPU_DBCU_MONITOR_RD_ST_1_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int readdata : 32;
    } reg;
} DPU_DBCU_MONITOR_RD_ST_2_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int rd_cmd_cnt : 32;
    } reg;
} DPU_DBCU_MONITOR_RD_ST_3_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int rd_done_cnt : 32;
    } reg;
} DPU_DBCU_MONITOR_RD_ST_4_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int rburst_cnt : 16;
        unsigned int reserved : 16;
    } reg;
} DPU_DBCU_MONITOR_RD_ST_5_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int avr_rdcycle_cnt : 32;
    } reg;
} DPU_DBCU_MONITOR_RD_ST_6_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int min_rdcycle_cnt : 16;
        unsigned int reserved : 16;
    } reg;
} DPU_DBCU_MONITOR_RD_ST_7_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int max_rdcycle_cnt : 16;
        unsigned int reserved : 16;
    } reg;
} DPU_DBCU_MONITOR_RD_ST_8_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int channel_os_zero_r : 32;
    } reg;
} DPU_DBCU_MONITOR_OS_R0_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int channel_os_r0 : 32;
    } reg;
} DPU_DBCU_MONITOR_OS_R1_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int channel_os_r1 : 32;
    } reg;
} DPU_DBCU_MONITOR_OS_R2_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int reserved : 32;
    } reg;
} DPU_DBCU_MONITOR_OS_R3_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int axi_os_r : 32;
    } reg;
} DPU_DBCU_MONITOR_OS_R4_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int channel_os_zero_w : 32;
    } reg;
} DPU_DBCU_MONITOR_OS_W0_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int channel_os_w0 : 32;
    } reg;
} DPU_DBCU_MONITOR_OS_W1_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int reserved : 32;
    } reg;
} DPU_DBCU_MONITOR_OS_W2_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int axi_os_w : 32;
    } reg;
} DPU_DBCU_MONITOR_OS_W3_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int aif_stat_0 : 32;
    } reg;
} DPU_DBCU_AIF_STAT_0_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int aif_stat_1 : 32;
    } reg;
} DPU_DBCU_AIF_STAT_1_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int aif_stat_2 : 32;
    } reg;
} DPU_DBCU_AIF_STAT_2_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int aif_stat_3 : 32;
    } reg;
} DPU_DBCU_AIF_STAT_3_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int aif_stat_4 : 32;
    } reg;
} DPU_DBCU_AIF_STAT_4_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int aif_stat_5 : 32;
    } reg;
} DPU_DBCU_AIF_STAT_5_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int aif_stat_6 : 32;
    } reg;
} DPU_DBCU_AIF_STAT_6_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int aif_stat_7 : 32;
    } reg;
} DPU_DBCU_AIF_STAT_7_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int aif_stat_8 : 32;
    } reg;
} DPU_DBCU_AIF_STAT_8_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int aif_stat_9 : 32;
    } reg;
} DPU_DBCU_AIF_STAT_9_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int aif_stat_10 : 32;
    } reg;
} DPU_DBCU_AIF_STAT_10_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int aif_stat_11 : 32;
    } reg;
} DPU_DBCU_AIF_STAT_11_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int aif_stat_12 : 32;
    } reg;
} DPU_DBCU_AIF_STAT_12_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int aif_stat_13 : 32;
    } reg;
} DPU_DBCU_AIF_STAT_13_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int aif_stat_14 : 32;
    } reg;
} DPU_DBCU_AIF_STAT_14_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int aif_stat_15 : 32;
    } reg;
} DPU_DBCU_AIF_STAT_15_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int mif_rd_shadow : 1;
        unsigned int reserved : 31;
    } reg;
} DPU_DBCU_MIF_SHADOW_READ_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int aif_cmd_reload : 1;
        unsigned int reserved : 31;
    } reg;
} DPU_DBCU_AIF_CMD_RELOAD_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int pref_en_r0 : 1;
        unsigned int reserved_0 : 1;
        unsigned int pref_pg_en_r0 : 2;
        unsigned int reserved_1 : 28;
    } reg;
} DPU_DBCU_MIF_CTRL_SMARTDMA_0_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int sub0_lead_num_r0 : 7;
        unsigned int sub1_lead_num_r0 : 7;
        unsigned int reserved : 18;
    } reg;
} DPU_DBCU_MIF_LEAD_SMARTDMA_0_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int sub0_os_num_r0 : 8;
        unsigned int sub1_os_num_r0 : 8;
        unsigned int reserved : 16;
    } reg;
} DPU_DBCU_MIF_OS_SMARTDMA_0_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int sub0_pref_cnt_r0 : 8;
        unsigned int sub1_pref_cnt_r0 : 8;
        unsigned int reserved : 16;
    } reg;
} DPU_DBCU_MIF_STAT_SMARTDMA_0_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int pref_en_r1 : 1;
        unsigned int reserved_0 : 1;
        unsigned int pref_pg_en_r1 : 2;
        unsigned int reserved_1 : 28;
    } reg;
} DPU_DBCU_MIF_CTRL_SMARTDMA_1_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int sub0_lead_num_r1 : 7;
        unsigned int sub1_lead_num_r1 : 7;
        unsigned int reserved : 18;
    } reg;
} DPU_DBCU_MIF_LEAD_SMARTDMA_1_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int sub0_os_num_r1 : 8;
        unsigned int sub1_os_num_r1 : 8;
        unsigned int reserved : 16;
    } reg;
} DPU_DBCU_MIF_OS_SMARTDMA_1_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int sub0_pref_cnt_r1 : 8;
        unsigned int sub1_pref_cnt_r1 : 8;
        unsigned int reserved : 16;
    } reg;
} DPU_DBCU_MIF_STAT_SMARTDMA_1_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int pref_en_r2 : 1;
        unsigned int reserved_0 : 1;
        unsigned int pref_pg_en_r2 : 2;
        unsigned int reserved_1 : 28;
    } reg;
} DPU_DBCU_MIF_CTRL_SMARTDMA_2_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int sub0_lead_num_r2 : 7;
        unsigned int sub1_lead_num_r2 : 7;
        unsigned int reserved : 18;
    } reg;
} DPU_DBCU_MIF_LEAD_SMARTDMA_2_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int sub0_os_num_r2 : 8;
        unsigned int sub1_os_num_r2 : 8;
        unsigned int reserved : 16;
    } reg;
} DPU_DBCU_MIF_OS_SMARTDMA_2_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int sub0_pref_cnt_r2 : 8;
        unsigned int sub1_pref_cnt_r2 : 8;
        unsigned int reserved : 16;
    } reg;
} DPU_DBCU_MIF_STAT_SMARTDMA_2_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int pref_en_r3 : 1;
        unsigned int reserved_0 : 1;
        unsigned int pref_pg_en_r3 : 2;
        unsigned int reserved_1 : 28;
    } reg;
} DPU_DBCU_MIF_CTRL_SMARTDMA_3_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int sub0_lead_num_r3 : 7;
        unsigned int sub1_lead_num_r3 : 7;
        unsigned int reserved : 18;
    } reg;
} DPU_DBCU_MIF_LEAD_SMARTDMA_3_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int sub0_os_num_r3 : 8;
        unsigned int sub1_os_num_r3 : 8;
        unsigned int reserved : 16;
    } reg;
} DPU_DBCU_MIF_OS_SMARTDMA_3_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int sub0_pref_cnt_r3 : 8;
        unsigned int sub1_pref_cnt_r3 : 8;
        unsigned int reserved : 16;
    } reg;
} DPU_DBCU_MIF_STAT_SMARTDMA_3_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int pref_en_w0 : 1;
        unsigned int pref_pg_en_w0 : 4;
        unsigned int reserved : 27;
    } reg;
} DPU_DBCU_MIF_CTRL_WCH0_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int sub0_lead_num_w0 : 7;
        unsigned int sub1_lead_num_w0 : 7;
        unsigned int sub2_lead_num_w0 : 7;
        unsigned int sub3_lead_num_w0 : 7;
        unsigned int reserved : 4;
    } reg;
} DPU_DBCU_MIF_LEAD_CTRL_WCH0_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int sub0_os_num_w0 : 8;
        unsigned int sub1_os_num_w0 : 8;
        unsigned int sub2_os_num_w0 : 8;
        unsigned int sub3_os_num_w0 : 8;
    } reg;
} DPU_DBCU_MIF_OS_CTRL_WCH0_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int sub0_pref_cnt_w0 : 8;
        unsigned int sub1_pref_cnt_w0 : 8;
        unsigned int sub2_pref_cnt_w0 : 8;
        unsigned int sub3_pref_cnt_w0 : 8;
    } reg;
} DPU_DBCU_MIF_STAT_WCH0_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int pref_en_w1 : 1;
        unsigned int pref_pg_en_w1 : 4;
        unsigned int reserved : 27;
    } reg;
} DPU_DBCU_MIF_CTRL_WCH1_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int sub0_lead_num_w1 : 7;
        unsigned int sub1_lead_num_w1 : 7;
        unsigned int sub2_lead_num_w1 : 7;
        unsigned int sub3_lead_num_w1 : 7;
        unsigned int reserved : 4;
    } reg;
} DPU_DBCU_MIF_LEAD_CTRL_WCH1_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int sub0_os_num_w1 : 8;
        unsigned int sub1_os_num_w1 : 8;
        unsigned int sub2_os_num_w1 : 8;
        unsigned int sub3_os_num_w1 : 8;
    } reg;
} DPU_DBCU_MIF_OS_CTRL_WCH1_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int sub0_pref_cnt_w1 : 8;
        unsigned int sub1_pref_cnt_w1 : 8;
        unsigned int sub2_pref_cnt_w1 : 8;
        unsigned int sub3_pref_cnt_w1 : 8;
    } reg;
} DPU_DBCU_MIF_STAT_WCH1_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int pref_en_w2 : 1;
        unsigned int pref_pg_en_w2 : 4;
        unsigned int reserved : 27;
    } reg;
} DPU_DBCU_MIF_CTRL_WCH2_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int sub0_lead_num_w2 : 7;
        unsigned int sub1_lead_num_w2 : 7;
        unsigned int sub2_lead_num_w2 : 7;
        unsigned int sub3_lead_num_w2 : 7;
        unsigned int reserved : 4;
    } reg;
} DPU_DBCU_MIF_LEAD_CTRL_WCH2_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int sub0_os_num_w2 : 8;
        unsigned int sub1_os_num_w2 : 8;
        unsigned int sub2_os_num_w2 : 8;
        unsigned int sub3_os_num_w2 : 8;
    } reg;
} DPU_DBCU_MIF_OS_CTRL_WCH2_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int sub0_pref_cnt_w2 : 8;
        unsigned int sub1_pref_cnt_w2 : 8;
        unsigned int sub2_pref_cnt_w2 : 8;
        unsigned int sub3_pref_cnt_w2 : 8;
    } reg;
} DPU_DBCU_MIF_STAT_WCH2_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int lbuf_ctl_clk_sel : 32;
    } reg;
} DPU_LBUF_CTL_CLK_SEL_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int lbuf_ctl_clk_en : 32;
    } reg;
} DPU_LBUF_CTL_CLK_EN_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int lbuf_part_clk_sel : 32;
    } reg;
} DPU_LBUF_PART_CLK_SEL_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int lbuf_part_clk_en : 32;
    } reg;
} DPU_LBUF_PART_CLK_EN_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int lbuf_mem_ctrl : 4;
        unsigned int reserved : 28;
    } reg;
} DPU_LBUF_MEM_CTRL_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int lbuf_mem_dslp_en : 1;
        unsigned int reserved : 31;
    } reg;
} DPU_LBUF_MEM_DSLP_CTRL_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int sdma_auto_ctrl_en : 1;
        unsigned int ov_auto_ctrl_en : 1;
        unsigned int reserved : 30;
    } reg;
} DPU_LBUF_SOUR_AUTO_CTRL_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int part0_dbg_en : 1;
        unsigned int part1_dbg_en : 1;
        unsigned int part2_dbg_en : 1;
        unsigned int mst_check_en : 1;
        unsigned int scn_check_sel : 4;
        unsigned int run_check_sel : 8;
        unsigned int lb_rd_shadow : 1;
        unsigned int reserved : 15;
    } reg;
} DPU_LBUF_DEBUG_CTRL_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int dbuf0_rqos_mask : 1;
        unsigned int dbuf0_dfs_ok_mask : 1;
        unsigned int dbuf0_flux_req_mask : 1;
        unsigned int dbuf0_dfs_ok_sel : 1;
        unsigned int dbuf0_rqos_dat : 2;
        unsigned int dbuf0_flux_req_sw_en : 1;
        unsigned int reserved : 25;
    } reg;
} DPU_LBUF_DBUF0_CTRL_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int dbuf0_online_empty : 1;
        unsigned int dbuf0_online_full : 1;
        unsigned int dbuf0_rqos : 2;
        unsigned int dbuf0_dfs_ok : 1;
        unsigned int dbuf0_flux_req : 1;
        unsigned int reserved_0 : 10;
        unsigned int dbuf0_lev_cnt : 15;
        unsigned int reserved_1 : 1;
    } reg;
} DPU_LBUF_DBUF0_STATE_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int dbuf0_thd_rqos_in : 15;
        unsigned int reserved_0 : 1;
        unsigned int dbuf0_thd_rqos_out : 15;
        unsigned int reserved_1 : 1;
    } reg;
} DPU_LBUF_DBUF0_THD_RQOS_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int dbuf0_thd_dfs_ok : 15;
        unsigned int reserved : 17;
    } reg;
} DPU_LBUF_DBUF0_THD_DFS_OK_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int dbuf0_thd_flux_req_bef_in : 15;
        unsigned int reserved_0 : 1;
        unsigned int dbuf0_thd_flux_req_bef_out : 15;
        unsigned int reserved_1 : 1;
    } reg;
} DPU_LBUF_DBUF0_THD_FLUX_REQ_BEF_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int dbuf0_thd_flux_req_aft_in : 15;
        unsigned int reserved_0 : 1;
        unsigned int dbuf0_thd_flux_req_aft_out : 15;
        unsigned int reserved_1 : 1;
    } reg;
} DPU_LBUF_DBUF0_THD_FLUX_REQ_AFT_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int dbuf0_online_fill_lev : 15;
        unsigned int reserved : 17;
    } reg;
} DPU_LBUF_DBUF0_ONLINE_FILL_LEVEL_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int scn0_flux_step_slow : 5;
        unsigned int reserved_0 : 11;
        unsigned int scn0_flux_thd : 15;
        unsigned int reserved_1 : 1;
    } reg;
} DPU_LBUF_DBUF0_THD_FILL_LEV0_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int lb_prs_scn0_low_thd : 15;
        unsigned int lb_prs_scn0_en : 1;
        unsigned int lb_prs_scn0_high_thd : 15;
        unsigned int reserved : 1;
    } reg;
} DPU_LBUF_DBUF0_PRS_SCN0_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int dbuf1_rqos_mask : 1;
        unsigned int dbuf1_dfs_ok_mask : 1;
        unsigned int dbuf1_flux_req_mask : 1;
        unsigned int dbuf1_dfs_ok_sel : 1;
        unsigned int dbuf1_rqos_dat : 2;
        unsigned int dbuf1_flux_req_sw_en : 1;
        unsigned int reserved : 25;
    } reg;
} DPU_LBUF_DBUF1_CTRL_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int dbuf1_online_empty : 1;
        unsigned int dbuf1_online_full : 1;
        unsigned int dbuf1_rqos : 2;
        unsigned int dbuf1_dfs_ok : 1;
        unsigned int dbuf1_flux_req : 1;
        unsigned int reserved_0 : 10;
        unsigned int dbuf1_lev_cnt : 15;
        unsigned int reserved_1 : 1;
    } reg;
} DPU_LBUF_DBUF1_STATE_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int dbuf1_thd_rqos_in : 15;
        unsigned int reserved_0 : 1;
        unsigned int dbuf1_thd_rqos_out : 15;
        unsigned int reserved_1 : 1;
    } reg;
} DPU_LBUF_DBUF1_THD_RQOS_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int dbuf1_thd_dfs_ok : 15;
        unsigned int reserved : 17;
    } reg;
} DPU_LBUF_DBUF1_THD_DFS_OK_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int dbuf1_thd_flux_req_bef_in : 15;
        unsigned int reserved_0 : 1;
        unsigned int dbuf1_thd_flux_req_bef_out : 15;
        unsigned int reserved_1 : 1;
    } reg;
} DPU_LBUF_DBUF1_THD_FLUX_REQ_BEF_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int dbuf1_thd_flux_req_aft_in : 15;
        unsigned int reserved_0 : 1;
        unsigned int dbuf1_thd_flux_req_aft_out : 15;
        unsigned int reserved_1 : 1;
    } reg;
} DPU_LBUF_DBUF1_THD_FLUX_REQ_AFT_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int dbuf1_online_fill_lev : 15;
        unsigned int reserved : 17;
    } reg;
} DPU_LBUF_DBUF1_ONLINE_FILL_LEVEL_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int scn1_flux_step_slow : 5;
        unsigned int reserved_0 : 11;
        unsigned int scn1_flux_thd : 15;
        unsigned int reserved_1 : 1;
    } reg;
} DPU_LBUF_DBUF1_THD_FILL_LEV0_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int lb_prs_scn1_low_thd : 15;
        unsigned int lb_prs_scn1_en : 1;
        unsigned int lb_prs_scn1_high_thd : 15;
        unsigned int reserved : 1;
    } reg;
} DPU_LBUF_DBUF1_PRS_SCN1_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int dbuf2_rqos_mask : 1;
        unsigned int reserved_0 : 3;
        unsigned int dbuf2_rqos_dat : 2;
        unsigned int reserved_1 : 26;
    } reg;
} DPU_LBUF_DBUF2_CTRL_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int dbuf2_online_empty : 1;
        unsigned int dbuf2_online_full : 1;
        unsigned int dbuf2_rqos : 2;
        unsigned int reserved_0 : 12;
        unsigned int dbuf2_lev_cnt : 15;
        unsigned int reserved_1 : 1;
    } reg;
} DPU_LBUF_DBUF2_STATE_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int dbuf2_thd_rqos_in : 15;
        unsigned int reserved_0 : 1;
        unsigned int dbuf2_thd_rqos_out : 15;
        unsigned int reserved_1 : 1;
    } reg;
} DPU_LBUF_DBUF2_THD_RQOS_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int dbuf2_online_fill_lev : 15;
        unsigned int reserved : 17;
    } reg;
} DPU_LBUF_DBUF2_ONLINE_FILL_LEVEL_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int scn2_flux_step_slow : 5;
        unsigned int reserved_0 : 11;
        unsigned int scn2_flux_thd : 15;
        unsigned int reserved_1 : 1;
    } reg;
} DPU_LBUF_DBUF2_THD_FILL_LEV0_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int lb_prs_scn2_low_thd : 15;
        unsigned int lb_prs_scn2_en : 1;
        unsigned int lb_prs_scn2_high_thd : 15;
        unsigned int reserved : 1;
    } reg;
} DPU_LBUF_DBUF2_PRS_SCN2_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int dbuf3_rqos_mask : 1;
        unsigned int reserved_0 : 3;
        unsigned int dbuf3_rqos_dat : 2;
        unsigned int reserved_1 : 26;
    } reg;
} DPU_LBUF_DBUF3_CTRL_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int dbuf3_online_empty : 1;
        unsigned int dbuf3_online_full : 1;
        unsigned int dbuf3_rqos : 2;
        unsigned int reserved_0 : 12;
        unsigned int dbuf3_lev_cnt : 15;
        unsigned int reserved_1 : 1;
    } reg;
} DPU_LBUF_DBUF3_STATE_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int dbuf3_thd_rqos_in : 15;
        unsigned int reserved_0 : 1;
        unsigned int dbuf3_thd_rqos_out : 15;
        unsigned int reserved_1 : 1;
    } reg;
} DPU_LBUF_DBUF3_THD_RQOS_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int dbuf3_online_fill_lev : 15;
        unsigned int reserved : 17;
    } reg;
} DPU_LBUF_DBUF3_ONLINE_FILL_LEVEL_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int scn3_flux_step_slow : 5;
        unsigned int reserved_0 : 11;
        unsigned int scn3_flux_thd : 15;
        unsigned int reserved_1 : 1;
    } reg;
} DPU_LBUF_DBUF3_THD_FILL_LEV0_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int lb_prs_scn3_low_thd : 15;
        unsigned int lb_prs_scn3_en : 1;
        unsigned int lb_prs_scn3_high_thd : 15;
        unsigned int reserved : 1;
    } reg;
} DPU_LBUF_DBUF3_PRS_SCN3_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int scn0_flux_ctl_en : 1;
        unsigned int scn0_flux_th_sel : 1;
        unsigned int scn0_out_mode_sel : 2;
        unsigned int scn0_flux_size : 9;
        unsigned int scn0_rsv_cfg : 3;
        unsigned int scn0_flux_step_fast : 5;
        unsigned int reserved : 3;
        unsigned int scn0_lb_num : 8;
    } reg;
} DPU_LBUF_SCN0_FLUX_CTRL_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int scn0_flux_lth : 14;
        unsigned int reserved_0 : 2;
        unsigned int scn0_flux_uth : 14;
        unsigned int reserved_1 : 2;
    } reg;
} DPU_LBUF_SCN0_FLUX_TH_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int scn1_flux_ctl_en : 1;
        unsigned int scn1_flux_th_sel : 1;
        unsigned int scn1_out_mode_sel : 2;
        unsigned int scn1_flux_size : 9;
        unsigned int scn1_rsv_cfg : 3;
        unsigned int scn1_flux_step_fast : 5;
        unsigned int reserved : 3;
        unsigned int scn1_lb_num : 8;
    } reg;
} DPU_LBUF_SCN1_FLUX_CTRL_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int scn1_flux_lth : 14;
        unsigned int reserved_0 : 2;
        unsigned int scn1_flux_uth : 14;
        unsigned int reserved_1 : 2;
    } reg;
} DPU_LBUF_SCN1_FLUX_TH_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int scn2_flux_ctl_en : 1;
        unsigned int scn2_flux_th_sel : 1;
        unsigned int scn2_out_mode_sel : 2;
        unsigned int scn2_flux_size : 9;
        unsigned int scn2_rsv_cfg : 3;
        unsigned int scn2_flux_step_fast : 5;
        unsigned int reserved : 3;
        unsigned int scn2_lb_num : 8;
    } reg;
} DPU_LBUF_SCN2_FLUX_CTRL_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int scn2_flux_lth : 14;
        unsigned int reserved_0 : 2;
        unsigned int scn2_flux_uth : 14;
        unsigned int reserved_1 : 2;
    } reg;
} DPU_LBUF_SCN2_FLUX_TH_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int scn3_flux_ctl_en : 1;
        unsigned int scn3_flux_th_sel : 1;
        unsigned int scn3_out_mode_sel : 2;
        unsigned int scn3_flux_size : 9;
        unsigned int scn3_rsv_cfg : 3;
        unsigned int scn3_flux_step_fast : 5;
        unsigned int reserved : 3;
        unsigned int scn3_lb_num : 8;
    } reg;
} DPU_LBUF_SCN3_FLUX_CTRL_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int scn3_flux_lth : 14;
        unsigned int reserved_0 : 2;
        unsigned int scn3_flux_uth : 14;
        unsigned int reserved_1 : 2;
    } reg;
} DPU_LBUF_SCN3_FLUX_TH_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int scn4_flux_ctl_en : 1;
        unsigned int scn4_flux_th_sel : 1;
        unsigned int scn4_out_mode_sel : 2;
        unsigned int scn4_flux_size : 9;
        unsigned int scn4_rsv_cfg : 3;
        unsigned int scn4_flux_step_fast : 5;
        unsigned int reserved : 3;
        unsigned int scn4_lb_num : 8;
    } reg;
} DPU_LBUF_SCN4_FLUX_CTRL_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int scn4_flux_lth : 14;
        unsigned int reserved_0 : 2;
        unsigned int scn4_flux_uth : 14;
        unsigned int reserved_1 : 2;
    } reg;
} DPU_LBUF_SCN4_FLUX_TH_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int scn5_flux_ctl_en : 1;
        unsigned int scn5_flux_th_sel : 1;
        unsigned int scn5_out_mode_sel : 2;
        unsigned int scn5_flux_size : 9;
        unsigned int scn5_rsv_cfg : 3;
        unsigned int scn5_flux_step_fast : 5;
        unsigned int reserved : 3;
        unsigned int scn5_lb_num : 8;
    } reg;
} DPU_LBUF_SCN5_FLUX_CTRL_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int scn5_flux_lth : 14;
        unsigned int reserved_0 : 2;
        unsigned int scn5_flux_uth : 14;
        unsigned int reserved_1 : 2;
    } reg;
} DPU_LBUF_SCN5_FLUX_TH_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int scn6_flux_ctl_en : 1;
        unsigned int scn6_flux_th_sel : 1;
        unsigned int scn6_out_mode_sel : 2;
        unsigned int scn6_flux_size : 9;
        unsigned int scn6_rsv_cfg : 3;
        unsigned int scn6_flux_step_fast : 5;
        unsigned int reserved : 3;
        unsigned int scn6_lb_num : 8;
    } reg;
} DPU_LBUF_SCN6_FLUX_CTRL_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int scn6_flux_lth : 14;
        unsigned int reserved_0 : 2;
        unsigned int scn6_flux_uth : 14;
        unsigned int reserved_1 : 2;
    } reg;
} DPU_LBUF_SCN6_FLUX_TH_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int lbuf_dcpt0_state : 4;
        unsigned int lbuf_dcpt1_state : 4;
        unsigned int lbuf_dcpt2_state : 4;
        unsigned int lbuf_dcpt3_state : 4;
        unsigned int lbuf_dcpt4_state : 4;
        unsigned int lbuf_dcpt5_state : 4;
        unsigned int lbuf_dcpt6_state : 4;
        unsigned int reserved : 4;
    } reg;
} DPU_LBUF_DCPT_STATE_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int lbuf_dcpt0_dma_state : 4;
        unsigned int lbuf_dcpt1_dma_state : 4;
        unsigned int lbuf_dcpt2_dma_state : 4;
        unsigned int lbuf_dcpt3_dma_state : 4;
        unsigned int lbuf_dcpt4_dma_state : 4;
        unsigned int lbuf_dcpt5_dma_state : 4;
        unsigned int lbuf_dcpt6_dma_state : 4;
        unsigned int reserved : 4;
    } reg;
} DPU_LBUF_DCPT_DMA_STATE_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int lbuf_idx_state : 4;
        unsigned int lbuf_run_state : 4;
        unsigned int reserved : 24;
    } reg;
} DPU_LBUF_CTL_STATE_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int lbuf_rf_use_state : 8;
        unsigned int reserved : 24;
    } reg;
} DPU_LBUF_RF_STATE_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int srot0_use_state : 1;
        unsigned int srot1_use_state : 1;
        unsigned int hemc0_use_state : 1;
        unsigned int hemc1_use_state : 1;
        unsigned int vscf0_use_state : 1;
        unsigned int vscf1_use_state : 1;
        unsigned int arsr0_use_state : 1;
        unsigned int arsr1_use_state : 1;
        unsigned int hdr_use_state : 1;
        unsigned int cld0_use_state : 1;
        unsigned int cld1_use_state : 1;
        unsigned int uvup_use_state : 1;
        unsigned int wch0_use_state : 1;
        unsigned int wch1_use_state : 1;
        unsigned int wch2_use_state : 1;
        unsigned int preload_use_state : 1;
        unsigned int reserved : 16;
    } reg;
} DPU_LBUF_PRE_USE_STATE_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int dpp0_use_state : 1;
        unsigned int dpp1_use_state : 1;
        unsigned int debn0_use_state : 1;
        unsigned int debn1_use_state : 1;
        unsigned int dsc0_use_state : 1;
        unsigned int dsc1_use_state : 1;
        unsigned int ddic0_use_state : 1;
        unsigned int ddic1_use_state : 1;
        unsigned int idle0_use_state : 1;
        unsigned int idle1_use_state : 1;
        unsigned int dema0_use_state : 1;
        unsigned int dema1_use_state : 1;
        unsigned int dema2_use_state : 1;
        unsigned int dema3_use_state : 1;
        unsigned int itfsw0_use_state : 1;
        unsigned int itfsw1_use_state : 1;
        unsigned int itfsw2_use_state : 1;
        unsigned int itfsw3_use_state : 1;
        unsigned int reserved : 14;
    } reg;
} DPU_LBUF_PPST_USE_STATE_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int srot0_check_state : 1;
        unsigned int srot1_check_state : 1;
        unsigned int hemc0_check_state : 1;
        unsigned int hemc1_check_state : 1;
        unsigned int vscf0_check_state : 1;
        unsigned int vscf1_check_state : 1;
        unsigned int arsr0_check_state : 1;
        unsigned int arsr1_check_state : 1;
        unsigned int hdr_check_state : 1;
        unsigned int cld0_check_state : 1;
        unsigned int cld1_check_state : 1;
        unsigned int uvup_check_state : 1;
        unsigned int wch0_check_state : 1;
        unsigned int wch1_check_state : 1;
        unsigned int wch2_check_state : 1;
        unsigned int preload_check_state : 1;
        unsigned int reserved : 16;
    } reg;
} DPU_LBUF_PRE_CHECK_STATE_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int dpp0_check_state : 1;
        unsigned int dpp1_check_state : 1;
        unsigned int debn0_check_state : 1;
        unsigned int debn1_check_state : 1;
        unsigned int dsc0_check_state : 1;
        unsigned int dsc1_check_state : 1;
        unsigned int ddic0_check_state : 1;
        unsigned int ddic1_check_state : 1;
        unsigned int idle0_check_state : 1;
        unsigned int idle1_check_state : 1;
        unsigned int dema0_check_state : 1;
        unsigned int dema1_check_state : 1;
        unsigned int dema2_check_state : 1;
        unsigned int dema3_check_state : 1;
        unsigned int itfsw0_check_state : 1;
        unsigned int itfsw1_check_state : 1;
        unsigned int itfsw2_check_state : 1;
        unsigned int itfsw3_check_state : 1;
        unsigned int reserved : 14;
    } reg;
} DPU_LBUF_PPST_CHECK_STATE_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int lbuf_scn_cur_mst : 6;
        unsigned int lbuf_scn_dst_mst : 6;
        unsigned int lbuf_scn_rf_fmt : 4;
        unsigned int lbuf_scn_idx_num : 6;
        unsigned int lbuf_scn_pln_num : 2;
        unsigned int lbuf_scn_run_idx : 7;
        unsigned int lbuf_scn_rw_flag : 1;
    } reg;
} DPU_LBUF_SCN_STATE_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int lbuf_scn_dma_cur_mst : 6;
        unsigned int lbuf_scn_dma_dst_mst : 6;
        unsigned int lbuf_scn_dma_rf_fmt : 4;
        unsigned int lbuf_scn_dma_idx_num : 6;
        unsigned int lbuf_scn_dma_pln_num : 2;
        unsigned int lbuf_scn_dma_run_idx : 7;
        unsigned int lbuf_scn_dma_rw_flag : 1;
    } reg;
} DPU_LBUF_SCN_DMA_STATE_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int lbuf_run_vir_idx : 14;
        unsigned int reserved_0 : 2;
        unsigned int lbuf_run_idx_num : 6;
        unsigned int reserved_1 : 2;
        unsigned int lbuf_run_rf_fmt : 4;
        unsigned int reserved_2 : 4;
    } reg;
} DPU_LBUF_RUN_STATE_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int lbuf_run_wr_mst : 6;
        unsigned int reserved_0 : 2;
        unsigned int lbuf_run_rd_mst : 6;
        unsigned int reserved_1 : 2;
        unsigned int lbuf_run_wln_num : 6;
        unsigned int reserved_2 : 2;
        unsigned int lbuf_run_rln_num : 6;
        unsigned int reserved_3 : 2;
    } reg;
} DPU_LBUF_RUN_MST_STATE_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int lbuf_run_vld : 1;
        unsigned int lbuf_run_wr_more : 1;
        unsigned int lbuf_run_w_image : 1;
        unsigned int lbuf_run_rep_rd : 1;
        unsigned int lbuf_run_wr_allow : 1;
        unsigned int lbuf_run_rd_allow : 1;
        unsigned int lbuf_run_rd2_allow : 1;
        unsigned int reserved_0 : 1;
        unsigned int lbuf_run_all_h : 14;
        unsigned int reserved_1 : 10;
    } reg;
} DPU_LBUF_RUN_CTL_STATE_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int lbuf_run_done_w_cnt : 14;
        unsigned int reserved_0 : 2;
        unsigned int lbuf_run_mst_w_cnt : 14;
        unsigned int reserved_1 : 2;
    } reg;
} DPU_LBUF_RUN_WR_STATE_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int lbuf_run_done_r_cnt : 14;
        unsigned int reserved_0 : 2;
        unsigned int lbuf_run_mst_r_cnt : 14;
        unsigned int reserved_1 : 2;
    } reg;
} DPU_LBUF_RUN_RD_STATE_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int lbuf_run_done_r2_cnt : 14;
        unsigned int reserved_0 : 2;
        unsigned int lbuf_run_mst_r2_cnt : 14;
        unsigned int reserved_1 : 2;
    } reg;
} DPU_LBUF_RUN_RD2_STATE_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int part0_rmst_pass : 24;
        unsigned int part0_rmst_rsv : 8;
    } reg;
} DPU_LBUF_PART0_RMST_PASS_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int part0_wmst_pass : 20;
        unsigned int part0_wmst_rsv : 12;
    } reg;
} DPU_LBUF_PART0_WMST_PASS_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int part0_idx_cmd : 23;
        unsigned int part0_idx_rsv : 8;
        unsigned int part0_idx_match : 1;
    } reg;
} DPU_LBUF_PART0_IDX_DATA_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int part0_run_cmd : 28;
        unsigned int part0_run_rsv : 3;
        unsigned int part0_run_match : 1;
    } reg;
} DPU_LBUF_PART0_RUN_DAT_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int part1_rmst_pass : 6;
        unsigned int part1_rmst_rsv : 26;
    } reg;
} DPU_LBUF_PART1_RMST_PASS_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int part1_wmst_pass : 13;
        unsigned int part1_wmst_rsv : 19;
    } reg;
} DPU_LBUF_PART1_WMST_PASS_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int part1_idx_cmd : 23;
        unsigned int part1_idx_rsv : 8;
        unsigned int part1_idx_match : 1;
    } reg;
} DPU_LBUF_PART1_IDX_DATA_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int part1_run_cmd : 28;
        unsigned int part1_run_rsv : 3;
        unsigned int part1_run_match : 1;
    } reg;
} DPU_LBUF_PART1_RUN_DATA_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int part2_rmst_pass : 9;
        unsigned int part2_rmst_rsv : 23;
    } reg;
} DPU_LBUF_PART2_RMST_PASS_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int part2_wmst_pass : 17;
        unsigned int part2_wmst_rsv : 15;
    } reg;
} DPU_LBUF_PART2_WMST_PASS_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int part2_idx_cmd : 23;
        unsigned int part2_idx_rsv : 8;
        unsigned int part2_idx_match : 1;
    } reg;
} DPU_LBUF_PART2_IDX_DATA_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int part2_run_cmd : 28;
        unsigned int part2_run_rsv : 3;
        unsigned int part2_run_match : 1;
    } reg;
} DPU_LBUF_PART2_RUN_DATA_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int lbuf_dbg_ctrl0 : 32;
    } reg;
} DPU_LBUF_DBG_CTRL0_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int lbuf_dbg_state0 : 32;
    } reg;
} DPU_LBUF_DBG_STATE0_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int lbuf_dbg_state1 : 32;
    } reg;
} DPU_LBUF_DBG_STATE1_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int arsr_top_rd_shadow : 1;
        unsigned int reserved : 31;
    } reg;
} DPU_ARSR_RD_SHADOW_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int reg_default : 1;
        unsigned int reserved : 31;
    } reg;
} DPU_ARSR_REG_DEFAULT_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int arsr_top_clk_sel : 32;
    } reg;
} DPU_ARSR_TOP_CLK_SEL_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int arsr_top_clk_en : 32;
    } reg;
} DPU_ARSR_TOP_CLK_EN_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int reg_ctrl_scene_id : 3;
        unsigned int reserved : 29;
    } reg;
} DPU_ARSR_REG_CTRL_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int reg_ctrl_flush_en : 1;
        unsigned int reserved : 31;
    } reg;
} DPU_ARSR_REG_CTRL_FLUSH_EN_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int reg_ctrl_dbg : 32;
    } reg;
} DPU_ARSR_REG_CTRL_DEBUG_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int reg_ctrl_state : 32;
    } reg;
} DPU_ARSR_REG_CTRL_STATE_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int r_lb_debug0 : 32;
    } reg;
} DPU_ARSR_R_LB_DEBUG0_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int r_lb_debug1 : 32;
    } reg;
} DPU_ARSR_R_LB_DEBUG1_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int r_lb_debug2 : 32;
    } reg;
} DPU_ARSR_R_LB_DEBUG2_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int r_lb_debug3 : 32;
    } reg;
} DPU_ARSR_R_LB_DEBUG3_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int w_lb_debug0 : 32;
    } reg;
} DPU_ARSR_W_LB_DEBUG0_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int w_lb_debug1 : 32;
    } reg;
} DPU_ARSR_W_LB_DEBUG1_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int w_lb_debug2 : 32;
    } reg;
} DPU_ARSR_W_LB_DEBUG2_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int w_lb_debug3 : 32;
    } reg;
} DPU_ARSR_W_LB_DEBUG3_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int reg_ctrl_layer_id : 5;
        unsigned int reserved : 27;
    } reg;
} DPU_ARSR_REG_CTRL_LAYER_ID_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int dfc_size_vrt : 13;
        unsigned int reserved_0 : 3;
        unsigned int dfc_size_hrz : 13;
        unsigned int reserved_1 : 3;
    } reg;
} DPU_ARSR_DFC_DISP_SIZE_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int dfc_pix_in_num : 1;
        unsigned int reserved : 31;
    } reg;
} DPU_ARSR_DFC_PIX_IN_NUM_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int dfc_glb_alpha1 : 10;
        unsigned int reserved_0 : 6;
        unsigned int dfc_glb_alpha0 : 10;
        unsigned int reserved_1 : 6;
    } reg;
} DPU_ARSR_DFC_GLB_ALPHA01_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int dfc_ax_swap : 1;
        unsigned int dfc_img_fmt : 5;
        unsigned int dfc_uv_swap : 1;
        unsigned int dfc_rb_swap : 1;
        unsigned int reserved : 24;
    } reg;
} DPU_ARSR_DFC_DISP_FMT_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int dfc_right_clip : 6;
        unsigned int reserved_0 : 10;
        unsigned int dfc_left_clip : 6;
        unsigned int reserved_1 : 10;
    } reg;
} DPU_ARSR_DFC_CLIP_CTL_HRZ_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int dfc_bot_clip : 6;
        unsigned int reserved_0 : 10;
        unsigned int dfc_top_clip : 6;
        unsigned int reserved_1 : 10;
    } reg;
} DPU_ARSR_DFC_CLIP_CTL_VRZ_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int dfc_ctl_clip_enable : 1;
        unsigned int reserved : 31;
    } reg;
} DPU_ARSR_DFC_CTL_CLIP_EN_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int dfc_module_en : 1;
        unsigned int reserved : 31;
    } reg;
} DPU_ARSR_DFC_ICG_MODULE_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int dfc_dither_en : 1;
        unsigned int reserved : 31;
    } reg;
} DPU_ARSR_DFC_DITHER_ENABLE_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int dfc_left_pad : 6;
        unsigned int reserved_0 : 2;
        unsigned int dfc_right_pad : 6;
        unsigned int reserved_1 : 2;
        unsigned int dfc_top_pad : 6;
        unsigned int reserved_2 : 2;
        unsigned int dfc_bot_pad : 6;
        unsigned int reserved_3 : 1;
        unsigned int dfc_ctl_pad_enable : 1;
    } reg;
} DPU_ARSR_DFC_PADDING_CTL_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int dfc_glb_alpha3 : 10;
        unsigned int reserved_0 : 6;
        unsigned int dfc_glb_alpha2 : 10;
        unsigned int reserved_1 : 6;
    } reg;
} DPU_ARSR_DFC_GLB_ALPHA23_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int bitext_en : 1;
        unsigned int bitext_rgb_en : 1;
        unsigned int bitext_alpha_en : 1;
        unsigned int bitext_reg_ini_cfg_en : 1;
        unsigned int reserved : 28;
    } reg;
} DPU_ARSR_BITEXT_CTL_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int bitext_reg_ini0_0 : 32;
    } reg;
} DPU_ARSR_BITEXT_REG_INI0_0_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int bitext_reg_ini0_1 : 32;
    } reg;
} DPU_ARSR_BITEXT_REG_INI0_1_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int bitext_reg_ini0_2 : 32;
    } reg;
} DPU_ARSR_BITEXT_REG_INI0_2_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int bitext_reg_ini0_3 : 31;
        unsigned int reserved : 1;
    } reg;
} DPU_ARSR_BITEXT_REG_INI0_3_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int bitext_reg_ini1_0 : 32;
    } reg;
} DPU_ARSR_BITEXT_REG_INI1_0_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int bitext_reg_ini1_1 : 32;
    } reg;
} DPU_ARSR_BITEXT_REG_INI1_1_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int bitext_reg_ini1_2 : 32;
    } reg;
} DPU_ARSR_BITEXT_REG_INI1_2_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int bitext_reg_ini1_3 : 31;
        unsigned int reserved : 1;
    } reg;
} DPU_ARSR_BITEXT_REG_INI1_3_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int bitext_reg_ini2_0 : 32;
    } reg;
} DPU_ARSR_BITEXT_REG_INI2_0_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int bitext_reg_ini2_1 : 32;
    } reg;
} DPU_ARSR_BITEXT_REG_INI2_1_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int bitext_reg_ini2_2 : 32;
    } reg;
} DPU_ARSR_BITEXT_REG_INI2_2_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int bitext_reg_ini2_3 : 31;
        unsigned int reserved : 1;
    } reg;
} DPU_ARSR_BITEXT_REG_INI2_3_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int bitext_power_ctrl_c : 2;
        unsigned int reserved : 30;
    } reg;
} DPU_ARSR_BITEXT_POWER_CTRL_C_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int bitext_power_ctrl_shift : 2;
        unsigned int reserved : 30;
    } reg;
} DPU_ARSR_BITEXT_POWER_CTRL_SHIFT_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int bitext_filt_00 : 5;
        unsigned int reserved : 27;
    } reg;
} DPU_ARSR_BITEXT_FILT_00_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int bitext_filt_01 : 5;
        unsigned int reserved : 27;
    } reg;
} DPU_ARSR_BITEXT_FILT_01_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int bitext_filt_02 : 5;
        unsigned int reserved : 27;
    } reg;
} DPU_ARSR_BITEXT_FILT_02_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int bitext_filt_10 : 5;
        unsigned int reserved : 27;
    } reg;
} DPU_ARSR_BITEXT_FILT_10_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int bitext_filt_11 : 5;
        unsigned int reserved : 27;
    } reg;
} DPU_ARSR_BITEXT_FILT_11_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int bitext_filt_12 : 5;
        unsigned int reserved : 27;
    } reg;
} DPU_ARSR_BITEXT_FILT_12_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int bitext_filt_20 : 5;
        unsigned int reserved : 27;
    } reg;
} DPU_ARSR_BITEXT_FILT_20_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int bitext_filt_21 : 5;
        unsigned int reserved : 27;
    } reg;
} DPU_ARSR_BITEXT_FILT_21_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int bitext_filt_22 : 5;
        unsigned int reserved : 27;
    } reg;
} DPU_ARSR_BITEXT_FILT_22_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int bitext_thd_r0 : 32;
    } reg;
} DPU_ARSR_BITEXT_THD_R0_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int bitext_thd_r1 : 16;
        unsigned int reserved : 16;
    } reg;
} DPU_ARSR_BITEXT_THD_R1_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int bitext_thd_g0 : 32;
    } reg;
} DPU_ARSR_BITEXT_THD_G0_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int bitext_thd_g1 : 16;
        unsigned int reserved : 16;
    } reg;
} DPU_ARSR_BITEXT_THD_G1_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int bitext_thd_b0 : 32;
    } reg;
} DPU_ARSR_BITEXT_THD_B0_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int bitext_thd_b1 : 16;
        unsigned int reserved : 16;
    } reg;
} DPU_ARSR_BITEXT_THD_B1_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int bitext_dbg0 : 32;
    } reg;
} DPU_ARSR_BITEXT0_DBG0_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int csc_idc0 : 11;
        unsigned int reserved_0: 5;
        unsigned int csc_idc1 : 11;
        unsigned int reserved_1: 5;
    } reg;
} DPU_ARSR_CSC_IDC0_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int csc_idc2 : 11;
        unsigned int reserved : 21;
    } reg;
} DPU_ARSR_CSC_IDC2_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int csc_odc0 : 11;
        unsigned int reserved_0: 5;
        unsigned int csc_odc1 : 11;
        unsigned int reserved_1: 5;
    } reg;
} DPU_ARSR_CSC_ODC0_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int csc_odc2 : 11;
        unsigned int reserved : 21;
    } reg;
} DPU_ARSR_CSC_ODC2_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int csc_p00 : 17;
        unsigned int reserved : 15;
    } reg;
} DPU_ARSR_CSC_P00_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int csc_p01 : 17;
        unsigned int reserved : 15;
    } reg;
} DPU_ARSR_CSC_P01_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int csc_p02 : 17;
        unsigned int reserved : 15;
    } reg;
} DPU_ARSR_CSC_P02_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int csc_p10 : 17;
        unsigned int reserved : 15;
    } reg;
} DPU_ARSR_CSC_P10_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int csc_p11 : 17;
        unsigned int reserved : 15;
    } reg;
} DPU_ARSR_CSC_P11_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int csc_p12 : 17;
        unsigned int reserved : 15;
    } reg;
} DPU_ARSR_CSC_P12_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int csc_p20 : 17;
        unsigned int reserved : 15;
    } reg;
} DPU_ARSR_CSC_P20_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int csc_p21 : 17;
        unsigned int reserved : 15;
    } reg;
} DPU_ARSR_CSC_P21_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int csc_p22 : 17;
        unsigned int reserved : 15;
    } reg;
} DPU_ARSR_CSC_P22_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int csc_module_en : 1;
        unsigned int reserved : 31;
    } reg;
} DPU_ARSR_CSC_ICG_MODULE_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int csc_mprec : 3;
        unsigned int reserved : 29;
    } reg;
} DPU_ARSR_CSC_MPREC_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int scf_en_hscl_str : 1;
        unsigned int scf_en_hscl_str_a : 1;
        unsigned int reserved : 30;
    } reg;
} DPU_ARSR_SCF_EN_HSCL_STR_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int scf_en_vscl_str : 1;
        unsigned int scf_en_vscl_str_a : 1;
        unsigned int reserved : 30;
    } reg;
} DPU_ARSR_SCF_EN_VSCL_STR_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int scf_h_v_order : 1;
        unsigned int reserved : 31;
    } reg;
} DPU_ARSR_SCF_H_V_ORDER_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int scf_a_core_gt : 1;
        unsigned int scf_r_core_gt : 1;
        unsigned int scf_g_core_gt : 1;
        unsigned int scf_b_core_gt : 1;
        unsigned int reserved : 28;
    } reg;
} DPU_ARSR_SCF_CH_CORE_GT_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int scf_input_height : 13;
        unsigned int reserved_0 : 3;
        unsigned int scf_input_width : 13;
        unsigned int reserved_1 : 3;
    } reg;
} DPU_ARSR_SCF_INPUT_WIDTH_HEIGHT_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int scf_output_height : 13;
        unsigned int reserved_0 : 3;
        unsigned int scf_output_width : 13;
        unsigned int reserved_1 : 3;
    } reg;
} DPU_ARSR_SCF_OUTPUT_WIDTH_HEIGHT_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int scf_hcoef_mem_ctrl : 4;
        unsigned int scf_vcoef_mem_ctrl : 4;
        unsigned int reserved : 24;
    } reg;
} DPU_ARSR_SCF_COEF_MEM_CTRL_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int scf_en_hscl_en : 1;
        unsigned int reserved : 31;
    } reg;
} DPU_ARSR_SCF_EN_HSCL_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int scf_en_vscl_en : 1;
        unsigned int scf_out_buffer_en : 1;
        unsigned int reserved : 30;
    } reg;
} DPU_ARSR_SCF_EN_VSCL_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int scf_acc_hscl : 30;
        unsigned int reserved : 2;
    } reg;
} DPU_ARSR_SCF_ACC_HSCL_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int scf_acc_hscl1 : 19;
        unsigned int reserved : 13;
    } reg;
} DPU_ARSR_SCF_ACC_HSCL1_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int scf_inc_hscl : 24;
        unsigned int reserved : 8;
    } reg;
} DPU_ARSR_SCF_INC_HSCL_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int scf_acc_vscl : 30;
        unsigned int reserved : 2;
    } reg;
} DPU_ARSR_SCF_ACC_VSCL_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int scf_acc_vscl1 : 19;
        unsigned int reserved : 13;
    } reg;
} DPU_ARSR_SCF_ACC_VSCL1_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int scf_inc_vscl : 24;
        unsigned int reserved : 8;
    } reg;
} DPU_ARSR_SCF_INC_VSCL_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int scf_sw_rst : 1;
        unsigned int reserved : 31;
    } reg;
} DPU_ARSR_SCF_EN_NONLINEAR_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int scf_en_mmp : 1;
        unsigned int reserved : 31;
    } reg;
} DPU_ARSR_SCF_EN_MMP_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int scf_debug_h0 : 32;
    } reg;
} DPU_ARSR_SCF_DB_H0_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int scf_debug_h1 : 32;
    } reg;
} DPU_ARSR_SCF_DB_H1_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int scf_debug_v0 : 32;
    } reg;
} DPU_ARSR_SCF_DB_V0_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int scf_debug_v1 : 32;
    } reg;
} DPU_ARSR_SCF_DB_V1_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int scf_lb_mem_ctrl : 4;
        unsigned int reserved : 28;
    } reg;
} DPU_ARSR_SCF_LB_MEM_CTRL_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int arsr_input_height : 13;
        unsigned int reserved_0 : 3;
        unsigned int arsr_input_width : 13;
        unsigned int reserved_1 : 3;
    } reg;
} DPU_ARSR_INPUT_WIDTH_HEIGHT_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int arsr_output_height : 13;
        unsigned int reserved_0 : 3;
        unsigned int arsr_output_width : 13;
        unsigned int reserved_1 : 3;
    } reg;
} DPU_ARSR_OUTPUT_WIDTH_HEIGHT_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int ihleft : 29;
        unsigned int reserved : 3;
    } reg;
} DPU_ARSR_IHLEFT_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int ihleft1 : 29;
        unsigned int reserved : 3;
    } reg;
} DPU_ARSR_IHLEFT1_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int ihright : 29;
        unsigned int reserved : 3;
    } reg;
} DPU_ARSR_IHRIGHT_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int ihright1 : 29;
        unsigned int reserved : 3;
    } reg;
} DPU_ARSR_IHRIGHT1_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int ivtop : 29;
        unsigned int reserved : 3;
    } reg;
} DPU_ARSR_IVTOP_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int ivbottom : 29;
        unsigned int reserved : 3;
    } reg;
} DPU_ARSR_IVBOTTOM_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int ivbottom1 : 29;
        unsigned int reserved : 3;
    } reg;
} DPU_ARSR_IVBOTTOM1_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int ihinc : 22;
        unsigned int reserved : 10;
    } reg;
} DPU_ARSR_IHINC_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int ivinc : 22;
        unsigned int reserved : 10;
    } reg;
} DPU_ARSR_IVINC_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int uv_voffset : 6;
        unsigned int reserved_0 : 10;
        unsigned int uv_hoffset : 6;
        unsigned int reserved_1 : 10;
    } reg;
} DPU_ARSR_OFFSET_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int bypass : 1;
        unsigned int sharpenen : 1;
        unsigned int shootdetecten : 1;
        unsigned int skinctrlen : 1;
        unsigned int textureanalysisen : 1;
        unsigned int diintplen : 1;
        unsigned int nearesten : 1;
        unsigned int prescaleren : 1;
        unsigned int nointplen : 1;
        unsigned int dbg_en : 3;
        unsigned int lowpower_en : 1;
        unsigned int arsr2p_ppro_bypass : 1;
        unsigned int data_format : 1;
        unsigned int reserved : 17;
    } reg;
} DPU_ARSR_MODE_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int ythresl : 10;
        unsigned int ythresh : 10;
        unsigned int yexpectvalue : 10;
        unsigned int reserved : 2;
    } reg;
} DPU_ARSR_SKIN_THRES_Y_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int uthresl : 10;
        unsigned int uthresh : 10;
        unsigned int uexpectvalue : 10;
        unsigned int reserved : 2;
    } reg;
} DPU_ARSR_SKIN_THRES_U_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int vthresl : 10;
        unsigned int vthresh : 10;
        unsigned int vexpectvalue : 10;
        unsigned int reserved : 2;
    } reg;
} DPU_ARSR_SKIN_THRES_V_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int yslop : 13;
        unsigned int clipdata : 10;
        unsigned int reserved : 9;
    } reg;
} DPU_ARSR_SKIN_CFG0_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int uslop : 13;
        unsigned int reserved : 19;
    } reg;
} DPU_ARSR_SKIN_CFG1_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int vslop : 13;
        unsigned int reserved : 19;
    } reg;
} DPU_ARSR_SKIN_CFG2_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int shootslop1 : 15;
        unsigned int reserved_0 : 1;
        unsigned int shootgradalpha : 6;
        unsigned int reserved_1 : 10;
    } reg;
} DPU_ARSR_SHOOT_CFG1_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int shootgradsubthrl : 11;
        unsigned int reserved_0 : 5;
        unsigned int shootgradsubthrh : 11;
        unsigned int reserved_1 : 5;
    } reg;
} DPU_ARSR_SHOOT_CFG2_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int shootshpratio : 6;
        unsigned int reserved : 26;
    } reg;
} DPU_ARSR_SHOOT_CFG3_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int shptocuthigh0 : 10;
        unsigned int reserved_0 : 6;
        unsigned int shptocuthigh1 : 10;
        unsigned int reserved_1 : 6;
    } reg;
} DPU_ARSR_SHARP_CFG3_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int sharpshootctrll : 10;
        unsigned int reserved_0 : 6;
        unsigned int sharpshootctrlh : 10;
        unsigned int reserved_1 : 6;
    } reg;
} DPU_ARSR_SHARP_CFG4_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int reserved : 32;
    } reg;
} DPU_ARSR_SHARP_CFG5_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int blendshootctrl : 10;
        unsigned int reserved_0 : 6;
        unsigned int sharpcoring : 10;
        unsigned int reserved_1 : 6;
    } reg;
} DPU_ARSR_SHARP_CFG6_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int skinthresh : 5;
        unsigned int reserved_0 : 3;
        unsigned int skinthresl : 5;
        unsigned int reserved_1 : 19;
    } reg;
} DPU_ARSR_SHARP_CFG7_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int linearshratiol : 8;
        unsigned int reserved_0 : 8;
        unsigned int linearshratioh : 8;
        unsigned int reserved_1 : 8;
    } reg;
} DPU_ARSR_SHARP_CFG8_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int sharplevel_mf : 4;
        unsigned int reserved_0 : 12;
        unsigned int sharplevel_hf : 4;
        unsigned int reserved_1 : 12;
    } reg;
} DPU_ARSR_SHARPLEVEL_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int shpgainlow_mf : 12;
        unsigned int reserved_0 : 4;
        unsigned int shpgainlow_hf : 12;
        unsigned int reserved_1 : 4;
    } reg;
} DPU_ARSR_SHPGAIN_LOW_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int shpgainmid_mf : 12;
        unsigned int reserved_0 : 4;
        unsigned int shpgainmid_hf : 12;
        unsigned int reserved_1 : 4;
    } reg;
} DPU_ARSR_SHPGAIN_MID_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int shpgainhigh_mf : 12;
        unsigned int reserved_0 : 4;
        unsigned int shpgainhigh_hf : 12;
        unsigned int reserved_1 : 4;
    } reg;
} DPU_ARSR_SHPGAIN_HIGH_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int gainctrlsloph_mf : 22;
        unsigned int reserved : 10;
    } reg;
} DPU_ARSR_GAINCTRLSLOPH_MF_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int gainctrlslopl_mf : 22;
        unsigned int reserved : 10;
    } reg;
} DPU_ARSR_GAINCTRLSLOPL_MF_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int gainctrlsloph_hf : 22;
        unsigned int reserved : 10;
    } reg;
} DPU_ARSR_GAINCTRLSLOPH_HF_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int gainctrlslopl_hf : 22;
        unsigned int reserved : 10;
    } reg;
} DPU_ARSR_GAINCTRLSLOPL_HF_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int mf_lmt : 10;
        unsigned int reserved : 22;
    } reg;
} DPU_ARSR_MF_LMT_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int gain_mf_l : 12;
        unsigned int reserved_0: 4;
        unsigned int gain_mf_h : 12;
        unsigned int reserved_1: 4;
    } reg;
} DPU_ARSR_GAIN_MF_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int mf_b : 23;
        unsigned int reserved : 9;
    } reg;
} DPU_ARSR_MF_B_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int hf_lmt : 10;
        unsigned int reserved : 22;
    } reg;
} DPU_ARSR_HF_LMT_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int gain_hf_l : 12;
        unsigned int reserved_0: 4;
        unsigned int gain_hf_h : 12;
        unsigned int reserved_1: 4;
    } reg;
} DPU_ARSR_GAIN_HF_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int hf_b : 23;
        unsigned int reserved : 9;
    } reg;
} DPU_ARSR_HF_B_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int lf_ctrl_l : 8;
        unsigned int reserved_0: 8;
        unsigned int lf_ctrl_h : 8;
        unsigned int reserved_1: 8;
    } reg;
} DPU_ARSR_LF_CTRL_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int lf_var_l : 10;
        unsigned int reserved_0: 6;
        unsigned int lf_var_h : 10;
        unsigned int reserved_1: 6;
    } reg;
} DPU_ARSR_LF_VAR_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int lf_ctrl_slop : 16;
        unsigned int reserved : 16;
    } reg;
} DPU_ARSR_LF_CTRL_SLOP_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int hf_select : 1;
        unsigned int reserved : 31;
    } reg;
} DPU_ARSR_HF_SELECT_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int shpgainvarhigh0 : 10;
        unsigned int reserved_0 : 6;
        unsigned int shpgainvarhigh1 : 10;
        unsigned int reserved_1 : 6;
    } reg;
} DPU_ARSR_SHARP_CFG2_H_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int shpgainvarlow0 : 10;
        unsigned int reserved_0 : 6;
        unsigned int shpgainvarlow1 : 10;
        unsigned int reserved_1 : 6;
    } reg;
} DPU_ARSR_SHARP_CFG2_L_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int difflow : 8;
        unsigned int reserved_0: 8;
        unsigned int diffhigh : 8;
        unsigned int reserved_1: 8;
    } reg;
} DPU_ARSR_TEXTURW_ANALYSTS_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int intplshootctrl : 10;
        unsigned int reserved : 22;
    } reg;
} DPU_ARSR_INTPLSHOOTCTRL_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int arsr2p_debug_h0 : 32;
    } reg;
} DPU_ARSR_ARSR2P_DEBUG0_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int arsr2p_debug_h1 : 32;
    } reg;
} DPU_ARSR_ARSR2P_DEBUG1_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int arsr2p_debug_v0 : 32;
    } reg;
} DPU_ARSR_ARSR2P_DEBUG2_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int arsr2p_debug_v1 : 32;
    } reg;
} DPU_ARSR_ARSR2P_DEBUG3_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int arsr_lb_mem_ctrl : 4;
        unsigned int reserved : 28;
    } reg;
} DPU_ARSR_ARSR2P_LB_MEM_CTRL_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int arsr_hcoef_mem_ctrl : 4;
        unsigned int arsr_vcoef_mem_ctrl : 4;
        unsigned int reserved : 24;
    } reg;
} DPU_ARSR_COEF_MEM_CTRL_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int post_csc_idc0 : 11;
        unsigned int reserved_0 : 5;
        unsigned int post_csc_idc1 : 11;
        unsigned int reserved_1 : 5;
    } reg;
} DPU_ARSR_POST_CSC_IDC0_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int post_csc_idc2 : 11;
        unsigned int reserved : 21;
    } reg;
} DPU_ARSR_POST_CSC_IDC2_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int post_csc_odc0 : 11;
        unsigned int reserved_0 : 5;
        unsigned int post_csc_odc1 : 11;
        unsigned int reserved_1 : 5;
    } reg;
} DPU_ARSR_POST_CSC_ODC0_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int post_csc_odc2 : 11;
        unsigned int reserved : 21;
    } reg;
} DPU_ARSR_POST_CSC_ODC2_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int post_csc_p00 : 17;
        unsigned int reserved : 15;
    } reg;
} DPU_ARSR_POST_CSC_P00_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int post_csc_p01 : 17;
        unsigned int reserved : 15;
    } reg;
} DPU_ARSR_POST_CSC_P01_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int post_csc_p02 : 17;
        unsigned int reserved : 15;
    } reg;
} DPU_ARSR_POST_CSC_P02_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int post_csc_p10 : 17;
        unsigned int reserved : 15;
    } reg;
} DPU_ARSR_POST_CSC_P10_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int post_csc_p11 : 17;
        unsigned int reserved : 15;
    } reg;
} DPU_ARSR_POST_CSC_P11_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int post_csc_p12 : 17;
        unsigned int reserved : 15;
    } reg;
} DPU_ARSR_POST_CSC_P12_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int post_csc_p20 : 17;
        unsigned int reserved : 15;
    } reg;
} DPU_ARSR_POST_CSC_P20_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int post_csc_p21 : 17;
        unsigned int reserved : 15;
    } reg;
} DPU_ARSR_POST_CSC_P21_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int post_csc_p22 : 17;
        unsigned int reserved : 15;
    } reg;
} DPU_ARSR_POST_CSC_P22_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int post_csc_module_en : 1;
        unsigned int reserved : 31;
    } reg;
} DPU_ARSR_POST_CSC_MODULE_EN_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int post_csc_mprec : 3;
        unsigned int reserved : 29;
    } reg;
} DPU_ARSR_POST_CSC_MPREC_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int post_clip_size_vrt : 13;
        unsigned int reserved_0 : 3;
        unsigned int post_clip_size_hrz : 13;
        unsigned int reserved_1 : 3;
    } reg;
} DPU_ARSR_POST_CLIP_DISP_SIZE_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int post_clip_right : 6;
        unsigned int reserved_0 : 10;
        unsigned int post_clip_left : 6;
        unsigned int reserved_1 : 10;
    } reg;
} DPU_ARSR_POST_CLIP_CTL_HRZ_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int post_clip_bot : 6;
        unsigned int reserved_0 : 10;
        unsigned int post_clip_top : 6;
        unsigned int reserved_1 : 10;
    } reg;
} DPU_ARSR_POST_CLIP_CTL_VRZ_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int post_clip_enable : 1;
        unsigned int reserved : 31;
    } reg;
} DPU_ARSR_POST_CLIP_EN_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int scf_video_6tap_coef : 32;
    } reg;
} DPU_ARSR_V0_SCF_VIDEO_6TAP_COEF_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int scf_video_5tap_coef : 32;
    } reg;
} DPU_ARSR_V0_SCF_VIDEO_5TAP_COEF_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int scf_video_4tap_coef : 32;
    } reg;
} DPU_ARSR_V0_SCF_VIDEO_4TAP_COEF_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int coefy_v : 27;
        unsigned int reserved : 5;
    } reg;
} DPU_ARSR_COEFY_V_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int coefy_h : 27;
        unsigned int reserved : 5;
    } reg;
} DPU_ARSR_COEFY_H_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int coefa_v : 18;
        unsigned int reserved : 14;
    } reg;
} DPU_ARSR_COEFA_V_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int coefa_h : 18;
        unsigned int reserved : 14;
    } reg;
} DPU_ARSR_COEFA_H_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int coefuv_v : 18;
        unsigned int reserved : 14;
    } reg;
} DPU_ARSR_COEFUV_V_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int coefuv_h : 18;
        unsigned int reserved : 14;
    } reg;
} DPU_ARSR_COEFUV_H_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int vscf_top_rd_shadow : 1;
        unsigned int reserved : 31;
    } reg;
} DPU_VSCF_RD_SHADOW_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int reg_default : 1;
        unsigned int reserved : 31;
    } reg;
} DPU_VSCF_REG_DEFAULT_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int vscf_top_clk_sel : 32;
    } reg;
} DPU_VSCF_TOP_CLK_SEL_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int vscf_top_clk_en : 32;
    } reg;
} DPU_VSCF_TOP_CLK_EN_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int reg_ctrl_scene_id : 3;
        unsigned int reserved : 29;
    } reg;
} DPU_VSCF_REG_CTRL_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int reg_ctrl_flush_en : 1;
        unsigned int reserved : 31;
    } reg;
} DPU_VSCF_REG_CTRL_FLUSH_EN_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int reg_ctrl_dbg : 32;
    } reg;
} DPU_VSCF_REG_CTRL_DEBUG_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int reg_ctrl_state : 32;
    } reg;
} DPU_VSCF_REG_CTRL_STATE_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int r_lb_debug0 : 32;
    } reg;
} DPU_VSCF_R_LB_DEBUG0_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int r_lb_debug1 : 32;
    } reg;
} DPU_VSCF_R_LB_DEBUG1_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int r_lb_debug2 : 32;
    } reg;
} DPU_VSCF_R_LB_DEBUG2_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int r_lb_debug3 : 32;
    } reg;
} DPU_VSCF_R_LB_DEBUG3_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int w_lb_debug0 : 32;
    } reg;
} DPU_VSCF_W_LB_DEBUG0_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int w_lb_debug1 : 32;
    } reg;
} DPU_VSCF_W_LB_DEBUG1_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int w_lb_debug2 : 32;
    } reg;
} DPU_VSCF_W_LB_DEBUG2_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int w_lb_debug3 : 32;
    } reg;
} DPU_VSCF_W_LB_DEBUG3_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int reg_ctrl_layer_id : 5;
        unsigned int reserved : 27;
    } reg;
} DPU_VSCF_REG_CTRL_LAYER_ID_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int dfc_size_vrt : 13;
        unsigned int reserved_0 : 3;
        unsigned int dfc_size_hrz : 13;
        unsigned int reserved_1 : 3;
    } reg;
} DPU_VSCF_DFC_DISP_SIZE_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int dfc_pix_in_num : 1;
        unsigned int reserved : 31;
    } reg;
} DPU_VSCF_DFC_PIX_IN_NUM_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int dfc_glb_alpha1 : 10;
        unsigned int reserved_0 : 6;
        unsigned int dfc_glb_alpha0 : 10;
        unsigned int reserved_1 : 6;
    } reg;
} DPU_VSCF_DFC_GLB_ALPHA01_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int dfc_ax_swap : 1;
        unsigned int dfc_img_fmt : 5;
        unsigned int dfc_uv_swap : 1;
        unsigned int dfc_rb_swap : 1;
        unsigned int reserved : 24;
    } reg;
} DPU_VSCF_DFC_DISP_FMT_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int dfc_right_clip : 6;
        unsigned int reserved_0 : 10;
        unsigned int dfc_left_clip : 6;
        unsigned int reserved_1 : 10;
    } reg;
} DPU_VSCF_DFC_CLIP_CTL_HRZ_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int dfc_bot_clip : 6;
        unsigned int reserved_0 : 10;
        unsigned int dfc_top_clip : 6;
        unsigned int reserved_1 : 10;
    } reg;
} DPU_VSCF_DFC_CLIP_CTL_VRZ_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int dfc_ctl_clip_enable : 1;
        unsigned int reserved : 31;
    } reg;
} DPU_VSCF_DFC_CTL_CLIP_EN_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int dfc_module_en : 1;
        unsigned int reserved : 31;
    } reg;
} DPU_VSCF_DFC_ICG_MODULE_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int dfc_dither_en : 1;
        unsigned int reserved : 31;
    } reg;
} DPU_VSCF_DFC_DITHER_ENABLE_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int dfc_left_pad : 6;
        unsigned int reserved_0 : 2;
        unsigned int dfc_right_pad : 6;
        unsigned int reserved_1 : 2;
        unsigned int dfc_top_pad : 6;
        unsigned int reserved_2 : 2;
        unsigned int dfc_bot_pad : 6;
        unsigned int reserved_3 : 1;
        unsigned int dfc_ctl_pad_enable : 1;
    } reg;
} DPU_VSCF_DFC_PADDING_CTL_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int dfc_glb_alpha3 : 10;
        unsigned int reserved_0 : 6;
        unsigned int dfc_glb_alpha2 : 10;
        unsigned int reserved_1 : 6;
    } reg;
} DPU_VSCF_DFC_GLB_ALPHA23_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int bitext_en : 1;
        unsigned int bitext_rgb_en : 1;
        unsigned int bitext_alpha_en : 1;
        unsigned int bitext_reg_ini_cfg_en : 1;
        unsigned int reserved : 28;
    } reg;
} DPU_VSCF_BITEXT_CTL_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int bitext_reg_ini0_0 : 32;
    } reg;
} DPU_VSCF_BITEXT_REG_INI0_0_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int bitext_reg_ini0_1 : 32;
    } reg;
} DPU_VSCF_BITEXT_REG_INI0_1_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int bitext_reg_ini0_2 : 32;
    } reg;
} DPU_VSCF_BITEXT_REG_INI0_2_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int bitext_reg_ini0_3 : 31;
        unsigned int reserved : 1;
    } reg;
} DPU_VSCF_BITEXT_REG_INI0_3_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int bitext_reg_ini1_0 : 32;
    } reg;
} DPU_VSCF_BITEXT_REG_INI1_0_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int bitext_reg_ini1_1 : 32;
    } reg;
} DPU_VSCF_BITEXT_REG_INI1_1_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int bitext_reg_ini1_2 : 32;
    } reg;
} DPU_VSCF_BITEXT_REG_INI1_2_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int bitext_reg_ini1_3 : 31;
        unsigned int reserved : 1;
    } reg;
} DPU_VSCF_BITEXT_REG_INI1_3_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int bitext_reg_ini2_0 : 32;
    } reg;
} DPU_VSCF_BITEXT_REG_INI2_0_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int bitext_reg_ini2_1 : 32;
    } reg;
} DPU_VSCF_BITEXT_REG_INI2_1_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int bitext_reg_ini2_2 : 32;
    } reg;
} DPU_VSCF_BITEXT_REG_INI2_2_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int bitext_reg_ini2_3 : 31;
        unsigned int reserved : 1;
    } reg;
} DPU_VSCF_BITEXT_REG_INI2_3_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int bitext_power_ctrl_c : 2;
        unsigned int reserved : 30;
    } reg;
} DPU_VSCF_BITEXT_POWER_CTRL_C_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int bitext_power_ctrl_shift : 2;
        unsigned int reserved : 30;
    } reg;
} DPU_VSCF_BITEXT_POWER_CTRL_SHIFT_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int bitext_filt_00 : 5;
        unsigned int reserved : 27;
    } reg;
} DPU_VSCF_BITEXT_FILT_00_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int bitext_filt_01 : 5;
        unsigned int reserved : 27;
    } reg;
} DPU_VSCF_BITEXT_FILT_01_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int bitext_filt_02 : 5;
        unsigned int reserved : 27;
    } reg;
} DPU_VSCF_BITEXT_FILT_02_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int bitext_filt_10 : 5;
        unsigned int reserved : 27;
    } reg;
} DPU_VSCF_BITEXT_FILT_10_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int bitext_filt_11 : 5;
        unsigned int reserved : 27;
    } reg;
} DPU_VSCF_BITEXT_FILT_11_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int bitext_filt_12 : 5;
        unsigned int reserved : 27;
    } reg;
} DPU_VSCF_BITEXT_FILT_12_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int bitext_filt_20 : 5;
        unsigned int reserved : 27;
    } reg;
} DPU_VSCF_BITEXT_FILT_20_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int bitext_filt_21 : 5;
        unsigned int reserved : 27;
    } reg;
} DPU_VSCF_BITEXT_FILT_21_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int bitext_filt_22 : 5;
        unsigned int reserved : 27;
    } reg;
} DPU_VSCF_BITEXT_FILT_22_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int bitext_thd_r0 : 32;
    } reg;
} DPU_VSCF_BITEXT_THD_R0_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int bitext_thd_r1 : 16;
        unsigned int reserved : 16;
    } reg;
} DPU_VSCF_BITEXT_THD_R1_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int bitext_thd_g0 : 32;
    } reg;
} DPU_VSCF_BITEXT_THD_G0_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int bitext_thd_g1 : 16;
        unsigned int reserved : 16;
    } reg;
} DPU_VSCF_BITEXT_THD_G1_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int bitext_thd_b0 : 32;
    } reg;
} DPU_VSCF_BITEXT_THD_B0_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int bitext_thd_b1 : 16;
        unsigned int reserved : 16;
    } reg;
} DPU_VSCF_BITEXT_THD_B1_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int bitext_dbg0 : 32;
    } reg;
} DPU_VSCF_BITEXT0_DBG0_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int csc_idc0 : 11;
        unsigned int reserved_0: 5;
        unsigned int csc_idc1 : 11;
        unsigned int reserved_1: 5;
    } reg;
} DPU_VSCF_CSC_IDC0_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int csc_idc2 : 11;
        unsigned int reserved : 21;
    } reg;
} DPU_VSCF_CSC_IDC2_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int csc_odc0 : 11;
        unsigned int reserved_0: 5;
        unsigned int csc_odc1 : 11;
        unsigned int reserved_1: 5;
    } reg;
} DPU_VSCF_CSC_ODC0_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int csc_odc2 : 11;
        unsigned int reserved : 21;
    } reg;
} DPU_VSCF_CSC_ODC2_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int csc_p00 : 17;
        unsigned int reserved : 15;
    } reg;
} DPU_VSCF_CSC_P00_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int csc_p01 : 17;
        unsigned int reserved : 15;
    } reg;
} DPU_VSCF_CSC_P01_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int csc_p02 : 17;
        unsigned int reserved : 15;
    } reg;
} DPU_VSCF_CSC_P02_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int csc_p10 : 17;
        unsigned int reserved : 15;
    } reg;
} DPU_VSCF_CSC_P10_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int csc_p11 : 17;
        unsigned int reserved : 15;
    } reg;
} DPU_VSCF_CSC_P11_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int csc_p12 : 17;
        unsigned int reserved : 15;
    } reg;
} DPU_VSCF_CSC_P12_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int csc_p20 : 17;
        unsigned int reserved : 15;
    } reg;
} DPU_VSCF_CSC_P20_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int csc_p21 : 17;
        unsigned int reserved : 15;
    } reg;
} DPU_VSCF_CSC_P21_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int csc_p22 : 17;
        unsigned int reserved : 15;
    } reg;
} DPU_VSCF_CSC_P22_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int csc_module_en : 1;
        unsigned int reserved : 31;
    } reg;
} DPU_VSCF_CSC_ICG_MODULE_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int csc_mprec : 3;
        unsigned int reserved : 29;
    } reg;
} DPU_VSCF_CSC_MPREC_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int scf_en_hscl_str : 1;
        unsigned int scf_en_hscl_str_a : 1;
        unsigned int reserved : 30;
    } reg;
} DPU_VSCF_SCF_EN_HSCL_STR_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int scf_en_vscl_str : 1;
        unsigned int scf_en_vscl_str_a : 1;
        unsigned int reserved : 30;
    } reg;
} DPU_VSCF_SCF_EN_VSCL_STR_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int scf_h_v_order : 1;
        unsigned int reserved : 31;
    } reg;
} DPU_VSCF_SCF_H_V_ORDER_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int scf_a_core_gt : 1;
        unsigned int scf_r_core_gt : 1;
        unsigned int scf_g_core_gt : 1;
        unsigned int scf_b_core_gt : 1;
        unsigned int reserved : 28;
    } reg;
} DPU_VSCF_SCF_CH_CORE_GT_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int scf_input_height : 13;
        unsigned int reserved_0 : 3;
        unsigned int scf_input_width : 13;
        unsigned int reserved_1 : 3;
    } reg;
} DPU_VSCF_SCF_INPUT_WIDTH_HEIGHT_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int scf_output_height : 13;
        unsigned int reserved_0 : 3;
        unsigned int scf_output_width : 13;
        unsigned int reserved_1 : 3;
    } reg;
} DPU_VSCF_SCF_OUTPUT_WIDTH_HEIGHT_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int scf_hcoef_mem_ctrl : 4;
        unsigned int scf_vcoef_mem_ctrl : 4;
        unsigned int reserved : 24;
    } reg;
} DPU_VSCF_SCF_COEF_MEM_CTRL_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int scf_en_hscl_en : 1;
        unsigned int reserved : 31;
    } reg;
} DPU_VSCF_SCF_EN_HSCL_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int scf_en_vscl_en : 1;
        unsigned int scf_out_buffer_en : 1;
        unsigned int reserved : 30;
    } reg;
} DPU_VSCF_SCF_EN_VSCL_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int scf_acc_hscl : 30;
        unsigned int reserved : 2;
    } reg;
} DPU_VSCF_SCF_ACC_HSCL_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int scf_acc_hscl1 : 19;
        unsigned int reserved : 13;
    } reg;
} DPU_VSCF_SCF_ACC_HSCL1_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int scf_inc_hscl : 24;
        unsigned int reserved : 8;
    } reg;
} DPU_VSCF_SCF_INC_HSCL_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int scf_acc_vscl : 30;
        unsigned int reserved : 2;
    } reg;
} DPU_VSCF_SCF_ACC_VSCL_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int scf_acc_vscl1 : 19;
        unsigned int reserved : 13;
    } reg;
} DPU_VSCF_SCF_ACC_VSCL1_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int scf_inc_vscl : 24;
        unsigned int reserved : 8;
    } reg;
} DPU_VSCF_SCF_INC_VSCL_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int scf_sw_rst : 1;
        unsigned int reserved : 31;
    } reg;
} DPU_VSCF_SCF_EN_NONLINEAR_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int scf_en_mmp : 1;
        unsigned int reserved : 31;
    } reg;
} DPU_VSCF_SCF_EN_MMP_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int scf_debug_h0 : 32;
    } reg;
} DPU_VSCF_SCF_DB_H0_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int scf_debug_h1 : 32;
    } reg;
} DPU_VSCF_SCF_DB_H1_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int scf_debug_v0 : 32;
    } reg;
} DPU_VSCF_SCF_DB_V0_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int scf_debug_v1 : 32;
    } reg;
} DPU_VSCF_SCF_DB_V1_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int scf_lb_mem_ctrl : 4;
        unsigned int reserved : 28;
    } reg;
} DPU_VSCF_SCF_LB_MEM_CTRL_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int post_clip_size_vrt : 13;
        unsigned int reserved_0 : 3;
        unsigned int post_clip_size_hrz : 13;
        unsigned int reserved_1 : 3;
    } reg;
} DPU_VSCF_POST_CLIP_DISP_SIZE_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int post_clip_right : 6;
        unsigned int reserved_0 : 10;
        unsigned int post_clip_left : 6;
        unsigned int reserved_1 : 10;
    } reg;
} DPU_VSCF_POST_CLIP_CTL_HRZ_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int post_clip_bot : 6;
        unsigned int reserved_0 : 10;
        unsigned int post_clip_top : 6;
        unsigned int reserved_1 : 10;
    } reg;
} DPU_VSCF_POST_CLIP_CTL_VRZ_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int post_clip_enable : 1;
        unsigned int reserved : 31;
    } reg;
} DPU_VSCF_POST_CLIP_EN_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int scf_video_6tap_coef : 32;
    } reg;
} DPU_VSCF_V0_SCF_VIDEO_6TAP_COEF_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int scf_video_5tap_coef : 32;
    } reg;
} DPU_VSCF_V0_SCF_VIDEO_5TAP_COEF_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int scf_video_4tap_coef : 32;
    } reg;
} DPU_VSCF_V0_SCF_VIDEO_4TAP_COEF_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int hdr_top_rd_shadow : 1;
        unsigned int reserved : 31;
    } reg;
} DPU_HDR_RD_SHADOW_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int reg_default : 1;
        unsigned int reserved : 31;
    } reg;
} DPU_HDR_REG_DEFAULT_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int hdr_top_clk_sel : 32;
    } reg;
} DPU_HDR_TOP_CLK_SEL_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int hdr_top_clk_en : 32;
    } reg;
} DPU_HDR_TOP_CLK_EN_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int reg_ctrl_scene_id : 3;
        unsigned int reserved : 29;
    } reg;
} DPU_HDR_REG_CTRL_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int reg_ctrl_flush_en : 1;
        unsigned int reserved : 31;
    } reg;
} DPU_HDR_REG_CTRL_FLUSH_EN_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int reg_ctrl_dbg : 32;
    } reg;
} DPU_HDR_REG_CTRL_DEBUG_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int reg_ctrl_state : 32;
    } reg;
} DPU_HDR_REG_CTRL_STATE_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int r_lb_debug0 : 32;
    } reg;
} DPU_HDR_R_LB_DEBUG0_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int r_lb_debug1 : 32;
    } reg;
} DPU_HDR_R_LB_DEBUG1_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int r_lb_debug2 : 32;
    } reg;
} DPU_HDR_R_LB_DEBUG2_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int r_lb_debug3 : 32;
    } reg;
} DPU_HDR_R_LB_DEBUG3_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int w_lb_debug0 : 32;
    } reg;
} DPU_HDR_W_LB_DEBUG0_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int w_lb_debug1 : 32;
    } reg;
} DPU_HDR_W_LB_DEBUG1_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int w_lb_debug2 : 32;
    } reg;
} DPU_HDR_W_LB_DEBUG2_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int w_lb_debug3 : 32;
    } reg;
} DPU_HDR_W_LB_DEBUG3_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int reg_ctrl_layer_id : 5;
        unsigned int reserved : 27;
    } reg;
} DPU_HDR_REG_CTRL_LAYER_ID_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int dfc_size_vrt : 13;
        unsigned int reserved_0 : 3;
        unsigned int dfc_size_hrz : 13;
        unsigned int reserved_1 : 3;
    } reg;
} DPU_HDR_DFC_DISP_SIZE_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int dfc_pix_in_num : 1;
        unsigned int reserved : 31;
    } reg;
} DPU_HDR_DFC_PIX_IN_NUM_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int dfc_glb_alpha1 : 10;
        unsigned int reserved_0 : 6;
        unsigned int dfc_glb_alpha0 : 10;
        unsigned int reserved_1 : 6;
    } reg;
} DPU_HDR_DFC_GLB_ALPHA01_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int dfc_ax_swap : 1;
        unsigned int dfc_img_fmt : 5;
        unsigned int dfc_uv_swap : 1;
        unsigned int dfc_rb_swap : 1;
        unsigned int reserved : 24;
    } reg;
} DPU_HDR_DFC_DISP_FMT_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int dfc_right_clip : 6;
        unsigned int reserved_0 : 10;
        unsigned int dfc_left_clip : 6;
        unsigned int reserved_1 : 10;
    } reg;
} DPU_HDR_DFC_CLIP_CTL_HRZ_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int dfc_bot_clip : 6;
        unsigned int reserved_0 : 10;
        unsigned int dfc_top_clip : 6;
        unsigned int reserved_1 : 10;
    } reg;
} DPU_HDR_DFC_CLIP_CTL_VRZ_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int dfc_ctl_clip_enable : 1;
        unsigned int reserved : 31;
    } reg;
} DPU_HDR_DFC_CTL_CLIP_EN_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int dfc_module_en : 1;
        unsigned int reserved : 31;
    } reg;
} DPU_HDR_DFC_ICG_MODULE_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int dfc_dither_en : 1;
        unsigned int reserved : 31;
    } reg;
} DPU_HDR_DFC_DITHER_ENABLE_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int dfc_left_pad : 6;
        unsigned int reserved_0 : 2;
        unsigned int dfc_right_pad : 6;
        unsigned int reserved_1 : 2;
        unsigned int dfc_top_pad : 6;
        unsigned int reserved_2 : 2;
        unsigned int dfc_bot_pad : 6;
        unsigned int reserved_3 : 1;
        unsigned int dfc_ctl_pad_enable : 1;
    } reg;
} DPU_HDR_DFC_PADDING_CTL_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int dfc_glb_alpha3 : 10;
        unsigned int reserved_0 : 6;
        unsigned int dfc_glb_alpha2 : 10;
        unsigned int reserved_1 : 6;
    } reg;
} DPU_HDR_DFC_GLB_ALPHA23_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int bitext_en : 1;
        unsigned int bitext_rgb_en : 1;
        unsigned int bitext_alpha_en : 1;
        unsigned int bitext_reg_ini_cfg_en : 1;
        unsigned int reserved : 28;
    } reg;
} DPU_HDR_BITEXT_CTL_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int bitext_reg_ini0_0 : 32;
    } reg;
} DPU_HDR_BITEXT_REG_INI0_0_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int bitext_reg_ini0_1 : 32;
    } reg;
} DPU_HDR_BITEXT_REG_INI0_1_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int bitext_reg_ini0_2 : 32;
    } reg;
} DPU_HDR_BITEXT_REG_INI0_2_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int bitext_reg_ini0_3 : 31;
        unsigned int reserved : 1;
    } reg;
} DPU_HDR_BITEXT_REG_INI0_3_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int bitext_reg_ini1_0 : 32;
    } reg;
} DPU_HDR_BITEXT_REG_INI1_0_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int bitext_reg_ini1_1 : 32;
    } reg;
} DPU_HDR_BITEXT_REG_INI1_1_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int bitext_reg_ini1_2 : 32;
    } reg;
} DPU_HDR_BITEXT_REG_INI1_2_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int bitext_reg_ini1_3 : 31;
        unsigned int reserved : 1;
    } reg;
} DPU_HDR_BITEXT_REG_INI1_3_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int bitext_reg_ini2_0 : 32;
    } reg;
} DPU_HDR_BITEXT_REG_INI2_0_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int bitext_reg_ini2_1 : 32;
    } reg;
} DPU_HDR_BITEXT_REG_INI2_1_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int bitext_reg_ini2_2 : 32;
    } reg;
} DPU_HDR_BITEXT_REG_INI2_2_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int bitext_reg_ini2_3 : 31;
        unsigned int reserved : 1;
    } reg;
} DPU_HDR_BITEXT_REG_INI2_3_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int bitext_power_ctrl_c : 2;
        unsigned int reserved : 30;
    } reg;
} DPU_HDR_BITEXT_POWER_CTRL_C_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int bitext_power_ctrl_shift : 2;
        unsigned int reserved : 30;
    } reg;
} DPU_HDR_BITEXT_POWER_CTRL_SHIFT_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int bitext_filt_00 : 5;
        unsigned int reserved : 27;
    } reg;
} DPU_HDR_BITEXT_FILT_00_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int bitext_filt_01 : 5;
        unsigned int reserved : 27;
    } reg;
} DPU_HDR_BITEXT_FILT_01_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int bitext_filt_02 : 5;
        unsigned int reserved : 27;
    } reg;
} DPU_HDR_BITEXT_FILT_02_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int bitext_filt_10 : 5;
        unsigned int reserved : 27;
    } reg;
} DPU_HDR_BITEXT_FILT_10_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int bitext_filt_11 : 5;
        unsigned int reserved : 27;
    } reg;
} DPU_HDR_BITEXT_FILT_11_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int bitext_filt_12 : 5;
        unsigned int reserved : 27;
    } reg;
} DPU_HDR_BITEXT_FILT_12_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int bitext_filt_20 : 5;
        unsigned int reserved : 27;
    } reg;
} DPU_HDR_BITEXT_FILT_20_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int bitext_filt_21 : 5;
        unsigned int reserved : 27;
    } reg;
} DPU_HDR_BITEXT_FILT_21_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int bitext_filt_22 : 5;
        unsigned int reserved : 27;
    } reg;
} DPU_HDR_BITEXT_FILT_22_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int bitext_thd_r0 : 32;
    } reg;
} DPU_HDR_BITEXT_THD_R0_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int bitext_thd_r1 : 16;
        unsigned int reserved : 16;
    } reg;
} DPU_HDR_BITEXT_THD_R1_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int bitext_thd_g0 : 32;
    } reg;
} DPU_HDR_BITEXT_THD_G0_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int bitext_thd_g1 : 16;
        unsigned int reserved : 16;
    } reg;
} DPU_HDR_BITEXT_THD_G1_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int bitext_thd_b0 : 32;
    } reg;
} DPU_HDR_BITEXT_THD_B0_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int bitext_thd_b1 : 16;
        unsigned int reserved : 16;
    } reg;
} DPU_HDR_BITEXT_THD_B1_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int bitext_dbg0 : 32;
    } reg;
} DPU_HDR_BITEXT0_DBG0_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int hdr_gtm_mem_ctrl : 4;
        unsigned int reserved : 28;
    } reg;
} DPU_HDR_MEM_CTRL_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int hdr_en : 1;
        unsigned int hdr_ppro_bypass : 1;
        unsigned int hdr_gtm_lut_sel : 1;
        unsigned int reserved : 29;
    } reg;
} DPU_HDR_CTRL_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int hdr_height : 16;
        unsigned int hdr_width : 16;
    } reg;
} DPU_HDR_SIZE_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int hdr_inv_size : 32;
    } reg;
} DPU_HDR_INV_SIZE_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int hdr_stt_mean_y : 10;
        unsigned int reserved : 22;
    } reg;
} DPU_HDR_MEAN_STT_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int hdr_csc_module_en : 1;
        unsigned int hdr_csc_isinrgb : 1;
        unsigned int reserved : 30;
    } reg;
} DPU_HDR_CSC_CTRL_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int hdr_csc_idc0 : 11;
        unsigned int reserved_0 : 5;
        unsigned int hdr_csc_idc1 : 11;
        unsigned int reserved_1 : 5;
    } reg;
} DPU_HDR_CSC_IDC0_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int hdr_csc_idc2 : 11;
        unsigned int reserved : 21;
    } reg;
} DPU_HDR_CSC_IDC2_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int hdr_csc_odc0 : 13;
        unsigned int reserved_0 : 3;
        unsigned int hdr_csc_odc1 : 13;
        unsigned int reserved_1 : 3;
    } reg;
} DPU_HDR_CSC_ODC0_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int hdr_csc_odc2 : 13;
        unsigned int reserved : 19;
    } reg;
} DPU_HDR_CSC_ODC2_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int hdr_csc_p00 : 20;
        unsigned int reserved : 12;
    } reg;
} DPU_HDR_CSC_P00_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int hdr_csc_p01 : 20;
        unsigned int reserved : 12;
    } reg;
} DPU_HDR_CSC_P01_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int hdr_csc_p02 : 20;
        unsigned int reserved : 12;
    } reg;
} DPU_HDR_CSC_P02_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int hdr_csc_p10 : 20;
        unsigned int reserved : 12;
    } reg;
} DPU_HDR_CSC_P10_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int hdr_csc_p11 : 20;
        unsigned int reserved : 12;
    } reg;
} DPU_HDR_CSC_P11_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int hdr_csc_p12 : 20;
        unsigned int reserved : 12;
    } reg;
} DPU_HDR_CSC_P12_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int hdr_csc_p20 : 20;
        unsigned int reserved : 12;
    } reg;
} DPU_HDR_CSC_P20_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int hdr_csc_p21 : 20;
        unsigned int reserved : 12;
    } reg;
} DPU_HDR_CSC_P21_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int hdr_csc_p22 : 20;
        unsigned int reserved : 12;
    } reg;
} DPU_HDR_CSC_P22_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int hdr_gtm_isinslf : 1;
        unsigned int hdr_gtm_pq2slf_en : 1;
        unsigned int hdr_gtm_lumcolor_en : 1;
        unsigned int hdr_gtm_coloryuv_en : 1;
        unsigned int reserved : 28;
    } reg;
} DPU_HDR_GTM_CTRL_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int hdr_gtm_input_min_e : 14;
        unsigned int reserved_0 : 2;
        unsigned int hdr_gtm_output_min_e : 14;
        unsigned int reserved_1 : 2;
    } reg;
} DPU_HDR_GTM_SLF_LCPROC_MIN_E_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int hdr_gtm_ratiolut1inclip : 14;
        unsigned int reserved_0 : 2;
        unsigned int hdr_gtm_ratiolutoutclip : 14;
        unsigned int reserved_1 : 2;
    } reg;
} DPU_HDR_GTM_SLF_LCPROC_CLIP1_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int hdr_gtm_ratiolut0inclip : 18;
        unsigned int reserved : 14;
    } reg;
} DPU_HDR_GTM_SLF_LCPROC_CLIP2_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int hdr_gtm_lumaratiolutstep0 : 3;
        unsigned int hdr_gtm_lumaratiolutstep1 : 4;
        unsigned int reserved_0 : 1;
        unsigned int hdr_gtm_chromaratiolutstep0 : 2;
        unsigned int reserved_1 : 2;
        unsigned int hdr_gtm_chromaratiolutstep1 : 3;
        unsigned int reserved_2 : 1;
        unsigned int hdr_gtm_lumaoutbit0 : 4;
        unsigned int hdr_gtm_lumaoutbit1 : 4;
        unsigned int reserved_3 : 8;
    } reg;
} DPU_HDR_GTM_SLF_LCPROC_SCALE_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int hdr_gtm_lumalutthres : 12;
        unsigned int reserved_0 : 4;
        unsigned int hdr_gtm_chromalutthres : 8;
        unsigned int reserved_1 : 8;
    } reg;
} DPU_HDR_GTM_SLF_LCPROC_THRESHOLD_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int hdr_gtm_y_transform_0 : 12;
        unsigned int reserved_0 : 4;
        unsigned int hdr_gtm_y_transform_1 : 12;
        unsigned int reserved_1 : 4;
    } reg;
} DPU_HDR_GTM_SLF_LCPROC_Y_TRANSFM0_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int hdr_gtm_y_transform_2 : 12;
        unsigned int reserved : 20;
    } reg;
} DPU_HDR_GTM_SLF_LCPROC_Y_TRANSFM1_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int hdr_gtm_chromaoutbit1 : 5;
        unsigned int reserved_0 : 3;
        unsigned int hdr_gtm_chromaoutbit2 : 3;
        unsigned int reserved_1 : 1;
        unsigned int hdr_gtm_chromaoutbit0 : 5;
        unsigned int reserved_2 : 15;
    } reg;
} DPU_HDR_GTM_SLF_LCPROC_OUT_BIT_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int hdr_gtm_chromaratioclip : 14;
        unsigned int reserved : 18;
    } reg;
} DPU_HDR_GTM_SLF_LCPROC_CRATIOCLIP_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int hdr_gtm_pq2linear_even_coef : 16;
        unsigned int hdr_gtm_pq2linear_odd_coef : 16;
    } reg;
} DPU_HDR_GTM_PQ2LINEAR_COEF_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int hdr_gtm_linear2pq_even_coef : 12;
        unsigned int reserved_0 : 4;
        unsigned int hdr_gtm_linear2pq_odd_coef : 12;
        unsigned int reserved_1 : 4;
    } reg;
} DPU_HDR_GTM_LINEAR2PQ_COEF_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int hdr_gtm_lumalut0_even_coef : 12;
        unsigned int reserved_0 : 4;
        unsigned int hdr_gtm_lumalut0_odd_coef : 12;
        unsigned int reserved_1 : 4;
    } reg;
} DPU_HDR_GTM_LUMA0_LUT_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int hdr_gtm_chromalut0_even_coef : 12;
        unsigned int reserved_0 : 4;
        unsigned int hdr_gtm_chromalut0_odd_coef : 12;
        unsigned int reserved_1 : 4;
    } reg;
} DPU_HDR_HDR_GTM_CHROMA_LUT_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int hdr_dbg : 32;
    } reg;
} DPU_HDR_DBG_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int hdr_gtm_input_min_o : 16;
        unsigned int hdr_gtm_output_min_o : 16;
    } reg;
} DPU_HDR_GTM_SLF_LCPROC_MIN_O_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int hdr_gtm_pq2linear_x1_step : 5;
        unsigned int reserved_0 : 3;
        unsigned int hdr_gtm_pq2linear_x2_step : 5;
        unsigned int reserved_1 : 3;
        unsigned int hdr_gtm_pq2linear_x3_step : 5;
        unsigned int reserved_2 : 3;
        unsigned int hdr_gtm_pq2linear_x4_step : 5;
        unsigned int reserved_3 : 3;
    } reg;
} DPU_HDR_GTM_PQ2LINEAR_STEP1_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int hdr_gtm_pq2linear_x5_step : 5;
        unsigned int reserved_0 : 3;
        unsigned int hdr_gtm_pq2linear_x6_step : 5;
        unsigned int reserved_1 : 3;
        unsigned int hdr_gtm_pq2linear_x7_step : 5;
        unsigned int reserved_2 : 3;
        unsigned int hdr_gtm_pq2linear_x8_step : 5;
        unsigned int reserved_3 : 3;
    } reg;
} DPU_HDR_GTM_PQ2LINEAR_STEP2_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int hdr_gtm_pq2linear_x1_pos : 10;
        unsigned int reserved : 22;
    } reg;
} DPU_HDR_GTM_PQ2LINEAR_POS1_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int hdr_gtm_pq2linear_x2_pos : 10;
        unsigned int reserved : 22;
    } reg;
} DPU_HDR_GTM_PQ2LINEAR_POS2_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int hdr_gtm_pq2linear_x3_pos : 10;
        unsigned int reserved : 22;
    } reg;
} DPU_HDR_GTM_PQ2LINEAR_POS3_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int hdr_gtm_pq2linear_x4_pos : 10;
        unsigned int reserved : 22;
    } reg;
} DPU_HDR_GTM_PQ2LINEAR_POS4_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int hdr_gtm_pq2linear_x5_pos : 10;
        unsigned int reserved : 22;
    } reg;
} DPU_HDR_GTM_PQ2LINEAR_POS5_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int hdr_gtm_pq2linear_x6_pos : 10;
        unsigned int reserved : 22;
    } reg;
} DPU_HDR_GTM_PQ2LINEAR_POS6_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int hdr_gtm_pq2linear_x7_pos : 10;
        unsigned int reserved : 22;
    } reg;
} DPU_HDR_GTM_PQ2LINEAR_POS7_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int hdr_gtm_pq2linear_x8_pos : 10;
        unsigned int reserved : 22;
    } reg;
} DPU_HDR_GTM_PQ2LINEAR_POS8_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int hdr_gtm_pq2linear_x1_num : 9;
        unsigned int reserved_0 : 3;
        unsigned int hdr_gtm_pq2linear_x2_num : 9;
        unsigned int reserved_1 : 11;
    } reg;
} DPU_HDR_GTM_PQ2LINEAR_NUM1_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int hdr_gtm_pq2linear_x3_num : 9;
        unsigned int reserved_0 : 3;
        unsigned int hdr_gtm_pq2linear_x4_num : 9;
        unsigned int reserved_1 : 11;
    } reg;
} DPU_HDR_GTM_PQ2LINEAR_NUM2_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int hdr_gtm_pq2linear_x5_num : 9;
        unsigned int reserved_0 : 3;
        unsigned int hdr_gtm_pq2linear_x6_num : 9;
        unsigned int reserved_1 : 11;
    } reg;
} DPU_HDR_GTM_PQ2LINEAR_NUM3_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int hdr_gtm_pq2linear_x7_num : 9;
        unsigned int reserved_0 : 3;
        unsigned int hdr_gtm_pq2linear_x8_num : 9;
        unsigned int reserved_1 : 11;
    } reg;
} DPU_HDR_GTM_PQ2LINEAR_NUM4_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int hdr_gtm_linear2pq_x1_step : 5;
        unsigned int reserved_0 : 3;
        unsigned int hdr_gtm_linear2pq_x2_step : 5;
        unsigned int reserved_1 : 3;
        unsigned int hdr_gtm_linear2pq_x3_step : 5;
        unsigned int reserved_2 : 3;
        unsigned int hdr_gtm_linear2pq_x4_step : 5;
        unsigned int reserved_3 : 3;
    } reg;
} DPU_HDR_GTM_LINEAR2PQ_STEP1_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int hdr_gtm_linear2pq_x5_step : 5;
        unsigned int reserved_0 : 3;
        unsigned int hdr_gtm_linear2pq_x6_step : 5;
        unsigned int reserved_1 : 3;
        unsigned int hdr_gtm_linear2pq_x7_step : 5;
        unsigned int reserved_2 : 3;
        unsigned int hdr_gtm_linear2pq_x8_step : 5;
        unsigned int reserved_3 : 3;
    } reg;
} DPU_HDR_GTM_LINEAR2PQ_STEP2_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int hdr_gtm_linear2pq_x1_pos : 16;
        unsigned int reserved : 16;
    } reg;
} DPU_HDR_GTM_LINEAR2PQ_POS1_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int hdr_gtm_linear2pq_x2_pos : 16;
        unsigned int reserved : 16;
    } reg;
} DPU_HDR_GTM_LINEAR2PQ_POS2_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int hdr_gtm_linear2pq_x3_pos : 16;
        unsigned int reserved : 16;
    } reg;
} DPU_HDR_GTM_LINEAR2PQ_POS3_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int hdr_gtm_linear2pq_x4_pos : 16;
        unsigned int reserved : 16;
    } reg;
} DPU_HDR_GTM_LINEAR2PQ_POS4_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int hdr_gtm_linear2pq_x5_pos : 16;
        unsigned int reserved : 16;
    } reg;
} DPU_HDR_GTM_LINEAR2PQ_POS5_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int hdr_gtm_linear2pq_x6_pos : 16;
        unsigned int reserved : 16;
    } reg;
} DPU_HDR_GTM_LINEAR2PQ_POS6_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int hdr_gtm_linear2pq_x7_pos : 16;
        unsigned int reserved : 16;
    } reg;
} DPU_HDR_GTM_LINEAR2PQ_POS7_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int hdr_gtm_linear2pq_x8_pos : 16;
        unsigned int reserved : 16;
    } reg;
} DPU_HDR_GTM_LINEAR2PQ_POS8_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int hdr_gtm_linear2pq_x1_num : 9;
        unsigned int reserved_0 : 3;
        unsigned int hdr_gtm_linear2pq_x2_num : 9;
        unsigned int reserved_1 : 11;
    } reg;
} DPU_HDR_GTM_LINEAR2PQ_NUM1_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int hdr_gtm_linear2pq_x3_num : 9;
        unsigned int reserved_0 : 3;
        unsigned int hdr_gtm_linear2pq_x4_num : 9;
        unsigned int reserved_1 : 11;
    } reg;
} DPU_HDR_GTM_LINEAR2PQ_NUM2_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int hdr_gtm_linear2pq_x5_num : 9;
        unsigned int reserved_0 : 3;
        unsigned int hdr_gtm_linear2pq_x6_num : 9;
        unsigned int reserved_1 : 11;
    } reg;
} DPU_HDR_GTM_LINEAR2PQ_NUM3_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int hdr_gtm_linear2pq_x7_num : 9;
        unsigned int reserved_0 : 3;
        unsigned int hdr_gtm_linear2pq_x8_num : 9;
        unsigned int reserved_1 : 11;
    } reg;
} DPU_HDR_GTM_LINEAR2PQ_NUM4_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int hdr_gtm_rgb2y_wt : 7;
        unsigned int reserved : 25;
    } reg;
} DPU_HDR_GTM_RGB2Y_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int hdr_csc1_idc0 : 11;
        unsigned int reserved_0 : 5;
        unsigned int hdr_csc1_idc1 : 11;
        unsigned int reserved_1 : 5;
    } reg;
} DPU_HDR_CSC1_IDC0_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int hdr_csc1_idc2 : 11;
        unsigned int reserved : 21;
    } reg;
} DPU_HDR_CSC1_IDC2_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int hdr_csc1_odc0 : 13;
        unsigned int reserved_0 : 3;
        unsigned int hdr_csc1_odc1 : 13;
        unsigned int reserved_1 : 3;
    } reg;
} DPU_HDR_CSC1_ODC0_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int hdr_csc1_odc2 : 13;
        unsigned int reserved : 19;
    } reg;
} DPU_HDR_CSC1_ODC2_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int hdr_csc1_p00 : 20;
        unsigned int reserved : 12;
    } reg;
} DPU_HDR_CSC1_P00_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int hdr_csc1_p01 : 20;
        unsigned int reserved : 12;
    } reg;
} DPU_HDR_CSC1_P01_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int hdr_csc1_p02 : 20;
        unsigned int reserved : 12;
    } reg;
} DPU_HDR_CSC1_P02_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int hdr_csc1_p10 : 20;
        unsigned int reserved : 12;
    } reg;
} DPU_HDR_CSC1_P10_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int hdr_csc1_p11 : 20;
        unsigned int reserved : 12;
    } reg;
} DPU_HDR_CSC1_P11_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int hdr_csc1_p12 : 20;
        unsigned int reserved : 12;
    } reg;
} DPU_HDR_CSC1_P12_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int hdr_csc1_p20 : 20;
        unsigned int reserved : 12;
    } reg;
} DPU_HDR_CSC1_P20_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int hdr_csc1_p21 : 20;
        unsigned int reserved : 12;
    } reg;
} DPU_HDR_CSC1_P21_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int hdr_csc1_p22 : 20;
        unsigned int reserved : 12;
    } reg;
} DPU_HDR_CSC1_P22_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int hdr_csc2_idc0 : 11;
        unsigned int reserved_0 : 5;
        unsigned int hdr_csc2_idc1 : 11;
        unsigned int reserved_1 : 5;
    } reg;
} DPU_HDR_CSC2_IDC0_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int hdr_csc2_idc2 : 11;
        unsigned int reserved : 21;
    } reg;
} DPU_HDR_CSC2_IDC2_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int hdr_csc2_odc0 : 13;
        unsigned int reserved_0 : 3;
        unsigned int hdr_csc2_odc1 : 13;
        unsigned int reserved_1 : 3;
    } reg;
} DPU_HDR_CSC2_ODC0_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int hdr_csc2_odc2 : 13;
        unsigned int reserved : 19;
    } reg;
} DPU_HDR_CSC2_ODC2_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int hdr_csc2_p00 : 20;
        unsigned int reserved : 12;
    } reg;
} DPU_HDR_CSC2_P00_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int hdr_csc2_p01 : 20;
        unsigned int reserved : 12;
    } reg;
} DPU_HDR_CSC2_P01_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int hdr_csc2_p02 : 20;
        unsigned int reserved : 12;
    } reg;
} DPU_HDR_CSC2_P02_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int hdr_csc2_p10 : 20;
        unsigned int reserved : 12;
    } reg;
} DPU_HDR_CSC2_P10_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int hdr_csc2_p11 : 20;
        unsigned int reserved : 12;
    } reg;
} DPU_HDR_CSC2_P11_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int hdr_csc2_p12 : 20;
        unsigned int reserved : 12;
    } reg;
} DPU_HDR_CSC2_P12_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int hdr_csc2_p20 : 20;
        unsigned int reserved : 12;
    } reg;
} DPU_HDR_CSC2_P20_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int hdr_csc2_p21 : 20;
        unsigned int reserved : 12;
    } reg;
} DPU_HDR_CSC2_P21_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int hdr_csc2_p22 : 20;
        unsigned int reserved : 12;
    } reg;
} DPU_HDR_CSC2_P22_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int degamma_en : 1;
        unsigned int reserved : 31;
    } reg;
} DPU_HDR_DEGAMMA_EN_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int degamma_mem_ctrl : 4;
        unsigned int reserved : 28;
    } reg;
} DPU_HDR_DEGAMMA_MEM_CTRL_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int degamma_lut_sel : 1;
        unsigned int reserved : 31;
    } reg;
} DPU_HDR_DEGAMMA_LUT_SEL_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int degamma_dbg0 : 32;
    } reg;
} DPU_HDR_DEGAMMA_DBG0_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int degamma_dbg1 : 32;
    } reg;
} DPU_HDR_DEGAMMA_DBG1_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int gmp_en : 1;
        unsigned int reserved : 31;
    } reg;
} DPU_HDR_GMP_EN_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int gmp_mem_ctrl : 4;
        unsigned int reserved : 28;
    } reg;
} DPU_HDR_GMP_MEM_CTRL_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int gmp_lut_sel : 1;
        unsigned int reserved : 31;
    } reg;
} DPU_HDR_GMP_LUT_SEL_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int gmp_dbg_w0 : 32;
    } reg;
} DPU_HDR_GMP_DBG_W0_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int gmp_dbg_r0 : 32;
    } reg;
} DPU_HDR_GMP_DBG_R0_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int gmp_dbg_r1 : 32;
    } reg;
} DPU_HDR_GMP_DBG_R1_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int gmp_dbg_r2 : 32;
    } reg;
} DPU_HDR_GMP_DBG_R2_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int gamma_en : 1;
        unsigned int reserved : 31;
    } reg;
} DPU_HDR_GAMMA_EN_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int gamma_mem_ctrl : 4;
        unsigned int reserved : 28;
    } reg;
} DPU_HDR_GAMMA_MEM_CTRL_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int gamma_lut_sel : 1;
        unsigned int reserved : 31;
    } reg;
} DPU_HDR_GAMMA_LUT_SEL_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int gamma_dbg0 : 32;
    } reg;
} DPU_HDR_GAMMA_DBG0_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int gamma_dbg1 : 32;
    } reg;
} DPU_HDR_GAMMA_DBG1_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int u_hdr_gtm_luma_even_coef : 12;
        unsigned int reserved_0 : 4;
        unsigned int u_hdr_gtm_luma_odd_coef : 12;
        unsigned int reserved_1 : 4;
    } reg;
} DPU_HDR_GTM_LUMA_LUT_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int u_hdr_gtm_chroma_even_coef : 12;
        unsigned int reserved_0 : 4;
        unsigned int u_hdr_gtm_chroma_odd_coef : 12;
        unsigned int reserved_1 : 4;
    } reg;
} DPU_HDR_GTM_CHROMA_LUT_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int u_hdr_gtm_chroma0_even_coef : 14;
        unsigned int reserved_0 : 2;
        unsigned int u_hdr_gtm_chroma0_odd_coef : 14;
        unsigned int reserved_1 : 2;
    } reg;
} DPU_HDR_GTM_CHROMA0_LUT_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int u_hdr_gtm_chroma1_even_coef : 14;
        unsigned int reserved_0 : 2;
        unsigned int u_hdr_gtm_chroma1_odd_coef : 14;
        unsigned int reserved_1 : 2;
    } reg;
} DPU_HDR_GTM_CHROMA1_LUT_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int u_hdr_gtm_chroma2_even_coef : 14;
        unsigned int reserved_0 : 2;
        unsigned int u_hdr_gtm_chroma2_odd_coef : 14;
        unsigned int reserved_1 : 2;
    } reg;
} DPU_HDR_GTM_CHROMA2_LUT_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int u_degamma_r_even_coef : 12;
        unsigned int reserved_0 : 4;
        unsigned int u_degamma_r_odd_coef : 12;
        unsigned int reserved_1 : 4;
    } reg;
} DPU_HDR_DEGAMMA_R_COEF_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int u_degamma_g_even_coef : 12;
        unsigned int reserved_0 : 4;
        unsigned int u_degamma_g_odd_coef : 12;
        unsigned int reserved_1 : 4;
    } reg;
} DPU_HDR_DEGAMMA_G_COEF_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int u_degamma_b_even_coef : 12;
        unsigned int reserved_0 : 4;
        unsigned int u_degamma_b_odd_coef : 12;
        unsigned int reserved_1 : 4;
    } reg;
} DPU_HDR_DEGAMMA_B_COEF_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int u_gamma_r_even_coef : 12;
        unsigned int reserved_0 : 4;
        unsigned int u_gamma_r_odd_coef : 12;
        unsigned int reserved_1 : 4;
    } reg;
} DPU_HDR_GAMMA_R_COEF_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int u_gamma_g_even_coef : 12;
        unsigned int reserved_0 : 4;
        unsigned int u_gamma_g_odd_coef : 12;
        unsigned int reserved_1 : 4;
    } reg;
} DPU_HDR_GAMMA_G_COEF_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int u_gamma_b_even_coef : 12;
        unsigned int reserved_0 : 4;
        unsigned int u_gamma_b_odd_coef : 12;
        unsigned int reserved_1 : 4;
    } reg;
} DPU_HDR_GAMMA_B_COEF_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int u_gmp_coef : 32;
    } reg;
} DPU_HDR_GMP_LUT_GMP_COEF_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int cld_rgb : 24;
        unsigned int reserved : 8;
    } reg;
} DPU_CLD_RGB_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int cld_clk_en : 2;
        unsigned int reserved : 30;
    } reg;
} DPU_CLD_CLK_EN_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int cld_clk_sel : 2;
        unsigned int reserved : 30;
    } reg;
} DPU_CLD_CLK_SEL_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int cld_dbg : 32;
    } reg;
} DPU_CLD_DBG_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int cld_dbg1 : 32;
    } reg;
} DPU_CLD_DBG1_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int cld_dbg2 : 32;
    } reg;
} DPU_CLD_DBG2_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int cld_dbg3 : 32;
    } reg;
} DPU_CLD_DBG3_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int cld_dbg4 : 32;
    } reg;
} DPU_CLD_DBG4_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int cld_dbg5 : 32;
    } reg;
} DPU_CLD_DBG5_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int cld_dbg6 : 32;
    } reg;
} DPU_CLD_DBG6_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int cld_dbg7 : 32;
    } reg;
} DPU_CLD_DBG7_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int cld_en : 1;
        unsigned int reserved : 31;
    } reg;
} DPU_CLD_EN_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int cld_size_hrz : 13;
        unsigned int cld_size_vrt : 13;
        unsigned int reserved : 6;
    } reg;
} DPU_CLD_SIZE_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int reg_ctrl_scene_id : 3;
        unsigned int reserved : 29;
    } reg;
} DPU_CLD_REG_CTRL_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int reg_ctrl_flush_en : 1;
        unsigned int reserved : 31;
    } reg;
} DPU_CLD_REG_CTRL_FLUSH_EN_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int reg_ctrl_dbg : 32;
    } reg;
} DPU_CLD_REG_CTRL_DEBUG_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int reg_ctrl_state : 32;
    } reg;
} DPU_CLD_REG_CTRL_STATE_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int reg_ctrl_layer_id : 5;
        unsigned int reserved : 27;
    } reg;
} DPU_CLD_REG_CTRL_LAYER_ID_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int reg_rd_shadow : 1;
        unsigned int reserved : 31;
    } reg;
} DPU_CLD_REG_RD_SHADOW_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int reg_default : 1;
        unsigned int reserved : 31;
    } reg;
} DPU_CLD_REG_DEFAULT_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int r_lb_debug0 : 32;
    } reg;
} DPU_CLD_R_LB_DEBUG0_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int r_lb_debug1 : 32;
    } reg;
} DPU_CLD_R_LB_DEBUG1_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int r_lb_debug2 : 32;
    } reg;
} DPU_CLD_R_LB_DEBUG2_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int r_lb_debug3 : 32;
    } reg;
} DPU_CLD_R_LB_DEBUG3_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int w_lb_debug0 : 32;
    } reg;
} DPU_CLD_W_LB_DEBUG0_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int w_lb_debug1 : 32;
    } reg;
} DPU_CLD_W_LB_DEBUG1_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int w_lb_debug2 : 32;
    } reg;
} DPU_CLD_W_LB_DEBUG2_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int w_lb_debug3 : 32;
    } reg;
} DPU_CLD_W_LB_DEBUG3_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int rch_ov0_top_clk_sel : 32;
    } reg;
} DPU_RCH_OV_RCH_OV0_TOP_CLK_SEL_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int rch_ov0_top_clk_en : 32;
    } reg;
} DPU_RCH_OV_RCH_OV0_TOP_CLK_EN_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int rch_ov1_top_clk_sel : 32;
    } reg;
} DPU_RCH_OV_RCH_OV1_TOP_CLK_SEL_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int rch_ov1_top_clk_en : 32;
    } reg;
} DPU_RCH_OV_RCH_OV1_TOP_CLK_EN_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int rch_ov2_top_clk_sel : 32;
    } reg;
} DPU_RCH_OV_RCH_OV2_TOP_CLK_SEL_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int rch_ov2_top_clk_en : 32;
    } reg;
} DPU_RCH_OV_RCH_OV2_TOP_CLK_EN_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int rch_ov3_top_clk_sel : 32;
    } reg;
} DPU_RCH_OV_RCH_OV3_TOP_CLK_SEL_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int rch_ov3_top_clk_en : 32;
    } reg;
} DPU_RCH_OV_RCH_OV3_TOP_CLK_EN_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int rch_ov_wrap_clk_sel : 32;
    } reg;
} DPU_RCH_OV_WRAP_CLK_SEL_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int rch_ov_wrap_clk_en : 32;
    } reg;
} DPU_RCH_OV_WRAP_CLK_EN_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int rch_dbg0 : 32;
    } reg;
} DPU_RCH_OV_RCH_DBG0_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int rch_dbg1 : 32;
    } reg;
} DPU_RCH_OV_RCH_DBG1_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int rch_dbg2 : 32;
    } reg;
} DPU_RCH_OV_RCH_DBG2_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int rch_dbg3 : 32;
    } reg;
} DPU_RCH_OV_RCH_DBG3_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int rch_dbg4 : 32;
    } reg;
} DPU_RCH_OV_RCH_DBG4_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int rch_dbg5 : 32;
    } reg;
} DPU_RCH_OV_RCH_DBG5_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int rch_dbg6 : 32;
    } reg;
} DPU_RCH_OV_RCH_DBG6_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int rch_dbg7 : 32;
    } reg;
} DPU_RCH_OV_RCH_DBG7_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int rch_dbg8 : 32;
    } reg;
} DPU_RCH_OV_RCH_DBG8_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int rch_dbg9 : 32;
    } reg;
} DPU_RCH_OV_RCH_DBG9_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int rch_dbg10 : 32;
    } reg;
} DPU_RCH_OV_RCH_DBG10_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int rch_dbg11 : 32;
    } reg;
} DPU_RCH_OV_RCH_DBG11_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int ov_mem_ctrl_0 : 4;
        unsigned int reserved : 28;
    } reg;
} DPU_RCH_OV_OV_MEM_CTRL_0_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int ov_mem_ctrl_1 : 4;
        unsigned int reserved : 28;
    } reg;
} DPU_RCH_OV_OV_MEM_CTRL_1_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int ov_mem_ctrl_2 : 4;
        unsigned int reserved : 28;
    } reg;
} DPU_RCH_OV_OV_MEM_CTRL_2_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int ov_mem_ctrl_3 : 4;
        unsigned int reserved : 28;
    } reg;
} DPU_RCH_OV_OV_MEM_CTRL_3_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int rch_dbg12 : 32;
    } reg;
} DPU_RCH_OV_RCH_DBG12_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int rch_dbg13 : 32;
    } reg;
} DPU_RCH_OV_RCH_DBG13_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int rch_dbg14 : 32;
    } reg;
} DPU_RCH_OV_RCH_DBG14_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int rch_dbg15 : 32;
    } reg;
} DPU_RCH_OV_RCH_DBG15_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int rch_dbg16 : 32;
    } reg;
} DPU_RCH_OV_RCH_DBG16_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int rch_dbg17 : 32;
    } reg;
} DPU_RCH_OV_RCH_DBG17_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int rch_dbg18 : 32;
    } reg;
} DPU_RCH_OV_RCH_DBG18_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int rch_dbg19 : 32;
    } reg;
} DPU_RCH_OV_RCH_DBG19_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int reserved_0 : 1;
        unsigned int reserved_1 : 1;
        unsigned int reserved_2 : 1;
        unsigned int bitext_reg_ini_cfg_en : 1;
        unsigned int reserved_3 : 28;
    } reg;
} DPU_RCH_OV_BITEXT_CTL_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int bitext_reg_ini0_0 : 32;
    } reg;
} DPU_RCH_OV_BITEXT_REG_INI0_0_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int bitext_reg_ini0_1 : 32;
    } reg;
} DPU_RCH_OV_BITEXT_REG_INI0_1_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int bitext_reg_ini0_2 : 32;
    } reg;
} DPU_RCH_OV_BITEXT_REG_INI0_2_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int bitext_reg_ini0_3 : 31;
        unsigned int reserved : 1;
    } reg;
} DPU_RCH_OV_BITEXT_REG_INI0_3_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int bitext_reg_ini1_0 : 32;
    } reg;
} DPU_RCH_OV_BITEXT_REG_INI1_0_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int bitext_reg_ini1_1 : 32;
    } reg;
} DPU_RCH_OV_BITEXT_REG_INI1_1_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int bitext_reg_ini1_2 : 32;
    } reg;
} DPU_RCH_OV_BITEXT_REG_INI1_2_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int bitext_reg_ini1_3 : 31;
        unsigned int reserved : 1;
    } reg;
} DPU_RCH_OV_BITEXT_REG_INI1_3_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int bitext_reg_ini2_0 : 32;
    } reg;
} DPU_RCH_OV_BITEXT_REG_INI2_0_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int bitext_reg_ini2_1 : 32;
    } reg;
} DPU_RCH_OV_BITEXT_REG_INI2_1_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int bitext_reg_ini2_2 : 32;
    } reg;
} DPU_RCH_OV_BITEXT_REG_INI2_2_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int bitext_reg_ini2_3 : 31;
        unsigned int reserved : 1;
    } reg;
} DPU_RCH_OV_BITEXT_REG_INI2_3_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int bitext_power_ctrl_c : 2;
        unsigned int reserved : 30;
    } reg;
} DPU_RCH_OV_BITEXT_POWER_CTRL_C_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int bitext_power_ctrl_shift : 2;
        unsigned int reserved : 30;
    } reg;
} DPU_RCH_OV_BITEXT_POWER_CTRL_SHIFT_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int bitext_filt_00 : 5;
        unsigned int reserved : 27;
    } reg;
} DPU_RCH_OV_BITEXT_FILT_00_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int bitext_filt_01 : 5;
        unsigned int reserved : 27;
    } reg;
} DPU_RCH_OV_BITEXT_FILT_01_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int bitext_filt_02 : 5;
        unsigned int reserved : 27;
    } reg;
} DPU_RCH_OV_BITEXT_FILT_02_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int bitext_filt_10 : 5;
        unsigned int reserved : 27;
    } reg;
} DPU_RCH_OV_BITEXT_FILT_10_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int bitext_filt_11 : 5;
        unsigned int reserved : 27;
    } reg;
} DPU_RCH_OV_BITEXT_FILT_11_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int bitext_filt_12 : 5;
        unsigned int reserved : 27;
    } reg;
} DPU_RCH_OV_BITEXT_FILT_12_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int bitext_filt_20 : 5;
        unsigned int reserved : 27;
    } reg;
} DPU_RCH_OV_BITEXT_FILT_20_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int bitext_filt_21 : 5;
        unsigned int reserved : 27;
    } reg;
} DPU_RCH_OV_BITEXT_FILT_21_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int bitext_filt_22 : 5;
        unsigned int reserved : 27;
    } reg;
} DPU_RCH_OV_BITEXT_FILT_22_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int bitext_thd_r0 : 32;
    } reg;
} DPU_RCH_OV_BITEXT_THD_R0_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int bitext_thd_r1 : 16;
        unsigned int reserved : 16;
    } reg;
} DPU_RCH_OV_BITEXT_THD_R1_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int bitext_thd_g0 : 32;
    } reg;
} DPU_RCH_OV_BITEXT_THD_G0_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int bitext_thd_g1 : 16;
        unsigned int reserved : 16;
    } reg;
} DPU_RCH_OV_BITEXT_THD_G1_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int bitext_thd_b0 : 32;
    } reg;
} DPU_RCH_OV_BITEXT_THD_B0_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int bitext_thd_b1 : 16;
        unsigned int reserved : 16;
    } reg;
} DPU_RCH_OV_BITEXT_THD_B1_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int dfc_glb_alpha3 : 10;
        unsigned int reserved_0 : 6;
        unsigned int dfc_glb_alpha2 : 10;
        unsigned int reserved_1 : 6;
    } reg;
} DPU_RCH_OV_DFC_GLB_ALPHA23_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int bitext_dbg0 : 32;
    } reg;
} DPU_RCH_OV_BITEXT0_DBG0_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int csc_idc0 : 11;
        unsigned int reserved_0: 5;
        unsigned int csc_idc1 : 11;
        unsigned int reserved_1: 5;
    } reg;
} DPU_RCH_OV_CSC_IDC0_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int csc_idc2 : 11;
        unsigned int reserved : 21;
    } reg;
} DPU_RCH_OV_CSC_IDC2_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int csc_odc0 : 11;
        unsigned int reserved_0: 5;
        unsigned int csc_odc1 : 11;
        unsigned int reserved_1: 5;
    } reg;
} DPU_RCH_OV_CSC_ODC0_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int csc_odc2 : 11;
        unsigned int reserved : 21;
    } reg;
} DPU_RCH_OV_CSC_ODC2_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int csc_p00 : 17;
        unsigned int reserved : 15;
    } reg;
} DPU_RCH_OV_CSC_P00_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int csc_p01 : 17;
        unsigned int reserved : 15;
    } reg;
} DPU_RCH_OV_CSC_P01_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int csc_p02 : 17;
        unsigned int reserved : 15;
    } reg;
} DPU_RCH_OV_CSC_P02_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int csc_p10 : 17;
        unsigned int reserved : 15;
    } reg;
} DPU_RCH_OV_CSC_P10_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int csc_p11 : 17;
        unsigned int reserved : 15;
    } reg;
} DPU_RCH_OV_CSC_P11_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int csc_p12 : 17;
        unsigned int reserved : 15;
    } reg;
} DPU_RCH_OV_CSC_P12_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int csc_p20 : 17;
        unsigned int reserved : 15;
    } reg;
} DPU_RCH_OV_CSC_P20_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int csc_p21 : 17;
        unsigned int reserved : 15;
    } reg;
} DPU_RCH_OV_CSC_P21_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int csc_p22 : 17;
        unsigned int reserved : 15;
    } reg;
} DPU_RCH_OV_CSC_P22_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int csc_mprec : 3;
        unsigned int reserved : 29;
    } reg;
} DPU_RCH_OV_CSC_MPREC_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int reserved_0 : 1;
        unsigned int mitm_ppro_bypass : 1;
        unsigned int reserved_1 : 30;
    } reg;
} DPU_RCH_OV_MITM_CTRL_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int mitm_slf_degmm_x1_step : 4;
        unsigned int reserved_0 : 4;
        unsigned int mitm_slf_degmm_x2_step : 4;
        unsigned int reserved_1 : 4;
        unsigned int mitm_slf_degmm_x3_step : 4;
        unsigned int reserved_2 : 4;
        unsigned int mitm_slf_degmm_x4_step : 4;
        unsigned int reserved_3 : 4;
    } reg;
} DPU_RCH_OV_MITM_SLF_DEGAMMA_STEP1_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int mitm_slf_degmm_x5_step : 4;
        unsigned int reserved_0 : 4;
        unsigned int mitm_slf_degmm_x6_step : 4;
        unsigned int reserved_1 : 4;
        unsigned int mitm_slf_degmm_x7_step : 4;
        unsigned int reserved_2 : 4;
        unsigned int mitm_slf_degmm_x8_step : 4;
        unsigned int reserved_3 : 4;
    } reg;
} DPU_RCH_OV_MITM_SLF_DEGAMMA_STEP2_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int mitm_slf_degmm_x1_pos : 10;
        unsigned int reserved_0 : 6;
        unsigned int mitm_slf_degmm_x2_pos : 10;
        unsigned int reserved_1 : 6;
    } reg;
} DPU_RCH_OV_MITM_SLF_DEGAMMA_POS1_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int mitm_slf_degmm_x3_pos : 10;
        unsigned int reserved_0 : 6;
        unsigned int mitm_slf_degmm_x4_pos : 10;
        unsigned int reserved_1 : 6;
    } reg;
} DPU_RCH_OV_MITM_SLF_DEGAMMA_POS2_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int mitm_slf_degmm_x5_pos : 10;
        unsigned int reserved_0 : 6;
        unsigned int mitm_slf_degmm_x6_pos : 10;
        unsigned int reserved_1 : 6;
    } reg;
} DPU_RCH_OV_MITM_SLF_DEGAMMA_POS3_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int mitm_slf_degmm_x7_pos : 10;
        unsigned int reserved_0 : 6;
        unsigned int mitm_slf_degmm_x8_pos : 10;
        unsigned int reserved_1 : 6;
    } reg;
} DPU_RCH_OV_MITM_SLF_DEGAMMA_POS4_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int mitm_slf_degmm_x1_num : 6;
        unsigned int reserved_0 : 2;
        unsigned int mitm_slf_degmm_x2_num : 6;
        unsigned int reserved_1 : 2;
        unsigned int mitm_slf_degmm_x3_num : 6;
        unsigned int reserved_2 : 2;
        unsigned int mitm_slf_degmm_x4_num : 6;
        unsigned int reserved_3 : 2;
    } reg;
} DPU_RCH_OV_MITM_SLF_DEGAMMA_NUM1_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int mitm_slf_degmm_x5_num : 6;
        unsigned int reserved_0 : 2;
        unsigned int mitm_slf_degmm_x6_num : 6;
        unsigned int reserved_1 : 2;
        unsigned int mitm_slf_degmm_x7_num : 6;
        unsigned int reserved_2 : 2;
        unsigned int mitm_slf_degmm_x8_num : 6;
        unsigned int reserved_3 : 2;
    } reg;
} DPU_RCH_OV_MITM_SLF_DEGAMMA_NUM2_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int mitm_slf_gamut_coef00 : 16;
        unsigned int reserved : 16;
    } reg;
} DPU_RCH_OV_MITM_SLF_GAMUT_COEF00_0_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int mitm_slf_gamut_coef01 : 16;
        unsigned int reserved : 16;
    } reg;
} DPU_RCH_OV_MITM_SLF_GAMUT_COEF01_0_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int mitm_slf_gamut_coef02 : 16;
        unsigned int reserved : 16;
    } reg;
} DPU_RCH_OV_MITM_SLF_GAMUT_COEF02_0_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int mitm_slf_gamut_coef10 : 16;
        unsigned int reserved : 16;
    } reg;
} DPU_RCH_OV_MITM_SLF_GAMUT_COEF10_0_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int mitm_slf_gamut_coef11 : 16;
        unsigned int reserved : 16;
    } reg;
} DPU_RCH_OV_MITM_SLF_GAMUT_COEF11_0_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int mitm_slf_gamut_coef12 : 16;
        unsigned int reserved : 16;
    } reg;
} DPU_RCH_OV_MITM_SLF_GAMUT_COEF12_0_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int mitm_slf_gamut_coef20 : 16;
        unsigned int reserved : 16;
    } reg;
} DPU_RCH_OV_MITM_SLF_GAMUT_COEF20_0_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int mitm_slf_gamut_coef21 : 16;
        unsigned int reserved : 16;
    } reg;
} DPU_RCH_OV_MITM_SLF_GAMUT_COEF21_0_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int mitm_slf_gamut_coef22 : 16;
        unsigned int reserved : 16;
    } reg;
} DPU_RCH_OV_MITM_SLF_GAMUT_COEF22_0_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int mitm_slf_gamut_scale : 5;
        unsigned int reserved : 27;
    } reg;
} DPU_RCH_OV_MITM_SLF_GAMUT_SCALE_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int mitm_slf_gamut_clip_max : 16;
        unsigned int reserved : 16;
    } reg;
} DPU_RCH_OV_MITM_SLF_GAMUT_CLIP_MAX_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int mitm_slf_gmm_x1_step : 5;
        unsigned int reserved_0 : 3;
        unsigned int mitm_slf_gmm_x2_step : 5;
        unsigned int reserved_1 : 3;
        unsigned int mitm_slf_gmm_x3_step : 5;
        unsigned int reserved_2 : 3;
        unsigned int mitm_slf_gmm_x4_step : 5;
        unsigned int reserved_3 : 3;
    } reg;
} DPU_RCH_OV_MITM_SLF_GAMMA_STEP1_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int mitm_slf_gmm_x5_step : 5;
        unsigned int reserved_0 : 3;
        unsigned int mitm_slf_gmm_x6_step : 5;
        unsigned int reserved_1 : 3;
        unsigned int mitm_slf_gmm_x7_step : 5;
        unsigned int reserved_2 : 3;
        unsigned int mitm_slf_gmm_x8_step : 5;
        unsigned int reserved_3 : 3;
    } reg;
} DPU_RCH_OV_MITM_SLF_GAMMA_STEP2_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int mitm_slf_gmm_x1_pos : 16;
        unsigned int reserved : 16;
    } reg;
} DPU_RCH_OV_MITM_SLF_GAMMA_POS1_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int mitm_slf_gmm_x2_pos : 16;
        unsigned int reserved : 16;
    } reg;
} DPU_RCH_OV_MITM_SLF_GAMMA_POS2_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int mitm_slf_gmm_x3_pos : 16;
        unsigned int reserved : 16;
    } reg;
} DPU_RCH_OV_MITM_SLF_GAMMA_POS3_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int mitm_slf_gmm_x4_pos : 16;
        unsigned int reserved : 16;
    } reg;
} DPU_RCH_OV_MITM_SLF_GAMMA_POS4_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int mitm_slf_gmm_x5_pos : 16;
        unsigned int reserved : 16;
    } reg;
} DPU_RCH_OV_MITM_SLF_GAMMA_POS5_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int mitm_slf_gmm_x6_pos : 16;
        unsigned int reserved : 16;
    } reg;
} DPU_RCH_OV_MITM_SLF_GAMMA_POS6_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int mitm_slf_gmm_x7_pos : 16;
        unsigned int reserved : 16;
    } reg;
} DPU_RCH_OV_MITM_SLF_GAMMA_POS7_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int mitm_slf_gmm_x8_pos : 16;
        unsigned int reserved : 16;
    } reg;
} DPU_RCH_OV_MITM_SLF_GAMMA_POS8_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int mitm_slf_gmm_x1_num : 6;
        unsigned int reserved_0 : 2;
        unsigned int mitm_slf_gmm_x2_num : 6;
        unsigned int reserved_1 : 2;
        unsigned int mitm_slf_gmm_x3_num : 6;
        unsigned int reserved_2 : 2;
        unsigned int mitm_slf_gmm_x4_num : 6;
        unsigned int reserved_3 : 2;
    } reg;
} DPU_RCH_OV_MITM_SLF_GAMMA_NUM1_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int mitm_slf_gmm_x5_num : 6;
        unsigned int reserved_0 : 2;
        unsigned int mitm_slf_gmm_x6_num : 6;
        unsigned int reserved_1 : 2;
        unsigned int mitm_slf_gmm_x7_num : 6;
        unsigned int reserved_2 : 2;
        unsigned int mitm_slf_gmm_x8_num : 6;
        unsigned int reserved_3 : 2;
    } reg;
} DPU_RCH_OV_MITM_SLF_GAMMA_NUM2_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int mitm_alpha_b : 10;
        unsigned int reserved_0 : 6;
        unsigned int mitm_alpha_a : 10;
        unsigned int reserved_1 : 6;
    } reg;
} DPU_RCH_OV_MITM_ALPHA_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int mitm_degamma_even_coef : 16;
        unsigned int mitm_degamma_odd_coef : 16;
    } reg;
} DPU_RCH_OV_MITM_DEGAMMA_LUT_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int mitm_gamma_even_coef : 10;
        unsigned int reserved_0 : 6;
        unsigned int mitm_gamma_odd_coef : 10;
        unsigned int reserved_1 : 6;
    } reg;
} DPU_RCH_OV_MITM_GAMMA_LUT_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int mitm_dbg0 : 32;
    } reg;
} DPU_RCH_OV_MITM_DBG0_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int mitm_dbg1 : 32;
    } reg;
} DPU_RCH_OV_MITM_DBG1_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int layer_src_locolorkey : 30;
        unsigned int reserved : 2;
    } reg;
} DPU_RCH_OV_LAYER_SRCLOKEY_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int layer_src_hicolorkey : 30;
        unsigned int reserved : 2;
    } reg;
} DPU_RCH_OV_LAYER_SRCHIKEY_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int layer_dst_locolorkey : 30;
        unsigned int reserved : 2;
    } reg;
} DPU_RCH_OV_LAYER_DSTLOKEY_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int layer_dst_hicolorkey : 30;
        unsigned int reserved : 2;
    } reg;
} DPU_RCH_OV_LAYER_DSTHIKEY_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int blayer_bgcolor_en : 1;
        unsigned int reserved_0 : 7;
        unsigned int layer_fix_mode : 1;
        unsigned int layer_dst_pmode : 1;
        unsigned int layer_alpha_offdst : 2;
        unsigned int layer_dst_gmode : 2;
        unsigned int layer_dst_amode : 2;
        unsigned int reserved_1 : 7;
        unsigned int layer_src_mmode : 1;
        unsigned int layer_alpha_smode : 1;
        unsigned int layer_src_pmode : 1;
        unsigned int layer_src_lmode : 1;
        unsigned int layer_alpha_offsrc : 1;
        unsigned int layer_src_gmode : 2;
        unsigned int layer_src_amode : 2;
    } reg;
} DPU_RCH_OV_LAYER_ALPHA_MODE_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int ov0_dbg0 : 32;
    } reg;
} DPU_RCH_OV_OV0_DBG0_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int ov0_dbg1 : 32;
    } reg;
} DPU_RCH_OV_OV0_DBG1_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int ov0_dbg2 : 32;
    } reg;
} DPU_RCH_OV_OV0_DBG2_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int ov0_dbg3 : 32;
    } reg;
} DPU_RCH_OV_OV0_DBG3_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int ov1_dbg0 : 32;
    } reg;
} DPU_RCH_OV_OV1_DBG0_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int ov1_dbg1 : 32;
    } reg;
} DPU_RCH_OV_OV1_DBG1_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int ov1_dbg2 : 32;
    } reg;
} DPU_RCH_OV_OV1_DBG2_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int ov1_dbg3 : 32;
    } reg;
} DPU_RCH_OV_OV1_DBG3_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int ov2_dbg0 : 32;
    } reg;
} DPU_RCH_OV_OV2_DBG0_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int ov2_dbg1 : 32;
    } reg;
} DPU_RCH_OV_OV2_DBG1_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int ov2_dbg2 : 32;
    } reg;
} DPU_RCH_OV_OV2_DBG2_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int ov2_dbg3 : 32;
    } reg;
} DPU_RCH_OV_OV2_DBG3_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int ov3_dbg0 : 32;
    } reg;
} DPU_RCH_OV_OV3_DBG0_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int ov3_dbg1 : 32;
    } reg;
} DPU_RCH_OV_OV3_DBG1_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int ov3_dbg2 : 32;
    } reg;
} DPU_RCH_OV_OV3_DBG2_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int ov3_dbg3 : 32;
    } reg;
} DPU_RCH_OV_OV3_DBG3_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int rxx0_dbg0 : 32;
    } reg;
} DPU_RCH_OV_RXX0_DBG0_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int rxx0_dbg1 : 32;
    } reg;
} DPU_RCH_OV_RXX0_DBG1_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int rxx0_dbg2 : 32;
    } reg;
} DPU_RCH_OV_RXX0_DBG2_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int rxx0_dbg3 : 32;
    } reg;
} DPU_RCH_OV_RXX0_DBG3_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int rxx1_dbg0 : 32;
    } reg;
} DPU_RCH_OV_RXX1_DBG0_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int rxx1_dbg1 : 32;
    } reg;
} DPU_RCH_OV_RXX1_DBG1_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int rxx1_dbg2 : 32;
    } reg;
} DPU_RCH_OV_RXX1_DBG2_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int rxx1_dbg3 : 32;
    } reg;
} DPU_RCH_OV_RXX1_DBG3_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int rxx2_dbg0 : 32;
    } reg;
} DPU_RCH_OV_RXX2_DBG0_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int rxx2_dbg1 : 32;
    } reg;
} DPU_RCH_OV_RXX2_DBG1_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int rxx2_dbg2 : 32;
    } reg;
} DPU_RCH_OV_RXX2_DBG2_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int rxx2_dbg3 : 32;
    } reg;
} DPU_RCH_OV_RXX2_DBG3_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int rxx3_dbg0 : 32;
    } reg;
} DPU_RCH_OV_RXX3_DBG0_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int rxx3_dbg1 : 32;
    } reg;
} DPU_RCH_OV_RXX3_DBG1_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int rxx3_dbg2 : 32;
    } reg;
} DPU_RCH_OV_RXX3_DBG2_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int rxx3_dbg3 : 32;
    } reg;
} DPU_RCH_OV_RXX3_DBG3_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int wxx0_dbg0 : 32;
    } reg;
} DPU_RCH_OV_WXX0_DBG0_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int wxx0_dbg1 : 32;
    } reg;
} DPU_RCH_OV_WXX0_DBG1_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int wxx0_dbg2 : 32;
    } reg;
} DPU_RCH_OV_WXX0_DBG2_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int wxx0_dbg3 : 32;
    } reg;
} DPU_RCH_OV_WXX0_DBG3_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int wxx1_dbg0 : 32;
    } reg;
} DPU_RCH_OV_WXX1_DBG0_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int wxx1_dbg1 : 32;
    } reg;
} DPU_RCH_OV_WXX1_DBG1_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int wxx1_dbg2 : 32;
    } reg;
} DPU_RCH_OV_WXX1_DBG2_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int wxx1_dbg3 : 32;
    } reg;
} DPU_RCH_OV_WXX1_DBG3_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int wxx2_dbg0 : 32;
    } reg;
} DPU_RCH_OV_WXX2_DBG0_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int wxx2_dbg1 : 32;
    } reg;
} DPU_RCH_OV_WXX2_DBG1_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int wxx2_dbg2 : 32;
    } reg;
} DPU_RCH_OV_WXX2_DBG2_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int wxx2_dbg3 : 32;
    } reg;
} DPU_RCH_OV_WXX2_DBG3_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int wxx3_dbg0 : 32;
    } reg;
} DPU_RCH_OV_WXX3_DBG0_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int wxx3_dbg1 : 32;
    } reg;
} DPU_RCH_OV_WXX3_DBG1_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int wxx3_dbg2 : 32;
    } reg;
} DPU_RCH_OV_WXX3_DBG2_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int wxx3_dbg3 : 32;
    } reg;
} DPU_RCH_OV_WXX3_DBG3_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int dpp_ints : 1;
        unsigned int reserved : 31;
    } reg;
} DPU_DPP_INTS_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int dpp_int_msk : 1;
        unsigned int reserved : 31;
    } reg;
} DPU_DPP_INT_MSK_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int ch_clk_sel : 32;
    } reg;
} DPU_DPP_CH_CLK_SEL_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int ch_clk_en : 32;
    } reg;
} DPU_DPP_CH_CLK_EN_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int dpp_clrbar_img_width : 13;
        unsigned int reserved_0 : 3;
        unsigned int dpp_clrbar_img_height : 13;
        unsigned int reserved_1 : 3;
    } reg;
} DPU_DPP_CLRBAR_IMG_SIZE_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int dpp_clrbar_en : 1;
        unsigned int dpp_clrbar_mode : 1;
        unsigned int reserved : 22;
        unsigned int dpp_clrbar_width : 8;
    } reg;
} DPU_DPP_CLRBAR_CTRL_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int dpp_clrbar_1st_clr : 30;
        unsigned int reserved : 2;
    } reg;
} DPU_DPP_CLRBAR_1ST_CLR_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int dpp_clrbar_2nd_clr : 30;
        unsigned int reserved : 2;
    } reg;
} DPU_DPP_CLRBAR_2ND_CLR_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int dpp_clrbar_3rd_clr : 30;
        unsigned int reserved : 2;
    } reg;
} DPU_DPP_CLRBAR_3RD_CLR_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int crc_cfg_en : 1;
        unsigned int reserved : 31;
    } reg;
} DPU_DPP_CRC_CFG_EN_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int crc_frm_cnt : 32;
    } reg;
} DPU_DPP_CRC_FRM_CNT_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int crc_result : 32;
    } reg;
} DPU_DPP_CRC_RESULT_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int ch_rd_shadow : 1;
        unsigned int reserved : 31;
    } reg;
} DPU_DPP_CH_RD_SHADOW_SEL_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int reg_default : 1;
        unsigned int reserved : 31;
    } reg;
} DPU_DPP_REG_DEFAULT_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int ch_in_h_cnt : 13;
        unsigned int reserved_0 : 3;
        unsigned int ch_in_v_cnt : 13;
        unsigned int reserved_1 : 3;
    } reg;
} DPU_DPP_CH_DBG_CNT_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int ch_ro_dbg : 32;
    } reg;
} DPU_DPP_CH_RO_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int ch_dbg1 : 32;
    } reg;
} DPU_DPP_CH_RW1_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int ch_dbg2 : 32;
    } reg;
} DPU_DPP_CH_RW2_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int reg_ctrl_scene_id : 3;
        unsigned int reserved : 29;
    } reg;
} DPU_DPP_REG_CTRL_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int reg_ctrl_flush_en : 1;
        unsigned int reserved : 31;
    } reg;
} DPU_DPP_REG_CTRL_FLUSH_EN_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int reg_ctrl_dbg : 32;
    } reg;
} DPU_DPP_REG_CTRL_DEBUG_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int reg_ctrl_state : 32;
    } reg;
} DPU_DPP_REG_CTRL_STATE_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int reg_ctrl_layer_id : 5;
        unsigned int reserved : 27;
    } reg;
} DPU_DPP_REG_CTRL_LAYER_ID_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int r_lb_debug0 : 32;
    } reg;
} DPU_DPP_R_LB_DEBUG0_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int r_lb_debug1 : 32;
    } reg;
} DPU_DPP_R_LB_DEBUG1_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int r_lb_debug2 : 32;
    } reg;
} DPU_DPP_R_LB_DEBUG2_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int r_lb_debug3 : 32;
    } reg;
} DPU_DPP_R_LB_DEBUG3_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int w_lb_debug0 : 32;
    } reg;
} DPU_DPP_W_LB_DEBUG0_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int w_lb_debug1 : 32;
    } reg;
} DPU_DPP_W_LB_DEBUG1_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int w_lb_debug2 : 32;
    } reg;
} DPU_DPP_W_LB_DEBUG2_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int w_lb_debug3 : 32;
    } reg;
} DPU_DPP_W_LB_DEBUG3_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int padd_en : 1;
        unsigned int reserved : 31;
    } reg;
} DPU_DPP_PADD_EN_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int padd_mode : 1;
        unsigned int reserved : 31;
    } reg;
} DPU_DPP_PADD_MODE_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int padd_data : 30;
        unsigned int reserved : 2;
    } reg;
} DPU_DPP_PADD_DATA_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int padd_left : 13;
        unsigned int reserved : 19;
    } reg;
} DPU_DPP_PADD_LEFT_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int width_aft_padd : 13;
        unsigned int reserved : 19;
    } reg;
} DPU_DPP_WIDTH_AFT_PADD_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int padd_pic_height : 13;
        unsigned int padd_pic_width : 13;
        unsigned int reserved : 6;
    } reg;
} DPU_DPP_PADD_PIC_SIZE_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int rgb_hist_en : 1;
        unsigned int rgb_hist_sel : 3;
        unsigned int reserved : 28;
    } reg;
} DPU_DPP_RGB_HIST_EN_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int r0_hist : 32;
    } reg;
} DPU_DPP_R0_HIST_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int r1_hist : 32;
    } reg;
} DPU_DPP_R1_HIST_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int r2_hist : 32;
    } reg;
} DPU_DPP_R2_HIST_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int r3_hist : 32;
    } reg;
} DPU_DPP_R3_HIST_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int r4_hist : 32;
    } reg;
} DPU_DPP_R4_HIST_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int r5_hist : 32;
    } reg;
} DPU_DPP_R5_HIST_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int r6_hist : 32;
    } reg;
} DPU_DPP_R6_HIST_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int r7_hist : 32;
    } reg;
} DPU_DPP_R7_HIST_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int r8_hist : 32;
    } reg;
} DPU_DPP_R8_HIST_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int r9_hist : 32;
    } reg;
} DPU_DPP_R9_HIST_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int r10_hist : 32;
    } reg;
} DPU_DPP_R10_HIST_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int r11_hist : 32;
    } reg;
} DPU_DPP_R11_HIST_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int r12_hist : 32;
    } reg;
} DPU_DPP_R12_HIST_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int r13_hist : 32;
    } reg;
} DPU_DPP_R13_HIST_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int r14_hist : 32;
    } reg;
} DPU_DPP_R14_HIST_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int r15_hist : 32;
    } reg;
} DPU_DPP_R15_HIST_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int g0_hist : 32;
    } reg;
} DPU_DPP_G0_HIST_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int g1_hist : 32;
    } reg;
} DPU_DPP_G1_HIST_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int g2_hist : 32;
    } reg;
} DPU_DPP_G2_HIST_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int g3_hist : 32;
    } reg;
} DPU_DPP_G3_HIST_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int g4_hist : 32;
    } reg;
} DPU_DPP_G4_HIST_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int g5_hist : 32;
    } reg;
} DPU_DPP_G5_HIST_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int g6_hist : 32;
    } reg;
} DPU_DPP_G6_HIST_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int g7_hist : 32;
    } reg;
} DPU_DPP_G7_HIST_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int g8_hist : 32;
    } reg;
} DPU_DPP_G8_HIST_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int g9_hist : 32;
    } reg;
} DPU_DPP_G9_HIST_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int g10_hist : 32;
    } reg;
} DPU_DPP_G10_HIST_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int g11_hist : 32;
    } reg;
} DPU_DPP_G11_HIST_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int g12_hist : 32;
    } reg;
} DPU_DPP_G12_HIST_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int g13_hist : 32;
    } reg;
} DPU_DPP_G13_HIST_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int g14_hist : 32;
    } reg;
} DPU_DPP_G14_HIST_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int g15_hist : 32;
    } reg;
} DPU_DPP_G15_HIST_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int b0_hist : 32;
    } reg;
} DPU_DPP_B0_HIST_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int b1_hist : 32;
    } reg;
} DPU_DPP_B1_HIST_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int b2_hist : 32;
    } reg;
} DPU_DPP_B2_HIST_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int b3_hist : 32;
    } reg;
} DPU_DPP_B3_HIST_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int b4_hist : 32;
    } reg;
} DPU_DPP_B4_HIST_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int b5_hist : 32;
    } reg;
} DPU_DPP_B5_HIST_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int b6_hist : 32;
    } reg;
} DPU_DPP_B6_HIST_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int b7_hist : 32;
    } reg;
} DPU_DPP_B7_HIST_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int b8_hist : 32;
    } reg;
} DPU_DPP_B8_HIST_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int b9_hist : 32;
    } reg;
} DPU_DPP_B9_HIST_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int b10_hist : 32;
    } reg;
} DPU_DPP_B10_HIST_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int b11_hist : 32;
    } reg;
} DPU_DPP_B11_HIST_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int b12_hist : 32;
    } reg;
} DPU_DPP_B12_HIST_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int b13_hist : 32;
    } reg;
} DPU_DPP_B13_HIST_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int b14_hist : 32;
    } reg;
} DPU_DPP_B14_HIST_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int b15_hist : 32;
    } reg;
} DPU_DPP_B15_HIST_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int spr_mem_ctrl : 4;
        unsigned int reserved : 28;
    } reg;
} DPU_DPP_SPR_MEM_CTRL_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int spr_width : 16;
        unsigned int spr_height : 16;
    } reg;
} DPU_DPP_SPR_SIZE_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int spr_en : 1;
        unsigned int spr_subpxl_layout : 1;
        unsigned int spr_dummymode : 1;
        unsigned int spr_linebuf_1dmode : 1;
        unsigned int spr_larea_en : 1;
        unsigned int spr_borderl_dummymode : 1;
        unsigned int spr_borderr_dummymode : 1;
        unsigned int spr_bordertb_dummymode : 1;
        unsigned int spr_multifac_en : 1;
        unsigned int spr_maskprocess_en : 1;
        unsigned int spr_subdir_en : 1;
        unsigned int spr_borderdet_en : 1;
        unsigned int spr_dirweightmode : 2;
        unsigned int spr_ppro_bypass : 1;
        unsigned int spr_partial_mode : 2;
        unsigned int spr_hpartial_mode : 2;
        unsigned int reserved : 13;
    } reg;
} DPU_DPP_SPR_CTRL_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int spr_coeffsel_even00 : 3;
        unsigned int reserved_0 : 1;
        unsigned int spr_coeffsel_even01 : 3;
        unsigned int reserved_1 : 1;
        unsigned int spr_coeffsel_even10 : 3;
        unsigned int reserved_2 : 1;
        unsigned int spr_coeffsel_even11 : 3;
        unsigned int reserved_3 : 1;
        unsigned int spr_coeffsel_even20 : 3;
        unsigned int reserved_4 : 1;
        unsigned int spr_coeffsel_even21 : 3;
        unsigned int reserved_5 : 9;
    } reg;
} DPU_DPP_SPR_PIX_EVEN_COEF_SEL_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int spr_coeffsel_odd00 : 3;
        unsigned int reserved_0 : 1;
        unsigned int spr_coeffsel_odd01 : 3;
        unsigned int reserved_1 : 1;
        unsigned int spr_coeffsel_odd10 : 3;
        unsigned int reserved_2 : 1;
        unsigned int spr_coeffsel_odd11 : 3;
        unsigned int reserved_3 : 1;
        unsigned int spr_coeffsel_odd20 : 3;
        unsigned int reserved_4 : 1;
        unsigned int spr_coeffsel_odd21 : 3;
        unsigned int reserved_5 : 9;
    } reg;
} DPU_DPP_SPR_PIX_ODD_COEF_SEL_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int spr_pxlsel_even0 : 2;
        unsigned int spr_pxlsel_even1 : 2;
        unsigned int spr_pxlsel_even2 : 2;
        unsigned int spr_pxlsel_odd0 : 2;
        unsigned int spr_pxlsel_odd1 : 2;
        unsigned int spr_pxlsel_odd2 : 2;
        unsigned int reserved : 20;
    } reg;
} DPU_DPP_SPR_PIX_PANEL_ARRANGE_SEL_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int spr_coeff_v0h0_horz_r3 : 6;
        unsigned int reserved_0 : 2;
        unsigned int spr_coeff_v0h0_horz_r2 : 6;
        unsigned int reserved_1 : 2;
        unsigned int spr_coeff_v0h0_horz_r1 : 6;
        unsigned int reserved_2 : 2;
        unsigned int spr_coeff_v0h0_horz_r0 : 6;
        unsigned int reserved_3 : 2;
    } reg;
} DPU_DPP_SPR_COEFF_V0H0_R0_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int spr_coeff_v0h0_vert_r2 : 6;
        unsigned int reserved_0 : 2;
        unsigned int spr_coeff_v0h0_vert_r1 : 6;
        unsigned int reserved_1 : 2;
        unsigned int spr_coeff_v0h0_vert_r0 : 6;
        unsigned int reserved_2 : 2;
        unsigned int spr_coeff_v0h0_horz_r4 : 6;
        unsigned int reserved_3 : 2;
    } reg;
} DPU_DPP_SPR_COEFF_V0H0_R1_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int spr_coeff_v0h1_horz_r3 : 6;
        unsigned int reserved_0 : 2;
        unsigned int spr_coeff_v0h1_horz_r2 : 6;
        unsigned int reserved_1 : 2;
        unsigned int spr_coeff_v0h1_horz_r1 : 6;
        unsigned int reserved_2 : 2;
        unsigned int spr_coeff_v0h1_horz_r0 : 6;
        unsigned int reserved_3 : 2;
    } reg;
} DPU_DPP_SPR_COEFF_V0H1_R0_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int spr_coeff_v0h1_vert_r2 : 6;
        unsigned int reserved_0 : 2;
        unsigned int spr_coeff_v0h1_vert_r1 : 6;
        unsigned int reserved_1 : 2;
        unsigned int spr_coeff_v0h1_vert_r0 : 6;
        unsigned int reserved_2 : 2;
        unsigned int spr_coeff_v0h1_horz_r4 : 6;
        unsigned int reserved_3 : 2;
    } reg;
} DPU_DPP_SPR_COEFF_V0H1_R1_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int spr_coeff_v1h0_horz_r3 : 6;
        unsigned int reserved_0 : 2;
        unsigned int spr_coeff_v1h0_horz_r2 : 6;
        unsigned int reserved_1 : 2;
        unsigned int spr_coeff_v1h0_horz_r1 : 6;
        unsigned int reserved_2 : 2;
        unsigned int spr_coeff_v1h0_horz_r0 : 6;
        unsigned int reserved_3 : 2;
    } reg;
} DPU_DPP_SPR_COEFF_V1H0_R0_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int spr_coeff_v1h0_vert_r2 : 6;
        unsigned int reserved_0 : 2;
        unsigned int spr_coeff_v1h0_vert_r1 : 6;
        unsigned int reserved_1 : 2;
        unsigned int spr_coeff_v1h0_vert_r0 : 6;
        unsigned int reserved_2 : 2;
        unsigned int spr_coeff_v1h0_horz_r4 : 6;
        unsigned int reserved_3 : 2;
    } reg;
} DPU_DPP_SPR_COEFF_V1H0_R1_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int spr_coeff_v1h1_horz_r3 : 6;
        unsigned int reserved_0 : 2;
        unsigned int spr_coeff_v1h1_horz_r2 : 6;
        unsigned int reserved_1 : 2;
        unsigned int spr_coeff_v1h1_horz_r1 : 6;
        unsigned int reserved_2 : 2;
        unsigned int spr_coeff_v1h1_horz_r0 : 6;
        unsigned int reserved_3 : 2;
    } reg;
} DPU_DPP_SPR_COEFF_V1H1_R0_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int spr_coeff_v1h1_vert_r2 : 6;
        unsigned int reserved_0 : 2;
        unsigned int spr_coeff_v1h1_vert_r1 : 6;
        unsigned int reserved_1 : 2;
        unsigned int spr_coeff_v1h1_vert_r0 : 6;
        unsigned int reserved_2 : 2;
        unsigned int spr_coeff_v1h1_horz_r4 : 6;
        unsigned int reserved_3 : 2;
    } reg;
} DPU_DPP_SPR_COEFF_V1H1_R1_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int spr_coeff_v0h0_horz_g3 : 6;
        unsigned int reserved_0 : 2;
        unsigned int spr_coeff_v0h0_horz_g2 : 6;
        unsigned int reserved_1 : 2;
        unsigned int spr_coeff_v0h0_horz_g1 : 6;
        unsigned int reserved_2 : 2;
        unsigned int spr_coeff_v0h0_horz_g0 : 6;
        unsigned int reserved_3 : 2;
    } reg;
} DPU_DPP_SPR_COEFF_V0H0_G0_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int spr_coeff_v0h0_vert_g2 : 6;
        unsigned int reserved_0 : 2;
        unsigned int spr_coeff_v0h0_vert_g1 : 6;
        unsigned int reserved_1 : 2;
        unsigned int spr_coeff_v0h0_vert_g0 : 6;
        unsigned int reserved_2 : 2;
        unsigned int spr_coeff_v0h0_horz_g4 : 6;
        unsigned int reserved_3 : 2;
    } reg;
} DPU_DPP_SPR_COEFF_V0H0_G1_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int spr_coeff_v0h1_horz_g3 : 6;
        unsigned int reserved_0 : 2;
        unsigned int spr_coeff_v0h1_horz_g2 : 6;
        unsigned int reserved_1 : 2;
        unsigned int spr_coeff_v0h1_horz_g1 : 6;
        unsigned int reserved_2 : 2;
        unsigned int spr_coeff_v0h1_horz_g0 : 6;
        unsigned int reserved_3 : 2;
    } reg;
} DPU_DPP_SPR_COEFF_V0H1_G0_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int spr_coeff_v0h1_vert_g2 : 6;
        unsigned int reserved_0 : 2;
        unsigned int spr_coeff_v0h1_vert_g1 : 6;
        unsigned int reserved_1 : 2;
        unsigned int spr_coeff_v0h1_vert_g0 : 6;
        unsigned int reserved_2 : 2;
        unsigned int spr_coeff_v0h1_horz_g4 : 6;
        unsigned int reserved_3 : 2;
    } reg;
} DPU_DPP_SPR_COEFF_V0H1_G1_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int spr_coeff_v1h0_horz_g3 : 6;
        unsigned int reserved_0 : 2;
        unsigned int spr_coeff_v1h0_horz_g2 : 6;
        unsigned int reserved_1 : 2;
        unsigned int spr_coeff_v1h0_horz_g1 : 6;
        unsigned int reserved_2 : 2;
        unsigned int spr_coeff_v1h0_horz_g0 : 6;
        unsigned int reserved_3 : 2;
    } reg;
} DPU_DPP_SPR_COEFF_V1H0_G0_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int spr_coeff_v1h0_vert_g2 : 6;
        unsigned int reserved_0 : 2;
        unsigned int spr_coeff_v1h0_vert_g1 : 6;
        unsigned int reserved_1 : 2;
        unsigned int spr_coeff_v1h0_vert_g0 : 6;
        unsigned int reserved_2 : 2;
        unsigned int spr_coeff_v1h0_horz_g4 : 6;
        unsigned int reserved_3 : 2;
    } reg;
} DPU_DPP_SPR_COEFF_V1H0_G1_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int spr_coeff_v1h1_horz_g3 : 6;
        unsigned int reserved_0 : 2;
        unsigned int spr_coeff_v1h1_horz_g2 : 6;
        unsigned int reserved_1 : 2;
        unsigned int spr_coeff_v1h1_horz_g1 : 6;
        unsigned int reserved_2 : 2;
        unsigned int spr_coeff_v1h1_horz_g0 : 6;
        unsigned int reserved_3 : 2;
    } reg;
} DPU_DPP_SPR_COEFF_V1H1_G0_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int spr_coeff_v1h1_vert_g2 : 6;
        unsigned int reserved_0 : 2;
        unsigned int spr_coeff_v1h1_vert_g1 : 6;
        unsigned int reserved_1 : 2;
        unsigned int spr_coeff_v1h1_vert_g0 : 6;
        unsigned int reserved_2 : 2;
        unsigned int spr_coeff_v1h1_horz_g4 : 6;
        unsigned int reserved_3 : 2;
    } reg;
} DPU_DPP_SPR_COEFF_V1H1_G1_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int spr_coeff_v0h0_horz_b3 : 6;
        unsigned int reserved_0 : 2;
        unsigned int spr_coeff_v0h0_horz_b2 : 6;
        unsigned int reserved_1 : 2;
        unsigned int spr_coeff_v0h0_horz_b1 : 6;
        unsigned int reserved_2 : 2;
        unsigned int spr_coeff_v0h0_horz_b0 : 6;
        unsigned int reserved_3 : 2;
    } reg;
} DPU_DPP_SPR_COEFF_V0H0_B0_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int spr_coeff_v0h0_vert_b2 : 6;
        unsigned int reserved_0 : 2;
        unsigned int spr_coeff_v0h0_vert_b1 : 6;
        unsigned int reserved_1 : 2;
        unsigned int spr_coeff_v0h0_vert_b0 : 6;
        unsigned int reserved_2 : 2;
        unsigned int spr_coeff_v0h0_horz_b4 : 6;
        unsigned int reserved_3 : 2;
    } reg;
} DPU_DPP_SPR_COEFF_V0H0_B1_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int spr_coeff_v0h1_horz_b3 : 6;
        unsigned int reserved_0 : 2;
        unsigned int spr_coeff_v0h1_horz_b2 : 6;
        unsigned int reserved_1 : 2;
        unsigned int spr_coeff_v0h1_horz_b1 : 6;
        unsigned int reserved_2 : 2;
        unsigned int spr_coeff_v0h1_horz_b0 : 6;
        unsigned int reserved_3 : 2;
    } reg;
} DPU_DPP_SPR_COEFF_V0H1_B0_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int spr_coeff_v0h1_vert_b2 : 6;
        unsigned int reserved_0 : 2;
        unsigned int spr_coeff_v0h1_vert_b1 : 6;
        unsigned int reserved_1 : 2;
        unsigned int spr_coeff_v0h1_vert_b0 : 6;
        unsigned int reserved_2 : 2;
        unsigned int spr_coeff_v0h1_horz_b4 : 6;
        unsigned int reserved_3 : 2;
    } reg;
} DPU_DPP_SPR_COEFF_V0H1_B1_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int spr_coeff_v1h0_horz_b3 : 6;
        unsigned int reserved_0 : 2;
        unsigned int spr_coeff_v1h0_horz_b2 : 6;
        unsigned int reserved_1 : 2;
        unsigned int spr_coeff_v1h0_horz_b1 : 6;
        unsigned int reserved_2 : 2;
        unsigned int spr_coeff_v1h0_horz_b0 : 6;
        unsigned int reserved_3 : 2;
    } reg;
} DPU_DPP_SPR_COEFF_V1H0_B0_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int spr_coeff_v1h0_vert_b2 : 6;
        unsigned int reserved_0 : 2;
        unsigned int spr_coeff_v1h0_vert_b1 : 6;
        unsigned int reserved_1 : 2;
        unsigned int spr_coeff_v1h0_vert_b0 : 6;
        unsigned int reserved_2 : 2;
        unsigned int spr_coeff_v1h0_horz_b4 : 6;
        unsigned int reserved_3 : 2;
    } reg;
} DPU_DPP_SPR_COEFF_V1H0_B1_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int spr_coeff_v1h1_horz_b3 : 6;
        unsigned int reserved_0 : 2;
        unsigned int spr_coeff_v1h1_horz_b2 : 6;
        unsigned int reserved_1 : 2;
        unsigned int spr_coeff_v1h1_horz_b1 : 6;
        unsigned int reserved_2 : 2;
        unsigned int spr_coeff_v1h1_horz_b0 : 6;
        unsigned int reserved_3 : 2;
    } reg;
} DPU_DPP_SPR_COEFF_V1H1_B0_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int spr_coeff_v1h1_vert_b2 : 6;
        unsigned int reserved_0 : 2;
        unsigned int spr_coeff_v1h1_vert_b1 : 6;
        unsigned int reserved_1 : 2;
        unsigned int spr_coeff_v1h1_vert_b0 : 6;
        unsigned int reserved_2 : 2;
        unsigned int spr_coeff_v1h1_horz_b4 : 6;
        unsigned int reserved_3 : 2;
    } reg;
} DPU_DPP_SPR_COEFF_V1H1_B1_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int spr_larea_sy : 12;
        unsigned int reserved_0 : 4;
        unsigned int spr_larea_sx : 12;
        unsigned int reserved_1 : 4;
    } reg;
} DPU_DPP_SPR_LAREA_START_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int spr_larea_ey : 12;
        unsigned int reserved_0 : 4;
        unsigned int spr_larea_ex : 12;
        unsigned int reserved_1 : 4;
    } reg;
} DPU_DPP_SPR_LAREA_END_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int spr_larea_y_offset : 8;
        unsigned int reserved_0 : 8;
        unsigned int spr_larea_x_offset : 8;
        unsigned int reserved_1 : 8;
    } reg;
} DPU_DPP_SPR_LAREA_OFFSET_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int spr_larea_gain_b : 8;
        unsigned int spr_larea_gain_g : 8;
        unsigned int spr_larea_gain_r : 8;
        unsigned int reserved : 8;
    } reg;
} DPU_DPP_SPR_LAREA_GAIN_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int spr_larea_border_gainb_r : 8;
        unsigned int spr_larea_border_gaint_r : 8;
        unsigned int spr_larea_border_gainr_r : 8;
        unsigned int spr_larea_border_gainl_r : 8;
    } reg;
} DPU_DPP_SPR_LAREA_BORDER_GAIN_R_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int spr_larea_border_gainb_g : 8;
        unsigned int spr_larea_border_gaint_g : 8;
        unsigned int spr_larea_border_gainr_g : 8;
        unsigned int spr_larea_border_gainl_g : 8;
    } reg;
} DPU_DPP_SPR_LAREA_BORDER_GAIN_G_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int spr_larea_border_gainb_b : 8;
        unsigned int spr_larea_border_gaint_b : 8;
        unsigned int spr_larea_border_gainr_b : 8;
        unsigned int spr_larea_border_gainl_b : 8;
    } reg;
} DPU_DPP_SPR_LAREA_BORDER_GAIN_B_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int spr_borderlr_gainl_r : 8;
        unsigned int spr_borderlr_offsetl_r : 8;
        unsigned int spr_borderlr_gainr_r : 8;
        unsigned int spr_borderlr_offsetr_r : 8;
    } reg;
} DPU_DPP_SPR_R_BORDERLR_REG_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int spr_bordertb_gaint_r : 8;
        unsigned int spr_bordertb_offsett_r : 8;
        unsigned int spr_bordertb_gainb_r : 8;
        unsigned int spr_bordertb_offsetb_r : 8;
    } reg;
} DPU_DPP_SPR_R_BORDERTB_REG_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int spr_borderlr_gainl_g : 8;
        unsigned int spr_borderlr_offsetl_g : 8;
        unsigned int spr_borderlr_gainr_g : 8;
        unsigned int spr_borderlr_offsetr_g : 8;
    } reg;
} DPU_DPP_SPR_G_BORDERLR_REG_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int spr_bordertb_gaint_g : 8;
        unsigned int spr_bordertb_offsett_g : 8;
        unsigned int spr_bordertb_gainb_g : 8;
        unsigned int spr_bordertb_offsetb_g : 8;
    } reg;
} DPU_DPP_SPR_G_BORDERTB_REG_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int spr_borderlr_gainl_b : 8;
        unsigned int spr_borderlr_offsetl_b : 8;
        unsigned int spr_borderlr_gainr_b : 8;
        unsigned int spr_borderlr_offsetr_b : 8;
    } reg;
} DPU_DPP_SPR_B_BORDERLR_REG_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int spr_bordertb_gaint_b : 8;
        unsigned int spr_bordertb_offsett_b : 8;
        unsigned int spr_bordertb_gainb_b : 8;
        unsigned int spr_bordertb_offsetb_b : 8;
    } reg;
} DPU_DPP_SPR_B_BORDERTB_REG_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int spr_finalgain_r : 8;
        unsigned int spr_finaloffset_r : 8;
        unsigned int spr_finalgain_g : 8;
        unsigned int spr_finaloffset_g : 8;
    } reg;
} DPU_DPP_SPR_PIXGAIN_REG_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int spr_finalgain_b : 8;
        unsigned int spr_finaloffset_b : 8;
        unsigned int reserved : 16;
    } reg;
} DPU_DPP_SPR_PIXGAIN_REG1_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int spr_borderl_sx : 12;
        unsigned int spr_borderl_ex : 12;
        unsigned int reserved : 8;
    } reg;
} DPU_DPP_SPR_BORDER_POSITION0_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int spr_borderr_sx : 12;
        unsigned int spr_borderr_ex : 12;
        unsigned int reserved : 8;
    } reg;
} DPU_DPP_SPR_BORDER_POSITION1_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int spr_bordert_sy : 12;
        unsigned int spr_bordert_ey : 12;
        unsigned int reserved : 8;
    } reg;
} DPU_DPP_SPR_BORDER_POSITION2_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int spr_borderb_sy : 12;
        unsigned int spr_borderb_ey : 12;
        unsigned int reserved : 8;
    } reg;
} DPU_DPP_SPR_BORDER_POSITION3_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int spr_subdirblend : 6;
        unsigned int reserved_0 : 10;
        unsigned int spr_gradorthblend : 5;
        unsigned int reserved_1 : 11;
    } reg;
} DPU_DPP_SPR_BLEND_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int spr_weight2bldmax : 6;
        unsigned int reserved_0 : 2;
        unsigned int spr_weight2bldmin : 5;
        unsigned int reserved_1 : 3;
        unsigned int spr_weight2bldcor : 5;
        unsigned int reserved_2 : 3;
        unsigned int spr_weight2bldk : 6;
        unsigned int reserved_3 : 2;
    } reg;
} DPU_DPP_SPR_WEIGHT_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int spr_edgestrbldmax_r : 8;
        unsigned int spr_edgestrbldmin_r : 8;
        unsigned int spr_edgestrbldcor_r : 8;
        unsigned int spr_edgestrbldk_r : 6;
        unsigned int reserved : 2;
    } reg;
} DPU_DPP_SPR_EDGESTR_R_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int spr_edgestrbldmax_g : 8;
        unsigned int spr_edgestrbldmin_g : 8;
        unsigned int spr_edgestrbldcor_g : 8;
        unsigned int spr_edgestrbldk_g : 6;
        unsigned int reserved : 2;
    } reg;
} DPU_DPP_SPR_EDGESTR_G_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int spr_edgestrbldmax_b : 8;
        unsigned int spr_edgestrbldmin_b : 8;
        unsigned int spr_edgestrbldcor_b : 8;
        unsigned int spr_edgestrbldk_b : 6;
        unsigned int reserved : 2;
    } reg;
} DPU_DPP_SPR_EDGESTR_B_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int spr_dirweibldmin_b : 8;
        unsigned int spr_dirweibldmin_g : 8;
        unsigned int spr_dirweibldmin_r : 8;
        unsigned int reserved : 8;
    } reg;
} DPU_DPP_SPR_DIRWEIBLDMIN_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int spr_dirweibldmax_b : 9;
        unsigned int spr_dirweibldmax_g : 9;
        unsigned int spr_dirweibldmax_r : 9;
        unsigned int reserved : 5;
    } reg;
} DPU_DPP_SPR_DIRWEIBLDMAX_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int spr_diffdirgain_r_3 : 7;
        unsigned int reserved_0 : 1;
        unsigned int spr_diffdirgain_r_2 : 7;
        unsigned int reserved_1 : 1;
        unsigned int spr_diffdirgain_r_1 : 7;
        unsigned int reserved_2 : 1;
        unsigned int spr_diffdirgain_r_0 : 7;
        unsigned int reserved_3 : 1;
    } reg;
} DPU_DPP_SPR_DIFFDIRGAIN_R0_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int spr_diffdirgain_r_7 : 7;
        unsigned int reserved_0 : 1;
        unsigned int spr_diffdirgain_r_6 : 7;
        unsigned int reserved_1 : 1;
        unsigned int spr_diffdirgain_r_5 : 7;
        unsigned int reserved_2 : 1;
        unsigned int spr_diffdirgain_r_4 : 7;
        unsigned int reserved_3 : 1;
    } reg;
} DPU_DPP_SPR_DIFFDIRGAIN_R1_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int spr_diffdirgain_g_3 : 7;
        unsigned int reserved_0 : 1;
        unsigned int spr_diffdirgain_g_2 : 7;
        unsigned int reserved_1 : 1;
        unsigned int spr_diffdirgain_g_1 : 7;
        unsigned int reserved_2 : 1;
        unsigned int spr_diffdirgain_g_0 : 7;
        unsigned int reserved_3 : 1;
    } reg;
} DPU_DPP_SPR_DIFFDIRGAIN_G0_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int spr_diffdirgain_g_7 : 7;
        unsigned int reserved_0 : 1;
        unsigned int spr_diffdirgain_g_6 : 7;
        unsigned int reserved_1 : 1;
        unsigned int spr_diffdirgain_g_5 : 7;
        unsigned int reserved_2 : 1;
        unsigned int spr_diffdirgain_g_4 : 7;
        unsigned int reserved_3 : 1;
    } reg;
} DPU_DPP_SPR_DIFFDIRGAIN_G1_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int spr_diffdirgain_b_3 : 7;
        unsigned int reserved_0 : 1;
        unsigned int spr_diffdirgain_b_2 : 7;
        unsigned int reserved_1 : 1;
        unsigned int spr_diffdirgain_b_1 : 7;
        unsigned int reserved_2 : 1;
        unsigned int spr_diffdirgain_b_0 : 7;
        unsigned int reserved_3 : 1;
    } reg;
} DPU_DPP_SPR_DIFFDIRGAIN_B0_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int spr_diffdirgain_b_7 : 7;
        unsigned int reserved_0 : 1;
        unsigned int spr_diffdirgain_b_6 : 7;
        unsigned int reserved_1 : 1;
        unsigned int spr_diffdirgain_b_5 : 7;
        unsigned int reserved_2 : 1;
        unsigned int spr_diffdirgain_b_4 : 7;
        unsigned int reserved_3 : 1;
    } reg;
} DPU_DPP_SPR_DIFFDIRGAIN_B1_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int spr_horzgradrblend : 5;
        unsigned int reserved_0 : 3;
        unsigned int spr_horzgradlblend : 5;
        unsigned int reserved_1 : 19;
    } reg;
} DPU_DPP_SPR_HORZGRADBLEND_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int spr_horzbdbldmax : 6;
        unsigned int reserved_0 : 2;
        unsigned int spr_horzbdbldmin : 5;
        unsigned int reserved_1 : 3;
        unsigned int spr_horzbdbldcor : 5;
        unsigned int reserved_2 : 3;
        unsigned int spr_horzbdbldk : 6;
        unsigned int reserved_3 : 2;
    } reg;
} DPU_DPP_SPR_HORZBDBLD_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int spr_horzbdweightmax : 9;
        unsigned int reserved_0 : 7;
        unsigned int spr_horzbdweightmin : 8;
        unsigned int reserved_1 : 8;
    } reg;
} DPU_DPP_SPR_HORZBDWEIGHT_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int spr_vertbdbldmax : 6;
        unsigned int reserved_0 : 2;
        unsigned int spr_vertbdbldmin : 5;
        unsigned int reserved_1 : 3;
        unsigned int spr_vertbdbldcor : 5;
        unsigned int reserved_2 : 3;
        unsigned int spr_vertbdbldk : 6;
        unsigned int reserved_3 : 2;
    } reg;
} DPU_DPP_SPR_VERTBDBLD_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int spr_vertbdweightmax : 9;
        unsigned int reserved_0 : 7;
        unsigned int spr_vertbdweightmin : 8;
        unsigned int reserved_1 : 8;
    } reg;
} DPU_DPP_SPR_VERTBDWEIGHT_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int spr_horzbd_gainbot_r : 7;
        unsigned int reserved_0 : 1;
        unsigned int spr_horzbd_gaintop_r : 7;
        unsigned int reserved_1 : 1;
        unsigned int spr_vertbd_gainrig_r : 7;
        unsigned int reserved_2 : 1;
        unsigned int spr_vertbd_gainlef_r : 7;
        unsigned int reserved_3 : 1;
    } reg;
} DPU_DPP_SPR_VERTBD_GAIN_R_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int spr_horzbd_gainbot_g : 7;
        unsigned int reserved_0 : 1;
        unsigned int spr_horzbd_gaintop_g : 7;
        unsigned int reserved_1 : 1;
        unsigned int spr_vertbd_gainrig_g : 7;
        unsigned int reserved_2 : 1;
        unsigned int spr_vertbd_gainlef_g : 7;
        unsigned int reserved_3 : 1;
    } reg;
} DPU_DPP_SPR_VERTBD_GAIN_G_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int spr_horzbd_gainbot_b : 7;
        unsigned int reserved_0 : 1;
        unsigned int spr_horzbd_gaintop_b : 7;
        unsigned int reserved_1 : 1;
        unsigned int spr_vertbd_gainrig_b : 7;
        unsigned int reserved_2 : 1;
        unsigned int spr_vertbd_gainlef_b : 7;
        unsigned int reserved_3 : 1;
    } reg;
} DPU_DPP_SPR_VERTBD_GAIN_B_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int spr_dbg : 32;
    } reg;
} DPU_DPP_SPR_DEBUG_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int ptg_en : 1;
        unsigned int ptg_mode : 2;
        unsigned int ptg_ppro_bypass : 1;
        unsigned int reserved : 28;
    } reg;
} DPU_DPP_PTG_CTRL_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int ptg_width : 16;
        unsigned int ptg_height : 16;
    } reg;
} DPU_DPP_PTG_SIZE_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int ptg_line_sy : 12;
        unsigned int reserved_0 : 4;
        unsigned int ptg_line_sx : 12;
        unsigned int reserved_1 : 4;
    } reg;
} DPU_DPP_PTG_POSITION_START_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int ptg_line_ey : 12;
        unsigned int reserved_0 : 4;
        unsigned int ptg_line_ex : 12;
        unsigned int reserved_1 : 4;
    } reg;
} DPU_DPP_PTG_POSITION_END_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int ptg_line_nb : 10;
        unsigned int ptg_line_ng : 10;
        unsigned int ptg_line_nr : 10;
        unsigned int reserved : 2;
    } reg;
} DPU_DPP_PTG_PIX0_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int ptg_line_lb : 10;
        unsigned int ptg_line_lg : 10;
        unsigned int ptg_line_lr : 10;
        unsigned int reserved : 2;
    } reg;
} DPU_DPP_PTG_PIX1_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int ptg_dbg : 32;
    } reg;
} DPU_DPP_PTG_DEBUG_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int degama_en : 1;
        unsigned int reserved : 31;
    } reg;
} DPU_DPP_DEGAMA_EN_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int degama_mem_ctrl : 3;
        unsigned int reserved : 29;
    } reg;
} DPU_DPP_DEGAMA_MEM_CTRL_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int degama_lut_sel : 1;
        unsigned int reserved : 31;
    } reg;
} DPU_DPP_DEGAMA_LUT_SEL_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int degama_dbg0 : 32;
    } reg;
} DPU_DPP_DEGAMA_DBG0_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int degama_dbg1 : 32;
    } reg;
} DPU_DPP_DEGAMA_DBG1_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int roigama0_en : 1;
        unsigned int reserved : 31;
    } reg;
} DPU_DPP_ROIGAMA0_EN_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int roigama0_mem_ctrl : 3;
        unsigned int reserved : 29;
    } reg;
} DPU_DPP_ROIGAMA0_MEM_CTRL_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int roigama0_lut_sel : 1;
        unsigned int reserved : 31;
    } reg;
} DPU_DPP_ROIGAMA0_LUT_SEL_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int roigama0_dbg0 : 32;
    } reg;
} DPU_DPP_ROIGAMA0_DBG0_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int roigama0_dbg1 : 32;
    } reg;
} DPU_DPP_ROIGAMA0_DBG1_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int roigama1_en : 1;
        unsigned int reserved : 31;
    } reg;
} DPU_DPP_ROIGAMA1_EN_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int roigama1_mem_ctrl : 3;
        unsigned int reserved : 29;
    } reg;
} DPU_DPP_ROIGAMA1_MEM_CTRL_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int roigama1_lut_sel : 1;
        unsigned int reserved : 31;
    } reg;
} DPU_DPP_ROIGAMA1_LUT_SEL_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int roigama1_dbg0 : 32;
    } reg;
} DPU_DPP_ROIGAMA1_DBG0_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int roigama1_dbg1 : 32;
    } reg;
} DPU_DPP_ROIGAMA1_DBG1_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int gama_width : 13;
        unsigned int reserved_0 : 3;
        unsigned int gama_height : 13;
        unsigned int reserved_1 : 3;
    } reg;
} DPU_DPP_GAMA_SIZE_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int gama_fullframe_mode : 2;
        unsigned int gama_roi1_mode : 2;
        unsigned int gama_roi0_mode : 2;
        unsigned int reserved : 26;
    } reg;
} DPU_DPP_GAMA_ROI_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int gama_roi0_start_y : 13;
        unsigned int reserved_0 : 3;
        unsigned int gama_roi0_start_x : 13;
        unsigned int reserved_1 : 3;
    } reg;
} DPU_DPP_GAMA_ROI0_START_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int gama_roi0_end_y : 13;
        unsigned int reserved_0 : 3;
        unsigned int gama_roi0_end_x : 13;
        unsigned int reserved_1 : 3;
    } reg;
} DPU_DPP_GAMA_ROI0_END_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int gama_roi1_start_y : 13;
        unsigned int reserved_0 : 3;
        unsigned int gama_roi1_start_x : 13;
        unsigned int reserved_1 : 3;
    } reg;
} DPU_DPP_GAMA_ROI1_START_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int gama_roi1_end_y : 13;
        unsigned int reserved_0 : 3;
        unsigned int gama_roi1_end_x : 13;
        unsigned int reserved_1 : 3;
    } reg;
} DPU_DPP_GAMA_ROI1_END_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int gmp_en : 1;
        unsigned int gmp_bitext_mode : 2;
        unsigned int gmp_roi_mode : 2;
        unsigned int reserved : 27;
    } reg;
} DPU_DPP_GMP_EN_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int gmp_mem_ctrl : 4;
        unsigned int reserved : 28;
    } reg;
} DPU_DPP_GMP_MEM_CTRL_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int gmp_lut_sel : 1;
        unsigned int reserved : 31;
    } reg;
} DPU_DPP_GMP_LUT_SEL_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int gmp_dbg_w0 : 32;
    } reg;
} DPU_DPP_GMP_DBG_W0_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int gmp_dbg_r0 : 32;
    } reg;
} DPU_DPP_GMP_DBG_R0_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int gmp_dbg_r1 : 32;
    } reg;
} DPU_DPP_GMP_DBG_R1_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int gmp_dbg_r2 : 32;
    } reg;
} DPU_DPP_GMP_DBG_R2_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int gmp_width : 13;
        unsigned int reserved_0 : 3;
        unsigned int gmp_height : 13;
        unsigned int reserved_1 : 3;
    } reg;
} DPU_DPP_GMP_SIZE_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int gmp_roi_start_y : 13;
        unsigned int reserved_0 : 3;
        unsigned int gmp_roi_start_x : 13;
        unsigned int reserved_1 : 3;
    } reg;
} DPU_DPP_GMP_ROI_START_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int gmp_roi_end_y : 13;
        unsigned int reserved_0 : 3;
        unsigned int gmp_roi_end_x : 13;
        unsigned int reserved_1 : 3;
    } reg;
} DPU_DPP_GMP_ROI_END_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int dither_mode : 2;
        unsigned int dither_sel : 3;
        unsigned int dither_auto_sel_en : 1;
        unsigned int reserved : 26;
    } reg;
} DPU_DPP_DITHER_CTL1_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int dither_en : 1;
        unsigned int dither_hifreq_noise_mode : 2;
        unsigned int dither_rgb_shift_mode : 1;
        unsigned int dither_uniform_en : 1;
        unsigned int reserved : 27;
    } reg;
} DPU_DPP_DITHER_CTL0_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int dither_tri_thd12_b : 12;
        unsigned int dither_tri_thd12_g : 12;
        unsigned int reserved : 8;
    } reg;
} DPU_DPP_DITHER_TRI_THD12_0_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int dither_tri_thd12_r : 12;
        unsigned int reserved : 20;
    } reg;
} DPU_DPP_DITHER_TRI_THD12_1_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int dither_tri_thd10_b : 10;
        unsigned int dither_tri_thd10_g : 10;
        unsigned int dither_tri_thd10_r : 10;
        unsigned int reserved : 2;
    } reg;
} DPU_DPP_DITHER_TRI_THD10_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int dither_tri_thd12_uni_b : 12;
        unsigned int dither_tri_thd12_uni_g : 12;
        unsigned int reserved : 8;
    } reg;
} DPU_DPP_DITHER_TRI_THD12_UNI_0_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int dither_tri_thd12_uni_r : 12;
        unsigned int reserved : 20;
    } reg;
} DPU_DPP_DITHER_TRI_THD12_UNI_1_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int dither_tri_thd10_uni_b : 10;
        unsigned int dither_tri_thd10_uni_g : 10;
        unsigned int dither_tri_thd10_uni_r : 10;
        unsigned int reserved : 2;
    } reg;
} DPU_DPP_DITHER_TRI_THD10_UNI_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int bayer_rgb_en : 1;
        unsigned int reserved_0 : 2;
        unsigned int bayer_alpha_handle_mode : 2;
        unsigned int bayer_alpha_shift_mode : 2;
        unsigned int reserved_1 : 9;
        unsigned int bayer_offset : 12;
        unsigned int reserved_2 : 4;
    } reg;
} DPU_DPP_BAYER_CTL_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int bayer_alpha_thd : 30;
        unsigned int reserved : 2;
    } reg;
} DPU_DPP_BAYER_ALPHA_THD_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int bayer_matrix_part1 : 32;
    } reg;
} DPU_DPP_BAYER_MATRIX_PART1_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int bayer_matrix_part0 : 32;
    } reg;
} DPU_DPP_BAYER_MATRIX_PART0_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int hifreq_reg_ini_cfg_en : 1;
        unsigned int reserved : 31;
    } reg;
} DPU_DPP_HIFREQ_REG_INI_CFG_EN_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int hifreq_reg_ini0_0 : 32;
    } reg;
} DPU_DPP_HIFREQ_REG_INI0_0_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int hifreq_reg_ini0_1 : 32;
    } reg;
} DPU_DPP_HIFREQ_REG_INI0_1_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int hifreq_reg_ini0_2 : 32;
    } reg;
} DPU_DPP_HIFREQ_REG_INI0_2_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int hifreq_reg_ini0_3 : 31;
        unsigned int reserved : 1;
    } reg;
} DPU_DPP_HIFREQ_REG_INI0_3_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int hifreq_reg_ini1_0 : 32;
    } reg;
} DPU_DPP_HIFREQ_REG_INI1_0_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int hifreq_reg_ini1_1 : 32;
    } reg;
} DPU_DPP_HIFREQ_REG_INI1_1_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int hifreq_reg_ini1_2 : 32;
    } reg;
} DPU_DPP_HIFREQ_REG_INI1_2_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int hifreq_reg_ini1_3 : 31;
        unsigned int reserved : 1;
    } reg;
} DPU_DPP_HIFREQ_REG_INI1_3_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int hifreq_reg_ini2_0 : 32;
    } reg;
} DPU_DPP_HIFREQ_REG_INI2_0_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int hifreq_reg_ini2_1 : 32;
    } reg;
} DPU_DPP_HIFREQ_REG_INI2_1_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int hifreq_reg_ini2_2 : 32;
    } reg;
} DPU_DPP_HIFREQ_REG_INI2_2_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int hifreq_reg_ini2_3 : 31;
        unsigned int reserved : 1;
    } reg;
} DPU_DPP_HIFREQ_REG_INI2_3_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int hifreq_power_ctrl_shift : 2;
        unsigned int hifreq_power_ctrl_c : 2;
        unsigned int reserved : 28;
    } reg;
} DPU_DPP_HIFREQ_POWER_CTRL_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int hifreq_filt_00 : 5;
        unsigned int hifreq_filt_01 : 5;
        unsigned int hifreq_filt_02 : 5;
        unsigned int reserved : 17;
    } reg;
} DPU_DPP_HIFREQ_FILT_0_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int hifreq_filt_10 : 5;
        unsigned int hifreq_filt_11 : 5;
        unsigned int hifreq_filt_12 : 5;
        unsigned int reserved : 17;
    } reg;
} DPU_DPP_HIFREQ_FILT_1_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int hifreq_filt_20 : 5;
        unsigned int hifreq_filt_21 : 5;
        unsigned int hifreq_filt_22 : 5;
        unsigned int reserved : 17;
    } reg;
} DPU_DPP_HIFREQ_FILT_2_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int hifreq_thd_r0 : 32;
    } reg;
} DPU_DPP_HIFREQ_THD_R0_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int hifreq_thd_r1 : 16;
        unsigned int reserved : 16;
    } reg;
} DPU_DPP_HIFREQ_THD_R1_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int hifreq_thd_g0 : 32;
    } reg;
} DPU_DPP_HIFREQ_THD_G0_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int hifreq_thd_g1 : 16;
        unsigned int reserved : 16;
    } reg;
} DPU_DPP_HIFREQ_THD_G1_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int hifreq_thd_b0 : 32;
    } reg;
} DPU_DPP_HIFREQ_THD_B0_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int hifreq_thd_b1 : 16;
        unsigned int reserved : 16;
    } reg;
} DPU_DPP_HIFREQ_THD_B1_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int hifreq_dbg0 : 32;
    } reg;
} DPU_DPP_HIFREQ_DBG0_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int hifreq_dbg1 : 32;
    } reg;
} DPU_DPP_HIFREQ_DBG1_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int hifreq_dbg2 : 32;
    } reg;
} DPU_DPP_HIFREQ_DBG2_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int errdiff_en : 1;
        unsigned int reserved_0 : 2;
        unsigned int reserved_1 : 29;
    } reg;
} DPU_DPP_ERRDIFF_CTL_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int errdiff_weight : 28;
        unsigned int reserved : 4;
    } reg;
} DPU_DPP_ERRDIFF_WEIGHT_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int frc_en : 1;
        unsigned int reserved_0 : 2;
        unsigned int frc_findex_mode : 1;
        unsigned int reserved_1 : 28;
    } reg;
} DPU_DPP_DITHER_FRC_CTL_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int frc_01_part1 : 32;
    } reg;
} DPU_DPP_FRC_01_PART1_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int frc_01_part0 : 32;
    } reg;
} DPU_DPP_FRC_01_PART0_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int frc_10_part1 : 32;
    } reg;
} DPU_DPP_FRC_10_PART1_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int frc_10_part0 : 32;
    } reg;
} DPU_DPP_FRC_10_PART0_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int frc_11_part1 : 32;
    } reg;
} DPU_DPP_FRC_11_PART1_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int frc_11_part0 : 32;
    } reg;
} DPU_DPP_FRC_11_PART0_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int dither_mem_ctrl : 4;
        unsigned int reserved : 28;
    } reg;
} DPU_DPP_DITHER_MEM_CTRL_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int dither_dbg0 : 32;
    } reg;
} DPU_DPP_DITHER_DBG0_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int dither_dbg1 : 32;
    } reg;
} DPU_DPP_DITHER_DBG1_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int dither_dbg2 : 32;
    } reg;
} DPU_DPP_DITHER_DBG2_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int frc_findex_clr_en : 1;
        unsigned int reserved : 31;
    } reg;
} DPU_DPP_DITHER_CTRL2_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int dpp_dither_img_width : 13;
        unsigned int reserved_0 : 3;
        unsigned int dpp_dither_img_height : 13;
        unsigned int reserved_1 : 3;
    } reg;
} DPU_DPP_DITHER_IMG_SIZE_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int dpp_clip_top : 6;
        unsigned int reserved : 26;
    } reg;
} DPU_DPP_CLIP_TOP_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int dpp_clip_bottom : 6;
        unsigned int reserved : 26;
    } reg;
} DPU_DPP_CLIP_BOTTOM_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int dpp_clip_left : 6;
        unsigned int reserved : 26;
    } reg;
} DPU_DPP_CLIP_LEFT_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int dpp_clip_right : 6;
        unsigned int reserved : 26;
    } reg;
} DPU_DPP_CLIP_RIGHT_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int dpp_clip_en : 1;
        unsigned int reserved : 31;
    } reg;
} DPU_DPP_CLIP_EN_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int dpp_clip_dbg : 32;
    } reg;
} DPU_DPP_CLIP_DBG_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int dpp_clip_img_width : 13;
        unsigned int reserved_0 : 3;
        unsigned int dpp_clip_img_height : 13;
        unsigned int reserved_1 : 3;
    } reg;
} DPU_DPP_CLIP_IMG_SIZE_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int nlxcc_coef_00 : 16;
        unsigned int reserved : 16;
    } reg;
} DPU_DPP_NLXCC_COEF_00_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int nlxcc_coef_01 : 17;
        unsigned int reserved : 15;
    } reg;
} DPU_DPP_NLXCC_COEF_01_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int nlxcc_coef_02 : 17;
        unsigned int reserved : 15;
    } reg;
} DPU_DPP_NLXCC_COEF_02_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int nlxcc_coef_03 : 17;
        unsigned int reserved : 15;
    } reg;
} DPU_DPP_NLXCC_COEF_03_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int nlxcc_coef_10 : 16;
        unsigned int reserved : 16;
    } reg;
} DPU_DPP_NLXCC_COEF_10_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int nlxcc_coef_11 : 17;
        unsigned int reserved : 15;
    } reg;
} DPU_DPP_NLXCC_COEF_11_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int nlxcc_coef_12 : 17;
        unsigned int reserved : 15;
    } reg;
} DPU_DPP_NLXCC_COEF_12_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int nlxcc_coef_13 : 17;
        unsigned int reserved : 15;
    } reg;
} DPU_DPP_NLXCC_COEF_13_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int nlxcc_coef_20 : 16;
        unsigned int reserved : 16;
    } reg;
} DPU_DPP_NLXCC_COEF_20_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int nlxcc_coef_21 : 17;
        unsigned int reserved : 15;
    } reg;
} DPU_DPP_NLXCC_COEF_21_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int nlxcc_coef_22 : 17;
        unsigned int reserved : 15;
    } reg;
} DPU_DPP_NLXCC_COEF_22_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int nlxcc_coef_23 : 17;
        unsigned int reserved : 15;
    } reg;
} DPU_DPP_NLXCC_COEF_23_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int nlxcc_en : 1;
        unsigned int nlxcc_roi_mode : 2;
        unsigned int reserved : 29;
    } reg;
} DPU_DPP_NLXCC_EN_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int nlxcc_dual_pxl_en : 1;
        unsigned int reserved : 31;
    } reg;
} DPU_DPP_NLXCC_DUAL_PXL_EN_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int nlxcc_debug : 1;
        unsigned int reserved : 31;
    } reg;
} DPU_DPP_NLXCC_DEBUG_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int nlxcc_width : 13;
        unsigned int reserved_0 : 3;
        unsigned int nlxcc_height : 13;
        unsigned int reserved_1 : 3;
    } reg;
} DPU_DPP_NLXCC_SIZE_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int nlxcc_roi_start_y : 13;
        unsigned int reserved_0 : 3;
        unsigned int nlxcc_roi_start_x : 13;
        unsigned int reserved_1 : 3;
    } reg;
} DPU_DPP_NLXCC_ROI_START_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int nlxcc_roi_end_y : 13;
        unsigned int reserved_0 : 3;
        unsigned int nlxcc_roi_end_x : 13;
        unsigned int reserved_1 : 3;
    } reg;
} DPU_DPP_NLXCC_ROI_END_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int xcc_coef_00 : 16;
        unsigned int reserved : 16;
    } reg;
} DPU_DPP_XCC_COEF_00_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int xcc_coef_01 : 17;
        unsigned int reserved : 15;
    } reg;
} DPU_DPP_XCC_COEF_01_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int xcc_coef_02 : 17;
        unsigned int reserved : 15;
    } reg;
} DPU_DPP_XCC_COEF_02_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int xcc_coef_03 : 17;
        unsigned int reserved : 15;
    } reg;
} DPU_DPP_XCC_COEF_03_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int xcc_coef_10 : 16;
        unsigned int reserved : 16;
    } reg;
} DPU_DPP_XCC_COEF_10_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int xcc_coef_11 : 17;
        unsigned int reserved : 15;
    } reg;
} DPU_DPP_XCC_COEF_11_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int xcc_coef_12 : 17;
        unsigned int reserved : 15;
    } reg;
} DPU_DPP_XCC_COEF_12_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int xcc_coef_13 : 17;
        unsigned int reserved : 15;
    } reg;
} DPU_DPP_XCC_COEF_13_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int xcc_coef_20 : 16;
        unsigned int reserved : 16;
    } reg;
} DPU_DPP_XCC_COEF_20_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int xcc_coef_21 : 17;
        unsigned int reserved : 15;
    } reg;
} DPU_DPP_XCC_COEF_21_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int xcc_coef_22 : 17;
        unsigned int reserved : 15;
    } reg;
} DPU_DPP_XCC_COEF_22_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int xcc_coef_23 : 17;
        unsigned int reserved : 15;
    } reg;
} DPU_DPP_XCC_COEF_23_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int xcc_en : 1;
        unsigned int xcc_roi_mode : 2;
        unsigned int reserved : 29;
    } reg;
} DPU_DPP_XCC_EN_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int xcc_width : 13;
        unsigned int reserved_0 : 3;
        unsigned int xcc_height : 13;
        unsigned int reserved_1 : 3;
    } reg;
} DPU_DPP_XCC_SIZE_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int xcc_roi_start_y : 13;
        unsigned int reserved_0 : 3;
        unsigned int xcc_roi_start_x : 13;
        unsigned int reserved_1 : 3;
    } reg;
} DPU_DPP_XCC_ROI_START_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int xcc_roi_end_y : 13;
        unsigned int reserved_0 : 3;
        unsigned int xcc_roi_end_x : 13;
        unsigned int reserved_1 : 3;
    } reg;
} DPU_DPP_XCC_ROI_END_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int reg_ave_min_th : 10;
        unsigned int reserved_0 : 6;
        unsigned int reg_local_highlight_en : 1;
        unsigned int reserved_1 : 15;
    } reg;
} DPU_DPP_HIGHLIGHT_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int reg_hl_hue_gain_1 : 9;
        unsigned int reserved_0 : 7;
        unsigned int reg_hl_hue_gain_0 : 9;
        unsigned int reserved_1 : 7;
    } reg;
} DPU_DPP_HL_HUE_GAIN_0_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int reg_hl_hue_gain_3 : 9;
        unsigned int reserved_0 : 7;
        unsigned int reg_hl_hue_gain_2 : 9;
        unsigned int reserved_1 : 7;
    } reg;
} DPU_DPP_HL_HUE_GAIN_1_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int reg_hl_hue_gain_5 : 9;
        unsigned int reserved_0 : 7;
        unsigned int reg_hl_hue_gain_4 : 9;
        unsigned int reserved_1 : 7;
    } reg;
} DPU_DPP_HL_HUE_GAIN_2_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int reg_hl_hue_gain_7 : 9;
        unsigned int reserved_0 : 7;
        unsigned int reg_hl_hue_gain_6 : 9;
        unsigned int reserved_1 : 7;
    } reg;
} DPU_DPP_HL_HUE_GAIN_3_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int reg_hl_hue_gain_9 : 9;
        unsigned int reserved_0 : 7;
        unsigned int reg_hl_hue_gain_8 : 9;
        unsigned int reserved_1 : 7;
    } reg;
} DPU_DPP_HL_HUE_GAIN_4_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int reg_hl_hue_gain_11 : 9;
        unsigned int reserved_0 : 7;
        unsigned int reg_hl_hue_gain_10 : 9;
        unsigned int reserved_1 : 7;
    } reg;
} DPU_DPP_HL_HUE_GAIN_5_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int reg_hl_sat_gain_min : 9;
        unsigned int reserved_0 : 7;
        unsigned int reg_hl_sat_bit : 4;
        unsigned int reserved_1 : 4;
        unsigned int reg_hl_sat_minth : 8;
    } reg;
} DPU_DPP_HL_SAT_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int reg_hl_yout_maxth_1 : 10;
        unsigned int reserved_0 : 6;
        unsigned int reg_hl_yout_maxth_0 : 10;
        unsigned int reserved_1 : 6;
    } reg;
} DPU_DPP_HL_YOUT_MAXTH_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int reg_hl_yout_gain : 10;
        unsigned int reserved_0 : 6;
        unsigned int reg_hl_yout_bit : 4;
        unsigned int reserved_1 : 12;
    } reg;
} DPU_DPP_HL_YOUT_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int reg_hl_diff_ave_slope_bit : 4;
        unsigned int reserved_0 : 12;
        unsigned int reg_hl_diff_ave_minth : 10;
        unsigned int reserved_1 : 6;
    } reg;
} DPU_DPP_HL_DIFF_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int reg_skin_minth : 8;
        unsigned int reserved_0 : 8;
        unsigned int reg_skin_count_en : 1;
        unsigned int reserved_1 : 15;
    } reg;
} DPU_DPP_SKIN_COUNT_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int reg_lre_improve_sat_en : 1;
        unsigned int reserved : 31;
    } reg;
} DPU_DPP_LRE_SAT_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int reg_horizontal_init_sub : 10;
        unsigned int reserved_0 : 6;
        unsigned int reg_horizontal_init_int : 3;
        unsigned int reserved_1 : 5;
        unsigned int reg_local_refresh_en : 1;
        unsigned int reserved_2 : 7;
    } reg;
} DPU_DPP_LOCAL_REFRESH_H_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int reg_vertical_init_sub : 10;
        unsigned int reserved_0 : 6;
        unsigned int reg_vertical_init_int : 3;
        unsigned int reserved_1 : 13;
    } reg;
} DPU_DPP_LOCAL_REFRESH_V_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int reg_roi_global_hist_en : 1;
        unsigned int reserved : 31;
    } reg;
} DPU_DPP_GLOBAL_HIST_EN_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int reg_global_hist_start_y : 13;
        unsigned int reserved_0 : 3;
        unsigned int reg_global_hist_start_x : 13;
        unsigned int reserved_1 : 3;
    } reg;
} DPU_DPP_GLOBAL_HIST_START_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int reg_global_hist_height : 13;
        unsigned int reserved_0 : 3;
        unsigned int reg_global_hist_width : 13;
        unsigned int reserved_1 : 3;
    } reg;
} DPU_DPP_GLOBAL_HIST_SIZE_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int reg_screen_mode : 3;
        unsigned int reserved : 29;
    } reg;
} DPU_DPP_SCREEN_MODE_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int reg_separator_block : 3;
        unsigned int reserved_0 : 13;
        unsigned int reg_separator : 13;
        unsigned int reserved_1 : 3;
    } reg;
} DPU_DPP_SEPARATOR_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int reg_weight_max : 9;
        unsigned int reserved_0 : 7;
        unsigned int reg_weight_min : 9;
        unsigned int reserved_1 : 7;
    } reg;
} DPU_DPP_WEIGHT_MIN_MAX_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int reg_fna_en : 1;
        unsigned int reserved : 31;
    } reg;
} DPU_DPP_FNA_EN_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int dbg_hiace_0_i : 1;
        unsigned int dbg_hiace_1_i : 1;
        unsigned int dbg_hiace_2_i : 1;
        unsigned int dbg_hiace_3_i : 1;
        unsigned int dbg_hiace_4_i : 1;
        unsigned int reserved : 27;
    } reg;
} DPU_DPP_DBG_HIACE_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int spr_gama_en : 1;
        unsigned int reserved : 31;
    } reg;
} DPU_DPP_SPR_GAMA_EN_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int spr_gama_mem_ctrl : 3;
        unsigned int reserved : 29;
    } reg;
} DPU_DPP_SPR_GAMA_MEM_CTRL_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int spr_gama_lut_sel : 1;
        unsigned int reserved : 31;
    } reg;
} DPU_DPP_SPR_GAMA_LUT_SEL_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int spr_gama_dbg0 : 32;
    } reg;
} DPU_DPP_SPR_GAMA_DBG0_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int spr_gama_dbg1 : 32;
    } reg;
} DPU_DPP_SPR_GAMA_DBG1_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int gama_mode : 1;
        unsigned int gama_shift_en : 1;
        unsigned int reserved : 30;
    } reg;
} DPU_DPP_SPR_GAMA_SHIFTEN_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int alsc_degamma_en : 1;
        unsigned int reserved : 31;
    } reg;
} DPU_DPP_ALSC_DEGAMMA_EN_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int alsc_degamma_mem_ctrl : 3;
        unsigned int reserved : 29;
    } reg;
} DPU_DPP_ALSC_DEGAMMA_MEM_CTRL_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int alsc_degamma_lut_sel : 1;
        unsigned int reserved : 31;
    } reg;
} DPU_DPP_ALSC_DEGAMMA_LUT_SEL_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int alsc_degamma_dbg0 : 32;
    } reg;
} DPU_DPP_ALSC_DEGAMMA_DBG0_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int alsc_degamma_dbg1 : 32;
    } reg;
} DPU_DPP_ALSC_DEGAMMA_DBG1_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int alsc_en : 1;
        unsigned int reserved : 31;
    } reg;
} DPU_DPP_ALSC_EN_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int alsc_ppro_bypass : 1;
        unsigned int reserved : 31;
    } reg;
} DPU_DPP_ALSC_PPRO_BYPASS_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int sensor_channel : 1;
        unsigned int reserved : 31;
    } reg;
} DPU_DPP_SENSOR_CHANNEL_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int pic_height : 13;
        unsigned int pic_width : 13;
        unsigned int reserved : 6;
    } reg;
} DPU_DPP_PIC_SIZE_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int alsc_addr_x : 13;
        unsigned int alsc_addr_y : 13;
        unsigned int reserved : 6;
    } reg;
} DPU_DPP_ALSC_ADDR_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int alsc_size_x : 8;
        unsigned int alsc_size_y : 8;
        unsigned int reserved : 16;
    } reg;
} DPU_DPP_ALSC_SIZE_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int noise1 : 32;
    } reg;
} DPU_DPP_NOISE1_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int noise2 : 32;
    } reg;
} DPU_DPP_NOISE2_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int noise3 : 32;
    } reg;
} DPU_DPP_NOISE3_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int noise4 : 32;
    } reg;
} DPU_DPP_NOISE4_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int coef_r2r : 32;
    } reg;
} DPU_DPP_COEF_R2R_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int coef_r2g : 32;
    } reg;
} DPU_DPP_COEF_R2G_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int coef_r2b : 32;
    } reg;
} DPU_DPP_COEF_R2B_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int coef_r2c : 32;
    } reg;
} DPU_DPP_COEF_R2C_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int coef_g2r : 32;
    } reg;
} DPU_DPP_COEF_G2R_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int coef_g2g : 32;
    } reg;
} DPU_DPP_COEF_G2G_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int coef_g2b : 32;
    } reg;
} DPU_DPP_COEF_G2B_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int coef_g2c : 32;
    } reg;
} DPU_DPP_COEF_G2C_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int coef_b2r : 32;
    } reg;
} DPU_DPP_COEF_B2R_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int coef_b2g : 32;
    } reg;
} DPU_DPP_COEF_B2G_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int coef_b2b : 32;
    } reg;
} DPU_DPP_COEF_B2B_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int coef_b2c : 32;
    } reg;
} DPU_DPP_COEF_B2C_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int timestamp_l : 32;
    } reg;
} DPU_DPP_TIMESTAMP_L_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int timestamp_h : 32;
    } reg;
} DPU_DPP_TIMESTAMP_H_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int alsc_dbg0 : 32;
    } reg;
} DPU_DPP_ALSC_DBG0_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int alsc_dbg1 : 32;
    } reg;
} DPU_DPP_ALSC_DBG1_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int addr_block_ly : 8;
        unsigned int addr_block_lx : 8;
        unsigned int addr_block_ry : 8;
        unsigned int addr_block_rx : 8;
    } reg;
} DPU_DPP_ADDR_BLOCK_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int ave_block : 16;
        unsigned int reserved : 16;
    } reg;
} DPU_DPP_AVE_BLOCK_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int coef_block_r : 32;
    } reg;
} DPU_DPP_COEF_BLOCK_R_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int coef_block_g : 32;
    } reg;
} DPU_DPP_COEF_BLOCK_G_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int coef_block_b : 32;
    } reg;
} DPU_DPP_COEF_BLOCK_B_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int coef_block_c : 32;
    } reg;
} DPU_DPP_COEF_BLOCK_C_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int u_degama_r_even_coef : 13;
        unsigned int reserved_0 : 3;
        unsigned int u_degama_r_odd_coef : 13;
        unsigned int reserved_1 : 3;
    } reg;
} DPU_DPP_U_DEGAMA_R_COEF_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int u_degama_g_even_coef : 13;
        unsigned int reserved_0 : 3;
        unsigned int u_degama_g_odd_coef : 13;
        unsigned int reserved_1 : 3;
    } reg;
} DPU_DPP_U_DEGAMA_G_COEF_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int u_degama_b_even_coef : 13;
        unsigned int reserved_0 : 3;
        unsigned int u_degama_b_odd_coef : 13;
        unsigned int reserved_1 : 3;
    } reg;
} DPU_DPP_U_DEGAMA_B_COEF_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int u_roigama0_r_even_coef : 13;
        unsigned int reserved_0 : 3;
        unsigned int u_roigama0_r_odd_coef : 13;
        unsigned int reserved_1 : 3;
    } reg;
} DPU_DPP_U_ROIGAMA0_R_COEF_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int u_roigama0_g_even_coef : 13;
        unsigned int reserved_0 : 3;
        unsigned int u_roigama0_g_odd_coef : 13;
        unsigned int reserved_1 : 3;
    } reg;
} DPU_DPP_U_ROIGAMA0_G_COEF_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int u_roigama0_b_even_coef : 13;
        unsigned int reserved_0 : 3;
        unsigned int u_roigama0_b_odd_coef : 13;
        unsigned int reserved_1 : 3;
    } reg;
} DPU_DPP_U_ROIGAMA0_B_COEF_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int u_roigama1_r_even_coef : 13;
        unsigned int reserved_0 : 3;
        unsigned int u_roigama1_r_odd_coef : 13;
        unsigned int reserved_1 : 3;
    } reg;
} DPU_DPP_U_ROIGAMA1_R_COEF_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int u_roigama1_g_even_coef : 13;
        unsigned int reserved_0 : 3;
        unsigned int u_roigama1_g_odd_coef : 13;
        unsigned int reserved_1 : 3;
    } reg;
} DPU_DPP_U_ROIGAMA1_G_COEF_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int u_roigama1_b_even_coef : 13;
        unsigned int reserved_0 : 3;
        unsigned int u_roigama1_b_odd_coef : 13;
        unsigned int reserved_1 : 3;
    } reg;
} DPU_DPP_U_ROIGAMA1_B_COEF_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int reserved_0: 16;
        unsigned int reserved_1: 16;
    } reg;
} DPU_DPP_LOCAL_HIST_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int reserved_0: 10;
        unsigned int reserved_1: 7;
        unsigned int reserved_2: 7;
        unsigned int reserved_3: 7;
        unsigned int reserved_4: 1;
    } reg;
} DPU_DPP_LOCAL_GAMMA_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int detail_weight0 : 8;
        unsigned int detail_weight1 : 8;
        unsigned int detail_weight2 : 8;
        unsigned int detail_weight3 : 8;
    } reg;
} DPU_DPP_DETAIL_WEIGHT_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int log_lum0 : 14;
        unsigned int reserved_0: 2;
        unsigned int log_lum1 : 14;
        unsigned int reserved_1: 2;
    } reg;
} DPU_DPP_LOG_LUM_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int luma_gamma0 : 10;
        unsigned int luma_gamma1 : 10;
        unsigned int luma_gamma2 : 10;
        unsigned int reserved : 2;
    } reg;
} DPU_DPP_LUMA_GAMMA_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int reserved_0: 23;
        unsigned int reserved_1: 9;
    } reg;
} DPU_DPP_GLB_HIST_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int reserved_0: 23;
        unsigned int reserved_1: 9;
    } reg;
} DPU_DPP_GLB_SAT_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int reserved_0: 10;
        unsigned int reserved_1: 10;
        unsigned int reserved_2: 10;
        unsigned int reserved_3: 2;
    } reg;
} DPU_DPP_FNA_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int skincount_0 : 16;
        unsigned int skincount_1 : 16;
    } reg;
} DPU_DPP_SKINCOUNT_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int u_gama_r_even_coef : 13;
        unsigned int reserved_0 : 3;
        unsigned int u_gama_r_odd_coef : 13;
        unsigned int reserved_1 : 3;
    } reg;
} DPU_DPP_U_GAMA_R_COEF_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int u_gama_g_even_coef : 13;
        unsigned int reserved_0 : 3;
        unsigned int u_gama_g_odd_coef : 13;
        unsigned int reserved_1 : 3;
    } reg;
} DPU_DPP_U_GAMA_G_COEF_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int u_gama_b_even_coef : 13;
        unsigned int reserved_0 : 3;
        unsigned int u_gama_b_odd_coef : 13;
        unsigned int reserved_1 : 3;
    } reg;
} DPU_DPP_U_GAMA_B_COEF_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int u_gmp_coef : 32;
    } reg;
} DPU_DPP_U_GMP_COEF_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int u_alsc_roigama_r_even_coef : 13;
        unsigned int reserved_0 : 3;
        unsigned int u_alsc_roigama_r_odd_coef : 13;
        unsigned int reserved_1 : 3;
    } reg;
} DPU_DPP_U_ALSC_DEGAMMA_R_COEF_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int u_alsc_roigama_g_even_coef : 13;
        unsigned int reserved_0 : 3;
        unsigned int u_alsc_roigama_g_odd_coef : 13;
        unsigned int reserved_1 : 3;
    } reg;
} DPU_DPP_U_ALSC_DEGAMM_G_COEF_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int u_alsc_roigama_b_even_coef : 13;
        unsigned int reserved_0 : 3;
        unsigned int u_alsc_roigama_b_odd_coef : 13;
        unsigned int reserved_1 : 3;
    } reg;
} DPU_DPP_U_ALSC_DEGAMM_B_COEF_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int ace_int_stat : 1;
        unsigned int reserved : 31;
    } reg;
} DPU_HIACE_ACE_INT_STAT_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int ace_int_unmask : 1;
        unsigned int reserved : 31;
    } reg;
} DPU_HIACE_ACE_INT_UNMASK_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int bypass_ace : 1;
        unsigned int reserved : 31;
    } reg;
} DPU_HIACE_BYPASS_ACE_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int bypass_ace_stat : 1;
        unsigned int reserved : 31;
    } reg;
} DPU_HIACE_BYPASS_ACE_STAT_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int update_local : 1;
        unsigned int reserved : 31;
    } reg;
} DPU_HIACE_UPDATE_LOCAL_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int local_valid : 1;
        unsigned int reserved : 31;
    } reg;
} DPU_HIACE_LOCAL_VALID_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int gamma_ab_shadow : 1;
        unsigned int luma_gamma_ab_shadow : 1;
        unsigned int log_lum_ab_shadow : 1;
        unsigned int detail_weight_ab_shadow : 1;
        unsigned int reserved : 28;
    } reg;
} DPU_HIACE_GAMMA_AB_SHADOW_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int gamma_ab_work : 1;
        unsigned int luma_gamma_ab_work : 1;
        unsigned int log_lum_ab_work : 1;
        unsigned int detail_weight_ab_work : 1;
        unsigned int reserved : 28;
    } reg;
} DPU_HIACE_GAMMA_AB_WORK_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int global_hist_ab_shadow : 1;
        unsigned int reserved : 31;
    } reg;
} DPU_HIACE_GLOBAL_HIST_AB_SHADOW_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int global_hist_ab_work : 1;
        unsigned int reserved : 31;
    } reg;
} DPU_HIACE_GLOBAL_HIST_AB_WORK_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int reg_image_width : 13;
        unsigned int reserved_0 : 3;
        unsigned int reg_image_height : 13;
        unsigned int reserved_1 : 3;
    } reg;
} DPU_HIACE_IMAGE_INFO_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int reg_half_block_w : 9;
        unsigned int reserved_0 : 7;
        unsigned int reg_half_block_h : 9;
        unsigned int reserved_1 : 7;
    } reg;
} DPU_HIACE_HALF_BLOCK_INFO_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int reg_reciprocal_x : 10;
        unsigned int reg_fixbit_x : 5;
        unsigned int reserved_0 : 1;
        unsigned int reg_reciprocal_y : 10;
        unsigned int reg_fixbit_y : 5;
        unsigned int reserved_1 : 1;
    } reg;
} DPU_HIACE_XYWEIGHT_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int lhist_sft : 3;
        unsigned int reserved : 29;
    } reg;
} DPU_HIACE_LHIST_SFT_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int reg_roi_rec_x : 13;
        unsigned int reserved_0 : 3;
        unsigned int reg_roi_rec_y : 13;
        unsigned int reserved_1 : 3;
    } reg;
} DPU_HIACE_ROI_START_POINT_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int reg_roi_w : 13;
        unsigned int reserved_0: 3;
        unsigned int reg_roi_h : 13;
        unsigned int reserved_1: 3;
    } reg;
} DPU_HIACE_ROI_WIDTH_HIGH_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int reg_roi_mode : 2;
        unsigned int reserved : 30;
    } reg;
} DPU_HIACE_ROI_MODE_CTRL_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int reg_roi_gb_hist_stat_all : 1;
        unsigned int reg_roi_gb_sat_hist_stat_all : 1;
        unsigned int reserved_0 : 14;
        unsigned int reg_roi_lc_hist_stat_all : 1;
        unsigned int reserved_1 : 15;
    } reg;
} DPU_HIACE_ROI_HIST_STAT_MODE_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int reg_th_min_h : 9;
        unsigned int reserved_0 : 3;
        unsigned int reg_th_max_h : 9;
        unsigned int reserved_1 : 3;
        unsigned int reg_slop_h : 8;
    } reg;
} DPU_HIACE_HUE_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int reg_th_min_s : 8;
        unsigned int reserved_0 : 4;
        unsigned int reg_th_max_s : 8;
        unsigned int reserved_1 : 4;
        unsigned int reg_slop_s : 8;
    } reg;
} DPU_HIACE_SATURATION_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int reg_th_min_v : 8;
        unsigned int reserved_0 : 4;
        unsigned int reg_th_max_v : 8;
        unsigned int reserved_1 : 4;
        unsigned int reg_slop_v : 8;
    } reg;
} DPU_HIACE_VALUE_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int reg_skin_gain : 8;
        unsigned int reserved : 24;
    } reg;
} DPU_HIACE_SKIN_GAIN_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int reg_low_thres : 8;
        unsigned int reg_up_thres : 8;
        unsigned int reserved : 16;
    } reg;
} DPU_HIACE_UP_LOW_TH_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int reg_red_w : 5;
        unsigned int reserved_0 : 3;
        unsigned int reg_green_w : 5;
        unsigned int reserved_1 : 3;
        unsigned int reg_blue_w : 5;
        unsigned int reserved_2 : 11;
    } reg;
} DPU_HIACE_RGB_BLEND_WEIGHT_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int reg_classifier_shift_d1 : 5;
        unsigned int reserved_0 : 3;
        unsigned int reg_classifier_shift_d2 : 5;
        unsigned int reserved_1 : 19;
    } reg;
} DPU_HIACE_FNA_STATISTIC_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int up_cnt : 23;
        unsigned int reserved : 9;
    } reg;
} DPU_HIACE_UP_CNT_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int low_cnt : 23;
        unsigned int reserved : 9;
    } reg;
} DPU_HIACE_LOW_CNT_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int sum_sat : 32;
    } reg;
} DPU_HIACE_SUM_SATURATION_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int global_hist_0 : 23;
        unsigned int reserved : 9;
    } reg;
} DPU_HIACE_GLOBAL_HIST_0_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int global_hist_1 : 23;
        unsigned int reserved : 9;
    } reg;
} DPU_HIACE_GLOBAL_HIST_1_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int global_hist_2 : 23;
        unsigned int reserved : 9;
    } reg;
} DPU_HIACE_GLOBAL_HIST_2_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int global_hist_3 : 23;
        unsigned int reserved : 9;
    } reg;
} DPU_HIACE_GLOBAL_HIST_3_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int global_hist_4 : 23;
        unsigned int reserved : 9;
    } reg;
} DPU_HIACE_GLOBAL_HIST_4_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int global_hist_5 : 23;
        unsigned int reserved : 9;
    } reg;
} DPU_HIACE_GLOBAL_HIST_5_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int global_hist_6 : 23;
        unsigned int reserved : 9;
    } reg;
} DPU_HIACE_GLOBAL_HIST_6_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int global_hist_7 : 23;
        unsigned int reserved : 9;
    } reg;
} DPU_HIACE_GLOBAL_HIST_7_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int global_hist_8 : 23;
        unsigned int reserved : 9;
    } reg;
} DPU_HIACE_GLOBAL_HIST_8_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int global_hist_9 : 23;
        unsigned int reserved : 9;
    } reg;
} DPU_HIACE_GLOBAL_HIST_9_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int global_hist_10 : 23;
        unsigned int reserved : 9;
    } reg;
} DPU_HIACE_GLOBAL_HIST_10_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int global_hist_11 : 23;
        unsigned int reserved : 9;
    } reg;
} DPU_HIACE_GLOBAL_HIST_11_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int global_hist_12 : 23;
        unsigned int reserved : 9;
    } reg;
} DPU_HIACE_GLOBAL_HIST_12_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int global_hist_13 : 23;
        unsigned int reserved : 9;
    } reg;
} DPU_HIACE_GLOBAL_HIST_13_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int global_hist_14 : 23;
        unsigned int reserved : 9;
    } reg;
} DPU_HIACE_GLOBAL_HIST_14_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int global_hist_15 : 23;
        unsigned int reserved : 9;
    } reg;
} DPU_HIACE_GLOBAL_HIST_15_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int global_hist_16 : 23;
        unsigned int reserved : 9;
    } reg;
} DPU_HIACE_GLOBAL_HIST_16_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int global_hist_17 : 23;
        unsigned int reserved : 9;
    } reg;
} DPU_HIACE_GLOBAL_HIST_17_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int global_hist_18 : 23;
        unsigned int reserved : 9;
    } reg;
} DPU_HIACE_GLOBAL_HIST_18_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int global_hist_19 : 23;
        unsigned int reserved : 9;
    } reg;
} DPU_HIACE_GLOBAL_HIST_19_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int global_hist_20 : 23;
        unsigned int reserved : 9;
    } reg;
} DPU_HIACE_GLOBAL_HIST_20_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int global_hist_21 : 23;
        unsigned int reserved : 9;
    } reg;
} DPU_HIACE_GLOBAL_HIST_21_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int global_hist_22 : 23;
        unsigned int reserved : 9;
    } reg;
} DPU_HIACE_GLOBAL_HIST_22_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int global_hist_23 : 23;
        unsigned int reserved : 9;
    } reg;
} DPU_HIACE_GLOBAL_HIST_23_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int global_hist_24 : 23;
        unsigned int reserved : 9;
    } reg;
} DPU_HIACE_GLOBAL_HIST_24_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int global_hist_25 : 23;
        unsigned int reserved : 9;
    } reg;
} DPU_HIACE_GLOBAL_HIST_25_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int global_hist_26 : 23;
        unsigned int reserved : 9;
    } reg;
} DPU_HIACE_GLOBAL_HIST_26_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int global_hist_27 : 23;
        unsigned int reserved : 9;
    } reg;
} DPU_HIACE_GLOBAL_HIST_27_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int global_hist_28 : 23;
        unsigned int reserved : 9;
    } reg;
} DPU_HIACE_GLOBAL_HIST_28_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int global_hist_29 : 23;
        unsigned int reserved : 9;
    } reg;
} DPU_HIACE_GLOBAL_HIST_29_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int global_hist_30 : 23;
        unsigned int reserved : 9;
    } reg;
} DPU_HIACE_GLOBAL_HIST_30_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int global_hist_31 : 23;
        unsigned int reserved : 9;
    } reg;
} DPU_HIACE_GLOBAL_HIST_31_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int lhist_n : 4;
        unsigned int lhist_v : 3;
        unsigned int lhist_h : 3;
        unsigned int lhist_quant_en : 1;
        unsigned int reserved : 20;
        unsigned int lhist_en : 1;
    } reg;
} DPU_HIACE_LHIST_EN_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int local_hist_vxhy_2z : 16;
        unsigned int local_hist_vxhy_2z1 : 16;
    } reg;
} DPU_HIACE_LOCAL_HIST_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int gamma_n_w : 3;
        unsigned int reserved_0 : 1;
        unsigned int gamma_v_w : 3;
        unsigned int gamma_h_w : 3;
        unsigned int reserved_1 : 21;
        unsigned int gamma_en_w : 1;
    } reg;
} DPU_HIACE_GAMMA_W_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int gamma_vxhy_3z_w : 10;
        unsigned int gamma_vxhy_3z1_w : 7;
        unsigned int gamma_vxhy_3z2_w : 7;
        unsigned int gamma_vxhy_3z3_w : 7;
        unsigned int reserved : 1;
    } reg;
} DPU_HIACE_GAMMA_VXHY_W_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int gamma_n_r : 4;
        unsigned int gamma_v_r : 3;
        unsigned int gamma_h_r : 3;
        unsigned int reserved : 21;
        unsigned int gamma_en_r : 1;
    } reg;
} DPU_HIACE_GAMMA_R_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int gamma_vxhy_3z_r : 10;
        unsigned int gamma_vxhy_3z1_r : 7;
        unsigned int gamma_vxhy_3z2_r : 7;
        unsigned int gamma_vxhy_3z3_r : 7;
        unsigned int reserved : 1;
    } reg;
} DPU_HIACE_GAMMA_VXHY_R_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int init_gamma : 1;
        unsigned int reserved : 31;
    } reg;
} DPU_HIACE_INIT_GAMMA_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int use_manual_reload : 1;
        unsigned int manual_reload_valid : 1;
        unsigned int reserved : 30;
    } reg;
} DPU_HIACE_MANUAL_RELOAD_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int mem_lp_en : 1;
        unsigned int reserved_0 : 3;
        unsigned int ramclk_func_en : 1;
        unsigned int ramclk_func_lhist_clkact : 1;
        unsigned int ramclk_func_gamma_a_clkact : 1;
        unsigned int ramclk_func_gamma_b_clkact : 1;
        unsigned int reserved_1 : 24;
    } reg;
} DPU_HIACE_RAMCLK_FUNC_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int clk_gate_en : 1;
        unsigned int reserved : 31;
    } reg;
} DPU_HIACE_CLK_GATE_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int gamma_ram_a_cfg_mem_ctrl : 32;
    } reg;
} DPU_HIACE_GAMMA_RAM_A_CFG_MEM_CTRL_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int gamma_ram_b_cfg_mem_ctrl : 32;
    } reg;
} DPU_HIACE_GAMMA_RAM_B_CFG_MEM_CTRL_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int lhist_ram_cfg_mem_ctrl : 32;
    } reg;
} DPU_HIACE_LHIST_RAM_CFG_MEM_CTRL_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int gamma_ram_a_cfg_pm_ctrl : 32;
    } reg;
} DPU_HIACE_GAMMA_RAM_A_CFG_PM_CTRL_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int gamma_ram_b_cfg_pm_ctrl : 32;
    } reg;
} DPU_HIACE_GAMMA_RAM_B_CFG_PM_CTRL_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int lhist_ram_cfg_pm_ctrl : 32;
    } reg;
} DPU_HIACE_LHIST_RAM_CFG_PM_CTRL_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int sat_global_hist_0 : 23;
        unsigned int reserved : 9;
    } reg;
} DPU_HIACE_SAT_GLOBAL_HIST_0_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int sat_global_hist_1 : 23;
        unsigned int reserved : 9;
    } reg;
} DPU_HIACE_SAT_GLOBAL_HIST_1_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int sat_global_hist_2 : 23;
        unsigned int reserved : 9;
    } reg;
} DPU_HIACE_SAT_GLOBAL_HIST_2_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int sat_global_hist_3 : 23;
        unsigned int reserved : 9;
    } reg;
} DPU_HIACE_SAT_GLOBAL_HIST_3_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int sat_global_hist_4 : 23;
        unsigned int reserved : 9;
    } reg;
} DPU_HIACE_SAT_GLOBAL_HIST_4_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int sat_global_hist_5 : 23;
        unsigned int reserved : 9;
    } reg;
} DPU_HIACE_SAT_GLOBAL_HIST_5_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int sat_global_hist_6 : 23;
        unsigned int reserved : 9;
    } reg;
} DPU_HIACE_SAT_GLOBAL_HIST_6_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int sat_global_hist_7 : 23;
        unsigned int reserved : 9;
    } reg;
} DPU_HIACE_SAT_GLOBAL_HIST_7_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int sat_global_hist_8 : 23;
        unsigned int reserved : 9;
    } reg;
} DPU_HIACE_SAT_GLOBAL_HIST_8_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int sat_global_hist_9 : 23;
        unsigned int reserved : 9;
    } reg;
} DPU_HIACE_SAT_GLOBAL_HIST_9_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int sat_global_hist_10 : 23;
        unsigned int reserved : 9;
    } reg;
} DPU_HIACE_SAT_GLOBAL_HIST_10_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int sat_global_hist_11 : 23;
        unsigned int reserved : 9;
    } reg;
} DPU_HIACE_SAT_GLOBAL_HIST_11_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int sat_global_hist_12 : 23;
        unsigned int reserved : 9;
    } reg;
} DPU_HIACE_SAT_GLOBAL_HIST_12_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int sat_global_hist_13 : 23;
        unsigned int reserved : 9;
    } reg;
} DPU_HIACE_SAT_GLOBAL_HIST_13_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int sat_global_hist_14 : 23;
        unsigned int reserved : 9;
    } reg;
} DPU_HIACE_SAT_GLOBAL_HIST_14_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int sat_global_hist_15 : 23;
        unsigned int reserved : 9;
    } reg;
} DPU_HIACE_SAT_GLOBAL_HIST_15_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int sat_global_hist_16 : 23;
        unsigned int reserved : 9;
    } reg;
} DPU_HIACE_SAT_GLOBAL_HIST_16_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int sat_global_hist_17 : 23;
        unsigned int reserved : 9;
    } reg;
} DPU_HIACE_SAT_GLOBAL_HIST_17_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int sat_global_hist_18 : 23;
        unsigned int reserved : 9;
    } reg;
} DPU_HIACE_SAT_GLOBAL_HIST_18_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int sat_global_hist_19 : 23;
        unsigned int reserved : 9;
    } reg;
} DPU_HIACE_SAT_GLOBAL_HIST_19_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int sat_global_hist_20 : 23;
        unsigned int reserved : 9;
    } reg;
} DPU_HIACE_SAT_GLOBAL_HIST_20_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int sat_global_hist_21 : 23;
        unsigned int reserved : 9;
    } reg;
} DPU_HIACE_SAT_GLOBAL_HIST_21_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int sat_global_hist_22 : 23;
        unsigned int reserved : 9;
    } reg;
} DPU_HIACE_SAT_GLOBAL_HIST_22_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int sat_global_hist_23 : 23;
        unsigned int reserved : 9;
    } reg;
} DPU_HIACE_SAT_GLOBAL_HIST_23_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int sat_global_hist_24 : 23;
        unsigned int reserved : 9;
    } reg;
} DPU_HIACE_SAT_GLOBAL_HIST_24_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int sat_global_hist_25 : 23;
        unsigned int reserved : 9;
    } reg;
} DPU_HIACE_SAT_GLOBAL_HIST_25_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int sat_global_hist_26 : 23;
        unsigned int reserved : 9;
    } reg;
} DPU_HIACE_SAT_GLOBAL_HIST_26_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int sat_global_hist_27 : 23;
        unsigned int reserved : 9;
    } reg;
} DPU_HIACE_SAT_GLOBAL_HIST_27_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int sat_global_hist_28 : 23;
        unsigned int reserved : 9;
    } reg;
} DPU_HIACE_SAT_GLOBAL_HIST_28_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int sat_global_hist_29 : 23;
        unsigned int reserved : 9;
    } reg;
} DPU_HIACE_SAT_GLOBAL_HIST_29_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int sat_global_hist_30 : 23;
        unsigned int reserved : 9;
    } reg;
} DPU_HIACE_SAT_GLOBAL_HIST_30_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int sat_global_hist_31 : 23;
        unsigned int reserved : 9;
    } reg;
} DPU_HIACE_SAT_GLOBAL_HIST_31_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int reserved_0: 4;
        unsigned int fna_v : 3;
        unsigned int fna_h : 3;
        unsigned int reserved_1: 21;
        unsigned int fna_en_r : 1;
    } reg;
} DPU_HIACE_FNA_ADDR_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int fna_vxhy : 30;
        unsigned int reserved : 2;
    } reg;
} DPU_HIACE_FNA_DATA_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int update_fna : 1;
        unsigned int reserved : 31;
    } reg;
} DPU_HIACE_UPDATE_FNA_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int fna_valid : 1;
        unsigned int reserved : 31;
    } reg;
} DPU_HIACE_FNA_VALID_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int reg_pipe_mode : 2;
        unsigned int reserved : 28;
        unsigned int reg_partition_mode : 1;
        unsigned int reg_is_left_pipe : 1;
    } reg;
} DPU_HIACE_DB_PIPE_CFG_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int reg_pipe_ext_width : 5;
        unsigned int reserved : 27;
    } reg;
} DPU_HIACE_DB_PIPE_EXT_WIDTH_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int reg_full_image_width : 13;
        unsigned int reserved : 19;
    } reg;
} DPU_HIACE_DB_PIPE_FULL_IMG_WIDTH_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int hdr_gammasampe_en : 1;
        unsigned int linear_invpqgamma_en : 1;
        unsigned int linear_hist_en : 1;
        unsigned int hdr_mode_en : 1;
        unsigned int reserved : 28;
    } reg;
} DPU_HIACE_HDR_EN_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int loglum_max : 14;
        unsigned int reserved : 18;
    } reg;
} DPU_HIACE_LOGLUM_MAX_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int yuv_coef_0 : 10;
        unsigned int yuv_coef_1 : 10;
        unsigned int yuv_coef_2 : 10;
        unsigned int hist_modev_en : 1;
        unsigned int reserved : 1;
    } reg;
} DPU_HIACE_HIST_MODEV_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int endpoint_slope : 10;
        unsigned int reserved_0 : 6;
        unsigned int endpoint : 10;
        unsigned int reserved_1 : 6;
    } reg;
} DPU_HIACE_END_POINT_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int col_cnt : 13;
        unsigned int row_cnt : 13;
        unsigned int hint : 3;
        unsigned int vint : 3;
    } reg;
} DPU_HIACE_ACE_DBG0_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int apb_state : 4;
        unsigned int lhist_state : 3;
        unsigned int gamma_rd_state : 3;
        unsigned int gamma_wr_state : 2;
        unsigned int fna_state : 2;
        unsigned int hind : 3;
        unsigned int vind : 3;
        unsigned int fna_calc : 1;
        unsigned int lhist_calc : 1;
        unsigned int reserved : 10;
    } reg;
} DPU_HIACE_ACE_DBG1_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int clk_state : 5;
        unsigned int slp_cnt : 2;
        unsigned int vblank_flag : 1;
        unsigned int lhistgram_alive : 1;
        unsigned int lhistgram_slp_en : 1;
        unsigned int lhistgram_clk_active : 1;
        unsigned int gamaram0_alive : 1;
        unsigned int gamaram0_slp_en : 1;
        unsigned int gamaram0_clk_active : 1;
        unsigned int gamaram1_alive : 1;
        unsigned int gamaram1_slp_en : 1;
        unsigned int gamaram1_clk_active : 1;
        unsigned int ace_active : 1;
        unsigned int ace_nonbypass_clk_active : 1;
        unsigned int reserved : 13;
    } reg;
} DPU_HIACE_ACE_DBG2_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int dbg_clk : 32;
    } reg;
} DPU_HIACE_ACE_DBG_CLK_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int bypass_nr : 1;
        unsigned int bypass_nr_gain : 1;
        unsigned int reserved : 30;
    } reg;
} DPU_HIACE_BYPASS_NR_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int somebrightness0 : 10;
        unsigned int reserved_0 : 6;
        unsigned int somebrightness1 : 10;
        unsigned int reserved_1 : 6;
    } reg;
} DPU_HIACE_S3_SOME_BRIGHTNESS01_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int somebrightness2 : 10;
        unsigned int reserved_0 : 6;
        unsigned int somebrightness3 : 10;
        unsigned int reserved_1 : 6;
    } reg;
} DPU_HIACE_S3_SOME_BRIGHTNESS23_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int somebrightness4 : 10;
        unsigned int reserved : 22;
    } reg;
} DPU_HIACE_S3_SOME_BRIGHTNESS4_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int minSigma : 5;
        unsigned int reserved_0: 11;
        unsigned int maxSigma : 5;
        unsigned int reserved_1: 11;
    } reg;
} DPU_HIACE_S3_MIN_MAX_SIGMA_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int greenSigma0 : 5;
        unsigned int reserved_0 : 3;
        unsigned int greenSigma1 : 5;
        unsigned int reserved_1 : 3;
        unsigned int greenSigma2 : 5;
        unsigned int reserved_2 : 3;
        unsigned int greenSigma3 : 5;
        unsigned int reserved_3 : 3;
    } reg;
} DPU_HIACE_S3_GREEN_SIGMA03_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int greenSigma4 : 5;
        unsigned int reserved_0 : 3;
        unsigned int greenSigma5 : 5;
        unsigned int reserved_1 : 19;
    } reg;
} DPU_HIACE_S3_GREEN_SIGMA45_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int redSigma0 : 5;
        unsigned int reserved_0: 3;
        unsigned int redSigma1 : 5;
        unsigned int reserved_1: 3;
        unsigned int redSigma2 : 5;
        unsigned int reserved_2: 3;
        unsigned int redSigma3 : 5;
        unsigned int reserved_3: 3;
    } reg;
} DPU_HIACE_S3_RED_SIGMA03_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int redSigma4 : 5;
        unsigned int reserved_0: 3;
        unsigned int redSigma5 : 5;
        unsigned int reserved_1: 19;
    } reg;
} DPU_HIACE_S3_RED_SIGMA45_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int blueSigma0 : 5;
        unsigned int reserved_0 : 3;
        unsigned int blueSigma1 : 5;
        unsigned int reserved_1 : 3;
        unsigned int blueSigma2 : 5;
        unsigned int reserved_2 : 3;
        unsigned int blueSigma3 : 5;
        unsigned int reserved_3 : 3;
    } reg;
} DPU_HIACE_S3_BLUE_SIGMA03_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int blueSigma4 : 5;
        unsigned int reserved_0 : 3;
        unsigned int blueSigma5 : 5;
        unsigned int reserved_1 : 19;
    } reg;
} DPU_HIACE_S3_BLUE_SIGMA45_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int whiteSigma0 : 5;
        unsigned int reserved_0 : 3;
        unsigned int whiteSigma1 : 5;
        unsigned int reserved_1 : 3;
        unsigned int whiteSigma2 : 5;
        unsigned int reserved_2 : 3;
        unsigned int whiteSigma3 : 5;
        unsigned int reserved_3 : 3;
    } reg;
} DPU_HIACE_S3_WHITE_SIGMA03_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int whiteSigma4 : 5;
        unsigned int reserved_0 : 3;
        unsigned int whiteSigma5 : 5;
        unsigned int reserved_1 : 19;
    } reg;
} DPU_HIACE_S3_WHITE_SIGMA45_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int filterLevel : 5;
        unsigned int reserved : 27;
    } reg;
} DPU_HIACE_S3_FILTER_LEVEL_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int similarityCoeff : 10;
        unsigned int reserved : 22;
    } reg;
} DPU_HIACE_S3_SIMILARITY_COEFF_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int verFilterWeightAdj : 2;
        unsigned int reserved : 30;
    } reg;
} DPU_HIACE_S3_V_FILTER_WEIGHT_ADJ_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int th_min_h : 9;
        unsigned int reserved_0: 3;
        unsigned int th_max_h : 9;
        unsigned int reserved_1: 3;
        unsigned int slop_h : 8;
    } reg;
} DPU_HIACE_S3_HUE_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int th_min_s : 8;
        unsigned int reserved_0: 4;
        unsigned int th_max_s : 8;
        unsigned int reserved_1: 4;
        unsigned int slop_s : 8;
    } reg;
} DPU_HIACE_S3_SATURATION_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int th_min_v : 8;
        unsigned int reserved_0: 4;
        unsigned int th_max_v : 8;
        unsigned int reserved_1: 4;
        unsigned int slop_v : 8;
    } reg;
} DPU_HIACE_S3_VALUE_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int skin_gain : 8;
        unsigned int reserved : 24;
    } reg;
} DPU_HIACE_S3_SKIN_GAIN_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int mem_lp_nr_en : 1;
        unsigned int reserved_0 : 3;
        unsigned int ramclk_func_nr_en : 1;
        unsigned int ramclk_func_nr_clkact : 1;
        unsigned int reserved_1 : 26;
    } reg;
} DPU_HIACE_NR_RAMCLK_FUNC_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int clk_gate_nr_en : 1;
        unsigned int reserved : 31;
    } reg;
} DPU_HIACE_NR_CLK_GATE_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int nr_ram_a_cfg_mem_ctrl : 32;
    } reg;
} DPU_HIACE_NR_RAM_A_CFG_MEM_CTRL_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int nr_ram_a_cfg_pm_ctrl : 32;
    } reg;
} DPU_HIACE_NR_RAM_A_CFG_PM_CTRL_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int dsc_version_minor : 4;
        unsigned int dsc_version_major : 4;
        unsigned int reserved : 24;
    } reg;
} DPU_DSC_VERSION_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int pps_identifier : 8;
        unsigned int reserved : 24;
    } reg;
} DPU_DSC_PPS_IDENTIFIER_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int dsc_en : 1;
        unsigned int dual_dsc_en : 1;
        unsigned int reset_ich_per_line : 1;
        unsigned int reserved : 29;
    } reg;
} DPU_DSC_EN_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int bits_per_component : 4;
        unsigned int linebuf_depth : 4;
        unsigned int dsc_insert_byte_num : 3;
        unsigned int convert_rgb : 1;
        unsigned int enable_422 : 1;
        unsigned int native_422 : 1;
        unsigned int vbr_enable : 1;
        unsigned int cmd_mode : 1;
        unsigned int bits_per_pixel : 10;
        unsigned int block_pred_enable : 1;
        unsigned int idata_422 : 1;
        unsigned int full_ich_err_precision : 1;
        unsigned int reserved : 3;
    } reg;
} DPU_DSC_CTRL_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int pic_height : 16;
        unsigned int pic_width : 16;
    } reg;
} DPU_DSC_PIC_SIZE_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int slice_height : 16;
        unsigned int slice_width : 16;
    } reg;
} DPU_DSC_SLICE_SIZE_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int chunk_size : 16;
        unsigned int reserved : 16;
    } reg;
} DPU_DSC_CHUNK_SIZE_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int initial_xmit_delay : 10;
        unsigned int reserved : 6;
        unsigned int initial_dec_delay : 16;
    } reg;
} DPU_DSC_INITIAL_DELAY_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int initial_scale_value : 6;
        unsigned int reserved : 10;
        unsigned int scale_increment_interval : 16;
    } reg;
} DPU_DSC_RC_PARAM0_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int scale_decrement_interval : 12;
        unsigned int reserved_0 : 4;
        unsigned int first_line_bpg_offset : 5;
        unsigned int reserved_1 : 11;
    } reg;
} DPU_DSC_RC_PARAM1_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int nfl_bpg_offset : 16;
        unsigned int slice_bpg_offset : 16;
    } reg;
} DPU_DSC_RC_PARAM2_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int initial_offset : 16;
        unsigned int final_offset : 16;
    } reg;
} DPU_DSC_RC_PARAM3_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int flatness_min_qp : 8;
        unsigned int somewhat_flat_qp : 5;
        unsigned int reserved_0 : 3;
        unsigned int flatness_max_qp : 8;
        unsigned int very_flat_qp : 5;
        unsigned int reserved_1 : 3;
    } reg;
} DPU_DSC_FLATNESS_PARAM_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int rc_model_size : 16;
        unsigned int reserved_0 : 4;
        unsigned int rc_edge_factor : 4;
        unsigned int reserved_1 : 8;
    } reg;
} DPU_DSC_RC_PARAM4_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int rc_quant_incr_limit0 : 5;
        unsigned int reserved_0 : 3;
        unsigned int rc_quant_incr_limit1 : 5;
        unsigned int reserved_1 : 3;
        unsigned int rc_tgt_offset_hi : 4;
        unsigned int rc_tgt_offset_lo : 4;
        unsigned int reserved_2 : 8;
    } reg;
} DPU_DSC_RC_PARAM5_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int rc_buf_thresh3 : 8;
        unsigned int rc_buf_thresh2 : 8;
        unsigned int rc_buf_thresh1 : 8;
        unsigned int rc_buf_thresh0 : 8;
    } reg;
} DPU_DSC_RC_BUF_THRESH0_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int rc_buf_thresh7 : 8;
        unsigned int rc_buf_thresh6 : 8;
        unsigned int rc_buf_thresh5 : 8;
        unsigned int rc_buf_thresh4 : 8;
    } reg;
} DPU_DSC_RC_BUF_THRESH1_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int rc_buf_thresh11 : 8;
        unsigned int rc_buf_thresh10 : 8;
        unsigned int rc_buf_thresh9 : 8;
        unsigned int rc_buf_thresh8 : 8;
    } reg;
} DPU_DSC_RC_BUF_THRESH2_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int reserved : 16;
        unsigned int rc_buf_thresh13 : 8;
        unsigned int rc_buf_thresh12 : 8;
    } reg;
} DPU_DSC_RC_BUF_THRESH3_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int range_bpg_offset1 : 6;
        unsigned int range_max_qp1 : 5;
        unsigned int range_min_qp1 : 5;
        unsigned int range_bpg_offset0 : 6;
        unsigned int range_max_qp0 : 5;
        unsigned int range_min_qp0 : 5;
    } reg;
} DPU_DSC_RC_RANGE_PARAM0_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int range_bpg_offset3 : 6;
        unsigned int range_max_qp3 : 5;
        unsigned int range_min_qp3 : 5;
        unsigned int range_bpg_offset2 : 6;
        unsigned int range_max_qp2 : 5;
        unsigned int range_min_qp2 : 5;
    } reg;
} DPU_DSC_RC_RANGE_PARAM1_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int range_bpg_offset5 : 6;
        unsigned int range_max_qp5 : 5;
        unsigned int range_min_qp5 : 5;
        unsigned int range_bpg_offset4 : 6;
        unsigned int range_max_qp4 : 5;
        unsigned int range_min_qp4 : 5;
    } reg;
} DPU_DSC_RC_RANGE_PARAM2_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int range_bpg_offset7 : 6;
        unsigned int range_max_qp7 : 5;
        unsigned int range_min_qp7 : 5;
        unsigned int range_bpg_offset6 : 6;
        unsigned int range_max_qp6 : 5;
        unsigned int range_min_qp6 : 5;
    } reg;
} DPU_DSC_RC_RANGE_PARAM3_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int range_bpg_offset9 : 6;
        unsigned int range_max_qp9 : 5;
        unsigned int range_min_qp9 : 5;
        unsigned int range_bpg_offset8 : 6;
        unsigned int range_max_qp8 : 5;
        unsigned int range_min_qp8 : 5;
    } reg;
} DPU_DSC_RC_RANGE_PARAM4_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int range_bpg_offset11 : 6;
        unsigned int range_max_qp11 : 5;
        unsigned int range_min_qp11 : 5;
        unsigned int range_bpg_offset10 : 6;
        unsigned int range_max_qp10 : 5;
        unsigned int range_min_qp10 : 5;
    } reg;
} DPU_DSC_RC_RANGE_PARAM5_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int range_bpg_offset13 : 6;
        unsigned int range_max_qp13 : 5;
        unsigned int range_min_qp13 : 5;
        unsigned int range_bpg_offset12 : 6;
        unsigned int range_max_qp12 : 5;
        unsigned int range_min_qp12 : 5;
    } reg;
} DPU_DSC_RC_RANGE_PARAM6_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int reserved : 16;
        unsigned int range_bpg_offset14 : 6;
        unsigned int range_max_qp14 : 5;
        unsigned int range_min_qp14 : 5;
    } reg;
} DPU_DSC_RC_RANGE_PARAM7_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int adjustment_bits : 4;
        unsigned int reserved : 28;
    } reg;
} DPU_DSC_ADJUSTMENT_BITS_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int bits_per_grp : 6;
        unsigned int reserved_0 : 2;
        unsigned int adj_bits_per_grp : 6;
        unsigned int reserved_1 : 18;
    } reg;
} DPU_DSC_BITS_PER_GRP_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int slices_per_line : 2;
        unsigned int reserved : 14;
        unsigned int pic_line_grp_num : 16;
    } reg;
} DPU_DSC_MULTI_SLICE_CTL_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int dsc_out_mode : 1;
        unsigned int reserved : 31;
    } reg;
} DPU_DSC_OUT_CTRL_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int dsc_clk_sel : 32;
    } reg;
} DPU_DSC_CLK_SEL_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int dsc_clk_en : 32;
    } reg;
} DPU_DSC_CLK_EN_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int dsc_mem_ctrl_0 : 4;
        unsigned int dsc_mem_ctrl_1 : 4;
        unsigned int dsc_mem_ctrl_2 : 4;
        unsigned int dsc_mem_ctrl_3 : 4;
        unsigned int dsc_mem_ctrl_4 : 4;
        unsigned int dsc_mem_ctrl_5 : 4;
        unsigned int dsc_mem_ctrl_6 : 4;
        unsigned int dsc_mem_ctrl_7 : 4;
    } reg;
} DPU_DSC_MEM_CTRL_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int datain_line_cnt : 13;
        unsigned int reserved_0 : 3;
        unsigned int datain_pix_cnt : 13;
        unsigned int reserved_1 : 3;
    } reg;
} DPU_DSC_ST_DATAIN_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int dataout_line_cnt : 13;
        unsigned int reserved : 3;
        unsigned int dataout_byte_cnt : 16;
    } reg;
} DPU_DSC_ST_DATAOUT_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int dsc_slice_line_cnt : 12;
        unsigned int reserved_0 : 4;
        unsigned int dsc_slice_pix_cnt : 12;
        unsigned int reserved_1 : 4;
    } reg;
} DPU_DSC_ST_SLC_POS_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int dsc_pic_line_cnt : 12;
        unsigned int reserved_0 : 4;
        unsigned int dsc_pic_pix_cnt : 12;
        unsigned int reserved_1 : 4;
    } reg;
} DPU_DSC_ST_PIC_POS_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int dsc_flatff_full : 1;
        unsigned int dsc_flatff_empty : 1;
        unsigned int dsc_blncff0_full : 1;
        unsigned int dsc_blncff0_empty : 1;
        unsigned int dsc_blncff1_full : 1;
        unsigned int dsc_blncff1_empty : 1;
        unsigned int dsc_sesizeff0_full : 1;
        unsigned int dsc_sesizeff0_empty : 1;
        unsigned int dsc_sesizeff1_full : 1;
        unsigned int dsc_sesizeff1_empty : 1;
        unsigned int dsc_rcbuf0_full : 1;
        unsigned int dsc_rcbuf0_empty : 1;
        unsigned int dsc_rcbuf1_full : 1;
        unsigned int dsc_rcbuf1_empty : 1;
        unsigned int reserved : 18;
    } reg;
} DPU_DSC_ST_FIFO_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int dsc_addr_linebuf : 10;
        unsigned int reserved_0 : 6;
        unsigned int dsc_infifo_full : 1;
        unsigned int dsc_infifo_empty : 1;
        unsigned int dsc_infifo_uflow : 1;
        unsigned int dsc_infifo_oflow : 1;
        unsigned int dsc_outfifo_full : 1;
        unsigned int dsc_outfifo_empty : 1;
        unsigned int dsc_outfifo_uflow : 1;
        unsigned int dsc_outfifo_oflow : 1;
        unsigned int reserved_1 : 8;
    } reg;
} DPU_DSC_ST_LINEBUF_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int dsc_pre_drdy : 1;
        unsigned int dsc_pre_dneed : 1;
        unsigned int dsc_dv2pred : 1;
        unsigned int dsc_nxt_dneed : 1;
        unsigned int dsc_nxt_drdy : 1;
        unsigned int reserved : 27;
    } reg;
} DPU_DSC_ST_ITFC_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int reserved_0: 16;
        unsigned int reserved_1: 16;
    } reg;
} DPU_DSC_REG_DEBUG_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int flatness_det_thresh : 10;
        unsigned int reserved : 22;
    } reg;
} DPU_DSC_12_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int csc_idc0 : 11;
        unsigned int reserved_0: 5;
        unsigned int csc_idc1 : 11;
        unsigned int reserved_1: 5;
    } reg;
} DPU_DSC_CSC_IDC0_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int csc_idc2 : 11;
        unsigned int reserved : 21;
    } reg;
} DPU_DSC_CSC_IDC2_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int csc_odc0 : 11;
        unsigned int reserved_0: 5;
        unsigned int csc_odc1 : 11;
        unsigned int reserved_1: 5;
    } reg;
} DPU_DSC_CSC_ODC0_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int csc_odc2 : 11;
        unsigned int reserved : 21;
    } reg;
} DPU_DSC_CSC_ODC2_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int csc_p00 : 17;
        unsigned int reserved : 15;
    } reg;
} DPU_DSC_CSC_P00_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int csc_p01 : 17;
        unsigned int reserved : 15;
    } reg;
} DPU_DSC_CSC_P01_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int csc_p02 : 17;
        unsigned int reserved : 15;
    } reg;
} DPU_DSC_CSC_P02_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int csc_p10 : 17;
        unsigned int reserved : 15;
    } reg;
} DPU_DSC_CSC_P10_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int csc_p11 : 17;
        unsigned int reserved : 15;
    } reg;
} DPU_DSC_CSC_P11_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int csc_p12 : 17;
        unsigned int reserved : 15;
    } reg;
} DPU_DSC_CSC_P12_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int csc_p20 : 17;
        unsigned int reserved : 15;
    } reg;
} DPU_DSC_CSC_P20_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int csc_p21 : 17;
        unsigned int reserved : 15;
    } reg;
} DPU_DSC_CSC_P21_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int csc_p22 : 17;
        unsigned int reserved : 15;
    } reg;
} DPU_DSC_CSC_P22_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int reserved : 32;
    } reg;
} DPU_DSC_CSC_MPREC_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int padd_en : 1;
        unsigned int padd_mode : 1;
        unsigned int reserved : 30;
    } reg;
} DPU_DSC_PADD_CTRL_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int padd_width_bef_padd : 13;
        unsigned int reserved_0 : 3;
        unsigned int padd_height : 13;
        unsigned int reserved_1 : 3;
    } reg;
} DPU_DSC_PADD_SIZE_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int padd_width_aft_padd : 13;
        unsigned int reserved_0 : 3;
        unsigned int padd_left : 13;
        unsigned int reserved_1 : 3;
    } reg;
} DPU_DSC_PADD_WIDTH_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int padd_data : 30;
        unsigned int reserved : 2;
    } reg;
} DPU_DSC_PADD_DATA_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int txip_en : 1;
        unsigned int txip_ppro_bypass : 1;
        unsigned int txip_shift : 4;
        unsigned int txip_rdmode : 1;
        unsigned int reserved : 25;
    } reg;
} DPU_DSC_TXIP_CTRL_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int txip_height : 16;
        unsigned int txip_width : 16;
    } reg;
} DPU_DSC_TXIP_SIZE_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int txip_coeff03 : 5;
        unsigned int reserved_0 : 3;
        unsigned int txip_coeff02 : 5;
        unsigned int reserved_1 : 3;
        unsigned int txip_coeff01 : 5;
        unsigned int reserved_2 : 3;
        unsigned int txip_coeff00 : 5;
        unsigned int reserved_3 : 3;
    } reg;
} DPU_DSC_TXIP_COEF0_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int txip_coeff13 : 5;
        unsigned int reserved_0 : 3;
        unsigned int txip_coeff12 : 5;
        unsigned int reserved_1 : 3;
        unsigned int txip_coeff11 : 5;
        unsigned int reserved_2 : 3;
        unsigned int txip_coeff10 : 5;
        unsigned int reserved_3 : 3;
    } reg;
} DPU_DSC_TXIP_COEF1_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int txip_coeff23 : 5;
        unsigned int reserved_0 : 3;
        unsigned int txip_coeff22 : 5;
        unsigned int reserved_1 : 3;
        unsigned int txip_coeff21 : 5;
        unsigned int reserved_2 : 3;
        unsigned int txip_coeff20 : 5;
        unsigned int reserved_3 : 3;
    } reg;
} DPU_DSC_TXIP_COEF2_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int txip_coeff33 : 5;
        unsigned int reserved_0 : 3;
        unsigned int txip_coeff32 : 5;
        unsigned int reserved_1 : 3;
        unsigned int txip_coeff31 : 5;
        unsigned int reserved_2 : 3;
        unsigned int txip_coeff30 : 5;
        unsigned int reserved_3 : 3;
    } reg;
} DPU_DSC_TXIP_COEF3_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int txip_offset1 : 11;
        unsigned int reserved_0 : 5;
        unsigned int txip_offset0 : 11;
        unsigned int reserved_1 : 5;
    } reg;
} DPU_DSC_TXIP_OFFSET0_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int txip_offset3 : 11;
        unsigned int reserved_0 : 5;
        unsigned int txip_offset2 : 11;
        unsigned int reserved_1 : 5;
    } reg;
} DPU_DSC_TXIP_OFFSET1_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int txip_clipmin : 10;
        unsigned int reserved_0 : 6;
        unsigned int txip_clipmax : 10;
        unsigned int reserved_1 : 6;
    } reg;
} DPU_DSC_TXIP_CLIP_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int datapack_en : 1;
        unsigned int datapack_ppro_bypass : 1;
        unsigned int datapack_subpxl_layout : 1;
        unsigned int datapack_packmode : 1;
        unsigned int reserved : 28;
    } reg;
} DPU_DSC_DATAPACK_CTRL_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int datapack_height : 16;
        unsigned int datapack_width : 16;
    } reg;
} DPU_DSC_DATAPACK_SIZE_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int dsc_top_rd_shadow : 1;
        unsigned int reserved : 31;
    } reg;
} DPU_DSC_RD_SHADOW_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int reg_default : 1;
        unsigned int reserved : 31;
    } reg;
} DPU_DSC_REG_DEFAULT_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int dsc_top_clk_sel : 32;
    } reg;
} DPU_DSC_TOP_CLK_SEL_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int dsc_top_clk_en : 32;
    } reg;
} DPU_DSC_TOP_CLK_EN_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int reg_ctrl_scene_id : 3;
        unsigned int reserved : 29;
    } reg;
} DPU_DSC_REG_CTRL_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int reg_ctrl_flush_en : 1;
        unsigned int reserved : 31;
    } reg;
} DPU_DSC_REG_CTRL_FLUSH_EN_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int reg_ctrl_dbg : 32;
    } reg;
} DPU_DSC_REG_CTRL_DEBUG_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int reg_ctrl_state : 32;
    } reg;
} DPU_DSC_REG_CTRL_STATE_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int r_lb_debug0 : 32;
    } reg;
} DPU_DSC_R_LB_DEBUG0_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int r_lb_debug1 : 32;
    } reg;
} DPU_DSC_R_LB_DEBUG1_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int r_lb_debug2 : 32;
    } reg;
} DPU_DSC_R_LB_DEBUG2_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int r_lb_debug3 : 32;
    } reg;
} DPU_DSC_R_LB_DEBUG3_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int w_lb_debug0 : 32;
    } reg;
} DPU_DSC_W_LB_DEBUG0_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int w_lb_debug1 : 32;
    } reg;
} DPU_DSC_W_LB_DEBUG1_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int w_lb_debug2 : 32;
    } reg;
} DPU_DSC_W_LB_DEBUG2_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int w_lb_debug3 : 32;
    } reg;
} DPU_DSC_W_LB_DEBUG3_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int reg_ctrl_layer_id : 5;
        unsigned int reserved : 27;
    } reg;
} DPU_DSC_REG_CTRL_LAYER_ID_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int width_sub1 : 13;
        unsigned int reserved_0 : 3;
        unsigned int height_sub1 : 13;
        unsigned int reserved_1 : 3;
    } reg;
} DPU_ITF_CH_IMG_SIZE_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int dpp_clip_top : 6;
        unsigned int reserved : 26;
    } reg;
} DPU_ITF_CH_DPP_CLIP_TOP_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int dpp_clip_bottom : 6;
        unsigned int reserved : 26;
    } reg;
} DPU_ITF_CH_DPP_CLIP_BOTTOM_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int dpp_clip_left : 6;
        unsigned int reserved : 26;
    } reg;
} DPU_ITF_CH_DPP_CLIP_LEFT_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int dpp_clip_right : 6;
        unsigned int reserved : 26;
    } reg;
} DPU_ITF_CH_DPP_CLIP_RIGHT_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int dpp_clip_en : 1;
        unsigned int reserved : 31;
    } reg;
} DPU_ITF_CH_DPP_CLIP_EN_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int dpp_clip_dbg : 32;
    } reg;
} DPU_ITF_CH_DPP_CLIP_DBG_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int wb_unpack_post_clip_size_vrt : 13;
        unsigned int reserved_0 : 3;
        unsigned int wb_unpack_post_clip_size_hrz : 13;
        unsigned int reserved_1 : 3;
    } reg;
} DPU_ITF_CH_WB_UNPACK_POST_CLIP_DISP_SIZE_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int wb_unpack_post_clip_right : 6;
        unsigned int reserved : 26;
    } reg;
} DPU_ITF_CH_WB_UNPACK_POST_CLIP_CTL_HRZ_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int wb_unpack_en : 1;
        unsigned int reserved : 31;
    } reg;
} DPU_ITF_CH_WB_UNPACK_EN_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int wb_unpack_vsize : 13;
        unsigned int reserved_0 : 3;
        unsigned int wb_unpack_hsize : 13;
        unsigned int reserved_1 : 3;
    } reg;
} DPU_ITF_CH_WB_UNPACK_SIZE_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int wb_unpack_img_fmt : 1;
        unsigned int reserved : 31;
    } reg;
} DPU_ITF_CH_WB_UNPACK_IMG_FMT_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int dpp_clrbar_en : 1;
        unsigned int dpp_clrbar_mode : 1;
        unsigned int reserved : 22;
        unsigned int dpp_clrbar_width : 8;
    } reg;
} DPU_ITF_CH_DPP_CLRBAR_CTRL_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int dpp_clrbar_1st_clr : 30;
        unsigned int reserved : 2;
    } reg;
} DPU_ITF_CH_DPP_CLRBAR_1ST_CLR_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int dpp_clrbar_2nd_clr : 30;
        unsigned int reserved : 2;
    } reg;
} DPU_ITF_CH_DPP_CLRBAR_2ND_CLR_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int dpp_clrbar_3rd_clr : 30;
        unsigned int reserved : 2;
    } reg;
} DPU_ITF_CH_DPP_CLRBAR_3RD_CLR_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int itfsw_rd_shadow : 1;
        unsigned int reserved : 31;
    } reg;
} DPU_ITF_CH_ITFSW_RD_SHADOW_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int hidic_format : 4;
        unsigned int hidic_mso_en : 1;
        unsigned int hidic_bypass : 1;
        unsigned int reserved : 26;
    } reg;
} DPU_ITF_CH_HIDIC_MODE_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int hidic_height : 16;
        unsigned int hidic_width : 16;
    } reg;
} DPU_ITF_CH_HIDIC_DIMENSION_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int hidic_cmp_reserved : 32;
    } reg;
} DPU_ITF_CH_HIDIC_CMP_RESERVED_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int hidic_dbg_out_hi : 32;
    } reg;
} DPU_ITF_CH_HIDIC_DBG_OUT_HI_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int hidic_dbg_out_lo : 32;
    } reg;
} DPU_ITF_CH_HIDIC_DBG_OUT_LO_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int reg_ctrl_scene_id : 3;
        unsigned int reserved : 29;
    } reg;
} DPU_ITF_CH_REG_CTRL_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int reg_ctrl_flush_en : 1;
        unsigned int reserved : 31;
    } reg;
} DPU_ITF_CH_REG_CTRL_FLUSH_EN_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int reg_ctrl_dbg : 32;
    } reg;
} DPU_ITF_CH_REG_CTRL_DEBUG_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int reg_ctrl_state : 32;
    } reg;
} DPU_ITF_CH_REG_CTRL_STATE_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int pre_ch_data_sel : 3;
        unsigned int reserved : 29;
    } reg;
} DPU_ITF_CH_ITFSW_DATA_SEL_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int colorbar_start : 1;
        unsigned int reserved : 31;
    } reg;
} DPU_ITF_CH_CLRBAR_START_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int itfch_clk_en : 1;
        unsigned int itfch_clk_sel : 1;
        unsigned int reserved : 30;
    } reg;
} DPU_ITF_CH_ITFCH_CLK_CTL_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int cnt_en : 1;
        unsigned int vsync_src_sel_1 : 3;
        unsigned int vsync_src_sel_0 : 3;
        unsigned int cnt_clk_en : 1;
        unsigned int cnt_clk_sel : 1;
        unsigned int reserved : 23;
    } reg;
} DPU_ITF_CH_VSYNC_DELAY_CNT_CTRL_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int vsync_delay_cnt : 32;
    } reg;
} DPU_ITF_CH_VSYNC_DELAY_CNT_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int r_lb_debug0 : 32;
    } reg;
} DPU_ITF_CH_R_LB_DEBUG0_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int r_lb_debug1 : 32;
    } reg;
} DPU_ITF_CH_R_LB_DEBUG1_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int r_lb_debug2 : 32;
    } reg;
} DPU_ITF_CH_R_LB_DEBUG2_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int r_lb_debug3 : 32;
    } reg;
} DPU_ITF_CH_R_LB_DEBUG3_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int reg_default : 1;
        unsigned int reserved : 31;
    } reg;
} DPU_ITF_CH_REG_DEFAULT_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int reg_ctrl_layer_id : 5;
        unsigned int reserved : 27;
    } reg;
} DPU_ITF_CH_REG_CTRL_LAYER_ID_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int vsync_timestamp_h : 32;
    } reg;
} DPU_ITF_CH_VSYNC_TIMESTAMP_H_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int vsync_timestamp_l : 32;
    } reg;
} DPU_ITF_CH_VSYNC_TIMESTAMP_L_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int pipe_sw_sig_ctrl_0 : 8;
        unsigned int reserved : 24;
    } reg;
} DPU_PIPE_SW_SIG_CTRL_0_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int sw_pos_ctrl_sig_en_0 : 1;
        unsigned int reserved : 31;
    } reg;
} DPU_PIPE_SW_SW_POS_CTRL_SIG_EN_0_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int pipe_sw_dat_ctrl_0 : 8;
        unsigned int reserved : 24;
    } reg;
} DPU_PIPE_SW_DAT_CTRL_0_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int sw_pos_ctrl_dat_en_0 : 1;
        unsigned int reserved : 31;
    } reg;
} DPU_PIPE_SW_SW_POS_CTRL_DAT_EN_0_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int nxt_sw_no_pr_0 : 1;
        unsigned int reserved : 31;
    } reg;
} DPU_PIPE_SW_NXT_SW_NO_PR_0_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int pipe_sw_clk_sel_0 : 1;
        unsigned int reserved : 31;
    } reg;
} DPU_PIPE_SW_CLK_SEL_0_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int pipe_sw_secu_gate_0 : 8;
        unsigned int pipe_sw_secu_en_0 : 1;
        unsigned int reserved : 23;
    } reg;
} DPU_PIPE_SW_SECU_GATE_0_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int pipe_sw_sig_ctrl_1 : 8;
        unsigned int reserved : 24;
    } reg;
} DPU_PIPE_SW_SIG_CTRL_1_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int sw_pos_ctrl_sig_en_1 : 1;
        unsigned int reserved : 31;
    } reg;
} DPU_PIPE_SW_SW_POS_CTRL_SIG_EN_1_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int pipe_sw_dat_ctrl_1 : 8;
        unsigned int reserved : 24;
    } reg;
} DPU_PIPE_SW_DAT_CTRL_1_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int sw_pos_ctrl_dat_en_1 : 1;
        unsigned int reserved : 31;
    } reg;
} DPU_PIPE_SW_SW_POS_CTRL_DAT_EN_1_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int nxt_sw_no_pr_1 : 1;
        unsigned int reserved : 31;
    } reg;
} DPU_PIPE_SW_NXT_SW_NO_PR_1_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int pipe_sw_clk_sel_1 : 1;
        unsigned int reserved : 31;
    } reg;
} DPU_PIPE_SW_CLK_SEL_1_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int pipe_sw_secu_gate_1 : 8;
        unsigned int pipe_sw_secu_en_1 : 1;
        unsigned int reserved : 23;
    } reg;
} DPU_PIPE_SW_SECU_GATE_1_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int pipe_sw_sig_ctrl_2 : 8;
        unsigned int reserved : 24;
    } reg;
} DPU_PIPE_SW_SIG_CTRL_2_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int sw_pos_ctrl_sig_en_2 : 1;
        unsigned int reserved : 31;
    } reg;
} DPU_PIPE_SW_SW_POS_CTRL_SIG_EN_2_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int pipe_sw_dat_ctrl_2 : 8;
        unsigned int reserved : 24;
    } reg;
} DPU_PIPE_SW_DAT_CTRL_2_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int sw_pos_ctrl_dat_en_2 : 1;
        unsigned int reserved : 31;
    } reg;
} DPU_PIPE_SW_SW_POS_CTRL_DAT_EN_2_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int nxt_sw_no_pr_2 : 1;
        unsigned int reserved : 31;
    } reg;
} DPU_PIPE_SW_NXT_SW_NO_PR_2_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int pipe_sw_clk_sel_2 : 1;
        unsigned int reserved : 31;
    } reg;
} DPU_PIPE_SW_CLK_SEL_2_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int pipe_sw_secu_gate_2 : 8;
        unsigned int pipe_sw_secu_en_2 : 1;
        unsigned int reserved : 23;
    } reg;
} DPU_PIPE_SW_SECU_GATE_2_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int pipe_sw_sig_ctrl_3 : 8;
        unsigned int reserved : 24;
    } reg;
} DPU_PIPE_SW_SIG_CTRL_3_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int sw_pos_ctrl_sig_en_3 : 1;
        unsigned int reserved : 31;
    } reg;
} DPU_PIPE_SW_SW_POS_CTRL_SIG_EN_3_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int pipe_sw_dat_ctrl_3 : 8;
        unsigned int reserved : 24;
    } reg;
} DPU_PIPE_SW_DAT_CTRL_3_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int sw_pos_ctrl_dat_en_3 : 1;
        unsigned int reserved : 31;
    } reg;
} DPU_PIPE_SW_SW_POS_CTRL_DAT_EN_3_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int nxt_sw_no_pr_3 : 1;
        unsigned int reserved : 31;
    } reg;
} DPU_PIPE_SW_NXT_SW_NO_PR_3_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int pipe_sw_clk_sel_3 : 1;
        unsigned int reserved : 31;
    } reg;
} DPU_PIPE_SW_CLK_SEL_3_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int pipe_sw_secu_gate_3 : 8;
        unsigned int pipe_sw_secu_en_3 : 1;
        unsigned int reserved : 23;
    } reg;
} DPU_PIPE_SW_SECU_GATE_3_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int pipe_sw_sig_ctrl_4 : 8;
        unsigned int reserved : 24;
    } reg;
} DPU_PIPE_SW_SIG_CTRL_4_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int sw_pos_ctrl_sig_en_4 : 1;
        unsigned int reserved : 31;
    } reg;
} DPU_PIPE_SW_SW_POS_CTRL_SIG_EN_4_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int pipe_sw_dat_ctrl_4 : 8;
        unsigned int reserved : 24;
    } reg;
} DPU_PIPE_SW_DAT_CTRL_4_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int sw_pos_ctrl_dat_en_4 : 1;
        unsigned int reserved : 31;
    } reg;
} DPU_PIPE_SW_SW_POS_CTRL_DAT_EN_4_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int nxt_sw_no_pr_4 : 1;
        unsigned int reserved : 31;
    } reg;
} DPU_PIPE_SW_NXT_SW_NO_PR_4_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int pipe_sw_clk_sel_4 : 1;
        unsigned int reserved : 31;
    } reg;
} DPU_PIPE_SW_CLK_SEL_4_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int pipe_sw_secu_gate_4 : 8;
        unsigned int pipe_sw_secu_en_4 : 1;
        unsigned int reserved : 23;
    } reg;
} DPU_PIPE_SW_SECU_GATE_4_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int split_hsize_total : 13;
        unsigned int split_hsize_left : 13;
        unsigned int reserved : 6;
    } reg;
} DPU_PIPE_SW_SPLIT_HSIZE_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int split_swap : 1;
        unsigned int reserved : 31;
    } reg;
} DPU_PIPE_SW_SPLIT_CTL_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int tvpack_vsize_in_0 : 13;
        unsigned int tvpack_hsize_in_0 : 13;
        unsigned int reserved : 6;
    } reg;
} DPU_PIPE_SW_TVPACK_SIZE_IN_0_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int tvpack_vsize_out_0 : 13;
        unsigned int tvpack_hsize_out_0 : 13;
        unsigned int reserved : 6;
    } reg;
} DPU_PIPE_SW_TVPACK_SIZE_OUT_0_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int tvpack_bypass_en_0 : 1;
        unsigned int tvpack_data_ctl_0 : 2;
        unsigned int reserved : 29;
    } reg;
} DPU_PIPE_SW_TVPACK_CTL_0_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int tvpack_vsize_in_2 : 13;
        unsigned int tvpack_hsize_in_2 : 13;
        unsigned int reserved : 6;
    } reg;
} DPU_PIPE_SW_TVPACK_SIZE_IN_2_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int tvpack_vsize_out_2 : 13;
        unsigned int tvpack_hsize_out_2 : 13;
        unsigned int reserved : 6;
    } reg;
} DPU_PIPE_SW_TVPACK_SIZE_OUT_2_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int tvpack_bypass_en_2 : 1;
        unsigned int tvpack_data_ctl_2 : 2;
        unsigned int reserved : 29;
    } reg;
} DPU_PIPE_SW_TVPACK_CTL_2_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int tvpack_vsize_in_3 : 13;
        unsigned int tvpack_hsize_in_3 : 13;
        unsigned int reserved : 6;
    } reg;
} DPU_PIPE_SW_TVPACK_SIZE_IN_3_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int tvpack_vsize_out_3 : 13;
        unsigned int tvpack_hsize_out_3 : 13;
        unsigned int reserved : 6;
    } reg;
} DPU_PIPE_SW_TVPACK_SIZE_OUT_3_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int tvpack_bypass_en_3 : 1;
        unsigned int tvpack_data_ctl_3 : 2;
        unsigned int reserved : 29;
    } reg;
} DPU_PIPE_SW_TVPACK_CTL_3_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int tvpack_vsize_in_4 : 13;
        unsigned int tvpack_hsize_in_4 : 13;
        unsigned int reserved : 6;
    } reg;
} DPU_PIPE_SW_TVPACK_SIZE_IN_4_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int tvpack_vsize_out_4 : 13;
        unsigned int tvpack_hsize_out_4 : 13;
        unsigned int reserved : 6;
    } reg;
} DPU_PIPE_SW_TVPACK_SIZE_OUT_4_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int tvpack_bypass_en_4 : 1;
        unsigned int tvpack_data_ctl_4 : 2;
        unsigned int reserved : 29;
    } reg;
} DPU_PIPE_SW_TVPACK_CTL_4_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int em_trig_line : 13;
        unsigned int reserved : 19;
    } reg;
} DPU_PIPE_SW_INT_EM_CTL_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int int_em_to_ipp : 1;
        unsigned int reserved : 31;
    } reg;
} DPU_PIPE_SW_INT_EM_TO_IPP_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int int_em_to_ipp_mask : 1;
        unsigned int reserved : 31;
    } reg;
} DPU_PIPE_SW_INT_EM_TO_IPP_MASK_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int int_em_to_isp : 1;
        unsigned int reserved : 31;
    } reg;
} DPU_PIPE_SW_INT_EM_TO_ISP_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int int_em_to_isp_mask : 1;
        unsigned int reserved : 31;
    } reg;
} DPU_PIPE_SW_INT_EM_TO_ISP_MASK_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int int_em_to_ivp : 1;
        unsigned int reserved : 31;
    } reg;
} DPU_PIPE_SW_INT_EM_TO_IVP_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int int_em_to_ivp_mask : 1;
        unsigned int reserved : 31;
    } reg;
} DPU_PIPE_SW_INT_EM_TO_IVP_MASK_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int int_em_to_sh : 1;
        unsigned int reserved : 31;
    } reg;
} DPU_PIPE_SW_INT_EM_TO_SH_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int int_em_to_sh_mask : 1;
        unsigned int reserved : 31;
    } reg;
} DPU_PIPE_SW_INT_EM_TO_SH_MASK_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int pipe_sw_rd_shadow : 1;
        unsigned int reserved : 31;
    } reg;
} DPU_PIPE_SW_RD_SHADOW_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int overlap_size : 8;
        unsigned int reserved : 24;
    } reg;
} DPU_PIPE_SW_OVERLAP_SIZE_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int tvpack4_clk_sel : 1;
        unsigned int tvpack3_clk_sel : 1;
        unsigned int tvpack2_clk_sel : 1;
        unsigned int tvpack0_clk_sel : 1;
        unsigned int crossbar_clk_sel : 1;
        unsigned int apb_clk_sel : 1;
        unsigned int reserved : 26;
    } reg;
} DPU_PIPE_SW_CLK_SEL_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int tvpack4_clk_en : 1;
        unsigned int tvpack3_clk_en : 1;
        unsigned int tvpack2_clk_en : 1;
        unsigned int tvpack0_clk_en : 1;
        unsigned int crossbar_clk_en : 1;
        unsigned int reserved : 27;
    } reg;
} DPU_PIPE_SW_CLK_EN_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int reg_default_pre_ch0 : 1;
        unsigned int reg_default_post_ch0 : 1;
        unsigned int reg_default_post_ch2 : 1;
        unsigned int reg_default_post_ch3 : 1;
        unsigned int reg_default_post_ch4 : 1;
        unsigned int reserved : 27;
    } reg;
} DPU_PIPE_SW_REG_DEFAULT_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int uvsamp_top_rd_shadow : 1;
        unsigned int reserved : 31;
    } reg;
} DPU_UVSAMP_RD_SHADOW_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int reg_default : 1;
        unsigned int reserved : 31;
    } reg;
} DPU_UVSAMP_REG_DEFAULT_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int uvsamp_top_clk_sel : 32;
    } reg;
} DPU_UVSAMP_TOP_CLK_SEL_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int uvsamp_top_clk_en : 32;
    } reg;
} DPU_UVSAMP_TOP_CLK_EN_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int reg_ctrl_scene_id : 3;
        unsigned int reserved : 29;
    } reg;
} DPU_UVSAMP_REG_CTRL_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int reg_ctrl_flush_en : 1;
        unsigned int reserved : 31;
    } reg;
} DPU_UVSAMP_REG_CTRL_FLUSH_EN_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int reg_ctrl_dbg : 32;
    } reg;
} DPU_UVSAMP_REG_CTRL_DEBUG_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int reg_ctrl_state : 32;
    } reg;
} DPU_UVSAMP_REG_CTRL_STATE_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int reg_ctrl_layer_id : 5;
        unsigned int reserved : 27;
    } reg;
} DPU_UVSAMP_REG_CTRL_LAYER_ID_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int r_lb_debug0 : 32;
    } reg;
} DPU_UVSAMP_R_LB_DEBUG0_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int r_lb_debug1 : 32;
    } reg;
} DPU_UVSAMP_R_LB_DEBUG1_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int r_lb_debug2 : 32;
    } reg;
} DPU_UVSAMP_R_LB_DEBUG2_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int r_lb_debug3 : 32;
    } reg;
} DPU_UVSAMP_R_LB_DEBUG3_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int w_lb_debug0 : 32;
    } reg;
} DPU_UVSAMP_W_LB_DEBUG0_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int w_lb_debug1 : 32;
    } reg;
} DPU_UVSAMP_W_LB_DEBUG1_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int w_lb_debug2 : 32;
    } reg;
} DPU_UVSAMP_W_LB_DEBUG2_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int w_lb_debug3 : 32;
    } reg;
} DPU_UVSAMP_W_LB_DEBUG3_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int csc_idc0 : 11;
        unsigned int reserved_0: 5;
        unsigned int csc_idc1 : 11;
        unsigned int reserved_1: 5;
    } reg;
} DPU_UVSAMP_CSC_IDC0_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int csc_idc2 : 11;
        unsigned int reserved : 21;
    } reg;
} DPU_UVSAMP_CSC_IDC2_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int csc_odc0 : 11;
        unsigned int reserved_0: 5;
        unsigned int csc_odc1 : 11;
        unsigned int reserved_1: 5;
    } reg;
} DPU_UVSAMP_CSC_ODC0_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int csc_odc2 : 11;
        unsigned int reserved : 21;
    } reg;
} DPU_UVSAMP_CSC_ODC2_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int csc_p00 : 17;
        unsigned int reserved : 15;
    } reg;
} DPU_UVSAMP_CSC_P00_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int csc_p01 : 17;
        unsigned int reserved : 15;
    } reg;
} DPU_UVSAMP_CSC_P01_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int csc_p02 : 17;
        unsigned int reserved : 15;
    } reg;
} DPU_UVSAMP_CSC_P02_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int csc_p10 : 17;
        unsigned int reserved : 15;
    } reg;
} DPU_UVSAMP_CSC_P10_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int csc_p11 : 17;
        unsigned int reserved : 15;
    } reg;
} DPU_UVSAMP_CSC_P11_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int csc_p12 : 17;
        unsigned int reserved : 15;
    } reg;
} DPU_UVSAMP_CSC_P12_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int csc_p20 : 17;
        unsigned int reserved : 15;
    } reg;
} DPU_UVSAMP_CSC_P20_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int csc_p21 : 17;
        unsigned int reserved : 15;
    } reg;
} DPU_UVSAMP_CSC_P21_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int csc_p22 : 17;
        unsigned int reserved : 15;
    } reg;
} DPU_UVSAMP_CSC_P22_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int csc_module_en : 1;
        unsigned int reserved : 31;
    } reg;
} DPU_UVSAMP_CSC_ICG_MODULE_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int csc_mprec : 3;
        unsigned int reserved : 29;
    } reg;
} DPU_UVSAMP_CSC_MPREC_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int uvsamp_mem_ctrl : 4;
        unsigned int reserved : 28;
    } reg;
} DPU_UVSAMP_MEM_CTRL_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int uvsamp_width : 16;
        unsigned int uvsamp_height : 16;
    } reg;
} DPU_UVSAMP_SIZE_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int uvsamp_en : 1;
        unsigned int uvsamp_mode : 1;
        unsigned int uvsamp_ppro_bypass : 1;
        unsigned int uvsamp_bitext_mode : 2;
        unsigned int reserved : 27;
    } reg;
} DPU_UVSAMP_CTRL_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int uvsamp_coeff_1 : 10;
        unsigned int reserved_0 : 6;
        unsigned int uvsamp_coeff_0 : 10;
        unsigned int reserved_1 : 6;
    } reg;
} DPU_UVSAMP_COEF_0_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int uvsamp_coeff_3 : 10;
        unsigned int reserved_0 : 6;
        unsigned int uvsamp_coeff_2 : 10;
        unsigned int reserved_1 : 6;
    } reg;
} DPU_UVSAMP_COEF_1_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int uvsamp_coeff_5 : 10;
        unsigned int reserved_0 : 6;
        unsigned int uvsamp_coeff_4 : 10;
        unsigned int reserved_1 : 6;
    } reg;
} DPU_UVSAMP_COEF_2_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int uvsamp_coeff_7 : 10;
        unsigned int reserved_0 : 6;
        unsigned int uvsamp_coeff_6 : 10;
        unsigned int reserved_1 : 6;
    } reg;
} DPU_UVSAMP_COEF_3_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int uvsamp_dbg : 32;
    } reg;
} DPU_UVSAMP_DEBUG_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int eotp_tx_en : 1;
        unsigned int eotp_rx_en : 1;
        unsigned int bta_en : 1;
        unsigned int ecc_rx_en : 1;
        unsigned int crc_rx_en : 1;
        unsigned int reserved : 27;
    } reg;
} DPU_DSI_PERIP_CHAR_CTRL_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int gen_vcid_rx : 2;
        unsigned int reserved : 30;
    } reg;
} DPU_DSI_APB_READ_VCID_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int cmd_video_mode : 1;
        unsigned int dpi_dualpix_en : 1;
        unsigned int cphy_ppi_dw_sel : 1;
        unsigned int scramb_en : 1;
        unsigned int reserved : 28;
    } reg;
} DPU_DSI_MODE_CTRL_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int gen_dt : 6;
        unsigned int gen_vc : 2;
        unsigned int gen_wc_lsbyte : 8;
        unsigned int gen_wc_msbyte : 8;
        unsigned int gen_vid_lpcmd : 1;
        unsigned int reserved : 7;
    } reg;
} DPU_DSI_APB_WR_LP_HDR_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int gen_pld_b1 : 8;
        unsigned int gen_pld_b2 : 8;
        unsigned int gen_pld_b3 : 8;
        unsigned int gen_pld_b4 : 8;
    } reg;
} DPU_DSI_APB_WR_LP_PLD_DATA_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int gen_cmd_empty : 1;
        unsigned int gen_cmd_full : 1;
        unsigned int gen_pld_w_empty : 1;
        unsigned int gen_pld_w_full : 1;
        unsigned int gen_pld_r_empty : 1;
        unsigned int gen_pld_r_full : 1;
        unsigned int gen_rd_cmd_busy : 1;
        unsigned int reserved_0 : 1;
        unsigned int dbi_cmd_empty : 1;
        unsigned int dbi_cmd_full : 1;
        unsigned int dbi_pld_w_empty : 1;
        unsigned int dbi_pld_w_full : 1;
        unsigned int dbi_pld_r_empty : 1;
        unsigned int dbi_pld_r_full : 1;
        unsigned int dbi_rd_cmd_busy : 1;
        unsigned int gen_cmd_idle : 1;
        unsigned int reserved_1 : 16;
    } reg;
} DPU_DSI_CMD_PLD_BUF_STATUS_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int mode_3d : 2;
        unsigned int format_3d : 2;
        unsigned int second_vsync : 1;
        unsigned int right_first : 1;
        unsigned int reserved_0 : 10;
        unsigned int send_3d_cfg : 1;
        unsigned int reserved_1 : 15;
    } reg;
} DPU_DSI_VID_3D_CTRL_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int dsi_ver : 32;
    } reg;
} DPU_DSI_VER_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int core_shutdownz : 1;
        unsigned int reserved : 31;
    } reg;
} DPU_DSI_POR_CTRL_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int tx_esc_clk_division : 8;
        unsigned int to_clk_division : 8;
        unsigned int pclk_en : 1;
        unsigned int pclk_sel : 1;
        unsigned int dpipclk_en : 1;
        unsigned int dpipclk_sel : 1;
        unsigned int reserved : 12;
    } reg;
} DPU_DSI_CLK_DIV_CTRL_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int dpi_vcid : 2;
        unsigned int reserved : 30;
    } reg;
} DPU_DSI_VIDEO_VCID_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int dpi_color_coding : 4;
        unsigned int reserved_0 : 4;
        unsigned int loosely18_en : 1;
        unsigned int reserved_1 : 23;
    } reg;
} DPU_DSI_VIDEO_COLOR_FORMAT_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int dataen_active_low : 1;
        unsigned int vsync_active_low : 1;
        unsigned int hsync_active_low : 1;
        unsigned int shutd_active_low : 1;
        unsigned int colorm_active_low : 1;
        unsigned int reserved : 27;
    } reg;
} DPU_DSI_VIDEO_POL_CTRL_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int invact_lpcmd_time : 8;
        unsigned int reserved_0 : 8;
        unsigned int outvact_lpcmd_time : 8;
        unsigned int reserved_1 : 8;
    } reg;
} DPU_DSI_VIDEO_MODE_LP_CMD_TIMING_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int edpi_allowed_cmd_size : 16;
        unsigned int reserved : 16;
    } reg;
} DPU_DSI_EDPI_CMD_SIZE_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int tear_fx_en : 1;
        unsigned int ack_rqst_en : 1;
        unsigned int reserved_0 : 6;
        unsigned int gen_sw_0p_tx : 1;
        unsigned int gen_sw_1p_tx : 1;
        unsigned int gen_sw_2p_tx : 1;
        unsigned int gen_sr_0p_tx : 1;
        unsigned int gen_sr_1p_tx : 1;
        unsigned int gen_sr_2p_tx : 1;
        unsigned int gen_lw_tx : 1;
        unsigned int reserved_1 : 1;
        unsigned int dcs_sw_0p_tx : 1;
        unsigned int dcs_sw_1p_tx : 1;
        unsigned int dcs_sr_0p_tx : 1;
        unsigned int dcs_lw_tx : 1;
        unsigned int reserved_2 : 4;
        unsigned int max_rd_pkt_size : 1;
        unsigned int reserved_3 : 7;
    } reg;
} DPU_DSI_CMD_MODE_CTRL_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int vid_mode_type : 2;
        unsigned int reserved_0 : 6;
        unsigned int lp_vsa_en : 1;
        unsigned int lp_vbp_en : 1;
        unsigned int lp_vfp_en : 1;
        unsigned int lp_vact_en : 1;
        unsigned int lp_hbp_en : 1;
        unsigned int lp_hfp_en : 1;
        unsigned int frame_bta_ack_en : 1;
        unsigned int lp_cmd_en : 1;
        unsigned int vpg_en : 1;
        unsigned int reserved_1 : 3;
        unsigned int vpg_mode : 1;
        unsigned int reserved_2 : 3;
        unsigned int vpg_orientation : 1;
        unsigned int reserved_3 : 7;
    } reg;
} DPU_DSI_VIDEO_MODE_CTRL_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int vid_pkt_size : 14;
        unsigned int reserved : 18;
    } reg;
} DPU_DSI_VIDEO_PKT_LEN_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int vid_null_chunks : 13;
        unsigned int reserved : 19;
    } reg;
} DPU_DSI_VIDEO_CHUNK_NUM_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int vid_null_size : 13;
        unsigned int reserved : 19;
    } reg;
} DPU_DSI_VIDEO_NULL_SIZE_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int vid_hsa_time : 12;
        unsigned int reserved : 20;
    } reg;
} DPU_DSI_VIDEO_HSA_NUM_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int vid_hbp_time : 12;
        unsigned int reserved : 20;
    } reg;
} DPU_DSI_VIDEO_HBP_NUM_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int vid_hline_time : 15;
        unsigned int reserved : 17;
    } reg;
} DPU_DSI_VIDEO_HLINE_NUM_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int vsa_lines : 20;
        unsigned int reserved : 12;
    } reg;
} DPU_DSI_VIDEO_VSA_NUM_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int vbp_lines : 20;
        unsigned int reserved : 12;
    } reg;
} DPU_DSI_VIDEO_VBP_NUM_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int vfp_lines : 20;
        unsigned int reserved : 12;
    } reg;
} DPU_DSI_VIDEO_VFP_NUM_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int v_active_lines : 24;
        unsigned int reserved : 8;
    } reg;
} DPU_DSI_VIDEO_VACT_NUM_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int phy_txrequestclkhs : 1;
        unsigned int auto_clklane_ctrl : 1;
        unsigned int cphy_clklane_sel_qst : 1;
        unsigned int reserved : 29;
    } reg;
} DPU_DSI_LP_CLK_CTRL_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int phy_clklp2hs_time : 10;
        unsigned int reserved_0 : 6;
        unsigned int phy_clkhs2lp_time : 10;
        unsigned int reserved_1 : 6;
    } reg;
} DPU_DSI_CLKLANE_TRANS_TIME_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int phy_lp2hs_time : 10;
        unsigned int reserved_0 : 6;
        unsigned int phy_hs2lp_time : 10;
        unsigned int reserved_1 : 6;
    } reg;
} DPU_DSI_DATALANE_TRNAS_TIME_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int phy_shutdownz : 1;
        unsigned int phy_rstz : 1;
        unsigned int phy_enableclk : 1;
        unsigned int phy_forcepll : 1;
        unsigned int reserved : 28;
    } reg;
} DPU_DSI_CDPHY_RST_CTRL_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int n_lanes : 2;
        unsigned int reserved_0 : 6;
        unsigned int phy_stop_wait_time : 8;
        unsigned int reserved_1 : 16;
    } reg;
} DPU_DSI_CDPHY_LANE_NUM_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int phy_txrequlpsclk : 1;
        unsigned int phy_txexitulpsclk : 1;
        unsigned int phy_txrequlpslan : 1;
        unsigned int phy_txexitulpslan : 1;
        unsigned int reserved : 28;
    } reg;
} DPU_DSI_CDPHY_ULPS_CTRL_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int phy_tx_triggers : 4;
        unsigned int reserved : 28;
    } reg;
} DPU_DSI_CDPHY_TX_TRIG_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int phy_lock : 1;
        unsigned int phy_direction : 1;
        unsigned int phy_stopstateclklane : 1;
        unsigned int phy_ulpsactivenotclk : 1;
        unsigned int phy_stopstate0lane : 1;
        unsigned int phy_ulpsactivenot0lane : 1;
        unsigned int phy_rxulpsesc0lane : 1;
        unsigned int phy_stopstate1lane : 1;
        unsigned int phy_ulpsactivenot1lane : 1;
        unsigned int phy_stopstate2lane : 1;
        unsigned int phy_ulpsactivenot2lane : 1;
        unsigned int phy_stopstate3lane : 1;
        unsigned int phy_ulpsactivenot3lane : 1;
        unsigned int reserved : 19;
    } reg;
} DPU_DSI_CDPHY_STATUS_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int phy_testclr : 1;
        unsigned int phy_testclk : 1;
        unsigned int reserved : 30;
    } reg;
} DPU_DSI_CDPHY_TEST_CTRL_0_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int phy_testdin : 8;
        unsigned int phy_testdout : 8;
        unsigned int phy_testen : 1;
        unsigned int reserved : 15;
    } reg;
} DPU_DSI_CDPHY_TEST_CTRL_1_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int ack_with_err_0 : 1;
        unsigned int ack_with_err_1 : 1;
        unsigned int ack_with_err_2 : 1;
        unsigned int ack_with_err_3 : 1;
        unsigned int ack_with_err_4 : 1;
        unsigned int ack_with_err_5 : 1;
        unsigned int ack_with_err_6 : 1;
        unsigned int ack_with_err_7 : 1;
        unsigned int ack_with_err_8 : 1;
        unsigned int ack_with_err_9 : 1;
        unsigned int ack_with_err_10 : 1;
        unsigned int ack_with_err_11 : 1;
        unsigned int ack_with_err_12 : 1;
        unsigned int ack_with_err_13 : 1;
        unsigned int ack_with_err_14 : 1;
        unsigned int ack_with_err_15 : 1;
        unsigned int dphy_errors_0 : 1;
        unsigned int dphy_errors_1 : 1;
        unsigned int dphy_errors_2 : 1;
        unsigned int dphy_errors_3 : 1;
        unsigned int dphy_errors_4 : 1;
        unsigned int reserved : 11;
    } reg;
} DPU_DSI_ERROR_STATUS_0_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int to_hs_tx : 1;
        unsigned int to_lp_rx : 1;
        unsigned int ecc_single_err : 1;
        unsigned int ecc_milti_err : 1;
        unsigned int crc_err : 1;
        unsigned int pkt_size_err : 1;
        unsigned int eopt_err : 1;
        unsigned int dpi_pld_wr_err : 1;
        unsigned int gen_cmd_wr_err : 1;
        unsigned int gen_pld_wr_err : 1;
        unsigned int gen_pld_send_err : 1;
        unsigned int gen_pld_rd_err : 1;
        unsigned int gen_pld_recv_err : 1;
        unsigned int reserved_0 : 5;
        unsigned int dpi_pld_rd_err : 1;
        unsigned int phy_lock_err : 1;
        unsigned int reserved_1 : 12;
    } reg;
} DPU_DSI_ERROR_STATUS_1_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int ack_with_err_msk_0 : 1;
        unsigned int ack_with_err_msk_1 : 1;
        unsigned int ack_with_err_msk_2 : 1;
        unsigned int ack_with_err_msk_3 : 1;
        unsigned int ack_with_err_msk_4 : 1;
        unsigned int ack_with_err_msk_5 : 1;
        unsigned int ack_with_err_msk_6 : 1;
        unsigned int ack_with_err_msk_7 : 1;
        unsigned int ack_with_err_msk_8 : 1;
        unsigned int ack_with_err_msk_9 : 1;
        unsigned int ack_with_err_msk_10 : 1;
        unsigned int ack_with_err_msk_11 : 1;
        unsigned int ack_with_err_msk_12 : 1;
        unsigned int ack_with_err_msk_13 : 1;
        unsigned int ack_with_err_msk_14 : 1;
        unsigned int ack_with_err_msk_15 : 1;
        unsigned int dphy_errors_msk_0 : 1;
        unsigned int dphy_errors_msk_1 : 1;
        unsigned int dphy_errors_msk_2 : 1;
        unsigned int dphy_errors_msk_3 : 1;
        unsigned int dphy_errors_msk_4 : 1;
        unsigned int reserved : 11;
    } reg;
} DPU_DSI_ERROR_MASK_0_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int to_hs_tx_msk : 1;
        unsigned int to_lp_rx_msk : 1;
        unsigned int ecc_single_err_msk : 1;
        unsigned int ecc_milti_err_msk : 1;
        unsigned int crc_err_msk : 1;
        unsigned int pkt_size_err_msk : 1;
        unsigned int eopt_err_msk : 1;
        unsigned int dpi_pld_wr_err_msk : 1;
        unsigned int gen_cmd_wr_err_msk : 1;
        unsigned int gen_pld_wr_err_msk : 1;
        unsigned int gen_pld_send_err_msk : 1;
        unsigned int gen_pld_rd_err_msk : 1;
        unsigned int gen_pld_recv_err_msk : 1;
        unsigned int reserved_0 : 5;
        unsigned int dpi_pld_rd_err_msk : 1;
        unsigned int phy_lock_err_msk : 1;
        unsigned int reserved_1 : 12;
    } reg;
} DPU_DSI_ERROR_MASK_1_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int txskewcalhs : 1;
        unsigned int reserved : 31;
    } reg;
} DPU_DSI_CDPHY_CAL_EN_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int ack_with_err_frc_0 : 1;
        unsigned int ack_with_err_frc_1 : 1;
        unsigned int ack_with_err_frc_2 : 1;
        unsigned int ack_with_err_frc_3 : 1;
        unsigned int ack_with_err_frc_4 : 1;
        unsigned int ack_with_err_frc_5 : 1;
        unsigned int ack_with_err_frc_6 : 1;
        unsigned int ack_with_err_frc_7 : 1;
        unsigned int ack_with_err_frc_8 : 1;
        unsigned int ack_with_err_frc_9 : 1;
        unsigned int ack_with_err_frc_10 : 1;
        unsigned int ack_with_err_frc_11 : 1;
        unsigned int ack_with_err_frc_12 : 1;
        unsigned int ack_with_err_frc_13 : 1;
        unsigned int ack_with_err_frc_14 : 1;
        unsigned int ack_with_err_frc_15 : 1;
        unsigned int dphy_error_frc_0 : 1;
        unsigned int dphy_error_frc_1 : 1;
        unsigned int dphy_error_frc_2 : 1;
        unsigned int dphy_error_frc_3 : 1;
        unsigned int dphy_error_frc_4 : 1;
        unsigned int reserved : 11;
    } reg;
} DPU_DSI_INT_ERROR_FORCE0_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int to_hs_tx_frc : 1;
        unsigned int to_lp_rx_frc : 1;
        unsigned int ecc_single_err_frc : 1;
        unsigned int ecc_milti_err_frc : 1;
        unsigned int crc_err_frc : 1;
        unsigned int pkt_size_err_frc : 1;
        unsigned int eopt_err_frc : 1;
        unsigned int dpi_pld_wr_err_frc : 1;
        unsigned int gen_cmd_wr_err_frc : 1;
        unsigned int gen_pld_wr_err_frc : 1;
        unsigned int gen_pld_send_err_frc : 1;
        unsigned int gen_pld_rd_err_frc : 1;
        unsigned int gen_pld_recev_err_frc : 1;
        unsigned int reserved_0 : 1;
        unsigned int reserved_1 : 1;
        unsigned int reserved_2 : 1;
        unsigned int reserved_3 : 1;
        unsigned int reserved_4 : 1;
        unsigned int dpi_pld_rd_err_frc : 1;
        unsigned int phy_lock_err_frc : 1;
        unsigned int reserved_5 : 12;
    } reg;
} DPU_DSI_INT_ERROR_FORCE1_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int auto_ulps : 1;
        unsigned int reserved_0 : 15;
        unsigned int pll_off_ulps : 1;
        unsigned int reserved_1 : 15;
    } reg;
} DPU_DSI_AUTO_ULPS_MODE_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int ulps_entry_delay : 32;
    } reg;
} DPU_DSI_AUTO_ULPS_ENTRY_DELAY_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int twakeup_clk_div : 16;
        unsigned int twakeup_cnt : 16;
    } reg;
} DPU_DSI_AUTO_ULPS_WAKEUP_TIME_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int compression_mode : 1;
        unsigned int reserved_0 : 7;
        unsigned int compress_algo : 2;
        unsigned int reserved_1 : 6;
        unsigned int pps_sel : 2;
        unsigned int reserved_2 : 6;
        unsigned int vid_lpcmd : 1;
        unsigned int reserved_3 : 7;
    } reg;
} DPU_DSI_DSC_PARA_SET_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int max_rd_time : 15;
        unsigned int reserved : 17;
    } reg;
} DPU_DSI_CDPHY_MAX_RD_TIME_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int auto_ulps_min_time : 12;
        unsigned int reserved : 20;
    } reg;
} DPU_DSI_AUTO_ULPS_MIN_TIME_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int phy_mode : 1;
        unsigned int reserved : 31;
    } reg;
} DPU_DSI_CPHY_OR_DPHY_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int vid_shadow_en : 1;
        unsigned int reserved_0 : 7;
        unsigned int vid_shadow_req : 1;
        unsigned int reserved_1 : 7;
        unsigned int vid_shadow_pin_req : 1;
        unsigned int reserved_2 : 15;
    } reg;
} DPU_DSI_SHADOW_REG_CTRL_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int dpi_vcid_act : 2;
        unsigned int reserved : 30;
    } reg;
} DPU_DSI_VIDEO_VCID_ACT_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int dpi_color_coding_act : 4;
        unsigned int reserved_0 : 4;
        unsigned int loosely18_en_act : 1;
        unsigned int reserved_1 : 18;
        unsigned int reserved_2 : 5;
    } reg;
} DPU_DSI_VIDEO_COLOR_FORMAT_ACT_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int invact_lpcmd_time_act : 8;
        unsigned int reserved_0 : 8;
        unsigned int outvact_lpcmd_time_act : 8;
        unsigned int reserved_1 : 8;
    } reg;
} DPU_DSI_VIDEO_MODE_LP_CMD_TIMING_ACT_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int lprx_to_cnt : 16;
        unsigned int hstx_to_cnt : 16;
    } reg;
} DPU_DSI_TO_TIME_CTRL_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int hs_rd_to_cnt : 16;
        unsigned int reserved : 16;
    } reg;
} DPU_DSI_HS_RD_TO_TIME_CTRL_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int lp_rd_to_cnt : 16;
        unsigned int reserved : 16;
    } reg;
} DPU_DSI_LP_RD_TO_TIME_CTRL_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int hs_wr_to_cnt : 16;
        unsigned int reserved_0 : 8;
        unsigned int presp_to_mode : 1;
        unsigned int reserved_1 : 7;
    } reg;
} DPU_DSI_HS_WR_TO_TIME_CTRL_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int lp_wr_to_cnt : 16;
        unsigned int reserved : 16;
    } reg;
} DPU_DSI_LP_WR_TO_TIME_CTRL_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int bta_to_cnt : 16;
        unsigned int reserved : 16;
    } reg;
} DPU_DSI_BTA_TO_TIME_CTRL_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int vid_mode_type_act : 2;
        unsigned int lp_vsa_en_act : 1;
        unsigned int lp_vbp_en_act : 1;
        unsigned int lp_vfp_en_act : 1;
        unsigned int lp_vact_en_act : 1;
        unsigned int lp_hbp_en_act : 1;
        unsigned int lp_hfp_en_act : 1;
        unsigned int frame_bta_ack_en_act : 1;
        unsigned int lp_cmd_en_act : 1;
        unsigned int reserved : 22;
    } reg;
} DPU_DSI_VIDEO_MODE_CTRL_ACT_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int vid_pkt_size_act : 14;
        unsigned int reserved : 18;
    } reg;
} DPU_DSI_VIDEO_PKT_LEN_ACT_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int vid_null_chunks_act : 13;
        unsigned int reserved : 19;
    } reg;
} DPU_DSI_VIDEO_CHUNK_NUM_ACT_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int vid_null_size_act : 13;
        unsigned int reserved : 19;
    } reg;
} DPU_DSI_VIDEO_NULL_SIZE_ACT_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int vid_has_time_act : 12;
        unsigned int reserved : 20;
    } reg;
} DPU_DSI_VIDEO_HSA_NUM_ACT_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int vid_hbp_time_act : 12;
        unsigned int reserved : 20;
    } reg;
} DPU_DSI_VIDEO_HBP_NUM_ACT_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int vid_hline_time_act : 15;
        unsigned int reserved : 17;
    } reg;
} DPU_DSI_VIDEO_HLINE_NUM_ACT_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int vsa_lines_act : 20;
        unsigned int reserved : 12;
    } reg;
} DPU_DSI_VIDEO_VSA_NUM_ACT_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int vbp_lines_act : 20;
        unsigned int reserved : 12;
    } reg;
} DPU_DSI_VIDEO_VBP_NUM_ACT_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int vfp_lines_act : 20;
        unsigned int reserved : 12;
    } reg;
} DPU_DSI_VIDEO_VFP_NUM_ACT_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int v_active_lines_act : 24;
        unsigned int reserved : 8;
    } reg;
} DPU_DSI_VIDEO_VACT_NUM_ACT_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int mode_3d_act : 2;
        unsigned int format_3d_act : 2;
        unsigned int second_vsync_act : 1;
        unsigned int right_first_act : 1;
        unsigned int reserved_0 : 10;
        unsigned int send_3d_cfg_act : 1;
        unsigned int reserved_1 : 15;
    } reg;
} DPU_DSI_VIDEO_3D_CTRL_ACT_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int pprot_cfg : 1;
        unsigned int reserved : 31;
    } reg;
} DPU_DSI_PPROT_CFG_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int dsi_mem_lpctrl : 4;
        unsigned int reserved : 12;
        unsigned int dss_tpram_ctrl : 16;
    } reg;
} DPU_DSI_MEM_CTRL_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int dsi_pm_ctrl : 31;
        unsigned int reserved : 1;
    } reg;
} DPU_DSI_PM_CTRL_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int vid_delay : 17;
        unsigned int phyfsm_st : 3;
        unsigned int dbips_st : 4;
        unsigned int vidregion : 3;
        unsigned int dpips_st : 5;
    } reg;
} DPU_DSI_DBG0_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int phy_pll_start_time_qst : 16;
        unsigned int reserved : 16;
    } reg;
} DPU_DSI_PHY_PLL_START_TIME_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int dbg_crc_init : 1;
        unsigned int dbg_crc_lane_sel : 2;
        unsigned int dbg_crc_en : 1;
        unsigned int reserved : 28;
    } reg;
} DPU_DSI_DBG_CRC_CTRL_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int dbg_crc_value : 32;
    } reg;
} DPU_DSI_DBG_CRC_VAL_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int secu_cfg_en : 1;
        unsigned int reserved : 31;
    } reg;
} DPU_DSI_SECU_CFG_EN_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int vsync_plr : 1;
        unsigned int hsync_plr : 1;
        unsigned int reserved_0 : 1;
        unsigned int data_en_plr : 1;
        unsigned int reserved_1 : 28;
    } reg;
} DPU_DSI_LDI_PLR_CTRL_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int active_space : 13;
        unsigned int reserved : 19;
    } reg;
} DPU_DSI_LDI_3D_CTRL_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int ldi_en : 1;
        unsigned int reserved_0 : 1;
        unsigned int reserved_1 : 3;
        unsigned int dual_mode_en : 1;
        unsigned int reserved_2 : 7;
        unsigned int dual_mode_ctl : 1;
        unsigned int color_mode : 1;
        unsigned int shutdown : 1;
        unsigned int reserved_3 : 16;
    } reg;
} DPU_DSI_LDI_CTRL_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int de_space_low : 1;
        unsigned int hsync_low : 1;
        unsigned int reserved : 30;
    } reg;
} DPU_DSI_LDI_DE_SPACE_LOW_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int reserved_0 : 1;
        unsigned int dsi_halt_video_en : 1;
        unsigned int dsi_wms_3d_mode : 1;
        unsigned int reserved_1 : 29;
    } reg;
} DPU_DSI_CMD_MOD_CTRL_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int dsi_te_hard_en : 1;
        unsigned int dsi_te0_pin_p : 1;
        unsigned int dsi_te1_pin_p : 1;
        unsigned int dsi_te_hard_sel : 1;
        unsigned int dsi_te_tri_sel : 2;
        unsigned int dsi_te_pin_hd_sel : 2;
        unsigned int dsi_te_mask_en : 1;
        unsigned int dsi_te_mask_dis : 4;
        unsigned int dsi_te_mask_und : 4;
        unsigned int dsi_te_pin_en : 1;
        unsigned int reserved : 14;
    } reg;
} DPU_DSI_TE_CTRL_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int dsi_te0_hs_num : 13;
        unsigned int dsi_te1_hs_num : 13;
        unsigned int reserved : 6;
    } reg;
} DPU_DSI_TE_HS_NUM_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int dsi_te0_hs_wd : 12;
        unsigned int dsi_te1_hs_wd : 12;
        unsigned int reserved : 8;
    } reg;
} DPU_DSI_TE_HS_WD_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int dsi_te0_vs_wd : 12;
        unsigned int dsi_te1_vs_wd : 12;
        unsigned int reserved : 8;
    } reg;
} DPU_DSI_TE_VS_WD_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int frm_msk_en : 1;
        unsigned int reserved_0 : 7;
        unsigned int frm_msk : 8;
        unsigned int frm_unmsk : 8;
        unsigned int reserved_1 : 8;
    } reg;
} DPU_DSI_LDI_FRM_MSK_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int frm_valid_dbg : 32;
    } reg;
} DPU_DSI_FRM_VALID_DBG_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int ldi_vinact_cnt : 13;
        unsigned int reserved : 19;
    } reg;
} DPU_DSI_LDI_VINACT_CNT_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int vsync_delay_dis : 1;
        unsigned int vsync_delay_en : 1;
        unsigned int reserved : 30;
    } reg;
} DPU_DSI_VSYNC_DELAY_CTRL_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int vsync_delay_time : 32;
    } reg;
} DPU_DSI_VSYNC_DELAY_TIME_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int rgb_pat : 6;
        unsigned int reserved : 26;
    } reg;
} DPU_DSI_RGB_PAT_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int dsi_hsize : 13;
        unsigned int reserved : 19;
    } reg;
} DPU_DSI_LDI_DPI0_HRZ_CTRL2_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int dsi_vsize : 13;
        unsigned int reserved : 19;
    } reg;
} DPU_DSI_LDI_VRT_CTRL2_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int dpi_hsize : 13;
        unsigned int reserved : 19;
    } reg;
} DPU_DSI_LDI_DPI0_HRZ_CTRL3_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int dss_sh_mask_int : 31;
        unsigned int reserved : 1;
    } reg;
} DPU_DSI_DSS_SH_MASK_INT_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int vstate : 11;
        unsigned int reserved : 21;
    } reg;
} DPU_DSI_VSTATE_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int dpi0_hstate : 6;
        unsigned int reserved : 26;
    } reg;
} DPU_DSI_DPI0_HSTATE_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int vcount : 20;
        unsigned int reserved : 12;
    } reg;
} DPU_DSI_VCOUNT_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int dpi0_hcount : 13;
        unsigned int reserved : 19;
    } reg;
} DPU_DSI_DPI0_HCOUNT_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int ldi_vinact_cnt_act : 13;
        unsigned int reserved : 19;
    } reg;
} DPU_DSI_LDI_VINACT_CNT_ACT_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int vsync_delay_time_act : 32;
    } reg;
} DPU_DSI_VSYNC_DELAY_TIME_ACT_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int dsi_hsize_act : 13;
        unsigned int reserved : 19;
    } reg;
} DPU_DSI_LDI_DPI0_HRZ_CTRL2_ACT_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int dsi_vsize_act : 13;
        unsigned int reserved : 19;
    } reg;
} DPU_DSI_LDI_VRT_CTRL2_ACT_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int dpi_hsize_act : 13;
        unsigned int reserved : 19;
    } reg;
} DPU_DSI_LDI_DPI0_HRZ_CTRL3_ACT_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int rgb_pat_act : 6;
        unsigned int reserved : 26;
    } reg;
} DPU_DSI_RGB_PAT_ACT_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int dphytx_stopcnt : 32;
    } reg;
} DPU_DSI_DPHYTX_STOPSNT_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int dphytx_ctrl : 16;
        unsigned int reserved : 16;
    } reg;
} DPU_DSI_DPHYTX_CTRL_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int dphytx_trstop_flag : 1;
        unsigned int reserved : 31;
    } reg;
} DPU_DSI_DPHYTX_TRSTOP_FLAG_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int dphytx_status : 32;
    } reg;
} DPU_DSI_DPHYTX_STATUS_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int ldi_cmd_event_sel : 4;
        unsigned int reserved : 28;
    } reg;
} DPU_DSI_LDI_CMD_EVENT_SEL_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int single_frm_update : 1;
        unsigned int reserved : 31;
    } reg;
} DPU_DSI_LDI_FRM_MSK_UP_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int mcu_itf_ints : 31;
        unsigned int reserved : 1;
    } reg;
} DPU_DSI_MCU_ITF_INTS_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int mcu_itf_int_msk : 31;
        unsigned int reserved : 1;
    } reg;
} DPU_DSI_MCU_ITF_INT_MSK_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int cpu_itf_ints : 31;
        unsigned int reserved : 1;
    } reg;
} DPU_DSI_CPU_ITF_INTS_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int cpu_itf_int_msk : 32;
    } reg;
} DPU_DSI_CPU_ITF_INT_MSK_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int pmm_itf_ints : 31;
        unsigned int reserved : 1;
    } reg;
} DPU_DSI_PMM_ITF_INTS_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int pmm_itf_int_msk : 31;
        unsigned int reserved : 1;
    } reg;
} DPU_DSI_PMM_ITF_INT_MSK_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int rgb_pat2 : 6;
        unsigned int reserved : 26;
    } reg;
} DPU_DSI_RGB_PAT2_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int rgb_pat2_act : 6;
        unsigned int reserved : 26;
    } reg;
} DPU_DSI_RGB_PAT2_ACT_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int vsync_delay_mode2_en : 1;
        unsigned int vsync_delay_mode2_quit : 1;
        unsigned int reserved : 30;
    } reg;
} DPU_DSI_VSYNC_DELAY_MODE2_CTRL_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int vsync_delay_mode2_line_time : 20;
        unsigned int reserved : 12;
    } reg;
} DPU_DSI_VSYNC_DELAY_MODE2_DELAY_LINE_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int vsync_delay_mode2_line_time_act : 20;
        unsigned int reserved : 12;
    } reg;
} DPU_DSI_VSYNC_DELAY_MODE2_DELAY_LINE_ACT_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int vsync_delay_mode3_en : 1;
        unsigned int reserved : 31;
    } reg;
} DPU_DSI_VSYNC_DELAY_MODE3_CTRL_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int forbta_line_time : 20;
        unsigned int vfp2_line_time : 12;
    } reg;
} DPU_DSI_VSYNC_DELAY_MODE3_LINE_NUM_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int forbta_line_time_act : 20;
        unsigned int vfp2_line_time_act : 12;
    } reg;
} DPU_DSI_VSYNC_DELAY_MODE3_LINE_NUM_ACT_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int vsync_delay_mode4_en : 1;
        unsigned int reserved : 31;
    } reg;
} DPU_DSI_VSYNC_DELAY_MODE4_CTRL_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int long_time_cnt_en : 1;
        unsigned int em_sync_fsm_en : 1;
        unsigned int reserved : 30;
    } reg;
} DPU_DSI_MODE3_EM_SYNC_FSM_CTRL_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int em_sync_as_quit : 1;
        unsigned int soc_alive : 1;
        unsigned int reserved : 30;
    } reg;
} DPU_DSI_MODE3_EM_SYNC_QUIT_CTRL_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int em_cycle_cnt : 32;
    } reg;
} DPU_DSI_EM_CYCLE_CNT_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int em_cycle_cnt_remainder : 32;
    } reg;
} DPU_DSI_EM_CYCLE_CNT_REMAINDER_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int soc_wake_pre_cnt : 32;
    } reg;
} DPU_DSI_SOC_WAKE_PRE_CNT_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int em_quit_cycle_num_0 : 32;
    } reg;
} DPU_DSI_EM_QUIT_CYCLE_NUM_0_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int em_quit_cycle_num_1 : 32;
    } reg;
} DPU_DSI_EM_QUIT_CYCLE_NUM_1_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int em_quit_cycle_num_2 : 32;
    } reg;
} DPU_DSI_EM_QUIT_CYCLE_NUM_2_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int em_quit_cycle_num_3 : 32;
    } reg;
} DPU_DSI_EM_QUIT_CYCLE_NUM_3_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int em_quit_cycle_num_4 : 32;
    } reg;
} DPU_DSI_EM_QUIT_CYCLE_NUM_4_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int em_quit_cycle_num_5 : 32;
    } reg;
} DPU_DSI_EM_QUIT_CYCLE_NUM_5_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int em_quit_cycle_num_6 : 32;
    } reg;
} DPU_DSI_EM_QUIT_CYCLE_NUM_6_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int em_quit_cycle_num_7 : 32;
    } reg;
} DPU_DSI_EM_QUIT_CYCLE_NUM_7_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int em_quit_valid_0 : 1;
        unsigned int em_quit_valid_1 : 1;
        unsigned int em_quit_valid_2 : 1;
        unsigned int em_quit_valid_3 : 1;
        unsigned int em_quit_valid_4 : 1;
        unsigned int em_quit_valid_5 : 1;
        unsigned int em_quit_valid_6 : 1;
        unsigned int em_quit_valid_7 : 1;
        unsigned int reserved : 24;
    } reg;
} DPU_DSI_EM_QUIT_VALID_CTRL_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int em_cycle_cnt_act : 32;
    } reg;
} DPU_DSI_EM_CYCLE_CNT_ACT_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int em_cycle_cnt_remainder_act : 32;
    } reg;
} DPU_DSI_EM_CYCLE_CNT_REMAINDER_ACT_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int soc_wake_pre_cnt_act : 32;
    } reg;
} DPU_DSI_SOC_WAKE_PRE_CNT_ACT_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int em_quit_cycle_num_0_act : 32;
    } reg;
} DPU_DSI_EM_QUIT_CYCLE_NUM_0_ACT_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int em_quit_cycle_num_1_act : 32;
    } reg;
} DPU_DSI_EM_QUIT_CYCLE_NUM_1_ACT_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int em_quit_cycle_num_2_act : 32;
    } reg;
} DPU_DSI_EM_QUIT_CYCLE_NUM_2_ACT_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int em_quit_cycle_num_3_act : 32;
    } reg;
} DPU_DSI_EM_QUIT_CYCLE_NUM_3_ACT_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int em_quit_cycle_num_4_act : 32;
    } reg;
} DPU_DSI_EM_QUIT_CYCLE_NUM_4_ACT_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int em_quit_cycle_num_5_act : 32;
    } reg;
} DPU_DSI_EM_QUIT_CYCLE_NUM_5_ACT_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int em_quit_cycle_num_6_act : 32;
    } reg;
} DPU_DSI_EM_QUIT_CYCLE_NUM_6_ACT_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int em_quit_cycle_num_7_act : 32;
    } reg;
} DPU_DSI_EM_QUIT_CYCLE_NUM_7_ACT_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int em_quit_valid_0_act : 1;
        unsigned int em_quit_valid_1_act : 1;
        unsigned int em_quit_valid_2_act : 1;
        unsigned int em_quit_valid_3_act : 1;
        unsigned int em_quit_valid_4_act : 1;
        unsigned int em_quit_valid_5_act : 1;
        unsigned int em_quit_valid_6_act : 1;
        unsigned int em_quit_valid_7_act : 1;
        unsigned int reserved : 24;
    } reg;
} DPU_DSI_EM_QUIT_VALID_CTRL_ACT_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int em_quit_cnt : 3;
        unsigned int em_quit_cnt_long_time : 16;
        unsigned int reserved : 13;
    } reg;
} DPU_DSI_EM_QUIT_CNT_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int em_sync_reg_cfg_done : 1;
        unsigned int em_sync_reg_update_req : 1;
        unsigned int reserved : 30;
    } reg;
} DPU_DSI_EM_SYNC_REG_CTRL_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int long_time_cnt_en_act : 1;
        unsigned int em_sync_fsm_en_act : 1;
        unsigned int reserved : 30;
    } reg;
} DPU_DSI_MODE3_EM_SYNC_FSM_CTRL_ACT_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int wait_cdphy_line_time : 20;
        unsigned int frm_end_line_time : 12;
    } reg;
} DPU_DSI_VSYNC_DELAY_MODE4_LINE_NUM_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int wait_cdphy_line_time_act : 20;
        unsigned int frm_end_line_time_act : 12;
    } reg;
} DPU_DSI_VSYNC_DELAY_MODE4_LINE_NUM_ACT_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int em_sync_state_cur : 5;
        unsigned int reserved : 27;
    } reg;
} DPU_DSI_EM_SYNC_STATE_CUR_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int em_sync_cnt : 32;
    } reg;
} DPU_DSI_EM_SYNC_CNT_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int em_sync_cnt_lt : 32;
    } reg;
} DPU_DSI_EM_SYNC_CNT_LT_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int em_quit_src_sel : 1;
        unsigned int reserved : 31;
    } reg;
} DPU_DSI_EM_QUIT_SRC_SEL_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int shutdownz2idle_cycle : 8;
        unsigned int txreqhs2shutdownz_cycle : 8;
        unsigned int frmend2txreqhs_cycle : 8;
        unsigned int reserved : 8;
    } reg;
} DPU_DSI_CDPHY_SHUTDOWN_TIMING_CTRL_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int txreqhs2idle_cycle : 7;
        unsigned int resetz2txreqhs_cycle : 13;
        unsigned int shutdownz2resetz_cycle : 4;
        unsigned int quit2shutdownz_cycle : 8;
    } reg;
} DPU_DSI_CDPHY_WAKEUP_TIMING_CTRL_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int cdphy_timing_manual_en : 1;
        unsigned int cdphy_timing_autoulps_en : 1;
        unsigned int reserved : 30;
    } reg;
} DPU_DSI_CDPHY_TIMING_CTRL_EN_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int gen_hp_dt : 6;
        unsigned int gen_hp_vc : 2;
        unsigned int gen_hp_wc_lsbyte : 8;
        unsigned int gen_hp_wc_msbyte : 8;
        unsigned int gen_hp_vid_lpcmd : 1;
        unsigned int reserved : 7;
    } reg;
} DPU_DSI_GEN_HP_HDR_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int gen_hp_pld_b1 : 8;
        unsigned int gen_hp_pld_b2 : 8;
        unsigned int gen_hp_pld_b3 : 8;
        unsigned int gen_hp_pld_b4 : 8;
    } reg;
} DPU_DSI_GEN_HP_PLD_DATA_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int hp_apb_cnt_clr : 1;
        unsigned int hp_cross_frm_ctrl : 1;
        unsigned int hp_dcs_send_timing_ctrl : 3;
        unsigned int reserved : 27;
    } reg;
} DPU_DSI_HP_CMD_FIFO_CTRL_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int hp_cmd_fifo_wr_cnt : 8;
        unsigned int hp_apb_dcs_cnt : 22;
        unsigned int reserved : 2;
    } reg;
} DPU_DSI_HP_CMD_FIFO_STATUS_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int hp_pld_fifo_wr_cnt : 8;
        unsigned int hp_apb_pld_cnt : 22;
        unsigned int reserved : 2;
    } reg;
} DPU_DSI_HP_PLD_FIFO_STATUS_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int lp_apb_cnt_clr : 1;
        unsigned int lp_cross_frm_ctrl : 1;
        unsigned int lp_dcs_send_timing_ctrl : 3;
        unsigned int reserved : 27;
    } reg;
} DPU_DSI_LP_CMD_FIFO_CTRL_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int lp_cmd_fifo_wr_cnt : 8;
        unsigned int lp_apb_dcs_cnt : 22;
        unsigned int reserved : 2;
    } reg;
} DPU_DSI_LP_CMD_FIFO_STATUS_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int lp_pld_fifo_wr_cnt : 8;
        unsigned int lp_apb_pld_cnt : 22;
        unsigned int reserved : 2;
    } reg;
} DPU_DSI_LP_PLD_FIFO_STATUS_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int frm_valid_dbg2 : 32;
    } reg;
} DPU_DSI_FRM_VALID_DBG2_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int iomcu_itf_ints : 31;
        unsigned int reserved : 1;
    } reg;
} DPU_DSI_IOMCU_ITF_INT_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int iomcu_itf_int_msk : 31;
        unsigned int reserved : 1;
    } reg;
} DPU_DSI_IOMCU_ITF_INT_MASK_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int clk_ctrl_delay_cnt : 24;
        unsigned int reserved : 8;
    } reg;
} DPU_DSI_ULPS_CLKLANE_DELAY_CNT_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int dsi_te_hard_en_2 : 1;
        unsigned int dsi_te0_pin_p_2 : 1;
        unsigned int dsi_te1_pin_p_2 : 1;
        unsigned int dsi_te_hard_sel_2 : 1;
        unsigned int dsi_te_tri_sel_2 : 2;
        unsigned int dsi_te_pin_hd_sel_2 : 2;
        unsigned int dsi_te_mask_en_2 : 1;
        unsigned int dsi_te_mask_dis_2 : 4;
        unsigned int dsi_te_mask_und_2 : 4;
        unsigned int dsi_te_pin_en_2 : 1;
        unsigned int reserved : 14;
    } reg;
} DPU_DSI_TE_CTRL_2_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int dsi_te0_hs_num_2 : 13;
        unsigned int dsi_te1_hs_num_2 : 13;
        unsigned int reserved : 6;
    } reg;
} DPU_DSI_TE_HS_NUM_2_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int dsi_te0_hs_wd_2 : 12;
        unsigned int dsi_te1_hs_wd_2 : 12;
        unsigned int reserved : 8;
    } reg;
} DPU_DSI_TE_HS_WD_2_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int dsi_te0_vs_wd_2 : 12;
        unsigned int dsi_te1_vs_wd_2 : 12;
        unsigned int reserved : 8;
    } reg;
} DPU_DSI_TE_VS_WD_2_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int use_same_reload_scheme : 1;
        unsigned int use_one_quit_mode : 1;
        unsigned int use_ulps_scheme : 1;
        unsigned int reserved : 29;
    } reg;
} DPU_DSI_MODE3_EXTRA_CTRL_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int hp_gen_cmd_empty : 1;
        unsigned int hp_gen_cmd_full : 1;
        unsigned int hp_gen_pld_w_empty : 1;
        unsigned int hp_gen_pld_w_full : 1;
        unsigned int hp_gen_cmd_idle : 1;
        unsigned int reserved : 27;
    } reg;
} DPU_DSI_HP_CMD_PKT_STATUS_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int vbp_line_left_overlap : 16;
        unsigned int vbp_line_left_allow : 16;
    } reg;
} DPU_DSI_VBP_LINE_LEFT_SETTING_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int vfp_line_left_allow : 16;
        unsigned int reserved : 16;
    } reg;
} DPU_DSI_VFP_LINE_LEFT_SETTING_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int cmd_hs_pkt_discontin_enable : 1;
        unsigned int reserved : 31;
    } reg;
} DPU_DSI_CMD_HS_PKT_DISCONTIN_CTRL_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int dsi_te_hard_en_3 : 1;
        unsigned int dsi_te0_pin_p_3 : 1;
        unsigned int dsi_te1_pin_p_3 : 1;
        unsigned int dsi_te_hard_sel_3 : 1;
        unsigned int dsi_te_tri_sel_3 : 2;
        unsigned int dsi_te_pin_hd_sel_3 : 2;
        unsigned int dsi_te_mask_en_3 : 1;
        unsigned int dsi_te_mask_dis_3 : 4;
        unsigned int dsi_te_mask_und_3 : 4;
        unsigned int dsi_te_pin_en_3 : 1;
        unsigned int reserved : 14;
    } reg;
} DPU_DSI_TE_CTRL_3_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int dsi_te0_hs_num_3 : 13;
        unsigned int dsi_te1_hs_num_3 : 13;
        unsigned int reserved : 6;
    } reg;
} DPU_DSI_TE_HS_NUM_3_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int dsi_te0_hs_wd_3 : 12;
        unsigned int dsi_te1_hs_wd_3 : 12;
        unsigned int reserved : 8;
    } reg;
} DPU_DSI_TE_HS_WD_3_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int dsi_te0_vs_wd_3 : 12;
        unsigned int dsi_te1_vs_wd_3 : 12;
        unsigned int reserved : 8;
    } reg;
} DPU_DSI_TE_VS_WD_3_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int ddsp_mode : 1;
        unsigned int ddsp_en : 1;
        unsigned int reserved : 30;
    } reg;
} DPU_DSI_GLB_DDSP_CTRL_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int lane_sel : 1;
        unsigned int reserved : 31;
    } reg;
} DPU_DSI_GLB_LANE_CTRL_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int dsi_host_reset_en : 3;
        unsigned int reserved : 29;
    } reg;
} DPU_DSI_GLB_DSI_RESET_EN_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int doze_dsi_sel : 2;
        unsigned int reserved : 30;
    } reg;
} DPU_DSI_GLB_DOZE_DSI_SEL_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int intr_sleep_idle_lmp3_dsi2 : 1;
        unsigned int intr_sleep_idle_lmp3_dsi0 : 1;
        unsigned int reserved : 30;
    } reg;
} DPU_DSI_GLB_INTR_SLEEP_IDLE_LPM3_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int intr_sleep_idle_lmp3_dsi2_mask : 1;
        unsigned int intr_sleep_idle_lmp3_dsi0_mask : 1;
        unsigned int reserved : 30;
    } reg;
} DPU_DSI_GLB_INTR_SLEEP_IDLE_LPM3_MASK_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int tp_sel : 3;
        unsigned int reserved : 29;
    } reg;
} DPU_DSI_GLB_TP_SEL_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int gen_dual_dt : 6;
        unsigned int gen_dual_vc : 2;
        unsigned int gen_dual_wc_lsbyte : 8;
        unsigned int gen_dual_wc_msbyte : 8;
        unsigned int gen_dual_vid_lpcmd : 1;
        unsigned int no_use : 7;
    } reg;
} DPU_DSI_GLB_GEN_DUAL_HDR_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int gen_dual_pld_b1 : 8;
        unsigned int gen_dual_pld_b2 : 8;
        unsigned int gen_dual_pld_b3 : 8;
        unsigned int gen_dual_pld_b4 : 8;
    } reg;
} DPU_DSI_GLB_GEN_DUAL_PLD_DATA_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int dual_reg_wr_sel : 2;
        unsigned int reserved : 30;
    } reg;
} DPU_DSI_GLB_DUAL_REG_WR_SEL_UNION;
#endif
#ifdef __cplusplus
    #if __cplusplus
        }
    #endif
#endif
#endif
