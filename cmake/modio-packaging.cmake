macro(create_target_installing_component Component )
add_custom_target(component_${Component} 
    COMMAND ${CMAKE_COMMAND} --install ${CMAKE_CURRENT_BINARY_DIR} --component ${Component}
    DEPENDS ${MODIO_TARGET_NAME}
    # WORKING_DIRECTORY directory of the modio target
    )
    set_target_properties(component_${Component} PROPERTIES FOLDER "Installation/Packaging")
endmacro()


create_target_installing_component(header_only)

