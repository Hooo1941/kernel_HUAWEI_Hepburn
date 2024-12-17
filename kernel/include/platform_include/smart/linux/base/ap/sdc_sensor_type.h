/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2022-2022. All rights reserved.
 * Description: sdc protocol header file
 * Create: 2022/07/25
 */
#ifndef __SDC_SENSOR_TYPE_H__
#define __SDC_SENSOR_TYPE_H__

enum sdc_sensor_type {
	SDC_SENSOR_TYPE_ACCELEROMETER,
	SDC_SENSOR_TYPE_MAGNETIC_FIELD,
	SDC_SENSOR_TYPE_ORIENTATION_FIELD,
	SDC_SENSOR_TYPE_GYROSCOPE,
	SDC_SENSOR_TYPE_LINEAR_ACCELERATION,
	SDC_SENSOR_TYPE_ROTATION_VECTOR,
	SDC_SENSOR_TYPE_MAX
};

#endif
