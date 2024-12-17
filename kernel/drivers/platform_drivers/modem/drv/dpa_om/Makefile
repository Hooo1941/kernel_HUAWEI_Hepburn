-include $(srctree)/drivers/platform_drivers/modem/drv/product_config/product_config.mk
ifeq ($(strip $(CFG_CONFIG_DPA_OM)),YES)
obj-y                   := diag_agent.o
else
obj-y                   := diag_agent_stub.o
endif
GCOV_PROFILE := y