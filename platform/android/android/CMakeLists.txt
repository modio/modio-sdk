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
	$<$<CXX_COMPILER_ID:GNU>:
		-Werror # Enable warnings as errors
	>
)
target_include_directories(platform INTERFACE ${CMAKE_CURRENT_LIST_DIR})

target_link_libraries(platform INTERFACE mbedtls mbedcrypto mbedx509)
target_link_libraries(platform INTERFACE httpparser ghc_filesystem)
target_link_libraries(platform INTERFACE log)
