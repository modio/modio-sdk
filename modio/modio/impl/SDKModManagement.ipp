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

#include "modio/core/ModioCoreTypes.h"
#include "modio/core/ModioErrorCode.h"
#include "modio/core/ModioModCollectionEntry.h"
#include "modio/core/ModioServices.h"
#include "modio/detail/AsioWrapper.h"
#include "modio/detail/FilesystemWrapper.h"
#include "modio/detail/ModioSDKSessionData.h"
#include "modio/detail/ops/AddOrUpdateModLogoOp.h"
#include "modio/detail/ops/ModManagementLoop.h"
#include "modio/detail/ops/SubscribeToModOp.h"
#include "modio/detail/ops/UnsubscribeFromMod.h"
#include "modio/detail/ops/mod/ArchiveModOp.h"
#include "modio/detail/ops/mod/SubmitModChangesOp.h"
#include "modio/detail/ops/mod/SubmitNewModFileOp.h"
#include "modio/detail/ops/mod/SubmitNewModOp.h"
#include "modio/detail/ops/modmanagement/ForceUninstallModOp.h"
#include "modio/file/ModioFileService.h"
#include "modio/impl/SDKPreconditionChecks.h"
#include "modio/userdata/ModioUserDataService.h"

namespace Modio
{
	Modio::ErrorCode EnableModManagement(std::function<void(Modio::ModManagementEvent)> ModManagementHandler)
	{
		if (!Modio::Detail::SDKSessionData::IsInitialized())
		{
			Modio::Detail::Logger().Log(LogLevel::Warning, LogCategory::Core,
										"SDK is not initialized. Cannot Enable Mod Management.");
			return Modio::make_error_code(Modio::GenericError::SDKNotInitialized);
		}
		if (Modio::Detail::SDKSessionData::IsModManagementEnabled())
		{
			Modio::Detail::Logger().Log(LogLevel::Warning, LogCategory::Core, "Mod Management is already enabled");
			return Modio::make_error_code(Modio::ModManagementError::ModManagementAlreadyEnabled);
		}
		Modio::Detail::SDKSessionData::SetUserModManagementCallback(ModManagementHandler);
		Modio::Detail::SDKSessionData::AllowModManagement();
		Modio::Detail::BeginModManagementLoopAsync([](Modio::ErrorCode ec) mutable {
			if (ec)
			{
				Modio::Detail::Logger().Log(LogLevel::Info, Modio::LogCategory::Core,
											"Mod Management Loop halted: status message {}", ec.message());
			}
		});
		return {};
	}
	void DisableModManagement()
	{
		Modio::Detail::SDKSessionData::SetUserModManagementCallback(std::function<void(Modio::ModManagementEvent)> {});
		Modio::Detail::SDKSessionData::DisableModManagement();
	}

	void FetchExternalUpdatesAsync(std::function<void(Modio::ErrorCode)> OnFetchDone)
	{
		if (Modio::Detail::RequireSDKIsInitialized(OnFetchDone) && Modio::Detail::RequireNotRateLimited(OnFetchDone) &&
			Modio::Detail::RequireUserIsAuthenticated(OnFetchDone) &&
			Modio::Detail::RequireModManagementEnabled(OnFetchDone))
		{
			Modio::Detail::FetchExternalUpdatesAsync(OnFetchDone);
		}
	}

	void SubscribeToModAsync(Modio::ModID ModToSubscribeTo, std::function<void(Modio::ErrorCode)> OnSubscribeComplete)
	{
		if (Modio::Detail::RequireSDKIsInitialized(OnSubscribeComplete) &&
			Modio::Detail::RequireNotRateLimited(OnSubscribeComplete) &&
			Modio::Detail::RequireUserIsAuthenticated(OnSubscribeComplete) &&
			Modio::Detail::RequireModManagementEnabled(OnSubscribeComplete) &&
			Modio::Detail::RequireModIsNotUninstallPending(ModToSubscribeTo, OnSubscribeComplete))

		{
			return asio::async_compose<std::function<void(Modio::ErrorCode)>, void(Modio::ErrorCode)>(
				Modio::Detail::SubscribeToModOp(Modio::Detail::SDKSessionData::CurrentGameID(),
												Modio::Detail::SDKSessionData::CurrentAPIKey(), ModToSubscribeTo),
				OnSubscribeComplete, Modio::Detail::Services::GetGlobalContext().get_executor());
		}
	}

	void UnsubscribeFromModAsync(Modio::ModID ModToUnsubscribeFrom,
								 std::function<void(Modio::ErrorCode)> OnUnsubscribeComplete)
	{
		if (Modio::Detail::RequireSDKIsInitialized(OnUnsubscribeComplete) &&
			Modio::Detail::RequireNotRateLimited(OnUnsubscribeComplete) &&
			Modio::Detail::RequireUserIsAuthenticated(OnUnsubscribeComplete) &&
			Modio::Detail::RequireModManagementEnabled(OnUnsubscribeComplete))
		{
			return Modio::Detail::UnsubscribeFromModAsync(ModToUnsubscribeFrom, OnUnsubscribeComplete);
		}
	}

