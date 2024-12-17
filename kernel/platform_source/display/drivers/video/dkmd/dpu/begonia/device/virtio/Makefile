obj-y			+= virtio.o
virtio-objs := \
	virtio_fb.o \
	virtio_iommu.o

EXTRA_CFLAGS += \
	-I$(srctree)/drivers/staging/android/ion/ \
	-I$(srctree)/include/platform_include/display/dkmd/ \
	-I$(srctree)/drivers/video/fbdev/dkmd/dksm
