/*
 * hal_cmdlist.c
 *
 * cmdlist config operation
 *
 * Copyright (c) 2019-2020 Huawei Technologies CO., Ltd.
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 */

#include "hal_cmdlist.h"
#include <linux/io.h>
#include "hal_common.h"
#include "drv_common.h"
#include "smmu.h"
#include "hal_venc.h"


static struct cmdlist_cfg g_cmdlist_cfg;

static void hal_cmdlist_init(uint32_t *base_addr, uint64_t iova_addr)
{
	size_t align_size;
	cmdlist_read_back_info_t *info = (cmdlist_read_back_info_t *)base_addr;

	align_size = align_up(sizeof(cmdlist_read_back_info_t), CMDLIST_ALIGN_SIZE) / 4;
	g_cmdlist_cfg.cfg_offset = (uint32_t)align_size;
	g_cmdlist_cfg.base_addr = base_addr;
	g_cmdlist_cfg.iova_addr = iova_addr;
	g_cmdlist_cfg.cfg_len0 = 0;
	g_cmdlist_cfg.cfg_len1 = 0;
	info->FUNC_VCPI_RAWINT.u32 = 0;
}

static int32_t check_offset_valid(void)
{
	uint32_t offset = g_cmdlist_cfg.cfg_offset + g_cmdlist_cfg.cfg_len0 + g_cmdlist_cfg.cfg_len1;

	/* 256 bytes are reserved in the cmdlist for filling data. */
	if (offset * 4 + CMDLIST_RESERVE_MEM_SIZE > CMDLIST_BUFFER_SIZE) {
		VCODEC_FATAL_VENC(
			"cfg area offset is 0x%x, len0 is 0x%x, len1 is 0x%x",
			g_cmdlist_cfg.cfg_offset,
			g_cmdlist_cfg.cfg_len0,
			g_cmdlist_cfg.cfg_len1);

		return VCODEC_FAILURE;
	}

	return 0;
}

void hal_cmdlist_wr_cmd_in_len0(uint32_t addr, uint32_t value)
{
	uint32_t *base = g_cmdlist_cfg.base_addr + g_cmdlist_cfg.cfg_offset;

	if (unlikely(check_offset_valid() != 0))
		return;

	base[g_cmdlist_cfg.cfg_len0++] = (CMDLIST_WRCMD_MASK & addr);
	base[g_cmdlist_cfg.cfg_len0++] = value;
}

static void hal_cmdlist_rd_cmd_in_len1(uint32_t addr, uint32_t bust_size)
{
	/* The start address of the len1 must be 128 byte aligned. */
	uint32_t cfg_len1_offset = align_up(g_cmdlist_cfg.cfg_len0, 32); // 32: Byte alignment
	uint32_t *base = g_cmdlist_cfg.base_addr + g_cmdlist_cfg.cfg_offset +
		cfg_len1_offset;

	if (unlikely(check_offset_valid() != 0))
		return;

	base[g_cmdlist_cfg.cfg_len1++] = addr + 1 + (bust_size << 24);
}

static void hal_cmdlist_wr_cmd_in_len1(uint32_t addr, uint32_t value)
{
	/* The start address of the len1 must be 128 byte aligned. */
	uint32_t cfg_len1_offset = align_up(g_cmdlist_cfg.cfg_len0, 32); // 32: Byte alignment
	uint32_t *base = g_cmdlist_cfg.base_addr + g_cmdlist_cfg.cfg_offset + cfg_len1_offset;

	if (unlikely(check_offset_valid() != 0))
		return;

	base[g_cmdlist_cfg.cfg_len1++] = (CMDLIST_WRCMD_MASK & addr);
	base[g_cmdlist_cfg.cfg_len1++] = value;
}

