/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2019-2021. All rights reserved.
 * Description: Contexthub dump process.
 * Create: 2019-06-10
 */

#include "securec.h"
#ifndef CONFIG_CONTEXTHUB_DFX_NOCMID
#ifndef CONFIG_SECURITY_HEADER_FILE_REPLACE
#include "soc_mid.h"
#endif
#endif
#include <linux/debugfs.h>
#include <linux/delay.h>
#include <platform_include/basicplatform/linux/rdr_pub.h>
#ifdef CONFIG_DFX_SEC_QIC_V300_MODID_REGISTER
#include <platform_include/basicplatform/linux/dfx_qic_v300_modid_para.h>
#include <soc_qic_bus.h>
#ifndef CONFIG_SECURITY_HEADER_FILE_REPLACE
#include <soc_mid.h>
#endif
#endif
#include <linux/of_irq.h>
#include <linux/of_platform.h>
#include <linux/syscalls.h>
#ifdef CONFIG_DFX_NOC_MODID_REGISTER
#include <platform_include/basicplatform/linux/dfx_noc_modid_para.h>
#endif
#include <platform_include/smart/linux/iomcu_dump.h>
#include <platform_include/smart/linux/iomcu_ipc.h>
#include <platform_include/smart/linux/iomcu_status.h>
#include "iomcu_dump_priv.h"
#ifndef CONFIG_SECURITY_HEADER_FILE_REPLACE
#include <iomcu_ddr_map.h>
#else
#include <internal_security_interface.h>
#endif

#ifdef CONFIG_CONTEXTHUB_LOAD_IOMCU_IMAGE
#include "load_iomcu_ca.h"
#endif


#ifdef CONFIG_SECURITY_HEADER_FILE_REPLACE
uint32_t iomcu_m7_mid = 0;
uint32_t iomcu_dma_mid = 0;
#endif

#define IOMCU_CLK_SEL 0x00
#define IOMCU_CFG_DIV0 0x04
#define IOMCU_CFG_DIV1 0x08
#define CLKEN0_OFFSET 0x010
#define CLKSTAT0_OFFSET 0x18
#define CLKEN1_OFFSET 0x090
#define RSTEN0_OFFSET 0x020
#define RSTDIS0_OFFSET 0x024
#define RSTSTAT0_OFFSET 0x028
#define I2C_0_RST_VAL (BIT(3))
#define IOM3_REC_NEST_MAX 5
#define WD_INT 144
#define SENSORHUB_MODID_END DFX_BB_MOD_IOM_END
#define IOMCU_DUMP_IPC_MAILNO_COUNT 3
#define IPMCU_DUMP_IPC_MAILNO_MAX 10
/*
 * it need to add to modid_react_dump when adding new SENSORHUB MOID to avoid
 * twice dump in sh_dump
 */
#define modid_react_dump(modid) ((modid) != SENSORHUB_MODID && (modid) != SENSORHUB_USER_MODID)

#ifdef CONFIG_SMART_WATCHDOG_V500
#define WDOGLOCK   0x0
#define WDOGUNLOCK 0x04
#define WDOGCTRL   0x14
#define WDOGINTCLR 0x1C
#define WDOGRSTM   0x24
#define REG_UNLOCK_KEY 0x1AAEE533
#else
#define WDOGCTRL 0x08
#define WDOGINTCLR 0x0C
#define WDOGLOCK 0xC00
#define WDOGUNLOCK 0xC00
#define REG_UNLOCK_KEY 0x1ACCE551
#endif

#define SH_DUMP_INIT 0
#define SH_DUMP_START 1
#define SH_DUMP_FINISH 2

#ifdef CONFIG_DFX_NOC_MODID_REGISTER
struct sh_noc_register_info {
	struct noc_err_para_s sh_noc_err_para;
	u32 modid;
};
#endif

static atomic_t g_iom3_state = ATOMIC_INIT(IOM3_ST_NORMAL);
static int g_sensorhub_wdt_irq = -1;
static BLOCKING_NOTIFIER_HEAD(iom3_recovery_notifier_list);
static wait_queue_head_t iom3_rec_waitq;
static atomic_t iom3_rec_state = ATOMIC_INIT(IOM3_RECOVERY_UNINIT);
static struct rdr_register_module_result current_sh_info;
static struct completion sensorhub_rdr_completion;
static uint8_t *g_sensorhub_dump_buff = NULL;
static uint32_t g_dump_extend_size;
static struct completion iom3_rec_done;

static struct workqueue_struct *g_iom3_rec_wq = NULL;
static struct delayed_work g_iom3_rec_work;
static struct wakeup_source *g_iom3_rec_wl;
static u64 g_current_sh_core_id = RDR_IOM3; /* const */
static struct semaphore g_rdr_sh_sem;
static struct semaphore g_rdr_exce_sem;
static void __iomem *g_watchdog_base = NULL;
static void __iomem *g_iomcu_ao_ns_ipc_base = NULL;
static unsigned int *g_dump_vir_addr = NULL;
static struct wakeup_source *g_rdr_wl;
static char g_dump_dir[PATH_MAXLEN] = SH_DMP_DIR;
#ifdef CONFIG_DFX_DEBUG_FS
#ifdef CONFIG_DFX_BB
static u64 g_rdr_core_mask = RDR_IOM3 | RDR_AP | RDR_LPM3;
#endif
#endif
static bool g_sensorhub_dump_reset = true;
static atomic_t g_dump_minready_done = ATOMIC_INIT(1);
static struct task_struct *thread_savefile = NULL;
static struct task_struct *thread_reco = NULL;
static union dump_info g_dump_info;

static rproc_id_t ipc_ap_to_iom_mbx = IPC_AO_ACPU_IOM3_MBX1;
static rproc_id_t ipc_ap_to_lpm_mbx = IPC_ACPU_LPM3_MBX_5;
static unsigned int g_dump_mail_no[IOMCU_DUMP_IPC_MAILNO_COUNT];

#ifdef CONFIG_SECURITY_HEADER_FILE_REPLACE
uint32_t g_sensorhub_dump_buff_addr = 0;
uint32_t g_sensorhub_dump_buff_size = 0;
#else
uint32_t g_sensorhub_dump_buff_addr = SENSORHUB_DUMP_BUFF_ADDR;
uint32_t g_sensorhub_dump_buff_size = SENSORHUB_DUMP_BUFF_SIZE;
#endif

static int iom3_need_recovery_for_dump(int modid, enum exp_source f);

int get_iom3_state(void)
{
	return atomic_read(&g_iom3_state);
}

int get_iom3_rec_state(void)
{
	return atomic_read(&iom3_rec_state);
}

void set_iom3_rec_state(int state)
{
	atomic_set(&iom3_rec_state, state);
}

