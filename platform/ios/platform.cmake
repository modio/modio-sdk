# 
#  Copyright (C) 2021 mod.io Pty Ltd. <https://mod.io>
#  
#  This file is part of the mod.io SDK.
#  
#  Distributed under the MIT License. (See accompanying file LICENSE or 
#   view online at <https://github.com/modio/modio-sdk/blob/main/LICENSE>)
#   
# 

if(MODIO_PLATFORM STREQUAL "IOS")

	if(MODIO_SET_TOOLCHAIN)
		#set(CMAKE_COMPILE_WARNING_AS_ERROR ON)
		modio_list_append_unique(CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/cmake")
		set(CMAKE_MODULE_PATH "${CMAKE_MODULE_PATH}" CACHE INTERNAL "")
		set (MODIO_PLATFORM_IOS ON INTERNAL "building for iOS")

		## `IPHONEOS_DEPLOYMENT_TARGET` refers to the oldest version to allow deployment of the mod.io SDK
		set (CMAKE_XCODE_ATTRIBUTE_IPHONEOS_DEPLOYMENT_TARGET "15.0" CACHE STRING "Minimum iOS deployment version")

		## By default, the mod.io SDK iOS platform would only compile for the active architecture
		EXECUTE_PROCESS(COMMAND uname -m COMMAND tr -d '\n' OUTPUT_VARIABLE CMAKE_OSX_ARCHITECTURES)
		message(STATUS "Architecture: ${CMAKE_OSX_ARCHITECTURES}")

		## In case you require both architectures (ARM, x86), uncomment the line below
		# set (CMAKE_OSX_ARCHITECTURES "arm64;x86_64")

		set (CMAKE_TOOLCHAIN_FILE "${CMAKE_CURRENT_LIST_DIR}/cmake/ios.toolchain.cmake" CACHE INTERNAL "iOS toolchain file")

	elseif(MODIO_LOAD_PLATFORM_MODULES)
		add_subdirectory(${CMAKE_CURRENT_LIST_DIR})
	endif()

endif()
