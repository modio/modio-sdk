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
#include "modio/detail/ModioConstants.h"
#include "modio/detail/ModioProfiling.h"
#include "modio/detail/ops/http/PerformRequestAndGetResponseOp.h"

#include <asio/yield.hpp>
namespace Modio
{
	namespace Detail
	{
		class GetUserWalletBalanceOp
		{
			Modio::Detail::DynamicBuffer ResponseBodyBuffer {};
			Modio::GameID GameID {};
			Modio::ApiKey ApiKey {};

			asio::coroutine CoroutineState {};

		public:
			GetUserWalletBalanceOp(Modio::GameID GameID, Modio::ApiKey ApiKey) : GameID(GameID), ApiKey(ApiKey) {}

			template<typename CoroType>
			void operator()(CoroType& Self, Modio::ErrorCode ec = {})
			{
				MODIO_PROFILE_SCOPE(GetModInfo);

				reenter(CoroutineState)
				{
					yield Modio::Detail::PerformRequestAndGetResponseAsync(
						ResponseBodyBuffer, Modio::Detail::GetUserWalletRequest.AddCurrentGameIdQueryParam(),
						Modio::Detail::CachedResponse::Disallow, std::move(Self));

					if (ec)
					{
						Self.complete(ec, {});
						return;
					}

					{
						Modio::Optional<uint64_t> UpdatedBalance = MarshalSubobjectResponse<uint64_t>(
							Modio::Detail::Constants::JSONKeys::WalletBalance, ResponseBodyBuffer);
						if (UpdatedBalance.has_value())
						{
							Self.complete({}, UpdatedBalance);
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

		template<typename GetBalanceCompleteCallback>
		void GetUserWalletBalanceAsync(GetBalanceCompleteCallback&& OnGetBalanceComplete)
		{
			return asio::async_compose<GetBalanceCompleteCallback, void(Modio::ErrorCode, Modio::Optional<uint64_t>)>(
				Modio::Detail::GetUserWalletBalanceOp(Modio::Detail::SDKSessionData::CurrentGameID(),
													  Modio::Detail::SDKSessionData::CurrentAPIKey()),
				OnGetBalanceComplete,
				Modio::Detail::Services::GetGlobalContext().get_executor());
		}
	} // namespace Detail
} // namespace Modio
#include <asio/unyield.hpp>