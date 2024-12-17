
#include <linux/string.h>
#include <linux/kernel.h>
#include <linux/power_supply.h>
#ifdef CONFIG_THERMAL
#include <linux/thermal.h>
#endif
#include "securec.h"
#include "utils/perf_buffered_log_sender.h"
#include "perf_ioctl.h"
#include "perf_lpm_limit_freq.h"
#ifdef CONFIG_IPA_MNTN_INFO
#include "../platform_source/cee/drivers/thermal_platform/ipa_thermal.h"
#define M3_RDR_THERMAL_FREQ_LIMIT_ADDR_COPY_SIZE 0x80
#define MAX_IPA_MNTN_INF_COPY_LEN (M3_RDR_THERMAL_FREQ_LIMIT_ADDR_COPY_SIZE + 5)
#endif

/* The data is aligned with HISI. The data content is four digits. */
#define LIMIT_FREQ_DATA_NUM 4
#define SOC_THERMAL_DATA_NUM 4
#define LPM_LIMIT_ENTRY_DATA_LENGTH (LIMIT_FREQ_DATA_NUM + SOC_THERMAL_DATA_NUM + 1)
static char *thermal_zone_types[] = {"soc_thermal", "cluster0", "cluster1", "cluster2"};
struct thermal_zone_device *thermal_zone_devices[] = {NULL, NULL, NULL, NULL};

#ifdef CONFIG_IPA_MNTN_INFO
/* decimal notation */
#define DATA_CONVERT_BASE 10
static int perf_limit_freq_data_parse(char *buf, int *freq_data,
	const int data_num)
{
	char *token = NULL;
	int i = 0;
	int ret;

	if (!buf || !freq_data) {
		pr_err("PerfD %s: parse limit freq data failed, para err.\n",
			__func__);
		return -EINVAL;
	}

	/* Numbers are separated by ',' without spaces. */
	while ((token = strsep(&buf, ",")) && (i < data_num)) {
		ret = kstrtos32(token, DATA_CONVERT_BASE, freq_data + i);
		if (ret < 0) {
			pr_err("PerfD %s: convert to sint failed, ret: %d.\n",
				__func__, ret);
			return -EINVAL;
		}
		i++;
	}

	if (buf && buf[0] != '\0') {
		pr_err("PerfD %s: failed to parse data, the data is too long.\n",
			__func__);
		return -EINVAL;
	}
	if (i < data_num) {
		pr_err("PerfD %s: failed to parse data, the data is too short, "
			"data length: %d\n", __func__, i);
		return -EINVAL;
	}

	return 0;
}

static int perf_get_limit_freq_data(struct perf_buffered_log_entry *entry,
	int *freq_data)
{
	int ret;
	char buf[MAX_IPA_MNTN_INF_COPY_LEN] = {0};

	if (!entry) {
		pr_err("PerfD %s: get limit freq data failed, para err.\n", __func__);
		return -EINVAL;
	}

	ret = read_limit_freq(buf);
	if (ret <= 0) {
		pr_err("PerfD %s: failed to get limit freq info, ret: %d\n",
			__func__, ret);
		return -EINVAL;
	}

	ret = perf_limit_freq_data_parse(buf, freq_data, LIMIT_FREQ_DATA_NUM);
	if (ret < 0) {
		pr_err("PerfD %s: data parse failed, ret: %d\n", __func__, ret);
		return -EINVAL;
	}

	return 0;
}
#endif

#ifdef CONFIG_THERMAL
static void perf_get_soc_thermal_data(int *soc_thermal_data)
{
	for (int i = 0; i < SOC_THERMAL_DATA_NUM; i++) {
		if (thermal_zone_devices[i] != NULL) {
			int soc_temp = -1;
			int ret = thermal_zone_get_temp(thermal_zone_devices[i], &soc_temp);
			if (ret == 0) {
				*(soc_thermal_data + i) = soc_temp;
			} else {
				pr_info("PerfD: get_soc_thermal failed. type:%s", thermal_zone_types[i]);
			}
		} else {
			pr_info("PerfD: type:%s is NULL", thermal_zone_types[i]);
		}
	}
}
#endif

