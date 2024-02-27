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
	#include "modio/ModioSDK.h"
#else
	#pragma once
#endif

#include "modio/cache/ModioCacheService.h"
#include "modio/detail/ModioProfiling.h"
#include "modio/detail/ModioSDKSessionData.h"
#include "modio/detail/ops/GetGameInfoOp.h"
#include "modio/detail/ops/GetMutedUsersOp.h"
#include "modio/detail/ops/MuteUserOp.h"
#include "modio/detail/ops/ReportContentOp.h"
#include "modio/detail/ops/ServiceInitializationOp.h"
#include "modio/detail/ops/Shutdown.h"
#include "modio/detail/ops/UnmuteUserOp.h"
#include "modio/file/ModioFileService.h"
#include "modio/http/ModioHttpService.h"
#include "modio/impl/SDKPostAsync.h"
#include "modio/impl/SDKPreconditionChecks.h"
#include "modio/userdata/ModioUserDataService.h"
#include <chrono>
#include <functional>
#include <thread>
// Implementation header - do not include directly

namespace Modio
{
	void InitializeAsync(Modio::InitializeOptions InitOptions, std::function<void(Modio::ErrorCode)> OnInitComplete)
	{
		Modio::Detail::SDKSessionData::EnqueueTask([InitOptions, OnInitComplete = std::move(OnInitComplete)]() mutable {
			if (Modio::Detail::RequireValidInitParams(InitOptions, OnInitComplete))
			{
				auto WrappedCallback = Modio::Detail::ApplyPostAsyncChecks(OnInitComplete);

				asio::async_compose<std::function<void(Modio::ErrorCode)>, void(Modio::ErrorCode)>(
					ServiceInitializationOp(InitOptions), WrappedCallback,
					Modio::Detail::Services::GetGlobalContext().get_executor());
			}
		});
	}

	void RunPendingHandlers()
	{
		auto ShutdownLock = Modio::Detail::SDKSessionData::TryGetShutdownLock();
		if (ShutdownLock.owns_lock())
		{
			MODIO_PROFILE_SCOPE(RunPendingHandlers);
			{
				MODIO_PROFILE_SCOPE(IOContext_Poll);
        
				// Run any pending handlers on the global io_context
				if (Modio::Detail::Services::GetGlobalContext().stopped())
				{
					Modio::Detail::Services::GetGlobalContext().restart();
				}
				Modio::Detail::SDKSessionData::PushQueuedTasksToGlobalContext();
				// Run handlers one at a time until we reach the timeout threshold
				std::chrono::time_point<std::chrono::steady_clock> PollStartTime = std::chrono::steady_clock::now();
				do
				{
					Modio::Detail::Services::GetGlobalContext().poll_one();
        
				} while (std::chrono::steady_clock::now() - PollStartTime < std::chrono::milliseconds(1));
			}
        
			{
				MODIO_PROFILE_SCOPE(FlushManagementLog);
				// invoke the mod management log callback if the user has set it
				Modio::Detail::SDKSessionData::FlushModManagementLog();
			}
        
			{
				MODIO_PROFILE_SCOPE(FlushLogBuffer);
				// invoke log callback if the user has set it
				Modio::Detail::Services::GetGlobalService<Modio::Detail::LogService>().FlushLogBuffer();
			}
		}
	}

#ifndef MODIO_SEPARATE_COMPILATION
	// Forward declaration
	void DisableModManagement();
#endif

	// This might need a timeout parameter
	void ShutdownAsync(std::function<void(Modio::ErrorCode)> OnShutdownComplete)
	{
		auto ShutdownLock = Modio::Detail::SDKSessionData::GetShutdownLock();
		if (Modio::Detail::RequireSDKIsInitialized(OnShutdownComplete))
		{
			// Halt the mod management loop
			Modio::DisableModManagement();
			// Signal the internal services their operations should shut down ASAP
			Modio::Detail::Services::GetGlobalService<Modio::Detail::CacheService>().Shutdown();
			Modio::Detail::Services::GetGlobalService<Modio::Detail::UserDataService>().Shutdown();
			Modio::Detail::Services::GetGlobalService<Modio::Detail::HttpService>().Shutdown();
			Modio::Detail::Services::GetGlobalService<Modio::Detail::FileService>().Shutdown();
			Modio::Detail::Services::GetGlobalService<Modio::Detail::LogService>().Shutdown();
			Modio::Detail::Services::GetGlobalService<Modio::Detail::TimerService>().Shutdown();
			Modio::Detail::SDKSessionData::Deinitialize();

			Modio::Detail::SDKSessionData::PushQueuedTasksToGlobalContext();
			// Steal the old io context (and associated services, and all of the enqueued tasks which we can be sure
			// we're the only consumer just now because we have the shutdown lock)
			auto OldContext = Modio::Detail::Services::ResetGlobalContext();
			// Post to the new io_context an operation that will exhaust the old context and call the user callback
			// when it's completed
			Modio::Detail::SDKSessionData::EnqueueTask(
				[OldContext = std::move(OldContext), OnShutdownComplete = std::move(OnShutdownComplete)]() mutable {
					asio::async_compose<std::function<void(Modio::ErrorCode)>, void(Modio::ErrorCode)>(

						Modio::Detail::ShutdownOp(std::move(OldContext)), OnShutdownComplete,
						Modio::Detail::Services::GetGlobalContext().get_executor());
				});
		}
	}

	void SetLogLevel(Modio::LogLevel Level)
	{
		auto Lock = Modio::Detail::SDKSessionData::GetWriteLock();

		Modio::Detail::LogService::SetGlobalLogLevel(Level);
	}