static int get_watchdog_base(void)
{
	struct device_node *np = NULL;

	if (g_watchdog_base == NULL) {
		np = of_find_compatible_node(NULL, NULL, "hisilicon,iomcu-watchdog");
		if (np == NULL) {
			pr_err("can not find watchdog node\n");
			return -1;
		}

		g_watchdog_base = of_iomap(np, 0);
		if (g_watchdog_base == NULL) {
			pr_err("get g_watchdog_base  error !\n");
			return -1;
		}
	}
	return 0;
}

static void get_dump_config_from_dts(void)
{
	struct dump_config *dump = NULL;
	pr_info("DMPenabled[%d]ext size[0x%x]\n", IS_ENABLE_DUMP, g_dump_extend_size);
	pr_info("log dir is %s\n", g_dump_dir);

	dump = iomcu_config_get(IOMCU_CONFIG_USR_DUMP, sizeof(struct dump_config));
	if (dump != NULL)
		dump->enable = IS_ENABLE_DUMP;
}

static int write_ramdump_info_to_sharemem(void)
{
	struct dump_config *dump = iomcu_config_get(IOMCU_CONFIG_USR_DUMP, sizeof(struct dump_config));
	if (dump == NULL) {
		pr_err("[%s] g_smplat_scfg is NULL\n", __func__);
		return -1;
	}
#ifdef CONFIG_SECURITY_HEADER_FILE_REPLACE
	g_sensorhub_dump_buff_addr = dts_ddr_addr_get(DTSI_DDR_LOG_BUFF_ADDR_AP);
	g_sensorhub_dump_buff_size = dts_ddr_size_get(DTSI_DDR_LOG_BUFF_ADDR_AP);
#endif
	dump->finish = SH_DUMP_INIT;
	dump->dump_addr = g_sensorhub_dump_buff_addr;
	dump->dump_size = g_sensorhub_dump_buff_size;
	dump->ext_dump_addr = 0;
	dump->ext_dump_size = 0;
	dump->reason = 0;

	pr_info("%s dumpaddr low is %x, dumpaddr high is %x, dumplen is %x, end!\n",\
		__func__, (u32)(dump->dump_addr),
		(u32)((dump->dump_addr) >> 32),
		dump->dump_size);
	return 0;
}

static void notify_rdr_thread(void)
{
	up(&g_rdr_sh_sem);
}

#ifdef CONFIG_DFX_BB
static int is_sh_need_reset(void)
{
#ifdef CONFIG_DFX_SEC_QIC_V100_MODID_REGISTER
#define SENSORHUB_WORKAROUND_QIC_ADDR 0xfae00000
	struct dfx_errlog_info errlog_info;

	dfx_qic_get_errlog_info(&errlog_info);
	pr_warn("%s: mid=0x%x, err_addr=0x%x\n", __func__, errlog_info.master_id, errlog_info.err_addr);
#ifdef CONFIG_SECURITY_HEADER_FILE_REPLACE
	if (errlog_info.master_id == iomcu_m7_mid && errlog_info.err_addr\
		== SENSORHUB_WORKAROUND_QIC_ADDR)
		return 0;
#else
	if (errlog_info.master_id == SOC_IOMCU_M7_MID && errlog_info.err_addr\
		== SENSORHUB_WORKAROUND_QIC_ADDR)
		return 0;
#endif
#endif
	return 1;
}

static void sh_dump(u32 modid, u32 etype, u64 coreid, char *pathname, pfn_cb_dump_done pfn_cb)
{
	enum exp_source f = SH_FAULT_REACT;
	union dump_info info;

#ifdef CONFIG_CONTEXTHUB_IGS
	if (modid == SENSORHUB_FDUL_MODID)
		f = SH_FAULT_NOC;
#endif
	pr_warn("%s: modid=0x%x, etype=0x%x, coreid=0x%llx, f=%d\n", __func__, modid, etype, coreid, f);

	if (modid_react_dump(modid) && is_sh_need_reset() != 0) {
		pr_warn("%s:start recovery\n", __func__);
		info.modid = modid;
		save_dump_info_to_history(info);
		iom3_need_recovery_for_dump((int)modid, f);
	}

	if (pfn_cb)
		pfn_cb(modid, g_current_sh_core_id);
}

static void sh_reset(u32 modid, u32 etype, u64 coreid)
{
	int ret;

	pr_info("%s\n", __func__);

	if ((g_dump_vir_addr != NULL) && (current_sh_info.log_len != 0)) {
		ret = memset_s(g_dump_vir_addr, current_sh_info.log_len, 0, current_sh_info.log_len);
		if (ret != EOK)
			pr_err("[%s]memset_s  fail[%d]\n", __func__, ret);
	}
}

static int clean_rdr_memory(struct rdr_register_module_result rdr_info)
{
	int ret;

	g_dump_vir_addr = (unsigned int *)dfx_bbox_map(rdr_info.log_addr, rdr_info.log_len);
	if (g_dump_vir_addr == NULL) {
		pr_err("dfx_bbox_map (%llx) failed in %s!\n", rdr_info.log_addr, __func__);
		return -1;
	}

	ret = memset_s(g_dump_vir_addr, rdr_info.log_len, 0, rdr_info.log_len);
	if (ret != EOK)
		pr_err("[%s]memset_s  fail[%d]\n", __func__, ret);
	return 0;
}

static int write_bbox_log_info_to_sharemem(struct rdr_register_module_result *rdr_info)
{
	struct bbox_config *bbox = iomcu_config_get(IOMCU_CONFIG_USR_BBOX, sizeof(struct bbox_config));
	if (bbox == NULL) {
		pr_err("%s: g_smplat_scfg is NULL\n", __func__);
		return -1;
	}

	if (rdr_info == NULL) {
		pr_err("%s: rdr_info is NULL\n", __func__);
		return -1;
	}

	bbox->log_addr = rdr_info->log_addr;
	bbox->log_size = rdr_info->log_len;

	pr_info("%s: bbox log addr: low 0x%x, high 0x%x, log len: 0x%x\n", __func__,
		(u32)(rdr_info->log_addr), (u32)((rdr_info->log_addr) >> 32),
		rdr_info->log_len);
	return 0;
}
static int rdr_sensorhub_register_core(void)
{
	struct rdr_module_ops_pub s_module_ops;
	int ret;

	pr_info("%s start!\n", __func__);

	s_module_ops.ops_dump = sh_dump;
	s_module_ops.ops_reset = sh_reset;

	ret = rdr_register_module_ops(g_current_sh_core_id, &s_module_ops, &current_sh_info);
	if (ret != 0)
		return ret;

	pr_info("%s end!\n", __func__);
	return ret;
}

