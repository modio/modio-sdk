/*
 *  Copyright (C) 2021-2024 mod.io Pty Ltd. <https://mod.io>
 *
 *  This file is part of the mod.io SDK.
 *
 *  Distributed under the MIT License. (See accompanying file LICENSE or
 *   view online at <https://github.com/modio/modio-sdk/blob/main/LICENSE>)
 *
 */

#pragma once

#include "modio/core/ModioModCollectionEntry.h"
#include "modio/detail/ModioConstants.h"
#include "modio/detail/ModioJsonHelpers.h"
#include "modio/detail/serialization/ModioModCollectionEntrySerialization.h"

namespace Modio
{
	inline void to_json(nlohmann::json& Json, const Modio::ModCollection& Collection)
	{
		std::vector<Modio::ModCollectionEntry> ResolvedEntries;
		for (auto& Mod : Collection.Entries())
		{
			ResolvedEntries.push_back(*(Mod.second));
		}
		Json = {Modio::Detail::Constants::JSONKeys::ModCollection, ResolvedEntries};
	}

	inline void from_json(const nlohmann::json& Json, Modio::ModCollection& Collection)
	{
		std::vector<Modio::ModCollectionEntry> LoadedEntries;
		Modio::Detail::ParseSafe(Json, LoadedEntries, Modio::Detail::Constants::JSONKeys::ModCollection);
		for (Modio::ModCollectionEntry& Entry : LoadedEntries)
		{
			Collection.Entries()[Entry.GetID()] = std::make_shared<Modio::ModCollectionEntry>(Entry);
		}
	}
} // namespace Modio
