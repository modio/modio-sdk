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

#include "modio/core/ModioCoreTypes.h"
#include "modio/detail/ModioTransactional.h"
#include "modio/core/entities/ModioModInfo.h"
#include <set>
#include <atomic>

namespace Modio
{
	class ModCollection;

	/// @docpublic
	/// @brief Enum representing the current state of a mod
	enum class ModState : std::int32_t
	{
		InstallationPending,
		Installed,
		UpdatePending,
		Downloading, // installing - don't save to disk, previous state will be saved instead
		Extracting, // installing- don't save to disk, previous state will be saved instead
		UninstallPending
	};

	constexpr static const char* ModStateToString(Modio::ModState State)
	{
		switch (State)
		{
			case Modio::ModState::InstallationPending:
				return "InstallationPending";
			case Modio::ModState::Installed:
				return "Installed";
			case Modio::ModState::UpdatePending:
				return "UpdatePending";
			case Modio::ModState::Downloading:
				return "Downloading";
			case Modio::ModState::Extracting:
				return "Extracting";
			case Modio::ModState::UninstallPending:
				return "UninstallPending";
			default:
				return "Unknown State";
		}
	}

	/// @docinternal
	/// @brief Serialisation helper for Mod Collection Entry
	class ModCollectionEntryAccessor;
	class ModCollectionEntryConstAccessor;

	/// @docinternal
	/// @brief If the conditions are met, it starts a transaction over the ModCollectionEntry
	void BeginTransactionImpl(Modio::ModCollectionEntry& Entry);

	/// @docinternal
	/// @brief Set the CurrentState to the RollbackState status
	void RollbackTransactionImpl(Modio::ModCollectionEntry& Entry);

	/// @docpublic
	/// @brief Class representing a mod which is installed locally
	class ModCollectionEntry : public Modio::Detail::Transactional<Modio::ModCollectionEntry>
	{
		/// @brief The ID of the mod
		Modio::ModID ID;

		/// @brief The status of the mod, is it pending installation, is it installed but needing an update, etc
		std::atomic<Modio::ModState> CurrentState {Modio::ModState::InstallationPending};

		Modio::Optional<Modio::ModState> RollbackState {};

		/// @brief Mod descriptor from the REST API
		Modio::ModInfo ModProfile {};

		/// @brief Reference counting to allow automatic uninstallation of unused local mods
		std::atomic<uint8_t> LocalUserSubscriptionCount {};
		std::set<Modio::UserID> LocalUserSubscriptions {};

		std::string PathOnDisk {};
		Modio::FileSize SizeOnDisk {};

		/// @docinternal
		/// @brief flag if this mod should not be processed because it encountered an unrecoverable error during
		/// installation/update. This should not be saved to disk as we will more than likely want to retry next session
		std::atomic<bool> ShouldNotRetry {};

		Modio::ErrorCode NeverRetryReason {};

		Modio::ErrorCode LastErrorCode {};

		std::uint8_t RetriesRemainingThisSession {};

		/// @docnone
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

		friend class ModCollectionEntryAccessor;
		friend class ModCollectionEntryConstAccessor;

	public:

		/// @docinternal
		/// @brief Default constructor
		ModCollectionEntry() = default;

		/// @docinternal
		/// @brief Constructor creating a ModCollection entry for the specified mod profile. Sets the default state to
		/// InstallationPending
		/// @param ProfileData Mod profile to create a collection entry for
		MODIOSDK_API ModCollectionEntry(Modio::ModInfo ProfileData, std::string CalculatedModPath);

		/// @docinternal
		/// @brief ModCollectionEntry copy constructor
		MODIOSDK_API ModCollectionEntry(const Modio::ModCollectionEntry& Other);

		/// @docnone
		MODIOSDK_API Modio::ModCollectionEntry& operator=(const Modio::ModCollectionEntry& Other);

		/// @docinternal
		/// @brief Gets the number of retries remaining this session
		/// @returns Integer number of retries remaining
		MODIOSDK_API std::uint8_t GetRetriesRemaining();

