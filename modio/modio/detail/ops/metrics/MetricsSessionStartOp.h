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
#include "modio/core/ModioCoreTypes.h"
#include "modio/core/ModioServices.h"
#include "modio/core/entities/ModioMetricsSessionStartParams.h"
#include "modio/detail/ops/http/PerformRequestAndGetResponseOp.h"
#include "modio/detail/serialization/ModioMetricsSessionStartParamsSerialization.h"
#include <asio/coroutine.hpp>

#include <asio/yield.hpp>
namespace Modio
{
	namespace Detail
	{
		class MetricsSessionStartOp
		{
		public:
			MetricsSessionStartOp(MetricsSessionStartParams RequestParams) : RequestParams(RequestParams) {}

			template<typename CoroType>
			void operator()(CoroType& Self, Modio::ErrorCode ec = {})
			{
				reenter(CoroutineState)
				{
					to_json(BodyJson, RequestParams);

					{
						yield Modio::Detail::PerformRequestAndGetResponseAsync(
							ResponseBodyBuffer,
							Modio::Detail::MetricsSessionStartRequest.AppendJsonPayloadValue(BodyJson.dump()),
							Modio::Detail::CachedResponse::Allow, std::move(Self));
					}

					if (ec)
					{
						if (Modio::ErrorCodeMatches(ec, Modio::ErrorConditionTypes::UserNotAuthenticatedError))
						{
							Modio::Detail::SDKSessionData::InvalidateOAuthToken();
						}

						Modio::Detail::Services::GetGlobalService<Modio::Detail::MetricsService>().SetSessionIsActive(
							false);
						Self.complete(ec);
						return;
					}

					// Start our session if we encountered no errors
					Modio::Detail::Services::GetGlobalService<Modio::Detail::MetricsService>().SetSessionIsActive(true);

					Self.complete({});
					return;
				}
			}

		private:
			nlohmann::json BodyJson {};
			Modio::Detail::DynamicBuffer ResponseBodyBuffer {};
			ModioAsio::coroutine CoroutineState {};
			MetricsSessionStartParams RequestParams {};
		};

		template<typename Callback>
		auto MetricsSessionStartOpAsync(Callback&& OnProcessComplete)
		{
			auto Request =
				Modio::Detail::Services::GetGlobalService<Modio::Detail::MetricsService>().GenerateSessionStartParams();
			return ModioAsio::async_compose<Callback, void(Modio::ErrorCode)>(
				MetricsSessionStartOp(Request), OnProcessComplete,
				Modio::Detail::Services::GetGlobalContext().get_executor());
		}
	} // namespace Detail
} // namespace Modio

#include <asio/unyield.hpp>