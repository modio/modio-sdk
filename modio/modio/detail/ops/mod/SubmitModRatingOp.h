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
#include "modio/core/ModioCoreTypes.h"
#include "modio/detail/AsioWrapper.h"
#include "modio/detail/CoreOps.h"
#include "modio/detail/FmtWrapper.h"

#include <asio/yield.hpp>
namespace Modio
{
	namespace Detail
	{
		class SubmitModRatingOp
		{
		public:
			SubmitModRatingOp(Modio::ModID Mod, Modio::Rating Rating) : Mod(Mod)
			{
				RawRating = fmt::format("{}", static_cast<std::int8_t>(Rating));
			};

			template<typename CoroType>
			void operator()(CoroType& Self, Modio::ErrorCode ec = {})
			{
				reenter(CoroutineState)
				{
					yield Modio::Detail::ComposedOps::PerformRequestAndGetResponseAsync(
						ResponseBodyBuffer,
						Modio::Detail::AddModRatingRequest.SetGameID(Modio::Detail::SDKSessionData::CurrentGameID())
							.SetModID(Mod)
							.AppendPayloadValue(Modio::Detail::Constants::APIStrings::Rating, RawRating),
						Modio::Detail::CachedResponse::Disallow, std::move(Self));
					// Treat an API error indicating a no-op as a success
					if (ec && Modio::ErrorCodeMatches(ec, Modio::ErrorConditionTypes::ApiErrorRefSuccess))
					{
						Self.complete({});
					}
					else
					{
						Self.complete(ec);
					}
					return;
				}
			}

		private:
			Modio::ModID Mod;
			std::string RawRating;
			asio::coroutine CoroutineState;
			Modio::Detail::DynamicBuffer ResponseBodyBuffer;
		};
	} // namespace Detail
} // namespace Modio

#include <asio/unyield.hpp>