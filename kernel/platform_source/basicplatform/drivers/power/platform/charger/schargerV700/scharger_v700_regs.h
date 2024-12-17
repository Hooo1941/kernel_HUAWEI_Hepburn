#ifndef __SCHARGER_V700_REGS_H__
#define __SCHARGER_V700_REGS_H__

#define CHG_ILIMIT_85                   85
#define CHG_ILIMIT_130                  130
#define CHG_ILIMIT_200                  200
#define CHG_ILIMIT_300                  300
#define CHG_ILIMIT_400                  400
#define CHG_ILIMIT_475                  475
#define CHG_ILIMIT_600                  600
#define CHG_ILIMIT_700                  700
#define CHG_ILIMIT_800                  800
#define CHG_ILIMIT_825                  825
#define CHG_ILIMIT_1000                 1000
#define CHG_ILIMIT_1100                 1100
#define CHG_ILIMIT_1200                 1200
#define CHG_ILIMIT_1300                 1300
#define CHG_ILIMIT_1400                 1400
#define CHG_ILIMIT_1500                 1500
#define CHG_ILIMIT_1600                 1600
#define CHG_ILIMIT_1700                 1700
#define CHG_ILIMIT_1800                 1800
#define CHG_ILIMIT_1900                 1900
#define CHG_ILIMIT_2000                 2000
#define CHG_ILIMIT_2100                 2100
#define CHG_ILIMIT_2200                 2200
#define CHG_ILIMIT_2300                 2300
#define CHG_ILIMIT_2400                 2400
#define CHG_ILIMIT_2500                 2500
#define CHG_ILIMIT_2600                 2600
#define CHG_ILIMIT_2700                 2700
#define CHG_ILIMIT_2800                 2800
#define CHG_ILIMIT_2900                 2900
#define CHG_ILIMIT_3000                 3000
#define CHG_ILIMIT_3100                 3100
#define CHG_ILIMIT_MAX                  (CHG_ILIMIT_3100)
#define CHG_ILIMIT_STEP_100             100
#define CHG_LMT_NUM_V700                32

#define CHG_VBAT_TH                     100

#define CHG_FAST_VCHG_REG               (REG_CHG_TOP_CFG_REG_8_ADDR(REG_ANA_BASE))
#define CHG_FAST_VCHG_SHIFT             (2)
#define CHG_FAST_VCHG_MSK               (0x3f << CHG_FAST_VCHG_SHIFT)
#define CHG_FAST_VCHG_STEP_8000UV       8000
#define CHG_FAST_VCHG_MIN               4152
#define CHG_FAST_VCHG_BASE_UV           (4152 * 1000)
#define CHG_FAST_VCHG_MAX               4656

#define CHG_FAST_VCHG_4250              4250
#define CHG_FAST_VCHG_4430              4430

#define VBAT_LV_H_REG                   (REG_VBAT_LV_H_REG_ADDR(REG_ANA_BASE))
#define VBAT_LV_H_SHIFT                 (0)
#define VBAT_LV_H_MASK                  (1 << VBAT_LV_H_SHIFT)
#define VBAT_LV_L_REG                   (REG_VBAT_LV_L_REG_ADDR(REG_ANA_BASE))
#define VBAT_LV_L_SHIFT                 (0)
#define VBAT_LV_L_MASK                  (1 << VBAT_LV_L_SHIFT)
#define VBAT_LV_SET1                    1

#define CHG_INPUT_SOURCE_REG            (REG_BUCK_OTG_CFG_REG_0_ADDR(REG_ANA_BASE))
#define CHG_ILIMIT_SHIFT                (3)
#define CHG_ILIMIT_MSK                  (0x1f << CHG_ILIMIT_SHIFT)

#define CHG_OTG_MODE_CFG_REG            (REG_CHG_OTG_MODE_CFG_ADDR(REG_GLB_BASE))
#define CHG_OTG_MODE_CFG_SHIFT          (0)
#define CHG_OTG_MODE_CFG_MSK            (0x3 << CHG_OTG_MODE_CFG_SHIFT)
#define DA_CHG_EN                       (0x01)
#define DA_CHG_DIS                      (0x00)
#define DA_CHG_OTG_EN                   (0x02)
#define DA_CHG_OTG_DIS                  (0x00)

#define CHG_VBUS_VSET_REG               (REG_DET_TOP_CFG_REG_5_ADDR(REG_ANA_BASE))
#define VBUS_VSET_SHIFT                 (REG_DET_TOP_CFG_REG_5_da_vbus_bkvset_START)
#define VBUS_VSET_MSK                   (0x03 << VBUS_VSET_SHIFT)
#define VBUS_VSET_5V                    5
#define VBUS_VSET_9V                    9
#define VBUS_VSET_12V                   12

