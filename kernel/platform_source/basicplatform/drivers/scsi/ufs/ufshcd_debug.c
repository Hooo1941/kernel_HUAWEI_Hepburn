/*
 * ufshcd_debug.c
 *
 * basic interface for hufs
 *
 * Copyright (c) Huawei Technologies Co., Ltd. 2022. All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 and
 * only version 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 */

#include "ufshcd_debug.h"
#include "ufshcd.h"
#include "ufshcd_extend.h"

#ifdef CONFIG_SCSI_UFS_HI1861_VCMD
#ifdef CONFIG_PLATFORM_DIEID
#define UFS_HIXXXX_PRODUCT_NAME                 "SS6100GBCV100"
#define HIXXXX_PROD_LEN                         (13)
#define UFS_PRODUCT_NAME_THOR920                "THR920"
#define UFS_PRODUCT_NAME_THOR925                "THR925"
#define UFS_PRODUCT_NAME_SS6100                 "SS6100"
#define UFS_PRODUCT_NAME_LEN                    (6)
static char ufs_product_name[UFS_PRODUCT_NAME_LEN + 1];
#endif
#endif

#if defined(CONFIG_SCSI_UFS_HI1861_VCMD) && defined(CONFIG_PLATFORM_DIEID)
static u8 *ufs_hixxxx_dieid;
static int is_fsr_read_failed;
#endif

const char *hufs_uic_link_state_to_string(
			enum uic_link_state state)
{
	switch (state) {
	case UIC_LINK_OFF_STATE:
		return "OFF";
	case UIC_LINK_ACTIVE_STATE:
		return "ACTIVE";
	case UIC_LINK_HIBERN8_STATE:
		return "HIBERN8";
	default:
		return "UNKNOWN";
	}
}

const char *hufs_dev_pwr_mode_to_string(
			enum ufs_dev_pwr_mode state)
{
	switch (state) {
	case UFS_ACTIVE_PWR_MODE:
		return "ACTIVE";
	case UFS_SLEEP_PWR_MODE:
		return "SLEEP";
	case UFS_POWERDOWN_PWR_MODE:
		return "POWERDOWN";
	default:
		return "UNKNOWN";
	}
}

ssize_t config_desc_store(struct device *dev,
				 struct device_attribute *attr, const char *buf,
				 size_t count)
{
	u32 value = 0;
	u32 origin_attr = 0;
	int ret;
	struct scsi_device *sdev = to_scsi_device(dev);
	struct ufs_hba *hba = shost_priv(sdev->host);

	ret = kstrtouint(buf, 0, &value);
	if (ret || value != 1) {
		dev_err(hba->dev, "%s invalid value ret %d value %d \n",
			__func__, ret, value);
		return -EINVAL;
	}
	ret = ufshcd_query_attr_retry(hba, UPIU_QUERY_OPCODE_READ_ATTR,
				      QUERY_ATTR_IDN_CONF_DESC_LOCK, 0, 0,
				      &origin_attr);
	if (ret) {
		dev_err(hba->dev, "%s read config desc lock failed \n",
			__func__);
		return -EBUSY;
	}
	dev_err(hba->dev, "%s config desc is %d\n", __func__, origin_attr);
	if (origin_attr == 1) {
		dev_err(hba->dev, "%s: config desc lock already locked \n",
			__func__);
		return -EPERM;
	}
	ret = ufshcd_query_attr_retry(hba, UPIU_QUERY_OPCODE_WRITE_ATTR,
				      QUERY_ATTR_IDN_CONF_DESC_LOCK, 0, 0,
				      &value);
	if (ret) {
		dev_err(hba->dev, "%s enable config desc lock failed \n",
			__func__);
		return -EBUSY;
	}
	return count;
}

ssize_t config_desc_show(struct device *dev,
				struct device_attribute *attr, char *buf)
{
	struct scsi_device *sdev = to_scsi_device(dev);
	struct ufs_hba *hba = shost_priv(sdev->host);
	u32 value = 0;
	int ret;

	ret = ufshcd_query_attr_retry(hba, UPIU_QUERY_OPCODE_READ_ATTR,
				      QUERY_ATTR_IDN_CONF_DESC_LOCK, 0, 0,
				      &value);
	if (ret) {
		dev_err(hba->dev, "%s read config desc lock failed \n",
			__func__);
		return -EBUSY;
	}
	return snprintf(buf, sizeof(u32), "%d\n", value);
}

