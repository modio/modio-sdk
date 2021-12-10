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
#include "modio/core/ModioCoreTypes.h"
#include "modio/detail/AsioWrapper.h"
#include "modio/detail/CoreOps.h"
#include "modio/http/ModioHttpParams.h"

#include <asio/yield.hpp>

namespace Modio
{
	namespace Detail
	{
		class AddOrUpdateModLogoOp
		{
			asio::coroutine CoroState;
			Modio::GameID GameID;
			Modio::ModID ModID;
			Modio::filesystem::path LogoPath;
			Modio::Detail::DynamicBuffer ResponseBodyBuffer;

		public:
			AddOrUpdateModLogoOp(Modio::GameID GameID, Modio::ModID ModID, Modio::filesystem::path LogoPath)
				: GameID(GameID),
				  ModID(ModID),
				  LogoPath(LogoPath)
			{}

			template<typename CoroType>
			void operator()(CoroType& Self, Modio::ErrorCode ec = {})
			{
				reenter(CoroState)
				{
					yield Modio::Detail::ComposedOps::PerformRequestAndGetResponseAsync(
						ResponseBodyBuffer, Modio::Detail::AddModMediaRequest.SetGameID(GameID).SetModID(ModID).AppendPayloadFile("logo", LogoPath),
						Modio::Detail::CachedResponse::Disallow, std::move(Self));

					// error checks
					if (ec)
					{
						//	Failure
						Self.complete(ec);
						return;
					}
					Self.complete({});
				}
			}
		};
	} // namespace Detail
} // namespace Modio