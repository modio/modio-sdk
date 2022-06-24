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
#include "modio/core/entities/ModioUser.h"
#include "modio/detail/AsioWrapper.h"
#include "modio/detail/ops/http/PerformRequestAndGetResponseOp.h"
#include "modio/http/ModioHttpParams.h"

namespace Modio
{
	namespace Detail
	{
#include <asio/yield.hpp>
		class VerifyUserAuthenticationOp
		{
		public:
			template<typename CoroType>
			void operator()(CoroType& Self, Modio::ErrorCode ec = {})
			{
				reenter(CoroutineState)
				{
					yield Modio::Detail::PerformRequestAndGetResponseAsync(
						ResponseBodyBuffer, GetAuthenticatedUserRequest, CachedResponse::Allow, std::move(Self));
					if (ec)
					{
						Self.complete(ec);
						return;
					}

					{
						Modio::Optional<Modio::User> User =
							Detail::TryMarshalResponse<Modio::User>(ResponseBodyBuffer);

						if (User.has_value())
						{
							Self.complete({});
							return;
						}
						else
						{
							Self.complete(Modio::make_error_code(Modio::HttpError::InvalidResponse));
							return;
						}
					}
				}
			}

		private:
			asio::coroutine CoroutineState;
			Modio::Detail::DynamicBuffer ResponseBodyBuffer;
		};
#include <asio/unyield.hpp>
	} // namespace Detail

} // namespace Modio