# 
#  Copyright (C) 2021 mod.io Pty Ltd. <https://mod.io>
#  
#  This file is part of the mod.io SDK.
#  
#  Distributed under the MIT License. (See accompanying file LICENSE or 
#   view online at <https://github.com/modio/modio-sdk/blob/main/LICENSE>)
# 

set(JSON_BuildTests OFF CACHE BOOL "" FORCE)
set(JSON_MultipleHeaders OFF CACHE BOOL "" FORCE)
add_subdirectory(json EXCLUDE_FROM_ALL)

set(nlohmann_json_DIR "${nlohmann_json_BINARY_DIR}" CACHE INTERNAL "")
#install(FILES "${CMAKE_CURRENT_LIST_DIR}/json/LICENSE.MIT"
#    DESTINATION "licenses/json/")
