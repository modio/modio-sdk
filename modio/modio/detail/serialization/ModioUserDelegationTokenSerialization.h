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

#include "modio/core/entities/ModioUserDelegationToken.h"

#include "modio/detail/ModioConstants.h"
#include "modio/detail/ModioJsonHelpers.h"

namespace Modio
{
	inline void from_json(const nlohmann::json& Json, Modio::UserDelegationToken& Token)
	{
		Modio::Detail::ParseSafe(Json, Token.Entity, Modio::Detail::Constants::JSONKeys::TokenEntity);
		Modio::Detail::ParseSafe(Json, Token.Token, Modio::Detail::Constants::JSONKeys::TokenValue);
	}

} // namespace Modio