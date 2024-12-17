/*
* Copyright (c) 2016, STMicroelectronics - All Rights Reserved
*
* License terms: BSD 3-clause "New" or "Revised" License.
*
* Redistribution and use in source and binary forms, with or without
* modification, are permitted provided that the following conditions are met:
*
* 1. Redistributions of source code must retain the above copyright notice, this
* list of conditions and the following disclaimer.
*
* 2. Redistributions in binary form must reproduce the above copyright notice,
* this list of conditions and the following disclaimer in the documentation
* and/or other materials provided with the distribution.
*
* 3. Neither the name of the copyright holder nor the names of its contributors
* may be used to endorse or promote products derived from this software
* without specific prior written permission.
*
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
* AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
* DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
* FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
* DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
* SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
* CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
* OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
* OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/
/**
 * @file stmvl53l3.h header for vl53l3 sensor driver
 */
#ifndef STMVL53L3_H
#define STMVL53L3_H

#include <linux/types.h>
#include <linux/mutex.h>
#include <linux/workqueue.h>
#include <linux/miscdevice.h>
#include <linux/wait.h>
#include <platform_include/camera/native/laser_cfg.h>
#include "vl53l3_api.h"
#include "hw_laser_dmd.h"

#define HWLASER_ADAPTER

/**
 * IPP adapt
 */
#ifdef DEBUG
#	define IPP_PRINT(...) printk(__VA_ARGS__)
#else
#	define IPP_PRINT(...) (void)0
#endif

#include "stmvl53l3_ipp.h"
#include "stmvl53l3_if.h"

/**
 * Configure the Netlink-id use
 */
#ifndef STMVL531_CFG_NETLINK_USER
#define STMVL531_CFG_NETLINK_USER 38
#endif

#define STMVL53L3_MAX_CCI_XFER_SZ 256
#define STMVL53L3_DRV_NAME "stmvl53l3"
#define ST_PROXIMITY_MAX_RANGE 4000

/**
 * laser to proximity sensor use
 */
#define EXT_PROX1_DIS_NEAR_THR		20
#define EXT_PROX1_DIS_NEAR_FAR		40
#define EXT_PROX1_DEFAULT_STATUS	5

/* default VDD average power consumption, units: ua */
#define DEVICE_LOAD_CURRENT             18000
#define PROXIMITY_MIN_DELAY             30000 /* in microseconds */
#define PROXIMITY_FIFO_RESERVED_COUNT   0
#define PROXIMITY_FIFO_MAX_COUNT        0

#define SENSORS_DEBUG_MASK_ONE                  1
#define SENSORS_ACCELERATION_HANDLE             0
#define SENSORS_MAGNETIC_FIELD_HANDLE           1
#define SENSORS_ORIENTATION_HANDLE              2
#define SENSORS_LIGHT_HANDLE                    3
#define SENSORS_PROXIMITY_HANDLE                4
#define SENSORS_GYROSCOPE_HANDLE                5
#define SENSORS_PRESSURE_HANDLE                 6
#define SENSORS_HALL_HANDLE                     7
#define SENSORS_PROX_HANDLE			8

#define SENSOR_TYPE_ACCELEROMETER		1
#define SENSOR_TYPE_GEOMAGNETIC_FIELD		2
#define SENSOR_TYPE_MAGNETIC_FIELD  SENSOR_TYPE_GEOMAGNETIC_FIELD
#define SENSOR_TYPE_ORIENTATION			3
#define SENSOR_TYPE_GYROSCOPE			4
#define SENSOR_TYPE_LIGHT			5
#define SENSOR_TYPE_PRESSURE			6
#define SENSOR_TYPE_TEMPERATURE			7
#define SENSOR_TYPE_PROXIMITY			8
#define SENSOR_TYPE_GRAVITY			9
#define SENSOR_TYPE_LINEAR_ACCELERATION		10
#define SENSOR_TYPE_ROTATION_VECTOR		11
#define SENSOR_TYPE_RELATIVE_HUMIDITY		12
#define SENSOR_TYPE_AMBIENT_TEMPERATURE		13
#define SENSOR_TYPE_MAGNETIC_FIELD_UNCALIBRATED	14
#define SENSOR_TYPE_GAME_ROTATION_VECTOR	15
#define SENSOR_TYPE_GYROSCOPE_UNCALIBRATED	16
#define SENSOR_TYPE_SIGNIFICANT_MOTION		17
#define SENSOR_TYPE_STEP_DETECTOR		18
#define SENSOR_TYPE_STEP_COUNTER		19
#define SENSOR_TYPE_GEOMAGNETIC_ROTATION_VECTOR	20
#define SENSOR_TYPE_HALL						0x10002
#define SENSOR_TYPE_ABOV_CAPSENSE (SENSOR_TYPE_DEVICE_PRIVATE_BASE + 24)

