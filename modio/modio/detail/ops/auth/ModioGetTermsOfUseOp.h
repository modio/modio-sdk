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
#include "modio/detail/ModioJsonHelpers.h"
#include "modio/detail/ops/http/PerformRequestAndGetResponseOp.h"

#include "modio/detail/AsioWrapper.h"

#include <asio/yield.hpp>
namespace Modio
{
	namespace Detail
	{
		class GetTermsOfUseOp
		{
		public:
			GetTermsOfUseOp()
			{}

			template<typename CoroType>
			void operator()(CoroType& Self, Modio::ErrorCode ec = {})
			{
				reenter(CoroutineState)
				{
					CachedResponse = Modio::Detail::SDKSessionData::IsTermsOfUseCacheInvalid()
										 ? Modio::Detail::CachedResponse::Disallow
										 : Modio::Detail::CachedResponse::Allow;

					yield Modio::Detail::PerformRequestAndGetResponseAsync(
						ResponseBodyBuffer, Modio::Detail::TermsRequest, CachedResponse, std::move(Self));

					Modio::Detail::SDKSessionData::ClearTermsOfUseCache();

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
			Modio::Detail::CachedResponse CachedResponse;
			asio::coroutine CoroutineState;
		};
	} // namespace Detail
} // namespace Modio
#include <asio/unyield.hpp>
