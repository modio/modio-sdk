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
#include "modio/core/ModioCoreTypes.h"
#include "modio/core/ModioFilterParams.h"
#include "modio/detail/serialization/ModioModInfoListSerialization.h"
#include "modio/detail/ModioProfiling.h"
#include "modio/detail/ops/http/PerformRequestAndGetResponseOp.h"
#include <asio/coroutine.hpp>

#include <asio/yield.hpp>

namespace Modio
{
	namespace Detail
	{
		/// @brief Asynchronous operation that contacts the mod.io REST API and marshals the filtered results into
		/// public-facing data types
		class ListAllModsOp
		{
			Modio::Detail::DynamicBuffer ResponseBodyBuffer {};
			Modio::FilterParams Filter {};
			Modio::GameID GameID {};

			asio::coroutine CoroutineState {};

		public:
			ListAllModsOp(Modio::GameID GameID, FilterParams InFilter) : Filter(std::move(InFilter)), GameID(GameID) {}

			template<typename CoroType>
			void operator()(CoroType& Self, Modio::ErrorCode ec = {})
			{
				MODIO_PROFILE_SCOPE(ListAllMods);
				reenter(CoroutineState)
				{
					// In case there is no filter, it could be possible to get all cached ModInfo
					if (Filter.ToQueryParamaters().empty())
					{
						Modio::Optional<Modio::ModInfoList> CachedModInfo =
							Services::GetGlobalService<CacheService>().FetchFromCache(GameID);

						if (CachedModInfo.has_value() == true)
						{
							Self.complete({}, std::move(CachedModInfo));
							return;
						}
					}

						yield Modio::Detail::PerformRequestAndGetResponseAsync(
							ResponseBodyBuffer,
								Modio::Detail::GetModsRequest.SetGameID(GameID)
								.AddPlatformStatusFilter()
								.AddStatusFilter()
								.AppendQueryParameterMap(Filter.ToQueryParamaters()),
							Modio::Detail::CachedResponse::Allow, std::move(Self));
					
					if (ec)
					{
						// Marshal all raw HTTP errors into generic RequestError
						// if (Modio::ErrorCodeMatches(ec, Modio::ErrorConditionTypes::RawHttpError)
						//{
						//	Self.complete(Modio::make_error_code(Modio::HttpError::InvalidResponse), {});
						//	return;
						//}
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
		};

		template<typename WrappedCallback>
		auto ListAllModsAsync(FilterParams Filter, WrappedCallback&& Callback)
		{
			return asio::async_compose<WrappedCallback, void(Modio::ErrorCode, Modio::Optional<Modio::ModInfoList>)>(
				Modio::Detail::ListAllModsOp(Modio::Detail::SDKSessionData::CurrentGameID(), Filter), Callback,
				Modio::Detail::Services::GetGlobalContext().get_executor());
		}

	} // namespace Detail
} // namespace Modio
#include <asio/unyield.hpp>
