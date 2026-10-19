/*
 *  Copyright (C) 2025 mod.io Pty Ltd. <https://mod.io>
 *
 *  This file is part of the mod.io SDK.
 *
 *  Distributed under the MIT License. (See accompanying file LICENSE or
 *   view online at <https://github.com/modio/modio-sdk/blob/main/LICENSE>)
 *
 */

#pragma once

#include "modio/core/ModioBuffer.h"
#include "modio/core/entities/ModioGameInfo.h"
#include "modio/core/entities/ModioModTagOptions.h"
#include "modio/detail/AsioWrapper.h"
#include "modio/detail/ModioConstants.h"
#include "modio/detail/ModioJsonHelpers.h"
#include "modio/detail/ModioSDKSessionData.h"
#include "modio/detail/ops/http/PerformRequestAndGetResponseOp.h"
#include "modio/detail/serialization/ModioGameInfoSerialization.h"
#include "modio/http/ModioHttpParams.h"

namespace Modio
{
	namespace Detail
	{
#include <asio/yield.hpp>
		class GetCollectionTagsOp
		{
		public:
			template<typename CoroType>
			void operator()(CoroType& Self, Modio::ErrorCode ec = {})
			{
				reenter(CoroutineState)
				{
					yield Modio::Detail::PerformRequestAndGetResponseAsync(
						ResponseBuffer,
						Modio::Detail::GetGameRequest.SetGameID(Modio::Detail::SDKSessionData::CurrentGameID())
							.AddQueryParamRaw(Modio::Detail::Constants::QueryParamStrings::ShowHiddenTags, "true"),
						Modio::Detail::CachedResponse::Allow, std::move(Self));
					if (ec)
					{
						Self.complete(ec, {});
						return;
					}
					{
						Modio::Optional<Modio::GameInfo> Game =
							TryMarshalResponse<Modio::GameInfo>(ResponseBuffer);
						if (Game.has_value())
						{
							Modio::ModTagOptions Options;
							Options.GetRawList() = Game->CollectionTagOptions;
							Self.complete({}, Modio::Optional<Modio::ModTagOptions>(std::move(Options)));
						}
						else
						{
							Self.complete(Modio::make_error_code(Modio::HttpError::InvalidResponse), {});
						}
					}
				}
			}

		private:
			ModioAsio::coroutine CoroutineState {};
			Modio::Detail::DynamicBuffer ResponseBuffer {};
		};
#include <asio/unyield.hpp>

		template<typename GetCollectionTagsCompleteCallback>
		void GetCollectionTagsAsync(GetCollectionTagsCompleteCallback&& OnGetCollectionTagsComplete)
		{
			return ModioAsio::async_compose<GetCollectionTagsCompleteCallback,
											void(Modio::ErrorCode, Modio::Optional<Modio::ModTagOptions>)>(
				Modio::Detail::GetCollectionTagsOp(), OnGetCollectionTagsComplete,
				Modio::Detail::Services::GetGlobalContext().get_executor());
		}
	} // namespace Detail
} // namespace Modio
