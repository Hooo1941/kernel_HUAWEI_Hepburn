/*
 * Copyright     :  Copyright (C) 2021, Huawei Technologies Co. Ltd.
 * File name     :  cmdlst_ipp_drv_priv.h
 * Project line  :  
 * Department    :  K5
 * Author        :  AnthonySixta
 * Version       :  1.0
 * Date          :  
 * Description   :  The description of xxx project
 * Others        :  Generated automatically by nManager V5.1 
 * History       :  AnthonySixta 2021/11/19 10:56:06 Create file
 */

#ifndef __CMDLST_IPP_DRV_PRIV_H__
#define __CMDLST_IPP_DRV_PRIV_H__

union cmdlst_st_ld {
	struct {
		unsigned int    store_load : 32  ; /* [31:0] */
	} bits;

	unsigned int    u32;
};

union cmdlst_start_addr {
	struct {
		unsigned int    start_addr : 32  ; /* [31:0] */
	} bits;

	unsigned int    u32;
};

union cmdlst_end_addr {
	struct {
		unsigned int    end_addr : 32  ; /* [31:0] */
	} bits;

	unsigned int    u32;
};

union cmdlst_check_byp_start_addr {
	struct {
		unsigned int    check_byp_start_addr : 32  ; /* [31:0] */
	} bits;

	unsigned int    u32;
};

union cmdlst_check_byp_end_addr {
	struct {
		unsigned int    check_byp_end_addr : 32  ; /* [31:0] */
	} bits;

	unsigned int    u32;
};

union cmdlst_trans_start {
	struct {
		unsigned int    translation_addr_start : 22  ; /* [21:0] */
		unsigned int    rsv_0                  : 10  ; /* [31:22] */
	} bits;

	unsigned int    u32;
};

union cmdlst_trans_end {
	struct {
		unsigned int    translation_addr_end : 22  ; /* [21:0] */
		unsigned int    rsv_1                : 10  ; /* [31:22] */
	} bits;

	unsigned int    u32;
};

union cmdlst_trans_offset {
	struct {
		unsigned int    translation_addr_offset : 23  ; /* [22:0] */
		unsigned int    rsv_2                   : 9  ; /* [31:23] */
	} bits;

	unsigned int    u32;
};

union cmdlst_cfg {
	struct {
		unsigned int    max_rd_burst_length : 4  ; /* [3:0] */
		unsigned int    rsv_3               : 28  ; /* [31:4] */
	} bits;

	unsigned int    u32;
};

union cmdlst_check_error_status_0 {
	struct {
		unsigned int    check_error_status_0 : 32  ; /* [31:0] */
	} bits;

	unsigned int    u32;
};

union cmdlst_check_error_status_1 {
	struct {
		unsigned int    check_error_status_1 : 32  ; /* [31:0] */
	} bits;

	unsigned int    u32;
};

union cmdlst_cmdlst_debug_0 {
	struct {
		unsigned int    dbg_fifo_nb_elem  : 3  ; /* [2:0] */
		unsigned int    rsv_4             : 1  ; /* [3] */
		unsigned int    dbg_lb_master_fsm : 4  ; /* [7:4] */
		unsigned int    dbg_vp_wr_fsm     : 2  ; /* [9:8] */
		unsigned int    dbg_vp_rd_fsm     : 2  ; /* [11:10] */
		unsigned int    dbg_arb_fsm       : 1  ; /* [12] */
		unsigned int    rsv_5             : 19  ; /* [31:13] */
	} bits;

	unsigned int    u32;
};

union cmdlst_cmdlst_debug_1 {
	struct {
		unsigned int    last_cmd_exec : 32  ; /* [31:0] */
	} bits;

	unsigned int    u32;
};

union cmdlst_cmdlst_debug_2 {
	struct {
		unsigned int    dbg_sw_start : 16  ; /* [15:0] */
		unsigned int    dbg_hw_start : 16  ; /* [31:16] */
	} bits;

	unsigned int    u32;
};

union cmdlst_cmdlst_debug_3 {
	struct {
		unsigned int    dbg_sw_start : 16  ; /* [15:0] */
		unsigned int    dbg_hw_start : 16  ; /* [31:16] */
	} bits;

	unsigned int    u32;
};

