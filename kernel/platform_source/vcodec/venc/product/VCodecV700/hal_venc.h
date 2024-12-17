/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2010-2019. All rights reserved.
 * Description: venc register config
 * Create: 2019-08-30
 */

#ifndef __HAL_VENC_H__
#define __HAL_VENC_H__

#include "vcodec_type.h"
#include "drv_venc_ioctl.h"
#include "smmu.h"
#include "hal_cmdlist.h"
#include "hal_common.h"
#include "drv_venc_efl.h"

#ifdef PCIE_UDP
#define VENC_COMPATIBLE       "hisilicon,VCodecV600-venc"
#else
#define VENC_COMPATIBLE       "hisilicon,VCodecV700-venc"
#endif

#define VEDU_VCPI_SPMEM_CTRL_0_VALUE       0x00015858
#define VEDU_VCPI_SPMEM_CTRL_1_VALUE       0x55554505

#define VENC_ES_SUPPORT
#define VENC_FPGAFLAG_ES      "fpga_es"
#define VENC_PCTRL_PERI_ES    0xFEC3E0BC
#define VENC_EXISTBIT_ES      0x38

#define VENC_CS_SUPPORT
#define VENC_FPGAFLAG_CS      "fpga_cs"
#define VENC_PCTRL_PERI_CS    0xFEC3E0BC
#define VENC_EXISTBIT_CS      0x38
#define VENC_EXIST_TRUE       0x8
#define AXIDFX_OFFSET (offsetof(S_HEVC_AVC_REGS_TYPE, AXIDFX_REGS))

/* used for ref streams syscache config */
#define VENC_REFLD_DATA_START_SID  0
#define VENC_REFLD_DATA_END_SID  10   // refld_h_chrom_sid + 1
#define VENC_REC_DATA_START_SID 21
#define VENC_REC_DATA_END_SID 31      // rec_h_chrom_sid + 1

void venc_hal_clr_all_int(S_HEVC_AVC_REGS_TYPE *vedu_reg);
void venc_hal_start_encode(S_HEVC_AVC_REGS_TYPE *vedu_reg);
void venc_mcore_start_encode(struct encode_info *info, S_HEVC_AVC_REGS_TYPE *vedu_reg);
void venc_hal_get_reg_venc(struct stream_info *stream_info, uint32_t *reg_base);
void venc_hal_get_reg_stream_len(struct stream_info *stream_info, uint32_t *reg_base);
void venc_hal_cfg_curld_osd01(struct encode_info *channel_cfg, uint32_t *reg_base);
void venc_hal_disable_all_int(S_HEVC_AVC_REGS_TYPE *vedu_reg);
void venc_hal_get_slice_reg(struct stream_info *stream_info, uint32_t *reg_base);
void venc_hal_cfg_platform_diff(struct encode_info *channel_cfg, uint32_t *reg_base);
void venc_hal_cfg_plat_diff_all_cfg(struct encode_info *channel_cfg, uint32_t *reg_base);
bool venc_device_need_bypass(void);
void vedu_hal_cfg(volatile uint32_t *base, uint32_t offset, uint32_t value);
void vedu_hal_set_int(uint32_t *reg_base);
int32_t vedu_hal_check_addr(const S_HEVC_AVC_REGS_TYPE_CFG *reg, venc_buffer_info_t *mem_info);
#endif
