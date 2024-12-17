-include $(srctree)/drivers/platform_drivers/modem/drv/product_config/product_config.mk 

EXTRA_CFLAGS += -Idrivers/platform_drivers/tzdriver

ifeq ($(strip $(CFG_CONFIG_RFILE_ON)),YES)
obj-y  += rfile_server.o
obj-y  += rfile_server_load_mode.o
obj-y  += rfile_server_dump.o
obj-y  += rfile_server_fipc.o
endif

GCOV_PROFILE := y