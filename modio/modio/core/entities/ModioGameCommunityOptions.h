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
	/// @brief Community options for a game
	///	* 0 = None set (default)
	///	* 1 = Comments are enabled
	///	* 2 = Guides are enabled
	///	* 4 = Pin on homepage
	///	* 8 = Show on homepage
	///	* 16 = Show more on homepage
	///	* 32 = Allow change status
	///	* 64 = Enable Previews (Game must be hidden)
	///	* 128 = Allow Preview Share-URL (Previews must be enabled)
	///	* 256 = Allow negative ratings
	///	* 512 = Allow mods to be edited on web
	///	* 1024 = Allow mod dependencies
	///	* 2048 = Allow comments on guides
	enum class GameCommunityOptions : uint32_t
	{
		None = 0,
		EnableComments = 1,
		EnableGuides = 2,
		PinOnHomepage = 4,
		ShowOnHomepage = 8,
		ShowMoreOnHomepage = 16,
		AllowChangeStatus = 32,
		EnablePreviews = 64,
		AllowPreviewShareURL = 128,
		AllowNegativeRatings = 256,
		AllowModsToBeEditedOnWeb = 512,
		AllowModDependencies = 1024,
		AllowCommentsOnGuides = 2048,
	};

	/// @docpublic
	/// @brief A strong type flag object to represent GameCommunityOptions from a mod.io info.
	struct GameCommunityOptionsFlags : public Modio::FlagImpl<GameCommunityOptions>
	{
		using Modio::FlagImpl<GameCommunityOptions>::FlagImpl;

		/// @docnone
		constexpr GameCommunityOptionsFlags(const Modio::FlagImpl<GameCommunityOptions>& InitialValue)
			: Modio::FlagImpl<GameCommunityOptions>(InitialValue)
		{}

		/// @docnone
		MODIO_IMPL friend void from_json(const nlohmann::json& Json, Modio::GameCommunityOptionsFlags& GameCommunity);

		/// @docnone
		MODIO_IMPL friend void to_json(nlohmann::json& Json, const Modio::GameCommunityOptionsFlags& GameCommunity);

	};
	MODIO_DEFINE_FLAG_OPERATORS(GameCommunityOptions, GameCommunityOptions);
} // namespace Modio
