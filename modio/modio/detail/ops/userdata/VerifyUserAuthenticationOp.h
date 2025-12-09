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
		class VerifyUserAuthenticationOp
		{
		public:
			template<typename CoroType>
			void operator()(CoroType& Self, Modio::ErrorCode ec = {})
			{
				reenter(CoroutineState)
				{
					yield Modio::Detail::PerformRequestAndGetResponseAsync(
						ResponseBodyBuffer, GetAuthenticatedUserRequest, CachedResponse::Allow, std::move(Self));

					if (!ec)
					{
						Modio::Optional<Modio::User> User = Detail::TryMarshalResponse<Modio::User>(ResponseBodyBuffer);

						if (User.has_value())
						{
							Self.complete({});
							return;
						}
						else
						{
							Self.complete(Modio::make_error_code(Modio::HttpError::InvalidResponse));
							return;
						}
					}

					if (Modio::ErrorCodeMatches(ec, Modio::ErrorConditionTypes::NetworkError) ||
						Modio::ErrorCodeMatches(ec, Modio::ErrorConditionTypes::UserNotAuthenticatedError) ||
						Modio::ErrorCodeMatches(ec, Modio::ErrorConditionTypes::EntityNotFoundError))
					{
						Modio::Detail::SDKSessionData::InvalidateOAuthToken();
					}

					Modio::Detail::Logger().Log(Modio::LogLevel::Error, Modio::LogCategory::Http,
												"Error during VerifyUserAuthenticationOp with message: {}",
												ec.message());
					Self.complete(Modio::make_error_code(Modio::UserAuthError::StatusAuthTokenInvalid));
				}
			}

		private:
			ModioAsio::coroutine CoroutineState {};
			Modio::Detail::DynamicBuffer ResponseBodyBuffer {};
		};
#include <asio/unyield.hpp>

		template<typename VerifyUserAuthenticationCallback>
		auto VerifyUserAuthenticationAsync(VerifyUserAuthenticationCallback&& OnVerifyComplete)
		{
			return ModioAsio::async_compose<VerifyUserAuthenticationCallback, void(Modio::ErrorCode)>(
				Modio::Detail::VerifyUserAuthenticationOp(), OnVerifyComplete,
				Modio::Detail::Services::GetGlobalContext().get_executor());
		}
	} // namespace Detail
} // namespace Modio