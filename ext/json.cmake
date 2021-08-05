set(JSON_BuildTests OFF CACHE BOOL "" FORCE)

add_subdirectory(json EXCLUDE_FROM_ALL)

#install(FILES "${CMAKE_CURRENT_LIST_DIR}/json/LICENSE.MIT"
#    DESTINATION "licenses/json/")
