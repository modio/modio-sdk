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
#include "modio/detail/ops/monetization/GetAvailableUserEntitlementsOp.h"
namespace Modio
{
	namespace Detail
	{
		inline void GetAvailableUserEntitlementsGoogleAsync(
			Modio::EntitlementParams Params,
			std::function<void(Modio::ErrorCode, Modio::Optional<Modio::EntitlementList>)> Callback)
		{
			const Modio::Detail::HttpRequestParams RequestParams =
				Modio::Detail::GetUserEntitlementsRequest.AppendPayloadValue(
					Modio::Detail::Constants::APIStrings::Receipt,
					Params.ExtendedParameters[Modio::Detail::Constants::APIStrings::Receipt]);

			return ModioAsio::async_compose<std::function<void(Modio::ErrorCode, Modio::Optional<Modio::EntitlementList>)>,
									   void(Modio::ErrorCode, Modio::Optional<Modio::EntitlementList>)>(
				Modio::Detail::GetAvailableUserEntitlementsOp(RequestParams), Callback,
				Modio::Detail::Services::GetGlobalContext().get_executor());
		}
	} // namespace Detail
} // namespace Modio