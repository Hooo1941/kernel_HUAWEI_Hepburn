/*
* Copyright (c) Huawei Technologies Co., Ltd. 2021-2021. All rights reserved.
* Description: IOMCU CONFIG DESC
* Create: 2021-12-13
*/

#ifndef __DDR_CONFIG_H__
#define __DDR_CONFIG_H__

#define EMG_LEVEL   0
#define ERR_LEVEL   1
#define WARN_LEVEL  2
#define INFO_LEVEL  3
#define DEBUG_LEVEL 4

#define PA_MARGIN_NUM           2 /* PA_HCI:0 PA_NBTI:1 */
#define NPU_AVS_DATA_NUN        14
#define NPU_SVFD_DATA_NUN       28
#define NPU_SVFD_PARA_NUN       2
#define PROFILE_NUM             7
#define DDR_CONFIG_MAGIC        0xABCD1243

#define VERSION_LENGTH          16

enum iomcu_config_usr_e {
	IOMCU_CONFIG_USR_DUMP = 0,
	IOMCU_CONFIG_USR_LOG,
	IOMCU_CONFIG_USR_WAKEUP,
	IOMCU_CONFIG_USR_GYRO,
	IOMCU_CONFIG_USR_IGS,
	IOMCU_CONFIG_USR_TIME,
	IOMCU_CONFIG_USR_HIFI,
	IOMCU_CONFIG_USR_MODEM,
	IOMCU_CONFIG_USR_COUL,
	IOMCU_CONFIG_USR_BBOX,
	IOMCU_CONFIG_USR_ALS,
	IOMCU_CONFIG_USR_VERSION,
	IOMCU_CONFIG_USR_PEDO,
	IOMCU_CONFIG_USR_MOTION,
	IOMCU_CONFIG_USR_NEARBY_BEACON,
	IOMCU_CONFIG_USR_ALS1,
	IOMCU_CONFIG_USR_MAX,
};

enum dump_loc {
	DL_NONE = 0,
	DL_TCM,
	DL_EXT,
	DL_BOTTOM = DL_EXT,
};

enum dump_region {
	DE_TCM_CODE,
	DE_DDR_CODE,
	DE_DDR_DATA,
	DE_BOTTOM = 16,
};

#define IOMCU_CONFIG_SIZE          (DDR_CONFIG_SIZE / 2)

struct iomcu_config_node {
	int id;
	int offset;
	int len;
	int resv; // reserved for align 8.
};

struct iomcu_config {
	struct iomcu_config_node node[IOMCU_CONFIG_USR_MAX];
	unsigned long long magic;
	unsigned int used;
	unsigned int resv; // reserved for align 8.
};

struct dump_region_config {
	unsigned char loc;
	unsigned char reserved[7]; // reserved for align 8.
};

struct dump_config {
	struct dump_region_config elements[DE_BOTTOM];
	unsigned long long dump_addr;
	unsigned int dump_size;
	unsigned int reserved1;
	unsigned long long ext_dump_addr;
	unsigned int ext_dump_size;
	unsigned char enable;
	unsigned char finish;
	unsigned char reason;
	unsigned char reserved2;
};

struct log_buff_t {
	unsigned int mutex;
	unsigned short index;
	unsigned short pingpang;
	unsigned int buff;
	unsigned int ddr_log_buff_cnt;
	unsigned int ddr_log_buff_index;
	unsigned int ddr_log_buff_last_update_index;
};

struct wakeup_config {
	unsigned int flag;
	unsigned int reserved1;
	unsigned long long time;
	unsigned int irq0;
	unsigned int irq1;
	unsigned int recvfromapmsg[4];
	unsigned int recvfromlpmsg[4];
	unsigned int sendtoapmsg[4];
	unsigned int sendtolpmsg[4];
	unsigned int recvfromapmsgmode;
	unsigned int recvfromlpmsgmode;
	unsigned int sendtoapmsgmode;
	unsigned int sendtolpmsgmode;
};
struct timestamp_kernel {
	unsigned long long syscnt;
	unsigned long long kernel_ns;
};

struct timestamp_iomcu_base {
	unsigned long long syscnt;
	unsigned long long kernel_ns;
	unsigned int timer32k_cyc;
	unsigned int reserved;
};

struct coul_core_info {
	signed int c_offset_a;
	signed int c_offset_b;
	signed int r_coul_mohm;
	signed int reserved;
};

struct read_data_als_ud {
	float rdata;
	float gdata;
	float bdata;
	float irdata;
};

struct bright_data {
	unsigned int mipi_data;
	unsigned int bright_data;
	unsigned long long time_stamp;
	unsigned int panel_id;
};
struct als_ud_config {
	struct bright_data bright_data_input;
	struct read_data_als_ud read_data_history;
	unsigned long long als_rgb_pa;
	unsigned char screen_status;
	unsigned char reserved[7]; /* 7 is reserved nums */
};
struct bbox_config {
	unsigned long long log_addr;
	unsigned int log_size;
	unsigned int rsvd;
};

struct log_config {
	struct log_buff_t log_buff_cb_backup;
	unsigned int log_level;
	unsigned int resv;
};

struct gyro_config {
	float gyro_offset[3];
	int resv; // reserved for align 8.
};

struct modem_config {
	unsigned char modem_open_app_state;
	unsigned char reserved[7]; /* 7 is reserved nums */
};

struct hifi_config {
	unsigned char hifi_open_sound_msg;
	unsigned char reserved[7]; /* 7 is reserved nums */
};

struct npu_cfg_data {
	unsigned int pa_margin[PA_MARGIN_NUM];
	unsigned int avs_data[NPU_AVS_DATA_NUN];
	unsigned int svfd_data[NPU_SVFD_DATA_NUN];
	unsigned int svfd_para[NPU_SVFD_PARA_NUN];
	unsigned int prof_margin[PROFILE_NUM];
	unsigned int level2_chip;
};

struct igs_config {
	struct npu_cfg_data npu_data;
	unsigned char igs_hardware_bypass;
	unsigned char screen_sts;
	unsigned char sc_flag;
	unsigned char is_pll_on;
	unsigned char resv[4]; // reserved for align 8
};

struct time_config {
	struct timestamp_kernel timestamp_base;
	struct timestamp_iomcu_base timestamp_base_iomcu;
};

struct coul_config {
	struct coul_core_info coul_info;
};

struct als_config {
	struct als_ud_config als_ud_config;
	unsigned int lcd_freq;
	unsigned int te_irq_tcs3701;
	unsigned int old_dc_flag;
	unsigned int resv;
};

struct version_config {
	char sensorhub_version[VERSION_LENGTH];
};

struct pedo_config {
	unsigned int pedo_total_steps;
	unsigned char pedo_steps_valid;
	unsigned char reserved[3];
};

struct motion_config {
	unsigned long long reserved;
	unsigned int fall_down_support;
	unsigned char phone_type_info[2];
	unsigned char resv[2]; // reserved for align 8.
	unsigned int hall_gpio_id;
	unsigned int hall_gpio_id2; // GL_AB_HALL
	unsigned int board_id;
};

struct nearby_beacon_config {
	unsigned char screen_status;
	unsigned char reserved[7]; // reserved for align 8.
};
#endif