/*
 * dsm_ufs.c
 *
 * deal with dsm_ufs
 *
 * Copyright (c) 2019 Huawei Technologies Co., Ltd.
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
#include "dsm_ufs.h"
#include <dsm/dsm_pub.h>
#include <linux/async.h>
#include <linux/of_gpio.h>
#include "ufs.h"
#include "ufshcd.h"
#include "unipro.h"
#include "mphy-unipro-vs.h"
#include "ufshcd_hufs_interface.h"
#include "linux/mfd/pmic_platform.h"
#include "linux/ufs/hufs_hcd.h"
#include <securec.h>

#ifdef CONFIG_HISYSEVENT
struct error_no_ufs_map {
	int error_no;
	char *name;
};

#define OK  0
#define ERR (-1)

static int ufs_errorno_to_str(int error_no, char *str, int str_len)
{
	int i;
	struct error_no_ufs_map isp_error_no_ufs_map[] = {
		{928008000, "UFS_POWER_MODE_FAIL"},
		{928008001, "UFS_RW_FAIL"},
		{928008002, "UFS_SYS_BUS_ERR"},
		{928008003, "UFS_UIC_TRAN_ERR"},
		{928008004, "UFS_UIC_CMD_ERR"},
		{928008005, "UFS_CONTROLLER_ERR"},
		{928008006, "UFS_DEVICE_ERR"},
		{928008007, "UFS_TIMEOUT_ERR"},
		{928008008, "UFS_UTP_ERR"},
		{928008009, "UFS_ASSERT_NUM_ERR"},
		{928008010, "UFS_VOLT_ERR"},
		{928008011, "UFS_LINKUP_ERR"},
		{928008012, "UFS_ENTER_OR_EXIT_H8_ERR"},
		{928008013, "UFS_LIFETIME_EXCCED_ERR"},
		{928008014, "UFS_HARDWARE_RESET"},
		{928008015, "UFS_MEDIUM_ERR"},
		{928008016, "UFS_PA_INIT_ERR"},
		{928008017, "UFS_INLINE_CRYPTO_ERR"},
		{928008018, "UFS_HARDWARE_ERR"},
		{928008019, "UFS_LINE_RESET_ERR"},
		{928008021, "UFS_HI186X_FSR"},
		{928008022, "UFS_DEVICE_FATAL_ERROR"},
		{928008023, "UFS_RTBB_ABNORMAL"},
		{928008024, "UFS_HINFO_ABNORMAL"},
		{928008025, "UFS_PROGRAM_FAIL_CNT"},
		{928008026, "UFS_ERASE_FAIL_CNT"},
		{928008027, "UFS_IO_TIMEOUT"},
		{928008028, "UFS_PWRMODE_SWITCH"},
		{925205300, "UFS_POWER_DOWN_FLUSH_ERR"},
		{925205510, "UFS_MPHY_CALIBRATION_INFO"},
	};
	int isp_error_no_map_len = sizeof(isp_error_no_ufs_map) / sizeof(struct error_no_ufs_map);
	for (i = 0; i < isp_error_no_map_len; i++) {
		if (isp_error_no_ufs_map[i].error_no == error_no) {
			if (strlen(isp_error_no_ufs_map[i].name) >= str_len)
				return ERR;
			if (strncpy_s(str, str_len, isp_error_no_ufs_map[i].name, str_len - 1) != EOK)
				return ERR;
			str[str_len - 1] = '\0';
			pr_err("dsm ufs_errorno_to_str str=%s\n", str);
			return OK;
		}
	}
	return ERR;
}

struct dsm_client_ops hisp_dsm_ops_ufs = {
	.poll_state = NULL,
	.dump_func = NULL,
#ifdef CONFIG_HISYSEVENT
	.errorno_to_str = ufs_errorno_to_str,
#endif
};
#endif

/* lint -e747 -e713 -e715 -e732 -e835 */
struct ufs_dsm_log  g_ufs_dsm_log;
static struct ufs_dsm_adaptor g_ufs_dsm_adaptor;
struct dsm_client *ufs_dclient;
unsigned int ufs_dsm_real_upload_size;
static int dsm_ufs_enable;
static int g_device_fatal_err_cnt;
struct ufs_uic_err_history uic_err_history;
struct dsm_dev dsm_ufs = {
#ifdef CONFIG_HISYSEVENT
	.name = "STORAGE",
	.device_name = NULL,
	.ic_name = NULL,
	.module_name = NULL,
	.fops = &hisp_dsm_ops_ufs,
	.buff_size = UFS_DSM_BUFFER_SIZE,
#else
	.name = "dsm_ufs",
	.buff_size = UFS_DSM_BUFFER_SIZE,
#endif
};

struct dsm_err_type_category err_type_category[] = {
	{UFS_FASTBOOT_PWMODE_ERR, DSM_UFS_FASTBOOT_PWMODE_ERR, "Fastboot PwrMode Error"},
	{UFS_FASTBOOT_RW_ERR, DSM_UFS_FASTBOOT_RW_ERR, "Fastboot RW Error"},
	{UFS_UIC_TRANS_ERR, DSM_UFS_UIC_TRANS_ERR, "UIC No Fatal Error"},
	{UFS_UIC_CMD_ERR, DSM_UFS_UIC_CMD_ERR, "UIC Cmd Error"},
	{UFS_CONTROLLER_ERR, DSM_UFS_CONTROLLER_ERR, "Ufs Controller Error"},
	{UFS_DEV_ERR, DSM_UFS_DEV_ERR, "UFS Device Error"},
	{UFS_TIMEOUT_ERR, DSM_UFS_TIMEOUT_ERR, "UFS TimeOut Error"},
	{UFS_UTP_TRANS_ERR, DSM_UFS_UTP_ERR, "UTP Transfer Error"},
	{UFS_SCSI_CMD_ERR, DSM_UFS_SCSI_CMD_ERR, "UFS SCSI CMD Error"},
	{UFS_VOLT_GPIO_ERR, DSM_UFS_VOLT_GPIO_ERR, "UFS Volt Fall Error"},
	{UFS_LINKUP_ERR, DSM_UFS_LINKUP_ERR, "UFS Linkup Error"},
	{UFS_ENTER_OR_EXIT_H8_ERR, DSM_UFS_ENTER_OR_EXIT_H8_ERR, "UFS Enter/Exit H8 Fail"},
	{UFS_LIFETIME_EXCCED_ERR, DSM_UFS_LIFETIME_EXCCED_ERR, "UFS Life time exceed"},
	{UFS_HARDWARE_RESET, DSM_UFS_HARDWARE_RESET, "UFS_HARDWARE_RESET"},
	{UFS_MEDIUM_ERR, DSM_UFS_MEDIUM_ERR, "UFS_MEDIUM_ERR"},
	{UFS_PA_INIT_ERR, DSM_UFS_PA_INIT_ERR, "UFS_PA_INIT_ERR"},
	{UFS_INLINE_CRYPTO_ERR, DSM_UFS_INLINE_CRYPTO_ERR, "UFS inline crypto error"},
	{UFS_HARDWARE_ERR, DSM_UFS_HARDWARE_ERR, "UFS hardware error:Sense_key = 4"},
	{UFS_LINE_RESET_ERR, DSM_UFS_LINE_RESET_ERR, "UFS occur Line reset"},
	{UFS_TEMP_LOW_ERR, DSM_UFS_TEMP_LOW_ERR, "UFS temperature too low"},
	{UFS_HI1861_INTERNEL_ERR, DSM_UFS_HI1861_INTERNEL_ERR, "DSM_UFS_HI1861_INTERNEL_ERR"},
	{UFS_POWER_DOWN_FLUSH_ERR, DSM_UFS_POWER_DOWN_FLUSH_ERR, "UFS_POWER_DOWN_FLUSH_ERR"},
	{UFS_MPHY_CALIBRATION_INFO, DSM_UFS_MPHY_CALIBRATION_INFO, "UFS_MPHY_CALIBRATION_INFO"},
	{UFS_DEVICE_FATAL_ERR, DSM_UFS_DEVICE_FATAL_ERROR, "UFS DEVICE FATAL ERROR, OCS = 8"},
#ifdef CONFIG_SCSI_UFS_GEAR_CTRL
	{UFS_DYNAMIC_SWITCH_SPEED_ERR, DSM_UFS_DYNAMIC_SWITCH_SPEED_ERR, "DSM_UFS_DYNAMIC_SWITCH_SPEED_ERR"},
#endif
};

EXPORT_SYMBOL(ufs_dclient);

unsigned long dsm_ufs_if_err(void)
{
	return g_ufs_dsm_adaptor.err_type;
}

