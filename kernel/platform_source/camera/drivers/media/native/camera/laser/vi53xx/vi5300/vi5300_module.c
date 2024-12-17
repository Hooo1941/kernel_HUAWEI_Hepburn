/*
 * Linux kernel modules for VI5300 FlightSense TOF sensor
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 */

#include <linux/uaccess.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/slab.h>
#include <linux/i2c.h>
#include <linux/mutex.h>
#include <linux/atomic.h>
#include <linux/interrupt.h>
#include <linux/irq.h>
#include <linux/gpio.h>
#include <linux/miscdevice.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/time.h>
#include <linux/of_gpio.h>
#include <linux/kobject.h>
#include <linux/kthread.h>
#include <linux/types.h>
#include <linux/err.h>
#include <linux/sched.h>
#include <linux/delay.h>
#include <linux/of.h>
#include <linux/of_gpio.h>
#include <linux/of_device.h>
#include <linux/regulator/consumer.h>
#include <securec.h>
#include <platform_include/camera/native/laser_cfg.h>

#ifdef CONFIG_HUAWEI_HW_DEV_DCT
#include <huawei_platform/devdetect/hw_dev_dec.h>
#endif

#include "vi5300.h"
#include "vi5300_platform.h"
#include "vi5300_firmware.h"
#include "vi5300_api.h"

#define VI5300_IOCTL_PERIOD _IOW('p', 0x01, uint32_t)
#define VI5300_IOCTL_XTALK_CALIB _IOR('p', 0x02, struct VI5300_XTALK_Calib_Data)
#define VI5300_IOCTL_XTALK_CONFIG _IOW('p', 0x03, int8_t)
#define VI5300_IOCTL_OFFSET_CALIB _IOR('p', 0x04, struct VI5300_OFFSET_Calib_Data)
#define VI5300_IOCTL_OFFSET_CONFIG _IOW('p', 0x05, int16_t)
#define VI5300_IOCTL_REFTOF_CONFIG _IOW('p', 0x0c, int16_t)
#define VI5300_IOCTL_POWER_ON _IO('p', 0x06)
#define VI5300_IOCTL_CHIP_INIT _IO('p', 0x07)
#define VI5300_IOCTL_START _IO('p', 0x08)
#define VI5300_IOCTL_STOP _IO('p', 0x09)
#define VI5300_IOCTL_MZ_DATA _IOR('p', 0x0a, struct VI5300_Measurement_Data)
#define VI5300_IOCTL_POWER_OFF _IO('p', 0x0b)

#define SWITCH_ON_LOG_LEN 1000
#define VER_MSB 3
#define VER_LSB 14
#define VI5300_LOAD_CURRENT 18000
#define VI5300_DRV_NAME "vi5300"
static int xtalk_mark;
static int offset_mark;

struct vi5300_api_fn_t {
	int32_t (*Power_ON)(VI5300_DEV dev);
	int32_t (*Power_OFF)(VI5300_DEV dev);
	void (*Chip_Register_Init)(VI5300_DEV dev);
	void (*Set_Period)(VI5300_DEV dev, uint32_t period);
	int32_t (*Single_Measure)(VI5300_DEV dev);
	int32_t (*Start_Continuous_Measure)(VI5300_DEV dev);
	int32_t (*Stop_Continuous_Measure)(VI5300_DEV dev);
	int32_t (*Get_Measure_Data)(VI5300_DEV dev);
	int32_t (*Get_Interrupt_State)(VI5300_DEV dev);
	int32_t (*Chip_Init)(VI5300_DEV dev);
	int32_t (*Start_XTalk_Calibration)(VI5300_DEV dev);
	int32_t (*Start_Offset_Calibration)(VI5300_DEV dev, uint32_t param);
	int32_t (*Get_XTalk_Parameter)(VI5300_DEV dev);
	int32_t (*Config_XTalk_Parameter)(VI5300_DEV dev);
	int32_t (*Config_RefTof_Parameter)(VI5300_DEV dev);
	void (*Read_ChipID)(VI5300_DEV dev, uint8_t *chipid);
};

static struct vi5300_api_fn_t vi5300_api_func_tbl = {
	.Power_ON = VI5300_Chip_PowerON,
	.Power_OFF = VI5300_Chip_PowerOFF,
	.Chip_Register_Init = VI5300_Chip_Register_Init,
	.Set_Period = VI5300_Set_Period,
	.Single_Measure = VI5300_Single_Measure,
	.Start_Continuous_Measure = VI5300_Start_Continuous_Measure,
	.Stop_Continuous_Measure = VI5300_Stop_Continuous_Measure,
	.Get_Measure_Data = VI5300_Get_Measure_Data,
	.Get_Interrupt_State = VI5300_Get_Interrupt_State,
	.Chip_Init = VI5300_Chip_Init,
	.Start_XTalk_Calibration = VI5300_Start_XTalk_Calibration,
	.Start_Offset_Calibration = VI5300_Start_Offset_Calibration,
	.Get_XTalk_Parameter = VI5300_Get_XTalk_Parameter,
	.Config_XTalk_Parameter = VI5300_Config_XTalk_Parameter,
};
struct vi5300_api_fn_t *vi5300_func_tbl;

static void vi5300_setupAPIFunctions(void)
{
	vi5300_func_tbl->Power_ON = VI5300_Chip_PowerON;
	vi5300_func_tbl->Power_OFF = VI5300_Chip_PowerOFF;
	vi5300_func_tbl->Chip_Register_Init = VI5300_Chip_Register_Init;
	vi5300_func_tbl->Set_Period = VI5300_Set_Period;
	vi5300_func_tbl->Single_Measure = VI5300_Single_Measure;
	vi5300_func_tbl->Start_Continuous_Measure = VI5300_Start_Continuous_Measure;
	vi5300_func_tbl->Stop_Continuous_Measure = VI5300_Stop_Continuous_Measure;
	vi5300_func_tbl->Get_Measure_Data = VI5300_Get_Measure_Data;
	vi5300_func_tbl->Get_Interrupt_State = VI5300_Get_Interrupt_State;
	vi5300_func_tbl->Chip_Init = VI5300_Chip_Init;
	vi5300_func_tbl->Start_XTalk_Calibration = VI5300_Start_XTalk_Calibration;
	vi5300_func_tbl->Start_Offset_Calibration = VI5300_Start_Offset_Calibration;
	vi5300_func_tbl->Get_XTalk_Parameter = VI5300_Get_XTalk_Parameter;
	vi5300_func_tbl->Config_XTalk_Parameter = VI5300_Config_XTalk_Parameter;
}

