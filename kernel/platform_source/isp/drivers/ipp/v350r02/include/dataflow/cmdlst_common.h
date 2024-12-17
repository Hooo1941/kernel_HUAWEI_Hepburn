/******************************************************************
 * Copyright    Copyright (c) 2021- Hisilicon Technologies CO., Ltd.
 * File name    cmdlst_common.h
 * Description:
 *
 * Date         2022-07-07 14:23:09
 ********************************************************************/

#ifndef __CMDLST_COMMON_H_INCLUDED__
#define __CMDLST_COMMON_H_INCLUDED__

#include "cfg_table_cmdlst.h"
#include <securec.h>

#define CH_CFG_OFFSET              (0x80)
#define CMDLST_BUFFER_SIZE         0x10000
#define CMDLST_HEADER_SIZE         288 // 72w; same as v350 isp; 16w header+ 54w irq+ 2w padding
#define CMDLST_RD_CFG_MAX          8
#define CMDLST_32K_PAGE            32768
#define CMD_RD_DU_NUM              (6)
#define CMD_MEM_PAGE_SHIFT         (15)
#define CMD_RD_LINE_SIZE           (16)
#define CMD_RD_BITS                (64) // D64


/****************************************************************
 * Note:The cmdlst read operation supports only burst_size of 16 in R02,
 * Write operations still support burst_size 256.
 *****************************************************************/
#define CMDLST_BURST_MAX_SIZE      256 // for burst write reg
#define CMDLST_BURST_READ_MAX_SIZE 16  // for burst read reg
#define CMDLST_PADDING_DATA        (0xFF7FFFFD) // R02 changed
#define CMDLST_STRIPE_MAX_NUM      80

#define CMD_IRQ_CFG_OFFSET         (0x10)
#define CMD_ADDR_OFFSET(ID)        ((0x0) + ((ID) * (0x4)))
#define BYTES_PER_WORD             (4)

typedef enum {
	CMD_CFG_DATA_3 = 0,
	CMD_CFG_DATA_2_3,
	CMD_CFG_DATA_1_2_3,
	CMD_CFG_DATA_0_1_2_3,
	CMD_CFG_DATA_MAX,
} cmdlst_rd_size_e;

typedef struct _cmd_buf_t {
	unsigned long long start_addr;
	unsigned int       start_addr_isp_map;
	unsigned int       buffer_size;
	unsigned int       header_size;
	unsigned long long data_addr;
	unsigned int       data_size;
	void               *next_buffer;
} cmd_buf_t;

typedef union {
	struct {
		unsigned int    cfg_mode   : 2;  /* [1:0]   */
		unsigned int    addr_range : 20; /* [21:2]  */
		unsigned int    reserve    : 1;  /* [22]    */
		unsigned int    incr_mode  : 1;  /* [23]    */
		unsigned int    burst_size : 8;  /* [31:24] */
	} bits;

	unsigned int    u32;
} cmdlst_reg_cmd;

typedef enum _cmd_cfg_id_e {
	CMD_RESOURCE_ADDR = 0,
	CMD_RESOURCE_VALUE,
	CMD_TASK_ADDR,
	CMD_TASK_VALUE,
	CMD_DATA_3_ADDR,
	CMD_DATA_3_VALUE, // 5
	CMD_DATA_0_ADDR,
	CMD_DATA_0_VALUE,
	CMD_DATA_1_ADDR,
	CMD_DATA_1_VALUE,
	CMD_DATA_2_ADDR, // 10
	CMD_DATA_2_VALUE,
	CMD_CMD_0_ADDR,
	CMD_CMD_0_VALUE,
	CMD_TOKEN_0_ADDR,
	CMD_TOKEN_0_VALUE, // 15
	CMD_CFG_ID_MAX,
} cmd_cfg_id_e;


/****************************************************************
 * Note: The Cmdlst's MemeryCopy function is not supported in r02 IPPs.
 *****************************************************************/
typedef enum {
	WR_OPS_MODE = 0, // write reg.
	RD_OPS_MODE,     // read reg.
	CFG_MODE_MAX,
} cmdlst_cfg_mode_e;

/****************************************************************
 * Note: INCR_MODE:   For Reg is not Ram_Style; e.g: 0x0/0x4/0x8.....
 *       UNINCR_MODE: For Reg is Ram_Style; e.g: 0x0/0x0/0x0.....
 *****************************************************************/
typedef enum {
	INCR_MODE = 0,
	UNINCR_MODE,
	INCR_MODE_MAX,
} cmdlst_incr_mode_e;

typedef enum {
	CMD_PRIO_LOW  = 0,
	CMD_PRIO_HIGH = 1,
} cmdlst_frame_prio_e;

typedef enum {
	CMD_EOF_ORB_ENH_MODE = 0,
	CMD_EOF_ARFEATURE_MODE,
	CMD_EOF_RDR_MODE,
	CMD_EOF_CMP_MODE,
	CMD_EOF_MC_MODE,
	CMD_EOF_KLT_MODE, // 5
	CMD_EOF_MODE_MAX,
} cmdlst_eof_mode_e;

