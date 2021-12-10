# 
#  Copyright (C) 2021 mod.io Pty Ltd. <https://mod.io>
#  
#  This file is part of the mod.io SDK.
#  
#  Distributed under the MIT License. (See accompanying file LICENSE or 
#   view online at <https://github.com/modio/modio-sdk/blob/main/LICENSE>)
# 

if(MODIO_PLATFORM STREQUAL "WIN" OR MODIO_FORCE_MS_COMMON)
	if (NOT TARGET platform)
		add_library(platform INTERFACE)
	endif()

	include(split-compilation)

	if (MSVC)
		target_compile_definitions(platform INTERFACE _SILENCE_CXX17_CODECVT_HEADER_DEPRECATION_WARNING )
		target_compile_options(platform INTERFACE /bigobj)
		target_compile_options(platform INTERFACE /permissive-)
	elseif(GCC)
		target_compile_options(platform INTERFACE -Wa,-mbig-obj -Og)
	endif()

	target_include_directories(platform INTERFACE ${CMAKE_CURRENT_LIST_DIR}/include)

	add_modio_implementation_file(${CMAKE_CURRENT_LIST_DIR}/include/common/HttpSharedState.ipp)

	if (MODIO_BUILD_TESTS)
		set(MODIO_TEST_RENAME_ISATTY ON CACHE STRING "")
		add_subdirectory(tests EXCLUDE_FROM_ALL)
	endif()

	install(
		DIRECTORY ${CMAKE_CURRENT_LIST_DIR}/include/
		DESTINATION ${CMAKE_INSTALL_PREFIX}/header_only/ms-common
		COMPONENT header_only
	)

endif()
