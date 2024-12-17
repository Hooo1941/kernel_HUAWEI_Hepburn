/*
 * Copyright (C) 2010 Trusted Logic S.A.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 *
 */

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/slab.h>
#include <linux/init.h>
#include <linux/list.h>
#include <linux/i2c.h>
#include <linux/irq.h>
#include <linux/jiffies.h>
#include <linux/uaccess.h>
#include <linux/delay.h>
#include <linux/interrupt.h>
#include <linux/io.h>
#include <linux/platform_device.h>
#include <linux/gpio.h>
#include <linux/miscdevice.h>
#include <linux/spinlock.h>
#include <linux/reboot.h>
#include <linux/clk.h>
#include <linux/pm_wakeup.h>
#include <linux/of.h>
#include <linux/of_platform.h>
#include <linux/of_device.h>
#include <linux/of_gpio.h>
#include <linux/regulator/consumer.h>
#include <linux/mtd/hisi_nve_interface.h>
#ifdef CONFIG_HUAWEI_DSM
#include <dsm/dsm_pub.h>
#endif
#include "pn547.h"
#include "securec.h"
#include <platform_include/basicplatform/linux/mfd/pmic_platform.h>
#include <chipset_common/hwpower/hardware_ic/boost_5v.h>

#ifdef CONFIG_HUAWEI_HW_DEV_DCT
#include <huawei_platform/devdetect/hw_dev_dec.h>
#endif
#include <huawei_platform/log/hw_log.h>
#include <huawei_platform/nfc/nfc_interface.h>
#define HWLOG_TAG nfc
HWLOG_REGIST();

#define RSSI_MAX 65535

#define PMU_CLK_NFC_EN_REG 0x42
#define PMU_CLK_LDO_MASK_REG 0xBF
#define PMU_CLK_SQUARE_OUTPUT_REG 0x401
#define PMU_CLK_SQUARE_DRIVER_REG 0xA7

#define NFC_DMD_PROBE_TIMER (10 * HZ) /* delay to probe dmd */

#define MAX_CHIP_LENGTH 8 /* The supported NFC chip types are listed as follows: */
#define NFC_CHIP_THN31 "thn31"
#define NFC_CHIP_ST "st21nfc"
#define NFC_CHIP_ST54K "st54k"
#define NFC_CHIP_PN553 "pn553"
#define NFC_CHIP_PN80T "pn80t"
#define NFC_CHIP_SN110U "sn110u"
#define NFC_CHIP_SZ "nfc_sz"
#define NFC_CHIP_BJ "nfc_bj"
#define NCI_HEADER 3
#define T1_HEAD 0x5A
#define I2C_ELAPSE_TIMEOUT (1500 * 1000 * 1000L)
#define MAX_I2C_WAKEUP_TIME 3
#define I2C_WAKEUP_SLEEP_TIME1 5000
#define I2C_WAKEUP_SLEEP_TIME2 5100

enum NFC_CHIPTYPE {
	NFCTYPE_INVALID = 0,
	NFCTYPE_ST21NFC = 1, // st54h or st21
	NFCTYPE_NXP     = 2, // pn80t
	NFCTYPE_SN110   = 3,
	NFCTYPE_ST54K   = 4,
	NFCTYPE_THN31   = 5,
	NFCTYPE_PN553   = 6,
	MAX_CHIP_TYPE
};

typedef struct {
	const char* type_str;
	int type_value;
} chip_type_mask;

/*lint -save -e528 -e529*/
static bool ven_felica_status;
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(5, 10, 0))
static struct wakeup_source *wlock_read;
#else
static struct wakeup_source wlock_read;
#endif
static int firmware_update;
static int nfc_switch_state;
static int nfc_at_result;
long nfc_get_rssi = 0;
static char g_nfc_nxp_config_name[MAX_CONFIG_NAME_SIZE];
static unsigned char g_nfc_nxp_config_clock_switch = 0;
static char g_nfc_brcm_conf_name[MAX_CONFIG_NAME_SIZE];
static char g_nfc_chip_type[MAX_NFC_CHIP_TYPE_SIZE];
static char g_nfc_fw_version[MAX_NFC_FW_VERSION_SIZE];
static unsigned int g_nfc_ext_gpio; /* use extented gpio, eg.codec */
static int g_nfc_svdd_sw; /* use for svdd switch */
static int g_nfc_on_type; /* nfc ven controlled by which type of gpio: gpio/hisi_pmic/regulator_bulk */
/* 0 -- hisi cpu(default); 1 -- hisi pmu */
static int g_nfc_clk_src = NFC_CLK_SRC_CPU;
static int g_nfcservice_lock;
/* 0 -- close nfcservice normally; 1 -- close nfcservice with enable CE */
static int g_nfc_close_type;
static int g_nfc_single_channel;
static int g_nfc_spec_rf;
static int g_nfc_delaytime_set;
static int g_nfc_delaytime_node;
static int g_nfc_card_num;
static int g_nfc_ese_num;
static int g_wake_lock_timeout = WAKE_LOCK_TIMEOUT_DISABLE;
static int g_nfc_ese_type = NFC_WITHOUT_ESE; /* record ese type wired to nfcc by dts */
static int g_nfc_activated_se_info; /* record activated se info when nfc enable process */
static int g_nfc_hal_dmd_no; /* record last hal dmd no */
static int g_clk_status_flag = 0; /* use for judging whether clk has been enabled */
static int g_nfc_sim2_omapi;
static t_pmu_reg_control nfc_on_hi6421v600 = {0x240, 0};
static t_pmu_reg_control nfc_on_hi6421v500 = {0x0C3, 4};
static t_pmu_reg_control nfc_on_hi6555v110 = {0x158, 0};
static t_pmu_reg_control nfc_on_hi6421v700 = {0x2E6, 0};
static t_pmu_reg_control nfc_on_hi6421v800 = {0x2E6, 0};
static t_pmu_reg_control nfc_on_hi6421v900 = {0x40C, 0};
static t_pmu_reg_control nfc_on_hi6555v300 = {0x22D, 0};
static t_pmu_reg_control nfc_on_hi6555v500 = {0x2E5, 0};
static bool g_is_gtboost = false;
static int g_chip_type = NFCTYPE_INVALID;
static bool g_is_uid = false;
static struct timer_list nfc_dmd_probe_timer;

/*lint -save -e* */
struct pn547_dev {
	wait_queue_head_t read_wq;
	struct mutex read_mutex;
	struct i2c_client *client;
	struct miscdevice pn547_device;
	struct pinctrl *pctrl;
	struct pinctrl_state *pins_default;
	struct pinctrl_state *pins_idle;
	unsigned int firm_gpio;
	unsigned int irq_gpio;
	unsigned int clkreq_gpio;
	int sim_switch;
	int sim_status;
	int enable_status;
	bool irq_enabled;
	spinlock_t irq_enabled_lock;
	struct mutex irq_mutex_lock;
	unsigned int ven_gpio;
	struct regulator_bulk_data ven_felica;
	struct clk *nfc_clk;
	bool release_read;
};
/*lint -restore*/
static struct pn547_dev *nfcdev;
static ktime_t g_pre_write_time;

#ifdef CONFIG_HUAWEI_DSM
static struct dsm_dev dsm_nfc = {
	.name = "dsm_nfc",
	.device_name = NULL,
	.ic_name = NULL,
	.module_name = NULL,
	.fops = NULL,
	.buff_size = 1024,
};

static struct dsm_client *nfc_dclient = NULL;

#endif
/*lint -save -e* */
void realse_read(void)
{
	hwlog_info("realse_read\n");
	wake_up(&nfcdev->read_wq);
}
EXPORT_SYMBOL(realse_read);
static void nfc_is_recovery_mode(void)
{
	if (strstr(saved_command_line, "rescue_mode=true")) {
		hwlog_info("%s: nfc_at_result = %d\n", __func__, nfc_at_result);
		nfc_at_result = RECOVERY_MODE;
		return;
	}
	hwlog_info("%s: else nfc_at_result = %d\n", __func__, nfc_at_result);
	nfc_at_result = NORMAL_MODE;
	return;
}

static int nfc_nv_opera(unsigned int opr, unsigned int idx, const char *name, unsigned int len, void *data)
{
	int err;
	int ret;
	struct hisi_nve_info_user user_info;
	 hwlog_info("%s:enter.\n", __func__);

	memset_s(&user_info, sizeof(user_info), 0x00, sizeof(user_info));
	user_info.nv_operation = opr;
	user_info.nv_number = idx;
	user_info.valid_size = len;

	err = strncpy_s(user_info.nv_name, sizeof(user_info.nv_name),
		name, sizeof(user_info.nv_name));
	if (err != 0)
		hwlog_err("%s: strncpy_s fail\n", __func__);
	user_info.nv_name[sizeof(user_info.nv_name) - 1] = '\0';

	/* write data to nv */
	if (opr == NV_WRITE) {
		if (len > sizeof(user_info.nv_data))
			len = sizeof(user_info.nv_data);
		err = memcpy_s((char *)user_info.nv_data, len, (char *)data, len);
		if (err != EOK)
			hwlog_err("%s memcpy_s fail\n", __func__);
	}

	ret = hisi_nve_direct_access(&user_info);
	if (ret != 0) {
		hwlog_err("%s:nve_direct_access read error %d\n", __func__, ret);
		return -1;
	}
	err = memcpy_s((char *)data, len, (char *)user_info.nv_data, len);
	if (err != EOK)
		hwlog_err("%s memcpy_s fail\n", __func__);
	return 0;
}

static unsigned int clr_pmu_reg_bit(unsigned int ret, int bit_pos)
{
	ret = ret & (~(1<<(bit_pos)));
	return ret;
}

static unsigned int set_pmu_reg_bit(unsigned int ret, int bit_pos)
{
	ret = ret | (1<<(bit_pos));
	return ret;
}

/*lint -restore*/
/*lint -save -e* */
static int hisi_pmu_reg_operate_by_bit(int reg_addr, int bit_pos, int op)
{
	unsigned int ret = 0;
	if (reg_addr <= 0 || bit_pos < 0 || op < 0) {
		hwlog_err("%s: reg_addr[%x], bit_pos[%d], op[%d], error!\n", __func__, reg_addr, bit_pos, op);
		return -1;
	}
	ret = pmic_read_reg(reg_addr);
	if (op == CLR_BIT) {
		ret = clr_pmu_reg_bit(ret, bit_pos);
	} else {
		ret = set_pmu_reg_bit(ret, bit_pos);
	}
	pmic_write_reg(reg_addr, ret);
	hwlog_info("%s: reg: 0x%x, pos: %d, value: 0x%x\n", __func__, reg_addr, bit_pos, ret);
	return 0;
}

static int get_hisi_pmic_reg_status(int reg_addr, int bit_pos)
{
	unsigned int ret = 0;
	if (reg_addr <= 0 || bit_pos < 0 || bit_pos > 31) {
		hwlog_err("%s: reg_addr[%x], bit_pos[%d], error!\n", __func__, reg_addr, bit_pos);
		return -1;
	}
	ret = pmic_read_reg(reg_addr);

	ret = (ret >> bit_pos) & 0x01;

	hwlog_info("%s: reg: 0x%x, pos: %d, value: 0x%x\n", __func__, reg_addr, bit_pos, ret);
	return (int)ret;
}

static int hisi_pmic_on(void)
{
	int ret = -1;
	if (g_nfc_on_type == NFC_ON_BY_HI6421V600_PMIC) {
		ret = hisi_pmu_reg_operate_by_bit(nfc_on_hi6421v600.addr, nfc_on_hi6421v600.pos, SET_BIT);
	} else if (g_nfc_on_type == NFC_ON_BY_HISI_PMIC) {
		ret = hisi_pmu_reg_operate_by_bit(nfc_on_hi6421v500.addr, nfc_on_hi6421v500.pos, SET_BIT);
	} else if (g_nfc_on_type == NFC_ON_BY_HI6555V110_PMIC) {
		ret = hisi_pmu_reg_operate_by_bit(nfc_on_hi6555v110.addr, nfc_on_hi6555v110.pos, SET_BIT);
	} else if (g_nfc_on_type == NFC_ON_BY_HI6421V700_PMIC) {
		ret = hisi_pmu_reg_operate_by_bit(nfc_on_hi6421v700.addr, nfc_on_hi6421v700.pos, SET_BIT);
	} else if (g_nfc_on_type == NFC_ON_BY_HI6421V800_PMIC) {
		ret = hisi_pmu_reg_operate_by_bit(nfc_on_hi6421v800.addr, nfc_on_hi6421v800.pos, SET_BIT);
	} else if (g_nfc_on_type == NFC_ON_BY_HI6421V900_PMIC) {
		ret = hisi_pmu_reg_operate_by_bit(nfc_on_hi6421v900.addr, nfc_on_hi6421v900.pos, SET_BIT);
	} else if (g_nfc_on_type == NFC_ON_BY_HI6555V300_PMIC) {
		ret = hisi_pmu_reg_operate_by_bit(nfc_on_hi6555v300.addr, nfc_on_hi6555v300.pos, SET_BIT);
	} else if (g_nfc_on_type == NFC_ON_BY_HI6555V500_PMIC) {
		ret = hisi_pmu_reg_operate_by_bit(nfc_on_hi6555v500.addr, nfc_on_hi6555v500.pos, SET_BIT);
	} else {
		hwlog_err("%s: bad g_nfc_on_type: %d\n", __func__, g_nfc_on_type);
	}
	return ret;
}

static int hisi_pmic_off(void)
{
	int ret = -1;
	if (g_nfc_on_type == NFC_ON_BY_HI6421V600_PMIC) {
		ret = hisi_pmu_reg_operate_by_bit(nfc_on_hi6421v600.addr, nfc_on_hi6421v600.pos, CLR_BIT);
	}  else if (g_nfc_on_type == NFC_ON_BY_HISI_PMIC) {
		ret = hisi_pmu_reg_operate_by_bit(nfc_on_hi6421v500.addr, nfc_on_hi6421v500.pos, CLR_BIT);
	}  else if (g_nfc_on_type == NFC_ON_BY_HI6555V110_PMIC) {
		ret = hisi_pmu_reg_operate_by_bit(nfc_on_hi6555v110.addr, nfc_on_hi6555v110.pos, CLR_BIT);
	} else if (g_nfc_on_type == NFC_ON_BY_HI6421V700_PMIC) {
		ret = hisi_pmu_reg_operate_by_bit(nfc_on_hi6421v700.addr, nfc_on_hi6421v700.pos, CLR_BIT);
	} else if (g_nfc_on_type == NFC_ON_BY_HI6421V800_PMIC) {
		ret = hisi_pmu_reg_operate_by_bit(nfc_on_hi6421v800.addr, nfc_on_hi6421v800.pos, CLR_BIT);
	} else if (g_nfc_on_type == NFC_ON_BY_HI6421V900_PMIC) {
		ret = hisi_pmu_reg_operate_by_bit(nfc_on_hi6421v900.addr, nfc_on_hi6421v900.pos, CLR_BIT);
	} else if (g_nfc_on_type == NFC_ON_BY_HI6555V300_PMIC) {
		ret = hisi_pmu_reg_operate_by_bit(nfc_on_hi6555v300.addr, nfc_on_hi6555v300.pos, CLR_BIT);
	} else if (g_nfc_on_type == NFC_ON_BY_HI6555V500_PMIC) {
		ret = hisi_pmu_reg_operate_by_bit(nfc_on_hi6555v500.addr, nfc_on_hi6555v500.pos, CLR_BIT);
	} else {
		hwlog_err("%s: bad g_nfc_on_type: %d\n", __func__, g_nfc_on_type);
	}
	return ret;
}

static void pmu0_svdd_sel_on(void)
{
	hwlog_info("%s: in g_nfc_svdd_sw: %d\n", __func__, g_nfc_svdd_sw);
	if (g_nfc_svdd_sw == NFC_SWP_SW_HI6421V600) {
		hisi_pmu_reg_operate_by_bit(nfc_on_hi6421v600.addr, nfc_on_hi6421v600.pos, SET_BIT);
	} else if (g_nfc_svdd_sw == NFC_SWP_SW_HI6421V500) {
		hisi_pmu_reg_operate_by_bit(nfc_on_hi6421v500.addr, nfc_on_hi6421v500.pos, SET_BIT);
	} else if (g_nfc_svdd_sw == NFC_SWP_SW_HI6555V110) {
		hisi_pmu_reg_operate_by_bit(nfc_on_hi6555v110.addr, nfc_on_hi6555v110.pos, SET_BIT);
	} else {
		hwlog_info("%s: pmu gpio don't connect to switch\n", __func__);
	}
}

static void pmu0_svdd_sel_off(void)
{
	hwlog_info("%s: in g_nfc_svdd_sw: %d\n", __func__, g_nfc_svdd_sw);
	if (g_nfc_svdd_sw == NFC_SWP_SW_HI6421V600) {
		hisi_pmu_reg_operate_by_bit(nfc_on_hi6421v600.addr, nfc_on_hi6421v600.pos, CLR_BIT);
	} else if (g_nfc_svdd_sw == NFC_SWP_SW_HI6421V500) {
		hisi_pmu_reg_operate_by_bit(nfc_on_hi6421v500.addr, nfc_on_hi6421v500.pos, CLR_BIT);
	} else if (g_nfc_svdd_sw == NFC_SWP_SW_HI6555V110) {
		hisi_pmu_reg_operate_by_bit(nfc_on_hi6555v110.addr, nfc_on_hi6555v110.pos, CLR_BIT);
	} else {
		hwlog_info("%s: pmu gpio don't connect to switch\n", __func__);
	}
}

static int get_pmu0_svdd_sel_status(void)
{
	int ret = -1;
	hwlog_info("%s: in g_nfc_svdd_sw: %d\n", __func__, g_nfc_svdd_sw);
	if (g_nfc_svdd_sw == NFC_SWP_SW_HI6421V600) {
		ret = get_hisi_pmic_reg_status(nfc_on_hi6421v600.addr, nfc_on_hi6421v600.pos);
	} else if (g_nfc_svdd_sw == NFC_SWP_SW_HI6421V500) {
		ret = get_hisi_pmic_reg_status(nfc_on_hi6421v500.addr, nfc_on_hi6421v500.pos);
	} else if (g_nfc_svdd_sw == NFC_SWP_SW_HI6555V110) {
		ret = get_hisi_pmic_reg_status(nfc_on_hi6555v110.addr, nfc_on_hi6555v110.pos);
	} else {
		hwlog_info("%s: pmu gpio don't connect to switch\n", __func__);
	}
	return ret;
}

static int nfc_get_dts_config_string(const char *node_name,
				const char *prop_name,
				char *out_string,
				int out_string_len)
{
	struct device_node *np = NULL;
	const char *out_value;
	int ret = -1;

	for_each_node_with_property(np, node_name) {
		ret = of_property_read_string(np, prop_name, (const char **)&out_value);
		if (ret != 0) {
			hwlog_err("%s: can not get prop values with prop_name: %s\n",
							__func__, prop_name);
		} else {
			if (out_value == NULL) {
				hwlog_info("%s: error out_value = NULL\n", __func__);
				ret = -1;
			} else if (strlen(out_value) >= out_string_len) {
				hwlog_info("%s: error out_value len :%d >= out_string_len:%d\n",
					__func__, (int)strlen(out_value), (int)out_string_len);
				ret = -1;
			} else {
				errno_t rc = strcpy_s(out_string, out_string_len, out_value);
				if (rc != EOK)
					hwlog_err("%s: strncpy_s fail\n", __func__);
				hwlog_info("%s: =%s\n", __func__, out_string);
			}
		}
	}
	return ret;
}

