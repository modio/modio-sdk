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
#include "modio/detail/serialization/ModioUserDelegationTokenSerialization.h"

#include <asio/yield.hpp>

namespace Modio
{
	namespace Detail
	{
		class GetUserDelegationTokenOp
		{
			Modio::Detail::DynamicBuffer ResponseBodyBuffer;
			Modio::GameID GameID;
			Modio::ApiKey ApiKey;

			asio::coroutine CoroutineState;

		public:
			GetUserDelegationTokenOp(Modio::GameID GameID, Modio::ApiKey ApiKey) : GameID(GameID), ApiKey(ApiKey) {}

			template<typename CoroType>
			void operator()(CoroType& Self, Modio::ErrorCode ec = {})
			{
				MODIO_PROFILE_SCOPE(GetModInfo);

				reenter(CoroutineState)
				{
					yield Modio::Detail::PerformRequestAndGetResponseAsync(
						ResponseBodyBuffer, Modio::Detail::RequestUserDelegationTokenRequest,
						Modio::Detail::CachedResponse::Disallow, std::move(Self));

					if (ec)
					{
						Self.complete(ec, {});
						return;
					}

					{
						Modio::Optional<Modio::UserDelegationToken> UDT =
							TryMarshalResponse<Modio::UserDelegationToken>(ResponseBodyBuffer);

						if (UDT.has_value())
						{
							Self.complete({}, UDT.value().Token);
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