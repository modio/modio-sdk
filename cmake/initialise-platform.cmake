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


set (MODIO_PLATFORM "WIN" CACHE STRING "Which platform to build the SDK for")
   
message("Fetching platform dependencies...")

#iterate through all the currently installed platform directories
list_subdirs(modio_platform_dirs ${MODIO_ROOT_DIR}/platform )

#add_subdirectory will only populate targets for the currently valid MODIO_PLATFORM
foreach (platform_dir ${modio_platform_dirs})
    set(full_platform_dir ${MODIO_ROOT_DIR}/platform/${platform_dir})
    if (EXISTS ${full_platform_dir}/platform.cmake)
        include(${full_platform_dir}/platform.cmake)
    endif()
endforeach()


add_generated_defines_to_platform()