static int nfc_get_dts_config_u32(const char *node_name, const char *prop_name, u32 *pvalue)
{
	struct device_node *np = NULL;
	int ret = -1;

	for_each_node_with_property(np, node_name) {
		ret = of_property_read_u32(np, prop_name, pvalue);
		if (ret != 0) {
			hwlog_err("%s: can not get prop values with prop_name: %s\n", __func__, prop_name);
		} else {
			hwlog_info("%s: %s=%d\n", __func__, prop_name, *pvalue);
		}
	}
	return ret;
}
/*lint -restore*/
/*
 * g_nfc_ext_gpio bit 0: 1 -- dload use extented gpio, 0 -- dload use soc gpio
 * g_nfc_ext_gpio bit 1: 1 -- irq use extented gpio, 0 -- irq use soc gpio
 */
 /*lint -save -e* */
static void get_ext_gpio_type(void)
{
	int ret = -1;

	ret = nfc_get_dts_config_u32("nfc_ext_gpio", "nfc_ext_gpio", &g_nfc_ext_gpio);
	if (ret != 0) {
		g_nfc_ext_gpio = 0;
		hwlog_err("%s: can't check_ext_gpio\n", __func__);
	}
	hwlog_info("%s: g_nfc_ext_gpio:%d\n", __func__, g_nfc_ext_gpio);
	return;
}

static void nfc_gpio_set_value(unsigned int gpio, int val)
{
	if (g_nfc_ext_gpio & DLOAD_EXTENTED_GPIO_MASK) {
		hwlog_info("%s: gpio_set_value_cansleep\n", __func__);
		gpio_set_value_cansleep(gpio, val);
	} else {
		hwlog_info("%s: gpio_set_value\n", __func__);
		gpio_set_value(gpio, val);
	}
}
/*lint -restore*/
static int nfc_gpio_get_value(unsigned int gpio)
{
	int ret = -1;

	if (g_nfc_ext_gpio & IRQ_EXTENTED_GPIO_MASK) {
		ret = gpio_get_value_cansleep(gpio);
	} else {
		ret = gpio_get_value(gpio);
	}
	return ret;
}
/*lint -save -e* */
static void get_wake_lock_timeout(void)
{
	char wake_lock_str[MAX_WAKE_LOCK_TIMEOUT_SIZE] = {0};
	int ret = -1;

	memset_s(wake_lock_str, sizeof(wake_lock_str), 0x00, MAX_WAKE_LOCK_TIMEOUT_SIZE);
	ret = nfc_get_dts_config_string("wake_lock_timeout", "wake_lock_timeout",
	wake_lock_str, sizeof(wake_lock_str));
	if (ret != 0) {
		memset_s(wake_lock_str, sizeof(wake_lock_str), 0x00, MAX_WAKE_LOCK_TIMEOUT_SIZE);
		g_wake_lock_timeout = WAKE_LOCK_TIMEOUT_DISABLE;
		hwlog_err("%s: can't find wake_lock_timeout\n", __func__);
		return;
	} else {
		if (!strncasecmp(wake_lock_str, "ok", strlen("ok"))) {
			g_wake_lock_timeout = WAKE_LOCK_TIMEOUT_ENALBE;
		} else {
			g_wake_lock_timeout = WAKE_LOCK_TIMEOUT_DISABLE;
		}
	}
	hwlog_info("%s: g_wake_lock_timeout:%d\n", __func__, g_wake_lock_timeout);
	return;
}

static void get_nfc_on_type(void)
{
	char nfc_on_str[MAX_DETECT_SE_SIZE] = {0};
	int ret = -1;

	memset_s(nfc_on_str, sizeof(nfc_on_str), 0x00, MAX_DETECT_SE_SIZE);
	ret = nfc_get_dts_config_string("nfc_on_type", "nfc_on_type",
	nfc_on_str, sizeof(nfc_on_str));
	if (ret != 0) {
		memset_s(nfc_on_str, sizeof(nfc_on_str), 0x00, MAX_DETECT_SE_SIZE);
		g_nfc_on_type = NFC_ON_BY_REGULATOR_BULK;
		hwlog_err("%s: can't check_ext_gpio\n", __func__);
		return;
	} else {
		if (!strncasecmp(nfc_on_str, "gpio", strlen("gpio"))) {
			g_nfc_on_type = NFC_ON_BY_GPIO;
		} else if (!strncasecmp(nfc_on_str, "hisi_pmic", strlen("hisi_pmic"))) {
			g_nfc_on_type = NFC_ON_BY_HISI_PMIC;
		} else if (!strncasecmp(nfc_on_str, "hi6421v600_pmic", strlen("hi6421v600_pmic"))) {
			g_nfc_on_type = NFC_ON_BY_HI6421V600_PMIC;
		} else if (!strncasecmp(nfc_on_str, "hi6421v700_pmic", strlen("hi6421v700_pmic"))) {
			g_nfc_on_type = NFC_ON_BY_HI6421V700_PMIC;
		} else if (!strncasecmp(nfc_on_str, "hi6421v800_pmic", strlen("hi6421v800_pmic"))) {
			g_nfc_on_type = NFC_ON_BY_HI6421V800_PMIC;
		} else if (!strncasecmp(nfc_on_str, "hi6421v900_pmic", strlen("hi6421v900_pmic"))) {
			g_nfc_on_type = NFC_ON_BY_HI6421V900_PMIC;
		} else if (!strncasecmp(nfc_on_str, "hi6555v300_pmic", strlen("hi6555v300_pmic"))) {
			g_nfc_on_type = NFC_ON_BY_HI6555V300_PMIC;
		} else if (!strncasecmp(nfc_on_str, "hi6555v500_pmic", strlen("hi6555v500_pmic"))) {
			g_nfc_on_type = NFC_ON_BY_HI6555V500_PMIC;
		} else if (!strncasecmp(nfc_on_str, "hi6555v110_pmic", strlen("hi6555v110_pmic"))) {
			g_nfc_on_type = NFC_ON_BY_HI6555V110_PMIC;
		} else if (!strncasecmp(nfc_on_str, "regulator_bulk", strlen("regulator_bulk"))) {
			g_nfc_on_type = NFC_ON_BY_REGULATOR_BULK;
		} else {
			g_nfc_on_type = NFC_ON_BY_GPIO;
		}
	}
	hwlog_info("%s: g_nfc_on_type:%d\n", __func__, g_nfc_on_type);
	return;
}
static void get_nfc_chip_type(void)
{
	char nfc_on_str[MAX_DETECT_SE_SIZE] = {0};
	int ret, cnt;
	chip_type_mask chip_type[MAX_CHIP_LENGTH] = {
		{NFC_CHIP_ST, NFCTYPE_ST21NFC},
		{NFC_CHIP_THN31, NFCTYPE_THN31},
		{NFC_CHIP_ST54K, NFCTYPE_ST54K},
		{NFC_CHIP_PN553, NFCTYPE_PN553},
		{NFC_CHIP_PN80T, NFCTYPE_NXP},
		{NFC_CHIP_SN110U, NFCTYPE_SN110},
		{NFC_CHIP_SZ, NFCTYPE_SN110},
		{NFC_CHIP_BJ, NFCTYPE_THN31},
	};

	memset_s(nfc_on_str, sizeof(nfc_on_str), 0x00, MAX_DETECT_SE_SIZE);
	ret = nfc_get_dts_config_string("nfc_chip_type", "nfc_chip_type",
					nfc_on_str, sizeof(nfc_on_str));
	if (ret != 0) {
		memset_s(nfc_on_str, sizeof(nfc_on_str), 0x00, MAX_DETECT_SE_SIZE);
		hwlog_err("%s: can't find nfc_chip_type node\n", __func__);
		return;
	}
	for (cnt = 0; cnt < MAX_CHIP_LENGTH; cnt++) {
		if (chip_type[cnt].type_str == NULL) {
			continue;
		}
		if (!strncasecmp(nfc_on_str, chip_type[cnt].type_str, MAX_DETECT_SE_SIZE)) {
			g_chip_type = chip_type[cnt].type_value;
			break;
		}
	}
	hwlog_info("%s: nfc_chip_type:%d\n", __func__, g_chip_type);
	return;
}
static void get_nfc_wired_ese_type(void)
{
	char nfc_on_str[MAX_DETECT_SE_SIZE] = {0};
	int ret = -1;

	memset_s(nfc_on_str, sizeof(nfc_on_str), 0x00, MAX_DETECT_SE_SIZE);
	ret = nfc_get_dts_config_string("nfc_ese_type", "nfc_ese_type",
	nfc_on_str, sizeof(nfc_on_str));
	if (ret != 0) {
		memset_s(nfc_on_str, sizeof(nfc_on_str), 0x00, MAX_DETECT_SE_SIZE);
		g_nfc_ese_type = NFC_WITHOUT_ESE;
		hwlog_err("%s: can't find nfc_ese_type node\n", __func__);
		return;
	} else {
		if (!strncasecmp(nfc_on_str, "hisee", strlen("hisee"))) {
			g_nfc_ese_type = NFC_ESE_HISEE;
		} else if (!strncasecmp(nfc_on_str, "p61", strlen("p61"))) {
			g_nfc_ese_type = NFC_ESE_P61;
		} else {
			g_nfc_ese_type = NFC_WITHOUT_ESE;
		}
	}
	hwlog_info("%s: g_nfc_ese_type:%d\n", __func__, g_nfc_ese_type);
	return;
}

static void get_nfc_gt_boost(void)
{
	char nfc_on_str[MAX_DETECT_GT_SIZE] = {0};
	int boost_ret = -1;
	int ret;

	memset_s(nfc_on_str, sizeof(nfc_on_str), 0x00, MAX_DETECT_GT_SIZE);
	ret = nfc_get_dts_config_string("nfc_gt_boost", "nfc_gt_boost",
		nfc_on_str, sizeof(nfc_on_str));
	if (ret != 0) {
		memset_s(nfc_on_str, sizeof(nfc_on_str), 0x00, MAX_DETECT_GT_SIZE);
		hwlog_info("%s: gt boost not supported on this board, ret = %d\n",
			__func__, ret);
		return;
	} else {
		hwlog_info("%s: enabling 5v, and setting g_is_gtboost = true\n",
			__func__);
		g_is_gtboost = true;
		boost_ret  = boost_5v_enable(BOOST_5V_ENABLE, BOOST_CTRL_NFC);
		if (boost_ret)
			hwlog_err("%s: boost_5v enable failed\n", __func__);
		else
			hwlog_info("%s: boost_5v enable succeeded\n", __func__);

		/* delay 10ms for volt stability */
		usleep_range(9500, 10500);
	}
	return;
}

static void get_nfc_svdd_sw(void)
{
	char nfc_svdd_sw_str[MAX_DETECT_SE_SIZE] = {0};
	int ret = -1;

	memset_s(nfc_svdd_sw_str, sizeof(nfc_svdd_sw_str), 0x00, MAX_DETECT_SE_SIZE);
	ret = nfc_get_dts_config_string("nfc_svdd_sw", "nfc_svdd_sw",
		nfc_svdd_sw_str, sizeof(nfc_svdd_sw_str));
	if (ret != 0) {
		memset_s(nfc_svdd_sw_str, sizeof(nfc_svdd_sw_str), 0x00, MAX_DETECT_SE_SIZE);
		g_nfc_svdd_sw = NFC_SWP_WITHOUT_SW;
		hwlog_err("%s: can't get_nfc_svdd_sw\n", __func__);
	} else {
		if (!strncasecmp(nfc_svdd_sw_str, "hi6421v600_pmic", strlen("hi6421v600_pmic"))) {
			g_nfc_svdd_sw = NFC_SWP_SW_HI6421V600;
		} else if (!strncasecmp(nfc_svdd_sw_str, "hi6421v500_pmic", strlen("hi6421v500_pmic"))) {
			g_nfc_svdd_sw = NFC_SWP_SW_HI6421V500;
		} else if (!strncasecmp(nfc_svdd_sw_str, "hi6555v110_pmic", strlen("hi6555v110_pmic"))) {
			g_nfc_svdd_sw = NFC_SWP_SW_HI6555V110;
		} else {
			g_nfc_svdd_sw = NFC_SWP_WITHOUT_SW;
		}
	}
	hwlog_info("%s: g_nfc_svdd_sw:%d\n", __func__, g_nfc_svdd_sw);
	return;
}

static void get_nfc_pmu_clock_switch(void)
{
	int ret;
	int clock_switch = 0;
	const int noah_pmu_switch = 1;

	ret = nfc_get_dts_config_u32("nfc_pmu_clock_switch", "nfc_pmu_clock_switch", &clock_switch);
	if (ret != 0) {
		g_nfc_nxp_config_clock_switch = 0;
		hwlog_info("%s: has no nfc PMU clock switch config!\n", __func__);
	} else if (clock_switch == noah_pmu_switch) {
		// for Noah pmu switch
		g_nfc_nxp_config_clock_switch = 1;
		// Configuring Square Wave Output
		pmic_write_reg(PMU_CLK_SQUARE_OUTPUT_REG, 0);
		pmic_write_reg(PMU_CLK_SQUARE_DRIVER_REG, 1);
		hwlog_info("%s: Set PMU clock to square wave!\n", __func__);
	}
	return;
}

/*lint -restore*/
/*lint -save -e* */
void set_nfc_nxp_config_name(void)
{
	int ret = -1;

	memset_s(g_nfc_nxp_config_name, sizeof(g_nfc_nxp_config_name), 0x00, MAX_CONFIG_NAME_SIZE);
	ret = nfc_get_dts_config_string("nfc_nxp_name", "nfc_nxp_name",
					g_nfc_nxp_config_name, sizeof(g_nfc_nxp_config_name));
	if (ret != 0) {
		memset_s(g_nfc_nxp_config_name, sizeof(g_nfc_nxp_config_name), 0x00, MAX_CONFIG_NAME_SIZE);
		hwlog_err("%s: can't get nfc nxp config name\n", __func__);
		return;
	}
	hwlog_info("%s: nfc nxp config name:%s\n", __func__, g_nfc_nxp_config_name);
	return;
}

void set_nfc_brcm_config_name(void)
{
	int ret = -1;

	memset_s(g_nfc_brcm_conf_name, sizeof(g_nfc_brcm_conf_name), 0x00, MAX_CONFIG_NAME_SIZE);
	ret = nfc_get_dts_config_string("nfc_brcm_conf_name", "nfc_brcm_conf_name",
					g_nfc_brcm_conf_name, sizeof(g_nfc_brcm_conf_name));
	if (ret != 0) {
		memset_s(g_nfc_brcm_conf_name, sizeof(g_nfc_brcm_conf_name), 0x00, MAX_CONFIG_NAME_SIZE);
		hwlog_err("%s: can't get nfc brcm config name\n", __func__);
		return;
	}
	hwlog_info("%s: nfc brcm config name:%s\n", __func__, g_nfc_brcm_conf_name);
	return;
}

void set_nfc_single_channel(void)
{
	int ret = -1;
	char single_channel_dts_str[MAX_CONFIG_NAME_SIZE] = {0};

	memset_s(single_channel_dts_str, sizeof(single_channel_dts_str), 0x00, MAX_CONFIG_NAME_SIZE);
	ret = nfc_get_dts_config_string("nfc_single_channel", "nfc_single_channel",
		single_channel_dts_str, sizeof(single_channel_dts_str));
	if (ret != 0) {
		memset_s(single_channel_dts_str, sizeof(single_channel_dts_str), 0x00, MAX_CONFIG_NAME_SIZE);
		hwlog_err("%s: can't get nfc single channel dts config\n", __func__);
		g_nfc_single_channel = 0;
		return;
	}
	if (!strcasecmp(single_channel_dts_str, "true"))
		g_nfc_single_channel = 1;
	hwlog_info("%s: nfc single channel:%d\n", __func__, g_nfc_single_channel);
	return;
}

void set_nfc_spec_rf(void)
{
	int ret;
	char spec_rf_dts_str[MAX_CONFIG_NAME_SIZE] = {0};

	memset_s(spec_rf_dts_str, sizeof(spec_rf_dts_str), 0x00, MAX_CONFIG_NAME_SIZE);
	ret = nfc_get_dts_config_string("nfc_spec_rf", "nfc_spec_rf",
		spec_rf_dts_str, sizeof(spec_rf_dts_str));
	if (ret != 0) {
		memset_s(spec_rf_dts_str, sizeof(spec_rf_dts_str), 0x00, MAX_CONFIG_NAME_SIZE);
		hwlog_err("%s: can't get nfc spec rf dts config\n", __func__);
		g_nfc_spec_rf = 0;    // special rf config flag 0:normal
		return;
	}
	if (!strcasecmp(spec_rf_dts_str, "true"))
		g_nfc_spec_rf = 1;    // special rf config flag 1:special

	hwlog_info("%s: nfc spec rf:%d\n", __func__, g_nfc_spec_rf);
	return;
}

void get_nfc_delaytime_set(void)
{
	int ret;
	char delaytime_set_dts_str[MAX_CONFIG_NAME_SIZE];

	memset_s(delaytime_set_dts_str, sizeof(delaytime_set_dts_str), 0x00, MAX_CONFIG_NAME_SIZE);
	ret = nfc_get_dts_config_string("nfc_delaytime_set", "nfc_delaytime_set",
		delaytime_set_dts_str, sizeof(delaytime_set_dts_str));
	if (ret != 0) {
		memset_s(delaytime_set_dts_str, sizeof(delaytime_set_dts_str), 0x00, MAX_CONFIG_NAME_SIZE);
		hwlog_err("%s: can't get nfc delaytime set dts config\n", __func__);
		g_nfc_delaytime_set = 0;
		return;
	}
	if (!strcasecmp(delaytime_set_dts_str, "true"))
		g_nfc_delaytime_set = 1;

	hwlog_info("%s: nfc delaytime set:%d\n", __func__, g_nfc_delaytime_set);
	return;
}

void get_nfc_delaytime_node(void)
{
	int ret;

	ret = nfc_get_dts_config_u32("nfc_delaytime_node", "nfc_delaytime_node", &g_nfc_delaytime_node);
	if (ret != 0) {
		g_nfc_delaytime_node = 0;
		hwlog_err("%s: can't get nfc delaytime set dts config!\n", __func__);
	}
	hwlog_info("%s: nfc delaytime node:%d\n", __func__, g_nfc_delaytime_node);

	return;
}

void set_nfc_chip_type_name(void)
{
	int ret = -1;

	memset_s(g_nfc_chip_type, sizeof(g_nfc_chip_type), 0x00, MAX_NFC_CHIP_TYPE_SIZE);
	ret = nfc_get_dts_config_string("nfc_chip_type", "nfc_chip_type",
		g_nfc_chip_type, sizeof(g_nfc_chip_type));
	if (ret != 0) {
		memset_s(g_nfc_chip_type, sizeof(g_nfc_chip_type), 0x00, MAX_NFC_CHIP_TYPE_SIZE);
		hwlog_err("%s: can't get nfc nfc_chip_type, default pn547\n", __func__);
		ret = strcpy_s(g_nfc_chip_type, strlen("pn547") + 1, "pn547");
		if (ret != EOK)
			hwlog_err("strcpy_s err %x\n", ret);
	}

	hwlog_info("%s: nfc chip type name:%s\n", __func__, g_nfc_chip_type);

	return;
}

void set_nfc_card_num(void)
{
	int ret = -1;

	ret = nfc_get_dts_config_u32("nfc_card_num", "nfc_card_num", &g_nfc_card_num);
	if (ret != 0) {
		g_nfc_card_num = 1;
		hwlog_err("%s: can't get nfc card num config!\n", __func__);
	}
	return;
}

void set_nfc_ese_num(void)
{
	int result = -1;

	result = nfc_get_dts_config_u32("nfc_ese_num", "nfc_ese_num", &g_nfc_ese_num);
	if (result != 0) {
		g_nfc_ese_num = 1;
		hwlog_err("%s: get config fail!\n", __func__);
	}

	return;
}

