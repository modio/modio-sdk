/*
 *  Copyright (C) 2025 mod.io Pty Ltd. <https://mod.io>
 *
 *  This file is part of the mod.io SDK.
 *
 *  Distributed under the MIT License. (See accompanying file LICENSE or
 *   view online at <https://github.com/modio/modio-sdk/blob/main/LICENSE>)
 *
 */

#ifdef MODIO_SEPARATE_COMPILATION
	#include "modio/ModioSDK.h"
#else
	#pragma once
#endif

#include "modio/cache/ModioCacheService.h"
#include "modio/impl/SDKPreconditionChecks.h"
#include "modio/detail/ops/collection/FollowModCollectionOp.h"
#include "modio/detail/ops/collection/GetModCollectionInfoOp.h"
#include "modio/detail/ops/collection/GetModCollectionModsOp.h"
#include "modio/detail/ops/collection/ListModCollectionsOp.h"
#include "modio/detail/ops/collection/ListUserFollowedModCollectionsOp.h"
#include "modio/detail/ops/collection/SubmitModCollectionRatingOp.h"
#include "modio/detail/ops/collection/SubscribeToModCollectionOp.h"
#include "modio/detail/ops/collection/UnfollowModCollectionOp.h"
#include "modio/detail/ops/collection/GetModCollectionMediaLogoOp.h"
#include "modio/detail/ops/collection/GetModCollectionMediaAvatarOp.h"
#include "modio/detail/ops/collection/UnsubscribeFromModCollectionOp.h"
#include "modio/detail/serialization/ModioModInfoSerialization.h"
#include "modio/detail/serialization/ModioModInfoListSerialization.h"


namespace Modio
{
	void ListModCollectionsAsync(
		Modio::FilterParams Filter,
		std::function<void(Modio::ErrorCode, Modio::Optional<Modio::ModCollectionInfoList>)> Callback)
	{
		Modio::Detail::SDKSessionData::EnqueueTask(
			[Filter = std::move(Filter), Callback = std::move(Callback)]() mutable {
				if (Modio::Detail::RequireSDKIsInitialized(Callback) && 
					Modio::Detail::RequireNotRateLimited(Callback))
				{
					Modio::Detail::ListModCollectionsAsync(Filter, Callback);
				}
			});
	}

	void ListUserFollowedModCollectionsAsync(
		Modio::FilterParams Filter,
		std::function<void(Modio::ErrorCode, Modio::Optional<Modio::ModCollectionInfoList>)> Callback)
	{
		Modio::Detail::SDKSessionData::EnqueueTask(
			[Filter = std::move(Filter), Callback = std::move(Callback)]() mutable {
				if (Modio::Detail::RequireSDKIsInitialized(Callback) &&
					Modio::Detail::RequireUserIsAuthenticated(Callback) && 
					Modio::Detail::RequireNotRateLimited(Callback))
				{
					Modio::Detail::ListUserFollowedModCollectionsAsync(Filter, Callback);
				}
			});
	}

	void GetModCollectionInfoAsync(
		Modio::ModCollectionID ModCollectionID,
		std::function<void(Modio::ErrorCode, Modio::Optional<Modio::ModCollectionInfo>)> Callback)
	{
		Modio::Detail::SDKSessionData::EnqueueTask([ModCollectionID, Callback = std::move(Callback)]() mutable {
			if (Modio::Detail::RequireSDKIsInitialized(Callback) &&
				Modio::Detail::RequireUserIsAuthenticated(Callback) &&
				Modio::Detail::RequireValidModCollectionID(ModCollectionID, Callback))
			{
				Modio::Detail::GetModCollectionInfoAsync(ModCollectionID, Callback);
			}
		});
	}

	void GetModCollectionModsAsync(
		Modio::ModCollectionID ModCollectionID,
		std::function<void(Modio::ErrorCode, Modio::Optional<Modio::ModInfoList>)> Callback)
	{
		Modio::Detail::SDKSessionData::EnqueueTask([ModCollectionID, Callback = std::move(Callback)]() mutable {
			if (Modio::Detail::RequireSDKIsInitialized(Callback) &&
				Modio::Detail::RequireUserIsAuthenticated(Callback) &&
				Modio::Detail::RequireValidModCollectionID(ModCollectionID, Callback))
			{
				Modio::Detail::GetModCollectionModsAsync(ModCollectionID, Callback);
			}
		});
	}

	void SubmitModCollectionRatingAsync(Modio::ModCollectionID ModCollectionID, Modio::Rating Rating,
													 std::function<void(Modio::ErrorCode)> Callback)
	{
		Modio::Detail::SDKSessionData::EnqueueTask([ModCollectionID, Callback = std::move(Callback), Rating]() mutable {
			if (Modio::Detail::RequireSDKIsInitialized(Callback) &&
				Modio::Detail::RequireUserIsAuthenticated(Callback) &&
				Modio::Detail::RequireValidModCollectionID(ModCollectionID, Callback))
			{
				Modio::Detail::SubmitModCollectionRatingAsync(ModCollectionID, Rating, Callback);
			}
		});
	}

