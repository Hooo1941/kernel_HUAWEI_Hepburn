ccflags-y += -I$(srctree)/mm
ccflags-$(CONFIG_MM_AMA) += -I$(srctree)/kernel/sched/

obj-$(CONFIG_H_MM_SLOW_PATH_COUNT) += slowpath_count.o
obj-$(CONFIG_H_MM_PAGE_TRACE) += mem_trace.o buddy_track.o slub_track.o
obj-$(CONFIG_H_MM_PAGE_TRACE_TEST) += mem_trace_test.o
obj-$(CONFIG_MM_MEM_OFFLINE) += mem_offline.o
ifndef CONFIG_MM_LTS_KERNEL
obj-$(CONFIG_HYPERHOLD) += memcg_control.o
obj-$(CONFIG_HYPERHOLD) += memcg_reclaim.o
endif
obj-$(CONFIG_VM_COPY) += vm_copy.o
obj-$(CONFIG_PARA_SWAPD) += parallel_swapd.o
obj-$(CONFIG_PROCESS_RECLAIM) += process_reclaim.o
obj-$(CONFIG_MM_SMART_RECLAIM) += smart_reclaim.o
obj-$(CONFIG_MM_AMA) += mem_ama.o
obj-$(CONFIG_MM_VIP) += mmvip_scene.o