void set_nfc_sim2_omapi(void)
{
	int ret = -1;
	char nfc_sim2_omapi_dts_str[MAX_CONFIG_NAME_SIZE] = {0};
	ret = nfc_get_dts_config_string("nfc_sim2_omapi", "nfc_sim2_omapi",
			nfc_sim2_omapi_dts_str, sizeof(nfc_sim2_omapi_dts_str));
	if (ret != 0) {
		g_nfc_sim2_omapi = 0;
		hwlog_err("%s: can't get nfc sim2 omapi config!\n", __func__);
		return;
	}
	if (!strcasecmp(nfc_sim2_omapi_dts_str, "true"))
		g_nfc_sim2_omapi = 1;
	hwlog_info("%s: nfc sim2 omapi:%d\n", __func__, g_nfc_sim2_omapi);
}

static int pn547_bulk_enable(struct  pn547_dev *pdev)
{
	int ret = 0;

	switch (g_nfc_on_type) {
	case NFC_ON_BY_GPIO:
		hwlog_info("%s: Nfc on by GPIO !\n", __func__);
		gpio_set_value(pdev->ven_gpio, 1);
		break;

	case NFC_ON_BY_REGULATOR_BULK:
		hwlog_info("%s: Nfc on by REGULATOR !\n", __func__);
		if (false == ven_felica_status) {
			ret = regulator_bulk_enable(1, &(pdev->ven_felica));
			if (ret < 0) {
				hwlog_err("%s: regulator_enable failed, ret:%d\n", __func__, ret);
			} else {
				ven_felica_status = true;
			}
		} else {
			hwlog_err("%s: ven already enable\n", __func__);
		}
		break;

	case NFC_ON_BY_HISI_PMIC:
	case NFC_ON_BY_HI6421V600_PMIC:
	case NFC_ON_BY_HI6555V110_PMIC:
	case NFC_ON_BY_HI6421V700_PMIC:
	case NFC_ON_BY_HI6421V800_PMIC:
	case NFC_ON_BY_HI6421V900_PMIC:
	case NFC_ON_BY_HI6555V300_PMIC:
	case NFC_ON_BY_HI6555V500_PMIC:
		hwlog_info("%s: Nfc on by HISI PMIC !\n", __func__);
		hisi_pmic_on();
		break;

	default:
		hwlog_err("%s: Unknown nfc on type !\n", __func__);
		break;
	}

	return ret;
}

static int pn547_bulk_disable(struct  pn547_dev *pdev)
{
	int ret = 0;

	if (pdev == NULL) {
		hwlog_err("%s: pdev is null !\n", __func__);
		return -1;
	}

	switch (g_nfc_on_type) {
	case NFC_ON_BY_GPIO:
		hwlog_info("%s: Nfc off by GPIO !\n", __func__);
		gpio_set_value(pdev->ven_gpio, 0);
		break;

	case NFC_ON_BY_REGULATOR_BULK:
		hwlog_info("%s: Nfc off by REGULATOR !\n", __func__);
		if (true == ven_felica_status) {
			ret = regulator_bulk_disable(1, &(pdev->ven_felica));
			if (ret < 0) {
				hwlog_err("%s: regulator_disable failed, ret:%d\n", __func__, ret);
			} else {
				ven_felica_status = false;
			}
		} else {
			hwlog_err("%s: ven already disable\n", __func__);
		}
		break;

	case NFC_ON_BY_HISI_PMIC:
	case NFC_ON_BY_HI6421V600_PMIC:
	case NFC_ON_BY_HI6555V110_PMIC:
	case NFC_ON_BY_HI6421V700_PMIC:
	case NFC_ON_BY_HI6421V800_PMIC:
	case NFC_ON_BY_HI6421V900_PMIC:
	case NFC_ON_BY_HI6555V300_PMIC:
	case NFC_ON_BY_HI6555V500_PMIC:
		hwlog_info("%s: Nfc off by HISI PMIC !\n", __func__);
		hisi_pmic_off();
		break;

	default:
		hwlog_err("%s: Unknown nfc off type !\n", __func__);
		break;
	}

	return ret;
}

static int pn547_enable_clk(struct	pn547_dev *pdev, int enableflg)
{
	int ret = 0;

	if (g_nfc_clk_src != NFC_CLK_SRC_CPU) {
		hwlog_info("%s: pmu clk is controlled by clk_req gpio or xtal .\n", __func__);
		return 0;
	}

	if (enableflg == g_clk_status_flag) {
		hwlog_info("%s: current nfc clk status is the same to enableflag [%d].\n", __func__, enableflg);
		return 0;
	}

	if (enableflg) {
		/* enable clock output */
		hwlog_info("%s: enable clock output\n", __func__);
		ret = pinctrl_select_state(pdev->pctrl, pdev->pins_default);
		if (ret < 0)
			hwlog_err("%s: unapply new state!\n", __func__);

		ret = clk_prepare_enable(pdev->nfc_clk);
		if (ret < 0)
			hwlog_err("%s: clk_enable failed, ret:%d\n", __func__, ret);
	} else {
		/* disable clock output */
		hwlog_info("%s: disable clock output\n", __func__);
		clk_disable_unprepare(pdev->nfc_clk);
		ret = pinctrl_select_state(pdev->pctrl, pdev->pins_idle);
		if (ret < 0)
			hwlog_err("%s: unapply new state!\n", __func__);
	}
	g_clk_status_flag = enableflg;

	return ret;
}

static int pn547_enable_nfc(struct pn547_dev *pdev)
{
	int ret = 0;

	/* enable chip */
	ret = pn547_bulk_enable(pdev);
	msleep(10);
	if (ret < 0) {
		return ret;
	}

	ret = pn547_bulk_disable(pdev);
	msleep(10);
	if (ret < 0) {
		return ret;
	}

	ret = pn547_bulk_enable(pdev);
	msleep(10);

	return ret;
}
/*lint -restore*/
/*lint -save -e* */
static void pn547_disable_irq(struct pn547_dev *pn547_dev)
{
	unsigned long flags;

	spin_lock_irqsave(&pn547_dev->irq_enabled_lock, flags);
	if (pn547_dev->irq_enabled) {
		disable_irq_nosync(pn547_dev->client->irq);
		pn547_dev->irq_enabled = false;
	}
	spin_unlock_irqrestore(&pn547_dev->irq_enabled_lock, flags);
}
/*lint -restore*/
/*lint -save -e* */
static void pn547_disable_irq_for_ext_gpio(struct pn547_dev *pn547_dev)
{
	mutex_lock(&pn547_dev->irq_mutex_lock);
	if (pn547_dev->irq_enabled) {
		disable_irq_nosync(pn547_dev->client->irq);
		pn547_dev->irq_enabled = false;
	}
	mutex_unlock(&pn547_dev->irq_mutex_lock);
}

static irqreturn_t pn547_dev_irq_handler(int irq, void *dev_id)
{
	struct pn547_dev *pn547_dev = dev_id;

	if (g_nfc_ext_gpio & IRQ_EXTENTED_GPIO_MASK) {
		pn547_disable_irq_for_ext_gpio(pn547_dev);
	} else {
		pn547_disable_irq(pn547_dev);
	}

	/* set a wakelock to avoid entering into suspend */
	if (WAKE_LOCK_TIMEOUT_ENALBE == g_wake_lock_timeout) {
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(5, 10, 0))
		__pm_wakeup_event(wlock_read, jiffies_to_msecs(5 * HZ));
#else
		__pm_wakeup_event(&wlock_read, jiffies_to_msecs(5 * HZ));
#endif
	} else {
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(5, 10, 0))
		__pm_wakeup_event(wlock_read, jiffies_to_msecs(1 * HZ));
#else
		__pm_wakeup_event(&wlock_read, jiffies_to_msecs(1 * HZ));
#endif
	}
	/* Wake up waiting readers */
	wake_up(&pn547_dev->read_wq);

	return IRQ_HANDLED;
}

// hide bank card number
uint16_t nfc_check_number(const char *string, uint16_t len)
{
	uint16_t i;
	if (string == NULL)
		return 0;
	for (i = 0; i < len; i++) {
		if (string[i] > '9' || string[i] < '0') // '9' & '0' means char must between 1-9
			return 0;
	}
	return 1; // 1 means success
}
// end

static ssize_t pn547_dev_read(struct file *filp, char __user *buf,
		size_t count, loff_t *offset)
{
	struct pn547_dev *pn547_dev = filp->private_data;
	char tmp[MAX_BUFFER_SIZE] = {0};
	char *tmpStr = NULL;
	int err = -1;
	int ret;
	int i;
	int retry;
	bool is_success = false;
	uint16_t buffer_count;
	uint16_t tmp_length;
	// 2 means ascii length of char, 1 means '\0'
	uint16_t tmp_size = sizeof(tmp) * 2 + 1;
	errno_t err_ret;

	if (count > MAX_BUFFER_SIZE) {
		count = MAX_BUFFER_SIZE;
	}

	mutex_lock(&pn547_dev->read_mutex);

	if (!nfc_gpio_get_value(pn547_dev->irq_gpio)) {
		if (filp->f_flags & O_NONBLOCK) {
			ret = -EAGAIN;
			goto fail;
		}

		if (g_nfc_ext_gpio & IRQ_EXTENTED_GPIO_MASK) {
			pn547_disable_irq_for_ext_gpio(pn547_dev);
		} else {
			pn547_disable_irq(pn547_dev);
		}

		pn547_dev->irq_enabled = true;
		enable_irq(pn547_dev->client->irq);
		ret = wait_event_interruptible(pn547_dev->read_wq,
		nfc_gpio_get_value(pn547_dev->irq_gpio));
		hwlog_err("%s: common ireq received\n", __func__);
		if (ret) {
			goto fail;
		}
	}

	tmpStr = (char *)kzalloc(sizeof(tmp)*2 + 1, GFP_KERNEL);
	if (!tmpStr) {
		hwlog_info("%s:Cannot allocate memory for read tmpStr.\n", __func__);
		ret = -ENOMEM;
		goto fail;
	}

	/* Read data, we have 3 chances */
	for (retry = 0; retry < NFC_TRY_NUM; retry++) {
		ret = i2c_master_recv(pn547_dev->client, tmp, (int)count);

		for (i = 0; i < count; i++) {
			err = snprintf_s(&tmpStr[i * 2], sizeof(tmp[i]) * 2 + 1, sizeof(tmp[i]) * 2 + 1, "%02X", tmp[i]);
			if (err < 0)
				hwlog_err("%s: snprintf_s is failed\n", __func__);
		}
		// 00 means Type A 0A means max UID len
		if ((count > MIN_NCI_CMD_LEN_WITH_DOOR_NUM) &&
			(tmp[NFC_A_PASSIVE] == 0x00) &&
			(tmp[NFCID_LEN] >= DOORCARD_MIN_LEN) &&
			(tmp[NFCID_LEN] <= 0x0A) && g_is_uid) {
				err_ret = memcpy_s(tmpStr + DOORCARD_OVERRIDE_LEN,
					tmp_size - DOORCARD_OVERRIDE_LEN,
					"XXXXXXXXXXXXXXXXXXXX",
					tmp[NFCID_LEN] * BYTE_CHAR_LEN);
				if (err_ret != EOK)
					pr_err("%s: memcpy_s fail,%d\n",
						__func__, err_ret);
		}
		if ((count > MIN_NCI_CMD_LEN_WITH_DOOR_NUM) &&
			(tmp[NCI_CMD_HEAD_OFFSET] == 0x10) && g_is_uid) {
			err_ret = memcpy_s(tmpStr + MIFARE_DATA_UID_OFFSET,
				tmp_size - MIFARE_DATA_UID_OFFSET,
				"XXXXXXXX",
				BANKCARD_NUM_OVERRIDE_LEN);
			if (err_ret != EOK)
				pr_err("%s: memcpy_s fail,%d\n",
				__func__, err_ret);
		}
		// hide bank card number,'6'&'2' means card number head
		buffer_count = count * 2 + 1; // 2 means ascii length of char, 1 means '\0'
		tmp_length = sizeof(tmp) * 2 + 1; // 2 means ascii length of char, 1 means '\0'
		if (buffer_count > MIN_NCI_CMD_LEN_WITH_BANKCARD_NUM) {
			for (i = NCI_CMD_HEAD_OFFSET; i < buffer_count - BANKCARD_NUM_LEN; i += BANKCARD_NUM_HEAD_LEN) {
				if ((*(tmpStr + i) == '6') && (*(tmpStr + i + 1) == '2') &&
					((i + BANKCARD_NUM_OVERRIDE_OFFSET) < tmp_length) &&
					nfc_check_number(tmpStr + i + BANKCARD_NUM_HEAD_LEN, BANKCARD_NUM_VALUE_LEN)) {
					err_ret = memcpy_s(tmpStr + i + BANKCARD_NUM_OVERRIDE_OFFSET, BANKCARD_NUM_OVERRIDE_LEN,
						"XXXXXXXX", BANKCARD_NUM_OVERRIDE_LEN);
					if (err_ret != EOK) {
						hwlog_err("%s memcpy_s fail\n", __func__);
					}
				}
			}
		}
		// end
		/* hide cplc
		 * 4 is start pos of 9F7F and length of 9F7F
		 * 18 is length of cplc and 2 is trans byte "9F" into two char
		 * 13 is length of print info and \0
		 */
		if ((2 * count > 18) && (strncmp(tmpStr + 4, "9F7F", 4) == 0))
			snprintf_s(tmpStr, sizeof("xxxx9F7Fxxxx") + 1, sizeof("xxxx9F7Fxxxx") + 1, "%s", "xxxx9F7Fxxxx");
		hwlog_info("%s : retry = %d, ret = %d, count = %3d > %s\n", __func__, retry, ret, (int)count, tmpStr);

		if (ret == (int)count) {
			is_success = true;
			break;
		} else {
			hwlog_info("%s : read data try =%d returned %d\n", __func__, retry, ret);
			msleep(1);
			continue;
		}
	}
	// 0&1 means '6105' or '000012' mifare, 2 means nci length
	if (((count >= FIRST_LINE_NCI_LEN) && (tmp[0] == 0x61) &&
		(tmp[1] == 0x05) && (tmp[2] >= 0x0A)) ||
		((tmp[0] == 0x00) && (tmp[1] == 0x00) && (tmp[2] == 0x12))) {
		g_is_uid = true;
	} else {
		g_is_uid = false;
	}
	kfree(tmpStr);
	tmpStr = NULL;
	mutex_unlock(&pn547_dev->read_mutex);
	if (false == is_success) {
		hwlog_err("%s : i2c_master_recv returned %d\n", __func__, ret);
		ret = -EIO;
	}

	if (ret < 0) {
		hwlog_err("%s: i2c_master_recv returned %d\n", __func__, ret);
		return ret;
	}
	if (ret > (int)count) {
		hwlog_err("%s: received too many bytes from i2c (%d)\n",
			__func__, ret);
		return -EIO;
	}
	g_pre_write_time = ktime_get_boottime();
	if (copy_to_user(buf, tmp, ret)) {
		hwlog_err("%s : failed to copy to user space\n", __func__);
		return -EFAULT;
	}
	return (size_t)ret;

fail:
	mutex_unlock(&pn547_dev->read_mutex);
	return (ssize_t)ret;
}

static ssize_t pn547_dev_write(struct file *filp, const char __user *buf,
		size_t count, loff_t *offset)
{
	struct pn547_dev  *pn547_dev;
	char data[MAX_BUFFER_SIZE] = {0};
	char *data_buf = NULL;
	char *tmpStr = NULL;
	int err;
	int ret;
	int retry;
	int i;
	bool is_success = false;
	char *pDataAlloc = NULL;
	char wakeup_cmd[1] = {0x20};
	int wakeup_len = 1;
	int retry_count = 0;
	ktime_t elapse_time = 0;
	ktime_t write_time;

	pn547_dev = filp->private_data;

	if (count > MAX_BUFFER_SIZE) {
		pDataAlloc = (char *)kzalloc(count, GFP_KERNEL);
		if (NULL == pDataAlloc) {
			hwlog_err("%s: memory kzalloc failed\n", __func__);
			return -ENOMEM;
		}
		data_buf = pDataAlloc;
	} else {
		data_buf = data;
	}


	if (copy_from_user(data_buf, buf, count)) {
		hwlog_err("%s : failed to copy from user space\n", __func__);
		if (NULL != pDataAlloc) {
			kfree(pDataAlloc);
			pDataAlloc = NULL;
		}
		return -EFAULT;
	}

	if (data_buf[0] != T1_HEAD) {
		write_time = ktime_get_boottime();
		elapse_time = write_time - g_pre_write_time;
		// make sure elapse_time is not overflow
		if (elapse_time < 0)
			elapse_time = I2C_ELAPSE_TIMEOUT;
		g_pre_write_time = write_time;
		if (elapse_time >= I2C_ELAPSE_TIMEOUT) {
			hwlog_info("%s : need to send 0x00\n", __func__);
			while (++retry_count < MAX_I2C_WAKEUP_TIME) {
				ret = i2c_master_send(pn547_dev->client, wakeup_cmd, wakeup_len);
				usleep_range(I2C_WAKEUP_SLEEP_TIME1, I2C_WAKEUP_SLEEP_TIME2);
				if (ret == wakeup_len) {
					break;
				}
			}
			if (ret < 0)
				hwlog_err("%s: failed to write wakeup_cmd : %d, retry for : %d times\n", __func__, ret, retry_count);
		}
	}

	tmpStr = (char *)kzalloc(count * 2 + 1, GFP_KERNEL);
	if (!tmpStr) {
		hwlog_info("%s:Cannot allocate memory for write tmpStr.\n", __func__);
		if (NULL != pDataAlloc) {
			pDataAlloc = NULL;
			kfree(pDataAlloc);
		}
			hwlog_err("%s:Cannot allocate memory for write tmpStr.\n",
				__func__);
		return -ENOMEM;
	}

	/* Write data, we have 3 chances */
	for (retry = 0; retry < NFC_TRY_NUM; retry++) {
		ret = i2c_master_send(pn547_dev->client, data_buf, (int)count);

		for (i = 0; i < count; i++) {
			err = snprintf_s(&tmpStr[i * 2], sizeof(data_buf[i]) * 2 + 1, sizeof(data_buf[i]) * 2 + 1, "%02X", data_buf[i]);
			if (err < 0)
				hwlog_err("%s: snprintf_s is failed\n", __func__);
		}

		hwlog_info("%s : retry = %d, ret = %d, count = %3d > %s\n", __func__, retry, ret, (int)count, tmpStr);

		if (ret == (int)count) {
			is_success = true;
			break;
		} else {
			if (retry > 0)
				hwlog_info("%s : send data try =%d returned %d\n", __func__, retry, ret);
			msleep(1);
			continue;
		}
	}
	kfree(tmpStr);
	tmpStr = NULL;
	if (false == is_success) {
		hwlog_err("%s : i2c_master_send returned %d\n", __func__, ret);
		ret = -EIO;
	}

	if (NULL != pDataAlloc) {
		kfree(pDataAlloc);
		pDataAlloc = NULL;
	}
	return (ssize_t)ret;
}

static int pn547_dev_open(struct inode *inode, struct file *filp)
{
	struct pn547_dev *pn547_dev = container_of(filp->private_data,
						struct pn547_dev,
						pn547_device);

	filp->private_data = pn547_dev;

	hwlog_info("%s : %d,%d\n", __func__, imajor(inode), iminor(inode));

	return 0;
}

