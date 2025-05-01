# 
#  Copyright (C) 2020-2022 mod.io Pty Ltd. <https://mod.io>
#  
#  This file is part of the mod.io SDK.
#  
#  Distributed under the MIT License. (See accompanying file LICENSE or 
#   view online at <https://github.com/modio/modio-sdk/blob/main/LICENSE>)
# 

set (CMAKE_CXX_STANDARD 17)


function(add_example FileName)
	add_executable(modio_${FileName}  ${CMAKE_CURRENT_LIST_DIR}/${FileName}.cpp)
	install(
		TARGETS modio_${FileName} RUNTIME
		DESTINATION ${CMAKE_INSTALL_PREFIX}/examples
		COMPONENT examples
	)
	target_link_libraries(modio_${FileName} ${MODIO_TARGET_NAME}Static)
	set_target_properties(modio_${FileName} PROPERTIES FOLDER "examples")
	target_include_directories(modio_${FileName} PRIVATE "${CMAKE_SOURCE_DIR}/ext/inja/third_party/include")

endfunction()



if (MODIO_BUILD_EXAMPLES)
	add_example(01_Initialization)
	add_example(02_ModQueries)
	add_example(03_Authentication)
	add_example(04_Threading)
	add_example(05_SubscriptionManagement)
	add_example(06_ForceUninstall)
	add_example(07_ReportMod)
	add_example(08_SubmitMod)
	add_example(09_MonetizationWalletManagement)
	add_example(10_MonetizationPurchase)
	add_example(11_TempModSets)
	add_example(12_RefreshEntitlements)
	add_example(13_MetricsSession)
endif()

if (MODIO_BUILD_ADVANCED_EXAMPLES)
    add_subdirectory(advanced)
endif()

