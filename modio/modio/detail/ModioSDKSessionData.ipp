/*
 *  Copyright (C) 2021 mod.io Pty Ltd. <https://mod.io>
 *
 *  This file is part of the mod.io SDK.
 *
 *  Distributed under the MIT License. (See accompanying file LICENSE or
 *   view online at <https://github.com/modio/modio-sdk/blob/main/LICENSE>)
 *
 */

#ifdef MODIO_SEPARATE_COMPILATION
	#include "modio/detail/ModioSDKSessionData.h"
#endif

#include "modio/cache/ModioCacheService.h"
#include "modio/core/ModioBuffer.h"
#include "modio/core/ModioInitializeOptions.h"
#include "modio/core/ModioLogger.h"
#include "modio/core/ModioTemporaryModSet.h"
#include "modio/detail/HedleyWrapper.h"
#include "modio/detail/serialization/ModioTokenSerialization.h"
#include "modio/detail/serialization/ModioUserDataContainerSerialization.h"
#include "modio/detail/serialization/ModioAvatarSerialization.h"

MODIO_DIAGNOSTIC_PUSH

MODIO_ALLOW_DEPRECATED_SYMBOLS

namespace Modio

{
	namespace Detail
	{
		SDKSessionData& SDKSessionData::Get()
		{
			static SDKSessionData Instance;
			return Instance;
		}

		MODIO_IMPL bool SDKSessionData::Initialize(const Modio::InitializeOptions& Options)
		{
			// If we are already initializing or done with init, bail
			if (Get().CurrentInitializationState != InitializationState::NotInitialized)
			{
				return false;
			}
			else
			{
				Get() = SDKSessionData(Options, Get().LocalLanguage);
				Get().CurrentInitializationState = InitializationState::Initializing;
				return true;
			}
		}

		void SDKSessionData::Deinitialize()
		{
			Get().FlushModManagementLog();
			auto Lock = Modio::Detail::SDKSessionData::GetWriteLock();
			Get().CurrentInitializationState = InitializationState::NotInitialized;
		}

		void SDKSessionData::ConfirmInitialize()
		{
			auto Lock = Modio::Detail::SDKSessionData::GetWriteLock();
			Get().CurrentInitializationState = InitializationState::InitializationComplete;
		}

		bool SDKSessionData::IsInitialized()
		{
			return Get().CurrentInitializationState == InitializationState::InitializationComplete;
		}

		Modio::GameID SDKSessionData::CurrentGameID()
		{
			return Get().GameID;
		}

		const Modio::ApiKey& SDKSessionData::CurrentAPIKey()
		{
			return Get().APIKey;
		}

		Modio::Environment SDKSessionData::GetEnvironment()
		{
			return Get().Environment;
		}

		Modio::Portal SDKSessionData::GetPortal()
		{
			return Get().PortalInUse;
		}

		bool SDKSessionData::IsModManagementEnabled()
		{
			return Get().bModManagementEnabled;
		}

		void SDKSessionData::AllowModManagement()
		{
			Get().bModManagementEnabled = true;
		}

		void SDKSessionData::DisableModManagement()
		{
			Modio::Detail::Logger().Log(LogLevel::Info, Modio::LogCategory::Core, "Mod Management has been disabled.");

			Get().bModManagementEnabled = false;
			if (Get().CurrentModInProgress != nullptr)
			{
				Get().CurrentModInProgress.reset();
			}
		}

		void SDKSessionData::MarkAsRateLimited(int SecondsDelay)
		{
			// In case we already have set bRateLimited as true,
			// prevent the clock from restarting
			if (Get().bRateLimited == true)
			{
				return;
			}

			Get().bRateLimited = true;
			Get().RateLimitedStop = std::chrono::system_clock::now() + std::chrono::seconds(SecondsDelay);
		}

		bool SDKSessionData::IsRateLimited()
		{
			std::chrono::system_clock::time_point Current = std::chrono::system_clock::now();
			auto Diff = (Get().RateLimitedStop - Current);

			// Compare the RateLimitedStop if it has passed more than the Current time
			// Diff will be negative when Current has passed RateLimitedStop
			if (Diff.count() <= 0)
			{
				// More than "Timeout" has happened since the last request, disable
				// rate limiting boolean
				Get().bRateLimited = false;
			}

			return Get().bRateLimited;
		}