#define REG_DET_TOP_CFG_REG_5           (REG_DET_TOP_CFG_REG_5_ADDR(REG_ANA_BASE))
#define CHG_BUCK_OVP_SET_SHIFT          (REG_DET_TOP_CFG_REG_5_da_vbus_ov_bk_START)
#define CHG_BUCK_OVP_SET_MSK            (0x03 << CHG_BUCK_OVP_SET_SHIFT)

#define CHG_UVP_VOLTAGE_REG             (REG_DET_TOP_CFG_REG_5_ADDR(REG_ANA_BASE))
#define CHG_BUCK_UVP_VOLTAGE_SHIFT      (REG_DET_TOP_CFG_REG_5_da_vbus_uv_bk_START)
#define CHG_BUCK_UVP_VOLTAGE_MSK        (0x03 << CHG_BUCK_UVP_VOLTAGE_SHIFT)

#define CHG_FAST_ICHG_100MA             100
#define CHG_FAST_ICHG_500MA             500
#define CHG_FAST_ICHG_2500MA            2500
#define CHG_FAST_ICHG_3000MA            3000
#define CHG_FAST_ICHG_MAX               3200

#define CHG_FAST_CURRENT_H_REG          (REG_CHG_TOP_CFG_REG_6_ADDR(REG_ANA_BASE))
#define CHG_FAST_ICHG_H_SHIFT           (REG_CHG_TOP_CFG_REG_6_da_chg_fast_ichg_h_START)
#define CHG_FAST_ICHG_H_MSK             (0x1f << CHG_FAST_ICHG_H_SHIFT)
#define CHG_FAST_CURRENT_L_REG          (REG_CHG_TOP_CFG_REG_7_ADDR(REG_ANA_BASE))
#define CHG_FAST_ICHG_L_SHIFT           (REG_CHG_TOP_CFG_REG_7_da_chg_fast_ichg_l_START)
#define CHG_FAST_ICHG_L_MSK             (0x1f << CHG_FAST_ICHG_L_SHIFT)
#define CHG_FAST_ICHG_STEP_100          100
#define CHG_FAST_ICHG_00MA              0

#define CHG_TERM_ICHG_REG               (REG_CHG_TOP_CFG_REG_20_ADDR(REG_ANA_BASE))
#define CHG_TERM_ICHG_SHIFT             (REG_CHG_TOP_CFG_REG_20_da_chg_term_ichg_START)
#define CHG_TERM_ICHG_MSK               (0x03 << CHG_TERM_ICHG_SHIFT)
#define CHG_TERM_ICHG_150MA             150
#define CHG_TERM_ICHG_200MA             200
#define CHG_TERM_ICHG_300MA             300
#define CHG_TERM_ICHG_400MA             400

#define CHG_TOP_RO_REG24_STATE          (REG_CHG_TOP_RO_REG_24_ADDR(REG_ANA_BASE))
#define CHG_IN_DPM_STATE                (0x01 << REG_CHG_TOP_RO_REG_24_ad_chg_dpm_state_START)
#define CHG_IN_ACL_STATE                (0x01 << REG_CHG_TOP_RO_REG_24_ad_chg_acl_state_START)

#define CHG_ACL_RPT_EN_REG              (REG_CHG_TOP_CFG_REG_1_ADDR(REG_ANA_BASE))
#define CHG_ACL_RPT_EN_SHIFT            (REG_CHG_TOP_CFG_REG_1_da_chg_acl_rpt_en_START)
#define CHG_ACL_PRT_EN_MASK             (0x01 << CHG_ACL_RPT_EN_SHIFT)

#define CHG_TOP_RO_REG26_STATE          (REG_CHG_TOP_RO_REG_26_ADDR(REG_ANA_BASE))
#define CHG_IN_THERM_STATE              (0x01 << REG_CHG_TOP_RO_REG_26_ad_chg_therm_state_START)

#define CHG_EN_TERM_REG                 (REG_CHG_TOP_CFG_REG_5_ADDR(REG_ANA_BASE))
#define CHG_EN_TERM_SHIFT               (REG_CHG_TOP_CFG_REG_5_da_chg_en_term_START)
#define CHG_EN_TERM_MSK                 (0x01 << CHG_EN_TERM_SHIFT)
#define CHG_TERM_EN                     1
#define CHG_TERM_DIS                    0

#define CHG_STAT_ENABLE                 1
#define CHG_STAT_DISABLE                0

#define CHG_BUCK_STATUS_REG             (REG_BUCK_OTG_RO_REG_30_ADDR(REG_ANA_BASE))
#define CHG_BUCK_OK                     (0x01 << 1)

#define CHG_STAT_CHARGE_DONE_H          (0x03 << REG_CHG_TOP_RO_REG_24_ad_chg_chgstate_h_START)
#define CHG_STAT_CHARGE_DONE_L          (0x03 << REG_CHG_TOP_RO_REG_24_ad_chg_chgstate_l_START)
#define CHG_STAT_L_SHIFT                (REG_CHG_TOP_RO_REG_24_ad_chg_chgstate_l_START)
#define CHG_STAT_L_MASK                 (0x03 << CHG_STAT_L_SHIFT)

