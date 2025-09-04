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
	///	* 64 = Enable Previews
	///	* 128 = Enable Preview URLs
	///	* 1024 = Allow mod dependencies
	///	* 8192 = Enable Collections
	///	* 16384 = Enable Collection Comments
	enum class ModCommunityOptions : uint32_t
	{
		None = 0,
		EnableComments = 1,
		EnablePreviews = 64,
		EnablePreviewURLs = 128,
		AllowDependencies = 1024,
		EnableCollections = 8192,
		EnableCollectionComments = 16384
	};

	/// @docpublic
	/// @brief A strong type flag object to represent ModCommunityOptions
	struct ModCommunityOptionsFlags : public Modio::FlagImpl<ModCommunityOptions>
	{
		using Modio::FlagImpl<ModCommunityOptions>::FlagImpl;

		/// @docnone
		constexpr ModCommunityOptionsFlags(const Modio::FlagImpl<ModCommunityOptions>& InitialValue)
			: Modio::FlagImpl<ModCommunityOptions>(InitialValue)
		{}

		/// @docnone
		MODIO_IMPL friend void from_json(const nlohmann::json& Json, Modio::ModCommunityOptionsFlags& ModCommunity);

		/// @docnone
		MODIO_IMPL friend void to_json(nlohmann::json& Json, const Modio::ModCommunityOptionsFlags& ModCommunity);
	};
	MODIO_DEFINE_FLAG_OPERATORS(ModCommunityOptions, ModCommunityOptionsFlags);

} // namespace Modio
