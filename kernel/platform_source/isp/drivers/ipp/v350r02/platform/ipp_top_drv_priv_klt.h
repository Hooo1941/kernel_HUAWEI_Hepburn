/******************************************************************
 * Copyright (c) Hisilicon Technologies Co., Ltd. 2012-2018. All rights reserved.
 * File name  :
 * Description:
 *
 * Date         2020-04-17 16:28:10
 ********************************************************************/

#ifndef __IPP_TOP_DRV_PRIV_H_KLT_
#define __IPP_TOP_DRV_PRIV_H_KLT_

union u_klt_crg_cfg0 {
	struct {
		unsigned int    klt_clken        : 1  ; /* [0] */
		unsigned int    rsv_85           : 15  ; /* [15:1] */
		unsigned int    klt_force_clk_on : 1  ; /* [16] */
		unsigned int    rsv_86           : 15  ; /* [31:17] */
	} bits;

	unsigned int    u32;
};

union u_klt_crg_cfg1 {
	struct {
		unsigned int    klt_soft_rst : 1  ; /* [0] */
		unsigned int    rsv_87       : 31  ; /* [31:1] */
	} bits;

	unsigned int    u32;
};

union u_klt_mem_cfg {
	struct {
		unsigned int    klt_mem_ctrl_sp : 3  ; /* [2:0] */
		unsigned int    rsv_88          : 29  ; /* [31:3] */
	} bits;

	unsigned int    u32;
};

union u_klt_irq_reg0 {
	struct {
		unsigned int    klt_irq_clr : 31  ; /* [30:0] */
		unsigned int    rsv_89      : 1  ; /* [31] */
	} bits;

	unsigned int    u32;
};

union u_klt_irq_reg1 {
	struct {
		unsigned int    klt_irq_outen : 2  ; /* [1:0] */
		unsigned int    rsv_90        : 30  ; /* [31:2] */
	} bits;

	unsigned int    u32;
};

union u_klt_irq_reg2 {
	struct {
		unsigned int    klt_irq_mask : 31  ; /* [30:0] */
		unsigned int    rsv_91       : 1  ; /* [31] */
	} bits;

	unsigned int    u32;
};

union u_klt_irq_reg3 {
	struct {
		unsigned int    klt_irq_state_raw : 31  ; /* [30:0] */
		unsigned int    rsv_92            : 1  ; /* [31] */
	} bits;

	unsigned int    u32;
};

union u_klt_irq_reg4 {
	struct {
		unsigned int    klt_irq_state_mask : 31  ; /* [30:0] */
		unsigned int    rsv_93             : 1  ; /* [31] */
	} bits;

	unsigned int    u32;
};

#endif // __IPP_TOP_DRV_PRIV_H_KLT_