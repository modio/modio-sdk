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

namespace Modio
{
	namespace Detail
	{
		class IFileServiceImplementation
		{
			public:
			virtual ~IFileServiceImplementation() {};
			virtual const Modio::filesystem::path LocalMetadataFolder() const = 0;
			virtual Modio::filesystem::path MakeTempFilePath(std::string Filename) const = 0;
			virtual Modio::filesystem::path MakeModPath(Modio::ModID ID) const = 0;
			virtual Modio::filesystem::path MakeModMediaFilePath(Modio::ModID ID, Modio::LogoSize Size,
																 const std::string& OriginalFilename) const = 0;
			virtual Modio::filesystem::path MakeModMediaFilePath(Modio::ModID ModID, Modio::GallerySize Size,
																 Modio::GalleryIndex ImageIndex,
																 const std::string& OriginalFileName) const = 0;
			virtual Modio::filesystem::path MakeLogoFolderPath(Modio::ModID ID) const = 0;
			virtual Modio::filesystem::path MakeGalleryFolderPath(Modio::ModID ID, Modio::GalleryIndex ImageIndex) const = 0;
			virtual Modio::filesystem::path MakeUserMediaFilePath(Modio::UserID ID, Modio::AvatarSize Size,
																  const std::string& OriginalFilename) const = 0;
			virtual Modio::filesystem::path MakeAvatarFolderPath(Modio::UserID ID) const = 0;

			virtual bool CheckSpaceAvailable(const Modio::filesystem::path& Destination, Modio::FileSize DesiredSize) = 0;
			virtual bool DirectoryExists(const Modio::filesystem::path& PathToCheck) const = 0;
			virtual bool CreateFolder(const Modio::filesystem::path& FolderPath) const = 0;
			virtual bool FileExists(const Modio::filesystem::path& FilePath) const = 0;
			virtual bool DeleteFile(const Modio::filesystem::path& FilePath) const = 0;
			virtual const Modio::filesystem::path& UserDataFolder() const = 0;
			virtual const Modio::filesystem::path& GetRootLocalStoragePath() const = 0;
			virtual Modio::ErrorCode ApplyGlobalConfigOverrides(const class std::map<std::string, std::string> Overrides) = 0;
			virtual void Shutdown() = 0;

		};
	}
}