/******************************************************************
 * Copyright    Copyright (c) 2021- Hisilicon Technologies CO., Ltd.
 * File name    cmdlst_manager.h
 * Description:
 *
 * Date         2022-07-07 14:23:09
 ********************************************************************/

#ifndef __CMDLST_MANAGER_H_INCLUDED__
#define __CMDLST_MANAGER_H_INCLUDED__

#include "cmdlst_common.h"

#define CMDLST_SET_REG_BY_CPU    (0)
#if defined(CMDLST_SET_REG_BY_CPU) && (CMDLST_SET_REG_BY_CPU == 1)
#define macro_cmdlst_set_reg(reg, val) cmdlst_set_reg_by_cpu(reg, val)
#else
#define macro_cmdlst_set_reg(reg, val) cmdlst_set_reg(dev->cmd_buf, reg, val)
#endif

#define macro_cmdlst_set_reg_incr(reg_addr, burst_size, incr_mode, cfg_mode) \
	cmdlst_set_reg_incr(dev->cmd_buf, reg_addr, burst_size, incr_mode, cfg_mode)
#define macro_cmdlst_set_reg_data(data)\
	cmdlst_set_reg_data(dev->cmd_buf, data)
#define macro_cmdlst_set_addr_align(align) \
	cmdlst_set_addr_align(dev->cmd_buf, align)
#define macro_cmdlst_set_reg_burst_data_align(reg_num, align) \
	cmdlst_set_reg_burst_data_align(dev->cmd_buf, reg_num, align)
#define macro_cmdlst_set_addr_offset(size_offset) \
	cmdlst_set_addr_offset(dev->cmd_buf, size_offset)

// Note: macro_cmdlst_set_reg_burst: is new add for r02_IPP and it refers to the function on the ISP.
#define macro_cmdlst_set_reg_burst(reg, size, incr, pdata) \
	cmdlst_set_reg_burst(dev->cmd_buf, reg, size, incr, pdata)

int  cmdlst_set_reg(cmd_buf_t *cmd_buf, unsigned int reg, unsigned int val);
void cmdlst_set_reg_by_cpu(unsigned int reg, unsigned int val);
void cmdlst_set_addr_align(cmd_buf_t *cmd_buf, unsigned int align);
void cmdlst_set_reg_burst_data_align(cmd_buf_t *cmd_buf, unsigned int reg_num, unsigned int align);
void cmdlst_set_reg_data(cmd_buf_t *cmd_buf, unsigned int data);
void cmdlst_set_addr_offset(cmd_buf_t *cmd_buf, unsigned int size_offset);
void cmdlst_set_reg_incr(cmd_buf_t *cmd_buf, unsigned int reg_addr,
						unsigned int burst_size, cmdlst_incr_mode_e incr_mode, cmdlst_cfg_mode_e cfg_mode);
int cmdlst_set_reg_burst(cmd_buf_t *cmd_buf, unsigned int reg, unsigned int size,
						unsigned int incr, unsigned int *pdata);
void cmdlst_buff_dump(cmd_buf_t *cmd_buf);


/**
 * @name        dequeue_channel_ready_list
 * @brief       dequeue cmdlst channel's ready_list;
 * @param[in]   void
 * @retval      0        init succeed
 * @retval      other    init failed, error code.
 */
void dequeue_channel_ready_list(unsigned int channel_id);


/**
 * @name        cmdlst_priv_init
 * @brief       Init cmdlst channel's ready_list: g_cmdlst_priv;
 * @param[in]   void
 * @retval      0        init succeed
 * @retval      other    init failed, error code.
 */
int cmdlst_priv_init(void);

/**
 * @name        cmdlst_priv_uninit
 * @brief       destroy the cmd_priv_lock;
 * @param[in]   void
 * @retval      0        init succeed
 * @retval      other    init failed, error code.
 */
int cmdlst_priv_uninit(void);

/**
 * @name   df_sched_prepare
 * @brief
 * @param[in]   cmdlst_para    the cmdlst para info
 * @retval      0        init succeed
 * @retval      other    init failed, error code.
 */
int df_sched_prepare(cmdlst_para_t *cmdlst_para);


/**
 * @name   df_sched_start
 * @brief  Enqueue cmdlst_buf and start cmdlst
 * @param[in]   cmdlst_para    the cmdlst para info
 * @retval      0        init succeed
 * @retval      other    init failed, error code.
 */
int df_sched_start(cmdlst_para_t *cmdlst_para);

/**
 * @name   ipp_eop_handler
 * @brief  Do eop process
 * @param[in]   mode
 * @retval      0        init succeed
 * @retval      other    init failed, error code.
 */
int ipp_eop_handler(unsigned int work_module);

#endif /* __CMDLST_MANAGER_H_INCLUDED__ */