static void vi5300_enable_irq(struct  vi5300_data *data)
{
	if(!data)
		return;

	if(data->intr_state == VI5300_INTR_DISABLED)
	{
		data->intr_state = VI5300_INTR_ENABLED;
		enable_irq(data->irq);
	}
}

static void vi5300_disable_irq(struct  vi5300_data *data)
{
	if(!data)
		return;

	if(data->intr_state == VI5300_INTR_ENABLED)
	{
		data->intr_state = VI5300_INTR_DISABLED;
		disable_irq(data->irq);
	}
}

static ssize_t vi5300_chip_enable_show(struct device *dev,
				struct device_attribute *attr, char *buf)
{
	struct vi5300_data *data = dev_get_drvdata(dev);

	if(NULL != data)
		return scnprintf(buf, PAGE_SIZE, "%u\n", data->chip_enable);

	return -EINVAL;
}

static ssize_t vi5300_chip_enable_store(struct device *dev,
				struct device_attribute *attr, const char *buf, size_t count)
{
	struct vi5300_data *data = dev_get_drvdata(dev);
	VI5300_Error Status = VI5300_ERROR_NONE;
	unsigned int val = 0;

	if(NULL != data)
	{
		mutex_lock(&data->work_mutex);
		if(sscanf(buf, "%u\n", &val) != 1)
		{
			mutex_unlock(&data->work_mutex);
			return -1;
		}
		if(val !=0 && val !=1)
		{
			vi5300_errmsg("enable store unvalid value=%u\n", val);
			mutex_unlock(&data->work_mutex);
			return count;
		}
		if(val == 1)
		{
			if(data->chip_enable == 0)
			{
				data->chip_enable = 1;
				vi5300_enable_irq(data);
				Status = vi5300_func_tbl->Power_ON(data);
				getnstimeofday(&data->start_ts);
			} else {
				vi5300_errmsg("already enabled!!\n");
			}
		} else {
			if(data->chip_enable == 1)
			{
				data->chip_enable = 0;
				vi5300_disable_irq(data);
				data->fwdl_status = 0;
				Status = vi5300_func_tbl->Power_OFF(data);
			}
			else {
				vi5300_errmsg("already disabled!!\n");
			}
		}
		mutex_unlock(&data->work_mutex);
		return Status ? -1 : count;
	}

	return -EPERM;
}

static DEVICE_ATTR(chip_enable, 0664, vi5300_chip_enable_show, vi5300_chip_enable_store);

/* for debug */
static ssize_t vi5300_enable_debug_show(struct device *dev,
				struct device_attribute *attr, char *buf)
{
	struct vi5300_data *data = dev_get_drvdata(dev);

	if(NULL != data)
		return snprintf(buf, PAGE_SIZE, "%u\n", data->enable_debug);

	return -EINVAL;
}

static ssize_t vi5300_enable_debug_store(struct device *dev,
					struct device_attribute *attr, const
					char *buf, size_t count)
{
	struct vi5300_data *data = dev_get_drvdata(dev);
	unsigned int on = 0;

	if(NULL != data)
	{
		mutex_lock(&data->work_mutex);
		if(sscanf(buf, "%u\n", &on) != 1)
		{
			mutex_unlock(&data->work_mutex);
			return -1;
		}
		if ((on != 0) &&  (on != 1)) {
			vi5300_errmsg("set debug=%d\n", on);
			mutex_unlock(&data->work_mutex);
			return count;
		}
		mutex_unlock(&data->work_mutex);
		data->enable_debug = on;
		return count;
	}

	return -EINVAL;
}

static DEVICE_ATTR(enable_debug, 0664, vi5300_enable_debug_show, vi5300_enable_debug_store);

static ssize_t vi5300_chip_init_store(struct device *dev,
				struct device_attribute *attr, const char *buf, size_t count)
{
	struct vi5300_data *data = dev_get_drvdata(dev);
	VI5300_Error Status = VI5300_ERROR_NONE;
	unsigned int val = 0;

	if(NULL != data)
	{
		mutex_lock(&data->work_mutex);
		if(sscanf(buf, "%u\n", &val) != 1)
		{
			mutex_unlock(&data->work_mutex);
			return -1;
		}

		if(val)
		{
			Status = vi5300_func_tbl->Chip_Init(data);
			data->fwdl_status = 1;
		} else {
			mutex_unlock(&data->work_mutex);
			return -EINVAL;
		}
		mutex_unlock(&data->work_mutex);
		return Status ? -1 : count;
	}

	return -EPERM;
}

static DEVICE_ATTR(chip_init, 0220, NULL, vi5300_chip_init_store);

static ssize_t vi5300_period_show(struct device *dev,
				struct device_attribute *attr, char *buf)
{
	struct vi5300_data *data = dev_get_drvdata(dev);

	if(data != NULL)
		return scnprintf(buf, PAGE_SIZE, "%u\n", data->period);

	return -EINVAL;
}

static ssize_t vi5300_period_store(struct device *dev,
				struct device_attribute *attr, const char *buf, size_t count)
{
	uint32_t val = 0;
	struct vi5300_data *data = dev_get_drvdata(dev);

	if(data != NULL)
	{
		mutex_lock(&data->work_mutex);
		if(sscanf(buf, "%u\n", &val) != 1)
		{
			mutex_unlock(&data->work_mutex);
			return -EINVAL;
		}

		data->period = val;
		vi5300_func_tbl->Set_Period(data, data->period);
		mutex_unlock(&data->work_mutex);
		return count;
	}

	return -EPERM;
}

static DEVICE_ATTR(period, 0664, vi5300_period_show, vi5300_period_store);

