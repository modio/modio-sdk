/*
 *  Copyright (C) 2026 mod.io Pty Ltd. <https://mod.io>
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
#include "modio/core/ModioErrorCode.h"
#include "modio/core/ModioFilterParams.h"
#include "modio/core/entities/ModioGameInfoList.h"
#include "modio/core/entities/ModioUserRoleInfoList.h"
#include "modio/detail/ModioSDKSessionData.h"
#include "modio/detail/ModioStringHelpers.h"

#include <asio/yield.hpp>
namespace Modio
{
	namespace Detail
	{
		/// @docinternal
		/// @brief Async operation to retrieve a user role for a particular resource (e.g. Game)
		/// @note Currently this is emulating the behaviour of the /me/roles endpoint,
		///	by hitting the /me/games endpoint, and looking for an associated game ID in the returned list.
		class GetUserRolesOp
		{
			Modio::Detail::DynamicBuffer ResponseBodyBuffer {};
		public:

			GetUserRolesOp(Modio::GameID GameID)
				: GameID(GameID)
			{
			}

			template<typename CoroType>
			void operator()(CoroType& Self, Modio::ErrorCode ec = {})
			{
				reenter(CoroutineState)
				{
					yield Modio::Detail::PerformRequestAndGetResponseAsync(
						ResponseBodyBuffer,
						Modio::Detail::GetUserGamesRequest,
						Modio::Detail::CachedResponse::Allow, std::move(Self));

					if (!ec)
					{
						// Got the response OK, try to marshal to the expected type
						Modio::Optional<Modio::GameInfoList> List = 
							TryMarshalResponse<Modio::GameInfoList>(ResponseBodyBuffer);

						// Marshalled OK
						if (List.has_value())
						{
							for (Modio::GameInfo Game : *List)
							{
								if (Game.GameID == GameID)
								{
									Modio::UserRoleInfoList UserRoles;
									UserRoles.Append(Modio::UserRoleInfo(GameID, Modio::UserRoleInfo::UserRoleType::Member));
									Modio::Optional<Modio::UserRoleInfoList> Optional = std::move(UserRoles);
									Self.complete(ec, std::move(Optional));
									return;
								}
							}
							Self.complete(ec, {});
						}
						else
						{
							// Marshalling failed
							Self.complete(Modio::make_error_code(Modio::HttpError::InvalidResponse), {});
						}
					}
					else
					{
						Self.complete(ec, {});
					}
				}
			}

		private:
			ModioAsio::coroutine CoroutineState {};

			// Parameters
			Modio::GameID GameID {};
		};

		template<typename GetUserRolesCompleteCallback>
		void GetUserRolesAsync(Modio::UserRoleResourceType Resource,
							   GetUserRolesCompleteCallback&& OnGetUserRolesComplete)
		{
			(void) Resource;
			return ModioAsio::async_compose<GetUserRolesCompleteCallback,
											void(Modio::ErrorCode, Modio::Optional<Modio::UserRoleInfoList>)>(
				Modio::Detail::GetUserRolesOp(Modio::Detail::SDKSessionData::CurrentGameID()), OnGetUserRolesComplete,
				Modio::Detail::Services::GetGlobalContext().get_executor());
		}
	} // namespace Detail
} // namespace Modio

#include <asio/unyield.hpp>
