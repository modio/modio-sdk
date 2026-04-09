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

#include "modio/core/ModioErrorCode.h"
#include "modio/core/ModioLogger.h"
#include "modio/detail/ModioConstants.h"
#include "modio/detail/ModioJsonHelpers.h"
#include "modio/detail/serialization/ModioModInfoSerialization.h"

namespace Modio
{
	/// @docinternal
	/// @brief Serialisation helper class
	class ModCollectionEntryConstAccessor
	{
		const ModCollectionEntry& Entry;

	public:
		/// @docinternal
		ModCollectionEntryConstAccessor(const ModCollectionEntry& Entry) : Entry(Entry) {}
		
		/// @docinternal
		const std::atomic<Modio::ModState>& CurrentState() const
		{
			return Entry.CurrentState;
		}
		/// @docinternal
		const Modio::Optional<Modio::ModState>& RollbackState() const
		{
			return Entry.RollbackState;
		}
	};

	/// @docinternal
	/// @brief Serialisation helper class
	class ModCollectionEntryAccessor
	{
		ModCollectionEntry& Entry;

	public:
		/// @docinternal
		ModCollectionEntryAccessor(ModCollectionEntry& Entry) : Entry(Entry) {}

		/// @docinternal
		/// @return Modio::ModState enum representing current state of the mod
		std::atomic<Modio::ModState>& GetCurrentState() 
		{
			return Entry.CurrentState;
		}

		/// @docinternal
		/// @return Mod ID
		Modio::ModID& GetID()
		{
			return Entry.ID;
		}

		/// @docinternal
		/// @return Modio::ModInfo containing mod profile data
		Modio::ModInfo& GetModProfile()
		{
			return Entry.ModProfile;
		}

		/// @docinternal
		/// @return Path to the mod's installation folder on disk
		/// NOTE: If the mod is not yet installed this path may not yet exist. Check
		/// @doc_xref{ModCollectionEntry::GetModState} before trying to load files in this location
		std::string& GetPath()
		{
			return Entry.PathOnDisk;
		}

		/// @docinternal
		/// @return Size on disk of the mod
		Modio::FileSize& GetRawSizeOnDisk()
		{
			return Entry.SizeOnDisk;
		}

		/// @docinternal
		/// @return The local user subscriptions
		std::set<Modio::UserID>& GetLocalUserSubscriptions()
		{
			return Entry.LocalUserSubscriptions;
		}

		/// @docinternal
		/// @return The reason for never retrying
		Modio::ErrorCode& GetNeverRetryReason()
		{
			return Entry.NeverRetryReason;
		}
	};

	inline void to_json(nlohmann::json& j, const ModCollectionEntry& Entry)
	{
		Modio::ModCollectionEntryConstAccessor Helper(Entry);
		Modio::ModState EntryState = Helper.CurrentState().load();
		// If the current state is in progress, then we store the previous state. As when loading, the user might
		// not want to start the download/installation during SDK initialization. Instead the progress will be
		// resumed when the mod management loop is enabled
		if (Helper.CurrentState() == Modio::ModState::Downloading ||
			Helper.CurrentState() == Modio::ModState::Extracting)
		{
			if (Helper.RollbackState().has_value())
			{
				EntryState = *Helper.RollbackState();
			}
			else
			{
				Modio::Detail::Logger().Log(Modio::LogLevel::Warning, Modio::LogCategory::ModManagement,
											"Mod {0} is in state Downloading or Extracting without a transaction in "
											"progress. Saving state as InstallationPending",
											Entry.GetID());
				EntryState = Modio::ModState::InstallationPending;
			}
		}

		j = nlohmann::json::object(
			{{Modio::Detail::Constants::JSONKeys::ModEntryID, Entry.GetID()},
			 {Modio::Detail::Constants::JSONKeys::ModEntryProfile, Entry.GetModProfile()},
			 {Modio::Detail::Constants::JSONKeys::ModEntrySubCount, Entry.GetLocalUserSubscriptions()},
			 {Modio::Detail::Constants::JSONKeys::ModEntryState, EntryState},
			 {Modio::Detail::Constants::JSONKeys::ModSizeOnDisk, Entry.GetRawSizeOnDisk()},
			 {Modio::Detail::Constants::JSONKeys::ModPathOnDisk, Entry.GetPath()},
			 {Modio::Detail::Constants::JSONKeys::ModNeverRetryCode, Entry.GetNeverRetryReason().value()},
			 {Modio::Detail::Constants::JSONKeys::ModNeverRetryCategory,
			  Modio::Detail::ModioErrorCategoryID(Entry.GetNeverRetryReason().category())}});
	}

	inline void from_json(const nlohmann::json& j, ModCollectionEntry& Entry)
	{
		Modio::ModCollectionEntryAccessor Helper(Entry);
		Modio::Detail::ParseSafe(j, Helper.GetID(), Modio::Detail::Constants::JSONKeys::ModEntryID);
		Modio::Detail::ParseSafe(j, Helper.GetModProfile(), Modio::Detail::Constants::JSONKeys::ModEntryProfile);
		Modio::Detail::ParseSafe(j, Helper.GetLocalUserSubscriptions(),
								 Modio::Detail::Constants::JSONKeys::ModEntrySubCount);
		Modio::Detail::ParseSafe(j, Helper.GetRawSizeOnDisk(), Modio::Detail::Constants::JSONKeys::ModSizeOnDisk);
		Modio::ModState StateTmp = ModState::InstallationPending;
		Modio::Detail::ParseSafe(j, StateTmp, Modio::Detail::Constants::JSONKeys::ModEntryState);
		Helper.GetCurrentState().store(StateTmp);
		Modio::Detail::ParseSafe(j, Helper.GetPath(), Modio::Detail::Constants::JSONKeys::ModPathOnDisk);
		if (j.contains(Modio::Detail::Constants::JSONKeys::ModNeverRetryCode) &&
			j.contains(Modio::Detail::Constants::JSONKeys::ModNeverRetryCategory))
		{
			Helper.GetNeverRetryReason() =
				std::error_code(j.at(Modio::Detail::Constants::JSONKeys::ModNeverRetryCode).get<int32_t>(),
								Modio::Detail::GetModioErrorCategoryByID(
									j.at(Modio::Detail::Constants::JSONKeys::ModNeverRetryCategory).get<uint64_t>()));
		}
	}
} // namespace Modio