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
#include "modio/core/ModioLogger.h"
#include "modio/core/ModioErrorCode.h"

namespace Modio
{
	inline void to_json(nlohmann::json& j, const ModCollectionEntry& Entry)
	{
		Modio::ModState EntryState = Entry.CurrentState.load();
		// If the current state is in progress, then we store the previous state. As when loading, the user might
		// not want to start the download/installation during SDK initialization. Instead the progress will be
		// resumed when the mod management loop is enabled
		if (Entry.CurrentState == Modio::ModState::Downloading || Entry.CurrentState == Modio::ModState::Extracting)
		{
			if (Entry.RollbackState.has_value())
			{
				EntryState = *Entry.RollbackState;
			}
			else
			{
				Modio::Detail::Logger().Log(
					Modio::LogLevel::Warning, Modio::LogCategory::ModManagement,
					"Mod {0} is in state Downloading or Extracting without a transaction in "
					"progress. Saving state as InstallationPending",
					Entry.ID);
				EntryState = Modio::ModState::InstallationPending;
			}
		}

		j = nlohmann::json::object(
			{{Modio::Detail::Constants::JSONKeys::ModEntryID, Entry.ID},
			 {Modio::Detail::Constants::JSONKeys::ModEntryProfile, Entry.ModProfile},
			 {Modio::Detail::Constants::JSONKeys::ModEntrySubCount, Entry.LocalUserSubscriptions},
			 {Modio::Detail::Constants::JSONKeys::ModEntryState, EntryState},
			 {Modio::Detail::Constants::JSONKeys::ModSizeOnDisk, Entry.SizeOnDisk},
			 {Modio::Detail::Constants::JSONKeys::ModPathOnDisk, Entry.PathOnDisk},
			 {Modio::Detail::Constants::JSONKeys::ModNeverRetryCode, Entry.NeverRetryReason.value()},
			 {Modio::Detail::Constants::JSONKeys::ModNeverRetryCategory,
			  Modio::Detail::ModioErrorCategoryID(Entry.NeverRetryReason.category())}});
	}

	inline void from_json(const nlohmann::json& j, ModCollectionEntry& Entry)
	{
		Modio::Detail::ParseSafe(j, Entry.ID, Modio::Detail::Constants::JSONKeys::ModEntryID);
		Modio::Detail::ParseSafe(j, Entry.ModProfile, Modio::Detail::Constants::JSONKeys::ModEntryProfile);
		Modio::Detail::ParseSafe(j, Entry.LocalUserSubscriptions,
								 Modio::Detail::Constants::JSONKeys::ModEntrySubCount);
		Modio::Detail::ParseSafe(j, Entry.SizeOnDisk, Modio::Detail::Constants::JSONKeys::ModSizeOnDisk);
		Modio::ModState StateTmp = ModState::InstallationPending;
		Modio::Detail::ParseSafe(j, StateTmp, Modio::Detail::Constants::JSONKeys::ModEntryState);
		Entry.CurrentState.store(StateTmp);
		Modio::Detail::ParseSafe(j, Entry.PathOnDisk, Modio::Detail::Constants::JSONKeys::ModPathOnDisk);
		if (j.contains(Modio::Detail::Constants::JSONKeys::ModNeverRetryCode) &&
			j.contains(Modio::Detail::Constants::JSONKeys::ModNeverRetryCategory))
		{
			Entry.NeverRetryReason = std::error_code(
				j.at(Modio::Detail::Constants::JSONKeys::ModNeverRetryCode).get<uint32_t>(),
				Modio::Detail::GetModioErrorCategoryByID(
					j.at(Modio::Detail::Constants::JSONKeys::ModNeverRetryCategory).get<uint64_t>()));
		}
	}
} // namespace Modio