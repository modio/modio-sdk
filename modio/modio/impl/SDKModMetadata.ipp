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
					asio::async_compose<std::function<void(Modio::ErrorCode, Modio::Optional<Modio::ModInfoList>)>,
										void(Modio::ErrorCode, Modio::Optional<Modio::ModInfoList>)>(
						Modio::Detail::ListAllModsOp(Modio::Detail::SDKSessionData::CurrentGameID(), std::move(Filter)),
						Callback, Modio::Detail::Services::GetGlobalContext().get_executor());
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
				asio::async_compose<std::function<void(Modio::ErrorCode, Modio::Optional<Modio::ModInfoList>)>,
									void(Modio::ErrorCode, Modio::Optional<Modio::ModInfoList>)>(
					Modio::Detail::ListUserCreatedModsOp(Modio::Detail::SDKSessionData::CurrentGameID(), Filter),
					Callback, Modio::Detail::Services::GetGlobalContext().get_executor());
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
				asio::async_compose<std::function<void(Modio::ErrorCode, Modio::Optional<Modio::ModInfo>)>,
									void(Modio::ErrorCode, Modio::Optional<Modio::ModInfo>)>(
					Modio::Detail::GetModInfoOp(Modio::Detail::SDKSessionData::CurrentGameID(),
												Modio::Detail::SDKSessionData::CurrentAPIKey(), ModId),
					Callback, Modio::Detail::Services::GetGlobalContext().get_executor());
			}
		});
	}
// Disabled
#if (0)
	void GetModFileDetailsAsync(Modio::ModID ModId,
								std::function<void(Modio::ErrorCode, Modio::Optional<Modio::ModDetails>)> Callback)
	{
		return asio::async_compose<std::function<void(Modio::ErrorCode, Modio::Optional<Modio::ModDetails>)>,
								   void(Modio::ErrorCode, Modio::Optional<Modio::ModDetails>)>(
			Modio::Detail::GetModDetailsOp(Modio::Detail::SDKSessionData::CurrentGameID(),
										   Modio::Detail::SDKSessionData::CurrentAPIKey(), ModId),
			Callback, Modio::Detail::Services::GetGlobalContext().get_executor());
	}
#endif
	void GetModMediaAsync(Modio::ModID ModId, Modio::LogoSize LogoSize,
						  std::function<void(Modio::ErrorCode, Modio::Optional<std::string>)> Callback)
	{
		Modio::Detail::SDKSessionData::EnqueueTask([ModId, LogoSize, Callback = std::move(Callback)]() mutable {
			if (Modio::Detail::RequireSDKIsInitialized(Callback) && Modio::Detail::RequireNotRateLimited(Callback) &&
				Modio::Detail::RequireValidModID(ModId, Callback))
			{
				asio::async_compose<std::function<void(Modio::ErrorCode, Modio::Optional<std::string>)>,
									void(Modio::ErrorCode, Modio::Optional<std::string>)>(
					Modio::Detail::GetModMediaLogoOp(Modio::Detail::SDKSessionData::CurrentGameID(),
													 Modio::Detail::SDKSessionData::CurrentAPIKey(), ModId, LogoSize),
					Callback, Modio::Detail::Services::GetGlobalContext().get_executor());
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
				asio::async_compose<std::function<void(Modio::ErrorCode, Modio::Optional<std::string>)>,
									void(Modio::ErrorCode, Modio::Optional<std::string>)>(
					Modio::Detail::GetModMediaAvatarOp(Modio::Detail::SDKSessionData::CurrentGameID(),
													   Modio::Detail::SDKSessionData::CurrentAPIKey(), ModId,
													   AvatarSize),
					Callback, Modio::Detail::Services::GetGlobalContext().get_executor());
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
					asio::async_compose<std::function<void(Modio::ErrorCode, Modio::Optional<std::string>)>,
										void(Modio::ErrorCode, Modio::Optional<std::string>)>(
						Modio::Detail::GetModMediaGalleryOp(Modio::Detail::SDKSessionData::CurrentGameID(),
															Modio::Detail::SDKSessionData::CurrentAPIKey(), ModId,
															GallerySize, Index),
						Callback, Modio::Detail::Services::GetGlobalContext().get_executor());
				}
			});
	}

	void GetModTagOptionsAsync(std::function<void(Modio::ErrorCode, Modio::Optional<Modio::ModTagOptions>)> Callback)
	{
		Modio::Detail::SDKSessionData::EnqueueTask([Callback = std::move(Callback)]() mutable {
			if (Modio::Detail::RequireSDKIsInitialized(Callback) && Modio::Detail::RequireNotRateLimited(Callback))
			{
				asio::async_compose<std::function<void(Modio::ErrorCode, Modio::Optional<Modio::ModTagOptions>)>,
									void(Modio::ErrorCode, Modio::Optional<Modio::ModTagOptions>)>(
					Modio::Detail::GetModTagsOp(), Callback,
					Modio::Detail::Services::GetGlobalContext().get_executor());
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
				asio::async_compose<std::function<void(Modio::ErrorCode)>, void(Modio::ErrorCode)>(
					Modio::Detail::SubmitModRatingOp(ModID, Rating), Callback,
					Modio::Detail::Services::GetGlobalContext().get_executor());
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
				asio::async_compose<std::function<void(Modio::ErrorCode, Modio::Optional<Modio::ModDependencyList>)>,
									void(Modio::ErrorCode, Modio::Optional<Modio::ModDependencyList>)>(
					Modio::Detail::GetModDependenciesOp(ModID, Modio::Detail::SDKSessionData::CurrentGameID(),
														Recursive),
					Callback, Modio::Detail::Services::GetGlobalContext().get_executor());
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
				asio::async_compose<std::function<void(Modio::ErrorCode)>, void(Modio::ErrorCode)>(
					Modio::Detail::AddModDependenciesOp(ModID, Dependencies), Callback,
					Modio::Detail::Services::GetGlobalContext().get_executor());
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
				asio::async_compose<std::function<void(Modio::ErrorCode)>, void(Modio::ErrorCode)>(
					Modio::Detail::DeleteModDependenciesOp(ModID, Dependencies), Callback,
					Modio::Detail::Services::GetGlobalContext().get_executor());
			}
		});
	}
} // namespace Modio