static ssize_t vi5300_capture_store(struct device *dev,
				struct device_attribute *attr, const char *buf, size_t count)
{
	struct vi5300_data *data = dev_get_drvdata(dev);
	VI5300_Error Status = VI5300_ERROR_NONE;
	unsigned int val = 0;

	if(NULL != data)
	{
		mutex_lock(&data->work_mutex);
		if(sscanf(buf, "%u\n", &val) != 1)
		{
			mutex_unlock(&data->work_mutex);
			return -1;
		}
		if(val !=0 && val !=1)
		{
			vi5300_errmsg("capture store unvalid value=%u\n", val);
			mutex_unlock(&data->work_mutex);
			return count;
		}
		if(val == 1)
		{
			Status = vi5300_func_tbl->Start_Continuous_Measure(data);
		} else {
			Status = vi5300_func_tbl->Stop_Continuous_Measure(data);
		}
		mutex_unlock(&data->work_mutex);
		return Status ? -1 : count;
	}

	return -EPERM;
}

static DEVICE_ATTR(capture, 0220, NULL, vi5300_capture_store);

static ssize_t vi5300_xtalk_calib_show(struct device *dev,
				struct device_attribute *attr, char *buf)
{
	struct vi5300_data *data = dev_get_drvdata(dev);

	if(data != NULL)
		return scnprintf(buf, PAGE_SIZE, "%d.%u\n", data->XtalkData.xtalk_cal, data->XtalkData.xtalk_peak);

	return -EPERM;
}

static ssize_t vi5300_xtalk_calib_store(struct device *dev,
				struct device_attribute *attr, const char *buf, size_t count)
{
	struct vi5300_data *data = dev_get_drvdata(dev);
	VI5300_Error Status = VI5300_ERROR_NONE;
	unsigned int val = 0;

	if(NULL != data)
	{
		mutex_lock(&data->work_mutex);
		if(sscanf(buf, "%u\n", &val) != 1)
		{
			mutex_unlock(&data->work_mutex);
			return -1;
		}
		if(val !=1)
		{
			vi5300_errmsg("xtalk calibration store unvalid value=%u\n", val);
			mutex_unlock(&data->work_mutex);
			return count;
		}
		xtalk_mark = 1;
		Status = vi5300_func_tbl->Start_XTalk_Calibration(data);
		mdelay(600);
		xtalk_mark = 0;
		mutex_unlock(&data->work_mutex);
		return Status ? -1 : count;
	}

	return -EPERM;
}

static DEVICE_ATTR(xtalk_calib, 0664, vi5300_xtalk_calib_show, vi5300_xtalk_calib_store);

static ssize_t vi5300_offset_calib_show(struct device *dev,
				struct device_attribute *attr, char *buf)
{
	struct vi5300_data *data = dev_get_drvdata(dev);

	if(data != NULL)
		return scnprintf(buf, PAGE_SIZE, "%d\n", data->OffsetData.offset_cal);

	return -EPERM;
}

static ssize_t vi5300_offset_calib_store(struct device *dev,
				struct device_attribute *attr, const char *buf, size_t count)
{
	struct vi5300_data *data = dev_get_drvdata(dev);
	VI5300_Error Status = VI5300_ERROR_NONE;
	unsigned int val = 0;
	uint32_t param = 0;

	if(NULL != data)
	{
		mutex_lock(&data->work_mutex);
		if(sscanf(buf, "%u\n", &val) != 1)
		{
			mutex_unlock(&data->work_mutex);
			return -1;
		}
		if(val !=1)
		{
			vi5300_errmsg("offset calibration store unvalid value=%u\n", val);
			mutex_unlock(&data->work_mutex);
			return count;
		}
		offset_mark = 1;
		Status = vi5300_func_tbl->Start_Offset_Calibration(data, param);
		offset_mark = 0;
		mutex_unlock(&data->work_mutex);
		return Status ? -1 : count;
	}

	return -EPERM;
}

static DEVICE_ATTR(offset_calib, 0664, vi5300_offset_calib_show, vi5300_offset_calib_store);

static ssize_t vi5300_xtalk_config_store(struct device *dev,
				struct device_attribute *attr, const char *buf, size_t count)
{
	struct vi5300_data *data = dev_get_drvdata(dev);
	VI5300_Error Status = VI5300_ERROR_NONE;
	int val = 0;

	if(NULL != data)
	{
		mutex_lock(&data->work_mutex);
		if(sscanf(buf, "%x\n", &val) != 1)
		{
			mutex_unlock(&data->work_mutex);
			return -1;
		}
		data->xtalk_config = (int8_t)val;
		Status = vi5300_func_tbl->Config_XTalk_Parameter(data);
		mutex_unlock(&data->work_mutex);
		return Status ? -1 : count;
	}

	return -EPERM;
}

static DEVICE_ATTR(xtalk_config, 0220, NULL, vi5300_xtalk_config_store);

static ssize_t vi5300_offset_config_store(struct device *dev,
				struct device_attribute *attr, const char *buf, size_t count)
{
	struct vi5300_data *data = dev_get_drvdata(dev);
	int val = 0;

	if(NULL != data)
	{
		mutex_lock(&data->work_mutex);
		if(sscanf(buf, "%x\n", &val) != 1)
		{
			mutex_unlock(&data->work_mutex);
			return -1;
		}
		data->xtalk_config = (int16_t)val;
		mutex_unlock(&data->work_mutex);
		return count;
	}

	return -EPERM;
}

static DEVICE_ATTR(offset_config, 0220, NULL, vi5300_offset_config_store);

static ssize_t vi5300_xtalk_data_read(struct file *filp,
	struct kobject *kobj, struct bin_attribute *attr,
	char *buf, loff_t off, size_t count)
{
	struct device *dev = container_of(kobj, struct device, kobj);
	struct vi5300_data *data = dev_get_drvdata(dev);
	void *src = (void *) &(data->XtalkData);
	int rc = 0;

	mutex_lock(&data->work_mutex);
	if (!data->chip_enable) {
		vi5300_errmsg("can't set calib data while disable sensor\n");
		mutex_unlock(&data->work_mutex);
		return -EBUSY;
	}

	if (count > sizeof(struct VI5300_XTALK_Calib_Data))
		count = sizeof(struct VI5300_XTALK_Calib_Data);

	memcpy(buf, src, count);
	data->xtalk_config = data->XtalkData.xtalk_cal;
	rc = vi5300_func_tbl->Config_XTalk_Parameter(data);
	if (rc) {
		vi5300_errmsg("config xtalk calibration data fail %d", rc);
		mutex_unlock(&data->work_mutex);
		return rc;
	}
	mutex_unlock(&data->work_mutex);

	return count;
}

