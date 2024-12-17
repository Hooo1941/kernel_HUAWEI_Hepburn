/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2021-2021. All rights reserved.
 * Description: Contexthub dump boot state process.
 * Create: 2021-12-20
 */

#include "iomcu_dump_priv.h"
#include <platform_include/smart/linux/iomcu_status.h>
#if defined (CONFIG_CONTEXTHUB_BOOT_STAT) && !defined(CONFIG_SECURITY_HEADER_FILE_REPLACE)
#include <soc_sctrl_interface.h>
#define BOOT_STAT_REG_OFFSET SOC_SCTRL_SCBAKDATA27_ADDR(0)
#endif
#ifdef CONFIG_SECURITY_HEADER_FILE_REPLACE
#include <internal_security_interface.h>
#endif
#include <platform_include/see/bl31_smc.h>
#include <linux/arm-smccc.h>
#ifdef CONFIG_CONTEXTHUB_TZSP
#include <platform_include/see/ffa/ffa_plat_drv.h>
#endif

static void __iomem *g_sysctrl_base = NULL;
static int g_nmi_reg;
#ifdef CONFIG_CONTEXTHUB_BOOT_STAT
static int boot_stat_reg_offset;
#endif
static unsigned int g_iomcu_mntn_enable;
static unsigned int g_iomcu_stat_reg;
static unsigned int g_sc_intr_wakeup_reg;

struct iomcu_reg_info {
	char reg_name[12];
	u32 value;
};

enum {
	IOMCU_REG_CLK_SEL,
	IOMCU_REG_DIV0,
	IOMCU_REG_DIV1,
	IOMCU_REG_CLKSTAT0,
	IOMCU_REG_RSTSTAT0,
	IOMCU_REG_CLKSTAT1,
	IOMCU_REG_M7_STAT0,
	IOMCU_REG_M7_STAT1,
	IOMCU_REG_WINDOW0,
	IOMCU_REG_WINDOW1,
	IOMCU_REG_SYS_STAT,
	IOMCU_REG_SYS_CONFIG,
	IOMCU_REG_MISC_CTRL,
	IOMCU_REG_DBG_INFO1,
	IOMCU_REG_DBG_INFO2,
	IOMCU_REG_RESERVED_REG0,
	IOMCU_REG_RESERVED_REG1
};

static struct iomcu_reg_info g_iomcu_regs[] = {
	{ "clk_sel", 0 },
	{ "div0", 0 },
	{ "div1", 0 },
	{ "clk_stat0", 0 },
	{ "rst_stat0", 0 },
	{ "clk_stat1", 0 },
	{ "m7_stat0", 0 },
	{ "m7_stat1", 0 },
	{ "m7_win0", 0 },
	{ "m7_win1", 0 },
	{ "sys_stat", 0 },
	{ "sys_cfg", 0 },
	{ "misc_ctrl", 0 },
	{ "dbg_info1", 0 },
	{ "dbg_info2", 0 } };

int get_sysctrl_base(void)
{
	struct device_node *np = NULL;

	if (g_sysctrl_base == NULL) {
		np = of_find_compatible_node(NULL, NULL, "hisilicon,sysctrl");
		if (np == NULL) {
			pr_err("can not find  sysctrl node !\n");
			return -1;
		}

		g_sysctrl_base = of_iomap(np, 0);
		if (g_sysctrl_base == NULL) {
			pr_err("get g_sysctrl_base  error !\n");
			return -1;
		}
	}
	return 0;
}

int get_nmi_offset(void)
{
	struct device_node *sh_node = NULL;

	sh_node = of_find_compatible_node(NULL, NULL, "huawei,sensorhub_nmi");
	if (!sh_node) {
		pr_err("%s, can not find node sensorhub_nmi \n", __func__);
		return -1;
	}

	if (of_property_read_u32(sh_node, "nmi_reg", &g_nmi_reg)) {
		pr_err("%s:read nmi reg err:value is %d\n", __func__, g_nmi_reg);
		return -1;
	}

	pr_info("%s arch nmi reg is 0x%x\n", __func__, g_nmi_reg);
	return 0;
}