ssize_t unique_number_show(struct device *dev,
					 struct device_attribute *attr,
					 char *buf)
{
	struct scsi_device *sdev = to_scsi_device(dev);
	struct ufs_hba *hba = shost_priv(sdev->host);
	int curr_len;

	curr_len = snprintf(
	    buf, PAGE_SIZE,
	    "%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%04x%04x\n",
	    hba->unique_number.serial_number[0],
	    hba->unique_number.serial_number[1],
	    hba->unique_number.serial_number[2],
	    hba->unique_number.serial_number[3],
	    hba->unique_number.serial_number[4],
	    hba->unique_number.serial_number[5],
	    hba->unique_number.serial_number[6],
	    hba->unique_number.serial_number[7],
	    hba->unique_number.serial_number[8],
	    hba->unique_number.serial_number[9],
	    hba->unique_number.serial_number[10],
	    hba->unique_number.serial_number[11],
	    hba->unique_number.manufacturer_date,
	    hba->unique_number.manufacturer_id);

	return curr_len;
}

ssize_t man_id_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	struct scsi_device *sdev = to_scsi_device(dev);
	struct ufs_hba *hba = shost_priv(sdev->host);
	int curr_len;

	curr_len = snprintf(buf, PAGE_SIZE, "%04x\n", hba->manufacturer_id);

	return curr_len;
}

ssize_t shared_alloc_units_show(struct device *dev,
				       struct device_attribute *attr, char *buf)
{
	struct scsi_device *sdev = to_scsi_device(dev);
	struct ufs_hba *hba = shost_priv(sdev->host);
	return snprintf(buf, PAGE_SIZE, "0x%x\n", hba->wb_shared_alloc_units);
}

ssize_t spec_version_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	struct scsi_device *sdev = to_scsi_device(dev);
	struct ufs_hba *hba = shost_priv(sdev->host);
	return snprintf(buf, PAGE_SIZE, "%x\n", hba->ufs_device_spec_version);
}

ssize_t ufshcd_rpm_lvl_show(struct device *dev,
		struct device_attribute *attr, char *buf)
{
	struct ufs_hba *hba = dev_get_drvdata(dev);
	int ret;
	int curr_len = 0;
	u8 lvl;

	if (hba->rpm_lvl >= UFS_PM_LVL_MAX)
		return 0;

	ret = snprintf(buf, PAGE_SIZE,
			"\nCurrent Runtime PM level [%d] => dev_state [%s] link_state [%s]\n",
			hba->rpm_lvl,
			hufs_dev_pwr_mode_to_string(
				ufs_pm_lvl_states[hba->rpm_lvl].dev_state),
			hufs_uic_link_state_to_string(
				ufs_pm_lvl_states[hba->rpm_lvl].link_state));
	if (ret <= 0)
		return -1;
	curr_len += ret;

	ret = snprintf((buf + curr_len), (PAGE_SIZE - curr_len),
			"\nAll available Runtime PM levels info:\n");
	if (ret <= 0)
		return -1;
	curr_len += ret;

	for (lvl = UFS_PM_LVL_0; lvl < UFS_PM_LVL_MAX; lvl++) {
		ret = snprintf((buf + curr_len), (PAGE_SIZE - curr_len),
				"\tRuntime PM level [%d] => dev_state [%s] link_state [%s]\n",
				lvl,
				hufs_dev_pwr_mode_to_string(
					ufs_pm_lvl_states[lvl].dev_state),
				hufs_uic_link_state_to_string(
					ufs_pm_lvl_states[lvl].link_state));
		if (ret <= 0)
			return -1;
		curr_len += ret;
	}

	return curr_len;
}

ssize_t ufshcd_pm_lvl_store(struct device *dev,
		struct device_attribute *attr,
		const char *buf, size_t count,
		bool rpm)
{
	struct ufs_hba *hba = dev_get_drvdata(dev);
	unsigned long flags, value;

	if (kstrtoul(buf, 0, &value))
		return -EINVAL;

	if (value >= UFS_PM_LVL_MAX)
		return -EINVAL;

	spin_lock_irqsave(hba->host->host_lock, flags);
	if (rpm)
		hba->rpm_lvl = value;
	else
		hba->spm_lvl = value;
	spin_unlock_irqrestore(hba->host->host_lock, flags);
	return count;
}

ssize_t ufshcd_rpm_lvl_store(struct device *dev,
		struct device_attribute *attr, const char *buf, size_t count)
{
	return ufshcd_pm_lvl_store(dev, attr, buf, count, true);
}

