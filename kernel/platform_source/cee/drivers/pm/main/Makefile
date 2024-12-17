ccflags-y += -Wextra -Wdate-time -Wfloat-equal -Wshadow -Wformat=2

PM_DIR :=$(srctree)/platform_source/cee/drivers/pm

EXTRA_CFLAGS += -I$(PM_DIR)

ifeq ($(CONFIG_LIBLINUX), y)
obj-$(CONFIG_SR) += suspend_host.o
else
obj-$(CONFIG_SR) += suspend.o
endif
obj-$(CONFIG_SR) += sr_mntn.o
obj-$(CONFIG_SR_SYNC) += sys_sync.o
obj-$(CONFIG_PM_LPCTRL) += lowpm_ctrl.o
obj-$(CONFIG_SYS_PM_ULSR) += ulsr.o
obj-$(CONFIG_SYS_PM_DOZE) += doze.o
obj-$(CONFIG_SR_HIBERNATION_WATCHDOG) += hibernate_watchdog.o
