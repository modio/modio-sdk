/*
 *  Copyright (C) 2026 mod.io Pty Ltd. <https://mod.io>
 *
 *  This file is part of the mod.io SDK.
 *
 *  Distributed under the MIT License. (See accompanying file LICENSE or
 *   view online at <https://github.com/modio/modio-sdk/blob/main/LICENSE>)
 *
 */

#pragma once

#ifndef __OBJC__
	#error "AppleCoreIncludes.h must only be included from Obj-C++ (.mm) translation units."
#endif

#ifdef MODIO_PLATFORM_UNREAL
	#include MODIO_UNREAL_PLATFORM_PREAMBLE
#endif

#import <CoreFoundation/CoreFoundation.h>
#import <Foundation/Foundation.h>

#ifdef MODIO_PLATFORM_UNREAL
	#include MODIO_UNREAL_PLATFORM_EPILOGUE
#endif
