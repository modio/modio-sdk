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

#include "modio/core/ModioStdTypes.h"
#include "modio/core/entities/ModioModInfoList.h"
#include "modio/core/entities/ModioUser.h"
#include "modio/detail/schema/AccessTokenObject.h"

#include <memory>

namespace Modio
{
	namespace Detail
	{
		enum class OAuthTokenState : std::uint8_t
		{
			Valid,
			Expired,
			Invalid
		};

		struct OAuthToken
		{
			OAuthToken() = default;
			OAuthToken(const std::string& InToken, Modio::Timestamp InExpireDate)
				: Token(InToken),
				  ExpireDate(InExpireDate),
				  State(OAuthTokenState::Valid)
			{}
			OAuthToken(Modio::Detail::Schema::AccessTokenObject AccessToken)
				: OAuthToken(AccessToken.AccessToken, AccessToken.DateExpires)
			{}
			OAuthTokenState GetTokenState() const
			{
				auto Now = std::chrono::system_clock::now();
				std::size_t CurrentUTCTime =
					std::chrono::duration_cast<std::chrono::seconds>(Now.time_since_epoch()).count();

				if (State == OAuthTokenState::Valid)
				{
					return CurrentUTCTime <= ExpireDate ? OAuthTokenState::Valid : OAuthTokenState::Invalid;
				}
				return OAuthTokenState::Invalid;
			}
			operator bool() const
			{
				return GetTokenState() == OAuthTokenState::Valid;
			}
			const Modio::Optional<std::string> GetToken() const
			{
				if (GetTokenState() == OAuthTokenState::Valid)
				{
					return Token.value();
				}
				return {};
			}

			static MODIO_IMPL Modio::Optional<std::string> NoToken;
			
			friend void from_json(const nlohmann::json& Json, Modio::Detail::OAuthToken& InToken)
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

			friend void to_json(nlohmann::json& Json, const Modio::Detail::OAuthToken& InToken)
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

		private:
			// Optional here so that the accessors can return references to avoid memcpy, will always be set
			Modio::Optional<std::string> Token;
			Modio::Timestamp ExpireDate;

		private:
			OAuthTokenState State = OAuthTokenState::Invalid;

			friend bool operator==(const Modio::Detail::OAuthToken& A, const Modio::Detail::OAuthToken& B)
			{
				return (A.Token == B.Token && A.ExpireDate == B.ExpireDate && A.State == B.State &&
						A.NoToken == B.NoToken);
			}
		};

		struct ProfileData
		{
			ProfileData(Modio::User InUser, Modio::Detail::OAuthToken AccessToken)
				: User(InUser),
				  Token(AccessToken)
			{}

			const Modio::User& GetUser() const
			{
				return User;
			}

			const Modio::Detail::Avatar GetAvatar() const
			{
				return User.Avatar;
			}

			const Modio::Detail::OAuthToken& GetToken() const
			{
				return Token;
			}

		private:
			Modio::User User;
			Modio::Detail::OAuthToken Token;

			friend bool operator==(const Modio::Detail::ProfileData& A, const Modio::Detail::ProfileData& B)
			{
				return (A.User == B.User && A.Token == B.Token);
			}
		};
	} // namespace Detail
} // namespace Modio

#ifndef MODIO_SEPARATE_COMPILATION
	#include "ModioUserProfile.ipp"
#endif
