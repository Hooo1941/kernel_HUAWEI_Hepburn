
#include "priv_nfc_driver.h"
#include "oal_os.h"
#include "oal_plat_op.h"
#include "board_nfc.h"
#include "securec.h"

// host唤醒device脉冲宽度（至少1ms）, 某些host计时精度不够，1可能被识别为0，可以在板级重定义
#ifndef DEVICE_WAKEUP_PULSE_TIME_MS
#define DEVICE_WAKEUP_PULSE_TIME_MS 1
#endif
// host唤醒device需要的时间, 根据不同的应用场景，device被唤醒的时间不一样，可在板级重定义
#ifndef DEVICE_WAKEUP_TIME_MS
#define DEVICE_WAKEUP_TIME_MS 1
#endif

#define DEVICE_BOOT_LOAD_TIME_MS 10 // 拉pwr_en后，device进入boot可以处理加载命令的时间
#define DEVICE_BOOT_WORK_TIME_MS 30 // 加载完后，device启动时间

#ifndef WRITE_TIME_MAX
#define WRITE_TIME_MAX 30
#endif

#define VERSION_LEN 64
#define DOWNLOAD_CMD_LEN 16
#define DOWNLOAD_CMD_PARA_LEN 96

/* 以下是发往device命令的关键字 */
#define VER_CMD_KEYWORD             "VERSION"
#define JUMP_CMD_KEYWORD            "JUMP"
#define FILES_CMD_KEYWORD           "FILES"
#define RMEM_CMD_KEYWORD            "READM"
#define WMEM_CMD_KEYWORD            "WRITEM"
#define QUIT_CMD_KEYWORD            "QUIT"
/* 以下是cfg文件配置命令的参数头，一条合法的cfg命令格式为:参数头+命令关键字(QUIT命令除外) */
#define FILE_TYPE_CMD_KEY           "ADDR_FILE_"
#define NUM_TYPE_CMD_KEY            "PARA_"

#define COMPART_KEYWORD ' '
#define CMD_LINE_SIGN   ';'
#define CFG_INFO_RESERVE_LEN 0

#define READ_CFG_BUF_LEN 384

#define FILE_COUNT_PER_SEND 1
#define RECV_BUF_LEN 256
#define FILE_HEAD_TYPE 0x7e

#define READ_LEN_PER_TIME   255

enum fw_cfg_cmd_enum {
	ERROR_TYPE_CMD = 0,            /* 错误的命令 */
	FILE_TYPE_CMD,                 /* 下载文件的命令 */
	NUM_TYPE_CMD,                  /* 下载配置参数的命令 */
	QUIT_TYPE_CMD                 /* 退出命令 */
};

enum fw_data_type_enum {
	TYPE_QUIT                = 0x0,
	TYPE_SET_DOWNLOAD_ADDR   = 0x1,
	TYPE_FILE_DOWNLOAD       = 0x2,
	TYPE_JUMP                = 0x3,
	TYPE_READM               = 0x4,
	TYPE_WRITEM              = 0x5,
	TYPE_CRC                 = 0x6,
	TYPE_LOOPBACK            = 0x7,

	TYPE_BUTT
};

typedef struct {
	int cmd_type;
	unsigned char cmd_name[DOWNLOAD_CMD_LEN];
	unsigned char cmd_para[DOWNLOAD_CMD_PARA_LEN];
} cmd_type_t;

typedef struct {
	int al_count;              /* 存储每个cfg文件解析后有效的命令个数 */
	cmd_type_t *apst_cmd;      /* 存储每个cfg文件的有效命令 */
	unsigned char auc_CfgVersion[VERSION_LEN];   /* 存储cfg文件中配置的版本号信息 */
	unsigned char auc_DevVersion[VERSION_LEN];   /* 存储加载时device侧上报的版本号信息 */
} firmware_globals_t;

struct download_file_addr_head {
	char start;
	char type;
	short len;
	unsigned int addr;
};
struct writem_content {
	unsigned int  len;
	unsigned int    addr;
	unsigned int    value;
};
struct readm_content {
	unsigned int  addr;
	unsigned int  len;
};
struct writem_cmd {
	struct download_file_head header;
	struct writem_content writevalue;
};
struct readm_cmd {
	struct download_file_head header;
	struct readm_content readvalue;
};
struct jump_cmd {
	struct download_file_head header;
	unsigned int addr;
};

oal_mutex_t g_read_mutex;
oal_mutex_t g_dev_mutex; // 访问deivce的互斥锁
oal_wait_queue_head_t g_read_wq;

/* 存储cfg文件信息，解析cfg文件时赋值，加载的时候使用该变量 */
firmware_globals_t  g_cfg_info;

#if defined(BUILD_FIRMWARE_TO_ARRAY) || defined(BIULD_FIRMWARE_TO_ARRAY)
__attribute__((weak)) const int fw_ext_file_len = 0;
__attribute__((weak)) const unsigned char fw_ext_file_data[0];
#endif