typedef enum {
	IPP_CMD_RES_SHARE_ORB_ENH = 0,
	IPP_CMD_RES_SHARE_ARFEATURE,
	IPP_CMD_RES_SHARE_RDR,
	IPP_CMD_RES_SHARE_CMP,
	IPP_CMD_RES_SHARE_MC,
	IPP_CMD_RES_SHARE_KLT,
} cmdlst_resource_share_e;

typedef enum {
	IPP_CMD_IRQ_MODULE_ORB_ENH = 0,
	IPP_CMD_IRQ_MODULE_ARFEATURE,
	IPP_CMD_IRQ_MODULE_RDR,
	IPP_CMD_IRQ_MODULE_CMP,
	IPP_CMD_IRQ_MODULE_MC,
	IPP_CMD_IRQ_MODULE_KLT,
} cmdlst_irq_module_e;

/****************************************************************
* [Note]: We have 6 sets of IRQ registers. They can be identified by the postfix.
* Postfix 0 is for ACPU;
* postfix 1 is for ISP_CPU;
* postfix 2 is for IVP; (Not used)
* postfix 3 is reserved;
* postfix 4 is for ARPP;(Not used)
* postfix 5 is reserved;
*****************************************************************/
typedef enum {
	IPP_CMDLST_IRQ_FOR_ACPU = 0,
	IPP_CMDLST_IRQ_FOR_ISPCPU,
	IPP_CMDLST_IRQ_MAX_NUM,
} cmdlst_irq_host_e;

typedef enum {
	CMDLST_CHANNEL_0 = 0,
	CMDLST_CHANNEL_1,
	CMDLST_CHANNEL_2,
	CMDLST_CHANNEL_3,
	CMDLST_CHANNEL_4,
	CMDLST_CHANNEL_5,
	CMDLST_CHANNEL_6,
	CMDLST_CHANNEL_7,
	CMDLST_CHANNEL_8,
	CMDLST_CHANNEL_9,
	CMDLST_CHANNEL_10,
	CMDLST_CHANNEL_11,
	CMDLST_CHANNEL_12,
	CMDLST_CHANNEL_13,
	CMDLST_CHANNEL_14,
	CMDLST_CHANNEL_15,
	CMDLST_CHANNEL_MAX,
} cmdlst_channel_id_e;

/**
* name: cmdlst_ctrl_chn_e:
* usage: Refer to CMDLST_CTRL_MAP reg; This is used to create the Mapping Between Channel and Module;
**/
typedef enum {
	IPP_CMDLST_SELECT_ORB_ENH = 0,
	IPP_CMDLST_SELECT_ARF     = 1,
	IPP_CMDLST_SELECT_RDR     = 2,
	IPP_CMDLST_SELECT_CMP     = 3,
	IPP_CMDLST_SELECT_MC      = 4,
	IPP_CMDLST_SELECT_GF      = 5, // (Not used)
	IPP_CMDLST_SELECT_KLT     = 6,
	IPP_CMDLST_SELECT_ICP     = 7, // (Not used)
	IPP_CMDLST_SELECT_HIOF    = 8, // (Not used)
} cmdlst_ctrl_chn_e;

typedef enum {
	CASCADE_DISABLE = 0,
	CASCADE_ENABLE = 1,
} ipp_path_cascade_en_e;

typedef struct _cmdlst_wr_cfg_t {
	unsigned int buff_wr_addr;
	unsigned int reg_rd_addr;
	unsigned int data_size;
	unsigned int is_incr;
	unsigned int is_wstrb;
	unsigned int read_mode; // read_mode=1 is for updataing mc's index
} cmdlst_wr_cfg_t;

typedef struct _schedule_cmdlst_link_t {
	unsigned int stripe_cnt;
	unsigned int stripe_index;
	cmd_buf_t    cmd_buf;
	cfg_tab_cmdlst_t cmdlst_cfg_tab;
	void *data;
	struct list_head list_link;
} schedule_cmdlst_link_t;

typedef struct _cmdlst_rd_cfg_info_t {
	// read buffer address
	unsigned int fs;
	// read reg num in one stripe
	unsigned int rd_cfg_num;
	// read reg cfgs
	unsigned int rd_cfg[CMDLST_RD_CFG_MAX];
} cmdlst_rd_cfg_info_t;

typedef struct _cmdlst_stripe_info_t {
	unsigned int  is_first_stripe;
	unsigned int  is_last_stripe;
	unsigned int  is_need_set_sop;
	unsigned int  irq_mode;
	unsigned int  irq_module; // change R02 irq_mode_sop
	unsigned int  hw_priority;
	unsigned int  resource_share;
	unsigned int  en_link;
	unsigned int  ch_link;
	unsigned int  ch_link_act_nbr;
	cmdlst_rd_cfg_info_t rd_cfg_info;
} cmdlst_stripe_info_t;

typedef struct _cmdlst_para_t {
	unsigned int stripe_cnt;
	cmdlst_stripe_info_t cmd_stripe_info[CMDLST_STRIPE_MAX_NUM];
	void *cmd_entry;
	unsigned int channel_id;

	unsigned int work_module;
	unsigned int cmd_entry_mem_id; // new add
	unsigned int cmd_buf_mem_id; // new add
} cmdlst_para_t;


#endif /* __CMDLST_COMMON_H_INCLUDED__ */
