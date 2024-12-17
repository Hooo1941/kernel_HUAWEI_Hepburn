/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
 * Description: hw_stat psi common
 * Author: Fan Weiwei <fanweiwei1@huawei.com>
 * Create: 2023-03-16
 */
#include "hw_stat_psi.h"

#ifdef CONFIG_HW_STAT_PSI_LRB
#include "hw_stat_psi_lrb.h"
#endif

void hw_stat_update_psi(void)
{
#ifdef CONFIG_HW_STAT_PSI_LRB
	update_psi_lrb();
#endif
}

void hw_stat_psi_init(struct proc_dir_entry *parent)
{
#ifdef CONFIG_HW_STAT_PSI_LRB
	hw_stat_psi_lrb_init(parent);
#endif
}

