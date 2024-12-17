/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2022-2022. All rights reserved.
 * Description: ufs debug header file
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */

#ifndef __UFSHCD_DEBUG_H__
#define __UFSHCD_DEBUG_H__
#include "ufshcd.h"
ssize_t config_desc_store(struct device *dev,
				 struct device_attribute *attr, const char *buf,
				 size_t count);
ssize_t config_desc_show(struct device *dev,
				struct device_attribute *attr, char *buf);
ssize_t unique_number_show(struct device *dev,
					 struct device_attribute *attr,
					 char *buf);
ssize_t man_id_show(struct device *dev, struct device_attribute *attr, char *buf);
ssize_t shared_alloc_units_show(struct device *dev,
				       struct device_attribute *attr, char *buf);
ssize_t spec_version_show(struct device *dev, struct device_attribute *attr, char *buf);

void ufshcd_complete_time(struct ufs_hba *hba, struct ufshcd_lrb *lrbp,
				 uint8_t opcode, ktime_t complete_time);
void ufshcd_print_host_regs(struct ufs_hba *hba);
void ufshcd_print_gic(struct ufs_hba *hba);
void ufshcd_init_ufs_temp_sys(struct ufs_hba *hba);
void ufshcd_fsr_dump_handler(struct work_struct *work);
void ufshcd_ufs_set_dieid(struct ufs_hba *hba, struct ufs_dev_info *dev_info);
void ufshcd_init_fsr_sys(struct ufs_hba *hba);
void hufs_parse_dev_desc(struct ufs_hba *hba,
				    struct ufs_dev_info *dev_info,
				    const u8 *desc_buf);
void ufshcd_add_rpm_lvl_sysfs_nodes(struct ufs_hba *hba);
void ufshcd_add_spm_lvl_sysfs_nodes(struct ufs_hba *hba);

static inline void ufshcd_add_sysfs_nodes(struct ufs_hba *hba)
{
	ufshcd_add_rpm_lvl_sysfs_nodes(hba);
	ufshcd_add_spm_lvl_sysfs_nodes(hba);
}

static inline void ufshcd_remove_sysfs_nodes(struct ufs_hba *hba)
{
	device_remove_file(hba->dev, &hba->rpm_lvl_attr);
	device_remove_file(hba->dev, &hba->spm_lvl_attr);
}

#endif