#define WATCHDOG_STATUS_REG             (REG_WDT_TIMEOUT_ADDR(REG_GLB_BASE))
#define WATCHDOG_OK                     (0x01 << REG_WDT_TIMEOUT_wdt_time_out_n_START)

/* anti-reverbst */
#define CHG_ANTI_REVERBST_REG           (REG_BUCK_OTG_CFG_REG_2_ADDR(REG_ANA_BASE))
#define CHG_ANTI_REVERBST_EN_SHIFT      7
#define CHG_ANTI_REVERBST_EN_MSK        (1 << CHG_ANTI_REVERBST_EN_SHIFT)
#define CHG_ANTI_REVERBST_EN            1
#define CHG_ANTI_REVERBST_DIS           0

#define VUSB_UV_DET_ENB_REG             (REG_USB_OVP_CFG_REG_3_ADDR(REG_ANA_BASE))
#define VUSB_UV_DET_ENB_SHIFT           REG_USB_OVP_CFG_REG_3_da_usb_uvlo_en_START
#define VUSB_UV_DET_ENB_MASK            (1 << VUSB_UV_DET_ENB_SHIFT)

#define CHG_OTG_SWITCH_CFG_REG          (REG_BUCK_OTG_CFG_REG_27_ADDR(REG_ANA_BASE))
#define CHG_OTG_SWITCH_SHIFT            (REG_BUCK_OTG_CFG_REG_27_da_otg_switch_START)
#define CHG_OTG_SWITCH_MASK             (1 << CHG_OTG_SWITCH_SHIFT)

#define CHG_OTG_REG0                    (REG_BUCK_OTG_CFG_REG_21_ADDR(REG_ANA_BASE))
#define CHG_OTG_LIM_SHIFT               (REG_BUCK_OTG_CFG_REG_21_da_otg_lim_set_START)
#define CHG_OTG_LIM_MSK                 (0x03 << CHG_OTG_LIM_SHIFT)
#define BOOST_LIM_MIN                   500
#define BOOST_LIM_500                   500
#define BOOST_LIM_1000                  1000
#define BOOST_LIM_1500                  1500
#define BOOST_LIM_2000                  2000
#define BOOST_LIM_2700                  2700
#define BOOST_LIM_MAX                   2700

#define CHG_HIZ_CTRL_REG                (REG_SC_BUCK_ENB_ADDR(REG_GLB_BASE))
#define CHG_HIZ_ENABLE_SHIFT            (REG_SC_BUCK_ENB_sc_buck_enb_START)
#define CHG_HIZ_ENABLE_MSK              (1 << CHG_HIZ_ENABLE_SHIFT)

#define WEAKSOURCE_FLAG_REG             PMIC_HRST_REG12_ADDR(0)
#define WAEKSOURCE_FLAG                 BIT(2)

#define IBAT_RES_SEL_REG                (REG_IBAT_RES_SEL_ADDR(REG_ANA_BASE))
#define IBAT_RES_SEL_SHIFT              (REG_IBAT_RES_SEL_da_bat_res_sel_START)
#define IBAT_RES_SEL_MASK               (1 << IBAT_RES_SEL_SHIFT)

#define CHG_DPM_MODE_REG                (REG_BUCK_OTG_CFG_REG_5_ADDR(REG_ANA_BASE))
#define CHG_DPM_MODE_SHIFT              (REG_BUCK_OTG_CFG_REG_5_da_buck_dpm_auto_START)
#define CHG_DPM_MODE_MSK                (1 << CHG_DPM_MODE_SHIFT)
#define CHG_DPM_MODE_AUTO               (1)

#define CHG_DPM_SEL_REG                 (REG_BUCK_OTG_CFG_REG_6_ADDR(REG_ANA_BASE))
#define CHG_DPM_SEL_SHIFT               (REG_BUCK_OTG_CFG_REG_6_da_buck_dpm_sel_START)
#define CHG_DPM_SEL_MSK                 (0x0F << CHG_DPM_SEL_SHIFT)
#define CHG_DPM_SEL_DEFAULT             0x06

#define CHG_RECHG_REG                   (REG_CHG_TOP_CFG_REG_21_ADDR(REG_ANA_BASE))
#define CHG_RECHG_SHIFT                 (REG_CHG_TOP_CFG_REG_21_da_chg_vrechg_hys_START)
#define CHG_RECHG_MSK                   (0x03 << CHG_RECHG_SHIFT)
#define CHG_RECHG_150                   0x00
#define CHG_RECHG_250                   0x01
#define CHG_RECHG_350                   0x02
#define CHG_RECHG_450                   0x03

