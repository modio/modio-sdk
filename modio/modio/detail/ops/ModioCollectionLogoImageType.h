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
#include "modio/detail/serialization/ModioLogoSerialization.h"
#include "modio/file/ModioFileService.h"

namespace Modio
{
	namespace Detail
	{
		/// @docinternal
		/// @brief Container for metadata related to logo images
		struct CollectionLogoImageType
		{
			CollectionLogoImageType(Modio::ModCollectionID CollectionId, Modio::LogoSize LogoSize,
									const Modio::Detail::Logo& Logo)
				: CollectionId(CollectionId),
				  LogoSize(LogoSize),
				  LogoURL(GetLogoURL(Logo, LogoSize))
			{}

			bool MakeDestinationPath(Modio::filesystem::path& OutPath) const
			{
				Modio::Detail::FileService& FileService =
					Modio::Detail::Services::GetGlobalService<Modio::Detail::FileService>();
				Modio::Optional<std::string> FileName = Modio::Detail::String::GetFilenameFromURL(LogoURL);
				if (FileName)
				{
					OutPath = FileService.MakeModCollectionMediaFilePath(CollectionId, LogoSize, *FileName);
					return true;
				}
				return false;
			}

			Modio::Optional<Modio::filesystem::path> GetCachePath() const
			{
				Modio::Detail::FileService& FileService =
					Modio::Detail::Services::GetGlobalService<Modio::Detail::FileService>();
				return FileService.GetLogo(CollectionId, LogoSize);
			}

			const std::string& GetDownloadURL() const
			{
				return LogoURL;
			}

			Modio::ModCollectionID CollectionId {};
			Modio::LogoSize LogoSize {};
			std::string LogoURL {};
		};

		// template<typename CallbackType>
		// using DownloadLogoAsync = DownloadLogoAsync<LogoImageType, CallbackType>;
	} // namespace Detail
} // namespace Modio
