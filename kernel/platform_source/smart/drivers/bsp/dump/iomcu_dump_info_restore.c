/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2021-2021. All rights reserved.
 * Description: Contexthub save dump file process.
 * Create: 2021-11-26
 */

#include "iomcu_dump_priv.h"
#include <platform_include/smart/linux/iomcu_dump.h>
#include <platform_include/basicplatform/linux/rdr_pub.h>
#include <linux/syscalls.h>
#include "securec.h"
#include <linux/delay.h>
#include <linux/namei.h>
#ifndef CONFIG_SECURITY_HEADER_FILE_REPLACE
#include <iomcu_ddr_map.h>
#else
#include <internal_security_interface.h>
#endif

#define CUR_PATH_LEN 64
#define MAX_DUMP_CNT 32
#define DUMP_REGS_SIZE 128
#define DUMP_REGS_MAGIC 0x7a686f75
#define REG_NAME_SIZE 25
#define LOG_EXTRA_NUM 2
#define SH_DUMP_IPC_MBX_DATA_COUNT 8
#define sh_dump_ao_ns_ipc_data_addr(base, no, id) (((base) + (0x020 + (no) * 64)) + ((id) << 2))

struct dump_zone_head {
	uint32_t cnt;
	uint32_t len;
	uint32_t tuncate;
};

struct dump_zone_element_t {
	uint32_t base;
	uint32_t size;
};

enum m7_register_name {
	SOURCE,
	R0,
	R1,
	R2,
	R3,
	R4,
	R5,
	R6,
	R7,
	R8,
	R9,
	R10,
	R11,
	R12,
	EXP_SP,
	EXP_LR,
	EXP_PC,
	SAVED_PSR,
	CFSR,
	HFSR,
	DFSR,
	MMFAR,
	BFAR,
	AFSR,
	PSP,
	MSP,
	ARADDR_CHK,
	AWADDR_CHK,
	PERI_USED,
	AO_CNT,
	MAGIC,
};

static char g_dump_restore_dir[PATH_MAXLEN] = SH_DMP_DIR;
static uint32_t *g_dp_regs = NULL;
static uint32_t g_dump_index = (uint32_t)-1;

#ifndef CONFIG_RISCV_DUMP_ADAPT
static const char *const g_sh_regs_name[] = {
	"SOURCE", "R0",   "R1",         "R2",         "R3",        "R4",
	"R5",     "R6",   "R7",         "R8",         "R9",        "R10",
	"R11",    "R12",  "EXP_SP",     "EXP_LR",     "EXP_PC",    "SAVED_PSR",
	"CFSR",   "HFSR", "DFSR",       "MMFAR",      "BFAR",      "AFSR",
	"PSP",    "MSP",  "ARADDR_CHK", "AWADDR_CHK", "PERI_USED", "AO_CNT",
};

static char *g_sh_reset_reasons[] = {
	"SH_FAULT_HARDFAULT", // 0
	"SH_FAULT_BUSFAULT",       "SH_FAULT_USAGEFAULT",
	"SH_FAULT_MEMFAULT",       "SH_FAULT_NMIFAULT",
	"SH_FAULT_ASSERT", // 5
	"UNKNOW DUMP FAULT",       "UNKNOW DUMP FAULT",
	"UNKNOW DUMP FAULT",       "UNKNOW DUMP FAULT",
	"UNKNOW DUMP FAULT", // 10
	"UNKNOW DUMP FAULT",       "UNKNOW DUMP FAULT",
	"UNKNOW DUMP FAULT",       "UNKNOW DUMP FAULT",
	"UNKNOW DUMP FAULT",      // 15
	"SH_FAULT_INTERNELFAULT", // 16
	"SH_FAULT_IPC_RX_TIMEOUT", "SH_FAULT_IPC_TX_TIMEOUT",
	"SH_FAULT_RESET",          "SH_FAULT_USER_DUMP",
	"SH_FAULT_RESUME",         "SH_FAULT_REDETECT",
	"SH_FAULT_PANIC",          "SH_FAULT_NOC",
	"SH_FAULT_EXP_BOTTOM", // also use as unknow dump
};
#else
static const char *const g_sh_regs_name[] = {
	"SOURCE", "t0",   "t1",         "t2",         "a0",        "a1",
	"a2",     "a3",   "a4",         "a5",         "a6",        "a7",
	"t3",     "t4",   "sp",         "ra",         "mepc",      "tp",
	"s0",     "s1",   "s2",         "s3",         "s4",        "s5",
	"s6",     "s7",   "s8",         "s9",         "PERI_USED", "AO_CNT",
};

