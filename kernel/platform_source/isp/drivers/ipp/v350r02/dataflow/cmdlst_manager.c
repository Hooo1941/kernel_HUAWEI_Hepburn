/******************************************************************
 * Copyright (c) Hisilicon Technologies Co., Ltd. 2012-2018. All rights reserved.
 * File name    cmdlst_manager.c
 * Description:
 *
 * Date         2022-07-04 16:28:10
 ********************************************************************/
#include <linux/types.h>
#include <linux/io.h>
#include <linux/module.h>
#include <linux/delay.h>
#include "ippmessage_common.h"
#include "cmdlst_common.h"
#include "memory.h"
#include "cmdlst_drv.h"
#include "cmdlst_reg_offset.h"
#include "cvdr_drv.h"
#include "cvdr_ipp_reg_offset.h"
#include "ipp_top_reg_offset.h"

#define MEM_PAGE_SHIFT        (15)
#define ALIGNMENT_128_BYTE    (128)

typedef struct _cmdlst_channel_t {
	struct list_head ready_list;
} cmdlst_channel_t;

typedef struct _cmdlst_priv_t {
	cmdlst_channel_t cmdlst_chan[CMDLST_CHANNEL_MAX];
	struct mutex     cmd_priv_lock;
} cmdlst_priv_t;

static cmdlst_priv_t g_cmdlst_priv;
unsigned int frame_num;

// This part is used to print DIO during the UT test.
#ifdef IPP_UT_DIO_DEBUG
typedef struct _reg_val_t {
	unsigned int reg_val;
	unsigned char flag;
} reg_val_t;

reg_val_t reg_val[MAX_DUMP_STRIPE_CNT][IPP_MAX_REG_OFFSET / 4 + 1] = {0};
unsigned int ut_module_addr = 0;
unsigned int ut_register_num = 0;
unsigned int ut_reg_addr_incr = 0;
unsigned int ut_reg_incr_num = 0;
unsigned int ut_reg_count = 0;
unsigned int ut_input_format = 0;

void set_dump_register_init(unsigned int addr, unsigned int offset, unsigned int input_fmt)
{
	ut_module_addr = addr;
	ut_register_num = 1 + offset / 4;
	ut_input_format = input_fmt;
	loge_if(memset_s(reg_val, sizeof(reg_val_t)*MAX_DUMP_STRIPE_CNT * ut_register_num, 0,
			 sizeof(reg_val_t)*MAX_DUMP_STRIPE_CNT * ut_register_num));
}

void get_register_info(unsigned int reg, unsigned int val)
{
	if (((reg & ut_module_addr) == ut_module_addr)
		&& ((reg - ut_module_addr) <= (ut_register_num * 4))) {
		for (int i = 0; i < MAX_DUMP_STRIPE_CNT; i++) {
			if (reg_val[i][(reg - ut_module_addr) / 4].flag == 0) {
				reg_val[i][(reg - ut_module_addr) / 4].reg_val = val;
				reg_val[i][(reg - ut_module_addr) / 4].flag = 1;
				break;
			}
		}
	}
}

void get_register_incr_info(unsigned int reg, unsigned int reg_num)
{
	if ((reg & ut_module_addr) == ut_module_addr) {
		ut_reg_addr_incr = reg;
		ut_reg_incr_num = reg_num;
		ut_reg_count = 0;
	}
}

void get_register_incr_data(unsigned int data)
{
	unsigned int index;
	index = ((ut_reg_addr_incr - ut_module_addr) / 4) + ut_reg_count;

	if (((ut_reg_addr_incr & ut_module_addr) == ut_module_addr)
		&& (ut_reg_count < ut_reg_incr_num)
		&& (index < (IPP_MAX_REG_OFFSET / 4 + 1))) {
		for (int i = 0; i < MAX_DUMP_STRIPE_CNT; i++) {
			if (reg_val[i][index].flag == 0) {
				reg_val[i][index].reg_val = data;
				reg_val[i][index].flag = 1;
				break;
			}
		}

		ut_reg_count++;
	}
}

void ut_dump_register(unsigned int stripe_cnt)
{
	logi("----------------start_dump_dio----------------");
	pr_err("START_NUM                          0\n");
	pr_err("END_NUM                            %d\n", stripe_cnt);

	for (int j = 0; j < stripe_cnt; j++) {
		pr_err("SUB_FRAME_INDEX                    %d\n", j);

		for (int i = 0; i < ut_register_num; i++)
			pr_err("1    %06X    %08X\n", i * 4, reg_val[j][i].reg_val);

		pr_err("1    %06X    %08X\n", 0xFFFFFE, 1);
	}

	memset_s(reg_val, sizeof(reg_val_t)*MAX_DUMP_STRIPE_CNT * ut_register_num, 0,
			 sizeof(reg_val_t)*MAX_DUMP_STRIPE_CNT * ut_register_num);
	logi("----------------end_dump_dio----------------");
}
#endif

void cmdlst_set_reg_by_cpu(unsigned int reg, unsigned int val) // need check, liang
{
	unsigned int temp_reg;
	temp_reg = ((reg) & 0x000fffff);

	if (temp_reg >= 0x4000 && temp_reg < 0x5000)
		hispcpe_reg_set(CPE_TOP, ((temp_reg) & 0x00000fff), val);
	else if (temp_reg >= 0x5000 && temp_reg < 0x6000)
		hispcpe_reg_set(CMDLIST_REG, ((temp_reg) & 0x00000fff), val);
	else if (temp_reg >= 0x6000 && temp_reg < 0x8000)
		hispcpe_reg_set(CVDR_REG, ((temp_reg) & 0x00000fff), val);
}

static void dump_addr(unsigned long long addr, unsigned int start_addr_isp_map, unsigned int num, char *info)
{
	int i = 0;
	int data_perline = 16;
	logi("%s Dump ......, num=%d", info, num);

	for (i = 0; i < num; i += data_perline)
		logi("0x%llx(0x%08x): 0x%08x 0x%08x 0x%08x 0x%08x",
			 addr + i,
			 start_addr_isp_map + i,
			 readl((void __iomem *)(uintptr_t)(addr + i + 0x00)),
			 readl((void __iomem *)(uintptr_t)(addr + i + 0x04)),
			 readl((void __iomem *)(uintptr_t)(addr + i + 0x08)),
			 readl((void __iomem *)(uintptr_t)(addr + i + 0x0C)));
}

void cmdlst_buff_dump(cmd_buf_t *cmd_buf)
{
	if (cmd_buf == NULL) {
		logi("cmd_buf null");
		return;
	}

	logi("CMDLST BUFF: Size:0x%x, Addr(VA): 0x%llx, start_addr_isp_map(DA):0x%llx",
		 cmd_buf->data_size, cmd_buf->start_addr, (unsigned long long)(cmd_buf->start_addr_isp_map));
	dump_addr(cmd_buf->start_addr, cmd_buf->start_addr_isp_map, cmd_buf->data_size, "cmdlst_buff");
}

