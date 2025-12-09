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
#include "modio/detail/AsioWrapper.h"
#include "modio/detail/ModioJsonHelpers.h"
#include "modio/detail/ops/http/PerformRequestAndGetResponseOp.h"
#include "modio/detail/serialization/ModioModDependencySerialization.h"
#include "modio/http/ModioHttpParams.h"

namespace Modio
{
	namespace Detail
	{
#include "asio/yield.hpp"
		class GetModDependenciesOp
		{
		public:
			GetModDependenciesOp(Modio::ModID ModID, Modio::GameID GameID, bool Recursive)
				: Recursive(Recursive),
				  ModID(ModID),
				  GameID(GameID)
			{}
			template<typename CoroType>
			void operator()(CoroType& Self, Modio::ErrorCode ec = {})
			{
				reenter(CoroutineState)
				{
					{
						// Check if this mod has dependencies first
						// Return an empty set if it doesn't
						Modio::Optional<Modio::ModInfo> CachedResponse =
							Services::GetGlobalService<CacheService>().FetchFromCache(ModID);

						if (CachedResponse.has_value() && !CachedResponse.value().Dependencies)
						{
							Modio::Detail::Logger().Log(Modio::LogLevel::Info, Modio::LogCategory::Http,
														"Mod has no dependencies {}", ModID);

							Self.complete({}, {});
							return;
						}
					}

					do
					{
						// Because we're making a raw request here, manually add the filter to paginate 100 results at a
						// time. We're going to gather all the results together at the end of this anyway so the biggest
						// pages give the best results because it means fewer REST calls
						yield Modio::Detail::PerformRequestAndGetResponseAsync(
							ResponseBodyBuffer,
							Modio::Detail::GetModDependenciesRequest.SetGameID(GameID)
								.SetModID(ModID)
								.AddQueryParamRaw(Modio::Detail::Constants::QueryParamStrings::Recursive,
												  (Recursive ? "true" : "false"))
								.AddLimitQueryParam()
								.AddOffsetQueryParam(CurrentResultIndex),
							Modio::Detail::CachedResponse::Allow, std::move(Self));

						if (ec)
						{
							Self.complete(ec, {});
							return;
						}
						else
						{
							if (CollatedResults == nullptr)
							{
								CollatedResults = std::make_unique<Modio::ModDependencyList>();
							}

							Modio::Optional<Modio::ModDependencyList> List =
								TryMarshalResponse<Modio::ModDependencyList>(ResponseBodyBuffer);
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
							CollatedResults->AppendUnique(List.value());

							// Safety check if we've been receiving a large amount of dependencies from the
							// backend. This could mean something has gone wrong, or a mod has an unreasonably large
							// dependency tree.
							if (CollatedResults->Size() > MaxResultSafetyLimit)
							{
								Modio::Detail::Logger().Log(
									Modio::LogLevel::Error, Modio::LogCategory::Http,
									"Response returned an unusually large amount of dependencies for mod {}", ModID);
								Self.complete(Modio::make_error_code(Modio::HttpError::InvalidResponse), {});
								return;
							}
						}

						CurrentResultIndex += 100;
					} while (CurrentResultIndex < PageInfo.GetTotalResultCount());

					// If we had any results, initialize our page data to reflect the collated results.
					if (CollatedResults->Size() > 0)
					{
						Services::GetGlobalService<CacheService>().AddToCache(
							ModID, CollatedResults->TotalFilesizeUncompressed, Recursive);

						InitializePageResult(*CollatedResults, 0, std::int32_t(CollatedResults->Size()), 1,
											 std::int32_t(CollatedResults->Size()),
											 std::int32_t(CollatedResults->Size()));
					}
					else
					{
						Modio::Detail::Logger().Log(Modio::LogLevel::Trace, Modio::LogCategory::Http,
													"Mod {} has no dependencies", ModID);
					}

					Self.complete({}, std::move(*CollatedResults));
				}
			}

		private:
			Modio::Detail::DynamicBuffer ResponseBodyBuffer {};
			ModioAsio::coroutine CoroutineState {};
			bool Recursive {};
			const std::uint16_t MaxResultSafetyLimit = 5000;
			Modio::ModID ModID {};
			Modio::GameID GameID {};

			// Keep track of our results
			std::unique_ptr<Modio::ModDependencyList> CollatedResults {};
			Modio::PagedResult PageInfo {};
			std::int32_t CurrentResultIndex = 0;
		};
#include "asio/unyield.hpp"

		template<typename GetDependenciesCompleteCallback>
		void GetModDependenciesAsync(Modio::ModID ModID, bool Recursive,
									 GetDependenciesCompleteCallback&& OnGetDependenciesComplete)
		{
			return ModioAsio::async_compose<GetDependenciesCompleteCallback,
									   void(Modio::ErrorCode, Modio::Optional<Modio::ModDependencyList>)>(
				Modio::Detail::GetModDependenciesOp(ModID, Modio::Detail::SDKSessionData::CurrentGameID(), Recursive),
				OnGetDependenciesComplete,
				Modio::Detail::Services::GetGlobalContext().get_executor());
		}
	} // namespace Detail
} // namespace Modio