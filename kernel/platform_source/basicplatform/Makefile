
obj-y += drivers/
obj-y += block/
obj-y += fs/
obj-y += kernel/
obj-$(CONFIG_H_MM) += mm/

menuconfig:
	cd ../..;make ARCH=arm64 menuconfig KBUILD_KCONFIG=platform_source/basicplatform/Kconfig KCONFIG_CONFIG=platform_source/basicplatform/.config
