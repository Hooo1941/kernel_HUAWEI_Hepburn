/*
 * QIC V100 err probe functions source file.
 *
 * Copyright (c) 2021-2023 Huawei Technologies Co., Ltd.
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 */
#include <linux/kernel.h>
#include <linux/io.h>
#include <linux/smp.h>
#include <linux/slab.h>
#include <linux/irqchip/arm-gic-v3.h>
#include "dfx_sec_qic_err_probe.h"
#include "dfx_sec_qic_init.h"
#include "dfx_sec_qic_modid_register.h"
#include "platform_include/see/bl31_smc.h"
#include "../common/sec_qic_share_info.h"
#include "platform_include/basicplatform/linux/pr_log.h"
#include "platform_include/basicplatform/linux/util.h"
#ifdef CONFIG_DFX_BB
#include "platform_include/basicplatform/linux/rdr_platform.h"
#include "platform_include/basicplatform/linux/rdr_pub.h"
#endif
#ifdef CONFIG_DFX_HW_DIAG
#include "platform_include/basicplatform/linux/dfx_hw_diag.h"
#endif
#include "mntn_public_interface.h"
#include "securec.h"
#define PR_LOG_TAG QIC_TAG

#define SIZE_4K 0x1000

#ifdef CONFIG_DFX_SEC_QIC_V100_MODID_REGISTER
static u32 g_qic_modid[QIC_MODID_NUM_MAX] =  { [0 ... (QIC_MODID_NUM_MAX - 1)] = QIC_MODID_NEGATIVE };
static u32 g_qic_modid_idx;
static u64 g_intr_status;
#endif

const char *g_dfx_sec_qic_err_resp[] = {
	"OKAY: normal access success or Exclusive fail",
	"EXOKAY: Exclusive access success",
	"SLVERR: Slave receive Error response or Decoder Error",
	"TMOERR: accessed slave timeout",
	"SECERR: Firewall intercpet and return SLVERR",
	"HIDEERR: Firewall intercpet and not return SLVERR",
	"DISCERR: access powerdown area",
	"UNSERR: received unsupported Opcode",
};

const char *g_dfx_sec_qic_safety_resp[] = {
	"Command in the Table has not been processed",
	"The QTP command channel is back pressed",
	"Current AXI command is not processed continuously at the entry",
	"AXI's R or B channels are back pressed",
	"Not support to parse",
	"Not support to parse",
	"Current Axi write data is not collected",
	"Not support to parse",
};

static void __iomem *qice_m1_p0;
static void __iomem *qice_m2_p0;
static void __iomem *qice_m2_p1;
static void __iomem *qice_qdbem_wrap_P0;
static void __iomem *qice_qdbem_wrap_P1;

void qice_regs_print_init(void)
{
	struct device_node *np = NULL;
	int ret;
	u32 qice_base;

	np = of_find_compatible_node(NULL, NULL, "hisilicon,qice_address");
	if (!np) {
		pr_err("%s: no compatible qice address node found\n", __func__);
		return;
	}

	ret = of_property_read_u32(np, "base_addr", &qice_base);
	if (ret) {
		pr_err("%s , get qice base addr err\n", __func__);
		return;
	}

	qice_m1_p0 = ioremap(qice_base + SOC_QICE_IBQE_MEDIA1_P0_BASE_ADDR, SIZE_4K);
	qice_m2_p0 = ioremap(qice_base + SOC_QICE_IBQE_MEDIA2_P0_BASE_ADDR, SIZE_4K);
	qice_m2_p1 = ioremap(qice_base + SOC_QICE_IBQE_MEDIA2_P1_BASE_ADDR, SIZE_4K);
	qice_qdbem_wrap_P0 = ioremap(qice_base + SOC_QICE_IBQE_QDBEM_WRAP_P0_BASE_ADDR, SIZE_4K);
	qice_qdbem_wrap_P1 = ioremap(qice_base + SOC_QICE_IBQE_QDBEM_WRAP_P2_BASE_ADDR, SIZE_4K);
}

