# 
#  Copyright (C) 2021 mod.io Pty Ltd. <https://mod.io>
#  
#  This file is part of the mod.io SDK.
#  
#  Distributed under the MIT License. (See accompanying file LICENSE or 
#   view online at <https://github.com/modio/modio-sdk/blob/main/LICENSE>)
# 

include(split-compilation)
add_modio_implementation_file(${CMAKE_CURRENT_LIST_DIR}/SDKCore.ipp)
add_modio_implementation_file(${CMAKE_CURRENT_LIST_DIR}/SDKModManagement.ipp)
add_modio_implementation_file(${CMAKE_CURRENT_LIST_DIR}/SDKModMetadata.ipp)
add_modio_implementation_file(${CMAKE_CURRENT_LIST_DIR}/SDKUserData.ipp)
add_modio_implementation_file(${CMAKE_CURRENT_LIST_DIR}/detail/ModioObjectTrack.ipp)
