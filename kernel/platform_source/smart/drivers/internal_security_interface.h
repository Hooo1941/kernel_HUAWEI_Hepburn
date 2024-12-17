/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2021-2021. All rights reserved.
 * Description: Internal Security Information Interface.
 * Create: 2023-04-04
 */

#ifndef __INTERNAL_SECURITY_INTERFACE_H__
#define __INTERNAL_SECURITY_INTERFACE_H__

#ifdef CONFIG_SECURITY_HEADER_FILE_REPLACE
/*
DDR TYPE: DTSI_DDR_CONFIG_ADDR_AP(For example)
you get (DDR_CONFIG_ADDR_AP(base addr)+ DDR_CONFIG_SIZE(ddr size)) by
DTSI_DDR_CONFIG_ADDR_AP(other similar)
*/
enum dtsi_ddr_prop_num {
	DTSI_DDR_CONFIG_ADDR_AP = 0,
	DTSI_DDR_LOG_BUFF_ADDR_AP,
	DTSI_DDR_SHMEM_CH_SEND_ADDR_AP,
	DTSI_DDR_SHMEM_AP_SEND_ADDR_AP,
	DTSI_DDR_LOADMONITOR_PHYMEM_BASE_AP,
	DTSI_DDR_SHMEMEXT_ADDR_AP,
	DTSI_OFFLINE_CELL_MMAP_ADDR,
	DTSI_OFFLINE_INDOOR_MMAP_ADDR,
	DTSI_RESERVED_SENSORHUB_SDC_BASE,
	DTSI_DDR_SHMEM_SDC_ACC_ADDR_AP,
	DTSI_DDR_SHMEM_SDC_MAG_ADDR_AP,
	DTSI_DDR_SHMEM_SDC_ORIENTATION_ADDR_AP,
	DTSI_DDR_SHMEM_SDC_GYRO_ADDR_AP,
	DTSI_DDR_SHMEM_SDC_LINEAR_ACC_ADDR_AP,
	DTSI_DDR_SHMEM_SDC_ROTATION_VECTOR_ADDR_AP,
	DTSI_MAX
};

/**
 * @brief  get ddr base addr from  dts.
 * @param  ddr_index : ddr number refer to enum dtsi_ddr_prop_num
 * @return unsigned int : ddr addr defined xxx_contexthub.dtsi
 */
unsigned int dts_ddr_addr_get(unsigned int ddr_index);

/**
 * @brief  get ddr size from  dts.
 * @param  ddr_index : ddr number refer to enum dtsi_ddr_prop_num
 * @return unsigned int : ddr size defined xxx_contexthub.dtsi
 */
unsigned int dts_ddr_size_get(unsigned int ddr_index);

/**
 * @brief  get contexhub boot stat from  dts.
 * @param  void :NULL
 * @return unsigned int :contexhub boot status  defined xxx_contexthub.dtsi
 */
unsigned int get_contexhub_boot_stat(void);

/**
 * @brief  Determine whether the dts value of ddr is obtained successfully.
 * @param  void :NULL
 * @return int :ddr dts status;
 */
int get_ddr_dts_status(void);

/**
 * @brief  get ddr dts info.
 * @param  void :NULL
 * @return int :ddr dts status;
 */
int get_dts_info(void);
#endif
#endif