static long pn547_dev_ioctl(struct file *filp,
				unsigned int cmd, unsigned long arg)
{
	struct pn547_dev *pn547_dev = filp->private_data;
	int ret  = 0;

	switch (cmd) {
	case PN547_SET_PWR:
		if (arg == 6) { // 6 means sn110 download complete
			hwlog_info("%s FW GPIO set to 0x00 >>>>>>>\n", __func__);
			nfc_gpio_set_value(pn547_dev->firm_gpio, 0);

			msleep(5);
		} else if (arg == 4) { // 4 means sn110 download start
			hwlog_info("%s FW dwldioctl called from NFC \n", __func__);
			nfc_gpio_set_value(pn547_dev->firm_gpio, 1);

			msleep(10);
		} else if (arg == 2) {
			/* power on with firmware download (requires hw reset)
			  */
			hwlog_info("%s power on with firmware\n", __func__);
			ret = pn547_bulk_enable(pn547_dev);
			if (ret < 0) {
				hwlog_err("%s: regulator_enable failed, ret:%d\n", __func__, ret);
				return ret;
			}
			msleep(1);
			nfc_gpio_set_value(pn547_dev->firm_gpio, 1);

			msleep(20);
			ret = pn547_bulk_disable(pn547_dev);
			if (ret < 0) {
				hwlog_err("%s: regulator_disable failed, ret:%d\n", __func__, ret);
				return ret;
			}

			msleep(60);
			ret = pn547_bulk_enable(pn547_dev);
			if (ret < 0) {
				hwlog_err("%s: regulator_enable failed, ret:%d\n", __func__, ret);
				return ret;
			}

			ret = pn547_enable_clk(pn547_dev, 1);
			if (ret < 0) {
				hwlog_err("%s: pn547_enable_clk failed, ret:%d\n", __func__, ret);
			}

			msleep(20);
		} else if (arg == 1) {
			/* power on */
			hwlog_info("%s power on\n", __func__);
			nfc_gpio_set_value(pn547_dev->firm_gpio, 0);
			ret = pn547_bulk_enable(pn547_dev);
			if (ret < 0) {
				hwlog_err("%s: regulator_enable failed, ret:%d\n", __func__, ret);
				return -EINVAL;
			}

			ret = pn547_enable_clk(pn547_dev, 1);
			if (ret < 0) {
				hwlog_err("%s: pn547_enable_clk failed, ret:%d\n", __func__, ret);
			}

			msleep(20);
		} else	if (arg == 0) {
			/* power off */
			hwlog_info("%s power off\n", __func__);
			nfc_gpio_set_value(pn547_dev->firm_gpio, 0);
			if (g_chip_type == NFCTYPE_SN110) {
				hwlog_info("%s sn110 power on\n", __func__);
			} else {
				ret = pn547_bulk_disable(pn547_dev);
				if (ret < 0) {
					hwlog_err("%s: regulator_disable failed, ret:%d\n", __func__, ret);
					return -EINVAL;
				}
			}

			ret = pn547_enable_clk(pn547_dev, 0);
			if (ret < 0) {
				hwlog_err("%s: pn547_disable_clk failed, ret:%d\n", __func__, ret);
			}

			msleep(60);
		} else {
			hwlog_err("%s bad arg %lu\n", __func__, arg);
			return -EINVAL;
		}
		break;
	case PN547_GET_CHIP_TYPE:
		ret = g_chip_type;
		return ret;
		break;
	default:
		hwlog_err("%s bad ioctl %u\n", __func__, cmd);
		return -EINVAL;
	}

	return 0;
}
/*lint -restore*/
/*lint -save -e* */
static const struct file_operations pn547_dev_fops = {
	.owner = THIS_MODULE,
	.llseek = no_llseek,
	.read = pn547_dev_read,
	.write = pn547_dev_write,
	.open = pn547_dev_open,
	.unlocked_ioctl = pn547_dev_ioctl,
#ifdef CONFIG_COMPAT
	.compat_ioctl = pn547_dev_ioctl,
#endif
};
/*lint -restore*/
/*lint -save -e* */
static ssize_t pn547_i2c_write(struct  pn547_dev *pdev, const char *buf, int count)
{
	int err;
	int ret;
	int retry;
	bool is_success = false;
	char *tmpStr = NULL;
	int i;

	tmpStr = (char *)kzalloc(255*2, GFP_KERNEL);
	if (!tmpStr) {
		hwlog_info("%s:Cannot allocate memory for write tmpStr.\n", __func__);
		return -ENOMEM;
	}

	/* Write data, we have 3 chances */
	for (retry = 0; retry < NFC_TRY_NUM; retry++) {
		ret = i2c_master_send(pdev->client, buf, (int)count);

		for (i = 0; i < count; i++) {
			err = snprintf_s(&tmpStr[i * 2], sizeof(buf[i]) * 2 + 1, sizeof(buf[i]) * 2 + 1, "%02X", buf[i]);
			if (err < 0)
				hwlog_err("%s: snprintf_s is failed\n", __func__);
		}

		hwlog_info("%s : retry = %d, ret = %d, count = %3d > %s\n", __func__, retry, ret, (int)count, tmpStr);

		if (ret == (int)count) {
			is_success = true;
			break;
		} else {
			if (retry > 0)
				hwlog_info("%s : send data try =%d returned %d\n", __func__, retry, ret);
			msleep(1);
			continue;
		}
	}
	kfree(tmpStr);
	tmpStr = NULL;
	if (false == is_success) {
		hwlog_err("%s : i2c_master_send returned %d\n", __func__, ret);
		ret = -EIO;
	}
	return (size_t)ret;
}

static ssize_t pn547_i2c_read(struct  pn547_dev *pdev, char *buf, int count)
{
	int err;
	int ret;
	int retry;
	bool is_success = false;
	char *tmpStr = NULL;
	int i;

	if (!nfc_gpio_get_value(pdev->irq_gpio)) {
		if (g_nfc_ext_gpio & IRQ_EXTENTED_GPIO_MASK) {
			pn547_disable_irq_for_ext_gpio(pdev);
		} else {
			pn547_disable_irq(pdev);
		}

		pdev->irq_enabled = true;
		enable_irq(pdev->client->irq);
		ret = wait_event_interruptible_timeout(pdev->read_wq,
		nfc_gpio_get_value(pdev->irq_gpio), msecs_to_jiffies(1000));
		if (ret <= 0) {
			ret  = -1;
			hwlog_err("%s : wait_event_interruptible_timeout error!\n", __func__);
			goto fail;
		}
	}

	tmpStr = (char *)kzalloc(255*2 + 1, GFP_KERNEL);
	if (!tmpStr) {
		hwlog_info("%s:Cannot allocate memory for write tmpStr.\n", __func__);
		return -ENOMEM;
	}

	/* Read data, we have 3 chances */
	for (retry = 0; retry < NFC_TRY_NUM; retry++) {
		ret = i2c_master_recv(pdev->client, buf, (int)count);

		for (i = 0; i < count; i++) {
			err = snprintf_s(&tmpStr[i * 2], sizeof(buf[i]) * 2 + 1, sizeof(buf[i]) * 2 + 1, "%02X", buf[i]);
			if (err < 0)
				hwlog_err("%s: snprintf_s is failed\n", __func__);
		}
		hwlog_info("%s : retry = %d, ret = %d, count = %3d > %s\n", __func__, retry, ret, (int)count, tmpStr);
		g_pre_write_time = ktime_get_boottime();
		if (ret == (int)count) {
			is_success = true;
			break;
		} else {
			hwlog_err("%s : read data try =%d returned %d\n", __func__, retry, ret);
			msleep(1);
			continue;
		}
	}
	kfree(tmpStr);
	tmpStr = NULL;
	if (false == is_success) {
		hwlog_err("%s : i2c_master_recv returned %d\n", __func__, ret);
		ret = -EIO;
	}
	return (size_t)ret;
fail:
	return (size_t)ret;
}
/*lint -restore*/
/*lint -save -e* */
static int check_sim_status(struct i2c_client *client, struct  pn547_dev *pdev)
{
	int ret;

	unsigned char recv_buf[40] = {0};
	const  char send_reset[] = {0x20, 0x00, 0x01, 0x00};
	const  char init_cmd[] = {0x20, 0x01, 0x02, 0x00, 0x00};

	const  char read_config[] = {0x2F, 0x02, 0x00};
	const  char read_config_uicc[] = {0x2F, 0x3E, 0x01, 0x00}; /* swp1 */
	const  char read_config_ese[] = {0x2F, 0x3E, 0x01, 0x01}; /* eSE */
	unsigned int status = 0;

	/* hardware reset */
	/* power on */
	nfc_gpio_set_value(pdev->firm_gpio, 0);
	ret = pn547_bulk_enable(pdev);
	if (ret < 0) {
		hwlog_err("%s: regulator_enable failed, ret:%d\n", __func__, ret);
		goto failed;
	}
	msleep(20);

	/* power off */
	ret = pn547_bulk_disable(pdev);
	if (ret < 0) {
		hwlog_err("%s: regulator_disable failed, ret:%d\n", __func__, ret);
		goto failed;
	}
	msleep(60);

	/* power on */
	ret = pn547_bulk_enable(pdev);
	if (ret < 0) {
		hwlog_err("%s: regulator_enable failed, ret:%d\n", __func__, ret);
		goto failed;
	}
	msleep(20);

	/* write CORE_RESET_CMD */
	ret = pn547_i2c_write(pdev, send_reset, sizeof(send_reset));
	if (ret < 0) {
		hwlog_err("%s: pn547_i2c_write failed, ret:%d\n", __func__, ret);
		goto failed;
	}
	/* read response */
	ret = pn547_i2c_read(pdev, recv_buf, recv_buf[2]);
	if (ret < 0) {
		hwlog_err("%s: pn547_i2c_read failed, ret:%d\n", __func__, ret);
		goto failed;
	}

	udelay(500);
	/* write CORE_INIT_CMD */
	ret = pn547_i2c_write(pdev, init_cmd, sizeof(init_cmd));
	if (ret < 0) {
		hwlog_err("%s: pn547_i2c_write failed, ret:%d\n", __func__, ret);
		goto failed;
	}
	/* read response */
	ret = pn547_i2c_read(pdev, recv_buf, recv_buf[2]);
	if (ret < 0) {
		hwlog_err("%s: pn547_i2c_read failed, ret:%d\n", __func__, ret);
		goto failed;
	}

	udelay(500);
	/* write NCI_PROPRIETARY_ACT_CMD */
	ret = pn547_i2c_write(pdev, read_config, sizeof(read_config));
	if (ret < 0) {
		hwlog_err("%s: pn547_i2c_write failed, ret:%d\n", __func__, ret);
		goto failed;
	}
	/* read response */
	ret = pn547_i2c_read(pdev, recv_buf, recv_buf[2]);
	if (ret < 0) {
		hwlog_err("%s: pn547_i2c_read failed, ret:%d\n", __func__, ret);
		goto failed;
	}

	udelay(500);

	/* write TEST_SWP_CMD UICC */
	ret = pn547_i2c_write(pdev, read_config_uicc, sizeof(read_config_uicc));
	if (ret < 0) {
		hwlog_err("%s: pn547_i2c_write failed, ret:%d\n", __func__, ret);
		goto failed;
	}
	/* read response */
	ret = pn547_i2c_read(pdev, recv_buf, recv_buf[2]);
	if (ret < 0) {
		hwlog_err("%s: pn547_i2c_read failed, ret:%d\n", __func__, ret);
		goto failed;
	}

	mdelay(10);
	/* read notification */
	ret = pn547_i2c_read(pdev, recv_buf, recv_buf[2]);
	if (ret < 0) {
		hwlog_err("%s: pn547_i2c_read failed, ret:%d\n", __func__, ret);
		goto failed;
	}

	/* NTF's format: 6F 3E 02 XX1 XX2 -> "XX1 == 0" means SWP link OK "XX1 == 3" means SWP link Failed */
	if (recv_buf[0] == 0x6F && recv_buf[1] == 0x3E && recv_buf[3] == 0x00) {
		status |= UICC_SUPPORT_CARD_EMULATION;
	}

	/*write TEST_SWP_CMD eSE*/
	ret = pn547_i2c_write(pdev, read_config_ese, sizeof(read_config_ese));
	if (ret < 0) {
		hwlog_err("%s: pn547_i2c_write failed, ret:%d\n", __func__, ret);
		goto failed;
	}
	/*read response*/
	ret = pn547_i2c_read(pdev, recv_buf, recv_buf[2]);
	if (ret < 0) {
		hwlog_err("%s: pn547_i2c_read failed, ret:%d\n", __func__, ret);
		goto failed;
	}

	mdelay(10);
	/*read notification*/
	ret = pn547_i2c_read(pdev, recv_buf, recv_buf[2]);
	if (ret < 0) {
		hwlog_err("%s: pn547_i2c_read failed, ret:%d\n", __func__, ret);
		goto failed;
	}

	if (recv_buf[0] == 0x6F && recv_buf[1] == 0x3E && recv_buf[3] == 0x00) {
		status |= eSE_SUPPORT_CARD_EMULATION;
	}

	pdev->sim_status = (int)status;
	return pdev->sim_status;
failed:
	pdev->sim_status = ret;
	return ret;
}

static void gt_set_idle(struct pn547_dev *pdev)
{
	int ret;
	/* 10 bytes read response buf for idle configure */
	unsigned char recv_buf[10] = {0};
	/* Set NFCC to Idle */
	const char idle_config[] = { 0x21, 0x06, 0x01, 0x00 };

	/* write idle_config */
	ret = pn547_i2c_write(pdev, idle_config, sizeof(idle_config));
	if (ret < 0)
		hwlog_err("%s: pn547_i2c_write failed, ret:%d\n", __func__, ret);

	ret = pn547_i2c_read(pdev, recv_buf, sizeof(recv_buf));
	if (ret < 0)
		hwlog_err("%s: pn547_i2c_read failed, ret:%d\n", __func__, ret);

	/* delay 500us for idle config */
	udelay(500);
}

static int gt_dcdc_config(struct pn547_dev *pdev, const char *dcdc_config)
{
	int ret;
	int retry;
	/* 10 bytes read response buf for idle configure */
	unsigned char recv_buf[10] = {0};
	/* read register cmd length = 18 */
	const int nfc_cmd_length = 18;

	/* dcdc_config */
	for (retry = 0; retry < NFC_TRY_NUM; retry++) {
		ret = pn547_i2c_write(pdev, dcdc_config, nfc_cmd_length);
		if (ret < 0) {
			hwlog_err("%s: pn547_i2c_write failed, ret:%d\n", __func__, ret);
			return -1;
		}

		ret = pn547_i2c_read(pdev, recv_buf, sizeof(recv_buf));
		if (ret < 0) {
			hwlog_err("%s: pn547_i2c_read failed, ret:%d\n", __func__, ret);
			msleep(1);
			continue;
		}
		/* recv_buf[3] means status, 00 means status OK */
		if (recv_buf[3] == 0x00) {
			hwlog_info("%s : dcdc config successful", __func__);
			break;
		} else {
			hwlog_info("%s : dcdc config try =%d returned status =%02X\n",
				__func__, retry, recv_buf[3]);
			/* delay 1ms for retry */
			msleep(1);
			continue;
		}
	}
	/* response recv_buf[3] != 0x00 means config status error */
	if (recv_buf[3] != 0x00)
		return -1;

	return 1;
}

static int gt_boost_config(struct pn547_dev *pdev)
{
	int err;
	int ret;
	int retry;
	/* read register response length = 19 */
	const int nfc_response_length = 19;
	/* recv_config_buf: read register response buf */
	unsigned char recv_config_buf[19] = {0};
	/* NXP_EXT_TVDD_CFG_2 configure cmd, cmd length = 18 */
	char dcdc_config[18] = {
		0x20, 0x02, 0x0F, 0x01, 0xA0, 0x0E, 0x0B, 0x11, 0x01,
		0xC2, 0xC2, 0x00, 0xBA, 0x26, 0x13, 0x00, 0xD0, 0xFF
	};
	/* read register cmd/recv head_length = 7/8 */
	const int cmd_head_length = 7;
	const int recv_head_length = 8;
	/* Read register A00E cmd */
	const char get_config[] = { 0x20, 0x03, 0x03, 0x01, 0xA0, 0x0E };

	gt_set_idle(pdev);

	/* get configure from register A00E */
	for (retry = 0; retry < NFC_TRY_NUM; retry++) {
		ret = pn547_i2c_write(pdev, get_config, sizeof(get_config));
		if (ret < 0) {
			hwlog_err("%s: write get_dcdc_config failed, ret:%d\n",
				__func__, ret);
			continue;
		}

		ret = pn547_i2c_read(pdev, recv_config_buf, sizeof(recv_config_buf));
		if (ret < 0) {
			hwlog_err("%s: pn547_i2c_read failed, ret:%d\n", __func__, ret);
			continue;
		}

		/* 4003 means register read response, 00 means status OK */
		if ((recv_config_buf[0] == 0x40) && (recv_config_buf[1] == 0x03)
			&& (recv_config_buf[3] == 0x00)) {
			hwlog_info("%s: get_dcdc_config successful\n", __func__);
			err = memcpy_s(dcdc_config + cmd_head_length, nfc_response_length - recv_head_length,
				recv_config_buf + recv_head_length, nfc_response_length - recv_head_length);
			if (err != EOK)
				hwlog_err("%s memcpy_s fail\n", __func__);

			/* 12 is tx_p_req position, BA means enabled */
			/* 13 is DCDC parameter position, 26 is recommended parameter */
			dcdc_config[12] = 0xBA;
			dcdc_config[13] = 0x26;
			break;
		} else {
			hwlog_info("%s: read register try =%d, returned status =%02X\n",
				__func__, retry, recv_config_buf[3]);
			continue;
		}
	}

	/* 4003 means register read response, 00 means status OK */
	if (!((recv_config_buf[0] == 0x40) && (recv_config_buf[1] == 0x03)
		&& (recv_config_buf[3] == 0x00))) {
		hwlog_err("%s : read register failed, aborting...", __func__);
		return -1;
	}

	ret = gt_dcdc_config(pdev, (const char *)dcdc_config);
	return ret;
}

int set_fwupdate_on_info(char info)
{
	char hex_para[TEL_HUAWEI_NV_NFCCAL_LEGTH] = {0};
	/* read nfc configure value from nvm */
	int ret;
	ret = nfc_nv_opera(NV_READ, TEL_HUAWEI_NV_NFCCAL_NUMBER,
			   TEL_HUAWEI_NV_NFCCAL_NAME,
			   TEL_HUAWEI_NV_NFCCAL_LEGTH, hex_para);
	hwlog_info("%s: enter!\n", __func__);
	if (ret < 0) {
		hwlog_err("%s: get nv error ret: %d!\n", __func__, ret);
		return -1;
	}
	/* write nfc configure value to nvm */
	hex_para[FWUPDATE_ON_OFFSET] = info;
	ret = nfc_nv_opera(NV_WRITE, TEL_HUAWEI_NV_NFCCAL_NUMBER, TEL_HUAWEI_NV_NFCCAL_NAME, TEL_HUAWEI_NV_NFCCAL_LEGTH, hex_para);
	if (ret < 0) {
		hwlog_err("%s: set nv error ret: %d!\n", __func__, ret);
		return -1;
	}
	return 0;
}

int get_fwupdate_on_info(char *info)
{
	char hex_para[TEL_HUAWEI_NV_NFCCAL_LEGTH] = {0};
	/* read nfc configure value from nvm */
	int ret;
	ret = nfc_nv_opera(NV_READ, TEL_HUAWEI_NV_NFCCAL_NUMBER, TEL_HUAWEI_NV_NFCCAL_NAME, TEL_HUAWEI_NV_NFCCAL_LEGTH, hex_para);
	hwlog_info("%s: enter!\n", __func__);
	if (ret < 0) {
		hwlog_err("%s: get nv error ret: %d!\n", __func__, ret);
		return -1;
	}
	*info = hex_para[FWUPDATE_ON_OFFSET];
	hwlog_info("%s: info = %d.\n", __func__, (int)*info);
	return 0;
}

static ssize_t nfc_fwupdate_on_show(struct device *dev, struct device_attribute *attr, char *buf)
{
		char fwupdate_flag = 0;
		(void)get_fwupdate_on_info(&fwupdate_flag);
		hwlog_info("%s: read fwupdate_on, returned =%d\n", __func__,  (unsigned char)fwupdate_flag);

		return (ssize_t)(snprintf_s(buf, MAX_ATTRIBUTE_BUFFER_SIZE, MAX_ATTRIBUTE_BUFFER_SIZE - 1,
			"%d\n", fwupdate_flag));
}

