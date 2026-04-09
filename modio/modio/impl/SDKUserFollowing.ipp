/*
 *  Copyright (C) 2026 mod.io Pty Ltd. <https://mod.io>
 *
 *  This file is part of the mod.io SDK.
 *
 *  Distributed under the MIT License. (See accompanying file LICENSE or
 *   view online at <https://github.com/modio/modio-sdk/blob/main/LICENSE>)
 *
 */

#define MODIO_SDK_PROTOTYPES_ONLY
#include "modio/ModioSDK.h"

#include "modio/detail/serialization/ModioUserSerialization.h"
#include "modio/detail/ops/userfollowing/GetFollowersOp.h"
#include "modio/detail/ops/userfollowing/GetUserFollowingOp.h"
#include "modio/detail/ops/userfollowing/GetUserFollowersOp.h"
#include "modio/detail/ops/userfollowing/FollowUserOp.h"
#include "modio/detail/ops/userfollowing/UnfollowUserOp.h"
#include "modio/impl/SDKPreconditionChecks.h"

namespace Modio
{
	void GetFollowersAsync(
		std::function<void(Modio::ErrorCode, Modio::Optional<Modio::UserList>)> Callback)
	{
		Modio::Detail::SDKSessionData::EnqueueTask([Callback = std::move(Callback)]() mutable {
			if (Modio::Detail::RequireSDKIsInitialized(Callback) && Modio::Detail::RequireNotRateLimited(Callback) &&
				Modio::Detail::RequireUserIsAuthenticated(Callback))
			{
				Modio::Detail::GetFollowersAsync(Callback);
			}
		});
	}

	void GetUserFollowersAsync(
		Modio::UserID UserID, std::function<void(Modio::ErrorCode, Modio::Optional<Modio::UserList>)> Callback)
	{
		Modio::Detail::SDKSessionData::EnqueueTask([UserID, Callback = std::move(Callback)]() mutable {
			if (Modio::Detail::RequireSDKIsInitialized(Callback) && Modio::Detail::RequireNotRateLimited(Callback) &&
				Modio::Detail::RequireUserIsAuthenticated(Callback) &&
				Modio::Detail::RequireValidUserID(UserID, Callback))
			{
				Modio::Detail::GetUserFollowersAsync(UserID, Callback);
			}
		});
	}

	void GetUserFollowingAsync(
		Modio::UserID UserID, std::function<void(Modio::ErrorCode, Modio::Optional<Modio::UserList>)> Callback)
	{
		Modio::Detail::SDKSessionData::EnqueueTask([UserID, Callback = std::move(Callback)]() mutable {
			if (Modio::Detail::RequireSDKIsInitialized(Callback) && Modio::Detail::RequireNotRateLimited(Callback) &&
				Modio::Detail::RequireUserIsAuthenticated(Callback) &&
				Modio::Detail::RequireValidUserID(UserID, Callback))
			{
				Modio::Detail::GetUserFollowingAsync(UserID, Callback);
			}
		});
	}

	void FollowUserAsync(Modio::UserID UserID, std::function<void(Modio::ErrorCode)> Callback)
	{
		Modio::Detail::SDKSessionData::EnqueueTask([UserID, Callback = std::move(Callback)]() mutable {
			if (Modio::Detail::RequireSDKIsInitialized(Callback) && Modio::Detail::RequireNotRateLimited(Callback) &&
				Modio::Detail::RequireUserIsAuthenticated(Callback) &&
				Modio::Detail::RequireValidUserID(UserID, Callback))
			{
				Modio::Detail::FollowUserAsync(UserID, Callback);
			}
		});
	}

	void UnfollowUserAsync(Modio::UserID UserID, std::function<void(Modio::ErrorCode)> Callback)
	{
		Modio::Detail::SDKSessionData::EnqueueTask([UserID, Callback = std::move(Callback)]() mutable {
			if (Modio::Detail::RequireSDKIsInitialized(Callback) && Modio::Detail::RequireNotRateLimited(Callback) &&
				Modio::Detail::RequireUserIsAuthenticated(Callback) &&
				Modio::Detail::RequireValidUserID(UserID, Callback))
			{
				Modio::Detail::UnfollowUserAsync(UserID, Callback);
			}
		});
	}

} // namespace Modio