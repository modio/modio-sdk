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

#include "modio/core/ModioBuffer.h"
#include "modio/core/ModioInitializeOptions.h"
#include "modio/core/ModioLogger.h"

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
				Get() = SDKSessionData(Options);
				Get().CurrentInitializationState = InitializationState::Initializing;
				return true;
			}
		}

		void SDKSessionData::Deinitialize()
		{
			Get().FlushModManagementLog();

			Get().CurrentInitializationState = InitializationState::NotInitialized;
		}

		void SDKSessionData::ConfirmInitialize()
		{
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
			Get().bModManagementEnabled = false;
		}

		void SDKSessionData::MarkAsRateLimited()
		{
			Get().bRateLimited = true;
			Get().RateLimitedStart = std::chrono::system_clock::now();
		}

		bool SDKSessionData::IsRateLimited()
		{
			// TODO: @modio-core need to compare the timestamp of start to now and work out time elapsed, reset flag if
			// need be
			return Get().bRateLimited;
		}

		Modio::ModCollection& SDKSessionData::GetSystemModCollection()
		{
			return Get().SystemModCollection;
		}

		Modio::ModCollection SDKSessionData::FilterSystemModCollectionByUserSubscriptions()
		{
			MODIO_PROFILE_SCOPE(FilterSystemModsByUserSubs);
			return Get().SystemModCollection.FilterByUserSubscriptions(Get().UserData.UserSubscriptions);
		}

		void SDKSessionData::InitializeForUser(Modio::User AuthenticatedUser, Modio::Detail::OAuthToken AuthToken)
		{
			Get().UserData.InitializeForUser(std::move(AuthenticatedUser), std::move(AuthToken));
		}

		const Modio::Optional<Modio::Detail::OAuthToken> SDKSessionData::GetAuthenticationToken()
		{
			return Get().UserData.GetAuthenticationToken();
		}

		Modio::UserSubscriptionList& SDKSessionData::GetUserSubscriptions()
		{
			return Get().UserData.UserSubscriptions;
		}

		Modio::Detail::Buffer SDKSessionData::SerializeUserData()
		{
			MODIO_PROFILE_SCOPE(SerializeUserData);
			nlohmann::json Data(Get().UserData);
			Data["version"] = 1;
			std::string UserData = Data.dump();
			Modio::Detail::Buffer DataBuffer(UserData.size());
			std::copy(UserData.begin(), UserData.end(), DataBuffer.begin());
			return DataBuffer;
		}

		bool SDKSessionData::DeserializeUserDataFromBuffer(Modio::Detail::Buffer UserDataBuffer)
		{
			MODIO_PROFILE_SCOPE(DeserializeUserData);
			nlohmann::json UserDataJson = Modio::Detail::ToJson(std::move(UserDataBuffer));
			from_json(UserDataJson, Get().UserData);
			return Get().UserData.IsValid();
		}

		void SDKSessionData::ClearUserData()
		{
			Get().UserData.ResetUserData();
		}

		void SDKSessionData::InvalidateOAuthToken()
		{
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
				for (auto&& Event : Get().EventLog.ClearLog())
				{
					Callback(Event);
				}
			}
		}

		Modio::ModEventLog& SDKSessionData::GetModManagementEventLog()
		{
			return Get().EventLog;
		}

		void SDKSessionData::SetLastValidationError(std::vector<FieldError> ExtendedErrorInformation)
		{
			Get().LastValidationError = ExtendedErrorInformation;
		}

		std::vector<FieldError> SDKSessionData::GetLastValidationError()
		{
			return Get().LastValidationError;
		}

		void SDKSessionData::ClearLastValidationError()
		{
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

		std::weak_ptr<Modio::ModProgressInfo> SDKSessionData::StartModDownloadOrUpdate(Modio::ModID ID)
		{
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
			if (Get().CurrentModInProgress != nullptr)
			{
				if (Get().CurrentModInProgress->ID == ID)
				{
					FinishModDownloadOrUpdate();
					return true;
				}
			}
			return false;
		}

		void SDKSessionData::FinishModDownloadOrUpdate()
		{
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
				if (Get().CurrentModInProgress->TotalDownloadSize == 0)
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
			if (Get().CreationHandles.find(Handle) != Get().CreationHandles.end())
			{
				Modio::Detail::Logger().Log(Modio::LogLevel::Warning, Modio::LogCategory::ModManagement,
											"Attempting to relink handle {} to mod id {}", Handle, ID);
			}
			Get().CreationHandles.insert({Handle, ID});
		}

		void SDKSessionData::SetEnvironmentOverrideUrl(std::string OverrideUrl)
		{
			Get().EnvironmentOverrideUrl = OverrideUrl;
		}

		Modio::Optional<std::string> SDKSessionData::GetEnvironmentOverrideUrl()
		{
			return Get().EnvironmentOverrideUrl;
		}

		SDKSessionData::SDKSessionData() {}

		SDKSessionData::SDKSessionData(const Modio::InitializeOptions& Options)
			: GameID(Options.GameID),
			  APIKey(Options.APIKey),
			  Environment(Options.GameEnvironment),
			  PortalInUse(Options.PortalInUse),
			  CurrentInitializationState(InitializationState::NotInitialized)
		{}
	} // namespace Detail

} // namespace Modio