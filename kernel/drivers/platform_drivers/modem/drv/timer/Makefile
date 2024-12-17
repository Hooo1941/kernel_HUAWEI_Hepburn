
-include $(srctree)/drivers/platform_drivers/modem/drv/product_config/product_config.mk
ifeq ($(strip $(CFG_CONFIG_MODULE_TIMER)),YES)
    obj-y += timer_slice.o
    obj-y += timer_driver_arm.o
    obj-y += timer_driver_modem.o
    obj-y += timer_hal.o
    obj-y += timer_dpm.o
    obj-y += timer_device.o
    obj-y += timer.o
    obj-y += hrtimer.o
endif

GCOV_PROFILE := y