/**
 * configure usage of regulator device from device tree info
 * to enable/disable sensor power
 * see module-i2c or module-cci file
 */
/* define CFG_STMVL53L3_HAVE_REGULATOR */

#define DRIVER_VERSION		"14.0.8"

/** @ingroup vl53l3_config
 * @{
 */
/**
 * Configure max number of device the driver can support
 */
#define STMVL53L3_CFG_MAX_DEV	2
/** @} */ /* ingroup vl53l3_config */

/** @ingroup vl53l3_mod_dbg
 * @{
 */
#if 0
#define DEBUG	1
#endif
#if 0
#define FORCE_CONSOLE_DEBUG
#endif

extern int stmvl53l3_enable_debug;

#ifdef DEBUG
#	ifdef FORCE_CONSOLE_DEBUG
#define vl53l3_dbgmsg(str, ...) do { \
	if (stmvl53l3_enable_debug) \
		pr_info("%s: " str, __func__, ##__VA_ARGS__); \
} while (0)
#	else
#define vl53l3_dbgmsg(str, ...) do { \
	if (stmvl53l3_enable_debug) \
		pr_debug("%s: " str, __func__, ##__VA_ARGS__); \
	} while (0)
#	endif
#else
#	define vl53l3_dbgmsg(...) (void)0
#endif

/**
 * set to 0 1 activate or not debug from work (data interrupt/polling)
 */
