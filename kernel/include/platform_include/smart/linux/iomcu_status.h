/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2021-2021. All rights reserved.
 * Description: Contexthub status driver.
 * Create: 2021-12-13
 */

#ifndef __IOMCU_STATUS_H__
#define __IOMCU_STATUS_H__

#define hm_en(n)                (0x10001 << (n))
#define hm_dis(n)               (0x10000 << (n))

#define writel_mask(mask, data, addr) \
	do { /*lint -save -e717 */ \
		writel((((u32)readl(addr)) & (~((u32)(mask)))) | ((data) & (mask)), (addr)); \
	} while (0) /*lint -restore */

static inline unsigned int is_bits_clr(unsigned int  mask, volatile void __iomem *addr)
{
	return (((*(volatile unsigned int *) (addr)) & (mask)) == 0);
}

static inline unsigned int is_bits_set(unsigned int  mask, volatile void __iomem *addr)
{
	return (((*(volatile unsigned int*) (addr))&(mask)) == (mask));
}

static inline void set_bits(unsigned int  mask, volatile void __iomem *addr)
{
	(*(volatile unsigned int *) (addr)) |= mask;
}

static inline void clr_bits(unsigned int mask, volatile void __iomem *addr)
{
	(*(volatile unsigned int *) (addr)) &= ~(mask);
}

/**
 * @brief get contexthub status from dts, "hisilicon,contexthub_status".
 *
 * @param svc_id : service id.
 * @param svc    : service for register.
 * @return INT32 : SUCCESS or error code.(refs to ECODE_XXX)
 */
int get_contexthub_dts_status(void);

/**
 * @brief get contexthub status from dts, "huawei,ext_sensorhub_status".
 *
 * @param svc_id : service id.
 * @param svc    : service for register.
 * @return INT32 : SUCCESS or error code.(refs to ECODE_XXX)
 */
int get_ext_contexthub_dts_status(void);

#endif

