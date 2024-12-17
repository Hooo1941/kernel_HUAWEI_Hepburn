obj-$(CONFIG_FINGERPRINT)  += fingerprint.o
ifeq ($(CONFIG_FINGERPRINT_TP_DECOUPLE),y)
EXTRA_CFLAGS += -DFINGERPRINT_TP_DECOUPLE_ENABLE
endif
EXTRA_CFLAGS += -I$(srctree)/drivers/hisi/tzdriver \
                -I$(srctree)/drivers/video/fbdev/dkmd/dpu/azalea
