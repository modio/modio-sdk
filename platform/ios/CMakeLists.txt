#
#  Copyright (C) 2021 mod.io Pty Ltd. <https://mod.io>
#
#  This file is part of the mod.io SDK.
#
#  Distributed under the MIT License. (See accompanying file LICENSE or
#   view online at <https://github.com/modio/modio-sdk/blob/main/LICENSE>)
#

if(MODIO_PLATFORM STREQUAL "IOS")
	
	enable_language(C)
	enable_language(CXX)

	add_subdirectory(ios)
	
    if (MODIO_BUILD_TESTS)
		add_subdirectory(tests EXCLUDE_FROM_ALL)
	endif()

	install(
		DIRECTORY ${CMAKE_CURRENT_LIST_DIR}/ios
		DESTINATION ${CMAKE_INSTALL_PREFIX}/header_only
		COMPONENT header_only
	)

	if (MODIO_BUILD_EXAMPLES)
		add_subdirectory(examples/modioSSO EXCLUDE_FROM_ALL)

		if(${CMAKE_GENERATOR} MATCHES "^Xcode.*")
			## Adding the Swift language as part of the project using Ninja, complains 
			## about not correctly compiling the sample code. 
			add_subdirectory(examples/modioSwift EXCLUDE_FROM_ALL)
		endif()
		
	endif()
endif()
