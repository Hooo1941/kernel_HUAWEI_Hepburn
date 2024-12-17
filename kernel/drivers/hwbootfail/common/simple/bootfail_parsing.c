

/* ---- includes ---- */
#include <linux/rtc.h>
#include <linux/vmalloc.h>
#include <linux/delay.h>
#include <linux/kernel.h>
#include <crypto/hash.h>
#include <crypto/sha.h>

#include <hwbootfail/core/boot_detector.h>
#include <hwbootfail/chipsets/common/bootfail_common.h>
#include <hwbootfail/chipsets/common/bootfail_chipsets.h>

#include <linux/blackbox.h>
#include <linux/blackbox_common.h>
#include <linux/blackbox_storage.h>
#include <securec.h>

#ifndef BF_SIZE_2K
#define BF_SIZE_2K (BF_SIZE_1K * 2)
#endif
#define RECORDS_COUNT 40

static const int time_zone = 28800; // for there is 8 hours between us and standard time
static const int recovery_success = 1;

static char error_name_bl1_core_dev_fault[] = "BL1 core device fault";
static char error_name_bl1_non_core_dev_fault[] = "BL1 non core device fault";
static char error_name_bl1_img_fault[] = "BL1 image damaged or verification failed";
static char error_name_bl1_system_crash[] = "BL1 crash";
static char error_name_bl1_system_freeze[] = "BL1 freeze";
static char error_name_bl1_bad_env[] = "BL1 bad working env";
static char error_name_bl2_core_dev_fault[] = "BL2 core device fault";
static char error_name_bl2_non_core_dev_fault[] = "BL2 non core device fault";
static char error_name_bl2_img_fault[] = "BL2 image damaged or verification failed";
static char error_name_bl2_system_crash[] = "BL2 crash";
static char error_name_bl2_system_freeze[] = "BL2 freeze";
static char error_name_bl2_bad_env[] = "BL2 bad working env";
static char error_name_kernel_core_dev_fault[] = "Kernel core device fault";
static char error_name_kernel_non_core_dev_fault[] = "Kernel non core device fault";
static char error_name_kernel_img_fault[] = "Kernel image damaged or verification failed";
static char error_name_kernel_subsys_fault[] = "Kernel susystem fault";
static char error_name_kernel_system_crash[] = "Kernel crash";
static char error_name_kernel_system_freeze[] = "Kernel freeze";
static char error_name_kernel_bad_env[] = "Kernel bad working env";
static char error_name_native_core_dev_fault[] = "Native core device fault";
static char error_name_native_non_core_dev_fault[] = "Native non core device fault";
static char error_name_native_service_crash[] = "Native critical service crash";
static char error_name_native_service_freeze[] = "Native critical service freeze";
static char error_name_native_data_part_mount_fail[] = "Native /data mount failed";
static char error_name_native_data_part_mount_ro[] = "Native /data mounted RO";
static char error_name_native_data_damaged[] = "Native critical data damaged";
static char error_name_native_data_mount_fail[] = "Native critical part mount failed";
static char error_name_native_subsys_fault[] = "Native subsystem fault";
static char error_name_native_system_crash[] = "Native crash";
static char error_name_native_system_freeze[] = "Native freeze";
static char error_name_native_bad_env[] = "Native bad working env";
static char error_name_framework_core_dev_fault[] = "Framework core device fault";
static char error_name_framework_non_core_dev_fault[] = "Framework non core device fault";
static char error_name_framework_service_crash[] = "Framework critical service crash";
static char error_name_framework_service_freeze[] = "Framework critical service freeze";
static char error_name_framework_data_damaged[] = "Framework critical data damaged";
static char error_name_framework_subsys_fault[] = "Framework subsystem fault";
static char error_name_framework_system_crash[] = "Framework crash";
static char error_name_framework_system_freeze[] = "Framework freeze";
static char error_name_framework_bad_env[] = "Framework bad working env";

