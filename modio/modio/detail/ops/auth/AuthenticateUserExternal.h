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
#include "modio/detail/AsioWrapper.h"
#include "modio/http/ModioHttpParams.h"
#include "modio/userdata/ModioUserDataService.h"
#include <memory>
namespace Modio
{
	namespace Detail
	{
#include <asio/yield.hpp>
		class AuthenticateUserExternal
		{
		public:
			AuthenticateUserExternal(Modio::Detail::HttpRequestParams AuthenticationParams)
			{
				LocalState = std::make_shared<Impl>();
				LocalState->AuthenticationParams = AuthenticationParams;
			};
			template<typename CoroType>
			void operator()(CoroType& Self, Modio::ErrorCode ec = {})
			{
				auto& UserDataService = Modio::Detail::Services::GetGlobalService<Modio::Detail::UserDataService>();
				reenter(LocalState->CoroutineState)
				{
					yield Modio::Detail::ComposedOps::PerformRequestAndGetResponseAsync(
						LocalState->ResponseBuffer, LocalState->AuthenticationParams, Detail::CachedResponse::Disallow,
						std::move(Self));
					if (ec)
					{
						Self.complete(ec);
						return;
					}

					{
						Modio::Optional<Modio::Detail::Schema::AccessTokenObject> Token =
							Detail::TryMarshalResponse<Detail::Schema::AccessTokenObject>(LocalState->ResponseBuffer);
						if (Token.has_value())
						{
							LocalState->AuthResponse = Token.value();
						}
						else
						{
							Self.complete(Modio::make_error_code(Modio::HttpError::InvalidResponse));
							return;
						}
					}

					LocalState->ResponseBuffer.Clear();

					yield Modio::Detail::ComposedOps::PerformRequestAndGetResponseAsync(
						LocalState->ResponseBuffer,
						Modio::Detail::GetAuthenticatedUserRequest.SetAuthTokenOverride(
							LocalState->AuthResponse.AccessToken),
						Modio::Detail::CachedResponse::Disallow, std::move(Self));

					if (ec)
					{
						Self.complete(ec);
						return;
					}

					{
						Modio::Optional<Modio::Detail::AuthenticatedUser> User =
							Detail::TryMarshalResponse<Modio::Detail::AuthenticatedUser>(LocalState->ResponseBuffer);
						if (User.has_value())
						{
							LocalState->AuthenticatedUserData = User.value();
						}
						else
						{
							Self.complete(Modio::make_error_code(Modio::HttpError::InvalidResponse));
							return;
						}
					}
					LocalState->ResponseBuffer.Clear();

					if (!Modio::Detail::SDKSessionData::GetAuthenticatedUser() ||
						LocalState->AuthenticatedUserData.User.UserId !=
							Modio::Detail::SDKSessionData::GetAuthenticatedUser()->UserId)
					{
						yield UserDataService.ClearUserDataAsync(std::move(Self));
						Modio::Detail::Services::GetGlobalService<Modio::Detail::CacheService>().ClearCache();
					}

					Modio::Detail::SDKSessionData::InitializeForAuthenticatedUser(
						std::move(LocalState->AuthenticatedUserData),
						Modio::Detail::OAuthToken(LocalState->AuthResponse));
					yield UserDataService.SaveUserDataToStorageAsync(std::move(Self));
					Self.complete({});
					return;
				}
			}

		private:
			struct Impl
			{
				asio::coroutine CoroutineState;
				Modio::Detail::DynamicBuffer ResponseBuffer;
				Modio::Detail::HttpRequestParams AuthenticationParams;
				Modio::Detail::Schema::AccessTokenObject AuthResponse;
				Modio::Detail::AuthenticatedUser AuthenticatedUserData;
			};
			Modio::StableStorage<Impl> LocalState;
		};
#include <asio/unyield.hpp>
	} // namespace Detail
} // namespace Modio