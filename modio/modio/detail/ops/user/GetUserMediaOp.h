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
#include "modio/core/ModioStdTypes.h"
#include "modio/detail/ops/ModioDownloadImage.h"
#include "modio/detail/CoreOps.h"
#include "modio/detail/ops/ModioAvatarImageType.h"
#include <asio/yield.hpp>

namespace Modio
{
	namespace Detail
	{
		class GetUserMediaOp
		{
		public:
			GetUserMediaOp(Modio::GameID GameID, Modio::ApiKey ApiKey, Modio::AvatarSize AvatarSize)
				: GameID(GameID),
				  ApiKey(ApiKey),
				  AvatarSize(AvatarSize)
			{
				OpState.DestinationPath = Modio::MakeStable<Modio::filesystem::path>();
			}

			template<typename CoroType>
			void operator()(CoroType& Self, Modio::ErrorCode ec = {})
			{
				reenter(CoroutineState)
				{
					OpState.Avatar = Modio::Detail::SDKSessionData::GetAuthenticatedUserAvatar();
					OpState.User = Modio::Detail::SDKSessionData::GetAuthenticatedUser();

					if (!OpState.Avatar || !OpState.User)
					{
						Self.complete(Modio::make_error_code(Modio::UserDataError::InvalidUser), {});
						return;
					}

					yield Modio::Detail::DownloadImageAsync(
						AvatarImageType(OpState.User->UserId, AvatarSize, *OpState.Avatar), OpState.DestinationPath,
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
			Modio::AvatarSize AvatarSize;

			// State that might get mutated during the corutine
			struct
			{
				Modio::Optional<Modio::Detail::Avatar> Avatar;
				Modio::Optional<Modio::User> User;
				Modio::StableStorage<Modio::filesystem::path> DestinationPath;
			} OpState;
		};
	} // namespace Detail
} // namespace Modio

#include <asio/unyield.hpp>