	void SubscribeToModCollectionAsync(Modio::ModCollectionID ModCollectionToSubscribeTo,
													std::function<void(Modio::ErrorCode)> Callback)
	{
		Modio::Detail::SDKSessionData::EnqueueTask(
			[ModCollectionToSubscribeTo, Callback = std::move(Callback)]() mutable {
				if (Modio::Detail::RequireSDKIsInitialized(Callback) &&
					Modio::Detail::RequireUserIsAuthenticated(Callback) &&
					Modio::Detail::RequireValidModCollectionID(ModCollectionToSubscribeTo, Callback))
				{
					Modio::Detail::SubscribeToModCollectionAsync(ModCollectionToSubscribeTo, Callback);
				}
			});
	}

	void UnsubscribeFromModCollectionAsync(Modio::ModCollectionID ModCollectionToUnsubscribeFrom,
														std::function<void(Modio::ErrorCode)> Callback)
	{
		Modio::Detail::SDKSessionData::EnqueueTask(
			[ModCollectionToUnsubscribeFrom, Callback = std::move(Callback)]() mutable {
				if (Modio::Detail::RequireSDKIsInitialized(Callback) &&
					Modio::Detail::RequireUserIsAuthenticated(Callback) &&
					Modio::Detail::RequireValidModCollectionID(ModCollectionToUnsubscribeFrom, Callback))
				{
					Modio::Detail::UnsubscribeFromModCollectionAsync(ModCollectionToUnsubscribeFrom, Callback);
				}
			});
	}

	void FollowModCollectionAsync(Modio::ModCollectionID ModCollectionToFollow,
		std::function<void(Modio::ErrorCode, Modio::Optional<Modio::ModCollectionInfo>)> Callback)
	{
		Modio::Detail::SDKSessionData::EnqueueTask([ModCollectionToFollow, Callback = std::move(Callback)]() mutable {
				if (Modio::Detail::RequireSDKIsInitialized(Callback) &&
					Modio::Detail::RequireUserIsAuthenticated(Callback) &&
				Modio::Detail::RequireValidModCollectionID(ModCollectionToFollow, Callback))
				{
					Modio::Detail::FollowModCollectionAsync(ModCollectionToFollow, Callback);
				}
			});
	}

	void UnfollowModCollectionAsync(Modio::ModCollectionID ModCollectionToUnfollow,
												 std::function<void(Modio::ErrorCode)> Callback)
	{
		Modio::Detail::SDKSessionData::EnqueueTask([ModCollectionToUnfollow, Callback = std::move(Callback)]() mutable {
				if (Modio::Detail::RequireSDKIsInitialized(Callback) &&
					Modio::Detail::RequireUserIsAuthenticated(Callback) &&
				Modio::Detail::RequireValidModCollectionID(ModCollectionToUnfollow, Callback))
				{
					Modio::Detail::UnfollowModCollectionAsync(ModCollectionToUnfollow, Callback);
				}
			});
	}

	void GetModCollectionMediaAsync(Modio::ModCollectionID CollectionId, Modio::LogoSize LogoSize,
		std::function<void(Modio::ErrorCode, Modio::Optional<std::string>)> Callback)
	{
		Modio::Detail::SDKSessionData::EnqueueTask([CollectionId, LogoSize, Callback = std::move(Callback)]() mutable {
			if (Modio::Detail::RequireSDKIsInitialized(Callback) && Modio::Detail::RequireNotRateLimited(Callback) &&
				Modio::Detail::RequireValidModCollectionID(CollectionId, Callback))
			{
				Modio::Detail::GetModCollectionMediaLogoAsync(CollectionId, LogoSize, Callback);
			}
		});
	}

	void GetModCollectionMediaAsync(Modio::ModCollectionID CollectionId, Modio::AvatarSize AvatarSize,
		std::function<void(Modio::ErrorCode, Modio::Optional<std::string>)> Callback)
	{
		Modio::Detail::SDKSessionData::EnqueueTask(
			[CollectionId, AvatarSize, Callback = std::move(Callback)]() mutable {
			if (Modio::Detail::RequireSDKIsInitialized(Callback) && Modio::Detail::RequireNotRateLimited(Callback) &&
				Modio::Detail::RequireValidModCollectionID(CollectionId, Callback))
			{
				Modio::Detail::GetModCollectionMediaAvatarAsync(CollectionId, AvatarSize, Callback);
			}
		});
	}
} // namespace Modio
