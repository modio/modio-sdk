/*
 *  Copyright (C) 2021-2025 mod.io Pty Ltd. <https://mod.io>
 *
 *  This file is part of the mod.io SDK.
 *
 *  Distributed under the MIT License. (See accompanying file LICENSE or
 *   view online at <https://github.com/modio/modio-sdk/blob/main/LICENSE>)
 *
 */

#pragma once
#include "modio/core/ModioCoreTypes.h"
#include "modio/detail/AsioWrapper.h"
#include "modio/detail/FmtWrapper.h"
#include "modio/detail/ops/http/PerformRequestAndGetResponseOp.h"
#include "modio/detail/ModioConstants.h"
#include "modio/http/ModioHttpParams.h"
#include <asio/yield.hpp>

namespace Modio
{
	namespace Detail
	{
		class SubmitModCollectionRatingOp
		{
		public:
			SubmitModCollectionRatingOp(Modio::ModCollectionID Collection, Modio::Rating Rating)
			{
				RatingParams = Modio::Detail::AddCollectionRatingRequest
					.SetGameID(Modio::Detail::SDKSessionData::CurrentGameID())
					.SetModCollectionID(Collection)
					.AppendPayloadValue(Modio::Detail::Constants::APIStrings::Rating,
									    fmt::format("{}", static_cast<std::int8_t>(Rating)));
			}

			template<typename CoroType>
			void operator()(CoroType& Self, Modio::ErrorCode ec = {})
			{
				reenter(CoroutineState)
				{
					yield Modio::Detail::PerformRequestAndGetResponseAsync(
						ResponseBodyBuffer, RatingParams,
						Modio::Detail::CachedResponse::Disallow, std::move(Self));

					if (!ec)
					{
						Self.complete({});
						return;
					}

					// Treat an API error indicating a no-op as a success
					if (Modio::ErrorCodeMatches(ec, Modio::ErrorConditionTypes::ApiErrorRefSuccess))
					{
						Self.complete({});
						return;
					}

					if (Modio::ErrorCodeMatches(ec, Modio::ErrorConditionTypes::UserNotAuthenticatedError))
					{
						Modio::Detail::SDKSessionData::InvalidateOAuthToken();
						Self.complete(Modio::make_error_code(Modio::UserDataError::InvalidUser));
						return;
					}

					Modio::Detail::Logger().Log(Modio::LogLevel::Error, Modio::LogCategory::Http, "Error message: {}",
												ec.message());

					// Check if the error is supported by the SDK
					if (ec.category() != std::system_category())
					{
						Self.complete(ec);
						return;
					}

					Self.complete(Modio::make_error_code(Modio::HttpError::InvalidResponse));
					return;
				}
			}

		private:
			HttpRequestParams RatingParams;
			ModioAsio::coroutine CoroutineState;
			Modio::Detail::DynamicBuffer ResponseBodyBuffer;
		};

		template<typename CallbackType>
		auto SubmitModCollectionRatingAsync(Modio::ModCollectionID ModCollectionID, Modio::Rating Rating, CallbackType&& OnComplete)
		{
			return ModioAsio::async_compose<CallbackType, void(Modio::ErrorCode)>(
				Modio::Detail::SubmitModCollectionRatingOp(ModCollectionID, Rating),
				OnComplete, Modio::Detail::Services::GetGlobalContext().get_executor());
		}
	} // namespace Detail
} // namespace Modio

#include <asio/unyield.hpp>