static ssize_t vi5300_xtalk_data_write(struct file *filp,
	struct kobject *kobj, struct bin_attribute *attr,
	char *buf, loff_t off, size_t count)
{
	struct device *dev = container_of(kobj, struct device, kobj);
	struct vi5300_data *data = dev_get_drvdata(dev);
	struct VI5300_XTALK_Calib_Data *xtalk_data =
				(struct VI5300_XTALK_Calib_Data *)buf;
	int rc = 0;

	mutex_lock(&data->work_mutex);

	if (!data->chip_enable) {
		rc = -EBUSY;
		vi5300_errmsg("can't set calib data while disable sensor\n");
		goto error;
	}

	if (count != sizeof(struct VI5300_XTALK_Calib_Data))
		goto invalid;

	if(data->enable_debug)
		vi5300_errmsg("xtalk config: %d\n", xtalk_data->xtalk_cal);

	data->xtalk_config = xtalk_data->xtalk_cal;
	rc = vi5300_func_tbl->Config_XTalk_Parameter(data);
	if (rc) {
		vi5300_errmsg("config xtalk calibration data fail %d", rc);
		goto error;
	}
	mutex_unlock(&data->work_mutex);

	return count;

invalid:
	vi5300_errmsg("invalid syntax");
	rc = -EINVAL;
	goto error;

error:
	mutex_unlock(&data->work_mutex);

	return rc;
}

static ssize_t vi5300_offset_data_read(struct file *filp,
	struct kobject *kobj, struct bin_attribute *attr,
	char *buf, loff_t off, size_t count)
{
	struct device *dev = container_of(kobj, struct device, kobj);
	struct vi5300_data *data = dev_get_drvdata(dev);
	void *src = (void *) &(data->OffsetData);

	mutex_lock(&data->work_mutex);
	if (!data->chip_enable) {
		vi5300_errmsg("can't set calib data while disable sensor\n");
		mutex_unlock(&data->work_mutex);
		return -EBUSY;
	}

	if (count > sizeof(struct VI5300_OFFSET_Calib_Data))
		count = sizeof(struct VI5300_OFFSET_Calib_Data);

	memcpy(buf, src, count);
	data->offset_config = data->OffsetData.offset_cal;
	mutex_unlock(&data->work_mutex);

	return count;
}

static ssize_t vi5300_offset_data_write(struct file *filp,
	struct kobject *kobj, struct bin_attribute *attr,
	char *buf, loff_t off, size_t count)
{
	struct device *dev = container_of(kobj, struct device, kobj);
	struct vi5300_data *data = dev_get_drvdata(dev);
	struct VI5300_OFFSET_Calib_Data *offset_data =
				(struct VI5300_OFFSET_Calib_Data *)buf;
	int rc = 0;

	mutex_lock(&data->work_mutex);
	if (!data->chip_enable) {
		rc = -EBUSY;
		vi5300_errmsg("can't set calib data while disable sensor\n");
		goto error;
	}

	if (count != sizeof(struct VI5300_OFFSET_Calib_Data))
		goto invalid;

	if(data->enable_debug)
		vi5300_errmsg("offset config: %d\n", offset_data->offset_cal);

	data->offset_config = offset_data->offset_cal;
	mutex_unlock(&data->work_mutex);
	return count;

invalid:
	vi5300_errmsg("invalid syntax");
	rc = -EINVAL;
	goto error;

error:
	mutex_unlock(&data->work_mutex);
	return rc;
}

static struct attribute *vi5300_attributes[] = {
	&dev_attr_chip_enable.attr,
	&dev_attr_enable_debug.attr,
	&dev_attr_chip_init.attr,
	&dev_attr_period.attr,
	&dev_attr_capture.attr,
	&dev_attr_xtalk_calib.attr,
	&dev_attr_offset_calib.attr,
	&dev_attr_xtalk_config.attr,
	&dev_attr_offset_config.attr,
	NULL,
};

static const struct attribute_group vi5300_attr_group = {
	.name = NULL,
	.attrs = vi5300_attributes,
};

static struct bin_attribute vi5300_xtalk_data_attr = {
	.attr = {
		.name = "xtalk_calib_data",
		.mode = 0664/*S_IWUGO | S_IRUGO*/,
	},
	.size = sizeof(struct VI5300_XTALK_Calib_Data),
	.read = vi5300_xtalk_data_read,
	.write = vi5300_xtalk_data_write,
};

static struct bin_attribute vi5300_offset_data_attr = {
	.attr = {
		.name = "offset_calib_data",
		.mode = 0664/*S_IWUGO | S_IRUGO*/,
	},
	.size = sizeof(struct VI5300_OFFSET_Calib_Data),
	.read = vi5300_offset_data_read,
	.write = vi5300_offset_data_write,
};

extern void laser_notify_data_event(hw_laser_ctrl_t *ctrl, struct laser_event_t *laser_event);

static void vi5300_notify_data_event(struct vi5300_data *data)
{
    struct laser_event_t laser_event = { 0 };

    if (!data || !(data->ctrl)) {
        vi5300_errmsg("notify data event failed\n");
        return;
    }

    laser_event.kind = FIRST_VALID_DATA;
    laser_notify_data_event(data->ctrl, &laser_event);
}

