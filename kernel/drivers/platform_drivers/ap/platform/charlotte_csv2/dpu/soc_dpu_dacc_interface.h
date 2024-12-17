#ifndef __SOC_DACC_INTERFACE_H__
#define __SOC_DACC_INTERFACE_H__ 
#ifdef __cplusplus
    #if __cplusplus
        extern "C" {
    #endif
#endif
#define SOC_DACC_SW_EXT_INT_REG_ADDR(base) ((base) + (0x0000))
#define SOC_DACC_SW_EXT_INT_MASK_REG_ADDR(base) ((base) + (0x0004))
#define SOC_DACC_POR_RESET_PC_REG_ADDR(base) ((base) + (0x0008))
#define SOC_DACC_CORE_CONFIG_REG_ADDR(base) ((base) + (0x000C))
#define SOC_DACC_CORE_STATUS_REG_ADDR(base) ((base) + (0x0010))
#define SOC_DACC_CORE_MON_PC_REG_ADDR(base) ((base) + (0x0014))
#define SOC_DACC_CORE_MON_LR_REG_ADDR(base) ((base) + (0x0018))
#define SOC_DACC_CTL_LDI_SEL0_REG_ADDR(base) ((base) + (0x001C))
#define SOC_DACC_CTL_LDI_SEL1_REG_ADDR(base) ((base) + (0x0020))
#define SOC_DACC_CTL_LDI_SEL2_REG_ADDR(base) ((base) + (0x0024))
#define SOC_DACC_CTL_LDI_SEL3_REG_ADDR(base) ((base) + (0x0028))
#define SOC_DACC_CTL_WCH_SEL0_ADDR(base) ((base) + (0x002C))
#define SOC_DACC_CTL_WCH_SEL1_ADDR(base) ((base) + (0x0030))
#define SOC_DACC_CTL_WCH_SEL2_ADDR(base) ((base) + (0x0034))
#define SOC_DACC_DBG_REG0_ADDR(base) ((base) + (0x0038))
#define SOC_DACC_DBG_SEL_REG_ADDR(base) ((base) + (0x003C))
#define SOC_DACC_MINIBUS_1T01_CKG_EN_ADDR(base) ((base) + (0x0040))
#define SOC_DACC_MINIBUS_1T01_STATE_ADDR(base) ((base) + (0x0044))
#define SOC_DACC_CKG_EN_ADDR(base) ((base) + (0x0048))
#define SOC_DACC_WLT_EN_ADDR(base) ((base) + (0x0060))
#define SOC_DACC_WLT_SLICE0_DDR_ADDR0_ADDR(base) ((base) + (0x0064))
#define SOC_DACC_WLT_SLICE0_DDR_ADDR1_ADDR(base) ((base) + (0x0068))
#define SOC_DACC_WLT_SLICE1_DDR_ADDR0_ADDR(base) ((base) + (0x006c))
#define SOC_DACC_WLT_SLICE1_DDR_ADDR1_ADDR(base) ((base) + (0x0070))
#define SOC_DACC_WLT_SLICE2_DDR_ADDR0_ADDR(base) ((base) + (0x0074))
#define SOC_DACC_WLT_SLICE2_DDR_ADDR1_ADDR(base) ((base) + (0x0078))
#define SOC_DACC_WLT_SLICE3_DDR_ADDR0_ADDR(base) ((base) + (0x007c))
#define SOC_DACC_WLT_SLICE3_DDR_ADDR1_ADDR(base) ((base) + (0x0080))
#define SOC_DACC_WLT_SLICE0_ADDR0_H_ADDR(base) ((base) + (0x0084))
#define SOC_DACC_WLT_SLICE0_ADDR0_L_ADDR(base) ((base) + (0x0088))
#define SOC_DACC_WLT_SLICE0_ADDR1_L_ADDR(base) ((base) + (0x008c))
#define SOC_DACC_WLT_SLICE0_ADDR2_L_ADDR(base) ((base) + (0x0094))
#define SOC_DACC_WLT_SLICE0_ADDR3_L_ADDR(base) ((base) + (0x009c))
#define SOC_DACC_WLT_SLICE1_ADDR0_H_ADDR(base) ((base) + (0x00a4))
#define SOC_DACC_WLT_SLICE1_ADDR0_L_ADDR(base) ((base) + (0x00a8))
#define SOC_DACC_WLT_SLICE1_ADDR1_L_ADDR(base) ((base) + (0x00ac))
#define SOC_DACC_WLT_SLICE1_ADDR2_L_ADDR(base) ((base) + (0x00b4))
#define SOC_DACC_WLT_SLICE1_ADDR3_L_ADDR(base) ((base) + (0x00bc))
#define SOC_DACC_WLT_SLICE2_ADDR0_H_ADDR(base) ((base) + (0x00c4))
#define SOC_DACC_WLT_SLICE2_ADDR0_L_ADDR(base) ((base) + (0x00c8))
#define SOC_DACC_WLT_SLICE2_ADDR1_L_ADDR(base) ((base) + (0x00cc))
#define SOC_DACC_WLT_SLICE2_ADDR2_L_ADDR(base) ((base) + (0x00d4))
#define SOC_DACC_WLT_SLICE2_ADDR3_L_ADDR(base) ((base) + (0x00dc))
#define SOC_DACC_WLT_SLICE3_ADDR0_H_ADDR(base) ((base) + (0x00e4))
#define SOC_DACC_WLT_SLICE3_ADDR0_L_ADDR(base) ((base) + (0x00e8))
#define SOC_DACC_WLT_SLICE3_ADDR1_L_ADDR(base) ((base) + (0x00ec))
#define SOC_DACC_WLT_SLICE3_ADDR2_L_ADDR(base) ((base) + (0x00f4))
#define SOC_DACC_WLT_SLICE3_ADDR3_L_ADDR(base) ((base) + (0x00fc))
#define SOC_DACC_WLT_SLICE_ROW_ADDR(base) ((base) + (0x0104))
#define SOC_DACC_WLT_CMDLIST_REFRESH_OFFSET_ADDR(base) ((base) + (0x0108))
#define SOC_DACC_WCH_FRM_BLK_END_ADDR(base) ((base) + (0x010c))
#define SOC_DACC_UVUP_SCENE_ASSIGN_ADDR(base) ((base) + (0x0110))
#define SOC_DACC_ARSR0_SCENE_ASSIGN_ADDR(base) ((base) + (0x0114))
#define SOC_DACC_ARSR1_SCENE_ASSIGN_ADDR(base) ((base) + (0x0118))
#define SOC_DACC_VSCF0_SCENE_ASSIGN_ADDR(base) ((base) + (0x011c))
#define SOC_DACC_VSCF1_SCENE_ASSIGN_ADDR(base) ((base) + (0x0120))
#define SOC_DACC_HDR_SCENE_ASSIGN_ADDR(base) ((base) + (0x0124))
#define SOC_DACC_CLD0_SCENE_ASSIGN_ADDR(base) ((base) + (0x0128))
#define SOC_DACC_CLD1_SCENE_ASSIGN_ADDR(base) ((base) + (0x012c))
#define SOC_DACC_SROT0_SCENE_ASSIGN_ADDR(base) ((base) + (0x0130))
#define SOC_DACC_SROT1_SCENE_ASSIGN_ADDR(base) ((base) + (0x0134))
#define SOC_DACC_DMA_SEND_AHEAD_OFFSET_ADDR(base) ((base) + (0x0138))
#define SOC_DACC_MASK_STA_FLAG_ADDR(base) ((base) + (0x0300))
#define SOC_DACC_MASK_AREA00_START_IDX_ADDR(base) ((base) + (0x0304))
#define SOC_DACC_MASK_AREA00_END_IDX_ADDR(base) ((base) + (0x0308))
#define SOC_DACC_MASK_AREAFF_START_IDX_ADDR(base) ((base) + (0x030c))
#define SOC_DACC_MASK_AREAFF_END_IDX_ADDR(base) ((base) + (0x0310))
#define SOC_DACC_CLK_SEL_REG_ADDR(base,s12) ((base) + (0x0400+(s12)*0x400))
#define SOC_DACC_CLK_EN_REG_ADDR(base,s12) ((base) + (0x0404+(s12)*0x400))
#define SOC_DACC_CTL_SW_EN_RELOAD_ADDR(base,s12) ((base) + (0x0408+(s12)*0x400))
#define SOC_DACC_CTL_SW_EN_REG_ADDR(base,s12) ((base) + (0x040C+(s12)*0x400))
#define SOC_DACC_CTL_SW_START_REG_ADDR(base,s12) ((base) + (0x0410+(s12)*0x400))
#define SOC_DACC_CTL_SW_INTR_EN_REG_ADDR(base,s12) ((base) + (0x0414+(s12)*0x400))
#define SOC_DACC_CTL_SELF_REC_EN_REG_ADDR(base,s12) ((base) + (0x0418+(s12)*0x400))
#define SOC_DACC_CTL_ST_FRM_SEL0_REG_ADDR(base,s12) ((base) + (0x041C+(s12)*0x400))
#define SOC_DACC_CTL_CFG_MODE0_ADDR(base,s12) ((base) + (0x0424+(s12)*0x400))
#define SOC_DACC_CFG_RD_SHADOW_ADDR(base,s12) ((base) + (0x0428+(s12)*0x400))
#define SOC_DACC_CTL_CLEAR_TIMEOUT_EN_REG_ADDR(base,s12) ((base) + (0x042C+(s12)*0x400))
#define SOC_DACC_CTL_CLEAR_TIMEOUT_THR_REG_ADDR(base,s12) ((base) + (0x0430+(s12)*0x400))
#define SOC_DACC_CLR_START_REG_ADDR(base,s12) ((base) + (0x0434+(s12)*0x400))
#define SOC_DACC_DCMD_LAYER_INFO_OFFSET_ADDR(base,s12) ((base) + (0x0440+(s12)*0x400))
#define SOC_DACC_CLR_RISCV_START_ADDR(base,s12) ((base) + (0x0444+(s12)*0x400))
#define SOC_DACC_CLR_TIMEOUT_INTS_MASK_REG_ADDR(base,s12) ((base) + (0x0448+(s12)*0x400))
#define SOC_DACC_CLR_TIMEOUT_STATUS_REG_ADDR(base,s12) ((base) + (0x044C+(s12)*0x400))
#define SOC_DACC_SCMD_START_REG_ADDR(base,s12) ((base) + (0x0450+(s12)*0x400))
#define SOC_DACC_DCMD_START_REG_ADDR(base,s12) ((base) + (0x0454+(s12)*0x400))
#define SOC_DACC_SCMD_DM_ADDR_REG_ADDR(base,s12) ((base) + (0x0458+(s12)*0x400))
#define SOC_DACC_DCMD_DM_ADDR_REG_ADDR(base,s12) ((base) + (0x045C+(s12)*0x400))
#define SOC_DACC_SCMD_NUM_REG_ADDR(base,s12) ((base) + (0x0460+(s12)*0x400))
#define SOC_DACC_SCMD_INTS_MASK_REG_ADDR(base,s12) ((base) + (0x0468+(s12)*0x400))
#define SOC_DACC_SCMD_STATUS_REG_ADDR(base,s12) ((base) + (0x046C+(s12)*0x400))
#define SOC_DACC_DCMD_INTS_MASK_REG_ADDR(base,s12) ((base) + (0x0474+(s12)*0x400))
#define SOC_DACC_DCMD_STATUS_REG_ADDR(base,s12) ((base) + (0x0478+(s12)*0x400))
#define SOC_DACC_LAYER_NUM_REG_ADDR(base,s12) ((base) + (0x047C+(s12)*0x400))
#define SOC_DACC_SW_CMD_REG0_ADDR(base,s12) ((base) + (0x0480+(s12)*0x400))
#define SOC_DACC_SW_CMD_REG1_ADDR(base,s12) ((base) + (0x0484+(s12)*0x400))
#define SOC_DACC_SW_CMD_TX_REG_ADDR(base,s12) ((base) + (0x0488+(s12)*0x400))
#define SOC_DACC_CTL_DBG0_REG_ADDR(base,s12) ((base) + (0x048C+(s12)*0x400))
#define SOC_DACC_CTL_DBG1_REG_ADDR(base,s12) ((base) + (0x0490+(s12)*0x400))
#define SOC_DACC_CTL_DBG2_REG_ADDR(base,s12) ((base) + (0x0494+(s12)*0x400))
#define SOC_DACC_SCEN_SECURE_REG_ADDR(base,s12) ((base) + (0x500+(s12)*0x400))
#define SOC_DACC_SCEN_SECURE_DM_ADDR_ADDR(base,s12) ((base) + (0x0504+(s12)*0x400))
#define SOC_DACC_SCEN_SECURE_RSV_ADDR(base,s12) ((base) + (0x0508+(s12)*0x400))
#define SOC_DACC_SCEN_SECURE_CMD_EN_ADDR(base,s12) ((base) + (0x050c+(s12)*0x400))
#define SOC_DACC_PING_CTL0_LAYER0_SECU_ADDR(base,s12) ((base) + (0x0510+(s12)*0x400))
#define SOC_DACC_PING_CTL0_LAYER1_SECU_ADDR(base,s12) ((base) + (0x0514+(s12)*0x400))
#define SOC_DACC_SEC_PAY_ABNORMAL_INT_ADDR(base,s12) ((base) + (0x0518+(s12)*0x400))
#define SOC_DACC_DBG_DEMURA_RLST_CNT_ADDR(base,s12) ((base) + (0x0520+(s12)*0x400))
#define SOC_DACC_DBG_SCL_RLST_CNT_01_ADDR(base,s12) ((base) + (0x0524+(s12)*0x400))
#define SOC_DACC_DBG_SCL_RLST_CNT_23_ADDR(base,s12) ((base) + (0x0528+(s12)*0x400))
#define SOC_DACC_DBG_UVUP_RLST_YCNT_ADDR(base,s12) ((base) + (0x052c+(s12)*0x400))
#define SOC_DACC_DBG_CST_ALL_ADDR(base,s12) ((base) + (0x0530+(s12)*0x400))
#define SOC_DACC_DBG_FSM_MISC_ADDR(base,s12) ((base) + (0x0534+(s12)*0x400))
#define SOC_DACC_DBG_BUSY_REQ_MISC_ADDR(base,s12) ((base) + (0x0538+(s12)*0x400))
#define SOC_DACC_DBG_RESERVED_0_ADDR(base,s12) ((base) + (0x053c+(s12)*0x400))
#define SOC_DACC_DBG_RESERVED_1_ADDR(base,s12) ((base) + (0x0540+(s12)*0x400))
#define SOC_DACC_JOINT_RALATION_0_ADDR(base,ly) ((base) + (0x0200+0x80*(ly)))
#define SOC_DACC_JOINT_RALATION_1_ADDR(base,ly) ((base) + (0x0204+0x80*(ly)))
#define SOC_DACC_LAYER_KEY_FEATURE_ADDR(base,ly) ((base) + (0x0210+0x80*(ly)))
#define SOC_DACC_MOD_IDX_TABLE_0_ADDR(base,ly) ((base) + (0x0214+0x80*(ly)))
#define SOC_DACC_MOD_IDX_TABLE_1_ADDR(base,ly) ((base) + (0x0218+0x80*(ly)))
#define SOC_DACC_OV_LINEREG_PARA_ADDR(base) ((base) + (0x0000))
#define SOC_DACC_HDR_LINEREG_PARA0_ADDR(base) ((base) + (0x0004))
#define SOC_DACC_HDR_LINEREG_PARA1_ADDR(base) ((base) + (0x0008))
#define SOC_DACC_HDR_LINEREG_PARA2_ADDR(base) ((base) + (0x000c))
#define SOC_DACC_UVUP_LINEREG_PARA0_ADDR(base) ((base) + (0x0010))
#define SOC_DACC_UVUP_LINEREG_PARA1_ADDR(base) ((base) + (0x0014))
#define SOC_DACC_UVUP_LINEREG_PARA2_ADDR(base) ((base) + (0x0018))
#define SOC_DACC_UVUP_LINEREG_PARA3_ADDR(base) ((base) + (0x001C))
#define SOC_DACC_SROT_BLK_WIDTH_ADDR(base,sr) ((base) + (0x0020+(sr)*0x20))
#define SOC_DACC_SROT_BLK_HEIGHT_ADDR(base,sr) ((base) + (0x0024+(sr)*0x20))
#define SOC_DACC_SROT_BLOCK_CNT_ADDR(base,sr) ((base) + (0x0028+(sr)*0x20))
#define SOC_DACC_SROT_RESERVED_ADDR(base,sr) ((base) + (0x002c+(sr)*0x20))
#define SOC_DACC_SROT_LINEREG_PARA0_ADDR(base,sr) ((base) + (0x0030+(sr)*0x20))
#define SOC_DACC_SROT_LINEREG_PARA1_ADDR(base,sr) ((base) + (0x0034+(sr)*0x20))
#define SOC_DACC_SROT_LINEREG_PARA2_ADDR(base,sr) ((base) + (0x0038+(sr)*0x20))
#define SOC_DACC_SROT_LINEREG_PARA3_ADDR(base,sr) ((base) + (0x003c+(sr)*0x20))
#define SOC_DACC_CLD_LINEREG_PARA0_ADDR(base,cl) ((base) + (0x0060+(cl)*0x10))
#define SOC_DACC_CLD_LINEREG_PARA1_ADDR(base,cl) ((base) + (0x0064+(cl)*0x10))
#define SOC_DACC_CLD_LINEREG_PARA2_ADDR(base,cl) ((base) + (0x0068+(cl)*0x10))
#define SOC_DACC_CLD_RESERVED_ADDR(base,cl) ((base) + (0x006c+(cl)*0x10))
#define SOC_DACC_SCL_LINEREG_PARA0_ADDR(base,scl) ((base) + (0x0080+(scl)*0x10))
#define SOC_DACC_SCL_LINEREG_PARA1_ADDR(base,scl) ((base) + (0x0084+(scl)*0x10))
#define SOC_DACC_SCL_LINEREG_PARA2_ADDR(base,scl) ((base) + (0x0088+(scl)*0x10))
#define SOC_DACC_SCL_RESERVED_ADDR(base,scl) ((base) + (0x008c+(scl)*0x10))
#define SOC_DACC_LAYER_LINEREG_PARA0_ADDR(base,ly) ((base) + (0x00d0+(ly)*0x38))
#define SOC_DACC_LAYER_LINEREG_PARA1_ADDR(base,ly) ((base) + (0x00d4+(ly)*0x38))
#define SOC_DACC_LAYER_LINEREG_PARA2_ADDR(base,ly) ((base) + (0x00d8+(ly)*0x38))
#define SOC_DACC_LAYER_LINEREG_PARA3_ADDR(base,ly) ((base) + (0x00dc+(ly)*0x38))
#define SOC_DACC_LAYER_LINEREG_PARA4_ADDR(base,ly) ((base) + (0x00e0+(ly)*0x38))
#define SOC_DACC_LAYER_LINEREG_PARA5_ADDR(base,ly) ((base) + (0x00e4+(ly)*0x38))
#define SOC_DACC_LAYER_LINEREG_PARA6_ADDR(base,ly) ((base) + (0x00e8+(ly)*0x38))
#define SOC_DACC_LAYER_LINEREG_PARA7_ADDR(base,ly) ((base) + (0x00ec+(ly)*0x38))
#define SOC_DACC_LAYER_LINEREG_PARA8_ADDR(base,ly) ((base) + (0x00f0+(ly)*0x38))
#define SOC_DACC_LAYER_LINEREG_PARA9_ADDR(base,ly) ((base) + (0x00f4+(ly)*0x38))
#define SOC_DACC_LAYER_LINEREG_PARA10_ADDR(base,ly) ((base) + (0x00f8+(ly)*0x38))
#define SOC_DACC_LAYER_LINEREG_PARA11_ADDR(base,ly) ((base) + (0x00fc+(ly)*0x38))
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int sw_ext_ints : 7;
        unsigned int reserved : 25;
    } reg;
} SOC_DACC_SW_EXT_INT_REG_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int sw_ext_ints_mask : 7;
        unsigned int reserved : 25;
    } reg;
} SOC_DACC_SW_EXT_INT_MASK_REG_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int core_rst_pc : 32;
    } reg;
} SOC_DACC_POR_RESET_PC_REG_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int core_hart_id : 4;
        unsigned int core_wait : 1;
        unsigned int reserved : 27;
    } reg;
} SOC_DACC_CORE_CONFIG_REG_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int core_sleep_mode : 1;
        unsigned int core_debug_mode : 1;
        unsigned int core_hard_fault_mode : 1;
        unsigned int core_in_nmi_handle : 1;
        unsigned int reserved : 28;
    } reg;
} SOC_DACC_CORE_STATUS_REG_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int core_mon_pc : 32;
    } reg;
} SOC_DACC_CORE_MON_PC_REG_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int core_mon_lr : 32;
    } reg;
} SOC_DACC_CORE_MON_LR_REG_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int ctl_ldi_sel0 : 3;
        unsigned int reserved : 29;
    } reg;
} SOC_DACC_CTL_LDI_SEL0_REG_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int ctl_ldi_sel1 : 3;
        unsigned int reserved : 29;
    } reg;
} SOC_DACC_CTL_LDI_SEL1_REG_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int ctl_ldi_sel2 : 3;
        unsigned int reserved : 29;
    } reg;
} SOC_DACC_CTL_LDI_SEL2_REG_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int ctl_ldi_sel3 : 3;
        unsigned int reserved : 29;
    } reg;
} SOC_DACC_CTL_LDI_SEL3_REG_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int ctl_wch_sel0 : 3;
        unsigned int reserved : 29;
    } reg;
} SOC_DACC_CTL_WCH_SEL0_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int ctl_wch_sel1 : 3;
        unsigned int reserved : 29;
    } reg;
} SOC_DACC_CTL_WCH_SEL1_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int ctl_wch_sel2 : 3;
        unsigned int reserved : 29;
    } reg;
} SOC_DACC_CTL_WCH_SEL2_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int dbg : 32;
    } reg;
} SOC_DACC_DBG_REG0_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int dbg_sel : 32;
    } reg;
} SOC_DACC_DBG_SEL_REG_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int p2h_ckg_en : 1;
        unsigned int h2p_ckg_en : 1;
        unsigned int reserved : 30;
    } reg;
} SOC_DACC_MINIBUS_1T01_CKG_EN_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int p2h_idle : 1;
        unsigned int h2p_idle : 1;
        unsigned int reserved : 30;
    } reg;
} SOC_DACC_MINIBUS_1T01_STATE_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int ckg_en : 8;
        unsigned int reserved : 24;
    } reg;
} SOC_DACC_CKG_EN_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int wlt_en : 1;
        unsigned int reserved_0 : 7;
        unsigned int wlt_layer_id : 5;
        unsigned int reserved_1 : 19;
    } reg;
} SOC_DACC_WLT_EN_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int wlt_slice0_ddr_addr0 : 32;
    } reg;
} SOC_DACC_WLT_SLICE0_DDR_ADDR0_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int wlt_slice0_ddr_addr1 : 4;
        unsigned int reserved : 28;
    } reg;
} SOC_DACC_WLT_SLICE0_DDR_ADDR1_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int wlt_slice1_ddr_addr0 : 32;
    } reg;
} SOC_DACC_WLT_SLICE1_DDR_ADDR0_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int wlt_slice1_ddr_addr1 : 4;
        unsigned int reserved : 28;
    } reg;
} SOC_DACC_WLT_SLICE1_DDR_ADDR1_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int wlt_slice2_ddr_addr0 : 32;
    } reg;
} SOC_DACC_WLT_SLICE2_DDR_ADDR0_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int wlt_slice2_ddr_addr1 : 4;
        unsigned int reserved : 28;
    } reg;
} SOC_DACC_WLT_SLICE2_DDR_ADDR1_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int wlt_slice3_ddr_addr0 : 32;
    } reg;
} SOC_DACC_WLT_SLICE3_DDR_ADDR0_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int wlt_slice3_ddr_addr1 : 4;
        unsigned int reserved : 28;
    } reg;
} SOC_DACC_WLT_SLICE3_DDR_ADDR1_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int wlt_slice0_addr0_h : 4;
        unsigned int reserved_0 : 4;
        unsigned int wlt_slice0_addr1_h : 4;
        unsigned int reserved_1 : 4;
        unsigned int wlt_slice0_addr2_h : 4;
        unsigned int reserved_2 : 4;
        unsigned int wlt_slice0_addr3_h : 4;
        unsigned int reserved_3 : 4;
    } reg;
} SOC_DACC_WLT_SLICE0_ADDR0_H_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int wlt_slice0_addr0_l : 32;
    } reg;
} SOC_DACC_WLT_SLICE0_ADDR0_L_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int wlt_slice0_addr1_l : 32;
    } reg;
} SOC_DACC_WLT_SLICE0_ADDR1_L_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int wlt_slice0_addr2_l : 32;
    } reg;
} SOC_DACC_WLT_SLICE0_ADDR2_L_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int wlt_slice0_addr3_l : 32;
    } reg;
} SOC_DACC_WLT_SLICE0_ADDR3_L_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int wlt_slice1_addr0_h : 4;
        unsigned int reserved_0 : 4;
        unsigned int wlt_slice1_addr1_h : 4;
        unsigned int reserved_1 : 4;
        unsigned int wlt_slice1_addr2_h : 4;
        unsigned int reserved_2 : 4;
        unsigned int wlt_slice1_addr3_h : 4;
        unsigned int reserved_3 : 4;
    } reg;
} SOC_DACC_WLT_SLICE1_ADDR0_H_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int wlt_slice1_addr0_l : 32;
    } reg;
} SOC_DACC_WLT_SLICE1_ADDR0_L_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int wlt_slice1_addr1_l : 32;
    } reg;
} SOC_DACC_WLT_SLICE1_ADDR1_L_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int wlt_slice1_addr2_l : 32;
    } reg;
} SOC_DACC_WLT_SLICE1_ADDR2_L_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int wlt_slice1_addr3_l : 32;
    } reg;
} SOC_DACC_WLT_SLICE1_ADDR3_L_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int wlt_slice2_addr0_h : 4;
        unsigned int reserved_0 : 4;
        unsigned int wlt_slice2_addr1_h : 4;
        unsigned int reserved_1 : 4;
        unsigned int wlt_slice2_addr2_h : 4;
        unsigned int reserved_2 : 4;
        unsigned int wlt_slice2_addr3_h : 4;
        unsigned int reserved_3 : 4;
    } reg;
} SOC_DACC_WLT_SLICE2_ADDR0_H_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int wlt_slice2_addr0_l : 32;
    } reg;
} SOC_DACC_WLT_SLICE2_ADDR0_L_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int wlt_slice2_addr1_l : 32;
    } reg;
} SOC_DACC_WLT_SLICE2_ADDR1_L_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int wlt_slice2_addr2_l : 32;
    } reg;
} SOC_DACC_WLT_SLICE2_ADDR2_L_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int wlt_slice2_addr3_l : 32;
    } reg;
} SOC_DACC_WLT_SLICE2_ADDR3_L_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int wlt_slice3_addr0_h : 4;
        unsigned int reserved_0 : 4;
        unsigned int wlt_slice3_addr1_h : 4;
        unsigned int reserved_1 : 4;
        unsigned int wlt_slice3_addr2_h : 4;
        unsigned int reserved_2 : 4;
        unsigned int wlt_slice3_addr3_h : 4;
        unsigned int reserved_3 : 4;
    } reg;
} SOC_DACC_WLT_SLICE3_ADDR0_H_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int wlt_slice3_addr0_l : 32;
    } reg;
} SOC_DACC_WLT_SLICE3_ADDR0_L_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int wlt_slice3_addr1_l : 32;
    } reg;
} SOC_DACC_WLT_SLICE3_ADDR1_L_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int wlt_slice3_addr2_l : 32;
    } reg;
} SOC_DACC_WLT_SLICE3_ADDR2_L_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int wlt_slice3_addr3_l : 32;
    } reg;
} SOC_DACC_WLT_SLICE3_ADDR3_L_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int wlt_slice_row : 32;
    } reg;
} SOC_DACC_WLT_SLICE_ROW_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int wlt_cmdlist_refresh_offset : 32;
    } reg;
} SOC_DACC_WLT_CMDLIST_REFRESH_OFFSET_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int wch_blk_end : 3;
        unsigned int wch_frm_end : 3;
        unsigned int reserved : 26;
    } reg;
} SOC_DACC_WCH_FRM_BLK_END_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int uvup_scene_assign : 3;
        unsigned int reserved : 29;
    } reg;
} SOC_DACC_UVUP_SCENE_ASSIGN_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int arsr0_scene_assign : 3;
        unsigned int reserved : 29;
    } reg;
} SOC_DACC_ARSR0_SCENE_ASSIGN_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int arsr1_scene_assign : 3;
        unsigned int reserved : 29;
    } reg;
} SOC_DACC_ARSR1_SCENE_ASSIGN_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int vscf0_scene_assign : 3;
        unsigned int reserved : 29;
    } reg;
} SOC_DACC_VSCF0_SCENE_ASSIGN_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int vscf1_scene_assign : 3;
        unsigned int reserved : 29;
    } reg;
} SOC_DACC_VSCF1_SCENE_ASSIGN_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int hdr_scene_assign : 3;
        unsigned int reserved : 29;
    } reg;
} SOC_DACC_HDR_SCENE_ASSIGN_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int cld0_scene_assign : 3;
        unsigned int reserved : 29;
    } reg;
} SOC_DACC_CLD0_SCENE_ASSIGN_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int cld1_scene_assign : 3;
        unsigned int reserved : 29;
    } reg;
} SOC_DACC_CLD1_SCENE_ASSIGN_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int srot0_scene_assign : 3;
        unsigned int reserved : 29;
    } reg;
} SOC_DACC_SROT0_SCENE_ASSIGN_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int srot1_scene_assign : 3;
        unsigned int reserved : 29;
    } reg;
} SOC_DACC_SROT1_SCENE_ASSIGN_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int dma_send_ahead_offset : 3;
        unsigned int reserved : 29;
    } reg;
} SOC_DACC_DMA_SEND_AHEAD_OFFSET_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int mask_sta_flag00 : 1;
        unsigned int mask_sta_flagff : 1;
        unsigned int reserved : 30;
    } reg;
} SOC_DACC_MASK_STA_FLAG_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int mask_area00_start_idx : 22;
        unsigned int reserved : 10;
    } reg;
} SOC_DACC_MASK_AREA00_START_IDX_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int mask_area00_end_idx : 22;
        unsigned int reserved : 10;
    } reg;
} SOC_DACC_MASK_AREA00_END_IDX_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int mask_areaff_start_idx : 22;
        unsigned int reserved : 10;
    } reg;
} SOC_DACC_MASK_AREAFF_START_IDX_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int mask_areaff_end_idx : 22;
        unsigned int reserved : 10;
    } reg;
} SOC_DACC_MASK_AREAFF_END_IDX_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int dacc_clk_sel : 1;
        unsigned int reserved : 31;
    } reg;
} SOC_DACC_CLK_SEL_REG_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int dacc_clk_en : 1;
        unsigned int reserved : 31;
    } reg;
} SOC_DACC_CLK_EN_REG_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int ctl_sw_reload_en : 1;
        unsigned int reserved : 31;
    } reg;
} SOC_DACC_CTL_SW_EN_RELOAD_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int ctl_sw_en : 1;
        unsigned int reserved : 31;
    } reg;
} SOC_DACC_CTL_SW_EN_REG_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int ctl_sw_start : 1;
        unsigned int reserved : 31;
    } reg;
} SOC_DACC_CTL_SW_START_REG_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int ctl_sw_intr_en : 1;
        unsigned int reserved : 31;
    } reg;
} SOC_DACC_CTL_SW_INTR_EN_REG_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int ctl_self_rec_en : 1;
        unsigned int reserved : 31;
    } reg;
} SOC_DACC_CTL_SELF_REC_EN_REG_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int ctl_st_frm_sel : 1;
        unsigned int reserved : 31;
    } reg;
} SOC_DACC_CTL_ST_FRM_SEL0_REG_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int ctl_cfg_mode : 1;
        unsigned int reserved : 31;
    } reg;
} SOC_DACC_CTL_CFG_MODE0_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int cfg_rd_shadow_en : 1;
        unsigned int reserved : 31;
    } reg;
} SOC_DACC_CFG_RD_SHADOW_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int ctl_clear_timeout_en : 1;
        unsigned int reserved : 31;
    } reg;
} SOC_DACC_CTL_CLEAR_TIMEOUT_EN_REG_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int ctl_clear_timeout_thr : 32;
    } reg;
} SOC_DACC_CTL_CLEAR_TIMEOUT_THR_REG_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int clr_start : 1;
        unsigned int reserved : 31;
    } reg;
} SOC_DACC_CLR_START_REG_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int dcmd_first_layer_info_offset : 32;
    } reg;
} SOC_DACC_DCMD_LAYER_INFO_OFFSET_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int clr_riscv_start : 1;
        unsigned int reserved : 31;
    } reg;
} SOC_DACC_CLR_RISCV_START_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int clr_timeout_ints_mask : 1;
        unsigned int reserved : 31;
    } reg;
} SOC_DACC_CLR_TIMEOUT_INTS_MASK_REG_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int clr_ack_timeout_status : 1;
        unsigned int clr_ack_status : 1;
        unsigned int reserved : 30;
    } reg;
} SOC_DACC_CLR_TIMEOUT_STATUS_REG_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int scmd_start : 1;
        unsigned int reserved : 31;
    } reg;
} SOC_DACC_SCMD_START_REG_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int dcmd_start : 1;
        unsigned int reserved : 31;
    } reg;
} SOC_DACC_DCMD_START_REG_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int scmd_addr : 32;
    } reg;
} SOC_DACC_SCMD_DM_ADDR_REG_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int dcmd_addr : 32;
    } reg;
} SOC_DACC_DCMD_DM_ADDR_REG_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int scmd_trans_num : 12;
        unsigned int reserved : 20;
    } reg;
} SOC_DACC_SCMD_NUM_REG_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int scmd_ints_mask : 1;
        unsigned int reserved : 31;
    } reg;
} SOC_DACC_SCMD_INTS_MASK_REG_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int scmd_status : 1;
        unsigned int reserved : 31;
    } reg;
} SOC_DACC_SCMD_STATUS_REG_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int dcmd_ints_mask : 1;
        unsigned int reserved : 31;
    } reg;
} SOC_DACC_DCMD_INTS_MASK_REG_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int dcmd_status : 1;
        unsigned int reserved : 31;
    } reg;
} SOC_DACC_DCMD_STATUS_REG_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int layer_num : 5;
        unsigned int reserved : 27;
    } reg;
} SOC_DACC_LAYER_NUM_REG_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int sw_cmd_reg0 : 32;
    } reg;
} SOC_DACC_SW_CMD_REG0_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int sw_cmd_reg1 : 32;
    } reg;
} SOC_DACC_SW_CMD_REG1_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int sw_cmd_tx_ena : 1;
        unsigned int reserved : 31;
    } reg;
} SOC_DACC_SW_CMD_TX_REG_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int ctl_dbg0 : 32;
    } reg;
} SOC_DACC_CTL_DBG0_REG_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int ctl_dbg1 : 32;
    } reg;
} SOC_DACC_CTL_DBG1_REG_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int q_aempty : 1;
        unsigned int q_afull : 1;
        unsigned int q_empty : 1;
        unsigned int q_full : 1;
        unsigned int reserved : 28;
    } reg;
} SOC_DACC_CTL_DBG2_REG_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int scen_sec_en : 2;
        unsigned int secure_index0 : 2;
        unsigned int secure_index1 : 2;
        unsigned int secure_layer_id_index0 : 5;
        unsigned int secure_layer_id_index1 : 5;
        unsigned int reserved : 16;
    } reg;
} SOC_DACC_SCEN_SECURE_REG_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int scen_secure_layer_dm_base_adr : 32;
    } reg;
} SOC_DACC_SCEN_SECURE_DM_ADDR_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int scen_secure_rsv : 32;
    } reg;
} SOC_DACC_SCEN_SECURE_RSV_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int scen_sec_rot_en : 1;
        unsigned int scen_sec_scl_en : 1;
        unsigned int scen_sec_hdr_en : 1;
        unsigned int scen_sec_uvup_en : 1;
        unsigned int scen_sec_cld_en : 1;
        unsigned int scen_sec_wch_en : 1;
        unsigned int scen_sec_abnormal : 1;
        unsigned int scen_sec_othercmd_en : 25;
    } reg;
} SOC_DACC_SCEN_SECURE_CMD_EN_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int ping_ctl0_layer_secu_en0 : 32;
    } reg;
} SOC_DACC_PING_CTL0_LAYER0_SECU_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int ping_ctl0_layer_secu_en1 : 4;
        unsigned int reserved : 28;
    } reg;
} SOC_DACC_PING_CTL0_LAYER1_SECU_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int intr_pay_abnormal_s : 1;
        unsigned int reserved : 31;
    } reg;
} SOC_DACC_SEC_PAY_ABNORMAL_INT_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int dbg_demura_rlst_cnt : 32;
    } reg;
} SOC_DACC_DBG_DEMURA_RLST_CNT_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int dbg_scl_rlst_cnt_01 : 32;
    } reg;
} SOC_DACC_DBG_SCL_RLST_CNT_01_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int dbg_scl_rlst_cnt_23 : 32;
    } reg;
} SOC_DACC_DBG_SCL_RLST_CNT_23_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int dbg_uvup_rlst_ycnt : 32;
    } reg;
} SOC_DACC_DBG_UVUP_RLST_YCNT_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int dbg_cst_all : 32;
    } reg;
} SOC_DACC_DBG_CST_ALL_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int dbg_fsm_misc : 32;
    } reg;
} SOC_DACC_DBG_FSM_MISC_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int dbg_busy_req_misc : 32;
    } reg;
} SOC_DACC_DBG_BUSY_REQ_MISC_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int dbg_reserved_0 : 32;
    } reg;
} SOC_DACC_DBG_RESERVED_0_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int dbg_reserved_1 : 32;
    } reg;
} SOC_DACC_DBG_RESERVED_1_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int instr_index_0 : 8;
        unsigned int instr_index_1 : 8;
        unsigned int instr_index_2 : 8;
        unsigned int instr_index_3 : 8;
    } reg;
} SOC_DACC_JOINT_RALATION_0_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int instr_index_4 : 8;
        unsigned int instr_index_5 : 8;
        unsigned int instr_index_6 : 8;
        unsigned int instr_index_7 : 8;
    } reg;
} SOC_DACC_JOINT_RALATION_1_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int connect_type : 8;
        unsigned int scmd_num : 8;
        unsigned int dcmd_num : 8;
        unsigned int layer_is_cld : 1;
        unsigned int layer_is_uvup : 1;
        unsigned int reserved : 6;
    } reg;
} SOC_DACC_LAYER_KEY_FEATURE_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int mod_index_0 : 8;
        unsigned int mod_index_1 : 8;
        unsigned int mod_index_2 : 8;
        unsigned int mod_index_3 : 8;
    } reg;
} SOC_DACC_MOD_IDX_TABLE_0_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int mod_index_4 : 8;
        unsigned int mod_index_5 : 8;
        unsigned int mod_index_6 : 8;
        unsigned int mod_index_7 : 8;
    } reg;
} SOC_DACC_MOD_IDX_TABLE_1_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int ov_w_linereg_cnt : 8;
        unsigned int ov_w_linereg_flag : 8;
        unsigned int ov_wtotal : 8;
        unsigned int ov_w_maxline : 8;
    } reg;
} SOC_DACC_OV_LINEREG_PARA_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int hdr_w_linereg_cnt : 8;
        unsigned int hdr_w_linereg_flag : 8;
        unsigned int hdr_wtotal : 8;
        unsigned int hdr_r_linereg_cnt_y : 8;
    } reg;
} SOC_DACC_HDR_LINEREG_PARA0_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int hdr_r_linereg_flag_y : 8;
        unsigned int hdr_rtotal_y : 8;
        unsigned int hdr_r_linereg_cnt_u : 8;
        unsigned int hdr_r_linereg_flag_u : 8;
    } reg;
} SOC_DACC_HDR_LINEREG_PARA1_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int hdr_rtotal_u : 8;
        unsigned int hdr_r_linereg_cnt_v : 8;
        unsigned int hdr_r_linereg_flag_v : 8;
        unsigned int hdr_rtotal_v : 8;
    } reg;
} SOC_DACC_HDR_LINEREG_PARA2_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int uvup_w_linereg_cnt : 8;
        unsigned int uvup_w_linereg_flag : 8;
        unsigned int uvup_wtotal : 8;
        unsigned int uvup_r_linereg_cnt_y : 8;
    } reg;
} SOC_DACC_UVUP_LINEREG_PARA0_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int uvup_r_linereg_flag_y : 8;
        unsigned int uvup_rtotal_y : 8;
        unsigned int uvup_r_linereg_cnt_u : 8;
        unsigned int uvup_r_linereg_flag_u : 8;
    } reg;
} SOC_DACC_UVUP_LINEREG_PARA1_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int uvup_rtotal_u : 8;
        unsigned int uvup_r_linereg_cnt_v : 8;
        unsigned int uvup_r_linereg_flag_v : 8;
        unsigned int uvup_rtotal_v : 8;
    } reg;
} SOC_DACC_UVUP_LINEREG_PARA2_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int reserved : 32;
    } reg;
} SOC_DACC_UVUP_LINEREG_PARA3_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int srot_blk_width : 32;
    } reg;
} SOC_DACC_SROT_BLK_WIDTH_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int srot_blk_height : 32;
    } reg;
} SOC_DACC_SROT_BLK_HEIGHT_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int srot_block_cnt : 32;
    } reg;
} SOC_DACC_SROT_BLOCK_CNT_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int srot_reserved : 32;
    } reg;
} SOC_DACC_SROT_RESERVED_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int srot_write_for_mod : 16;
        unsigned int rot_post_static_or_dyn : 8;
        unsigned int srot_w_linereg_cnt_y : 8;
    } reg;
} SOC_DACC_SROT_LINEREG_PARA0_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int srot_w_linereg_flag_y : 8;
        unsigned int srot_wtotal_y : 8;
        unsigned int srot_w_linereg_cnt_c : 8;
        unsigned int srot_w_linereg_flag_c : 8;
    } reg;
} SOC_DACC_SROT_LINEREG_PARA1_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int srot_wtotal_c : 8;
        unsigned int srot_r_linereg_cnt_y : 8;
        unsigned int srot_r_linereg_flag_y : 8;
        unsigned int srot_rtotal_y : 8;
    } reg;
} SOC_DACC_SROT_LINEREG_PARA2_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int srot_r_linereg_cnt_c : 8;
        unsigned int srot_r_linereg_flag_c : 8;
        unsigned int srot_rtotal_c : 8;
        unsigned int rot_rsv : 8;
    } reg;
} SOC_DACC_SROT_LINEREG_PARA3_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int cld_w_linereg_cnt : 8;
        unsigned int cld_w_linereg_flag : 8;
        unsigned int cld_wtotal : 8;
        unsigned int cld_r_linereg_cnt_y : 8;
    } reg;
} SOC_DACC_CLD_LINEREG_PARA0_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int cld_r_linereg_flag_y : 8;
        unsigned int cld_rtotal_y : 8;
        unsigned int cld_r_linereg_cnt_u : 8;
        unsigned int cld_r_linereg_flag_u : 8;
    } reg;
} SOC_DACC_CLD_LINEREG_PARA1_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int cld_rtotal_u : 8;
        unsigned int cld_r_linereg_cnt_v : 8;
        unsigned int cld_r_linereg_flag_v : 8;
        unsigned int cld_rtotal_v : 8;
    } reg;
} SOC_DACC_CLD_LINEREG_PARA2_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int cld_reserved : 32;
    } reg;
} SOC_DACC_CLD_RESERVED_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int scl_w_linereg_cnt : 8;
        unsigned int scl_w_linereg_flag : 8;
        unsigned int scl_wtotal : 8;
        unsigned int scl_r_linereg_cnt_y : 8;
    } reg;
} SOC_DACC_SCL_LINEREG_PARA0_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int scl_r_linereg_flag_y : 8;
        unsigned int scl_rtotal_y : 8;
        unsigned int scl_r_linereg_cnt_u : 8;
        unsigned int scl_r_linereg_flag_u : 8;
    } reg;
} SOC_DACC_SCL_LINEREG_PARA1_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int scl_rtotal_u : 8;
        unsigned int scl_r_linereg_cnt_v : 8;
        unsigned int scl_r_linereg_flag_v : 8;
        unsigned int scl_rtotal_v : 8;
    } reg;
} SOC_DACC_SCL_LINEREG_PARA2_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int cld_reserved0 : 32;
    } reg;
} SOC_DACC_SCL_RESERVED_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int dma_w_linereg_cnt_y : 8;
        unsigned int dma_w_linereg_flag_y : 8;
        unsigned int dma_wtotal_y : 8;
        unsigned int dma_w_linereg_cnt_u : 8;
    } reg;
} SOC_DACC_LAYER_LINEREG_PARA0_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int dma_w_linereg_flag_u : 8;
        unsigned int dma_wtotal_u : 8;
        unsigned int dma_w_linereg_cnt_v : 8;
        unsigned int dma_w_linereg_flag_v : 8;
    } reg;
} SOC_DACC_LAYER_LINEREG_PARA1_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int dma_wtotal_v : 8;
        unsigned int ov_r_linereg_cnt_y : 8;
        unsigned int ov_r_linereg_flag_y : 8;
        unsigned int ov_r_linereg_cnt_u : 8;
    } reg;
} SOC_DACC_LAYER_LINEREG_PARA2_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int ov_r_linereg_flag_u : 8;
        unsigned int ov_r_linereg_cnt_v : 8;
        unsigned int ov_r_linereg_flag_v : 8;
        unsigned int dma_post_static_or_dyn : 8;
    } reg;
} SOC_DACC_LAYER_LINEREG_PARA3_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int ov_read_pre_static_or_dyn : 8;
        unsigned int max_line_y0 : 8;
        unsigned int max_line_u0 : 8;
        unsigned int max_line_v0 : 8;
    } reg;
} SOC_DACC_LAYER_LINEREG_PARA4_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int max_line_y1 : 8;
        unsigned int max_line_u1 : 8;
        unsigned int max_line_v1 : 8;
        unsigned int max_line_y2 : 8;
    } reg;
} SOC_DACC_LAYER_LINEREG_PARA5_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int max_line_u2 : 8;
        unsigned int max_line_v2 : 8;
        unsigned int max_line_y3 : 8;
        unsigned int max_line_u3 : 8;
    } reg;
} SOC_DACC_LAYER_LINEREG_PARA6_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int max_line_v3 : 8;
        unsigned int max_line_y4 : 8;
        unsigned int max_line_u4 : 8;
        unsigned int max_line_v4 : 8;
    } reg;
} SOC_DACC_LAYER_LINEREG_PARA7_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int max_line_y5 : 8;
        unsigned int max_line_u5 : 8;
        unsigned int max_line_v5 : 8;
        unsigned int max_line_y6 : 8;
    } reg;
} SOC_DACC_LAYER_LINEREG_PARA8_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int max_line_u6 : 8;
        unsigned int max_line_v6 : 8;
        unsigned int max_line_y7 : 8;
        unsigned int max_line_u7 : 8;
    } reg;
} SOC_DACC_LAYER_LINEREG_PARA9_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int max_line_v7 : 8;
        unsigned int dma_block_high_last : 8;
        unsigned int dma_block_width : 16;
    } reg;
} SOC_DACC_LAYER_LINEREG_PARA10_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int dma_block_high : 16;
        unsigned int dma_block_width_last : 16;
    } reg;
} SOC_DACC_LAYER_LINEREG_PARA11_UNION;
#endif
#ifdef __cplusplus
    #if __cplusplus
        }
    #endif
#endif
#endif
