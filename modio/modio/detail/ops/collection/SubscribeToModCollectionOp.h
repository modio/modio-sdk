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
#include "modio/detail/ops/http/PerformRequestAndGetResponseOp.h"
#include "modio/detail/ModioConstants.h"
#include "modio/http/ModioHttpParams.h"
#include <asio/yield.hpp>

namespace Modio
{
	namespace Detail
	{
		class SubscribeToModCollectionOp
		{
		public:
			SubscribeToModCollectionOp(Modio::GameID GameID, Modio::ModCollectionID CollectionID,
									   bool IncludeDependencies)
				: ModCollectionId(CollectionID)
			{
				SubscribeParams = Modio::Detail::SubscribeToCollectionModsRequest.SetGameID(GameID)
									  .SetModCollectionID(CollectionID)
									  .AddQueryParamRaw(Modio::Detail::Constants::QueryParamStrings::IncludeDependecies,
														(IncludeDependencies ? "true" : "false"));
			}

			template<typename CoroType>
			void operator()(CoroType& Self, Modio::ErrorCode ec = {})
			{
				reenter(CoroutineState)
				{
					yield Modio::Detail::PerformRequestAndGetResponseAsync(
						ResponseBuffer, SubscribeParams, Modio::Detail::CachedResponse::Allow, std::move(Self));

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
			asio::coroutine CoroutineState;
			Modio::Detail::HttpRequestParams SubscribeParams;
			Modio::Detail::DynamicBuffer ResponseBuffer;
			Modio::ModCollectionID ModCollectionId;
		};

		template<typename CallbackType>
		auto SubscribeToModCollectionAsync(Modio::ModCollectionID ModCollectionID, CallbackType&& OnComplete)
		{
			return asio::async_compose<CallbackType, void(Modio::ErrorCode)>(
				Modio::Detail::SubscribeToModCollectionOp(Modio::Detail::SDKSessionData::CurrentGameID(),
														  ModCollectionID, false),
				OnComplete, Modio::Detail::Services::GetGlobalContext().get_executor());
		}
	} // namespace Detail
} // namespace Modio
#include <asio/unyield.hpp>
