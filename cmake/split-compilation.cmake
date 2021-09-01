# 
#  Copyright (C) 2021 mod.io Pty Ltd. <https://mod.io>
#  
#  This file is part of the mod.io SDK.
#  
#  Distributed under the MIT License. (See accompanying file LICENSE or 
#   view online at <https://github.com/modio/modio-sdk/blob/main/LICENSE>)
#   
# 

macro(add_modio_implementation_file FilePath)
set_property(GLOBAL APPEND PROPERTY modio_implementation_files ${FilePath})
endmacro()

macro(add_modio_implementation_to_target Target)
get_property(impl_files GLOBAL PROPERTY modio_implementation_files)
target_sources(${Target} PRIVATE ${impl_files})
set_source_files_properties(${impl_files} TARGET_DIRECTORY ${Target} PROPERTIES LANGUAGE CXX)
endmacro()

macro(add_public_header Target FilePath)
set_property(TARGET ${Target} APPEND PROPERTY PUBLIC_HEADER ${FilePath})
endmacro()

macro(generate_implementation_source_target)
get_property(impl_files GLOBAL PROPERTY modio_implementation_files)
foreach(impl_file IN LISTS impl_files)
string(REPLACE ".ipp" ".cpp" renamed_file ${impl_file})
get_filename_component(file_only ${renamed_file} NAME)
install(
	FILES ${impl_file}
	RENAME ${file_only}
	DESTINATION ${CMAKE_INSTALL_PREFIX}/source
	COMPONENT source
)
endforeach()
add_custom_target(component_source
    COMMAND ${CMAKE_COMMAND} --install ${CMAKE_CURRENT_BINARY_DIR} --component source)
    set_target_properties(component_source PROPERTIES FOLDER "Installation/Packaging")
endmacro()