void cmdlst_set_addr_align(cmd_buf_t *cmd_buf, unsigned int align)
{
	unsigned int i = 0;
	unsigned int size = 2;
	unsigned int padding_index = 0;

	if (align == 0) {
		loge("Failed : align is equal to zero: %d", align);
		return;
	}

	padding_index =
		(align - ((cmd_buf->data_addr + (size + 2) * 4) % align)) / 4;
	padding_index = (padding_index == 4) ? 0 : padding_index;

	if ((cmd_buf->data_size + padding_index * 4) > cmd_buf->buffer_size) {
		loge("Failed : cmdlst buffer is full: %u, padding_index = %d",
			 cmd_buf->data_size, padding_index);
		return;
	}

	if ((cmd_buf->data_size + (padding_index * BYTES_PER_WORD)) > cmd_buf->buffer_size) {
		loge("Failed : cmdlst buffer is full: %u", cmd_buf->data_size);
		return;
	}

	for (i = 0; i < padding_index; i++) {
		*(unsigned int *)(uintptr_t)(cmd_buf->data_addr) = CMDLST_PADDING_DATA;
		cmd_buf->data_addr += 4;
		cmd_buf->data_size += 4;
	}
}

void cmdlst_set_reg_burst_data_align(cmd_buf_t *cmd_buf, unsigned int reg_num, unsigned int align)
{
	unsigned int padding_num = 0;
	unsigned int burst_cnt = 0;
	unsigned int data_size = 0;
	unsigned long long tmp = 0xF;
	unsigned int i = 0;

	if (((cmd_buf->data_addr & tmp) != 0) || (reg_num == 0)) {
		logi("This function does not work; input para: reg_num = %u", reg_num);
		return;
	}

	burst_cnt = (reg_num + (CMDLST_BURST_MAX_SIZE - 1)) / CMDLST_BURST_MAX_SIZE;
	padding_num = (4 - ((reg_num + burst_cnt) % 4));
	padding_num = (padding_num == 4) ? 0 : padding_num;
	data_size = (reg_num + burst_cnt) * 4;
	tmp = ((cmd_buf->data_addr + data_size - 1) >> 4) << 4;

	if (((tmp + 0x10) & (align - 1)) != 0)
		padding_num = padding_num + 4;

	if ((cmd_buf->data_size + (padding_num * BYTES_PER_WORD)) > cmd_buf->buffer_size) {
		loge("Failed : cmdlst buffer is full: %u", cmd_buf->data_size);
		return;
	}

	for (i = 0; i < padding_num; i++) {
		*(unsigned int *)(uintptr_t)(cmd_buf->data_addr) = CMDLST_PADDING_DATA;
		cmd_buf->data_addr += 4;
		cmd_buf->data_size += 4;
	}

	return;
}

/********************************************************************************************
 * function name: cmdlst_set_reg
 * description: fill current cmdlst buf
 * input:
 *     cmd_buf: current cmdlst buf to config.
 *     reg    : reg_addr
 *     value  : reg_val
 * return:NULL
 *********************************************************************************************/
int cmdlst_set_reg(cmd_buf_t *cmd_buf, unsigned int reg, unsigned int val)
{
	int ret = 0;
	loge_if_null_set_ret(cmd_buf, ret);
	if (ret) return ret;

	if ((cmd_buf->data_size + 8) <= cmd_buf->buffer_size) {
		cmdlst_reg_cmd *cmd_obj = (cmdlst_reg_cmd*)(uintptr_t)(cmd_buf->data_addr); // new cmd command type -r02
		cmd_obj->bits.cfg_mode   = WR_OPS_MODE;
		cmd_obj->bits.addr_range = (reg - JPG_SUBSYS_BASE_ADDR) >> 2; // new r02
		cmd_obj->bits.incr_mode  = INCR_MODE;
		cmd_obj->bits.burst_size = 0; // means 1;
		cmd_buf->data_addr += BYTES_PER_WORD;
		cmd_buf->data_size += BYTES_PER_WORD;

		*(unsigned int *)(uintptr_t)(cmd_buf->data_addr) = (val);
		cmd_buf->data_addr += BYTES_PER_WORD;
		cmd_buf->data_size += BYTES_PER_WORD;

#ifdef IPP_UT_DIO_DEBUG
		get_register_info(reg, val);
#endif
	} else {
		loge("failed: cmdlst buffer(buffer_size.0x%x) is full: data_size.0x%x",
			cmd_buf->buffer_size, cmd_buf->data_size);
		return ISP_IPP_ERR;
	}

	return ISP_IPP_OK;
}

/********************************************************************************************
 * function name: cmdlst_set_reg_incr
 * description: current cmdlst buf
 * input:
 *     cmd_buf    : current cmdlst buf to config.
 *     reg_addr   : register start address
 *     burst_size : register numbers
 *     incr       : register address increment or not
 *     cfg_mode   : 0- write the reg, 1- read the reg
 * return: NULL
 ********************************************************************************************/
void cmdlst_set_reg_incr(cmd_buf_t *cmd_buf, unsigned int reg_addr,
						unsigned int burst_size, cmdlst_incr_mode_e incr_mode, cmdlst_cfg_mode_e cfg_mode)
{
	cmdlst_reg_cmd *cmd_obj = NULL;
	if ((cmd_buf->data_size + (burst_size + 1) * 4) > cmd_buf->buffer_size) {
		loge("Failed : cmdlst buffer is full: %u", cmd_buf->data_size);
		return;
	}

#ifdef IPP_UT_DIO_DEBUG
	get_register_incr_info(reg_addr, burst_size);
#endif

	// New command description (for R02):
	cmd_obj = (cmdlst_reg_cmd *)(uintptr_t)(cmd_buf->data_addr);
	cmd_obj->bits.cfg_mode   = cfg_mode;
	cmd_obj->bits.addr_range = (reg_addr - JPG_SUBSYS_BASE_ADDR) >> 2;
	cmd_obj->bits.incr_mode  = incr_mode;
	cmd_obj->bits.burst_size = burst_size - 1;

	cmd_buf->data_addr += BYTES_PER_WORD;
	cmd_buf->data_size += BYTES_PER_WORD;
}

/********************************************************************************************
 * function name: cmdlst_set_reg_data
 * description: Set the data to cmdlst_buff directly;
 * input:
 *     cmd_buf: current cmdlst buf to config.
 *     data   : register value
 * return: NULL
 ********************************************************************************************/
