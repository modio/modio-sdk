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
#include "modio/detail/ops/ModioCollectionLogoImageType.h"
#include "modio/detail/ops/http/PerformRequestAndGetResponseOp.h"
#include "modio/detail/serialization/ModioLogoSerialization.h"
#include "modio/detail/serialization/ModioModCollectionInfoSerialization.h"
#include <asio/coroutine.hpp>
#include <asio/yield.hpp>

namespace Modio
{
	namespace Detail
	{
		class GetModCollectionMediaLogoOp
		{
			ModioAsio::coroutine CoroState {};

			// Parameters
			Modio::GameID GameID {};
			Modio::ApiKey ApiKey {};
			Modio::ModCollectionID CollectionId {};
			Modio::LogoSize LogoSize {};

			// State needed to persist during coroutine
			struct
			{
				Modio::Detail::DynamicBuffer ResponseBodyBuffer;
				Modio::StableStorage<Modio::filesystem::path> DestinationPath;
				Modio::Detail::Logo Logo;
			} OpState;

			public:
			GetModCollectionMediaLogoOp(Modio::GameID GameID, Modio::ApiKey ApiKey, Modio::ModCollectionID CollectionId,
							  Modio::LogoSize LogoSize)
				: GameID(GameID),
				  ApiKey(ApiKey),
				  CollectionId(CollectionId),
				  LogoSize(LogoSize)
			{
				OpState.DestinationPath = Modio::MakeStable<Modio::filesystem::path>();
			}

			template<typename CoroType>
			void operator()(CoroType& Self, Modio::ErrorCode ec = {})
			{
				reenter(CoroState)
				{
					// Only check cache if no invalidation has occurred
					if (Modio::Detail::SDKSessionData::IsModCollectionCacheInvalid(CollectionId) == false)
					{
						Modio::Optional<Modio::ModCollectionInfo> CachedInfo =
							Services::GetGlobalService<CacheService>().FetchFromCache(CollectionId);

						if (CachedInfo.has_value() == true)
						{
							OpState.Logo = CachedInfo.value().Logo;
						}
					}

					if (OpState.Logo.Filename.empty())
					{
						// Fetch the details about the request from the server. Let's hope it's in the cache (would be
						// nice if we could extend the cache for this call)
						yield Modio::Detail::PerformRequestAndGetResponseAsync(
							OpState.ResponseBodyBuffer, Modio::Detail::GetModCollectionRequest.SetGameID(GameID).SetModCollectionID(CollectionId),
							Modio::Detail::CachedResponse::Allow, std::move(Self));

						if (ec)
						{
							// FAILED
							Self.complete(ec, {});
							return;
						}

						// Marshall the result of the request
						if (auto ParsedLogo = Modio::Detail::MarshalSubobjectResponse<Modio::Detail::Logo>(
								"logo", OpState.ResponseBodyBuffer))
						{
							OpState.Logo = ParsedLogo.value();
						}
						else
						{
							Self.complete(Modio::make_error_code(Modio::HttpError::InvalidResponse), {});
							return;
						}
					}

					yield Modio::Detail::DownloadImageAsync(
						CollectionLogoImageType(CollectionId, LogoSize, OpState.Logo),
															OpState.DestinationPath, std::move(Self));

					if (ec)
					{
						Self.complete(ec, {});
						return;
					}

					Self.complete({}, Modio::ToModioString(OpState.DestinationPath->u8string()));
				}
			}
		};

		template<typename WrappedCallback>
		auto GetModCollectionMediaLogoAsync(Modio::ModCollectionID CollectionId, Modio::LogoSize LogoSize, WrappedCallback&& Callback)
		{
			return ModioAsio::async_compose<WrappedCallback, void(Modio::ErrorCode, Modio::Optional<std::string>)>(
				Modio::Detail::GetModCollectionMediaLogoOp(Modio::Detail::SDKSessionData::CurrentGameID(),
														   Modio::Detail::SDKSessionData::CurrentAPIKey(), CollectionId, LogoSize),
				Callback,
				Modio::Detail::Services::GetGlobalContext().get_executor());
		}

	} // namespace Detail
} // namespace Modio
#include <asio/unyield.hpp>