static irqreturn_t vi5300_irq_handler(int vec, void *info)
{
	struct vi5300_data *data = (struct vi5300_data *)info;
	VI5300_Error Status = VI5300_ERROR_NONE;

	vi5300_infomsg("enter vi5300_irq_handler:\n");

	if(!data || !data->fwdl_status)
		return IRQ_HANDLED;

	if (data->irq == vec) {
		if (xtalk_mark) {
			Status = vi5300_func_tbl->Get_XTalk_Parameter(data);
			if (Status != VI5300_ERROR_NONE)
				vi5300_errmsg("%d vi5300_irq_handler: Get_XTalk_Parameter failed\n", __LINE__);
		}
		if(!xtalk_mark && !offset_mark)
		{
			Status = vi5300_func_tbl->Get_Measure_Data(data);
			if(Status != VI5300_ERROR_NONE)
			{
				vi5300_errmsg("%d : Status = %d\n" , __LINE__, Status);
				return IRQ_HANDLED;
			}
			if (data->is_first_valid_data) {
				vi5300_notify_data_event(data);
				data->is_first_valid_data = false;
			}
		}
	}
	return IRQ_HANDLED;
}

static int vi5300_setup_regulator(struct device_node *np, struct vi5300_data *data)
{
	uint32_t rc = 0;
	uint32_t rgltr_min_volt[1];
	uint32_t rgltr_max_volt[1];
	uint32_t rgltr_load_current[1];
	struct regulator *vreg = NULL;

	vreg = devm_regulator_get(data->dev, "lvdd");
	if (IS_ERR(vreg)) {
		vi5300_errmsg("get regulator fail");
		goto regulator_err;
	}
	rc = of_property_read_u32_array(np, "regulator-min-microvolt", rgltr_min_volt, 1);
	if (rc) {
		vi5300_errmsg("regulator_read_voltage fail");
		goto regulator_err;
	}
	rc = of_property_read_u32_array(np, "regulator-max-microvolt", rgltr_max_volt, 1);
	if (rc) {
		vi5300_errmsg("regulator_read_voltage fail");
		goto regulator_err;
	}
	rc = regulator_set_voltage(vreg, rgltr_min_volt[0], rgltr_max_volt[0]);
	if (rc) {
		vi5300_errmsg("regulator_set_voltage fail");
		goto regulator_err;
	}
	vi5300_infomsg("Minimum volatage value = %u, Maximum volatage value = %u", rgltr_min_volt[0], rgltr_max_volt[0]);

	rc = of_property_read_u32_array(np, "rgltr-load-current", rgltr_load_current, 1);
	if (rc) {
		rgltr_load_current[0] = VI5300_LOAD_CURRENT;
	}
	vi5300_infomsg("Load curent value = %u", rgltr_load_current[0]);
	rc = regulator_set_load(vreg, rgltr_load_current[0]);
	if (rc) {
		vi5300_errmsg("regulator_set_load fail");
		goto regulator_err;
	}
	rc = regulator_enable(vreg);
	if (rc) {
		vi5300_errmsg("regulator_enable fail");
		goto regulator_err;
	}
	data->lvdd = vreg;
	return 0;

regulator_err:
	vi5300_errmsg("regulator_err");
	return 1;
}

static int vi5300_release_regulator(struct vi5300_data *data)
{
	if (data->lvdd == NULL) {
		vi5300_errmsg("release regulator fail");
		return 1;
	}
	regulator_disable(data->lvdd);
	return 0;
}

static int vi5300_parse_dt(struct device_node *np, struct vi5300_data *data)
{
	enum of_gpio_flags flags;
	uint32_t inte_counts = 0;
	uint8_t frame_counts = 0;
	data->lvdd = NULL;

	if (!data || !np)
		return -EINVAL;

	data->irq_gpio = of_get_named_gpio_flags(np, "vi5300,irq-gpio", 0, &flags);
	if (data->irq_gpio < 0) {
		vi5300_errmsg("get irq gpio: %d error\n", data->irq_gpio);
		return -ENODEV;
	}
	vi5300_infomsg("INT GPIO %d\n", data->irq_gpio);

	data->xshut_gpio = of_get_named_gpio_flags(np, "vi5300,xshut-gpio", 0, &flags);
	if (data->xshut_gpio < 0) {
		vi5300_errmsg("get xshut gpio: %d error\n", data->xshut_gpio);
		return -ENODEV;
	}
	vi5300_infomsg("XSHUT GPIO %d\n", data->xshut_gpio);

	if (!of_property_read_u32(np, "vi5300,integral-counts", &inte_counts))
		data->integral_counts = inte_counts;
	else
		data->integral_counts = DEFAULT_INTEGRAL_COUNTS;

	if (vi5300_setup_regulator(np, data))
		vi5300_errmsg("VI5300 Setup Regulator Failed\n");
	return 0;
}

static int vi5300_open(struct inode *inode, struct file *file)
{
	vi5300_errmsg("open!\n");
	return 0;
}

	/* match id */
static int vi5300_status(struct vi5300_data* data, hwlaser_status_t *laser_status)
{
	int rc = -EINVAL;
	if (!data || !laser_status)
		return rc;

	laser_status->status = VI5300_ERROR_NONE;

	return VI5300_ERROR_NONE;
}

static int vi5300_GetCalibrationData(struct vi5300_data *data, hwlaser_calibration_data_t *cal_data)
{
	int rc = 0;
	mutex_lock(&data->work_mutex);

	rc = memcpy_s(&(cal_data->u.dataL2.VI5300_XTALK_Calib_Data), sizeof(struct VI5300_XTALK_Calib_Data), &(data->XtalkData), sizeof(struct VI5300_XTALK_Calib_Data));
	if (rc) {
		mutex_unlock(&data->work_mutex);
		return -EFAULT;
	}

	rc = memcpy_s(&(cal_data->u.dataL2.VI5300_OFFSET_Calib_Data), sizeof(struct VI5300_OFFSET_Calib_Data), &(data->OffsetData), sizeof(struct VI5300_OFFSET_Calib_Data));
	if (rc) {
		mutex_unlock(&data->work_mutex);
		return -EFAULT;
	}

	vi5300_infomsg("kernel xtalk_cal(%d), kernel xtalk_peak(%u)",
		data->XtalkData.xtalk_cal, data->XtalkData.xtalk_peak);
	vi5300_infomsg("hal xtalk_cal(%d), hal xtalk_peak(%u), hal xtalk_tof(%d)",
		cal_data->u.dataL2.VI5300_XTALK_Calib_Data.xtalk_cal, cal_data->u.dataL2.VI5300_XTALK_Calib_Data.xtalk_peak,
		cal_data->u.dataL2.VI5300_XTALK_Calib_Data.xtalk_tof);
	vi5300_infomsg("kernel offset_cal(%d)", data->OffsetData.offset_cal);
	vi5300_infomsg("hal offset_cal(%d)", cal_data->u.dataL2.VI5300_OFFSET_Calib_Data.offset_cal);

	mutex_unlock(&data->work_mutex);
	return rc;
}

