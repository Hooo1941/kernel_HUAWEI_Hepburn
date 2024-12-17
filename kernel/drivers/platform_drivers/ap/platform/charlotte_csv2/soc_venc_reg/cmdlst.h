#ifndef __CMDLST_H__
#define __CMDLST_H__ 
typedef union
{
    struct
    {
        unsigned int force_clk_on : 1 ;
        unsigned int reserved_0 : 31 ;
    } bits;
    unsigned int u32;
} U_CMDLST_CFG;
typedef union
{
    struct
    {
        unsigned int sw_cfg_buf_len0 : 16 ;
        unsigned int sw_cfg_buf_len1 : 16 ;
    } bits;
    unsigned int u32;
} U_CMDLST_SW_BUF_LEN;
typedef union
{
    struct
    {
        unsigned int branching : 1 ;
        unsigned int reserved_0 : 31 ;
    } bits;
    unsigned int u32;
} U_CMDLST_SW_CFG;
typedef union
{
    struct
    {
        unsigned int hw_cfg_buf_len0 : 16 ;
        unsigned int hw_cfg_buf_len1 : 16 ;
    } bits;
    unsigned int u32;
} U_CMDLST_HW_BUF_LEN;
typedef struct
{
    volatile U_CMDLST_CFG CMDLST_CFG;
    volatile unsigned int RESERVED0[3];
    volatile unsigned int CMDLST_SW_AXI_ADDR;
    volatile U_CMDLST_SW_BUF_LEN CMDLST_SW_BUF_LEN;
    volatile U_CMDLST_SW_CFG CMDLST_SW_CFG;
    volatile unsigned int RESERVED1;
    volatile unsigned int CMDLST_HW_AXI_ADDR;
    volatile U_CMDLST_HW_BUF_LEN CMDLST_HW_BUF_LEN;
    volatile unsigned int CMDLST_HW_AXI_ADDR_DUMP;
    volatile unsigned int RESERVED2;
    volatile unsigned int CMDLST_AXI_ADDR_RO;
    volatile unsigned int CMDLST_OTHER_RO0;
    volatile unsigned int CMDLST_OTHER_RO1;
    volatile unsigned int RESERVED3[1009];
} S_CMDLST_REGS_TYPE;
#endif