struct rdr_exception_info_s g_iomcu_exception_info[] = {
	{
		.e_modid = SENSORHUB_MODID,
		.e_modid_end = SENSORHUB_MODID,
		.e_process_priority = RDR_WARN,
		.e_reboot_priority = RDR_REBOOT_NO,
		.e_notify_core_mask = RDR_IOM3 | RDR_AP | RDR_LPM3,
		.e_reentrant = RDR_REENTRANT_ALLOW,
		.e_exce_type = IOM3_S_EXCEPTION,
		.e_from_core = RDR_IOM3,
		.e_upload_flag = RDR_UPLOAD_YES,
		.e_save_log_flags = RDR_SAVE_LOGBUF,
		.e_from_module = "RDR_IOM7",
		.e_desc = "RDR_IOM7 for watchdog timeout issue."
	},
	{
		.e_modid = SENSORHUB_USER_MODID,
		.e_modid_end = SENSORHUB_USER_MODID,
		.e_process_priority = RDR_WARN,
		.e_reboot_priority = RDR_REBOOT_NO,
		.e_notify_core_mask = RDR_IOM3 | RDR_AP | RDR_LPM3,
		.e_reentrant = RDR_REENTRANT_ALLOW,
		.e_exce_type = IOM3_S_USER_EXCEPTION,
		.e_from_core = RDR_IOM3,
		.e_upload_flag = RDR_UPLOAD_YES,
		.e_save_log_flags = RDR_SAVE_LOGBUF,
		.e_from_module = "RDR_IOM7",
		.e_desc = "RDR_IOM7 for user trigger dump."
	},
#ifdef CONFIG_CONTEXTHUB_DFX_UNI_NOC
	{
		.e_modid = SENSORHUB_IOMCU_DMA_MODID,
		.e_modid_end = SENSORHUB_IOMCU_DMA_MODID,
		.e_process_priority = RDR_WARN,
		.e_reboot_priority = RDR_REBOOT_NO,
		.e_notify_core_mask = RDR_IOM3 | RDR_AP | RDR_LPM3,
		.e_reentrant = RDR_REENTRANT_ALLOW,
		.e_exce_type = IOM3_S_EXCEPTION,
		.e_from_core = RDR_IOM3,
		.e_upload_flag = RDR_UPLOAD_YES,
		.e_save_log_flags = RDR_SAVE_LOGBUF,
		.e_from_module = "RDR_IOM7",
		.e_desc = "RDR_IOM7 for IOMCU_DMA trigger dump."
	},
#endif
#ifdef CONFIG_CONTEXTHUB_IGS
	{
		.e_modid = SENSORHUB_FDUL_MODID,
		.e_modid_end = SENSORHUB_FDUL_MODID,
		.e_process_priority = RDR_WARN,
		.e_reboot_priority = RDR_REBOOT_NO,
		.e_notify_core_mask = RDR_IOM3 | RDR_AP | RDR_LPM3,
		.e_reset_core_mask = RDR_IOM3,
		.e_reentrant = RDR_REENTRANT_ALLOW,
		.e_exce_type = IOM3_S_FDUL_EXCEPTION,
		.e_from_core = RDR_IOM3,
		.e_upload_flag = RDR_UPLOAD_YES,
		.e_save_log_flags = RDR_SAVE_LOGBUF,
		.e_from_module = "RDR_IOM7",
		.e_desc = "RDR_IOM7 for FD_UL trigger dump."
	},
#ifdef CONFIG_CONTEXTHUB_IGS_20
	{
		.e_modid = SENSORHUB_TINY_MODID,
		.e_modid_end = SENSORHUB_TINY_MODID,
		.e_process_priority = RDR_WARN,
		.e_reboot_priority = RDR_REBOOT_NO,
		.e_notify_core_mask = RDR_IOM3 | RDR_AP | RDR_LPM3,
		.e_reset_core_mask = RDR_IOM3,
		.e_reentrant = RDR_REENTRANT_ALLOW,
		.e_exce_type = IOM3_S_FDUL_EXCEPTION,
		.e_from_core = RDR_IOM3,
		.e_upload_flag = RDR_UPLOAD_YES,
		.e_save_log_flags = RDR_SAVE_LOGBUF,
		.e_from_module = "RDR_IOM7",
		.e_desc = "RDR_IOM7 for NPU_TINY trigger dump."
	},
#endif
#endif
};


static void sh_rdr_register(void)
{
	unsigned int array_size;
	unsigned int ret;
	unsigned int index;
	pr_info("%s start!\n", __func__);

	array_size = (unsigned int)(sizeof(g_iomcu_exception_info) / sizeof(g_iomcu_exception_info[0]));
	for (index = 0; index < array_size; index++) {
		ret = rdr_register_exception(&g_iomcu_exception_info[index]);
		if (ret == 0)
			pr_err("%s: register modid=%u failed[%d]\n", __func__, g_iomcu_exception_info[index].e_modid, ret);
	}
}

#ifdef CONFIG_DFX_DEBUG_FS
static void sh_rdr_unregister(void)
{
	unsigned int array_size;
	unsigned int index;
	pr_info("%s start!\n", __func__);

	array_size =(unsigned int)(sizeof(g_iomcu_exception_info) / sizeof(g_iomcu_exception_info[0]));
	for (index = 0; index < array_size; index++)
		(void)rdr_unregister_exception(g_iomcu_exception_info[index].e_modid);
}
#endif

#ifdef CONFIG_DFX_NOC_MODID_REGISTER
static struct sh_noc_register_info g_iomcu_noc_info[] = {
#ifdef CONFIG_CONTEXTHUB_IGS
	{
		.sh_noc_err_para = {SOC_FD_UL_MID,
				    0xFF,
				    NOC_ERRBUS_SYS_CONFIG},
		.modid = SENSORHUB_FDUL_MODID
	},
#ifdef CONFIG_CONTEXTHUB_IGS_20
	{
		.sh_noc_err_para = {SOC_NPU_TINY_MID,
				    0xFF,
				    NOC_ERRBUS_NPU},
		.modid = SENSORHUB_TINY_MODID
	},
	{
		.sh_noc_err_para = {SOC_NPU_TINY_MID,
				    0xFF,
				    NOC_ERRBUS_SYS_CONFIG},
		.modid = SENSORHUB_TINY_MODID
	},
#endif
#endif
#ifdef CONFIG_CONTEXTHUB_DFX_UNI_NOC
	{
		.sh_noc_err_para = {SOC_IOMCU_M7_MID,
				    0xFF,
				    NOC_ERRBUS_SYS_CONFIG},
		.modid = SENSORHUB_MODID
	},
	{
		.sh_noc_err_para = {SOC_IOMCU_DMA_AO_TCP_BB_DRX_MID,
				    0xFF,
				    NOC_ERRBUS_SYS_CONFIG},
		.modid = SENSORHUB_IOMCU_DMA_MODID
	},
	{
		.sh_noc_err_para = {SOC_IOMCU_DMA_AO_TCP_BB_DRX_MID,
				    0xFF,
				    NOC_ERRBUS_NPU},
		.modid = SENSORHUB_IOMCU_DMA_MODID
	},
#endif
};
#endif

