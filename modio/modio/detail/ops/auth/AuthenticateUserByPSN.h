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
#include "modio/core/ModioCoreTypes.h"
#include "modio/detail/AsioWrapper.h"
#include "modio/detail/ops/auth/AuthenticateUserExternal.h"
namespace Modio
{
	namespace Detail
	{
		inline void AuthenticateUserByPSNAsync(Modio::AuthenticationParams User,
											   std::function<void(Modio::ErrorCode)> Callback)
		{
			Modio::Detail::HttpRequestParams Params =
				Modio::Detail::AuthenticateViaPsnRequest
					.AppendPayloadValue(Modio::Detail::Constants::APIStrings::AuthCode, User.AuthToken)
					.EncodeAndAppendPayloadValue(Modio::Detail::Constants::APIStrings::EmailAddress, User.UserEmail)
					.AppendPayloadValue(Modio::Detail::Constants::APIStrings::TermsAgreed,
										User.bUserHasAcceptedTerms ? "true" : "false");

			// If the env parameter is present in the extended init params, use that
			auto ParamIterator = User.ExtendedParameters.find("env");
			if (ParamIterator != User.ExtendedParameters.end())
			{
				Params = Params.AppendPayloadValue("env", ParamIterator->second);
			}
			// Otherwise, see if we have a platform 
			else if (Modio::Detail::SDKSessionData::GetPlatformEnvironment().has_value())
			{
				Params = Params.AppendPayloadValue("env", Modio::Detail::SDKSessionData::GetPlatformEnvironment().value());
			}
			// Otherwise, fall back to the default env
			else
			{
				Params = Params.AppendPayloadValue("env", "256");
			}
			return asio::async_compose<std::function<void(Modio::ErrorCode)>, void(Modio::ErrorCode)>(
				Modio::Detail::AuthenticateUserExternal(Params), Callback,
				Modio::Detail::Services::GetGlobalContext().get_executor());
		}
	} // namespace Detail
} // namespace Modio