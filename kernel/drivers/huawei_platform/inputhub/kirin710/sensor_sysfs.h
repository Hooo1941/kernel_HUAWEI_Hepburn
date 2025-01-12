

#ifndef __SENSOR_SYSFS_H
#define __SENSOR_SYSFS_H

#include "contexthub_route.h"
#include "sensor_config.h"

struct pdr_start_config {
	unsigned int report_interval;
	unsigned int report_precise;
	unsigned int report_count;
	unsigned int report_times;
};

struct acc_gyr_offset_threshold {
	int32_t low_threshold;
	int32_t high_threshold;
};

typedef struct {
	unsigned int sub_cmd;
	union {
		struct pdr_start_config start_param;
		unsigned int stop_param;
	};
} pdr_ioctl_t;

enum {
	OFFSET_MIN_THREDHOLD,
	OFFSET_MAX_THREDHOLD,
	DIFF_MIN_THREDHOLD,
	DIFF_MAX_THREDHOLD
};

enum {
	PS_XTALK_CALIBRATE = 0x01,
	PS_5CM_CALIBRATE,    /* 2 */
	PS_3CM_CALIBRATE,    /* 3 */
	TOF_ZERO_CALIBRATE,  /* 4 */
	TOF_6CM_CALIBRATE,   /* 5 */
	TOF_10CM_CALIBRATE,  /* 6 */
	TOF_60CM_CALIBRATE,  /* 7 */
};

#define MAX_TOF_NUM 2
#define CAP_PROX_THRESHOLD_NUM      2
#define CAP_PROX_CAL_NUM            2

#ifdef CONFIG_HUAWEI_DSM
#ifdef CONFIG_HUAWEI_DATA_ACQUISITION
#define SENSOR_DATA_ACQUISITION
#endif
#endif

#ifdef SENSOR_DATA_ACQUISITION
#define NA                          "NA"
#define ACC_TEST_CAL                "ACC"
#define GYRO_TEST_CAL               "GYRO"
#define ALS_TEST_CAL                "ALS"
#define AIRPRESS_TEST_CAL           "AIRPRESS"
#define PS_TEST_CAL                 "PS"
#define CAP_PROX_TEST_CAL           "SAR"

#define ACC_CAL_RESULT              ((return_calibration == SUC) ? "pass" : "fail")
#define GYRO_CAL_RESULT             ((gyro_calibration_res == SUC) ? "pass" : "fail")
#define ALS_CAL_RESULT              ((als_calibration_res == SUC) ? "pass" : "fail")
#define PS_CAL_RESULT               ((ps_calibration_res == SUC) ? "pass" : "fail")
#define CAP_PROX_RESULT             "pass"

#define ACC_CAL_NUM                  15
#define ACC_THRESHOLD_NUM            3
#define GYRO_CAL_NUM                 15
#define GYRO_THRESHOLD_NUM           3
#define ALS_CAL_NUM                  6
#define ALS_THRESHOLD_NUM            6
#define PS_CAL_NUM                   4
#define PS_THRESHOLD_NUM             4
#define AIRPRESS_CAL_NUM             1

#define MAX_COPY_EVENT_SIZE          32
#define MAX_TEST_NAME_LEN            32
#define MAX_RESULT_LEN               8

#define  ACC_CALI_X_OFFSET_MSG       703015001
#define  ACC_CALI_Y_OFFSET_MSG       703015002
#define  ACC_CALI_Z_OFFSET_MSG       703015003
#define  ACC_CALI_X_SEN_MSG          703015004
#define  ACC_CALI_Y_SEN_MSG          703015005
#define  ACC_CALI_Z_SEN_MSG          703015006
#define  ACC_CALI_XIS_00_MSG         703015007
#define  ACC_CALI_XIS_01_MSG         703015008
#define  ACC_CALI_XIS_02_MSG         703015009
#define  ACC_CALI_XIS_10_MSG         703015010
#define  ACC_CALI_XIS_11_MSG         703015011
#define  ACC_CALI_XIS_12_MSG         703015012
#define  ACC_CALI_XIS_20_MSG         703015013
#define  ACC_CALI_XIS_21_MSG         703015014
#define  ACC_CALI_XIS_22_MSG         703015015

