# 
#  Copyright (C) 2021 mod.io Pty Ltd. <https://mod.io>
#  
#  This file is part of the mod.io SDK.
#  
#  Distributed under the MIT License. (See accompanying file LICENSE or 
#   view online at <https://github.com/modio/modio-sdk/blob/main/LICENSE>)
# 

set(APP_NAME "modioSSO")
# Update the bundle identifier to match your environment, which has "Sign in with Apple" enabled
set(APP_BUNDLE_IDENTIFIER "io.mod.test.modioiap")
set(DEPLOYMENT_TARGET 15.0)
set(PRODUCT_NAME ${APP_NAME})
set(EXECUTABLE_NAME ${APP_NAME})
set(IPHONEOS_DEPLOYMENT_TARGET ${DEPLOYMENT_TARGET})

set(APP_HEADER_FILES
    AppDelegate.h
    AppViewController.h
    ModioSDKHelper.h
)

set(APP_SOURCE_FILES
    main.m
    AppDelegate.m
    AppViewController.m
    ModioSDKHelper.mm
)

set(RESOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/resources/MainMenu.storyboard
    ${CMAKE_CURRENT_SOURCE_DIR}/resources/icons.icns
)

if(${CMAKE_GENERATOR} MATCHES "^Xcode.*")
    add_executable(
        ${APP_NAME}
        MACOSX_BUNDLE
        ${APP_HEADER_FILES}
        ${APP_SOURCE_FILES}
        ${RESOURCES}
    )
else()
    ## If the executable is not for Xcode, then avoid adding the storyboard,
    ## it will be compiled in a later stage
    add_executable(
        ${APP_NAME}
        MACOSX_BUNDLE
        ${APP_HEADER_FILES}
        ${APP_SOURCE_FILES}
    )
endif()

# Turn on ARC
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fcxx-modules -fobjc-arc")
target_compile_features(${APP_NAME} PRIVATE cxx_std_17)

# Locate system libraries
find_library(APPKIT UIKit)
find_library(FOUNDATION Foundation)
find_library(CFNETWORK CFNetwork)
find_library(AUTHSERV AuthenticationServices)

# link the frameworks located above
target_link_libraries(${APP_NAME} ${APPKIT})
target_link_libraries(${APP_NAME} ${FOUNDATION})
target_link_libraries(${APP_NAME} ${CFNETWORK})
target_link_libraries(${APP_NAME} ${AUTHSERV})
target_link_libraries(${APP_NAME} clipp)

# Modio dependency to the Static Library
target_link_libraries(${APP_NAME} ${MODIO_TARGET_NAME}Static)

# Create the app target
set_target_properties(${APP_NAME} PROPERTIES
    RESOURCE "${RESOURCES}"
    MACOSX_BUNDLE TRUE
    MACOSX_BUNDLE_BUNDLE_NAME ${APP_NAME}
    MACOSX_BUNDLE_BUNDLE_IDENTIFIER ${APP_BUNDLE_IDENTIFIER}
    MACOSX_BUNDLE_EXECUTABLE_NAME ${APP_NAME}
    MACOSX_BUNDLE_INFO_STRING ${APP_BUNDLE_IDENTIFIER}
    MACOSX_BUNDLE_GUI_IDENTIFIER ${APP_BUNDLE_IDENTIFIER}
    MACOSX_BUNDLE_ICON_FILE ${CMAKE_CURRENT_SOURCE_DIR}/resources/icon.png
    MACOSX_PACKAGE_LOCATION ${CMAKE_CURRENT_SOURCE_DIR}/resources/icons.icns
    MACOSX_BUNDLE_LONG_VERSION_STRING "1.0"
    MACOSX_BUNDLE_SHORT_VERSION_STRING "1.0"
    MACOSX_BUNDLE_BUNDLE_VERSION "1.0"
    MACOSX_BUNDLE_COPYRIGHT "Copyright modio"
    XCODE_ATTRIBUTE_PRODUCT_BUNDLE_IDENTIFIER "io.mod.test.modioiap"
    XCODE_ATTRIBUTE_CLANG_ENABLE_OBJC_ARC YES
    XCODE_ATTRIBUTE_COMBINE_HIDPI_IMAGES NO
    XCODE_ATTRIBUTE_ENABLE_TESTABILITY YES
    XCODE_ATTRIBUTE_GCC_SYMBOLS_PRIVATE_EXTERN YES
    XCODE_ATTRIBUTE_CODE_SIGNING_REQUIRED "YES"
    XCODE_ATTRIBUTE_CODE_SIGN_IDENTITY "-"
    XCODE_ATTRIBUTE_DEBUG_INFORMATION_FORMAT "dwarf-with-dsym"
    XCODE_ATTRIBUTE_ENABLE_SIGN_IN_WITH_APPLE "YES"
    XCODE_ATTRIBUTE_TARGETED_DEVICE_FAMILY "1,2"
    MACOSX_BUNDLE_INFO_PLIST ${CMAKE_CURRENT_SOURCE_DIR}/resources/Info-iOS.plist
    XCODE_ATTRIBUTE_CLANG_ENABLE_MODULES YES
    XCODE_ATTRIBUTE_INFOPLIST_FILE ${CMAKE_CURRENT_SOURCE_DIR}/resources/Info-iOS.plist
    XCODE_ATTRIBUTE_CODE_SIGN_ENTITLEMENTS ${CMAKE_CURRENT_SOURCE_DIR}/resources/modioSSO.entitlements
)

if(NOT ${CMAKE_GENERATOR} MATCHES "^Xcode.*")
# Compile the storyboard file with the ibtool.
add_custom_command(TARGET ${APP_NAME} POST_BUILD
                   COMMAND ibtool
                           --errors
                           --warnings
                           --notices
                           --output-format human-readable-text
                           --compile ${CMAKE_CURRENT_BINARY_DIR}/${APP_NAME}.app/Contents/Resources/MainMenu.storyboardc
                                     ${CMAKE_CURRENT_SOURCE_DIR}/resources/MainMenu.storyboard
                   COMMENT "Compiling storyboard")
endif()

target_include_directories(${APP_NAME} PUBLIC .)

