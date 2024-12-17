ifeq ($(chip_type), )
	HECA_CHIP_TYPE := cs
else
	HECA_CHIP_TYPE := $(chip_type)
endif

HECA_ROOT := $(srctree)/platform_source/cee/drivers/heca
HECA_INC_PATH := $(HECA_ROOT)/include

ccflags-y += -I$(HECA_INC_PATH)/base/
ccflags-y += -I$(HECA_INC_PATH)/core/
ccflags-y += -I$(HECA_INC_PATH)/drv/
ccflags-y += -I$(HECA_INC_PATH)/fs/
ccflags-y += -I$(HECA_INC_PATH)/kernel/

ccflags-y += -I$(HECA_INC_PATH)/base/$(HECA_CHIP_TYPE)
ccflags-y += -I$(HECA_INC_PATH)/core/$(HECA_CHIP_TYPE)
ccflags-y += -I$(HECA_INC_PATH)/drv/$(HECA_CHIP_TYPE)
ccflags-y += -I$(HECA_INC_PATH)/fs/$(HECA_CHIP_TYPE)
ccflags-y += -I$(HECA_INC_PATH)/kernel/$(HECA_CHIP_TYPE)

ifeq ($(HECA_CHIP_TYPE), csv2)

# base srcs
obj-y += src/base/heca_base.o
obj-y += src/base/heca_ipc.o
obj-y += src/base/heca_worker.o
obj-y += src/base/heca_map.o
obj-y += src/base/heca.o
obj-y += src/base/$(HECA_CHIP_TYPE)/heca_sys.o

#core srcs
obj-y += src/core/heca_stat.o
obj-y += src/core/heca_game.o
obj-y += src/core/heca_game_stat.o
obj-y += src/core/heca_game_filter.o
obj-$(CONFIG_HECA_GPU) += src/core/heca_game_gpu.o
obj-y += src/core/$(HECA_CHIP_TYPE)/heca_plat.o

# drv src
obj-y += src/drv/heca_drv.o

# fs srcs
obj-y += src/fs/heca_sysfs.o
obj-y += src/fs/heca_game_sysfs.o
obj-y += src/fs/heca_sys_sysfs.o
obj-$(CONFIG_HECA_DEBUG) += src/fs/heca_debugfs.o
obj-$(CONFIG_HECA_DEBUG) += src/fs/heca_game_debugfs.o

# kernel srcs
obj-y += src/kernel/heca_freq.o
obj-y += src/kernel/heca_pm.o

else

obj-y += src/base/heca_stub.o

endif