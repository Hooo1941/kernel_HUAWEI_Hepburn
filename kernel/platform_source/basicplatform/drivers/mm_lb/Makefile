LB_CHIP_TYPE := $(strip $(subst ", , $(CONFIG_VENDOR_CHIP_SUFFIX)))

ifneq ($(LB_CHIP_TYPE), es)
EXTRA_CFLAGS += -DCONFIG_HISI_LB_WAY_16
EXTRA_CFLAGS += -DCONFIG_HISI_LB_SCB_ENABLE
endif

obj-$(CONFIG_MM_LB_V550) += v550/

ifneq ($(CONFIG_MM_LB_V550), y)
obj-y += mm_lb_cache.o
obj-y += mm_lb_driver.o
obj-y += mm_lb_policy.o
obj-y += mm_lb.o
endif
obj-$(CONFIG_MM_LB_PMU) += mm_lb_pmu.o
obj-$(CONFIG_MM_LB_V500) += mm_lb_driver_v500.o mm_lb_mpam.o
obj-$(CONFIG_MM_LB_PMU_V500) += mm_lb_event_v500.o
obj-$(CONFIG_MM_LB_TST) += mm_lb_tst.o