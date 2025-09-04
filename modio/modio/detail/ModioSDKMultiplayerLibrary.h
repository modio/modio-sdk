/*
 *  Copyright (C) 2025 mod.io Pty Ltd. <https://mod.io>
 *
 *  This file is part of the mod.io SDK.
 *
 *  Distributed under the MIT License. (See accompanying file LICENSE or
 *   view online at <https://github.com/modio/modio-sdk/blob/main/LICENSE>)
 *
 */

#pragma once

#include "modio/ModioSDK.h"

#include "modio/core/ModioServerInitializeOptions.h"


namespace Modio
{
	class ModioServer
	{
	public:

		Modio::ServerInitializeOptions CachedServerInitializationOptions;
		std::set<Modio::ModID> ClientMods;

		MODIO_IMPL static std::set<Modio::ModID> GetClientMods()
		{
			return Get().ClientMods;
		}

		MODIO_IMPL static std::set<Modio::ModID> AddClientMods(std::vector<Modio::ModID> Mods)
		{
			for (const Modio::ModID& Mod : Mods)
			{
				Get().ClientMods.insert(Mod);
			}
			return Get().ClientMods;
		}

		MODIO_IMPL static void ClearClientMods()
		{
			Get().ClientMods.clear();
		}

		MODIO_IMPL static ModioServer& Get();

		MODIO_IMPL static bool IsValid()
		{
			return Get().CachedServerInitializationOptions.Token.length() > 0;
		}

		// Maybe call it reset? Or clear?
		// Do we want an internally tracked variable for "Init state" or such?
		MODIO_IMPL static void Shutdown()
		{
			Get().ClearClientMods();
			Get().CachedServerInitializationOptions = {};
		}
	};
} // namespace Modio

#ifndef MODIO_SEPARATE_COMPILATION
	#include "modio/impl/ModioSDKMultiplayerLibrary.ipp"
#endif