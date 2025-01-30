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
target_compile_options(platform INTERFACE 
	$<$<CXX_COMPILER_ID:MSVC>:
		/WX # Enable warnings as errors
	>
	$<$<CXX_COMPILER_ID:Clang>:
		-Werror # Enable warnings as errors
	>
)

target_include_directories(platform INTERFACE ${CMAKE_CURRENT_LIST_DIR})

target_link_libraries(platform INTERFACE utf8cpp ws2_32 winhttp ghc_filesystem rpcrt4)

# In case you have a custom FMT library, update accordingly below
# The default linking of the library uses the FMT Header only mode
if (MODIO_USE_CUSTOM_FMT)
	if (NOT DEFINED MODIO_CUSTOM_FMT_PATH)
		message(FATAL_ERROR "The build system needs MODIO_CUSTOM_FMT_PATH defined pointing to the custom FMT library to use.")
	else ()
		message(STATUS "Using FMT path ${MODIO_CUSTOM_FMT_PATH}")
		add_subdirectory(${MODIO_CUSTOM_FMT_PATH} ext/custom_fmt)

		if (MODIO_CUSTOM_FMT_STATIC)
			target_link_libraries(platform INTERFACE fmt)
		else()
			target_link_libraries(platform INTERFACE fmt-header-only)
		endif()
	endif()
else ()
	target_link_libraries(platform INTERFACE fmt-header-only)
endif()

target_compile_definitions(platform INTERFACE WIN32_LEAN_AND_MEAN NOMINMAX D_UNICODE UNICODE)
target_include_directories(platform INTERFACE ${ModioGeneratedHeaderPath})
