/*
 *  Copyright (C) 2026 mod.io Pty Ltd. <https://mod.io>
 *
 *  This file is part of the mod.io SDK.
 *
 *  Distributed under the MIT License. (See accompanying file LICENSE or
 *   view online at <https://github.com/modio/modio-sdk/blob/main/LICENSE>)
 *
 */

#pragma once

#include "modio/detail/ops/http/PerformRequestAndGetResponseOp.h"

#include <asio/yield.hpp>
namespace Modio::Detail
{
	class UnfollowUserOp
	{
	public:
		UnfollowUserOp(Modio::UserID UserID) : ID(UserID){}

		template<typename CoroType>
		void operator()(CoroType& Self, Modio::ErrorCode ec = {})
		{
			MODIO_PROFILE_SCOPE(UnfollowUser);
			reenter(CoroutineState)
			{
				yield Modio::Detail::PerformRequestAndGetResponseAsync(
					ResponseBodyBuffer,
					Modio::Detail::UnfollowUserRequest
						.SetUserID(Modio::Detail::SDKSessionData::GetAuthenticatedUser().value().UserId)
						.SetTargetUserID(ID),
					Modio::Detail::CachedResponse::Disallow, std::move(Self));
				// Treat an API error indicating a no-op as a success
				if (Modio::ErrorCodeMatches(ec, Modio::ErrorConditionTypes::ApiErrorRefSuccess))
				{
					Self.complete({});
					return;
				}
				Self.complete(ec);
				return;
			}
		}

	private:
		Modio::UserID ID {};
		Modio::Detail::DynamicBuffer ResponseBodyBuffer {};
		ModioAsio::coroutine CoroutineState {};
	};

	template<typename UnfollowUserCompleteCallback>
	void UnfollowUserAsync(Modio::UserID UserID, UnfollowUserCompleteCallback&& OnComplete)
	{
		return ModioAsio::async_compose<UnfollowUserCompleteCallback, void(Modio::ErrorCode)>(
			Modio::Detail::UnfollowUserOp(UserID), OnComplete,
			Modio::Detail::Services::GetGlobalContext().get_executor());
	}

} // namespace Modio::Detail
#include <asio/unyield.hpp>