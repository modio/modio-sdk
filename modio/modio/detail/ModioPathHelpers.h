/*
 *  Copyright (C) 2025 mod.io Pty Ltd. <https://mod.io>
 *
 *  This file is part of the mod.io SDK.
 *
 *  Distributed under the MIT License. (See accompanying file LICENSE or
 *   view online at <https://github.com/modio/modio-sdk/blob/main/LICENSE>)
 *
 */

#pragma once

#include "modio/detail/ModioStringHelpers.h"

namespace Modio
{
	namespace Detail
	{
		namespace Path
		{
			/// @brief Converts a path to all lowercase characters
			/// @param InPath The path to convert to lowercase
			/// @return A path with all lowercase characters
			inline Modio::filesystem::path PathToLowercase(const Modio::filesystem::path& InPath)
			{
				return Modio::filesystem::path(Modio::Detail::String::ToLowercase(InPath.generic_string()));
			}
		} // namespace Path
	} // namespace Detail
} // namespace Modio