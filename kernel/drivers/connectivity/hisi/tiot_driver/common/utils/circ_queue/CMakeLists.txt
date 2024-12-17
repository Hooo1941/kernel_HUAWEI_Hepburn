#===============================================================================
# @brief    cmake make file
# Copyright (c) @CompanyNameMagicTag 2023-2023. All rights reserved.
#===============================================================================
set(
    tiot_circ_queue_src_list
    ${CMAKE_CURRENT_SOURCE_DIR}/tiot_circ_queue.c
)

set(TIOT_HEADER_LIST "${TIOT_HEADER_LIST}" "${CMAKE_CURRENT_SOURCE_DIR}" CACHE INTERNAL "tiot header list" FORCE)
set(TIOT_SRC_LIST "${TIOT_SRC_LIST}" "${tiot_circ_queue_src_list}" CACHE INTERNAL "tiot src list" FORCE)