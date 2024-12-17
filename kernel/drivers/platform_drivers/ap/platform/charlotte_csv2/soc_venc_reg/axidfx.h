#ifndef __AXIDFX_H__
#define __AXIDFX_H__ 
typedef union
{
    struct
    {
        unsigned int dfx_ar_r_err : 1 ;
        unsigned int dfx_aw_w_err : 1 ;
        unsigned int dfx_aw_b_err : 1 ;
        unsigned int dfx_arid_err : 1 ;
        unsigned int dfx_rid_err : 1 ;
        unsigned int dfx_awid_err : 1 ;
        unsigned int dfx_wid_err : 1 ;
        unsigned int dfx_bid_err : 1 ;
        unsigned int dfx_arid_tx_err : 1 ;
        unsigned int dfx_rid_rx_err : 1 ;
        unsigned int dfx_awid_tx_err : 1 ;
        unsigned int dfx_bid_rx_err : 1 ;
        unsigned int dfx_arid_len_err : 1 ;
        unsigned int dfx_awid_len : 1 ;
        unsigned int dfx_rresp_err : 1 ;
        unsigned int dfx_bresp_err : 1 ;
        unsigned int dfx_ar_ovr_err : 1 ;
        unsigned int dfx_r_ovr_err : 1 ;
        unsigned int dfx_aw_ovr_err : 1 ;
        unsigned int dfx_w_ovr_err : 1 ;
        unsigned int dfx_b_ovr_err : 1 ;
        unsigned int dfx_ar_outstanding_err : 1 ;
        unsigned int dfx_aw_outstanding_err : 1 ;
        unsigned int dfx_arlen_err : 1 ;
        unsigned int dfx_awlen_err : 1 ;
        unsigned int reserved_0 : 7 ;
    } bits;
    unsigned int u32;
} U_AXIDFX_ERR;
typedef union
{
    struct
    {
        unsigned int ar_r_cnt : 7 ;
        unsigned int reserved_0 : 25 ;
    } bits;
    unsigned int u32;
} U_AXIDFX_AR_R_CNT;
typedef union
{
    struct
    {
        unsigned int aw_w_cnt : 6 ;
        unsigned int reserved_0 : 26 ;
    } bits;
    unsigned int u32;
} U_AXIDFX_AW_W_CNT;
typedef union
{
    struct
    {
        unsigned int aw_b_cnt : 6 ;
        unsigned int reserved_0 : 26 ;
    } bits;
    unsigned int u32;
} U_AXIDFX_AW_B_CNT;
typedef union
{
    struct
    {
        unsigned int dfx_arid_err : 1 ;
        unsigned int reserved_0 : 3 ;
        unsigned int dfx_rid_err : 1 ;
        unsigned int reserved_1 : 27 ;
    } bits;
    unsigned int u32;
} U_AXIDFX_AR_R_ID_ERR;
typedef union
{
    struct
    {
        unsigned int dfx_err_arid : 7 ;
        unsigned int reserved_0 : 25 ;
    } bits;
    unsigned int u32;
} U_AXIDFX_ERR_ARID;
typedef union
{
    struct
    {
        unsigned int dfx_err_rid : 7 ;
        unsigned int reserved_0 : 25 ;
    } bits;
    unsigned int u32;
} U_AXIDFX_ERR_RID;
typedef union
{
    struct
    {
        unsigned int dfx_awid_err : 1 ;
        unsigned int reserved_0 : 3 ;
        unsigned int dfx_wid_err : 1 ;
        unsigned int reserved_1 : 3 ;
        unsigned int dfx_bid_err : 1 ;
        unsigned int reserved_2 : 23 ;
    } bits;
    unsigned int u32;
} U_AXIDFX_AW_W_B_ID_ERR;
typedef union
{
    struct
    {
        unsigned int dfx_err_awid : 6 ;
        unsigned int reserved_0 : 26 ;
    } bits;
    unsigned int u32;
} U_AXIDFX_ERR_AWID;
typedef union
{
    struct
    {
        unsigned int dfx_err_wid : 6 ;
        unsigned int reserved_0 : 26 ;
    } bits;
    unsigned int u32;
} U_AXIDFX_ERR_WID;
typedef union
{
    struct
    {
        unsigned int dfx_err_bid : 6 ;
        unsigned int reserved_0 : 26 ;
    } bits;
    unsigned int u32;
} U_AXIDFX_ERR_BID;
typedef union
{
    struct
    {
        unsigned int dfx_arid0_tx_err : 1 ;
        unsigned int dfx_arid1_tx_err : 1 ;
        unsigned int dfx_arid2_tx_err : 1 ;
        unsigned int dfx_arid3_tx_err : 1 ;
        unsigned int dfx_arid4_tx_err : 1 ;
        unsigned int dfx_arid5_tx_err : 1 ;
        unsigned int dfx_arid6_tx_err : 1 ;
        unsigned int dfx_arid7_tx_err : 1 ;
        unsigned int dfx_arid8_tx_err : 1 ;
        unsigned int dfx_arid9_tx_err : 1 ;
        unsigned int dfx_arid10_tx_err : 1 ;
        unsigned int dfx_arid11_tx_err : 1 ;
        unsigned int dfx_arid12_tx_err : 1 ;
        unsigned int dfx_arid13_tx_err : 1 ;
        unsigned int dfx_arid14_tx_err : 1 ;
        unsigned int dfx_arid15_tx_err : 1 ;
        unsigned int dfx_arid16_tx_err : 1 ;
        unsigned int dfx_arid17_tx_err : 1 ;
        unsigned int dfx_arid18_tx_err : 1 ;
        unsigned int dfx_arid19_tx_err : 1 ;
        unsigned int dfx_arid20_tx_err : 1 ;
        unsigned int dfx_arid21_tx_err : 1 ;
        unsigned int dfx_arid22_tx_err : 1 ;
        unsigned int dfx_arid23_tx_err : 1 ;
        unsigned int dfx_arid24_tx_err : 1 ;
        unsigned int dfx_arid25_tx_err : 1 ;
        unsigned int dfx_arid26_tx_err : 1 ;
        unsigned int dfx_arid27_tx_err : 1 ;
        unsigned int dfx_arid28_tx_err : 1 ;
        unsigned int dfx_arid29_tx_err : 1 ;
        unsigned int dfx_arid30_tx_err : 1 ;
        unsigned int dfx_arid31_tx_err : 1 ;
    } bits;
    unsigned int u32;
} U_AXIDFX_ARID_TX_0ERR;
typedef union
{
    struct
    {
        unsigned int dfx_arid32_tx_err : 1 ;
        unsigned int dfx_arid33_tx_err : 1 ;
        unsigned int dfx_arid34_tx_err : 1 ;
        unsigned int dfx_arid35_tx_err : 1 ;
        unsigned int dfx_arid36_tx_err : 1 ;
        unsigned int dfx_arid37_tx_err : 1 ;
        unsigned int dfx_arid38_tx_err : 1 ;
        unsigned int dfx_arid39_tx_err : 1 ;
        unsigned int dfx_arid40_tx_err : 1 ;
        unsigned int dfx_arid41_tx_err : 1 ;
        unsigned int dfx_arid42_tx_err : 1 ;
        unsigned int dfx_arid43_tx_err : 1 ;
        unsigned int dfx_arid44_tx_err : 1 ;
        unsigned int dfx_arid45_tx_err : 1 ;
        unsigned int dfx_arid46_tx_err : 1 ;
        unsigned int dfx_arid47_tx_err : 1 ;
        unsigned int dfx_arid48_tx_err : 1 ;
        unsigned int dfx_arid49_tx_err : 1 ;
        unsigned int dfx_arid50_tx_err : 1 ;
        unsigned int dfx_arid51_tx_err : 1 ;
        unsigned int dfx_arid52_tx_err : 1 ;
        unsigned int dfx_arid53_tx_err : 1 ;
        unsigned int dfx_arid54_tx_err : 1 ;
        unsigned int dfx_arid55_tx_err : 1 ;
        unsigned int dfx_arid56_tx_err : 1 ;
        unsigned int dfx_arid57_tx_err : 1 ;
        unsigned int dfx_arid58_tx_err : 1 ;
        unsigned int dfx_arid59_tx_err : 1 ;
        unsigned int dfx_arid60_tx_err : 1 ;
        unsigned int dfx_arid61_tx_err : 1 ;
        unsigned int dfx_arid62_tx_err : 1 ;
        unsigned int dfx_arid63_tx_err : 1 ;
    } bits;
    unsigned int u32;
} U_AXIDFX_ARID_TX_1ERR;
typedef union
{
    struct
    {
        unsigned int dfx_arid64_tx_err : 1 ;
        unsigned int dfx_arid65_tx_err : 1 ;
        unsigned int dfx_arid66_tx_err : 1 ;
        unsigned int dfx_arid67_tx_err : 1 ;
        unsigned int reserved_0 : 28 ;
    } bits;
    unsigned int u32;
} U_AXIDFX_ARID_TX_2ERR;
typedef union
{
    struct
    {
        unsigned int dfx_rid0_rx_err : 1 ;
        unsigned int dfx_rid1_rx_err : 1 ;
        unsigned int dfx_rid2_rx_err : 1 ;
        unsigned int dfx_rid3_rx_err : 1 ;
        unsigned int dfx_rid4_rx_err : 1 ;
        unsigned int dfx_rid5_rx_err : 1 ;
        unsigned int dfx_rid6_rx_err : 1 ;
        unsigned int dfx_rid7_rx_err : 1 ;
        unsigned int dfx_rid8_rx_err : 1 ;
        unsigned int dfx_rid9_rx_err : 1 ;
        unsigned int dfx_rid10_rx_err : 1 ;
        unsigned int dfx_rid11_rx_err : 1 ;
        unsigned int dfx_rid12_rx_err : 1 ;
        unsigned int dfx_rid13_rx_err : 1 ;
        unsigned int dfx_rid14_rx_err : 1 ;
        unsigned int dfx_rid15_rx_err : 1 ;
        unsigned int dfx_rid16_rx_err : 1 ;
        unsigned int dfx_rid17_rx_err : 1 ;
        unsigned int dfx_rid18_rx_err : 1 ;
        unsigned int dfx_rid19_rx_err : 1 ;
        unsigned int dfx_rid20_rx_err : 1 ;
        unsigned int dfx_rid21_rx_err : 1 ;
        unsigned int dfx_rid22_rx_err : 1 ;
        unsigned int dfx_rid23_rx_err : 1 ;
        unsigned int dfx_rid24_rx_err : 1 ;
        unsigned int dfx_rid25_rx_err : 1 ;
        unsigned int dfx_rid26_rx_err : 1 ;
        unsigned int dfx_rid27_rx_err : 1 ;
        unsigned int dfx_rid28_rx_err : 1 ;
        unsigned int dfx_rid29_rx_err : 1 ;
        unsigned int dfx_rid30_rx_err : 1 ;
        unsigned int dfx_rid31_rx_err : 1 ;
    } bits;
    unsigned int u32;
} U_AXIDFX_RID_RX_0ERR;
typedef union
{
    struct
    {
        unsigned int dfx_rid32_rx_err : 1 ;
        unsigned int dfx_rid33_rx_err : 1 ;
        unsigned int dfx_rid34_rx_err : 1 ;
        unsigned int dfx_rid35_rx_err : 1 ;
        unsigned int dfx_rid36_rx_err : 1 ;
        unsigned int dfx_rid37_rx_err : 1 ;
        unsigned int dfx_rid38_rx_err : 1 ;
        unsigned int dfx_rid39_rx_err : 1 ;
        unsigned int dfx_rid40_rx_err : 1 ;
        unsigned int dfx_rid41_rx_err : 1 ;
        unsigned int dfx_rid42_rx_err : 1 ;
        unsigned int dfx_rid43_rx_err : 1 ;
        unsigned int dfx_rid44_rx_err : 1 ;
        unsigned int dfx_rid45_rx_err : 1 ;
        unsigned int dfx_rid46_rx_err : 1 ;
        unsigned int dfx_rid47_rx_err : 1 ;
        unsigned int dfx_rid48_rx_err : 1 ;
        unsigned int dfx_rid49_rx_err : 1 ;
        unsigned int dfx_rid50_rx_err : 1 ;
        unsigned int dfx_rid51_rx_err : 1 ;
        unsigned int dfx_rid52_rx_err : 1 ;
        unsigned int dfx_rid53_rx_err : 1 ;
        unsigned int dfx_rid54_rx_err : 1 ;
        unsigned int dfx_rid55_rx_err : 1 ;
        unsigned int dfx_rid56_rx_err : 1 ;
        unsigned int dfx_rid57_rx_err : 1 ;
        unsigned int dfx_rid58_rx_err : 1 ;
        unsigned int dfx_rid59_rx_err : 1 ;
        unsigned int dfx_rid60_rx_err : 1 ;
        unsigned int dfx_rid61_rx_err : 1 ;
        unsigned int dfx_rid62_rx_err : 1 ;
        unsigned int dfx_rid63_rx_err : 1 ;
    } bits;
    unsigned int u32;
} U_AXIDFX_RID_RX_1ERR;
typedef union
{
    struct
    {
        unsigned int dfx_rid64_rx_err : 1 ;
        unsigned int dfx_rid65_rx_err : 1 ;
        unsigned int dfx_rid66_rx_err : 1 ;
        unsigned int dfx_rid67_rx_err : 1 ;
        unsigned int reserved_0 : 28 ;
    } bits;
    unsigned int u32;
} U_AXIDFX_RID_RX_2ERR;
typedef union
{
    struct
    {
        unsigned int dfx_awid0_rx_err : 1 ;
        unsigned int dfx_awid1_rx_err : 1 ;
        unsigned int dfx_awid2_rx_err : 1 ;
        unsigned int dfx_awid3_rx_err : 1 ;
        unsigned int dfx_awid4_rx_err : 1 ;
        unsigned int dfx_awid5_rx_err : 1 ;
        unsigned int dfx_awid6_rx_err : 1 ;
        unsigned int dfx_awid7_rx_err : 1 ;
        unsigned int reserved_0 : 24 ;
    } bits;
    unsigned int u32;
} U_AXIDFX_ARID_RX_0ERR;
typedef union
{
    struct
    {
        unsigned int dfx_bid0_rx_err : 1 ;
        unsigned int dfx_bid1_rx_err : 1 ;
        unsigned int dfx_bid2_rx_err : 1 ;
        unsigned int dfx_bid3_rx_err : 1 ;
        unsigned int dfx_bid4_rx_err : 1 ;
        unsigned int dfx_bid5_rx_err : 1 ;
        unsigned int dfx_bid6_rx_err : 1 ;
        unsigned int dfx_bid7_rx_err : 1 ;
        unsigned int reserved_0 : 24 ;
    } bits;
    unsigned int u32;
} U_AXIDFX_BID_RX_ERR;
typedef union
{
    struct
    {
        unsigned int dfx_arid0_len_err : 1 ;
        unsigned int dfx_arid1_len_err : 1 ;
        unsigned int dfx_arid2_len_err : 1 ;
        unsigned int dfx_arid3_len_err : 1 ;
        unsigned int dfx_arid4_len_err : 1 ;
        unsigned int dfx_arid5_len_err : 1 ;
        unsigned int dfx_arid6_len_err : 1 ;
        unsigned int dfx_arid7_len_err : 1 ;
        unsigned int dfx_arid8_len_err : 1 ;
        unsigned int dfx_arid9_len_err : 1 ;
        unsigned int dfx_arid10_len_err : 1 ;
        unsigned int dfx_arid11_len_err : 1 ;
        unsigned int dfx_arid12_len_err : 1 ;
        unsigned int dfx_arid13_len_err : 1 ;
        unsigned int dfx_arid14_len_err : 1 ;
        unsigned int dfx_arid15_len_err : 1 ;
        unsigned int dfx_arid16_len_err : 1 ;
        unsigned int dfx_arid17_len_err : 1 ;
        unsigned int dfx_arid18_len_err : 1 ;
        unsigned int dfx_arid19_len_err : 1 ;
        unsigned int dfx_arid20_len_err : 1 ;
        unsigned int dfx_arid21_len_err : 1 ;
        unsigned int dfx_arid22_len_err : 1 ;
        unsigned int dfx_arid23_len_err : 1 ;
        unsigned int dfx_arid24_len_err : 1 ;
        unsigned int dfx_arid25_len_err : 1 ;
        unsigned int dfx_arid26_len_err : 1 ;
        unsigned int dfx_arid27_len_err : 1 ;
        unsigned int dfx_arid28_len_err : 1 ;
        unsigned int dfx_arid29_len_err : 1 ;
        unsigned int dfx_arid30_len_err : 1 ;
        unsigned int dfx_arid31_len_err : 1 ;
    } bits;
    unsigned int u32;
} U_AXIDFX_ARID_LEN_0ERR;
typedef union
{
    struct
    {
        unsigned int dfx_arid32_len_err : 1 ;
        unsigned int dfx_arid33_len_err : 1 ;
        unsigned int dfx_arid34_len_err : 1 ;
        unsigned int dfx_arid35_len_err : 1 ;
        unsigned int dfx_arid36_len_err : 1 ;
        unsigned int dfx_arid37_len_err : 1 ;
        unsigned int dfx_arid38_len_err : 1 ;
        unsigned int dfx_arid39_len_err : 1 ;
        unsigned int dfx_arid40_len_err : 1 ;
        unsigned int dfx_arid41_len_err : 1 ;
        unsigned int dfx_arid42_len_err : 1 ;
        unsigned int dfx_arid43_len_err : 1 ;
        unsigned int dfx_arid44_len_err : 1 ;
        unsigned int dfx_arid45_len_err : 1 ;
        unsigned int dfx_arid46_len_err : 1 ;
        unsigned int dfx_arid47_len_err : 1 ;
        unsigned int dfx_arid48_len_err : 1 ;
        unsigned int dfx_arid49_len_err : 1 ;
        unsigned int dfx_arid50_len_err : 1 ;
        unsigned int dfx_arid51_len_err : 1 ;
        unsigned int dfx_arid52_len_err : 1 ;
        unsigned int dfx_arid53_len_err : 1 ;
        unsigned int dfx_arid54_len_err : 1 ;
        unsigned int dfx_arid55_len_err : 1 ;
        unsigned int dfx_arid56_len_err : 1 ;
        unsigned int dfx_arid57_len_err : 1 ;
        unsigned int dfx_arid58_len_err : 1 ;
        unsigned int dfx_arid59_len_err : 1 ;
        unsigned int dfx_arid60_len_err : 1 ;
        unsigned int dfx_arid61_len_err : 1 ;
        unsigned int dfx_arid62_len_err : 1 ;
        unsigned int dfx_arid63_len_err : 1 ;
    } bits;
    unsigned int u32;
} U_AXIDFX_ARID_LEN_1ERR;
typedef union
{
    struct
    {
        unsigned int dfx_arid64_len_err : 1 ;
        unsigned int dfx_arid65_len_err : 1 ;
        unsigned int dfx_arid66_len_err : 1 ;
        unsigned int dfx_arid67_len_err : 1 ;
        unsigned int reserved_0 : 28 ;
    } bits;
    unsigned int u32;
} U_AXIDFX_ARID_LEN_2ERR;
typedef union
{
    struct
    {
        unsigned int dfx_rresp_err : 1 ;
        unsigned int reserved_0 : 3 ;
        unsigned int dfx_bresp_err : 1 ;
        unsigned int reserved_1 : 27 ;
    } bits;
    unsigned int u32;
} U_AXIDFX_RESP_ERR;
typedef union
{
    struct
    {
        unsigned int dfx_err_rresp : 2 ;
        unsigned int dfx_err_bresp : 2 ;
        unsigned int reserved_0 : 28 ;
    } bits;
    unsigned int u32;
} U_AXIDFX_ERR_RESP;
typedef union
{
    struct
    {
        unsigned int dfx_arlen_err : 1 ;
        unsigned int reserved_0 : 3 ;
        unsigned int dfx_awlen_err : 1 ;
        unsigned int reserved_1 : 27 ;
    } bits;
    unsigned int u32;
} U_AXIDFX_LEN_ERR;
typedef union
{
    struct
    {
        unsigned int dfx_err_arlen : 4 ;
        unsigned int dfx_err_awlen : 4 ;
        unsigned int reserved_0 : 24 ;
    } bits;
    unsigned int u32;
} U_AXIDFX_ERR_LEN;
typedef union
{
    struct
    {
        unsigned int dfx_2rid_flag : 4 ;
        unsigned int reserved_0 : 28 ;
    } bits;
    unsigned int u32;
} U_AXIDFX_2RID_FLAG;
typedef union
{
    struct
    {
        unsigned int dfx_wid_flag : 8 ;
        unsigned int reserved_0 : 24 ;
    } bits;
    unsigned int u32;
} U_AXIDFX_WID_FLAG;
typedef union
{
    struct
    {
        unsigned int arvalid : 1 ;
        unsigned int arready : 1 ;
        unsigned int rvalid : 1 ;
        unsigned int rready : 1 ;
        unsigned int awvalid : 1 ;
        unsigned int awready : 1 ;
        unsigned int wvalid : 1 ;
        unsigned int wready : 1 ;
        unsigned int bvalid : 1 ;
        unsigned int bready : 1 ;
        unsigned int reserved_0 : 22 ;
    } bits;
    unsigned int u32;
} U_AXIDFX_AXI_ST;
typedef union
{
    struct
    {
        unsigned int axi_soft_rst_req : 1 ;
        unsigned int reserved_0 : 31 ;
    } bits;
    unsigned int u32;
} U_AXIDFX_SOFT_RST_REQ;
typedef union
{
    struct
    {
        unsigned int axi_soft_rst_ack : 1 ;
        unsigned int reserved_0 : 31 ;
    } bits;
    unsigned int u32;
} U_AXIDFX_SOFT_RST_ACK;
typedef union
{
    struct
    {
        unsigned int axi_soft_rst_force_req_ack : 1 ;
        unsigned int reserved_0 : 31 ;
    } bits;
    unsigned int u32;
} U_AXIDFX_SOFT_RST_FORCE_REQ_ACK;
typedef struct
{
    volatile U_AXIDFX_ERR AXIDFX_ERR;
    volatile unsigned int RESERVED_1[3];
    volatile U_AXIDFX_AR_R_CNT AXIDFX_AR_R_CNT;
    volatile U_AXIDFX_AW_W_CNT AXIDFX_AW_W_CNT;
    volatile U_AXIDFX_AW_B_CNT AXIDFX_AW_B_CNT;
    volatile unsigned int RESERVED_2;
    volatile U_AXIDFX_AR_R_ID_ERR AXIDFX_AR_R_ID_ERR;
    volatile U_AXIDFX_ERR_ARID AXIDFX_ERR_ARID;
    volatile U_AXIDFX_ERR_RID AXIDFX_ERR_RID;
    volatile unsigned int RESERVED_3;
    volatile U_AXIDFX_AW_W_B_ID_ERR AXIDFX_AW_W_B_ID_ERR;
    volatile U_AXIDFX_ERR_AWID AXIDFX_ERR_AWID;
    volatile U_AXIDFX_ERR_WID AXIDFX_ERR_WID;
    volatile U_AXIDFX_ERR_BID AXIDFX_ERR_BID;
    volatile U_AXIDFX_ARID_TX_0ERR AXIDFX_ARID_TX_0ERR;
    volatile U_AXIDFX_ARID_TX_1ERR AXIDFX_ARID_TX_1ERR;
    volatile U_AXIDFX_ARID_TX_2ERR AXIDFX_ARID_TX_2ERR;
    volatile unsigned int RESERVED_4;
    volatile U_AXIDFX_RID_RX_0ERR AXIDFX_RID_RX_0ERR;
    volatile U_AXIDFX_RID_RX_1ERR AXIDFX_RID_RX_1ERR;
    volatile U_AXIDFX_RID_RX_2ERR AXIDFX_RID_RX_2ERR;
    volatile unsigned int RESERVED_5;
    volatile U_AXIDFX_ARID_RX_0ERR AXIDFX_ARID_RX_0ERR;
    volatile U_AXIDFX_BID_RX_ERR AXIDFX_BID_RX_ERR;
    volatile unsigned int RESERVED_6[2];
    volatile U_AXIDFX_ARID_LEN_0ERR AXIDFX_ARID_LEN_0ERR;
    volatile U_AXIDFX_ARID_LEN_1ERR AXIDFX_ARID_LEN_1ERR;
    volatile U_AXIDFX_ARID_LEN_2ERR AXIDFX_ARID_LEN_2ERR;
    volatile unsigned int RESERVED_7;
    volatile unsigned int AXIDFX_AWLEN_CNT;
    volatile unsigned int AXIDFX_WLEN_CNT;
    volatile unsigned int RESERVED_8[2];
    volatile U_AXIDFX_RESP_ERR AXIDFX_RESP_ERR;
    volatile U_AXIDFX_ERR_RESP AXIDFX_ERR_RESP;
    volatile U_AXIDFX_LEN_ERR AXIDFX_LEN_ERR;
    volatile U_AXIDFX_ERR_LEN AXIDFX_ERR_LEN;
    volatile unsigned int AXIDFX_0RID_FLAG;
    volatile unsigned int AXIDFX_1RID_FLAG;
    volatile U_AXIDFX_2RID_FLAG AXIDFX_2RID_FLAG;
    volatile U_AXIDFX_WID_FLAG AXIDFX_WID_FLAG;
    volatile U_AXIDFX_AXI_ST AXIDFX_AXI_ST;
    volatile unsigned int RESERVED_9[19];
    volatile U_AXIDFX_SOFT_RST_REQ AXIDFX_SOFT_RST_REQ;
    volatile U_AXIDFX_SOFT_RST_ACK AXIDFX_SOFT_RST_ACK;
    volatile unsigned int RESERVED_11[2];
    volatile U_AXIDFX_SOFT_RST_FORCE_REQ_ACK AXIDFX_SOFT_RST_FORCE_REQ_ACK;
    volatile unsigned int AXIDFX_SOFT_RST_STATE0;
    volatile unsigned int AXIDFX_SOFT_RST_STATE1;
    volatile unsigned int RESERVED_12[57];
    volatile unsigned int RESERVED_S_NS_0;
    volatile unsigned int RESERVED_S_NS_1;
    volatile unsigned int RESERVED_S_NS_2;
    volatile unsigned int RESERVED_S_NS_3;
    volatile unsigned int RESERVED_S_NS_4;
    volatile unsigned int RESERVED_S_NS_5;
    volatile unsigned int RESERVED_S_NS_6;
    volatile unsigned int RESERVED_S_NS_7;
} S_AXIDFX_REGS_TYPE;
extern volatile S_AXIDFX_REGS_TYPE *gopAXIDFXAllReg;
int iSetAXIDFX_SOFT_RST_REQaxi_soft_rst_req(unsigned int uaxi_soft_rst_req);
int iSetAXIDFX_SOFT_RST_FORCE_REQ_ACKaxi_soft_rst_force_req_ack(unsigned int uaxi_soft_rst_force_req_ack);
int iSetRESERVED_S_NS_0sec_ns_set0(unsigned int usec_ns_set0);
int iSetRESERVED_S_NS_1sec_ns_set1(unsigned int usec_ns_set1);
int iSetRESERVED_S_NS_2sec_ns_set2(unsigned int usec_ns_set2);
int iSetRESERVED_S_NS_3sec_ns_set3(unsigned int usec_ns_set3);
int iSetRESERVED_S_NS_4sec_ns_set4(unsigned int usec_ns_set4);
int iSetRESERVED_S_NS_5sec_ns_set5(unsigned int usec_ns_set5);
int iSetRESERVED_S_NS_6sec_ns_set6(unsigned int usec_ns_set6);
int iSetRESERVED_S_NS_7sec_ns_set7(unsigned int usec_ns_set7);
#endif
