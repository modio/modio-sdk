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

		class OAuthTokenStateAccessor
		{
			Modio::Detail::OAuthToken& InToken;
		public:
			OAuthTokenStateAccessor(Modio::Detail::OAuthToken& InToken) : InToken(InToken) {}

			/// @docinternal
			/// @brief Optional here so that the accessors can return references to avoid memcpy, will always be set
			Modio::Optional<std::string>& Token()
			{
				return InToken.Token;
			}

			/// @docinternal
			/// @brief The date the token expires
			Modio::Timestamp& ExpireDate()
			{
				return InToken.ExpireDate;
			}

			/// @docinternal
			/// @brief The state of the token
			OAuthTokenState& State()
			{
				return InToken.State;
			}
		};

		inline void from_json(const nlohmann::json& Json, Modio::Detail::OAuthToken& InToken)
		{
			OAuthTokenStateAccessor Token(InToken);
			Detail::ParseSafe(Json, Token.ExpireDate(), "expiry");
			Detail::ParseSafe(Json, Token.State(), "status");
			std::string TokenString;

			if (Detail::ParseSafe(Json, TokenString, "token"))
			{
				Token.Token() = TokenString;
			}
			else
			{
				Token.State() = OAuthTokenState::Invalid;
			}
		}

		inline void to_json(nlohmann::json& Json, const Modio::Detail::OAuthToken& InToken)
		{
			if (InToken.GetRawState() == OAuthTokenState::Valid && InToken.GetRawToken().has_value())
			{
				Json = nlohmann::json {{"expiry", InToken.GetExpireDate()},
									   {"status", InToken.GetRawState()},
									   {"token", InToken.GetRawToken().value()}};
			}
			else
			{
				Json = nlohmann::json {{"expiry", InToken.GetExpireDate()}, {"status", OAuthTokenState::Invalid}};
			}
		}
	} // namespace Detail
} // namespace Modio