		/// @docinternal
		/// @brief Updates the associated profile data in the collection entry, marks the mod as requiring an update if
		/// necessary
		/// @param ProfileData New profile data
		MODIOSDK_API void UpdateModProfile(Modio::ModInfo ProfileData);

		/// @docinternal
		/// @brief Increases the local user subscription count for the associated mod
		/// @return the new number of local user subscriptions
		MODIOSDK_API std::uint8_t AddLocalUserSubscription(Modio::Optional<Modio::User> User);

		/// @docinternal
		/// @brief Decrements the local user subscription count for the associated mod. Use with care - this
		/// subscription count is used to determine if a mod requires uninstallation from local storage
		/// @return The updated user subscription count
		MODIOSDK_API std::uint8_t RemoveLocalUserSubscription(Modio::Optional<Modio::User> User);

		/// @docinternal
		/// @brief Updates the state of the associated mod - this is used by the mod management subsystem to determine
		/// if a mod requires install/uninstall/update
		/// @param NewState The new state the mod should have
		MODIOSDK_API void SetModState(Modio::ModState NewState);

		/// @docinternal
		/// @brief Marks the mod as having encountered an error that means installation or updates should not be
		/// reattempted this session
		MODIOSDK_API void MarkModNoRetryThisSession();

		/// @docinternal
		/// @brief Updates the error status on this mod and internal variables determining if the mod should have the
		/// relevant operation retried
		MODIOSDK_API void SetLastError(Modio::ErrorCode Reason);

		/// @docinternal
		/// @brief Clears the no retry flag - this probably won't ever get called but may if we want to retry everything
		/// after free space has been made or something similar
		/// We could probably do something cleaner with logic about which types of errors occurred and more selectively
		/// clear flags, but that can wait until the error code overhaul.
		MODIOSDK_API void ClearModNoRetry();

		/// @docinternal
		/// @brief Fetches the retry flag for this mod
		/// @return if the mod's last error was one that permits retries
		MODIOSDK_API bool ShouldRetry();

		/// @docinternal
		/// @return Modio::ModState enum representing current state of the mod
		MODIOSDK_API Modio::ModState GetModState() const;

		/// @docpublic
		/// @return Mod ID
		MODIOSDK_API Modio::ModID GetID() const;

		/// @docpublic
		/// @return Modio::ModInfo containing mod profile data
		MODIOSDK_API const Modio::ModInfo& GetModProfile() const;

		/// @docpublic
		/// @return Path to the mod's installation folder on disk
		/// NOTE: If the mod is not yet installed this path may not yet exist. Check
		/// @doc_xref\{ModCollectionEntry::GetModState} before trying to load files in this location
		MODIOSDK_API std::string GetPath() const;

		/// @docpublic
		/// @return Size on disk of the mod
		MODIOSDK_API Modio::FileSize GetRawSizeOnDisk() const;

		/// @docpublic
		/// @return Size on disk if the mod has been installed, or empty optional if installation is in progress
		MODIOSDK_API Modio::Optional<Modio::FileSize> GetSizeOnDisk() const;

		/// @docpublic
		/// @return The local user subscriptions
		MODIOSDK_API std::set<Modio::UserID> GetLocalUserSubscriptions() const;

		/// @docpublic
		/// @return The reason for never retrying
		MODIOSDK_API Modio::ErrorCode GetNeverRetryReason() const;

		/// @docinternal
		/// @brief Updates the size of the mod on disk in the collection entry. Called by the archive extraction code on
		/// successful extraction/installation
		/// @param NewSize The total size on disk of all files in the mod
		MODIOSDK_API void UpdateSizeOnDisk(Modio::FileSize NewSize);

		/// @docinternal
		/// @brief Updates the path of the mod on disk in the collection entry. 
		/// @param NewPath The new path for the mod
		MODIOSDK_API void UpdateModPath(std::string NewPath);

		/// @docpublic
		/// @return Modio::ErrorCode The last error that occurred for this mod
		MODIOSDK_API Modio::ErrorCode GetLastError() const;

