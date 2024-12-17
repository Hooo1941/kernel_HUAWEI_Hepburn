/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2021-2021. All rights reserved.
 * Description: Contexthub dump private head file
 * Create: 2021-11-26
 */
#ifndef __IOMCU_DUMP_PRIV_H__
#define __IOMCU_DUMP_PRIV_H__

#include <linux/types.h>
#include <linux/of_address.h>
#include <platform_include/smart/linux/iomcu_status.h>
#include <platform_include/smart/linux/iomcu_dump.h>
#include <platform_include/smart/linux/iomcu_config.h>

#define SENSORHUB_DUMP_BUFF_ADDR DDR_LOG_BUFF_ADDR_AP
#define SENSORHUB_DUMP_BUFF_SIZE DDR_LOG_BUFF_SIZE
#define IS_ENABLE_DUMP 1

#ifdef CONFIG_CONTEXTHUB_BOOT_STAT
union sctrl_sh_boot {
	unsigned int value;
	struct {
		unsigned int boot_stat : 24; /* bit[8-31]    : boot stat */
		unsigned int boot_step : 8;  /* bit[24 ~ 31] : 0~255 boot step pos */
	} reg;
};

union sctrl_sh_boot get_boot_stat(void);
void reset_boot_stat(void);
union sctrl_sh_boot iomcu_show_boot_step(void);
#endif

int get_sysctrl_base(void);
int get_nmi_offset(void);
void send_nmi(void);
void clear_nmi(void);
void iomcu_get_mntn_sctrl_regs(void);
#ifdef CONFIG_DFX_DEBUG_FS
void show_iom3_stat(void);
#endif

int save_sh_dump_file(uint8_t * sensorhub_dump_buff, union dump_info dump_info,
	void __iomem *ao_ns_ipc_base, unsigned int *dump_mailno, unsigned int mailno_count);
#endif /* __IOMCU_DUMP_PRIV_H__ */
