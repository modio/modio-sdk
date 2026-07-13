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

	if (MODIO_BUILD_DYNAMIC_LIBRARY)
		if (NOT TARGET ${MODIO_TARGET_NAME}Shared)
			add_library(${MODIO_TARGET_NAME}Shared SHARED EXCLUDE_FROM_ALL)
		endif()
		target_compile_definitions(${MODIO_TARGET_NAME}Shared PUBLIC  -DMODIO_DLL_IMPORT=1)
		target_compile_definitions(${MODIO_TARGET_NAME}Shared PRIVATE -DMODIO_DLL_EXPORT=1)
		target_link_libraries(${MODIO_TARGET_NAME}Shared PUBLIC "$<$<CONFIG:DEBUG>:msvcrtd.lib>" "$<$<CONFIG:RELEASE>:msvcrt.lib>")
	endif()
	
	
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
