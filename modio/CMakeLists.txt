# 
#  Copyright (C) 2021 mod.io Pty Ltd. <https://mod.io>
#  
#  This file is part of the mod.io SDK.
#  
#  Distributed under the MIT License. (See accompanying file LICENSE or 
#   view online at <https://github.com/modio/modio-sdk/blob/main/LICENSE>)
# 

cmake_minimum_required(VERSION 3.28)


include(split-compilation)

if (NOT TARGET ${MODIO_TARGET_NAME})
add_library (${MODIO_TARGET_NAME} INTERFACE)
endif()

if (MODIO_SERVER_SIDE_TEST)
    target_compile_definitions(${MODIO_TARGET_NAME} INTERFACE MODIO_SERVER_SIDE_TEST="${MODIO_SERVER_SIDE_TEST}")
    message(STATUS "Setting MODIO_SERVER_SIDE_TEST")
endif()

if (MODIO_TEST_REQUIRES_STATE)
        target_compile_definitions(${MODIO_TARGET_NAME} INTERFACE MODIO_TEST_REQUIRES_STATE=1)
        message(STATUS "Setting MODIO_TEST_REQUIRES_STATE")
endif()

if (MODIO_CI)
target_compile_definitions(${MODIO_TARGET_NAME} INTERFACE MODIO_CI=1)
        message(STATUS "Setting MODIO_CI")
endif()

target_include_directories(${MODIO_TARGET_NAME} INTERFACE ${CMAKE_CURRENT_LIST_DIR})

add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/modio/cache)
add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/modio/compression)
add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/modio/core)
add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/modio/detail)
add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/modio/http)
add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/modio/impl)
add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/modio/timer)

#Populate INTERFACE_SOURCES so CMake view in VS shows the headers in projects using the SDK
target_sources(${MODIO_TARGET_NAME} INTERFACE 
		modio/ModioSDK.h
	)

add_public_header(${MODIO_TARGET_NAME} modio/ModioSDK.h)


target_include_directories(${MODIO_TARGET_NAME} INTERFACE ${ModioGeneratedHeaderPath})

target_link_libraries(${MODIO_TARGET_NAME} INTERFACE asio tl::optional platform nlohmann_json function2 concurrentqueue)

# In case you have a custom FMT library, update accordingly below
# The default linking of the library uses the FMT Header only mode
if (MODIO_USE_CUSTOM_FMT)
	if (NOT DEFINED MODIO_CUSTOM_FMT_PATH)
		message(FATAL_ERROR "The build system needs MODIO_CUSTOM_FMT_PATH defined pointing to the custom FMT library to use.")
	else ()
		message(STATUS "Using FMT path ${MODIO_CUSTOM_FMT_PATH}")
		add_subdirectory(${MODIO_CUSTOM_FMT_PATH} ext/custom_fmt)

		if (MODIO_CUSTOM_FMT_STATIC)
			target_link_libraries(${MODIO_TARGET_NAME} INTERFACE fmt)
		else()
			target_link_libraries(${MODIO_TARGET_NAME} INTERFACE fmt-header-only)
		endif()
	endif()
else ()
	target_link_libraries(${MODIO_TARGET_NAME} INTERFACE fmt-header-only)
endif()


add_library(${MODIO_TARGET_NAME}Static STATIC)

set_target_properties(${MODIO_TARGET_NAME}Static PROPERTIES LINKER_LANGUAGE CXX)

add_modio_implementation_to_target(${MODIO_TARGET_NAME}Static)
target_link_libraries(${MODIO_TARGET_NAME}Static PUBLIC ${MODIO_TARGET_NAME})
target_compile_definitions(${MODIO_TARGET_NAME}Static PUBLIC -DMODIO_SEPARATE_COMPILATION=1)
#Suppress internal deprecations
target_compile_definitions(${MODIO_TARGET_NAME}Static PRIVATE -DMODIO_DISABLE_ALL_DEPRECATIONS)