static void _qice_regs_print(const void __iomem *base)
{
	if (!base)
		return;

	pr_err("\tintr: 0x%x, info0-info2: 0x%x 0x%x 0x%x\n",
			readl(SOC_QICE_IBQE_ABNML_NS_INT_STATUS1_ADDR(base)),
			readl(SOC_QICE_IBQE_ABNML_NS_INT_INFO0_ADDR(base)),
			readl(SOC_QICE_IBQE_ABNML_NS_INT_INFO1_ADDR(base)),
			readl(SOC_QICE_IBQE_ABNML_NS_INT_INFO2_ADDR(base)));
}

static void qice_regs_print(void)
{
	pr_err("media1 p0 qice ibqe regs:\n");
	_qice_regs_print(qice_m1_p0);

	pr_err("media2 p0 qice ibqe regs:\n");
	_qice_regs_print(qice_m2_p0);

	pr_err("media2 p1 qice ibqe regs:\n");
	_qice_regs_print(qice_m2_p1);

	pr_err("npu p0 qice ibqe regs:\n");
	_qice_regs_print(qice_qdbem_wrap_P0);

	pr_err("npu p1 qice ibqe regs:\n");
	_qice_regs_print(qice_qdbem_wrap_P1);
}

static void dfx_sec_qic_print_dfx_info(const struct sec_qic_dfx_info *dfx_info)
{
	pr_err("core_id is:0x%x\n", dfx_info->coreid);
	pr_err("irq_status is:0x%x\n", dfx_info->irq_status);
	pr_err("bus_key is:0x%x\n", dfx_info->bus_key);
	pr_err("ib_type is:0x%x\n", dfx_info->ib_type);
	pr_err("ib_dfx_low is:0x%llx\n", dfx_info->ib_dfx_low);
	pr_err("ib_dfx_high is:0x%llx\n", dfx_info->ib_dfx_high);
	if (dfx_info->ib_name[0])
		pr_err("ib_name is:%s\n", dfx_info->ib_name);
	else
		pr_err("no ib error on this bus\n");
	if (dfx_info->tb_name[0])
		pr_err("tb_name is:%s\n", dfx_info->tb_name);
	else
		pr_err("no tb error on this bus\n");
}

static struct qic_bus_info* dfx_sec_qic_match_bus_info(u32 bus_key, const struct dfx_sec_qic_device *qic_dev)
{
	u32 i;

	for (i = 0; i < qic_dev->bus_info_num; i++) {
		if (bus_key == qic_dev->bus_info[i].bus_key)
			return &(qic_dev->bus_info[i]);
	}

	return NULL;
}

static void dfx_sec_qic_parse_ib_info_with_type(const struct sec_qic_dfx_info *dfx_info, u32 type, u32 *opc_type)
{
	u32 rw_opc;
	u32 qtp_rsp;
	const char *rsp_msg = NULL;

	/* bit[120:118]:if it is safety irq, use safety rsp */
	qtp_rsp = (u32)((dfx_info->ib_dfx_high >> DFX_SEC_QIC_QTP_RSP_OFFSET) & DFX_SEC_QIC_QTP_RSP_MASK);
	if (type == DFX_SEC_QIC_INT_NORMAL_TYPE) {
		rsp_msg = g_dfx_sec_qic_err_resp[qtp_rsp];
		rw_opc = ((dfx_info->irq_status >> DFX_SEC_QIC_RW_NORMAL_OFFSET) & DFX_SEC_QIC_RW_MASK);
	} else if (type == DFX_SEC_QIC_INT_SAFETY_TYPE) {
		rsp_msg = g_dfx_sec_qic_safety_resp[qtp_rsp];
		rw_opc = ((dfx_info->irq_status >> DFX_SEC_QIC_RW_SAFETY_OFFSET) & DFX_SEC_QIC_RW_MASK);
	} else {
		rsp_msg = "Not support to parse";
		rw_opc = DFX_SEC_QIC_RW_ERROR;
	}
	pr_err("qtp_rsp = 0x%x,%s\n", qtp_rsp, rsp_msg);
	*opc_type = rw_opc;
	if (rw_opc == DFX_SEC_QIC_WRITE_OPERATION)
		pr_err("opreation =%u: write operation\n", rw_opc);
	else if (rw_opc == DFX_SEC_QIC_READ_OPERATION)
		pr_err("opreation =%u: read operation\n", rw_opc);
	else
		pr_err("opreation =%u: read/write operation parse error\n", rw_opc);
}