void ufshcd_add_rpm_lvl_sysfs_nodes(struct ufs_hba *hba)
{
	hba->rpm_lvl_attr.show = ufshcd_rpm_lvl_show;
	hba->rpm_lvl_attr.store = ufshcd_rpm_lvl_store;
	sysfs_attr_init(&hba->rpm_lvl_attr.attr);
	hba->rpm_lvl_attr.attr.name = "rpm_lvl";
	hba->rpm_lvl_attr.attr.mode = 0644;
	if (device_create_file(hba->dev, &hba->rpm_lvl_attr))
		dev_err(hba->dev, "Failed to create sysfs for rpm_lvl\n");
}

ssize_t ufshcd_spm_lvl_show(struct device *dev,
		struct device_attribute *attr, char *buf)
{
	struct ufs_hba *hba = dev_get_drvdata(dev);
	int curr_len;
	u8 lvl;

	if (hba->spm_lvl >= UFS_PM_LVL_MAX)
		return 0;

	curr_len = snprintf(buf, PAGE_SIZE,
			    "\nCurrent System PM level [%d] => dev_state [%s] link_state [%s]\n",
			    hba->spm_lvl,
			    hufs_dev_pwr_mode_to_string(
				ufs_pm_lvl_states[hba->spm_lvl].dev_state),
			    hufs_uic_link_state_to_string(
				ufs_pm_lvl_states[hba->spm_lvl].link_state));

	curr_len += snprintf((buf + curr_len), (PAGE_SIZE - curr_len),
			     "\nAll available System PM levels info:\n");
	for (lvl = UFS_PM_LVL_0; lvl < UFS_PM_LVL_MAX; lvl++)
		curr_len += snprintf((buf + curr_len), (PAGE_SIZE - curr_len),
				     "\tSystem PM level [%u] => dev_state [%s] link_state [%s]\n",
				    lvl,
				    hufs_dev_pwr_mode_to_string(
					ufs_pm_lvl_states[lvl].dev_state),
				    hufs_uic_link_state_to_string(
					ufs_pm_lvl_states[lvl].link_state));

	return curr_len;
}

ssize_t ufshcd_spm_lvl_store(struct device *dev,
		struct device_attribute *attr, const char *buf, size_t count)
{
	return ufshcd_pm_lvl_store(dev, attr, buf, count, false);
}

void ufshcd_add_spm_lvl_sysfs_nodes(struct ufs_hba *hba)
{
	hba->spm_lvl_attr.show = ufshcd_spm_lvl_show;
	hba->spm_lvl_attr.store = ufshcd_spm_lvl_store;
	sysfs_attr_init(&hba->spm_lvl_attr.attr);
	hba->spm_lvl_attr.attr.name = "spm_lvl";
	hba->spm_lvl_attr.attr.mode = 0644;
	if (device_create_file(hba->dev, &hba->spm_lvl_attr))
		dev_err(hba->dev, "Failed to create sysfs for spm_lvl\n");
}

void ufshcd_complete_time(struct ufs_hba *hba, struct ufshcd_lrb *lrbp,
		uint8_t opcode, ktime_t complete_time)
{
	s64 cost_time;

	lrbp->compl_time_stamp = complete_time;
	cost_time = ktime_ms_delta(lrbp->compl_time_stamp,
				   lrbp->issue_time_stamp);
	/* if request costs 2000ms, print it */
	if (cost_time >= 2000)
		dev_info(hba->dev,
			"opcode 0x%x, cost %lldms, issue %lld, complete %lld\n",
			opcode, cost_time, lrbp->issue_time_stamp,
			lrbp->compl_time_stamp);
}

void ufshcd_print_uic_err_hist(struct ufs_hba *hba,
		struct ufs_uic_err_reg_hist *err_hist, char *err_name)
{
	unsigned int i;

	for (i = 0; i < UIC_ERR_REG_HIST_LENGTH; i++) {
		if (err_hist->reg[i] == 0)
			continue;
		dev_err(hba->dev, "%s[%u] = 0x%x at %llu us\n", err_name, i,
			err_hist->reg[i], ktime_to_us(err_hist->tstamp[i]));
	}
}

