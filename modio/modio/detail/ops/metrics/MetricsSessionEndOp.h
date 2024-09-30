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
#include "modio/core/entities/ModioMetricsSessionEndParams.h"
#include "modio/detail/ModioHashHelpers.h"
#include "modio/detail/ops/http/PerformRequestAndGetResponseOp.h"
#include "modio/detail/serialization/ModioMetricsSessionEndParamsSerialization.h"
#include <asio/coroutine.hpp>

#include <asio/yield.hpp>
namespace Modio
{
	namespace Detail
	{
		class MetricsSessionEndOp
		{
		public:
			MetricsSessionEndOp(MetricsSessionEndParams RequestParams) : RequestParams(RequestParams) {}

			template<typename CoroType>
			void operator()(CoroType& Self, Modio::ErrorCode ec = {})
			{
				reenter(CoroutineState)
				{
					Modio::Detail::Logger().Log(Modio::LogLevel::Trace, Modio::LogCategory::ModMetrics,
												"MetricsEndSessionRequest. Order ID: {}", RequestParams.SessionOrderId);
					to_json(BodyJson, RequestParams);

					yield Modio::Detail::PerformRequestAndGetResponseAsync(
						ResponseBodyBuffer,
						Modio::Detail::MetricsSessionEndRequest.AppendJsonPayloadValue(BodyJson.dump()),
						Modio::Detail::CachedResponse::Allow, std::move(Self));

					Modio::Detail::Services::GetGlobalService<Modio::Detail::MetricsService>().SetSessionIsActive(
						false);

					if (ec)
					{
						if (Modio::ErrorCodeMatches(ec, Modio::ErrorConditionTypes::UserNotAuthenticatedError))
						{
							Modio::Detail::SDKSessionData::InvalidateOAuthToken();
						}

						Self.complete(ec);
						return;
					}

					Self.complete({});
					return;
				}
			}

		private:
			nlohmann::json BodyJson;
			Modio::Detail::DynamicBuffer ResponseBodyBuffer;
			asio::coroutine CoroutineState;
			MetricsSessionEndParams RequestParams;
		};

		template<typename Callback>
		auto MetricsSessionEndOpAsync(Callback&& OnProcessComplete)
		{
			auto Request =
				Modio::Detail::Services::GetGlobalService<Modio::Detail::MetricsService>().GenerateSessionEndParams();
			return asio::async_compose<Callback, void(Modio::ErrorCode)>(
				MetricsSessionEndOp(Request), OnProcessComplete,
				Modio::Detail::Services::GetGlobalContext().get_executor());
		}
	} // namespace Detail
} // namespace Modio

#include <asio/unyield.hpp>