static void hal_cmdlist_fill_dummy_data_len0(void)
{
	uint32_t i;
	uint32_t align_len = align_up(g_cmdlist_cfg.cfg_len0, 4); // 4: Byte alignment
	uint32_t fill_len = align_len - g_cmdlist_cfg.cfg_len0;
	uint32_t *base = g_cmdlist_cfg.base_addr + g_cmdlist_cfg.cfg_offset;

	if (unlikely(check_offset_valid() != 0))
		return;

	/* The length of len0 must be 16-byte aligned. If the length of is less than 16 bytes, use 0xFF1FFFFD. */
	for (i = 0; i < fill_len; i++)
		base[g_cmdlist_cfg.cfg_len0++] = CMDLIST_DUMMY_DATA;
}

static void hal_cmdlist_fill_dummy_data_len1(void)
{
	uint32_t i;
	/* The start address of the len1 must be 128 byte aligned. */
	uint32_t cfg_len1_offset = align_up(g_cmdlist_cfg.cfg_len0, 32); // 32: Byte alignment
	uint32_t align_len = align_up(g_cmdlist_cfg.cfg_len1, 4); // 4: Byte alignment
	uint32_t fill_len = align_len - g_cmdlist_cfg.cfg_len1;
	uint32_t *base = g_cmdlist_cfg.base_addr + g_cmdlist_cfg.cfg_offset + cfg_len1_offset;

	if (unlikely(check_offset_valid() != 0))
		return;

	/* The length of len1 must be 16-byte aligned. If the length of is less than 16 bytes, use 0xFF1FFFFD. */
	for (i = 0; i < fill_len; i++)
		base[g_cmdlist_cfg.cfg_len1++] = CMDLIST_DUMMY_DATA;
}

static void hal_cmdlist_update_len0(struct cmdlist_node *cur_node)
{
	uint32_t *base = NULL;

	if (cur_node) {
		base = g_cmdlist_cfg.base_addr + g_cmdlist_cfg.cfg_offset;
		/* Just take a placeholder, not update the value. */
		cur_node->pre_len_addr = base + g_cmdlist_cfg.cfg_len0 + 1;
		hal_cmdlist_wr_cmd_in_len0(
			offsetof(S_CMDLST_REGS_TYPE, CMDLST_HW_BUF_LEN) +
			CMDLIST_OFFSET, 0);
		cur_node->pre_cmdlist_addr = base + g_cmdlist_cfg.cfg_len0 + 1;
		hal_cmdlist_wr_cmd_in_len0(
			offsetof(S_CMDLST_REGS_TYPE, CMDLST_HW_AXI_ADDR) +
			CMDLIST_OFFSET, 0);
	}
	/* config dump area iova addr */
	hal_cmdlist_wr_cmd_in_len0(
		offsetof(S_CMDLST_REGS_TYPE, CMDLST_HW_AXI_ADDR_DUMP) +
		CMDLIST_OFFSET, (uint32_t)(g_cmdlist_cfg.iova_addr >> 7));
	hal_cmdlist_wr_cmd_in_len0(
		offsetof(S_HEVC_AVC_REGS_TYPE, VEDU_VCPI_START.u32), 0);
	hal_cmdlist_wr_cmd_in_len0(
		offsetof(S_HEVC_AVC_REGS_TYPE, VEDU_VCPI_START.u32), 1);
	hal_cmdlist_fill_dummy_data_len0();
}