void (*g_device_wakeup_host_cb)(void) = NULL;

static int firmware_parse_cfg(char *cfg_info_buf, int buf_len);
static int firmware_parse_cmd(unsigned char *puc_cfg_buffer, unsigned char *puc_cmd_name, unsigned int cmd_name_len,
							unsigned char *puc_cmd_para, unsigned int cmd_para_len);
static int firmware_read_cfg(const char *cfg_path, unsigned char *read_buffer, int buffer_len);

static int exec_file_type_cmd(unsigned char *value);
static int exec_number_type_cmd(unsigned char *key, unsigned char *value);
static int exec_quit_type_cmd(void);
static int execute_download_cmd(int cmd_type, unsigned char *cmd_name, unsigned char *cmd_para);
static void string_to_num(const unsigned char *string, int *number);
static int parse_file_cmd(unsigned char *string, unsigned long *addr, char **file_path);

void nfc_host_wkup_dev(void)
{
	board_set_host_wkup_dev_value(0);
	oal_msleep(DEVICE_WAKEUP_PULSE_TIME_MS);
	board_set_host_wkup_dev_value(1);
	PS_PRINT_INFO("nfc_host_wkup_dev success!\n");
	oal_msleep(DEVICE_WAKEUP_PULSE_TIME_MS);
	board_set_host_wkup_dev_value(0);
}

static int device_i2c_write(char *buf, int count)
{
	int ret;
	int retry;

	for (retry = 0; retry < WRITE_TIME_MAX; retry++) {
		ret = board_i2c_write(buf, count);
		if (ret == count) {
			break;
		} else {
			if (retry > 0) {
				PS_PRINT_INFO("send data try =%d returned %d\n", retry, ret);
			} else {
				nfc_host_wkup_dev();
			}
			oal_msleep(DEVICE_WAKEUP_TIME_MS);
			continue;
		}
	}

	if (retry == WRITE_TIME_MAX) {
		PS_PRINT_ERR("device_i2c_write send fail.\n");
		return -NFC_EFAIL;
	}

	return ret;
}

static int device_i2c_read(char *buf, int count)
{
	int ret;

	if (count > MAX_RECV_DATA_LEN) {
		PS_PRINT_ERR("read len:%d, change to %d\n", count, MAX_RECV_DATA_LEN);
		count = MAX_RECV_DATA_LEN;
	}

	if (count == 0) {
		PS_PRINT_ERR("expect count is 0\n");
		return 0;
	}

	if (!board_get_dev_wkup_host_status()) {
		PS_PRINT_ERR("device no data to report\n");
		return 0;
	}

	ret = board_i2c_read(buf, count);
	if ((ret < 0) || (ret > count)) {
		PS_PRINT_ERR("i2c read data fail, len: %d, expect:%d\n", ret, count);
		return ret;
	}

	return ret;
}

static int wait_to_read(unsigned long timeout)
{
	int ret;
	board_wkup_host_gpio_int_enable(NFC_ENABLE);
	ret = oal_wait_event_interruptible_timeout(g_read_wq, board_get_dev_wkup_host_status(), timeout);
	if (ret != 0)
		PS_PRINT_INFO("wait_event_interruptible return fail\n");
	return ret;
}

int nfc_i2c_write(char *buf, int count)
{
	int ret;
	oal_mutex_lock(&g_dev_mutex);
	ret = device_i2c_write(buf, count);
	oal_mutex_unlock(&g_dev_mutex);
	return ret;
}

// 注意，此函数只能被priv_i2c_read_statble调用
static int priv_i2c_read(char *buf, int count, unsigned long timeout)
{
	int ret;

	oal_mutex_lock(&g_dev_mutex); // device访问互斥
	ret = board_get_dev_wkup_host_status();
	PS_PRINT_INFO("nfc_i2c_read gpio_get_value is %d, count = %d.\n", ret, count);
	if (ret == 0) {
		oal_mutex_unlock(&g_dev_mutex); // 无数据读时会在wait_to_read中阻塞，释放锁，可以给device发送数据
		ret = wait_to_read(timeout);
		if (ret != 0)
			return ret;
		oal_mutex_lock(&g_dev_mutex);
	}

	ret = device_i2c_read(buf, count);
	oal_mutex_unlock(&g_dev_mutex);

	return ret;
}

// 注意，此函数只能内部调用
static inline int priv_i2c_read_stable(char *buf, int count, unsigned long timeout)
{
	int ret;
	do
		ret = priv_i2c_read(buf, count, timeout);
	while (ret == 0); // ret 如果为0， 可能是dev_wakeup_host_gpio误触发信号
	return ret;
}

int nfc_i2c_read(char *buf, int count, unsigned long timeout)
{
	int ret;
	if (count == 0) {
		PS_PRINT_WARNING("read count is 0\n");
		return 0;
	}

	oal_mutex_lock(&g_read_mutex); // 防止多个线程同时读
	ret = priv_i2c_read_stable(buf, count, timeout);
	oal_mutex_unlock(&g_read_mutex);

	return ret;
}

