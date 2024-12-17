/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2019-2021. All rights reserved.
 * Description: Contexthub share memory driver
 * Create: 2019-10-01
 */
#ifndef __IOMCU_IPC_H__
#define __IOMCU_IPC_H__

#include <linux/types.h>
#include <platform_include/smart/linux/base/ap/protocol.h>
#include <platform_include/basicplatform/linux/ipc_rproc.h>

#ifdef __cplusplus
extern "C" {
#endif

#ifdef CONFIG_INPUTHUB_30

#define AMF20_CMD_ALL      0xFFFF
#define TAG_ALL            TAG_END

#define IOMCU_IPC_SEND     0x11


struct write_info {
	int tag;
	int cmd;
	const void *wr_buf; /* maybe NULL */
	int wr_len; /* maybe zero */
	int app_tag;
};

struct read_info {
	int errno;
	int data_length;
	char data[MAX_PKT_LENGTH];
};

/**
 * @brief register ipc send command notifier.
 *
 * @param nb     : notifier.
 * @return int   : SUCCESS or FAIL.
 */
int register_mcu_send_cmd_notifier(struct notifier_block *nb);

/**
 * @brief unregister ipc send command notifier.
 *
 * @param nb     : notifier.
 * @return int   : SUCCESS or FAIL.
 */
int unregister_mcu_send_cmd_notifier(struct notifier_block *nb);

/**
 * @brief send ipc to iomcu(amsm).
 *
 * @param wr      : write info.
 * @param rd      : read info.
 * @param is_lock : lock when send.
 * @return INT32 : SUCCESS or FAIL.
 */
int write_customize_cmd(const struct write_info *wr, struct read_info *rd, bool is_lock);

/**
 * @brief register notify for recving data from IOMCU(amsm).
 *
 * @param tag      : tag.
 * @param cmd      : cmd.
 * @param notify   : notify.
 * @return INT32 : SUCCESS or FAIL.
 */
int register_mcu_event_notifier(int tag, int cmd, int (*notify)(const struct pkt_header *head));

/**
 * @brief unregister notify for recving data from IOMCU(amsm).
 *
 * @param tag      : tag.
 * @param cmd      : cmd.
 * @param notify   : notify.
 * @return INT32 : SUCCESS or FAIL.
 */
int unregister_mcu_event_notifier(int tag, int cmd, int (*notify)(const struct pkt_header *head));

#ifdef CONFIG_IPC_V5
typedef int (*tcp_amf20_notify_f)(const uint16_t svc_id, const uint16_t cmd, const void *data, const uint16_t len);

/**
 * @brief send ipc to iomcu(amf20).
 *
 * @param svc_id   : service id.
 * @param cmd      : command.
 * @param data     : data.
  * @param         : data_len.
 * @return INT32 : SUCCESS or FAIL.
 */
int tcp_send_data_by_amf20(const uint16_t svc_id, const uint16_t cmd, uintptr_t *data, uint16_t data_len);

/**
 * @brief register notify for recving data from IOMCU(amf20).
 *
 * @param svc_id       : service id.
 * @param cmd          : cmd.
 * @param tcp_notify   : notify.
 * @return INT32 : SUCCESS or FAIL.
 */
int register_amf20_notifier(uint16_t svc_id, uint16_t cmd, tcp_amf20_notify_f notify);

/**
 * @brief unregister notify for recving data from IOMCU(amf20).
 *
 * @param svc_id       : service id.
 * @param cmd          : cmd.
 * @param tcp_notify   : notify.
 * @return INT32 : SUCCESS or FAIL.
 */
int unregister_amf20_notifier(uint16_t svc_id, uint16_t cmd, tcp_amf20_notify_f notify);

void ipc_shm_free_for_sh_crash(void);
#endif
#endif

/*
 * send cmd to sensorhub with sub-cmd
 */
int send_cmd_from_kernel(unsigned char cmd_tag, unsigned char cmd_type, unsigned int subtype, const char *buf,
			 size_t count);
/*
 * send cmd to sensorhub with sub-cmd and wait response
 */
int send_cmd_from_kernel_response(unsigned char cmd_tag, unsigned char cmd_type, unsigned int subtype, const char *buf,
				  size_t count, struct read_info *rd);
/*
 * send cmd to sensorhub with sub-cmd without lock
 */
int send_cmd_from_kernel_nolock(unsigned char cmd_tag, unsigned char cmd_type, unsigned int subtype, const char *buf,
				size_t count);

#ifdef __cplusplus
}
#endif
#endif