static int vi5300_SetCalibrationData(struct vi5300_data *data, hwlaser_calibration_data_t *cal_data)
{
	int rc = 0;
	mutex_lock(&data->work_mutex);

	rc = memcpy_s(&(data->xtalk_config), sizeof(int8_t), &(cal_data->u.dataL2.VI5300_XTALK_Calib_Data.xtalk_cal), sizeof(int8_t));
	vi5300_infomsg("hal xtalk_config(%d)", cal_data->u.dataL2.VI5300_XTALK_Calib_Data.xtalk_cal);
	vi5300_infomsg("kernel xtalk_config(%d)", data->xtalk_config);
	if (rc) {
		mutex_unlock(&data->work_mutex);
		return -EFAULT;
	}
	rc = vi5300_func_tbl->Config_XTalk_Parameter(data);
	if (rc != VI5300_ERROR_NONE)
		vi5300_infomsg("kernel xtalk_config failed(%d)", rc);

	rc = memcpy_s(&(data->offset_config), sizeof(int16_t), &(cal_data->u.dataL2.VI5300_OFFSET_Calib_Data.offset_cal), sizeof(int16_t));
	vi5300_infomsg("hal offset_config(%d)", cal_data->u.dataL2.VI5300_OFFSET_Calib_Data.offset_cal);
	vi5300_infomsg("kernel offset_config(%d)", data->offset_config);
	if (rc) {
		mutex_unlock(&data->work_mutex);
		return -EFAULT;
	}

	mutex_unlock(&data->work_mutex);
	return rc;
}

static int vi5300_laser_get_set_cal_data(struct vi5300_data* data,
                                            hwlaser_calibration_data_t *cal_data)
{
	int rc = 0;

	if (cal_data->is_read) {
		vi5300_dbgmsg("vi5300 hal get calibration data\n");
		rc = vi5300_GetCalibrationData(data, cal_data);
		if (rc) {
			vi5300_errmsg("%d, hal get xtalk, offset fail\n", __LINE__);
			return -EFAULT;
		}
	} else {
		vi5300_dbgmsg("vi5300 hal set calibration data to kernel\n");
		rc = vi5300_SetCalibrationData(data, cal_data);
		if (rc) {
			vi5300_errmsg("%d, kernel get xtalk_config, offset_config fail\n", __LINE__);
			return -EFAULT;
		}
	}

	return rc;
}

static int vi5300_perform_cal(struct vi5300_data* data, hwlaser_ioctl_perform_calibration_t* cal)
{
	int rc = 0;

	mutex_lock(&data->work_mutex);
	switch (cal->calibration_type) {
	case HWLASER_CALIBRATION_CROSSTALK:
		xtalk_mark = 1;
		rc = vi5300_func_tbl->Start_XTalk_Calibration(data);
		if(rc != VI5300_ERROR_NONE) {
			vi5300_errmsg("vi5300 perform XTALK calibration fail");
		} else {
			msleep(1000); // delay 1000 ms for SoC cal data ready
		}
		xtalk_mark = 0;
		break;
	case HWLASER_CALIBRATION_OFFSET:
		offset_mark = 1;
		rc = vi5300_func_tbl->Start_Offset_Calibration(data, cal->param1);
		if(rc != VI5300_ERROR_NONE) {
			vi5300_errmsg("vi5300 perform OFFSET calibration fail");
		}
		offset_mark = 0;
		break;
	default:
		vi5300_errmsg("vi5300_perform_cal %d, fail %d",
			cal->calibration_type, rc);
	}
	mutex_unlock(&data->work_mutex);

	return rc;
}

static int vi5300_set_params(struct vi5300_data *data, hwlaser_parameter_t *pParameter)
{
	int rc = 0;
	if (!pParameter) {
		rc = -EFAULT;
		return rc;
	}

	mutex_lock(&data->work_mutex);

	switch(pParameter->name) {
	case XTALKRATE_PAR:
		rc = memcpy_s(&(data->xtalk_config), sizeof(int8_t), &(pParameter->xtalk_config), sizeof(int8_t));
		vi5300_infomsg("kernel xtalk_config(%d)", pParameter->xtalk_config);
		vi5300_infomsg("hal xtalk_config(%d)", data->xtalk_config);
		if (rc) {
			vi5300_errmsg("%d, GET XTALK_CONFIG PARAM fail\n", __LINE__);
			mutex_unlock(&data->work_mutex);
			return -EFAULT;
		}
		rc = vi5300_func_tbl->Config_XTalk_Parameter(data);
		if (rc != VI5300_ERROR_NONE)
		break;
	case OFFSET_PAR:
		rc = memcpy_s(&(data->offset_config), sizeof(int16_t), &(pParameter->offset_config), sizeof(int16_t));
		vi5300_infomsg("kernel offset_config(%d)", pParameter->offset_config);
		vi5300_infomsg("hal offset_config(%d)", data->offset_config);
		if (rc) {
			vi5300_errmsg("%d, GET OFFSET_CONFIG PARAM fail\n", __LINE__);
			mutex_unlock(&data->work_mutex);
			return -EFAULT;
		}
		break;
	default:
		vi5300_errmsg("unkown or unsupported cmd %d", pParameter->name);
		rc = -EINVAL;
	}

	mutex_unlock(&data->work_mutex);
	return rc;
}

