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
#include <chrono>

namespace Modio
{
	namespace Detail
	{
		/// @docinternal
		/// @brief Access token object
		struct AccessTokenObject
		{
			/// @brief The default constructor
			std::int32_t HttpResponseCode {};

			/// @brief The access token
			std::string AccessToken {};

			/// @brief The date the token expires
			Modio::Timestamp DateExpires {};
		};

		/// @docinternal
		/// @brief OAuth token object
		enum class OAuthTokenState : std::uint8_t
		{
			Valid,
			Expired,
			Invalid
		};

		/// @docinternal
		/// @brief Serialization helper class
		class OAuthTokenStateAccessor;

		/// @docinternal
		/// @brief OAuth token object
		struct OAuthToken
		{
			/// @docinternal
			/// @brief The default constructor
			OAuthToken() = default;

			/// @docinternal
			/// @brief Constructor to create a valid token
			OAuthToken(const std::string& InToken, Modio::Timestamp InExpireDate)
				: Token(InToken),
				  ExpireDate(InExpireDate),
				  State(OAuthTokenState::Valid)
			{}

			/// @docinternal
			/// @brief Constructor that creates token from an access token object
			OAuthToken(Modio::Detail::AccessTokenObject AccessToken)
				: OAuthToken(AccessToken.AccessToken, AccessToken.DateExpires)
			{}

			/// @docinternal
			/// @brief Invalidate the token
			void SetInvalidState()
			{
				State = OAuthTokenState::Invalid;
			}

			/// @docinternal
			/// @brief Get the state of the token
			OAuthTokenState GetTokenState() const
			{
				if (State == OAuthTokenState::Valid)
				{
					auto Now = std::chrono::system_clock::now();
					std::size_t CurrentUTCTime =
						std::size_t(std::chrono::duration_cast<std::chrono::seconds>(Now.time_since_epoch()).count());

					return CurrentUTCTime <= ExpireDate ? OAuthTokenState::Valid : OAuthTokenState::Invalid;
				}
				return OAuthTokenState::Invalid;
			}

			/// @docinternal
			/// @brief Implicit conversion to bool
			operator bool() const
			{
				return GetTokenState() == OAuthTokenState::Valid;
			}

			/// @docinternal
			/// @brief Get the token
			Modio::Optional<std::string> GetToken() const
			{
				if (GetTokenState() == OAuthTokenState::Valid)
				{
					return Token.value();
				}
				return {};
			}

			static MODIO_IMPL Modio::Optional<std::string> NoToken;

			/// @docinternal
			/// @brief Access token for serialization
			Modio::Optional<std::string> GetRawToken() const 
			{
				return Token;
			}

			/// @docinternal
			/// @brief Access expire for serialization
			Modio::Timestamp GetExpireDate() const
			{
				return ExpireDate;
			}

			/// @docinternal
			/// @brief Access the state of the token for serialization
			OAuthTokenState GetRawState() const
			{
				return State;
			}
		private:
			/// @docinternal
			/// @brief Optional here so that the accessors can return references to avoid memcpy, will always be set
			Modio::Optional<std::string> Token {};

			/// @docinternal
			/// @brief The date the token expires
			Modio::Timestamp ExpireDate {};

		private:
			/// @docinternal
			/// @brief The state of the token
			OAuthTokenState State = OAuthTokenState::Invalid;

			/// @docnone
			friend bool operator==(const Modio::Detail::OAuthToken& A, const Modio::Detail::OAuthToken& B)
			{
				return (A.Token == B.Token && A.ExpireDate == B.ExpireDate && A.State == B.State &&
						A.NoToken == B.NoToken);
			}

			friend class Modio::Detail::OAuthTokenStateAccessor;
		};	
	} // namespace Detail
} // namespace Modio