static ssize_t nfc_fwupdate_store(struct device *dev, struct device_attribute *attr,
			const char *buf, size_t count)
{
	if ((buf != NULL) && (buf[0] >= CHAR_0) && (buf[0] <= CHAR_9)) {
		firmware_update = buf[0] - CHAR_0;;
		hwlog_info("%s:firmware update success: %d\n", __func__, firmware_update);
	}

	return (ssize_t)count;
}

static ssize_t nfc_fwupdate_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	return (ssize_t)(snprintf_s(buf, sizeof(firmware_update) + 1, sizeof(firmware_update),
		"%d", firmware_update));
}

static ssize_t nfc_switch_state_store(struct device *dev, struct device_attribute *attr,
			const char *buf, size_t count)
{
	if ((buf!=NULL) && (buf[0]>=CHAR_0) && (buf[0]<=CHAR_9))
		nfc_switch_state = buf[0] - CHAR_0; /* file storage str */
	return (ssize_t)count;
}

static ssize_t nfc_switch_state_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	return (ssize_t)(snprintf_s(buf, sizeof(nfc_switch_state) + 1, sizeof(nfc_switch_state),
		"%d", nfc_switch_state));
}

static ssize_t nfc_at_result_store(struct device *dev, struct device_attribute *attr,
			const char *buf, size_t count)
{
	if ((buf!=NULL) && (buf[0]>=CHAR_0) && (buf[0]<=CHAR_9))
		nfc_at_result = buf[0] - CHAR_0; /* file storage str */
	return (ssize_t)count;
}

static ssize_t nfc_at_result_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	return (ssize_t)(snprintf_s(buf, sizeof(nfc_at_result) + 1, sizeof(nfc_at_result),
		"%d", nfc_at_result));
}

static ssize_t nfc_get_rssi_store(struct device *dev, struct device_attribute *attr,
			const char *buf, size_t count)
{
	int i = 0;
	int flag = 1;
    nfc_get_rssi = 0;
    //hwlog_info("%s:%s,%d, %d\n", __func__, buf, nfc_get_rssi, count);
	if (buf!=NULL) {
		if (buf[0] == '-') {
			flag = -1;
			i++;
		}
		while (buf[i] != '\0') {
			if ((buf[i] >= CHAR_0) && (buf[i] <= CHAR_9) && (nfc_get_rssi <= RSSI_MAX))
				nfc_get_rssi = (long)(nfc_get_rssi * 10) + (buf[i] - CHAR_0); /* file storage str */
			i++;
		}
		nfc_get_rssi = flag * nfc_get_rssi;
	}
	return (ssize_t)count;
}

static ssize_t nfc_get_rssi_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	hwlog_info("%s:%s,%ld\n", __func__, buf, nfc_get_rssi);
	return (ssize_t)(snprintf_s(buf, MAX_SIZE + 1, MAX_SIZE, "%ld", nfc_get_rssi));
}


static ssize_t nxp_config_name_store(struct device *dev, struct device_attribute *attr,
			const char *buf, size_t count)
{
	return (ssize_t)count;
}

static ssize_t nxp_config_name_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	return (ssize_t)(snprintf_s(buf, strlen(g_nfc_nxp_config_name) + 1, strlen(g_nfc_nxp_config_name),
		"%s", g_nfc_nxp_config_name));
}

static ssize_t nfc_brcm_conf_name_store(struct device *dev, struct device_attribute *attr,
			const char *buf, size_t count)
{
	return (ssize_t)count;
}
static ssize_t nfc_brcm_conf_name_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	return (ssize_t)(snprintf_s(buf, strlen(g_nfc_brcm_conf_name) + 1, strlen(g_nfc_brcm_conf_name),
		"%s", g_nfc_brcm_conf_name));
}
/* lint -restore */
/*
	function: check which sim card support card Emulation.
	return value:
		eSE		UICC	value
		0		  0			 0			(not support)
		0		  1			 1			(swp1 support)
		1		  0			 2			(swp2 support)
		1		  1			 3			(all support)
		<0>	:error
*/
/* lint -save -e* */
static ssize_t nfc_sim_status_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	int status = -1;
	struct i2c_client *i2c_client_dev = container_of(dev, struct i2c_client, dev);
	struct pn547_dev *pn547_dev;
	pn547_dev = i2c_get_clientdata(i2c_client_dev);
	if (pn547_dev == NULL) {
		hwlog_err("%s: pn547_dev == NULL!\n", __func__);
		return status;
	}
	hwlog_info("%s: enter!\n", __func__);
	status = check_sim_status(i2c_client_dev, pn547_dev);
	if (status < 0)
		hwlog_err("%s: check_sim_status error!\n", __func__);
	hwlog_info("%s: status=%d\n", __func__, status);
	return (ssize_t)(snprintf_s(buf, MAX_ATTRIBUTE_BUFFER_SIZE, MAX_ATTRIBUTE_BUFFER_SIZE - 1, "%d\n", status));
}

static ssize_t nfc_gt_boost_show(struct device *dev,
	struct device_attribute *attr,
	char *buf)
{
	int status = -1;  // -1: error; 0: not supported on board; 1: successful
	struct i2c_client *i2c_client_dev = container_of(dev, struct i2c_client, dev);
	struct pn547_dev *pn547_dev;
	pn547_dev = i2c_get_clientdata(i2c_client_dev);
	if (pn547_dev == NULL) {
		hwlog_err("%s: pn547_dev == NULL!\n", __func__);
		return status;
	}
	hwlog_info("%s: enter!\n", __func__);
	if (g_is_gtboost) {
		status = gt_boost_config(pn547_dev);
		hwlog_info("%s: status=%d\n", __func__, status);
	} else {
		status = 0;
		hwlog_info("%s: gt_boost not supported on board\n", __func__);
	}
	return (ssize_t)(snprintf_s(buf, MAX_ATTRIBUTE_BUFFER_SIZE, MAX_ATTRIBUTE_BUFFER_SIZE - 1, "%d\n", status));
}


static ssize_t nfc_sim_switch_store(struct device *dev, struct device_attribute *attr,
			const char *buf, size_t count)
{
	struct i2c_client *i2c_client_dev = container_of(dev, struct i2c_client, dev);
	struct pn547_dev *pn547_dev;
	int val = 0;

	pn547_dev = i2c_get_clientdata(i2c_client_dev);
	if (pn547_dev == NULL) {
		hwlog_err("%s: pn547_dev == NULL!\n", __func__);
		return 0;
	}
	if (sscanf_s(buf, "%1d", &val) == 1) {
		if (val >= 1 && val <= 3) {
			pn547_dev->sim_switch = val;
		} else {
			return -EINVAL;
		}
	} else {
		return -EINVAL;
	}
	return (ssize_t)count;
}

static ssize_t nfc_sim_switch_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	struct i2c_client *i2c_client_dev = container_of(dev, struct i2c_client, dev);
	struct pn547_dev *pn547_dev;
	pn547_dev = i2c_get_clientdata(i2c_client_dev);
	if (pn547_dev == NULL) {
		hwlog_err("%s: pn547_dev == NULL!\n", __func__);
		return 0;
	}
	return (ssize_t)(snprintf_s(buf, MAX_ATTRIBUTE_BUFFER_SIZE, MAX_ATTRIBUTE_BUFFER_SIZE - 1, "%d\n", pn547_dev->sim_switch));
}
static ssize_t rd_nfc_sim_status_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	int status = -1;
	struct i2c_client *i2c_client_dev = container_of(dev, struct i2c_client, dev);
	struct pn547_dev *pn547_dev;
	pn547_dev = i2c_get_clientdata(i2c_client_dev);
	if (pn547_dev == NULL) {
		hwlog_err("%s:	pn547_dev == NULL!\n", __func__);
		return status;
	}
	return (ssize_t)(snprintf_s(buf, MAX_ATTRIBUTE_BUFFER_SIZE, MAX_ATTRIBUTE_BUFFER_SIZE - 1, "%d\n", pn547_dev->sim_status));
}
static ssize_t nfc_enable_status_store(struct device *dev, struct device_attribute *attr,
			 const char *buf, size_t count)
{
	struct i2c_client *i2c_client_dev = container_of(dev, struct i2c_client, dev);
	struct pn547_dev *pn547_dev;
	int val = 0;

	pn547_dev = i2c_get_clientdata(i2c_client_dev);
	if (pn547_dev == NULL) {
		hwlog_err("%s: pn547_dev == NULL!\n", __func__);
		return 0;
	}
	if (sscanf_s(buf, "%1d", &val) == 1) {
		if (val == ENABLE_START) {
			pn547_dev->enable_status = ENABLE_START;
		} else if (val == ENABLE_END) {
			pn547_dev->enable_status = ENABLE_END;
		} else {
			return -EINVAL;
		}
	} else {
		return -EINVAL;
	}
	return (ssize_t)count;
}
static ssize_t nfc_enable_status_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	struct i2c_client *i2c_client_dev = container_of(dev, struct i2c_client, dev);
	struct pn547_dev *pn547_dev;
	pn547_dev = i2c_get_clientdata(i2c_client_dev);
	if (pn547_dev == NULL) {
		hwlog_err("%s: pn547_dev == NULL!\n", __func__);
		return 0;
	}
	return (ssize_t)(snprintf_s(buf, MAX_ATTRIBUTE_BUFFER_SIZE, MAX_ATTRIBUTE_BUFFER_SIZE - 1,
	"%d\n", pn547_dev->enable_status));
}
static ssize_t nfc_card_num_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	return (ssize_t)(snprintf_s(buf, MAX_ATTRIBUTE_BUFFER_SIZE, MAX_ATTRIBUTE_BUFFER_SIZE - 1,
		"%d\n", g_nfc_card_num));
}

static ssize_t nfc_ese_num_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	return (ssize_t)(snprintf_s(buf, MAX_ATTRIBUTE_BUFFER_SIZE, MAX_ATTRIBUTE_BUFFER_SIZE - 1,
		"%d\n", g_nfc_ese_num));
}

static ssize_t nfc_delaytime_node_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	return (ssize_t)(snprintf_s(buf, MAX_ATTRIBUTE_BUFFER_SIZE, MAX_ATTRIBUTE_BUFFER_SIZE - 1,
		"%d\n", g_nfc_delaytime_node));
}

static ssize_t nfc_fw_version_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	int ret = -1;

	memset_s(g_nfc_fw_version, sizeof(g_nfc_fw_version), 0x00, MAX_NFC_FW_VERSION_SIZE);
	ret = nfc_get_dts_config_string("nfc_fw_version", "nfc_fw_version",
		g_nfc_fw_version, sizeof(g_nfc_fw_version));
	if (ret != 0) {
		memset_s(g_nfc_fw_version, sizeof(g_nfc_fw_version), 0x00, MAX_NFC_FW_VERSION_SIZE);
		hwlog_err("%s: can't get nfc g_nfc_fw_version, default FW 8.1.24\n", __func__);
		ret = strcpy_s(g_nfc_fw_version, strlen("FW 8.1.24") + 1, "FW 8.1.24");
		if (ret != EOK)
			hwlog_err("strcpy_s err %x\n", ret);
	}
	return (ssize_t)(snprintf_s(buf, MAX_ATTRIBUTE_BUFFER_SIZE, MAX_ATTRIBUTE_BUFFER_SIZE - 1,
		"%s", g_nfc_fw_version));
}
static ssize_t nfc_chip_type_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	return (ssize_t)(snprintf_s(buf, strlen(g_nfc_chip_type) + 1, strlen(g_nfc_chip_type),
		"%s", g_nfc_chip_type));
}

static ssize_t nfc_chip_type_store(struct device *dev, struct device_attribute *attr,
			const char *buf, size_t count)
{
	int ret;

	hwlog_err("%s: %s count=%lu\n", __func__, buf, count);
	ret = strncpy_s(g_nfc_chip_type, sizeof(g_nfc_chip_type),
		buf, MAX_NFC_CHIP_TYPE_SIZE - 1);
	if (ret != 0)
		hwlog_err("%s: strncpy_s fail\n", __func__);
	return (ssize_t)count;
}


static ssize_t nfcservice_lock_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	return (ssize_t)(snprintf_s(buf, MAX_ATTRIBUTE_BUFFER_SIZE, MAX_ATTRIBUTE_BUFFER_SIZE - 1,
		"%d", g_nfcservice_lock));
}

static ssize_t nfcservice_lock_store(struct device *dev, struct device_attribute *attr,
			 const char *buf, size_t count)
{
	int val = 0;

	if (sscanf_s(buf, "%1d", &val) == 1) {
		g_nfcservice_lock = val;
	} else {
		hwlog_err("%s: set g_nfcservice_lock error\n", __func__);
		g_nfcservice_lock = 0;
	}
	hwlog_info("%s: g_nfcservice_lock:%d\n", __func__, g_nfcservice_lock);
	return (ssize_t)count;
}

static ssize_t nfc_svdd_sw_store(struct device *dev, struct device_attribute *attr,
			 const char *buf, size_t count)
{
	int val = 0;

	if (sscanf_s(buf, "%1d", &val) == 1) {
		switch (val) {
		case NFC_SVDD_SW_OFF:
			pmu0_svdd_sel_off();
			break;
		case NFC_SVDD_SW_ON:
			pmu0_svdd_sel_on();
			break;
		default:
			hwlog_err("%s: svdd switch error, val:%d[0:pulldown, 1:pullup]\n", __func__, val);
			break;
		}
	} else {
		hwlog_err("%s: val len error\n", __func__);
		return (ssize_t)count;
	}
	hwlog_info("%s: nfc svdd switch to %d\n", __func__, val);
	return (ssize_t)count;
}

static ssize_t nfc_svdd_sw_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	int sw_status = 0;

	sw_status = get_pmu0_svdd_sel_status();
	return (ssize_t)(snprintf_s(buf, MAX_ATTRIBUTE_BUFFER_SIZE, MAX_ATTRIBUTE_BUFFER_SIZE - 1, "%d", sw_status));
}

static ssize_t nfc_close_type_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	return (ssize_t)(snprintf_s(buf, MAX_ATTRIBUTE_BUFFER_SIZE, MAX_ATTRIBUTE_BUFFER_SIZE - 1, "%d", g_nfc_close_type));
}

static ssize_t nfc_close_type_store(struct device *dev, struct device_attribute *attr,
			 const char *buf, size_t count)
{
	int val = 0;
	if (sscanf_s(buf, "%1d", &val) == 1) {
		g_nfc_close_type = val;
	} else {
		hwlog_err("%s: set g_nfc_close_type error\n", __func__);
		g_nfc_close_type = 0;
	}
	hwlog_info("%s: g_nfc_close_type:%d\n", __func__, g_nfc_close_type);
	return (ssize_t)count;
}

static ssize_t nfc_single_channel_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	return (ssize_t)(snprintf_s(buf, MAX_ATTRIBUTE_BUFFER_SIZE, MAX_ATTRIBUTE_BUFFER_SIZE - 1,
		"%d", g_nfc_single_channel));
}

static ssize_t nfc_spec_rf_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	return (ssize_t)(snprintf_s(buf, MAX_ATTRIBUTE_BUFFER_SIZE, MAX_ATTRIBUTE_BUFFER_SIZE - 1,
		"%d", g_nfc_spec_rf));
}

static ssize_t nfc_delaytime_set_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	return (ssize_t)(snprintf_s(buf, MAX_ATTRIBUTE_BUFFER_SIZE, MAX_ATTRIBUTE_BUFFER_SIZE - 1,
		"%d", g_nfc_delaytime_set));
}

static ssize_t nfc_sim2_omapi_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	return (ssize_t)(snprintf_s(buf, MAX_ATTRIBUTE_BUFFER_SIZE, MAX_ATTRIBUTE_BUFFER_SIZE - 1,
		"%d", g_nfc_sim2_omapi));
}

static ssize_t nfc_wired_ese_info_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	return (ssize_t)(snprintf_s(buf, MAX_ATTRIBUTE_BUFFER_SIZE, MAX_ATTRIBUTE_BUFFER_SIZE - 1,
		"%d", g_nfc_ese_type));
}

static ssize_t nfc_wired_ese_info_store(struct device *dev, struct device_attribute *attr,
		const char *buf, size_t count)
{
	int val = 0;
	if (sscanf_s(buf, "%1d", &val) == 1) {
		g_nfc_ese_type = val;
	} else {
		hwlog_err("%s: set g_nfc_ese_type  error\n", __func__);
	}
	hwlog_info("%s: g_nfc_ese_type:%d\n", __func__, g_nfc_ese_type);
	return (ssize_t)count;
}
static ssize_t nfc_activated_se_info_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	return (ssize_t)(snprintf_s(buf, MAX_ATTRIBUTE_BUFFER_SIZE, MAX_ATTRIBUTE_BUFFER_SIZE - 1,
		"%d", g_nfc_activated_se_info));
}

static ssize_t nfc_activated_se_info_store(struct device *dev, struct device_attribute *attr,
			 const char *buf, size_t count)
{
	int val = 0;
	if (sscanf_s(buf, "%1d", &val) == 1) {
		g_nfc_activated_se_info = val;
	} else {
		hwlog_err("%s: set g_nfc_activated_se_info error\n", __func__);
		g_nfc_activated_se_info = 0;
	}
	hwlog_info("%s: g_nfc_activated_se_info:%d\n", __func__, g_nfc_activated_se_info);
	return (ssize_t)count;
}

static ssize_t nfc_hal_dmd_info_store(struct device *dev, struct device_attribute *attr,
			 const char *buf, size_t count)
{
	long val = 0;
	char dmd_info_from_hal[64] = {'\0'};
	/* The length of DMD error number is 9. */
	if (sscanf_s(buf, "%9ld", &val) == 1) {
		if (val < NFC_DMD_NUMBER_MIN || val > NFC_DMD_NUMBER_MAX)
			return (ssize_t)count;
		g_nfc_hal_dmd_no = val;
		/* The max length of content for current dmd description set as 63.
		   Example for DMD Buf: '923002014 CoreReset:600006A000D1A72000'.
		   A space as a separator is between dmd error no and description. */
		if (sscanf_s(buf, "%*s%63s", dmd_info_from_hal) == 1)
			nfc_record_dmd_info(val, dmd_info_from_hal);
	} else {
		hwlog_err("%s: get dmd number error\n", __func__);
	}
	return (ssize_t)count;
}

/* lint -e516 -e515 -e717 -e960 -e712 -e747 */
int nfc_record_dmd_info(long dmd_no, const char *dmd_info)
{
/* lint -e529 -esym(529,*) */
#ifdef CONFIG_HUAWEI_DSM
	if (dmd_no < NFC_DMD_NUMBER_MIN || dmd_no > NFC_DMD_NUMBER_MAX
		|| dmd_info == NULL || NULL == nfc_dclient) {
		hwlog_info("%s: para error: %ld\n", __func__, dmd_no); /*lint !e960*/
		return -1;
	}

	hwlog_info("%s: dmd no: %ld - %s", __func__, dmd_no, dmd_info); /*lint !e960*/
	if (!dsm_client_ocuppy(nfc_dclient)) {
		dsm_client_record(nfc_dclient, "DMD info:%s", dmd_info);
		dsm_client_notify(nfc_dclient, dmd_no);
	}
#endif
	return 0;
}
/* lint +e516 +e515 +e717 +e960 +e712 +e747 */
/* lint -e529 +esym(529,*) */
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(5, 10, 0))
static void nfc_record_dmd_probe_timerhandler(struct timer_list *data)
#else
static void nfc_record_dmd_probe_timerhandler(unsigned long data)
#endif
{
	del_timer(&nfc_dmd_probe_timer);
	if (get_nfc_dmd_probe_flag() == 0) {
		if (g_chip_type == NFCTYPE_SN110)
			nfc_record_dmd_info(NFC_DMD_I2C_READ_ERROR_NO, "PN54X-Error-in-Probe-Error:nfc_sz");
		else
			nfc_record_dmd_info(NFC_DMD_I2C_READ_ERROR_NO, "PN54X-Error-in-Probe-Error:nfc_dg");
	}
}