static const char *dfx_sec_qic_get_mid_msg(u32 mid, const struct qic_master_info *master_info, u32 master_num)
{
	u32 i;

	for (i = 0; i < master_num; i++) {
		if (mid == master_info[i].masterid_value)
			return master_info[i].masterid_name;
	}

	return NULL;
}

static void dfx_sec_qic_parse_master_id(u32 mid, const struct sec_qic_dfx_info *dfx_info,
					const struct dfx_sec_qic_device *qic_dev)
{
	const char *mid_msg = NULL;
	const char *acpu_msg = NULL;
	u32 acpu_coreid;

	mid_msg = dfx_sec_qic_get_mid_msg(mid, qic_dev->mid_info, qic_dev->mid_info_num);
	if (mid_msg)
		pr_err("MID(Master id) = %u, means:%s\n", mid, mid_msg);
	else
		pr_err("MID(Master id) = %u, master name mismatch\n", mid);

	if (dfx_info->coreid == DFX_SEC_QIC_FCM_PERI_PORT) {
		acpu_coreid = (u32)(dfx_info->ib_dfx_low >> DFX_SEC_QIC_ACPU_COREID_OFFSET) &
				   DFX_SEC_QIC_ACPU_COREID_MASK;
		acpu_msg = dfx_sec_qic_get_mid_msg(acpu_coreid, qic_dev->acpu_core_info, qic_dev->acpu_core_info_num);
		if (acpu_msg)
			pr_err("FCM_PERI_PORT err, acpu coreid is:0x%x, means:%s\n", acpu_coreid, acpu_msg);
		else
			pr_err("FCM_PERI_PORT err, acpu coreid is:0x%x, core name mismatch\n", acpu_coreid);
	}
}

#ifdef CONFIG_DFX_SEC_QIC_V100_MODID_REGISTER
static u64 get_qic_interrupts_status(const struct dfx_sec_qic_device *qic_dev)
{
	u64 value = 0;
	u32 i;
	u32 j;

	for (i = 0; i < qic_dev->irq_reg_num; i++) {
		for (j = 0; j < qic_dev->irq_reg[i].len; j++) {
				value |= readl_relaxed(qic_dev->irq_reg[i].reg_base +
					qic_dev->irq_reg[i].offsets[j]);
				pr_err("qic irq status:0x%llx\n", value);
			}
	}

	return value;
}

static void gic_irq_set_irqchip_state(u32 irq_number, void __iomem *gic_ctrl_base, u32 offset)
{
	u32 mask = 1 << (irq_number % 32);
	writel_relaxed(mask, (u8 __iomem *)gic_ctrl_base + offset + (irq_number / 32) * 4);
}

static u32 gic_irq_get_irqchip_state(u32 irq_number, void __iomem *gic_ctrl_base, u32 offset)
{
	return readl_relaxed((u8 __iomem *)gic_ctrl_base + offset + (irq_number/ 32) * 4);
}

static void save_bus_timeout_status(const struct sec_qic_dfx_info *dfx_info)
{
	if ((dfx_info->irq_status & QIC_HAVE_TB0_TIMEOUT_INTR) && (dfx_info->bus_key == QIC_SYS_BUS_KEY))
		g_intr_status |= QIC_HAVE_TB0_TIMEOUT_INTR;
}