static char *g_sh_reset_reasons_acpu[] = {
	"UNKNOW DUMP FAULT", // 0
	"UNKNOW DUMP FAULT",       "UNKNOW DUMP FAULT",
	"UNKNOW DUMP FAULT",       "UNKNOW DUMP FAULT",
	"UNKNOW DUMP FAULT", // 5
	"UNKNOW DUMP FAULT",       "UNKNOW DUMP FAULT",
	"UNKNOW DUMP FAULT",       "UNKNOW DUMP FAULT",
	"UNKNOW DUMP FAULT", // 10
	"UNKNOW DUMP FAULT",       "UNKNOW DUMP FAULT",
	"UNKNOW DUMP FAULT",       "UNKNOW DUMP FAULT",
	"UNKNOW DUMP FAULT",      // 15
	"SH_FAULT_INTERNELFAULT", // 16
	"SH_FAULT_IPC_RX_TIMEOUT", "SH_FAULT_IPC_TX_TIMEOUT",
	"SH_FAULT_RESET",          "SH_FAULT_USER_DUMP",
	"SH_FAULT_RESUME",         "SH_FAULT_REDETECT",
	"SH_FAULT_PANIC",          "SH_FAULT_NOC",
	"SH_FAULT_EXP_BOTTOM", // also use as unknow dump
};

static char *g_sh_reset_reasons_iomcu[] = {
	"UNKNOW DUMP FAULT", // also use as unknow dump
	"INSTRUCTION_ACCESS_FAULT",  "ILLEGAL_INSTRUCTION",     // 2
	"BREAK_POINT",               "LOAD_ADDRESS_MISALIGNED", // 4
	"LOAD_ACCESS_FAULT", // 5
	"STORE_ADDRESS_MISALIGNED",  "STORE_ACCESS_FAULT",      // 7
	"ENV_CALL_FROM_U_MODE",      "ENV_CALL_FROM_S_MODE",
	"ASSERT_FAULT", // 10
	"ENV_CALL_FROM_M_MODE",      "INSTRUCTION_PAGE_FAULT",
	"LOAD_PAGE_FAULT",           "UNKNOW DUMP FAULT",       // 14
	"STORE_PAGE_FAULT",
	"UNKNOW DUMP FAULT", // 16
	"UNKNOW DUMP FAULT",         "UNKNOW DUMP FAULT",
	"UNKNOW DUMP FAULT",         "UNKNOW DUMP FAULT",
	"UNKNOW DUMP FAULT",         "UNKNOW DUMP FAULT",
	"UNKNOW DUMP FAULT",         "CUSTOMIZED_ASYN_EXCEPTION", // 24
	"NMI_FAULT",
};

#define RISCV_EXCEPTION_START 0x80
#define RISCV_NMI_FAULT 0xFF
#endif

long do_unlinkat(int dfd, struct filename *name);
static int judge_access(const char *filename)
{
	struct path path;
	int error;

	error = kern_path(filename, LOOKUP_FOLLOW, &path);
	if (error)
		return error;
	error = inode_permission(d_inode(path.dentry), MAY_ACCESS);
	path_put(&path);
	return error;
}

static int iomcu_mkdir(const char *pathname, umode_t mode)
{
	struct dentry *dentry;
	struct path path;
	int error;

	dentry = kern_path_create(AT_FDCWD, pathname, &path, LOOKUP_DIRECTORY);
	if (IS_ERR(dentry))
		return (int)PTR_ERR(dentry);
	if (IS_POSIXACL(path.dentry->d_inode) == 0)
		mode &= (umode_t)(~((uint32_t)current_umask()));
	error = security_path_mkdir(&path, dentry, mode);
	if (error == 0)
		error = vfs_mkdir(path.dentry->d_inode, dentry, mode);
	done_path_create(&path, dentry);
	return error;
}

