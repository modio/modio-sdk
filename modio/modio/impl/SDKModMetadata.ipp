/*
 *  Copyright (C) 2021 mod.io Pty Ltd. <https://mod.io>
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

#include "modio/core/ModioStdTypes.h"
#include "modio/core/entities/ModioModDetails.h"
#include "modio/core/entities/ModioModInfo.h"
#include "modio/core/entities/ModioModInfoList.h"
#include "modio/core/entities/ModioModTagOptions.h"
#include "modio/detail/ops/mod/GetModDependenciesOp.h"
#include "modio/detail/ops/mod/GetModDetailsOp.h"
#include "modio/detail/ops/mod/GetModInfoOp.h"
#include "modio/detail/ops/mod/GetModMediaAvatarOp.h"
#include "modio/detail/ops/mod/GetModMediaGalleryOp.h"
#include "modio/detail/ops/mod/GetModMediaLogoOp.h"
#include "modio/detail/ops/mod/GetModTagsOp.h"
#include "modio/detail/ops/mod/ListAllModsOp.h"
#include "modio/detail/ops/mod/ListUserCreatedModsOp.h"
#include "modio/detail/ops/mod/SubmitModRatingOp.h"
#include "modio/detail/ops/mod/AddModDependenciesOp.h"
#include "modio/detail/ops/mod/DeleteModDependenciesOp.h"
#include "modio/detail/serialization/ModioFileMetadataSerialization.h"
#include "modio/detail/serialization/ModioImageSerialization.h"
#include "modio/detail/serialization/ModioModStatsSerialization.h"
#include "modio/detail/serialization/ModioProfileMaturitySerialization.h"
#include "modio/detail/serialization/ModioResponseErrorSerialization.h"
#include "modio/impl/SDKPreconditionChecks.h"

// Implementation header - do not include directly

namespace Modio
{
	void ListAllModsAsync(FilterParams Filter,
						  std::function<void(Modio::ErrorCode, Modio::Optional<Modio::ModInfoList>)> Callback)
	{
		Modio::Detail::SDKSessionData::EnqueueTask(
			[Filter = std::move(Filter), Callback = std::move(Callback)]() mutable {
				if (Modio::Detail::RequireSDKIsInitialized(Callback) && Modio::Detail::RequireNotRateLimited(Callback))
				{
					Modio::Detail::ListAllModsAsync(Filter, Callback);
				}
			});
	}

	void ListUserCreatedModsAsync(FilterParams Filter,
								  std::function<void(Modio::ErrorCode, Modio::Optional<Modio::ModInfoList>)> Callback)
	{
		Modio::Detail::SDKSessionData::EnqueueTask([Filter = std::move(Filter),
													Callback = std::move(Callback)]() mutable {
			if (Modio::Detail::RequireSDKIsInitialized(Callback) && Modio::Detail::RequireNotRateLimited(Callback) &&
				Modio::Detail::RequireUserIsAuthenticated(Callback))
			{
				Modio::Detail::ListUserCreatedModsAsync(Filter, Callback);
			}
		});
	}

	void GetModInfoAsync(Modio::ModID ModId,
						 std::function<void(Modio::ErrorCode, Modio::Optional<Modio::ModInfo>)> Callback)
	{
		Modio::Detail::SDKSessionData::EnqueueTask([ModId, Callback = std::move(Callback)]() mutable {
			if (Modio::Detail::RequireSDKIsInitialized(Callback) && Modio::Detail::RequireNotRateLimited(Callback) &&
				Modio::Detail::RequireValidModID(ModId, Callback))
			{
				Modio::Detail::GetModInfoAsync(ModId, Callback);
			}
		});
	}
// Disabled
#if (0)
	void GetModFileDetailsAsync(Modio::ModID ModId,
								std::function<void(Modio::ErrorCode, Modio::Optional<Modio::ModDetails>)> Callback)
	{
		return Modio::Detail::GetModDetailsAsync(ModID, Callback);
	}
#endif
	void GetModMediaAsync(Modio::ModID ModId, Modio::LogoSize LogoSize,
						  std::function<void(Modio::ErrorCode, Modio::Optional<std::string>)> Callback)
	{
		Modio::Detail::SDKSessionData::EnqueueTask([ModId, LogoSize, Callback = std::move(Callback)]() mutable {
			if (Modio::Detail::RequireSDKIsInitialized(Callback) && Modio::Detail::RequireNotRateLimited(Callback) &&
				Modio::Detail::RequireValidModID(ModId, Callback))
			{
				Modio::Detail::GetModMediaLogoAsync(ModId, LogoSize, Callback);
			}
		});
	}

	void GetModMediaAsync(Modio::ModID ModId, Modio::AvatarSize AvatarSize,
						  std::function<void(Modio::ErrorCode, Modio::Optional<std::string>)> Callback)
	{
		Modio::Detail::SDKSessionData::EnqueueTask([ModId, AvatarSize, Callback = std::move(Callback)]() mutable {
			if (Modio::Detail::RequireSDKIsInitialized(Callback) && Modio::Detail::RequireNotRateLimited(Callback) &&
				Modio::Detail::RequireValidModID(ModId, Callback))
			{
				Modio::Detail::GetModMediaAvatarAsync(ModId, AvatarSize, Callback);
			}
		});
	}

	void GetModMediaAsync(Modio::ModID ModId, Modio::GallerySize GallerySize, Modio::GalleryIndex Index,
						  std::function<void(Modio::ErrorCode, Modio::Optional<std::string>)> Callback)
	{
		Modio::Detail::SDKSessionData::EnqueueTask(
			[ModId, GallerySize, Index, Callback = std::move(Callback)]() mutable {
				if (Modio::Detail::RequireSDKIsInitialized(Callback) &&
					Modio::Detail::RequireNotRateLimited(Callback) && Modio::Detail::RequireValidModID(ModId, Callback))
				{
					Modio::Detail::GetModMediaGalleryAsync(ModId, GallerySize, Index, Callback);
				}
			});
	}

	void GetModTagOptionsAsync(std::function<void(Modio::ErrorCode, Modio::Optional<Modio::ModTagOptions>)> Callback)
	{
		Modio::Detail::SDKSessionData::EnqueueTask([Callback = std::move(Callback)]() mutable {
			if (Modio::Detail::RequireSDKIsInitialized(Callback) && Modio::Detail::RequireNotRateLimited(Callback))
			{
				Modio::Detail::GetModTagsAsync(Callback);
			}
		});
	}

	void SubmitModRatingAsync(Modio::ModID ModID, Modio::Rating Rating, std::function<void(Modio::ErrorCode)> Callback)
	{
		Modio::Detail::SDKSessionData::EnqueueTask([ModID, Rating, Callback = std::move(Callback)]() mutable {
			if (Modio::Detail::RequireSDKIsInitialized(Callback) && Modio::Detail::RequireNotRateLimited(Callback) &&
				Modio::Detail::RequireUserIsAuthenticated(Callback) &&
				Modio::Detail::RequireValidModID(ModID, Callback))
			{
				Modio::Detail::SubmitModRatingAsync(ModID, Rating, Callback);
			}
		});
	}

	void GetModDependenciesAsync(
		Modio::ModID ModID, bool Recursive,
		std::function<void(Modio::ErrorCode, Modio::Optional<Modio::ModDependencyList> Dependencies)> Callback)
	{
		Modio::Detail::SDKSessionData::EnqueueTask([ModID, Recursive, Callback = std::move(Callback)]() mutable {
			if (Modio::Detail::RequireSDKIsInitialized(Callback) && Modio::Detail::RequireNotRateLimited(Callback) &&
				Modio::Detail::RequireValidModID(ModID, Callback))
			{
				Modio::Detail::GetModDependenciesAsync(ModID, Recursive, Callback);
			}
		});
	}

	void AddModDependenciesAsync(Modio::ModID ModID, std::vector<Modio::ModID> Dependencies,
								 std::function<void(Modio::ErrorCode)> Callback)
	{
		Modio::Detail::SDKSessionData::EnqueueTask([ModID, Dependencies = std::move(Dependencies),
													Callback = std::move(Callback)]() mutable {
			if (Modio::Detail::RequireSDKIsInitialized(Callback) && Modio::Detail::RequireNotRateLimited(Callback) &&
				Modio::Detail::RequireUserIsAuthenticated(Callback) &&
				Modio::Detail::RequireValidModID(ModID, Callback))
			{
				Modio::Detail::AddModDependenciesAsync(ModID, Dependencies, Callback);
			}
		});
	}

	void DeleteModDependenciesAsync(Modio::ModID ModID, std::vector<Modio::ModID> Dependencies,
												 std::function<void(Modio::ErrorCode)> Callback)
	{
		Modio::Detail::SDKSessionData::EnqueueTask([ModID, Dependencies = std::move(Dependencies),
													Callback = std::move(Callback)]() mutable {
			if (Modio::Detail::RequireSDKIsInitialized(Callback) && Modio::Detail::RequireNotRateLimited(Callback) &&
				Modio::Detail::RequireUserIsAuthenticated(Callback) &&
				Modio::Detail::RequireValidModID(ModID, Callback))
			{
				Modio::Detail::DeleteModDependenciesAsync(ModID, Dependencies, Callback);
			}
		});
	}
} // namespace Modio