#ifdef CONFIG_SCSI_UFS_HI1861_VCMD
static int ufshcd_read_fsr_info(struct ufs_hba *hba,
				u8 desc_id,
				int desc_index,
				u8 *param_read_buf,
				u32 param_size)
{
	int ret;
	struct ufs_query_vcmd cmd = { 0 };

	cmd.buf_len = param_size;
	/* allocate memory to hold full descriptor */
	cmd.desc_buf = kmalloc(cmd.buf_len, GFP_KERNEL);
	if (!cmd.desc_buf)
		return -ENOMEM;
	memset(cmd.desc_buf, 0, cmd.buf_len);

	cmd.opcode = UPIU_QUERY_OPCODE_READ_HI1861_FSR;
	cmd.idn = desc_id;
	cmd.index = desc_index;
	cmd.query_func = UPIU_QUERY_FUNC_STANDARD_READ_REQUEST;

	ret = ufshcd_query_vcmd_retry(hba, &cmd);
	if (ret) {
		dev_err(hba->dev, "%s: Failed reading FSR. desc_id %d "
				  "buff_len %d ret %d",
			__func__, desc_id, cmd.buf_len, ret);

		goto out;
	}
	memcpy(param_read_buf, cmd.desc_buf, cmd.buf_len);
out:
	kfree(cmd.desc_buf);

	return ret;
}

static int ufshcd_read_fsr(struct ufs_hba *hba, u8 *buf, u32 size)
{
	if (UFS_VENDOR_HI1861 != hba->manufacturer_id)
		return -EINVAL;
	if (!buf)
		return -ENOMEM;
	if (!size)
		return -EINVAL;

#ifdef CONFIG_SCSI_UFS_UNISTORE
	if (ufshcd_rw_buffer_is_enabled(hba))
		return ufshcd_get_fsr_by_read_buffer(hba, buf, (u16)size);
#endif
	return ufshcd_read_fsr_info(hba, 0, 0, buf, size);
}
#endif

#ifdef CONFIG_SCSI_UFS_HI1861_VCMD
/**
* ufshcd_get_fsr_command -scsi layer get fsr func
* @hba: Pointer to adapter instance
* @buf: the buf pointer to fsr info
* @size:the buf size, just like 4k byte
*/
int ufshcd_get_fsr_command(struct ufs_hba *hba, u8 *buf,
				u32 size)
{
	if (!hba) {
		pr_err( "%s shost_priv host failed\n", __func__);
		return -1;
	}
	return ufshcd_read_fsr(hba, buf, size);
}
EXPORT_SYMBOL(ufshcd_get_fsr_command);

static ssize_t ufshcd_fsr_info_show(struct device *dev,
				struct device_attribute *attr, char *buf)
{
	struct ufs_hba *hba = dev_get_drvdata(dev);
	/* allocate memory to hold full descriptor */
	u8 *fbuf = NULL;
	int len = 0;
	int i = 0;
	int ret = 0;

	if (UFS_VENDOR_HI1861 != hba->manufacturer_id)
		return 0;
	fbuf = kmalloc(HI1861_FSR_INFO_SIZE, GFP_KERNEL);
	if (!fbuf)
		return -ENOMEM;

	memset(fbuf, 0, HI1861_FSR_INFO_SIZE);
	ret = ufshcd_read_fsr(hba, fbuf, HI1861_FSR_INFO_SIZE);
	if (ret) {
		kfree(fbuf);
		dev_err(hba->dev, "[%s]READ FSR FAILED\n", __func__);
		return ret;
	}
	/*lint -save -e661 -e662*/
	for (i = 0; i < HI1861_FSR_INFO_SIZE; i = i + 16)
		len += snprintf(buf + len, PAGE_SIZE - len ,
	"0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x\n",
		*(fbuf + i + 0), *(fbuf + i + 1), *(fbuf + i + 2), *(fbuf + i + 3),
		*(fbuf + i + 4), *(fbuf + i + 5), *(fbuf + i + 6), *(fbuf + i + 7),
		*(fbuf + i + 8), *(fbuf + i + 9), *(fbuf + i + 10), *(fbuf + i + 11),
		*(fbuf + i + 12), *(fbuf + i + 13), *(fbuf + i + 14), *(fbuf + i + 15));
	/*lint -restore*/
	kfree(fbuf);
	return len;
}
#endif