union cmdlst_cmdlst_debug_4 {
	struct {
		unsigned int    dbg_fsm_ch_0 : 4  ; /* [3:0] */
		unsigned int    dbg_fsm_ch_1 : 4  ; /* [7:4] */
		unsigned int    dbg_fsm_ch_2 : 4  ; /* [11:8] */
		unsigned int    dbg_fsm_ch_3 : 4  ; /* [15:12] */
		unsigned int    dbg_fsm_ch_4 : 4  ; /* [19:16] */
		unsigned int    dbg_fsm_ch_5 : 4  ; /* [23:20] */
		unsigned int    dbg_fsm_ch_6 : 4  ; /* [27:24] */
		unsigned int    dbg_fsm_ch_7 : 4  ; /* [31:28] */
	} bits;

	unsigned int    u32;
};

union cmdlst_ch_cfg {
	struct {
		unsigned int    active_token_nbr        : 8  ; /* [7:0] */
		unsigned int    active_token_nbr_enable : 1  ; /* [8] */
		unsigned int    rsv_6                   : 23  ; /* [31:9] */
	} bits;

	unsigned int    u32;
};

union cmdlst_check_error_ptr {
	struct {
		unsigned int    check_error_ptr : 32  ; /* [31:0] */
	} bits;

	unsigned int    u32;
};

union cmdlst_check_error_task {
	struct {
		unsigned int    check_error_task : 10  ; /* [9:0] */
		unsigned int    rsv_7            : 22  ; /* [31:10] */
	} bits;

	unsigned int    u32;
};

union cmdlst_check_error_cmd {
	struct {
		unsigned int    check_error_cmd : 32  ; /* [31:0] */
	} bits;

	unsigned int    u32;
};

union cmdlst_sw_branch {
	struct {
		unsigned int    branching : 1  ; /* [0] */
		unsigned int    rsv_8     : 31  ; /* [31:1] */
	} bits;

	unsigned int    u32;
};

union cmdlst_last_exec_ptr {
	struct {
		unsigned int    last_exec_ptr : 32  ; /* [31:0] */
	} bits;

	unsigned int    u32;
};

union cmdlst_last_exec_task {
	struct {
		unsigned int    last_exec_task : 10  ; /* [9:0] */
		unsigned int    rsv_9          : 22  ; /* [31:10] */
	} bits;

	unsigned int    u32;
};

union cmdlst_sw_resources {
	struct {
		unsigned int    sw_resource : 31  ; /* [30:0] */
		unsigned int    sw_priority : 1  ; /* [31] */
	} bits;

	unsigned int    u32;
};

union cmdlst_sw_task {
	struct {
		unsigned int    sw_task : 10  ; /* [9:0] */
		unsigned int    rsv_10  : 22  ; /* [31:10] */
	} bits;

	unsigned int    u32;
};

union cmdlst_sw_cvdr_rd_addr {
	struct {
		unsigned int    reserved_sw_0      : 2  ; /* [1:0] */
		unsigned int    sw_cvdr_rd_address : 20  ; /* [21:2] */
		unsigned int    reserved_sw_1      : 2  ; /* [23:22] */
		unsigned int    sw_cvdr_rd_size    : 2  ; /* [25:24] */
		unsigned int    reserved_sw_2      : 6  ; /* [31:26] */
	} bits;

	unsigned int    u32;
};

union cmdlst_sw_cvdr_rd_data_0 {
	struct {
		unsigned int    sw_vprd_pixel_format    : 5  ; /* [4:0] */
		unsigned int    sw_vprd_pixel_expansion : 1  ; /* [5] */
		unsigned int    sw_vprd_allocated_du    : 5  ; /* [10:6] */
		unsigned int    sw_reserved_0           : 2  ; /* [12:11] */
		unsigned int    sw_vprd_last_page       : 19  ; /* [31:13] */
	} bits;

	unsigned int    u32;
};

union cmdlst_sw_cvdr_rd_data_1 {
	struct {
		unsigned int    sw_vprd_line_size : 15  ; /* [14:0] */
		unsigned int    sw_reserved_2     : 9  ; /* [23:15] */
		unsigned int    sw_vprd_ihleft    : 6  ; /* [29:24] */
		unsigned int    sw_reserved_1     : 2  ; /* [31:30] */
	} bits;

	unsigned int    u32;
};

union cmdlst_sw_cvdr_rd_data_2 {
	struct {
		unsigned int    sw_vprd_frame_size : 15  ; /* [14:0] */
		unsigned int    sw_reserved_3      : 17  ; /* [31:15] */
	} bits;

	unsigned int    u32;
};

union cmdlst_sw_cvdr_rd_data_3 {
	struct {
		unsigned int    sw_reserved_5           : 2  ; /* [1:0] */
		unsigned int    sw_vprd_axi_frame_start : 30  ; /* [31:2] */
	} bits;

