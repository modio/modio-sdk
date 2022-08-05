set(JSON_BuildTests OFF CACHE BOOL "" FORCE)
set(JSON_MultipleHeaders OFF CACHE BOOL "" FORCE)
add_subdirectory(json EXCLUDE_FROM_ALL)

set(nlohmann_json_DIR "${nlohmann_json_BINARY_DIR}" CACHE INTERNAL "")
#install(FILES "${CMAKE_CURRENT_LIST_DIR}/json/LICENSE.MIT"
#    DESTINATION "licenses/json/")