int dsm_ufs_update_upiu_info(struct ufs_hba *hba, int tag, int err_code)
{
	struct scsi_cmnd *scmd = NULL;
	int i;
	if (!dsm_ufs_enable)
		return -1;
	if (err_code == DSM_UFS_TIMEOUT_ERR) {
		if (test_and_set_bit_lock(UFS_TIMEOUT_ERR,
			&g_ufs_dsm_adaptor.err_type))
			return -1;
	} else if (err_code == DSM_UFS_POWER_DOWN_FLUSH_ERR) {
		if (test_and_set_bit_lock(UFS_POWER_DOWN_FLUSH_ERR,
			&g_ufs_dsm_adaptor.err_type))
			return -1;
	} else {
		return -1;
	}
	if ((tag != -1) && (hba->lrb[tag].cmd)) {
		scmd = hba->lrb[tag].cmd;
		for (i = 0; i < COMMAND_SIZE(scmd->cmnd[0]); i++)
		g_ufs_dsm_adaptor.timeout_scmd[i] = scmd->cmnd[i];
	} else {
		for (i = 0; i < MAX_CDB_SIZE; i++)
		g_ufs_dsm_adaptor.timeout_scmd[i] = 0;
	}
	g_ufs_dsm_adaptor.timeout_slot_id = tag;
	g_ufs_dsm_adaptor.tr_doorbell = read_utr_doorbell(hba);
	g_ufs_dsm_adaptor.outstanding_reqs = hba->outstanding_reqs;

	return 0;
}

static u32 crypto_conf_index[CRYPTO_CONFIG_LEN];
int dsm_ufs_updata_ice_info(struct ufs_hba *hba)
{
	unsigned int i;
	if (!dsm_ufs_enable)
		return -1;
	if (test_and_set_bit_lock(UFS_INLINE_CRYPTO_ERR,
	&g_ufs_dsm_adaptor.err_type))
		return -1;
	g_ufs_dsm_adaptor.ice_outstanding = hba->outstanding_reqs;
	g_ufs_dsm_adaptor.ice_doorbell = read_utr_doorbell(hba);
	for (i = 0; i < (unsigned int)hba->nutrs; i++) {
		if (!((g_ufs_dsm_adaptor.ice_doorbell >> i) & 0x1))
			continue;
#ifdef CONFIG_DFA_MCQ
		if (is_dfa_enabled(hba)) {
			crypto_conf_index[i] =
				hba->lrb[i].dlrb.dfa_utr_descriptor_ptr->header.dword_0;
			continue;
		}
#endif
		if (ufshcd_is_hufs_hc(hba))
			crypto_conf_index[i] =
				(hba->lrb[i].hufs_utr_descriptor_ptr)
					->header.dword_0;
		else
			crypto_conf_index[i] = (hba->lrb[i].utr_descriptor_ptr)
						       ->header.dword_0;
	}
	return 0;
}
int dsm_ufs_update_scsi_info(struct ufshcd_lrb *lrbp,
	int scsi_status, int err_code)
{
	struct scsi_cmnd *scmd = NULL;
	int i;
	if (!dsm_ufs_enable)
		return -1;
	switch (err_code) {
	case DSM_UFS_MEDIUM_ERR:
		if (test_and_set_bit_lock(UFS_MEDIUM_ERR,
			&g_ufs_dsm_adaptor.err_type))
			return -1;
		break;
	case DSM_UFS_SCSI_CMD_ERR:
		if (test_and_set_bit_lock(UFS_SCSI_CMD_ERR,
			&g_ufs_dsm_adaptor.err_type))
			return -1;
		break;
	case DSM_UFS_HARDWARE_ERR:
		if (test_and_set_bit_lock(UFS_HARDWARE_ERR,
			&g_ufs_dsm_adaptor.err_type))
			return -1;
		break;
	default:
		return -1;
	}
	if (lrbp->cmd)
		scmd = lrbp->cmd;
	else
		return -1;
	g_ufs_dsm_adaptor.scsi_status = scsi_status;
	for (i = 0; i < COMMAND_SIZE(scmd->cmnd[0]); i++)
		g_ufs_dsm_adaptor.timeout_scmd[i] = scmd->cmnd[i];
	for (i = 0; i < SCSI_SENSE_BUFFERSIZE; i++)
		g_ufs_dsm_adaptor.sense_buffer[i] = lrbp->sense_buffer[i];
	return 0;
}

int dsm_ufs_update_ocs_info(struct ufs_hba *hba, int err_code, int ocs)
{
	if (!dsm_ufs_enable)
		return -1;
	switch (err_code) {
	case DSM_UFS_UTP_ERR:
		if (test_and_set_bit_lock(UFS_UTP_TRANS_ERR, &g_ufs_dsm_adaptor.err_type))
			return -1;
		break;
	case DSM_UFS_DEVICE_FATAL_ERROR:
		if (test_and_set_bit_lock(UFS_DEVICE_FATAL_ERR, &g_ufs_dsm_adaptor.err_type))
			return -1;
		break;
	default:
		return -1;
	}
	g_ufs_dsm_adaptor.ocs = ocs;
	return 0;
}

int dsm_ufs_update_fastboot_info(struct ufs_hba *hba)
{
	char *pstr = NULL;
	unsigned int err_code;
	if (!dsm_ufs_enable)
		return -1;
	pstr = strstr(saved_command_line, "fastbootdmd=");
	if (!pstr) {
		pr_err("No fastboot dmd info\n");
		return -EINVAL;
	}
	if (1 != sscanf(pstr, "fastbootdmd=%d", &err_code)) {
		pr_err("Failed to get err_code\n");
		return -EINVAL;
	}
	if (err_code) {
		if (err_code & FASTBOOTDMD_PWR_ERR)
			if (test_and_set_bit_lock(UFS_FASTBOOT_PWMODE_ERR,
				&g_ufs_dsm_adaptor.err_type))
				return -1;
		if (err_code & FASTBOOTDMD_RW_ERR)
			if (test_and_set_bit_lock(UFS_FASTBOOT_RW_ERR,
				&g_ufs_dsm_adaptor.err_type))
				return -1;
	} else {
		return -EINVAL;
	}
	return 0;
}

static void dsm_ufs_fastboot_async(void *data, async_cookie_t cookie)
{
	int ret;
	struct ufs_hba *hba = (struct ufs_hba *)data;
	ret = dsm_ufs_update_fastboot_info(hba);
	if (!ret) {
		pr_info("Ufs get fastboot dmd err info\n");
		if (dsm_ufs_enabled())
			queue_work(system_freezable_wq, &hba->dsm_work);
	}
}

void dsm_ufs_enable_uic_err(struct ufs_hba *hba)
{
	unsigned long flags;
	clear_bit_unlock(0x0, &(g_ufs_dsm_adaptor.uic_disable));
	spin_lock_irqsave(hba->host->host_lock, flags); /* lint !e550 */
	ufshcd_enable_intr(hba, UIC_ERROR);
	spin_unlock_irqrestore(hba->host->host_lock, flags); /* lint !e550 */
	return;
}

int dsm_ufs_disable_uic_err(void)
{
	return (test_and_set_bit_lock(0x0, &(g_ufs_dsm_adaptor.uic_disable)));
}

int dsm_ufs_if_uic_err_disable(void)
{
	return (test_bit(0x0, &(g_ufs_dsm_adaptor.uic_disable)));
}

int dsm_ufs_update_uic_info(struct ufs_hba *hba, int err_code)
{
	int pos;
	if (!dsm_ufs_enable)
		return -1;
	if (dsm_ufs_if_uic_err_disable())
		return 0;
	if (test_and_set_bit_lock(UFS_UIC_TRANS_ERR, &g_ufs_dsm_adaptor.err_type))
		return -1;
	pos = (hba->ufs_stats.event[UFS_EVT_PA_ERR].pos - 1) % UIC_ERR_REG_HIST_LENGTH;
	g_ufs_dsm_adaptor.uic_uecpa = hba->ufs_stats.event[UFS_EVT_PA_ERR].val[pos];
	pos = (hba->ufs_stats.event[UFS_EVT_DL_ERR].pos - 1) % UIC_ERR_REG_HIST_LENGTH;
	g_ufs_dsm_adaptor.uic_uecdl = hba->ufs_stats.event[UFS_EVT_DL_ERR].val[pos];
	pos = (hba->ufs_stats.event[UFS_EVT_NL_ERR].pos - 1) % UIC_ERR_REG_HIST_LENGTH;
	g_ufs_dsm_adaptor.uic_uecn = hba->ufs_stats.event[UFS_EVT_NL_ERR].val[pos];
	pos = (hba->ufs_stats.event[UFS_EVT_TL_ERR].pos - 1) % UIC_ERR_REG_HIST_LENGTH;
	g_ufs_dsm_adaptor.uic_uect = hba->ufs_stats.event[UFS_EVT_TL_ERR].val[pos];
	pos = (hba->ufs_stats.event[UFS_EVT_DME_ERR].pos - 1) % UIC_ERR_REG_HIST_LENGTH;
	g_ufs_dsm_adaptor.uic_uedme = hba->ufs_stats.event[UFS_EVT_DME_ERR].val[pos];
	if (g_ufs_dsm_adaptor.uic_uecpa & UIC_PHY_ADAPTER_LAYER_ERROR_LINE_RESET) {
		if (test_and_set_bit_lock(UFS_LINE_RESET_ERR,
			&g_ufs_dsm_adaptor.err_type))
			return -1;
	}
	if (g_ufs_dsm_adaptor.uic_uecdl & UIC_DATA_LINK_LAYER_ERROR_PA_INIT) {
		if (test_and_set_bit_lock(UFS_PA_INIT_ERR, &g_ufs_dsm_adaptor.err_type))
			return -1;
	}
	return 0;
}

