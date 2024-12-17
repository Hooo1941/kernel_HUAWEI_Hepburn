#
# Makefile for the memory technology device drivers.
#
# Core functionality.

ifeq ($(strip $(llt_gcov)),y)
GCOV_PROFILE := y
endif

obj-y				+= dump_log_strategy.o 
obj-y				+= dump_logs.o
obj-y				+= dump_reset_log.o
obj-y				+= dump_sec_log.o
obj-y				+= dump_teeos_service.o
obj-y                           += dump_log_parser.o
obj-y				+= dump_log_uploader.o
obj-y				+= dump_genl.o
obj-y				+= dump_extra_logs.o
obj-y                           += dump_avs_escape.o

GCOV_PROFILE := y
GCOV_PROFILE_dump_log_uploader.o := n
GCOV_PROFILE_dump_teeos_service.o := n
GCOV_PROFILE_dump_log_strategy.o := n
GCOV_PROFILE_dump_reset_log.o := n
GCOV_PROFILE_dump_genl.o := n
GCOV_PROFILE_dump_logs.o := n
GCOV_PROFILE_dump_sec_log.o := n
GCOV_PROFILE_dump_log_parser.o := n
GCOV_PROFILE_dump_extra_logs.o := n
