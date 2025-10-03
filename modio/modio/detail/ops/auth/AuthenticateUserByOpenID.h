/*
 *  Copyright (C) 2021-2023 mod.io Pty Ltd. <https://mod.io>
 *
 *  This file is part of the mod.io SDK.
 *
 *  Distributed under the MIT License. (See accompanying file LICENSE or
 *   view online at <https://github.com/modio/modio-sdk/blob/main/LICENSE>)
 *
 */

#pragma once

#include "modio/detail/AsioWrapper.h"
#include "modio/detail/ModioConstants.h"
#include "modio/detail/ops/auth/AuthenticateUserExternal.h"
#include "modio/http/ModioHttpParams.h"

namespace Modio
{
	namespace Detail
	{
		inline void AuthenticateUserByOpenIDAsync(Modio::AuthenticationParams User,
												  std::function<void(Modio::ErrorCode)> Callback)
		{
			Modio::Detail::HttpRequestParams Params =
				Modio::Detail::AuthenticateViaOpenidRequest
					.AppendPayloadValue(Modio::Detail::Constants::APIStrings::OpenIDToken, User.AuthToken)
					.EncodeAndAppendPayloadValue(Modio::Detail::Constants::APIStrings::EmailAddress, User.UserEmail)
					.AppendPayloadValue(Modio::Detail::Constants::APIStrings::TermsAgreed,
										User.bUserHasAcceptedTerms ? "true" : "false");

			auto ParamIterator = User.ExtendedParameters.find("psn_token");
			if (ParamIterator != User.ExtendedParameters.end())
			{
				Params = Params.AppendPayloadValue("psn_token", ParamIterator->second);
			}

			if (Modio::Detail::SDKSessionData::GetPlatformEnvironment().has_value())
			{
				Params = Params.AppendPayloadValue("psn_env",
												   Modio::Detail::SDKSessionData::GetPlatformEnvironment().value());
			}
			else
			{
				ParamIterator = User.ExtendedParameters.find("psn_env");
				if (ParamIterator != User.ExtendedParameters.end())
				{
					Params = Params.AppendPayloadValue("psn_env", ParamIterator->second);
				}
			}

			Modio::Detail::AuthenticateUserExternalAsync(Params, Callback);
		}
	} // namespace Detail
} // namespace Modio