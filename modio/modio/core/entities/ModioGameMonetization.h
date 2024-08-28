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
#include "modio/core/ModioFlag.h"
#include "modio/detail/JsonWrapper.h"

namespace Modio
{
	/// @brief Monetization properties of a game
	///	* 0 = None set (default)
	///	* 1 = Monetization is enabled
	///	* 2 = Marketplace is enabled
	///	* 4 = Partner Program is enabled
	enum class GameMonetizationOptions : std::uint8_t
	{
		None = 0,
		Monetization = 1,
		Marketplace = 2,
		PartnerProgram = 4,
	};

	/// @docpublic
	/// @brief A strong type flag object to represent GameMonetizationOptions from a mod.io info.
	struct GameMonetization : public Modio::FlagImpl<GameMonetizationOptions>
	{
		/// @docinternal
		/// @brief The default constructor sets GameMonetizationOptions to "None"
		GameMonetization()
		{
			Value = Convert(GameMonetizationOptions::None);
		}
		using Modio::FlagImpl<GameMonetizationOptions>::FlagImpl;

		/// @docnone
		MODIO_IMPL friend void from_json(const nlohmann::json& Json, Modio::GameMonetization& GameMonetization);

		/// @docnone
		MODIO_IMPL friend void to_json(nlohmann::json& Json, const Modio::GameMonetization& GameMonetization);
	};
} // namespace Modio
