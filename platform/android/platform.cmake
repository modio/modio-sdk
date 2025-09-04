# 
#  Copyright (C) 2021 mod.io Pty Ltd. <https://mod.io>
#  
#  This file is part of the mod.io SDK.
#  
#  Distributed under the MIT License. (See accompanying file LICENSE or 
#   view online at <https://github.com/modio/modio-sdk/blob/main/LICENSE>)
#   
# 

if(MODIO_PLATFORM STREQUAL "ANDROID" OR MODIO_PLATFORM STREQUAL "OCULUS")

	if(MODIO_SET_TOOLCHAIN)

		modio_list_append_unique(CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/cmake")
		set (CMAKE_MODULE_PATH "${CMAKE_MODULE_PATH}" CACHE INTERNAL "")

		# Look for CMAKE_ANDROID_NDK and fall back to env var is its empty
		if (DEFINED CMAKE_ANDROID_NDK)
			set (CMAKE_TOOLCHAIN_FILE ${CMAKE_ANDROID_NDK}/build/cmake/android.toolchain.cmake CACHE INTERNAL "toolchain file")
		else()
			set (CMAKE_ANDROID_NDK $ENV{MODIO_NDK_ROOT})	
			set (CMAKE_TOOLCHAIN_FILE ${CMAKE_ANDROID_NDK}/build/cmake/android.toolchain.cmake CACHE INTERNAL "toolchain file")
		endif()
	elseif(MODIO_LOAD_PLATFORM_MODULES)
		add_subdirectory(${CMAKE_CURRENT_LIST_DIR})
	endif()

endif()
