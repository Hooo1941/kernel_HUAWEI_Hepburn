obj-$(CONFIG_HKIP_MODULE_ALLOC)	+= hhee_module.o
obj-$(CONFIG_HHEE)		+= hhee_dev.o hhee_msg.o
obj-$(CONFIG_HHEE)		+= hkip/
obj-$(CONFIG_HHEE_DEBUG)	+= hhee_debugfs.o hhee_log.o
obj-$(CONFIG_HHEE_DEBUG)	+= hkip_test/
obj-$(CONFIG_HHEE_HIBERNATE)	+= hhee_hibernate.o

subdir-ccflags-y += -Wdate-time -Wextra -Wfloat-equal -Wformat=2 -Wshadow
