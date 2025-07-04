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
#include "modio/detail/AsioWrapper.h"
#include "modio/detail/ModioProfiling.h"
#include "modio/detail/ops/http/PerformRequestAndGetResponseOp.h"
#include "modio/detail/serialization/ModioModInfoSerialization.h"

#include <asio/yield.hpp>
namespace Modio
{
	namespace Detail
	{
		class GetModInfoOp
		{
			Modio::Detail::DynamicBuffer ResponseBodyBuffer {};
			Modio::GameID GameID {};
			Modio::ApiKey ApiKey {};
			Modio::ModID ModId {};
			Modio::Detail::CachedResponse CachedResponse {};

			asio::coroutine CoroutineState {};
			
		public:
			GetModInfoOp(Modio::GameID GameID, Modio::ApiKey ApiKey, Modio::ModID ModId)
				: GameID(GameID),
				  ApiKey(ApiKey),
				  ModId(ModId)
			{}

			template<typename CoroType>
			void operator()(CoroType& Self, Modio::ErrorCode ec = {})
			{
				MODIO_PROFILE_SCOPE(GetModInfo);

				reenter(CoroutineState)
				{
					{
						if (!Modio::Detail::SDKSessionData::IsModCacheInvalid(ModId))
						{
							Modio::Optional<Modio::ModInfo> CachedModInfo =
								Services::GetGlobalService<CacheService>().FetchFromCache(ModId);

							if (CachedModInfo.has_value() == true)
							{
								Self.complete({}, CachedModInfo);
								return;
							}
						}
					}

					CachedResponse = Modio::Detail::SDKSessionData::IsModCacheInvalid(ModId)
										 ? Modio::Detail::CachedResponse::Disallow
										 : Modio::Detail::CachedResponse::Allow;

					yield Modio::Detail::PerformRequestAndGetResponseAsync(
						ResponseBodyBuffer,
						Modio::Detail::GetModRequest.SetGameID(GameID)
							.SetModID(ModId)
							.AddPlatformStatusFilter()
							.AddStatusFilter(),
						CachedResponse, std::move(Self));

					if (ec)
					{
						Self.complete(ec, {});
						return;
					}

					{
						Modio::Detail::SDKSessionData::ClearModCacheInvalid(ModId);
						auto ModInfoData = TryMarshalResponse<Modio::ModInfo>(ResponseBodyBuffer);
						if (ModInfoData.has_value())
						{
							Services::GetGlobalService<CacheService>().AddToCache(ModInfoData.value());
							Self.complete(ec, ModInfoData);
						}
						else
						{
							Self.complete(Modio::make_error_code(Modio::HttpError::InvalidResponse), {});
						}
					}
					return;
				}
			}
		};
	} // namespace Detail
} // namespace Modio
#include <asio/unyield.hpp>