#define GYRO_CALI_X_OFFSET_MSG       703014001
#define GYRO_CALI_Y_OFFSET_MSG       703014002
#define GYRO_CALI_Z_OFFSET_MSG       703014003
#define GYRO_CALI_X_SEN_MSG          703014004
#define GYRO_CALI_Y_SEN_MSG          703014005
#define GYRO_CALI_Z_SEN_MSG          703014006
#define GYRO_CALI_XIS_00_MSG         703014007
#define GYRO_CALI_XIS_01_MSG         703014008
#define GYRO_CALI_XIS_02_MSG         703014009
#define GYRO_CALI_XIS_10_MSG         703014010
#define GYRO_CALI_XIS_11_MSG         703014011
#define GYRO_CALI_XIS_12_MSG         703014012
#define GYRO_CALI_XIS_20_MSG         703014013
#define GYRO_CALI_XIS_21_MSG         703014014
#define GYRO_CALI_XIS_22_MSG         703014015

#define ALS_CALI_R_MSG               703013001
#define ALS_CALI_G_MSG               703013002
#define ALS_CALI_B_MSG               703013003
#define ALS_CALI_C_MSG               703013004
#define ALS_CALI_LUX_MSG             703013005
#define ALS_CALI_CCT_MSG             703013006
#define ALS_CALI_DARK_OFFSET_MSG     703013007

#define SAR_SENSOR_OFFSET_MSG        703016001
#define SAR_SENSOR_DIFF_MSG          703016002

#define SAR_SENSOR_PH1_OFFSET_MSG    703016001
#define SAR_SENSOR_PH2_OFFSET_MSG    703016002
#define SAR_SENSOR_DIFF1_MSG         703016003
#define SAR_SENSOR_DIFF2_MSG         703016004

#define PS_CALI_XTALK                703018001
#define PS_FAR_PDATA                 703018002
#define PS_NEAR_PDATA                703018003
#define PS_OFFSET_PDATA              703018004

#define PS_CALI_NAME                 "PS_CALI_XTALK"
#define ALS_DARK_CALI_NAME           "ALS_DARK_CALI_OFFSET"
#define CAP_PROX_OFFSET              "SAR_SENSOR_OFFSET_MSG"
#define CAP_PROX_DIFF                "SAR_SENSOR_DIFF_MSG"

static char *acc_test_name[ACC_CAL_NUM] = {
	"ACC_CALI_X_OFFSET_MSG", "ACC_CALI_Y_OFFSET_MSG", "ACC_CALI_Z_OFFSET_MSG",
	"ACC_CALI_X_SEN_MSG", "ACC_CALI_Y_SEN_MSG", "ACC_CALI_Z_SEN_MSG",
	"ACC_CALI_XIS_00_MSG", "ACC_CALI_XIS_01_MSG", "ACC_CALI_XIS_02_MSG",
	"ACC_CALI_XIS_10_MSG", "ACC_CALI_XIS_11_MSG", "ACC_CALI_XIS_12_MSG",
	"ACC_CALI_XIS_20_MSG", "ACC_CALI_XIS_21_MSG", "ACC_CALI_XIS_22_MSG"
};
static char *gyro_test_name[GYRO_CAL_NUM] = {
	"GYRO_CALI_X_OFFSET_MSG", "GYRO_CALI_Y_OFFSET_MSG", "GYRO_CALI_Z_OFFSET_MSG",
	"GYRO_CALI_X_SEN_MSG", "GYRO_CALI_Y_SEN_MSG", "GYRO_CALI_Z_SEN_MSG",
	"GYRO_CALI_XIS_00_MSG", "GYRO_CALI_XIS_01_MSG", "GYRO_CALI_XIS_02_MSG",
	"GYRO_CALI_XIS_10_MSG", "GYRO_CALI_XIS_11_MSG", "GYRO_CALI_XIS_12_MSG",
	"GYRO_CALI_XIS_20_MSG", "GYRO_CALI_XIS_21_MSG", "GYRO_CALI_XIS_22_MSG"
};
static char *als_test_name[ALS_CAL_NUM] = {
	"ALS_CALI_R_MSG", "ALS_CALI_G_MSG", "ALS_CALI_B_MSG",
	"ALS_CALI_C_MSG", "ALS_CALI_LUX_MSG", "ALS_CALI_CCT_MSG"
};
static char *ps_test_name[PS_CAL_NUM] = {
	"PS_CALI_XTALK_MSG", "PS_FAR_PDATA_MSG", "PS_NEAR_PDATA_MSG", "PS_OFFSET_PDATA_MSG"
};

