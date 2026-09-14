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

#include "modio/detail/ops/auth/AuthenticateUserExternal.h"

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

			auto ParamIterator = User.ExtendedParameters.find(Modio::Detail::Constants::APIStrings::PsToken);
			if (ParamIterator != User.ExtendedParameters.end())
			{
				Params =
					Params.AppendPayloadValue(Modio::Detail::Constants::APIStrings::PsToken, ParamIterator->second);
			}

			if (Modio::Detail::SDKSessionData::GetPlatformEnvironment().has_value())
			{
				Params = Params.AppendPayloadValue(Modio::Detail::Constants::APIStrings::PsEnv,
												   Modio::Detail::SDKSessionData::GetPlatformEnvironment().value());
			}
			else
			{
				ParamIterator = User.ExtendedParameters.find(Modio::Detail::Constants::APIStrings::PsEnv);
				if (ParamIterator != User.ExtendedParameters.end())
				{
					Params =
						Params.AppendPayloadValue(Modio::Detail::Constants::APIStrings::PsEnv, ParamIterator->second);
				}
			}

			Modio::Detail::AuthenticateUserExternalAsync(Params, Callback);
		}
	} // namespace Detail
} // namespace Modio