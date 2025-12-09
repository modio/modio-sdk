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
#include "modio/core/ModioBuffer.h"
#include "modio/core/ModioCoreTypes.h"
#include "modio/core/entities/ModioModCollection.h"
#include "modio/detail/ModioConstants.h"
#include "modio/http/ModioHttpParams.h"
#include "modio/detail/ops/http/PerformRequestAndGetResponseOp.h"
#include <asio/yield.hpp>

namespace Modio
{
	namespace Detail
	{
		class FollowModCollectionOp
		{
		public:
			FollowModCollectionOp(Modio::GameID GameID, Modio::ModCollectionID CollectionID)
			{
				FollowParams =
					Modio::Detail::FollowCollectionRequest.SetGameID(GameID)
								   .SetModCollectionID(CollectionID);
			}

			template<typename CoroType>
			void operator()(CoroType& Self, Modio::ErrorCode ec = {})
			{
				reenter(CoroutineState)
				{
					yield Modio::Detail::PerformRequestAndGetResponseAsync(
						ResponseBuffer, FollowParams, Modio::Detail::CachedResponse::Allow, std::move(Self));

					if (!ec)
					{
						Modio::Optional<Modio::ModCollectionInfo> Collection =
							TryMarshalResponse<Modio::ModCollectionInfo>(ResponseBuffer);

						if (Collection.has_value())
						{
							Services::GetGlobalService<CacheService>().AddToCache(Collection.value());
							Self.complete(ec, std::move(Collection));
						}
						else
						{
							// Marshalling failed
							Self.complete({}, {});
						}
						return;
					}

					// Treat an API error indicating a no-op as a success
					if (Modio::ErrorCodeMatches(ec, Modio::ErrorConditionTypes::ApiErrorRefSuccess))
					{
						Self.complete({}, {});
						return;
					}

					if (Modio::ErrorCodeMatches(ec, Modio::ErrorConditionTypes::UserNotAuthenticatedError))
					{
						Modio::Detail::SDKSessionData::InvalidateOAuthToken();
						Self.complete(Modio::make_error_code(Modio::UserDataError::InvalidUser), {});
						return;
					}
				}
			}

		private:
			ModioAsio::coroutine CoroutineState;
			Modio::Detail::HttpRequestParams FollowParams;
			Modio::Detail::DynamicBuffer ResponseBuffer;
		};

		template<typename CallbackType>
		auto FollowModCollectionAsync(Modio::ModCollectionID ModCollectionID, CallbackType&& OnComplete)
		{
			return ModioAsio::async_compose<CallbackType, void(Modio::ErrorCode, Modio::Optional<Modio::ModCollectionInfo>)>(
				Modio::Detail::FollowModCollectionOp(Modio::Detail::SDKSessionData::CurrentGameID(), ModCollectionID),
				OnComplete, Modio::Detail::Services::GetGlobalContext().get_executor());
		}
	} // namespace Detail
} // namespace Modio
#include <asio/unyield.hpp>