static bool is_timeout_irq_happen(void)
{
	return (g_intr_status & QIC_HAVE_SYS_BUS_INTR) && (g_intr_status & QIC_HAVE_TB0_TIMEOUT_INTR);
}

/*
 * The unclearable timeout_irq(tb0:sys2npu) on sys_bus causes an interruption storm and the subsystem cannot save logs.
 * The qic interrupt is disabled through the GIC.
 */
static void gic_disable_qic_irq(u32 modid_match_flag)
{
	u32 value;
	struct dfx_sec_qic_device *qic_dev = dfx_sec_qic_get_dev();

	if (modid_match_flag == 0)
		return;

	if (is_timeout_irq_happen()) {
	/* GICD_ISENABLER:irq clear register GICD_ISENABLER:irq enable register */
		value = gic_irq_get_irqchip_state(QIC_SOFTWARE_IRQ_NUMBER, qic_dev->gic_ctrl_base, GICD_ISENABLER);
		pr_err("gicd_13group defalut value = 0x%x qic_irq_state = 0x%x\n", value, value & QIC_GICD_QIC_BIT_MASK);
		gic_irq_set_irqchip_state(QIC_SOFTWARE_IRQ_NUMBER, qic_dev->gic_ctrl_base, GICD_ICENABLER);
		value = gic_irq_get_irqchip_state(QIC_SOFTWARE_IRQ_NUMBER, qic_dev->gic_ctrl_base, GICD_ISENABLER);
		pr_err("gicd_13group disable qic value = 0x%x qic_irq_state = 0x%x\n ", value, value & QIC_GICD_QIC_BIT_MASK);
	}
}

/*
 * unclearable timeout_irq(tb0:sys2npu) on sys_bus
 * The subsystem is about to be reset.
 */
static void register_ap_s_noc_modid(const struct dfx_sec_qic_device *qic_dev)
{
	bool timeout_irq_status = false;

	g_intr_status |= get_qic_interrupts_status(qic_dev);
	timeout_irq_status = is_timeout_irq_happen();
	if (timeout_irq_status && (g_qic_modid_idx > 0) && (g_qic_modid_idx < QIC_MODID_NUM_MAX)) {
		g_qic_modid[g_qic_modid_idx++] = MODID_AP_S_NOC;
		pr_err("register AP_S_NOC to qic_modid array!\n");
	}
}
#endif

static void dfx_sec_qic_parse_dfx_info(const struct sec_qic_dfx_info *dfx_info,
					const struct dfx_sec_qic_device *qic_dev)
{
	u64 qic_address;
	u32 qtp_sf;
	u32 mid;
	u32 type;
	u32 opc_type;
#ifdef CONFIG_DFX_SEC_QIC_V100_MODID_REGISTER
	u32 modid = QIC_MODID_NEGATIVE;
	struct dfx_errlog_info errlog_info;
#endif

	struct qic_bus_info* bus_info= dfx_sec_qic_match_bus_info(dfx_info->bus_key, qic_dev);
	if (!bus_info) {
		pr_err("get bus info fail\n");
		return;
	}

#ifdef CONFIG_DFX_SEC_QIC_V100_MODID_REGISTER
	save_bus_timeout_status(dfx_info);
#endif

	type = dfx_info->irq_status & DFX_SEC_QIC_INT_STATUS_MASK;
	if (!type) {
		pr_err("ib err probe not happen, no need to parse\n");
		return;
	}
	dfx_sec_qic_parse_ib_info_with_type(dfx_info, type, &opc_type);

	/* bit[84:47]:qtp addr */
	if (dfx_info->ib_type) {
		qic_address = dfx_sec_qic_parse_addr(dfx_info->ib_dfx_high, dfx_info->ib_dfx_low);
		pr_err("qic_addr = 0x%llx\n", qic_address);
	} else {
		pr_err("qic_addr not support, please check dmss/qice info\n");
	}
	/* bit[46:41]:only use last bit */
	qtp_sf = ((u32)(dfx_info->ib_dfx_low >> DFX_SEC_QIC_QTP_SF_OFFSET) & DFX_SEC_QIC_QTP_SF_MASK);
	if (qtp_sf == 1)
		pr_err("sf = %u: no Security access\n", qtp_sf);
	else
		pr_err("sf = %u: Security access\n", qtp_sf);

	mid = (u32)(dfx_info->ib_dfx_low >> (bus_info->mid_offset)) & DFX_SEC_QIC_MID_MASK;
	dfx_sec_qic_parse_master_id(mid, dfx_info, qic_dev);

#ifdef CONFIG_DFX_SEC_QIC_V100_MODID_REGISTER
	modid = dfx_qic_modid_find(dfx_info->coreid, mid);
	if ((modid != QIC_MODID_NEGATIVE) && (g_qic_modid_idx < QIC_MODID_NUM_MAX))
		g_qic_modid[g_qic_modid_idx++] = modid;

	errlog_info.err_addr = (dfx_info->ib_type == 0 ? DFX_SEC_ERR_ADDR_ERROR : qic_address);
	errlog_info.master_id = mid;
	errlog_info.opc_type = opc_type;
	errlog_info.safe_flag = qtp_sf;
	dfx_qic_set_errlog_info(&errlog_info);
#endif
}