static u64 iomcu_get_tick(void)
{
#if (LINUX_VERSION_CODE < KERNEL_VERSION(4, 19, 0))
	struct timespec uptime;
	get_monotonic_boottime(&uptime);
#else
	struct timespec64 uptime;
	ktime_get_boottime_ts64(&uptime);
#endif

	return (u64)uptime.tv_sec;
}

static int __sh_create_dir(char *path, unsigned int len)
{
	int permission;
	int mkdir;
	mm_segment_t old_fs;

	if (len > CUR_PATH_LEN) {
		pr_err("invalid  parameter. len is %d\n", len);
		return -1;
	}
	if (path == NULL) {
		pr_err("null path\n");
		return -1;
	}

	old_fs = get_fs();
	set_fs(KERNEL_DS); /*lint !e501*/
#ifdef CONFIG_LIBLINUX
	permission = ksys_access(path, 0);
#else
	permission = judge_access(path);
#endif
	if (permission != 0) {
		pr_info("sh: need create dir %s\n", path);
#ifdef CONFIG_LIBLINUX
		mkdir = ksys_mkdir(path, DIR_LIMIT);
#else
		mkdir = iomcu_mkdir(path, DIR_LIMIT);
#endif
		if (mkdir < 0) {
			pr_err("sh: create dir %s failed! ret = %d\n", path, mkdir);
			set_fs(old_fs);
			return mkdir;
		}

		pr_info("sh: create dir %s successed [%d]!!!\n", path, mkdir);
	}

	set_fs(old_fs);
	return 0;
}

static int sh_create_dir(const char *path)
{
	char cur_path[CUR_PATH_LEN];
	int index = 0;

	if (path == NULL) {
		pr_err("invalid  parameter path\n");
		return -1;
	}
	(void)memset_s(cur_path, CUR_PATH_LEN, 0, CUR_PATH_LEN);
	if (*path != '/')
		return -1;
	cur_path[index++] = *path++;
	while ((*path != '\0') && (index < CUR_PATH_LEN)) {
		if (*path == '/')
			__sh_create_dir(cur_path, CUR_PATH_LEN);

		cur_path[index] = *path;
		path++;
		index++;
		if (index >= CUR_PATH_LEN) {
			pr_err("[%s]invalid index %d\n", __func__, index);
			return -1;
		}
	}
	return 0;
}

static int sh_wait_fs(const char *path)
{
	int permission = 0;
	int ret = 0;
	unsigned int wait_index = 0;
	unsigned int wait_max = 1000;
	mm_segment_t old_fs;

	old_fs = get_fs();
	set_fs(KERNEL_DS); /*lint !e501*/

	do {
#ifdef CONFIG_LIBLINUX
		permission = ksys_access(path, 0);
#else
		permission = judge_access(path);
#endif
		if (permission != 0) {
			msleep(10);
			pr_info("[%s] wait ...\n", __func__);
			wait_index++;
		}
	} while ((permission != 0) && (wait_index <= wait_max));
	if (permission != 0) {
		ret = -1;
		pr_err("[%s] cannot access path! errno %d\n", __func__, permission);
	}
	set_fs(old_fs);
	return ret;
}

static int sh_readfs2buf(char *logpath, char *filename, void *buf, u32 len)
{
	int ret = -1;
	ssize_t read_size;
	int flags;
	struct file *fp = NULL;
	mm_segment_t old_fs;
	char path[PATH_MAXLEN];
	int permission;

	if ((logpath == NULL) || (filename == NULL) || (buf == NULL) || (len <= 0)) {
		pr_err("invalid  parameter len:0x%x\n", len);
		goto FIN;
	}

	if (sprintf_s(path, PATH_MAXLEN, "%s/%s", logpath, filename) < 0) {
		pr_err("%s():path length is too large\n", __func__);
		return -1;
	}

	old_fs = get_fs();
	set_fs(KERNEL_DS); /*lint !e501*/

#ifdef CONFIG_LIBLINUX
	permission = ksys_access(path, 0);
#else
	permission = judge_access(path);
#endif
	if (permission != 0)
		goto SET_OLD_FS;

	flags = O_RDWR;
	fp = filp_open(path, flags, FILE_LIMIT);
	if (IS_ERR(fp)) {
		pr_err("%s():open file %s err.\n", __func__, path);
		goto SET_OLD_FS;
	}

	vfs_llseek(fp, 0L, SEEK_SET);
	read_size = vfs_read(fp, buf, len, &(fp->f_pos));
	if (read_size != len)
		pr_err("%s:read file %s exception with ret %u.\n", __func__, path, read_size);
	else
		ret = (int)read_size;

	filp_close(fp, NULL);
SET_OLD_FS:
	set_fs(old_fs);
FIN:
	return ret;
}

