
#ifdef CONFIG_HUAWEI_DSM
#include <dsm/dsm_pub.h>
#include <linux/errno.h>
#include <linux/clk.h>
#endif
#include "nfc_dev_attr_q.h"
#include "priv_nfc_driver.h"
#include "oal_os.h"
#include "oal_plat_op.h"
#include "nfc_dev_q.h"
#include "board_nfc.h"
#include "securec.h"

#define MAX_ATTRIBUTE_BUFFER_SIZE       128
#define NFC_DMD_NUMBER_MIN              923002000
#define NFC_DMD_NUMBER_MAX              923002016

typedef struct {
	struct device *dev;
} caxx_attr_info_t;

static caxx_attr_info_t g_caxx_attr_info;
static int g_nfc_download_flag = 0;
static unsigned long g_nfc_read_timeout = 0;

/* 0 -- close nfcservice normally; 1 -- close nfcservice with enable CE */
static int g_nfc_close_type;
static int g_nfc_hal_dmd_no = 0;    /* record last hal dmd no */
static int g_nfc_activated_se_info; /* record activated se info when nfc enable process */
static char g_nfc_at_result = 0;
static char g_nfc_switch_state;

extern int g_nfc_ese_type;
extern struct dsm_client *nfc_dclient;

static int nfc_record_dmd_info(long dmd_no, const char *dmd_info)
{
#ifdef CONFIG_HUAWEI_DSM
	if (dmd_no < NFC_DMD_NUMBER_MIN || dmd_no > NFC_DMD_NUMBER_MAX
		|| dmd_info == NULL || NULL == nfc_dclient) {
		PS_PRINT_ERR("para error: %ld\n", dmd_no);
		return -1;
	}

	PS_PRINT_INFO("dmd no: %ld - %s\n", dmd_no, dmd_info);

	if (!dsm_client_ocuppy(nfc_dclient)) {
		dsm_client_record(nfc_dclient, "DMD info:%s", dmd_info);
		dsm_client_notify(nfc_dclient, dmd_no);
	}
#else
	unused(dmd_no);
	unused(dmd_info);
#endif
	return 0;
}

/* FUNCTION: nfc_fwupdate_show
 * DESCRIPTION: show nfc firmware update result.
 * Parameters
 * struct device *dev:device structure
 * struct device_attribute *attr:device attribute
 * const char *buf:user buffer
 * RETURN VALUE
 * ssize_t:  result
 */
static ssize_t nfc_fwupdate_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	unused(dev);
	unused(attr);
	PS_PRINT_INFO("nfc_download_flag = %d\n", g_nfc_download_flag);
	/* get firmware update result from variate firmware_update */
	return (ssize_t)(snprintf_s(buf, MAX_ATTRIBUTE_BUFFER_SIZE, MAX_ATTRIBUTE_BUFFER_SIZE - 1,
								"%d", g_nfc_download_flag));
}

/* FUNCTION: nfc_fwupdate_store
 * DESCRIPTION: store nfc firmware update result.
 * Parameters
 * struct device *dev:device structure
 * struct device_attribute *attr:device attribute
 * const char *buf:user buffer
 * size_t count:data count
 * RETURN VALUE
 * ssize_t: result
 */
static ssize_t nfc_fwupdate_store(struct device *dev, struct device_attribute *attr,
								  const char *buf, size_t count)
{
	unused(dev);
	unused(attr);
	int val = 0;
	if (sscanf_s(buf, "%1d", &val) == 1) {
		g_nfc_download_flag = val;
	} else {
		PS_PRINT_ERR("set g_nfc_download_flag error\n");
		g_nfc_download_flag = 0;
	}
	PS_PRINT_INFO("g_nfc_download_flag:%d\n", g_nfc_download_flag);
	return (ssize_t)count;
}

/* FUNCTION: nfc_close_type_show
 * DESCRIPTION: show nfc closetype result.
 * Parameters
 * struct device *dev:device structure
 * struct device_attribute *attr:device attribute
 * const char *buf:user buffer
 * RETURN VALUE
 * ssize_t:  result
 */
static ssize_t nfc_close_type_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	unused(dev);
	unused(attr);
	return (ssize_t)(snprintf_s(buf, MAX_ATTRIBUTE_BUFFER_SIZE, MAX_ATTRIBUTE_BUFFER_SIZE - 1,
					"%d", g_nfc_close_type));
}

