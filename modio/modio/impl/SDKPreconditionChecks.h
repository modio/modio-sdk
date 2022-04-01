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

#include "modio/core/ModioErrorCode.h"
#include "modio/core/ModioInitializeOptions.h"
#include "modio/core/ModioLogger.h"
#include "modio/core/ModioServices.h"
#include "modio/detail/ModioSDKSessionData.h"

/// @brief These are templated helper functions - they take in the user-provided completion handler, check some
/// precondition, and if the precondition check fails they :
/// * Post the user completion handler to the io_context with the specified error AND
/// * return false
///
/// This allows the calling function to skip over the initiation of the asynchronous operation if any of the checks
/// failed, knowing that the completion handler has already been queued with the correct error code, so the caller
/// doesn't need to care which of the preconditions failed.

namespace Modio
{
	namespace Detail
	{
		template<typename... OtherArgs>
		bool RequireSDKIsInitialized(std::function<void(Modio::ErrorCode, OtherArgs...)>& Handler)
		{
			if (Modio::Detail::SDKSessionData::IsInitialized())
			{
				return true;
			}
			else
			{
				asio::post(Modio::Detail::Services::GetGlobalContext().get_executor(),
						   [CompletionHandler =
								std::forward<std::function<void(Modio::ErrorCode, OtherArgs...)>>(Handler)]() mutable {
							   CompletionHandler(Modio::make_error_code(Modio::GenericError::SDKNotInitialized),
												 (OtherArgs {})...);
						   });
				return false;
			}
		}

		template<typename... OtherArgs>
		bool RequireUserIsAuthenticated(std::function<void(Modio::ErrorCode, OtherArgs...)>& Handler)
		{
			if (Modio::Detail::SDKSessionData::GetAuthenticatedUser().has_value())
			{
				return true;
			}
			else
			{
				asio::post(Modio::Detail::Services::GetGlobalContext().get_executor(),
						   [CompletionHandler =
								std::forward<std::function<void(Modio::ErrorCode, OtherArgs...)>>(Handler)]() mutable {
							   CompletionHandler(Modio::make_error_code(Modio::UserDataError::InvalidUser),
												 (OtherArgs {})...);
						   });
				return false;
			}
		}

		template<typename... OtherArgs>
		bool RequireUserIsNOTAuthenticated(std::function<void(Modio::ErrorCode, OtherArgs...)>& Handler)
		{
			if (!Modio::Detail::SDKSessionData::GetAuthenticatedUser().has_value())
			{
				return true;
			}
			else
			{
				asio::post(Modio::Detail::Services::GetGlobalContext().get_executor(),
						   [CompletionHandler =
								std::forward<std::function<void(Modio::ErrorCode, OtherArgs...)>>(Handler)]() mutable {
							   CompletionHandler(Modio::make_error_code(Modio::UserAuthError::AlreadyAuthenticated),
												 (OtherArgs {})...);
						   });
				return false;
			}
		}

		template<typename... OtherArgs>
		bool RequireNotRateLimited(std::function<void(Modio::ErrorCode, OtherArgs...)>& Handler)
		{
			if (!Modio::Detail::SDKSessionData::IsRateLimited())
			{
				return true;
			}
			else
			{
				asio::post(Modio::Detail::Services::GetGlobalContext().get_executor(),
						   [CompletionHandler =
								std::forward<std::function<void(Modio::ErrorCode, OtherArgs...)>>(Handler)]() mutable {
							   CompletionHandler(Modio::make_error_code(Modio::HttpError::RateLimited),
												 (OtherArgs {})...);
						   });
				return false;
			}
		}

		template<typename... OtherArgs>
		bool RequireModManagementEnabled(std::function<void(Modio::ErrorCode, OtherArgs...)>& Handler)
		{
			if (Modio::Detail::SDKSessionData::IsModManagementEnabled())
			{
				return true;
			}
			else
			{
				asio::post(Modio::Detail::Services::GetGlobalContext().get_executor(),
						   [CompletionHandler =
								std::forward<std::function<void(Modio::ErrorCode, OtherArgs...)>>(Handler)]() mutable {
							   CompletionHandler(
								   Modio::make_error_code(Modio::ModManagementError::ModManagementDisabled),
								   (OtherArgs {})...);
						   });
				return false;
			}
		}

		template<typename... OtherArgs>
		bool RequireUserNotSubscribed(Modio::ModID IDToCheck,
									  std::function<void(Modio::ErrorCode, OtherArgs...)>& Handler)
		{
			if (Modio::Detail::SDKSessionData::GetUserSubscriptions().Get().count(IDToCheck))
			{
				asio::post(Modio::Detail::Services::GetGlobalContext().get_executor(),
						   [CompletionHandler =
								std::forward<std::function<void(Modio::ErrorCode, OtherArgs...)>>(Handler)]() mutable {
							   CompletionHandler(Modio::make_error_code(Modio::ModManagementError::AlreadySubscribed),
												 (OtherArgs {})...);
						   });
				return false;
			}
			else
			{
				return true;
			}
		}

		template<typename... OtherArgs>
		bool RequireValidInitParams(const Modio::InitializeOptions& Options,
									std::function<void(Modio::ErrorCode, OtherArgs...)>& Handler)
		{
			bool bInvalidParameter = false;
			if (!Options.GameID.IsValid())
			{
				Modio::Detail::Logger().Log(LogLevel::Error, LogCategory::Core, "GameID {} is Invalid",
											*Options.GameID);
				bInvalidParameter = true;
			}
			else if (!Options.APIKey.IsValid())
			{
				Modio::Detail::Logger().Log(LogLevel::Error, LogCategory::Core, "APIKey {} is Invalid",
											*Options.APIKey);
				bInvalidParameter = true;
			}
			else if (Options.User.size() == 0)
			{
				Modio::Detail::Logger().Log(LogLevel::Error, LogCategory::Core, "Session ID may not be empty");
				bInvalidParameter = true;
			}

			if (bInvalidParameter)
			{
				asio::post(Modio::Detail::Services::GetGlobalContext().get_executor(),
						   [CompletionHandler =
								std::forward<std::function<void(Modio::ErrorCode, OtherArgs...)>>(Handler)]() mutable {
							   CompletionHandler(Modio::make_error_code(Modio::GenericError::BadParameter),
												 (OtherArgs {})...);
						   });
				return false;
			}
			else
			{
				return true;
			}
		}
	} // namespace Detail
} // namespace Modio