	void SetLogCallback(std::function<void(Modio::LogLevel Level, const std::string& Message)> LogCallback)
	{
		auto Lock = Modio::Detail::SDKSessionData::GetWriteLock();

		Modio::Detail::LogService::SetLogCallback(LogCallback);
	}

	std::vector<Modio::FieldError> GetLastValidationError()
	{
		auto Lock = Modio::Detail::SDKSessionData::GetReadLock();

		return Modio::Detail::SDKSessionData::GetLastValidationError();
	}

	void ReportContentAsync(Modio::ReportParams Report, std::function<void(Modio::ErrorCode)> Callback)
	{
		Modio::Detail::SDKSessionData::EnqueueTask(
			[Report = std::move(Report), Callback = std::move(Callback)]() mutable {
				if (Modio::Detail::RequireSDKIsInitialized(Callback) &&
				Modio::Detail::RequireValidReportParams(Report, Callback))
				{
					asio::async_compose<std::function<void(Modio::ErrorCode)>, void(Modio::ErrorCode)>(
						Modio::Detail::ReportContentOp(Report), Callback,
						Modio::Detail::Services::GetGlobalContext().get_executor());
				}
			});
	}

	void MuteUserAsync(Modio::UserID UserID, std::function<void(Modio::ErrorCode)> Callback)
	{
		Modio::Detail::SDKSessionData::EnqueueTask([UserID, Callback = std::move(Callback)]() mutable {
			if (Modio::Detail::RequireSDKIsInitialized(Callback) && Modio::Detail::RequireNotRateLimited(Callback) &&
				Modio::Detail::RequireUserIsAuthenticated(Callback) &&
				Modio::Detail::RequireValidUserID(UserID, Callback))
			{
				asio::async_compose<std::function<void(Modio::ErrorCode)>, void(Modio::ErrorCode)>(
					Modio::Detail::MuteUserOp(UserID), Callback,
					Modio::Detail::Services::GetGlobalContext().get_executor());
			}
		});
	}

	void UnmuteUserAsync(Modio::UserID UserID, std::function<void(Modio::ErrorCode)> Callback)
	{
		Modio::Detail::SDKSessionData::EnqueueTask([UserID, Callback = std::move(Callback)]() mutable {
			if (Modio::Detail::RequireSDKIsInitialized(Callback) && Modio::Detail::RequireNotRateLimited(Callback) &&
				Modio::Detail::RequireUserIsAuthenticated(Callback) &&
				Modio::Detail::RequireValidUserID(UserID, Callback))
			{
				asio::async_compose<std::function<void(Modio::ErrorCode)>, void(Modio::ErrorCode)>(
					Modio::Detail::UnmuteUserOp(UserID), Callback,
					Modio::Detail::Services::GetGlobalContext().get_executor());
			}
		});
	}

	void GetMutedUsersAsync(std::function<void(Modio::ErrorCode, Modio::Optional<Modio::UserList>)> Callback)
	{
		Modio::Detail::SDKSessionData::EnqueueTask([Callback = std::move(Callback)]() mutable {
			if (Modio::Detail::RequireSDKIsInitialized(Callback) && Modio::Detail::RequireNotRateLimited(Callback) &&
				Modio::Detail::RequireUserIsAuthenticated(Callback))
			{
				return asio::async_compose<std::function<void(Modio::ErrorCode, Modio::Optional<Modio::UserList>)>,
										   void(Modio::ErrorCode, Modio::Optional<Modio::UserList>)>(
					Modio::Detail::GetMutedUsersOp(),

					Callback, Modio::Detail::Services::GetGlobalContext().get_executor());
			}
		});
	}

	void GetGameInfoAsync(Modio::GameID GameID,
						  std::function<void(Modio::ErrorCode, Modio::Optional<Modio::GameInfo>)> Callback)
	{
		Modio::Detail::SDKSessionData::EnqueueTask([GameID, Callback = std::move(Callback)]() mutable {
			if (Modio::Detail::RequireSDKIsInitialized(Callback) && Modio::Detail::RequireNotRateLimited(Callback) &&
				Modio::Detail::RequireValidGameID(GameID, Callback))
			{
				return asio::async_compose<std::function<void(Modio::ErrorCode, Modio::Optional<Modio::GameInfo>)>,
										   void(Modio::ErrorCode, Modio::Optional<Modio::GameInfo>)>(
					Modio::Detail::GetGameInfoOp(GameID, Modio::Detail::SDKSessionData::CurrentAPIKey()), Callback,
					Modio::Detail::Services::GetGlobalContext().get_executor());
			}
		});
	}

	void SetLanguage(Modio::Language Locale)
	{
		bool bLanguageChanged = false;
		Modio::Language CurrentLanguage = Modio::Language::English;
		{
			auto Lock = Modio::Detail::SDKSessionData::GetReadLock();
			CurrentLanguage = Modio::Detail::SDKSessionData::GetLocalLanguage();
			bLanguageChanged = CurrentLanguage != Locale;
		}
		if (bLanguageChanged)
		{
			auto Lock = Modio::Detail::SDKSessionData::GetWriteLock();
			// Guard against the language being mutated while we didnt have the lock
			if (CurrentLanguage == Modio::Detail::SDKSessionData::GetLocalLanguage())
			{
				// Invalidate all necessary cache
				Modio::Detail::SDKSessionData::InvalidateTermsOfUseCache();
				Modio::Detail::SDKSessionData::InvalidateSubscriptionCache();
				Modio::Detail::SDKSessionData::InvalidateAllModsCache();
				Detail::Services::GetGlobalService<Detail::CacheService>().ClearCache();
				Modio::Detail::SDKSessionData::SetLocalLanguage(Locale);
			}
		}
	}

} // namespace Modio