		Modio::ModCollection& SDKSessionData::GetSystemModCollection()
		{
			return Get().SystemModCollection;
		}

		Modio::ModCollection& SDKSessionData::GetTempModCollection()
		{
			return Get().TempModCollection;
		}

		std::shared_ptr<Modio::Detail::TemporaryModSet> SDKSessionData::GetTemporaryModSet()
		{
			return Get().TempModSet;
		}

		Modio::ModCollection SDKSessionData::FilterSystemModCollectionByUserSubscriptions()
		{
			MODIO_PROFILE_SCOPE(FilterSystemModsByUserSubs);
			return Get().SystemModCollection.FilterByUserSubscriptions(Get().UserData.UserSubscriptions);
		}

		void SDKSessionData::InitializeForUser(Modio::User AuthenticatedUser, Modio::Detail::OAuthToken AuthToken)
		{
			auto Lock = Modio::Detail::SDKSessionData::GetWriteLock();
			Get().UserData.InitializeForUser(std::move(AuthenticatedUser), std::move(AuthToken));
		}

		void SDKSessionData::UpdateTokenForExistingUser(Modio::Detail::OAuthToken AuthToken)
		{
			auto Lock = Modio::Detail::SDKSessionData::GetWriteLock();
			Get().UserData.UpdateTokenForExistingUser(std::move(AuthToken));
		}

		const Modio::Optional<Modio::Detail::OAuthToken> SDKSessionData::GetAuthenticationToken()
		{
			return Get().UserData.GetAuthenticationToken();
		}

		Modio::UserSubscriptionList& SDKSessionData::GetUserSubscriptions()
		{
			return Get().UserData.UserSubscriptions;
		}

		std::map<Modio::ModID, Modio::ModInfo>& SDKSessionData::GetModPurchases()
		{
			return Get().UserModPurchaseCache;
		}

		Modio::Detail::Buffer SDKSessionData::SerializeUserData()
		{
			MODIO_PROFILE_SCOPE(SerializeUserData);
			nlohmann::json Data(Get().UserData);
			Data["version"] = 1;
			std::string UserData = Data.dump();
			Modio::Detail::Buffer DataBuffer(UserData.size());
			std::copy(UserData.begin(), UserData.end(), reinterpret_cast<char*>(DataBuffer.begin()));
			return DataBuffer;
		}

		bool SDKSessionData::DeserializeUserDataFromBuffer(Modio::Detail::Buffer UserDataBuffer)
		{
			auto Lock = Modio::Detail::SDKSessionData::GetWriteLock();
			MODIO_PROFILE_SCOPE(DeserializeUserData);
			nlohmann::json UserDataJson = Modio::Detail::ToJson(std::move(UserDataBuffer));
			from_json(UserDataJson, Get().UserData);
			return Get().UserData.IsValid();
		}

		void SDKSessionData::ClearUserData()
		{
			auto Lock = Modio::Detail::SDKSessionData::GetWriteLock();
			Get().UserData.ResetUserData();
		}

		void SDKSessionData::InvalidateOAuthToken()
		{
			auto Lock = Modio::Detail::SDKSessionData::GetWriteLock();
			Get().UserData.InvalidateOAuthToken();
		}

		const Modio::Optional<Modio::User> SDKSessionData::GetAuthenticatedUser()
		{
			return Get().UserData.GetAuthenticatedUser();
		}

		const Modio::Optional<Modio::Detail::Avatar> SDKSessionData::GetAuthenticatedUserAvatar()
		{
			return Get().UserData.GetAuthenticatedUserAvatar();
		}

		void SDKSessionData::AddToDeferredUnsubscriptions(Modio::ModID Mod)
		{
			Get().UserData.DeferredUnsubscriptions.push_back(Mod);
		}

		bool SDKSessionData::HasDeferredUnsubscriptions()
		{
			return Get().UserData.DeferredUnsubscriptions.size() > 0;
		}

		Modio::ModID SDKSessionData::GetDeferredUnsubscription()
		{
			return Get().UserData.DeferredUnsubscriptions.front();
		}