int nfc_i2c_nci_read(char *buf, int buf_len, unsigned long timeout)
{
	int ret;
	if (buf_len <= NCI_HEAD_LEN) {
		PS_PRINT_WARNING("buf_len <= NCI_HEAD_LEN\n");
		return 0;
	}

	oal_mutex_lock(&g_read_mutex); // 防止多个线程同时读

	// read nci head
	ret = priv_i2c_read_stable(buf, NCI_HEAD_LEN, timeout);
	// read payload
	if (ret == NCI_HEAD_LEN) {
		uint8_t *pars_buf = (uint8_t *)buf; // 确保之后的计算是正数
		int payload_len = pars_buf[NCI_HEAD_LEN - 1];

		if (payload_len > buf_len - NCI_HEAD_LEN) {
			PS_PRINT_ERR("buf_len = %d, less than payload_len = %d\n", buf_len, payload_len);
			payload_len = buf_len - NCI_HEAD_LEN;
		}

		ret = priv_i2c_read_stable(buf + NCI_HEAD_LEN, payload_len, timeout);

		if (payload_len != pars_buf[NCI_HEAD_LEN - 1]) {
			ret = -1;
		} else {
			ret += NCI_HEAD_LEN;
		}
	}
	oal_mutex_unlock(&g_read_mutex);
	return ret;
}

static int firmware_read_cfg(const char *cfg_path, unsigned char *read_buffer, int buffer_len)
{
	int ret;
	oal_file_t *fp = NULL;

	if ((cfg_path == NULL) || (read_buffer == NULL)) {
		PS_PRINT_ERR("para is NULL\n");
		return -NFC_EFAIL;
	}

	fp = oal_file_open_readonly(cfg_path);
	if (fp == NULL)
		return -NFC_EFAIL;

	if (memset_s(read_buffer, buffer_len, 0, buffer_len) != EOK) {
		PS_PRINT_ERR("memset_s error\n");
		return -NFC_EFAIL;
	}

	ret = oal_file_read_ext(fp, fp->f_pos, (char *)read_buffer, buffer_len);
	if (ret <= 0)
		PS_PRINT_ERR("kernel_read error, ret = %d\n", ret);

	oal_file_close(fp);
	fp = NULL;

	return ret;
}

static int parse_file_cmd(unsigned char *str, unsigned long *addr, char **file_path)
{
	unsigned char *tmp = NULL;
	int count = 0;
	char *after = NULL;

	if (str == NULL || addr == NULL || file_path == NULL) {
		PS_PRINT_ERR("param is error!\n");
		return -NFC_EFAIL;
	}

	/* 获得发送的文件的个数，此处必须为1，str字符串的格式必须是"1,0xXXXXX,file_path" */
	tmp = str;
	while (*tmp == COMPART_KEYWORD)
		tmp++;

	string_to_num(tmp, &count);
	if (count != FILE_COUNT_PER_SEND) {
		PS_PRINT_ERR("the count of send file must be 1, count = [%d]\n", count);
		return -NFC_EFAIL;
	}

	/* 让tmp指向地址的首字母 */
	tmp = (unsigned char *)strchr((const char *)str, ',');
	if (tmp == NULL) {
		PS_PRINT_ERR("param string is err!\n");
		return -NFC_EFAIL;
	} else {
		tmp++;
		while (*tmp == COMPART_KEYWORD)
			tmp++;
	}

	*addr = (unsigned long)oal_strtoul((const char *)tmp, &after, 16); /* 将字符串转换成16进制数 */

	/* "1,0xXXXX,file_path" */
	/*         ^          */
	/*       after        */
	/* 跳过空格和','字符 */
	while ((*after == COMPART_KEYWORD) || (*after == ','))
		after++;

	*file_path = after;

	return NFC_SUCC;
}

static void string_to_num(const unsigned char *str, int *number)
{
	int i, num;

	if (str == NULL) {
		PS_PRINT_ERR("str is NULL!\n");
		return;
	}

	num = 0;
	for (i = 0; (str[i] >= '0') && (str[i] <= '9'); i++)
		num = (num * 10) + (str[i] - '0');   /* 10进制字符串转数字的逻辑需要 */

	*number = num;
}

static int nfc_send_and_rcv(char *buf, int count)
{
	int ret;
	char recv_buf[RECV_BUF_LEN];
	unsigned short len = 0;

	ret = device_i2c_write(buf, count);
	if (ret < 0) {
		PS_PRINT_ERR("device_i2c_write failed, ret:%d\n", ret);
		return -1;
	}
	/* read response */
	if (wait_to_read(0) != 0)
		return -1;
	ret = device_i2c_read((char *)&len, sizeof(len));
	if ((ret < 0) || (len >= RECV_BUF_LEN)) {
		PS_PRINT_ERR("device_i2c_read fail, ret:%d, len=%d\n", ret, len);
		return ret;
	}

	if (wait_to_read(0) != 0)
		return -1;
	ret = device_i2c_read(recv_buf, len);
	if (ret < 0) {
		PS_PRINT_ERR("device_i2c_read fail, ret:%d\n", ret);
		return ret;
	}
	return NFC_SUCC;
}

