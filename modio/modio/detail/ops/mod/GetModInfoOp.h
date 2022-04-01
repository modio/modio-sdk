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
#include "modio/core/entities/ModioModInfo.h"
#include "modio/detail/CoreOps.h"
#include "modio/detail/AsioWrapper.h"

#include <asio/yield.hpp>
namespace Modio
{
	namespace Detail
	{
		class GetModInfoOp
		{
			Modio::Detail::DynamicBuffer ResponseBodyBuffer;
			Modio::GameID GameID;
			Modio::ApiKey ApiKey;
			Modio::ModID ModId;

			asio::coroutine CoroutineState;

		public:
			GetModInfoOp(Modio::GameID GameID, Modio::ApiKey ApiKey, Modio::ModID ModId)
				: GameID(GameID),
				ApiKey(ApiKey),
				ModId(ModId)
			{
			}

			template<typename CoroType>
			void operator()(CoroType& Self, Modio::ErrorCode ec = {})
			{
				reenter(CoroutineState)
				{
					{
						Modio::Optional<Modio::ModInfo> CachedModInfo =
							Services::GetGlobalService<CacheService>().FetchFromCache(ModId);

						if (CachedModInfo.has_value() == true)
						{
							Self.complete({}, CachedModInfo);
							return;
						}
					}

					yield Modio::Detail::ComposedOps::PerformRequestAndGetResponseAsync(
						ResponseBodyBuffer,
						Modio::Detail::GetModRequest.SetGameID(GameID).SetModID(ModId),
						Modio::Detail::CachedResponse::Allow, std::move(Self));

					if (ec)
					{
						Self.complete(ec, {});
						return;
					}

					{
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
	}
}
#include <asio/unyield.hpp>