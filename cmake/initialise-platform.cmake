#
#  Copyright (C) 2021 mod.io Pty Ltd. <https://mod.io>
#
#  This file is part of the mod.io SDK.
#
#  Distributed under the MIT License. (See accompanying file LICENSE or
#   view online at <https://github.com/modio/modio-sdk/blob/main/LICENSE>)
#
#

include_guard(GLOBAL)
include(platform-defines)

macro(list_subdirs result curdir)
  FILE(GLOB children RELATIVE ${curdir} ${curdir}/*)
  SET(dirlist "")
  FOREACH(child ${children})
    IF(IS_DIRECTORY ${curdir}/${child})
        SET(dirlist ${dirlist} ${child})
    ENDIF()
  ENDFOREACH()
  SET(${result} ${dirlist})
endmacro()

function (modio_set_toolchain)
    set(MODIO_SET_TOOLCHAIN TRUE)

    message("Fetching platform dependencies...")

    #iterate through all the currently installed platform directories
    list_subdirs(modio_platform_dirs ${MODIO_ROOT_DIR}/platform )

    set(MODIO_PLATFORM_CMAKE_FILES "")
    set(MODIO_PLATFORM_CI_DIRS "")

    #add_subdirectory will only populate targets for the currently valid MODIO_PLATFORM
    foreach (platform_dir ${modio_platform_dirs})
        set(full_platform_dir ${MODIO_ROOT_DIR}/platform/${platform_dir})
        if (EXISTS ${full_platform_dir}/platform.cmake)
            include(${full_platform_dir}/platform.cmake)
            list(APPEND MODIO_PLATFORM_CMAKE_FILES "${full_platform_dir}/platform.cmake")
        endif()
    endforeach()

    set(MODIO_PLATFORM_CMAKE_FILES "${MODIO_PLATFORM_CMAKE_FILES}" PARENT_SCOPE)
    set(MODIO_PLATFORM_CI_DIRS "${MODIO_PLATFORM_CI_DIRS}" PARENT_SCOPE)

    set(MODIO_SET_TOOLCHAIN FALSE)
endfunction()

function (modio_platform_init)
    set(MODIO_LOAD_PLATFORM_MODULES TRUE)

    message("Loading platform modules...")

    foreach(file ${MODIO_PLATFORM_CMAKE_FILES})
        include(${file})
    endforeach()

    foreach(dir ${MODIO_PLATFORM_CI_DIRS})
        add_subdirectory(${dir})
    endforeach()


    add_generated_defines_to_platform()

    set(MODIO_LOAD_PLATFORM_MODULES FALSE)
endfunction()
