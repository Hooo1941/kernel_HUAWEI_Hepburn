ION_SOC_CHIPNAME := $(strip $(subst ", , $(CONFIG_VENDOR_CHIP_PREFIX)))

ccflags-y += -I$(srctree)/drivers/staging/android/ion
ccflags-y += -I$(srctree)/drivers/platform_drivers/ap/platform/$(ION_SOC_CHIPNAME)
ccflags-y += -I$(srctree)/drivers/platform_drivers/tzdriver

obj-$(CONFIG_ION_MM)+= of_mm_ion.o mm_ion_dump.o sec_alloc.o ion_alloc_debug.o
obj-$(CONFIG_ION_MM_SECSG) += ion_tee_op.o
obj-$(CONFIG_ION_MM_DMA_POOL) += ion_tee_op.o
obj-$(CONFIG_MM_VLTMM) += vltmm.o vltmm_agent.o
