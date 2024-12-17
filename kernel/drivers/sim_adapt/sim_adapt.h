/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2020. All rights reserved.
 * Description: power on/off sim card and eSIM for different device hardware types
 * Create: 2020-06-20
 * History: 2020-11-12 code style fix
 */

#ifndef _SIM_ADAPT_
#define _SIM_ADAPT_

#include <chipset_common/hwpower/hardware_ic/boost_5v.h>
#include <linux/delay.h>
#include <linux/device.h>
#include <linux/gpio.h>
#include <linux/init.h>
#include <linux/io.h>
#include <linux/irqdomain.h>
#include <linux/kern_levels.h>
#include <linux/kernel.h>
#include <linux/mfd/pmic_platform.h>
#include <linux/module.h>
#include <linux/mutex.h>
#include <linux/of.h>
#include <linux/of_address.h>
#include <linux/of_device.h>
#include <linux/of_gpio.h>
#include <linux/regulator/consumer.h>
#include <linux/slab.h>
#include <linux/string.h>
#include <linux/interrupt.h>

#define HW_SIM_OK 0
#define HW_SIM_ERR (-1)

#define SIM0 0
#define SIM1 1
#define MAX_SIMHP_NUM 2
#define USIM_OFFSET_SIZE 3
#define ESIM_DET_OFFSET_SIZE 4
#define SD_CMD_INDEX 0
#define SD_DATA0_INDEX 1
#define SD_DATA2_INDEX 2
#define SD_DATA3_INDEX 3

#define USIM_IO_MUX 0x001
#define GPIO_IO_MUX 0x000
#define SD_USIM_MUX 0x04
#define SD_GPIO_IO_MUX 0x00
#define USIM_IOCG_VALUE 0x10
#define USIM_SD_IOCG_PU_VALUE 0x11
#define USIM_SD_IOCG_PD_VALUE 0x12

#define STATUS_SIM 5
#define STATUS_SD 6
#define STATUS_NO_CARD 7
#define STATUS_SD2JTAG 8
#define VCC_1_8V 1800000
#define VCC_3_0V 3000000
#define VCC_2_95V 2950000
#define AP_POWER_SCHE_A 1
#define AP_POWER_SCHE_B 2
#define SIM_FUNC 2
#define GPIO_IO_FUNC 0
#define GPIO_SIM_FUNC 1
#define ESIM_PHY_CARD_DET_FUNC 3
#define SIM1_RST_GPIO_STR "sim1_rst_gpio" 
#define SIM1_DATA_GPIO_STR "sim1_data_gpio" 
#define SIM1_CLK_FUNC_ID 0
#define SIM1_RESET_FUNC_ID 1
#define SIM1_IO_FUNC_ID 2
#define ESIM_DET_FUNC 0
#define ESIM_DET_DATA_IO 1
#define ESIM_DET_RST_IO 2
#define ESIM_RESER_LOW_TIME 8000
#define ESIM_RESER_HIGH_MAX_TIME 8000
#define ESIM_RESER_HIGH_MAX_STEP 500
#define ESIM_DET_FUNC_ON 1
#define ESIM_NO_SD_3TO2_HARDWARE 1

#define HW_SIM_HARDWARE0 "hw-sim-hardware0"
#define HW_SIM_HARDWARE1 "hw-sim-hardware1"

#define hwsim_loge(fmt, ...) (void)printk(KERN_ERR "sim_adapt <%s>"fmt, __func__, ##__VA_ARGS__)
#define hwsim_logi(fmt, ...) (void)printk(KERN_INFO "sim_adapt <%s>"fmt, __func__, ##__VA_ARGS__)

typedef enum {
	SIM_POWER_TYPE_SINGLE = 0x01,
	SIM_POWER_TYPE_SIM_SD = 0x02,
	SIM_POWER_TYPE_EXTRAL_MODEM = 0x03,
	SIM_POWER_TYPE_EXTRAL_LDO = 0x04,
	SIM_POWER_TYPE_AUX = 0x05,
	SIM_POWER_TYPE_EXTRAL_LDO_BUT_NO_SD = 0x06, // teton slave card use esim io
	SIM_POWER_TYPE_SIM_SD_WITHOUT_IO_PMU = 0x07,
	SIM_POWER_TYPE_BUTT
} sim_power_type_e;

typedef enum {
	SIM_IO_MUX_SIMIO_USIM_NANOSDIO_SD_REQUEST = 0x01,
	SIM_IO_MUX_SIMIO_GPIO_NANOSDIO_USIM_REQUEST = 0x02,
	SIM_IO_MUX_SIMIO_USIM_ONLY_REQUEST = 0x03,
	SIM_IO_MUX_GPIO_DET_REQUEST = 0x04,
	SIM_IO_GPIO_FOR_NVT_SHIFT = 0x05,
	SIM_IO_USIM_FOR_NVT_SHIFT = 0x06,
	SIM_IO_MUX_3TO2_ESIM_NOSD_REQUEST = 0x07,
	SIM_IO_MUX_3TO2_SIM1_NOSD_REQUEST = 0x08,
	SIM_IO_MUX_BUTT
} set_sim_io_type;

