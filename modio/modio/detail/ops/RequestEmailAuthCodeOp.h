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

#include "modio/detail/ops/userdata/VerifyUserAuthenticationOp.h"

#include <asio/yield.hpp>
namespace Modio::Detail
{
	class RequestEmailAuthCodeOp
	{
	public:
		RequestEmailAuthCodeOp(Modio::EmailAddress EmailAddress) : EmailAddress(EmailAddress) {}
		template<typename CoroType>
		void operator()(CoroType& Self, Modio::ErrorCode ec = {})
		{
			reenter(CoroutineState)
			{
				Modio::Detail::SDKSessionData::SetLastUsedEmailAddress(EmailAddress);

				Request = Modio::Detail::RequestEmailSecurityCodeRequest;
				if (EmailAddress.bURLEncodeEmail)
				{
					Request = Request.EncodeAndAppendPayloadValue(Modio::Detail::Constants::APIStrings::EmailAddress,
																  EmailAddress.InternalAddress);
				}
				else
				{
					Request = Request.AppendPayloadValue(Modio::Detail::Constants::APIStrings::EmailAddress,
														 EmailAddress.InternalAddress);
				}

				yield Modio::Detail::PerformRequestAndGetResponseAsync(ResponseBuffer, Request,
																	   CachedResponse::Disallow, std::move(Self));

				Self.complete(ec);
				return;
			}
		}

	private:
		ModioAsio::coroutine CoroutineState {};
		Modio::Detail::DynamicBuffer ResponseBuffer {};
		Modio::EmailAddress EmailAddress;
		Modio::Detail::HttpRequestParams Request;
	};

	template<typename RequestEmailAuthCodeCompleteCallback>
	void RequestEmailAuthCodeAsync(Modio::EmailAddress EmailAddress,
								   RequestEmailAuthCodeCompleteCallback&& OnRequestComplete)
	{
		return ModioAsio::async_compose<RequestEmailAuthCodeCompleteCallback, void(Modio::ErrorCode)>(
			Modio::Detail::RequestEmailAuthCodeOp(EmailAddress), OnRequestComplete,
			Modio::Detail::Services::GetGlobalContext().get_executor());
	}

} // namespace Modio::Detail

#include <asio/unyield.hpp>
