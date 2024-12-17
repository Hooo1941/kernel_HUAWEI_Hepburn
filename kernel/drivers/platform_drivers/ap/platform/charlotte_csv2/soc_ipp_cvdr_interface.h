#ifndef __SOC_IPP_CVDR_INTERFACE_H__
#define __SOC_IPP_CVDR_INTERFACE_H__ 
#ifdef __cplusplus
    #if __cplusplus
        extern "C" {
    #endif
#endif
#define SOC_IPP_CVDR_CVDR_IPP_CVDR_CFG_ADDR(base) ((base) + (0x0))
#define SOC_IPP_CVDR_CVDR_IPP_CVDR_WR_QOS_CFG_ADDR(base) ((base) + (0x4))
#define SOC_IPP_CVDR_CVDR_IPP_CVDR_RD_QOS_CFG_ADDR(base) ((base) + (0x8))
#define SOC_IPP_CVDR_CVDR_IPP_FORCE_CLK_ADDR(base) ((base) + (0xc))
#define SOC_IPP_CVDR_CVDR_IPP_CVDR_DEBUG_EN_ADDR(base) ((base) + (0x10))
#define SOC_IPP_CVDR_CVDR_IPP_CVDR_DEBUG_ADDR(base) ((base) + (0x14))
#define SOC_IPP_CVDR_CVDR_IPP_DEBUG_ADDR(base) ((base) + (0x18))
#define SOC_IPP_CVDR_CVDR_IPP_OTHER_RO_ADDR(base) ((base) + (0x1c))
#define SOC_IPP_CVDR_CVDR_IPP_OTHER_RW_ADDR(base) ((base) + (0x20))
#define SOC_IPP_CVDR_CVDR_IPP_NR_RD_CFG_2_ADDR(base) ((base) + (0x40))
#define SOC_IPP_CVDR_CVDR_IPP_NR_RD_LIMITER_2_ADDR(base) ((base) + (0x44))
#define SOC_IPP_CVDR_CVDR_IPP_NR_RD_DEBUG_2_ADDR(base) ((base) + (0x48))
#define SOC_IPP_CVDR_CVDR_IPP_NR_RD_CFG_3_ADDR(base) ((base) + (0x4c))
#define SOC_IPP_CVDR_CVDR_IPP_NR_RD_LIMITER_3_ADDR(base) ((base) + (0x50))
#define SOC_IPP_CVDR_CVDR_IPP_NR_RD_DEBUG_3_ADDR(base) ((base) + (0x54))
#define SOC_IPP_CVDR_CVDR_IPP_NR_RD_CFG_4_ADDR(base) ((base) + (0x58))
#define SOC_IPP_CVDR_CVDR_IPP_NR_RD_LIMITER_4_ADDR(base) ((base) + (0x5c))
#define SOC_IPP_CVDR_CVDR_IPP_NR_RD_DEBUG_4_ADDR(base) ((base) + (0x60))
#define SOC_IPP_CVDR_CVDR_IPP_NR_RD_CFG_7_ADDR(base) ((base) + (0x7c))
#define SOC_IPP_CVDR_CVDR_IPP_NR_RD_LIMITER_7_ADDR(base) ((base) + (0x80))
#define SOC_IPP_CVDR_CVDR_IPP_NR_RD_DEBUG_7_ADDR(base) ((base) + (0x84))
#define SOC_IPP_CVDR_CVDR_IPP_NR_RD_CFG_8_ADDR(base) ((base) + (0x88))
#define SOC_IPP_CVDR_CVDR_IPP_NR_RD_LIMITER_8_ADDR(base) ((base) + (0x8c))
#define SOC_IPP_CVDR_CVDR_IPP_NR_RD_DEBUG_8_ADDR(base) ((base) + (0x90))
#define SOC_IPP_CVDR_CVDR_IPP_NR_WR_CFG_6_ADDR(base) ((base) + (0xdc))
#define SOC_IPP_CVDR_CVDR_IPP_NR_WR_LIMITER_6_ADDR(base) ((base) + (0xe0))
#define SOC_IPP_CVDR_CVDR_IPP_NR_WR_DEBUG_6_ADDR(base) ((base) + (0xe4))
#define SOC_IPP_CVDR_CVDR_IPP_NR_WR_CFG_7_ADDR(base) ((base) + (0xe8))
#define SOC_IPP_CVDR_CVDR_IPP_NR_WR_LIMITER_7_ADDR(base) ((base) + (0xec))
#define SOC_IPP_CVDR_CVDR_IPP_NR_WR_DEBUG_7_ADDR(base) ((base) + (0xf0))
#define SOC_IPP_CVDR_CVDR_IPP_NR_WR_CFG_8_ADDR(base) ((base) + (0xf4))
#define SOC_IPP_CVDR_CVDR_IPP_NR_WR_LIMITER_8_ADDR(base) ((base) + (0xf8))
#define SOC_IPP_CVDR_CVDR_IPP_NR_WR_DEBUG_8_ADDR(base) ((base) + (0xfc))
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_CFG_0_ADDR(base) ((base) + (0x100))
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_LWG_0_ADDR(base) ((base) + (0x104))
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_FHG_0_ADDR(base) ((base) + (0x108))
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_AXI_FS_0_ADDR(base) ((base) + (0x10c))
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_AXI_LINE_0_ADDR(base) ((base) + (0x110))
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_IF_CFG_0_ADDR(base) ((base) + (0x114))
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_LIMITER_0_ADDR(base) ((base) + (0x118))
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_DEBUG_0_ADDR(base) ((base) + (0x11c))
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_CFG_6_ADDR(base) ((base) + (0x1c0))
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_LWG_6_ADDR(base) ((base) + (0x1c4))
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_FHG_6_ADDR(base) ((base) + (0x1c8))
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_AXI_FS_6_ADDR(base) ((base) + (0x1cc))
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_AXI_LINE_6_ADDR(base) ((base) + (0x1d0))
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_IF_CFG_6_ADDR(base) ((base) + (0x1d4))
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_LIMITER_6_ADDR(base) ((base) + (0x1d8))
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_DEBUG_6_ADDR(base) ((base) + (0x1dc))
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_CFG_7_ADDR(base) ((base) + (0x1e0))
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_LWG_7_ADDR(base) ((base) + (0x1e4))
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_FHG_7_ADDR(base) ((base) + (0x1e8))
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_AXI_FS_7_ADDR(base) ((base) + (0x1ec))
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_AXI_LINE_7_ADDR(base) ((base) + (0x1f0))
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_IF_CFG_7_ADDR(base) ((base) + (0x1f4))
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_LIMITER_7_ADDR(base) ((base) + (0x1f8))
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_DEBUG_7_ADDR(base) ((base) + (0x1fc))
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_CFG_9_ADDR(base) ((base) + (0x220))
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_LWG_9_ADDR(base) ((base) + (0x224))
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_FHG_9_ADDR(base) ((base) + (0x228))
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_AXI_FS_9_ADDR(base) ((base) + (0x22c))
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_AXI_LINE_9_ADDR(base) ((base) + (0x230))
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_IF_CFG_9_ADDR(base) ((base) + (0x234))
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_LIMITER_9_ADDR(base) ((base) + (0x238))
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_DEBUG_9_ADDR(base) ((base) + (0x23c))
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_CFG_10_ADDR(base) ((base) + (0x240))
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_LWG_10_ADDR(base) ((base) + (0x244))
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_FHG_10_ADDR(base) ((base) + (0x248))
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_AXI_FS_10_ADDR(base) ((base) + (0x24c))
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_AXI_LINE_10_ADDR(base) ((base) + (0x250))
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_IF_CFG_10_ADDR(base) ((base) + (0x254))
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_LIMITER_10_ADDR(base) ((base) + (0x258))
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_DEBUG_10_ADDR(base) ((base) + (0x25c))
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_CFG_12_ADDR(base) ((base) + (0x280))
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_LWG_12_ADDR(base) ((base) + (0x284))
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_FHG_12_ADDR(base) ((base) + (0x288))
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_AXI_FS_12_ADDR(base) ((base) + (0x28c))
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_AXI_LINE_12_ADDR(base) ((base) + (0x290))
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_IF_CFG_12_ADDR(base) ((base) + (0x294))
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_LIMITER_12_ADDR(base) ((base) + (0x298))
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_DEBUG_12_ADDR(base) ((base) + (0x29c))
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_CFG_13_ADDR(base) ((base) + (0x2a0))
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_LWG_13_ADDR(base) ((base) + (0x2a4))
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_FHG_13_ADDR(base) ((base) + (0x2a8))
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_AXI_FS_13_ADDR(base) ((base) + (0x2ac))
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_AXI_LINE_13_ADDR(base) ((base) + (0x2b0))
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_IF_CFG_13_ADDR(base) ((base) + (0x2b4))
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_LIMITER_13_ADDR(base) ((base) + (0x2b8))
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_DEBUG_13_ADDR(base) ((base) + (0x2bc))
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_CFG_14_ADDR(base) ((base) + (0x2c0))
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_LWG_14_ADDR(base) ((base) + (0x2c4))
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_FHG_14_ADDR(base) ((base) + (0x2c8))
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_AXI_FS_14_ADDR(base) ((base) + (0x2cc))
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_AXI_LINE_14_ADDR(base) ((base) + (0x2d0))
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_IF_CFG_14_ADDR(base) ((base) + (0x2d4))
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_LIMITER_14_ADDR(base) ((base) + (0x2d8))
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_DEBUG_14_ADDR(base) ((base) + (0x2dc))
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_CFG_15_ADDR(base) ((base) + (0x2e0))
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_LWG_15_ADDR(base) ((base) + (0x2e4))
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_FHG_15_ADDR(base) ((base) + (0x2e8))
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_AXI_FS_15_ADDR(base) ((base) + (0x2ec))
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_AXI_LINE_15_ADDR(base) ((base) + (0x2f0))
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_IF_CFG_15_ADDR(base) ((base) + (0x2f4))
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_LIMITER_15_ADDR(base) ((base) + (0x2f8))
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_DEBUG_15_ADDR(base) ((base) + (0x2fc))
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_CFG_16_ADDR(base) ((base) + (0x300))
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_LWG_16_ADDR(base) ((base) + (0x304))
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_FHG_16_ADDR(base) ((base) + (0x308))
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_AXI_FS_16_ADDR(base) ((base) + (0x30c))
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_AXI_LINE_16_ADDR(base) ((base) + (0x310))
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_IF_CFG_16_ADDR(base) ((base) + (0x314))
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_LIMITER_16_ADDR(base) ((base) + (0x318))
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_DEBUG_16_ADDR(base) ((base) + (0x31c))
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_CFG_17_ADDR(base) ((base) + (0x320))
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_LWG_17_ADDR(base) ((base) + (0x324))
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_FHG_17_ADDR(base) ((base) + (0x328))
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_AXI_FS_17_ADDR(base) ((base) + (0x32c))
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_AXI_LINE_17_ADDR(base) ((base) + (0x330))
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_IF_CFG_17_ADDR(base) ((base) + (0x334))
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_LIMITER_17_ADDR(base) ((base) + (0x338))
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_DEBUG_17_ADDR(base) ((base) + (0x33c))
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_CFG_18_ADDR(base) ((base) + (0x340))
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_LWG_18_ADDR(base) ((base) + (0x344))
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_FHG_18_ADDR(base) ((base) + (0x348))
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_AXI_FS_18_ADDR(base) ((base) + (0x34c))
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_AXI_LINE_18_ADDR(base) ((base) + (0x350))
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_IF_CFG_18_ADDR(base) ((base) + (0x354))
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_LIMITER_18_ADDR(base) ((base) + (0x358))
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_DEBUG_18_ADDR(base) ((base) + (0x35c))
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_CFG_19_ADDR(base) ((base) + (0x360))
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_LWG_19_ADDR(base) ((base) + (0x364))
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_FHG_19_ADDR(base) ((base) + (0x368))
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_AXI_FS_19_ADDR(base) ((base) + (0x36c))
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_AXI_LINE_19_ADDR(base) ((base) + (0x370))
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_IF_CFG_19_ADDR(base) ((base) + (0x374))
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_LIMITER_19_ADDR(base) ((base) + (0x378))
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_DEBUG_19_ADDR(base) ((base) + (0x37c))
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_CFG_20_ADDR(base) ((base) + (0x380))
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_LWG_20_ADDR(base) ((base) + (0x384))
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_FHG_20_ADDR(base) ((base) + (0x388))
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_AXI_FS_20_ADDR(base) ((base) + (0x38c))
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_AXI_LINE_20_ADDR(base) ((base) + (0x390))
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_IF_CFG_20_ADDR(base) ((base) + (0x394))
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_LIMITER_20_ADDR(base) ((base) + (0x398))
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_DEBUG_20_ADDR(base) ((base) + (0x39c))
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_CFG_21_ADDR(base) ((base) + (0x3a0))
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_LWG_21_ADDR(base) ((base) + (0x3a4))
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_FHG_21_ADDR(base) ((base) + (0x3a8))
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_AXI_FS_21_ADDR(base) ((base) + (0x3ac))
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_AXI_LINE_21_ADDR(base) ((base) + (0x3b0))
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_IF_CFG_21_ADDR(base) ((base) + (0x3b4))
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_LIMITER_21_ADDR(base) ((base) + (0x3b8))
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_DEBUG_21_ADDR(base) ((base) + (0x3bc))
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_CFG_22_ADDR(base) ((base) + (0x3c0))
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_LWG_22_ADDR(base) ((base) + (0x3c4))
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_FHG_22_ADDR(base) ((base) + (0x3c8))
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_AXI_FS_22_ADDR(base) ((base) + (0x3cc))
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_AXI_LINE_22_ADDR(base) ((base) + (0x3d0))
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_IF_CFG_22_ADDR(base) ((base) + (0x3d4))
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_LIMITER_22_ADDR(base) ((base) + (0x3d8))
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_DEBUG_22_ADDR(base) ((base) + (0x3dc))
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_CFG_23_ADDR(base) ((base) + (0x3e0))
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_LWG_23_ADDR(base) ((base) + (0x3e4))
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_FHG_23_ADDR(base) ((base) + (0x3e8))
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_AXI_FS_23_ADDR(base) ((base) + (0x3ec))
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_AXI_LINE_23_ADDR(base) ((base) + (0x3f0))
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_IF_CFG_23_ADDR(base) ((base) + (0x3f4))
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_LIMITER_23_ADDR(base) ((base) + (0x3f8))
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_DEBUG_23_ADDR(base) ((base) + (0x3fc))
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_CFG_24_ADDR(base) ((base) + (0x400))
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_LWG_24_ADDR(base) ((base) + (0x404))
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_FHG_24_ADDR(base) ((base) + (0x408))
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_AXI_FS_24_ADDR(base) ((base) + (0x40c))
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_AXI_LINE_24_ADDR(base) ((base) + (0x410))
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_IF_CFG_24_ADDR(base) ((base) + (0x414))
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_LIMITER_24_ADDR(base) ((base) + (0x418))
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_DEBUG_24_ADDR(base) ((base) + (0x41c))
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_CFG_25_ADDR(base) ((base) + (0x420))
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_LWG_25_ADDR(base) ((base) + (0x424))
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_FHG_25_ADDR(base) ((base) + (0x428))
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_AXI_FS_25_ADDR(base) ((base) + (0x42c))
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_AXI_LINE_25_ADDR(base) ((base) + (0x430))
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_IF_CFG_25_ADDR(base) ((base) + (0x434))
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_LIMITER_25_ADDR(base) ((base) + (0x438))
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_DEBUG_25_ADDR(base) ((base) + (0x43c))
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_CFG_26_ADDR(base) ((base) + (0x440))
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_LWG_26_ADDR(base) ((base) + (0x444))
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_FHG_26_ADDR(base) ((base) + (0x448))
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_AXI_FS_26_ADDR(base) ((base) + (0x44c))
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_AXI_LINE_26_ADDR(base) ((base) + (0x450))
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_IF_CFG_26_ADDR(base) ((base) + (0x454))
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_LIMITER_26_ADDR(base) ((base) + (0x458))
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_DEBUG_26_ADDR(base) ((base) + (0x45c))
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_CFG_27_ADDR(base) ((base) + (0x460))
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_LWG_27_ADDR(base) ((base) + (0x464))
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_FHG_27_ADDR(base) ((base) + (0x468))
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_AXI_FS_27_ADDR(base) ((base) + (0x46c))
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_AXI_LINE_27_ADDR(base) ((base) + (0x470))
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_IF_CFG_27_ADDR(base) ((base) + (0x474))
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_LIMITER_27_ADDR(base) ((base) + (0x478))
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_DEBUG_27_ADDR(base) ((base) + (0x47c))
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_CFG_28_ADDR(base) ((base) + (0x480))
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_LWG_28_ADDR(base) ((base) + (0x484))
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_FHG_28_ADDR(base) ((base) + (0x488))
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_AXI_FS_28_ADDR(base) ((base) + (0x48c))
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_AXI_LINE_28_ADDR(base) ((base) + (0x490))
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_IF_CFG_28_ADDR(base) ((base) + (0x494))
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_LIMITER_28_ADDR(base) ((base) + (0x498))
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_DEBUG_28_ADDR(base) ((base) + (0x49c))
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_CFG_29_ADDR(base) ((base) + (0x4a0))
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_LWG_29_ADDR(base) ((base) + (0x4a4))
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_FHG_29_ADDR(base) ((base) + (0x4a8))
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_AXI_FS_29_ADDR(base) ((base) + (0x4ac))
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_AXI_LINE_29_ADDR(base) ((base) + (0x4b0))
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_IF_CFG_29_ADDR(base) ((base) + (0x4b4))
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_LIMITER_29_ADDR(base) ((base) + (0x4b8))
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_DEBUG_29_ADDR(base) ((base) + (0x4bc))
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_IF_CFG_0_ADDR(base) ((base) + (0x500))
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_LIMITER_0_ADDR(base) ((base) + (0x504))
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_SID_AXI_LINE_sid0_0_ADDR(base) ((base) + (0x50c))
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_SID_PREFETCH_sid0_0_ADDR(base) ((base) + (0x510))
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_SID_CFG_sid0_0_ADDR(base) ((base) + (0x514))
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_SID_AXI_FS_sid0_0_ADDR(base) ((base) + (0x518))
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_SID_DEBUG_sid0_0_ADDR(base) ((base) + (0x51c))
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_IF_CFG_3_ADDR(base) ((base) + (0x560))
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_LIMITER_3_ADDR(base) ((base) + (0x564))
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_SID_AXI_LINE_sid0_3_ADDR(base) ((base) + (0x56c))
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_SID_PREFETCH_sid0_3_ADDR(base) ((base) + (0x570))
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_SID_CFG_sid0_3_ADDR(base) ((base) + (0x574))
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_SID_AXI_FS_sid0_3_ADDR(base) ((base) + (0x578))
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_SID_DEBUG_sid0_3_ADDR(base) ((base) + (0x57c))
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_IF_CFG_8_ADDR(base) ((base) + (0x600))
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_LIMITER_8_ADDR(base) ((base) + (0x604))
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_SID_AXI_LINE_sid0_8_ADDR(base) ((base) + (0x60c))
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_SID_PREFETCH_sid0_8_ADDR(base) ((base) + (0x610))
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_SID_CFG_sid0_8_ADDR(base) ((base) + (0x614))
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_SID_AXI_FS_sid0_8_ADDR(base) ((base) + (0x618))
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_SID_DEBUG_sid0_8_ADDR(base) ((base) + (0x61c))
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_IF_CFG_9_ADDR(base) ((base) + (0x620))
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_LIMITER_9_ADDR(base) ((base) + (0x624))
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_SID_AXI_LINE_sid0_9_ADDR(base) ((base) + (0x62c))
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_SID_PREFETCH_sid0_9_ADDR(base) ((base) + (0x630))
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_SID_CFG_sid0_9_ADDR(base) ((base) + (0x634))
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_SID_AXI_FS_sid0_9_ADDR(base) ((base) + (0x638))
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_SID_DEBUG_sid0_9_ADDR(base) ((base) + (0x63c))
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_IF_CFG_10_ADDR(base) ((base) + (0x640))
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_LIMITER_10_ADDR(base) ((base) + (0x644))
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_SID_AXI_LINE_sid0_10_ADDR(base) ((base) + (0x64c))
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_SID_PREFETCH_sid0_10_ADDR(base) ((base) + (0x650))
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_SID_CFG_sid0_10_ADDR(base) ((base) + (0x654))
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_SID_AXI_FS_sid0_10_ADDR(base) ((base) + (0x658))
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_SID_DEBUG_sid0_10_ADDR(base) ((base) + (0x65c))
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_IF_CFG_11_ADDR(base) ((base) + (0x660))
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_LIMITER_11_ADDR(base) ((base) + (0x664))
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_SID_AXI_LINE_sid0_11_ADDR(base) ((base) + (0x66c))
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_SID_PREFETCH_sid0_11_ADDR(base) ((base) + (0x670))
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_SID_CFG_sid0_11_ADDR(base) ((base) + (0x674))
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_SID_AXI_FS_sid0_11_ADDR(base) ((base) + (0x678))
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_SID_DEBUG_sid0_11_ADDR(base) ((base) + (0x67c))
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_IF_CFG_12_ADDR(base) ((base) + (0x680))
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_LIMITER_12_ADDR(base) ((base) + (0x684))
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_SID_AXI_LINE_sid0_12_ADDR(base) ((base) + (0x68c))
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_SID_PREFETCH_sid0_12_ADDR(base) ((base) + (0x690))
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_SID_CFG_sid0_12_ADDR(base) ((base) + (0x694))
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_SID_AXI_FS_sid0_12_ADDR(base) ((base) + (0x698))
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_SID_DEBUG_sid0_12_ADDR(base) ((base) + (0x69c))
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_IF_CFG_13_ADDR(base) ((base) + (0x6a0))
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_LIMITER_13_ADDR(base) ((base) + (0x6a4))
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_SID_AXI_LINE_sid0_13_ADDR(base) ((base) + (0x6ac))
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_SID_PREFETCH_sid0_13_ADDR(base) ((base) + (0x6b0))
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_SID_CFG_sid0_13_ADDR(base) ((base) + (0x6b4))
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_SID_AXI_FS_sid0_13_ADDR(base) ((base) + (0x6b8))
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_SID_DEBUG_sid0_13_ADDR(base) ((base) + (0x6bc))
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_IF_CFG_14_ADDR(base) ((base) + (0x6c0))
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_LIMITER_14_ADDR(base) ((base) + (0x6c4))
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_SID_AXI_LINE_sid0_14_ADDR(base) ((base) + (0x6cc))
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_SID_PREFETCH_sid0_14_ADDR(base) ((base) + (0x6d0))
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_SID_CFG_sid0_14_ADDR(base) ((base) + (0x6d4))
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_SID_AXI_FS_sid0_14_ADDR(base) ((base) + (0x6d8))
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_SID_DEBUG_sid0_14_ADDR(base) ((base) + (0x6dc))
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_IF_CFG_15_ADDR(base) ((base) + (0x6e0))
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_LIMITER_15_ADDR(base) ((base) + (0x6e4))
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_SID_AXI_LINE_sid0_15_ADDR(base) ((base) + (0x6ec))
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_SID_PREFETCH_sid0_15_ADDR(base) ((base) + (0x6f0))
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_SID_CFG_sid0_15_ADDR(base) ((base) + (0x6f4))
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_SID_AXI_FS_sid0_15_ADDR(base) ((base) + (0x6f8))
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_SID_DEBUG_sid0_15_ADDR(base) ((base) + (0x6fc))
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_IF_CFG_16_ADDR(base) ((base) + (0x700))
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_LIMITER_16_ADDR(base) ((base) + (0x704))
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_SID_AXI_LINE_sid0_16_ADDR(base) ((base) + (0x70c))
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_SID_PREFETCH_sid0_16_ADDR(base) ((base) + (0x710))
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_SID_CFG_sid0_16_ADDR(base) ((base) + (0x714))
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_SID_AXI_FS_sid0_16_ADDR(base) ((base) + (0x718))
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_SID_DEBUG_sid0_16_ADDR(base) ((base) + (0x71c))
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_IF_CFG_17_ADDR(base) ((base) + (0x720))
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_LIMITER_17_ADDR(base) ((base) + (0x724))
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_SID_AXI_LINE_sid0_17_ADDR(base) ((base) + (0x72c))
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_SID_PREFETCH_sid0_17_ADDR(base) ((base) + (0x730))
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_SID_CFG_sid0_17_ADDR(base) ((base) + (0x734))
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_SID_AXI_FS_sid0_17_ADDR(base) ((base) + (0x738))
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_SID_DEBUG_sid0_17_ADDR(base) ((base) + (0x73c))
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_IF_CFG_20_ADDR(base) ((base) + (0x780))
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_LIMITER_20_ADDR(base) ((base) + (0x784))
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_SID_AXI_LINE_sid0_20_ADDR(base) ((base) + (0x78c))
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_SID_PREFETCH_sid0_20_ADDR(base) ((base) + (0x790))
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_SID_CFG_sid0_20_ADDR(base) ((base) + (0x794))
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_SID_AXI_FS_sid0_20_ADDR(base) ((base) + (0x798))
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_SID_DEBUG_sid0_20_ADDR(base) ((base) + (0x79c))
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_IF_CFG_21_ADDR(base) ((base) + (0x7a0))
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_LIMITER_21_ADDR(base) ((base) + (0x7a4))
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_SID_AXI_LINE_sid0_21_ADDR(base) ((base) + (0x7ac))
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_SID_PREFETCH_sid0_21_ADDR(base) ((base) + (0x7b0))
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_SID_CFG_sid0_21_ADDR(base) ((base) + (0x7b4))
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_SID_AXI_FS_sid0_21_ADDR(base) ((base) + (0x7b8))
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_SID_DEBUG_sid0_21_ADDR(base) ((base) + (0x7bc))
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_IF_CFG_22_ADDR(base) ((base) + (0x7c0))
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_LIMITER_22_ADDR(base) ((base) + (0x7c4))
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_SID_AXI_LINE_sid0_22_ADDR(base) ((base) + (0x7cc))
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_SID_PREFETCH_sid0_22_ADDR(base) ((base) + (0x7d0))
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_SID_CFG_sid0_22_ADDR(base) ((base) + (0x7d4))
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_SID_AXI_FS_sid0_22_ADDR(base) ((base) + (0x7d8))
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_SID_DEBUG_sid0_22_ADDR(base) ((base) + (0x7dc))
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_IF_CFG_23_ADDR(base) ((base) + (0x7e0))
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_LIMITER_23_ADDR(base) ((base) + (0x7e4))
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_SID_AXI_LINE_sid0_23_ADDR(base) ((base) + (0x7ec))
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_SID_PREFETCH_sid0_23_ADDR(base) ((base) + (0x7f0))
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_SID_CFG_sid0_23_ADDR(base) ((base) + (0x7f4))
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_SID_AXI_FS_sid0_23_ADDR(base) ((base) + (0x7f8))
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_SID_DEBUG_sid0_23_ADDR(base) ((base) + (0x7fc))
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_IF_CFG_24_ADDR(base) ((base) + (0x800))
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_LIMITER_24_ADDR(base) ((base) + (0x804))
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_SID_AXI_LINE_sid0_24_ADDR(base) ((base) + (0x80c))
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_SID_PREFETCH_sid0_24_ADDR(base) ((base) + (0x810))
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_SID_CFG_sid0_24_ADDR(base) ((base) + (0x814))
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_SID_AXI_FS_sid0_24_ADDR(base) ((base) + (0x818))
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_SID_DEBUG_sid0_24_ADDR(base) ((base) + (0x81c))
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_IF_CFG_25_ADDR(base) ((base) + (0x820))
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_LIMITER_25_ADDR(base) ((base) + (0x824))
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_SID_AXI_LINE_sid0_25_ADDR(base) ((base) + (0x82c))
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_SID_PREFETCH_sid0_25_ADDR(base) ((base) + (0x830))
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_SID_CFG_sid0_25_ADDR(base) ((base) + (0x834))
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_SID_AXI_FS_sid0_25_ADDR(base) ((base) + (0x838))
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_SID_DEBUG_sid0_25_ADDR(base) ((base) + (0x83c))
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_IF_CFG_30_ADDR(base) ((base) + (0x8c0))
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_LIMITER_30_ADDR(base) ((base) + (0x8c4))
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_SID_AXI_LINE_sid0_30_ADDR(base) ((base) + (0x8cc))
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_SID_PREFETCH_sid0_30_ADDR(base) ((base) + (0x8d0))
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_SID_CFG_sid0_30_ADDR(base) ((base) + (0x8d4))
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_SID_AXI_FS_sid0_30_ADDR(base) ((base) + (0x8d8))
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_SID_DEBUG_sid0_30_ADDR(base) ((base) + (0x8dc))
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int axiwrite_du_threshold : 6;
        unsigned int reserved_0 : 2;
        unsigned int du_threshold_reached : 8;
        unsigned int max_axiread_id : 6;
        unsigned int axi_wr_buffer_behavior : 2;
        unsigned int max_axiwrite_id : 6;
        unsigned int reserved_1 : 2;
    } reg;
} SOC_IPP_CVDR_CVDR_IPP_CVDR_CFG_UNION;
#endif
#define SOC_IPP_CVDR_CVDR_IPP_CVDR_CFG_axiwrite_du_threshold_START (0)
#define SOC_IPP_CVDR_CVDR_IPP_CVDR_CFG_axiwrite_du_threshold_END (5)
#define SOC_IPP_CVDR_CVDR_IPP_CVDR_CFG_du_threshold_reached_START (8)
#define SOC_IPP_CVDR_CVDR_IPP_CVDR_CFG_du_threshold_reached_END (15)
#define SOC_IPP_CVDR_CVDR_IPP_CVDR_CFG_max_axiread_id_START (16)
#define SOC_IPP_CVDR_CVDR_IPP_CVDR_CFG_max_axiread_id_END (21)
#define SOC_IPP_CVDR_CVDR_IPP_CVDR_CFG_axi_wr_buffer_behavior_START (22)
#define SOC_IPP_CVDR_CVDR_IPP_CVDR_CFG_axi_wr_buffer_behavior_END (23)
#define SOC_IPP_CVDR_CVDR_IPP_CVDR_CFG_max_axiwrite_id_START (24)
#define SOC_IPP_CVDR_CVDR_IPP_CVDR_CFG_max_axiwrite_id_END (29)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int wr_qos_threshold_01_stop : 4;
        unsigned int wr_qos_threshold_01_start : 4;
        unsigned int wr_qos_threshold_10_stop : 4;
        unsigned int wr_qos_threshold_10_start : 4;
        unsigned int wr_qos_threshold_11_stop : 4;
        unsigned int wr_qos_threshold_11_start : 4;
        unsigned int reserved : 2;
        unsigned int wr_qos_min : 2;
        unsigned int wr_qos_max : 2;
        unsigned int wr_qos_sr : 2;
    } reg;
} SOC_IPP_CVDR_CVDR_IPP_CVDR_WR_QOS_CFG_UNION;
#endif
#define SOC_IPP_CVDR_CVDR_IPP_CVDR_WR_QOS_CFG_wr_qos_threshold_01_stop_START (0)
#define SOC_IPP_CVDR_CVDR_IPP_CVDR_WR_QOS_CFG_wr_qos_threshold_01_stop_END (3)
#define SOC_IPP_CVDR_CVDR_IPP_CVDR_WR_QOS_CFG_wr_qos_threshold_01_start_START (4)
#define SOC_IPP_CVDR_CVDR_IPP_CVDR_WR_QOS_CFG_wr_qos_threshold_01_start_END (7)
#define SOC_IPP_CVDR_CVDR_IPP_CVDR_WR_QOS_CFG_wr_qos_threshold_10_stop_START (8)
#define SOC_IPP_CVDR_CVDR_IPP_CVDR_WR_QOS_CFG_wr_qos_threshold_10_stop_END (11)
#define SOC_IPP_CVDR_CVDR_IPP_CVDR_WR_QOS_CFG_wr_qos_threshold_10_start_START (12)
#define SOC_IPP_CVDR_CVDR_IPP_CVDR_WR_QOS_CFG_wr_qos_threshold_10_start_END (15)
#define SOC_IPP_CVDR_CVDR_IPP_CVDR_WR_QOS_CFG_wr_qos_threshold_11_stop_START (16)
#define SOC_IPP_CVDR_CVDR_IPP_CVDR_WR_QOS_CFG_wr_qos_threshold_11_stop_END (19)
#define SOC_IPP_CVDR_CVDR_IPP_CVDR_WR_QOS_CFG_wr_qos_threshold_11_start_START (20)
#define SOC_IPP_CVDR_CVDR_IPP_CVDR_WR_QOS_CFG_wr_qos_threshold_11_start_END (23)
#define SOC_IPP_CVDR_CVDR_IPP_CVDR_WR_QOS_CFG_wr_qos_min_START (26)
#define SOC_IPP_CVDR_CVDR_IPP_CVDR_WR_QOS_CFG_wr_qos_min_END (27)
#define SOC_IPP_CVDR_CVDR_IPP_CVDR_WR_QOS_CFG_wr_qos_max_START (28)
#define SOC_IPP_CVDR_CVDR_IPP_CVDR_WR_QOS_CFG_wr_qos_max_END (29)
#define SOC_IPP_CVDR_CVDR_IPP_CVDR_WR_QOS_CFG_wr_qos_sr_START (30)
#define SOC_IPP_CVDR_CVDR_IPP_CVDR_WR_QOS_CFG_wr_qos_sr_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int rd_qos_threshold_01_stop : 4;
        unsigned int rd_qos_threshold_01_start : 4;
        unsigned int rd_qos_threshold_10_stop : 4;
        unsigned int rd_qos_threshold_10_start : 4;
        unsigned int rd_qos_threshold_11_stop : 4;
        unsigned int rd_qos_threshold_11_start : 4;
        unsigned int reserved : 2;
        unsigned int rd_qos_min : 2;
        unsigned int rd_qos_max : 2;
        unsigned int rd_qos_sr : 2;
    } reg;
} SOC_IPP_CVDR_CVDR_IPP_CVDR_RD_QOS_CFG_UNION;
#endif
#define SOC_IPP_CVDR_CVDR_IPP_CVDR_RD_QOS_CFG_rd_qos_threshold_01_stop_START (0)
#define SOC_IPP_CVDR_CVDR_IPP_CVDR_RD_QOS_CFG_rd_qos_threshold_01_stop_END (3)
#define SOC_IPP_CVDR_CVDR_IPP_CVDR_RD_QOS_CFG_rd_qos_threshold_01_start_START (4)
#define SOC_IPP_CVDR_CVDR_IPP_CVDR_RD_QOS_CFG_rd_qos_threshold_01_start_END (7)
#define SOC_IPP_CVDR_CVDR_IPP_CVDR_RD_QOS_CFG_rd_qos_threshold_10_stop_START (8)
#define SOC_IPP_CVDR_CVDR_IPP_CVDR_RD_QOS_CFG_rd_qos_threshold_10_stop_END (11)
#define SOC_IPP_CVDR_CVDR_IPP_CVDR_RD_QOS_CFG_rd_qos_threshold_10_start_START (12)
#define SOC_IPP_CVDR_CVDR_IPP_CVDR_RD_QOS_CFG_rd_qos_threshold_10_start_END (15)
#define SOC_IPP_CVDR_CVDR_IPP_CVDR_RD_QOS_CFG_rd_qos_threshold_11_stop_START (16)
#define SOC_IPP_CVDR_CVDR_IPP_CVDR_RD_QOS_CFG_rd_qos_threshold_11_stop_END (19)
#define SOC_IPP_CVDR_CVDR_IPP_CVDR_RD_QOS_CFG_rd_qos_threshold_11_start_START (20)
#define SOC_IPP_CVDR_CVDR_IPP_CVDR_RD_QOS_CFG_rd_qos_threshold_11_start_END (23)
#define SOC_IPP_CVDR_CVDR_IPP_CVDR_RD_QOS_CFG_rd_qos_min_START (26)
#define SOC_IPP_CVDR_CVDR_IPP_CVDR_RD_QOS_CFG_rd_qos_min_END (27)
#define SOC_IPP_CVDR_CVDR_IPP_CVDR_RD_QOS_CFG_rd_qos_max_START (28)
#define SOC_IPP_CVDR_CVDR_IPP_CVDR_RD_QOS_CFG_rd_qos_max_END (29)
#define SOC_IPP_CVDR_CVDR_IPP_CVDR_RD_QOS_CFG_rd_qos_sr_START (30)
#define SOC_IPP_CVDR_CVDR_IPP_CVDR_RD_QOS_CFG_rd_qos_sr_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int force_vprd_clk_on : 1;
        unsigned int force_vpwr_clk_on : 1;
        unsigned int force_nrrd_clk_on : 1;
        unsigned int force_nrwr_clk_on : 1;
        unsigned int force_axi_rd_clk_on : 1;
        unsigned int force_axi_wr_clk_on : 1;
        unsigned int force_du_rd_clk_on : 1;
        unsigned int force_du_wr_clk_on : 1;
        unsigned int force_cfg_clk_on : 1;
        unsigned int reserved : 23;
    } reg;
} SOC_IPP_CVDR_CVDR_IPP_FORCE_CLK_UNION;
#endif
#define SOC_IPP_CVDR_CVDR_IPP_FORCE_CLK_force_vprd_clk_on_START (0)
#define SOC_IPP_CVDR_CVDR_IPP_FORCE_CLK_force_vprd_clk_on_END (0)
#define SOC_IPP_CVDR_CVDR_IPP_FORCE_CLK_force_vpwr_clk_on_START (1)
#define SOC_IPP_CVDR_CVDR_IPP_FORCE_CLK_force_vpwr_clk_on_END (1)
#define SOC_IPP_CVDR_CVDR_IPP_FORCE_CLK_force_nrrd_clk_on_START (2)
#define SOC_IPP_CVDR_CVDR_IPP_FORCE_CLK_force_nrrd_clk_on_END (2)
#define SOC_IPP_CVDR_CVDR_IPP_FORCE_CLK_force_nrwr_clk_on_START (3)
#define SOC_IPP_CVDR_CVDR_IPP_FORCE_CLK_force_nrwr_clk_on_END (3)
#define SOC_IPP_CVDR_CVDR_IPP_FORCE_CLK_force_axi_rd_clk_on_START (4)
#define SOC_IPP_CVDR_CVDR_IPP_FORCE_CLK_force_axi_rd_clk_on_END (4)
#define SOC_IPP_CVDR_CVDR_IPP_FORCE_CLK_force_axi_wr_clk_on_START (5)
#define SOC_IPP_CVDR_CVDR_IPP_FORCE_CLK_force_axi_wr_clk_on_END (5)
#define SOC_IPP_CVDR_CVDR_IPP_FORCE_CLK_force_du_rd_clk_on_START (6)
#define SOC_IPP_CVDR_CVDR_IPP_FORCE_CLK_force_du_rd_clk_on_END (6)
#define SOC_IPP_CVDR_CVDR_IPP_FORCE_CLK_force_du_wr_clk_on_START (7)
#define SOC_IPP_CVDR_CVDR_IPP_FORCE_CLK_force_du_wr_clk_on_END (7)
#define SOC_IPP_CVDR_CVDR_IPP_FORCE_CLK_force_cfg_clk_on_START (8)
#define SOC_IPP_CVDR_CVDR_IPP_FORCE_CLK_force_cfg_clk_on_END (8)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int wr_peak_en : 1;
        unsigned int reserved_0 : 7;
        unsigned int rd_peak_en : 1;
        unsigned int reserved_1 : 23;
    } reg;
} SOC_IPP_CVDR_CVDR_IPP_CVDR_DEBUG_EN_UNION;
#endif
#define SOC_IPP_CVDR_CVDR_IPP_CVDR_DEBUG_EN_wr_peak_en_START (0)
#define SOC_IPP_CVDR_CVDR_IPP_CVDR_DEBUG_EN_wr_peak_en_END (0)
#define SOC_IPP_CVDR_CVDR_IPP_CVDR_DEBUG_EN_rd_peak_en_START (8)
#define SOC_IPP_CVDR_CVDR_IPP_CVDR_DEBUG_EN_rd_peak_en_END (8)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int wr_peak : 8;
        unsigned int rd_peak : 8;
        unsigned int reserved : 16;
    } reg;
} SOC_IPP_CVDR_CVDR_IPP_CVDR_DEBUG_UNION;
#endif
#define SOC_IPP_CVDR_CVDR_IPP_CVDR_DEBUG_wr_peak_START (0)
#define SOC_IPP_CVDR_CVDR_IPP_CVDR_DEBUG_wr_peak_END (7)
#define SOC_IPP_CVDR_CVDR_IPP_CVDR_DEBUG_rd_peak_START (8)
#define SOC_IPP_CVDR_CVDR_IPP_CVDR_DEBUG_rd_peak_END (15)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int debug : 32;
    } reg;
} SOC_IPP_CVDR_CVDR_IPP_DEBUG_UNION;
#endif
#define SOC_IPP_CVDR_CVDR_IPP_DEBUG_debug_START (0)
#define SOC_IPP_CVDR_CVDR_IPP_DEBUG_debug_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int other_ro : 32;
    } reg;
} SOC_IPP_CVDR_CVDR_IPP_OTHER_RO_UNION;
#endif
#define SOC_IPP_CVDR_CVDR_IPP_OTHER_RO_other_ro_START (0)
#define SOC_IPP_CVDR_CVDR_IPP_OTHER_RO_other_ro_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int other_rw : 32;
    } reg;
} SOC_IPP_CVDR_CVDR_IPP_OTHER_RW_UNION;
#endif
#define SOC_IPP_CVDR_CVDR_IPP_OTHER_RW_other_rw_START (0)
#define SOC_IPP_CVDR_CVDR_IPP_OTHER_RW_other_rw_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int reserved_0 : 5;
        unsigned int nrrd_allocated_du_2 : 5;
        unsigned int reserved_1 : 6;
        unsigned int nr_rd_stop_enable_du_threshold_reached_2 : 1;
        unsigned int nr_rd_stop_enable_flux_ctrl_2 : 1;
        unsigned int nr_rd_stop_enable_pressure_2 : 1;
        unsigned int reserved_2 : 5;
        unsigned int nr_rd_stop_ok_2 : 1;
        unsigned int nr_rd_stop_2 : 1;
        unsigned int reserved_3 : 5;
        unsigned int nrrd_enable_2 : 1;
    } reg;
} SOC_IPP_CVDR_CVDR_IPP_NR_RD_CFG_2_UNION;
#endif
#define SOC_IPP_CVDR_CVDR_IPP_NR_RD_CFG_2_nrrd_allocated_du_2_START (5)
#define SOC_IPP_CVDR_CVDR_IPP_NR_RD_CFG_2_nrrd_allocated_du_2_END (9)
#define SOC_IPP_CVDR_CVDR_IPP_NR_RD_CFG_2_nr_rd_stop_enable_du_threshold_reached_2_START (16)
#define SOC_IPP_CVDR_CVDR_IPP_NR_RD_CFG_2_nr_rd_stop_enable_du_threshold_reached_2_END (16)
#define SOC_IPP_CVDR_CVDR_IPP_NR_RD_CFG_2_nr_rd_stop_enable_flux_ctrl_2_START (17)
#define SOC_IPP_CVDR_CVDR_IPP_NR_RD_CFG_2_nr_rd_stop_enable_flux_ctrl_2_END (17)
#define SOC_IPP_CVDR_CVDR_IPP_NR_RD_CFG_2_nr_rd_stop_enable_pressure_2_START (18)
#define SOC_IPP_CVDR_CVDR_IPP_NR_RD_CFG_2_nr_rd_stop_enable_pressure_2_END (18)
#define SOC_IPP_CVDR_CVDR_IPP_NR_RD_CFG_2_nr_rd_stop_ok_2_START (24)
#define SOC_IPP_CVDR_CVDR_IPP_NR_RD_CFG_2_nr_rd_stop_ok_2_END (24)
#define SOC_IPP_CVDR_CVDR_IPP_NR_RD_CFG_2_nr_rd_stop_2_START (25)
#define SOC_IPP_CVDR_CVDR_IPP_NR_RD_CFG_2_nr_rd_stop_2_END (25)
#define SOC_IPP_CVDR_CVDR_IPP_NR_RD_CFG_2_nrrd_enable_2_START (31)
#define SOC_IPP_CVDR_CVDR_IPP_NR_RD_CFG_2_nrrd_enable_2_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int nrrd_access_limiter_0_2 : 4;
        unsigned int nrrd_access_limiter_1_2 : 4;
        unsigned int nrrd_access_limiter_2_2 : 4;
        unsigned int nrrd_access_limiter_3_2 : 4;
        unsigned int reserved_0 : 8;
        unsigned int nrrd_access_limiter_reload_2 : 4;
        unsigned int reserved_1 : 4;
    } reg;
} SOC_IPP_CVDR_CVDR_IPP_NR_RD_LIMITER_2_UNION;
#endif
#define SOC_IPP_CVDR_CVDR_IPP_NR_RD_LIMITER_2_nrrd_access_limiter_0_2_START (0)
#define SOC_IPP_CVDR_CVDR_IPP_NR_RD_LIMITER_2_nrrd_access_limiter_0_2_END (3)
#define SOC_IPP_CVDR_CVDR_IPP_NR_RD_LIMITER_2_nrrd_access_limiter_1_2_START (4)
#define SOC_IPP_CVDR_CVDR_IPP_NR_RD_LIMITER_2_nrrd_access_limiter_1_2_END (7)
#define SOC_IPP_CVDR_CVDR_IPP_NR_RD_LIMITER_2_nrrd_access_limiter_2_2_START (8)
#define SOC_IPP_CVDR_CVDR_IPP_NR_RD_LIMITER_2_nrrd_access_limiter_2_2_END (11)
#define SOC_IPP_CVDR_CVDR_IPP_NR_RD_LIMITER_2_nrrd_access_limiter_3_2_START (12)
#define SOC_IPP_CVDR_CVDR_IPP_NR_RD_LIMITER_2_nrrd_access_limiter_3_2_END (15)
#define SOC_IPP_CVDR_CVDR_IPP_NR_RD_LIMITER_2_nrrd_access_limiter_reload_2_START (24)
#define SOC_IPP_CVDR_CVDR_IPP_NR_RD_LIMITER_2_nrrd_access_limiter_reload_2_END (27)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int nr_rd_debug_2 : 32;
    } reg;
} SOC_IPP_CVDR_CVDR_IPP_NR_RD_DEBUG_2_UNION;
#endif
#define SOC_IPP_CVDR_CVDR_IPP_NR_RD_DEBUG_2_nr_rd_debug_2_START (0)
#define SOC_IPP_CVDR_CVDR_IPP_NR_RD_DEBUG_2_nr_rd_debug_2_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int reserved_0 : 5;
        unsigned int nrrd_allocated_du_3 : 5;
        unsigned int reserved_1 : 6;
        unsigned int nr_rd_stop_enable_du_threshold_reached_3 : 1;
        unsigned int nr_rd_stop_enable_flux_ctrl_3 : 1;
        unsigned int nr_rd_stop_enable_pressure_3 : 1;
        unsigned int reserved_2 : 5;
        unsigned int nr_rd_stop_ok_3 : 1;
        unsigned int nr_rd_stop_3 : 1;
        unsigned int reserved_3 : 5;
        unsigned int nrrd_enable_3 : 1;
    } reg;
} SOC_IPP_CVDR_CVDR_IPP_NR_RD_CFG_3_UNION;
#endif
#define SOC_IPP_CVDR_CVDR_IPP_NR_RD_CFG_3_nrrd_allocated_du_3_START (5)
#define SOC_IPP_CVDR_CVDR_IPP_NR_RD_CFG_3_nrrd_allocated_du_3_END (9)
#define SOC_IPP_CVDR_CVDR_IPP_NR_RD_CFG_3_nr_rd_stop_enable_du_threshold_reached_3_START (16)
#define SOC_IPP_CVDR_CVDR_IPP_NR_RD_CFG_3_nr_rd_stop_enable_du_threshold_reached_3_END (16)
#define SOC_IPP_CVDR_CVDR_IPP_NR_RD_CFG_3_nr_rd_stop_enable_flux_ctrl_3_START (17)
#define SOC_IPP_CVDR_CVDR_IPP_NR_RD_CFG_3_nr_rd_stop_enable_flux_ctrl_3_END (17)
#define SOC_IPP_CVDR_CVDR_IPP_NR_RD_CFG_3_nr_rd_stop_enable_pressure_3_START (18)
#define SOC_IPP_CVDR_CVDR_IPP_NR_RD_CFG_3_nr_rd_stop_enable_pressure_3_END (18)
#define SOC_IPP_CVDR_CVDR_IPP_NR_RD_CFG_3_nr_rd_stop_ok_3_START (24)
#define SOC_IPP_CVDR_CVDR_IPP_NR_RD_CFG_3_nr_rd_stop_ok_3_END (24)
#define SOC_IPP_CVDR_CVDR_IPP_NR_RD_CFG_3_nr_rd_stop_3_START (25)
#define SOC_IPP_CVDR_CVDR_IPP_NR_RD_CFG_3_nr_rd_stop_3_END (25)
#define SOC_IPP_CVDR_CVDR_IPP_NR_RD_CFG_3_nrrd_enable_3_START (31)
#define SOC_IPP_CVDR_CVDR_IPP_NR_RD_CFG_3_nrrd_enable_3_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int nrrd_access_limiter_0_3 : 4;
        unsigned int nrrd_access_limiter_1_3 : 4;
        unsigned int nrrd_access_limiter_2_3 : 4;
        unsigned int nrrd_access_limiter_3_3 : 4;
        unsigned int reserved_0 : 8;
        unsigned int nrrd_access_limiter_reload_3 : 4;
        unsigned int reserved_1 : 4;
    } reg;
} SOC_IPP_CVDR_CVDR_IPP_NR_RD_LIMITER_3_UNION;
#endif
#define SOC_IPP_CVDR_CVDR_IPP_NR_RD_LIMITER_3_nrrd_access_limiter_0_3_START (0)
#define SOC_IPP_CVDR_CVDR_IPP_NR_RD_LIMITER_3_nrrd_access_limiter_0_3_END (3)
#define SOC_IPP_CVDR_CVDR_IPP_NR_RD_LIMITER_3_nrrd_access_limiter_1_3_START (4)
#define SOC_IPP_CVDR_CVDR_IPP_NR_RD_LIMITER_3_nrrd_access_limiter_1_3_END (7)
#define SOC_IPP_CVDR_CVDR_IPP_NR_RD_LIMITER_3_nrrd_access_limiter_2_3_START (8)
#define SOC_IPP_CVDR_CVDR_IPP_NR_RD_LIMITER_3_nrrd_access_limiter_2_3_END (11)
#define SOC_IPP_CVDR_CVDR_IPP_NR_RD_LIMITER_3_nrrd_access_limiter_3_3_START (12)
#define SOC_IPP_CVDR_CVDR_IPP_NR_RD_LIMITER_3_nrrd_access_limiter_3_3_END (15)
#define SOC_IPP_CVDR_CVDR_IPP_NR_RD_LIMITER_3_nrrd_access_limiter_reload_3_START (24)
#define SOC_IPP_CVDR_CVDR_IPP_NR_RD_LIMITER_3_nrrd_access_limiter_reload_3_END (27)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int nr_rd_debug_3 : 32;
    } reg;
} SOC_IPP_CVDR_CVDR_IPP_NR_RD_DEBUG_3_UNION;
#endif
#define SOC_IPP_CVDR_CVDR_IPP_NR_RD_DEBUG_3_nr_rd_debug_3_START (0)
#define SOC_IPP_CVDR_CVDR_IPP_NR_RD_DEBUG_3_nr_rd_debug_3_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int reserved_0 : 5;
        unsigned int nrrd_allocated_du_4 : 5;
        unsigned int reserved_1 : 6;
        unsigned int nr_rd_stop_enable_du_threshold_reached_4 : 1;
        unsigned int nr_rd_stop_enable_flux_ctrl_4 : 1;
        unsigned int nr_rd_stop_enable_pressure_4 : 1;
        unsigned int reserved_2 : 5;
        unsigned int nr_rd_stop_ok_4 : 1;
        unsigned int nr_rd_stop_4 : 1;
        unsigned int reserved_3 : 5;
        unsigned int nrrd_enable_4 : 1;
    } reg;
} SOC_IPP_CVDR_CVDR_IPP_NR_RD_CFG_4_UNION;
#endif
#define SOC_IPP_CVDR_CVDR_IPP_NR_RD_CFG_4_nrrd_allocated_du_4_START (5)
#define SOC_IPP_CVDR_CVDR_IPP_NR_RD_CFG_4_nrrd_allocated_du_4_END (9)
#define SOC_IPP_CVDR_CVDR_IPP_NR_RD_CFG_4_nr_rd_stop_enable_du_threshold_reached_4_START (16)
#define SOC_IPP_CVDR_CVDR_IPP_NR_RD_CFG_4_nr_rd_stop_enable_du_threshold_reached_4_END (16)
#define SOC_IPP_CVDR_CVDR_IPP_NR_RD_CFG_4_nr_rd_stop_enable_flux_ctrl_4_START (17)
#define SOC_IPP_CVDR_CVDR_IPP_NR_RD_CFG_4_nr_rd_stop_enable_flux_ctrl_4_END (17)
#define SOC_IPP_CVDR_CVDR_IPP_NR_RD_CFG_4_nr_rd_stop_enable_pressure_4_START (18)
#define SOC_IPP_CVDR_CVDR_IPP_NR_RD_CFG_4_nr_rd_stop_enable_pressure_4_END (18)
#define SOC_IPP_CVDR_CVDR_IPP_NR_RD_CFG_4_nr_rd_stop_ok_4_START (24)
#define SOC_IPP_CVDR_CVDR_IPP_NR_RD_CFG_4_nr_rd_stop_ok_4_END (24)
#define SOC_IPP_CVDR_CVDR_IPP_NR_RD_CFG_4_nr_rd_stop_4_START (25)
#define SOC_IPP_CVDR_CVDR_IPP_NR_RD_CFG_4_nr_rd_stop_4_END (25)
#define SOC_IPP_CVDR_CVDR_IPP_NR_RD_CFG_4_nrrd_enable_4_START (31)
#define SOC_IPP_CVDR_CVDR_IPP_NR_RD_CFG_4_nrrd_enable_4_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int nrrd_access_limiter_0_4 : 4;
        unsigned int nrrd_access_limiter_1_4 : 4;
        unsigned int nrrd_access_limiter_2_4 : 4;
        unsigned int nrrd_access_limiter_3_4 : 4;
        unsigned int reserved_0 : 8;
        unsigned int nrrd_access_limiter_reload_4 : 4;
        unsigned int reserved_1 : 4;
    } reg;
} SOC_IPP_CVDR_CVDR_IPP_NR_RD_LIMITER_4_UNION;
#endif
#define SOC_IPP_CVDR_CVDR_IPP_NR_RD_LIMITER_4_nrrd_access_limiter_0_4_START (0)
#define SOC_IPP_CVDR_CVDR_IPP_NR_RD_LIMITER_4_nrrd_access_limiter_0_4_END (3)
#define SOC_IPP_CVDR_CVDR_IPP_NR_RD_LIMITER_4_nrrd_access_limiter_1_4_START (4)
#define SOC_IPP_CVDR_CVDR_IPP_NR_RD_LIMITER_4_nrrd_access_limiter_1_4_END (7)
#define SOC_IPP_CVDR_CVDR_IPP_NR_RD_LIMITER_4_nrrd_access_limiter_2_4_START (8)
#define SOC_IPP_CVDR_CVDR_IPP_NR_RD_LIMITER_4_nrrd_access_limiter_2_4_END (11)
#define SOC_IPP_CVDR_CVDR_IPP_NR_RD_LIMITER_4_nrrd_access_limiter_3_4_START (12)
#define SOC_IPP_CVDR_CVDR_IPP_NR_RD_LIMITER_4_nrrd_access_limiter_3_4_END (15)
#define SOC_IPP_CVDR_CVDR_IPP_NR_RD_LIMITER_4_nrrd_access_limiter_reload_4_START (24)
#define SOC_IPP_CVDR_CVDR_IPP_NR_RD_LIMITER_4_nrrd_access_limiter_reload_4_END (27)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int nr_rd_debug_4 : 32;
    } reg;
} SOC_IPP_CVDR_CVDR_IPP_NR_RD_DEBUG_4_UNION;
#endif
#define SOC_IPP_CVDR_CVDR_IPP_NR_RD_DEBUG_4_nr_rd_debug_4_START (0)
#define SOC_IPP_CVDR_CVDR_IPP_NR_RD_DEBUG_4_nr_rd_debug_4_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int reserved_0 : 5;
        unsigned int nrrd_allocated_du_7 : 5;
        unsigned int reserved_1 : 6;
        unsigned int nr_rd_stop_enable_du_threshold_reached_7 : 1;
        unsigned int nr_rd_stop_enable_flux_ctrl_7 : 1;
        unsigned int nr_rd_stop_enable_pressure_7 : 1;
        unsigned int reserved_2 : 5;
        unsigned int nr_rd_stop_ok_7 : 1;
        unsigned int nr_rd_stop_7 : 1;
        unsigned int reserved_3 : 5;
        unsigned int nrrd_enable_7 : 1;
    } reg;
} SOC_IPP_CVDR_CVDR_IPP_NR_RD_CFG_7_UNION;
#endif
#define SOC_IPP_CVDR_CVDR_IPP_NR_RD_CFG_7_nrrd_allocated_du_7_START (5)
#define SOC_IPP_CVDR_CVDR_IPP_NR_RD_CFG_7_nrrd_allocated_du_7_END (9)
#define SOC_IPP_CVDR_CVDR_IPP_NR_RD_CFG_7_nr_rd_stop_enable_du_threshold_reached_7_START (16)
#define SOC_IPP_CVDR_CVDR_IPP_NR_RD_CFG_7_nr_rd_stop_enable_du_threshold_reached_7_END (16)
#define SOC_IPP_CVDR_CVDR_IPP_NR_RD_CFG_7_nr_rd_stop_enable_flux_ctrl_7_START (17)
#define SOC_IPP_CVDR_CVDR_IPP_NR_RD_CFG_7_nr_rd_stop_enable_flux_ctrl_7_END (17)
#define SOC_IPP_CVDR_CVDR_IPP_NR_RD_CFG_7_nr_rd_stop_enable_pressure_7_START (18)
#define SOC_IPP_CVDR_CVDR_IPP_NR_RD_CFG_7_nr_rd_stop_enable_pressure_7_END (18)
#define SOC_IPP_CVDR_CVDR_IPP_NR_RD_CFG_7_nr_rd_stop_ok_7_START (24)
#define SOC_IPP_CVDR_CVDR_IPP_NR_RD_CFG_7_nr_rd_stop_ok_7_END (24)
#define SOC_IPP_CVDR_CVDR_IPP_NR_RD_CFG_7_nr_rd_stop_7_START (25)
#define SOC_IPP_CVDR_CVDR_IPP_NR_RD_CFG_7_nr_rd_stop_7_END (25)
#define SOC_IPP_CVDR_CVDR_IPP_NR_RD_CFG_7_nrrd_enable_7_START (31)
#define SOC_IPP_CVDR_CVDR_IPP_NR_RD_CFG_7_nrrd_enable_7_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int nrrd_access_limiter_0_7 : 4;
        unsigned int nrrd_access_limiter_1_7 : 4;
        unsigned int nrrd_access_limiter_2_7 : 4;
        unsigned int nrrd_access_limiter_3_7 : 4;
        unsigned int reserved_0 : 8;
        unsigned int nrrd_access_limiter_reload_7 : 4;
        unsigned int reserved_1 : 4;
    } reg;
} SOC_IPP_CVDR_CVDR_IPP_NR_RD_LIMITER_7_UNION;
#endif
#define SOC_IPP_CVDR_CVDR_IPP_NR_RD_LIMITER_7_nrrd_access_limiter_0_7_START (0)
#define SOC_IPP_CVDR_CVDR_IPP_NR_RD_LIMITER_7_nrrd_access_limiter_0_7_END (3)
#define SOC_IPP_CVDR_CVDR_IPP_NR_RD_LIMITER_7_nrrd_access_limiter_1_7_START (4)
#define SOC_IPP_CVDR_CVDR_IPP_NR_RD_LIMITER_7_nrrd_access_limiter_1_7_END (7)
#define SOC_IPP_CVDR_CVDR_IPP_NR_RD_LIMITER_7_nrrd_access_limiter_2_7_START (8)
#define SOC_IPP_CVDR_CVDR_IPP_NR_RD_LIMITER_7_nrrd_access_limiter_2_7_END (11)
#define SOC_IPP_CVDR_CVDR_IPP_NR_RD_LIMITER_7_nrrd_access_limiter_3_7_START (12)
#define SOC_IPP_CVDR_CVDR_IPP_NR_RD_LIMITER_7_nrrd_access_limiter_3_7_END (15)
#define SOC_IPP_CVDR_CVDR_IPP_NR_RD_LIMITER_7_nrrd_access_limiter_reload_7_START (24)
#define SOC_IPP_CVDR_CVDR_IPP_NR_RD_LIMITER_7_nrrd_access_limiter_reload_7_END (27)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int nr_rd_debug_7 : 32;
    } reg;
} SOC_IPP_CVDR_CVDR_IPP_NR_RD_DEBUG_7_UNION;
#endif
#define SOC_IPP_CVDR_CVDR_IPP_NR_RD_DEBUG_7_nr_rd_debug_7_START (0)
#define SOC_IPP_CVDR_CVDR_IPP_NR_RD_DEBUG_7_nr_rd_debug_7_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int reserved_0 : 5;
        unsigned int nrrd_allocated_du_8 : 5;
        unsigned int reserved_1 : 6;
        unsigned int nr_rd_stop_enable_du_threshold_reached_8 : 1;
        unsigned int nr_rd_stop_enable_flux_ctrl_8 : 1;
        unsigned int nr_rd_stop_enable_pressure_8 : 1;
        unsigned int reserved_2 : 5;
        unsigned int nr_rd_stop_ok_8 : 1;
        unsigned int nr_rd_stop_8 : 1;
        unsigned int reserved_3 : 5;
        unsigned int nrrd_enable_8 : 1;
    } reg;
} SOC_IPP_CVDR_CVDR_IPP_NR_RD_CFG_8_UNION;
#endif
#define SOC_IPP_CVDR_CVDR_IPP_NR_RD_CFG_8_nrrd_allocated_du_8_START (5)
#define SOC_IPP_CVDR_CVDR_IPP_NR_RD_CFG_8_nrrd_allocated_du_8_END (9)
#define SOC_IPP_CVDR_CVDR_IPP_NR_RD_CFG_8_nr_rd_stop_enable_du_threshold_reached_8_START (16)
#define SOC_IPP_CVDR_CVDR_IPP_NR_RD_CFG_8_nr_rd_stop_enable_du_threshold_reached_8_END (16)
#define SOC_IPP_CVDR_CVDR_IPP_NR_RD_CFG_8_nr_rd_stop_enable_flux_ctrl_8_START (17)
#define SOC_IPP_CVDR_CVDR_IPP_NR_RD_CFG_8_nr_rd_stop_enable_flux_ctrl_8_END (17)
#define SOC_IPP_CVDR_CVDR_IPP_NR_RD_CFG_8_nr_rd_stop_enable_pressure_8_START (18)
#define SOC_IPP_CVDR_CVDR_IPP_NR_RD_CFG_8_nr_rd_stop_enable_pressure_8_END (18)
#define SOC_IPP_CVDR_CVDR_IPP_NR_RD_CFG_8_nr_rd_stop_ok_8_START (24)
#define SOC_IPP_CVDR_CVDR_IPP_NR_RD_CFG_8_nr_rd_stop_ok_8_END (24)
#define SOC_IPP_CVDR_CVDR_IPP_NR_RD_CFG_8_nr_rd_stop_8_START (25)
#define SOC_IPP_CVDR_CVDR_IPP_NR_RD_CFG_8_nr_rd_stop_8_END (25)
#define SOC_IPP_CVDR_CVDR_IPP_NR_RD_CFG_8_nrrd_enable_8_START (31)
#define SOC_IPP_CVDR_CVDR_IPP_NR_RD_CFG_8_nrrd_enable_8_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int nrrd_access_limiter_0_8 : 4;
        unsigned int nrrd_access_limiter_1_8 : 4;
        unsigned int nrrd_access_limiter_2_8 : 4;
        unsigned int nrrd_access_limiter_3_8 : 4;
        unsigned int reserved_0 : 8;
        unsigned int nrrd_access_limiter_reload_8 : 4;
        unsigned int reserved_1 : 4;
    } reg;
} SOC_IPP_CVDR_CVDR_IPP_NR_RD_LIMITER_8_UNION;
#endif
#define SOC_IPP_CVDR_CVDR_IPP_NR_RD_LIMITER_8_nrrd_access_limiter_0_8_START (0)
#define SOC_IPP_CVDR_CVDR_IPP_NR_RD_LIMITER_8_nrrd_access_limiter_0_8_END (3)
#define SOC_IPP_CVDR_CVDR_IPP_NR_RD_LIMITER_8_nrrd_access_limiter_1_8_START (4)
#define SOC_IPP_CVDR_CVDR_IPP_NR_RD_LIMITER_8_nrrd_access_limiter_1_8_END (7)
#define SOC_IPP_CVDR_CVDR_IPP_NR_RD_LIMITER_8_nrrd_access_limiter_2_8_START (8)
#define SOC_IPP_CVDR_CVDR_IPP_NR_RD_LIMITER_8_nrrd_access_limiter_2_8_END (11)
#define SOC_IPP_CVDR_CVDR_IPP_NR_RD_LIMITER_8_nrrd_access_limiter_3_8_START (12)
#define SOC_IPP_CVDR_CVDR_IPP_NR_RD_LIMITER_8_nrrd_access_limiter_3_8_END (15)
#define SOC_IPP_CVDR_CVDR_IPP_NR_RD_LIMITER_8_nrrd_access_limiter_reload_8_START (24)
#define SOC_IPP_CVDR_CVDR_IPP_NR_RD_LIMITER_8_nrrd_access_limiter_reload_8_END (27)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int nr_rd_debug_8 : 32;
    } reg;
} SOC_IPP_CVDR_CVDR_IPP_NR_RD_DEBUG_8_UNION;
#endif
#define SOC_IPP_CVDR_CVDR_IPP_NR_RD_DEBUG_8_nr_rd_debug_8_START (0)
#define SOC_IPP_CVDR_CVDR_IPP_NR_RD_DEBUG_8_nr_rd_debug_8_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int reserved_0 : 16;
        unsigned int nr_wr_stop_enable_du_threshold_reached_6 : 1;
        unsigned int nr_wr_stop_enable_flux_ctrl_6 : 1;
        unsigned int nr_wr_stop_enable_pressure_6 : 1;
        unsigned int reserved_1 : 5;
        unsigned int nr_wr_stop_ok_6 : 1;
        unsigned int nr_wr_stop_6 : 1;
        unsigned int reserved_2 : 5;
        unsigned int nrwr_enable_6 : 1;
    } reg;
} SOC_IPP_CVDR_CVDR_IPP_NR_WR_CFG_6_UNION;
#endif
#define SOC_IPP_CVDR_CVDR_IPP_NR_WR_CFG_6_nr_wr_stop_enable_du_threshold_reached_6_START (16)
#define SOC_IPP_CVDR_CVDR_IPP_NR_WR_CFG_6_nr_wr_stop_enable_du_threshold_reached_6_END (16)
#define SOC_IPP_CVDR_CVDR_IPP_NR_WR_CFG_6_nr_wr_stop_enable_flux_ctrl_6_START (17)
#define SOC_IPP_CVDR_CVDR_IPP_NR_WR_CFG_6_nr_wr_stop_enable_flux_ctrl_6_END (17)
#define SOC_IPP_CVDR_CVDR_IPP_NR_WR_CFG_6_nr_wr_stop_enable_pressure_6_START (18)
#define SOC_IPP_CVDR_CVDR_IPP_NR_WR_CFG_6_nr_wr_stop_enable_pressure_6_END (18)
#define SOC_IPP_CVDR_CVDR_IPP_NR_WR_CFG_6_nr_wr_stop_ok_6_START (24)
#define SOC_IPP_CVDR_CVDR_IPP_NR_WR_CFG_6_nr_wr_stop_ok_6_END (24)
#define SOC_IPP_CVDR_CVDR_IPP_NR_WR_CFG_6_nr_wr_stop_6_START (25)
#define SOC_IPP_CVDR_CVDR_IPP_NR_WR_CFG_6_nr_wr_stop_6_END (25)
#define SOC_IPP_CVDR_CVDR_IPP_NR_WR_CFG_6_nrwr_enable_6_START (31)
#define SOC_IPP_CVDR_CVDR_IPP_NR_WR_CFG_6_nrwr_enable_6_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int nrwr_access_limiter_0_6 : 4;
        unsigned int nrwr_access_limiter_1_6 : 4;
        unsigned int nrwr_access_limiter_2_6 : 4;
        unsigned int nrwr_access_limiter_3_6 : 4;
        unsigned int reserved_0 : 8;
        unsigned int nrwr_access_limiter_reload_6 : 4;
        unsigned int reserved_1 : 4;
    } reg;
} SOC_IPP_CVDR_CVDR_IPP_NR_WR_LIMITER_6_UNION;
#endif
#define SOC_IPP_CVDR_CVDR_IPP_NR_WR_LIMITER_6_nrwr_access_limiter_0_6_START (0)
#define SOC_IPP_CVDR_CVDR_IPP_NR_WR_LIMITER_6_nrwr_access_limiter_0_6_END (3)
#define SOC_IPP_CVDR_CVDR_IPP_NR_WR_LIMITER_6_nrwr_access_limiter_1_6_START (4)
#define SOC_IPP_CVDR_CVDR_IPP_NR_WR_LIMITER_6_nrwr_access_limiter_1_6_END (7)
#define SOC_IPP_CVDR_CVDR_IPP_NR_WR_LIMITER_6_nrwr_access_limiter_2_6_START (8)
#define SOC_IPP_CVDR_CVDR_IPP_NR_WR_LIMITER_6_nrwr_access_limiter_2_6_END (11)
#define SOC_IPP_CVDR_CVDR_IPP_NR_WR_LIMITER_6_nrwr_access_limiter_3_6_START (12)
#define SOC_IPP_CVDR_CVDR_IPP_NR_WR_LIMITER_6_nrwr_access_limiter_3_6_END (15)
#define SOC_IPP_CVDR_CVDR_IPP_NR_WR_LIMITER_6_nrwr_access_limiter_reload_6_START (24)
#define SOC_IPP_CVDR_CVDR_IPP_NR_WR_LIMITER_6_nrwr_access_limiter_reload_6_END (27)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int nr_wr_debug_6 : 32;
    } reg;
} SOC_IPP_CVDR_CVDR_IPP_NR_WR_DEBUG_6_UNION;
#endif
#define SOC_IPP_CVDR_CVDR_IPP_NR_WR_DEBUG_6_nr_wr_debug_6_START (0)
#define SOC_IPP_CVDR_CVDR_IPP_NR_WR_DEBUG_6_nr_wr_debug_6_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int reserved_0 : 16;
        unsigned int nr_wr_stop_enable_du_threshold_reached_7 : 1;
        unsigned int nr_wr_stop_enable_flux_ctrl_7 : 1;
        unsigned int nr_wr_stop_enable_pressure_7 : 1;
        unsigned int reserved_1 : 5;
        unsigned int nr_wr_stop_ok_7 : 1;
        unsigned int nr_wr_stop_7 : 1;
        unsigned int reserved_2 : 5;
        unsigned int nrwr_enable_7 : 1;
    } reg;
} SOC_IPP_CVDR_CVDR_IPP_NR_WR_CFG_7_UNION;
#endif
#define SOC_IPP_CVDR_CVDR_IPP_NR_WR_CFG_7_nr_wr_stop_enable_du_threshold_reached_7_START (16)
#define SOC_IPP_CVDR_CVDR_IPP_NR_WR_CFG_7_nr_wr_stop_enable_du_threshold_reached_7_END (16)
#define SOC_IPP_CVDR_CVDR_IPP_NR_WR_CFG_7_nr_wr_stop_enable_flux_ctrl_7_START (17)
#define SOC_IPP_CVDR_CVDR_IPP_NR_WR_CFG_7_nr_wr_stop_enable_flux_ctrl_7_END (17)
#define SOC_IPP_CVDR_CVDR_IPP_NR_WR_CFG_7_nr_wr_stop_enable_pressure_7_START (18)
#define SOC_IPP_CVDR_CVDR_IPP_NR_WR_CFG_7_nr_wr_stop_enable_pressure_7_END (18)
#define SOC_IPP_CVDR_CVDR_IPP_NR_WR_CFG_7_nr_wr_stop_ok_7_START (24)
#define SOC_IPP_CVDR_CVDR_IPP_NR_WR_CFG_7_nr_wr_stop_ok_7_END (24)
#define SOC_IPP_CVDR_CVDR_IPP_NR_WR_CFG_7_nr_wr_stop_7_START (25)
#define SOC_IPP_CVDR_CVDR_IPP_NR_WR_CFG_7_nr_wr_stop_7_END (25)
#define SOC_IPP_CVDR_CVDR_IPP_NR_WR_CFG_7_nrwr_enable_7_START (31)
#define SOC_IPP_CVDR_CVDR_IPP_NR_WR_CFG_7_nrwr_enable_7_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int nrwr_access_limiter_0_7 : 4;
        unsigned int nrwr_access_limiter_1_7 : 4;
        unsigned int nrwr_access_limiter_2_7 : 4;
        unsigned int nrwr_access_limiter_3_7 : 4;
        unsigned int reserved_0 : 8;
        unsigned int nrwr_access_limiter_reload_7 : 4;
        unsigned int reserved_1 : 4;
    } reg;
} SOC_IPP_CVDR_CVDR_IPP_NR_WR_LIMITER_7_UNION;
#endif
#define SOC_IPP_CVDR_CVDR_IPP_NR_WR_LIMITER_7_nrwr_access_limiter_0_7_START (0)
#define SOC_IPP_CVDR_CVDR_IPP_NR_WR_LIMITER_7_nrwr_access_limiter_0_7_END (3)
#define SOC_IPP_CVDR_CVDR_IPP_NR_WR_LIMITER_7_nrwr_access_limiter_1_7_START (4)
#define SOC_IPP_CVDR_CVDR_IPP_NR_WR_LIMITER_7_nrwr_access_limiter_1_7_END (7)
#define SOC_IPP_CVDR_CVDR_IPP_NR_WR_LIMITER_7_nrwr_access_limiter_2_7_START (8)
#define SOC_IPP_CVDR_CVDR_IPP_NR_WR_LIMITER_7_nrwr_access_limiter_2_7_END (11)
#define SOC_IPP_CVDR_CVDR_IPP_NR_WR_LIMITER_7_nrwr_access_limiter_3_7_START (12)
#define SOC_IPP_CVDR_CVDR_IPP_NR_WR_LIMITER_7_nrwr_access_limiter_3_7_END (15)
#define SOC_IPP_CVDR_CVDR_IPP_NR_WR_LIMITER_7_nrwr_access_limiter_reload_7_START (24)
#define SOC_IPP_CVDR_CVDR_IPP_NR_WR_LIMITER_7_nrwr_access_limiter_reload_7_END (27)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int nr_wr_debug_7 : 32;
    } reg;
} SOC_IPP_CVDR_CVDR_IPP_NR_WR_DEBUG_7_UNION;
#endif
#define SOC_IPP_CVDR_CVDR_IPP_NR_WR_DEBUG_7_nr_wr_debug_7_START (0)
#define SOC_IPP_CVDR_CVDR_IPP_NR_WR_DEBUG_7_nr_wr_debug_7_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int reserved_0 : 16;
        unsigned int nr_wr_stop_enable_du_threshold_reached_8 : 1;
        unsigned int nr_wr_stop_enable_flux_ctrl_8 : 1;
        unsigned int nr_wr_stop_enable_pressure_8 : 1;
        unsigned int reserved_1 : 5;
        unsigned int nr_wr_stop_ok_8 : 1;
        unsigned int nr_wr_stop_8 : 1;
        unsigned int reserved_2 : 5;
        unsigned int nrwr_enable_8 : 1;
    } reg;
} SOC_IPP_CVDR_CVDR_IPP_NR_WR_CFG_8_UNION;
#endif
#define SOC_IPP_CVDR_CVDR_IPP_NR_WR_CFG_8_nr_wr_stop_enable_du_threshold_reached_8_START (16)
#define SOC_IPP_CVDR_CVDR_IPP_NR_WR_CFG_8_nr_wr_stop_enable_du_threshold_reached_8_END (16)
#define SOC_IPP_CVDR_CVDR_IPP_NR_WR_CFG_8_nr_wr_stop_enable_flux_ctrl_8_START (17)
#define SOC_IPP_CVDR_CVDR_IPP_NR_WR_CFG_8_nr_wr_stop_enable_flux_ctrl_8_END (17)
#define SOC_IPP_CVDR_CVDR_IPP_NR_WR_CFG_8_nr_wr_stop_enable_pressure_8_START (18)
#define SOC_IPP_CVDR_CVDR_IPP_NR_WR_CFG_8_nr_wr_stop_enable_pressure_8_END (18)
#define SOC_IPP_CVDR_CVDR_IPP_NR_WR_CFG_8_nr_wr_stop_ok_8_START (24)
#define SOC_IPP_CVDR_CVDR_IPP_NR_WR_CFG_8_nr_wr_stop_ok_8_END (24)
#define SOC_IPP_CVDR_CVDR_IPP_NR_WR_CFG_8_nr_wr_stop_8_START (25)
#define SOC_IPP_CVDR_CVDR_IPP_NR_WR_CFG_8_nr_wr_stop_8_END (25)
#define SOC_IPP_CVDR_CVDR_IPP_NR_WR_CFG_8_nrwr_enable_8_START (31)
#define SOC_IPP_CVDR_CVDR_IPP_NR_WR_CFG_8_nrwr_enable_8_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int nrwr_access_limiter_0_8 : 4;
        unsigned int nrwr_access_limiter_1_8 : 4;
        unsigned int nrwr_access_limiter_2_8 : 4;
        unsigned int nrwr_access_limiter_3_8 : 4;
        unsigned int reserved_0 : 8;
        unsigned int nrwr_access_limiter_reload_8 : 4;
        unsigned int reserved_1 : 4;
    } reg;
} SOC_IPP_CVDR_CVDR_IPP_NR_WR_LIMITER_8_UNION;
#endif
#define SOC_IPP_CVDR_CVDR_IPP_NR_WR_LIMITER_8_nrwr_access_limiter_0_8_START (0)
#define SOC_IPP_CVDR_CVDR_IPP_NR_WR_LIMITER_8_nrwr_access_limiter_0_8_END (3)
#define SOC_IPP_CVDR_CVDR_IPP_NR_WR_LIMITER_8_nrwr_access_limiter_1_8_START (4)
#define SOC_IPP_CVDR_CVDR_IPP_NR_WR_LIMITER_8_nrwr_access_limiter_1_8_END (7)
#define SOC_IPP_CVDR_CVDR_IPP_NR_WR_LIMITER_8_nrwr_access_limiter_2_8_START (8)
#define SOC_IPP_CVDR_CVDR_IPP_NR_WR_LIMITER_8_nrwr_access_limiter_2_8_END (11)
#define SOC_IPP_CVDR_CVDR_IPP_NR_WR_LIMITER_8_nrwr_access_limiter_3_8_START (12)
#define SOC_IPP_CVDR_CVDR_IPP_NR_WR_LIMITER_8_nrwr_access_limiter_3_8_END (15)
#define SOC_IPP_CVDR_CVDR_IPP_NR_WR_LIMITER_8_nrwr_access_limiter_reload_8_START (24)
#define SOC_IPP_CVDR_CVDR_IPP_NR_WR_LIMITER_8_nrwr_access_limiter_reload_8_END (27)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int nr_wr_debug_8 : 32;
    } reg;
} SOC_IPP_CVDR_CVDR_IPP_NR_WR_DEBUG_8_UNION;
#endif
#define SOC_IPP_CVDR_CVDR_IPP_NR_WR_DEBUG_8_nr_wr_debug_8_START (0)
#define SOC_IPP_CVDR_CVDR_IPP_NR_WR_DEBUG_8_nr_wr_debug_8_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int vprd_pixel_format_0 : 5;
        unsigned int vprd_pixel_expansion_0 : 1;
        unsigned int vprd_allocated_du_0 : 5;
        unsigned int reserved : 2;
        unsigned int vprd_last_page_0 : 19;
    } reg;
} SOC_IPP_CVDR_CVDR_IPP_VP_RD_CFG_0_UNION;
#endif
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_CFG_0_vprd_pixel_format_0_START (0)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_CFG_0_vprd_pixel_format_0_END (4)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_CFG_0_vprd_pixel_expansion_0_START (5)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_CFG_0_vprd_pixel_expansion_0_END (5)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_CFG_0_vprd_allocated_du_0_START (6)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_CFG_0_vprd_allocated_du_0_END (10)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_CFG_0_vprd_last_page_0_START (13)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_CFG_0_vprd_last_page_0_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int vprd_line_size_0 : 15;
        unsigned int reserved_0 : 1;
        unsigned int vprd_horizontal_blanking_0 : 8;
        unsigned int reserved_1 : 8;
    } reg;
} SOC_IPP_CVDR_CVDR_IPP_VP_RD_LWG_0_UNION;
#endif
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_LWG_0_vprd_line_size_0_START (0)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_LWG_0_vprd_line_size_0_END (14)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_LWG_0_vprd_horizontal_blanking_0_START (16)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_LWG_0_vprd_horizontal_blanking_0_END (23)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int vprd_frame_size_0 : 15;
        unsigned int reserved_0 : 1;
        unsigned int vprd_vertical_blanking_0 : 8;
        unsigned int reserved_1 : 8;
    } reg;
} SOC_IPP_CVDR_CVDR_IPP_VP_RD_FHG_0_UNION;
#endif
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_FHG_0_vprd_frame_size_0_START (0)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_FHG_0_vprd_frame_size_0_END (14)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_FHG_0_vprd_vertical_blanking_0_START (16)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_FHG_0_vprd_vertical_blanking_0_END (23)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int reserved : 2;
        unsigned int vprd_axi_frame_start_0 : 30;
    } reg;
} SOC_IPP_CVDR_CVDR_IPP_VP_RD_AXI_FS_0_UNION;
#endif
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_AXI_FS_0_vprd_axi_frame_start_0_START (2)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_AXI_FS_0_vprd_axi_frame_start_0_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int vprd_line_stride_0 : 14;
        unsigned int reserved : 4;
        unsigned int vprd_line_wrap_0 : 14;
    } reg;
} SOC_IPP_CVDR_CVDR_IPP_VP_RD_AXI_LINE_0_UNION;
#endif
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_AXI_LINE_0_vprd_line_stride_0_START (0)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_AXI_LINE_0_vprd_line_stride_0_END (13)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_AXI_LINE_0_vprd_line_wrap_0_START (18)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_AXI_LINE_0_vprd_line_wrap_0_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int reserved_0 : 16;
        unsigned int vp_rd_stop_enable_du_threshold_reached_0 : 1;
        unsigned int vp_rd_stop_enable_flux_ctrl_0 : 1;
        unsigned int vp_rd_stop_enable_pressure_0 : 1;
        unsigned int reserved_1 : 5;
        unsigned int vp_rd_stop_ok_0 : 1;
        unsigned int vp_rd_stop_0 : 1;
        unsigned int reserved_2 : 5;
        unsigned int vprd_prefetch_bypass_0 : 1;
    } reg;
} SOC_IPP_CVDR_CVDR_IPP_VP_RD_IF_CFG_0_UNION;
#endif
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_IF_CFG_0_vp_rd_stop_enable_du_threshold_reached_0_START (16)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_IF_CFG_0_vp_rd_stop_enable_du_threshold_reached_0_END (16)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_IF_CFG_0_vp_rd_stop_enable_flux_ctrl_0_START (17)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_IF_CFG_0_vp_rd_stop_enable_flux_ctrl_0_END (17)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_IF_CFG_0_vp_rd_stop_enable_pressure_0_START (18)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_IF_CFG_0_vp_rd_stop_enable_pressure_0_END (18)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_IF_CFG_0_vp_rd_stop_ok_0_START (24)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_IF_CFG_0_vp_rd_stop_ok_0_END (24)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_IF_CFG_0_vp_rd_stop_0_START (25)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_IF_CFG_0_vp_rd_stop_0_END (25)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_IF_CFG_0_vprd_prefetch_bypass_0_START (31)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_IF_CFG_0_vprd_prefetch_bypass_0_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int vprd_access_limiter_0_0 : 4;
        unsigned int vprd_access_limiter_1_0 : 4;
        unsigned int vprd_access_limiter_2_0 : 4;
        unsigned int vprd_access_limiter_3_0 : 4;
        unsigned int reserved_0 : 8;
        unsigned int vprd_access_limiter_reload_0 : 4;
        unsigned int reserved_1 : 4;
    } reg;
} SOC_IPP_CVDR_CVDR_IPP_VP_RD_LIMITER_0_UNION;
#endif
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_LIMITER_0_vprd_access_limiter_0_0_START (0)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_LIMITER_0_vprd_access_limiter_0_0_END (3)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_LIMITER_0_vprd_access_limiter_1_0_START (4)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_LIMITER_0_vprd_access_limiter_1_0_END (7)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_LIMITER_0_vprd_access_limiter_2_0_START (8)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_LIMITER_0_vprd_access_limiter_2_0_END (11)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_LIMITER_0_vprd_access_limiter_3_0_START (12)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_LIMITER_0_vprd_access_limiter_3_0_END (15)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_LIMITER_0_vprd_access_limiter_reload_0_START (24)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_LIMITER_0_vprd_access_limiter_reload_0_END (27)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int vp_rd_debug_0 : 32;
    } reg;
} SOC_IPP_CVDR_CVDR_IPP_VP_RD_DEBUG_0_UNION;
#endif
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_DEBUG_0_vp_rd_debug_0_START (0)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_DEBUG_0_vp_rd_debug_0_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int vprd_pixel_format_6 : 5;
        unsigned int vprd_pixel_expansion_6 : 1;
        unsigned int vprd_allocated_du_6 : 5;
        unsigned int reserved : 2;
        unsigned int vprd_last_page_6 : 19;
    } reg;
} SOC_IPP_CVDR_CVDR_IPP_VP_RD_CFG_6_UNION;
#endif
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_CFG_6_vprd_pixel_format_6_START (0)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_CFG_6_vprd_pixel_format_6_END (4)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_CFG_6_vprd_pixel_expansion_6_START (5)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_CFG_6_vprd_pixel_expansion_6_END (5)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_CFG_6_vprd_allocated_du_6_START (6)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_CFG_6_vprd_allocated_du_6_END (10)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_CFG_6_vprd_last_page_6_START (13)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_CFG_6_vprd_last_page_6_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int vprd_line_size_6 : 15;
        unsigned int reserved_0 : 1;
        unsigned int vprd_horizontal_blanking_6 : 8;
        unsigned int reserved_1 : 8;
    } reg;
} SOC_IPP_CVDR_CVDR_IPP_VP_RD_LWG_6_UNION;
#endif
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_LWG_6_vprd_line_size_6_START (0)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_LWG_6_vprd_line_size_6_END (14)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_LWG_6_vprd_horizontal_blanking_6_START (16)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_LWG_6_vprd_horizontal_blanking_6_END (23)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int vprd_frame_size_6 : 15;
        unsigned int reserved_0 : 1;
        unsigned int vprd_vertical_blanking_6 : 8;
        unsigned int reserved_1 : 8;
    } reg;
} SOC_IPP_CVDR_CVDR_IPP_VP_RD_FHG_6_UNION;
#endif
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_FHG_6_vprd_frame_size_6_START (0)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_FHG_6_vprd_frame_size_6_END (14)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_FHG_6_vprd_vertical_blanking_6_START (16)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_FHG_6_vprd_vertical_blanking_6_END (23)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int reserved : 2;
        unsigned int vprd_axi_frame_start_6 : 30;
    } reg;
} SOC_IPP_CVDR_CVDR_IPP_VP_RD_AXI_FS_6_UNION;
#endif
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_AXI_FS_6_vprd_axi_frame_start_6_START (2)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_AXI_FS_6_vprd_axi_frame_start_6_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int vprd_line_stride_6 : 14;
        unsigned int reserved : 4;
        unsigned int vprd_line_wrap_6 : 14;
    } reg;
} SOC_IPP_CVDR_CVDR_IPP_VP_RD_AXI_LINE_6_UNION;
#endif
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_AXI_LINE_6_vprd_line_stride_6_START (0)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_AXI_LINE_6_vprd_line_stride_6_END (13)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_AXI_LINE_6_vprd_line_wrap_6_START (18)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_AXI_LINE_6_vprd_line_wrap_6_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int reserved_0 : 16;
        unsigned int vp_rd_stop_enable_du_threshold_reached_6 : 1;
        unsigned int vp_rd_stop_enable_flux_ctrl_6 : 1;
        unsigned int vp_rd_stop_enable_pressure_6 : 1;
        unsigned int reserved_1 : 5;
        unsigned int vp_rd_stop_ok_6 : 1;
        unsigned int vp_rd_stop_6 : 1;
        unsigned int reserved_2 : 5;
        unsigned int vprd_prefetch_bypass_6 : 1;
    } reg;
} SOC_IPP_CVDR_CVDR_IPP_VP_RD_IF_CFG_6_UNION;
#endif
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_IF_CFG_6_vp_rd_stop_enable_du_threshold_reached_6_START (16)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_IF_CFG_6_vp_rd_stop_enable_du_threshold_reached_6_END (16)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_IF_CFG_6_vp_rd_stop_enable_flux_ctrl_6_START (17)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_IF_CFG_6_vp_rd_stop_enable_flux_ctrl_6_END (17)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_IF_CFG_6_vp_rd_stop_enable_pressure_6_START (18)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_IF_CFG_6_vp_rd_stop_enable_pressure_6_END (18)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_IF_CFG_6_vp_rd_stop_ok_6_START (24)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_IF_CFG_6_vp_rd_stop_ok_6_END (24)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_IF_CFG_6_vp_rd_stop_6_START (25)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_IF_CFG_6_vp_rd_stop_6_END (25)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_IF_CFG_6_vprd_prefetch_bypass_6_START (31)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_IF_CFG_6_vprd_prefetch_bypass_6_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int vprd_access_limiter_0_6 : 4;
        unsigned int vprd_access_limiter_1_6 : 4;
        unsigned int vprd_access_limiter_2_6 : 4;
        unsigned int vprd_access_limiter_3_6 : 4;
        unsigned int reserved_0 : 8;
        unsigned int vprd_access_limiter_reload_6 : 4;
        unsigned int reserved_1 : 4;
    } reg;
} SOC_IPP_CVDR_CVDR_IPP_VP_RD_LIMITER_6_UNION;
#endif
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_LIMITER_6_vprd_access_limiter_0_6_START (0)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_LIMITER_6_vprd_access_limiter_0_6_END (3)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_LIMITER_6_vprd_access_limiter_1_6_START (4)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_LIMITER_6_vprd_access_limiter_1_6_END (7)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_LIMITER_6_vprd_access_limiter_2_6_START (8)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_LIMITER_6_vprd_access_limiter_2_6_END (11)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_LIMITER_6_vprd_access_limiter_3_6_START (12)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_LIMITER_6_vprd_access_limiter_3_6_END (15)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_LIMITER_6_vprd_access_limiter_reload_6_START (24)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_LIMITER_6_vprd_access_limiter_reload_6_END (27)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int vp_rd_debug_6 : 32;
    } reg;
} SOC_IPP_CVDR_CVDR_IPP_VP_RD_DEBUG_6_UNION;
#endif
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_DEBUG_6_vp_rd_debug_6_START (0)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_DEBUG_6_vp_rd_debug_6_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int vprd_pixel_format_7 : 5;
        unsigned int vprd_pixel_expansion_7 : 1;
        unsigned int vprd_allocated_du_7 : 5;
        unsigned int reserved : 2;
        unsigned int vprd_last_page_7 : 19;
    } reg;
} SOC_IPP_CVDR_CVDR_IPP_VP_RD_CFG_7_UNION;
#endif
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_CFG_7_vprd_pixel_format_7_START (0)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_CFG_7_vprd_pixel_format_7_END (4)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_CFG_7_vprd_pixel_expansion_7_START (5)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_CFG_7_vprd_pixel_expansion_7_END (5)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_CFG_7_vprd_allocated_du_7_START (6)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_CFG_7_vprd_allocated_du_7_END (10)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_CFG_7_vprd_last_page_7_START (13)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_CFG_7_vprd_last_page_7_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int vprd_line_size_7 : 15;
        unsigned int reserved_0 : 1;
        unsigned int vprd_horizontal_blanking_7 : 8;
        unsigned int reserved_1 : 8;
    } reg;
} SOC_IPP_CVDR_CVDR_IPP_VP_RD_LWG_7_UNION;
#endif
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_LWG_7_vprd_line_size_7_START (0)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_LWG_7_vprd_line_size_7_END (14)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_LWG_7_vprd_horizontal_blanking_7_START (16)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_LWG_7_vprd_horizontal_blanking_7_END (23)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int vprd_frame_size_7 : 15;
        unsigned int reserved_0 : 1;
        unsigned int vprd_vertical_blanking_7 : 8;
        unsigned int reserved_1 : 8;
    } reg;
} SOC_IPP_CVDR_CVDR_IPP_VP_RD_FHG_7_UNION;
#endif
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_FHG_7_vprd_frame_size_7_START (0)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_FHG_7_vprd_frame_size_7_END (14)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_FHG_7_vprd_vertical_blanking_7_START (16)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_FHG_7_vprd_vertical_blanking_7_END (23)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int reserved : 2;
        unsigned int vprd_axi_frame_start_7 : 30;
    } reg;
} SOC_IPP_CVDR_CVDR_IPP_VP_RD_AXI_FS_7_UNION;
#endif
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_AXI_FS_7_vprd_axi_frame_start_7_START (2)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_AXI_FS_7_vprd_axi_frame_start_7_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int vprd_line_stride_7 : 14;
        unsigned int reserved : 4;
        unsigned int vprd_line_wrap_7 : 14;
    } reg;
} SOC_IPP_CVDR_CVDR_IPP_VP_RD_AXI_LINE_7_UNION;
#endif
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_AXI_LINE_7_vprd_line_stride_7_START (0)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_AXI_LINE_7_vprd_line_stride_7_END (13)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_AXI_LINE_7_vprd_line_wrap_7_START (18)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_AXI_LINE_7_vprd_line_wrap_7_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int reserved_0 : 16;
        unsigned int vp_rd_stop_enable_du_threshold_reached_7 : 1;
        unsigned int vp_rd_stop_enable_flux_ctrl_7 : 1;
        unsigned int vp_rd_stop_enable_pressure_7 : 1;
        unsigned int reserved_1 : 5;
        unsigned int vp_rd_stop_ok_7 : 1;
        unsigned int vp_rd_stop_7 : 1;
        unsigned int reserved_2 : 5;
        unsigned int vprd_prefetch_bypass_7 : 1;
    } reg;
} SOC_IPP_CVDR_CVDR_IPP_VP_RD_IF_CFG_7_UNION;
#endif
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_IF_CFG_7_vp_rd_stop_enable_du_threshold_reached_7_START (16)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_IF_CFG_7_vp_rd_stop_enable_du_threshold_reached_7_END (16)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_IF_CFG_7_vp_rd_stop_enable_flux_ctrl_7_START (17)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_IF_CFG_7_vp_rd_stop_enable_flux_ctrl_7_END (17)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_IF_CFG_7_vp_rd_stop_enable_pressure_7_START (18)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_IF_CFG_7_vp_rd_stop_enable_pressure_7_END (18)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_IF_CFG_7_vp_rd_stop_ok_7_START (24)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_IF_CFG_7_vp_rd_stop_ok_7_END (24)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_IF_CFG_7_vp_rd_stop_7_START (25)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_IF_CFG_7_vp_rd_stop_7_END (25)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_IF_CFG_7_vprd_prefetch_bypass_7_START (31)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_IF_CFG_7_vprd_prefetch_bypass_7_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int vprd_access_limiter_0_7 : 4;
        unsigned int vprd_access_limiter_1_7 : 4;
        unsigned int vprd_access_limiter_2_7 : 4;
        unsigned int vprd_access_limiter_3_7 : 4;
        unsigned int reserved_0 : 8;
        unsigned int vprd_access_limiter_reload_7 : 4;
        unsigned int reserved_1 : 4;
    } reg;
} SOC_IPP_CVDR_CVDR_IPP_VP_RD_LIMITER_7_UNION;
#endif
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_LIMITER_7_vprd_access_limiter_0_7_START (0)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_LIMITER_7_vprd_access_limiter_0_7_END (3)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_LIMITER_7_vprd_access_limiter_1_7_START (4)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_LIMITER_7_vprd_access_limiter_1_7_END (7)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_LIMITER_7_vprd_access_limiter_2_7_START (8)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_LIMITER_7_vprd_access_limiter_2_7_END (11)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_LIMITER_7_vprd_access_limiter_3_7_START (12)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_LIMITER_7_vprd_access_limiter_3_7_END (15)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_LIMITER_7_vprd_access_limiter_reload_7_START (24)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_LIMITER_7_vprd_access_limiter_reload_7_END (27)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int vp_rd_debug_7 : 32;
    } reg;
} SOC_IPP_CVDR_CVDR_IPP_VP_RD_DEBUG_7_UNION;
#endif
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_DEBUG_7_vp_rd_debug_7_START (0)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_DEBUG_7_vp_rd_debug_7_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int vprd_pixel_format_9 : 5;
        unsigned int vprd_pixel_expansion_9 : 1;
        unsigned int vprd_allocated_du_9 : 5;
        unsigned int reserved : 2;
        unsigned int vprd_last_page_9 : 19;
    } reg;
} SOC_IPP_CVDR_CVDR_IPP_VP_RD_CFG_9_UNION;
#endif
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_CFG_9_vprd_pixel_format_9_START (0)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_CFG_9_vprd_pixel_format_9_END (4)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_CFG_9_vprd_pixel_expansion_9_START (5)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_CFG_9_vprd_pixel_expansion_9_END (5)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_CFG_9_vprd_allocated_du_9_START (6)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_CFG_9_vprd_allocated_du_9_END (10)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_CFG_9_vprd_last_page_9_START (13)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_CFG_9_vprd_last_page_9_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int vprd_line_size_9 : 15;
        unsigned int reserved_0 : 1;
        unsigned int vprd_horizontal_blanking_9 : 8;
        unsigned int reserved_1 : 8;
    } reg;
} SOC_IPP_CVDR_CVDR_IPP_VP_RD_LWG_9_UNION;
#endif
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_LWG_9_vprd_line_size_9_START (0)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_LWG_9_vprd_line_size_9_END (14)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_LWG_9_vprd_horizontal_blanking_9_START (16)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_LWG_9_vprd_horizontal_blanking_9_END (23)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int vprd_frame_size_9 : 15;
        unsigned int reserved_0 : 1;
        unsigned int vprd_vertical_blanking_9 : 8;
        unsigned int reserved_1 : 8;
    } reg;
} SOC_IPP_CVDR_CVDR_IPP_VP_RD_FHG_9_UNION;
#endif
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_FHG_9_vprd_frame_size_9_START (0)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_FHG_9_vprd_frame_size_9_END (14)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_FHG_9_vprd_vertical_blanking_9_START (16)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_FHG_9_vprd_vertical_blanking_9_END (23)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int reserved : 2;
        unsigned int vprd_axi_frame_start_9 : 30;
    } reg;
} SOC_IPP_CVDR_CVDR_IPP_VP_RD_AXI_FS_9_UNION;
#endif
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_AXI_FS_9_vprd_axi_frame_start_9_START (2)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_AXI_FS_9_vprd_axi_frame_start_9_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int vprd_line_stride_9 : 14;
        unsigned int reserved : 4;
        unsigned int vprd_line_wrap_9 : 14;
    } reg;
} SOC_IPP_CVDR_CVDR_IPP_VP_RD_AXI_LINE_9_UNION;
#endif
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_AXI_LINE_9_vprd_line_stride_9_START (0)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_AXI_LINE_9_vprd_line_stride_9_END (13)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_AXI_LINE_9_vprd_line_wrap_9_START (18)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_AXI_LINE_9_vprd_line_wrap_9_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int reserved_0 : 16;
        unsigned int vp_rd_stop_enable_du_threshold_reached_9 : 1;
        unsigned int vp_rd_stop_enable_flux_ctrl_9 : 1;
        unsigned int vp_rd_stop_enable_pressure_9 : 1;
        unsigned int reserved_1 : 5;
        unsigned int vp_rd_stop_ok_9 : 1;
        unsigned int vp_rd_stop_9 : 1;
        unsigned int reserved_2 : 5;
        unsigned int vprd_prefetch_bypass_9 : 1;
    } reg;
} SOC_IPP_CVDR_CVDR_IPP_VP_RD_IF_CFG_9_UNION;
#endif
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_IF_CFG_9_vp_rd_stop_enable_du_threshold_reached_9_START (16)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_IF_CFG_9_vp_rd_stop_enable_du_threshold_reached_9_END (16)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_IF_CFG_9_vp_rd_stop_enable_flux_ctrl_9_START (17)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_IF_CFG_9_vp_rd_stop_enable_flux_ctrl_9_END (17)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_IF_CFG_9_vp_rd_stop_enable_pressure_9_START (18)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_IF_CFG_9_vp_rd_stop_enable_pressure_9_END (18)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_IF_CFG_9_vp_rd_stop_ok_9_START (24)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_IF_CFG_9_vp_rd_stop_ok_9_END (24)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_IF_CFG_9_vp_rd_stop_9_START (25)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_IF_CFG_9_vp_rd_stop_9_END (25)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_IF_CFG_9_vprd_prefetch_bypass_9_START (31)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_IF_CFG_9_vprd_prefetch_bypass_9_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int vprd_access_limiter_0_9 : 4;
        unsigned int vprd_access_limiter_1_9 : 4;
        unsigned int vprd_access_limiter_2_9 : 4;
        unsigned int vprd_access_limiter_3_9 : 4;
        unsigned int reserved_0 : 8;
        unsigned int vprd_access_limiter_reload_9 : 4;
        unsigned int reserved_1 : 4;
    } reg;
} SOC_IPP_CVDR_CVDR_IPP_VP_RD_LIMITER_9_UNION;
#endif
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_LIMITER_9_vprd_access_limiter_0_9_START (0)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_LIMITER_9_vprd_access_limiter_0_9_END (3)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_LIMITER_9_vprd_access_limiter_1_9_START (4)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_LIMITER_9_vprd_access_limiter_1_9_END (7)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_LIMITER_9_vprd_access_limiter_2_9_START (8)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_LIMITER_9_vprd_access_limiter_2_9_END (11)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_LIMITER_9_vprd_access_limiter_3_9_START (12)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_LIMITER_9_vprd_access_limiter_3_9_END (15)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_LIMITER_9_vprd_access_limiter_reload_9_START (24)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_LIMITER_9_vprd_access_limiter_reload_9_END (27)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int vp_rd_debug_9 : 32;
    } reg;
} SOC_IPP_CVDR_CVDR_IPP_VP_RD_DEBUG_9_UNION;
#endif
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_DEBUG_9_vp_rd_debug_9_START (0)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_DEBUG_9_vp_rd_debug_9_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int vprd_pixel_format_10 : 5;
        unsigned int vprd_pixel_expansion_10 : 1;
        unsigned int vprd_allocated_du_10 : 5;
        unsigned int reserved : 2;
        unsigned int vprd_last_page_10 : 19;
    } reg;
} SOC_IPP_CVDR_CVDR_IPP_VP_RD_CFG_10_UNION;
#endif
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_CFG_10_vprd_pixel_format_10_START (0)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_CFG_10_vprd_pixel_format_10_END (4)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_CFG_10_vprd_pixel_expansion_10_START (5)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_CFG_10_vprd_pixel_expansion_10_END (5)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_CFG_10_vprd_allocated_du_10_START (6)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_CFG_10_vprd_allocated_du_10_END (10)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_CFG_10_vprd_last_page_10_START (13)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_CFG_10_vprd_last_page_10_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int vprd_line_size_10 : 15;
        unsigned int reserved_0 : 1;
        unsigned int vprd_horizontal_blanking_10 : 8;
        unsigned int reserved_1 : 8;
    } reg;
} SOC_IPP_CVDR_CVDR_IPP_VP_RD_LWG_10_UNION;
#endif
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_LWG_10_vprd_line_size_10_START (0)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_LWG_10_vprd_line_size_10_END (14)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_LWG_10_vprd_horizontal_blanking_10_START (16)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_LWG_10_vprd_horizontal_blanking_10_END (23)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int vprd_frame_size_10 : 15;
        unsigned int reserved_0 : 1;
        unsigned int vprd_vertical_blanking_10 : 8;
        unsigned int reserved_1 : 8;
    } reg;
} SOC_IPP_CVDR_CVDR_IPP_VP_RD_FHG_10_UNION;
#endif
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_FHG_10_vprd_frame_size_10_START (0)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_FHG_10_vprd_frame_size_10_END (14)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_FHG_10_vprd_vertical_blanking_10_START (16)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_FHG_10_vprd_vertical_blanking_10_END (23)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int reserved : 2;
        unsigned int vprd_axi_frame_start_10 : 30;
    } reg;
} SOC_IPP_CVDR_CVDR_IPP_VP_RD_AXI_FS_10_UNION;
#endif
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_AXI_FS_10_vprd_axi_frame_start_10_START (2)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_AXI_FS_10_vprd_axi_frame_start_10_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int vprd_line_stride_10 : 14;
        unsigned int reserved : 4;
        unsigned int vprd_line_wrap_10 : 14;
    } reg;
} SOC_IPP_CVDR_CVDR_IPP_VP_RD_AXI_LINE_10_UNION;
#endif
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_AXI_LINE_10_vprd_line_stride_10_START (0)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_AXI_LINE_10_vprd_line_stride_10_END (13)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_AXI_LINE_10_vprd_line_wrap_10_START (18)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_AXI_LINE_10_vprd_line_wrap_10_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int reserved_0 : 16;
        unsigned int vp_rd_stop_enable_du_threshold_reached_10 : 1;
        unsigned int vp_rd_stop_enable_flux_ctrl_10 : 1;
        unsigned int vp_rd_stop_enable_pressure_10 : 1;
        unsigned int reserved_1 : 5;
        unsigned int vp_rd_stop_ok_10 : 1;
        unsigned int vp_rd_stop_10 : 1;
        unsigned int reserved_2 : 5;
        unsigned int vprd_prefetch_bypass_10 : 1;
    } reg;
} SOC_IPP_CVDR_CVDR_IPP_VP_RD_IF_CFG_10_UNION;
#endif
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_IF_CFG_10_vp_rd_stop_enable_du_threshold_reached_10_START (16)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_IF_CFG_10_vp_rd_stop_enable_du_threshold_reached_10_END (16)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_IF_CFG_10_vp_rd_stop_enable_flux_ctrl_10_START (17)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_IF_CFG_10_vp_rd_stop_enable_flux_ctrl_10_END (17)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_IF_CFG_10_vp_rd_stop_enable_pressure_10_START (18)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_IF_CFG_10_vp_rd_stop_enable_pressure_10_END (18)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_IF_CFG_10_vp_rd_stop_ok_10_START (24)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_IF_CFG_10_vp_rd_stop_ok_10_END (24)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_IF_CFG_10_vp_rd_stop_10_START (25)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_IF_CFG_10_vp_rd_stop_10_END (25)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_IF_CFG_10_vprd_prefetch_bypass_10_START (31)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_IF_CFG_10_vprd_prefetch_bypass_10_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int vprd_access_limiter_0_10 : 4;
        unsigned int vprd_access_limiter_1_10 : 4;
        unsigned int vprd_access_limiter_2_10 : 4;
        unsigned int vprd_access_limiter_3_10 : 4;
        unsigned int reserved_0 : 8;
        unsigned int vprd_access_limiter_reload_10 : 4;
        unsigned int reserved_1 : 4;
    } reg;
} SOC_IPP_CVDR_CVDR_IPP_VP_RD_LIMITER_10_UNION;
#endif
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_LIMITER_10_vprd_access_limiter_0_10_START (0)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_LIMITER_10_vprd_access_limiter_0_10_END (3)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_LIMITER_10_vprd_access_limiter_1_10_START (4)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_LIMITER_10_vprd_access_limiter_1_10_END (7)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_LIMITER_10_vprd_access_limiter_2_10_START (8)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_LIMITER_10_vprd_access_limiter_2_10_END (11)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_LIMITER_10_vprd_access_limiter_3_10_START (12)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_LIMITER_10_vprd_access_limiter_3_10_END (15)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_LIMITER_10_vprd_access_limiter_reload_10_START (24)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_LIMITER_10_vprd_access_limiter_reload_10_END (27)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int vp_rd_debug_10 : 32;
    } reg;
} SOC_IPP_CVDR_CVDR_IPP_VP_RD_DEBUG_10_UNION;
#endif
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_DEBUG_10_vp_rd_debug_10_START (0)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_DEBUG_10_vp_rd_debug_10_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int vprd_pixel_format_12 : 5;
        unsigned int vprd_pixel_expansion_12 : 1;
        unsigned int vprd_allocated_du_12 : 5;
        unsigned int reserved : 2;
        unsigned int vprd_last_page_12 : 19;
    } reg;
} SOC_IPP_CVDR_CVDR_IPP_VP_RD_CFG_12_UNION;
#endif
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_CFG_12_vprd_pixel_format_12_START (0)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_CFG_12_vprd_pixel_format_12_END (4)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_CFG_12_vprd_pixel_expansion_12_START (5)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_CFG_12_vprd_pixel_expansion_12_END (5)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_CFG_12_vprd_allocated_du_12_START (6)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_CFG_12_vprd_allocated_du_12_END (10)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_CFG_12_vprd_last_page_12_START (13)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_CFG_12_vprd_last_page_12_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int vprd_line_size_12 : 15;
        unsigned int reserved_0 : 1;
        unsigned int vprd_horizontal_blanking_12 : 8;
        unsigned int reserved_1 : 8;
    } reg;
} SOC_IPP_CVDR_CVDR_IPP_VP_RD_LWG_12_UNION;
#endif
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_LWG_12_vprd_line_size_12_START (0)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_LWG_12_vprd_line_size_12_END (14)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_LWG_12_vprd_horizontal_blanking_12_START (16)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_LWG_12_vprd_horizontal_blanking_12_END (23)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int vprd_frame_size_12 : 15;
        unsigned int reserved_0 : 1;
        unsigned int vprd_vertical_blanking_12 : 8;
        unsigned int reserved_1 : 8;
    } reg;
} SOC_IPP_CVDR_CVDR_IPP_VP_RD_FHG_12_UNION;
#endif
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_FHG_12_vprd_frame_size_12_START (0)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_FHG_12_vprd_frame_size_12_END (14)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_FHG_12_vprd_vertical_blanking_12_START (16)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_FHG_12_vprd_vertical_blanking_12_END (23)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int reserved : 2;
        unsigned int vprd_axi_frame_start_12 : 30;
    } reg;
} SOC_IPP_CVDR_CVDR_IPP_VP_RD_AXI_FS_12_UNION;
#endif
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_AXI_FS_12_vprd_axi_frame_start_12_START (2)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_AXI_FS_12_vprd_axi_frame_start_12_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int vprd_line_stride_12 : 14;
        unsigned int reserved : 4;
        unsigned int vprd_line_wrap_12 : 14;
    } reg;
} SOC_IPP_CVDR_CVDR_IPP_VP_RD_AXI_LINE_12_UNION;
#endif
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_AXI_LINE_12_vprd_line_stride_12_START (0)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_AXI_LINE_12_vprd_line_stride_12_END (13)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_AXI_LINE_12_vprd_line_wrap_12_START (18)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_AXI_LINE_12_vprd_line_wrap_12_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int reserved_0 : 16;
        unsigned int vp_rd_stop_enable_du_threshold_reached_12 : 1;
        unsigned int vp_rd_stop_enable_flux_ctrl_12 : 1;
        unsigned int vp_rd_stop_enable_pressure_12 : 1;
        unsigned int reserved_1 : 5;
        unsigned int vp_rd_stop_ok_12 : 1;
        unsigned int vp_rd_stop_12 : 1;
        unsigned int reserved_2 : 5;
        unsigned int vprd_prefetch_bypass_12 : 1;
    } reg;
} SOC_IPP_CVDR_CVDR_IPP_VP_RD_IF_CFG_12_UNION;
#endif
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_IF_CFG_12_vp_rd_stop_enable_du_threshold_reached_12_START (16)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_IF_CFG_12_vp_rd_stop_enable_du_threshold_reached_12_END (16)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_IF_CFG_12_vp_rd_stop_enable_flux_ctrl_12_START (17)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_IF_CFG_12_vp_rd_stop_enable_flux_ctrl_12_END (17)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_IF_CFG_12_vp_rd_stop_enable_pressure_12_START (18)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_IF_CFG_12_vp_rd_stop_enable_pressure_12_END (18)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_IF_CFG_12_vp_rd_stop_ok_12_START (24)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_IF_CFG_12_vp_rd_stop_ok_12_END (24)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_IF_CFG_12_vp_rd_stop_12_START (25)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_IF_CFG_12_vp_rd_stop_12_END (25)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_IF_CFG_12_vprd_prefetch_bypass_12_START (31)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_IF_CFG_12_vprd_prefetch_bypass_12_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int vprd_access_limiter_0_12 : 4;
        unsigned int vprd_access_limiter_1_12 : 4;
        unsigned int vprd_access_limiter_2_12 : 4;
        unsigned int vprd_access_limiter_3_12 : 4;
        unsigned int reserved_0 : 8;
        unsigned int vprd_access_limiter_reload_12 : 4;
        unsigned int reserved_1 : 4;
    } reg;
} SOC_IPP_CVDR_CVDR_IPP_VP_RD_LIMITER_12_UNION;
#endif
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_LIMITER_12_vprd_access_limiter_0_12_START (0)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_LIMITER_12_vprd_access_limiter_0_12_END (3)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_LIMITER_12_vprd_access_limiter_1_12_START (4)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_LIMITER_12_vprd_access_limiter_1_12_END (7)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_LIMITER_12_vprd_access_limiter_2_12_START (8)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_LIMITER_12_vprd_access_limiter_2_12_END (11)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_LIMITER_12_vprd_access_limiter_3_12_START (12)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_LIMITER_12_vprd_access_limiter_3_12_END (15)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_LIMITER_12_vprd_access_limiter_reload_12_START (24)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_LIMITER_12_vprd_access_limiter_reload_12_END (27)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int vp_rd_debug_12 : 32;
    } reg;
} SOC_IPP_CVDR_CVDR_IPP_VP_RD_DEBUG_12_UNION;
#endif
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_DEBUG_12_vp_rd_debug_12_START (0)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_DEBUG_12_vp_rd_debug_12_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int vprd_pixel_format_13 : 5;
        unsigned int vprd_pixel_expansion_13 : 1;
        unsigned int vprd_allocated_du_13 : 5;
        unsigned int reserved : 2;
        unsigned int vprd_last_page_13 : 19;
    } reg;
} SOC_IPP_CVDR_CVDR_IPP_VP_RD_CFG_13_UNION;
#endif
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_CFG_13_vprd_pixel_format_13_START (0)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_CFG_13_vprd_pixel_format_13_END (4)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_CFG_13_vprd_pixel_expansion_13_START (5)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_CFG_13_vprd_pixel_expansion_13_END (5)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_CFG_13_vprd_allocated_du_13_START (6)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_CFG_13_vprd_allocated_du_13_END (10)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_CFG_13_vprd_last_page_13_START (13)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_CFG_13_vprd_last_page_13_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int vprd_line_size_13 : 15;
        unsigned int reserved_0 : 1;
        unsigned int vprd_horizontal_blanking_13 : 8;
        unsigned int reserved_1 : 8;
    } reg;
} SOC_IPP_CVDR_CVDR_IPP_VP_RD_LWG_13_UNION;
#endif
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_LWG_13_vprd_line_size_13_START (0)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_LWG_13_vprd_line_size_13_END (14)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_LWG_13_vprd_horizontal_blanking_13_START (16)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_LWG_13_vprd_horizontal_blanking_13_END (23)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int vprd_frame_size_13 : 15;
        unsigned int reserved_0 : 1;
        unsigned int vprd_vertical_blanking_13 : 8;
        unsigned int reserved_1 : 8;
    } reg;
} SOC_IPP_CVDR_CVDR_IPP_VP_RD_FHG_13_UNION;
#endif
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_FHG_13_vprd_frame_size_13_START (0)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_FHG_13_vprd_frame_size_13_END (14)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_FHG_13_vprd_vertical_blanking_13_START (16)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_FHG_13_vprd_vertical_blanking_13_END (23)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int reserved : 2;
        unsigned int vprd_axi_frame_start_13 : 30;
    } reg;
} SOC_IPP_CVDR_CVDR_IPP_VP_RD_AXI_FS_13_UNION;
#endif
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_AXI_FS_13_vprd_axi_frame_start_13_START (2)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_AXI_FS_13_vprd_axi_frame_start_13_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int vprd_line_stride_13 : 14;
        unsigned int reserved : 4;
        unsigned int vprd_line_wrap_13 : 14;
    } reg;
} SOC_IPP_CVDR_CVDR_IPP_VP_RD_AXI_LINE_13_UNION;
#endif
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_AXI_LINE_13_vprd_line_stride_13_START (0)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_AXI_LINE_13_vprd_line_stride_13_END (13)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_AXI_LINE_13_vprd_line_wrap_13_START (18)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_AXI_LINE_13_vprd_line_wrap_13_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int reserved_0 : 16;
        unsigned int vp_rd_stop_enable_du_threshold_reached_13 : 1;
        unsigned int vp_rd_stop_enable_flux_ctrl_13 : 1;
        unsigned int vp_rd_stop_enable_pressure_13 : 1;
        unsigned int reserved_1 : 5;
        unsigned int vp_rd_stop_ok_13 : 1;
        unsigned int vp_rd_stop_13 : 1;
        unsigned int reserved_2 : 5;
        unsigned int vprd_prefetch_bypass_13 : 1;
    } reg;
} SOC_IPP_CVDR_CVDR_IPP_VP_RD_IF_CFG_13_UNION;
#endif
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_IF_CFG_13_vp_rd_stop_enable_du_threshold_reached_13_START (16)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_IF_CFG_13_vp_rd_stop_enable_du_threshold_reached_13_END (16)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_IF_CFG_13_vp_rd_stop_enable_flux_ctrl_13_START (17)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_IF_CFG_13_vp_rd_stop_enable_flux_ctrl_13_END (17)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_IF_CFG_13_vp_rd_stop_enable_pressure_13_START (18)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_IF_CFG_13_vp_rd_stop_enable_pressure_13_END (18)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_IF_CFG_13_vp_rd_stop_ok_13_START (24)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_IF_CFG_13_vp_rd_stop_ok_13_END (24)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_IF_CFG_13_vp_rd_stop_13_START (25)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_IF_CFG_13_vp_rd_stop_13_END (25)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_IF_CFG_13_vprd_prefetch_bypass_13_START (31)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_IF_CFG_13_vprd_prefetch_bypass_13_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int vprd_access_limiter_0_13 : 4;
        unsigned int vprd_access_limiter_1_13 : 4;
        unsigned int vprd_access_limiter_2_13 : 4;
        unsigned int vprd_access_limiter_3_13 : 4;
        unsigned int reserved_0 : 8;
        unsigned int vprd_access_limiter_reload_13 : 4;
        unsigned int reserved_1 : 4;
    } reg;
} SOC_IPP_CVDR_CVDR_IPP_VP_RD_LIMITER_13_UNION;
#endif
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_LIMITER_13_vprd_access_limiter_0_13_START (0)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_LIMITER_13_vprd_access_limiter_0_13_END (3)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_LIMITER_13_vprd_access_limiter_1_13_START (4)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_LIMITER_13_vprd_access_limiter_1_13_END (7)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_LIMITER_13_vprd_access_limiter_2_13_START (8)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_LIMITER_13_vprd_access_limiter_2_13_END (11)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_LIMITER_13_vprd_access_limiter_3_13_START (12)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_LIMITER_13_vprd_access_limiter_3_13_END (15)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_LIMITER_13_vprd_access_limiter_reload_13_START (24)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_LIMITER_13_vprd_access_limiter_reload_13_END (27)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int vp_rd_debug_13 : 32;
    } reg;
} SOC_IPP_CVDR_CVDR_IPP_VP_RD_DEBUG_13_UNION;
#endif
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_DEBUG_13_vp_rd_debug_13_START (0)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_DEBUG_13_vp_rd_debug_13_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int vprd_pixel_format_14 : 5;
        unsigned int vprd_pixel_expansion_14 : 1;
        unsigned int vprd_allocated_du_14 : 5;
        unsigned int reserved : 2;
        unsigned int vprd_last_page_14 : 19;
    } reg;
} SOC_IPP_CVDR_CVDR_IPP_VP_RD_CFG_14_UNION;
#endif
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_CFG_14_vprd_pixel_format_14_START (0)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_CFG_14_vprd_pixel_format_14_END (4)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_CFG_14_vprd_pixel_expansion_14_START (5)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_CFG_14_vprd_pixel_expansion_14_END (5)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_CFG_14_vprd_allocated_du_14_START (6)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_CFG_14_vprd_allocated_du_14_END (10)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_CFG_14_vprd_last_page_14_START (13)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_CFG_14_vprd_last_page_14_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int vprd_line_size_14 : 15;
        unsigned int reserved_0 : 1;
        unsigned int vprd_horizontal_blanking_14 : 8;
        unsigned int reserved_1 : 8;
    } reg;
} SOC_IPP_CVDR_CVDR_IPP_VP_RD_LWG_14_UNION;
#endif
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_LWG_14_vprd_line_size_14_START (0)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_LWG_14_vprd_line_size_14_END (14)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_LWG_14_vprd_horizontal_blanking_14_START (16)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_LWG_14_vprd_horizontal_blanking_14_END (23)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int vprd_frame_size_14 : 15;
        unsigned int reserved_0 : 1;
        unsigned int vprd_vertical_blanking_14 : 8;
        unsigned int reserved_1 : 8;
    } reg;
} SOC_IPP_CVDR_CVDR_IPP_VP_RD_FHG_14_UNION;
#endif
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_FHG_14_vprd_frame_size_14_START (0)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_FHG_14_vprd_frame_size_14_END (14)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_FHG_14_vprd_vertical_blanking_14_START (16)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_FHG_14_vprd_vertical_blanking_14_END (23)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int reserved : 2;
        unsigned int vprd_axi_frame_start_14 : 30;
    } reg;
} SOC_IPP_CVDR_CVDR_IPP_VP_RD_AXI_FS_14_UNION;
#endif
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_AXI_FS_14_vprd_axi_frame_start_14_START (2)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_AXI_FS_14_vprd_axi_frame_start_14_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int vprd_line_stride_14 : 14;
        unsigned int reserved : 4;
        unsigned int vprd_line_wrap_14 : 14;
    } reg;
} SOC_IPP_CVDR_CVDR_IPP_VP_RD_AXI_LINE_14_UNION;
#endif
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_AXI_LINE_14_vprd_line_stride_14_START (0)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_AXI_LINE_14_vprd_line_stride_14_END (13)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_AXI_LINE_14_vprd_line_wrap_14_START (18)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_AXI_LINE_14_vprd_line_wrap_14_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int reserved_0 : 16;
        unsigned int vp_rd_stop_enable_du_threshold_reached_14 : 1;
        unsigned int vp_rd_stop_enable_flux_ctrl_14 : 1;
        unsigned int vp_rd_stop_enable_pressure_14 : 1;
        unsigned int reserved_1 : 5;
        unsigned int vp_rd_stop_ok_14 : 1;
        unsigned int vp_rd_stop_14 : 1;
        unsigned int reserved_2 : 5;
        unsigned int vprd_prefetch_bypass_14 : 1;
    } reg;
} SOC_IPP_CVDR_CVDR_IPP_VP_RD_IF_CFG_14_UNION;
#endif
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_IF_CFG_14_vp_rd_stop_enable_du_threshold_reached_14_START (16)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_IF_CFG_14_vp_rd_stop_enable_du_threshold_reached_14_END (16)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_IF_CFG_14_vp_rd_stop_enable_flux_ctrl_14_START (17)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_IF_CFG_14_vp_rd_stop_enable_flux_ctrl_14_END (17)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_IF_CFG_14_vp_rd_stop_enable_pressure_14_START (18)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_IF_CFG_14_vp_rd_stop_enable_pressure_14_END (18)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_IF_CFG_14_vp_rd_stop_ok_14_START (24)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_IF_CFG_14_vp_rd_stop_ok_14_END (24)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_IF_CFG_14_vp_rd_stop_14_START (25)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_IF_CFG_14_vp_rd_stop_14_END (25)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_IF_CFG_14_vprd_prefetch_bypass_14_START (31)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_IF_CFG_14_vprd_prefetch_bypass_14_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int vprd_access_limiter_0_14 : 4;
        unsigned int vprd_access_limiter_1_14 : 4;
        unsigned int vprd_access_limiter_2_14 : 4;
        unsigned int vprd_access_limiter_3_14 : 4;
        unsigned int reserved_0 : 8;
        unsigned int vprd_access_limiter_reload_14 : 4;
        unsigned int reserved_1 : 4;
    } reg;
} SOC_IPP_CVDR_CVDR_IPP_VP_RD_LIMITER_14_UNION;
#endif
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_LIMITER_14_vprd_access_limiter_0_14_START (0)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_LIMITER_14_vprd_access_limiter_0_14_END (3)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_LIMITER_14_vprd_access_limiter_1_14_START (4)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_LIMITER_14_vprd_access_limiter_1_14_END (7)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_LIMITER_14_vprd_access_limiter_2_14_START (8)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_LIMITER_14_vprd_access_limiter_2_14_END (11)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_LIMITER_14_vprd_access_limiter_3_14_START (12)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_LIMITER_14_vprd_access_limiter_3_14_END (15)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_LIMITER_14_vprd_access_limiter_reload_14_START (24)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_LIMITER_14_vprd_access_limiter_reload_14_END (27)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int vp_rd_debug_14 : 32;
    } reg;
} SOC_IPP_CVDR_CVDR_IPP_VP_RD_DEBUG_14_UNION;
#endif
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_DEBUG_14_vp_rd_debug_14_START (0)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_DEBUG_14_vp_rd_debug_14_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int vprd_pixel_format_15 : 5;
        unsigned int vprd_pixel_expansion_15 : 1;
        unsigned int vprd_allocated_du_15 : 5;
        unsigned int reserved : 2;
        unsigned int vprd_last_page_15 : 19;
    } reg;
} SOC_IPP_CVDR_CVDR_IPP_VP_RD_CFG_15_UNION;
#endif
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_CFG_15_vprd_pixel_format_15_START (0)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_CFG_15_vprd_pixel_format_15_END (4)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_CFG_15_vprd_pixel_expansion_15_START (5)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_CFG_15_vprd_pixel_expansion_15_END (5)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_CFG_15_vprd_allocated_du_15_START (6)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_CFG_15_vprd_allocated_du_15_END (10)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_CFG_15_vprd_last_page_15_START (13)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_CFG_15_vprd_last_page_15_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int vprd_line_size_15 : 15;
        unsigned int reserved_0 : 1;
        unsigned int vprd_horizontal_blanking_15 : 8;
        unsigned int reserved_1 : 8;
    } reg;
} SOC_IPP_CVDR_CVDR_IPP_VP_RD_LWG_15_UNION;
#endif
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_LWG_15_vprd_line_size_15_START (0)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_LWG_15_vprd_line_size_15_END (14)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_LWG_15_vprd_horizontal_blanking_15_START (16)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_LWG_15_vprd_horizontal_blanking_15_END (23)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int vprd_frame_size_15 : 15;
        unsigned int reserved_0 : 1;
        unsigned int vprd_vertical_blanking_15 : 8;
        unsigned int reserved_1 : 8;
    } reg;
} SOC_IPP_CVDR_CVDR_IPP_VP_RD_FHG_15_UNION;
#endif
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_FHG_15_vprd_frame_size_15_START (0)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_FHG_15_vprd_frame_size_15_END (14)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_FHG_15_vprd_vertical_blanking_15_START (16)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_FHG_15_vprd_vertical_blanking_15_END (23)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int reserved : 2;
        unsigned int vprd_axi_frame_start_15 : 30;
    } reg;
} SOC_IPP_CVDR_CVDR_IPP_VP_RD_AXI_FS_15_UNION;
#endif
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_AXI_FS_15_vprd_axi_frame_start_15_START (2)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_AXI_FS_15_vprd_axi_frame_start_15_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int vprd_line_stride_15 : 14;
        unsigned int reserved : 4;
        unsigned int vprd_line_wrap_15 : 14;
    } reg;
} SOC_IPP_CVDR_CVDR_IPP_VP_RD_AXI_LINE_15_UNION;
#endif
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_AXI_LINE_15_vprd_line_stride_15_START (0)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_AXI_LINE_15_vprd_line_stride_15_END (13)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_AXI_LINE_15_vprd_line_wrap_15_START (18)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_AXI_LINE_15_vprd_line_wrap_15_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int reserved_0 : 16;
        unsigned int vp_rd_stop_enable_du_threshold_reached_15 : 1;
        unsigned int vp_rd_stop_enable_flux_ctrl_15 : 1;
        unsigned int vp_rd_stop_enable_pressure_15 : 1;
        unsigned int reserved_1 : 5;
        unsigned int vp_rd_stop_ok_15 : 1;
        unsigned int vp_rd_stop_15 : 1;
        unsigned int reserved_2 : 5;
        unsigned int vprd_prefetch_bypass_15 : 1;
    } reg;
} SOC_IPP_CVDR_CVDR_IPP_VP_RD_IF_CFG_15_UNION;
#endif
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_IF_CFG_15_vp_rd_stop_enable_du_threshold_reached_15_START (16)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_IF_CFG_15_vp_rd_stop_enable_du_threshold_reached_15_END (16)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_IF_CFG_15_vp_rd_stop_enable_flux_ctrl_15_START (17)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_IF_CFG_15_vp_rd_stop_enable_flux_ctrl_15_END (17)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_IF_CFG_15_vp_rd_stop_enable_pressure_15_START (18)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_IF_CFG_15_vp_rd_stop_enable_pressure_15_END (18)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_IF_CFG_15_vp_rd_stop_ok_15_START (24)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_IF_CFG_15_vp_rd_stop_ok_15_END (24)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_IF_CFG_15_vp_rd_stop_15_START (25)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_IF_CFG_15_vp_rd_stop_15_END (25)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_IF_CFG_15_vprd_prefetch_bypass_15_START (31)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_IF_CFG_15_vprd_prefetch_bypass_15_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int vprd_access_limiter_0_15 : 4;
        unsigned int vprd_access_limiter_1_15 : 4;
        unsigned int vprd_access_limiter_2_15 : 4;
        unsigned int vprd_access_limiter_3_15 : 4;
        unsigned int reserved_0 : 8;
        unsigned int vprd_access_limiter_reload_15 : 4;
        unsigned int reserved_1 : 4;
    } reg;
} SOC_IPP_CVDR_CVDR_IPP_VP_RD_LIMITER_15_UNION;
#endif
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_LIMITER_15_vprd_access_limiter_0_15_START (0)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_LIMITER_15_vprd_access_limiter_0_15_END (3)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_LIMITER_15_vprd_access_limiter_1_15_START (4)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_LIMITER_15_vprd_access_limiter_1_15_END (7)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_LIMITER_15_vprd_access_limiter_2_15_START (8)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_LIMITER_15_vprd_access_limiter_2_15_END (11)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_LIMITER_15_vprd_access_limiter_3_15_START (12)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_LIMITER_15_vprd_access_limiter_3_15_END (15)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_LIMITER_15_vprd_access_limiter_reload_15_START (24)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_LIMITER_15_vprd_access_limiter_reload_15_END (27)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int vp_rd_debug_15 : 32;
    } reg;
} SOC_IPP_CVDR_CVDR_IPP_VP_RD_DEBUG_15_UNION;
#endif
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_DEBUG_15_vp_rd_debug_15_START (0)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_DEBUG_15_vp_rd_debug_15_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int vprd_pixel_format_16 : 5;
        unsigned int vprd_pixel_expansion_16 : 1;
        unsigned int vprd_allocated_du_16 : 5;
        unsigned int reserved : 2;
        unsigned int vprd_last_page_16 : 19;
    } reg;
} SOC_IPP_CVDR_CVDR_IPP_VP_RD_CFG_16_UNION;
#endif
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_CFG_16_vprd_pixel_format_16_START (0)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_CFG_16_vprd_pixel_format_16_END (4)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_CFG_16_vprd_pixel_expansion_16_START (5)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_CFG_16_vprd_pixel_expansion_16_END (5)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_CFG_16_vprd_allocated_du_16_START (6)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_CFG_16_vprd_allocated_du_16_END (10)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_CFG_16_vprd_last_page_16_START (13)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_CFG_16_vprd_last_page_16_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int vprd_line_size_16 : 15;
        unsigned int reserved_0 : 1;
        unsigned int vprd_horizontal_blanking_16 : 8;
        unsigned int reserved_1 : 8;
    } reg;
} SOC_IPP_CVDR_CVDR_IPP_VP_RD_LWG_16_UNION;
#endif
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_LWG_16_vprd_line_size_16_START (0)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_LWG_16_vprd_line_size_16_END (14)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_LWG_16_vprd_horizontal_blanking_16_START (16)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_LWG_16_vprd_horizontal_blanking_16_END (23)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int vprd_frame_size_16 : 15;
        unsigned int reserved_0 : 1;
        unsigned int vprd_vertical_blanking_16 : 8;
        unsigned int reserved_1 : 8;
    } reg;
} SOC_IPP_CVDR_CVDR_IPP_VP_RD_FHG_16_UNION;
#endif
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_FHG_16_vprd_frame_size_16_START (0)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_FHG_16_vprd_frame_size_16_END (14)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_FHG_16_vprd_vertical_blanking_16_START (16)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_FHG_16_vprd_vertical_blanking_16_END (23)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int reserved : 2;
        unsigned int vprd_axi_frame_start_16 : 30;
    } reg;
} SOC_IPP_CVDR_CVDR_IPP_VP_RD_AXI_FS_16_UNION;
#endif
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_AXI_FS_16_vprd_axi_frame_start_16_START (2)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_AXI_FS_16_vprd_axi_frame_start_16_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int vprd_line_stride_16 : 14;
        unsigned int reserved : 4;
        unsigned int vprd_line_wrap_16 : 14;
    } reg;
} SOC_IPP_CVDR_CVDR_IPP_VP_RD_AXI_LINE_16_UNION;
#endif
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_AXI_LINE_16_vprd_line_stride_16_START (0)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_AXI_LINE_16_vprd_line_stride_16_END (13)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_AXI_LINE_16_vprd_line_wrap_16_START (18)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_AXI_LINE_16_vprd_line_wrap_16_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int reserved_0 : 16;
        unsigned int vp_rd_stop_enable_du_threshold_reached_16 : 1;
        unsigned int vp_rd_stop_enable_flux_ctrl_16 : 1;
        unsigned int vp_rd_stop_enable_pressure_16 : 1;
        unsigned int reserved_1 : 5;
        unsigned int vp_rd_stop_ok_16 : 1;
        unsigned int vp_rd_stop_16 : 1;
        unsigned int reserved_2 : 5;
        unsigned int vprd_prefetch_bypass_16 : 1;
    } reg;
} SOC_IPP_CVDR_CVDR_IPP_VP_RD_IF_CFG_16_UNION;
#endif
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_IF_CFG_16_vp_rd_stop_enable_du_threshold_reached_16_START (16)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_IF_CFG_16_vp_rd_stop_enable_du_threshold_reached_16_END (16)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_IF_CFG_16_vp_rd_stop_enable_flux_ctrl_16_START (17)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_IF_CFG_16_vp_rd_stop_enable_flux_ctrl_16_END (17)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_IF_CFG_16_vp_rd_stop_enable_pressure_16_START (18)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_IF_CFG_16_vp_rd_stop_enable_pressure_16_END (18)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_IF_CFG_16_vp_rd_stop_ok_16_START (24)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_IF_CFG_16_vp_rd_stop_ok_16_END (24)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_IF_CFG_16_vp_rd_stop_16_START (25)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_IF_CFG_16_vp_rd_stop_16_END (25)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_IF_CFG_16_vprd_prefetch_bypass_16_START (31)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_IF_CFG_16_vprd_prefetch_bypass_16_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int vprd_access_limiter_0_16 : 4;
        unsigned int vprd_access_limiter_1_16 : 4;
        unsigned int vprd_access_limiter_2_16 : 4;
        unsigned int vprd_access_limiter_3_16 : 4;
        unsigned int reserved_0 : 8;
        unsigned int vprd_access_limiter_reload_16 : 4;
        unsigned int reserved_1 : 4;
    } reg;
} SOC_IPP_CVDR_CVDR_IPP_VP_RD_LIMITER_16_UNION;
#endif
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_LIMITER_16_vprd_access_limiter_0_16_START (0)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_LIMITER_16_vprd_access_limiter_0_16_END (3)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_LIMITER_16_vprd_access_limiter_1_16_START (4)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_LIMITER_16_vprd_access_limiter_1_16_END (7)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_LIMITER_16_vprd_access_limiter_2_16_START (8)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_LIMITER_16_vprd_access_limiter_2_16_END (11)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_LIMITER_16_vprd_access_limiter_3_16_START (12)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_LIMITER_16_vprd_access_limiter_3_16_END (15)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_LIMITER_16_vprd_access_limiter_reload_16_START (24)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_LIMITER_16_vprd_access_limiter_reload_16_END (27)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int vp_rd_debug_16 : 32;
    } reg;
} SOC_IPP_CVDR_CVDR_IPP_VP_RD_DEBUG_16_UNION;
#endif
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_DEBUG_16_vp_rd_debug_16_START (0)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_DEBUG_16_vp_rd_debug_16_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int vprd_pixel_format_17 : 5;
        unsigned int vprd_pixel_expansion_17 : 1;
        unsigned int vprd_allocated_du_17 : 5;
        unsigned int reserved : 2;
        unsigned int vprd_last_page_17 : 19;
    } reg;
} SOC_IPP_CVDR_CVDR_IPP_VP_RD_CFG_17_UNION;
#endif
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_CFG_17_vprd_pixel_format_17_START (0)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_CFG_17_vprd_pixel_format_17_END (4)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_CFG_17_vprd_pixel_expansion_17_START (5)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_CFG_17_vprd_pixel_expansion_17_END (5)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_CFG_17_vprd_allocated_du_17_START (6)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_CFG_17_vprd_allocated_du_17_END (10)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_CFG_17_vprd_last_page_17_START (13)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_CFG_17_vprd_last_page_17_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int vprd_line_size_17 : 15;
        unsigned int reserved_0 : 1;
        unsigned int vprd_horizontal_blanking_17 : 8;
        unsigned int reserved_1 : 8;
    } reg;
} SOC_IPP_CVDR_CVDR_IPP_VP_RD_LWG_17_UNION;
#endif
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_LWG_17_vprd_line_size_17_START (0)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_LWG_17_vprd_line_size_17_END (14)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_LWG_17_vprd_horizontal_blanking_17_START (16)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_LWG_17_vprd_horizontal_blanking_17_END (23)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int vprd_frame_size_17 : 15;
        unsigned int reserved_0 : 1;
        unsigned int vprd_vertical_blanking_17 : 8;
        unsigned int reserved_1 : 8;
    } reg;
} SOC_IPP_CVDR_CVDR_IPP_VP_RD_FHG_17_UNION;
#endif
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_FHG_17_vprd_frame_size_17_START (0)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_FHG_17_vprd_frame_size_17_END (14)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_FHG_17_vprd_vertical_blanking_17_START (16)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_FHG_17_vprd_vertical_blanking_17_END (23)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int reserved : 2;
        unsigned int vprd_axi_frame_start_17 : 30;
    } reg;
} SOC_IPP_CVDR_CVDR_IPP_VP_RD_AXI_FS_17_UNION;
#endif
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_AXI_FS_17_vprd_axi_frame_start_17_START (2)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_AXI_FS_17_vprd_axi_frame_start_17_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int vprd_line_stride_17 : 14;
        unsigned int reserved : 4;
        unsigned int vprd_line_wrap_17 : 14;
    } reg;
} SOC_IPP_CVDR_CVDR_IPP_VP_RD_AXI_LINE_17_UNION;
#endif
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_AXI_LINE_17_vprd_line_stride_17_START (0)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_AXI_LINE_17_vprd_line_stride_17_END (13)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_AXI_LINE_17_vprd_line_wrap_17_START (18)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_AXI_LINE_17_vprd_line_wrap_17_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int reserved_0 : 16;
        unsigned int vp_rd_stop_enable_du_threshold_reached_17 : 1;
        unsigned int vp_rd_stop_enable_flux_ctrl_17 : 1;
        unsigned int vp_rd_stop_enable_pressure_17 : 1;
        unsigned int reserved_1 : 5;
        unsigned int vp_rd_stop_ok_17 : 1;
        unsigned int vp_rd_stop_17 : 1;
        unsigned int reserved_2 : 5;
        unsigned int vprd_prefetch_bypass_17 : 1;
    } reg;
} SOC_IPP_CVDR_CVDR_IPP_VP_RD_IF_CFG_17_UNION;
#endif
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_IF_CFG_17_vp_rd_stop_enable_du_threshold_reached_17_START (16)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_IF_CFG_17_vp_rd_stop_enable_du_threshold_reached_17_END (16)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_IF_CFG_17_vp_rd_stop_enable_flux_ctrl_17_START (17)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_IF_CFG_17_vp_rd_stop_enable_flux_ctrl_17_END (17)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_IF_CFG_17_vp_rd_stop_enable_pressure_17_START (18)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_IF_CFG_17_vp_rd_stop_enable_pressure_17_END (18)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_IF_CFG_17_vp_rd_stop_ok_17_START (24)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_IF_CFG_17_vp_rd_stop_ok_17_END (24)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_IF_CFG_17_vp_rd_stop_17_START (25)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_IF_CFG_17_vp_rd_stop_17_END (25)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_IF_CFG_17_vprd_prefetch_bypass_17_START (31)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_IF_CFG_17_vprd_prefetch_bypass_17_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int vprd_access_limiter_0_17 : 4;
        unsigned int vprd_access_limiter_1_17 : 4;
        unsigned int vprd_access_limiter_2_17 : 4;
        unsigned int vprd_access_limiter_3_17 : 4;
        unsigned int reserved_0 : 8;
        unsigned int vprd_access_limiter_reload_17 : 4;
        unsigned int reserved_1 : 4;
    } reg;
} SOC_IPP_CVDR_CVDR_IPP_VP_RD_LIMITER_17_UNION;
#endif
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_LIMITER_17_vprd_access_limiter_0_17_START (0)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_LIMITER_17_vprd_access_limiter_0_17_END (3)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_LIMITER_17_vprd_access_limiter_1_17_START (4)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_LIMITER_17_vprd_access_limiter_1_17_END (7)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_LIMITER_17_vprd_access_limiter_2_17_START (8)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_LIMITER_17_vprd_access_limiter_2_17_END (11)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_LIMITER_17_vprd_access_limiter_3_17_START (12)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_LIMITER_17_vprd_access_limiter_3_17_END (15)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_LIMITER_17_vprd_access_limiter_reload_17_START (24)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_LIMITER_17_vprd_access_limiter_reload_17_END (27)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int vp_rd_debug_17 : 32;
    } reg;
} SOC_IPP_CVDR_CVDR_IPP_VP_RD_DEBUG_17_UNION;
#endif
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_DEBUG_17_vp_rd_debug_17_START (0)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_DEBUG_17_vp_rd_debug_17_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int vprd_pixel_format_18 : 5;
        unsigned int vprd_pixel_expansion_18 : 1;
        unsigned int vprd_allocated_du_18 : 5;
        unsigned int reserved : 2;
        unsigned int vprd_last_page_18 : 19;
    } reg;
} SOC_IPP_CVDR_CVDR_IPP_VP_RD_CFG_18_UNION;
#endif
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_CFG_18_vprd_pixel_format_18_START (0)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_CFG_18_vprd_pixel_format_18_END (4)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_CFG_18_vprd_pixel_expansion_18_START (5)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_CFG_18_vprd_pixel_expansion_18_END (5)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_CFG_18_vprd_allocated_du_18_START (6)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_CFG_18_vprd_allocated_du_18_END (10)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_CFG_18_vprd_last_page_18_START (13)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_CFG_18_vprd_last_page_18_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int vprd_line_size_18 : 15;
        unsigned int reserved_0 : 1;
        unsigned int vprd_horizontal_blanking_18 : 8;
        unsigned int reserved_1 : 8;
    } reg;
} SOC_IPP_CVDR_CVDR_IPP_VP_RD_LWG_18_UNION;
#endif
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_LWG_18_vprd_line_size_18_START (0)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_LWG_18_vprd_line_size_18_END (14)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_LWG_18_vprd_horizontal_blanking_18_START (16)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_LWG_18_vprd_horizontal_blanking_18_END (23)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int vprd_frame_size_18 : 15;
        unsigned int reserved_0 : 1;
        unsigned int vprd_vertical_blanking_18 : 8;
        unsigned int reserved_1 : 8;
    } reg;
} SOC_IPP_CVDR_CVDR_IPP_VP_RD_FHG_18_UNION;
#endif
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_FHG_18_vprd_frame_size_18_START (0)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_FHG_18_vprd_frame_size_18_END (14)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_FHG_18_vprd_vertical_blanking_18_START (16)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_FHG_18_vprd_vertical_blanking_18_END (23)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int reserved : 2;
        unsigned int vprd_axi_frame_start_18 : 30;
    } reg;
} SOC_IPP_CVDR_CVDR_IPP_VP_RD_AXI_FS_18_UNION;
#endif
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_AXI_FS_18_vprd_axi_frame_start_18_START (2)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_AXI_FS_18_vprd_axi_frame_start_18_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int vprd_line_stride_18 : 14;
        unsigned int reserved : 4;
        unsigned int vprd_line_wrap_18 : 14;
    } reg;
} SOC_IPP_CVDR_CVDR_IPP_VP_RD_AXI_LINE_18_UNION;
#endif
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_AXI_LINE_18_vprd_line_stride_18_START (0)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_AXI_LINE_18_vprd_line_stride_18_END (13)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_AXI_LINE_18_vprd_line_wrap_18_START (18)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_AXI_LINE_18_vprd_line_wrap_18_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int reserved_0 : 16;
        unsigned int vp_rd_stop_enable_du_threshold_reached_18 : 1;
        unsigned int vp_rd_stop_enable_flux_ctrl_18 : 1;
        unsigned int vp_rd_stop_enable_pressure_18 : 1;
        unsigned int reserved_1 : 5;
        unsigned int vp_rd_stop_ok_18 : 1;
        unsigned int vp_rd_stop_18 : 1;
        unsigned int reserved_2 : 5;
        unsigned int vprd_prefetch_bypass_18 : 1;
    } reg;
} SOC_IPP_CVDR_CVDR_IPP_VP_RD_IF_CFG_18_UNION;
#endif
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_IF_CFG_18_vp_rd_stop_enable_du_threshold_reached_18_START (16)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_IF_CFG_18_vp_rd_stop_enable_du_threshold_reached_18_END (16)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_IF_CFG_18_vp_rd_stop_enable_flux_ctrl_18_START (17)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_IF_CFG_18_vp_rd_stop_enable_flux_ctrl_18_END (17)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_IF_CFG_18_vp_rd_stop_enable_pressure_18_START (18)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_IF_CFG_18_vp_rd_stop_enable_pressure_18_END (18)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_IF_CFG_18_vp_rd_stop_ok_18_START (24)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_IF_CFG_18_vp_rd_stop_ok_18_END (24)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_IF_CFG_18_vp_rd_stop_18_START (25)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_IF_CFG_18_vp_rd_stop_18_END (25)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_IF_CFG_18_vprd_prefetch_bypass_18_START (31)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_IF_CFG_18_vprd_prefetch_bypass_18_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int vprd_access_limiter_0_18 : 4;
        unsigned int vprd_access_limiter_1_18 : 4;
        unsigned int vprd_access_limiter_2_18 : 4;
        unsigned int vprd_access_limiter_3_18 : 4;
        unsigned int reserved_0 : 8;
        unsigned int vprd_access_limiter_reload_18 : 4;
        unsigned int reserved_1 : 4;
    } reg;
} SOC_IPP_CVDR_CVDR_IPP_VP_RD_LIMITER_18_UNION;
#endif
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_LIMITER_18_vprd_access_limiter_0_18_START (0)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_LIMITER_18_vprd_access_limiter_0_18_END (3)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_LIMITER_18_vprd_access_limiter_1_18_START (4)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_LIMITER_18_vprd_access_limiter_1_18_END (7)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_LIMITER_18_vprd_access_limiter_2_18_START (8)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_LIMITER_18_vprd_access_limiter_2_18_END (11)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_LIMITER_18_vprd_access_limiter_3_18_START (12)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_LIMITER_18_vprd_access_limiter_3_18_END (15)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_LIMITER_18_vprd_access_limiter_reload_18_START (24)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_LIMITER_18_vprd_access_limiter_reload_18_END (27)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int vp_rd_debug_18 : 32;
    } reg;
} SOC_IPP_CVDR_CVDR_IPP_VP_RD_DEBUG_18_UNION;
#endif
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_DEBUG_18_vp_rd_debug_18_START (0)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_DEBUG_18_vp_rd_debug_18_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int vprd_pixel_format_19 : 5;
        unsigned int vprd_pixel_expansion_19 : 1;
        unsigned int vprd_allocated_du_19 : 5;
        unsigned int reserved : 2;
        unsigned int vprd_last_page_19 : 19;
    } reg;
} SOC_IPP_CVDR_CVDR_IPP_VP_RD_CFG_19_UNION;
#endif
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_CFG_19_vprd_pixel_format_19_START (0)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_CFG_19_vprd_pixel_format_19_END (4)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_CFG_19_vprd_pixel_expansion_19_START (5)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_CFG_19_vprd_pixel_expansion_19_END (5)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_CFG_19_vprd_allocated_du_19_START (6)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_CFG_19_vprd_allocated_du_19_END (10)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_CFG_19_vprd_last_page_19_START (13)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_CFG_19_vprd_last_page_19_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int vprd_line_size_19 : 15;
        unsigned int reserved_0 : 1;
        unsigned int vprd_horizontal_blanking_19 : 8;
        unsigned int reserved_1 : 8;
    } reg;
} SOC_IPP_CVDR_CVDR_IPP_VP_RD_LWG_19_UNION;
#endif
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_LWG_19_vprd_line_size_19_START (0)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_LWG_19_vprd_line_size_19_END (14)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_LWG_19_vprd_horizontal_blanking_19_START (16)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_LWG_19_vprd_horizontal_blanking_19_END (23)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int vprd_frame_size_19 : 15;
        unsigned int reserved_0 : 1;
        unsigned int vprd_vertical_blanking_19 : 8;
        unsigned int reserved_1 : 8;
    } reg;
} SOC_IPP_CVDR_CVDR_IPP_VP_RD_FHG_19_UNION;
#endif
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_FHG_19_vprd_frame_size_19_START (0)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_FHG_19_vprd_frame_size_19_END (14)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_FHG_19_vprd_vertical_blanking_19_START (16)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_FHG_19_vprd_vertical_blanking_19_END (23)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int reserved : 2;
        unsigned int vprd_axi_frame_start_19 : 30;
    } reg;
} SOC_IPP_CVDR_CVDR_IPP_VP_RD_AXI_FS_19_UNION;
#endif
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_AXI_FS_19_vprd_axi_frame_start_19_START (2)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_AXI_FS_19_vprd_axi_frame_start_19_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int vprd_line_stride_19 : 14;
        unsigned int reserved : 4;
        unsigned int vprd_line_wrap_19 : 14;
    } reg;
} SOC_IPP_CVDR_CVDR_IPP_VP_RD_AXI_LINE_19_UNION;
#endif
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_AXI_LINE_19_vprd_line_stride_19_START (0)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_AXI_LINE_19_vprd_line_stride_19_END (13)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_AXI_LINE_19_vprd_line_wrap_19_START (18)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_AXI_LINE_19_vprd_line_wrap_19_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int reserved_0 : 16;
        unsigned int vp_rd_stop_enable_du_threshold_reached_19 : 1;
        unsigned int vp_rd_stop_enable_flux_ctrl_19 : 1;
        unsigned int vp_rd_stop_enable_pressure_19 : 1;
        unsigned int reserved_1 : 5;
        unsigned int vp_rd_stop_ok_19 : 1;
        unsigned int vp_rd_stop_19 : 1;
        unsigned int reserved_2 : 5;
        unsigned int vprd_prefetch_bypass_19 : 1;
    } reg;
} SOC_IPP_CVDR_CVDR_IPP_VP_RD_IF_CFG_19_UNION;
#endif
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_IF_CFG_19_vp_rd_stop_enable_du_threshold_reached_19_START (16)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_IF_CFG_19_vp_rd_stop_enable_du_threshold_reached_19_END (16)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_IF_CFG_19_vp_rd_stop_enable_flux_ctrl_19_START (17)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_IF_CFG_19_vp_rd_stop_enable_flux_ctrl_19_END (17)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_IF_CFG_19_vp_rd_stop_enable_pressure_19_START (18)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_IF_CFG_19_vp_rd_stop_enable_pressure_19_END (18)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_IF_CFG_19_vp_rd_stop_ok_19_START (24)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_IF_CFG_19_vp_rd_stop_ok_19_END (24)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_IF_CFG_19_vp_rd_stop_19_START (25)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_IF_CFG_19_vp_rd_stop_19_END (25)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_IF_CFG_19_vprd_prefetch_bypass_19_START (31)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_IF_CFG_19_vprd_prefetch_bypass_19_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int vprd_access_limiter_0_19 : 4;
        unsigned int vprd_access_limiter_1_19 : 4;
        unsigned int vprd_access_limiter_2_19 : 4;
        unsigned int vprd_access_limiter_3_19 : 4;
        unsigned int reserved_0 : 8;
        unsigned int vprd_access_limiter_reload_19 : 4;
        unsigned int reserved_1 : 4;
    } reg;
} SOC_IPP_CVDR_CVDR_IPP_VP_RD_LIMITER_19_UNION;
#endif
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_LIMITER_19_vprd_access_limiter_0_19_START (0)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_LIMITER_19_vprd_access_limiter_0_19_END (3)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_LIMITER_19_vprd_access_limiter_1_19_START (4)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_LIMITER_19_vprd_access_limiter_1_19_END (7)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_LIMITER_19_vprd_access_limiter_2_19_START (8)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_LIMITER_19_vprd_access_limiter_2_19_END (11)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_LIMITER_19_vprd_access_limiter_3_19_START (12)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_LIMITER_19_vprd_access_limiter_3_19_END (15)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_LIMITER_19_vprd_access_limiter_reload_19_START (24)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_LIMITER_19_vprd_access_limiter_reload_19_END (27)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int vp_rd_debug_19 : 32;
    } reg;
} SOC_IPP_CVDR_CVDR_IPP_VP_RD_DEBUG_19_UNION;
#endif
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_DEBUG_19_vp_rd_debug_19_START (0)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_DEBUG_19_vp_rd_debug_19_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int vprd_pixel_format_20 : 5;
        unsigned int vprd_pixel_expansion_20 : 1;
        unsigned int vprd_allocated_du_20 : 5;
        unsigned int reserved : 2;
        unsigned int vprd_last_page_20 : 19;
    } reg;
} SOC_IPP_CVDR_CVDR_IPP_VP_RD_CFG_20_UNION;
#endif
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_CFG_20_vprd_pixel_format_20_START (0)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_CFG_20_vprd_pixel_format_20_END (4)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_CFG_20_vprd_pixel_expansion_20_START (5)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_CFG_20_vprd_pixel_expansion_20_END (5)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_CFG_20_vprd_allocated_du_20_START (6)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_CFG_20_vprd_allocated_du_20_END (10)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_CFG_20_vprd_last_page_20_START (13)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_CFG_20_vprd_last_page_20_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int vprd_line_size_20 : 15;
        unsigned int reserved_0 : 1;
        unsigned int vprd_horizontal_blanking_20 : 8;
        unsigned int reserved_1 : 8;
    } reg;
} SOC_IPP_CVDR_CVDR_IPP_VP_RD_LWG_20_UNION;
#endif
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_LWG_20_vprd_line_size_20_START (0)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_LWG_20_vprd_line_size_20_END (14)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_LWG_20_vprd_horizontal_blanking_20_START (16)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_LWG_20_vprd_horizontal_blanking_20_END (23)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int vprd_frame_size_20 : 15;
        unsigned int reserved_0 : 1;
        unsigned int vprd_vertical_blanking_20 : 8;
        unsigned int reserved_1 : 8;
    } reg;
} SOC_IPP_CVDR_CVDR_IPP_VP_RD_FHG_20_UNION;
#endif
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_FHG_20_vprd_frame_size_20_START (0)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_FHG_20_vprd_frame_size_20_END (14)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_FHG_20_vprd_vertical_blanking_20_START (16)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_FHG_20_vprd_vertical_blanking_20_END (23)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int reserved : 2;
        unsigned int vprd_axi_frame_start_20 : 30;
    } reg;
} SOC_IPP_CVDR_CVDR_IPP_VP_RD_AXI_FS_20_UNION;
#endif
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_AXI_FS_20_vprd_axi_frame_start_20_START (2)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_AXI_FS_20_vprd_axi_frame_start_20_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int vprd_line_stride_20 : 14;
        unsigned int reserved : 4;
        unsigned int vprd_line_wrap_20 : 14;
    } reg;
} SOC_IPP_CVDR_CVDR_IPP_VP_RD_AXI_LINE_20_UNION;
#endif
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_AXI_LINE_20_vprd_line_stride_20_START (0)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_AXI_LINE_20_vprd_line_stride_20_END (13)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_AXI_LINE_20_vprd_line_wrap_20_START (18)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_AXI_LINE_20_vprd_line_wrap_20_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int reserved_0 : 16;
        unsigned int vp_rd_stop_enable_du_threshold_reached_20 : 1;
        unsigned int vp_rd_stop_enable_flux_ctrl_20 : 1;
        unsigned int vp_rd_stop_enable_pressure_20 : 1;
        unsigned int reserved_1 : 5;
        unsigned int vp_rd_stop_ok_20 : 1;
        unsigned int vp_rd_stop_20 : 1;
        unsigned int reserved_2 : 5;
        unsigned int vprd_prefetch_bypass_20 : 1;
    } reg;
} SOC_IPP_CVDR_CVDR_IPP_VP_RD_IF_CFG_20_UNION;
#endif
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_IF_CFG_20_vp_rd_stop_enable_du_threshold_reached_20_START (16)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_IF_CFG_20_vp_rd_stop_enable_du_threshold_reached_20_END (16)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_IF_CFG_20_vp_rd_stop_enable_flux_ctrl_20_START (17)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_IF_CFG_20_vp_rd_stop_enable_flux_ctrl_20_END (17)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_IF_CFG_20_vp_rd_stop_enable_pressure_20_START (18)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_IF_CFG_20_vp_rd_stop_enable_pressure_20_END (18)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_IF_CFG_20_vp_rd_stop_ok_20_START (24)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_IF_CFG_20_vp_rd_stop_ok_20_END (24)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_IF_CFG_20_vp_rd_stop_20_START (25)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_IF_CFG_20_vp_rd_stop_20_END (25)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_IF_CFG_20_vprd_prefetch_bypass_20_START (31)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_IF_CFG_20_vprd_prefetch_bypass_20_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int vprd_access_limiter_0_20 : 4;
        unsigned int vprd_access_limiter_1_20 : 4;
        unsigned int vprd_access_limiter_2_20 : 4;
        unsigned int vprd_access_limiter_3_20 : 4;
        unsigned int reserved_0 : 8;
        unsigned int vprd_access_limiter_reload_20 : 4;
        unsigned int reserved_1 : 4;
    } reg;
} SOC_IPP_CVDR_CVDR_IPP_VP_RD_LIMITER_20_UNION;
#endif
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_LIMITER_20_vprd_access_limiter_0_20_START (0)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_LIMITER_20_vprd_access_limiter_0_20_END (3)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_LIMITER_20_vprd_access_limiter_1_20_START (4)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_LIMITER_20_vprd_access_limiter_1_20_END (7)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_LIMITER_20_vprd_access_limiter_2_20_START (8)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_LIMITER_20_vprd_access_limiter_2_20_END (11)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_LIMITER_20_vprd_access_limiter_3_20_START (12)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_LIMITER_20_vprd_access_limiter_3_20_END (15)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_LIMITER_20_vprd_access_limiter_reload_20_START (24)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_LIMITER_20_vprd_access_limiter_reload_20_END (27)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int vp_rd_debug_20 : 32;
    } reg;
} SOC_IPP_CVDR_CVDR_IPP_VP_RD_DEBUG_20_UNION;
#endif
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_DEBUG_20_vp_rd_debug_20_START (0)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_DEBUG_20_vp_rd_debug_20_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int vprd_pixel_format_21 : 5;
        unsigned int vprd_pixel_expansion_21 : 1;
        unsigned int vprd_allocated_du_21 : 5;
        unsigned int reserved : 2;
        unsigned int vprd_last_page_21 : 19;
    } reg;
} SOC_IPP_CVDR_CVDR_IPP_VP_RD_CFG_21_UNION;
#endif
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_CFG_21_vprd_pixel_format_21_START (0)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_CFG_21_vprd_pixel_format_21_END (4)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_CFG_21_vprd_pixel_expansion_21_START (5)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_CFG_21_vprd_pixel_expansion_21_END (5)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_CFG_21_vprd_allocated_du_21_START (6)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_CFG_21_vprd_allocated_du_21_END (10)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_CFG_21_vprd_last_page_21_START (13)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_CFG_21_vprd_last_page_21_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int vprd_line_size_21 : 15;
        unsigned int reserved_0 : 1;
        unsigned int vprd_horizontal_blanking_21 : 8;
        unsigned int reserved_1 : 8;
    } reg;
} SOC_IPP_CVDR_CVDR_IPP_VP_RD_LWG_21_UNION;
#endif
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_LWG_21_vprd_line_size_21_START (0)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_LWG_21_vprd_line_size_21_END (14)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_LWG_21_vprd_horizontal_blanking_21_START (16)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_LWG_21_vprd_horizontal_blanking_21_END (23)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int vprd_frame_size_21 : 15;
        unsigned int reserved_0 : 1;
        unsigned int vprd_vertical_blanking_21 : 8;
        unsigned int reserved_1 : 8;
    } reg;
} SOC_IPP_CVDR_CVDR_IPP_VP_RD_FHG_21_UNION;
#endif
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_FHG_21_vprd_frame_size_21_START (0)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_FHG_21_vprd_frame_size_21_END (14)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_FHG_21_vprd_vertical_blanking_21_START (16)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_FHG_21_vprd_vertical_blanking_21_END (23)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int reserved : 2;
        unsigned int vprd_axi_frame_start_21 : 30;
    } reg;
} SOC_IPP_CVDR_CVDR_IPP_VP_RD_AXI_FS_21_UNION;
#endif
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_AXI_FS_21_vprd_axi_frame_start_21_START (2)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_AXI_FS_21_vprd_axi_frame_start_21_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int vprd_line_stride_21 : 14;
        unsigned int reserved : 4;
        unsigned int vprd_line_wrap_21 : 14;
    } reg;
} SOC_IPP_CVDR_CVDR_IPP_VP_RD_AXI_LINE_21_UNION;
#endif
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_AXI_LINE_21_vprd_line_stride_21_START (0)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_AXI_LINE_21_vprd_line_stride_21_END (13)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_AXI_LINE_21_vprd_line_wrap_21_START (18)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_AXI_LINE_21_vprd_line_wrap_21_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int reserved_0 : 16;
        unsigned int vp_rd_stop_enable_du_threshold_reached_21 : 1;
        unsigned int vp_rd_stop_enable_flux_ctrl_21 : 1;
        unsigned int vp_rd_stop_enable_pressure_21 : 1;
        unsigned int reserved_1 : 5;
        unsigned int vp_rd_stop_ok_21 : 1;
        unsigned int vp_rd_stop_21 : 1;
        unsigned int reserved_2 : 5;
        unsigned int vprd_prefetch_bypass_21 : 1;
    } reg;
} SOC_IPP_CVDR_CVDR_IPP_VP_RD_IF_CFG_21_UNION;
#endif
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_IF_CFG_21_vp_rd_stop_enable_du_threshold_reached_21_START (16)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_IF_CFG_21_vp_rd_stop_enable_du_threshold_reached_21_END (16)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_IF_CFG_21_vp_rd_stop_enable_flux_ctrl_21_START (17)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_IF_CFG_21_vp_rd_stop_enable_flux_ctrl_21_END (17)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_IF_CFG_21_vp_rd_stop_enable_pressure_21_START (18)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_IF_CFG_21_vp_rd_stop_enable_pressure_21_END (18)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_IF_CFG_21_vp_rd_stop_ok_21_START (24)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_IF_CFG_21_vp_rd_stop_ok_21_END (24)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_IF_CFG_21_vp_rd_stop_21_START (25)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_IF_CFG_21_vp_rd_stop_21_END (25)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_IF_CFG_21_vprd_prefetch_bypass_21_START (31)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_IF_CFG_21_vprd_prefetch_bypass_21_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int vprd_access_limiter_0_21 : 4;
        unsigned int vprd_access_limiter_1_21 : 4;
        unsigned int vprd_access_limiter_2_21 : 4;
        unsigned int vprd_access_limiter_3_21 : 4;
        unsigned int reserved_0 : 8;
        unsigned int vprd_access_limiter_reload_21 : 4;
        unsigned int reserved_1 : 4;
    } reg;
} SOC_IPP_CVDR_CVDR_IPP_VP_RD_LIMITER_21_UNION;
#endif
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_LIMITER_21_vprd_access_limiter_0_21_START (0)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_LIMITER_21_vprd_access_limiter_0_21_END (3)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_LIMITER_21_vprd_access_limiter_1_21_START (4)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_LIMITER_21_vprd_access_limiter_1_21_END (7)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_LIMITER_21_vprd_access_limiter_2_21_START (8)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_LIMITER_21_vprd_access_limiter_2_21_END (11)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_LIMITER_21_vprd_access_limiter_3_21_START (12)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_LIMITER_21_vprd_access_limiter_3_21_END (15)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_LIMITER_21_vprd_access_limiter_reload_21_START (24)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_LIMITER_21_vprd_access_limiter_reload_21_END (27)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int vp_rd_debug_21 : 32;
    } reg;
} SOC_IPP_CVDR_CVDR_IPP_VP_RD_DEBUG_21_UNION;
#endif
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_DEBUG_21_vp_rd_debug_21_START (0)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_DEBUG_21_vp_rd_debug_21_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int vprd_pixel_format_22 : 5;
        unsigned int vprd_pixel_expansion_22 : 1;
        unsigned int vprd_allocated_du_22 : 5;
        unsigned int reserved : 2;
        unsigned int vprd_last_page_22 : 19;
    } reg;
} SOC_IPP_CVDR_CVDR_IPP_VP_RD_CFG_22_UNION;
#endif
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_CFG_22_vprd_pixel_format_22_START (0)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_CFG_22_vprd_pixel_format_22_END (4)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_CFG_22_vprd_pixel_expansion_22_START (5)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_CFG_22_vprd_pixel_expansion_22_END (5)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_CFG_22_vprd_allocated_du_22_START (6)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_CFG_22_vprd_allocated_du_22_END (10)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_CFG_22_vprd_last_page_22_START (13)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_CFG_22_vprd_last_page_22_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int vprd_line_size_22 : 15;
        unsigned int reserved_0 : 1;
        unsigned int vprd_horizontal_blanking_22 : 8;
        unsigned int reserved_1 : 8;
    } reg;
} SOC_IPP_CVDR_CVDR_IPP_VP_RD_LWG_22_UNION;
#endif
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_LWG_22_vprd_line_size_22_START (0)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_LWG_22_vprd_line_size_22_END (14)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_LWG_22_vprd_horizontal_blanking_22_START (16)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_LWG_22_vprd_horizontal_blanking_22_END (23)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int vprd_frame_size_22 : 15;
        unsigned int reserved_0 : 1;
        unsigned int vprd_vertical_blanking_22 : 8;
        unsigned int reserved_1 : 8;
    } reg;
} SOC_IPP_CVDR_CVDR_IPP_VP_RD_FHG_22_UNION;
#endif
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_FHG_22_vprd_frame_size_22_START (0)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_FHG_22_vprd_frame_size_22_END (14)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_FHG_22_vprd_vertical_blanking_22_START (16)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_FHG_22_vprd_vertical_blanking_22_END (23)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int reserved : 2;
        unsigned int vprd_axi_frame_start_22 : 30;
    } reg;
} SOC_IPP_CVDR_CVDR_IPP_VP_RD_AXI_FS_22_UNION;
#endif
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_AXI_FS_22_vprd_axi_frame_start_22_START (2)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_AXI_FS_22_vprd_axi_frame_start_22_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int vprd_line_stride_22 : 14;
        unsigned int reserved : 4;
        unsigned int vprd_line_wrap_22 : 14;
    } reg;
} SOC_IPP_CVDR_CVDR_IPP_VP_RD_AXI_LINE_22_UNION;
#endif
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_AXI_LINE_22_vprd_line_stride_22_START (0)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_AXI_LINE_22_vprd_line_stride_22_END (13)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_AXI_LINE_22_vprd_line_wrap_22_START (18)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_AXI_LINE_22_vprd_line_wrap_22_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int reserved_0 : 16;
        unsigned int vp_rd_stop_enable_du_threshold_reached_22 : 1;
        unsigned int vp_rd_stop_enable_flux_ctrl_22 : 1;
        unsigned int vp_rd_stop_enable_pressure_22 : 1;
        unsigned int reserved_1 : 5;
        unsigned int vp_rd_stop_ok_22 : 1;
        unsigned int vp_rd_stop_22 : 1;
        unsigned int reserved_2 : 5;
        unsigned int vprd_prefetch_bypass_22 : 1;
    } reg;
} SOC_IPP_CVDR_CVDR_IPP_VP_RD_IF_CFG_22_UNION;
#endif
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_IF_CFG_22_vp_rd_stop_enable_du_threshold_reached_22_START (16)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_IF_CFG_22_vp_rd_stop_enable_du_threshold_reached_22_END (16)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_IF_CFG_22_vp_rd_stop_enable_flux_ctrl_22_START (17)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_IF_CFG_22_vp_rd_stop_enable_flux_ctrl_22_END (17)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_IF_CFG_22_vp_rd_stop_enable_pressure_22_START (18)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_IF_CFG_22_vp_rd_stop_enable_pressure_22_END (18)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_IF_CFG_22_vp_rd_stop_ok_22_START (24)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_IF_CFG_22_vp_rd_stop_ok_22_END (24)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_IF_CFG_22_vp_rd_stop_22_START (25)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_IF_CFG_22_vp_rd_stop_22_END (25)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_IF_CFG_22_vprd_prefetch_bypass_22_START (31)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_IF_CFG_22_vprd_prefetch_bypass_22_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int vprd_access_limiter_0_22 : 4;
        unsigned int vprd_access_limiter_1_22 : 4;
        unsigned int vprd_access_limiter_2_22 : 4;
        unsigned int vprd_access_limiter_3_22 : 4;
        unsigned int reserved_0 : 8;
        unsigned int vprd_access_limiter_reload_22 : 4;
        unsigned int reserved_1 : 4;
    } reg;
} SOC_IPP_CVDR_CVDR_IPP_VP_RD_LIMITER_22_UNION;
#endif
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_LIMITER_22_vprd_access_limiter_0_22_START (0)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_LIMITER_22_vprd_access_limiter_0_22_END (3)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_LIMITER_22_vprd_access_limiter_1_22_START (4)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_LIMITER_22_vprd_access_limiter_1_22_END (7)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_LIMITER_22_vprd_access_limiter_2_22_START (8)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_LIMITER_22_vprd_access_limiter_2_22_END (11)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_LIMITER_22_vprd_access_limiter_3_22_START (12)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_LIMITER_22_vprd_access_limiter_3_22_END (15)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_LIMITER_22_vprd_access_limiter_reload_22_START (24)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_LIMITER_22_vprd_access_limiter_reload_22_END (27)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int vp_rd_debug_22 : 32;
    } reg;
} SOC_IPP_CVDR_CVDR_IPP_VP_RD_DEBUG_22_UNION;
#endif
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_DEBUG_22_vp_rd_debug_22_START (0)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_DEBUG_22_vp_rd_debug_22_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int vprd_pixel_format_23 : 5;
        unsigned int vprd_pixel_expansion_23 : 1;
        unsigned int vprd_allocated_du_23 : 5;
        unsigned int reserved : 2;
        unsigned int vprd_last_page_23 : 19;
    } reg;
} SOC_IPP_CVDR_CVDR_IPP_VP_RD_CFG_23_UNION;
#endif
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_CFG_23_vprd_pixel_format_23_START (0)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_CFG_23_vprd_pixel_format_23_END (4)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_CFG_23_vprd_pixel_expansion_23_START (5)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_CFG_23_vprd_pixel_expansion_23_END (5)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_CFG_23_vprd_allocated_du_23_START (6)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_CFG_23_vprd_allocated_du_23_END (10)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_CFG_23_vprd_last_page_23_START (13)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_CFG_23_vprd_last_page_23_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int vprd_line_size_23 : 15;
        unsigned int reserved_0 : 1;
        unsigned int vprd_horizontal_blanking_23 : 8;
        unsigned int reserved_1 : 8;
    } reg;
} SOC_IPP_CVDR_CVDR_IPP_VP_RD_LWG_23_UNION;
#endif
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_LWG_23_vprd_line_size_23_START (0)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_LWG_23_vprd_line_size_23_END (14)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_LWG_23_vprd_horizontal_blanking_23_START (16)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_LWG_23_vprd_horizontal_blanking_23_END (23)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int vprd_frame_size_23 : 15;
        unsigned int reserved_0 : 1;
        unsigned int vprd_vertical_blanking_23 : 8;
        unsigned int reserved_1 : 8;
    } reg;
} SOC_IPP_CVDR_CVDR_IPP_VP_RD_FHG_23_UNION;
#endif
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_FHG_23_vprd_frame_size_23_START (0)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_FHG_23_vprd_frame_size_23_END (14)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_FHG_23_vprd_vertical_blanking_23_START (16)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_FHG_23_vprd_vertical_blanking_23_END (23)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int reserved : 2;
        unsigned int vprd_axi_frame_start_23 : 30;
    } reg;
} SOC_IPP_CVDR_CVDR_IPP_VP_RD_AXI_FS_23_UNION;
#endif
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_AXI_FS_23_vprd_axi_frame_start_23_START (2)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_AXI_FS_23_vprd_axi_frame_start_23_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int vprd_line_stride_23 : 14;
        unsigned int reserved : 4;
        unsigned int vprd_line_wrap_23 : 14;
    } reg;
} SOC_IPP_CVDR_CVDR_IPP_VP_RD_AXI_LINE_23_UNION;
#endif
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_AXI_LINE_23_vprd_line_stride_23_START (0)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_AXI_LINE_23_vprd_line_stride_23_END (13)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_AXI_LINE_23_vprd_line_wrap_23_START (18)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_AXI_LINE_23_vprd_line_wrap_23_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int reserved_0 : 16;
        unsigned int vp_rd_stop_enable_du_threshold_reached_23 : 1;
        unsigned int vp_rd_stop_enable_flux_ctrl_23 : 1;
        unsigned int vp_rd_stop_enable_pressure_23 : 1;
        unsigned int reserved_1 : 5;
        unsigned int vp_rd_stop_ok_23 : 1;
        unsigned int vp_rd_stop_23 : 1;
        unsigned int reserved_2 : 5;
        unsigned int vprd_prefetch_bypass_23 : 1;
    } reg;
} SOC_IPP_CVDR_CVDR_IPP_VP_RD_IF_CFG_23_UNION;
#endif
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_IF_CFG_23_vp_rd_stop_enable_du_threshold_reached_23_START (16)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_IF_CFG_23_vp_rd_stop_enable_du_threshold_reached_23_END (16)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_IF_CFG_23_vp_rd_stop_enable_flux_ctrl_23_START (17)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_IF_CFG_23_vp_rd_stop_enable_flux_ctrl_23_END (17)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_IF_CFG_23_vp_rd_stop_enable_pressure_23_START (18)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_IF_CFG_23_vp_rd_stop_enable_pressure_23_END (18)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_IF_CFG_23_vp_rd_stop_ok_23_START (24)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_IF_CFG_23_vp_rd_stop_ok_23_END (24)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_IF_CFG_23_vp_rd_stop_23_START (25)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_IF_CFG_23_vp_rd_stop_23_END (25)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_IF_CFG_23_vprd_prefetch_bypass_23_START (31)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_IF_CFG_23_vprd_prefetch_bypass_23_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int vprd_access_limiter_0_23 : 4;
        unsigned int vprd_access_limiter_1_23 : 4;
        unsigned int vprd_access_limiter_2_23 : 4;
        unsigned int vprd_access_limiter_3_23 : 4;
        unsigned int reserved_0 : 8;
        unsigned int vprd_access_limiter_reload_23 : 4;
        unsigned int reserved_1 : 4;
    } reg;
} SOC_IPP_CVDR_CVDR_IPP_VP_RD_LIMITER_23_UNION;
#endif
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_LIMITER_23_vprd_access_limiter_0_23_START (0)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_LIMITER_23_vprd_access_limiter_0_23_END (3)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_LIMITER_23_vprd_access_limiter_1_23_START (4)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_LIMITER_23_vprd_access_limiter_1_23_END (7)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_LIMITER_23_vprd_access_limiter_2_23_START (8)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_LIMITER_23_vprd_access_limiter_2_23_END (11)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_LIMITER_23_vprd_access_limiter_3_23_START (12)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_LIMITER_23_vprd_access_limiter_3_23_END (15)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_LIMITER_23_vprd_access_limiter_reload_23_START (24)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_LIMITER_23_vprd_access_limiter_reload_23_END (27)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int vp_rd_debug_23 : 32;
    } reg;
} SOC_IPP_CVDR_CVDR_IPP_VP_RD_DEBUG_23_UNION;
#endif
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_DEBUG_23_vp_rd_debug_23_START (0)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_DEBUG_23_vp_rd_debug_23_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int vprd_pixel_format_24 : 5;
        unsigned int vprd_pixel_expansion_24 : 1;
        unsigned int vprd_allocated_du_24 : 5;
        unsigned int reserved : 2;
        unsigned int vprd_last_page_24 : 19;
    } reg;
} SOC_IPP_CVDR_CVDR_IPP_VP_RD_CFG_24_UNION;
#endif
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_CFG_24_vprd_pixel_format_24_START (0)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_CFG_24_vprd_pixel_format_24_END (4)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_CFG_24_vprd_pixel_expansion_24_START (5)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_CFG_24_vprd_pixel_expansion_24_END (5)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_CFG_24_vprd_allocated_du_24_START (6)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_CFG_24_vprd_allocated_du_24_END (10)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_CFG_24_vprd_last_page_24_START (13)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_CFG_24_vprd_last_page_24_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int vprd_line_size_24 : 15;
        unsigned int reserved_0 : 1;
        unsigned int vprd_horizontal_blanking_24 : 8;
        unsigned int reserved_1 : 8;
    } reg;
} SOC_IPP_CVDR_CVDR_IPP_VP_RD_LWG_24_UNION;
#endif
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_LWG_24_vprd_line_size_24_START (0)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_LWG_24_vprd_line_size_24_END (14)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_LWG_24_vprd_horizontal_blanking_24_START (16)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_LWG_24_vprd_horizontal_blanking_24_END (23)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int vprd_frame_size_24 : 15;
        unsigned int reserved_0 : 1;
        unsigned int vprd_vertical_blanking_24 : 8;
        unsigned int reserved_1 : 8;
    } reg;
} SOC_IPP_CVDR_CVDR_IPP_VP_RD_FHG_24_UNION;
#endif
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_FHG_24_vprd_frame_size_24_START (0)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_FHG_24_vprd_frame_size_24_END (14)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_FHG_24_vprd_vertical_blanking_24_START (16)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_FHG_24_vprd_vertical_blanking_24_END (23)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int reserved : 2;
        unsigned int vprd_axi_frame_start_24 : 30;
    } reg;
} SOC_IPP_CVDR_CVDR_IPP_VP_RD_AXI_FS_24_UNION;
#endif
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_AXI_FS_24_vprd_axi_frame_start_24_START (2)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_AXI_FS_24_vprd_axi_frame_start_24_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int vprd_line_stride_24 : 14;
        unsigned int reserved : 4;
        unsigned int vprd_line_wrap_24 : 14;
    } reg;
} SOC_IPP_CVDR_CVDR_IPP_VP_RD_AXI_LINE_24_UNION;
#endif
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_AXI_LINE_24_vprd_line_stride_24_START (0)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_AXI_LINE_24_vprd_line_stride_24_END (13)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_AXI_LINE_24_vprd_line_wrap_24_START (18)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_AXI_LINE_24_vprd_line_wrap_24_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int reserved_0 : 16;
        unsigned int vp_rd_stop_enable_du_threshold_reached_24 : 1;
        unsigned int vp_rd_stop_enable_flux_ctrl_24 : 1;
        unsigned int vp_rd_stop_enable_pressure_24 : 1;
        unsigned int reserved_1 : 5;
        unsigned int vp_rd_stop_ok_24 : 1;
        unsigned int vp_rd_stop_24 : 1;
        unsigned int reserved_2 : 5;
        unsigned int vprd_prefetch_bypass_24 : 1;
    } reg;
} SOC_IPP_CVDR_CVDR_IPP_VP_RD_IF_CFG_24_UNION;
#endif
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_IF_CFG_24_vp_rd_stop_enable_du_threshold_reached_24_START (16)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_IF_CFG_24_vp_rd_stop_enable_du_threshold_reached_24_END (16)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_IF_CFG_24_vp_rd_stop_enable_flux_ctrl_24_START (17)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_IF_CFG_24_vp_rd_stop_enable_flux_ctrl_24_END (17)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_IF_CFG_24_vp_rd_stop_enable_pressure_24_START (18)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_IF_CFG_24_vp_rd_stop_enable_pressure_24_END (18)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_IF_CFG_24_vp_rd_stop_ok_24_START (24)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_IF_CFG_24_vp_rd_stop_ok_24_END (24)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_IF_CFG_24_vp_rd_stop_24_START (25)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_IF_CFG_24_vp_rd_stop_24_END (25)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_IF_CFG_24_vprd_prefetch_bypass_24_START (31)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_IF_CFG_24_vprd_prefetch_bypass_24_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int vprd_access_limiter_0_24 : 4;
        unsigned int vprd_access_limiter_1_24 : 4;
        unsigned int vprd_access_limiter_2_24 : 4;
        unsigned int vprd_access_limiter_3_24 : 4;
        unsigned int reserved_0 : 8;
        unsigned int vprd_access_limiter_reload_24 : 4;
        unsigned int reserved_1 : 4;
    } reg;
} SOC_IPP_CVDR_CVDR_IPP_VP_RD_LIMITER_24_UNION;
#endif
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_LIMITER_24_vprd_access_limiter_0_24_START (0)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_LIMITER_24_vprd_access_limiter_0_24_END (3)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_LIMITER_24_vprd_access_limiter_1_24_START (4)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_LIMITER_24_vprd_access_limiter_1_24_END (7)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_LIMITER_24_vprd_access_limiter_2_24_START (8)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_LIMITER_24_vprd_access_limiter_2_24_END (11)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_LIMITER_24_vprd_access_limiter_3_24_START (12)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_LIMITER_24_vprd_access_limiter_3_24_END (15)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_LIMITER_24_vprd_access_limiter_reload_24_START (24)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_LIMITER_24_vprd_access_limiter_reload_24_END (27)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int vp_rd_debug_24 : 32;
    } reg;
} SOC_IPP_CVDR_CVDR_IPP_VP_RD_DEBUG_24_UNION;
#endif
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_DEBUG_24_vp_rd_debug_24_START (0)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_DEBUG_24_vp_rd_debug_24_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int vprd_pixel_format_25 : 5;
        unsigned int vprd_pixel_expansion_25 : 1;
        unsigned int vprd_allocated_du_25 : 5;
        unsigned int reserved : 2;
        unsigned int vprd_last_page_25 : 19;
    } reg;
} SOC_IPP_CVDR_CVDR_IPP_VP_RD_CFG_25_UNION;
#endif
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_CFG_25_vprd_pixel_format_25_START (0)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_CFG_25_vprd_pixel_format_25_END (4)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_CFG_25_vprd_pixel_expansion_25_START (5)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_CFG_25_vprd_pixel_expansion_25_END (5)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_CFG_25_vprd_allocated_du_25_START (6)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_CFG_25_vprd_allocated_du_25_END (10)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_CFG_25_vprd_last_page_25_START (13)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_CFG_25_vprd_last_page_25_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int vprd_line_size_25 : 15;
        unsigned int reserved_0 : 1;
        unsigned int vprd_horizontal_blanking_25 : 8;
        unsigned int reserved_1 : 8;
    } reg;
} SOC_IPP_CVDR_CVDR_IPP_VP_RD_LWG_25_UNION;
#endif
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_LWG_25_vprd_line_size_25_START (0)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_LWG_25_vprd_line_size_25_END (14)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_LWG_25_vprd_horizontal_blanking_25_START (16)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_LWG_25_vprd_horizontal_blanking_25_END (23)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int vprd_frame_size_25 : 15;
        unsigned int reserved_0 : 1;
        unsigned int vprd_vertical_blanking_25 : 8;
        unsigned int reserved_1 : 8;
    } reg;
} SOC_IPP_CVDR_CVDR_IPP_VP_RD_FHG_25_UNION;
#endif
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_FHG_25_vprd_frame_size_25_START (0)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_FHG_25_vprd_frame_size_25_END (14)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_FHG_25_vprd_vertical_blanking_25_START (16)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_FHG_25_vprd_vertical_blanking_25_END (23)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int reserved : 2;
        unsigned int vprd_axi_frame_start_25 : 30;
    } reg;
} SOC_IPP_CVDR_CVDR_IPP_VP_RD_AXI_FS_25_UNION;
#endif
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_AXI_FS_25_vprd_axi_frame_start_25_START (2)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_AXI_FS_25_vprd_axi_frame_start_25_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int vprd_line_stride_25 : 14;
        unsigned int reserved : 4;
        unsigned int vprd_line_wrap_25 : 14;
    } reg;
} SOC_IPP_CVDR_CVDR_IPP_VP_RD_AXI_LINE_25_UNION;
#endif
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_AXI_LINE_25_vprd_line_stride_25_START (0)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_AXI_LINE_25_vprd_line_stride_25_END (13)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_AXI_LINE_25_vprd_line_wrap_25_START (18)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_AXI_LINE_25_vprd_line_wrap_25_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int reserved_0 : 16;
        unsigned int vp_rd_stop_enable_du_threshold_reached_25 : 1;
        unsigned int vp_rd_stop_enable_flux_ctrl_25 : 1;
        unsigned int vp_rd_stop_enable_pressure_25 : 1;
        unsigned int reserved_1 : 5;
        unsigned int vp_rd_stop_ok_25 : 1;
        unsigned int vp_rd_stop_25 : 1;
        unsigned int reserved_2 : 5;
        unsigned int vprd_prefetch_bypass_25 : 1;
    } reg;
} SOC_IPP_CVDR_CVDR_IPP_VP_RD_IF_CFG_25_UNION;
#endif
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_IF_CFG_25_vp_rd_stop_enable_du_threshold_reached_25_START (16)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_IF_CFG_25_vp_rd_stop_enable_du_threshold_reached_25_END (16)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_IF_CFG_25_vp_rd_stop_enable_flux_ctrl_25_START (17)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_IF_CFG_25_vp_rd_stop_enable_flux_ctrl_25_END (17)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_IF_CFG_25_vp_rd_stop_enable_pressure_25_START (18)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_IF_CFG_25_vp_rd_stop_enable_pressure_25_END (18)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_IF_CFG_25_vp_rd_stop_ok_25_START (24)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_IF_CFG_25_vp_rd_stop_ok_25_END (24)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_IF_CFG_25_vp_rd_stop_25_START (25)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_IF_CFG_25_vp_rd_stop_25_END (25)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_IF_CFG_25_vprd_prefetch_bypass_25_START (31)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_IF_CFG_25_vprd_prefetch_bypass_25_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int vprd_access_limiter_0_25 : 4;
        unsigned int vprd_access_limiter_1_25 : 4;
        unsigned int vprd_access_limiter_2_25 : 4;
        unsigned int vprd_access_limiter_3_25 : 4;
        unsigned int reserved_0 : 8;
        unsigned int vprd_access_limiter_reload_25 : 4;
        unsigned int reserved_1 : 4;
    } reg;
} SOC_IPP_CVDR_CVDR_IPP_VP_RD_LIMITER_25_UNION;
#endif
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_LIMITER_25_vprd_access_limiter_0_25_START (0)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_LIMITER_25_vprd_access_limiter_0_25_END (3)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_LIMITER_25_vprd_access_limiter_1_25_START (4)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_LIMITER_25_vprd_access_limiter_1_25_END (7)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_LIMITER_25_vprd_access_limiter_2_25_START (8)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_LIMITER_25_vprd_access_limiter_2_25_END (11)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_LIMITER_25_vprd_access_limiter_3_25_START (12)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_LIMITER_25_vprd_access_limiter_3_25_END (15)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_LIMITER_25_vprd_access_limiter_reload_25_START (24)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_LIMITER_25_vprd_access_limiter_reload_25_END (27)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int vp_rd_debug_25 : 32;
    } reg;
} SOC_IPP_CVDR_CVDR_IPP_VP_RD_DEBUG_25_UNION;
#endif
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_DEBUG_25_vp_rd_debug_25_START (0)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_DEBUG_25_vp_rd_debug_25_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int vprd_pixel_format_26 : 5;
        unsigned int vprd_pixel_expansion_26 : 1;
        unsigned int vprd_allocated_du_26 : 5;
        unsigned int reserved : 2;
        unsigned int vprd_last_page_26 : 19;
    } reg;
} SOC_IPP_CVDR_CVDR_IPP_VP_RD_CFG_26_UNION;
#endif
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_CFG_26_vprd_pixel_format_26_START (0)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_CFG_26_vprd_pixel_format_26_END (4)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_CFG_26_vprd_pixel_expansion_26_START (5)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_CFG_26_vprd_pixel_expansion_26_END (5)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_CFG_26_vprd_allocated_du_26_START (6)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_CFG_26_vprd_allocated_du_26_END (10)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_CFG_26_vprd_last_page_26_START (13)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_CFG_26_vprd_last_page_26_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int vprd_line_size_26 : 15;
        unsigned int reserved_0 : 1;
        unsigned int vprd_horizontal_blanking_26 : 8;
        unsigned int reserved_1 : 8;
    } reg;
} SOC_IPP_CVDR_CVDR_IPP_VP_RD_LWG_26_UNION;
#endif
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_LWG_26_vprd_line_size_26_START (0)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_LWG_26_vprd_line_size_26_END (14)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_LWG_26_vprd_horizontal_blanking_26_START (16)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_LWG_26_vprd_horizontal_blanking_26_END (23)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int vprd_frame_size_26 : 15;
        unsigned int reserved_0 : 1;
        unsigned int vprd_vertical_blanking_26 : 8;
        unsigned int reserved_1 : 8;
    } reg;
} SOC_IPP_CVDR_CVDR_IPP_VP_RD_FHG_26_UNION;
#endif
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_FHG_26_vprd_frame_size_26_START (0)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_FHG_26_vprd_frame_size_26_END (14)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_FHG_26_vprd_vertical_blanking_26_START (16)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_FHG_26_vprd_vertical_blanking_26_END (23)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int reserved : 2;
        unsigned int vprd_axi_frame_start_26 : 30;
    } reg;
} SOC_IPP_CVDR_CVDR_IPP_VP_RD_AXI_FS_26_UNION;
#endif
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_AXI_FS_26_vprd_axi_frame_start_26_START (2)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_AXI_FS_26_vprd_axi_frame_start_26_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int vprd_line_stride_26 : 14;
        unsigned int reserved : 4;
        unsigned int vprd_line_wrap_26 : 14;
    } reg;
} SOC_IPP_CVDR_CVDR_IPP_VP_RD_AXI_LINE_26_UNION;
#endif
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_AXI_LINE_26_vprd_line_stride_26_START (0)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_AXI_LINE_26_vprd_line_stride_26_END (13)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_AXI_LINE_26_vprd_line_wrap_26_START (18)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_AXI_LINE_26_vprd_line_wrap_26_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int reserved_0 : 16;
        unsigned int vp_rd_stop_enable_du_threshold_reached_26 : 1;
        unsigned int vp_rd_stop_enable_flux_ctrl_26 : 1;
        unsigned int vp_rd_stop_enable_pressure_26 : 1;
        unsigned int reserved_1 : 5;
        unsigned int vp_rd_stop_ok_26 : 1;
        unsigned int vp_rd_stop_26 : 1;
        unsigned int reserved_2 : 5;
        unsigned int vprd_prefetch_bypass_26 : 1;
    } reg;
} SOC_IPP_CVDR_CVDR_IPP_VP_RD_IF_CFG_26_UNION;
#endif
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_IF_CFG_26_vp_rd_stop_enable_du_threshold_reached_26_START (16)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_IF_CFG_26_vp_rd_stop_enable_du_threshold_reached_26_END (16)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_IF_CFG_26_vp_rd_stop_enable_flux_ctrl_26_START (17)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_IF_CFG_26_vp_rd_stop_enable_flux_ctrl_26_END (17)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_IF_CFG_26_vp_rd_stop_enable_pressure_26_START (18)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_IF_CFG_26_vp_rd_stop_enable_pressure_26_END (18)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_IF_CFG_26_vp_rd_stop_ok_26_START (24)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_IF_CFG_26_vp_rd_stop_ok_26_END (24)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_IF_CFG_26_vp_rd_stop_26_START (25)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_IF_CFG_26_vp_rd_stop_26_END (25)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_IF_CFG_26_vprd_prefetch_bypass_26_START (31)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_IF_CFG_26_vprd_prefetch_bypass_26_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int vprd_access_limiter_0_26 : 4;
        unsigned int vprd_access_limiter_1_26 : 4;
        unsigned int vprd_access_limiter_2_26 : 4;
        unsigned int vprd_access_limiter_3_26 : 4;
        unsigned int reserved_0 : 8;
        unsigned int vprd_access_limiter_reload_26 : 4;
        unsigned int reserved_1 : 4;
    } reg;
} SOC_IPP_CVDR_CVDR_IPP_VP_RD_LIMITER_26_UNION;
#endif
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_LIMITER_26_vprd_access_limiter_0_26_START (0)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_LIMITER_26_vprd_access_limiter_0_26_END (3)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_LIMITER_26_vprd_access_limiter_1_26_START (4)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_LIMITER_26_vprd_access_limiter_1_26_END (7)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_LIMITER_26_vprd_access_limiter_2_26_START (8)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_LIMITER_26_vprd_access_limiter_2_26_END (11)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_LIMITER_26_vprd_access_limiter_3_26_START (12)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_LIMITER_26_vprd_access_limiter_3_26_END (15)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_LIMITER_26_vprd_access_limiter_reload_26_START (24)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_LIMITER_26_vprd_access_limiter_reload_26_END (27)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int vp_rd_debug_26 : 32;
    } reg;
} SOC_IPP_CVDR_CVDR_IPP_VP_RD_DEBUG_26_UNION;
#endif
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_DEBUG_26_vp_rd_debug_26_START (0)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_DEBUG_26_vp_rd_debug_26_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int vprd_pixel_format_27 : 5;
        unsigned int vprd_pixel_expansion_27 : 1;
        unsigned int vprd_allocated_du_27 : 5;
        unsigned int reserved : 2;
        unsigned int vprd_last_page_27 : 19;
    } reg;
} SOC_IPP_CVDR_CVDR_IPP_VP_RD_CFG_27_UNION;
#endif
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_CFG_27_vprd_pixel_format_27_START (0)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_CFG_27_vprd_pixel_format_27_END (4)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_CFG_27_vprd_pixel_expansion_27_START (5)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_CFG_27_vprd_pixel_expansion_27_END (5)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_CFG_27_vprd_allocated_du_27_START (6)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_CFG_27_vprd_allocated_du_27_END (10)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_CFG_27_vprd_last_page_27_START (13)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_CFG_27_vprd_last_page_27_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int vprd_line_size_27 : 15;
        unsigned int reserved_0 : 1;
        unsigned int vprd_horizontal_blanking_27 : 8;
        unsigned int reserved_1 : 8;
    } reg;
} SOC_IPP_CVDR_CVDR_IPP_VP_RD_LWG_27_UNION;
#endif
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_LWG_27_vprd_line_size_27_START (0)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_LWG_27_vprd_line_size_27_END (14)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_LWG_27_vprd_horizontal_blanking_27_START (16)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_LWG_27_vprd_horizontal_blanking_27_END (23)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int vprd_frame_size_27 : 15;
        unsigned int reserved_0 : 1;
        unsigned int vprd_vertical_blanking_27 : 8;
        unsigned int reserved_1 : 8;
    } reg;
} SOC_IPP_CVDR_CVDR_IPP_VP_RD_FHG_27_UNION;
#endif
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_FHG_27_vprd_frame_size_27_START (0)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_FHG_27_vprd_frame_size_27_END (14)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_FHG_27_vprd_vertical_blanking_27_START (16)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_FHG_27_vprd_vertical_blanking_27_END (23)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int reserved : 2;
        unsigned int vprd_axi_frame_start_27 : 30;
    } reg;
} SOC_IPP_CVDR_CVDR_IPP_VP_RD_AXI_FS_27_UNION;
#endif
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_AXI_FS_27_vprd_axi_frame_start_27_START (2)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_AXI_FS_27_vprd_axi_frame_start_27_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int vprd_line_stride_27 : 14;
        unsigned int reserved : 4;
        unsigned int vprd_line_wrap_27 : 14;
    } reg;
} SOC_IPP_CVDR_CVDR_IPP_VP_RD_AXI_LINE_27_UNION;
#endif
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_AXI_LINE_27_vprd_line_stride_27_START (0)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_AXI_LINE_27_vprd_line_stride_27_END (13)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_AXI_LINE_27_vprd_line_wrap_27_START (18)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_AXI_LINE_27_vprd_line_wrap_27_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int reserved_0 : 16;
        unsigned int vp_rd_stop_enable_du_threshold_reached_27 : 1;
        unsigned int vp_rd_stop_enable_flux_ctrl_27 : 1;
        unsigned int vp_rd_stop_enable_pressure_27 : 1;
        unsigned int reserved_1 : 5;
        unsigned int vp_rd_stop_ok_27 : 1;
        unsigned int vp_rd_stop_27 : 1;
        unsigned int reserved_2 : 5;
        unsigned int vprd_prefetch_bypass_27 : 1;
    } reg;
} SOC_IPP_CVDR_CVDR_IPP_VP_RD_IF_CFG_27_UNION;
#endif
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_IF_CFG_27_vp_rd_stop_enable_du_threshold_reached_27_START (16)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_IF_CFG_27_vp_rd_stop_enable_du_threshold_reached_27_END (16)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_IF_CFG_27_vp_rd_stop_enable_flux_ctrl_27_START (17)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_IF_CFG_27_vp_rd_stop_enable_flux_ctrl_27_END (17)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_IF_CFG_27_vp_rd_stop_enable_pressure_27_START (18)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_IF_CFG_27_vp_rd_stop_enable_pressure_27_END (18)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_IF_CFG_27_vp_rd_stop_ok_27_START (24)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_IF_CFG_27_vp_rd_stop_ok_27_END (24)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_IF_CFG_27_vp_rd_stop_27_START (25)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_IF_CFG_27_vp_rd_stop_27_END (25)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_IF_CFG_27_vprd_prefetch_bypass_27_START (31)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_IF_CFG_27_vprd_prefetch_bypass_27_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int vprd_access_limiter_0_27 : 4;
        unsigned int vprd_access_limiter_1_27 : 4;
        unsigned int vprd_access_limiter_2_27 : 4;
        unsigned int vprd_access_limiter_3_27 : 4;
        unsigned int reserved_0 : 8;
        unsigned int vprd_access_limiter_reload_27 : 4;
        unsigned int reserved_1 : 4;
    } reg;
} SOC_IPP_CVDR_CVDR_IPP_VP_RD_LIMITER_27_UNION;
#endif
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_LIMITER_27_vprd_access_limiter_0_27_START (0)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_LIMITER_27_vprd_access_limiter_0_27_END (3)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_LIMITER_27_vprd_access_limiter_1_27_START (4)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_LIMITER_27_vprd_access_limiter_1_27_END (7)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_LIMITER_27_vprd_access_limiter_2_27_START (8)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_LIMITER_27_vprd_access_limiter_2_27_END (11)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_LIMITER_27_vprd_access_limiter_3_27_START (12)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_LIMITER_27_vprd_access_limiter_3_27_END (15)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_LIMITER_27_vprd_access_limiter_reload_27_START (24)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_LIMITER_27_vprd_access_limiter_reload_27_END (27)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int vp_rd_debug_27 : 32;
    } reg;
} SOC_IPP_CVDR_CVDR_IPP_VP_RD_DEBUG_27_UNION;
#endif
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_DEBUG_27_vp_rd_debug_27_START (0)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_DEBUG_27_vp_rd_debug_27_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int vprd_pixel_format_28 : 5;
        unsigned int vprd_pixel_expansion_28 : 1;
        unsigned int vprd_allocated_du_28 : 5;
        unsigned int reserved : 2;
        unsigned int vprd_last_page_28 : 19;
    } reg;
} SOC_IPP_CVDR_CVDR_IPP_VP_RD_CFG_28_UNION;
#endif
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_CFG_28_vprd_pixel_format_28_START (0)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_CFG_28_vprd_pixel_format_28_END (4)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_CFG_28_vprd_pixel_expansion_28_START (5)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_CFG_28_vprd_pixel_expansion_28_END (5)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_CFG_28_vprd_allocated_du_28_START (6)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_CFG_28_vprd_allocated_du_28_END (10)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_CFG_28_vprd_last_page_28_START (13)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_CFG_28_vprd_last_page_28_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int vprd_line_size_28 : 15;
        unsigned int reserved_0 : 1;
        unsigned int vprd_horizontal_blanking_28 : 8;
        unsigned int reserved_1 : 8;
    } reg;
} SOC_IPP_CVDR_CVDR_IPP_VP_RD_LWG_28_UNION;
#endif
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_LWG_28_vprd_line_size_28_START (0)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_LWG_28_vprd_line_size_28_END (14)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_LWG_28_vprd_horizontal_blanking_28_START (16)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_LWG_28_vprd_horizontal_blanking_28_END (23)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int vprd_frame_size_28 : 15;
        unsigned int reserved_0 : 1;
        unsigned int vprd_vertical_blanking_28 : 8;
        unsigned int reserved_1 : 8;
    } reg;
} SOC_IPP_CVDR_CVDR_IPP_VP_RD_FHG_28_UNION;
#endif
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_FHG_28_vprd_frame_size_28_START (0)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_FHG_28_vprd_frame_size_28_END (14)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_FHG_28_vprd_vertical_blanking_28_START (16)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_FHG_28_vprd_vertical_blanking_28_END (23)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int reserved : 2;
        unsigned int vprd_axi_frame_start_28 : 30;
    } reg;
} SOC_IPP_CVDR_CVDR_IPP_VP_RD_AXI_FS_28_UNION;
#endif
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_AXI_FS_28_vprd_axi_frame_start_28_START (2)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_AXI_FS_28_vprd_axi_frame_start_28_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int vprd_line_stride_28 : 14;
        unsigned int reserved : 4;
        unsigned int vprd_line_wrap_28 : 14;
    } reg;
} SOC_IPP_CVDR_CVDR_IPP_VP_RD_AXI_LINE_28_UNION;
#endif
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_AXI_LINE_28_vprd_line_stride_28_START (0)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_AXI_LINE_28_vprd_line_stride_28_END (13)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_AXI_LINE_28_vprd_line_wrap_28_START (18)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_AXI_LINE_28_vprd_line_wrap_28_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int reserved_0 : 16;
        unsigned int vp_rd_stop_enable_du_threshold_reached_28 : 1;
        unsigned int vp_rd_stop_enable_flux_ctrl_28 : 1;
        unsigned int vp_rd_stop_enable_pressure_28 : 1;
        unsigned int reserved_1 : 5;
        unsigned int vp_rd_stop_ok_28 : 1;
        unsigned int vp_rd_stop_28 : 1;
        unsigned int reserved_2 : 5;
        unsigned int vprd_prefetch_bypass_28 : 1;
    } reg;
} SOC_IPP_CVDR_CVDR_IPP_VP_RD_IF_CFG_28_UNION;
#endif
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_IF_CFG_28_vp_rd_stop_enable_du_threshold_reached_28_START (16)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_IF_CFG_28_vp_rd_stop_enable_du_threshold_reached_28_END (16)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_IF_CFG_28_vp_rd_stop_enable_flux_ctrl_28_START (17)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_IF_CFG_28_vp_rd_stop_enable_flux_ctrl_28_END (17)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_IF_CFG_28_vp_rd_stop_enable_pressure_28_START (18)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_IF_CFG_28_vp_rd_stop_enable_pressure_28_END (18)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_IF_CFG_28_vp_rd_stop_ok_28_START (24)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_IF_CFG_28_vp_rd_stop_ok_28_END (24)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_IF_CFG_28_vp_rd_stop_28_START (25)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_IF_CFG_28_vp_rd_stop_28_END (25)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_IF_CFG_28_vprd_prefetch_bypass_28_START (31)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_IF_CFG_28_vprd_prefetch_bypass_28_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int vprd_access_limiter_0_28 : 4;
        unsigned int vprd_access_limiter_1_28 : 4;
        unsigned int vprd_access_limiter_2_28 : 4;
        unsigned int vprd_access_limiter_3_28 : 4;
        unsigned int reserved_0 : 8;
        unsigned int vprd_access_limiter_reload_28 : 4;
        unsigned int reserved_1 : 4;
    } reg;
} SOC_IPP_CVDR_CVDR_IPP_VP_RD_LIMITER_28_UNION;
#endif
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_LIMITER_28_vprd_access_limiter_0_28_START (0)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_LIMITER_28_vprd_access_limiter_0_28_END (3)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_LIMITER_28_vprd_access_limiter_1_28_START (4)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_LIMITER_28_vprd_access_limiter_1_28_END (7)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_LIMITER_28_vprd_access_limiter_2_28_START (8)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_LIMITER_28_vprd_access_limiter_2_28_END (11)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_LIMITER_28_vprd_access_limiter_3_28_START (12)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_LIMITER_28_vprd_access_limiter_3_28_END (15)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_LIMITER_28_vprd_access_limiter_reload_28_START (24)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_LIMITER_28_vprd_access_limiter_reload_28_END (27)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int vp_rd_debug_28 : 32;
    } reg;
} SOC_IPP_CVDR_CVDR_IPP_VP_RD_DEBUG_28_UNION;
#endif
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_DEBUG_28_vp_rd_debug_28_START (0)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_DEBUG_28_vp_rd_debug_28_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int vprd_pixel_format_29 : 5;
        unsigned int vprd_pixel_expansion_29 : 1;
        unsigned int vprd_allocated_du_29 : 5;
        unsigned int reserved : 2;
        unsigned int vprd_last_page_29 : 19;
    } reg;
} SOC_IPP_CVDR_CVDR_IPP_VP_RD_CFG_29_UNION;
#endif
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_CFG_29_vprd_pixel_format_29_START (0)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_CFG_29_vprd_pixel_format_29_END (4)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_CFG_29_vprd_pixel_expansion_29_START (5)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_CFG_29_vprd_pixel_expansion_29_END (5)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_CFG_29_vprd_allocated_du_29_START (6)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_CFG_29_vprd_allocated_du_29_END (10)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_CFG_29_vprd_last_page_29_START (13)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_CFG_29_vprd_last_page_29_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int vprd_line_size_29 : 15;
        unsigned int reserved_0 : 1;
        unsigned int vprd_horizontal_blanking_29 : 8;
        unsigned int reserved_1 : 8;
    } reg;
} SOC_IPP_CVDR_CVDR_IPP_VP_RD_LWG_29_UNION;
#endif
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_LWG_29_vprd_line_size_29_START (0)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_LWG_29_vprd_line_size_29_END (14)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_LWG_29_vprd_horizontal_blanking_29_START (16)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_LWG_29_vprd_horizontal_blanking_29_END (23)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int vprd_frame_size_29 : 15;
        unsigned int reserved_0 : 1;
        unsigned int vprd_vertical_blanking_29 : 8;
        unsigned int reserved_1 : 8;
    } reg;
} SOC_IPP_CVDR_CVDR_IPP_VP_RD_FHG_29_UNION;
#endif
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_FHG_29_vprd_frame_size_29_START (0)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_FHG_29_vprd_frame_size_29_END (14)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_FHG_29_vprd_vertical_blanking_29_START (16)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_FHG_29_vprd_vertical_blanking_29_END (23)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int reserved : 2;
        unsigned int vprd_axi_frame_start_29 : 30;
    } reg;
} SOC_IPP_CVDR_CVDR_IPP_VP_RD_AXI_FS_29_UNION;
#endif
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_AXI_FS_29_vprd_axi_frame_start_29_START (2)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_AXI_FS_29_vprd_axi_frame_start_29_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int vprd_line_stride_29 : 14;
        unsigned int reserved : 4;
        unsigned int vprd_line_wrap_29 : 14;
    } reg;
} SOC_IPP_CVDR_CVDR_IPP_VP_RD_AXI_LINE_29_UNION;
#endif
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_AXI_LINE_29_vprd_line_stride_29_START (0)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_AXI_LINE_29_vprd_line_stride_29_END (13)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_AXI_LINE_29_vprd_line_wrap_29_START (18)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_AXI_LINE_29_vprd_line_wrap_29_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int reserved_0 : 16;
        unsigned int vp_rd_stop_enable_du_threshold_reached_29 : 1;
        unsigned int vp_rd_stop_enable_flux_ctrl_29 : 1;
        unsigned int vp_rd_stop_enable_pressure_29 : 1;
        unsigned int reserved_1 : 5;
        unsigned int vp_rd_stop_ok_29 : 1;
        unsigned int vp_rd_stop_29 : 1;
        unsigned int reserved_2 : 5;
        unsigned int vprd_prefetch_bypass_29 : 1;
    } reg;
} SOC_IPP_CVDR_CVDR_IPP_VP_RD_IF_CFG_29_UNION;
#endif
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_IF_CFG_29_vp_rd_stop_enable_du_threshold_reached_29_START (16)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_IF_CFG_29_vp_rd_stop_enable_du_threshold_reached_29_END (16)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_IF_CFG_29_vp_rd_stop_enable_flux_ctrl_29_START (17)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_IF_CFG_29_vp_rd_stop_enable_flux_ctrl_29_END (17)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_IF_CFG_29_vp_rd_stop_enable_pressure_29_START (18)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_IF_CFG_29_vp_rd_stop_enable_pressure_29_END (18)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_IF_CFG_29_vp_rd_stop_ok_29_START (24)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_IF_CFG_29_vp_rd_stop_ok_29_END (24)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_IF_CFG_29_vp_rd_stop_29_START (25)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_IF_CFG_29_vp_rd_stop_29_END (25)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_IF_CFG_29_vprd_prefetch_bypass_29_START (31)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_IF_CFG_29_vprd_prefetch_bypass_29_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int vprd_access_limiter_0_29 : 4;
        unsigned int vprd_access_limiter_1_29 : 4;
        unsigned int vprd_access_limiter_2_29 : 4;
        unsigned int vprd_access_limiter_3_29 : 4;
        unsigned int reserved_0 : 8;
        unsigned int vprd_access_limiter_reload_29 : 4;
        unsigned int reserved_1 : 4;
    } reg;
} SOC_IPP_CVDR_CVDR_IPP_VP_RD_LIMITER_29_UNION;
#endif
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_LIMITER_29_vprd_access_limiter_0_29_START (0)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_LIMITER_29_vprd_access_limiter_0_29_END (3)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_LIMITER_29_vprd_access_limiter_1_29_START (4)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_LIMITER_29_vprd_access_limiter_1_29_END (7)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_LIMITER_29_vprd_access_limiter_2_29_START (8)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_LIMITER_29_vprd_access_limiter_2_29_END (11)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_LIMITER_29_vprd_access_limiter_3_29_START (12)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_LIMITER_29_vprd_access_limiter_3_29_END (15)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_LIMITER_29_vprd_access_limiter_reload_29_START (24)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_LIMITER_29_vprd_access_limiter_reload_29_END (27)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int vp_rd_debug_29 : 32;
    } reg;
} SOC_IPP_CVDR_CVDR_IPP_VP_RD_DEBUG_29_UNION;
#endif
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_DEBUG_29_vp_rd_debug_29_START (0)
#define SOC_IPP_CVDR_CVDR_IPP_VP_RD_DEBUG_29_vp_rd_debug_29_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int reserved_0 : 16;
        unsigned int vp_wr_stop_enable_du_threshold_reached_0 : 1;
        unsigned int vp_wr_stop_enable_flux_ctrl_0 : 1;
        unsigned int vp_wr_stop_enable_pressure_0 : 1;
        unsigned int reserved_1 : 5;
        unsigned int vp_wr_stop_ok_0 : 1;
        unsigned int vp_wr_stop_0 : 1;
        unsigned int reserved_2 : 5;
        unsigned int vpwr_prefetch_bypass_0 : 1;
    } reg;
} SOC_IPP_CVDR_CVDR_IPP_VP_WR_IF_CFG_0_UNION;
#endif
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_IF_CFG_0_vp_wr_stop_enable_du_threshold_reached_0_START (16)
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_IF_CFG_0_vp_wr_stop_enable_du_threshold_reached_0_END (16)
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_IF_CFG_0_vp_wr_stop_enable_flux_ctrl_0_START (17)
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_IF_CFG_0_vp_wr_stop_enable_flux_ctrl_0_END (17)
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_IF_CFG_0_vp_wr_stop_enable_pressure_0_START (18)
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_IF_CFG_0_vp_wr_stop_enable_pressure_0_END (18)
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_IF_CFG_0_vp_wr_stop_ok_0_START (24)
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_IF_CFG_0_vp_wr_stop_ok_0_END (24)
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_IF_CFG_0_vp_wr_stop_0_START (25)
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_IF_CFG_0_vp_wr_stop_0_END (25)
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_IF_CFG_0_vpwr_prefetch_bypass_0_START (31)
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_IF_CFG_0_vpwr_prefetch_bypass_0_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int vpwr_access_limiter_0_0 : 4;
        unsigned int vpwr_access_limiter_1_0 : 4;
        unsigned int vpwr_access_limiter_2_0 : 4;
        unsigned int vpwr_access_limiter_3_0 : 4;
        unsigned int reserved_0 : 8;
        unsigned int vpwr_access_limiter_reload_0 : 4;
        unsigned int reserved_1 : 4;
    } reg;
} SOC_IPP_CVDR_CVDR_IPP_VP_WR_LIMITER_0_UNION;
#endif
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_LIMITER_0_vpwr_access_limiter_0_0_START (0)
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_LIMITER_0_vpwr_access_limiter_0_0_END (3)
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_LIMITER_0_vpwr_access_limiter_1_0_START (4)
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_LIMITER_0_vpwr_access_limiter_1_0_END (7)
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_LIMITER_0_vpwr_access_limiter_2_0_START (8)
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_LIMITER_0_vpwr_access_limiter_2_0_END (11)
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_LIMITER_0_vpwr_access_limiter_3_0_START (12)
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_LIMITER_0_vpwr_access_limiter_3_0_END (15)
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_LIMITER_0_vpwr_access_limiter_reload_0_START (24)
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_LIMITER_0_vpwr_access_limiter_reload_0_END (27)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int vpwr_line_stride_sid0_0 : 14;
        unsigned int vpwr_line_start_wstrb_sid0_0 : 4;
        unsigned int vpwr_line_wrap_sid0_0 : 14;
    } reg;
} SOC_IPP_CVDR_CVDR_IPP_VP_WR_SID_AXI_LINE_sid0_0_UNION;
#endif
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_SID_AXI_LINE_sid0_0_vpwr_line_stride_sid0_0_START (0)
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_SID_AXI_LINE_sid0_0_vpwr_line_stride_sid0_0_END (13)
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_SID_AXI_LINE_sid0_0_vpwr_line_start_wstrb_sid0_0_START (14)
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_SID_AXI_LINE_sid0_0_vpwr_line_start_wstrb_sid0_0_END (17)
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_SID_AXI_LINE_sid0_0_vpwr_line_wrap_sid0_0_START (18)
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_SID_AXI_LINE_sid0_0_vpwr_line_wrap_sid0_0_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int vp_wr_stripe_size_bytes_sid0_0 : 16;
        unsigned int reserved : 16;
    } reg;
} SOC_IPP_CVDR_CVDR_IPP_VP_WR_SID_PREFETCH_sid0_0_UNION;
#endif
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_SID_PREFETCH_sid0_0_vp_wr_stripe_size_bytes_sid0_0_START (0)
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_SID_PREFETCH_sid0_0_vp_wr_stripe_size_bytes_sid0_0_END (15)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int vpwr_pixel_format_sid0_0 : 5;
        unsigned int vpwr_pixel_expansion_sid0_0 : 1;
        unsigned int vpwr_4pf_enable_sid0_0 : 1;
        unsigned int reserved : 6;
        unsigned int vpwr_last_page_sid0_0 : 19;
    } reg;
} SOC_IPP_CVDR_CVDR_IPP_VP_WR_SID_CFG_sid0_0_UNION;
#endif
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_SID_CFG_sid0_0_vpwr_pixel_format_sid0_0_START (0)
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_SID_CFG_sid0_0_vpwr_pixel_format_sid0_0_END (4)
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_SID_CFG_sid0_0_vpwr_pixel_expansion_sid0_0_START (5)
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_SID_CFG_sid0_0_vpwr_pixel_expansion_sid0_0_END (5)
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_SID_CFG_sid0_0_vpwr_4pf_enable_sid0_0_START (6)
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_SID_CFG_sid0_0_vpwr_4pf_enable_sid0_0_END (6)
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_SID_CFG_sid0_0_vpwr_last_page_sid0_0_START (13)
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_SID_CFG_sid0_0_vpwr_last_page_sid0_0_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int reserved : 2;
        unsigned int vpwr_address_frame_start_sid0_0 : 30;
    } reg;
} SOC_IPP_CVDR_CVDR_IPP_VP_WR_SID_AXI_FS_sid0_0_UNION;
#endif
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_SID_AXI_FS_sid0_0_vpwr_address_frame_start_sid0_0_START (2)
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_SID_AXI_FS_sid0_0_vpwr_address_frame_start_sid0_0_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int vp_wr_debug_sid0_0 : 32;
    } reg;
} SOC_IPP_CVDR_CVDR_IPP_VP_WR_SID_DEBUG_sid0_0_UNION;
#endif
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_SID_DEBUG_sid0_0_vp_wr_debug_sid0_0_START (0)
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_SID_DEBUG_sid0_0_vp_wr_debug_sid0_0_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int reserved_0 : 16;
        unsigned int vp_wr_stop_enable_du_threshold_reached_3 : 1;
        unsigned int vp_wr_stop_enable_flux_ctrl_3 : 1;
        unsigned int vp_wr_stop_enable_pressure_3 : 1;
        unsigned int reserved_1 : 5;
        unsigned int vp_wr_stop_ok_3 : 1;
        unsigned int vp_wr_stop_3 : 1;
        unsigned int reserved_2 : 5;
        unsigned int vpwr_prefetch_bypass_3 : 1;
    } reg;
} SOC_IPP_CVDR_CVDR_IPP_VP_WR_IF_CFG_3_UNION;
#endif
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_IF_CFG_3_vp_wr_stop_enable_du_threshold_reached_3_START (16)
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_IF_CFG_3_vp_wr_stop_enable_du_threshold_reached_3_END (16)
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_IF_CFG_3_vp_wr_stop_enable_flux_ctrl_3_START (17)
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_IF_CFG_3_vp_wr_stop_enable_flux_ctrl_3_END (17)
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_IF_CFG_3_vp_wr_stop_enable_pressure_3_START (18)
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_IF_CFG_3_vp_wr_stop_enable_pressure_3_END (18)
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_IF_CFG_3_vp_wr_stop_ok_3_START (24)
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_IF_CFG_3_vp_wr_stop_ok_3_END (24)
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_IF_CFG_3_vp_wr_stop_3_START (25)
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_IF_CFG_3_vp_wr_stop_3_END (25)
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_IF_CFG_3_vpwr_prefetch_bypass_3_START (31)
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_IF_CFG_3_vpwr_prefetch_bypass_3_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int vpwr_access_limiter_0_3 : 4;
        unsigned int vpwr_access_limiter_1_3 : 4;
        unsigned int vpwr_access_limiter_2_3 : 4;
        unsigned int vpwr_access_limiter_3_3 : 4;
        unsigned int reserved_0 : 8;
        unsigned int vpwr_access_limiter_reload_3 : 4;
        unsigned int reserved_1 : 4;
    } reg;
} SOC_IPP_CVDR_CVDR_IPP_VP_WR_LIMITER_3_UNION;
#endif
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_LIMITER_3_vpwr_access_limiter_0_3_START (0)
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_LIMITER_3_vpwr_access_limiter_0_3_END (3)
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_LIMITER_3_vpwr_access_limiter_1_3_START (4)
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_LIMITER_3_vpwr_access_limiter_1_3_END (7)
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_LIMITER_3_vpwr_access_limiter_2_3_START (8)
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_LIMITER_3_vpwr_access_limiter_2_3_END (11)
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_LIMITER_3_vpwr_access_limiter_3_3_START (12)
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_LIMITER_3_vpwr_access_limiter_3_3_END (15)
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_LIMITER_3_vpwr_access_limiter_reload_3_START (24)
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_LIMITER_3_vpwr_access_limiter_reload_3_END (27)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int vpwr_line_stride_sid0_3 : 14;
        unsigned int vpwr_line_start_wstrb_sid0_3 : 4;
        unsigned int vpwr_line_wrap_sid0_3 : 14;
    } reg;
} SOC_IPP_CVDR_CVDR_IPP_VP_WR_SID_AXI_LINE_sid0_3_UNION;
#endif
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_SID_AXI_LINE_sid0_3_vpwr_line_stride_sid0_3_START (0)
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_SID_AXI_LINE_sid0_3_vpwr_line_stride_sid0_3_END (13)
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_SID_AXI_LINE_sid0_3_vpwr_line_start_wstrb_sid0_3_START (14)
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_SID_AXI_LINE_sid0_3_vpwr_line_start_wstrb_sid0_3_END (17)
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_SID_AXI_LINE_sid0_3_vpwr_line_wrap_sid0_3_START (18)
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_SID_AXI_LINE_sid0_3_vpwr_line_wrap_sid0_3_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int vp_wr_stripe_size_bytes_sid0_3 : 16;
        unsigned int reserved : 16;
    } reg;
} SOC_IPP_CVDR_CVDR_IPP_VP_WR_SID_PREFETCH_sid0_3_UNION;
#endif
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_SID_PREFETCH_sid0_3_vp_wr_stripe_size_bytes_sid0_3_START (0)
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_SID_PREFETCH_sid0_3_vp_wr_stripe_size_bytes_sid0_3_END (15)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int vpwr_pixel_format_sid0_3 : 5;
        unsigned int vpwr_pixel_expansion_sid0_3 : 1;
        unsigned int vpwr_4pf_enable_sid0_3 : 1;
        unsigned int reserved : 6;
        unsigned int vpwr_last_page_sid0_3 : 19;
    } reg;
} SOC_IPP_CVDR_CVDR_IPP_VP_WR_SID_CFG_sid0_3_UNION;
#endif
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_SID_CFG_sid0_3_vpwr_pixel_format_sid0_3_START (0)
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_SID_CFG_sid0_3_vpwr_pixel_format_sid0_3_END (4)
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_SID_CFG_sid0_3_vpwr_pixel_expansion_sid0_3_START (5)
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_SID_CFG_sid0_3_vpwr_pixel_expansion_sid0_3_END (5)
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_SID_CFG_sid0_3_vpwr_4pf_enable_sid0_3_START (6)
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_SID_CFG_sid0_3_vpwr_4pf_enable_sid0_3_END (6)
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_SID_CFG_sid0_3_vpwr_last_page_sid0_3_START (13)
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_SID_CFG_sid0_3_vpwr_last_page_sid0_3_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int reserved : 2;
        unsigned int vpwr_address_frame_start_sid0_3 : 30;
    } reg;
} SOC_IPP_CVDR_CVDR_IPP_VP_WR_SID_AXI_FS_sid0_3_UNION;
#endif
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_SID_AXI_FS_sid0_3_vpwr_address_frame_start_sid0_3_START (2)
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_SID_AXI_FS_sid0_3_vpwr_address_frame_start_sid0_3_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int vp_wr_debug_sid0_3 : 32;
    } reg;
} SOC_IPP_CVDR_CVDR_IPP_VP_WR_SID_DEBUG_sid0_3_UNION;
#endif
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_SID_DEBUG_sid0_3_vp_wr_debug_sid0_3_START (0)
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_SID_DEBUG_sid0_3_vp_wr_debug_sid0_3_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int reserved_0 : 16;
        unsigned int vp_wr_stop_enable_du_threshold_reached_8 : 1;
        unsigned int vp_wr_stop_enable_flux_ctrl_8 : 1;
        unsigned int vp_wr_stop_enable_pressure_8 : 1;
        unsigned int reserved_1 : 5;
        unsigned int vp_wr_stop_ok_8 : 1;
        unsigned int vp_wr_stop_8 : 1;
        unsigned int reserved_2 : 5;
        unsigned int vpwr_prefetch_bypass_8 : 1;
    } reg;
} SOC_IPP_CVDR_CVDR_IPP_VP_WR_IF_CFG_8_UNION;
#endif
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_IF_CFG_8_vp_wr_stop_enable_du_threshold_reached_8_START (16)
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_IF_CFG_8_vp_wr_stop_enable_du_threshold_reached_8_END (16)
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_IF_CFG_8_vp_wr_stop_enable_flux_ctrl_8_START (17)
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_IF_CFG_8_vp_wr_stop_enable_flux_ctrl_8_END (17)
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_IF_CFG_8_vp_wr_stop_enable_pressure_8_START (18)
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_IF_CFG_8_vp_wr_stop_enable_pressure_8_END (18)
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_IF_CFG_8_vp_wr_stop_ok_8_START (24)
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_IF_CFG_8_vp_wr_stop_ok_8_END (24)
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_IF_CFG_8_vp_wr_stop_8_START (25)
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_IF_CFG_8_vp_wr_stop_8_END (25)
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_IF_CFG_8_vpwr_prefetch_bypass_8_START (31)
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_IF_CFG_8_vpwr_prefetch_bypass_8_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int vpwr_access_limiter_0_8 : 4;
        unsigned int vpwr_access_limiter_1_8 : 4;
        unsigned int vpwr_access_limiter_2_8 : 4;
        unsigned int vpwr_access_limiter_3_8 : 4;
        unsigned int reserved_0 : 8;
        unsigned int vpwr_access_limiter_reload_8 : 4;
        unsigned int reserved_1 : 4;
    } reg;
} SOC_IPP_CVDR_CVDR_IPP_VP_WR_LIMITER_8_UNION;
#endif
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_LIMITER_8_vpwr_access_limiter_0_8_START (0)
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_LIMITER_8_vpwr_access_limiter_0_8_END (3)
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_LIMITER_8_vpwr_access_limiter_1_8_START (4)
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_LIMITER_8_vpwr_access_limiter_1_8_END (7)
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_LIMITER_8_vpwr_access_limiter_2_8_START (8)
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_LIMITER_8_vpwr_access_limiter_2_8_END (11)
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_LIMITER_8_vpwr_access_limiter_3_8_START (12)
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_LIMITER_8_vpwr_access_limiter_3_8_END (15)
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_LIMITER_8_vpwr_access_limiter_reload_8_START (24)
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_LIMITER_8_vpwr_access_limiter_reload_8_END (27)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int vpwr_line_stride_sid0_8 : 14;
        unsigned int vpwr_line_start_wstrb_sid0_8 : 4;
        unsigned int vpwr_line_wrap_sid0_8 : 14;
    } reg;
} SOC_IPP_CVDR_CVDR_IPP_VP_WR_SID_AXI_LINE_sid0_8_UNION;
#endif
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_SID_AXI_LINE_sid0_8_vpwr_line_stride_sid0_8_START (0)
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_SID_AXI_LINE_sid0_8_vpwr_line_stride_sid0_8_END (13)
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_SID_AXI_LINE_sid0_8_vpwr_line_start_wstrb_sid0_8_START (14)
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_SID_AXI_LINE_sid0_8_vpwr_line_start_wstrb_sid0_8_END (17)
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_SID_AXI_LINE_sid0_8_vpwr_line_wrap_sid0_8_START (18)
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_SID_AXI_LINE_sid0_8_vpwr_line_wrap_sid0_8_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int vp_wr_stripe_size_bytes_sid0_8 : 16;
        unsigned int reserved : 16;
    } reg;
} SOC_IPP_CVDR_CVDR_IPP_VP_WR_SID_PREFETCH_sid0_8_UNION;
#endif
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_SID_PREFETCH_sid0_8_vp_wr_stripe_size_bytes_sid0_8_START (0)
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_SID_PREFETCH_sid0_8_vp_wr_stripe_size_bytes_sid0_8_END (15)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int vpwr_pixel_format_sid0_8 : 5;
        unsigned int vpwr_pixel_expansion_sid0_8 : 1;
        unsigned int vpwr_4pf_enable_sid0_8 : 1;
        unsigned int reserved : 6;
        unsigned int vpwr_last_page_sid0_8 : 19;
    } reg;
} SOC_IPP_CVDR_CVDR_IPP_VP_WR_SID_CFG_sid0_8_UNION;
#endif
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_SID_CFG_sid0_8_vpwr_pixel_format_sid0_8_START (0)
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_SID_CFG_sid0_8_vpwr_pixel_format_sid0_8_END (4)
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_SID_CFG_sid0_8_vpwr_pixel_expansion_sid0_8_START (5)
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_SID_CFG_sid0_8_vpwr_pixel_expansion_sid0_8_END (5)
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_SID_CFG_sid0_8_vpwr_4pf_enable_sid0_8_START (6)
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_SID_CFG_sid0_8_vpwr_4pf_enable_sid0_8_END (6)
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_SID_CFG_sid0_8_vpwr_last_page_sid0_8_START (13)
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_SID_CFG_sid0_8_vpwr_last_page_sid0_8_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int reserved : 2;
        unsigned int vpwr_address_frame_start_sid0_8 : 30;
    } reg;
} SOC_IPP_CVDR_CVDR_IPP_VP_WR_SID_AXI_FS_sid0_8_UNION;
#endif
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_SID_AXI_FS_sid0_8_vpwr_address_frame_start_sid0_8_START (2)
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_SID_AXI_FS_sid0_8_vpwr_address_frame_start_sid0_8_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int vp_wr_debug_sid0_8 : 32;
    } reg;
} SOC_IPP_CVDR_CVDR_IPP_VP_WR_SID_DEBUG_sid0_8_UNION;
#endif
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_SID_DEBUG_sid0_8_vp_wr_debug_sid0_8_START (0)
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_SID_DEBUG_sid0_8_vp_wr_debug_sid0_8_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int reserved_0 : 16;
        unsigned int vp_wr_stop_enable_du_threshold_reached_9 : 1;
        unsigned int vp_wr_stop_enable_flux_ctrl_9 : 1;
        unsigned int vp_wr_stop_enable_pressure_9 : 1;
        unsigned int reserved_1 : 5;
        unsigned int vp_wr_stop_ok_9 : 1;
        unsigned int vp_wr_stop_9 : 1;
        unsigned int reserved_2 : 5;
        unsigned int vpwr_prefetch_bypass_9 : 1;
    } reg;
} SOC_IPP_CVDR_CVDR_IPP_VP_WR_IF_CFG_9_UNION;
#endif
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_IF_CFG_9_vp_wr_stop_enable_du_threshold_reached_9_START (16)
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_IF_CFG_9_vp_wr_stop_enable_du_threshold_reached_9_END (16)
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_IF_CFG_9_vp_wr_stop_enable_flux_ctrl_9_START (17)
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_IF_CFG_9_vp_wr_stop_enable_flux_ctrl_9_END (17)
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_IF_CFG_9_vp_wr_stop_enable_pressure_9_START (18)
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_IF_CFG_9_vp_wr_stop_enable_pressure_9_END (18)
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_IF_CFG_9_vp_wr_stop_ok_9_START (24)
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_IF_CFG_9_vp_wr_stop_ok_9_END (24)
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_IF_CFG_9_vp_wr_stop_9_START (25)
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_IF_CFG_9_vp_wr_stop_9_END (25)
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_IF_CFG_9_vpwr_prefetch_bypass_9_START (31)
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_IF_CFG_9_vpwr_prefetch_bypass_9_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int vpwr_access_limiter_0_9 : 4;
        unsigned int vpwr_access_limiter_1_9 : 4;
        unsigned int vpwr_access_limiter_2_9 : 4;
        unsigned int vpwr_access_limiter_3_9 : 4;
        unsigned int reserved_0 : 8;
        unsigned int vpwr_access_limiter_reload_9 : 4;
        unsigned int reserved_1 : 4;
    } reg;
} SOC_IPP_CVDR_CVDR_IPP_VP_WR_LIMITER_9_UNION;
#endif
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_LIMITER_9_vpwr_access_limiter_0_9_START (0)
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_LIMITER_9_vpwr_access_limiter_0_9_END (3)
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_LIMITER_9_vpwr_access_limiter_1_9_START (4)
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_LIMITER_9_vpwr_access_limiter_1_9_END (7)
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_LIMITER_9_vpwr_access_limiter_2_9_START (8)
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_LIMITER_9_vpwr_access_limiter_2_9_END (11)
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_LIMITER_9_vpwr_access_limiter_3_9_START (12)
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_LIMITER_9_vpwr_access_limiter_3_9_END (15)
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_LIMITER_9_vpwr_access_limiter_reload_9_START (24)
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_LIMITER_9_vpwr_access_limiter_reload_9_END (27)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int vpwr_line_stride_sid0_9 : 14;
        unsigned int vpwr_line_start_wstrb_sid0_9 : 4;
        unsigned int vpwr_line_wrap_sid0_9 : 14;
    } reg;
} SOC_IPP_CVDR_CVDR_IPP_VP_WR_SID_AXI_LINE_sid0_9_UNION;
#endif
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_SID_AXI_LINE_sid0_9_vpwr_line_stride_sid0_9_START (0)
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_SID_AXI_LINE_sid0_9_vpwr_line_stride_sid0_9_END (13)
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_SID_AXI_LINE_sid0_9_vpwr_line_start_wstrb_sid0_9_START (14)
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_SID_AXI_LINE_sid0_9_vpwr_line_start_wstrb_sid0_9_END (17)
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_SID_AXI_LINE_sid0_9_vpwr_line_wrap_sid0_9_START (18)
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_SID_AXI_LINE_sid0_9_vpwr_line_wrap_sid0_9_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int vp_wr_stripe_size_bytes_sid0_9 : 16;
        unsigned int reserved : 16;
    } reg;
} SOC_IPP_CVDR_CVDR_IPP_VP_WR_SID_PREFETCH_sid0_9_UNION;
#endif
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_SID_PREFETCH_sid0_9_vp_wr_stripe_size_bytes_sid0_9_START (0)
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_SID_PREFETCH_sid0_9_vp_wr_stripe_size_bytes_sid0_9_END (15)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int vpwr_pixel_format_sid0_9 : 5;
        unsigned int vpwr_pixel_expansion_sid0_9 : 1;
        unsigned int vpwr_4pf_enable_sid0_9 : 1;
        unsigned int reserved : 6;
        unsigned int vpwr_last_page_sid0_9 : 19;
    } reg;
} SOC_IPP_CVDR_CVDR_IPP_VP_WR_SID_CFG_sid0_9_UNION;
#endif
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_SID_CFG_sid0_9_vpwr_pixel_format_sid0_9_START (0)
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_SID_CFG_sid0_9_vpwr_pixel_format_sid0_9_END (4)
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_SID_CFG_sid0_9_vpwr_pixel_expansion_sid0_9_START (5)
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_SID_CFG_sid0_9_vpwr_pixel_expansion_sid0_9_END (5)
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_SID_CFG_sid0_9_vpwr_4pf_enable_sid0_9_START (6)
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_SID_CFG_sid0_9_vpwr_4pf_enable_sid0_9_END (6)
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_SID_CFG_sid0_9_vpwr_last_page_sid0_9_START (13)
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_SID_CFG_sid0_9_vpwr_last_page_sid0_9_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int reserved : 2;
        unsigned int vpwr_address_frame_start_sid0_9 : 30;
    } reg;
} SOC_IPP_CVDR_CVDR_IPP_VP_WR_SID_AXI_FS_sid0_9_UNION;
#endif
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_SID_AXI_FS_sid0_9_vpwr_address_frame_start_sid0_9_START (2)
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_SID_AXI_FS_sid0_9_vpwr_address_frame_start_sid0_9_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int vp_wr_debug_sid0_9 : 32;
    } reg;
} SOC_IPP_CVDR_CVDR_IPP_VP_WR_SID_DEBUG_sid0_9_UNION;
#endif
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_SID_DEBUG_sid0_9_vp_wr_debug_sid0_9_START (0)
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_SID_DEBUG_sid0_9_vp_wr_debug_sid0_9_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int reserved_0 : 16;
        unsigned int vp_wr_stop_enable_du_threshold_reached_10 : 1;
        unsigned int vp_wr_stop_enable_flux_ctrl_10 : 1;
        unsigned int vp_wr_stop_enable_pressure_10 : 1;
        unsigned int reserved_1 : 5;
        unsigned int vp_wr_stop_ok_10 : 1;
        unsigned int vp_wr_stop_10 : 1;
        unsigned int reserved_2 : 5;
        unsigned int vpwr_prefetch_bypass_10 : 1;
    } reg;
} SOC_IPP_CVDR_CVDR_IPP_VP_WR_IF_CFG_10_UNION;
#endif
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_IF_CFG_10_vp_wr_stop_enable_du_threshold_reached_10_START (16)
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_IF_CFG_10_vp_wr_stop_enable_du_threshold_reached_10_END (16)
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_IF_CFG_10_vp_wr_stop_enable_flux_ctrl_10_START (17)
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_IF_CFG_10_vp_wr_stop_enable_flux_ctrl_10_END (17)
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_IF_CFG_10_vp_wr_stop_enable_pressure_10_START (18)
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_IF_CFG_10_vp_wr_stop_enable_pressure_10_END (18)
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_IF_CFG_10_vp_wr_stop_ok_10_START (24)
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_IF_CFG_10_vp_wr_stop_ok_10_END (24)
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_IF_CFG_10_vp_wr_stop_10_START (25)
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_IF_CFG_10_vp_wr_stop_10_END (25)
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_IF_CFG_10_vpwr_prefetch_bypass_10_START (31)
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_IF_CFG_10_vpwr_prefetch_bypass_10_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int vpwr_access_limiter_0_10 : 4;
        unsigned int vpwr_access_limiter_1_10 : 4;
        unsigned int vpwr_access_limiter_2_10 : 4;
        unsigned int vpwr_access_limiter_3_10 : 4;
        unsigned int reserved_0 : 8;
        unsigned int vpwr_access_limiter_reload_10 : 4;
        unsigned int reserved_1 : 4;
    } reg;
} SOC_IPP_CVDR_CVDR_IPP_VP_WR_LIMITER_10_UNION;
#endif
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_LIMITER_10_vpwr_access_limiter_0_10_START (0)
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_LIMITER_10_vpwr_access_limiter_0_10_END (3)
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_LIMITER_10_vpwr_access_limiter_1_10_START (4)
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_LIMITER_10_vpwr_access_limiter_1_10_END (7)
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_LIMITER_10_vpwr_access_limiter_2_10_START (8)
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_LIMITER_10_vpwr_access_limiter_2_10_END (11)
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_LIMITER_10_vpwr_access_limiter_3_10_START (12)
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_LIMITER_10_vpwr_access_limiter_3_10_END (15)
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_LIMITER_10_vpwr_access_limiter_reload_10_START (24)
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_LIMITER_10_vpwr_access_limiter_reload_10_END (27)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int vpwr_line_stride_sid0_10 : 14;
        unsigned int vpwr_line_start_wstrb_sid0_10 : 4;
        unsigned int vpwr_line_wrap_sid0_10 : 14;
    } reg;
} SOC_IPP_CVDR_CVDR_IPP_VP_WR_SID_AXI_LINE_sid0_10_UNION;
#endif
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_SID_AXI_LINE_sid0_10_vpwr_line_stride_sid0_10_START (0)
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_SID_AXI_LINE_sid0_10_vpwr_line_stride_sid0_10_END (13)
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_SID_AXI_LINE_sid0_10_vpwr_line_start_wstrb_sid0_10_START (14)
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_SID_AXI_LINE_sid0_10_vpwr_line_start_wstrb_sid0_10_END (17)
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_SID_AXI_LINE_sid0_10_vpwr_line_wrap_sid0_10_START (18)
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_SID_AXI_LINE_sid0_10_vpwr_line_wrap_sid0_10_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int vp_wr_stripe_size_bytes_sid0_10 : 16;
        unsigned int reserved : 16;
    } reg;
} SOC_IPP_CVDR_CVDR_IPP_VP_WR_SID_PREFETCH_sid0_10_UNION;
#endif
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_SID_PREFETCH_sid0_10_vp_wr_stripe_size_bytes_sid0_10_START (0)
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_SID_PREFETCH_sid0_10_vp_wr_stripe_size_bytes_sid0_10_END (15)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int vpwr_pixel_format_sid0_10 : 5;
        unsigned int vpwr_pixel_expansion_sid0_10 : 1;
        unsigned int vpwr_4pf_enable_sid0_10 : 1;
        unsigned int reserved : 6;
        unsigned int vpwr_last_page_sid0_10 : 19;
    } reg;
} SOC_IPP_CVDR_CVDR_IPP_VP_WR_SID_CFG_sid0_10_UNION;
#endif
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_SID_CFG_sid0_10_vpwr_pixel_format_sid0_10_START (0)
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_SID_CFG_sid0_10_vpwr_pixel_format_sid0_10_END (4)
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_SID_CFG_sid0_10_vpwr_pixel_expansion_sid0_10_START (5)
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_SID_CFG_sid0_10_vpwr_pixel_expansion_sid0_10_END (5)
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_SID_CFG_sid0_10_vpwr_4pf_enable_sid0_10_START (6)
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_SID_CFG_sid0_10_vpwr_4pf_enable_sid0_10_END (6)
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_SID_CFG_sid0_10_vpwr_last_page_sid0_10_START (13)
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_SID_CFG_sid0_10_vpwr_last_page_sid0_10_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int reserved : 2;
        unsigned int vpwr_address_frame_start_sid0_10 : 30;
    } reg;
} SOC_IPP_CVDR_CVDR_IPP_VP_WR_SID_AXI_FS_sid0_10_UNION;
#endif
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_SID_AXI_FS_sid0_10_vpwr_address_frame_start_sid0_10_START (2)
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_SID_AXI_FS_sid0_10_vpwr_address_frame_start_sid0_10_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int vp_wr_debug_sid0_10 : 32;
    } reg;
} SOC_IPP_CVDR_CVDR_IPP_VP_WR_SID_DEBUG_sid0_10_UNION;
#endif
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_SID_DEBUG_sid0_10_vp_wr_debug_sid0_10_START (0)
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_SID_DEBUG_sid0_10_vp_wr_debug_sid0_10_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int reserved_0 : 16;
        unsigned int vp_wr_stop_enable_du_threshold_reached_11 : 1;
        unsigned int vp_wr_stop_enable_flux_ctrl_11 : 1;
        unsigned int vp_wr_stop_enable_pressure_11 : 1;
        unsigned int reserved_1 : 5;
        unsigned int vp_wr_stop_ok_11 : 1;
        unsigned int vp_wr_stop_11 : 1;
        unsigned int reserved_2 : 5;
        unsigned int vpwr_prefetch_bypass_11 : 1;
    } reg;
} SOC_IPP_CVDR_CVDR_IPP_VP_WR_IF_CFG_11_UNION;
#endif
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_IF_CFG_11_vp_wr_stop_enable_du_threshold_reached_11_START (16)
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_IF_CFG_11_vp_wr_stop_enable_du_threshold_reached_11_END (16)
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_IF_CFG_11_vp_wr_stop_enable_flux_ctrl_11_START (17)
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_IF_CFG_11_vp_wr_stop_enable_flux_ctrl_11_END (17)
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_IF_CFG_11_vp_wr_stop_enable_pressure_11_START (18)
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_IF_CFG_11_vp_wr_stop_enable_pressure_11_END (18)
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_IF_CFG_11_vp_wr_stop_ok_11_START (24)
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_IF_CFG_11_vp_wr_stop_ok_11_END (24)
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_IF_CFG_11_vp_wr_stop_11_START (25)
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_IF_CFG_11_vp_wr_stop_11_END (25)
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_IF_CFG_11_vpwr_prefetch_bypass_11_START (31)
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_IF_CFG_11_vpwr_prefetch_bypass_11_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int vpwr_access_limiter_0_11 : 4;
        unsigned int vpwr_access_limiter_1_11 : 4;
        unsigned int vpwr_access_limiter_2_11 : 4;
        unsigned int vpwr_access_limiter_3_11 : 4;
        unsigned int reserved_0 : 8;
        unsigned int vpwr_access_limiter_reload_11 : 4;
        unsigned int reserved_1 : 4;
    } reg;
} SOC_IPP_CVDR_CVDR_IPP_VP_WR_LIMITER_11_UNION;
#endif
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_LIMITER_11_vpwr_access_limiter_0_11_START (0)
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_LIMITER_11_vpwr_access_limiter_0_11_END (3)
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_LIMITER_11_vpwr_access_limiter_1_11_START (4)
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_LIMITER_11_vpwr_access_limiter_1_11_END (7)
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_LIMITER_11_vpwr_access_limiter_2_11_START (8)
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_LIMITER_11_vpwr_access_limiter_2_11_END (11)
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_LIMITER_11_vpwr_access_limiter_3_11_START (12)
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_LIMITER_11_vpwr_access_limiter_3_11_END (15)
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_LIMITER_11_vpwr_access_limiter_reload_11_START (24)
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_LIMITER_11_vpwr_access_limiter_reload_11_END (27)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int vpwr_line_stride_sid0_11 : 14;
        unsigned int vpwr_line_start_wstrb_sid0_11 : 4;
        unsigned int vpwr_line_wrap_sid0_11 : 14;
    } reg;
} SOC_IPP_CVDR_CVDR_IPP_VP_WR_SID_AXI_LINE_sid0_11_UNION;
#endif
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_SID_AXI_LINE_sid0_11_vpwr_line_stride_sid0_11_START (0)
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_SID_AXI_LINE_sid0_11_vpwr_line_stride_sid0_11_END (13)
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_SID_AXI_LINE_sid0_11_vpwr_line_start_wstrb_sid0_11_START (14)
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_SID_AXI_LINE_sid0_11_vpwr_line_start_wstrb_sid0_11_END (17)
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_SID_AXI_LINE_sid0_11_vpwr_line_wrap_sid0_11_START (18)
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_SID_AXI_LINE_sid0_11_vpwr_line_wrap_sid0_11_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int vp_wr_stripe_size_bytes_sid0_11 : 16;
        unsigned int reserved : 16;
    } reg;
} SOC_IPP_CVDR_CVDR_IPP_VP_WR_SID_PREFETCH_sid0_11_UNION;
#endif
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_SID_PREFETCH_sid0_11_vp_wr_stripe_size_bytes_sid0_11_START (0)
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_SID_PREFETCH_sid0_11_vp_wr_stripe_size_bytes_sid0_11_END (15)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int vpwr_pixel_format_sid0_11 : 5;
        unsigned int vpwr_pixel_expansion_sid0_11 : 1;
        unsigned int vpwr_4pf_enable_sid0_11 : 1;
        unsigned int reserved : 6;
        unsigned int vpwr_last_page_sid0_11 : 19;
    } reg;
} SOC_IPP_CVDR_CVDR_IPP_VP_WR_SID_CFG_sid0_11_UNION;
#endif
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_SID_CFG_sid0_11_vpwr_pixel_format_sid0_11_START (0)
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_SID_CFG_sid0_11_vpwr_pixel_format_sid0_11_END (4)
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_SID_CFG_sid0_11_vpwr_pixel_expansion_sid0_11_START (5)
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_SID_CFG_sid0_11_vpwr_pixel_expansion_sid0_11_END (5)
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_SID_CFG_sid0_11_vpwr_4pf_enable_sid0_11_START (6)
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_SID_CFG_sid0_11_vpwr_4pf_enable_sid0_11_END (6)
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_SID_CFG_sid0_11_vpwr_last_page_sid0_11_START (13)
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_SID_CFG_sid0_11_vpwr_last_page_sid0_11_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int reserved : 2;
        unsigned int vpwr_address_frame_start_sid0_11 : 30;
    } reg;
} SOC_IPP_CVDR_CVDR_IPP_VP_WR_SID_AXI_FS_sid0_11_UNION;
#endif
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_SID_AXI_FS_sid0_11_vpwr_address_frame_start_sid0_11_START (2)
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_SID_AXI_FS_sid0_11_vpwr_address_frame_start_sid0_11_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int vp_wr_debug_sid0_11 : 32;
    } reg;
} SOC_IPP_CVDR_CVDR_IPP_VP_WR_SID_DEBUG_sid0_11_UNION;
#endif
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_SID_DEBUG_sid0_11_vp_wr_debug_sid0_11_START (0)
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_SID_DEBUG_sid0_11_vp_wr_debug_sid0_11_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int reserved_0 : 16;
        unsigned int vp_wr_stop_enable_du_threshold_reached_12 : 1;
        unsigned int vp_wr_stop_enable_flux_ctrl_12 : 1;
        unsigned int vp_wr_stop_enable_pressure_12 : 1;
        unsigned int reserved_1 : 5;
        unsigned int vp_wr_stop_ok_12 : 1;
        unsigned int vp_wr_stop_12 : 1;
        unsigned int reserved_2 : 5;
        unsigned int vpwr_prefetch_bypass_12 : 1;
    } reg;
} SOC_IPP_CVDR_CVDR_IPP_VP_WR_IF_CFG_12_UNION;
#endif
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_IF_CFG_12_vp_wr_stop_enable_du_threshold_reached_12_START (16)
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_IF_CFG_12_vp_wr_stop_enable_du_threshold_reached_12_END (16)
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_IF_CFG_12_vp_wr_stop_enable_flux_ctrl_12_START (17)
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_IF_CFG_12_vp_wr_stop_enable_flux_ctrl_12_END (17)
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_IF_CFG_12_vp_wr_stop_enable_pressure_12_START (18)
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_IF_CFG_12_vp_wr_stop_enable_pressure_12_END (18)
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_IF_CFG_12_vp_wr_stop_ok_12_START (24)
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_IF_CFG_12_vp_wr_stop_ok_12_END (24)
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_IF_CFG_12_vp_wr_stop_12_START (25)
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_IF_CFG_12_vp_wr_stop_12_END (25)
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_IF_CFG_12_vpwr_prefetch_bypass_12_START (31)
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_IF_CFG_12_vpwr_prefetch_bypass_12_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int vpwr_access_limiter_0_12 : 4;
        unsigned int vpwr_access_limiter_1_12 : 4;
        unsigned int vpwr_access_limiter_2_12 : 4;
        unsigned int vpwr_access_limiter_3_12 : 4;
        unsigned int reserved_0 : 8;
        unsigned int vpwr_access_limiter_reload_12 : 4;
        unsigned int reserved_1 : 4;
    } reg;
} SOC_IPP_CVDR_CVDR_IPP_VP_WR_LIMITER_12_UNION;
#endif
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_LIMITER_12_vpwr_access_limiter_0_12_START (0)
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_LIMITER_12_vpwr_access_limiter_0_12_END (3)
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_LIMITER_12_vpwr_access_limiter_1_12_START (4)
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_LIMITER_12_vpwr_access_limiter_1_12_END (7)
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_LIMITER_12_vpwr_access_limiter_2_12_START (8)
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_LIMITER_12_vpwr_access_limiter_2_12_END (11)
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_LIMITER_12_vpwr_access_limiter_3_12_START (12)
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_LIMITER_12_vpwr_access_limiter_3_12_END (15)
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_LIMITER_12_vpwr_access_limiter_reload_12_START (24)
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_LIMITER_12_vpwr_access_limiter_reload_12_END (27)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int vpwr_line_stride_sid0_12 : 14;
        unsigned int vpwr_line_start_wstrb_sid0_12 : 4;
        unsigned int vpwr_line_wrap_sid0_12 : 14;
    } reg;
} SOC_IPP_CVDR_CVDR_IPP_VP_WR_SID_AXI_LINE_sid0_12_UNION;
#endif
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_SID_AXI_LINE_sid0_12_vpwr_line_stride_sid0_12_START (0)
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_SID_AXI_LINE_sid0_12_vpwr_line_stride_sid0_12_END (13)
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_SID_AXI_LINE_sid0_12_vpwr_line_start_wstrb_sid0_12_START (14)
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_SID_AXI_LINE_sid0_12_vpwr_line_start_wstrb_sid0_12_END (17)
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_SID_AXI_LINE_sid0_12_vpwr_line_wrap_sid0_12_START (18)
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_SID_AXI_LINE_sid0_12_vpwr_line_wrap_sid0_12_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int vp_wr_stripe_size_bytes_sid0_12 : 16;
        unsigned int reserved : 16;
    } reg;
} SOC_IPP_CVDR_CVDR_IPP_VP_WR_SID_PREFETCH_sid0_12_UNION;
#endif
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_SID_PREFETCH_sid0_12_vp_wr_stripe_size_bytes_sid0_12_START (0)
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_SID_PREFETCH_sid0_12_vp_wr_stripe_size_bytes_sid0_12_END (15)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int vpwr_pixel_format_sid0_12 : 5;
        unsigned int vpwr_pixel_expansion_sid0_12 : 1;
        unsigned int vpwr_4pf_enable_sid0_12 : 1;
        unsigned int reserved : 6;
        unsigned int vpwr_last_page_sid0_12 : 19;
    } reg;
} SOC_IPP_CVDR_CVDR_IPP_VP_WR_SID_CFG_sid0_12_UNION;
#endif
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_SID_CFG_sid0_12_vpwr_pixel_format_sid0_12_START (0)
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_SID_CFG_sid0_12_vpwr_pixel_format_sid0_12_END (4)
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_SID_CFG_sid0_12_vpwr_pixel_expansion_sid0_12_START (5)
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_SID_CFG_sid0_12_vpwr_pixel_expansion_sid0_12_END (5)
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_SID_CFG_sid0_12_vpwr_4pf_enable_sid0_12_START (6)
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_SID_CFG_sid0_12_vpwr_4pf_enable_sid0_12_END (6)
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_SID_CFG_sid0_12_vpwr_last_page_sid0_12_START (13)
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_SID_CFG_sid0_12_vpwr_last_page_sid0_12_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int reserved : 2;
        unsigned int vpwr_address_frame_start_sid0_12 : 30;
    } reg;
} SOC_IPP_CVDR_CVDR_IPP_VP_WR_SID_AXI_FS_sid0_12_UNION;
#endif
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_SID_AXI_FS_sid0_12_vpwr_address_frame_start_sid0_12_START (2)
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_SID_AXI_FS_sid0_12_vpwr_address_frame_start_sid0_12_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int vp_wr_debug_sid0_12 : 32;
    } reg;
} SOC_IPP_CVDR_CVDR_IPP_VP_WR_SID_DEBUG_sid0_12_UNION;
#endif
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_SID_DEBUG_sid0_12_vp_wr_debug_sid0_12_START (0)
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_SID_DEBUG_sid0_12_vp_wr_debug_sid0_12_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int reserved_0 : 16;
        unsigned int vp_wr_stop_enable_du_threshold_reached_13 : 1;
        unsigned int vp_wr_stop_enable_flux_ctrl_13 : 1;
        unsigned int vp_wr_stop_enable_pressure_13 : 1;
        unsigned int reserved_1 : 5;
        unsigned int vp_wr_stop_ok_13 : 1;
        unsigned int vp_wr_stop_13 : 1;
        unsigned int reserved_2 : 5;
        unsigned int vpwr_prefetch_bypass_13 : 1;
    } reg;
} SOC_IPP_CVDR_CVDR_IPP_VP_WR_IF_CFG_13_UNION;
#endif
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_IF_CFG_13_vp_wr_stop_enable_du_threshold_reached_13_START (16)
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_IF_CFG_13_vp_wr_stop_enable_du_threshold_reached_13_END (16)
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_IF_CFG_13_vp_wr_stop_enable_flux_ctrl_13_START (17)
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_IF_CFG_13_vp_wr_stop_enable_flux_ctrl_13_END (17)
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_IF_CFG_13_vp_wr_stop_enable_pressure_13_START (18)
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_IF_CFG_13_vp_wr_stop_enable_pressure_13_END (18)
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_IF_CFG_13_vp_wr_stop_ok_13_START (24)
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_IF_CFG_13_vp_wr_stop_ok_13_END (24)
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_IF_CFG_13_vp_wr_stop_13_START (25)
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_IF_CFG_13_vp_wr_stop_13_END (25)
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_IF_CFG_13_vpwr_prefetch_bypass_13_START (31)
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_IF_CFG_13_vpwr_prefetch_bypass_13_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int vpwr_access_limiter_0_13 : 4;
        unsigned int vpwr_access_limiter_1_13 : 4;
        unsigned int vpwr_access_limiter_2_13 : 4;
        unsigned int vpwr_access_limiter_3_13 : 4;
        unsigned int reserved_0 : 8;
        unsigned int vpwr_access_limiter_reload_13 : 4;
        unsigned int reserved_1 : 4;
    } reg;
} SOC_IPP_CVDR_CVDR_IPP_VP_WR_LIMITER_13_UNION;
#endif
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_LIMITER_13_vpwr_access_limiter_0_13_START (0)
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_LIMITER_13_vpwr_access_limiter_0_13_END (3)
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_LIMITER_13_vpwr_access_limiter_1_13_START (4)
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_LIMITER_13_vpwr_access_limiter_1_13_END (7)
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_LIMITER_13_vpwr_access_limiter_2_13_START (8)
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_LIMITER_13_vpwr_access_limiter_2_13_END (11)
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_LIMITER_13_vpwr_access_limiter_3_13_START (12)
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_LIMITER_13_vpwr_access_limiter_3_13_END (15)
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_LIMITER_13_vpwr_access_limiter_reload_13_START (24)
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_LIMITER_13_vpwr_access_limiter_reload_13_END (27)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int vpwr_line_stride_sid0_13 : 14;
        unsigned int vpwr_line_start_wstrb_sid0_13 : 4;
        unsigned int vpwr_line_wrap_sid0_13 : 14;
    } reg;
} SOC_IPP_CVDR_CVDR_IPP_VP_WR_SID_AXI_LINE_sid0_13_UNION;
#endif
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_SID_AXI_LINE_sid0_13_vpwr_line_stride_sid0_13_START (0)
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_SID_AXI_LINE_sid0_13_vpwr_line_stride_sid0_13_END (13)
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_SID_AXI_LINE_sid0_13_vpwr_line_start_wstrb_sid0_13_START (14)
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_SID_AXI_LINE_sid0_13_vpwr_line_start_wstrb_sid0_13_END (17)
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_SID_AXI_LINE_sid0_13_vpwr_line_wrap_sid0_13_START (18)
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_SID_AXI_LINE_sid0_13_vpwr_line_wrap_sid0_13_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int vp_wr_stripe_size_bytes_sid0_13 : 16;
        unsigned int reserved : 16;
    } reg;
} SOC_IPP_CVDR_CVDR_IPP_VP_WR_SID_PREFETCH_sid0_13_UNION;
#endif
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_SID_PREFETCH_sid0_13_vp_wr_stripe_size_bytes_sid0_13_START (0)
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_SID_PREFETCH_sid0_13_vp_wr_stripe_size_bytes_sid0_13_END (15)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int vpwr_pixel_format_sid0_13 : 5;
        unsigned int vpwr_pixel_expansion_sid0_13 : 1;
        unsigned int vpwr_4pf_enable_sid0_13 : 1;
        unsigned int reserved : 6;
        unsigned int vpwr_last_page_sid0_13 : 19;
    } reg;
} SOC_IPP_CVDR_CVDR_IPP_VP_WR_SID_CFG_sid0_13_UNION;
#endif
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_SID_CFG_sid0_13_vpwr_pixel_format_sid0_13_START (0)
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_SID_CFG_sid0_13_vpwr_pixel_format_sid0_13_END (4)
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_SID_CFG_sid0_13_vpwr_pixel_expansion_sid0_13_START (5)
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_SID_CFG_sid0_13_vpwr_pixel_expansion_sid0_13_END (5)
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_SID_CFG_sid0_13_vpwr_4pf_enable_sid0_13_START (6)
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_SID_CFG_sid0_13_vpwr_4pf_enable_sid0_13_END (6)
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_SID_CFG_sid0_13_vpwr_last_page_sid0_13_START (13)
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_SID_CFG_sid0_13_vpwr_last_page_sid0_13_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int reserved : 2;
        unsigned int vpwr_address_frame_start_sid0_13 : 30;
    } reg;
} SOC_IPP_CVDR_CVDR_IPP_VP_WR_SID_AXI_FS_sid0_13_UNION;
#endif
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_SID_AXI_FS_sid0_13_vpwr_address_frame_start_sid0_13_START (2)
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_SID_AXI_FS_sid0_13_vpwr_address_frame_start_sid0_13_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int vp_wr_debug_sid0_13 : 32;
    } reg;
} SOC_IPP_CVDR_CVDR_IPP_VP_WR_SID_DEBUG_sid0_13_UNION;
#endif
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_SID_DEBUG_sid0_13_vp_wr_debug_sid0_13_START (0)
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_SID_DEBUG_sid0_13_vp_wr_debug_sid0_13_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int reserved_0 : 16;
        unsigned int vp_wr_stop_enable_du_threshold_reached_14 : 1;
        unsigned int vp_wr_stop_enable_flux_ctrl_14 : 1;
        unsigned int vp_wr_stop_enable_pressure_14 : 1;
        unsigned int reserved_1 : 5;
        unsigned int vp_wr_stop_ok_14 : 1;
        unsigned int vp_wr_stop_14 : 1;
        unsigned int reserved_2 : 5;
        unsigned int vpwr_prefetch_bypass_14 : 1;
    } reg;
} SOC_IPP_CVDR_CVDR_IPP_VP_WR_IF_CFG_14_UNION;
#endif
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_IF_CFG_14_vp_wr_stop_enable_du_threshold_reached_14_START (16)
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_IF_CFG_14_vp_wr_stop_enable_du_threshold_reached_14_END (16)
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_IF_CFG_14_vp_wr_stop_enable_flux_ctrl_14_START (17)
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_IF_CFG_14_vp_wr_stop_enable_flux_ctrl_14_END (17)
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_IF_CFG_14_vp_wr_stop_enable_pressure_14_START (18)
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_IF_CFG_14_vp_wr_stop_enable_pressure_14_END (18)
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_IF_CFG_14_vp_wr_stop_ok_14_START (24)
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_IF_CFG_14_vp_wr_stop_ok_14_END (24)
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_IF_CFG_14_vp_wr_stop_14_START (25)
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_IF_CFG_14_vp_wr_stop_14_END (25)
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_IF_CFG_14_vpwr_prefetch_bypass_14_START (31)
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_IF_CFG_14_vpwr_prefetch_bypass_14_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int vpwr_access_limiter_0_14 : 4;
        unsigned int vpwr_access_limiter_1_14 : 4;
        unsigned int vpwr_access_limiter_2_14 : 4;
        unsigned int vpwr_access_limiter_3_14 : 4;
        unsigned int reserved_0 : 8;
        unsigned int vpwr_access_limiter_reload_14 : 4;
        unsigned int reserved_1 : 4;
    } reg;
} SOC_IPP_CVDR_CVDR_IPP_VP_WR_LIMITER_14_UNION;
#endif
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_LIMITER_14_vpwr_access_limiter_0_14_START (0)
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_LIMITER_14_vpwr_access_limiter_0_14_END (3)
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_LIMITER_14_vpwr_access_limiter_1_14_START (4)
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_LIMITER_14_vpwr_access_limiter_1_14_END (7)
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_LIMITER_14_vpwr_access_limiter_2_14_START (8)
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_LIMITER_14_vpwr_access_limiter_2_14_END (11)
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_LIMITER_14_vpwr_access_limiter_3_14_START (12)
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_LIMITER_14_vpwr_access_limiter_3_14_END (15)
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_LIMITER_14_vpwr_access_limiter_reload_14_START (24)
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_LIMITER_14_vpwr_access_limiter_reload_14_END (27)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int vpwr_line_stride_sid0_14 : 14;
        unsigned int vpwr_line_start_wstrb_sid0_14 : 4;
        unsigned int vpwr_line_wrap_sid0_14 : 14;
    } reg;
} SOC_IPP_CVDR_CVDR_IPP_VP_WR_SID_AXI_LINE_sid0_14_UNION;
#endif
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_SID_AXI_LINE_sid0_14_vpwr_line_stride_sid0_14_START (0)
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_SID_AXI_LINE_sid0_14_vpwr_line_stride_sid0_14_END (13)
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_SID_AXI_LINE_sid0_14_vpwr_line_start_wstrb_sid0_14_START (14)
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_SID_AXI_LINE_sid0_14_vpwr_line_start_wstrb_sid0_14_END (17)
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_SID_AXI_LINE_sid0_14_vpwr_line_wrap_sid0_14_START (18)
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_SID_AXI_LINE_sid0_14_vpwr_line_wrap_sid0_14_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int vp_wr_stripe_size_bytes_sid0_14 : 16;
        unsigned int reserved : 16;
    } reg;
} SOC_IPP_CVDR_CVDR_IPP_VP_WR_SID_PREFETCH_sid0_14_UNION;
#endif
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_SID_PREFETCH_sid0_14_vp_wr_stripe_size_bytes_sid0_14_START (0)
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_SID_PREFETCH_sid0_14_vp_wr_stripe_size_bytes_sid0_14_END (15)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int vpwr_pixel_format_sid0_14 : 5;
        unsigned int vpwr_pixel_expansion_sid0_14 : 1;
        unsigned int vpwr_4pf_enable_sid0_14 : 1;
        unsigned int reserved : 6;
        unsigned int vpwr_last_page_sid0_14 : 19;
    } reg;
} SOC_IPP_CVDR_CVDR_IPP_VP_WR_SID_CFG_sid0_14_UNION;
#endif
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_SID_CFG_sid0_14_vpwr_pixel_format_sid0_14_START (0)
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_SID_CFG_sid0_14_vpwr_pixel_format_sid0_14_END (4)
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_SID_CFG_sid0_14_vpwr_pixel_expansion_sid0_14_START (5)
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_SID_CFG_sid0_14_vpwr_pixel_expansion_sid0_14_END (5)
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_SID_CFG_sid0_14_vpwr_4pf_enable_sid0_14_START (6)
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_SID_CFG_sid0_14_vpwr_4pf_enable_sid0_14_END (6)
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_SID_CFG_sid0_14_vpwr_last_page_sid0_14_START (13)
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_SID_CFG_sid0_14_vpwr_last_page_sid0_14_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int reserved : 2;
        unsigned int vpwr_address_frame_start_sid0_14 : 30;
    } reg;
} SOC_IPP_CVDR_CVDR_IPP_VP_WR_SID_AXI_FS_sid0_14_UNION;
#endif
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_SID_AXI_FS_sid0_14_vpwr_address_frame_start_sid0_14_START (2)
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_SID_AXI_FS_sid0_14_vpwr_address_frame_start_sid0_14_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int vp_wr_debug_sid0_14 : 32;
    } reg;
} SOC_IPP_CVDR_CVDR_IPP_VP_WR_SID_DEBUG_sid0_14_UNION;
#endif
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_SID_DEBUG_sid0_14_vp_wr_debug_sid0_14_START (0)
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_SID_DEBUG_sid0_14_vp_wr_debug_sid0_14_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int reserved_0 : 16;
        unsigned int vp_wr_stop_enable_du_threshold_reached_15 : 1;
        unsigned int vp_wr_stop_enable_flux_ctrl_15 : 1;
        unsigned int vp_wr_stop_enable_pressure_15 : 1;
        unsigned int reserved_1 : 5;
        unsigned int vp_wr_stop_ok_15 : 1;
        unsigned int vp_wr_stop_15 : 1;
        unsigned int reserved_2 : 5;
        unsigned int vpwr_prefetch_bypass_15 : 1;
    } reg;
} SOC_IPP_CVDR_CVDR_IPP_VP_WR_IF_CFG_15_UNION;
#endif
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_IF_CFG_15_vp_wr_stop_enable_du_threshold_reached_15_START (16)
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_IF_CFG_15_vp_wr_stop_enable_du_threshold_reached_15_END (16)
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_IF_CFG_15_vp_wr_stop_enable_flux_ctrl_15_START (17)
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_IF_CFG_15_vp_wr_stop_enable_flux_ctrl_15_END (17)
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_IF_CFG_15_vp_wr_stop_enable_pressure_15_START (18)
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_IF_CFG_15_vp_wr_stop_enable_pressure_15_END (18)
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_IF_CFG_15_vp_wr_stop_ok_15_START (24)
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_IF_CFG_15_vp_wr_stop_ok_15_END (24)
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_IF_CFG_15_vp_wr_stop_15_START (25)
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_IF_CFG_15_vp_wr_stop_15_END (25)
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_IF_CFG_15_vpwr_prefetch_bypass_15_START (31)
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_IF_CFG_15_vpwr_prefetch_bypass_15_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int vpwr_access_limiter_0_15 : 4;
        unsigned int vpwr_access_limiter_1_15 : 4;
        unsigned int vpwr_access_limiter_2_15 : 4;
        unsigned int vpwr_access_limiter_3_15 : 4;
        unsigned int reserved_0 : 8;
        unsigned int vpwr_access_limiter_reload_15 : 4;
        unsigned int reserved_1 : 4;
    } reg;
} SOC_IPP_CVDR_CVDR_IPP_VP_WR_LIMITER_15_UNION;
#endif
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_LIMITER_15_vpwr_access_limiter_0_15_START (0)
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_LIMITER_15_vpwr_access_limiter_0_15_END (3)
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_LIMITER_15_vpwr_access_limiter_1_15_START (4)
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_LIMITER_15_vpwr_access_limiter_1_15_END (7)
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_LIMITER_15_vpwr_access_limiter_2_15_START (8)
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_LIMITER_15_vpwr_access_limiter_2_15_END (11)
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_LIMITER_15_vpwr_access_limiter_3_15_START (12)
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_LIMITER_15_vpwr_access_limiter_3_15_END (15)
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_LIMITER_15_vpwr_access_limiter_reload_15_START (24)
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_LIMITER_15_vpwr_access_limiter_reload_15_END (27)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int vpwr_line_stride_sid0_15 : 14;
        unsigned int vpwr_line_start_wstrb_sid0_15 : 4;
        unsigned int vpwr_line_wrap_sid0_15 : 14;
    } reg;
} SOC_IPP_CVDR_CVDR_IPP_VP_WR_SID_AXI_LINE_sid0_15_UNION;
#endif
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_SID_AXI_LINE_sid0_15_vpwr_line_stride_sid0_15_START (0)
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_SID_AXI_LINE_sid0_15_vpwr_line_stride_sid0_15_END (13)
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_SID_AXI_LINE_sid0_15_vpwr_line_start_wstrb_sid0_15_START (14)
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_SID_AXI_LINE_sid0_15_vpwr_line_start_wstrb_sid0_15_END (17)
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_SID_AXI_LINE_sid0_15_vpwr_line_wrap_sid0_15_START (18)
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_SID_AXI_LINE_sid0_15_vpwr_line_wrap_sid0_15_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int vp_wr_stripe_size_bytes_sid0_15 : 16;
        unsigned int reserved : 16;
    } reg;
} SOC_IPP_CVDR_CVDR_IPP_VP_WR_SID_PREFETCH_sid0_15_UNION;
#endif
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_SID_PREFETCH_sid0_15_vp_wr_stripe_size_bytes_sid0_15_START (0)
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_SID_PREFETCH_sid0_15_vp_wr_stripe_size_bytes_sid0_15_END (15)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int vpwr_pixel_format_sid0_15 : 5;
        unsigned int vpwr_pixel_expansion_sid0_15 : 1;
        unsigned int vpwr_4pf_enable_sid0_15 : 1;
        unsigned int reserved : 6;
        unsigned int vpwr_last_page_sid0_15 : 19;
    } reg;
} SOC_IPP_CVDR_CVDR_IPP_VP_WR_SID_CFG_sid0_15_UNION;
#endif
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_SID_CFG_sid0_15_vpwr_pixel_format_sid0_15_START (0)
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_SID_CFG_sid0_15_vpwr_pixel_format_sid0_15_END (4)
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_SID_CFG_sid0_15_vpwr_pixel_expansion_sid0_15_START (5)
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_SID_CFG_sid0_15_vpwr_pixel_expansion_sid0_15_END (5)
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_SID_CFG_sid0_15_vpwr_4pf_enable_sid0_15_START (6)
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_SID_CFG_sid0_15_vpwr_4pf_enable_sid0_15_END (6)
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_SID_CFG_sid0_15_vpwr_last_page_sid0_15_START (13)
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_SID_CFG_sid0_15_vpwr_last_page_sid0_15_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int reserved : 2;
        unsigned int vpwr_address_frame_start_sid0_15 : 30;
    } reg;
} SOC_IPP_CVDR_CVDR_IPP_VP_WR_SID_AXI_FS_sid0_15_UNION;
#endif
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_SID_AXI_FS_sid0_15_vpwr_address_frame_start_sid0_15_START (2)
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_SID_AXI_FS_sid0_15_vpwr_address_frame_start_sid0_15_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int vp_wr_debug_sid0_15 : 32;
    } reg;
} SOC_IPP_CVDR_CVDR_IPP_VP_WR_SID_DEBUG_sid0_15_UNION;
#endif
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_SID_DEBUG_sid0_15_vp_wr_debug_sid0_15_START (0)
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_SID_DEBUG_sid0_15_vp_wr_debug_sid0_15_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int reserved_0 : 16;
        unsigned int vp_wr_stop_enable_du_threshold_reached_16 : 1;
        unsigned int vp_wr_stop_enable_flux_ctrl_16 : 1;
        unsigned int vp_wr_stop_enable_pressure_16 : 1;
        unsigned int reserved_1 : 5;
        unsigned int vp_wr_stop_ok_16 : 1;
        unsigned int vp_wr_stop_16 : 1;
        unsigned int reserved_2 : 5;
        unsigned int vpwr_prefetch_bypass_16 : 1;
    } reg;
} SOC_IPP_CVDR_CVDR_IPP_VP_WR_IF_CFG_16_UNION;
#endif
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_IF_CFG_16_vp_wr_stop_enable_du_threshold_reached_16_START (16)
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_IF_CFG_16_vp_wr_stop_enable_du_threshold_reached_16_END (16)
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_IF_CFG_16_vp_wr_stop_enable_flux_ctrl_16_START (17)
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_IF_CFG_16_vp_wr_stop_enable_flux_ctrl_16_END (17)
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_IF_CFG_16_vp_wr_stop_enable_pressure_16_START (18)
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_IF_CFG_16_vp_wr_stop_enable_pressure_16_END (18)
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_IF_CFG_16_vp_wr_stop_ok_16_START (24)
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_IF_CFG_16_vp_wr_stop_ok_16_END (24)
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_IF_CFG_16_vp_wr_stop_16_START (25)
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_IF_CFG_16_vp_wr_stop_16_END (25)
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_IF_CFG_16_vpwr_prefetch_bypass_16_START (31)
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_IF_CFG_16_vpwr_prefetch_bypass_16_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int vpwr_access_limiter_0_16 : 4;
        unsigned int vpwr_access_limiter_1_16 : 4;
        unsigned int vpwr_access_limiter_2_16 : 4;
        unsigned int vpwr_access_limiter_3_16 : 4;
        unsigned int reserved_0 : 8;
        unsigned int vpwr_access_limiter_reload_16 : 4;
        unsigned int reserved_1 : 4;
    } reg;
} SOC_IPP_CVDR_CVDR_IPP_VP_WR_LIMITER_16_UNION;
#endif
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_LIMITER_16_vpwr_access_limiter_0_16_START (0)
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_LIMITER_16_vpwr_access_limiter_0_16_END (3)
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_LIMITER_16_vpwr_access_limiter_1_16_START (4)
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_LIMITER_16_vpwr_access_limiter_1_16_END (7)
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_LIMITER_16_vpwr_access_limiter_2_16_START (8)
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_LIMITER_16_vpwr_access_limiter_2_16_END (11)
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_LIMITER_16_vpwr_access_limiter_3_16_START (12)
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_LIMITER_16_vpwr_access_limiter_3_16_END (15)
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_LIMITER_16_vpwr_access_limiter_reload_16_START (24)
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_LIMITER_16_vpwr_access_limiter_reload_16_END (27)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int vpwr_line_stride_sid0_16 : 14;
        unsigned int vpwr_line_start_wstrb_sid0_16 : 4;
        unsigned int vpwr_line_wrap_sid0_16 : 14;
    } reg;
} SOC_IPP_CVDR_CVDR_IPP_VP_WR_SID_AXI_LINE_sid0_16_UNION;
#endif
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_SID_AXI_LINE_sid0_16_vpwr_line_stride_sid0_16_START (0)
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_SID_AXI_LINE_sid0_16_vpwr_line_stride_sid0_16_END (13)
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_SID_AXI_LINE_sid0_16_vpwr_line_start_wstrb_sid0_16_START (14)
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_SID_AXI_LINE_sid0_16_vpwr_line_start_wstrb_sid0_16_END (17)
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_SID_AXI_LINE_sid0_16_vpwr_line_wrap_sid0_16_START (18)
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_SID_AXI_LINE_sid0_16_vpwr_line_wrap_sid0_16_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int vp_wr_stripe_size_bytes_sid0_16 : 16;
        unsigned int reserved : 16;
    } reg;
} SOC_IPP_CVDR_CVDR_IPP_VP_WR_SID_PREFETCH_sid0_16_UNION;
#endif
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_SID_PREFETCH_sid0_16_vp_wr_stripe_size_bytes_sid0_16_START (0)
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_SID_PREFETCH_sid0_16_vp_wr_stripe_size_bytes_sid0_16_END (15)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int vpwr_pixel_format_sid0_16 : 5;
        unsigned int vpwr_pixel_expansion_sid0_16 : 1;
        unsigned int vpwr_4pf_enable_sid0_16 : 1;
        unsigned int reserved : 6;
        unsigned int vpwr_last_page_sid0_16 : 19;
    } reg;
} SOC_IPP_CVDR_CVDR_IPP_VP_WR_SID_CFG_sid0_16_UNION;
#endif
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_SID_CFG_sid0_16_vpwr_pixel_format_sid0_16_START (0)
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_SID_CFG_sid0_16_vpwr_pixel_format_sid0_16_END (4)
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_SID_CFG_sid0_16_vpwr_pixel_expansion_sid0_16_START (5)
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_SID_CFG_sid0_16_vpwr_pixel_expansion_sid0_16_END (5)
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_SID_CFG_sid0_16_vpwr_4pf_enable_sid0_16_START (6)
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_SID_CFG_sid0_16_vpwr_4pf_enable_sid0_16_END (6)
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_SID_CFG_sid0_16_vpwr_last_page_sid0_16_START (13)
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_SID_CFG_sid0_16_vpwr_last_page_sid0_16_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int reserved : 2;
        unsigned int vpwr_address_frame_start_sid0_16 : 30;
    } reg;
} SOC_IPP_CVDR_CVDR_IPP_VP_WR_SID_AXI_FS_sid0_16_UNION;
#endif
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_SID_AXI_FS_sid0_16_vpwr_address_frame_start_sid0_16_START (2)
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_SID_AXI_FS_sid0_16_vpwr_address_frame_start_sid0_16_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int vp_wr_debug_sid0_16 : 32;
    } reg;
} SOC_IPP_CVDR_CVDR_IPP_VP_WR_SID_DEBUG_sid0_16_UNION;
#endif
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_SID_DEBUG_sid0_16_vp_wr_debug_sid0_16_START (0)
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_SID_DEBUG_sid0_16_vp_wr_debug_sid0_16_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int reserved_0 : 16;
        unsigned int vp_wr_stop_enable_du_threshold_reached_17 : 1;
        unsigned int vp_wr_stop_enable_flux_ctrl_17 : 1;
        unsigned int vp_wr_stop_enable_pressure_17 : 1;
        unsigned int reserved_1 : 5;
        unsigned int vp_wr_stop_ok_17 : 1;
        unsigned int vp_wr_stop_17 : 1;
        unsigned int reserved_2 : 5;
        unsigned int vpwr_prefetch_bypass_17 : 1;
    } reg;
} SOC_IPP_CVDR_CVDR_IPP_VP_WR_IF_CFG_17_UNION;
#endif
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_IF_CFG_17_vp_wr_stop_enable_du_threshold_reached_17_START (16)
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_IF_CFG_17_vp_wr_stop_enable_du_threshold_reached_17_END (16)
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_IF_CFG_17_vp_wr_stop_enable_flux_ctrl_17_START (17)
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_IF_CFG_17_vp_wr_stop_enable_flux_ctrl_17_END (17)
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_IF_CFG_17_vp_wr_stop_enable_pressure_17_START (18)
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_IF_CFG_17_vp_wr_stop_enable_pressure_17_END (18)
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_IF_CFG_17_vp_wr_stop_ok_17_START (24)
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_IF_CFG_17_vp_wr_stop_ok_17_END (24)
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_IF_CFG_17_vp_wr_stop_17_START (25)
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_IF_CFG_17_vp_wr_stop_17_END (25)
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_IF_CFG_17_vpwr_prefetch_bypass_17_START (31)
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_IF_CFG_17_vpwr_prefetch_bypass_17_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int vpwr_access_limiter_0_17 : 4;
        unsigned int vpwr_access_limiter_1_17 : 4;
        unsigned int vpwr_access_limiter_2_17 : 4;
        unsigned int vpwr_access_limiter_3_17 : 4;
        unsigned int reserved_0 : 8;
        unsigned int vpwr_access_limiter_reload_17 : 4;
        unsigned int reserved_1 : 4;
    } reg;
} SOC_IPP_CVDR_CVDR_IPP_VP_WR_LIMITER_17_UNION;
#endif
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_LIMITER_17_vpwr_access_limiter_0_17_START (0)
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_LIMITER_17_vpwr_access_limiter_0_17_END (3)
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_LIMITER_17_vpwr_access_limiter_1_17_START (4)
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_LIMITER_17_vpwr_access_limiter_1_17_END (7)
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_LIMITER_17_vpwr_access_limiter_2_17_START (8)
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_LIMITER_17_vpwr_access_limiter_2_17_END (11)
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_LIMITER_17_vpwr_access_limiter_3_17_START (12)
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_LIMITER_17_vpwr_access_limiter_3_17_END (15)
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_LIMITER_17_vpwr_access_limiter_reload_17_START (24)
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_LIMITER_17_vpwr_access_limiter_reload_17_END (27)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int vpwr_line_stride_sid0_17 : 14;
        unsigned int vpwr_line_start_wstrb_sid0_17 : 4;
        unsigned int vpwr_line_wrap_sid0_17 : 14;
    } reg;
} SOC_IPP_CVDR_CVDR_IPP_VP_WR_SID_AXI_LINE_sid0_17_UNION;
#endif
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_SID_AXI_LINE_sid0_17_vpwr_line_stride_sid0_17_START (0)
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_SID_AXI_LINE_sid0_17_vpwr_line_stride_sid0_17_END (13)
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_SID_AXI_LINE_sid0_17_vpwr_line_start_wstrb_sid0_17_START (14)
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_SID_AXI_LINE_sid0_17_vpwr_line_start_wstrb_sid0_17_END (17)
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_SID_AXI_LINE_sid0_17_vpwr_line_wrap_sid0_17_START (18)
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_SID_AXI_LINE_sid0_17_vpwr_line_wrap_sid0_17_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int vp_wr_stripe_size_bytes_sid0_17 : 16;
        unsigned int reserved : 16;
    } reg;
} SOC_IPP_CVDR_CVDR_IPP_VP_WR_SID_PREFETCH_sid0_17_UNION;
#endif
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_SID_PREFETCH_sid0_17_vp_wr_stripe_size_bytes_sid0_17_START (0)
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_SID_PREFETCH_sid0_17_vp_wr_stripe_size_bytes_sid0_17_END (15)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int vpwr_pixel_format_sid0_17 : 5;
        unsigned int vpwr_pixel_expansion_sid0_17 : 1;
        unsigned int vpwr_4pf_enable_sid0_17 : 1;
        unsigned int reserved : 6;
        unsigned int vpwr_last_page_sid0_17 : 19;
    } reg;
} SOC_IPP_CVDR_CVDR_IPP_VP_WR_SID_CFG_sid0_17_UNION;
#endif
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_SID_CFG_sid0_17_vpwr_pixel_format_sid0_17_START (0)
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_SID_CFG_sid0_17_vpwr_pixel_format_sid0_17_END (4)
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_SID_CFG_sid0_17_vpwr_pixel_expansion_sid0_17_START (5)
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_SID_CFG_sid0_17_vpwr_pixel_expansion_sid0_17_END (5)
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_SID_CFG_sid0_17_vpwr_4pf_enable_sid0_17_START (6)
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_SID_CFG_sid0_17_vpwr_4pf_enable_sid0_17_END (6)
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_SID_CFG_sid0_17_vpwr_last_page_sid0_17_START (13)
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_SID_CFG_sid0_17_vpwr_last_page_sid0_17_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int reserved : 2;
        unsigned int vpwr_address_frame_start_sid0_17 : 30;
    } reg;
} SOC_IPP_CVDR_CVDR_IPP_VP_WR_SID_AXI_FS_sid0_17_UNION;
#endif
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_SID_AXI_FS_sid0_17_vpwr_address_frame_start_sid0_17_START (2)
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_SID_AXI_FS_sid0_17_vpwr_address_frame_start_sid0_17_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int vp_wr_debug_sid0_17 : 32;
    } reg;
} SOC_IPP_CVDR_CVDR_IPP_VP_WR_SID_DEBUG_sid0_17_UNION;
#endif
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_SID_DEBUG_sid0_17_vp_wr_debug_sid0_17_START (0)
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_SID_DEBUG_sid0_17_vp_wr_debug_sid0_17_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int reserved_0 : 16;
        unsigned int vp_wr_stop_enable_du_threshold_reached_20 : 1;
        unsigned int vp_wr_stop_enable_flux_ctrl_20 : 1;
        unsigned int vp_wr_stop_enable_pressure_20 : 1;
        unsigned int reserved_1 : 5;
        unsigned int vp_wr_stop_ok_20 : 1;
        unsigned int vp_wr_stop_20 : 1;
        unsigned int reserved_2 : 5;
        unsigned int vpwr_prefetch_bypass_20 : 1;
    } reg;
} SOC_IPP_CVDR_CVDR_IPP_VP_WR_IF_CFG_20_UNION;
#endif
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_IF_CFG_20_vp_wr_stop_enable_du_threshold_reached_20_START (16)
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_IF_CFG_20_vp_wr_stop_enable_du_threshold_reached_20_END (16)
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_IF_CFG_20_vp_wr_stop_enable_flux_ctrl_20_START (17)
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_IF_CFG_20_vp_wr_stop_enable_flux_ctrl_20_END (17)
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_IF_CFG_20_vp_wr_stop_enable_pressure_20_START (18)
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_IF_CFG_20_vp_wr_stop_enable_pressure_20_END (18)
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_IF_CFG_20_vp_wr_stop_ok_20_START (24)
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_IF_CFG_20_vp_wr_stop_ok_20_END (24)
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_IF_CFG_20_vp_wr_stop_20_START (25)
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_IF_CFG_20_vp_wr_stop_20_END (25)
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_IF_CFG_20_vpwr_prefetch_bypass_20_START (31)
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_IF_CFG_20_vpwr_prefetch_bypass_20_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int vpwr_access_limiter_0_20 : 4;
        unsigned int vpwr_access_limiter_1_20 : 4;
        unsigned int vpwr_access_limiter_2_20 : 4;
        unsigned int vpwr_access_limiter_3_20 : 4;
        unsigned int reserved_0 : 8;
        unsigned int vpwr_access_limiter_reload_20 : 4;
        unsigned int reserved_1 : 4;
    } reg;
} SOC_IPP_CVDR_CVDR_IPP_VP_WR_LIMITER_20_UNION;
#endif
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_LIMITER_20_vpwr_access_limiter_0_20_START (0)
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_LIMITER_20_vpwr_access_limiter_0_20_END (3)
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_LIMITER_20_vpwr_access_limiter_1_20_START (4)
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_LIMITER_20_vpwr_access_limiter_1_20_END (7)
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_LIMITER_20_vpwr_access_limiter_2_20_START (8)
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_LIMITER_20_vpwr_access_limiter_2_20_END (11)
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_LIMITER_20_vpwr_access_limiter_3_20_START (12)
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_LIMITER_20_vpwr_access_limiter_3_20_END (15)
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_LIMITER_20_vpwr_access_limiter_reload_20_START (24)
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_LIMITER_20_vpwr_access_limiter_reload_20_END (27)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int vpwr_line_stride_sid0_20 : 14;
        unsigned int vpwr_line_start_wstrb_sid0_20 : 4;
        unsigned int vpwr_line_wrap_sid0_20 : 14;
    } reg;
} SOC_IPP_CVDR_CVDR_IPP_VP_WR_SID_AXI_LINE_sid0_20_UNION;
#endif
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_SID_AXI_LINE_sid0_20_vpwr_line_stride_sid0_20_START (0)
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_SID_AXI_LINE_sid0_20_vpwr_line_stride_sid0_20_END (13)
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_SID_AXI_LINE_sid0_20_vpwr_line_start_wstrb_sid0_20_START (14)
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_SID_AXI_LINE_sid0_20_vpwr_line_start_wstrb_sid0_20_END (17)
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_SID_AXI_LINE_sid0_20_vpwr_line_wrap_sid0_20_START (18)
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_SID_AXI_LINE_sid0_20_vpwr_line_wrap_sid0_20_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int vp_wr_stripe_size_bytes_sid0_20 : 16;
        unsigned int reserved : 16;
    } reg;
} SOC_IPP_CVDR_CVDR_IPP_VP_WR_SID_PREFETCH_sid0_20_UNION;
#endif
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_SID_PREFETCH_sid0_20_vp_wr_stripe_size_bytes_sid0_20_START (0)
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_SID_PREFETCH_sid0_20_vp_wr_stripe_size_bytes_sid0_20_END (15)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int vpwr_pixel_format_sid0_20 : 5;
        unsigned int vpwr_pixel_expansion_sid0_20 : 1;
        unsigned int vpwr_4pf_enable_sid0_20 : 1;
        unsigned int reserved : 6;
        unsigned int vpwr_last_page_sid0_20 : 19;
    } reg;
} SOC_IPP_CVDR_CVDR_IPP_VP_WR_SID_CFG_sid0_20_UNION;
#endif
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_SID_CFG_sid0_20_vpwr_pixel_format_sid0_20_START (0)
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_SID_CFG_sid0_20_vpwr_pixel_format_sid0_20_END (4)
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_SID_CFG_sid0_20_vpwr_pixel_expansion_sid0_20_START (5)
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_SID_CFG_sid0_20_vpwr_pixel_expansion_sid0_20_END (5)
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_SID_CFG_sid0_20_vpwr_4pf_enable_sid0_20_START (6)
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_SID_CFG_sid0_20_vpwr_4pf_enable_sid0_20_END (6)
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_SID_CFG_sid0_20_vpwr_last_page_sid0_20_START (13)
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_SID_CFG_sid0_20_vpwr_last_page_sid0_20_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int reserved : 2;
        unsigned int vpwr_address_frame_start_sid0_20 : 30;
    } reg;
} SOC_IPP_CVDR_CVDR_IPP_VP_WR_SID_AXI_FS_sid0_20_UNION;
#endif
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_SID_AXI_FS_sid0_20_vpwr_address_frame_start_sid0_20_START (2)
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_SID_AXI_FS_sid0_20_vpwr_address_frame_start_sid0_20_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int vp_wr_debug_sid0_20 : 32;
    } reg;
} SOC_IPP_CVDR_CVDR_IPP_VP_WR_SID_DEBUG_sid0_20_UNION;
#endif
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_SID_DEBUG_sid0_20_vp_wr_debug_sid0_20_START (0)
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_SID_DEBUG_sid0_20_vp_wr_debug_sid0_20_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int reserved_0 : 16;
        unsigned int vp_wr_stop_enable_du_threshold_reached_21 : 1;
        unsigned int vp_wr_stop_enable_flux_ctrl_21 : 1;
        unsigned int vp_wr_stop_enable_pressure_21 : 1;
        unsigned int reserved_1 : 5;
        unsigned int vp_wr_stop_ok_21 : 1;
        unsigned int vp_wr_stop_21 : 1;
        unsigned int reserved_2 : 5;
        unsigned int vpwr_prefetch_bypass_21 : 1;
    } reg;
} SOC_IPP_CVDR_CVDR_IPP_VP_WR_IF_CFG_21_UNION;
#endif
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_IF_CFG_21_vp_wr_stop_enable_du_threshold_reached_21_START (16)
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_IF_CFG_21_vp_wr_stop_enable_du_threshold_reached_21_END (16)
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_IF_CFG_21_vp_wr_stop_enable_flux_ctrl_21_START (17)
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_IF_CFG_21_vp_wr_stop_enable_flux_ctrl_21_END (17)
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_IF_CFG_21_vp_wr_stop_enable_pressure_21_START (18)
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_IF_CFG_21_vp_wr_stop_enable_pressure_21_END (18)
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_IF_CFG_21_vp_wr_stop_ok_21_START (24)
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_IF_CFG_21_vp_wr_stop_ok_21_END (24)
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_IF_CFG_21_vp_wr_stop_21_START (25)
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_IF_CFG_21_vp_wr_stop_21_END (25)
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_IF_CFG_21_vpwr_prefetch_bypass_21_START (31)
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_IF_CFG_21_vpwr_prefetch_bypass_21_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int vpwr_access_limiter_0_21 : 4;
        unsigned int vpwr_access_limiter_1_21 : 4;
        unsigned int vpwr_access_limiter_2_21 : 4;
        unsigned int vpwr_access_limiter_3_21 : 4;
        unsigned int reserved_0 : 8;
        unsigned int vpwr_access_limiter_reload_21 : 4;
        unsigned int reserved_1 : 4;
    } reg;
} SOC_IPP_CVDR_CVDR_IPP_VP_WR_LIMITER_21_UNION;
#endif
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_LIMITER_21_vpwr_access_limiter_0_21_START (0)
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_LIMITER_21_vpwr_access_limiter_0_21_END (3)
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_LIMITER_21_vpwr_access_limiter_1_21_START (4)
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_LIMITER_21_vpwr_access_limiter_1_21_END (7)
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_LIMITER_21_vpwr_access_limiter_2_21_START (8)
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_LIMITER_21_vpwr_access_limiter_2_21_END (11)
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_LIMITER_21_vpwr_access_limiter_3_21_START (12)
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_LIMITER_21_vpwr_access_limiter_3_21_END (15)
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_LIMITER_21_vpwr_access_limiter_reload_21_START (24)
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_LIMITER_21_vpwr_access_limiter_reload_21_END (27)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int vpwr_line_stride_sid0_21 : 14;
        unsigned int vpwr_line_start_wstrb_sid0_21 : 4;
        unsigned int vpwr_line_wrap_sid0_21 : 14;
    } reg;
} SOC_IPP_CVDR_CVDR_IPP_VP_WR_SID_AXI_LINE_sid0_21_UNION;
#endif
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_SID_AXI_LINE_sid0_21_vpwr_line_stride_sid0_21_START (0)
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_SID_AXI_LINE_sid0_21_vpwr_line_stride_sid0_21_END (13)
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_SID_AXI_LINE_sid0_21_vpwr_line_start_wstrb_sid0_21_START (14)
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_SID_AXI_LINE_sid0_21_vpwr_line_start_wstrb_sid0_21_END (17)
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_SID_AXI_LINE_sid0_21_vpwr_line_wrap_sid0_21_START (18)
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_SID_AXI_LINE_sid0_21_vpwr_line_wrap_sid0_21_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int vp_wr_stripe_size_bytes_sid0_21 : 16;
        unsigned int reserved : 16;
    } reg;
} SOC_IPP_CVDR_CVDR_IPP_VP_WR_SID_PREFETCH_sid0_21_UNION;
#endif
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_SID_PREFETCH_sid0_21_vp_wr_stripe_size_bytes_sid0_21_START (0)
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_SID_PREFETCH_sid0_21_vp_wr_stripe_size_bytes_sid0_21_END (15)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int vpwr_pixel_format_sid0_21 : 5;
        unsigned int vpwr_pixel_expansion_sid0_21 : 1;
        unsigned int vpwr_4pf_enable_sid0_21 : 1;
        unsigned int reserved : 6;
        unsigned int vpwr_last_page_sid0_21 : 19;
    } reg;
} SOC_IPP_CVDR_CVDR_IPP_VP_WR_SID_CFG_sid0_21_UNION;
#endif
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_SID_CFG_sid0_21_vpwr_pixel_format_sid0_21_START (0)
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_SID_CFG_sid0_21_vpwr_pixel_format_sid0_21_END (4)
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_SID_CFG_sid0_21_vpwr_pixel_expansion_sid0_21_START (5)
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_SID_CFG_sid0_21_vpwr_pixel_expansion_sid0_21_END (5)
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_SID_CFG_sid0_21_vpwr_4pf_enable_sid0_21_START (6)
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_SID_CFG_sid0_21_vpwr_4pf_enable_sid0_21_END (6)
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_SID_CFG_sid0_21_vpwr_last_page_sid0_21_START (13)
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_SID_CFG_sid0_21_vpwr_last_page_sid0_21_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int reserved : 2;
        unsigned int vpwr_address_frame_start_sid0_21 : 30;
    } reg;
} SOC_IPP_CVDR_CVDR_IPP_VP_WR_SID_AXI_FS_sid0_21_UNION;
#endif
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_SID_AXI_FS_sid0_21_vpwr_address_frame_start_sid0_21_START (2)
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_SID_AXI_FS_sid0_21_vpwr_address_frame_start_sid0_21_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int vp_wr_debug_sid0_21 : 32;
    } reg;
} SOC_IPP_CVDR_CVDR_IPP_VP_WR_SID_DEBUG_sid0_21_UNION;
#endif
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_SID_DEBUG_sid0_21_vp_wr_debug_sid0_21_START (0)
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_SID_DEBUG_sid0_21_vp_wr_debug_sid0_21_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int reserved_0 : 16;
        unsigned int vp_wr_stop_enable_du_threshold_reached_22 : 1;
        unsigned int vp_wr_stop_enable_flux_ctrl_22 : 1;
        unsigned int vp_wr_stop_enable_pressure_22 : 1;
        unsigned int reserved_1 : 5;
        unsigned int vp_wr_stop_ok_22 : 1;
        unsigned int vp_wr_stop_22 : 1;
        unsigned int reserved_2 : 5;
        unsigned int vpwr_prefetch_bypass_22 : 1;
    } reg;
} SOC_IPP_CVDR_CVDR_IPP_VP_WR_IF_CFG_22_UNION;
#endif
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_IF_CFG_22_vp_wr_stop_enable_du_threshold_reached_22_START (16)
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_IF_CFG_22_vp_wr_stop_enable_du_threshold_reached_22_END (16)
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_IF_CFG_22_vp_wr_stop_enable_flux_ctrl_22_START (17)
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_IF_CFG_22_vp_wr_stop_enable_flux_ctrl_22_END (17)
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_IF_CFG_22_vp_wr_stop_enable_pressure_22_START (18)
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_IF_CFG_22_vp_wr_stop_enable_pressure_22_END (18)
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_IF_CFG_22_vp_wr_stop_ok_22_START (24)
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_IF_CFG_22_vp_wr_stop_ok_22_END (24)
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_IF_CFG_22_vp_wr_stop_22_START (25)
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_IF_CFG_22_vp_wr_stop_22_END (25)
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_IF_CFG_22_vpwr_prefetch_bypass_22_START (31)
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_IF_CFG_22_vpwr_prefetch_bypass_22_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int vpwr_access_limiter_0_22 : 4;
        unsigned int vpwr_access_limiter_1_22 : 4;
        unsigned int vpwr_access_limiter_2_22 : 4;
        unsigned int vpwr_access_limiter_3_22 : 4;
        unsigned int reserved_0 : 8;
        unsigned int vpwr_access_limiter_reload_22 : 4;
        unsigned int reserved_1 : 4;
    } reg;
} SOC_IPP_CVDR_CVDR_IPP_VP_WR_LIMITER_22_UNION;
#endif
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_LIMITER_22_vpwr_access_limiter_0_22_START (0)
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_LIMITER_22_vpwr_access_limiter_0_22_END (3)
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_LIMITER_22_vpwr_access_limiter_1_22_START (4)
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_LIMITER_22_vpwr_access_limiter_1_22_END (7)
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_LIMITER_22_vpwr_access_limiter_2_22_START (8)
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_LIMITER_22_vpwr_access_limiter_2_22_END (11)
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_LIMITER_22_vpwr_access_limiter_3_22_START (12)
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_LIMITER_22_vpwr_access_limiter_3_22_END (15)
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_LIMITER_22_vpwr_access_limiter_reload_22_START (24)
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_LIMITER_22_vpwr_access_limiter_reload_22_END (27)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int vpwr_line_stride_sid0_22 : 14;
        unsigned int vpwr_line_start_wstrb_sid0_22 : 4;
        unsigned int vpwr_line_wrap_sid0_22 : 14;
    } reg;
} SOC_IPP_CVDR_CVDR_IPP_VP_WR_SID_AXI_LINE_sid0_22_UNION;
#endif
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_SID_AXI_LINE_sid0_22_vpwr_line_stride_sid0_22_START (0)
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_SID_AXI_LINE_sid0_22_vpwr_line_stride_sid0_22_END (13)
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_SID_AXI_LINE_sid0_22_vpwr_line_start_wstrb_sid0_22_START (14)
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_SID_AXI_LINE_sid0_22_vpwr_line_start_wstrb_sid0_22_END (17)
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_SID_AXI_LINE_sid0_22_vpwr_line_wrap_sid0_22_START (18)
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_SID_AXI_LINE_sid0_22_vpwr_line_wrap_sid0_22_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int vp_wr_stripe_size_bytes_sid0_22 : 16;
        unsigned int reserved : 16;
    } reg;
} SOC_IPP_CVDR_CVDR_IPP_VP_WR_SID_PREFETCH_sid0_22_UNION;
#endif
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_SID_PREFETCH_sid0_22_vp_wr_stripe_size_bytes_sid0_22_START (0)
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_SID_PREFETCH_sid0_22_vp_wr_stripe_size_bytes_sid0_22_END (15)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int vpwr_pixel_format_sid0_22 : 5;
        unsigned int vpwr_pixel_expansion_sid0_22 : 1;
        unsigned int vpwr_4pf_enable_sid0_22 : 1;
        unsigned int reserved : 6;
        unsigned int vpwr_last_page_sid0_22 : 19;
    } reg;
} SOC_IPP_CVDR_CVDR_IPP_VP_WR_SID_CFG_sid0_22_UNION;
#endif
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_SID_CFG_sid0_22_vpwr_pixel_format_sid0_22_START (0)
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_SID_CFG_sid0_22_vpwr_pixel_format_sid0_22_END (4)
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_SID_CFG_sid0_22_vpwr_pixel_expansion_sid0_22_START (5)
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_SID_CFG_sid0_22_vpwr_pixel_expansion_sid0_22_END (5)
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_SID_CFG_sid0_22_vpwr_4pf_enable_sid0_22_START (6)
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_SID_CFG_sid0_22_vpwr_4pf_enable_sid0_22_END (6)
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_SID_CFG_sid0_22_vpwr_last_page_sid0_22_START (13)
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_SID_CFG_sid0_22_vpwr_last_page_sid0_22_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int reserved : 2;
        unsigned int vpwr_address_frame_start_sid0_22 : 30;
    } reg;
} SOC_IPP_CVDR_CVDR_IPP_VP_WR_SID_AXI_FS_sid0_22_UNION;
#endif
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_SID_AXI_FS_sid0_22_vpwr_address_frame_start_sid0_22_START (2)
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_SID_AXI_FS_sid0_22_vpwr_address_frame_start_sid0_22_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int vp_wr_debug_sid0_22 : 32;
    } reg;
} SOC_IPP_CVDR_CVDR_IPP_VP_WR_SID_DEBUG_sid0_22_UNION;
#endif
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_SID_DEBUG_sid0_22_vp_wr_debug_sid0_22_START (0)
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_SID_DEBUG_sid0_22_vp_wr_debug_sid0_22_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int reserved_0 : 16;
        unsigned int vp_wr_stop_enable_du_threshold_reached_23 : 1;
        unsigned int vp_wr_stop_enable_flux_ctrl_23 : 1;
        unsigned int vp_wr_stop_enable_pressure_23 : 1;
        unsigned int reserved_1 : 5;
        unsigned int vp_wr_stop_ok_23 : 1;
        unsigned int vp_wr_stop_23 : 1;
        unsigned int reserved_2 : 5;
        unsigned int vpwr_prefetch_bypass_23 : 1;
    } reg;
} SOC_IPP_CVDR_CVDR_IPP_VP_WR_IF_CFG_23_UNION;
#endif
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_IF_CFG_23_vp_wr_stop_enable_du_threshold_reached_23_START (16)
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_IF_CFG_23_vp_wr_stop_enable_du_threshold_reached_23_END (16)
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_IF_CFG_23_vp_wr_stop_enable_flux_ctrl_23_START (17)
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_IF_CFG_23_vp_wr_stop_enable_flux_ctrl_23_END (17)
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_IF_CFG_23_vp_wr_stop_enable_pressure_23_START (18)
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_IF_CFG_23_vp_wr_stop_enable_pressure_23_END (18)
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_IF_CFG_23_vp_wr_stop_ok_23_START (24)
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_IF_CFG_23_vp_wr_stop_ok_23_END (24)
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_IF_CFG_23_vp_wr_stop_23_START (25)
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_IF_CFG_23_vp_wr_stop_23_END (25)
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_IF_CFG_23_vpwr_prefetch_bypass_23_START (31)
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_IF_CFG_23_vpwr_prefetch_bypass_23_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int vpwr_access_limiter_0_23 : 4;
        unsigned int vpwr_access_limiter_1_23 : 4;
        unsigned int vpwr_access_limiter_2_23 : 4;
        unsigned int vpwr_access_limiter_3_23 : 4;
        unsigned int reserved_0 : 8;
        unsigned int vpwr_access_limiter_reload_23 : 4;
        unsigned int reserved_1 : 4;
    } reg;
} SOC_IPP_CVDR_CVDR_IPP_VP_WR_LIMITER_23_UNION;
#endif
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_LIMITER_23_vpwr_access_limiter_0_23_START (0)
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_LIMITER_23_vpwr_access_limiter_0_23_END (3)
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_LIMITER_23_vpwr_access_limiter_1_23_START (4)
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_LIMITER_23_vpwr_access_limiter_1_23_END (7)
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_LIMITER_23_vpwr_access_limiter_2_23_START (8)
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_LIMITER_23_vpwr_access_limiter_2_23_END (11)
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_LIMITER_23_vpwr_access_limiter_3_23_START (12)
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_LIMITER_23_vpwr_access_limiter_3_23_END (15)
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_LIMITER_23_vpwr_access_limiter_reload_23_START (24)
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_LIMITER_23_vpwr_access_limiter_reload_23_END (27)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int vpwr_line_stride_sid0_23 : 14;
        unsigned int vpwr_line_start_wstrb_sid0_23 : 4;
        unsigned int vpwr_line_wrap_sid0_23 : 14;
    } reg;
} SOC_IPP_CVDR_CVDR_IPP_VP_WR_SID_AXI_LINE_sid0_23_UNION;
#endif
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_SID_AXI_LINE_sid0_23_vpwr_line_stride_sid0_23_START (0)
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_SID_AXI_LINE_sid0_23_vpwr_line_stride_sid0_23_END (13)
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_SID_AXI_LINE_sid0_23_vpwr_line_start_wstrb_sid0_23_START (14)
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_SID_AXI_LINE_sid0_23_vpwr_line_start_wstrb_sid0_23_END (17)
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_SID_AXI_LINE_sid0_23_vpwr_line_wrap_sid0_23_START (18)
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_SID_AXI_LINE_sid0_23_vpwr_line_wrap_sid0_23_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int vp_wr_stripe_size_bytes_sid0_23 : 16;
        unsigned int reserved : 16;
    } reg;
} SOC_IPP_CVDR_CVDR_IPP_VP_WR_SID_PREFETCH_sid0_23_UNION;
#endif
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_SID_PREFETCH_sid0_23_vp_wr_stripe_size_bytes_sid0_23_START (0)
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_SID_PREFETCH_sid0_23_vp_wr_stripe_size_bytes_sid0_23_END (15)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int vpwr_pixel_format_sid0_23 : 5;
        unsigned int vpwr_pixel_expansion_sid0_23 : 1;
        unsigned int vpwr_4pf_enable_sid0_23 : 1;
        unsigned int reserved : 6;
        unsigned int vpwr_last_page_sid0_23 : 19;
    } reg;
} SOC_IPP_CVDR_CVDR_IPP_VP_WR_SID_CFG_sid0_23_UNION;
#endif
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_SID_CFG_sid0_23_vpwr_pixel_format_sid0_23_START (0)
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_SID_CFG_sid0_23_vpwr_pixel_format_sid0_23_END (4)
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_SID_CFG_sid0_23_vpwr_pixel_expansion_sid0_23_START (5)
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_SID_CFG_sid0_23_vpwr_pixel_expansion_sid0_23_END (5)
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_SID_CFG_sid0_23_vpwr_4pf_enable_sid0_23_START (6)
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_SID_CFG_sid0_23_vpwr_4pf_enable_sid0_23_END (6)
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_SID_CFG_sid0_23_vpwr_last_page_sid0_23_START (13)
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_SID_CFG_sid0_23_vpwr_last_page_sid0_23_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int reserved : 2;
        unsigned int vpwr_address_frame_start_sid0_23 : 30;
    } reg;
} SOC_IPP_CVDR_CVDR_IPP_VP_WR_SID_AXI_FS_sid0_23_UNION;
#endif
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_SID_AXI_FS_sid0_23_vpwr_address_frame_start_sid0_23_START (2)
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_SID_AXI_FS_sid0_23_vpwr_address_frame_start_sid0_23_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int vp_wr_debug_sid0_23 : 32;
    } reg;
} SOC_IPP_CVDR_CVDR_IPP_VP_WR_SID_DEBUG_sid0_23_UNION;
#endif
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_SID_DEBUG_sid0_23_vp_wr_debug_sid0_23_START (0)
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_SID_DEBUG_sid0_23_vp_wr_debug_sid0_23_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int reserved_0 : 16;
        unsigned int vp_wr_stop_enable_du_threshold_reached_24 : 1;
        unsigned int vp_wr_stop_enable_flux_ctrl_24 : 1;
        unsigned int vp_wr_stop_enable_pressure_24 : 1;
        unsigned int reserved_1 : 5;
        unsigned int vp_wr_stop_ok_24 : 1;
        unsigned int vp_wr_stop_24 : 1;
        unsigned int reserved_2 : 5;
        unsigned int vpwr_prefetch_bypass_24 : 1;
    } reg;
} SOC_IPP_CVDR_CVDR_IPP_VP_WR_IF_CFG_24_UNION;
#endif
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_IF_CFG_24_vp_wr_stop_enable_du_threshold_reached_24_START (16)
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_IF_CFG_24_vp_wr_stop_enable_du_threshold_reached_24_END (16)
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_IF_CFG_24_vp_wr_stop_enable_flux_ctrl_24_START (17)
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_IF_CFG_24_vp_wr_stop_enable_flux_ctrl_24_END (17)
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_IF_CFG_24_vp_wr_stop_enable_pressure_24_START (18)
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_IF_CFG_24_vp_wr_stop_enable_pressure_24_END (18)
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_IF_CFG_24_vp_wr_stop_ok_24_START (24)
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_IF_CFG_24_vp_wr_stop_ok_24_END (24)
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_IF_CFG_24_vp_wr_stop_24_START (25)
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_IF_CFG_24_vp_wr_stop_24_END (25)
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_IF_CFG_24_vpwr_prefetch_bypass_24_START (31)
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_IF_CFG_24_vpwr_prefetch_bypass_24_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int vpwr_access_limiter_0_24 : 4;
        unsigned int vpwr_access_limiter_1_24 : 4;
        unsigned int vpwr_access_limiter_2_24 : 4;
        unsigned int vpwr_access_limiter_3_24 : 4;
        unsigned int reserved_0 : 8;
        unsigned int vpwr_access_limiter_reload_24 : 4;
        unsigned int reserved_1 : 4;
    } reg;
} SOC_IPP_CVDR_CVDR_IPP_VP_WR_LIMITER_24_UNION;
#endif
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_LIMITER_24_vpwr_access_limiter_0_24_START (0)
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_LIMITER_24_vpwr_access_limiter_0_24_END (3)
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_LIMITER_24_vpwr_access_limiter_1_24_START (4)
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_LIMITER_24_vpwr_access_limiter_1_24_END (7)
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_LIMITER_24_vpwr_access_limiter_2_24_START (8)
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_LIMITER_24_vpwr_access_limiter_2_24_END (11)
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_LIMITER_24_vpwr_access_limiter_3_24_START (12)
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_LIMITER_24_vpwr_access_limiter_3_24_END (15)
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_LIMITER_24_vpwr_access_limiter_reload_24_START (24)
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_LIMITER_24_vpwr_access_limiter_reload_24_END (27)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int vpwr_line_stride_sid0_24 : 14;
        unsigned int vpwr_line_start_wstrb_sid0_24 : 4;
        unsigned int vpwr_line_wrap_sid0_24 : 14;
    } reg;
} SOC_IPP_CVDR_CVDR_IPP_VP_WR_SID_AXI_LINE_sid0_24_UNION;
#endif
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_SID_AXI_LINE_sid0_24_vpwr_line_stride_sid0_24_START (0)
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_SID_AXI_LINE_sid0_24_vpwr_line_stride_sid0_24_END (13)
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_SID_AXI_LINE_sid0_24_vpwr_line_start_wstrb_sid0_24_START (14)
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_SID_AXI_LINE_sid0_24_vpwr_line_start_wstrb_sid0_24_END (17)
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_SID_AXI_LINE_sid0_24_vpwr_line_wrap_sid0_24_START (18)
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_SID_AXI_LINE_sid0_24_vpwr_line_wrap_sid0_24_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int vp_wr_stripe_size_bytes_sid0_24 : 16;
        unsigned int reserved : 16;
    } reg;
} SOC_IPP_CVDR_CVDR_IPP_VP_WR_SID_PREFETCH_sid0_24_UNION;
#endif
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_SID_PREFETCH_sid0_24_vp_wr_stripe_size_bytes_sid0_24_START (0)
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_SID_PREFETCH_sid0_24_vp_wr_stripe_size_bytes_sid0_24_END (15)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int vpwr_pixel_format_sid0_24 : 5;
        unsigned int vpwr_pixel_expansion_sid0_24 : 1;
        unsigned int vpwr_4pf_enable_sid0_24 : 1;
        unsigned int reserved : 6;
        unsigned int vpwr_last_page_sid0_24 : 19;
    } reg;
} SOC_IPP_CVDR_CVDR_IPP_VP_WR_SID_CFG_sid0_24_UNION;
#endif
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_SID_CFG_sid0_24_vpwr_pixel_format_sid0_24_START (0)
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_SID_CFG_sid0_24_vpwr_pixel_format_sid0_24_END (4)
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_SID_CFG_sid0_24_vpwr_pixel_expansion_sid0_24_START (5)
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_SID_CFG_sid0_24_vpwr_pixel_expansion_sid0_24_END (5)
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_SID_CFG_sid0_24_vpwr_4pf_enable_sid0_24_START (6)
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_SID_CFG_sid0_24_vpwr_4pf_enable_sid0_24_END (6)
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_SID_CFG_sid0_24_vpwr_last_page_sid0_24_START (13)
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_SID_CFG_sid0_24_vpwr_last_page_sid0_24_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int reserved : 2;
        unsigned int vpwr_address_frame_start_sid0_24 : 30;
    } reg;
} SOC_IPP_CVDR_CVDR_IPP_VP_WR_SID_AXI_FS_sid0_24_UNION;
#endif
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_SID_AXI_FS_sid0_24_vpwr_address_frame_start_sid0_24_START (2)
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_SID_AXI_FS_sid0_24_vpwr_address_frame_start_sid0_24_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int vp_wr_debug_sid0_24 : 32;
    } reg;
} SOC_IPP_CVDR_CVDR_IPP_VP_WR_SID_DEBUG_sid0_24_UNION;
#endif
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_SID_DEBUG_sid0_24_vp_wr_debug_sid0_24_START (0)
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_SID_DEBUG_sid0_24_vp_wr_debug_sid0_24_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int reserved_0 : 16;
        unsigned int vp_wr_stop_enable_du_threshold_reached_25 : 1;
        unsigned int vp_wr_stop_enable_flux_ctrl_25 : 1;
        unsigned int vp_wr_stop_enable_pressure_25 : 1;
        unsigned int reserved_1 : 5;
        unsigned int vp_wr_stop_ok_25 : 1;
        unsigned int vp_wr_stop_25 : 1;
        unsigned int reserved_2 : 5;
        unsigned int vpwr_prefetch_bypass_25 : 1;
    } reg;
} SOC_IPP_CVDR_CVDR_IPP_VP_WR_IF_CFG_25_UNION;
#endif
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_IF_CFG_25_vp_wr_stop_enable_du_threshold_reached_25_START (16)
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_IF_CFG_25_vp_wr_stop_enable_du_threshold_reached_25_END (16)
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_IF_CFG_25_vp_wr_stop_enable_flux_ctrl_25_START (17)
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_IF_CFG_25_vp_wr_stop_enable_flux_ctrl_25_END (17)
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_IF_CFG_25_vp_wr_stop_enable_pressure_25_START (18)
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_IF_CFG_25_vp_wr_stop_enable_pressure_25_END (18)
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_IF_CFG_25_vp_wr_stop_ok_25_START (24)
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_IF_CFG_25_vp_wr_stop_ok_25_END (24)
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_IF_CFG_25_vp_wr_stop_25_START (25)
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_IF_CFG_25_vp_wr_stop_25_END (25)
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_IF_CFG_25_vpwr_prefetch_bypass_25_START (31)
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_IF_CFG_25_vpwr_prefetch_bypass_25_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int vpwr_access_limiter_0_25 : 4;
        unsigned int vpwr_access_limiter_1_25 : 4;
        unsigned int vpwr_access_limiter_2_25 : 4;
        unsigned int vpwr_access_limiter_3_25 : 4;
        unsigned int reserved_0 : 8;
        unsigned int vpwr_access_limiter_reload_25 : 4;
        unsigned int reserved_1 : 4;
    } reg;
} SOC_IPP_CVDR_CVDR_IPP_VP_WR_LIMITER_25_UNION;
#endif
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_LIMITER_25_vpwr_access_limiter_0_25_START (0)
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_LIMITER_25_vpwr_access_limiter_0_25_END (3)
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_LIMITER_25_vpwr_access_limiter_1_25_START (4)
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_LIMITER_25_vpwr_access_limiter_1_25_END (7)
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_LIMITER_25_vpwr_access_limiter_2_25_START (8)
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_LIMITER_25_vpwr_access_limiter_2_25_END (11)
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_LIMITER_25_vpwr_access_limiter_3_25_START (12)
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_LIMITER_25_vpwr_access_limiter_3_25_END (15)
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_LIMITER_25_vpwr_access_limiter_reload_25_START (24)
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_LIMITER_25_vpwr_access_limiter_reload_25_END (27)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int vpwr_line_stride_sid0_25 : 14;
        unsigned int vpwr_line_start_wstrb_sid0_25 : 4;
        unsigned int vpwr_line_wrap_sid0_25 : 14;
    } reg;
} SOC_IPP_CVDR_CVDR_IPP_VP_WR_SID_AXI_LINE_sid0_25_UNION;
#endif
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_SID_AXI_LINE_sid0_25_vpwr_line_stride_sid0_25_START (0)
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_SID_AXI_LINE_sid0_25_vpwr_line_stride_sid0_25_END (13)
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_SID_AXI_LINE_sid0_25_vpwr_line_start_wstrb_sid0_25_START (14)
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_SID_AXI_LINE_sid0_25_vpwr_line_start_wstrb_sid0_25_END (17)
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_SID_AXI_LINE_sid0_25_vpwr_line_wrap_sid0_25_START (18)
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_SID_AXI_LINE_sid0_25_vpwr_line_wrap_sid0_25_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int vp_wr_stripe_size_bytes_sid0_25 : 16;
        unsigned int reserved : 16;
    } reg;
} SOC_IPP_CVDR_CVDR_IPP_VP_WR_SID_PREFETCH_sid0_25_UNION;
#endif
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_SID_PREFETCH_sid0_25_vp_wr_stripe_size_bytes_sid0_25_START (0)
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_SID_PREFETCH_sid0_25_vp_wr_stripe_size_bytes_sid0_25_END (15)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int vpwr_pixel_format_sid0_25 : 5;
        unsigned int vpwr_pixel_expansion_sid0_25 : 1;
        unsigned int vpwr_4pf_enable_sid0_25 : 1;
        unsigned int reserved : 6;
        unsigned int vpwr_last_page_sid0_25 : 19;
    } reg;
} SOC_IPP_CVDR_CVDR_IPP_VP_WR_SID_CFG_sid0_25_UNION;
#endif
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_SID_CFG_sid0_25_vpwr_pixel_format_sid0_25_START (0)
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_SID_CFG_sid0_25_vpwr_pixel_format_sid0_25_END (4)
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_SID_CFG_sid0_25_vpwr_pixel_expansion_sid0_25_START (5)
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_SID_CFG_sid0_25_vpwr_pixel_expansion_sid0_25_END (5)
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_SID_CFG_sid0_25_vpwr_4pf_enable_sid0_25_START (6)
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_SID_CFG_sid0_25_vpwr_4pf_enable_sid0_25_END (6)
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_SID_CFG_sid0_25_vpwr_last_page_sid0_25_START (13)
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_SID_CFG_sid0_25_vpwr_last_page_sid0_25_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int reserved : 2;
        unsigned int vpwr_address_frame_start_sid0_25 : 30;
    } reg;
} SOC_IPP_CVDR_CVDR_IPP_VP_WR_SID_AXI_FS_sid0_25_UNION;
#endif
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_SID_AXI_FS_sid0_25_vpwr_address_frame_start_sid0_25_START (2)
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_SID_AXI_FS_sid0_25_vpwr_address_frame_start_sid0_25_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int vp_wr_debug_sid0_25 : 32;
    } reg;
} SOC_IPP_CVDR_CVDR_IPP_VP_WR_SID_DEBUG_sid0_25_UNION;
#endif
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_SID_DEBUG_sid0_25_vp_wr_debug_sid0_25_START (0)
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_SID_DEBUG_sid0_25_vp_wr_debug_sid0_25_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int reserved_0 : 16;
        unsigned int vp_wr_stop_enable_du_threshold_reached_30 : 1;
        unsigned int vp_wr_stop_enable_flux_ctrl_30 : 1;
        unsigned int vp_wr_stop_enable_pressure_30 : 1;
        unsigned int reserved_1 : 5;
        unsigned int vp_wr_stop_ok_30 : 1;
        unsigned int vp_wr_stop_30 : 1;
        unsigned int reserved_2 : 5;
        unsigned int vpwr_prefetch_bypass_30 : 1;
    } reg;
} SOC_IPP_CVDR_CVDR_IPP_VP_WR_IF_CFG_30_UNION;
#endif
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_IF_CFG_30_vp_wr_stop_enable_du_threshold_reached_30_START (16)
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_IF_CFG_30_vp_wr_stop_enable_du_threshold_reached_30_END (16)
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_IF_CFG_30_vp_wr_stop_enable_flux_ctrl_30_START (17)
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_IF_CFG_30_vp_wr_stop_enable_flux_ctrl_30_END (17)
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_IF_CFG_30_vp_wr_stop_enable_pressure_30_START (18)
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_IF_CFG_30_vp_wr_stop_enable_pressure_30_END (18)
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_IF_CFG_30_vp_wr_stop_ok_30_START (24)
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_IF_CFG_30_vp_wr_stop_ok_30_END (24)
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_IF_CFG_30_vp_wr_stop_30_START (25)
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_IF_CFG_30_vp_wr_stop_30_END (25)
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_IF_CFG_30_vpwr_prefetch_bypass_30_START (31)
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_IF_CFG_30_vpwr_prefetch_bypass_30_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int vpwr_access_limiter_0_30 : 4;
        unsigned int vpwr_access_limiter_1_30 : 4;
        unsigned int vpwr_access_limiter_2_30 : 4;
        unsigned int vpwr_access_limiter_3_30 : 4;
        unsigned int reserved_0 : 8;
        unsigned int vpwr_access_limiter_reload_30 : 4;
        unsigned int reserved_1 : 4;
    } reg;
} SOC_IPP_CVDR_CVDR_IPP_VP_WR_LIMITER_30_UNION;
#endif
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_LIMITER_30_vpwr_access_limiter_0_30_START (0)
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_LIMITER_30_vpwr_access_limiter_0_30_END (3)
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_LIMITER_30_vpwr_access_limiter_1_30_START (4)
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_LIMITER_30_vpwr_access_limiter_1_30_END (7)
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_LIMITER_30_vpwr_access_limiter_2_30_START (8)
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_LIMITER_30_vpwr_access_limiter_2_30_END (11)
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_LIMITER_30_vpwr_access_limiter_3_30_START (12)
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_LIMITER_30_vpwr_access_limiter_3_30_END (15)
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_LIMITER_30_vpwr_access_limiter_reload_30_START (24)
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_LIMITER_30_vpwr_access_limiter_reload_30_END (27)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int vpwr_line_stride_sid0_30 : 14;
        unsigned int vpwr_line_start_wstrb_sid0_30 : 4;
        unsigned int vpwr_line_wrap_sid0_30 : 14;
    } reg;
} SOC_IPP_CVDR_CVDR_IPP_VP_WR_SID_AXI_LINE_sid0_30_UNION;
#endif
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_SID_AXI_LINE_sid0_30_vpwr_line_stride_sid0_30_START (0)
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_SID_AXI_LINE_sid0_30_vpwr_line_stride_sid0_30_END (13)
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_SID_AXI_LINE_sid0_30_vpwr_line_start_wstrb_sid0_30_START (14)
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_SID_AXI_LINE_sid0_30_vpwr_line_start_wstrb_sid0_30_END (17)
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_SID_AXI_LINE_sid0_30_vpwr_line_wrap_sid0_30_START (18)
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_SID_AXI_LINE_sid0_30_vpwr_line_wrap_sid0_30_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int vp_wr_stripe_size_bytes_sid0_30 : 16;
        unsigned int reserved : 16;
    } reg;
} SOC_IPP_CVDR_CVDR_IPP_VP_WR_SID_PREFETCH_sid0_30_UNION;
#endif
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_SID_PREFETCH_sid0_30_vp_wr_stripe_size_bytes_sid0_30_START (0)
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_SID_PREFETCH_sid0_30_vp_wr_stripe_size_bytes_sid0_30_END (15)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int vpwr_pixel_format_sid0_30 : 5;
        unsigned int vpwr_pixel_expansion_sid0_30 : 1;
        unsigned int vpwr_4pf_enable_sid0_30 : 1;
        unsigned int reserved : 6;
        unsigned int vpwr_last_page_sid0_30 : 19;
    } reg;
} SOC_IPP_CVDR_CVDR_IPP_VP_WR_SID_CFG_sid0_30_UNION;
#endif
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_SID_CFG_sid0_30_vpwr_pixel_format_sid0_30_START (0)
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_SID_CFG_sid0_30_vpwr_pixel_format_sid0_30_END (4)
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_SID_CFG_sid0_30_vpwr_pixel_expansion_sid0_30_START (5)
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_SID_CFG_sid0_30_vpwr_pixel_expansion_sid0_30_END (5)
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_SID_CFG_sid0_30_vpwr_4pf_enable_sid0_30_START (6)
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_SID_CFG_sid0_30_vpwr_4pf_enable_sid0_30_END (6)
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_SID_CFG_sid0_30_vpwr_last_page_sid0_30_START (13)
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_SID_CFG_sid0_30_vpwr_last_page_sid0_30_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int reserved : 2;
        unsigned int vpwr_address_frame_start_sid0_30 : 30;
    } reg;
} SOC_IPP_CVDR_CVDR_IPP_VP_WR_SID_AXI_FS_sid0_30_UNION;
#endif
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_SID_AXI_FS_sid0_30_vpwr_address_frame_start_sid0_30_START (2)
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_SID_AXI_FS_sid0_30_vpwr_address_frame_start_sid0_30_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int vp_wr_debug_sid0_30 : 32;
    } reg;
} SOC_IPP_CVDR_CVDR_IPP_VP_WR_SID_DEBUG_sid0_30_UNION;
#endif
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_SID_DEBUG_sid0_30_vp_wr_debug_sid0_30_START (0)
#define SOC_IPP_CVDR_CVDR_IPP_VP_WR_SID_DEBUG_sid0_30_vp_wr_debug_sid0_30_END (31)
#ifdef __cplusplus
    #if __cplusplus
        }
    #endif
#endif
#endif