static void sh_noc_modid_register(void)
{
#ifdef CONFIG_DFX_NOC_MODID_REGISTER

	unsigned int array_size;
	unsigned int index;

	array_size = (unsigned int)(sizeof(g_iomcu_noc_info) / sizeof(g_iomcu_noc_info[0]));
	for (index = 0; index < array_size; index++)
		noc_modid_register(g_iomcu_noc_info[index].sh_noc_err_para, g_iomcu_noc_info[index].modid);
#else
	return;
#endif
}

#ifdef CONFIG_SECURITY_HEADER_FILE_REPLACE
static int get_iomcu_qic_mid(unsigned int *m7_mid, unsigned int *dma_mid)
{
	int ret;
	struct device_node *qic_node;
	qic_node = of_find_node_by_name(NULL, "sensorhub_qic_mid");
	if (!qic_node) {
		pr_err("%s, can not find node ddr_log_buff\n", __func__);
		return -1;
	}
	ret = of_property_read_u32_index(qic_node, "qic_mid", 0, dma_mid);
	if (ret) {
		pr_err("%s failed to get dma_mid! ret=%d\n", __func__, ret);
		of_node_put(qic_node);
		return -1;
	}
	ret = of_property_read_u32_index(qic_node, "qic_mid", 1, m7_mid);
	if (ret) {
		pr_err("%s failed to get m7_mid! ret=%d\n", __func__, ret);
		of_node_put(qic_node);
		return -1;
	}
	of_node_put(qic_node);
	/* qic id get OK */
	return 0;
}
#endif

static void sh_qic_register(void)
{
#ifdef CONFIG_DFX_SEC_QIC_V100_MODID_REGISTER
	dfx_qic_modid_register(QIC_IOMCU, SENSORHUB_MODID);
	dfx_qic_modid_register(QIC_IOMCU_DMA, SENSORHUB_IOMCU_DMA_MODID);
	dfx_qic_modid_register(QIC_NPU_AICORE0, SENSORHUB_TINY_MODID);
	dfx_qic_modid_register(QIC_NPU_TS_0, SENSORHUB_TINY_MODID);
	dfx_qic_modid_register(QIC_NPU_TS_1, SENSORHUB_TINY_MODID);
#elif defined (CONFIG_DFX_SEC_QIC_V300_MODID_REGISTER)
#ifdef CONFIG_SECURITY_HEADER_FILE_REPLACE
	dfx_qic_modid_register(iomcu_m7_mid, iomcu_m7_mid,
						   QIC_AO_BUS | QIC_IOMCU_BUS, SENSORHUB_MODID);
#else
	dfx_qic_modid_register(SOC_IOMCU_M7_MID, SOC_IOMCU_M7_MID,
						   QIC_AO_BUS | QIC_IOMCU_BUS, SENSORHUB_MODID);
#endif						   
#ifdef CONFIG_CONTEXTHUB_DFX_UNI_NOC
#ifdef CONFIG_SECURITY_HEADER_FILE_REPLACE
	dfx_qic_modid_register(iomcu_dma_mid, iomcu_dma_mid,
						   QIC_AO_BUS | QIC_IOMCU_BUS, SENSORHUB_IOMCU_DMA_MODID);
#else
	dfx_qic_modid_register(SOC_IOMCU_DMA_MID, SOC_IOMCU_DMA_MID,
						   QIC_AO_BUS | QIC_IOMCU_BUS, SENSORHUB_IOMCU_DMA_MODID);
#endif
#endif
#else
	return;
#endif
}

static void sh_register_exception(void)
{
	sh_rdr_register();

	sh_qic_register();

	sh_noc_modid_register();
}
#endif // CONFIG_DFX_BB

static void wdt_stop(void)
{
	/* only here */
	if (g_watchdog_base == NULL) {
		pr_err("[%s] g_watchdog_base is NULL!\n", __func__);
		return;
	}
	writel(REG_UNLOCK_KEY, g_watchdog_base + WDOGLOCK);
	writel(0x0, g_watchdog_base + WDOGCTRL);
	writel(0x1, g_watchdog_base + WDOGINTCLR);
#ifdef WDOGRSTM
	writel(0x1, g_watchdog_base + WDOGRSTM);
#endif
	writel(0x01, g_watchdog_base + WDOGUNLOCK);
}

static irqreturn_t watchdog_handler(int irq, void *data)
{
	wdt_stop();
	pr_warn("%s start\n", __func__);
	__pm_stay_awake(g_rdr_wl);
	/* release exception sem */
	up(&g_rdr_exce_sem);
	return IRQ_HANDLED;
}

static int rdr_sh_thread(void *arg)
{
	int timeout;
	struct dump_config *dump = NULL;

	pr_warn("%s start!\n", __func__);

	while (1) {
		timeout = 2000;
		down(&g_rdr_sh_sem);
		if (IS_ENABLE_DUMP) {
			pr_warn(" ===========dump sensorhub log start==========\n");
			while (1) {
				dump = iomcu_config_get(IOMCU_CONFIG_USR_DUMP, sizeof(struct dump_config));
				if (dump == NULL || dump->finish == SH_DUMP_FINISH)
					break;
				if (timeout == 0)
					break;
				timeout--;
				mdelay(1);
			}
			pr_warn(" ===========sensorhub dump finished==========\n");
			if (dump != NULL)
				pr_warn("dump reason idx %d\n", dump->reason);
			/* write to fs */
			save_sh_dump_file(g_sensorhub_dump_buff, g_dump_info,
				g_iomcu_ao_ns_ipc_base, g_dump_mail_no, IOMCU_DUMP_IPC_MAILNO_COUNT);
			pr_warn(" ===========dump sensorhub log end==========\n");
#ifdef CONFIG_DFX_DEBUG_FS
			show_iom3_stat();
#endif
		}
		__pm_relax(g_rdr_wl);
		complete_all(&sensorhub_rdr_completion);
#ifdef __LLT_UT__
		break;
#endif
	}

	return 0;
}

static int rdr_exce_thread(void *arg)
{
	pr_warn("%s start!\n", __func__);
	while (1) {
		down(&g_rdr_exce_sem);
		pr_warn(" ==============trigger exception==============\n");
		iom3_need_recovery(SENSORHUB_MODID, SH_FAULT_INTERNELFAULT);
#ifdef __LLT_UT__
	break;
#endif
	}

	return 0;
}

