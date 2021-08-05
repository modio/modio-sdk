#pragma once

#include "modio/core/ModioCoreTypes.h"
#include "modio/detail/entities/ModioLogo.h"
#include "modio/file/ModioFileService.h"

namespace Modio
{
	namespace Detail
	{
		struct LogoImageType
		{
			LogoImageType(Modio::ModID ModID, Modio::LogoSize LogoSize, const Modio::Detail::Logo& Logo)
				: ModID(ModID),
				  LogoSize(LogoSize),
				  LogoURL(GetLogoURL(Logo, LogoSize))
			{}

			bool MakeDestinationPath(Modio::filesystem::path& OutPath) const
			{
				Modio::Detail::FileService& FileService = Modio::Detail::Services::GetGlobalService<Modio::Detail::FileService>();
				Modio::Optional<std::string> FileName = Modio::Detail::String::GetFilenameFromURL(LogoURL);
				if (FileName)
				{
					OutPath = FileService.MakeModMediaFilePath(ModID, LogoSize, *FileName);
					return true;
				}
				return false;
			}

			Modio::Optional<Modio::filesystem::path> GetCachePath() const
			{
				Modio::Detail::FileService& FileService = Modio::Detail::Services::GetGlobalService<Modio::Detail::FileService>();
				return FileService.GetLogo(ModID, LogoSize);
			}

			const std::string& GetDownloadURL() const
			{
				return LogoURL;
			}

			Modio::ModID ModID;
			Modio::LogoSize LogoSize;
			std::string LogoURL;
		};

		// template<typename CallbackType>
		// using DownloadLogoAsync = DownloadLogoAsync<LogoImageType, CallbackType>;
	}
}