static int sh_savebuf2fs(char *logpath, char *filename, void *buf, u32 len, u32 is_append)
{
	int ret = 0;
	ssize_t write_size;
	int flags;
	struct file *fp = NULL;
	mm_segment_t old_fs;
	char path[PATH_MAXLEN];

	if ((logpath == NULL) || (filename == NULL) || (buf == NULL) || (len <= 0)) {
		pr_err("invalid  parameter len:0x%x\n", len);
		ret = -1;
		goto FIN;
	}

	if (sprintf_s(path, PATH_MAXLEN, "%s/%s", logpath, filename) < 0) {
		pr_err("%s():path length is too large\n", __func__);
		return -1;
	}

	old_fs = get_fs();
	set_fs(KERNEL_DS); /*lint !e501*/

	flags = O_CREAT | O_RDWR | (is_append ? O_APPEND : O_TRUNC);
	fp = filp_open(path, flags, FILE_LIMIT);
	if (IS_ERR(fp)) {
		set_fs(old_fs);
		pr_err("%s():create file err.\n", __func__);
		ret = -1;
		goto FIN;
	}

	vfs_llseek(fp, 0L, SEEK_END);
	write_size = vfs_write(fp, buf, len, &(fp->f_pos));
	if (write_size != len) {
		pr_err("%s:write file exception with ret %u.\n", __func__, write_size);
		ret = -1;
		goto FS_CLOSE;
	}

	vfs_fsync(fp, 0);
FS_CLOSE:
	ret = (int)vfs_fchown(fp, ROOT_UID, SYSTEM_GID);
	if (ret)
		pr_err("[%s], chown %s uid [%d] gid [%d] failed err [%d]!\n",
			__func__, path, ROOT_UID, SYSTEM_GID, ret);

	filp_close(fp, NULL);
	set_fs(old_fs);
FIN:
	return ret;
}

static void get_max_dump_cnt(void)
{
	int ret;
	uint32_t index;

	/* find max index */
	ret = sh_readfs2buf(g_dump_restore_dir, "dump_max", &index, sizeof(index));
	if (ret < 0)
		g_dump_index = (uint32_t)-1;
	else
		g_dump_index = index;

	g_dump_index++;
	if (g_dump_index == MAX_DUMP_CNT)
		g_dump_index = 0;
	sh_savebuf2fs(g_dump_restore_dir, "dump_max", &g_dump_index, sizeof(g_dump_index), 0);
}

#ifdef CONFIG_DFX_DEBUG_FS
static void sh_display_dump_info(uint32_t *dp_regs, uint32_t reg_count)
{
	uint32_t i;
	int32_t words = 0;
	int32_t ret;
	char buf[DUMP_REGS_SIZE] = {0};

	for (i = 0; i < reg_count; i++) {
		ret = snprintf_s(buf + words, DUMP_REGS_SIZE - words, REG_NAME_SIZE,
			"%s--%08x, ", g_sh_regs_name[i], dp_regs[i]);
		if (ret < 0)
			return;
		words += ret;
		if ((DUMP_REGS_SIZE - words < REG_NAME_SIZE + LOG_EXTRA_NUM) || (i == (reg_count - 1))) {
			buf[words] = '\n';
			buf[words + 1] = '\0';
			pr_err("%s", buf);
			words = 0;
		}
	}
}