void cmdlst_set_reg_data(cmd_buf_t *cmd_buf, unsigned int data)
{
	if ((cmd_buf->data_size + BYTES_PER_WORD) > cmd_buf->buffer_size) {
		loge("Failed : cmdlst buffer is full: %u", cmd_buf->data_size);
		return;
	}

	*(unsigned int *)(uintptr_t)(cmd_buf->data_addr) = data;
	cmd_buf->data_addr += BYTES_PER_WORD;
	cmd_buf->data_size += BYTES_PER_WORD;

#ifdef IPP_UT_DIO_DEBUG
	get_register_incr_data(data);
#endif
}

void cmdlst_set_addr_offset(cmd_buf_t *cmd_buf, unsigned int size_offset)
{
	cmd_buf->data_addr += BYTES_PER_WORD * size_offset;
	cmd_buf->data_size += BYTES_PER_WORD * size_offset;
}

int cmdlst_set_reg_burst(cmd_buf_t *cmd_buf, unsigned int reg, unsigned int size,
						  unsigned int incr, unsigned int *pdata) // new add
{
	if (cmd_buf == NULL || pdata == NULL) {
		loge("ptr is null");
		return ISP_IPP_ERR;
	}

	int ret = 0;
	unsigned int i, cnt;

	for (i = 0; i < size; i += CMDLST_BURST_MAX_SIZE) {
		cnt = size - i;

		if (cnt > CMDLST_BURST_MAX_SIZE)
			cnt = CMDLST_BURST_MAX_SIZE;

		if (incr == UNINCR_MODE) {
			cmdlst_set_reg_incr(cmd_buf, reg, cnt, incr, WR_OPS_MODE);
		} else {
			cmdlst_set_reg_incr(cmd_buf, (reg + i * BYTES_PER_WORD), cnt, incr, WR_OPS_MODE);
#ifdef IPP_UT_DIO_DEBUG
			for (int j = 0; j < cnt; j++)
				get_register_info((reg + (i * BYTES_PER_WORD) + (4 * j)), *(pdata + j + i));
#endif
		}

		ret = memcpy_s((unsigned int *)(uintptr_t)cmd_buf->data_addr, cnt * BYTES_PER_WORD, (pdata + i),
			cnt * BYTES_PER_WORD);
		if (ret != 0) {
			loge("Failed : do memcpy_s err");
			return ISP_IPP_ERR;
		}
		cmdlst_set_addr_offset(cmd_buf, cnt);
	}

	return ISP_IPP_OK;
}

/********************************************************************************************
 * @function name: cmdlst_priv_init
 * @description: init channel_list, and cmd_priv_lock
 * @input: NULL
 * @return: 0
 *********************************************************************************************/
int cmdlst_priv_init(void)
{
	unsigned int i;

	for (i = 0; i < CMDLST_CHANNEL_MAX; i++)
		INIT_LIST_HEAD(&g_cmdlst_priv.cmdlst_chan[i].ready_list);

	mutex_init(&g_cmdlst_priv.cmd_priv_lock);
	return ISP_IPP_OK;
}

/********************************************************************************************
 * @function name: cmdlst_priv_uninit
 * @description: destroy cmd_priv_lock
 * @input: NULL
 * @return: 0
 *********************************************************************************************/
int cmdlst_priv_uninit(void)
{
	mutex_destroy(&g_cmdlst_priv.cmd_priv_lock);
	return ISP_IPP_OK;
}

void dequeue_channel_ready_list(unsigned int channel_id)
{
	struct list_head *ready_list = NULL;
	mutex_lock(&g_cmdlst_priv.cmd_priv_lock);

	ready_list = &g_cmdlst_priv.cmdlst_chan[channel_id].ready_list;
	while (list_empty(ready_list) == 0)
		list_del(ready_list->next);

	mutex_unlock(&g_cmdlst_priv.cmd_priv_lock);
}

 /********************************************************************************************
 * function name: ipp_eop_handler
 * description: eop handler to dequeue a done-frame
 * input: NULL
 * return: 0
 *********************************************************************************************/
int ipp_eop_handler(unsigned int work_module)
{
	switch (work_module) {
	case ENH_ONLY:
	case ARFEATURE_ONLY:
	case ENH_ARF:
		cpe_mem_free(MEM_ID_CMDLST_BUF_ENH_ARF,   work_module);
		cpe_mem_free(MEM_ID_CMDLST_ENTRY_ENH_ARF, work_module);
		cpe_mem_free(MEM_ID_CMDLST_PARA_ENH_ARF,  work_module);
		break;
	case MATCHER_ONLY:
		cpe_mem_free(MEM_ID_CMDLST_BUF_REORDER,   work_module);
		cpe_mem_free(MEM_ID_CMDLST_ENTRY_REORDER, work_module);
		cpe_mem_free(MEM_ID_CMDLST_PARA_REORDER,  work_module);
		cpe_mem_free(MEM_ID_CMDLST_BUF_COMPARE,   work_module);
		cpe_mem_free(MEM_ID_CMDLST_ENTRY_COMPARE, work_module);
		cpe_mem_free(MEM_ID_CMDLST_PARA_COMPARE,  work_module);
		break;
	case MC_ONLY:
		cpe_mem_free(MEM_ID_CMDLST_BUF_MC,   work_module);
		cpe_mem_free(MEM_ID_CMDLST_ENTRY_MC, work_module);
		cpe_mem_free(MEM_ID_CMDLST_PARA_MC,  work_module);
		break;
	case ARF_MATCHER:
	case ENH_ARF_MATCHER:
		cpe_mem_free(MEM_ID_CMDLST_BUF_ENH_ARF,   work_module);
		cpe_mem_free(MEM_ID_CMDLST_ENTRY_ENH_ARF, work_module);
		cpe_mem_free(MEM_ID_CMDLST_PARA_ENH_ARF,  work_module);
		cpe_mem_free(MEM_ID_CMDLST_BUF_REORDER,   work_module);
		cpe_mem_free(MEM_ID_CMDLST_ENTRY_REORDER, work_module);
		cpe_mem_free(MEM_ID_CMDLST_PARA_REORDER,  work_module);
		cpe_mem_free(MEM_ID_CMDLST_BUF_COMPARE,   work_module);
		cpe_mem_free(MEM_ID_CMDLST_ENTRY_COMPARE, work_module);
		cpe_mem_free(MEM_ID_CMDLST_PARA_COMPARE,  work_module);
		break;
	case MATCHER_MC:
		cpe_mem_free(MEM_ID_CMDLST_BUF_MC,        work_module);
		cpe_mem_free(MEM_ID_CMDLST_ENTRY_MC,      work_module);
		cpe_mem_free(MEM_ID_CMDLST_PARA_MC,       work_module);
		cpe_mem_free(MEM_ID_CMDLST_BUF_REORDER,   work_module);
		cpe_mem_free(MEM_ID_CMDLST_ENTRY_REORDER, work_module);
		cpe_mem_free(MEM_ID_CMDLST_PARA_REORDER,  work_module);
		cpe_mem_free(MEM_ID_CMDLST_BUF_COMPARE,   work_module);
		cpe_mem_free(MEM_ID_CMDLST_ENTRY_COMPARE, work_module);
		cpe_mem_free(MEM_ID_CMDLST_PARA_COMPARE,  work_module);
		break;
	case ARF_MATCHER_MC:
	case ENH_ARF_MATCHER_MC:
		cpe_mem_free(MEM_ID_CMDLST_BUF_ENH_ARF,   work_module);
		cpe_mem_free(MEM_ID_CMDLST_ENTRY_ENH_ARF, work_module);
		cpe_mem_free(MEM_ID_CMDLST_PARA_ENH_ARF,  work_module);
		cpe_mem_free(MEM_ID_CMDLST_BUF_REORDER,   work_module);
		cpe_mem_free(MEM_ID_CMDLST_ENTRY_REORDER, work_module);
		cpe_mem_free(MEM_ID_CMDLST_PARA_REORDER,  work_module);
		cpe_mem_free(MEM_ID_CMDLST_BUF_COMPARE,   work_module);
		cpe_mem_free(MEM_ID_CMDLST_ENTRY_COMPARE, work_module);
		cpe_mem_free(MEM_ID_CMDLST_PARA_COMPARE,  work_module);
		cpe_mem_free(MEM_ID_CMDLST_BUF_MC,        work_module);
		cpe_mem_free(MEM_ID_CMDLST_ENTRY_MC,      work_module);
		cpe_mem_free(MEM_ID_CMDLST_PARA_MC,       work_module);
		break;
	case KLT_ONLY:
		cpe_mem_free(MEM_ID_CMDLST_BUF_KLT,   work_module);
		cpe_mem_free(MEM_ID_CMDLST_ENTRY_KLT, work_module);
		cpe_mem_free(MEM_ID_CMDLST_PARA_KLT,  work_module);
		break;
	case ENH_ARF_KLT:
		cpe_mem_free(MEM_ID_CMDLST_BUF_ENH_ARF,   work_module);
		cpe_mem_free(MEM_ID_CMDLST_ENTRY_ENH_ARF, work_module);
		cpe_mem_free(MEM_ID_CMDLST_PARA_ENH_ARF,  work_module);
		cpe_mem_free(MEM_ID_CMDLST_BUF_KLT,       work_module);
		cpe_mem_free(MEM_ID_CMDLST_ENTRY_KLT,     work_module);
		cpe_mem_free(MEM_ID_CMDLST_PARA_KLT,      work_module);
		break;
	default:
		loge("Failed : Invilid work_module = %d", work_module);
		return ISP_IPP_ERR;
	}

	return ISP_IPP_OK;
}