		/// @docinternal
		/// @brief If the conditions are met, it starts a transaction over the ModCollectionEntry
		friend void Modio::BeginTransactionImpl(Modio::ModCollectionEntry& Entry);

		/// @docinternal
		/// @brief Set the CurrentState to the RollbackState status
		friend void Modio::RollbackTransactionImpl(Modio::ModCollectionEntry& Entry);
	};

	// TODO: @modio-core refactor ModProgressInfo to expose Total, Current, and State (hiding internal members)

	/// @docpublic
	/// @brief Class representing the progress of a mod installation or update
	class ModProgressInfo
	{
	public:

		/// @docpublic
		/// @brief Enum representing which state the currently processing mod is in
		enum class EModProgressState : std::int32_t
		{
			Initializing, // Download information is being retrieved from mod.io servers
			Downloading, // Mod archive is downloading from mod.io servers
			Extracting, // Mod archive is downloaded and now extracting
			Compressing, // Mod archive is being compressed from files on disk
			Uploading // Mod archive is uploading to mod.io servers
		};

		/// @docpublic
		/// @brief Retrieves the state of the currently processing mod
		/// @return Enum value indicating the current state
		Modio::ModProgressInfo::EModProgressState GetCurrentState() const
		{
			return CurrentState;
		}

		/// @docpublic
		/// @brief Retrieves the progress value for the specified state. CurrentProgress == TotalProgress for states
		/// which have completed, for example if a mod is currently Extracting, then passing in Downloading would give
		/// you a value equal to the total download size
		/// @param State which state to query progress information for
		/// @return Modio::FileSize for current progress in bytes
		Modio::FileSize GetCurrentProgress(Modio::ModProgressInfo::EModProgressState State) const
		{
			switch (State)
			{
				case EModProgressState::Initializing:
					return Modio::FileSize(0);
				case EModProgressState::Downloading:
					return DownloadCurrent;
				case EModProgressState::Extracting:
					return ExtractCurrent;
				case EModProgressState::Uploading:
					return UploadCurrent;
				case EModProgressState::Compressing:
					return CompressCurrent;
				default:
					return Modio::FileSize(0);
			}
		}

		/// @docpublic
		/// @brief Retrieves the total amount of progress required for the specified state.
		/// @param State which state to query total progress for
		/// @return Modio::FileSize for total progress in bytes
		Modio::FileSize GetTotalProgress(Modio::ModProgressInfo::EModProgressState State) const
		{
			switch (State)
			{
				case EModProgressState::Initializing:
					return Modio::FileSize(0);
				case EModProgressState::Downloading:
					return DownloadTotal;
				case EModProgressState::Extracting:
					return ExtractTotal;
				case EModProgressState::Uploading:
					return UploadTotal;
				case EModProgressState::Compressing:
					return CompressTotal;
				default:
					return Modio::FileSize(0);
			}
		}

		/// @brief The mod ID of the mod being processed
		Modio::ModID ID;

		/// @docinternal
		/// @brief Default constructor
		ModProgressInfo(Modio::ModID ID)
			: ID(ID),
			  DownloadCurrent(0),
			  DownloadTotal(0),
			  ExtractCurrent(0),
			  ExtractTotal(0),
			  UploadCurrent(0),
			  UploadTotal(0),
			  CurrentState(EModProgressState::Initializing) {}

	private:
		/// @docnone
		Modio::FileSize DownloadCurrent {};
		/// @docnone
		Modio::FileSize DownloadTotal {};
		/// @docnone
		Modio::FileSize ExtractCurrent {};
		/// @docnone
		Modio::FileSize ExtractTotal {};
		/// @docnone
		Modio::FileSize CompressCurrent {};
		/// @docnone
		Modio::FileSize CompressTotal {};
		/// @docnone
		Modio::FileSize UploadCurrent {};
		/// @docnone
		Modio::FileSize UploadTotal {};
		/// @docnone
		Modio::ModProgressInfo::EModProgressState CurrentState {};

