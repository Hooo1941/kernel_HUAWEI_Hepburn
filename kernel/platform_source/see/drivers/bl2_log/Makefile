ifeq ($(USE_BISHENG_CLANG),true)
subdir-ccflags-y += -Wno-error=unused-but-set-variable
endif

obj-$(CONFIG_ATF_KERNEL_BL2_LOG)     += bl2_log.o

subdir-ccflags-y += -Wdate-time -Wextra -Wfloat-equal -Wformat=2 -Wshadow
