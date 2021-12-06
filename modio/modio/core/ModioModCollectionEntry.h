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
#include "ModioGeneratedVariables.h"
#include "modio/detail/ModioDefines.h"

#include "modio/core/ModioCoreTypes.h"
#include "modio/core/ModioStdTypes.h"
#include "modio/core/entities/ModioModInfo.h"
#include "modio/detail/ModioConstants.h"
#include "modio/detail/ModioTransactional.h"
#include <atomic>
#include <memory>
#include <nlohmann/json.hpp>
#include <set>

namespace Modio
{
	/// @docpublic
	/// @brief Enum representing the current state of a mod
	enum class ModState
	{
		InstallationPending,
		Installed,
		UpdatePending,
		Downloading, // installing - don't save to disk, previous state will be saved instead
		Extracting, // installing- don't save to disk, previous state will be saved instead
		UninstallPending,
	};

	/// @docpublic
	/// @brief Class representing a mod which is installed locally
	class ModCollectionEntry : public Modio::Detail::Transactional<ModCollectionEntry>
	{
		/// @brief The ID of the mod
		Modio::ModID ID;

		/// @brief The status of the mod, is it pending installation, is it installed but needing an update, etc
		std::atomic<Modio::ModState> CurrentState {Modio::ModState::InstallationPending};

		Modio::Optional<Modio::ModState> RollbackState;

		/// @brief Mod descriptor from the REST API
		ModInfo ModProfile;

		/// @brief Reference counting to allow automatic uninstalltion of unused local mods
		std::atomic<uint8_t> LocalUserSubscriptionCount {};
		std::set<Modio::UserID> LocalUserSubscriptions;

		Modio::filesystem::path PathOnDisk;
		Modio::FileSize SizeOnDisk;

		/// @docinternal
		/// @brief flag if this mod should not be processed because it encountered an unrecoverable error during
		/// installation/update. This should not be saved to disk as we will more than likely want to retry next session
		std::atomic<bool> ShouldNotRetry {};

		Modio::ErrorCode NeverRetryReason;

		uint8_t RetriesRemainingThisSession;

		friend bool operator==(const Modio::ModCollectionEntry& A, const Modio::ModCollectionEntry& B)
		{
			// Note: Operator==()  ignores transient fields ShouldNotRetry and RetriesRemainingThisSession
			if ((A.ID == B.ID) && (A.CurrentState == B.CurrentState) && (A.RollbackState == B.RollbackState) &&
				(A.ModProfile == B.ModProfile) && (A.LocalUserSubscriptionCount == B.LocalUserSubscriptionCount) &&
				(A.LocalUserSubscriptions == B.LocalUserSubscriptions) && (A.PathOnDisk == B.PathOnDisk) &&
				(A.SizeOnDisk == B.SizeOnDisk) && (A.NeverRetryReason == B.NeverRetryReason))
			{
				return true;
			}
			else
			{
				return false;
			}
		}

	public:
		ModCollectionEntry() = default;

		/// @docinternal
		/// @brief Constructor creating a ModCollection entry for the specified mod profile. Sets the default state to
		/// InstallationPending
		/// @param ProfileData Mod profile to create a collection entry for
		MODIO_IMPL ModCollectionEntry(ModInfo ProfileData, Modio::filesystem::path CalculatedModPath);

		MODIO_IMPL ModCollectionEntry(const ModCollectionEntry& Other);

		MODIO_IMPL ModCollectionEntry& operator=(const ModCollectionEntry& Other);

		/// @docinternal
		/// @brief Gets the number of retries remaining this session
		/// @returns Integer number of retries remaining
		MODIO_IMPL uint8_t GetRetriesRemaining();

		/// @docinternal
		/// @brief Updates the associated profile data in the collection entry, marks the mod as requiring an update if
		/// necessary
		/// @param ProfileData New profile data
		MODIO_IMPL void UpdateModProfile(ModInfo ProfileData);

		/// @docinternal
		/// @brief Increases the local user subscription count for the associated mod
		/// @return the new number of local user subscriptions
		MODIO_IMPL uint8_t AddLocalUserSubscription(Modio::Optional<User> User);

		/// @docinternal
		/// @brief Decrements the local user subscription count for the associated mod. Use with care - this
		/// subscription count is used to determine if a mod requires uninstallation from local storage
		/// @return The updated user subscription count
		MODIO_IMPL uint8_t RemoveLocalUserSubscription(Modio::Optional<Modio::User> User);

		/// @docinternal
		/// @brief Updates the state of the associated mod - this is used by the mod management subsystem to determine
		/// if a mod requires install/uninstall/update
		/// @param NewState The new state the mod should have
		MODIO_IMPL void SetModState(Modio::ModState NewState);

		/// @docinternal
		/// @brief Marks the mod as having encountered an error that means installation or updates should not be
		/// reattempted this session
		MODIO_IMPL void MarkModNoRetryThisSession();

		/// @docinternal
		/// @brief Updates the error status on this mod and internal variables determining if the mod should have the
		/// relevant operation retried
		MODIO_IMPL void SetLastError(Modio::ErrorCode Reason);

