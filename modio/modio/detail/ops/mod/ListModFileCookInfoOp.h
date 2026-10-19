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

#include "modio/core/ModioBuffer.h"
#include "modio/core/ModioCoreTypes.h"
#include "modio/core/ModioFilterParams.h"
#include "modio/core/entities/ModioModFileCookInfoList.h"
#include "modio/detail/ModioProfiling.h"
#include "modio/detail/ops/http/PerformRequestAndGetResponseOp.h"
#include "modio/detail/serialization/ModioModFileCookInfoListSerialization.h"

#include <asio/yield.hpp>

namespace Modio
{
	namespace Detail
	{
		/// @brief Asynchronous operation that contacts the mod.io REST API and marshals the filtered results into
		/// public-facing data types
		class ListModFileCookInfoOp
		{
			Modio::Detail::DynamicBuffer ResponseBodyBuffer {};
			Modio::GameID GameID {};
			Modio::ModID ModID {};
			Modio::FilterParams Filter {};

			ModioAsio::coroutine CoroutineState {};

		public:
			ListModFileCookInfoOp(Modio::GameID GameID, Modio::ModID ModID, FilterParams InFilter)
				: GameID(GameID),
				  ModID(ModID),
				  Filter(std::move(InFilter))
			{}

			template<typename CoroType>
			void operator()(CoroType& Self, Modio::ErrorCode ec = {})
			{
				MODIO_PROFILE_SCOPE(ListModFileCookInfo);
				reenter(CoroutineState)
				{
					// In case there is no filter, it could be possible to get all cached ModInfo
					if (Filter.ToQueryParameters().empty())
					{
						Modio::Optional<Modio::ModFileCookInfoList> CachedModFileCookInfoList =
							Services::GetGlobalService<CacheService>().FetchModFileCookListFromCache(ModID);

						if (CachedModFileCookInfoList.has_value() == true)
						{
							Self.complete({}, std::move(CachedModFileCookInfoList));
							return;
						}
					}

					yield Modio::Detail::PerformRequestAndGetResponseAsync(
						ResponseBodyBuffer,
						Modio::Detail::BrowseModfileCooksRequest.SetGameID(GameID).SetModID(ModID)
							.AppendQueryParameterMap(Filter.ToQueryParameters()),
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
						Modio::Optional<Modio::ModFileCookInfoList> List =
							TryMarshalResponse<Modio::ModFileCookInfoList>(ResponseBodyBuffer);
						// Marshalled OK
						if (List.has_value())
						{
							Services::GetGlobalService<CacheService>().AddToCache(ModID, List.value());
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
		auto ListModFileCookInfoAsync(Modio::ModID ModID, FilterParams Filter, WrappedCallback&& Callback)
		{
			return ModioAsio::async_compose<WrappedCallback,
											void(Modio::ErrorCode, Modio::Optional<Modio::ModFileCookInfoList>)>(
				Modio::Detail::ListModFileCookInfoOp(Modio::Detail::SDKSessionData::CurrentGameID(), ModID, Filter), Callback,
				Modio::Detail::Services::GetGlobalContext().get_executor());
		}

	} // namespace Detail
} // namespace Modio
#include <asio/unyield.hpp>
