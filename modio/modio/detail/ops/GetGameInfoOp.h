/*
 *  Copyright (C) 2021-2023 mod.io Pty Ltd. <https://mod.io>
 *
 *  This file is part of the mod.io SDK.
 *
 *  Distributed under the MIT License. (See accompanying file LICENSE or
 *   view online at <https://github.com/modio/modio-sdk/blob/main/LICENSE>)
 *
 */

#pragma once

#include "modio/core/ModioCoreTypes.h"
#include "modio/detail/serialization/ModioGameInfoSerialization.h"

#include "modio/detail/AsioWrapper.h"
#include "modio/detail/ModioProfiling.h"
#include "modio/detail/ops/http/PerformRequestAndGetResponseOp.h"

#include <asio/yield.hpp>
namespace Modio
{
	namespace Detail
	{
		class GetGameInfoOp
		{
			Modio::Detail::DynamicBuffer ResponseBodyBuffer;
			Modio::GameID GameID;
			Modio::ApiKey ApiKey;

			asio::coroutine CoroutineState;

		public:
			GetGameInfoOp(Modio::GameID GameID, Modio::ApiKey ApiKey) : GameID(GameID), ApiKey(ApiKey) {}

			template<typename CoroType>
			void operator()(CoroType& Self, Modio::ErrorCode ec = {})
			{
				MODIO_PROFILE_SCOPE(GetGameInfo);
				reenter(CoroutineState)
				{
					if (Modio::Optional<Modio::GameInfo> CachedGameInfo =
							Services::GetGlobalService<CacheService>().FetchGameInfoFromCache(GameID);
						CachedGameInfo.has_value())
					{
						Self.complete({}, CachedGameInfo);
						return;
					}

					yield Modio::Detail::PerformRequestAndGetResponseAsync(
						ResponseBodyBuffer, Modio::Detail::GetGameRequest.SetGameID(GameID),
						Modio::Detail::CachedResponse::Allow, std::move(Self));

					if (ec)
					{
						Self.complete(ec, {});
						return;
					}

					if (Modio::Optional<Modio::GameInfo> GameInfoData =
							TryMarshalResponse<Modio::GameInfo>(ResponseBodyBuffer);
						GameInfoData.has_value())
					{
						//// TODO GameInfoData->TagOptions.Locale = Modio::Detail::ToString(Modio::Detail::SDKSessionData::GetLocalLanguage());
						Services::GetGlobalService<CacheService>().AddToCache(GameInfoData.value());
						Self.complete(ec, GameInfoData);
					}
					else
					{
						Self.complete(Modio::make_error_code(Modio::HttpError::InvalidResponse), {});
					}
					
					return;
				}
			}
		};
	} // namespace Detail
} // namespace Modio
#include <asio/unyield.hpp>