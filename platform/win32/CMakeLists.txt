# 
#  Copyright (C) 2021 mod.io Pty Ltd. <https://mod.io>
#  
#  This file is part of the mod.io SDK.
#  
#  Distributed under the MIT License. (See accompanying file LICENSE or 
#   view online at <https://github.com/modio/modio-sdk/blob/main/LICENSE>)
# 

if(MODIO_PLATFORM STREQUAL "WIN")
	message("Initializing modio-WIN")
	modio_list_append_unique(CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/cmake")
    set(CMAKE_MODULE_PATH "${CMAKE_MODULE_PATH}" CACHE INTERNAL "")
	set (MODIO_PLATFORM_WIN ON INTERNAL "building windows")

	enable_language(C)
	enable_language(CXX)

	add_subdirectory(win32)

	
	
	add_subdirectory(../ms-common ../ms-common)

	if (MODIO_BUILD_TESTS)
		add_subdirectory(tests)
	endif()

	install(
		DIRECTORY ${CMAKE_CURRENT_LIST_DIR}/win32
		DESTINATION ${CMAKE_INSTALL_PREFIX}/header_only
		COMPONENT header_only
	)
endif()
