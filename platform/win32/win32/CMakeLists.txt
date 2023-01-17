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

target_compile_features(platform INTERFACE cxx_std_14)

target_include_directories(platform INTERFACE ${CMAKE_CURRENT_LIST_DIR})
target_link_libraries(platform INTERFACE utf8cpp ws2_32 winhttp)
target_compile_definitions(platform INTERFACE WIN32_LEAN_AND_MEAN NOMINMAX D_UNICODE UNICODE)
target_include_directories(platform INTERFACE ${ModioGeneratedHeaderPath})
