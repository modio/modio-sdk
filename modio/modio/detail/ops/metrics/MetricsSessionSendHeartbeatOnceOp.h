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
#include "modio/core/ModioBuffer.h"
#include "modio/core/ModioServices.h"
#include "modio/core/entities/ModioMetricsSessionHeartbeatParams.h"
#include "modio/detail/serialization/ModioMetricsSessionHeartbeatParamsSerialization.h"
#include <asio/coroutine.hpp>

#include <asio/yield.hpp>
namespace Modio
{
	namespace Detail
	{
		class MetricsSessionSendHeartbeatOnceOp
		{
		public:
			MetricsSessionSendHeartbeatOnceOp(const MetricsSessionHeartbeatParams RequestParams)
				: RequestParams(RequestParams)
			{}

			template<typename CoroType>
			void operator()(CoroType& Self, Modio::ErrorCode ec = {})
			{
				reenter(CoroutineState)
				{
					Modio::Detail::Logger().Log(Modio::LogLevel::Trace, Modio::LogCategory::ModMetrics,
												"MetricsHeartbeatRequest. Order ID: {}", RequestParams.SessionOrderId);
					to_json(BodyJson, RequestParams);

					{
						yield Modio::Detail::PerformRequestAndGetResponseAsync(
							ResponseBodyBuffer,
							Modio::Detail::MetricsSessionHeartbeatRequest.AppendJsonPayloadValue(BodyJson.dump()),
							Modio::Detail::CachedResponse::Allow, std::move(Self));
					}

					if (ec)
					{
						if (Modio::ErrorCodeMatches(ec, Modio::ErrorConditionTypes::UserNotAuthenticatedError))
						{
							Modio::Detail::SDKSessionData::InvalidateOAuthToken();
						}

						// If we have any errors when performing a heartbeat, we end the session now.
						Modio::Detail::Services::GetGlobalService<Modio::Detail::MetricsService>().SetSessionIsActive(
							false);
						Self.complete(ec);
						return;
					}

					Modio::Detail::Services::GetGlobalService<Modio::Detail::MetricsService>().HeartbeatPerformed();
					Self.complete({});
					return;
				}
			}

		private:
			nlohmann::json BodyJson;
			Modio::Detail::DynamicBuffer ResponseBodyBuffer;
			asio::coroutine CoroutineState;
			MetricsSessionHeartbeatParams RequestParams;
		};

		template<typename Callback>
		auto MetricsSessionSendHeartbeatOnceOpAsync(Callback&& OnProcessComplete)
		{
			auto Request = Modio::Detail::Services::GetGlobalService<Modio::Detail::MetricsService>()
							   .GenerateSessionHeartbeatParams();
			return asio::async_compose<Callback, void(Modio::ErrorCode)>(
				MetricsSessionSendHeartbeatOnceOp(Request), OnProcessComplete,
				Modio::Detail::Services::GetGlobalContext().get_executor());
		}
	} // namespace Detail
} // namespace Modio

#include <asio/unyield.hpp>