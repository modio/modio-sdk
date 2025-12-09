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
#include "modio/detail/ops/http/PerformRequestAndGetResponseOp.h"
#include "modio/detail/ops/userdata/VerifyUserAuthenticationOp.h"
#include "modio/http/ModioHttpParams.h"

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
				yield Modio::Detail::PerformRequestAndGetResponseAsync(
					ResponseBuffer,
					Modio::Detail::RequestEmailSecurityCodeRequest.EncodeAndAppendPayloadValue(
						Modio::Detail::Constants::APIStrings::EmailAddress, EmailAddress.InternalAddress),
					CachedResponse::Disallow, std::move(Self));

				Self.complete(ec);
				return;
			}
		}

	private:
		ModioAsio::coroutine CoroutineState {};
		Modio::Detail::DynamicBuffer ResponseBuffer {};
		Modio::EmailAddress EmailAddress;
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
