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

#include "modio/core/ModioBuffer.h"
#include "modio/core/entities/ModioUser.h"
#include "modio/detail/AsioWrapper.h"
#include "modio/detail/ops/http/PerformRequestAndGetResponseOp.h"
#include "modio/http/ModioHttpParams.h"

namespace Modio
{
	namespace Detail
	{
#include <asio/yield.hpp>
		class RefreshUserDataOp
		{
		public:
			template<typename CoroType>
			void operator()(CoroType& Self, Modio::ErrorCode ec = {})
			{
				reenter(CoroutineState)
				{
					if (Modio::Detail::SDKSessionData::GetAuthenticationToken().has_value())
					{
						yield Modio::Detail::PerformRequestAndGetResponseAsync(
							Local.ResponseBodyBuffer,
							Modio::Detail::GetAuthenticatedUserRequest.SetAuthTokenOverride(
								Modio::Detail::SDKSessionData::GetAuthenticationToken().and_then(
									&Modio::Detail::OAuthToken::GetToken).value()),
							Modio::Detail::CachedResponse::Disallow, std::move(Self));
					}
					else
					{
						Self.complete(Modio::make_error_code(Modio::UserDataError::InvalidUser));
						return;
					}

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
							Local.AuthenticatedUser = User.value();
						}
						else
						{
							Self.complete(Modio::make_error_code(Modio::HttpError::InvalidResponse));
							return;
						}
					}

					Local.ResponseBodyBuffer.Clear();

					Modio::Detail::SDKSessionData::InitializeForUser(std::move(Local.AuthenticatedUser),
						Modio::Detail::SDKSessionData::GetAuthenticationToken().value());

					Detail::Services::GetGlobalService<Detail::CacheService>().ClearCache();
					yield Modio::Detail::Services::GetGlobalService<Modio::Detail::UserDataService>()
						.SaveUserDataToStorageAsync(std::move(Self));

					Self.complete({});
					return;
				}
			}

		private:
			ModioAsio::coroutine CoroutineState {};
			struct
			{
				Modio::Detail::DynamicBuffer ResponseBodyBuffer {};
				Modio::User AuthenticatedUser {};
			} Local;
		};
#include <asio/unyield.hpp>

		template<typename RefreshUserDataCallback>
		auto RefreshUserDataAsync(RefreshUserDataCallback&& OnRefreshComplete)
		{
			return ModioAsio::async_compose<RefreshUserDataCallback, void(Modio::ErrorCode)>(
				Modio::Detail::RefreshUserDataOp(), OnRefreshComplete,
				Modio::Detail::Services::GetGlobalContext().get_executor());
		}
	} // namespace Detail
} // namespace Modio