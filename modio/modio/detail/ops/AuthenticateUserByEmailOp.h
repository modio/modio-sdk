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

#include "modio/cache/ModioCacheService.h"
#include "modio/core/ModioBuffer.h"
#include "modio/core/ModioCoreTypes.h"
#include "modio/core/entities/ModioUser.h"
#include "modio/detail/CoreOps.h"
#include "modio/detail/ModioSDKSessionData.h"
#include "modio/detail/schema/AccessTokenObject.h"
#include "modio/http/ModioHttpParams.h"
#include "modio/userdata/ModioUserDataService.h"

#include "modio/detail/AsioWrapper.h"

#include <asio/yield.hpp>
namespace Modio
{
	namespace Detail
	{
		class AuthenticateUserByEmailOp
		{
		public:
			AuthenticateUserByEmailOp(Modio::EmailAuthCode EmailCode) : EmailCode(EmailCode) {}

			template<typename OperationState>
			void operator()(OperationState& Self, Modio::ErrorCode ec = {})
			{
				using namespace Modio;

				Detail::UserDataService& UserDataService =
					Detail::Services::GetGlobalService<Detail::UserDataService>();

				reenter(CoroutineState)
				{
					yield Detail::ComposedOps::PerformRequestAndGetResponseAsync(
						Local.ResponseBodyBuffer,
						ExchangeEmailSecurityCodeRequest.AppendPayloadValue(
							Modio::Detail::Constants::APIStrings::SecurityCode, EmailCode.InternalCode),
						Detail::CachedResponse::Disallow, std::move(Self));

					if (ec)
					{
						Self.complete(ec);
						return;
					}

					{
						Modio::Optional<Modio::Detail::Schema::AccessTokenObject> Token =
							Detail::TryMarshalResponse<Detail::Schema::AccessTokenObject>(Local.ResponseBodyBuffer);
						if (Token.has_value())
						{
							Local.AuthResponse = Token.value();
						}
						else
						{
							Self.complete(Modio::make_error_code(Modio::HttpError::InvalidResponse));
							return;
						}
					}

					Local.ResponseBodyBuffer.Clear();

					yield Detail::ComposedOps::PerformRequestAndGetResponseAsync(
						Local.ResponseBodyBuffer,
						Modio::Detail::GetAuthenticatedUserRequest.SetAuthTokenOverride(Local.AuthResponse.AccessToken),
						Detail::CachedResponse::Disallow, std::move(Self));

					if (ec)
					{
						Self.complete(ec);
						return;
					}
					
					{
						Modio::Optional<Modio::User> User =
							Detail::TryMarshalResponse<Modio::User>(Local.ResponseBodyBuffer);
						if (User.has_value())
						{
							Local.NewlyAuthenticatedUser = User.value();
						}
						else
						{
							Self.complete(Modio::make_error_code(Modio::HttpError::InvalidResponse));
							return;
						}
					}
					Local.ResponseBodyBuffer.Clear();

					Modio::Detail::SDKSessionData::InitializeForUser(std::move(Local.NewlyAuthenticatedUser),
																	 Modio::Detail::OAuthToken(Local.AuthResponse));

					Detail::Services::GetGlobalService<Detail::CacheService>().ClearCache();
					yield UserDataService.SaveUserDataToStorageAsync(std::move(Self));

					Self.complete({});
					return;
				}
			}

		private:
			asio::coroutine CoroutineState;
			Modio::EmailAuthCode EmailCode;
			struct
			{
				Modio::Detail::DynamicBuffer ResponseBodyBuffer;
				Modio::Detail::Schema::AccessTokenObject AuthResponse;
				Modio::User NewlyAuthenticatedUser;
			} Local;
		};
	} // namespace Detail
} // namespace Modio
#include <asio/unyield.hpp>
