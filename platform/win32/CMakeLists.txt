if(MODIO_PLATFORM STREQUAL "WIN")
	set (CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/cmake;${CMAKE_MODULE_PATH}")
	set (MODIO_PLATFORM_WIN ON INTERNAL "building windows")

	enable_language(C)
	enable_language(CXX)

	add_subdirectory(win32)

	
	
	add_subdirectory(../ms-common ../ms-common)

	if (MODIO_BUILD_TESTS)
		add_subdirectory(tests EXCLUDE_FROM_ALL)
	endif()

	install(
		DIRECTORY ${CMAKE_CURRENT_LIST_DIR}/win32
		DESTINATION ${CMAKE_INSTALL_PREFIX}/header_only
		COMPONENT header_only
	)
endif()
