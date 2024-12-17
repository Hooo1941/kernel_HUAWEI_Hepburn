-include $(srctree)/drivers/platform_drivers/modem/drv/product_config/product_config.mk

KBUILD_CPPFLAGS += -I$(srctree)/drivers/platform_drivers/modem/drv/omserver/omp/hdlc \
                   -I$(srctree)/drivers/platform_drivers/modem/drv/omserver/omp

obj-y += omp_service.o

obj-y += hdlc/

GCOV_PROFILE := y