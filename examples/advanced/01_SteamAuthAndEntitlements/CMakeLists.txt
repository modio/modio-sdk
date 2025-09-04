# 
#  Copyright (C) 2020-2022 mod.io Pty Ltd. <https://mod.io>
#  
#  This file is part of the mod.io SDK.
#  
#  Distributed under the MIT License. (See accompanying file LICENSE or 
#   view online at <https://github.com/modio/modio-sdk/blob/main/LICENSE>)
# 

set (CMAKE_CXX_STANDARD 17)

function(add_steamworks_example FileName)
		add_executable(modio_${FileName}  ${CMAKE_CURRENT_LIST_DIR}/${FileName}.cpp)
	target_link_libraries(modio_${FileName} ${MODIO_TARGET_NAME}Static)
	set_target_properties(modio_${FileName} PROPERTIES FOLDER "examples/advanced")
	target_include_directories(modio_${FileName} PRIVATE "${CMAKE_SOURCE_DIR}/ext/inja/third_party/include")
	install(
		TARGETS modio_${FileName} RUNTIME
		DESTINATION ${CMAKE_INSTALL_PREFIX}/examples/advanced
		COMPONENT examples
	)

	set(STEAMWORKS_SDK_PLATFORM win64)
    set(STEAMWORKS_LIB_NAME steam_api64.lib)
    set(STEAMWORKS_APPTICKET_LIB_NAME sdkencryptedappticket64.lib)
    include("${CMAKE_SOURCE_DIR}/ext/steamworks_sdk/SteamworksSetup.cmake")

     target_link_libraries(modio_${FileName} SteamworksSDK::SteamworksSDK SteamworksSDK::AppTicket)

    add_custom_command(
        TARGET modio_${FileName} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy ${STEAMWORKS_BASE_DIR}/public/steam/lib/win64/sdkencryptedappticket64.dll $<TARGET_FILE_DIR:modio_${FileName}>/sdkencryptedappticket64.dll
    )
    
    add_custom_command(
        TARGET modio_${FileName} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy ${STEAMWORKS_BASE_DIR}/redistributable_bin/win64/steam_api64.dll $<TARGET_FILE_DIR:modio_${FileName}>/steam_api64.dll
    )
    
    add_custom_command(
        TARGET modio_${FileName} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy ${STEAMWORKS_BASE_DIR}/steam_appid.txt $<TARGET_FILE_DIR:modio_${FileName}>/steam_appid.txt
    )

endfunction()

if (MODIO_BUILD_ADVANCED_EXAMPLES)
	add_steamworks_example(01_SteamAuthEntitlements)
endif()