static int writem_cmd_send(char *value)
{
	struct writem_cmd write_cmd;
	int ret;
	char *tmp = NULL;
	int count = 0;
	unsigned int addr;
	int data = 0;
	char *after = NULL;

	/* string字符串的格式须是"2,0xXXXXX,value" */
	tmp = value;
	while (*tmp == COMPART_KEYWORD)
		tmp++;
	string_to_num((const unsigned char *)tmp, &count);
	PS_PRINT_INFO("the send file count = [%d]\n", count);

	/* 让tmp指向地址的首字母 */
	tmp = strchr(value, ',');
	if (tmp == NULL) {
		PS_PRINT_ERR("param string is err!\n");
		return -NFC_EFAIL;
	} else {
		tmp++;
		while (*tmp == COMPART_KEYWORD)
			tmp++;
	}

	addr = (unsigned int)oal_strtoul(tmp, &after, 16); /* 将字符串转换成16进制数 */

	/* "2,0xXXXX,value" */
	/*         ^          */
	/*       after        */
	/* 跳过空格和','字符 */
	while ((*after == COMPART_KEYWORD) || (*after == ','))
		after++;
	string_to_num((unsigned char *)after, &data);

	write_cmd.header.start  = FILE_HEAD_TYPE;
	write_cmd.header.type   = TYPE_WRITEM;
	write_cmd.header.len    = sizeof(struct writem_content);
	write_cmd.writevalue.len = (unsigned int)count;
	write_cmd.writevalue.addr = addr;
	write_cmd.writevalue.value = (unsigned int)data;

	ret = nfc_send_and_rcv((char *)&write_cmd, sizeof(write_cmd));
	if (ret < 0) {
		PS_PRINT_ERR("nfc_send_and_rcv failed, ret:%d\n", ret);
		return -NFC_EFAIL;
	}

	return NFC_SUCC;
}

static int jump_cmd_send(const char *value)
{
	struct jump_cmd cmd;
	int ret;
	char *after = NULL;

	cmd.header.start  = FILE_HEAD_TYPE;
	cmd.header.type   = TYPE_JUMP;
	cmd.header.len    = sizeof(cmd.addr);
	cmd.addr = (unsigned int)oal_strtoul(value, &after, 16); // 16进制转换

	ret = nfc_send_and_rcv((char *)&cmd, sizeof(cmd));
	if (ret < 0) {
		PS_PRINT_ERR("nfc_send_and_rcv failed, ret:%d\n", ret);
		return -NFC_EFAIL;
	}

	return NFC_SUCC;
}

static int readm_cmd_send(char *value)
{
	struct readm_cmd read_cmd;
	int ret;
	char *tmp = NULL;
	int count = 0;
	char *after = NULL;

	/* string字符串的格式须是"2,0xXXXXX" */
	tmp = value;
	while (*tmp == COMPART_KEYWORD)
		tmp++;
	string_to_num((const unsigned char *)tmp, &count);

	/* 让tmp指向地址的首字母 */
	tmp = strchr(value, ',');
	if (tmp == NULL) {
		PS_PRINT_ERR("param string is err!\n");
		return -NFC_EFAIL;
	} else {
		tmp++;
		while (*tmp == COMPART_KEYWORD)
			tmp++;
	}

	read_cmd.header.start  = FILE_HEAD_TYPE;
	read_cmd.header.type   = TYPE_READM;
	read_cmd.header.len    = sizeof(struct readm_content);
	read_cmd.readvalue.len = (unsigned int)count;
	read_cmd.readvalue.addr = (unsigned int)oal_strtoul(tmp, &after, 16); /* 将字符串转换成16进制数 */

	ret = nfc_send_and_rcv((char *)&read_cmd, sizeof(read_cmd));
	if (ret < 0) {
		PS_PRINT_ERR("nfc_send_and_rcv failed, ret:%d\n", ret);
		return -NFC_EFAIL;
	}

	return NFC_SUCC;
}