static ssize_t nfc_hal_dmd_info_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	return (ssize_t)(snprintf_s(buf, MAX_ATTRIBUTE_BUFFER_SIZE, MAX_ATTRIBUTE_BUFFER_SIZE - 1,
		"%d", g_nfc_hal_dmd_no));
}

static ssize_t nfc_calibration_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	char cali_info[TEL_HUAWEI_NV_NFCCAL_LEGTH] = {0};
	char cali_str[MAX_BUFFER_SIZE] = {0};
	int err;
	int ret = -1;
	int i = 0;
	int cal_len = 0;
	ret = nfc_nv_opera(NV_READ, TEL_HUAWEI_NV_NFCCAL_NUMBER, TEL_HUAWEI_NV_NFCCAL_NAME, TEL_HUAWEI_NV_NFCCAL_LEGTH, cali_info);
	if (ret < 0) {
		hwlog_err("%s: get nv error ret: %d!\n", __func__, ret);
		return -1;
	}
	cal_len = cali_info[0];
	if (cal_len >= TEL_HUAWEI_NV_NFCCAL_LEGTH) {
		cal_len = TEL_HUAWEI_NV_NFCCAL_LEGTH - 1;
		hwlog_info("%s: nfc cal info len %d!\n", __func__, cali_info[0]);
	}
	for (i = 0; i < cal_len + 1; i++) {
		err = snprintf_s(&cali_str[i * 2], sizeof(cali_info[i]) * 2 + 1, sizeof(cali_info[i]) * 2 + 1, "%02X", cali_info[i]);
		if (err < 0)
			hwlog_err("%s: snprintf_s is failed\n", __func__);
	}
	hwlog_info("%s: nfc cal info: [%s]!\n", __func__, cali_str);
	return (ssize_t)(snprintf_s(buf, MAX_BUFFER_SIZE, MAX_BUFFER_SIZE-1, "%s", cali_str));
}

static ssize_t nfc_clk_src_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	return (ssize_t)(snprintf_s(buf, MAX_ATTRIBUTE_BUFFER_SIZE, MAX_ATTRIBUTE_BUFFER_SIZE-1,
		"%d", g_nfc_clk_src));
}



static int recovery_close_nfc(struct i2c_client *client, struct  pn547_dev *pdev)
{
	int ret;
	int nfc_rece_length = 40;

	unsigned char recv_buf[40] = {0};

	const  char send_reset[] = {0x20, 0x00, 0x01, 0x00};
	const  char init_cmd[] = {0x20, 0x01, 0x00};

	unsigned char set_ven_config[] = {0x20,0x02,0x05,0x01,0xA0,0x07,0x01,0x02};
	//unsigned char get_ven_config[] = {0x40,0x02,0x02,0x00,0x00};

	/* hardware reset */
	/* power on */
	nfc_gpio_set_value(pdev->firm_gpio, 0);
	ret = pn547_bulk_enable(pdev);
	if (ret < 0) {
		hwlog_err("%s: regulator_enable failed, ret:%d\n", __func__, ret);
		goto failed;
	}
	msleep(20);

	/* power off */
	ret = pn547_bulk_disable(pdev);
	if (ret < 0) {
		hwlog_err("%s: regulator_disable failed, ret:%d\n", __func__, ret);
		goto failed;
	}
	msleep(60);

	/* power on */
	ret = pn547_bulk_enable(pdev);
	if (ret < 0) {
		hwlog_err("%s: regulator_enable failed, ret:%d\n", __func__, ret);
		goto failed;
	}
	msleep(20);

	/* write CORE_RESET_CMD */
	ret = pn547_i2c_write(pdev, send_reset, sizeof(send_reset));
	if (ret < 0) {
		hwlog_err("%s: pn547_i2c_write failed, ret:%d\n", __func__, ret);
		goto failed;
	}
	/* read response */
	ret = pn547_i2c_read(pdev, recv_buf, nfc_rece_length);
	if (ret < 0) {
		hwlog_err("%s: pn547_i2c_read failed, ret:%d\n", __func__, ret);
		goto failed;
	}

	udelay(500);
	/*write CORE_INIT_CMD*/
	ret = pn547_i2c_write(pdev, init_cmd, sizeof(init_cmd));
	if (ret < 0) {
		hwlog_err("%s: pn547_i2c_write failed, ret:%d\n", __func__, ret);
		goto failed;
	}
	/*read response*/
	ret = pn547_i2c_read(pdev, recv_buf, nfc_rece_length);
	if (ret < 0) {
		hwlog_err("%s: pn547_i2c_read failed, ret:%d\n", __func__, ret);
		goto failed;
	}

	msleep(10);

	if (g_chip_type == NFCTYPE_SN110)
		set_ven_config[7] = 0x00;
	else
		set_ven_config[7] = 0x02;

	/*write set_ven_config*/
	ret = pn547_i2c_write(pdev, set_ven_config, sizeof(set_ven_config));
	if (ret < 0) {
		hwlog_err("%s: pn547_i2c_write failed, ret:%d\n", __func__, ret);
		goto failed;
	}
	/*read response*/
	ret = pn547_i2c_read(pdev, recv_buf, nfc_rece_length);
	if (ret < 0) {
		hwlog_err("%s: pn547_i2c_read failed, ret:%d\n", __func__, ret);
		goto failed;
	}

	return 0;
failed:
	return -1;
}


static ssize_t nfc_recovery_close_nfc_store(struct device *dev, struct device_attribute *attr,
	const char *buf, size_t count)
{
	return (ssize_t)count;
}

static ssize_t nfc_recovery_close_nfc_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	int status = -1;
	struct i2c_client *i2c_client_dev = container_of(dev, struct i2c_client, dev);
	struct pn547_dev *pn547_dev = NULL;

	/* write fwUpdate_on to nvm when recovery */
	hwlog_info("set_fwupdate_on_info: close enter!\n");
	(void)set_fwupdate_on_info(0);

	pn547_dev = i2c_get_clientdata(i2c_client_dev);
	if (pn547_dev == NULL) {
		hwlog_err("%s: pn547_dev == NULL!\n", __func__);
		return status;
	}
	hwlog_info("%s: enter!\n", __func__);
	status = recovery_close_nfc(i2c_client_dev, pn547_dev);
	if (status < 0)
		hwlog_err("%s: check_sim_status error!\n", __func__);
	hwlog_info("%s: status=%d\n", __func__, status);
	return (ssize_t)(snprintf_s(buf, MAX_ATTRIBUTE_BUFFER_SIZE, MAX_ATTRIBUTE_BUFFER_SIZE - 1,
		"%d\n", status));
}

static ssize_t nxp_config_clock_switch_store(struct device *dev, struct device_attribute *attr,
	const char *buf, size_t count)
{
	unsigned int value;
	unsigned int value_set;
	if (g_nfc_nxp_config_clock_switch == 0) // 0 means clock switch not config
		return (ssize_t)count;
	if ((buf != NULL) && (buf[0] == '1')) {
		pmic_write_reg(PMU_CLK_NFC_EN_REG, 1); // 1 - clock enable
		value = pmic_read_reg(PMU_CLK_LDO_MASK_REG);
		value_set = value | 0x2; // Masking the ldo_buf switch by enable reg bit 1
		pmic_write_reg(PMU_CLK_LDO_MASK_REG, value_set);
		hwlog_info("%s: PMU clock write open ok, and config ldo mask from %d to %d\n", __func__, value, value_set);
	} else if ((buf != NULL) && (buf[0] == '0')) {
		pmic_write_reg(PMU_CLK_NFC_EN_REG, 0); // 0 - clock disable
		value = pmic_read_reg(PMU_CLK_LDO_MASK_REG);
		value_set = value & 0xfd; // not mask the ldo_buf switch by disable reg bit 1
		pmic_write_reg(PMU_CLK_LDO_MASK_REG, value_set);
		hwlog_info("%s: PMU clock write close ok, and config ldo mask from %d to %d\n", __func__, value, value_set);
	} else {
		hwlog_info("%s: param input error\n", __func__);
	}

	return (ssize_t)count;
}

static ssize_t nxp_config_clock_switch_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	return (ssize_t)(snprintf_s(buf, MAX_ATTRIBUTE_BUFFER_SIZE, MAX_ATTRIBUTE_BUFFER_SIZE - 1,
		"%s\n", g_nfc_nxp_config_clock_switch));
}
static struct ese_config_data g_ese_config_data = {0};
#define DTS_COMP_ESS_CA_NAME "hisilicon,ese_ca"
static int ese_ca_get_dt_val(struct device_node *pnode)
{
	if (of_property_read_u32(pnode, "nfc_ese_num_dts", &(g_ese_config_data.nfc_ese_num_dts))) {
		hwlog_err("get nfc_ese_num_dts from dts failed\n");
		return -ENODEV;
	}
	if (of_property_read_u32(pnode, "spi_bus", &(g_ese_config_data.spi_bus))) {
		hwlog_err("get spi_bus from dts failed\n");
		return -ENODEV;
	}
	if (of_property_read_u32(pnode, "gpio_spi_cs", &(g_ese_config_data.gpio_spi_cs))) {
		hwlog_err("get gpio_spi_cs from dts failed\n");
		return -ENODEV;
	}
	if (of_property_read_u32(pnode, "gpio_ese_irq", &(g_ese_config_data.gpio_ese_irq))) {
		hwlog_err("get gpio_ese_irq from dts failed\n");
		return -ENODEV;
	}
	if (of_property_read_u32(pnode, "gpio_ese_reset", &(g_ese_config_data.gpio_ese_reset))) {
		hwlog_err("get gpio_ese_reset from dts failed\n");
		return -ENODEV;
	}
	if (of_property_read_u32(pnode, "svdd_pwr_req_need", &(g_ese_config_data.svdd_pwr_req_need))) {
		hwlog_err("get svdd_pwr_req_need from dts failed\n");
		return -ENODEV;
	}
	if (of_property_read_u32(pnode, "gpio_svdd_pwr_req", &(g_ese_config_data.gpio_svdd_pwr_req))) {
		hwlog_err("get gpio_svdd_pwr_req from dts failed\n");
		return -ENODEV;
	}
	if (of_property_read_u32(pnode, "spi_switch_need", &(g_ese_config_data.spi_switch_need))) {
		hwlog_err("get spi_switch_need from dts failed\n");
		return -ENODEV;
	}
	if (of_property_read_u32(pnode, "gpio_spi_switch", &(g_ese_config_data.gpio_spi_switch))) {
		hwlog_err("get gpio_spi_switch from dts failed\n");
		return -ENODEV;
	}
	return 0;
}

static int ese_ca_parse_dt(void)
{
	char *ese_config_status = NULL;
	struct device_node *tmp_pnode;
	int ret;
	tmp_pnode = of_find_compatible_node(NULL, NULL, DTS_COMP_ESS_CA_NAME);
	if (tmp_pnode == NULL) {
		hwlog_err("get ese_config hisilicon,ese_ca from dts failed\n");
		return -ENODEV;
	}
	if (of_property_read_string(tmp_pnode, "status",
		(const char **)&ese_config_status) >= 0) {
		hwlog_err("ese_config_status=%s\n", ese_config_status);
		if (strncmp(ese_config_status, "ok", 2)) {
			hwlog_err("get ese_config status from dts failed\n");
			return -ENODEV;
		}
	}

	ret = ese_ca_get_dt_val(tmp_pnode);
	return ret;
}

static ssize_t nfc_ese_num_dts_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	return (ssize_t)(snprintf_s(buf, MAX_ATTRIBUTE_BUFFER_SIZE, MAX_ATTRIBUTE_BUFFER_SIZE - 1,
		"%u", g_ese_config_data.nfc_ese_num_dts));
}

static ssize_t nfc_ese_num_dts_store(struct device *dev, struct device_attribute *attr,
			const char *buf, size_t count)
{
	int ret = -1;

	hwlog_err("%s: %s count=%lu\n", __func__, buf, count);
	if (g_ese_config_data.nfc_ese_num_dts != NULL)
		ret = strncpy_s(g_ese_config_data.nfc_ese_num_dts, sizeof(g_ese_config_data.nfc_ese_num_dts),
			buf, MAX_NFC_CHIP_TYPE_SIZE - 1);
	if (ret != 0)
		hwlog_err("%s: strncpy_s fail\n", __func__);
	return (ssize_t)count;
}

static ssize_t spi_bus_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	return (ssize_t)(snprintf_s(buf, MAX_ATTRIBUTE_BUFFER_SIZE, MAX_ATTRIBUTE_BUFFER_SIZE - 1,
		"%u", g_ese_config_data.spi_bus));
}

static ssize_t spi_bus_store(struct device *dev, struct device_attribute *attr,
			const char *buf, size_t count)
{
	int ret;

	hwlog_err("%s: %s count=%lu\n", __func__, buf, count);
	ret = strncpy_s(g_ese_config_data.spi_bus, sizeof(g_ese_config_data.spi_bus),
		buf, MAX_NFC_CHIP_TYPE_SIZE - 1);
	if (ret != 0)
		hwlog_err("%s: strncpy_s fail\n", __func__);
	return (ssize_t)count;
}

static ssize_t gpio_spi_cs_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	return (ssize_t)(snprintf_s(buf, MAX_ATTRIBUTE_BUFFER_SIZE, MAX_ATTRIBUTE_BUFFER_SIZE - 1,
		"%u", g_ese_config_data.gpio_spi_cs));
}

static ssize_t gpio_spi_cs_store(struct device *dev, struct device_attribute *attr,
			const char *buf, size_t count)
{
	int ret = -1;

	hwlog_err("%s: %s count=%lu\n", __func__, buf, count);
	if (g_ese_config_data.gpio_spi_cs != NULL)
		ret = strncpy_s(g_ese_config_data.gpio_spi_cs, sizeof(g_ese_config_data.gpio_spi_cs),
			buf, MAX_NFC_CHIP_TYPE_SIZE - 1);
	if (ret != 0)
		hwlog_err("%s: strncpy_s fail\n", __func__);
	return (ssize_t)count;
}

static ssize_t gpio_ese_irq_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	return (ssize_t)(snprintf_s(buf, MAX_ATTRIBUTE_BUFFER_SIZE, MAX_ATTRIBUTE_BUFFER_SIZE - 1,
		"%u", g_ese_config_data.gpio_ese_irq));
}

static ssize_t gpio_ese_irq_store(struct device *dev, struct device_attribute *attr,
			const char *buf, size_t count)
{
	int ret;

	hwlog_err("%s: %s count=%lu\n", __func__, buf, count);
	ret = strncpy_s(g_ese_config_data.gpio_ese_irq, sizeof(g_ese_config_data.gpio_ese_irq),
		buf, MAX_NFC_CHIP_TYPE_SIZE - 1);
	if (ret != 0)
		hwlog_err("%s: strncpy_s fail\n", __func__);
	return (ssize_t)count;
}

static ssize_t gpio_ese_reset_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	return (ssize_t)(snprintf_s(buf, MAX_ATTRIBUTE_BUFFER_SIZE, MAX_ATTRIBUTE_BUFFER_SIZE - 1,
		"%u", g_ese_config_data.gpio_ese_reset));
}

static ssize_t gpio_ese_reset_store(struct device *dev, struct device_attribute *attr,
			const char *buf, size_t count)
{
	int ret;

	hwlog_err("%s: %s count=%lu\n", __func__, buf, count);
	ret = strncpy_s(g_ese_config_data.gpio_ese_reset, sizeof(g_ese_config_data.gpio_ese_reset),
		buf, MAX_NFC_CHIP_TYPE_SIZE - 1);
	if (ret != 0)
		hwlog_err("%s: strncpy_s fail\n", __func__);
	return (ssize_t)count;
}

static ssize_t svdd_pwr_req_need_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	return (ssize_t)(snprintf_s(buf, MAX_ATTRIBUTE_BUFFER_SIZE, MAX_ATTRIBUTE_BUFFER_SIZE - 1,
		"%u", g_ese_config_data.svdd_pwr_req_need));
}

static ssize_t svdd_pwr_req_need_store(struct device *dev, struct device_attribute *attr,
			const char *buf, size_t count)
{
	int ret;

	hwlog_err("%s: %s count=%lu\n", __func__, buf, count);
	ret = strncpy_s(g_ese_config_data.svdd_pwr_req_need, sizeof(g_ese_config_data.svdd_pwr_req_need),
		buf, MAX_NFC_CHIP_TYPE_SIZE - 1);
	if (ret != 0)
		hwlog_err("%s: strncpy_s fail\n", __func__);
	return (ssize_t)count;
}

static ssize_t gpio_svdd_pwr_req_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	return (ssize_t)(snprintf_s(buf, MAX_ATTRIBUTE_BUFFER_SIZE, MAX_ATTRIBUTE_BUFFER_SIZE - 1,
		"%u", g_ese_config_data.gpio_svdd_pwr_req));
}

static ssize_t gpio_svdd_pwr_req_store(struct device *dev, struct device_attribute *attr,
			const char *buf, size_t count)
{
	int ret;

	hwlog_err("%s: %s count=%lu\n", __func__, buf, count);
	ret = strncpy_s(g_ese_config_data.gpio_svdd_pwr_req, sizeof(g_ese_config_data.gpio_svdd_pwr_req),
		buf, MAX_NFC_CHIP_TYPE_SIZE - 1);
	if (ret != 0)
		hwlog_err("%s: strncpy_s fail\n", __func__);
	return (ssize_t)count;
}

static ssize_t spi_switch_need_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	return (ssize_t)(snprintf_s(buf, MAX_ATTRIBUTE_BUFFER_SIZE, MAX_ATTRIBUTE_BUFFER_SIZE - 1,
		"%u", g_ese_config_data.spi_switch_need));
}

static ssize_t spi_switch_need_store(struct device *dev, struct device_attribute *attr,
			const char *buf, size_t count)
{
	int ret;

	hwlog_err("%s: %s count=%lu\n", __func__, buf, count);
	ret = strncpy_s(g_ese_config_data.spi_switch_need, sizeof(g_ese_config_data.spi_switch_need),
		buf, MAX_NFC_CHIP_TYPE_SIZE - 1);
	if (ret != 0)
		hwlog_err("%s: strncpy_s fail\n", __func__);
	return (ssize_t)count;
}

static ssize_t gpio_spi_switch_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	return (ssize_t)(snprintf_s(buf, MAX_ATTRIBUTE_BUFFER_SIZE, MAX_ATTRIBUTE_BUFFER_SIZE - 1,
		"%u", g_ese_config_data.gpio_spi_switch));
}

static ssize_t gpio_spi_switch_store(struct device *dev, struct device_attribute *attr,
			const char *buf, size_t count)
{
	int ret;

	hwlog_err("%s: %s count=%lu\n", __func__, buf, count);
	ret = strncpy_s(g_ese_config_data.gpio_spi_switch, sizeof(g_ese_config_data.gpio_spi_switch),
		buf, MAX_NFC_CHIP_TYPE_SIZE - 1);
	if (ret != 0)
		hwlog_err("%s: strncpy_s fail\n", __func__);
	return (ssize_t)count;
}
static struct device_attribute pn547_attr[] = {

