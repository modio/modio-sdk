/*
 *  Copyright (C) 2024 mod.io Pty Ltd. <https://mod.io>
 *
 *  This file is part of the mod.io SDK.
 *
 *  Distributed under the MIT License. (See accompanying file LICENSE or
 *   view online at <https://github.com/modio/modio-sdk/blob/main/LICENSE>)
 *
 */

#pragma once
#include "modio/core/ModioCoreTypes.h"
#include "modio/core/ModioLogger.h"

#include "modio/detail/AsioWrapper.h"

namespace Modio
{
	struct MetricsSessionParams;
	struct MetricsSessionStartParams;
	struct MetricsSessionHeartbeatParams;
	struct MetricsSessionEndParams;

	namespace Detail
	{
		/// @docinternal
		/// @brief Class to manage metric session data
		class MetricsService : public asio::detail::service_base<MetricsService>
		{
		public:
			explicit MetricsService(asio::io_context& IOService)
				: asio::detail::service_base<MetricsService>(IOService),
				  SessionId(Modio::Guid::InvalidGuid()),
				  SessionStartTime(0),
				  CurrentSessionOrderId(0),
				  LastHeartbeatPerformed(0),
				  bIsSessionActive(false),
				  bIsInitialized(false)
			{}
			MetricsService(MetricsService&&) = delete;

			MODIO_IMPL void Shutdown() {}

			MODIO_IMPL void InitMetricsSession(const std::string& SecretKeyString);
			MODIO_IMPL std::string GetSecretKey() const;

			MODIO_IMPL void StartSessionAsync(Modio::MetricsSessionParams Params,
											  std::function<void(Modio::ErrorCode)> Callback);
			MODIO_IMPL void SendHeartbeatOnceAsync(std::function<void(Modio::ErrorCode)> Callback);
			MODIO_IMPL void SendHeartbeatAtIntervalAsync(uint32_t IntervalSeconds,
														 std::function<void(Modio::ErrorCode)> Callback);
			MODIO_IMPL void EndSessionAsync(std::function<void(Modio::ErrorCode)> Callback);

			MODIO_IMPL MetricsSessionStartParams GenerateSessionStartParams();
			MODIO_IMPL MetricsSessionHeartbeatParams GenerateSessionHeartbeatParams();
			MODIO_IMPL MetricsSessionEndParams GenerateSessionEndParams();

			MODIO_IMPL void SetSessionIsActive(bool bActive);
			MODIO_IMPL bool GetSessionIsActive() const;
			MODIO_IMPL Modio::Guid GetSessionId() const;

			MODIO_IMPL std::vector<ModID> GetModIds() const;
			MODIO_IMPL bool ContainsModId(Modio::ModID Id) const;

			bool IsInitialized() const
			{
				return bIsInitialized;
			}

			void HeartbeatPerformed()
			{
				LastHeartbeatPerformed = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
			}

			time_t GetLastHeartbeatPerformed() const
			{
				return LastHeartbeatPerformed;
			}

			time_t GetTimeSinceLastHeartbeatPerformed() const
			{
				if (LastHeartbeatPerformed > 0)
				{
					return std::chrono::system_clock::to_time_t(std::chrono::system_clock::now()) -
						   LastHeartbeatPerformed;
				}
				return 0;
			}

			time_t GetSessionStartTime() const
			{
				return SessionStartTime;
			}

			time_t GetSessionDuration() const
			{
				if (!GetSessionIsActive())
				{
					Modio::Detail::Logger().Log(Modio::LogLevel::Warning, Modio::LogCategory::ModMetrics,
												"Cannot get session duration when no session is active");
					return 0;
				}
				return std::chrono::system_clock::to_time_t(std::chrono::system_clock::now()) - SessionStartTime;
			}

		private:
			MODIO_IMPL void StartSession();
			MODIO_IMPL void EndSession();
			MODIO_IMPL void ResetMetricsData();

			MODIO_IMPL void AddMods(const std::vector<ModID>& Ids);

			std::string SecretKey {};
			Modio::Optional<Modio::Guid> SessionId {};
			// The format of the ModID list for hashing purposes is a comma separated list of
			// all ModIDs for this session all ModIDs for this session
			std::string ModIdsString {};
			std::vector<ModID> ModIds {};
			time_t SessionStartTime = 0;
			uint64_t CurrentSessionOrderId = 0;

			time_t LastHeartbeatPerformed = 0;

			bool bIsSessionActive = false;
			bool bIsInitialized = false;
		};

	} // namespace Detail
} // namespace Modio

#ifndef MODIO_SEPARATE_COMPILATION
	#include "ModioMetricsService.ipp"
#endif