static int file_cmd_send(const char *path, unsigned long addr)
{
	unused(addr);
	unsigned int file_len, per_send_len, send_count;
	int ret, rdlen;
	unsigned int i;
	unsigned int offset = 0;
	oal_file_t *fp = NULL;
	unsigned char fw_down_buff[READ_LEN_PER_TIME] = { 0 };
	struct download_file_head cmd_head = { 0 };

	ret = oal_file_open_get_len(path, &fp, &file_len);
	if (ret < 0)
		return ret;

	per_send_len = READ_LEN_PER_TIME;
	send_count = (file_len + per_send_len - 1) / per_send_len;
	PS_PRINT_INFO("firmware file_len=%d, send_count=%d\n", file_len, send_count);
	for (i = 0; i < send_count; i++) {
		rdlen = oal_file_read_ext(fp, fp->f_pos, (char *)fw_down_buff, per_send_len);
		if (rdlen > 0) {
			PS_PRINT_DBG("len of kernel_read is [%d], i=%d\n", rdlen, i);
			fp->f_pos += rdlen;
		} else {
			PS_PRINT_ERR("len of kernel_read is error! ret=[%d], i=%d\n", rdlen, i);
			oal_file_close(fp);
			return rdlen;
		}

		cmd_head.start  = FILE_HEAD_TYPE;
		cmd_head.type   = TYPE_FILE_DOWNLOAD;
		cmd_head.len    = (short)rdlen;
		ret = device_i2c_write((char *)&cmd_head, sizeof(cmd_head));
		if (ret < 0) {
			PS_PRINT_ERR("device_i2c_write failed, ret:%d\n", ret);
			return -NFC_EFAIL;
		}
		ret = nfc_send_and_rcv((char *)fw_down_buff, rdlen);
		if (ret < 0) {
			PS_PRINT_ERR("nfc_send_and_rcv failed, ret:%d\n", ret);
			return -NFC_EFAIL;
		}
		offset += (unsigned int)rdlen;
	}

	ret = oal_file_close(fp);
	/* 发送的长度要和文件的长度一致 */
	if (ret != 0 || offset != file_len) {
		PS_PRINT_ERR("send file len err! send len is [%d], file len is [%d], ret = %d\n", offset, file_len, ret);
		return -NFC_EFAIL;
	}

	return NFC_SUCC;
}

static int exec_file_type_cmd(unsigned char *value)
{
	unsigned long addr;
	char *path = NULL;
	int ret;
	struct download_file_addr_head addr_head = { 0 };

	ret = parse_file_cmd(value, &addr, &path);
	if (ret < 0) {
		PS_PRINT_ERR("parse file cmd fail!\n");
		return ret;
	}

	addr_head.start  = FILE_HEAD_TYPE;
	addr_head.type   = TYPE_SET_DOWNLOAD_ADDR;
	addr_head.len    = sizeof(addr_head.addr);
	addr_head.addr = (unsigned int)addr;
	ret = nfc_send_and_rcv((char *)&addr_head, sizeof(addr_head));
	if (ret < 0) {
		PS_PRINT_ERR("nfc_send_and_rcv failed, ret:%d\n", ret);
		return -NFC_EFAIL;
	}

	ret = file_cmd_send(path, addr);
	if (ret < 0) {
		PS_PRINT_ERR("download file  fail!\n");
		return ret;
	}

	return NFC_SUCC;
}

static int exec_number_type_cmd(unsigned char *key, unsigned char *value)
{
	int ret = -NFC_EFAIL;

	if (!strcmp((char *)key, WMEM_CMD_KEYWORD)) {
		ret = writem_cmd_send((char *)value);
		if (ret < 0) {
			PS_PRINT_ERR("send writem key=[%s],value=[%s] fail\n", key, value);
			return ret;
		}
	} else if (!strcmp((char *)key, JUMP_CMD_KEYWORD)) {
		ret = jump_cmd_send((char *)value);
		if (ret < 0) {
			PS_PRINT_ERR("send jump key=[%s],value=[%s] fail\n", key, value);
			return ret;
		}
	} else if (!strcmp((char *)key, RMEM_CMD_KEYWORD)) {
		ret = readm_cmd_send((char *)value);
		if (ret < 0) {
			PS_PRINT_ERR("send readm key=[%s],value=[%s] fail\n", key, value);
			return ret;
		}
	}

	return NFC_SUCC;
}

static int exec_quit_type_cmd(void)
{
	int ret;
	struct download_file_addr_head quit_cmd;

	quit_cmd.start = FILE_HEAD_TYPE;
	quit_cmd.type = TYPE_QUIT;
	quit_cmd.len = sizeof(quit_cmd.addr);
	quit_cmd.addr = 0x0;
	PS_PRINT_INFO("enter quit");
	ret = nfc_send_and_rcv((char *)&quit_cmd, sizeof(quit_cmd));
	if (ret < 0) {
		PS_PRINT_ERR("nfc_send_and_rcv failed, ret:%d\n", ret);
		return -NFC_EFAIL;
	}

	return NFC_SUCC;
}

static int execute_download_cmd(int cmd_type, unsigned char *cmd_name, unsigned char *cmd_para)
{
	int ret;

	switch (cmd_type) {
	case FILE_TYPE_CMD:
		PS_PRINT_INFO(" command type FILE_TYPE_CMD\n");
		ret = exec_file_type_cmd(cmd_para);
		break;
	case NUM_TYPE_CMD:
		PS_PRINT_INFO(" command type NUM_TYPE_CMD\n");
		ret = exec_number_type_cmd(cmd_name, cmd_para);
		break;
	case QUIT_TYPE_CMD:
		PS_PRINT_INFO(" command type QUIT_TYPE_CMD\n");
		ret = exec_quit_type_cmd();
		break;
	default:
		PS_PRINT_ERR("command type error[%d]\n", cmd_type);
		ret = -NFC_EFAIL;
		break;
	}

	return ret;
}