static void hal_cmdlist_update_len1(const struct cmdlist_node *cur_node)
{
	/* The sequence and length are the same as "cmdlist_read_back_info_t". */
	hal_cmdlist_rd_cmd_in_len1(
		offsetof(S_HEVC_AVC_REGS_TYPE, VEDU_VCPI_PICSIZE_PIX), 0);
	hal_cmdlist_rd_cmd_in_len1(
		offsetof(S_HEVC_AVC_REGS_TYPE, VEDU_VCPI_MODE), 0);
	hal_cmdlist_rd_cmd_in_len1(
		offsetof(S_HEVC_AVC_REGS_TYPE, FUNC_SEL_OPT_4X4_CNT), 0);
	hal_cmdlist_rd_cmd_in_len1(
		offsetof(S_HEVC_AVC_REGS_TYPE, FUNC_SEL_INTRA_OPT_8X8_CNT), 0);
	hal_cmdlist_rd_cmd_in_len1(
		offsetof(S_HEVC_AVC_REGS_TYPE, FUNC_SEL_INTRA_OPT_16X16_CNT), 0);
	hal_cmdlist_rd_cmd_in_len1(
		offsetof(S_HEVC_AVC_REGS_TYPE, FUNC_SEL_INTRA_OPT_32X32_CNT), 0);
	hal_cmdlist_rd_cmd_in_len1(
		offsetof(S_HEVC_AVC_REGS_TYPE, FUNC_PME_MADI_SUM), 0);
	hal_cmdlist_rd_cmd_in_len1(
		offsetof(S_HEVC_AVC_REGS_TYPE, FUNC_PME_MADP_SUM), 0);
	hal_cmdlist_rd_cmd_in_len1(
		offsetof(S_HEVC_AVC_REGS_TYPE, FUNC_SEL_TOTAL_LUMA_QP), 0);
	hal_cmdlist_rd_cmd_in_len1(
		offsetof(S_HEVC_AVC_REGS_TYPE, FUNC_SAO_MSE_SUM), 0);
	hal_cmdlist_rd_cmd_in_len1(
		offsetof(S_HEVC_AVC_REGS_TYPE, FUNC_VLC_PIC_STRMSIZE), 0);
	hal_cmdlist_rd_cmd_in_len1(
		offsetof(S_HEVC_AVC_REGS_TYPE, FUNC_CABAC_PIC_STRMSIZE), 0);
	hal_cmdlist_rd_cmd_in_len1(
		offsetof(S_HEVC_AVC_REGS_TYPE, FUNC_VLCST_DSRPTR00), MAX_SLICE_NUM - 1);
	hal_cmdlist_rd_cmd_in_len1(
		offsetof(S_HEVC_AVC_REGS_TYPE, FUNC_VLCST_DSRPTR01), MAX_SLICE_NUM - 1);
	hal_cmdlist_rd_cmd_in_len1(
		offsetof(S_HEVC_AVC_REGS_TYPE, FUNC_SEL_LUMA_QP0_CNT), QP_HISTOGRAM_NUM - 1);
	hal_cmdlist_rd_cmd_in_len1(
		offsetof(S_HEVC_AVC_REGS_TYPE, FUNC_VCPI_RAWINT),
		0); /* isr status */
	/* clear venc eop/buffer full/cmdlist eop */
	hal_cmdlist_wr_cmd_in_len1(
		offsetof(S_HEVC_AVC_REGS_TYPE, VEDU_VCPI_INTCLR), CMDLIST_CLR_INT);

	if (cur_node)
		hal_cmdlist_wr_cmd_in_len1(
			offsetof(S_HEVC_AVC_REGS_TYPE, VEDU_VCPI_SOFTINTSET),
			0x1); /* trigger soft int0 */
	else
		hal_cmdlist_wr_cmd_in_len1(
			offsetof(S_HEVC_AVC_REGS_TYPE, VEDU_VCPI_INTMASK),
			CMDLIST_LAST_FRAME_MASK); /* last frame enable cmdlist eop */
	hal_cmdlist_fill_dummy_data_len1();
}

/*
 * After the second frame is delivered, updating the cmdlist_addr and the
 * cmdlist_len of the second frame. After the third frame is delivered, the
 * cmdlist_addr and *cmdlist_len of the first frame are updated.
 */
static void hal_cmdlist_update(struct cmdlist_head *head,
	struct cmdlist_node *cur_node)
{
	struct cmdlist_node *pre_node = NULL;

	if (list_empty(&head->list))
		return;

	/* Continue to update from last location */
	(void)memcpy_s(&g_cmdlist_cfg, sizeof(struct cmdlist_cfg), &head->cfg,
		sizeof(struct cmdlist_cfg));

	hal_cmdlist_update_len0(cur_node);
	hal_cmdlist_update_len1(cur_node);

