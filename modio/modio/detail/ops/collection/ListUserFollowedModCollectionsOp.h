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
#include "modio/cache/ModioCacheService.h"
#include "modio/core/entities/ModioModCollection.h"
#include "modio/detail/ModioConstants.h"
#include "modio/detail/serialization/ModioModCollectionInfoListSerialization.h"
#include "modio/http/ModioHttpParams.h"
#include <asio/yield.hpp>

namespace Modio
{
	namespace Detail
	{
		class ListUserFollowedModCollectionsOp
		{
		public:
			ListUserFollowedModCollectionsOp(Modio::FilterParams FilterParams)
			{
				ListFollowedModCollectionsParams = Modio::Detail::GetMeFollowedCollectionsRequest
											   .AddPlatformStatusFilter()
											   .AddStatusFilter()
											   .AppendQueryParameterMap(FilterParams.ToQueryParamaters());
			}

			template<typename CoroType>
			void operator()(CoroType& Self, Modio::ErrorCode ec = {})
			{
				reenter(CoroutineState)
				{
					yield Modio::Detail::PerformRequestAndGetResponseAsync(
						ResponseBodyBuffer, ListFollowedModCollectionsParams, Modio::Detail::CachedResponse::Allow,
						std::move(Self));

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
			Modio::Detail::DynamicBuffer ResponseBodyBuffer;
			Modio::Detail::HttpRequestParams ListFollowedModCollectionsParams;
			asio::coroutine CoroutineState;
		};

		template<typename CallbackType>
		auto ListUserFollowedModCollectionsAsync(Modio::FilterParams Filter, CallbackType&& OnComplete)
		{
			return asio::async_compose<CallbackType,
									   void(Modio::ErrorCode, Modio::Optional<Modio::ModCollectionInfoList>)>(
				Modio::Detail::ListUserFollowedModCollectionsOp(Filter), OnComplete,
				Modio::Detail::Services::GetGlobalContext().get_executor());
		}
	} // namespace Detail
} // namespace Modio
#include <asio/unyield.hpp>