	unsigned int    u32;
};

union cmdlst_hw_resources {
	struct {
		unsigned int    hw_resource : 31  ; /* [30:0] */
		unsigned int    hw_priority : 1  ; /* [31] */
	} bits;

	unsigned int    u32;
};

union cmdlst_hw_task {
	struct {
		unsigned int    hw_task : 10  ; /* [9:0] */
		unsigned int    rsv_11  : 22  ; /* [31:10] */
	} bits;

	unsigned int    u32;
};

union cmdlst_hw_cvdr_rd_addr {
	struct {
		unsigned int    reserved_hw_0      : 2  ; /* [1:0] */
		unsigned int    hw_cvdr_rd_address : 20  ; /* [21:2] */
		unsigned int    reserved_hw_1      : 2  ; /* [23:22] */
		unsigned int    hw_cvdr_rd_size    : 2  ; /* [25:24] */
		unsigned int    reserved_hw_2      : 6  ; /* [31:26] */
	} bits;

	unsigned int    u32;
};

union cmdlst_hw_cvdr_rd_data_0 {
	struct {
		unsigned int    hw_vprd_pixel_format    : 5  ; /* [4:0] */
		unsigned int    hw_vprd_pixel_expansion : 1  ; /* [5] */
		unsigned int    hw_vprd_allocated_du    : 5  ; /* [10:6] */
		unsigned int    hw_reserved_0           : 2  ; /* [12:11] */
		unsigned int    hw_vprd_last_page       : 19  ; /* [31:13] */
	} bits;

	unsigned int    u32;
};

union cmdlst_hw_cvdr_rd_data_1 {
	struct {
		unsigned int    hw_vprd_line_size : 15  ; /* [14:0] */
		unsigned int    hw_reserved_2     : 9  ; /* [23:15] */
		unsigned int    hw_vprd_ihleft    : 6  ; /* [29:24] */
		unsigned int    hw_reserved_1     : 2  ; /* [31:30] */
	} bits;

	unsigned int    u32;
};

union cmdlst_hw_cvdr_rd_data_2 {
	struct {
		unsigned int    hw_vprd_frame_size : 15  ; /* [14:0] */
		unsigned int    hw_reserved_3      : 17  ; /* [31:15] */
	} bits;

	unsigned int    u32;
};

union cmdlst_hw_cvdr_rd_data_3 {
	struct {
		unsigned int    hw_reserved_5           : 2  ; /* [1:0] */
		unsigned int    hw_vprd_axi_frame_start : 30  ; /* [31:2] */
	} bits;

	unsigned int    u32;
};

union cmdlst_cmd_cfg {
	struct {
		unsigned int    wait_eop : 1  ; /* [0] */
		unsigned int    rsv_12   : 31  ; /* [31:1] */
	} bits;

	unsigned int    u32;
};

union cmdlst_token_cfg {
	struct {
		unsigned int    token_mode     : 1  ; /* [0] */
		unsigned int    rsv_13         : 7  ; /* [7:1] */
		unsigned int    link_channel   : 5  ; /* [12:8] */
		unsigned int    rsv_14         : 3  ; /* [15:13] */
		unsigned int    link_token_nbr : 4  ; /* [19:16] */
		unsigned int    rsv_15         : 12  ; /* [31:20] */
	} bits;

	unsigned int    u32;
};

union cmdlst_eos {
	struct {
		unsigned int    line_nbr     : 12  ; /* [11:0] */
		unsigned int    rsv_16       : 12  ; /* [23:12] */
		unsigned int    line_ovl_nbr : 7  ; /* [30:24] */
		unsigned int    rsv_17       : 1  ; /* [31] */
	} bits;

	unsigned int    u32;
};

union cmdlst_check_ctrl{
	struct {
		unsigned int    check_enable : 1  ; /* [0] */
		unsigned int    rsv_18       : 31  ; /* [31:1] */
	} bits;

	unsigned int    u32;
};

union cmdlst_vp_wr_flush{
	struct {
		unsigned int    vp_wr_flush : 1  ; /* [0] */
		unsigned int    rsv_19      : 31  ; /* [31:1] */
	} bits;

	unsigned int    u32;
};

union cmdlst_cmdlst_debug_eos{
	struct {
		unsigned int    eos_dbg : 32  ; /* [31:0] */
	} bits;

	unsigned int    u32;
};

