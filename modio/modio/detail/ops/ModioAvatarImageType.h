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
#include "modio/detail/entities/ModioAvatar.h"
#include "modio/file/ModioFileService.h"

namespace Modio
{
	namespace Detail
	{
		struct AvatarImageType
		{
			AvatarImageType(Modio::UserID UserId, Modio::AvatarSize AvatarSize, const Modio::Detail::Avatar& Avatar)
				: UserId(UserId),
				  AvatarSize(AvatarSize),
				  AvatarURL(Modio::Detail::GetAvatarURL(Avatar, AvatarSize))
			{}

			bool MakeDestinationPath(Modio::filesystem::path& OutPath) const
			{
				Modio::Detail::FileService& FileService = Modio::Detail::Services::GetGlobalService<Modio::Detail::FileService>();
				Modio::Optional<std::string> FileName = Modio::Detail::String::GetFilenameFromURL(AvatarURL);
				if (FileName)
				{
					OutPath = FileService.MakeModMediaFilePath(UserId, AvatarSize, *FileName);
					return true;
				}
				return false;
			}

			Modio::Optional<Modio::filesystem::path> GetCachePath() const
			{
				Modio::Detail::FileService& FileService = Modio::Detail::Services::GetGlobalService<Modio::Detail::FileService>();
				return FileService.GetAvatar(UserId, AvatarSize);
			}

			const std::string& GetDownloadURL() const
			{
				return AvatarURL;
			}

			Modio::UserID UserId;
			Modio::AvatarSize AvatarSize;
			std::string AvatarURL;
		};

		//template<typename CallbackType>
		//using DownloadAvatarAsync = DownloadImageAsync<AvatarImageType, CallbackType>;
	} // namespace Detail
} // namespace Modio

