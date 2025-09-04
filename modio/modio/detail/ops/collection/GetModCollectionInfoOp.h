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
#include "modio/cache/ModioCacheService.h"
#include "modio/detail/ModioSDKSessionData.h"
#include "modio/detail/ops/http/PerformRequestAndGetResponseOp.h"
#include "modio/detail/serialization/ModioModCollectionInfoSerialization.h"
#include <asio/yield.hpp>

namespace Modio
{
	namespace Detail
	{
		class GetModCollectionInfoOp
		{
		public:

			GetModCollectionInfoOp(Modio::GameID GameID, Modio::ModCollectionID ModCollectionId)
				: ModCollectionId(ModCollectionId)
			{
				GetCollectionRequestParams = Modio::Detail::GetModCollectionRequest.SetGameID(GameID)
							.SetModCollectionID(ModCollectionId)
							.AddPlatformStatusFilter()
							.AddStatusFilter();
			}

			template<typename CoroType>
			void operator()(CoroType& Self, Modio::ErrorCode ec = {})
			{
				reenter(CoroutineState)
				{
					{
						if (!Modio::Detail::SDKSessionData::IsModCollectionCacheInvalid(ModCollectionId))
						{
							Modio::Optional<Modio::ModCollectionInfo> CachedModCollectionInfo =
								Services::GetGlobalService<CacheService>().FetchFromCache(ModCollectionId);

							if (CachedModCollectionInfo.has_value() == true)
							{
								Self.complete({}, CachedModCollectionInfo);
								return;
							}
						}
					}

					CachedResponse = Modio::Detail::SDKSessionData::IsModCollectionCacheInvalid(ModCollectionId)
										 ? Modio::Detail::CachedResponse::Disallow
										 : Modio::Detail::CachedResponse::Allow;

					yield Modio::Detail::PerformRequestAndGetResponseAsync(
						ResponseBodyBuffer, GetCollectionRequestParams,
						CachedResponse, std::move(Self));

					if (ec)
					{
						Self.complete(ec, {});
						return;
					}

					{
						Modio::Detail::SDKSessionData::ClearModCollectionCacheInvalid(ModCollectionId);
						auto CachedModCollectionInfo = TryMarshalResponse<Modio::ModCollectionInfo>(ResponseBodyBuffer);
						if (CachedModCollectionInfo.has_value())
						{
							Services::GetGlobalService<CacheService>().AddToCache(CachedModCollectionInfo.value());
							Self.complete(ec, CachedModCollectionInfo);
						}
						else
						{
							Self.complete(Modio::make_error_code(Modio::HttpError::InvalidResponse), {});
						}
					}
					return;
				}
			}

		private:
			Modio::Detail::DynamicBuffer ResponseBodyBuffer;
			Modio::Detail::HttpRequestParams GetCollectionRequestParams;
			Modio::Detail::CachedResponse CachedResponse;
			Modio::ModCollectionID ModCollectionId;
			asio::coroutine CoroutineState;
		};

		template<typename CallbackType>
		auto GetModCollectionInfoAsync(Modio::ModCollectionID ModCollectionID, CallbackType&& OnComplete)
		{
			return asio::async_compose<CallbackType, void(Modio::ErrorCode, Modio::Optional<Modio::ModCollectionInfo>)>(
				Modio::Detail::GetModCollectionInfoOp(Modio::Detail::SDKSessionData::CurrentGameID(), ModCollectionID),
				OnComplete, Modio::Detail::Services::GetGlobalContext().get_executor());
		}
	} // namespace Detail
} // namespace Modio
#include <asio/unyield.hpp>
