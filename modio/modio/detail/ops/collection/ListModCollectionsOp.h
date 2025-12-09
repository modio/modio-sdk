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
#include "modio/detail/serialization/ModioModCollectionInfoListSerialization.h"
#include "modio/detail/ModioConstants.h"
#include "modio/http/ModioHttpParams.h"
#include "modio/cache/ModioCacheService.h"
#include <asio/yield.hpp>

namespace Modio
{
	namespace Detail
	{
		class ListModCollectionsOp
		{
		public:

			ListModCollectionsOp(Modio::GameID GameID, Modio::FilterParams FilterParams)
				: GameID(GameID),
				  FilterParams(FilterParams)
			{
				std::map<std::string, std::string> QueryParams = FilterParams.ToQueryParamaters();
				QueryParams.insert({"mods_total-min", "1"});
				ListModCollectionsParams = Modio::Detail::GetModCollectionsRequest.SetGameID(GameID)
											   .AddPlatformStatusFilter()
											   .AddStatusFilter()
											   .AppendQueryParameterMap(QueryParams);
			}

			template<typename CoroType>
			void operator()(CoroType& Self, Modio::ErrorCode ec = {})
			{
				reenter(CoroutineState)
				{
					yield Modio::Detail::PerformRequestAndGetResponseAsync(
						ResponseBodyBuffer, ListModCollectionsParams,
						Modio::Detail::CachedResponse::Allow, std::move(Self));

					if (ec)
					{
						Self.complete(ec, {});
						return;
					}

					{
						// Got the response OK, try to marshal to the expected type
						Modio::Optional<Modio::ModCollectionInfoList> List =
							TryMarshalResponse<Modio::ModCollectionInfoList>(ResponseBodyBuffer);
						// Marshalled OK
						if (List.has_value())
						{
							Services::GetGlobalService<CacheService>().AddToCache(GameID, List.value());
							Self.complete(ec, std::move(List));
						}
						else
						{
							// Marshalling failed
							Self.complete(Modio::make_error_code(Modio::HttpError::InvalidResponse), {});
						}
						return;
					}
				}
			}

		private:
			Modio::GameID GameID;
			Modio::Detail::DynamicBuffer ResponseBodyBuffer;
			Modio::Detail::HttpRequestParams ListModCollectionsParams;
			ModioAsio::coroutine CoroutineState;
			Modio::FilterParams FilterParams;
		};

		template<typename CallbackType>
		auto ListModCollectionsAsync(Modio::FilterParams Filter, CallbackType&& OnComplete)
		{
			return ModioAsio::async_compose<CallbackType, void(Modio::ErrorCode, Modio::Optional<Modio::ModCollectionInfoList>)>(
				Modio::Detail::ListModCollectionsOp(Modio::Detail::SDKSessionData::CurrentGameID(), Filter),
				OnComplete, Modio::Detail::Services::GetGlobalContext().get_executor());
		}
	} // namespace Detail
} // namespace Modio
#include <asio/unyield.hpp>