static void *malloc_cmd_buf(unsigned char *cfg_info_buf)
{
	size_t len;
	unsigned char *flag = NULL;
	unsigned char *buf = NULL;

	if (cfg_info_buf == NULL) {
		PS_PRINT_ERR("malloc_cmd_buf: buf is NULL!\n");
		return NULL;
	}

	/* 统计命令个数 */
	flag = cfg_info_buf;
	g_cfg_info.al_count = 0;
	while (flag != NULL) {
		/* 一个正确的命令行结束符为 ; */
		flag = (unsigned char *)strchr((const char *)flag, CMD_LINE_SIGN);
		if (flag == NULL) {
			break;
		}
		g_cfg_info.al_count++;
		flag++;
	}

	/* 申请存储命令空间 */
	len = (size_t)((g_cfg_info.al_count) + CFG_INFO_RESERVE_LEN) * sizeof(cmd_type_t);
	buf = oal_malloc(len);
	if (buf == NULL) {
		PS_PRINT_ERR("kmalloc CMD_TYPE_ST fail\n");
		return NULL;
	}
	memset_s((void *)buf, len, 0, len);

	return buf;
}

static unsigned char *delete_space(unsigned char *string, int *len)
{
	int i;

	if ((string == NULL) || (len == NULL))
		return NULL;

	/* 删除尾部的空格 */
	for (i = *len - 1; i >= 0; i--) {
		if (string[i] != COMPART_KEYWORD) {
			break;
		}
		string[i] = '\0';
	}
	/* 出错 */
	if (i < 0) {
		PS_PRINT_ERR(" string is Space bar\n");
		return NULL;
	}
	/* 在for语句中减去1，这里加上1 */
	*len = i + 1;

	/* 删除头部的空格 */
	for (i = 0; i < *len; i++) {
		if (string[i] != COMPART_KEYWORD) {
			/* 减去空格的个数 */
			*len = *len - i;
			return &string[i];
		}
	}

	return NULL;
}

static int get_cmd_type(unsigned char **handle_ptr, int *cmd_len_ptr)
{
	int cmd_type;
	unsigned char *handle_temp = NULL;
	/* 判断命令类型 */
	if (!memcmp(*handle_ptr, (unsigned char *)FILE_TYPE_CMD_KEY, strlen((char *)FILE_TYPE_CMD_KEY))) {
		handle_temp = (unsigned char *)strstr((const char *)(*handle_ptr), (const char *)FILE_TYPE_CMD_KEY);
		if (handle_temp == NULL) {
			PS_PRINT_ERR("'ADDR_FILE_'is not handle child string, *handle_ptr=%s", *handle_ptr);
			return ERROR_TYPE_CMD;
		}
		*handle_ptr = handle_temp + strlen(FILE_TYPE_CMD_KEY);
		*cmd_len_ptr = *cmd_len_ptr - strlen(FILE_TYPE_CMD_KEY);
		cmd_type = FILE_TYPE_CMD;
	} else if (!memcmp(*handle_ptr, (unsigned char *)NUM_TYPE_CMD_KEY, strlen(NUM_TYPE_CMD_KEY))) {
		handle_temp = (unsigned char *)strstr((const char *)(*handle_ptr), (const char *)NUM_TYPE_CMD_KEY);
		if (handle_temp == NULL) {
			PS_PRINT_ERR("'PARA_' is not handle child string, *handle_ptr=%s", *handle_ptr);
			return ERROR_TYPE_CMD;
		}
		*handle_ptr = handle_temp + strlen(NUM_TYPE_CMD_KEY);
		*cmd_len_ptr = *cmd_len_ptr - strlen(NUM_TYPE_CMD_KEY);
		cmd_type = NUM_TYPE_CMD;
	} else {
		return ERROR_TYPE_CMD;
	}

	return cmd_type;
}

