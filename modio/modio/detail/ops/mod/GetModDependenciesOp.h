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
#include "modio/detail/AsioWrapper.h"
#include "modio/detail/ModioJsonHelpers.h"
#include "modio/detail/serialization/ModioModDependencySerialization.h"
#include "modio/detail/ops/http/PerformRequestAndGetResponseOp.h"
#include "modio/http/ModioHttpParams.h"

#include "asio/yield.hpp"
namespace Modio
{
	namespace Detail
	{
		class GetModDependenciesOp
		{
		public:
			GetModDependenciesOp(Modio::ModID ModID, Modio::GameID GameID, bool Recursive)
				: ModID(ModID),
				  GameID(GameID),
				  Recursive(Recursive) {};
			template<typename CoroType>
			void operator()(CoroType& Self, Modio::ErrorCode ec = {})
			{
				reenter(CoroutineState)
				{
					yield Modio::Detail::PerformRequestAndGetResponseAsync(
						ResponseBodyBuffer,
						Modio::Detail::GetModDependenciesRequest.SetGameID(GameID).SetModID(ModID).AddQueryParamRaw(
							Modio::Detail::Constants::QueryParamStrings::Recursive,
							(Recursive ? "true" : "false")),
						Modio::Detail::CachedResponse::Allow, std::move(Self));
					if (ec)
					{
						Self.complete(ec, {});
						return;
					}
					else
					{
						Modio::Optional<Modio::ModDependencyList> List =
							TryMarshalResponse<Modio::ModDependencyList>(ResponseBodyBuffer);
						if (List.has_value())
						{
							Self.complete({}, List);
						}
						else
						{
							Self.complete(Modio::make_error_code(Modio::HttpError::InvalidResponse), {});
						}
					}
				}
			}

		private:
			Modio::Detail::DynamicBuffer ResponseBodyBuffer;
			asio::coroutine CoroutineState;
			Modio::ModID ModID;
			Modio::GameID GameID;
			bool Recursive;
		};
	} // namespace Detail
} // namespace Modio
#include "asio/unyield.hpp"