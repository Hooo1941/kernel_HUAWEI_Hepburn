

#include "tof_channel.h"

#include <linux/err.h>
#include <linux/mtd/hisi_nve_interface.h>
#include <linux/of.h>
#include <linux/of_device.h>
#include <linux/of_gpio.h>
#include <linux/slab.h>
#include <linux/types.h>

#include <securec.h>

#include "contexthub_route.h"
#include "tof_sysfs.h"

int send_tofp_calibrate_data_to_mcu(void)
{
	int ret = 0;
	union tof_vi5300_calibrate_data calibrate_data;
	struct tof_device_info *dev_info_tof = tof_get_device_info(TAG_TOF);

	if (!dev_info_tof) {
		hwlog_err("[%s-%d]:params are null\n", __func__, __LINE__);
		return -1;
	}

	if (read_calibrate_data_from_nv(PS_CALIDATA_NV_NUM,
		sizeof(calibrate_data.data), "PSENSOR") < 0) {
		hwlog_err("[%s-%d]:read_calibrate_data_from_nv fail\n", __func__, __LINE__);
		return -1;
	}

	if (memcpy_s(&calibrate_data.data, sizeof(calibrate_data.data), user_info.nv_data,
		sizeof(union tof_vi5300_calibrate_data)) != EOK) {
		hwlog_err("[%s-%d]:memcpy_s fail\n", __func__, __LINE__);
		return -1;
	}

	if (dev_info_tof && dev_info_tof->tof_enable_flag == 1 &&
		dev_info_tof->tof_first_start_flag == 0) {
		dev_info_tof->tof_first_start_flag = 1;
		hwlog_info("[%s-%d]:tof set calibrate data\n", __func__, __LINE__);
		ret = send_calibrate_data_to_mcu(TAG_TOF, SUB_CMD_SET_OFFSET_REQ,
			&calibrate_data.data, sizeof(union tof_vi5300_calibrate_data), false);
		if (ret < 0)
			hwlog_err("[%s-%d]:send_calibrate_data_to_mcu tag:%d fail\n",
				__func__, __LINE__, TAG_TOF);
	}
	return ret;
}
