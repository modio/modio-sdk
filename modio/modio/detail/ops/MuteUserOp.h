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

#include "modio/detail/AsioWrapper.h"
#include "modio/detail/ops/http/PerformRequestAndGetResponseOp.h"
#include "modio/http/ModioHttpParams.h"

#include <asio/yield.hpp>
namespace Modio::Detail
{
	class MuteUserOp
	{
	public:
		MuteUserOp(Modio::UserID UserID)
		{
			ID = UserID;
		}

		template<typename CoroType>
		void operator()(CoroType& Self, Modio::ErrorCode ec = {})
		{
			reenter(CoroutineState)
			{
				yield Modio::Detail::PerformRequestAndGetResponseAsync(
					ResponseBodyBuffer, Modio::Detail::MuteAUserRequest.SetUserID(ID),
					Modio::Detail::CachedResponse::Disallow, std::move(Self));

				Self.complete(ec);
				return;
			}
		}

	private:
		Modio::UserID ID {};
		Modio::Detail::DynamicBuffer ResponseBodyBuffer {};
		asio::coroutine CoroutineState {};
	};
} // namespace Modio::Detail
#include <asio/unyield.hpp>
