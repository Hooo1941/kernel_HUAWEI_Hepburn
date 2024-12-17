/******************************************************************
 * Copyright (c) Hisilicon Technologies Co., Ltd. 2012-2018. All rights reserved.
 * File name  :
 * Description:
 *
 * Date         2020-04-17 16:28:10
 ********************************************************************/

#ifndef __IPP_TOP_DRV_PRIV_H_HIOF_
#define __IPP_TOP_DRV_PRIV_H_HIOF_

union u_hiof_crg_cfg0 {
	struct {
		unsigned int    hiof_clken        : 1  ; /* [0] */
		unsigned int    rsv_94            : 15  ; /* [15:1] */
		unsigned int    hiof_force_clk_on : 1  ; /* [16] */
		unsigned int    rsv_95            : 15  ; /* [31:17] */
	} bits;

	unsigned int    u32;
};

union u_hiof_crg_cfg1 {
	struct {
		unsigned int    hiof_soft_rst : 1  ; /* [0] */
		unsigned int    rsv_96        : 31  ; /* [31:1] */
	} bits;

	unsigned int    u32;
};

union u_hiof_mem_cfg {
	struct {
		unsigned int    hiof_mem_ctrl_sp : 3  ; /* [2:0] */
		unsigned int    rsv_97           : 29  ; /* [31:3] */
	} bits;

	unsigned int    u32;
};

union u_hiof_irq_reg0 {
	struct {
		unsigned int    hiof_irq_clr : 13  ; /* [12:0] */
		unsigned int    rsv_98       : 19  ; /* [31:13] */
	} bits;

	unsigned int    u32;
};

union u_hiof_irq_reg1 {
	struct {
		unsigned int    hiof_irq_outen : 2  ; /* [1:0] */
		unsigned int    rsv_99         : 30  ; /* [31:2] */
	} bits;

	unsigned int    u32;
};

union u_hiof_irq_reg2 {
	struct {
		unsigned int    hiof_irq_mask : 24  ; /* [23:0] */
		unsigned int    rsv_100       : 8  ; /* [31:24] */
	} bits;

	unsigned int    u32;
};

union u_hiof_irq_reg3 {
	struct {
		unsigned int    hiof_irq_state_raw : 24  ; /* [23:0] */
		unsigned int    rsv_101            : 8  ; /* [31:24] */
	} bits;

	unsigned int    u32;
};

union u_hiof_irq_reg4 {
	struct {
		unsigned int    hiof_irq_state_mask : 24  ; /* [23:0] */
		unsigned int    rsv_102             : 8  ; /* [31:24] */
	} bits;

	unsigned int    u32;
};

#endif // __IPP_TOP_DRV_PRIV_H_HIOF_