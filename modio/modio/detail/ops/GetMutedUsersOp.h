/*
 *  Copyright (C) 2021-2022 mod.io Pty Ltd. <https://mod.io>
 *
 *  This file is part of the mod.io SDK.
 *
 *  Distributed under the MIT License. (See accompanying file LICENSE or
 *   view online at <https://github.com/modio/modio-sdk/blob/main/LICENSE>)
 *
 */

#pragma once

#include "modio/detail/serialization/ModioUserListSerialization.h"
#include "modio/detail/AsioWrapper.h"
#include "modio/detail/ops/http/PerformRequestAndGetResponseOp.h"
#include "modio/http/ModioHttpParams.h"

#include <asio/yield.hpp>

namespace Modio
{
	namespace Detail
	{
		class GetMutedUsersOp
		{
		public:
			template<typename CoroType>
			void operator()(CoroType& Self, Modio::ErrorCode ec = {})
			{
				reenter(CoroutineState)
				{
					yield Modio::Detail::PerformRequestAndGetResponseAsync(
						ResponseBodyBuffer, Modio::Detail::GetUsersMutedRequest,
						Modio::Detail::CachedResponse::Disallow, std::move(Self));

					if (ec)
					{
						Self.complete(ec, {});
						return;
					}

					if (Modio::Optional<Modio::UserList> UserList =
							TryMarshalResponse<Modio::UserList>(ResponseBodyBuffer);
						UserList.has_value())
					{
						Self.complete(ec, std::move(UserList));
						return;
					}
					else
					{
						Self.complete(Modio::make_error_code(Modio::HttpError::InvalidResponse), {});
						return;
					}
				}
			}

		private:
			Modio::Detail::DynamicBuffer ResponseBodyBuffer;
			asio::coroutine CoroutineState;
		};
	} // namespace Detail
} // namespace Modio
#include <asio/unyield.hpp>