	bool IsModManagementBusy()
	{
		if (!Modio::Detail::SDKSessionData::IsModManagementEnabled())
		{
			return false;
		}

		Modio::ModCollection UserModCollection =
			Modio::Detail::SDKSessionData::FilterSystemModCollectionByUserSubscriptions();

		for (auto ModEntry : UserModCollection.Entries())
		{
			Modio::ModState CurrentState = ModEntry.second->GetModState();
			if (CurrentState != Modio::ModState::Installed)
			{
				return true;
			}
		}
		return false;
	}

	Modio::ErrorCode PrioritizeTransferForMod(Modio::ModID IDToPrioritize)
	{
		// Check if we have already set this ID to be prioritized
		if (Modio::Optional<Modio::ModID> PrioritizedID = Modio::Detail::SDKSessionData::GetPriorityModID())
		{
			if (*PrioritizedID == IDToPrioritize)
			{
				Modio::Detail::Logger().Log(
					LogLevel::Info, LogCategory::ModManagement,
					"Called PrioritizeTransferForMod() on mod {}.  This mod is already prioritized", IDToPrioritize);
				return {};
			}
		}

		// Check if priority ID is currently being processed
		if (Modio::Optional<Modio::ModProgressInfo> CurrentOperationInfo =
				Modio::Detail::SDKSessionData::GetModProgress())
		{
			if (CurrentOperationInfo->ID == IDToPrioritize)
			{
				Modio::Detail::Logger().Log(
					LogLevel::Info, LogCategory::ModManagement,
					"Called PrioritizeTransferForMod() on mod {}.  This mod is already being processed",
					IDToPrioritize);
				return {};
			}
		}

		// Check if the ID corresponds to a pending upload first, then check if it is a pending download
		if (Modio::Detail::SDKSessionData::PrioritizeModfileUpload(IDToPrioritize) ||
			Modio::Detail::SDKSessionData::PrioritizeModfileDownload(IDToPrioritize))
		{
			// Cancel the in-progress thing so we move onto the priority immediately

			// FinishModDownloadOrUpdate() causes issues -- cancelled op doesn't retry. State gets confused.  Need more
			// robust testing before we implement this feature

			// Modio::Detail::SDKSessionData::FinishModDownloadOrUpdate();
			return {};
		}

		// ID is not present in the list of pending operations, return error
		return Modio::make_error_code(Modio::GenericError::BadParameter);
	}

	Modio::Optional<Modio::ModProgressInfo> QueryCurrentModUpdate()
	{
		return Modio::Detail::SDKSessionData::GetModProgress();
	}

	std::map<Modio::ModID, Modio::ModCollectionEntry> QueryUserSubscriptions()
	{
		Modio::ModCollection UserModCollection =
			Modio::Detail::SDKSessionData::FilterSystemModCollectionByUserSubscriptions();
		std::map<Modio::ModID, ModCollectionEntry> UserSubscriptions;

		for (auto ModEntry : UserModCollection.Entries())
		{
			UserSubscriptions.emplace(std::make_pair(ModEntry.first, (*ModEntry.second)));
		}
		return UserSubscriptions;
	}

	std::vector<std::string> GetBaseModInstallationDirectories()
	{
		auto RootInstallationDirectory = Modio::Detail::Services::GetGlobalService<Modio::Detail::FileService>()
											 .GetModRootInstallationPath()
											 .string();

		std::vector<std::string> Paths;

		Paths.push_back(RootInstallationDirectory);

		return Paths;
	}

	std::map<Modio::ModID, Modio::ModCollectionEntry> QueryUserInstallations(bool bIncludeOutdatedMods)
	{
		Modio::ModCollection UserModCollection =
			Modio::Detail::SDKSessionData::FilterSystemModCollectionByUserSubscriptions();
		std::map<Modio::ModID, ModCollectionEntry> UserInstallations;

		// Only return mods that are either installed, and if bIncludeOutdatedMods mods that are installed but have
		// an update available that isn't currently being processed
		for (auto ModEntry : UserModCollection.Entries())
		{
			if (ModEntry.second->GetModState() == ModState::Installed)
			{
				UserInstallations.emplace(std::make_pair(ModEntry.first, (*ModEntry.second)));
			}
			if (bIncludeOutdatedMods && ModEntry.second->GetModState() == ModState::UpdatePending)
			{
				UserInstallations.emplace(std::make_pair(ModEntry.first, (*ModEntry.second)));
			}
		}
		return UserInstallations;
	}

	std::map<Modio::ModID, Modio::ModCollectionEntry> QuerySystemInstallations()
	{
		std::map<Modio::ModID, ModCollectionEntry> InstalledMods;
		Modio::ModCollection& AllInstalledMods = Modio::Detail::SDKSessionData::GetSystemModCollection();
		for (auto& ModEntry : AllInstalledMods.Entries())
		{
			InstalledMods.emplace(std::make_pair(ModEntry.first, (*ModEntry.second)));
		}
		return InstalledMods;
	}

