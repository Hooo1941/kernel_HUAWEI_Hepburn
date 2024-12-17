/******************************************************************
 * Copyright (c) Hisilicon Technologies Co., Ltd. 2012-2018. All rights reserved.
 * File name  :
 * Description:
 *
 * Date         2020-04-17 16:28:10
 ********************************************************************/

#ifndef __IPP_TOP_DRV_PRIV_H_ENH_
#define __IPP_TOP_DRV_PRIV_H_ENH_

union u_enh_crg_cfg0 {
	struct {
		unsigned int    enh_clken        : 1  ; /* [0] */
		unsigned int    rsv_66           : 15  ; /* [15:1] */
		unsigned int    enh_force_clk_on : 1  ; /* [16] */
		unsigned int    rsv_67           : 15  ; /* [31:17] */
	} bits;

	unsigned int    u32;
};

union u_enh_crg_cfg1 {
	struct {
		unsigned int    enh_soft_rst : 1  ; /* [0] */
		unsigned int    rsv_68       : 31  ; /* [31:1] */
	} bits;

	unsigned int    u32;
};

union u_enh_mem_cfg {
	struct {
		unsigned int    enh_mem_ctrl_sp : 3  ; /* [2:0] */
		unsigned int    rsv_69          : 29  ; /* [31:3] */
	} bits;

	unsigned int    u32;
};

union u_enh_vpb_cfg {
	struct {
		unsigned int    enh_vpbg_en : 2  ; /* [1:0] */
		unsigned int    rsv_70      : 30  ; /* [31:2] */
	} bits;

	unsigned int    u32;
};

union u_enh_irq_reg0 {
	struct {
		unsigned int    enh_irq_clr : 10  ; /* [9:0] */
		unsigned int    rsv_71      : 22  ; /* [31:10] */
	} bits;

	unsigned int    u32;
};

union u_enh_irq_reg1 {
	struct {
		unsigned int    enh_irq_outen : 2  ; /* [1:0] */
		unsigned int    rsv_72        : 30  ; /* [31:2] */
	} bits;

	unsigned int    u32;
};

union u_enh_irq_reg2 {
	struct {
		unsigned int    enh_irq_mask : 10  ; /* [9:0] */
		unsigned int    rsv_73       : 22  ; /* [31:10] */
	} bits;

	unsigned int    u32;
};

union u_enh_irq_reg3 {
	struct {
		unsigned int    enh_irq_state_raw : 10  ; /* [9:0] */
		unsigned int    rsv_74            : 22  ; /* [31:10] */
	} bits;

	unsigned int    u32;
};

union u_enh_irq_reg4 {
	struct {
		unsigned int    enh_irq_state_mask : 10  ; /* [9:0] */
		unsigned int    rsv_75             : 22  ; /* [31:10] */
	} bits;

	unsigned int    u32;
};

#endif // __IPP_TOP_DRV_PRIV_H_ENH_
