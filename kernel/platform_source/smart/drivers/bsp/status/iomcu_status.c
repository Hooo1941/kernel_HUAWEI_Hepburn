/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2017-2021. All rights reserved.
 * Description: Contexthub common driver.
 * Create: 2017-07-21
 */
#include <linux/of.h>
#include <linux/of_address.h>
#include <linux/time.h>
#include <linux/platform_device.h>
#include <linux/workqueue.h>
#include <linux/delay.h>
#include <linux/fb.h>
#include <linux/module.h>
#include <linux/types.h>
#include <linux/init.h>
#include <linux/err.h>
#include <securec.h>
#include <securectype.h>
#include <platform_include/smart/linux/iomcu_status.h>
#include <platform_include/smart/linux/base/ap/protocol.h>
#ifdef CONFIG_SECURITY_HEADER_FILE_REPLACE
#include <internal_security_interface.h>
#endif
#ifdef __LLT_UT__
#define STATIC
#else
#define STATIC static
#endif

#ifndef CONFIG_INPUTHUB_20_970
#define IOMCU_LOGIC_NODE_COMPATIBLE "hisilicon,iomcu_logic"
#define IOMCU_LOGIC_NODE_PROPERTY   "logic_available_pair"

enum {
	IOMCU_LOGIC_REG_ADDR,
	IOMCU_LOGIC_REG_BIT,
	IOMCU_LOGIC_REG_MAX,
};
#endif
#ifdef CONFIG_SECURITY_HEADER_FILE_REPLACE
unsigned int g_ddr_addr_buff[DTSI_MAX];
unsigned int g_ddr_size_buff[DTSI_MAX];
int g_dts_get_flag;
unsigned int g_contexhub_boot_stat;
#endif

static bool recovery_mode_skip_load(void)
{
	int len = 0;
	struct device_node *recovery_node = NULL;
	const char *recovery_attr = NULL;

	if (!strstr(saved_command_line, "recovery_update=1"))
		return false;

	recovery_node = of_find_compatible_node(NULL, NULL, "hisilicon,recovery_iomcu_image_skip");
	if (!recovery_node)
		return false;

	recovery_attr = of_get_property(recovery_node, "status", &len);
	if (!recovery_attr)
		return false;

	if (strcmp(recovery_attr, "ok") != 0)
		return false;

	return true;
}

#ifndef CONFIG_INPUTHUB_20_970
/* FPGA, if iomcu logic is available. available by default */
static bool ctxhub_is_iomcu_logic_available(void)
{
	struct device_node *node = NULL;
	unsigned int reg_pair[IOMCU_LOGIC_REG_MAX];
	void __iomem *addr = NULL;
	int ret;

	node = of_find_compatible_node(NULL, NULL, IOMCU_LOGIC_NODE_COMPATIBLE);
	if (node == NULL)
		return true;

	if (!of_device_is_available(node))
		return true;

	ret = of_property_read_u32_array(node, IOMCU_LOGIC_NODE_PROPERTY, reg_pair, IOMCU_LOGIC_REG_MAX);
	if (ret != 0) {
		pr_warn("[%s] read node property fail, ret[%d]!\n", __func__, ret);
		return true;
	}

	addr = ioremap(reg_pair[IOMCU_LOGIC_REG_ADDR], sizeof(unsigned int));
	if (addr == NULL)
		return true;

	if (is_bits_set(1 << reg_pair[IOMCU_LOGIC_REG_BIT], addr)) {
		iounmap(addr);
		return true;
	}

	iounmap(addr);

	return false;
}
#endif

#ifdef CONFIG_SECURITY_HEADER_FILE_REPLACE
static int get_boot_stat(unsigned int *boot_stat)
{
	int ret;
	struct device_node *ddr_node;
	ddr_node = of_find_node_by_name(NULL, "contexhub_boot_stat_sctrl_offset");
	if (!ddr_node) {
		pr_err("%s, can not find node ddr_log_buff\n", __func__);
		return -1;
	}
	ret = of_property_read_u32(ddr_node, "reg", boot_stat);
	if (ret) {
		pr_err("%s failed to get reg err! ret=%d\n", __func__, ret);
		of_node_put(ddr_node);
		return -1;
	}
	of_node_put(ddr_node);
	/* SUCCESS */
	return 0;
}

static int get_ddr_dts_addr(int ddr_index, u32 *base_addr)
{
	int ret = 0;
	struct device_node *ddr_node = NULL;

	ddr_node = of_find_node_by_name(NULL, "ddr_buff");
	if (!ddr_node) {
		pr_err("%s, can not find node ddr_log_buff\n", __func__);
		return -1;
	}
	ret = of_property_read_u32_index(ddr_node, "reg",  (u32)(ddr_index * 2), base_addr);
	if (ret) {
		pr_err("%s failed to get ddr node \"reg\" property err! ret=%d\n", __func__, ret);
		of_node_put(ddr_node);
		return -1;
	}
	of_node_put(ddr_node);
	return ret;
}

