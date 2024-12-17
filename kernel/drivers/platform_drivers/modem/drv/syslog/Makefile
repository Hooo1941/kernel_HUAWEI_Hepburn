# Core functionality.
-include $(srctree)/drivers/platform_drivers/modem/drv/product_config/product_config.mk
obj-y	+= syslog_print.o syslog_dmesg.o syslog_logser.o syslog_logmem.o syslog_msg.o

GCOV_PROFILE := y
GCOV_PROFILE_syslog_dmesg.o := n
GCOV_PROFILE_syslog_logser.o := n
GCOV_PROFILE_syslog_msg.o := n
GCOV_PROFILE_syslog_print.o := n
GCOV_PROFILE_syslog_logmem.o := n