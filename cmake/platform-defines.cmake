# 
#  Copyright (C) 2021 mod.io Pty Ltd. <https://mod.io>
#  
#  This file is part of the mod.io SDK.
#  
#  Distributed under the MIT License. (See accompanying file LICENSE or 
#   view online at <https://github.com/modio/modio-sdk/blob/main/LICENSE>)
#   
# 

include(GetGitRevisionDescription)
include(modio-version)

function (add_generated_defines_to_platform)
	message("Initializing platform")
	
	set(MODIO_COMMIT_HASH "NOT_SET")
	#get_git_head_revision(MODIO_REFSPEC MODIO_COMMIT_HASH)

	#if (NOT MODIO_REFSPEC)
	#set(MODIO_REFSPEC UNKNOWN)
	#endif()
	#if (NOT MODIO_COMMIT_HASH)
	#set(MODIO_COMMIT_HASH UNKNOWN)
	#endif()
	
	GetVersionInfo("main" MAIN_COMMIT_COUNT CURRENT_BRANCH_PREFIX CURRENT_BRANCH_COMMIT_COUNT FALSE)

	set(MODIO_COMMIT_HASH "${MAIN_COMMIT_COUNT}${CURRENT_BRANCH_PREFIX}${CURRENT_BRANCH_COMMIT_COUNT}")
	
	message(STATUS "Setting revision to ${MODIO_COMMIT_HASH}")

	message(STATUS "Generating global definitions in ${CMAKE_BINARY_DIR}/generated/ModioGeneratedVariables.h")

	set(MODIO_TARGET_PLATFORM_ID ${MODIO_PLATFORM})

	configure_file(cmake/ModioGeneratedVariables.h.in ${CMAKE_BINARY_DIR}/generated/ModioGeneratedVariables.h)
	target_include_directories(platform INTERFACE ${CMAKE_BINARY_DIR}/generated)
	target_sources(platform INTERFACE ${CMAKE_BINARY_DIR}/generated/ModioGeneratedVariables.h)

endfunction()
