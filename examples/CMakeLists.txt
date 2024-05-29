# 
#  Copyright (C) 2020-2022 mod.io Pty Ltd. <https://mod.io>
#  
#  This file is part of the mod.io SDK.
#  
#  Distributed under the MIT License. (See accompanying file LICENSE or 
#   view online at <https://github.com/modio/modio-sdk/blob/main/LICENSE>)
# 

set (MODIO_EXAMPLES_API_KEY "ca842a1f60c40bc8fb2044bc9932d763" CACHE STRING "API Key to use in examples")
set (MODIO_EXAMPLES_GAME_ID "3609" CACHE STRING "Game ID to use in examples")
set (MODIO_EXAMPLES_SERVER "LIVE" CACHE STRING "Server to use in examples")

set (CMAKE_CXX_STANDARD 17)

function(add_example FileName)
	add_executable(modio_${FileName}  ${CMAKE_CURRENT_LIST_DIR}/${FileName}.cpp)
	target_link_libraries(modio_${FileName} ${MODIO_TARGET_NAME}Static)
	set_target_properties(modio_${FileName} PROPERTIES FOLDER "examples")
	target_include_directories(modio_${FileName} PRIVATE "${CMAKE_SOURCE_DIR}/ext/inja/third_party/include")
	install(
		TARGETS modio_${FileName} RUNTIME
		DESTINATION ${CMAKE_INSTALL_PREFIX}/examples
		COMPONENT examples
	)
endfunction()

if (MODIO_BUILD_EXAMPLES)
	add_example(01_Initialization)
	add_example(02_ModQueries)
	add_example(03_Authentication)
	add_example(04_SubscriptionManagement)
	add_example(05_ForceUninstall)
	add_example(06_ReportMod)
	add_example(07_Threading)
	add_example(08_SubmitMod)
	add_example(09_MonetizationWalletManagement)
	add_example(10_MonetizationPurchase)
endif()
