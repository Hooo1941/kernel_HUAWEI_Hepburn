/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2022-2022. All rights reserved.
 * Description: atf_regs_dump.h
 */

#ifndef __SAVE_SOC_REGISTER_H__
#define __SAVE_SOC_REGISTER_H__

struct soc_regs_info_kernel{
	unsigned int soc_reg_mask;
	unsigned int soc_reg_size;
	char soc_reg_name[32];
};

#ifdef CONFIG_DFX_SAVE_SOC_REGISTER
void save_soc_regs(const char *path, unsigned int atf_regs_mask);
#else
static inline void save_soc_regs(const char *path, unsigned int atf_regs_mask){}
#endif
#endif
