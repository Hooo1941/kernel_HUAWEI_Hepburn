

#ifndef __TOF_DETECT_H__
#define __TOF_DETECT_H__

#include "sensor_sysfs.h"

enum {
	TOF_CHIP_VI5300 = 0,
	TOF_CHIP_INVALID,
};

union tof_vi5300_calibrate_data {
	int32_t data[7];
	struct {
		int32_t xtalk_calib;
		int32_t xtalk_ratio;
		int32_t xtalk_peak;
		int32_t offset_calib;
		int32_t reftof;
		int32_t near_dist;
		int32_t far_dist;
	} cali;
};

struct tof_device_info {
	uint8_t tof_first_start_flag;
	uint32_t chip_type;
	uint32_t tof_enable_flag;
};

struct tof_device_info *tof_get_device_info(int32_t tag);
int32_t tof_sensor_detect(struct device_node *dn, struct sensor_detect_manager *sm, int index);
void read_tofp_data_from_dts(pkt_sys_dynload_req_t *dyn_req, struct device_node *dn);
void tof_detect_init(struct sensor_detect_manager *sm, uint32_t len);

#endif

