# 
#  Copyright (C) 2021 mod.io Pty Ltd. <https://mod.io>
#  
#  This file is part of the mod.io SDK.
#  
#  Distributed under the MIT License. (See accompanying file LICENSE or 
#   view online at <https://github.com/modio/modio-sdk/blob/main/LICENSE>)
# 

include(split-compilation)
add_modio_implementation_file(${CMAKE_CURRENT_LIST_DIR}/ModioSDKSessionData.ipp)
add_modio_implementation_file(${CMAKE_CURRENT_LIST_DIR}/ModioJsonHelpers.ipp)
add_modio_implementation_file(${CMAKE_CURRENT_LIST_DIR}/compression/zip/ArchiveFileImplementation.ipp)
add_modio_implementation_file(${CMAKE_CURRENT_LIST_DIR}/compression/zlib/detail/deflate_stream.ipp)
add_modio_implementation_file(${CMAKE_CURRENT_LIST_DIR}/compression/zlib/detail/inflate_stream.ipp)
add_modio_implementation_file(${CMAKE_CURRENT_LIST_DIR}/userdata/ModioUserDataContainer.ipp)
add_modio_implementation_file(${CMAKE_CURRENT_LIST_DIR}/userdata/ModioUserProfile.ipp)
add_public_header(${MODIO_TARGET_NAME} ${CMAKE_CURRENT_LIST_DIR}/ModioDefines.h)
add_public_header(${MODIO_TARGET_NAME} ${CMAKE_CURRENT_LIST_DIR}/ModioLibraryConfigurationHelpers.h)
