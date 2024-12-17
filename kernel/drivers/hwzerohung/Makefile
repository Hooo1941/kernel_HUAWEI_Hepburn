ifeq ($(CONFIG_DFX_ZEROHUNG), y)
obj-y += zrhung_config.o zrhung_ioctl.o zrhung_event.o zrhung_mod.o
obj-$(CONFIG_DFX_ZEROHUNG_SOFTLOCKUP) += watchpoint/wp_softlockup.o
obj-$(CONFIG_DFX_ZEROHUNG_SOFTLOCKUP_INJECT) += watchpoint/wp_softlockup_inject.o
ifneq ($(TARGET_VERSION_MODE),factory)
ccflags-$(CONFIG_DFX_ZEROHUNG_EXT) += -DCONFIG_DFX_ZEROHUNG_WP_SCREEN
obj-$(CONFIG_DFX_ZEROHUNG_EXT) += watchpoint/wp_screen.o
endif
endif