/********************************************************************************************
 * function name: cmdlst_set_buffer_header
 * description: set current cmdlst buf header.
 *              cpu_enable: 1 to cmdlst, 3 to cmdlst and cpu;
 ********************************************************************************************/
static int cmdlst_set_ctrl_pm(cmdlst_para_t *cmdlst_para, cmd_buf_t *cmd_buf, unsigned int stripe_index)
{
	unsigned int channel_id = cmdlst_para->channel_id;
	unsigned int cpu_enable = (cmdlst_para->cmd_stripe_info[stripe_index].is_last_stripe == 1) ? 3 : 1;
	unsigned int reg_offset = IPP_BASE_ADDR_TOP + IPP_TOP_CMDLST_CTRL_PM_0_REG + CMD_IRQ_CFG_OFFSET * channel_id;
	cmdlst_set_reg(cmd_buf, reg_offset, cpu_enable);
	cmdlst_set_reg(cmd_buf, reg_offset, 0);
	logd("stripe_index = %d, channel_id = %d, cpu_enable = %d", stripe_index, channel_id, cpu_enable);
	return ISP_IPP_OK;
}

static int cmdlst_set_modules_irq_reg(cmd_buf_t *cmd_buf, unsigned int ipp_irq_module,
											unsigned int channel_id, unsigned int irq_mode)
{
	/* xxx_irq_outen must be set 0, that means disable to generate eop or raw irq to software; */
	if (ipp_irq_module & (1 << IPP_CMD_RES_SHARE_ORB_ENH)) {
		cmdlst_set_reg(cmd_buf, IPP_BASE_ADDR_TOP + IPP_TOP_ENH_IRQ_REG0_REG, 0x3FF); /* clear all ORB_ENH irq */
		cmdlst_set_reg(cmd_buf, IPP_BASE_ADDR_TOP + IPP_TOP_ENH_IRQ_REG1_REG, 0x0); // cfg outen
		cmdlst_set_reg(cmd_buf, IPP_BASE_ADDR_TOP + IPP_TOP_ENH_IRQ_REG2_REG, (0x3FF & ~irq_mode)); // mask irq
		cmdlst_set_reg(cmd_buf, IPP_BASE_ADDR_TOP + IPP_TOP_CMDLST_CTRL_MAP_0_REG + 0x10 * channel_id,
						IPP_CMDLST_SELECT_ORB_ENH);
	}

	if (ipp_irq_module & (1 << IPP_CMD_RES_SHARE_ARFEATURE)) {
		cmdlst_set_reg(cmd_buf, IPP_BASE_ADDR_TOP + IPP_TOP_ARF_IRQ_REG0_REG, 0x3FFFFFFF); /* clear all ARF irq */
		cmdlst_set_reg(cmd_buf, IPP_BASE_ADDR_TOP + IPP_TOP_ARF_IRQ_REG1_REG, 0x0);
		cmdlst_set_reg(cmd_buf, IPP_BASE_ADDR_TOP + IPP_TOP_ARF_IRQ_REG2_REG, (0x3FFFFFFF & ~irq_mode));
		cmdlst_set_reg(cmd_buf, IPP_BASE_ADDR_TOP + IPP_TOP_CMDLST_CTRL_MAP_0_REG + 0x10 * channel_id,
						IPP_CMDLST_SELECT_ARF);
	}

	if (ipp_irq_module & (1 << IPP_CMD_RES_SHARE_RDR)) {
		cmdlst_set_reg(cmd_buf, IPP_BASE_ADDR_TOP + IPP_TOP_RDR_IRQ_REG0_REG, 0x1F); /* clear all RDR irq */
		cmdlst_set_reg(cmd_buf, IPP_BASE_ADDR_TOP + IPP_TOP_RDR_IRQ_REG1_REG, 0x0);
		cmdlst_set_reg(cmd_buf, IPP_BASE_ADDR_TOP + IPP_TOP_RDR_IRQ_REG2_REG, (0x1F & ~irq_mode));
		cmdlst_set_reg(cmd_buf, IPP_BASE_ADDR_TOP + IPP_TOP_CMDLST_CTRL_MAP_0_REG + 0x10 * channel_id,
						IPP_CMDLST_SELECT_RDR);
	}

	if (ipp_irq_module & (1 << IPP_CMD_RES_SHARE_CMP)) {
		cmdlst_set_reg(cmd_buf, IPP_BASE_ADDR_TOP + IPP_TOP_CMP_IRQ_REG0_REG, 0x1F); /* clear all CMP irq */
		cmdlst_set_reg(cmd_buf, IPP_BASE_ADDR_TOP + IPP_TOP_CMP_IRQ_REG1_REG, 0x0);
		cmdlst_set_reg(cmd_buf, IPP_BASE_ADDR_TOP + IPP_TOP_CMP_IRQ_REG2_REG, (0x1F & ~irq_mode));
		cmdlst_set_reg(cmd_buf, IPP_BASE_ADDR_TOP + IPP_TOP_CMDLST_CTRL_MAP_0_REG + 0x10 * channel_id,
						IPP_CMDLST_SELECT_CMP);
	}

	if (ipp_irq_module & (1 << IPP_CMD_RES_SHARE_MC)) {
		cmdlst_set_reg(cmd_buf, IPP_BASE_ADDR_TOP + IPP_TOP_MC_IRQ_REG0_REG, 0xF); /* clear all MC irq */
		cmdlst_set_reg(cmd_buf, IPP_BASE_ADDR_TOP + IPP_TOP_MC_IRQ_REG1_REG, 0x0);
		cmdlst_set_reg(cmd_buf, IPP_BASE_ADDR_TOP + IPP_TOP_MC_IRQ_REG2_REG, (0xF & ~irq_mode));
		cmdlst_set_reg(cmd_buf, IPP_BASE_ADDR_TOP + IPP_TOP_CMDLST_CTRL_MAP_0_REG + 0x10 * channel_id,
						IPP_CMDLST_SELECT_MC);
	}

	if (ipp_irq_module & (1 << IPP_CMD_RES_SHARE_KLT)) {
		cmdlst_set_reg(cmd_buf, IPP_BASE_ADDR_TOP + IPP_TOP_KLT_IRQ_REG0_REG, 0x7FFFFFFF); /* clear all KLT irq */
		cmdlst_set_reg(cmd_buf, IPP_BASE_ADDR_TOP + IPP_TOP_KLT_IRQ_REG1_REG, 0x0);
		cmdlst_set_reg(cmd_buf, IPP_BASE_ADDR_TOP + IPP_TOP_KLT_IRQ_REG2_REG, (0x7FFFFFFF & ~irq_mode));
		cmdlst_set_reg(cmd_buf, IPP_BASE_ADDR_TOP + IPP_TOP_CMDLST_CTRL_MAP_0_REG + 0x10 * channel_id,
						IPP_CMDLST_SELECT_KLT);
	}

	return ISP_IPP_OK;
}

