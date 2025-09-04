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
#include "modio/core/ModioCoreTypes.h"
#include "modio/detail/AsioWrapper.h"
#include "modio/detail/FmtWrapper.h"
#include "modio/detail/ops/http/PerformRequestAndGetResponseOp.h"

namespace Modio
{
	namespace Detail
	{
#include <asio/yield.hpp>
		class SubmitModRatingOp
		{
		public:
			SubmitModRatingOp(Modio::ModID Mod, Modio::Rating Rating) : Mod(Mod)
			{
				RawRating = fmt::format("{}", static_cast<std::int8_t>(Rating));
			}

			template<typename CoroType>
			void operator()(CoroType& Self, Modio::ErrorCode ec = {})
			{
				reenter(CoroutineState)
				{
					yield Modio::Detail::PerformRequestAndGetResponseAsync(
						ResponseBodyBuffer,
						Modio::Detail::AddModRatingRequest.SetGameID(Modio::Detail::SDKSessionData::CurrentGameID())
							.SetModID(Mod)
							.AppendPayloadValue(Modio::Detail::Constants::APIStrings::Rating, RawRating),
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
			Modio::ModID Mod {};
			std::string RawRating {};
			asio::coroutine CoroutineState {};
			Modio::Detail::DynamicBuffer ResponseBodyBuffer {};
		};
#include <asio/unyield.hpp>

		template<typename SubmitCompleteCallback>
		void SubmitModRatingAsync(Modio::ModID ModId, Modio::Rating Rating, SubmitCompleteCallback&& OnSubmitComplete)
		{
			return asio::async_compose<SubmitCompleteCallback, void(Modio::ErrorCode)>(
				Modio::Detail::SubmitModRatingOp(ModId, Rating), OnSubmitComplete,
				Modio::Detail::Services::GetGlobalContext().get_executor());
		}
	} // namespace Detail
} // namespace Modio