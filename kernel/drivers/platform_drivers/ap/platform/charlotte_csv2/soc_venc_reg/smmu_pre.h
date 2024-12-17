#ifndef __SMMU_PRE_H__
#define __SMMU_PRE_H__ 
typedef union
{
    struct
    {
        unsigned int dpm_freq_sel : 3 ;
        unsigned int smmu_clkgate : 1 ;
        unsigned int smmu_softrst : 1 ;
        unsigned int reserved_0 : 27 ;
    } bits;
    unsigned int u32;
} U_VEDU_DPM_FREQ_SEL;
typedef union
{
    struct
    {
        unsigned int smmu_sid : 8 ;
        unsigned int reserved_0 : 24 ;
    } bits;
    unsigned int u32;
} U_VEDU_SMMU_SID;
typedef union
{
    struct
    {
        unsigned int smmu_ssid : 8 ;
        unsigned int reserved_0 : 24 ;
    } bits;
    unsigned int u32;
} U_VEDU_SMMU_SSID;
typedef union
{
    struct
    {
        unsigned int smmu_ssidv : 1 ;
        unsigned int reserved_0 : 31 ;
    } bits;
    unsigned int u32;
} U_VEDU_SMMU_SSIDV;
typedef union
{
    struct
    {
        unsigned int ddr_addr_intercept_en : 1 ;
        unsigned int reserved_0 : 31 ;
    } bits;
    unsigned int u32;
} U_VEDU_DDR_ADDR_INTERCEPT_EN;
typedef union
{
    struct
    {
        unsigned int ddr_waddr_over_state_clr : 1 ;
        unsigned int ddr_raddr_over_state_clr : 1 ;
        unsigned int reserved_0 : 30 ;
    } bits;
    unsigned int u32;
} U_VEDU_DDR_ADDR_OVER_STATE_CLR;
typedef union
{
    struct
    {
        unsigned int ddr_waddr_over_state : 1 ;
        unsigned int ddr_raddr_over_state : 1 ;
        unsigned int reserved_0 : 30 ;
    } bits;
    unsigned int u32;
} U_VEDU_DDR_ADDR_OVER_STATE;
typedef union
{
    struct
    {
        unsigned int sc_esp_hint_l0 : 4 ;
        unsigned int sc_esp_hint_l1 : 4 ;
        unsigned int sc_esp_hint_l2 : 4 ;
        unsigned int sc_esp_hint_en : 1 ;
        unsigned int reserved_0 : 19 ;
    } bits;
    unsigned int u32;
} U_VEDU_REFLD_ESP_HINT;
typedef union
{
    struct
    {
        unsigned int va_str : 24 ;
        unsigned int reserved_0 : 8 ;
    } bits;
    unsigned int u32;
} U_VEDU_DDR_ADDR_VA_STR;
typedef union
{
    struct
    {
        unsigned int va_end : 24 ;
        unsigned int reserved_0 : 8 ;
    } bits;
    unsigned int u32;
} U_VEDU_DDR_ADDR_VA_END;
typedef union
{
    struct
    {
        unsigned int cache_as_ful : 1 ;
        unsigned int sc_gid_hint : 4 ;
        unsigned int sc_new_hint : 4 ;
        unsigned int cache : 4 ;
        unsigned int domain : 2 ;
        unsigned int reserved_0 : 17 ;
    } bits;
    unsigned int u32;
} U_VEDU_CACHE_AS_FUL;
typedef union
{
    struct
    {
        unsigned int waddr_over_id : 6 ;
        unsigned int raddr_over_id : 6 ;
        unsigned int reserved_0 : 20 ;
    } bits;
    unsigned int u32;
} U_VEDU_DDR_ADDR_OVERID;
typedef union
{
    struct
    {
        unsigned int awqos : 4 ;
        unsigned int arqos : 4 ;
        unsigned int reserved_0 : 24 ;
    } bits;
    unsigned int u32;
} U_VEDU_DDR_QOS;
typedef union
{
    struct
    {
        unsigned int clk_bypass : 1 ;
        unsigned int mst_priority_m : 2 ;
        unsigned int dlock_and_dbg_clr : 1 ;
        unsigned int reserved_0 : 28 ;
    } bits;
    unsigned int u32;
} U_VEDU_HIAXI_DBG_CTRL;
typedef union
{
    struct
    {
        unsigned int dlock_slv : 2 ;
        unsigned int dlock_mst : 4 ;
        unsigned int reserved_0 : 26 ;
    } bits;
    unsigned int u32;
} U_VEDU_HIAXI_DBG_DLOCK;
typedef union
{
    struct
    {
        unsigned int hiaxi_idle : 1 ;
        unsigned int reserved_0 : 31 ;
    } bits;
    unsigned int u32;
} U_VEDU_HIAXI_IDLE;
typedef union
{
    struct
    {
        unsigned int dbg_awid : 9 ;
        unsigned int reserved_0 : 23 ;
    } bits;
    unsigned int u32;
} U_VEDU_HIAXI_DBG_AWID;
typedef union
{
    struct
    {
        unsigned int dbg_awaddr_0 : 31 ;
        unsigned int reserved_0 : 1 ;
    } bits;
    unsigned int u32;
} U_VEDU_HIAXI_DBG_AWADDR_0;
typedef union
{
    struct
    {
        unsigned int dbg_awaddr_1 : 5 ;
        unsigned int reserved_0 : 27 ;
    } bits;
    unsigned int u32;
} U_VEDU_HIAXI_DBG_AWADDR_1;
typedef union
{
    struct
    {
        unsigned int dbg_awlen : 8 ;
        unsigned int reserved_0 : 24 ;
    } bits;
    unsigned int u32;
} U_VEDU_HIAXI_DBG_AWLEN;
typedef union
{
    struct
    {
        unsigned int dbg_awsize : 3 ;
        unsigned int reserved_0 : 29 ;
    } bits;
    unsigned int u32;
} U_VEDU_HIAXI_DBG_AWSIZE;
typedef union
{
    struct
    {
        unsigned int dbg_awburst : 2 ;
        unsigned int reserved_0 : 30 ;
    } bits;
    unsigned int u32;
} U_VEDU_HIAXI_DBG_AWBURST;
typedef union
{
    struct
    {
        unsigned int dbg_awlock : 2 ;
        unsigned int reserved_0 : 30 ;
    } bits;
    unsigned int u32;
} U_VEDU_HIAXI_DBG_AWLOCK;
typedef union
{
    struct
    {
        unsigned int dbg_awcache : 4 ;
        unsigned int reserved_0 : 28 ;
    } bits;
    unsigned int u32;
} U_VEDU_HIAXI_DBG_AWCACHE;
typedef union
{
    struct
    {
        unsigned int dbg_awprot : 3 ;
        unsigned int reserved_0 : 29 ;
    } bits;
    unsigned int u32;
} U_VEDU_HIAXI_DBG_AWPROT;
typedef union
{
    struct
    {
        unsigned int dbg_awvalid : 1 ;
        unsigned int reserved_0 : 31 ;
    } bits;
    unsigned int u32;
} U_VEDU_HIAXI_DBG_AWVALID;
typedef union
{
    struct
    {
        unsigned int dbg_arid : 9 ;
        unsigned int reserved_0 : 23 ;
    } bits;
    unsigned int u32;
} U_VEDU_HIAXI_DBG_ARID;
typedef union
{
    struct
    {
        unsigned int dbg_araddr_0 : 31 ;
        unsigned int reserved_0 : 1 ;
    } bits;
    unsigned int u32;
} U_VEDU_HIAXI_DBG_ARADDR_0;
typedef union
{
    struct
    {
        unsigned int dbg_araddr_1 : 5 ;
        unsigned int reserved_0 : 27 ;
    } bits;
    unsigned int u32;
} U_VEDU_HIAXI_DBG_ARADDR_1;
typedef union
{
    struct
    {
        unsigned int dbg_arlen : 8 ;
        unsigned int reserved_0 : 24 ;
    } bits;
    unsigned int u32;
} U_VEDU_HIAXI_DBG_ARLEN;
typedef union
{
    struct
    {
        unsigned int dbg_arsize : 3 ;
        unsigned int reserved_0 : 29 ;
    } bits;
    unsigned int u32;
} U_VEDU_HIAXI_DBG_ARSIZE;
typedef union
{
    struct
    {
        unsigned int dbg_arburst : 2 ;
        unsigned int reserved_0 : 30 ;
    } bits;
    unsigned int u32;
} U_VEDU_HIAXI_DBG_ARBURST;
typedef union
{
    struct
    {
        unsigned int dbg_arlock : 2 ;
        unsigned int reserved_0 : 30 ;
    } bits;
    unsigned int u32;
} U_VEDU_HIAXI_DBG_ARLOCK;
typedef union
{
    struct
    {
        unsigned int dbg_arcache : 4 ;
        unsigned int reserved_0 : 28 ;
    } bits;
    unsigned int u32;
} U_VEDU_HIAXI_DBG_ARCACHE;
typedef union
{
    struct
    {
        unsigned int dbg_arprot : 3 ;
        unsigned int reserved_0 : 29 ;
    } bits;
    unsigned int u32;
} U_VEDU_HIAXI_DBG_ARPROT;
typedef union
{
    struct
    {
        unsigned int dbg_arvalid : 1 ;
        unsigned int reserved_0 : 31 ;
    } bits;
    unsigned int u32;
} U_VEDU_HIAXI_DBG_ARVALID;
typedef union
{
    struct
    {
        unsigned int smmu_secsid : 1 ;
        unsigned int smmu_secsid_mcu : 1 ;
        unsigned int reserved_0 : 30 ;
    } bits;
    unsigned int u32;
} U_VEDU_SMMU_SECSID;
typedef union
{
    struct
    {
        unsigned int smmu_sid_safe : 8 ;
        unsigned int reserved_0 : 24 ;
    } bits;
    unsigned int u32;
} U_VEDU_SMMU_SID_SAFE;
typedef union
{
    struct
    {
        unsigned int smmu_ssid_safe : 8 ;
        unsigned int reserved_0 : 24 ;
    } bits;
    unsigned int u32;
} U_VEDU_SMMU_SSID_SAFE;
typedef union
{
    struct
    {
        unsigned int smmu_ssidv_safe : 1 ;
        unsigned int reserved_0 : 31 ;
    } bits;
    unsigned int u32;
} U_VEDU_SMMU_SSIDV_SAFE;
typedef union
{
    struct
    {
        unsigned int sc_esp_hint_l0_safe : 4 ;
        unsigned int sc_esp_hint_l1_safe : 4 ;
        unsigned int sc_esp_hint_l2_safe : 4 ;
        unsigned int sc_esp_hint_en_safe : 1 ;
        unsigned int reserved_0 : 19 ;
    } bits;
    unsigned int u32;
} U_VEDU_REFLD_ESP_HINT_SAFE;
typedef union
{
    struct
    {
        unsigned int cache_as_ful_safe : 1 ;
        unsigned int sc_gid_hint_safe : 4 ;
        unsigned int sc_new_hint_safe : 4 ;
        unsigned int cache_safe : 4 ;
        unsigned int reserved_0 : 19 ;
    } bits;
    unsigned int u32;
} U_VEDU_CACHE_AS_FUL_SAFE;
typedef struct
{
    volatile U_VEDU_DPM_FREQ_SEL VEDU_DPM_FREQ_SEL;
    volatile U_VEDU_SMMU_SID VEDU_SMMU_SID;
    volatile U_VEDU_SMMU_SSID VEDU_SMMU_SSID;
    volatile U_VEDU_SMMU_SSIDV VEDU_SMMU_SSIDV;
    volatile U_VEDU_DDR_ADDR_INTERCEPT_EN VEDU_DDR_ADDR_INTERCEPT_EN;
    volatile U_VEDU_DDR_ADDR_OVER_STATE_CLR VEDU_DDR_ADDR_OVER_STATE_CLR;
    volatile U_VEDU_DDR_ADDR_OVER_STATE VEDU_DDR_ADDR_OVER_STATE;
    volatile unsigned int VEDU_RESERVE0;
    volatile unsigned int VEDU_DDR_RADDR_OVER_ADDR;
    volatile unsigned int VEDU_DDR_WADDR_OVER_ADDR;
    volatile U_VEDU_REFLD_ESP_HINT VEDU_REFLD_ESP_HINT;
 volatile unsigned int VEDU_RESERVE1[53];
    volatile U_VEDU_DDR_ADDR_VA_STR VEDU_DDR_ADDR_VA_STR[64];
    volatile U_VEDU_DDR_ADDR_VA_END VEDU_DDR_ADDR_VA_END[64];
    volatile U_VEDU_CACHE_AS_FUL VEDU_CACHE_AS_FUL[64];
    volatile U_VEDU_DDR_ADDR_OVERID VEDU_DDR_ADDR_OVERID;
    volatile U_VEDU_DDR_QOS VEDU_DDR_QOS;
 volatile unsigned int VEDU_RESERVE2[62];
    volatile U_VEDU_HIAXI_DBG_CTRL VEDU_HIAXI_DBG_CTRL;
    volatile U_VEDU_HIAXI_DBG_DLOCK VEDU_HIAXI_DBG_DLOCK;
    volatile U_VEDU_HIAXI_IDLE VEDU_HIAXI_IDLE;
    volatile U_VEDU_HIAXI_DBG_AWID VEDU_HIAXI_DBG_AWID;
    volatile U_VEDU_HIAXI_DBG_AWADDR_0 VEDU_HIAXI_DBG_AWADDR_0;
    volatile U_VEDU_HIAXI_DBG_AWADDR_1 VEDU_HIAXI_DBG_AWADDR_1;
    volatile U_VEDU_HIAXI_DBG_AWLEN VEDU_HIAXI_DBG_AWLEN;
    volatile U_VEDU_HIAXI_DBG_AWSIZE VEDU_HIAXI_DBG_AWSIZE;
    volatile U_VEDU_HIAXI_DBG_AWBURST VEDU_HIAXI_DBG_AWBURST;
    volatile U_VEDU_HIAXI_DBG_AWLOCK VEDU_HIAXI_DBG_AWLOCK;
    volatile U_VEDU_HIAXI_DBG_AWCACHE VEDU_HIAXI_DBG_AWCACHE;
    volatile U_VEDU_HIAXI_DBG_AWPROT VEDU_HIAXI_DBG_AWPROT;
    volatile U_VEDU_HIAXI_DBG_AWVALID VEDU_HIAXI_DBG_AWVALID;
    volatile U_VEDU_HIAXI_DBG_ARID VEDU_HIAXI_DBG_ARID;
    volatile U_VEDU_HIAXI_DBG_ARADDR_0 VEDU_HIAXI_DBG_ARADDR_0;
    volatile U_VEDU_HIAXI_DBG_ARADDR_1 VEDU_HIAXI_DBG_ARADDR_1;
    volatile U_VEDU_HIAXI_DBG_ARLEN VEDU_HIAXI_DBG_ARLEN;
    volatile U_VEDU_HIAXI_DBG_ARSIZE VEDU_HIAXI_DBG_ARSIZE;
    volatile U_VEDU_HIAXI_DBG_ARBURST VEDU_HIAXI_DBG_ARBURST;
    volatile U_VEDU_HIAXI_DBG_ARLOCK VEDU_HIAXI_DBG_ARLOCK;
    volatile U_VEDU_HIAXI_DBG_ARCACHE VEDU_HIAXI_DBG_ARCACHE;
    volatile U_VEDU_HIAXI_DBG_ARPROT VEDU_HIAXI_DBG_ARPROT;
    volatile U_VEDU_HIAXI_DBG_ARVALID VEDU_HIAXI_DBG_ARVALID;
 volatile unsigned int VEDU_RESERVE3[617];
    volatile U_VEDU_SMMU_SECSID VEDU_SMMU_SECSID;
    volatile U_VEDU_SMMU_SID_SAFE VEDU_SMMU_SID_SAFE;
    volatile U_VEDU_SMMU_SSID_SAFE VEDU_SMMU_SSID_SAFE;
    volatile U_VEDU_SMMU_SSIDV_SAFE VEDU_SMMU_SSIDV_SAFE;
    volatile U_VEDU_REFLD_ESP_HINT_SAFE VEDU_REFLD_ESP_HINT_SAFE;
 volatile unsigned int VEDU_RESERVE4[3];
    volatile U_VEDU_CACHE_AS_FUL_SAFE VEDU_CACHE_AS_FUL_SAFE[39];
} S_SMMU_PRE_REGS_TYPE;
#endif
