#===============================================================================
# @brief    cmake make file
# Copyright (c) @CompanyNameMagicTag 2023-2023. All rights reserved.
#===============================================================================
set(
    tiot_interface_src_list
    ${CMAKE_CURRENT_SOURCE_DIR}/board_interface/board_internal/tiot_board_xmit_ops.c
    ${CMAKE_CURRENT_SOURCE_DIR}/device_interface/tiot_service_interface.c
)

set(
    interface_header_lists
    "${CMAKE_CURRENT_SOURCE_DIR}/board_interface/board_internal"
    "${CMAKE_CURRENT_SOURCE_DIR}/board_interface/board_porting"
    "${CMAKE_CURRENT_SOURCE_DIR}/board_interface/osal_porting"
    "${CMAKE_CURRENT_SOURCE_DIR}/device_interface"
)

set(TIOT_HEADER_LIST "${TIOT_HEADER_LIST}" "${interface_header_lists}" CACHE INTERNAL "tiot header list" FORCE)
set(TIOT_SRC_LIST "${TIOT_SRC_LIST}" "${tiot_interface_src_list}" CACHE INTERNAL "tiot src list" FORCE)