static char invalid_boot_stage[] = "INVALID STAGE";
static char invalid_errno[] = "INVALID_ERRNO";
struct bootfail_log_content {
	int bl_log_size;
	int kernel_log_size;
	int bf_info_start;
	char bf_info_content[BF_SIZE_2K];
	char bl_log_content[BF_SIZE_128K];
	char kernel_log_content[BF_SIZE_512K];
};
boot_stage_map_st boot_stage_map[] = {
	{BL1_STAGE, "BL1_STAGE"},
	{BL2_STAGE, "BL2_STAGE"},
	{KERNEL_STAGE, "KERNEL_STAGE"},
	{NATIVE_STAGE, "NATIVE_STAGE"},
	{FRAMEWORK_STAGE, "FRAMEWORK_STAGE"},
};
error_no_map_st error_no_map[] = {
	{BL1_CORE_DEV_FAULT, error_name_bl1_core_dev_fault},
	{BL1_NON_CORE_DEV_FAULT, error_name_bl1_non_core_dev_fault},
	{BL1_IMG_FAULT, error_name_bl1_img_fault},
	{BL1_SYSTEM_CRASH, error_name_bl1_system_crash},
	{BL1_SYSTEM_FREEZE, error_name_bl1_system_freeze},
	{BL1_BAD_ENV, error_name_bl1_bad_env},
	{BL2_CORE_DEV_FAULT, error_name_bl2_core_dev_fault},
	{BL2_NON_CORE_DEV_FAULT, error_name_bl2_non_core_dev_fault},
	{BL2_IMG_FAULT, error_name_bl2_img_fault},
	{BL2_SYSTEM_CRASH, error_name_bl2_system_crash},
	{BL2_SYSTEM_FREEZE, error_name_bl2_system_freeze},
	{BL2_BAD_ENV, error_name_bl2_bad_env},
	{KERNEL_CORE_DEV_FAULT, error_name_kernel_core_dev_fault},
	{KERNEL_NON_CORE_DEV_FAULT, error_name_kernel_non_core_dev_fault},
	{KERNEL_IMG_FAULT, error_name_kernel_img_fault},
	{KERNEL_SUBSYS_FAULT, error_name_kernel_subsys_fault},
	{KERNEL_SYSTEM_CRASH, error_name_kernel_system_crash},
	{KERNEL_SYSTEM_FREEZE, error_name_kernel_system_freeze},
	{KERNEL_BAD_ENV, error_name_kernel_bad_env},
	{NATIVE_CORE_DEV_FAULT, error_name_native_core_dev_fault},
	{NATIVE_NON_CORE_DEV_FAULT, error_name_native_non_core_dev_fault},
	{NATIVE_SERVICE_CRASH, error_name_native_service_crash},
	{NATIVE_SERVICE_FREEZE, error_name_native_service_freeze},
	{NATIVE_DATA_PART_MOUNT_FAIL, error_name_native_data_part_mount_fail},
	{NATIVE_DATA_PART_MOUNT_RO, error_name_native_data_part_mount_ro},
	{NATIVE_DATA_DAMAGED, error_name_native_data_damaged},
	{NATIVE_PART_MOUNT_FAIL, error_name_native_data_mount_fail},
	{NATIVE_SUBSYS_FAULT, error_name_native_subsys_fault},
	{NATIVE_SYSTEM_CRASH, error_name_native_system_crash},
	{NATIVE_SYSTEM_FREEZE, error_name_native_system_freeze},
	{NATIVE_BAD_ENV, error_name_native_bad_env},
	{FRAMEWORK_CORE_DEV_FAULT, error_name_framework_core_dev_fault},
	{FRAMEWORK_NON_CORE_DEV_FAULT, error_name_framework_non_core_dev_fault},
	{FRAMEWORK_SERVICE_CRASH, error_name_framework_service_crash},
	{FRAMEWORK_SERVICE_FREEZE, error_name_framework_service_freeze},
	{FRAMEWORK_DATA_DAMAGED, error_name_framework_data_damaged},
	{FRAMEWORK_SUBSYS_FAULT, error_name_framework_subsys_fault},
	{FRAMEWORK_SYSTEM_CRASH, error_name_framework_system_crash},
	{FRAMEWORK_SYSTEM_FREEZE, error_name_framework_system_freeze},
	{FRAMEWORK_BAD_ENV, error_name_framework_bad_env},
};

