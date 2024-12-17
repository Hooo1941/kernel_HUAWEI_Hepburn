# Copyright 2010 The Android Open Source Project

LOCAL_PATH:= $(call my-dir)

#
# -- All host/targets including windows
#
common_src_files := \
		lib/cache.c \
		lib/compress.c \
		lib/xattr.c \
		lib/compressor.c \
		lib/compressor_lz4hc.c \
		lib/io.c \
		lib/inode.c \
		lib/config.c \
		lib/compressor_lz4.c \
		lib/map.c
include $(CLEAR_VARS)
LOCAL_SRC_FILES := \
		$(common_src_files) \
		mkfs/main.c \
                lib/exclude.c \
                lib/data.c \
                lib/decompress.c \
                lib/namei.c \
                lib/super.c \
                lib/zmap.c

LOCAL_MODULE := make_erofs
LOCAL_C_INCLUDES := \
    $(LOCAL_PATH)/include \
    $(LOCAL_PATH)/include/erofs \
    $(LOCAL_PATH)/external/e2fsprogs/lib

LOCAL_SHARED_LIBRARIES += libcutils \
    libext2_uuid \
    libselinux
LOCAL_STATIC_LIBRARIES += \
    libext4_utils \
    libsparse \
    libz \
    liblz4
LOCAL_LDLIBS_windows += -lws2_32
LOCAL_SHARED_LIBRARIES_darwin += libselinux
LOCAL_SHARED_LIBRARIES_linux  += libselinux
LOCAL_CFLAGS_darwin := -DHOST
LOCAL_CFLAGS_linux := -DHOST
LOCAL_CFLAGS += \
    -DEROFS_SUPPORT_COMPR_MODE_3 \
    -fomit-frame-pointer \
    -Wall \
    -Werror \
    -Wsign-compare \
    -DLZ4_ENABLED=1 \
    -DLZ4HC_ENABLED=1 \
    -Wno-unused-parameter \
    -DHAVE_FALLOCATE \
    -DHAVE_LINUX_TYPES_H \
    -DHAVE_LIBSELINUX \
    -DHAVE_LIBUUID \
    -DWITH_ANDROID \
    -DEROFS_UTILS121 \
    -Wno-pointer-arith

ifeq ($(shell getconf LONG_BIT), 32)
LOCAL_CFLAGS += -D_FILE_OFFSET_BITS=64
endif

LOCAL_IS_HOST_MODULE := true
include $(BUILD_HOST_EXECUTABLE)

include $(CLEAR_VARS)
LOCAL_SRC_FILES := \
		$(common_src_files) \
		mkfs/main.c

LOCAL_MODULE := make_erofs_android
LOCAL_C_INCLUDES := \
    $(LOCAL_PATH)/include \
    $(LOCAL_PATH)/include/erofs \
    $(LOCAL_PATH)/external/e2fsprogs/lib

LOCAL_STATIC_LIBRARIES += \
    libcutils \
    libext4_utils \
    libsparse \
    libz \
    liblz4 \
    libselinux
LOCAL_LDLIBS_windows += -lws2_32

LOCAL_CFLAGS_darwin := -DHOST
LOCAL_CFLAGS_linux := -DHOST
LOCAL_CFLAGS += \
    -DEROFS_SUPPORT_COMPR_MODE_3 \
    -DCONFIG_EROFS_READ_XATTR_FROM_DIR \
    -fomit-frame-pointer \
    -Wall \
    -Werror \
    -Wsign-compare \
    -DLZ4_ENABLED=1 \
    -DLZ4HC_ENABLED=1 \
    -Wno-unused-parameter \
    -DHAVE_FALLOCATE \
    -DHAVE_LINUX_TYPES_H \
    -DHAVE_LIBSELINUX \
    -DHAVE_LIBUUID \
    -DWITH_ANDROID \
    -DEROFS_UTILS121 \
    -Wno-pointer-arith

ifeq ($(shell getconf LONG_BIT), 32)
LOCAL_CFLAGS += -D_FILE_OFFSET_BITS=64
endif
LOCAL_FORCE_STATIC_EXECUTABLE := true
LOCAL_LDFLAGS += -static
LOCAL_IS_HOST_MODULE := true
include $(BUILD_HOST_EXECUTABLE)