void iomcu_get_mntn_sctrl_regs(void)
{
	struct device_node *sh_node = NULL;
	const char *status = NULL;
	int len = 0;

	sh_node = of_find_compatible_node(NULL, NULL, "hisilicon,contexthub_mntn");
	if (!sh_node) {
		pr_err("%s, can not find node iomcu_mntn \n", __func__);
		return;
	}

	status = of_get_property(sh_node, "status", &len);
	if (status == NULL) {
		pr_err("%s, of_get_property status err\n", __func__);
		return;
	}
	if (strstr(status, "ok") == NULL) {
		pr_info("%s, disabled\n", __func__);
		return;
	}

	g_iomcu_mntn_enable = 1;
	if (of_property_read_u32(sh_node, "iomcu_stat_reg", &g_iomcu_stat_reg)) {
		pr_err("%s:read iomcu stat reg err:value is 0x%x\n", __func__, g_iomcu_stat_reg);
		return;
	}
	if (of_property_read_u32(sh_node, "sc_intr_wakeup_reg", &g_sc_intr_wakeup_reg)) {
		pr_err("%s:read sc intr wakeup reg err:value is 0x%x\n", __func__, g_sc_intr_wakeup_reg);
		return;
	}

	pr_info("%s mntn reg 0x%x, 0x%x\n", __func__, g_iomcu_stat_reg, g_sc_intr_wakeup_reg);
}

static void iomcu_dump_mntn_regs(void)
{
	unsigned int iomcu_stat = 0;
	unsigned int sc_intr_wakeup = 0;

	if (g_iomcu_stat_reg != 0)
		iomcu_stat = readl(g_sysctrl_base + g_iomcu_stat_reg);
	if (g_sc_intr_wakeup_reg != 0)
		sc_intr_wakeup = readl(g_sysctrl_base + g_sc_intr_wakeup_reg);

	pr_err("%s: iomcu_stat 0x%x, sc_intr_wakeup 0x%x\n", __func__, iomcu_stat, sc_intr_wakeup);
}

#ifdef CONFIG_DFX_DEBUG_FS
void show_iom3_stat(void)
{
	unsigned int i;
#ifdef CONFIG_CONTEXTHUB_TZSP
	int ret;
	struct ffa_send_direct_data args = {0};
	args.data0 = BL31_IOMCU_READ_RESERVED_REGS;
#else
	struct arm_smccc_res res = {0};
#endif

	if (g_iomcu_mntn_enable == 0)
		return;

	for (i = IOMCU_REG_CLK_SEL; i <= IOMCU_REG_DBG_INFO2; i++) {
#ifdef CONFIG_CONTEXTHUB_TZSP
		args.data1 = i;
		ret = ffa_platdrv_send_msg(&args);
		if (ret != 0) {
			pr_err("%s: failed to send_msg! ret = %d\n",__func__, ret);
			return;
		}
		g_iomcu_regs[i].value = (u32)args.data0;
#else
		arm_smccc_smc(BL31_IOMCU_READ_RESERVED_REGS, i, 0, 0, 0, 0, 0, 0, &res);
		g_iomcu_regs[i].value = (u32)(res.a0);
#endif
	}

	pr_err("%s:\n", __func__);
	for (i = 0; i < 5; i++)
		pr_err("%s 0x%x, %s 0x%x, %s 0x%x\n",
			g_iomcu_regs[i * 3].reg_name, g_iomcu_regs[i * 3].value,
			g_iomcu_regs[i * 3 + 1].reg_name, g_iomcu_regs[i * 3 + 1].value,
			g_iomcu_regs[i * 3 + 2].reg_name, g_iomcu_regs[i * 3 + 2].value);

	iomcu_dump_mntn_regs();
}
#endif

