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
		class GetModCollectionModsOp
		{
		public:

			GetModCollectionModsOp(Modio::GameID GameID, Modio::ModCollectionID ModCollectionId)
			{
				GetModCollectionModsParams = Modio::Detail::GetCollectionModsRequest.SetGameID(GameID)
							.SetModCollectionID(ModCollectionId)
							.AddPlatformStatusFilter()
							.AddStatusFilter();
			}

			template<typename CoroType>
			void operator()(CoroType& Self, Modio::ErrorCode ec = {})
			{
				reenter(CoroutineState)
				{
					yield Modio::Detail::PerformRequestAndGetResponseAsync(ResponseBodyBuffer, GetModCollectionModsParams,
																		   Modio::Detail::CachedResponse::Allow,
																		   std::move(Self));

					if (ec)
					{
						Self.complete(ec, {});
						return;
					}

					{
						// Got the response OK, try to marshal to the expected type
						Modio::Optional<Modio::ModInfoList> List =
							TryMarshalResponse<Modio::ModInfoList>(ResponseBodyBuffer);
						// Marshalled OK
						if (List.has_value())
						{
							//Services::GetGlobalService<CacheService>().AddToCache(GameID, List.value());
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
			Modio::Detail::HttpRequestParams GetModCollectionModsParams;
			ModioAsio::coroutine CoroutineState;
		};

		template<typename CallbackType>
		auto GetModCollectionModsAsync(Modio::ModCollectionID ModCollectionID, CallbackType&& OnComplete)
		{
			return ModioAsio::async_compose<CallbackType, void(Modio::ErrorCode, Modio::Optional<Modio::ModInfoList>)>(
				Modio::Detail::GetModCollectionModsOp(Modio::Detail::SDKSessionData::CurrentGameID(), ModCollectionID),
				OnComplete, Modio::Detail::Services::GetGlobalContext().get_executor());
		}
	} // namespace Detail
} // namespace Modio
#include <asio/unyield.hpp>