static int firmware_parse_cmd(unsigned char *puc_cfg_buffer, unsigned char *puc_cmd_name, unsigned int cmd_name_len,
							  unsigned char *puc_cmd_para, unsigned int cmd_para_len)
{
	int cmd_type, cmd_len, para_len;
	unsigned char *begin, *end, *link, *handle;

	end = NULL;
	link = NULL;
	handle = NULL;
	begin = puc_cfg_buffer;
	if ((puc_cfg_buffer == NULL) || (puc_cmd_name == NULL) || (puc_cmd_para == NULL)) {
		PS_PRINT_ERR("para is NULL\n");
		return ERROR_TYPE_CMD;
	}

	/* 注释行 */
	if (puc_cfg_buffer[0] == '@')
		return ERROR_TYPE_CMD;

	/* 错误行，或者退出命令行 */
	link = (unsigned char *)strchr((char *)begin, '=');
	if (link == NULL) {
		/* 退出命令行 */
		if (strstr((char *)puc_cfg_buffer, QUIT_CMD_KEYWORD) != NULL)
			return QUIT_TYPE_CMD;

		return ERROR_TYPE_CMD;
	}

	/* 错误行，没有结束符 */
	end = (unsigned char *)strchr((char *)link, ';');
	if (end == NULL)
		return ERROR_TYPE_CMD;

	cmd_len = link - begin;

	/* 删除关键字的两边空格 */
	handle = delete_space((unsigned char *)begin, &cmd_len);
	if (handle == NULL)
		return ERROR_TYPE_CMD;

	cmd_type = get_cmd_type(&handle, &cmd_len);
	if (cmd_type == ERROR_TYPE_CMD)
		return ERROR_TYPE_CMD;

	if (memcpy_s(puc_cmd_name, cmd_name_len, handle, (size_t)cmd_len) != EOK)
		PS_PRINT_ERR("memcpy_s error\n");

	/* 删除值两边空格 */
	begin = link + 1;
	para_len = end - begin;

	handle = delete_space((unsigned char *)begin, &para_len);
	if (handle == NULL)
		return ERROR_TYPE_CMD;

	if (memcpy_s(puc_cmd_para, cmd_para_len, handle, (size_t)para_len) != EOK)
		PS_PRINT_ERR("memcpy_s error\n");
	return cmd_type;
}

static int firmware_parse_cfg(char *cfg_info_buf, int buf_len)
{
	int i, len, ret, cmd_type;
	char *flag = NULL;
	char *begin = NULL;
	char *end = NULL;
	char cmd_name[DOWNLOAD_CMD_LEN];
	char cmd_para[DOWNLOAD_CMD_PARA_LEN];

	if (cfg_info_buf == NULL) {
		PS_PRINT_ERR("cfg_info_buf is NULL!\n");
		return -NFC_EFAIL;
	}

	g_cfg_info.apst_cmd = (cmd_type_t *)malloc_cmd_buf((unsigned char *)cfg_info_buf);
	if (g_cfg_info.apst_cmd == NULL) {
		PS_PRINT_ERR(" malloc_cmd_buf fail!\n");
		return -NFC_EFAIL;
	}

	/* 解析CMD BUF */
	flag = cfg_info_buf;
	len = buf_len;
	i = 0;
	while ((i < g_cfg_info.al_count) && (flag < &cfg_info_buf[len])) {
		/*
		 * 获取配置文件中的一行,配置文件必须是unix格式.
		 * 配置文件中的某一行含有字符 @ 则认为该行为注释行
		 */
		begin = flag;
		end   = strchr(flag, '\n');
		if (end == NULL) {           /* 文件的最后一行，没有换行符 */
			PS_PRINT_DBG("lost of new line!\n");
			end = &cfg_info_buf[len];
		} else if (end == begin) {     /* 该行只有一个换行符 */
			PS_PRINT_DBG("blank line\n");
			flag = end + 1;
			continue;
		}
		*end = '\0';

		memset_s(cmd_name, DOWNLOAD_CMD_LEN, 0, DOWNLOAD_CMD_LEN);
		memset_s(cmd_para, DOWNLOAD_CMD_PARA_LEN, 0, DOWNLOAD_CMD_PARA_LEN);

		cmd_type = firmware_parse_cmd((unsigned char *)begin, (unsigned char *)cmd_name, sizeof(cmd_name),
									  (unsigned char *)cmd_para, sizeof(cmd_para));
		if (cmd_type != ERROR_TYPE_CMD) {
			g_cfg_info.apst_cmd[i].cmd_type = cmd_type;
			ret = memcpy_s(g_cfg_info.apst_cmd[i].cmd_name, DOWNLOAD_CMD_LEN, cmd_name, DOWNLOAD_CMD_LEN);
			ret = memcpy_s(g_cfg_info.apst_cmd[i].cmd_para, DOWNLOAD_CMD_PARA_LEN, cmd_para, DOWNLOAD_CMD_PARA_LEN);
			if (ret != EOK)
				PS_PRINT_ERR("memcpy_s fail, ret = %d!\n", ret);
			i++;
		}

		flag = end + 1;
	}

	/* 根据实际命令个数，修改最终的命令个数 */
	g_cfg_info.al_count = i;
	PS_PRINT_INFO("effective cmd count: al_count = %d\n", g_cfg_info.al_count);

	return NFC_SUCC;
}