#define CHG_PRE_ICHG_REG                (REG_CHG_TOP_CFG_REG_11_ADDR(REG_ANA_BASE))
#define CHG_PRE_ICHG_H_SHIFT            (REG_CHG_TOP_CFG_REG_11_da_chg_pre_ichg_h_START)
#define CHG_PRE_ICHG_H_MSK              (0x03 << CHG_PRE_ICHG_H_SHIFT)
#define CHG_PRE_ICHG_L_SHIFT            (REG_CHG_TOP_CFG_REG_11_da_chg_pre_ichg_l_START)
#define CHG_PRE_ICHG_L_MSK              (0x03 << CHG_PRE_ICHG_L_SHIFT)
#define CHG_PRG_ICHG_STEP               100
#define CHG_PRG_ICHG_MIN                100
#define CHG_PRG_ICHG_100MA              100
#define CHG_PRG_ICHG_200MA              200
#define CHG_PRG_ICHG_300MA              300
#define CHG_PRG_ICHG_400MA              400
#define CHG_PRG_ICHG_MAX                400

#define CHG_PRE_VCHG_SHIFT              (REG_CHG_TOP_CFG_REG_11_da_chg_pre_vchg_START)
#define CHG_PRE_VCHG_MSK                (0x03 << CHG_PRE_VCHG_SHIFT)
#define CHG_PRG_VCHG_2800               2800
#define CHG_PRG_VCHG_3000               3000
#define CHG_PRG_VCHG_3100               3100
#define CHG_PRG_VCHG_3200               3200

#define CHG_FASTCHG_TIMER_REG           (REG_CHG_TOP_CFG_REG_8_ADDR(REG_ANA_BASE))
#define CHG_FASTCHG_TIMER_SHIFT         (REG_CHG_TOP_CFG_REG_8_da_chg_fastchg_timer_START)
#define CHG_FASTCHG_TIMER_MSK           (0x03 << CHG_FASTCHG_TIMER_SHIFT)
#define CHG_FASTCHG_TIMER_5H            0
#define CHG_FASTCHG_TIMER_7H            1
#define CHG_FASTCHG_TIMER_12H           2
#define CHG_FASTCHG_TIMER_20H           3

#define CHG_IR_COMP_REG                 (REG_CHG_TOP_CFG_REG_9_ADDR(REG_ANA_BASE))
#define CHG_IR_COMP_H_SHIFT             (REG_CHG_TOP_CFG_REG_9_da_chg_ir_set_h_START)
#define CHG_IR_COMP_H_MSK               (0x07 << CHG_IR_COMP_H_SHIFT)
#define CHG_IR_COMP_L_SHIFT             (REG_CHG_TOP_CFG_REG_9_da_chg_ir_set_l_START)
#define CHG_IR_COMP_L_MSK               (0x07 << CHG_IR_COMP_L_SHIFT)

#define CHG_BATL_IR_COMP_REG2           (REG_CHG_TOP_CFG_REG_17_ADDR(REG_ANA_BASE))
#define CHG_BATH_IR_COMP_REG2           (REG_CHG_TOP_CFG_REG_18_ADDR(REG_ANA_BASE))
#define CHG_IR_COMP_SHIFT2             (2)
#define CHG_IR_COMP_MSK2               (0x03 << CHG_IR_COMP_SHIFT2)

#define CHG_IR_COMP_MIN                 0
#define CHG_IR_COMP_MAX                 100
#define CHG_IR_COMP_0MOHM               0
#define CHG_IR_COMP_10MOHM              10
#define CHG_IR_COMP_20MOHM              20
#define CHG_IR_COMP_30MOHM              30
#define CHG_IR_COMP_40MOHM              40
#define CHG_IR_COMP_60MOHM              60
#define CHG_IR_COMP_80MOHM              80
#define CHG_IR_COMP_100MOHM             100

#define CHG_IR_VCLAMP_H_REG             (REG_CHG_TOP_CFG_REG_20_ADDR(REG_ANA_BASE))
#define CHG_IR_VCLAMP_H_SHIFT           (REG_CHG_TOP_CFG_REG_20_da_chg_vclamp_set_h_START)
#define CHG_IR_VCLAMP_H_MSK             (0x07 << CHG_IR_VCLAMP_H_SHIFT)
#define CHG_IR_VCLAMP_L_REG             (REG_CHG_TOP_CFG_REG_21_ADDR(REG_ANA_BASE))
#define CHG_IR_VCLAMP_L_SHIFT           (REG_CHG_TOP_CFG_REG_21_da_chg_vclamp_set_l_START)
#define CHG_IR_VCLAMP_L_MSK             (0x07 << CHG_IR_VCLAMP_L_SHIFT)
#define CHG_IR_VCLAMP_STEP              50
#define CHG_IR_VCLAMP_MIN               0
#define CHG_IR_VCLAMP_MAX               350

