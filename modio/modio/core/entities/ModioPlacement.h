/*
 *  Copyright (C) 2026 mod.io Pty Ltd. <https://mod.io>
 *
 *  This file is part of the mod.io SDK.
 *
 *  Distributed under the MIT License. (See accompanying file LICENSE or
 *   view online at <https://github.com/modio/modio-sdk/blob/main/LICENSE>)
 *
 */

#pragma once

#include "modio/core/ModioCoreTypes.h"
#include "modio/core/ModioFilterParams.h"

namespace Modio
{

	/// @docpublic
	/// @brief Contains placements info for mod views
	struct Placement
	{
		/// @docpublic
		/// @brief Type of the placement
		enum class PlacementType : std::uint16_t
		{
			Filters = 0 /** (default) */
		};

		/// @docpublic
		/// @brief Size of the placement to display
		enum class PlacementSize : std::uint16_t
		{
			Small = 0, /** (default) */
			Medium = 1,
			Large = 2
		};

		/// @brief Unique placement id
		std::int64_t ID = 1;

		/// @brief Placement name
		std::string Name = "";

		/// @brief Map of placement name with localization tag
		std::string NameLocalized {};

		/// @brief Structured filters describing the mods this placement should display
		Modio::FilterParams Filters {};

		/// @brief Placement's display position
		std::uint64_t DisplayPosition = 0;

		/// @brief Total number in the placement to show
		std::uint64_t Total = 10;

		/// @brief Placement type
		PlacementType Type = PlacementType::Filters;

		/// @brief Placement's display size
		PlacementSize Size = PlacementSize::Small;

		/// @brief Placement's enabled status
		bool bEnabled = false;
	};
} // namespace Modio