		void SDKSessionData::RemoveDeferredUnsubscription(Modio::ModID NoLongerDeferredUnsubscription)
		{
			UserDataContainer& UserData = Get().UserData;
			UserData.DeferredUnsubscriptions.erase(std::remove(UserData.DeferredUnsubscriptions.begin(),
															   UserData.DeferredUnsubscriptions.end(),
															   NoLongerDeferredUnsubscription),
												   UserData.DeferredUnsubscriptions.end());
		}

		Modio::Optional<Modio::filesystem::path> SDKSessionData::GetUserModDirectoryOverride()
		{
			return Get().UserData.UserModDirectoryOverride;
		}

		void SDKSessionData::SetUserModManagementCallback(std::function<void(Modio::ModManagementEvent)> Callback)
		{
			Get().ModManagementEventCallback = Callback;
		}

		void SDKSessionData::FlushModManagementLog()
		{
			if (auto&& Callback = Get().ModManagementEventCallback)
			{
				if (Get().EventLog.Size())
				{
					std::vector<ModManagementEvent> Events;
					{
						auto Lock = Modio::Detail::SDKSessionData::GetWriteLock();
						Events = Get().EventLog.ClearLog();
					}
					for (auto&& Event : Events)
					{
						Callback(Event);
					}
				}
			}
		}

		Modio::ModEventLog& SDKSessionData::GetModManagementEventLog()
		{
			return Get().EventLog;
		}

		void SDKSessionData::SetLastValidationError(std::vector<FieldError> ExtendedErrorInformation)
		{
			auto Lock = Modio::Detail::SDKSessionData::GetWriteLock();
			Get().LastValidationError = ExtendedErrorInformation;
		}

		std::vector<FieldError> SDKSessionData::GetLastValidationError()
		{
			return Get().LastValidationError;
		}

		void SDKSessionData::ClearLastValidationError()
		{
			auto Lock = Modio::Detail::SDKSessionData::GetWriteLock();
			Get().LastValidationError.clear();
		}

		void SDKSessionData::AddPendingModfileUpload(Modio::ModID ID, Modio::CreateModFileParams Params)
		{
			Modio::filesystem::path ModRoot = Params.RootDirectory;
			if (ModRoot.has_filename())
			{
				ModRoot /= "";
				Modio::Detail::Logger().Log(
					Modio::LogLevel::Warning, Modio::LogCategory::ModManagement,
					"Modfile directory path {} does not end in a path separator. Adding manually",
					Params.RootDirectory);
				Params.RootDirectory = ModRoot.u8string();
			}

			auto ExistingEntry = Get().PendingModUploads.find(ID);
			if (ExistingEntry != Get().PendingModUploads.end())
			{
				Modio::Detail::Logger().Log(Modio::LogLevel::Warning, Modio::LogCategory::ModManagement,
											"New modfile upload queued for mod {} which already had a pending upload. "
											"Dropping previous modfile upload request.",
											ID);
				Get().PendingModUploads.erase(ID);
			}
			Get().PendingModUploads.insert({ID, Params});
		}

		Modio::Optional<std::pair<Modio::ModID, Modio::CreateModFileParams>> SDKSessionData::GetPriorityModfileUpload()
		{
			if (Get().ModIDToPrioritize.has_value())
			{
				if (Get().PendingModUploads.size())
				{
					auto Iterator = Get().PendingModUploads.find(*Get().ModIDToPrioritize);
					if (Iterator != Get().PendingModUploads.end())
					{
						std::pair<Modio::ModID, Modio::CreateModFileParams> NextPending = *Iterator;
						Get().PendingModUploads.erase(Iterator);
						return NextPending;
					}
				}
			}
			return {};
		}

		Modio::Optional<std::pair<Modio::ModID, Modio::CreateModFileParams>> SDKSessionData::
			GetNextPendingModfileUpload()
		{
			if (Get().PendingModUploads.size())
			{
				std::pair<Modio::ModID, Modio::CreateModFileParams> PendingUpload = *Get().PendingModUploads.begin();
				Get().PendingModUploads.erase(Get().PendingModUploads.begin());
				return PendingUpload;
			}
			else
			{
				return {};
			}
		}

