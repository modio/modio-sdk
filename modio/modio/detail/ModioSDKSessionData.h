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
#include "modio/core/ModioCreateModFileParams.h"
#include "modio/core/ModioCreateSourceFileParams.h"
#include "modio/core/ModioModCollectionEntry.h"
#include "modio/core/entities/ModioModCollection.h"
#include "modio/core/entities/ModioUser.h"
#include "modio/detail/ConcurrentQueueWrapper.h"
#include "modio/detail/Function2Wrapper.h"
#include "modio/detail/HedleyWrapper.h"
#include "modio/detail/userdata/ModioUserDataContainer.h"
#include "modio/detail/userdata/ModioUserProfile.h"

#include <chrono>
#include <map>
#include <memory>
#include <queue>
#include <shared_mutex>
#include <vector>

namespace Modio
{
	struct InitializeOptions;

	namespace Detail
	{
		class Buffer;
		class DynamicBuffer;
		class TemporaryModSet;

		class SDKSessionData
		{
		public:
			/// @brief Initializes the static SDK state with the provided parameters.
			/// @param Options initialization parameters
			/// @return false if the SDK is already initialized, or already running when this is called
			MODIO_IMPL static bool Initialize(const Modio::InitializeOptions& Options);

			MODIO_IMPL static void Deinitialize();
			MODIO_IMPL static void ConfirmInitialize();
			MODIO_IMPL static bool IsInitialized();
			MODIO_IMPL static Modio::GameID CurrentGameID();
			MODIO_IMPL static const Modio::ApiKey& CurrentAPIKey();
			MODIO_IMPL static Modio::Environment GetEnvironment();
			MODIO_IMPL static Modio::Portal GetPortal();

			MODIO_IMPL static bool IsModManagementEnabled();
			MODIO_IMPL static void AllowModManagement();
			MODIO_IMPL static void DisableModManagement();

			MODIO_IMPL static void MarkAsRateLimited(int SecondsDelay);
			MODIO_IMPL static bool IsRateLimited();
			MODIO_IMPL static ModCollection& GetSystemModCollection();
			MODIO_IMPL static ModCollection& GetTempModCollection();
			MODIO_IMPL static std::shared_ptr<Modio::Detail::TemporaryModSet> GetTemporaryModSet();

			MODIO_IMPL static ModCollection FilterSystemModCollectionByUserSubscriptions();
			MODIO_IMPL static void InitializeForUser(Modio::User User, Modio::Detail::OAuthToken AuthToken);
			MODIO_IMPL static void UpdateTokenForExistingUser(Modio::Detail::OAuthToken AuthToken);
			MODIO_IMPL static const Modio::Optional<Modio::Detail::OAuthToken> GetAuthenticationToken();

			MODIO_IMPL static Modio::UserSubscriptionList& GetUserSubscriptions();

			MODIO_IMPL static std::map<Modio::ModID, Modio::ModInfo>& GetModPurchases();

			MODIO_IMPL static Modio::Detail::Buffer SerializeUserData();

			MODIO_IMPL static bool DeserializeUserDataFromBuffer(Modio::Detail::Buffer UserDataBuffer);

			MODIO_IMPL static void ClearUserData();
			MODIO_IMPL static void InvalidateOAuthToken();

			MODIO_IMPL static const Modio::Optional<Modio::User> GetAuthenticatedUser();
			MODIO_IMPL static const Modio::Optional<Modio::Detail::Avatar> GetAuthenticatedUserAvatar();

			MODIO_IMPL static void AddToDeferredUnsubscriptions(Modio::ModID Mod);
			MODIO_IMPL static bool HasDeferredUnsubscriptions();

			MODIO_IMPL static Modio::ModID GetDeferredUnsubscription();

			MODIO_IMPL static void RemoveDeferredUnsubscription(Modio::ModID NoLongerDeferredUnsubscription);

			MODIO_IMPL static Modio::Optional<Modio::filesystem::path> GetUserModDirectoryOverride();

			MODIO_IMPL static void SetUserModManagementCallback(
				std::function<void(Modio::ModManagementEvent)> Callback);

			MODIO_IMPL static void FlushModManagementLog();

			MODIO_IMPL static ModEventLog& GetModManagementEventLog();

			MODIO_IMPL static void SetLastValidationError(std::vector<FieldError> ExtendedErrorInformation);
			MODIO_IMPL static std::vector<FieldError> GetLastValidationError();
			MODIO_IMPL static void ClearLastValidationError();

			MODIO_IMPL static void AddPendingModfileUpload(Modio::ModID ID, Modio::CreateModFileParams Params);

			MODIO_IMPL static void AddPendingSourceFileUpload(Modio::ModID ID, Modio::CreateSourceFileParams Params);
			MODIO_IMPL static Modio::Optional<std::pair<Modio::ModID, Modio::CreateSourceFileParams>>
				GetNextPendingSourceFileUpload();

