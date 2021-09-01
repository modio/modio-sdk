# 
#  Copyright (C) 2021 mod.io Pty Ltd. <https://mod.io>
#  
#  This file is part of the mod.io SDK.
#  
#  Distributed under the MIT License. (See accompanying file LICENSE or 
#   view online at <https://github.com/modio/modio-sdk/blob/main/LICENSE>)
# 

include(split-compilation)
add_subdirectory(entities)
add_modio_implementation_file(${CMAKE_CURRENT_LIST_DIR}/ModioFilterParams.ipp)
add_modio_implementation_file(${CMAKE_CURRENT_LIST_DIR}/ModioInitializeOptions.ipp)
add_modio_implementation_file(${CMAKE_CURRENT_LIST_DIR}/ModioBuffer.ipp)
add_modio_implementation_file(${CMAKE_CURRENT_LIST_DIR}/ModioModCollectionEntry.ipp)
add_modio_implementation_file(${CMAKE_CURRENT_LIST_DIR}/ModioLogService.ipp)
add_modio_implementation_file(${CMAKE_CURRENT_LIST_DIR}/ModioReportParams.ipp)
add_public_header(${MODIO_TARGET_NAME} ${CMAKE_CURRENT_LIST_DIR}/ModioFilterParams.h)
add_public_header(${MODIO_TARGET_NAME} ${CMAKE_CURRENT_LIST_DIR}/ModioModCollectionEntry.h)
add_public_header(${MODIO_TARGET_NAME} ${CMAKE_CURRENT_LIST_DIR}/ModioStdTypes.h)
add_public_header(${MODIO_TARGET_NAME} ${CMAKE_CURRENT_LIST_DIR}/entities/ModioModDetails.h)
add_public_header(${MODIO_TARGET_NAME} ${CMAKE_CURRENT_LIST_DIR}/entities/ModioModTagOptions.h)
add_public_header(${MODIO_TARGET_NAME} ${CMAKE_CURRENT_LIST_DIR}/entities/ModioUser.h)
add_public_header(${MODIO_TARGET_NAME} ${CMAKE_CURRENT_LIST_DIR}/entities/ModioModInfoList.h)
add_public_header(${MODIO_TARGET_NAME} ${CMAKE_CURRENT_LIST_DIR}/ModioReportParams.h)


