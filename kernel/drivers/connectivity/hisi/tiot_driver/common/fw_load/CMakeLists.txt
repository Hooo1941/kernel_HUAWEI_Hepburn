#===============================================================================
# @brief    cmake make file
# Copyright (c) @CompanyNameMagicTag 2023-2023. All rights reserved.
#===============================================================================
set(
    tiot_firmware_src_list
    ${CMAKE_CURRENT_SOURCE_DIR}/tiot_firmware.c
)

set(
    tiot_firmware_header_list
    ${CMAKE_CURRENT_SOURCE_DIR}
)

if(CONFIG_FIRMWARE_CFG_HANDLE_PRIV)
list(APPEND
     tiot_firmware_src_list
     ${CMAKE_CURRENT_SOURCE_DIR}/cfg_handle_priv/tiot_cfg_handle_priv.c)

list(APPEND
     tiot_firmware_header_list
     ${CMAKE_CURRENT_SOURCE_DIR}/cfg_handle_priv)
endif()

if(CONFIG_FIRMWARE_CFG_HANDLE_XCI)
list(APPEND
     tiot_firmware_src_list
     ${CMAKE_CURRENT_SOURCE_DIR}/cfg_handle_xci/tiot_cfg_handle_xci.c)

list(APPEND
     tiot_firmware_header_list
     ${CMAKE_CURRENT_SOURCE_DIR}/cfg_handle_xci)
endif()

if(CONFIG_FIRMWARE_CFG_HANDLE_HIBURN)
list(APPEND
     tiot_firmware_src_list
     ${CMAKE_CURRENT_SOURCE_DIR}/cfg_handle_hiburn/tiot_cfg_handle_hiburn.c)

list(APPEND
     tiot_firmware_header_list
     ${CMAKE_CURRENT_SOURCE_DIR}/cfg_handle_hiburn)
endif()

set(TIOT_HEADER_LIST "${TIOT_HEADER_LIST}" "${tiot_firmware_header_list}" CACHE INTERNAL "tiot header list" FORCE)
set(TIOT_SRC_LIST "${TIOT_SRC_LIST}" "${tiot_firmware_src_list}" CACHE INTERNAL "tiot src list" FORCE)