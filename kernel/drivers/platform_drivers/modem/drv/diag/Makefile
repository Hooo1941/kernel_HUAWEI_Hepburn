-include $(srctree)/drivers/platform_drivers/modem/drv/product_config/product_config.mk

KBUILD_CPPFLAGS += -I$(srctree)/drivers/platform_drivers/modem/drv/diag  \
                   -I$(srctree)/drivers/platform_drivers/modem/drv/diag/srvchan \
                   -I$(srctree)/drivers/platform_drivers/modem/drv/diag/msgserver

obj-y += diag_init.o
obj-y += diag_api.o
obj-y += diag_debug.o
obj-y += srvchan/srvchan.o
obj-y += srvchan/nlsrvport.o
obj-y += encoder.o
obj-y += decoder.o
obj-y += diagmsg.o
obj-y += logreport.o
obj-y += diagnve.o
obj-y += msgserver/msgsrv.o
obj-y += msgserver/connectsrv.o
obj-y += msgserver/configsrv.o
obj-y += transfertask.o

GCOV_PROFILE := y