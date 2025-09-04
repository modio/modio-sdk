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
#include "modio/detail/ops/auth/AuthenticateUserExternal.h"

namespace Modio
{
	namespace Detail
	{
		inline void AuthenticateUserByOculusAsync(Modio::AuthenticationParams User,
												  std::function<void(Modio::ErrorCode)> Callback)
		{
			Modio::Detail::HttpRequestParams Params =
				Modio::Detail::AuthenticateViaOculusRequest
					.AppendPayloadValue(Modio::Detail::Constants::APIStrings::AccessToken, User.AuthToken)
					.AppendPayloadValue(Modio::Detail::Constants::APIStrings::Device,
										User.ExtendedParameters[Modio::Detail::Constants::APIStrings::Device])
					.AppendPayloadValue(Modio::Detail::Constants::APIStrings::UserID,
										User.ExtendedParameters[Modio::Detail::Constants::APIStrings::UserID])
					.AppendPayloadValue(Modio::Detail::Constants::APIStrings::Nonce,
										User.ExtendedParameters[Modio::Detail::Constants::APIStrings::Nonce])
					.EncodeAndAppendPayloadValue(Modio::Detail::Constants::APIStrings::EmailAddress, User.UserEmail)
					.AppendPayloadValue(Modio::Detail::Constants::APIStrings::TermsAgreed,
										User.bUserHasAcceptedTerms ? "true" : "false");
			Modio::Detail::AuthenticateUserExternalAsync(Params, Callback);
		}
	} // namespace Detail
} // namespace Modio