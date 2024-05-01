#
#  Copyright (C) 2021 mod.io Pty Ltd. <https://mod.io>
#
#  This file is part of the mod.io SDK.
#
#  Distributed under the MIT License. (See accompanying file LICENSE or
#   view online at <https://github.com/modio/modio-sdk/blob/main/LICENSE>)
#

if(MODIO_PLATFORM STREQUAL "IOS")
    set (CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/cmake;${CMAKE_MODULE_PATH}")
	set (MODIO_PLATFORM_IOS ON INTERNAL "building for iOS")

	## `IPHONEOS_DEPLOYMENT_TARGET` refers to the oldest version to allow deployment of the mod.io SDK
	set (CMAKE_XCODE_ATTRIBUTE_IPHONEOS_DEPLOYMENT_TARGET "15.0" CACHE STRING "Minimum iOS deployment version")

	## By default, the mod.io SDK iOS platform would only compile for the active architecture
	EXECUTE_PROCESS(COMMAND uname -m COMMAND tr -d '\n' OUTPUT_VARIABLE CMAKE_OSX_ARCHITECTURES)
	message(STATUS "Architecture: ${CMAKE_OSX_ARCHITECTURES}")

	## In case you require both architectures (ARM, x86), uncomment the line below
	# set (CMAKE_OSX_ARCHITECTURES "arm64;x86_64")

	set (CMAKE_TOOLCHAIN_FILE "${CMAKE_CURRENT_LIST_DIR}/cmake/ios.toolchain.cmake" CACHE INTERNAL "iOS toolchain file")
	
	enable_language(C)
	enable_language(CXX)

	add_subdirectory(ios)
	
    if (MODIO_BUILD_TESTS)
		add_subdirectory(tests EXCLUDE_FROM_ALL)
	endif()

	install(
		DIRECTORY ${CMAKE_CURRENT_LIST_DIR}/ios
		DESTINATION ${CMAKE_INSTALL_PREFIX}/header_only
		COMPONENT header_only
	)

	if (MODIO_BUILD_EXAMPLES)
		add_subdirectory(examples/modioSSO EXCLUDE_FROM_ALL)
	endif()
endif()