void hufs_parse_dev_desc(struct ufs_hba *hba,
				    struct ufs_dev_info *dev_info,
				    const u8 *desc_buf)
{
	dev_info->spec_version =
		get_unaligned_be16(&desc_buf[DEVICE_DESC_PARAM_SPEC_VER]);
	dev_info->wmanufacturer_date =
		get_unaligned_be16(&desc_buf[DEVICE_DESC_PARAM_MANF_DATE]);
	dev_info->serial_num_index = desc_buf[DEVICE_DESC_PARAM_SN];
	if (hba->desc_size[QUERY_DESC_IDN_DEVICE] >= DEVICE_DESC_PARAM_FEATURE)
		dev_info->vendor_feature = get_unaligned_be32(
			&desc_buf[DEVICE_DESC_PARAM_FEATURE]);

	hba->manufacturer_id = dev_info->wmanufacturerid;
	hba->manufacturer_date = dev_info->wmanufacturer_date;
	hba->ufs_device_spec_version = dev_info->spec_version;

	dev_err(hba->dev, "ufs spec version: 0x%x\n",
		hba->ufs_device_spec_version);

	if (hba->ufs_device_spec_version >= UFS_DEVICE_SPEC_3_1 ||
		hba->ufs_device_spec_version == UFS_DEVICE_SPEC_2_2) {
		hba->wb_type = desc_buf[DEVICE_DESC_PARAM_WB_TYPE];
		hba->wb_shared_alloc_units = get_unaligned_be32(
			desc_buf + DEVICE_DESC_PARAM_WB_SHARED_ALLOC_UNITS);
		dev_err(hba->dev, "write shared alloc units = 0x%x\n",
			hba->wb_shared_alloc_units);

		hba->d_ext_ufs_feature_sup = get_unaligned_be32(
			desc_buf + DEVICE_DESC_PARAM_EXT_UFS_FEATURE_SUP);
		dev_err(hba->dev, "d_ext_ufs_feature_sup = 0x%x\n",
			hba->d_ext_ufs_feature_sup);
	}
}

#ifdef CONFIG_SCSI_UFS_HI1861_VCMD
/**
* ufshcd_init_fsr_sys -init fsr attr node
* @hba: Pointer to adapter instance
* the node only to be read
*/
void ufshcd_init_fsr_sys(struct ufs_hba *hba)
{
	hba->ufs_fsr.fsr_attr.show = ufshcd_fsr_info_show;
	sysfs_attr_init(&hba->ufs_fsr.fsr_attr.attr);
	hba->ufs_fsr.fsr_attr.attr.name = "ufs_fsr";
	hba->ufs_fsr.fsr_attr.attr.mode = S_IRUSR | S_IRGRP;
	if (device_create_file(hba->dev, &hba->ufs_fsr.fsr_attr))
		dev_err(hba->dev, "Failed to create sysfs for ufs fsrs\n");
}

