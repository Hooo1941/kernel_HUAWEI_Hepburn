/******************************************************************
 * Copyright (c) Hisilicon Technologies Co., Ltd. 2012-2018. All rights reserved.
 * File name  :
 * Description:
 *
 * Date         2020-04-17 16:28:10
 ********************************************************************/

#ifndef __IPP_TOP_DRV_PRIV_H_ARF_
#define __IPP_TOP_DRV_PRIV_H_ARF_

union u_arf_crg_cfg0 {
	struct {
		unsigned int    arf_clken        : 1  ; /* [0] */
		unsigned int    rsv_39           : 15  ; /* [15:1] */
		unsigned int    arf_force_clk_on : 1  ; /* [16] */
		unsigned int    rsv_40           : 15  ; /* [31:17] */
	} bits;

	unsigned int    u32;
};

union u_arf_crg_cfg1 {
	struct {
		unsigned int    arf_soft_rst : 1  ; /* [0] */
		unsigned int    rsv_41       : 31  ; /* [31:1] */
	} bits;

	unsigned int    u32;
};

union u_arf_mem_cfg {
	struct {
		unsigned int    arf_mem_ctrl_sp : 3  ; /* [2:0] */
		unsigned int    rsv_42          : 29  ; /* [31:3] */
	} bits;

	unsigned int    u32;
};

union u_arf_irq_reg0 {
	struct {
		unsigned int    arf_irq_clr : 30  ; /* [29:0] */
		unsigned int    rsv_43      : 2  ; /* [31:30] */
	} bits;

	unsigned int    u32;
};

union u_arf_irq_reg1 {
	struct {
		unsigned int    arf_irq_outen : 2  ; /* [1:0] */
		unsigned int    rsv_44        : 30  ; /* [31:2] */
	} bits;

	unsigned int    u32;
};

union u_arf_irq_reg2 {
	struct {
		unsigned int    arf_irq_mask : 30  ; /* [29:0] */
		unsigned int    rsv_45       : 2  ; /* [31:30] */
	} bits;

	unsigned int    u32;
};

union u_arf_irq_reg3 {
	struct {
		unsigned int    arf_irq_state_raw : 30  ; /* [29:0] */
		unsigned int    rsv_46            : 2  ; /* [31:30] */
	} bits;

	unsigned int    u32;
};

union u_arf_irq_reg4 {
	struct {
		unsigned int    arf_irq_state_mask : 30  ; /* [29:0] */
		unsigned int    rsv_47             : 2  ; /* [31:30] */
	} bits;

	unsigned int    u32;
};

#endif // __IPP_TOP_DRV_PRIV_H_ARF_
