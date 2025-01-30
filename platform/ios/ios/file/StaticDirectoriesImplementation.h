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
			char *HomeDirChar = std::getenv("HOME");
            
            // First check if there are any contents on the response
            if (HomeDirChar == nullptr)
            {
                Modio::Detail::Logger().Log(Modio::LogLevel::Error, Modio::LogCategory::File,
                                            "Could not get home directory environment variable!");
                return false;
            }
            
            std::string HomeDir(HomeDirChar);
            // Then check if the path is not empty
			if (HomeDir.empty())
			{
				Modio::Detail::Logger().Log(Modio::LogLevel::Error, Modio::LogCategory::File,
											"Could not get home directory environment variable!");
				return false;
			}

			// In case this code runs on device, "getenv" above adds "private" to the path, but that prevents
			// the correct initialization of files. This check makes sure to remove it.
			std::string Priv = "/private";
			size_t pos = HomeDir.find(Priv, 0);
			if (pos != std::string::npos && pos == 0)
			{
				HomeDir.replace(pos, Priv.length(), "");
			}

			CommonDataPath = Modio::filesystem::path(HomeDir) / "Documents/mod.io/common/";
			return true;
		}
	} // namespace Detail
} // namespace Modio