		bool SDKSessionData::PrioritizeModfileUpload(Modio::ModID IdToPrioritize)
		{
			// Checks the mod is a pending upload.
			if (Get().PendingModUploads.size())
			{
				std::map<Modio::ModID, Modio::CreateModFileParams> PendingUploads = Get().PendingModUploads;
				if (Get().PendingModUploads.find(IdToPrioritize) != Get().PendingModUploads.end())
				{
					Modio::Detail::Logger().Log(LogLevel::Info, LogCategory::ModManagement,
												"Prioritizing mod {}, currently pending upload", IdToPrioritize);
					Get().ModIDToPrioritize = IdToPrioritize;
					return true;
				}
			}
			return false;
		}

		bool SDKSessionData::PrioritizeModfileDownload(Modio::ModID IdToPrioritize)
		{
			// needs to check if the mod exists in the collection and if it requires an update or installation
			if (Modio::Optional<Modio::ModCollectionEntry&> CollectionEntry =
					FilterSystemModCollectionByUserSubscriptions().GetByModID(IdToPrioritize))
			{
				Modio::ModState CurrentState = (*CollectionEntry).GetModState();
				if (CurrentState == Modio::ModState::InstallationPending ||
					CurrentState == Modio::ModState::UpdatePending)
				{
					Modio::Detail::Logger().Log(LogLevel::Info, LogCategory::ModManagement,
												"Prioritizing mod {}, currently pending install or update",
												IdToPrioritize);
					Get().ModIDToPrioritize = IdToPrioritize;
					return true;
				}
			}
			return false;
		}

		std::weak_ptr<Modio::Detail::TemporaryModSet> SDKSessionData::InitTempModSet(std::vector<Modio::ModID> ModIds)
		{
			auto Lock = Modio::Detail::SDKSessionData::GetWriteLock();
			if (Get().TempModSet == nullptr)
			{
				Get().TempModSet = std::make_shared<Modio::Detail::TemporaryModSet>(ModIds);
				return Get().TempModSet;
			}
			else
			{
				return std::weak_ptr<Modio::Detail::TemporaryModSet>();
			}
		}

		bool SDKSessionData::CloseTempModSet()
		{
			auto Lock = Modio::Detail::SDKSessionData::GetWriteLock();
			if (Get().TempModSet != nullptr)
			{
				Get().TempModSet.reset();
				return true;
			}
			return false;
		}


		std::weak_ptr<Modio::ModProgressInfo> SDKSessionData::StartModDownloadOrUpdate(Modio::ModID ID)
		{
			auto Lock = Modio::Detail::SDKSessionData::GetWriteLock();
			if (Get().CurrentModInProgress != nullptr)
			{
				return std::weak_ptr<Modio::ModProgressInfo>();
			}
			else
			{
				Get().CurrentModInProgress = std::make_shared<Modio::ModProgressInfo>(ID);
				return Get().CurrentModInProgress;
			}
		}

		bool SDKSessionData::CancelModDownloadOrUpdate(Modio::ModID ID)
		{
			auto Lock = Modio::Detail::SDKSessionData::GetWriteLock();
			if (Get().CurrentModInProgress != nullptr)
			{
				if (Get().CurrentModInProgress->ID == ID)
				{
					Get().CurrentModInProgress.reset();
					return true;
				}
			}
			return false;
		}

		void SDKSessionData::FinishModDownloadOrUpdate()
		{
			auto Lock = Modio::Detail::SDKSessionData::GetWriteLock();

			Get().CurrentModInProgress.reset();
		}

		Modio::Optional<const Modio::ModProgressInfo> SDKSessionData::GetModProgress()
		{
			if (Get().CurrentModInProgress == nullptr)
			{
				return {};
			}
			else
			{
				// Workaround : Don't tell consumers that a mod operation is in progress until we've resolved how much
				// work there is to do This should eventually be replaced with a more robust check for the mod
				// operation's state
				if (Get().CurrentModInProgress->GetCurrentState() == ModProgressInfo::EModProgressState::Initializing)
				{
					return {};
				}
				return *(Get().CurrentModInProgress);
			}
		}