#define BATFET_H_CTRL_CFG_REG           (REG_CHARGER_BATFET_H_CTRL_ADDR(REG_ANA_BASE))
#define BATFET_H_CTRL_CFG_SHIFT         (REG_CHARGER_BATFET_H_CTRL_batfet_h_ctrl_START)
#define BATFET_H_CTRL_CFG_MSK           (1 << BATFET_H_CTRL_CFG_SHIFT)

#define BATFET_L_CTRL_CFG_REG           (REG_CHARGER_BATFET_L_CTRL_ADDR(REG_ANA_BASE))
#define BATFET_L_CTRL_CFG_SHIFT         (REG_CHARGER_BATFET_L_CTRL_batfet_l_ctrl_START)
#define BATFET_L_CTRL_CFG_MSK           (1 << BATFET_L_CTRL_CFG_SHIFT)

#define SC_DBFET_SEL_REG                (REG_DBFET_SEL_ADDR(REG_GLB_BASE))
#define SC_DBFET_SEL_SHIFT              (REG_DBFET_SEL_sc_dbfet_sel_START)
#define SC_DBFET_SEL_MSK                (1 << SC_DBFET_SEL_SHIFT)
#define SC_DBFET_SEL_EN_REG             (REG_DBFET_SEL_EN_ADDR(REG_GLB_BASE))
#define SC_DBFET_SEL_EN_SHIFT           (REG_DBFET_SEL_EN_sc_dbfet_sel_en_START)
#define SC_DBFET_SEL_EN_MSK             (1 << SC_DBFET_SEL_EN_SHIFT)

#define CHG_STATUS_REG                  (REG_CHG_TOP_RO_REG_24_ADDR(REG_ANA_BASE))
#define CHG_STAT_H_CHARGE_DONE          (0x03 << REG_CHG_TOP_RO_REG_24_ad_chg_chgstate_h_START)
#define CHG_STAT_H_SHIFT                (REG_CHG_TOP_RO_REG_24_ad_chg_chgstate_h_START)
#define CHG_STAT_H_MASK                 (0x03 << CHG_STAT_H_SHIFT)

#define REG_DET_TOP_CFG_REG_19          (REG_CHG_TOP_CFG_REG_19_ADDR(REG_ANA_BASE))
#define CHG_CHG_RPT_EN_SHIFT            (REG_CHG_TOP_CFG_REG_19_da_chg_rpt_en_START)
#define CHG_CHG_RPT_EN_MSK              (0x01 << REG_CHG_TOP_CFG_REG_19_da_chg_rpt_en_START)

#define CHG_STAT_L_CHARGE_DONE          (0x03 << REG_CHG_TOP_RO_REG_24_ad_chg_chgstate_l_START)
#define CHG_STAT_L_SHIFT                (REG_CHG_TOP_RO_REG_24_ad_chg_chgstate_l_START)
#define CHG_STAT_L_MASK                 (0x03 << CHG_STAT_L_SHIFT)

#define CHG_TERM_DONE                   3

#define OTG_DMD_CFG_REG                 (REG_OTG_DMD_CFG_ADDR(REG_GLB_BASE))
#define OTG_DMD_OFS_SHIFT               (REG_OTG_DMD_CFG_sc_otg_dmd_ofs_START)
#define OTG_DMD_OFS_MSK                 (0xF << OTG_DMD_OFS_SHIFT)
#define OTG_DMD_RAMP_SHIFT              (REG_OTG_DMD_CFG_sc_otg_dmd_ramp_START)
#define OTG_DMD_RAMP_MSK                (0x7 << OTG_DMD_RAMP_SHIFT)
#define OTG_DMD_RAMP_DEFAULT            0

#define DMD_VOL_0MV                     (0x9)

#define OVP_MODE_REG                    (REG_OVP_MODE_ADDR(REG_GLB_BASE))
#define OVP_MODE_SHIFT                  (REG_OVP_MODE_sc_ovp_mode_START)
#define OVP_MODE_MSK                    (1 << OVP_MODE_SHIFT)
#define OVP_POSITIVE_MODE               0
#define OVP_REVERSE_MODE                1

#define OVP_EN_REG                      (REG_OVP_EN_ADDR(REG_GLB_BASE))
#define OVP_EN_SHIFT                    (REG_OVP_EN_sc_ovp_en_START)
#define OVP_EN_MSK                      (1 << OVP_EN_SHIFT)
#define OVP_ENABLE                      1
#define OVP_DISABLE                     0

#define PSW_MODE_REG                    (REG_PSW_MODE_ADDR(REG_GLB_BASE))
#define PSW_MODE_SHIFT                  (REG_PSW_MODE_sc_psw_mode_START)
#define PSW_MODE_MSK                    (1 << PSW_MODE_SHIFT)
#define PSW_POSITIVE_MODE               0
#define PSW_REVERSE_MODE                1

