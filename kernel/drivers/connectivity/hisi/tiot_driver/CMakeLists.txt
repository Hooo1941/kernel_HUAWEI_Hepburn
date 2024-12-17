#===============================================================================
# @brief    cmake make file
# Copyright (c) @CompanyNameMagicTag 2023-2023. All rights reserved.
#===============================================================================
# 此处为TIoT驱动顶层CMake

# CMake读取Kconfig配置
function(KCONFIG_GET_PARAMS KCONFIG_PATH)
    file(STRINGS ${KCONFIG_PATH} TIOT_CONFIG_LIST ENCODING "UTF-8")

    foreach (CONFIG ${TIOT_CONFIG_LIST})
        # CONFIG maybe: CONFIG_CC_STACKPROTECTOR_STRONG=y

        # The first: variable name
        string(REGEX MATCH "[^=]+" CONFIG_VAR_NAME ${CONFIG})

        # The second: variable value
        string(REGEX MATCH "=(.+$)" CONFIG_VAR_VAL ${CONFIG})
        set(CONFIG_VAR_VAL ${CMAKE_MATCH_1})
        if(CONFIG_VAR_VAL MATCHES "^y$")
            set(${CONFIG_VAR_NAME} ${CONFIG_VAR_VAL} PARENT_SCOPE)
        endif()
    endforeach()
endfunction()

macro(add_subdirectory_if_exist _path)
    if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/${_path}/CMakeLists.txt)
        add_subdirectory(${_path})
    endif()
endmacro()

macro(list_sub_dirs result curdir)
  file(GLOB children RELATIVE ${curdir} ${curdir}/*)
  set(dirlist "")
  foreach(child ${children})
    if(IS_DIRECTORY ${curdir}/${child})
      list(APPEND dirlist ${child})
    endif()
  endforeach()
  set(${result} ${dirlist})
endmacro()

if(NOT DEFINED TIOT_COMMON_DIR)
    set(TIOT_COMMON_DIR "${CMAKE_CURRENT_SOURCE_DIR}/common")
endif()

if(NOT DEFINED TIOT_PORTING_DIR)
    set(TIOT_PORTING_DIR "${CMAKE_CURRENT_SOURCE_DIR}/product_porting")
endif()

if(NOT DEFINED TIOT_PORTING_CONFIG_DIR)
    set(TIOT_PORTING_CONFIG_DIR "${CMAKE_CURRENT_SOURCE_DIR}/product_porting")
endif()

if(NOT DEFINED TIOT_DEVICE_LIST)
    list_sub_dirs(TIOT_DEVICE_LIST ${CMAKE_CURRENT_SOURCE_DIR}/device)
endif()

# 初始化CMake全局变量
set(TIOT_CFLAGS "SHELL:-include ${TIOT_PORTING_CONFIG_DIR}/tiot_autoconfig.h" CACHE INTERNAL "tiot c flags" FORCE)
# all header and files listed here
set(TIOT_HEADER_LIST "" CACHE INTERNAL "tiot header list" FORCE)
set(TIOT_SRC_LIST "" CACHE INTERNAL "tiot src list" FORCE)

# 获取Kconfig配置
KCONFIG_GET_PARAMS(${TIOT_PORTING_CONFIG_DIR}/tiot_defconfig)

# devices
foreach(tiot_device ${TIOT_DEVICE_LIST})
    add_subdirectory_if_exist(device/${tiot_device})
endforeach()
# common
add_subdirectory("${TIOT_COMMON_DIR}" common.out)
# porting
add_subdirectory("${TIOT_PORTING_DIR}" porting.out)