/* FUNCTION: nfc_close_type_store
 * DESCRIPTION: store nfc close type result.
 * Parameters
 * struct device *dev:device structure
 * struct device_attribute *attr:device attribute
 * const char *buf:user buffer
 * size_t count:data count
 * RETURN VALUE
 * ssize_t: result
 */
static ssize_t nfc_close_type_store(struct device *dev, struct device_attribute *attr,
			 const char *buf, size_t count)
{
	unused(dev);
	unused(attr);
	int val = 0;
	if (sscanf_s(buf, "%1d", &val) == 1) {
		g_nfc_close_type = val;
	} else {
		PS_PRINT_ERR("set g_nfc_close_type error\n");
		g_nfc_close_type = 0;
	}
	PS_PRINT_INFO("g_nfc_close_type:%d\n", g_nfc_close_type);
	return (ssize_t)count;
}

/* FUNCTION: nfc_config_name_show
 * DESCRIPTION: get nfc_config_name from variate g_nfc_caxx_config_name.
 * Parameters
 * struct device *dev:device structure
 * struct device_attribute *attr:device attribute
 * const char *buf:user buffer
 * RETURN VALUE
 * ssize_t:  result
 */
static ssize_t nfc_config_name_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	struct device_node *np = dev->of_node;
	const char *out_value = NULL;
	unused(attr);

	if (of_property_read_string(np, "nfc_caxx_conf_name", &out_value) != 0) {
		PS_PRINT_ERR("get nfc_caxx_conf_name failed.\n");
		return -EFAULT;
	}

	PS_PRINT_INFO("nfc_caxx_conf_name = %s\n", out_value);
	return (ssize_t)(snprintf_s(buf, MAX_ATTRIBUTE_BUFFER_SIZE, MAX_ATTRIBUTE_BUFFER_SIZE - 1, "%s", out_value));
}


/* FUNCTION: nfc_config_name_store
 * DESCRIPTION: store nfc_config_name, infact do nothing now.
 * Parameters
 *  struct device *dev:device structure
 *  struct device_attribute *attr:device attribute
 *  const char *buf:user buffer
 *  size_t count:data count
 * RETURN VALUE
 *  ssize_t:  result
 */
static ssize_t nfc_config_name_store(struct device *dev, struct device_attribute *attr,
									const char *buf, size_t count)
{
	unused(dev);
	unused(attr);
	unused(buf);
	PS_PRINT_ERR("do nothing just keep the interface\n");
	return (ssize_t)count;
}

/* FUNCTION: nfc_brcm_conf_name_show
 * DESCRIPTION: get nfc_brcm_conf_name from variate g_nfc_brcm_config_name.
 * Parameters
 *  struct device *dev:device structure
 *  struct device_attribute *attr:device attribute
 *  const char *buf:user buffer
 * RETURN VALUE
 *  ssize_t:  result
 */
static ssize_t nfc_brcm_conf_name_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	struct device_node *np = dev->of_node;
	const char *out_value = NULL;
	unused(attr);

	if (of_property_read_string(np, "nfc_brcm_conf_name", &out_value) != 0) {
		PS_PRINT_ERR("nfc_brcm_conf_name_show failed.\n");
		return -EFAULT;
	}
	PS_PRINT_INFO("nfc_brcm_conf_name = %s\n", out_value);
	return (ssize_t)(snprintf_s(buf, MAX_ATTRIBUTE_BUFFER_SIZE, MAX_ATTRIBUTE_BUFFER_SIZE - 1, "%s", out_value));
}

/* FUNCTION: nfc_brcm_conf_name_store
 * DESCRIPTION: store nfc_brcm_conf_name, infact do nothing now.
 * Parameters
 * struct device *dev:device structure
 * struct device_attribute *attr:device attribute
 * const char *buf:user buffer
 * size_t count:data count
 * RETURN VALUE
 * ssize_t:  result
 */
static ssize_t nfc_brcm_conf_name_store(struct device *dev, struct device_attribute *attr,
										const char *buf, size_t count)
{
	unused(dev);
	unused(attr);
	unused(buf);
	PS_PRINT_ERR("do nothing just keep the interface\n");
	return (ssize_t)count;
}