union cmdlst_cmdlst_debug_channel{
	struct {
		unsigned int    dbg_fsm_ch   : 4  ; /* [3:0] */
		unsigned int    dbg_sw_start : 1  ; /* [4] */
		unsigned int    rsv_20       : 3  ; /* [7:5] */
		unsigned int    dbg_hw_start : 1  ; /* [8] */
		unsigned int    rsv_21       : 23  ; /* [31:9] */
	} bits;

	unsigned int    u32;
};

// ==============================================================================
/* Define the global struct */
typedef struct {
	union cmdlst_st_ld                st_ld                 ; /* 0 */
	union cmdlst_start_addr           start_addr            ; /* 4 */
	union cmdlst_end_addr             end_addr              ; /* 8 */
	union cmdlst_check_byp_start_addr check_byp_start_addr  ; /* c */
	union cmdlst_check_byp_end_addr   check_byp_end_addr    ; /* 10 */
	union cmdlst_trans_start          trans_start           ; /* 14 */
	union cmdlst_trans_end            trans_end             ; /* 18 */
	union cmdlst_trans_offset         trans_offset          ; /* 1c */
	union cmdlst_cfg                  cfg                   ; /* 2c */
	union cmdlst_check_error_status_0 check_error_status_0  ; /* 3c */
	union cmdlst_check_error_status_1 check_error_status_1  ; /* 40 */
	union cmdlst_cmdlst_debug_0       debug_0               ; /* 48 */
	union cmdlst_cmdlst_debug_1       debug_1               ; /* 4c */
	union cmdlst_cmdlst_debug_2       debug_2               ; /* 50 */
	union cmdlst_cmdlst_debug_3       debug_3               ; /* 54 */
	union cmdlst_cmdlst_debug_4       debug_4[4]            ; /* 58 */
	union cmdlst_ch_cfg               ch_cfg[16]            ; /* 80 */
	union cmdlst_check_error_ptr      check_error_ptr[16]   ; /* 84 */
	union cmdlst_check_error_task     check_error_task[16]  ; /* 88 */
	union cmdlst_check_error_cmd      check_error_cmd[16]   ; /* 8c */
	union cmdlst_sw_branch            sw_branch[16]         ; /* 90 */
	union cmdlst_last_exec_ptr        last_exec_ptr[16]     ; /* 94 */
	union cmdlst_last_exec_task       last_exec_task[16]    ; /* 98 */
	union cmdlst_sw_resources         sw_resources[16]      ; /* 9c */
	union cmdlst_sw_task              sw_task[16]           ; /* a0 */
	union cmdlst_sw_cvdr_rd_addr      sw_cvdr_rd_addr[16]   ; /* a4 */
	union cmdlst_sw_cvdr_rd_data_0    sw_cvdr_rd_data_0[16] ; /* a8 */
	union cmdlst_sw_cvdr_rd_data_1    sw_cvdr_rd_data_1[16] ; /* ac */
	union cmdlst_sw_cvdr_rd_data_2    sw_cvdr_rd_data_2[16] ; /* b0 */
	union cmdlst_sw_cvdr_rd_data_3    sw_cvdr_rd_data_3[16] ; /* b4 */
	union cmdlst_hw_resources         hw_resources[16]      ; /* b8 */
	union cmdlst_hw_task              hw_task[16]           ; /* bc */
	union cmdlst_hw_cvdr_rd_addr      hw_cvdr_rd_addr[16]   ; /* c0 */
	union cmdlst_hw_cvdr_rd_data_0    hw_cvdr_rd_data_0[16] ; /* c4 */
	union cmdlst_hw_cvdr_rd_data_1    hw_cvdr_rd_data_1[16] ; /* c8 */
	union cmdlst_hw_cvdr_rd_data_2    hw_cvdr_rd_data_2[16] ; /* cc */
	union cmdlst_hw_cvdr_rd_data_3    hw_cvdr_rd_data_3[16] ; /* d0 */
	union cmdlst_cmd_cfg              cmd_cfg[16]           ; /* d4 */
	union cmdlst_token_cfg            token_cfg[16]         ; /* d8 */
	union cmdlst_eos                  eos[16]               ; /* dc */
	union cmdlst_check_ctrl           check_ctrl[16]        ; /* e0 */
	union cmdlst_vp_wr_flush          vp_wr_flush[16]       ; /* e8 */
	union cmdlst_cmdlst_debug_eos     debug_eos[16]         ; /* ec */
	union cmdlst_cmdlst_debug_channel debug_channel[16]     ; /* f0 */
} s_cmdlst_ipp_regs_type;

#endif // __CMDLST_IPP_DRV_PRIV_H__
