# 
#  Copyright (C) 2021 mod.io Pty Ltd. <https://mod.io>
#  
#  This file is part of the mod.io SDK.
#  
#  Distributed under the MIT License. (See accompanying file LICENSE or 
#   view online at <https://github.com/modio/modio-sdk/blob/main/LICENSE>)
# 

cmake_minimum_required(VERSION 3.20)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/cmake")

if (NOT MODIO_PLATFORM)
    ## The MODIO_PLATFORM variable is required to compile the mod.io SDK
    message(FATAL_ERROR "Variable 'MODIO_PLATFORM' must be defined to compile the SDK. For example: '-D MODIO_PLATFORM=WIN'")
endif()

if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release)
endif()

# This needs to be setup before we start including any source, as the install() calls are using the CMAKE_INSTALL_PREFIX variable
set(CMAKE_INSTALL_PREFIX "${CMAKE_CURRENT_LIST_DIR}/out/install/${MODIO_PLATFORM}-${CMAKE_BUILD_TYPE}" CACHE STRING "Installation directory")
set(CPACK_INSTALL_PREFIX "${CMAKE_INSTALL_PREFIX}")

#Enable policy forcing CMake to explicitly tell non-MSVC compilers IPP files are CXX files
cmake_policy(SET CMP0119 NEW)
set(CMAKE_CXX_SOURCE_FILE_EXTENSIONS ${CMAKE_CXX_SOURCE_FILE_EXTENSIONS};ipp)

set( CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(ModioGeneratedHeaderPath "${CMAKE_BINARY_DIR}/generated" CACHE INTERNAL "")

set (MODIO_ROOT_DIR ${CMAKE_CURRENT_LIST_DIR})
set (MODIO_VERSION "v2.0.0" CACHE STRING "Version number for the packaged version")

#Global definition of the library target name so it can be referenced in platform tests, etc, and to ease renaming if it becomes necessary
set (MODIO_TARGET_NAME modio CACHE STRING "")


#Injects toolchain file if necessary for the specified platform, downloads NDA dependencies
set (CMAKE_PROJECT_${MODIO_TARGET_NAME}_INCLUDE_BEFORE ${CMAKE_CURRENT_LIST_DIR}/cmake/initialise-platform.cmake)

project(${MODIO_TARGET_NAME} C CXX )


set_property(GLOBAL PROPERTY USE_FOLDERS ON)

add_subdirectory(ext)

add_subdirectory(modio)


#TODO : use cmake install() command to specify files to install
#then we want to invoke package() somewhere, I think? set(CPACK_INCLUDE_TOPLEVEL_DIRECTORY OFF)

set(CPACK_PACKAGE_NAME "modio-SDK")
set(CPACK_PACKAGE_VENDOR "mod.io pty")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "mod.io SDKv2")

# Capture a version number, result is just discarded
string(REGEX MATCHALL 
	"^v([0-9]+)\.([0-9]+)\.([0-9]+).*$" result "${MODIO_VERSION}")

if(NOT "${CMAKE_MATCH_COUNT}" EQUAL "3")
	message(FATAL_ERROR "MODIO_VERSION has a invalid format \"${MODIO_VERSION}\"")
endif()

set(CPACK_PACKAGE_VERSION_MAJOR "${CMAKE_MATCH_1}")
set(CPACK_PACKAGE_VERSION_MINOR "${CMAKE_MATCH_2}")
set(CPACK_PACKAGE_VERSION_PATCH "${CMAKE_MATCH_3}")
set(CPACK_PACKAGE_VERSION "${CPACK_PACKAGE_VERSION_MAJOR}.${CPACK_PACKAGE_VERSION_MINOR}.${CPACK_PACKAGE_VERSION_PATCH}")
set(CPACK_GENERATOR "ZIP")
set(CPACK_INSTALL_PREFIX "${CMAKE_CURRENT_LIST_DIR}/out/install/${MODIO_PLATFORM}-${CMAKE_BUILD_TYPE}")

set(CPACK_BINARY_NSIS off)
set(CPACK_VERBATIM_VARIABLES YES)
include(CPack)


include(modio-packaging)

generate_implementation_source_target()

