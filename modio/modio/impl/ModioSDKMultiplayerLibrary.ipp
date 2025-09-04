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
	#include "modio/detail/ModioSDKMultiplayerLibrary.h"
	#include "modio/ModioSDK.h"
#else
	#pragma once
#endif

#include "modio/core/ModioLogger.h"
#include "modio/impl/SDKPostAsync.h"
#include "modio/detail/ModioSDKSessionData.h"
#include "modio/detail/ops/modmanagement/InstallOrUpdateMod.h"
#include "modio/detail/ops/multiplayer/InstallOrUpdateServerModsOp.h"
#include "modio/detail/ops/multiplayer/RegisterClientModsWithServerOp.h"
#include "modio/detail/ops/multiplayer/InitializeModioServerOp.h"
#include "modio/file/ModioFileService.h"
#include "modio/core/ModioServices.h"
#include "modio/core/ModioTemporaryModSet.h"
#include "modio/impl/SDKPreconditionChecks.h"

namespace Modio
{
	ModioServer& ModioServer::Get()
	{
		static ModioServer Instance;
		return Instance;
	}

	void InitializeModioServerAsync(Modio::ServerInitializeOptions InitOptions, std::function<void(Modio::ErrorCode)> OnInitComplete)
	{
		Modio::Detail::SDKSessionData::EnqueueTask([InitOptions, OnInitComplete = std::move(OnInitComplete)]() mutable {
			if (Modio::Detail::RequireValidServerInitParam(InitOptions, OnInitComplete))
			{
				auto WrappedCallback = Modio::Detail::ApplyPostAsyncChecks(OnInitComplete);
				Modio::Detail::InitializeModioServerAsync(InitOptions, WrappedCallback);
			}
		});
	}

	void InstallOrUpdateServerModsAsync(std::vector<ModID> Mods,
		std::function<void(Modio::ErrorCode)> Callback)
	{
		Modio::Detail::SDKSessionData::EnqueueTask([Mods, Callback = std::move(Callback)]() mutable {
			if (Modio::Detail::RequireSDKIsInitialized(Callback) &&
				Modio::Detail::RequireModManagementEnabled(Callback) &&
				Modio::Detail::RequireAllModIDsValid(Mods, Callback))
			{
				Modio::Detail::InstallOrUpdateServerModsAsync(Mods, Callback);
			}
		});
	}

	void RegisterClientModsWithServerAsync(std::vector<Modio::ModID> ModIDs,
		std::function<void(Modio::ErrorCode, std::set<Modio::ModID>)> Callback)
	{
		Modio::Detail::SDKSessionData::EnqueueTask([ModIDs, Callback = std::move(Callback)]() mutable {
			if (Modio::Detail::RequireSDKIsInitialized(Callback) &&
				Modio::Detail::RequireAllModIDsValid(ModIDs, Callback))
			{
				Modio::Detail::RegisterClientModsWithServerAsync(ModIDs, Callback);
			}
		});
	}

	std::set<Modio::ModID> GetRegisteredClientMods()
	{
		return ModioServer::Get().ClientMods;
	}

	void ClearRegisteredClientMods()
	{
		ModioServer::Get().ClientMods.clear();
	}
} // namespace Modio
