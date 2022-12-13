/* 
 *  Copyright (C) 2021 mod.io Pty Ltd. <https://mod.io>
 *  
 *  This file is part of the mod.io SDK.
 *  
 *  Distributed under the MIT License. (See accompanying file LICENSE or 
 *   view online at <https://github.com/modio/modio-sdk/blob/main/LICENSE>)
 *   
 */

#pragma once

#include "modio/core/ModioCoreTypes.h"

namespace Modio
{
	/// @docpublic
	/// @brief Structure with all initialization parameters needed to start the API, all need to be filled in for the
	/// SDK to properly initialize
	struct InitializeOptions
	{
		/// @docpublic
		/// @brief Default constructor that sets all variables to an invalid base value
		MODIO_IMPL InitializeOptions();

		/// @docpublic
		/// @brief Explicit and preferred constructor with the necessary variables to store
		/// @param GameID Mod.io provided to identify the game
		/// @param APIKey Mod.io provided key associated with the game
		/// @param GameEnvironment Mod.io environment to use, listed in the enumeration Modio::Environment
		/// @param PortalInUse The service portal to use, listed in the enumeration Modio::Portal
		/// @param LocalSessionIdentifier A unique identifier for the local session. This local session can have a single mod.io user account associated with it.
		MODIO_IMPL explicit InitializeOptions(Modio::GameID GameID, Modio::ApiKey APIKey,
											  Modio::Environment GameEnvironment, Modio::Portal PortalInUse,
											  Modio::UserHandleType LocalSessionIdentifier);

		/// @brief The Mod.io-provided ID for the game.
		Modio::GameID GameID = Modio::GameID::InvalidGameID();
		/// @brief The Mod.io-provided API key for your application or game.
		Modio::ApiKey APIKey = Modio::ApiKey::InvalidAPIKey();
		/// @brief Unique identifier for the local session, which will optionally contain user data for an authenticated mod.io user (once authentication is completed).
		Modio::UserHandleType User;
		/// @brief The storefront or distribution method to use. The API uses this parameter to serve
		/// platform-compliant user names. Use `Modio::Portal::None` if your title does not require this.
		Modio::Portal PortalInUse = Modio::Portal::None;
		/// @brief The mod.io environment you want to run the SDK on.
		Modio::Environment GameEnvironment = Modio::Environment::Test;
		/// @brief Platform-specific extended initialization parameters. See xref:Platform-specific Notes[] for more
		/// information regarding any special values your platform requires or supports.
		std::map<std::string, std::string> ExtendedParameters;
	};
} // namespace Modio

#ifndef MODIO_SEPARATE_COMPILATION
	#include "ModioInitializeOptions.ipp"
#endif
