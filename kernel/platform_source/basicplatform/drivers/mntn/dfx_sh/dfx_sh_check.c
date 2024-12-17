/* dfx_sh_check.c
 * Copyright (c) 2023 Huawei Technologies Co., Ltd.
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
#include "dfx_sh.h"
#include "dfx_sh_check.h"
#include "rdr_inner.h"
#include "mntn_subtype_exception.h"
#include <platform_include/basicplatform/linux/rdr_platform.h>
#include <platform_include/basicplatform/linux/dfx_hw_diag.h>
#include <platform_include/cee/linux/hisi_ddr.h>
#include <linux/cpu.h>

typedef bool (*sh_check_fn)(void);
enum cpu_cluster {
	CPU_CLUSTER_LIT = 0,
	CPU_CLUSTER_MID = 1,
	CPU_CLUSTER_BIG = 2,
};

static bool sh_check_cpu_panic(enum cpu_cluster cluster)
{
	int cpunum;
	if (rdr_get_reboot_type() == AP_S_PANIC &&
		rdr_get_exec_subtype_value() == HI_APPANIC_RESERVED) {
		cpunum = dfx_hw_diag_get_panic_cpunum();
		if (cpunum < 0 ||  cpunum >= nr_cpu_ids) {
			pr_err("dfx_sh check cpu panic failed, invalid cpu num %d\n", cpunum);
			return false;
		}
		pr_info("dfx_sh check cpu panic, cpu num %d, cluster: %d\n",
				cpunum, topology_physical_package_id(cpunum));
		return (topology_physical_package_id(cpunum) == cluster);	
	}
	return false;
}

static bool is_sh_cpu_lit_panic(void)
{
	return sh_check_cpu_panic(CPU_CLUSTER_LIT);
}

static bool is_sh_cpu_mid_panic(void)
{
	return sh_check_cpu_panic(CPU_CLUSTER_MID);
}

static bool is_sh_cpu_big_panic(void)
{
	return sh_check_cpu_panic(CPU_CLUSTER_BIG);
}

#define QIC_PERI_BUSKEY		0x400
static bool is_sh_peri_panic(void)
{
	unsigned int reboot_type;
	unsigned int qic_buskey = 0;
	bool qice_intr;

	reboot_type = rdr_get_reboot_type();
	qice_intr = check_qice_intr();
#ifdef CONFIG_DFX_SEC_QIC
	qic_buskey = dfx_hw_diag_get_qic_buskey();
#endif

	pr_info("dfx_sh check peri panic, reboot_type 0x%x, qic_buskey 0x%X, qice_intr %u\n",
			reboot_type, qic_buskey, qice_intr);
	if (qice_intr)
		return true;
#ifdef CONFIG_DFX_SEC_QIC
	if (qic_buskey == QIC_PERI_BUSKEY)
		return true;
#endif
	if (reboot_type == AP_S_AWDT || reboot_type == LPM3_S_LPMCURST)
		return true;
	return false;
}

sh_check_fn g_sh_check[sh_type_max] = {
	is_sh_cpu_lit_panic,
	is_sh_cpu_mid_panic,
	is_sh_cpu_big_panic,
	NULL, /* check L3 cache panic */
	is_sh_peri_panic,
};

bool sh_check_panic(enum sh_type type)
{
	bool panic_happen = false;
	if (type < 0 || type >= sh_type_max)
		return false;
	if (g_sh_check[type])
		panic_happen = g_sh_check[type]();

	if (panic_happen)
		pr_info("sh type %d panic happened\n", type);
	return panic_happen;
}

bool sh_peri_need_fast_pv(void)
{
	unsigned int reboot_type;
	unsigned int excep_sub_type;
	bool qice_intr;

	reboot_type = rdr_get_reboot_type();
	excep_sub_type = rdr_get_exec_subtype_value();
	qice_intr = check_qice_intr();

	return (reboot_type == LPM3_S_EXCEPTION && excep_sub_type == PSCI_AHB_TIMEOUT && qice_intr);
}