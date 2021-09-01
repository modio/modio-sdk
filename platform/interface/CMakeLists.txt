# 
#  Copyright (C) 2021 mod.io Pty Ltd. <https://mod.io>
#  
#  This file is part of the mod.io SDK.
#  
#  Distributed under the MIT License. (See accompanying file LICENSE or 
#   view online at <https://github.com/modio/modio-sdk/blob/main/LICENSE>)
# 

if (NOT TARGET ${MODIO_TARGET_NAME})
add_library (${MODIO_TARGET_NAME} INTERFACE)
endif()

target_include_directories(${MODIO_TARGET_NAME} INTERFACE ${CMAKE_CURRENT_LIST_DIR})

	
install(
	DIRECTORY ${CMAKE_CURRENT_LIST_DIR}/
	DESTINATION ${CMAKE_INSTALL_PREFIX}/header_only/common
	COMPONENT header_only
	
)