static char *cap_prox_offset_test_name[CAP_PROX_CAL_NUM] = {
	"SAR_SENSOR_PH1_OFFSET_MSG", "SAR_SENSOR_PH2_OFFSET_MSG"
};

static char *cap_prox_diff_test_name[CAP_PROX_CAL_NUM] = {
	"SAR_SENSOR_DIFF1_MSG", "SAR_SENSOR_DIFF2_MSG"
};

struct sensor_eng_cal_test {
	int first_item;
	int32_t *cal_value;
	int value_num;
	int32_t *min_threshold;
	int32_t *max_threshold;
	int threshold_num;
	char name[MAX_TEST_NAME_LEN];
	char result[MAX_RESULT_LEN];
	char *test_name[MAX_COPY_EVENT_SIZE];
};
#endif

#define CLI_TIME_STR_LEN    20
#define CLI_CONTENT_LEN_MAX 256
#define BL_SETTING_LEN      16
#define SENSOR_LIST_NUM     50
#define DEBUG_DATA_LENGTH   10
#define TO_DECIMALISM 10
#define TO_HEXADECIMAL 16

#define DATA_CLLCT      "/data/hwzd_logs/dataCollection.log"
#define HAND_DATA_CLLCT "/data/hwzd_logs/handSensorCalibData.log"