include $(CLEAR_VARS)
LOCAL_SRC_FILES := \
		$(common_src_files) \
		mkfs/main.c \
		lib/patch.c \
        lib/exclude.c

LOCAL_MODULE := make_erofs_patch
LOCAL_C_INCLUDES := \
    $(LOCAL_PATH)/include \
    $(LOCAL_PATH)/include/erofs \
    $(LOCAL_PATH)/external/e2fsprogs/lib

LOCAL_SHARED_LIBRARIES += libcutils \
    libext2_uuid
LOCAL_STATIC_LIBRARIES += \
    libext4_utils \
    libsparse \
    libz \
    liblz4
LOCAL_LDLIBS_windows += -lws2_32
LOCAL_SHARED_LIBRARIES_darwin += libselinux
LOCAL_SHARED_LIBRARIES_linux  += libselinux
LOCAL_CFLAGS_darwin := -DHOST
LOCAL_CFLAGS_linux := -DHOST
LOCAL_CFLAGS += \
    -DEROFS_SUPPORT_COMPR_MODE_3 \
    -fomit-frame-pointer \
    -Wall \
    -Werror \
    -Wsign-compare \
    -DLZ4_ENABLED=1 \
    -DLZ4HC_ENABLED=1 \
    -DPATCH_ENABLED=1 \
    -Wno-unused-parameter \
    -DHAVE_FALLOCATE \
    -DHAVE_LINUX_TYPES_H \
    -DHAVE_LIBSELINUX \
    -DHAVE_LIBUUID \
    -DWITH_ANDROID \
    -DEROFS_UTILS121 \
    -Wno-pointer-arith

ifeq ($(shell getconf LONG_BIT), 32)
LOCAL_CFLAGS += -D_FILE_OFFSET_BITS=64
endif

LOCAL_IS_HOST_MODULE := true
include $(BUILD_HOST_EXECUTABLE)

include $(CLEAR_VARS)
LOCAL_CLANG := true
LOCAL_MODULE := erofs_diff_compress
LOCAL_SRC_FILES :=  $(common_src_files) \
                    diff/erofs_diff_main.cpp
LOCAL_C_INCLUDES := \
    $(LOCAL_PATH)/include \
    $(LOCAL_PATH)/lz4-lib \
    vendor/huawei/chipset_common/modules/hwrecovery/dload/huawei_update_binary \
    vendor/huawei/chipset_common/modules/hwrecovery/recovery_comm_lib

LOCAL_CFLAGS += \
    -DEROFS_SUPPORT_COMPR_MODE_3 \
    -DCONFIG_EROFS_READ_XATTR_FROM_DIR \
    -fomit-frame-pointer \
    -Wall -O1 \
    -Wsign-compare \
    -DLZ4_ENABLED=1 \
    -DLZ4HC_ENABLED=1 \
    -DPATCH_ENABLED=1 \
    -Wno-unused-parameter \
    -DHAVE_FALLOCATE \
    -DHAVE_LINUX_TYPES_H \
    -DHAVE_LIBSELINUX \
    -DHAVE_LIBUUID \
    -DWITH_ANDROID \
    -DEROFS_UTILS121 \
    -Wno-pointer-arith

LOCAL_STATIC_LIBRARIES += \
    lib_rssrange lib_hwotafault lib_volumn_utils libc_secstatic libcrypto_static liblog libcutils \
    libext4_utils \
    libfec libc_secstatic libcrypto_static libselinux libsparse \
    liblz4 \
    libselinux \
    liblog libbase

# used in recovery and must be static-linked.
LOCAL_FORCE_STATIC_EXECUTABLE := true
# generate in "recovery_ramdisk/sbin"
LOCAL_MODULE_PATH := $(PRODUCT_OUT)
include $(BUILD_EXECUTABLE)

include $(CLEAR_VARS)
LOCAL_MODULE := mkerofsuserimg.sh
LOCAL_SRC_FILES := mkerofsuserimg.sh
LOCAL_MODULE_CLASS := EXECUTABLES
# We don't need any additional suffix.
LOCAL_MODULE_SUFFIX :=
LOCAL_BUILT_MODULE_STEM := $(notdir $(LOCAL_SRC_FILES))
LOCAL_IS_HOST_MODULE := true
include $(BUILD_PREBUILT)
