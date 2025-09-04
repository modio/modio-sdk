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
	#include "modio/core/ModioServerInitializeOptions.h"
#endif

namespace Modio
{
	ServerInitializeOptions::ServerInitializeOptions() {}

	ServerInitializeOptions::ServerInitializeOptions(Modio::GameID GameID, Modio::ApiKey APIKey,
													 Modio::Environment GameEnvironment, Modio::Portal PortalInUse,
													 Modio::UserHandleType LocalSessionIdentifier,
													 std::string& ModsDirectory, std::string& Token,
													 std::vector<Modio::ModID> Mods)
		: InitializeOptions(GameID, APIKey, GameEnvironment, PortalInUse, LocalSessionIdentifier),
		  ModsDirectory(ModsDirectory),
		  Token(Token),
		  Mods(Mods)
		  
	{}
} // namespace Modio