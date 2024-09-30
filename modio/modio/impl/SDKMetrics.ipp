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

#include "modio/core/ModioMetricsService.h"
#include "modio/detail/ModioSDKSessionData.h"
#include "modio/detail/ops/metrics/MetricsSessionEndOp.h"
#include "modio/detail/ops/metrics/MetricsSessionSendHeartbeatOnceOp.h"
#include "modio/impl/SDKPreconditionChecks.h"
#include <functional>

namespace Modio
{
	void MetricsSessionStartAsync(Modio::MetricsSessionParams Params, std::function<void(Modio::ErrorCode)> Callback)
	{
		if (!Params.SessionId.has_value() || !Params.SessionId.value().IsValid())
		{
			Modio::Guid SessionId = Modio::Guid::GenerateGuid();
			Modio::Detail::Logger().Log(Modio::LogLevel::Trace, Modio::LogCategory::ModMetrics,
										fmt::format("No SessionId passed in, generating a new one: `{}`.", *SessionId));
			Params.SessionId = SessionId;
		}

		Modio::Detail::SDKSessionData::EnqueueTask([Params, Callback = std::move(Callback)]() mutable {
			if (Modio::Detail::RequireSDKIsInitialized(Callback) && Modio::Detail::RequireNotRateLimited(Callback) &&
				Modio::Detail::RequireUserIsAuthenticated(Callback) &&
				Modio::Detail::RequireMetricsSessionIsInitialized(Callback) &&
				Modio::Detail::RequireMetricsSessionIsNOTActive(Callback) &&
				Modio::Detail::RequireValidMetricSessionParams(Params, Callback))
			{
				Modio::Detail::Services::GetGlobalService<Modio::Detail::MetricsService>().StartSessionAsync(Params,
																											 Callback);
			}
		});
	}

	void MetricsSessionSendHeartbeatOnceAsync(std::function<void(Modio::ErrorCode)> Callback)
	{
		Modio::Detail::SDKSessionData::EnqueueTask([Callback = std::move(Callback)]() mutable {
			if (Modio::Detail::RequireSDKIsInitialized(Callback) && Modio::Detail::RequireNotRateLimited(Callback) &&
				Modio::Detail::RequireUserIsAuthenticated(Callback) &&
				Modio::Detail::RequireMetricsSessionIsInitialized(Callback) &&
				Modio::Detail::RequireMetricsSessionIsActive(Callback))
			{
				Modio::Detail::Services::GetGlobalService<Modio::Detail::MetricsService>().SendHeartbeatOnceAsync(
					Callback);
			}
		});
	}

	void MetricsSessionSendHeartbeatAtIntervalAsync(uint32_t IntervalSeconds,
													std::function<void(Modio::ErrorCode)> Callback)
	{
		Modio::Detail::SDKSessionData::EnqueueTask([IntervalSeconds, Callback = std::move(Callback)]() mutable {
			if (Modio::Detail::RequireSDKIsInitialized(Callback) && Modio::Detail::RequireNotRateLimited(Callback) &&
				Modio::Detail::RequireUserIsAuthenticated(Callback) &&
				Modio::Detail::RequireMetricsSessionIsInitialized(Callback) &&
				Modio::Detail::RequireMetricsSessionIsActive(Callback))
			{
				Modio::Detail::Services::GetGlobalService<Modio::Detail::MetricsService>().SendHeartbeatAtIntervalAsync(
					IntervalSeconds, Callback);
			}
		});
	}

	void MetricsSessionEndAsync(std::function<void(Modio::ErrorCode)> Callback)
	{
		Modio::Detail::SDKSessionData::EnqueueTask([Callback = std::move(Callback)]() mutable {
			if (Modio::Detail::RequireSDKIsInitialized(Callback) && Modio::Detail::RequireNotRateLimited(Callback) &&
				Modio::Detail::RequireUserIsAuthenticated(Callback) &&
				Modio::Detail::RequireMetricsSessionIsInitialized(Callback) &&
				Modio::Detail::RequireMetricsSessionIsActive(Callback))
			{
				Modio::Detail::Services::GetGlobalService<Modio::Detail::MetricsService>().EndSessionAsync(Callback);
			}
		});
	}

} // namespace Modio