/* FUNCTION: nfc_sim_switch_show
 * DESCRIPTION: get user sim card select result.
 * Parameters
 * struct device *dev:device structure
 * struct device_attribute *attr:device attribute
 * const char *buf:user buffer
 * RETURN VALUE
 * ssize_t:  result
 */
static ssize_t nfc_sim_switch_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	unused(attr);
	struct i2c_client *i2c_client_dev = container_of(dev, struct i2c_client, dev);
	struct caxx_dev *caxx_dev;
	caxx_dev = i2c_get_clientdata(i2c_client_dev);
	if (caxx_dev == NULL) {
		PS_PRINT_ERR("nfc_dg_dev is NULL.\n");
		return 0;
	}
	return (ssize_t)(snprintf_s(buf, MAX_ATTRIBUTE_BUFFER_SIZE, MAX_ATTRIBUTE_BUFFER_SIZE - 1,
					"%d\n", caxx_dev->sim_switch));
}

/* FUNCTION: nfc_sim_switch_store
 * DESCRIPTION: save user sim card select result.
 * Parameters
 *  struct device *dev:device structure
 *  struct device_attribute *attr:device attribute
 *  const char *buf:user buffer
 *  size_t count:data count
 * RETURN VALUE
 *  ssize_t:result
 */
static ssize_t nfc_sim_switch_store(struct device *dev, struct device_attribute *attr,
									const char *buf, size_t count)
{
	unused(attr);
	struct i2c_client *i2c_client_dev = container_of(dev, struct i2c_client, dev);
	struct caxx_dev *caxx_dev;
	int val = 0;

	caxx_dev = i2c_get_clientdata(i2c_client_dev);
	if (caxx_dev == NULL) {
		PS_PRINT_ERR("nfc_dg_dev is NULL.\n");
		return 0;
	}
	if (sscanf_s(buf, "%1d", &val) == 1) {
		if (val >= 1 && val <= 3) {
			caxx_dev->sim_switch = val;
		} else {
			return -EINVAL;
		}
	} else {
		return -EINVAL;
	}
	return (ssize_t)count;
}

/* FUNCTION: nfc_sim_status_show
 * DESCRIPTION: get nfc-sim card support result.
 * Parameters
 *  struct device *dev:device structure
 *  struct device_attribute *attr:device attribute
 *  const char *buf:user buffer
 * RETURN VALUE
 *		eSE		UICC	value
 *		0		0		0	(not support)
 *		0		1		1	(swp1 support)
 *		1		0		2	(swp2 support)
 *		1		1		3	(all support)
 *		<0	:error
 */
static ssize_t nfc_sim_status_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	unused(attr);
	struct i2c_client *i2c_client_dev = container_of(dev, struct i2c_client, dev);
	struct caxx_dev *caxx_dev;
	caxx_dev = i2c_get_clientdata(i2c_client_dev);
	if (caxx_dev == NULL) {
		PS_PRINT_ERR("nfc_dg_dev is NULL.\n");
		return 0;
	}
	/* Need check sim status. */
	return (ssize_t)(snprintf_s(buf, MAX_ATTRIBUTE_BUFFER_SIZE, MAX_ATTRIBUTE_BUFFER_SIZE - 1,
					"%d\n", caxx_dev->sim_status));
}

/* FUNCTION: rd_nfc_sim_status_show
 * DESCRIPTION: get nfc-sim card support result from variate, no need to send check commands again.
 * Parameters
 *  struct device *dev:device structure
 *  struct device_attribute *attr:device attribute
 *  const char *buf:user buffer
 * RETURN VALUE
 *		eSE		UICC	value
 *		0		0		0	(not support)
 *		0		1		1	(swp1 support)
 *		1		0		2	(swp2 support)
 *		1		1		3	(all support)
 *		<0	:error
 */
static ssize_t rd_nfc_sim_status_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	unused(dev);
	unused(attr);
	unused(buf);
	PS_PRINT_ERR("do nothing just keep the interface\n");
	return -EFAULT;
}

/* FUNCTION: nfc_enable_status_show
 * DESCRIPTION: show nfc_enable_status for RD test.
 * Parameters
 *  struct device *dev:device structure
 *  struct device_attribute *attr:device attribute
 *  const char *buf:user buffer
 * RETURN VALUE
 *  ssize_t:  result
 */
