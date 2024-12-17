#ifndef __SOC_SCHARGERV700_INTERFACE_H__
#define __SOC_SCHARGERV700_INTERFACE_H__ 
#ifdef __cplusplus
    #if __cplusplus
        extern "C" {
    #endif
#endif
#define REG_PD_BASE 0x0000
#define REG_FCP_BASE 0x0100
#define REG_ADC_BASE 0x0200
#define REG_ANA_BASE 0x0900
#define REG_GLB_BASE 0x0300
#define REG_COUL_PWR_BASE 0x0400
#define REG_COUL_NOPWR0_BASE 0x0500
#define REG_COUL_NOPWR1_BASE 0x0600
#define REG_IRQ_BASE 0x0800
#ifndef __SOC_H_FOR_ASM__
#define REG_VENDIDL_ADDR(base) ((base) + (0x00UL))
#define REG_VENDIDH_ADDR(base) ((base) + (0x01UL))
#define REG_PRODIDL_ADDR(base) ((base) + (0x02UL))
#define REG_PRODIDH_ADDR(base) ((base) + (0x03UL))
#define REG_DEVIDL_ADDR(base) ((base) + (0x04UL))
#define REG_DEVIDH_ADDR(base) ((base) + (0x05UL))
#define REG_TYPECREVL_ADDR(base) ((base) + (0x06UL))
#define REG_TYPECREVH_ADDR(base) ((base) + (0x07UL))
#define REG_USBPDVER_ADDR(base) ((base) + (0x08UL))
#define REG_USBPDREV_ADDR(base) ((base) + (0x09UL))
#define REG_PDIFREVL_ADDR(base) ((base) + (0x0AUL))
#define REG_PDIFREVH_ADDR(base) ((base) + (0x0BUL))
#define REG_PD_ALERT_L_ADDR(base) ((base) + (0x10UL))
#define REG_PD_ALERT_H_ADDR(base) ((base) + (0x11UL))
#define REG_PD_ALERT_MSK_L_ADDR(base) ((base) + (0x12UL))
#define REG_PD_ALERT_MSK_H_ADDR(base) ((base) + (0x13UL))
#define REG_PD_PWRSTAT_MSK_ADDR(base) ((base) + (0x14UL))
#define REG_PD_FAULTSTAT_MSK_ADDR(base) ((base) + (0x15UL))
#define REG_EXTENDED_STATUS_MASK_ADDR(base) ((base) + (0x16UL))
#define REG_ALERT_EXTENDED_MASK_ADDR(base) ((base) + (0x17UL))
#define REG_PD_TCPC_CTRL_ADDR(base) ((base) + (0x19UL))
#define REG_PD_ROLE_CTRL_ADDR(base) ((base) + (0x1AUL))
#define REG_PD_FAULT_CTRL_ADDR(base) ((base) + (0x1BUL))
#define REG_PD_PWR_CTRL_ADDR(base) ((base) + (0x1CUL))
#define REG_PD_CC_STATUS_ADDR(base) ((base) + (0x1DUL))
#define REG_PD_PWR_STATUS_ADDR(base) ((base) + (0x1EUL))
#define REG_PD_FAULT_STATUS_ADDR(base) ((base) + (0x1FUL))
#define REG_EXTENDED_STATUS_ADDR(base) ((base) + (0x20UL))
#define REG_ALERT_EXTENDED_ADDR(base) ((base) + (0x21UL))
#define REG_PD_COMMAND_ADDR(base) ((base) + (0x23UL))
#define REG_PD_DEVCAP1L_ADDR(base) ((base) + (0x24UL))
#define REG_PD_DEVCAP1H_ADDR(base) ((base) + (0x25UL))
#define REG_PD_DEVCAP2L_ADDR(base) ((base) + (0x26UL))
#define REG_PD_DEVCAP2H_ADDR(base) ((base) + (0x27UL))
#define REG_PD_STDIN_CAP_ADDR(base) ((base) + (0x28UL))
#define REG_PD_STDOUT_CAP_ADDR(base) ((base) + (0x29UL))
#define REG_PD_MSG_HEADR_ADDR(base) ((base) + (0x2EUL))
#define REG_PD_RXDETECT_ADDR(base) ((base) + (0x2FUL))
#define REG_PD_RXBYTECNT_ADDR(base) ((base) + (0x30UL))
#define REG_PD_RXTYPE_ADDR(base) ((base) + (0x31UL))
#define REG_PD_RXHEADL_ADDR(base) ((base) + (0x32UL))
#define REG_PD_RXHEADH_ADDR(base) ((base) + (0x33UL))
#define REG_PD_RXDATA_0_ADDR(base,k) ((base) + ((k)*1+0x34UL))
#define REG_PD_TRANSMIT_ADDR(base) ((base) + (0x50UL))
#define REG_PD_TXBYTECNT_ADDR(base) ((base) + (0x51UL))
#define REG_PD_TXHEADL_ADDR(base) ((base) + (0x52UL))
#define REG_PD_TXHEADH_ADDR(base) ((base) + (0x53UL))
#define REG_PD_TXDATA_ADDR(base,k) ((base) + ((k)*1+0x54UL))
#define REG_PD_VBUS_VOL_L_ADDR(base) ((base) + (0x70UL))
#define REG_PD_VBUS_VOL_H_ADDR(base) ((base) + (0x71UL))
#define REG_PD_VBUS_SNK_DISCL_ADDR(base) ((base) + (0x72UL))
#define REG_PD_VBUS_SNK_DISCH_ADDR(base) ((base) + (0x73UL))
#define REG_PD_VBUS_STOP_DISCL_ADDR(base) ((base) + (0x74UL))
#define REG_PD_VBUS_STOP_DISCH_ADDR(base) ((base) + (0x75UL))
#define REG_PD_VALARMH_CFGL_ADDR(base) ((base) + (0x76UL))
#define REG_PD_VALARMH_CFGH_ADDR(base) ((base) + (0x77UL))
#define REG_PD_VALARML_CFGL_ADDR(base) ((base) + (0x78UL))
#define REG_PD_VALARML_CFGH_ADDR(base) ((base) + (0x79UL))
#define REG_PD_VDM_CFG_0_ADDR(base) ((base) + (0x7AUL))
#define REG_PD_VDM_ENABLE_ADDR(base) ((base) + (0x7BUL))
#define REG_PD_VDM_CFG_1_ADDR(base) ((base) + (0x7CUL))
#define REG_PD_DBG_RDATA_CFG_ADDR(base) ((base) + (0x7DUL))
#define REG_PD_DBG_RDATA_ADDR(base) ((base) + (0x7EUL))
#else
#define REG_VENDIDL_ADDR(base) ((base) + (0x00))
#define REG_VENDIDH_ADDR(base) ((base) + (0x01))
#define REG_PRODIDL_ADDR(base) ((base) + (0x02))
#define REG_PRODIDH_ADDR(base) ((base) + (0x03))
#define REG_DEVIDL_ADDR(base) ((base) + (0x04))
#define REG_DEVIDH_ADDR(base) ((base) + (0x05))
#define REG_TYPECREVL_ADDR(base) ((base) + (0x06))
#define REG_TYPECREVH_ADDR(base) ((base) + (0x07))
#define REG_USBPDVER_ADDR(base) ((base) + (0x08))
#define REG_USBPDREV_ADDR(base) ((base) + (0x09))
#define REG_PDIFREVL_ADDR(base) ((base) + (0x0A))
#define REG_PDIFREVH_ADDR(base) ((base) + (0x0B))
#define REG_PD_ALERT_L_ADDR(base) ((base) + (0x10))
#define REG_PD_ALERT_H_ADDR(base) ((base) + (0x11))
#define REG_PD_ALERT_MSK_L_ADDR(base) ((base) + (0x12))
#define REG_PD_ALERT_MSK_H_ADDR(base) ((base) + (0x13))
#define REG_PD_PWRSTAT_MSK_ADDR(base) ((base) + (0x14))
#define REG_PD_FAULTSTAT_MSK_ADDR(base) ((base) + (0x15))
#define REG_EXTENDED_STATUS_MASK_ADDR(base) ((base) + (0x16))
#define REG_ALERT_EXTENDED_MASK_ADDR(base) ((base) + (0x17))
#define REG_PD_TCPC_CTRL_ADDR(base) ((base) + (0x19))
#define REG_PD_ROLE_CTRL_ADDR(base) ((base) + (0x1A))
#define REG_PD_FAULT_CTRL_ADDR(base) ((base) + (0x1B))
#define REG_PD_PWR_CTRL_ADDR(base) ((base) + (0x1C))
#define REG_PD_CC_STATUS_ADDR(base) ((base) + (0x1D))
#define REG_PD_PWR_STATUS_ADDR(base) ((base) + (0x1E))
#define REG_PD_FAULT_STATUS_ADDR(base) ((base) + (0x1F))
#define REG_EXTENDED_STATUS_ADDR(base) ((base) + (0x20))
#define REG_ALERT_EXTENDED_ADDR(base) ((base) + (0x21))
#define REG_PD_COMMAND_ADDR(base) ((base) + (0x23))
#define REG_PD_DEVCAP1L_ADDR(base) ((base) + (0x24))
#define REG_PD_DEVCAP1H_ADDR(base) ((base) + (0x25))
#define REG_PD_DEVCAP2L_ADDR(base) ((base) + (0x26))
#define REG_PD_DEVCAP2H_ADDR(base) ((base) + (0x27))
#define REG_PD_STDIN_CAP_ADDR(base) ((base) + (0x28))
#define REG_PD_STDOUT_CAP_ADDR(base) ((base) + (0x29))
#define REG_PD_MSG_HEADR_ADDR(base) ((base) + (0x2E))
#define REG_PD_RXDETECT_ADDR(base) ((base) + (0x2F))
#define REG_PD_RXBYTECNT_ADDR(base) ((base) + (0x30))
#define REG_PD_RXTYPE_ADDR(base) ((base) + (0x31))
#define REG_PD_RXHEADL_ADDR(base) ((base) + (0x32))
#define REG_PD_RXHEADH_ADDR(base) ((base) + (0x33))
#define REG_PD_RXDATA_0_ADDR(base,k) ((base) + ((k)*1+0x34))
#define REG_PD_TRANSMIT_ADDR(base) ((base) + (0x50))
#define REG_PD_TXBYTECNT_ADDR(base) ((base) + (0x51))
#define REG_PD_TXHEADL_ADDR(base) ((base) + (0x52))
#define REG_PD_TXHEADH_ADDR(base) ((base) + (0x53))
#define REG_PD_TXDATA_ADDR(base,k) ((base) + ((k)*1+0x54))
#define REG_PD_VBUS_VOL_L_ADDR(base) ((base) + (0x70))
#define REG_PD_VBUS_VOL_H_ADDR(base) ((base) + (0x71))
#define REG_PD_VBUS_SNK_DISCL_ADDR(base) ((base) + (0x72))
#define REG_PD_VBUS_SNK_DISCH_ADDR(base) ((base) + (0x73))
#define REG_PD_VBUS_STOP_DISCL_ADDR(base) ((base) + (0x74))
#define REG_PD_VBUS_STOP_DISCH_ADDR(base) ((base) + (0x75))
#define REG_PD_VALARMH_CFGL_ADDR(base) ((base) + (0x76))
#define REG_PD_VALARMH_CFGH_ADDR(base) ((base) + (0x77))
#define REG_PD_VALARML_CFGL_ADDR(base) ((base) + (0x78))
#define REG_PD_VALARML_CFGH_ADDR(base) ((base) + (0x79))
#define REG_PD_VDM_CFG_0_ADDR(base) ((base) + (0x7A))
#define REG_PD_VDM_ENABLE_ADDR(base) ((base) + (0x7B))
#define REG_PD_VDM_CFG_1_ADDR(base) ((base) + (0x7C))
#define REG_PD_DBG_RDATA_CFG_ADDR(base) ((base) + (0x7D))
#define REG_PD_DBG_RDATA_ADDR(base) ((base) + (0x7E))
#endif
#ifndef __SOC_H_FOR_ASM__
#define REG_STATUIS_ADDR(base) ((base) + (0x00UL))
#define REG_CNTL_ADDR(base) ((base) + (0x01UL))
#define REG_CMD_ADDR(base) ((base) + (0x04UL))
#define REG_FCP_LENGTH_ADDR(base) ((base) + (0x05UL))
#define REG_ADDR_ADDR(base) ((base) + (0x07UL))
#define REG_DATA0_ADDR(base) ((base) + (0x08UL))
#define REG_DATA1_ADDR(base) ((base) + (0x09UL))
#define REG_DATA2_ADDR(base) ((base) + (0x0AUL))
#define REG_DATA3_ADDR(base) ((base) + (0x0BUL))
#define REG_DATA4_ADDR(base) ((base) + (0x0CUL))
#define REG_DATA5_ADDR(base) ((base) + (0x0DUL))
#define REG_DATA6_ADDR(base) ((base) + (0x0EUL))
#define REG_DATA7_ADDR(base) ((base) + (0x0FUL))
#define REG_FCP_RDATA0_ADDR(base) ((base) + (0x10UL))
#define REG_FCP_RDATA1_ADDR(base) ((base) + (0x11UL))
#define REG_FCP_RDATA2_ADDR(base) ((base) + (0x12UL))
#define REG_FCP_RDATA3_ADDR(base) ((base) + (0x13UL))
#define REG_FCP_RDATA4_ADDR(base) ((base) + (0x14UL))
#define REG_FCP_RDATA5_ADDR(base) ((base) + (0x15UL))
#define REG_FCP_RDATA6_ADDR(base) ((base) + (0x16UL))
#define REG_FCP_RDATA7_ADDR(base) ((base) + (0x17UL))
#define REG_ISR1_ADDR(base) ((base) + (0x19UL))
#define REG_ISR2_ADDR(base) ((base) + (0x1AUL))
#define REG_IMR1_ADDR(base) ((base) + (0x1BUL))
#define REG_IMR2_ADDR(base) ((base) + (0x1CUL))
#define REG_FCP_IRQ5_ADDR(base) ((base) + (0x1DUL))
#define REG_FCP_IRQ5_MASK_ADDR(base) ((base) + (0x1EUL))
#define REG_FCP_CTRL_ADDR(base) ((base) + (0x1FUL))
#define REG_FCP_MODE2_SET_ADDR(base) ((base) + (0x20UL))
#define REG_FCP_ADAP_CTRL_ADDR(base) ((base) + (0x21UL))
#define REG_RDATA_READY_ADDR(base) ((base) + (0x22UL))
#define REG_FCP_SOFT_RST_CTRL_ADDR(base) ((base) + (0x23UL))
#define REG_FCP_RDATA_PARITY_ERR_ADDR(base) ((base) + (0x25UL))
#define REG_FCP_INIT_RETRY_CFG_ADDR(base) ((base) + (0x26UL))
#define REG_FCP_IRQ3_ADDR(base) ((base) + (0x30UL))
#define REG_FCP_IRQ4_ADDR(base) ((base) + (0x31UL))
#define REG_FCP_IRQ3_MASK_ADDR(base) ((base) + (0x32UL))
#define REG_FCP_IRQ4_MASK_ADDR(base) ((base) + (0x33UL))
#define REG_MSTATE_ADDR(base) ((base) + (0x34UL))
#define REG_CRC_ENABLE_ADDR(base) ((base) + (0x35UL))
#define REG_CRC_START_VALUE_ADDR(base) ((base) + (0x36UL))
#define REG_SAMPLE_CNT_ADJ_ADDR(base) ((base) + (0x37UL))
#define REG_RX_PING_WIDTH_MIN_H_ADDR(base) ((base) + (0x38UL))
#define REG_RX_PING_WIDTH_MIN_L_ADDR(base) ((base) + (0x39UL))
#define REG_RX_PING_WIDTH_MAX_H_ADDR(base) ((base) + (0x3AUL))
#define REG_RX_PING_WIDTH_MAX_L_ADDR(base) ((base) + (0x3BUL))
#define REG_DATA_WAIT_TIME_ADDR(base) ((base) + (0x3CUL))
#define REG_RETRY_CNT_ADDR(base) ((base) + (0x3DUL))
#define REG_FCP_RO_RESERVE_ADDR(base) ((base) + (0x3EUL))
#define REG_FCP_DEBUG_REG0_ADDR(base) ((base) + (0x3FUL))
#define REG_FCP_DEBUG_REG1_ADDR(base) ((base) + (0x40UL))
#define REG_FCP_DEBUG_REG2_ADDR(base) ((base) + (0x41UL))
#define REG_FCP_DEBUG_REG3_ADDR(base) ((base) + (0x42UL))
#define REG_FCP_DEBUG_REG4_ADDR(base) ((base) + (0x43UL))
#define REG_RX_PACKET_WAIT_ADJUST_ADDR(base) ((base) + (0x44UL))
#define REG_SAMPLE_CNT_TINYJUST_ADDR(base) ((base) + (0x45UL))
#define REG_RX_PING_CNT_TINYJUST_ADDR(base) ((base) + (0x46UL))
#define REG_SHIFT_CNT_CFG_MAX_ADDR(base) ((base) + (0x47UL))
#define REG_PD_CDR_CFG_0_ADDR(base) ((base) + (0x50UL))
#define REG_PD_CDR_CFG_1_ADDR(base) ((base) + (0x51UL))
#define REG_PD_DBG_CFG_0_ADDR(base) ((base) + (0x52UL))
#define REG_PD_DBG_CFG_1_ADDR(base) ((base) + (0x53UL))
#define REG_PD_DBG_CFG_2_ADDR(base) ((base) + (0x54UL))
#define REG_PD_DBG_CFG_3_ADDR(base) ((base) + (0x55UL))
#define REG_PD_DBG_RO_0_ADDR(base) ((base) + (0x56UL))
#define REG_PD_DBG_RO_1_ADDR(base) ((base) + (0x57UL))
#define REG_PD_DBG_RO_2_ADDR(base) ((base) + (0x58UL))
#define REG_PD_DBG_RO_3_ADDR(base) ((base) + (0x59UL))
#define REG_PD_EXT_OTG_SEL_ADDR(base) ((base) + (0x5AUL))
#define REG_PD_EXT_OTG_EN_ADDR(base) ((base) + (0x5BUL))
#define REG_PD_EXT_OVP_SEL_ADDR(base) ((base) + (0x5CUL))
#define REG_PD_EXT_OVP_EN_ADDR(base) ((base) + (0x5DUL))
#define REG_BC12_CFG_REG_0_ADDR(base) ((base) + (0x60UL))
#define REG_BC12_CFG_REG_1_ADDR(base) ((base) + (0x61UL))
#define REG_BC12_CFG_REG_2_ADDR(base) ((base) + (0x62UL))
#define REG_BC12_STATUS_0_ADDR(base) ((base) + (0x63UL))
#else
#define REG_STATUIS_ADDR(base) ((base) + (0x00))
#define REG_CNTL_ADDR(base) ((base) + (0x01))
#define REG_CMD_ADDR(base) ((base) + (0x04))
#define REG_FCP_LENGTH_ADDR(base) ((base) + (0x05))
#define REG_ADDR_ADDR(base) ((base) + (0x07))
#define REG_DATA0_ADDR(base) ((base) + (0x08))
#define REG_DATA1_ADDR(base) ((base) + (0x09))
#define REG_DATA2_ADDR(base) ((base) + (0x0A))
#define REG_DATA3_ADDR(base) ((base) + (0x0B))
#define REG_DATA4_ADDR(base) ((base) + (0x0C))
#define REG_DATA5_ADDR(base) ((base) + (0x0D))
#define REG_DATA6_ADDR(base) ((base) + (0x0E))
#define REG_DATA7_ADDR(base) ((base) + (0x0F))
#define REG_FCP_RDATA0_ADDR(base) ((base) + (0x10))
#define REG_FCP_RDATA1_ADDR(base) ((base) + (0x11))
#define REG_FCP_RDATA2_ADDR(base) ((base) + (0x12))
#define REG_FCP_RDATA3_ADDR(base) ((base) + (0x13))
#define REG_FCP_RDATA4_ADDR(base) ((base) + (0x14))
#define REG_FCP_RDATA5_ADDR(base) ((base) + (0x15))
#define REG_FCP_RDATA6_ADDR(base) ((base) + (0x16))
#define REG_FCP_RDATA7_ADDR(base) ((base) + (0x17))
#define REG_ISR1_ADDR(base) ((base) + (0x19))
#define REG_ISR2_ADDR(base) ((base) + (0x1A))
#define REG_IMR1_ADDR(base) ((base) + (0x1B))
#define REG_IMR2_ADDR(base) ((base) + (0x1C))
#define REG_FCP_IRQ5_ADDR(base) ((base) + (0x1D))
#define REG_FCP_IRQ5_MASK_ADDR(base) ((base) + (0x1E))
#define REG_FCP_CTRL_ADDR(base) ((base) + (0x1F))
#define REG_FCP_MODE2_SET_ADDR(base) ((base) + (0x20))
#define REG_FCP_ADAP_CTRL_ADDR(base) ((base) + (0x21))
#define REG_RDATA_READY_ADDR(base) ((base) + (0x22))
#define REG_FCP_SOFT_RST_CTRL_ADDR(base) ((base) + (0x23))
#define REG_FCP_RDATA_PARITY_ERR_ADDR(base) ((base) + (0x25))
#define REG_FCP_INIT_RETRY_CFG_ADDR(base) ((base) + (0x26))
#define REG_FCP_IRQ3_ADDR(base) ((base) + (0x30))
#define REG_FCP_IRQ4_ADDR(base) ((base) + (0x31))
#define REG_FCP_IRQ3_MASK_ADDR(base) ((base) + (0x32))
#define REG_FCP_IRQ4_MASK_ADDR(base) ((base) + (0x33))
#define REG_MSTATE_ADDR(base) ((base) + (0x34))
#define REG_CRC_ENABLE_ADDR(base) ((base) + (0x35))
#define REG_CRC_START_VALUE_ADDR(base) ((base) + (0x36))
#define REG_SAMPLE_CNT_ADJ_ADDR(base) ((base) + (0x37))
#define REG_RX_PING_WIDTH_MIN_H_ADDR(base) ((base) + (0x38))
#define REG_RX_PING_WIDTH_MIN_L_ADDR(base) ((base) + (0x39))
#define REG_RX_PING_WIDTH_MAX_H_ADDR(base) ((base) + (0x3A))
#define REG_RX_PING_WIDTH_MAX_L_ADDR(base) ((base) + (0x3B))
#define REG_DATA_WAIT_TIME_ADDR(base) ((base) + (0x3C))
#define REG_RETRY_CNT_ADDR(base) ((base) + (0x3D))
#define REG_FCP_RO_RESERVE_ADDR(base) ((base) + (0x3E))
#define REG_FCP_DEBUG_REG0_ADDR(base) ((base) + (0x3F))
#define REG_FCP_DEBUG_REG1_ADDR(base) ((base) + (0x40))
#define REG_FCP_DEBUG_REG2_ADDR(base) ((base) + (0x41))
#define REG_FCP_DEBUG_REG3_ADDR(base) ((base) + (0x42))
#define REG_FCP_DEBUG_REG4_ADDR(base) ((base) + (0x43))
#define REG_RX_PACKET_WAIT_ADJUST_ADDR(base) ((base) + (0x44))
#define REG_SAMPLE_CNT_TINYJUST_ADDR(base) ((base) + (0x45))
#define REG_RX_PING_CNT_TINYJUST_ADDR(base) ((base) + (0x46))
#define REG_SHIFT_CNT_CFG_MAX_ADDR(base) ((base) + (0x47))
#define REG_PD_CDR_CFG_0_ADDR(base) ((base) + (0x50))
#define REG_PD_CDR_CFG_1_ADDR(base) ((base) + (0x51))
#define REG_PD_DBG_CFG_0_ADDR(base) ((base) + (0x52))
#define REG_PD_DBG_CFG_1_ADDR(base) ((base) + (0x53))
#define REG_PD_DBG_CFG_2_ADDR(base) ((base) + (0x54))
#define REG_PD_DBG_CFG_3_ADDR(base) ((base) + (0x55))
#define REG_PD_DBG_RO_0_ADDR(base) ((base) + (0x56))
#define REG_PD_DBG_RO_1_ADDR(base) ((base) + (0x57))
#define REG_PD_DBG_RO_2_ADDR(base) ((base) + (0x58))
#define REG_PD_DBG_RO_3_ADDR(base) ((base) + (0x59))
#define REG_PD_EXT_OTG_SEL_ADDR(base) ((base) + (0x5A))
#define REG_PD_EXT_OTG_EN_ADDR(base) ((base) + (0x5B))
#define REG_PD_EXT_OVP_SEL_ADDR(base) ((base) + (0x5C))
#define REG_PD_EXT_OVP_EN_ADDR(base) ((base) + (0x5D))
#define REG_BC12_CFG_REG_0_ADDR(base) ((base) + (0x60))
#define REG_BC12_CFG_REG_1_ADDR(base) ((base) + (0x61))
#define REG_BC12_CFG_REG_2_ADDR(base) ((base) + (0x62))
#define REG_BC12_STATUS_0_ADDR(base) ((base) + (0x63))
#endif
#ifndef __SOC_H_FOR_ASM__
#define REG_HKADC_EN_ADDR(base) ((base) + (0x00UL))
#define REG_EIS_HKADC_START_ADDR(base) ((base) + (0x01UL))
#define REG_HKADC_CTRL1_ADDR(base) ((base) + (0x02UL))
#define REG_HKADC_SEQ_CH_H_ADDR(base) ((base) + (0x03UL))
#define REG_HKADC_SEQ_CH_L_ADDR(base) ((base) + (0x04UL))
#define REG_HKADC_RD_SEQ_ADDR(base) ((base) + (0x05UL))
#define REG_HKADC_DATA_VALID_ADDR(base) ((base) + (0x06UL))
#define REG_VUSB_ADC_L_ADDR(base) ((base) + (0x07UL))
#define REG_VUSB_ADC_H_ADDR(base) ((base) + (0x08UL))
#define REG_VPSW_ADC_L_ADDR(base) ((base) + (0x09UL))
#define REG_VPSW_ADC_H_ADDR(base) ((base) + (0x0AUL))
#define REG_VBUS_ADC_L_ADDR(base) ((base) + (0x0BUL))
#define REG_VBUS_ADC_H_ADDR(base) ((base) + (0x0CUL))
#define REG_VBATL_ADC_L_ADDR(base) ((base) + (0x0DUL))
#define REG_VBATL_ADC_H_ADDR(base) ((base) + (0x0EUL))
#define REG_VBATH_ADC_L_ADDR(base) ((base) + (0x0FUL))
#define REG_VBATH_ADC_H_ADDR(base) ((base) + (0x10UL))
#define REG_IBATL_ADC_L_ADDR(base) ((base) + (0x11UL))
#define REG_IBATL_ADC_H_ADDR(base) ((base) + (0x12UL))
#define REG_IBATH_ADC_L_ADDR(base) ((base) + (0x13UL))
#define REG_IBATH_ADC_H_ADDR(base) ((base) + (0x14UL))
#define REG_IBUS_REF_ADC_L_ADDR(base) ((base) + (0x15UL))
#define REG_IBUS_REF_ADC_H_ADDR(base) ((base) + (0x16UL))
#define REG_IBUS_ADC_L_ADDR(base) ((base) + (0x17UL))
#define REG_IBUS_ADC_H_ADDR(base) ((base) + (0x18UL))
#define REG_TDIE_ADC_L_ADDR(base) ((base) + (0x19UL))
#define REG_TDIE_ADC_H_ADDR(base) ((base) + (0x1AUL))
#define REG_TSBATL_ADC_L_ADDR(base) ((base) + (0x1BUL))
#define REG_TSBATL_ADC_H_ADDR(base) ((base) + (0x1CUL))
#define REG_TSBATH_ADC_L_ADDR(base) ((base) + (0x1DUL))
#define REG_TSBATH_ADC_H_ADDR(base) ((base) + (0x1EUL))
#define REG_VEIS_ADC_L_ADDR(base) ((base) + (0x1FUL))
#define REG_VEIS_ADC_H_ADDR(base) ((base) + (0x20UL))
#define REG_VCAL_ADC_L_ADDR(base) ((base) + (0x21UL))
#define REG_VCAL_ADC_H_ADDR(base) ((base) + (0x22UL))
#define REG_VDPDN_ADC_L_ADDR(base) ((base) + (0x23UL))
#define REG_VDPDN_ADC_H_ADDR(base) ((base) + (0x24UL))
#define REG_SBU_ADC_L_ADDR(base) ((base) + (0x25UL))
#define REG_SBU_ADC_H_ADDR(base) ((base) + (0x26UL))
#define REG_HKADC_CLK_EN_ADDR(base) ((base) + (0x2CUL))
#define REG_HKADC_TB_EN_SEL_ADDR(base) ((base) + (0x2DUL))
#define REG_HKADC_TB_DATA0_ADDR(base) ((base) + (0x2EUL))
#define REG_HKADC_TB_DATA1_ADDR(base) ((base) + (0x2FUL))
#define REG_SOH_CTRL_ADDR(base) ((base) + (0x30UL))
#define REG_OTVP_DISC_EN_ADDR(base) ((base) + (0x31UL))
#define REG_OTVP_TBAT_TH0_0_ADDR(base) ((base) + (0x32UL))
#define REG_OTVP_TBAT_TH0_1_ADDR(base) ((base) + (0x33UL))
#define REG_OTVP_TBAT_TH1_0_ADDR(base) ((base) + (0x34UL))
#define REG_OTVP_TBAT_TH1_1_ADDR(base) ((base) + (0x35UL))
#define REG_OTVP_TBAT_TH2_0_ADDR(base) ((base) + (0x36UL))
#define REG_OTVP_TBAT_TH2_1_ADDR(base) ((base) + (0x37UL))
#define REG_OTVP_VBAT_TH0_0_ADDR(base) ((base) + (0x38UL))
#define REG_OTVP_VBAT_TH0_1_ADDR(base) ((base) + (0x39UL))
#define REG_OTVP_VBAT_TH1_0_ADDR(base) ((base) + (0x3AUL))
#define REG_OTVP_VBAT_TH1_1_ADDR(base) ((base) + (0x3BUL))
#define REG_OTVP_VBAT_TH2_0_ADDR(base) ((base) + (0x3CUL))
#define REG_OTVP_VBAT_TH2_1_ADDR(base) ((base) + (0x3DUL))
#define REG_SOH_OVP_STATUS_0_ADDR(base) ((base) + (0x3EUL))
#define REG_SOH_OVP_STATUS_1_ADDR(base) ((base) + (0x3FUL))
#define REG_OTP_EN_ADDR(base) ((base) + (0x41UL))
#define REG_TH_TBAT_OTP_ADDR(base) ((base) + (0x42UL))
#define REG_TH_TBAT_OTP_HYS_ADDR(base) ((base) + (0x43UL))
#define REG_EIS_EN_ADDR(base) ((base) + (0x50UL))
#define REG_ADC_EIS_SEL_ADDR(base) ((base) + (0x51UL))
#define REG_EIS_CLK_SEL_ADDR(base) ((base) + (0x52UL))
#define REG_EIS_EXT_CLK_FREQ_CFG_ADDR(base) ((base) + (0x53UL))
#define REG_EIS_WORK_START_CFG_ADDR(base) ((base) + (0x54UL))
#define REG_EIS_DET_TIME_ADDR(base) ((base) + (0x55UL))
#define REG_EIS_DET_TIME_M_0_ADDR(base) ((base) + (0x56UL))
#define REG_EIS_DET_TIME_M_1_ADDR(base) ((base) + (0x57UL))
#define REG_EIS_DET_TIME_N_ADDR(base) ((base) + (0x58UL))
#define REG_EIS_DET_TIME_JK_ADDR(base) ((base) + (0x59UL))
#define REG_EIS_TW_CFG_ADDR(base) ((base) + (0x5AUL))
#define REG_EIS_DISCHG_CFG_ADDR(base) ((base) + (0x5BUL))
#define REG_EIS_HKADC_AVERAGE_ADDR(base) ((base) + (0x5CUL))
#define REG_EIS_HTHRS_ERR_ADDR(base) ((base) + (0x5DUL))
#define REG_EIS_LTHRS_ERR_ADDR(base) ((base) + (0x5EUL))
#define REG_EIS_ICOMP_HTHRS_0_ADDR(base) ((base) + (0x5FUL))
#define REG_EIS_ICOMP_HTHRS_1_ADDR(base) ((base) + (0x60UL))
#define REG_EIS_ICOMP_LTHRS_0_ADDR(base) ((base) + (0x61UL))
#define REG_EIS_ICOMP_LTHRS_1_ADDR(base) ((base) + (0x62UL))
#define REG_EIS_INIT_CFG_ADDR(base) ((base) + (0x63UL))
#define REG_EIS_ICOMP_STATE_CLR_ADDR(base) ((base) + (0x64UL))
#define REG_EIS_MEM_CLEAR_ADDR(base) ((base) + (0x65UL))
#define REG_EIS_VBAT_OFS_CFG_0_ADDR(base) ((base) + (0x66UL))
#define REG_EIS_VBAT_OFS_CFG_1_ADDR(base) ((base) + (0x67UL))
#define REG_EIS_RO_DET_T_NUM_0_ADDR(base) ((base) + (0x68UL))
#define REG_EIS_RO_DET_T_NUM_1_ADDR(base) ((base) + (0x69UL))
#define REG_EIS_RO_I_CURR_0_ADDR(base) ((base) + (0x6AUL))
#define REG_EIS_RO_I_CURR_1_ADDR(base) ((base) + (0x6BUL))
#define REG_EIS_RO_NT_I_0_ADDR(base) ((base) + (0x6CUL))
#define REG_EIS_RO_NT_I_1_ADDR(base) ((base) + (0x6DUL))
#define REG_EIS_RO_VBAT_SAMP_NUM_ADDR(base) ((base) + (0x6EUL))
#define REG_EIS_RO_INIT_I_DAT_0_ADDR(base) ((base) + (0x6FUL))
#define REG_EIS_RO_INIT_I_DAT_1_ADDR(base) ((base) + (0x70UL))
#define REG_EIS_RO_INIT_I_OFS_0_ADDR(base) ((base) + (0x71UL))
#define REG_EIS_RO_INIT_I_OFS_1_ADDR(base) ((base) + (0x72UL))
#define REG_EIS_RO_INIT_V_ABS_0_ADDR(base) ((base) + (0x73UL))
#define REG_EIS_RO_INIT_V_ABS_1_ADDR(base) ((base) + (0x74UL))
#define REG_EIS_RO_INIT_V_OFS_0_ADDR(base) ((base) + (0x75UL))
#define REG_EIS_RO_INIT_V_OFS_1_ADDR(base) ((base) + (0x76UL))
#define REG_EIS_RO_INIT_V_RELA_0_ADDR(base) ((base) + (0x77UL))
#define REG_EIS_RO_INIT_V_RELA_1_ADDR(base) ((base) + (0x78UL))
#define REG_EIS_RO_INIT_VBAT0_0_ADDR(base) ((base) + (0x79UL))
#define REG_EIS_RO_INIT_VBAT0_1_ADDR(base) ((base) + (0x7AUL))
#define REG_EIS_RO_T_DATA_0_ADDR(base) ((base) + (0x7BUL))
#define REG_EIS_RO_T_DATA_1_ADDR(base) ((base) + (0x7CUL))
#define REG_EIS_DBG_CFG_REG_0_ADDR(base) ((base) + (0x7DUL))
#define REG_EIS_DBG_CFG_REG_1_ADDR(base) ((base) + (0x7EUL))
#define REG_EIS_DBG_CFG_REG_2_ADDR(base) ((base) + (0x7FUL))
#define REG_EIS_DET_FLAG_ADDR(base) ((base) + (0x80UL))
#define REG_EIS_INIT_END_FLAG_ADDR(base) ((base) + (0x81UL))
#define REG_EIS_TB_EN_SEL_ADDR(base) ((base) + (0x82UL))
#define REG_EIS_TB_DATA0_ADDR(base) ((base) + (0x83UL))
#define REG_EIS_TB_DATA1_ADDR(base) ((base) + (0x84UL))
#define REG_EIS_ADC_DATA_DEBUG_0_ADDR(base) ((base) + (0x85UL))
#define REG_EIS_ADC_DATA_DEBUG_1_ADDR(base) ((base) + (0x86UL))
#define REG_EIS_ADC_DATA_DEBUG_EN_ADDR(base) ((base) + (0x87UL))
#define REG_EIS_DBG_ADC_START_ADDR(base) ((base) + (0x88UL))
#else
#define REG_HKADC_EN_ADDR(base) ((base) + (0x00))
#define REG_EIS_HKADC_START_ADDR(base) ((base) + (0x01))
#define REG_HKADC_CTRL1_ADDR(base) ((base) + (0x02))
#define REG_HKADC_SEQ_CH_H_ADDR(base) ((base) + (0x03))
#define REG_HKADC_SEQ_CH_L_ADDR(base) ((base) + (0x04))
#define REG_HKADC_RD_SEQ_ADDR(base) ((base) + (0x05))
#define REG_HKADC_DATA_VALID_ADDR(base) ((base) + (0x06))
#define REG_VUSB_ADC_L_ADDR(base) ((base) + (0x07))
#define REG_VUSB_ADC_H_ADDR(base) ((base) + (0x08))
#define REG_VPSW_ADC_L_ADDR(base) ((base) + (0x09))
#define REG_VPSW_ADC_H_ADDR(base) ((base) + (0x0A))
#define REG_VBUS_ADC_L_ADDR(base) ((base) + (0x0B))
#define REG_VBUS_ADC_H_ADDR(base) ((base) + (0x0C))
#define REG_VBATL_ADC_L_ADDR(base) ((base) + (0x0D))
#define REG_VBATL_ADC_H_ADDR(base) ((base) + (0x0E))
#define REG_VBATH_ADC_L_ADDR(base) ((base) + (0x0F))
#define REG_VBATH_ADC_H_ADDR(base) ((base) + (0x10))
#define REG_IBATL_ADC_L_ADDR(base) ((base) + (0x11))
#define REG_IBATL_ADC_H_ADDR(base) ((base) + (0x12))
#define REG_IBATH_ADC_L_ADDR(base) ((base) + (0x13))
#define REG_IBATH_ADC_H_ADDR(base) ((base) + (0x14))
#define REG_IBUS_REF_ADC_L_ADDR(base) ((base) + (0x15))
#define REG_IBUS_REF_ADC_H_ADDR(base) ((base) + (0x16))
#define REG_IBUS_ADC_L_ADDR(base) ((base) + (0x17))
#define REG_IBUS_ADC_H_ADDR(base) ((base) + (0x18))
#define REG_TDIE_ADC_L_ADDR(base) ((base) + (0x19))
#define REG_TDIE_ADC_H_ADDR(base) ((base) + (0x1A))
#define REG_TSBATL_ADC_L_ADDR(base) ((base) + (0x1B))
#define REG_TSBATL_ADC_H_ADDR(base) ((base) + (0x1C))
#define REG_TSBATH_ADC_L_ADDR(base) ((base) + (0x1D))
#define REG_TSBATH_ADC_H_ADDR(base) ((base) + (0x1E))
#define REG_VEIS_ADC_L_ADDR(base) ((base) + (0x1F))
#define REG_VEIS_ADC_H_ADDR(base) ((base) + (0x20))
#define REG_VCAL_ADC_L_ADDR(base) ((base) + (0x21))
#define REG_VCAL_ADC_H_ADDR(base) ((base) + (0x22))
#define REG_VDPDN_ADC_L_ADDR(base) ((base) + (0x23))
#define REG_VDPDN_ADC_H_ADDR(base) ((base) + (0x24))
#define REG_SBU_ADC_L_ADDR(base) ((base) + (0x25))
#define REG_SBU_ADC_H_ADDR(base) ((base) + (0x26))
#define REG_HKADC_CLK_EN_ADDR(base) ((base) + (0x2C))
#define REG_HKADC_TB_EN_SEL_ADDR(base) ((base) + (0x2D))
#define REG_HKADC_TB_DATA0_ADDR(base) ((base) + (0x2E))
#define REG_HKADC_TB_DATA1_ADDR(base) ((base) + (0x2F))
#define REG_SOH_CTRL_ADDR(base) ((base) + (0x30))
#define REG_OTVP_DISC_EN_ADDR(base) ((base) + (0x31))
#define REG_OTVP_TBAT_TH0_0_ADDR(base) ((base) + (0x32))
#define REG_OTVP_TBAT_TH0_1_ADDR(base) ((base) + (0x33))
#define REG_OTVP_TBAT_TH1_0_ADDR(base) ((base) + (0x34))
#define REG_OTVP_TBAT_TH1_1_ADDR(base) ((base) + (0x35))
#define REG_OTVP_TBAT_TH2_0_ADDR(base) ((base) + (0x36))
#define REG_OTVP_TBAT_TH2_1_ADDR(base) ((base) + (0x37))
#define REG_OTVP_VBAT_TH0_0_ADDR(base) ((base) + (0x38))
#define REG_OTVP_VBAT_TH0_1_ADDR(base) ((base) + (0x39))
#define REG_OTVP_VBAT_TH1_0_ADDR(base) ((base) + (0x3A))
#define REG_OTVP_VBAT_TH1_1_ADDR(base) ((base) + (0x3B))
#define REG_OTVP_VBAT_TH2_0_ADDR(base) ((base) + (0x3C))
#define REG_OTVP_VBAT_TH2_1_ADDR(base) ((base) + (0x3D))
#define REG_SOH_OVP_STATUS_0_ADDR(base) ((base) + (0x3E))
#define REG_SOH_OVP_STATUS_1_ADDR(base) ((base) + (0x3F))
#define REG_OTP_EN_ADDR(base) ((base) + (0x41))
#define REG_TH_TBAT_OTP_ADDR(base) ((base) + (0x42))
#define REG_TH_TBAT_OTP_HYS_ADDR(base) ((base) + (0x43))
#define REG_EIS_EN_ADDR(base) ((base) + (0x50))
#define REG_ADC_EIS_SEL_ADDR(base) ((base) + (0x51))
#define REG_EIS_CLK_SEL_ADDR(base) ((base) + (0x52))
#define REG_EIS_EXT_CLK_FREQ_CFG_ADDR(base) ((base) + (0x53))
#define REG_EIS_WORK_START_CFG_ADDR(base) ((base) + (0x54))
#define REG_EIS_DET_TIME_ADDR(base) ((base) + (0x55))
#define REG_EIS_DET_TIME_M_0_ADDR(base) ((base) + (0x56))
#define REG_EIS_DET_TIME_M_1_ADDR(base) ((base) + (0x57))
#define REG_EIS_DET_TIME_N_ADDR(base) ((base) + (0x58))
#define REG_EIS_DET_TIME_JK_ADDR(base) ((base) + (0x59))
#define REG_EIS_TW_CFG_ADDR(base) ((base) + (0x5A))
#define REG_EIS_DISCHG_CFG_ADDR(base) ((base) + (0x5B))
#define REG_EIS_HKADC_AVERAGE_ADDR(base) ((base) + (0x5C))
#define REG_EIS_HTHRS_ERR_ADDR(base) ((base) + (0x5D))
#define REG_EIS_LTHRS_ERR_ADDR(base) ((base) + (0x5E))
#define REG_EIS_ICOMP_HTHRS_0_ADDR(base) ((base) + (0x5F))
#define REG_EIS_ICOMP_HTHRS_1_ADDR(base) ((base) + (0x60))
#define REG_EIS_ICOMP_LTHRS_0_ADDR(base) ((base) + (0x61))
#define REG_EIS_ICOMP_LTHRS_1_ADDR(base) ((base) + (0x62))
#define REG_EIS_INIT_CFG_ADDR(base) ((base) + (0x63))
#define REG_EIS_ICOMP_STATE_CLR_ADDR(base) ((base) + (0x64))
#define REG_EIS_MEM_CLEAR_ADDR(base) ((base) + (0x65))
#define REG_EIS_VBAT_OFS_CFG_0_ADDR(base) ((base) + (0x66))
#define REG_EIS_VBAT_OFS_CFG_1_ADDR(base) ((base) + (0x67))
#define REG_EIS_RO_DET_T_NUM_0_ADDR(base) ((base) + (0x68))
#define REG_EIS_RO_DET_T_NUM_1_ADDR(base) ((base) + (0x69))
#define REG_EIS_RO_I_CURR_0_ADDR(base) ((base) + (0x6A))
#define REG_EIS_RO_I_CURR_1_ADDR(base) ((base) + (0x6B))
#define REG_EIS_RO_NT_I_0_ADDR(base) ((base) + (0x6C))
#define REG_EIS_RO_NT_I_1_ADDR(base) ((base) + (0x6D))
#define REG_EIS_RO_VBAT_SAMP_NUM_ADDR(base) ((base) + (0x6E))
#define REG_EIS_RO_INIT_I_DAT_0_ADDR(base) ((base) + (0x6F))
#define REG_EIS_RO_INIT_I_DAT_1_ADDR(base) ((base) + (0x70))
#define REG_EIS_RO_INIT_I_OFS_0_ADDR(base) ((base) + (0x71))
#define REG_EIS_RO_INIT_I_OFS_1_ADDR(base) ((base) + (0x72))
#define REG_EIS_RO_INIT_V_ABS_0_ADDR(base) ((base) + (0x73))
#define REG_EIS_RO_INIT_V_ABS_1_ADDR(base) ((base) + (0x74))
#define REG_EIS_RO_INIT_V_OFS_0_ADDR(base) ((base) + (0x75))
#define REG_EIS_RO_INIT_V_OFS_1_ADDR(base) ((base) + (0x76))
#define REG_EIS_RO_INIT_V_RELA_0_ADDR(base) ((base) + (0x77))
#define REG_EIS_RO_INIT_V_RELA_1_ADDR(base) ((base) + (0x78))
#define REG_EIS_RO_INIT_VBAT0_0_ADDR(base) ((base) + (0x79))
#define REG_EIS_RO_INIT_VBAT0_1_ADDR(base) ((base) + (0x7A))
#define REG_EIS_RO_T_DATA_0_ADDR(base) ((base) + (0x7B))
#define REG_EIS_RO_T_DATA_1_ADDR(base) ((base) + (0x7C))
#define REG_EIS_DBG_CFG_REG_0_ADDR(base) ((base) + (0x7D))
#define REG_EIS_DBG_CFG_REG_1_ADDR(base) ((base) + (0x7E))
#define REG_EIS_DBG_CFG_REG_2_ADDR(base) ((base) + (0x7F))
#define REG_EIS_DET_FLAG_ADDR(base) ((base) + (0x80))
#define REG_EIS_INIT_END_FLAG_ADDR(base) ((base) + (0x81))
#define REG_EIS_TB_EN_SEL_ADDR(base) ((base) + (0x82))
#define REG_EIS_TB_DATA0_ADDR(base) ((base) + (0x83))
#define REG_EIS_TB_DATA1_ADDR(base) ((base) + (0x84))
#define REG_EIS_ADC_DATA_DEBUG_0_ADDR(base) ((base) + (0x85))
#define REG_EIS_ADC_DATA_DEBUG_1_ADDR(base) ((base) + (0x86))
#define REG_EIS_ADC_DATA_DEBUG_EN_ADDR(base) ((base) + (0x87))
#define REG_EIS_DBG_ADC_START_ADDR(base) ((base) + (0x88))
#endif
#ifndef __SOC_H_FOR_ASM__
#define REG_CHARGER_BATFET_H_CTRL_ADDR(base) ((base) + (0x00UL))
#define REG_CHARGER_BATFET_L_CTRL_ADDR(base) ((base) + (0x01UL))
#define REG_SHIP_MODE_ENB_ADDR(base) ((base) + (0x02UL))
#define REG_VBAT_LV_H_REG_ADDR(base) ((base) + (0x03UL))
#define REG_VBAT_LV_L_REG_ADDR(base) ((base) + (0x04UL))
#define REG_HBAT_RES_MODE_ADDR(base) ((base) + (0x05UL))
#define REG_LBAT_RES_MODE_ADDR(base) ((base) + (0x06UL))
#define REG_VDM_CFG_REG_0_ADDR(base) ((base) + (0x07UL))
#define REG_VDM_CFG_REG_1_ADDR(base) ((base) + (0x08UL))
#define REG_VDM_CFG_REG_2_ADDR(base) ((base) + (0x09UL))
#define REG_VDM_CFG_REG_3_ADDR(base) ((base) + (0x0AUL))
#define REG_VDM_CFG_REG_4_ADDR(base) ((base) + (0x0BUL))
#define REG_VDM_CFG_REG_5_ADDR(base) ((base) + (0x0CUL))
#define REG_VDM_CFG_RSV_0_ADDR(base) ((base) + (0x0DUL))
#define REG_VDM_CFG_RSV_1_ADDR(base) ((base) + (0x0EUL))
#define REG_VDM5_CFG_EN_ADDR(base) ((base) + (0x0FUL))
#define REG_BATFET_OCP_FLAG_ADDR(base) ((base) + (0x1AUL))
#define REG_DA_BUCK_REVERSBST_MODE_ADDR(base) ((base) + (0x1BUL))
#define REG_SC_BUCK_REVERSBST_CLEAR_ADDR(base) ((base) + (0x1CUL))
#define REG_JTAG_CFG_REG_0_ADDR(base) ((base) + (0x1DUL))
#define REG_COUL_CHANNEL_SEL_ADDR(base) ((base) + (0x1EUL))
#define REG_IBAT_RES_SEL_ADDR(base) ((base) + (0x1FUL))
#define REG_BUCK_OTG_CFG_REG_0_ADDR(base) ((base) + (0x20UL))
#define REG_BUCK_OTG_CFG_REG_1_ADDR(base) ((base) + (0x21UL))
#define REG_BUCK_OTG_CFG_REG_2_ADDR(base) ((base) + (0x22UL))
#define REG_BUCK_OTG_CFG_REG_3_ADDR(base) ((base) + (0x23UL))
#define REG_BUCK_OTG_CFG_REG_4_ADDR(base) ((base) + (0x24UL))
#define REG_BUCK_OTG_CFG_REG_5_ADDR(base) ((base) + (0x25UL))
#define REG_BUCK_OTG_CFG_REG_6_ADDR(base) ((base) + (0x26UL))
#define REG_BUCK_OTG_CFG_REG_7_ADDR(base) ((base) + (0x27UL))
#define REG_BUCK_OTG_CFG_REG_8_ADDR(base) ((base) + (0x28UL))
#define REG_BUCK_OTG_CFG_REG_9_ADDR(base) ((base) + (0x29UL))
#define REG_BUCK_OTG_CFG_REG_10_ADDR(base) ((base) + (0x2AUL))
#define REG_BUCK_OTG_CFG_REG_11_ADDR(base) ((base) + (0x2BUL))
#define REG_BUCK_OTG_CFG_REG_12_ADDR(base) ((base) + (0x2CUL))
#define REG_BUCK_OTG_CFG_REG_13_ADDR(base) ((base) + (0x2DUL))
#define REG_BUCK_OTG_CFG_REG_14_ADDR(base) ((base) + (0x2EUL))
#define REG_BUCK_OTG_CFG_REG_15_ADDR(base) ((base) + (0x2FUL))
#define REG_BUCK_OTG_CFG_REG_16_ADDR(base) ((base) + (0x30UL))
#define REG_BUCK_OTG_CFG_REG_17_ADDR(base) ((base) + (0x31UL))
#define REG_BUCK_OTG_CFG_REG_18_ADDR(base) ((base) + (0x32UL))
#define REG_BUCK_OTG_CFG_REG_19_ADDR(base) ((base) + (0x33UL))
#define REG_BUCK_OTG_CFG_REG_20_ADDR(base) ((base) + (0x34UL))
#define REG_BUCK_OTG_CFG_REG_21_ADDR(base) ((base) + (0x35UL))
#define REG_BUCK_OTG_CFG_REG_22_ADDR(base) ((base) + (0x36UL))
#define REG_BUCK_OTG_CFG_REG_23_ADDR(base) ((base) + (0x37UL))
#define REG_BUCK_OTG_CFG_REG_24_ADDR(base) ((base) + (0x38UL))
#define REG_BUCK_OTG_CFG_REG_25_ADDR(base) ((base) + (0x39UL))
#define REG_BUCK_OTG_CFG_REG_26_ADDR(base) ((base) + (0x3AUL))
#define REG_BUCK_OTG_CFG_REG_27_ADDR(base) ((base) + (0x3BUL))
#define REG_BUCK_OTG_RO_REG_28_ADDR(base) ((base) + (0x3CUL))
#define REG_BUCK_OTG_RO_REG_29_ADDR(base) ((base) + (0x3DUL))
#define REG_BUCK_OTG_RO_REG_30_ADDR(base) ((base) + (0x3EUL))
#define REG_BUCK_OTG_RO_REG_31_ADDR(base) ((base) + (0x3FUL))
#define REG_BUCK_OTG_RO_REG_32_ADDR(base) ((base) + (0x40UL))
#define REG_BUCK_OTG_RO_REG_33_ADDR(base) ((base) + (0x41UL))
#define REG_BUCK_OTG_CFG_REG_34_ADDR(base) ((base) + (0x42UL))
#define REG_CHG_TOP_CFG_REG_0_ADDR(base) ((base) + (0x44UL))
#define REG_CHG_TOP_CFG_REG_1_ADDR(base) ((base) + (0x45UL))
#define REG_CHG_TOP_CFG_REG_2_ADDR(base) ((base) + (0x46UL))
#define REG_CHG_TOP_CFG_REG_3_ADDR(base) ((base) + (0x47UL))
#define REG_CHG_TOP_CFG_REG_4_ADDR(base) ((base) + (0x48UL))
#define REG_CHG_TOP_CFG_REG_5_ADDR(base) ((base) + (0x49UL))
#define REG_CHG_TOP_CFG_REG_6_ADDR(base) ((base) + (0x4AUL))
#define REG_CHG_TOP_CFG_REG_7_ADDR(base) ((base) + (0x4BUL))
#define REG_CHG_TOP_CFG_REG_8_ADDR(base) ((base) + (0x4CUL))
#define REG_CHG_TOP_CFG_REG_9_ADDR(base) ((base) + (0x4DUL))
#define REG_CHG_TOP_CFG_REG_10_ADDR(base) ((base) + (0x4EUL))
#define REG_CHG_TOP_CFG_REG_11_ADDR(base) ((base) + (0x4FUL))
#define REG_CHG_TOP_CFG_REG_12_ADDR(base) ((base) + (0x50UL))
#define REG_CHG_TOP_CFG_REG_13_ADDR(base) ((base) + (0x51UL))
#define REG_CHG_TOP_CFG_REG_14_ADDR(base) ((base) + (0x52UL))
#define REG_CHG_TOP_CFG_REG_15_ADDR(base) ((base) + (0x53UL))
#define REG_CHG_TOP_CFG_REG_16_ADDR(base) ((base) + (0x54UL))
#define REG_CHG_TOP_CFG_REG_17_ADDR(base) ((base) + (0x55UL))
#define REG_CHG_TOP_CFG_REG_18_ADDR(base) ((base) + (0x56UL))
#define REG_CHG_TOP_CFG_REG_19_ADDR(base) ((base) + (0x57UL))
#define REG_CHG_TOP_CFG_REG_20_ADDR(base) ((base) + (0x58UL))
#define REG_CHG_TOP_CFG_REG_21_ADDR(base) ((base) + (0x59UL))
#define REG_CHG_TOP_CFG_REG_22_ADDR(base) ((base) + (0x5AUL))
#define REG_CHG_TOP_CFG_REG_23_ADDR(base) ((base) + (0x5BUL))
#define REG_CHG_TOP_RO_REG_24_ADDR(base) ((base) + (0x5CUL))
#define REG_CHG_TOP_RO_REG_25_ADDR(base) ((base) + (0x5DUL))
#define REG_CHG_TOP_RO_REG_26_ADDR(base) ((base) + (0x5EUL))
#define REG_CHG_TOP_RO_REG_27_ADDR(base) ((base) + (0x5FUL))
#define REG_DET_TOP_CFG_REG_0_ADDR(base) ((base) + (0x60UL))
#define REG_DET_TOP_CFG_REG_1_ADDR(base) ((base) + (0x61UL))
#define REG_DET_TOP_CFG_REG_2_ADDR(base) ((base) + (0x62UL))
#define REG_DET_TOP_CFG_REG_3_ADDR(base) ((base) + (0x63UL))
#define REG_DET_TOP_CFG_REG_4_ADDR(base) ((base) + (0x64UL))
#define REG_DET_TOP_CFG_REG_5_ADDR(base) ((base) + (0x65UL))
#define REG_DET_TOP_CFG_REG_6_ADDR(base) ((base) + (0x66UL))
#define REG_EIS_TOP_CFG_REG_0_ADDR(base) ((base) + (0x70UL))
#define REG_EIS_TOP_CFG_REG_1_ADDR(base) ((base) + (0x71UL))
#define REG_EIS_TOP_CFG_REG_2_ADDR(base) ((base) + (0x72UL))
#define REG_EIS_TOP_CFG_REG_3_ADDR(base) ((base) + (0x73UL))
#define REG_EIS_TOP_CFG_REG_4_ADDR(base) ((base) + (0x74UL))
#define REG_EIS_TOP_CFG_REG_5_ADDR(base) ((base) + (0x75UL))
#define REG_EIS_TOP_CFG_REG_6_ADDR(base) ((base) + (0x76UL))
#define REG_EIS_TOP_CFG_REG_7_ADDR(base) ((base) + (0x77UL))
#define REG_EIS_TOP_CFG_REG_8_ADDR(base) ((base) + (0x78UL))
#define REG_EIS_TOP_CFG_REG_9_ADDR(base) ((base) + (0x79UL))
#define REG_EIS_TOP_CFG_REG_10_ADDR(base) ((base) + (0x7AUL))
#define REG_HKADC_CFG_REG_0_ADDR(base) ((base) + (0x7BUL))
#define REG_HKADC_CFG_REG_1_ADDR(base) ((base) + (0x7CUL))
#define REG_REF_TOP_CFG_REG_0_ADDR(base) ((base) + (0x80UL))
#define REG_REF_TOP_CFG_REG_1_ADDR(base) ((base) + (0x81UL))
#define REG_REF_TOP_CFG_REG_2_ADDR(base) ((base) + (0x82UL))
#define REG_REF_TOP_CFG_REG_3_ADDR(base) ((base) + (0x83UL))
#define REG_REF_TOP_CFG_REG_4_ADDR(base) ((base) + (0x84UL))
#define REG_PSEL_TOP_CFG_REG_0_ADDR(base) ((base) + (0x85UL))
#define REG_PSEL_TOP_CFG_REG_1_ADDR(base) ((base) + (0x86UL))
#define REG_PSEL_TOP_CFG_REG_2_ADDR(base) ((base) + (0x87UL))
#define REG_PSEL_TOP_CFG_REG_3_ADDR(base) ((base) + (0x88UL))
#define REG_PSEL_TOP_CFG_REG_4_ADDR(base) ((base) + (0x89UL))
#define REG_PSEL_TOP_RO_REG_0_ADDR(base) ((base) + (0x8AUL))
#define REG_USB_OVP_CFG_REG_0_ADDR(base) ((base) + (0x90UL))
#define REG_USB_OVP_CFG_REG_1_ADDR(base) ((base) + (0x91UL))
#define REG_USB_OVP_CFG_REG_2_ADDR(base) ((base) + (0x92UL))
#define REG_USB_OVP_CFG_REG_3_ADDR(base) ((base) + (0x93UL))
#define REG_USB_OVP_RO_REG_4_ADDR(base) ((base) + (0x94UL))
#define REG_PSW_OVP_CFG_REG_0_ADDR(base) ((base) + (0x95UL))
#define REG_PSW_OVP_CFG_REG_1_ADDR(base) ((base) + (0x96UL))
#define REG_PSW_OVP_CFG_REG_2_ADDR(base) ((base) + (0x97UL))
#define REG_PSW_OVP_CFG_REG_3_ADDR(base) ((base) + (0x98UL))
#define REG_SCHG_LOGIC_CFG_REG_0_ADDR(base) ((base) + (0xA0UL))
#define REG_SCHG_LOGIC_CFG_REG_1_ADDR(base) ((base) + (0xA1UL))
#define REG_SCHG_LOGIC_CFG_REG_2_ADDR(base) ((base) + (0xA2UL))
#define REG_SCHG_LOGIC_CFG_REG_3_ADDR(base) ((base) + (0xA3UL))
#define REG_SCHG_LOGIC_CFG_REG_4_ADDR(base) ((base) + (0xA4UL))
#define REG_SCHG_LOGIC_CFG_REG_5_ADDR(base) ((base) + (0xA5UL))
#define REG_SCHG_LOGIC_CFG_REG_6_ADDR(base) ((base) + (0xA6UL))
#define REG_SCHG_LOGIC_CFG_REG_7_ADDR(base) ((base) + (0xA7UL))
#define REG_SCHG_LOGIC_CFG_REG_8_ADDR(base) ((base) + (0xA8UL))
#define REG_SCHG_LOGIC_RO_REG_0_ADDR(base) ((base) + (0xA9UL))
#define REG_SCHG_LOGIC_RO_REG_1_ADDR(base) ((base) + (0xAAUL))
#define REG_TCPC_CFG_REG_0_ADDR(base) ((base) + (0xB0UL))
#define REG_TCPC_CFG_REG_1_ADDR(base) ((base) + (0xB1UL))
#define REG_TCPC_CFG_REG_2_ADDR(base) ((base) + (0xB2UL))
#define REG_TCPC_CFG_REG_3_ADDR(base) ((base) + (0xB3UL))
#define REG_TYPE_CFG_REG_4_ADDR(base) ((base) + (0xB4UL))
#define REG_TCPC_RO_REG_1_ADDR(base) ((base) + (0xB5UL))
#define REG_OSC_TOP_CFG_REG_0_ADDR(base) ((base) + (0xB8UL))
#define REG_OSC_TOP_CFG_REG_1_ADDR(base) ((base) + (0xB9UL))
#define REG_OSC_TOP_CFG_REG_2_ADDR(base) ((base) + (0xBAUL))
#else
#define REG_CHARGER_BATFET_H_CTRL_ADDR(base) ((base) + (0x00))
#define REG_CHARGER_BATFET_L_CTRL_ADDR(base) ((base) + (0x01))
#define REG_SHIP_MODE_ENB_ADDR(base) ((base) + (0x02))
#define REG_VBAT_LV_H_REG_ADDR(base) ((base) + (0x03))
#define REG_VBAT_LV_L_REG_ADDR(base) ((base) + (0x04))
#define REG_HBAT_RES_MODE_ADDR(base) ((base) + (0x05))
#define REG_LBAT_RES_MODE_ADDR(base) ((base) + (0x06))
#define REG_VDM_CFG_REG_0_ADDR(base) ((base) + (0x07))
#define REG_VDM_CFG_REG_1_ADDR(base) ((base) + (0x08))
#define REG_VDM_CFG_REG_2_ADDR(base) ((base) + (0x09))
#define REG_VDM_CFG_REG_3_ADDR(base) ((base) + (0x0A))
#define REG_VDM_CFG_REG_4_ADDR(base) ((base) + (0x0B))
#define REG_VDM_CFG_REG_5_ADDR(base) ((base) + (0x0C))
#define REG_VDM_CFG_RSV_0_ADDR(base) ((base) + (0x0D))
#define REG_VDM_CFG_RSV_1_ADDR(base) ((base) + (0x0E))
#define REG_VDM5_CFG_EN_ADDR(base) ((base) + (0x0F))
#define REG_BATFET_OCP_FLAG_ADDR(base) ((base) + (0x1A))
#define REG_DA_BUCK_REVERSBST_MODE_ADDR(base) ((base) + (0x1B))
#define REG_SC_BUCK_REVERSBST_CLEAR_ADDR(base) ((base) + (0x1C))
#define REG_JTAG_CFG_REG_0_ADDR(base) ((base) + (0x1D))
#define REG_COUL_CHANNEL_SEL_ADDR(base) ((base) + (0x1E))
#define REG_IBAT_RES_SEL_ADDR(base) ((base) + (0x1F))
#define REG_BUCK_OTG_CFG_REG_0_ADDR(base) ((base) + (0x20))
#define REG_BUCK_OTG_CFG_REG_1_ADDR(base) ((base) + (0x21))
#define REG_BUCK_OTG_CFG_REG_2_ADDR(base) ((base) + (0x22))
#define REG_BUCK_OTG_CFG_REG_3_ADDR(base) ((base) + (0x23))
#define REG_BUCK_OTG_CFG_REG_4_ADDR(base) ((base) + (0x24))
#define REG_BUCK_OTG_CFG_REG_5_ADDR(base) ((base) + (0x25))
#define REG_BUCK_OTG_CFG_REG_6_ADDR(base) ((base) + (0x26))
#define REG_BUCK_OTG_CFG_REG_7_ADDR(base) ((base) + (0x27))
#define REG_BUCK_OTG_CFG_REG_8_ADDR(base) ((base) + (0x28))
#define REG_BUCK_OTG_CFG_REG_9_ADDR(base) ((base) + (0x29))
#define REG_BUCK_OTG_CFG_REG_10_ADDR(base) ((base) + (0x2A))
#define REG_BUCK_OTG_CFG_REG_11_ADDR(base) ((base) + (0x2B))
#define REG_BUCK_OTG_CFG_REG_12_ADDR(base) ((base) + (0x2C))
#define REG_BUCK_OTG_CFG_REG_13_ADDR(base) ((base) + (0x2D))
#define REG_BUCK_OTG_CFG_REG_14_ADDR(base) ((base) + (0x2E))
#define REG_BUCK_OTG_CFG_REG_15_ADDR(base) ((base) + (0x2F))
#define REG_BUCK_OTG_CFG_REG_16_ADDR(base) ((base) + (0x30))
#define REG_BUCK_OTG_CFG_REG_17_ADDR(base) ((base) + (0x31))
#define REG_BUCK_OTG_CFG_REG_18_ADDR(base) ((base) + (0x32))
#define REG_BUCK_OTG_CFG_REG_19_ADDR(base) ((base) + (0x33))
#define REG_BUCK_OTG_CFG_REG_20_ADDR(base) ((base) + (0x34))
#define REG_BUCK_OTG_CFG_REG_21_ADDR(base) ((base) + (0x35))
#define REG_BUCK_OTG_CFG_REG_22_ADDR(base) ((base) + (0x36))
#define REG_BUCK_OTG_CFG_REG_23_ADDR(base) ((base) + (0x37))
#define REG_BUCK_OTG_CFG_REG_24_ADDR(base) ((base) + (0x38))
#define REG_BUCK_OTG_CFG_REG_25_ADDR(base) ((base) + (0x39))
#define REG_BUCK_OTG_CFG_REG_26_ADDR(base) ((base) + (0x3A))
#define REG_BUCK_OTG_CFG_REG_27_ADDR(base) ((base) + (0x3B))
#define REG_BUCK_OTG_RO_REG_28_ADDR(base) ((base) + (0x3C))
#define REG_BUCK_OTG_RO_REG_29_ADDR(base) ((base) + (0x3D))
#define REG_BUCK_OTG_RO_REG_30_ADDR(base) ((base) + (0x3E))
#define REG_BUCK_OTG_RO_REG_31_ADDR(base) ((base) + (0x3F))
#define REG_BUCK_OTG_RO_REG_32_ADDR(base) ((base) + (0x40))
#define REG_BUCK_OTG_RO_REG_33_ADDR(base) ((base) + (0x41))
#define REG_BUCK_OTG_CFG_REG_34_ADDR(base) ((base) + (0x42))
#define REG_CHG_TOP_CFG_REG_0_ADDR(base) ((base) + (0x44))
#define REG_CHG_TOP_CFG_REG_1_ADDR(base) ((base) + (0x45))
#define REG_CHG_TOP_CFG_REG_2_ADDR(base) ((base) + (0x46))
#define REG_CHG_TOP_CFG_REG_3_ADDR(base) ((base) + (0x47))
#define REG_CHG_TOP_CFG_REG_4_ADDR(base) ((base) + (0x48))
#define REG_CHG_TOP_CFG_REG_5_ADDR(base) ((base) + (0x49))
#define REG_CHG_TOP_CFG_REG_6_ADDR(base) ((base) + (0x4A))
#define REG_CHG_TOP_CFG_REG_7_ADDR(base) ((base) + (0x4B))
#define REG_CHG_TOP_CFG_REG_8_ADDR(base) ((base) + (0x4C))
#define REG_CHG_TOP_CFG_REG_9_ADDR(base) ((base) + (0x4D))
#define REG_CHG_TOP_CFG_REG_10_ADDR(base) ((base) + (0x4E))
#define REG_CHG_TOP_CFG_REG_11_ADDR(base) ((base) + (0x4F))
#define REG_CHG_TOP_CFG_REG_12_ADDR(base) ((base) + (0x50))
#define REG_CHG_TOP_CFG_REG_13_ADDR(base) ((base) + (0x51))
#define REG_CHG_TOP_CFG_REG_14_ADDR(base) ((base) + (0x52))
#define REG_CHG_TOP_CFG_REG_15_ADDR(base) ((base) + (0x53))
#define REG_CHG_TOP_CFG_REG_16_ADDR(base) ((base) + (0x54))
#define REG_CHG_TOP_CFG_REG_17_ADDR(base) ((base) + (0x55))
#define REG_CHG_TOP_CFG_REG_18_ADDR(base) ((base) + (0x56))
#define REG_CHG_TOP_CFG_REG_19_ADDR(base) ((base) + (0x57))
#define REG_CHG_TOP_CFG_REG_20_ADDR(base) ((base) + (0x58))
#define REG_CHG_TOP_CFG_REG_21_ADDR(base) ((base) + (0x59))
#define REG_CHG_TOP_CFG_REG_22_ADDR(base) ((base) + (0x5A))
#define REG_CHG_TOP_CFG_REG_23_ADDR(base) ((base) + (0x5B))
#define REG_CHG_TOP_RO_REG_24_ADDR(base) ((base) + (0x5C))
#define REG_CHG_TOP_RO_REG_25_ADDR(base) ((base) + (0x5D))
#define REG_CHG_TOP_RO_REG_26_ADDR(base) ((base) + (0x5E))
#define REG_CHG_TOP_RO_REG_27_ADDR(base) ((base) + (0x5F))
#define REG_DET_TOP_CFG_REG_0_ADDR(base) ((base) + (0x60))
#define REG_DET_TOP_CFG_REG_1_ADDR(base) ((base) + (0x61))
#define REG_DET_TOP_CFG_REG_2_ADDR(base) ((base) + (0x62))
#define REG_DET_TOP_CFG_REG_3_ADDR(base) ((base) + (0x63))
#define REG_DET_TOP_CFG_REG_4_ADDR(base) ((base) + (0x64))
#define REG_DET_TOP_CFG_REG_5_ADDR(base) ((base) + (0x65))
#define REG_DET_TOP_CFG_REG_6_ADDR(base) ((base) + (0x66))
#define REG_EIS_TOP_CFG_REG_0_ADDR(base) ((base) + (0x70))
#define REG_EIS_TOP_CFG_REG_1_ADDR(base) ((base) + (0x71))
#define REG_EIS_TOP_CFG_REG_2_ADDR(base) ((base) + (0x72))
#define REG_EIS_TOP_CFG_REG_3_ADDR(base) ((base) + (0x73))
#define REG_EIS_TOP_CFG_REG_4_ADDR(base) ((base) + (0x74))
#define REG_EIS_TOP_CFG_REG_5_ADDR(base) ((base) + (0x75))
#define REG_EIS_TOP_CFG_REG_6_ADDR(base) ((base) + (0x76))
#define REG_EIS_TOP_CFG_REG_7_ADDR(base) ((base) + (0x77))
#define REG_EIS_TOP_CFG_REG_8_ADDR(base) ((base) + (0x78))
#define REG_EIS_TOP_CFG_REG_9_ADDR(base) ((base) + (0x79))
#define REG_EIS_TOP_CFG_REG_10_ADDR(base) ((base) + (0x7A))
#define REG_HKADC_CFG_REG_0_ADDR(base) ((base) + (0x7B))
#define REG_HKADC_CFG_REG_1_ADDR(base) ((base) + (0x7C))
#define REG_REF_TOP_CFG_REG_0_ADDR(base) ((base) + (0x80))
#define REG_REF_TOP_CFG_REG_1_ADDR(base) ((base) + (0x81))
#define REG_REF_TOP_CFG_REG_2_ADDR(base) ((base) + (0x82))
#define REG_REF_TOP_CFG_REG_3_ADDR(base) ((base) + (0x83))
#define REG_REF_TOP_CFG_REG_4_ADDR(base) ((base) + (0x84))
#define REG_PSEL_TOP_CFG_REG_0_ADDR(base) ((base) + (0x85))
#define REG_PSEL_TOP_CFG_REG_1_ADDR(base) ((base) + (0x86))
#define REG_PSEL_TOP_CFG_REG_2_ADDR(base) ((base) + (0x87))
#define REG_PSEL_TOP_CFG_REG_3_ADDR(base) ((base) + (0x88))
#define REG_PSEL_TOP_CFG_REG_4_ADDR(base) ((base) + (0x89))
#define REG_PSEL_TOP_RO_REG_0_ADDR(base) ((base) + (0x8A))
#define REG_USB_OVP_CFG_REG_0_ADDR(base) ((base) + (0x90))
#define REG_USB_OVP_CFG_REG_1_ADDR(base) ((base) + (0x91))
#define REG_USB_OVP_CFG_REG_2_ADDR(base) ((base) + (0x92))
#define REG_USB_OVP_CFG_REG_3_ADDR(base) ((base) + (0x93))
#define REG_USB_OVP_RO_REG_4_ADDR(base) ((base) + (0x94))
#define REG_PSW_OVP_CFG_REG_0_ADDR(base) ((base) + (0x95))
#define REG_PSW_OVP_CFG_REG_1_ADDR(base) ((base) + (0x96))
#define REG_PSW_OVP_CFG_REG_2_ADDR(base) ((base) + (0x97))
#define REG_PSW_OVP_CFG_REG_3_ADDR(base) ((base) + (0x98))
#define REG_SCHG_LOGIC_CFG_REG_0_ADDR(base) ((base) + (0xA0))
#define REG_SCHG_LOGIC_CFG_REG_1_ADDR(base) ((base) + (0xA1))
#define REG_SCHG_LOGIC_CFG_REG_2_ADDR(base) ((base) + (0xA2))
#define REG_SCHG_LOGIC_CFG_REG_3_ADDR(base) ((base) + (0xA3))
#define REG_SCHG_LOGIC_CFG_REG_4_ADDR(base) ((base) + (0xA4))
#define REG_SCHG_LOGIC_CFG_REG_5_ADDR(base) ((base) + (0xA5))
#define REG_SCHG_LOGIC_CFG_REG_6_ADDR(base) ((base) + (0xA6))
#define REG_SCHG_LOGIC_CFG_REG_7_ADDR(base) ((base) + (0xA7))
#define REG_SCHG_LOGIC_CFG_REG_8_ADDR(base) ((base) + (0xA8))
#define REG_SCHG_LOGIC_RO_REG_0_ADDR(base) ((base) + (0xA9))
#define REG_SCHG_LOGIC_RO_REG_1_ADDR(base) ((base) + (0xAA))
#define REG_TCPC_CFG_REG_0_ADDR(base) ((base) + (0xB0))
#define REG_TCPC_CFG_REG_1_ADDR(base) ((base) + (0xB1))
#define REG_TCPC_CFG_REG_2_ADDR(base) ((base) + (0xB2))
#define REG_TCPC_CFG_REG_3_ADDR(base) ((base) + (0xB3))
#define REG_TYPE_CFG_REG_4_ADDR(base) ((base) + (0xB4))
#define REG_TCPC_RO_REG_1_ADDR(base) ((base) + (0xB5))
#define REG_OSC_TOP_CFG_REG_0_ADDR(base) ((base) + (0xB8))
#define REG_OSC_TOP_CFG_REG_1_ADDR(base) ((base) + (0xB9))
#define REG_OSC_TOP_CFG_REG_2_ADDR(base) ((base) + (0xBA))
#endif
#ifndef __SOC_H_FOR_ASM__
#define REG_VERSION0_RO_REG_0_ADDR(base) ((base) + (0x00UL))
#define REG_VERSION1_RO_REG_0_ADDR(base) ((base) + (0x01UL))
#define REG_VERSION2_RO_REG_0_ADDR(base) ((base) + (0x02UL))
#define REG_VERSION3_RO_REG_0_ADDR(base) ((base) + (0x03UL))
#define REG_VERSION4_RO_REG_0_ADDR(base) ((base) + (0x04UL))
#define REG_VERSION5_RO_REG_0_ADDR(base) ((base) + (0x05UL))
#define REG_SC_TESTBUS_CFG_ADDR(base) ((base) + (0x10UL))
#define REG_SC_TESTBUS_RDATA_ADDR(base) ((base) + (0x11UL))
#define REG_GLB_SOFT_RST_CTRL_ADDR(base) ((base) + (0x12UL))
#define REG_EFUSE_SOFT_RST_CTRL_ADDR(base) ((base) + (0x13UL))
#define REG_HKADC_SOFT_RST_CTRL_ADDR(base) ((base) + (0x14UL))
#define REG_COUL_SOFT_RST_CTRL_ADDR(base) ((base) + (0x15UL))
#define REG_COUL_PD_RST_CTRL_ADDR(base) ((base) + (0x16UL))
#define REG_EIS_SOFT_RST_CTRL_ADDR(base) ((base) + (0x17UL))
#define REG_SC_CRG_CLK_EN_CTRL_ADDR(base) ((base) + (0x18UL))
#define REG_SC_CRG_CLK_EN_CTRL_1_ADDR(base) ((base) + (0x19UL))
#define REG_SC_CRG_CLK_EN_CTRL_2_ADDR(base) ((base) + (0x1AUL))
#define REG_SC_TESTSIG_CFG_ADDR(base) ((base) + (0x1BUL))
#define REG_SOH_SOFT_RST_CTRL_ADDR(base) ((base) + (0x1CUL))
#define REG_SC_WDT_EN_ADDR(base) ((base) + (0x20UL))
#define REG_WDT_SOFT_RST_ADDR(base) ((base) + (0x21UL))
#define REG_WDT_CTRL_ADDR(base) ((base) + (0x22UL))
#define REG_WDT_TIMEOUT_ADDR(base) ((base) + (0x23UL))
#define REG_BAT_CNCT_SEL_ADDR(base) ((base) + (0x30UL))
#define REG_DBFET_SEL_ADDR(base) ((base) + (0x31UL))
#define REG_DBFET_SEL_EN_ADDR(base) ((base) + (0x32UL))
#define REG_SC_BUCK_ENB_ADDR(base) ((base) + (0x33UL))
#define REG_B_DIE_EN_ADDR(base) ((base) + (0x34UL))
#define REG_BAT_SW_EN_ADDR(base) ((base) + (0x35UL))
#define REG_LOWBAT_EN_ADDR(base) ((base) + (0x36UL))
#define REG_OVP_EN_ADDR(base) ((base) + (0x37UL))
#define REG_OVP_MODE_ADDR(base) ((base) + (0x38UL))
#define REG_PSW_EN_ADDR(base) ((base) + (0x39UL))
#define REG_PSW_MODE_ADDR(base) ((base) + (0x3AUL))
#define REG_AUX_SBU_EN_ADDR(base) ((base) + (0x3BUL))
#define REG_BAT_SEL_STATUS_ADDR(base) ((base) + (0x3CUL))
#define REG_EFUSE_PDOB_SEL_ADDR(base) ((base) + (0x60UL))
#define REG_EFUSE_MODE_ADDR(base) ((base) + (0x61UL))
#define REG_EFUSE_RD_CTRL_ADDR(base) ((base) + (0x62UL))
#define REG_EFUSE_AEN_ADDR(base) ((base) + (0x63UL))
#define REG_EFUSE_PGMEN_ADDR(base) ((base) + (0x64UL))
#define REG_EFUSE_RDEN_ADDR(base) ((base) + (0x65UL))
#define REG_EFUSE_ADDR_ADDR(base) ((base) + (0x66UL))
#define REG_EFUSE_ADDR_H_ADDR(base) ((base) + (0x67UL))
#define REG_EFUSE_PDOB_PRE_ADDR_WE_ADDR(base) ((base) + (0x68UL))
#define REG_EFUSE_PDOB_PRE_WDATA_ADDR(base) ((base) + (0x69UL))
#define REG_EFUSE_STATUS_ADDR(base) ((base) + (0x6AUL))
#define REG_EFUSE_FSM_RESET_ADDR(base) ((base) + (0x6BUL))
#define REG_EFUSE_TESTBUS_CFG_ADDR(base) ((base) + (0x6CUL))
#define REG_EFUSE_TESTBUS_RDATA_ADDR(base) ((base) + (0x6DUL))
#define REG_EFUSE_ADDR_CAL_SEL_ADDR(base) ((base) + (0x6EUL))
#define REG_CHG_OTG_MODE_CFG_ADDR(base) ((base) + (0x70UL))
#define REG_OTG_DMD_CFG_ADDR(base) ((base) + (0x71UL))
#define REG_BUCK_CFG_REG_0_ADDR(base) ((base) + (0x72UL))
#define REG_BUCK_CFG_REG_1_ADDR(base) ((base) + (0x73UL))
#define REG_BUCK_CFG_REG_2_ADDR(base) ((base) + (0x74UL))
#define REG_BUCK_CFG_REG_3_ADDR(base) ((base) + (0x75UL))
#define REG_BUCK_CFG_REG_4_ADDR(base) ((base) + (0x76UL))
#define REG_BUCK_CFG_REG_5_ADDR(base) ((base) + (0x77UL))
#define REG_BUCK_CFG_REG_6_ADDR(base) ((base) + (0x78UL))
#define REG_BUCK_CFG_REG_7_ADDR(base) ((base) + (0x79UL))
#define REG_BUCK_CFG_REG_8_ADDR(base) ((base) + (0x7AUL))
#define REG_OTG_CFG_REG_0_ADDR(base) ((base) + (0x7BUL))
#define REG_OTG_CFG_REG_1_ADDR(base) ((base) + (0x7CUL))
#define REG_OTG_CFG_REG_2_ADDR(base) ((base) + (0x7DUL))
#define REG_OTG_CFG_REG_3_ADDR(base) ((base) + (0x7EUL))
#define REG_OTG_CFG_REG_4_ADDR(base) ((base) + (0x7FUL))
#define REG_OTG_CFG_REG_5_ADDR(base) ((base) + (0x80UL))
#define REG_OTG_CFG_REG_6_ADDR(base) ((base) + (0x81UL))
#define REG_OTG_CFG_REG_7_ADDR(base) ((base) + (0x82UL))
#define REG_OTG_CFG_REG_8_ADDR(base) ((base) + (0x83UL))
#define REG_CHG_STATUS_ADDR(base) ((base) + (0x8EUL))
#define REG_CHG_TB_CFG_ADDR(base) ((base) + (0x8FUL))
#define REG_CHG_TB_BUS_ADDR(base) ((base) + (0x90UL))
#define REG_CHG_TB_SIG_ADDR(base) ((base) + (0x91UL))
#define REG_EFS_COUL_TRIM_0_ADDR(base) ((base) + (0x92UL))
#define REG_EFS_COUL_TRIM_1_ADDR(base) ((base) + (0x93UL))
#define REG_EFS_COUL_TRIM_2_ADDR(base) ((base) + (0x94UL))
#define REG_EFS_COUL_TRIM_3_ADDR(base) ((base) + (0x95UL))
#define REG_EFS_COUL_TRIM_4_ADDR(base) ((base) + (0x96UL))
#define REG_EFS_COUL_TRIM_5_ADDR(base) ((base) + (0x97UL))
#define REG_EFS_COUL_TRIM_6_ADDR(base) ((base) + (0x98UL))
#define REG_EFS_COUL_TRIM_7_ADDR(base) ((base) + (0x99UL))
#define REG_EFS_COUL_TRIM_8_ADDR(base) ((base) + (0x9AUL))
#define REG_EFS_COUL_TRIM_9_ADDR(base) ((base) + (0x9BUL))
#define REG_EFS_COUL_TRIM_10_ADDR(base) ((base) + (0x9CUL))
#define REG_EFS_COUL_TRIM_11_ADDR(base) ((base) + (0x9DUL))
#define REG_EFS_ADC_TRIM_0_ADDR(base) ((base) + (0x9EUL))
#define REG_EFS_DIG_TRIM_0_ADDR(base) ((base) + (0x9FUL))
#define REG_EFS_DIG_TRIM_1_ADDR(base) ((base) + (0xA0UL))
#else
#define REG_VERSION0_RO_REG_0_ADDR(base) ((base) + (0x00))
#define REG_VERSION1_RO_REG_0_ADDR(base) ((base) + (0x01))
#define REG_VERSION2_RO_REG_0_ADDR(base) ((base) + (0x02))
#define REG_VERSION3_RO_REG_0_ADDR(base) ((base) + (0x03))
#define REG_VERSION4_RO_REG_0_ADDR(base) ((base) + (0x04))
#define REG_VERSION5_RO_REG_0_ADDR(base) ((base) + (0x05))
#define REG_SC_TESTBUS_CFG_ADDR(base) ((base) + (0x10))
#define REG_SC_TESTBUS_RDATA_ADDR(base) ((base) + (0x11))
#define REG_GLB_SOFT_RST_CTRL_ADDR(base) ((base) + (0x12))
#define REG_EFUSE_SOFT_RST_CTRL_ADDR(base) ((base) + (0x13))
#define REG_HKADC_SOFT_RST_CTRL_ADDR(base) ((base) + (0x14))
#define REG_COUL_SOFT_RST_CTRL_ADDR(base) ((base) + (0x15))
#define REG_COUL_PD_RST_CTRL_ADDR(base) ((base) + (0x16))
#define REG_EIS_SOFT_RST_CTRL_ADDR(base) ((base) + (0x17))
#define REG_SC_CRG_CLK_EN_CTRL_ADDR(base) ((base) + (0x18))
#define REG_SC_CRG_CLK_EN_CTRL_1_ADDR(base) ((base) + (0x19))
#define REG_SC_CRG_CLK_EN_CTRL_2_ADDR(base) ((base) + (0x1A))
#define REG_SC_TESTSIG_CFG_ADDR(base) ((base) + (0x1B))
#define REG_SOH_SOFT_RST_CTRL_ADDR(base) ((base) + (0x1C))
#define REG_SC_WDT_EN_ADDR(base) ((base) + (0x20))
#define REG_WDT_SOFT_RST_ADDR(base) ((base) + (0x21))
#define REG_WDT_CTRL_ADDR(base) ((base) + (0x22))
#define REG_WDT_TIMEOUT_ADDR(base) ((base) + (0x23))
#define REG_BAT_CNCT_SEL_ADDR(base) ((base) + (0x30))
#define REG_DBFET_SEL_ADDR(base) ((base) + (0x31))
#define REG_DBFET_SEL_EN_ADDR(base) ((base) + (0x32))
#define REG_SC_BUCK_ENB_ADDR(base) ((base) + (0x33))
#define REG_B_DIE_EN_ADDR(base) ((base) + (0x34))
#define REG_BAT_SW_EN_ADDR(base) ((base) + (0x35))
#define REG_LOWBAT_EN_ADDR(base) ((base) + (0x36))
#define REG_OVP_EN_ADDR(base) ((base) + (0x37))
#define REG_OVP_MODE_ADDR(base) ((base) + (0x38))
#define REG_PSW_EN_ADDR(base) ((base) + (0x39))
#define REG_PSW_MODE_ADDR(base) ((base) + (0x3A))
#define REG_AUX_SBU_EN_ADDR(base) ((base) + (0x3B))
#define REG_BAT_SEL_STATUS_ADDR(base) ((base) + (0x3C))
#define REG_EFUSE_PDOB_SEL_ADDR(base) ((base) + (0x60))
#define REG_EFUSE_MODE_ADDR(base) ((base) + (0x61))
#define REG_EFUSE_RD_CTRL_ADDR(base) ((base) + (0x62))
#define REG_EFUSE_AEN_ADDR(base) ((base) + (0x63))
#define REG_EFUSE_PGMEN_ADDR(base) ((base) + (0x64))
#define REG_EFUSE_RDEN_ADDR(base) ((base) + (0x65))
#define REG_EFUSE_ADDR_ADDR(base) ((base) + (0x66))
#define REG_EFUSE_ADDR_H_ADDR(base) ((base) + (0x67))
#define REG_EFUSE_PDOB_PRE_ADDR_WE_ADDR(base) ((base) + (0x68))
#define REG_EFUSE_PDOB_PRE_WDATA_ADDR(base) ((base) + (0x69))
#define REG_EFUSE_STATUS_ADDR(base) ((base) + (0x6A))
#define REG_EFUSE_FSM_RESET_ADDR(base) ((base) + (0x6B))
#define REG_EFUSE_TESTBUS_CFG_ADDR(base) ((base) + (0x6C))
#define REG_EFUSE_TESTBUS_RDATA_ADDR(base) ((base) + (0x6D))
#define REG_EFUSE_ADDR_CAL_SEL_ADDR(base) ((base) + (0x6E))
#define REG_CHG_OTG_MODE_CFG_ADDR(base) ((base) + (0x70))
#define REG_OTG_DMD_CFG_ADDR(base) ((base) + (0x71))
#define REG_BUCK_CFG_REG_0_ADDR(base) ((base) + (0x72))
#define REG_BUCK_CFG_REG_1_ADDR(base) ((base) + (0x73))
#define REG_BUCK_CFG_REG_2_ADDR(base) ((base) + (0x74))
#define REG_BUCK_CFG_REG_3_ADDR(base) ((base) + (0x75))
#define REG_BUCK_CFG_REG_4_ADDR(base) ((base) + (0x76))
#define REG_BUCK_CFG_REG_5_ADDR(base) ((base) + (0x77))
#define REG_BUCK_CFG_REG_6_ADDR(base) ((base) + (0x78))
#define REG_BUCK_CFG_REG_7_ADDR(base) ((base) + (0x79))
#define REG_BUCK_CFG_REG_8_ADDR(base) ((base) + (0x7A))
#define REG_OTG_CFG_REG_0_ADDR(base) ((base) + (0x7B))
#define REG_OTG_CFG_REG_1_ADDR(base) ((base) + (0x7C))
#define REG_OTG_CFG_REG_2_ADDR(base) ((base) + (0x7D))
#define REG_OTG_CFG_REG_3_ADDR(base) ((base) + (0x7E))
#define REG_OTG_CFG_REG_4_ADDR(base) ((base) + (0x7F))
#define REG_OTG_CFG_REG_5_ADDR(base) ((base) + (0x80))
#define REG_OTG_CFG_REG_6_ADDR(base) ((base) + (0x81))
#define REG_OTG_CFG_REG_7_ADDR(base) ((base) + (0x82))
#define REG_OTG_CFG_REG_8_ADDR(base) ((base) + (0x83))
#define REG_CHG_STATUS_ADDR(base) ((base) + (0x8E))
#define REG_CHG_TB_CFG_ADDR(base) ((base) + (0x8F))
#define REG_CHG_TB_BUS_ADDR(base) ((base) + (0x90))
#define REG_CHG_TB_SIG_ADDR(base) ((base) + (0x91))
#define REG_EFS_COUL_TRIM_0_ADDR(base) ((base) + (0x92))
#define REG_EFS_COUL_TRIM_1_ADDR(base) ((base) + (0x93))
#define REG_EFS_COUL_TRIM_2_ADDR(base) ((base) + (0x94))
#define REG_EFS_COUL_TRIM_3_ADDR(base) ((base) + (0x95))
#define REG_EFS_COUL_TRIM_4_ADDR(base) ((base) + (0x96))
#define REG_EFS_COUL_TRIM_5_ADDR(base) ((base) + (0x97))
#define REG_EFS_COUL_TRIM_6_ADDR(base) ((base) + (0x98))
#define REG_EFS_COUL_TRIM_7_ADDR(base) ((base) + (0x99))
#define REG_EFS_COUL_TRIM_8_ADDR(base) ((base) + (0x9A))
#define REG_EFS_COUL_TRIM_9_ADDR(base) ((base) + (0x9B))
#define REG_EFS_COUL_TRIM_10_ADDR(base) ((base) + (0x9C))
#define REG_EFS_COUL_TRIM_11_ADDR(base) ((base) + (0x9D))
#define REG_EFS_ADC_TRIM_0_ADDR(base) ((base) + (0x9E))
#define REG_EFS_DIG_TRIM_0_ADDR(base) ((base) + (0x9F))
#define REG_EFS_DIG_TRIM_1_ADDR(base) ((base) + (0xA0))
#endif
#ifndef __SOC_H_FOR_ASM__
#define REG_CLJ_CTRL_REG_ADDR(base) ((base) + (0x00UL))
#define REG_CLJ_CTRL_REGS3_ADDR(base) ((base) + (0x01UL))
#define REG_CLJ_CTRL_REGS4_ADDR(base) ((base) + (0x02UL))
#define REG_CLJ_CTRL_REGS5_ADDR(base) ((base) + (0x03UL))
#define REG_SLIDE_CNT_DATA_OUT0_ADDR(base) ((base) + (0x04UL))
#define REG_SLIDE_CNT_DATA_OUT1_ADDR(base) ((base) + (0x05UL))
#define REG_SLIDE_CNT_DATA_OUT2_ADDR(base) ((base) + (0x06UL))
#define REG_SLIDE_IH_DATA0_OUT0_ADDR(base) ((base) + (0x07UL))
#define REG_SLIDE_IH_DATA0_OUT1_ADDR(base) ((base) + (0x08UL))
#define REG_SLIDE_IH_DATA0_OUT2_ADDR(base) ((base) + (0x09UL))
#define REG_SLIDE_IH_DATA1_OUT0_ADDR(base) ((base) + (0x0AUL))
#define REG_SLIDE_IH_DATA1_OUT1_ADDR(base) ((base) + (0x0BUL))
#define REG_SLIDE_IH_DATA1_OUT2_ADDR(base) ((base) + (0x0CUL))
#define REG_SLIDE_IH_DATA2_OUT0_ADDR(base) ((base) + (0x0DUL))
#define REG_SLIDE_IH_DATA2_OUT1_ADDR(base) ((base) + (0x0EUL))
#define REG_SLIDE_IH_DATA2_OUT2_ADDR(base) ((base) + (0x0FUL))
#define REG_SLIDE_IH_DATA3_OUT0_ADDR(base) ((base) + (0x10UL))
#define REG_SLIDE_IH_DATA3_OUT1_ADDR(base) ((base) + (0x11UL))
#define REG_SLIDE_IH_DATA3_OUT2_ADDR(base) ((base) + (0x12UL))
#define REG_SLIDE_VH_DATA0_OUT0_ADDR(base) ((base) + (0x13UL))
#define REG_SLIDE_VH_DATA0_OUT1_ADDR(base) ((base) + (0x14UL))
#define REG_SLIDE_VH_DATA0_OUT2_ADDR(base) ((base) + (0x15UL))
#define REG_SLIDE_VH_DATA1_OUT0_ADDR(base) ((base) + (0x16UL))
#define REG_SLIDE_VH_DATA1_OUT1_ADDR(base) ((base) + (0x17UL))
#define REG_SLIDE_VH_DATA1_OUT2_ADDR(base) ((base) + (0x18UL))
#define REG_SLIDE_VH_DATA2_OUT0_ADDR(base) ((base) + (0x19UL))
#define REG_SLIDE_VH_DATA2_OUT1_ADDR(base) ((base) + (0x1AUL))
#define REG_SLIDE_VH_DATA2_OUT2_ADDR(base) ((base) + (0x1BUL))
#define REG_SLIDE_VH_DATA3_OUT0_ADDR(base) ((base) + (0x1CUL))
#define REG_SLIDE_VH_DATA3_OUT1_ADDR(base) ((base) + (0x1DUL))
#define REG_SLIDE_VH_DATA3_OUT2_ADDR(base) ((base) + (0x1EUL))
#define REG_SLIDE_IL_DATA0_OUT0_ADDR(base) ((base) + (0x1FUL))
#define REG_SLIDE_IL_DATA0_OUT1_ADDR(base) ((base) + (0x20UL))
#define REG_SLIDE_IL_DATA0_OUT2_ADDR(base) ((base) + (0x21UL))
#define REG_SLIDE_IL_DATA1_OUT0_ADDR(base) ((base) + (0x22UL))
#define REG_SLIDE_IL_DATA1_OUT1_ADDR(base) ((base) + (0x23UL))
#define REG_SLIDE_IL_DATA1_OUT2_ADDR(base) ((base) + (0x24UL))
#define REG_SLIDE_IL_DATA2_OUT0_ADDR(base) ((base) + (0x25UL))
#define REG_SLIDE_IL_DATA2_OUT1_ADDR(base) ((base) + (0x26UL))
#define REG_SLIDE_IL_DATA2_OUT2_ADDR(base) ((base) + (0x27UL))
#define REG_SLIDE_IL_DATA3_OUT0_ADDR(base) ((base) + (0x28UL))
#define REG_SLIDE_IL_DATA3_OUT1_ADDR(base) ((base) + (0x29UL))
#define REG_SLIDE_IL_DATA3_OUT2_ADDR(base) ((base) + (0x2AUL))
#define REG_SLIDE_VL_DATA0_OUT0_ADDR(base) ((base) + (0x2BUL))
#define REG_SLIDE_VL_DATA0_OUT1_ADDR(base) ((base) + (0x2CUL))
#define REG_SLIDE_VL_DATA0_OUT2_ADDR(base) ((base) + (0x2DUL))
#define REG_SLIDE_VL_DATA1_OUT0_ADDR(base) ((base) + (0x2EUL))
#define REG_SLIDE_VL_DATA1_OUT1_ADDR(base) ((base) + (0x2FUL))
#define REG_SLIDE_VL_DATA1_OUT2_ADDR(base) ((base) + (0x30UL))
#define REG_SLIDE_VL_DATA2_OUT0_ADDR(base) ((base) + (0x31UL))
#define REG_SLIDE_VL_DATA2_OUT1_ADDR(base) ((base) + (0x32UL))
#define REG_SLIDE_VL_DATA2_OUT2_ADDR(base) ((base) + (0x33UL))
#define REG_SLIDE_VL_DATA3_OUT0_ADDR(base) ((base) + (0x34UL))
#define REG_SLIDE_VL_DATA3_OUT1_ADDR(base) ((base) + (0x35UL))
#define REG_SLIDE_VL_DATA3_OUT2_ADDR(base) ((base) + (0x36UL))
#else
#define REG_CLJ_CTRL_REG_ADDR(base) ((base) + (0x00))
#define REG_CLJ_CTRL_REGS3_ADDR(base) ((base) + (0x01))
#define REG_CLJ_CTRL_REGS4_ADDR(base) ((base) + (0x02))
#define REG_CLJ_CTRL_REGS5_ADDR(base) ((base) + (0x03))
#define REG_SLIDE_CNT_DATA_OUT0_ADDR(base) ((base) + (0x04))
#define REG_SLIDE_CNT_DATA_OUT1_ADDR(base) ((base) + (0x05))
#define REG_SLIDE_CNT_DATA_OUT2_ADDR(base) ((base) + (0x06))
#define REG_SLIDE_IH_DATA0_OUT0_ADDR(base) ((base) + (0x07))
#define REG_SLIDE_IH_DATA0_OUT1_ADDR(base) ((base) + (0x08))
#define REG_SLIDE_IH_DATA0_OUT2_ADDR(base) ((base) + (0x09))
#define REG_SLIDE_IH_DATA1_OUT0_ADDR(base) ((base) + (0x0A))
#define REG_SLIDE_IH_DATA1_OUT1_ADDR(base) ((base) + (0x0B))
#define REG_SLIDE_IH_DATA1_OUT2_ADDR(base) ((base) + (0x0C))
#define REG_SLIDE_IH_DATA2_OUT0_ADDR(base) ((base) + (0x0D))
#define REG_SLIDE_IH_DATA2_OUT1_ADDR(base) ((base) + (0x0E))
#define REG_SLIDE_IH_DATA2_OUT2_ADDR(base) ((base) + (0x0F))
#define REG_SLIDE_IH_DATA3_OUT0_ADDR(base) ((base) + (0x10))
#define REG_SLIDE_IH_DATA3_OUT1_ADDR(base) ((base) + (0x11))
#define REG_SLIDE_IH_DATA3_OUT2_ADDR(base) ((base) + (0x12))
#define REG_SLIDE_VH_DATA0_OUT0_ADDR(base) ((base) + (0x13))
#define REG_SLIDE_VH_DATA0_OUT1_ADDR(base) ((base) + (0x14))
#define REG_SLIDE_VH_DATA0_OUT2_ADDR(base) ((base) + (0x15))
#define REG_SLIDE_VH_DATA1_OUT0_ADDR(base) ((base) + (0x16))
#define REG_SLIDE_VH_DATA1_OUT1_ADDR(base) ((base) + (0x17))
#define REG_SLIDE_VH_DATA1_OUT2_ADDR(base) ((base) + (0x18))
#define REG_SLIDE_VH_DATA2_OUT0_ADDR(base) ((base) + (0x19))
#define REG_SLIDE_VH_DATA2_OUT1_ADDR(base) ((base) + (0x1A))
#define REG_SLIDE_VH_DATA2_OUT2_ADDR(base) ((base) + (0x1B))
#define REG_SLIDE_VH_DATA3_OUT0_ADDR(base) ((base) + (0x1C))
#define REG_SLIDE_VH_DATA3_OUT1_ADDR(base) ((base) + (0x1D))
#define REG_SLIDE_VH_DATA3_OUT2_ADDR(base) ((base) + (0x1E))
#define REG_SLIDE_IL_DATA0_OUT0_ADDR(base) ((base) + (0x1F))
#define REG_SLIDE_IL_DATA0_OUT1_ADDR(base) ((base) + (0x20))
#define REG_SLIDE_IL_DATA0_OUT2_ADDR(base) ((base) + (0x21))
#define REG_SLIDE_IL_DATA1_OUT0_ADDR(base) ((base) + (0x22))
#define REG_SLIDE_IL_DATA1_OUT1_ADDR(base) ((base) + (0x23))
#define REG_SLIDE_IL_DATA1_OUT2_ADDR(base) ((base) + (0x24))
#define REG_SLIDE_IL_DATA2_OUT0_ADDR(base) ((base) + (0x25))
#define REG_SLIDE_IL_DATA2_OUT1_ADDR(base) ((base) + (0x26))
#define REG_SLIDE_IL_DATA2_OUT2_ADDR(base) ((base) + (0x27))
#define REG_SLIDE_IL_DATA3_OUT0_ADDR(base) ((base) + (0x28))
#define REG_SLIDE_IL_DATA3_OUT1_ADDR(base) ((base) + (0x29))
#define REG_SLIDE_IL_DATA3_OUT2_ADDR(base) ((base) + (0x2A))
#define REG_SLIDE_VL_DATA0_OUT0_ADDR(base) ((base) + (0x2B))
#define REG_SLIDE_VL_DATA0_OUT1_ADDR(base) ((base) + (0x2C))
#define REG_SLIDE_VL_DATA0_OUT2_ADDR(base) ((base) + (0x2D))
#define REG_SLIDE_VL_DATA1_OUT0_ADDR(base) ((base) + (0x2E))
#define REG_SLIDE_VL_DATA1_OUT1_ADDR(base) ((base) + (0x2F))
#define REG_SLIDE_VL_DATA1_OUT2_ADDR(base) ((base) + (0x30))
#define REG_SLIDE_VL_DATA2_OUT0_ADDR(base) ((base) + (0x31))
#define REG_SLIDE_VL_DATA2_OUT1_ADDR(base) ((base) + (0x32))
#define REG_SLIDE_VL_DATA2_OUT2_ADDR(base) ((base) + (0x33))
#define REG_SLIDE_VL_DATA3_OUT0_ADDR(base) ((base) + (0x34))
#define REG_SLIDE_VL_DATA3_OUT1_ADDR(base) ((base) + (0x35))
#define REG_SLIDE_VL_DATA3_OUT2_ADDR(base) ((base) + (0x36))
#endif
#ifndef __SOC_H_FOR_ASM__
#define REG_COUL_IRQ_FLAG_ADDR(base) ((base) + (0x00UL))
#define REG_COUL_H_IRQ_FLAG_ADDR(base) ((base) + (0x01UL))
#define REG_COUL_L_IRQ_FLAG_ADDR(base) ((base) + (0x02UL))
#define REG_COUL_IRQ_MASK_ADDR(base) ((base) + (0x03UL))
#define REG_COUL_H_IRQ_MASK_ADDR(base) ((base) + (0x04UL))
#define REG_COUL_L_IRQ_MASK_ADDR(base) ((base) + (0x05UL))
#define REG_COUL_H_IRQ_STATUS_ADDR(base) ((base) + (0x06UL))
#define REG_COUL_L_IRQ_STATUS_ADDR(base) ((base) + (0x07UL))
#define REG_CLJ_CTRL_REG1_ADDR(base) ((base) + (0x09UL))
#define REG_CLJ_CTRL_REGS2_ADDR(base) ((base) + (0x0AUL))
#define REG_CIC_CTRL_ADDR(base) ((base) + (0x0BUL))
#define REG_STATE_TEST_ADDR(base) ((base) + (0x0CUL))
#define REG_CLJ_DEBUG0_ADDR(base) ((base) + (0x0DUL))
#define REG_CLJ_DEBUG1_ADDR(base) ((base) + (0x0EUL))
#define REG_DEBUG_WRITE_PRO_ADDR(base) ((base) + (0x10UL))
#define REG_COUL_CLK_CALI_EN_ADDR(base) ((base) + (0x11UL))
#define REG_DEBUG_CIC_DATA_SEL_ADDR(base) ((base) + (0x12UL))
#define REG_COUL_TB_CFG_ADDR(base) ((base) + (0x13UL))
#define REG_COUL_I_TB_BUS0_ADDR(base) ((base) + (0x14UL))
#define REG_COUL_I_TB_BUS1_ADDR(base) ((base) + (0x15UL))
#define REG_COUL_I_TB_BUS2_ADDR(base) ((base) + (0x16UL))
#define REG_COUL_V_TB_BUS0_ADDR(base) ((base) + (0x17UL))
#define REG_COUL_V_TB_BUS1_ADDR(base) ((base) + (0x18UL))
#define REG_COUL_V_TB_BUS2_ADDR(base) ((base) + (0x19UL))
#define REG_V_INT0_ADDR(base) ((base) + (0x1AUL))
#define REG_V_INT1_ADDR(base) ((base) + (0x1BUL))
#define REG_V_INT2_ADDR(base) ((base) + (0x1CUL))
#define REG_I_OUT_GATE0_ADDR(base) ((base) + (0x1DUL))
#define REG_I_OUT_GATE1_ADDR(base) ((base) + (0x1EUL))
#define REG_I_OUT_GATE2_ADDR(base) ((base) + (0x1FUL))
#define REG_I_IN_GATE0_ADDR(base) ((base) + (0x20UL))
#define REG_I_IN_GATE1_ADDR(base) ((base) + (0x21UL))
#define REG_I_IN_GATE2_ADDR(base) ((base) + (0x22UL))
#define REG_CL_INT0_ADDR(base) ((base) + (0x23UL))
#define REG_CL_INT1_ADDR(base) ((base) + (0x24UL))
#define REG_CL_INT2_ADDR(base) ((base) + (0x25UL))
#define REG_CL_INT3_ADDR(base) ((base) + (0x26UL))
#define REG_CL_INT4_ADDR(base) ((base) + (0x27UL))
#define REG_RTC_OCV_OUT_ADDR(base) ((base) + (0x28UL))
#define REG_RD_SYNC_PRO_ADDR(base) ((base) + (0x29UL))
#define REG_RD_SYNC_SEL_ADDR(base) ((base) + (0x2AUL))
#define REG_CIC_EN_DEBUG_ADDR(base) ((base) + (0x2BUL))
#define REG_SC_NICE_TB_SEL_ADDR(base) ((base) + (0x2CUL))
#define REG_SC_NICE_TB_TRIG_ADDR(base) ((base) + (0x2DUL))
#define REG_ECO_PWROFF_VLD_ADDR(base) ((base) + (0x2EUL))
#define REG_CLJ_DBG_CFG_ADDR(base) ((base) + (0x2FUL))
#define REG_CLJ_DBG_STATUS_ADDR(base) ((base) + (0x30UL))
#define REG_H_CL_OUT_RO0_ADDR(base) ((base) + (0x40UL))
#define REG_H_CL_OUT_RO1_ADDR(base) ((base) + (0x41UL))
#define REG_H_CL_OUT_RO2_ADDR(base) ((base) + (0x42UL))
#define REG_H_CL_OUT_RO3_ADDR(base) ((base) + (0x43UL))
#define REG_H_CL_OUT_RO4_ADDR(base) ((base) + (0x44UL))
#define REG_H_CL_IN_RO0_ADDR(base) ((base) + (0x45UL))
#define REG_H_CL_IN_RO1_ADDR(base) ((base) + (0x46UL))
#define REG_H_CL_IN_RO2_ADDR(base) ((base) + (0x47UL))
#define REG_H_CL_IN_RO3_ADDR(base) ((base) + (0x48UL))
#define REG_H_CL_IN_RO4_ADDR(base) ((base) + (0x49UL))
#define REG_H_CHG_TIMER_RO0_ADDR(base) ((base) + (0x4AUL))
#define REG_H_CHG_TIMER_RO1_ADDR(base) ((base) + (0x4BUL))
#define REG_H_CHG_TIMER_RO2_ADDR(base) ((base) + (0x4CUL))
#define REG_H_CHG_TIMER_RO3_ADDR(base) ((base) + (0x4DUL))
#define REG_H_LOAD_TIMER_RO0_ADDR(base) ((base) + (0x4EUL))
#define REG_H_LOAD_TIMER_RO1_ADDR(base) ((base) + (0x4FUL))
#define REG_H_LOAD_TIMER_RO2_ADDR(base) ((base) + (0x50UL))
#define REG_H_LOAD_TIMER_RO3_ADDR(base) ((base) + (0x51UL))
#define REG_L_CL_OUT_RO0_ADDR(base) ((base) + (0x52UL))
#define REG_L_CL_OUT_RO1_ADDR(base) ((base) + (0x53UL))
#define REG_L_CL_OUT_RO2_ADDR(base) ((base) + (0x54UL))
#define REG_L_CL_OUT_RO3_ADDR(base) ((base) + (0x55UL))
#define REG_L_CL_OUT_RO4_ADDR(base) ((base) + (0x56UL))
#define REG_L_CL_IN_RO0_ADDR(base) ((base) + (0x57UL))
#define REG_L_CL_IN_RO1_ADDR(base) ((base) + (0x58UL))
#define REG_L_CL_IN_RO2_ADDR(base) ((base) + (0x59UL))
#define REG_L_CL_IN_RO3_ADDR(base) ((base) + (0x5AUL))
#define REG_L_CL_IN_RO4_ADDR(base) ((base) + (0x5BUL))
#define REG_L_CHG_TIMER_RO0_ADDR(base) ((base) + (0x5CUL))
#define REG_L_CHG_TIMER_RO1_ADDR(base) ((base) + (0x5DUL))
#define REG_L_CHG_TIMER_RO2_ADDR(base) ((base) + (0x5EUL))
#define REG_L_CHG_TIMER_RO3_ADDR(base) ((base) + (0x5FUL))
#define REG_L_LOAD_TIMER_RO0_ADDR(base) ((base) + (0x60UL))
#define REG_L_LOAD_TIMER_RO1_ADDR(base) ((base) + (0x61UL))
#define REG_L_LOAD_TIMER_RO2_ADDR(base) ((base) + (0x62UL))
#define REG_L_LOAD_TIMER_RO3_ADDR(base) ((base) + (0x63UL))
#define REG_ECO_CNT1_OUT0_ADDR(base) ((base) + (0x64UL))
#define REG_ECO_CNT1_OUT1_ADDR(base) ((base) + (0x65UL))
#define REG_ECO_CNT2_OUT0_ADDR(base) ((base) + (0x66UL))
#define REG_ECO_CNT2_OUT1_ADDR(base) ((base) + (0x67UL))
#define REG_ECO_CNT3_OUT0_ADDR(base) ((base) + (0x68UL))
#define REG_ECO_CNT3_OUT1_ADDR(base) ((base) + (0x69UL))
#define REG_ECO_CNT4_OUT0_ADDR(base) ((base) + (0x6AUL))
#define REG_ECO_CNT4_OUT1_ADDR(base) ((base) + (0x6BUL))
#define REG_COUL_TOP_CFG_REG_0_ADDR(base) ((base) + (0x80UL))
#define REG_COUL_TOP_CFG_REG_1_ADDR(base) ((base) + (0x81UL))
#define REG_COUL_TOP_CFG_REG_2_ADDR(base) ((base) + (0x82UL))
#define REG_COUL_TOP_CFG_REG_3_ADDR(base) ((base) + (0x83UL))
#define REG_COUL_TOP_CFG_REG_4_ADDR(base) ((base) + (0x84UL))
#define REG_COUL_TOP_CFG_REG_5_ADDR(base) ((base) + (0x85UL))
#define REG_COUL_TOP_CFG_REG_6_ADDR(base) ((base) + (0x86UL))
#define REG_COUL_TOP_CFG_REG_7_ADDR(base) ((base) + (0x87UL))
#define REG_COUL_TOP_CFG_REG_8_ADDR(base) ((base) + (0x88UL))
#define REG_COUL_TOP_CFG_REG_9_ADDR(base) ((base) + (0x89UL))
#define REG_COUL_TOP_CFG_REG_10_ADDR(base) ((base) + (0x8AUL))
#define REG_COUL_TOP_CFG_REG_11_ADDR(base) ((base) + (0x8BUL))
#else
#define REG_COUL_IRQ_FLAG_ADDR(base) ((base) + (0x00))
#define REG_COUL_H_IRQ_FLAG_ADDR(base) ((base) + (0x01))
#define REG_COUL_L_IRQ_FLAG_ADDR(base) ((base) + (0x02))
#define REG_COUL_IRQ_MASK_ADDR(base) ((base) + (0x03))
#define REG_COUL_H_IRQ_MASK_ADDR(base) ((base) + (0x04))
#define REG_COUL_L_IRQ_MASK_ADDR(base) ((base) + (0x05))
#define REG_COUL_H_IRQ_STATUS_ADDR(base) ((base) + (0x06))
#define REG_COUL_L_IRQ_STATUS_ADDR(base) ((base) + (0x07))
#define REG_CLJ_CTRL_REG1_ADDR(base) ((base) + (0x09))
#define REG_CLJ_CTRL_REGS2_ADDR(base) ((base) + (0x0A))
#define REG_CIC_CTRL_ADDR(base) ((base) + (0x0B))
#define REG_STATE_TEST_ADDR(base) ((base) + (0x0C))
#define REG_CLJ_DEBUG0_ADDR(base) ((base) + (0x0D))
#define REG_CLJ_DEBUG1_ADDR(base) ((base) + (0x0E))
#define REG_DEBUG_WRITE_PRO_ADDR(base) ((base) + (0x10))
#define REG_COUL_CLK_CALI_EN_ADDR(base) ((base) + (0x11))
#define REG_DEBUG_CIC_DATA_SEL_ADDR(base) ((base) + (0x12))
#define REG_COUL_TB_CFG_ADDR(base) ((base) + (0x13))
#define REG_COUL_I_TB_BUS0_ADDR(base) ((base) + (0x14))
#define REG_COUL_I_TB_BUS1_ADDR(base) ((base) + (0x15))
#define REG_COUL_I_TB_BUS2_ADDR(base) ((base) + (0x16))
#define REG_COUL_V_TB_BUS0_ADDR(base) ((base) + (0x17))
#define REG_COUL_V_TB_BUS1_ADDR(base) ((base) + (0x18))
#define REG_COUL_V_TB_BUS2_ADDR(base) ((base) + (0x19))
#define REG_V_INT0_ADDR(base) ((base) + (0x1A))
#define REG_V_INT1_ADDR(base) ((base) + (0x1B))
#define REG_V_INT2_ADDR(base) ((base) + (0x1C))
#define REG_I_OUT_GATE0_ADDR(base) ((base) + (0x1D))
#define REG_I_OUT_GATE1_ADDR(base) ((base) + (0x1E))
#define REG_I_OUT_GATE2_ADDR(base) ((base) + (0x1F))
#define REG_I_IN_GATE0_ADDR(base) ((base) + (0x20))
#define REG_I_IN_GATE1_ADDR(base) ((base) + (0x21))
#define REG_I_IN_GATE2_ADDR(base) ((base) + (0x22))
#define REG_CL_INT0_ADDR(base) ((base) + (0x23))
#define REG_CL_INT1_ADDR(base) ((base) + (0x24))
#define REG_CL_INT2_ADDR(base) ((base) + (0x25))
#define REG_CL_INT3_ADDR(base) ((base) + (0x26))
#define REG_CL_INT4_ADDR(base) ((base) + (0x27))
#define REG_RTC_OCV_OUT_ADDR(base) ((base) + (0x28))
#define REG_RD_SYNC_PRO_ADDR(base) ((base) + (0x29))
#define REG_RD_SYNC_SEL_ADDR(base) ((base) + (0x2A))
#define REG_CIC_EN_DEBUG_ADDR(base) ((base) + (0x2B))
#define REG_SC_NICE_TB_SEL_ADDR(base) ((base) + (0x2C))
#define REG_SC_NICE_TB_TRIG_ADDR(base) ((base) + (0x2D))
#define REG_ECO_PWROFF_VLD_ADDR(base) ((base) + (0x2E))
#define REG_CLJ_DBG_CFG_ADDR(base) ((base) + (0x2F))
#define REG_CLJ_DBG_STATUS_ADDR(base) ((base) + (0x30))
#define REG_H_CL_OUT_RO0_ADDR(base) ((base) + (0x40))
#define REG_H_CL_OUT_RO1_ADDR(base) ((base) + (0x41))
#define REG_H_CL_OUT_RO2_ADDR(base) ((base) + (0x42))
#define REG_H_CL_OUT_RO3_ADDR(base) ((base) + (0x43))
#define REG_H_CL_OUT_RO4_ADDR(base) ((base) + (0x44))
#define REG_H_CL_IN_RO0_ADDR(base) ((base) + (0x45))
#define REG_H_CL_IN_RO1_ADDR(base) ((base) + (0x46))
#define REG_H_CL_IN_RO2_ADDR(base) ((base) + (0x47))
#define REG_H_CL_IN_RO3_ADDR(base) ((base) + (0x48))
#define REG_H_CL_IN_RO4_ADDR(base) ((base) + (0x49))
#define REG_H_CHG_TIMER_RO0_ADDR(base) ((base) + (0x4A))
#define REG_H_CHG_TIMER_RO1_ADDR(base) ((base) + (0x4B))
#define REG_H_CHG_TIMER_RO2_ADDR(base) ((base) + (0x4C))
#define REG_H_CHG_TIMER_RO3_ADDR(base) ((base) + (0x4D))
#define REG_H_LOAD_TIMER_RO0_ADDR(base) ((base) + (0x4E))
#define REG_H_LOAD_TIMER_RO1_ADDR(base) ((base) + (0x4F))
#define REG_H_LOAD_TIMER_RO2_ADDR(base) ((base) + (0x50))
#define REG_H_LOAD_TIMER_RO3_ADDR(base) ((base) + (0x51))
#define REG_L_CL_OUT_RO0_ADDR(base) ((base) + (0x52))
#define REG_L_CL_OUT_RO1_ADDR(base) ((base) + (0x53))
#define REG_L_CL_OUT_RO2_ADDR(base) ((base) + (0x54))
#define REG_L_CL_OUT_RO3_ADDR(base) ((base) + (0x55))
#define REG_L_CL_OUT_RO4_ADDR(base) ((base) + (0x56))
#define REG_L_CL_IN_RO0_ADDR(base) ((base) + (0x57))
#define REG_L_CL_IN_RO1_ADDR(base) ((base) + (0x58))
#define REG_L_CL_IN_RO2_ADDR(base) ((base) + (0x59))
#define REG_L_CL_IN_RO3_ADDR(base) ((base) + (0x5A))
#define REG_L_CL_IN_RO4_ADDR(base) ((base) + (0x5B))
#define REG_L_CHG_TIMER_RO0_ADDR(base) ((base) + (0x5C))
#define REG_L_CHG_TIMER_RO1_ADDR(base) ((base) + (0x5D))
#define REG_L_CHG_TIMER_RO2_ADDR(base) ((base) + (0x5E))
#define REG_L_CHG_TIMER_RO3_ADDR(base) ((base) + (0x5F))
#define REG_L_LOAD_TIMER_RO0_ADDR(base) ((base) + (0x60))
#define REG_L_LOAD_TIMER_RO1_ADDR(base) ((base) + (0x61))
#define REG_L_LOAD_TIMER_RO2_ADDR(base) ((base) + (0x62))
#define REG_L_LOAD_TIMER_RO3_ADDR(base) ((base) + (0x63))
#define REG_ECO_CNT1_OUT0_ADDR(base) ((base) + (0x64))
#define REG_ECO_CNT1_OUT1_ADDR(base) ((base) + (0x65))
#define REG_ECO_CNT2_OUT0_ADDR(base) ((base) + (0x66))
#define REG_ECO_CNT2_OUT1_ADDR(base) ((base) + (0x67))
#define REG_ECO_CNT3_OUT0_ADDR(base) ((base) + (0x68))
#define REG_ECO_CNT3_OUT1_ADDR(base) ((base) + (0x69))
#define REG_ECO_CNT4_OUT0_ADDR(base) ((base) + (0x6A))
#define REG_ECO_CNT4_OUT1_ADDR(base) ((base) + (0x6B))
#define REG_COUL_TOP_CFG_REG_0_ADDR(base) ((base) + (0x80))
#define REG_COUL_TOP_CFG_REG_1_ADDR(base) ((base) + (0x81))
#define REG_COUL_TOP_CFG_REG_2_ADDR(base) ((base) + (0x82))
#define REG_COUL_TOP_CFG_REG_3_ADDR(base) ((base) + (0x83))
#define REG_COUL_TOP_CFG_REG_4_ADDR(base) ((base) + (0x84))
#define REG_COUL_TOP_CFG_REG_5_ADDR(base) ((base) + (0x85))
#define REG_COUL_TOP_CFG_REG_6_ADDR(base) ((base) + (0x86))
#define REG_COUL_TOP_CFG_REG_7_ADDR(base) ((base) + (0x87))
#define REG_COUL_TOP_CFG_REG_8_ADDR(base) ((base) + (0x88))
#define REG_COUL_TOP_CFG_REG_9_ADDR(base) ((base) + (0x89))
#define REG_COUL_TOP_CFG_REG_10_ADDR(base) ((base) + (0x8A))
#define REG_COUL_TOP_CFG_REG_11_ADDR(base) ((base) + (0x8B))
#endif
#ifndef __SOC_H_FOR_ASM__
#define REG_H_CL_OUT0_ADDR(base) ((base) + (0x00UL))
#define REG_H_CL_OUT1_ADDR(base) ((base) + (0x01UL))
#define REG_H_CL_OUT2_ADDR(base) ((base) + (0x02UL))
#define REG_H_CL_OUT3_ADDR(base) ((base) + (0x03UL))
#define REG_H_CL_OUT4_ADDR(base) ((base) + (0x04UL))
#define REG_H_CL_IN0_ADDR(base) ((base) + (0x05UL))
#define REG_H_CL_IN1_ADDR(base) ((base) + (0x06UL))
#define REG_H_CL_IN2_ADDR(base) ((base) + (0x07UL))
#define REG_H_CL_IN3_ADDR(base) ((base) + (0x08UL))
#define REG_H_CL_IN4_ADDR(base) ((base) + (0x09UL))
#define REG_H_CHG_TIMER0_ADDR(base) ((base) + (0x0AUL))
#define REG_H_CHG_TIMER1_ADDR(base) ((base) + (0x0BUL))
#define REG_H_CHG_TIMER2_ADDR(base) ((base) + (0x0CUL))
#define REG_H_CHG_TIMER3_ADDR(base) ((base) + (0x0DUL))
#define REG_H_LOAD_TIMER0_ADDR(base) ((base) + (0x0EUL))
#define REG_H_LOAD_TIMER1_ADDR(base) ((base) + (0x0FUL))
#define REG_H_LOAD_TIMER2_ADDR(base) ((base) + (0x10UL))
#define REG_H_LOAD_TIMER3_ADDR(base) ((base) + (0x11UL))
#define REG_H0_OFFSET_CURRENT_MOD_0_ADDR(base) ((base) + (0x12UL))
#define REG_H0_OFFSET_CURRENT_MOD_1_ADDR(base) ((base) + (0x13UL))
#define REG_H0_OFFSET_CURRENT_MOD_2_ADDR(base) ((base) + (0x14UL))
#define REG_H1_OFFSET_CURRENT_MOD_0_ADDR(base) ((base) + (0x15UL))
#define REG_H1_OFFSET_CURRENT_MOD_1_ADDR(base) ((base) + (0x16UL))
#define REG_H1_OFFSET_CURRENT_MOD_2_ADDR(base) ((base) + (0x17UL))
#define REG_H_OFFSET_VOLTAGE_MOD_0_ADDR(base) ((base) + (0x18UL))
#define REG_H_OFFSET_VOLTAGE_MOD_1_ADDR(base) ((base) + (0x19UL))
#define REG_H_OFFSET_VOLTAGE_MOD_2_ADDR(base) ((base) + (0x1AUL))
#define REG_H_CURRENT_0_ADDR(base) ((base) + (0x1BUL))
#define REG_H_CURRENT_1_ADDR(base) ((base) + (0x1CUL))
#define REG_H_CURRENT_2_ADDR(base) ((base) + (0x1DUL))
#define REG_H_V_OUT_0_ADDR(base) ((base) + (0x1EUL))
#define REG_H_V_OUT_1_ADDR(base) ((base) + (0x1FUL))
#define REG_H_V_OUT_2_ADDR(base) ((base) + (0x20UL))
#define REG_H0_OFFSET_CURRENT0_ADDR(base) ((base) + (0x21UL))
#define REG_H0_OFFSET_CURRENT1_ADDR(base) ((base) + (0x22UL))
#define REG_H0_OFFSET_CURRENT2_ADDR(base) ((base) + (0x23UL))
#define REG_H1_OFFSET_CURRENT0_ADDR(base) ((base) + (0x24UL))
#define REG_H1_OFFSET_CURRENT1_ADDR(base) ((base) + (0x25UL))
#define REG_H1_OFFSET_CURRENT2_ADDR(base) ((base) + (0x26UL))
#define REG_H_OFFSET_VOLTAGE0_ADDR(base) ((base) + (0x27UL))
#define REG_H_OFFSET_VOLTAGE1_ADDR(base) ((base) + (0x28UL))
#define REG_H_OFFSET_VOLTAGE2_ADDR(base) ((base) + (0x29UL))
#define REG_H_OCV_VOLTAGE0_ADDR(base) ((base) + (0x2AUL))
#define REG_H_OCV_VOLTAGE1_ADDR(base) ((base) + (0x2BUL))
#define REG_H_OCV_VOLTAGE2_ADDR(base) ((base) + (0x2CUL))
#define REG_H_OCV_CURRENT0_ADDR(base) ((base) + (0x2DUL))
#define REG_H_OCV_CURRENT1_ADDR(base) ((base) + (0x2EUL))
#define REG_H_OCV_CURRENT2_ADDR(base) ((base) + (0x2FUL))
#define REG_H_ECO_OUT_CLIN_0_ADDR(base) ((base) + (0x30UL))
#define REG_H_ECO_OUT_CLIN_1_ADDR(base) ((base) + (0x31UL))
#define REG_H_ECO_OUT_CLIN_2_ADDR(base) ((base) + (0x32UL))
#define REG_H_ECO_OUT_CLIN_3_ADDR(base) ((base) + (0x33UL))
#define REG_H_ECO_OUT_CLIN_4_ADDR(base) ((base) + (0x34UL))
#define REG_H_ECO_OUT_CLOUT_0_ADDR(base) ((base) + (0x35UL))
#define REG_H_ECO_OUT_CLOUT_1_ADDR(base) ((base) + (0x36UL))
#define REG_H_ECO_OUT_CLOUT_2_ADDR(base) ((base) + (0x37UL))
#define REG_H_ECO_OUT_CLOUT_3_ADDR(base) ((base) + (0x38UL))
#define REG_H_ECO_OUT_CLOUT_4_ADDR(base) ((base) + (0x39UL))
#define REG_H_V_PRE0_OUT0_ADDR(base) ((base) + (0x3AUL))
#define REG_H_V_PRE0_OUT1_ADDR(base) ((base) + (0x3BUL))
#define REG_H_V_PRE0_OUT2_ADDR(base) ((base) + (0x3CUL))
#define REG_H_V_PRE1_OUT0_ADDR(base) ((base) + (0x3DUL))
#define REG_H_V_PRE1_OUT1_ADDR(base) ((base) + (0x3EUL))
#define REG_H_V_PRE1_OUT2_ADDR(base) ((base) + (0x3FUL))
#define REG_H_V_PRE2_OUT0_ADDR(base) ((base) + (0x40UL))
#define REG_H_V_PRE2_OUT1_ADDR(base) ((base) + (0x41UL))
#define REG_H_V_PRE2_OUT2_ADDR(base) ((base) + (0x42UL))
#define REG_H_V_PRE3_OUT0_ADDR(base) ((base) + (0x43UL))
#define REG_H_V_PRE3_OUT1_ADDR(base) ((base) + (0x44UL))
#define REG_H_V_PRE3_OUT2_ADDR(base) ((base) + (0x45UL))
#define REG_H_V_PRE4_OUT0_ADDR(base) ((base) + (0x46UL))
#define REG_H_V_PRE4_OUT1_ADDR(base) ((base) + (0x47UL))
#define REG_H_V_PRE4_OUT2_ADDR(base) ((base) + (0x48UL))
#define REG_H_V_PRE5_OUT0_ADDR(base) ((base) + (0x49UL))
#define REG_H_V_PRE5_OUT1_ADDR(base) ((base) + (0x4AUL))
#define REG_H_V_PRE5_OUT2_ADDR(base) ((base) + (0x4BUL))
#define REG_H_V_PRE6_OUT0_ADDR(base) ((base) + (0x4CUL))
#define REG_H_V_PRE6_OUT1_ADDR(base) ((base) + (0x4DUL))
#define REG_H_V_PRE6_OUT2_ADDR(base) ((base) + (0x4EUL))
#define REG_H_V_PRE7_OUT0_ADDR(base) ((base) + (0x4FUL))
#define REG_H_V_PRE7_OUT1_ADDR(base) ((base) + (0x50UL))
#define REG_H_V_PRE7_OUT2_ADDR(base) ((base) + (0x51UL))
#define REG_H_CURRENT_PRE0_OUT0_ADDR(base) ((base) + (0x52UL))
#define REG_H_CURRENT_PRE0_OUT1_ADDR(base) ((base) + (0x53UL))
#define REG_H_CURRENT_PRE0_OUT2_ADDR(base) ((base) + (0x54UL))
#define REG_H_CURRENT_PRE1_OUT0_ADDR(base) ((base) + (0x55UL))
#define REG_H_CURRENT_PRE1_OUT1_ADDR(base) ((base) + (0x56UL))
#define REG_H_CURRENT_PRE1_OUT2_ADDR(base) ((base) + (0x57UL))
#define REG_H_CURRENT_PRE2_OUT0_ADDR(base) ((base) + (0x58UL))
#define REG_H_CURRENT_PRE2_OUT1_ADDR(base) ((base) + (0x59UL))
#define REG_H_CURRENT_PRE2_OUT2_ADDR(base) ((base) + (0x5AUL))
#define REG_H_CURRENT_PRE3_OUT0_ADDR(base) ((base) + (0x5BUL))
#define REG_H_CURRENT_PRE3_OUT1_ADDR(base) ((base) + (0x5CUL))
#define REG_H_CURRENT_PRE3_OUT2_ADDR(base) ((base) + (0x5DUL))
#define REG_H_CURRENT_PRE4_OUT0_ADDR(base) ((base) + (0x5EUL))
#define REG_H_CURRENT_PRE4_OUT1_ADDR(base) ((base) + (0x5FUL))
#define REG_H_CURRENT_PRE4_OUT2_ADDR(base) ((base) + (0x60UL))
#define REG_H_CURRENT_PRE5_OUT0_ADDR(base) ((base) + (0x61UL))
#define REG_H_CURRENT_PRE5_OUT1_ADDR(base) ((base) + (0x62UL))
#define REG_H_CURRENT_PRE5_OUT2_ADDR(base) ((base) + (0x63UL))
#define REG_H_CURRENT_PRE6_OUT0_ADDR(base) ((base) + (0x64UL))
#define REG_H_CURRENT_PRE6_OUT1_ADDR(base) ((base) + (0x65UL))
#define REG_H_CURRENT_PRE6_OUT2_ADDR(base) ((base) + (0x66UL))
#define REG_H_CURRENT_PRE7_OUT0_ADDR(base) ((base) + (0x67UL))
#define REG_H_CURRENT_PRE7_OUT1_ADDR(base) ((base) + (0x68UL))
#define REG_H_CURRENT_PRE7_OUT2_ADDR(base) ((base) + (0x69UL))
#define REG_H_V_OCV_PRE1_OUT0_ADDR(base) ((base) + (0x6AUL))
#define REG_H_V_OCV_PRE1_OUT1_ADDR(base) ((base) + (0x6BUL))
#define REG_H_V_OCV_PRE1_OUT2_ADDR(base) ((base) + (0x6CUL))
#define REG_H_V_OCV_PRE2_OUT0_ADDR(base) ((base) + (0x6DUL))
#define REG_H_V_OCV_PRE2_OUT1_ADDR(base) ((base) + (0x6EUL))
#define REG_H_V_OCV_PRE2_OUT2_ADDR(base) ((base) + (0x6FUL))
#define REG_H_V_OCV_PRE3_OUT0_ADDR(base) ((base) + (0x70UL))
#define REG_H_V_OCV_PRE3_OUT1_ADDR(base) ((base) + (0x71UL))
#define REG_H_V_OCV_PRE3_OUT2_ADDR(base) ((base) + (0x72UL))
#define REG_H_V_OCV_PRE4_OUT0_ADDR(base) ((base) + (0x73UL))
#define REG_H_V_OCV_PRE4_OUT1_ADDR(base) ((base) + (0x74UL))
#define REG_H_V_OCV_PRE4_OUT2_ADDR(base) ((base) + (0x75UL))
#define REG_H_I_OCV_PRE1_OUT0_ADDR(base) ((base) + (0x76UL))
#define REG_H_I_OCV_PRE1_OUT1_ADDR(base) ((base) + (0x77UL))
#define REG_H_I_OCV_PRE1_OUT2_ADDR(base) ((base) + (0x78UL))
#define REG_H_I_OCV_PRE2_OUT0_ADDR(base) ((base) + (0x79UL))
#define REG_H_I_OCV_PRE2_OUT1_ADDR(base) ((base) + (0x7AUL))
#define REG_H_I_OCV_PRE2_OUT2_ADDR(base) ((base) + (0x7BUL))
#define REG_H_I_OCV_PRE3_OUT0_ADDR(base) ((base) + (0x7CUL))
#define REG_H_I_OCV_PRE3_OUT1_ADDR(base) ((base) + (0x7DUL))
#define REG_H_I_OCV_PRE3_OUT2_ADDR(base) ((base) + (0x7EUL))
#define REG_H_I_OCV_PRE4_OUT0_ADDR(base) ((base) + (0x7FUL))
#define REG_H_I_OCV_PRE4_OUT1_ADDR(base) ((base) + (0x80UL))
#define REG_H_I_OCV_PRE4_OUT2_ADDR(base) ((base) + (0x81UL))
#define REG_L_CL_OUT0_ADDR(base) ((base) + (0x82UL))
#define REG_L_CL_OUT1_ADDR(base) ((base) + (0x83UL))
#define REG_L_CL_OUT2_ADDR(base) ((base) + (0x84UL))
#define REG_L_CL_OUT3_ADDR(base) ((base) + (0x85UL))
#define REG_L_CL_OUT4_ADDR(base) ((base) + (0x86UL))
#define REG_L_CL_IN0_ADDR(base) ((base) + (0x87UL))
#define REG_L_CL_IN1_ADDR(base) ((base) + (0x88UL))
#define REG_L_CL_IN2_ADDR(base) ((base) + (0x89UL))
#define REG_L_CL_IN3_ADDR(base) ((base) + (0x8AUL))
#define REG_L_CL_IN4_ADDR(base) ((base) + (0x8BUL))
#define REG_L_CHG_TIMER0_ADDR(base) ((base) + (0x8CUL))
#define REG_L_CHG_TIMER1_ADDR(base) ((base) + (0x8DUL))
#define REG_L_CHG_TIMER2_ADDR(base) ((base) + (0x8EUL))
#define REG_L_CHG_TIMER3_ADDR(base) ((base) + (0x8FUL))
#define REG_L_LOAD_TIMER0_ADDR(base) ((base) + (0x90UL))
#define REG_L_LOAD_TIMER1_ADDR(base) ((base) + (0x91UL))
#define REG_L_LOAD_TIMER2_ADDR(base) ((base) + (0x92UL))
#define REG_L_LOAD_TIMER3_ADDR(base) ((base) + (0x93UL))
#define REG_L_OFFSET_CURRENT_MOD_0_ADDR(base) ((base) + (0x94UL))
#define REG_L_OFFSET_CURRENT_MOD_1_ADDR(base) ((base) + (0x95UL))
#define REG_L_OFFSET_CURRENT_MOD_2_ADDR(base) ((base) + (0x96UL))
#define REG_L_OFFSET_VOLTAGE_MOD_0_ADDR(base) ((base) + (0x97UL))
#define REG_L_OFFSET_VOLTAGE_MOD_1_ADDR(base) ((base) + (0x98UL))
#define REG_L_OFFSET_VOLTAGE_MOD_2_ADDR(base) ((base) + (0x99UL))
#define REG_L_CURRENT_0_ADDR(base) ((base) + (0x9AUL))
#define REG_L_CURRENT_1_ADDR(base) ((base) + (0x9BUL))
#define REG_L_CURRENT_2_ADDR(base) ((base) + (0x9CUL))
#define REG_L_V_OUT_0_ADDR(base) ((base) + (0x9DUL))
#define REG_L_V_OUT_1_ADDR(base) ((base) + (0x9EUL))
#define REG_L_V_OUT_2_ADDR(base) ((base) + (0x9FUL))
#define REG_L_OFFSET_CURRENT0_ADDR(base) ((base) + (0xA0UL))
#define REG_L_OFFSET_CURRENT1_ADDR(base) ((base) + (0xA1UL))
#define REG_L_OFFSET_CURRENT2_ADDR(base) ((base) + (0xA2UL))
#define REG_L_OFFSET_VOLTAGE0_ADDR(base) ((base) + (0xA3UL))
#define REG_L_OFFSET_VOLTAGE1_ADDR(base) ((base) + (0xA4UL))
#define REG_L_OFFSET_VOLTAGE2_ADDR(base) ((base) + (0xA5UL))
#define REG_L_OCV_VOLTAGE0_ADDR(base) ((base) + (0xA6UL))
#define REG_L_OCV_VOLTAGE1_ADDR(base) ((base) + (0xA7UL))
#define REG_L_OCV_VOLTAGE2_ADDR(base) ((base) + (0xA8UL))
#define REG_L_OCV_CURRENT0_ADDR(base) ((base) + (0xA9UL))
#define REG_L_OCV_CURRENT1_ADDR(base) ((base) + (0xAAUL))
#define REG_L_OCV_CURRENT2_ADDR(base) ((base) + (0xABUL))
#define REG_L_ECO_OUT_CLIN_0_ADDR(base) ((base) + (0xACUL))
#define REG_L_ECO_OUT_CLIN_1_ADDR(base) ((base) + (0xADUL))
#define REG_L_ECO_OUT_CLIN_2_ADDR(base) ((base) + (0xAEUL))
#define REG_L_ECO_OUT_CLIN_3_ADDR(base) ((base) + (0xAFUL))
#define REG_L_ECO_OUT_CLIN_4_ADDR(base) ((base) + (0xB0UL))
#define REG_L_ECO_OUT_CLOUT_0_ADDR(base) ((base) + (0xB1UL))
#define REG_L_ECO_OUT_CLOUT_1_ADDR(base) ((base) + (0xB2UL))
#define REG_L_ECO_OUT_CLOUT_2_ADDR(base) ((base) + (0xB3UL))
#define REG_L_ECO_OUT_CLOUT_3_ADDR(base) ((base) + (0xB4UL))
#define REG_L_ECO_OUT_CLOUT_4_ADDR(base) ((base) + (0xB5UL))
#define REG_L_V_PRE0_OUT0_ADDR(base) ((base) + (0xB6UL))
#define REG_L_V_PRE0_OUT1_ADDR(base) ((base) + (0xB7UL))
#define REG_L_V_PRE0_OUT2_ADDR(base) ((base) + (0xB8UL))
#define REG_L_V_PRE1_OUT0_ADDR(base) ((base) + (0xB9UL))
#define REG_L_V_PRE1_OUT1_ADDR(base) ((base) + (0xBAUL))
#define REG_L_V_PRE1_OUT2_ADDR(base) ((base) + (0xBBUL))
#define REG_L_V_PRE2_OUT0_ADDR(base) ((base) + (0xBCUL))
#define REG_L_V_PRE2_OUT1_ADDR(base) ((base) + (0xBDUL))
#define REG_L_V_PRE2_OUT2_ADDR(base) ((base) + (0xBEUL))
#define REG_L_V_PRE3_OUT0_ADDR(base) ((base) + (0xBFUL))
#define REG_L_V_PRE3_OUT1_ADDR(base) ((base) + (0xC0UL))
#define REG_L_V_PRE3_OUT2_ADDR(base) ((base) + (0xC1UL))
#define REG_L_V_PRE4_OUT0_ADDR(base) ((base) + (0xC2UL))
#define REG_L_V_PRE4_OUT1_ADDR(base) ((base) + (0xC3UL))
#define REG_L_V_PRE4_OUT2_ADDR(base) ((base) + (0xC4UL))
#define REG_L_V_PRE5_OUT0_ADDR(base) ((base) + (0xC5UL))
#define REG_L_V_PRE5_OUT1_ADDR(base) ((base) + (0xC6UL))
#define REG_L_V_PRE5_OUT2_ADDR(base) ((base) + (0xC7UL))
#define REG_L_V_PRE6_OUT0_ADDR(base) ((base) + (0xC8UL))
#define REG_L_V_PRE6_OUT1_ADDR(base) ((base) + (0xC9UL))
#define REG_L_V_PRE6_OUT2_ADDR(base) ((base) + (0xCAUL))
#define REG_L_V_PRE7_OUT0_ADDR(base) ((base) + (0xCBUL))
#define REG_L_V_PRE7_OUT1_ADDR(base) ((base) + (0xCCUL))
#define REG_L_V_PRE7_OUT2_ADDR(base) ((base) + (0xCDUL))
#define REG_L_CURRENT_PRE0_OUT0_ADDR(base) ((base) + (0xCEUL))
#define REG_L_CURRENT_PRE0_OUT1_ADDR(base) ((base) + (0xCFUL))
#define REG_L_CURRENT_PRE0_OUT2_ADDR(base) ((base) + (0xD0UL))
#define REG_L_CURRENT_PRE1_OUT0_ADDR(base) ((base) + (0xD1UL))
#define REG_L_CURRENT_PRE1_OUT1_ADDR(base) ((base) + (0xD2UL))
#define REG_L_CURRENT_PRE1_OUT2_ADDR(base) ((base) + (0xD3UL))
#define REG_L_CURRENT_PRE2_OUT0_ADDR(base) ((base) + (0xD4UL))
#define REG_L_CURRENT_PRE2_OUT1_ADDR(base) ((base) + (0xD5UL))
#define REG_L_CURRENT_PRE2_OUT2_ADDR(base) ((base) + (0xD6UL))
#define REG_L_CURRENT_PRE3_OUT0_ADDR(base) ((base) + (0xD7UL))
#define REG_L_CURRENT_PRE3_OUT1_ADDR(base) ((base) + (0xD8UL))
#define REG_L_CURRENT_PRE3_OUT2_ADDR(base) ((base) + (0xD9UL))
#define REG_L_CURRENT_PRE4_OUT0_ADDR(base) ((base) + (0xDAUL))
#define REG_L_CURRENT_PRE4_OUT1_ADDR(base) ((base) + (0xDBUL))
#define REG_L_CURRENT_PRE4_OUT2_ADDR(base) ((base) + (0xDCUL))
#define REG_L_CURRENT_PRE5_OUT0_ADDR(base) ((base) + (0xDDUL))
#define REG_L_CURRENT_PRE5_OUT1_ADDR(base) ((base) + (0xDEUL))
#define REG_L_CURRENT_PRE5_OUT2_ADDR(base) ((base) + (0xDFUL))
#define REG_L_CURRENT_PRE6_OUT0_ADDR(base) ((base) + (0xE0UL))
#define REG_L_CURRENT_PRE6_OUT1_ADDR(base) ((base) + (0xE1UL))
#define REG_L_CURRENT_PRE6_OUT2_ADDR(base) ((base) + (0xE2UL))
#define REG_L_CURRENT_PRE7_OUT0_ADDR(base) ((base) + (0xE3UL))
#define REG_L_CURRENT_PRE7_OUT1_ADDR(base) ((base) + (0xE4UL))
#define REG_L_CURRENT_PRE7_OUT2_ADDR(base) ((base) + (0xE5UL))
#define REG_L_V_OCV_PRE1_OUT0_ADDR(base) ((base) + (0xE6UL))
#define REG_L_V_OCV_PRE1_OUT1_ADDR(base) ((base) + (0xE7UL))
#define REG_L_V_OCV_PRE1_OUT2_ADDR(base) ((base) + (0xE8UL))
#define REG_L_V_OCV_PRE2_OUT0_ADDR(base) ((base) + (0xE9UL))
#define REG_L_V_OCV_PRE2_OUT1_ADDR(base) ((base) + (0xEAUL))
#define REG_L_V_OCV_PRE2_OUT2_ADDR(base) ((base) + (0xEBUL))
#define REG_L_V_OCV_PRE3_OUT0_ADDR(base) ((base) + (0xECUL))
#define REG_L_V_OCV_PRE3_OUT1_ADDR(base) ((base) + (0xEDUL))
#define REG_L_V_OCV_PRE3_OUT2_ADDR(base) ((base) + (0xEEUL))
#define REG_L_V_OCV_PRE4_OUT0_ADDR(base) ((base) + (0xEFUL))
#define REG_L_V_OCV_PRE4_OUT1_ADDR(base) ((base) + (0xF0UL))
#define REG_L_V_OCV_PRE4_OUT2_ADDR(base) ((base) + (0xF1UL))
#define REG_L_I_OCV_PRE1_OUT0_ADDR(base) ((base) + (0xF2UL))
#define REG_L_I_OCV_PRE1_OUT1_ADDR(base) ((base) + (0xF3UL))
#define REG_L_I_OCV_PRE1_OUT2_ADDR(base) ((base) + (0xF4UL))
#define REG_L_I_OCV_PRE2_OUT0_ADDR(base) ((base) + (0xF5UL))
#define REG_L_I_OCV_PRE2_OUT1_ADDR(base) ((base) + (0xF6UL))
#define REG_L_I_OCV_PRE2_OUT2_ADDR(base) ((base) + (0xF7UL))
#define REG_L_I_OCV_PRE3_OUT0_ADDR(base) ((base) + (0xF8UL))
#define REG_L_I_OCV_PRE3_OUT1_ADDR(base) ((base) + (0xF9UL))
#define REG_L_I_OCV_PRE3_OUT2_ADDR(base) ((base) + (0xFAUL))
#define REG_L_I_OCV_PRE4_OUT0_ADDR(base) ((base) + (0xFBUL))
#define REG_L_I_OCV_PRE4_OUT1_ADDR(base) ((base) + (0xFCUL))
#define REG_L_I_OCV_PRE4_OUT2_ADDR(base) ((base) + (0xFDUL))
#else
#define REG_H_CL_OUT0_ADDR(base) ((base) + (0x00))
#define REG_H_CL_OUT1_ADDR(base) ((base) + (0x01))
#define REG_H_CL_OUT2_ADDR(base) ((base) + (0x02))
#define REG_H_CL_OUT3_ADDR(base) ((base) + (0x03))
#define REG_H_CL_OUT4_ADDR(base) ((base) + (0x04))
#define REG_H_CL_IN0_ADDR(base) ((base) + (0x05))
#define REG_H_CL_IN1_ADDR(base) ((base) + (0x06))
#define REG_H_CL_IN2_ADDR(base) ((base) + (0x07))
#define REG_H_CL_IN3_ADDR(base) ((base) + (0x08))
#define REG_H_CL_IN4_ADDR(base) ((base) + (0x09))
#define REG_H_CHG_TIMER0_ADDR(base) ((base) + (0x0A))
#define REG_H_CHG_TIMER1_ADDR(base) ((base) + (0x0B))
#define REG_H_CHG_TIMER2_ADDR(base) ((base) + (0x0C))
#define REG_H_CHG_TIMER3_ADDR(base) ((base) + (0x0D))
#define REG_H_LOAD_TIMER0_ADDR(base) ((base) + (0x0E))
#define REG_H_LOAD_TIMER1_ADDR(base) ((base) + (0x0F))
#define REG_H_LOAD_TIMER2_ADDR(base) ((base) + (0x10))
#define REG_H_LOAD_TIMER3_ADDR(base) ((base) + (0x11))
#define REG_H0_OFFSET_CURRENT_MOD_0_ADDR(base) ((base) + (0x12))
#define REG_H0_OFFSET_CURRENT_MOD_1_ADDR(base) ((base) + (0x13))
#define REG_H0_OFFSET_CURRENT_MOD_2_ADDR(base) ((base) + (0x14))
#define REG_H1_OFFSET_CURRENT_MOD_0_ADDR(base) ((base) + (0x15))
#define REG_H1_OFFSET_CURRENT_MOD_1_ADDR(base) ((base) + (0x16))
#define REG_H1_OFFSET_CURRENT_MOD_2_ADDR(base) ((base) + (0x17))
#define REG_H_OFFSET_VOLTAGE_MOD_0_ADDR(base) ((base) + (0x18))
#define REG_H_OFFSET_VOLTAGE_MOD_1_ADDR(base) ((base) + (0x19))
#define REG_H_OFFSET_VOLTAGE_MOD_2_ADDR(base) ((base) + (0x1A))
#define REG_H_CURRENT_0_ADDR(base) ((base) + (0x1B))
#define REG_H_CURRENT_1_ADDR(base) ((base) + (0x1C))
#define REG_H_CURRENT_2_ADDR(base) ((base) + (0x1D))
#define REG_H_V_OUT_0_ADDR(base) ((base) + (0x1E))
#define REG_H_V_OUT_1_ADDR(base) ((base) + (0x1F))
#define REG_H_V_OUT_2_ADDR(base) ((base) + (0x20))
#define REG_H0_OFFSET_CURRENT0_ADDR(base) ((base) + (0x21))
#define REG_H0_OFFSET_CURRENT1_ADDR(base) ((base) + (0x22))
#define REG_H0_OFFSET_CURRENT2_ADDR(base) ((base) + (0x23))
#define REG_H1_OFFSET_CURRENT0_ADDR(base) ((base) + (0x24))
#define REG_H1_OFFSET_CURRENT1_ADDR(base) ((base) + (0x25))
#define REG_H1_OFFSET_CURRENT2_ADDR(base) ((base) + (0x26))
#define REG_H_OFFSET_VOLTAGE0_ADDR(base) ((base) + (0x27))
#define REG_H_OFFSET_VOLTAGE1_ADDR(base) ((base) + (0x28))
#define REG_H_OFFSET_VOLTAGE2_ADDR(base) ((base) + (0x29))
#define REG_H_OCV_VOLTAGE0_ADDR(base) ((base) + (0x2A))
#define REG_H_OCV_VOLTAGE1_ADDR(base) ((base) + (0x2B))
#define REG_H_OCV_VOLTAGE2_ADDR(base) ((base) + (0x2C))
#define REG_H_OCV_CURRENT0_ADDR(base) ((base) + (0x2D))
#define REG_H_OCV_CURRENT1_ADDR(base) ((base) + (0x2E))
#define REG_H_OCV_CURRENT2_ADDR(base) ((base) + (0x2F))
#define REG_H_ECO_OUT_CLIN_0_ADDR(base) ((base) + (0x30))
#define REG_H_ECO_OUT_CLIN_1_ADDR(base) ((base) + (0x31))
#define REG_H_ECO_OUT_CLIN_2_ADDR(base) ((base) + (0x32))
#define REG_H_ECO_OUT_CLIN_3_ADDR(base) ((base) + (0x33))
#define REG_H_ECO_OUT_CLIN_4_ADDR(base) ((base) + (0x34))
#define REG_H_ECO_OUT_CLOUT_0_ADDR(base) ((base) + (0x35))
#define REG_H_ECO_OUT_CLOUT_1_ADDR(base) ((base) + (0x36))
#define REG_H_ECO_OUT_CLOUT_2_ADDR(base) ((base) + (0x37))
#define REG_H_ECO_OUT_CLOUT_3_ADDR(base) ((base) + (0x38))
#define REG_H_ECO_OUT_CLOUT_4_ADDR(base) ((base) + (0x39))
#define REG_H_V_PRE0_OUT0_ADDR(base) ((base) + (0x3A))
#define REG_H_V_PRE0_OUT1_ADDR(base) ((base) + (0x3B))
#define REG_H_V_PRE0_OUT2_ADDR(base) ((base) + (0x3C))
#define REG_H_V_PRE1_OUT0_ADDR(base) ((base) + (0x3D))
#define REG_H_V_PRE1_OUT1_ADDR(base) ((base) + (0x3E))
#define REG_H_V_PRE1_OUT2_ADDR(base) ((base) + (0x3F))
#define REG_H_V_PRE2_OUT0_ADDR(base) ((base) + (0x40))
#define REG_H_V_PRE2_OUT1_ADDR(base) ((base) + (0x41))
#define REG_H_V_PRE2_OUT2_ADDR(base) ((base) + (0x42))
#define REG_H_V_PRE3_OUT0_ADDR(base) ((base) + (0x43))
#define REG_H_V_PRE3_OUT1_ADDR(base) ((base) + (0x44))
#define REG_H_V_PRE3_OUT2_ADDR(base) ((base) + (0x45))
#define REG_H_V_PRE4_OUT0_ADDR(base) ((base) + (0x46))
#define REG_H_V_PRE4_OUT1_ADDR(base) ((base) + (0x47))
#define REG_H_V_PRE4_OUT2_ADDR(base) ((base) + (0x48))
#define REG_H_V_PRE5_OUT0_ADDR(base) ((base) + (0x49))
#define REG_H_V_PRE5_OUT1_ADDR(base) ((base) + (0x4A))
#define REG_H_V_PRE5_OUT2_ADDR(base) ((base) + (0x4B))
#define REG_H_V_PRE6_OUT0_ADDR(base) ((base) + (0x4C))
#define REG_H_V_PRE6_OUT1_ADDR(base) ((base) + (0x4D))
#define REG_H_V_PRE6_OUT2_ADDR(base) ((base) + (0x4E))
#define REG_H_V_PRE7_OUT0_ADDR(base) ((base) + (0x4F))
#define REG_H_V_PRE7_OUT1_ADDR(base) ((base) + (0x50))
#define REG_H_V_PRE7_OUT2_ADDR(base) ((base) + (0x51))
#define REG_H_CURRENT_PRE0_OUT0_ADDR(base) ((base) + (0x52))
#define REG_H_CURRENT_PRE0_OUT1_ADDR(base) ((base) + (0x53))
#define REG_H_CURRENT_PRE0_OUT2_ADDR(base) ((base) + (0x54))
#define REG_H_CURRENT_PRE1_OUT0_ADDR(base) ((base) + (0x55))
#define REG_H_CURRENT_PRE1_OUT1_ADDR(base) ((base) + (0x56))
#define REG_H_CURRENT_PRE1_OUT2_ADDR(base) ((base) + (0x57))
#define REG_H_CURRENT_PRE2_OUT0_ADDR(base) ((base) + (0x58))
#define REG_H_CURRENT_PRE2_OUT1_ADDR(base) ((base) + (0x59))
#define REG_H_CURRENT_PRE2_OUT2_ADDR(base) ((base) + (0x5A))
#define REG_H_CURRENT_PRE3_OUT0_ADDR(base) ((base) + (0x5B))
#define REG_H_CURRENT_PRE3_OUT1_ADDR(base) ((base) + (0x5C))
#define REG_H_CURRENT_PRE3_OUT2_ADDR(base) ((base) + (0x5D))
#define REG_H_CURRENT_PRE4_OUT0_ADDR(base) ((base) + (0x5E))
#define REG_H_CURRENT_PRE4_OUT1_ADDR(base) ((base) + (0x5F))
#define REG_H_CURRENT_PRE4_OUT2_ADDR(base) ((base) + (0x60))
#define REG_H_CURRENT_PRE5_OUT0_ADDR(base) ((base) + (0x61))
#define REG_H_CURRENT_PRE5_OUT1_ADDR(base) ((base) + (0x62))
#define REG_H_CURRENT_PRE5_OUT2_ADDR(base) ((base) + (0x63))
#define REG_H_CURRENT_PRE6_OUT0_ADDR(base) ((base) + (0x64))
#define REG_H_CURRENT_PRE6_OUT1_ADDR(base) ((base) + (0x65))
#define REG_H_CURRENT_PRE6_OUT2_ADDR(base) ((base) + (0x66))
#define REG_H_CURRENT_PRE7_OUT0_ADDR(base) ((base) + (0x67))
#define REG_H_CURRENT_PRE7_OUT1_ADDR(base) ((base) + (0x68))
#define REG_H_CURRENT_PRE7_OUT2_ADDR(base) ((base) + (0x69))
#define REG_H_V_OCV_PRE1_OUT0_ADDR(base) ((base) + (0x6A))
#define REG_H_V_OCV_PRE1_OUT1_ADDR(base) ((base) + (0x6B))
#define REG_H_V_OCV_PRE1_OUT2_ADDR(base) ((base) + (0x6C))
#define REG_H_V_OCV_PRE2_OUT0_ADDR(base) ((base) + (0x6D))
#define REG_H_V_OCV_PRE2_OUT1_ADDR(base) ((base) + (0x6E))
#define REG_H_V_OCV_PRE2_OUT2_ADDR(base) ((base) + (0x6F))
#define REG_H_V_OCV_PRE3_OUT0_ADDR(base) ((base) + (0x70))
#define REG_H_V_OCV_PRE3_OUT1_ADDR(base) ((base) + (0x71))
#define REG_H_V_OCV_PRE3_OUT2_ADDR(base) ((base) + (0x72))
#define REG_H_V_OCV_PRE4_OUT0_ADDR(base) ((base) + (0x73))
#define REG_H_V_OCV_PRE4_OUT1_ADDR(base) ((base) + (0x74))
#define REG_H_V_OCV_PRE4_OUT2_ADDR(base) ((base) + (0x75))
#define REG_H_I_OCV_PRE1_OUT0_ADDR(base) ((base) + (0x76))
#define REG_H_I_OCV_PRE1_OUT1_ADDR(base) ((base) + (0x77))
#define REG_H_I_OCV_PRE1_OUT2_ADDR(base) ((base) + (0x78))
#define REG_H_I_OCV_PRE2_OUT0_ADDR(base) ((base) + (0x79))
#define REG_H_I_OCV_PRE2_OUT1_ADDR(base) ((base) + (0x7A))
#define REG_H_I_OCV_PRE2_OUT2_ADDR(base) ((base) + (0x7B))
#define REG_H_I_OCV_PRE3_OUT0_ADDR(base) ((base) + (0x7C))
#define REG_H_I_OCV_PRE3_OUT1_ADDR(base) ((base) + (0x7D))
#define REG_H_I_OCV_PRE3_OUT2_ADDR(base) ((base) + (0x7E))
#define REG_H_I_OCV_PRE4_OUT0_ADDR(base) ((base) + (0x7F))
#define REG_H_I_OCV_PRE4_OUT1_ADDR(base) ((base) + (0x80))
#define REG_H_I_OCV_PRE4_OUT2_ADDR(base) ((base) + (0x81))
#define REG_L_CL_OUT0_ADDR(base) ((base) + (0x82))
#define REG_L_CL_OUT1_ADDR(base) ((base) + (0x83))
#define REG_L_CL_OUT2_ADDR(base) ((base) + (0x84))
#define REG_L_CL_OUT3_ADDR(base) ((base) + (0x85))
#define REG_L_CL_OUT4_ADDR(base) ((base) + (0x86))
#define REG_L_CL_IN0_ADDR(base) ((base) + (0x87))
#define REG_L_CL_IN1_ADDR(base) ((base) + (0x88))
#define REG_L_CL_IN2_ADDR(base) ((base) + (0x89))
#define REG_L_CL_IN3_ADDR(base) ((base) + (0x8A))
#define REG_L_CL_IN4_ADDR(base) ((base) + (0x8B))
#define REG_L_CHG_TIMER0_ADDR(base) ((base) + (0x8C))
#define REG_L_CHG_TIMER1_ADDR(base) ((base) + (0x8D))
#define REG_L_CHG_TIMER2_ADDR(base) ((base) + (0x8E))
#define REG_L_CHG_TIMER3_ADDR(base) ((base) + (0x8F))
#define REG_L_LOAD_TIMER0_ADDR(base) ((base) + (0x90))
#define REG_L_LOAD_TIMER1_ADDR(base) ((base) + (0x91))
#define REG_L_LOAD_TIMER2_ADDR(base) ((base) + (0x92))
#define REG_L_LOAD_TIMER3_ADDR(base) ((base) + (0x93))
#define REG_L_OFFSET_CURRENT_MOD_0_ADDR(base) ((base) + (0x94))
#define REG_L_OFFSET_CURRENT_MOD_1_ADDR(base) ((base) + (0x95))
#define REG_L_OFFSET_CURRENT_MOD_2_ADDR(base) ((base) + (0x96))
#define REG_L_OFFSET_VOLTAGE_MOD_0_ADDR(base) ((base) + (0x97))
#define REG_L_OFFSET_VOLTAGE_MOD_1_ADDR(base) ((base) + (0x98))
#define REG_L_OFFSET_VOLTAGE_MOD_2_ADDR(base) ((base) + (0x99))
#define REG_L_CURRENT_0_ADDR(base) ((base) + (0x9A))
#define REG_L_CURRENT_1_ADDR(base) ((base) + (0x9B))
#define REG_L_CURRENT_2_ADDR(base) ((base) + (0x9C))
#define REG_L_V_OUT_0_ADDR(base) ((base) + (0x9D))
#define REG_L_V_OUT_1_ADDR(base) ((base) + (0x9E))
#define REG_L_V_OUT_2_ADDR(base) ((base) + (0x9F))
#define REG_L_OFFSET_CURRENT0_ADDR(base) ((base) + (0xA0))
#define REG_L_OFFSET_CURRENT1_ADDR(base) ((base) + (0xA1))
#define REG_L_OFFSET_CURRENT2_ADDR(base) ((base) + (0xA2))
#define REG_L_OFFSET_VOLTAGE0_ADDR(base) ((base) + (0xA3))
#define REG_L_OFFSET_VOLTAGE1_ADDR(base) ((base) + (0xA4))
#define REG_L_OFFSET_VOLTAGE2_ADDR(base) ((base) + (0xA5))
#define REG_L_OCV_VOLTAGE0_ADDR(base) ((base) + (0xA6))
#define REG_L_OCV_VOLTAGE1_ADDR(base) ((base) + (0xA7))
#define REG_L_OCV_VOLTAGE2_ADDR(base) ((base) + (0xA8))
#define REG_L_OCV_CURRENT0_ADDR(base) ((base) + (0xA9))
#define REG_L_OCV_CURRENT1_ADDR(base) ((base) + (0xAA))
#define REG_L_OCV_CURRENT2_ADDR(base) ((base) + (0xAB))
#define REG_L_ECO_OUT_CLIN_0_ADDR(base) ((base) + (0xAC))
#define REG_L_ECO_OUT_CLIN_1_ADDR(base) ((base) + (0xAD))
#define REG_L_ECO_OUT_CLIN_2_ADDR(base) ((base) + (0xAE))
#define REG_L_ECO_OUT_CLIN_3_ADDR(base) ((base) + (0xAF))
#define REG_L_ECO_OUT_CLIN_4_ADDR(base) ((base) + (0xB0))
#define REG_L_ECO_OUT_CLOUT_0_ADDR(base) ((base) + (0xB1))
#define REG_L_ECO_OUT_CLOUT_1_ADDR(base) ((base) + (0xB2))
#define REG_L_ECO_OUT_CLOUT_2_ADDR(base) ((base) + (0xB3))
#define REG_L_ECO_OUT_CLOUT_3_ADDR(base) ((base) + (0xB4))
#define REG_L_ECO_OUT_CLOUT_4_ADDR(base) ((base) + (0xB5))
#define REG_L_V_PRE0_OUT0_ADDR(base) ((base) + (0xB6))
#define REG_L_V_PRE0_OUT1_ADDR(base) ((base) + (0xB7))
#define REG_L_V_PRE0_OUT2_ADDR(base) ((base) + (0xB8))
#define REG_L_V_PRE1_OUT0_ADDR(base) ((base) + (0xB9))
#define REG_L_V_PRE1_OUT1_ADDR(base) ((base) + (0xBA))
#define REG_L_V_PRE1_OUT2_ADDR(base) ((base) + (0xBB))
#define REG_L_V_PRE2_OUT0_ADDR(base) ((base) + (0xBC))
#define REG_L_V_PRE2_OUT1_ADDR(base) ((base) + (0xBD))
#define REG_L_V_PRE2_OUT2_ADDR(base) ((base) + (0xBE))
#define REG_L_V_PRE3_OUT0_ADDR(base) ((base) + (0xBF))
#define REG_L_V_PRE3_OUT1_ADDR(base) ((base) + (0xC0))
#define REG_L_V_PRE3_OUT2_ADDR(base) ((base) + (0xC1))
#define REG_L_V_PRE4_OUT0_ADDR(base) ((base) + (0xC2))
#define REG_L_V_PRE4_OUT1_ADDR(base) ((base) + (0xC3))
#define REG_L_V_PRE4_OUT2_ADDR(base) ((base) + (0xC4))
#define REG_L_V_PRE5_OUT0_ADDR(base) ((base) + (0xC5))
#define REG_L_V_PRE5_OUT1_ADDR(base) ((base) + (0xC6))
#define REG_L_V_PRE5_OUT2_ADDR(base) ((base) + (0xC7))
#define REG_L_V_PRE6_OUT0_ADDR(base) ((base) + (0xC8))
#define REG_L_V_PRE6_OUT1_ADDR(base) ((base) + (0xC9))
#define REG_L_V_PRE6_OUT2_ADDR(base) ((base) + (0xCA))
#define REG_L_V_PRE7_OUT0_ADDR(base) ((base) + (0xCB))
#define REG_L_V_PRE7_OUT1_ADDR(base) ((base) + (0xCC))
#define REG_L_V_PRE7_OUT2_ADDR(base) ((base) + (0xCD))
#define REG_L_CURRENT_PRE0_OUT0_ADDR(base) ((base) + (0xCE))
#define REG_L_CURRENT_PRE0_OUT1_ADDR(base) ((base) + (0xCF))
#define REG_L_CURRENT_PRE0_OUT2_ADDR(base) ((base) + (0xD0))
#define REG_L_CURRENT_PRE1_OUT0_ADDR(base) ((base) + (0xD1))
#define REG_L_CURRENT_PRE1_OUT1_ADDR(base) ((base) + (0xD2))
#define REG_L_CURRENT_PRE1_OUT2_ADDR(base) ((base) + (0xD3))
#define REG_L_CURRENT_PRE2_OUT0_ADDR(base) ((base) + (0xD4))
#define REG_L_CURRENT_PRE2_OUT1_ADDR(base) ((base) + (0xD5))
#define REG_L_CURRENT_PRE2_OUT2_ADDR(base) ((base) + (0xD6))
#define REG_L_CURRENT_PRE3_OUT0_ADDR(base) ((base) + (0xD7))
#define REG_L_CURRENT_PRE3_OUT1_ADDR(base) ((base) + (0xD8))
#define REG_L_CURRENT_PRE3_OUT2_ADDR(base) ((base) + (0xD9))
#define REG_L_CURRENT_PRE4_OUT0_ADDR(base) ((base) + (0xDA))
#define REG_L_CURRENT_PRE4_OUT1_ADDR(base) ((base) + (0xDB))
#define REG_L_CURRENT_PRE4_OUT2_ADDR(base) ((base) + (0xDC))
#define REG_L_CURRENT_PRE5_OUT0_ADDR(base) ((base) + (0xDD))
#define REG_L_CURRENT_PRE5_OUT1_ADDR(base) ((base) + (0xDE))
#define REG_L_CURRENT_PRE5_OUT2_ADDR(base) ((base) + (0xDF))
#define REG_L_CURRENT_PRE6_OUT0_ADDR(base) ((base) + (0xE0))
#define REG_L_CURRENT_PRE6_OUT1_ADDR(base) ((base) + (0xE1))
#define REG_L_CURRENT_PRE6_OUT2_ADDR(base) ((base) + (0xE2))
#define REG_L_CURRENT_PRE7_OUT0_ADDR(base) ((base) + (0xE3))
#define REG_L_CURRENT_PRE7_OUT1_ADDR(base) ((base) + (0xE4))
#define REG_L_CURRENT_PRE7_OUT2_ADDR(base) ((base) + (0xE5))
#define REG_L_V_OCV_PRE1_OUT0_ADDR(base) ((base) + (0xE6))
#define REG_L_V_OCV_PRE1_OUT1_ADDR(base) ((base) + (0xE7))
#define REG_L_V_OCV_PRE1_OUT2_ADDR(base) ((base) + (0xE8))
#define REG_L_V_OCV_PRE2_OUT0_ADDR(base) ((base) + (0xE9))
#define REG_L_V_OCV_PRE2_OUT1_ADDR(base) ((base) + (0xEA))
#define REG_L_V_OCV_PRE2_OUT2_ADDR(base) ((base) + (0xEB))
#define REG_L_V_OCV_PRE3_OUT0_ADDR(base) ((base) + (0xEC))
#define REG_L_V_OCV_PRE3_OUT1_ADDR(base) ((base) + (0xED))
#define REG_L_V_OCV_PRE3_OUT2_ADDR(base) ((base) + (0xEE))
#define REG_L_V_OCV_PRE4_OUT0_ADDR(base) ((base) + (0xEF))
#define REG_L_V_OCV_PRE4_OUT1_ADDR(base) ((base) + (0xF0))
#define REG_L_V_OCV_PRE4_OUT2_ADDR(base) ((base) + (0xF1))
#define REG_L_I_OCV_PRE1_OUT0_ADDR(base) ((base) + (0xF2))
#define REG_L_I_OCV_PRE1_OUT1_ADDR(base) ((base) + (0xF3))
#define REG_L_I_OCV_PRE1_OUT2_ADDR(base) ((base) + (0xF4))
#define REG_L_I_OCV_PRE2_OUT0_ADDR(base) ((base) + (0xF5))
#define REG_L_I_OCV_PRE2_OUT1_ADDR(base) ((base) + (0xF6))
#define REG_L_I_OCV_PRE2_OUT2_ADDR(base) ((base) + (0xF7))
#define REG_L_I_OCV_PRE3_OUT0_ADDR(base) ((base) + (0xF8))
#define REG_L_I_OCV_PRE3_OUT1_ADDR(base) ((base) + (0xF9))
#define REG_L_I_OCV_PRE3_OUT2_ADDR(base) ((base) + (0xFA))
#define REG_L_I_OCV_PRE4_OUT0_ADDR(base) ((base) + (0xFB))
#define REG_L_I_OCV_PRE4_OUT1_ADDR(base) ((base) + (0xFC))
#define REG_L_I_OCV_PRE4_OUT2_ADDR(base) ((base) + (0xFD))
#endif
#ifndef __SOC_H_FOR_ASM__
#define REG_IRQ_FLAG_ADDR(base) ((base) + (0x00UL))
#define REG_IRQ_FLAG_0_ADDR(base) ((base) + (0x01UL))
#define REG_IRQ_FLAG_1_ADDR(base) ((base) + (0x02UL))
#define REG_IRQ_FLAG_2_ADDR(base) ((base) + (0x03UL))
#define REG_IRQ_FLAG_3_ADDR(base) ((base) + (0x04UL))
#define REG_IRQ_FLAG_4_ADDR(base) ((base) + (0x05UL))
#define REG_IRQ_FLAG_5_ADDR(base) ((base) + (0x06UL))
#define REG_IRQ_FLAG_6_ADDR(base) ((base) + (0x07UL))
#define REG_IRQ_FLAG_7_ADDR(base) ((base) + (0x08UL))
#define REG_IRQ_FLAG_8_ADDR(base) ((base) + (0x09UL))
#define REG_IRQ_FLAG_9_ADDR(base) ((base) + (0x0AUL))
#define REG_IRQ_FLAG_10_ADDR(base) ((base) + (0x0BUL))
#define REG_IRQ_FLAG_11_ADDR(base) ((base) + (0x0CUL))
#define REG_IRQ_MASK_ADDR(base) ((base) + (0x0DUL))
#define REG_IRQ_MASK_0_ADDR(base) ((base) + (0x0EUL))
#define REG_IRQ_MASK_1_ADDR(base) ((base) + (0x0FUL))
#define REG_IRQ_MASK_2_ADDR(base) ((base) + (0x10UL))
#define REG_IRQ_MASK_3_ADDR(base) ((base) + (0x11UL))
#define REG_IRQ_MASK_4_ADDR(base) ((base) + (0x12UL))
#define REG_IRQ_MASK_5_ADDR(base) ((base) + (0x13UL))
#define REG_IRQ_MASK_6_ADDR(base) ((base) + (0x14UL))
#define REG_IRQ_MASK_7_ADDR(base) ((base) + (0x15UL))
#define REG_IRQ_MASK_8_ADDR(base) ((base) + (0x16UL))
#define REG_IRQ_MASK_9_ADDR(base) ((base) + (0x17UL))
#define REG_IRQ_MASK_10_ADDR(base) ((base) + (0x18UL))
#define REG_IRQ_MASK_11_ADDR(base) ((base) + (0x19UL))
#define REG_IRQ_STATUS_0_ADDR(base) ((base) + (0x1AUL))
#define REG_IRQ_STATUS_1_ADDR(base) ((base) + (0x1BUL))
#define REG_IRQ_STATUS_2_ADDR(base) ((base) + (0x1CUL))
#define REG_IRQ_STATUS_3_ADDR(base) ((base) + (0x1DUL))
#define REG_IRQ_STATUS_4_ADDR(base) ((base) + (0x1EUL))
#define REG_IRQ_STATUS_5_ADDR(base) ((base) + (0x1FUL))
#define REG_IRQ_STATUS_6_ADDR(base) ((base) + (0x20UL))
#define REG_IRQ_STATUS_7_ADDR(base) ((base) + (0x21UL))
#define REG_IRQ_STATUS_8_ADDR(base) ((base) + (0x22UL))
#define REG_IRQ_STATUS_9_ADDR(base) ((base) + (0x23UL))
#define REG_IRQ_STATUS_10_ADDR(base) ((base) + (0x24UL))
#define REG_IRQ_STATUS_11_ADDR(base) ((base) + (0x25UL))
#define REG_IRQ_ANA_STATUS_ADDR(base) ((base) + (0x26UL))
#define REG_FAKE_IRQ_ADDR(base) ((base) + (0x30UL))
#define REG_IRQ_SEL_ADDR(base) ((base) + (0x31UL))
#else
#define REG_IRQ_FLAG_ADDR(base) ((base) + (0x00))
#define REG_IRQ_FLAG_0_ADDR(base) ((base) + (0x01))
#define REG_IRQ_FLAG_1_ADDR(base) ((base) + (0x02))
#define REG_IRQ_FLAG_2_ADDR(base) ((base) + (0x03))
#define REG_IRQ_FLAG_3_ADDR(base) ((base) + (0x04))
#define REG_IRQ_FLAG_4_ADDR(base) ((base) + (0x05))
#define REG_IRQ_FLAG_5_ADDR(base) ((base) + (0x06))
#define REG_IRQ_FLAG_6_ADDR(base) ((base) + (0x07))
#define REG_IRQ_FLAG_7_ADDR(base) ((base) + (0x08))
#define REG_IRQ_FLAG_8_ADDR(base) ((base) + (0x09))
#define REG_IRQ_FLAG_9_ADDR(base) ((base) + (0x0A))
#define REG_IRQ_FLAG_10_ADDR(base) ((base) + (0x0B))
#define REG_IRQ_FLAG_11_ADDR(base) ((base) + (0x0C))
#define REG_IRQ_MASK_ADDR(base) ((base) + (0x0D))
#define REG_IRQ_MASK_0_ADDR(base) ((base) + (0x0E))
#define REG_IRQ_MASK_1_ADDR(base) ((base) + (0x0F))
#define REG_IRQ_MASK_2_ADDR(base) ((base) + (0x10))
#define REG_IRQ_MASK_3_ADDR(base) ((base) + (0x11))
#define REG_IRQ_MASK_4_ADDR(base) ((base) + (0x12))
#define REG_IRQ_MASK_5_ADDR(base) ((base) + (0x13))
#define REG_IRQ_MASK_6_ADDR(base) ((base) + (0x14))
#define REG_IRQ_MASK_7_ADDR(base) ((base) + (0x15))
#define REG_IRQ_MASK_8_ADDR(base) ((base) + (0x16))
#define REG_IRQ_MASK_9_ADDR(base) ((base) + (0x17))
#define REG_IRQ_MASK_10_ADDR(base) ((base) + (0x18))
#define REG_IRQ_MASK_11_ADDR(base) ((base) + (0x19))
#define REG_IRQ_STATUS_0_ADDR(base) ((base) + (0x1A))
#define REG_IRQ_STATUS_1_ADDR(base) ((base) + (0x1B))
#define REG_IRQ_STATUS_2_ADDR(base) ((base) + (0x1C))
#define REG_IRQ_STATUS_3_ADDR(base) ((base) + (0x1D))
#define REG_IRQ_STATUS_4_ADDR(base) ((base) + (0x1E))
#define REG_IRQ_STATUS_5_ADDR(base) ((base) + (0x1F))
#define REG_IRQ_STATUS_6_ADDR(base) ((base) + (0x20))
#define REG_IRQ_STATUS_7_ADDR(base) ((base) + (0x21))
#define REG_IRQ_STATUS_8_ADDR(base) ((base) + (0x22))
#define REG_IRQ_STATUS_9_ADDR(base) ((base) + (0x23))
#define REG_IRQ_STATUS_10_ADDR(base) ((base) + (0x24))
#define REG_IRQ_STATUS_11_ADDR(base) ((base) + (0x25))
#define REG_IRQ_ANA_STATUS_ADDR(base) ((base) + (0x26))
#define REG_FAKE_IRQ_ADDR(base) ((base) + (0x30))
#define REG_IRQ_SEL_ADDR(base) ((base) + (0x31))
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char pd_vend_id_l : 8;
    } reg;
} REG_VENDIDL_UNION;
#endif
#define REG_VENDIDL_pd_vend_id_l_START (0)
#define REG_VENDIDL_pd_vend_id_l_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char pd_vend_id_h : 8;
    } reg;
} REG_VENDIDH_UNION;
#endif
#define REG_VENDIDH_pd_vend_id_h_START (0)
#define REG_VENDIDH_pd_vend_id_h_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char pd_prod_id_l : 8;
    } reg;
} REG_PRODIDL_UNION;
#endif
#define REG_PRODIDL_pd_prod_id_l_START (0)
#define REG_PRODIDL_pd_prod_id_l_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char pd_prod_id_h : 8;
    } reg;
} REG_PRODIDH_UNION;
#endif
#define REG_PRODIDH_pd_prod_id_h_START (0)
#define REG_PRODIDH_pd_prod_id_h_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char pd_dev_id_l : 8;
    } reg;
} REG_DEVIDL_UNION;
#endif
#define REG_DEVIDL_pd_dev_id_l_START (0)
#define REG_DEVIDL_pd_dev_id_l_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char pd_dev_id_h : 8;
    } reg;
} REG_DEVIDH_UNION;
#endif
#define REG_DEVIDH_pd_dev_id_h_START (0)
#define REG_DEVIDH_pd_dev_id_h_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char pd_typc_revision_l : 8;
    } reg;
} REG_TYPECREVL_UNION;
#endif
#define REG_TYPECREVL_pd_typc_revision_l_START (0)
#define REG_TYPECREVL_pd_typc_revision_l_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char pd_typc_revision_h : 8;
    } reg;
} REG_TYPECREVH_UNION;
#endif
#define REG_TYPECREVH_pd_typc_revision_h_START (0)
#define REG_TYPECREVH_pd_typc_revision_h_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char pd_version : 8;
    } reg;
} REG_USBPDVER_UNION;
#endif
#define REG_USBPDVER_pd_version_START (0)
#define REG_USBPDVER_pd_version_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char pd_revision : 8;
    } reg;
} REG_USBPDREV_UNION;
#endif
#define REG_USBPDREV_pd_revision_START (0)
#define REG_USBPDREV_pd_revision_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char pd_itf_revision_l : 8;
    } reg;
} REG_PDIFREVL_UNION;
#endif
#define REG_PDIFREVL_pd_itf_revision_l_START (0)
#define REG_PDIFREVL_pd_itf_revision_l_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char pd_itf_revision_h : 8;
    } reg;
} REG_PDIFREVH_UNION;
#endif
#define REG_PDIFREVH_pd_itf_revision_h_START (0)
#define REG_PDIFREVH_pd_itf_revision_h_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char pd_int_ccstatus : 1;
        unsigned char pd_int_port_pwr : 1;
        unsigned char pd_int_rxstat : 1;
        unsigned char pd_int_rxhardrst : 1;
        unsigned char pd_int_txfail : 1;
        unsigned char pd_int_txdisc : 1;
        unsigned char pd_int_txsucc : 1;
        unsigned char pd_int_vbus_alrm_h : 1;
    } reg;
} REG_PD_ALERT_L_UNION;
#endif
#define REG_PD_ALERT_L_pd_int_ccstatus_START (0)
#define REG_PD_ALERT_L_pd_int_ccstatus_END (0)
#define REG_PD_ALERT_L_pd_int_port_pwr_START (1)
#define REG_PD_ALERT_L_pd_int_port_pwr_END (1)
#define REG_PD_ALERT_L_pd_int_rxstat_START (2)
#define REG_PD_ALERT_L_pd_int_rxstat_END (2)
#define REG_PD_ALERT_L_pd_int_rxhardrst_START (3)
#define REG_PD_ALERT_L_pd_int_rxhardrst_END (3)
#define REG_PD_ALERT_L_pd_int_txfail_START (4)
#define REG_PD_ALERT_L_pd_int_txfail_END (4)
#define REG_PD_ALERT_L_pd_int_txdisc_START (5)
#define REG_PD_ALERT_L_pd_int_txdisc_END (5)
#define REG_PD_ALERT_L_pd_int_txsucc_START (6)
#define REG_PD_ALERT_L_pd_int_txsucc_END (6)
#define REG_PD_ALERT_L_pd_int_vbus_alrm_h_START (7)
#define REG_PD_ALERT_L_pd_int_vbus_alrm_h_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char pd_int_vbus_alrm_l : 1;
        unsigned char pd_int_fault : 1;
        unsigned char pd_int_rx_full : 1;
        unsigned char pd_int_vbus_snk_disc : 1;
        unsigned char reserved : 3;
        unsigned char pd_int_fr_swap : 1;
    } reg;
} REG_PD_ALERT_H_UNION;
#endif
#define REG_PD_ALERT_H_pd_int_vbus_alrm_l_START (0)
#define REG_PD_ALERT_H_pd_int_vbus_alrm_l_END (0)
#define REG_PD_ALERT_H_pd_int_fault_START (1)
#define REG_PD_ALERT_H_pd_int_fault_END (1)
#define REG_PD_ALERT_H_pd_int_rx_full_START (2)
#define REG_PD_ALERT_H_pd_int_rx_full_END (2)
#define REG_PD_ALERT_H_pd_int_vbus_snk_disc_START (3)
#define REG_PD_ALERT_H_pd_int_vbus_snk_disc_END (3)
#define REG_PD_ALERT_H_pd_int_fr_swap_START (7)
#define REG_PD_ALERT_H_pd_int_fr_swap_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char pd_msk_ccstatus : 1;
        unsigned char pd_msk_port_pwr : 1;
        unsigned char pd_msk_rxstat : 1;
        unsigned char pd_msk_rxhardrst : 1;
        unsigned char pd_msk_txfail : 1;
        unsigned char pd_msk_txdisc : 1;
        unsigned char pd_msk_txsucc : 1;
        unsigned char pd_msk_vbus_alrm_h : 1;
    } reg;
} REG_PD_ALERT_MSK_L_UNION;
#endif
#define REG_PD_ALERT_MSK_L_pd_msk_ccstatus_START (0)
#define REG_PD_ALERT_MSK_L_pd_msk_ccstatus_END (0)
#define REG_PD_ALERT_MSK_L_pd_msk_port_pwr_START (1)
#define REG_PD_ALERT_MSK_L_pd_msk_port_pwr_END (1)
#define REG_PD_ALERT_MSK_L_pd_msk_rxstat_START (2)
#define REG_PD_ALERT_MSK_L_pd_msk_rxstat_END (2)
#define REG_PD_ALERT_MSK_L_pd_msk_rxhardrst_START (3)
#define REG_PD_ALERT_MSK_L_pd_msk_rxhardrst_END (3)
#define REG_PD_ALERT_MSK_L_pd_msk_txfail_START (4)
#define REG_PD_ALERT_MSK_L_pd_msk_txfail_END (4)
#define REG_PD_ALERT_MSK_L_pd_msk_txdisc_START (5)
#define REG_PD_ALERT_MSK_L_pd_msk_txdisc_END (5)
#define REG_PD_ALERT_MSK_L_pd_msk_txsucc_START (6)
#define REG_PD_ALERT_MSK_L_pd_msk_txsucc_END (6)
#define REG_PD_ALERT_MSK_L_pd_msk_vbus_alrm_h_START (7)
#define REG_PD_ALERT_MSK_L_pd_msk_vbus_alrm_h_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char pd_msk_vbus_alrm_l : 1;
        unsigned char pd_msk_fault : 1;
        unsigned char pd_msk_rx_full : 1;
        unsigned char pd_msk_vbus_snk_disc : 1;
        unsigned char reserved : 3;
        unsigned char pd_msk_fr_swap : 1;
    } reg;
} REG_PD_ALERT_MSK_H_UNION;
#endif
#define REG_PD_ALERT_MSK_H_pd_msk_vbus_alrm_l_START (0)
#define REG_PD_ALERT_MSK_H_pd_msk_vbus_alrm_l_END (0)
#define REG_PD_ALERT_MSK_H_pd_msk_fault_START (1)
#define REG_PD_ALERT_MSK_H_pd_msk_fault_END (1)
#define REG_PD_ALERT_MSK_H_pd_msk_rx_full_START (2)
#define REG_PD_ALERT_MSK_H_pd_msk_rx_full_END (2)
#define REG_PD_ALERT_MSK_H_pd_msk_vbus_snk_disc_START (3)
#define REG_PD_ALERT_MSK_H_pd_msk_vbus_snk_disc_END (3)
#define REG_PD_ALERT_MSK_H_pd_msk_fr_swap_START (7)
#define REG_PD_ALERT_MSK_H_pd_msk_fr_swap_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char pd_msk_snkvbus : 1;
        unsigned char pd_msk_vconn_vld : 1;
        unsigned char pd_msk_vbus_vld : 1;
        unsigned char pd_msk_vbus_vld_en : 1;
        unsigned char pd_msk_src_vbus : 1;
        unsigned char reserved : 1;
        unsigned char pd_msk_init_stat : 1;
        unsigned char pd_msk_debug_acc : 1;
    } reg;
} REG_PD_PWRSTAT_MSK_UNION;
#endif
#define REG_PD_PWRSTAT_MSK_pd_msk_snkvbus_START (0)
#define REG_PD_PWRSTAT_MSK_pd_msk_snkvbus_END (0)
#define REG_PD_PWRSTAT_MSK_pd_msk_vconn_vld_START (1)
#define REG_PD_PWRSTAT_MSK_pd_msk_vconn_vld_END (1)
#define REG_PD_PWRSTAT_MSK_pd_msk_vbus_vld_START (2)
#define REG_PD_PWRSTAT_MSK_pd_msk_vbus_vld_END (2)
#define REG_PD_PWRSTAT_MSK_pd_msk_vbus_vld_en_START (3)
#define REG_PD_PWRSTAT_MSK_pd_msk_vbus_vld_en_END (3)
#define REG_PD_PWRSTAT_MSK_pd_msk_src_vbus_START (4)
#define REG_PD_PWRSTAT_MSK_pd_msk_src_vbus_END (4)
#define REG_PD_PWRSTAT_MSK_pd_msk_init_stat_START (6)
#define REG_PD_PWRSTAT_MSK_pd_msk_init_stat_END (6)
#define REG_PD_PWRSTAT_MSK_pd_msk_debug_acc_START (7)
#define REG_PD_PWRSTAT_MSK_pd_msk_debug_acc_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char pd_msk_i2c_err : 1;
        unsigned char pd_msk_cc_fault : 1;
        unsigned char pd_msk_vbus_ovp : 1;
        unsigned char reserved_0 : 1;
        unsigned char pd_msk_force_disch_fail : 1;
        unsigned char pd_msk_auto_disch_fail : 1;
        unsigned char reserved_1 : 2;
    } reg;
} REG_PD_FAULTSTAT_MSK_UNION;
#endif
#define REG_PD_FAULTSTAT_MSK_pd_msk_i2c_err_START (0)
#define REG_PD_FAULTSTAT_MSK_pd_msk_i2c_err_END (0)
#define REG_PD_FAULTSTAT_MSK_pd_msk_cc_fault_START (1)
#define REG_PD_FAULTSTAT_MSK_pd_msk_cc_fault_END (1)
#define REG_PD_FAULTSTAT_MSK_pd_msk_vbus_ovp_START (2)
#define REG_PD_FAULTSTAT_MSK_pd_msk_vbus_ovp_END (2)
#define REG_PD_FAULTSTAT_MSK_pd_msk_force_disch_fail_START (4)
#define REG_PD_FAULTSTAT_MSK_pd_msk_force_disch_fail_END (4)
#define REG_PD_FAULTSTAT_MSK_pd_msk_auto_disch_fail_START (5)
#define REG_PD_FAULTSTAT_MSK_pd_msk_auto_disch_fail_END (5)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char pd_msk_vbus_vsafe0 : 1;
        unsigned char reserved : 7;
    } reg;
} REG_EXTENDED_STATUS_MASK_UNION;
#endif
#define REG_EXTENDED_STATUS_MASK_pd_msk_vbus_vsafe0_START (0)
#define REG_EXTENDED_STATUS_MASK_pd_msk_vbus_vsafe0_END (0)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char pd_msk_snk_rec_frs : 1;
        unsigned char pd_msk_src_rec_frs : 1;
        unsigned char reserved : 6;
    } reg;
} REG_ALERT_EXTENDED_MASK_UNION;
#endif
#define REG_ALERT_EXTENDED_MASK_pd_msk_snk_rec_frs_START (0)
#define REG_ALERT_EXTENDED_MASK_pd_msk_snk_rec_frs_END (0)
#define REG_ALERT_EXTENDED_MASK_pd_msk_src_rec_frs_START (1)
#define REG_ALERT_EXTENDED_MASK_pd_msk_src_rec_frs_END (1)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char pd_orient : 1;
        unsigned char pd_bist_mode : 1;
        unsigned char reserved_0 : 2;
        unsigned char pd_debug_acc_ctrl : 1;
        unsigned char reserved_1 : 1;
        unsigned char pd_en_look4con_alert : 1;
        unsigned char reserved_2 : 1;
    } reg;
} REG_PD_TCPC_CTRL_UNION;
#endif
#define REG_PD_TCPC_CTRL_pd_orient_START (0)
#define REG_PD_TCPC_CTRL_pd_orient_END (0)
#define REG_PD_TCPC_CTRL_pd_bist_mode_START (1)
#define REG_PD_TCPC_CTRL_pd_bist_mode_END (1)
#define REG_PD_TCPC_CTRL_pd_debug_acc_ctrl_START (4)
#define REG_PD_TCPC_CTRL_pd_debug_acc_ctrl_END (4)
#define REG_PD_TCPC_CTRL_pd_en_look4con_alert_START (6)
#define REG_PD_TCPC_CTRL_pd_en_look4con_alert_END (6)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char pd_cc1_cfg : 2;
        unsigned char pd_cc2_cfg : 2;
        unsigned char pd_rp_cfg : 2;
        unsigned char pd_drp : 1;
        unsigned char reserved : 1;
    } reg;
} REG_PD_ROLE_CTRL_UNION;
#endif
#define REG_PD_ROLE_CTRL_pd_cc1_cfg_START (0)
#define REG_PD_ROLE_CTRL_pd_cc1_cfg_END (1)
#define REG_PD_ROLE_CTRL_pd_cc2_cfg_START (2)
#define REG_PD_ROLE_CTRL_pd_cc2_cfg_END (3)
#define REG_PD_ROLE_CTRL_pd_rp_cfg_START (4)
#define REG_PD_ROLE_CTRL_pd_rp_cfg_END (5)
#define REG_PD_ROLE_CTRL_pd_drp_START (6)
#define REG_PD_ROLE_CTRL_pd_drp_END (6)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char pd_vconn_ocp_en : 1;
        unsigned char reserved_0 : 2;
        unsigned char pd_disch_timer_dis : 1;
        unsigned char reserved_1 : 4;
    } reg;
} REG_PD_FAULT_CTRL_UNION;
#endif
#define REG_PD_FAULT_CTRL_pd_vconn_ocp_en_START (0)
#define REG_PD_FAULT_CTRL_pd_vconn_ocp_en_END (0)
#define REG_PD_FAULT_CTRL_pd_disch_timer_dis_START (3)
#define REG_PD_FAULT_CTRL_pd_disch_timer_dis_END (3)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char pd_vconn_en : 1;
        unsigned char reserved_0 : 1;
        unsigned char pd_force_disch_en : 1;
        unsigned char pd_bleed_disch_en : 1;
        unsigned char pd_auto_disch : 1;
        unsigned char pd_valarm_dis : 1;
        unsigned char pd_vbus_mon_dis : 1;
        unsigned char reserved_1 : 1;
    } reg;
} REG_PD_PWR_CTRL_UNION;
#endif
#define REG_PD_PWR_CTRL_pd_vconn_en_START (0)
#define REG_PD_PWR_CTRL_pd_vconn_en_END (0)
#define REG_PD_PWR_CTRL_pd_force_disch_en_START (2)
#define REG_PD_PWR_CTRL_pd_force_disch_en_END (2)
#define REG_PD_PWR_CTRL_pd_bleed_disch_en_START (3)
#define REG_PD_PWR_CTRL_pd_bleed_disch_en_END (3)
#define REG_PD_PWR_CTRL_pd_auto_disch_START (4)
#define REG_PD_PWR_CTRL_pd_auto_disch_END (4)
#define REG_PD_PWR_CTRL_pd_valarm_dis_START (5)
#define REG_PD_PWR_CTRL_pd_valarm_dis_END (5)
#define REG_PD_PWR_CTRL_pd_vbus_mon_dis_START (6)
#define REG_PD_PWR_CTRL_pd_vbus_mon_dis_END (6)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char pd_cc1_stat : 2;
        unsigned char pd_cc2_stat : 2;
        unsigned char pd_con_result : 1;
        unsigned char pd_look4con : 1;
        unsigned char reserved : 2;
    } reg;
} REG_PD_CC_STATUS_UNION;
#endif
#define REG_PD_CC_STATUS_pd_cc1_stat_START (0)
#define REG_PD_CC_STATUS_pd_cc1_stat_END (1)
#define REG_PD_CC_STATUS_pd_cc2_stat_START (2)
#define REG_PD_CC_STATUS_pd_cc2_stat_END (3)
#define REG_PD_CC_STATUS_pd_con_result_START (4)
#define REG_PD_CC_STATUS_pd_con_result_END (4)
#define REG_PD_CC_STATUS_pd_look4con_START (5)
#define REG_PD_CC_STATUS_pd_look4con_END (5)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char pd_sinking_vbus : 1;
        unsigned char pd_vconn_present : 1;
        unsigned char pd_vbus_present : 1;
        unsigned char pd_vbus_pres_detect_en : 1;
        unsigned char pd_source_vbus : 1;
        unsigned char reserved : 1;
        unsigned char pd_tcpc_init_stat : 1;
        unsigned char pd_debug_acc_connect : 1;
    } reg;
} REG_PD_PWR_STATUS_UNION;
#endif
#define REG_PD_PWR_STATUS_pd_sinking_vbus_START (0)
#define REG_PD_PWR_STATUS_pd_sinking_vbus_END (0)
#define REG_PD_PWR_STATUS_pd_vconn_present_START (1)
#define REG_PD_PWR_STATUS_pd_vconn_present_END (1)
#define REG_PD_PWR_STATUS_pd_vbus_present_START (2)
#define REG_PD_PWR_STATUS_pd_vbus_present_END (2)
#define REG_PD_PWR_STATUS_pd_vbus_pres_detect_en_START (3)
#define REG_PD_PWR_STATUS_pd_vbus_pres_detect_en_END (3)
#define REG_PD_PWR_STATUS_pd_source_vbus_START (4)
#define REG_PD_PWR_STATUS_pd_source_vbus_END (4)
#define REG_PD_PWR_STATUS_pd_tcpc_init_stat_START (6)
#define REG_PD_PWR_STATUS_pd_tcpc_init_stat_END (6)
#define REG_PD_PWR_STATUS_pd_debug_acc_connect_START (7)
#define REG_PD_PWR_STATUS_pd_debug_acc_connect_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char pd_i2c_err : 1;
        unsigned char pd_cc_fault : 1;
        unsigned char pd_vusb_ovp : 1;
        unsigned char reserved_0 : 1;
        unsigned char pd_force_disch_fail : 1;
        unsigned char pd_auto_disch_fail : 1;
        unsigned char reserved_1 : 1;
        unsigned char pd_reg_reset_default : 1;
    } reg;
} REG_PD_FAULT_STATUS_UNION;
#endif
#define REG_PD_FAULT_STATUS_pd_i2c_err_START (0)
#define REG_PD_FAULT_STATUS_pd_i2c_err_END (0)
#define REG_PD_FAULT_STATUS_pd_cc_fault_START (1)
#define REG_PD_FAULT_STATUS_pd_cc_fault_END (1)
#define REG_PD_FAULT_STATUS_pd_vusb_ovp_START (2)
#define REG_PD_FAULT_STATUS_pd_vusb_ovp_END (2)
#define REG_PD_FAULT_STATUS_pd_force_disch_fail_START (4)
#define REG_PD_FAULT_STATUS_pd_force_disch_fail_END (4)
#define REG_PD_FAULT_STATUS_pd_auto_disch_fail_START (5)
#define REG_PD_FAULT_STATUS_pd_auto_disch_fail_END (5)
#define REG_PD_FAULT_STATUS_pd_reg_reset_default_START (7)
#define REG_PD_FAULT_STATUS_pd_reg_reset_default_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char pd_vbus_vsafe0 : 1;
        unsigned char reserved : 7;
    } reg;
} REG_EXTENDED_STATUS_UNION;
#endif
#define REG_EXTENDED_STATUS_pd_vbus_vsafe0_START (0)
#define REG_EXTENDED_STATUS_pd_vbus_vsafe0_END (0)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char pd_snk_rec_frs : 1;
        unsigned char pd_src_rec_frs : 1;
        unsigned char pd_timer_expired : 1;
        unsigned char reserved : 5;
    } reg;
} REG_ALERT_EXTENDED_UNION;
#endif
#define REG_ALERT_EXTENDED_pd_snk_rec_frs_START (0)
#define REG_ALERT_EXTENDED_pd_snk_rec_frs_END (0)
#define REG_ALERT_EXTENDED_pd_src_rec_frs_START (1)
#define REG_ALERT_EXTENDED_pd_src_rec_frs_END (1)
#define REG_ALERT_EXTENDED_pd_timer_expired_START (2)
#define REG_ALERT_EXTENDED_pd_timer_expired_END (2)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char pd_command : 8;
    } reg;
} REG_PD_COMMAND_UNION;
#endif
#define REG_PD_COMMAND_pd_command_START (0)
#define REG_PD_COMMAND_pd_command_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char pd_dev_cap1_l : 8;
    } reg;
} REG_PD_DEVCAP1L_UNION;
#endif
#define REG_PD_DEVCAP1L_pd_dev_cap1_l_START (0)
#define REG_PD_DEVCAP1L_pd_dev_cap1_l_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char pd_dev_cap1_h : 8;
    } reg;
} REG_PD_DEVCAP1H_UNION;
#endif
#define REG_PD_DEVCAP1H_pd_dev_cap1_h_START (0)
#define REG_PD_DEVCAP1H_pd_dev_cap1_h_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char pd_dev_cap2_l : 8;
    } reg;
} REG_PD_DEVCAP2L_UNION;
#endif
#define REG_PD_DEVCAP2L_pd_dev_cap2_l_START (0)
#define REG_PD_DEVCAP2L_pd_dev_cap2_l_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char pd_dev_cap2_h : 8;
    } reg;
} REG_PD_DEVCAP2H_UNION;
#endif
#define REG_PD_DEVCAP2H_pd_dev_cap2_h_START (0)
#define REG_PD_DEVCAP2H_pd_dev_cap2_h_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char pd_stdin_cap : 8;
    } reg;
} REG_PD_STDIN_CAP_UNION;
#endif
#define REG_PD_STDIN_CAP_pd_stdin_cap_START (0)
#define REG_PD_STDIN_CAP_pd_stdin_cap_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char pd_stdout_cap : 8;
    } reg;
} REG_PD_STDOUT_CAP_UNION;
#endif
#define REG_PD_STDOUT_CAP_pd_stdout_cap_START (0)
#define REG_PD_STDOUT_CAP_pd_stdout_cap_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char pd_msg_header : 8;
    } reg;
} REG_PD_MSG_HEADR_UNION;
#endif
#define REG_PD_MSG_HEADR_pd_msg_header_START (0)
#define REG_PD_MSG_HEADR_pd_msg_header_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char pd_en_sop : 1;
        unsigned char pd_en_sop1 : 1;
        unsigned char pd_en_sop2 : 1;
        unsigned char pd_en_sop1_debug : 1;
        unsigned char pd_en_sop2_debug : 1;
        unsigned char pd_hard_rst : 1;
        unsigned char pd_cable_rst : 1;
        unsigned char reserved : 1;
    } reg;
} REG_PD_RXDETECT_UNION;
#endif
#define REG_PD_RXDETECT_pd_en_sop_START (0)
#define REG_PD_RXDETECT_pd_en_sop_END (0)
#define REG_PD_RXDETECT_pd_en_sop1_START (1)
#define REG_PD_RXDETECT_pd_en_sop1_END (1)
#define REG_PD_RXDETECT_pd_en_sop2_START (2)
#define REG_PD_RXDETECT_pd_en_sop2_END (2)
#define REG_PD_RXDETECT_pd_en_sop1_debug_START (3)
#define REG_PD_RXDETECT_pd_en_sop1_debug_END (3)
#define REG_PD_RXDETECT_pd_en_sop2_debug_START (4)
#define REG_PD_RXDETECT_pd_en_sop2_debug_END (4)
#define REG_PD_RXDETECT_pd_hard_rst_START (5)
#define REG_PD_RXDETECT_pd_hard_rst_END (5)
#define REG_PD_RXDETECT_pd_cable_rst_START (6)
#define REG_PD_RXDETECT_pd_cable_rst_END (6)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char pd_rx_bytecnt : 8;
    } reg;
} REG_PD_RXBYTECNT_UNION;
#endif
#define REG_PD_RXBYTECNT_pd_rx_bytecnt_START (0)
#define REG_PD_RXBYTECNT_pd_rx_bytecnt_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char pd_rx_type : 3;
        unsigned char reserved : 5;
    } reg;
} REG_PD_RXTYPE_UNION;
#endif
#define REG_PD_RXTYPE_pd_rx_type_START (0)
#define REG_PD_RXTYPE_pd_rx_type_END (2)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char pd_rx_head_l : 8;
    } reg;
} REG_PD_RXHEADL_UNION;
#endif
#define REG_PD_RXHEADL_pd_rx_head_l_START (0)
#define REG_PD_RXHEADL_pd_rx_head_l_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char pd_rx_head_h : 8;
    } reg;
} REG_PD_RXHEADH_UNION;
#endif
#define REG_PD_RXHEADH_pd_rx_head_h_START (0)
#define REG_PD_RXHEADH_pd_rx_head_h_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char pd_rx_data : 8;
    } reg;
} REG_PD_RXDATA_0_UNION;
#endif
#define REG_PD_RXDATA_0_pd_rx_data_START (0)
#define REG_PD_RXDATA_0_pd_rx_data_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char pd_transmit : 3;
        unsigned char reserved_0 : 1;
        unsigned char pd_retry_cnt : 2;
        unsigned char reserved_1 : 2;
    } reg;
} REG_PD_TRANSMIT_UNION;
#endif
#define REG_PD_TRANSMIT_pd_transmit_START (0)
#define REG_PD_TRANSMIT_pd_transmit_END (2)
#define REG_PD_TRANSMIT_pd_retry_cnt_START (4)
#define REG_PD_TRANSMIT_pd_retry_cnt_END (5)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char pd_tx_bytecnt : 8;
    } reg;
} REG_PD_TXBYTECNT_UNION;
#endif
#define REG_PD_TXBYTECNT_pd_tx_bytecnt_START (0)
#define REG_PD_TXBYTECNT_pd_tx_bytecnt_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char pd_tx_head_l : 8;
    } reg;
} REG_PD_TXHEADL_UNION;
#endif
#define REG_PD_TXHEADL_pd_tx_head_l_START (0)
#define REG_PD_TXHEADL_pd_tx_head_l_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char pd_tx_head_h : 8;
    } reg;
} REG_PD_TXHEADH_UNION;
#endif
#define REG_PD_TXHEADH_pd_tx_head_h_START (0)
#define REG_PD_TXHEADH_pd_tx_head_h_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char pd_tx_data : 8;
    } reg;
} REG_PD_TXDATA_UNION;
#endif
#define REG_PD_TXDATA_pd_tx_data_START (0)
#define REG_PD_TXDATA_pd_tx_data_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char pd_vbus_vol_l : 8;
    } reg;
} REG_PD_VBUS_VOL_L_UNION;
#endif
#define REG_PD_VBUS_VOL_L_pd_vbus_vol_l_START (0)
#define REG_PD_VBUS_VOL_L_pd_vbus_vol_l_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char pd_vbus_vol_h : 8;
    } reg;
} REG_PD_VBUS_VOL_H_UNION;
#endif
#define REG_PD_VBUS_VOL_H_pd_vbus_vol_h_START (0)
#define REG_PD_VBUS_VOL_H_pd_vbus_vol_h_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char pd_vbus_snk_disc_l : 8;
    } reg;
} REG_PD_VBUS_SNK_DISCL_UNION;
#endif
#define REG_PD_VBUS_SNK_DISCL_pd_vbus_snk_disc_l_START (0)
#define REG_PD_VBUS_SNK_DISCL_pd_vbus_snk_disc_l_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char pd_vbus_snk_disc_h : 8;
    } reg;
} REG_PD_VBUS_SNK_DISCH_UNION;
#endif
#define REG_PD_VBUS_SNK_DISCH_pd_vbus_snk_disc_h_START (0)
#define REG_PD_VBUS_SNK_DISCH_pd_vbus_snk_disc_h_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char pd_vbus_stop_disc_l : 8;
    } reg;
} REG_PD_VBUS_STOP_DISCL_UNION;
#endif
#define REG_PD_VBUS_STOP_DISCL_pd_vbus_stop_disc_l_START (0)
#define REG_PD_VBUS_STOP_DISCL_pd_vbus_stop_disc_l_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char pd_vbus_stop_disc_h : 8;
    } reg;
} REG_PD_VBUS_STOP_DISCH_UNION;
#endif
#define REG_PD_VBUS_STOP_DISCH_pd_vbus_stop_disc_h_START (0)
#define REG_PD_VBUS_STOP_DISCH_pd_vbus_stop_disc_h_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char pd_valarm_high_l : 8;
    } reg;
} REG_PD_VALARMH_CFGL_UNION;
#endif
#define REG_PD_VALARMH_CFGL_pd_valarm_high_l_START (0)
#define REG_PD_VALARMH_CFGL_pd_valarm_high_l_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char pd_valarm_high_h : 8;
    } reg;
} REG_PD_VALARMH_CFGH_UNION;
#endif
#define REG_PD_VALARMH_CFGH_pd_valarm_high_h_START (0)
#define REG_PD_VALARMH_CFGH_pd_valarm_high_h_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char pd_valarm_low_l : 8;
    } reg;
} REG_PD_VALARML_CFGL_UNION;
#endif
#define REG_PD_VALARML_CFGL_pd_valarm_low_l_START (0)
#define REG_PD_VALARML_CFGL_pd_valarm_low_l_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char pd_valarm_low_h : 8;
    } reg;
} REG_PD_VALARML_CFGH_UNION;
#endif
#define REG_PD_VALARML_CFGH_pd_valarm_low_h_START (0)
#define REG_PD_VALARML_CFGH_pd_valarm_low_h_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char da_frs_en : 1;
        unsigned char da_vconn_dis_en : 1;
        unsigned char pd_drp_thres : 2;
        unsigned char pd_try_snk_en : 1;
        unsigned char da_force_unplug_en : 1;
        unsigned char pd_bmc_cdr_sel : 1;
        unsigned char pd_cc_orient_sel : 1;
    } reg;
} REG_PD_VDM_CFG_0_UNION;
#endif
#define REG_PD_VDM_CFG_0_da_frs_en_START (0)
#define REG_PD_VDM_CFG_0_da_frs_en_END (0)
#define REG_PD_VDM_CFG_0_da_vconn_dis_en_START (1)
#define REG_PD_VDM_CFG_0_da_vconn_dis_en_END (1)
#define REG_PD_VDM_CFG_0_pd_drp_thres_START (2)
#define REG_PD_VDM_CFG_0_pd_drp_thres_END (3)
#define REG_PD_VDM_CFG_0_pd_try_snk_en_START (4)
#define REG_PD_VDM_CFG_0_pd_try_snk_en_END (4)
#define REG_PD_VDM_CFG_0_da_force_unplug_en_START (5)
#define REG_PD_VDM_CFG_0_da_force_unplug_en_END (5)
#define REG_PD_VDM_CFG_0_pd_bmc_cdr_sel_START (6)
#define REG_PD_VDM_CFG_0_pd_bmc_cdr_sel_END (6)
#define REG_PD_VDM_CFG_0_pd_cc_orient_sel_START (7)
#define REG_PD_VDM_CFG_0_pd_cc_orient_sel_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char enable_pd : 1;
        unsigned char reserved : 7;
    } reg;
} REG_PD_VDM_ENABLE_UNION;
#endif
#define REG_PD_VDM_ENABLE_enable_pd_START (0)
#define REG_PD_VDM_ENABLE_enable_pd_END (0)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char tc_fsm_reset : 1;
        unsigned char pd_fsm_reset : 1;
        unsigned char pd_tx_phy_soft_reset : 1;
        unsigned char pd_rx_phy_soft_reset : 1;
        unsigned char pd_snk_disc_by_cc : 1;
        unsigned char da_new_vconn_dis_en : 1;
        unsigned char reserved : 2;
    } reg;
} REG_PD_VDM_CFG_1_UNION;
#endif
#define REG_PD_VDM_CFG_1_tc_fsm_reset_START (0)
#define REG_PD_VDM_CFG_1_tc_fsm_reset_END (0)
#define REG_PD_VDM_CFG_1_pd_fsm_reset_START (1)
#define REG_PD_VDM_CFG_1_pd_fsm_reset_END (1)
#define REG_PD_VDM_CFG_1_pd_tx_phy_soft_reset_START (2)
#define REG_PD_VDM_CFG_1_pd_tx_phy_soft_reset_END (2)
#define REG_PD_VDM_CFG_1_pd_rx_phy_soft_reset_START (3)
#define REG_PD_VDM_CFG_1_pd_rx_phy_soft_reset_END (3)
#define REG_PD_VDM_CFG_1_pd_snk_disc_by_cc_START (4)
#define REG_PD_VDM_CFG_1_pd_snk_disc_by_cc_END (4)
#define REG_PD_VDM_CFG_1_da_new_vconn_dis_en_START (5)
#define REG_PD_VDM_CFG_1_da_new_vconn_dis_en_END (5)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char pd_dbg_rdata_sel : 4;
        unsigned char pd_dbg_module_sel : 3;
        unsigned char pd_dbg_rdata_en : 1;
    } reg;
} REG_PD_DBG_RDATA_CFG_UNION;
#endif
#define REG_PD_DBG_RDATA_CFG_pd_dbg_rdata_sel_START (0)
#define REG_PD_DBG_RDATA_CFG_pd_dbg_rdata_sel_END (3)
#define REG_PD_DBG_RDATA_CFG_pd_dbg_module_sel_START (4)
#define REG_PD_DBG_RDATA_CFG_pd_dbg_module_sel_END (6)
#define REG_PD_DBG_RDATA_CFG_pd_dbg_rdata_en_START (7)
#define REG_PD_DBG_RDATA_CFG_pd_dbg_rdata_en_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char pd_dbg_rdata : 8;
    } reg;
} REG_PD_DBG_RDATA_UNION;
#endif
#define REG_PD_DBG_RDATA_pd_dbg_rdata_START (0)
#define REG_PD_DBG_RDATA_pd_dbg_rdata_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char attach : 1;
        unsigned char reserved : 5;
        unsigned char dvc : 2;
    } reg;
} REG_STATUIS_UNION;
#endif
#define REG_STATUIS_attach_START (0)
#define REG_STATUIS_attach_END (0)
#define REG_STATUIS_dvc_START (6)
#define REG_STATUIS_dvc_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char sndcmd : 1;
        unsigned char reserved_0: 1;
        unsigned char mstr_rst : 1;
        unsigned char enable : 1;
        unsigned char reserved_1: 4;
    } reg;
} REG_CNTL_UNION;
#endif
#define REG_CNTL_sndcmd_START (0)
#define REG_CNTL_sndcmd_END (0)
#define REG_CNTL_mstr_rst_START (2)
#define REG_CNTL_mstr_rst_END (2)
#define REG_CNTL_enable_START (3)
#define REG_CNTL_enable_END (3)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char cmd : 8;
    } reg;
} REG_CMD_UNION;
#endif
#define REG_CMD_cmd_START (0)
#define REG_CMD_cmd_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char fcp_length : 4;
        unsigned char reserved : 4;
    } reg;
} REG_FCP_LENGTH_UNION;
#endif
#define REG_FCP_LENGTH_fcp_length_START (0)
#define REG_FCP_LENGTH_fcp_length_END (3)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char addr : 8;
    } reg;
} REG_ADDR_UNION;
#endif
#define REG_ADDR_addr_START (0)
#define REG_ADDR_addr_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char data0 : 8;
    } reg;
} REG_DATA0_UNION;
#endif
#define REG_DATA0_data0_START (0)
#define REG_DATA0_data0_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char data1 : 8;
    } reg;
} REG_DATA1_UNION;
#endif
#define REG_DATA1_data1_START (0)
#define REG_DATA1_data1_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char data2 : 8;
    } reg;
} REG_DATA2_UNION;
#endif
#define REG_DATA2_data2_START (0)
#define REG_DATA2_data2_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char data3 : 8;
    } reg;
} REG_DATA3_UNION;
#endif
#define REG_DATA3_data3_START (0)
#define REG_DATA3_data3_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char data4 : 8;
    } reg;
} REG_DATA4_UNION;
#endif
#define REG_DATA4_data4_START (0)
#define REG_DATA4_data4_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char data5 : 8;
    } reg;
} REG_DATA5_UNION;
#endif
#define REG_DATA5_data5_START (0)
#define REG_DATA5_data5_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char data6 : 8;
    } reg;
} REG_DATA6_UNION;
#endif
#define REG_DATA6_data6_START (0)
#define REG_DATA6_data6_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char data7 : 8;
    } reg;
} REG_DATA7_UNION;
#endif
#define REG_DATA7_data7_START (0)
#define REG_DATA7_data7_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char fcp_rdata0 : 8;
    } reg;
} REG_FCP_RDATA0_UNION;
#endif
#define REG_FCP_RDATA0_fcp_rdata0_START (0)
#define REG_FCP_RDATA0_fcp_rdata0_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char fcp_rdata1 : 8;
    } reg;
} REG_FCP_RDATA1_UNION;
#endif
#define REG_FCP_RDATA1_fcp_rdata1_START (0)
#define REG_FCP_RDATA1_fcp_rdata1_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char fcp_rdata2 : 8;
    } reg;
} REG_FCP_RDATA2_UNION;
#endif
#define REG_FCP_RDATA2_fcp_rdata2_START (0)
#define REG_FCP_RDATA2_fcp_rdata2_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char fcp_rdata3 : 8;
    } reg;
} REG_FCP_RDATA3_UNION;
#endif
#define REG_FCP_RDATA3_fcp_rdata3_START (0)
#define REG_FCP_RDATA3_fcp_rdata3_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char fcp_rdata4 : 8;
    } reg;
} REG_FCP_RDATA4_UNION;
#endif
#define REG_FCP_RDATA4_fcp_rdata4_START (0)
#define REG_FCP_RDATA4_fcp_rdata4_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char fcp_rdata5 : 8;
    } reg;
} REG_FCP_RDATA5_UNION;
#endif
#define REG_FCP_RDATA5_fcp_rdata5_START (0)
#define REG_FCP_RDATA5_fcp_rdata5_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char fcp_rdata6 : 8;
    } reg;
} REG_FCP_RDATA6_UNION;
#endif
#define REG_FCP_RDATA6_fcp_rdata6_START (0)
#define REG_FCP_RDATA6_fcp_rdata6_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char fcp_rdata7 : 8;
    } reg;
} REG_FCP_RDATA7_UNION;
#endif
#define REG_FCP_RDATA7_fcp_rdata7_START (0)
#define REG_FCP_RDATA7_fcp_rdata7_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char reserved_0 : 1;
        unsigned char nack_alarm : 1;
        unsigned char ack_alarm : 1;
        unsigned char crcpar : 1;
        unsigned char nack : 1;
        unsigned char reserved_1 : 1;
        unsigned char ack : 1;
        unsigned char cmdcpl : 1;
    } reg;
} REG_ISR1_UNION;
#endif
#define REG_ISR1_nack_alarm_START (1)
#define REG_ISR1_nack_alarm_END (1)
#define REG_ISR1_ack_alarm_START (2)
#define REG_ISR1_ack_alarm_END (2)
#define REG_ISR1_crcpar_START (3)
#define REG_ISR1_crcpar_END (3)
#define REG_ISR1_nack_START (4)
#define REG_ISR1_nack_END (4)
#define REG_ISR1_ack_START (6)
#define REG_ISR1_ack_END (6)
#define REG_ISR1_cmdcpl_START (7)
#define REG_ISR1_cmdcpl_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char reserved_0: 1;
        unsigned char protstat : 1;
        unsigned char reserved_1: 1;
        unsigned char parrx : 1;
        unsigned char crcrx : 1;
        unsigned char reserved_2: 3;
    } reg;
} REG_ISR2_UNION;
#endif
#define REG_ISR2_protstat_START (1)
#define REG_ISR2_protstat_END (1)
#define REG_ISR2_parrx_START (3)
#define REG_ISR2_parrx_END (3)
#define REG_ISR2_crcrx_START (4)
#define REG_ISR2_crcrx_END (4)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char reserved_0 : 1;
        unsigned char nack_alarm_mk : 1;
        unsigned char ack_alarm_mk : 1;
        unsigned char crcpar_mk : 1;
        unsigned char nack_mk : 1;
        unsigned char reserved_1 : 1;
        unsigned char ack_mk : 1;
        unsigned char cmdcpl_mk : 1;
    } reg;
} REG_IMR1_UNION;
#endif
#define REG_IMR1_nack_alarm_mk_START (1)
#define REG_IMR1_nack_alarm_mk_END (1)
#define REG_IMR1_ack_alarm_mk_START (2)
#define REG_IMR1_ack_alarm_mk_END (2)
#define REG_IMR1_crcpar_mk_START (3)
#define REG_IMR1_crcpar_mk_END (3)
#define REG_IMR1_nack_mk_START (4)
#define REG_IMR1_nack_mk_END (4)
#define REG_IMR1_ack_mk_START (6)
#define REG_IMR1_ack_mk_END (6)
#define REG_IMR1_cmdcpl_mk_START (7)
#define REG_IMR1_cmdcpl_mk_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char reserved_0 : 1;
        unsigned char protstat_mk : 1;
        unsigned char reserved_1 : 1;
        unsigned char parrx_mk : 1;
        unsigned char crcrx_mk : 1;
        unsigned char reserved_2 : 3;
    } reg;
} REG_IMR2_UNION;
#endif
#define REG_IMR2_protstat_mk_START (1)
#define REG_IMR2_protstat_mk_END (1)
#define REG_IMR2_parrx_mk_START (3)
#define REG_IMR2_parrx_mk_END (3)
#define REG_IMR2_crcrx_mk_START (4)
#define REG_IMR2_crcrx_mk_END (4)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char fcp_set_d60m_int : 1;
        unsigned char fcp_en_det_int : 1;
        unsigned char reserved : 6;
    } reg;
} REG_FCP_IRQ5_UNION;
#endif
#define REG_FCP_IRQ5_fcp_set_d60m_int_START (0)
#define REG_FCP_IRQ5_fcp_set_d60m_int_END (0)
#define REG_FCP_IRQ5_fcp_en_det_int_START (1)
#define REG_FCP_IRQ5_fcp_en_det_int_END (1)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char fcp_set_d60m_r_mk : 1;
        unsigned char fcp_en_det_int_mk : 1;
        unsigned char reserved : 6;
    } reg;
} REG_FCP_IRQ5_MASK_UNION;
#endif
#define REG_FCP_IRQ5_MASK_fcp_set_d60m_r_mk_START (0)
#define REG_FCP_IRQ5_MASK_fcp_set_d60m_r_mk_END (0)
#define REG_FCP_IRQ5_MASK_fcp_en_det_int_mk_START (1)
#define REG_FCP_IRQ5_MASK_fcp_en_det_int_mk_END (1)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char fcp_clk_test : 1;
        unsigned char fcp_mode : 1;
        unsigned char fcp_cmp_en : 1;
        unsigned char fcp_det_en : 1;
        unsigned char reserved : 4;
    } reg;
} REG_FCP_CTRL_UNION;
#endif
#define REG_FCP_CTRL_fcp_clk_test_START (0)
#define REG_FCP_CTRL_fcp_clk_test_END (0)
#define REG_FCP_CTRL_fcp_mode_START (1)
#define REG_FCP_CTRL_fcp_mode_END (1)
#define REG_FCP_CTRL_fcp_cmp_en_START (2)
#define REG_FCP_CTRL_fcp_cmp_en_END (2)
#define REG_FCP_CTRL_fcp_det_en_START (3)
#define REG_FCP_CTRL_fcp_det_en_END (3)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char fcp_mod2_set : 2;
        unsigned char reserved : 6;
    } reg;
} REG_FCP_MODE2_SET_UNION;
#endif
#define REG_FCP_MODE2_SET_fcp_mod2_set_START (0)
#define REG_FCP_MODE2_SET_fcp_mod2_set_END (1)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char fcp_set_d60m_r : 1;
        unsigned char reserved : 7;
    } reg;
} REG_FCP_ADAP_CTRL_UNION;
#endif
#define REG_FCP_ADAP_CTRL_fcp_set_d60m_r_START (0)
#define REG_FCP_ADAP_CTRL_fcp_set_d60m_r_END (0)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char fcp_rdata_ready : 1;
        unsigned char reserved : 7;
    } reg;
} REG_RDATA_READY_UNION;
#endif
#define REG_RDATA_READY_fcp_rdata_ready_START (0)
#define REG_RDATA_READY_fcp_rdata_ready_END (0)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char fcp_soft_rst_cfg : 8;
    } reg;
} REG_FCP_SOFT_RST_CTRL_UNION;
#endif
#define REG_FCP_SOFT_RST_CTRL_fcp_soft_rst_cfg_START (0)
#define REG_FCP_SOFT_RST_CTRL_fcp_soft_rst_cfg_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char rdata_parity_err0 : 1;
        unsigned char rdata_parity_err1 : 1;
        unsigned char rdata_parity_err2 : 1;
        unsigned char rdata_parity_err3 : 1;
        unsigned char rdata_parity_err4 : 1;
        unsigned char rdata_parity_err5 : 1;
        unsigned char rdata_parity_err6 : 1;
        unsigned char rdata_parity_err7 : 1;
    } reg;
} REG_FCP_RDATA_PARITY_ERR_UNION;
#endif
#define REG_FCP_RDATA_PARITY_ERR_rdata_parity_err0_START (0)
#define REG_FCP_RDATA_PARITY_ERR_rdata_parity_err0_END (0)
#define REG_FCP_RDATA_PARITY_ERR_rdata_parity_err1_START (1)
#define REG_FCP_RDATA_PARITY_ERR_rdata_parity_err1_END (1)
#define REG_FCP_RDATA_PARITY_ERR_rdata_parity_err2_START (2)
#define REG_FCP_RDATA_PARITY_ERR_rdata_parity_err2_END (2)
#define REG_FCP_RDATA_PARITY_ERR_rdata_parity_err3_START (3)
#define REG_FCP_RDATA_PARITY_ERR_rdata_parity_err3_END (3)
#define REG_FCP_RDATA_PARITY_ERR_rdata_parity_err4_START (4)
#define REG_FCP_RDATA_PARITY_ERR_rdata_parity_err4_END (4)
#define REG_FCP_RDATA_PARITY_ERR_rdata_parity_err5_START (5)
#define REG_FCP_RDATA_PARITY_ERR_rdata_parity_err5_END (5)
#define REG_FCP_RDATA_PARITY_ERR_rdata_parity_err6_START (6)
#define REG_FCP_RDATA_PARITY_ERR_rdata_parity_err6_END (6)
#define REG_FCP_RDATA_PARITY_ERR_rdata_parity_err7_START (7)
#define REG_FCP_RDATA_PARITY_ERR_rdata_parity_err7_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char fcp_init_retry_cfg : 5;
        unsigned char reserved : 3;
    } reg;
} REG_FCP_INIT_RETRY_CFG_UNION;
#endif
#define REG_FCP_INIT_RETRY_CFG_fcp_init_retry_cfg_START (0)
#define REG_FCP_INIT_RETRY_CFG_fcp_init_retry_cfg_END (4)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char last_hand_fail_irq : 1;
        unsigned char tail_hand_fail_irq : 1;
        unsigned char trans_hand_fail_irq : 1;
        unsigned char init_hand_fail_irq : 1;
        unsigned char rx_data_det_irq : 1;
        unsigned char rx_head_det_irq : 1;
        unsigned char cmd_err_irq : 1;
        unsigned char length_err_irq : 1;
    } reg;
} REG_FCP_IRQ3_UNION;
#endif
#define REG_FCP_IRQ3_last_hand_fail_irq_START (0)
#define REG_FCP_IRQ3_last_hand_fail_irq_END (0)
#define REG_FCP_IRQ3_tail_hand_fail_irq_START (1)
#define REG_FCP_IRQ3_tail_hand_fail_irq_END (1)
#define REG_FCP_IRQ3_trans_hand_fail_irq_START (2)
#define REG_FCP_IRQ3_trans_hand_fail_irq_END (2)
#define REG_FCP_IRQ3_init_hand_fail_irq_START (3)
#define REG_FCP_IRQ3_init_hand_fail_irq_END (3)
#define REG_FCP_IRQ3_rx_data_det_irq_START (4)
#define REG_FCP_IRQ3_rx_data_det_irq_END (4)
#define REG_FCP_IRQ3_rx_head_det_irq_START (5)
#define REG_FCP_IRQ3_rx_head_det_irq_END (5)
#define REG_FCP_IRQ3_cmd_err_irq_START (6)
#define REG_FCP_IRQ3_cmd_err_irq_END (6)
#define REG_FCP_IRQ3_length_err_irq_START (7)
#define REG_FCP_IRQ3_length_err_irq_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char last_hand_no_respond_irq : 1;
        unsigned char tail_hand_no_respond_irq : 1;
        unsigned char trans_hand_no_respond_irq : 1;
        unsigned char init_hand_no_respond_irq : 1;
        unsigned char enable_hand_fail_irq : 1;
        unsigned char enable_hand_no_respond_irq : 1;
        unsigned char reserved : 2;
    } reg;
} REG_FCP_IRQ4_UNION;
#endif
#define REG_FCP_IRQ4_last_hand_no_respond_irq_START (0)
#define REG_FCP_IRQ4_last_hand_no_respond_irq_END (0)
#define REG_FCP_IRQ4_tail_hand_no_respond_irq_START (1)
#define REG_FCP_IRQ4_tail_hand_no_respond_irq_END (1)
#define REG_FCP_IRQ4_trans_hand_no_respond_irq_START (2)
#define REG_FCP_IRQ4_trans_hand_no_respond_irq_END (2)
#define REG_FCP_IRQ4_init_hand_no_respond_irq_START (3)
#define REG_FCP_IRQ4_init_hand_no_respond_irq_END (3)
#define REG_FCP_IRQ4_enable_hand_fail_irq_START (4)
#define REG_FCP_IRQ4_enable_hand_fail_irq_END (4)
#define REG_FCP_IRQ4_enable_hand_no_respond_irq_START (5)
#define REG_FCP_IRQ4_enable_hand_no_respond_irq_END (5)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char last_hand_fail_irq_mk : 1;
        unsigned char tail_hand_fail_irq_mk : 1;
        unsigned char trans_hand_fail_irq_mk : 1;
        unsigned char init_hand_fail_irq_mk : 1;
        unsigned char rx_data_det_irq_mk : 1;
        unsigned char rx_head_det_irq_mk : 1;
        unsigned char cmd_err_irq_mk : 1;
        unsigned char length_err_irq_mk : 1;
    } reg;
} REG_FCP_IRQ3_MASK_UNION;
#endif
#define REG_FCP_IRQ3_MASK_last_hand_fail_irq_mk_START (0)
#define REG_FCP_IRQ3_MASK_last_hand_fail_irq_mk_END (0)
#define REG_FCP_IRQ3_MASK_tail_hand_fail_irq_mk_START (1)
#define REG_FCP_IRQ3_MASK_tail_hand_fail_irq_mk_END (1)
#define REG_FCP_IRQ3_MASK_trans_hand_fail_irq_mk_START (2)
#define REG_FCP_IRQ3_MASK_trans_hand_fail_irq_mk_END (2)
#define REG_FCP_IRQ3_MASK_init_hand_fail_irq_mk_START (3)
#define REG_FCP_IRQ3_MASK_init_hand_fail_irq_mk_END (3)
#define REG_FCP_IRQ3_MASK_rx_data_det_irq_mk_START (4)
#define REG_FCP_IRQ3_MASK_rx_data_det_irq_mk_END (4)
#define REG_FCP_IRQ3_MASK_rx_head_det_irq_mk_START (5)
#define REG_FCP_IRQ3_MASK_rx_head_det_irq_mk_END (5)
#define REG_FCP_IRQ3_MASK_cmd_err_irq_mk_START (6)
#define REG_FCP_IRQ3_MASK_cmd_err_irq_mk_END (6)
#define REG_FCP_IRQ3_MASK_length_err_irq_mk_START (7)
#define REG_FCP_IRQ3_MASK_length_err_irq_mk_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char last_hand_no_respond_irq_mk : 1;
        unsigned char tail_hand_no_respond_irq_mk : 1;
        unsigned char trans_hand_no_respond_irq_mk : 1;
        unsigned char init_hand_no_respond_irq_mk : 1;
        unsigned char enable_hand_fail_irq_mk : 1;
        unsigned char enable_hand_no_respond_irq_mk : 1;
        unsigned char reserved : 2;
    } reg;
} REG_FCP_IRQ4_MASK_UNION;
#endif
#define REG_FCP_IRQ4_MASK_last_hand_no_respond_irq_mk_START (0)
#define REG_FCP_IRQ4_MASK_last_hand_no_respond_irq_mk_END (0)
#define REG_FCP_IRQ4_MASK_tail_hand_no_respond_irq_mk_START (1)
#define REG_FCP_IRQ4_MASK_tail_hand_no_respond_irq_mk_END (1)
#define REG_FCP_IRQ4_MASK_trans_hand_no_respond_irq_mk_START (2)
#define REG_FCP_IRQ4_MASK_trans_hand_no_respond_irq_mk_END (2)
#define REG_FCP_IRQ4_MASK_init_hand_no_respond_irq_mk_START (3)
#define REG_FCP_IRQ4_MASK_init_hand_no_respond_irq_mk_END (3)
#define REG_FCP_IRQ4_MASK_enable_hand_fail_irq_mk_START (4)
#define REG_FCP_IRQ4_MASK_enable_hand_fail_irq_mk_END (4)
#define REG_FCP_IRQ4_MASK_enable_hand_no_respond_irq_mk_START (5)
#define REG_FCP_IRQ4_MASK_enable_hand_no_respond_irq_mk_END (5)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char mstate : 4;
        unsigned char reserved : 4;
    } reg;
} REG_MSTATE_UNION;
#endif
#define REG_MSTATE_mstate_START (0)
#define REG_MSTATE_mstate_END (3)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char crc_en : 1;
        unsigned char reserved : 7;
    } reg;
} REG_CRC_ENABLE_UNION;
#endif
#define REG_CRC_ENABLE_crc_en_START (0)
#define REG_CRC_ENABLE_crc_en_END (0)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char crc_start_val : 8;
    } reg;
} REG_CRC_START_VALUE_UNION;
#endif
#define REG_CRC_START_VALUE_crc_start_val_START (0)
#define REG_CRC_START_VALUE_crc_start_val_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char sample_cnt_adjust : 5;
        unsigned char reserved : 3;
    } reg;
} REG_SAMPLE_CNT_ADJ_UNION;
#endif
#define REG_SAMPLE_CNT_ADJ_sample_cnt_adjust_START (0)
#define REG_SAMPLE_CNT_ADJ_sample_cnt_adjust_END (4)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char rx_ping_width_min_h : 1;
        unsigned char reserved : 7;
    } reg;
} REG_RX_PING_WIDTH_MIN_H_UNION;
#endif
#define REG_RX_PING_WIDTH_MIN_H_rx_ping_width_min_h_START (0)
#define REG_RX_PING_WIDTH_MIN_H_rx_ping_width_min_h_END (0)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char rx_ping_width_min_l : 8;
    } reg;
} REG_RX_PING_WIDTH_MIN_L_UNION;
#endif
#define REG_RX_PING_WIDTH_MIN_L_rx_ping_width_min_l_START (0)
#define REG_RX_PING_WIDTH_MIN_L_rx_ping_width_min_l_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char rx_ping_width_max_h : 1;
        unsigned char reserved : 7;
    } reg;
} REG_RX_PING_WIDTH_MAX_H_UNION;
#endif
#define REG_RX_PING_WIDTH_MAX_H_rx_ping_width_max_h_START (0)
#define REG_RX_PING_WIDTH_MAX_H_rx_ping_width_max_h_END (0)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char rx_ping_width_max_l : 8;
    } reg;
} REG_RX_PING_WIDTH_MAX_L_UNION;
#endif
#define REG_RX_PING_WIDTH_MAX_L_rx_ping_width_max_l_START (0)
#define REG_RX_PING_WIDTH_MAX_L_rx_ping_width_max_l_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char data_wait_time : 7;
        unsigned char reserved : 1;
    } reg;
} REG_DATA_WAIT_TIME_UNION;
#endif
#define REG_DATA_WAIT_TIME_data_wait_time_START (0)
#define REG_DATA_WAIT_TIME_data_wait_time_END (6)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char cmd_retry_config : 4;
        unsigned char reserved : 4;
    } reg;
} REG_RETRY_CNT_UNION;
#endif
#define REG_RETRY_CNT_cmd_retry_config_START (0)
#define REG_RETRY_CNT_cmd_retry_config_END (3)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char fcp_ro_reserve : 8;
    } reg;
} REG_FCP_RO_RESERVE_UNION;
#endif
#define REG_FCP_RO_RESERVE_fcp_ro_reserve_START (0)
#define REG_FCP_RO_RESERVE_fcp_ro_reserve_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char slv_crc_err : 1;
        unsigned char resp_ack_parity_err : 1;
        unsigned char fcp_head_early_err : 1;
        unsigned char slv_crc_parity_err : 1;
        unsigned char rdata_range_err : 1;
        unsigned char reserved : 3;
    } reg;
} REG_FCP_DEBUG_REG0_UNION;
#endif
#define REG_FCP_DEBUG_REG0_slv_crc_err_START (0)
#define REG_FCP_DEBUG_REG0_slv_crc_err_END (0)
#define REG_FCP_DEBUG_REG0_resp_ack_parity_err_START (1)
#define REG_FCP_DEBUG_REG0_resp_ack_parity_err_END (1)
#define REG_FCP_DEBUG_REG0_fcp_head_early_err_START (2)
#define REG_FCP_DEBUG_REG0_fcp_head_early_err_END (2)
#define REG_FCP_DEBUG_REG0_slv_crc_parity_err_START (3)
#define REG_FCP_DEBUG_REG0_slv_crc_parity_err_END (3)
#define REG_FCP_DEBUG_REG0_rdata_range_err_START (4)
#define REG_FCP_DEBUG_REG0_rdata_range_err_END (4)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char resp_ack : 8;
    } reg;
} REG_FCP_DEBUG_REG1_UNION;
#endif
#define REG_FCP_DEBUG_REG1_resp_ack_START (0)
#define REG_FCP_DEBUG_REG1_resp_ack_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char slv_crc : 8;
    } reg;
} REG_FCP_DEBUG_REG2_UNION;
#endif
#define REG_FCP_DEBUG_REG2_slv_crc_START (0)
#define REG_FCP_DEBUG_REG2_slv_crc_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char rx_ping_cnt_l : 8;
    } reg;
} REG_FCP_DEBUG_REG3_UNION;
#endif
#define REG_FCP_DEBUG_REG3_rx_ping_cnt_l_START (0)
#define REG_FCP_DEBUG_REG3_rx_ping_cnt_l_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char reserved : 2;
        unsigned char rx_ping_low_cnt : 5;
        unsigned char rx_ping_cnt_h : 1;
    } reg;
} REG_FCP_DEBUG_REG4_UNION;
#endif
#define REG_FCP_DEBUG_REG4_rx_ping_low_cnt_START (2)
#define REG_FCP_DEBUG_REG4_rx_ping_low_cnt_END (6)
#define REG_FCP_DEBUG_REG4_rx_ping_cnt_h_START (7)
#define REG_FCP_DEBUG_REG4_rx_ping_cnt_h_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char rx_packet_wait_adjust : 7;
        unsigned char reserved : 1;
    } reg;
} REG_RX_PACKET_WAIT_ADJUST_UNION;
#endif
#define REG_RX_PACKET_WAIT_ADJUST_rx_packet_wait_adjust_START (0)
#define REG_RX_PACKET_WAIT_ADJUST_rx_packet_wait_adjust_END (6)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char sample_cnt_tinyjust : 5;
        unsigned char reserved : 3;
    } reg;
} REG_SAMPLE_CNT_TINYJUST_UNION;
#endif
#define REG_SAMPLE_CNT_TINYJUST_sample_cnt_tinyjust_START (0)
#define REG_SAMPLE_CNT_TINYJUST_sample_cnt_tinyjust_END (4)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char rx_ping_cnt_tinyjust : 5;
        unsigned char reserved : 3;
    } reg;
} REG_RX_PING_CNT_TINYJUST_UNION;
#endif
#define REG_RX_PING_CNT_TINYJUST_rx_ping_cnt_tinyjust_START (0)
#define REG_RX_PING_CNT_TINYJUST_rx_ping_cnt_tinyjust_END (4)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char shift_cnt_cfg_max : 4;
        unsigned char reserved : 4;
    } reg;
} REG_SHIFT_CNT_CFG_MAX_UNION;
#endif
#define REG_SHIFT_CNT_CFG_MAX_shift_cnt_cfg_max_START (0)
#define REG_SHIFT_CNT_CFG_MAX_shift_cnt_cfg_max_END (3)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char pd_cdr_cfg_0 : 8;
    } reg;
} REG_PD_CDR_CFG_0_UNION;
#endif
#define REG_PD_CDR_CFG_0_pd_cdr_cfg_0_START (0)
#define REG_PD_CDR_CFG_0_pd_cdr_cfg_0_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char pd_cdr_cfg_1 : 8;
    } reg;
} REG_PD_CDR_CFG_1_UNION;
#endif
#define REG_PD_CDR_CFG_1_pd_cdr_cfg_1_START (0)
#define REG_PD_CDR_CFG_1_pd_cdr_cfg_1_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char pd_dbg_cfg_0 : 8;
    } reg;
} REG_PD_DBG_CFG_0_UNION;
#endif
#define REG_PD_DBG_CFG_0_pd_dbg_cfg_0_START (0)
#define REG_PD_DBG_CFG_0_pd_dbg_cfg_0_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char pd_dbg_cfg_1 : 8;
    } reg;
} REG_PD_DBG_CFG_1_UNION;
#endif
#define REG_PD_DBG_CFG_1_pd_dbg_cfg_1_START (0)
#define REG_PD_DBG_CFG_1_pd_dbg_cfg_1_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char pd_dbg_cfg_2 : 8;
    } reg;
} REG_PD_DBG_CFG_2_UNION;
#endif
#define REG_PD_DBG_CFG_2_pd_dbg_cfg_2_START (0)
#define REG_PD_DBG_CFG_2_pd_dbg_cfg_2_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char pd_dbg_cfg_3 : 8;
    } reg;
} REG_PD_DBG_CFG_3_UNION;
#endif
#define REG_PD_DBG_CFG_3_pd_dbg_cfg_3_START (0)
#define REG_PD_DBG_CFG_3_pd_dbg_cfg_3_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char pd_dbg_status_0 : 8;
    } reg;
} REG_PD_DBG_RO_0_UNION;
#endif
#define REG_PD_DBG_RO_0_pd_dbg_status_0_START (0)
#define REG_PD_DBG_RO_0_pd_dbg_status_0_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char pd_dbg_status_1 : 8;
    } reg;
} REG_PD_DBG_RO_1_UNION;
#endif
#define REG_PD_DBG_RO_1_pd_dbg_status_1_START (0)
#define REG_PD_DBG_RO_1_pd_dbg_status_1_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char pd_dbg_status_2 : 8;
    } reg;
} REG_PD_DBG_RO_2_UNION;
#endif
#define REG_PD_DBG_RO_2_pd_dbg_status_2_START (0)
#define REG_PD_DBG_RO_2_pd_dbg_status_2_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char pd_dbg_status_3 : 8;
    } reg;
} REG_PD_DBG_RO_3_UNION;
#endif
#define REG_PD_DBG_RO_3_pd_dbg_status_3_START (0)
#define REG_PD_DBG_RO_3_pd_dbg_status_3_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char pd_ext_otg_sel : 1;
        unsigned char reserved : 7;
    } reg;
} REG_PD_EXT_OTG_SEL_UNION;
#endif
#define REG_PD_EXT_OTG_SEL_pd_ext_otg_sel_START (0)
#define REG_PD_EXT_OTG_SEL_pd_ext_otg_sel_END (0)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char pd_ext_otg_en : 1;
        unsigned char reserved : 7;
    } reg;
} REG_PD_EXT_OTG_EN_UNION;
#endif
#define REG_PD_EXT_OTG_EN_pd_ext_otg_en_START (0)
#define REG_PD_EXT_OTG_EN_pd_ext_otg_en_END (0)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char pd_ext_ovp_sel : 1;
        unsigned char reserved : 7;
    } reg;
} REG_PD_EXT_OVP_SEL_UNION;
#endif
#define REG_PD_EXT_OVP_SEL_pd_ext_ovp_sel_START (0)
#define REG_PD_EXT_OVP_SEL_pd_ext_ovp_sel_END (0)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char pd_ext_ovp_en : 1;
        unsigned char reserved : 7;
    } reg;
} REG_PD_EXT_OVP_EN_UNION;
#endif
#define REG_PD_EXT_OVP_EN_pd_ext_ovp_en_START (0)
#define REG_PD_EXT_OVP_EN_pd_ext_ovp_en_END (0)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char da_bc12_det_en : 1;
        unsigned char da_bc12_dcd_vref_sel : 2;
        unsigned char da_bc12_dcd_det_en : 1;
        unsigned char da_bc12_data_src_en : 1;
        unsigned char da_bc12_charge_sel : 1;
        unsigned char reserved : 2;
    } reg;
} REG_BC12_CFG_REG_0_UNION;
#endif
#define REG_BC12_CFG_REG_0_da_bc12_det_en_START (0)
#define REG_BC12_CFG_REG_0_da_bc12_det_en_END (0)
#define REG_BC12_CFG_REG_0_da_bc12_dcd_vref_sel_START (1)
#define REG_BC12_CFG_REG_0_da_bc12_dcd_vref_sel_END (2)
#define REG_BC12_CFG_REG_0_da_bc12_dcd_det_en_START (3)
#define REG_BC12_CFG_REG_0_da_bc12_dcd_det_en_END (3)
#define REG_BC12_CFG_REG_0_da_bc12_data_src_en_START (4)
#define REG_BC12_CFG_REG_0_da_bc12_data_src_en_END (4)
#define REG_BC12_CFG_REG_0_da_bc12_charge_sel_START (5)
#define REG_BC12_CFG_REG_0_da_bc12_charge_sel_END (5)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char da_bc12_vbus_det_en : 1;
        unsigned char da_bc12_test_vref_sel : 1;
        unsigned char da_bc12_src_vref_sel : 2;
        unsigned char da_bc12_vbus_vref : 2;
        unsigned char reserved : 2;
    } reg;
} REG_BC12_CFG_REG_1_UNION;
#endif
#define REG_BC12_CFG_REG_1_da_bc12_vbus_det_en_START (0)
#define REG_BC12_CFG_REG_1_da_bc12_vbus_det_en_END (0)
#define REG_BC12_CFG_REG_1_da_bc12_test_vref_sel_START (1)
#define REG_BC12_CFG_REG_1_da_bc12_test_vref_sel_END (1)
#define REG_BC12_CFG_REG_1_da_bc12_src_vref_sel_START (2)
#define REG_BC12_CFG_REG_1_da_bc12_src_vref_sel_END (3)
#define REG_BC12_CFG_REG_1_da_bc12_vbus_vref_START (4)
#define REG_BC12_CFG_REG_1_da_bc12_vbus_vref_END (5)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char da_bc12_reserved : 4;
        unsigned char reserved : 4;
    } reg;
} REG_BC12_CFG_REG_2_UNION;
#endif
#define REG_BC12_CFG_REG_2_da_bc12_reserved_START (0)
#define REG_BC12_CFG_REG_2_da_bc12_reserved_END (3)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char ad_bc12_reserved : 4;
        unsigned char ad_bc12_otgsessvid : 1;
        unsigned char ad_bc12_fsvplus : 1;
        unsigned char ad_bc12_fsvminus : 1;
        unsigned char ad_bc12_charge_det : 1;
    } reg;
} REG_BC12_STATUS_0_UNION;
#endif
#define REG_BC12_STATUS_0_ad_bc12_reserved_START (0)
#define REG_BC12_STATUS_0_ad_bc12_reserved_END (3)
#define REG_BC12_STATUS_0_ad_bc12_otgsessvid_START (4)
#define REG_BC12_STATUS_0_ad_bc12_otgsessvid_END (4)
#define REG_BC12_STATUS_0_ad_bc12_fsvplus_START (5)
#define REG_BC12_STATUS_0_ad_bc12_fsvplus_END (5)
#define REG_BC12_STATUS_0_ad_bc12_fsvminus_START (6)
#define REG_BC12_STATUS_0_ad_bc12_fsvminus_END (6)
#define REG_BC12_STATUS_0_ad_bc12_charge_det_START (7)
#define REG_BC12_STATUS_0_ad_bc12_charge_det_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char sc_hkadc_en : 1;
        unsigned char reserved : 7;
    } reg;
} REG_HKADC_EN_UNION;
#endif
#define REG_HKADC_EN_sc_hkadc_en_START (0)
#define REG_HKADC_EN_sc_hkadc_en_END (0)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char sc_eis_hkadc_start : 1;
        unsigned char reserved : 7;
    } reg;
} REG_EIS_HKADC_START_UNION;
#endif
#define REG_EIS_HKADC_START_sc_eis_hkadc_start_START (0)
#define REG_EIS_HKADC_START_sc_eis_hkadc_start_END (0)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char sc_hkadc_avg_times : 2;
        unsigned char reserved_0 : 2;
        unsigned char sc_hkadc_seq_loop : 1;
        unsigned char sc_hkadc_cul_time : 1;
        unsigned char sc_hkadc_chopper_time : 1;
        unsigned char reserved_1 : 1;
    } reg;
} REG_HKADC_CTRL1_UNION;
#endif
#define REG_HKADC_CTRL1_sc_hkadc_avg_times_START (0)
#define REG_HKADC_CTRL1_sc_hkadc_avg_times_END (1)
#define REG_HKADC_CTRL1_sc_hkadc_seq_loop_START (4)
#define REG_HKADC_CTRL1_sc_hkadc_seq_loop_END (4)
#define REG_HKADC_CTRL1_sc_hkadc_cul_time_START (5)
#define REG_HKADC_CTRL1_sc_hkadc_cul_time_END (5)
#define REG_HKADC_CTRL1_sc_hkadc_chopper_time_START (6)
#define REG_HKADC_CTRL1_sc_hkadc_chopper_time_END (6)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char sc_hkadc_seq_chanel_h : 8;
    } reg;
} REG_HKADC_SEQ_CH_H_UNION;
#endif
#define REG_HKADC_SEQ_CH_H_sc_hkadc_seq_chanel_h_START (0)
#define REG_HKADC_SEQ_CH_H_sc_hkadc_seq_chanel_h_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char sc_hkadc_seq_chanel_l : 8;
    } reg;
} REG_HKADC_SEQ_CH_L_UNION;
#endif
#define REG_HKADC_SEQ_CH_L_sc_hkadc_seq_chanel_l_START (0)
#define REG_HKADC_SEQ_CH_L_sc_hkadc_seq_chanel_l_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char sc_hkadc_rd_req : 1;
        unsigned char reserved : 7;
    } reg;
} REG_HKADC_RD_SEQ_UNION;
#endif
#define REG_HKADC_RD_SEQ_sc_hkadc_rd_req_START (0)
#define REG_HKADC_RD_SEQ_sc_hkadc_rd_req_END (0)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char hkadc_data_valid : 1;
        unsigned char reserved : 7;
    } reg;
} REG_HKADC_DATA_VALID_UNION;
#endif
#define REG_HKADC_DATA_VALID_hkadc_data_valid_START (0)
#define REG_HKADC_DATA_VALID_hkadc_data_valid_END (0)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char vusb_adc_l : 8;
    } reg;
} REG_VUSB_ADC_L_UNION;
#endif
#define REG_VUSB_ADC_L_vusb_adc_l_START (0)
#define REG_VUSB_ADC_L_vusb_adc_l_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char vusb_adc_h : 7;
        unsigned char reserved : 1;
    } reg;
} REG_VUSB_ADC_H_UNION;
#endif
#define REG_VUSB_ADC_H_vusb_adc_h_START (0)
#define REG_VUSB_ADC_H_vusb_adc_h_END (6)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char vpsw_adc_l : 8;
    } reg;
} REG_VPSW_ADC_L_UNION;
#endif
#define REG_VPSW_ADC_L_vpsw_adc_l_START (0)
#define REG_VPSW_ADC_L_vpsw_adc_l_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char vpsw_adc_h : 7;
        unsigned char reserved : 1;
    } reg;
} REG_VPSW_ADC_H_UNION;
#endif
#define REG_VPSW_ADC_H_vpsw_adc_h_START (0)
#define REG_VPSW_ADC_H_vpsw_adc_h_END (6)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char vbus_adc_l : 8;
    } reg;
} REG_VBUS_ADC_L_UNION;
#endif
#define REG_VBUS_ADC_L_vbus_adc_l_START (0)
#define REG_VBUS_ADC_L_vbus_adc_l_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char vbus_adc_h : 7;
        unsigned char reserved : 1;
    } reg;
} REG_VBUS_ADC_H_UNION;
#endif
#define REG_VBUS_ADC_H_vbus_adc_h_START (0)
#define REG_VBUS_ADC_H_vbus_adc_h_END (6)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char vbatl_adc_l : 8;
    } reg;
} REG_VBATL_ADC_L_UNION;
#endif
#define REG_VBATL_ADC_L_vbatl_adc_l_START (0)
#define REG_VBATL_ADC_L_vbatl_adc_l_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char vbatl_adc_h : 5;
        unsigned char reserved : 3;
    } reg;
} REG_VBATL_ADC_H_UNION;
#endif
#define REG_VBATL_ADC_H_vbatl_adc_h_START (0)
#define REG_VBATL_ADC_H_vbatl_adc_h_END (4)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char vbath_adc_l : 8;
    } reg;
} REG_VBATH_ADC_L_UNION;
#endif
#define REG_VBATH_ADC_L_vbath_adc_l_START (0)
#define REG_VBATH_ADC_L_vbath_adc_l_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char vbath_adc_h : 5;
        unsigned char reserved : 3;
    } reg;
} REG_VBATH_ADC_H_UNION;
#endif
#define REG_VBATH_ADC_H_vbath_adc_h_START (0)
#define REG_VBATH_ADC_H_vbath_adc_h_END (4)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char ibatl_adc_l : 8;
    } reg;
} REG_IBATL_ADC_L_UNION;
#endif
#define REG_IBATL_ADC_L_ibatl_adc_l_START (0)
#define REG_IBATL_ADC_L_ibatl_adc_l_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char ibatl_adc_h : 6;
        unsigned char reserved : 2;
    } reg;
} REG_IBATL_ADC_H_UNION;
#endif
#define REG_IBATL_ADC_H_ibatl_adc_h_START (0)
#define REG_IBATL_ADC_H_ibatl_adc_h_END (5)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char ibath_adc_l : 8;
    } reg;
} REG_IBATH_ADC_L_UNION;
#endif
#define REG_IBATH_ADC_L_ibath_adc_l_START (0)
#define REG_IBATH_ADC_L_ibath_adc_l_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char ibath_adc_h : 6;
        unsigned char reserved : 2;
    } reg;
} REG_IBATH_ADC_H_UNION;
#endif
#define REG_IBATH_ADC_H_ibath_adc_h_START (0)
#define REG_IBATH_ADC_H_ibath_adc_h_END (5)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char ibus_ref_adc_l : 8;
    } reg;
} REG_IBUS_REF_ADC_L_UNION;
#endif
#define REG_IBUS_REF_ADC_L_ibus_ref_adc_l_START (0)
#define REG_IBUS_REF_ADC_L_ibus_ref_adc_l_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char ibus_ref_adc_h : 4;
        unsigned char reserved : 4;
    } reg;
} REG_IBUS_REF_ADC_H_UNION;
#endif
#define REG_IBUS_REF_ADC_H_ibus_ref_adc_h_START (0)
#define REG_IBUS_REF_ADC_H_ibus_ref_adc_h_END (3)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char ibus_adc_l : 8;
    } reg;
} REG_IBUS_ADC_L_UNION;
#endif
#define REG_IBUS_ADC_L_ibus_adc_l_START (0)
#define REG_IBUS_ADC_L_ibus_adc_l_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char ibus_adc_h : 5;
        unsigned char reserved : 3;
    } reg;
} REG_IBUS_ADC_H_UNION;
#endif
#define REG_IBUS_ADC_H_ibus_adc_h_START (0)
#define REG_IBUS_ADC_H_ibus_adc_h_END (4)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char tdie_adc_l : 8;
    } reg;
} REG_TDIE_ADC_L_UNION;
#endif
#define REG_TDIE_ADC_L_tdie_adc_l_START (0)
#define REG_TDIE_ADC_L_tdie_adc_l_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char tdie_adc_h : 4;
        unsigned char reserved : 4;
    } reg;
} REG_TDIE_ADC_H_UNION;
#endif
#define REG_TDIE_ADC_H_tdie_adc_h_START (0)
#define REG_TDIE_ADC_H_tdie_adc_h_END (3)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char tsbatl_adc_l : 8;
    } reg;
} REG_TSBATL_ADC_L_UNION;
#endif
#define REG_TSBATL_ADC_L_tsbatl_adc_l_START (0)
#define REG_TSBATL_ADC_L_tsbatl_adc_l_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char tsbatl_adc_h : 4;
        unsigned char reserved : 4;
    } reg;
} REG_TSBATL_ADC_H_UNION;
#endif
#define REG_TSBATL_ADC_H_tsbatl_adc_h_START (0)
#define REG_TSBATL_ADC_H_tsbatl_adc_h_END (3)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char tsbath_adc_l : 8;
    } reg;
} REG_TSBATH_ADC_L_UNION;
#endif
#define REG_TSBATH_ADC_L_tsbath_adc_l_START (0)
#define REG_TSBATH_ADC_L_tsbath_adc_l_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char tsbath_adc_h : 4;
        unsigned char reserved : 4;
    } reg;
} REG_TSBATH_ADC_H_UNION;
#endif
#define REG_TSBATH_ADC_H_tsbath_adc_h_START (0)
#define REG_TSBATH_ADC_H_tsbath_adc_h_END (3)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char veis_adc_l : 8;
    } reg;
} REG_VEIS_ADC_L_UNION;
#endif
#define REG_VEIS_ADC_L_veis_adc_l_START (0)
#define REG_VEIS_ADC_L_veis_adc_l_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char veis_adc_h : 4;
        unsigned char reserved : 4;
    } reg;
} REG_VEIS_ADC_H_UNION;
#endif
#define REG_VEIS_ADC_H_veis_adc_h_START (0)
#define REG_VEIS_ADC_H_veis_adc_h_END (3)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char vcal_adc_l : 8;
    } reg;
} REG_VCAL_ADC_L_UNION;
#endif
#define REG_VCAL_ADC_L_vcal_adc_l_START (0)
#define REG_VCAL_ADC_L_vcal_adc_l_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char vcal_adc_h : 4;
        unsigned char reserved : 4;
    } reg;
} REG_VCAL_ADC_H_UNION;
#endif
#define REG_VCAL_ADC_H_vcal_adc_h_START (0)
#define REG_VCAL_ADC_H_vcal_adc_h_END (3)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char vdpdn_adc_l : 8;
    } reg;
} REG_VDPDN_ADC_L_UNION;
#endif
#define REG_VDPDN_ADC_L_vdpdn_adc_l_START (0)
#define REG_VDPDN_ADC_L_vdpdn_adc_l_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char vdpdn_adc_h : 4;
        unsigned char reserved : 4;
    } reg;
} REG_VDPDN_ADC_H_UNION;
#endif
#define REG_VDPDN_ADC_H_vdpdn_adc_h_START (0)
#define REG_VDPDN_ADC_H_vdpdn_adc_h_END (3)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char sbu_adc_l : 8;
    } reg;
} REG_SBU_ADC_L_UNION;
#endif
#define REG_SBU_ADC_L_sbu_adc_l_START (0)
#define REG_SBU_ADC_L_sbu_adc_l_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char sbu_adc_h : 4;
        unsigned char reserved : 4;
    } reg;
} REG_SBU_ADC_H_UNION;
#endif
#define REG_SBU_ADC_H_sbu_adc_h_START (0)
#define REG_SBU_ADC_H_sbu_adc_h_END (3)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char sc_hk_clk_en : 1;
        unsigned char reserved : 7;
    } reg;
} REG_HKADC_CLK_EN_UNION;
#endif
#define REG_HKADC_CLK_EN_sc_hk_clk_en_START (0)
#define REG_HKADC_CLK_EN_sc_hk_clk_en_END (0)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char sc_hkadc_tb_sel : 5;
        unsigned char reserved : 2;
        unsigned char sc_hkadc_tb_en : 1;
    } reg;
} REG_HKADC_TB_EN_SEL_UNION;
#endif
#define REG_HKADC_TB_EN_SEL_sc_hkadc_tb_sel_START (0)
#define REG_HKADC_TB_EN_SEL_sc_hkadc_tb_sel_END (4)
#define REG_HKADC_TB_EN_SEL_sc_hkadc_tb_en_START (7)
#define REG_HKADC_TB_EN_SEL_sc_hkadc_tb_en_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char hkadc_tb_bus_0 : 8;
    } reg;
} REG_HKADC_TB_DATA0_UNION;
#endif
#define REG_HKADC_TB_DATA0_hkadc_tb_bus_0_START (0)
#define REG_HKADC_TB_DATA0_hkadc_tb_bus_0_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char hkadc_tb_bus_1 : 8;
    } reg;
} REG_HKADC_TB_DATA1_UNION;
#endif
#define REG_HKADC_TB_DATA1_hkadc_tb_bus_1_START (0)
#define REG_HKADC_TB_DATA1_hkadc_tb_bus_1_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char sc_otvp_timer : 2;
        unsigned char reserved_0 : 2;
        unsigned char sc_soh_det_timer : 2;
        unsigned char reserved_1 : 1;
        unsigned char sc_otvp_en : 1;
    } reg;
} REG_SOH_CTRL_UNION;
#endif
#define REG_SOH_CTRL_sc_otvp_timer_START (0)
#define REG_SOH_CTRL_sc_otvp_timer_END (1)
#define REG_SOH_CTRL_sc_soh_det_timer_START (4)
#define REG_SOH_CTRL_sc_soh_det_timer_END (5)
#define REG_SOH_CTRL_sc_otvp_en_START (7)
#define REG_SOH_CTRL_sc_otvp_en_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char sc_otvp_discg_h_en : 1;
        unsigned char sc_otvp_discg_l_en : 1;
        unsigned char reserved : 6;
    } reg;
} REG_OTVP_DISC_EN_UNION;
#endif
#define REG_OTVP_DISC_EN_sc_otvp_discg_h_en_START (0)
#define REG_OTVP_DISC_EN_sc_otvp_discg_h_en_END (0)
#define REG_OTVP_DISC_EN_sc_otvp_discg_l_en_START (1)
#define REG_OTVP_DISC_EN_sc_otvp_discg_l_en_END (1)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char sc_otvp_tbat_th0_0 : 8;
    } reg;
} REG_OTVP_TBAT_TH0_0_UNION;
#endif
#define REG_OTVP_TBAT_TH0_0_sc_otvp_tbat_th0_0_START (0)
#define REG_OTVP_TBAT_TH0_0_sc_otvp_tbat_th0_0_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char sc_otvp_tbat_th0_1 : 4;
        unsigned char reserved : 4;
    } reg;
} REG_OTVP_TBAT_TH0_1_UNION;
#endif
#define REG_OTVP_TBAT_TH0_1_sc_otvp_tbat_th0_1_START (0)
#define REG_OTVP_TBAT_TH0_1_sc_otvp_tbat_th0_1_END (3)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char sc_otvp_tbat_th1_0 : 8;
    } reg;
} REG_OTVP_TBAT_TH1_0_UNION;
#endif
#define REG_OTVP_TBAT_TH1_0_sc_otvp_tbat_th1_0_START (0)
#define REG_OTVP_TBAT_TH1_0_sc_otvp_tbat_th1_0_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char sc_otvp_tbat_th1_1 : 4;
        unsigned char reserved : 4;
    } reg;
} REG_OTVP_TBAT_TH1_1_UNION;
#endif
#define REG_OTVP_TBAT_TH1_1_sc_otvp_tbat_th1_1_START (0)
#define REG_OTVP_TBAT_TH1_1_sc_otvp_tbat_th1_1_END (3)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char sc_otvp_tbat_th2_0 : 8;
    } reg;
} REG_OTVP_TBAT_TH2_0_UNION;
#endif
#define REG_OTVP_TBAT_TH2_0_sc_otvp_tbat_th2_0_START (0)
#define REG_OTVP_TBAT_TH2_0_sc_otvp_tbat_th2_0_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char sc_otvp_tbat_th2_1 : 4;
        unsigned char reserved : 4;
    } reg;
} REG_OTVP_TBAT_TH2_1_UNION;
#endif
#define REG_OTVP_TBAT_TH2_1_sc_otvp_tbat_th2_1_START (0)
#define REG_OTVP_TBAT_TH2_1_sc_otvp_tbat_th2_1_END (3)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char sc_otvp_vbat_th0_0 : 8;
    } reg;
} REG_OTVP_VBAT_TH0_0_UNION;
#endif
#define REG_OTVP_VBAT_TH0_0_sc_otvp_vbat_th0_0_START (0)
#define REG_OTVP_VBAT_TH0_0_sc_otvp_vbat_th0_0_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char sc_otvp_vbat_th0_1 : 4;
        unsigned char reserved : 4;
    } reg;
} REG_OTVP_VBAT_TH0_1_UNION;
#endif
#define REG_OTVP_VBAT_TH0_1_sc_otvp_vbat_th0_1_START (0)
#define REG_OTVP_VBAT_TH0_1_sc_otvp_vbat_th0_1_END (3)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char sc_otvp_vbat_th1_0 : 8;
    } reg;
} REG_OTVP_VBAT_TH1_0_UNION;
#endif
#define REG_OTVP_VBAT_TH1_0_sc_otvp_vbat_th1_0_START (0)
#define REG_OTVP_VBAT_TH1_0_sc_otvp_vbat_th1_0_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char sc_otvp_vbat_th1_1 : 4;
        unsigned char reserved : 4;
    } reg;
} REG_OTVP_VBAT_TH1_1_UNION;
#endif
#define REG_OTVP_VBAT_TH1_1_sc_otvp_vbat_th1_1_START (0)
#define REG_OTVP_VBAT_TH1_1_sc_otvp_vbat_th1_1_END (3)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char sc_otvp_vbat_th2_0 : 8;
    } reg;
} REG_OTVP_VBAT_TH2_0_UNION;
#endif
#define REG_OTVP_VBAT_TH2_0_sc_otvp_vbat_th2_0_START (0)
#define REG_OTVP_VBAT_TH2_0_sc_otvp_vbat_th2_0_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char sc_otvp_vbat_th2_1 : 4;
        unsigned char reserved : 4;
    } reg;
} REG_OTVP_VBAT_TH2_1_UNION;
#endif
#define REG_OTVP_VBAT_TH2_1_sc_otvp_vbat_th2_1_START (0)
#define REG_OTVP_VBAT_TH2_1_sc_otvp_vbat_th2_1_END (3)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char soh_otvp_l_result : 1;
        unsigned char st_tmp_ovh_l_2 : 1;
        unsigned char st_soh_ovh_l_2 : 1;
        unsigned char st_tmp_ovh_l_1 : 1;
        unsigned char st_soh_ovh_l_1 : 1;
        unsigned char st_tmp_ovh_l_0 : 1;
        unsigned char st_soh_ovh_l_0 : 1;
        unsigned char reserved : 1;
    } reg;
} REG_SOH_OVP_STATUS_0_UNION;
#endif
#define REG_SOH_OVP_STATUS_0_soh_otvp_l_result_START (0)
#define REG_SOH_OVP_STATUS_0_soh_otvp_l_result_END (0)
#define REG_SOH_OVP_STATUS_0_st_tmp_ovh_l_2_START (1)
#define REG_SOH_OVP_STATUS_0_st_tmp_ovh_l_2_END (1)
#define REG_SOH_OVP_STATUS_0_st_soh_ovh_l_2_START (2)
#define REG_SOH_OVP_STATUS_0_st_soh_ovh_l_2_END (2)
#define REG_SOH_OVP_STATUS_0_st_tmp_ovh_l_1_START (3)
#define REG_SOH_OVP_STATUS_0_st_tmp_ovh_l_1_END (3)
#define REG_SOH_OVP_STATUS_0_st_soh_ovh_l_1_START (4)
#define REG_SOH_OVP_STATUS_0_st_soh_ovh_l_1_END (4)
#define REG_SOH_OVP_STATUS_0_st_tmp_ovh_l_0_START (5)
#define REG_SOH_OVP_STATUS_0_st_tmp_ovh_l_0_END (5)
#define REG_SOH_OVP_STATUS_0_st_soh_ovh_l_0_START (6)
#define REG_SOH_OVP_STATUS_0_st_soh_ovh_l_0_END (6)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char soh_otvp_h_result : 1;
        unsigned char st_tmp_ovh_h_2 : 1;
        unsigned char st_soh_ovh_h_2 : 1;
        unsigned char st_tmp_ovh_h_1 : 1;
        unsigned char st_soh_ovh_h_1 : 1;
        unsigned char st_tmp_ovh_h_0 : 1;
        unsigned char st_soh_ovh_h_0 : 1;
        unsigned char reserved : 1;
    } reg;
} REG_SOH_OVP_STATUS_1_UNION;
#endif
#define REG_SOH_OVP_STATUS_1_soh_otvp_h_result_START (0)
#define REG_SOH_OVP_STATUS_1_soh_otvp_h_result_END (0)
#define REG_SOH_OVP_STATUS_1_st_tmp_ovh_h_2_START (1)
#define REG_SOH_OVP_STATUS_1_st_tmp_ovh_h_2_END (1)
#define REG_SOH_OVP_STATUS_1_st_soh_ovh_h_2_START (2)
#define REG_SOH_OVP_STATUS_1_st_soh_ovh_h_2_END (2)
#define REG_SOH_OVP_STATUS_1_st_tmp_ovh_h_1_START (3)
#define REG_SOH_OVP_STATUS_1_st_tmp_ovh_h_1_END (3)
#define REG_SOH_OVP_STATUS_1_st_soh_ovh_h_1_START (4)
#define REG_SOH_OVP_STATUS_1_st_soh_ovh_h_1_END (4)
#define REG_SOH_OVP_STATUS_1_st_tmp_ovh_h_0_START (5)
#define REG_SOH_OVP_STATUS_1_st_tmp_ovh_h_0_END (5)
#define REG_SOH_OVP_STATUS_1_st_soh_ovh_h_0_START (6)
#define REG_SOH_OVP_STATUS_1_st_soh_ovh_h_0_END (6)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char sc_otp_en : 1;
        unsigned char reserved : 7;
    } reg;
} REG_OTP_EN_UNION;
#endif
#define REG_OTP_EN_sc_otp_en_START (0)
#define REG_OTP_EN_sc_otp_en_END (0)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char sc_th_tbat_otp : 8;
    } reg;
} REG_TH_TBAT_OTP_UNION;
#endif
#define REG_TH_TBAT_OTP_sc_th_tbat_otp_START (0)
#define REG_TH_TBAT_OTP_sc_th_tbat_otp_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char sc_th_tbat_otp_hys : 8;
    } reg;
} REG_TH_TBAT_OTP_HYS_UNION;
#endif
#define REG_TH_TBAT_OTP_HYS_sc_th_tbat_otp_hys_START (0)
#define REG_TH_TBAT_OTP_HYS_sc_th_tbat_otp_hys_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char sc_eis_en : 1;
        unsigned char reserved : 7;
    } reg;
} REG_EIS_EN_UNION;
#endif
#define REG_EIS_EN_sc_eis_en_START (0)
#define REG_EIS_EN_sc_eis_en_END (0)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char sc_adc_eis_sel : 1;
        unsigned char reserved : 7;
    } reg;
} REG_ADC_EIS_SEL_UNION;
#endif
#define REG_ADC_EIS_SEL_sc_adc_eis_sel_START (0)
#define REG_ADC_EIS_SEL_sc_adc_eis_sel_END (0)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char sc_eis_clk_sel : 1;
        unsigned char reserved : 7;
    } reg;
} REG_EIS_CLK_SEL_UNION;
#endif
#define REG_EIS_CLK_SEL_sc_eis_clk_sel_START (0)
#define REG_EIS_CLK_SEL_sc_eis_clk_sel_END (0)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char sc_clk_eis_ext_div : 2;
        unsigned char reserved : 6;
    } reg;
} REG_EIS_EXT_CLK_FREQ_CFG_UNION;
#endif
#define REG_EIS_EXT_CLK_FREQ_CFG_sc_clk_eis_ext_div_START (0)
#define REG_EIS_EXT_CLK_FREQ_CFG_sc_clk_eis_ext_div_END (1)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char sc_eis_work_start : 2;
        unsigned char reserved : 6;
    } reg;
} REG_EIS_WORK_START_CFG_UNION;
#endif
#define REG_EIS_WORK_START_CFG_sc_eis_work_start_START (0)
#define REG_EIS_WORK_START_CFG_sc_eis_work_start_END (1)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char sc_eis_det_timer : 4;
        unsigned char reserved : 4;
    } reg;
} REG_EIS_DET_TIME_UNION;
#endif
#define REG_EIS_DET_TIME_sc_eis_det_timer_START (0)
#define REG_EIS_DET_TIME_sc_eis_det_timer_END (3)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char sc_eis_det_timer_m_0 : 8;
    } reg;
} REG_EIS_DET_TIME_M_0_UNION;
#endif
#define REG_EIS_DET_TIME_M_0_sc_eis_det_timer_m_0_START (0)
#define REG_EIS_DET_TIME_M_0_sc_eis_det_timer_m_0_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char sc_eis_det_timer_m_1 : 2;
        unsigned char reserved : 6;
    } reg;
} REG_EIS_DET_TIME_M_1_UNION;
#endif
#define REG_EIS_DET_TIME_M_1_sc_eis_det_timer_m_1_START (0)
#define REG_EIS_DET_TIME_M_1_sc_eis_det_timer_m_1_END (1)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char sc_eis_det_timer_n : 4;
        unsigned char reserved : 4;
    } reg;
} REG_EIS_DET_TIME_N_UNION;
#endif
#define REG_EIS_DET_TIME_N_sc_eis_det_timer_n_START (0)
#define REG_EIS_DET_TIME_N_sc_eis_det_timer_n_END (3)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char sc_eis_ibat_timer_k : 3;
        unsigned char sc_eis_ibat_timer_j : 2;
        unsigned char reserved : 3;
    } reg;
} REG_EIS_DET_TIME_JK_UNION;
#endif
#define REG_EIS_DET_TIME_JK_sc_eis_ibat_timer_k_START (0)
#define REG_EIS_DET_TIME_JK_sc_eis_ibat_timer_k_END (2)
#define REG_EIS_DET_TIME_JK_sc_eis_ibat_timer_j_START (3)
#define REG_EIS_DET_TIME_JK_sc_eis_ibat_timer_j_END (4)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char sc_eis_tw : 2;
        unsigned char reserved : 6;
    } reg;
} REG_EIS_TW_CFG_UNION;
#endif
#define REG_EIS_TW_CFG_sc_eis_tw_START (0)
#define REG_EIS_TW_CFG_sc_eis_tw_END (1)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char sc_eis_discg_mode : 1;
        unsigned char sc_eis_discg_en : 1;
        unsigned char reserved : 6;
    } reg;
} REG_EIS_DISCHG_CFG_UNION;
#endif
#define REG_EIS_DISCHG_CFG_sc_eis_discg_mode_START (0)
#define REG_EIS_DISCHG_CFG_sc_eis_discg_mode_END (0)
#define REG_EIS_DISCHG_CFG_sc_eis_discg_en_START (1)
#define REG_EIS_DISCHG_CFG_sc_eis_discg_en_END (1)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char sc_eis_hkadc_average : 1;
        unsigned char reserved : 7;
    } reg;
} REG_EIS_HKADC_AVERAGE_UNION;
#endif
#define REG_EIS_HKADC_AVERAGE_sc_eis_hkadc_average_START (0)
#define REG_EIS_HKADC_AVERAGE_sc_eis_hkadc_average_END (0)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char sc_eis_hthreshold_err : 8;
    } reg;
} REG_EIS_HTHRS_ERR_UNION;
#endif
#define REG_EIS_HTHRS_ERR_sc_eis_hthreshold_err_START (0)
#define REG_EIS_HTHRS_ERR_sc_eis_hthreshold_err_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char sc_eis_lthreshold_err : 8;
    } reg;
} REG_EIS_LTHRS_ERR_UNION;
#endif
#define REG_EIS_LTHRS_ERR_sc_eis_lthreshold_err_START (0)
#define REG_EIS_LTHRS_ERR_sc_eis_lthreshold_err_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char sc_eis_icomp_hthreshold_0 : 8;
    } reg;
} REG_EIS_ICOMP_HTHRS_0_UNION;
#endif
#define REG_EIS_ICOMP_HTHRS_0_sc_eis_icomp_hthreshold_0_START (0)
#define REG_EIS_ICOMP_HTHRS_0_sc_eis_icomp_hthreshold_0_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char sc_eis_icomp_hthreshold_1 : 4;
        unsigned char reserved : 4;
    } reg;
} REG_EIS_ICOMP_HTHRS_1_UNION;
#endif
#define REG_EIS_ICOMP_HTHRS_1_sc_eis_icomp_hthreshold_1_START (0)
#define REG_EIS_ICOMP_HTHRS_1_sc_eis_icomp_hthreshold_1_END (3)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char sc_eis_icomp_lthreshold_0 : 8;
    } reg;
} REG_EIS_ICOMP_LTHRS_0_UNION;
#endif
#define REG_EIS_ICOMP_LTHRS_0_sc_eis_icomp_lthreshold_0_START (0)
#define REG_EIS_ICOMP_LTHRS_0_sc_eis_icomp_lthreshold_0_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char sc_eis_icomp_lthreshold_1 : 4;
        unsigned char reserved : 4;
    } reg;
} REG_EIS_ICOMP_LTHRS_1_UNION;
#endif
#define REG_EIS_ICOMP_LTHRS_1_sc_eis_icomp_lthreshold_1_START (0)
#define REG_EIS_ICOMP_LTHRS_1_sc_eis_icomp_lthreshold_1_END (3)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char sc_eis_init_start : 1;
        unsigned char sc_eis_init_chanl2start : 2;
        unsigned char reserved : 5;
    } reg;
} REG_EIS_INIT_CFG_UNION;
#endif
#define REG_EIS_INIT_CFG_sc_eis_init_start_START (0)
#define REG_EIS_INIT_CFG_sc_eis_init_start_END (0)
#define REG_EIS_INIT_CFG_sc_eis_init_chanl2start_START (1)
#define REG_EIS_INIT_CFG_sc_eis_init_chanl2start_END (2)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char sc_eis_icomp_state_clr : 1;
        unsigned char reserved : 7;
    } reg;
} REG_EIS_ICOMP_STATE_CLR_UNION;
#endif
#define REG_EIS_ICOMP_STATE_CLR_sc_eis_icomp_state_clr_START (0)
#define REG_EIS_ICOMP_STATE_CLR_sc_eis_icomp_state_clr_END (0)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char sc_eis_memory_clr : 1;
        unsigned char reserved : 7;
    } reg;
} REG_EIS_MEM_CLEAR_UNION;
#endif
#define REG_EIS_MEM_CLEAR_sc_eis_memory_clr_START (0)
#define REG_EIS_MEM_CLEAR_sc_eis_memory_clr_END (0)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char sc_eis_vbat0_offset_0 : 8;
    } reg;
} REG_EIS_VBAT_OFS_CFG_0_UNION;
#endif
#define REG_EIS_VBAT_OFS_CFG_0_sc_eis_vbat0_offset_0_START (0)
#define REG_EIS_VBAT_OFS_CFG_0_sc_eis_vbat0_offset_0_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char sc_eis_vbat0_offset_sel : 1;
        unsigned char sc_eis_vbat0_offset_1 : 1;
        unsigned char reserved : 6;
    } reg;
} REG_EIS_VBAT_OFS_CFG_1_UNION;
#endif
#define REG_EIS_VBAT_OFS_CFG_1_sc_eis_vbat0_offset_sel_START (0)
#define REG_EIS_VBAT_OFS_CFG_1_sc_eis_vbat0_offset_sel_END (0)
#define REG_EIS_VBAT_OFS_CFG_1_sc_eis_vbat0_offset_1_START (1)
#define REG_EIS_VBAT_OFS_CFG_1_sc_eis_vbat0_offset_1_END (1)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char eis_det_t_num_0 : 8;
    } reg;
} REG_EIS_RO_DET_T_NUM_0_UNION;
#endif
#define REG_EIS_RO_DET_T_NUM_0_eis_det_t_num_0_START (0)
#define REG_EIS_RO_DET_T_NUM_0_eis_det_t_num_0_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char eis_det_t_num_1 : 3;
        unsigned char reserved : 5;
    } reg;
} REG_EIS_RO_DET_T_NUM_1_UNION;
#endif
#define REG_EIS_RO_DET_T_NUM_1_eis_det_t_num_1_START (0)
#define REG_EIS_RO_DET_T_NUM_1_eis_det_t_num_1_END (2)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char eis_i_curr_data_0 : 8;
    } reg;
} REG_EIS_RO_I_CURR_0_UNION;
#endif
#define REG_EIS_RO_I_CURR_0_eis_i_curr_data_0_START (0)
#define REG_EIS_RO_I_CURR_0_eis_i_curr_data_0_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char eis_i_curr_data_1 : 4;
        unsigned char reserved : 4;
    } reg;
} REG_EIS_RO_I_CURR_1_UNION;
#endif
#define REG_EIS_RO_I_CURR_1_eis_i_curr_data_1_START (0)
#define REG_EIS_RO_I_CURR_1_eis_i_curr_data_1_END (3)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char eis_nt_i_state_0 : 8;
    } reg;
} REG_EIS_RO_NT_I_0_UNION;
#endif
#define REG_EIS_RO_NT_I_0_eis_nt_i_state_0_START (0)
#define REG_EIS_RO_NT_I_0_eis_nt_i_state_0_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char eis_nt_i_state_1 : 8;
    } reg;
} REG_EIS_RO_NT_I_1_UNION;
#endif
#define REG_EIS_RO_NT_I_1_eis_nt_i_state_1_START (0)
#define REG_EIS_RO_NT_I_1_eis_nt_i_state_1_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char eis_vbat_sample_num : 4;
        unsigned char reserved : 4;
    } reg;
} REG_EIS_RO_VBAT_SAMP_NUM_UNION;
#endif
#define REG_EIS_RO_VBAT_SAMP_NUM_eis_vbat_sample_num_START (0)
#define REG_EIS_RO_VBAT_SAMP_NUM_eis_vbat_sample_num_END (3)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char eisadc_init_i_data_0 : 8;
    } reg;
} REG_EIS_RO_INIT_I_DAT_0_UNION;
#endif
#define REG_EIS_RO_INIT_I_DAT_0_eisadc_init_i_data_0_START (0)
#define REG_EIS_RO_INIT_I_DAT_0_eisadc_init_i_data_0_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char eisadc_init_i_data_1 : 4;
        unsigned char reserved : 4;
    } reg;
} REG_EIS_RO_INIT_I_DAT_1_UNION;
#endif
#define REG_EIS_RO_INIT_I_DAT_1_eisadc_init_i_data_1_START (0)
#define REG_EIS_RO_INIT_I_DAT_1_eisadc_init_i_data_1_END (3)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char eisadc_init_i_offset_data_0 : 8;
    } reg;
} REG_EIS_RO_INIT_I_OFS_0_UNION;
#endif
#define REG_EIS_RO_INIT_I_OFS_0_eisadc_init_i_offset_data_0_START (0)
#define REG_EIS_RO_INIT_I_OFS_0_eisadc_init_i_offset_data_0_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char eisadc_init_i_offset_data_1 : 4;
        unsigned char reserved : 4;
    } reg;
} REG_EIS_RO_INIT_I_OFS_1_UNION;
#endif
#define REG_EIS_RO_INIT_I_OFS_1_eisadc_init_i_offset_data_1_START (0)
#define REG_EIS_RO_INIT_I_OFS_1_eisadc_init_i_offset_data_1_END (3)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char eisadc_init_v_abs_data_0 : 8;
    } reg;
} REG_EIS_RO_INIT_V_ABS_0_UNION;
#endif
#define REG_EIS_RO_INIT_V_ABS_0_eisadc_init_v_abs_data_0_START (0)
#define REG_EIS_RO_INIT_V_ABS_0_eisadc_init_v_abs_data_0_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char eisadc_init_v_abs_data_1 : 4;
        unsigned char reserved : 4;
    } reg;
} REG_EIS_RO_INIT_V_ABS_1_UNION;
#endif
#define REG_EIS_RO_INIT_V_ABS_1_eisadc_init_v_abs_data_1_START (0)
#define REG_EIS_RO_INIT_V_ABS_1_eisadc_init_v_abs_data_1_END (3)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char eisadc_init_v_offset_data_0 : 8;
    } reg;
} REG_EIS_RO_INIT_V_OFS_0_UNION;
#endif
#define REG_EIS_RO_INIT_V_OFS_0_eisadc_init_v_offset_data_0_START (0)
#define REG_EIS_RO_INIT_V_OFS_0_eisadc_init_v_offset_data_0_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char eisadc_init_v_offset_data_1 : 4;
        unsigned char reserved : 4;
    } reg;
} REG_EIS_RO_INIT_V_OFS_1_UNION;
#endif
#define REG_EIS_RO_INIT_V_OFS_1_eisadc_init_v_offset_data_1_START (0)
#define REG_EIS_RO_INIT_V_OFS_1_eisadc_init_v_offset_data_1_END (3)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char eisadc_init_v_rela_data_0 : 8;
    } reg;
} REG_EIS_RO_INIT_V_RELA_0_UNION;
#endif
#define REG_EIS_RO_INIT_V_RELA_0_eisadc_init_v_rela_data_0_START (0)
#define REG_EIS_RO_INIT_V_RELA_0_eisadc_init_v_rela_data_0_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char eisadc_init_v_rela_data_1 : 4;
        unsigned char reserved : 4;
    } reg;
} REG_EIS_RO_INIT_V_RELA_1_UNION;
#endif
#define REG_EIS_RO_INIT_V_RELA_1_eisadc_init_v_rela_data_1_START (0)
#define REG_EIS_RO_INIT_V_RELA_1_eisadc_init_v_rela_data_1_END (3)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char eisadc_init_vbat0_data_0 : 8;
    } reg;
} REG_EIS_RO_INIT_VBAT0_0_UNION;
#endif
#define REG_EIS_RO_INIT_VBAT0_0_eisadc_init_vbat0_data_0_START (0)
#define REG_EIS_RO_INIT_VBAT0_0_eisadc_init_vbat0_data_0_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char eisadc_init_vbat0_data_1 : 4;
        unsigned char reserved : 4;
    } reg;
} REG_EIS_RO_INIT_VBAT0_1_UNION;
#endif
#define REG_EIS_RO_INIT_VBAT0_1_eisadc_init_vbat0_data_1_START (0)
#define REG_EIS_RO_INIT_VBAT0_1_eisadc_init_vbat0_data_1_END (3)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char eisadc_t_data_0 : 8;
    } reg;
} REG_EIS_RO_T_DATA_0_UNION;
#endif
#define REG_EIS_RO_T_DATA_0_eisadc_t_data_0_START (0)
#define REG_EIS_RO_T_DATA_0_eisadc_t_data_0_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char eisadc_t_data_1 : 4;
        unsigned char reserved : 4;
    } reg;
} REG_EIS_RO_T_DATA_1_UNION;
#endif
#define REG_EIS_RO_T_DATA_1_eisadc_t_data_1_START (0)
#define REG_EIS_RO_T_DATA_1_eisadc_t_data_1_END (3)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char dbg_eis_ana_en : 1;
        unsigned char reserved : 3;
        unsigned char dbg_eis_adc_chal_sel : 4;
    } reg;
} REG_EIS_DBG_CFG_REG_0_UNION;
#endif
#define REG_EIS_DBG_CFG_REG_0_dbg_eis_ana_en_START (0)
#define REG_EIS_DBG_CFG_REG_0_dbg_eis_ana_en_END (0)
#define REG_EIS_DBG_CFG_REG_0_dbg_eis_adc_chal_sel_START (4)
#define REG_EIS_DBG_CFG_REG_0_dbg_eis_adc_chal_sel_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char dbg_eis_dischg_en : 1;
        unsigned char dbg_eis_chanl_sel : 3;
        unsigned char reserved : 4;
    } reg;
} REG_EIS_DBG_CFG_REG_1_UNION;
#endif
#define REG_EIS_DBG_CFG_REG_1_dbg_eis_dischg_en_START (0)
#define REG_EIS_DBG_CFG_REG_1_dbg_eis_dischg_en_END (0)
#define REG_EIS_DBG_CFG_REG_1_dbg_eis_chanl_sel_START (1)
#define REG_EIS_DBG_CFG_REG_1_dbg_eis_chanl_sel_END (3)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char debug_eis_en : 8;
    } reg;
} REG_EIS_DBG_CFG_REG_2_UNION;
#endif
#define REG_EIS_DBG_CFG_REG_2_debug_eis_en_START (0)
#define REG_EIS_DBG_CFG_REG_2_debug_eis_en_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char eis_flag : 1;
        unsigned char reserved : 7;
    } reg;
} REG_EIS_DET_FLAG_UNION;
#endif
#define REG_EIS_DET_FLAG_eis_flag_START (0)
#define REG_EIS_DET_FLAG_eis_flag_END (0)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char eis_initial_end_flag : 1;
        unsigned char reserved : 7;
    } reg;
} REG_EIS_INIT_END_FLAG_UNION;
#endif
#define REG_EIS_INIT_END_FLAG_eis_initial_end_flag_START (0)
#define REG_EIS_INIT_END_FLAG_eis_initial_end_flag_END (0)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char sc_eis_tb_sel : 5;
        unsigned char reserved : 2;
        unsigned char sc_eis_tb_en : 1;
    } reg;
} REG_EIS_TB_EN_SEL_UNION;
#endif
#define REG_EIS_TB_EN_SEL_sc_eis_tb_sel_START (0)
#define REG_EIS_TB_EN_SEL_sc_eis_tb_sel_END (4)
#define REG_EIS_TB_EN_SEL_sc_eis_tb_en_START (7)
#define REG_EIS_TB_EN_SEL_sc_eis_tb_en_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char eis_tb_bus_0 : 8;
    } reg;
} REG_EIS_TB_DATA0_UNION;
#endif
#define REG_EIS_TB_DATA0_eis_tb_bus_0_START (0)
#define REG_EIS_TB_DATA0_eis_tb_bus_0_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char eis_tb_bus_1 : 8;
    } reg;
} REG_EIS_TB_DATA1_UNION;
#endif
#define REG_EIS_TB_DATA1_eis_tb_bus_1_START (0)
#define REG_EIS_TB_DATA1_eis_tb_bus_1_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char sc_eis_adc_data_debug_l : 8;
    } reg;
} REG_EIS_ADC_DATA_DEBUG_0_UNION;
#endif
#define REG_EIS_ADC_DATA_DEBUG_0_sc_eis_adc_data_debug_l_START (0)
#define REG_EIS_ADC_DATA_DEBUG_0_sc_eis_adc_data_debug_l_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char sc_eis_adc_data_debug_h : 4;
        unsigned char reserved : 4;
    } reg;
} REG_EIS_ADC_DATA_DEBUG_1_UNION;
#endif
#define REG_EIS_ADC_DATA_DEBUG_1_sc_eis_adc_data_debug_h_START (0)
#define REG_EIS_ADC_DATA_DEBUG_1_sc_eis_adc_data_debug_h_END (3)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char sc_eis_data_debug_en : 1;
        unsigned char reserved : 7;
    } reg;
} REG_EIS_ADC_DATA_DEBUG_EN_UNION;
#endif
#define REG_EIS_ADC_DATA_DEBUG_EN_sc_eis_data_debug_en_START (0)
#define REG_EIS_ADC_DATA_DEBUG_EN_sc_eis_data_debug_en_END (0)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char dbg_eis_adc_start : 1;
        unsigned char reserved : 7;
    } reg;
} REG_EIS_DBG_ADC_START_UNION;
#endif
#define REG_EIS_DBG_ADC_START_dbg_eis_adc_start_START (0)
#define REG_EIS_DBG_ADC_START_dbg_eis_adc_start_END (0)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char batfet_h_ctrl : 1;
        unsigned char reserved : 7;
    } reg;
} REG_CHARGER_BATFET_H_CTRL_UNION;
#endif
#define REG_CHARGER_BATFET_H_CTRL_batfet_h_ctrl_START (0)
#define REG_CHARGER_BATFET_H_CTRL_batfet_h_ctrl_END (0)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char batfet_l_ctrl : 1;
        unsigned char reserved : 7;
    } reg;
} REG_CHARGER_BATFET_L_CTRL_UNION;
#endif
#define REG_CHARGER_BATFET_L_CTRL_batfet_l_ctrl_START (0)
#define REG_CHARGER_BATFET_L_CTRL_batfet_l_ctrl_END (0)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char ship_mode_enb : 1;
        unsigned char reserved : 7;
    } reg;
} REG_SHIP_MODE_ENB_UNION;
#endif
#define REG_SHIP_MODE_ENB_ship_mode_enb_START (0)
#define REG_SHIP_MODE_ENB_ship_mode_enb_END (0)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char vbat_lv_h : 1;
        unsigned char reserved : 7;
    } reg;
} REG_VBAT_LV_H_REG_UNION;
#endif
#define REG_VBAT_LV_H_REG_vbat_lv_h_START (0)
#define REG_VBAT_LV_H_REG_vbat_lv_h_END (0)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char vbat_lv_l : 1;
        unsigned char reserved : 7;
    } reg;
} REG_VBAT_LV_L_REG_UNION;
#endif
#define REG_VBAT_LV_L_REG_vbat_lv_l_START (0)
#define REG_VBAT_LV_L_REG_vbat_lv_l_END (0)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char sc_hbat_res_mode : 1;
        unsigned char reserved : 7;
    } reg;
} REG_HBAT_RES_MODE_UNION;
#endif
#define REG_HBAT_RES_MODE_sc_hbat_res_mode_START (0)
#define REG_HBAT_RES_MODE_sc_hbat_res_mode_END (0)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char sc_lbat_res_mode : 1;
        unsigned char reserved : 7;
    } reg;
} REG_LBAT_RES_MODE_UNION;
#endif
#define REG_LBAT_RES_MODE_sc_lbat_res_mode_START (0)
#define REG_LBAT_RES_MODE_sc_lbat_res_mode_END (0)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char sc_pmos_en : 1;
        unsigned char reserved_0 : 3;
        unsigned char sc_chg_lp : 1;
        unsigned char reserved_1 : 3;
    } reg;
} REG_VDM_CFG_REG_0_UNION;
#endif
#define REG_VDM_CFG_REG_0_sc_pmos_en_START (0)
#define REG_VDM_CFG_REG_0_sc_pmos_en_END (0)
#define REG_VDM_CFG_REG_0_sc_chg_lp_START (4)
#define REG_VDM_CFG_REG_0_sc_chg_lp_END (4)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char reserved_0 : 4;
        unsigned char sc_boot_en : 1;
        unsigned char reserved_1 : 3;
    } reg;
} REG_VDM_CFG_REG_1_UNION;
#endif
#define REG_VDM_CFG_REG_1_sc_boot_en_START (4)
#define REG_VDM_CFG_REG_1_sc_boot_en_END (4)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char sc_regn_en : 1;
        unsigned char reserved_0 : 3;
        unsigned char sc_vusb_off_mask : 1;
        unsigned char reserved_1 : 3;
    } reg;
} REG_VDM_CFG_REG_2_UNION;
#endif
#define REG_VDM_CFG_REG_2_sc_regn_en_START (0)
#define REG_VDM_CFG_REG_2_sc_regn_en_END (0)
#define REG_VDM_CFG_REG_2_sc_vusb_off_mask_START (4)
#define REG_VDM_CFG_REG_2_sc_vusb_off_mask_END (4)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char sc_jig_sw : 1;
        unsigned char reserved : 7;
    } reg;
} REG_VDM_CFG_REG_3_UNION;
#endif
#define REG_VDM_CFG_REG_3_sc_jig_sw_START (0)
#define REG_VDM_CFG_REG_3_sc_jig_sw_END (0)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char sc_uart_en : 1;
        unsigned char reserved : 7;
    } reg;
} REG_VDM_CFG_REG_4_UNION;
#endif
#define REG_VDM_CFG_REG_4_sc_uart_en_START (0)
#define REG_VDM_CFG_REG_4_sc_uart_en_END (0)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char sc_jtag_sbu_en : 1;
        unsigned char reserved : 7;
    } reg;
} REG_VDM_CFG_REG_5_UNION;
#endif
#define REG_VDM_CFG_REG_5_sc_jtag_sbu_en_START (0)
#define REG_VDM_CFG_REG_5_sc_jtag_sbu_en_END (0)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char pd_vdm_rsv_0 : 1;
        unsigned char pd_vdm_rsv_1 : 1;
        unsigned char pd_vdm_rsv_2 : 1;
        unsigned char pd_vdm_rsv_3 : 1;
        unsigned char pd_vdm_rsv_4 : 1;
        unsigned char pd_vdm_rsv_5 : 1;
        unsigned char pd_vdm_rsv_6 : 1;
        unsigned char pd_vdm_rsv_7 : 1;
    } reg;
} REG_VDM_CFG_RSV_0_UNION;
#endif
#define REG_VDM_CFG_RSV_0_pd_vdm_rsv_0_START (0)
#define REG_VDM_CFG_RSV_0_pd_vdm_rsv_0_END (0)
#define REG_VDM_CFG_RSV_0_pd_vdm_rsv_1_START (1)
#define REG_VDM_CFG_RSV_0_pd_vdm_rsv_1_END (1)
#define REG_VDM_CFG_RSV_0_pd_vdm_rsv_2_START (2)
#define REG_VDM_CFG_RSV_0_pd_vdm_rsv_2_END (2)
#define REG_VDM_CFG_RSV_0_pd_vdm_rsv_3_START (3)
#define REG_VDM_CFG_RSV_0_pd_vdm_rsv_3_END (3)
#define REG_VDM_CFG_RSV_0_pd_vdm_rsv_4_START (4)
#define REG_VDM_CFG_RSV_0_pd_vdm_rsv_4_END (4)
#define REG_VDM_CFG_RSV_0_pd_vdm_rsv_5_START (5)
#define REG_VDM_CFG_RSV_0_pd_vdm_rsv_5_END (5)
#define REG_VDM_CFG_RSV_0_pd_vdm_rsv_6_START (6)
#define REG_VDM_CFG_RSV_0_pd_vdm_rsv_6_END (6)
#define REG_VDM_CFG_RSV_0_pd_vdm_rsv_7_START (7)
#define REG_VDM_CFG_RSV_0_pd_vdm_rsv_7_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char pd_vdm_rsv_8 : 1;
        unsigned char pd_vdm_rsv_9 : 1;
        unsigned char pd_vdm_rsv_10 : 1;
        unsigned char pd_vdm_rsv_11 : 1;
        unsigned char pd_vdm_rsv_12 : 1;
        unsigned char pd_vdm_rsv_13 : 1;
        unsigned char pd_vdm_rsv_14 : 1;
        unsigned char pd_vdm_rsv_15 : 1;
    } reg;
} REG_VDM_CFG_RSV_1_UNION;
#endif
#define REG_VDM_CFG_RSV_1_pd_vdm_rsv_8_START (0)
#define REG_VDM_CFG_RSV_1_pd_vdm_rsv_8_END (0)
#define REG_VDM_CFG_RSV_1_pd_vdm_rsv_9_START (1)
#define REG_VDM_CFG_RSV_1_pd_vdm_rsv_9_END (1)
#define REG_VDM_CFG_RSV_1_pd_vdm_rsv_10_START (2)
#define REG_VDM_CFG_RSV_1_pd_vdm_rsv_10_END (2)
#define REG_VDM_CFG_RSV_1_pd_vdm_rsv_11_START (3)
#define REG_VDM_CFG_RSV_1_pd_vdm_rsv_11_END (3)
#define REG_VDM_CFG_RSV_1_pd_vdm_rsv_12_START (4)
#define REG_VDM_CFG_RSV_1_pd_vdm_rsv_12_END (4)
#define REG_VDM_CFG_RSV_1_pd_vdm_rsv_13_START (5)
#define REG_VDM_CFG_RSV_1_pd_vdm_rsv_13_END (5)
#define REG_VDM_CFG_RSV_1_pd_vdm_rsv_14_START (6)
#define REG_VDM_CFG_RSV_1_pd_vdm_rsv_14_END (6)
#define REG_VDM_CFG_RSV_1_pd_vdm_rsv_15_START (7)
#define REG_VDM_CFG_RSV_1_pd_vdm_rsv_15_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char sc_pd_vdm5_en : 8;
    } reg;
} REG_VDM5_CFG_EN_UNION;
#endif
#define REG_VDM5_CFG_EN_sc_pd_vdm5_en_START (0)
#define REG_VDM5_CFG_EN_sc_pd_vdm5_en_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char flag_batfet_ocp : 1;
        unsigned char reserved : 7;
    } reg;
} REG_BATFET_OCP_FLAG_UNION;
#endif
#define REG_BATFET_OCP_FLAG_flag_batfet_ocp_START (0)
#define REG_BATFET_OCP_FLAG_flag_batfet_ocp_END (0)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char da_buck_reversbst_mode : 1;
        unsigned char reserved : 7;
    } reg;
} REG_DA_BUCK_REVERSBST_MODE_UNION;
#endif
#define REG_DA_BUCK_REVERSBST_MODE_da_buck_reversbst_mode_START (0)
#define REG_DA_BUCK_REVERSBST_MODE_da_buck_reversbst_mode_END (0)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char sc_buck_reversbst_clear : 1;
        unsigned char reserved : 7;
    } reg;
} REG_SC_BUCK_REVERSBST_CLEAR_UNION;
#endif
#define REG_SC_BUCK_REVERSBST_CLEAR_sc_buck_reversbst_clear_START (0)
#define REG_SC_BUCK_REVERSBST_CLEAR_sc_buck_reversbst_clear_END (0)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char da_irq_sbu_ovp_mask : 1;
        unsigned char reserved : 7;
    } reg;
} REG_JTAG_CFG_REG_0_UNION;
#endif
#define REG_JTAG_CFG_REG_0_da_irq_sbu_ovp_mask_START (0)
#define REG_JTAG_CFG_REG_0_da_irq_sbu_ovp_mask_END (0)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char da_coul_channel_sel : 1;
        unsigned char reserved : 7;
    } reg;
} REG_COUL_CHANNEL_SEL_UNION;
#endif
#define REG_COUL_CHANNEL_SEL_da_coul_channel_sel_START (0)
#define REG_COUL_CHANNEL_SEL_da_coul_channel_sel_END (0)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char da_bat_res_sel : 1;
        unsigned char reserved : 7;
    } reg;
} REG_IBAT_RES_SEL_UNION;
#endif
#define REG_IBAT_RES_SEL_da_bat_res_sel_START (0)
#define REG_IBAT_RES_SEL_da_bat_res_sel_END (0)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char da_buck_12v_maxduty_en : 1;
        unsigned char da_buck_12v_maxduty_adj : 2;
        unsigned char da_buck_ilimit : 5;
    } reg;
} REG_BUCK_OTG_CFG_REG_0_UNION;
#endif
#define REG_BUCK_OTG_CFG_REG_0_da_buck_12v_maxduty_en_START (0)
#define REG_BUCK_OTG_CFG_REG_0_da_buck_12v_maxduty_en_END (0)
#define REG_BUCK_OTG_CFG_REG_0_da_buck_12v_maxduty_adj_START (1)
#define REG_BUCK_OTG_CFG_REG_0_da_buck_12v_maxduty_adj_END (2)
#define REG_BUCK_OTG_CFG_REG_0_da_buck_ilimit_START (3)
#define REG_BUCK_OTG_CFG_REG_0_da_buck_ilimit_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char da_buck_9v_maxduty_en : 1;
        unsigned char da_buck_9v_maxduty_adj : 2;
        unsigned char reserved : 5;
    } reg;
} REG_BUCK_OTG_CFG_REG_1_UNION;
#endif
#define REG_BUCK_OTG_CFG_REG_1_da_buck_9v_maxduty_en_START (0)
#define REG_BUCK_OTG_CFG_REG_1_da_buck_9v_maxduty_en_END (0)
#define REG_BUCK_OTG_CFG_REG_1_da_buck_9v_maxduty_adj_START (1)
#define REG_BUCK_OTG_CFG_REG_1_da_buck_9v_maxduty_adj_END (2)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char da_buck_antibst : 8;
    } reg;
} REG_BUCK_OTG_CFG_REG_2_UNION;
#endif
#define REG_BUCK_OTG_CFG_REG_2_da_buck_antibst_START (0)
#define REG_BUCK_OTG_CFG_REG_2_da_buck_antibst_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char da_buck_cmp_cur : 2;
        unsigned char da_buck_cc_4m : 1;
        unsigned char da_buck_cap5_cc : 2;
        unsigned char da_buck_block_ctrl : 3;
    } reg;
} REG_BUCK_OTG_CFG_REG_3_UNION;
#endif
#define REG_BUCK_OTG_CFG_REG_3_da_buck_cmp_cur_START (0)
#define REG_BUCK_OTG_CFG_REG_3_da_buck_cmp_cur_END (1)
#define REG_BUCK_OTG_CFG_REG_3_da_buck_cc_4m_START (2)
#define REG_BUCK_OTG_CFG_REG_3_da_buck_cc_4m_END (2)
#define REG_BUCK_OTG_CFG_REG_3_da_buck_cap5_cc_START (3)
#define REG_BUCK_OTG_CFG_REG_3_da_buck_cap5_cc_END (4)
#define REG_BUCK_OTG_CFG_REG_3_da_buck_block_ctrl_START (5)
#define REG_BUCK_OTG_CFG_REG_3_da_buck_block_ctrl_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char da_buck_dmd_en : 1;
        unsigned char da_buck_dmd_delay : 1;
        unsigned char da_buck_dmd_clamp : 1;
        unsigned char da_buck_cvdmd : 5;
    } reg;
} REG_BUCK_OTG_CFG_REG_4_UNION;
#endif
#define REG_BUCK_OTG_CFG_REG_4_da_buck_dmd_en_START (0)
#define REG_BUCK_OTG_CFG_REG_4_da_buck_dmd_en_END (0)
#define REG_BUCK_OTG_CFG_REG_4_da_buck_dmd_delay_START (1)
#define REG_BUCK_OTG_CFG_REG_4_da_buck_dmd_delay_END (1)
#define REG_BUCK_OTG_CFG_REG_4_da_buck_dmd_clamp_START (2)
#define REG_BUCK_OTG_CFG_REG_4_da_buck_dmd_clamp_END (2)
#define REG_BUCK_OTG_CFG_REG_4_da_buck_cvdmd_START (3)
#define REG_BUCK_OTG_CFG_REG_4_da_buck_cvdmd_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char da_buck_dpm_auto : 1;
        unsigned char da_buck_dmd_sel : 4;
        unsigned char da_buck_dmd_ibias : 2;
        unsigned char reserved : 1;
    } reg;
} REG_BUCK_OTG_CFG_REG_5_UNION;
#endif
#define REG_BUCK_OTG_CFG_REG_5_da_buck_dpm_auto_START (0)
#define REG_BUCK_OTG_CFG_REG_5_da_buck_dpm_auto_END (0)
#define REG_BUCK_OTG_CFG_REG_5_da_buck_dmd_sel_START (1)
#define REG_BUCK_OTG_CFG_REG_5_da_buck_dmd_sel_END (4)
#define REG_BUCK_OTG_CFG_REG_5_da_buck_dmd_ibias_START (5)
#define REG_BUCK_OTG_CFG_REG_5_da_buck_dmd_ibias_END (6)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char da_buck_eco : 1;
        unsigned char da_buck_dpm_sel : 4;
        unsigned char reserved : 3;
    } reg;
} REG_BUCK_OTG_CFG_REG_6_UNION;
#endif
#define REG_BUCK_OTG_CFG_REG_6_da_buck_eco_START (0)
#define REG_BUCK_OTG_CFG_REG_6_da_buck_eco_END (0)
#define REG_BUCK_OTG_CFG_REG_6_da_buck_dpm_sel_START (1)
#define REG_BUCK_OTG_CFG_REG_6_da_buck_dpm_sel_END (4)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char da_buck_fullduty_offtime : 2;
        unsigned char da_buck_fullduty_en : 1;
        unsigned char da_buck_fullduty_delaytime : 4;
        unsigned char reserved : 1;
    } reg;
} REG_BUCK_OTG_CFG_REG_7_UNION;
#endif
#define REG_BUCK_OTG_CFG_REG_7_da_buck_fullduty_offtime_START (0)
#define REG_BUCK_OTG_CFG_REG_7_da_buck_fullduty_offtime_END (1)
#define REG_BUCK_OTG_CFG_REG_7_da_buck_fullduty_en_START (2)
#define REG_BUCK_OTG_CFG_REG_7_da_buck_fullduty_en_END (2)
#define REG_BUCK_OTG_CFG_REG_7_da_buck_fullduty_delaytime_START (3)
#define REG_BUCK_OTG_CFG_REG_7_da_buck_fullduty_delaytime_END (6)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char da_buck_lmos_on_sel : 1;
        unsigned char da_buck_lmos_on_en : 1;
        unsigned char da_buck_hmosocp_lpf : 2;
        unsigned char da_buck_gap_auto : 1;
        unsigned char da_buck_gap : 3;
    } reg;
} REG_BUCK_OTG_CFG_REG_8_UNION;
#endif
#define REG_BUCK_OTG_CFG_REG_8_da_buck_lmos_on_sel_START (0)
#define REG_BUCK_OTG_CFG_REG_8_da_buck_lmos_on_sel_END (0)
#define REG_BUCK_OTG_CFG_REG_8_da_buck_lmos_on_en_START (1)
#define REG_BUCK_OTG_CFG_REG_8_da_buck_lmos_on_en_END (1)
#define REG_BUCK_OTG_CFG_REG_8_da_buck_hmosocp_lpf_START (2)
#define REG_BUCK_OTG_CFG_REG_8_da_buck_hmosocp_lpf_END (3)
#define REG_BUCK_OTG_CFG_REG_8_da_buck_gap_auto_START (4)
#define REG_BUCK_OTG_CFG_REG_8_da_buck_gap_auto_END (4)
#define REG_BUCK_OTG_CFG_REG_8_da_buck_gap_START (5)
#define REG_BUCK_OTG_CFG_REG_8_da_buck_gap_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char da_buck_min_offtime_sel : 1;
        unsigned char da_buck_min_offtime : 2;
        unsigned char da_buck_ls_down : 2;
        unsigned char da_buck_lmosocp_lpf : 2;
        unsigned char reserved : 1;
    } reg;
} REG_BUCK_OTG_CFG_REG_9_UNION;
#endif
#define REG_BUCK_OTG_CFG_REG_9_da_buck_min_offtime_sel_START (0)
#define REG_BUCK_OTG_CFG_REG_9_da_buck_min_offtime_sel_END (0)
#define REG_BUCK_OTG_CFG_REG_9_da_buck_min_offtime_START (1)
#define REG_BUCK_OTG_CFG_REG_9_da_buck_min_offtime_END (2)
#define REG_BUCK_OTG_CFG_REG_9_da_buck_ls_down_START (3)
#define REG_BUCK_OTG_CFG_REG_9_da_buck_ls_down_END (4)
#define REG_BUCK_OTG_CFG_REG_9_da_buck_lmosocp_lpf_START (5)
#define REG_BUCK_OTG_CFG_REG_9_da_buck_lmosocp_lpf_END (6)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char da_buck_ocp_peak : 4;
        unsigned char da_buck_ocp_enb : 1;
        unsigned char da_buck_min_ontime_sel : 1;
        unsigned char da_buck_min_ontime : 2;
    } reg;
} REG_BUCK_OTG_CFG_REG_10_UNION;
#endif
#define REG_BUCK_OTG_CFG_REG_10_da_buck_ocp_peak_START (0)
#define REG_BUCK_OTG_CFG_REG_10_da_buck_ocp_peak_END (3)
#define REG_BUCK_OTG_CFG_REG_10_da_buck_ocp_enb_START (4)
#define REG_BUCK_OTG_CFG_REG_10_da_buck_ocp_enb_END (4)
#define REG_BUCK_OTG_CFG_REG_10_da_buck_min_ontime_sel_START (5)
#define REG_BUCK_OTG_CFG_REG_10_da_buck_min_ontime_sel_END (5)
#define REG_BUCK_OTG_CFG_REG_10_da_buck_min_ontime_START (6)
#define REG_BUCK_OTG_CFG_REG_10_da_buck_min_ontime_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char da_buck_osc_frq : 4;
        unsigned char da_buck_ocp_vally : 2;
        unsigned char da_buck_ocp_raise : 1;
        unsigned char reserved : 1;
    } reg;
} REG_BUCK_OTG_CFG_REG_11_UNION;
#endif
#define REG_BUCK_OTG_CFG_REG_11_da_buck_osc_frq_START (0)
#define REG_BUCK_OTG_CFG_REG_11_da_buck_osc_frq_END (3)
#define REG_BUCK_OTG_CFG_REG_11_da_buck_ocp_vally_START (4)
#define REG_BUCK_OTG_CFG_REG_11_da_buck_ocp_vally_END (5)
#define REG_BUCK_OTG_CFG_REG_11_da_buck_ocp_raise_START (6)
#define REG_BUCK_OTG_CFG_REG_11_da_buck_ocp_raise_END (6)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char da_buck_pre_op : 8;
    } reg;
} REG_BUCK_OTG_CFG_REG_12_UNION;
#endif
#define REG_BUCK_OTG_CFG_REG_12_da_buck_pre_op_START (0)
#define REG_BUCK_OTG_CFG_REG_12_da_buck_pre_op_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char da_buck_res10_cc_rout : 2;
        unsigned char da_buck_rdy_en : 1;
        unsigned char da_buck_rc_thm : 2;
        unsigned char da_buck_random : 3;
    } reg;
} REG_BUCK_OTG_CFG_REG_13_UNION;
#endif
#define REG_BUCK_OTG_CFG_REG_13_da_buck_res10_cc_rout_START (0)
#define REG_BUCK_OTG_CFG_REG_13_da_buck_res10_cc_rout_END (1)
#define REG_BUCK_OTG_CFG_REG_13_da_buck_rdy_en_START (2)
#define REG_BUCK_OTG_CFG_REG_13_da_buck_rdy_en_END (2)
#define REG_BUCK_OTG_CFG_REG_13_da_buck_rc_thm_START (3)
#define REG_BUCK_OTG_CFG_REG_13_da_buck_rc_thm_END (4)
#define REG_BUCK_OTG_CFG_REG_13_da_buck_random_START (5)
#define REG_BUCK_OTG_CFG_REG_13_da_buck_random_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char da_buck_res7_sys_gm : 2;
        unsigned char da_buck_res6_thm_gm : 2;
        unsigned char da_buck_res12_cc_pz : 2;
        unsigned char da_buck_res11_sel : 2;
    } reg;
} REG_BUCK_OTG_CFG_REG_14_UNION;
#endif
#define REG_BUCK_OTG_CFG_REG_14_da_buck_res7_sys_gm_START (0)
#define REG_BUCK_OTG_CFG_REG_14_da_buck_res7_sys_gm_END (1)
#define REG_BUCK_OTG_CFG_REG_14_da_buck_res6_thm_gm_START (2)
#define REG_BUCK_OTG_CFG_REG_14_da_buck_res6_thm_gm_END (3)
#define REG_BUCK_OTG_CFG_REG_14_da_buck_res12_cc_pz_START (4)
#define REG_BUCK_OTG_CFG_REG_14_da_buck_res12_cc_pz_END (5)
#define REG_BUCK_OTG_CFG_REG_14_da_buck_res11_sel_START (6)
#define REG_BUCK_OTG_CFG_REG_14_da_buck_res11_sel_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char da_buck_res9_cc_gm : 2;
        unsigned char da_buck_res8_cv_gm : 2;
        unsigned char reserved : 4;
    } reg;
} REG_BUCK_OTG_CFG_REG_15_UNION;
#endif
#define REG_BUCK_OTG_CFG_REG_15_da_buck_res9_cc_gm_START (0)
#define REG_BUCK_OTG_CFG_REG_15_da_buck_res9_cc_gm_END (1)
#define REG_BUCK_OTG_CFG_REG_15_da_buck_res8_cv_gm_START (2)
#define REG_BUCK_OTG_CFG_REG_15_da_buck_res8_cv_gm_END (3)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char da_buck_reserve1 : 8;
    } reg;
} REG_BUCK_OTG_CFG_REG_16_UNION;
#endif
#define REG_BUCK_OTG_CFG_REG_16_da_buck_reserve1_START (0)
#define REG_BUCK_OTG_CFG_REG_16_da_buck_reserve1_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char da_buck_reserve2 : 8;
    } reg;
} REG_BUCK_OTG_CFG_REG_17_UNION;
#endif
#define REG_BUCK_OTG_CFG_REG_17_da_buck_reserve2_START (0)
#define REG_BUCK_OTG_CFG_REG_17_da_buck_reserve2_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char da_buck_saw_vally_adj : 1;
        unsigned char da_buck_saw_sel : 2;
        unsigned char da_buck_saw_peak_adj : 1;
        unsigned char da_buck_reverbst_mode2 : 4;
    } reg;
} REG_BUCK_OTG_CFG_REG_18_UNION;
#endif
#define REG_BUCK_OTG_CFG_REG_18_da_buck_saw_vally_adj_START (0)
#define REG_BUCK_OTG_CFG_REG_18_da_buck_saw_vally_adj_END (0)
#define REG_BUCK_OTG_CFG_REG_18_da_buck_saw_sel_START (1)
#define REG_BUCK_OTG_CFG_REG_18_da_buck_saw_sel_END (2)
#define REG_BUCK_OTG_CFG_REG_18_da_buck_saw_peak_adj_START (3)
#define REG_BUCK_OTG_CFG_REG_18_da_buck_saw_peak_adj_END (3)
#define REG_BUCK_OTG_CFG_REG_18_da_buck_reverbst_mode2_START (4)
#define REG_BUCK_OTG_CFG_REG_18_da_buck_reverbst_mode2_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char da_buck_sysovp_sel : 1;
        unsigned char da_buck_sysovp_en : 1;
        unsigned char da_buck_sysmin_sel : 2;
        unsigned char da_buck_ssmode_en : 1;
        unsigned char da_buck_sft_maxduty_en : 1;
        unsigned char da_buck_scp_dis : 1;
        unsigned char reserved : 1;
    } reg;
} REG_BUCK_OTG_CFG_REG_19_UNION;
#endif
#define REG_BUCK_OTG_CFG_REG_19_da_buck_sysovp_sel_START (0)
#define REG_BUCK_OTG_CFG_REG_19_da_buck_sysovp_sel_END (0)
#define REG_BUCK_OTG_CFG_REG_19_da_buck_sysovp_en_START (1)
#define REG_BUCK_OTG_CFG_REG_19_da_buck_sysovp_en_END (1)
#define REG_BUCK_OTG_CFG_REG_19_da_buck_sysmin_sel_START (2)
#define REG_BUCK_OTG_CFG_REG_19_da_buck_sysmin_sel_END (3)
#define REG_BUCK_OTG_CFG_REG_19_da_buck_ssmode_en_START (4)
#define REG_BUCK_OTG_CFG_REG_19_da_buck_ssmode_en_END (4)
#define REG_BUCK_OTG_CFG_REG_19_da_buck_sft_maxduty_en_START (5)
#define REG_BUCK_OTG_CFG_REG_19_da_buck_sft_maxduty_en_END (5)
#define REG_BUCK_OTG_CFG_REG_19_da_buck_scp_dis_START (6)
#define REG_BUCK_OTG_CFG_REG_19_da_buck_scp_dis_END (6)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char da_otg_hmos : 1;
        unsigned char da_otg_dmd : 1;
        unsigned char da_otg_clp_l_set : 1;
        unsigned char da_otg_clp_l_en : 1;
        unsigned char da_otg_clp_h_en : 1;
        unsigned char da_otg_ckmin_raise : 1;
        unsigned char da_otg_ckmin : 2;
    } reg;
} REG_BUCK_OTG_CFG_REG_20_UNION;
#endif
#define REG_BUCK_OTG_CFG_REG_20_da_otg_hmos_START (0)
#define REG_BUCK_OTG_CFG_REG_20_da_otg_hmos_END (0)
#define REG_BUCK_OTG_CFG_REG_20_da_otg_dmd_START (1)
#define REG_BUCK_OTG_CFG_REG_20_da_otg_dmd_END (1)
#define REG_BUCK_OTG_CFG_REG_20_da_otg_clp_l_set_START (2)
#define REG_BUCK_OTG_CFG_REG_20_da_otg_clp_l_set_END (2)
#define REG_BUCK_OTG_CFG_REG_20_da_otg_clp_l_en_START (3)
#define REG_BUCK_OTG_CFG_REG_20_da_otg_clp_l_en_END (3)
#define REG_BUCK_OTG_CFG_REG_20_da_otg_clp_h_en_START (4)
#define REG_BUCK_OTG_CFG_REG_20_da_otg_clp_h_en_END (4)
#define REG_BUCK_OTG_CFG_REG_20_da_otg_ckmin_raise_START (5)
#define REG_BUCK_OTG_CFG_REG_20_da_otg_ckmin_raise_END (5)
#define REG_BUCK_OTG_CFG_REG_20_da_otg_ckmin_START (6)
#define REG_BUCK_OTG_CFG_REG_20_da_otg_ckmin_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char da_otg_ocp_en : 1;
        unsigned char da_otg_lmos_ocp : 2;
        unsigned char da_otg_lim_set : 2;
        unsigned char da_otg_lim_dcoffset : 3;
    } reg;
} REG_BUCK_OTG_CFG_REG_21_UNION;
#endif
#define REG_BUCK_OTG_CFG_REG_21_da_otg_ocp_en_START (0)
#define REG_BUCK_OTG_CFG_REG_21_da_otg_ocp_en_END (0)
#define REG_BUCK_OTG_CFG_REG_21_da_otg_lmos_ocp_START (1)
#define REG_BUCK_OTG_CFG_REG_21_da_otg_lmos_ocp_END (2)
#define REG_BUCK_OTG_CFG_REG_21_da_otg_lim_set_START (3)
#define REG_BUCK_OTG_CFG_REG_21_da_otg_lim_set_END (4)
#define REG_BUCK_OTG_CFG_REG_21_da_otg_lim_dcoffset_START (5)
#define REG_BUCK_OTG_CFG_REG_21_da_otg_lim_dcoffset_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char da_otg_ovp_en : 1;
        unsigned char da_otg_osc_ckmax : 2;
        unsigned char da_otg_osc : 4;
        unsigned char da_otg_ocp_sys : 1;
    } reg;
} REG_BUCK_OTG_CFG_REG_22_UNION;
#endif
#define REG_BUCK_OTG_CFG_REG_22_da_otg_ovp_en_START (0)
#define REG_BUCK_OTG_CFG_REG_22_da_otg_ovp_en_END (0)
#define REG_BUCK_OTG_CFG_REG_22_da_otg_osc_ckmax_START (1)
#define REG_BUCK_OTG_CFG_REG_22_da_otg_osc_ckmax_END (2)
#define REG_BUCK_OTG_CFG_REG_22_da_otg_osc_START (3)
#define REG_BUCK_OTG_CFG_REG_22_da_otg_osc_END (6)
#define REG_BUCK_OTG_CFG_REG_22_da_otg_ocp_sys_START (7)
#define REG_BUCK_OTG_CFG_REG_22_da_otg_ocp_sys_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char da_otg_pfm_v_en : 1;
        unsigned char da_otg_pfm_sel : 1;
        unsigned char reserved : 6;
    } reg;
} REG_BUCK_OTG_CFG_REG_23_UNION;
#endif
#define REG_BUCK_OTG_CFG_REG_23_da_otg_pfm_v_en_START (0)
#define REG_BUCK_OTG_CFG_REG_23_da_otg_pfm_v_en_END (0)
#define REG_BUCK_OTG_CFG_REG_23_da_otg_pfm_sel_START (1)
#define REG_BUCK_OTG_CFG_REG_23_da_otg_pfm_sel_END (1)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char da_otg_reserve1 : 8;
    } reg;
} REG_BUCK_OTG_CFG_REG_24_UNION;
#endif
#define REG_BUCK_OTG_CFG_REG_24_da_otg_reserve1_START (0)
#define REG_BUCK_OTG_CFG_REG_24_da_otg_reserve1_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char da_otg_reserve2 : 8;
    } reg;
} REG_BUCK_OTG_CFG_REG_25_UNION;
#endif
#define REG_BUCK_OTG_CFG_REG_25_da_otg_reserve2_START (0)
#define REG_BUCK_OTG_CFG_REG_25_da_otg_reserve2_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char da_otg_slop_set : 2;
        unsigned char da_otg_slop_en : 1;
        unsigned char da_otg_skip_set : 1;
        unsigned char da_otg_scp_time : 1;
        unsigned char da_otg_scp_en : 1;
        unsigned char da_otg_rf : 2;
    } reg;
} REG_BUCK_OTG_CFG_REG_26_UNION;
#endif
#define REG_BUCK_OTG_CFG_REG_26_da_otg_slop_set_START (0)
#define REG_BUCK_OTG_CFG_REG_26_da_otg_slop_set_END (1)
#define REG_BUCK_OTG_CFG_REG_26_da_otg_slop_en_START (2)
#define REG_BUCK_OTG_CFG_REG_26_da_otg_slop_en_END (2)
#define REG_BUCK_OTG_CFG_REG_26_da_otg_skip_set_START (3)
#define REG_BUCK_OTG_CFG_REG_26_da_otg_skip_set_END (3)
#define REG_BUCK_OTG_CFG_REG_26_da_otg_scp_time_START (4)
#define REG_BUCK_OTG_CFG_REG_26_da_otg_scp_time_END (4)
#define REG_BUCK_OTG_CFG_REG_26_da_otg_scp_en_START (5)
#define REG_BUCK_OTG_CFG_REG_26_da_otg_scp_en_END (5)
#define REG_BUCK_OTG_CFG_REG_26_da_otg_rf_START (6)
#define REG_BUCK_OTG_CFG_REG_26_da_otg_rf_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char da_otg_voutcap : 3;
        unsigned char da_otg_vbus : 2;
        unsigned char da_otg_uvp_en : 1;
        unsigned char da_otg_switch : 1;
        unsigned char reserved : 1;
    } reg;
} REG_BUCK_OTG_CFG_REG_27_UNION;
#endif
#define REG_BUCK_OTG_CFG_REG_27_da_otg_voutcap_START (0)
#define REG_BUCK_OTG_CFG_REG_27_da_otg_voutcap_END (2)
#define REG_BUCK_OTG_CFG_REG_27_da_otg_vbus_START (3)
#define REG_BUCK_OTG_CFG_REG_27_da_otg_vbus_END (4)
#define REG_BUCK_OTG_CFG_REG_27_da_otg_uvp_en_START (5)
#define REG_BUCK_OTG_CFG_REG_27_da_otg_uvp_en_END (5)
#define REG_BUCK_OTG_CFG_REG_27_da_otg_switch_START (6)
#define REG_BUCK_OTG_CFG_REG_27_da_otg_switch_END (6)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char ad_buck_ok : 1;
        unsigned char reserved : 7;
    } reg;
} REG_BUCK_OTG_RO_REG_28_UNION;
#endif
#define REG_BUCK_OTG_RO_REG_28_ad_buck_ok_START (0)
#define REG_BUCK_OTG_RO_REG_28_ad_buck_ok_END (0)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char ad_buck_state0 : 8;
    } reg;
} REG_BUCK_OTG_RO_REG_29_UNION;
#endif
#define REG_BUCK_OTG_RO_REG_29_ad_buck_state0_START (0)
#define REG_BUCK_OTG_RO_REG_29_ad_buck_state0_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char ad_buck_state1 : 8;
    } reg;
} REG_BUCK_OTG_RO_REG_30_UNION;
#endif
#define REG_BUCK_OTG_RO_REG_30_ad_buck_state1_START (0)
#define REG_BUCK_OTG_RO_REG_30_ad_buck_state1_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char ad_buck_state2 : 8;
    } reg;
} REG_BUCK_OTG_RO_REG_31_UNION;
#endif
#define REG_BUCK_OTG_RO_REG_31_ad_buck_state2_START (0)
#define REG_BUCK_OTG_RO_REG_31_ad_buck_state2_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char ad_otg_on : 1;
        unsigned char ad_otg_en_in : 1;
        unsigned char reserved : 6;
    } reg;
} REG_BUCK_OTG_RO_REG_32_UNION;
#endif
#define REG_BUCK_OTG_RO_REG_32_ad_otg_on_START (0)
#define REG_BUCK_OTG_RO_REG_32_ad_otg_on_END (0)
#define REG_BUCK_OTG_RO_REG_32_ad_otg_en_in_START (1)
#define REG_BUCK_OTG_RO_REG_32_ad_otg_en_in_END (1)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char ad_otg_state0 : 8;
    } reg;
} REG_BUCK_OTG_RO_REG_33_UNION;
#endif
#define REG_BUCK_OTG_RO_REG_33_ad_otg_state0_START (0)
#define REG_BUCK_OTG_RO_REG_33_ad_otg_state0_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char da_otg_uvp_sel : 2;
        unsigned char da_otg_switch_regnoff : 1;
        unsigned char da_otg_ovp_sel : 2;
        unsigned char reserved : 3;
    } reg;
} REG_BUCK_OTG_CFG_REG_34_UNION;
#endif
#define REG_BUCK_OTG_CFG_REG_34_da_otg_uvp_sel_START (0)
#define REG_BUCK_OTG_CFG_REG_34_da_otg_uvp_sel_END (1)
#define REG_BUCK_OTG_CFG_REG_34_da_otg_switch_regnoff_START (2)
#define REG_BUCK_OTG_CFG_REG_34_da_otg_switch_regnoff_END (2)
#define REG_BUCK_OTG_CFG_REG_34_da_otg_ovp_sel_START (3)
#define REG_BUCK_OTG_CFG_REG_34_da_otg_ovp_sel_END (4)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char da_batsw_s_det_force : 1;
        unsigned char da_batsw_p_det_force : 1;
        unsigned char da_batsw_open_dt_sel : 2;
        unsigned char da_batsw_dt_det_force : 1;
        unsigned char da_batsw_det_en : 1;
        unsigned char da_batp_h_det_en : 1;
        unsigned char da_batn_h_det_en : 1;
    } reg;
} REG_CHG_TOP_CFG_REG_0_UNION;
#endif
#define REG_CHG_TOP_CFG_REG_0_da_batsw_s_det_force_START (0)
#define REG_CHG_TOP_CFG_REG_0_da_batsw_s_det_force_END (0)
#define REG_CHG_TOP_CFG_REG_0_da_batsw_p_det_force_START (1)
#define REG_CHG_TOP_CFG_REG_0_da_batsw_p_det_force_END (1)
#define REG_CHG_TOP_CFG_REG_0_da_batsw_open_dt_sel_START (2)
#define REG_CHG_TOP_CFG_REG_0_da_batsw_open_dt_sel_END (3)
#define REG_CHG_TOP_CFG_REG_0_da_batsw_dt_det_force_START (4)
#define REG_CHG_TOP_CFG_REG_0_da_batsw_dt_det_force_END (4)
#define REG_CHG_TOP_CFG_REG_0_da_batsw_det_en_START (5)
#define REG_CHG_TOP_CFG_REG_0_da_batsw_det_en_END (5)
#define REG_CHG_TOP_CFG_REG_0_da_batp_h_det_en_START (6)
#define REG_CHG_TOP_CFG_REG_0_da_batp_h_det_en_END (6)
#define REG_CHG_TOP_CFG_REG_0_da_batn_h_det_en_START (7)
#define REG_CHG_TOP_CFG_REG_0_da_batn_h_det_en_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char da_chg_bat_open_h : 1;
        unsigned char da_chg_ate_mode : 1;
        unsigned char da_chg_acl_rpt_en : 1;
        unsigned char da_chg_2x_ibias_en : 1;
        unsigned char da_batsw_src_en : 1;
        unsigned char da_batsw_sink_en : 1;
        unsigned char da_batsw_scp_dt_sel : 2;
    } reg;
} REG_CHG_TOP_CFG_REG_1_UNION;
#endif
#define REG_CHG_TOP_CFG_REG_1_da_chg_bat_open_h_START (0)
#define REG_CHG_TOP_CFG_REG_1_da_chg_bat_open_h_END (0)
#define REG_CHG_TOP_CFG_REG_1_da_chg_ate_mode_START (1)
#define REG_CHG_TOP_CFG_REG_1_da_chg_ate_mode_END (1)
#define REG_CHG_TOP_CFG_REG_1_da_chg_acl_rpt_en_START (2)
#define REG_CHG_TOP_CFG_REG_1_da_chg_acl_rpt_en_END (2)
#define REG_CHG_TOP_CFG_REG_1_da_chg_2x_ibias_en_START (3)
#define REG_CHG_TOP_CFG_REG_1_da_chg_2x_ibias_en_END (3)
#define REG_CHG_TOP_CFG_REG_1_da_batsw_src_en_START (4)
#define REG_CHG_TOP_CFG_REG_1_da_batsw_src_en_END (4)
#define REG_CHG_TOP_CFG_REG_1_da_batsw_sink_en_START (5)
#define REG_CHG_TOP_CFG_REG_1_da_batsw_sink_en_END (5)
#define REG_CHG_TOP_CFG_REG_1_da_batsw_scp_dt_sel_START (6)
#define REG_CHG_TOP_CFG_REG_1_da_batsw_scp_dt_sel_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char da_chg_cap2_sel_h : 2;
        unsigned char da_chg_cap1_sel_l : 2;
        unsigned char da_chg_cap1_sel_h : 2;
        unsigned char da_chg_bat_open_l : 1;
        unsigned char reserved : 1;
    } reg;
} REG_CHG_TOP_CFG_REG_2_UNION;
#endif
#define REG_CHG_TOP_CFG_REG_2_da_chg_cap2_sel_h_START (0)
#define REG_CHG_TOP_CFG_REG_2_da_chg_cap2_sel_h_END (1)
#define REG_CHG_TOP_CFG_REG_2_da_chg_cap1_sel_l_START (2)
#define REG_CHG_TOP_CFG_REG_2_da_chg_cap1_sel_l_END (3)
#define REG_CHG_TOP_CFG_REG_2_da_chg_cap1_sel_h_START (4)
#define REG_CHG_TOP_CFG_REG_2_da_chg_cap1_sel_h_END (5)
#define REG_CHG_TOP_CFG_REG_2_da_chg_bat_open_l_START (6)
#define REG_CHG_TOP_CFG_REG_2_da_chg_bat_open_l_END (6)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char da_chg_cap4_sel_h : 2;
        unsigned char da_chg_cap3_sel_l : 2;
        unsigned char da_chg_cap3_sel_h : 2;
        unsigned char da_chg_cap2_sel_l : 2;
    } reg;
} REG_CHG_TOP_CFG_REG_3_UNION;
#endif
#define REG_CHG_TOP_CFG_REG_3_da_chg_cap4_sel_h_START (0)
#define REG_CHG_TOP_CFG_REG_3_da_chg_cap4_sel_h_END (1)
#define REG_CHG_TOP_CFG_REG_3_da_chg_cap3_sel_l_START (2)
#define REG_CHG_TOP_CFG_REG_3_da_chg_cap3_sel_l_END (3)
#define REG_CHG_TOP_CFG_REG_3_da_chg_cap3_sel_h_START (4)
#define REG_CHG_TOP_CFG_REG_3_da_chg_cap3_sel_h_END (5)
#define REG_CHG_TOP_CFG_REG_3_da_chg_cap2_sel_l_START (6)
#define REG_CHG_TOP_CFG_REG_3_da_chg_cap2_sel_l_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char da_chg_cap6_sel_h : 2;
        unsigned char da_chg_cap5_sel_l : 2;
        unsigned char da_chg_cap5_sel_h : 2;
        unsigned char da_chg_cap4_sel_l : 2;
    } reg;
} REG_CHG_TOP_CFG_REG_4_UNION;
#endif
#define REG_CHG_TOP_CFG_REG_4_da_chg_cap6_sel_h_START (0)
#define REG_CHG_TOP_CFG_REG_4_da_chg_cap6_sel_h_END (1)
#define REG_CHG_TOP_CFG_REG_4_da_chg_cap5_sel_l_START (2)
#define REG_CHG_TOP_CFG_REG_4_da_chg_cap5_sel_l_END (3)
#define REG_CHG_TOP_CFG_REG_4_da_chg_cap5_sel_h_START (4)
#define REG_CHG_TOP_CFG_REG_4_da_chg_cap5_sel_h_END (5)
#define REG_CHG_TOP_CFG_REG_4_da_chg_cap4_sel_l_START (6)
#define REG_CHG_TOP_CFG_REG_4_da_chg_cap4_sel_l_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char da_chg_eoc_del2m_en : 1;
        unsigned char da_chg_en_term : 1;
        unsigned char da_chg_cp_src_sel : 1;
        unsigned char da_chg_clk_div2_shd : 1;
        unsigned char da_chg_cap6_sel_l : 2;
        unsigned char reserved : 2;
    } reg;
} REG_CHG_TOP_CFG_REG_5_UNION;
#endif
#define REG_CHG_TOP_CFG_REG_5_da_chg_eoc_del2m_en_START (0)
#define REG_CHG_TOP_CFG_REG_5_da_chg_eoc_del2m_en_END (0)
#define REG_CHG_TOP_CFG_REG_5_da_chg_en_term_START (1)
#define REG_CHG_TOP_CFG_REG_5_da_chg_en_term_END (1)
#define REG_CHG_TOP_CFG_REG_5_da_chg_cp_src_sel_START (2)
#define REG_CHG_TOP_CFG_REG_5_da_chg_cp_src_sel_END (2)
#define REG_CHG_TOP_CFG_REG_5_da_chg_clk_div2_shd_START (3)
#define REG_CHG_TOP_CFG_REG_5_da_chg_clk_div2_shd_END (3)
#define REG_CHG_TOP_CFG_REG_5_da_chg_cap6_sel_l_START (4)
#define REG_CHG_TOP_CFG_REG_5_da_chg_cap6_sel_l_END (5)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char da_chg_fast_ichg_h : 5;
        unsigned char reserved : 3;
    } reg;
} REG_CHG_TOP_CFG_REG_6_UNION;
#endif
#define REG_CHG_TOP_CFG_REG_6_da_chg_fast_ichg_h_START (0)
#define REG_CHG_TOP_CFG_REG_6_da_chg_fast_ichg_h_END (4)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char da_chg_fast_ichg_l : 5;
        unsigned char reserved : 3;
    } reg;
} REG_CHG_TOP_CFG_REG_7_UNION;
#endif
#define REG_CHG_TOP_CFG_REG_7_da_chg_fast_ichg_l_START (0)
#define REG_CHG_TOP_CFG_REG_7_da_chg_fast_ichg_l_END (4)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char da_chg_fastchg_timer : 2;
        unsigned char da_chg_fast_vchg : 6;
    } reg;
} REG_CHG_TOP_CFG_REG_8_UNION;
#endif
#define REG_CHG_TOP_CFG_REG_8_da_chg_fastchg_timer_START (0)
#define REG_CHG_TOP_CFG_REG_8_da_chg_fastchg_timer_END (1)
#define REG_CHG_TOP_CFG_REG_8_da_chg_fast_vchg_START (2)
#define REG_CHG_TOP_CFG_REG_8_da_chg_fast_vchg_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char da_chg_ir_set_l : 3;
        unsigned char da_chg_ir_set_h : 3;
        unsigned char da_chg_fully_ir_en_l : 1;
        unsigned char da_chg_fully_ir_en_h : 1;
    } reg;
} REG_CHG_TOP_CFG_REG_9_UNION;
#endif
#define REG_CHG_TOP_CFG_REG_9_da_chg_ir_set_l_START (0)
#define REG_CHG_TOP_CFG_REG_9_da_chg_ir_set_l_END (2)
#define REG_CHG_TOP_CFG_REG_9_da_chg_ir_set_h_START (3)
#define REG_CHG_TOP_CFG_REG_9_da_chg_ir_set_h_END (5)
#define REG_CHG_TOP_CFG_REG_9_da_chg_fully_ir_en_l_START (6)
#define REG_CHG_TOP_CFG_REG_9_da_chg_fully_ir_en_l_END (6)
#define REG_CHG_TOP_CFG_REG_9_da_chg_fully_ir_en_h_START (7)
#define REG_CHG_TOP_CFG_REG_9_da_chg_fully_ir_en_h_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char da_chg_ocp_l_adj : 2;
        unsigned char da_chg_ocp_h_adj : 2;
        unsigned char da_chg_ocp_delay_shd : 1;
        unsigned char da_chg_md_sel : 1;
        unsigned char da_chg_iref_clamp : 2;
    } reg;
} REG_CHG_TOP_CFG_REG_10_UNION;
#endif
#define REG_CHG_TOP_CFG_REG_10_da_chg_ocp_l_adj_START (0)
#define REG_CHG_TOP_CFG_REG_10_da_chg_ocp_l_adj_END (1)
#define REG_CHG_TOP_CFG_REG_10_da_chg_ocp_h_adj_START (2)
#define REG_CHG_TOP_CFG_REG_10_da_chg_ocp_h_adj_END (3)
#define REG_CHG_TOP_CFG_REG_10_da_chg_ocp_delay_shd_START (4)
#define REG_CHG_TOP_CFG_REG_10_da_chg_ocp_delay_shd_END (4)
#define REG_CHG_TOP_CFG_REG_10_da_chg_md_sel_START (5)
#define REG_CHG_TOP_CFG_REG_10_da_chg_md_sel_END (5)
#define REG_CHG_TOP_CFG_REG_10_da_chg_iref_clamp_START (6)
#define REG_CHG_TOP_CFG_REG_10_da_chg_iref_clamp_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char da_chg_pre_vchg : 2;
        unsigned char da_chg_pre_ichg_l : 2;
        unsigned char da_chg_pre_ichg_h : 2;
        unsigned char da_chg_ocp_test : 1;
        unsigned char da_chg_ocp_shd : 1;
    } reg;
} REG_CHG_TOP_CFG_REG_11_UNION;
#endif
#define REG_CHG_TOP_CFG_REG_11_da_chg_pre_vchg_START (0)
#define REG_CHG_TOP_CFG_REG_11_da_chg_pre_vchg_END (1)
#define REG_CHG_TOP_CFG_REG_11_da_chg_pre_ichg_l_START (2)
#define REG_CHG_TOP_CFG_REG_11_da_chg_pre_ichg_l_END (3)
#define REG_CHG_TOP_CFG_REG_11_da_chg_pre_ichg_h_START (4)
#define REG_CHG_TOP_CFG_REG_11_da_chg_pre_ichg_h_END (5)
#define REG_CHG_TOP_CFG_REG_11_da_chg_ocp_test_START (6)
#define REG_CHG_TOP_CFG_REG_11_da_chg_ocp_test_END (6)
#define REG_CHG_TOP_CFG_REG_11_da_chg_ocp_shd_START (7)
#define REG_CHG_TOP_CFG_REG_11_da_chg_ocp_shd_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char da_chg_rechg_time : 2;
        unsigned char da_chg_prechg_timer : 2;
        unsigned char reserved : 4;
    } reg;
} REG_CHG_TOP_CFG_REG_12_UNION;
#endif
#define REG_CHG_TOP_CFG_REG_12_da_chg_rechg_time_START (0)
#define REG_CHG_TOP_CFG_REG_12_da_chg_rechg_time_END (1)
#define REG_CHG_TOP_CFG_REG_12_da_chg_prechg_timer_START (2)
#define REG_CHG_TOP_CFG_REG_12_da_chg_prechg_timer_END (3)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char da_chg_resved0 : 8;
    } reg;
} REG_CHG_TOP_CFG_REG_13_UNION;
#endif
#define REG_CHG_TOP_CFG_REG_13_da_chg_resved0_START (0)
#define REG_CHG_TOP_CFG_REG_13_da_chg_resved0_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char da_chg_resved1 : 8;
    } reg;
} REG_CHG_TOP_CFG_REG_14_UNION;
#endif
#define REG_CHG_TOP_CFG_REG_14_da_chg_resved1_START (0)
#define REG_CHG_TOP_CFG_REG_14_da_chg_resved1_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char da_chg_resved2 : 8;
    } reg;
} REG_CHG_TOP_CFG_REG_15_UNION;
#endif
#define REG_CHG_TOP_CFG_REG_15_da_chg_resved2_START (0)
#define REG_CHG_TOP_CFG_REG_15_da_chg_resved2_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char da_chg_resved3 : 8;
    } reg;
} REG_CHG_TOP_CFG_REG_16_UNION;
#endif
#define REG_CHG_TOP_CFG_REG_16_da_chg_resved3_START (0)
#define REG_CHG_TOP_CFG_REG_16_da_chg_resved3_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char da_chg_resved4 : 8;
    } reg;
} REG_CHG_TOP_CFG_REG_17_UNION;
#endif
#define REG_CHG_TOP_CFG_REG_17_da_chg_resved4_START (0)
#define REG_CHG_TOP_CFG_REG_17_da_chg_resved4_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char da_chg_resved5 : 8;
    } reg;
} REG_CHG_TOP_CFG_REG_18_UNION;
#endif
#define REG_CHG_TOP_CFG_REG_18_da_chg_resved5_START (0)
#define REG_CHG_TOP_CFG_REG_18_da_chg_resved5_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char da_chg_scp_en : 1;
        unsigned char da_chg_rpt_en : 1;
        unsigned char da_chg_rev_mode_l : 2;
        unsigned char da_chg_rev_mode_h : 2;
        unsigned char da_chg_rev_hys_sel : 2;
    } reg;
} REG_CHG_TOP_CFG_REG_19_UNION;
#endif
#define REG_CHG_TOP_CFG_REG_19_da_chg_scp_en_START (0)
#define REG_CHG_TOP_CFG_REG_19_da_chg_scp_en_END (0)
#define REG_CHG_TOP_CFG_REG_19_da_chg_rpt_en_START (1)
#define REG_CHG_TOP_CFG_REG_19_da_chg_rpt_en_END (1)
#define REG_CHG_TOP_CFG_REG_19_da_chg_rev_mode_l_START (2)
#define REG_CHG_TOP_CFG_REG_19_da_chg_rev_mode_l_END (3)
#define REG_CHG_TOP_CFG_REG_19_da_chg_rev_mode_h_START (4)
#define REG_CHG_TOP_CFG_REG_19_da_chg_rev_mode_h_END (5)
#define REG_CHG_TOP_CFG_REG_19_da_chg_rev_hys_sel_START (6)
#define REG_CHG_TOP_CFG_REG_19_da_chg_rev_hys_sel_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char da_chg_vclamp_set_h : 3;
        unsigned char da_chg_vbat_plus6mv : 1;
        unsigned char da_chg_timer_en : 1;
        unsigned char da_chg_test_md : 1;
        unsigned char da_chg_term_ichg : 2;
    } reg;
} REG_CHG_TOP_CFG_REG_20_UNION;
#endif
#define REG_CHG_TOP_CFG_REG_20_da_chg_vclamp_set_h_START (0)
#define REG_CHG_TOP_CFG_REG_20_da_chg_vclamp_set_h_END (2)
#define REG_CHG_TOP_CFG_REG_20_da_chg_vbat_plus6mv_START (3)
#define REG_CHG_TOP_CFG_REG_20_da_chg_vbat_plus6mv_END (3)
#define REG_CHG_TOP_CFG_REG_20_da_chg_timer_en_START (4)
#define REG_CHG_TOP_CFG_REG_20_da_chg_timer_en_END (4)
#define REG_CHG_TOP_CFG_REG_20_da_chg_test_md_START (5)
#define REG_CHG_TOP_CFG_REG_20_da_chg_test_md_END (5)
#define REG_CHG_TOP_CFG_REG_20_da_chg_term_ichg_START (6)
#define REG_CHG_TOP_CFG_REG_20_da_chg_term_ichg_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char da_chg_vres1_sel_h : 2;
        unsigned char da_chg_vrechg_hys : 2;
        unsigned char da_chg_vclamp_set_l : 3;
        unsigned char reserved : 1;
    } reg;
} REG_CHG_TOP_CFG_REG_21_UNION;
#endif
#define REG_CHG_TOP_CFG_REG_21_da_chg_vres1_sel_h_START (0)
#define REG_CHG_TOP_CFG_REG_21_da_chg_vres1_sel_h_END (1)
#define REG_CHG_TOP_CFG_REG_21_da_chg_vrechg_hys_START (2)
#define REG_CHG_TOP_CFG_REG_21_da_chg_vrechg_hys_END (3)
#define REG_CHG_TOP_CFG_REG_21_da_chg_vclamp_set_l_START (4)
#define REG_CHG_TOP_CFG_REG_21_da_chg_vclamp_set_l_END (6)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char da_dischg_cp_db_md : 1;
        unsigned char da_dischg_cmp_trim_md : 1;
        unsigned char da_chg_vres2_sel_l : 2;
        unsigned char da_chg_vres2_sel_h : 2;
        unsigned char da_chg_vres1_sel_l : 2;
    } reg;
} REG_CHG_TOP_CFG_REG_22_UNION;
#endif
#define REG_CHG_TOP_CFG_REG_22_da_dischg_cp_db_md_START (0)
#define REG_CHG_TOP_CFG_REG_22_da_dischg_cp_db_md_END (0)
#define REG_CHG_TOP_CFG_REG_22_da_dischg_cmp_trim_md_START (1)
#define REG_CHG_TOP_CFG_REG_22_da_dischg_cmp_trim_md_END (1)
#define REG_CHG_TOP_CFG_REG_22_da_chg_vres2_sel_l_START (2)
#define REG_CHG_TOP_CFG_REG_22_da_chg_vres2_sel_l_END (3)
#define REG_CHG_TOP_CFG_REG_22_da_chg_vres2_sel_h_START (4)
#define REG_CHG_TOP_CFG_REG_22_da_chg_vres2_sel_h_END (5)
#define REG_CHG_TOP_CFG_REG_22_da_chg_vres1_sel_l_START (6)
#define REG_CHG_TOP_CFG_REG_22_da_chg_vres1_sel_l_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char da_dischg_vth_sel : 2;
        unsigned char reserved : 6;
    } reg;
} REG_CHG_TOP_CFG_REG_23_UNION;
#endif
#define REG_CHG_TOP_CFG_REG_23_da_dischg_vth_sel_START (0)
#define REG_CHG_TOP_CFG_REG_23_da_dischg_vth_sel_END (1)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char ad_chg_dpm_state : 1;
        unsigned char ad_chg_chgstate_l : 2;
        unsigned char ad_chg_chgstate_h : 2;
        unsigned char ad_chg_acl_state : 1;
        unsigned char ad_batfet_l_disch : 1;
        unsigned char ad_batfet_h_disch : 1;
    } reg;
} REG_CHG_TOP_RO_REG_24_UNION;
#endif
#define REG_CHG_TOP_RO_REG_24_ad_chg_dpm_state_START (0)
#define REG_CHG_TOP_RO_REG_24_ad_chg_dpm_state_END (0)
#define REG_CHG_TOP_RO_REG_24_ad_chg_chgstate_l_START (1)
#define REG_CHG_TOP_RO_REG_24_ad_chg_chgstate_l_END (2)
#define REG_CHG_TOP_RO_REG_24_ad_chg_chgstate_h_START (3)
#define REG_CHG_TOP_RO_REG_24_ad_chg_chgstate_h_END (4)
#define REG_CHG_TOP_RO_REG_24_ad_chg_acl_state_START (5)
#define REG_CHG_TOP_RO_REG_24_ad_chg_acl_state_END (5)
#define REG_CHG_TOP_RO_REG_24_ad_batfet_l_disch_START (6)
#define REG_CHG_TOP_RO_REG_24_ad_batfet_l_disch_END (6)
#define REG_CHG_TOP_RO_REG_24_ad_batfet_h_disch_START (7)
#define REG_CHG_TOP_RO_REG_24_ad_batfet_h_disch_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char ad_chg_pu_btft_l : 1;
        unsigned char ad_chg_pu_btft_h : 1;
        unsigned char ad_chg_pre_state_l : 1;
        unsigned char ad_chg_pre_state_h : 1;
        unsigned char ad_chg_ov_state_l : 1;
        unsigned char ad_chg_ov_state_h : 1;
        unsigned char ad_chg_fwd_en_l : 1;
        unsigned char ad_chg_fwd_en_h : 1;
    } reg;
} REG_CHG_TOP_RO_REG_25_UNION;
#endif
#define REG_CHG_TOP_RO_REG_25_ad_chg_pu_btft_l_START (0)
#define REG_CHG_TOP_RO_REG_25_ad_chg_pu_btft_l_END (0)
#define REG_CHG_TOP_RO_REG_25_ad_chg_pu_btft_h_START (1)
#define REG_CHG_TOP_RO_REG_25_ad_chg_pu_btft_h_END (1)
#define REG_CHG_TOP_RO_REG_25_ad_chg_pre_state_l_START (2)
#define REG_CHG_TOP_RO_REG_25_ad_chg_pre_state_l_END (2)
#define REG_CHG_TOP_RO_REG_25_ad_chg_pre_state_h_START (3)
#define REG_CHG_TOP_RO_REG_25_ad_chg_pre_state_h_END (3)
#define REG_CHG_TOP_RO_REG_25_ad_chg_ov_state_l_START (4)
#define REG_CHG_TOP_RO_REG_25_ad_chg_ov_state_l_END (4)
#define REG_CHG_TOP_RO_REG_25_ad_chg_ov_state_h_START (5)
#define REG_CHG_TOP_RO_REG_25_ad_chg_ov_state_h_END (5)
#define REG_CHG_TOP_RO_REG_25_ad_chg_fwd_en_l_START (6)
#define REG_CHG_TOP_RO_REG_25_ad_chg_fwd_en_l_END (6)
#define REG_CHG_TOP_RO_REG_25_ad_chg_fwd_en_h_START (7)
#define REG_CHG_TOP_RO_REG_25_ad_chg_fwd_en_h_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char ad_chg_tri_state_h : 1;
        unsigned char ad_chg_therm_state : 1;
        unsigned char ad_chg_sft_pd_l : 1;
        unsigned char ad_chg_sft_pd_h : 1;
        unsigned char ad_chg_rev_ok_l : 1;
        unsigned char ad_chg_rev_ok_h : 1;
        unsigned char ad_chg_rev_en_l : 1;
        unsigned char ad_chg_rev_en_h : 1;
    } reg;
} REG_CHG_TOP_RO_REG_26_UNION;
#endif
#define REG_CHG_TOP_RO_REG_26_ad_chg_tri_state_h_START (0)
#define REG_CHG_TOP_RO_REG_26_ad_chg_tri_state_h_END (0)
#define REG_CHG_TOP_RO_REG_26_ad_chg_therm_state_START (1)
#define REG_CHG_TOP_RO_REG_26_ad_chg_therm_state_END (1)
#define REG_CHG_TOP_RO_REG_26_ad_chg_sft_pd_l_START (2)
#define REG_CHG_TOP_RO_REG_26_ad_chg_sft_pd_l_END (2)
#define REG_CHG_TOP_RO_REG_26_ad_chg_sft_pd_h_START (3)
#define REG_CHG_TOP_RO_REG_26_ad_chg_sft_pd_h_END (3)
#define REG_CHG_TOP_RO_REG_26_ad_chg_rev_ok_l_START (4)
#define REG_CHG_TOP_RO_REG_26_ad_chg_rev_ok_l_END (4)
#define REG_CHG_TOP_RO_REG_26_ad_chg_rev_ok_h_START (5)
#define REG_CHG_TOP_RO_REG_26_ad_chg_rev_ok_h_END (5)
#define REG_CHG_TOP_RO_REG_26_ad_chg_rev_en_l_START (6)
#define REG_CHG_TOP_RO_REG_26_ad_chg_rev_en_l_END (6)
#define REG_CHG_TOP_RO_REG_26_ad_chg_rev_en_h_START (7)
#define REG_CHG_TOP_RO_REG_26_ad_chg_rev_en_h_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char ad_chg_tri_state_l : 1;
        unsigned char reserved : 7;
    } reg;
} REG_CHG_TOP_RO_REG_27_UNION;
#endif
#define REG_CHG_TOP_RO_REG_27_ad_chg_tri_state_l_START (0)
#define REG_CHG_TOP_RO_REG_27_ad_chg_tri_state_l_END (0)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char da_buck_vbus_uvlo_en : 1;
        unsigned char da_buck_vbus_plugout_en : 1;
        unsigned char da_buck_vbus_plugin_en : 1;
        unsigned char da_buck_vbus_ovp_en : 1;
        unsigned char da_bat_gd_shield : 1;
        unsigned char da_bat_gd_sel : 1;
        unsigned char da_app_det_en : 1;
        unsigned char da_app_det_chsel : 1;
    } reg;
} REG_DET_TOP_CFG_REG_0_UNION;
#endif
#define REG_DET_TOP_CFG_REG_0_da_buck_vbus_uvlo_en_START (0)
#define REG_DET_TOP_CFG_REG_0_da_buck_vbus_uvlo_en_END (0)
#define REG_DET_TOP_CFG_REG_0_da_buck_vbus_plugout_en_START (1)
#define REG_DET_TOP_CFG_REG_0_da_buck_vbus_plugout_en_END (1)
#define REG_DET_TOP_CFG_REG_0_da_buck_vbus_plugin_en_START (2)
#define REG_DET_TOP_CFG_REG_0_da_buck_vbus_plugin_en_END (2)
#define REG_DET_TOP_CFG_REG_0_da_buck_vbus_ovp_en_START (3)
#define REG_DET_TOP_CFG_REG_0_da_buck_vbus_ovp_en_END (3)
#define REG_DET_TOP_CFG_REG_0_da_bat_gd_shield_START (4)
#define REG_DET_TOP_CFG_REG_0_da_bat_gd_shield_END (4)
#define REG_DET_TOP_CFG_REG_0_da_bat_gd_sel_START (5)
#define REG_DET_TOP_CFG_REG_0_da_bat_gd_sel_END (5)
#define REG_DET_TOP_CFG_REG_0_da_app_det_en_START (6)
#define REG_DET_TOP_CFG_REG_0_da_app_det_en_END (6)
#define REG_DET_TOP_CFG_REG_0_da_app_det_chsel_START (7)
#define REG_DET_TOP_CFG_REG_0_da_app_det_chsel_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char da_buck_vbus_uvp_en : 1;
        unsigned char reserved : 7;
    } reg;
} REG_DET_TOP_CFG_REG_1_UNION;
#endif
#define REG_DET_TOP_CFG_REG_1_da_buck_vbus_uvp_en_START (0)
#define REG_DET_TOP_CFG_REG_1_da_buck_vbus_uvp_en_END (0)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char da_det_reserve : 8;
    } reg;
} REG_DET_TOP_CFG_REG_2_UNION;
#endif
#define REG_DET_TOP_CFG_REG_2_da_det_reserve_START (0)
#define REG_DET_TOP_CFG_REG_2_da_det_reserve_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char da_dp_res_det_iqsel : 2;
        unsigned char da_dp_res_det_en : 1;
        unsigned char reserved : 5;
    } reg;
} REG_DET_TOP_CFG_REG_3_UNION;
#endif
#define REG_DET_TOP_CFG_REG_3_da_dp_res_det_iqsel_START (0)
#define REG_DET_TOP_CFG_REG_3_da_dp_res_det_iqsel_END (1)
#define REG_DET_TOP_CFG_REG_3_da_dp_res_det_en_START (2)
#define REG_DET_TOP_CFG_REG_3_da_dp_res_det_en_END (2)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char da_vbatl_ovp_en : 1;
        unsigned char da_vbatl_lv_en : 1;
        unsigned char da_vbath_ovp_en : 1;
        unsigned char da_vbath_lv_en : 1;
        unsigned char da_vbat_lv_t : 1;
        unsigned char da_slp_vset : 1;
        unsigned char da_sleep_block : 1;
        unsigned char reserved : 1;
    } reg;
} REG_DET_TOP_CFG_REG_4_UNION;
#endif
#define REG_DET_TOP_CFG_REG_4_da_vbatl_ovp_en_START (0)
#define REG_DET_TOP_CFG_REG_4_da_vbatl_ovp_en_END (0)
#define REG_DET_TOP_CFG_REG_4_da_vbatl_lv_en_START (1)
#define REG_DET_TOP_CFG_REG_4_da_vbatl_lv_en_END (1)
#define REG_DET_TOP_CFG_REG_4_da_vbath_ovp_en_START (2)
#define REG_DET_TOP_CFG_REG_4_da_vbath_ovp_en_END (2)
#define REG_DET_TOP_CFG_REG_4_da_vbath_lv_en_START (3)
#define REG_DET_TOP_CFG_REG_4_da_vbath_lv_en_END (3)
#define REG_DET_TOP_CFG_REG_4_da_vbat_lv_t_START (4)
#define REG_DET_TOP_CFG_REG_4_da_vbat_lv_t_END (4)
#define REG_DET_TOP_CFG_REG_4_da_slp_vset_START (5)
#define REG_DET_TOP_CFG_REG_4_da_slp_vset_END (5)
#define REG_DET_TOP_CFG_REG_4_da_sleep_block_START (6)
#define REG_DET_TOP_CFG_REG_4_da_sleep_block_END (6)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char da_vbus_uv_bk : 2;
        unsigned char da_vbus_ov_bk : 2;
        unsigned char da_vbus_hi_ovp_sel : 1;
        unsigned char da_vbus_bkvset : 2;
        unsigned char reserved : 1;
    } reg;
} REG_DET_TOP_CFG_REG_5_UNION;
#endif
#define REG_DET_TOP_CFG_REG_5_da_vbus_uv_bk_START (0)
#define REG_DET_TOP_CFG_REG_5_da_vbus_uv_bk_END (1)
#define REG_DET_TOP_CFG_REG_5_da_vbus_ov_bk_START (2)
#define REG_DET_TOP_CFG_REG_5_da_vbus_ov_bk_END (3)
#define REG_DET_TOP_CFG_REG_5_da_vbus_hi_ovp_sel_START (4)
#define REG_DET_TOP_CFG_REG_5_da_vbus_hi_ovp_sel_END (4)
#define REG_DET_TOP_CFG_REG_5_da_vbus_bkvset_START (5)
#define REG_DET_TOP_CFG_REG_5_da_vbus_bkvset_END (6)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char da_adc_det_sel : 1;
        unsigned char da_adc_det_psechop_en : 1;
        unsigned char reserved : 6;
    } reg;
} REG_DET_TOP_CFG_REG_6_UNION;
#endif
#define REG_DET_TOP_CFG_REG_6_da_adc_det_sel_START (0)
#define REG_DET_TOP_CFG_REG_6_da_adc_det_sel_END (0)
#define REG_DET_TOP_CFG_REG_6_da_adc_det_psechop_en_START (1)
#define REG_DET_TOP_CFG_REG_6_da_adc_det_psechop_en_END (1)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char da_eis_cur_ampcap_as_sel : 2;
        unsigned char da_eis_bat_sel : 1;
        unsigned char da_eis_adc_buffer_sel : 1;
        unsigned char reserved : 4;
    } reg;
} REG_EIS_TOP_CFG_REG_0_UNION;
#endif
#define REG_EIS_TOP_CFG_REG_0_da_eis_cur_ampcap_as_sel_START (0)
#define REG_EIS_TOP_CFG_REG_0_da_eis_cur_ampcap_as_sel_END (1)
#define REG_EIS_TOP_CFG_REG_0_da_eis_bat_sel_START (2)
#define REG_EIS_TOP_CFG_REG_0_da_eis_bat_sel_END (2)
#define REG_EIS_TOP_CFG_REG_0_da_eis_adc_buffer_sel_START (3)
#define REG_EIS_TOP_CFG_REG_0_da_eis_adc_buffer_sel_END (3)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char da_eis_cur_rescap_sel : 4;
        unsigned char da_eis_cur_ampcap_bs_sel : 4;
    } reg;
} REG_EIS_TOP_CFG_REG_1_UNION;
#endif
#define REG_EIS_TOP_CFG_REG_1_da_eis_cur_rescap_sel_START (0)
#define REG_EIS_TOP_CFG_REG_1_da_eis_cur_rescap_sel_END (3)
#define REG_EIS_TOP_CFG_REG_1_da_eis_cur_ampcap_bs_sel_START (4)
#define REG_EIS_TOP_CFG_REG_1_da_eis_cur_ampcap_bs_sel_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char da_eis_dis_curr_set : 2;
        unsigned char da_eis_dac_vreffilter_sel : 2;
        unsigned char da_eis_curr_gain_set : 3;
        unsigned char da_eis_curamp_chopper_en : 1;
    } reg;
} REG_EIS_TOP_CFG_REG_2_UNION;
#endif
#define REG_EIS_TOP_CFG_REG_2_da_eis_dis_curr_set_START (0)
#define REG_EIS_TOP_CFG_REG_2_da_eis_dis_curr_set_END (1)
#define REG_EIS_TOP_CFG_REG_2_da_eis_dac_vreffilter_sel_START (2)
#define REG_EIS_TOP_CFG_REG_2_da_eis_dac_vreffilter_sel_END (3)
#define REG_EIS_TOP_CFG_REG_2_da_eis_curr_gain_set_START (4)
#define REG_EIS_TOP_CFG_REG_2_da_eis_curr_gain_set_END (6)
#define REG_EIS_TOP_CFG_REG_2_da_eis_curamp_chopper_en_START (7)
#define REG_EIS_TOP_CFG_REG_2_da_eis_curamp_chopper_en_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char da_eis_ibias_set : 8;
    } reg;
} REG_EIS_TOP_CFG_REG_3_UNION;
#endif
#define REG_EIS_TOP_CFG_REG_3_da_eis_ibias_set_START (0)
#define REG_EIS_TOP_CFG_REG_3_da_eis_ibias_set_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char da_eis_icom_sel : 2;
        unsigned char da_eis_icom_chop_en : 1;
        unsigned char reserved : 5;
    } reg;
} REG_EIS_TOP_CFG_REG_4_UNION;
#endif
#define REG_EIS_TOP_CFG_REG_4_da_eis_icom_sel_START (0)
#define REG_EIS_TOP_CFG_REG_4_da_eis_icom_sel_END (1)
#define REG_EIS_TOP_CFG_REG_4_da_eis_icom_chop_en_START (2)
#define REG_EIS_TOP_CFG_REG_4_da_eis_icom_chop_en_END (2)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char da_eis_reserve0 : 8;
    } reg;
} REG_EIS_TOP_CFG_REG_5_UNION;
#endif
#define REG_EIS_TOP_CFG_REG_5_da_eis_reserve0_START (0)
#define REG_EIS_TOP_CFG_REG_5_da_eis_reserve0_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char da_eis_reserve1 : 8;
    } reg;
} REG_EIS_TOP_CFG_REG_6_UNION;
#endif
#define REG_EIS_TOP_CFG_REG_6_da_eis_reserve1_START (0)
#define REG_EIS_TOP_CFG_REG_6_da_eis_reserve1_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char da_eis_volamp_chopper_en : 1;
        unsigned char da_eis_vcom_sel : 2;
        unsigned char da_eis_vcom_chop_en : 1;
        unsigned char da_eis_testmode : 1;
        unsigned char reserved : 3;
    } reg;
} REG_EIS_TOP_CFG_REG_7_UNION;
#endif
#define REG_EIS_TOP_CFG_REG_7_da_eis_volamp_chopper_en_START (0)
#define REG_EIS_TOP_CFG_REG_7_da_eis_volamp_chopper_en_END (0)
#define REG_EIS_TOP_CFG_REG_7_da_eis_vcom_sel_START (1)
#define REG_EIS_TOP_CFG_REG_7_da_eis_vcom_sel_END (2)
#define REG_EIS_TOP_CFG_REG_7_da_eis_vcom_chop_en_START (3)
#define REG_EIS_TOP_CFG_REG_7_da_eis_vcom_chop_en_END (3)
#define REG_EIS_TOP_CFG_REG_7_da_eis_testmode_START (4)
#define REG_EIS_TOP_CFG_REG_7_da_eis_testmode_END (4)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char da_eis_volt_ampcap_bs_sel : 2;
        unsigned char da_eis_volt_ampcap_as_sel : 4;
        unsigned char reserved : 2;
    } reg;
} REG_EIS_TOP_CFG_REG_8_UNION;
#endif
#define REG_EIS_TOP_CFG_REG_8_da_eis_volt_ampcap_bs_sel_START (0)
#define REG_EIS_TOP_CFG_REG_8_da_eis_volt_ampcap_bs_sel_END (1)
#define REG_EIS_TOP_CFG_REG_8_da_eis_volt_ampcap_as_sel_START (2)
#define REG_EIS_TOP_CFG_REG_8_da_eis_volt_ampcap_as_sel_END (5)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char da_eis_volt_rescap_sel : 4;
        unsigned char da_eis_volt_gain_set : 3;
        unsigned char reserved : 1;
    } reg;
} REG_EIS_TOP_CFG_REG_9_UNION;
#endif
#define REG_EIS_TOP_CFG_REG_9_da_eis_volt_rescap_sel_START (0)
#define REG_EIS_TOP_CFG_REG_9_da_eis_volt_rescap_sel_END (3)
#define REG_EIS_TOP_CFG_REG_9_da_eis_volt_gain_set_START (4)
#define REG_EIS_TOP_CFG_REG_9_da_eis_volt_gain_set_END (6)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char da_ilbatdet_chop_en : 1;
        unsigned char da_ihbatdet_chop_en : 1;
        unsigned char da_ibatdet_testmode : 3;
        unsigned char reserved : 3;
    } reg;
} REG_EIS_TOP_CFG_REG_10_UNION;
#endif
#define REG_EIS_TOP_CFG_REG_10_da_ilbatdet_chop_en_START (0)
#define REG_EIS_TOP_CFG_REG_10_da_ilbatdet_chop_en_END (0)
#define REG_EIS_TOP_CFG_REG_10_da_ihbatdet_chop_en_START (1)
#define REG_EIS_TOP_CFG_REG_10_da_ihbatdet_chop_en_END (1)
#define REG_EIS_TOP_CFG_REG_10_da_ibatdet_testmode_START (2)
#define REG_EIS_TOP_CFG_REG_10_da_ibatdet_testmode_END (4)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char da_hkadc_ibias_sel : 8;
    } reg;
} REG_HKADC_CFG_REG_0_UNION;
#endif
#define REG_HKADC_CFG_REG_0_da_hkadc_ibias_sel_START (0)
#define REG_HKADC_CFG_REG_0_da_hkadc_ibias_sel_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char da_hkadc_reset : 1;
        unsigned char da_hkadc_reserved : 4;
        unsigned char da_hkadc_ibsel : 3;
    } reg;
} REG_HKADC_CFG_REG_1_UNION;
#endif
#define REG_HKADC_CFG_REG_1_da_hkadc_reset_START (0)
#define REG_HKADC_CFG_REG_1_da_hkadc_reset_END (0)
#define REG_HKADC_CFG_REG_1_da_hkadc_reserved_START (1)
#define REG_HKADC_CFG_REG_1_da_hkadc_reserved_END (4)
#define REG_HKADC_CFG_REG_1_da_hkadc_ibsel_START (5)
#define REG_HKADC_CFG_REG_1_da_hkadc_ibsel_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char da_ref_ichg_sel : 2;
        unsigned char da_ref_clk_chop_sel : 2;
        unsigned char da_ref_chop_en : 1;
        unsigned char da_otmp_adj : 2;
        unsigned char da_ibias_switch_en : 1;
    } reg;
} REG_REF_TOP_CFG_REG_0_UNION;
#endif
#define REG_REF_TOP_CFG_REG_0_da_ref_ichg_sel_START (0)
#define REG_REF_TOP_CFG_REG_0_da_ref_ichg_sel_END (1)
#define REG_REF_TOP_CFG_REG_0_da_ref_clk_chop_sel_START (2)
#define REG_REF_TOP_CFG_REG_0_da_ref_clk_chop_sel_END (3)
#define REG_REF_TOP_CFG_REG_0_da_ref_chop_en_START (4)
#define REG_REF_TOP_CFG_REG_0_da_ref_chop_en_END (4)
#define REG_REF_TOP_CFG_REG_0_da_otmp_adj_START (5)
#define REG_REF_TOP_CFG_REG_0_da_otmp_adj_END (6)
#define REG_REF_TOP_CFG_REG_0_da_ibias_switch_en_START (7)
#define REG_REF_TOP_CFG_REG_0_da_ibias_switch_en_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char da_ref_iref_sel : 1;
        unsigned char reserved : 7;
    } reg;
} REG_REF_TOP_CFG_REG_1_UNION;
#endif
#define REG_REF_TOP_CFG_REG_1_da_ref_iref_sel_START (0)
#define REG_REF_TOP_CFG_REG_1_da_ref_iref_sel_END (0)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char da_ref_reserve0 : 8;
    } reg;
} REG_REF_TOP_CFG_REG_2_UNION;
#endif
#define REG_REF_TOP_CFG_REG_2_da_ref_reserve0_START (0)
#define REG_REF_TOP_CFG_REG_2_da_ref_reserve0_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char da_ref_reserve1 : 8;
    } reg;
} REG_REF_TOP_CFG_REG_3_UNION;
#endif
#define REG_REF_TOP_CFG_REG_3_da_ref_reserve1_START (0)
#define REG_REF_TOP_CFG_REG_3_da_ref_reserve1_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char da_tdie_treg_set : 2;
        unsigned char da_tdie_alm_set : 2;
        unsigned char da_ref_vrc_en : 1;
        unsigned char da_ref_testib_en : 1;
        unsigned char da_ref_testbg_en : 1;
        unsigned char da_ref_selgate_en : 1;
    } reg;
} REG_REF_TOP_CFG_REG_4_UNION;
#endif
#define REG_REF_TOP_CFG_REG_4_da_tdie_treg_set_START (0)
#define REG_REF_TOP_CFG_REG_4_da_tdie_treg_set_END (1)
#define REG_REF_TOP_CFG_REG_4_da_tdie_alm_set_START (2)
#define REG_REF_TOP_CFG_REG_4_da_tdie_alm_set_END (3)
#define REG_REF_TOP_CFG_REG_4_da_ref_vrc_en_START (4)
#define REG_REF_TOP_CFG_REG_4_da_ref_vrc_en_END (4)
#define REG_REF_TOP_CFG_REG_4_da_ref_testib_en_START (5)
#define REG_REF_TOP_CFG_REG_4_da_ref_testib_en_END (5)
#define REG_REF_TOP_CFG_REG_4_da_ref_testbg_en_START (6)
#define REG_REF_TOP_CFG_REG_4_da_ref_testbg_en_END (6)
#define REG_REF_TOP_CFG_REG_4_da_ref_selgate_en_START (7)
#define REG_REF_TOP_CFG_REG_4_da_ref_selgate_en_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char da_psel_ocp_shield : 1;
        unsigned char da_ldo1p8_d_vset : 3;
        unsigned char da_ldo1p8_a_vset : 3;
        unsigned char reserved : 1;
    } reg;
} REG_PSEL_TOP_CFG_REG_0_UNION;
#endif
#define REG_PSEL_TOP_CFG_REG_0_da_psel_ocp_shield_START (0)
#define REG_PSEL_TOP_CFG_REG_0_da_psel_ocp_shield_END (0)
#define REG_PSEL_TOP_CFG_REG_0_da_ldo1p8_d_vset_START (1)
#define REG_PSEL_TOP_CFG_REG_0_da_ldo1p8_d_vset_END (3)
#define REG_PSEL_TOP_CFG_REG_0_da_ldo1p8_a_vset_START (4)
#define REG_PSEL_TOP_CFG_REG_0_da_ldo1p8_a_vset_END (6)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char da_psel_regn_vset : 2;
        unsigned char da_psel_vsysmux_sel : 1;
        unsigned char da_psel_ldo3p3_tsmod : 1;
        unsigned char reserved : 4;
    } reg;
} REG_PSEL_TOP_CFG_REG_1_UNION;
#endif
#define REG_PSEL_TOP_CFG_REG_1_da_psel_regn_vset_START (0)
#define REG_PSEL_TOP_CFG_REG_1_da_psel_regn_vset_END (1)
#define REG_PSEL_TOP_CFG_REG_1_da_psel_vsysmux_sel_START (2)
#define REG_PSEL_TOP_CFG_REG_1_da_psel_vsysmux_sel_END (2)
#define REG_PSEL_TOP_CFG_REG_1_da_psel_ldo3p3_tsmod_START (3)
#define REG_PSEL_TOP_CFG_REG_1_da_psel_ldo3p3_tsmod_END (3)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char da_psel_reserved : 8;
    } reg;
} REG_PSEL_TOP_CFG_REG_2_UNION;
#endif
#define REG_PSEL_TOP_CFG_REG_2_da_psel_reserved_START (0)
#define REG_PSEL_TOP_CFG_REG_2_da_psel_reserved_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char da_regn_ocp_irq_shield : 2;
        unsigned char reserved : 6;
    } reg;
} REG_PSEL_TOP_CFG_REG_3_UNION;
#endif
#define REG_PSEL_TOP_CFG_REG_3_da_regn_ocp_irq_shield_START (0)
#define REG_PSEL_TOP_CFG_REG_3_da_regn_ocp_irq_shield_END (1)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char da_psel_vsys_sel_hys : 1;
        unsigned char da_psel_vout_sel_hys : 1;
        unsigned char reserved : 6;
    } reg;
} REG_PSEL_TOP_CFG_REG_4_UNION;
#endif
#define REG_PSEL_TOP_CFG_REG_4_da_psel_vsys_sel_hys_START (0)
#define REG_PSEL_TOP_CFG_REG_4_da_psel_vsys_sel_hys_END (0)
#define REG_PSEL_TOP_CFG_REG_4_da_psel_vout_sel_hys_START (1)
#define REG_PSEL_TOP_CFG_REG_4_da_psel_vout_sel_hys_END (1)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char ad_ldo1p8_d_ocp : 1;
        unsigned char ad_ldo1p8_a_ocp : 1;
        unsigned char reserved : 6;
    } reg;
} REG_PSEL_TOP_RO_REG_0_UNION;
#endif
#define REG_PSEL_TOP_RO_REG_0_ad_ldo1p8_d_ocp_START (0)
#define REG_PSEL_TOP_RO_REG_0_ad_ldo1p8_d_ocp_END (0)
#define REG_PSEL_TOP_RO_REG_0_ad_ldo1p8_a_ocp_START (1)
#define REG_PSEL_TOP_RO_REG_0_ad_ldo1p8_a_ocp_END (1)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char da_usb_ovp_pro_en : 1;
        unsigned char da_usb_ovp_ichg_sel : 2;
        unsigned char da_usb_ovp_dt_sel : 1;
        unsigned char da_usb_ovp_alm_en : 1;
        unsigned char da_usb_drop_ovp_mode : 2;
        unsigned char reserved : 1;
    } reg;
} REG_USB_OVP_CFG_REG_0_UNION;
#endif
#define REG_USB_OVP_CFG_REG_0_da_usb_ovp_pro_en_START (0)
#define REG_USB_OVP_CFG_REG_0_da_usb_ovp_pro_en_END (0)
#define REG_USB_OVP_CFG_REG_0_da_usb_ovp_ichg_sel_START (1)
#define REG_USB_OVP_CFG_REG_0_da_usb_ovp_ichg_sel_END (2)
#define REG_USB_OVP_CFG_REG_0_da_usb_ovp_dt_sel_START (3)
#define REG_USB_OVP_CFG_REG_0_da_usb_ovp_dt_sel_END (3)
#define REG_USB_OVP_CFG_REG_0_da_usb_ovp_alm_en_START (4)
#define REG_USB_OVP_CFG_REG_0_da_usb_ovp_alm_en_END (4)
#define REG_USB_OVP_CFG_REG_0_da_usb_drop_ovp_mode_START (5)
#define REG_USB_OVP_CFG_REG_0_da_usb_drop_ovp_mode_END (6)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char da_usb_ovp_resved : 8;
    } reg;
} REG_USB_OVP_CFG_REG_1_UNION;
#endif
#define REG_USB_OVP_CFG_REG_1_da_usb_ovp_resved_START (0)
#define REG_USB_OVP_CFG_REG_1_da_usb_ovp_resved_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char da_usb_plugout_en : 1;
        unsigned char da_usb_plugin_en : 1;
        unsigned char da_usb_ovpsub_pd_enb : 1;
        unsigned char da_usb_ovp_shdn_dt_sel : 1;
        unsigned char da_usb_ovp_set : 3;
        unsigned char da_usb_ovp_rev_md_dt_sel : 1;
    } reg;
} REG_USB_OVP_CFG_REG_2_UNION;
#endif
#define REG_USB_OVP_CFG_REG_2_da_usb_plugout_en_START (0)
#define REG_USB_OVP_CFG_REG_2_da_usb_plugout_en_END (0)
#define REG_USB_OVP_CFG_REG_2_da_usb_plugin_en_START (1)
#define REG_USB_OVP_CFG_REG_2_da_usb_plugin_en_END (1)
#define REG_USB_OVP_CFG_REG_2_da_usb_ovpsub_pd_enb_START (2)
#define REG_USB_OVP_CFG_REG_2_da_usb_ovpsub_pd_enb_END (2)
#define REG_USB_OVP_CFG_REG_2_da_usb_ovp_shdn_dt_sel_START (3)
#define REG_USB_OVP_CFG_REG_2_da_usb_ovp_shdn_dt_sel_END (3)
#define REG_USB_OVP_CFG_REG_2_da_usb_ovp_set_START (4)
#define REG_USB_OVP_CFG_REG_2_da_usb_ovp_set_END (6)
#define REG_USB_OVP_CFG_REG_2_da_usb_ovp_rev_md_dt_sel_START (7)
#define REG_USB_OVP_CFG_REG_2_da_usb_ovp_rev_md_dt_sel_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char da_usb_vdrop_ovp_test_md : 1;
        unsigned char da_usb_vdrop_ovp_set : 1;
        unsigned char da_usb_vdrop_ovp_en : 2;
        unsigned char da_usb_vdrop_ovp_dt_sel : 1;
        unsigned char da_usb_uvlo_en : 1;
        unsigned char reserved : 2;
    } reg;
} REG_USB_OVP_CFG_REG_3_UNION;
#endif
#define REG_USB_OVP_CFG_REG_3_da_usb_vdrop_ovp_test_md_START (0)
#define REG_USB_OVP_CFG_REG_3_da_usb_vdrop_ovp_test_md_END (0)
#define REG_USB_OVP_CFG_REG_3_da_usb_vdrop_ovp_set_START (1)
#define REG_USB_OVP_CFG_REG_3_da_usb_vdrop_ovp_set_END (1)
#define REG_USB_OVP_CFG_REG_3_da_usb_vdrop_ovp_en_START (2)
#define REG_USB_OVP_CFG_REG_3_da_usb_vdrop_ovp_en_END (3)
#define REG_USB_OVP_CFG_REG_3_da_usb_vdrop_ovp_dt_sel_START (4)
#define REG_USB_OVP_CFG_REG_3_da_usb_vdrop_ovp_dt_sel_END (4)
#define REG_USB_OVP_CFG_REG_3_da_usb_uvlo_en_START (5)
#define REG_USB_OVP_CFG_REG_3_da_usb_uvlo_en_END (5)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char ad_fwd_ovp_vdrop_ovp_l : 1;
        unsigned char ad_fwd_ovp_vdrop_ovp_h : 1;
        unsigned char reserved : 6;
    } reg;
} REG_USB_OVP_RO_REG_4_UNION;
#endif
#define REG_USB_OVP_RO_REG_4_ad_fwd_ovp_vdrop_ovp_l_START (0)
#define REG_USB_OVP_RO_REG_4_ad_fwd_ovp_vdrop_ovp_l_END (0)
#define REG_USB_OVP_RO_REG_4_ad_fwd_ovp_vdrop_ovp_h_START (1)
#define REG_USB_OVP_RO_REG_4_ad_fwd_ovp_vdrop_ovp_h_END (1)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char da_psw_ovp_pro_en : 1;
        unsigned char da_psw_ovp_ichg_sel : 2;
        unsigned char da_psw_ovp_dt_sel : 1;
        unsigned char da_psw_ovp_alm_en : 1;
        unsigned char da_psw_drop_ovp_mode : 2;
        unsigned char reserved : 1;
    } reg;
} REG_PSW_OVP_CFG_REG_0_UNION;
#endif
#define REG_PSW_OVP_CFG_REG_0_da_psw_ovp_pro_en_START (0)
#define REG_PSW_OVP_CFG_REG_0_da_psw_ovp_pro_en_END (0)
#define REG_PSW_OVP_CFG_REG_0_da_psw_ovp_ichg_sel_START (1)
#define REG_PSW_OVP_CFG_REG_0_da_psw_ovp_ichg_sel_END (2)
#define REG_PSW_OVP_CFG_REG_0_da_psw_ovp_dt_sel_START (3)
#define REG_PSW_OVP_CFG_REG_0_da_psw_ovp_dt_sel_END (3)
#define REG_PSW_OVP_CFG_REG_0_da_psw_ovp_alm_en_START (4)
#define REG_PSW_OVP_CFG_REG_0_da_psw_ovp_alm_en_END (4)
#define REG_PSW_OVP_CFG_REG_0_da_psw_drop_ovp_mode_START (5)
#define REG_PSW_OVP_CFG_REG_0_da_psw_drop_ovp_mode_END (6)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char da_psw_ovp_resved : 8;
    } reg;
} REG_PSW_OVP_CFG_REG_1_UNION;
#endif
#define REG_PSW_OVP_CFG_REG_1_da_psw_ovp_resved_START (0)
#define REG_PSW_OVP_CFG_REG_1_da_psw_ovp_resved_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char da_psw_pswsub_pd_enb : 1;
        unsigned char da_psw_plugout_en : 1;
        unsigned char da_psw_plugin_en : 1;
        unsigned char da_psw_ovp_shdn_dt_sel : 1;
        unsigned char da_psw_ovp_set : 3;
        unsigned char da_psw_ovp_rev_md_dt_sel : 1;
    } reg;
} REG_PSW_OVP_CFG_REG_2_UNION;
#endif
#define REG_PSW_OVP_CFG_REG_2_da_psw_pswsub_pd_enb_START (0)
#define REG_PSW_OVP_CFG_REG_2_da_psw_pswsub_pd_enb_END (0)
#define REG_PSW_OVP_CFG_REG_2_da_psw_plugout_en_START (1)
#define REG_PSW_OVP_CFG_REG_2_da_psw_plugout_en_END (1)
#define REG_PSW_OVP_CFG_REG_2_da_psw_plugin_en_START (2)
#define REG_PSW_OVP_CFG_REG_2_da_psw_plugin_en_END (2)
#define REG_PSW_OVP_CFG_REG_2_da_psw_ovp_shdn_dt_sel_START (3)
#define REG_PSW_OVP_CFG_REG_2_da_psw_ovp_shdn_dt_sel_END (3)
#define REG_PSW_OVP_CFG_REG_2_da_psw_ovp_set_START (4)
#define REG_PSW_OVP_CFG_REG_2_da_psw_ovp_set_END (6)
#define REG_PSW_OVP_CFG_REG_2_da_psw_ovp_rev_md_dt_sel_START (7)
#define REG_PSW_OVP_CFG_REG_2_da_psw_ovp_rev_md_dt_sel_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char da_psw_vdrop_ovp_test_md : 1;
        unsigned char da_psw_vdrop_ovp_set : 1;
        unsigned char da_psw_vdrop_ovp_en : 2;
        unsigned char da_psw_vdrop_ovp_dt_sel : 1;
        unsigned char da_psw_uvlo_en : 1;
        unsigned char reserved : 2;
    } reg;
} REG_PSW_OVP_CFG_REG_3_UNION;
#endif
#define REG_PSW_OVP_CFG_REG_3_da_psw_vdrop_ovp_test_md_START (0)
#define REG_PSW_OVP_CFG_REG_3_da_psw_vdrop_ovp_test_md_END (0)
#define REG_PSW_OVP_CFG_REG_3_da_psw_vdrop_ovp_set_START (1)
#define REG_PSW_OVP_CFG_REG_3_da_psw_vdrop_ovp_set_END (1)
#define REG_PSW_OVP_CFG_REG_3_da_psw_vdrop_ovp_en_START (2)
#define REG_PSW_OVP_CFG_REG_3_da_psw_vdrop_ovp_en_END (3)
#define REG_PSW_OVP_CFG_REG_3_da_psw_vdrop_ovp_dt_sel_START (4)
#define REG_PSW_OVP_CFG_REG_3_da_psw_vdrop_ovp_dt_sel_END (4)
#define REG_PSW_OVP_CFG_REG_3_da_psw_uvlo_en_START (5)
#define REG_PSW_OVP_CFG_REG_3_da_psw_uvlo_en_END (5)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char da_sys_resvo0 : 8;
    } reg;
} REG_SCHG_LOGIC_CFG_REG_0_UNION;
#endif
#define REG_SCHG_LOGIC_CFG_REG_0_da_sys_resvo0_START (0)
#define REG_SCHG_LOGIC_CFG_REG_0_da_sys_resvo0_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char da_sys_resvo1 : 8;
    } reg;
} REG_SCHG_LOGIC_CFG_REG_1_UNION;
#endif
#define REG_SCHG_LOGIC_CFG_REG_1_da_sys_resvo1_START (0)
#define REG_SCHG_LOGIC_CFG_REG_1_da_sys_resvo1_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char da_sys_resvo2 : 8;
    } reg;
} REG_SCHG_LOGIC_CFG_REG_2_UNION;
#endif
#define REG_SCHG_LOGIC_CFG_REG_2_da_sys_resvo2_START (0)
#define REG_SCHG_LOGIC_CFG_REG_2_da_sys_resvo2_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char da_sys_resvo3 : 8;
    } reg;
} REG_SCHG_LOGIC_CFG_REG_3_UNION;
#endif
#define REG_SCHG_LOGIC_CFG_REG_3_da_sys_resvo3_START (0)
#define REG_SCHG_LOGIC_CFG_REG_3_da_sys_resvo3_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char da_sys_resvo4 : 8;
    } reg;
} REG_SCHG_LOGIC_CFG_REG_4_UNION;
#endif
#define REG_SCHG_LOGIC_CFG_REG_4_da_sys_resvo4_START (0)
#define REG_SCHG_LOGIC_CFG_REG_4_da_sys_resvo4_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char da_sys_resvo5 : 8;
    } reg;
} REG_SCHG_LOGIC_CFG_REG_5_UNION;
#endif
#define REG_SCHG_LOGIC_CFG_REG_5_da_sys_resvo5_START (0)
#define REG_SCHG_LOGIC_CFG_REG_5_da_sys_resvo5_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char da_timer_test : 1;
        unsigned char reserved : 7;
    } reg;
} REG_SCHG_LOGIC_CFG_REG_6_UNION;
#endif
#define REG_SCHG_LOGIC_CFG_REG_6_da_timer_test_START (0)
#define REG_SCHG_LOGIC_CFG_REG_6_da_timer_test_END (0)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char da_vbus_uvp_dt_sel : 1;
        unsigned char da_vbus_ok_nflt_mask : 1;
        unsigned char da_otg_chg_regnok : 1;
        unsigned char da_osc_test_sel : 3;
        unsigned char da_frs_hiz : 1;
        unsigned char da_fr_swap_en_mask : 1;
    } reg;
} REG_SCHG_LOGIC_CFG_REG_7_UNION;
#endif
#define REG_SCHG_LOGIC_CFG_REG_7_da_vbus_uvp_dt_sel_START (0)
#define REG_SCHG_LOGIC_CFG_REG_7_da_vbus_uvp_dt_sel_END (0)
#define REG_SCHG_LOGIC_CFG_REG_7_da_vbus_ok_nflt_mask_START (1)
#define REG_SCHG_LOGIC_CFG_REG_7_da_vbus_ok_nflt_mask_END (1)
#define REG_SCHG_LOGIC_CFG_REG_7_da_otg_chg_regnok_START (2)
#define REG_SCHG_LOGIC_CFG_REG_7_da_otg_chg_regnok_END (2)
#define REG_SCHG_LOGIC_CFG_REG_7_da_osc_test_sel_START (3)
#define REG_SCHG_LOGIC_CFG_REG_7_da_osc_test_sel_END (5)
#define REG_SCHG_LOGIC_CFG_REG_7_da_frs_hiz_START (6)
#define REG_SCHG_LOGIC_CFG_REG_7_da_frs_hiz_END (6)
#define REG_SCHG_LOGIC_CFG_REG_7_da_fr_swap_en_mask_START (7)
#define REG_SCHG_LOGIC_CFG_REG_7_da_fr_swap_en_mask_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char da_otg_delay_sel : 2;
        unsigned char da_ldo33_en : 1;
        unsigned char da_ldo18a_en : 1;
        unsigned char reserved : 4;
    } reg;
} REG_SCHG_LOGIC_CFG_REG_8_UNION;
#endif
#define REG_SCHG_LOGIC_CFG_REG_8_da_otg_delay_sel_START (0)
#define REG_SCHG_LOGIC_CFG_REG_8_da_otg_delay_sel_END (1)
#define REG_SCHG_LOGIC_CFG_REG_8_da_ldo33_en_START (2)
#define REG_SCHG_LOGIC_CFG_REG_8_da_ldo33_en_END (2)
#define REG_SCHG_LOGIC_CFG_REG_8_da_ldo18a_en_START (3)
#define REG_SCHG_LOGIC_CFG_REG_8_da_ldo18a_en_END (3)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char ad_sys_resvi0 : 8;
    } reg;
} REG_SCHG_LOGIC_RO_REG_0_UNION;
#endif
#define REG_SCHG_LOGIC_RO_REG_0_ad_sys_resvi0_START (0)
#define REG_SCHG_LOGIC_RO_REG_0_ad_sys_resvi0_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char ad_sys_resvi1 : 8;
    } reg;
} REG_SCHG_LOGIC_RO_REG_1_UNION;
#endif
#define REG_SCHG_LOGIC_RO_REG_1_ad_sys_resvi1_START (0)
#define REG_SCHG_LOGIC_RO_REG_1_ad_sys_resvi1_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char da_typ_cfg0 : 8;
    } reg;
} REG_TCPC_CFG_REG_0_UNION;
#endif
#define REG_TCPC_CFG_REG_0_da_typ_cfg0_START (0)
#define REG_TCPC_CFG_REG_0_da_typ_cfg0_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char da_typ_cfg1 : 8;
    } reg;
} REG_TCPC_CFG_REG_1_UNION;
#endif
#define REG_TCPC_CFG_REG_1_da_typ_cfg1_START (0)
#define REG_TCPC_CFG_REG_1_da_typ_cfg1_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char da_typ_cfg2 : 8;
    } reg;
} REG_TCPC_CFG_REG_2_UNION;
#endif
#define REG_TCPC_CFG_REG_2_da_typ_cfg2_START (0)
#define REG_TCPC_CFG_REG_2_da_typ_cfg2_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char da_cc_reserved : 4;
        unsigned char reserved : 4;
    } reg;
} REG_TCPC_CFG_REG_3_UNION;
#endif
#define REG_TCPC_CFG_REG_3_da_cc_reserved_START (0)
#define REG_TCPC_CFG_REG_3_da_cc_reserved_END (3)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char da_sbu2_det_en : 1;
        unsigned char da_sbu1_det_en : 1;
        unsigned char da_sbu_cur_sel : 2;
        unsigned char da_sbu_adc_chsel : 1;
        unsigned char reserved : 3;
    } reg;
} REG_TYPE_CFG_REG_4_UNION;
#endif
#define REG_TYPE_CFG_REG_4_da_sbu2_det_en_START (0)
#define REG_TYPE_CFG_REG_4_da_sbu2_det_en_END (0)
#define REG_TYPE_CFG_REG_4_da_sbu1_det_en_START (1)
#define REG_TYPE_CFG_REG_4_da_sbu1_det_en_END (1)
#define REG_TYPE_CFG_REG_4_da_sbu_cur_sel_START (2)
#define REG_TYPE_CFG_REG_4_da_sbu_cur_sel_END (3)
#define REG_TYPE_CFG_REG_4_da_sbu_adc_chsel_START (4)
#define REG_TYPE_CFG_REG_4_da_sbu_adc_chsel_END (4)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char ad_cc_resvi : 4;
        unsigned char reserved : 4;
    } reg;
} REG_TCPC_RO_REG_1_UNION;
#endif
#define REG_TCPC_RO_REG_1_ad_cc_resvi_START (0)
#define REG_TCPC_RO_REG_1_ad_cc_resvi_END (3)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char da_osc_reserve1 : 8;
    } reg;
} REG_OSC_TOP_CFG_REG_0_UNION;
#endif
#define REG_OSC_TOP_CFG_REG_0_da_osc_reserve1_START (0)
#define REG_OSC_TOP_CFG_REG_0_da_osc_reserve1_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char da_osc_reserve2 : 8;
    } reg;
} REG_OSC_TOP_CFG_REG_1_UNION;
#endif
#define REG_OSC_TOP_CFG_REG_1_da_osc_reserve2_START (0)
#define REG_OSC_TOP_CFG_REG_1_da_osc_reserve2_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char da_osc16m_en : 1;
        unsigned char reserved : 7;
    } reg;
} REG_OSC_TOP_CFG_REG_2_UNION;
#endif
#define REG_OSC_TOP_CFG_REG_2_da_osc16m_en_START (0)
#define REG_OSC_TOP_CFG_REG_2_da_osc16m_en_END (0)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char chip_id0 : 8;
    } reg;
} REG_VERSION0_RO_REG_0_UNION;
#endif
#define REG_VERSION0_RO_REG_0_chip_id0_START (0)
#define REG_VERSION0_RO_REG_0_chip_id0_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char chip_id1 : 8;
    } reg;
} REG_VERSION1_RO_REG_0_UNION;
#endif
#define REG_VERSION1_RO_REG_0_chip_id1_START (0)
#define REG_VERSION1_RO_REG_0_chip_id1_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char chip_id2 : 8;
    } reg;
} REG_VERSION2_RO_REG_0_UNION;
#endif
#define REG_VERSION2_RO_REG_0_chip_id2_START (0)
#define REG_VERSION2_RO_REG_0_chip_id2_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char chip_id3 : 8;
    } reg;
} REG_VERSION3_RO_REG_0_UNION;
#endif
#define REG_VERSION3_RO_REG_0_chip_id3_START (0)
#define REG_VERSION3_RO_REG_0_chip_id3_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char chip_id4 : 8;
    } reg;
} REG_VERSION4_RO_REG_0_UNION;
#endif
#define REG_VERSION4_RO_REG_0_chip_id4_START (0)
#define REG_VERSION4_RO_REG_0_chip_id4_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char chip_id5 : 8;
    } reg;
} REG_VERSION5_RO_REG_0_UNION;
#endif
#define REG_VERSION5_RO_REG_0_chip_id5_START (0)
#define REG_VERSION5_RO_REG_0_chip_id5_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char sc_cg_testbus_sel : 4;
        unsigned char reserved : 3;
        unsigned char sc_cg_testbus_en : 1;
    } reg;
} REG_SC_TESTBUS_CFG_UNION;
#endif
#define REG_SC_TESTBUS_CFG_sc_cg_testbus_sel_START (0)
#define REG_SC_TESTBUS_CFG_sc_cg_testbus_sel_END (3)
#define REG_SC_TESTBUS_CFG_sc_cg_testbus_en_START (7)
#define REG_SC_TESTBUS_CFG_sc_cg_testbus_en_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char cg_testbus_rdata : 8;
    } reg;
} REG_SC_TESTBUS_RDATA_UNION;
#endif
#define REG_SC_TESTBUS_RDATA_cg_testbus_rdata_START (0)
#define REG_SC_TESTBUS_RDATA_cg_testbus_rdata_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char sc_cg_glb_soft_rst_n : 8;
    } reg;
} REG_GLB_SOFT_RST_CTRL_UNION;
#endif
#define REG_GLB_SOFT_RST_CTRL_sc_cg_glb_soft_rst_n_START (0)
#define REG_GLB_SOFT_RST_CTRL_sc_cg_glb_soft_rst_n_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char sc_cg_efuse_soft_rst_n : 8;
    } reg;
} REG_EFUSE_SOFT_RST_CTRL_UNION;
#endif
#define REG_EFUSE_SOFT_RST_CTRL_sc_cg_efuse_soft_rst_n_START (0)
#define REG_EFUSE_SOFT_RST_CTRL_sc_cg_efuse_soft_rst_n_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char sc_hkadc_soft_rst_n : 8;
    } reg;
} REG_HKADC_SOFT_RST_CTRL_UNION;
#endif
#define REG_HKADC_SOFT_RST_CTRL_sc_hkadc_soft_rst_n_START (0)
#define REG_HKADC_SOFT_RST_CTRL_sc_hkadc_soft_rst_n_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char sc_cg_coul_soft_rst_n : 8;
    } reg;
} REG_COUL_SOFT_RST_CTRL_UNION;
#endif
#define REG_COUL_SOFT_RST_CTRL_sc_cg_coul_soft_rst_n_START (0)
#define REG_COUL_SOFT_RST_CTRL_sc_cg_coul_soft_rst_n_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char sc_cg_coul_pd_rst_n : 8;
    } reg;
} REG_COUL_PD_RST_CTRL_UNION;
#endif
#define REG_COUL_PD_RST_CTRL_sc_cg_coul_pd_rst_n_START (0)
#define REG_COUL_PD_RST_CTRL_sc_cg_coul_pd_rst_n_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char sc_cg_eis_rst_n : 8;
    } reg;
} REG_EIS_SOFT_RST_CTRL_UNION;
#endif
#define REG_EIS_SOFT_RST_CTRL_sc_cg_eis_rst_n_START (0)
#define REG_EIS_SOFT_RST_CTRL_sc_cg_eis_rst_n_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char sc_cg_com_clk_en : 1;
        unsigned char sc_cg_red_clk_en : 1;
        unsigned char sc_cg_pd_clk_en : 1;
        unsigned char sc_cg_2m_clk_en : 1;
        unsigned char sc_cg_pd_clk_en_sel : 1;
        unsigned char sc_cg_2m_clk_en_sel : 1;
        unsigned char reserved : 2;
    } reg;
} REG_SC_CRG_CLK_EN_CTRL_UNION;
#endif
#define REG_SC_CRG_CLK_EN_CTRL_sc_cg_com_clk_en_START (0)
#define REG_SC_CRG_CLK_EN_CTRL_sc_cg_com_clk_en_END (0)
#define REG_SC_CRG_CLK_EN_CTRL_sc_cg_red_clk_en_START (1)
#define REG_SC_CRG_CLK_EN_CTRL_sc_cg_red_clk_en_END (1)
#define REG_SC_CRG_CLK_EN_CTRL_sc_cg_pd_clk_en_START (2)
#define REG_SC_CRG_CLK_EN_CTRL_sc_cg_pd_clk_en_END (2)
#define REG_SC_CRG_CLK_EN_CTRL_sc_cg_2m_clk_en_START (3)
#define REG_SC_CRG_CLK_EN_CTRL_sc_cg_2m_clk_en_END (3)
#define REG_SC_CRG_CLK_EN_CTRL_sc_cg_pd_clk_en_sel_START (4)
#define REG_SC_CRG_CLK_EN_CTRL_sc_cg_pd_clk_en_sel_END (4)
#define REG_SC_CRG_CLK_EN_CTRL_sc_cg_2m_clk_en_sel_START (5)
#define REG_SC_CRG_CLK_EN_CTRL_sc_cg_2m_clk_en_sel_END (5)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char sc_eisclk_ok_sel : 1;
        unsigned char sc_cg_eis_clk_en : 1;
        unsigned char sc_cg_eisext_unlck_en : 1;
        unsigned char reserved : 5;
    } reg;
} REG_SC_CRG_CLK_EN_CTRL_1_UNION;
#endif
#define REG_SC_CRG_CLK_EN_CTRL_1_sc_eisclk_ok_sel_START (0)
#define REG_SC_CRG_CLK_EN_CTRL_1_sc_eisclk_ok_sel_END (0)
#define REG_SC_CRG_CLK_EN_CTRL_1_sc_cg_eis_clk_en_START (1)
#define REG_SC_CRG_CLK_EN_CTRL_1_sc_cg_eis_clk_en_END (1)
#define REG_SC_CRG_CLK_EN_CTRL_1_sc_cg_eisext_unlck_en_START (2)
#define REG_SC_CRG_CLK_EN_CTRL_1_sc_cg_eisext_unlck_en_END (2)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char sc_coul_clk_en_sel : 1;
        unsigned char sc_cg_coul_clk_en : 1;
        unsigned char reserved : 6;
    } reg;
} REG_SC_CRG_CLK_EN_CTRL_2_UNION;
#endif
#define REG_SC_CRG_CLK_EN_CTRL_2_sc_coul_clk_en_sel_START (0)
#define REG_SC_CRG_CLK_EN_CTRL_2_sc_coul_clk_en_sel_END (0)
#define REG_SC_CRG_CLK_EN_CTRL_2_sc_cg_coul_clk_en_START (1)
#define REG_SC_CRG_CLK_EN_CTRL_2_sc_cg_coul_clk_en_END (1)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char sc_cg_testsig_sel : 7;
        unsigned char reserved : 1;
    } reg;
} REG_SC_TESTSIG_CFG_UNION;
#endif
#define REG_SC_TESTSIG_CFG_sc_cg_testsig_sel_START (0)
#define REG_SC_TESTSIG_CFG_sc_cg_testsig_sel_END (6)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char sc_cg_soh_soft_rst_n : 8;
    } reg;
} REG_SOH_SOFT_RST_CTRL_UNION;
#endif
#define REG_SOH_SOFT_RST_CTRL_sc_cg_soh_soft_rst_n_START (0)
#define REG_SOH_SOFT_RST_CTRL_sc_cg_soh_soft_rst_n_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char sc_wdt_en : 1;
        unsigned char reserved : 7;
    } reg;
} REG_SC_WDT_EN_UNION;
#endif
#define REG_SC_WDT_EN_sc_wdt_en_START (0)
#define REG_SC_WDT_EN_sc_wdt_en_END (0)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char wd_rst_n : 1;
        unsigned char reserved : 7;
    } reg;
} REG_WDT_SOFT_RST_UNION;
#endif
#define REG_WDT_SOFT_RST_wd_rst_n_START (0)
#define REG_WDT_SOFT_RST_wd_rst_n_END (0)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char sc_watchdog_timer : 2;
        unsigned char reserved_0 : 2;
        unsigned char sc_wdt_test_sel : 1;
        unsigned char reserved_1 : 3;
    } reg;
} REG_WDT_CTRL_UNION;
#endif
#define REG_WDT_CTRL_sc_watchdog_timer_START (0)
#define REG_WDT_CTRL_sc_watchdog_timer_END (1)
#define REG_WDT_CTRL_sc_wdt_test_sel_START (4)
#define REG_WDT_CTRL_sc_wdt_test_sel_END (4)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char wdt_time_out_n : 1;
        unsigned char reserved : 7;
    } reg;
} REG_WDT_TIMEOUT_UNION;
#endif
#define REG_WDT_TIMEOUT_wdt_time_out_n_START (0)
#define REG_WDT_TIMEOUT_wdt_time_out_n_END (0)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char sc_bat_cnct_sel : 1;
        unsigned char reserved : 7;
    } reg;
} REG_BAT_CNCT_SEL_UNION;
#endif
#define REG_BAT_CNCT_SEL_sc_bat_cnct_sel_START (0)
#define REG_BAT_CNCT_SEL_sc_bat_cnct_sel_END (0)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char sc_dbfet_sel : 1;
        unsigned char reserved : 7;
    } reg;
} REG_DBFET_SEL_UNION;
#endif
#define REG_DBFET_SEL_sc_dbfet_sel_START (0)
#define REG_DBFET_SEL_sc_dbfet_sel_END (0)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char sc_dbfet_sel_en : 1;
        unsigned char reserved : 7;
    } reg;
} REG_DBFET_SEL_EN_UNION;
#endif
#define REG_DBFET_SEL_EN_sc_dbfet_sel_en_START (0)
#define REG_DBFET_SEL_EN_sc_dbfet_sel_en_END (0)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char sc_buck_enb : 1;
        unsigned char reserved : 7;
    } reg;
} REG_SC_BUCK_ENB_UNION;
#endif
#define REG_SC_BUCK_ENB_sc_buck_enb_START (0)
#define REG_SC_BUCK_ENB_sc_buck_enb_END (0)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char sc_b_die_en : 1;
        unsigned char reserved : 7;
    } reg;
} REG_B_DIE_EN_UNION;
#endif
#define REG_B_DIE_EN_sc_b_die_en_START (0)
#define REG_B_DIE_EN_sc_b_die_en_END (0)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char sc_bat_sw_en : 1;
        unsigned char reserved : 7;
    } reg;
} REG_BAT_SW_EN_UNION;
#endif
#define REG_BAT_SW_EN_sc_bat_sw_en_START (0)
#define REG_BAT_SW_EN_sc_bat_sw_en_END (0)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char da_lowbat_en : 1;
        unsigned char reserved : 7;
    } reg;
} REG_LOWBAT_EN_UNION;
#endif
#define REG_LOWBAT_EN_da_lowbat_en_START (0)
#define REG_LOWBAT_EN_da_lowbat_en_END (0)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char sc_ovp_en : 1;
        unsigned char reserved : 7;
    } reg;
} REG_OVP_EN_UNION;
#endif
#define REG_OVP_EN_sc_ovp_en_START (0)
#define REG_OVP_EN_sc_ovp_en_END (0)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char sc_ovp_mode : 1;
        unsigned char reserved : 7;
    } reg;
} REG_OVP_MODE_UNION;
#endif
#define REG_OVP_MODE_sc_ovp_mode_START (0)
#define REG_OVP_MODE_sc_ovp_mode_END (0)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char sc_psw_en : 1;
        unsigned char reserved : 7;
    } reg;
} REG_PSW_EN_UNION;
#endif
#define REG_PSW_EN_sc_psw_en_START (0)
#define REG_PSW_EN_sc_psw_en_END (0)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char sc_psw_mode : 1;
        unsigned char reserved : 7;
    } reg;
} REG_PSW_MODE_UNION;
#endif
#define REG_PSW_MODE_sc_psw_mode_START (0)
#define REG_PSW_MODE_sc_psw_mode_END (0)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char sc_aux_sbu_en : 1;
        unsigned char reserved : 7;
    } reg;
} REG_AUX_SBU_EN_UNION;
#endif
#define REG_AUX_SBU_EN_sc_aux_sbu_en_START (0)
#define REG_AUX_SBU_EN_sc_aux_sbu_en_END (0)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char ad_bat_sel : 1;
        unsigned char reserved : 7;
    } reg;
} REG_BAT_SEL_STATUS_UNION;
#endif
#define REG_BAT_SEL_STATUS_ad_bat_sel_START (0)
#define REG_BAT_SEL_STATUS_ad_bat_sel_END (0)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char sc_efuse_pdob_sel : 1;
        unsigned char reserved : 7;
    } reg;
} REG_EFUSE_PDOB_SEL_UNION;
#endif
#define REG_EFUSE_PDOB_SEL_sc_efuse_pdob_sel_START (0)
#define REG_EFUSE_PDOB_SEL_sc_efuse_pdob_sel_END (0)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char sc_efuse_prog_int : 1;
        unsigned char sc_efuse_prog_sel : 1;
        unsigned char sc_efuse_inctrl_sel : 1;
        unsigned char reserved : 5;
    } reg;
} REG_EFUSE_MODE_UNION;
#endif
#define REG_EFUSE_MODE_sc_efuse_prog_int_START (0)
#define REG_EFUSE_MODE_sc_efuse_prog_int_END (0)
#define REG_EFUSE_MODE_sc_efuse_prog_sel_START (1)
#define REG_EFUSE_MODE_sc_efuse_prog_sel_END (1)
#define REG_EFUSE_MODE_sc_efuse_inctrl_sel_START (2)
#define REG_EFUSE_MODE_sc_efuse_inctrl_sel_END (2)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char sc_efuse_rd_ctrl : 1;
        unsigned char reserved : 7;
    } reg;
} REG_EFUSE_RD_CTRL_UNION;
#endif
#define REG_EFUSE_RD_CTRL_sc_efuse_rd_ctrl_START (0)
#define REG_EFUSE_RD_CTRL_sc_efuse_rd_ctrl_END (0)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char sc_efuse_aen_cfg : 1;
        unsigned char reserved : 7;
    } reg;
} REG_EFUSE_AEN_UNION;
#endif
#define REG_EFUSE_AEN_sc_efuse_aen_cfg_START (0)
#define REG_EFUSE_AEN_sc_efuse_aen_cfg_END (0)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char sc_efuse_pgmen_cfg : 1;
        unsigned char reserved : 7;
    } reg;
} REG_EFUSE_PGMEN_UNION;
#endif
#define REG_EFUSE_PGMEN_sc_efuse_pgmen_cfg_START (0)
#define REG_EFUSE_PGMEN_sc_efuse_pgmen_cfg_END (0)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char sc_efuse_rden_cfg : 1;
        unsigned char reserved : 7;
    } reg;
} REG_EFUSE_RDEN_UNION;
#endif
#define REG_EFUSE_RDEN_sc_efuse_rden_cfg_START (0)
#define REG_EFUSE_RDEN_sc_efuse_rden_cfg_END (0)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char sc_efuse_a_cfg : 8;
    } reg;
} REG_EFUSE_ADDR_UNION;
#endif
#define REG_EFUSE_ADDR_sc_efuse_a_cfg_START (0)
#define REG_EFUSE_ADDR_sc_efuse_a_cfg_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char sc_efuse_a_cfg_h : 8;
    } reg;
} REG_EFUSE_ADDR_H_UNION;
#endif
#define REG_EFUSE_ADDR_H_sc_efuse_a_cfg_h_START (0)
#define REG_EFUSE_ADDR_H_sc_efuse_a_cfg_h_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char sc_efuse_pdob_pre_addr : 6;
        unsigned char reserved : 1;
        unsigned char sc_efuse_pdob_pre_we : 1;
    } reg;
} REG_EFUSE_PDOB_PRE_ADDR_WE_UNION;
#endif
#define REG_EFUSE_PDOB_PRE_ADDR_WE_sc_efuse_pdob_pre_addr_START (0)
#define REG_EFUSE_PDOB_PRE_ADDR_WE_sc_efuse_pdob_pre_addr_END (5)
#define REG_EFUSE_PDOB_PRE_ADDR_WE_sc_efuse_pdob_pre_we_START (7)
#define REG_EFUSE_PDOB_PRE_ADDR_WE_sc_efuse_pdob_pre_we_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char sc_efuse_pdob_pre_wdata : 8;
    } reg;
} REG_EFUSE_PDOB_PRE_WDATA_UNION;
#endif
#define REG_EFUSE_PDOB_PRE_WDATA_sc_efuse_pdob_pre_wdata_START (0)
#define REG_EFUSE_PDOB_PRE_WDATA_sc_efuse_pdob_pre_wdata_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char tb_efuse_state : 4;
        unsigned char tb_efuse_por_int_ro : 1;
        unsigned char tb_efuse_rd_done : 1;
        unsigned char reserved : 2;
    } reg;
} REG_EFUSE_STATUS_UNION;
#endif
#define REG_EFUSE_STATUS_tb_efuse_state_START (0)
#define REG_EFUSE_STATUS_tb_efuse_state_END (3)
#define REG_EFUSE_STATUS_tb_efuse_por_int_ro_START (4)
#define REG_EFUSE_STATUS_tb_efuse_por_int_ro_END (4)
#define REG_EFUSE_STATUS_tb_efuse_rd_done_START (5)
#define REG_EFUSE_STATUS_tb_efuse_rd_done_END (5)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char sc_efuse_state_reset : 1;
        unsigned char reserved : 7;
    } reg;
} REG_EFUSE_FSM_RESET_UNION;
#endif
#define REG_EFUSE_FSM_RESET_sc_efuse_state_reset_START (0)
#define REG_EFUSE_FSM_RESET_sc_efuse_state_reset_END (0)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char sc_efuse1_testbus_sel : 7;
        unsigned char sc_efuse_testbus_en : 1;
    } reg;
} REG_EFUSE_TESTBUS_CFG_UNION;
#endif
#define REG_EFUSE_TESTBUS_CFG_sc_efuse1_testbus_sel_START (0)
#define REG_EFUSE_TESTBUS_CFG_sc_efuse1_testbus_sel_END (6)
#define REG_EFUSE_TESTBUS_CFG_sc_efuse_testbus_en_START (7)
#define REG_EFUSE_TESTBUS_CFG_sc_efuse_testbus_en_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char efuse_testbus_rdata : 8;
    } reg;
} REG_EFUSE_TESTBUS_RDATA_UNION;
#endif
#define REG_EFUSE_TESTBUS_RDATA_efuse_testbus_rdata_START (0)
#define REG_EFUSE_TESTBUS_RDATA_efuse_testbus_rdata_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char sc_efuse_a_cal_sel : 1;
        unsigned char reserved : 7;
    } reg;
} REG_EFUSE_ADDR_CAL_SEL_UNION;
#endif
#define REG_EFUSE_ADDR_CAL_SEL_sc_efuse_a_cal_sel_START (0)
#define REG_EFUSE_ADDR_CAL_SEL_sc_efuse_a_cal_sel_END (0)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char sc_chg_otg_mode : 2;
        unsigned char reserved : 6;
    } reg;
} REG_CHG_OTG_MODE_CFG_UNION;
#endif
#define REG_CHG_OTG_MODE_CFG_sc_chg_otg_mode_START (0)
#define REG_CHG_OTG_MODE_CFG_sc_chg_otg_mode_END (1)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char sc_otg_dmd_ofs : 4;
        unsigned char sc_otg_dmd_ramp : 3;
        unsigned char reserved : 1;
    } reg;
} REG_OTG_DMD_CFG_UNION;
#endif
#define REG_OTG_DMD_CFG_sc_otg_dmd_ofs_START (0)
#define REG_OTG_DMD_CFG_sc_otg_dmd_ofs_END (3)
#define REG_OTG_DMD_CFG_sc_otg_dmd_ramp_START (4)
#define REG_OTG_DMD_CFG_sc_otg_dmd_ramp_END (6)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char sc_buck_cap3_ea : 2;
        unsigned char sc_buck_cap2_ea : 2;
        unsigned char sc_buck_cap1_ea : 3;
        unsigned char sc_buck_acl_downen : 1;
    } reg;
} REG_BUCK_CFG_REG_0_UNION;
#endif
#define REG_BUCK_CFG_REG_0_sc_buck_cap3_ea_START (0)
#define REG_BUCK_CFG_REG_0_sc_buck_cap3_ea_END (1)
#define REG_BUCK_CFG_REG_0_sc_buck_cap2_ea_START (2)
#define REG_BUCK_CFG_REG_0_sc_buck_cap2_ea_END (3)
#define REG_BUCK_CFG_REG_0_sc_buck_cap1_ea_START (4)
#define REG_BUCK_CFG_REG_0_sc_buck_cap1_ea_END (6)
#define REG_BUCK_CFG_REG_0_sc_buck_acl_downen_START (7)
#define REG_BUCK_CFG_REG_0_sc_buck_acl_downen_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char sc_buck_dt_lshs : 1;
        unsigned char sc_buck_dt_hsls : 1;
        unsigned char sc_buck_dpm_gm_3k : 1;
        unsigned char sc_buck_cap7_acl : 2;
        unsigned char sc_buck_cap6_dpm : 2;
        unsigned char reserved : 1;
    } reg;
} REG_BUCK_CFG_REG_1_UNION;
#endif
#define REG_BUCK_CFG_REG_1_sc_buck_dt_lshs_START (0)
#define REG_BUCK_CFG_REG_1_sc_buck_dt_lshs_END (0)
#define REG_BUCK_CFG_REG_1_sc_buck_dt_hsls_START (1)
#define REG_BUCK_CFG_REG_1_sc_buck_dt_hsls_END (1)
#define REG_BUCK_CFG_REG_1_sc_buck_dpm_gm_3k_START (2)
#define REG_BUCK_CFG_REG_1_sc_buck_dpm_gm_3k_END (2)
#define REG_BUCK_CFG_REG_1_sc_buck_cap7_acl_START (3)
#define REG_BUCK_CFG_REG_1_sc_buck_cap7_acl_END (4)
#define REG_BUCK_CFG_REG_1_sc_buck_cap6_dpm_START (5)
#define REG_BUCK_CFG_REG_1_sc_buck_cap6_dpm_END (6)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char sc_buck_hmos_fall : 3;
        unsigned char sc_buck_dt_sel : 2;
        unsigned char sc_buck_dt_lshs_delay : 2;
        unsigned char reserved : 1;
    } reg;
} REG_BUCK_CFG_REG_2_UNION;
#endif
#define REG_BUCK_CFG_REG_2_sc_buck_hmos_fall_START (0)
#define REG_BUCK_CFG_REG_2_sc_buck_hmos_fall_END (2)
#define REG_BUCK_CFG_REG_2_sc_buck_dt_sel_START (3)
#define REG_BUCK_CFG_REG_2_sc_buck_dt_sel_END (4)
#define REG_BUCK_CFG_REG_2_sc_buck_dt_lshs_delay_START (5)
#define REG_BUCK_CFG_REG_2_sc_buck_dt_lshs_delay_END (6)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char sc_buck_lmos_fall : 3;
        unsigned char sc_buck_hmos_rise : 3;
        unsigned char reserved : 2;
    } reg;
} REG_BUCK_CFG_REG_3_UNION;
#endif
#define REG_BUCK_CFG_REG_3_sc_buck_lmos_fall_START (0)
#define REG_BUCK_CFG_REG_3_sc_buck_lmos_fall_END (2)
#define REG_BUCK_CFG_REG_3_sc_buck_hmos_rise_START (3)
#define REG_BUCK_CFG_REG_3_sc_buck_hmos_rise_END (5)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char sc_buck_res13_dpm_pz : 2;
        unsigned char sc_buck_res1_ea : 3;
        unsigned char sc_buck_lmos_rise : 3;
    } reg;
} REG_BUCK_CFG_REG_4_UNION;
#endif
#define REG_BUCK_CFG_REG_4_sc_buck_res13_dpm_pz_START (0)
#define REG_BUCK_CFG_REG_4_sc_buck_res13_dpm_pz_END (1)
#define REG_BUCK_CFG_REG_4_sc_buck_res1_ea_START (2)
#define REG_BUCK_CFG_REG_4_sc_buck_res1_ea_END (4)
#define REG_BUCK_CFG_REG_4_sc_buck_lmos_rise_START (5)
#define REG_BUCK_CFG_REG_4_sc_buck_lmos_rise_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char sc_buck_res2_ea : 2;
        unsigned char sc_buck_res15_acl_z : 2;
        unsigned char sc_buck_res14_acl_p : 2;
        unsigned char reserved : 2;
    } reg;
} REG_BUCK_CFG_REG_5_UNION;
#endif
#define REG_BUCK_CFG_REG_5_sc_buck_res2_ea_START (0)
#define REG_BUCK_CFG_REG_5_sc_buck_res2_ea_END (1)
#define REG_BUCK_CFG_REG_5_sc_buck_res15_acl_z_START (2)
#define REG_BUCK_CFG_REG_5_sc_buck_res15_acl_z_END (3)
#define REG_BUCK_CFG_REG_5_sc_buck_res14_acl_p_START (4)
#define REG_BUCK_CFG_REG_5_sc_buck_res14_acl_p_END (5)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char sc_buck_res5_acl_rout : 2;
        unsigned char sc_buck_res4_dpm_gm : 2;
        unsigned char sc_buck_res3_ea : 3;
        unsigned char reserved : 1;
    } reg;
} REG_BUCK_CFG_REG_6_UNION;
#endif
#define REG_BUCK_CFG_REG_6_sc_buck_res5_acl_rout_START (0)
#define REG_BUCK_CFG_REG_6_sc_buck_res5_acl_rout_END (1)
#define REG_BUCK_CFG_REG_6_sc_buck_res4_dpm_gm_START (2)
#define REG_BUCK_CFG_REG_6_sc_buck_res4_dpm_gm_END (3)
#define REG_BUCK_CFG_REG_6_sc_buck_res3_ea_START (4)
#define REG_BUCK_CFG_REG_6_sc_buck_res3_ea_END (6)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char sc_buck_reserve : 8;
    } reg;
} REG_BUCK_CFG_REG_7_UNION;
#endif
#define REG_BUCK_CFG_REG_7_sc_buck_reserve_START (0)
#define REG_BUCK_CFG_REG_7_sc_buck_reserve_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char sc_buck_ss_speed : 2;
        unsigned char reserved : 6;
    } reg;
} REG_BUCK_CFG_REG_8_UNION;
#endif
#define REG_BUCK_CFG_REG_8_sc_buck_ss_speed_START (0)
#define REG_BUCK_CFG_REG_8_sc_buck_ss_speed_END (1)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char sc_otg_cap3_ea : 2;
        unsigned char sc_otg_cap2_ea : 2;
        unsigned char sc_otg_cap1_ea : 3;
        unsigned char sc_otg_acl_downen : 1;
    } reg;
} REG_OTG_CFG_REG_0_UNION;
#endif
#define REG_OTG_CFG_REG_0_sc_otg_cap3_ea_START (0)
#define REG_OTG_CFG_REG_0_sc_otg_cap3_ea_END (1)
#define REG_OTG_CFG_REG_0_sc_otg_cap2_ea_START (2)
#define REG_OTG_CFG_REG_0_sc_otg_cap2_ea_END (3)
#define REG_OTG_CFG_REG_0_sc_otg_cap1_ea_START (4)
#define REG_OTG_CFG_REG_0_sc_otg_cap1_ea_END (6)
#define REG_OTG_CFG_REG_0_sc_otg_acl_downen_START (7)
#define REG_OTG_CFG_REG_0_sc_otg_acl_downen_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char sc_otg_dt_lshs : 1;
        unsigned char sc_otg_dt_hsls : 1;
        unsigned char sc_otg_dpm_gm_3k : 1;
        unsigned char sc_otg_cap7_acl : 2;
        unsigned char sc_otg_cap6_dpm : 2;
        unsigned char reserved : 1;
    } reg;
} REG_OTG_CFG_REG_1_UNION;
#endif
#define REG_OTG_CFG_REG_1_sc_otg_dt_lshs_START (0)
#define REG_OTG_CFG_REG_1_sc_otg_dt_lshs_END (0)
#define REG_OTG_CFG_REG_1_sc_otg_dt_hsls_START (1)
#define REG_OTG_CFG_REG_1_sc_otg_dt_hsls_END (1)
#define REG_OTG_CFG_REG_1_sc_otg_dpm_gm_3k_START (2)
#define REG_OTG_CFG_REG_1_sc_otg_dpm_gm_3k_END (2)
#define REG_OTG_CFG_REG_1_sc_otg_cap7_acl_START (3)
#define REG_OTG_CFG_REG_1_sc_otg_cap7_acl_END (4)
#define REG_OTG_CFG_REG_1_sc_otg_cap6_dpm_START (5)
#define REG_OTG_CFG_REG_1_sc_otg_cap6_dpm_END (6)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char sc_otg_hmos_fall : 3;
        unsigned char sc_otg_dt_sel : 2;
        unsigned char sc_otg_dt_lshs_delay : 2;
        unsigned char reserved : 1;
    } reg;
} REG_OTG_CFG_REG_2_UNION;
#endif
#define REG_OTG_CFG_REG_2_sc_otg_hmos_fall_START (0)
#define REG_OTG_CFG_REG_2_sc_otg_hmos_fall_END (2)
#define REG_OTG_CFG_REG_2_sc_otg_dt_sel_START (3)
#define REG_OTG_CFG_REG_2_sc_otg_dt_sel_END (4)
#define REG_OTG_CFG_REG_2_sc_otg_dt_lshs_delay_START (5)
#define REG_OTG_CFG_REG_2_sc_otg_dt_lshs_delay_END (6)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char sc_otg_lmos_fall : 3;
        unsigned char sc_otg_hmos_rise : 3;
        unsigned char reserved : 2;
    } reg;
} REG_OTG_CFG_REG_3_UNION;
#endif
#define REG_OTG_CFG_REG_3_sc_otg_lmos_fall_START (0)
#define REG_OTG_CFG_REG_3_sc_otg_lmos_fall_END (2)
#define REG_OTG_CFG_REG_3_sc_otg_hmos_rise_START (3)
#define REG_OTG_CFG_REG_3_sc_otg_hmos_rise_END (5)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char sc_otg_res13_dpm_pz : 2;
        unsigned char sc_otg_res1_ea : 3;
        unsigned char sc_otg_lmos_rise : 3;
    } reg;
} REG_OTG_CFG_REG_4_UNION;
#endif
#define REG_OTG_CFG_REG_4_sc_otg_res13_dpm_pz_START (0)
#define REG_OTG_CFG_REG_4_sc_otg_res13_dpm_pz_END (1)
#define REG_OTG_CFG_REG_4_sc_otg_res1_ea_START (2)
#define REG_OTG_CFG_REG_4_sc_otg_res1_ea_END (4)
#define REG_OTG_CFG_REG_4_sc_otg_lmos_rise_START (5)
#define REG_OTG_CFG_REG_4_sc_otg_lmos_rise_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char sc_otg_res2_ea : 2;
        unsigned char sc_otg_res15_acl_z : 2;
        unsigned char sc_otg_res14_acl_p : 2;
        unsigned char reserved : 2;
    } reg;
} REG_OTG_CFG_REG_5_UNION;
#endif
#define REG_OTG_CFG_REG_5_sc_otg_res2_ea_START (0)
#define REG_OTG_CFG_REG_5_sc_otg_res2_ea_END (1)
#define REG_OTG_CFG_REG_5_sc_otg_res15_acl_z_START (2)
#define REG_OTG_CFG_REG_5_sc_otg_res15_acl_z_END (3)
#define REG_OTG_CFG_REG_5_sc_otg_res14_acl_p_START (4)
#define REG_OTG_CFG_REG_5_sc_otg_res14_acl_p_END (5)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char sc_otg_res5_acl_rout : 2;
        unsigned char sc_otg_res4_dpm_gm : 2;
        unsigned char sc_otg_res3_ea : 3;
        unsigned char reserved : 1;
    } reg;
} REG_OTG_CFG_REG_6_UNION;
#endif
#define REG_OTG_CFG_REG_6_sc_otg_res5_acl_rout_START (0)
#define REG_OTG_CFG_REG_6_sc_otg_res5_acl_rout_END (1)
#define REG_OTG_CFG_REG_6_sc_otg_res4_dpm_gm_START (2)
#define REG_OTG_CFG_REG_6_sc_otg_res4_dpm_gm_END (3)
#define REG_OTG_CFG_REG_6_sc_otg_res3_ea_START (4)
#define REG_OTG_CFG_REG_6_sc_otg_res3_ea_END (6)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char sc_otg_reserve : 8;
    } reg;
} REG_OTG_CFG_REG_7_UNION;
#endif
#define REG_OTG_CFG_REG_7_sc_otg_reserve_START (0)
#define REG_OTG_CFG_REG_7_sc_otg_reserve_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char sc_otg_ss_speed : 2;
        unsigned char reserved : 6;
    } reg;
} REG_OTG_CFG_REG_8_UNION;
#endif
#define REG_OTG_CFG_REG_8_sc_otg_ss_speed_START (0)
#define REG_OTG_CFG_REG_8_sc_otg_ss_speed_END (1)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char tb_chg_ctrl_status : 8;
    } reg;
} REG_CHG_STATUS_UNION;
#endif
#define REG_CHG_STATUS_tb_chg_ctrl_status_START (0)
#define REG_CHG_STATUS_tb_chg_ctrl_status_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char sc_chg_tb_sel : 6;
        unsigned char reserved : 1;
        unsigned char sc_chg_tb_en : 1;
    } reg;
} REG_CHG_TB_CFG_UNION;
#endif
#define REG_CHG_TB_CFG_sc_chg_tb_sel_START (0)
#define REG_CHG_TB_CFG_sc_chg_tb_sel_END (5)
#define REG_CHG_TB_CFG_sc_chg_tb_en_START (7)
#define REG_CHG_TB_CFG_sc_chg_tb_en_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char chg_tb_bus : 8;
    } reg;
} REG_CHG_TB_BUS_UNION;
#endif
#define REG_CHG_TB_BUS_chg_tb_bus_START (0)
#define REG_CHG_TB_BUS_chg_tb_bus_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char chg_tb_sig : 1;
        unsigned char reserved : 7;
    } reg;
} REG_CHG_TB_SIG_UNION;
#endif
#define REG_CHG_TB_SIG_chg_tb_sig_START (0)
#define REG_CHG_TB_SIG_chg_tb_sig_END (0)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char efs_coul_trim_0 : 8;
    } reg;
} REG_EFS_COUL_TRIM_0_UNION;
#endif
#define REG_EFS_COUL_TRIM_0_efs_coul_trim_0_START (0)
#define REG_EFS_COUL_TRIM_0_efs_coul_trim_0_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char efs_coul_trim_1 : 8;
    } reg;
} REG_EFS_COUL_TRIM_1_UNION;
#endif
#define REG_EFS_COUL_TRIM_1_efs_coul_trim_1_START (0)
#define REG_EFS_COUL_TRIM_1_efs_coul_trim_1_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char efs_coul_trim_2 : 8;
    } reg;
} REG_EFS_COUL_TRIM_2_UNION;
#endif
#define REG_EFS_COUL_TRIM_2_efs_coul_trim_2_START (0)
#define REG_EFS_COUL_TRIM_2_efs_coul_trim_2_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char efs_coul_trim_3 : 8;
    } reg;
} REG_EFS_COUL_TRIM_3_UNION;
#endif
#define REG_EFS_COUL_TRIM_3_efs_coul_trim_3_START (0)
#define REG_EFS_COUL_TRIM_3_efs_coul_trim_3_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char efs_coul_trim_4 : 8;
    } reg;
} REG_EFS_COUL_TRIM_4_UNION;
#endif
#define REG_EFS_COUL_TRIM_4_efs_coul_trim_4_START (0)
#define REG_EFS_COUL_TRIM_4_efs_coul_trim_4_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char efs_coul_trim_5 : 8;
    } reg;
} REG_EFS_COUL_TRIM_5_UNION;
#endif
#define REG_EFS_COUL_TRIM_5_efs_coul_trim_5_START (0)
#define REG_EFS_COUL_TRIM_5_efs_coul_trim_5_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char efs_coul_trim_6 : 8;
    } reg;
} REG_EFS_COUL_TRIM_6_UNION;
#endif
#define REG_EFS_COUL_TRIM_6_efs_coul_trim_6_START (0)
#define REG_EFS_COUL_TRIM_6_efs_coul_trim_6_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char efs_coul_trim_7 : 8;
    } reg;
} REG_EFS_COUL_TRIM_7_UNION;
#endif
#define REG_EFS_COUL_TRIM_7_efs_coul_trim_7_START (0)
#define REG_EFS_COUL_TRIM_7_efs_coul_trim_7_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char efs_coul_trim_8 : 8;
    } reg;
} REG_EFS_COUL_TRIM_8_UNION;
#endif
#define REG_EFS_COUL_TRIM_8_efs_coul_trim_8_START (0)
#define REG_EFS_COUL_TRIM_8_efs_coul_trim_8_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char efs_coul_trim_9 : 8;
    } reg;
} REG_EFS_COUL_TRIM_9_UNION;
#endif
#define REG_EFS_COUL_TRIM_9_efs_coul_trim_9_START (0)
#define REG_EFS_COUL_TRIM_9_efs_coul_trim_9_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char efs_coul_trim_10 : 8;
    } reg;
} REG_EFS_COUL_TRIM_10_UNION;
#endif
#define REG_EFS_COUL_TRIM_10_efs_coul_trim_10_START (0)
#define REG_EFS_COUL_TRIM_10_efs_coul_trim_10_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char efs_coul_trim_11 : 8;
    } reg;
} REG_EFS_COUL_TRIM_11_UNION;
#endif
#define REG_EFS_COUL_TRIM_11_efs_coul_trim_11_START (0)
#define REG_EFS_COUL_TRIM_11_efs_coul_trim_11_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char efs_adc_ibus_trim_ro : 5;
        unsigned char reserved : 3;
    } reg;
} REG_EFS_ADC_TRIM_0_UNION;
#endif
#define REG_EFS_ADC_TRIM_0_efs_adc_ibus_trim_ro_START (0)
#define REG_EFS_ADC_TRIM_0_efs_adc_ibus_trim_ro_END (4)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char efs_dig_resv_0 : 8;
    } reg;
} REG_EFS_DIG_TRIM_0_UNION;
#endif
#define REG_EFS_DIG_TRIM_0_efs_dig_resv_0_START (0)
#define REG_EFS_DIG_TRIM_0_efs_dig_resv_0_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char efs_dig_resv_1 : 8;
    } reg;
} REG_EFS_DIG_TRIM_1_UNION;
#endif
#define REG_EFS_DIG_TRIM_1_efs_dig_resv_1_START (0)
#define REG_EFS_DIG_TRIM_1_efs_dig_resv_1_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char eco_ctrl : 3;
        unsigned char reserved : 5;
    } reg;
} REG_CLJ_CTRL_REG_UNION;
#endif
#define REG_CLJ_CTRL_REG_eco_ctrl_START (0)
#define REG_CLJ_CTRL_REG_eco_ctrl_END (2)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char reserved_0 : 1;
        unsigned char coul_eco_dly_sel : 2;
        unsigned char wait_comp_en : 1;
        unsigned char wait_comp_sel : 2;
        unsigned char reserved_1 : 2;
    } reg;
} REG_CLJ_CTRL_REGS3_UNION;
#endif
#define REG_CLJ_CTRL_REGS3_coul_eco_dly_sel_START (1)
#define REG_CLJ_CTRL_REGS3_coul_eco_dly_sel_END (2)
#define REG_CLJ_CTRL_REGS3_wait_comp_en_START (3)
#define REG_CLJ_CTRL_REGS3_wait_comp_en_END (3)
#define REG_CLJ_CTRL_REGS3_wait_comp_sel_START (4)
#define REG_CLJ_CTRL_REGS3_wait_comp_sel_END (5)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char data_eco_en : 1;
        unsigned char eco_data_clr : 1;
        unsigned char coul_charge_en : 1;
        unsigned char coul_charge_flag : 1;
        unsigned char reserved : 4;
    } reg;
} REG_CLJ_CTRL_REGS4_UNION;
#endif
#define REG_CLJ_CTRL_REGS4_data_eco_en_START (0)
#define REG_CLJ_CTRL_REGS4_data_eco_en_END (0)
#define REG_CLJ_CTRL_REGS4_eco_data_clr_START (1)
#define REG_CLJ_CTRL_REGS4_eco_data_clr_END (1)
#define REG_CLJ_CTRL_REGS4_coul_charge_en_START (2)
#define REG_CLJ_CTRL_REGS4_coul_charge_en_END (2)
#define REG_CLJ_CTRL_REGS4_coul_charge_flag_START (3)
#define REG_CLJ_CTRL_REGS4_coul_charge_flag_END (3)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char slide_en : 1;
        unsigned char slide_refresh_en : 1;
        unsigned char slide_cnt_cfg : 2;
        unsigned char slide_ready : 1;
        unsigned char reserved : 3;
    } reg;
} REG_CLJ_CTRL_REGS5_UNION;
#endif
#define REG_CLJ_CTRL_REGS5_slide_en_START (0)
#define REG_CLJ_CTRL_REGS5_slide_en_END (0)
#define REG_CLJ_CTRL_REGS5_slide_refresh_en_START (1)
#define REG_CLJ_CTRL_REGS5_slide_refresh_en_END (1)
#define REG_CLJ_CTRL_REGS5_slide_cnt_cfg_START (2)
#define REG_CLJ_CTRL_REGS5_slide_cnt_cfg_END (3)
#define REG_CLJ_CTRL_REGS5_slide_ready_START (4)
#define REG_CLJ_CTRL_REGS5_slide_ready_END (4)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char slide_cnt_data_out0 : 8;
    } reg;
} REG_SLIDE_CNT_DATA_OUT0_UNION;
#endif
#define REG_SLIDE_CNT_DATA_OUT0_slide_cnt_data_out0_START (0)
#define REG_SLIDE_CNT_DATA_OUT0_slide_cnt_data_out0_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char slide_cnt_data_out1 : 8;
    } reg;
} REG_SLIDE_CNT_DATA_OUT1_UNION;
#endif
#define REG_SLIDE_CNT_DATA_OUT1_slide_cnt_data_out1_START (0)
#define REG_SLIDE_CNT_DATA_OUT1_slide_cnt_data_out1_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char slide_cnt_data_out2 : 4;
        unsigned char reserved : 4;
    } reg;
} REG_SLIDE_CNT_DATA_OUT2_UNION;
#endif
#define REG_SLIDE_CNT_DATA_OUT2_slide_cnt_data_out2_START (0)
#define REG_SLIDE_CNT_DATA_OUT2_slide_cnt_data_out2_END (3)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char slide_ih_data0_out0 : 8;
    } reg;
} REG_SLIDE_IH_DATA0_OUT0_UNION;
#endif
#define REG_SLIDE_IH_DATA0_OUT0_slide_ih_data0_out0_START (0)
#define REG_SLIDE_IH_DATA0_OUT0_slide_ih_data0_out0_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char slide_ih_data0_out1 : 8;
    } reg;
} REG_SLIDE_IH_DATA0_OUT1_UNION;
#endif
#define REG_SLIDE_IH_DATA0_OUT1_slide_ih_data0_out1_START (0)
#define REG_SLIDE_IH_DATA0_OUT1_slide_ih_data0_out1_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char slide_ih_data0_out2 : 8;
    } reg;
} REG_SLIDE_IH_DATA0_OUT2_UNION;
#endif
#define REG_SLIDE_IH_DATA0_OUT2_slide_ih_data0_out2_START (0)
#define REG_SLIDE_IH_DATA0_OUT2_slide_ih_data0_out2_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char slide_ih_data1_out0 : 8;
    } reg;
} REG_SLIDE_IH_DATA1_OUT0_UNION;
#endif
#define REG_SLIDE_IH_DATA1_OUT0_slide_ih_data1_out0_START (0)
#define REG_SLIDE_IH_DATA1_OUT0_slide_ih_data1_out0_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char slide_ih_data1_out1 : 8;
    } reg;
} REG_SLIDE_IH_DATA1_OUT1_UNION;
#endif
#define REG_SLIDE_IH_DATA1_OUT1_slide_ih_data1_out1_START (0)
#define REG_SLIDE_IH_DATA1_OUT1_slide_ih_data1_out1_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char slide_ih_data1_out2 : 8;
    } reg;
} REG_SLIDE_IH_DATA1_OUT2_UNION;
#endif
#define REG_SLIDE_IH_DATA1_OUT2_slide_ih_data1_out2_START (0)
#define REG_SLIDE_IH_DATA1_OUT2_slide_ih_data1_out2_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char slide_ih_data2_out0 : 8;
    } reg;
} REG_SLIDE_IH_DATA2_OUT0_UNION;
#endif
#define REG_SLIDE_IH_DATA2_OUT0_slide_ih_data2_out0_START (0)
#define REG_SLIDE_IH_DATA2_OUT0_slide_ih_data2_out0_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char slide_ih_data2_out1 : 8;
    } reg;
} REG_SLIDE_IH_DATA2_OUT1_UNION;
#endif
#define REG_SLIDE_IH_DATA2_OUT1_slide_ih_data2_out1_START (0)
#define REG_SLIDE_IH_DATA2_OUT1_slide_ih_data2_out1_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char slide_ih_data2_out2 : 8;
    } reg;
} REG_SLIDE_IH_DATA2_OUT2_UNION;
#endif
#define REG_SLIDE_IH_DATA2_OUT2_slide_ih_data2_out2_START (0)
#define REG_SLIDE_IH_DATA2_OUT2_slide_ih_data2_out2_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char slide_ih_data3_out0 : 8;
    } reg;
} REG_SLIDE_IH_DATA3_OUT0_UNION;
#endif
#define REG_SLIDE_IH_DATA3_OUT0_slide_ih_data3_out0_START (0)
#define REG_SLIDE_IH_DATA3_OUT0_slide_ih_data3_out0_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char slide_ih_data3_out1 : 8;
    } reg;
} REG_SLIDE_IH_DATA3_OUT1_UNION;
#endif
#define REG_SLIDE_IH_DATA3_OUT1_slide_ih_data3_out1_START (0)
#define REG_SLIDE_IH_DATA3_OUT1_slide_ih_data3_out1_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char slide_ih_data3_out2 : 8;
    } reg;
} REG_SLIDE_IH_DATA3_OUT2_UNION;
#endif
#define REG_SLIDE_IH_DATA3_OUT2_slide_ih_data3_out2_START (0)
#define REG_SLIDE_IH_DATA3_OUT2_slide_ih_data3_out2_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char slide_vh_data0_out0 : 8;
    } reg;
} REG_SLIDE_VH_DATA0_OUT0_UNION;
#endif
#define REG_SLIDE_VH_DATA0_OUT0_slide_vh_data0_out0_START (0)
#define REG_SLIDE_VH_DATA0_OUT0_slide_vh_data0_out0_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char slide_vh_data0_out1 : 8;
    } reg;
} REG_SLIDE_VH_DATA0_OUT1_UNION;
#endif
#define REG_SLIDE_VH_DATA0_OUT1_slide_vh_data0_out1_START (0)
#define REG_SLIDE_VH_DATA0_OUT1_slide_vh_data0_out1_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char slide_vh_data0_out2 : 8;
    } reg;
} REG_SLIDE_VH_DATA0_OUT2_UNION;
#endif
#define REG_SLIDE_VH_DATA0_OUT2_slide_vh_data0_out2_START (0)
#define REG_SLIDE_VH_DATA0_OUT2_slide_vh_data0_out2_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char slide_vh_data1_out0 : 8;
    } reg;
} REG_SLIDE_VH_DATA1_OUT0_UNION;
#endif
#define REG_SLIDE_VH_DATA1_OUT0_slide_vh_data1_out0_START (0)
#define REG_SLIDE_VH_DATA1_OUT0_slide_vh_data1_out0_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char slide_vh_data1_out1 : 8;
    } reg;
} REG_SLIDE_VH_DATA1_OUT1_UNION;
#endif
#define REG_SLIDE_VH_DATA1_OUT1_slide_vh_data1_out1_START (0)
#define REG_SLIDE_VH_DATA1_OUT1_slide_vh_data1_out1_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char slide_vh_data1_out2 : 8;
    } reg;
} REG_SLIDE_VH_DATA1_OUT2_UNION;
#endif
#define REG_SLIDE_VH_DATA1_OUT2_slide_vh_data1_out2_START (0)
#define REG_SLIDE_VH_DATA1_OUT2_slide_vh_data1_out2_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char slide_vh_data2_out0 : 8;
    } reg;
} REG_SLIDE_VH_DATA2_OUT0_UNION;
#endif
#define REG_SLIDE_VH_DATA2_OUT0_slide_vh_data2_out0_START (0)
#define REG_SLIDE_VH_DATA2_OUT0_slide_vh_data2_out0_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char slide_vh_data2_out1 : 8;
    } reg;
} REG_SLIDE_VH_DATA2_OUT1_UNION;
#endif
#define REG_SLIDE_VH_DATA2_OUT1_slide_vh_data2_out1_START (0)
#define REG_SLIDE_VH_DATA2_OUT1_slide_vh_data2_out1_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char slide_vh_data2_out2 : 8;
    } reg;
} REG_SLIDE_VH_DATA2_OUT2_UNION;
#endif
#define REG_SLIDE_VH_DATA2_OUT2_slide_vh_data2_out2_START (0)
#define REG_SLIDE_VH_DATA2_OUT2_slide_vh_data2_out2_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char slide_vh_data3_out0 : 8;
    } reg;
} REG_SLIDE_VH_DATA3_OUT0_UNION;
#endif
#define REG_SLIDE_VH_DATA3_OUT0_slide_vh_data3_out0_START (0)
#define REG_SLIDE_VH_DATA3_OUT0_slide_vh_data3_out0_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char slide_vh_data3_out1 : 8;
    } reg;
} REG_SLIDE_VH_DATA3_OUT1_UNION;
#endif
#define REG_SLIDE_VH_DATA3_OUT1_slide_vh_data3_out1_START (0)
#define REG_SLIDE_VH_DATA3_OUT1_slide_vh_data3_out1_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char slide_vh_data3_out2 : 8;
    } reg;
} REG_SLIDE_VH_DATA3_OUT2_UNION;
#endif
#define REG_SLIDE_VH_DATA3_OUT2_slide_vh_data3_out2_START (0)
#define REG_SLIDE_VH_DATA3_OUT2_slide_vh_data3_out2_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char slide_il_data0_out0 : 8;
    } reg;
} REG_SLIDE_IL_DATA0_OUT0_UNION;
#endif
#define REG_SLIDE_IL_DATA0_OUT0_slide_il_data0_out0_START (0)
#define REG_SLIDE_IL_DATA0_OUT0_slide_il_data0_out0_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char slide_il_data0_out1 : 8;
    } reg;
} REG_SLIDE_IL_DATA0_OUT1_UNION;
#endif
#define REG_SLIDE_IL_DATA0_OUT1_slide_il_data0_out1_START (0)
#define REG_SLIDE_IL_DATA0_OUT1_slide_il_data0_out1_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char slide_il_data0_out2 : 8;
    } reg;
} REG_SLIDE_IL_DATA0_OUT2_UNION;
#endif
#define REG_SLIDE_IL_DATA0_OUT2_slide_il_data0_out2_START (0)
#define REG_SLIDE_IL_DATA0_OUT2_slide_il_data0_out2_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char slide_il_data1_out0 : 8;
    } reg;
} REG_SLIDE_IL_DATA1_OUT0_UNION;
#endif
#define REG_SLIDE_IL_DATA1_OUT0_slide_il_data1_out0_START (0)
#define REG_SLIDE_IL_DATA1_OUT0_slide_il_data1_out0_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char slide_il_data1_out1 : 8;
    } reg;
} REG_SLIDE_IL_DATA1_OUT1_UNION;
#endif
#define REG_SLIDE_IL_DATA1_OUT1_slide_il_data1_out1_START (0)
#define REG_SLIDE_IL_DATA1_OUT1_slide_il_data1_out1_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char slide_il_data1_out2 : 8;
    } reg;
} REG_SLIDE_IL_DATA1_OUT2_UNION;
#endif
#define REG_SLIDE_IL_DATA1_OUT2_slide_il_data1_out2_START (0)
#define REG_SLIDE_IL_DATA1_OUT2_slide_il_data1_out2_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char slide_il_data2_out0 : 8;
    } reg;
} REG_SLIDE_IL_DATA2_OUT0_UNION;
#endif
#define REG_SLIDE_IL_DATA2_OUT0_slide_il_data2_out0_START (0)
#define REG_SLIDE_IL_DATA2_OUT0_slide_il_data2_out0_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char slide_il_data2_out1 : 8;
    } reg;
} REG_SLIDE_IL_DATA2_OUT1_UNION;
#endif
#define REG_SLIDE_IL_DATA2_OUT1_slide_il_data2_out1_START (0)
#define REG_SLIDE_IL_DATA2_OUT1_slide_il_data2_out1_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char slide_il_data2_out2 : 8;
    } reg;
} REG_SLIDE_IL_DATA2_OUT2_UNION;
#endif
#define REG_SLIDE_IL_DATA2_OUT2_slide_il_data2_out2_START (0)
#define REG_SLIDE_IL_DATA2_OUT2_slide_il_data2_out2_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char slide_il_data3_out0 : 8;
    } reg;
} REG_SLIDE_IL_DATA3_OUT0_UNION;
#endif
#define REG_SLIDE_IL_DATA3_OUT0_slide_il_data3_out0_START (0)
#define REG_SLIDE_IL_DATA3_OUT0_slide_il_data3_out0_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char slide_il_data3_out1 : 8;
    } reg;
} REG_SLIDE_IL_DATA3_OUT1_UNION;
#endif
#define REG_SLIDE_IL_DATA3_OUT1_slide_il_data3_out1_START (0)
#define REG_SLIDE_IL_DATA3_OUT1_slide_il_data3_out1_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char slide_il_data3_out2 : 8;
    } reg;
} REG_SLIDE_IL_DATA3_OUT2_UNION;
#endif
#define REG_SLIDE_IL_DATA3_OUT2_slide_il_data3_out2_START (0)
#define REG_SLIDE_IL_DATA3_OUT2_slide_il_data3_out2_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char slide_vl_data0_out0 : 8;
    } reg;
} REG_SLIDE_VL_DATA0_OUT0_UNION;
#endif
#define REG_SLIDE_VL_DATA0_OUT0_slide_vl_data0_out0_START (0)
#define REG_SLIDE_VL_DATA0_OUT0_slide_vl_data0_out0_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char slide_vl_data0_out1 : 8;
    } reg;
} REG_SLIDE_VL_DATA0_OUT1_UNION;
#endif
#define REG_SLIDE_VL_DATA0_OUT1_slide_vl_data0_out1_START (0)
#define REG_SLIDE_VL_DATA0_OUT1_slide_vl_data0_out1_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char slide_vl_data0_out2 : 8;
    } reg;
} REG_SLIDE_VL_DATA0_OUT2_UNION;
#endif
#define REG_SLIDE_VL_DATA0_OUT2_slide_vl_data0_out2_START (0)
#define REG_SLIDE_VL_DATA0_OUT2_slide_vl_data0_out2_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char slide_vl_data1_out0 : 8;
    } reg;
} REG_SLIDE_VL_DATA1_OUT0_UNION;
#endif
#define REG_SLIDE_VL_DATA1_OUT0_slide_vl_data1_out0_START (0)
#define REG_SLIDE_VL_DATA1_OUT0_slide_vl_data1_out0_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char slide_vl_data1_out1 : 8;
    } reg;
} REG_SLIDE_VL_DATA1_OUT1_UNION;
#endif
#define REG_SLIDE_VL_DATA1_OUT1_slide_vl_data1_out1_START (0)
#define REG_SLIDE_VL_DATA1_OUT1_slide_vl_data1_out1_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char slide_vl_data1_out2 : 8;
    } reg;
} REG_SLIDE_VL_DATA1_OUT2_UNION;
#endif
#define REG_SLIDE_VL_DATA1_OUT2_slide_vl_data1_out2_START (0)
#define REG_SLIDE_VL_DATA1_OUT2_slide_vl_data1_out2_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char slide_vl_data2_out0 : 8;
    } reg;
} REG_SLIDE_VL_DATA2_OUT0_UNION;
#endif
#define REG_SLIDE_VL_DATA2_OUT0_slide_vl_data2_out0_START (0)
#define REG_SLIDE_VL_DATA2_OUT0_slide_vl_data2_out0_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char slide_vl_data2_out1 : 8;
    } reg;
} REG_SLIDE_VL_DATA2_OUT1_UNION;
#endif
#define REG_SLIDE_VL_DATA2_OUT1_slide_vl_data2_out1_START (0)
#define REG_SLIDE_VL_DATA2_OUT1_slide_vl_data2_out1_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char slide_vl_data2_out2 : 8;
    } reg;
} REG_SLIDE_VL_DATA2_OUT2_UNION;
#endif
#define REG_SLIDE_VL_DATA2_OUT2_slide_vl_data2_out2_START (0)
#define REG_SLIDE_VL_DATA2_OUT2_slide_vl_data2_out2_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char slide_vl_data3_out0 : 8;
    } reg;
} REG_SLIDE_VL_DATA3_OUT0_UNION;
#endif
#define REG_SLIDE_VL_DATA3_OUT0_slide_vl_data3_out0_START (0)
#define REG_SLIDE_VL_DATA3_OUT0_slide_vl_data3_out0_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char slide_vl_data3_out1 : 8;
    } reg;
} REG_SLIDE_VL_DATA3_OUT1_UNION;
#endif
#define REG_SLIDE_VL_DATA3_OUT1_slide_vl_data3_out1_START (0)
#define REG_SLIDE_VL_DATA3_OUT1_slide_vl_data3_out1_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char slide_vl_data3_out2 : 8;
    } reg;
} REG_SLIDE_VL_DATA3_OUT2_UNION;
#endif
#define REG_SLIDE_VL_DATA3_OUT2_slide_vl_data3_out2_START (0)
#define REG_SLIDE_VL_DATA3_OUT2_slide_vl_data3_out2_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char irq_l_coul_flag : 1;
        unsigned char irq_h_coul_flag : 1;
        unsigned char reserved : 6;
    } reg;
} REG_COUL_IRQ_FLAG_UNION;
#endif
#define REG_COUL_IRQ_FLAG_irq_l_coul_flag_START (0)
#define REG_COUL_IRQ_FLAG_irq_l_coul_flag_END (0)
#define REG_COUL_IRQ_FLAG_irq_h_coul_flag_START (1)
#define REG_COUL_IRQ_FLAG_irq_h_coul_flag_END (1)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char irq_h_coul_cl_gate_flag : 1;
        unsigned char irq_h_coul_cl_out_flag : 1;
        unsigned char irq_h_coul_cl_in_flag : 1;
        unsigned char irq_h_coul_v_gate_flag : 1;
        unsigned char irq_h_coul_i_in_flag : 1;
        unsigned char irq_h_coul_i_out_flag : 1;
        unsigned char reserved : 2;
    } reg;
} REG_COUL_H_IRQ_FLAG_UNION;
#endif
#define REG_COUL_H_IRQ_FLAG_irq_h_coul_cl_gate_flag_START (0)
#define REG_COUL_H_IRQ_FLAG_irq_h_coul_cl_gate_flag_END (0)
#define REG_COUL_H_IRQ_FLAG_irq_h_coul_cl_out_flag_START (1)
#define REG_COUL_H_IRQ_FLAG_irq_h_coul_cl_out_flag_END (1)
#define REG_COUL_H_IRQ_FLAG_irq_h_coul_cl_in_flag_START (2)
#define REG_COUL_H_IRQ_FLAG_irq_h_coul_cl_in_flag_END (2)
#define REG_COUL_H_IRQ_FLAG_irq_h_coul_v_gate_flag_START (3)
#define REG_COUL_H_IRQ_FLAG_irq_h_coul_v_gate_flag_END (3)
#define REG_COUL_H_IRQ_FLAG_irq_h_coul_i_in_flag_START (4)
#define REG_COUL_H_IRQ_FLAG_irq_h_coul_i_in_flag_END (4)
#define REG_COUL_H_IRQ_FLAG_irq_h_coul_i_out_flag_START (5)
#define REG_COUL_H_IRQ_FLAG_irq_h_coul_i_out_flag_END (5)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char irq_l_coul_cl_gate_flag : 1;
        unsigned char irq_l_coul_cl_out_flag : 1;
        unsigned char irq_l_coul_cl_in_flag : 1;
        unsigned char irq_l_coul_v_gate_flag : 1;
        unsigned char irq_l_coul_i_in_flag : 1;
        unsigned char irq_l_coul_i_out_flag : 1;
        unsigned char reserved : 2;
    } reg;
} REG_COUL_L_IRQ_FLAG_UNION;
#endif
#define REG_COUL_L_IRQ_FLAG_irq_l_coul_cl_gate_flag_START (0)
#define REG_COUL_L_IRQ_FLAG_irq_l_coul_cl_gate_flag_END (0)
#define REG_COUL_L_IRQ_FLAG_irq_l_coul_cl_out_flag_START (1)
#define REG_COUL_L_IRQ_FLAG_irq_l_coul_cl_out_flag_END (1)
#define REG_COUL_L_IRQ_FLAG_irq_l_coul_cl_in_flag_START (2)
#define REG_COUL_L_IRQ_FLAG_irq_l_coul_cl_in_flag_END (2)
#define REG_COUL_L_IRQ_FLAG_irq_l_coul_v_gate_flag_START (3)
#define REG_COUL_L_IRQ_FLAG_irq_l_coul_v_gate_flag_END (3)
#define REG_COUL_L_IRQ_FLAG_irq_l_coul_i_in_flag_START (4)
#define REG_COUL_L_IRQ_FLAG_irq_l_coul_i_in_flag_END (4)
#define REG_COUL_L_IRQ_FLAG_irq_l_coul_i_out_flag_START (5)
#define REG_COUL_L_IRQ_FLAG_irq_l_coul_i_out_flag_END (5)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char sc_irq_l_coul_mask : 1;
        unsigned char sc_irq_h_coul_mask : 1;
        unsigned char reserved : 6;
    } reg;
} REG_COUL_IRQ_MASK_UNION;
#endif
#define REG_COUL_IRQ_MASK_sc_irq_l_coul_mask_START (0)
#define REG_COUL_IRQ_MASK_sc_irq_l_coul_mask_END (0)
#define REG_COUL_IRQ_MASK_sc_irq_h_coul_mask_START (1)
#define REG_COUL_IRQ_MASK_sc_irq_h_coul_mask_END (1)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char sc_irq_h_coul_cl_gate_mask : 1;
        unsigned char sc_irq_h_coul_cl_out_mask : 1;
        unsigned char sc_irq_h_coul_cl_in_mask : 1;
        unsigned char sc_irq_h_coul_v_gate_mask : 1;
        unsigned char sc_irq_h_coul_i_in_mask : 1;
        unsigned char sc_irq_h_coul_i_out_mask : 1;
        unsigned char reserved : 2;
    } reg;
} REG_COUL_H_IRQ_MASK_UNION;
#endif
#define REG_COUL_H_IRQ_MASK_sc_irq_h_coul_cl_gate_mask_START (0)
#define REG_COUL_H_IRQ_MASK_sc_irq_h_coul_cl_gate_mask_END (0)
#define REG_COUL_H_IRQ_MASK_sc_irq_h_coul_cl_out_mask_START (1)
#define REG_COUL_H_IRQ_MASK_sc_irq_h_coul_cl_out_mask_END (1)
#define REG_COUL_H_IRQ_MASK_sc_irq_h_coul_cl_in_mask_START (2)
#define REG_COUL_H_IRQ_MASK_sc_irq_h_coul_cl_in_mask_END (2)
#define REG_COUL_H_IRQ_MASK_sc_irq_h_coul_v_gate_mask_START (3)
#define REG_COUL_H_IRQ_MASK_sc_irq_h_coul_v_gate_mask_END (3)
#define REG_COUL_H_IRQ_MASK_sc_irq_h_coul_i_in_mask_START (4)
#define REG_COUL_H_IRQ_MASK_sc_irq_h_coul_i_in_mask_END (4)
#define REG_COUL_H_IRQ_MASK_sc_irq_h_coul_i_out_mask_START (5)
#define REG_COUL_H_IRQ_MASK_sc_irq_h_coul_i_out_mask_END (5)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char sc_irq_l_coul_cl_gate_mask : 1;
        unsigned char sc_irq_l_coul_cl_out_mask : 1;
        unsigned char sc_irq_l_coul_cl_in_mask : 1;
        unsigned char sc_irq_l_coul_v_gate_mask : 1;
        unsigned char sc_irq_l_coul_i_in_mask : 1;
        unsigned char sc_irq_l_coul_i_out_mask : 1;
        unsigned char reserved : 2;
    } reg;
} REG_COUL_L_IRQ_MASK_UNION;
#endif
#define REG_COUL_L_IRQ_MASK_sc_irq_l_coul_cl_gate_mask_START (0)
#define REG_COUL_L_IRQ_MASK_sc_irq_l_coul_cl_gate_mask_END (0)
#define REG_COUL_L_IRQ_MASK_sc_irq_l_coul_cl_out_mask_START (1)
#define REG_COUL_L_IRQ_MASK_sc_irq_l_coul_cl_out_mask_END (1)
#define REG_COUL_L_IRQ_MASK_sc_irq_l_coul_cl_in_mask_START (2)
#define REG_COUL_L_IRQ_MASK_sc_irq_l_coul_cl_in_mask_END (2)
#define REG_COUL_L_IRQ_MASK_sc_irq_l_coul_v_gate_mask_START (3)
#define REG_COUL_L_IRQ_MASK_sc_irq_l_coul_v_gate_mask_END (3)
#define REG_COUL_L_IRQ_MASK_sc_irq_l_coul_i_in_mask_START (4)
#define REG_COUL_L_IRQ_MASK_sc_irq_l_coul_i_in_mask_END (4)
#define REG_COUL_L_IRQ_MASK_sc_irq_l_coul_i_out_mask_START (5)
#define REG_COUL_L_IRQ_MASK_sc_irq_l_coul_i_out_mask_END (5)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char irq_h_coul_cl_gate_status : 1;
        unsigned char irq_h_coul_cl_out_status : 1;
        unsigned char irq_h_coul_cl_in_status : 1;
        unsigned char irq_h_coul_v_gate_status : 1;
        unsigned char irq_h_coul_i_in_status : 1;
        unsigned char irq_h_coul_i_out_status : 1;
        unsigned char reserved : 2;
    } reg;
} REG_COUL_H_IRQ_STATUS_UNION;
#endif
#define REG_COUL_H_IRQ_STATUS_irq_h_coul_cl_gate_status_START (0)
#define REG_COUL_H_IRQ_STATUS_irq_h_coul_cl_gate_status_END (0)
#define REG_COUL_H_IRQ_STATUS_irq_h_coul_cl_out_status_START (1)
#define REG_COUL_H_IRQ_STATUS_irq_h_coul_cl_out_status_END (1)
#define REG_COUL_H_IRQ_STATUS_irq_h_coul_cl_in_status_START (2)
#define REG_COUL_H_IRQ_STATUS_irq_h_coul_cl_in_status_END (2)
#define REG_COUL_H_IRQ_STATUS_irq_h_coul_v_gate_status_START (3)
#define REG_COUL_H_IRQ_STATUS_irq_h_coul_v_gate_status_END (3)
#define REG_COUL_H_IRQ_STATUS_irq_h_coul_i_in_status_START (4)
#define REG_COUL_H_IRQ_STATUS_irq_h_coul_i_in_status_END (4)
#define REG_COUL_H_IRQ_STATUS_irq_h_coul_i_out_status_START (5)
#define REG_COUL_H_IRQ_STATUS_irq_h_coul_i_out_status_END (5)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char irq_l_coul_cl_gate_status : 1;
        unsigned char irq_l_coul_cl_out_status : 1;
        unsigned char irq_l_coul_cl_in_status : 1;
        unsigned char irq_l_coul_v_gate_status : 1;
        unsigned char irq_l_coul_i_in_status : 1;
        unsigned char irq_l_coul_i_out_status : 1;
        unsigned char reserved : 2;
    } reg;
} REG_COUL_L_IRQ_STATUS_UNION;
#endif
#define REG_COUL_L_IRQ_STATUS_irq_l_coul_cl_gate_status_START (0)
#define REG_COUL_L_IRQ_STATUS_irq_l_coul_cl_gate_status_END (0)
#define REG_COUL_L_IRQ_STATUS_irq_l_coul_cl_out_status_START (1)
#define REG_COUL_L_IRQ_STATUS_irq_l_coul_cl_out_status_END (1)
#define REG_COUL_L_IRQ_STATUS_irq_l_coul_cl_in_status_START (2)
#define REG_COUL_L_IRQ_STATUS_irq_l_coul_cl_in_status_END (2)
#define REG_COUL_L_IRQ_STATUS_irq_l_coul_v_gate_status_START (3)
#define REG_COUL_L_IRQ_STATUS_irq_l_coul_v_gate_status_END (3)
#define REG_COUL_L_IRQ_STATUS_irq_l_coul_i_in_status_START (4)
#define REG_COUL_L_IRQ_STATUS_irq_l_coul_i_in_status_END (4)
#define REG_COUL_L_IRQ_STATUS_irq_l_coul_i_out_status_START (5)
#define REG_COUL_L_IRQ_STATUS_irq_l_coul_i_out_status_END (5)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char reflash_value_ctrl : 1;
        unsigned char eco_filter_time : 2;
        unsigned char reserved_0 : 1;
        unsigned char calibration_ctrl : 1;
        unsigned char reserved_1 : 3;
    } reg;
} REG_CLJ_CTRL_REG1_UNION;
#endif
#define REG_CLJ_CTRL_REG1_reflash_value_ctrl_START (0)
#define REG_CLJ_CTRL_REG1_reflash_value_ctrl_END (0)
#define REG_CLJ_CTRL_REG1_eco_filter_time_START (1)
#define REG_CLJ_CTRL_REG1_eco_filter_time_END (2)
#define REG_CLJ_CTRL_REG1_calibration_ctrl_START (4)
#define REG_CLJ_CTRL_REG1_calibration_ctrl_END (4)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char pd_ocv_i_onoff : 1;
        unsigned char reg_data_clr : 1;
        unsigned char cali_auto_time : 2;
        unsigned char cali_auto_onoff_ctrl : 1;
        unsigned char reserved : 3;
    } reg;
} REG_CLJ_CTRL_REGS2_UNION;
#endif
#define REG_CLJ_CTRL_REGS2_pd_ocv_i_onoff_START (0)
#define REG_CLJ_CTRL_REGS2_pd_ocv_i_onoff_END (0)
#define REG_CLJ_CTRL_REGS2_reg_data_clr_START (1)
#define REG_CLJ_CTRL_REGS2_reg_data_clr_END (1)
#define REG_CLJ_CTRL_REGS2_cali_auto_time_START (2)
#define REG_CLJ_CTRL_REGS2_cali_auto_time_END (3)
#define REG_CLJ_CTRL_REGS2_cali_auto_onoff_ctrl_START (4)
#define REG_CLJ_CTRL_REGS2_cali_auto_onoff_ctrl_END (4)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char reserved_0 : 1;
        unsigned char coul_average_sel : 1;
        unsigned char reserved_1 : 1;
        unsigned char debug_refresh_en : 1;
        unsigned char coul_i_data_rate_sel : 1;
        unsigned char coul_v_data_rate_sel : 1;
        unsigned char reserved_2 : 2;
    } reg;
} REG_CIC_CTRL_UNION;
#endif
#define REG_CIC_CTRL_coul_average_sel_START (1)
#define REG_CIC_CTRL_coul_average_sel_END (1)
#define REG_CIC_CTRL_debug_refresh_en_START (3)
#define REG_CIC_CTRL_debug_refresh_en_END (3)
#define REG_CIC_CTRL_coul_i_data_rate_sel_START (4)
#define REG_CIC_CTRL_coul_i_data_rate_sel_END (4)
#define REG_CIC_CTRL_coul_v_data_rate_sel_START (5)
#define REG_CIC_CTRL_coul_v_data_rate_sel_END (5)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char coul_mstate : 4;
        unsigned char reserved : 4;
    } reg;
} REG_STATE_TEST_UNION;
#endif
#define REG_STATE_TEST_coul_mstate_START (0)
#define REG_STATE_TEST_coul_mstate_END (3)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char il_cic_clk_sel : 1;
        unsigned char ih_cic_clk_sel : 1;
        unsigned char v_cic_clk_sel : 1;
        unsigned char reserved : 1;
        unsigned char cali_en_i : 1;
        unsigned char cali_en_i_force : 1;
        unsigned char cali_en_v_force : 1;
        unsigned char cali_en_v : 1;
    } reg;
} REG_CLJ_DEBUG0_UNION;
#endif
#define REG_CLJ_DEBUG0_il_cic_clk_sel_START (0)
#define REG_CLJ_DEBUG0_il_cic_clk_sel_END (0)
#define REG_CLJ_DEBUG0_ih_cic_clk_sel_START (1)
#define REG_CLJ_DEBUG0_ih_cic_clk_sel_END (1)
#define REG_CLJ_DEBUG0_v_cic_clk_sel_START (2)
#define REG_CLJ_DEBUG0_v_cic_clk_sel_END (2)
#define REG_CLJ_DEBUG0_cali_en_i_START (4)
#define REG_CLJ_DEBUG0_cali_en_i_END (4)
#define REG_CLJ_DEBUG0_cali_en_i_force_START (5)
#define REG_CLJ_DEBUG0_cali_en_i_force_END (5)
#define REG_CLJ_DEBUG0_cali_en_v_force_START (6)
#define REG_CLJ_DEBUG0_cali_en_v_force_END (6)
#define REG_CLJ_DEBUG0_cali_en_v_START (7)
#define REG_CLJ_DEBUG0_cali_en_v_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char current_coul_always_off : 1;
        unsigned char voltage_coul_always_off : 1;
        unsigned char coul_ctrl_onoff : 1;
        unsigned char coul_ocv_onoff : 1;
        unsigned char reserved : 4;
    } reg;
} REG_CLJ_DEBUG1_UNION;
#endif
#define REG_CLJ_DEBUG1_current_coul_always_off_START (0)
#define REG_CLJ_DEBUG1_current_coul_always_off_END (0)
#define REG_CLJ_DEBUG1_voltage_coul_always_off_START (1)
#define REG_CLJ_DEBUG1_voltage_coul_always_off_END (1)
#define REG_CLJ_DEBUG1_coul_ctrl_onoff_START (2)
#define REG_CLJ_DEBUG1_coul_ctrl_onoff_END (2)
#define REG_CLJ_DEBUG1_coul_ocv_onoff_START (3)
#define REG_CLJ_DEBUG1_coul_ocv_onoff_END (3)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char debug_write_pro : 8;
    } reg;
} REG_DEBUG_WRITE_PRO_UNION;
#endif
#define REG_DEBUG_WRITE_PRO_debug_write_pro_START (0)
#define REG_DEBUG_WRITE_PRO_debug_write_pro_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char sc_coul_clk_cali_en : 1;
        unsigned char sc_coul_clk_cali_disable : 1;
        unsigned char reserved : 6;
    } reg;
} REG_COUL_CLK_CALI_EN_UNION;
#endif
#define REG_COUL_CLK_CALI_EN_sc_coul_clk_cali_en_START (0)
#define REG_COUL_CLK_CALI_EN_sc_coul_clk_cali_en_END (0)
#define REG_COUL_CLK_CALI_EN_sc_coul_clk_cali_disable_START (1)
#define REG_COUL_CLK_CALI_EN_sc_coul_clk_cali_disable_END (1)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char coul_debug_data_sel : 3;
        unsigned char reserved : 5;
    } reg;
} REG_DEBUG_CIC_DATA_SEL_UNION;
#endif
#define REG_DEBUG_CIC_DATA_SEL_coul_debug_data_sel_START (0)
#define REG_DEBUG_CIC_DATA_SEL_coul_debug_data_sel_END (2)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char sc_coul_tb_sel : 6;
        unsigned char sc_coul_tb_en : 1;
        unsigned char coul_tb_sig : 1;
    } reg;
} REG_COUL_TB_CFG_UNION;
#endif
#define REG_COUL_TB_CFG_sc_coul_tb_sel_START (0)
#define REG_COUL_TB_CFG_sc_coul_tb_sel_END (5)
#define REG_COUL_TB_CFG_sc_coul_tb_en_START (6)
#define REG_COUL_TB_CFG_sc_coul_tb_en_END (6)
#define REG_COUL_TB_CFG_coul_tb_sig_START (7)
#define REG_COUL_TB_CFG_coul_tb_sig_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char coul_i_tb_bus0 : 8;
    } reg;
} REG_COUL_I_TB_BUS0_UNION;
#endif
#define REG_COUL_I_TB_BUS0_coul_i_tb_bus0_START (0)
#define REG_COUL_I_TB_BUS0_coul_i_tb_bus0_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char coul_i_tb_bus1 : 8;
    } reg;
} REG_COUL_I_TB_BUS1_UNION;
#endif
#define REG_COUL_I_TB_BUS1_coul_i_tb_bus1_START (0)
#define REG_COUL_I_TB_BUS1_coul_i_tb_bus1_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char coul_i_tb_bus2 : 8;
    } reg;
} REG_COUL_I_TB_BUS2_UNION;
#endif
#define REG_COUL_I_TB_BUS2_coul_i_tb_bus2_START (0)
#define REG_COUL_I_TB_BUS2_coul_i_tb_bus2_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char coul_v_tb_bus0 : 8;
    } reg;
} REG_COUL_V_TB_BUS0_UNION;
#endif
#define REG_COUL_V_TB_BUS0_coul_v_tb_bus0_START (0)
#define REG_COUL_V_TB_BUS0_coul_v_tb_bus0_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char coul_v_tb_bus1 : 8;
    } reg;
} REG_COUL_V_TB_BUS1_UNION;
#endif
#define REG_COUL_V_TB_BUS1_coul_v_tb_bus1_START (0)
#define REG_COUL_V_TB_BUS1_coul_v_tb_bus1_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char coul_v_tb_bus2 : 8;
    } reg;
} REG_COUL_V_TB_BUS2_UNION;
#endif
#define REG_COUL_V_TB_BUS2_coul_v_tb_bus2_START (0)
#define REG_COUL_V_TB_BUS2_coul_v_tb_bus2_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char v_int_s0 : 8;
    } reg;
} REG_V_INT0_UNION;
#endif
#define REG_V_INT0_v_int_s0_START (0)
#define REG_V_INT0_v_int_s0_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char v_int_s1 : 8;
    } reg;
} REG_V_INT1_UNION;
#endif
#define REG_V_INT1_v_int_s1_START (0)
#define REG_V_INT1_v_int_s1_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char v_int_s2 : 8;
    } reg;
} REG_V_INT2_UNION;
#endif
#define REG_V_INT2_v_int_s2_START (0)
#define REG_V_INT2_v_int_s2_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char i_out_gate_s0 : 8;
    } reg;
} REG_I_OUT_GATE0_UNION;
#endif
#define REG_I_OUT_GATE0_i_out_gate_s0_START (0)
#define REG_I_OUT_GATE0_i_out_gate_s0_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char i_out_gate_s1 : 8;
    } reg;
} REG_I_OUT_GATE1_UNION;
#endif
#define REG_I_OUT_GATE1_i_out_gate_s1_START (0)
#define REG_I_OUT_GATE1_i_out_gate_s1_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char i_out_gate_s2 : 8;
    } reg;
} REG_I_OUT_GATE2_UNION;
#endif
#define REG_I_OUT_GATE2_i_out_gate_s2_START (0)
#define REG_I_OUT_GATE2_i_out_gate_s2_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char i_in_gate_s0 : 8;
    } reg;
} REG_I_IN_GATE0_UNION;
#endif
#define REG_I_IN_GATE0_i_in_gate_s0_START (0)
#define REG_I_IN_GATE0_i_in_gate_s0_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char i_in_gate_s1 : 8;
    } reg;
} REG_I_IN_GATE1_UNION;
#endif
#define REG_I_IN_GATE1_i_in_gate_s1_START (0)
#define REG_I_IN_GATE1_i_in_gate_s1_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char i_in_gate_s2 : 8;
    } reg;
} REG_I_IN_GATE2_UNION;
#endif
#define REG_I_IN_GATE2_i_in_gate_s2_START (0)
#define REG_I_IN_GATE2_i_in_gate_s2_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char cl_int_s0 : 8;
    } reg;
} REG_CL_INT0_UNION;
#endif
#define REG_CL_INT0_cl_int_s0_START (0)
#define REG_CL_INT0_cl_int_s0_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char cl_int_s1 : 8;
    } reg;
} REG_CL_INT1_UNION;
#endif
#define REG_CL_INT1_cl_int_s1_START (0)
#define REG_CL_INT1_cl_int_s1_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char cl_int_s2 : 8;
    } reg;
} REG_CL_INT2_UNION;
#endif
#define REG_CL_INT2_cl_int_s2_START (0)
#define REG_CL_INT2_cl_int_s2_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char cl_int_s3 : 8;
    } reg;
} REG_CL_INT3_UNION;
#endif
#define REG_CL_INT3_cl_int_s3_START (0)
#define REG_CL_INT3_cl_int_s3_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char cl_int_s4 : 8;
    } reg;
} REG_CL_INT4_UNION;
#endif
#define REG_CL_INT4_cl_int_s4_START (0)
#define REG_CL_INT4_cl_int_s4_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char rtc_ocv_out_pre4 : 2;
        unsigned char rtc_ocv_out_pre3 : 2;
        unsigned char rtc_ocv_out_pre2 : 2;
        unsigned char rtc_ocv_out_pre1 : 2;
    } reg;
} REG_RTC_OCV_OUT_UNION;
#endif
#define REG_RTC_OCV_OUT_rtc_ocv_out_pre4_START (0)
#define REG_RTC_OCV_OUT_rtc_ocv_out_pre4_END (1)
#define REG_RTC_OCV_OUT_rtc_ocv_out_pre3_START (2)
#define REG_RTC_OCV_OUT_rtc_ocv_out_pre3_END (3)
#define REG_RTC_OCV_OUT_rtc_ocv_out_pre2_START (4)
#define REG_RTC_OCV_OUT_rtc_ocv_out_pre2_END (5)
#define REG_RTC_OCV_OUT_rtc_ocv_out_pre1_START (6)
#define REG_RTC_OCV_OUT_rtc_ocv_out_pre1_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char rd_sync_pro : 8;
    } reg;
} REG_RD_SYNC_PRO_UNION;
#endif
#define REG_RD_SYNC_PRO_rd_sync_pro_START (0)
#define REG_RD_SYNC_PRO_rd_sync_pro_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char rd_sync_sel : 1;
        unsigned char reserved : 7;
    } reg;
} REG_RD_SYNC_SEL_UNION;
#endif
#define REG_RD_SYNC_SEL_rd_sync_sel_START (0)
#define REG_RD_SYNC_SEL_rd_sync_sel_END (0)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char il_cic_en_sel : 1;
        unsigned char ih_cic_en_sel : 1;
        unsigned char v_cic_en_sel : 1;
        unsigned char reserved_0 : 1;
        unsigned char sc_coul_cic_il_en : 1;
        unsigned char sc_coul_cic_ih_en : 1;
        unsigned char sc_coul_cic_v_en : 1;
        unsigned char reserved_1 : 1;
    } reg;
} REG_CIC_EN_DEBUG_UNION;
#endif
#define REG_CIC_EN_DEBUG_il_cic_en_sel_START (0)
#define REG_CIC_EN_DEBUG_il_cic_en_sel_END (0)
#define REG_CIC_EN_DEBUG_ih_cic_en_sel_START (1)
#define REG_CIC_EN_DEBUG_ih_cic_en_sel_END (1)
#define REG_CIC_EN_DEBUG_v_cic_en_sel_START (2)
#define REG_CIC_EN_DEBUG_v_cic_en_sel_END (2)
#define REG_CIC_EN_DEBUG_sc_coul_cic_il_en_START (4)
#define REG_CIC_EN_DEBUG_sc_coul_cic_il_en_END (4)
#define REG_CIC_EN_DEBUG_sc_coul_cic_ih_en_START (5)
#define REG_CIC_EN_DEBUG_sc_coul_cic_ih_en_END (5)
#define REG_CIC_EN_DEBUG_sc_coul_cic_v_en_START (6)
#define REG_CIC_EN_DEBUG_sc_coul_cic_v_en_END (6)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char sc_nice_tb_sel : 1;
        unsigned char sc_regn_ok_sel : 1;
        unsigned char reserved : 6;
    } reg;
} REG_SC_NICE_TB_SEL_UNION;
#endif
#define REG_SC_NICE_TB_SEL_sc_nice_tb_sel_START (0)
#define REG_SC_NICE_TB_SEL_sc_nice_tb_sel_END (0)
#define REG_SC_NICE_TB_SEL_sc_regn_ok_sel_START (1)
#define REG_SC_NICE_TB_SEL_sc_regn_ok_sel_END (1)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char sc_nice_tb_trig : 1;
        unsigned char sc_regn_ok_trig : 1;
        unsigned char reserved : 6;
    } reg;
} REG_SC_NICE_TB_TRIG_UNION;
#endif
#define REG_SC_NICE_TB_TRIG_sc_nice_tb_trig_START (0)
#define REG_SC_NICE_TB_TRIG_sc_nice_tb_trig_END (0)
#define REG_SC_NICE_TB_TRIG_sc_regn_ok_trig_START (1)
#define REG_SC_NICE_TB_TRIG_sc_regn_ok_trig_END (1)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char sc_eco_nice_vld : 1;
        unsigned char reserved : 7;
    } reg;
} REG_ECO_PWROFF_VLD_UNION;
#endif
#define REG_ECO_PWROFF_VLD_sc_eco_nice_vld_START (0)
#define REG_ECO_PWROFF_VLD_sc_eco_nice_vld_END (0)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char clj_dbg_cfg : 8;
    } reg;
} REG_CLJ_DBG_CFG_UNION;
#endif
#define REG_CLJ_DBG_CFG_clj_dbg_cfg_START (0)
#define REG_CLJ_DBG_CFG_clj_dbg_cfg_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char coul_crg_cali_value : 6;
        unsigned char reserved : 2;
    } reg;
} REG_CLJ_DBG_STATUS_UNION;
#endif
#define REG_CLJ_DBG_STATUS_coul_crg_cali_value_START (0)
#define REG_CLJ_DBG_STATUS_coul_crg_cali_value_END (5)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char h_cl_out_ro0 : 8;
    } reg;
} REG_H_CL_OUT_RO0_UNION;
#endif
#define REG_H_CL_OUT_RO0_h_cl_out_ro0_START (0)
#define REG_H_CL_OUT_RO0_h_cl_out_ro0_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char h_cl_out_ro1 : 8;
    } reg;
} REG_H_CL_OUT_RO1_UNION;
#endif
#define REG_H_CL_OUT_RO1_h_cl_out_ro1_START (0)
#define REG_H_CL_OUT_RO1_h_cl_out_ro1_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char h_cl_out_ro2 : 8;
    } reg;
} REG_H_CL_OUT_RO2_UNION;
#endif
#define REG_H_CL_OUT_RO2_h_cl_out_ro2_START (0)
#define REG_H_CL_OUT_RO2_h_cl_out_ro2_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char h_cl_out_ro3 : 8;
    } reg;
} REG_H_CL_OUT_RO3_UNION;
#endif
#define REG_H_CL_OUT_RO3_h_cl_out_ro3_START (0)
#define REG_H_CL_OUT_RO3_h_cl_out_ro3_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char h_cl_out_ro4 : 8;
    } reg;
} REG_H_CL_OUT_RO4_UNION;
#endif
#define REG_H_CL_OUT_RO4_h_cl_out_ro4_START (0)
#define REG_H_CL_OUT_RO4_h_cl_out_ro4_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char h_cl_in_ro0 : 8;
    } reg;
} REG_H_CL_IN_RO0_UNION;
#endif
#define REG_H_CL_IN_RO0_h_cl_in_ro0_START (0)
#define REG_H_CL_IN_RO0_h_cl_in_ro0_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char h_cl_in_ro1 : 8;
    } reg;
} REG_H_CL_IN_RO1_UNION;
#endif
#define REG_H_CL_IN_RO1_h_cl_in_ro1_START (0)
#define REG_H_CL_IN_RO1_h_cl_in_ro1_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char h_cl_in_ro2 : 8;
    } reg;
} REG_H_CL_IN_RO2_UNION;
#endif
#define REG_H_CL_IN_RO2_h_cl_in_ro2_START (0)
#define REG_H_CL_IN_RO2_h_cl_in_ro2_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char h_cl_in_ro3 : 8;
    } reg;
} REG_H_CL_IN_RO3_UNION;
#endif
#define REG_H_CL_IN_RO3_h_cl_in_ro3_START (0)
#define REG_H_CL_IN_RO3_h_cl_in_ro3_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char h_cl_in_ro4 : 8;
    } reg;
} REG_H_CL_IN_RO4_UNION;
#endif
#define REG_H_CL_IN_RO4_h_cl_in_ro4_START (0)
#define REG_H_CL_IN_RO4_h_cl_in_ro4_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char h_chg_timer_ro0 : 8;
    } reg;
} REG_H_CHG_TIMER_RO0_UNION;
#endif
#define REG_H_CHG_TIMER_RO0_h_chg_timer_ro0_START (0)
#define REG_H_CHG_TIMER_RO0_h_chg_timer_ro0_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char h_chg_timer_ro1 : 8;
    } reg;
} REG_H_CHG_TIMER_RO1_UNION;
#endif
#define REG_H_CHG_TIMER_RO1_h_chg_timer_ro1_START (0)
#define REG_H_CHG_TIMER_RO1_h_chg_timer_ro1_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char h_chg_timer_ro2 : 8;
    } reg;
} REG_H_CHG_TIMER_RO2_UNION;
#endif
#define REG_H_CHG_TIMER_RO2_h_chg_timer_ro2_START (0)
#define REG_H_CHG_TIMER_RO2_h_chg_timer_ro2_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char h_chg_timer_ro3 : 8;
    } reg;
} REG_H_CHG_TIMER_RO3_UNION;
#endif
#define REG_H_CHG_TIMER_RO3_h_chg_timer_ro3_START (0)
#define REG_H_CHG_TIMER_RO3_h_chg_timer_ro3_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char h_load_timer_ro0 : 8;
    } reg;
} REG_H_LOAD_TIMER_RO0_UNION;
#endif
#define REG_H_LOAD_TIMER_RO0_h_load_timer_ro0_START (0)
#define REG_H_LOAD_TIMER_RO0_h_load_timer_ro0_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char h_load_timer_ro1 : 8;
    } reg;
} REG_H_LOAD_TIMER_RO1_UNION;
#endif
#define REG_H_LOAD_TIMER_RO1_h_load_timer_ro1_START (0)
#define REG_H_LOAD_TIMER_RO1_h_load_timer_ro1_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char h_load_timer_ro2 : 8;
    } reg;
} REG_H_LOAD_TIMER_RO2_UNION;
#endif
#define REG_H_LOAD_TIMER_RO2_h_load_timer_ro2_START (0)
#define REG_H_LOAD_TIMER_RO2_h_load_timer_ro2_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char h_load_timer_ro3 : 8;
    } reg;
} REG_H_LOAD_TIMER_RO3_UNION;
#endif
#define REG_H_LOAD_TIMER_RO3_h_load_timer_ro3_START (0)
#define REG_H_LOAD_TIMER_RO3_h_load_timer_ro3_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char l_cl_out_ro0 : 8;
    } reg;
} REG_L_CL_OUT_RO0_UNION;
#endif
#define REG_L_CL_OUT_RO0_l_cl_out_ro0_START (0)
#define REG_L_CL_OUT_RO0_l_cl_out_ro0_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char l_cl_out_ro1 : 8;
    } reg;
} REG_L_CL_OUT_RO1_UNION;
#endif
#define REG_L_CL_OUT_RO1_l_cl_out_ro1_START (0)
#define REG_L_CL_OUT_RO1_l_cl_out_ro1_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char l_cl_out_ro2 : 8;
    } reg;
} REG_L_CL_OUT_RO2_UNION;
#endif
#define REG_L_CL_OUT_RO2_l_cl_out_ro2_START (0)
#define REG_L_CL_OUT_RO2_l_cl_out_ro2_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char l_cl_out_ro3 : 8;
    } reg;
} REG_L_CL_OUT_RO3_UNION;
#endif
#define REG_L_CL_OUT_RO3_l_cl_out_ro3_START (0)
#define REG_L_CL_OUT_RO3_l_cl_out_ro3_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char l_cl_out_ro4 : 8;
    } reg;
} REG_L_CL_OUT_RO4_UNION;
#endif
#define REG_L_CL_OUT_RO4_l_cl_out_ro4_START (0)
#define REG_L_CL_OUT_RO4_l_cl_out_ro4_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char l_cl_in_ro0 : 8;
    } reg;
} REG_L_CL_IN_RO0_UNION;
#endif
#define REG_L_CL_IN_RO0_l_cl_in_ro0_START (0)
#define REG_L_CL_IN_RO0_l_cl_in_ro0_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char l_cl_in_ro1 : 8;
    } reg;
} REG_L_CL_IN_RO1_UNION;
#endif
#define REG_L_CL_IN_RO1_l_cl_in_ro1_START (0)
#define REG_L_CL_IN_RO1_l_cl_in_ro1_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char l_cl_in_ro2 : 8;
    } reg;
} REG_L_CL_IN_RO2_UNION;
#endif
#define REG_L_CL_IN_RO2_l_cl_in_ro2_START (0)
#define REG_L_CL_IN_RO2_l_cl_in_ro2_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char l_cl_in_ro3 : 8;
    } reg;
} REG_L_CL_IN_RO3_UNION;
#endif
#define REG_L_CL_IN_RO3_l_cl_in_ro3_START (0)
#define REG_L_CL_IN_RO3_l_cl_in_ro3_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char l_cl_in_ro4 : 8;
    } reg;
} REG_L_CL_IN_RO4_UNION;
#endif
#define REG_L_CL_IN_RO4_l_cl_in_ro4_START (0)
#define REG_L_CL_IN_RO4_l_cl_in_ro4_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char l_chg_timer_ro0 : 8;
    } reg;
} REG_L_CHG_TIMER_RO0_UNION;
#endif
#define REG_L_CHG_TIMER_RO0_l_chg_timer_ro0_START (0)
#define REG_L_CHG_TIMER_RO0_l_chg_timer_ro0_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char l_chg_timer_ro1 : 8;
    } reg;
} REG_L_CHG_TIMER_RO1_UNION;
#endif
#define REG_L_CHG_TIMER_RO1_l_chg_timer_ro1_START (0)
#define REG_L_CHG_TIMER_RO1_l_chg_timer_ro1_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char l_chg_timer_ro2 : 8;
    } reg;
} REG_L_CHG_TIMER_RO2_UNION;
#endif
#define REG_L_CHG_TIMER_RO2_l_chg_timer_ro2_START (0)
#define REG_L_CHG_TIMER_RO2_l_chg_timer_ro2_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char l_chg_timer_ro3 : 8;
    } reg;
} REG_L_CHG_TIMER_RO3_UNION;
#endif
#define REG_L_CHG_TIMER_RO3_l_chg_timer_ro3_START (0)
#define REG_L_CHG_TIMER_RO3_l_chg_timer_ro3_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char l_load_timer_ro0 : 8;
    } reg;
} REG_L_LOAD_TIMER_RO0_UNION;
#endif
#define REG_L_LOAD_TIMER_RO0_l_load_timer_ro0_START (0)
#define REG_L_LOAD_TIMER_RO0_l_load_timer_ro0_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char l_load_timer_ro1 : 8;
    } reg;
} REG_L_LOAD_TIMER_RO1_UNION;
#endif
#define REG_L_LOAD_TIMER_RO1_l_load_timer_ro1_START (0)
#define REG_L_LOAD_TIMER_RO1_l_load_timer_ro1_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char l_load_timer_ro2 : 8;
    } reg;
} REG_L_LOAD_TIMER_RO2_UNION;
#endif
#define REG_L_LOAD_TIMER_RO2_l_load_timer_ro2_START (0)
#define REG_L_LOAD_TIMER_RO2_l_load_timer_ro2_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char l_load_timer_ro3 : 8;
    } reg;
} REG_L_LOAD_TIMER_RO3_UNION;
#endif
#define REG_L_LOAD_TIMER_RO3_l_load_timer_ro3_START (0)
#define REG_L_LOAD_TIMER_RO3_l_load_timer_ro3_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char eco_cnt_out1_s0 : 8;
    } reg;
} REG_ECO_CNT1_OUT0_UNION;
#endif
#define REG_ECO_CNT1_OUT0_eco_cnt_out1_s0_START (0)
#define REG_ECO_CNT1_OUT0_eco_cnt_out1_s0_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char eco_cnt_out1_s1 : 8;
    } reg;
} REG_ECO_CNT1_OUT1_UNION;
#endif
#define REG_ECO_CNT1_OUT1_eco_cnt_out1_s1_START (0)
#define REG_ECO_CNT1_OUT1_eco_cnt_out1_s1_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char eco_cnt_out2_s0 : 8;
    } reg;
} REG_ECO_CNT2_OUT0_UNION;
#endif
#define REG_ECO_CNT2_OUT0_eco_cnt_out2_s0_START (0)
#define REG_ECO_CNT2_OUT0_eco_cnt_out2_s0_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char eco_cnt_out2_s1 : 8;
    } reg;
} REG_ECO_CNT2_OUT1_UNION;
#endif
#define REG_ECO_CNT2_OUT1_eco_cnt_out2_s1_START (0)
#define REG_ECO_CNT2_OUT1_eco_cnt_out2_s1_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char eco_cnt_out3_s0 : 8;
    } reg;
} REG_ECO_CNT3_OUT0_UNION;
#endif
#define REG_ECO_CNT3_OUT0_eco_cnt_out3_s0_START (0)
#define REG_ECO_CNT3_OUT0_eco_cnt_out3_s0_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char eco_cnt_out3_s1 : 8;
    } reg;
} REG_ECO_CNT3_OUT1_UNION;
#endif
#define REG_ECO_CNT3_OUT1_eco_cnt_out3_s1_START (0)
#define REG_ECO_CNT3_OUT1_eco_cnt_out3_s1_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char eco_cnt_out4_s0 : 8;
    } reg;
} REG_ECO_CNT4_OUT0_UNION;
#endif
#define REG_ECO_CNT4_OUT0_eco_cnt_out4_s0_START (0)
#define REG_ECO_CNT4_OUT0_eco_cnt_out4_s0_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char eco_cnt_out4_s1 : 8;
    } reg;
} REG_ECO_CNT4_OUT1_UNION;
#endif
#define REG_ECO_CNT4_OUT1_eco_cnt_out4_s1_START (0)
#define REG_ECO_CNT4_OUT1_eco_cnt_out4_s1_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char da_coul_adc_ibias_sel : 6;
        unsigned char reserved : 2;
    } reg;
} REG_COUL_TOP_CFG_REG_0_UNION;
#endif
#define REG_COUL_TOP_CFG_REG_0_da_coul_adc_ibias_sel_START (0)
#define REG_COUL_TOP_CFG_REG_0_da_coul_adc_ibias_sel_END (5)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char da_coul_dt_ctrl : 2;
        unsigned char da_coul_chop_clk_sel : 4;
        unsigned char da_coul_i_pga_gain_adj : 2;
    } reg;
} REG_COUL_TOP_CFG_REG_1_UNION;
#endif
#define REG_COUL_TOP_CFG_REG_1_da_coul_dt_ctrl_START (0)
#define REG_COUL_TOP_CFG_REG_1_da_coul_dt_ctrl_END (1)
#define REG_COUL_TOP_CFG_REG_1_da_coul_chop_clk_sel_START (2)
#define REG_COUL_TOP_CFG_REG_1_da_coul_chop_clk_sel_END (5)
#define REG_COUL_TOP_CFG_REG_1_da_coul_i_pga_gain_adj_START (6)
#define REG_COUL_TOP_CFG_REG_1_da_coul_i_pga_gain_adj_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char da_coul_i_pga_cap_n : 4;
        unsigned char da_coul_i_amp_ibias_sel : 4;
    } reg;
} REG_COUL_TOP_CFG_REG_2_UNION;
#endif
#define REG_COUL_TOP_CFG_REG_2_da_coul_i_pga_cap_n_START (0)
#define REG_COUL_TOP_CFG_REG_2_da_coul_i_pga_cap_n_END (3)
#define REG_COUL_TOP_CFG_REG_2_da_coul_i_amp_ibias_sel_START (4)
#define REG_COUL_TOP_CFG_REG_2_da_coul_i_amp_ibias_sel_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char da_coul_ih_debug_ctrlh : 1;
        unsigned char da_coul_ih_debug_ctrl : 2;
        unsigned char reserved : 1;
        unsigned char da_coul_i_pga_cap_p : 4;
    } reg;
} REG_COUL_TOP_CFG_REG_3_UNION;
#endif
#define REG_COUL_TOP_CFG_REG_3_da_coul_ih_debug_ctrlh_START (0)
#define REG_COUL_TOP_CFG_REG_3_da_coul_ih_debug_ctrlh_END (0)
#define REG_COUL_TOP_CFG_REG_3_da_coul_ih_debug_ctrl_START (1)
#define REG_COUL_TOP_CFG_REG_3_da_coul_ih_debug_ctrl_END (2)
#define REG_COUL_TOP_CFG_REG_3_da_coul_i_pga_cap_p_START (4)
#define REG_COUL_TOP_CFG_REG_3_da_coul_i_pga_cap_p_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char da_coul_il_pga_chop_en : 1;
        unsigned char da_coul_il_int1_chop_en : 1;
        unsigned char da_coul_il_debug_ctrlh : 1;
        unsigned char da_coul_il_debug_ctrl : 2;
        unsigned char da_coul_ih_sys_chop_en : 1;
        unsigned char da_coul_ih_pga_chop_en : 1;
        unsigned char da_coul_ih_int1_chop_en : 1;
    } reg;
} REG_COUL_TOP_CFG_REG_4_UNION;
#endif
#define REG_COUL_TOP_CFG_REG_4_da_coul_il_pga_chop_en_START (0)
#define REG_COUL_TOP_CFG_REG_4_da_coul_il_pga_chop_en_END (0)
#define REG_COUL_TOP_CFG_REG_4_da_coul_il_int1_chop_en_START (1)
#define REG_COUL_TOP_CFG_REG_4_da_coul_il_int1_chop_en_END (1)
#define REG_COUL_TOP_CFG_REG_4_da_coul_il_debug_ctrlh_START (2)
#define REG_COUL_TOP_CFG_REG_4_da_coul_il_debug_ctrlh_END (2)
#define REG_COUL_TOP_CFG_REG_4_da_coul_il_debug_ctrl_START (3)
#define REG_COUL_TOP_CFG_REG_4_da_coul_il_debug_ctrl_END (4)
#define REG_COUL_TOP_CFG_REG_4_da_coul_ih_sys_chop_en_START (5)
#define REG_COUL_TOP_CFG_REG_4_da_coul_ih_sys_chop_en_END (5)
#define REG_COUL_TOP_CFG_REG_4_da_coul_ih_pga_chop_en_START (6)
#define REG_COUL_TOP_CFG_REG_4_da_coul_ih_pga_chop_en_END (6)
#define REG_COUL_TOP_CFG_REG_4_da_coul_ih_int1_chop_en_START (7)
#define REG_COUL_TOP_CFG_REG_4_da_coul_ih_int1_chop_en_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char da_coul_il_sys_chop_en : 1;
        unsigned char reserved : 7;
    } reg;
} REG_COUL_TOP_CFG_REG_5_UNION;
#endif
#define REG_COUL_TOP_CFG_REG_5_da_coul_il_sys_chop_en_START (0)
#define REG_COUL_TOP_CFG_REG_5_da_coul_il_sys_chop_en_END (0)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char da_coul_reserve0 : 8;
    } reg;
} REG_COUL_TOP_CFG_REG_6_UNION;
#endif
#define REG_COUL_TOP_CFG_REG_6_da_coul_reserve0_START (0)
#define REG_COUL_TOP_CFG_REG_6_da_coul_reserve0_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char da_coul_reserve1 : 8;
    } reg;
} REG_COUL_TOP_CFG_REG_7_UNION;
#endif
#define REG_COUL_TOP_CFG_REG_7_da_coul_reserve1_START (0)
#define REG_COUL_TOP_CFG_REG_7_da_coul_reserve1_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char da_coul_reserve2 : 8;
    } reg;
} REG_COUL_TOP_CFG_REG_8_UNION;
#endif
#define REG_COUL_TOP_CFG_REG_8_da_coul_reserve2_START (0)
#define REG_COUL_TOP_CFG_REG_8_da_coul_reserve2_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char da_coul_reserve3 : 8;
    } reg;
} REG_COUL_TOP_CFG_REG_9_UNION;
#endif
#define REG_COUL_TOP_CFG_REG_9_da_coul_reserve3_START (0)
#define REG_COUL_TOP_CFG_REG_9_da_coul_reserve3_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char da_coul_v_debug_ctrl : 2;
        unsigned char da_coul_v_amp_ibias_sel : 2;
        unsigned char da_coul_v_alias_en : 1;
        unsigned char da_coul_test_mode : 2;
        unsigned char da_coul_sys_chop_freq_sel : 1;
    } reg;
} REG_COUL_TOP_CFG_REG_10_UNION;
#endif
#define REG_COUL_TOP_CFG_REG_10_da_coul_v_debug_ctrl_START (0)
#define REG_COUL_TOP_CFG_REG_10_da_coul_v_debug_ctrl_END (1)
#define REG_COUL_TOP_CFG_REG_10_da_coul_v_amp_ibias_sel_START (2)
#define REG_COUL_TOP_CFG_REG_10_da_coul_v_amp_ibias_sel_END (3)
#define REG_COUL_TOP_CFG_REG_10_da_coul_v_alias_en_START (4)
#define REG_COUL_TOP_CFG_REG_10_da_coul_v_alias_en_END (4)
#define REG_COUL_TOP_CFG_REG_10_da_coul_test_mode_START (5)
#define REG_COUL_TOP_CFG_REG_10_da_coul_test_mode_END (6)
#define REG_COUL_TOP_CFG_REG_10_da_coul_sys_chop_freq_sel_START (7)
#define REG_COUL_TOP_CFG_REG_10_da_coul_sys_chop_freq_sel_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char da_coul_v_sys_chop_en : 1;
        unsigned char da_coul_v_pga_gain_sel : 1;
        unsigned char da_coul_v_pga_chop_en : 1;
        unsigned char da_coul_v_int1_chop_en : 1;
        unsigned char da_coul_v_debug_pgah : 1;
        unsigned char reserved : 3;
    } reg;
} REG_COUL_TOP_CFG_REG_11_UNION;
#endif
#define REG_COUL_TOP_CFG_REG_11_da_coul_v_sys_chop_en_START (0)
#define REG_COUL_TOP_CFG_REG_11_da_coul_v_sys_chop_en_END (0)
#define REG_COUL_TOP_CFG_REG_11_da_coul_v_pga_gain_sel_START (1)
#define REG_COUL_TOP_CFG_REG_11_da_coul_v_pga_gain_sel_END (1)
#define REG_COUL_TOP_CFG_REG_11_da_coul_v_pga_chop_en_START (2)
#define REG_COUL_TOP_CFG_REG_11_da_coul_v_pga_chop_en_END (2)
#define REG_COUL_TOP_CFG_REG_11_da_coul_v_int1_chop_en_START (3)
#define REG_COUL_TOP_CFG_REG_11_da_coul_v_int1_chop_en_END (3)
#define REG_COUL_TOP_CFG_REG_11_da_coul_v_debug_pgah_START (4)
#define REG_COUL_TOP_CFG_REG_11_da_coul_v_debug_pgah_END (4)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char h_cl_out_s0 : 8;
    } reg;
} REG_H_CL_OUT0_UNION;
#endif
#define REG_H_CL_OUT0_h_cl_out_s0_START (0)
#define REG_H_CL_OUT0_h_cl_out_s0_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char h_cl_out_s1 : 8;
    } reg;
} REG_H_CL_OUT1_UNION;
#endif
#define REG_H_CL_OUT1_h_cl_out_s1_START (0)
#define REG_H_CL_OUT1_h_cl_out_s1_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char h_cl_out_s2 : 8;
    } reg;
} REG_H_CL_OUT2_UNION;
#endif
#define REG_H_CL_OUT2_h_cl_out_s2_START (0)
#define REG_H_CL_OUT2_h_cl_out_s2_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char h_cl_out_s3 : 8;
    } reg;
} REG_H_CL_OUT3_UNION;
#endif
#define REG_H_CL_OUT3_h_cl_out_s3_START (0)
#define REG_H_CL_OUT3_h_cl_out_s3_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char h_cl_out_s4 : 8;
    } reg;
} REG_H_CL_OUT4_UNION;
#endif
#define REG_H_CL_OUT4_h_cl_out_s4_START (0)
#define REG_H_CL_OUT4_h_cl_out_s4_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char h_cl_in_s0 : 8;
    } reg;
} REG_H_CL_IN0_UNION;
#endif
#define REG_H_CL_IN0_h_cl_in_s0_START (0)
#define REG_H_CL_IN0_h_cl_in_s0_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char h_cl_in_s1 : 8;
    } reg;
} REG_H_CL_IN1_UNION;
#endif
#define REG_H_CL_IN1_h_cl_in_s1_START (0)
#define REG_H_CL_IN1_h_cl_in_s1_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char h_cl_in_s2 : 8;
    } reg;
} REG_H_CL_IN2_UNION;
#endif
#define REG_H_CL_IN2_h_cl_in_s2_START (0)
#define REG_H_CL_IN2_h_cl_in_s2_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char h_cl_in_s3 : 8;
    } reg;
} REG_H_CL_IN3_UNION;
#endif
#define REG_H_CL_IN3_h_cl_in_s3_START (0)
#define REG_H_CL_IN3_h_cl_in_s3_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char h_cl_in_s4 : 8;
    } reg;
} REG_H_CL_IN4_UNION;
#endif
#define REG_H_CL_IN4_h_cl_in_s4_START (0)
#define REG_H_CL_IN4_h_cl_in_s4_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char h_chg_timer_s0 : 8;
    } reg;
} REG_H_CHG_TIMER0_UNION;
#endif
#define REG_H_CHG_TIMER0_h_chg_timer_s0_START (0)
#define REG_H_CHG_TIMER0_h_chg_timer_s0_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char h_chg_timer_s1 : 8;
    } reg;
} REG_H_CHG_TIMER1_UNION;
#endif
#define REG_H_CHG_TIMER1_h_chg_timer_s1_START (0)
#define REG_H_CHG_TIMER1_h_chg_timer_s1_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char h_chg_timer_s2 : 8;
    } reg;
} REG_H_CHG_TIMER2_UNION;
#endif
#define REG_H_CHG_TIMER2_h_chg_timer_s2_START (0)
#define REG_H_CHG_TIMER2_h_chg_timer_s2_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char h_chg_timer_s3 : 8;
    } reg;
} REG_H_CHG_TIMER3_UNION;
#endif
#define REG_H_CHG_TIMER3_h_chg_timer_s3_START (0)
#define REG_H_CHG_TIMER3_h_chg_timer_s3_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char h_load_timer_s0 : 8;
    } reg;
} REG_H_LOAD_TIMER0_UNION;
#endif
#define REG_H_LOAD_TIMER0_h_load_timer_s0_START (0)
#define REG_H_LOAD_TIMER0_h_load_timer_s0_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char h_load_timer_s1 : 8;
    } reg;
} REG_H_LOAD_TIMER1_UNION;
#endif
#define REG_H_LOAD_TIMER1_h_load_timer_s1_START (0)
#define REG_H_LOAD_TIMER1_h_load_timer_s1_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char h_load_timer_s2 : 8;
    } reg;
} REG_H_LOAD_TIMER2_UNION;
#endif
#define REG_H_LOAD_TIMER2_h_load_timer_s2_START (0)
#define REG_H_LOAD_TIMER2_h_load_timer_s2_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char h_load_timer_s3 : 8;
    } reg;
} REG_H_LOAD_TIMER3_UNION;
#endif
#define REG_H_LOAD_TIMER3_h_load_timer_s3_START (0)
#define REG_H_LOAD_TIMER3_h_load_timer_s3_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char h0_offset_current_mod_s0 : 8;
    } reg;
} REG_H0_OFFSET_CURRENT_MOD_0_UNION;
#endif
#define REG_H0_OFFSET_CURRENT_MOD_0_h0_offset_current_mod_s0_START (0)
#define REG_H0_OFFSET_CURRENT_MOD_0_h0_offset_current_mod_s0_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char h0_offset_current_mod_s1 : 8;
    } reg;
} REG_H0_OFFSET_CURRENT_MOD_1_UNION;
#endif
#define REG_H0_OFFSET_CURRENT_MOD_1_h0_offset_current_mod_s1_START (0)
#define REG_H0_OFFSET_CURRENT_MOD_1_h0_offset_current_mod_s1_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char h0_offset_current_mod_s2 : 8;
    } reg;
} REG_H0_OFFSET_CURRENT_MOD_2_UNION;
#endif
#define REG_H0_OFFSET_CURRENT_MOD_2_h0_offset_current_mod_s2_START (0)
#define REG_H0_OFFSET_CURRENT_MOD_2_h0_offset_current_mod_s2_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char h1_offset_current_mod_s0 : 8;
    } reg;
} REG_H1_OFFSET_CURRENT_MOD_0_UNION;
#endif
#define REG_H1_OFFSET_CURRENT_MOD_0_h1_offset_current_mod_s0_START (0)
#define REG_H1_OFFSET_CURRENT_MOD_0_h1_offset_current_mod_s0_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char h1_offset_current_mod_s1 : 8;
    } reg;
} REG_H1_OFFSET_CURRENT_MOD_1_UNION;
#endif
#define REG_H1_OFFSET_CURRENT_MOD_1_h1_offset_current_mod_s1_START (0)
#define REG_H1_OFFSET_CURRENT_MOD_1_h1_offset_current_mod_s1_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char h1_offset_current_mod_s2 : 8;
    } reg;
} REG_H1_OFFSET_CURRENT_MOD_2_UNION;
#endif
#define REG_H1_OFFSET_CURRENT_MOD_2_h1_offset_current_mod_s2_START (0)
#define REG_H1_OFFSET_CURRENT_MOD_2_h1_offset_current_mod_s2_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char h_offset_voltage_mod_s0 : 8;
    } reg;
} REG_H_OFFSET_VOLTAGE_MOD_0_UNION;
#endif
#define REG_H_OFFSET_VOLTAGE_MOD_0_h_offset_voltage_mod_s0_START (0)
#define REG_H_OFFSET_VOLTAGE_MOD_0_h_offset_voltage_mod_s0_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char h_offset_voltage_mod_s1 : 8;
    } reg;
} REG_H_OFFSET_VOLTAGE_MOD_1_UNION;
#endif
#define REG_H_OFFSET_VOLTAGE_MOD_1_h_offset_voltage_mod_s1_START (0)
#define REG_H_OFFSET_VOLTAGE_MOD_1_h_offset_voltage_mod_s1_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char h_offset_voltage_mod_s2 : 8;
    } reg;
} REG_H_OFFSET_VOLTAGE_MOD_2_UNION;
#endif
#define REG_H_OFFSET_VOLTAGE_MOD_2_h_offset_voltage_mod_s2_START (0)
#define REG_H_OFFSET_VOLTAGE_MOD_2_h_offset_voltage_mod_s2_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char h_current_s0 : 8;
    } reg;
} REG_H_CURRENT_0_UNION;
#endif
#define REG_H_CURRENT_0_h_current_s0_START (0)
#define REG_H_CURRENT_0_h_current_s0_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char h_current_s1 : 8;
    } reg;
} REG_H_CURRENT_1_UNION;
#endif
#define REG_H_CURRENT_1_h_current_s1_START (0)
#define REG_H_CURRENT_1_h_current_s1_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char h_current_s2 : 8;
    } reg;
} REG_H_CURRENT_2_UNION;
#endif
#define REG_H_CURRENT_2_h_current_s2_START (0)
#define REG_H_CURRENT_2_h_current_s2_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char h_v_out_s0 : 8;
    } reg;
} REG_H_V_OUT_0_UNION;
#endif
#define REG_H_V_OUT_0_h_v_out_s0_START (0)
#define REG_H_V_OUT_0_h_v_out_s0_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char h_v_out_s1 : 8;
    } reg;
} REG_H_V_OUT_1_UNION;
#endif
#define REG_H_V_OUT_1_h_v_out_s1_START (0)
#define REG_H_V_OUT_1_h_v_out_s1_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char h_v_out_s2 : 8;
    } reg;
} REG_H_V_OUT_2_UNION;
#endif
#define REG_H_V_OUT_2_h_v_out_s2_START (0)
#define REG_H_V_OUT_2_h_v_out_s2_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char h0_offset_current_s0 : 8;
    } reg;
} REG_H0_OFFSET_CURRENT0_UNION;
#endif
#define REG_H0_OFFSET_CURRENT0_h0_offset_current_s0_START (0)
#define REG_H0_OFFSET_CURRENT0_h0_offset_current_s0_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char h0_offset_current_s1 : 8;
    } reg;
} REG_H0_OFFSET_CURRENT1_UNION;
#endif
#define REG_H0_OFFSET_CURRENT1_h0_offset_current_s1_START (0)
#define REG_H0_OFFSET_CURRENT1_h0_offset_current_s1_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char h0_offset_current_s2 : 8;
    } reg;
} REG_H0_OFFSET_CURRENT2_UNION;
#endif
#define REG_H0_OFFSET_CURRENT2_h0_offset_current_s2_START (0)
#define REG_H0_OFFSET_CURRENT2_h0_offset_current_s2_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char h1_offset_current_s0 : 8;
    } reg;
} REG_H1_OFFSET_CURRENT0_UNION;
#endif
#define REG_H1_OFFSET_CURRENT0_h1_offset_current_s0_START (0)
#define REG_H1_OFFSET_CURRENT0_h1_offset_current_s0_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char h1_offset_current_s1 : 8;
    } reg;
} REG_H1_OFFSET_CURRENT1_UNION;
#endif
#define REG_H1_OFFSET_CURRENT1_h1_offset_current_s1_START (0)
#define REG_H1_OFFSET_CURRENT1_h1_offset_current_s1_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char h1_offset_current_s2 : 8;
    } reg;
} REG_H1_OFFSET_CURRENT2_UNION;
#endif
#define REG_H1_OFFSET_CURRENT2_h1_offset_current_s2_START (0)
#define REG_H1_OFFSET_CURRENT2_h1_offset_current_s2_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char h_offset_voltage_s0 : 8;
    } reg;
} REG_H_OFFSET_VOLTAGE0_UNION;
#endif
#define REG_H_OFFSET_VOLTAGE0_h_offset_voltage_s0_START (0)
#define REG_H_OFFSET_VOLTAGE0_h_offset_voltage_s0_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char h_offset_voltage_s1 : 8;
    } reg;
} REG_H_OFFSET_VOLTAGE1_UNION;
#endif
#define REG_H_OFFSET_VOLTAGE1_h_offset_voltage_s1_START (0)
#define REG_H_OFFSET_VOLTAGE1_h_offset_voltage_s1_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char h_offset_voltage_s2 : 8;
    } reg;
} REG_H_OFFSET_VOLTAGE2_UNION;
#endif
#define REG_H_OFFSET_VOLTAGE2_h_offset_voltage_s2_START (0)
#define REG_H_OFFSET_VOLTAGE2_h_offset_voltage_s2_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char h_v_ocv_data_s0 : 8;
    } reg;
} REG_H_OCV_VOLTAGE0_UNION;
#endif
#define REG_H_OCV_VOLTAGE0_h_v_ocv_data_s0_START (0)
#define REG_H_OCV_VOLTAGE0_h_v_ocv_data_s0_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char h_v_ocv_data_s1 : 8;
    } reg;
} REG_H_OCV_VOLTAGE1_UNION;
#endif
#define REG_H_OCV_VOLTAGE1_h_v_ocv_data_s1_START (0)
#define REG_H_OCV_VOLTAGE1_h_v_ocv_data_s1_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char h_v_ocv_data_s2 : 8;
    } reg;
} REG_H_OCV_VOLTAGE2_UNION;
#endif
#define REG_H_OCV_VOLTAGE2_h_v_ocv_data_s2_START (0)
#define REG_H_OCV_VOLTAGE2_h_v_ocv_data_s2_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char h_i_ocv_data_s0 : 8;
    } reg;
} REG_H_OCV_CURRENT0_UNION;
#endif
#define REG_H_OCV_CURRENT0_h_i_ocv_data_s0_START (0)
#define REG_H_OCV_CURRENT0_h_i_ocv_data_s0_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char h_i_ocv_data_s1 : 8;
    } reg;
} REG_H_OCV_CURRENT1_UNION;
#endif
#define REG_H_OCV_CURRENT1_h_i_ocv_data_s1_START (0)
#define REG_H_OCV_CURRENT1_h_i_ocv_data_s1_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char h_i_ocv_data_s2 : 8;
    } reg;
} REG_H_OCV_CURRENT2_UNION;
#endif
#define REG_H_OCV_CURRENT2_h_i_ocv_data_s2_START (0)
#define REG_H_OCV_CURRENT2_h_i_ocv_data_s2_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char h_eco_out_clin_s0 : 8;
    } reg;
} REG_H_ECO_OUT_CLIN_0_UNION;
#endif
#define REG_H_ECO_OUT_CLIN_0_h_eco_out_clin_s0_START (0)
#define REG_H_ECO_OUT_CLIN_0_h_eco_out_clin_s0_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char h_eco_out_clin_s1 : 8;
    } reg;
} REG_H_ECO_OUT_CLIN_1_UNION;
#endif
#define REG_H_ECO_OUT_CLIN_1_h_eco_out_clin_s1_START (0)
#define REG_H_ECO_OUT_CLIN_1_h_eco_out_clin_s1_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char h_eco_out_clin_s2 : 8;
    } reg;
} REG_H_ECO_OUT_CLIN_2_UNION;
#endif
#define REG_H_ECO_OUT_CLIN_2_h_eco_out_clin_s2_START (0)
#define REG_H_ECO_OUT_CLIN_2_h_eco_out_clin_s2_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char h_eco_out_clin_s3 : 8;
    } reg;
} REG_H_ECO_OUT_CLIN_3_UNION;
#endif
#define REG_H_ECO_OUT_CLIN_3_h_eco_out_clin_s3_START (0)
#define REG_H_ECO_OUT_CLIN_3_h_eco_out_clin_s3_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char h_eco_out_clin_s4 : 8;
    } reg;
} REG_H_ECO_OUT_CLIN_4_UNION;
#endif
#define REG_H_ECO_OUT_CLIN_4_h_eco_out_clin_s4_START (0)
#define REG_H_ECO_OUT_CLIN_4_h_eco_out_clin_s4_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char h_eco_out_clout_s0 : 8;
    } reg;
} REG_H_ECO_OUT_CLOUT_0_UNION;
#endif
#define REG_H_ECO_OUT_CLOUT_0_h_eco_out_clout_s0_START (0)
#define REG_H_ECO_OUT_CLOUT_0_h_eco_out_clout_s0_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char h_eco_out_clout_s1 : 8;
    } reg;
} REG_H_ECO_OUT_CLOUT_1_UNION;
#endif
#define REG_H_ECO_OUT_CLOUT_1_h_eco_out_clout_s1_START (0)
#define REG_H_ECO_OUT_CLOUT_1_h_eco_out_clout_s1_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char h_eco_out_clout_s2 : 8;
    } reg;
} REG_H_ECO_OUT_CLOUT_2_UNION;
#endif
#define REG_H_ECO_OUT_CLOUT_2_h_eco_out_clout_s2_START (0)
#define REG_H_ECO_OUT_CLOUT_2_h_eco_out_clout_s2_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char h_eco_out_clout_s3 : 8;
    } reg;
} REG_H_ECO_OUT_CLOUT_3_UNION;
#endif
#define REG_H_ECO_OUT_CLOUT_3_h_eco_out_clout_s3_START (0)
#define REG_H_ECO_OUT_CLOUT_3_h_eco_out_clout_s3_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char h_eco_out_clout_s4 : 8;
    } reg;
} REG_H_ECO_OUT_CLOUT_4_UNION;
#endif
#define REG_H_ECO_OUT_CLOUT_4_h_eco_out_clout_s4_START (0)
#define REG_H_ECO_OUT_CLOUT_4_h_eco_out_clout_s4_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char h_vout_pre0_s0 : 8;
    } reg;
} REG_H_V_PRE0_OUT0_UNION;
#endif
#define REG_H_V_PRE0_OUT0_h_vout_pre0_s0_START (0)
#define REG_H_V_PRE0_OUT0_h_vout_pre0_s0_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char h_vout_pre0_s1 : 8;
    } reg;
} REG_H_V_PRE0_OUT1_UNION;
#endif
#define REG_H_V_PRE0_OUT1_h_vout_pre0_s1_START (0)
#define REG_H_V_PRE0_OUT1_h_vout_pre0_s1_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char h_vout_pre0_s2 : 8;
    } reg;
} REG_H_V_PRE0_OUT2_UNION;
#endif
#define REG_H_V_PRE0_OUT2_h_vout_pre0_s2_START (0)
#define REG_H_V_PRE0_OUT2_h_vout_pre0_s2_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char h_vout_pre1_s0 : 8;
    } reg;
} REG_H_V_PRE1_OUT0_UNION;
#endif
#define REG_H_V_PRE1_OUT0_h_vout_pre1_s0_START (0)
#define REG_H_V_PRE1_OUT0_h_vout_pre1_s0_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char h_vout_pre1_s1 : 8;
    } reg;
} REG_H_V_PRE1_OUT1_UNION;
#endif
#define REG_H_V_PRE1_OUT1_h_vout_pre1_s1_START (0)
#define REG_H_V_PRE1_OUT1_h_vout_pre1_s1_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char h_vout_pre1_s2 : 8;
    } reg;
} REG_H_V_PRE1_OUT2_UNION;
#endif
#define REG_H_V_PRE1_OUT2_h_vout_pre1_s2_START (0)
#define REG_H_V_PRE1_OUT2_h_vout_pre1_s2_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char h_vout_pre2_s0 : 8;
    } reg;
} REG_H_V_PRE2_OUT0_UNION;
#endif
#define REG_H_V_PRE2_OUT0_h_vout_pre2_s0_START (0)
#define REG_H_V_PRE2_OUT0_h_vout_pre2_s0_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char h_vout_pre2_s1 : 8;
    } reg;
} REG_H_V_PRE2_OUT1_UNION;
#endif
#define REG_H_V_PRE2_OUT1_h_vout_pre2_s1_START (0)
#define REG_H_V_PRE2_OUT1_h_vout_pre2_s1_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char h_vout_pre2_s2 : 8;
    } reg;
} REG_H_V_PRE2_OUT2_UNION;
#endif
#define REG_H_V_PRE2_OUT2_h_vout_pre2_s2_START (0)
#define REG_H_V_PRE2_OUT2_h_vout_pre2_s2_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char h_vout_pre3_s0 : 8;
    } reg;
} REG_H_V_PRE3_OUT0_UNION;
#endif
#define REG_H_V_PRE3_OUT0_h_vout_pre3_s0_START (0)
#define REG_H_V_PRE3_OUT0_h_vout_pre3_s0_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char h_vout_pre3_s1 : 8;
    } reg;
} REG_H_V_PRE3_OUT1_UNION;
#endif
#define REG_H_V_PRE3_OUT1_h_vout_pre3_s1_START (0)
#define REG_H_V_PRE3_OUT1_h_vout_pre3_s1_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char h_vout_pre3_s2 : 8;
    } reg;
} REG_H_V_PRE3_OUT2_UNION;
#endif
#define REG_H_V_PRE3_OUT2_h_vout_pre3_s2_START (0)
#define REG_H_V_PRE3_OUT2_h_vout_pre3_s2_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char h_vout_pre4_s0 : 8;
    } reg;
} REG_H_V_PRE4_OUT0_UNION;
#endif
#define REG_H_V_PRE4_OUT0_h_vout_pre4_s0_START (0)
#define REG_H_V_PRE4_OUT0_h_vout_pre4_s0_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char h_vout_pre4_s1 : 8;
    } reg;
} REG_H_V_PRE4_OUT1_UNION;
#endif
#define REG_H_V_PRE4_OUT1_h_vout_pre4_s1_START (0)
#define REG_H_V_PRE4_OUT1_h_vout_pre4_s1_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char h_vout_pre4_s2 : 8;
    } reg;
} REG_H_V_PRE4_OUT2_UNION;
#endif
#define REG_H_V_PRE4_OUT2_h_vout_pre4_s2_START (0)
#define REG_H_V_PRE4_OUT2_h_vout_pre4_s2_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char h_vout_pre5_s0 : 8;
    } reg;
} REG_H_V_PRE5_OUT0_UNION;
#endif
#define REG_H_V_PRE5_OUT0_h_vout_pre5_s0_START (0)
#define REG_H_V_PRE5_OUT0_h_vout_pre5_s0_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char h_vout_pre5_s1 : 8;
    } reg;
} REG_H_V_PRE5_OUT1_UNION;
#endif
#define REG_H_V_PRE5_OUT1_h_vout_pre5_s1_START (0)
#define REG_H_V_PRE5_OUT1_h_vout_pre5_s1_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char h_vout_pre5_s2 : 8;
    } reg;
} REG_H_V_PRE5_OUT2_UNION;
#endif
#define REG_H_V_PRE5_OUT2_h_vout_pre5_s2_START (0)
#define REG_H_V_PRE5_OUT2_h_vout_pre5_s2_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char h_vout_pre6_s0 : 8;
    } reg;
} REG_H_V_PRE6_OUT0_UNION;
#endif
#define REG_H_V_PRE6_OUT0_h_vout_pre6_s0_START (0)
#define REG_H_V_PRE6_OUT0_h_vout_pre6_s0_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char h_vout_pre6_s1 : 8;
    } reg;
} REG_H_V_PRE6_OUT1_UNION;
#endif
#define REG_H_V_PRE6_OUT1_h_vout_pre6_s1_START (0)
#define REG_H_V_PRE6_OUT1_h_vout_pre6_s1_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char h_vout_pre6_s2 : 8;
    } reg;
} REG_H_V_PRE6_OUT2_UNION;
#endif
#define REG_H_V_PRE6_OUT2_h_vout_pre6_s2_START (0)
#define REG_H_V_PRE6_OUT2_h_vout_pre6_s2_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char h_vout_pre7_s0 : 8;
    } reg;
} REG_H_V_PRE7_OUT0_UNION;
#endif
#define REG_H_V_PRE7_OUT0_h_vout_pre7_s0_START (0)
#define REG_H_V_PRE7_OUT0_h_vout_pre7_s0_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char h_vout_pre7_s1 : 8;
    } reg;
} REG_H_V_PRE7_OUT1_UNION;
#endif
#define REG_H_V_PRE7_OUT1_h_vout_pre7_s1_START (0)
#define REG_H_V_PRE7_OUT1_h_vout_pre7_s1_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char h_vout_pre7_s2 : 8;
    } reg;
} REG_H_V_PRE7_OUT2_UNION;
#endif
#define REG_H_V_PRE7_OUT2_h_vout_pre7_s2_START (0)
#define REG_H_V_PRE7_OUT2_h_vout_pre7_s2_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char h_current_pre0_s0 : 8;
    } reg;
} REG_H_CURRENT_PRE0_OUT0_UNION;
#endif
#define REG_H_CURRENT_PRE0_OUT0_h_current_pre0_s0_START (0)
#define REG_H_CURRENT_PRE0_OUT0_h_current_pre0_s0_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char h_current_pre0_s1 : 8;
    } reg;
} REG_H_CURRENT_PRE0_OUT1_UNION;
#endif
#define REG_H_CURRENT_PRE0_OUT1_h_current_pre0_s1_START (0)
#define REG_H_CURRENT_PRE0_OUT1_h_current_pre0_s1_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char h_current_pre0_s2 : 8;
    } reg;
} REG_H_CURRENT_PRE0_OUT2_UNION;
#endif
#define REG_H_CURRENT_PRE0_OUT2_h_current_pre0_s2_START (0)
#define REG_H_CURRENT_PRE0_OUT2_h_current_pre0_s2_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char h_current_pre1_s0 : 8;
    } reg;
} REG_H_CURRENT_PRE1_OUT0_UNION;
#endif
#define REG_H_CURRENT_PRE1_OUT0_h_current_pre1_s0_START (0)
#define REG_H_CURRENT_PRE1_OUT0_h_current_pre1_s0_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char h_current_pre1_s1 : 8;
    } reg;
} REG_H_CURRENT_PRE1_OUT1_UNION;
#endif
#define REG_H_CURRENT_PRE1_OUT1_h_current_pre1_s1_START (0)
#define REG_H_CURRENT_PRE1_OUT1_h_current_pre1_s1_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char h_current_pre1_s2 : 8;
    } reg;
} REG_H_CURRENT_PRE1_OUT2_UNION;
#endif
#define REG_H_CURRENT_PRE1_OUT2_h_current_pre1_s2_START (0)
#define REG_H_CURRENT_PRE1_OUT2_h_current_pre1_s2_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char h_current_pre2_s0 : 8;
    } reg;
} REG_H_CURRENT_PRE2_OUT0_UNION;
#endif
#define REG_H_CURRENT_PRE2_OUT0_h_current_pre2_s0_START (0)
#define REG_H_CURRENT_PRE2_OUT0_h_current_pre2_s0_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char h_current_pre2_s1 : 8;
    } reg;
} REG_H_CURRENT_PRE2_OUT1_UNION;
#endif
#define REG_H_CURRENT_PRE2_OUT1_h_current_pre2_s1_START (0)
#define REG_H_CURRENT_PRE2_OUT1_h_current_pre2_s1_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char h_current_pre2_s2 : 8;
    } reg;
} REG_H_CURRENT_PRE2_OUT2_UNION;
#endif
#define REG_H_CURRENT_PRE2_OUT2_h_current_pre2_s2_START (0)
#define REG_H_CURRENT_PRE2_OUT2_h_current_pre2_s2_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char h_current_pre3_s0 : 8;
    } reg;
} REG_H_CURRENT_PRE3_OUT0_UNION;
#endif
#define REG_H_CURRENT_PRE3_OUT0_h_current_pre3_s0_START (0)
#define REG_H_CURRENT_PRE3_OUT0_h_current_pre3_s0_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char h_current_pre3_s1 : 8;
    } reg;
} REG_H_CURRENT_PRE3_OUT1_UNION;
#endif
#define REG_H_CURRENT_PRE3_OUT1_h_current_pre3_s1_START (0)
#define REG_H_CURRENT_PRE3_OUT1_h_current_pre3_s1_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char h_current_pre3_s2 : 8;
    } reg;
} REG_H_CURRENT_PRE3_OUT2_UNION;
#endif
#define REG_H_CURRENT_PRE3_OUT2_h_current_pre3_s2_START (0)
#define REG_H_CURRENT_PRE3_OUT2_h_current_pre3_s2_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char h_current_pre4_s0 : 8;
    } reg;
} REG_H_CURRENT_PRE4_OUT0_UNION;
#endif
#define REG_H_CURRENT_PRE4_OUT0_h_current_pre4_s0_START (0)
#define REG_H_CURRENT_PRE4_OUT0_h_current_pre4_s0_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char h_current_pre4_s1 : 8;
    } reg;
} REG_H_CURRENT_PRE4_OUT1_UNION;
#endif
#define REG_H_CURRENT_PRE4_OUT1_h_current_pre4_s1_START (0)
#define REG_H_CURRENT_PRE4_OUT1_h_current_pre4_s1_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char h_current_pre4_s2 : 8;
    } reg;
} REG_H_CURRENT_PRE4_OUT2_UNION;
#endif
#define REG_H_CURRENT_PRE4_OUT2_h_current_pre4_s2_START (0)
#define REG_H_CURRENT_PRE4_OUT2_h_current_pre4_s2_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char h_current_pre5_s0 : 8;
    } reg;
} REG_H_CURRENT_PRE5_OUT0_UNION;
#endif
#define REG_H_CURRENT_PRE5_OUT0_h_current_pre5_s0_START (0)
#define REG_H_CURRENT_PRE5_OUT0_h_current_pre5_s0_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char h_current_pre5_s1 : 8;
    } reg;
} REG_H_CURRENT_PRE5_OUT1_UNION;
#endif
#define REG_H_CURRENT_PRE5_OUT1_h_current_pre5_s1_START (0)
#define REG_H_CURRENT_PRE5_OUT1_h_current_pre5_s1_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char h_current_pre5_s2 : 8;
    } reg;
} REG_H_CURRENT_PRE5_OUT2_UNION;
#endif
#define REG_H_CURRENT_PRE5_OUT2_h_current_pre5_s2_START (0)
#define REG_H_CURRENT_PRE5_OUT2_h_current_pre5_s2_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char h_current_pre6_s0 : 8;
    } reg;
} REG_H_CURRENT_PRE6_OUT0_UNION;
#endif
#define REG_H_CURRENT_PRE6_OUT0_h_current_pre6_s0_START (0)
#define REG_H_CURRENT_PRE6_OUT0_h_current_pre6_s0_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char h_current_pre6_s1 : 8;
    } reg;
} REG_H_CURRENT_PRE6_OUT1_UNION;
#endif
#define REG_H_CURRENT_PRE6_OUT1_h_current_pre6_s1_START (0)
#define REG_H_CURRENT_PRE6_OUT1_h_current_pre6_s1_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char h_current_pre6_s2 : 8;
    } reg;
} REG_H_CURRENT_PRE6_OUT2_UNION;
#endif
#define REG_H_CURRENT_PRE6_OUT2_h_current_pre6_s2_START (0)
#define REG_H_CURRENT_PRE6_OUT2_h_current_pre6_s2_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char h_current_pre7_s0 : 8;
    } reg;
} REG_H_CURRENT_PRE7_OUT0_UNION;
#endif
#define REG_H_CURRENT_PRE7_OUT0_h_current_pre7_s0_START (0)
#define REG_H_CURRENT_PRE7_OUT0_h_current_pre7_s0_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char h_current_pre7_s1 : 8;
    } reg;
} REG_H_CURRENT_PRE7_OUT1_UNION;
#endif
#define REG_H_CURRENT_PRE7_OUT1_h_current_pre7_s1_START (0)
#define REG_H_CURRENT_PRE7_OUT1_h_current_pre7_s1_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char h_current_pre7_s2 : 8;
    } reg;
} REG_H_CURRENT_PRE7_OUT2_UNION;
#endif
#define REG_H_CURRENT_PRE7_OUT2_h_current_pre7_s2_START (0)
#define REG_H_CURRENT_PRE7_OUT2_h_current_pre7_s2_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char h_v_ocv_out_pre1_s0 : 8;
    } reg;
} REG_H_V_OCV_PRE1_OUT0_UNION;
#endif
#define REG_H_V_OCV_PRE1_OUT0_h_v_ocv_out_pre1_s0_START (0)
#define REG_H_V_OCV_PRE1_OUT0_h_v_ocv_out_pre1_s0_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char h_v_ocv_out_pre1_s1 : 8;
    } reg;
} REG_H_V_OCV_PRE1_OUT1_UNION;
#endif
#define REG_H_V_OCV_PRE1_OUT1_h_v_ocv_out_pre1_s1_START (0)
#define REG_H_V_OCV_PRE1_OUT1_h_v_ocv_out_pre1_s1_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char h_v_ocv_out_pre1_s2 : 8;
    } reg;
} REG_H_V_OCV_PRE1_OUT2_UNION;
#endif
#define REG_H_V_OCV_PRE1_OUT2_h_v_ocv_out_pre1_s2_START (0)
#define REG_H_V_OCV_PRE1_OUT2_h_v_ocv_out_pre1_s2_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char h_v_ocv_out_pre2_s0 : 8;
    } reg;
} REG_H_V_OCV_PRE2_OUT0_UNION;
#endif
#define REG_H_V_OCV_PRE2_OUT0_h_v_ocv_out_pre2_s0_START (0)
#define REG_H_V_OCV_PRE2_OUT0_h_v_ocv_out_pre2_s0_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char h_v_ocv_out_pre2_s1 : 8;
    } reg;
} REG_H_V_OCV_PRE2_OUT1_UNION;
#endif
#define REG_H_V_OCV_PRE2_OUT1_h_v_ocv_out_pre2_s1_START (0)
#define REG_H_V_OCV_PRE2_OUT1_h_v_ocv_out_pre2_s1_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char h_v_ocv_out_pre2_s2 : 8;
    } reg;
} REG_H_V_OCV_PRE2_OUT2_UNION;
#endif
#define REG_H_V_OCV_PRE2_OUT2_h_v_ocv_out_pre2_s2_START (0)
#define REG_H_V_OCV_PRE2_OUT2_h_v_ocv_out_pre2_s2_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char h_v_ocv_out_pre3_s0 : 8;
    } reg;
} REG_H_V_OCV_PRE3_OUT0_UNION;
#endif
#define REG_H_V_OCV_PRE3_OUT0_h_v_ocv_out_pre3_s0_START (0)
#define REG_H_V_OCV_PRE3_OUT0_h_v_ocv_out_pre3_s0_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char h_v_ocv_out_pre3_s1 : 8;
    } reg;
} REG_H_V_OCV_PRE3_OUT1_UNION;
#endif
#define REG_H_V_OCV_PRE3_OUT1_h_v_ocv_out_pre3_s1_START (0)
#define REG_H_V_OCV_PRE3_OUT1_h_v_ocv_out_pre3_s1_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char h_v_ocv_out_pre3_s2 : 8;
    } reg;
} REG_H_V_OCV_PRE3_OUT2_UNION;
#endif
#define REG_H_V_OCV_PRE3_OUT2_h_v_ocv_out_pre3_s2_START (0)
#define REG_H_V_OCV_PRE3_OUT2_h_v_ocv_out_pre3_s2_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char h_v_ocv_out_pre4_s0 : 8;
    } reg;
} REG_H_V_OCV_PRE4_OUT0_UNION;
#endif
#define REG_H_V_OCV_PRE4_OUT0_h_v_ocv_out_pre4_s0_START (0)
#define REG_H_V_OCV_PRE4_OUT0_h_v_ocv_out_pre4_s0_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char h_v_ocv_out_pre4_s1 : 8;
    } reg;
} REG_H_V_OCV_PRE4_OUT1_UNION;
#endif
#define REG_H_V_OCV_PRE4_OUT1_h_v_ocv_out_pre4_s1_START (0)
#define REG_H_V_OCV_PRE4_OUT1_h_v_ocv_out_pre4_s1_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char h_v_ocv_out_pre4_s2 : 8;
    } reg;
} REG_H_V_OCV_PRE4_OUT2_UNION;
#endif
#define REG_H_V_OCV_PRE4_OUT2_h_v_ocv_out_pre4_s2_START (0)
#define REG_H_V_OCV_PRE4_OUT2_h_v_ocv_out_pre4_s2_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char h_i_ocv_out_pre1_s0 : 8;
    } reg;
} REG_H_I_OCV_PRE1_OUT0_UNION;
#endif
#define REG_H_I_OCV_PRE1_OUT0_h_i_ocv_out_pre1_s0_START (0)
#define REG_H_I_OCV_PRE1_OUT0_h_i_ocv_out_pre1_s0_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char h_i_ocv_out_pre1_s1 : 8;
    } reg;
} REG_H_I_OCV_PRE1_OUT1_UNION;
#endif
#define REG_H_I_OCV_PRE1_OUT1_h_i_ocv_out_pre1_s1_START (0)
#define REG_H_I_OCV_PRE1_OUT1_h_i_ocv_out_pre1_s1_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char h_i_ocv_out_pre1_s2 : 8;
    } reg;
} REG_H_I_OCV_PRE1_OUT2_UNION;
#endif
#define REG_H_I_OCV_PRE1_OUT2_h_i_ocv_out_pre1_s2_START (0)
#define REG_H_I_OCV_PRE1_OUT2_h_i_ocv_out_pre1_s2_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char h_i_ocv_out_pre2_s0 : 8;
    } reg;
} REG_H_I_OCV_PRE2_OUT0_UNION;
#endif
#define REG_H_I_OCV_PRE2_OUT0_h_i_ocv_out_pre2_s0_START (0)
#define REG_H_I_OCV_PRE2_OUT0_h_i_ocv_out_pre2_s0_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char h_i_ocv_out_pre2_s1 : 8;
    } reg;
} REG_H_I_OCV_PRE2_OUT1_UNION;
#endif
#define REG_H_I_OCV_PRE2_OUT1_h_i_ocv_out_pre2_s1_START (0)
#define REG_H_I_OCV_PRE2_OUT1_h_i_ocv_out_pre2_s1_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char h_i_ocv_out_pre2_s2 : 8;
    } reg;
} REG_H_I_OCV_PRE2_OUT2_UNION;
#endif
#define REG_H_I_OCV_PRE2_OUT2_h_i_ocv_out_pre2_s2_START (0)
#define REG_H_I_OCV_PRE2_OUT2_h_i_ocv_out_pre2_s2_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char h_i_ocv_out_pre3_s0 : 8;
    } reg;
} REG_H_I_OCV_PRE3_OUT0_UNION;
#endif
#define REG_H_I_OCV_PRE3_OUT0_h_i_ocv_out_pre3_s0_START (0)
#define REG_H_I_OCV_PRE3_OUT0_h_i_ocv_out_pre3_s0_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char h_i_ocv_out_pre3_s1 : 8;
    } reg;
} REG_H_I_OCV_PRE3_OUT1_UNION;
#endif
#define REG_H_I_OCV_PRE3_OUT1_h_i_ocv_out_pre3_s1_START (0)
#define REG_H_I_OCV_PRE3_OUT1_h_i_ocv_out_pre3_s1_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char h_i_ocv_out_pre3_s2 : 8;
    } reg;
} REG_H_I_OCV_PRE3_OUT2_UNION;
#endif
#define REG_H_I_OCV_PRE3_OUT2_h_i_ocv_out_pre3_s2_START (0)
#define REG_H_I_OCV_PRE3_OUT2_h_i_ocv_out_pre3_s2_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char h_i_ocv_out_pre4_s0 : 8;
    } reg;
} REG_H_I_OCV_PRE4_OUT0_UNION;
#endif
#define REG_H_I_OCV_PRE4_OUT0_h_i_ocv_out_pre4_s0_START (0)
#define REG_H_I_OCV_PRE4_OUT0_h_i_ocv_out_pre4_s0_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char h_i_ocv_out_pre4_s1 : 8;
    } reg;
} REG_H_I_OCV_PRE4_OUT1_UNION;
#endif
#define REG_H_I_OCV_PRE4_OUT1_h_i_ocv_out_pre4_s1_START (0)
#define REG_H_I_OCV_PRE4_OUT1_h_i_ocv_out_pre4_s1_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char h_i_ocv_out_pre4_s2 : 8;
    } reg;
} REG_H_I_OCV_PRE4_OUT2_UNION;
#endif
#define REG_H_I_OCV_PRE4_OUT2_h_i_ocv_out_pre4_s2_START (0)
#define REG_H_I_OCV_PRE4_OUT2_h_i_ocv_out_pre4_s2_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char l_cl_out_s0 : 8;
    } reg;
} REG_L_CL_OUT0_UNION;
#endif
#define REG_L_CL_OUT0_l_cl_out_s0_START (0)
#define REG_L_CL_OUT0_l_cl_out_s0_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char l_cl_out_s1 : 8;
    } reg;
} REG_L_CL_OUT1_UNION;
#endif
#define REG_L_CL_OUT1_l_cl_out_s1_START (0)
#define REG_L_CL_OUT1_l_cl_out_s1_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char l_cl_out_s2 : 8;
    } reg;
} REG_L_CL_OUT2_UNION;
#endif
#define REG_L_CL_OUT2_l_cl_out_s2_START (0)
#define REG_L_CL_OUT2_l_cl_out_s2_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char l_cl_out_s3 : 8;
    } reg;
} REG_L_CL_OUT3_UNION;
#endif
#define REG_L_CL_OUT3_l_cl_out_s3_START (0)
#define REG_L_CL_OUT3_l_cl_out_s3_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char l_cl_out_s4 : 8;
    } reg;
} REG_L_CL_OUT4_UNION;
#endif
#define REG_L_CL_OUT4_l_cl_out_s4_START (0)
#define REG_L_CL_OUT4_l_cl_out_s4_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char l_cl_in_s0 : 8;
    } reg;
} REG_L_CL_IN0_UNION;
#endif
#define REG_L_CL_IN0_l_cl_in_s0_START (0)
#define REG_L_CL_IN0_l_cl_in_s0_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char l_cl_in_s1 : 8;
    } reg;
} REG_L_CL_IN1_UNION;
#endif
#define REG_L_CL_IN1_l_cl_in_s1_START (0)
#define REG_L_CL_IN1_l_cl_in_s1_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char l_cl_in_s2 : 8;
    } reg;
} REG_L_CL_IN2_UNION;
#endif
#define REG_L_CL_IN2_l_cl_in_s2_START (0)
#define REG_L_CL_IN2_l_cl_in_s2_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char l_cl_in_s3 : 8;
    } reg;
} REG_L_CL_IN3_UNION;
#endif
#define REG_L_CL_IN3_l_cl_in_s3_START (0)
#define REG_L_CL_IN3_l_cl_in_s3_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char l_cl_in_s4 : 8;
    } reg;
} REG_L_CL_IN4_UNION;
#endif
#define REG_L_CL_IN4_l_cl_in_s4_START (0)
#define REG_L_CL_IN4_l_cl_in_s4_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char l_chg_timer_s0 : 8;
    } reg;
} REG_L_CHG_TIMER0_UNION;
#endif
#define REG_L_CHG_TIMER0_l_chg_timer_s0_START (0)
#define REG_L_CHG_TIMER0_l_chg_timer_s0_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char l_chg_timer_s1 : 8;
    } reg;
} REG_L_CHG_TIMER1_UNION;
#endif
#define REG_L_CHG_TIMER1_l_chg_timer_s1_START (0)
#define REG_L_CHG_TIMER1_l_chg_timer_s1_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char l_chg_timer_s2 : 8;
    } reg;
} REG_L_CHG_TIMER2_UNION;
#endif
#define REG_L_CHG_TIMER2_l_chg_timer_s2_START (0)
#define REG_L_CHG_TIMER2_l_chg_timer_s2_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char l_chg_timer_s3 : 8;
    } reg;
} REG_L_CHG_TIMER3_UNION;
#endif
#define REG_L_CHG_TIMER3_l_chg_timer_s3_START (0)
#define REG_L_CHG_TIMER3_l_chg_timer_s3_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char l_load_timer_s0 : 8;
    } reg;
} REG_L_LOAD_TIMER0_UNION;
#endif
#define REG_L_LOAD_TIMER0_l_load_timer_s0_START (0)
#define REG_L_LOAD_TIMER0_l_load_timer_s0_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char l_load_timer_s1 : 8;
    } reg;
} REG_L_LOAD_TIMER1_UNION;
#endif
#define REG_L_LOAD_TIMER1_l_load_timer_s1_START (0)
#define REG_L_LOAD_TIMER1_l_load_timer_s1_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char l_load_timer_s2 : 8;
    } reg;
} REG_L_LOAD_TIMER2_UNION;
#endif
#define REG_L_LOAD_TIMER2_l_load_timer_s2_START (0)
#define REG_L_LOAD_TIMER2_l_load_timer_s2_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char l_load_timer_s3 : 8;
    } reg;
} REG_L_LOAD_TIMER3_UNION;
#endif
#define REG_L_LOAD_TIMER3_l_load_timer_s3_START (0)
#define REG_L_LOAD_TIMER3_l_load_timer_s3_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char l_offset_current_mod_s0 : 8;
    } reg;
} REG_L_OFFSET_CURRENT_MOD_0_UNION;
#endif
#define REG_L_OFFSET_CURRENT_MOD_0_l_offset_current_mod_s0_START (0)
#define REG_L_OFFSET_CURRENT_MOD_0_l_offset_current_mod_s0_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char l_offset_current_mod_s1 : 8;
    } reg;
} REG_L_OFFSET_CURRENT_MOD_1_UNION;
#endif
#define REG_L_OFFSET_CURRENT_MOD_1_l_offset_current_mod_s1_START (0)
#define REG_L_OFFSET_CURRENT_MOD_1_l_offset_current_mod_s1_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char l_offset_current_mod_s2 : 8;
    } reg;
} REG_L_OFFSET_CURRENT_MOD_2_UNION;
#endif
#define REG_L_OFFSET_CURRENT_MOD_2_l_offset_current_mod_s2_START (0)
#define REG_L_OFFSET_CURRENT_MOD_2_l_offset_current_mod_s2_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char l_offset_voltage_mod_s0 : 8;
    } reg;
} REG_L_OFFSET_VOLTAGE_MOD_0_UNION;
#endif
#define REG_L_OFFSET_VOLTAGE_MOD_0_l_offset_voltage_mod_s0_START (0)
#define REG_L_OFFSET_VOLTAGE_MOD_0_l_offset_voltage_mod_s0_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char l_offset_voltage_mod_s1 : 8;
    } reg;
} REG_L_OFFSET_VOLTAGE_MOD_1_UNION;
#endif
#define REG_L_OFFSET_VOLTAGE_MOD_1_l_offset_voltage_mod_s1_START (0)
#define REG_L_OFFSET_VOLTAGE_MOD_1_l_offset_voltage_mod_s1_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char l_offset_voltage_mod_s2 : 8;
    } reg;
} REG_L_OFFSET_VOLTAGE_MOD_2_UNION;
#endif
#define REG_L_OFFSET_VOLTAGE_MOD_2_l_offset_voltage_mod_s2_START (0)
#define REG_L_OFFSET_VOLTAGE_MOD_2_l_offset_voltage_mod_s2_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char l_current_s0 : 8;
    } reg;
} REG_L_CURRENT_0_UNION;
#endif
#define REG_L_CURRENT_0_l_current_s0_START (0)
#define REG_L_CURRENT_0_l_current_s0_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char l_current_s1 : 8;
    } reg;
} REG_L_CURRENT_1_UNION;
#endif
#define REG_L_CURRENT_1_l_current_s1_START (0)
#define REG_L_CURRENT_1_l_current_s1_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char l_current_s2 : 8;
    } reg;
} REG_L_CURRENT_2_UNION;
#endif
#define REG_L_CURRENT_2_l_current_s2_START (0)
#define REG_L_CURRENT_2_l_current_s2_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char l_v_out_s0 : 8;
    } reg;
} REG_L_V_OUT_0_UNION;
#endif
#define REG_L_V_OUT_0_l_v_out_s0_START (0)
#define REG_L_V_OUT_0_l_v_out_s0_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char l_v_out_s1 : 8;
    } reg;
} REG_L_V_OUT_1_UNION;
#endif
#define REG_L_V_OUT_1_l_v_out_s1_START (0)
#define REG_L_V_OUT_1_l_v_out_s1_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char l_v_out_s2 : 8;
    } reg;
} REG_L_V_OUT_2_UNION;
#endif
#define REG_L_V_OUT_2_l_v_out_s2_START (0)
#define REG_L_V_OUT_2_l_v_out_s2_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char l_offset_current_s0 : 8;
    } reg;
} REG_L_OFFSET_CURRENT0_UNION;
#endif
#define REG_L_OFFSET_CURRENT0_l_offset_current_s0_START (0)
#define REG_L_OFFSET_CURRENT0_l_offset_current_s0_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char l_offset_current_s1 : 8;
    } reg;
} REG_L_OFFSET_CURRENT1_UNION;
#endif
#define REG_L_OFFSET_CURRENT1_l_offset_current_s1_START (0)
#define REG_L_OFFSET_CURRENT1_l_offset_current_s1_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char l_offset_current_s2 : 8;
    } reg;
} REG_L_OFFSET_CURRENT2_UNION;
#endif
#define REG_L_OFFSET_CURRENT2_l_offset_current_s2_START (0)
#define REG_L_OFFSET_CURRENT2_l_offset_current_s2_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char l_offset_voltage_s0 : 8;
    } reg;
} REG_L_OFFSET_VOLTAGE0_UNION;
#endif
#define REG_L_OFFSET_VOLTAGE0_l_offset_voltage_s0_START (0)
#define REG_L_OFFSET_VOLTAGE0_l_offset_voltage_s0_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char l_offset_voltage_s1 : 8;
    } reg;
} REG_L_OFFSET_VOLTAGE1_UNION;
#endif
#define REG_L_OFFSET_VOLTAGE1_l_offset_voltage_s1_START (0)
#define REG_L_OFFSET_VOLTAGE1_l_offset_voltage_s1_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char l_offset_voltage_s2 : 8;
    } reg;
} REG_L_OFFSET_VOLTAGE2_UNION;
#endif
#define REG_L_OFFSET_VOLTAGE2_l_offset_voltage_s2_START (0)
#define REG_L_OFFSET_VOLTAGE2_l_offset_voltage_s2_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char l_v_ocv_data_s0 : 8;
    } reg;
} REG_L_OCV_VOLTAGE0_UNION;
#endif
#define REG_L_OCV_VOLTAGE0_l_v_ocv_data_s0_START (0)
#define REG_L_OCV_VOLTAGE0_l_v_ocv_data_s0_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char l_v_ocv_data_s1 : 8;
    } reg;
} REG_L_OCV_VOLTAGE1_UNION;
#endif
#define REG_L_OCV_VOLTAGE1_l_v_ocv_data_s1_START (0)
#define REG_L_OCV_VOLTAGE1_l_v_ocv_data_s1_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char l_v_ocv_data_s2 : 8;
    } reg;
} REG_L_OCV_VOLTAGE2_UNION;
#endif
#define REG_L_OCV_VOLTAGE2_l_v_ocv_data_s2_START (0)
#define REG_L_OCV_VOLTAGE2_l_v_ocv_data_s2_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char l_i_ocv_data_s0 : 8;
    } reg;
} REG_L_OCV_CURRENT0_UNION;
#endif
#define REG_L_OCV_CURRENT0_l_i_ocv_data_s0_START (0)
#define REG_L_OCV_CURRENT0_l_i_ocv_data_s0_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char l_i_ocv_data_s1 : 8;
    } reg;
} REG_L_OCV_CURRENT1_UNION;
#endif
#define REG_L_OCV_CURRENT1_l_i_ocv_data_s1_START (0)
#define REG_L_OCV_CURRENT1_l_i_ocv_data_s1_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char l_i_ocv_data_s2 : 8;
    } reg;
} REG_L_OCV_CURRENT2_UNION;
#endif
#define REG_L_OCV_CURRENT2_l_i_ocv_data_s2_START (0)
#define REG_L_OCV_CURRENT2_l_i_ocv_data_s2_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char l_eco_out_clin_s0 : 8;
    } reg;
} REG_L_ECO_OUT_CLIN_0_UNION;
#endif
#define REG_L_ECO_OUT_CLIN_0_l_eco_out_clin_s0_START (0)
#define REG_L_ECO_OUT_CLIN_0_l_eco_out_clin_s0_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char l_eco_out_clin_s1 : 8;
    } reg;
} REG_L_ECO_OUT_CLIN_1_UNION;
#endif
#define REG_L_ECO_OUT_CLIN_1_l_eco_out_clin_s1_START (0)
#define REG_L_ECO_OUT_CLIN_1_l_eco_out_clin_s1_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char l_eco_out_clin_s2 : 8;
    } reg;
} REG_L_ECO_OUT_CLIN_2_UNION;
#endif
#define REG_L_ECO_OUT_CLIN_2_l_eco_out_clin_s2_START (0)
#define REG_L_ECO_OUT_CLIN_2_l_eco_out_clin_s2_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char l_eco_out_clin_s3 : 8;
    } reg;
} REG_L_ECO_OUT_CLIN_3_UNION;
#endif
#define REG_L_ECO_OUT_CLIN_3_l_eco_out_clin_s3_START (0)
#define REG_L_ECO_OUT_CLIN_3_l_eco_out_clin_s3_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char l_eco_out_clin_s4 : 8;
    } reg;
} REG_L_ECO_OUT_CLIN_4_UNION;
#endif
#define REG_L_ECO_OUT_CLIN_4_l_eco_out_clin_s4_START (0)
#define REG_L_ECO_OUT_CLIN_4_l_eco_out_clin_s4_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char l_eco_out_clout_s0 : 8;
    } reg;
} REG_L_ECO_OUT_CLOUT_0_UNION;
#endif
#define REG_L_ECO_OUT_CLOUT_0_l_eco_out_clout_s0_START (0)
#define REG_L_ECO_OUT_CLOUT_0_l_eco_out_clout_s0_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char l_eco_out_clout_s1 : 8;
    } reg;
} REG_L_ECO_OUT_CLOUT_1_UNION;
#endif
#define REG_L_ECO_OUT_CLOUT_1_l_eco_out_clout_s1_START (0)
#define REG_L_ECO_OUT_CLOUT_1_l_eco_out_clout_s1_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char l_eco_out_clout_s2 : 8;
    } reg;
} REG_L_ECO_OUT_CLOUT_2_UNION;
#endif
#define REG_L_ECO_OUT_CLOUT_2_l_eco_out_clout_s2_START (0)
#define REG_L_ECO_OUT_CLOUT_2_l_eco_out_clout_s2_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char l_eco_out_clout_s3 : 8;
    } reg;
} REG_L_ECO_OUT_CLOUT_3_UNION;
#endif
#define REG_L_ECO_OUT_CLOUT_3_l_eco_out_clout_s3_START (0)
#define REG_L_ECO_OUT_CLOUT_3_l_eco_out_clout_s3_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char l_eco_out_clout_s4 : 8;
    } reg;
} REG_L_ECO_OUT_CLOUT_4_UNION;
#endif
#define REG_L_ECO_OUT_CLOUT_4_l_eco_out_clout_s4_START (0)
#define REG_L_ECO_OUT_CLOUT_4_l_eco_out_clout_s4_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char l_vout_pre0_s0 : 8;
    } reg;
} REG_L_V_PRE0_OUT0_UNION;
#endif
#define REG_L_V_PRE0_OUT0_l_vout_pre0_s0_START (0)
#define REG_L_V_PRE0_OUT0_l_vout_pre0_s0_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char l_vout_pre0_s1 : 8;
    } reg;
} REG_L_V_PRE0_OUT1_UNION;
#endif
#define REG_L_V_PRE0_OUT1_l_vout_pre0_s1_START (0)
#define REG_L_V_PRE0_OUT1_l_vout_pre0_s1_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char l_vout_pre0_s2 : 8;
    } reg;
} REG_L_V_PRE0_OUT2_UNION;
#endif
#define REG_L_V_PRE0_OUT2_l_vout_pre0_s2_START (0)
#define REG_L_V_PRE0_OUT2_l_vout_pre0_s2_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char l_vout_pre1_s0 : 8;
    } reg;
} REG_L_V_PRE1_OUT0_UNION;
#endif
#define REG_L_V_PRE1_OUT0_l_vout_pre1_s0_START (0)
#define REG_L_V_PRE1_OUT0_l_vout_pre1_s0_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char l_vout_pre1_s1 : 8;
    } reg;
} REG_L_V_PRE1_OUT1_UNION;
#endif
#define REG_L_V_PRE1_OUT1_l_vout_pre1_s1_START (0)
#define REG_L_V_PRE1_OUT1_l_vout_pre1_s1_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char l_vout_pre1_s2 : 8;
    } reg;
} REG_L_V_PRE1_OUT2_UNION;
#endif
#define REG_L_V_PRE1_OUT2_l_vout_pre1_s2_START (0)
#define REG_L_V_PRE1_OUT2_l_vout_pre1_s2_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char l_vout_pre2_s0 : 8;
    } reg;
} REG_L_V_PRE2_OUT0_UNION;
#endif
#define REG_L_V_PRE2_OUT0_l_vout_pre2_s0_START (0)
#define REG_L_V_PRE2_OUT0_l_vout_pre2_s0_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char l_vout_pre2_s1 : 8;
    } reg;
} REG_L_V_PRE2_OUT1_UNION;
#endif
#define REG_L_V_PRE2_OUT1_l_vout_pre2_s1_START (0)
#define REG_L_V_PRE2_OUT1_l_vout_pre2_s1_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char l_vout_pre2_s2 : 8;
    } reg;
} REG_L_V_PRE2_OUT2_UNION;
#endif
#define REG_L_V_PRE2_OUT2_l_vout_pre2_s2_START (0)
#define REG_L_V_PRE2_OUT2_l_vout_pre2_s2_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char l_vout_pre3_s0 : 8;
    } reg;
} REG_L_V_PRE3_OUT0_UNION;
#endif
#define REG_L_V_PRE3_OUT0_l_vout_pre3_s0_START (0)
#define REG_L_V_PRE3_OUT0_l_vout_pre3_s0_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char l_vout_pre3_s1 : 8;
    } reg;
} REG_L_V_PRE3_OUT1_UNION;
#endif
#define REG_L_V_PRE3_OUT1_l_vout_pre3_s1_START (0)
#define REG_L_V_PRE3_OUT1_l_vout_pre3_s1_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char l_vout_pre3_s2 : 8;
    } reg;
} REG_L_V_PRE3_OUT2_UNION;
#endif
#define REG_L_V_PRE3_OUT2_l_vout_pre3_s2_START (0)
#define REG_L_V_PRE3_OUT2_l_vout_pre3_s2_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char l_vout_pre4_s0 : 8;
    } reg;
} REG_L_V_PRE4_OUT0_UNION;
#endif
#define REG_L_V_PRE4_OUT0_l_vout_pre4_s0_START (0)
#define REG_L_V_PRE4_OUT0_l_vout_pre4_s0_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char l_vout_pre4_s1 : 8;
    } reg;
} REG_L_V_PRE4_OUT1_UNION;
#endif
#define REG_L_V_PRE4_OUT1_l_vout_pre4_s1_START (0)
#define REG_L_V_PRE4_OUT1_l_vout_pre4_s1_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char l_vout_pre4_s2 : 8;
    } reg;
} REG_L_V_PRE4_OUT2_UNION;
#endif
#define REG_L_V_PRE4_OUT2_l_vout_pre4_s2_START (0)
#define REG_L_V_PRE4_OUT2_l_vout_pre4_s2_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char l_vout_pre5_s0 : 8;
    } reg;
} REG_L_V_PRE5_OUT0_UNION;
#endif
#define REG_L_V_PRE5_OUT0_l_vout_pre5_s0_START (0)
#define REG_L_V_PRE5_OUT0_l_vout_pre5_s0_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char l_vout_pre5_s1 : 8;
    } reg;
} REG_L_V_PRE5_OUT1_UNION;
#endif
#define REG_L_V_PRE5_OUT1_l_vout_pre5_s1_START (0)
#define REG_L_V_PRE5_OUT1_l_vout_pre5_s1_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char l_vout_pre5_s2 : 8;
    } reg;
} REG_L_V_PRE5_OUT2_UNION;
#endif
#define REG_L_V_PRE5_OUT2_l_vout_pre5_s2_START (0)
#define REG_L_V_PRE5_OUT2_l_vout_pre5_s2_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char l_vout_pre6_s0 : 8;
    } reg;
} REG_L_V_PRE6_OUT0_UNION;
#endif
#define REG_L_V_PRE6_OUT0_l_vout_pre6_s0_START (0)
#define REG_L_V_PRE6_OUT0_l_vout_pre6_s0_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char l_vout_pre6_s1 : 8;
    } reg;
} REG_L_V_PRE6_OUT1_UNION;
#endif
#define REG_L_V_PRE6_OUT1_l_vout_pre6_s1_START (0)
#define REG_L_V_PRE6_OUT1_l_vout_pre6_s1_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char l_vout_pre6_s2 : 8;
    } reg;
} REG_L_V_PRE6_OUT2_UNION;
#endif
#define REG_L_V_PRE6_OUT2_l_vout_pre6_s2_START (0)
#define REG_L_V_PRE6_OUT2_l_vout_pre6_s2_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char l_vout_pre7_s0 : 8;
    } reg;
} REG_L_V_PRE7_OUT0_UNION;
#endif
#define REG_L_V_PRE7_OUT0_l_vout_pre7_s0_START (0)
#define REG_L_V_PRE7_OUT0_l_vout_pre7_s0_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char l_vout_pre7_s1 : 8;
    } reg;
} REG_L_V_PRE7_OUT1_UNION;
#endif
#define REG_L_V_PRE7_OUT1_l_vout_pre7_s1_START (0)
#define REG_L_V_PRE7_OUT1_l_vout_pre7_s1_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char l_vout_pre7_s2 : 8;
    } reg;
} REG_L_V_PRE7_OUT2_UNION;
#endif
#define REG_L_V_PRE7_OUT2_l_vout_pre7_s2_START (0)
#define REG_L_V_PRE7_OUT2_l_vout_pre7_s2_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char l_current_pre0_s0 : 8;
    } reg;
} REG_L_CURRENT_PRE0_OUT0_UNION;
#endif
#define REG_L_CURRENT_PRE0_OUT0_l_current_pre0_s0_START (0)
#define REG_L_CURRENT_PRE0_OUT0_l_current_pre0_s0_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char l_current_pre0_s1 : 8;
    } reg;
} REG_L_CURRENT_PRE0_OUT1_UNION;
#endif
#define REG_L_CURRENT_PRE0_OUT1_l_current_pre0_s1_START (0)
#define REG_L_CURRENT_PRE0_OUT1_l_current_pre0_s1_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char l_current_pre0_s2 : 8;
    } reg;
} REG_L_CURRENT_PRE0_OUT2_UNION;
#endif
#define REG_L_CURRENT_PRE0_OUT2_l_current_pre0_s2_START (0)
#define REG_L_CURRENT_PRE0_OUT2_l_current_pre0_s2_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char l_current_pre1_s0 : 8;
    } reg;
} REG_L_CURRENT_PRE1_OUT0_UNION;
#endif
#define REG_L_CURRENT_PRE1_OUT0_l_current_pre1_s0_START (0)
#define REG_L_CURRENT_PRE1_OUT0_l_current_pre1_s0_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char l_current_pre1_s1 : 8;
    } reg;
} REG_L_CURRENT_PRE1_OUT1_UNION;
#endif
#define REG_L_CURRENT_PRE1_OUT1_l_current_pre1_s1_START (0)
#define REG_L_CURRENT_PRE1_OUT1_l_current_pre1_s1_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char l_current_pre1_s2 : 8;
    } reg;
} REG_L_CURRENT_PRE1_OUT2_UNION;
#endif
#define REG_L_CURRENT_PRE1_OUT2_l_current_pre1_s2_START (0)
#define REG_L_CURRENT_PRE1_OUT2_l_current_pre1_s2_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char l_current_pre2_s0 : 8;
    } reg;
} REG_L_CURRENT_PRE2_OUT0_UNION;
#endif
#define REG_L_CURRENT_PRE2_OUT0_l_current_pre2_s0_START (0)
#define REG_L_CURRENT_PRE2_OUT0_l_current_pre2_s0_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char l_current_pre2_s1 : 8;
    } reg;
} REG_L_CURRENT_PRE2_OUT1_UNION;
#endif
#define REG_L_CURRENT_PRE2_OUT1_l_current_pre2_s1_START (0)
#define REG_L_CURRENT_PRE2_OUT1_l_current_pre2_s1_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char l_current_pre2_s2 : 8;
    } reg;
} REG_L_CURRENT_PRE2_OUT2_UNION;
#endif
#define REG_L_CURRENT_PRE2_OUT2_l_current_pre2_s2_START (0)
#define REG_L_CURRENT_PRE2_OUT2_l_current_pre2_s2_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char l_current_pre3_s0 : 8;
    } reg;
} REG_L_CURRENT_PRE3_OUT0_UNION;
#endif
#define REG_L_CURRENT_PRE3_OUT0_l_current_pre3_s0_START (0)
#define REG_L_CURRENT_PRE3_OUT0_l_current_pre3_s0_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char l_current_pre3_s1 : 8;
    } reg;
} REG_L_CURRENT_PRE3_OUT1_UNION;
#endif
#define REG_L_CURRENT_PRE3_OUT1_l_current_pre3_s1_START (0)
#define REG_L_CURRENT_PRE3_OUT1_l_current_pre3_s1_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char l_current_pre3_s2 : 8;
    } reg;
} REG_L_CURRENT_PRE3_OUT2_UNION;
#endif
#define REG_L_CURRENT_PRE3_OUT2_l_current_pre3_s2_START (0)
#define REG_L_CURRENT_PRE3_OUT2_l_current_pre3_s2_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char l_current_pre4_s0 : 8;
    } reg;
} REG_L_CURRENT_PRE4_OUT0_UNION;
#endif
#define REG_L_CURRENT_PRE4_OUT0_l_current_pre4_s0_START (0)
#define REG_L_CURRENT_PRE4_OUT0_l_current_pre4_s0_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char l_current_pre4_s1 : 8;
    } reg;
} REG_L_CURRENT_PRE4_OUT1_UNION;
#endif
#define REG_L_CURRENT_PRE4_OUT1_l_current_pre4_s1_START (0)
#define REG_L_CURRENT_PRE4_OUT1_l_current_pre4_s1_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char l_current_pre4_s2 : 8;
    } reg;
} REG_L_CURRENT_PRE4_OUT2_UNION;
#endif
#define REG_L_CURRENT_PRE4_OUT2_l_current_pre4_s2_START (0)
#define REG_L_CURRENT_PRE4_OUT2_l_current_pre4_s2_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char l_current_pre5_s0 : 8;
    } reg;
} REG_L_CURRENT_PRE5_OUT0_UNION;
#endif
#define REG_L_CURRENT_PRE5_OUT0_l_current_pre5_s0_START (0)
#define REG_L_CURRENT_PRE5_OUT0_l_current_pre5_s0_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char l_current_pre5_s1 : 8;
    } reg;
} REG_L_CURRENT_PRE5_OUT1_UNION;
#endif
#define REG_L_CURRENT_PRE5_OUT1_l_current_pre5_s1_START (0)
#define REG_L_CURRENT_PRE5_OUT1_l_current_pre5_s1_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char l_current_pre5_s2 : 8;
    } reg;
} REG_L_CURRENT_PRE5_OUT2_UNION;
#endif
#define REG_L_CURRENT_PRE5_OUT2_l_current_pre5_s2_START (0)
#define REG_L_CURRENT_PRE5_OUT2_l_current_pre5_s2_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char l_current_pre6_s0 : 8;
    } reg;
} REG_L_CURRENT_PRE6_OUT0_UNION;
#endif
#define REG_L_CURRENT_PRE6_OUT0_l_current_pre6_s0_START (0)
#define REG_L_CURRENT_PRE6_OUT0_l_current_pre6_s0_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char l_current_pre6_s1 : 8;
    } reg;
} REG_L_CURRENT_PRE6_OUT1_UNION;
#endif
#define REG_L_CURRENT_PRE6_OUT1_l_current_pre6_s1_START (0)
#define REG_L_CURRENT_PRE6_OUT1_l_current_pre6_s1_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char l_current_pre6_s2 : 8;
    } reg;
} REG_L_CURRENT_PRE6_OUT2_UNION;
#endif
#define REG_L_CURRENT_PRE6_OUT2_l_current_pre6_s2_START (0)
#define REG_L_CURRENT_PRE6_OUT2_l_current_pre6_s2_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char l_current_pre7_s0 : 8;
    } reg;
} REG_L_CURRENT_PRE7_OUT0_UNION;
#endif
#define REG_L_CURRENT_PRE7_OUT0_l_current_pre7_s0_START (0)
#define REG_L_CURRENT_PRE7_OUT0_l_current_pre7_s0_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char l_current_pre7_s1 : 8;
    } reg;
} REG_L_CURRENT_PRE7_OUT1_UNION;
#endif
#define REG_L_CURRENT_PRE7_OUT1_l_current_pre7_s1_START (0)
#define REG_L_CURRENT_PRE7_OUT1_l_current_pre7_s1_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char l_current_pre7_s2 : 8;
    } reg;
} REG_L_CURRENT_PRE7_OUT2_UNION;
#endif
#define REG_L_CURRENT_PRE7_OUT2_l_current_pre7_s2_START (0)
#define REG_L_CURRENT_PRE7_OUT2_l_current_pre7_s2_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char l_v_ocv_out_pre1_s0 : 8;
    } reg;
} REG_L_V_OCV_PRE1_OUT0_UNION;
#endif
#define REG_L_V_OCV_PRE1_OUT0_l_v_ocv_out_pre1_s0_START (0)
#define REG_L_V_OCV_PRE1_OUT0_l_v_ocv_out_pre1_s0_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char l_v_ocv_out_pre1_s1 : 8;
    } reg;
} REG_L_V_OCV_PRE1_OUT1_UNION;
#endif
#define REG_L_V_OCV_PRE1_OUT1_l_v_ocv_out_pre1_s1_START (0)
#define REG_L_V_OCV_PRE1_OUT1_l_v_ocv_out_pre1_s1_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char l_v_ocv_out_pre1_s2 : 8;
    } reg;
} REG_L_V_OCV_PRE1_OUT2_UNION;
#endif
#define REG_L_V_OCV_PRE1_OUT2_l_v_ocv_out_pre1_s2_START (0)
#define REG_L_V_OCV_PRE1_OUT2_l_v_ocv_out_pre1_s2_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char l_v_ocv_out_pre2_s0 : 8;
    } reg;
} REG_L_V_OCV_PRE2_OUT0_UNION;
#endif
#define REG_L_V_OCV_PRE2_OUT0_l_v_ocv_out_pre2_s0_START (0)
#define REG_L_V_OCV_PRE2_OUT0_l_v_ocv_out_pre2_s0_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char l_v_ocv_out_pre2_s1 : 8;
    } reg;
} REG_L_V_OCV_PRE2_OUT1_UNION;
#endif
#define REG_L_V_OCV_PRE2_OUT1_l_v_ocv_out_pre2_s1_START (0)
#define REG_L_V_OCV_PRE2_OUT1_l_v_ocv_out_pre2_s1_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char l_v_ocv_out_pre2_s2 : 8;
    } reg;
} REG_L_V_OCV_PRE2_OUT2_UNION;
#endif
#define REG_L_V_OCV_PRE2_OUT2_l_v_ocv_out_pre2_s2_START (0)
#define REG_L_V_OCV_PRE2_OUT2_l_v_ocv_out_pre2_s2_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char l_v_ocv_out_pre3_s0 : 8;
    } reg;
} REG_L_V_OCV_PRE3_OUT0_UNION;
#endif
#define REG_L_V_OCV_PRE3_OUT0_l_v_ocv_out_pre3_s0_START (0)
#define REG_L_V_OCV_PRE3_OUT0_l_v_ocv_out_pre3_s0_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char l_v_ocv_out_pre3_s1 : 8;
    } reg;
} REG_L_V_OCV_PRE3_OUT1_UNION;
#endif
#define REG_L_V_OCV_PRE3_OUT1_l_v_ocv_out_pre3_s1_START (0)
#define REG_L_V_OCV_PRE3_OUT1_l_v_ocv_out_pre3_s1_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char l_v_ocv_out_pre3_s2 : 8;
    } reg;
} REG_L_V_OCV_PRE3_OUT2_UNION;
#endif
#define REG_L_V_OCV_PRE3_OUT2_l_v_ocv_out_pre3_s2_START (0)
#define REG_L_V_OCV_PRE3_OUT2_l_v_ocv_out_pre3_s2_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char l_v_ocv_out_pre4_s0 : 8;
    } reg;
} REG_L_V_OCV_PRE4_OUT0_UNION;
#endif
#define REG_L_V_OCV_PRE4_OUT0_l_v_ocv_out_pre4_s0_START (0)
#define REG_L_V_OCV_PRE4_OUT0_l_v_ocv_out_pre4_s0_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char l_v_ocv_out_pre4_s1 : 8;
    } reg;
} REG_L_V_OCV_PRE4_OUT1_UNION;
#endif
#define REG_L_V_OCV_PRE4_OUT1_l_v_ocv_out_pre4_s1_START (0)
#define REG_L_V_OCV_PRE4_OUT1_l_v_ocv_out_pre4_s1_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char l_v_ocv_out_pre4_s2 : 8;
    } reg;
} REG_L_V_OCV_PRE4_OUT2_UNION;
#endif
#define REG_L_V_OCV_PRE4_OUT2_l_v_ocv_out_pre4_s2_START (0)
#define REG_L_V_OCV_PRE4_OUT2_l_v_ocv_out_pre4_s2_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char l_i_ocv_out_pre1_s0 : 8;
    } reg;
} REG_L_I_OCV_PRE1_OUT0_UNION;
#endif
#define REG_L_I_OCV_PRE1_OUT0_l_i_ocv_out_pre1_s0_START (0)
#define REG_L_I_OCV_PRE1_OUT0_l_i_ocv_out_pre1_s0_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char l_i_ocv_out_pre1_s1 : 8;
    } reg;
} REG_L_I_OCV_PRE1_OUT1_UNION;
#endif
#define REG_L_I_OCV_PRE1_OUT1_l_i_ocv_out_pre1_s1_START (0)
#define REG_L_I_OCV_PRE1_OUT1_l_i_ocv_out_pre1_s1_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char l_i_ocv_out_pre1_s2 : 8;
    } reg;
} REG_L_I_OCV_PRE1_OUT2_UNION;
#endif
#define REG_L_I_OCV_PRE1_OUT2_l_i_ocv_out_pre1_s2_START (0)
#define REG_L_I_OCV_PRE1_OUT2_l_i_ocv_out_pre1_s2_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char l_i_ocv_out_pre2_s0 : 8;
    } reg;
} REG_L_I_OCV_PRE2_OUT0_UNION;
#endif
#define REG_L_I_OCV_PRE2_OUT0_l_i_ocv_out_pre2_s0_START (0)
#define REG_L_I_OCV_PRE2_OUT0_l_i_ocv_out_pre2_s0_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char l_i_ocv_out_pre2_s1 : 8;
    } reg;
} REG_L_I_OCV_PRE2_OUT1_UNION;
#endif
#define REG_L_I_OCV_PRE2_OUT1_l_i_ocv_out_pre2_s1_START (0)
#define REG_L_I_OCV_PRE2_OUT1_l_i_ocv_out_pre2_s1_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char l_i_ocv_out_pre2_s2 : 8;
    } reg;
} REG_L_I_OCV_PRE2_OUT2_UNION;
#endif
#define REG_L_I_OCV_PRE2_OUT2_l_i_ocv_out_pre2_s2_START (0)
#define REG_L_I_OCV_PRE2_OUT2_l_i_ocv_out_pre2_s2_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char l_i_ocv_out_pre3_s0 : 8;
    } reg;
} REG_L_I_OCV_PRE3_OUT0_UNION;
#endif
#define REG_L_I_OCV_PRE3_OUT0_l_i_ocv_out_pre3_s0_START (0)
#define REG_L_I_OCV_PRE3_OUT0_l_i_ocv_out_pre3_s0_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char l_i_ocv_out_pre3_s1 : 8;
    } reg;
} REG_L_I_OCV_PRE3_OUT1_UNION;
#endif
#define REG_L_I_OCV_PRE3_OUT1_l_i_ocv_out_pre3_s1_START (0)
#define REG_L_I_OCV_PRE3_OUT1_l_i_ocv_out_pre3_s1_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char l_i_ocv_out_pre3_s2 : 8;
    } reg;
} REG_L_I_OCV_PRE3_OUT2_UNION;
#endif
#define REG_L_I_OCV_PRE3_OUT2_l_i_ocv_out_pre3_s2_START (0)
#define REG_L_I_OCV_PRE3_OUT2_l_i_ocv_out_pre3_s2_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char l_i_ocv_out_pre4_s0 : 8;
    } reg;
} REG_L_I_OCV_PRE4_OUT0_UNION;
#endif
#define REG_L_I_OCV_PRE4_OUT0_l_i_ocv_out_pre4_s0_START (0)
#define REG_L_I_OCV_PRE4_OUT0_l_i_ocv_out_pre4_s0_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char l_i_ocv_out_pre4_s1 : 8;
    } reg;
} REG_L_I_OCV_PRE4_OUT1_UNION;
#endif
#define REG_L_I_OCV_PRE4_OUT1_l_i_ocv_out_pre4_s1_START (0)
#define REG_L_I_OCV_PRE4_OUT1_l_i_ocv_out_pre4_s1_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char l_i_ocv_out_pre4_s2 : 8;
    } reg;
} REG_L_I_OCV_PRE4_OUT2_UNION;
#endif
#define REG_L_I_OCV_PRE4_OUT2_l_i_ocv_out_pre4_s2_START (0)
#define REG_L_I_OCV_PRE4_OUT2_l_i_ocv_out_pre4_s2_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char sc_irq_flag_0 : 1;
        unsigned char sc_irq_flag_1 : 1;
        unsigned char sc_irq_flag_2 : 1;
        unsigned char sc_irq_flag_3 : 1;
        unsigned char sc_irq_flag_4 : 1;
        unsigned char sc_irq_flag_5 : 1;
        unsigned char sc_irq_flag_6 : 1;
        unsigned char sc_irq_flag_7 : 1;
    } reg;
} REG_IRQ_FLAG_UNION;
#endif
#define REG_IRQ_FLAG_sc_irq_flag_0_START (0)
#define REG_IRQ_FLAG_sc_irq_flag_0_END (0)
#define REG_IRQ_FLAG_sc_irq_flag_1_START (1)
#define REG_IRQ_FLAG_sc_irq_flag_1_END (1)
#define REG_IRQ_FLAG_sc_irq_flag_2_START (2)
#define REG_IRQ_FLAG_sc_irq_flag_2_END (2)
#define REG_IRQ_FLAG_sc_irq_flag_3_START (3)
#define REG_IRQ_FLAG_sc_irq_flag_3_END (3)
#define REG_IRQ_FLAG_sc_irq_flag_4_START (4)
#define REG_IRQ_FLAG_sc_irq_flag_4_END (4)
#define REG_IRQ_FLAG_sc_irq_flag_5_START (5)
#define REG_IRQ_FLAG_sc_irq_flag_5_END (5)
#define REG_IRQ_FLAG_sc_irq_flag_6_START (6)
#define REG_IRQ_FLAG_sc_irq_flag_6_END (6)
#define REG_IRQ_FLAG_sc_irq_flag_7_START (7)
#define REG_IRQ_FLAG_sc_irq_flag_7_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char sc_irq_flag_0_0 : 1;
        unsigned char sc_irq_flag_0_1 : 1;
        unsigned char sc_irq_flag_0_2 : 1;
        unsigned char sc_irq_flag_0_3 : 1;
        unsigned char sc_irq_flag_0_4 : 1;
        unsigned char sc_irq_flag_0_5 : 1;
        unsigned char sc_irq_flag_0_6 : 1;
        unsigned char sc_irq_flag_0_7 : 1;
    } reg;
} REG_IRQ_FLAG_0_UNION;
#endif
#define REG_IRQ_FLAG_0_sc_irq_flag_0_0_START (0)
#define REG_IRQ_FLAG_0_sc_irq_flag_0_0_END (0)
#define REG_IRQ_FLAG_0_sc_irq_flag_0_1_START (1)
#define REG_IRQ_FLAG_0_sc_irq_flag_0_1_END (1)
#define REG_IRQ_FLAG_0_sc_irq_flag_0_2_START (2)
#define REG_IRQ_FLAG_0_sc_irq_flag_0_2_END (2)
#define REG_IRQ_FLAG_0_sc_irq_flag_0_3_START (3)
#define REG_IRQ_FLAG_0_sc_irq_flag_0_3_END (3)
#define REG_IRQ_FLAG_0_sc_irq_flag_0_4_START (4)
#define REG_IRQ_FLAG_0_sc_irq_flag_0_4_END (4)
#define REG_IRQ_FLAG_0_sc_irq_flag_0_5_START (5)
#define REG_IRQ_FLAG_0_sc_irq_flag_0_5_END (5)
#define REG_IRQ_FLAG_0_sc_irq_flag_0_6_START (6)
#define REG_IRQ_FLAG_0_sc_irq_flag_0_6_END (6)
#define REG_IRQ_FLAG_0_sc_irq_flag_0_7_START (7)
#define REG_IRQ_FLAG_0_sc_irq_flag_0_7_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char sc_irq_flag_1_0 : 1;
        unsigned char sc_irq_flag_1_1 : 1;
        unsigned char sc_irq_flag_1_2 : 1;
        unsigned char sc_irq_flag_1_3 : 1;
        unsigned char sc_irq_flag_1_4 : 1;
        unsigned char sc_irq_flag_1_5 : 1;
        unsigned char sc_irq_flag_1_6 : 1;
        unsigned char sc_irq_flag_1_7 : 1;
    } reg;
} REG_IRQ_FLAG_1_UNION;
#endif
#define REG_IRQ_FLAG_1_sc_irq_flag_1_0_START (0)
#define REG_IRQ_FLAG_1_sc_irq_flag_1_0_END (0)
#define REG_IRQ_FLAG_1_sc_irq_flag_1_1_START (1)
#define REG_IRQ_FLAG_1_sc_irq_flag_1_1_END (1)
#define REG_IRQ_FLAG_1_sc_irq_flag_1_2_START (2)
#define REG_IRQ_FLAG_1_sc_irq_flag_1_2_END (2)
#define REG_IRQ_FLAG_1_sc_irq_flag_1_3_START (3)
#define REG_IRQ_FLAG_1_sc_irq_flag_1_3_END (3)
#define REG_IRQ_FLAG_1_sc_irq_flag_1_4_START (4)
#define REG_IRQ_FLAG_1_sc_irq_flag_1_4_END (4)
#define REG_IRQ_FLAG_1_sc_irq_flag_1_5_START (5)
#define REG_IRQ_FLAG_1_sc_irq_flag_1_5_END (5)
#define REG_IRQ_FLAG_1_sc_irq_flag_1_6_START (6)
#define REG_IRQ_FLAG_1_sc_irq_flag_1_6_END (6)
#define REG_IRQ_FLAG_1_sc_irq_flag_1_7_START (7)
#define REG_IRQ_FLAG_1_sc_irq_flag_1_7_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char sc_irq_flag_2_0 : 1;
        unsigned char sc_irq_flag_2_1 : 1;
        unsigned char sc_irq_flag_2_2 : 1;
        unsigned char sc_irq_flag_2_3 : 1;
        unsigned char sc_irq_flag_2_4 : 1;
        unsigned char sc_irq_flag_2_5 : 1;
        unsigned char sc_irq_flag_2_6 : 1;
        unsigned char sc_irq_flag_2_7 : 1;
    } reg;
} REG_IRQ_FLAG_2_UNION;
#endif
#define REG_IRQ_FLAG_2_sc_irq_flag_2_0_START (0)
#define REG_IRQ_FLAG_2_sc_irq_flag_2_0_END (0)
#define REG_IRQ_FLAG_2_sc_irq_flag_2_1_START (1)
#define REG_IRQ_FLAG_2_sc_irq_flag_2_1_END (1)
#define REG_IRQ_FLAG_2_sc_irq_flag_2_2_START (2)
#define REG_IRQ_FLAG_2_sc_irq_flag_2_2_END (2)
#define REG_IRQ_FLAG_2_sc_irq_flag_2_3_START (3)
#define REG_IRQ_FLAG_2_sc_irq_flag_2_3_END (3)
#define REG_IRQ_FLAG_2_sc_irq_flag_2_4_START (4)
#define REG_IRQ_FLAG_2_sc_irq_flag_2_4_END (4)
#define REG_IRQ_FLAG_2_sc_irq_flag_2_5_START (5)
#define REG_IRQ_FLAG_2_sc_irq_flag_2_5_END (5)
#define REG_IRQ_FLAG_2_sc_irq_flag_2_6_START (6)
#define REG_IRQ_FLAG_2_sc_irq_flag_2_6_END (6)
#define REG_IRQ_FLAG_2_sc_irq_flag_2_7_START (7)
#define REG_IRQ_FLAG_2_sc_irq_flag_2_7_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char sc_irq_flag_3_0 : 1;
        unsigned char sc_irq_flag_3_1 : 1;
        unsigned char sc_irq_flag_3_2 : 1;
        unsigned char sc_irq_flag_3_3 : 1;
        unsigned char sc_irq_flag_3_4 : 1;
        unsigned char sc_irq_flag_3_5 : 1;
        unsigned char sc_irq_flag_3_6 : 1;
        unsigned char sc_irq_flag_3_7 : 1;
    } reg;
} REG_IRQ_FLAG_3_UNION;
#endif
#define REG_IRQ_FLAG_3_sc_irq_flag_3_0_START (0)
#define REG_IRQ_FLAG_3_sc_irq_flag_3_0_END (0)
#define REG_IRQ_FLAG_3_sc_irq_flag_3_1_START (1)
#define REG_IRQ_FLAG_3_sc_irq_flag_3_1_END (1)
#define REG_IRQ_FLAG_3_sc_irq_flag_3_2_START (2)
#define REG_IRQ_FLAG_3_sc_irq_flag_3_2_END (2)
#define REG_IRQ_FLAG_3_sc_irq_flag_3_3_START (3)
#define REG_IRQ_FLAG_3_sc_irq_flag_3_3_END (3)
#define REG_IRQ_FLAG_3_sc_irq_flag_3_4_START (4)
#define REG_IRQ_FLAG_3_sc_irq_flag_3_4_END (4)
#define REG_IRQ_FLAG_3_sc_irq_flag_3_5_START (5)
#define REG_IRQ_FLAG_3_sc_irq_flag_3_5_END (5)
#define REG_IRQ_FLAG_3_sc_irq_flag_3_6_START (6)
#define REG_IRQ_FLAG_3_sc_irq_flag_3_6_END (6)
#define REG_IRQ_FLAG_3_sc_irq_flag_3_7_START (7)
#define REG_IRQ_FLAG_3_sc_irq_flag_3_7_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char sc_irq_flag_4_0 : 1;
        unsigned char sc_irq_flag_4_1 : 1;
        unsigned char sc_irq_flag_4_2 : 1;
        unsigned char sc_irq_flag_4_3 : 1;
        unsigned char sc_irq_flag_4_4 : 1;
        unsigned char sc_irq_flag_4_5 : 1;
        unsigned char sc_irq_flag_4_6 : 1;
        unsigned char sc_irq_flag_4_7 : 1;
    } reg;
} REG_IRQ_FLAG_4_UNION;
#endif
#define REG_IRQ_FLAG_4_sc_irq_flag_4_0_START (0)
#define REG_IRQ_FLAG_4_sc_irq_flag_4_0_END (0)
#define REG_IRQ_FLAG_4_sc_irq_flag_4_1_START (1)
#define REG_IRQ_FLAG_4_sc_irq_flag_4_1_END (1)
#define REG_IRQ_FLAG_4_sc_irq_flag_4_2_START (2)
#define REG_IRQ_FLAG_4_sc_irq_flag_4_2_END (2)
#define REG_IRQ_FLAG_4_sc_irq_flag_4_3_START (3)
#define REG_IRQ_FLAG_4_sc_irq_flag_4_3_END (3)
#define REG_IRQ_FLAG_4_sc_irq_flag_4_4_START (4)
#define REG_IRQ_FLAG_4_sc_irq_flag_4_4_END (4)
#define REG_IRQ_FLAG_4_sc_irq_flag_4_5_START (5)
#define REG_IRQ_FLAG_4_sc_irq_flag_4_5_END (5)
#define REG_IRQ_FLAG_4_sc_irq_flag_4_6_START (6)
#define REG_IRQ_FLAG_4_sc_irq_flag_4_6_END (6)
#define REG_IRQ_FLAG_4_sc_irq_flag_4_7_START (7)
#define REG_IRQ_FLAG_4_sc_irq_flag_4_7_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char sc_irq_flag_5_0 : 1;
        unsigned char sc_irq_flag_5_1 : 1;
        unsigned char sc_irq_flag_5_2 : 1;
        unsigned char sc_irq_flag_5_3 : 1;
        unsigned char sc_irq_flag_5_4 : 1;
        unsigned char sc_irq_flag_5_5 : 1;
        unsigned char sc_irq_flag_5_6 : 1;
        unsigned char sc_irq_flag_5_7 : 1;
    } reg;
} REG_IRQ_FLAG_5_UNION;
#endif
#define REG_IRQ_FLAG_5_sc_irq_flag_5_0_START (0)
#define REG_IRQ_FLAG_5_sc_irq_flag_5_0_END (0)
#define REG_IRQ_FLAG_5_sc_irq_flag_5_1_START (1)
#define REG_IRQ_FLAG_5_sc_irq_flag_5_1_END (1)
#define REG_IRQ_FLAG_5_sc_irq_flag_5_2_START (2)
#define REG_IRQ_FLAG_5_sc_irq_flag_5_2_END (2)
#define REG_IRQ_FLAG_5_sc_irq_flag_5_3_START (3)
#define REG_IRQ_FLAG_5_sc_irq_flag_5_3_END (3)
#define REG_IRQ_FLAG_5_sc_irq_flag_5_4_START (4)
#define REG_IRQ_FLAG_5_sc_irq_flag_5_4_END (4)
#define REG_IRQ_FLAG_5_sc_irq_flag_5_5_START (5)
#define REG_IRQ_FLAG_5_sc_irq_flag_5_5_END (5)
#define REG_IRQ_FLAG_5_sc_irq_flag_5_6_START (6)
#define REG_IRQ_FLAG_5_sc_irq_flag_5_6_END (6)
#define REG_IRQ_FLAG_5_sc_irq_flag_5_7_START (7)
#define REG_IRQ_FLAG_5_sc_irq_flag_5_7_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char sc_irq_flag_6_0 : 1;
        unsigned char sc_irq_flag_6_1 : 1;
        unsigned char sc_irq_flag_6_2 : 1;
        unsigned char sc_irq_flag_6_3 : 1;
        unsigned char sc_irq_flag_6_4 : 1;
        unsigned char sc_irq_flag_6_5 : 1;
        unsigned char sc_irq_flag_6_6 : 1;
        unsigned char sc_irq_flag_6_7 : 1;
    } reg;
} REG_IRQ_FLAG_6_UNION;
#endif
#define REG_IRQ_FLAG_6_sc_irq_flag_6_0_START (0)
#define REG_IRQ_FLAG_6_sc_irq_flag_6_0_END (0)
#define REG_IRQ_FLAG_6_sc_irq_flag_6_1_START (1)
#define REG_IRQ_FLAG_6_sc_irq_flag_6_1_END (1)
#define REG_IRQ_FLAG_6_sc_irq_flag_6_2_START (2)
#define REG_IRQ_FLAG_6_sc_irq_flag_6_2_END (2)
#define REG_IRQ_FLAG_6_sc_irq_flag_6_3_START (3)
#define REG_IRQ_FLAG_6_sc_irq_flag_6_3_END (3)
#define REG_IRQ_FLAG_6_sc_irq_flag_6_4_START (4)
#define REG_IRQ_FLAG_6_sc_irq_flag_6_4_END (4)
#define REG_IRQ_FLAG_6_sc_irq_flag_6_5_START (5)
#define REG_IRQ_FLAG_6_sc_irq_flag_6_5_END (5)
#define REG_IRQ_FLAG_6_sc_irq_flag_6_6_START (6)
#define REG_IRQ_FLAG_6_sc_irq_flag_6_6_END (6)
#define REG_IRQ_FLAG_6_sc_irq_flag_6_7_START (7)
#define REG_IRQ_FLAG_6_sc_irq_flag_6_7_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char sc_irq_flag_7_0 : 1;
        unsigned char sc_irq_flag_7_1 : 1;
        unsigned char sc_irq_flag_7_2 : 1;
        unsigned char sc_irq_flag_7_3 : 1;
        unsigned char sc_irq_flag_7_4 : 1;
        unsigned char sc_irq_flag_7_5 : 1;
        unsigned char sc_irq_flag_7_6 : 1;
        unsigned char sc_irq_flag_7_7 : 1;
    } reg;
} REG_IRQ_FLAG_7_UNION;
#endif
#define REG_IRQ_FLAG_7_sc_irq_flag_7_0_START (0)
#define REG_IRQ_FLAG_7_sc_irq_flag_7_0_END (0)
#define REG_IRQ_FLAG_7_sc_irq_flag_7_1_START (1)
#define REG_IRQ_FLAG_7_sc_irq_flag_7_1_END (1)
#define REG_IRQ_FLAG_7_sc_irq_flag_7_2_START (2)
#define REG_IRQ_FLAG_7_sc_irq_flag_7_2_END (2)
#define REG_IRQ_FLAG_7_sc_irq_flag_7_3_START (3)
#define REG_IRQ_FLAG_7_sc_irq_flag_7_3_END (3)
#define REG_IRQ_FLAG_7_sc_irq_flag_7_4_START (4)
#define REG_IRQ_FLAG_7_sc_irq_flag_7_4_END (4)
#define REG_IRQ_FLAG_7_sc_irq_flag_7_5_START (5)
#define REG_IRQ_FLAG_7_sc_irq_flag_7_5_END (5)
#define REG_IRQ_FLAG_7_sc_irq_flag_7_6_START (6)
#define REG_IRQ_FLAG_7_sc_irq_flag_7_6_END (6)
#define REG_IRQ_FLAG_7_sc_irq_flag_7_7_START (7)
#define REG_IRQ_FLAG_7_sc_irq_flag_7_7_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char sc_irq_flag_8_0 : 1;
        unsigned char sc_irq_flag_8_1 : 1;
        unsigned char sc_irq_flag_8_2 : 1;
        unsigned char sc_irq_flag_8_3 : 1;
        unsigned char sc_irq_flag_8_4 : 1;
        unsigned char sc_irq_flag_8_5 : 1;
        unsigned char sc_irq_flag_8_6 : 1;
        unsigned char sc_irq_flag_8_7 : 1;
    } reg;
} REG_IRQ_FLAG_8_UNION;
#endif
#define REG_IRQ_FLAG_8_sc_irq_flag_8_0_START (0)
#define REG_IRQ_FLAG_8_sc_irq_flag_8_0_END (0)
#define REG_IRQ_FLAG_8_sc_irq_flag_8_1_START (1)
#define REG_IRQ_FLAG_8_sc_irq_flag_8_1_END (1)
#define REG_IRQ_FLAG_8_sc_irq_flag_8_2_START (2)
#define REG_IRQ_FLAG_8_sc_irq_flag_8_2_END (2)
#define REG_IRQ_FLAG_8_sc_irq_flag_8_3_START (3)
#define REG_IRQ_FLAG_8_sc_irq_flag_8_3_END (3)
#define REG_IRQ_FLAG_8_sc_irq_flag_8_4_START (4)
#define REG_IRQ_FLAG_8_sc_irq_flag_8_4_END (4)
#define REG_IRQ_FLAG_8_sc_irq_flag_8_5_START (5)
#define REG_IRQ_FLAG_8_sc_irq_flag_8_5_END (5)
#define REG_IRQ_FLAG_8_sc_irq_flag_8_6_START (6)
#define REG_IRQ_FLAG_8_sc_irq_flag_8_6_END (6)
#define REG_IRQ_FLAG_8_sc_irq_flag_8_7_START (7)
#define REG_IRQ_FLAG_8_sc_irq_flag_8_7_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char sc_irq_flag_9_0 : 1;
        unsigned char sc_irq_flag_9_1 : 1;
        unsigned char sc_irq_flag_9_2 : 1;
        unsigned char sc_irq_flag_9_3 : 1;
        unsigned char sc_irq_flag_9_4 : 1;
        unsigned char sc_irq_flag_9_5 : 1;
        unsigned char sc_irq_flag_9_6 : 1;
        unsigned char sc_irq_flag_9_7 : 1;
    } reg;
} REG_IRQ_FLAG_9_UNION;
#endif
#define REG_IRQ_FLAG_9_sc_irq_flag_9_0_START (0)
#define REG_IRQ_FLAG_9_sc_irq_flag_9_0_END (0)
#define REG_IRQ_FLAG_9_sc_irq_flag_9_1_START (1)
#define REG_IRQ_FLAG_9_sc_irq_flag_9_1_END (1)
#define REG_IRQ_FLAG_9_sc_irq_flag_9_2_START (2)
#define REG_IRQ_FLAG_9_sc_irq_flag_9_2_END (2)
#define REG_IRQ_FLAG_9_sc_irq_flag_9_3_START (3)
#define REG_IRQ_FLAG_9_sc_irq_flag_9_3_END (3)
#define REG_IRQ_FLAG_9_sc_irq_flag_9_4_START (4)
#define REG_IRQ_FLAG_9_sc_irq_flag_9_4_END (4)
#define REG_IRQ_FLAG_9_sc_irq_flag_9_5_START (5)
#define REG_IRQ_FLAG_9_sc_irq_flag_9_5_END (5)
#define REG_IRQ_FLAG_9_sc_irq_flag_9_6_START (6)
#define REG_IRQ_FLAG_9_sc_irq_flag_9_6_END (6)
#define REG_IRQ_FLAG_9_sc_irq_flag_9_7_START (7)
#define REG_IRQ_FLAG_9_sc_irq_flag_9_7_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char sc_irq_flag_10_0 : 1;
        unsigned char sc_irq_flag_10_1 : 1;
        unsigned char sc_irq_flag_10_2 : 1;
        unsigned char sc_irq_flag_10_3 : 1;
        unsigned char sc_irq_flag_10_4 : 1;
        unsigned char sc_irq_flag_10_5 : 1;
        unsigned char sc_irq_flag_10_6 : 1;
        unsigned char sc_irq_flag_10_7 : 1;
    } reg;
} REG_IRQ_FLAG_10_UNION;
#endif
#define REG_IRQ_FLAG_10_sc_irq_flag_10_0_START (0)
#define REG_IRQ_FLAG_10_sc_irq_flag_10_0_END (0)
#define REG_IRQ_FLAG_10_sc_irq_flag_10_1_START (1)
#define REG_IRQ_FLAG_10_sc_irq_flag_10_1_END (1)
#define REG_IRQ_FLAG_10_sc_irq_flag_10_2_START (2)
#define REG_IRQ_FLAG_10_sc_irq_flag_10_2_END (2)
#define REG_IRQ_FLAG_10_sc_irq_flag_10_3_START (3)
#define REG_IRQ_FLAG_10_sc_irq_flag_10_3_END (3)
#define REG_IRQ_FLAG_10_sc_irq_flag_10_4_START (4)
#define REG_IRQ_FLAG_10_sc_irq_flag_10_4_END (4)
#define REG_IRQ_FLAG_10_sc_irq_flag_10_5_START (5)
#define REG_IRQ_FLAG_10_sc_irq_flag_10_5_END (5)
#define REG_IRQ_FLAG_10_sc_irq_flag_10_6_START (6)
#define REG_IRQ_FLAG_10_sc_irq_flag_10_6_END (6)
#define REG_IRQ_FLAG_10_sc_irq_flag_10_7_START (7)
#define REG_IRQ_FLAG_10_sc_irq_flag_10_7_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char sc_irq_flag_11_0 : 1;
        unsigned char sc_irq_flag_11_1 : 1;
        unsigned char sc_irq_flag_11_2 : 1;
        unsigned char sc_irq_flag_11_3 : 1;
        unsigned char sc_irq_flag_11_4 : 1;
        unsigned char sc_irq_flag_11_5 : 1;
        unsigned char sc_irq_flag_11_6 : 1;
        unsigned char sc_irq_flag_11_7 : 1;
    } reg;
} REG_IRQ_FLAG_11_UNION;
#endif
#define REG_IRQ_FLAG_11_sc_irq_flag_11_0_START (0)
#define REG_IRQ_FLAG_11_sc_irq_flag_11_0_END (0)
#define REG_IRQ_FLAG_11_sc_irq_flag_11_1_START (1)
#define REG_IRQ_FLAG_11_sc_irq_flag_11_1_END (1)
#define REG_IRQ_FLAG_11_sc_irq_flag_11_2_START (2)
#define REG_IRQ_FLAG_11_sc_irq_flag_11_2_END (2)
#define REG_IRQ_FLAG_11_sc_irq_flag_11_3_START (3)
#define REG_IRQ_FLAG_11_sc_irq_flag_11_3_END (3)
#define REG_IRQ_FLAG_11_sc_irq_flag_11_4_START (4)
#define REG_IRQ_FLAG_11_sc_irq_flag_11_4_END (4)
#define REG_IRQ_FLAG_11_sc_irq_flag_11_5_START (5)
#define REG_IRQ_FLAG_11_sc_irq_flag_11_5_END (5)
#define REG_IRQ_FLAG_11_sc_irq_flag_11_6_START (6)
#define REG_IRQ_FLAG_11_sc_irq_flag_11_6_END (6)
#define REG_IRQ_FLAG_11_sc_irq_flag_11_7_START (7)
#define REG_IRQ_FLAG_11_sc_irq_flag_11_7_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char sc_irq_mask_0 : 1;
        unsigned char sc_irq_mask_1 : 1;
        unsigned char sc_irq_mask_2 : 1;
        unsigned char sc_irq_mask_3 : 1;
        unsigned char sc_irq_mask_4 : 1;
        unsigned char sc_irq_mask_5 : 1;
        unsigned char sc_irq_mask_6 : 1;
        unsigned char sc_irq_mask_7 : 1;
    } reg;
} REG_IRQ_MASK_UNION;
#endif
#define REG_IRQ_MASK_sc_irq_mask_0_START (0)
#define REG_IRQ_MASK_sc_irq_mask_0_END (0)
#define REG_IRQ_MASK_sc_irq_mask_1_START (1)
#define REG_IRQ_MASK_sc_irq_mask_1_END (1)
#define REG_IRQ_MASK_sc_irq_mask_2_START (2)
#define REG_IRQ_MASK_sc_irq_mask_2_END (2)
#define REG_IRQ_MASK_sc_irq_mask_3_START (3)
#define REG_IRQ_MASK_sc_irq_mask_3_END (3)
#define REG_IRQ_MASK_sc_irq_mask_4_START (4)
#define REG_IRQ_MASK_sc_irq_mask_4_END (4)
#define REG_IRQ_MASK_sc_irq_mask_5_START (5)
#define REG_IRQ_MASK_sc_irq_mask_5_END (5)
#define REG_IRQ_MASK_sc_irq_mask_6_START (6)
#define REG_IRQ_MASK_sc_irq_mask_6_END (6)
#define REG_IRQ_MASK_sc_irq_mask_7_START (7)
#define REG_IRQ_MASK_sc_irq_mask_7_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char sc_irq_mask_0_0 : 1;
        unsigned char sc_irq_mask_0_1 : 1;
        unsigned char sc_irq_mask_0_2 : 1;
        unsigned char sc_irq_mask_0_3 : 1;
        unsigned char sc_irq_mask_0_4 : 1;
        unsigned char sc_irq_mask_0_5 : 1;
        unsigned char sc_irq_mask_0_6 : 1;
        unsigned char sc_irq_mask_0_7 : 1;
    } reg;
} REG_IRQ_MASK_0_UNION;
#endif
#define REG_IRQ_MASK_0_sc_irq_mask_0_0_START (0)
#define REG_IRQ_MASK_0_sc_irq_mask_0_0_END (0)
#define REG_IRQ_MASK_0_sc_irq_mask_0_1_START (1)
#define REG_IRQ_MASK_0_sc_irq_mask_0_1_END (1)
#define REG_IRQ_MASK_0_sc_irq_mask_0_2_START (2)
#define REG_IRQ_MASK_0_sc_irq_mask_0_2_END (2)
#define REG_IRQ_MASK_0_sc_irq_mask_0_3_START (3)
#define REG_IRQ_MASK_0_sc_irq_mask_0_3_END (3)
#define REG_IRQ_MASK_0_sc_irq_mask_0_4_START (4)
#define REG_IRQ_MASK_0_sc_irq_mask_0_4_END (4)
#define REG_IRQ_MASK_0_sc_irq_mask_0_5_START (5)
#define REG_IRQ_MASK_0_sc_irq_mask_0_5_END (5)
#define REG_IRQ_MASK_0_sc_irq_mask_0_6_START (6)
#define REG_IRQ_MASK_0_sc_irq_mask_0_6_END (6)
#define REG_IRQ_MASK_0_sc_irq_mask_0_7_START (7)
#define REG_IRQ_MASK_0_sc_irq_mask_0_7_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char sc_irq_mask_1_0 : 1;
        unsigned char sc_irq_mask_1_1 : 1;
        unsigned char sc_irq_mask_1_2 : 1;
        unsigned char sc_irq_mask_1_3 : 1;
        unsigned char sc_irq_mask_1_4 : 1;
        unsigned char sc_irq_mask_1_5 : 1;
        unsigned char sc_irq_mask_1_6 : 1;
        unsigned char sc_irq_mask_1_7 : 1;
    } reg;
} REG_IRQ_MASK_1_UNION;
#endif
#define REG_IRQ_MASK_1_sc_irq_mask_1_0_START (0)
#define REG_IRQ_MASK_1_sc_irq_mask_1_0_END (0)
#define REG_IRQ_MASK_1_sc_irq_mask_1_1_START (1)
#define REG_IRQ_MASK_1_sc_irq_mask_1_1_END (1)
#define REG_IRQ_MASK_1_sc_irq_mask_1_2_START (2)
#define REG_IRQ_MASK_1_sc_irq_mask_1_2_END (2)
#define REG_IRQ_MASK_1_sc_irq_mask_1_3_START (3)
#define REG_IRQ_MASK_1_sc_irq_mask_1_3_END (3)
#define REG_IRQ_MASK_1_sc_irq_mask_1_4_START (4)
#define REG_IRQ_MASK_1_sc_irq_mask_1_4_END (4)
#define REG_IRQ_MASK_1_sc_irq_mask_1_5_START (5)
#define REG_IRQ_MASK_1_sc_irq_mask_1_5_END (5)
#define REG_IRQ_MASK_1_sc_irq_mask_1_6_START (6)
#define REG_IRQ_MASK_1_sc_irq_mask_1_6_END (6)
#define REG_IRQ_MASK_1_sc_irq_mask_1_7_START (7)
#define REG_IRQ_MASK_1_sc_irq_mask_1_7_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char sc_irq_mask_2_0 : 1;
        unsigned char sc_irq_mask_2_1 : 1;
        unsigned char sc_irq_mask_2_2 : 1;
        unsigned char sc_irq_mask_2_3 : 1;
        unsigned char sc_irq_mask_2_4 : 1;
        unsigned char sc_irq_mask_2_5 : 1;
        unsigned char sc_irq_mask_2_6 : 1;
        unsigned char sc_irq_mask_2_7 : 1;
    } reg;
} REG_IRQ_MASK_2_UNION;
#endif
#define REG_IRQ_MASK_2_sc_irq_mask_2_0_START (0)
#define REG_IRQ_MASK_2_sc_irq_mask_2_0_END (0)
#define REG_IRQ_MASK_2_sc_irq_mask_2_1_START (1)
#define REG_IRQ_MASK_2_sc_irq_mask_2_1_END (1)
#define REG_IRQ_MASK_2_sc_irq_mask_2_2_START (2)
#define REG_IRQ_MASK_2_sc_irq_mask_2_2_END (2)
#define REG_IRQ_MASK_2_sc_irq_mask_2_3_START (3)
#define REG_IRQ_MASK_2_sc_irq_mask_2_3_END (3)
#define REG_IRQ_MASK_2_sc_irq_mask_2_4_START (4)
#define REG_IRQ_MASK_2_sc_irq_mask_2_4_END (4)
#define REG_IRQ_MASK_2_sc_irq_mask_2_5_START (5)
#define REG_IRQ_MASK_2_sc_irq_mask_2_5_END (5)
#define REG_IRQ_MASK_2_sc_irq_mask_2_6_START (6)
#define REG_IRQ_MASK_2_sc_irq_mask_2_6_END (6)
#define REG_IRQ_MASK_2_sc_irq_mask_2_7_START (7)
#define REG_IRQ_MASK_2_sc_irq_mask_2_7_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char sc_irq_mask_3_0 : 1;
        unsigned char sc_irq_mask_3_1 : 1;
        unsigned char sc_irq_mask_3_2 : 1;
        unsigned char sc_irq_mask_3_3 : 1;
        unsigned char sc_irq_mask_3_4 : 1;
        unsigned char sc_irq_mask_3_5 : 1;
        unsigned char sc_irq_mask_3_6 : 1;
        unsigned char sc_irq_mask_3_7 : 1;
    } reg;
} REG_IRQ_MASK_3_UNION;
#endif
#define REG_IRQ_MASK_3_sc_irq_mask_3_0_START (0)
#define REG_IRQ_MASK_3_sc_irq_mask_3_0_END (0)
#define REG_IRQ_MASK_3_sc_irq_mask_3_1_START (1)
#define REG_IRQ_MASK_3_sc_irq_mask_3_1_END (1)
#define REG_IRQ_MASK_3_sc_irq_mask_3_2_START (2)
#define REG_IRQ_MASK_3_sc_irq_mask_3_2_END (2)
#define REG_IRQ_MASK_3_sc_irq_mask_3_3_START (3)
#define REG_IRQ_MASK_3_sc_irq_mask_3_3_END (3)
#define REG_IRQ_MASK_3_sc_irq_mask_3_4_START (4)
#define REG_IRQ_MASK_3_sc_irq_mask_3_4_END (4)
#define REG_IRQ_MASK_3_sc_irq_mask_3_5_START (5)
#define REG_IRQ_MASK_3_sc_irq_mask_3_5_END (5)
#define REG_IRQ_MASK_3_sc_irq_mask_3_6_START (6)
#define REG_IRQ_MASK_3_sc_irq_mask_3_6_END (6)
#define REG_IRQ_MASK_3_sc_irq_mask_3_7_START (7)
#define REG_IRQ_MASK_3_sc_irq_mask_3_7_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char sc_irq_mask_4_0 : 1;
        unsigned char sc_irq_mask_4_1 : 1;
        unsigned char sc_irq_mask_4_2 : 1;
        unsigned char sc_irq_mask_4_3 : 1;
        unsigned char sc_irq_mask_4_4 : 1;
        unsigned char sc_irq_mask_4_5 : 1;
        unsigned char sc_irq_mask_4_6 : 1;
        unsigned char sc_irq_mask_4_7 : 1;
    } reg;
} REG_IRQ_MASK_4_UNION;
#endif
#define REG_IRQ_MASK_4_sc_irq_mask_4_0_START (0)
#define REG_IRQ_MASK_4_sc_irq_mask_4_0_END (0)
#define REG_IRQ_MASK_4_sc_irq_mask_4_1_START (1)
#define REG_IRQ_MASK_4_sc_irq_mask_4_1_END (1)
#define REG_IRQ_MASK_4_sc_irq_mask_4_2_START (2)
#define REG_IRQ_MASK_4_sc_irq_mask_4_2_END (2)
#define REG_IRQ_MASK_4_sc_irq_mask_4_3_START (3)
#define REG_IRQ_MASK_4_sc_irq_mask_4_3_END (3)
#define REG_IRQ_MASK_4_sc_irq_mask_4_4_START (4)
#define REG_IRQ_MASK_4_sc_irq_mask_4_4_END (4)
#define REG_IRQ_MASK_4_sc_irq_mask_4_5_START (5)
#define REG_IRQ_MASK_4_sc_irq_mask_4_5_END (5)
#define REG_IRQ_MASK_4_sc_irq_mask_4_6_START (6)
#define REG_IRQ_MASK_4_sc_irq_mask_4_6_END (6)
#define REG_IRQ_MASK_4_sc_irq_mask_4_7_START (7)
#define REG_IRQ_MASK_4_sc_irq_mask_4_7_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char sc_irq_mask_5_0 : 1;
        unsigned char sc_irq_mask_5_1 : 1;
        unsigned char sc_irq_mask_5_2 : 1;
        unsigned char sc_irq_mask_5_3 : 1;
        unsigned char sc_irq_mask_5_4 : 1;
        unsigned char sc_irq_mask_5_5 : 1;
        unsigned char sc_irq_mask_5_6 : 1;
        unsigned char sc_irq_mask_5_7 : 1;
    } reg;
} REG_IRQ_MASK_5_UNION;
#endif
#define REG_IRQ_MASK_5_sc_irq_mask_5_0_START (0)
#define REG_IRQ_MASK_5_sc_irq_mask_5_0_END (0)
#define REG_IRQ_MASK_5_sc_irq_mask_5_1_START (1)
#define REG_IRQ_MASK_5_sc_irq_mask_5_1_END (1)
#define REG_IRQ_MASK_5_sc_irq_mask_5_2_START (2)
#define REG_IRQ_MASK_5_sc_irq_mask_5_2_END (2)
#define REG_IRQ_MASK_5_sc_irq_mask_5_3_START (3)
#define REG_IRQ_MASK_5_sc_irq_mask_5_3_END (3)
#define REG_IRQ_MASK_5_sc_irq_mask_5_4_START (4)
#define REG_IRQ_MASK_5_sc_irq_mask_5_4_END (4)
#define REG_IRQ_MASK_5_sc_irq_mask_5_5_START (5)
#define REG_IRQ_MASK_5_sc_irq_mask_5_5_END (5)
#define REG_IRQ_MASK_5_sc_irq_mask_5_6_START (6)
#define REG_IRQ_MASK_5_sc_irq_mask_5_6_END (6)
#define REG_IRQ_MASK_5_sc_irq_mask_5_7_START (7)
#define REG_IRQ_MASK_5_sc_irq_mask_5_7_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char sc_irq_mask_6_0 : 1;
        unsigned char sc_irq_mask_6_1 : 1;
        unsigned char sc_irq_mask_6_2 : 1;
        unsigned char sc_irq_mask_6_3 : 1;
        unsigned char sc_irq_mask_6_4 : 1;
        unsigned char sc_irq_mask_6_5 : 1;
        unsigned char sc_irq_mask_6_6 : 1;
        unsigned char sc_irq_mask_6_7 : 1;
    } reg;
} REG_IRQ_MASK_6_UNION;
#endif
#define REG_IRQ_MASK_6_sc_irq_mask_6_0_START (0)
#define REG_IRQ_MASK_6_sc_irq_mask_6_0_END (0)
#define REG_IRQ_MASK_6_sc_irq_mask_6_1_START (1)
#define REG_IRQ_MASK_6_sc_irq_mask_6_1_END (1)
#define REG_IRQ_MASK_6_sc_irq_mask_6_2_START (2)
#define REG_IRQ_MASK_6_sc_irq_mask_6_2_END (2)
#define REG_IRQ_MASK_6_sc_irq_mask_6_3_START (3)
#define REG_IRQ_MASK_6_sc_irq_mask_6_3_END (3)
#define REG_IRQ_MASK_6_sc_irq_mask_6_4_START (4)
#define REG_IRQ_MASK_6_sc_irq_mask_6_4_END (4)
#define REG_IRQ_MASK_6_sc_irq_mask_6_5_START (5)
#define REG_IRQ_MASK_6_sc_irq_mask_6_5_END (5)
#define REG_IRQ_MASK_6_sc_irq_mask_6_6_START (6)
#define REG_IRQ_MASK_6_sc_irq_mask_6_6_END (6)
#define REG_IRQ_MASK_6_sc_irq_mask_6_7_START (7)
#define REG_IRQ_MASK_6_sc_irq_mask_6_7_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char sc_irq_mask_7_0 : 1;
        unsigned char sc_irq_mask_7_1 : 1;
        unsigned char sc_irq_mask_7_2 : 1;
        unsigned char sc_irq_mask_7_3 : 1;
        unsigned char sc_irq_mask_7_4 : 1;
        unsigned char sc_irq_mask_7_5 : 1;
        unsigned char sc_irq_mask_7_6 : 1;
        unsigned char sc_irq_mask_7_7 : 1;
    } reg;
} REG_IRQ_MASK_7_UNION;
#endif
#define REG_IRQ_MASK_7_sc_irq_mask_7_0_START (0)
#define REG_IRQ_MASK_7_sc_irq_mask_7_0_END (0)
#define REG_IRQ_MASK_7_sc_irq_mask_7_1_START (1)
#define REG_IRQ_MASK_7_sc_irq_mask_7_1_END (1)
#define REG_IRQ_MASK_7_sc_irq_mask_7_2_START (2)
#define REG_IRQ_MASK_7_sc_irq_mask_7_2_END (2)
#define REG_IRQ_MASK_7_sc_irq_mask_7_3_START (3)
#define REG_IRQ_MASK_7_sc_irq_mask_7_3_END (3)
#define REG_IRQ_MASK_7_sc_irq_mask_7_4_START (4)
#define REG_IRQ_MASK_7_sc_irq_mask_7_4_END (4)
#define REG_IRQ_MASK_7_sc_irq_mask_7_5_START (5)
#define REG_IRQ_MASK_7_sc_irq_mask_7_5_END (5)
#define REG_IRQ_MASK_7_sc_irq_mask_7_6_START (6)
#define REG_IRQ_MASK_7_sc_irq_mask_7_6_END (6)
#define REG_IRQ_MASK_7_sc_irq_mask_7_7_START (7)
#define REG_IRQ_MASK_7_sc_irq_mask_7_7_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char sc_irq_mask_8_0 : 1;
        unsigned char sc_irq_mask_8_1 : 1;
        unsigned char sc_irq_mask_8_2 : 1;
        unsigned char sc_irq_mask_8_3 : 1;
        unsigned char sc_irq_mask_8_4 : 1;
        unsigned char sc_irq_mask_8_5 : 1;
        unsigned char sc_irq_mask_8_6 : 1;
        unsigned char sc_irq_mask_8_7 : 1;
    } reg;
} REG_IRQ_MASK_8_UNION;
#endif
#define REG_IRQ_MASK_8_sc_irq_mask_8_0_START (0)
#define REG_IRQ_MASK_8_sc_irq_mask_8_0_END (0)
#define REG_IRQ_MASK_8_sc_irq_mask_8_1_START (1)
#define REG_IRQ_MASK_8_sc_irq_mask_8_1_END (1)
#define REG_IRQ_MASK_8_sc_irq_mask_8_2_START (2)
#define REG_IRQ_MASK_8_sc_irq_mask_8_2_END (2)
#define REG_IRQ_MASK_8_sc_irq_mask_8_3_START (3)
#define REG_IRQ_MASK_8_sc_irq_mask_8_3_END (3)
#define REG_IRQ_MASK_8_sc_irq_mask_8_4_START (4)
#define REG_IRQ_MASK_8_sc_irq_mask_8_4_END (4)
#define REG_IRQ_MASK_8_sc_irq_mask_8_5_START (5)
#define REG_IRQ_MASK_8_sc_irq_mask_8_5_END (5)
#define REG_IRQ_MASK_8_sc_irq_mask_8_6_START (6)
#define REG_IRQ_MASK_8_sc_irq_mask_8_6_END (6)
#define REG_IRQ_MASK_8_sc_irq_mask_8_7_START (7)
#define REG_IRQ_MASK_8_sc_irq_mask_8_7_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char sc_irq_mask_9_0 : 1;
        unsigned char sc_irq_mask_9_1 : 1;
        unsigned char sc_irq_mask_9_2 : 1;
        unsigned char sc_irq_mask_9_3 : 1;
        unsigned char sc_irq_mask_9_4 : 1;
        unsigned char sc_irq_mask_9_5 : 1;
        unsigned char sc_irq_mask_9_6 : 1;
        unsigned char sc_irq_mask_9_7 : 1;
    } reg;
} REG_IRQ_MASK_9_UNION;
#endif
#define REG_IRQ_MASK_9_sc_irq_mask_9_0_START (0)
#define REG_IRQ_MASK_9_sc_irq_mask_9_0_END (0)
#define REG_IRQ_MASK_9_sc_irq_mask_9_1_START (1)
#define REG_IRQ_MASK_9_sc_irq_mask_9_1_END (1)
#define REG_IRQ_MASK_9_sc_irq_mask_9_2_START (2)
#define REG_IRQ_MASK_9_sc_irq_mask_9_2_END (2)
#define REG_IRQ_MASK_9_sc_irq_mask_9_3_START (3)
#define REG_IRQ_MASK_9_sc_irq_mask_9_3_END (3)
#define REG_IRQ_MASK_9_sc_irq_mask_9_4_START (4)
#define REG_IRQ_MASK_9_sc_irq_mask_9_4_END (4)
#define REG_IRQ_MASK_9_sc_irq_mask_9_5_START (5)
#define REG_IRQ_MASK_9_sc_irq_mask_9_5_END (5)
#define REG_IRQ_MASK_9_sc_irq_mask_9_6_START (6)
#define REG_IRQ_MASK_9_sc_irq_mask_9_6_END (6)
#define REG_IRQ_MASK_9_sc_irq_mask_9_7_START (7)
#define REG_IRQ_MASK_9_sc_irq_mask_9_7_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char sc_irq_mask_10_0 : 1;
        unsigned char sc_irq_mask_10_1 : 1;
        unsigned char sc_irq_mask_10_2 : 1;
        unsigned char sc_irq_mask_10_3 : 1;
        unsigned char sc_irq_mask_10_4 : 1;
        unsigned char sc_irq_mask_10_5 : 1;
        unsigned char sc_irq_mask_10_6 : 1;
        unsigned char sc_irq_mask_10_7 : 1;
    } reg;
} REG_IRQ_MASK_10_UNION;
#endif
#define REG_IRQ_MASK_10_sc_irq_mask_10_0_START (0)
#define REG_IRQ_MASK_10_sc_irq_mask_10_0_END (0)
#define REG_IRQ_MASK_10_sc_irq_mask_10_1_START (1)
#define REG_IRQ_MASK_10_sc_irq_mask_10_1_END (1)
#define REG_IRQ_MASK_10_sc_irq_mask_10_2_START (2)
#define REG_IRQ_MASK_10_sc_irq_mask_10_2_END (2)
#define REG_IRQ_MASK_10_sc_irq_mask_10_3_START (3)
#define REG_IRQ_MASK_10_sc_irq_mask_10_3_END (3)
#define REG_IRQ_MASK_10_sc_irq_mask_10_4_START (4)
#define REG_IRQ_MASK_10_sc_irq_mask_10_4_END (4)
#define REG_IRQ_MASK_10_sc_irq_mask_10_5_START (5)
#define REG_IRQ_MASK_10_sc_irq_mask_10_5_END (5)
#define REG_IRQ_MASK_10_sc_irq_mask_10_6_START (6)
#define REG_IRQ_MASK_10_sc_irq_mask_10_6_END (6)
#define REG_IRQ_MASK_10_sc_irq_mask_10_7_START (7)
#define REG_IRQ_MASK_10_sc_irq_mask_10_7_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char sc_irq_mask_11_0 : 1;
        unsigned char sc_irq_mask_11_1 : 1;
        unsigned char sc_irq_mask_11_2 : 1;
        unsigned char sc_irq_mask_11_3 : 1;
        unsigned char sc_irq_mask_11_4 : 1;
        unsigned char sc_irq_mask_11_5 : 1;
        unsigned char sc_irq_mask_11_6 : 1;
        unsigned char sc_irq_mask_11_7 : 1;
    } reg;
} REG_IRQ_MASK_11_UNION;
#endif
#define REG_IRQ_MASK_11_sc_irq_mask_11_0_START (0)
#define REG_IRQ_MASK_11_sc_irq_mask_11_0_END (0)
#define REG_IRQ_MASK_11_sc_irq_mask_11_1_START (1)
#define REG_IRQ_MASK_11_sc_irq_mask_11_1_END (1)
#define REG_IRQ_MASK_11_sc_irq_mask_11_2_START (2)
#define REG_IRQ_MASK_11_sc_irq_mask_11_2_END (2)
#define REG_IRQ_MASK_11_sc_irq_mask_11_3_START (3)
#define REG_IRQ_MASK_11_sc_irq_mask_11_3_END (3)
#define REG_IRQ_MASK_11_sc_irq_mask_11_4_START (4)
#define REG_IRQ_MASK_11_sc_irq_mask_11_4_END (4)
#define REG_IRQ_MASK_11_sc_irq_mask_11_5_START (5)
#define REG_IRQ_MASK_11_sc_irq_mask_11_5_END (5)
#define REG_IRQ_MASK_11_sc_irq_mask_11_6_START (6)
#define REG_IRQ_MASK_11_sc_irq_mask_11_6_END (6)
#define REG_IRQ_MASK_11_sc_irq_mask_11_7_START (7)
#define REG_IRQ_MASK_11_sc_irq_mask_11_7_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char irq_status_0_0 : 1;
        unsigned char irq_status_0_1 : 1;
        unsigned char irq_status_0_2 : 1;
        unsigned char irq_status_0_3 : 1;
        unsigned char irq_status_0_4 : 1;
        unsigned char irq_status_0_5 : 1;
        unsigned char irq_status_0_6 : 1;
        unsigned char irq_status_0_7 : 1;
    } reg;
} REG_IRQ_STATUS_0_UNION;
#endif
#define REG_IRQ_STATUS_0_irq_status_0_0_START (0)
#define REG_IRQ_STATUS_0_irq_status_0_0_END (0)
#define REG_IRQ_STATUS_0_irq_status_0_1_START (1)
#define REG_IRQ_STATUS_0_irq_status_0_1_END (1)
#define REG_IRQ_STATUS_0_irq_status_0_2_START (2)
#define REG_IRQ_STATUS_0_irq_status_0_2_END (2)
#define REG_IRQ_STATUS_0_irq_status_0_3_START (3)
#define REG_IRQ_STATUS_0_irq_status_0_3_END (3)
#define REG_IRQ_STATUS_0_irq_status_0_4_START (4)
#define REG_IRQ_STATUS_0_irq_status_0_4_END (4)
#define REG_IRQ_STATUS_0_irq_status_0_5_START (5)
#define REG_IRQ_STATUS_0_irq_status_0_5_END (5)
#define REG_IRQ_STATUS_0_irq_status_0_6_START (6)
#define REG_IRQ_STATUS_0_irq_status_0_6_END (6)
#define REG_IRQ_STATUS_0_irq_status_0_7_START (7)
#define REG_IRQ_STATUS_0_irq_status_0_7_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char irq_status_1_0 : 1;
        unsigned char irq_status_1_1 : 1;
        unsigned char irq_status_1_2 : 1;
        unsigned char irq_status_1_3 : 1;
        unsigned char irq_status_1_4 : 1;
        unsigned char irq_status_1_5 : 1;
        unsigned char irq_status_1_6 : 1;
        unsigned char irq_status_1_7 : 1;
    } reg;
} REG_IRQ_STATUS_1_UNION;
#endif
#define REG_IRQ_STATUS_1_irq_status_1_0_START (0)
#define REG_IRQ_STATUS_1_irq_status_1_0_END (0)
#define REG_IRQ_STATUS_1_irq_status_1_1_START (1)
#define REG_IRQ_STATUS_1_irq_status_1_1_END (1)
#define REG_IRQ_STATUS_1_irq_status_1_2_START (2)
#define REG_IRQ_STATUS_1_irq_status_1_2_END (2)
#define REG_IRQ_STATUS_1_irq_status_1_3_START (3)
#define REG_IRQ_STATUS_1_irq_status_1_3_END (3)
#define REG_IRQ_STATUS_1_irq_status_1_4_START (4)
#define REG_IRQ_STATUS_1_irq_status_1_4_END (4)
#define REG_IRQ_STATUS_1_irq_status_1_5_START (5)
#define REG_IRQ_STATUS_1_irq_status_1_5_END (5)
#define REG_IRQ_STATUS_1_irq_status_1_6_START (6)
#define REG_IRQ_STATUS_1_irq_status_1_6_END (6)
#define REG_IRQ_STATUS_1_irq_status_1_7_START (7)
#define REG_IRQ_STATUS_1_irq_status_1_7_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char irq_status_2_0 : 1;
        unsigned char irq_status_2_1 : 1;
        unsigned char irq_status_2_2 : 1;
        unsigned char irq_status_2_3 : 1;
        unsigned char irq_status_2_4 : 1;
        unsigned char irq_status_2_5 : 1;
        unsigned char irq_status_2_6 : 1;
        unsigned char irq_status_2_7 : 1;
    } reg;
} REG_IRQ_STATUS_2_UNION;
#endif
#define REG_IRQ_STATUS_2_irq_status_2_0_START (0)
#define REG_IRQ_STATUS_2_irq_status_2_0_END (0)
#define REG_IRQ_STATUS_2_irq_status_2_1_START (1)
#define REG_IRQ_STATUS_2_irq_status_2_1_END (1)
#define REG_IRQ_STATUS_2_irq_status_2_2_START (2)
#define REG_IRQ_STATUS_2_irq_status_2_2_END (2)
#define REG_IRQ_STATUS_2_irq_status_2_3_START (3)
#define REG_IRQ_STATUS_2_irq_status_2_3_END (3)
#define REG_IRQ_STATUS_2_irq_status_2_4_START (4)
#define REG_IRQ_STATUS_2_irq_status_2_4_END (4)
#define REG_IRQ_STATUS_2_irq_status_2_5_START (5)
#define REG_IRQ_STATUS_2_irq_status_2_5_END (5)
#define REG_IRQ_STATUS_2_irq_status_2_6_START (6)
#define REG_IRQ_STATUS_2_irq_status_2_6_END (6)
#define REG_IRQ_STATUS_2_irq_status_2_7_START (7)
#define REG_IRQ_STATUS_2_irq_status_2_7_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char irq_status_3_0 : 1;
        unsigned char irq_status_3_1 : 1;
        unsigned char irq_status_3_2 : 1;
        unsigned char irq_status_3_3 : 1;
        unsigned char irq_status_3_4 : 1;
        unsigned char irq_status_3_5 : 1;
        unsigned char irq_status_3_6 : 1;
        unsigned char irq_status_3_7 : 1;
    } reg;
} REG_IRQ_STATUS_3_UNION;
#endif
#define REG_IRQ_STATUS_3_irq_status_3_0_START (0)
#define REG_IRQ_STATUS_3_irq_status_3_0_END (0)
#define REG_IRQ_STATUS_3_irq_status_3_1_START (1)
#define REG_IRQ_STATUS_3_irq_status_3_1_END (1)
#define REG_IRQ_STATUS_3_irq_status_3_2_START (2)
#define REG_IRQ_STATUS_3_irq_status_3_2_END (2)
#define REG_IRQ_STATUS_3_irq_status_3_3_START (3)
#define REG_IRQ_STATUS_3_irq_status_3_3_END (3)
#define REG_IRQ_STATUS_3_irq_status_3_4_START (4)
#define REG_IRQ_STATUS_3_irq_status_3_4_END (4)
#define REG_IRQ_STATUS_3_irq_status_3_5_START (5)
#define REG_IRQ_STATUS_3_irq_status_3_5_END (5)
#define REG_IRQ_STATUS_3_irq_status_3_6_START (6)
#define REG_IRQ_STATUS_3_irq_status_3_6_END (6)
#define REG_IRQ_STATUS_3_irq_status_3_7_START (7)
#define REG_IRQ_STATUS_3_irq_status_3_7_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char irq_status_4_0 : 1;
        unsigned char irq_status_4_1 : 1;
        unsigned char irq_status_4_2 : 1;
        unsigned char irq_status_4_3 : 1;
        unsigned char irq_status_4_4 : 1;
        unsigned char irq_status_4_5 : 1;
        unsigned char irq_status_4_6 : 1;
        unsigned char irq_status_4_7 : 1;
    } reg;
} REG_IRQ_STATUS_4_UNION;
#endif
#define REG_IRQ_STATUS_4_irq_status_4_0_START (0)
#define REG_IRQ_STATUS_4_irq_status_4_0_END (0)
#define REG_IRQ_STATUS_4_irq_status_4_1_START (1)
#define REG_IRQ_STATUS_4_irq_status_4_1_END (1)
#define REG_IRQ_STATUS_4_irq_status_4_2_START (2)
#define REG_IRQ_STATUS_4_irq_status_4_2_END (2)
#define REG_IRQ_STATUS_4_irq_status_4_3_START (3)
#define REG_IRQ_STATUS_4_irq_status_4_3_END (3)
#define REG_IRQ_STATUS_4_irq_status_4_4_START (4)
#define REG_IRQ_STATUS_4_irq_status_4_4_END (4)
#define REG_IRQ_STATUS_4_irq_status_4_5_START (5)
#define REG_IRQ_STATUS_4_irq_status_4_5_END (5)
#define REG_IRQ_STATUS_4_irq_status_4_6_START (6)
#define REG_IRQ_STATUS_4_irq_status_4_6_END (6)
#define REG_IRQ_STATUS_4_irq_status_4_7_START (7)
#define REG_IRQ_STATUS_4_irq_status_4_7_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char irq_status_5_0 : 1;
        unsigned char irq_status_5_1 : 1;
        unsigned char irq_status_5_2 : 1;
        unsigned char irq_status_5_3 : 1;
        unsigned char irq_status_5_4 : 1;
        unsigned char irq_status_5_5 : 1;
        unsigned char irq_status_5_6 : 1;
        unsigned char irq_status_5_7 : 1;
    } reg;
} REG_IRQ_STATUS_5_UNION;
#endif
#define REG_IRQ_STATUS_5_irq_status_5_0_START (0)
#define REG_IRQ_STATUS_5_irq_status_5_0_END (0)
#define REG_IRQ_STATUS_5_irq_status_5_1_START (1)
#define REG_IRQ_STATUS_5_irq_status_5_1_END (1)
#define REG_IRQ_STATUS_5_irq_status_5_2_START (2)
#define REG_IRQ_STATUS_5_irq_status_5_2_END (2)
#define REG_IRQ_STATUS_5_irq_status_5_3_START (3)
#define REG_IRQ_STATUS_5_irq_status_5_3_END (3)
#define REG_IRQ_STATUS_5_irq_status_5_4_START (4)
#define REG_IRQ_STATUS_5_irq_status_5_4_END (4)
#define REG_IRQ_STATUS_5_irq_status_5_5_START (5)
#define REG_IRQ_STATUS_5_irq_status_5_5_END (5)
#define REG_IRQ_STATUS_5_irq_status_5_6_START (6)
#define REG_IRQ_STATUS_5_irq_status_5_6_END (6)
#define REG_IRQ_STATUS_5_irq_status_5_7_START (7)
#define REG_IRQ_STATUS_5_irq_status_5_7_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char irq_status_6_0 : 1;
        unsigned char irq_status_6_1 : 1;
        unsigned char irq_status_6_2 : 1;
        unsigned char irq_status_6_3 : 1;
        unsigned char irq_status_6_4 : 1;
        unsigned char irq_status_6_5 : 1;
        unsigned char irq_status_6_6 : 1;
        unsigned char irq_status_6_7 : 1;
    } reg;
} REG_IRQ_STATUS_6_UNION;
#endif
#define REG_IRQ_STATUS_6_irq_status_6_0_START (0)
#define REG_IRQ_STATUS_6_irq_status_6_0_END (0)
#define REG_IRQ_STATUS_6_irq_status_6_1_START (1)
#define REG_IRQ_STATUS_6_irq_status_6_1_END (1)
#define REG_IRQ_STATUS_6_irq_status_6_2_START (2)
#define REG_IRQ_STATUS_6_irq_status_6_2_END (2)
#define REG_IRQ_STATUS_6_irq_status_6_3_START (3)
#define REG_IRQ_STATUS_6_irq_status_6_3_END (3)
#define REG_IRQ_STATUS_6_irq_status_6_4_START (4)
#define REG_IRQ_STATUS_6_irq_status_6_4_END (4)
#define REG_IRQ_STATUS_6_irq_status_6_5_START (5)
#define REG_IRQ_STATUS_6_irq_status_6_5_END (5)
#define REG_IRQ_STATUS_6_irq_status_6_6_START (6)
#define REG_IRQ_STATUS_6_irq_status_6_6_END (6)
#define REG_IRQ_STATUS_6_irq_status_6_7_START (7)
#define REG_IRQ_STATUS_6_irq_status_6_7_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char irq_status_7_0 : 1;
        unsigned char irq_status_7_1 : 1;
        unsigned char irq_status_7_2 : 1;
        unsigned char irq_status_7_3 : 1;
        unsigned char irq_status_7_4 : 1;
        unsigned char irq_status_7_5 : 1;
        unsigned char irq_status_7_6 : 1;
        unsigned char irq_status_7_7 : 1;
    } reg;
} REG_IRQ_STATUS_7_UNION;
#endif
#define REG_IRQ_STATUS_7_irq_status_7_0_START (0)
#define REG_IRQ_STATUS_7_irq_status_7_0_END (0)
#define REG_IRQ_STATUS_7_irq_status_7_1_START (1)
#define REG_IRQ_STATUS_7_irq_status_7_1_END (1)
#define REG_IRQ_STATUS_7_irq_status_7_2_START (2)
#define REG_IRQ_STATUS_7_irq_status_7_2_END (2)
#define REG_IRQ_STATUS_7_irq_status_7_3_START (3)
#define REG_IRQ_STATUS_7_irq_status_7_3_END (3)
#define REG_IRQ_STATUS_7_irq_status_7_4_START (4)
#define REG_IRQ_STATUS_7_irq_status_7_4_END (4)
#define REG_IRQ_STATUS_7_irq_status_7_5_START (5)
#define REG_IRQ_STATUS_7_irq_status_7_5_END (5)
#define REG_IRQ_STATUS_7_irq_status_7_6_START (6)
#define REG_IRQ_STATUS_7_irq_status_7_6_END (6)
#define REG_IRQ_STATUS_7_irq_status_7_7_START (7)
#define REG_IRQ_STATUS_7_irq_status_7_7_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char irq_status_8_0 : 1;
        unsigned char irq_status_8_1 : 1;
        unsigned char irq_status_8_2 : 1;
        unsigned char irq_status_8_3 : 1;
        unsigned char irq_status_8_4 : 1;
        unsigned char irq_status_8_5 : 1;
        unsigned char irq_status_8_6 : 1;
        unsigned char irq_status_8_7 : 1;
    } reg;
} REG_IRQ_STATUS_8_UNION;
#endif
#define REG_IRQ_STATUS_8_irq_status_8_0_START (0)
#define REG_IRQ_STATUS_8_irq_status_8_0_END (0)
#define REG_IRQ_STATUS_8_irq_status_8_1_START (1)
#define REG_IRQ_STATUS_8_irq_status_8_1_END (1)
#define REG_IRQ_STATUS_8_irq_status_8_2_START (2)
#define REG_IRQ_STATUS_8_irq_status_8_2_END (2)
#define REG_IRQ_STATUS_8_irq_status_8_3_START (3)
#define REG_IRQ_STATUS_8_irq_status_8_3_END (3)
#define REG_IRQ_STATUS_8_irq_status_8_4_START (4)
#define REG_IRQ_STATUS_8_irq_status_8_4_END (4)
#define REG_IRQ_STATUS_8_irq_status_8_5_START (5)
#define REG_IRQ_STATUS_8_irq_status_8_5_END (5)
#define REG_IRQ_STATUS_8_irq_status_8_6_START (6)
#define REG_IRQ_STATUS_8_irq_status_8_6_END (6)
#define REG_IRQ_STATUS_8_irq_status_8_7_START (7)
#define REG_IRQ_STATUS_8_irq_status_8_7_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char irq_status_9_0 : 1;
        unsigned char irq_status_9_1 : 1;
        unsigned char irq_status_9_2 : 1;
        unsigned char irq_status_9_3 : 1;
        unsigned char irq_status_9_4 : 1;
        unsigned char irq_status_9_5 : 1;
        unsigned char irq_status_9_6 : 1;
        unsigned char irq_status_9_7 : 1;
    } reg;
} REG_IRQ_STATUS_9_UNION;
#endif
#define REG_IRQ_STATUS_9_irq_status_9_0_START (0)
#define REG_IRQ_STATUS_9_irq_status_9_0_END (0)
#define REG_IRQ_STATUS_9_irq_status_9_1_START (1)
#define REG_IRQ_STATUS_9_irq_status_9_1_END (1)
#define REG_IRQ_STATUS_9_irq_status_9_2_START (2)
#define REG_IRQ_STATUS_9_irq_status_9_2_END (2)
#define REG_IRQ_STATUS_9_irq_status_9_3_START (3)
#define REG_IRQ_STATUS_9_irq_status_9_3_END (3)
#define REG_IRQ_STATUS_9_irq_status_9_4_START (4)
#define REG_IRQ_STATUS_9_irq_status_9_4_END (4)
#define REG_IRQ_STATUS_9_irq_status_9_5_START (5)
#define REG_IRQ_STATUS_9_irq_status_9_5_END (5)
#define REG_IRQ_STATUS_9_irq_status_9_6_START (6)
#define REG_IRQ_STATUS_9_irq_status_9_6_END (6)
#define REG_IRQ_STATUS_9_irq_status_9_7_START (7)
#define REG_IRQ_STATUS_9_irq_status_9_7_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char irq_status_10_0 : 1;
        unsigned char irq_status_10_1 : 1;
        unsigned char irq_status_10_2 : 1;
        unsigned char irq_status_10_3 : 1;
        unsigned char irq_status_10_4 : 1;
        unsigned char irq_status_10_5 : 1;
        unsigned char irq_status_10_6 : 1;
        unsigned char irq_status_10_7 : 1;
    } reg;
} REG_IRQ_STATUS_10_UNION;
#endif
#define REG_IRQ_STATUS_10_irq_status_10_0_START (0)
#define REG_IRQ_STATUS_10_irq_status_10_0_END (0)
#define REG_IRQ_STATUS_10_irq_status_10_1_START (1)
#define REG_IRQ_STATUS_10_irq_status_10_1_END (1)
#define REG_IRQ_STATUS_10_irq_status_10_2_START (2)
#define REG_IRQ_STATUS_10_irq_status_10_2_END (2)
#define REG_IRQ_STATUS_10_irq_status_10_3_START (3)
#define REG_IRQ_STATUS_10_irq_status_10_3_END (3)
#define REG_IRQ_STATUS_10_irq_status_10_4_START (4)
#define REG_IRQ_STATUS_10_irq_status_10_4_END (4)
#define REG_IRQ_STATUS_10_irq_status_10_5_START (5)
#define REG_IRQ_STATUS_10_irq_status_10_5_END (5)
#define REG_IRQ_STATUS_10_irq_status_10_6_START (6)
#define REG_IRQ_STATUS_10_irq_status_10_6_END (6)
#define REG_IRQ_STATUS_10_irq_status_10_7_START (7)
#define REG_IRQ_STATUS_10_irq_status_10_7_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char irq_status_11_0 : 1;
        unsigned char irq_status_11_1 : 1;
        unsigned char irq_status_11_2 : 1;
        unsigned char irq_status_11_3 : 1;
        unsigned char irq_status_11_4 : 1;
        unsigned char irq_status_11_5 : 1;
        unsigned char irq_status_11_6 : 1;
        unsigned char irq_status_11_7 : 1;
    } reg;
} REG_IRQ_STATUS_11_UNION;
#endif
#define REG_IRQ_STATUS_11_irq_status_11_0_START (0)
#define REG_IRQ_STATUS_11_irq_status_11_0_END (0)
#define REG_IRQ_STATUS_11_irq_status_11_1_START (1)
#define REG_IRQ_STATUS_11_irq_status_11_1_END (1)
#define REG_IRQ_STATUS_11_irq_status_11_2_START (2)
#define REG_IRQ_STATUS_11_irq_status_11_2_END (2)
#define REG_IRQ_STATUS_11_irq_status_11_3_START (3)
#define REG_IRQ_STATUS_11_irq_status_11_3_END (3)
#define REG_IRQ_STATUS_11_irq_status_11_4_START (4)
#define REG_IRQ_STATUS_11_irq_status_11_4_END (4)
#define REG_IRQ_STATUS_11_irq_status_11_5_START (5)
#define REG_IRQ_STATUS_11_irq_status_11_5_END (5)
#define REG_IRQ_STATUS_11_irq_status_11_6_START (6)
#define REG_IRQ_STATUS_11_irq_status_11_6_END (6)
#define REG_IRQ_STATUS_11_irq_status_11_7_START (7)
#define REG_IRQ_STATUS_11_irq_status_11_7_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char ad_psw_vdrop_ovp_l : 1;
        unsigned char ad_psw_vdrop_ovp_h : 1;
        unsigned char reserved : 6;
    } reg;
} REG_IRQ_ANA_STATUS_UNION;
#endif
#define REG_IRQ_ANA_STATUS_ad_psw_vdrop_ovp_l_START (0)
#define REG_IRQ_ANA_STATUS_ad_psw_vdrop_ovp_l_END (0)
#define REG_IRQ_ANA_STATUS_ad_psw_vdrop_ovp_h_START (1)
#define REG_IRQ_ANA_STATUS_ad_psw_vdrop_ovp_h_END (1)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char sc_fake_irq : 8;
    } reg;
} REG_FAKE_IRQ_UNION;
#endif
#define REG_FAKE_IRQ_sc_fake_irq_START (0)
#define REG_FAKE_IRQ_sc_fake_irq_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char sc_irq_sel : 8;
    } reg;
} REG_IRQ_SEL_UNION;
#endif
#define REG_IRQ_SEL_sc_irq_sel_START (0)
#define REG_IRQ_SEL_sc_irq_sel_END (7)
#ifdef __cplusplus
    #if __cplusplus
        }
    #endif
#endif
#endif
