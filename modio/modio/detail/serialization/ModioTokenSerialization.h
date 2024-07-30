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

#include "modio/core/entities/ModioToken.h"

#include "modio/detail/ModioJsonHelpers.h"

namespace Modio
{
	namespace Detail
	{
		inline void from_json(const nlohmann::json& Json, AccessTokenObject& AccessToken)
		{
			Modio::Detail::ParseSafe(Json, AccessToken.HttpResponseCode, "code");
			Modio::Detail::ParseSafe(Json, AccessToken.AccessToken, "access_token");
			Modio::Detail::ParseSafe(Json, AccessToken.DateExpires, "date_expires");
		}

		inline void from_json(const nlohmann::json& Json, Modio::Detail::OAuthToken& InToken)
		{
			Detail::ParseSafe(Json, InToken.ExpireDate, "expiry");
			Detail::ParseSafe(Json, InToken.State, "status");
			std::string TokenString = "";

			if (Detail::ParseSafe(Json, TokenString, "token"))
			{
				InToken.Token = TokenString;
			}
			else
			{
				InToken.State = OAuthTokenState::Invalid;
			}
		}

		inline void to_json(nlohmann::json& Json, const Modio::Detail::OAuthToken& InToken)
		{
			if (InToken.State == OAuthTokenState::Valid && InToken.Token.has_value())
			{
				Json = nlohmann::json {{"expiry", InToken.ExpireDate},
									   {"status", InToken.State},
									   {"token", InToken.Token.value()}};
			}
			else
			{
				Json = nlohmann::json {{"expiry", InToken.ExpireDate}, {"status", OAuthTokenState::Invalid}};
			}
		}
	} // namespace Detail
} // namespace Modio
