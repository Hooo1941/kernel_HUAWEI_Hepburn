/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2015-2020. All rights reserved.
 * Description: show ddr info
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
#include <linux/errno.h>
#include <linux/init.h>
#include <linux/io.h>
#include <linux/ioport.h>
#include <linux/kernel.h>
#include <linux/mm.h>
#include <linux/module.h>
#include <linux/of.h>
#include <linux/printk.h>
#include <linux/proc_fs.h>
#include <linux/seq_file.h>
#include <linux/slab.h>
#include <linux/stat.h>
#include <linux/string.h>
#include <linux/types.h>
#include <securec.h>

#ifdef DDR_CHARLOTTE_PLATFORM
#define DDR_INFO_ADDR 0xFB21B428
#elif defined DDR_BURBANK_PLATFORM
#define DDR_INFO_ADDR 0xFA89B428
#else
#include "soc_acpu_baseaddr_interface.h"
#include "soc_sctrl_interface.h"
#define DDR_INFO_ADDR (SOC_SCTRL_SCBAKDATA7_ADDR(SOC_ACPU_SCTRL_BASE_ADDR))
#endif

#define DDR_MANUFID_MAX_LEN 16
#define DDR_INFO_MASK 0x100

/*
 * DDR_INFO_ADDR is the physical address of the register for storing
 * DDR manufacturer id and memory size.the register vlaue is conveyed
 * from xloader to Linux kernel,defined in "xloader/ddr/ddr_asic.h",
 * and named  SYSCTRL_DDRINFO.
 */

/*
 * getting the DDR manufacturer id and memory size from xloader.
 * RETURN VALUE:
 *   0 -- error.
 *   nonzero value -- the register value,DDR manufacturer id and memory size.
 */
static unsigned int get_ddr_info(void)
{
	unsigned int tmp_reg_value;

#if LINUX_VERSION_CODE >= KERNEL_VERSION(5, 10, 0)
	unsigned long *virtual_addr = (unsigned long *)ioremap
		(DDR_INFO_ADDR & 0xFFFFF000, 0x800);
#else
	unsigned long *virtual_addr = (unsigned long *)ioremap_nocache
		(DDR_INFO_ADDR & 0xFFFFF000, 0x800);
#endif

	if (virtual_addr == NULL) {
		pr_err("%s  ioremap ERROR !!\n", __func__);
		return 0;
	}
	tmp_reg_value = *(unsigned long *)((uintptr_t)virtual_addr +
		(DDR_INFO_ADDR & 0x00000FFF));
	iounmap(virtual_addr);
	return tmp_reg_value;
}

#ifdef DDR_ANONYMOUS_FACTORY
static bool get_ddr_anonymization_from_dts(void)
{
	return false;
}
#else
static bool get_ddr_anonymization_from_dts(void)
{
	int ret;
	unsigned int flag = 0;
	struct device_node *nd = NULL;

	nd = of_find_compatible_node(NULL, NULL, "huawei,memory");
	if (nd == NULL)
		return false;

	ret = of_property_read_u32(nd, "memory-anonymization", &flag);
	if (ret != 0 || flag != 1)
		return false;

	return true;
}
#endif

static char hex_number_convert(char c)
{
	if ((c >= '0' && c < '9') || (c >= 'a' && c < 'f'))
		return c + 1;

	if (c == '9')
		return 'a';

	if (c == 'f')
		return '0';

	return c;
}

static void hex_arry_convert(char arry[], int arry_len)
{
	int i;
	for (i = 0; i < arry_len; i++)
		arry[i] = hex_number_convert(arry[i]);
}

static int ddr_info_show(struct seq_file *m, void *v)
{
	int ret;
	char manufid[DDR_MANUFID_MAX_LEN] = {0};
	unsigned int tmp_reg_value;

	tmp_reg_value = get_ddr_info();

	tmp_reg_value = tmp_reg_value & 0xFFFF;

	ret = snprintf_s(manufid, DDR_MANUFID_MAX_LEN,
		DDR_MANUFID_MAX_LEN - 1, "%02x", tmp_reg_value % DDR_INFO_MASK);
	if (ret == -1) {
		pr_err("%s snprintf_s fail.\n", __func__);
		return ret;
	}
	hex_arry_convert(manufid, DDR_MANUFID_MAX_LEN - 1);

	if (get_ddr_anonymization_from_dts())
		seq_printf(m, "ddr_info:\n0x%x%s\n", tmp_reg_value / DDR_INFO_MASK, manufid);
	else
		seq_printf(m, "ddr_info:\n0x%x\n", tmp_reg_value);
	return 0;
}

static int ddrinfo_open(struct inode *inode, struct file *file)
{
	return single_open(file, ddr_info_show, NULL);
}

#if LINUX_VERSION_CODE >= KERNEL_VERSION(5, 10, 0)
static const struct proc_ops proc_ddrinfo_operations = {
	.proc_open	= ddrinfo_open,
	.proc_read	= seq_read,
	.proc_lseek	= seq_lseek,
	.proc_release	= single_release,
};
#else
static const struct file_operations proc_ddrinfo_operations = {
	.open		= ddrinfo_open,
	.read		= seq_read,
	.llseek		= seq_lseek,
	.release	= single_release,
};
#endif

static int __init proc_ddr_info_init(void)
{
	void *ret = NULL;

	ret = proc_create("ddr_info", 0644, NULL, &proc_ddrinfo_operations);
	if (ret == NULL)
		pr_err("%s /proc/ddr_info init ERROR !!\n", __func__);

	return 0;
}

module_init(proc_ddr_info_init);
