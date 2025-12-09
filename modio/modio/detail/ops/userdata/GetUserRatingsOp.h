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

#include "modio/core/ModioCoreTypes.h"
#include "modio/detail/serialization/ModioUserRatingListSerialization.h"
#include <asio/yield.hpp>

namespace Modio
{
	namespace Detail
	{
		/// @docinternal
		/// @brief Async operation to retrieve a user media metadata
		class GetUserRatingsOp
		{

			ModioAsio::coroutine CoroutineState {};
			Modio::Detail::DynamicBuffer ResponseBodyBuffer {};

		public:
			GetUserRatingsOp() {}

			template<typename CoroType>
			void operator()(CoroType& Self, Modio::ErrorCode ec = {})
			{
				reenter(CoroutineState) 
				{
					yield Modio::Detail::PerformRequestAndGetResponseAsync(
						ResponseBodyBuffer,
						Modio::Detail::GetUserRatingsRequest,
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
						Modio::Optional<Modio::UserRatingList> List = TryMarshalResponse<Modio::UserRatingList>(ResponseBodyBuffer);
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
			

		};

		template<typename GetUserRatingsCompleteCallback>
		void GetUserRatingsAsync(GetUserRatingsCompleteCallback&& OnGetUserRatingsComplete)
		{
			return ModioAsio::async_compose<GetUserRatingsCompleteCallback,
									   void(Modio::ErrorCode, Modio::Optional<Modio::UserRatingList>)>(
				Modio::Detail::GetUserRatingsOp(), OnGetUserRatingsComplete,
				Modio::Detail::Services::GetGlobalContext().get_executor());
		}
	} // namespace Detail
} // namespace Modio

#include <asio/unyield.hpp>