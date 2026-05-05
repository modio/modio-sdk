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

#include "modio/core/ModioSplitCompilation.h"
#include "modio/core/ModioFlag.h"

namespace Modio
{
	/// @docpublic
	/// @brief Maturity options for a game
	///	* 0 = Don't allow mature content in mods (default)
	///	* 1 = This game allows mods containing mature content
	///	* 2 = This game is for mature audiences only
	enum class GameMaturityOptions : uint32_t
	{
		None = 0,
		MatureModsAllowed = 1,
		MatureAudiencesOnly = 2,
	};

	/// @docpublic
	/// @brief A strong type flag object to represent GameMaturityOptions from a mod.io info.
	struct GameMaturityOptionsFlags : public Modio::FlagImpl<Modio::GameMaturityOptions>
	{
		using Modio::FlagImpl<Modio::GameMaturityOptions>::FlagImpl;

		/// @docnone
		constexpr GameMaturityOptionsFlags(const Modio::FlagImpl<Modio::GameMaturityOptions>& InitialValue)
			: Modio::FlagImpl<Modio::GameMaturityOptions>(InitialValue)
		{}
	};
	MODIO_DEFINE_FLAG_OPERATORS(Modio::GameMaturityOptions, Modio::GameMaturityOptionsFlags);

} // namespace Modio
