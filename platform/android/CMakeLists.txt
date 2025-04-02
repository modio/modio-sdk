# 
#  Copyright (C) 2021 mod.io Pty Ltd. <https://mod.io>
#  
#  This file is part of the mod.io SDK.
#  
#  Distributed under the MIT License. (See accompanying file LICENSE or 
#   view online at <https://github.com/modio/modio-sdk/blob/main/LICENSE>)
# 

if(MODIO_PLATFORM STREQUAL "ANDROID" OR MODIO_PLATFORM STREQUAL "OCULUS")

	# Look for CMAKE_ANDROID_NDK and fall back to env var is its empty
	if (DEFINED CMAKE_ANDROID_NDK)
		set (CMAKE_TOOLCHAIN_FILE ${CMAKE_ANDROID_NDK}/build/cmake/android.toolchain.cmake CACHE INTERNAL "toolchain file")
	else()
		set (CMAKE_ANDROID_NDK $ENV{MODIO_NDK_ROOT})	
		set (CMAKE_TOOLCHAIN_FILE ${CMAKE_ANDROID_NDK}/build/cmake/android.toolchain.cmake CACHE INTERNAL "toolchain file")
	endif()

	set(CMAKE_POSITION_INDEPENDENT_CODE ON)
    cmake_policy(SET CMP0077 NEW)
	set (CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/cmake;${CMAKE_MODULE_PATH}")

	if (MODIO_PLATFORM STREQUAL "ANDROID")
		set (MODIO_PLATFORM_ANDROID ON INTERNAL "building for android")
	elseif (MODIO_PLATFORM STREQUAL "OCULUS")
		set (MODIO_PLATFORM_OCULUS ON INTERNAL "building for oculus")
	endif()

	set (MODIO_MBEDTLS_ANDROID_BINARY_DIR "${CMAKE_CURRENT_BINARY_DIR}/mbedtls/library/")

	
	enable_language(C)
	enable_language(CXX)
	
	add_subdirectory(android)
	set(ENABLE_PROGRAMS OFF CACHE BOOL "")
	set(ENABLE_TESTING OFF CACHE BOOL "")

	cmake_path(SET ANDROID_NDK_PATH ${CMAKE_ANDROID_NDK})
	
	set(CMAKE_CXX_FLAGS -std=gnu++17)

	set(CMAKE_CXX_FLAGS_DEBUG "-g2 -gdwarf-2")

	set(CMAKE_SYSROOT ${CMAKE_ANDROID_NDK}/toolchains/llvm/prebuilt/windows-x86_64/sysroot)

	include(split-compilation)

	add_modio_implementation_file(${CMAKE_CURRENT_LIST_DIR}/android/ModioAndroid.ipp)
	add_modio_implementation_file(${CMAKE_CURRENT_LIST_DIR}/android/jni/AndroidContextService.ipp)
	add_modio_implementation_file(${CMAKE_CURRENT_LIST_DIR}/android/jni/JavaHelpers.ipp)
	add_modio_implementation_file(${CMAKE_CURRENT_LIST_DIR}/android/jni/JavaClassWrapper.ipp)
	add_modio_implementation_file(${CMAKE_CURRENT_LIST_DIR}/android/jni/JavaClassWrapperModio.ipp)

	execute_process(COMMAND ${CMAKE_ANDROID_NDK}/prebuilt/windows-x86_64/bin/make generated_files WORKING_DIRECTORY ${MODIO_ROOT_DIR}/ext/mbedtls)
	add_subdirectory(${MODIO_ROOT_DIR}/ext/mbedtls mbedtls EXCLUDE_FROM_ALL)
	target_compile_options(mbedcrypto PRIVATE 
		$<$<CXX_COMPILER_ID:Clang>:
			-Wno-documentation
			-Wno-unused-but-set-variable
		>
		$<$<CXX_COMPILER_ID:GNU>:
			-Wno-documentation
		>
	)

	if (MODIO_BUILD_TESTS)
		add_subdirectory(tests EXCLUDE_FROM_ALL)
	endif()

	install(
		DIRECTORY ${CMAKE_CURRENT_LIST_DIR}/android
		DESTINATION ${CMAKE_INSTALL_PREFIX}/header_only
		COMPONENT header_only
	)
	
	install(
		DIRECTORY ${MODIO_ROOT_DIR}/ext/mbedtls/include/
		DESTINATION ${CMAKE_INSTALL_PREFIX}/header_only/ext/
		COMPONENT header_only
		PATTERN "psa" EXCLUDE
		PATTERN ".gitignore" EXCLUDE
	)

	install(
		DIRECTORY ${CMAKE_CURRENT_LIST_DIR}/android
		DESTINATION ${CMAKE_INSTALL_PREFIX}/static/inc/
		COMPONENT static_library
	)
	
	install(
		DIRECTORY ${MODIO_ROOT_DIR}/ext/mbedtls/include/
		DESTINATION ${CMAKE_INSTALL_PREFIX}/static/inc/
		COMPONENT static_library
		PATTERN "psa" EXCLUDE
		PATTERN ".gitignore" EXCLUDE
	)

	install(
		DIRECTORY ${MODIO_MBEDTLS_ANDROID_BINARY_DIR}
		DESTINATION ${CMAKE_INSTALL_PREFIX}/static/lib/
		COMPONENT static_library
		PATTERN ".cmake" EXCLUDE
		PATTERN "CMakeFiles" EXCLUDE
	)
endif()
