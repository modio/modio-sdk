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

#include "modio/detail/AsioWrapper.h"
#include "modio/detail/ops/http/PerformRequestAndGetResponseOp.h"
#include "modio/http/ModioHttpParams.h"

#include <asio/yield.hpp>
namespace Modio::Detail
{
	class FollowUserOp
	{
	public:
		FollowUserOp(Modio::UserID UserID) : ID(UserID)
		{
			SubmitParams =
				Modio::Detail::FollowUserRequest.SetUserID(Modio::Detail::SDKSessionData::GetAuthenticatedUser().value().UserId);
			SubmitParams = SubmitParams.AppendPayloadValue("user_id", fmt::format("{}", ID));
		}

		template<typename CoroType>
		void operator()(CoroType& Self, Modio::ErrorCode ec = {})
		{
			MODIO_PROFILE_SCOPE(FollowUser);
			reenter(CoroutineState)
			{
				yield Modio::Detail::PerformRequestAndGetResponseAsync(
					ResponseBodyBuffer, SubmitParams,
					Modio::Detail::CachedResponse::Disallow, std::move(Self));

				Self.complete(ec);
				return;
			}
		}

	private:
		Modio::UserID ID {};
		Modio::Detail::DynamicBuffer ResponseBodyBuffer {};
		ModioAsio::coroutine CoroutineState {};
		Modio::Detail::HttpRequestParams SubmitParams {};
	};

	template<typename FollowUserCompleteCallback>
	void FollowUserAsync(Modio::UserID UserID, FollowUserCompleteCallback&& OnComplete)
	{
		return ModioAsio::async_compose<FollowUserCompleteCallback, void(Modio::ErrorCode)>(
			Modio::Detail::FollowUserOp(UserID), OnComplete,
			Modio::Detail::Services::GetGlobalContext().get_executor());
	}

} // namespace Modio::Detail
#include <asio/unyield.hpp>