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
namespace Modio
{
	namespace Detail
	{
		class RequestEmailAuthCodeOp
		{
		public:
			RequestEmailAuthCodeOp(Modio::EmailAddress EmailAddress) : EmailAddress(EmailAddress) {};
			template<typename CoroType>
			void operator()(CoroType& Self, Modio::ErrorCode ec = {})
			{
				reenter(CoroutineState)
				{
					yield Modio::Detail::VerifyUserAuthenticationAsync(std::move(Self));
					// No error during verification indicates the user is already authenticated
					if (!ec)
					{
						Self.complete(Modio::make_error_code(Modio::UserAuthError::AlreadyAuthenticated));
						return;
					}

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
			asio::coroutine CoroutineState;
			Modio::Detail::DynamicBuffer ResponseBuffer;
			Modio::EmailAddress EmailAddress;
		};
	} // namespace Detail
} // namespace Modio

#include <asio/unyield.hpp>