subdir-ccflags-y += -I$(srctree)/drivers/staging/android
subdir-ccflags-y += -I$(srctree)/drivers/staging/android/ion
subdir-ccflags-y += -I$(srctree)/mm
subdir-ccflags-y += -I$(srctree)/kernel

ifeq ($(CONFIG_HISI_GPU), y)
subdir-ccflags-y += -I$(srctree)/drivers/gpu/hisi/kmd
obj-y += memcheck_log_gpumem_hvgr.o
else ifeq ($(CONFIG_DFX_OHOS)_$(CONFIG_MALI_MIDGARD), y_y)
obj-y += memcheck_log_gpumem_mali.o
endif

obj-$(CONFIG_DFX_MEMCHECK) += memcheck_mod.o memcheck_ioctl.o memcheck_account.o \
	memcheck_log_ion.o memcheck_log_ashmem.o memcheck_log_cma.o memcheck_log_vmalloc.o \
	memcheck_fd_socket.o memcheck_fd_pipe.o memcheck_fd_fence.o
obj-$(CONFIG_DFX_MEMCHECK_STACK) += memcheck_stack.o memcheck_detail.o
