#ifndef __SCHARGER_V700_ADC_H__
#define __SCHARGER_V700_ADC_H__

#include <soc_schargerV700_interface.h>

#define scharger_err(fmt, args...) pr_err("[schargerV700]" fmt, ## args)
#define scharger_evt(fmt, args...) pr_warn("[schargerV700]" fmt, ## args)
#define scharger_inf(fmt, args...) pr_info("[schargerV700]" fmt, ## args)
#define scharger_dbg(fmt, args...) pr_debug("[schargerV700]" fmt, ## args)

#define REG_HKADC_CTRL1 0x202

#define SC_HKADC_SEQ_LOOP_SHIFT 4
#define SC_HKADC_SEQ_LOOP_MASK BIT(4)

#define REG_EIS_EN 0x250
#define SC_ADC_EIS_SEL_MASK BIT(0)

#define REG_HKADC_EN           REG_HKADC_EN_ADDR(REG_ADC_BASE)
#define SC_HKADC_EN_MASK       BIT(0)

#define REG_HKADC_SEQ_CH_H     REG_HKADC_SEQ_CH_H_ADDR(REG_ADC_BASE)
#define REG_HKADC_SEQ_CH_L     REG_HKADC_SEQ_CH_L_ADDR(REG_ADC_BASE)

#define REG_EIS_HKADC_START    REG_EIS_HKADC_START_ADDR(REG_ADC_BASE)
#define SC_EIS_HKADC_START_MASK BIT(0)

#define REG_HKADC_DATA_VALID   REG_HKADC_DATA_VALID_ADDR(REG_ADC_BASE)
#define HKADC_DATA_VALID_MASK BIT(0)

#define REG_HKADC_RD_SEQ       REG_HKADC_RD_SEQ_ADDR(REG_ADC_BASE)
#define SC_HKADC_RD_REQ BIT(0)

#define REG_VUSB_ADC_L       REG_VUSB_ADC_L_ADDR(REG_ADC_BASE)
#define REG_ADC_DATA_BASE REG_VUSB_ADC_L
#define ADC_DATA_LEN 2

#define REG_DET_TOP_CFG_REG_6       0x966
#define ADC_DET_SEL_MASK            BIT(0)
#define VOL_DETECT_RANGE_3V_12V     0
#define VOL_DETECT_RANGE_3V_22V     1

#define CHG_INPUT_SOURCE_REG            (REG_BUCK_OTG_CFG_REG_0_ADDR(REG_ANA_BASE))
#define CHG_ILIMIT_SHIFT                (3)
#define CHG_ILIMIT_MSK                  (0x1f << CHG_ILIMIT_SHIFT)
#define CHG_ILIMIT_600MA                0x06
#define CHG_IBUS_DIV                    5

#define REG_SBU_DET_CFG           (REG_TYPE_CFG_REG_4_ADDR(REG_ANA_BASE))
#define SBU1_10UA_DET             (BIT(2)|BIT(1))
#define SBU2_10UA_DET             (BIT(4)|BIT(2)|BIT(0))
#define SBU_DET_DIS               (0)

#define REG_DP_DET_CFG            (REG_DET_TOP_CFG_REG_3_ADDR(REG_ANA_BASE))
#define DP_1UA_DET                (BIT(2))
#define DP_DET_DIS                (0)

/* NTC Table length */
#define T_V_ARRAY_LENGTH	31

#define CHG_ADC_CH_VUSB        0
#define CHG_ADC_CH_VPSW        1
#define CHG_ADC_CH_VBUS        2
#define CHG_ADC_CH_VBATL       3
#define CHG_ADC_CH_VBATH       4
#define CHG_ADC_CH_IBATL       5
#define CHG_ADC_CH_IBATH       6
#define CHG_ADC_CH_IBUS_REF    7
#define CHG_ADC_CH_L_MAX       8

#define CHG_ADC_CH_IBUS        (0 + CHG_ADC_CH_L_MAX)
#define CHG_ADC_CH_TDIE        (1 + CHG_ADC_CH_L_MAX)
#define CHG_ADC_CH_TSBATL      (2 + CHG_ADC_CH_L_MAX)
#define CHG_ADC_CH_TSBATH      (3 + CHG_ADC_CH_L_MAX)
#define CHG_ADC_CH_VEIS        (4 + CHG_ADC_CH_L_MAX)
#define CHG_ADC_CH_VCAL        (5 + CHG_ADC_CH_L_MAX)
#define CHG_ADC_CH_VDP         (6 + CHG_ADC_CH_L_MAX)
#define CHG_ADC_CH_SBU         (7 + CHG_ADC_CH_L_MAX)

#define REG_SCHG_LOGIC_CFG_REG_5     (0x9a5)
#define CLOSE_PART_ADC_CIRCUIT_SHIFT (7)
#define CLOSE_PART_ADC_CIRCUIT_MSK   (0x1 << CLOSE_PART_ADC_CIRCUIT_SHIFT)

int scharger_get_ibat_h(void);
int scharger_get_ibat_l(void);
int scharger_get_vbat_h(void);
int scharger_get_vbat_l(void);
int scharger_get_vbus(void);
int scharger_get_vpsw(void);
int scharger_get_vusb(void);
int scharger_get_tsbat_h(void);
int scharger_get_tsbat_l(void);
int scharger_get_tdie(void);
int scharger_get_ibus(void);
int scharger_adc_get_vbus(unsigned int *val);
int scharger_adc_get_vusb(int *val);
int scharger_adc_get_ibus(int *ibus);
int scharger_get_sbu1_res(void);
int scharger_get_sbu2_res(void);
int scharger_get_dp_res(void);

#endif