static int firmware_get_cfg(const char *cfg_path)
{
	unsigned char *read_cfg_buf = NULL;
	int read_len;
	int ret;

	if (cfg_path == NULL) {
		PS_PRINT_ERR("cfg file path is null!\n");
		return -NFC_EFAIL;
	}

	/* cfg文件限定在小于2048,如果cfg文件的大小确实大于2048，可以修改READ_CFG_BUF_LEN的值 */
	read_cfg_buf = (unsigned char *)oal_malloc(READ_CFG_BUF_LEN);
	if (read_cfg_buf == NULL) {
		PS_PRINT_ERR("kmalloc READ_CFG_BUF fail!\n");
		return -NFC_EFAIL;
	}

	read_len = firmware_read_cfg(cfg_path, read_cfg_buf, READ_CFG_BUF_LEN);
	if (read_len < 0) {
		PS_PRINT_ERR("read cfg error!\n");
		oal_free(read_cfg_buf);
		read_cfg_buf = NULL;
		return -NFC_EFAIL;
	} else if (read_len > READ_CFG_BUF_LEN - 1) {
		/* 减1是为了确保cfg文件的长度不超过READ_CFG_BUF_LEN，因为firmware_read_cfg最多只会读取READ_CFG_BUF_LEN长度的内容 */
		PS_PRINT_ERR("cfg file [%s] larger than %d\n", cfg_path, READ_CFG_BUF_LEN);
		oal_free(read_cfg_buf);
		read_cfg_buf = NULL;
		return -NFC_EFAIL;
	} else {
		PS_PRINT_DBG("read cfg file [%s] ok, size is [%d]\n", cfg_path, read_len);
	}

	ret = firmware_parse_cfg((char *)read_cfg_buf, read_len);
	oal_free(read_cfg_buf);
	read_cfg_buf = NULL;
	if (ret < 0) {
		PS_PRINT_ERR("parse cfg error!\n");
		return -NFC_EFAIL;
	}
	PS_PRINT_INFO("get cfg succ!\n");

	return ret;
}

#ifdef CONFIG_CA11_NFC
int nfc_chip_power_on(void)
{
	oal_mutex_lock(&g_dev_mutex);
	// power off
	board_pwn_ctrl(NFC_DISABLE);
	oal_msleep(20); // 下电后20ms再重新上电
	// power on
	board_pwn_ctrl(NFC_ENABLE);
	oal_msleep(DEVICE_BOOT_WORK_TIME_MS);
	oal_mutex_unlock(&g_dev_mutex);

	PS_PRINT_INFO("finish power on\n");
	return NFC_SUCC;
}
#else
int nfc_chip_power_on(void)
{
	int i, ret;
	int cmd_type;
	unsigned char *cmd_name = NULL;
	unsigned char *cmd_para = NULL;

	static unsigned short cfg_file_init = 0;

	if (!cfg_file_init) {
		ret = firmware_get_cfg(board_get_fw_cfg_path());
		if (ret != NFC_SUCC) {
			PS_PRINT_ERR("download fw firmware_get_cfg fail\n");
			return -NFC_EFAIL;
		}
		cfg_file_init = 1;
	}

	oal_mutex_lock(&g_dev_mutex);
	// power off
	board_pwn_ctrl(NFC_DISABLE);
	oal_msleep(20); // 下电后20ms再重新上电
	// power on
	board_pwn_ctrl(NFC_ENABLE);
	oal_msleep(DEVICE_BOOT_LOAD_TIME_MS);

	for (i = 0; i < g_cfg_info.al_count; i++) {
		cmd_type = g_cfg_info.apst_cmd[i].cmd_type;
		cmd_name = g_cfg_info.apst_cmd[i].cmd_name;
		cmd_para = g_cfg_info.apst_cmd[i].cmd_para;

		ret = execute_download_cmd(cmd_type, cmd_name, cmd_para);
		if (ret < 0) {
			PS_PRINT_ERR("download firmware fail\n");
			oal_mutex_unlock(&g_dev_mutex);
			return ret;
		}
		oal_msleep(20); // 命令之间延时20ms, 等待上一个命令处理完
	}
	oal_msleep(DEVICE_BOOT_WORK_TIME_MS);
	oal_mutex_unlock(&g_dev_mutex);

	PS_PRINT_INFO("finish download firmware\n");
	return NFC_SUCC;
}
#endif

void nfc_wakeup_host_gpio_isr(void)
{
	if (board_get_dev_wkup_host_status() != 0) {
		PS_PRINT_INFO("This is true of wakeup_host_gpio_isr.\n");
		if (g_device_wakeup_host_cb != NULL)
			g_device_wakeup_host_cb();
	} else {
		PS_PRINT_ERR("wakeup_host_gpio_isr false positives, and set read event also.\n");
	}
	board_wkup_host_gpio_int_enable(NFC_DISABLE);
	oal_wkup_event(&g_read_wq);
}

int nfc_driver_init(void *param)
{
	unused(param);
	oal_init_waitqueue_head(&g_read_wq);
	oal_mutex_init(&g_read_mutex);
	oal_mutex_init(&g_dev_mutex);
	return NFC_SUCC;
}
void nfc_driver_deinit(void)
{
	oal_delete_waitqueue_head(&g_read_wq);
	oal_mutex_destroy(&g_read_mutex);
	oal_mutex_destroy(&g_dev_mutex);
}

// 上层业务注册device唤醒事件回调，设置为NULL为解注册
void nfc_register_wakeup_host_callback(void (*cb)(void))
{
	g_device_wakeup_host_cb = cb;
}
