#ifndef __MNTN_SUBTYPE_EXCEPTION_H__
#define __MNTN_SUBTYPE_EXCEPTION_H__ 
#include "pmic_interface.h"
#define PMU_EXCSUBTYPE_REG_OFFSET (PMIC_HRST_REG10_ADDR(0))
enum mmc_exception_subtype
{
 MMC_EXCEPT_INIT_FAIL = 0x0,
 MMC_EXCEPT_CMD_TIMEOUT,
 MMC_EXCEPT_COLDBOOT,
};
enum ddrc_sec_subtype {
 L1BUS_UNKNOWN_MASTER = 0x0,
 L1BUS_LPMCU,
 L1BUS_IOMCU_M7,
 L1BUS_PCIE_1,
 L1BUS_PERF_STAT,
 L1BUS_MODEM,
 L1BUS_DJTAG_M,
 L1BUS_IOMCU_DMA,
 L1BUS_UFS,
 L1BUS_SD,
 L1BUS_HSDT,
 L1BUS_DDR,
 L1BUS_DPC,
 L1BUS_USB31OTG,
 L1BUS_ASP_DMA,
 L1BUS_DMAC,
 L1BUS_ASP_HIFI,
 L1BUS_PCIE_0,
 L1BUS_HSDT_TCU,
 L1BUS_POWER_STAT,
 L1BUS_SEC_HASH,
 L1BUS_DDR_LPCTRL,
 L1BUS_SDIO,
 L1BUS_EPS_SCE1,
 L1BUS_EPS_SCE2,
 L1BUS_EPS_SEC_REE,
 L1BUS_NPU,
 L1BUS_FCM,
 L1BUS_GPU,
 L1BUS_IPP_JPGENC,
 L1BUS_IPP_JPGDEC,
 L1BUS_IPP_VBK,
 L1BUS_IPP_GF,
 L1BUS_IPP_SLAM,
 L1BUS_IPP_CORE,
 L1BUS_VDEC,
 L1BUS_VENC,
 L1BUS_DSS,
 L1BUS_ISP,
 L1BUS_IDI2AXI,
 L1BUS_MEDIA_TCU,
 L1BUS_IVP,
};
enum appanic_subtype
{
 HI_APPANIC_RESERVED = 0x0,
 HI_APPANIC_BC_PANIC = 0x1,
 HI_APPANIC_L3CACHE_ECC1 = 0x2,
 HI_APPANIC_SOFTLOCKUP = 0x3,
 HI_APPANIC_OHARDLOCKUP = 0x4,
 HI_APPANIC_HARDLOCKUP = 0x5,
 HI_APPANIC_L3CACHE_ECC2 = 0x6,
 HI_APPANIC_Storage = 0x7,
 HI_APPANIC_ISP = 0x9,
 HI_APPANIC_IVP = 0xa,
 HI_APPANIC_GPU = 0xc,
 HI_APPANIC_MODEM = 0xd,
 HI_APPANIC_CPU0_CE = 0x10,
 HI_APPANIC_CPU0_UE = 0x11,
 HI_APPANIC_CPU1_CE = 0x12,
 HI_APPANIC_CPU1_UE = 0x13,
 HI_APPANIC_CPU2_CE = 0x14,
 HI_APPANIC_CPU2_UE = 0x15,
 HI_APPANIC_CPU3_CE = 0x16,
 HI_APPANIC_CPU3_UE = 0x17,
 HI_APPANIC_CPU4_CE = 0x18,
 HI_APPANIC_CPU4_UE = 0x19,
 HI_APPANIC_CPU5_CE = 0x1a,
 HI_APPANIC_CPU5_UE = 0x1b,
 HI_APPANIC_CPU6_CE = 0x1c,
 HI_APPANIC_CPU6_UE = 0x1d,
 HI_APPANIC_CPU7_CE = 0x1e,
 HI_APPANIC_CPU7_UE = 0x1f,
 HI_APPANIC_CPU8_CE = 0x20,
 HI_APPANIC_CPU8_UE = 0x21,
 HI_APPANIC_CPU9_CE = 0x22,
 HI_APPANIC_CPU9_UE = 0x23,
 HI_APPANIC_CPU10_CE = 0x24,
 HI_APPANIC_CPU10_UE = 0x25,
 HI_APPANIC_CPU11_CE = 0x26,
 HI_APPANIC_CPU11_UE = 0x27,
 HI_APPANIC_CPU01_L2_CE = 0x28,
 HI_APPANIC_CPU01_L2_UE = 0x29,
 HI_APPANIC_CPU23_L2_CE = 0x2a,
 HI_APPANIC_CPU23_L2_UE = 0x2b,
 HI_APPANIC_L3_CE = 0x30,
 HI_APPANIC_L3_UE = 0x31,
 HI_APPANIC_LB = 0x32,
 HI_APPANIC_PLL_UNLOCK = 0x33,
 HI_APPANIC_EARLY_PANIC = 0x34,
 HI_APPANIC_IO_DIE_STS = 0x35,
};
enum apbl31panic_subtype
{
 HI_APBL31PANIC_RESERVED = 0x0,
 HI_APBL31PANIC_ASSERT = 0x1,
};
enum apvndpanic_subtype
{
 HI_APVNDPANIC_RESERVED = 0x0,
 HI_APVNDPANIC_CFI = 0x1,
};
enum apwdt_subtype
{
 HI_APWDT_HW = 0x0,
 HI_APWDT_LPM3 = 0x1,
 HI_APWDT_BL31 = 0x2,
 HI_APWDT_BL31LPM3 = 0x3,
 HI_APWDT_AP = 0x4,
 HI_APWDT_BL31AP = 0x6,
 HI_APWDT_APBL31LPM3 = 0x7,
};
enum lpm3_subtype
{
 PSCI_RESERVED = 0x0,
 PSCI_AP_WDT_LOCAL,
 PSCI_AP_WDT_REMOTE,
 PSCI_M3_WDT_LOCAL,
 PSCI_M3_WDT_REMOTE,
 PSCI_REASON_WDT_END,
 PSCI_AP_SYS_PANIC = PSCI_REASON_WDT_END,
 PSCI_M3_SYS_PANIC,
 PSCI_PMUSSI_PANIC,
 PSCI_CLK_ERR,
 PSCI_REGULATOR_ERR,
 PSCI_ASYNC_EXCEPTION,
 PSCI_DMA_ERR,
 PSCI_NOC_TIMEOUT,
 PSCI_G3D_PWR_ERR,
 PSCI_CPU_PWR_ERR,
 PSCI_TSENSOR_ERR,
 PSCI_CPUDVFS_ERR,
 PSCI_GPUDVFS_ERR,
 PSCI_MEMRP_ERR,
 PSCI_NOC_BUS_IDLE_PEND,
 PSCI_AHB_TIMEOUT,
 PCSI_PERIVOLT_ERR,
 PSCI_REASON_SYS_EXC_END = 0x1f,
 PSCI_REASON_DDR_UNAVAILABLE_BEGIN = 0x20,
 PSCI_DDR_PANIC = PSCI_REASON_DDR_UNAVAILABLE_BEGIN,
 PSCI_DDR_FATAL_ERR,
 PSCI_DDR_SREF_ERR,
 PSCI_DDR_OSC_ERR,
 PSCI_DDR_TMON_LOW,
 PSCI_DDR_TMON_HIGH,
 PSCI_DDR_GATE_ERR,
 PSCI_UCE0_EXC,
 PSCI_UCE1_EXC,
 PSCI_UCE2_EXC,
 PSCI_UCE3_EXC,
 PSCI_DDR_AREF_ALARM,
 PSCI_DDR_RDTIMEOUT,
 PSCI_DDR_PLLUNLOCK_ERR,
 PSCI_DDR_RETRAIN_ERR,
 PSCI_DDR_TMON_ERR,
 PSCI_DDR_DFS_OFF_TIMEOUT,
 PSCI_DDR_DVALID_ERR,
 PSCI_DDR_DFI_SEL_ERR,
 PSCI_DDR_PLLUNLOCK_LP,
 PSCI_DDR_UNKNOWN_ERR,
 PSCI_UCE_EXC,
 PSCI_DDR_LOAD_GENERAL_SE_TIMEOUT,
 PSCI_DDR_DFS_REQ_TIMEOUT,
 PSCI_DDR_CORE_VOLT_NULL,
 PSCI_DDR_SCENE_ID_ERR,
 PSCI_DDR_LATSTAT_SR_ERR,
 PSCI_DDR_SC_LOCK,
 PSCI_DDR_SLT_SC_LOCK,
 PSCI_DDR_ZCAL_ERR,
 PSCI_DDR_XPU_WFI_TIMEOUT,
 PSCI_DDR_XPU_EXC,
 PSCI_DDR_QICE_DLOCK_INT,
 PSCI_DDR_TMON_TIMEOUT,
 PSCI_REASON_DDR_PANIC_END = 0x5f,
 PSCI_DMA_TIMEOUT = 0x60,
 PSCI_SUBPMU0_PANIC,
 PSCI_SUBPMU1_PANIC,
 PSCI_REASON_DDR_UNAVAILABLE_END = 0x7f,
 PSCI_REASON_DDR_AVAILABLE_BEGIN = 0x80,
 PSCI_DDR_DMSS_VOLT_NULL = PSCI_REASON_DDR_AVAILABLE_BEGIN,
 PSCI_DDR_DMSS_VOLT_ERR,
 PSCI_QICE_DLOCK_INT,
 PSCI_DDR_PERI_MEM_NULL,
 PSCI_REASON_DDR_AVAILABLE_END = 0x9f,
 PSCI_REASON_OTHERIP_REQ_BEGIN = 0xa0,
 PSCI_OTHERIP_REQ_MODEM,
 PSCI_OTHERIP_REQ_HIFI,
 PSCI_OTHERIP_REQ_IOMCU,
 PSCI_REASON_OTHERIP_REQ_END = 0xaf,
 PSCI_MODEM_BEGIN = 0xb0,
 PSCI_MODEM_MEM_REPAIR = PSCI_MODEM_BEGIN,
 PSCI_MODEM_TIME_OUT,
 PSCI_MODEM_END = 0xbf,
 PSCI_REASON_MAX,
 PSCI_REASON_UNKNOWN = 0xff,
};
enum lpcpu_subtype
{
 LPCPU_EXCP_RESERVED = 0x0,
 LPCPU_EXCP_WDT,
 LPCPU_EXCP_PANIC,
 LPCPU_EXCP_BUGON,
};
enum lpgpu_subtype
{
 LPGPU_EXCP_RESERVED = 0x0,
 LPGPU_EXCP_WDT,
 LPGPU_EXCP_PANIC,
 LPGPU_EXCP_BUGON,
};
enum mem_repair_subtype {
 OMR_PASS_REBOOT = 0x1,
 OMR_REPAIR_FAIL = 0x2,
 OMR_RETEST_FAIL = 0x3,
};
enum scharger_subtype{
 PMU_VSYS_OV = 0x1,
 PMU_VSYS_PWROFF_ABS = 0x2,
 PMU_VSYS_PWROFF_DEB = 0x4,
 PMU_CALI_VSYSPWROFF_DEB = 0x8,
 PMU_CALI_VSYSPWROFF_ABS = 0x10,
 PMU_CALI_VSYS_OV = 0x20,
};
enum pmu_subtype
{
 SYS_NRST_4S = 0x1,
 PMUA_SHORT_F = 0x2,
 PMUH_SHORT_F = 0X3,
 VIN_LDOH_SHUTDOWN = 0x4,
 VSYS_PWRONEXP_SHUTDOWN = 0x5,
 CALI_PMUH_OCP = 0x6,
 CALI_LDO26_OCP = 0x7,
 CALI_BUCK2_SCP = 0x8,
 CALI_BUCK2_OCP = 0x9,
 CALI_PMUH_SHORT = 0xA,
 CALI_PMUD_SHORT = 0xB,
 OCP_BUCK1 = 0xC,
 OCP_BUCK2 = 0xD,
 OCP_BUCK3 = 0xE,
 OCP_BUCK40 = 0xF,
 OCP_BUCK41 = 0x10,
 OCP_BUCK42 = 0x11,
 OCP_BUCK70 = 0x12,
 OCP_BUCK71 = 0x13,
 OCP_LDO6 = 0x14,
 OCP_LDO4 = 0x15,
 OCP_LDO3 = 0x16,
 OCP_LDO2 = 0x17,
 OCP_LDO1 = 0x18,
 OCP_LDO0 = 0x19,
 OCP_BUCK5 = 0x1A,
 OCP_BUCK9 = 0x1B,
 OCP_LDO18 = 0x1C,
 OCP_LDO17 = 0x1D,
 OCP_LDO16 = 0x1E,
 OCP_LDO15 = 0x1F,
 OCP_LDO14 = 0x20,
 OCP_LDO12 = 0x21,
 OCP_LDO11 = 0x22,
 OCP_LDO8 = 0x23,
 OCP_LDO28 = 0x24,
 OCP_LDO27 = 0x25,
 OCP_LDO26 = 0x26,
 OCP_LDO25 = 0x27,
 OCP_LDO24 = 0x28,
 OCP_LDO23 = 0x29,
 OCP_LDO22 = 0x2A,
 OCP_LDO21 = 0x2B,
 OCP_LDO39 = 0x2C,
 OCP_LDO38 = 0x2D,
 OCP_LDO37 = 0x2E,
 OCP_LDO36 = 0x2F,
 OCP_PMUH = 0x30,
 OCP_LDO32 = 0x31,
 OCP_LDO30 = 0x32,
 OCP_LDO29 = 0x33,
 OCP_SW1 = 0x34,
 OCP_LDO46 = 0x35,
 OCP_LDO45 = 0x36,
 OCP_LDO44 = 0x37,
 OCP_LDO43 = 0x38,
 OCP_LDO42 = 0x39,
 OCP_LDO41 = 0x3A,
 OCP_SINK = 0x3B,
 SCP_BUCK1 = 0x3C,
 SCP_BUCK2 = 0x3D,
 SCP_BUCK3 = 0x3E,
 SCP_BUCK401 = 0x3F,
 SCP_BUCK42 = 0x40,
 SCP_BUCK5 = 0x41,
 SCP_BUCK70 = 0x42,
 SCP_BUCK71 = 0x43,
 SCP_BUCK9 = 0x44,
 SCP_LDOBUFF = 0x45,
 OCP_DISCHARGER = 0x46,
 SCP_SINK = 0x47,
 OVP_BUCK401 = 0x48,
 OVP_BUCK42 = 0x49,
 OVP_BUCK70 = 0x4A,
 OVP_BUCK71 = 0x4B,
 CUR_DET_BUCK401 = 0x4C,
 CUR_DET_BUCK42 = 0x4D,
 RAMP_BUCK40 = 0x4E,
 RAMP_BUCK42 = 0x4F,
 RAMP_BUCK5 = 0x50,
 RAMP_BUCK70 = 0x51,
 RAMP_BUCK71 = 0x52,
 RAMP_LDO0 = 0x53,
 RAMP_LDO39 = 0x54,
 RAMP_LDO44 = 0x55,
 RAMP_LDO45 = 0x56,
 RAMP_ABNOR_BUCK40 = 0x57,
 RAMP_ABNOR_BUCK42 = 0x58,
 RAMP_ABNOR_BUCK5 = 0x59,
 RAMP_ABNOR_BUCK70 = 0x5A,
 RAMP_ABNOR_BUCK71 = 0x5B,
 RAMP_ABNOR_LDO0 = 0x5C,
 RAMP_ABNOR_LDO39 = 0x5D,
 RAMP_ABNOR_LDO44 = 0x5E,
 RAMP_ABNOR_LDO45 = 0x5F,
};
enum sub_pmu_subtype
{
 SUBPMU_VSYS_PWRON = 0x1,
 SUBPMU_OCP_B31 = 0x02,
 SUBPMU_OCP_B30 = 0x03,
 SUBPMU_OCP_B21 = 0x04,
 SUBPMU_OCP_B20 = 0x05,
 SUBPMU_OCP_B11 = 0x06,
 SUBPMU_OCP_B10 = 0x07,
 SUBPMU_OCP_B01 = 0x08,
 SUBPMU_OCP_B00 = 0x09,
 SUBPMU_OCP_B13 = 0x0A,
 SUBPMU_OCP_B12 = 0x0B,
 SUBPMU_OCP_B8 = 0x0C,
 SUBPMU_OCP_B7 = 0x0D,
 SUBPMU_OCP_B6 = 0x0E,
 SUBPMU_OCP_PMUH = 0x0F,
 SUBPMU_OCP_BB = 0x10,
 SUBPMU_OCP_LDO54 = 0x11,
 SUBPMU_OCP_LDO53 = 0x12,
 SUBPMU_OCP_LDO52 = 0x13,
 SUBPMU_OCP_LDO51 = 0x14,
 SUBPMU_OCP_LDO50 = 0x15,
 SUBPMU_OCP_LDO34 = 0x16,
 SUBPMU_OCP_LDO9 = 0x17,
 SUBPMU_OCP_SW10 = 0x18,
 SUBPMU_SCP_B30 = 0x19,
 SUBPMU_SCP_B20 = 0x1A,
 SUBPMU_SCP_B10 = 0x1B,
 SUBPMU_SCP_B00 = 0x1C,
 SUBPMU_SCP_B13 = 0x1D,
 SUBPMU_SCP_B12 = 0x1E,
 SUBPMU_SCP_B8 = 0x1F,
 SUBPMU_SCP_B7 = 0x20,
 SUBPMU_SCP_B6 = 0x21,
 SUBPMU_SCP_BB = 0x22,
 SUBPMU_OVP_B30 = 0x23,
 SUBPMU_OVP_B20 = 0x24,
 SUBPMU_OVP_B10 = 0x25,
 SUBPMU_OVP_B00 = 0x26,
 SUBPMU_OVP_B12 = 0x27,
 SUBPMU_OVP_B8 = 0x28,
 SUBPMU_OVP_BB = 0x29,
 SUBPMU_CUR_B30 = 0x2A,
 SUBPMU_CUR_B20 = 0x2B,
 SUBPMU_CUR_B10 = 0x2C,
 SUBPMU_CUR_B00 = 0x2D,
 SUBPMU_CUR_B12 = 0x2E,
 SUBPMU_RAMP_ABN_B7 = 0x2F,
 SUBPMU_RAMP_ABN_B6 = 0x30,
 SUBPMU_RAMP_ABN_B12 = 0x31,
 SUBPMU_RAMP_ABN_B30 = 0x32,
 SUBPMU_RAMP_ABN_B20 = 0x33,
 SUBPMU_RAMP_ABN_B10 = 0x34,
 SUBPMU_RAMP_ABN_B00 = 0x35,
 SUBPMU_PMUD_SHORT_F = 0x36,
 SUBPMU_PMUH_SHORT_F = 0x37,
 SUBPMU_VIN_LDOH = 0x38,
 SUBPMU_RAMP_ECO_B7 = 0x39,
 SUBPMU_RAMP_ECO_B6 = 0x3A,
 SUBPMU_RAMP_ECO_B12 = 0x3B,
 SUBPMU_RAMP_ECO_B30 = 0x3C,
 SUBPMU_RAMP_ECO_B20 = 0x3D,
 SUBPMU_RAMP_ECO_B10 = 0x3E,
 SUBPMU_RAMP_ECO_B00 = 0x3F,
};
enum npu_subtype
{
 AICORE_EXCP = 0x0,
 AICORE_TIMEOUT,
 TS_RUNNING_EXCP,
 TS_RUNNING_TIMEOUT,
 TS_INIT_EXCP,
 AICPU_INIT_EXCP,
 AICPU_HEARTBEAT_EXCP,
 POWERUP_FAIL,
 POWERDOWN_FAIL,
 NPU_NOC_ERR,
 NPU_AICORE0_NOC_ERR,
 NPU_AICORE1_NOC_ERR,
 NPU_SDMA1_NOC_ERR,
 NPU_TS_CPU_NOC_ERR,
 NPU_TS_HWTS_NOC_ERR,
 NPU_TCU_NOC_ERR,
 SMMU_EXCP,
 HWTS_EXCP,
 AIV_EXCP,
 AIV_TIMEOUT,
};
enum conn_subtype
{
    CONN_WIFI_EXEC = 0,
    CONN_WIFI_CHAN_EXEC,
    CONN_WIFI_WAKEUP_FAIL,
    CONN_BFGX_EXEC,
    CONN_BFGX_BEAT_TIMEOUT,
    CONN_BFGX_WAKEUP_FAIL,
};
enum general_see_subtype {
 EXC_SENSOR_CTRL = 0,
 EXC_SIC,
 EXC_MED,
 EXC_MISC,
 EXC_OTPC,
 EXC_HARD,
 EXC_IPC_MAILBOX,
 EXC_MPU,
 EXC_BUS,
 EXC_SSP,
 EXC_SEC_EXTERN,
 EXC_WDG,
 EXC_SYSALARM,
 EXC_NV_COUNTER,
 EXC_COS,
 EXC_SYSINFO,
 EXC_MNTN_COS,
 EXC_MNTN_COS_RESET,
 EXC_LIBC,
 EXC_NVM,
 EXC_SECENG_TRNG,
 EXC_SECENG_TRIM,
 EXC_SECENG_SCE,
 EXC_SECENG_RSA,
 EXC_SECENG_SM2,
 EXC_SECENG_KM,
 EXC_SECENG_SCRAMBLING,
 EXC_SECBOOT,
 EXC_MAIN, EXC_SECFLASH_FACTORY, EXC_SECFLASH_DATALINK,
 EXC_SECFLASH_SW, EXC_SECFLASH_SCP03, EXC_SECFLASH_SA,
 EXC_BOTTOM,
 EXC_MSPC_ROM, EXC_RPMB_KO,
 EXC_ALARM0, EXC_ALARM1,
 EXC_AS2AP_IRQ,
 EXC_DS2AP_IRQ,
 EXC_SENC2AP_IRQ,
 EXC_SENC2AP_IRQ0,
 EXC_SENC2AP_IRQ1,
 EXC_LOCKUP,
 EXC_EH2H_SLV,
 EXC_TSENSOR0,
 EXC_TSENSOR1,
 EXC_RST,
 EXC_SOC_ALARM_IRQ,
 EXC_CORE_PARITY_IRQ,
 EXC_SUBSYS_ALARM_IRQ,
 EXC_DUAL_VIOLATION_IRQ,
 EXC_INT_TRAP_IRQ,
 EXC_UNKNOWN,
};
enum dss_subtype {
 DSS_NOC_EXCEPTION = 0,
 DSS_DDRC_EXCEPTION,
};
enum gpu_subtype {
 GPU_HVGR_HANG = 0,
 GPU_HVGR_FAULT,
 GPU_HVGR_PAGE_FAULT,
 GPU_HVGR_CRASH,
};
typedef enum {
 RDR_REG_BACKUP_IDEX_0 = 0,
 RDR_REG_BACKUP_IDEX_1,
 RDR_REG_BACKUP_IDEX_2,
 RDR_REG_BACKUP_IDEX_3,
 RDR_REG_BACKUP_IDEX_MAX
} RDR_REG_BACKUP_IDEX;
typedef enum {
 CATEGORY_START = 0x0,
 NORMALBOOT,
 PANIC,
 HWWATCHDOG,
 LPM3EXCEPTION,
 BOOTLOADER_CRASH,
 TRUSTZONE_REBOOTSYS,
 MODEM_REBOOTSYS,
 BOOTFAIL,
 HARDWARE_FAULT,
 MODEMCRASH,
 HIFICRASH,
 AUDIO_CODEC_CRASH,
 SENSORHUBCRASH,
 ISPCRASH,
 IVPCRASH,
 TRUSTZONECRASH,
 GENERAL_SEE_CRASH,
 UNKNOWNS,
 PRESS10S,
 PRESS6S,
 NPUEXCEPTION,
 CONNEXCEPTION,
 FDULCRASH,
 DSSCRASH,
 GPUEXCEPTION,
 LPCPUEXCEPTION,
 LPGPUEXCEPTION,
 SUBTYPE = 0xff,
}CATEGORY_SOURCE;
struct exp_subtype {
 unsigned int exception;
 unsigned char category_name[24];
 unsigned char subtype_name[24];
 unsigned int subtype_num;
};
typedef struct exc_special_backup_data {
 unsigned char reset_reason[RDR_REG_BACKUP_IDEX_MAX];
 unsigned int slice;
 unsigned int rtc;
 unsigned int REG_Reg13;
 unsigned int REG_LR1;
 unsigned int REG_PC;
 unsigned int REG_XPSR;
 unsigned int NVIC_CFSR;
 unsigned int NVIC_HFSR;
 unsigned int NVIC_BFAR;
 unsigned char exc_trace;
 unsigned char ddr_exc;
 unsigned short irq_id;
 unsigned int task_id;
} EXC_SPECIAL_BACKUP_DATA_STRU;
typedef struct rdr_reg_backup_head {
 unsigned int idex;
 unsigned int reason[RDR_REG_BACKUP_IDEX_MAX - 1];
} RDR_REG_BACKUP_HEAD_STRU;
typedef struct rdr_reg_backup_data {
 unsigned int Reg0;
 unsigned int Reg1;
 unsigned int Reg2;
 unsigned int Reg3;
 unsigned int Reg4;
 unsigned int Reg5;
 unsigned int Reg6;
 unsigned int Reg7;
 unsigned int Reg8;
 unsigned int Reg9;
 unsigned int Reg10;
 unsigned int Reg11;
 unsigned int Reg12;
 unsigned int Reg13;
 unsigned int MSP;
 unsigned int PSP;
 unsigned int LR0_CONTROL;
 unsigned int LR1;
 unsigned int PC;
 unsigned int XPSR;
 unsigned int PRIMASK;
 unsigned int BASEPRI;
 unsigned int FAULTMASK;
 unsigned int CONTROL;
} RDR_REG_BACKUP_DATA_STRU;
#endif