#ifdef CONFIG_PLATFORM_DIEID
void ufshcd_ufs_set_dieid(struct ufs_hba *hba, struct ufs_dev_info *dev_info)
{
	/* allocate memory to hold full descriptor */
	u8 *fbuf = NULL;
	int ret = 0;

	if (hba->manufacturer_id != UFS_VENDOR_HI1861)
		return;

#ifdef CONFIG_SCSI_UFS_UNISTORE
	if (hba->host && hba->host->unistore_enable)
		return;
#endif
	if (ufs_hixxxx_dieid == NULL)
		ufs_hixxxx_dieid = kmalloc(UFS_DIEID_TOTAL_SIZE, GFP_KERNEL);
	if (!ufs_hixxxx_dieid)
		return;

	memset(ufs_hixxxx_dieid, 0, UFS_DIEID_TOTAL_SIZE);

	fbuf = kmalloc(HI1861_FSR_INFO_SIZE, GFP_KERNEL);
	if (!fbuf) {
		kfree(ufs_hixxxx_dieid);
		ufs_hixxxx_dieid = NULL;
		return;
	}

	memset(fbuf, 0, HI1861_FSR_INFO_SIZE);

	ret = ufshcd_read_fsr_info(hba, 0, 0, fbuf, HI1861_FSR_INFO_SIZE);
	if (ret) {
		is_fsr_read_failed = 1;
		dev_err(hba->dev, "[%s]READ FSR FAILED\n", __func__);
		goto out;
	}

	/* get ufs product name */
	ret = snprintf(ufs_product_name, UFS_PRODUCT_NAME_LEN, dev_info->model);
	if (ret <= 0) {
		dev_err(hba->dev, "[%s]copy ufs product name fail\n", __func__);
		goto out;
	}
	ret = strncmp(UFS_HIXXXX_PRODUCT_NAME, dev_info->model, HIXXXX_PROD_LEN);
	if (ret != 0) {
		/* after hi1861 ver. */
		memcpy(ufs_hixxxx_dieid, fbuf + 16, UFS_DIEID_NUM_SIZE_THOR920);
		memcpy(ufs_hixxxx_dieid + UFS_DIEID_CHIP_VER_OFFSET,
			fbuf + 36, UFS_NAND_CHIP_VER_SIZE);
		memcpy(ufs_hixxxx_dieid + UFS_DIEID_CONTROLLER_OFFSET,
			fbuf + 256, UFS_CONTROLLER_DIEID_SIZE);
		memcpy(ufs_hixxxx_dieid + UFS_DIEID_FLASH_OFFSET,
			fbuf + 448, UFS_FLASH_DIE_ID_SIZE);
		memcpy(ufs_hixxxx_dieid + UFS_DCIP_CRACK_NOW_OFFSET,
			fbuf + 440, UFS_DCIP_CRACK_NOW_SIZE);
		memcpy(ufs_hixxxx_dieid + UFS_DCIP_CRACK_EVER_OFFSET,
			fbuf + 441, UFS_DCIP_CRACK_EVER_SIZE);
	} else {
		/* hi1861 ver. */
		memcpy(ufs_hixxxx_dieid, fbuf + 12, UFS_DIEID_NUM_SIZE);
		memcpy(ufs_hixxxx_dieid + UFS_DIEID_CHIP_VER_OFFSET,
			fbuf + 28, UFS_NAND_CHIP_VER_SIZE);
		memcpy(ufs_hixxxx_dieid + UFS_DIEID_CONTROLLER_OFFSET,
			fbuf + 1692, UFS_CONTROLLER_DIEID_SIZE);
		memcpy(ufs_hixxxx_dieid + UFS_DIEID_FLASH_OFFSET,
			fbuf + 1900, UFS_FLASH_DIE_ID_SIZE);
	}
out:
	kfree(fbuf);
}

int hufs_get_flash_dieid(
	char *dieid, u32 offset, u32 dieid_num, u8 vendor_id, u32 *flash_id)
{
	int len = 0;
	int i = 0;
	int j = 0;
	int flag = 0;
	int ret = 0;

	dieid += offset;
	/*
	 * T vendor flash id, the length is 32B.As is required,
	 * the output flash ids need to formatted in hex with appropriate prefix
	 * eg:\r\nDIEID_UFS_FLASH_B:0x00CD...\r\n
	 *    \r\nDIEID_UFS_FLASH_C:0xAC3D...\r\n
	 */
	/*lint -save -e574 -e679 */
	if (offset > UFS_DIEID_BUFFER_SIZE)
		return -EINVAL;

	if (((strncmp(ufs_product_name, UFS_PRODUCT_NAME_THOR920,
		      UFS_PRODUCT_NAME_LEN) == 0) ||
	     (strncmp(ufs_product_name, UFS_PRODUCT_NAME_SS6100,
		      UFS_PRODUCT_NAME_LEN) == 0)) &&
	    (vendor_id == UFS_FLASH_VENDOR_M)) {
		for (i = 0; i < dieid_num; i++) {
			if (dieid_num == UFS_FLASH_FOUR_DIE &&
			    (i == UFS_FLASH_TWO_DIE ||
			     i == UFS_FLASH_THREE_DIE)) {
				i += 2;
				flag = 1;
			}

			ret = snprintf(
				dieid + len,
				UFS_DIEID_BUFFER_SIZE - len - offset,
				"\r\nDIEID_UFS_FLASH_%c:0x%08X%08X%08X%08X00000000000000000000000000000000\r\n",
				'B' + j++, *(flash_id + i * 4),
				*(flash_id + i * 4 + 1),
				*(flash_id + i * 4 + 2),
				*(flash_id + i * 4 + 3));
			if (ret <= 0)
				return -2;
			len += ret;

			if (flag) {
				flag = 0;
				i -= 2;
			}
		}
	} else if ((vendor_id == UFS_FLASH_VENDOR_T) ||
		   (vendor_id == UFS_FLASH_VENDOR_H) ||
		   (vendor_id == UFS_FLASH_VENDOR_M) ||
		   (vendor_id == UFS_FLASH_VENDOR_Y)) {
		for (i = 0; i < dieid_num; i++) {
			ret = snprintf(
				dieid + len,
				UFS_DIEID_BUFFER_SIZE - len - offset,
				"\r\nDIEID_UFS_FLASH_%c:0x%08X%08X%08X%08X\r\n",
				'B' + i, *(flash_id + i * 4),
				*(flash_id + i * 4 + 1),
				*(flash_id + i * 4 + 2),
				*(flash_id + i * 4 + 3));
			if (ret <= 0)
				return -2;
			len += ret;
		}
	} else {
		return -2;
	}

	return 0;
}
#endif
#endif

