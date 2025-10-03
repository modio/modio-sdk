/*
 *  Copyright (C) 2025 mod.io Pty Ltd. <https://mod.io>
 *
 *  This file is part of the mod.io SDK.
 *
 *  Distributed under the MIT License. (See accompanying file LICENSE or
 *   view online at <https://github.com/modio/modio-sdk/blob/main/LICENSE>)
 *
 */

#pragma once
#include "modio/detail/AsioWrapper.h"
#include "modio/detail/ops/monetization/PurchaseModOp.h"
namespace Modio
{
	namespace Detail
	{
		inline void PurchaseModWithEntitlementPSNAsync(
			Modio::ModID ModID, Modio::EntitlementParams Params,
			std::function<void(Modio::ErrorCode, Modio::Optional<Modio::TransactionRecord>)> Callback)
		{
			Modio::Detail::HttpRequestParams RequestParams =
				Modio::Detail::PurchaseRequest
					.AppendPayloadValue("psn_auth_code",
										Params.ExtendedParameters[Modio::Detail::Constants::APIStrings::AuthCode])
					.AddQueryParamRaw("type", "1");

			if (Modio::Detail::SDKSessionData::GetPlatformEnvironment().has_value())
			{
				RequestParams = RequestParams.AppendPayloadValue(
					"psn_env", Modio::Detail::SDKSessionData::GetPlatformEnvironment().value());
			}

			auto ParamIterator = Params.ExtendedParameters.find("service_label");
			if (ParamIterator != Params.ExtendedParameters.end())
			{
				RequestParams = RequestParams.AppendPayloadValue("psn_service_label", ParamIterator->second);
			}

			return asio::async_compose<std::function<void(Modio::ErrorCode, Modio::Optional<Modio::TransactionRecord>)>,
									   void(Modio::ErrorCode, Modio::Optional<Modio::TransactionRecord>)>(
				Modio::Detail::PurchaseModOp(Modio::Detail::SDKSessionData::CurrentGameID(),
											 Modio::Detail::SDKSessionData::CurrentAPIKey(), ModID, RequestParams),
				Callback, Modio::Detail::Services::GetGlobalContext().get_executor());
		}
	} // namespace Detail
} // namespace Modio