static void update_uic_info(struct ufs_hba *hba)
{
	g_ufs_dsm_adaptor.uic_info[0] = ufshcd_readl(hba, REG_UIC_COMMAND);
	g_ufs_dsm_adaptor.uic_info[UIC_INFO_CMD_INDEX_1] = ufshcd_readl(hba,
		REG_UIC_COMMAND_ARG_1);
	g_ufs_dsm_adaptor.uic_info[UIC_INFO_CMD_INDEX_2] = ufshcd_readl(hba,
		REG_UIC_COMMAND_ARG_2);
	g_ufs_dsm_adaptor.uic_info[UIC_INFO_CMD_INDEX_3] = ufshcd_readl(hba,
		REG_UIC_COMMAND_ARG_3);
}

#ifdef CONFIG_SCSI_UFS_GEAR_CTRL
int dsm_ufs_update_ufs_dynamic_switch_info(struct ufs_hba *hba, u32 type, u32 set_gear, int subcode, int code)
{
	int ret = DSM_UFS_DSS_ERR;

	if (!dsm_ufs_enable)
		return ret;
	switch (code) {
	case DSM_UFS_DYNAMIC_SWITCH_SPEED_ERR:
		if (subcode == UFS_DSS_SWITCH_ERR) {
			(void)test_and_set_bit_lock(UFS_DYNAMIC_SWITCH_SPEED_ERR,
				&g_ufs_dsm_adaptor.err_type);
			g_ufs_dsm_adaptor.ufs_dss_sub_err |= (1 << subcode);
			g_ufs_dsm_adaptor.ufs_dss_set = set_gear;
			g_ufs_dsm_adaptor.ufs_dss_type = type;
			ret = DSM_UFS_DSS_SCHEDUAL;
		} else if (subcode == UFS_DSS_INFO_SET_LOW || subcode == UFS_DSS_INFO_SET_HIGH) {
			if (g_ufs_dsm_adaptor.ufs_dss_magic != UFS_DSS_MAGIC) {
				g_ufs_dsm_adaptor.ufs_dss_magic = UFS_DSS_MAGIC;
				g_ufs_dsm_adaptor.ufs_dss_count = 0;
			}
			g_ufs_dsm_adaptor.ufs_dss_count++;
			ret = DSM_UFS_DSS_NOSCHEDUAL;
		} else {
			g_ufs_dsm_adaptor.ufs_dss_sub_err |= (1 << subcode);
			ret = DSM_UFS_DSS_SCHEDUAL;
		}
		break;
	default:
		return ret;
	}
	return ret;
}
#endif

int dsm_ufs_update_error_info(struct ufs_hba *hba, int code)
{
	if (!dsm_ufs_enable)
		return -1;
	switch (code) {
	case DSM_UFS_VOLT_GPIO_ERR:
		(void)test_and_set_bit_lock(UFS_VOLT_GPIO_ERR,
			&g_ufs_dsm_adaptor.err_type);
		break;
	case DSM_UFS_LINKUP_ERR:
		(void)test_and_set_bit_lock(UFS_LINKUP_ERR,
			&g_ufs_dsm_adaptor.err_type);
		break;
	case DSM_UFS_UIC_CMD_ERR:
		if (test_and_set_bit_lock(UFS_UIC_CMD_ERR,
			&g_ufs_dsm_adaptor.err_type))
			break;
		update_uic_info(hba);
		break;
	case DSM_UFS_CONTROLLER_ERR:
		(void)test_and_set_bit_lock(UFS_CONTROLLER_ERR,
				&g_ufs_dsm_adaptor.err_type);
		break;
	case DSM_UFS_PA_INIT_ERR:
		(void)test_and_set_bit_lock(UFS_PA_INIT_ERR, &g_ufs_dsm_adaptor.err_type);
		break;
	case DSM_UFS_DEV_ERR:
		(void)test_and_set_bit_lock(UFS_DEV_ERR,
				&g_ufs_dsm_adaptor.err_type);
		break;
	case DSM_UFS_ENTER_OR_EXIT_H8_ERR:
		(void)test_and_set_bit_lock(UFS_ENTER_OR_EXIT_H8_ERR,
				&g_ufs_dsm_adaptor.err_type);
		break;
	case DSM_UFS_LIFETIME_EXCCED_ERR:
		(void)test_and_set_bit_lock(UFS_LIFETIME_EXCCED_ERR,
			&g_ufs_dsm_adaptor.err_type);
		break;
	case DSM_UFS_TEMP_LOW_ERR:
		(void)test_and_set_bit_lock(UFS_TEMP_LOW_ERR,
			&g_ufs_dsm_adaptor.err_type);
		g_ufs_dsm_adaptor.temperature = hba->ufs_temp.temp;
		break;
#if defined(CONFIG_SCSI_UFS_SCORPIO) ||                         \
	defined(CONFIG_SCSI_UFS_SGR)
	case DSM_UFS_MPHY_CALIBRATION_INFO:
		(void)test_and_set_bit_lock(UFS_MPHY_CALIBRATION_INFO,
			&g_ufs_dsm_adaptor.err_type);
		break;
#endif
	default:
		return 0;
	}
	return 0;
}

void dsm_ufs_update_lifetime_info(u8 lifetime_a, u8 lifetime_b)
{
	g_ufs_dsm_adaptor.lifetime_a = lifetime_a;
	g_ufs_dsm_adaptor.lifetime_b = lifetime_b;
}

/* put error message into buffer */
#ifdef CONFIG_BOOTDEVICE
void get_bootdevice_product_name(char *product_name, u32 len);
#endif

static void set_dsm_dev_ic_module_name(const char *product_name, const char *rev)
{
	int ret;
	dsm_ufs.ic_name = product_name;
	dsm_ufs.module_name = rev;
	ret = dsm_update_client_vendor_info(&dsm_ufs);
	if (ret)
		pr_err("update dsm ufs ic_name and module_name failed!\n");
	dsm_ufs.ic_name = NULL;
	dsm_ufs.module_name = NULL;
}

static void update_dsm_log_module_name(struct ufs_hba *hba, char **log_buff,
	int *size)
{
	int ret;
	char product_name[PRODUCT_NAME_LEN] = {0};
	char rev[REV_LEN] = {0};

#ifdef CONFIG_BOOTDEVICE
	/* depend on hufs bootdevice */
	get_bootdevice_product_name(product_name, PRODUCT_NAME_LEN);
	ret = snprintf(*log_buff, *size, "%s,",
		product_name);
	*log_buff += ret;
	*size -= ret;
#endif
	if ((hba->sdev_ufs_device != NULL)
		&& (hba->sdev_ufs_device->rev != NULL))
		snprintf(rev, REV_LEN, "%.4s,", hba->sdev_ufs_device->rev);
	ret = snprintf(*log_buff, *size, "%s,", rev);
	*log_buff += ret;
	*size -= ret;
	set_dsm_dev_ic_module_name(product_name, rev);
}

static void update_fastboot_pwmode_err_log(char *log_buff, int *size,
	unsigned long code)
{
	int ret;
	if (!(g_ufs_dsm_adaptor.err_type & (1 << UFS_FASTBOOT_PWMODE_ERR))) {
		pr_err("UFS DSM Error! Err Num:%lu err_type:%lu\n",
			code, g_ufs_dsm_adaptor.err_type);
		return;
	}
	if (*size < DSM_LOG_BUFF_LEFT) {
		pr_err("dsm log buff left space too small to load log.\n");
		return;
	}
	ret = snprintf(log_buff, *size,
			"Fastboot Power Mode Change Err\n");
	log_buff += ret;
	*size -= ret;
}

static void update_fastboot_rw_err_log(char *log_buff, int *size,
	unsigned long code)
{
	int ret;
	if (!(g_ufs_dsm_adaptor.err_type & (1 << UFS_FASTBOOT_RW_ERR))) {
		pr_err("UFS DSM Error! Err Num:%lu err_type:%lu\n",
			code, g_ufs_dsm_adaptor.err_type);
		return;
	}
	if (*size < DSM_LOG_BUFF_LEFT) {
		pr_err("dsm log buff left space too small to load log.\n");
		return;
	}
	ret = snprintf(log_buff, *size, "Fastboot Read Write Err\n");
	log_buff += ret;
	*size -= ret;
}

