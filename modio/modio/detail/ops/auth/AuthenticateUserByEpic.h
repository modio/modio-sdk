#pragma once
#include "modio/detail/AsioWrapper.h"
#include "modio/detail/ops/auth/AuthenticateUserExternal.h"

namespace Modio
{
	namespace Detail
	{

		//unimplemented for now
/*
		{
		void AuthenticateUserByEpicAsync(Modio::AuthenticationParams User, std::function<void(Modio::ErrorCode)> Callback)
		{
						Modio::Detail::HttpRequestParams Params =
								Modio::AuthenticateViaEpicRequest
									.AppendPayloadValue(Modio::Detail::Constants::APIStrings::ItchToken, User.AuthToken)
									.AppendPayloadValue(Modio::Detail::Constants::APIStrings::EmailAddress,
		   User.UserEmail) .AppendPayloadValue(Modio::Detail::Constants::APIStrings::TermsAgreed,
															User.bUserHasAcceptedTerms ? "true" : "false");
			return asio::async_compose<std::function<void(Modio::ErrorCode)>, void(Modio::ErrorCode)>(
				AuthenticateUserExternal(User, Modio::AuthenticateViaEpic), Callback, Modio::Detail::Services::GetGlobalContext().get_executor());
		}
		}
*/
	}
}