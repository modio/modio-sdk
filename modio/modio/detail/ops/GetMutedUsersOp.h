/*
 *  Copyright (C) 2021-2022 mod.io Pty Ltd. <https://mod.io>
 *
 *  This file is part of the mod.io SDK.
 *
 *  Distributed under the MIT License. (See accompanying file LICENSE or
 *   view online at <https://github.com/modio/modio-sdk/blob/main/LICENSE>)
 *
 */

#pragma once

#include "modio/detail/AsioWrapper.h"
#include "modio/detail/ops/http/PerformRequestAndGetResponseOp.h"
#include "modio/detail/serialization/ModioUserListSerialization.h"
#include "modio/http/ModioHttpParams.h"

#include <asio/yield.hpp>

namespace Modio
{
	namespace Detail
	{
		class GetMutedUsersOp
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
						// time. We're going to gather all the results together at the end of this anyway so the biggest
						// pages give the best results because it means fewer REST calls
						yield Modio::Detail::PerformRequestAndGetResponseAsync(
							ResponseBodyBuffer, Modio::Detail::GetUsersMutedRequest,
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

							{
								auto PageData = TryMarshalResponse<Modio::PagedResult>(ResponseBodyBuffer);
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

							ResponseBodyBuffer = Modio::Detail::DynamicBuffer();
							CollatedResults->Append(List.value());
						}

						CurrentResultIndex += 100;
					} while (CurrentResultIndex < PageInfo.GetTotalResultCount());

					// If we had any results, initialize our page data to reflect the collated results.
					if (CollatedResults->Size() > 0)
					{
						InitializePageResult(*CollatedResults, 0, std::int32_t(CollatedResults->Size()), 1,
											 std::int32_t(CollatedResults->Size()),
											 std::int32_t(CollatedResults->Size()));
					}
					else
					{
						Modio::Detail::Logger().Log(Modio::LogLevel::Trace, Modio::LogCategory::Http,
													"User has no muted users");
					}

					Self.complete({}, std::move(*CollatedResults));
				}
			}

		private:
			Modio::Detail::DynamicBuffer ResponseBodyBuffer {};
			asio::coroutine CoroutineState {};

			// Keep track of our results
			std::int32_t CurrentResultIndex = 0;
			std::unique_ptr<Modio::UserList> CollatedResults {};
			Modio::PagedResult PageInfo {};
		};
	} // namespace Detail
} // namespace Modio
#include <asio/unyield.hpp>
