/*
 *  Copyright (C) 2021-2024 mod.io Pty Ltd. <https://mod.io>
 *
 *  This file is part of the mod.io SDK.
 *
 *  Distributed under the MIT License. (See accompanying file LICENSE or
 *   view online at <https://github.com/modio/modio-sdk/blob/main/LICENSE>)
 *
 */

#pragma once

namespace Modio
{
	struct GuidV4;
	namespace Detail
	{
		namespace PlatformUtilService
		{
			inline GuidV4 GuidCreate();

			inline std::string GuidToString(const Modio::GuidV4& InGuid);

			inline Modio::GuidV4 GuidFromString(const std::string& InString);

		} // namespace PlatformUtilService
	} // namespace Detail
} // namespace Modio