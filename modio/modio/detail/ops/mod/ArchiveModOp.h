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

#include "modio/detail/ops/mod/GetModInfoOp.h"
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
		class ArchiveModOp
		{
			asio::coroutine CoroState;
			Modio::GameID GameID;
			Modio::ModID ModID;
			Modio::Detail::DynamicBuffer ResponseBodyBuffer;

		public:
			ArchiveModOp(Modio::GameID GameID, Modio::ModID ModID) : GameID(GameID), ModID(ModID) {}

			template<typename CoroType>
			void operator()(CoroType& Self, Modio::ErrorCode ec = {},
							Modio::Optional<Modio::ModInfo> CheckedModInfo = {})
			{
				reenter(CoroState)
				{
					// Get ModInfo to find ModStatus and ensure it has not already been archived
					yield asio::async_compose<CoroType, void(Modio::ErrorCode, Modio::Optional<Modio::ModInfo>)>(
						Modio::Detail::GetModInfoOp(GameID, Modio::Detail::SDKSessionData::CurrentAPIKey(), ModID),
						Self, Modio::Detail::Services::GetGlobalContext().get_executor());

					if (ec)
					{
						// Failure retrieving ModInfo
						Self.complete(ec);
						return;
					}
					else if (CheckedModInfo.has_value())
					{
						if (CheckedModInfo.value().ModStatus == Modio::ModServerSideStatus::Deleted)
						{
							// Mod has already been archived
							Self.complete(Modio::make_error_code(Modio::ApiError::RequestedModNotFound));
							return;
						}
						else
						{
							// Make archive request
							yield Modio::Detail::ComposedOps::PerformRequestAndGetResponseAsync(
								ResponseBodyBuffer, Modio::Detail::DeleteModRequest.SetGameID(GameID).SetModID(ModID),
								Modio::Detail::CachedResponse::Disallow, std::move(Self));

							if (ec)
							{
								// Failure
								Self.complete(ec);
								return;
							}
							else
							{
								// Success
								Self.complete({});
							}
						}
					}
					return;
				}
			}
		};
	} // namespace Detail
} // namespace Modio