#define ACC_CALI_X_OFFSET     "testName:ACC_CALI_X_OFFSET*#value:%d*#minThreshold:NA*#maxThreshold:NA*#result:%s*#cycle:%d*#errorCode:%s*#time:%s*#\n"
#define ACC_CALI_Y_OFFSET     "testName:ACC_CALI_Y_OFFSET*#value:%d*#minThreshold:NA*#maxThreshold:NA*#result:%s*#cycle:%d*#errorCode:%s*#time:%s*#\n"
#define ACC_CALI_Z_OFFSET     "testName:ACC_CALI_Z_OFFSET*#value:%d*#minThreshold:NA*#maxThreshold:NA*#result:%s*#cycle:%d*#errorCode:%s*#time:%s*#\n"
#define ACC_CALI_X_SEN        "testName:ACC_CALI_X_SEN*#value:%d*#minThreshold:NA*#maxThreshold:NA*#result:%s*#cycle:%d*#errorCode:%s*#time:%s*#\n"
#define ACC_CALI_Y_SEN        "testName:ACC_CALI_Y_SEN*#value:%d*#minThreshold:NA*#maxThreshold:NA*#result:%s*#cycle:%d*#errorCode:%s*#time:%s*#\n"
#define ACC_CALI_Z_SEN        "testName:ACC_CALI_Z_SEN*#value:%d*#minThreshold:NA*#maxThreshold:NA*#result:%s*#cycle:%d*#errorCode:%s*#time:%s*#\n"
#define ACC_CALI_XIS_00       "testName:ACC_CALI_XIS_00*#value:%d*#minThreshold:NA*#maxThreshold:NA*#result:%s*#cycle:%d*#errorCode:%s*#time:%s*#\n"
#define ACC_CALI_XIS_01       "testName:ACC_CALI_XIS_01*#value:%d*#minThreshold:NA*#maxThreshold:NA*#result:%s*#cycle:%d*#errorCode:%s*#time:%s*#\n"
#define ACC_CALI_XIS_02       "testName:ACC_CALI_XIS_02*#value:%d*#minThreshold:NA*#maxThreshold:NA*#result:%s*#cycle:%d*#errorCode:%s*#time:%s*#\n"
#define ACC_CALI_XIS_10       "testName:ACC_CALI_XIS_10*#value:%d*#minThreshold:NA*#maxThreshold:NA*#result:%s*#cycle:%d*#errorCode:%s*#time:%s*#\n"
#define ACC_CALI_XIS_11       "testName:ACC_CALI_XIS_11*#value:%d*#minThreshold:NA*#maxThreshold:NA*#result:%s*#cycle:%d*#errorCode:%s*#time:%s*#\n"
#define ACC_CALI_XIS_12       "testName:ACC_CALI_XIS_12*#value:%d*#minThreshold:NA*#maxThreshold:NA*#result:%s*#cycle:%d*#errorCode:%s*#time:%s*#\n"
#define ACC_CALI_XIS_20       "testName:ACC_CALI_XIS_20*#value:%d*#minThreshold:NA*#maxThreshold:NA*#result:%s*#cycle:%d*#errorCode:%s*#time:%s*#\n"
#define ACC_CALI_XIS_21       "testName:ACC_CALI_XIS_21*#value:%d*#minThreshold:NA*#maxThreshold:NA*#result:%s*#cycle:%d*#errorCode:%s*#time:%s*#\n"
#define ACC_CALI_XIS_22       "testName:ACC_CALI_XIS_22*#value:%d*#minThreshold:NA*#maxThreshold:NA*#result:%s*#cycle:%d*#errorCode:%s*#time:%s*#\n"
#define PS_CALI_RAW_DATA      "testName:PS_CALI_RAW_DATA*#value:%d*#minThreshold:NA*#maxThreshold:NA*#result:%s*#cycle:%d*#errorCode:%s*#time:%s*#\n"
#define TOF_CALI_RAW_DATA     "testName:TOF_CALI_RAW_DATA*#value:%d*#minThreshold:NA*#maxThreshold:NA*#result:%s*#cycle:%d*#errorCode:%s*#time:%s*#\n"
#define ALS_CALI_R            "testName:ALS_CALI_R*#value:%d*#minThreshold:NA*#maxThreshold:NA*#result:%s*#cycle:NA*#errorCode:%s*#time:%s*#\n"
#define ALS_CALI_G            "testName:ALS_CALI_G*#value:%d*#minThreshold:NA*#maxThreshold:NA*#result:%s*#cycle:NA*#errorCode:%s*#time:%s*#\n"
#define ALS_CALI_B            "testName:ALS_CALI_B*#value:%d*#minThreshold:NA*#maxThreshold:NA*#result:%s*#cycle:NA*#errorCode:%s*#time:%s*#\n"
#define ALS_CALI_C            "testName:ALS_CALI_C*#value:%d*#minThreshold:NA*#maxThreshold:NA*#result:%s*#cycle:NA*#errorCode:%s*#time:%s*#\n"
#define ALS_CALI_LUX          "testName:ALS_CALI_LUX*#value:%d*#minThreshold:NA*#maxThreshold:NA*#result:%s*#cycle:NA*#errorCode:%s*#time:%s*#\n"
#define ALS_CALI_CCT          "testName:ALS_CALI_CCT*#value:%d*#minThreshold:NA*#maxThreshold:NA*#result:%s*#cycle:NA*#errorCode:%s*#time:%s*#\n"
#define GYRO_CALI_X_OFFSET    "testName:GYRO_CALI_X_OFFSET*#value:%d*#minThreshold:NA*#maxThreshold:NA*#result:%s*#cycle:%d*#errorCode:%s*#time:%s*#\n"
#define GYRO_CALI_Y_OFFSET    "testName:GYRO_CALI_Y_OFFSET*#value:%d*#minThreshold:NA*#maxThreshold:NA*#result:%s*#cycle:%d*#errorCode:%s*#time:%s*#\n"
#define GYRO_CALI_Z_OFFSET    "testName:GYRO_CALI_Z_OFFSET*#value:%d*#minThreshold:NA*#maxThreshold:NA*#result:%s*#cycle:%d*#errorCode:%s*#time:%s*#\n"
#define GYRO_CALI_X_SEN       "testName:GYRO_CALI_X_SEN*#value:%d*#minThreshold:NA*#maxThreshold:NA*#result:%s*#cycle:%d*#errorCode:%s*#time:%s*#\n"
#define GYRO_CALI_Y_SEN       "testName:GYRO_CALI_Y_SEN*#value:%d*#minThreshold:NA*#maxThreshold:NA*#result:%s*#cycle:%d*#errorCode:%s*#time:%s*#\n"
#define GYRO_CALI_Z_SEN       "testName:GYRO_CALI_Z_SEN*#value:%d*#minThreshold:NA*#maxThreshold:NA*#result:%s*#cycle:%d*#errorCode:%s*#time:%s*#\n"
#define GYRO_CALI_XIS_00      "testName:GYRO_CALI_XIS_00*#value:%d*#minThreshold:NA*#maxThreshold:NA*#result:%s*#cycle:%d*#errorCode:%s*#time:%s*#\n"
#define GYRO_CALI_XIS_01      "testName:GYRO_CALI_XIS_01*#value:%d*#minThreshold:NA*#maxThreshold:NA*#result:%s*#cycle:%d*#errorCode:%s*#time:%s*#\n"
#define GYRO_CALI_XIS_02      "testName:GYRO_CALI_XIS_02*#value:%d*#minThreshold:NA*#maxThreshold:NA*#result:%s*#cycle:%d*#errorCode:%s*#time:%s*#\n"
#define GYRO_CALI_XIS_10      "testName:GYRO_CALI_XIS_10*#value:%d*#minThreshold:NA*#maxThreshold:NA*#result:%s*#cycle:%d*#errorCode:%s*#time:%s*#\n"
#define GYRO_CALI_XIS_11      "testName:GYRO_CALI_XIS_11*#value:%d*#minThreshold:NA*#maxThreshold:NA*#result:%s*#cycle:%d*#errorCode:%s*#time:%s*#\n"
#define GYRO_CALI_XIS_12      "testName:GYRO_CALI_XIS_12*#value:%d*#minThreshold:NA*#maxThreshold:NA*#result:%s*#cycle:%d*#errorCode:%s*#time:%s*#\n"
#define GYRO_CALI_XIS_20      "testName:GYRO_CALI_XIS_20*#value:%d*#minThreshold:NA*#maxThreshold:NA*#result:%s*#cycle:%d*#errorCode:%s*#time:%s*#\n"
#define GYRO_CALI_XIS_21      "testName:GYRO_CALI_XIS_21*#value:%d*#minThreshold:NA*#maxThreshold:NA*#result:%s*#cycle:%d*#errorCode:%s*#time:%s*#\n"
#define GYRO_CALI_XIS_22      "testName:GYRO_CALI_XIS_22*#value:%d*#minThreshold:NA*#maxThreshold:NA*#result:%s*#cycle:%d*#errorCode:%s*#time:%s*#\n"
#define PRESS_CALI_OFFSET     "testName:PRESS_CALI_OFFSET*#value:%d*#minThreshold:NA*#maxThreshold:NA*#result:%s*#cycle:NA*#errorCode:%s*#time:%s*#\n"

