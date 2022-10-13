/*
 *  Copyright (C) 2021 mod.io Pty Ltd. <https://mod.io>
 *
 *  This file is part of the mod.io SDK.
 *
 *  Distributed under the MIT License. (See accompanying file LICENSE or
 *   view online at <https://github.com/modio/modio-sdk/blob/main/LICENSE>)
 *
 */

#pragma once

#pragma push_macro("JSON_HAS_CPP_14")

#define JSON_HAS_CPP_14

#pragma push_macro("JSON_NOEXCEPTION")

#define JSON_NOEXCEPTION 

#ifdef MODIO_PLATFORM_UNREAL

	#include MODIO_UNREAL_PLATFORM_PREAMBLE


	#include "nlohmann/json.hpp"


	#include MODIO_UNREAL_PLATFORM_EPILOGUE
#else
	#include "nlohmann/json.hpp"
#endif

#pragma pop_macro("JSON_NOEXCEPTION")

#pragma pop_macro("JSON_HAS_CPP_14")