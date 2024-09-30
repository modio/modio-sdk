/*
 *  Copyright (C) 2024 mod.io Pty Ltd. <https://mod.io>
 *
 *  This file is part of the mod.io SDK.
 *
 *  Distributed under the MIT License. (See accompanying file LICENSE or
 *   view online at <https://github.com/modio/modio-sdk/blob/main/LICENSE>)
 *
 */

#ifdef MODIO_SEPARATE_COMPILATION
	#include "modio/core/ModioMetricsService.h"
#endif

#include "modio/core/ModioCoreTypes.h"
#include "modio/detail/ops/metrics/MetricsSessionEndOp.h"
#include "modio/detail/ops/metrics/MetricsSessionSendHeartbeatAtIntervalOp.h"
#include "modio/detail/ops/metrics/MetricsSessionSendHeartbeatOnceOp.h"
#include "modio/detail/ops/metrics/MetricsSessionStartOp.h"

#include <algorithm>

namespace Modio
{
	namespace Detail
	{
		void MetricsService::InitMetricsSession(const std::string& SecretKeyString)
		{
			SecretKey = SecretKeyString;
			bIsInitialized = true;
		}

		void MetricsService::StartSessionAsync(Modio::MetricsSessionParams Params,
											   std::function<void(Modio::ErrorCode)> Callback)
		{
			SessionId = Params.SessionId;
			AddMods(Params.ModIds);

			Modio::Detail::MetricsSessionStartOpAsync(Callback);
		}

		void MetricsService::SendHeartbeatOnceAsync(std::function<void(Modio::ErrorCode)> Callback)
		{
			Modio::Detail::MetricsSessionSendHeartbeatOnceOpAsync(Callback);
		}

		void MetricsService::SendHeartbeatAtIntervalAsync(uint32_t IntervalSeconds,
														  std::function<void(Modio::ErrorCode)> Callback)
		{
			Modio::Detail::MetricsSessionSendHeartbeatAtIntervalOpAsync(IntervalSeconds, Callback);
		}

		void MetricsService::EndSessionAsync(std::function<void(Modio::ErrorCode)> Callback)
		{
			Modio::Detail::MetricsSessionEndOpAsync(Callback);
		}

		void MetricsService::SetSessionIsActive(bool bActive)
		{
			Modio::Detail::Logger().Log(Modio::LogLevel::Trace, Modio::LogCategory::ModMetrics,
										"Setting metrics session to {}", (bActive ? "active" : "inactive"));
			bIsSessionActive = bActive;
			if (bActive)
			{
				StartSession();
			}
			else
			{
				EndSession();
			}
		}

		void MetricsService::StartSession() {}

		void MetricsService::EndSession()
		{
			ResetMetricsData();
		}

		void MetricsService::ResetMetricsData()
		{
			SessionId.reset();
			ModIds.clear();
			ModIds.shrink_to_fit();
			ModIdsString.clear();
			ModIdsString.shrink_to_fit();
			SessionStartTime = 0;
			LastHeartbeatPerformed = 0;
			CurrentSessionOrderId = 0;
			bIsSessionActive = false;
		}

		void MetricsService::AddMods(const std::vector<ModID>& Ids)
		{
			ModIds.insert(ModIds.end(), Ids.begin(), Ids.end());

			ModIdsString.reserve(256);
			for (auto Mod : ModIds)
			{
				ModIdsString += std::to_string(Mod) + ",";
			}
			ModIdsString.pop_back();

			ModIdsString.shrink_to_fit();
		}

		bool MetricsService::GetSessionIsActive() const
		{
			return bIsSessionActive;
		}

		Modio::Guid MetricsService::GetSessionId() const
		{
			if (!SessionId.has_value())
			{
				return Modio::Guid::InvalidGuid();
			}
			return SessionId.value();
		}

		std::vector<Modio::ModID> MetricsService::GetModIds() const
		{
			return ModIds;
		}

		std::string MetricsService::GetSecretKey() const
		{
			return SecretKey;
		}

		bool MetricsService::ContainsModId(Modio::ModID Id) const
		{
			return std::find(ModIds.begin(), ModIds.end(), Id) != ModIds.end();
		}

		MetricsSessionStartParams MetricsService::GenerateSessionStartParams()
		{
			MetricsSessionStartParams Request;

			// All session order IDs start at 1, so we reset our order counter when a new session starts.
			CurrentSessionOrderId = 1;

			SessionStartTime = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
			const Modio::Guid SessionStartNonce = Modio::Guid::GenerateGuid();

			Request.SessionId = SessionId.value();
			Request.SessionOrderId = CurrentSessionOrderId;
			Request.ModIds = ModIds;
			Request.SessionNonce = SessionStartNonce;
			Request.SessionTimestamp = SessionStartTime;

			const std::string StringToHash =
				ModIdsString + std::to_string(SessionStartTime) + *SessionId.value() + *SessionStartNonce;

			Request.SessionHash = Modio::Detail::Hash::HMACSHA256String(SecretKey, StringToHash);

			return Request;
		}

		MetricsSessionHeartbeatParams MetricsService::GenerateSessionHeartbeatParams()
		{
			MetricsSessionHeartbeatParams Request;
			CurrentSessionOrderId++;

			const auto HeartbeatTimestamp = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
			const Modio::Guid HeartbeatNonce = Modio::Guid::GenerateGuid();

			Request.SessionId = SessionId.value();
			Request.SessionOrderId = CurrentSessionOrderId;
			Request.SessionNonce = HeartbeatNonce;
			Request.SessionTimestamp = HeartbeatTimestamp;

			const std::string StringToHash = std::to_string(HeartbeatTimestamp) + *SessionId.value() + *HeartbeatNonce;
			Request.SessionHash = Modio::Detail::Hash::HMACSHA256String(SecretKey, StringToHash);
			return Request;
		}

		MetricsSessionEndParams MetricsService::GenerateSessionEndParams()
		{
			CurrentSessionOrderId++;

			MetricsSessionEndParams Request;

			const auto SessionEndTimestamp = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
			const Modio::Guid SessionEndNonce = Modio::Guid::GenerateGuid();

			Request.SessionId = SessionId.value();
			Request.SessionOrderId = CurrentSessionOrderId;
			Request.SessionNonce = SessionEndNonce;
			Request.SessionTimestamp = SessionEndTimestamp;

			std::string StringToHash = std::to_string(SessionEndTimestamp) + *SessionId.value() + *SessionEndNonce;
			Request.SessionHash = Modio::Detail::Hash::HMACSHA256String(SecretKey, StringToHash);

			return Request;
		}
	} // namespace Detail
} // namespace Modio