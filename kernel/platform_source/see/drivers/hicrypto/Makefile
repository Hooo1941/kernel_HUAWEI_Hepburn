obj-$(CONFIG_CRYPTO_HICRYPTO) += hicrypto.o

hicrypto-objs := hicrypto_driver.o hicrypto_dev.o

ifeq ($(CONFIG_KERNEL_MODE_NEON), y)
	obj-$(CONFIG_CRYPTO_HICRYPTO) += hicrypto_neon.o
CFLAGS_REMOVE_hicrypto_neon.o += -mgeneral-regs-only
CFLAGS_hicrypto_neon.o += -ffreestanding
CFLAGS_hicrypto_neon.o += -isystem $(shell $(CC) -print-file-name=include)
endif

hicrypto-$(CONFIG_CRYPTO_HICRYPTO_SHA) += hicrypto_hash.o
hicrypto-$(CONFIG_CRYPTO_HICRYPTO_CIPHER) += hicrypto_cipher.o
hicrypto-$(CONFIG_CRYPTO_HICRYPTO_AEAD) += hicrypto_aead_coherent.o
hicrypto-$(CONFIG_CRYPTO_HICRYPTO_AEAD) += hicrypto_dev_aead.o
hicrypto-$(CONFIG_CRYPTO_HICRYPTO_MAC) += hicrypto_mac.o

hicrypto-$(CONFIG_CRYPTO_HICRYPTO_DEBUG) += hicrypto_dft.o

subdir-ccflags-y += -I$(srctree)/drivers/platform_drivers/ap/platform/common/lowpm
subdir-ccflags-y += -Wdate-time -Wextra -Wfloat-equal -Wformat=2 -Wshadow
