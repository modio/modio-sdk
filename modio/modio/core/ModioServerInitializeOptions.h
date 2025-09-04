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

#include "modio/core/ModioCoreTypes.h"
#include "modio/core/ModioInitializeOptions.h"

namespace Modio
{
	/// @docpublic
	/// @brief Structure with all initialization parameters needed to start the API in the context of a dedicated server
	/// all fields need to be filled in for the SDK to properly initialize. Inherits from the base InitializeOptions.
	/// @experimental
	struct ServerInitializeOptions : public InitializeOptions
	{
		/// @docpublic
		/// @brief Default constructor that sets all variables to an invalid base value
		/// @experimental
		MODIO_IMPL ServerInitializeOptions();

		/// @docpublic
		/// @brief Explicit and preferred constructor with the necessary variables to store
		/// @param GameID The mod.io provided ID to identify the game
		/// @param APIKey The mod.io provided key associated with the game
		/// @param GameEnvironment The mod.io environment to use, listed in the enumeration Modio::Environment
		/// @param PortalInUse The service portal to use, listed in the enumeration Modio::Portal
		/// @param LocalSessionIdentifier A unique identifier for the local session. This local session can have a
		/// single mod.io Server token associated with it.
		/// @param ModsDirectory The directory where the SDK will install and look for any and all Mod files
		/// @param Token The OAuth token for this Server
		/// @param Mods A vector of ModIDs to be installed on this server as part of the the initialization process
		/// @experimental
		MODIO_IMPL explicit ServerInitializeOptions(
			Modio::GameID GameID, Modio::ApiKey APIKey, Modio::Environment GameEnvironment,
			Modio::Portal PortalInUse,
			Modio::UserHandleType LocalSessionIdentifier,
			std::string& ModsDirectory, std::string& Token,
			std::vector<Modio::ModID> Mods);

		/// @docpublic
		/// @brief The directory this Server should store its modfiles in.
		/// @experimental
		std::string ModsDirectory;

		/// @docpublic
		/// @brief The S2S token for this Server. 
		/// @experimental
		std::string Token;

		/// @docpublic
		/// @brief The base mods for this Server.
		/// @experimental
		std::vector<Modio::ModID> Mods;

	};
} // namespace Modio

#ifndef MODIO_SEPARATE_COMPILATION
	#include "ModioServerInitializeOptions.ipp"
#endif