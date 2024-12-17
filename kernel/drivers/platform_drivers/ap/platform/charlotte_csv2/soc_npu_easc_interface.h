#ifndef __SOC_NPU_EASC_INTERFACE_H__
#define __SOC_NPU_EASC_INTERFACE_H__ 
#ifdef __cplusplus
#if __cplusplus
    extern "C" {
#endif
#endif
#ifndef __SOC_H_FOR_ASM__
#define SOC_NPU_EASC_REGION0_SECURE_ADDR(base) ((base) + (0x00UL))
#define SOC_NPU_EASC_REGION0_START_ADDR_LOW_ADDR(base) ((base) + (0x04UL))
#define SOC_NPU_EASC_REGION0_START_ADDR_HIGH_ADDR(base) ((base) + (0x08UL))
#define SOC_NPU_EASC_REGION0_END_ADDR_LOW_ADDR(base) ((base) + (0x0cUL))
#define SOC_NPU_EASC_REGION0_END_ADDR_HIGH_ADDR(base) ((base) + (0x10UL))
#define SOC_NPU_EASC_REGION0_WR_RD_TYPE_ADDR(base) ((base) + (0x14UL))
#define SOC_NPU_EASC_REGION0_CFG_STATE_ADDR(base) ((base) + (0x18UL))
#define SOC_NPU_EASC_REGION0_SETTING_LOCK_ADDR(base) ((base) + (0x1cUL))
#define SOC_NPU_EASC_REGION0_ID_3_ADDR(base) ((base) + (0x20UL))
#define SOC_NPU_EASC_REGION1_SECURE_ADDR(base) ((base) + (0x24UL))
#define SOC_NPU_EASC_REGION1_START_ADDR_LOW_ADDR(base) ((base) + (0x28UL))
#define SOC_NPU_EASC_REGION1_START_ADDR_HIGH_ADDR(base) ((base) + (0x2cUL))
#define SOC_NPU_EASC_REGION1_END_ADDR_LOW_ADDR(base) ((base) + (0x30UL))
#define SOC_NPU_EASC_REGION1_END_ADDR_HIGH_ADDR(base) ((base) + (0x34UL))
#define SOC_NPU_EASC_REGION1_WR_RD_TYPE_ADDR(base) ((base) + (0x38UL))
#define SOC_NPU_EASC_REGION1_CFG_STATE_ADDR(base) ((base) + (0x3cUL))
#define SOC_NPU_EASC_REGION1_SETTING_LOCK_ADDR(base) ((base) + (0x40UL))
#define SOC_NPU_EASC_REGION1_ID_3_ADDR(base) ((base) + (0x44UL))
#define SOC_NPU_EASC_REGION2_SECURE_ADDR(base) ((base) + (0x48UL))
#define SOC_NPU_EASC_REGION2_START_ADDR_LOW_ADDR(base) ((base) + (0x04cUL))
#define SOC_NPU_EASC_REGION2_START_ADDR_HIGH_ADDR(base) ((base) + (0x50UL))
#define SOC_NPU_EASC_REGION2_END_ADDR_LOW_ADDR(base) ((base) + (0x54UL))
#define SOC_NPU_EASC_REGION2_END_ADDR_HIGH_ADDR(base) ((base) + (0x58UL))
#define SOC_NPU_EASC_REGION2_WR_RD_TYPE_ADDR(base) ((base) + (0x5cUL))
#define SOC_NPU_EASC_REGION2_CFG_STATE_ADDR(base) ((base) + (0x60UL))
#define SOC_NPU_EASC_REGION2_SETTING_LOCK_ADDR(base) ((base) + (0x64UL))
#define SOC_NPU_EASC_REGION2_ID_3_ADDR(base) ((base) + (0x68UL))
#define SOC_NPU_EASC_REGION3_SECURE_ADDR(base) ((base) + (0x6cUL))
#define SOC_NPU_EASC_REGION3_START_ADDR_LOW_ADDR(base) ((base) + (0x70UL))
#define SOC_NPU_EASC_REGION3_START_ADDR_HIGH_ADDR(base) ((base) + (0x74UL))
#define SOC_NPU_EASC_REGION3_END_ADDR_LOW_ADDR(base) ((base) + (0x78UL))
#define SOC_NPU_EASC_REGION3_END_ADDR_HIGH_ADDR(base) ((base) + (0x7cUL))
#define SOC_NPU_EASC_REGION3_WR_RD_TYPE_ADDR(base) ((base) + (0x80UL))
#define SOC_NPU_EASC_REGION3_CFG_STATE_ADDR(base) ((base) + (0x84UL))
#define SOC_NPU_EASC_REGION3_SETTING_LOCK_ADDR(base) ((base) + (0x88UL))
#define SOC_NPU_EASC_REGION3_ID_3_ADDR(base) ((base) + (0x8cUL))
#define SOC_NPU_EASC_REGION4_SECURE_ADDR(base) ((base) + (0x90UL))
#define SOC_NPU_EASC_REGION4_START_ADDR_LOW_ADDR(base) ((base) + (0x94UL))
#define SOC_NPU_EASC_REGION4_START_ADDR_HIGH_ADDR(base) ((base) + (0x98UL))
#define SOC_NPU_EASC_REGION4_END_ADDR_LOW_ADDR(base) ((base) + (0x9cUL))
#define SOC_NPU_EASC_REGION4_END_ADDR_HIGH_ADDR(base) ((base) + (0xa0UL))
#define SOC_NPU_EASC_REGION4_WR_RD_TYPE_ADDR(base) ((base) + (0xa4UL))
#define SOC_NPU_EASC_REGION4_CFG_STATE_ADDR(base) ((base) + (0xa8UL))
#define SOC_NPU_EASC_REGION4_SETTING_LOCK_ADDR(base) ((base) + (0xacUL))
#define SOC_NPU_EASC_REGION4_ID_3_ADDR(base) ((base) + (0xb0UL))
#define SOC_NPU_EASC_REGION5_SECURE_ADDR(base) ((base) + (0xb4UL))
#define SOC_NPU_EASC_REGION5_START_ADDR_LOW_ADDR(base) ((base) + (0xb8UL))
#define SOC_NPU_EASC_REGION5_START_ADDR_HIGH_ADDR(base) ((base) + (0xbcUL))
#define SOC_NPU_EASC_REGION5_END_ADDR_LOW_ADDR(base) ((base) + (0xc0UL))
#define SOC_NPU_EASC_REGION5_END_ADDR_HIGH_ADDR(base) ((base) + (0xc4UL))
#define SOC_NPU_EASC_REGION5_WR_RD_TYPE_ADDR(base) ((base) + (0xc8UL))
#define SOC_NPU_EASC_REGION5_CFG_STATE_ADDR(base) ((base) + (0xccUL))
#define SOC_NPU_EASC_REGION5_SETTING_LOCK_ADDR(base) ((base) + (0xd0UL))
#define SOC_NPU_EASC_REGION5_ID_3_ADDR(base) ((base) + (0xd4UL))
#define SOC_NPU_EASC_REGION6_SECURE_ADDR(base) ((base) + (0xd8UL))
#define SOC_NPU_EASC_REGION6_START_ADDR_LOW_ADDR(base) ((base) + (0xdcUL))
#define SOC_NPU_EASC_REGION6_START_ADDR_HIGH_ADDR(base) ((base) + (0xe0UL))
#define SOC_NPU_EASC_REGION6_END_ADDR_LOW_ADDR(base) ((base) + (0xe4UL))
#define SOC_NPU_EASC_REGION6_END_ADDR_HIGH_ADDR(base) ((base) + (0xe8UL))
#define SOC_NPU_EASC_REGION6_WR_RD_TYPE_ADDR(base) ((base) + (0xecUL))
#define SOC_NPU_EASC_REGION6_CFG_STATE_ADDR(base) ((base) + (0xf0UL))
#define SOC_NPU_EASC_REGION6_SETTING_LOCK_ADDR(base) ((base) + (0xf4UL))
#define SOC_NPU_EASC_REGION6_ID_3_ADDR(base) ((base) + (0xf8UL))
#define SOC_NPU_EASC_REGION7_SECURE_ADDR(base) ((base) + (0xfcUL))
#define SOC_NPU_EASC_REGION7_START_ADDR_LOW_ADDR(base) ((base) + (0x100UL))
#define SOC_NPU_EASC_REGION7_START_ADDR_HIGH_ADDR(base) ((base) + (0x104UL))
#define SOC_NPU_EASC_REGION7_END_ADDR_LOW_ADDR(base) ((base) + (0x108UL))
#define SOC_NPU_EASC_REGION7_END_ADDR_HIGH_ADDR(base) ((base) + (0x10cUL))
#define SOC_NPU_EASC_REGION7_WR_RD_TYPE_ADDR(base) ((base) + (0x110UL))
#define SOC_NPU_EASC_REGION7_CFG_STATE_ADDR(base) ((base) + (0x114UL))
#define SOC_NPU_EASC_REGION7_SETTING_LOCK_ADDR(base) ((base) + (0x118UL))
#define SOC_NPU_EASC_REGION7_ID_3_ADDR(base) ((base) + (0x11cUL))
#define SOC_NPU_EASC_REGION_SEC_DEFAULT_ADDR(base) ((base) + (0x300UL))
#define SOC_NPU_EASC_DEFAULT_SEC_LOCK_ADDR(base) ((base) + (0x304UL))
#define SOC_NPU_EASC_INT_STATUS0_ADDR(base) ((base) + (0x308UL))
#define SOC_NPU_EASC_INT_CLEAR_ADDR(base) ((base) + (0x30cUL))
#define SOC_NPU_EASC_FAIL_ADDR_LOW0_ADDR(base) ((base) + (0x310UL))
#define SOC_NPU_EASC_FAIL_ADDR_HIGH0_ADDR(base) ((base) + (0x314UL))
#define SOC_NPU_EASC_FAIL_ID0_ADDR(base) ((base) + (0x318UL))
#define SOC_NPU_EASC_FAIL_INFO0_ADDR(base) ((base) + (0x31cUL))
#define SOC_NPU_EASC_INT_STATUS1_ADDR(base) ((base) + (0x3a0UL))
#define SOC_NPU_EASC_FAIL_ADDR_LOW1_ADDR(base) ((base) + (0x3a4UL))
#define SOC_NPU_EASC_FAIL_ADDR_HIGH_ADDR(base) ((base) + (0x3a8UL))
#define SOC_NPU_EASC_FAIL_ID1_ADDR(base) ((base) + (0x3acUL))
#define SOC_NPU_EASC_FAIL_INFO1_ADDR(base) ((base) + (0x3b0UL))
#define SOC_NPU_EASC_SET_OK_REGION0_START_ADDR_LOW_ADDR(base) ((base) + (0x320UL))
#define SOC_NPU_EASC_SET_OK_REGION0_START_ADDR_HIGH_ADDR(base) ((base) + (0x324UL))
#define SOC_NPU_EASC_SET_OK_REGION0_END_ADDR_LOW_ADDR(base) ((base) + (0x328UL))
#define SOC_NPU_EASC_SET_OK_REGION0_END_ADDR_HIGH_ADDR(base) ((base) + (0x32cUL))
#define SOC_NPU_EASC_SET_OK_REGION1_START_ADDR_LOW_ADDR(base) ((base) + (0x330UL))
#define SOC_NPU_EASC_SET_OK_REGION1_START_ADDR_HIGH_ADDR(base) ((base) + (0x334UL))
#define SOC_NPU_EASC_SET_OK_REGION1_END_ADDR_LOW_ADDR(base) ((base) + (0x338UL))
#define SOC_NPU_EASC_SET_OK_REGION1_END_ADDR_HIGH_ADDR(base) ((base) + (0x33cUL))
#define SOC_NPU_EASC_SET_OK_REGION2_START_ADDR_LOW_ADDR(base) ((base) + (0x340UL))
#define SOC_NPU_EASC_SET_OK_REGION2_START_ADDR_HIGH_ADDR(base) ((base) + (0x0344UL))
#define SOC_NPU_EASC_SET_OK_REGION2_END_ADDR_LOW_ADDR(base) ((base) + (0x348UL))
#define SOC_NPU_EASC_SET_OK_REGION2_END_ADDR_HIGH_ADDR(base) ((base) + (0x34cUL))
#define SOC_NPU_EASC_SET_OK_REGION3_START_ADDR_LOW_ADDR(base) ((base) + (0x350UL))
#define SOC_NPU_EASC_SET_OK_REGION3_START_ADDR_HIGH_ADDR(base) ((base) + (0x354UL))
#define SOC_NPU_EASC_SET_OK_REGION3_END_ADDR_LOW_ADDR(base) ((base) + (0x358UL))
#define SOC_NPU_EASC_SET_OK_REGION3_END_ADDR_HIGH_ADDR(base) ((base) + (0x35cUL))
#define SOC_NPU_EASC_SET_OK_REGION4_START_ADDR_LOW_ADDR(base) ((base) + (0x360UL))
#define SOC_NPU_EASC_SET_OK_REGION4_START_ADDR_HIGH_ADDR(base) ((base) + (0x364UL))
#define SOC_NPU_EASC_SET_OK_REGION4_END_ADDR_LOW_ADDR(base) ((base) + (0x368UL))
#define SOC_NPU_EASC_SET_OK_REGION4_END_ADDR_HIGH_ADDR(base) ((base) + (0x36cUL))
#define SOC_NPU_EASC_SET_OK_REGION5_START_ADDR_LOW_ADDR(base) ((base) + (0x370UL))
#define SOC_NPU_EASC_SET_OK_REGION5_START_ADDR_HIGH_ADDR(base) ((base) + (0x374UL))
#define SOC_NPU_EASC_SET_OK_REGION5_END_ADDR_LOW_ADDR(base) ((base) + (0x378UL))
#define SOC_NPU_EASC_SET_OK_REGION5_END_ADDR_HIGH_ADDR(base) ((base) + (0x37cUL))
#define SOC_NPU_EASC_SET_OK_REGION6_START_ADDR_LOW_ADDR(base) ((base) + (0x380UL))
#define SOC_NPU_EASC_SET_OK_REGION6_START_ADDR_HIGH_ADDR(base) ((base) + (0x384UL))
#define SOC_NPU_EASC_SET_OK_REGION6_END_ADDR_LOW_ADDR(base) ((base) + (0x388UL))
#define SOC_NPU_EASC_SET_OK_REGION6_END_ADDR_HIGH_ADDR(base) ((base) + (0x38cUL))
#define SOC_NPU_EASC_SET_OK_REGION7_START_ADDR_LOW_ADDR(base) ((base) + (0x390UL))
#define SOC_NPU_EASC_SET_OK_REGION7_START_ADDR_HIGH_ADDR(base) ((base) + (0x394UL))
#define SOC_NPU_EASC_SET_OK_REGION7_END_ADDR_LOW_ADDR(base) ((base) + (0x398UL))
#define SOC_NPU_EASC_SET_OK_REGION7_END_ADDR_HIGH_ADDR(base) ((base) + (0x39cUL))
#define SOC_NPU_EASC_MINIBUS_CFG_ADDR(base) ((base) + (0x3f0UL))
#define SOC_NPU_EASC_RSV1_ADDR(base) ((base) + (0x3f4UL))
#define SOC_NPU_EASC_RSV2_ADDR(base) ((base) + (0x3f8UL))
#define SOC_NPU_EASC_RSV3_ADDR(base) ((base) + (0x3fcUL))
#define SOC_NPU_EASC_DW_1TO2_DEBUG_ADDR(base) ((base) + (0x400UL))
#else
#define SOC_NPU_EASC_REGION0_SECURE_ADDR(base) ((base) + (0x00))
#define SOC_NPU_EASC_REGION0_START_ADDR_LOW_ADDR(base) ((base) + (0x04))
#define SOC_NPU_EASC_REGION0_START_ADDR_HIGH_ADDR(base) ((base) + (0x08))
#define SOC_NPU_EASC_REGION0_END_ADDR_LOW_ADDR(base) ((base) + (0x0c))
#define SOC_NPU_EASC_REGION0_END_ADDR_HIGH_ADDR(base) ((base) + (0x10))
#define SOC_NPU_EASC_REGION0_WR_RD_TYPE_ADDR(base) ((base) + (0x14))
#define SOC_NPU_EASC_REGION0_CFG_STATE_ADDR(base) ((base) + (0x18))
#define SOC_NPU_EASC_REGION0_SETTING_LOCK_ADDR(base) ((base) + (0x1c))
#define SOC_NPU_EASC_REGION0_ID_3_ADDR(base) ((base) + (0x20))
#define SOC_NPU_EASC_REGION1_SECURE_ADDR(base) ((base) + (0x24))
#define SOC_NPU_EASC_REGION1_START_ADDR_LOW_ADDR(base) ((base) + (0x28))
#define SOC_NPU_EASC_REGION1_START_ADDR_HIGH_ADDR(base) ((base) + (0x2c))
#define SOC_NPU_EASC_REGION1_END_ADDR_LOW_ADDR(base) ((base) + (0x30))
#define SOC_NPU_EASC_REGION1_END_ADDR_HIGH_ADDR(base) ((base) + (0x34))
#define SOC_NPU_EASC_REGION1_WR_RD_TYPE_ADDR(base) ((base) + (0x38))
#define SOC_NPU_EASC_REGION1_CFG_STATE_ADDR(base) ((base) + (0x3c))
#define SOC_NPU_EASC_REGION1_SETTING_LOCK_ADDR(base) ((base) + (0x40))
#define SOC_NPU_EASC_REGION1_ID_3_ADDR(base) ((base) + (0x44))
#define SOC_NPU_EASC_REGION2_SECURE_ADDR(base) ((base) + (0x48))
#define SOC_NPU_EASC_REGION2_START_ADDR_LOW_ADDR(base) ((base) + (0x04c))
#define SOC_NPU_EASC_REGION2_START_ADDR_HIGH_ADDR(base) ((base) + (0x50))
#define SOC_NPU_EASC_REGION2_END_ADDR_LOW_ADDR(base) ((base) + (0x54))
#define SOC_NPU_EASC_REGION2_END_ADDR_HIGH_ADDR(base) ((base) + (0x58))
#define SOC_NPU_EASC_REGION2_WR_RD_TYPE_ADDR(base) ((base) + (0x5c))
#define SOC_NPU_EASC_REGION2_CFG_STATE_ADDR(base) ((base) + (0x60))
#define SOC_NPU_EASC_REGION2_SETTING_LOCK_ADDR(base) ((base) + (0x64))
#define SOC_NPU_EASC_REGION2_ID_3_ADDR(base) ((base) + (0x68))
#define SOC_NPU_EASC_REGION3_SECURE_ADDR(base) ((base) + (0x6c))
#define SOC_NPU_EASC_REGION3_START_ADDR_LOW_ADDR(base) ((base) + (0x70))
#define SOC_NPU_EASC_REGION3_START_ADDR_HIGH_ADDR(base) ((base) + (0x74))
#define SOC_NPU_EASC_REGION3_END_ADDR_LOW_ADDR(base) ((base) + (0x78))
#define SOC_NPU_EASC_REGION3_END_ADDR_HIGH_ADDR(base) ((base) + (0x7c))
#define SOC_NPU_EASC_REGION3_WR_RD_TYPE_ADDR(base) ((base) + (0x80))
#define SOC_NPU_EASC_REGION3_CFG_STATE_ADDR(base) ((base) + (0x84))
#define SOC_NPU_EASC_REGION3_SETTING_LOCK_ADDR(base) ((base) + (0x88))
#define SOC_NPU_EASC_REGION3_ID_3_ADDR(base) ((base) + (0x8c))
#define SOC_NPU_EASC_REGION4_SECURE_ADDR(base) ((base) + (0x90))
#define SOC_NPU_EASC_REGION4_START_ADDR_LOW_ADDR(base) ((base) + (0x94))
#define SOC_NPU_EASC_REGION4_START_ADDR_HIGH_ADDR(base) ((base) + (0x98))
#define SOC_NPU_EASC_REGION4_END_ADDR_LOW_ADDR(base) ((base) + (0x9c))
#define SOC_NPU_EASC_REGION4_END_ADDR_HIGH_ADDR(base) ((base) + (0xa0))
#define SOC_NPU_EASC_REGION4_WR_RD_TYPE_ADDR(base) ((base) + (0xa4))
#define SOC_NPU_EASC_REGION4_CFG_STATE_ADDR(base) ((base) + (0xa8))
#define SOC_NPU_EASC_REGION4_SETTING_LOCK_ADDR(base) ((base) + (0xac))
#define SOC_NPU_EASC_REGION4_ID_3_ADDR(base) ((base) + (0xb0))
#define SOC_NPU_EASC_REGION5_SECURE_ADDR(base) ((base) + (0xb4))
#define SOC_NPU_EASC_REGION5_START_ADDR_LOW_ADDR(base) ((base) + (0xb8))
#define SOC_NPU_EASC_REGION5_START_ADDR_HIGH_ADDR(base) ((base) + (0xbc))
#define SOC_NPU_EASC_REGION5_END_ADDR_LOW_ADDR(base) ((base) + (0xc0))
#define SOC_NPU_EASC_REGION5_END_ADDR_HIGH_ADDR(base) ((base) + (0xc4))
#define SOC_NPU_EASC_REGION5_WR_RD_TYPE_ADDR(base) ((base) + (0xc8))
#define SOC_NPU_EASC_REGION5_CFG_STATE_ADDR(base) ((base) + (0xcc))
#define SOC_NPU_EASC_REGION5_SETTING_LOCK_ADDR(base) ((base) + (0xd0))
#define SOC_NPU_EASC_REGION5_ID_3_ADDR(base) ((base) + (0xd4))
#define SOC_NPU_EASC_REGION6_SECURE_ADDR(base) ((base) + (0xd8))
#define SOC_NPU_EASC_REGION6_START_ADDR_LOW_ADDR(base) ((base) + (0xdc))
#define SOC_NPU_EASC_REGION6_START_ADDR_HIGH_ADDR(base) ((base) + (0xe0))
#define SOC_NPU_EASC_REGION6_END_ADDR_LOW_ADDR(base) ((base) + (0xe4))
#define SOC_NPU_EASC_REGION6_END_ADDR_HIGH_ADDR(base) ((base) + (0xe8))
#define SOC_NPU_EASC_REGION6_WR_RD_TYPE_ADDR(base) ((base) + (0xec))
#define SOC_NPU_EASC_REGION6_CFG_STATE_ADDR(base) ((base) + (0xf0))
#define SOC_NPU_EASC_REGION6_SETTING_LOCK_ADDR(base) ((base) + (0xf4))
#define SOC_NPU_EASC_REGION6_ID_3_ADDR(base) ((base) + (0xf8))
#define SOC_NPU_EASC_REGION7_SECURE_ADDR(base) ((base) + (0xfc))
#define SOC_NPU_EASC_REGION7_START_ADDR_LOW_ADDR(base) ((base) + (0x100))
#define SOC_NPU_EASC_REGION7_START_ADDR_HIGH_ADDR(base) ((base) + (0x104))
#define SOC_NPU_EASC_REGION7_END_ADDR_LOW_ADDR(base) ((base) + (0x108))
#define SOC_NPU_EASC_REGION7_END_ADDR_HIGH_ADDR(base) ((base) + (0x10c))
#define SOC_NPU_EASC_REGION7_WR_RD_TYPE_ADDR(base) ((base) + (0x110))
#define SOC_NPU_EASC_REGION7_CFG_STATE_ADDR(base) ((base) + (0x114))
#define SOC_NPU_EASC_REGION7_SETTING_LOCK_ADDR(base) ((base) + (0x118))
#define SOC_NPU_EASC_REGION7_ID_3_ADDR(base) ((base) + (0x11c))
#define SOC_NPU_EASC_REGION_SEC_DEFAULT_ADDR(base) ((base) + (0x300))
#define SOC_NPU_EASC_DEFAULT_SEC_LOCK_ADDR(base) ((base) + (0x304))
#define SOC_NPU_EASC_INT_STATUS0_ADDR(base) ((base) + (0x308))
#define SOC_NPU_EASC_INT_CLEAR_ADDR(base) ((base) + (0x30c))
#define SOC_NPU_EASC_FAIL_ADDR_LOW0_ADDR(base) ((base) + (0x310))
#define SOC_NPU_EASC_FAIL_ADDR_HIGH0_ADDR(base) ((base) + (0x314))
#define SOC_NPU_EASC_FAIL_ID0_ADDR(base) ((base) + (0x318))
#define SOC_NPU_EASC_FAIL_INFO0_ADDR(base) ((base) + (0x31c))
#define SOC_NPU_EASC_INT_STATUS1_ADDR(base) ((base) + (0x3a0))
#define SOC_NPU_EASC_FAIL_ADDR_LOW1_ADDR(base) ((base) + (0x3a4))
#define SOC_NPU_EASC_FAIL_ADDR_HIGH_ADDR(base) ((base) + (0x3a8))
#define SOC_NPU_EASC_FAIL_ID1_ADDR(base) ((base) + (0x3ac))
#define SOC_NPU_EASC_FAIL_INFO1_ADDR(base) ((base) + (0x3b0))
#define SOC_NPU_EASC_SET_OK_REGION0_START_ADDR_LOW_ADDR(base) ((base) + (0x320))
#define SOC_NPU_EASC_SET_OK_REGION0_START_ADDR_HIGH_ADDR(base) ((base) + (0x324))
#define SOC_NPU_EASC_SET_OK_REGION0_END_ADDR_LOW_ADDR(base) ((base) + (0x328))
#define SOC_NPU_EASC_SET_OK_REGION0_END_ADDR_HIGH_ADDR(base) ((base) + (0x32c))
#define SOC_NPU_EASC_SET_OK_REGION1_START_ADDR_LOW_ADDR(base) ((base) + (0x330))
#define SOC_NPU_EASC_SET_OK_REGION1_START_ADDR_HIGH_ADDR(base) ((base) + (0x334))
#define SOC_NPU_EASC_SET_OK_REGION1_END_ADDR_LOW_ADDR(base) ((base) + (0x338))
#define SOC_NPU_EASC_SET_OK_REGION1_END_ADDR_HIGH_ADDR(base) ((base) + (0x33c))
#define SOC_NPU_EASC_SET_OK_REGION2_START_ADDR_LOW_ADDR(base) ((base) + (0x340))
#define SOC_NPU_EASC_SET_OK_REGION2_START_ADDR_HIGH_ADDR(base) ((base) + (0x0344))
#define SOC_NPU_EASC_SET_OK_REGION2_END_ADDR_LOW_ADDR(base) ((base) + (0x348))
#define SOC_NPU_EASC_SET_OK_REGION2_END_ADDR_HIGH_ADDR(base) ((base) + (0x34c))
#define SOC_NPU_EASC_SET_OK_REGION3_START_ADDR_LOW_ADDR(base) ((base) + (0x350))
#define SOC_NPU_EASC_SET_OK_REGION3_START_ADDR_HIGH_ADDR(base) ((base) + (0x354))
#define SOC_NPU_EASC_SET_OK_REGION3_END_ADDR_LOW_ADDR(base) ((base) + (0x358))
#define SOC_NPU_EASC_SET_OK_REGION3_END_ADDR_HIGH_ADDR(base) ((base) + (0x35c))
#define SOC_NPU_EASC_SET_OK_REGION4_START_ADDR_LOW_ADDR(base) ((base) + (0x360))
#define SOC_NPU_EASC_SET_OK_REGION4_START_ADDR_HIGH_ADDR(base) ((base) + (0x364))
#define SOC_NPU_EASC_SET_OK_REGION4_END_ADDR_LOW_ADDR(base) ((base) + (0x368))
#define SOC_NPU_EASC_SET_OK_REGION4_END_ADDR_HIGH_ADDR(base) ((base) + (0x36c))
#define SOC_NPU_EASC_SET_OK_REGION5_START_ADDR_LOW_ADDR(base) ((base) + (0x370))
#define SOC_NPU_EASC_SET_OK_REGION5_START_ADDR_HIGH_ADDR(base) ((base) + (0x374))
#define SOC_NPU_EASC_SET_OK_REGION5_END_ADDR_LOW_ADDR(base) ((base) + (0x378))
#define SOC_NPU_EASC_SET_OK_REGION5_END_ADDR_HIGH_ADDR(base) ((base) + (0x37c))
#define SOC_NPU_EASC_SET_OK_REGION6_START_ADDR_LOW_ADDR(base) ((base) + (0x380))
#define SOC_NPU_EASC_SET_OK_REGION6_START_ADDR_HIGH_ADDR(base) ((base) + (0x384))
#define SOC_NPU_EASC_SET_OK_REGION6_END_ADDR_LOW_ADDR(base) ((base) + (0x388))
#define SOC_NPU_EASC_SET_OK_REGION6_END_ADDR_HIGH_ADDR(base) ((base) + (0x38c))
#define SOC_NPU_EASC_SET_OK_REGION7_START_ADDR_LOW_ADDR(base) ((base) + (0x390))
#define SOC_NPU_EASC_SET_OK_REGION7_START_ADDR_HIGH_ADDR(base) ((base) + (0x394))
#define SOC_NPU_EASC_SET_OK_REGION7_END_ADDR_LOW_ADDR(base) ((base) + (0x398))
#define SOC_NPU_EASC_SET_OK_REGION7_END_ADDR_HIGH_ADDR(base) ((base) + (0x39c))
#define SOC_NPU_EASC_MINIBUS_CFG_ADDR(base) ((base) + (0x3f0))
#define SOC_NPU_EASC_RSV1_ADDR(base) ((base) + (0x3f4))
#define SOC_NPU_EASC_RSV2_ADDR(base) ((base) + (0x3f8))
#define SOC_NPU_EASC_RSV3_ADDR(base) ((base) + (0x3fc))
#define SOC_NPU_EASC_DW_1TO2_DEBUG_ADDR(base) ((base) + (0x400))
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int region0_secure : 1;
        unsigned int reserved : 31;
    } reg;
} SOC_NPU_EASC_REGION0_SECURE_UNION;
#endif
#define SOC_NPU_EASC_REGION0_SECURE_region0_secure_START (0)
#define SOC_NPU_EASC_REGION0_SECURE_region0_secure_END (0)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int reserved : 12;
        unsigned int region0_start_addr_low : 20;
    } reg;
} SOC_NPU_EASC_REGION0_START_ADDR_LOW_UNION;
#endif
#define SOC_NPU_EASC_REGION0_START_ADDR_LOW_region0_start_addr_low_START (12)
#define SOC_NPU_EASC_REGION0_START_ADDR_LOW_region0_start_addr_low_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int region0_start_addr_high : 32;
    } reg;
} SOC_NPU_EASC_REGION0_START_ADDR_HIGH_UNION;
#endif
#define SOC_NPU_EASC_REGION0_START_ADDR_HIGH_region0_start_addr_high_START (0)
#define SOC_NPU_EASC_REGION0_START_ADDR_HIGH_region0_start_addr_high_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int reserved : 12;
        unsigned int region0_end_addr_low : 20;
    } reg;
} SOC_NPU_EASC_REGION0_END_ADDR_LOW_UNION;
#endif
#define SOC_NPU_EASC_REGION0_END_ADDR_LOW_region0_end_addr_low_START (12)
#define SOC_NPU_EASC_REGION0_END_ADDR_LOW_region0_end_addr_low_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int region0_end_addr_high : 32;
    } reg;
} SOC_NPU_EASC_REGION0_END_ADDR_HIGH_UNION;
#endif
#define SOC_NPU_EASC_REGION0_END_ADDR_HIGH_region0_end_addr_high_START (0)
#define SOC_NPU_EASC_REGION0_END_ADDR_HIGH_region0_end_addr_high_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int region0_wr_rd_type : 2;
        unsigned int reserved : 30;
    } reg;
} SOC_NPU_EASC_REGION0_WR_RD_TYPE_UNION;
#endif
#define SOC_NPU_EASC_REGION0_WR_RD_TYPE_region0_wr_rd_type_START (0)
#define SOC_NPU_EASC_REGION0_WR_RD_TYPE_region0_wr_rd_type_END (1)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int region0_cfg_ok : 1;
        unsigned int region0_cfg_err : 1;
        unsigned int reserved : 30;
    } reg;
} SOC_NPU_EASC_REGION0_CFG_STATE_UNION;
#endif
#define SOC_NPU_EASC_REGION0_CFG_STATE_region0_cfg_ok_START (0)
#define SOC_NPU_EASC_REGION0_CFG_STATE_region0_cfg_ok_END (0)
#define SOC_NPU_EASC_REGION0_CFG_STATE_region0_cfg_err_START (1)
#define SOC_NPU_EASC_REGION0_CFG_STATE_region0_cfg_err_END (1)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int region0_setting_lock : 1;
        unsigned int reserved : 31;
    } reg;
} SOC_NPU_EASC_REGION0_SETTING_LOCK_UNION;
#endif
#define SOC_NPU_EASC_REGION0_SETTING_LOCK_region0_setting_lock_START (0)
#define SOC_NPU_EASC_REGION0_SETTING_LOCK_region0_setting_lock_END (0)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int region0_id0 : 8;
        unsigned int region0_id1 : 8;
        unsigned int region0_id2 : 8;
        unsigned int region0_id3 : 8;
    } reg;
} SOC_NPU_EASC_REGION0_ID_3_UNION;
#endif
#define SOC_NPU_EASC_REGION0_ID_3_region0_id0_START (0)
#define SOC_NPU_EASC_REGION0_ID_3_region0_id0_END (7)
#define SOC_NPU_EASC_REGION0_ID_3_region0_id1_START (8)
#define SOC_NPU_EASC_REGION0_ID_3_region0_id1_END (15)
#define SOC_NPU_EASC_REGION0_ID_3_region0_id2_START (16)
#define SOC_NPU_EASC_REGION0_ID_3_region0_id2_END (23)
#define SOC_NPU_EASC_REGION0_ID_3_region0_id3_START (24)
#define SOC_NPU_EASC_REGION0_ID_3_region0_id3_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int region1_secure : 1;
        unsigned int reserved : 31;
    } reg;
} SOC_NPU_EASC_REGION1_SECURE_UNION;
#endif
#define SOC_NPU_EASC_REGION1_SECURE_region1_secure_START (0)
#define SOC_NPU_EASC_REGION1_SECURE_region1_secure_END (0)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int reserved : 12;
        unsigned int region1_start_addr_low : 20;
    } reg;
} SOC_NPU_EASC_REGION1_START_ADDR_LOW_UNION;
#endif
#define SOC_NPU_EASC_REGION1_START_ADDR_LOW_region1_start_addr_low_START (12)
#define SOC_NPU_EASC_REGION1_START_ADDR_LOW_region1_start_addr_low_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int region1_start_addr_high : 32;
    } reg;
} SOC_NPU_EASC_REGION1_START_ADDR_HIGH_UNION;
#endif
#define SOC_NPU_EASC_REGION1_START_ADDR_HIGH_region1_start_addr_high_START (0)
#define SOC_NPU_EASC_REGION1_START_ADDR_HIGH_region1_start_addr_high_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int reserved : 12;
        unsigned int region1_end_addr_low : 20;
    } reg;
} SOC_NPU_EASC_REGION1_END_ADDR_LOW_UNION;
#endif
#define SOC_NPU_EASC_REGION1_END_ADDR_LOW_region1_end_addr_low_START (12)
#define SOC_NPU_EASC_REGION1_END_ADDR_LOW_region1_end_addr_low_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int region1_end_addr_high : 32;
    } reg;
} SOC_NPU_EASC_REGION1_END_ADDR_HIGH_UNION;
#endif
#define SOC_NPU_EASC_REGION1_END_ADDR_HIGH_region1_end_addr_high_START (0)
#define SOC_NPU_EASC_REGION1_END_ADDR_HIGH_region1_end_addr_high_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int region1_wr_rd_type : 2;
        unsigned int reserved : 30;
    } reg;
} SOC_NPU_EASC_REGION1_WR_RD_TYPE_UNION;
#endif
#define SOC_NPU_EASC_REGION1_WR_RD_TYPE_region1_wr_rd_type_START (0)
#define SOC_NPU_EASC_REGION1_WR_RD_TYPE_region1_wr_rd_type_END (1)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int region1_cfg_ok : 1;
        unsigned int region1_cfg_err : 1;
        unsigned int reserved : 30;
    } reg;
} SOC_NPU_EASC_REGION1_CFG_STATE_UNION;
#endif
#define SOC_NPU_EASC_REGION1_CFG_STATE_region1_cfg_ok_START (0)
#define SOC_NPU_EASC_REGION1_CFG_STATE_region1_cfg_ok_END (0)
#define SOC_NPU_EASC_REGION1_CFG_STATE_region1_cfg_err_START (1)
#define SOC_NPU_EASC_REGION1_CFG_STATE_region1_cfg_err_END (1)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int region1_setting_lock : 1;
        unsigned int reserved : 31;
    } reg;
} SOC_NPU_EASC_REGION1_SETTING_LOCK_UNION;
#endif
#define SOC_NPU_EASC_REGION1_SETTING_LOCK_region1_setting_lock_START (0)
#define SOC_NPU_EASC_REGION1_SETTING_LOCK_region1_setting_lock_END (0)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int region1_id0 : 8;
        unsigned int region1_id1 : 8;
        unsigned int region1_id2 : 8;
        unsigned int region1_id3 : 8;
    } reg;
} SOC_NPU_EASC_REGION1_ID_3_UNION;
#endif
#define SOC_NPU_EASC_REGION1_ID_3_region1_id0_START (0)
#define SOC_NPU_EASC_REGION1_ID_3_region1_id0_END (7)
#define SOC_NPU_EASC_REGION1_ID_3_region1_id1_START (8)
#define SOC_NPU_EASC_REGION1_ID_3_region1_id1_END (15)
#define SOC_NPU_EASC_REGION1_ID_3_region1_id2_START (16)
#define SOC_NPU_EASC_REGION1_ID_3_region1_id2_END (23)
#define SOC_NPU_EASC_REGION1_ID_3_region1_id3_START (24)
#define SOC_NPU_EASC_REGION1_ID_3_region1_id3_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int region2_secure : 1;
        unsigned int reserved : 31;
    } reg;
} SOC_NPU_EASC_REGION2_SECURE_UNION;
#endif
#define SOC_NPU_EASC_REGION2_SECURE_region2_secure_START (0)
#define SOC_NPU_EASC_REGION2_SECURE_region2_secure_END (0)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int reserved : 12;
        unsigned int region2_start_addr_low : 20;
    } reg;
} SOC_NPU_EASC_REGION2_START_ADDR_LOW_UNION;
#endif
#define SOC_NPU_EASC_REGION2_START_ADDR_LOW_region2_start_addr_low_START (12)
#define SOC_NPU_EASC_REGION2_START_ADDR_LOW_region2_start_addr_low_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int region2_start_addr_high : 32;
    } reg;
} SOC_NPU_EASC_REGION2_START_ADDR_HIGH_UNION;
#endif
#define SOC_NPU_EASC_REGION2_START_ADDR_HIGH_region2_start_addr_high_START (0)
#define SOC_NPU_EASC_REGION2_START_ADDR_HIGH_region2_start_addr_high_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int reserved : 12;
        unsigned int region2_end_addr_low : 20;
    } reg;
} SOC_NPU_EASC_REGION2_END_ADDR_LOW_UNION;
#endif
#define SOC_NPU_EASC_REGION2_END_ADDR_LOW_region2_end_addr_low_START (12)
#define SOC_NPU_EASC_REGION2_END_ADDR_LOW_region2_end_addr_low_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int region2_end_addr_high : 32;
    } reg;
} SOC_NPU_EASC_REGION2_END_ADDR_HIGH_UNION;
#endif
#define SOC_NPU_EASC_REGION2_END_ADDR_HIGH_region2_end_addr_high_START (0)
#define SOC_NPU_EASC_REGION2_END_ADDR_HIGH_region2_end_addr_high_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int region2_wr_rd_type : 2;
        unsigned int reserved : 30;
    } reg;
} SOC_NPU_EASC_REGION2_WR_RD_TYPE_UNION;
#endif
#define SOC_NPU_EASC_REGION2_WR_RD_TYPE_region2_wr_rd_type_START (0)
#define SOC_NPU_EASC_REGION2_WR_RD_TYPE_region2_wr_rd_type_END (1)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int region2_cfg_ok : 1;
        unsigned int region2_cfg_err : 1;
        unsigned int reserved : 30;
    } reg;
} SOC_NPU_EASC_REGION2_CFG_STATE_UNION;
#endif
#define SOC_NPU_EASC_REGION2_CFG_STATE_region2_cfg_ok_START (0)
#define SOC_NPU_EASC_REGION2_CFG_STATE_region2_cfg_ok_END (0)
#define SOC_NPU_EASC_REGION2_CFG_STATE_region2_cfg_err_START (1)
#define SOC_NPU_EASC_REGION2_CFG_STATE_region2_cfg_err_END (1)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int region2_setting_lock : 1;
        unsigned int reserved : 31;
    } reg;
} SOC_NPU_EASC_REGION2_SETTING_LOCK_UNION;
#endif
#define SOC_NPU_EASC_REGION2_SETTING_LOCK_region2_setting_lock_START (0)
#define SOC_NPU_EASC_REGION2_SETTING_LOCK_region2_setting_lock_END (0)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int region2_id0 : 8;
        unsigned int region2_id1 : 8;
        unsigned int region2_id2 : 8;
        unsigned int region2_id3 : 8;
    } reg;
} SOC_NPU_EASC_REGION2_ID_3_UNION;
#endif
#define SOC_NPU_EASC_REGION2_ID_3_region2_id0_START (0)
#define SOC_NPU_EASC_REGION2_ID_3_region2_id0_END (7)
#define SOC_NPU_EASC_REGION2_ID_3_region2_id1_START (8)
#define SOC_NPU_EASC_REGION2_ID_3_region2_id1_END (15)
#define SOC_NPU_EASC_REGION2_ID_3_region2_id2_START (16)
#define SOC_NPU_EASC_REGION2_ID_3_region2_id2_END (23)
#define SOC_NPU_EASC_REGION2_ID_3_region2_id3_START (24)
#define SOC_NPU_EASC_REGION2_ID_3_region2_id3_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int region3_secure : 1;
        unsigned int reserved : 31;
    } reg;
} SOC_NPU_EASC_REGION3_SECURE_UNION;
#endif
#define SOC_NPU_EASC_REGION3_SECURE_region3_secure_START (0)
#define SOC_NPU_EASC_REGION3_SECURE_region3_secure_END (0)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int reserved : 12;
        unsigned int region3_start_addr_low : 20;
    } reg;
} SOC_NPU_EASC_REGION3_START_ADDR_LOW_UNION;
#endif
#define SOC_NPU_EASC_REGION3_START_ADDR_LOW_region3_start_addr_low_START (12)
#define SOC_NPU_EASC_REGION3_START_ADDR_LOW_region3_start_addr_low_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int region3_start_addr_high : 32;
    } reg;
} SOC_NPU_EASC_REGION3_START_ADDR_HIGH_UNION;
#endif
#define SOC_NPU_EASC_REGION3_START_ADDR_HIGH_region3_start_addr_high_START (0)
#define SOC_NPU_EASC_REGION3_START_ADDR_HIGH_region3_start_addr_high_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int reserved : 12;
        unsigned int region3_end_addr_low : 20;
    } reg;
} SOC_NPU_EASC_REGION3_END_ADDR_LOW_UNION;
#endif
#define SOC_NPU_EASC_REGION3_END_ADDR_LOW_region3_end_addr_low_START (12)
#define SOC_NPU_EASC_REGION3_END_ADDR_LOW_region3_end_addr_low_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int region3_end_addr_high : 32;
    } reg;
} SOC_NPU_EASC_REGION3_END_ADDR_HIGH_UNION;
#endif
#define SOC_NPU_EASC_REGION3_END_ADDR_HIGH_region3_end_addr_high_START (0)
#define SOC_NPU_EASC_REGION3_END_ADDR_HIGH_region3_end_addr_high_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int region3_wr_rd_type : 2;
        unsigned int reserved : 30;
    } reg;
} SOC_NPU_EASC_REGION3_WR_RD_TYPE_UNION;
#endif
#define SOC_NPU_EASC_REGION3_WR_RD_TYPE_region3_wr_rd_type_START (0)
#define SOC_NPU_EASC_REGION3_WR_RD_TYPE_region3_wr_rd_type_END (1)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int region3_cfg_ok : 1;
        unsigned int region3_cfg_err : 1;
        unsigned int reserved : 30;
    } reg;
} SOC_NPU_EASC_REGION3_CFG_STATE_UNION;
#endif
#define SOC_NPU_EASC_REGION3_CFG_STATE_region3_cfg_ok_START (0)
#define SOC_NPU_EASC_REGION3_CFG_STATE_region3_cfg_ok_END (0)
#define SOC_NPU_EASC_REGION3_CFG_STATE_region3_cfg_err_START (1)
#define SOC_NPU_EASC_REGION3_CFG_STATE_region3_cfg_err_END (1)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int region3_setting_lock : 1;
        unsigned int reserved : 31;
    } reg;
} SOC_NPU_EASC_REGION3_SETTING_LOCK_UNION;
#endif
#define SOC_NPU_EASC_REGION3_SETTING_LOCK_region3_setting_lock_START (0)
#define SOC_NPU_EASC_REGION3_SETTING_LOCK_region3_setting_lock_END (0)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int region3_id0 : 8;
        unsigned int region3_id1 : 8;
        unsigned int region3_id2 : 8;
        unsigned int region3_id3 : 8;
    } reg;
} SOC_NPU_EASC_REGION3_ID_3_UNION;
#endif
#define SOC_NPU_EASC_REGION3_ID_3_region3_id0_START (0)
#define SOC_NPU_EASC_REGION3_ID_3_region3_id0_END (7)
#define SOC_NPU_EASC_REGION3_ID_3_region3_id1_START (8)
#define SOC_NPU_EASC_REGION3_ID_3_region3_id1_END (15)
#define SOC_NPU_EASC_REGION3_ID_3_region3_id2_START (16)
#define SOC_NPU_EASC_REGION3_ID_3_region3_id2_END (23)
#define SOC_NPU_EASC_REGION3_ID_3_region3_id3_START (24)
#define SOC_NPU_EASC_REGION3_ID_3_region3_id3_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int region4_secure : 1;
        unsigned int reserved : 31;
    } reg;
} SOC_NPU_EASC_REGION4_SECURE_UNION;
#endif
#define SOC_NPU_EASC_REGION4_SECURE_region4_secure_START (0)
#define SOC_NPU_EASC_REGION4_SECURE_region4_secure_END (0)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int reserved : 12;
        unsigned int region4_start_addr_low : 20;
    } reg;
} SOC_NPU_EASC_REGION4_START_ADDR_LOW_UNION;
#endif
#define SOC_NPU_EASC_REGION4_START_ADDR_LOW_region4_start_addr_low_START (12)
#define SOC_NPU_EASC_REGION4_START_ADDR_LOW_region4_start_addr_low_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int region4_start_addr_high : 32;
    } reg;
} SOC_NPU_EASC_REGION4_START_ADDR_HIGH_UNION;
#endif
#define SOC_NPU_EASC_REGION4_START_ADDR_HIGH_region4_start_addr_high_START (0)
#define SOC_NPU_EASC_REGION4_START_ADDR_HIGH_region4_start_addr_high_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int reserved : 12;
        unsigned int region4_end_addr_low : 20;
    } reg;
} SOC_NPU_EASC_REGION4_END_ADDR_LOW_UNION;
#endif
#define SOC_NPU_EASC_REGION4_END_ADDR_LOW_region4_end_addr_low_START (12)
#define SOC_NPU_EASC_REGION4_END_ADDR_LOW_region4_end_addr_low_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int region4_end_addr_high : 32;
    } reg;
} SOC_NPU_EASC_REGION4_END_ADDR_HIGH_UNION;
#endif
#define SOC_NPU_EASC_REGION4_END_ADDR_HIGH_region4_end_addr_high_START (0)
#define SOC_NPU_EASC_REGION4_END_ADDR_HIGH_region4_end_addr_high_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int region4_wr_rd_type : 2;
        unsigned int reserved : 30;
    } reg;
} SOC_NPU_EASC_REGION4_WR_RD_TYPE_UNION;
#endif
#define SOC_NPU_EASC_REGION4_WR_RD_TYPE_region4_wr_rd_type_START (0)
#define SOC_NPU_EASC_REGION4_WR_RD_TYPE_region4_wr_rd_type_END (1)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int region4_cfg_ok : 1;
        unsigned int region4_cfg_err : 1;
        unsigned int reserved : 30;
    } reg;
} SOC_NPU_EASC_REGION4_CFG_STATE_UNION;
#endif
#define SOC_NPU_EASC_REGION4_CFG_STATE_region4_cfg_ok_START (0)
#define SOC_NPU_EASC_REGION4_CFG_STATE_region4_cfg_ok_END (0)
#define SOC_NPU_EASC_REGION4_CFG_STATE_region4_cfg_err_START (1)
#define SOC_NPU_EASC_REGION4_CFG_STATE_region4_cfg_err_END (1)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int region4_setting_lock : 1;
        unsigned int reserved : 31;
    } reg;
} SOC_NPU_EASC_REGION4_SETTING_LOCK_UNION;
#endif
#define SOC_NPU_EASC_REGION4_SETTING_LOCK_region4_setting_lock_START (0)
#define SOC_NPU_EASC_REGION4_SETTING_LOCK_region4_setting_lock_END (0)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int region4_id0 : 8;
        unsigned int region4_id1 : 8;
        unsigned int region4_id2 : 8;
        unsigned int region4_id3 : 8;
    } reg;
} SOC_NPU_EASC_REGION4_ID_3_UNION;
#endif
#define SOC_NPU_EASC_REGION4_ID_3_region4_id0_START (0)
#define SOC_NPU_EASC_REGION4_ID_3_region4_id0_END (7)
#define SOC_NPU_EASC_REGION4_ID_3_region4_id1_START (8)
#define SOC_NPU_EASC_REGION4_ID_3_region4_id1_END (15)
#define SOC_NPU_EASC_REGION4_ID_3_region4_id2_START (16)
#define SOC_NPU_EASC_REGION4_ID_3_region4_id2_END (23)
#define SOC_NPU_EASC_REGION4_ID_3_region4_id3_START (24)
#define SOC_NPU_EASC_REGION4_ID_3_region4_id3_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int region5_secure : 1;
        unsigned int reserved : 31;
    } reg;
} SOC_NPU_EASC_REGION5_SECURE_UNION;
#endif
#define SOC_NPU_EASC_REGION5_SECURE_region5_secure_START (0)
#define SOC_NPU_EASC_REGION5_SECURE_region5_secure_END (0)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int reserved : 12;
        unsigned int region5_start_addr_low : 20;
    } reg;
} SOC_NPU_EASC_REGION5_START_ADDR_LOW_UNION;
#endif
#define SOC_NPU_EASC_REGION5_START_ADDR_LOW_region5_start_addr_low_START (12)
#define SOC_NPU_EASC_REGION5_START_ADDR_LOW_region5_start_addr_low_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int region5_start_addr_high : 32;
    } reg;
} SOC_NPU_EASC_REGION5_START_ADDR_HIGH_UNION;
#endif
#define SOC_NPU_EASC_REGION5_START_ADDR_HIGH_region5_start_addr_high_START (0)
#define SOC_NPU_EASC_REGION5_START_ADDR_HIGH_region5_start_addr_high_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int reserved : 12;
        unsigned int region5_end_addr_low : 20;
    } reg;
} SOC_NPU_EASC_REGION5_END_ADDR_LOW_UNION;
#endif
#define SOC_NPU_EASC_REGION5_END_ADDR_LOW_region5_end_addr_low_START (12)
#define SOC_NPU_EASC_REGION5_END_ADDR_LOW_region5_end_addr_low_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int region5_end_addr_high : 32;
    } reg;
} SOC_NPU_EASC_REGION5_END_ADDR_HIGH_UNION;
#endif
#define SOC_NPU_EASC_REGION5_END_ADDR_HIGH_region5_end_addr_high_START (0)
#define SOC_NPU_EASC_REGION5_END_ADDR_HIGH_region5_end_addr_high_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int region5_wr_rd_type : 2;
        unsigned int reserved : 30;
    } reg;
} SOC_NPU_EASC_REGION5_WR_RD_TYPE_UNION;
#endif
#define SOC_NPU_EASC_REGION5_WR_RD_TYPE_region5_wr_rd_type_START (0)
#define SOC_NPU_EASC_REGION5_WR_RD_TYPE_region5_wr_rd_type_END (1)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int region5_cfg_ok : 1;
        unsigned int region5_cfg_err : 1;
        unsigned int reserved : 30;
    } reg;
} SOC_NPU_EASC_REGION5_CFG_STATE_UNION;
#endif
#define SOC_NPU_EASC_REGION5_CFG_STATE_region5_cfg_ok_START (0)
#define SOC_NPU_EASC_REGION5_CFG_STATE_region5_cfg_ok_END (0)
#define SOC_NPU_EASC_REGION5_CFG_STATE_region5_cfg_err_START (1)
#define SOC_NPU_EASC_REGION5_CFG_STATE_region5_cfg_err_END (1)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int region5_setting_lock : 1;
        unsigned int reserved : 31;
    } reg;
} SOC_NPU_EASC_REGION5_SETTING_LOCK_UNION;
#endif
#define SOC_NPU_EASC_REGION5_SETTING_LOCK_region5_setting_lock_START (0)
#define SOC_NPU_EASC_REGION5_SETTING_LOCK_region5_setting_lock_END (0)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int region5_id0 : 8;
        unsigned int region5_id1 : 8;
        unsigned int region5_id2 : 8;
        unsigned int region5_id3 : 8;
    } reg;
} SOC_NPU_EASC_REGION5_ID_3_UNION;
#endif
#define SOC_NPU_EASC_REGION5_ID_3_region5_id0_START (0)
#define SOC_NPU_EASC_REGION5_ID_3_region5_id0_END (7)
#define SOC_NPU_EASC_REGION5_ID_3_region5_id1_START (8)
#define SOC_NPU_EASC_REGION5_ID_3_region5_id1_END (15)
#define SOC_NPU_EASC_REGION5_ID_3_region5_id2_START (16)
#define SOC_NPU_EASC_REGION5_ID_3_region5_id2_END (23)
#define SOC_NPU_EASC_REGION5_ID_3_region5_id3_START (24)
#define SOC_NPU_EASC_REGION5_ID_3_region5_id3_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int region6_secure : 1;
        unsigned int reserved : 31;
    } reg;
} SOC_NPU_EASC_REGION6_SECURE_UNION;
#endif
#define SOC_NPU_EASC_REGION6_SECURE_region6_secure_START (0)
#define SOC_NPU_EASC_REGION6_SECURE_region6_secure_END (0)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int reserved : 12;
        unsigned int region6_start_addr_low : 20;
    } reg;
} SOC_NPU_EASC_REGION6_START_ADDR_LOW_UNION;
#endif
#define SOC_NPU_EASC_REGION6_START_ADDR_LOW_region6_start_addr_low_START (12)
#define SOC_NPU_EASC_REGION6_START_ADDR_LOW_region6_start_addr_low_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int region6_start_addr_high : 32;
    } reg;
} SOC_NPU_EASC_REGION6_START_ADDR_HIGH_UNION;
#endif
#define SOC_NPU_EASC_REGION6_START_ADDR_HIGH_region6_start_addr_high_START (0)
#define SOC_NPU_EASC_REGION6_START_ADDR_HIGH_region6_start_addr_high_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int reserved : 12;
        unsigned int region6_end_addr_low : 20;
    } reg;
} SOC_NPU_EASC_REGION6_END_ADDR_LOW_UNION;
#endif
#define SOC_NPU_EASC_REGION6_END_ADDR_LOW_region6_end_addr_low_START (12)
#define SOC_NPU_EASC_REGION6_END_ADDR_LOW_region6_end_addr_low_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int region6_end_addr_high : 32;
    } reg;
} SOC_NPU_EASC_REGION6_END_ADDR_HIGH_UNION;
#endif
#define SOC_NPU_EASC_REGION6_END_ADDR_HIGH_region6_end_addr_high_START (0)
#define SOC_NPU_EASC_REGION6_END_ADDR_HIGH_region6_end_addr_high_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int region6_wr_rd_type : 2;
        unsigned int reserved : 30;
    } reg;
} SOC_NPU_EASC_REGION6_WR_RD_TYPE_UNION;
#endif
#define SOC_NPU_EASC_REGION6_WR_RD_TYPE_region6_wr_rd_type_START (0)
#define SOC_NPU_EASC_REGION6_WR_RD_TYPE_region6_wr_rd_type_END (1)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int region6_cfg_ok : 1;
        unsigned int region6_cfg_err : 1;
        unsigned int reserved : 30;
    } reg;
} SOC_NPU_EASC_REGION6_CFG_STATE_UNION;
#endif
#define SOC_NPU_EASC_REGION6_CFG_STATE_region6_cfg_ok_START (0)
#define SOC_NPU_EASC_REGION6_CFG_STATE_region6_cfg_ok_END (0)
#define SOC_NPU_EASC_REGION6_CFG_STATE_region6_cfg_err_START (1)
#define SOC_NPU_EASC_REGION6_CFG_STATE_region6_cfg_err_END (1)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int region6_setting_lock : 1;
        unsigned int reserved : 31;
    } reg;
} SOC_NPU_EASC_REGION6_SETTING_LOCK_UNION;
#endif
#define SOC_NPU_EASC_REGION6_SETTING_LOCK_region6_setting_lock_START (0)
#define SOC_NPU_EASC_REGION6_SETTING_LOCK_region6_setting_lock_END (0)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int region6_id0 : 8;
        unsigned int region6_id1 : 8;
        unsigned int region6_id2 : 8;
        unsigned int region6_id3 : 8;
    } reg;
} SOC_NPU_EASC_REGION6_ID_3_UNION;
#endif
#define SOC_NPU_EASC_REGION6_ID_3_region6_id0_START (0)
#define SOC_NPU_EASC_REGION6_ID_3_region6_id0_END (7)
#define SOC_NPU_EASC_REGION6_ID_3_region6_id1_START (8)
#define SOC_NPU_EASC_REGION6_ID_3_region6_id1_END (15)
#define SOC_NPU_EASC_REGION6_ID_3_region6_id2_START (16)
#define SOC_NPU_EASC_REGION6_ID_3_region6_id2_END (23)
#define SOC_NPU_EASC_REGION6_ID_3_region6_id3_START (24)
#define SOC_NPU_EASC_REGION6_ID_3_region6_id3_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int region7_secure : 1;
        unsigned int reserved : 31;
    } reg;
} SOC_NPU_EASC_REGION7_SECURE_UNION;
#endif
#define SOC_NPU_EASC_REGION7_SECURE_region7_secure_START (0)
#define SOC_NPU_EASC_REGION7_SECURE_region7_secure_END (0)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int reserved : 12;
        unsigned int region7_start_addr_low : 20;
    } reg;
} SOC_NPU_EASC_REGION7_START_ADDR_LOW_UNION;
#endif
#define SOC_NPU_EASC_REGION7_START_ADDR_LOW_region7_start_addr_low_START (12)
#define SOC_NPU_EASC_REGION7_START_ADDR_LOW_region7_start_addr_low_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int region7_start_addr_high : 32;
    } reg;
} SOC_NPU_EASC_REGION7_START_ADDR_HIGH_UNION;
#endif
#define SOC_NPU_EASC_REGION7_START_ADDR_HIGH_region7_start_addr_high_START (0)
#define SOC_NPU_EASC_REGION7_START_ADDR_HIGH_region7_start_addr_high_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int reserved : 12;
        unsigned int region7_end_addr_low : 20;
    } reg;
} SOC_NPU_EASC_REGION7_END_ADDR_LOW_UNION;
#endif
#define SOC_NPU_EASC_REGION7_END_ADDR_LOW_region7_end_addr_low_START (12)
#define SOC_NPU_EASC_REGION7_END_ADDR_LOW_region7_end_addr_low_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int region7_end_addr_high : 32;
    } reg;
} SOC_NPU_EASC_REGION7_END_ADDR_HIGH_UNION;
#endif
#define SOC_NPU_EASC_REGION7_END_ADDR_HIGH_region7_end_addr_high_START (0)
#define SOC_NPU_EASC_REGION7_END_ADDR_HIGH_region7_end_addr_high_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int region7_wr_rd_type : 2;
        unsigned int reserved : 30;
    } reg;
} SOC_NPU_EASC_REGION7_WR_RD_TYPE_UNION;
#endif
#define SOC_NPU_EASC_REGION7_WR_RD_TYPE_region7_wr_rd_type_START (0)
#define SOC_NPU_EASC_REGION7_WR_RD_TYPE_region7_wr_rd_type_END (1)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int region7_cfg_ok : 1;
        unsigned int region7_cfg_err : 1;
        unsigned int reserved : 30;
    } reg;
} SOC_NPU_EASC_REGION7_CFG_STATE_UNION;
#endif
#define SOC_NPU_EASC_REGION7_CFG_STATE_region7_cfg_ok_START (0)
#define SOC_NPU_EASC_REGION7_CFG_STATE_region7_cfg_ok_END (0)
#define SOC_NPU_EASC_REGION7_CFG_STATE_region7_cfg_err_START (1)
#define SOC_NPU_EASC_REGION7_CFG_STATE_region7_cfg_err_END (1)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int region7_setting_lock : 1;
        unsigned int reserved : 31;
    } reg;
} SOC_NPU_EASC_REGION7_SETTING_LOCK_UNION;
#endif
#define SOC_NPU_EASC_REGION7_SETTING_LOCK_region7_setting_lock_START (0)
#define SOC_NPU_EASC_REGION7_SETTING_LOCK_region7_setting_lock_END (0)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int region7_id0 : 8;
        unsigned int region7_id1 : 8;
        unsigned int region7_id2 : 8;
        unsigned int region7_id3 : 8;
    } reg;
} SOC_NPU_EASC_REGION7_ID_3_UNION;
#endif
#define SOC_NPU_EASC_REGION7_ID_3_region7_id0_START (0)
#define SOC_NPU_EASC_REGION7_ID_3_region7_id0_END (7)
#define SOC_NPU_EASC_REGION7_ID_3_region7_id1_START (8)
#define SOC_NPU_EASC_REGION7_ID_3_region7_id1_END (15)
#define SOC_NPU_EASC_REGION7_ID_3_region7_id2_START (16)
#define SOC_NPU_EASC_REGION7_ID_3_region7_id2_END (23)
#define SOC_NPU_EASC_REGION7_ID_3_region7_id3_START (24)
#define SOC_NPU_EASC_REGION7_ID_3_region7_id3_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int id_sec_default : 2;
        unsigned int reserved : 30;
    } reg;
} SOC_NPU_EASC_REGION_SEC_DEFAULT_UNION;
#endif
#define SOC_NPU_EASC_REGION_SEC_DEFAULT_id_sec_default_START (0)
#define SOC_NPU_EASC_REGION_SEC_DEFAULT_id_sec_default_END (1)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int default_sec_lock : 1;
        unsigned int reserved : 31;
    } reg;
} SOC_NPU_EASC_DEFAULT_SEC_LOCK_UNION;
#endif
#define SOC_NPU_EASC_DEFAULT_SEC_LOCK_default_sec_lock_START (0)
#define SOC_NPU_EASC_DEFAULT_SEC_LOCK_default_sec_lock_END (0)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int int_status0 : 1;
        unsigned int reserved : 31;
    } reg;
} SOC_NPU_EASC_INT_STATUS0_UNION;
#endif
#define SOC_NPU_EASC_INT_STATUS0_int_status0_START (0)
#define SOC_NPU_EASC_INT_STATUS0_int_status0_END (0)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int int_clear : 1;
        unsigned int reserved : 31;
    } reg;
} SOC_NPU_EASC_INT_CLEAR_UNION;
#endif
#define SOC_NPU_EASC_INT_CLEAR_int_clear_START (0)
#define SOC_NPU_EASC_INT_CLEAR_int_clear_END (0)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int fail_addr_low0 : 32;
    } reg;
} SOC_NPU_EASC_FAIL_ADDR_LOW0_UNION;
#endif
#define SOC_NPU_EASC_FAIL_ADDR_LOW0_fail_addr_low0_START (0)
#define SOC_NPU_EASC_FAIL_ADDR_LOW0_fail_addr_low0_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int fail_addr_high0 : 32;
    } reg;
} SOC_NPU_EASC_FAIL_ADDR_HIGH0_UNION;
#endif
#define SOC_NPU_EASC_FAIL_ADDR_HIGH0_fail_addr_high0_START (0)
#define SOC_NPU_EASC_FAIL_ADDR_HIGH0_fail_addr_high0_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int fail_id0 : 24;
        unsigned int reserved : 8;
    } reg;
} SOC_NPU_EASC_FAIL_ID0_UNION;
#endif
#define SOC_NPU_EASC_FAIL_ID0_fail_id0_START (0)
#define SOC_NPU_EASC_FAIL_ID0_fail_id0_END (23)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int fail_wr0 : 1;
        unsigned int fail_rd0 : 1;
        unsigned int fail_len0 : 8;
        unsigned int fail_size0 : 3;
        unsigned int fail_burst0 : 2;
        unsigned int fail_lock0 : 2;
        unsigned int fail_cache0 : 4;
        unsigned int fail_prot0 : 3;
        unsigned int reserved : 8;
    } reg;
} SOC_NPU_EASC_FAIL_INFO0_UNION;
#endif
#define SOC_NPU_EASC_FAIL_INFO0_fail_wr0_START (0)
#define SOC_NPU_EASC_FAIL_INFO0_fail_wr0_END (0)
#define SOC_NPU_EASC_FAIL_INFO0_fail_rd0_START (1)
#define SOC_NPU_EASC_FAIL_INFO0_fail_rd0_END (1)
#define SOC_NPU_EASC_FAIL_INFO0_fail_len0_START (2)
#define SOC_NPU_EASC_FAIL_INFO0_fail_len0_END (9)
#define SOC_NPU_EASC_FAIL_INFO0_fail_size0_START (10)
#define SOC_NPU_EASC_FAIL_INFO0_fail_size0_END (12)
#define SOC_NPU_EASC_FAIL_INFO0_fail_burst0_START (13)
#define SOC_NPU_EASC_FAIL_INFO0_fail_burst0_END (14)
#define SOC_NPU_EASC_FAIL_INFO0_fail_lock0_START (15)
#define SOC_NPU_EASC_FAIL_INFO0_fail_lock0_END (16)
#define SOC_NPU_EASC_FAIL_INFO0_fail_cache0_START (17)
#define SOC_NPU_EASC_FAIL_INFO0_fail_cache0_END (20)
#define SOC_NPU_EASC_FAIL_INFO0_fail_prot0_START (21)
#define SOC_NPU_EASC_FAIL_INFO0_fail_prot0_END (23)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int int_status1 : 1;
        unsigned int reserved : 31;
    } reg;
} SOC_NPU_EASC_INT_STATUS1_UNION;
#endif
#define SOC_NPU_EASC_INT_STATUS1_int_status1_START (0)
#define SOC_NPU_EASC_INT_STATUS1_int_status1_END (0)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int fail_addr_low1 : 32;
    } reg;
} SOC_NPU_EASC_FAIL_ADDR_LOW1_UNION;
#endif
#define SOC_NPU_EASC_FAIL_ADDR_LOW1_fail_addr_low1_START (0)
#define SOC_NPU_EASC_FAIL_ADDR_LOW1_fail_addr_low1_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int fail_addr_high1 : 32;
    } reg;
} SOC_NPU_EASC_FAIL_ADDR_HIGH_UNION;
#endif
#define SOC_NPU_EASC_FAIL_ADDR_HIGH_fail_addr_high1_START (0)
#define SOC_NPU_EASC_FAIL_ADDR_HIGH_fail_addr_high1_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int fail_id1 : 24;
        unsigned int reserved : 8;
    } reg;
} SOC_NPU_EASC_FAIL_ID1_UNION;
#endif
#define SOC_NPU_EASC_FAIL_ID1_fail_id1_START (0)
#define SOC_NPU_EASC_FAIL_ID1_fail_id1_END (23)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int fail_wr1 : 1;
        unsigned int fail_rd1 : 1;
        unsigned int fail_len1 : 8;
        unsigned int fail_size1 : 3;
        unsigned int fail_burst1 : 2;
        unsigned int fail_lock1 : 2;
        unsigned int fail_cache1 : 4;
        unsigned int fail_prot1 : 3;
        unsigned int reserved : 8;
    } reg;
} SOC_NPU_EASC_FAIL_INFO1_UNION;
#endif
#define SOC_NPU_EASC_FAIL_INFO1_fail_wr1_START (0)
#define SOC_NPU_EASC_FAIL_INFO1_fail_wr1_END (0)
#define SOC_NPU_EASC_FAIL_INFO1_fail_rd1_START (1)
#define SOC_NPU_EASC_FAIL_INFO1_fail_rd1_END (1)
#define SOC_NPU_EASC_FAIL_INFO1_fail_len1_START (2)
#define SOC_NPU_EASC_FAIL_INFO1_fail_len1_END (9)
#define SOC_NPU_EASC_FAIL_INFO1_fail_size1_START (10)
#define SOC_NPU_EASC_FAIL_INFO1_fail_size1_END (12)
#define SOC_NPU_EASC_FAIL_INFO1_fail_burst1_START (13)
#define SOC_NPU_EASC_FAIL_INFO1_fail_burst1_END (14)
#define SOC_NPU_EASC_FAIL_INFO1_fail_lock1_START (15)
#define SOC_NPU_EASC_FAIL_INFO1_fail_lock1_END (16)
#define SOC_NPU_EASC_FAIL_INFO1_fail_cache1_START (17)
#define SOC_NPU_EASC_FAIL_INFO1_fail_cache1_END (20)
#define SOC_NPU_EASC_FAIL_INFO1_fail_prot1_START (21)
#define SOC_NPU_EASC_FAIL_INFO1_fail_prot1_END (23)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int reserved : 12;
        unsigned int set_ok_region0_start_addr_low : 20;
    } reg;
} SOC_NPU_EASC_SET_OK_REGION0_START_ADDR_LOW_UNION;
#endif
#define SOC_NPU_EASC_SET_OK_REGION0_START_ADDR_LOW_set_ok_region0_start_addr_low_START (12)
#define SOC_NPU_EASC_SET_OK_REGION0_START_ADDR_LOW_set_ok_region0_start_addr_low_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int set_ok_region0_start_addr_high : 32;
    } reg;
} SOC_NPU_EASC_SET_OK_REGION0_START_ADDR_HIGH_UNION;
#endif
#define SOC_NPU_EASC_SET_OK_REGION0_START_ADDR_HIGH_set_ok_region0_start_addr_high_START (0)
#define SOC_NPU_EASC_SET_OK_REGION0_START_ADDR_HIGH_set_ok_region0_start_addr_high_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int reserved : 12;
        unsigned int set_ok_region0_end_addr_low : 20;
    } reg;
} SOC_NPU_EASC_SET_OK_REGION0_END_ADDR_LOW_UNION;
#endif
#define SOC_NPU_EASC_SET_OK_REGION0_END_ADDR_LOW_set_ok_region0_end_addr_low_START (12)
#define SOC_NPU_EASC_SET_OK_REGION0_END_ADDR_LOW_set_ok_region0_end_addr_low_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int set_ok_region0_end_addr_high : 32;
    } reg;
} SOC_NPU_EASC_SET_OK_REGION0_END_ADDR_HIGH_UNION;
#endif
#define SOC_NPU_EASC_SET_OK_REGION0_END_ADDR_HIGH_set_ok_region0_end_addr_high_START (0)
#define SOC_NPU_EASC_SET_OK_REGION0_END_ADDR_HIGH_set_ok_region0_end_addr_high_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int reserved : 12;
        unsigned int set_ok_region1_start_addr_low : 20;
    } reg;
} SOC_NPU_EASC_SET_OK_REGION1_START_ADDR_LOW_UNION;
#endif
#define SOC_NPU_EASC_SET_OK_REGION1_START_ADDR_LOW_set_ok_region1_start_addr_low_START (12)
#define SOC_NPU_EASC_SET_OK_REGION1_START_ADDR_LOW_set_ok_region1_start_addr_low_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int set_ok_region1_start_addr_high : 32;
    } reg;
} SOC_NPU_EASC_SET_OK_REGION1_START_ADDR_HIGH_UNION;
#endif
#define SOC_NPU_EASC_SET_OK_REGION1_START_ADDR_HIGH_set_ok_region1_start_addr_high_START (0)
#define SOC_NPU_EASC_SET_OK_REGION1_START_ADDR_HIGH_set_ok_region1_start_addr_high_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int reserved : 12;
        unsigned int set_ok_region1_end_addr_low : 20;
    } reg;
} SOC_NPU_EASC_SET_OK_REGION1_END_ADDR_LOW_UNION;
#endif
#define SOC_NPU_EASC_SET_OK_REGION1_END_ADDR_LOW_set_ok_region1_end_addr_low_START (12)
#define SOC_NPU_EASC_SET_OK_REGION1_END_ADDR_LOW_set_ok_region1_end_addr_low_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int set_ok_region1_end_addr_high : 32;
    } reg;
} SOC_NPU_EASC_SET_OK_REGION1_END_ADDR_HIGH_UNION;
#endif
#define SOC_NPU_EASC_SET_OK_REGION1_END_ADDR_HIGH_set_ok_region1_end_addr_high_START (0)
#define SOC_NPU_EASC_SET_OK_REGION1_END_ADDR_HIGH_set_ok_region1_end_addr_high_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int reserved : 12;
        unsigned int set_ok_region2_start_addr_low : 20;
    } reg;
} SOC_NPU_EASC_SET_OK_REGION2_START_ADDR_LOW_UNION;
#endif
#define SOC_NPU_EASC_SET_OK_REGION2_START_ADDR_LOW_set_ok_region2_start_addr_low_START (12)
#define SOC_NPU_EASC_SET_OK_REGION2_START_ADDR_LOW_set_ok_region2_start_addr_low_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int set_ok_region2_start_addr_high : 32;
    } reg;
} SOC_NPU_EASC_SET_OK_REGION2_START_ADDR_HIGH_UNION;
#endif
#define SOC_NPU_EASC_SET_OK_REGION2_START_ADDR_HIGH_set_ok_region2_start_addr_high_START (0)
#define SOC_NPU_EASC_SET_OK_REGION2_START_ADDR_HIGH_set_ok_region2_start_addr_high_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int reserved : 12;
        unsigned int set_ok_region2_end_addr_low : 20;
    } reg;
} SOC_NPU_EASC_SET_OK_REGION2_END_ADDR_LOW_UNION;
#endif
#define SOC_NPU_EASC_SET_OK_REGION2_END_ADDR_LOW_set_ok_region2_end_addr_low_START (12)
#define SOC_NPU_EASC_SET_OK_REGION2_END_ADDR_LOW_set_ok_region2_end_addr_low_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int set_ok_region2_end_addr_high : 32;
    } reg;
} SOC_NPU_EASC_SET_OK_REGION2_END_ADDR_HIGH_UNION;
#endif
#define SOC_NPU_EASC_SET_OK_REGION2_END_ADDR_HIGH_set_ok_region2_end_addr_high_START (0)
#define SOC_NPU_EASC_SET_OK_REGION2_END_ADDR_HIGH_set_ok_region2_end_addr_high_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int reserved : 12;
        unsigned int set_ok_region3_start_addr_low : 20;
    } reg;
} SOC_NPU_EASC_SET_OK_REGION3_START_ADDR_LOW_UNION;
#endif
#define SOC_NPU_EASC_SET_OK_REGION3_START_ADDR_LOW_set_ok_region3_start_addr_low_START (12)
#define SOC_NPU_EASC_SET_OK_REGION3_START_ADDR_LOW_set_ok_region3_start_addr_low_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int set_ok_region3_start_addr_high : 32;
    } reg;
} SOC_NPU_EASC_SET_OK_REGION3_START_ADDR_HIGH_UNION;
#endif
#define SOC_NPU_EASC_SET_OK_REGION3_START_ADDR_HIGH_set_ok_region3_start_addr_high_START (0)
#define SOC_NPU_EASC_SET_OK_REGION3_START_ADDR_HIGH_set_ok_region3_start_addr_high_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int reserved : 12;
        unsigned int set_ok_region3_end_addr_low : 20;
    } reg;
} SOC_NPU_EASC_SET_OK_REGION3_END_ADDR_LOW_UNION;
#endif
#define SOC_NPU_EASC_SET_OK_REGION3_END_ADDR_LOW_set_ok_region3_end_addr_low_START (12)
#define SOC_NPU_EASC_SET_OK_REGION3_END_ADDR_LOW_set_ok_region3_end_addr_low_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int set_ok_region3_end_addr_high : 32;
    } reg;
} SOC_NPU_EASC_SET_OK_REGION3_END_ADDR_HIGH_UNION;
#endif
#define SOC_NPU_EASC_SET_OK_REGION3_END_ADDR_HIGH_set_ok_region3_end_addr_high_START (0)
#define SOC_NPU_EASC_SET_OK_REGION3_END_ADDR_HIGH_set_ok_region3_end_addr_high_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int reserved : 12;
        unsigned int set_ok_region4_start_addr_low : 20;
    } reg;
} SOC_NPU_EASC_SET_OK_REGION4_START_ADDR_LOW_UNION;
#endif
#define SOC_NPU_EASC_SET_OK_REGION4_START_ADDR_LOW_set_ok_region4_start_addr_low_START (12)
#define SOC_NPU_EASC_SET_OK_REGION4_START_ADDR_LOW_set_ok_region4_start_addr_low_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int set_ok_region4_start_addr_high : 32;
    } reg;
} SOC_NPU_EASC_SET_OK_REGION4_START_ADDR_HIGH_UNION;
#endif
#define SOC_NPU_EASC_SET_OK_REGION4_START_ADDR_HIGH_set_ok_region4_start_addr_high_START (0)
#define SOC_NPU_EASC_SET_OK_REGION4_START_ADDR_HIGH_set_ok_region4_start_addr_high_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int reserved : 12;
        unsigned int set_ok_region4_end_addr_low : 20;
    } reg;
} SOC_NPU_EASC_SET_OK_REGION4_END_ADDR_LOW_UNION;
#endif
#define SOC_NPU_EASC_SET_OK_REGION4_END_ADDR_LOW_set_ok_region4_end_addr_low_START (12)
#define SOC_NPU_EASC_SET_OK_REGION4_END_ADDR_LOW_set_ok_region4_end_addr_low_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int set_ok_region4_end_addr_high : 32;
    } reg;
} SOC_NPU_EASC_SET_OK_REGION4_END_ADDR_HIGH_UNION;
#endif
#define SOC_NPU_EASC_SET_OK_REGION4_END_ADDR_HIGH_set_ok_region4_end_addr_high_START (0)
#define SOC_NPU_EASC_SET_OK_REGION4_END_ADDR_HIGH_set_ok_region4_end_addr_high_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int reserved : 12;
        unsigned int set_ok_region5_start_addr_low : 20;
    } reg;
} SOC_NPU_EASC_SET_OK_REGION5_START_ADDR_LOW_UNION;
#endif
#define SOC_NPU_EASC_SET_OK_REGION5_START_ADDR_LOW_set_ok_region5_start_addr_low_START (12)
#define SOC_NPU_EASC_SET_OK_REGION5_START_ADDR_LOW_set_ok_region5_start_addr_low_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int set_ok_region5_start_addr_high : 32;
    } reg;
} SOC_NPU_EASC_SET_OK_REGION5_START_ADDR_HIGH_UNION;
#endif
#define SOC_NPU_EASC_SET_OK_REGION5_START_ADDR_HIGH_set_ok_region5_start_addr_high_START (0)
#define SOC_NPU_EASC_SET_OK_REGION5_START_ADDR_HIGH_set_ok_region5_start_addr_high_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int reserved : 12;
        unsigned int set_ok_region5_end_addr_low : 20;
    } reg;
} SOC_NPU_EASC_SET_OK_REGION5_END_ADDR_LOW_UNION;
#endif
#define SOC_NPU_EASC_SET_OK_REGION5_END_ADDR_LOW_set_ok_region5_end_addr_low_START (12)
#define SOC_NPU_EASC_SET_OK_REGION5_END_ADDR_LOW_set_ok_region5_end_addr_low_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int set_ok_region5_end_addr_high : 32;
    } reg;
} SOC_NPU_EASC_SET_OK_REGION5_END_ADDR_HIGH_UNION;
#endif
#define SOC_NPU_EASC_SET_OK_REGION5_END_ADDR_HIGH_set_ok_region5_end_addr_high_START (0)
#define SOC_NPU_EASC_SET_OK_REGION5_END_ADDR_HIGH_set_ok_region5_end_addr_high_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int reserved : 12;
        unsigned int set_ok_region6_start_addr_low : 20;
    } reg;
} SOC_NPU_EASC_SET_OK_REGION6_START_ADDR_LOW_UNION;
#endif
#define SOC_NPU_EASC_SET_OK_REGION6_START_ADDR_LOW_set_ok_region6_start_addr_low_START (12)
#define SOC_NPU_EASC_SET_OK_REGION6_START_ADDR_LOW_set_ok_region6_start_addr_low_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int set_ok_region6_start_addr_high : 32;
    } reg;
} SOC_NPU_EASC_SET_OK_REGION6_START_ADDR_HIGH_UNION;
#endif
#define SOC_NPU_EASC_SET_OK_REGION6_START_ADDR_HIGH_set_ok_region6_start_addr_high_START (0)
#define SOC_NPU_EASC_SET_OK_REGION6_START_ADDR_HIGH_set_ok_region6_start_addr_high_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int reserved : 12;
        unsigned int set_ok_region6_end_addr_low : 20;
    } reg;
} SOC_NPU_EASC_SET_OK_REGION6_END_ADDR_LOW_UNION;
#endif
#define SOC_NPU_EASC_SET_OK_REGION6_END_ADDR_LOW_set_ok_region6_end_addr_low_START (12)
#define SOC_NPU_EASC_SET_OK_REGION6_END_ADDR_LOW_set_ok_region6_end_addr_low_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int set_ok_region6_end_addr_high : 32;
    } reg;
} SOC_NPU_EASC_SET_OK_REGION6_END_ADDR_HIGH_UNION;
#endif
#define SOC_NPU_EASC_SET_OK_REGION6_END_ADDR_HIGH_set_ok_region6_end_addr_high_START (0)
#define SOC_NPU_EASC_SET_OK_REGION6_END_ADDR_HIGH_set_ok_region6_end_addr_high_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int reserved : 12;
        unsigned int set_ok_region7_start_addr_low : 20;
    } reg;
} SOC_NPU_EASC_SET_OK_REGION7_START_ADDR_LOW_UNION;
#endif
#define SOC_NPU_EASC_SET_OK_REGION7_START_ADDR_LOW_set_ok_region7_start_addr_low_START (12)
#define SOC_NPU_EASC_SET_OK_REGION7_START_ADDR_LOW_set_ok_region7_start_addr_low_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int set_ok_region7_start_addr_high : 32;
    } reg;
} SOC_NPU_EASC_SET_OK_REGION7_START_ADDR_HIGH_UNION;
#endif
#define SOC_NPU_EASC_SET_OK_REGION7_START_ADDR_HIGH_set_ok_region7_start_addr_high_START (0)
#define SOC_NPU_EASC_SET_OK_REGION7_START_ADDR_HIGH_set_ok_region7_start_addr_high_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int reserved : 12;
        unsigned int set_ok_region7_end_addr_low : 20;
    } reg;
} SOC_NPU_EASC_SET_OK_REGION7_END_ADDR_LOW_UNION;
#endif
#define SOC_NPU_EASC_SET_OK_REGION7_END_ADDR_LOW_set_ok_region7_end_addr_low_START (12)
#define SOC_NPU_EASC_SET_OK_REGION7_END_ADDR_LOW_set_ok_region7_end_addr_low_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int set_ok_region7_end_addr_high : 32;
    } reg;
} SOC_NPU_EASC_SET_OK_REGION7_END_ADDR_HIGH_UNION;
#endif
#define SOC_NPU_EASC_SET_OK_REGION7_END_ADDR_HIGH_set_ok_region7_end_addr_high_START (0)
#define SOC_NPU_EASC_SET_OK_REGION7_END_ADDR_HIGH_set_ok_region7_end_addr_high_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int ckg_en : 1;
        unsigned int dlock_and_dbg_clr : 1;
        unsigned int rsv0 : 30;
    } reg;
} SOC_NPU_EASC_MINIBUS_CFG_UNION;
#endif
#define SOC_NPU_EASC_MINIBUS_CFG_ckg_en_START (0)
#define SOC_NPU_EASC_MINIBUS_CFG_ckg_en_END (0)
#define SOC_NPU_EASC_MINIBUS_CFG_dlock_and_dbg_clr_START (1)
#define SOC_NPU_EASC_MINIBUS_CFG_dlock_and_dbg_clr_END (1)
#define SOC_NPU_EASC_MINIBUS_CFG_rsv0_START (2)
#define SOC_NPU_EASC_MINIBUS_CFG_rsv0_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int rsv1 : 32;
    } reg;
} SOC_NPU_EASC_RSV1_UNION;
#endif
#define SOC_NPU_EASC_RSV1_rsv1_START (0)
#define SOC_NPU_EASC_RSV1_rsv1_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int rsv2 : 32;
    } reg;
} SOC_NPU_EASC_RSV2_UNION;
#endif
#define SOC_NPU_EASC_RSV2_rsv2_START (0)
#define SOC_NPU_EASC_RSV2_rsv2_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int rsv3 : 32;
    } reg;
} SOC_NPU_EASC_RSV3_UNION;
#endif
#define SOC_NPU_EASC_RSV3_rsv3_START (0)
#define SOC_NPU_EASC_RSV3_rsv3_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int dw1to2_dbg_ar_err : 4;
        unsigned int dw1to2_dbg_aw_err : 4;
        unsigned int dlock_slv : 4;
        unsigned int dlock_mst : 2;
        unsigned int hiaxi_idle : 1;
        unsigned int reserved : 17;
    } reg;
} SOC_NPU_EASC_DW_1TO2_DEBUG_UNION;
#endif
#define SOC_NPU_EASC_DW_1TO2_DEBUG_dw1to2_dbg_ar_err_START (0)
#define SOC_NPU_EASC_DW_1TO2_DEBUG_dw1to2_dbg_ar_err_END (3)
#define SOC_NPU_EASC_DW_1TO2_DEBUG_dw1to2_dbg_aw_err_START (4)
#define SOC_NPU_EASC_DW_1TO2_DEBUG_dw1to2_dbg_aw_err_END (7)
#define SOC_NPU_EASC_DW_1TO2_DEBUG_dlock_slv_START (8)
#define SOC_NPU_EASC_DW_1TO2_DEBUG_dlock_slv_END (11)
#define SOC_NPU_EASC_DW_1TO2_DEBUG_dlock_mst_START (12)
#define SOC_NPU_EASC_DW_1TO2_DEBUG_dlock_mst_END (13)
#define SOC_NPU_EASC_DW_1TO2_DEBUG_hiaxi_idle_START (14)
#define SOC_NPU_EASC_DW_1TO2_DEBUG_hiaxi_idle_END (14)
#ifdef __cplusplus
#if __cplusplus
    }
#endif
#endif
#endif