		/// @docnone
		friend MODIO_IMPL void SetState(Modio::ModProgressInfo& Info, Modio::ModProgressInfo::EModProgressState State);
		
		/// @docnone
		friend MODIO_IMPL void SetCurrentProgress(Modio::ModProgressInfo& Info, Modio::FileSize NewValue);
		
		/// @docnone
		friend MODIO_IMPL void IncrementCurrentProgress(Modio::ModProgressInfo& Info, Modio::FileSize NewValue);
		
		/// @docnone
		friend MODIO_IMPL void CompleteProgressState(Modio::ModProgressInfo& Info,
													 Modio::ModProgressInfo::EModProgressState State);
		
		/// @docnone
		friend MODIO_IMPL void SetTotalProgress(Modio::ModProgressInfo& Info,
												Modio::ModProgressInfo::EModProgressState State,
												Modio::FileSize NewTotal);
	};

	/// @docnone
	MODIO_IMPL void SetState(Modio::ModProgressInfo& Info, Modio::ModProgressInfo::EModProgressState State);

	/// @docnone
	MODIO_IMPL void SetCurrentProgress(Modio::ModProgressInfo& Info, Modio::FileSize NewValue);

	/// @docnone
	MODIO_IMPL void IncrementCurrentProgress(Modio::ModProgressInfo& Info, Modio::FileSize NewValue);

	/// @docnone
	MODIO_IMPL void CompleteProgressState(Modio::ModProgressInfo& Info,
												 Modio::ModProgressInfo::EModProgressState State);

	/// @docnone
	MODIO_IMPL void SetTotalProgress(Modio::ModProgressInfo& Info,
											Modio::ModProgressInfo::EModProgressState State, Modio::FileSize NewTotal);

	class BaseModList
	{
	public:
		/// @docinternal
		/// @brief Constructs an empty subscription list
		MODIOSDK_API BaseModList();

		/// @docinternal
		/// @brief Constructs a User subscription list by copying Mod IDs from the provided vector
		/// @param NewIDs The source Mod IDs to use
		MODIOSDK_API BaseModList(std::vector<Modio::ModID> NewIDs);

		/// @docinternal
		/// @brief Constructs a User subscription list by consuming Mod IDs from the provided vector
		/// @param NewIDs The source Mod IDs to use
		MODIOSDK_API BaseModList(std::vector<Modio::ModID>&& NewIDs);

		/// @docinternal
		/// @brief Adds a new mod to the subscription list
		/// @param Mod Mod info object to add
		/// @return True if the mod was added, false if already in the list
		MODIOSDK_API bool AddMod(Modio::ModInfo Mod);

		/// @docinternal
		/// @brief Removes a mod from the subscription list
		/// @param Mod Mod ID to remove from the list
		MODIOSDK_API void RemoveMod(Modio::ModID Mod);

		/// @docinternal
		/// @brief Retrieve a set of ModID
		MODIOSDK_API const std::set<Modio::ModID>& Get() const;

		/// @docinternal
		/// @brief Retrieve a set of ModID
		MODIOSDK_API std::set<Modio::ModID>& Get();

		/// @docnone
		friend bool operator==(const Modio::BaseModList& A, const Modio::BaseModList& B)
		{
			// Written for Test_JsonToAndFrom.cpp, re-check functionality before using in actual code. This operator
			// enforces ordering.
			return (A.InternalList == B.InternalList);
		}

	protected:
		std::set<Modio::ModID> InternalList {};
	};

	/// @docpublic
	/// @brief Enum indicating if a mod was added or removed when calculating the difference of two user
	/// subscription lists
	enum class UserSubscriptionListChangeType : std::int32_t
	{
		Added,
		Removed,
		Updated
	};

	/// @docpublic
	/// @brief Class containing the mod IDs the current user is subscribed to
	class UserSubscriptionList : public Modio::BaseModList
	{
	public:
		/// @docpublic
		/// @brief Enum indicating if a mod was added or removed when calculating the difference of two user
		/// subscription lists
		using ChangeType = UserSubscriptionListChangeType;