int register_iom3_recovery_notifier(struct notifier_block *nb)
{
	return blocking_notifier_chain_register(&iom3_recovery_notifier_list, nb);
}

int unregister_iom3_recovery_notifier(struct notifier_block *nb)
{
	return blocking_notifier_chain_unregister(&iom3_recovery_notifier_list, nb);
}

static void notify_modem_when_iomcu_recovery_finish(void)
{
	uint16_t status = ST_RECOVERY_FINISH;
	struct write_info pkg_ap = {0};
	int ret;

	pr_info("%s\n", __func__);

	pkg_ap.tag = TAG_SYS;
	pkg_ap.cmd = CMD_SYS_STATUSCHANGE_REQ;
	pkg_ap.wr_buf = &status;
	pkg_ap.wr_len = (int)(sizeof(status));
	ret = write_customize_cmd(&pkg_ap, NULL, false);
	if (ret != 0)
		pr_warn("[%s]write_customize_cmd err ret[%d]\n", __func__, ret);
}

/*
 * return value
 * 0:success
 * <0:error
 */
int rdr_sensorhub_init_early(void)
{
#ifdef CONFIG_DFX_BB
	int ret;
	/* register module. */
	ret = rdr_sensorhub_register_core();
	if (ret != 0)
		return ret;

	ret = clean_rdr_memory(current_sh_info);
	if (ret != 0)
		return ret;

	ret = write_bbox_log_info_to_sharemem(&current_sh_info);
	if (ret != 0)
		return ret;
	/* write ramdump info to share mem */
	ret = write_ramdump_info_to_sharemem();
	if (ret != 0)
		return ret;
	get_dump_config_from_dts();
#ifdef CONFIG_SECURITY_HEADER_FILE_REPLACE
	g_sensorhub_dump_buff_addr = dts_ddr_addr_get(DTSI_DDR_LOG_BUFF_ADDR_AP);
	g_sensorhub_dump_buff_size = dts_ddr_size_get(DTSI_DDR_LOG_BUFF_ADDR_AP);
#endif
	g_sensorhub_dump_buff = (uint8_t *)ioremap_wc(g_sensorhub_dump_buff_addr, g_sensorhub_dump_buff_size);
	if (g_sensorhub_dump_buff == NULL) {
		pr_err("%s failed remap dump buff\n", __func__);
		return -EINVAL;
	}

	sh_register_exception();

	return ret;
#else
	return 0;
#endif
}

static int sensorhub_panic_notify(struct notifier_block *nb, unsigned long action, void *data)
{
	int timeout;

	pr_warn("%s start\n", __func__);
	timeout = 100;
	send_nmi();
	pr_warn("%s\n", __func__);

	while (1) {
		struct dump_config *dump = iomcu_config_get(IOMCU_CONFIG_USR_DUMP, sizeof(struct dump_config));
		if (dump == NULL || dump->finish == SH_DUMP_FINISH)
			break;
		if (timeout == 0)
			break;
		timeout--;
		mdelay(1);
	}
	pr_warn("%s done\n", __func__);
	return NOTIFY_OK;
}

static struct notifier_block sensorhub_panic_block = {
	.notifier_call = sensorhub_panic_notify,
};

static int get_iomcu_wdt_irq_num(void)
{
	struct device_node *np = NULL;

	if (g_sensorhub_wdt_irq < 0) {
		np = of_find_compatible_node(NULL, NULL, "huawei,sensorhub_io");
		if (np == NULL) {
			pr_err("can not find  iomcuctrl node !\n");
			return -1;
		}
		g_sensorhub_wdt_irq = of_irq_get(np, 0);
	}

	return g_sensorhub_wdt_irq;
}

static void iomcu_rdr_notifier(void)
{
	/* repeat send cmd */
	msleep(100); /* wait iom3 finish handle config-data */
	atomic_set(&iom3_rec_state, IOM3_RECOVERY_DOING);
	pr_info("%s IOM3_RECOVERY_DOING\n", __func__);
	blocking_notifier_call_chain(&iom3_recovery_notifier_list, IOM3_RECOVERY_DOING, NULL);

	notify_modem_when_iomcu_recovery_finish();
	/* recovery pdr */
	blocking_notifier_call_chain(&iom3_recovery_notifier_list, IOM3_RECOVERY_3RD_DOING, NULL);
	pr_err("%s pdr recovery\n", __func__);
	atomic_set(&iom3_rec_state, IOM3_RECOVERY_IDLE);
	__pm_relax(g_iom3_rec_wl);
	pr_err("%s finish recovery\n", __func__);
	blocking_notifier_call_chain(&iom3_recovery_notifier_list, IOM3_RECOVERY_IDLE, NULL);
	pr_err("%s exit\n", __func__);
}

static void iomcu_reinit(void)
{
	write_ramdump_info_to_sharemem();
	write_timestamp_base_to_sharemem();
	msleep(5);
	atomic_set(&iom3_rec_state, IOM3_RECOVERY_MINISYS);
	blocking_notifier_call_chain(&iom3_recovery_notifier_list, IOM3_RECOVERY_MINISYS, NULL);
}

