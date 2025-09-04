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

#include "modio/core/ModioCoreTypes.h"
#include "modio/detail/ops/ModioDownloadImage.h"
#include "modio/detail/ops/ModioLogoImageType.h"
#include "modio/detail/ops/http/PerformRequestAndGetResponseOp.h"
#include "modio/detail/serialization/ModioUserSerialization.h"
#include "modio/detail/serialization/ModioModCollectionInfoSerialization.h"
#include "modio/detail/ops/ModioAvatarImageType.h"
#include <asio/coroutine.hpp>
#include <asio/yield.hpp>

namespace Modio
{
	namespace Detail
	{
		class GetModCollectionMediaAvatarOp
		{
			asio::coroutine CoroState {};

			// Parameters
			Modio::GameID GameID {};
			Modio::ApiKey ApiKey {};
			Modio::ModCollectionID CollectionId {};
			Modio::AvatarSize AvatarSize {};

			// State that might get mutated during the coroutine
			struct
			{
				Modio::Detail::DynamicBuffer ResponseBodyBuffer {};
				Modio::StableStorage<Modio::filesystem::path> DestinationPath {};
				Modio::User User {};
			} OpState;

		public:
			GetModCollectionMediaAvatarOp(Modio::GameID GameID, Modio::ApiKey ApiKey,
										  Modio::ModCollectionID CollectionId, Modio::AvatarSize AvatarSize)
				: GameID(GameID),
				  ApiKey(ApiKey),
				  CollectionId(CollectionId),
				  AvatarSize(AvatarSize)
			{
				OpState.DestinationPath = Modio::MakeStable<Modio::filesystem::path>();
			}

			template<typename CoroType>
			void operator()(CoroType& Self, Modio::ErrorCode ec = {})
			{
				reenter(CoroState)
				{
					{
						Modio::Optional<Modio::ModCollectionInfo> CachedResponse =
							Services::GetGlobalService<CacheService>().FetchFromCache(CollectionId);
						if (CachedResponse.has_value())
						{
							Modio::ModCollectionInfo CachedInfo = CachedResponse.value();
							OpState.User = CachedInfo.ProfileSubmittedBy;
						}
					}

					if (OpState.User.Avatar.Filename.empty() && OpState.User.UserId == 0 &&
						OpState.User.Username.empty())
					{
						// Fetch the details about the request from the server. Let's hope it's in the cache (would
						// be nice if we could extend the cache for this call)
						yield Modio::Detail::PerformRequestAndGetResponseAsync(
							OpState.ResponseBodyBuffer,
							Modio::Detail::GetModCollectionRequest.SetGameID(GameID).SetModCollectionID(CollectionId),
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

					Self.complete({}, Modio::ToModioString(std::move(OpState.DestinationPath->u8string())));
				}
			}
		};
		
		template<typename WrappedCallback>
		auto GetModCollectionMediaAvatarAsync(Modio::ModCollectionID CollectionId, Modio::AvatarSize AvatarSize,
											  WrappedCallback&& Callback)
		{
			return asio::async_compose<WrappedCallback, void(Modio::ErrorCode, Modio::Optional<std::string>)>(
				Modio::Detail::GetModCollectionMediaAvatarOp(Modio::Detail::SDKSessionData::CurrentGameID(),
															 Modio::Detail::SDKSessionData::CurrentAPIKey(),
															 CollectionId, AvatarSize),
				Callback, Modio::Detail::Services::GetGlobalContext().get_executor());
		}
	} // namespace Detail
} // namespace Modio

#include <asio/unyield.hpp>