static void get_sh_ao_ns_ipc_dump_info(void __iomem *ao_ns_ipc_base,
	unsigned int *dump_mailno, unsigned int mailno_count)
{
	uint32_t ipc_dp_regs[(uint32_t)MAGIC + 1] = {0};
	uint32_t idx = 0;
	uint32_t i;
	uint32_t j;

	if (ao_ns_ipc_base == NULL) {
		pr_info("%s: dump by ipc not support\n", __func__);
		return;
	}

	for (i = 0; i <= mailno_count; i++) {
		for (j = 0; j < SH_DUMP_IPC_MBX_DATA_COUNT; j++) {
			if (idx == R8)
				idx = EXP_SP; /* skip unsaved reg */
			else if (idx == ARADDR_CHK)
				idx = PERI_USED; /* skip unsaved reg */
			else if (idx > (uint32_t)MAGIC)
				break;

			ipc_dp_regs[idx++] = readl(sh_dump_ao_ns_ipc_data_addr(ao_ns_ipc_base, dump_mailno[i], j));
		}
	}
	pr_err("%s: dump regs magic 0x%x\n", __func__, ipc_dp_regs[MAGIC]);
	sh_display_dump_info(ipc_dp_regs, MAGIC);
}
#endif

static void get_sh_dump_regs(uint8_t * sensorhub_dump_buff)
{
	uint32_t sensorhub_dump_buff_size;
	struct dump_zone_head *dzh = NULL;
	uint32_t magic;

	dzh = (struct dump_zone_head *)sensorhub_dump_buff;
	g_dp_regs = (uint32_t *)(sensorhub_dump_buff + sizeof(struct dump_zone_head) +
		sizeof(struct dump_zone_element_t) * dzh->cnt);
#ifdef CONFIG_SECURITY_HEADER_FILE_REPLACE
	sensorhub_dump_buff_size = dts_ddr_size_get(DTSI_DDR_LOG_BUFF_ADDR_AP);
#else
	sensorhub_dump_buff_size = SENSORHUB_DUMP_BUFF_SIZE;
#endif
	if ((uintptr_t)(g_dp_regs + MAGIC) > (uintptr_t)(sensorhub_dump_buff + sensorhub_dump_buff_size)) {
		pr_err("%s: ramdump maybe failed\n", __func__);
		g_dp_regs = NULL;
		return;
	}
	magic = g_dp_regs[MAGIC];
	pr_info("%s: dump regs magic 0x%x\n", __func__, magic);
	if (magic != DUMP_REGS_MAGIC) {
		g_dp_regs = NULL;
		return;
	}

#ifdef CONFIG_DFX_DEBUG_FS
	sh_display_dump_info(g_dp_regs, MAGIC);
#endif
}

static int get_dump_reason_idx(void)
{
	struct dump_config *dump = iomcu_config_get(IOMCU_CONFIG_USR_DUMP, sizeof(struct dump_config));

#ifndef CONFIG_RISCV_DUMP_ADAPT
	if (dump == NULL)
		return (int)(ARRAY_SIZE(g_sh_reset_reasons) - 1);
	if (dump->reason >= (uint32_t)ARRAY_SIZE(g_sh_reset_reasons))
		return (int)(ARRAY_SIZE(g_sh_reset_reasons) - 1);
	else
		return dump->reason;
#else
	if (dump == NULL)
		return (int)(ARRAY_SIZE(g_sh_reset_reasons_acpu) - 1);
	return dump->reason;
#endif
}

static char *get_dump_reason(void)
{
	u8 reason = (u8)get_dump_reason_idx();

#ifndef CONFIG_RISCV_DUMP_ADAPT
	return g_sh_reset_reasons[reason];
#else
	// IOMCU exception
	if (reason > RISCV_EXCEPTION_START) {
		if (reason == RISCV_NMI_FAULT) { // IOMCU NMI
			reason = ARRAY_SIZE(g_sh_reset_reasons_iomcu) - 1;
		} else {
			reason -= RISCV_EXCEPTION_START;
			if (reason > (ARRAY_SIZE(g_sh_reset_reasons_iomcu) - 1))
				reason = 0;
		}
		pr_info("[%s] iomcu %u\n", __func__, reason);
		return g_sh_reset_reasons_iomcu[reason];
	}
	// ACPU reset IOMCU
	if (reason > (ARRAY_SIZE(g_sh_reset_reasons_acpu) - 1))
		reason = ARRAY_SIZE(g_sh_reset_reasons_acpu) - 1;
	pr_info("[%s] acpu %u\n", __func__, reason);
	return g_sh_reset_reasons_acpu[reason];
#endif
}