		/// @docinternal
		/// @brief Calculates removals or additions between the two user subscription lists
		/// @param Original The original list of subscriptions
		/// @param Updated The updated list of subscriptions
		/// @return Map containing all CHANGED mod IDs, with ChangeType indicating if the change was an addition or
		/// removal
		MODIOSDK_API static std::map<Modio::ModID, Modio::UserSubscriptionListChangeType> CalculateChanges(
			const Modio::UserSubscriptionList& Original,
			const Modio::UserSubscriptionList& Updated);

		/// @docinternal
		/// @brief Calculates updates between the ModInfoList and a ModCollection
		/// @param Baseline List of mods comming from the mod.io service
		/// @param Collection List of current subscriptions
		/// @return Map containing all mod ID that have updates in their ModInfo
		MODIOSDK_API static std::map<Modio::ModID, Modio::UserSubscriptionListChangeType> CalculateUpdates(
			const Modio::ModInfoList& Baseline,
			const Modio::ModCollection& Collection);
	};

	// vector of these for the log

	/// @docpublic
	/// @brief Simple struct representing the outcome of a mod management operation
	struct ModManagementEvent
	{
		/// @docpublic
		/// @brief What type of event occurred
		enum class EventType : std::uint8_t
		{
			BeginInstall, /** Mod event started to install a mod **/
			Installed, /** Mod installation to local storage **/
			BeginUninstall, /** Mod event started to uninstall a mod **/
			Uninstalled, /** Mod uninstallation from local storage **/
			BeginUpdate, /** Mod event started to update a mod **/
			Updated, /** Mod local installation updated to latest version*/
			BeginUpload, /** Mod event started to upload a mod **/
			Uploaded /** Mod event that upload completed **/
		};

		/// @docnone
		constexpr static const char* ModManagementEventToString(Modio::ModManagementEvent::EventType Event)
		{
			switch (Event)
			{
				case EventType::BeginInstall:
					return "BeginInstall";
				case EventType::Installed:
					return "Installed";
				case EventType::BeginUninstall:
					return "BeginUninstall";
				case EventType::Uninstalled:
					return "Uninstalled";
				case EventType::BeginUpdate:
					return "BeginUpdate";
				case EventType::Updated:
					return "Updated";
				case EventType::BeginUpload:
					return "BeginUpload";
				case EventType::Uploaded:
					return "Uploaded";
				default:
					return "UNKNOWN";
			}
		}

		/// @brief ID for the mod that the event occurred on
		Modio::ModID ID {};

		/// @brief What type of event occurred
		Modio::ModManagementEvent::EventType Event {};

		/// @brief Empty if operation completed successfully, truthy/contains error code if operation failed
		Modio::ErrorCode Status {};
	};

	/// @docnone
	inline auto format_as(Modio::ModManagementEvent::EventType EnumValue)
	{
		// return static_cast<std::underlying_type_t<Modio::ModManagementEvent::EventType>>(EnumValue);
		return ModManagementEvent::ModManagementEventToString(EnumValue);
	}

	/// @docpublic
	/// @brief Container of multiple mods organized by Modio::ModID
	class ModCollection
	{
	public:
		/// @docinternal
		/// @brief Cloning constructor
		/// @param Entries Raw entries from another mod collection
		MODIOSDK_API ModCollection(std::map<Modio::ModID, std::shared_ptr<Modio::ModCollectionEntry>> Entries);

		/// @docinternal
		/// @brief Default constructor
		MODIOSDK_API ModCollection() : ModEntries() {}
		/// @docpublic
		/// @brief Retrieve a ModCollection given a UserSubscriptionList
		/// @param UserSubscriptions patterns that would entitle a filter
		/// @return ModCollection filled with patterns from UserSubscriptions or empty if nothing is found
		MODIOSDK_API const Modio::ModCollection FilterByUserSubscriptions(const Modio::UserSubscriptionList& UserSubscriptions) const;

