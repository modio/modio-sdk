# 
#  Copyright (C) 2021 mod.io Pty Ltd. <https://mod.io>
#  
#  This file is part of the mod.io SDK.
#  
#  Distributed under the MIT License. (See accompanying file LICENSE or 
#   view online at <https://github.com/modio/modio-sdk/blob/main/LICENSE>)
# 

set(OPTIONAL_BUILD_PACKAGE OFF)
set(OPTIONAL_BUILD_TESTS OFF)

add_subdirectory(optional EXCLUDE_FROM_ALL)
add_subdirectory(expected EXCLUDE_FROM_ALL)
add_subdirectory(filesystem EXCLUDE_FROM_ALL)
add_subdirectory(fmt EXCLUDE_FROM_ALL)
add_subdirectory(utfcpp EXCLUDE_FROM_ALL)
add_subdirectory(function2 EXCLUDE_FROM_ALL)
add_subdirectory(concurrentqueue EXCLUDE_FROM_ALL)

include(json.cmake)
include(asio.cmake)
include(httpparser.cmake)

install(
	DIRECTORY ${CMAKE_CURRENT_LIST_DIR}/fmt/include/
	DESTINATION ${CMAKE_INSTALL_PREFIX}/header_only/ext
	COMPONENT header_only
)
install(
	DIRECTORY ${CMAKE_CURRENT_LIST_DIR}/filesystem/include/
	DESTINATION ${CMAKE_INSTALL_PREFIX}/header_only/ext
	COMPONENT header_only
)

install(
	DIRECTORY ${CMAKE_CURRENT_LIST_DIR}/function2/include/
	DESTINATION ${CMAKE_INSTALL_PREFIX}/header_only/ext
	COMPONENT header_only
)

install(
	DIRECTORY ${CMAKE_CURRENT_LIST_DIR}/optional/include/
	DESTINATION ${CMAKE_INSTALL_PREFIX}/header_only/ext
	COMPONENT header_only
)
install(
	DIRECTORY ${CMAKE_CURRENT_LIST_DIR}/expected/include/
	DESTINATION ${CMAKE_INSTALL_PREFIX}/header_only/ext
	COMPONENT header_only
)

install(
	DIRECTORY ${CMAKE_CURRENT_LIST_DIR}/utfcpp/source/
	DESTINATION ${CMAKE_INSTALL_PREFIX}/header_only/ext
	COMPONENT header_only
)

install(
	DIRECTORY ${CMAKE_CURRENT_LIST_DIR}/function2/include
	DESTINATION ${CMAKE_INSTALL_PREFIX}/header_only/ext
	COMPONENT header_only
)

install(
	DIRECTORY ${CMAKE_CURRENT_LIST_DIR}/asio/asio/include/
	DESTINATION ${CMAKE_INSTALL_PREFIX}/header_only/ext
	COMPONENT header_only
	PATTERN "*.cpp" EXCLUDE
	
)

install(
	DIRECTORY ${CMAKE_CURRENT_LIST_DIR}/httpparser/src/
	DESTINATION ${CMAKE_INSTALL_PREFIX}/header_only/ext
	COMPONENT header_only
)

install(
	DIRECTORY ${CMAKE_CURRENT_LIST_DIR}/json/single_include/
	DESTINATION ${CMAKE_INSTALL_PREFIX}/header_only/ext
	COMPONENT header_only
)

install(
	DIRECTORY ${CMAKE_CURRENT_LIST_DIR}/function2/include/function2
	DESTINATION ${CMAKE_INSTALL_PREFIX}/header_only/ext
	COMPONENT header_only
)

install(
	FILES ${CMAKE_CURRENT_LIST_DIR}/concurrentqueue/concurrentqueue.h
	DESTINATION ${CMAKE_INSTALL_PREFIX}/header_only/ext
	COMPONENT header_only
)

install(
	DIRECTORY ${CMAKE_CURRENT_LIST_DIR}/fmt/include/
	DESTINATION ${CMAKE_INSTALL_PREFIX}/static/inc
	COMPONENT static_library
	FILES_MATCHING
	PATTERN "*.h"
	PATTERN "*.hpp"
)


# Filesystem should no longer be a public dependency
#install(
#	DIRECTORY ${CMAKE_CURRENT_LIST_DIR}/filesystem/include/
#	DESTINATION ${CMAKE_INSTALL_PREFIX}/static/inc
#	COMPONENT static_library
#	FILES_MATCHING
#	PATTERN "*.h"
#	PATTERN "*.hpp"
#)

install(
	DIRECTORY ${CMAKE_CURRENT_LIST_DIR}/optional/include/
	DESTINATION ${CMAKE_INSTALL_PREFIX}/static/inc
	COMPONENT static_library
	FILES_MATCHING
	PATTERN "*.h"
	PATTERN "*.hpp"
)

install(
	DIRECTORY ${CMAKE_CURRENT_LIST_DIR}/expected/include/
	DESTINATION ${CMAKE_INSTALL_PREFIX}/static/inc
	COMPONENT static_library
	FILES_MATCHING
	PATTERN "*.h"
	PATTERN "*.hpp"
)

install(
	DIRECTORY ${CMAKE_CURRENT_LIST_DIR}/utfcpp/source/
	DESTINATION ${CMAKE_INSTALL_PREFIX}/static/inc
	COMPONENT static_library
	FILES_MATCHING
	PATTERN "*.h"
	PATTERN "*.hpp"
)

install(
	DIRECTORY ${CMAKE_CURRENT_LIST_DIR}/asio/asio/include/
	DESTINATION ${CMAKE_INSTALL_PREFIX}/static/inc
	COMPONENT static_library
	FILES_MATCHING
	PATTERN "*.h"
	PATTERN "*.hpp"
	PATTERN "*.ipp"
)

install(
	DIRECTORY ${CMAKE_CURRENT_LIST_DIR}/httpparser/src
	DESTINATION ${CMAKE_INSTALL_PREFIX}/static/inc
	COMPONENT static_library
	FILES_MATCHING
	PATTERN "*.h"
	PATTERN "*.hpp"
)

install(
	DIRECTORY ${CMAKE_CURRENT_LIST_DIR}/json/single_include/
	DESTINATION ${CMAKE_INSTALL_PREFIX}/static/inc
	COMPONENT static_library
	FILES_MATCHING
	PATTERN "*.h"
	PATTERN "*.hpp"
)


