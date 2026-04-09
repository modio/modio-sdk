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

#include "modio/core/ModioSDKForwardDecls.h"
#include "modio/detail/FilesystemWrapper.h"

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
		bool RequireSDKIsInitialized(std::function<void(Modio::ErrorCode, OtherArgs...)>& Handler);

		template<typename... OtherArgs>
		bool RequireUserIsAuthenticated(std::function<void(Modio::ErrorCode, OtherArgs...)>& Handler);

		template<typename... OtherArgs>
		bool RequireMetricsSessionIsInitialized(std::function<void(Modio::ErrorCode, OtherArgs...)>& Handler);

		template<typename... OtherArgs>
		bool RequireMetricsSessionIsActive(std::function<void(Modio::ErrorCode, OtherArgs...)>& Handler);

		template<typename... OtherArgs>
		bool RequireMetricsSessionIsNOTActive(std::function<void(Modio::ErrorCode, OtherArgs...)>& Handler);

		template<typename... OtherArgs>
		bool RequireValidMetricSessionParams(const Modio::MetricsSessionParams& Params,
											 std::function<void(Modio::ErrorCode, OtherArgs...)>& Handler);

		template<typename... OtherArgs>
		bool RequireNotRateLimited(std::function<void(Modio::ErrorCode, OtherArgs...)>& Handler);

		template<typename... OtherArgs>
		bool RequireModManagementEnabled(std::function<void(Modio::ErrorCode, OtherArgs...)>& Handler);

		template<typename... OtherArgs>
		bool RequireUserNotSubscribed(Modio::ModID IDToCheck,
									  std::function<void(Modio::ErrorCode, OtherArgs...)>& Handler);

		template<typename... OtherArgs>
		bool RequireValidInitParams(const Modio::InitializeOptions& Options,
									std::function<void(Modio::ErrorCode, OtherArgs...)>& Handler);

		template<typename... OtherArgs>
		bool RequireValidServerInitParam(const Modio::ServerInitializeOptions& Options,
										 std::function<void(Modio::ErrorCode, OtherArgs...)>& Handler);

		template<typename... OtherArgs>
		bool RequireValidEditModParams(const Modio::EditModParams& Params,
									   std::function<void(Modio::ErrorCode, OtherArgs...)>& Handler);

		template<typename... OtherArgs>
		bool RequireModIsNotUninstallPending(Modio::ModID ModToSubscribeTo,
											 std::function<void(Modio::ErrorCode, OtherArgs...)>& Handler);

		template<typename... OtherArgs>
		bool RequireValidXBoxRefreshEntitlementsExtendedParameters(
			Modio::EntitlementParams User, std::function<void(Modio::ErrorCode, OtherArgs...)>& Handler);

		template<typename... OtherArgs>
		bool RequireValidPSNRefreshEntitlementsExtendedParameters(
			Modio::EntitlementParams User, std::function<void(Modio::ErrorCode, OtherArgs...)>& Handler);

		template<typename... OtherArgs>
		bool RequireValidOculusExtendedParameters(Modio::AuthenticationParams User,
												  std::function<void(Modio::ErrorCode, OtherArgs...)>& Handler);

		template<typename... OtherArgs>
		bool RequireValidGoogleRefreshEntitlementsExtendedParameters(
			Modio::EntitlementParams User, std::function<void(Modio::ErrorCode, OtherArgs...)>& Handler);

		template<typename... OtherArgs>
		bool RequireValidMetaRefreshEntitlementsExtendedParameters(
			Modio::EntitlementParams User, std::function<void(Modio::ErrorCode, OtherArgs...)>& Handler);

		template<typename... OtherArgs>
		bool RequireFileExists(Modio::filesystem::path FileToCheck,
							   std::function<void(Modio::ErrorCode, OtherArgs...)>& Handler);

		template<typename... OtherArgs>
		bool RequireModIDNotInTempModSet(const Modio::ModID& ID,
										 std::function<void(Modio::ErrorCode, OtherArgs...)>& Handler);

		template<typename... OtherArgs>
		bool RequireValidModID(const Modio::ModID& ID, std::function<void(Modio::ErrorCode, OtherArgs...)>& Handler);

		template<typename... OtherArgs>
		bool RequireAllModIDsValid(const std::vector<Modio::ModID>& Mods,
								   std::function<void(Modio::ErrorCode, OtherArgs...)>& Handler);

		template<typename... OtherArgs>
		bool RequireValidModCollectionID(const Modio::ModCollectionID& ID,
										 std::function<void(Modio::ErrorCode, OtherArgs...)>& Handler);

		template<typename... OtherArgs>
		bool RequireValidGameID(const Modio::GameID& ID, std::function<void(Modio::ErrorCode, OtherArgs...)>& Handler);

		template<typename... OtherArgs>
		bool RequireValidUserID(const Modio::UserID& ID, std::function<void(Modio::ErrorCode, OtherArgs...)>& Handler);

		template<typename... OtherArgs>
		bool RequireValidReportParams(const Modio::ReportParams& Params,
									  std::function<void(Modio::ErrorCode, OtherArgs...)>& Handler);

		template<typename... OtherArgs>
		bool RequireFetchExternalUpdatesNOTRunning(std::function<void(Modio::ErrorCode, OtherArgs...)>& Handler);
	} // namespace Detail
} // namespace Modio