		/// @docpublic
		/// @brief Either inserts a mod into the collection or updates its associated profile
		/// @param ModToAdd The mod to insert or update
		/// @param CalculatedModPath The new path to the mod on disk
		/// @return True of mod was inserted, false if mod was updated
		MODIOSDK_API bool AddOrUpdateMod(Modio::ModInfo ModToAdd, std::string CalculatedModPath);

		/// @docpublic
		/// @brief Updates a mod in the collection if it exists, otherwise does nothing
		/// @param ModToUpdate The mod to update
		/// @param CalculatedModPath The new path to the mod on disk
		/// @return True of mod was updated, false if mod was not present
		MODIOSDK_API bool UpdateMod(Modio::ModInfo ModToUpdate, std::string CalculatedModPath);

		/// @docpublic
		/// @brief Updates the mod path in the mod collection if it exists, otherwise does nothing
		/// @param ModToUpdate The mod to update
		/// @param CalculatedModPath The new path to the mod on disk
		/// @return True of mod was updated, false if mod was not present
		MODIOSDK_API bool UpdateModPath(Modio::ModID ModToUpdate, std::string CalculatedModPath);

		/// @docpublic
		/// @brief Retrieve a dictionary of ModID - ModCollectionEntry stored in this ModCollection
		/// @return Dictionary where keys are ModID and values are ModCollectionEntry
		MODIOSDK_API const std::map<Modio::ModID, std::shared_ptr<Modio::ModCollectionEntry>>& Entries() const;

		/// @docpublic
		/// @brief Retrieve a dictionary of ModID - ModCollectionEntry stored in this ModCollection
		/// @return Dictionary where keys are ModID and values are ModCollectionEntry
		MODIOSDK_API std::map<Modio::ModID, std::shared_ptr<Modio::ModCollectionEntry>>& Entries();

		/// @docpublic
		/// @brief Retrieve a single ModCollectionEntry if one is found using a ModID
		/// @return A ModCollectionEntry when the ModCollection finds it using a ModID, otherwise empty
		MODIOSDK_API Modio::Optional<Modio::ModCollectionEntry&> GetByModID(Modio::ModID ModId) const;

		/// @docpublic
		/// @brief Remove a ModCollectionEntry inside the ModCollection indexed by ModID
		/// @return True when the mod was found and removed, otherwise false
		MODIOSDK_API bool RemoveMod(Modio::ModID ModId, bool bForce = false);

		/// @docpublic
		/// @brief Retrieved a sorted ModCollectionEntry vector by priority
		/// @return Vector with ModCollectionEntry
		MODIOSDK_API std::vector<std::shared_ptr<Modio::ModCollectionEntry>> SortEntriesByRetryPriority() const;

		/// @docnone
		friend bool operator==(const Modio::ModCollection& A, const Modio::ModCollection& B)
		{
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

	private:
		std::map<Modio::ModID, std::shared_ptr<Modio::ModCollectionEntry>> ModEntries;
	};

	/// @docpublic
	/// @brief Container of events directly related to a ModID, in particular any error occurred
	/// during execution
	class ModEventLog
	{
	public:
		/// @docpublic
		/// @brief Adds a new entry to the event log. If a given mod ID has an existing entry, this replaces it,
		/// effectively updating it.
		/// @param Entry The entry to add
		MODIOSDK_API void AddEntry(Modio::ModManagementEvent Entry);

		/// @docpublic
		/// @brief Clears the log of events
		/// @return All entries in the log at the time ClearLog was called
		inline std::vector<Modio::ModManagementEvent> ClearLog()
		{
			auto TmpLog = InternalData;
			InternalData.clear();
			return TmpLog;
		}
		inline std::size_t Size()
		{
			return InternalData.size();
		}

	private:
		std::vector<Modio::ModManagementEvent> InternalData {};
	};

} // namespace Modio

#ifndef MODIO_SEPARATE_COMPILATION
	#include "ModioModCollectionEntry.ipp"
#endif