static ssize_t nfc_enable_status_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	unused(attr);
	struct i2c_client *i2c_client_dev = container_of(dev, struct i2c_client, dev);
	struct caxx_dev *caxx_dev;
	caxx_dev = i2c_get_clientdata(i2c_client_dev);
	if (caxx_dev == NULL) {
		PS_PRINT_ERR("nfc_dg_dev is NULL.\n");
		return 0;
	}
	return (ssize_t)(snprintf_s(buf, MAX_ATTRIBUTE_BUFFER_SIZE, MAX_ATTRIBUTE_BUFFER_SIZE - 1,
					"%d\n", caxx_dev->enable_status));
}

/* FUNCTION: nfc_enable_status_store
 * DESCRIPTION: store nfc_enable_status for RD test.
 * Parameters
 *  struct device *dev:device structure
 *  struct device_attribute *attr:device attribute
 *  const char *buf:user buffer
 *  size_t count:data count
 * RETURN VALUE
 *  ssize_t:  result
 */
static ssize_t nfc_enable_status_store(struct device *dev, struct device_attribute *attr,
	const char *buf, size_t count)
{
	unused(attr);
	struct i2c_client *i2c_client_dev = container_of(dev, struct i2c_client, dev);
	struct caxx_dev *caxx_dev;
	int val = 0;

	caxx_dev = i2c_get_clientdata(i2c_client_dev);
	if (caxx_dev == NULL) {
		PS_PRINT_ERR("nfc_dg_dev is NULL.\n");
		return 0;
	}
	if (sscanf_s(buf, "%1d", &val) == 1) {
		if (val == ENABLE_START) {
			caxx_dev->enable_status = ENABLE_START;
		} else if (val == ENABLE_END) {
			caxx_dev->enable_status = ENABLE_END;
		} else {
			return -EINVAL;
		}
	} else {
		return -EINVAL;
	}
	return (ssize_t)count;
}

/* FUNCTION: nfc_card_num_show
 * DESCRIPTION: show supported nfc_card_num, which config in device tree system.
 * Parameters
 *  struct device *dev:device structure
 *  struct device_attribute *attr:device attribute
 *  const char *buf:user buffer
 * RETURN VALUE
 *  ssize_t:  result
 */
static ssize_t nfc_card_num_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	struct device_node *np = dev->of_node;
	int temp = 0;
	unused(attr);

	if (of_property_read_u32(np, "nfc_card_num", &temp) != 0) {
		PS_PRINT_ERR("nfc_card_num_show failed.\n");
		return -EFAULT;
	}
	PS_PRINT_INFO("nfc_card_num = %d\n", temp);
	return (ssize_t)(snprintf_s(buf, MAX_ATTRIBUTE_BUFFER_SIZE, MAX_ATTRIBUTE_BUFFER_SIZE - 1, "%d", temp));
}

/* FUNCTION: nfc_chip_type_show
 * DESCRIPTION: show nfc_chip_type, which config in device tree system.
 * Parameters
 *  struct device *dev:device structure
 *  struct device_attribute *attr:device attribute
 *  const char *buf:user buffer
 * RETURN VALUE
 *  ssize_t:  result
 */
static ssize_t nfc_chip_type_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	struct device_node *np = dev->of_node;
	const char *out_value = NULL;
	unused(attr);

	if (of_property_read_string(np, "nfc_chip_type", &out_value) != 0) {
		PS_PRINT_ERR("nfc_chip_type_show failed.\n");
		return -EFAULT;
	}

	PS_PRINT_INFO("nfc_chip_type = %s\n", out_value);
	return (ssize_t)(snprintf_s(buf, MAX_ATTRIBUTE_BUFFER_SIZE, MAX_ATTRIBUTE_BUFFER_SIZE - 1, "%s", out_value));
}

static ssize_t nfc_single_channel_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	int single_channel;
	struct device_node *np = dev->of_node;
	const char *out_value = NULL;
	unused(attr);

	if (of_property_read_string(np, "nfc_single_channel", &out_value) != 0) {
		PS_PRINT_ERR("nfc_single_channel_show failed.\n");
		return -EFAULT;
	}
	single_channel = (!strcasecmp(out_value, "true")) ? 1 : 0;

	PS_PRINT_INFO("nfc_single_channel = %s\n", out_value);
	return (ssize_t)(snprintf_s(buf, MAX_ATTRIBUTE_BUFFER_SIZE, MAX_ATTRIBUTE_BUFFER_SIZE - 1,
					"%d", single_channel));
}

