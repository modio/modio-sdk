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
#include "modio/core/entities/ModioModTagOptions.h"
#include "modio/detail/AsioWrapper.h"
#include "modio/detail/CoreOps.h"
#include "modio/detail/ModioJsonHelpers.h"
#include "modio/detail/ModioSDKSessionData.h"
#include "modio/http/ModioHttpParams.h"

#include <asio/yield.hpp>
namespace Modio
{
	namespace Detail
	{
		class GetModTagsOp
		{
		public:
			template<typename CoroType>
			void operator()(CoroType& Self, Modio::ErrorCode ec = {})
			{
				reenter(CoroutineState)
				{
					yield Modio::Detail::ComposedOps::PerformRequestAndGetResponseAsync(
						TagResponseBuffer,
						Modio::Detail::GetGameTagOptionsRequest.SetGameID(
							Modio::Detail::SDKSessionData::CurrentGameID()),
						Modio::Detail::CachedResponse::Allow, std::move(Self));
					if (ec)
					{
						Self.complete(ec, {});
						return;
					}
					{
						Modio::Optional<Modio::ModTagOptions> Options =
							TryMarshalResponse<Modio::ModTagOptions>(TagResponseBuffer);
						// need to convert this to our own tag object here then
						if (Options.has_value())
						{
							Self.complete({}, Options);
						}
						else
						{
							Self.complete(Modio::make_error_code(Modio::HttpError::InvalidResponse), {});
						}
					}
				}
			}

		private:
			asio::coroutine CoroutineState;
			Modio::Detail::DynamicBuffer TagResponseBuffer;
		};
	} // namespace Detail
} // namespace Modio
#include <asio/unyield.hpp>