#ifdef CONFIG_PLATFORM_DIEID
int hufs_get_dieid(char *dieid, unsigned int len)
{
#ifdef CONFIG_SCSI_UFS_HI1861_VCMD
	int length = 0;
	int ret = 0;
	u32 dieid_num = 0;
	u8 vendor_id = 0;
	u32 *controller_id = NULL;
	u32 *flash_id = NULL;
	u8 die_crack_now = 0;
	u8 die_crack_ever = 0;
	char buf[UFS_DIEID_BUFFER_SIZE] = {0};

	if (dieid == NULL || ufs_hixxxx_dieid == NULL)
		return -2;
	if (is_fsr_read_failed)
		return -1;

	dieid_num = *(u32 *)ufs_hixxxx_dieid;
	vendor_id = *(u8 *)(ufs_hixxxx_dieid + UFS_DIEID_CHIP_VER_OFFSET);
	controller_id = (u32 *)(ufs_hixxxx_dieid + UFS_DIEID_CONTROLLER_OFFSET);
	flash_id = (u32 *)(ufs_hixxxx_dieid + UFS_DIEID_FLASH_OFFSET);
	die_crack_now = *(u8 *)(ufs_hixxxx_dieid + UFS_DCIP_CRACK_NOW_OFFSET);
	die_crack_ever = *(u8 *)(ufs_hixxxx_dieid + UFS_DCIP_CRACK_EVER_OFFSET);

	ret = snprintf(buf, UFS_DIEID_BUFFER_SIZE,
			"\r\nDIEID_UFS_CONTROLLER_A:0x%08X%08X%08X%08X%08X%08X%08X%08X\r\n",
			*controller_id, *(controller_id + 1),         /* 1: array index */
			*(controller_id + 2), *(controller_id + 3),   /* 2, 3: array index */
			*(controller_id + 4), *(controller_id + 5),   /* 4, 5: array index */
			*(controller_id + 6), *(controller_id + 7));  /* 6, 7: array index */
	if (ret <= 0)
		return -2;
	length += ret;

	ret = hufs_get_flash_dieid(
		buf, length, dieid_num, vendor_id, flash_id);
	if (ret != 0)
		return ret;

	length = strlen(buf);

	ret = snprintf(buf + length, UFS_DIEID_BUFFER_SIZE - length,
		"\r\nCRACK_NOW:0x%08X\r\n\r\nCRACK_EVER:0x%08X\r\n",
		die_crack_now, die_crack_ever);
	if (ret <= 0)
		return -2;

	if (len >= strlen(buf))
		strncat(dieid, buf, strlen(buf));
	else
		return strlen(buf);

	return 0;
#else
	return -1;
#endif
}
#endif

#ifdef CONFIG_SCSI_UFS_HI1861_VCMD
void ufshcd_fsr_dump_handler(struct work_struct *work)
{
	struct ufs_hba *hba;
	u8 *fbuf = NULL;
	int i = 0;
	int ret = 0;

	hba = container_of(work, struct ufs_hba, fsr_work);

	if (UFS_VENDOR_HI1861 != hba->manufacturer_id)
		return;
	/* allocate memory to hold full descriptor */
	fbuf = kmalloc(HI1861_FSR_INFO_SIZE, GFP_KERNEL);
	if (!fbuf)
		return;
	memset(fbuf, 0, HI1861_FSR_INFO_SIZE);

	ret = ufshcd_read_fsr(hba, fbuf, HI1861_FSR_INFO_SIZE);
	if (ret) {
		kfree(fbuf);
		dev_err(hba->dev, "[%s]READ FSR FAILED\n", __func__);
		return;
	}
#ifdef CONFIG_DFX_DEBUG_FS
	dev_err(hba->dev, "===============UFS HI1861 FSR INFO===============\n");
#endif
	/*lint -save -e661 -e662*/
	for (i = 0 ; i < HI1861_FSR_INFO_SIZE; i = i + 16)
		dev_err(hba->dev, "0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x\n",
		*(fbuf + i + 0), *(fbuf + i + 1), *(fbuf + i + 2), *(fbuf + i + 3),
		*(fbuf + i + 4), *(fbuf + i + 5), *(fbuf + i + 6), *(fbuf + i + 7),
		*(fbuf + i + 8), *(fbuf + i + 9), *(fbuf + i + 10), *(fbuf + i + 11),
		*(fbuf + i + 12), *(fbuf + i + 13), *(fbuf + i + 14), *(fbuf + i + 15));
	/*lint -restore*/
	kfree(fbuf);
}
#endif