static int write_sh_dump_history(union dump_info dump_info)
{
	int ret = 0;
	char buf[HISTORY_LOG_SIZE];
	struct kstat historylog_stat;
	mm_segment_t old_fs;
	char local_path[PATH_MAXLEN];
	char date[DATATIME_MAXLEN] = {0};
	int print_ret = 0;
#ifdef CONFIG_CONTEXTHUB_BOOT_STAT
	int print_loc = 0;
	union sctrl_sh_boot sh_boot;
#endif
	char *content = NULL;

	pr_info("%s: write sensorhub dump history file\n", __func__);
	ret = memset_s(date, DATATIME_MAXLEN, 0, DATATIME_MAXLEN);
	if (ret != EOK)
		pr_err("[%s]memset_s date fail[%d]\n", __func__, ret);
#ifdef CONFIG_DFX_BB
	ret = snprintf_s(date, DATATIME_MAXLEN, DATATIME_MAXLEN - 1, "%s-%08llu", rdr_get_timestamp(), iomcu_get_tick());
	if (ret < 0)
		pr_err("[%s]sprintf_s date fail[%d]\n", __func__, ret);
#endif

	ret = memset_s(local_path, PATH_MAXLEN, 0, PATH_MAXLEN);
	if (ret != EOK)
		pr_err("[%s]memset_s local_path fail[%d]\n", __func__, ret);
	ret = sprintf_s(local_path, PATH_MAXLEN, "%s/%s", g_dump_restore_dir, "history.log");
	if (ret < 0)
		pr_err("[%s]sprintf_s local_path fail[%d]\n", __func__, ret);
	old_fs = get_fs();
	set_fs(KERNEL_DS); /*lint !e501*/

	/* check file size */
	if (vfs_stat(local_path, &historylog_stat) == 0 && historylog_stat.size > HISTORY_LOG_MAX) {
		pr_info("truncate dump history log\n");
		do_unlinkat(AT_FDCWD, getname(local_path)); /* delete history.log */
	}

	set_fs(old_fs);
	/* write history file */
	ret = memset_s(buf, HISTORY_LOG_SIZE, 0, HISTORY_LOG_SIZE);
	if (ret != EOK)
		pr_err("[%s]memset_s local_path fail[%d]\n", __func__, ret);

	content = get_dump_reason();
	if (g_dp_regs != NULL) {
		print_ret = snprintf_s(buf, sizeof(buf), sizeof(buf) - 1, "reason [%s], [%02u], time [%s],"
			"%x, %x, %x, %x, %x, %x, %x, %x, dump_info[0x%x]\n",
			content, g_dump_index, date,
			g_dp_regs[EXP_PC], g_dp_regs[EXP_LR], g_dp_regs[CFSR],
			g_dp_regs[HFSR], g_dp_regs[MMFAR], g_dp_regs[BFAR],
			g_dp_regs[PERI_USED], g_dp_regs[AO_CNT], dump_info.value);
		if (print_ret < 0)
			pr_err("%s write dp regs history failed\n", __func__);
	} else {
		print_ret = snprintf_s(buf, sizeof(buf), sizeof(buf) - 1,
			"reason [%s], [%02u], time [%s],dump_info[0x%x]",
			content, g_dump_index, date, dump_info.value);
		if (print_ret < 0)
			pr_err("%s write history failed\n", __func__);
	}
	/* write boot_step and boot_stat to history.log */
#ifdef CONFIG_CONTEXTHUB_BOOT_STAT
	sh_boot = get_boot_stat();
	reset_boot_stat();
	if (print_ret > 0) {
		print_loc += print_ret;
		if (print_loc >= (int)(sizeof(buf) - 1)) {
			pr_err("%s write boot stat failed\n", __func__);
			goto save_buf;
		}
		print_ret = snprintf_s(buf + print_loc, sizeof(buf) - print_loc, sizeof(buf) - print_loc - 1,
				"boot_step [%x], boot_stat [%x]\n", sh_boot.reg.boot_step, sh_boot.reg.boot_stat);
		if (print_ret < 0)
			pr_err("[%s]sprintf_s boot_state fail[%d]\n", __func__, ret);
	}
save_buf:
#endif
	(void)sh_savebuf2fs(g_dump_restore_dir, "history.log", buf, strlen(buf), 1);
	return ret;
}