long vi5300_ioctl(void *hw_data, unsigned int cmd, void *p)
{
	long rc = 0;
	struct vi5300_data *data = (struct vi5300_data*)hw_data;
	hwlaser_calibration_data_t *cal_data = NULL;
	hwlaser_ioctl_perform_calibration_t* cal_perform = NULL;
	hwlaser_status_t* status = NULL;
	hwlaser_info_t *pinfo = NULL;
	hwlaser_RangingData_t *pRangingData = NULL;
	hwlaser_parameter_t *pParameter = NULL;
	hwlaser_ioctl_freq_param_t * pFreqpara = NULL;

	if (!data)
		return -EFAULT;

	switch (cmd) {
	case HWLASER_IOCTL_POWERON:
		vi5300_infomsg("HWLASER_IOCTL_POWERON\n");
		mutex_lock(&data->work_mutex);
		rc = vi5300_func_tbl->Power_ON(data);
		mutex_unlock(&data->work_mutex);
		break;
	case HWLASER_IOCTL_INIT:
		vi5300_infomsg("HWLASER_IOCTL_INIT\n");
		mutex_lock(&data->work_mutex);
		rc = vi5300_func_tbl->Chip_Init(data);
		if (rc == VI5300_ERROR_NONE)
			data->fwdl_status = 1;
		mutex_unlock(&data->work_mutex);
		break;
	case HWLASER_IOCTL_GET_INFO:
		vi5300_infomsg("HWLASER_IOCTL_GET_INFO\n");
		mutex_lock(&data->work_mutex);
		pinfo = (hwlaser_info_t *)p;
		if (strncpy_s(pinfo->name, HWLASER_NAME_SIZE - 1 ,"vi5300", HWLASER_NAME_SIZE - 1))
			vi5300_errmsg("strncpy_s failed %d", __LINE__);
		if (strncpy_s(pinfo->product_name, HWLASER_NAME_SIZE - 1 ,"LASER", HWLASER_NAME_SIZE - 1))
			vi5300_errmsg("strncpy_s failed %d", __LINE__);
		pinfo->version = HWLASER_VI_I0_VERSION;
		pinfo->ap_pos = HWLASER_POS_AP;
		mutex_unlock(&data->work_mutex);
		break;
	case HWLASER_IOCTL_START:
		vi5300_infomsg("HWLASER_IOCTL_START\n");
		mutex_lock(&data->work_mutex);
		vi5300_enable_irq(data);
		data->is_first_valid_data = true;
		rc = vi5300_func_tbl->Start_Continuous_Measure(data);
		mutex_unlock(&data->work_mutex);
		break;
	case HWLASER_IOCTL_MZ_DATA:
		vi5300_infomsg("HWLASER_IOCTL_GET_DATA\n");
		mutex_lock(&data->work_mutex);
		pRangingData = (hwlaser_RangingData_t *)p;
		if (memcpy_s(&(pRangingData->v_data), sizeof(struct VI5300_Measurement_Data), &(data->Rangedata), sizeof(struct VI5300_Measurement_Data)) != EOK) {
			vi5300_errmsg("memcpy failed %d", __LINE__);
			mutex_unlock(&data->work_mutex);
			return -EFAULT;
		}
		mutex_unlock(&data->work_mutex);
		vi5300_infomsg("milimeter(%d),status(%u)",
			pRangingData->v_data.milimeter,
			pRangingData->v_data.status);
		vi5300_infomsg("milimeter(%d), status(%u)",
			data->Rangedata.milimeter,
			data->Rangedata.status);
		if (pRangingData->v_data.milimeter < 0)
			laser_dsm_report_dmd(HWLASER_VI5300, DSM_LASER_MEASURE_DISTANCE_ERROR_NO);
		break;
	case HWLASER_IOCTL_STOP:
		vi5300_infomsg("HWLASER_IOCTL_STOP\n");
		vi5300_disable_irq(data);
		mutex_lock(&data->work_mutex);
		rc = vi5300_func_tbl->Stop_Continuous_Measure(data);
		mutex_unlock(&data->work_mutex);
		break;
	case HWLASER_IOCTL_POWEROFF:
		vi5300_infomsg("HWLASER_IOCTL_POWEROFF\n");
		data->fwdl_status = 0;
		mutex_lock(&data->work_mutex);
		rc = vi5300_func_tbl->Power_OFF(data);
		mutex_unlock(&data->work_mutex);
		break;
	case HWLASER_IOCTL_CALIBRATION_DATA:
		vi5300_infomsg("HWLASER_IOCTL_CALIBRATION_DATA\n");
		cal_data = (hwlaser_calibration_data_t *)p;
		rc = vi5300_laser_get_set_cal_data(data, cal_data);
		break;
	case HWLASER_IOCTL_PERFORM_CALIBRATION:
		vi5300_infomsg("HWLASER_IOCTL_PERFORM_CALIBRATION\n");
		cal_perform = (hwlaser_ioctl_perform_calibration_t*) p;
		rc = vi5300_perform_cal(data, cal_perform);
		break;
	case HWLASER_IOCTL_PARAMETER:
		vi5300_infomsg("HWLASER_IOCTL_PARAMETER\n");
		pParameter = (hwlaser_parameter_t *)p;
		rc = vi5300_set_params(data, pParameter);
		break;
	case HWLASER_IOCTL_ROI:
		vi5300_infomsg("HWLASER_IOCTL_ROI\n");
		break;
	case HWLASER_IOCTL_CONFIG:
		vi5300_infomsg("HWLASER_IOCTL_CONFIG\n");
		break;
	case HWLASER_IOCTL_MATCHID:
		vi5300_infomsg("HWLASER_IOCTL_MATCHID");
		mutex_lock(&data->work_mutex);
		status = (hwlaser_status_t*) p;
		rc = vi5300_status(data, p);
		mutex_unlock(&data->work_mutex);
		break;
	case HWLASER_IOCTL_SET_FREQ:
		vi5300_infomsg("HWLASER_IOCTL_SET_FREQ");
		mutex_lock(&data->work_mutex);
		pFreqpara = (hwlaser_ioctl_freq_param_t *)p;
		vi5300_infomsg("freq set to: %u", pFreqpara->freq);
		rc = vi5300_func_tbl->Stop_Continuous_Measure(data);
		vi5300_func_tbl->Set_Period(data, pFreqpara->freq);
		rc += vi5300_func_tbl->Start_Continuous_Measure(data);
		mutex_unlock(&data->work_mutex);
		break;
	default:
		vi5300_infomsg("cmd not support: %u", cmd);
		return -EFAULT;
	}
	return rc;
}
EXPORT_SYMBOL(vi5300_ioctl);