		/// @docinternal
		/// @brief Clears the no retry flag - this probably won't ever get called but may if we want to retry everything
		/// after free space has been made or something similar
		/// We could probably do something cleaner with logic about which types of errors occurred and more selectively
		/// clear flags, but that can wait until the error code overhaul.
		MODIO_IMPL void ClearModNoRetry();

		/// @docinternal
		/// @brief Fetches the retry flag for this mod
		/// @return if the mod's last error was one that permits retries
		MODIO_IMPL bool ShouldRetry();

		/// @docpublic
		/// @return Modio::ModState enum representing current state of the mod
		MODIO_IMPL Modio::ModState GetModState() const;

		/// @docpublic
		/// @return Mod ID
		MODIO_IMPL Modio::ModID GetID() const;

		/// @docpublic
		/// @return Modio::ModInfo containing mod profile data
		MODIO_IMPL Modio::ModInfo GetModProfile() const;

		/// @docpublic
		/// @return Path to the mod's installation folder on disk
		/// NOTE: If the mod is not yet installed this path may not yet exist. Check
		/// @doc_xref{ModCollectionEntry::GetModState} before trying to load files in this location
		MODIO_IMPL Modio::filesystem::path GetPath() const;

		/// @docpublic
		/// @brief Updates the size of the mod on disk in the collection entry. Called by the archive extraction code on
		/// successful extraction/installation
		/// @param NewSize The total size on disk of all files in the mod
		MODIO_IMPL void UpdateSizeOnDisk(Modio::FileSize NewSize);

		friend MODIO_IMPL void to_json(nlohmann::json& j, const ModCollectionEntry& Entry);

		friend MODIO_IMPL void from_json(const nlohmann::json& j, ModCollectionEntry& Entry);

		friend MODIO_IMPL void BeginTransactionImpl(ModCollectionEntry& Entry);

		friend MODIO_IMPL void RollbackTransactionImpl(ModCollectionEntry& Entry);
	};

	// TODO: @modio-core refactor ModProgressInfo to expose Total, Current, and State (hiding internal members)

	/// @docpublic
	/// @brief Class representing the progress of a mod installation or update
	class ModProgressInfo
	{
	public:
		// requires enum indicating if we're downloading or installing?

		/// @brief Total size of the downloaded file
		Modio::FileSize TotalDownloadSize;

		/// @brief Current amount downloaded in bytes
		Modio::FileSize CurrentlyDownloadedBytes;

		/// @brief Total size on disk when fully extracted
		Modio::FileSize TotalExtractedSizeOnDisk;

		/// @brief Amount of data currently extracted
		Modio::FileSize CurrentlyExtractedBytes;

		/// @brief The mod ID of the mod being processed
		Modio::ModID ID;

		ModProgressInfo(Modio::ModID ID) : ID(ID) {};
	};

	/// @doc_internal
	/// @brief Class containing the mod IDs the current user is subscribed to
	class UserSubscriptionList
	{
	public:
		/// @brief Constructs an empty subscription list
		MODIO_IMPL UserSubscriptionList();

		/// @brief Constructs a User subscription list by copying Mod IDs from the provided vector
		/// @param NewIDs The source Mod IDs to use
		MODIO_IMPL UserSubscriptionList(std::vector<Modio::ModID> NewIDs);

		/// @brief Constructs a User subscription list by consuming Mod IDs from the provided vector
		/// @param NewIDs The source Mod IDs to use
		MODIO_IMPL UserSubscriptionList(std::vector<Modio::ModID>&& NewIDs);

		/// @brief Adds a new mod to the subscription list
		/// @param Mod Mod info object to add
		/// @return True if the mod was added, false if already in the list
		MODIO_IMPL bool AddMod(Modio::ModInfo Mod);

		/// @brief Removes a mod from the subscription list
		/// @param Mod Mod ID to remove from the list
		MODIO_IMPL void RemoveMod(Modio::ModID Mod);

		MODIO_IMPL const std::set<Modio::ModID>& Get() const;

		/// @brief Enum indicating if a mod was added or removed when calculating the difference of two user
		/// subscription lists
		enum class ChangeType
		{
			Added,
			Removed
		};

		/// @brief Calculates removals or additions between the two user subscription lists
		/// @param Original The original list of subscriptions
		/// @param Updated The updated list of subscriptions
		/// @return Map containing all CHANGED mod IDs, with ChangeType indicating if the change was an addition or
		/// removal
		MODIO_IMPL static std::map<Modio::ModID, ChangeType> CalculateChanges(const UserSubscriptionList& Original,
																			  const UserSubscriptionList& Updated);
		friend void to_json(nlohmann::json& j, const UserSubscriptionList& List)
		{
			j = nlohmann::json {Modio::Detail::Constants::JSONKeys::UserSubscriptionList, List.InternalList};
		}
		friend void from_json(const nlohmann::json& j, UserSubscriptionList& List)
		{
			if (j.is_array())
			{
				using nlohmann::from_json;
				from_json(j, List.InternalList);
			}
			else
			{
				Modio::Detail::Logger().Log(
					Modio::LogLevel::Warning, Modio::LogCategory::Core,
					"from_json failed for UserSubscriptionList. List must be an array.");
			}
		}