static ssize_t nfc_wired_ese_info_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	unused(dev);
	unused(attr);
	PS_PRINT_INFO("nfc_wired_ese_info = %d\n", g_nfc_ese_type);
	return (ssize_t)(snprintf_s(buf, MAX_ATTRIBUTE_BUFFER_SIZE, MAX_ATTRIBUTE_BUFFER_SIZE - 1,
					"%d", g_nfc_ese_type));
}

static ssize_t nfc_wired_ese_info_store(struct device *dev, struct device_attribute *attr,
	const char *buf, size_t count)
{
	int val = 0;
	unused(dev);
	unused(attr);
	if (sscanf_s(buf, "%d", &val) == 1) {
		g_nfc_ese_type = val;
	} else {
		PS_PRINT_ERR("set g_nfc_ese_type error!\n");
	}
	PS_PRINT_INFO("g_nfc_ese_type:%d\n", g_nfc_ese_type);
	return (ssize_t)count;
}

static ssize_t nfc_read_timeout_store(struct device *dev, struct device_attribute *attr,
	const char *buf, size_t count)
{
	unused(dev);
	unused(attr);
	if (count != sizeof(unsigned long)) {
		PS_PRINT_ERR("set g_nfc_read_timeout error!\n");
		return (ssize_t)count;
	}
	g_nfc_read_timeout = *(unsigned long*)buf;
	PS_PRINT_INFO("g_nfc_read_timeout:%u\n", g_nfc_read_timeout);
	return (ssize_t)count;
}

static ssize_t nfc_activated_se_info_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	unused(dev);
	unused(attr);
	unused(buf);
	PS_PRINT_ERR("do nothing just keep the interface\n");
	return -EFAULT;
}

static ssize_t nfc_activated_se_info_store(struct device *dev, struct device_attribute *attr,
										const char *buf, size_t count)
{
	unused(dev);
	unused(attr);
	int val = 0;
	if (sscanf_s(buf, "%1d", &val) == 1) {
		g_nfc_activated_se_info = val;
	} else {
		PS_PRINT_ERR("set g_nfc_activated_se_info error\n");
		g_nfc_activated_se_info = 0;
	}
	PS_PRINT_INFO("g_nfc_activated_se_info:%d\n", g_nfc_activated_se_info);
	return (ssize_t)count;
}

static ssize_t nfc_hal_dmd_info_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	unused(dev);
	unused(attr);
	PS_PRINT_INFO("nfc_hal_dmd_info = %d\n", g_nfc_hal_dmd_no);
	return (ssize_t)(snprintf_s(buf, MAX_ATTRIBUTE_BUFFER_SIZE, MAX_ATTRIBUTE_BUFFER_SIZE - 1,
					"%d", g_nfc_hal_dmd_no));
}

static ssize_t nfc_hal_dmd_info_store(struct device *dev, struct device_attribute *attr,
									const char *buf, size_t count)
{
	int val = 0;
	char dmd_info_from_hal[64] = {'\0'};
	unused(dev);
	unused(attr);
	/* The length of DMD error number is 9 */
	if (sscanf_s(buf, "%9d", &val) == 1) {
		if ((val < NFC_DMD_NUMBER_MIN) || (val > NFC_DMD_NUMBER_MAX))
			return (ssize_t)count;

		g_nfc_hal_dmd_no = val;
		/* The max length of content for current dmd description set as 63.
		   Example for DMD Buf: '923002014 CoreReset:600006A000D1A72000'.
		   A space as a separator is between dmd error no and description. */
		if (sscanf_s(buf, "%*s%63s", dmd_info_from_hal, sizeof(dmd_info_from_hal)) == 1)
			nfc_record_dmd_info(val, dmd_info_from_hal);
	} else {
		PS_PRINT_ERR("get dmd number error!\n");
	}
	return (ssize_t)count;
}