static int cmdlst_set_irq_mode(cmdlst_para_t *cmdlst_para, cmd_buf_t *cmd_buf, unsigned int stripe_index)
{
	unsigned int irq_mode = cmdlst_para->cmd_stripe_info[stripe_index].irq_mode;
	unsigned int channel_id = cmdlst_para->channel_id;
	unsigned int ipp_irq_module = cmdlst_para->cmd_stripe_info[stripe_index].irq_module;

	cmdlst_set_ctrl_pm(cmdlst_para, cmd_buf, stripe_index);
	cmdlst_set_modules_irq_reg(cmd_buf, ipp_irq_module, channel_id, irq_mode);
	logd("stripe_index = %d, channel_id = %d, irq_mode = 0x%x", stripe_index, channel_id, irq_mode);
	return ISP_IPP_OK;
}

static int cmdlst_set_header_cmdlst_reg(cmdlst_para_t *cmdlst_para, cmd_buf_t *cmd_buf, unsigned int stripe_index)
{
	unsigned int reg_offset_addr = 0;
	cmdlst_token_cfg_t token_cfg = {0};
	cmdlst_cmd_cfg_t cmd_cfg = {0};
	cmdlst_stripe_info_t *tile_info = &cmdlst_para->cmd_stripe_info[stripe_index];
	token_cfg.bits.token_mode     = 0; // 0: EOP mode; 1: EOS mode;
	token_cfg.bits.link_channel   = tile_info->ch_link;
	token_cfg.bits.link_token_nbr = tile_info->ch_link_act_nbr;
	cmd_cfg.bits.wait_eop         = 1; // wait HW EOP

	logd("stripe_index = %d, ch_link = %d, ch_link_act_nbr = %d",
		stripe_index, tile_info->ch_link, tile_info->ch_link_act_nbr);
	reg_offset_addr = CMDLST_CMD_CFG_0_REG + CH_CFG_OFFSET * cmdlst_para->channel_id;
	cmdlst_set_reg(cmd_buf, g_cmdlst_devs[0].base_addr + reg_offset_addr, cmd_cfg.u32);

	reg_offset_addr = CMDLST_TOKEN_CFG_0_REG + CH_CFG_OFFSET * cmdlst_para->channel_id;
	cmdlst_set_reg(cmd_buf, g_cmdlst_devs[0].base_addr + reg_offset_addr, token_cfg.u32);
	return ISP_IPP_OK;
}

static int cmdlst_set_buffer_header(unsigned int stripe_index, cmd_buf_t *cmd_buf, cmdlst_para_t *cmdlst_para)
{
	unsigned int idx;
	int ret;
	logd("+");

	/* CMD_CFG_ID_MAX*4 bytes are reserved for cmdlst_head update operations
	(Updata by the following interface: cmdlst_enqueue). */
	for (idx = 0; idx < CMD_CFG_ID_MAX; idx++) {
		*(unsigned int *)(uintptr_t)(cmd_buf->data_addr) = CMDLST_PADDING_DATA;
		cmd_buf->data_addr += 4;
		cmd_buf->data_size += 4;
	}

	ret = cmdlst_set_header_cmdlst_reg(cmdlst_para, cmd_buf, stripe_index);
	if (ret != 0) {
		loge("Failed : cmdlst_set_header_cmdlst_reg");
		return ISP_IPP_ERR;
	}

	ret = cmdlst_set_irq_mode(cmdlst_para, cmd_buf, stripe_index);
	if (ret != 0) {
		loge("Failed : cmdlst_set_irq_mode");
		return ISP_IPP_ERR;
	}

	while (cmd_buf->data_size < CMDLST_HEADER_SIZE) {
		*(unsigned int *)(uintptr_t)(cmd_buf->data_addr) = CMDLST_PADDING_DATA;
		cmd_buf->data_addr += 4;
		cmd_buf->data_size += 4;
	}

	logd("-");
	return ISP_IPP_OK;
}

