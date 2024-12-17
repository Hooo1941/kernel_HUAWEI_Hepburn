ccflags-$(CONFIG_SECURITY_INFO) += -I$(srctree)/drivers/platform_drivers/tzdriver
ccflags-$(CONFIG_VENDOR_EFUSE)  += -I$(srctree)/drivers/platform_drivers/efuse

obj-$(CONFIG_DFX_DEBUG_FS)	+= test/efuse_kernel_function_test.o
obj-$(CONFIG_DFX_DEBUG_FS)	+= test/efuse_exception_test.o
ifeq ($(CONFIG_EFUSE_SUPPORT_FFA), y)
obj-$(CONFIG_DFX_DEBUG_FS)	+= test/efuse_tzsp_function_test.o
endif
ifeq ($(CONFIG_SECURITY_INFO), y)
obj-$(CONFIG_DFX_DEBUG_FS)	+= test/efuse_teeos_function_test.o
endif

ifeq ($(CONFIG_EFUSE_SUPPORT_FFA), y)
obj-$(CONFIG_VENDOR_EFUSE)	+= efuse_call_ffa.o
obj-$(CONFIG_VENDOR_EFUSE)	+= custom_call/efuse_cust_rw_ffa.o
else
obj-$(CONFIG_VENDOR_EFUSE)	+= efuse_call_smc.o
obj-$(CONFIG_VENDOR_EFUSE)	+= custom_call/efuse_cust_rw_smc.o
endif

obj-$(CONFIG_VENDOR_EFUSE)	+= common_call/efuse.o
obj-$(CONFIG_VENDOR_EFUSE)	+= common_call/efuse_internal.o

obj-$(CONFIG_VENDOR_EFUSE)	+= custom_call/efuse_driver.o
obj-$(CONFIG_GENERAL_SEE)	+= custom_call/flash_general_see_otp.o

subdir-ccflags-y += -Wdate-time -Wextra -Wfloat-equal -Wformat=2 -Wshadow
