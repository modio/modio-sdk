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
#include "modio/core/entities/ModioModInfoList.h"
#include "modio/core/entities/ModioPagedResult.h"
#include "modio/detail/AsioWrapper.h"
#include "modio/detail/CoreOps.h"
#include "modio/detail/FmtWrapper.h"
#include "modio/detail/ModioJsonHelpers.h"

namespace Modio
{
	namespace Detail
	{
#include <asio/yield.hpp>
		class FetchUserSubscriptionsFromServerOp
		{
		public:
			template<typename CoroType>
			void operator()(CoroType& Self, Modio::ErrorCode ec = {})
			{
				reenter(CoroutineState)
				{
					do
					{
						// Because we're making a raw request here, manually add the filter to paginate 100 results at a
						// time We're going to gather all the results together at the end of this anyways so the biggest
						// pages give the best results because it means fewer REST calls
						yield Modio::Detail::ComposedOps::PerformRequestAndGetResponseAsync(
							SubscriptionBuffer,
							Modio::Detail::GetUserSubscriptionsRequest.SetFilterString(
								fmt::format("_limit=100&_offset={}&game_id={}", CurrentResultIndex,
											Modio::Detail::SDKSessionData::CurrentGameID())),
							Modio::Detail::CachedResponse::Allow, std::move(Self));
						if (ec)
						{
							Self.complete(ec, {});
							return;
						}
						if (CollatedResults == nullptr)
						{
							CollatedResults = std::make_unique<Modio::ModInfoList>();
						}
						// append the results to a modinfolist in stable storage
						{
							Modio::Optional<Modio::ModInfoList> CurrentModInfoPage =
								TryMarshalResponse<Modio::ModInfoList>(SubscriptionBuffer);
							if (!CurrentModInfoPage.has_value())
							{
								Self.complete(Modio::make_error_code(Modio::HttpError::InvalidResponse), {});
								return;
							}

							{
								auto PageData = TryMarshalResponse<Modio::PagedResult>(SubscriptionBuffer);
								if (PageData.has_value())
								{
									PageInfo = PageData.value();
								}
								else
								{
									Self.complete(Modio::make_error_code(Modio::HttpError::InvalidResponse), {});
									return;
								}
							}
							
							SubscriptionBuffer = Modio::Detail::DynamicBuffer();
							CollatedResults->Append(CurrentModInfoPage.value());
							Services::GetGlobalService<CacheService>().AddToCache(
								Modio::Detail::SDKSessionData::CurrentGameID(), CurrentModInfoPage.value());
						}
						CurrentResultIndex += 100;
					} while (CurrentResultIndex < PageInfo.GetTotalResultCount());

					Self.complete({}, std::move(*CollatedResults));
				}
			}

		private:
			asio::coroutine CoroutineState;
			Modio::Detail::DynamicBuffer SubscriptionBuffer;
			Modio::PagedResult PageInfo;
			std::unique_ptr<Modio::ModInfoList> CollatedResults;
			std::int32_t CurrentResultIndex = 0;
		};
#include <asio/unyield.hpp>

		template<typename FetchCompleteCallback>
		auto FetchUserSubscriptionsFromServerAsync(FetchCompleteCallback&& OnFetchComplete)
		{
			return asio::async_compose<FetchCompleteCallback,
									   void(Modio::ErrorCode, Modio::Optional<Modio::ModInfoList>)>(
				FetchUserSubscriptionsFromServerOp(), OnFetchComplete,
				Modio::Detail::Services::GetGlobalContext().get_executor());
		}

	} // namespace Detail
} // namespace Modio