	private:
		std::set<Modio::ModID> InternalList;

		friend bool operator==(const Modio::UserSubscriptionList& A, const Modio::UserSubscriptionList& B)
		{
			// Written for Test_JsonToAndFrom.cpp, re-check functionality before using in actual code. This operator
			// enforces ordering.
			return (A.InternalList == B.InternalList);
		}
	};

	// vector of these for the log

	/// @docpublic
	/// @brief Simple struct representing the outcome of a mod management operation
	struct ModManagementEvent
	{
		/// @brief What type of event occurred
		enum class EventType
		{
			Installed, /** Mod installation to local storage */
			Uninstalled, /** Mod uninstallation from local storage*/
			Updated, /** Mod local installation updated to latest version*/
			Uploaded
		};

		/// @brief ID for the mod that the event occurred on
		Modio::ModID ID;

		/// @brief What type of event occurred
		EventType Event;

		/// @brief Empty if operation completed successfully, truthy/contains error code if operation failed
		Modio::ErrorCode Status;
	};

	class ModCollection
	{
	public:
		MODIO_IMPL const ModCollection FilterByUserSubscriptions(const UserSubscriptionList& UserSubscriptions) const;

		/// @brief Either inserts a mod into the collection or updates its associated profile
		/// @param ModToAdd The mod to insert or update
		/// @param CalculatedModPath The new path to the mod on disk
		/// @return True of mod was inserted, false if mod was updated
		MODIO_IMPL bool AddOrUpdateMod(Modio::ModInfo ModToAdd, Modio::filesystem::path CalculatedModPath);
		MODIO_IMPL const std::map<Modio::ModID, std::shared_ptr<Modio::ModCollectionEntry>>& Entries();
		MODIO_IMPL Modio::Optional<Modio::ModCollectionEntry&> GetByModID(Modio::ModID ModId) const;
		MODIO_IMPL bool RemoveMod(Modio::ModID ModId);
		MODIO_IMPL std::vector<std::shared_ptr<Modio::ModCollectionEntry>> SortEntriesByRetryPriority() const;

		friend void to_json(nlohmann::json& Json, const Modio::ModCollection& Collection)
		{
			std::vector<Modio::ModCollectionEntry> ResolvedEntries;
			for (auto& Mod : Collection.ModEntries)
			{
				ResolvedEntries.push_back(*(Mod.second));
			}
			Json = {Modio::Detail::Constants::JSONKeys::ModCollection, ResolvedEntries};
		}

		friend void from_json(const nlohmann::json& Json, Modio::ModCollection& Collection)
		{
			std::vector<Modio::ModCollectionEntry> LoadedEntries;
			Modio::Detail::ParseSafe(Json, LoadedEntries, Modio::Detail::Constants::JSONKeys::ModCollection);
			for (Modio::ModCollectionEntry& Entry : LoadedEntries)
			{
				Collection.ModEntries[Entry.GetID()] = std::make_shared<Modio::ModCollectionEntry>(Entry);
			}
		}

	private:
		std::map<Modio::ModID, std::shared_ptr<Modio::ModCollectionEntry>> ModEntries;

		friend bool operator==(const Modio::ModCollection& A, const Modio::ModCollection& B)
		{
			// redo. check length, and every pair in A, range based for, check keys are the same, deref shared pointer
			// and compare
			if (A.ModEntries.size() != B.ModEntries.size())
			{
				return false;
			}
			if (A.ModEntries.empty() && B.ModEntries.empty())
			{
				return true;
			}
			for (auto It = A.ModEntries.begin(); It != A.ModEntries.end(); It++)
			{
				if (B.ModEntries.find(It->first) == B.ModEntries.end())
				{
					return false;
				}
				else if (!(*(A.ModEntries.at(It->first)) == *(B.ModEntries.at(It->first))))
				{
					return false;
				}
			}
			return true;
		}
	};

	class ModEventLog
	{
	public:
		/// @brief Adds a new entry to the event log. If a given mod ID has an existing entry, this replaces it,
		/// effectively updating it.
		/// @param Entry The entry to add
		void AddEntry(Modio::ModManagementEvent Entry)
		{
			InternalData.push_back(std::move(Entry));
		}

		/// @brief Updates the error/status code for a given Mod ID in the log
		/// @param ID The mod to update
		/// @param ec The new status code
		void UpdateStatusCode(Modio::ModID ID, Modio::ErrorCode ec);

		/// @brief Clears the log of events
		/// @return All entries in the log at the time ClearLog was called
		inline std::vector<Modio::ModManagementEvent> ClearLog()
		{
			auto TmpLog = InternalData;
			InternalData.clear();
			return TmpLog;
		}

	private:
		std::vector<Modio::ModManagementEvent> InternalData;
	};

} // namespace Modio

#ifndef MODIO_SEPARATE_COMPILATION
	#include "ModioModCollectionEntry.ipp"
#endif