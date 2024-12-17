/******************************************************************
 * Copyright (c) Hisilicon Technologies Co., Ltd. 2012-2018. All rights reserved.
 * File name  :
 * Description:
 *
 * Date         2020-04-17 16:28:10
 ********************************************************************/

#ifndef __IPP_TOP_DRV_PRIV_H_MC_
#define __IPP_TOP_DRV_PRIV_H_MC_

union u_mc_crg_cfg0 {
	struct {
		unsigned int    mc_clken        : 1  ; /* [0] */
		unsigned int    rsv_109         : 15  ; /* [15:1] */
		unsigned int    mc_force_clk_on : 1  ; /* [16] */
		unsigned int    rsv_110         : 15  ; /* [31:17] */
	} bits;

	unsigned int    u32;
};

union u_mc_crg_cfg1 {
	struct {
		unsigned int    mc_soft_rst : 1  ; /* [0] */
		unsigned int    rsv_111     : 31  ; /* [31:1] */
	} bits;

	unsigned int    u32;
};

union u_mc_mem_cfg {
	struct {
		unsigned int    mc_mem_ctrl_sp : 3  ; /* [2:0] */
		unsigned int    rsv_112        : 29  ; /* [31:3] */
	} bits;

	unsigned int    u32;
};

union u_mc_irq_reg0 {
	struct {
		unsigned int    mc_irq_clr : 4  ; /* [3:0] */
		unsigned int    rsv_113    : 28  ; /* [31:4] */
	} bits;

	unsigned int    u32;
};

union u_mc_irq_reg1 {
	struct {
		unsigned int    mc_irq_outen : 2  ; /* [1:0] */
		unsigned int    rsv_114      : 30  ; /* [31:2] */
	} bits;

	unsigned int    u32;
};

union u_mc_irq_reg2 {
	struct {
		unsigned int    mc_irq_mask : 4  ; /* [3:0] */
		unsigned int    rsv_115     : 28  ; /* [31:4] */
	} bits;

	unsigned int    u32;
};

union u_mc_irq_reg3 {
	struct {
		unsigned int    mc_irq_state_raw : 4  ; /* [3:0] */
		unsigned int    rsv_116          : 28  ; /* [31:4] */
	} bits;

	unsigned int    u32;
};

union u_mc_irq_reg4 {
	struct {
		unsigned int    mc_irq_state_mask : 4  ; /* [3:0] */
		unsigned int    rsv_117           : 28  ; /* [31:4] */
	} bits;

	unsigned int    u32;
};

#endif // __IPP_TOP_DRV_PRIV_H_MC_