static void update_uic_trans_line_reset_err_log(char *log_buff, int *size,
	unsigned long code)
{
	int ret;
	if (!(g_ufs_dsm_adaptor.err_type & ((1<<UFS_UIC_TRANS_ERR) |
		(1<<UFS_LINE_RESET_ERR) | (1 << UFS_PA_INIT_ERR)))) {
		pr_err("UFS DSM Error! Err Num: %lu, %lu\n",
			code, g_ufs_dsm_adaptor.err_type);
		return;
	}
	if (*size < DSM_LOG_BUFF_LEFT) {
		pr_err("dsm log buff left space too small to load log.\n");
		return;
	}
	ret = snprintf(log_buff, *size,
			"UECPA:%08x,UECDL:%08x,UECN:%08x,UECT:%08x,UEDME:%08x\n",
			g_ufs_dsm_adaptor.uic_uecpa, g_ufs_dsm_adaptor.uic_uecdl,
			g_ufs_dsm_adaptor.uic_uecn,
			g_ufs_dsm_adaptor.uic_uect, g_ufs_dsm_adaptor.uic_uedme);
	log_buff += ret;
	*size -= ret;
}

static void update_uic_cmd_err_log(char *log_buff, int *size,
	unsigned long code)
{
	int ret;
	if (!(g_ufs_dsm_adaptor.err_type & (1<<UFS_UIC_CMD_ERR))) {
		pr_err("UFS DSM Error! Err Num: %lu, %ld\n",
			code, g_ufs_dsm_adaptor.err_type);
		return;
	}
	if (*size < DSM_LOG_BUFF_LEFT) {
		pr_err("dsm log buff left space too small to load log.\n");
		return;
	}
	ret = snprintf(log_buff, *size,
			"UIC_CMD:%08x,ARG1:%08x,ARG2:%08x,ARG3:%08x\n",
			g_ufs_dsm_adaptor.uic_info[0],
			g_ufs_dsm_adaptor.uic_info[UIC_INFO_CMD_INDEX_1],
			g_ufs_dsm_adaptor.uic_info[UIC_INFO_CMD_INDEX_2],
			g_ufs_dsm_adaptor.uic_info[UIC_INFO_CMD_INDEX_3]);
	log_buff += ret;
	*size -= ret;
}

#if defined(CONFIG_SCSI_UFS_SCORPIO) ||                         \
	defined(CONFIG_SCSI_UFS_SGR)
static void update_device_reg_log(struct ufs_hba *hba, char *log_buff,
	int *size, unsigned long code)
{
	u32 val0;
	u32 val1;
	int ret;
	if (!(g_ufs_dsm_adaptor.err_type & (1 << UFS_MPHY_CALIBRATION_INFO))) {
		pr_err("UFS DSM Error! Err Num: %lu, %ld\n",
			code, g_ufs_dsm_adaptor.err_type);
		return;
	}
	if (*size < DSM_LOG_BUFF_LEFT) {
		pr_err("dsm log buff left space too small to load log.\n");
		return;
	}
	val0 = pmic_read_reg(VP_VSET);
	val1 = pmic_read_reg(VPH_VEST);

	ret = snprintf(log_buff, *size,
			"UFS Voltage vp: 0x%x, vph: 0x%x\n",
			val0, val1);
	log_buff += ret;
	*size -= ret;
	(void)hufs_uic_read_reg(hba, ATTR_MTX0(MPHY_SET_VCO_BAND_I), &val0);
	(void)hufs_uic_read_reg(hba, ATTR_MTX1(MPHY_SET_VCO_BAND_I), &val1);
	ret = snprintf(log_buff, *size,
			"UFS Mphy TX 0x%02x, Lane0: 0x%08x, Lane1: 0x%08x\n",
			MPHY_SET_VCO_BAND_I, val0, val1);
	log_buff += ret;
	*size -= ret;
	(void)hufs_uic_read_reg(hba, ATTR_MTX0(MPHY_VCO_CNT_VALUE_I), &val0);
	(void)hufs_uic_read_reg(hba, ATTR_MTX1(MPHY_VCO_CNT_VALUE_I), &val1);
	ret = snprintf(log_buff, *size,
			"UFS Mphy TX 0x%02x, Lane0: 0x%08x, Lane1: 0x%08x\n",
			MPHY_VCO_CNT_VALUE_I, val0, val1);
	log_buff += ret;
	*size -= ret;
	(void)hufs_uic_read_reg(hba, ATTR_MTX0(MPHY_VCO_CNT_VALUE_II), &val0);
	(void)hufs_uic_read_reg(hba, ATTR_MTX1(MPHY_VCO_CNT_VALUE_II), &val1);
	ret = snprintf(log_buff, *size,
			"UFS Mphy TX 0x%02x, Lane0: 0x%08x, Lane1: 0x%08x\n",
			MPHY_VCO_CNT_VALUE_II, val0, val1);
	log_buff += ret;
	*size -= ret;
}
#endif

static void update_common_err_log(char *log_buff, int *size,
	unsigned long code)
{
	int ret;
#ifndef CONFIG_HUFS_HC_CORE_UTR
	int i;
#endif
	if (!(g_ufs_dsm_adaptor.err_type & ((1 << UFS_CONTROLLER_ERR) |
		(1 << UFS_INLINE_CRYPTO_ERR) | (1 << UFS_DEV_ERR) |
		(1 << UFS_ENTER_OR_EXIT_H8_ERR)))) {
		pr_err("UFS DSM Error! Err Num: %lu, %ld\n",
			code, g_ufs_dsm_adaptor.err_type);
		return;
	}
	if (*size < DSM_LOG_BUFF_LEFT) {
		pr_err("dsm log buff left space too small to load log.\n");
		return;
	}
#ifndef CONFIG_HUFS_HC_CORE_UTR
	if (g_ufs_dsm_adaptor.err_type & (1 << UFS_INLINE_CRYPTO_ERR)) {
		ret = snprintf(log_buff, *size, "outstanding:0x%lx, doorbell: 0x%lx\n",
			g_ufs_dsm_adaptor.ice_outstanding, g_ufs_dsm_adaptor.ice_doorbell);
		log_buff += ret;
		*size -= ret;
		for (i = 0; i < CRYPTO_CONFIG_LEN; i++) {
			if (crypto_conf_index[i]) {
				ret = snprintf(log_buff, *size, "UTP_DESC[%d]: 0x%x\n", i,
					crypto_conf_index[i]);
				log_buff += ret;
				*size -= ret;
				crypto_conf_index[i] = 0;
			}
		}
	}
#endif
	if ((g_ufs_dsm_adaptor.err_type & (1 << UFS_CONTROLLER_ERR))) {
		ret = snprintf(log_buff, *size, "controller error\n");
		log_buff += ret;
		*size -= ret;
	}
	ret = snprintf(log_buff, *size, "UECDL:%08x,UECN:%08x\n",
		g_ufs_dsm_adaptor.uic_uecdl, g_ufs_dsm_adaptor.uic_uecn);
	log_buff += ret;
	*size -= ret;
	ret = snprintf(log_buff, *size, "UECT:%08x,UEDME:%08x\n",
			g_ufs_dsm_adaptor.uic_uect, g_ufs_dsm_adaptor.uic_uedme);
	log_buff += ret;
	*size -= ret;
}

static void update_device_fatal_err_log(char *log_buff, int *size,
	unsigned long code)
{
	int ret;
	if (!(g_ufs_dsm_adaptor.err_type & (1 << UFS_DEVICE_FATAL_ERR))) {
		pr_err("UFS DSM Error! Err Num: %lu, %lu\n",
			code, g_ufs_dsm_adaptor.err_type);
		return;
	}
	if (*size < DSM_LOG_BUFF_LEFT) {
		pr_err("dsm log buff left space too small to load log.\n");
		return;
	}
	ret = snprintf_s(log_buff, *size, *size - 1,
			"OCS=8, UFS Device Fatal Error, ocs:%d\n", g_ufs_dsm_adaptor.ocs);
	log_buff += ret;
	*size -= ret;
}


static void update_utp_err_log(char *log_buff, int *size,
	unsigned long code)
{
	int ret;
	if (!(g_ufs_dsm_adaptor.err_type & (1 << UFS_UTP_TRANS_ERR))) {
		pr_err("UFS DSM Error! Err Num: %lu, %lu\n",
			code, g_ufs_dsm_adaptor.err_type);
		return;
	}
	if (*size < DSM_LOG_BUFF_LEFT) {
		pr_err("dsm log buff left space too small to load log.\n");
		return;
	}
	ret = snprintf(log_buff, *size, "UTP_OCS_ERR:%d\n", g_ufs_dsm_adaptor.ocs);
	log_buff += ret;
	*size -= ret;
}

