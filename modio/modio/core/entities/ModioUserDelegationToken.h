/*
 *  Copyright (C) 2021-2023 mod.io Pty Ltd. <https://mod.io>
 *
 *  This file is part of the mod.io SDK.
 *
 *  Distributed under the MIT License. (See accompanying file LICENSE or
 *   view online at <https://github.com/modio/modio-sdk/blob/main/LICENSE>)
 *
 */

#pragma once

#include "modio/core/ModioSplitCompilation.h"
#include <string>

namespace Modio
{
	/// @docpublic
	/// @brief Contains information about a UserDelegationToken used for S2S transactions
	struct UserDelegationToken
	{
		/// @brief The type of token we are getting
		std::string Entity {};

		/// @brief The token that we got
		std::string Token {};
	};
} // namespace Modio