	void ForceUninstallModAsync(Modio::ModID ModToRemove, std::function<void(Modio::ErrorCode)> Callback)
	{
		if (Modio::Detail::RequireSDKIsInitialized(Callback) && Modio::Detail::RequireUserIsAuthenticated(Callback) &&
			Modio::Detail::RequireModManagementEnabled(Callback) &&
			Modio::Detail::RequireUserNotSubscribed(ModToRemove, Callback))
		{
			asio::async_compose<std::function<void(Modio::ErrorCode)>, void(Modio::ErrorCode)>(
				Modio::Detail::ForceUninstallModOp(ModToRemove), Callback,
				Modio::Detail::Services::GetGlobalContext().get_executor());
		}
	}

	void SubmitNewModAsync(Modio::ModCreationHandle Handle, Modio::CreateModParams Params,
						   std::function<void(Modio::ErrorCode, Modio::Optional<Modio::ModID> CreatedModID)> Callback)
	{
		if (Modio::Detail::RequireSDKIsInitialized(Callback) && Modio::Detail::RequireUserIsAuthenticated(Callback))
		{
			if (Modio::Optional<Modio::ModID> ResolvedID =
					Modio::Detail::SDKSessionData::ResolveModCreationHandle(Handle))
			{
				Modio::Detail::Logger().Log(
					Modio::LogLevel::Warning, Modio::LogCategory::ModManagement,
					"Attempted to call SubmitNewModAsync with an already-used handle. Returning existing Mod ID");
				asio::post(Modio::Detail::Services::GetGlobalContext().get_executor(),
						   [ID = ResolvedID.value(), Callback]() { Callback({}, ID); });
				return;
			}
			return Modio::Detail::SubmitNewModAsync(Handle, Params, Callback);
		}
	}

	void SubmitModChangesAsync(Modio::ModID Mod, Modio::EditModParams Params,
							   std::function<void(Modio::ErrorCode ec, Modio::Optional<Modio::ModInfo>)> Callback)
	{
		if (Modio::Detail::RequireSDKIsInitialized(Callback) && Modio::Detail::RequireUserIsAuthenticated(Callback) &&
			Modio::Detail::RequireValidEditModParams(Params, Callback))
		{
			return Modio::Detail::SubmitModChangesAsync(Mod, Params, Callback);
		}
	}

	Modio::ModCreationHandle GetModCreationHandle()
	{
		return Modio::Detail::SDKSessionData::GetNextModCreationHandle();
	}

	MODIOSDK_API void SubmitNewModFileForMod(Modio::ModID Mod, Modio::CreateModFileParams Params)
	{
		// @TODO: Right now we don't have a pattern for returning Modio::ErrorCode from a method if we don't have a
		// callback For now, we're just going to log if this fails so developers have some way to track it
		if (!Modio::Detail::SDKSessionData::IsInitialized())
		{
			Modio::Detail::Logger().Log(Modio::LogLevel::Warning, Modio::LogCategory::ModManagement,
										"Attempted to call SubmitNewModFileForMod but the SDK was not initialized.");

			return;
		}

		if (!Modio::Detail::SDKSessionData::GetAuthenticatedUser().has_value())
		{
			Modio::Detail::Logger().Log(
				Modio::LogLevel::Warning, Modio::LogCategory::ModManagement,
				"Attempted to call SubmitNewModFileForMod but there was no authenticated user.");

			return;
		}

		if (!Modio::Detail::SDKSessionData::IsModManagementEnabled())
		{
			Modio::Detail::Logger().Log(Modio::LogLevel::Warning, Modio::LogCategory::ModManagement,
										"Attempted to call SubmitNewModFileForMod but mod management was not enabled.");

			return;
		}

		// TODO: @modio-core we should return the error code from this function so we can do our precondition checks
		Modio::Detail::SDKSessionData::AddPendingModfileUpload(Mod, Params);
	}

	void AddOrUpdateModLogoAsync(Modio::ModID ModID, std::string LogoPath,
								 std::function<void(Modio::ErrorCode)> Callback)
	{
		if (Modio::Detail::RequireSDKIsInitialized(Callback) && Modio::Detail::RequireNotRateLimited(Callback) &&
			Modio::Detail::RequireUserIsAuthenticated(Callback))
		{
			return asio::async_compose<std::function<void(Modio::ErrorCode)>, void(Modio::ErrorCode)>(
				Modio::Detail::AddOrUpdateModLogoOp(Modio::Detail::SDKSessionData::CurrentGameID(), ModID, Modio::filesystem::path(LogoPath)),
				Callback, Modio::Detail::Services::GetGlobalContext().get_executor());
		}
	}

	void ArchiveModAsync(Modio::ModID ModID, std::function<void(Modio::ErrorCode)> Callback)
	{
		if (Modio::Detail::RequireSDKIsInitialized(Callback) && Modio::Detail::RequireNotRateLimited(Callback) &&
			Modio::Detail::RequireUserIsAuthenticated(Callback))
		{
			return asio::async_compose<std::function<void(Modio::ErrorCode)>, void(Modio::ErrorCode)>(
				Modio::Detail::ArchiveModOp(Modio::Detail::SDKSessionData::CurrentGameID(), ModID), Callback,
				Modio::Detail::Services::GetGlobalContext().get_executor());
		}
	}

} // namespace Modio