	__ATTR(nfc_fwupdate, 0664, nfc_fwupdate_show, nfc_fwupdate_store),
	__ATTR(nxp_config_name, 0664, nxp_config_name_show, nxp_config_name_store),
	__ATTR(nfc_brcm_conf_name, 0664, nfc_brcm_conf_name_show, nfc_brcm_conf_name_store),
	__ATTR(nfc_sim_switch, 0664, nfc_sim_switch_show, nfc_sim_switch_store),
	__ATTR(nfc_sim_status, 0444, nfc_sim_status_show, NULL),
	__ATTR(rd_nfc_sim_status, 0444, rd_nfc_sim_status_show, NULL),
	__ATTR(nfc_enable_status, 0664, nfc_enable_status_show, nfc_enable_status_store),
	__ATTR(nfc_card_num, 0444, nfc_card_num_show, NULL),
	__ATTR(nfc_ese_num, 0444, nfc_ese_num_show, NULL),
	__ATTR(nfc_chip_type, 0664, nfc_chip_type_show, nfc_chip_type_store),
	__ATTR(nfc_fw_version, 0444, nfc_fw_version_show, NULL),
	__ATTR(nfcservice_lock, 0664, nfcservice_lock_show, nfcservice_lock_store),
	__ATTR(nfc_svdd_sw, 0664, nfc_svdd_sw_show, nfc_svdd_sw_store),
	__ATTR(nfc_close_type, 0664, nfc_close_type_show, nfc_close_type_store),
	__ATTR(nfc_single_channel, 0444, nfc_single_channel_show, NULL),
	__ATTR(nfc_delaytime_set, 0444, nfc_delaytime_set_show, NULL),
	__ATTR(nfc_delaytime_node, 0444, nfc_delaytime_node_show, NULL),
	__ATTR(nfc_sim2_omapi, 0444, nfc_sim2_omapi_show, NULL),
	__ATTR(nfc_wired_ese_type, 0664, nfc_wired_ese_info_show, nfc_wired_ese_info_store),
	__ATTR(nfc_activated_se_info, 0664, nfc_activated_se_info_show, nfc_activated_se_info_store),
	__ATTR(nfc_hal_dmd, 0664, nfc_hal_dmd_info_show, nfc_hal_dmd_info_store),
	__ATTR(nfc_calibration, 0444, nfc_calibration_show, NULL),
	__ATTR(nfc_clk_src, 0444, nfc_clk_src_show, NULL),
	__ATTR(nfc_switch_state, 0664, nfc_switch_state_show, nfc_switch_state_store),
	__ATTR(nfc_at_result, 0664, nfc_at_result_show, nfc_at_result_store),
	/* start : recovery close nfc  2018-03-20 */
	__ATTR(nfc_recovery_close_nfc, 0664, nfc_recovery_close_nfc_show, nfc_recovery_close_nfc_store),
	/* end   : recovery close nfc  2018-03-20 */
	__ATTR(nfc_get_rssi, 0664, nfc_get_rssi_show, nfc_get_rssi_store),
	__ATTR(nfc_gt_boost, 0444, nfc_gt_boost_show, NULL),
	__ATTR(nfc_spec_rf, 0444, nfc_spec_rf_show, NULL),
	__ATTR(nfc_fwupdate_on, 0444, nfc_fwupdate_on_show, NULL),
	__ATTR(nxp_config_clock_switch, 0664, nxp_config_clock_switch_show, nxp_config_clock_switch_store),
	__ATTR(nfc_ese_num_dts, 0664, nfc_ese_num_dts_show, nfc_ese_num_dts_store),
	__ATTR(spi_bus, 0664, spi_bus_show, spi_bus_store),
	__ATTR(gpio_spi_cs, 0664, gpio_spi_cs_show, gpio_spi_cs_store),
	__ATTR(gpio_ese_irq, 0664, gpio_ese_irq_show, gpio_ese_irq_store),
	__ATTR(gpio_ese_reset, 0664, gpio_ese_reset_show, gpio_ese_reset_store),
	__ATTR(svdd_pwr_req_need, 0664, svdd_pwr_req_need_show, svdd_pwr_req_need_store),
	__ATTR(gpio_svdd_pwr_req, 0664, gpio_svdd_pwr_req_show, gpio_svdd_pwr_req_store),
	__ATTR(spi_switch_need, 0664, spi_switch_need_show, spi_switch_need_store),
	__ATTR(gpio_spi_switch, 0664, gpio_spi_switch_show, gpio_spi_switch_store),
};
/* lint -restore */
/* lint -save -e* */
static int create_sysfs_interfaces(struct device *dev)
{
	int i;
	for (i = 0; i < ARRAY_SIZE(pn547_attr); i++) {
		if (device_create_file(dev, pn547_attr + i)) {
			goto error;
		}
	}

	return 0;
error:
	for ( ; i >= 0; i--) {
		device_remove_file(dev, pn547_attr + i);
	}

	hwlog_err("%s:pn547 unable to create sysfs interface.\n", __func__);
	return -1;
}

static int remove_sysfs_interfaces(struct device *dev)
{
	int i;
	for (i = 0; i < ARRAY_SIZE(pn547_attr); i++) {
		device_remove_file(dev, pn547_attr + i);
	}

	return 0;
}
static int nfc_ven_low_beforepwd(struct notifier_block *this, unsigned long code,
				void *unused)
{
	int retval = 0;

	hwlog_info("[%s]: enter!\n", __func__);
	retval = pn547_bulk_disable(nfcdev);
	if (retval < 0) {
		hwlog_err("[%s,%d]:pn547_bulk_disable failed; ret:%d\n", __func__, __LINE__, retval);
	}
	msleep(10);
	return 0;
}

static struct notifier_block nfc_ven_low_notifier = {
	.notifier_call = nfc_ven_low_beforepwd,
};
/* lint -restore */
/* lint -save -e* */
static int check_pn547(struct i2c_client *client, struct  pn547_dev *pdev)
{
	int ret = -1;
	int count = 0;
	const char host_to_pn547[1] = {0x20};
	const char firm_dload_cmd[8] = {0x00, 0x04, 0xD0, 0x09, 0x00, 0x00, 0xB1, 0x84};

	ret = pn547_bulk_enable(pdev);
	if (ret < 0) {
		hwlog_err("%s: regulator_enable failed, ret:%d\n", __func__, ret);
		return ret;
	}

	msleep(10);
	ret = pn547_bulk_disable(pdev);
	if (ret < 0) {
		hwlog_err("%s: regulator_disable failed, ret:%d\n", __func__, ret);
		return ret;
	}

	msleep(10);
	ret = pn547_bulk_enable(pdev);
	if (ret < 0) {
		hwlog_err("%s: regulator_enable failed, ret:%d\n", __func__, ret);
		return ret;
	}
	msleep(10);

	do {
		ret = i2c_master_send(client,  host_to_pn547, sizeof(host_to_pn547));
		if (ret < 0) {
			hwlog_err("%s:pn547_i2c_write failed and ret = %d,at %d times\n", __func__, ret, count);
		} else {
			hwlog_info("%s:pn547_i2c_write success and ret = %d,at %d times\n", __func__, ret, count);
			msleep(10);
			ret = pn547_enable_nfc(pdev);
			if (ret < 0) {
				hwlog_err("%s: pn547_enable_nfc exit i2c check failed, ret:%d\n", __func__, ret);
				return ret;
			}
			return ret;
		}
		count++;
		msleep(10);
	} while (count < NFC_TRY_NUM);

	for (count = 0; count < NFC_TRY_NUM; count++) {
		nfc_gpio_set_value(pdev->firm_gpio, 1);
		ret = pn547_enable_nfc(pdev);
		if (ret < 0) {
			hwlog_err("%s: pn547_enable_nfc enter firmware failed, ret:%d\n", __func__, ret);
			return ret;
		}

		ret = i2c_master_send(client, firm_dload_cmd, sizeof(firm_dload_cmd));
		if (ret < 0) {
			hwlog_info("%s:pn547_i2c_write download cmd failed:%d, ret = %d\n", __func__, count, ret);
			nfc_gpio_set_value(pdev->firm_gpio, 0);
			continue;
		}

		nfc_gpio_set_value(pdev->firm_gpio, 0);
		ret = pn547_enable_nfc(pdev);
		if (ret < 0) {
			hwlog_err("%s: pn547_enable_nfc exit firmware failed, ret:%d\n", __func__, ret);
			return ret;
		}
		break;
	}
	return ret;
}
static int pn547_get_resource(struct  pn547_dev *pdev,	struct i2c_client *client)
{
	int ret = 0;
	char *nfc_clk_status = NULL;
	t_pmu_reg_control nfc_clk_hd_reg[] = {{0x000, 0}, {0x0C5, 5}, {0x125, 4}, {0x119, 4},
		{0x000, 0}, {0x196, 6}, {0x196, 6}, {0x0BF, 6}}; // HARDWIRE_CTRL0
	t_pmu_reg_control nfc_clk_en_reg[] = {{0x000, 0}, {0x10A, 2}, {0x110, 0}, {0x000, 0},
		{0x000, 0}, {0x03E, 0}, {0x03E, 0}, {0x042, 0}}; // CLK_NFC_EN
	t_pmu_reg_control nfc_clk_dig_reg[] = {{0x000, 0}, {0x10C, 0}, {0x116, 2}, {0x238, 0},
		{0x000, 0}, {0x2DC, 0}, {0x2DC, 0}, {0x0A8, 0}}; // CLK_NFC_CTRL1
	t_pmu_reg_control clk_driver_strength[] = {{0x000, 0}, {0x109, 4}, {0x116, 0}, {0x10D, 0},
		{0x000, 0}, {0x188, 0}, {0x188, 0}, {0x0A7, 0}}; // CLK_NFC_CTRL0

	pdev->irq_gpio = (unsigned int)of_get_named_gpio(client->dev.of_node, "pn547,irq", 0);
	hwlog_err("irq_gpio:%u\n", pdev->irq_gpio);
	if (!gpio_is_valid(pdev->irq_gpio)) {
		hwlog_err("failed to get irq!\n");
		return -ENODEV;
	}
	pdev->firm_gpio = (unsigned int)of_get_named_gpio(client->dev.of_node, "pn547,dload", 0);
	hwlog_err("firm_gpio:%u\n", pdev->firm_gpio);
	if (!gpio_is_valid(pdev->firm_gpio)) {
		hwlog_err("failed to get firm!\n");
		return -ENODEV;
	}

	ret = of_property_read_string(client->dev.of_node, "clk_status", (const char **)&nfc_clk_status);
	if (ret) {
		hwlog_err("[%s,%d]:read clk status fail\n", __func__, __LINE__);
		return -ENODEV;
	} else if (!strcmp(nfc_clk_status, "pmu")) {
		hwlog_info("[%s,%d]:clock source is pmu\n", __func__, __LINE__);
		g_nfc_clk_src = NFC_CLK_SRC_PMU;
	} else if (!strcmp(nfc_clk_status, "pmu_hi6555")) {
		hwlog_info("[%s,%d]:clock source is pmu_hi6555\n", __func__, __LINE__);
		g_nfc_clk_src = NFC_CLK_SRC_PMU_HI6555;
	} else if (!strcmp(nfc_clk_status, "pmu_hi6421v600")) {
		hwlog_info("[%s,%d]:clock source is pmu_hi6421v600\n", __func__, __LINE__);
		g_nfc_clk_src = NFC_CLK_SRC_PMU_HI6421V600;
	} else if (!strcmp(nfc_clk_status, "pmu_hi6555v200")) {
		hwlog_info("[%s,%d]:clock source is pmu_hi6555v200\n", __func__, __LINE__);
		g_nfc_clk_src = NFC_CLK_SRC_PMU_HI6555V200;
	} else if (!strcmp(nfc_clk_status, "pmu_hi6421v700")) {
		hwlog_info("[%s,%d]:clock source is pmu_hi6421v700\n", __func__, __LINE__);
		g_nfc_clk_src = NFC_CLK_SRC_PMU_HI6421V700;
	} else if (!strcmp(nfc_clk_status, "pmu_hi6421v800")) {
		hwlog_info("[%s,%d]:clock source is pmu_hi6421v800\n", __func__, __LINE__);
		g_nfc_clk_src = NFC_CLK_SRC_PMU_HI6421V800;
	} else if (!strcmp(nfc_clk_status, "pmu_hi6421v900")) {
		hwlog_info("[%s,%d]:clock source is pmu_hi6421v900\n", __func__, __LINE__);
		g_nfc_clk_src = NFC_CLK_SRC_PMU_HI6421V900;
	} else if (!strcmp(nfc_clk_status, "pmu_hi6555v300")) {
		hwlog_info("[%s,%d]:clock source is pmu_hi6555v300\n", __func__, __LINE__);
		g_nfc_clk_src = NFC_CLK_SRC_PMU_HI6555V300;
	} else if (!strcmp(nfc_clk_status, "pmu_hi6555v500")) {
		hwlog_info("[%s,%d]:clock source is pmu_hi6555v500\n", __func__, __LINE__);
		g_nfc_clk_src = NFC_CLK_SRC_PMU_HI6555V500;
	} else if (!strcmp(nfc_clk_status, "xtal")) {
		hwlog_info("[%s,%d]:clock source is XTAL\n", __func__, __LINE__);
		g_nfc_clk_src = NFC_CLK_SRC_XTAL;
	} else {
		hwlog_info("[%s,%d]:clock source is cpu by default\n", __func__, __LINE__);
		g_nfc_clk_src = NFC_CLK_SRC_CPU;
	}

	pdev->pctrl = devm_pinctrl_get(&client->dev);
	if (IS_ERR(pdev->pctrl)) {
		pdev->pctrl = NULL;
		hwlog_err("failed to get clk pin!\n");
		return -ENODEV;
	}

	/* fix udp bug */
	pdev->pins_default = pinctrl_lookup_state(pdev->pctrl, "default");
	pdev->pins_idle = pinctrl_lookup_state(pdev->pctrl, "idle");
	ret = pinctrl_select_state(pdev->pctrl, pdev->pins_default);
	if (ret < 0) {
		hwlog_err("%s: unapply new state!\n", __func__);
		return -ENODEV;
	}

	if (NFC_CLK_SRC_PMU == g_nfc_clk_src) {
		/* use pmu wifibt clk */
		hwlog_info("%s: config pmu clock in register!\n", __func__);
		/* Register 0x0c5 bit5 = 0 -- not disable wifi_clk_en gpio to control wifi_clk output */
		hisi_pmu_reg_operate_by_bit(nfc_clk_hd_reg[NFC_CLK_SRC_PMU].addr,
									nfc_clk_hd_reg[NFC_CLK_SRC_PMU].pos, CLR_BIT);
		/* Register 0x10A bit2 = 0 -- disable internal register to control wifi_clk output */
		hisi_pmu_reg_operate_by_bit(nfc_clk_en_reg[NFC_CLK_SRC_PMU].addr,
									nfc_clk_en_reg[NFC_CLK_SRC_PMU].pos, CLR_BIT);
		/* Register 0x10C bit0 = 0 -- sine wave(default); 1 --	square wave */
		hisi_pmu_reg_operate_by_bit(nfc_clk_dig_reg[NFC_CLK_SRC_PMU].addr,
									nfc_clk_dig_reg[NFC_CLK_SRC_PMU].pos, SET_BIT);
		/* Register 0x109 bit5:bit4 = drive-strength
							  00 --3pF//100K;
							  01 --10pF//100K;
							  10 --16pF//100K;
							  11 --25pF//100K */
		hisi_pmu_reg_operate_by_bit(clk_driver_strength[NFC_CLK_SRC_PMU].addr,
									clk_driver_strength[NFC_CLK_SRC_PMU].pos, SET_BIT);
		hisi_pmu_reg_operate_by_bit(clk_driver_strength[NFC_CLK_SRC_PMU].addr,
									clk_driver_strength[NFC_CLK_SRC_PMU].pos + 1, SET_BIT);
	} else if (NFC_CLK_SRC_PMU_HI6555 == g_nfc_clk_src) {
		/* use pmu wifibt clk */
		hwlog_info("%s: config pmu_hi6555 clock in register!\n", __func__);
		/* Register 0x125 bit4 = 0 -- not disable wifi_clk_en gpio to control wifi_clk output */
		hisi_pmu_reg_operate_by_bit(nfc_clk_hd_reg[NFC_CLK_SRC_PMU_HI6555].addr,
									nfc_clk_hd_reg[NFC_CLK_SRC_PMU_HI6555].pos, CLR_BIT);
		/* Register 0x110 bit0 = 0 -- disable internal register to control wifi_clk output */
		hisi_pmu_reg_operate_by_bit(nfc_clk_en_reg[NFC_CLK_SRC_PMU_HI6555].addr,
									nfc_clk_en_reg[NFC_CLK_SRC_PMU_HI6555].pos, CLR_BIT);
		/* Register 0x116 bit2 = sine wave(default):1 --  square wave:0 */
		hisi_pmu_reg_operate_by_bit(nfc_clk_dig_reg[NFC_CLK_SRC_PMU_HI6555].addr,
									nfc_clk_dig_reg[NFC_CLK_SRC_PMU_HI6555].pos, CLR_BIT);
		/* Register 0x116 bit1:bit0 = drive-strength
									00 --3pF//100K;
									01 --10pF//100K;
									10 --16pF//100K;
									11 --30pF//100K */
		hisi_pmu_reg_operate_by_bit(clk_driver_strength[NFC_CLK_SRC_PMU_HI6555].addr,
									clk_driver_strength[NFC_CLK_SRC_PMU_HI6555].pos, SET_BIT);
		hisi_pmu_reg_operate_by_bit(clk_driver_strength[NFC_CLK_SRC_PMU_HI6555].addr,
									clk_driver_strength[NFC_CLK_SRC_PMU_HI6555].pos + 1, SET_BIT);
	} else if (NFC_CLK_SRC_PMU_HI6421V600 == g_nfc_clk_src) {
		/* use pmu nfc clk */
		hwlog_info("%s: config pmu_hi6421v600 clock in register!\n", __func__);
		/* Register 0x119 bit4 = 0 -- not disable nfc_clk_en gpio to control nfc_clk output */
		hisi_pmu_reg_operate_by_bit(nfc_clk_hd_reg[NFC_CLK_SRC_PMU_HI6421V600].addr,
									nfc_clk_hd_reg[NFC_CLK_SRC_PMU_HI6421V600].pos, CLR_BIT);
		/* Register 0x238 bit0 = sine wave(default):1 --  square wave:0 */
		hisi_pmu_reg_operate_by_bit(nfc_clk_dig_reg[NFC_CLK_SRC_PMU_HI6421V600].addr,
									nfc_clk_dig_reg[NFC_CLK_SRC_PMU_HI6421V600].pos, CLR_BIT);
		/* Register 0x10D bit1:bit0 = drive-strength
									00 --3pF//100K;
									01 --10pF//100K;
									10 --16pF//100K;
									11 --30pF//100K */
		hisi_pmu_reg_operate_by_bit(clk_driver_strength[NFC_CLK_SRC_PMU_HI6421V600].addr,
									clk_driver_strength[NFC_CLK_SRC_PMU_HI6421V600].pos, SET_BIT);
		hisi_pmu_reg_operate_by_bit(clk_driver_strength[NFC_CLK_SRC_PMU_HI6421V600].addr,
									clk_driver_strength[NFC_CLK_SRC_PMU_HI6421V600].pos + 1, CLR_BIT);
	} else if (NFC_CLK_SRC_PMU_HI6421V700 == g_nfc_clk_src) {
		/* use pmu nfc clk */
		hwlog_info("%s: config pmu_hi6421v700 clock in register!\n", __func__);
		/* Register 0x196 bit6 = 0 -- not disable nfc_clk_en gpio to control nfc_clk output */
		hisi_pmu_reg_operate_by_bit(nfc_clk_hd_reg[NFC_CLK_SRC_PMU_HI6421V700].addr,
									nfc_clk_hd_reg[NFC_CLK_SRC_PMU_HI6421V700].pos, CLR_BIT);
		/* Register 0x2DC bit0 = sine wave(default):1 --  square wave:0 */
		/* Register 0x188 bit1:bit0 = drive-strength
			00 --3pF//100K;
			01 --10pF//100K;
			10 --16pF//100K;
			11 --30pF//100K */
		hisi_pmu_reg_operate_by_bit(clk_driver_strength[NFC_CLK_SRC_PMU_HI6421V700].addr,
									clk_driver_strength[NFC_CLK_SRC_PMU_HI6421V700].pos, SET_BIT);
		hisi_pmu_reg_operate_by_bit(clk_driver_strength[NFC_CLK_SRC_PMU_HI6421V700].addr,
									clk_driver_strength[NFC_CLK_SRC_PMU_HI6421V700].pos + 1, CLR_BIT);
	} else if (NFC_CLK_SRC_PMU_HI6421V800 == g_nfc_clk_src) {
		/* use pmu nfc clk */
		hwlog_info("%s: config pmu_hi6421v800 clock in register!\n", __func__);
		/* Register 0x196 bit6 = 0 -- not disable nfc_clk_en gpio to control nfc_clk output */
		hisi_pmu_reg_operate_by_bit(nfc_clk_hd_reg[NFC_CLK_SRC_PMU_HI6421V800].addr,
									nfc_clk_hd_reg[NFC_CLK_SRC_PMU_HI6421V800].pos, CLR_BIT);
		/* Register 0x2DC bit0 = sine wave(default):1 --  square wave:0 */
		/* Register 0x188 bit1:bit0 = drive-strength
			00 --3pF//100K;
			01 --10pF//100K;
			10 --16pF//100K;
			11 --30pF//100K */
		hisi_pmu_reg_operate_by_bit(clk_driver_strength[NFC_CLK_SRC_PMU_HI6421V800].addr,
									clk_driver_strength[NFC_CLK_SRC_PMU_HI6421V800].pos, SET_BIT);
		hisi_pmu_reg_operate_by_bit(clk_driver_strength[NFC_CLK_SRC_PMU_HI6421V800].addr,
									clk_driver_strength[NFC_CLK_SRC_PMU_HI6421V800].pos + 1, CLR_BIT);
	} else if (NFC_CLK_SRC_PMU_HI6421V900 == g_nfc_clk_src) {
		/* use pmu nfc clk */
		hwlog_info("%s: config pmu_hi6421v900 clock in register!\n", __func__);
		/* Register 0x196 bit6 = 0 -- not disable nfc_clk_en gpio to control nfc_clk output */
		hisi_pmu_reg_operate_by_bit(nfc_clk_hd_reg[NFC_CLK_SRC_PMU_HI6421V900].addr,
									nfc_clk_hd_reg[NFC_CLK_SRC_PMU_HI6421V900].pos, CLR_BIT);
		/* Register 0x2DC bit0 = sine wave(default):1 --  square wave:0 */
		/* Register 0x188 bit1:bit0 = drive-strength
			00 --3pF//100K;
			01 --10pF//100K;
			10 --16pF//100K;
			11 --30pF//100K */
		hisi_pmu_reg_operate_by_bit(clk_driver_strength[NFC_CLK_SRC_PMU_HI6421V900].addr,
									clk_driver_strength[NFC_CLK_SRC_PMU_HI6421V900].pos, SET_BIT);
		hisi_pmu_reg_operate_by_bit(clk_driver_strength[NFC_CLK_SRC_PMU_HI6421V900].addr,
									clk_driver_strength[NFC_CLK_SRC_PMU_HI6421V900].pos + 1, CLR_BIT);
	} else if (NFC_CLK_SRC_PMU_HI6555V200 == g_nfc_clk_src) {
		/* use pmu nfc clk */
		/* pmu nfc clk is controlled by pmu, not here */
		hwlog_info("%s: config pmu_hi6555v200 clock in register!\n", __func__);
	} else if (NFC_CLK_SRC_PMU_HI6555V300 == g_nfc_clk_src) {
		/* use pmu nfc clk */
		/* pmu nfc clk is controlled by pmu, not here */
		hwlog_info("%s: config pmu_hi6555v300 clock in register!\n", __func__);
	} else if (NFC_CLK_SRC_PMU_HI6555V500 == g_nfc_clk_src) {
		/* use pmu nfc clk */
		/* pmu nfc clk is controlled by pmu, not here */
		hwlog_info("%s: config pmu_hi6555v500 clock in register!\n", __func__);
	} else if (NFC_CLK_SRC_CPU == g_nfc_clk_src) {
		/* use default soc clk */
		pdev->nfc_clk = devm_clk_get(&client->dev, NFC_CLK_PIN);
		if (IS_ERR(pdev->nfc_clk)) {
			hwlog_err("failed to get clk out\n");
			return -ENODEV;
		} else {
			ret = clk_set_rate(pdev->nfc_clk, DEFAULT_NFC_CLK_RATE);
			if (ret < 0)
				return -EINVAL;
		}
	}

	switch (g_nfc_on_type) {
	case NFC_ON_BY_GPIO:
		pdev->ven_gpio = (unsigned int)of_get_named_gpio(client->dev.of_node, "pn547,ven", 0);
		hwlog_info("Nfc on by ven_gpio:%d\n", pdev->ven_gpio); /* lint !e515 !e516 !e717 !e960 */
		if (!gpio_is_valid(pdev->ven_gpio)) {				   /* lint !e713 */
			hwlog_err("failed to get \"huawei,nfc_ven\"\n");
			return -ENODEV;
		}
		break;

	case NFC_ON_BY_REGULATOR_BULK:
		hwlog_info("Nfc on by regulator bulk!\n");
		pdev->ven_felica.supply =  "pn547ven";
		ret = devm_regulator_bulk_get(&client->dev, 1, &(pdev->ven_felica));
		if (ret) {
			hwlog_err("failed to get ven felica!\n");
			if (pdev->nfc_clk) {
				clk_put(pdev->nfc_clk);
				pdev->nfc_clk = NULL;
			}
			return -ENODEV;
		}
		break;

	case NFC_ON_BY_HISI_PMIC:
	case NFC_ON_BY_HI6421V600_PMIC:
	case NFC_ON_BY_HI6555V110_PMIC:
	case NFC_ON_BY_HI6421V700_PMIC:
	case NFC_ON_BY_HI6421V800_PMIC:
	case NFC_ON_BY_HI6421V900_PMIC:
	case NFC_ON_BY_HI6555V300_PMIC:
	case NFC_ON_BY_HI6555V500_PMIC:
		hwlog_info("Nfc on by hisi pmic!\n");
		break;

	default:
		hwlog_info("Error nfc on type!\n");
		break;
	}

	return 0;
}