			/// @brief Retrieves the next pending modfile upload from the queue. *Removes the element from the queue*.
			/// @return The pending upload information, or empty Optional if nothing pending
			MODIO_IMPL static Modio::Optional<std::pair<Modio::ModID, Modio::CreateModFileParams>>
				GetNextPendingModfileUpload();
			MODIO_IMPL static Modio::Optional<std::pair<Modio::ModID, Modio::CreateModFileParams>>
				GetPriorityModfileUpload();
			MODIO_IMPL static bool PrioritizeModfileUpload(Modio::ModID IdToPrioritize);
			MODIO_IMPL static bool PrioritizeModfileDownload(Modio::ModID IdToPrioritize);

			MODIO_IMPL static Modio::Optional<Modio::ModID> GetPriorityModID()
			{
				return Get().ModIDToPrioritize;
			}

			MODIO_IMPL static std::weak_ptr<Modio::Detail::TemporaryModSet> InitTempModSet(
				std::vector<Modio::ModID> ModIds);

			MODIO_IMPL static bool CloseTempModSet();

			/// @brief Initializes a ModProgressInfo for the specified mod, storing it in the global state. This method
			/// is only intended for use by InstallOrUpdateModOp
			/// @param ID Mod ID for the mod to begin reporting progress on
			/// @return Weak pointer to the ModProgressInfo, or nullptr if a mod is already downloading/updating
			MODIO_IMPL static std::weak_ptr<Modio::ModProgressInfo> StartModDownloadOrUpdate(Modio::ModID ID);

			MODIO_IMPL static bool CancelModDownloadOrUpdate(Modio::ModID ID);

			/// @brief Resets the internal ModProgressInfo object such that calls to GetModProgress return an empty
			/// Optional indicating no mods are currently updating or installing. This method is only intended for use
			/// by InstallOrUpdateModOp.
			MODIO_IMPL static void FinishModDownloadOrUpdate();

			/// @brief Fetches a static snapshot of the progress of the current download or update
			/// @return Copy of the progress data for the current download/update, or an empty Optional if no such
			/// operation is in progress
			MODIO_IMPL static Modio::Optional<const Modio::ModProgressInfo> GetModProgress();

			MODIO_IMPL static Modio::ModCreationHandle GetNextModCreationHandle();

			MODIO_IMPL static void SetEnvironmentOverrideUrl(std::string OverrideUrl);
			MODIO_IMPL static Modio::Optional<std::string> GetEnvironmentOverrideUrl();

			MODIO_IMPL static void SetPlatformOverride(std::string Override);
			MODIO_IMPL static Modio::Optional<std::string> GetPlatformOverride();

			MODIO_IMPL static void SetPlatformEnvironment(std::string Environment);
			MODIO_IMPL static Modio::Optional<std::string> GetPlatformEnvironment();

			MODIO_IMPL static void SetPlatformStatusFilter(std::string PendingOnlyResults);
			MODIO_IMPL static Modio::Optional<Modio::PlatformStatus> GetPlatformStatusFilter();
			MODIO_IMPL static std::string GetAcceptanceFilterStringForRequestedPlatformStatus();

			MODIO_IMPL static void SetModStorageQuota(Modio::FileSize ModStorageQuota);
			MODIO_IMPL static Modio::Optional<Modio::FileSize> GetModStorageQuota();
			MODIO_IMPL static Modio::Optional<Modio::FileSize> GetTempModStorageQuota();
			MODIO_IMPL static Modio::ErrorCode SetCacheStorageQuota(Modio::FileSize CacheStorageQuota);
			MODIO_IMPL static Modio::Optional<Modio::FileSize> GetCacheStorageQuota();
			MODIO_IMPL static Modio::ErrorCode CheckCacheQuotaAndClearSpace();
			MODIO_IMPL static Modio::FileSize GetTotalImageCacheSize();
			MODIO_IMPL static Modio::ErrorCode AddToImageCacheData(Modio::filesystem::path NewImagePath);

			MODIO_IMPL static void SetLocalLanguage(Modio::Language Local);
			MODIO_IMPL static Modio::Language GetLocalLanguage();

			MODIO_IMPL static Modio::Optional<Modio::ModID> ResolveModCreationHandle(Modio::ModCreationHandle Handle);
			MODIO_IMPL static void LinkModCreationHandle(Modio::ModCreationHandle Handle, Modio::ModID ID);

			MODIO_IMPL static void InvalidateSubscriptionCache();
			MODIO_IMPL static void ClearSubscriptionCacheInvalid();
			MODIO_IMPL static bool IsSubscriptionCacheInvalid();

			MODIO_IMPL static void InvalidateTermsOfUseCache();
			MODIO_IMPL static void ClearTermsOfUseCache();
			MODIO_IMPL static bool IsTermsOfUseCacheInvalid();

			MODIO_IMPL static void InvalidateAllModsCache();

			MODIO_IMPL static void InvalidatePurchaseCache();
			MODIO_IMPL static void ClearPurchaseCacheInvalid();
			MODIO_IMPL static bool IsPurchaseCacheInvalid();

			MODIO_IMPL static void InvalidateModCache(Modio::ModID ID);
			MODIO_IMPL static void ClearModCacheInvalid(Modio::ModID ID);
			MODIO_IMPL static bool IsModCacheInvalid(Modio::ModID ID);

