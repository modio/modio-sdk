/*
 *  Copyright (C) 2021 mod.io Pty Ltd. <https://mod.io>
 *
 *  This file is part of the mod.io SDK. All rights reserved.
 *  Redistribution prohibited without prior written consent of mod.io Pty Ltd.
 *
 */

 // clang-format off

#include "Clang/ClangPlatformCompilerPreSetup.h"
#include "HAL/PlatformMisc.h"
#include "Misc/AssertionMacros.h"
#include "Linux/LinuxPlatform.h"
#include "Linux/LinuxPlatformCompilerPreSetup.h"
#include "Linux/LinuxPlatformCompilerSetup.h"

// NOTE!! I tried to add "-Wshadow" as a "target_compile_options" in many CMake files:
// platform/macos/macos, asio.cmake, howeever it dit not like it.
#pragma GCC diagnostic push

// This is required by file "resolver_base.hpp" in line 69 which complains about this:
// Declaration shadows a variable in namespace 'asio::ip'
// https://github.com/chriskohlhoff/asio/issues/721
#pragma GCC diagnostic ignored "-Wshadow"

// clang-format on