static int nfc_get_dts_config_u32(const char *node_name, const char *prop_name, u32 *pvalue)
{
	struct device_node *np = NULL;
	int ret = -1;

	for_each_node_with_property(np, node_name) {
		ret = of_property_read_u32(np, prop_name, pvalue);
		if (ret != 0) {
			PS_PRINT_ERR("can not get prop values with prop_name: %s\n", prop_name);
		} else {
			PS_PRINT_INFO("%s=%d\n", prop_name, *pvalue);
		}
	}
	return ret;
}

static ssize_t nfc_clk_src_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	struct device_node *np = dev->of_node;
	const char *out_value = NULL;
	unused(attr);

	if (of_property_read_string(np, "clk_status", &out_value) != 0) {
		PS_PRINT_ERR("nfc_clk_src_show failed.\n");
		return -EFAULT;
	}

	PS_PRINT_INFO("clk_status = %s\n", out_value);
	return (ssize_t)(snprintf_s(buf, MAX_ATTRIBUTE_BUFFER_SIZE, MAX_ATTRIBUTE_BUFFER_SIZE - 1, "%s", out_value));
}

static ssize_t nfc_switch_state_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	unused(dev);
	unused(attr);
	return (ssize_t)(snprintf_s(buf, sizeof(g_nfc_switch_state) + 1, sizeof(g_nfc_switch_state),
					"%c", g_nfc_switch_state));
}

static ssize_t nfc_switch_state_store(struct device *dev, struct device_attribute *attr,
									const char *buf, size_t count)
{
	unused(dev);
	unused(attr);
	if ((buf != NULL) && (buf[0] >= '0') && (buf[0] <= '9'))
		g_nfc_switch_state = buf[0]; /* file storage str */
	return (ssize_t)count;
}

static ssize_t nfc_at_result_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	unused(dev);
	unused(attr);
	return (ssize_t)(snprintf_s(buf, sizeof(g_nfc_at_result) + 1, sizeof(g_nfc_at_result),
					"%c", g_nfc_at_result));
}

static ssize_t nfc_at_result_store(struct device *dev, struct device_attribute *attr,
								const char *buf, size_t count)
{
	unused(dev);
	unused(attr);
	if ((buf != NULL) && (buf[0] >= '0') && (buf[0] <= '9'))
		g_nfc_at_result = buf[0]; /* file storage char */
	return (ssize_t)count;
}

static ssize_t nfc_dev_name_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	int type = 0;
	unused(dev);
	unused(attr);
	if (nfc_get_dts_config_u32("nfc_dev_name_type", "nfc_dev_name_type", &type) != 0) {
		PS_PRINT_ERR("nfc_dev_name_show failed.\n");
		return -EFAULT;
	}

	PS_PRINT_INFO("nfc_dev_name_type = %d\n", type);
	return (ssize_t)(snprintf_s(buf, MAX_ATTRIBUTE_BUFFER_SIZE, MAX_ATTRIBUTE_BUFFER_SIZE - 1, "%d", type));
}

static ssize_t nfc_download_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	unused(dev);
	unused(attr);
	PS_PRINT_INFO("nfc_download_flag = %d\n", g_nfc_download_flag);
	/* get firmware update result from variate firmware_update */
	return (ssize_t)(snprintf_s(buf, MAX_ATTRIBUTE_BUFFER_SIZE, MAX_ATTRIBUTE_BUFFER_SIZE - 1,
								"%d", g_nfc_download_flag));
}

static ssize_t nfc_poweroff_store(struct device *dev, struct device_attribute *attr,
								const char *buf, size_t count)
{
	unused(dev);
	unused(attr);
	PS_PRINT_INFO("nfc poweroff store cmd = %d\n", buf);
	/* power off */
	board_pwn_ctrl(NFC_DISABLE);
	msleep(50); // 下电后等待50ms，防止上层马上重新上电
	g_nfc_download_flag = 0;
	return (ssize_t)count;
}

static ssize_t nfc_download_store(struct device *dev, struct device_attribute *attr,
								const char *buf, size_t count)
{
	int ret;
	unused(dev);
	unused(attr);
	unused(buf);

	PS_PRINT_INFO("nfc download store\n");

	ret = nfc_chip_power_on();
	if (ret != 0) {
		g_nfc_download_flag = 0;
		return (ssize_t)count;
	}
	g_nfc_download_flag = 1;
	return (ssize_t)count;
}