static int vi5300_flush(struct file *file, fl_owner_t id)
{
	return 0;
}

static int vi5300_setup(struct vi5300_data *data)
{
	int rc = 0;
	int irq = 0;
	uint8_t buf = 0;
	uint8_t chipid[3] = {0};
	uint32_t ChipVersion = 0;

	if (!data)
		return -EINVAL;

	if (!gpio_is_valid(data->irq_gpio) || !gpio_is_valid(data->xshut_gpio))
		return -ENODEV;

	gpio_request(data->xshut_gpio, "vi5300 xshut gpio");
	gpio_request(data->irq_gpio, "vi5300 irq gpio");
	gpio_direction_input(data->irq_gpio);
	irq = gpio_to_irq(data->irq_gpio);
	if(irq < 0) {
		vi5300_errmsg("fail to map GPIO: %d to INT: %d\n", data->irq_gpio, irq);
		rc = -EINVAL;
		goto exit_free_gpio;
	} else {
		vi5300_dbgmsg("request irq: %d\n", irq);
		rc = request_threaded_irq(irq, NULL, vi5300_irq_handler,
			IRQF_TRIGGER_FALLING | IRQF_ONESHOT, "vi5300_interrupt", (void *)data);
		if (rc) {
			vi5300_errmsg("%s(%d), Could not allocate VI5300_INT ! result:%d\n", __FUNCTION__, __LINE__, rc);
			goto exit_free_gpio;
		}
	}
	data->irq = irq;
	data->intr_state = VI5300_INTR_DISABLED;
	disable_irq_nosync(data->irq);
	data->fwdl_status = 0;
	vi5300_func_tbl = &vi5300_api_func_tbl;
	vi5300_setupAPIFunctions();
	vi5300_func_tbl->Power_ON(data);
	vi5300_read_multibytes(data, VI5300_REG_CHIPID_BASE, chipid, 3);
	ChipVersion = (chipid[1] << 16) + (chipid[0] << 8) + chipid[2];
	if(ChipVersion == 0x00)
	{
		vi5300_func_tbl->Chip_Register_Init(data);
	}
	vi5300_read_byte(data, VI5300_REG_DEV_ADDR, &buf);
	vi5300_func_tbl->Power_OFF(data);

	if (buf != VI5300_CHIP_ADDR) {
		vi5300_errmsg("VI5300 I2C Transfer Failed\n");
		rc = -EFAULT;
		goto exit_free_irq;
	}
	vi5300_infomsg("VI5300 I2C Transfer Successfully\n");

	data->period = 30;
	data->xtalk_config = 0;
	data->offset_config = 0;
	data->enable_debug = 0;
	data->Rangedata.status = 255;
	data->is_first_valid_data = true;

#ifdef CONFIG_HUAWEI_HW_DEV_DCT
	set_hw_dev_flag(DEV_I2C_LASER);
#endif
	return 0;

exit_free_irq:
	free_irq(irq, data);
exit_free_gpio:
	gpio_free(data->xshut_gpio);
	gpio_free(data->irq_gpio);
	return rc;
}

int vi5300_probe(struct i2c_client *client, const struct i2c_device_id *id)
{
	struct vi5300_data *vi5300_data = NULL;
	struct device *dev = &client->dev;
	struct device_node *node = NULL;
	hw_laser_ctrl_t *ctrl = NULL;
	int ret  = 0;

	vi5300_infomsg("%s Enter\n", __FUNCTION__);
	vi5300_data = kzalloc(sizeof(struct vi5300_data), GFP_KERNEL);
	if (!vi5300_data) {
		vi5300_errmsg("devm_kzalloc error\n");
		return -ENOMEM;
	}
	if (!dev->of_node) {
		kfree(vi5300_data);
		return -EINVAL;
	}
	/* setup device data */
	vi5300_data->dev_name = dev_name(&client->dev);
	vi5300_data->client = client;
	vi5300_data->dev = dev;
	ctrl = (hw_laser_ctrl_t *)id->driver_data;
	vi5300_data->ctrl = ctrl;
	ctrl->data = (void *)vi5300_data;
	node = dev->of_node;
	i2c_set_clientdata(client, vi5300_data);
	mutex_init(&vi5300_data->update_lock);
	mutex_init(&vi5300_data->work_mutex);
	ret = vi5300_parse_dt(node, vi5300_data);
	if (ret) {
		vi5300_errmsg("VI5300 Parse DT Failed\n");
		goto exit_error;
	}
	ret = vi5300_setup(vi5300_data);
	if (ret) {
		vi5300_errmsg("VI5300 Setup Failed\n");
		goto exit_error;
	}
	vi5300_infomsg("End!\n");
	return 0;

exit_error:
	mutex_destroy(&vi5300_data->work_mutex);
	vi5300_release_regulator(vi5300_data);
	i2c_set_clientdata(client, NULL);
	kfree(vi5300_data);
	return ret;
}
EXPORT_SYMBOL(vi5300_probe);

int vi5300_remove(struct i2c_client *client)
{
	vi5300_dbgmsg("vi5300:%s\n", __func__);
	struct vi5300_data *data = i2c_get_clientdata(client);

	if (!IS_ERR(data->miscdev.this_device) &&
			data->miscdev.this_device != NULL) {
		vi5300_dbgmsg("to unregister misc dev\n");
		misc_deregister(&data->miscdev);
	}
	if (data->xshut_gpio) {
		gpio_direction_output(data->xshut_gpio, 0);
		gpio_free(data->xshut_gpio);
	}
	if (data->irq_gpio) {
		free_irq(data->irq, data);
		gpio_free(data->irq_gpio);
	}
	misc_deregister(&data->miscdev);
	i2c_set_clientdata(client, NULL);
	mutex_destroy(&data->work_mutex);
	kfree(data);
	return 0;
}
EXPORT_SYMBOL(vi5300_remove);
