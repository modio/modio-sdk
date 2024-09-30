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
#include "MetricsSessionSendHeartbeatOnceOp.h"
#include "modio/core/ModioBuffer.h"
#include "modio/core/ModioServices.h"
#include <asio/coroutine.hpp>

#include <asio/yield.hpp>
namespace Modio
{
	namespace Detail
	{
		class MetricsSessionSendHeartbeatAtIntervalOp
		{
		public:
			MetricsSessionSendHeartbeatAtIntervalOp(uint32_t SecondsInterval)
			{
				Interval = SecondsInterval;
			}

			template<typename CoroType>
			void operator()(CoroType& Self, Modio::ErrorCode ec = {})
			{
				if (ec == Modio::GenericError::OperationCanceled)
				{
					Self.complete(ec);
					return;
				}

				reenter(CoroutineState)
				{
					while (
						Modio::Detail::Services::GetGlobalService<Modio::Detail::MetricsService>().GetSessionIsActive())
					{
						yield Modio::Detail::MetricsSessionSendHeartbeatOnceOpAsync(std::move(Self));

						if (ec)
						{
							if (Modio::ErrorCodeMatches(ec, Modio::ErrorConditionTypes::UserNotAuthenticatedError))
							{
								Modio::Detail::SDKSessionData::InvalidateOAuthToken();
							}

							// If we have any errors when performing a heartbeat, we end the session now.
							Modio::Detail::Services::GetGlobalService<Modio::Detail::MetricsService>()
								.SetSessionIsActive(false);
							Self.complete(ec);
							return;
						}

						Timer.ExpiresAfter(std::chrono::seconds(Interval));
						yield Timer.WaitAsync(std::move(Self));
					}

					Self.complete({});
					return;
				}
			}

		private:
			Modio::Detail::Timer Timer;
			uint32_t Interval;
			nlohmann::json BodyJson;
			Modio::Detail::DynamicBuffer ResponseBodyBuffer;
			asio::coroutine CoroutineState;
		};

		template<typename Callback>
		auto MetricsSessionSendHeartbeatAtIntervalOpAsync(uint32_t SecondsInterval, Callback&& OnProcessComplete)
		{
			return asio::async_compose<Callback, void(Modio::ErrorCode)>(
				MetricsSessionSendHeartbeatAtIntervalOp(SecondsInterval), OnProcessComplete,
				Modio::Detail::Services::GetGlobalContext().get_executor());
		}
	} // namespace Detail
} // namespace Modio

#include <asio/unyield.hpp>