#ifdef CONFIG_CONTEXTHUB_LOAD_IOMCU_IMAGE
void iomcu_load_image(void)
{
	int ret;
	int count_of_try = 3; // try loading 3 times

	pr_info("%s enter\n", __func__);
	while (count_of_try-- > 0) {
		ret = load_and_verify_iomcu_image();  // load iom3 image
		if (ret == LOAD_IOMCU_OK) {
			break;
		}
		if (count_of_try == 0) {
			pr_err("%s: failed!\n", __func__);
			BUG_ON(1);
		}
	}
	pr_info("%s leave, left count_of_try is %d\n", __func__, count_of_try);
}
#endif
static void iom3_recovery_work(struct work_struct *work)
{
	int rec_nest_count = 0;
	int rc;
	u32 ack_buffer;
	u32 tx_buffer;

	pr_err("%s enter\n", __func__);
	__pm_stay_awake(g_iom3_rec_wl);
	wait_for_completion(&sensorhub_rdr_completion);

recovery_iom3:
	if (rec_nest_count++ > IOM3_REC_NEST_MAX) {
		pr_err("unlucky recovery iom3 times exceed limit\n");
		atomic_set(&iom3_rec_state, IOM3_RECOVERY_FAILED);
		blocking_notifier_call_chain(&iom3_recovery_notifier_list, IOM3_RECOVERY_FAILED, NULL);
		atomic_set(&iom3_rec_state, IOM3_RECOVERY_IDLE);
		__pm_relax(g_iom3_rec_wl);
		pr_err("%s exit\n", __func__);
		return;
	}

#ifdef CONFIG_IPC_V5
	ipc_shm_free_for_sh_crash();
#endif

	clear_nmi();

#ifdef CONFIG_DFX_DEBUG_FS
	show_iom3_stat();
#endif
	// reload iom3 system
	tx_buffer = RELOAD_IOM3_CMD;
	rc = RPROC_ASYNC_SEND(ipc_ap_to_lpm_mbx, &tx_buffer, 1);
	if (rc) {
		pr_err("RPROC reload iom3 failed %d, nest_count %d\n", rc, rec_nest_count);
		goto recovery_iom3;
	}

	iomcu_reinit();

#ifdef CONFIG_CONTEXTHUB_BOOT_STAT
	pr_err("%s:RELOAD_IOM3_CMD\n", __func__);
	(void)iomcu_show_boot_step();
#endif

#ifdef CONFIG_CONTEXTHUB_LOAD_IOMCU_IMAGE
	iomcu_load_image();
#endif

	/* startup iom3 system */
	reinit_completion(&iom3_rec_done);
	tx_buffer = STARTUP_IOM3_CMD;
	rc = RPROC_SYNC_SEND(ipc_ap_to_iom_mbx, &tx_buffer, 1, &ack_buffer, 1);
	if (rc) {
		pr_err("RPROC start iom3 failed %d, nest_count %d\n", rc, rec_nest_count);
		goto recovery_iom3;
	}

	pr_err("RPROC restart iom3 success\n");
#ifdef CONFIG_DFX_DEBUG_FS
	show_iom3_stat();
#endif
	/* dynamic loading */
	if (wait_for_completion_timeout(&iom3_rec_done, 5 * HZ) == 0) {
		pr_err("wait for iom3 system ready timeout\n");
		msleep(1000);
		goto recovery_iom3;
	}

#ifdef CONFIG_CONTEXTHUB_BOOT_STAT
	(void)iomcu_show_boot_step();
#endif

	iomcu_rdr_notifier();
}

static int __iom3_need_recovery(int modid, enum exp_source f, bool rdr_reset)
{
	int old_state;
	struct dump_config *dump = NULL;

	if (!g_sensorhub_dump_reset) {
		pr_info("%s: skip dump reset.\n", __func__);
		return 0;
	}

	old_state = atomic_cmpxchg(&iom3_rec_state, IOM3_RECOVERY_IDLE, IOM3_RECOVERY_START);
	pr_err("%s: recovery prev state %d, modid 0x%x, f %u, rdr_reset %u\n",
		__func__, old_state, modid, (uint8_t)f, (uint8_t)rdr_reset);

	/* prev state is IDLE start recovery progress */
	if (old_state == IOM3_RECOVERY_IDLE) {
		__pm_wakeup_event(g_iom3_rec_wl, jiffies_to_msecs(10 * HZ));
		blocking_notifier_call_chain(&iom3_recovery_notifier_list, IOM3_RECOVERY_START, NULL);

		dump = iomcu_config_get(IOMCU_CONFIG_USR_DUMP, sizeof(struct dump_config));
		if (f > SH_FAULT_INTERNELFAULT && dump != NULL)
			dump->reason = (uint8_t)f;

#ifndef CONFIG_CONTEXTHUB_BOOT_STAT
#ifdef CONFIG_DFX_BB
		if (rdr_reset)
			rdr_system_error((u32)modid, 0, 0);
#endif
#endif
		reinit_completion(&sensorhub_rdr_completion);
		send_nmi();
		atomic_set(&g_dump_minready_done, 0);
		notify_rdr_thread();

#ifdef CONFIG_CONTEXTHUB_BOOT_STAT
		if (!wait_for_completion_timeout(&sensorhub_rdr_completion, 3 * HZ))
			pr_err("%s:wait_for_completion_timeout err\n", __func__);
#ifdef CONFIG_DFX_BB
		if (rdr_reset)
			rdr_system_error((u32)modid, 0, 0);
#endif
#endif
		queue_delayed_work(g_iom3_rec_wq, &g_iom3_rec_work, 0);
	} else if ((f == SH_FAULT_INTERNELFAULT) && completion_done(&sensorhub_rdr_completion)) {
		__pm_relax(g_rdr_wl);
	}
	return 0;
}

int iom3_need_recovery(int modid, enum exp_source f)
{
	return __iom3_need_recovery(modid, f, true);
}

static int iom3_need_recovery_for_dump(int modid, enum exp_source f)
{
	return __iom3_need_recovery(modid, f, false);
}

static int shb_recovery_notifier(struct notifier_block *nb, unsigned long foo, void *bar)
{
	/* prevent access the emmc now: */
	pr_info("%s (%lu) +\n", __func__, foo);
	switch (foo) {
	case IOM3_RECOVERY_START:
		/* fall-through */
	case IOM3_RECOVERY_MINISYS:
		atomic_set(&g_iom3_state, IOM3_ST_RECOVERY);
		break;
	case IOM3_RECOVERY_DOING:
		/* fall-through */
	case IOM3_RECOVERY_3RD_DOING:
		atomic_set(&g_iom3_state, IOM3_ST_REPEAT);
		break;
	case IOM3_RECOVERY_FAILED:
		pr_err("%s -recovery failed\n", __func__);
		/* fall-through */
	case IOM3_RECOVERY_IDLE:
		atomic_set(&g_iom3_state, IOM3_ST_NORMAL);
		wake_up_all(&iom3_rec_waitq);
		break;
	default:
		pr_err("%s -unknow state %ld\n", __func__, foo);
		break;
	}
	pr_info("%s -\n", __func__);
	return 0;
}

static struct notifier_block recovery_notify = {
	.notifier_call = shb_recovery_notifier,
	.priority = -1,
};

static void iomcu_rdr_exit(void)
{
	wakeup_source_unregister(g_rdr_wl);

	(void)atomic_notifier_chain_unregister(&panic_notifier_list, &sensorhub_panic_block);
	if (g_sensorhub_wdt_irq != -1) {
		(void)free_irq((unsigned int)g_sensorhub_wdt_irq, NULL);
		g_sensorhub_wdt_irq = -1;
	}
	if (g_watchdog_base != NULL) {
		iounmap(g_watchdog_base);
		g_watchdog_base = NULL;
	}
	if (g_iomcu_ao_ns_ipc_base != NULL) {
		iounmap(g_iomcu_ao_ns_ipc_base);
		g_iomcu_ao_ns_ipc_base = NULL;
	}
	if (thread_savefile != NULL) {
		(void)kthread_stop(thread_savefile);
		thread_savefile = NULL;
	}
	if (thread_reco != NULL) {
		(void)kthread_stop(thread_reco);
		thread_reco = NULL;
	}
}