	/* update first frame cmdlist len and cmdlist addr */
	pre_node = list_last_entry(&head->list, struct cmdlist_node, list);
	/* Hardware will use every 16 bytes as a group. */
	pre_node->cfg_len =
		g_cmdlist_cfg.cfg_len0 / 4 + ((g_cmdlist_cfg.cfg_len1 / 4) << 16);

	if (list_first_entry(&head->list, struct cmdlist_node, list) != pre_node) {
		*pre_node->pre_len_addr = pre_node->cfg_len;
		*pre_node->pre_cmdlist_addr =
			(uint32_t)((pre_node->iova_addr + g_cmdlist_cfg.cfg_offset * 4) >> 7);
	}

}

void hal_cmdlist_cfg(struct cmdlist_head *head, struct cmdlist_node *cur_node,
	struct encode_info *info)
{
	hal_cmdlist_update(head, cur_node);

	hal_cmdlist_init((uint32_t *)(cur_node->virt_addr), cur_node->iova_addr);

	if (info->reg_cfg_mode == VENC_SET_CFGREGSIMPLE)
		vedu_hal_cfg_reg_simple(info, 0);
	else
		vedu_hal_cfg_reg(info, 0);

	/* Record the current location. */
	(void)memcpy_s(&head->cfg, sizeof(struct cmdlist_cfg), &g_cmdlist_cfg,
		sizeof(struct cmdlist_cfg));
}

void hal_cmdlist_encode(struct cmdlist_head *head, uint32_t *reg_base)
{
	struct cmdlist_node *first_node = NULL;
	S_CMDLST_REGS_TYPE *cmdlist_reg =
		(S_CMDLST_REGS_TYPE *)((uint64_t)(uintptr_t)reg_base + CMDLIST_OFFSET);
	S_HEVC_AVC_REGS_TYPE *venc_reg = (S_HEVC_AVC_REGS_TYPE *)reg_base;

	hal_cmdlist_update(head, NULL);

	venc_reg->VEDU_VCPI_CMDLST_CLKGATE.bits.vcpi_cmdlst_clkgate = 1;
	venc_reg->VEDU_VCPI_CMDLST_CLKGATE.bits.vcpi_vcpi2cmdlst_eop_en = 1;
	cmdlist_reg->CMDLST_CFG.bits.force_clk_on = 1;

	first_node = list_first_entry(&head->list, struct cmdlist_node, list);
	cmdlist_reg->CMDLST_SW_BUF_LEN.u32 = first_node->cfg_len;
	cmdlist_reg->CMDLST_SW_AXI_ADDR =
		(uint32_t)((first_node->iova_addr + g_cmdlist_cfg.cfg_offset * 4) >> 7);
}

bool hal_cmdlist_is_encode_done(uint32_t *base_addr)
{
	cmdlist_read_back_info_t *info = (cmdlist_read_back_info_t *)base_addr;

	if (info->FUNC_VCPI_RAWINT.bits.vcpi_rint_ve_eop)
		return true;

	return false;
}

static void hal_cmdlist_get_qp(struct stream_info *stream_info, const uint32_t *base_addr)
{
	uint32_t i;
	cmdlist_read_back_info_t *info = (cmdlist_read_back_info_t *)base_addr;

	for (i = 0; i < QP_HISTOGRAM_NUM; i++)
		stream_info->luma_qp_cnt[i] =
			info->FUNC_SEL_LUMA_QP_CNT[i].bits.luma_qp0_cnt;
}

static void hal_cmdlist_get_stream_len(struct stream_info *stream_info,
	const uint32_t *base_addr)
{
	uint32_t i;
	cmdlist_read_back_info_t *info = (cmdlist_read_back_info_t *)base_addr;
	volatile U_FUNC_VLCST_DSRPTR00 *base0 = info->FUNC_VLCST_DSRPTR00;
	volatile U_FUNC_VLCST_DSRPTR01 *base1 = info->FUNC_VLCST_DSRPTR01;

	for (i = 0; i < MAX_SLICE_NUM; i++) {
		stream_info->slice_len[i] =
			base0[i].bits.slc_len0 - base1[i].bits.invalidnum0;
		stream_info->aligned_slice_len[i] = base0[i].bits.slc_len0;
		stream_info->slice_num++;
		if (base1[i].bits.islastslc0)
			break;
	}
}