static int sha256_check(char *data, unsigned int len, char hash[SHA256_DIGEST_SIZE])
{
	int ret = 0;
	struct crypto_shash *tfm;
	size_t desc_size;
	struct shash_desc *desc;

	tfm = crypto_alloc_shash("sha256", 0, 0);
	if (IS_ERR(tfm)) {
		ret = PTR_ERR(tfm);
		goto out;
	}

	desc_size = crypto_shash_descsize(tfm) + sizeof(struct shash_desc);
	desc = kzalloc(desc_size, GFP_KERNEL);
	if (!desc) {
		ret = -ENOMEM;
		goto out_free_tfm;
	}
	desc->tfm = tfm;

	ret = crypto_shash_init(desc);
	if (ret < 0)
		goto out_free_desc;

	ret = crypto_shash_update(desc, data, len);
	if (ret)
		goto out_free_desc;

	ret = crypto_shash_final(desc, hash);

out_free_desc:
	kfree(desc);
out_free_tfm:
	crypto_free_shash(tfm);
out:
	return ret;
}

static bool get_meta_log(struct bootfail_meta_log *bf_meta_log)
{
	int ret = 0;
	char hash[SHA256_DIGEST_SIZE];
	memset_s(bf_meta_log, sizeof(struct bootfail_meta_log), 0, sizeof(struct bootfail_meta_log));
	memset_s(hash, SHA256_DIGEST_SIZE, 0, SHA256_DIGEST_SIZE);

	ret = read_part(get_bfi_dev_path(), sizeof(union bfi_part_header) + sizeof(struct bootfail_proc_param),
				(char *)bf_meta_log, sizeof(struct bootfail_meta_log));
	if (ret != 0) {
		print_err("read rrecord part failed!\n");
		return false;
	}

	ret = sha256_check((char *)bf_meta_log + sizeof(bf_meta_log->sha256),
					   sizeof(struct bootfail_meta_log) - sizeof(bf_meta_log->sha256),
					   hash);
	if (ret != 0) {
		print_err("sha256_check failed!\n");
		return false;
	}

	if (memcmp(hash, bf_meta_log->sha256, SHA256_DIGEST_SIZE) != 0) {
		print_err("sha256 failed!\n");
		return false;
	}
	return true;
}

static int read_content(char *content, unsigned int start, unsigned int size)
{
	int ret;

	ret = read_part(get_bfi_dev_path(), start, (char *)content, size);
	if (ret != 0) {
		print_err("read log from record partition failed!");
		return -1;
	}
	return 0;
}

static bool write_content(char *content, unsigned int start, unsigned int size)
{
	int ret = write_part(get_bfi_dev_path(), start, (char *)content, size);
	if (ret != 0) {
		print_err("write log to record partition failed!");
		return false;
	}
	return true;
}

static void clear_recovery_info(struct bootfail_meta_log *bf_meta_log)
{
	int offset = bf_meta_log->rrecord_offset;
	int record_bak_addr = bf_meta_log->rrecord_offset + RECORDS_COUNT * sizeof(struct rrecord) + sizeof(struct rrecord_header);
	struct rrecord_header rheader;
	struct rrecord *records;
	memset_s(&rheader, sizeof(struct rrecord_header), 0, sizeof(struct rrecord_header));

	records = (struct rrecord *)vmalloc(sizeof(struct rrecord) * RECORDS_COUNT);
	if (records == NULL) {
		print_err("failed to alloc size for rrecord, exit\n");
		return;
	}

	memset_s(records, sizeof(struct rrecord) * RECORDS_COUNT, 0, sizeof(struct rrecord) * RECORDS_COUNT);

	bool ret = write_content((char *)(&rheader), bf_meta_log->rrecord_offset, sizeof(struct rrecord_header));
	if (!ret) {
		print_err("clear rrecord header failed");
		write_content((char *)(&rheader), bf_meta_log->rrecord_offset, sizeof(rheader.sha256));
	}

	ret = write_content((char *)records, bf_meta_log->rrecord_offset + sizeof(struct rrecord_header), sizeof(struct rrecord) * RECORDS_COUNT);
	if (!ret)
		print_err("clear rrecord  failed ");

	ret = write_content((char *)(&rheader), record_bak_addr, sizeof(struct rrecord_header));
	if (!ret) {
		print_err("clear rrecord bak header failed");
		write_content((char *)(&rheader), record_bak_addr, sizeof(rheader.sha256));
	}

	ret = write_content((char *)records, record_bak_addr + sizeof(struct rrecord_header), sizeof(struct rrecord) * RECORDS_COUNT);
	if (!ret)
		print_err("clear rrecord bak failed ");

	vfree(records);
}

