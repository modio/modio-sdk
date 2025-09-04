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
#include "modio/detail/ops/http/PerformRequestAndGetResponseOp.h"
#include "modio/detail/serialization/ModioModDetailsSerialization.h"
#include <asio/coroutine.hpp>

#include <asio/yield.hpp>

namespace Modio
{
	namespace Detail
	{
		class GetModDetailsOp
		{
			Modio::Detail::DynamicBuffer ResponseBodyBuffer {};
			Modio::GameID GameID {};
			Modio::ApiKey ApiKey {};
			Modio::ModID ModId {};

			asio::coroutine CoroutineState {};

		public:
			GetModDetailsOp(Modio::GameID GameID, Modio::ApiKey ApiKey, Modio::ModID ModId)
				: GameID(GameID),
				  ApiKey(ApiKey),
				  ModId(ModId)
			{}

			template<typename CoroType>
			void operator()(CoroType& Self, Modio::ErrorCode ec = {})
			{
				reenter(CoroutineState)
				{
					yield Modio::Detail::PerformRequestAndGetResponseAsync(
						ResponseBodyBuffer,
						Modio::Detail::GetModfilesRequest.SetGameID(GameID)
							.SetModID(ModId)
							.AddPlatformStatusFilter()
							.AddStatusFilter(),
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

		template<typename GetModDetailsCompleteCallback>
		void GetModDetailsAsync(Modio::ModID ModId, GetModDetailsCompleteCallback&& OnGetModDetailsComplete)
		{
			return asio::async_compose<GetModDetailsCompleteCallback,
									   void(Modio::ErrorCode, Modio::Optional<Modio::ModDetails>)>(
				Modio::Detail::GetModDetailsOp(Modio::Detail::SDKSessionData::CurrentGameID(),
											   Modio::Detail::SDKSessionData::CurrentAPIKey(), ModId),
				OnGetModDetailsComplete,
				Modio::Detail::Services::GetGlobalContext().get_executor());
		}
	} // namespace Detail
} // namespace Modio