static int get_new_entry_and_cmdlst_buf_addr(
	schedule_cmdlst_link_t **new_entry, unsigned int work_module,
	unsigned int cmd_entry_mem_id, unsigned int cmd_buf_mem_id, unsigned int stripe_cnt)
{
	unsigned long long va = 0;
	unsigned int da = 0;
	int ret;
	ret = cpe_mem_get(cmd_entry_mem_id, work_module, &va, &da);
	if (ret != 0) {
		loge("Failed to memory_alloc new entry!");
		return ISP_IPP_ERR;
	}
	*new_entry = (schedule_cmdlst_link_t *)(uintptr_t)(va);
	logd("new_entry: va = 0x%llx", va);
	ret = cpe_mem_get(cmd_buf_mem_id, work_module, &va, &da);
	if (ret != 0) {
		loge("Failed to memory_alloc cmdlst buf!");
		return ISP_IPP_ERR;
	}

	ret = memset_s(*new_entry, stripe_cnt * sizeof(schedule_cmdlst_link_t),
					0, stripe_cnt * sizeof(schedule_cmdlst_link_t));
	if (ret != 0) {
		loge("Failed : memset_s");
		return ISP_IPP_ERR;
	}

	(*new_entry)[0].cmd_buf.start_addr = va;
	(*new_entry)[0].cmd_buf.start_addr_isp_map = da;
	logd("cmd_buf:va = 0x%llx, da=0x%x, cmd_buf_mem_id=%d, cmd_entry_mem_id=%d",
		va, da, cmd_buf_mem_id, cmd_entry_mem_id);

	return ISP_IPP_OK;
}


/********************************************************************************************
 * @function name: df_sched_prepare
 * @description: Get cmd_entry and cmdlst_buf memory; Init cmd_entry.
 * @input: NULL
 * @return: 0
 *********************************************************************************************/
int df_sched_prepare(cmdlst_para_t *cmdlst_para)
{
	int ret;
	unsigned int i;
	unsigned int channel_id = cmdlst_para->channel_id;
	schedule_cmdlst_link_t *new_entry = NULL;

	mutex_lock(&g_cmdlst_priv.cmd_priv_lock);
	if (list_empty((&g_cmdlst_priv.cmdlst_chan[channel_id].ready_list)) == 0) {
		loge("Failed : ready list not clean out");
		goto error_out;
	}

	if (cmdlst_para->stripe_cnt > CMDLST_STRIPE_MAX_NUM) {
		loge("Failed : invalid stripe_cnt = %d", cmdlst_para->stripe_cnt);
		goto error_out;
	}

	ret = get_new_entry_and_cmdlst_buf_addr(&new_entry, cmdlst_para->work_module,
			cmdlst_para->cmd_entry_mem_id, cmdlst_para->cmd_buf_mem_id, cmdlst_para->stripe_cnt);
	if (ret) {
		loge("Failed : get_new_entry_and_cmdlst_buf_addr");
		goto error_out;
	}

	cmdlst_para->cmd_entry = (void *)new_entry;
	for (i = 0; i < cmdlst_para->stripe_cnt; i++) {
		new_entry[i].stripe_cnt = cmdlst_para->stripe_cnt;
		new_entry[i].stripe_index = i;
		new_entry[i].data = (void *)cmdlst_para;
		list_add_tail(&new_entry[i].list_link, &g_cmdlst_priv.cmdlst_chan[channel_id].ready_list);
		new_entry[i].cmd_buf.start_addr = new_entry[0].cmd_buf.start_addr +
										  (unsigned long long)(CMDLST_BUFFER_SIZE * (unsigned long long)i);
		new_entry[i].cmd_buf.start_addr_isp_map = new_entry[0].cmd_buf.start_addr_isp_map + CMDLST_BUFFER_SIZE * i;
		logd("new_entry[%d]: start_addr = 0x%llx, start_addr_isp_map=0x%x",
			i, new_entry[i].cmd_buf.start_addr, new_entry[i].cmd_buf.start_addr_isp_map);

		if (new_entry[i].cmd_buf.start_addr == 0 || new_entry[i].cmd_buf.start_addr_isp_map == 0) {
			loge("Failed : fail to get cmdlist buffer!");
			goto error_out;
		}

		new_entry[i].cmd_buf.buffer_size = CMDLST_BUFFER_SIZE;
		new_entry[i].cmd_buf.header_size = CMDLST_HEADER_SIZE;
		new_entry[i].cmd_buf.data_addr   = new_entry[i].cmd_buf.start_addr; // init data_addr
		new_entry[i].cmd_buf.data_size   = 0; // init data_size
		new_entry[i].cmd_buf.next_buffer = NULL;

		ret = cmdlst_set_buffer_header(i, &new_entry[i].cmd_buf, cmdlst_para);
		if (ret) {
			loge("Failed : cmdlst_set_buffer_header error");
			goto error_out;
		}
	}

	mutex_unlock(&g_cmdlst_priv.cmd_priv_lock);
	return ISP_IPP_OK;
error_out:
	mutex_unlock(&g_cmdlst_priv.cmd_priv_lock);
	return ISP_IPP_ERR;
}


#ifdef IPP_UT_DEBUG
static void cmdlst_dump_queue(cmdlst_para_t *cmdlst_para)
{
	unsigned int channel_id = cmdlst_para->channel_id;
	schedule_cmdlst_link_t *cmdlst_temp_link = NULL;
	schedule_cmdlst_link_t *n = NULL;
	struct list_head *cmdlst_insert_queue = &g_cmdlst_priv.cmdlst_chan[channel_id].ready_list;
	list_for_each_entry_safe(cmdlst_temp_link, n, cmdlst_insert_queue, list_link) {
		logi("dump cmdlst queue:");
		logi("stripe_cnt = %d, stripe_index = %d, channel_id = %d",
			 cmdlst_temp_link->stripe_cnt, cmdlst_temp_link->stripe_index, channel_id);
		logi("cmd_buf: start_addr = 0x%llx, start_addr_isp_map = 0x%08x, data_size = 0x%x",
			 cmdlst_temp_link->cmd_buf.start_addr,
			 cmdlst_temp_link->cmd_buf.start_addr_isp_map,
			 cmdlst_temp_link->cmd_buf.data_size);
		logi("en_link = %d, ch_link = %d, ch_link_act_nbr = %d, irq_mode = 0x%x",
			 cmdlst_para->cmd_stripe_info[cmdlst_temp_link->stripe_index].en_link,
			 cmdlst_para->cmd_stripe_info[cmdlst_temp_link->stripe_index].ch_link,
			 cmdlst_para->cmd_stripe_info[cmdlst_temp_link->stripe_index].ch_link_act_nbr,
			 cmdlst_para->cmd_stripe_info[cmdlst_temp_link->stripe_index].irq_mode);
	}

	return;
}
#endif

