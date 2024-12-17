

#ifndef __TOF_SYSFS_H__
#define __TOF_SYSFS_H__

#include "tof_channel.h"
#include "tof_detect.h"

#define ROOT_UID 0
#define SYSTEM_GID 1000
#define CLI_TIME_STR_LEN    20
#define CLI_CONTENT_LEN_MAX 256
#define TOF_DATA_DIR      "/data/vendor/log/sensor"
#define TOF_DATA_FILE     "/data/vendor/log/sensor/dataCollection.log"
#define TOF_CALI_XTALK_TEST   "testName:TOF_CALI_XTALK_TEST*#peak:%d*#xtalk:%d*#ratio:%d*#result:%s*#cycle:%d*#errorCode:%s*#time:%s*#\n"
#define TOF_CALI_OFFSET_TEST  "testName:TOF_CALI_OFFSET_TEST*#reftof:%d*#offset:%d*#result:%s*#cycle:%d*#errorCode:%s*#time:%s*#\n"
#define TOF_CALI_NEAR_TEST    "testName:TOF_CALI_NEAR_TEST*#near_dist:%d*#result:%s*#cycle:%d*#errorCode:%s*#time:%s*#\n"
#define TOF_CALI_FAR_TEST     "testName:TOF_CALI_FAR_TEST*#far_dist:%d*#result:%s*#cycle:%d*#errorCode:%s*#time:%s*#\n"

enum {
	TOF_CALIBRATE_CMD_START,
	TOF_CALIBRATE_CMD_XTALK = 1,
	TOF_CALIBRATE_CMD_OFFSET,
	TOF_CALIBRATE_CMD_NEAR,
	TOF_CALIBRATE_CMD_FAR,
	TOF_CALIBRATE_CMD_STOP,
};

ssize_t attr_tof_calibrate_show(struct device *dev,
	struct device_attribute *attr, char *buf);
ssize_t attr_tof_calibrate_store(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t count);
#endif
