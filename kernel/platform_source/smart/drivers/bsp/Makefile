obj-$(CONFIG_INPUTHUB_30) += boot/iomcu_boot.o
obj-$(CONFIG_CONTEXTHUB_LOAD_IOMCU_IMAGE) += dump/load_iomcu_ca.o
obj-$(CONFIG_INPUTHUB_30) += dump/iomcu_dump.o
obj-$(CONFIG_INPUTHUB_30) += dump/iomcu_dump_info_restore.o
obj-$(CONFIG_INPUTHUB_30) += dump/iomcu_dump_sysctrl.o
obj-$(CONFIG_INPUTHUB_30) += pm/iomcu_pm.o
obj-$(CONFIG_CONTEXTHUB) += status/iomcu_status.o
obj-$(CONFIG_CONTEXTHUB) += config/iomcu_config.o

ifneq ($(CONFIG_IPC_V5),y)
obj-$(CONFIG_INPUTHUB_30) += ipc_v3/iomcu_ipc.o
obj-$(CONFIG_INPUTHUB_30) += ipc_v3/iomcu_shmem.o
endif

obj-$(CONFIG_IPC_V5) += ipc_v5/iomcu_tcp_amsm.o
obj-$(CONFIG_IPC_V5) += ipc_v5/iomcu_tcp_amsm_adapter.o
obj-$(CONFIG_IPC_V5) += ipc_v5/iomcu_tcp_amf20.o
obj-$(CONFIG_IPC_V5) += ipc_v5/iomcu_link_ipc.o
obj-$(CONFIG_IPC_V5) += ipc_v5/iomcu_link_ipc_shm.o

ifeq ($(CONFIG_CONTEXTHUB_LOAD_IOMCU_IMAGE),y)
EXTRA_CFLAGS += -I$(srctree)/drivers/platform_drivers/tzdriver/
endif