/* register device node to communication with user space */
static struct device_attribute caxx_attr[] = {
	__ATTR(nfc_fwupdate, 0664, nfc_fwupdate_show, nfc_fwupdate_store),
	__ATTR(nfc_close_type, 0664, nfc_close_type_show, nfc_close_type_store),
	__ATTR(nfc_config_name, 0664, nfc_config_name_show, nfc_config_name_store),
	__ATTR(nfc_brcm_conf_name, 0664, nfc_brcm_conf_name_show, nfc_brcm_conf_name_store),
	__ATTR(nfc_sim_switch, 0664, nfc_sim_switch_show, nfc_sim_switch_store),
	__ATTR(nfc_sim_status, 0444, nfc_sim_status_show, NULL),
	__ATTR(rd_nfc_sim_status, 0444, rd_nfc_sim_status_show, NULL),
	__ATTR(nfc_enable_status, 0664, nfc_enable_status_show, nfc_enable_status_store),
	__ATTR(nfc_card_num, 0444, nfc_card_num_show, NULL),
	__ATTR(nfc_chip_type, 0444, nfc_chip_type_show, NULL),
	__ATTR(nfc_single_channel, 0444, nfc_single_channel_show, NULL),
	__ATTR(nfc_wired_ese_type, 0664, nfc_wired_ese_info_show, nfc_wired_ese_info_store),
	__ATTR(nfc_activated_se_info, 0664, nfc_activated_se_info_show, nfc_activated_se_info_store),
	__ATTR(nfc_hal_dmd, 0664, nfc_hal_dmd_info_show, nfc_hal_dmd_info_store),
	__ATTR(nfc_clk_src, 0444, nfc_clk_src_show, NULL),
	__ATTR(nfc_switch_state, 0664, nfc_switch_state_show, nfc_switch_state_store),
	__ATTR(nfc_at_result, 0664, nfc_at_result_show, nfc_at_result_store),
	__ATTR(nfc_dev_name, 0444, nfc_dev_name_show, NULL),
	__ATTR(nfc_poweroff, 0220, NULL, nfc_poweroff_store),
	__ATTR(nfc_download, 0220, NULL, nfc_download_store),
	__ATTR(nfc_get_result, 0444, nfc_download_show, NULL),
	__ATTR(nfc_set_timeout, 0220, NULL, nfc_read_timeout_store),
};

/* FUNCTION: remove_attribute_sysfs
 * DESCRIPTION: remove_attribute_sysfs.
 * Parameters
 * struct device *dev:device structure
 * RETURN VALUE
 * int:  result
 */
int remove_attribute_sysfs(struct device *dev)
{
	int i;
	for (i = 0; i < ARRAY_SIZE(caxx_attr); i++)
		device_remove_file(dev, caxx_attr + i);

	return 0;
}

/* FUNCTION: create_sysfs_interfaces
 * DESCRIPTION: create_sysfs_interfaces.
 * Parameters
 * struct device *dev:device structure
 * RETURN VALUE
 * int:  result
 */
static int create_sysfs_interfaces(struct device *dev)
{
	int i;
	for (i = 0; i < ARRAY_SIZE(caxx_attr); i++) {
		if (device_create_file(dev, caxx_attr + i)) {
			goto error;
		}
	}
	return 0;
error:
	for (; i >= 0; i--)
		device_remove_file(dev, caxx_attr + i);
	PS_PRINT_ERR("caxx unable to create attribute sysfs interface.\n");
	return -1;
}

int create_attribute_node(struct i2c_client *client)
{
	int ret;

	ret = create_sysfs_interfaces(&client->dev);
	if (ret < 0) {
		PS_PRINT_ERR("Failed to create_attribute_node\n");
		return -ENODEV;
	}
	ret = sysfs_create_link(NULL, &client->dev.kobj, "nfc");
	if (ret < 0) {
		PS_PRINT_ERR("Create_attribute_node failed to sysfs_create_link\n");
		return -ENODEV;
	}
	return 0;
}

int caxx_attr_init(struct i2c_client *client)
{
	g_caxx_attr_info.dev = &client->dev;
	return 0;
}
int caxx_attr_deinit(void)
{
	return 0;
}

unsigned long get_timeout(void)
{
	return g_nfc_read_timeout;
}