#define PSW_EN_REG                      (REG_PSW_EN_ADDR(REG_GLB_BASE))
#define PSW_EN_SHIFT                    (REG_PSW_EN_sc_psw_en_START)
#define PSW_EN_MSK                      (1 << PSW_EN_SHIFT)
#define PSW_ENABLE                      1
#define PSW_DISABLE                     0

#define PSW_OVP_CFG_REG_1               (REG_PSW_OVP_CFG_REG_1_ADDR(REG_ANA_BASE))
#define OVP3_DROPOVP_MODE_SHIFT         (2)
#define OVP3_DROPOVP_MODE_MSK           (0x1 << OVP3_DROPOVP_MODE_SHIFT)
#define OVP2_EN_SHIFT                   (3)
#define OVP2_EN_MSK                     (0x1 << OVP2_EN_SHIFT)
#define OVP2_DROPOVP_MIN_EN_SHIFT       (5)
#define OVP2_DROPOVP_MIN_EN_MSK         (0x1 << OVP2_DROPOVP_MIN_EN_SHIFT)
#define OVP3_EN_SHIFT                   (4)
#define OVP3_EN_MSK                     (0x1 << OVP3_EN_SHIFT)
#define OVP3_DROPOVP_MIN_EN_SHIFT       (6)
#define OVP3_DROPOVP_MIN_EN_MSK         (0x1 << OVP3_DROPOVP_MIN_EN_SHIFT)
#define OVP3_ENABLE                     1
#define OVP3_DISABLE                    0

#define CHG_IRQ_ADDR                    (REG_IRQ_FLAG_ADDR(REG_IRQ_BASE))
#define CHG_OTHERS_IRQ                  (BIT(2))
#define CHG_OVP_PSW_IRQ                 (BIT(4))
#define CHG_BUCK_IRQ                    (BIT(5))
#define CHG_BUCK_IRQ_ADDR               (REG_IRQ_FLAG_0_ADDR(REG_IRQ_BASE))
#define CHG_OVP_PSW_IRQ_ADDR            (REG_IRQ_FLAG_4_ADDR(REG_IRQ_BASE))
#define CHG_OTHERS_IRQ_ADDR             (REG_IRQ_FLAG_7_ADDR(REG_IRQ_BASE))
#define BUCK_IRQ_MASK                   0xFFFFFF01
#define BUCK_IRQ_BULK_NUM               4
#define OVP_PSW_IRQ_BULK_NUM            2
#define OTHERS_IRQ_BULK_NUM             4

#define BUF_MAX_512

#define FAULT_OTG_OCP                   0x40000000
#define FAULT_OTG_SCP                   0x80000000
#define FAULT_REVERSBST                 0x20000000
#define FAULT_CHG_DONE                  0x00001000
#define FAULT_CHG_FAULT                 0x00002000
#define FAULT_RECHG                     0x00000800
#define FAULT_BATSW1_SCP                0x00100000
#define FAULT_BATSW2_SCP                0x00080000
#define FAULT_BATSW1_OPEN               0x00040000
#define FAULT_BATSW2_OPEN               0x00020000

#define OTHERS_RCPF_DET_IRQ             0x08000000
#define OTHERS_VBATH_LV_IRQ             0x00400000
#define VBATH_LV_TIMEOUT                200

#define REVERBST_RETRY                  300

#define OTHERS_VBATL_OVP_MASK           0x000200
#define OTHERS_VBATH_OVP_MASK           0x000100
#define OTHERS_VBATL_LV_IRQ_MASK        0x800000
#define OTHERS_VBATH_LV_IRQ_MASK        0x400000

#define CHG_IRQ_MASK_ALL_ADDR           (REG_IRQ_MASK_ADDR(REG_IRQ_BASE))
#define CHG_IRQ_MASK_ALL_SHIFT          0
#define CHG_IRQ_MASK_BUCK_MSK           (0x24 << CHG_IRQ_MASK_ALL_SHIFT)

#define WAIT_PD_IRQ_TIME                10
#define IRQ_RETRY_TIME                  500

#define CHG_IRQ_STATUS_8                (REG_IRQ_STATUS_8_ADDR(REG_IRQ_BASE))
#define CHG_VBATH_OVP                   (BIT(0))
#define CHG_VBATL_OVP                   (BIT(1))

#define REG_USB_OVP_CFG_REG_2           (REG_USB_OVP_CFG_REG_2_ADDR(REG_ANA_BASE))
#define CHG_USB_OVP_SET_SHIFT           (REG_USB_OVP_CFG_REG_2_da_usb_ovp_set_START)
#define CHG_USB_OVP_SET_MSK             (0x07 << CHG_USB_OVP_SET_SHIFT)
#define USB_OVP_24V                     24
#define USB_OVP_23V                     23
#define USB_OVP_22V                     22
#define USB_OVP_18V                     18
#define USB_OVP_15V                     15
#define USB_OVP_14V                     14
#define USB_OVP_11V                     11

