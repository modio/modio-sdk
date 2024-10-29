# 
#  Copyright (C) 2021 mod.io Pty Ltd. <https://mod.io>
#  
#  This file is part of the mod.io SDK.
#  
#  Distributed under the MIT License. (See accompanying file LICENSE or 
#   view online at <https://github.com/modio/modio-sdk/blob/main/LICENSE>)
# 

project(${MODIO_TARGET_NAME} LANGUAGES Swift)

set(APP_NAME "modioSwift")
# Update the bundle identifier to match your environment, which has "Sign in with Apple" enabled
set(APP_BUNDLE_IDENTIFIER "io.mod.modioSwift")
set(DEPLOYMENT_TARGET 15.0)
set(PRODUCT_NAME ${APP_NAME})
set(EXECUTABLE_NAME ${APP_NAME})
set(IPHONEOS_DEPLOYMENT_TARGET ${DEPLOYMENT_TARGET})

set(APP_HEADER_FILES
    modioSDK-BridgingHeader.h
    ModioSDKHelper.h
)

set(APP_SOURCE_FILES
    ModioSDKApp.swift
    ContentView.swift
    ModioSDKHelper.mm
)

set(RESOURCES
    resources/icons.icns
)

# Verify that we have a new enough compiler
if("${CMAKE_Swift_COMPILER_VERSION}" VERSION_LESS 5.9)
  message(FATAL_ERROR "Bidirectional C++ Interop requires Swift 5.9 or greater. Have ${CMAKE_Swift_COMPILER_VERSION}")
endif()

if(NOT "${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang" AND
   NOT "${CMAKE_CXX_COMPILER_ID}" STREQUAL "AppleClang")
  message(FATAL_ERROR "Project requires building with Clang.
  Have ${CMAKE_CXX_COMPILER_ID}")
endif()

set(CMAKE_OSX_DEPLOYMENT_TARGET 13.0)
set(CMAKE_CXX_STANDARD 17)

add_executable(
    ${APP_NAME}
    MACOSX_BUNDLE
    ${APP_HEADER_FILES}
    ${APP_SOURCE_FILES}
    ${RESOURCES}
)

# Turn on ARC
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fcxx-modules -fobjc-arc")
target_compile_features(${APP_NAME} PRIVATE cxx_std_17)

# Locate system libraries
# find_library(APPKIT UIKit)
find_library(FOUNDATION Foundation)
find_library(CFNETWORK CFNetwork)
find_library(AUTHSERV AuthenticationServices)

# link the frameworks located above
# target_link_libraries(${APP_NAME} ${APPKIT})
target_link_libraries(${APP_NAME} ${FOUNDATION})
target_link_libraries(${APP_NAME} ${CFNETWORK})
target_link_libraries(${APP_NAME} ${AUTHSERV})
target_link_libraries(${APP_NAME} clipp)

# Modio dependency to the Static Library
target_link_libraries(${APP_NAME} ${MODIO_TARGET_NAME}Static)
target_compile_options(${MODIO_TARGET_NAME} INTERFACE "")

# Create the app target
set_target_properties(${APP_NAME} PROPERTIES
    RESOURCE "${RESOURCES}"
    MACOSX_BUNDLE TRUE
    MACOSX_BUNDLE_INFO_PLIST ${CMAKE_CURRENT_SOURCE_DIR}/resources/Info-iOS.plist
    MACOSX_BUNDLE_BUNDLE_NAME ${APP_NAME}
    MACOSX_BUNDLE_BUNDLE_IDENTIFIER ${APP_BUNDLE_IDENTIFIER}
    MACOSX_BUNDLE_EXECUTABLE_NAME ${APP_NAME}
    MACOSX_BUNDLE_INFO_STRING ${APP_BUNDLE_IDENTIFIER}
    MACOSX_BUNDLE_GUI_IDENTIFIER ${APP_BUNDLE_IDENTIFIER}
    MACOSX_BUNDLE_LONG_VERSION_STRING "1.0"
    MACOSX_BUNDLE_SHORT_VERSION_STRING "1.0"
    MACOSX_BUNDLE_BUNDLE_VERSION "1.0"
    MACOSX_BUNDLE_COPYRIGHT "Copyright mod.io"
    XCODE_ATTRIBUTE_PRODUCT_BUNDLE_IDENTIFIER ${APP_BUNDLE_IDENTIFIER}
    MACOSX_BUNDLE_ICON_FILE ${CMAKE_CURRENT_SOURCE_DIR}/resources/icon.png
    MACOSX_PACKAGE_LOCATION ${CMAKE_CURRENT_SOURCE_DIR}/resources/icons.icns
    XCODE_ATTRIBUTE_SWIFT_OBJC_BRIDGING_HEADER ${CMAKE_CURRENT_SOURCE_DIR}/modioSDK-BridgingHeader.h
    XCODE_ATTRIBUTE_CLANG_ENABLE_OBJC_ARC YES
    XCODE_ATTRIBUTE_COMBINE_HIDPI_IMAGES NO
    XCODE_ATTRIBUTE_ENABLE_TESTABILITY YES
    XCODE_ATTRIBUTE_GCC_SYMBOLS_PRIVATE_EXTERN YES
    XCODE_ATTRIBUTE_CODE_SIGNING_REQUIRED "YES"
    XCODE_ATTRIBUTE_CODE_SIGN_IDENTITY "-"
    XCODE_ATTRIBUTE_DEBUG_INFORMATION_FORMAT "dwarf-with-dsym"
    XCODE_ATTRIBUTE_ENABLE_SIGN_IN_WITH_APPLE "YES"
    XCODE_ATTRIBUTE_TARGETED_DEVICE_FAMILY "1,2"
    XCODE_ATTRIBUTE_CLANG_ENABLE_MODULES YES
)

target_include_directories(${APP_NAME} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_compile_options(${APP_NAME} PUBLIC
  "$<$<COMPILE_LANGUAGE:Swift>:-cxx-interoperability-mode=default>")
