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
#include "modio/detail/ops/ModioAvatarImageType.h"
#include "modio/detail/ops/ModioDownloadImage.h"
#include "modio/detail/serialization/ModioAvatarSerialization.h"
#include <asio/yield.hpp>

namespace Modio
{
	namespace Detail
	{
		/// @docinternal
		/// @brief Async operation to retrieve a user media metadata
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
					OpState.User = Modio::Detail::SDKSessionData::GetAuthenticatedUser();

					if (!OpState.User)
					{
						Self.complete(Modio::make_error_code(Modio::UserDataError::InvalidUser), {});
						return;
					}

					yield Modio::Detail::DownloadImageAsync(
						AvatarImageType(OpState.User->UserId, AvatarSize, OpState.User->Avatar),
						OpState.DestinationPath, std::move(Self));

					if (ec)
					{
						Self.complete(ec, {});
						return;
					}

					Self.complete({}, std::move(Modio::ToModioString(OpState.DestinationPath->u8string())));
				}
			}

		private:
			ModioAsio::coroutine CoroutineState {};

			// Parameters
			Modio::GameID GameID {};
			Modio::ApiKey ApiKey {};
			Modio::AvatarSize AvatarSize {};

			// State that might get mutated during the corutine
			struct
			{
				Modio::Optional<Modio::User> User {};
				Modio::StableStorage<Modio::filesystem::path> DestinationPath {};
			} OpState;
		};

		template<typename GetUserMediaCompleteCallback>
		void GetUserMediaAsync(Modio::AvatarSize AvatarSize, GetUserMediaCompleteCallback&& OnGetUserMediaComplete)
		{
			return ModioAsio::async_compose<GetUserMediaCompleteCallback,
									   void(Modio::ErrorCode, Modio::Optional<std::string>)>(
				Modio::Detail::GetUserMediaOp(Modio::Detail::SDKSessionData::CurrentGameID(),
											  Modio::Detail::SDKSessionData::CurrentAPIKey(), AvatarSize),
				OnGetUserMediaComplete,
				Modio::Detail::Services::GetGlobalContext().get_executor());
		}
	} // namespace Detail
} // namespace Modio

#include <asio/unyield.hpp>