		Modio::ModCreationHandle SDKSessionData::GetNextModCreationHandle()
		{
			if (Get().CreationHandles.size())
			{
				Modio::ModCreationHandle NextHandle = Get().CreationHandles.rbegin()->first;
				NextHandle += Modio::ModCreationHandle(1);
				Get().CreationHandles.insert({NextHandle, {}});
				return NextHandle;
			}
			else
			{
				Modio::ModCreationHandle NextHandle = Modio::ModCreationHandle(0);
				Get().CreationHandles.insert({NextHandle, {}});
				return NextHandle;
			}
		}

		Modio::Optional<Modio::ModID> SDKSessionData::ResolveModCreationHandle(Modio::ModCreationHandle Handle)
		{
			return Get().CreationHandles.at(Handle);
		}

		void SDKSessionData::LinkModCreationHandle(Modio::ModCreationHandle Handle, Modio::ModID ID)
		{
			auto Lock = Modio::Detail::SDKSessionData::GetWriteLock();

			if (Get().CreationHandles.find(Handle) != Get().CreationHandles.end())
			{
				Modio::Detail::Logger().Log(Modio::LogLevel::Warning, Modio::LogCategory::ModManagement,
											"Attempting to relink handle {} to mod id {}", Handle, ID);
			}
			Get().CreationHandles.insert({Handle, ID});
		}

		void SDKSessionData::SetPlatformOverride(std::string Override)
		{
			Get().PlatformOverride = Override;
		}

		Modio::Optional<std::string> SDKSessionData::GetPlatformOverride()
		{
			return Get().PlatformOverride;
		}

		void SDKSessionData::SetPlatformEnvironment(std::string Environment)
		{
			Get().PlatformEnvironment = Environment;
		}

		Modio::Optional<std::string> SDKSessionData::GetPlatformEnvironment()
		{
			return Get().PlatformEnvironment;
		}

		void SDKSessionData::SetEnvironmentOverrideUrl(std::string OverrideUrl)
		{
			Get().EnvironmentOverrideUrl = OverrideUrl;
		}

		Modio::Optional<std::string> SDKSessionData::GetEnvironmentOverrideUrl()
		{
			return Get().EnvironmentOverrideUrl;
		}

		void SDKSessionData::SetPlatformStatusFilter(std::string PendingOnlyResults)
		{
			if (PendingOnlyResults == "true")
			{
				Get().PlatformStatusFilter = Modio::PlatformStatus::PendingOnly;
			}
		}

		Modio::Optional<Modio::PlatformStatus> SDKSessionData::GetPlatformStatusFilter()
		{
			return Get().PlatformStatusFilter;
		}

		std::string SDKSessionData::GetPlatformStatusFilterString()
		{
			if (Get().PlatformStatusFilter.has_value())
			{
				switch (Get().PlatformStatusFilter.value())
				{
					case PlatformStatus::ApprovedOnly:
						return "1";
					case PlatformStatus::LiveAndPending:
						return "0,1";
					case PlatformStatus::PendingOnly:
						return "0,1";
					default:
						return "1";
				}
			}

			return std::string();
		}

		void SDKSessionData::SetLocalLanguage(Modio::Language Local)
		{
			Get().LocalLanguage = Local;
		}

		Modio::Language SDKSessionData::GetLocalLanguage()
		{
			return Get().LocalLanguage;
		}

		void SDKSessionData::InvalidateSubscriptionCache()
		{
			Get().bSubscriptionCacheInvalid = true;
		}

		void SDKSessionData::ClearSubscriptionCacheInvalid()
		{
			Get().bSubscriptionCacheInvalid = false;
		}

		bool SDKSessionData::IsSubscriptionCacheInvalid()
		{
			return Get().bSubscriptionCacheInvalid;
		}

		void SDKSessionData::InvalidateTermsOfUseCache()
		{
			Get().bTermsOfUseCacheInvalid = true;
		}

		void SDKSessionData::ClearTermsOfUseCache()
		{
			Get().bTermsOfUseCacheInvalid = false;
		}

		bool SDKSessionData::IsTermsOfUseCacheInvalid()
		{
			return Get().bTermsOfUseCacheInvalid;
		}

