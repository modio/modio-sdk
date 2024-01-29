/*
 *  Copyright (C) 2023 mod.io Pty Ltd. <https://mod.io>
 *
 *  This file is part of the mod.io SDK.
 *
 *  Distributed under the MIT License. (See accompanying file LICENSE or
 *   view online at <https://github.com/modio/modio-sdk/blob/main/LICENSE>)
 *
 */

#pragma once

#include <string>

namespace Modio
{
	namespace Detail
	{
		static bool GetDefaultCommonDataPath(Modio::filesystem::path& CommonDataPath)
		{
			const char* HomeDir = std::getenv("HOME");
			if (HomeDir == nullptr)
			{
				Modio::Detail::Logger().Log(Modio::LogLevel::Error, Modio::LogCategory::File,
											"Could not get home directory environment variable!");
				return false;
			}
			CommonDataPath = Modio::filesystem::path(HomeDir) / "Library/Application Support/mod.io/common/";
			return true;
		}
	} // namespace Detail
} // namespace Modio