static void update_timeout_err_log(char *log_buff, int *size,
	unsigned long code)
{
	int ret, i;
	if (!(g_ufs_dsm_adaptor.err_type & ((1 << UFS_TIMEOUT_ERR) |
		(1 << UFS_HARDWARE_RESET)))) {
		pr_err("UFS DSM Error! Err Num: %lu, %ld\n",
			code, g_ufs_dsm_adaptor.err_type);
		return;
	}
	if (*size < DSM_LOG_BUFF_LEFT) {
		pr_err("dsm log buff left space too small to load log.\n");
		return;
	}
	ret = snprintf(log_buff, *size,
			"UTP_tag:%02x,doorbell:%lx,outstandings:%lu, "
			"pwrmode:0x%x, tx_gear:%d, rx_gear:%d, scsi_cdb:\n",
			g_ufs_dsm_adaptor.timeout_slot_id,
			g_ufs_dsm_adaptor.tr_doorbell,
			g_ufs_dsm_adaptor.outstanding_reqs,
			g_ufs_dsm_adaptor.pwrmode,
			g_ufs_dsm_adaptor.tx_gear,
			g_ufs_dsm_adaptor.rx_gear);
	log_buff += ret;
	*size -= ret;
	for (i = 0; i < COMMAND_SIZE(g_ufs_dsm_adaptor.timeout_scmd[0]); i++) {
		ret = snprintf(log_buff, *size, "%02x",
			g_ufs_dsm_adaptor.timeout_scmd[i]);
		log_buff += ret;
		*size -= ret;
	}
}

static void update_hardware_common_err_log(char *log_buff, int *size,
	unsigned long code)
{
	int ret, i;
	if (!(g_ufs_dsm_adaptor.err_type & ((1 << UFS_SCSI_CMD_ERR) |
		(1 << UFS_MEDIUM_ERR) | (1 << UFS_HARDWARE_ERR)))) {
		pr_err("UFS DSM Error! Err Num: %lu, %ld\n",
			code, g_ufs_dsm_adaptor.err_type);
		return;
	}
	if (*size < DSM_LOG_BUFF_LEFT) {
		pr_err("dsm log buff left space too small to load log.\n");
		return;
	}
	ret = snprintf(log_buff, *size, "scsi_status:%x scsi_cdb:\n",
		g_ufs_dsm_adaptor.scsi_status);
	log_buff += ret;
	*size -= ret;
	for (i = 0; i < COMMAND_SIZE(g_ufs_dsm_adaptor.timeout_scmd[0]); i++) {
		ret = snprintf(log_buff, *size, "%02x",
			g_ufs_dsm_adaptor.timeout_scmd[i]);
		log_buff += ret;
		*size -= ret;
	}
	ret = snprintf(log_buff, *size,
		"\t\tResponse_code:%02x, Sense_key:%02x, ASC:%02x, ASCQ:%x\n",
		g_ufs_dsm_adaptor.sense_buffer[0],
		g_ufs_dsm_adaptor.sense_buffer[SENSE_KEY_OFFSET],
		g_ufs_dsm_adaptor.sense_buffer[ASC_OFFSET],
		g_ufs_dsm_adaptor.sense_buffer[ASCQ_OFFSET]);
	log_buff += ret;
	*size -= ret;
}

static void update_vplt_gpio_err_log(char *log_buff, int *size,
	unsigned long code)
{
	int ret;
	if (!(g_ufs_dsm_adaptor.err_type & (1 << UFS_VOLT_GPIO_ERR))) {
		pr_err("UFS DSM Error! Err Num: %lu, %lu\n",
			code, g_ufs_dsm_adaptor.err_type);
		return;
	}
	if (*size < DSM_LOG_BUFF_LEFT) {
		pr_err("dsm log buff left space too small to load log.\n");
		return;
	}
	ret = snprintf(log_buff, *size,
			"UFS BUCK has got a falling down Volt\n");
	log_buff += ret;
	*size -= ret;
}

static void update_linkup_err_log(struct ufs_hba *hba, char *log_buff,
	int *size, unsigned long code)
{
	int ret;
	if (!(g_ufs_dsm_adaptor.err_type & (1 << UFS_LINKUP_ERR))) {
		pr_err("UFS DSM Error! Err Num: %lu, %lu\n",
			code, g_ufs_dsm_adaptor.err_type);
		return;
	}
	if (*size < DSM_LOG_BUFF_LEFT) {
		pr_err("dsm log buff left space too small to load log.\n");
		return;
	}
	ret = snprintf(log_buff, *size,
			"UFS Linkup Line not correct, lane_rx:%d, lane_tx:%d\n",
			hba->max_pwr_info.info.lane_rx,
			hba->max_pwr_info.info.lane_tx);
	log_buff += ret;
	*size -= ret;
}

#ifdef CONFIG_SCSI_UFS_GEAR_CTRL
static void update_dynamic_switch_speed_switch_err_log(struct ufs_hba *hba, char *log_buff,
	int *size, unsigned long code)
{
	int ret = 0;
	if (!(g_ufs_dsm_adaptor.err_type & (1 << UFS_DYNAMIC_SWITCH_SPEED_ERR))) {
		pr_err("UFS DSM Error! Err Num: %lu, %lu\n", code, g_ufs_dsm_adaptor.err_type);
		return;
	}
	if (*size < DSM_LOG_BUFF_LEFT) {
		pr_err("dsm log buff left space too small to load log.\n");
		return;
	}

	if (g_ufs_dsm_adaptor.ufs_dss_sub_err & (1 << UFS_DSS_SWITCH_ERR)) {
		ret = snprintf_s(
			log_buff, *size, *size - 1,
			"UFS dynamic switch failed(%x), gear_rx:%d, gear_tx:%d, "
			"lane_rx:%d, lane_tx:%d, pwr_rx:%d, pwr_tx:%d, type %s, set %s\n",
			g_ufs_dsm_adaptor.ufs_dss_sub_err, hba->pwr_info.gear_rx, hba->pwr_info.gear_tx,
			hba->pwr_info.lane_rx, hba->pwr_info.lane_tx, hba->pwr_info.pwr_rx, hba->pwr_info.pwr_tx,
			g_ufs_dsm_adaptor.ufs_dss_type == 0 ? "gear" : "lane",
			g_ufs_dsm_adaptor.ufs_dss_set == 0 ? "low" : "high");
		ret += snprintf_s(log_buff + ret, *size - ret, *size - ret - 1, "switch count: %d!\n",
				  (g_ufs_dsm_adaptor.ufs_dss_magic == UFS_DSS_MAGIC) ? g_ufs_dsm_adaptor.ufs_dss_count :
										       0);
	} else {
		ret = snprintf_s(log_buff, *size, *size - 1, "UFS dynamic switch count period upload. %d!\n",
				 (g_ufs_dsm_adaptor.ufs_dss_magic == UFS_DSS_MAGIC) ? g_ufs_dsm_adaptor.ufs_dss_count :
										      0);
	}
	if (g_ufs_dsm_adaptor.ufs_dss_sub_err & (1 << UFS_DSS_RESET))
		ret += snprintf_s(log_buff + ret, *size - ret, *size - ret - 1, "Occurr ufs hard reset!\n");
	if (g_ufs_dsm_adaptor.ufs_dss_sub_err & (1 << UFS_DSS_FEATURE_CLOSE))
		ret += snprintf_s(log_buff + ret, *size - ret, *size - ret - 1,
				  "Occurr ufs dynamic switch speed feature closed!\n");
	g_ufs_dsm_adaptor.ufs_dss_sub_err = 0;
	g_ufs_dsm_adaptor.ufs_dss_type = 0;
	g_ufs_dsm_adaptor.ufs_dss_set = 0;
	log_buff += ret;
	*size -= ret;
}
#endif

static void update_lifetime_exceed_err_log(char *log_buff, int *size,
	unsigned long code)
{
	int ret;
	if (!(g_ufs_dsm_adaptor.err_type & (1 << UFS_LIFETIME_EXCCED_ERR))) {
		pr_err("UFS DSM Error! Err Num: %lu, %lu\n",
			code, g_ufs_dsm_adaptor.err_type);
		return;
	}
	if (*size < DSM_LOG_BUFF_LEFT) {
		pr_err("dsm log buff left space too small to load log.\n");
		return;
	}
	ret = snprintf(log_buff, *size, "life_time_a:%u, life_time_b:%u\n",
			g_ufs_dsm_adaptor.lifetime_a, g_ufs_dsm_adaptor.lifetime_b);
	log_buff += ret;
	*size -= ret;
}

static void update_temp_low_err_log(char *log_buff, int *size,
	unsigned long code)
{
	int ret;
	if (!(g_ufs_dsm_adaptor.err_type & (1 << UFS_TEMP_LOW_ERR))) {
		pr_err("UFS DSM Error! Err Num: %lu, %lu\n",
			code, g_ufs_dsm_adaptor.err_type);
		return;
	}
	if (*size < DSM_LOG_BUFF_LEFT) {
		pr_err("dsm log buff left space too small to load log.\n");
		return;
	}
	ret = snprintf(log_buff, *size,
		"UFS temp too low: %d\n", g_ufs_dsm_adaptor.temperature);
	log_buff += ret;
	*size -= ret;
}

static void update_pwrdown_flush_err_log(char *log_buff, int *size,
	unsigned long code)
{
	int ret;
	if (!(g_ufs_dsm_adaptor.err_type & (1 << UFS_POWER_DOWN_FLUSH_ERR))) {
		pr_err("UFS DSM Error! Err Num: %lu, %lu\n",
			code, g_ufs_dsm_adaptor.err_type);
		return;
	}
	if (*size < DSM_LOG_BUFF_LEFT) {
		pr_err("dsm log buff left space too small to load log.\n");
		return;
	}
	ret = snprintf(log_buff, *size,
		"UFS power down: No flush sent!\n");
	log_buff += ret;
	*size -= ret;
}

