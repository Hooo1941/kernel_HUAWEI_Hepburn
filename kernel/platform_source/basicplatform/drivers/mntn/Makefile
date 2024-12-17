ccflags-y += -I$(srctree)/drivers/platform_drivers/secure_boot/inc -Wall -Werror -Wno-declaration-after-statement -Wno-unused-but-set-variable
ccflags-$(CONFIG_CC_IS_CLANG) += -Wno-unknown-warning-option
EXTRA_CFLAGS += -Idrivers/platform_drivers/tzdriver

obj-$(CONFIG_DFX_PLATFORM_MAINTAIN)	+= platform_mntn.o
obj-$(CONFIG_PLATFORM_DFX_EASYSHELL)	+= ecall/
obj-$(CONFIG_DFX_BOOT_TIME) += boottime.o
obj-$(CONFIG_DFX_RECOVERY_BIGCORE_PANIC) += mntn_bc_panic.o
obj-$(CONFIG_DFX_BB) += blackbox/
obj-$(CONFIG_DDRC_KERNEL_CODE_PROTECTION) += code_protect/
ifeq ($(TARGET_VERSION_MODE),factory)
obj-$(CONFIG_DFX_VIRT_TO_PHYS) += virt_to_phys.o
obj-$(CONFIG_DFX_MNTN_FACTORY) += mntn_factory.o
endif
obj-$(CONFIG_DFX_POWER_OFF)	+= poweroff/dfx_poweroff.o poweroff/hck_dfx_poweroff.o
obj-$(CONFIG_DFX_FIQ)	+= dfx_fiq.o
platform_mntn-y := util.o dump.o fastbootlog.o mntn_filesys.o dfx_bootup_keypoint.o
obj-y += pstore/dfx_pstore.o
subdir-ccflags-$(CONFIG_DFX_BB)               +=  -I$(srctree)/platform_source/basicplatform/drivers/mntn/blackbox/
obj-$(CONFIG_MNTN_ALOADER_LOG) += aloader_log_record.o
obj-$(CONFIG_DFX_BB_DEBUG) += dfx_mntn_test.o
obj-$(CONFIG_DFX_MNTNDUMP)       += mntn_dump.o
obj-$(CONFIG_DFX_RECORD_SP)       += mntn_record_sp.o
obj-$(CONFIG_DFX_HW_SP)           += mntn_hw_sp.o
obj-$(CONFIG_GENERAL_SEE_MNTN) += general_see/
obj-$(CONFIG_DFX_BL31_MNTN) += bl31/
obj-$(CONFIG_SUBPMU) += subpmu/
obj-$(CONFIG_DFX_L3CACHE_ECC) += l3cache_ecc/
obj-$(CONFIG_DFX_HEALTH_GRADE) += health_grade/
obj-$(CONFIG_DFX_MNTN_SWITCH) += mntn_switch.o
obj-$(CONFIG_MNTN_HM_KLOG) += hm_klog/
obj-$(CONFIG_MNTN_BBOX_LOG) += mntn_bbox_log.o
obj-$(CONFIG_DFX_PAGE_TABLE_DUMP) += dfx_pagetable_dump.o
obj-$(CONFIG_DFX_PSTORE) += pstore/hck_dfx_pstore.o
obj-$(CONFIG_DFX_SAVE_SOC_REGISTER) += save_soc_register.o
obj-$(CONFIG_BALL_CRACK_DETECT) += ball_crack/
obj-$(CONFIG_RECORD_BOOT_TIME) += recordboottime.o
obj-$(CONFIG_DFX_SH) += dfx_sh/
