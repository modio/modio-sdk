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
#include "modio/core/entities/ModioModDetails.h"
#include "modio/detail/CoreOps.h"
#include <asio/coroutine.hpp>

#include <asio/yield.hpp>

namespace Modio
{
	namespace Detail
	{
		class GetModDetailsOp
		{
			Modio::Detail::DynamicBuffer ResponseBodyBuffer;
			Modio::GameID GameID;
			Modio::ApiKey ApiKey;
			Modio::ModID ModId;

			asio::coroutine CoroutineState;

		public:
			GetModDetailsOp(Modio::GameID GameID, Modio::ApiKey ApiKey, Modio::ModID ModId)
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
					yield Modio::Detail::ComposedOps::PerformRequestAndGetResponseAsync(
						ResponseBodyBuffer,
						Modio::Detail::GetModfilesRequest.SetGameID(GameID).SetModID(ModId),
						Modio::Detail::CachedResponse::Disallow, std::move(Self));

					if (ec)
					{
						Self.complete(ec, {});
						return;
					}

					{
						auto ModDetailsData = TryMarshalResponse<Modio::ModDetails>(ResponseBodyBuffer);
						if (ModDetailsData.has_value())
						{
							Self.complete(ec, ModDetailsData);
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