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
		inline void PurchaseModWithEntitlementXboxLiveAsync(
			Modio::ModID ModID, Modio::EntitlementParams Params,
			std::function<void(Modio::ErrorCode, Modio::Optional<Modio::TransactionRecord>)> OnPurchaseComplete)
		{
			const Modio::Detail::HttpRequestParams RequestParams =
				Modio::Detail::PurchaseRequest
					.AppendPayloadValue("xbox_token",
										Params.ExtendedParameters[Modio::Detail::Constants::APIStrings::XboxToken])
					.AddQueryParamRaw("type", "1");

			return ModioAsio::async_compose<std::function<void(Modio::ErrorCode, Modio::Optional<Modio::TransactionRecord>)>,
									   void(Modio::ErrorCode, Modio::Optional<Modio::TransactionRecord>)>(
				Modio::Detail::PurchaseModOp(Modio::Detail::SDKSessionData::CurrentGameID(),
											 Modio::Detail::SDKSessionData::CurrentAPIKey(), ModID, RequestParams),
				OnPurchaseComplete, Modio::Detail::Services::GetGlobalContext().get_executor());
		}
	} // namespace Detail
} // namespace Modio