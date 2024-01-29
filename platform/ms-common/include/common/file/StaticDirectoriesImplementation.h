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

#include "modio/detail/FilesystemWrapper.h"
#include "modio/detail/FmtWrapper.h"
#include <ShlObj.h>
#include <string>

namespace Modio
{
	namespace Detail
	{
		static bool GetDefaultCommonDataPath(Modio::filesystem::path& CommonDataPath)
		{
			PWSTR PublicPath = NULL;
			HRESULT hr = SHGetKnownFolderPath(FOLDERID_Public, 0, NULL, &PublicPath);
			if (SUCCEEDED(hr))
			{
				CommonDataPath = Modio::filesystem::path(std::wstring(PublicPath));
				CommonDataPath /= fmt::format("mod.io/");
				CoTaskMemFree(PublicPath);
				return true;
			}
			else
			{
				CoTaskMemFree(PublicPath);
				return false;
			}
		}
	} // namespace Detail
} // namespace Modio
