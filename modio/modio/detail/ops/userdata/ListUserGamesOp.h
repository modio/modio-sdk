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
#include "modio/detail/serialization/ModioGameInfoListSerialization.h"
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
		class ListUserGamesOp
		{
			Modio::FilterParams Filter {};
			Modio::Detail::DynamicBuffer ResponseBodyBuffer {};

			asio::coroutine CoroutineState {};

		public:
			ListUserGamesOp(FilterParams InFilter) : Filter(std::move(InFilter)) {}

			template<typename CoroType>
			void operator()(CoroType& Self, Modio::ErrorCode ec = {})
			{
				MODIO_PROFILE_SCOPE(ListAllGamess);
				reenter(CoroutineState)
				{
					yield Modio::Detail::PerformRequestAndGetResponseAsync(
						ResponseBodyBuffer,
						Modio::Detail::GetUserGamesRequest.AppendQueryParameterMap(
							Filter.ToQueryParamaters()),
						Modio::Detail::CachedResponse::Allow, std::move(Self));

					if (ec)
					{
						// If the server says we're not authenticated (and we think we are) then unauth locally.
						if (Modio::ErrorCodeMatches(ec, Modio::ErrorConditionTypes::UserNotAuthenticatedError))
						{
							Modio::Detail::SDKSessionData::InvalidateOAuthToken();
						}

						Self.complete(ec, {});
						return;
					}

					{
						// Got the response OK, try to marshal to the expected type
						Modio::Optional<Modio::GameInfoList> List =
							TryMarshalResponse<Modio::GameInfoList>(ResponseBodyBuffer);
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
		};
	} // namespace Detail
} // namespace Modio
