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
#include "modio/detail/ops/http/PerformRequestAndGetResponseOp.h"
#include "modio/detail/ops/userdata/VerifyUserAuthenticationOp.h"
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
			}
			template<typename CoroType>
			void operator()(CoroType& Self, Modio::ErrorCode ec = {})
			{
				auto& UserDataService = Modio::Detail::Services::GetGlobalService<Modio::Detail::UserDataService>();
				bool bShouldDisableModManagement = true;

				reenter(LocalState->CoroutineState)
				{
					
					yield Modio::Detail::PerformRequestAndGetResponseAsync(
						LocalState->ResponseBuffer, LocalState->AuthenticationParams, Detail::CachedResponse::Disallow,
						std::move(Self));

					{
						Modio::Optional<ResponseError> Error =
							TryMarshalResponse<ResponseError>(LocalState->ResponseBuffer);
						if (Error.has_value())
						{
							Modio::ErrorCode ErrRef =
								Modio::make_error_code(static_cast<Modio::ApiError>(Error->ErrorRef));

							if (ErrRef == Modio::ApiError::PSNChildAccountNotPermitted ||
								ErrRef == Modio::ApiError::XBoxLiveChildAccountNotPermitted ||
								ErrRef == Modio::ApiError::PSNNotAllowedToInteractWithUGC||
								ErrRef == Modio::ApiError::XBoxLiveNotAllowedToInteractWithUGC)
							{
								Self.complete(Modio::make_error_code(Modio::ParentalControlRestriction::ParentalControlRestriction));
								return;
							}

						}
					}


					if (ec)
					{
						Self.complete(ec);
						return;
					}
					{
						Modio::Optional<Modio::Detail::AccessTokenObject> Token =
							Detail::TryMarshalResponse<Detail::AccessTokenObject>(LocalState->ResponseBuffer);
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
					yield Modio::Detail::PerformRequestAndGetResponseAsync(
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
						Modio::Optional<Modio::User> User =
							Detail::TryMarshalResponse<Modio::User>(LocalState->ResponseBuffer);
						if (User.has_value())
						{
							LocalState->AuthUser = User.value();
						}
						else
						{
							Self.complete(Modio::make_error_code(Modio::HttpError::InvalidResponse));
							return;
						}
					}
					LocalState->ResponseBuffer.Clear();

					if (!Modio::Detail::SDKSessionData::GetAuthenticatedUser() ||
						LocalState->AuthUser.UserId != Modio::Detail::SDKSessionData::GetAuthenticatedUser()->UserId)
					{
						// If we do not have a currently authenticated user (ie this is a fresh login), then we do
						// not want to disable mod management
						if (!Modio::Detail::SDKSessionData::GetAuthenticatedUser())
						{
							bShouldDisableModManagement = false;
						}

						yield UserDataService.ClearUserDataAsync(std::move(Self), bShouldDisableModManagement);

						Modio::Detail::SDKSessionData::InitializeForUser(
							std::move(LocalState->AuthUser), Modio::Detail::OAuthToken(LocalState->AuthResponse));
					}
					else 
					{
						Modio::Detail::SDKSessionData::UpdateTokenForExistingUser(Modio::Detail::OAuthToken(LocalState->AuthResponse));
					}
					

					Modio::Detail::Services::GetGlobalService<Modio::Detail::CacheService>().ClearCache();
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
				Modio::Detail::AccessTokenObject AuthResponse;
				Modio::User AuthUser;
			};
			Modio::StableStorage<Impl> LocalState;
		};
#include <asio/unyield.hpp>
	} // namespace Detail
} // namespace Modio
