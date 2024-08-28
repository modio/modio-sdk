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
#include "modio/detail/AsioWrapper.h"
#include "modio/detail/ops/monetization/RefreshUserEntitlementsOp.h"
namespace Modio
{
	namespace Detail
	{
		inline void RefreshUserEntitlementsPSNAsync(Modio::EntitlementParams Params,
			std::function<void(Modio::ErrorCode, Modio::Optional<Modio::EntitlementConsumptionStatusList>)> Callback)
		{
			Modio::Detail::HttpRequestParams RequestParams = Modio::Detail::SyncPlaystationNetworkEntitlementsRequest.AppendPayloadValue(
					Modio::Detail::Constants::APIStrings::AuthCode,
					Params.ExtendedParameters[Modio::Detail::Constants::APIStrings::AuthCode]);

			if (Modio::Detail::SDKSessionData::GetPlatformEnvironment().has_value())
			{
				RequestParams = RequestParams.AppendPayloadValue(
					"env", Modio::Detail::SDKSessionData::GetPlatformEnvironment().value());
			}

			auto ParamIterator = Params.ExtendedParameters.find("service_label");
			if (ParamIterator != Params.ExtendedParameters.end())
			{
				RequestParams = RequestParams.AppendPayloadValue("service_label", ParamIterator->second);
			}

			return asio::async_compose<
				std::function<void(Modio::ErrorCode, Modio::Optional<Modio::EntitlementConsumptionStatusList>)>,
				void(Modio::ErrorCode, Modio::Optional<Modio::EntitlementConsumptionStatusList>)>(
				Modio::Detail::RefreshUserEntitlementsOp(RequestParams), Callback,
				Modio::Detail::Services::GetGlobalContext().get_executor());
		}
	} // namespace Detail
} // namespace Modio