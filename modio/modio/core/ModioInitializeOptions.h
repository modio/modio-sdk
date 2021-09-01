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
		/// @brief Initializes all variables to a invalid state if possible and minimal functionality
		MODIO_IMPL InitializeOptions();
		MODIO_IMPL explicit InitializeOptions(Modio::GameID GameID, Modio::ApiKey APIKey,
											  Modio::Environment GameEnvironment, Modio::Portal PortalInUse,
											  Modio::UserHandleType User);

		/// @brief The Mod.io-provided ID for the game
		Modio::GameID GameID = Modio::GameID::InvalidGameID();
		/// @brief The Mod.io-provided API key for your application or game
		Modio::ApiKey APIKey = Modio::ApiKey::InvalidAPIKey();
		/// @brief The user handle for the platform you are targeting
		Modio::UserHandleType User;
		/// @brief Which portal is this game currently running in?
		Modio::Portal PortalInUse = Modio::Portal::None;
		/// @brief The mod.io environment you want to run the SDK on
		Modio::Environment GameEnvironment = Modio::Environment::Test;
		/// @brief Platform-specific extended initialization parameters. See xref:Platform-specific Notes[] for more
		/// information regarding any special values your platform requires or supports.
		std::map<std::string, std::string> ExtendedParameters;
	};
} // namespace Modio

#ifndef MODIO_SEPARATE_COMPILATION
	#include "ModioInitializeOptions.ipp"
#endif