static void update_rrecord_part(struct bootfail_meta_log *bf_meta_log)
{
	clear_recovery_info(bf_meta_log);
	bf_meta_log->log_count = 0;
	int ret = sha256_check((char *)(bf_meta_log) + sizeof(bf_meta_log->sha256),
		sizeof(struct bootfail_meta_log) - sizeof(bf_meta_log->sha256),
		(char *)(bf_meta_log));
	if (ret != 0) {
		print_err("sha256_check failed!\n");
		memset_s(bf_meta_log, sizeof(struct bootfail_meta_log), 0, sizeof(struct bootfail_meta_log));
	}
	write_content((char *)(bf_meta_log), sizeof(union bfi_part_header) +
				sizeof(struct bootfail_proc_param), sizeof(struct bootfail_meta_log));
}

static void translstate_stage(char *stage, int boot_stage)
{
	for (unsigned int index = 0; index < ARRAY_SIZE(boot_stage_map); index++) {
		if (boot_stage_map[index].stage == boot_stage) {
			strncpy_s(stage, BF_SIZE_128 - 1, boot_stage_map[index].stage_dec, strlen(boot_stage_map[index].stage_dec));
			return;
		}
	}
}

static void translstate_errno(char *errnum, int err_no)
{
	for (unsigned int index = 0; index < ARRAY_SIZE(error_no_map); index++) {
		if (error_no_map[index].error == err_no) {
			strncpy_s(errnum, BF_SIZE_128 - 1, error_no_map[index].err_dec, strlen(error_no_map[index].err_dec));
			return;
		}
	}
}

static void save_boofail_info(char *bf_content, struct bootfail_log_header *bf_bootfail_info, struct error_info *info)
{
	char stage[BF_SIZE_128];
	char errnum[BF_SIZE_128];
	struct rtc_time real_time;
	int res = 0;

	memset_s(stage, BF_SIZE_128, 0, BF_SIZE_128);
	memset_s(errnum, BF_SIZE_128, 0, BF_SIZE_128);

	translstate_stage(stage, bf_bootfail_info->binfo.stage);
	if (strlen(stage) == 0) {
		print_err("save_bootfail_info: translate stage failed!\n");
		res = strncpy_s(stage, BF_SIZE_128 - 1, invalid_boot_stage, strlen(invalid_boot_stage));
		if (res)
			print_err("copy boot stage failed!\n");
	}

	translstate_errno(errnum, bf_bootfail_info->binfo.bootfail_errno);
	if (strlen(errnum) == 0) {
		print_err("save_bootfail_info: translate errno failed!\n");
		res = strncpy_s(errnum, BF_SIZE_128 - 1, invalid_errno, strlen(invalid_errno));
		if (res)
			print_err("copy errno failed!\n");
	}

	strncpy_s(info->top_category, CATEGORY_MAX_LEN - 1, stage, strlen(stage));
	strncpy_s(info->error_desc, ERROR_DESC_MAX_LEN - 1, errnum, strlen(errnum));

	rtc_time64_to_tm(bf_bootfail_info->binfo.rtc_time + time_zone, &real_time);
	scnprintf((char *)bf_content, BF_SIZE_2K - 1,
				"                   Generated by HiviewDFX@HarmonyOS               \n"
				"==========================Parsed data=============================\n"
				"Time of Error Occoured: %d/%02d/%02d-%02d:%02d:%02d\n"
				"Boot Stage when Error Occoured: %s\nFault Reason: %s\n"
				"Boot Up Time: %d seconds\nDMD Error No: %d\nIs Root: %s\n"
				"Space Left On Device: %uMB\nInodes Left On Device: %u\n"
				"Error Description: %s\n"
				"\n=======================Original data=============================\n"
				"Error No: 0x%x\nBoot Stage: 0x%x\nException Type: %d\n"
				"Sub Exception Type: %d\nSuggest Recovery Method: %d\n"
				"Exception Stage: %d\n",
				real_time.tm_year + 1900, real_time.tm_mon + 1, real_time.tm_mday, real_time.tm_hour,
				real_time.tm_min, real_time.tm_sec, stage, errnum, bf_bootfail_info->binfo.bootup_time,
				bf_bootfail_info->binfo.dmd_errno, bf_bootfail_info->binfo.is_rooted == 0 ? "False" : "True",
				bf_bootfail_info->space_left, bf_bootfail_info->inodes_left, bf_bootfail_info->detail_info,
				bf_bootfail_info->binfo.bootfail_errno, bf_bootfail_info->binfo.stage,
				bf_bootfail_info->binfo.exception_type, bf_bootfail_info->binfo.sub_exception_type,
				bf_bootfail_info->binfo.suggest_recovery_method, bf_bootfail_info->binfo.exception_stage);

	scnprintf(info->error_time, TIMESTAMP_MAX_LEN, "%04d%02d%02d%02d%02d%02d-%08d",
			  real_time.tm_year + 1900, real_time.tm_mon + 1, real_time.tm_mday, real_time.tm_hour,
			  real_time.tm_min, real_time.tm_sec, bf_bootfail_info->binfo.bootup_time);
}