void perf_set_thermal_zone_device(const char *type, struct thermal_zone_device *tz)
{
	for (int i = 0; i < SOC_THERMAL_DATA_NUM; i++) {
		if (strcmp(type, thermal_zone_types[i]) == 0) {
			pr_info("PerfD: thermal zone register. type:%s \n", type);
			thermal_zone_devices[i] = tz;
		}
	}
}

static void perf_get_power_supply_status(int *val)
{
	struct power_supply *psy = NULL;
	union power_supply_propval data = { 0 };

	psy = power_supply_get_by_name("Battery");
	if (psy == NULL)
		return;

	int ret = power_supply_get_property(psy, POWER_SUPPLY_PROP_STATUS, &data);
	if ((ret != 0) || (data.intval == 0)) {
		pr_info("PerfD: get power supply data failed. ret:%d \n", ret);
		return;
	}
	*val = data.intval;
}

static void fill_entry_data(struct perf_buffered_log_entry *entry, int *freq_data,
	int *soc_thermal_data, int power_supply_status)
{
	int index = 0;
	int i;
	int data[LPM_LIMIT_ENTRY_DATA_LENGTH];
	for (i = 0; i < LIMIT_FREQ_DATA_NUM; i++) {
		data[index] = freq_data[i];
		index++;
	}

	for (i = 0; i < SOC_THERMAL_DATA_NUM; i++) {
		data[index] = soc_thermal_data[i];
		index++;
	}
	data[index] = power_supply_status;

	int ret = memcpy_s(entry->data, LPM_LIMIT_ENTRY_DATA_LENGTH * sizeof(int), data, sizeof(data));
	if (ret != EOK)
		pr_err("PerfD %s: data memcpy failed, ret: %d\n", __func__, ret);
}

long perf_ioctl_lpm_limit_freq(unsigned int cmd, void __user *argp)
{
	int ret = -EINVAL;
	unsigned int entry_size;
	struct perf_buffered_log_entry *entry = NULL;
	int i;
	int freq_data[LIMIT_FREQ_DATA_NUM] = {-1, -1, -1, -1};
	int soc_thermal_data[SOC_THERMAL_DATA_NUM] = {-1, -1, -1, -1};
	int power_supply_status = -1;

	entry_size = sizeof(struct perf_buffered_log_entry) + LPM_LIMIT_ENTRY_DATA_LENGTH * sizeof(int);

	entry = perf_create_log_entry(entry_size, IOC_LPM_LIMIT_FREQ, LPM_LIMIT_ENTRY_DATA_LENGTH);
	if (entry == NULL) {
		pr_err("PerfD %s: failed to create log entry\n", __func__);
		return ret;
	}

	/*
	 * By default, the data is written as error data. When the upper layer
	 * detects incorrect data, it indicates that the function is faulty or
	 * the function macro is not enabled for the HISI.
	 */
	for (i = 0; i < LPM_LIMIT_ENTRY_DATA_LENGTH; i++) {
		((int *)(entry->data))[i] = -1;
	}

#ifdef CONFIG_IPA_MNTN_INFO
	ret = perf_get_limit_freq_data(entry, freq_data);
	if (ret < 0) {
		pr_err("PerfD %s: get freq data failed, ret: %d\n", __func__, ret);
		goto error;
	}
#endif

#ifdef CONFIG_THERMAL
	perf_get_soc_thermal_data(soc_thermal_data);
#endif

	perf_get_power_supply_status(&power_supply_status);
	fill_entry_data(entry, freq_data, soc_thermal_data, power_supply_status);

	ret = send_perf_buffered_log_entry(entry);
	if (ret < 0) {
		pr_err("PerfD %s: failed to send log entry ret %d\n", __func__, ret);
		goto error;
	}

error:
	free_perf_buffered_log_entry(entry);
	return ret;
}