static void dsm_ufs_host_reset_history_cmd(struct ufs_hba *hba,
	char *dsm_log_buff, unsigned int *buff_size)
{
	int ret;
	int pos;
	int loop_cnt = 0;
	struct ufs_reset_info *ufs_reset_dmd = g_ufs_dsm_log.ufs_reset_dmd;
	struct dsm_ufs_cmd_log *cmd_log = NULL;
	struct dsm_ufs_cmd_log_entry *entry = NULL;
	if (ufs_reset_dmd == NULL) {
		dev_err(hba->dev,"ufs_dmd is NULL\n");
		return;
	}
	cmd_log = ufs_reset_dmd->cmd_log;
	if (cmd_log == NULL)
		return;
	if (!(g_ufs_dsm_adaptor.err_type & (1 << UFS_HARDWARE_RESET))) {
		pr_err("UFS DSM Error! Err Num: %lu, %lu\n",
			UFS_HARDWARE_RESET, g_ufs_dsm_adaptor.err_type);
		return;
	}
	if (ufs_reset_dmd->full_reset_flag) {
		ret = snprintf_s(dsm_log_buff, *buff_size, *buff_size-1, "FR;");
		dsm_log_buff += ret;
		*buff_size -= ret;
	}
	pos = cmd_log->pos;
	while(loop_cnt < hba->nutrs && pos < hba->nutrs) {
		loop_cnt++;
		entry = &cmd_log->cmd_log_entry[pos];
		pos = ++pos >= hba->nutrs ? 0 : pos;
		if (!entry->cmd_op)
			continue;

		dev_err(hba->dev, "[%d], cmd_op 0x%x, lba %llu, tag %u, "
			"trans_len %d, DPO %u, FUA %u, issue_time %llu,"
			"complete_time delta %llu\n", loop_cnt, entry->cmd_op,
			entry->lba, entry->tag, entry->transfer_len,
			entry->dpo, entry->fua, entry->issue_time,
			ktime_us_delta(entry->complete_time, entry->issue_time));

		ret = snprintf_s(dsm_log_buff, *buff_size, *buff_size-1,
			"%d,0x%x,%llu,%u,%d,%u,%u,%llu,%llu;",
			loop_cnt, entry->cmd_op, entry->lba, entry->tag,
			entry->transfer_len, entry->dpo, entry->fua, entry->issue_time,
			ktime_us_delta(entry->complete_time, entry->issue_time));
		if (ret < 0)
			break;

		dsm_log_buff += ret;
		*buff_size -= ret;
	}
	return;
}

static int dsm_ufs_update_host_reset_info(struct ufs_hba *hba, int err_code)
{
	if (!dsm_ufs_enable || err_code != DSM_UFS_HARDWARE_RESET)
		return -1;

	if (test_and_set_bit_lock(UFS_HARDWARE_RESET,
		&g_ufs_dsm_adaptor.err_type)) {
		dev_err(hba->dev, "dsm host_reset bit already be set.\n");
		return -1;
	}

	return 0;
}

void dsm_ufs_report_host_reset(struct ufs_hba *hba)
{
	int ret;
	ret = dsm_ufs_update_host_reset_info(hba, DSM_UFS_HARDWARE_RESET);
	if (ret)
		return;
	schedule_ufs_dsm_work(hba);
	return;
}


static void dsm_ufs_restore_cmd_init(struct ufs_hba *hba)
{
	struct device *dev = hba->dev;
	struct ufs_reset_info *ufs_reset_dmd = g_ufs_dsm_log.ufs_reset_dmd;
	ufs_reset_dmd->cmd_log = devm_kzalloc(dev,
		sizeof(struct dsm_ufs_cmd_log), GFP_KERNEL);
	if (ufs_reset_dmd->cmd_log == NULL) {
		dev_err(hba->dev,"alloc dsm_cmd_log fail in %s\n", __func__);
		return;
	}

	ufs_reset_dmd->cmd_log->cmd_log_entry = devm_kzalloc(dev,
	sizeof(struct dsm_ufs_cmd_log_entry) * hba->nutrs, GFP_KERNEL);
	if (ufs_reset_dmd->cmd_log->cmd_log_entry == NULL) {
		dev_err(hba->dev,"alloc cmd_log_entry fail in %s\n", __func__);
		devm_kfree(dev, ufs_reset_dmd->cmd_log);
		ufs_reset_dmd->cmd_log = NULL;
	}
	return;
}

void dsm_ufs_restore_cmd(struct ufs_hba *hba, unsigned int tag)
{
	struct ufshcd_lrb *lrbp = &hba->lrb[tag];
	struct scsi_cmnd *cmd = lrbp->cmd;
	struct ufs_reset_info *ufs_reset_dmd = g_ufs_dsm_log.ufs_reset_dmd;
	struct dsm_ufs_cmd_log *cmd_log = NULL;
	struct dsm_ufs_cmd_log_entry *log_entry = NULL;
	if (!cmd || !ufs_reset_dmd || !ufs_reset_dmd->cmd_log)
		return;

	cmd_log = ufs_reset_dmd->cmd_log;
	cmd_log->pos = ++cmd_log->pos >= hba->nutrs ? 0 : cmd_log->pos;

	log_entry = &cmd_log->cmd_log_entry[cmd_log->pos];
	log_entry->cmd_op = cmd->cmnd[0];
	log_entry->dpo = (cmd->cmnd[1] >> DPO_SHIFT) & 0x01;
	log_entry->fua = (cmd->cmnd[1] >> FUA_SHIFT) & 0x01;
	log_entry->lun = lrbp->lun;
	log_entry->tag = tag;
	log_entry->issue_time = lrbp->issue_time_stamp;
	log_entry->complete_time = ktime_get();

	if (!cmd->request || !cmd->request->bio) {
		log_entry->lba = 0;
		log_entry->transfer_len = 0;
		return;
	}

	log_entry->lba = cmd->request->bio->bi_iter.bi_sector;
	log_entry->transfer_len =
		be32_to_cpu(lrbp->ucd_req_ptr->sc.exp_data_transfer_len);
	return;
}

static int update_log_buff(struct ufs_hba *hba, char *log_buff,
	int *size, unsigned long code)
{
	switch (code) {
	case DSM_UFS_FASTBOOT_PWMODE_ERR:
		update_fastboot_pwmode_err_log(log_buff, size, code);
		break;
	case DSM_UFS_FASTBOOT_RW_ERR:
		update_fastboot_rw_err_log(log_buff, size, code);
		break;
	case DSM_UFS_UIC_TRANS_ERR:
		/* fall through */
	case DSM_UFS_LINE_RESET_ERR:
		/* fall through */
	case DSM_UFS_PA_INIT_ERR:
		update_uic_trans_line_reset_err_log(log_buff, size, code);
		break;
	case DSM_UFS_UIC_CMD_ERR:
		update_uic_cmd_err_log(log_buff, size, code);
		break;
	case DSM_UFS_CONTROLLER_ERR:
		/* fall through */
	case DSM_UFS_DEV_ERR:
		/* fall through */
	case DSM_UFS_ENTER_OR_EXIT_H8_ERR:
		/* fall through */
	case DSM_UFS_INLINE_CRYPTO_ERR:
		update_common_err_log(log_buff, size, code);
		break;
	case DSM_UFS_DEVICE_FATAL_ERROR:
		update_device_fatal_err_log(log_buff, size, code);
		break;
	case DSM_UFS_UTP_ERR:
		update_utp_err_log(log_buff, size, code);
		break;
	case DSM_UFS_TIMEOUT_ERR:
		update_timeout_err_log(log_buff, size, code);
		break;
	case DSM_UFS_HARDWARE_RESET:
		/* This DMD is temporarily used to record ufs host reset. */
		dsm_ufs_host_reset_history_cmd(hba, log_buff, size);
		break;
	case DSM_UFS_SCSI_CMD_ERR:
		/* fall through */
	case DSM_UFS_MEDIUM_ERR:
		/* fall through */
	case DSM_UFS_HARDWARE_ERR:
		update_hardware_common_err_log(log_buff, size, code);
		break;
	case DSM_UFS_VOLT_GPIO_ERR:
		update_vplt_gpio_err_log(log_buff, size, code);
		dev_err(hba->dev, "%s: UFS BUCK got a falling Volt \n", __func__);
		break;
	case DSM_UFS_LINKUP_ERR:
		update_linkup_err_log(hba, log_buff, size, code);
		break;
	case DSM_UFS_LIFETIME_EXCCED_ERR:
		update_lifetime_exceed_err_log(log_buff, size, code);
		break;
	case DSM_UFS_TEMP_LOW_ERR:
		update_temp_low_err_log(log_buff, size, code);
		break;
	case DSM_UFS_POWER_DOWN_FLUSH_ERR:
		update_pwrdown_flush_err_log(log_buff, size, code);
		break;
#if defined(CONFIG_SCSI_UFS_SCORPIO) ||                         \
	defined(CONFIG_SCSI_UFS_SGR)
	case DSM_UFS_MPHY_CALIBRATION_INFO:
		update_device_reg_log(hba, log_buff, size, code);
		break;
#endif
#ifdef CONFIG_SCSI_UFS_GEAR_CTRL
	case DSM_UFS_DYNAMIC_SWITCH_SPEED_ERR:
		update_dynamic_switch_speed_switch_err_log(hba, log_buff, size, code);
		break;
#endif
	default:
		pr_err("unknown error code: %lu\n", code);
		return -1;
	}
	return 0;
}

