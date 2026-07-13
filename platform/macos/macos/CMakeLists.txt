#
#  Copyright (C) 2021 mod.io Pty Ltd. <https://mod.io>
#
#  This file is part of the mod.io SDK.
#
#  Distributed under the MIT License. (See accompanying file LICENSE or
#   view online at <https://github.com/modio/modio-sdk/blob/main/LICENSE>)
#

if (NOT TARGET platform)
	add_library(platform INTERFACE)
endif()

target_compile_features(platform INTERFACE cxx_std_17)
target_compile_definitions(platform INTERFACE -DFU2_WITH_NO_EMPTY_PROPAGATION)
target_include_directories(platform INTERFACE ${CMAKE_CURRENT_LIST_DIR})

find_library(FOUNDATION Foundation)
find_library(CORE_FOUNDATION CoreFoundation)

target_link_libraries(platform INTERFACE httpparser pthread ghc_filesystem ${FOUNDATION} ${CORE_FOUNDATION})

# Compiled as an OBJECT library with ARC so consumers pick up the object files
# transitively without needing their own -fobjc-arc plumbing.

# This target wires its compile-time usage requirements directly to the ext
# targets instead of going through the modio chain to avoid a CMake
# INTERFACE_SOURCES self-reference cycle.
add_library(modio_apple_http OBJECT
	"${CMAKE_CURRENT_LIST_DIR}/macos/AppleHttpSession.mm"
	"${CMAKE_CURRENT_LIST_DIR}/macos/AppleHttpRequest.mm"
)

target_include_directories(modio_apple_http PRIVATE
	${CMAKE_CURRENT_LIST_DIR}
	${MODIO_ROOT_DIR}/modio
	${ModioGeneratedHeaderPath}
)

target_link_libraries(modio_apple_http PRIVATE
	asio tl::optional nlohmann_json function2 concurrentqueue
	ghc_filesystem httpparser fmt-header-only
)

target_compile_definitions(modio_apple_http PRIVATE
	FU2_WITH_NO_EMPTY_PROPAGATION
	MODIO_TARGET_PLATFORM_ID="${MODIO_TARGET_PLATFORM_ID}"
	MODIO_COMMIT_HASH="${MODIO_COMMIT_HASH}"
)

target_compile_options(modio_apple_http PRIVATE -fobjc-arc)

# PIC so the same .o files work for both static and shared link contexts.
set_target_properties(modio_apple_http PROPERTIES
	POSITION_INDEPENDENT_CODE ON
	FOLDER "modio"
)


# Attach object files to the platform INTERFACE library so every downstream
# consumer (modioStatic / modioShared / modioHeaderOnly / unittests / examples)
# picks them up transitively.
target_sources(platform INTERFACE $<TARGET_OBJECTS:modio_apple_http>)

