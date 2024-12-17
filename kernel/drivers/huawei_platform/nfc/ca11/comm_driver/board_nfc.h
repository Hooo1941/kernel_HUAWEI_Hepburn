

#ifndef __BOARD_NFC_H__
#define __BOARD_NFC_H__

enum NFC_CHIPTYPE {
	NFCTYPE_INVALID = 0,
	NFCTYPE_CA01    = 0x7,
	NFCTYPE_CA11    = 0x8
};

enum {
	BOARD_SUCC = 0,
	BOARD_FAIL = (-1)
};

typedef enum {
	NFC_DISABLE = 0,
	NFC_ENABLE  = 1
} nfc_enable_ctrl_t;

// 板级初始化
int board_init(void *param);
void board_deinit(void);

// 业务接口
void board_pwn_ctrl(nfc_enable_ctrl_t enable);               // 控制device上下电
void board_wkup_host_gpio_int_enable(nfc_enable_ctrl_t en);  // device唤醒host的gpio中断使能和除能
int board_get_dev_wkup_host_status(void);                    // 获取device唤醒host的gpio电平状态
int board_get_host_wkup_dev_status(void);                    // 获取host唤醒device的gpio电平状态
void board_set_host_wkup_dev_value(int value);               // 设置host唤醒device的gpio电平状态
const char *board_get_fw_cfg_path(void);                     // 获取固件配置文件的存放路径
int board_i2c_write(char *buf, int count);                   // i2c_write板级接口
int board_i2c_read(char *buf, int count);                    // i2c_reade板级接口

#ifdef PLATFORM_DEBUG_ENABLE
// SSI相关接口
int board_get_ssi_data_gpio_value(void);
void board_set_ssi_data_gpio_value(int value);
void board_set_ssi_clk_gpio_value(int value);
void board_ssi_gpio_init(void);
void board_ssi_gpio_restore(void);
#endif

#endif