int dsm_ufs_get_log(struct ufs_hba *hba, unsigned long code, char *err_msg)
{
	int ret, buff_size;
	char *dsm_log_buff = NULL;
	unsigned int j;
	buff_size = sizeof(g_ufs_dsm_log.dsm_log);
	dsm_log_buff = g_ufs_dsm_log.dsm_log;
	if (!dsm_ufs_enable)
		return 0;
	g_ufs_dsm_adaptor.manufacturer_id = hba->manufacturer_id;
	memset(g_ufs_dsm_log.dsm_log, 0, buff_size);

	ret = snprintf(dsm_log_buff, buff_size,
		"0x%04x,",
		g_ufs_dsm_adaptor.manufacturer_id);
	dsm_log_buff += ret;
	buff_size -= ret;
	update_dsm_log_module_name(hba, &dsm_log_buff, &buff_size);

	ret = snprintf(dsm_log_buff, buff_size,
		"%04x,",
		hba->manufacturer_date);
	dsm_log_buff += ret;
	buff_size -= ret;
#ifndef DSM_FOR_FACTORY_MODE
	for (j = 0; j < strlen(g_ufs_dsm_log.dsm_log); j++)
		g_ufs_dsm_log.dsm_log[j] -= 1;
#endif
	ret = snprintf(dsm_log_buff, buff_size,
		"\nError Num:%lu, %s\n",
		code, err_msg);
	dsm_log_buff += ret;
	buff_size -= ret;

	/* print vendor info */
	ret = update_log_buff(hba, dsm_log_buff, &buff_size, code);
	if (ret) {
		pr_err("unknown error code: %lu\n", code);
		return 0;
	}
	if (buff_size <= 0) {
		ufs_dsm_real_upload_size = sizeof(g_ufs_dsm_log.dsm_log) - 1;
		pr_err("dsm log buff overflow!\n");
	} else {
		ufs_dsm_real_upload_size = sizeof(g_ufs_dsm_log.dsm_log) - (unsigned int)buff_size;
	}

	pr_err("dsm_log:\n %s\n", g_ufs_dsm_log.dsm_log);
	pr_err("buff_size = %d\n", buff_size);
#ifdef HEALTH_INFO_FACTORY_MODE
	if (code == DSM_UFS_HARDWARE_RESET)
		return 0; // not report DSM_UFS_HARDWARE_RESET red screen in factory mode
#endif
	return 1;
}

int dsm_ufs_enabled(void)
{
	return dsm_ufs_enable;
}

void dsm_ufs_flag_enable(void)
{
	dsm_ufs_enable = 1;
}

void dsm_ufs_flag_disable(void)
{
	dsm_ufs_enable = 0;
}

/* lint -e648 -e845 */
#if defined(CONFIG_RPMB_UFS)
int get_rpmb_key_status(void);
#endif
int dsm_ufs_utp_err_need_report(struct ufs_hba *hba)
{
	int key_status = 0;
	if (unlikely(hba->host->is_emulator))
		return 0;
#if defined(CONFIG_RPMB_UFS)
	key_status = get_rpmb_key_status();
#endif
	if ((key_status == 0) && (g_ufs_dsm_adaptor.ocs == OCS_FATAL_ERROR))
		return 0;
	return 1;
}

void dsm_ufs_enable_volt_irq(struct ufs_hba *hba)
{
	if (hba->volt_irq < 0)
		return;
	else
		enable_irq(hba->volt_irq);
}

void dsm_ufs_disable_volt_irq(struct ufs_hba *hba)
{
	if (hba->volt_irq < 0)
		return;
	else
		disable_irq(hba->volt_irq);
}

int dsm_ufs_uic_cmd_err_need_report(void)
{
	u32 uic_cmd = g_ufs_dsm_adaptor.uic_info[0];
	if ((uic_cmd == UIC_CMD_DME_HIBER_ENTER) ||
		(uic_cmd == UIC_CMD_DME_HIBER_EXIT))
		return 0;
	return 1;
}

void print_uic_err_history(void)
{
	int i;
	for (i = 0; i < UIC_ERR_HISTORY_LEN; i++)
		pr_info("transfered_bits[%d] = %lu (10^10)\n", i,
		uic_err_history.transfered_bits[i]);
	pr_info("Pos: %u, delta_err_cnt: %lu, delta_bit_cnt: %lu\n",
		uic_err_history.pos,
		uic_err_history.delta_err_cnt,
		uic_err_history.delta_bit_cnt);
}

int dsm_ufs_device_fatal_err_need_report(struct ufs_hba *hba)
{
	g_device_fatal_err_cnt++;
	if (g_device_fatal_err_cnt >= DSM_DEVICE_FATAL_ERR_THRESHOLD)
		return 1;
	return 0;
}

int dsm_ufs_uic_err_need_report(struct ufs_hba *hba)
{
	u32 ov_flag = 0;
	u32 counter = 0;
	static unsigned long uic_err_cnt = 0;
	static unsigned long tmp_bits = 0;
	static unsigned long ten_billon_bits_cnt = 0;

	uic_err_cnt++;
	ufshcd_dme_get(hba, UIC_ARG_MIB((u32)VS_DEBUG_COUNTER_OVERFLOW), &ov_flag); /* get ov flag */
	/* get synbol cnt /1024 */
	ufshcd_dme_get(hba, UIC_ARG_MIB((u32)VS_DEBUG_COUNTER0), &counter);
	/* reset cnter0 and enable it */
#ifdef CONFIG_HUFS_HC
	ufshcd_dme_set(hba, UIC_ARG_MIB((u32)DEBUGCOUNTER_CLR), BIT_DBG_CNT0_CLR);
	ufshcd_dme_set(hba, UIC_ARG_MIB((u32)DEBUGCOUNTER_EN), BIT_DBG_CNT0_EN);
#else
	ufshcd_dme_set(hba, UIC_ARG_MIB((u32)VS_DEBUG_COUNTER_CONTROL), 0x5);
#endif
	if (ov_flag & 0x1)
		tmp_bits += (1UL << SHIFT_32) * BITS_PER_CNT;
	tmp_bits += (unsigned long)counter * BITS_PER_CNT;
	ten_billon_bits_cnt +=  tmp_bits / TEN_BILLION_BITS;
	tmp_bits = tmp_bits % TEN_BILLION_BITS;
	uic_err_history.transfered_bits[uic_err_history.pos] = ten_billon_bits_cnt;
	uic_err_history.pos = (uic_err_history.pos + 1) % UIC_ERR_HISTORY_LEN;
	uic_err_history.delta_err_cnt = min(uic_err_cnt,
		(unsigned long)UIC_ERR_HISTORY_LEN);
	uic_err_history.delta_bit_cnt = ten_billon_bits_cnt -
		uic_err_history.transfered_bits[uic_err_history.pos];
	print_uic_err_history();
	pr_info("%s: errcnt = %lu, 10^10_cnt = %lu, tmpbits = %lu, "
		"ovflag = %u, counter = %u\n", __func__, uic_err_cnt,
		ten_billon_bits_cnt, tmp_bits, ov_flag & 0x1, counter);
	/* The UIC error threshold is exceeded, and either ov_flag or counter is not 0. */
	if ((ten_billon_bits_cnt >= MIN_BER_SAMPLE_BITS) &&
		(uic_err_history.delta_err_cnt > uic_err_history.delta_bit_cnt) &&
		(((ov_flag & 0x1) != 0) || (counter != 0)))
		return 1;
	pr_info("%s: UECPA:%08x,UECDL:%08x,UECN:%08x,UECT:%08x,UEDME:%08x\n",
		__func__, g_ufs_dsm_adaptor.uic_uecpa,
		g_ufs_dsm_adaptor.uic_uecdl, g_ufs_dsm_adaptor.uic_uecn,
		g_ufs_dsm_adaptor.uic_uect, g_ufs_dsm_adaptor.uic_uedme);
	pr_info("%s: UIC error cnt below threshold, do not report!\n", __func__);
	return 0;
}

void report_controller_fatal_error(struct ufs_hba *hba)
{
	if (g_ufs_dsm_adaptor.uic_uecdl & UIC_DATA_LINK_LAYER_ERROR_PA_INIT)
		dsm_ufs_log(hba, DSM_UFS_CONTROLLER_ERR, "UFS PA INIT ERROR");
	else
		dsm_ufs_log(hba, DSM_UFS_CONTROLLER_ERR, "UFS Controller Error");
}

