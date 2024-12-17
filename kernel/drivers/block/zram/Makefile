# SPDX-License-Identifier: GPL-2.0-only
ifdef CONFIG_CRYPTO_DELTA
zram-y	:=	sddc/zcomp_ext.o zram_drv.o
else
zram-y	:=	zcomp.o zram_drv.o
endif

obj-$(CONFIG_ZRAM)	+=	zram.o
obj-$(CONFIG_HP_CORE) += hp/
obj-$(CONFIG_CRC32)	+=	crc32.o
obj-$(CONFIG_CRYPTO_DELTA) += sddc/