static struct of_device_id pn547_i2c_dt_ids[] = {
	{.compatible = "hisilicon,pn547_nfc" },
	{}
};
/* lint -restore */
/* lint -save -e* */
static int pn547_probe(struct i2c_client *client,
		const struct i2c_device_id *id)
{
	int ret;
	struct pn547_dev *pn547_dev = NULL;

	hwlog_info("[%s,%d]: probe start !\n", __func__, __LINE__);
	if (!of_match_device(pn547_i2c_dt_ids, &client->dev)) {
		hwlog_err("[%s,%d]: pn547 NFC match fail !\n", __func__, __LINE__);
		return -ENODEV;
	}
	if (!i2c_check_functionality(client->adapter, I2C_FUNC_I2C)) {
		hwlog_err("[%s,%d]: need I2C_FUNC_I2C\n", __func__, __LINE__);
		return -ENODEV;
	}
#ifdef CONFIG_HUAWEI_DSM
	if (!nfc_dclient)
		nfc_dclient = dsm_register_client(&dsm_nfc);
#endif
	get_ext_gpio_type();
	get_nfc_on_type();
	get_nfc_svdd_sw();
	get_wake_lock_timeout();
	get_nfc_wired_ese_type();
	ret = ese_ca_parse_dt();
	hwlog_err("[%s]:ese_ca_parse_dt ret =%d\n", __func__, ret);

	ret = create_sysfs_interfaces(&client->dev);
	if (ret < 0) {
		hwlog_err("[%s,%d]:Failed to create_sysfs_interfaces\n", __func__, __LINE__);
		return -ENODEV;
	}
	ret = sysfs_create_link(NULL, &client->dev.kobj, "nfc");
	if (ret < 0) {
		hwlog_err("[%s,%d]:Failed to sysfs_create_link\n", __func__, __LINE__);
		return -ENODEV;
	}

	pn547_dev = kzalloc(sizeof(*pn547_dev), GFP_KERNEL);
	if (pn547_dev == NULL) {
		hwlog_err("[%s,%d]:failed to allocate memory for module data\n", __func__, __LINE__);
		ret = -ENOMEM;
		goto err_exit;
	}
	/* get clk ,gpio,supply resource from dts */
	ret = pn547_get_resource(pn547_dev, client);
	if (ret < 0) {
		hwlog_err("[%s,%d]: pn547 probe get resource failed \n", __func__, __LINE__);
		goto err_pn547_get_resource;
	}

	ret = gpio_request(pn547_dev->firm_gpio, NULL);
	if (ret < 0) {
		hwlog_err("[%s,%d]: gpio_request failed, firm_gpio:%u, ret:%d.\n", __func__, __LINE__,
			pn547_dev->firm_gpio, ret);
		goto err_firm_gpio_request;
	}

	ret = gpio_direction_output(pn547_dev->firm_gpio, 0);
	if (ret < 0) {
		hwlog_err("[%s,%d]: Fail set gpio as output, firm_gpio:%d.\n", __func__, __LINE__,
			pn547_dev->firm_gpio);
		goto err_ven_gpio_request;
	}

	if (NFC_ON_BY_GPIO == g_nfc_on_type) {
		ret = gpio_request(pn547_dev->ven_gpio, NULL);
		if (ret < 0) {
			hwlog_err("%s: gpio_request failed, ret:%d.\n", __func__,
					pn547_dev->ven_gpio);
			goto err_ven_gpio_request;
		}

		ret = gpio_direction_output(pn547_dev->ven_gpio, 0);
		if (ret < 0) {
			hwlog_err("%s: Fail set gpio as output, ven_gpio:%d.\n", __func__,
					pn547_dev->ven_gpio);
			goto err_check_pn547;
		}
	}
	nfcdev = pn547_dev;
	/* notifier for supply shutdown */
	register_reboot_notifier(&nfc_ven_low_notifier);

	get_nfc_gt_boost();
	get_nfc_chip_type();

	/*using i2c to find nfc device */
	ret = check_pn547(client, pn547_dev);
	if (ret < 0) {
		hwlog_err("[%s,%d]: pn547 probe failed \n", __func__, __LINE__);
		goto err_check_pn547;
	}

	//ret = pn547_bulk_disable(pn547_dev);
	if (ret < 0) {
		hwlog_err("[%s,%d]:pn547_bulk_disable failed; ret:%d\n", __func__, __LINE__, ret);
	}

	pn547_dev->client = client;
	/* init mutex and queues */
	init_waitqueue_head(&pn547_dev->read_wq);
	mutex_init(&pn547_dev->read_mutex);
	spin_lock_init(&pn547_dev->irq_enabled_lock);

	mutex_init(&pn547_dev->irq_mutex_lock);
	/* Initialize wakelock */
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(5, 10, 0))
	wlock_read = wakeup_source_register(&client->dev, "nfc_read");
#else
	wakeup_source_init(&wlock_read, "nfc_read");
#endif
	/* register pn544 char device */
	pn547_dev->pn547_device.minor = MISC_DYNAMIC_MINOR;
	if (g_chip_type == NFCTYPE_SN110)
		pn547_dev->pn547_device.name = "nfc_dg";
	else
		pn547_dev->pn547_device.name = "pn544";
	pn547_dev->pn547_device.fops = &pn547_dev_fops;
	ret = misc_register(&pn547_dev->pn547_device);
	if (ret) {
		hwlog_err("[%s,%d]: misc_register failed\n", __func__, __LINE__);
		goto err_misc_register;
	}

	/* request irq.  the irq is set whenever the chip has data available
	  * for reading.  it is cleared when all data has been read.
	  */
	pn547_dev->irq_enabled = true;
	ret = gpio_request(pn547_dev->irq_gpio, NULL);
	if (ret < 0) {
		hwlog_err("[%s,%d]: gpio_request failed, ret:%d.\n", __func__, __LINE__,
			pn547_dev->irq_gpio);
		goto err_misc_register;
	}
	ret = gpio_direction_input(pn547_dev->irq_gpio);
	if (ret < 0) {
		hwlog_err("[%s,%d]: Fail set gpio as input, irq_gpio:%d.\n", __func__, __LINE__,
			pn547_dev->irq_gpio);
		goto err_request_irq_failed;
	}
	if (g_nfc_ext_gpio & IRQ_EXTENTED_GPIO_MASK) {
		client->irq = gpio_to_irq(pn547_dev->irq_gpio);
		ret = request_threaded_irq(client->irq, NULL,
				pn547_dev_irq_handler,
				IRQF_TRIGGER_HIGH | IRQF_NO_SUSPEND, client->name, pn547_dev);
		if (ret < 0) {
			hwlog_err("[%s,%d]:ext gpio request_irq client->irq=%d failed,ret=%d\n", __func__, __LINE__, client->irq, ret);
			goto err_request_irq_failed;
		}
		hwlog_err("[%s,%d]:request_irq client->irq=%d success,ret=%d\n", __func__, __LINE__, client->irq, ret);
		pn547_disable_irq_for_ext_gpio(pn547_dev);
	} else {
		client->irq = gpio_to_irq(pn547_dev->irq_gpio);
		ret = request_irq(client->irq, pn547_dev_irq_handler,
				IRQF_TRIGGER_HIGH | IRQF_NO_SUSPEND, client->name, pn547_dev);
		if (ret) {
			hwlog_err("[%s,%d]:soc gpio request_irq client->irq=%d failed,ret=%d\n", __func__, __LINE__, client->irq, ret);
			goto err_request_irq_failed;
		}
		pn547_disable_irq(pn547_dev);
	}
	/* sim_select = 1,UICC select */
	pn547_dev->sim_switch = CARD1_SELECT;
	pn547_dev->sim_status = CARD_UNKNOWN;

	pn547_dev->enable_status = ENABLE_START;
	/* set device data to client devices */
	i2c_set_clientdata(client, pn547_dev);

#ifdef CONFIG_HUAWEI_HW_DEV_DCT
	/* detect current device successful, set the flag as present */
	set_hw_dev_flag(DEV_I2C_NFC);
#endif

	set_nfc_nxp_config_name();
	set_nfc_brcm_config_name();
	set_nfc_chip_type_name();
	set_nfc_single_channel();
    set_nfc_spec_rf();
	get_nfc_delaytime_set();
	get_nfc_delaytime_node();
	set_nfc_card_num();
	set_nfc_ese_num();
	set_nfc_sim2_omapi();
	get_nfc_pmu_clock_switch();
	nfc_is_recovery_mode();
	/* set probe flag: nxp success */
	set_nfc_dmd_probe_flag(NFC_PN547_PROBE_SUCCESS);

	hwlog_info("[%s,%d]: probe end !\n", __func__, __LINE__);

	return 0;

err_request_irq_failed:
	gpio_free(pn547_dev->irq_gpio);
err_misc_register:
	misc_deregister(&pn547_dev->pn547_device);
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(5, 10, 0))
	wakeup_source_unregister(wlock_read);
	wlock_read = NULL;
#else
	wakeup_source_trash(&wlock_read);
#endif
	mutex_destroy(&pn547_dev->read_mutex);
err_check_pn547:
	ret = pn547_bulk_disable(pn547_dev);
	if (ret < 0) {
		hwlog_err("[%s,%d]:pn547_bulk_disable failed; ret:%d\n", __func__, __LINE__, ret);
	}
	if (NFC_ON_BY_GPIO == g_nfc_on_type) {
		gpio_free(pn547_dev->ven_gpio);
	}
err_ven_gpio_request:
	gpio_free(pn547_dev->firm_gpio);
err_firm_gpio_request:
	if (pn547_dev->nfc_clk) {
		clk_put(pn547_dev->nfc_clk);
		pn547_dev->nfc_clk = NULL;
		ret = pinctrl_select_state(pn547_dev->pctrl, pn547_dev->pins_idle);
		if (ret < 0) {
			hwlog_err("[%s,%d]:unapply new state!\n", __func__, __LINE__);
		}
	}
err_pn547_get_resource:
	if(pn547_dev->pctrl != NULL) {
		hwlog_err("[%s,%d]: err_pn547_get_resource pinctrl_put \n", __func__, __LINE__);
		devm_pinctrl_put(pn547_dev->pctrl);
	}
	kfree(pn547_dev);
err_exit:
	hwlog_err("[%s,%d]: err_pn547_get_resource sysfs_remove_link \n", __func__, __LINE__);
	sysfs_remove_link(NULL, "nfc");
	remove_sysfs_interfaces(&client->dev);
	/* set probe flag: nxp failed */
	set_nfc_dmd_probe_flag(NFC_PN547_PROBE_FAIL);
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(5, 10, 0))
	timer_setup(&nfc_dmd_probe_timer, nfc_record_dmd_probe_timerhandler, 0);
#else
	setup_timer(&nfc_dmd_probe_timer, nfc_record_dmd_probe_timerhandler, 0);
#endif
	nfc_dmd_probe_timer.expires = jiffies + NFC_DMD_PROBE_TIMER;
	add_timer(&nfc_dmd_probe_timer);
	return ret;
}

static int pn547_remove(struct i2c_client *client)
{
	struct pn547_dev *pn547_dev = NULL;
	int ret = 0;

	hwlog_info("[%s]: %s removed !\n", __func__, g_nfc_chip_type);

	unregister_reboot_notifier(&nfc_ven_low_notifier);
	pn547_dev = i2c_get_clientdata(client);
	if (pn547_dev == NULL) {
		hwlog_err("%s:	pn547_dev == NULL!\n", __func__);
		goto out;
	}
	ret = pn547_bulk_disable(pn547_dev);
	if (ret < 0) {
		hwlog_err("[%s,%d]:pn547_bulk_disable failed; ret:%d\n", __func__, __LINE__, ret);
	}
	free_irq(client->irq, pn547_dev);
	misc_deregister(&pn547_dev->pn547_device);
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(5, 10, 0))
	wakeup_source_unregister(wlock_read);
	wlock_read = NULL;
#else
	wakeup_source_trash(&wlock_read);
#endif
	mutex_destroy(&pn547_dev->read_mutex);
	gpio_free(pn547_dev->irq_gpio);
	gpio_free(pn547_dev->firm_gpio);
	remove_sysfs_interfaces(&client->dev);
	if (pn547_dev->nfc_clk) {
		clk_put(pn547_dev->nfc_clk);
		pn547_dev->nfc_clk = NULL;
		ret = pinctrl_select_state(pn547_dev->pctrl, pn547_dev->pins_idle);
		if (ret < 0) {
			hwlog_err("%s: unapply new state!\n", __func__);
		}
	}
	kfree(pn547_dev);
	if (g_is_gtboost) {
		ret = boost_5v_enable(BOOST_5V_DISABLE, BOOST_CTRL_NFC);
		if (ret)
			hwlog_err("%s: boost_5v disable failed\n", __func__);
		else
			hwlog_info("%s: boost_5v disable succeeded\n", __func__);
	}
	del_timer(&nfc_dmd_probe_timer);
out:
	return 0;
}

static const struct i2c_device_id pn547_id[] = {
	{ "pn547", 0 },
	{ }
};
/* lint -restore */
/* lint -save -e* */
MODULE_DEVICE_TABLE(of, pn547_i2c_dt_ids);
static struct i2c_driver pn547_driver = {
	.id_table = pn547_id,
	.probe = pn547_probe,
	.remove = pn547_remove,
	.driver = {
		.owner = THIS_MODULE,
		.name = "pn544",
		.of_match_table = of_match_ptr(pn547_i2c_dt_ids),
	},
};
/* lint -restore */
/*
 * module load/unload record keeping
 */
 /* lint -save -e* */
module_i2c_driver(pn547_driver);
MODULE_AUTHOR("Sylvain Fonteneau");
MODULE_DESCRIPTION("NFC pn547 driver");
MODULE_LICENSE("GPL");
/* lint -restore */
