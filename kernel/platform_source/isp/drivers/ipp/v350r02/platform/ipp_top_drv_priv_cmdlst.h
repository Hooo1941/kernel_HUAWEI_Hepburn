/******************************************************************
 * Copyright (c) Hisilicon Technologies Co., Ltd. 2012-2018. All rights reserved.
 * File name  :
 * Description:
 *
 * Date         2020-04-17 16:28:10
 ********************************************************************/

#ifndef __IPP_TOP_DRV_PRIV_H_CMDLST_
#define __IPP_TOP_DRV_PRIV_H_CMDLST_

union u_cmdlst_ctrl_map {
	struct {
		unsigned int    cmdlst_ctrl_chn : 4  ; /* [3:0] */
		unsigned int    rsv_103         : 28  ; /* [31:4] */
	} bits;

	unsigned int    u32;
};

union u_cmdlst_ctrl_pm {
	struct {
		unsigned int    cmdlst_post_mask : 2  ; /* [1:0] */
		unsigned int    rsv_104          : 30  ; /* [31:2] */
	} bits;

	unsigned int    u32;
};

union u_cmdlst_irq_clr {
	struct {
		unsigned int    cmdlst_irq_clr : 16  ; /* [15:0] */
		unsigned int    rsv_105        : 16  ; /* [31:16] */
	} bits;

	unsigned int    u32;
};

union u_cmdlst_irq_msk {
	struct {
		unsigned int    cmdlst_irq_mask : 16  ; /* [15:0] */
		unsigned int    rsv_106         : 16  ; /* [31:16] */
	} bits;

	unsigned int    u32;
};

union u_cmdlst_irq_state_raw {
	struct {
		unsigned int    cmdlst_irq_state_raw : 16  ; /* [15:0] */
		unsigned int    rsv_107              : 16  ; /* [31:16] */
	} bits;

	unsigned int    u32;
};

union u_cmdlst_irq_state_msk {
	struct {
		unsigned int    cmdlst_irq_state_mask : 16  ; /* [15:0] */
		unsigned int    rsv_108               : 16  ; /* [31:16] */
	} bits;

	unsigned int    u32;
};

#endif // __IPP_TOP_DRV_PRIV_H_CMDLST_