static void dfx_sec_qic_parse_process(const struct dfx_sec_qic_device *qic_dev)
{
	u64 *dfx_info_num = qic_dev->qic_share_base;
	struct sec_qic_dfx_info *dfx_info = NULL;
	u32 i;
	struct qic_bus_info *bus_info = NULL;

	if (!dfx_info_num || *dfx_info_num > MAX_QIC_DFX_INFO) {
		pr_err("[%s] share data error!\n", __func__);
		return;
	}

	dfx_info = (struct sec_qic_dfx_info *)(dfx_info_num + 1);

	pr_err("start to show qic error info!\n");
	for (i = 0; i < *dfx_info_num; i++) {
		bus_info = dfx_sec_qic_match_bus_info((dfx_info + i)->bus_key, qic_dev);
		if (!bus_info) {
			pr_err("get bus info fail\n");
			return;
		}
		pr_err("******qic_err occurred on %s******\n", bus_info->bus_name);
		dfx_sec_qic_print_dfx_info(dfx_info + i);
		dfx_sec_qic_parse_dfx_info(dfx_info + i, qic_dev);
	}
	pr_err("end to show qic error info!\n");
#ifdef CONFIG_DFX_SEC_QIC_V100_MODID_REGISTER
	register_ap_s_noc_modid(qic_dev);
#endif
}

static bool dfx_sec_qic_need_stop_acpu(u32 bus_key, const struct dfx_sec_qic_device *qic_dev)
{
	u32 i;

	for (i = 0; i < qic_dev->need_stop_cpu_buses_num; i++) {
		if (bus_key == qic_dev->need_stop_cpu_buses[i])
			return true;
	}

	return false;
}

static void dfx_sec_qic_err_smp_stop_acpu(u64 bus_key, const struct dfx_sec_qic_device *qic_dev)
{
	if (check_mntn_switch(MNTN_NOC_ERROR_REBOOT) && dfx_sec_qic_need_stop_acpu((u32)bus_key, qic_dev)) {
		/* Change Consloe Log Level. */
		console_verbose();
		/* Stop Other CPUs. */
		smp_send_stop();
		/* Debug Print. */
		pr_err("%s -- Stop CPUs First.\n", __func__);
	}
}

