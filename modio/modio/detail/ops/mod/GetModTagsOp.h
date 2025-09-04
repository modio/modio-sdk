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
#include "modio/detail/serialization/ModioModTagOptionsSerialization.h"
#include "modio/detail/AsioWrapper.h"
#include "modio/detail/ModioJsonHelpers.h"
#include "modio/detail/ModioSDKSessionData.h"
#include "modio/detail/ops/http/PerformRequestAndGetResponseOp.h"
#include "modio/http/ModioHttpParams.h"

namespace Modio
{
	namespace Detail
	{
#include <asio/yield.hpp>
		class GetModTagsOp
		{
		public:
			template<typename CoroType>
			void operator()(CoroType& Self, Modio::ErrorCode ec = {})
			{
				reenter(CoroutineState)
				{
					yield Modio::Detail::PerformRequestAndGetResponseAsync(
						TagResponseBuffer,
						Modio::Detail::GetGameTagOptionsRequest.SetGameID(Modio::Detail::SDKSessionData::CurrentGameID())
							.AddQueryParamRaw(Modio::Detail::Constants::QueryParamStrings::ShowHiddenTags, "true"),
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
			asio::coroutine CoroutineState {};
			Modio::Detail::DynamicBuffer TagResponseBuffer {};
		};
#include <asio/unyield.hpp>

		template<typename GetTagsCompleteCallback>
		void GetModTagsAsync(GetTagsCompleteCallback&& OnGetTagsComplete)
		{
			return asio::async_compose<GetTagsCompleteCallback,
									   void(Modio::ErrorCode, Modio::Optional<Modio::ModTagOptions>)>(
				Modio::Detail::GetModTagsOp(), OnGetTagsComplete,
				Modio::Detail::Services::GetGlobalContext().get_executor());
		}
	} // namespace Detail
} // namespace Modio