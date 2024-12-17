# This is the top of TIoT driver core Makefile.
# 编译形态，builts-in, module, 不编译
CONFIG_TIOT_DRIVER ?= m
TIOT_PORTING_DIR ?= product_porting
TIOT_PORTING_CONFIG_DIR ?= product_porting
KERNEL_DIR ?=
MODULE_PARAM ?=
TIOT_OUT_OF_TREE_BUILD=n

# 当前目录
PWD := $(abspath $(lastword $(MAKEFILE_LIST)))
# 指定TIoT驱动目录，用于Out of tree编译
TIOT_DRIVER_PATH ?= $(dir $(PWD))
$(warning pwd = $(PWD))

# 寻找所有可用devices
TIOT_TARGET_DEVICES := $(patsubst $(TIOT_DRIVER_PATH)/device/%,%,$(wildcard $(TIOT_DRIVER_PATH)/device/*))
$(warning device lists = $(TIOT_TARGET_DEVICES))

ifneq ($(KERNEL_DIR),)
ifeq ($(KERNELRELEASE),)
ifeq ($(CONFIG_TIOT_DRIVER),m)
TIOT_OUT_OF_TREE_BUILD=y
endif
endif
endif

# 对于ko编译，首先进入内核源码，再通过kbuild系统进入此Makefile
ifeq ($(TIOT_OUT_OF_TREE_BUILD),y)
# 进入内核目录，之后再回到当前目录
$(warning Module param $(MODULE_PARAM))
default:
	$(MAKE) -C $(KERNEL_DIR) $(MODULE_PARAM) M=$(TIOT_DRIVER_PATH) modules

clean:
	$(MAKE) -C $(KERNEL_DIR) M=$(TIOT_DRIVER_PATH) clean
	-find device/ -type f -name "*.o" -exec rm -f {} \;
	-find device/ -type f -name "*.o.cmd" -exec rm -f {} \;
	-find device/ -type f -name "*.o.d" -exec rm -f {} \;
	-find device/ -type f -name "*.o.symversions" -exec rm -f {} \;
	-find common/ -type f -name "*.o" -exec rm -f {} \;
	-find common/ -type f -name "*.o.cmd" -exec rm -f {} \;
	-find common/ -type f -name "*.o.d" -exec rm -f {} \;
	-find common/ -type f -name "*.o.symversions" -exec rm -f {} \;
else
# 此处为第二次进入
$(warning twice enter)
include $(TIOT_DRIVER_PATH)/$(TIOT_PORTING_CONFIG_DIR)/tiot_defconfig

# Devices
$(foreach dev,$(TIOT_TARGET_DEVICES),$(eval -include $(TIOT_DRIVER_PATH)/device/$(dev)/Makefile))

# Common
tiot-controller-objs += tiot_controller.o
tiot-controller-objs := $(addprefix common/controller/,$(tiot-controller-objs))

tiot-interface-objs += board_interface/board_internal/tiot_board_xmit_ops.o
tiot-interface-objs += device_interface/tiot_service_interface.o
tiot-interface-objs := $(addprefix common/interface/,$(tiot-interface-objs))

tiot-fw-objs += tiot_firmware.o
ifeq ($(CONFIG_FIRMWARE_CFG_HANDLE_PRIV), y)
tiot-fw-priv-objs += tiot_cfg_handle_priv.o
tiot-fw-priv-objs := $(addprefix cfg_handle_priv/,$(tiot-fw-priv-objs))
tiot-fw-objs += ${tiot-fw-priv-objs}
endif
ifeq ($(CONFIG_FIRMWARE_CFG_HANDLE_XCI), y)
tiot-fw-xci-objs += tiot_cfg_handle_xci.o
tiot-fw-xci-objs := $(addprefix cfg_handle_xci/,$(tiot-fw-xci-objs))
tiot-fw-objs += ${tiot-fw-xci-objs}
endif
ifeq ($(CONFIG_FIRMWARE_CFG_HANDLE_HIBURN), y)
tiot-fw-hiburn-objs += cfg_handle_hiburn/tiot_cfg_handle_hiburn.o
tiot-fw-objs += ${tiot-fw-hiburn-objs}
endif
tiot-fw-objs := $(addprefix common/fw_load/,$(tiot-fw-objs))

tiot-pm-objs += tiot_pm.o tiot_pm_wakelock.o
tiot-pm-objs += pm_default/tiot_pm_default.o
ifeq ($(CONFIG_DEV_TIOT_SLAVE), y)
tiot-pm-objs += pm_slave/tiot_pm_slave.o
endif
tiot-pm-objs := $(addprefix common/pm/,$(tiot-pm-objs))

tiot-transfer-objs += tiot_xfer.o tiot_xfer_utils.o
ifeq ($(CONFIG_XFER_PACKET_TIOT), y)
tiot-packet-objs += tiot_packet.o tiot_packet_manager.o
tiot-packet-objs := $(addprefix packet_handle/tiot_packet/,$(tiot-packet-objs))
tiot-transfer-objs += $(tiot-packet-objs)
endif
ifeq ($(CONFIG_XFER_PACKET_NCI), y)
tiot-nci-packet-objs += tiot_nci_packet.o
tiot-nci-packet-objs := $(addprefix packet_handle/nci_packet/,$(tiot-nci-packet-objs))
tiot-transfer-objs += $(tiot-nci-packet-objs)
endif
tiot-transfer-objs := $(addprefix common/transfer/,$(tiot-transfer-objs))

tiot-sys-msg-handler-objs += tiot_sys_msg_handle.o
ifeq ($(CONFIG_DEV_TIOT_SLAVE), y)
tiot-sys-msg-handler-objs += tiot_sys_msg_slave_handle.o
endif
tiot-sys-msg-handler-objs := $(addprefix common/sys_msg_handler/,$(tiot-sys-msg-handler-objs))

# Utils
tiot-circ-queue-objs += circ_queue/tiot_circ_queue.o
tiot-fileops-objs += fileops/tiot_fileops.o

tiot-utils-objs += $(tiot-circ-queue-objs)
tiot-utils-objs += $(tiot-fileops-objs)
tiot-utils-objs := $(addprefix common/utils/,$(tiot-utils-objs))

ifneq ($(CONFIG_LIBBOUNDSCHECK_IMPLEMENTED), y)
# libboundscheck
tiot-boundscheck-objs += memcpy_s.o memmove_s.o memset_s.o securecutil.o secureprintoutput_a.o \
						 snprintf_s.o sprintf_s.o strcat_s.o strcpy_s.o strncat_s.o strncpy_s.o \
						 vsnprintf_s.o vsprintf_s.o strtok_s.o sscanf_s.o vsscanf_s.o secureinput_a.o
tiot-boundscheck-objs := $(addprefix common/libboundscheck/src/,$(tiot-boundscheck-objs))
endif

# Porting
include $(TIOT_DRIVER_PATH)/$(TIOT_PORTING_DIR)/Makefile

# All
tiot_driver-objs += $(tiot-controller-objs)

tiot_driver-objs += $(tiot-interface-objs)

tiot_driver-objs += $(tiot-fw-objs)

tiot_driver-objs += $(tiot-pm-objs)

tiot_driver-objs += $(tiot-transfer-objs)

tiot_driver-objs += $(tiot-sys-msg-handler-objs)

tiot_driver-objs += $(tiot-utils-objs)

tiot_driver-objs += $(tiot-porting-objs)

ifneq ($(CONFIG_LIBBOUNDSCHECK_IMPLEMENTED), y)
tiot_driver-objs += $(tiot-boundscheck-objs)
endif

obj-$(CONFIG_TIOT_DRIVER) += tiot_driver.o

TIOT_SRC_LIST := $(tiot_driver-objs)

TIOT_HEADER_LIST += -I$(TIOT_DRIVER_PATH)/common/controller
TIOT_HEADER_LIST += -I$(TIOT_DRIVER_PATH)/common/fw_load
ifeq ($(CONFIG_FIRMWARE_CFG_HANDLE_PRIV), y)
TIOT_HEADER_LIST += -I$(TIOT_DRIVER_PATH)/common/fw_load/cfg_handle_priv
endif
ifeq ($(CONFIG_FIRMWARE_CFG_HANDLE_XCI), y)
TIOT_HEADER_LIST += -I$(TIOT_DRIVER_PATH)/common/fw_load/cfg_handle_xci
endif
ifeq ($(CONFIG_FIRMWARE_CFG_HANDLE_HIBURN), y)
TIOT_HEADER_LIST += -I$(TIOT_DRIVER_PATH)/common/fw_load/cfg_handle_hiburn
endif
TIOT_HEADER_LIST += -I$(TIOT_DRIVER_PATH)/common/interface/board_interface/board_internal
TIOT_HEADER_LIST += -I$(TIOT_DRIVER_PATH)/common/interface/board_interface/board_porting
TIOT_HEADER_LIST += -I$(TIOT_DRIVER_PATH)/common/interface/board_interface/osal_porting
TIOT_HEADER_LIST += -I$(TIOT_DRIVER_PATH)/common/interface/device_interface
TIOT_HEADER_LIST += -I$(TIOT_DRIVER_PATH)/common/pm
TIOT_HEADER_LIST += -I$(TIOT_DRIVER_PATH)/common/pm/pm_default
ifeq ($(CONFIG_DEV_TIOT_SLAVE), y)
TIOT_HEADER_LIST += -I$(TIOT_DRIVER_PATH)/common/pm/pm_slave
endif
TIOT_HEADER_LIST += -I$(TIOT_DRIVER_PATH)/common/transfer
ifeq ($(CONFIG_XFER_PACKET_TIOT), y)
TIOT_HEADER_LIST += -I$(TIOT_DRIVER_PATH)/common/transfer/packet_handle/tiot_packet
endif
ifeq ($(CONFIG_XFER_PACKET_NCI), y)
TIOT_HEADER_LIST += -I$(TIOT_DRIVER_PATH)/common/transfer/packet_handle/nci_packet
endif
TIOT_HEADER_LIST += -I$(TIOT_DRIVER_PATH)/common/sys_msg_handler
TIOT_HEADER_LIST += -I$(TIOT_DRIVER_PATH)/common/utils
TIOT_HEADER_LIST += -I$(TIOT_DRIVER_PATH)/common/utils/list
TIOT_HEADER_LIST += -I$(TIOT_DRIVER_PATH)/common/utils/circ_buf
TIOT_HEADER_LIST += -I$(TIOT_DRIVER_PATH)/common/utils/circ_queue
TIOT_HEADER_LIST += -I$(TIOT_DRIVER_PATH)/common/utils/fileops
TIOT_HEADER_LIST += -I$(TIOT_DRIVER_PATH)/common/libboundscheck/include

TIOT_CFLAGS += -include $(TIOT_DRIVER_PATH)/$(TIOT_PORTING_CONFIG_DIR)/tiot_autoconfig.h
TIOT_CFLAGS += $(TIOT_HEADER_LIST)

# We use c99, suppress some warning for kernel c89.
subdir-ccflags-y := $(TIOT_CFLAGS) -Wno-error=declaration-after-statement

$(warning TIOT_SRC_LIST: $(TIOT_SRC_LIST))

endif