#define WORK_DEBUG	0
#if WORK_DEBUG
#	define work_dbg(msg, ...)\
		printk("[D WK53L3] :" msg "\n", ##__VA_ARGS__)
#else
#	define work_dbg(...) (void)0
#endif

#define vl53l3_info(str, args...) \
	pr_info("%s: " str "\n", __func__, ##args)

#define vl53l3_errmsg(str, args...) \
	pr_err("%s: " str, __func__, ##args)

#define vl53l3_wanrmsg(str, args...) \
	pr_warn("%s: " str, __func__, ##args)

/* turn off poll log if not defined */
#ifndef STMVL53L3_LOG_POLL_TIMING
#	define STMVL53L3_LOG_POLL_TIMING	0
#endif
/* turn off cci log timing if not defined */
#ifndef STMVL53L3_LOG_CCI_TIMING
#	define STMVL53L3_LOG_CCI_TIMING	0
#endif

/**@} */ /* ingroup mod_dbg*/

#include <net/sock.h>
#include <linux/netlink.h>
#include <linux/wait.h>

#ifndef VL53L3_FULL_KERNEL
/** if set to 1 enable ipp execution timing (if debug enabled)
 * @ingroup vl53l3_mod_dbg
 */
#define IPP_LOG_TIMING	1

struct ipp_data_t {
	struct ipp_work_t work;
	struct ipp_work_t work_out;
	int test_n;
	/*!< buzy state 0 is idle
	 *any other value do not try to use (state value defined in source)
	 */
	int buzy;
	int waited_xfer_id;
	/*!< when buzy is set that is the id we are expecting
	 * note that value 0 is reserved and stand for "not waiting"
	 * as such never id 0 will be in any round trip exchange
	 * it's ok for daemon to use 0 in "ping" when it identify himself
	 */
	int status;	/** if that is not 0 do not look at out work data */
	wait_queue_head_t waitq;
	/*!< ipp caller are put in that queue wait while job is posted to user
	 * @warning  ipp and dev mutex will be released before waiting
	 * see @ref ipp_abort
	 */
#if IPP_LOG_TIMING
	struct timespec64 start_tv, stop_tv;
#endif
};
#endif

struct stmvl53l3_waiters {
	struct list_head list;
	pid_t pid;
};

/*
 *  driver data structs
 */
struct stmvl53l3_data {
	int id;			/*!< multiple device id 0 based*/
	char name[64];		/*!< misc device name */
	uint8_t product_type; /*!< L1:0xCC L3:0xAA i.e. module_type in datasheet */

	VL53L3_DevData_t stdev;	/*!<embed ST VL53L3 Dev data as "stdev" */
	hw_laser_ctrl_t *ctrl;
	void *client_object;	/*!< cci or i2c model i/f specific ptr  */
	bool is_device_remove;	/*!< true when device has been remove */

	struct mutex work_mutex; /*!< main dev mutex/lock */;
	struct delayed_work	dwork;

	struct wakeup_source *wake_lock;
	/*!< work for pseudo irq polling check  */

	struct input_dev *input_dev_ps;
	/*!< input device used for sending event */

	/* misc device */
	struct miscdevice miscdev;
	/* first irq has no valid data, so avoid to update data on first one */
	int is_first_irq;
	/* set when first start has be done */
	int is_first_start_done;

	/* control data */
	int poll_mode;	/*!< use poll even if interrupt line present*/
	int poll_delay_ms;	/*!< rescheduled time use in poll mode  */
	int enable_sensor;	/*!< actual device enabled state  */
	struct timespec64 start_tv;/*!< stream start time */
	int enable_debug;
	bool allow_hidden_start_stop; /*!< allow stop/start sequence in bare */
	unsigned long sensor_vote;
	unsigned long camera_vote;
	int is_first_valid_data;

	/* Custom values set by app */

	int preset_mode;	/*!< preset working mode of the device */
	uint32_t timing_budget;	/*!< Timing Budget */
	int distance_mode;	/*!< distance mode of the device */
	int crosstalk_enable;	/*!< is crosstalk compensation is enable */
	int output_mode;	/*!< output mode of the device */
	bool force_device_on_en;/*!< keep device active when stopped */
	VL53L3_Error last_error;/*!< last device internal error */
	int offset_correction_mode;/*!< offset correction mode to apply */
	FixPoint1616_t dmax_reflectance;/*!< reflectance use for dmax calc */
	int dmax_mode;		/*!< Dmax mode of the device */
	int smudge_correction_mode; /*!< smudge mode */

	/* Read only values */
	FixPoint1616_t optical_offset_x;
	FixPoint1616_t optical_offset_y;
	bool is_xtalk_value_changed; /*!< xtalk values has been updated */

	/* PS parameters */

	/* Calibration parameters */
	bool is_calibrating;	/*!< active during calibration phases */

	/* Range Data and stat */
	struct range_t {
		uint32_t	cnt;
		uint32_t	intr;
		int	poll_cnt;
		uint32_t	err_cnt; /* on actual measurement */
		uint32_t	err_tot; /* from start */
		struct timespec64 start_tv;
		struct timespec64 comp_tv;
		VL53L3_RangingMeasurementData_t single_range_data;
		VL53L3_MultiRangingData_t multi_range_data;
		VL53L3_MultiRangingData_t tmp_range_data;
		VL53L3_AdditionalData_t additional_data;
		/* non mode 1 for data agregation */
	} meas;

	/* calibration data */
	hwlaser_info_t pinfo;
	hwlaser_RangingData_t udata;
	/* workqueue use to fire flush event */
	uint32_t flushCount;
	int flush_todo_counter;

	/* Device parameters */
	/* Polling thread */
	/* Wait Queue on which the poll thread blocks */

	/* Manage blocking ioctls */
	struct list_head simple_data_reader_list;
	struct list_head mz_data_reader_list;
	wait_queue_head_t waiter_for_data;
	bool is_data_valid;

	/* control when using delay is acceptable */
	bool is_delay_allowed;

	/* maintain reset state */
	int reset_state;

	/* Recent interrupt status */
	/* roi */
	VL53L3_RoiConfig_t roi_cfg;

	/* use for zone calibration / roi mismatch detection */
	uint32_t current_roi_id;

	/* Embed here since it's too huge for kernek stack */
	struct stmvl53l3_ioctl_zone_calibration_data_t calib;

	/* autonomous config */
	uint32_t auto_pollingTimeInMs;
	VL53L3_DetectionConfig_t auto_config;
	/* print data */
	unsigned int print_counter;
	unsigned int print_interval;
	unsigned int print_detail;

	/* Debug */
#ifndef VL53L3_FULL_KERNEL
	struct ipp_data_t ipp;
#if IPP_LOG_TIMING
#	define stmvl531_ipp_tim_stop(data)\
	do_gettimeofday(&data->ipp.stop_tv)
#	define stmvl531_ipp_tim_start(data)\
	do_gettimeofday(&data->ipp.start_tv)
#	define stmvl531_ipp_time(data)\
	stmvl53l3_tv_dif(&data->ipp.start_tv, &data->ipp.stop_tv)
#	define stmvl531_ipp_stat(data, fmt, ...)\
	vl53l3_dbgmsg("IPPSTAT " fmt "\n", ##__VA_ARGS__)
#else
#	define stmvl531_ipp_tim_stop(data) (void)0
#	define stmvl531_ipp_tim_start(data) (void)0
#	define stmvl531_ipp_stat(...) (void)0
#endif
#endif
};


/**
 * timeval diff in us
 *
 * @param pstart_tv
 * @param pstop_tv
 */
long stmvl53l3_tv_dif(struct timespec64 *pstart_tv, struct timespec64 *pstop_tv);


/**
 * The device table list table is update as device get added
 * we do not support adding removing device mutiple time !
 * use for clean "unload" purpose
 */
extern struct stmvl53l3_data *stmvl53l3_dev_table[];

int stmvl53l3_setup(struct stmvl53l3_data *data);
void stmvl53l3_cleanup(struct stmvl53l3_data *data);
#ifdef CONFIG_PM_SLEEP
void stmvl53l3_pm_suspend_stop(struct stmvl53l3_data *data);
#endif
int stmvl53l3_intr_handler(struct stmvl53l3_data *data);

#ifndef VL53L3_FULL_KERNEL
/**
 * request ipp to abort or stop
 *
 * require dev work_mutex held
 *
 * @warning because the "waiting" work can't be aborted we must wake it up
 * it will happen and at some later time not earlier than release of lock
 * if after lock release we have a new request to start the race may not be
 * handled correctly
 *
 * @param data the device
 * @return 0 if no ipp got canceled, @warning this is maybe not grant we
 * can't re-sched "dev work"  and re-run the worker back
 */
int stmvl53l3_ipp_stop(struct stmvl53l3_data *data);

int stmvl53l3_ipp_do(struct stmvl53l3_data *data, struct ipp_work_t *work_in,
		struct ipp_work_t *work_out);

/**
 * per device netlink init
 * @param data
 * @return
 */
int stmvl53l3_ipp_setup(struct stmvl53l3_data *data);
/**
 * per device ipp netlink cleaning
 * @param data
 * @return
 */
void stmvl53l3_ipp_cleanup(struct stmvl53l3_data *data);

/**
 * Module init for netlink
 * @return 0 on success
 */
int stmvl53l3_ipp_init(void);

/**
 * Module exit for netlink
 * @return 0 on success
 */
void stmvl53l3_ipp_exit(void);

/**
 * enable and start ipp exhange
 * @param n_dev number of device to run on
 * @param data  dev struct
 * @return 0 on success
 */
int stmvl53l3_ipp_enable(int n_dev, struct stmvl53l3_data *data);
#endif

/*
 *  function pointer structs
 */


int stmvl53l3_probe(struct i2c_client *client, const struct i2c_device_id *id);
int stmvl53l3_remove(struct i2c_client *client);

#endif /* STMVL53L3_H */