#define PSW_OVP_CFG_REG_2              (REG_PSW_OVP_CFG_REG_2_ADDR(REG_ANA_BASE))
#define OVP2_DROPOVP_MODE_SHIFT        (6)
#define OVP2_DROPOVP_MODE_MSK          (0x1 << OVP2_DROPOVP_MODE_SHIFT)
#define PSW_PLUGIN_EN_SHIFT            (REG_PSW_OVP_CFG_REG_2_da_psw_plugin_en_START)
#define PSW_PLUGIN_EN_MSK              (0x1 << PSW_PLUGIN_EN_SHIFT)
#define PSW_PLUGIN_DIS                 0
#define PSW_PLUGIN_EN                  1

#define PSW_OVP_SET_SHIFT              (REG_PSW_OVP_CFG_REG_2_da_psw_ovp_set_START)
#define PSW_OVP_SET_MSK                (0x7 << PSW_OVP_SET_SHIFT)
#define PSW_OVP_14V                    (0x2)

#define USB_PLUGIN_EN_SHIFT            (REG_USB_OVP_CFG_REG_2_da_usb_plugin_en_START)
#define USB_PLUGIN_EN_MSK              (0x1 << USB_PLUGIN_EN_SHIFT)
#define USB_PLUGIN_DIS                 0
#define USB_PLUGIN_EN                  1

#define BAT_SERIES_MODE                 1
#define BAT_PARALLEL_MODE               0

#define CHG_PSEL_TOP_CFG_REG_2_REG      (REG_PSEL_TOP_CFG_REG_2_ADDR(REG_ANA_BASE))
#define OTG_REGN_FORCE_CLOSE_EN_SHIFT   (5)
#define OTG_REGN_FORCE_CLOSE_EN_MSK     (0x1 << OTG_REGN_FORCE_CLOSE_EN_SHIFT)
#define CHG_BUCK_OTG_CFG_REG_34_REG     (REG_BUCK_OTG_CFG_REG_34_ADDR(REG_ANA_BASE))
#define OTG_REGN_EN_SHIFT               (2)
#define OTG_REGN_EN_MSK                 (0x1 << OTG_REGN_EN_SHIFT)

#define REG_CHG_TOP_CFG_REG_18         (REG_CHG_TOP_CFG_REG_18_ADDR(REG_ANA_BASE))
#define BATH_MINUTE_ADJUST_CV_SHIFT          (4)
#define BATH_MINUTE_ADJUST_CV_MSK            (0x7 << BATH_MINUTE_ADJUST_CV_SHIFT)
#define BATH_MINUTE_ADJUST_CV_SIGN_SHIFT     (7)
#define BATH_MINUTE_ADJUST_CV_SIGN_MSK       (0x1 << BATH_MINUTE_ADJUST_CV_SIGN_SHIFT)

#define REG_CHG_TOP_CFG_REG_17         (REG_CHG_TOP_CFG_REG_17_ADDR(REG_ANA_BASE))
#define BATL_MINUTE_ADJUST_CV_SHIFT          (4)
#define BATL_MINUTE_ADJUST_CV_MSK            (0x7 << BATL_MINUTE_ADJUST_CV_SHIFT)
#define BATL_MINUTE_ADJUST_CV_SIGN_SHIFT     (7)
#define BATL_MINUTE_ADJUST_CV_SIGN_MSK       (0x1 << BATL_MINUTE_ADJUST_CV_SIGN_SHIFT)

#define SCHG_LOGIC_CFG_REG_1              (REG_SCHG_LOGIC_CFG_REG_1_ADDR(REG_ANA_BASE))
#define BATFET_FORCE_CONDUCTION_SHIFT     (6)
#define BATFET_FORCE_CONDUCTION_MSK       (0x1 << BATFET_FORCE_CONDUCTION_SHIFT)
#define BATFET_CONDUCTION_ENABLE          1
#define BATFET_CONDUCTION_DISABLE         0

#define EFUSE_TESTBUS_CFG_REG             (REG_EFUSE_TESTBUS_CFG_ADDR(REG_GLB_BASE))
#define EFUSE_TESTBUS_EN_SHIFT            (REG_EFUSE_TESTBUS_CFG_sc_efuse_testbus_en_START)
#define EFUSE_TESTBUS_EN_MSK              (0x1 << EFUSE_TESTBUS_EN_SHIFT)
#define EFUSE_TESTBUS_SEL_SHIFT           (REG_EFUSE_TESTBUS_CFG_sc_efuse1_testbus_sel_START)
#define EFUSE_TESTBUS_SEL_MSK             (0x7f << EFUSE_TESTBUS_SEL_SHIFT)
#define EFUSE_TESTBUS_RDATA_REG           (REG_EFUSE_TESTBUS_RDATA_ADDR(REG_GLB_BASE))
#define EFUSE_RD_ENABLE                   1
#define EFUSE_RD_DISABLE                  0

