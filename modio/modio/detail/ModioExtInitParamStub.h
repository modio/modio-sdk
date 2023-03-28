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

namespace Modio
{
	namespace Detail
	{
		/// @brief Stub struct for handling internal-only initialization options
		struct ExtendedInitParamHandler
		{
			static void PostSessionDataInit(struct Modio::InitializeOptions& Opts) {};
			static void PostUserDataServiceInit(Modio::InitializeOptions& InitParams) {};
		};
	} // namespace Detail
} // namespace Modio