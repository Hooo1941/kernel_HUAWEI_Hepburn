/******************************************************************
 * Copyright (c) Hisilicon Technologies Co., Ltd. 2012-2018. All rights reserved.
 * File name  :
 * Description:
 *
 * Date         2020-04-17 16:28:10
 ********************************************************************/

#ifndef __IPP_TOP_DRV_PRIV_H_CMP_
#define __IPP_TOP_DRV_PRIV_H_CMP_

union u_cmp_crg_cfg0 {
	struct {
		unsigned int    cmp_clken        : 1  ; /* [0] */
		unsigned int    rsv_57           : 15  ; /* [15:1] */
		unsigned int    cmp_force_clk_on : 1  ; /* [16] */
		unsigned int    rsv_58           : 15  ; /* [31:17] */
	} bits;

	unsigned int    u32;
};

union u_cmp_crg_cfg1 {
	struct {
		unsigned int    cmp_soft_rst : 1  ; /* [0] */
		unsigned int    rsv_59       : 31  ; /* [31:1] */
	} bits;

	unsigned int    u32;
};

union u_cmp_mem_cfg {
	struct {
		unsigned int    cmp_mem_ctrl_sp : 3  ; /* [2:0] */
		unsigned int    rsv_60          : 29  ; /* [31:3] */
	} bits;

	unsigned int    u32;
};

union u_cmp_irq_reg0 {
	struct {
		unsigned int    cmp_irq_clr : 5  ; /* [4:0] */
		unsigned int    rsv_61      : 27  ; /* [31:5] */
	} bits;

	unsigned int    u32;
};

union u_cmp_irq_reg1 {
	struct {
		unsigned int    cmp_irq_outen : 2  ; /* [1:0] */
		unsigned int    rsv_62        : 30  ; /* [31:2] */
	} bits;

	unsigned int    u32;
};

union u_cmp_irq_reg2 {
	struct {
		unsigned int    cmp_irq_mask : 5  ; /* [4:0] */
		unsigned int    rsv_63       : 27  ; /* [31:5] */
	} bits;

	unsigned int    u32;
};

union u_cmp_irq_reg3 {
	struct {
		unsigned int    cmp_irq_state_raw : 5  ; /* [4:0] */
		unsigned int    rsv_64            : 27  ; /* [31:5] */
	} bits;

	unsigned int    u32;
};

union u_cmp_irq_reg4 {
	struct {
		unsigned int    cmp_irq_state_mask : 5  ; /* [4:0] */
		unsigned int    rsv_65             : 27  ; /* [31:5] */
	} bits;

	unsigned int    u32;
};

#endif // __IPP_TOP_DRV_PRIV_H_CMP_
