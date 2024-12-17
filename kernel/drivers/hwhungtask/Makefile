obj-$(CONFIG_DFX_HUNGTASK) += hungtask_mod.o hungtask_base.o
obj-$(CONFIG_DFX_HUNGTASK_EXT) += hungtask_ext.o
obj-$(CONFIG_DFX_HUNGTASK_USER) += hungtask_user.o
obj-$(CONFIG_DFX_HUNGTASK_MMAP_SEM) += hungtask_mmap_sem.o

ifeq ($(HLTHERM_RUNTEST), true)
    CFLAGS_KERNEL += -DCONFIG_HLTHERM_RUNTEST
endif