static int write_file(const char *log_path, char *content, unsigned int size)
{
	int ret;
	struct file *fp = NULL;
	mm_segment_t fs;

	fp = filp_open(log_path, O_CREAT | O_RDWR, 0664);
	if (IS_ERR_OR_NULL(fp)) {
		print_err("BootDetector bbox_dump filp_open failed!\n");
		return -1;
	}

	fs = get_fs();
	set_fs(KERNEL_DS);

	ret = vfs_write(fp, content, size, &fp->f_pos);
	if (ret < 0)
		print_err("BootDetector bbox_dump write is error res %d!\n", ret);

	set_fs(fs);
	filp_close(fp, NULL);
	return ret;
}

static int save_log(const char *path, char *content, unsigned int start, unsigned int size)
{
	int ret = 0;
	ret = read_content(content, start, size);
	if (ret != 0) {
		print_err("read content failed !\n");
		return ret;
	}
	ret = write_file(path, content, size);
	if (ret < 0) {
		print_err("save_log: write file failed!\n");
		return ret;
	}
	return ret;
}

static void get_file_dir(struct error_info *info, char *bf_info_path, char *bl_log_path, char *kernel_log_path, unsigned int index)
{
	const char *bf_info = "bootfail_info";
	const char *last_kmsg = "last_kmsg";
	const char *bl_log = "bl_log";
	char log_dir[BF_SIZE_128];

	memset_s(log_dir, BF_SIZE_128, 0, BF_SIZE_128);
	memset_s(bf_info_path, BF_SIZE_128, 0, BF_SIZE_128);
	memset_s(bl_log_path, BF_SIZE_128, 0, BF_SIZE_128);
	memset_s(kernel_log_path, BF_SIZE_128, 0, BF_SIZE_128);

	scnprintf(log_dir, BF_SIZE_128 - 1, "%s/%s", CONFIG_BLACKBOX_LOG_ROOT_PATH, info->error_time);
	print_err("get_file_dir: log_dir! %s\n", log_dir);
	create_log_dir(log_dir);
	scnprintf(bf_info_path, BF_SIZE_128 - 1, "%s/%s_%u", log_dir, bf_info, index);
	scnprintf(bl_log_path, BF_SIZE_128 - 1, "%s/%s_%u", log_dir, bl_log, index);
	scnprintf(kernel_log_path, BF_SIZE_128 - 1, "%s/%s_%u", log_dir, last_kmsg, index);
	print_err("get_file_dir: bf_info_path! %s\n", bf_info_path);
	print_err("get_file_dir: bl_log_path! %s\n", bl_log_path);
	print_err("get_file_dir: kernel_log_path! %s\n", kernel_log_path);
}