static void dsm_ufs_err_handle(struct dsm_err_type_category ufs_err,
	struct ufs_hba *hba)
{
	if (g_ufs_dsm_adaptor.err_type & (unsigned long)(unsigned)(1 << ufs_err.err_bit)) {
		dsm_ufs_log(hba, ufs_err.err_no, ufs_err.err_desc); /*lint !e592*/
		clear_bit_unlock(ufs_err.err_bit, &g_ufs_dsm_adaptor.err_type);
	}
}

static void dsm_uic_trans_err_handle(struct ufs_hba *hba)
{
	unsigned long flags;
	if (g_ufs_dsm_adaptor.err_type & (1 << UFS_UIC_TRANS_ERR)) {
		if (dsm_ufs_uic_err_need_report(hba))
			dsm_ufs_log(hba, DSM_UFS_UIC_TRANS_ERR,
				"UIC No Fatal Error");
		clear_bit_unlock(UFS_UIC_TRANS_ERR, &g_ufs_dsm_adaptor.err_type);
		msleep(DELAY_MS_100);
		spin_lock_irqsave(hba->host->host_lock, flags);
		ufshcd_enable_intr(hba, UIC_ERROR);
		spin_unlock_irqrestore(hba->host->host_lock, flags);
	}
}

static void dsm_timeout_err_handle(struct ufs_hba *hba)
{
	if (g_ufs_dsm_adaptor.err_type & (1 << UFS_TIMEOUT_ERR)) {
		ufshcd_dme_get(hba, UIC_ARG_MIB(PA_PWRMODE),
			&g_ufs_dsm_adaptor.pwrmode);
		ufshcd_dme_get(hba, UIC_ARG_MIB(PA_TXGEAR),
			&g_ufs_dsm_adaptor.tx_gear);
		ufshcd_dme_get(hba, UIC_ARG_MIB(PA_RXGEAR),
			&g_ufs_dsm_adaptor.rx_gear);
		dsm_ufs_log(hba, DSM_UFS_TIMEOUT_ERR, "UFS TimeOut Error");
		clear_bit_unlock(UFS_TIMEOUT_ERR, &g_ufs_dsm_adaptor.err_type);
	}
}

static void dsm_uic_cmd_err_handle(struct ufs_hba *hba)
{
	if (g_ufs_dsm_adaptor.err_type & (1 << UFS_UIC_CMD_ERR)) {
		if (dsm_ufs_uic_cmd_err_need_report())
			dsm_ufs_log(hba, DSM_UFS_UIC_CMD_ERR, "UIC Cmd Error");
		clear_bit_unlock(UFS_UIC_CMD_ERR, &g_ufs_dsm_adaptor.err_type);
	}
}

static void dsm_ufs_device_fatal_err_handle(struct ufs_hba *hba)
{
	if (g_ufs_dsm_adaptor.err_type & (1 << UFS_DEVICE_FATAL_ERR)) {
		if (dsm_ufs_device_fatal_err_need_report(hba))
			dsm_ufs_log(hba, DSM_UFS_DEVICE_FATAL_ERROR,
				"OCS = 8, Ufs Device Fatal Error");
		clear_bit_unlock(UFS_DEVICE_FATAL_ERR, &g_ufs_dsm_adaptor.err_type);
	}
}

static void dsm_utp_trans_err_handle(struct ufs_hba *hba)
{
	if (g_ufs_dsm_adaptor.err_type & (1 << UFS_UTP_TRANS_ERR)) {
		if (dsm_ufs_utp_err_need_report(hba))
			dsm_ufs_log(hba, DSM_UFS_UTP_ERR, "UTP Transfer Error");
		clear_bit_unlock(UFS_UTP_TRANS_ERR, &g_ufs_dsm_adaptor.err_type);
	}
}

static void dsm_controller_err_handle(struct ufs_hba *hba)
{
	if (g_ufs_dsm_adaptor.err_type & (1 << UFS_CONTROLLER_ERR)) {
		report_controller_fatal_error(hba);
		clear_bit_unlock(UFS_CONTROLLER_ERR, &g_ufs_dsm_adaptor.err_type);
	}
}

static void dsm_pwrdownflush_err_handle(struct ufs_hba *hba)
{
	if (g_ufs_dsm_adaptor.err_type & (1 << UFS_POWER_DOWN_FLUSH_ERR)) {
		dsm_ufs_log(hba, DSM_UFS_POWER_DOWN_FLUSH_ERR, "Powerdown Flush Error");
		clear_bit_unlock(UFS_POWER_DOWN_FLUSH_ERR, &g_ufs_dsm_adaptor.err_type);
	}
}

void dsm_ufs_handle_work(struct work_struct *work)
{
	struct ufs_hba *hba;
	int len, i;
	hba = container_of(work, struct ufs_hba, dsm_work);
	mutex_lock(&hba->eh_mutex);
	len = sizeof(err_type_category) / sizeof(err_type_category[0]);
	for (i = 0; i < len; i++) {
		switch (err_type_category[i].err_bit) {
		case UFS_UIC_TRANS_ERR:
			dsm_uic_trans_err_handle(hba);
			break;
		case UFS_UIC_CMD_ERR:
			dsm_uic_cmd_err_handle(hba);
			break;
		case UFS_CONTROLLER_ERR:
			dsm_controller_err_handle(hba);
			break;
		case UFS_DEVICE_FATAL_ERR:
			dsm_ufs_device_fatal_err_handle(hba);
			break;
		case UFS_UTP_TRANS_ERR:
			dsm_utp_trans_err_handle(hba);
			break;
		case UFS_TIMEOUT_ERR:
			dsm_timeout_err_handle(hba);
			break;
		case UFS_POWER_DOWN_FLUSH_ERR:
			dsm_pwrdownflush_err_handle(hba);
			break;
		default:
			dsm_ufs_err_handle(err_type_category[i], hba);
			break;
		}
	}
	mutex_unlock(&hba->eh_mutex);
}

void schedule_ufs_dsm_work(struct ufs_hba *hba)
{
	if ((dsm_ufs_enabled()) && (dsm_ufs_if_err()))
		queue_work(system_freezable_wq, &hba->dsm_work);
}

static irqreturn_t ufs_gpio_irq_handle(int irq, void *__hba)
{
	struct ufs_hba *hba = __hba;
	dsm_ufs_update_error_info(hba, DSM_UFS_VOLT_GPIO_ERR);
	return IRQ_HANDLED;
}

static int ufs_volt_gpio_init(struct device *dev)
{
	int err;
	int irq;
	struct device_node *node =  dev->of_node;
	unsigned int gpio;
	gpio = of_get_named_gpio(node, "volt-irq,ufs-gpios", 0);
	if (!gpio_is_valid(gpio)) {
		pr_err("gpio is unvalid!\n");
		return -ENOENT;
	}
	err = gpio_request(gpio, "ufs_volt");
	if (err < 0) {
		pr_info("Can`t request UFS_VOLT gpio %d\n", gpio);
		return -EIO;
	}
	err = gpio_direction_input(gpio);
	if (err < 0)
		goto out_free;
	irq = gpio_to_irq(gpio);
	if (irq < 0) {
		pr_info("Could not set UFS_VOLT_GPIO irq = %d!, err = %d\n",
				gpio, irq);
		goto out_free;
	}
	err = request_irq((unsigned int)irq,
		ufs_gpio_irq_handle, (unsigned long)IRQF_TRIGGER_FALLING,
		"ufs_volt", NULL);
	if (err) {
		free_irq(gpio, NULL);
		pr_info("Request ufs volt gpio irq failed.\n");
		goto out_free;
	}
	return irq;
out_free:
	gpio_free(gpio);
	return -EIO;
}

void dsm_ufs_init(struct ufs_hba *hba)
{
	int ufs_volt_irq;
	struct device *dev = hba->dev;
	memset((void*)&uic_err_history, 0, sizeof(uic_err_history));
	ufs_volt_irq = ufs_volt_gpio_init(dev);
	if (ufs_volt_irq < 0) {
		hba->volt_irq = -1;
		pr_err("ufs volt gpio irq failed.\n");
	} else {
		hba->volt_irq = ufs_volt_irq;
	}
	ufs_dclient = dsm_register_client(&dsm_ufs);
	if (!ufs_dclient) {
		pr_err("ufs dclient init error");
		return;
	}
	g_ufs_dsm_log.ufs_reset_dmd = kzalloc(sizeof(struct ufs_reset_info), GFP_KERNEL);
	if (!g_ufs_dsm_log.ufs_reset_dmd) {
		dev_err(hba->dev,"alloc g_ufs_dsm_log.ufs_reset_dmd fail in %s\n", __func__);
		return;
	}
	dsm_ufs_restore_cmd_init(hba);
	INIT_WORK(&hba->dsm_work, dsm_ufs_handle_work);
	mutex_init(&g_ufs_dsm_log.lock);
	dsm_ufs_enable = 1;
	g_device_fatal_err_cnt = 0;
	async_schedule(dsm_ufs_fastboot_async, hba);
}
/* lint +e747 +e713 +e715 +e732 +e835 */
