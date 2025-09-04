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

#include "modio/core/ModioReportParams.h"
#include "modio/detail/AsioWrapper.h"
#include "modio/detail/ops/http/PerformRequestAndGetResponseOp.h"
#include "modio/http/ModioHttpParams.h"

#include <asio/yield.hpp>
namespace Modio
{
	namespace Detail
	{
		class ReportContentOp
		{
			Modio::Detail::DynamicBuffer ResponseBodyBuffer {};

			asio::coroutine CoroutineState {};
			Modio::Detail::HttpRequestParams Request {};

		public:
			ReportContentOp(Modio::ReportParams Params)
			{
				Request = ToRequest(Params);
			}

			template<typename CoroType>
			void operator()(CoroType& Self, Modio::ErrorCode ec = {})
			{
				reenter(CoroutineState)
				{
					yield Modio::Detail::PerformRequestAndGetResponseAsync(
						ResponseBodyBuffer, Request, Modio::Detail::CachedResponse::Disallow, std::move(Self));
					Self.complete(ec);
					return;
				}
			}
		};

		template<typename ReportContentCompleteCallback>
		void ReportContentAsync(Modio::ReportParams Params, ReportContentCompleteCallback&& OnReportContentComplete)
		{
			return asio::async_compose<ReportContentCompleteCallback, void(Modio::ErrorCode)>(
				Modio::Detail::ReportContentOp(Params), OnReportContentComplete,
				Modio::Detail::Services::GetGlobalContext().get_executor());
		}
	} // namespace Detail
} // namespace Modio
#include <asio/unyield.hpp>