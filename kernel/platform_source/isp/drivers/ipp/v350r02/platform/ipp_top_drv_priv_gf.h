/******************************************************************
 * Copyright (c) Hisilicon Technologies Co., Ltd. 2012-2018. All rights reserved.
 * File name  :
 * Description:
 *
 * Date         2020-04-17 16:28:10
 ********************************************************************/

#ifndef __IPP_TOP_DRV_PRIV_H_GF_
#define __IPP_TOP_DRV_PRIV_H_GF_

union u_gf_crg_cfg0 {
	struct {
		unsigned int    gf_clken        : 1  ; /* [0] */
		unsigned int    rsv_30          : 15  ; /* [15:1] */
		unsigned int    gf_force_clk_on : 1  ; /* [16] */
		unsigned int    rsv_31          : 15  ; /* [31:17] */
	} bits;

	unsigned int    u32;
};

union u_gf_crg_cfg1 {
	struct {
		unsigned int    gf_soft_rst : 1  ; /* [0] */
		unsigned int    rsv_32      : 31  ; /* [31:1] */
	} bits;

	unsigned int    u32;
};

union u_gf_mem_cfg {
	struct {
		unsigned int    gf_mem_ctrl_sp : 3  ; /* [2:0] */
		unsigned int    rsv_33         : 29  ; /* [31:3] */
	} bits;

	unsigned int    u32;
};

union u_gf_irq_reg0 {
	struct {
		unsigned int    gf_irq_clr : 13  ; /* [12:0] */
		unsigned int    rsv_34     : 19  ; /* [31:13] */
	} bits;

	unsigned int    u32;
};

union u_gf_irq_reg1 {
	struct {
		unsigned int    gf_irq_outen : 2  ; /* [1:0] */
		unsigned int    rsv_35       : 30  ; /* [31:2] */
	} bits;

	unsigned int    u32;
};

union u_gf_irq_reg2 {
	struct {
		unsigned int    gf_irq_mask : 13  ; /* [12:0] */
		unsigned int    rsv_36      : 19  ; /* [31:13] */
	} bits;

	unsigned int    u32;
};

union u_gf_irq_reg3 {
	struct {
		unsigned int    gf_irq_state_raw : 13  ; /* [12:0] */
		unsigned int    rsv_37           : 19  ; /* [31:13] */
	} bits;

	unsigned int    u32;
};

union u_gf_irq_reg4 {
	struct {
		unsigned int    gf_irq_state_mask : 13  ; /* [12:0] */
		unsigned int    rsv_38            : 19  ; /* [31:13] */
	} bits;

	unsigned int    u32;
};

#endif // __IPP_TOP_DRV_PRIV_H_GF_
