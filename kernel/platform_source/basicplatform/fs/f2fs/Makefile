ifdef CONFIG_FS_CRYPTO_OHOS
ccflags-y += -I$(srctree)/fs/f2fs
else
ccflags-y += -I$(srctree)/fs/hw_f2fs
endif
obj-$(CONFIG_HP_FILE) += hp_preallocate.o
obj-$(CONFIG_HW_F2FS_CHIP_KIRIN) += sdp_metadata.o
