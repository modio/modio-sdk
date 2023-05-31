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

#include "modio/core/ModioStdTypes.h"
#include "modio/detail/JsonWrapper.h"
#include <memory>

namespace Modio
{
	namespace Detail
	{

		struct AccessTokenObject
		{
			std::int32_t HttpResponseCode;
			std::string AccessToken;
			Modio::Timestamp DateExpires;
		};

		/// @docnone
		MODIO_IMPL void from_json(const nlohmann::json& Json, AccessTokenObject& AccessToken);

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
			OAuthToken(Modio::Detail::AccessTokenObject AccessToken)
				: OAuthToken(AccessToken.AccessToken, AccessToken.DateExpires)
			{}
			void SetInvalidState()
			{
				State = OAuthTokenState::Invalid;
			}
			OAuthTokenState GetTokenState() const
			{
				if (State == OAuthTokenState::Valid)
				{
					auto Now = std::chrono::system_clock::now();
					std::size_t CurrentUTCTime =
						std::chrono::duration_cast<std::chrono::seconds>(Now.time_since_epoch()).count();

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

			/// @docnone
			friend MODIO_IMPL void from_json(const nlohmann::json& Json, Modio::Detail::OAuthToken& InToken);

			/// @docnone
			friend MODIO_IMPL void to_json(nlohmann::json& Json, const Modio::Detail::OAuthToken& InToken);

		private:
			// Optional here so that the accessors can return references to avoid memcpy, will always be set
			Modio::Optional<std::string> Token;
			Modio::Timestamp ExpireDate;

		private:
			OAuthTokenState State = OAuthTokenState::Invalid;

			/// @docnone
			friend bool operator==(const Modio::Detail::OAuthToken& A, const Modio::Detail::OAuthToken& B)
			{
				return (A.Token == B.Token && A.ExpireDate == B.ExpireDate && A.State == B.State &&
						A.NoToken == B.NoToken);
			}
		};	
	} // namespace Detail
} // namespace Modio

#ifndef MODIO_SEPARATE_COMPILATION
	#include "modio/core/entities/ModioToken.ipp"
#endif