#define EFUSE_PDOB_SEL_REG                 (REG_EFUSE_PDOB_SEL_ADDR(REG_GLB_BASE))
#define EFUSE_PDOB_SEL_SHIFT               (REG_EFUSE_PDOB_SEL_sc_efuse_pdob_sel_START)
#define EFUSE_PDOB_SEL_MSK                 (0x01 << EFUSE_PDOB_SEL_SHIFT)
#define EFUSE_PDOB_PRE_ADDR_WE_REG         (REG_EFUSE_PDOB_PRE_ADDR_WE_ADDR(REG_GLB_BASE))
#define EFUSE_PDOB_PRE_ADDR_SHIFT          (REG_EFUSE_PDOB_PRE_ADDR_WE_sc_efuse_pdob_pre_addr_START)
#define EFUSE_PDOB_PRE_ADDR_MSK            (0x3f << EFUSE_PDOB_PRE_ADDR_SHIFT)
#define EFUSE_PDOB_PRE_WR_EN_SHIFT         (REG_EFUSE_PDOB_PRE_ADDR_WE_sc_efuse_pdob_pre_we_START)
#define EFUSE_PDOB_PRE_WR_EN_MSK           (0x1 << EFUSE_PDOB_PRE_WR_EN_SHIFT)

#define EFUSE_PDOB_PRE_WDATA_REG           (REG_EFUSE_PDOB_PRE_WDATA_ADDR(REG_GLB_BASE))
#define EFUSE_PDOB_PRE_WDATA_SHIFT         (REG_EFUSE_PDOB_PRE_WDATA_sc_efuse_pdob_pre_wdata_START)
#define EFUSE_PDOB_PRE_WDATA_MSK           (0xff << EFUSE_PDOB_PRE_WDATA_SHIFT)
#define EFUSE_WR_ENABLE                    0x01
#define EFUSE_WR_DISABLE                   0x00
#define EFUSE_WR_START                     0x01
#define EFUSE_WR_END                       0x00
#define MASK_RESET_N_EFUSE_ADDR            0x07
#define DISABLE_MASK_RESET_N               (0x01 << 7)
#define DISABLE_MASK_RESET_N_MSK           (0x01 << 7)

#define REG_SHIP_MODE_ENB                  (REG_SHIP_MODE_ENB_ADDR(REG_ANA_BASE))
#define SHIP_MODE_ENB_SHIFT                (REG_SHIP_MODE_ENB_ship_mode_enb_START)
#define SHIP_MODE_ENB_MSK                  (0x1 << SHIP_MODE_ENB_SHIFT)

#define REG_LOWBAT_EN                      (REG_LOWBAT_EN_ADDR(REG_GLB_BASE))
#define LOWBAT_EN_SHIFT                    (REG_LOWBAT_EN_da_lowbat_en_START)
#define LOWBAT_EN_MSK                      (0x1 << LOWBAT_EN_SHIFT)
#define BOOST_ENABLE                       1
#define BOOST_DISABLE                      0

#define REG_DPM_ENB                        (REG_BUCK_OTG_CFG_REG_12_ADDR(REG_ANA_BASE))
#define DPM_EN_SHIFT                       1
#define DPM_EN_MASK                        (0x1 << DPM_EN_SHIFT)

#define SCHG_LOGIC_CFG_REG_1              (REG_SCHG_LOGIC_CFG_REG_1_ADDR(REG_ANA_BASE))
#define QSW2_RCP_FAST_PROTECT_SHIFT       (2)
#define QSW2_RCP_FAST_PROTECT_MSK         (0x1 << QSW2_RCP_FAST_PROTECT_SHIFT)
#define QSW2_RCP_SLOW_PROTECT_SHIFT       (3)
#define QSW2_RCP_SLOW_PROTECT_MSK         (0x1 << QSW2_RCP_SLOW_PROTECT_SHIFT)
#define QSW2_RCP_PROTECT_ENANLE           1
#define QSW2_RCP_PROTECT_DISABLE          0

#define OTHERS_TBAT1_OTP_IRQ              0x00020000
#define OTHERS_TBAT2_OTP_IRQ              0x00040000
#define OTHERS_TDIE_OTP_IRQ               0x00200000
#define OTHERS_WGT_TOT_IRQ                0x00010000

#define OVP_PSW_VUSB_OVP_IRQ              0x0020
#define OTHERS_OVP2_DROPMIN_IRQ           0x80000000

#define BAT_SEL_REG                       (REG_BAT_SEL_STATUS_ADDR(REG_GLB_BASE))
#endif