static int get_ddr_dts_size(int ddr_index, u32 *addr_size)
{
	int ret = 0;
	struct device_node *ddr_node = NULL;

	ddr_node = of_find_node_by_name(NULL, "ddr_buff");
	if (!ddr_node) {
		pr_err("%s, can not find node ddr_log_buff\n", __func__);
		return -1;
	}
	ret = of_property_read_u32_index(ddr_node, "reg",  (u32)(ddr_index * 2 + 1), addr_size);
	if (ret) {
		pr_err("%s failed to get ddr node \"reg\" property err! ret=%d\n", __func__, ret);
		of_node_put(ddr_node);
		return -1;
	}
	of_node_put(ddr_node);
	return ret;
}
#endif

int get_contexthub_dts_status(void)
{
	int len = 0;
	int result = 0;
	unsigned int value = 0;
	struct device_node *node = NULL;
	const char *status = NULL;
	static int ret = 0;
	static int once;

#ifdef CONFIG_SECURITY_HEADER_FILE_REPLACE
	if (get_ddr_dts_status() == 0) {
		pr_err("%s:failed to get ddr dts info\n", __func__);
		ret = -EINVAL;
	}
#endif
	if (once != 0) {
		pr_info("[%s]status[%d]\n", __func__, ret);
		return ret;
	}

	if (recovery_mode_skip_load()) {
		pr_err("%s: recovery update mode, do not start sensorhub\n", __func__);
		once = 1;
		ret = -EINVAL;
		return ret;
	}

#ifndef CONFIG_INPUTHUB_20_970
	if (!ctxhub_is_iomcu_logic_available()) {
		pr_warn("%s: iomcu logic not availale\n", __func__);
		once = 1;
		ret = -EINVAL;
		return ret;
	}
#endif

	node = of_find_compatible_node(NULL, NULL, "hisilicon,contexthub_status");
	if (node != NULL) {
		status = of_get_property(node, "status", &len);
		if (status == NULL) {
			pr_err("[%s]of_get_property status err\n", __func__);
			return -EINVAL;
		}

		if (strstr(status, "disabled")) {
			pr_info("[%s][disabled]\n", __func__);
			ret = -EINVAL;
		}

		result = of_property_read_u32(node, "hardware_bypass", &value);
		if (result != 0) {
			pr_warn("%s: read iomcu hardware_bypass fail, ret[%d]\n", __func__, result);
		} else if (value == 1) {
			pr_warn("%s: iomcu hardware bypass\n", __func__);
			ret = -EINVAL;
		}
	}

	once = 1;
	return ret;
}

int get_ext_contexthub_dts_status(void)
{
	int len = 0;
	struct device_node *node = NULL;
	const char *status = NULL;
	static int ret = -EINVAL;
	static int once;

	if (once) {
		pr_info("[%s]status[%d]\n", __func__, ret);
		return ret;
	}

	node = of_find_compatible_node(NULL, NULL, "huawei,ext_sensorhub_status");
	if (node != NULL) {
		status = of_get_property(node, "status", &len);
		if (status == NULL) {
			pr_err("[%s]of_get_property status err\n", __func__);
			return -EINVAL;
		}

		if (strstr(status, "ok")) {
			pr_info("[%s][disabled]\n", __func__);
			ret = 0;
		}
	}

	once = 1;
	pr_info("[%s][enabled]\n", __func__);
	return ret;
}

#ifdef CONFIG_SECURITY_HEADER_FILE_REPLACE
unsigned int dts_ddr_addr_get(unsigned int ddr_index)
{
	if (ddr_index >= DTSI_MAX) {
		pr_err("[%s] :failed to get ddr dts base addr!\n", __func__);
		return 0;
	}
	return g_ddr_addr_buff[ddr_index];
}

unsigned int dts_ddr_size_get(unsigned int ddr_index)
{
	if (ddr_index >= DTSI_MAX) {
		pr_err("[%s] :failed to get ddr dts szie!\n", __func__);
		return 0;
	}
	return g_ddr_size_buff[ddr_index];
}

unsigned int get_contexhub_boot_stat(void)
{
	return g_contexhub_boot_stat;
}

int get_ddr_dts_status(void)
{
	return g_dts_get_flag;
}

int get_dts_info(void)
{
	int ddr_index = 0;
	int ret1 = 0;
	int ret2 = 0;
	int ret3 = 0;
	for(ddr_index = 0;ddr_index < DTSI_MAX;ddr_index++) {
		ret1 = get_ddr_dts_addr(ddr_index, (u32 *)(g_ddr_addr_buff + ddr_index));
		ret2 = get_ddr_dts_size(ddr_index, (u32 *)(g_ddr_size_buff + ddr_index));
		if(ret1 != 0 || (g_ddr_addr_buff[ddr_index] == 0) || ret2 != 0 || (g_ddr_size_buff[ddr_index] == 0)) {
		   pr_err("[%s] :failed to get ddr dts info !\n", __func__);
		   return -1;
		}
	}
	ret3 = get_boot_stat(&g_contexhub_boot_stat);
	if (ret3) {
		pr_err("[%s] : failed to get boot stat\n", __func__);
		return -1;
	}
	if(!ret1 && !ret2 && !ret3)
		g_dts_get_flag = 1;
	return 0;
}
#endif