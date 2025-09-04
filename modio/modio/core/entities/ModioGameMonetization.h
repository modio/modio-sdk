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
	/// @docpublic
	/// @brief Monetization properties of a game
	///	* 0 = None set (default)
	///	* 1 = Monetization is enabled
	///	* 2 = Marketplace is enabled
	///	* 4 = Partner Program is enabled
	///	* 8 = Scarcity feature is enabled (limited items)
	///	* 16 = Forced Scarcity feature enabled, limiting mod creators to a scarcity range
	///	* 32 = Unified monetization enabled
	///	* 64 = Removes the requirements of files for monetized mods
	///	* 128 = Premium Features
	///	* 256 = Currency Packs can be purchased via the web, rather than just in-game
	///	* 512 = Mods can be purchased via the web, rather than just in-game
	///	* 1024 = UGC Self Refunds are disabled
	///	* 2048 = Marketplace USD is enabled
	enum class GameMonetizationOptions : std::uint16_t
	{
		None = 0,
		Monetization = 1,
		Marketplace = 2,
		PartnerProgram = 4,
		Limited = 8,
		ForcedLimited = 16,
		Unified = 32,
		MonetizationFileless = 64,
		PremiumFeatures = 128,
		WebPurchasesCurrencyPacks = 256,
		WebPurchasesMods = 512,
		UGCSelfRefundsDisabled = 1024,
		MarketplaceUSD = 2048
	};

	/// @docpublic
	/// @brief A strong type flag object to represent GameMonetizationOptions from a mod.io info.
	struct GameMonetization : public Modio::FlagImpl<GameMonetizationOptions>
	{
		using Modio::FlagImpl<GameMonetizationOptions>::FlagImpl;

		/// @docnone
		constexpr GameMonetization(const Modio::FlagImpl<GameMonetizationOptions>& InitialValue)
			: Modio::FlagImpl<GameMonetizationOptions>(InitialValue)
		{}

		/// @docnone
		MODIO_IMPL friend void from_json(const nlohmann::json& Json, Modio::GameMonetization& GameMonetization);

		/// @docnone
		MODIO_IMPL friend void to_json(nlohmann::json& Json, const Modio::GameMonetization& GameMonetization);
	};
	MODIO_DEFINE_FLAG_OPERATORS(GameMonetizationOptions, GameMonetization);

} // namespace Modio
