ccflags-y += -I$(srctree)/drivers/staging/android/ion

obj-$(CONFIG_BG_DMABUF_RECLAIM) += bg_dmabuf_info_manager.o bg_dmabuf_ops.o bg_internal_buf_interface.o

ifeq ($(CONFIG_BG_DMABUF_RECLAIM),y)
obj-$(CONFIG_ION) += bg_internal_buf_ion.o
endif

obj-$(CONFIG_BG_DMABUF_TEST) += bg_dmabuf_test.o
