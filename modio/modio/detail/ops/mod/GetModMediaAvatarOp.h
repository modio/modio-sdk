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
#include "modio/core/entities/ModioUser.h"
#include "modio/detail/ops/ModioAvatarImageType.h"
#include "modio/detail/ops/ModioDownloadImage.h"
#include <asio/yield.hpp>

namespace Modio
{
	namespace Detail
	{
		class GetModMediaAvatarOp
		{
		public:
			GetModMediaAvatarOp(Modio::GameID GameID, Modio::ApiKey ApiKey, Modio::ModID ModId,
								Modio::AvatarSize AvatarSize)
				: GameID(GameID),
				  ApiKey(ApiKey),
				  ModId(ModId),
				  AvatarSize(AvatarSize)
			{
				OpState.DestinationPath = Modio::MakeStable<Modio::filesystem::path>();
			}

			template<typename CoroType>
			void operator()(CoroType& Self, Modio::ErrorCode ec = {})
			{
				reenter(CoroutineState)
				{
					{
						Modio::Optional<Modio::ModInfo> CachedResponse =
							Services::GetGlobalService<CacheService>().FetchFromCache(ModId);
						if (CachedResponse.has_value())
						{
							Modio::ModInfo CachedInfo = CachedResponse.value();
							OpState.User = CachedInfo.ProfileSubmittedBy;
						}
					}

					if (OpState.User.Avatar.Filename == "" && OpState.User.UserId == 0 && OpState.User.Username == "")
					{
						// Fetch the details about the request from the server. Let's hope it's in the cache (would be
						// nice if we could extend the cache for this call)
						yield Modio::Detail::ComposedOps::PerformRequestAndGetResponseAsync(
							OpState.ResponseBodyBuffer, Modio::Detail::GetModRequest.SetGameID(GameID).SetModID(ModId),
							Modio::Detail::CachedResponse::Allow, std::move(Self));

						if (ec)
						{
							// FAILED
							Self.complete(ec, {});
							return;
						}

						// Marshall the result of the request
						if (auto ParsedUser = Modio::Detail::MarshalSubobjectResponse<Modio::User>(
								"submitted_by", OpState.ResponseBodyBuffer))
						{
							OpState.User = ParsedUser.value();
						}
						else
						{
							Self.complete(Modio::make_error_code(Modio::HttpError::InvalidResponse), {});
							return;
						}
					}

					yield Modio::Detail::DownloadImageAsync(
						AvatarImageType(OpState.User.UserId, AvatarSize, OpState.User.Avatar), OpState.DestinationPath,
						std::move(Self));

					if (ec)
					{
						Self.complete(ec, {});
						return;
					}

					Self.complete({}, std::move(*OpState.DestinationPath));
				}
			}

		private:
			asio::coroutine CoroutineState;

			// Parameters
			Modio::GameID GameID;
			Modio::ApiKey ApiKey;
			Modio::ModID ModId;
			Modio::AvatarSize AvatarSize;

			// State that might get mutated during the coroutine
			struct
			{
				Modio::Detail::DynamicBuffer ResponseBodyBuffer;
				Modio::StableStorage<Modio::filesystem::path> DestinationPath;
				Modio::User User;
			} OpState;
		};
	} // namespace Detail
} // namespace Modio

#include <asio/unyield.hpp>
