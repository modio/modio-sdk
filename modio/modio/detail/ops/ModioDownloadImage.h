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
#include "modio/core/ModioStdTypes.h"
#include "modio/detail/CoreOps.h"
#include "modio/detail/ModioObjectTrack.h"

#include <asio/yield.hpp>
namespace Modio
{
	namespace Detail
	{		
		/// @brief Generic operation which downloads a specific image type from the mod.io REST API. Checks if the image is cached first before downloading.
		/// @tparam ImageType Type of image we're requesting
		template<typename ImageType>
		class DownloadImageOp : public Modio::Detail::BaseOperation<DownloadImageOp<ImageType>>
		{
		public:
			DownloadImageOp(ImageType ImageTypeImp, Modio::StableStorage<Modio::filesystem::path> Result)
				: ImageTypeImp(ImageTypeImp),
				  Result(Result)
			{}

			template<typename CoroType>
			void operator()(CoroType& Self, Modio::ErrorCode ec = {})
			{
				reenter(CoroutineState)
				{
					// Scope FileService
					{
						Modio::Detail::FileService& FileService =
							Modio::Detail::Services::GetGlobalService<Modio::Detail::FileService>();

						// Calculate destination path
						if (!ImageTypeImp.MakeDestinationPath(OpState.DestinationPath))
						{
							Self.complete(Modio::make_error_code(HttpError::ResourceNotAvailable));
							return;
						}
						OpState.DestinationTempPath = OpState.DestinationPath;
						//OpState.DestinationTempPath += Modio::filesystem::path(".download");

						// If the Image is already downloaded, then don't fetch it again
						if (FileService.FileExists(OpState.DestinationPath))
						{
							// Set result and complete successfully
							*Result = std::move(OpState.DestinationPath);
							Self.complete({});
							return;
						}

						OpState.DownloadRequestParams =
							Modio::Detail::HttpRequestParams::FileDownload(ImageTypeImp.GetDownloadURL());

						if (!OpState.DownloadRequestParams.has_value())
						{
							// We tried to download something from outside the mod.io servers. Abort
							Self.complete(Modio::make_error_code(Modio::HttpError::DownloadNotPermitted));
							return;
						}

						// Create the folder where the image will reside so that we can ensure that we can write to the
						// file
						if (!FileService.CreateFolder(OpState.DestinationPath.parent_path()))
						{
							// Failed
							Self.complete(Modio::make_error_code(FilesystemError::UnableToCreateFolder));
							return;
						}

						// We had a image before we downloaded the new image, try to delete it before we download the
						// new image
						if (Modio::Optional<Modio::filesystem::path> OldImage = ImageTypeImp.GetCachePath())
						{
							if (!FileService.DeleteFile(*OldImage))
							{
								Self.complete(Modio::make_error_code(Modio::FilesystemError::FileLocked));
								return;
							}
						}
					}

					// Download the file
					yield Modio::Detail::ComposedOps::DownloadImageAsync(
						OpState.DownloadRequestParams.value(),
						OpState.DestinationTempPath, std::move(Self));

					if (ec)
					{
						// FAILED
						Self.complete(ec);
						return;
					}
					Modio::filesystem::rename(OpState.DestinationTempPath,
											  OpState.DestinationPath, ec);
					if (ec)
					{
						Self.complete(ec);
						return;
					}
					// Set result buffer
					*Result = std::move(OpState.DestinationPath);
					// SUCCESS
					Self.complete({});
					return;
				}
			}

		private:
			asio::coroutine CoroutineState;

			// Parameters
			ImageType ImageTypeImp;

			// Return
			Modio::StableStorage<Modio::filesystem::path> Result;

			// State that might get mutated during the coroutine
			struct
			{
				Modio::Detail::DynamicBuffer ResponseBodyBuffer;
				Modio::filesystem::path DestinationPath;
				Modio::filesystem::path DestinationTempPath;
				Modio::Optional<Modio::Detail::HttpRequestParams> DownloadRequestParams;
			} OpState;
		};

		template<typename ImageType, typename DownloadImageCallback>
		auto DownloadImageAsync(ImageType&& Wrapper, Modio::StableStorage<Modio::filesystem::path> OutAvatarPath,
								DownloadImageCallback&& OnDownloadComplete)
		{
			return asio::async_compose<DownloadImageCallback, void(Modio::ErrorCode)>(
				DownloadImageOp<ImageType>(Wrapper, OutAvatarPath), OnDownloadComplete,
				Modio::Detail::Services::GetGlobalContext().get_executor());
		}
	} // namespace Detail
} // namespace Modio

#include <asio/unyield.hpp>
