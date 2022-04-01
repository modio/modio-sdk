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
#include "modio/detail/ops/ModioGalleryImageType.h"
#include <asio/yield.hpp>

namespace Modio
{
	namespace Detail
	{
		class GetModMediaGalleryOp
		{
		public:
			GetModMediaGalleryOp(Modio::GameID GameID, Modio::ApiKey ApiKey, Modio::ModID ModId,
								 Modio::GallerySize GallerySize, Modio::GalleryIndex ImageIndex)
				: GameID(GameID),
				  ApiKey(ApiKey),
				  ModId(ModId),
				  GallerySize(GallerySize),
				  ImageIndex(ImageIndex)
			{
				OpState.DestinationPath = Modio::MakeStable<Modio::filesystem::path>();
			}

			template<typename CoroType>
			void operator()(CoroType& Self, Modio::ErrorCode ec = {})
			{
				reenter(CoroutineState)
				{
					if (ImageIndex < 0)
					{
						Self.complete(Modio::make_error_code(Modio::GenericError::IndexOutOfRange), {});
						return;
					}

					{
						Modio::Optional<Modio::ModInfo> CachedModInfo =
							Services::GetGlobalService<CacheService>().FetchFromCache(ModId);

						if (CachedModInfo.has_value() == true)
						{
							OpState.GalleryList = CachedModInfo.value().GalleryImages;
						}
					}

					if (OpState.GalleryList.Size() == 0)
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

						{
							nlohmann::json ResponseJson = ToJson(OpState.ResponseBodyBuffer);
							if (ResponseJson.is_discarded())
							{
								Self.complete(Modio::make_error_code(Modio::HttpError::InvalidResponse), {});
								return;
							}
							if (ResponseJson.contains("media"))
							{
								Modio::Detail::ParseSubobjectSafe(ResponseJson, OpState.GalleryList, "media", "images");
							}
							else
							{
								Self.complete(Modio::make_error_code(Modio::HttpError::InvalidResponse), {});
								return;
							}
						}
					}

					if (ImageIndex >= OpState.GalleryList.Size())
					{
						// FAILED
						Self.complete(Modio::make_error_code(Modio::GenericError::IndexOutOfRange), {});
						return;
					}

					yield Modio::Detail::DownloadImageAsync(
						GalleryImageType(ModId, OpState.GalleryList, GallerySize, ImageIndex), OpState.DestinationPath,
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
			Modio::GallerySize GallerySize;
			Modio::GalleryIndex ImageIndex;

			// State that might get mutated during the coroutine
			struct
			{
				Modio::Detail::DynamicBuffer ResponseBodyBuffer;
				Modio::StableStorage<Modio::filesystem::path> DestinationPath;
				Modio::GalleryList GalleryList;
			} OpState;
		};
	} // namespace Detail
} // namespace Modio

#include <asio/unyield.hpp>