static ssize_t ufshcd_ufs_temp_show(struct device *dev,
				struct device_attribute *attr, char *buf)
{
	int len = 0;
	struct ufs_hba *hba = dev_get_drvdata(dev);
	hba->ufs_temp.temp = get_soc_temp() / 1000;
	if (hba->ufs_temp.temp <= 0) {
		dsm_ufs_update_error_info(hba, DSM_UFS_TEMP_LOW_ERR);
		schedule_ufs_dsm_work(hba);
	}
	dev_info(hba->dev, "UFS temp is %d\n", hba->ufs_temp.temp);
	len = snprintf(buf, PAGE_SIZE, "%d\n", hba->ufs_temp.temp);
	return len;
}

void ufshcd_init_ufs_temp_sys(struct ufs_hba *hba)
{
	hba->ufs_temp.temp_attr.show = ufshcd_ufs_temp_show;
	sysfs_attr_init(&hba->ufs_temp.temp_attr.attr);
	hba->ufs_temp.temp_attr.attr.name = "ufs_temp";
	hba->ufs_temp.temp_attr.attr.mode = S_IRUSR | S_IRGRP;
	if (device_create_file(hba->dev, &hba->ufs_temp.temp_attr))
		dev_err(hba->dev, "Failed to create sysfs for ufs_temp\n");
}

void ufshcd_print_host_regs(struct ufs_hba *hba)
{
	if (hba->vops && hba->vops->dbg_hci_dump)
		hba->vops->dbg_hci_dump(hba);
	if (ufshcd_is_hufs_hc(hba) && hba->vops &&
	    hba->vops->dbg_hufs_dme_dump)
		hba->vops->dbg_hufs_dme_dump(hba);
#ifdef CONFIG_HUFS_HC
	if (hba->vops && hba->vops->dbg_uic_dump)
		hba->vops->dbg_uic_dump(hba);
#endif
	/*
	 * hex_dump reads its data without the readl macro. This might
	 * cause inconsistency issues on some platform, as the printed
	 * values may be from cache and not the most recent value.
	 * To know whether you are looking at an un-cached version verify
	 * that IORESOURCE_MEM flag is on when xxx_get_resource() is invoked
	 * during platform/pci probe function.
	 */
	ufshcd_dump_regs(hba, 0, UFSHCI_REG_SPACE_SIZE, "host_regs: ");
	dev_err(hba->dev, "hba->ufs_version = 0x%x, hba->capabilities = 0x%x\n",
		hba->ufs_version, hba->capabilities);
	dev_err(hba->dev,
		"hba->outstanding_reqs = 0x%x, hba->outstanding_tasks = 0x%x\n",
		(u32)hba->outstanding_reqs, (u32)hba->outstanding_tasks);
	dev_err(hba->dev,
		"last_hibern8_exit_tstamp at %lld us, hibern8_exit_cnt = %d\n",
		ktime_to_us(hba->ufs_stats.last_hibern8_exit_tstamp),
		hba->ufs_stats.hibern8_exit_cnt);

	ufshcd_print_uic_err_hist(hba, &hba->ufs_stats.pa_err, "pa_err");
	ufshcd_print_uic_err_hist(hba, &hba->ufs_stats.dl_err, "dl_err");
	ufshcd_print_uic_err_hist(hba, &hba->ufs_stats.nl_err, "nl_err");
	ufshcd_print_uic_err_hist(hba, &hba->ufs_stats.tl_err, "tl_err");
	ufshcd_print_uic_err_hist(hba, &hba->ufs_stats.dme_err, "dme_err");

	ufshcd_print_clk_freqs(hba);

	if (hba->vops && hba->vops->dbg_register_dump)
		hba->vops->dbg_register_dump(hba);
}

void ufshcd_print_gic(struct ufs_hba *hba)
{
	if (hba->vops && hba->vops->dbg_gic_dump)
		hba->vops->dbg_gic_dump(hba);
}