typedef enum {
	EXTRAL_LDO_OUT_TYPE_GPIO = 0x01,
	EXTRAL_LDO_OUT_TYPE_PMU = 0x02
} extral_ldo_out_type_e;

typedef enum {
	SIM_TYPE_USIM = 0x01,
	SIM_TYPE_ESIM = 0x02
} current_usim_esim_type_e;

typedef enum {
	TYPE_NONE = 0x00,
	TYPE_CPLD = 0x01,
	TYPE_IODIE_STS = 0x02,
	TYPE_BUTT
} bridge_type;

typedef struct {
	u32 reg_bit;
	u32 class_c_value;
	u32 class_b_value;
} reg_bit_info;

typedef struct {
	u32 phy_addr;
	u32 phy_addr_offset;
	void *virt_addr;
	reg_bit_info sim_reg_info;
	reg_bit_info sd_reg_info;
} sim_sd_reg_info;

typedef struct {
	u32 func;
	u32 reg_off;
	u32 reg_bit;
} sim_sysctrl_s;

typedef struct {
	u32 func;
	u32 nvt_shift_gpio; // GPIO_090 SEL
	u32 nvt_fix_leak;
} nvt_shift_crl;

typedef struct {
	struct regulator *sim_vcc_io;
} single_sim_pmu_cfg;

typedef struct {
	struct regulator *sim_vcc;
	struct regulator *aux_ldo;
} aux_sim_pmu_cfg;

typedef struct {
	struct regulator *sim_sd_vcc;
	struct regulator *sim_sd_io;
} sim_sd_pmu_cfg;

typedef struct {
	u32 sim1_ap_power_scheme;
	struct regulator *sim_pmu_switch;
	struct regulator *sd_io_pmu;
	struct regulator *sd_vcc_pmu;
	struct regulator *sd_switch_pmu;
} extral_modem_pmu_cfg;

typedef struct {
	int extral_ldo_bias_use;
	extral_ldo_out_type_e extral_ldo_out_type;
	struct regulator *extral_ldo_en;
	struct regulator *extral_ldo_out_select;
	u32 extral_ldo_out_gpio_select;
	struct regulator *sim_sd_io;
} extral_ldo_pmu_cfg;

typedef struct {
	struct regulator *esim_vcc;
	unsigned int gpio_d0;
	unsigned int gpio_d1;
	unsigned int gpio_d2;
	unsigned int gpio_d3;
} esim_ctrl_cfg;

typedef enum {
	SIM_VCC_IO_FLAG,
	SIM_VCC_FLAG,
	AUX_LDO_FLAG,
	SIM_SD_VCC_FLAG,
	SIM_SD_IO_FLAG,
	SIM_PMU_SWITCH_FLAG,
	SD_IO_PMU_FLAG,
	SD_VCC_PMU_FLAG,
	SD_SWITCH_PMU_FLAG,
	EXTRAL_LDO_EN_FLAG,
	EXTRAL_LDO_OUT_SELECT_FLAG,
	EXTRAL_LDO_PMU_SIM_SD_IO_FLAG,
	ESIM_VCC_FLAG,
	REGULATOR_FLAG_BUTT
} sim_regulator_flag_type;

typedef struct {
	sim_power_type_e sim_pmu_hardware_type;
	sim_sysctrl_s lv_shift_ctrl;
	nvt_shift_crl nvt_shift;
	esim_ctrl_cfg esim_ctrl;
	union {
		single_sim_pmu_cfg single_sim_pmu;
		sim_sd_pmu_cfg sim_sd_pmu;
		extral_modem_pmu_cfg extral_modem_pmu;
		extral_ldo_pmu_cfg extral_ldo_pmu;
		aux_sim_pmu_cfg aux_sim_pmu;
	} hard_cfg;
} sim_pmu_hardware_ctrl;

struct sim_io_mux_cfg {
	u32 phy_addr;  // IOC_IOMG
	void *virt_addr;
	u32 pad_iomg_phy_addr; // PAD_IOMG
	void *pad_iomg_virt_addr;
	u32 pad_iocg_phy_addr; // PAD_IOCG
	void *pad_iocg_virt_addr;
	u32 iomg_usim_offset[USIM_OFFSET_SIZE];
	u32 iocg_usim_offset[USIM_OFFSET_SIZE];
	u32 usim1_iocfg_offset[USIM_OFFSET_SIZE];
	u32 esim_iocfg_offset[USIM_OFFSET_SIZE];
	u32 esim_detect_en;
	u32 esim_det_func_offset[ESIM_DET_OFFSET_SIZE];
	u32 esim_det_ctrl_offset[ESIM_DET_OFFSET_SIZE];
};

typedef struct {
	u32 type;
	u32 reg;
	u32 dump_reg;
	u32 bit; // 1:1.8V, 0:3.0V
} sim_briged_ctrl_info ;