static int rdr_sensorhub_init(void)
{
	int ret = 0;

	if (rdr_sensorhub_init_early() != 0) {
		pr_err("rdr_sensorhub_init_early faild.\n");
		ret = -EINVAL;
	}

	sema_init(&g_rdr_sh_sem, 0);
	thread_savefile = kthread_run(rdr_sh_thread, NULL, "rdr_sh_thread");
	if (IS_ERR(thread_savefile)) {
		pr_err("create thread rdr_sh_main_thread faild.\n");
		ret = -EINVAL;
		return ret;
	}

	sema_init(&g_rdr_exce_sem, 0);
	thread_reco = kthread_run(rdr_exce_thread, NULL, "rdr_exce_thread");
	if (IS_ERR(thread_reco)) {
		kthread_stop(thread_savefile);
		thread_savefile = NULL;
		pr_err("create thread rdr_sh_exce_thread faild.\n");
		ret = -EINVAL;
		return ret;
	}
	if (get_sysctrl_base() != 0) {
		pr_err("get sysctrl addr faild.\n");
		goto RELEASE_THREAD;
	}
	if (get_watchdog_base() != 0) {
		pr_err("get watchdog addr faild.\n");
		goto RELEASE_THREAD;
	}

	if (get_iomcu_wdt_irq_num() < 0) {
		pr_err("%s g_sensorhub_wdt_irq get error!\n", __func__);
		goto UNMAP_WD_SYSCTRL;
	}

	if (get_nmi_offset() != 0) {
		pr_err("get_nmi_offset faild.\n");
		goto UNMAP_WD_SYSCTRL;
	}
	iomcu_get_mntn_sctrl_regs();

	if (request_irq((unsigned int)g_sensorhub_wdt_irq, watchdog_handler, 0, "watchdog", NULL)) {
		pr_err("%s failure requesting watchdog irq!\n", __func__);
		goto UNMAP_WD_SYSCTRL;
	}

	if (atomic_notifier_chain_register(&panic_notifier_list, &sensorhub_panic_block)) {
		pr_err("%s sensorhub panic register failed !\n", __func__);
		goto ERR_NOTIFIER;
	}
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(5, 4, 0))
	g_rdr_wl = wakeup_source_register(NULL,"rdr_sensorhub");
#else
	g_rdr_wl = wakeup_source_register("rdr_sensorhub");
#endif
	if (g_rdr_wl == NULL) {
		pr_err("%s rdr_sensorhub wakeup_source register failed !\n", __func__);
		goto ERR_WAKEUPSOURCE;
	}

	init_completion(&sensorhub_rdr_completion);
	return ret;
ERR_WAKEUPSOURCE:
	atomic_notifier_chain_unregister(&panic_notifier_list, &sensorhub_panic_block);
ERR_NOTIFIER:
	free_irq((unsigned int)g_sensorhub_wdt_irq, NULL);
	g_sensorhub_wdt_irq = -1;

UNMAP_WD_SYSCTRL:
	if (g_watchdog_base == NULL) {
		iounmap(g_watchdog_base);
		g_watchdog_base = NULL;
	}
RELEASE_THREAD:
	kthread_stop(thread_reco);
	thread_reco = NULL;
	kthread_stop(thread_savefile);
	thread_savefile = NULL;
	return -EINVAL;
}

#ifdef CONFIG_DFX_DEBUG_FS
#ifdef CONFIG_DFX_BB
static int sh_debugfs_u64_set(void *data, u64 val)
{
	int ret = 0;
	u64 mask_val;

	if (data == NULL || val == 0) {
		pr_warn("%s invalid para.\n", __func__);
		return -1;
	}

	mask_val = val & (RDR_AP | RDR_LPM3 | RDR_IOM3);

	pr_info("%s data=0x%llx, rdr mask=0x%llx, val=0x%llx, mask_val=0x%llx\n",
		__func__, *(u64 *)data, g_rdr_core_mask, val, mask_val);

	if (mask_val != g_rdr_core_mask) {
		*(u64 *)data = mask_val;
		sh_rdr_unregister();
		sh_rdr_register();
	}

	return ret;
}

static int sh_debugfs_u64_get(void *data, u64 *val)
{
	if (data == NULL || val == NULL) {
		pr_warn("%s invalid para.\n", __func__);
		return -1;
	}

	pr_info("%s data=0x%llx, rdr mask=0x%llx.\n", __func__, *(u64 *)data, g_rdr_core_mask);
	*val = *(u64 *)data;
	return 0;
}

DEFINE_DEBUGFS_ATTRIBUTE(sh_fops_x64, sh_debugfs_u64_get, sh_debugfs_u64_set, "0x%016llx\n");

static struct dentry *sh_debugfs_create_x64(const char *name, umode_t mode, struct dentry *parent, u64 *value)
{
	pr_info("%s enter.\n", __func__);
	return debugfs_create_file_unsafe(name, mode, parent, value, &sh_fops_x64);
}
#endif

static int rdr_create_file(void)
{
	struct dentry *ch_root = NULL;
	struct dentry *file_dentry = NULL;

	ch_root = debugfs_lookup("contexthub", NULL);
	if (IS_ERR_OR_NULL(ch_root)) {
		ch_root = debugfs_create_dir("contexthub", NULL);
		if (IS_ERR_OR_NULL(ch_root)) {
			pr_err("%s debugfs_create_dir contexthub failed!\n", __func__);
			ch_root = NULL;
			goto ERR_CREATE_DBGDIR;
		}
	} else {
		pr_info("%s dir contexthub contexthub is already exist\n", __func__);
	}

	/* 0660 : mode = S_IRUSR | S_IWUSR | S_IWGRP | S_IRGRP */
	file_dentry = debugfs_create_bool("dump_reset", 0660, ch_root, &g_sensorhub_dump_reset);
	if (IS_ERR_OR_NULL(file_dentry)) {
		pr_err("%s debugfs_create_file dump_reset failed\n", __func__);
		goto ERR_CREATE_DBGFS;
	}
#ifdef CONFIG_DFX_BB
	/* 0660 : mode = S_IRUSR | S_IWUSR | S_IWGRP | S_IRGRP */
	file_dentry = sh_debugfs_create_x64("rdr_core_mask", 0660, ch_root, &g_rdr_core_mask);
	if (IS_ERR_OR_NULL(file_dentry)) {
		pr_err("%s debugfs_create_file rdr_core_mask failed\n", __func__);
		goto ERR_CREATE_DBGFS;
	}
#endif
	return 0;

ERR_CREATE_DBGFS:
	if (ch_root != NULL)
		debugfs_remove_recursive(ch_root);

ERR_CREATE_DBGDIR:
	return -1;
}
#endif

