# 
#  Copyright (C) 2021 mod.io Pty Ltd. <https://mod.io>
#  
#  This file is part of the mod.io SDK.
#  
#  Distributed under the MIT License. (See accompanying file LICENSE or 
#   view online at <https://github.com/modio/modio-sdk/blob/main/LICENSE>)
#   
# 

function(locate_vs VersionYear InstallationPath)
	message(STATUS "Locating Visual Studio ${VersionYear}[r[n")
	if (VersionYear STREQUAL "2017")
	set(Parameters -version [[15,16[))
	elseif(VersionYear STREQUAL "2019")
	set(Parameters -version [16.0,17.0[))
	else()
	set(Parameters -latest)
	endif()
	#set(CMAKE_FIND_DEBUG_MODE TRUE)
	set(PROGx86 "PROGRAMFILES(X86)")
	find_program(VsWherePath NAME vswhere.exe HINTS $ENV{${PROGx86}} PATH_SUFFIXES "Microsoft Visual Studio/Installer" REQUIRED)
	message(STATUS "Invoking vswhere at ${VsWherePath}")
	
	execute_process(COMMAND "${VsWherePath}" ${Parameters} -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 -property installationPath OUTPUT_STRIP_TRAILING_WHITESPACE OUTPUT_VARIABLE LOCATED_VS_INSTALL_DIR)

	if (LOCATED_VS_INSTALL_DIR STREQUAL "")
		message(STATUS "VS component x86.x64 was not located, trying any VS installation")
		execute_process(COMMAND "${VsWherePath}" ${Parameters} -property installationPath OUTPUT_STRIP_TRAILING_WHITESPACE OUTPUT_VARIABLE LOCATED_VS_INSTALL_DIR)
		message(STATUS "VS located at ${LOCATED_VS_INSTALL_DIR}")
	else()
		message(STATUS "VS component x86.x64 located at ${LOCATED_VS_INSTALL_DIR}")
	endif()
	set(${InstallationPath} "${LOCATED_VS_INSTALL_DIR}" PARENT_SCOPE)
endfunction()
 
function (locate_asan_dll VersionYear AsanLocation)
	message(STATUS "Locating Visual Studio ${VersionYear}[r[n")
	if (VersionYear STREQUAL "2017")
	set(Parameters -version [[15,16[))
	elseif(VersionYear STREQUAL "2019")
	set(Parameters -version [16.0,17.0[))
	else()
	set(Parameters -latest)
	endif()
	#set(CMAKE_FIND_DEBUG_MODE TRUE)
	set(PROGx86 "PROGRAMFILES(X86)")
	find_program(VsWherePath NAME vswhere.exe HINTS $ENV{${PROGx86}} PATH_SUFFIXES "Microsoft Visual Studio/Installer" REQUIRED)
	message(STATUS "Invoking vswhere at ${VsWherePath}")
	
	execute_process(COMMAND "${VsWherePath}" ${Parameters} -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 -property installationPath OUTPUT_STRIP_TRAILING_WHITESPACE OUTPUT_VARIABLE LOCATED_VS_INSTALL_DIR)

	if (LOCATED_VS_INSTALL_DIR STREQUAL "")
		message(STATUS "VS component x86.x64 was not located, trying any VS installation")
		execute_process(COMMAND "${VsWherePath}" ${Parameters} -property installationPath OUTPUT_STRIP_TRAILING_WHITESPACE OUTPUT_VARIABLE LOCATED_VS_INSTALL_DIR)
		message(STATUS "VS located at ${LOCATED_VS_INSTALL_DIR}")
	else()
		message(STATUS "VS component x86.x64 located at ${LOCATED_VS_INSTALL_DIR}")
	endif()
	# set(${InstallationPath} "${LOCATED_VS_INSTALL_DIR}" PARENT_SCOPE)

	message("Checking for asan binary")

	execute_process(COMMAND "${VsWherePath}" ${Parameters} -sort -requires Microsoft.VisualStudio.Component.VC.ASAN -find **/Hostx64/x64/clang_rt.asan_dynamic-x86_64.dll OUTPUT_STRIP_TRAILING_WHITESPACE OUTPUT_VARIABLE LOCATED_VS_ASAN_DLL_PATH)

	string(REPLACE "[n" "[
" LOCATED_VS_ASAN_DLL_PATH_LIST ${LOCATED_VS_ASAN_DLL_PATH})

	list(GET LOCATED_VS_ASAN_DLL_PATH_LIST -1 LOCATED_VS_ASAN_DLL_LOCAL)

	set(LOCATED_VS_ASAN_DLL "${LOCATED_VS_ASAN_DLL_LOCAL}")

	message("ASAN located at ${LOCATED_VS_ASAN_DLL}")

	set(${AsanLocation} "${LOCATED_VS_ASAN_DLL}" PARENT_SCOPE)
endfunction()

function (locate_sanitizer_lib_dir VersionYear LibDirLocation)
	message(STATUS "Locating Visual Studio ${VersionYear}[r[n")
	if (VersionYear STREQUAL "2017")
	set(Parameters -version [[15,16[))
	elseif(VersionYear STREQUAL "2019")
	set(Parameters -version [16.0,17.0[))
	else()
	set(Parameters -latest)
	endif()
	#set(CMAKE_FIND_DEBUG_MODE TRUE)
	set(PROGx86 "PROGRAMFILES(X86)")
	find_program(VsWherePath NAME vswhere.exe HINTS $ENV{${PROGx86}} PATH_SUFFIXES "Microsoft Visual Studio/Installer")

	if (NOT CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
  		message(FATAL_ERROR "Asan/Ubsan requires clang as the main compiler, this configuration can not continue")
	endif()

	if(NOT VsWherePath)
		if (MODIO_ENABLE_ASAN)
			## https://llvm.org/docs/CMake.html
			set(LLVM_USE_SANITIZER "Address[
Thread")
		elseif (MODIO_ENABLE_UBSAN)
			set(LLVM_USE_SANITIZER "Undefined")
			set(LLVM_UBSAN_FLAGS " -fsanitize=undefined -fsanitize-recover=undefined")
		endif()
	else()
		message(STATUS "Invoking vswhere at ${VsWherePath}")
		
		execute_process(COMMAND "${VsWherePath}" ${Parameters} -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 -property installationPath OUTPUT_STRIP_TRAILING_WHITESPACE OUTPUT_VARIABLE LOCATED_VS_INSTALL_DIR)

		if (LOCATED_VS_INSTALL_DIR STREQUAL "")
			message(STATUS "VS component x86.x64 was not located, trying any VS installation")
			execute_process(COMMAND "${VsWherePath}" ${Parameters} -property installationPath OUTPUT_STRIP_TRAILING_WHITESPACE OUTPUT_VARIABLE LOCATED_VS_INSTALL_DIR)
			message(STATUS "VS located at ${LOCATED_VS_INSTALL_DIR}")
		else()
			message(STATUS "VS component x86.x64 located at ${LOCATED_VS_INSTALL_DIR}")
		endif()
		# set(${InstallationPath} "${LOCATED_VS_INSTALL_DIR}" PARENT_SCOPE)

		message("Checking for asan binary")

		if (MODIO_ENABLE_UBSAN)
			execute_process(COMMAND "${VsWherePath}" ${Parameters} -sort -requires Microsoft.VisualStudio.Component.VC.ASAN -find **/Tools/MSVC/**/clang_rt.ubsan_standalone-x86_64.lib OUTPUT_STRIP_TRAILING_WHITESPACE OUTPUT_VARIABLE LOCATED_VS_ASAN_DLL_PATH)
		else()
			execute_process(COMMAND "${VsWherePath}" ${Parameters} -sort -requires Microsoft.VisualStudio.Component.VC.ASAN -find **/Tools/MSVC/**/clang_rt.asan_dynamic-x86_64.lib OUTPUT_STRIP_TRAILING_WHITESPACE OUTPUT_VARIABLE LOCATED_VS_ASAN_DLL_PATH)
		endif()

		string(REPLACE "[n" "[
" LOCATED_VS_ASAN_DLL_PATH_LIST ${LOCATED_VS_ASAN_DLL_PATH})

		list(GET LOCATED_VS_ASAN_DLL_PATH_LIST -1 LOCATED_VS_ASAN_DLL_LOCAL)
		set(LOCATED_VS_ASAN_DLL "${LOCATED_VS_ASAN_DLL_LOCAL}")

		get_filename_component(ITEM_PATH ${LOCATED_VS_ASAN_DLL} DIRECTORY)
		message("ASAN located at ${ITEM_PATH}")

		set(${LibDirLocation} "${ITEM_PATH}" PARENT_SCOPE)
	endif()
endfunction()
