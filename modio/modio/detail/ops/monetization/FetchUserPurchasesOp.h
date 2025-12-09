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
#include "modio/detail/serialization/ModioModInfoListSerialization.h"
#include "modio/detail/ModioProfiling.h"
#include "modio/detail/ops/http/PerformRequestAndGetResponseOp.h"
#include "modio/userdata/ModioUserDataService.h"
#include <asio/coroutine.hpp>

#include <asio/yield.hpp>
namespace Modio
{
	namespace Detail
	{
		/// @brief Asynchronous operation that contacts the mod.io REST API and marshals the filtered results into
		/// public-facing data types
		class FetchUserPurchasesOp
		{
			Modio::Detail::DynamicBuffer ResponseBodyBuffer {};
			Modio::GameID GameID {};
			Modio::Detail::CachedResponse CachedResponse {};
			Modio::Optional<Modio::ModInfoList> List {};

			ModioAsio::coroutine CoroutineState {};


		public:
			template<typename CoroType>
			void operator()(CoroType& Self, Modio::ErrorCode ec = {})
			{
				GameID = Modio::Detail::SDKSessionData::CurrentGameID();

				Modio::Detail::Services::GetGlobalService<Modio::Detail::UserDataService>();


				MODIO_PROFILE_SCOPE(FetchUserPurchases);
				reenter(CoroutineState)
				{
					CachedResponse = Modio::Detail::SDKSessionData::IsPurchaseCacheInvalid()
										 ? Modio::Detail::CachedResponse::Disallow
										 : Modio::Detail::CachedResponse::Allow;

					yield Modio::Detail::PerformRequestAndGetResponseAsync(
						ResponseBodyBuffer,
						Modio::Detail::GetUserPurchasesRequest.SetGameID(GameID).AddCurrentGameIdQueryParam(),
						CachedResponse, std::move(Self));

					Modio::Detail::SDKSessionData::ClearPurchaseCacheInvalid();

					if (ec)
					{
						// Marshal all raw HTTP errors into generic RequestError
						// if (Modio::ErrorCodeMatches(ec, Modio::ErrorConditionTypes::RawHttpError)
						//{
						//	Self.complete(Modio::make_error_code(Modio::HttpError::InvalidResponse), {});
						//	return;
						//}
						Self.complete(ec);
						return;
					}

					{
						// Got the response OK, try to marshal to the expected type
						List =
							TryMarshalResponse<Modio::ModInfoList>(ResponseBodyBuffer);
						// Marshalled OK
						if (List.has_value())
						{
							for (Modio::ModInfo& Profile : List.value())
							{
								// Mod exists in the cache, so just update it with the most recent ModInfo for now
								if (Modio::Detail::SDKSessionData::GetModPurchases().count(Profile.ModId) > 0)
								{
									Modio::Detail::SDKSessionData::GetModPurchases()[Profile.ModId] = Profile;
								} else
								{
									Modio::Detail::SDKSessionData::GetModPurchases().emplace(Profile.ModId, Profile);
								} 
							}
						}
						else
						{
							// Marshalling failed
							Self.complete(Modio::make_error_code(Modio::HttpError::InvalidResponse));
						}
					}

					Services::GetGlobalService<CacheService>().AddToCache(GameID, List.value());

					Self.complete(ec);
				}
			}
		};

		#include <asio/unyield.hpp>

		template<typename FetchDoneCallback>
		auto FetchUserPurchasesAsync(FetchDoneCallback&& OnFetchComplete)
		{
			return ModioAsio::async_compose<FetchDoneCallback, void(Modio::ErrorCode)>(
				FetchUserPurchasesOp(), OnFetchComplete, Modio::Detail::Services::GetGlobalContext().get_executor());
		}
	} // namespace Detail
} // namespace Modio