set(OPTIONAL_BUILD_PACKAGE OFF)
set(OPTIONAL_BUILD_TESTS OFF)

add_subdirectory(optional EXCLUDE_FROM_ALL)
add_subdirectory(expected EXCLUDE_FROM_ALL)
add_subdirectory(filesystem EXCLUDE_FROM_ALL)
add_subdirectory(fmt EXCLUDE_FROM_ALL)
add_subdirectory(utfcpp EXCLUDE_FROM_ALL)

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
	DIRECTORY ${CMAKE_CURRENT_LIST_DIR}/asio/asio/include/
	DESTINATION ${CMAKE_INSTALL_PREFIX}/header_only/ext
	COMPONENT header_only
	PATTERN "*.cpp" EXCLUDE
	
)

install(
	DIRECTORY ${CMAKE_CURRENT_LIST_DIR}/httpparser/src
	DESTINATION ${CMAKE_INSTALL_PREFIX}/header_only/ext
	COMPONENT header_only
)

install(
	DIRECTORY ${CMAKE_CURRENT_LIST_DIR}/json/single_include/
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
install(
	DIRECTORY ${CMAKE_CURRENT_LIST_DIR}/filesystem/include/
	DESTINATION ${CMAKE_INSTALL_PREFIX}/static/inc
	COMPONENT static_library
	FILES_MATCHING
	PATTERN "*.h"
	PATTERN "*.hpp"
)

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