#define MSPC_MIN_LPRAM_ADDR  0xFFFFE7C0
#define MSPC_MAX_LPRAM_ADDR  0xFFFFE7D4
static bool iomcu_qic_ignore(void)
{
	struct dfx_errlog_info errlog_info = {0};

	dfx_qic_get_errlog_info(&errlog_info);
	if (errlog_info.master_id == SOC_SEC_HASH_MASTER_MID &&
		errlog_info.err_addr >= MSPC_MIN_LPRAM_ADDR &&
		errlog_info.err_addr < MSPC_MAX_LPRAM_ADDR) {
		pr_err("%s, err_addr 0x%x\n", __func__, errlog_info.err_addr);
		return true;
	}

	return false;
}
void dfx_sec_qic_reset_handler(void)
{
	u32 modid_match_flag = 0;
#ifdef CONFIG_DFX_SEC_QIC_V100_MODID_REGISTER
	u32 i;
#endif
	bool ignore = false;

	ignore = iomcu_qic_ignore();
	if (ignore)
		return;

	if (check_mntn_switch(MNTN_NOC_ERROR_REBOOT)) {
#ifdef CONFIG_DFX_SEC_QIC_V100_MODID_REGISTER
		for (i = 0; i < g_qic_modid_idx; i++) {
			if (g_qic_modid[i] != QIC_MODID_NEGATIVE) {
				modid_match_flag = 1;
				pr_err("[%s] qic modid is matched, modid = 0x%x!\n", __func__, g_qic_modid[i]);
				rdr_system_error(g_qic_modid[i], 0, 0);
			}
		}
		gic_disable_qic_irq(modid_match_flag);
#endif
		if (modid_match_flag == 0) {
			pr_err("%s qic reset begin\n", __func__);
			rdr_syserr_process_for_ap(MODID_AP_S_NOC, 0, 0);
		}
	}
}

static void dfx_sec_clear_share_mem(const struct dfx_sec_qic_device *qic_dev)
{
	static u32 clr_flag = 0;

	if (clr_flag != 0)
		(void)memset_s(qic_dev->qic_share_base, QIC_SHARE_MEM_SIZE, 0, QIC_SHARE_MEM_SIZE);
	clr_flag++;
}

#define QIC_BUS_KEY_ALL		0xFF
int dfx_sec_qic_get_errinfo(const struct dfx_sec_qic_device *qic_dev, bool from_irq)
{
	ulong position;
	ulong value;
	u32 i;
	u32 j;
#ifdef CONFIG_DFX_HW_DIAG
	union dfx_hw_diag_info hwdiag_qic_info;
#endif

	if (!qic_dev) {
		pr_err("[%s] qic_init fail!\n", __func__);
		return -1;
	}

#ifdef CONFIG_DFX_SEC_QIC_V100_MODID_REGISTER
	g_qic_modid_idx = 0;
#endif
	dfx_sec_clear_share_mem(qic_dev);
	for (i = 0; i < qic_dev->irq_reg_num; i++) {
		for (j = 0; j < qic_dev->irq_reg[i].len; j++) {
			value = readl_relaxed(qic_dev->irq_reg[i].reg_base +
				qic_dev->irq_reg[i].offsets[j]);
			pr_err("qic irq status:0x%lx\n", value);
			value = value & qic_dev->irq_reg[i].reg_mask[j];
			if (from_irq) {
#ifdef CONFIG_DFX_HW_DIAG
				hwdiag_qic_info.qic_info.buskey = value;
				dfx_hw_diaginfo_trace(QIC_FAULT_INFO, &hwdiag_qic_info);
#endif
				if (value == 0) {
					qice_regs_print();
					dfx_sec_qic_smc_call(QIC_SECIRQ_FID_SUBTYPE, QIC_BUS_KEY_ALL);
				}
			}
			for_each_set_bit(position, &value, BITS_PER_LONG) {
				pr_err("irq bit:%lu\n", position);
				dfx_sec_qic_err_smp_stop_acpu(position, qic_dev);
				(void)dfx_sec_qic_smc_call(QIC_SECIRQ_FID_SUBTYPE, position + j * BITS_PER_TYPE(u32)
					+ i * qic_dev->irq_reg[i].len * BITS_PER_TYPE(u32));
			}
		}
	}
	dfx_sec_qic_parse_process(qic_dev);
	return 0;
}
