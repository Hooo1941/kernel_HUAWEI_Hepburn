
obj-$(CONFIG_NPU_KERNEL_DRIVER) += drivers/npu/

menuconfig:
	cd ../..;make ARCH=arm64 menuconfig KBUILD_KCONFIG=platform_source/ai/Kconfig KCONFIG_CONFIG=platform_source/ai/.config