static int cmdlst_set_buffer_padding(cmd_buf_t *cmd_buf)
{
	unsigned int i = 0;
	unsigned int aligned_data_size = 0;

	/* To better show the ending, we do Padding two words(D64) */
	*(unsigned int *)(uintptr_t)(cmd_buf->data_addr) = CMDLST_PADDING_DATA;
	cmd_buf->data_addr += BYTES_PER_WORD;
	cmd_buf->data_size += BYTES_PER_WORD;

	/* 128-byte alignment */
	aligned_data_size = align_up(cmd_buf->data_size, ALIGNMENT_128_BYTE);
	for (i = cmd_buf->data_size; i < aligned_data_size; i += BYTES_PER_WORD) {
		if ((cmd_buf->data_size + BYTES_PER_WORD) <= cmd_buf->buffer_size) {
			*(unsigned int *)(uintptr_t)(cmd_buf->data_addr) = CMDLST_PADDING_DATA;
			cmd_buf->data_addr += BYTES_PER_WORD;
			cmd_buf->data_size += BYTES_PER_WORD;
		} else {
			loge("cmdlst buffer is full: data_size:0x%x, buffer_size:0x%x, start_addr:0x%llx",
					cmd_buf->data_size, cmd_buf->buffer_size, cmd_buf->start_addr);
			return ISP_IPP_ERR;
		}
	}

	logd("cmd_buf: data_size = 0x%x, buffer_size = 0x%x, data_addr = 0x%llx",
			cmd_buf->data_size, cmd_buf->buffer_size, cmd_buf->data_addr);
	return ISP_IPP_OK;
}

static int update_cmdlst_cfg_tab(cmdlst_para_t *cmdlst_para)
{
	unsigned int i = 0;
	schedule_cmdlst_link_t *cmd_link_entry = (schedule_cmdlst_link_t *)cmdlst_para->cmd_entry;

	for (i = 0; i < cmdlst_para->stripe_cnt; i++) {
		cfg_tab_cmdlst_t *p_cfgtab = &cmd_link_entry[i].cmdlst_cfg_tab;

		// step1: Padding_Data is filled to ensure that the finale 'data_size' of cmdlst_buf is 128-byte-aligned.
		cmdlst_set_buffer_padding(&cmd_link_entry[i].cmd_buf);

		p_cfgtab->channel_id = cmdlst_para->channel_id;
		p_cfgtab->sw_cvdr_rd_addr.bits.sw_cvdr_rd_address       = (JPG_CVDR_OFFSET + CVDR_IPP_CVDR_IPP_VP_RD_CFG_0_REG) >> 2;
		p_cfgtab->sw_cvdr_rd_addr.bits.sw_cvdr_rd_size          = CMD_CFG_DATA_0_1_2_3;
		p_cfgtab->sw_cvdr_rd_data0.bits.sw_vprd_pixel_format	= DF_D64;
		p_cfgtab->sw_cvdr_rd_data0.bits.sw_vprd_pixel_expansion = EXP_PIX;
		p_cfgtab->sw_cvdr_rd_data0.bits.sw_vprd_allocated_du	= CMD_RD_DU_NUM;
		p_cfgtab->sw_cvdr_rd_data0.bits.sw_vprd_last_page       = (cmd_link_entry[i].cmd_buf.start_addr_isp_map +
			cmd_link_entry[i].cmd_buf.data_size - 1) >> MEM_PAGE_SHIFT;
		p_cfgtab->sw_cvdr_rd_data1.bits.sw_vprd_line_size       = CMD_RD_LINE_SIZE - 1;
		p_cfgtab->sw_cvdr_rd_data1.bits.sw_vprd_ihleft          = 0;
		p_cfgtab->sw_cvdr_rd_data2.bits.sw_vprd_frame_size      =
			macro_cmd_rd_frame_size(cmd_link_entry[i].cmd_buf.data_size, CMD_RD_LINE_SIZE, CMD_RD_BITS) - 1;
		p_cfgtab->sw_cvdr_rd_data3.bits.sw_vprd_axi_frame_start = cmd_link_entry[i].cmd_buf.start_addr_isp_map >> 4;
		p_cfgtab->sw_resource_cfg.bits.sw_priority              = cmdlst_para->cmd_stripe_info[i].hw_priority;
		p_cfgtab->sw_resource_cfg.bits.sw_resource              = cmdlst_para->cmd_stripe_info[i].resource_share;
		p_cfgtab->sw_task_cfg.bits.sw_task                      = 0; // This configuration is optional for IPP.
	}

	return ISP_IPP_OK;
}

static int cmdlst_set_branch(unsigned int channel_id)
{
	unsigned int ch_offset = CH_CFG_OFFSET * channel_id;
	hispcpe_reg_set(CMDLIST_REG, CMDLST_SW_BRANCH_0_REG + ch_offset, 0x00000001);
	udelay(1);
	return ISP_IPP_OK;
}

 /********************************************************************************************
 * function name: cmdlst_update_buffer_header
 * description: fill current cmdlst buf
 * input: cmd_buf: current cmdlst buf to config.
 * return: 0
 *********************************************************************************************/
