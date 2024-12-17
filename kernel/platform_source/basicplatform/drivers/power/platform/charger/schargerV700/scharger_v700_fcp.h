/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2022-2022. All rights reserved.
 * Description: Device driver for regulators in PMIC IC
 */
#include <soc_schargerV700_interface.h>

#ifndef TRUE
#define TRUE 1
#endif
#ifndef FALSE
#define FALSE 0
#endif

#define FCP_ADAPTER_5V                  5000
#define FCP_ADAPTER_9V                  9000
#define FCP_ADAPTER_12V                 12000
#define FCP_ADAPTER_MAX_VOL             (FCP_ADAPTER_12V)
#define FCP_ADAPTER_MIN_VOL             (FCP_ADAPTER_5V)
#define FCP_ADAPTER_RST_VOL             (FCP_ADAPTER_5V)
#define FCP_ADAPTER_VOL_CHECK_ERROR     500
#define FCP_ADAPTER_VOL_CHECK_POLLTIME  100
#define FCP_ADAPTER_VOL_CHECK_TIMEOUT   10

#define FCP_ACK_RETRY_CYCLE             10

#define FCP_RESTART_TIME             4
#define FCP_RETRY_TIME                  5
#define FCP_FAIL                     (-1)

#define CHG_FCP_ADAPTER_DETECT_FAIL     1
#define CHG_FCP_ADAPTER_DETECT_SUCC     0
#define CHG_FCP_ADAPTER_DETECT_OTHER    (-1)
#define CHG_FCP_POLL_TIME               100 /* 100ms */
#define CHG_FCP_DETECT_MAX_COUT         20 /* fcp detect MAX COUT */


#define CHG_FCP_ATTATCH_SHIFT           (REG_STATUIS_attach_START)
#define CHG_FCP_ATTATCH_MSK             (1 << CHG_FCP_ATTATCH_SHIFT)
#define CHG_FCP_DVC_SHIFT               (REG_STATUIS_dvc_START)
#define CHG_FCP_DVC_MSK                 (0x03 << CHG_FCP_DVC_SHIFT)
#define CHG_FCP_SLAVE_GOOD              (CHG_FCP_DVC_MSK | CHG_FCP_ATTATCH_MSK)

#define CHG_FCP_SET_STATUS_SHIFT        (0)
#define CHG_FCP_SET_STATUS_MSK          (1 << CHG_FCP_SET_STATUS_SHIFT)

#define CHG_FCP_CMDCPL                  (1 << 7)
#define CHG_FCP_ACK                     (1 << 6)
#define CHG_FCP_NACK                    (1 << 4)
#define CHG_FCP_CRCPAR                  (1 << 3)

#define CHG_FCP_CRCRX                   (1 << 4)
#define CHG_FCP_PARRX                   (1 << 3)
#define CHG_FCP_PROTSTAT                (1 << 1)

#define CHG_FCP_TAIL_HAND_FAIL          (1 << 1)
#define CHG_FCP_INIT_HAND_FAIL          (1 << 3)
#define CHG_FCP_ENABLE_HAND_FAIL        (1 << 4)

#define VBUS_VSET_5V                    5
#define VBUS_VSET_9V                    9
#define VBUS_VSET_12V                   12
#define FCP_DATA_LEN                    8

#define REG_ISR1                        (REG_ISR1_ADDR(REG_FCP_BASE))
#define REG_ISR2                        (REG_ISR2_ADDR(REG_FCP_BASE))
#define REG_CMD                         (REG_CMD_ADDR(REG_FCP_BASE))
#define REG_ADDR                        (REG_ADDR_ADDR(REG_FCP_BASE))
#define REG_CNTL                    (REG_CNTL_ADDR(REG_FCP_BASE))
#define CHG_FCP_SNDCMD_SHIFT            (REG_CNTL_sndcmd_START)
#define CHG_FCP_SNDCMD_MSK              (1 << REG_CNTL_sndcmd_START)
#define CHG_FCP_MSTR_RST_SHIFT (REG_CNTL_mstr_rst_START)
#define CHG_FCP_MSTR_RST_MSK (0x1 << CHG_FCP_MSTR_RST_SHIFT)

#define REG_FCP_IRQ3                    (REG_FCP_IRQ3_ADDR(REG_FCP_BASE))
#define REG_FCP_IRQ4                    (REG_FCP_IRQ4_ADDR(REG_FCP_BASE))

#define CHG_FCP_EN                      1

#define REG_FCP_RDATA0                  (REG_FCP_RDATA0_ADDR(REG_FCP_BASE))

#define CHG_FCP_EN_SHIFT  (REG_CNTL_enable_START)
#define CHG_FCP_EN_MSK    (0x1 << CHG_FCP_EN_SHIFT)

#define REG_STATUIS   (REG_STATUIS_ADDR(REG_FCP_BASE))

#define REG_IRQ_STATUS_4  (REG_IRQ_STATUS_4_ADDR(REG_IRQ_BASE))
#define CHG_VUSB_PLUGOUT_SHIFT (REG_IRQ_STATUS_4_irq_status_4_2_START)
#define CHG_VUSB_PLUGOUT_MSK (0x1 << CHG_VUSB_PLUGOUT_SHIFT)

#define REG_FCP_CTRL  (REG_FCP_CTRL_ADDR(REG_FCP_BASE))
#define CHG_FCP_CMP_EN_SHIFT (REG_FCP_CTRL_fcp_cmp_en_START)
#define CHG_FCP_CMP_EN_MSK (0x01 << CHG_FCP_CMP_EN_SHIFT)
#define CHG_FCP_DET_EN_SHIFT (REG_FCP_CTRL_fcp_det_en_START)
#define CHG_FCP_DET_EN_MSK (0x01 << CHG_FCP_DET_EN_SHIFT)

#define REG_DATA0 (REG_DATA0_ADDR(REG_FCP_BASE))

#define REG_FCP_ADAP_CTRL (REG_FCP_ADAP_CTRL_ADDR(REG_FCP_BASE))
#define FCP_SET_D60M_R_SHIFT (REG_FCP_IRQ5_MASK_fcp_set_d60m_r_mk_START)
#define FCP_SET_D60M_R_MSK (0 << FCP_SET_D60M_R_SHIFT)

#define REG_FCP_SOFT_RST_CTRL (REG_FCP_SOFT_RST_CTRL_ADDR(REG_FCP_BASE))

#define REG_FCP_LENGTH (REG_FCP_LENGTH_ADDR(REG_FCP_BASE))


__attribute__((weak)) int scharger_set_vbus_vset(unsigned int value)
{
	return 0;
}
__attribute__((weak)) void scharger_set_vbus_ovp(int vbus)
{}
__attribute__((weak)) int config_opt_param(int value)
{
	return 0;
}
__attribute__((weak)) int set_vbus_ovp(int value)
{
	return 0;
}

int scharger_v700_scp_read_block(int reg, int *val, int num);