static int write_sh_dump_file(uint8_t * sensorhub_dump_buff)
{
	int ret;
	char date[DATATIME_MAXLEN];
	char path[PATH_MAXLEN];
	struct dump_zone_head *dzh = NULL;
	struct dump_config *dump = NULL;

	ret = memset_s(date, DATATIME_MAXLEN, 0, DATATIME_MAXLEN);
	if (ret != EOK)
		pr_err("[%s]memset_s data fail[%d]\n", __func__, ret);
#ifdef CONFIG_DFX_BB
	ret = snprintf_s(date, DATATIME_MAXLEN, DATATIME_MAXLEN - 1, "%s-%08llu", rdr_get_timestamp(), iomcu_get_tick());
	if (ret < 0)
		pr_err("[%s]sprintf_s DATATIME_MAXLEN fail[%d]\n", __func__, ret);
#endif

	ret = memset_s(path, PATH_MAXLEN, 0, PATH_MAXLEN);
	if (ret != EOK)
		pr_err("[%s]memset_s path fail[%d]\n", __func__, ret);

	ret = sprintf_s(path, PATH_MAXLEN, "sensorhub-%02u.dmp", g_dump_index);
	if (ret < 0)
		pr_err("[%s]sprintf_s PATH_MAXLEN fail[%d]\n", __func__, ret);
	pr_info("%s: write sensorhub dump  file %s\n", __func__, path);
	pr_err("sensorhub recovery source is %s\n", get_dump_reason());
#if (KERNEL_VERSION(4, 4, 0) > LINUX_VERSION_CODE)
	flush_cache_all();
#endif

	dump = iomcu_config_get(IOMCU_CONFIG_USR_DUMP, sizeof(struct dump_config));
	/* write share part */
	if ((sensorhub_dump_buff != NULL) && (dump != NULL)) {
		dzh = (struct dump_zone_head *)sensorhub_dump_buff;
		sh_savebuf2fs(g_dump_restore_dir, path, sensorhub_dump_buff,
			min(dump->dump_size, dzh->len), 0);
	}

	return 0;
}

int save_sh_dump_file(uint8_t * sensorhub_dump_buff, union dump_info dump_info,
	void __iomem *ao_ns_ipc_base, unsigned int *dump_mailno, unsigned int mailno_count)
{
	char dump_fs[PATH_MAXLEN] = SH_DMP_FS;
	if (IS_ENABLE_DUMP == 0) {
		pr_info("%s skipped!\n", __func__);
		return 0;
	}
	if (sh_wait_fs(dump_fs) != 0) {
		pr_err("[%s] fs not ready!\n", __func__);
		return -1;
	}
	pr_info("%s fs ready\n", __func__);
	// check and create dump dir
	if (sh_create_dir(g_dump_restore_dir)) {
		pr_err("%s failed to create dir %s\n", __func__, g_dump_restore_dir);
		return -1;
	}

	if (sensorhub_dump_buff == NULL) {
		pr_err("%s sensorhub_dump_buff is null\n", __func__);
		return -1;
	}

	get_max_dump_cnt();
	get_sh_dump_regs(sensorhub_dump_buff);
#ifdef CONFIG_DFX_DEBUG_FS
	get_sh_ao_ns_ipc_dump_info(ao_ns_ipc_base, dump_mailno, mailno_count);
#endif
	write_sh_dump_history(dump_info);
	write_sh_dump_file(sensorhub_dump_buff);
	return 0;
}
