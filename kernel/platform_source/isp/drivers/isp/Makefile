ifeq ($(CONFIG_HISP_DEBUG), y)
subdir-ccflags-y += -DDEBUG_HISP
# pcie_fpga_verify may use CONFIG_PCIE_KPORT_EP_FPGA_VERIFY macro
obj-$(CONFIG_ISP) += hisp_pcie.o

ifeq ($(CONFIG_KCOV), y)
subdir-ccflags-y += -DHISP_KERNEL_FUZZ
endif
endif

ifeq ($(es_low_freq),true)
subdir-ccflags-y += -DCONFIG_ES_ISP_LOW_FREQ
endif

ifeq ($(CONFIG_ISP_BUF_PRIVATIZE), y)
subdir-ccflags-y += -DISP_BUF_PRIVATIZE
endif

ifeq ($(CONFIG_KERNEL_CAMERA_ISPV500), y)
subdir-ccflags-y += -DISP_LINX_CPU_SUPPORT
subdir-ccflags-y += -DHISP_VERSION_V500
obj-$(CONFIG_ISP) += v500/
endif

obj-$(CONFIG_HISP_RDR) += hisp_rdr.o hisp_log.o

obj-$(CONFIG_HISP_RPMSG) += hisp_virtio_rpmsg.o
obj-$(CONFIG_ISP_INTERNAL_MSG) += hisp_msg_service.o

obj-$(CONFIG_ISP) += hisp_smc.o hisp_rpm.o hisp_cma.o hisp_ca.o \
                     hisp_nsec.o hisp_sec.o hisp_dump.o \
                     hisp_device.o hisp_pwr.o hisp_sensor.o \
                     hisp_remoteproc.o hisp_notify.o

ifeq ($(CONFIG_KERNEL_CAMERA_ISPV500), y)
obj-$(CONFIG_ISP) += hisp_memory.o
else
obj-$(CONFIG_ISP) += hisp_mempool.o
endif

ifdef CONFIG_HISP_FFA_SUPPORT
obj-$(CONFIG_ISP) += hisp_ffa.o
endif

ifdef CONFIG_HISP_MEMECC_SUPPORT
obj-$(CONFIG_ISP) += hisp_memecc.o
endif

ifdef CONFIG_HISP_LOAD_SECISPIMAGE
obj-$(CONFIG_ISP) += hisp_load_image_ca.o
endif

ifeq ($(CONFIG_ISP), y)
subdir-ccflags-y += -Wdate-time -Wextra -Wfloat-equal -Wformat=2 -Wshadow
subdir-ccflags-y += -I$(srctree)/drivers/platform_drivers/tzdriver \
                    -I$(srctree)/drivers/rpmsg \
                    -I$(srctree)/platform_source/see/drivers/load_image \
                    -I$(srctree)/platform_source/basicplatform/drivers/mntn/blackbox \
                    -I$(srctree)/platform_source/camera/drivers/media/native/camera \
                    -I$(srctree)/platform_source/camera/drivers/media/native/camera/sensor \
                    -I$(srctree)/platform_source/camera/drivers/media/native/camera/platform \
                    -I$(srctree)/../../vendor/huawei/chipset_common/plugins/kernel/include
endif

ifeq ($(CONFIG_KERNEL_CAMERA_ISPV500), y)
subdir-ccflags-y += -I$(srctree)/platform_include/isp/linux/v500
endif

obj-$(CONFIG_HISP_DPM) += hisp_dpm.o

ifeq ($(CONFIG_HISP_FUZZ), y)
subdir-ccflags-y += -DHISP_FUZZ
ifeq ($(CONFIG_HISP_KCOV_ENABLE_DUMP), y)
subdir-ccflags-y += -DHISP_KCOV_ENABLE_DUMP
endif
obj-$(CONFIG_ISP) += hisp_fuzz.o hisp_kcov.o
endif