struct hw_sim_hardware_info {
	struct device *spmidev;
	struct device_node *np;
	u32 sim_id;
	u32 sd_cmd_gpio; // gpio_321 for 8425L
	u32 sim2jtag_support;
	u32 lv_shift_ctrl_support;
	current_usim_esim_type_e sim_esim_type;
	u32 sim_status;
	u32 esim_classc_only;
	u32 esim_no_sd_3to2_hardware;
	u32 sim1_func[SIM_FUNC];
	u32 esim_func[SIM_FUNC];
	u32 esim_phy_card_det[ESIM_PHY_CARD_DET_FUNC];
	struct sim_io_mux_cfg sim_io_cfg;
	struct sim_io_mux_cfg sd_io_cfg;
	sim_pmu_hardware_ctrl sim_pmu_hardware;
	sim_sd_reg_info sim_sd_reg_cfg;
	sim_briged_ctrl_info bridge_ctrl;
	struct regulator *sim_det_vcc_io;
	int sim_data_irq;
	bool sim_card_det;
	bool sim_card_start_det;
	bool det_mode;
};

typedef enum {
	SIM_HW_CARD_TYPE_INQUIRE = 0x11,
	SIM_HW_CARD_DET_WHILE_ESIM_INQUIRE = 0x12,
	SIM_HW_POWER_ON_CLASS_C_REQUEST = 0x21, // set 1.8V voltage
	SIM_HW_POWER_ON_CLASS_B_REQUEST = 0x22, // set 3.0V voltage
	SIM_HW_POWER_OFF_REQUEST = 0x23,
	SIM_HW_DEACTIVE_NOTIFIER = 0x31,
	SIM_HW_SET_ESIM_SWITCH = 0x41,
	SIM_HW_SET_USIM_SWITCH = 0x42,
	SIM_HW_QUERY_DUMP_VOTE_RET_FAIL = 0x5B, // vote success, but return fail
	SIM_HW_QUERY_DUMP_VOTE_TIMEOUT = 0x5C, // vote not respone, timeout
	SIM_HW_MSG_TYPE_BUTT
} sci_hw_op_type_e;

typedef enum {
	SIM_HW_OP_RESULT_SUCC = 0x00,
	SIM_HW_HOTPLUG_DTS_ERR,
	SIM_HW_HOTPLUG_DET_ERR,
	SIM_HW_HOTPLUG_HPD_ERR,
	SIM_HW_HOTPLUG_STATE_ERR,
	SIM_HW_HOTPLUG_HPD_FAIL,
	SIM_HW_INVALID_INQUIRE_MSG = 0x100,
	SIM_HW_CARD_TYPE_SIM = 0x101,
	SIM_HW_CARD_TYPE_MUXSIM = 0x102,
	SIM_HW_CARD_TYPE_SD,
	SIM_HW_CARD_TYPE_JTAG,
	SIM_HW_CARD_TYPE_FAIL,
	SIM_HW_CARD_EXIST_WHILE_ESIM,
	SIM_HW_CARD_NO_EXIST_WHILE_ESIM,
	SIM_HW_SET_GPIO_FAIL,
	SIM_HW_INVALID_POWER_MSG = 0x200,
	SIM_HW_POWER_ON_SUCC = 0x201,
	SIM_HW_POWER_ON_FAIL,
	SIM_HW_POWER_OFF_SUCC,
	SIM_HW_POWER_OFF_FAIL,
	SIM_HW_REQUESR_PMU_FAIL,
	SIM_HW_SET_VOLTAGE_FAIL,
	SIM_HW_INVALID_NOTIFIER_MSG = 0x300,
	SIM_HW_DEACTIVE_SUCC = 0x301,
	SIM_HW_DEACTIVE_FAIL,
	SIM_HW_INVALID_SWITCH_MSG = 0x400,
	SIM_HW_SET_ESIM_SUCC = 0x401,
	SIM_HW_SET_ESIM_FAIL,
	SIM_HW_SET_USIM_SUCC,
	SIM_HW_SET_USIM_FAIL,
	SIM_HW_QUERY_DUMP_SUCC = 0x501,
	SIM_HW_REQUEST_RESULT_BUTT
} sci_hw_op_result_e;

struct sim_hw_debug_info {
	u32 iomux_status[USIM_OFFSET_SIZE];
	u32 power_status;
	u32 event_len; /* event queue len */
	u32 data[]; /* events */
};

extern u8 get_card1_type(void);
extern int sim_hwadp_init(u32 sim_id, struct device *dev);
extern sci_hw_op_result_e sim_hwadp_handler(u32 sim_id, sci_hw_op_type_e msg_id);
extern int sim_get_hw_debug_info(u32 sim_id, struct sim_hw_debug_info *dbg_info);
sci_hw_op_result_e esim_no_sd_phy_card_det(struct hw_sim_hardware_info *info);
void put_regulator(u32 sim_id, sim_regulator_flag_type type, const struct regulator *pmu);
int hw_sim_read_regulator(u32 sim_id, sim_regulator_flag_type type, struct regulator **sim_regulator,
	const struct device *spmidev, const char *prop_str);

#endif // _SIM_ADAPT_