static void save_log_to_file(struct error_info *info, struct bootfail_meta_log *bf_meta_log, unsigned int index)
{
	int ret;
	struct bootfail_log_header bf_log_header;
	struct bootfail_log_content *log_content;

	char bf_info_path[BF_SIZE_128];
	char bl_log_path[BF_SIZE_128];
	char kernel_log_path[BF_SIZE_128];

	log_content = (struct bootfail_log_content *)vmalloc(sizeof(struct bootfail_log_content));
	if (log_content == NULL) {
		print_err("bbox_dump: vmalloc memory for bootfail logContent failed!\n");
		return;
	}

	memset_s(&bf_log_header, sizeof(bf_log_header), 0, sizeof(bf_log_header));
	memset_s(log_content, sizeof(struct bootfail_log_content), 0, sizeof(struct bootfail_log_content));

	log_content->bl_log_size = bf_meta_log->logs_info[index].bl_log.size > BF_SIZE_128K ?
			BF_SIZE_128K : bf_meta_log->logs_info[index].bl_log.size;
	log_content->kernel_log_size = bf_meta_log->logs_info[index].kernel_log.size > BF_SIZE_512K ?
			BF_SIZE_512K : bf_meta_log->logs_info[index].kernel_log.size;
	log_content->bf_info_start = bf_meta_log->logs_info[index].bl_log.start - BF_SIZE_1K;

	ret = read_content((char *)&bf_log_header, log_content->bf_info_start, BF_SIZE_1K);
	if (ret != 0) {
		print_err("save_to_log_file: read_content bootfail_info failed !\n");
		goto end;
	}

	save_boofail_info(log_content->bf_info_content, &bf_log_header, info);
	bf_log_header.binfo.recovery_result = recovery_success;
	write_content((char *)&bf_log_header, log_content->bf_info_start, BF_SIZE_1K);
	get_file_dir(info, bf_info_path, bl_log_path, kernel_log_path, index);

	ret = write_file(bf_info_path, log_content->bf_info_content, strlen(log_content->bf_info_content));
	if (ret < 0)
		print_err("save_to_log_file: write bootfail info file failed!\n");

	ret = save_log(bl_log_path, log_content->bl_log_content, bf_meta_log->logs_info[index].bl_log.start,
				log_content->bl_log_size);
	if (ret < 0)
		print_err("save_to_log_file: bl_log failed ! ret == %d\n", ret);

	ret = save_log(kernel_log_path, log_content->kernel_log_content, bf_meta_log->logs_info[index].kernel_log.start,
				log_content->kernel_log_size);
	if (ret < 0)
		print_err("save_to_log_file: kernel_log failed ! ret == %d\n", ret);

end:
	vfree(log_content);
}

int send_bbox_notify()
{
	struct bootfail_meta_log bf_meta_log;
	memset_s(&bf_meta_log, sizeof(struct bootfail_meta_log), 0, sizeof(struct bootfail_meta_log));
	bool ret = get_meta_log(&bf_meta_log);
	if (!ret) {
		print_err("send_bbox_notify_kthread, get_meta_log faild!\n");
		return -1;
	}
	unsigned int log_count = bf_meta_log.log_count;
	if (log_count == 0) {
		print_err("send_bbox_notify_kthread, not find bootfail!\n");
		return -1;
	}
	print_err("send_bbox_notify_kthread!\n");

	for (unsigned int i = 0; i < log_count; ++i) {
		struct error_info info;
		memset_s(&info, sizeof(struct error_info), 0, sizeof(struct error_info));
		strncpy_s(info.event, EVENT_MAX_LEN - 1, EVENT_BOOTFAIL, strlen(EVENT_BOOTFAIL));
		strncpy_s(info.module, MODULE_MAX_LEN - 1, MODULE_BOOTFAIL, strlen(MODULE_BOOTFAIL));
		strncpy_s(info.category, CATEGORY_MAX_LEN - 1, CATEGORY_SYSTEM_BOOTFAIL, strlen(CATEGORY_SYSTEM_BOOTFAIL));
		strncpy_s(info.log_path, ROOT_LOG_PATH_LEN - 1, CONFIG_BLACKBOX_LOG_ROOT_PATH,
				strlen(CONFIG_BLACKBOX_LOG_ROOT_PATH));
		info.sysreboot = 0;

		save_log_to_file(&info, &bf_meta_log, i);
		bbox_notify_error(EVENT_BOOTFAIL, MODULE_BOOTFAIL, (char *)&info, false);
	}
	update_rrecord_part(&bf_meta_log);
	return 0;
}