#define RGB_SENSOR_CAL_FILE_PATH      "/data/light"
#define RGB_SENSOR_CAL_RESULT_MAX_LEN 96
#define GYRO_DYN_CALIBRATE_END_ORDER  5
#define SAR_ABOV_CH_0                 0
#define SAR_ABOV_CH_1                 1


int fingersense_commu(unsigned int cmd, unsigned int pare,
	unsigned int responsed, bool is_subcmd);
int fingersense_enable(unsigned int enable);
ssize_t sensors_calibrate_show(int tag, struct device *dev,
	struct device_attribute *attr, char *buf);
ssize_t sensors_calibrate_store(int tag, struct device *dev,
	struct device_attribute *attr, const char *buf, size_t count);
ssize_t show_sensor_read_airpress_common(struct device *dev,
	struct device_attribute *attr, char *buf);
int ois_commu(int tag, unsigned int cmd, unsigned int pare,
	unsigned int responsed, bool is_subcmd);
ssize_t show_cap_prox_calibrate_method(struct device *dev,
	struct device_attribute *attr, char *buf);
ssize_t show_cap_prox_calibrate_orders(struct device *dev,
	struct device_attribute *attr, char *buf);
extern volatile int hall_value;
struct read_info send_airpress_calibrate_cmd(uint8_t tag, unsigned long val,
	enum ret_type *rtype);
int *get_fingersense_enabled(void);
int get_stop_auto_als(void);
int get_stop_auto_ps(void);

#endif /* __SENSOR_SYSFS_H */
