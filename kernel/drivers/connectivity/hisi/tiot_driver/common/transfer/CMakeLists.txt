set(
    tiot_transfer_src_list
    ${CMAKE_CURRENT_SOURCE_DIR}/tiot_xfer.c
    ${CMAKE_CURRENT_SOURCE_DIR}/tiot_xfer_utils.c
)

set(
    tiot_transfer_header_list
    ${CMAKE_CURRENT_SOURCE_DIR}
)

if(DEFINED CONFIG_XFER_PACKET_TIOT)
add_subdirectory(packet_handle/tiot_packet)
endif()

if(DEFINED CONFIG_XFER_PACKET_NCI)
add_subdirectory(packet_handle/nci_packet)
endif()

set(TIOT_SRC_LIST "${TIOT_SRC_LIST}" "${tiot_transfer_src_list}" CACHE INTERNAL "tiot src list" FORCE)
set(TIOT_HEADER_LIST "${TIOT_HEADER_LIST}" "${tiot_transfer_header_list}" CACHE INTERNAL "tiot header list" FORCE)