		void SDKSessionData::InvalidateAllModsCache()
		{
			Get().ModCacheInvalidMap.clear();

			List<std::vector, Modio::ModID> listModIds =
				Detail::Services::GetGlobalService<Detail::CacheService>().GetAllModIdsInCache();

			for (auto& ModId : listModIds.GetRawList())
			{
				Get().ModCacheInvalidMap.emplace(std::make_pair(ModId, true));
			}
		}

		void SDKSessionData::InvalidatePurchaseCache()
		{
			Get().bPurchaseCacheInvalid = true;
		}

		void SDKSessionData::ClearPurchaseCacheInvalid()
		{
			Get().bPurchaseCacheInvalid = false;
		}

		bool SDKSessionData::IsPurchaseCacheInvalid()
		{
			return Get().bPurchaseCacheInvalid;
		}

		void SDKSessionData::InvalidateModCache(Modio::ModID ID)
		{
			auto CacheEntryIterator = Get().ModCacheInvalidMap.find(ID);
			if (CacheEntryIterator != Get().ModCacheInvalidMap.end())
			{
				CacheEntryIterator->second = true;
			}
			else
			{
				Get().ModCacheInvalidMap.emplace(std::make_pair(ID, true));
			}
		}

		void SDKSessionData::ClearModCacheInvalid(Modio::ModID ID)
		{
			auto CacheEntryIterator = Get().ModCacheInvalidMap.find(ID);
			if (CacheEntryIterator != Get().ModCacheInvalidMap.end())
			{
				CacheEntryIterator->second = false;
			}
			else
			{
				Get().ModCacheInvalidMap.emplace(std::make_pair(ID, false));
			}
		}

		bool SDKSessionData::IsModCacheInvalid(Modio::ModID ID)
		{
			auto CacheEntryIterator = Get().ModCacheInvalidMap.find(ID);
			if (CacheEntryIterator != Get().ModCacheInvalidMap.end())
			{
				return CacheEntryIterator->second;
			}
			return false;
		}

		void SDKSessionData::EnqueueTask(fu2::unique_function<void()> Task)
		{
			Get().IncomingTaskQueue.enqueue(std::move(Task));
		}

		void SDKSessionData::PushQueuedTasksToGlobalContext()
		{
			fu2::unique_function<void()> Task;
			while (Get().IncomingTaskQueue.size_approx() > 0)
			{
				if (Get().IncomingTaskQueue.try_dequeue(Task))
				{
					asio::post(Modio::Detail::Services::GetGlobalContext().get_executor(), std::move(Task));
				}
			}
		}

		std::shared_timed_mutex& SDKSessionData::GetRWMutex()
		{
			static std::shared_timed_mutex Underlying;
			return Underlying;
		}
		std::shared_timed_mutex& SDKSessionData::GetShutdownMutex()
		{
			static std::shared_timed_mutex Underlying;
			return Underlying;
		}

		MODIO_NODISCARD std::shared_lock<std::shared_timed_mutex> SDKSessionData::GetReadLock()
		{
			return std::shared_lock<std::shared_timed_mutex>(GetRWMutex());
		}
		MODIO_NODISCARD std::unique_lock<std::shared_timed_mutex> SDKSessionData::GetWriteLock()
		{
			return std::unique_lock<std::shared_timed_mutex>(GetRWMutex());
		}
		MODIO_NODISCARD std::unique_lock<std::shared_timed_mutex> SDKSessionData::GetShutdownLock()
		{
			return std::unique_lock<std::shared_timed_mutex>(GetShutdownMutex());
		}
		MODIO_NODISCARD std::unique_lock<std::shared_timed_mutex> SDKSessionData::TryGetShutdownLock()
		{
			return std::unique_lock<std::shared_timed_mutex>(GetShutdownMutex(), std::try_to_lock);
		}

		SDKSessionData::SDKSessionData() {}

		SDKSessionData::SDKSessionData(const Modio::InitializeOptions& Options, Modio::Language InLocalLanguage)
			: GameID(Options.GameID),
			  APIKey(Options.APIKey),
			  Environment(Options.GameEnvironment),
			  PortalInUse(Options.PortalInUse),
			  LocalLanguage(InLocalLanguage),
			  CurrentInitializationState(InitializationState::NotInitialized)
		{}
	} // namespace Detail

} // namespace Modio

MODIO_DIAGNOSTIC_POP