int iomcu_check_dump_status(void)
{
	int rec_state;
	int minready;

	rec_state = atomic_read(&iom3_rec_state);
	minready = atomic_read(&g_dump_minready_done);
	if (rec_state == IOM3_RECOVERY_START || rec_state == IOM3_RECOVERY_FAILED || minready == 0) {
		pr_warn("%s iomcu is dumping [%d]minready[%d]\n", __func__, rec_state, minready);
		return -ESPIPE;
	}

	return 0;
}

static void iomcu_minready_done(void)
{
	atomic_set(&g_dump_minready_done, 1);
}

static void iomcu_dump_exit(void)
{
	iomcu_rdr_exit();
	wakeup_source_unregister(g_iom3_rec_wl);
	if (g_iom3_rec_wq != NULL) {
		destroy_workqueue(g_iom3_rec_wq);
		g_iom3_rec_wq = NULL;
	}
	(void)unregister_iom3_recovery_notifier(&recovery_notify);
}

static void notify_recovery_done(void)
{
	complete(&iom3_rec_done);
}

static int iomcu_dump_sys_callback(const struct pkt_header *head)
{
	iomcu_minready_done();

	pr_info("%s: rec state %x- %x\n", __func__, get_iom3_rec_state(), IOM3_RECOVERY_MINISYS);
	if (get_iom3_rec_state() == IOM3_RECOVERY_MINISYS) {
		pr_info("%s: status %x- %x\n", __func__, ((pkt_sys_statuschange_req_t *)head)->status, ST_MCUREADY);
		if (ST_MCUREADY == ((pkt_sys_statuschange_req_t *)head)->status) {
			pr_info("iomcu_dump mcu all ready!\n");
			/* notify dump that device cfg data had send */
			notify_recovery_done();
		}
	}

	return 0;
}

static void iomcu_dump_get_ao_ns_ipc_base(void)
{
	struct device_node *np = NULL;
	unsigned int i;
	int ret;

	np = of_find_compatible_node(NULL, NULL, "hisilicon,iomcu_ao_ns_ipc");
	if (!np) {
		pr_err("can not find iomcu_ao_ns_ipc node !\n");
		return;
	}

	ret = of_property_read_u32_array(np, "mailno", g_dump_mail_no, IOMCU_DUMP_IPC_MAILNO_COUNT);
	if (ret != 0) {
		pr_warn("[%s] read node property fail, ret[%d]!\n", __func__, ret);
		return;
	}

	for (i = 0; i < IOMCU_DUMP_IPC_MAILNO_COUNT; i++) {
		if (g_dump_mail_no[i] > IPMCU_DUMP_IPC_MAILNO_MAX) {
			pr_warn("[%s] invalid mailno[%u]!\n", __func__, g_dump_mail_no[i]);
			return;
		}
	}

	g_iomcu_ao_ns_ipc_base = of_iomap(np, 0);
	if (g_iomcu_ao_ns_ipc_base == NULL) {
		pr_err("get iomcu_ao_ns_ipc base error !\n");
		return;
	}
}

static int iomcu_dump_init(void)
{
	int ret;

	pr_info("[%s] ++", __func__);

	if (get_contexthub_dts_status())
		return -EACCES;
#ifdef CONFIG_SECURITY_HEADER_FILE_REPLACE
	if (get_iomcu_qic_mid(&iomcu_m7_mid, &iomcu_dma_mid))
		return -EACCES;
#endif
	ret = rdr_sensorhub_init();
	if (ret < 0) {
		pr_err("%s rdr_sensorhub_init ret=%d\n", __func__, ret);
		return -EINVAL;
	}
	atomic_set(&iom3_rec_state, IOM3_RECOVERY_IDLE);
	g_iom3_rec_wq = create_singlethread_workqueue("g_iom3_rec_wq");
	if (g_iom3_rec_wq == NULL) {
		pr_err("%s : faild create iom3 recovery workqueue!\n", __func__);
		goto ERR_FREE_RDR;
	}

	INIT_DELAYED_WORK((struct delayed_work *)(&g_iom3_rec_work), iom3_recovery_work);
	init_completion(&iom3_rec_done);
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(5, 4, 0))
	g_iom3_rec_wl = wakeup_source_register(NULL, "g_iom3_rec_wl");
#else
	g_iom3_rec_wl = wakeup_source_register("g_iom3_rec_wl");
#endif
	if (g_iom3_rec_wl == NULL) {
		pr_err("register g_iom3_rec_wl wakeup source faild\n");
		goto ERR_WAKEUPSOURCE;
	}

	init_waitqueue_head(&iom3_rec_waitq);
	ret = register_iom3_recovery_notifier(&recovery_notify);
	if (ret != 0) {
		pr_err("%s : register_iom3_recovery_notifier faild\n", __func__);
		goto ERR_NOTIFY;
	}
#ifdef CONFIG_DFX_DEBUG_FS
	ret = rdr_create_file();
	if (ret != 0) {
		pr_err("%s : register_iom3_recovery_notifier faild\n", __func__);
		goto ERR_RDR_CF;
	}
#endif

	ret = register_mcu_event_notifier(TAG_SYS, CMD_SYS_STATUSCHANGE_REQ,
					  iomcu_dump_sys_callback);
	if (ret != 0)
		pr_err("[%s], register sys status failed\n", __func__);

	iomcu_dump_get_ao_ns_ipc_base();
	pr_info("[%s] --", __func__);

	return 0;
#ifdef CONFIG_DFX_DEBUG_FS
ERR_RDR_CF:
	(void)unregister_iom3_recovery_notifier(&recovery_notify);
#endif
ERR_NOTIFY:
	wakeup_source_unregister(g_iom3_rec_wl);
ERR_WAKEUPSOURCE:
	if (g_iom3_rec_wq != NULL) {
		destroy_workqueue(g_iom3_rec_wq);
		g_iom3_rec_wq = NULL;
	}
ERR_FREE_RDR:
	iomcu_rdr_exit();
	return ret;
}

#ifndef CONFIG_CONTEXTHUB_DFX_UNI_NOC
int sensorhub_noc_notify(int value)
{
	union dump_info info;

	if (get_contexthub_dts_status() != 0) {
		pr_err("[%s]do not handle noc as sensorhub is disabled\n", __func__);
		return 0;
	}

	(void)value;
	pr_err("%s start\n", __func__);
	info.modid = SENSORHUB_MODID;
	save_dump_info_to_history(info);
	iom3_need_recovery(SENSORHUB_MODID, SH_FAULT_NOC);
	if (wait_for_completion_timeout(&sensorhub_rdr_completion, 3 * HZ) == 0)
		pr_err("%s:wait_for_completion_timeout err\n", __func__);
	pr_err("%s done\n", __func__);

	return 0;
}
#endif

void save_dump_info_to_history(union dump_info info)
{
	g_dump_info = info;
}

late_initcall(iomcu_dump_init);
module_exit(iomcu_dump_exit);
