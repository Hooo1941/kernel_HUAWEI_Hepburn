#===============================================================================
# @brief    cmake make file
# Copyright (c) @CompanyNameMagicTag 2023-2023. All rights reserved.
#===============================================================================
set(
    utils_header_lists
    "${CMAKE_CURRENT_SOURCE_DIR}"
    "${CMAKE_CURRENT_SOURCE_DIR}/circ_buf"
    "${CMAKE_CURRENT_SOURCE_DIR}/list"
)

set(TIOT_HEADER_LIST "${TIOT_HEADER_LIST}" "${utils_header_lists}" CACHE INTERNAL "tiot header list" FORCE)
add_subdirectory(circ_queue)
add_subdirectory(fileops)
