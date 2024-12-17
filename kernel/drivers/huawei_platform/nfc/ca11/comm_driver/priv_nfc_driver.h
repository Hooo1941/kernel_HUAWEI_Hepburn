

#ifndef __PRIV_NFC_DRIVER_H__
#define __PRIV_NFC_DRIVER_H__

#define MAX_SEND_DATA_LEN 255 // 用户发送数据的最大长度
#define MAX_RECV_DATA_LEN 255 // 用户接收数据的最大长度
#define NCI_HEAD_LEN 3

enum return_type_enum {
	NFC_SUCC = 0,
	NFC_EFAIL,
};

struct download_file_head {
	char start;
	char type;
	short len;
};

int nfc_driver_init(void *param);
void nfc_driver_deinit(void);

void nfc_wakeup_host_gpio_isr(void); // 在上层初始化时被注册
void nfc_host_wkup_dev(void);
int nfc_i2c_write(char *buf, int count);
int nfc_i2c_read(char *buf, int count, unsigned long timeout);
int nfc_i2c_nci_read(char *buf, int buf_len, unsigned long timeout);
int nfc_chip_power_on(void);

// 上层业务注册device唤醒事件回调，设置为NULL为解注册
void nfc_register_wakeup_host_callback(void (*cb)(void));

#endif
