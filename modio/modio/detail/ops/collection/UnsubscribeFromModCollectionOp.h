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
#include "modio/core/entities/ModioModCollection.h"
#include "modio/detail/ModioConstants.h"
#include "modio/http/ModioHttpParams.h"
#include <asio/yield.hpp>

namespace Modio
{
	namespace Detail
	{
		class UnsubscribeFromModCollectionOp
		{
		public:
			UnsubscribeFromModCollectionOp(Modio::GameID GameID, Modio::ModCollectionID CollectionID)
			{
				UnsubscribeParams =
					Modio::Detail::UnsubscribeFromCollectionModsRequest.SetGameID(GameID)
								   .SetModCollectionID(CollectionID);
			}

			template<typename CoroType>
			void operator()(CoroType& Self, Modio::ErrorCode  ec = {})
			{
				reenter(CoroutineState)
				{
					yield Modio::Detail::PerformRequestAndGetResponseAsync(ResponseBuffer, UnsubscribeParams,
																		   Modio::Detail::CachedResponse::Allow, std::move(Self));
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
				}
			}

		private:
			ModioAsio::coroutine CoroutineState;
			Modio::Detail::HttpRequestParams UnsubscribeParams;
			Modio::Detail::DynamicBuffer ResponseBuffer;
		};

		template<typename CallbackType>
		auto UnsubscribeFromModCollectionAsync(Modio::ModCollectionID ModCollectionID, CallbackType&& OnComplete)
		{
			return ModioAsio::async_compose<CallbackType, void(Modio::ErrorCode)>(
				Modio::Detail::UnsubscribeFromModCollectionOp(Modio::Detail::SDKSessionData::CurrentGameID(),
															  ModCollectionID),
				OnComplete, Modio::Detail::Services::GetGlobalContext().get_executor());
		}
	} // namespace Detail
} // namespace Modio
#include <asio/unyield.hpp>