#ifdef CONFIG_CONTEXTHUB_BOOT_STAT
#ifdef CONFIG_DFX_DEBUG_FS
static void get_boot_stat_from_iomcu_reg(void)
{
	unsigned int step0 = 0;
	unsigned int step1 = 0;
#ifdef CONFIG_CONTEXTHUB_TZSP
	int ret;
	struct ffa_send_direct_data args = {0};

	args.data0 = BL31_IOMCU_READ_RESERVED_REGS;
	args.data1 = IOMCU_REG_RESERVED_REG0;
	ret = ffa_platdrv_send_msg(&args);
	if (ret != 0) {
		pr_err("%s: failed to send_msg! ret = %d\n",__func__, ret);
		return;
	}
	step0 = (unsigned int)args.data0;

	args.data0 = BL31_IOMCU_READ_RESERVED_REGS;
	args.data1 = IOMCU_REG_RESERVED_REG1;
	ret = ffa_platdrv_send_msg(&args);
	if (ret != 0) {
		pr_err("%s: failed to send_msg! ret = %d\n",__func__, ret);
		return;
	}
	step1 = (unsigned int)args.data0;
#else
	struct arm_smccc_res res = {0};

	arm_smccc_smc(BL31_IOMCU_READ_RESERVED_REGS, IOMCU_REG_RESERVED_REG0, 0, 0, 0, 0, 0, 0, &res);
	step0 = res.a0;
	arm_smccc_smc(BL31_IOMCU_READ_RESERVED_REGS, IOMCU_REG_RESERVED_REG1, 0, 0, 0, 0, 0, 0, &res);
	step1 = res.a0;
#endif
	pr_err("%s: sensorhub reserved0=0x%x, reserved1=0x%x\n", __func__, step0, step1);
}
#endif

union sctrl_sh_boot iomcu_show_boot_step(void)
{
	union sctrl_sh_boot sh_boot;

	if (g_sysctrl_base != NULL)
		sh_boot.value = readl(g_sysctrl_base + boot_stat_reg_offset);
	else
		sh_boot.value = 0;
	pr_info("%s: sensorhub sh_boot=0x%x, boot_step=%x, boot_stat=%x\n",
		__func__, sh_boot.value, sh_boot.reg.boot_step, sh_boot.reg.boot_stat);
	return sh_boot;
}

union sctrl_sh_boot get_boot_stat(void)
{
	union sctrl_sh_boot sh_boot;
#ifdef CONFIG_SECURITY_HEADER_FILE_REPLACE
	boot_stat_reg_offset = (int)get_contexhub_boot_stat();
#else
	boot_stat_reg_offset = BOOT_STAT_REG_OFFSET;
#endif

	sh_boot = iomcu_show_boot_step();
#ifdef CONFIG_DFX_DEBUG_FS
	get_boot_stat_from_iomcu_reg();
#endif
	return sh_boot;
}

void reset_boot_stat(void)
{
#ifdef CONFIG_SECURITY_HEADER_FILE_REPLACE
	boot_stat_reg_offset = (int)get_contexhub_boot_stat();
#else
	boot_stat_reg_offset = BOOT_STAT_REG_OFFSET;
#endif
	if (g_sysctrl_base != NULL)
		writel(0, g_sysctrl_base + boot_stat_reg_offset);
}
#endif

void send_nmi(void)
{
#ifdef CONFIG_CONTEXTHUB_BOOT_STAT
	(void)get_boot_stat();
#endif
	pr_warn("%s:nmi_reg is 0x%x\n", __func__, g_nmi_reg);
	if (g_sysctrl_base != NULL && g_nmi_reg != 0) {
		writel(0x2, g_sysctrl_base + g_nmi_reg); /* 0x2 only here */
		pr_err("IOMCU NMI\n");
	}
}

void clear_nmi(void)
{
	/* fix bug nmi can't be clear by iomcu, or iomcu will not start correctly */
	if ((unsigned int)readl(g_sysctrl_base + g_nmi_reg) & 0x2)
		pr_err("%s nmi remain!\n", __func__);
	writel(0, g_sysctrl_base + g_nmi_reg);
}