#turn on more warnings for the static library target
#disabled-modio-public-cmake target_compile_options(${MODIO_TARGET_NAME}Static PRIVATE
#disabled-modio-public-cmake      $<$<OR:$<CXX_COMPILER_ID:Clang>,$<CXX_COMPILER_ID:AppleClang>,$<CXX_COMPILER_ID:GNU>>:
#disabled-modio-public-cmake           -Wall>
#disabled-modio-public-cmake      $<$<CXX_COMPILER_ID:MSVC>:
#disabled-modio-public-cmake           /W3>)

set_target_properties(${MODIO_TARGET_NAME}Static PROPERTIES DEBUG_POSTFIX _d)

if (MODIO_STATIC_LIB_SUFFIX)
set_target_properties(${MODIO_TARGET_NAME}Static PROPERTIES SUFFIX ${MODIO_STATIC_LIB_SUFFIX})
endif()

# Set up a header-only target that we can build for verifying header-only functionality in a lightweight way
file(WRITE "${CMAKE_BINARY_DIR}/header-only/main.cpp" 
"// Automatically generated dummy file for validating header-only compilation.
#include \"modio/ModioSDK.h\"
int main() 
{
	Modio::SetLogLevel(Modio::LogLevel::Info);	
	return 0;
}
")
file(WRITE "${CMAKE_BINARY_DIR}/header-only/CMakeLists.txt" 
"Project(modioHeaderOnly)\n
set( CMAKE_EXPORT_COMPILE_COMMANDS ON)\n 
add_executable(modioHeaderOnly main.cpp)\n 
target_link_libraries(modioHeaderOnly PUBLIC ${MODIO_TARGET_NAME})\n
")

	FetchContent_Declare(
    headerOnly
    SOURCE_DIR "${CMAKE_BINARY_DIR}/header-only/"
    CMAKE_GENERATOR Ninja
	EXCLUDE_FROM_ALL 
)

FetchContent_MakeAvailable(headerOnly)

install(
	DIRECTORY ${CMAKE_CURRENT_LIST_DIR}/modio/core
	DESTINATION ${CMAKE_INSTALL_PREFIX}/static/inc/modio
	COMPONENT static_library
	FILES_MATCHING
	PATTERN "*.h"
	PATTERN "*.hpp"
)

install(
	DIRECTORY ${CMAKE_CURRENT_LIST_DIR}/modio/detail
	DESTINATION ${CMAKE_INSTALL_PREFIX}/static/inc/modio
	COMPONENT static_library
	FILES_MATCHING
	PATTERN "*.h"
	PATTERN "*.hpp"
	PATTERN "ops/*" EXCLUDE
)

install(
	DIRECTORY ${ModioGeneratedHeaderPath}/
	DESTINATION ${CMAKE_INSTALL_PREFIX}/static/inc
	COMPONENT static_library
	PATTERN ModioTestConfig.h EXCLUDE	
)

install(
	FILES ${CMAKE_CURRENT_LIST_DIR}/modio/ModioSDK.h
	DESTINATION ${CMAKE_INSTALL_PREFIX}/static/inc/modio
	COMPONENT static_library
)

#todo: ensure different platforms are named correctly or at least placed into split folders
install(
	TARGETS ${MODIO_TARGET_NAME}Static ARCHIVE
	DESTINATION ${CMAKE_INSTALL_PREFIX}/static/lib
	COMPONENT static_library
)

install(
	DIRECTORY ${CMAKE_CURRENT_LIST_DIR}/
	DESTINATION ${CMAKE_INSTALL_PREFIX}/header_only/common
	COMPONENT header_only
	
)

install(
	DIRECTORY ${ModioGeneratedHeaderPath}/
	DESTINATION ${CMAKE_INSTALL_PREFIX}/header_only/common
	COMPONENT header_only
	PATTERN ModioTestConfig.h EXCLUDE
)

