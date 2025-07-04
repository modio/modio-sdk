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
#include "modio/detail/ops/http/PerformRequestAndGetResponseOp.h"
#include "modio/http/ModioHttpParams.h"

#include <asio/yield.hpp>

namespace Modio
{
	namespace Detail
	{
		class AddOrUpdateModGalleryImagesOp
		{
			asio::coroutine CoroState;
			Modio::ModID ModID;
			Modio::Detail::DynamicBuffer ResponseBodyBuffer;
			Modio::Detail::HttpRequestParams AddOrUpdateRequest;
			std::vector<std::string> ImagePaths;

		public:
			AddOrUpdateModGalleryImagesOp(Modio::GameID GameID, Modio::ModID ModID, std::vector<std::string> InImagePaths,
										  bool SyncGallery)
				: ModID(ModID),
				  ImagePaths(std::move(InImagePaths))
			{
				AddOrUpdateRequest = Modio::Detail::AddModMediaRequest.SetGameID(GameID).SetModID(ModID);

				// Add sync parameter if needed
				if (SyncGallery)
				{
					AddOrUpdateRequest = AddOrUpdateRequest.AppendPayloadValue("sync", "true");
				}

				// Append all images with proper field names
				for (size_t i = 0; i < ImagePaths.size(); ++i)
				{
					const std::string& ImagePath = ImagePaths[i];
					std::string fieldName = "image" + std::to_string(i + 1);
					AddOrUpdateRequest = AddOrUpdateRequest.AppendPayloadFile(fieldName, ImagePath);
				}
			}

			template<typename CoroType>
			void operator()(CoroType& Self, Modio::ErrorCode ec = {})
			{
				reenter(CoroState)
				{
					yield Modio::Detail::PerformRequestAndGetResponseAsync(ResponseBodyBuffer, AddOrUpdateRequest,
																		   Modio::Detail::CachedResponse::Disallow,
																		   std::move(Self));

					if (Modio::ErrorCodeMatches(ec, Modio::ErrorConditionTypes::UserNotAuthenticatedError))
					{
						Modio::Detail::SDKSessionData::InvalidateOAuthToken();
					}

					if (ec)
					{
						Self.complete(ec);
						return;
					}

					// After a gallery update, invalidate the mod cache to ensure fresh data on next fetch
					Modio::Detail::SDKSessionData::InvalidateModCache(ModID);

					// Also, remove the gallery files stored for that mod
					for (size_t i = 0; i < ImagePaths.size(); ++i)
					{
						RemoveFileIfExists(Modio::GallerySize::Original, i);
						RemoveFileIfExists(Modio::GallerySize::Thumb320, i);
						RemoveFileIfExists(Modio::GallerySize::Thumb1280, i);
					}

					Self.complete({});
				}
			}

			void RemoveFileIfExists(Modio::GallerySize Size, Modio::GalleryIndex Index)
			{
				Modio::Detail::FileService& FileService =
					Modio::Detail::Services::GetGlobalService<Modio::Detail::FileService>();
				Modio::Optional<Modio::filesystem::path> Path = FileService.GetGalleryImage(ModID, Size, Index);
				if (Path.has_value() == true)
				{
					FileService.DeleteFile(Path.value());
				}
			}
		};
	} // namespace Detail
} // namespace Modio
#include <asio/unyield.hpp>
