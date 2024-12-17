#
#kernel dump Makefile
#
subdir-ccflags-y              += -Wall -Werror -Wno-declaration-after-statement -DTEXT_OFFSET=$(TEXT_OFFSET)
ifeq ($(USE_BISHENG_CLANG),true)
    subdir-ccflags-y += -Wno-error=unused-but-set-variable
endif
obj-$(CONFIG_DFX_KERNELDUMP) += kernel_dump.o hck_dfx_kdump.o
