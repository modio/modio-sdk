/*
 *  Copyright (C) 2026 mod.io Pty Ltd. <https://mod.io>
 *
 *  This file is part of the mod.io SDK.
 *
 *  Distributed under the MIT License. (See accompanying file LICENSE or
 *   view online at <https://github.com/modio/modio-sdk/blob/main/LICENSE>)
 *
 */

#pragma once

#include "modio/detail/ops/http/PerformRequestAndGetResponseOp.h"
#include "modio/detail/ModioJsonHelpers.h"
#include "modio/core/entities/ModioUserList.h"

#include <asio/yield.hpp>

namespace Modio::Detail
{
	class GetFollowersOp
	{
	public:
		
		template<typename CoroType>
		void operator()(CoroType& Self, Modio::ErrorCode ec = {})
		{
			MODIO_PROFILE_SCOPE(GetFollowers);
			reenter(CoroutineState)
			{
				do
				{
					yield Modio::Detail::PerformRequestAndGetResponseAsync(ResponseBodyBuffer,
						Modio::Detail::MeGetFollowersRequest.AddLimitQueryParam()
						.AddOffsetQueryParam(CurrentResultIndex),
						Modio::Detail::CachedResponse::Disallow, std::move(Self));

					if (ec)
					{
						Self.complete(ec, {});
						return;
					}
					else
					{
						if (CollatedResults == nullptr)
						{
							CollatedResults = std::make_unique<Modio::UserList>();
						}

						Modio::Optional<Modio::UserList> List =
							TryMarshalResponse<Modio::UserList>(ResponseBodyBuffer);
						if (!List.has_value())
						{
							Self.complete(Modio::make_error_code(Modio::HttpError::InvalidResponse), {});
							return;
						}
						PageInfo = List.value();

						ResponseBodyBuffer = Modio::Detail::DynamicBuffer();
						CollatedResults->Append(List.value());
					}

					CurrentResultIndex += 100;
				} while (CurrentResultIndex < PageInfo.GetTotalResultCount());

				// If results exist, init page data to reflect collated results
				if (CollatedResults->Size() > 0)
				{
					InitializePageResult(*CollatedResults, 0, std::int32_t(CollatedResults->Size()), 1,
										  std::int32_t(CollatedResults->Size()), std::int32_t(CollatedResults->Size()));
				}
				else
				{
					Modio::Detail::Logger().Log(Modio::LogLevel::Trace, Modio::LogCategory::Http,
												"User has no followers.");
				}

				Self.complete({}, std::move(*CollatedResults));
			}
		}

	private:
		Modio::Detail::DynamicBuffer ResponseBodyBuffer {};
		ModioAsio::coroutine CoroutineState {};

		// Results
		std::unique_ptr<Modio::UserList> CollatedResults {};
		Modio::PagedResult PageInfo {};
		std::int32_t CurrentResultIndex = 0;
	};

	template<typename GetFollowersCompleteCallback>
	void GetFollowersAsync(GetFollowersCompleteCallback&& OnComplete)
	{
		return ModioAsio::async_compose<GetFollowersCompleteCallback, void(Modio::ErrorCode, Modio::Optional<Modio::UserList>)>(
			Modio::Detail::GetFollowersOp(), OnComplete,
			Modio::Detail::Services::GetGlobalContext().get_executor());
	}

} // namespace Modio::Detail

#include <asio/unyield.hpp>
