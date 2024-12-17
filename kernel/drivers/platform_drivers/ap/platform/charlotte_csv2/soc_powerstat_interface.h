#ifndef __SOC_POWERSTAT_INTERFACE_H__
#define __SOC_POWERSTAT_INTERFACE_H__ 
#ifdef __cplusplus
    #if __cplusplus
        extern "C" {
    #endif
#endif
#ifndef __SOC_H_FOR_ASM__
#define SOC_POWERSTAT_SAMPLE_EN_ADDR(base) ((base) + (0x000UL))
#define SOC_POWERSTAT_CHNL_EN0_ADDR(base) ((base) + (0x004UL))
#define SOC_POWERSTAT_PWR_EN_ADDR(base) ((base) + (0x014UL))
#define SOC_POWERSTAT_WORK_MODE_CFG_ADDR(base) ((base) + (0x018UL))
#define SOC_POWERSTAT_INTERVAL_ADDR(base) ((base) + (0x01CUL))
#define SOC_POWERSTAT_SAMPLE_TIME_ADDR(base) ((base) + (0x020UL))
#define SOC_POWERSTAT_SRAM_CTRL_ADDR(base) ((base) + (0x024UL))
#define SOC_POWERSTAT_AXI_INTF_CFG_ADDR(base) ((base) + (0x028UL))
#define SOC_POWERSTAT_SFIFO_CFG_ADDR(base) ((base) + (0x030UL))
#define SOC_POWERSTAT_SEQ_ADDR_CFG0_ADDR(base) ((base) + (0x034UL))
#define SOC_POWERSTAT_SEQ_ADDR_CFG1_ADDR(base) ((base) + (0x038UL))
#define SOC_POWERSTAT_SEQ_SPACE_CFG0_ADDR(base) ((base) + (0x03CUL))
#define SOC_POWERSTAT_SEQ_SPACE_CFG1_ADDR(base) ((base) + (0x040UL))
#define SOC_POWERSTAT_LINK_ADDR_CFG0_ADDR(base) ((base) + (0x044UL))
#define SOC_POWERSTAT_LINK_ADDR_CFG1_ADDR(base) ((base) + (0x048UL))
#define SOC_POWERSTAT_NOP_TIME_ADDR(base) ((base) + (0x04CUL))
#define SOC_POWERSTAT_AXI_INT_EN_ADDR(base) ((base) + (0x070UL))
#define SOC_POWERSTAT_AXI_INTF_INT_CLR_ADDR(base) ((base) + (0x074UL))
#define SOC_POWERSTAT_AXI_INT_MASK_ADDR(base) ((base) + (0x078UL))
#define SOC_POWERSTAT_INBAND_INT_EN_ADDR(base,m) ((base) + (0x080+0x004*(m)))
#define SOC_POWERSTAT_CHNL_CTRL_IN_BAND_INT_CLR_ADDR(base,m) ((base) + (0x090+0x004*(m)))
#define SOC_POWERSTAT_INBAND_INT_MASK_ADDR(base,m) ((base) + (0x0A0+0x004*(m)))
#define SOC_POWERSTAT_TIMEOUT_INT_EN_ADDR(base,m) ((base) + (0x0B0+0x004*(m)))
#define SOC_POWERSTAT_CHNL_CTRL_TIMEOUT_INT_CLR_ADDR(base,m) ((base) + (0x0C0+0x004*(m)))
#define SOC_POWERSTAT_TIMEOUT_INT_MASK_ADDR(base,m) ((base) + (0x0D0+0x004*(m)))
#define SOC_POWERSTAT_POWER_STAT_TIMER_DBG_ADDR(base) ((base) + (0x200UL))
#define SOC_POWERSTAT_AXI_INT_STAT_ADDR(base) ((base) + (0x204UL))
#define SOC_POWERSTAT_AXI_ERR_RESP_DBG_ADDR(base) ((base) + (0x208UL))
#define SOC_POWERSTAT_AXI_ERR_RESP_TIME_DBG_ADDR(base) ((base) + (0x20CUL))
#define SOC_POWERSTAT_AXI_AW_OT_NUM_ADDR(base) ((base) + (0x210UL))
#define SOC_POWERSTAT_INBAND_INT_STAT_ADDR(base) ((base) + (0x220UL))
#define SOC_POWERSTAT_TIMEOUT_INT_STAT_ADDR(base) ((base) + (0x230UL))
#define SOC_POWERSTAT_DPM_TOTAL_ENERGY_LOW_ADDR(base,n) ((base) + (0x240+0x008*(n)))
#define SOC_POWERSTAT_DPM_TOTAL_ENERGY_HIGH_ADDR(base,n) ((base) + (0x244+0x008*(n)))
#define SOC_POWERSTAT_DPM_DYN_ENERGY_LOW_ADDR(base,n) ((base) + (0x380+0x008*(n)))
#define SOC_POWERSTAT_DPM_DYN_ENERGY_HIGH_ADDR(base,n) ((base) + (0x384+0x008*(n)))
#else
#define SOC_POWERSTAT_SAMPLE_EN_ADDR(base) ((base) + (0x000))
#define SOC_POWERSTAT_CHNL_EN0_ADDR(base) ((base) + (0x004))
#define SOC_POWERSTAT_PWR_EN_ADDR(base) ((base) + (0x014))
#define SOC_POWERSTAT_WORK_MODE_CFG_ADDR(base) ((base) + (0x018))
#define SOC_POWERSTAT_INTERVAL_ADDR(base) ((base) + (0x01C))
#define SOC_POWERSTAT_SAMPLE_TIME_ADDR(base) ((base) + (0x020))
#define SOC_POWERSTAT_SRAM_CTRL_ADDR(base) ((base) + (0x024))
#define SOC_POWERSTAT_AXI_INTF_CFG_ADDR(base) ((base) + (0x028))
#define SOC_POWERSTAT_SFIFO_CFG_ADDR(base) ((base) + (0x030))
#define SOC_POWERSTAT_SEQ_ADDR_CFG0_ADDR(base) ((base) + (0x034))
#define SOC_POWERSTAT_SEQ_ADDR_CFG1_ADDR(base) ((base) + (0x038))
#define SOC_POWERSTAT_SEQ_SPACE_CFG0_ADDR(base) ((base) + (0x03C))
#define SOC_POWERSTAT_SEQ_SPACE_CFG1_ADDR(base) ((base) + (0x040))
#define SOC_POWERSTAT_LINK_ADDR_CFG0_ADDR(base) ((base) + (0x044))
#define SOC_POWERSTAT_LINK_ADDR_CFG1_ADDR(base) ((base) + (0x048))
#define SOC_POWERSTAT_NOP_TIME_ADDR(base) ((base) + (0x04C))
#define SOC_POWERSTAT_AXI_INT_EN_ADDR(base) ((base) + (0x070))
#define SOC_POWERSTAT_AXI_INTF_INT_CLR_ADDR(base) ((base) + (0x074))
#define SOC_POWERSTAT_AXI_INT_MASK_ADDR(base) ((base) + (0x078))
#define SOC_POWERSTAT_INBAND_INT_EN_ADDR(base,m) ((base) + (0x080+0x004*(m)))
#define SOC_POWERSTAT_CHNL_CTRL_IN_BAND_INT_CLR_ADDR(base,m) ((base) + (0x090+0x004*(m)))
#define SOC_POWERSTAT_INBAND_INT_MASK_ADDR(base,m) ((base) + (0x0A0+0x004*(m)))
#define SOC_POWERSTAT_TIMEOUT_INT_EN_ADDR(base,m) ((base) + (0x0B0+0x004*(m)))
#define SOC_POWERSTAT_CHNL_CTRL_TIMEOUT_INT_CLR_ADDR(base,m) ((base) + (0x0C0+0x004*(m)))
#define SOC_POWERSTAT_TIMEOUT_INT_MASK_ADDR(base,m) ((base) + (0x0D0+0x004*(m)))
#define SOC_POWERSTAT_POWER_STAT_TIMER_DBG_ADDR(base) ((base) + (0x200))
#define SOC_POWERSTAT_AXI_INT_STAT_ADDR(base) ((base) + (0x204))
#define SOC_POWERSTAT_AXI_ERR_RESP_DBG_ADDR(base) ((base) + (0x208))
#define SOC_POWERSTAT_AXI_ERR_RESP_TIME_DBG_ADDR(base) ((base) + (0x20C))
#define SOC_POWERSTAT_AXI_AW_OT_NUM_ADDR(base) ((base) + (0x210))
#define SOC_POWERSTAT_INBAND_INT_STAT_ADDR(base) ((base) + (0x220))
#define SOC_POWERSTAT_TIMEOUT_INT_STAT_ADDR(base) ((base) + (0x230))
#define SOC_POWERSTAT_DPM_TOTAL_ENERGY_LOW_ADDR(base,n) ((base) + (0x240+0x008*(n)))
#define SOC_POWERSTAT_DPM_TOTAL_ENERGY_HIGH_ADDR(base,n) ((base) + (0x244+0x008*(n)))
#define SOC_POWERSTAT_DPM_DYN_ENERGY_LOW_ADDR(base,n) ((base) + (0x380+0x008*(n)))
#define SOC_POWERSTAT_DPM_DYN_ENERGY_HIGH_ADDR(base,n) ((base) + (0x384+0x008*(n)))
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int sample_en : 1;
        unsigned int reserved : 31;
    } reg;
} SOC_POWERSTAT_SAMPLE_EN_UNION;
#endif
#define SOC_POWERSTAT_SAMPLE_EN_sample_en_START (0)
#define SOC_POWERSTAT_SAMPLE_EN_sample_en_END (0)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int channel_en : 32;
    } reg;
} SOC_POWERSTAT_CHNL_EN0_UNION;
#endif
#define SOC_POWERSTAT_CHNL_EN0_channel_en_START (0)
#define SOC_POWERSTAT_CHNL_EN0_channel_en_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int fcm_cluster_power_en : 1;
        unsigned int g3d_wrap_pwr_en : 1;
        unsigned int npu_subsys_pwr_en : 1;
        unsigned int media1_pwr_en : 1;
        unsigned int media2_pwr_en : 1;
        unsigned int reserved_0 : 11;
        unsigned int reserved_1 : 16;
    } reg;
} SOC_POWERSTAT_PWR_EN_UNION;
#endif
#define SOC_POWERSTAT_PWR_EN_fcm_cluster_power_en_START (0)
#define SOC_POWERSTAT_PWR_EN_fcm_cluster_power_en_END (0)
#define SOC_POWERSTAT_PWR_EN_g3d_wrap_pwr_en_START (1)
#define SOC_POWERSTAT_PWR_EN_g3d_wrap_pwr_en_END (1)
#define SOC_POWERSTAT_PWR_EN_npu_subsys_pwr_en_START (2)
#define SOC_POWERSTAT_PWR_EN_npu_subsys_pwr_en_END (2)
#define SOC_POWERSTAT_PWR_EN_media1_pwr_en_START (3)
#define SOC_POWERSTAT_PWR_EN_media1_pwr_en_END (3)
#define SOC_POWERSTAT_PWR_EN_media2_pwr_en_START (4)
#define SOC_POWERSTAT_PWR_EN_media2_pwr_en_END (4)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int cmd_mode : 2;
        unsigned int sample_mode : 1;
        unsigned int addr_mode : 1;
        unsigned int slv_timeout : 4;
        unsigned int axi_intf_awlen : 8;
        unsigned int aw_max_ot_num : 4;
        unsigned int reserved : 3;
        unsigned int frame_len : 9;
    } reg;
} SOC_POWERSTAT_WORK_MODE_CFG_UNION;
#endif
#define SOC_POWERSTAT_WORK_MODE_CFG_cmd_mode_START (0)
#define SOC_POWERSTAT_WORK_MODE_CFG_cmd_mode_END (1)
#define SOC_POWERSTAT_WORK_MODE_CFG_sample_mode_START (2)
#define SOC_POWERSTAT_WORK_MODE_CFG_sample_mode_END (2)
#define SOC_POWERSTAT_WORK_MODE_CFG_addr_mode_START (3)
#define SOC_POWERSTAT_WORK_MODE_CFG_addr_mode_END (3)
#define SOC_POWERSTAT_WORK_MODE_CFG_slv_timeout_START (4)
#define SOC_POWERSTAT_WORK_MODE_CFG_slv_timeout_END (7)
#define SOC_POWERSTAT_WORK_MODE_CFG_axi_intf_awlen_START (8)
#define SOC_POWERSTAT_WORK_MODE_CFG_axi_intf_awlen_END (15)
#define SOC_POWERSTAT_WORK_MODE_CFG_aw_max_ot_num_START (16)
#define SOC_POWERSTAT_WORK_MODE_CFG_aw_max_ot_num_END (19)
#define SOC_POWERSTAT_WORK_MODE_CFG_frame_len_START (23)
#define SOC_POWERSTAT_WORK_MODE_CFG_frame_len_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int sample_interval : 32;
    } reg;
} SOC_POWERSTAT_INTERVAL_UNION;
#endif
#define SOC_POWERSTAT_INTERVAL_sample_interval_START (0)
#define SOC_POWERSTAT_INTERVAL_sample_interval_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int sample_time : 32;
    } reg;
} SOC_POWERSTAT_SAMPLE_TIME_UNION;
#endif
#define SOC_POWERSTAT_SAMPLE_TIME_sample_time_START (0)
#define SOC_POWERSTAT_SAMPLE_TIME_sample_time_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int power_stat_mem_ctrl : 16;
        unsigned int reserved : 16;
    } reg;
} SOC_POWERSTAT_SRAM_CTRL_UNION;
#endif
#define SOC_POWERSTAT_SRAM_CTRL_power_stat_mem_ctrl_START (0)
#define SOC_POWERSTAT_SRAM_CTRL_power_stat_mem_ctrl_END (15)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int chnl_ctrl_head_timeout : 32;
    } reg;
} SOC_POWERSTAT_AXI_INTF_CFG_UNION;
#endif
#define SOC_POWERSTAT_AXI_INTF_CFG_chnl_ctrl_head_timeout_START (0)
#define SOC_POWERSTAT_AXI_INTF_CFG_chnl_ctrl_head_timeout_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int chnl_sfifo_depth : 9;
        unsigned int reserved : 23;
    } reg;
} SOC_POWERSTAT_SFIFO_CFG_UNION;
#endif
#define SOC_POWERSTAT_SFIFO_CFG_chnl_sfifo_depth_START (0)
#define SOC_POWERSTAT_SFIFO_CFG_chnl_sfifo_depth_END (8)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int seq_start_addr_low : 32;
    } reg;
} SOC_POWERSTAT_SEQ_ADDR_CFG0_UNION;
#endif
#define SOC_POWERSTAT_SEQ_ADDR_CFG0_seq_start_addr_low_START (0)
#define SOC_POWERSTAT_SEQ_ADDR_CFG0_seq_start_addr_low_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int seq_start_addr_high : 32;
    } reg;
} SOC_POWERSTAT_SEQ_ADDR_CFG1_UNION;
#endif
#define SOC_POWERSTAT_SEQ_ADDR_CFG1_seq_start_addr_high_START (0)
#define SOC_POWERSTAT_SEQ_ADDR_CFG1_seq_start_addr_high_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int seq_ddr_space_low : 32;
    } reg;
} SOC_POWERSTAT_SEQ_SPACE_CFG0_UNION;
#endif
#define SOC_POWERSTAT_SEQ_SPACE_CFG0_seq_ddr_space_low_START (0)
#define SOC_POWERSTAT_SEQ_SPACE_CFG0_seq_ddr_space_low_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int seq_ddr_space_high : 32;
    } reg;
} SOC_POWERSTAT_SEQ_SPACE_CFG1_UNION;
#endif
#define SOC_POWERSTAT_SEQ_SPACE_CFG1_seq_ddr_space_high_START (0)
#define SOC_POWERSTAT_SEQ_SPACE_CFG1_seq_ddr_space_high_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int link_start_addr_low : 32;
    } reg;
} SOC_POWERSTAT_LINK_ADDR_CFG0_UNION;
#endif
#define SOC_POWERSTAT_LINK_ADDR_CFG0_link_start_addr_low_START (0)
#define SOC_POWERSTAT_LINK_ADDR_CFG0_link_start_addr_low_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int link_start_addr_high : 32;
    } reg;
} SOC_POWERSTAT_LINK_ADDR_CFG1_UNION;
#endif
#define SOC_POWERSTAT_LINK_ADDR_CFG1_link_start_addr_high_START (0)
#define SOC_POWERSTAT_LINK_ADDR_CFG1_link_start_addr_high_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int nop_time : 8;
        unsigned int reserved : 24;
    } reg;
} SOC_POWERSTAT_NOP_TIME_UNION;
#endif
#define SOC_POWERSTAT_NOP_TIME_nop_time_START (0)
#define SOC_POWERSTAT_NOP_TIME_nop_time_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int axi_intf_int_en : 1;
        unsigned int reserved : 31;
    } reg;
} SOC_POWERSTAT_AXI_INT_EN_UNION;
#endif
#define SOC_POWERSTAT_AXI_INT_EN_axi_intf_int_en_START (0)
#define SOC_POWERSTAT_AXI_INT_EN_axi_intf_int_en_END (0)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int axi_intf_int_clr : 1;
        unsigned int reserved : 31;
    } reg;
} SOC_POWERSTAT_AXI_INTF_INT_CLR_UNION;
#endif
#define SOC_POWERSTAT_AXI_INTF_INT_CLR_axi_intf_int_clr_START (0)
#define SOC_POWERSTAT_AXI_INTF_INT_CLR_axi_intf_int_clr_END (0)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int axi_intf_int_mask : 1;
        unsigned int reserved : 31;
    } reg;
} SOC_POWERSTAT_AXI_INT_MASK_UNION;
#endif
#define SOC_POWERSTAT_AXI_INT_MASK_axi_intf_int_mask_START (0)
#define SOC_POWERSTAT_AXI_INT_MASK_axi_intf_int_mask_END (0)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int chnl_ctrl_in_band_int_en : 16;
        unsigned int reserved : 16;
    } reg;
} SOC_POWERSTAT_INBAND_INT_EN_UNION;
#endif
#define SOC_POWERSTAT_INBAND_INT_EN_chnl_ctrl_in_band_int_en_START (0)
#define SOC_POWERSTAT_INBAND_INT_EN_chnl_ctrl_in_band_int_en_END (15)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int chnl_ctrl_in_band_int_clr : 16;
        unsigned int reserved : 16;
    } reg;
} SOC_POWERSTAT_CHNL_CTRL_IN_BAND_INT_CLR_UNION;
#endif
#define SOC_POWERSTAT_CHNL_CTRL_IN_BAND_INT_CLR_chnl_ctrl_in_band_int_clr_START (0)
#define SOC_POWERSTAT_CHNL_CTRL_IN_BAND_INT_CLR_chnl_ctrl_in_band_int_clr_END (15)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int chnl_ctrl_in_band_int_mask : 16;
        unsigned int reserved : 16;
    } reg;
} SOC_POWERSTAT_INBAND_INT_MASK_UNION;
#endif
#define SOC_POWERSTAT_INBAND_INT_MASK_chnl_ctrl_in_band_int_mask_START (0)
#define SOC_POWERSTAT_INBAND_INT_MASK_chnl_ctrl_in_band_int_mask_END (15)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int chnl_ctrl_timeout_int_en : 16;
        unsigned int reserved : 16;
    } reg;
} SOC_POWERSTAT_TIMEOUT_INT_EN_UNION;
#endif
#define SOC_POWERSTAT_TIMEOUT_INT_EN_chnl_ctrl_timeout_int_en_START (0)
#define SOC_POWERSTAT_TIMEOUT_INT_EN_chnl_ctrl_timeout_int_en_END (15)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int chnl_ctrl_timeout_int_clr : 16;
        unsigned int reserved : 16;
    } reg;
} SOC_POWERSTAT_CHNL_CTRL_TIMEOUT_INT_CLR_UNION;
#endif
#define SOC_POWERSTAT_CHNL_CTRL_TIMEOUT_INT_CLR_chnl_ctrl_timeout_int_clr_START (0)
#define SOC_POWERSTAT_CHNL_CTRL_TIMEOUT_INT_CLR_chnl_ctrl_timeout_int_clr_END (15)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int chnl_ctrl_timeout_int_mask : 16;
        unsigned int reserved : 16;
    } reg;
} SOC_POWERSTAT_TIMEOUT_INT_MASK_UNION;
#endif
#define SOC_POWERSTAT_TIMEOUT_INT_MASK_chnl_ctrl_timeout_int_mask_START (0)
#define SOC_POWERSTAT_TIMEOUT_INT_MASK_chnl_ctrl_timeout_int_mask_END (15)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int power_stat_timer : 32;
    } reg;
} SOC_POWERSTAT_POWER_STAT_TIMER_DBG_UNION;
#endif
#define SOC_POWERSTAT_POWER_STAT_TIMER_DBG_power_stat_timer_START (0)
#define SOC_POWERSTAT_POWER_STAT_TIMER_DBG_power_stat_timer_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int axi_intf_int : 1;
        unsigned int reserved : 31;
    } reg;
} SOC_POWERSTAT_AXI_INT_STAT_UNION;
#endif
#define SOC_POWERSTAT_AXI_INT_STAT_axi_intf_int_START (0)
#define SOC_POWERSTAT_AXI_INT_STAT_axi_intf_int_END (0)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int bresp_sample : 2;
        unsigned int rresp_sample : 2;
        unsigned int reserved : 28;
    } reg;
} SOC_POWERSTAT_AXI_ERR_RESP_DBG_UNION;
#endif
#define SOC_POWERSTAT_AXI_ERR_RESP_DBG_bresp_sample_START (0)
#define SOC_POWERSTAT_AXI_ERR_RESP_DBG_bresp_sample_END (1)
#define SOC_POWERSTAT_AXI_ERR_RESP_DBG_rresp_sample_START (2)
#define SOC_POWERSTAT_AXI_ERR_RESP_DBG_rresp_sample_END (3)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int bresp_error_time : 16;
        unsigned int rresp_error_time : 16;
    } reg;
} SOC_POWERSTAT_AXI_ERR_RESP_TIME_DBG_UNION;
#endif
#define SOC_POWERSTAT_AXI_ERR_RESP_TIME_DBG_bresp_error_time_START (0)
#define SOC_POWERSTAT_AXI_ERR_RESP_TIME_DBG_bresp_error_time_END (15)
#define SOC_POWERSTAT_AXI_ERR_RESP_TIME_DBG_rresp_error_time_START (16)
#define SOC_POWERSTAT_AXI_ERR_RESP_TIME_DBG_rresp_error_time_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int axi_aw_ot_num : 8;
        unsigned int reserved : 24;
    } reg;
} SOC_POWERSTAT_AXI_AW_OT_NUM_UNION;
#endif
#define SOC_POWERSTAT_AXI_AW_OT_NUM_axi_aw_ot_num_START (0)
#define SOC_POWERSTAT_AXI_AW_OT_NUM_axi_aw_ot_num_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int chnl_ctrl_in_band_int : 32;
    } reg;
} SOC_POWERSTAT_INBAND_INT_STAT_UNION;
#endif
#define SOC_POWERSTAT_INBAND_INT_STAT_chnl_ctrl_in_band_int_START (0)
#define SOC_POWERSTAT_INBAND_INT_STAT_chnl_ctrl_in_band_int_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int chnl_ctrl_timeout_int : 32;
    } reg;
} SOC_POWERSTAT_TIMEOUT_INT_STAT_UNION;
#endif
#define SOC_POWERSTAT_TIMEOUT_INT_STAT_chnl_ctrl_timeout_int_START (0)
#define SOC_POWERSTAT_TIMEOUT_INT_STAT_chnl_ctrl_timeout_int_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int dpm_total_energy_low : 32;
    } reg;
} SOC_POWERSTAT_DPM_TOTAL_ENERGY_LOW_UNION;
#endif
#define SOC_POWERSTAT_DPM_TOTAL_ENERGY_LOW_dpm_total_energy_low_START (0)
#define SOC_POWERSTAT_DPM_TOTAL_ENERGY_LOW_dpm_total_energy_low_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int dpm_total_energy_high : 32;
    } reg;
} SOC_POWERSTAT_DPM_TOTAL_ENERGY_HIGH_UNION;
#endif
#define SOC_POWERSTAT_DPM_TOTAL_ENERGY_HIGH_dpm_total_energy_high_START (0)
#define SOC_POWERSTAT_DPM_TOTAL_ENERGY_HIGH_dpm_total_energy_high_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int dpm_dyn_energy_low : 32;
    } reg;
} SOC_POWERSTAT_DPM_DYN_ENERGY_LOW_UNION;
#endif
#define SOC_POWERSTAT_DPM_DYN_ENERGY_LOW_dpm_dyn_energy_low_START (0)
#define SOC_POWERSTAT_DPM_DYN_ENERGY_LOW_dpm_dyn_energy_low_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int dpm_dyn_energy_high : 32;
    } reg;
} SOC_POWERSTAT_DPM_DYN_ENERGY_HIGH_UNION;
#endif
#define SOC_POWERSTAT_DPM_DYN_ENERGY_HIGH_dpm_dyn_energy_high_START (0)
#define SOC_POWERSTAT_DPM_DYN_ENERGY_HIGH_dpm_dyn_energy_high_END (31)
#ifdef __cplusplus
    #if __cplusplus
        }
    #endif
#endif
#endif