			MODIO_IMPL static void InvalidateModCollectionCache(Modio::ModCollectionID ID);
			MODIO_IMPL static void ClearModCollectionCacheInvalid(Modio::ModCollectionID ID);
			MODIO_IMPL static bool IsModCollectionCacheInvalid(Modio::ModCollectionID ID);

			MODIO_IMPL static bool IsFetchExternalUpdatesRunning();
			MODIO_IMPL static void SetFetchExternalUpdatesRunning(bool IsRunning);

			MODIO_IMPL static void EnqueueTask(fu2::unique_function<void()> Task);
			MODIO_IMPL static void PushQueuedTasksToGlobalContext();

			MODIO_IMPL static std::shared_timed_mutex& GetRWMutex();
			MODIO_IMPL static std::shared_timed_mutex& GetShutdownMutex();

			MODIO_NODISCARD MODIO_IMPL static std::shared_lock<std::shared_timed_mutex> GetReadLock();
			MODIO_NODISCARD MODIO_IMPL static std::unique_lock<std::shared_timed_mutex> GetWriteLock();
			MODIO_NODISCARD MODIO_IMPL static std::unique_lock<std::shared_timed_mutex> GetShutdownLock();
			MODIO_NODISCARD MODIO_IMPL static std::unique_lock<std::shared_timed_mutex> TryGetShutdownLock();

			MODIO_IMPL static bool HasModManagementEventQueued();
			MODIO_IMPL static void IncrementModManagementEventQueued();
			MODIO_IMPL static void DecrementModManagementEventQueued();

		private:
			enum class InitializationState
			{
				NotInitialized,
				Initializing,
				InitializationComplete
			};

			MODIO_IMPL SDKSessionData(const Modio::InitializeOptions& Options, Modio::Language InLocalLanguage);
			MODIO_IMPL SDKSessionData();

			// This may not need to be public, can probably just expose static accessors that call it
			MODIO_IMPL static SDKSessionData& Get();

			Modio::GameID GameID {};
			Modio::ApiKey APIKey {};
			Modio::Environment Environment = Modio::Environment::Live;
			Modio::Optional<std::string> EnvironmentOverrideUrl {};
			Modio::Optional<std::string> PlatformOverride {};
			Modio::Optional<std::string> PlatformEnvironment {};
			Modio::Optional<Modio::PlatformStatus> PlatformStatusFilter {};
			Modio::Optional<Modio::FileSize> ModStorageQuota {};
			Modio::Optional<Modio::FileSize> CacheStorageQuota {};
			Modio::Portal PortalInUse = Modio::Portal::None;
			Modio::Language LocalLanguage = Modio::Language::English;
			InitializationState CurrentInitializationState = InitializationState::NotInitialized;
			bool bModManagementEnabled = false;
			uint32_t ModManagementEventQueued = 0;
			std::vector<struct FieldError> LastValidationError {};
			// Implemented as shared_ptr because that way operations that need to alter the state of the entry can get a
			// cheap reference to the original without the lack of safety from a potentially dangling raw reference
			std::shared_ptr<Modio::ModProgressInfo> CurrentModInProgress {};
			std::function<void(Modio::ModManagementEvent)> ModManagementEventCallback {};
			Modio::ModCollection SystemModCollection {};
			Modio::ModCollection TempModCollection {};
			// Could be a vector if we need multiple TempModSet
			std::shared_ptr<Modio::Detail::TemporaryModSet> TempModSet {};
			Modio::Detail::UserDataContainer UserData {};
			// We may need to make this a shared pointer and give a reference to operations so if we shut down they
			// write into the stale log instead
			Modio::ModEventLog EventLog {};
			bool bRateLimited = false;
			std::chrono::system_clock::time_point RateLimitedStop {};
			std::map<Modio::ModCreationHandle, Modio::Optional<Modio::ModID>> CreationHandles {};
			Modio::Optional<Modio::ModID> ModIDToPrioritize {};
			std::map<Modio::ModID, Modio::CreateModFileParams> PendingModUploads {};
			std::map<Modio::ModID, Modio::CreateSourceFileParams> PendingSourceUploads {};
			bool bSubscriptionCacheInvalid = false;
			bool bTermsOfUseCacheInvalid = false;
			bool bPurchaseCacheInvalid = false;
			std::unordered_map<std::int64_t, bool> ModCacheInvalidMap {};
			moodycamel::ConcurrentQueue<fu2::unique_function<void()>> IncomingTaskQueue;
			std::map<Modio::ModID, Modio::ModInfo> UserModPurchaseCache {};
			// Only populated if a cache storage quota is set
			std::queue<Modio::filesystem::path> CacheImagePaths {};
			// Only set if a cache storage quota is set
			Modio::FileSize TotalImageCacheSize {};
			bool FetchExternalUpdatesRunning = false;
			std::unordered_map<std::int64_t, bool> CollectionCacheInvalidMap;
		};
	} // namespace Detail
} // namespace Modio

#ifndef MODIO_SEPARATE_COMPILATION
	#include "ModioSDKSessionData.ipp"
#endif
