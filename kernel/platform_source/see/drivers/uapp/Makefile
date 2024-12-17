ccflags-$(CONFIG_UAPP)          += -I$(srctree)/platform_source/see/drivers/uapp/include

obj-$(CONFIG_UAPP)              += src/uapp.o
obj-$(CONFIG_UAPP)              += src/uapp_file_pos.o
obj-$(CONFIG_DFX_DEBUG_FS)      += test/uapp_test.o

subdir-ccflags-y += -Wdate-time -Wextra -Wfloat-equal -Wformat=2 -Wshadow
