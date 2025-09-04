/*
 *  Copyright (C) 2021-2025 mod.io Pty Ltd. <https://mod.io>
 *
 *  This file is part of the mod.io SDK.
 *
 *  Distributed under the MIT License. (See accompanying file LICENSE or
 *   view online at <https://github.com/modio/modio-sdk/blob/main/LICENSE>)
 *
 */

#pragma once

#if __cplusplus >= 201703L
	#include <string_view>
#endif
#include <string>
namespace Modio
{
#if __cplusplus >= 202002L
	/// @brief direct pass through for u8string_views
	/// @param S the u8 string view to convert
	/// @return the same view
	static inline std::u8string_view ToModioString(const std::u8string_view& S)
	{
		return S;
	}

	/// @brief direct pass through for u8strings
	/// @param S the u8 string to convert
	/// @return the same
	static inline std::u8string ToModioString(const std::u8string& S)
	{
		return S;
	}
#endif
} // namespace Modio