void hal_cmdlist_get_reg(struct stream_info *stream_info, const uint32_t *base_addr)
{
	cmdlist_read_back_info_t *info = (cmdlist_read_back_info_t *)base_addr;

	uint32_t width = info->VEDU_VCPI_PICSIZE_PIX.bits.vcpi_imgwidth_pix + 1;
	uint32_t height = info->VEDU_VCPI_PICSIZE_PIX.bits.vcpi_imgheight_pix + 1;
	uint32_t lcu_size;
	uint32_t lcu_block_num;
	uint32_t lcu8_block_num;

	stream_info->is_buf_full =
		info->FUNC_VCPI_RAWINT.bits.vcpi_rint_ve_buffull;
	stream_info->is_eop = info->FUNC_VCPI_RAWINT.bits.vcpi_rint_ve_eop;
	if (info->VEDU_VCPI_MODE.bits.vcpi_protocol == VEDU_H265) {
		stream_info->pic_stream_size = info->FUNC_CABAC_PIC_STRMSIZE;
		stream_info->block_cnt[BLOCK_4X4] =
			info->FUNC_SEL_OPT_4X4_CNT.bits.opt_4x4_cnt;
		stream_info->block_cnt[BLOCK_8X8] =
			info->FUNC_SEL_INTRA_OPT_8X8_CNT.bits.intra_opt_8x8_cnt;
		stream_info->block_cnt[BLOCK_16X16] =
			info->FUNC_SEL_INTRA_OPT_16X16_CNT.bits.intra_opt_16x16_cnt * 4;
		stream_info->block_cnt[BLOCK_32X32] =
			info->FUNC_SEL_INTRA_OPT_32X32_CNT.bits.intra_opt_32x32_cnt * 16;
		lcu_size = LCU_SIZE_H265;
	} else {
		stream_info->pic_stream_size = info->FUNC_VLC_PIC_STRMSIZE;
		stream_info->block_cnt[BLOCK_4X4] =
			info->FUNC_SEL_OPT_4X4_CNT.bits.opt_4x4_cnt;
		stream_info->block_cnt[BLOCK_8X8] =
			info->FUNC_SEL_INTRA_OPT_8X8_CNT.bits.intra_opt_8x8_cnt;
		stream_info->block_cnt[BLOCK_16X16] =
			info->FUNC_SEL_INTRA_OPT_16X16_CNT.bits.intra_opt_16x16_cnt;
		stream_info->block_cnt[BLOCK_32X32] = 0;
		lcu_size = LCU_SIZE_H264;
	}

	lcu_block_num = (((width + lcu_size - 1) / lcu_size) * ((height + lcu_size - 1) / lcu_size));
	lcu8_block_num = (((width + BASE_BLOCK_SIZE - 1) / BASE_BLOCK_SIZE) *
		((height + BASE_BLOCK_SIZE - 1) / BASE_BLOCK_SIZE));

	stream_info->ave_madi =
		info->FUNC_PME_MADI_SUM.bits.pme_madi_sum / lcu_block_num;
	stream_info->frame_madp = info->FUNC_PME_MADP_SUM.bits.pme_madp_sum;
	stream_info->average_luma_qp =
		info->FUNC_SEL_TOTAL_LUMA_QP.bits.total_luma_qp / lcu8_block_num;
	stream_info->average_lcu_mse = info->FUNC_SAO_MSE_SUM / lcu_block_num;

	hal_cmdlist_get_stream_len(stream_info, base_addr);
	hal_cmdlist_get_qp(stream_info, base_addr);
}

void vedu_hal_cfg_smmu(struct encode_info *channel_cfg, uint32_t core_id)
{
}

