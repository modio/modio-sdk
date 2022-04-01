# 
#  Copyright (C) 2021 mod.io Pty Ltd. <https://mod.io>
#  
#  This file is part of the mod.io SDK.
#  
#  Distributed under the MIT License. (See accompanying file LICENSE or 
#   view online at <https://github.com/modio/modio-sdk/blob/main/LICENSE>)
# 


include(split-compilation)

if (NOT TARGET ${MODIO_TARGET_NAME})
add_library (${MODIO_TARGET_NAME} INTERFACE)
endif()

if (MODIO_SERVER_SIDE_TEST)
        target_compile_definitions(${MODIO_TARGET_NAME} INTERFACE MODIO_SERVER_SIDE_TEST="${MODIO_SERVER_SIDE_TEST}")
        message(DEBUG "Setting MODIO_SERVER_SIDE_TEST")
endif()

target_include_directories(${MODIO_TARGET_NAME} INTERFACE ${CMAKE_CURRENT_LIST_DIR})

add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/modio/cache)
add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/modio/compression)
add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/modio/core)
add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/modio/detail)
add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/modio/http)
add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/modio/impl)

#Populate INTERFACE_SOURCES so CMake view in VS shows the headers in projects using the SDK
target_sources(${MODIO_TARGET_NAME} INTERFACE 
		modio/ModioSDK.h
	)

add_public_header(${MODIO_TARGET_NAME} modio/ModioSDK.h)


target_include_directories(${MODIO_TARGET_NAME} INTERFACE ${ModioGeneratedHeaderPath})

target_link_libraries(${MODIO_TARGET_NAME} INTERFACE asio ghc_filesystem fmt-header-only tl::optional platform nlohmann_json)



add_library(${MODIO_TARGET_NAME}Static STATIC)

set_target_properties(${MODIO_TARGET_NAME}Static PROPERTIES LINKER_LANGUAGE CXX)

add_modio_implementation_to_target(${MODIO_TARGET_NAME}Static)
target_link_libraries(${MODIO_TARGET_NAME}Static PUBLIC ${MODIO_TARGET_NAME})
target_compile_definitions(${MODIO_TARGET_NAME}Static PUBLIC -DMODIO_SEPARATE_COMPILATION=1)
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
	DESTINATION ${CMAKE_INSTALL_PREFIX}/static/inc
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

