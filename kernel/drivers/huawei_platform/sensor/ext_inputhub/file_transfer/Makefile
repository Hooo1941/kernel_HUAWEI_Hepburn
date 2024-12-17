ccflags-y += -I$(srctree)/drivers/huawei_platform/sensor/ext_inputhub/default/
ccflags-y += -I$(srctree)/drivers/huawei_platform/sensor/ext_inputhub/default/inner_cmd/
ccflags-y += -I$(srctree)/drivers/huawei_platform/sensor/ext_inputhub/file_transfer/commu/
ccflags-y += -I$(srctree)/drivers/huawei_platform/sensor/ext_inputhub/file_transfer/transfer_work/
ccflags-y += -I$(srctree)/drivers/huawei_platform/sensor/ext_inputhub/file_transfer/transfer_work/steps/
ccflags-y += -I$(srctree)/drivers/huawei_platform/sensor/ext_inputhub/file_transfer/transfer_work/processors/steps/

#file transfer Driver
obj-$(CONFIG_EXT_INPUTHUB) += transfer_scheduler.o
obj-$(CONFIG_EXT_INPUTHUB) += transfer_topic.o
obj-$(CONFIG_EXT_INPUTHUB) += transfer_buffer.o
obj-$(CONFIG_EXT_INPUTHUB) += file_mgr.o
obj-$(CONFIG_EXT_INPUTHUB) += transfer_record.o
#transfer communication
obj-$(CONFIG_EXT_INPUTHUB) += commu/transfer_inner_commu.o
obj-$(CONFIG_EXT_INPUTHUB) += commu/transfer_commu.o
# transfer works
obj-$(CONFIG_EXT_INPUTHUB) += transfer_work/transfer_work.o
obj-$(CONFIG_EXT_INPUTHUB) += transfer_work/transfer_processor.o
# transfer processors
obj-$(CONFIG_EXT_INPUTHUB) += transfer_work/processors/transfer_file.o
obj-$(CONFIG_EXT_INPUTHUB) += transfer_work/processors/fetch_file.o
obj-$(CONFIG_EXT_INPUTHUB) += transfer_work/processors/remote_transfer.o
obj-$(CONFIG_EXT_INPUTHUB) += transfer_work/processors/remote_fetch.o
obj-$(CONFIG_EXT_INPUTHUB) += transfer_work/processors/transfer_buffer.o
obj-$(CONFIG_EXT_INPUTHUB) += transfer_work/processors/remote_transfer_buffer.o
# transfer processors steps
obj-$(CONFIG_EXT_INPUTHUB) += transfer_work/processors/steps/handshake.o
obj-$(CONFIG_EXT_INPUTHUB) += transfer_work/processors/steps/file_process.o
obj-$(CONFIG_EXT_INPUTHUB) += transfer_work/processors/steps/data_transfer.o
obj-$(CONFIG_EXT_INPUTHUB) += transfer_work/processors/steps/inner_event.o