static void cmdlst_update_buffer_header(
	schedule_cmdlst_link_t *cur_point,
	schedule_cmdlst_link_t *next_point,
	unsigned int channel_id)
{
	unsigned int channel_offset = JPG_CMDLST_OFFSET + CH_CFG_OFFSET * channel_id;
	unsigned int i = 0;
	cmd_buf_t *cmd_buf = NULL;
	cmd_buf_t *next_buf = NULL;

	cur_point->cmd_buf.next_buffer = (void *)&next_point->cmd_buf;
	cmd_buf  = &cur_point->cmd_buf;
	next_buf = cmd_buf->next_buffer;

	if (next_buf == NULL) {
		for (i = 0; i < CMD_CFG_ID_MAX; i++)
			*(unsigned int *)(uintptr_t)(cmd_buf->start_addr + BYTES_PER_WORD * i) = CMDLST_PADDING_DATA;
		return;
	}

	*(unsigned int *)(uintptr_t)(cmd_buf->start_addr + CMD_ADDR_OFFSET(CMD_RESOURCE_ADDR))  =
		channel_offset + CMDLST_HW_RESOURCES_0_REG;
	*(unsigned int *)(uintptr_t)(cmd_buf->start_addr + CMD_ADDR_OFFSET(CMD_RESOURCE_VALUE)) =
		next_point->cmdlst_cfg_tab.sw_resource_cfg.u32;

	// The CMDLST_HW_TASK_0_REG configuration is optional for IPP.
	*(unsigned int *)(uintptr_t)(cmd_buf->start_addr + CMD_ADDR_OFFSET(CMD_TASK_ADDR))    =
		channel_offset + CMDLST_HW_TASK_0_REG;
	*(unsigned int *)(uintptr_t)(cmd_buf->start_addr + CMD_ADDR_OFFSET(CMD_TASK_VALUE))   =
		next_point->cmdlst_cfg_tab.sw_task_cfg.u32;

	*(unsigned int *)(uintptr_t)(cmd_buf->start_addr + CMD_ADDR_OFFSET(CMD_DATA_3_ADDR))  =
		channel_offset + CMDLST_HW_CVDR_RD_DATA_3_0_REG;
	*(unsigned int *)(uintptr_t)(cmd_buf->start_addr + CMD_ADDR_OFFSET(CMD_DATA_3_VALUE)) =
		next_point->cmdlst_cfg_tab.sw_cvdr_rd_data3.u32;
	*(unsigned int *)(uintptr_t)(cmd_buf->start_addr + CMD_ADDR_OFFSET(CMD_DATA_0_ADDR))  =
		channel_offset + CMDLST_HW_CVDR_RD_DATA_0_0_REG;
	*(unsigned int *)(uintptr_t)(cmd_buf->start_addr + CMD_ADDR_OFFSET(CMD_DATA_0_VALUE)) =
		next_point->cmdlst_cfg_tab.sw_cvdr_rd_data0.u32;
	*(unsigned int *)(uintptr_t)(cmd_buf->start_addr + CMD_ADDR_OFFSET(CMD_DATA_1_ADDR))  =
		channel_offset + CMDLST_HW_CVDR_RD_DATA_1_0_REG;
	*(unsigned int *)(uintptr_t)(cmd_buf->start_addr + CMD_ADDR_OFFSET(CMD_DATA_1_VALUE)) =
		next_point->cmdlst_cfg_tab.sw_cvdr_rd_data1.u32;
	*(unsigned int *)(uintptr_t)(cmd_buf->start_addr + CMD_ADDR_OFFSET(CMD_DATA_2_ADDR))  =
		channel_offset + CMDLST_HW_CVDR_RD_DATA_2_0_REG;
	*(unsigned int *)(uintptr_t)(cmd_buf->start_addr + CMD_ADDR_OFFSET(CMD_DATA_2_VALUE)) =
		next_point->cmdlst_cfg_tab.sw_cvdr_rd_data2.u32;

	return;
}

static void cmdlst_update_buffer_header_last(schedule_cmdlst_link_t *cur_point)
{
	cmd_buf_t *cmd_buf = &cur_point->cmd_buf;
	unsigned int i = 0;

	for (i = 0; i < CMD_CFG_ID_MAX; i++)
		*(unsigned int *)(uintptr_t)(cmd_buf->start_addr + BYTES_PER_WORD * i) = CMDLST_PADDING_DATA;

	return;
}

static int cmdlst_enqueue(cmdlst_para_t *cmdlst_para)
{
	unsigned int channel_id = cmdlst_para->channel_id;
	schedule_cmdlst_link_t *pos = NULL;
	schedule_cmdlst_link_t *n = NULL;
#ifdef IPP_UT_DEBUG
	struct list_head *dump_list = NULL;
#endif
	struct list_head *ready_list = NULL;

	ready_list = &g_cmdlst_priv.cmdlst_chan[channel_id].ready_list;
#ifdef IPP_UT_DEBUG
	for (dump_list = ready_list->next; dump_list != ready_list; dump_list = dump_list->next) {
		if ((void *)dump_list == NULL)
			break;
	}
#endif

	list_for_each_entry_safe(pos, n, ready_list, list_link) {
		if (pos->list_link.next != ready_list)
			cmdlst_update_buffer_header(pos, n, channel_id);
		else
			cmdlst_update_buffer_header_last(pos);
	}

	return ISP_IPP_OK;
}

/********************************************************************************************
* function name: cmdlst_start
* description: start cmdlst when branch,or the first frame.
* input:
*     last_exec:last exec stripe's start_addr
*     cmdlst_cfg: cmdlst config table
* return: 0
*********************************************************************************************/
static int cmdlst_start(cfg_tab_cmdlst_t *cmdlst_cfg, unsigned int channel_id)
{
	if (cmdlst_do_config(&g_cmdlst_devs[0], cmdlst_cfg)) {
		loge("Failed: cmdlst_do_config");
		return ISP_IPP_ERR;
	}

	return ISP_IPP_OK;
}

/********************************************************************************************
* @function name: df_sched_start
* @description: do cmdlst_enqueque, and start cmdlst;
*    According to the current design, the same channel does not process two frames at
*    the same time. Therefore, the hardware queue does not need to be maintained.
*    Each start_cmdslt only needs to start the current stripe. That is, the cmdlst_buf
*    address of the current frame needs to be configured in the sw read port register
*    of the cmdslt.
* @input: cmdlst_para
* @return: 0
*********************************************************************************************/
int df_sched_start(cmdlst_para_t *cmdlst_para)
{
	unsigned int channel_id = cmdlst_para->channel_id;
	schedule_cmdlst_link_t *cmd_link_entry = (schedule_cmdlst_link_t *)cmdlst_para->cmd_entry;

	mutex_lock(&g_cmdlst_priv.cmd_priv_lock);
	if (update_cmdlst_cfg_tab(cmdlst_para)) {
		loge("Failed: update_cmdlst_cfg_tab");
		goto out_error;
	}

	if (cmdlst_set_branch(channel_id)) {
		loge("Failed: cmdlst_set_branch");
		goto out_error;
	}

	if (cmdlst_enqueue(cmdlst_para)) {
		loge("Failed: cmdlst_enqueue");
		goto out_error;
	}

#ifdef IPP_UT_DEBUG
	cmdlst_dump_queue(cmdlst_para);
#endif

#ifdef IPP_UT_DIO_DEBUG
	ut_dump_register(cmdlst_para->stripe_cnt);
#endif
	if (cmdlst_start(&(cmd_link_entry[0].cmdlst_cfg_tab), channel_id)) {
		loge("Failed: cmdlst_start");
		goto out_error;
	}

#ifdef IPP_UT_DEBUG
	list_for_each_entry(cmd_link_entry, &g_cmdlst_priv.cmdlst_chan[cmdlst_para->channel_id].ready_list, list_link) {
		logi("cmd_link_entry->stripe_index = %d", cmd_link_entry->stripe_index);
		cmdlst_buff_dump(&cmd_link_entry->cmd_buf);
	}
#endif

	frame_num++;
	mutex_unlock(&g_cmdlst_priv.cmd_priv_lock);
	return ISP_IPP_OK;
out_error:
	mutex_unlock(&g_cmdlst_priv.cmd_priv_lock);
	return ISP_IPP_ERR;
}

// End file
