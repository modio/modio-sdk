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
#include "modio/http/ModioHttpParams.h"

namespace Modio
{
	namespace Detail
	{

		inline void AuthenticateUserByEpicAsync(Modio::AuthenticationParams User,
										 std::function<void(Modio::ErrorCode)> Callback)
		{
			Modio::Detail::HttpRequestParams Params =
				Modio::Detail::AuthenticateViaEpicgamesRequest
					.AppendPayloadValue(Modio::Detail::Constants::APIStrings::AccessToken, User.AuthToken)
					.EncodeAndAppendPayloadValue(Modio::Detail::Constants::APIStrings::EmailAddress, User.UserEmail)
					.AppendPayloadValue(Modio::Detail::Constants::APIStrings::TermsAgreed,
										User.bUserHasAcceptedTerms ? "true" : "false");
			Modio::Detail::AuthenticateUserExternalAsync(Params, Callback);
		}

	} // namespace Detail
} // namespace Modio