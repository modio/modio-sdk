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

#include "modio/core/ModioBuffer.h"
#include "modio/core/ModioDefaultRequestParameters.h"
#include "modio/core/ModioStdTypes.h"
#include "modio/detail/CoreOps.h"
#include "modio/detail/ModioJsonHelpers.h"

#include "modio/detail/AsioWrapper.h"

#include <asio/yield.hpp>
namespace Modio
{
	namespace Detail
	{
		class GetTermsOfUseOp
		{
		public:
			GetTermsOfUseOp(Modio::AuthenticationProvider Provider, Modio::Language Locale)
				: Provider(Provider),
				  Locale(Locale)
			{}

			template<typename CoroType>
			void operator()(CoroType& Self, Modio::ErrorCode ec = {})
			{
				reenter(CoroutineState)
				{
					yield Modio::Detail::ComposedOps::PerformRequestAndGetResponseAsync(
						ResponseBodyBuffer,
						Modio::Detail::TermsRequest.SetLocale(Locale).AppendPayloadValue(
							"service", Modio::Detail::ToString(Provider)),
						Modio::Detail::CachedResponse::Allow, std::move(Self));

					if (ec)
					{
						Self.complete(ec, {});
						return;
					}
					{
						auto TermsData = TryMarshalResponse<Modio::Terms>(ResponseBodyBuffer);
						if (TermsData.has_value())
						{
							Self.complete(ec, TermsData);
						}
						else
						{
							Self.complete(Modio::make_error_code(Modio::HttpError::InvalidResponse), {});
						}
					}
					return;
				}
			}

		private:
			Modio::Detail::DynamicBuffer ResponseBodyBuffer;
			Modio::AuthenticationProvider Provider;
			Modio::Language Locale;

			asio::coroutine CoroutineState;
		};
	} // namespace Detail
} // namespace Modio
#include <asio/unyield.hpp>
