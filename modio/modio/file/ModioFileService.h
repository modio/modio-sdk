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

#include "file/FileSystemImplementation.h"
#include "modio/core/ModioBuffer.h"
#include "modio/core/ModioLogEnum.h"
#include "modio/core/ModioLogger.h"
#include "modio/detail/AsioWrapper.h"
#include "modio/detail/FmtWrapper.h"
#include "modio/detail/ModioProfiling.h"
#include "modio/detail/ModioStringHelpers.h"
#include "modio/detail/entities/ModioAvatar.h"
#include "modio/detail/entities/ModioImage.h"
#include "modio/detail/entities/ModioLogo.h"
#include <iostream>
#include <memory>
#include <queue>

namespace Modio
{
	namespace Detail
	{
		class FileService : public asio::detail::service_base<FileService>
		{
		public:
			explicit FileService(asio::io_context& IOService) : asio::detail::service_base<FileService>(IOService)
			{
				auto NewImplementation = std::make_shared<FileSystemImplementation>(*this);
				PlatformImplementation.swap(NewImplementation);
			}

			using implementation_type = FileSystemImplementation::IOObjectImplementationType;

			void construct(implementation_type& Implementation)
			{
				PlatformImplementation->InitializeIOObjectImplementation(Implementation);
			}

			void destroy(implementation_type& Implementation)
			{
				PlatformImplementation->DestroyIOObjectImplementation(Implementation);
			}

			void move_construct(implementation_type& Implementation, implementation_type& Other)
			{
				PlatformImplementation->MoveIOObjectImplementation(Implementation, Other);
			}

			void move_assign(implementation_type& Implementation,
							 Modio::Detail::FileService& MODIO_UNUSED_ARGUMENT(OtherService),
							 implementation_type& Other)
			{
				// No difference between this and move_construct for us because our application will only have a single
				// io_context and a single HttpService instance
				PlatformImplementation->MoveIOObjectImplementation(Implementation, Other);
			}

			void converting_move_construct(implementation_type& impl, Modio::Detail::FileService&,
										   implementation_type& other_impl)
			{
				move_construct(impl, other_impl);
			}

			void converting_move_assign(implementation_type& impl, Modio::Detail::FileService& other_service,
										implementation_type& other_impl)
			{
				move_assign(impl, other_service, other_impl);
			}

			void Shutdown()
			{
				PlatformImplementation->Shutdown();
			}

			template<typename CompletionHandlerType>
			auto InitializeAsync(Modio::InitializeOptions InitParams, CompletionHandlerType&& Handler)
			{
				return PlatformImplementation->InitializeAsync(InitParams,
															   std::forward<CompletionHandlerType>(Handler));
			}

			template<typename CompletionTokenType>
			auto WriteSomeAtAsync(implementation_type& PlatformIOObject, std::uintmax_t Offset,
								  Modio::Detail::Buffer Buffer, CompletionTokenType&& Token)
			{
				return PlatformImplementation->WriteSomeAtAsync(PlatformIOObject, Offset, std::move(Buffer),
																std::forward<CompletionTokenType>(Token));
			}

			template<typename CompletionTokenType>
			auto ReadSomeAtAsync(implementation_type& PlatformIOObject, std::uintmax_t Offset, std::uintmax_t Length,
								 CompletionTokenType&& Token)
			{
				return PlatformImplementation->ReadSomeAtAsync(PlatformIOObject, Offset, Length,
															   std::forward<CompletionTokenType>(Token));
			}

			template<typename CompletionTokenType>
			auto ReadSomeAtAsync(implementation_type& PlatformIOObject, std::uintmax_t Offset,
								 std::uintmax_t MaxBytesToRead, Modio::Detail::DynamicBuffer Destination,
								 CompletionTokenType&& Token)
			{
				return PlatformImplementation->ReadSomeAtAsync(PlatformIOObject, Offset, MaxBytesToRead, Destination,
															   std::forward<CompletionTokenType>(Token));
			}

			template<typename CompletionTokenType>
			auto ReadAsync(implementation_type& PlatformIOObject, std::uintmax_t MaxBytesToRead,
						   Modio::Detail::DynamicBuffer Destination, CompletionTokenType&& Token)
			{
				return PlatformImplementation->ReadAsync(PlatformIOObject, MaxBytesToRead, Destination,
														 std::forward<CompletionTokenType>(Token));
			}

			template<typename CompletionTokenType>
			auto WriteAsync(implementation_type& PlatformIOObject, Modio::Detail::Buffer Buffer,
							CompletionTokenType&& Token)
			{
				return PlatformImplementation->WriteAsync(PlatformIOObject, std::move(Buffer),
														  std::forward<CompletionTokenType>(Token));
			}

			template<typename CompletionTokenType>
			auto DeleteFolderAsync(Modio::filesystem::path FolderPath, CompletionTokenType&& Token)
			{
				return PlatformImplementation->DeleteFolderAsync(FolderPath, std::forward<CompletionTokenType>(Token));
			}

			Modio::ErrorCode ApplyGlobalConfigOverrides(const std::map<std::string, std::string> Overrides)
			{
				return PlatformImplementation->ApplyGlobalConfigOverrides(Overrides);
			}

			Modio::filesystem::path UserDataFolder() const
			{
				return PlatformImplementation->UserDataFolder();
			}

			Modio::filesystem::path LocalMetadataFolder() const
			{
				return PlatformImplementation->LocalMetadataFolder();
			}

			Modio::Optional<Modio::filesystem::path> MakeTempFilePath(std::string FileName) const
			{
				return PlatformImplementation->MakeTempFilePath(FileName);
			}

			Modio::filesystem::path MakeTempModPath(Modio::ModID ModID) const
			{
				return PlatformImplementation->MakeTempModPath(ModID);
			}

			Modio::filesystem::path MakeModPath(Modio::ModID ModID) const
			{
				return PlatformImplementation->MakeModPath(ModID);
			}

			Modio::filesystem::path MakeMediaCachePath() const
			{
				return PlatformImplementation->MakeMediaCachePath();
			}

			/// @brief Calculates the path of a mod's logo
			/// @param ModID The mod ID for the mod
			/// @param Size The image size
			/// @param OriginalFileName The original filename
			/// @return The fully-qualified path to the logo file
			Modio::filesystem::path MakeModMediaFilePath(Modio::ModID ModID, Modio::LogoSize Size,
														 const std::string& OriginalFileName) const
			{
				return PlatformImplementation->MakeModMediaFilePath(ModID, Size, OriginalFileName);
			}

			/// @brief Calculates the path of a mod's gallery image
			/// @param ModID The mod ID for the mod
			/// @param Size The image size
			/// @param ImageIndex The index of the image to retrieve
			/// @param OriginalFileName The original filename
			/// @return The fully-qualified path to the gallery image file
			Modio::filesystem::path MakeModMediaFilePath(Modio::ModID ModID, Modio::GallerySize Size,
														 Modio::GalleryIndex ImageIndex,
														 const std::string& OriginalFileName) const
			{
				return PlatformImplementation->MakeModMediaFilePath(ModID, Size, ImageIndex, OriginalFileName);
			}

			/// @brief Calculates the path where cached logos will be stored for a mod
			/// @param ModID The mod ID for the mod
			/// @return The fully-qualified path to the logo cache directory
			Modio::filesystem::path MakeLogoFolderPath(Modio::ModID ModID) const
			{
				return PlatformImplementation->MakeLogoFolderPath(ModID);
			}

			/// @brief Calculates the path where cached gallery images will be stored for a mod
			/// @param ModID The mod ID for the mod
			/// @param ImageIndex The index of the cached gallery image
			/// @return The fully-qualified path to the cache directory for the specified gallery index for the mod
			Modio::filesystem::path MakeGalleryFolderPath(Modio::ModID ModID, Modio::GalleryIndex ImageIndex) const
			{
				return PlatformImplementation->MakeGalleryFolderPath(ModID, ImageIndex);
			}

			/// @brief Calculates the path where a creator's avatar will be cached
			/// @param UserId The User ID of the creator
			/// @param AvatarSize The image size
			/// @param OriginalFileName The filename of the image
			/// @return The fully-qualified path of the creator avatar file
			inline Modio::filesystem::path MakeModMediaFilePath(Modio::UserID UserId, Modio::AvatarSize AvatarSize,
																const std::string& OriginalFileName) const
			{
				return MakeUserMediaFilePath(UserId, AvatarSize, OriginalFileName);
			}

			/// @brief Calculates the path where a creator's avatar will be cached
			/// @param UserId The User ID of the creator
			/// @param AvatarSize The image size
			/// @param OriginalFileName The filename of the image
			/// @return The fully-qualified path of the creator avatar file
			Modio::filesystem::path MakeUserMediaFilePath(Modio::UserID UserId, Modio::AvatarSize AvatarSize,
														  const std::string& OriginalFileName) const
			{
				return PlatformImplementation->MakeUserMediaFilePath(UserId, AvatarSize, OriginalFileName);
			}

			/// @brief Calculates the path where creator avatars will be cached
			/// @param UserId The User ID of the creator
			/// @return The fully-qualified path of the avatar cache directory
			Modio::filesystem::path MakeAvatarFolderPath(Modio::UserID UserId) const
			{
				return PlatformImplementation->MakeAvatarFolderPath(UserId);
			}

			/// @brief Gets the path for a mod's logo if one exists in the specified size
			/// @param ModID The mod ID for the mod
			/// @param Size the desired size
			/// @return Optional containing the fully-qualified path to the mod logo if one exists at that size, else
			/// empty Optional object
			Modio::Optional<Modio::filesystem::path> GetLogo(Modio::ModID ModID, Modio::LogoSize Size) const
			{
				return GetMediaInternal(MakeLogoFolderPath(ModID), Modio::Detail::ToString(Size));
			}

			/// @brief Gets the path for a user avatar image if one exists in the specified size
			/// @param UserId The user ID of the user
			/// @param Size The desired size
			/// @return Optional containing the fully-qualified path to the creator avatar if one exists at that size,
			/// else empty Optional object
			Modio::Optional<Modio::filesystem::path> GetAvatar(Modio::UserID UserId, Modio::AvatarSize Size) const
			{
				return GetMediaInternal(MakeAvatarFolderPath(UserId), Modio::Detail::ToString(Size));
			}

			Modio::Optional<Modio::filesystem::path> GetGalleryImage(Modio::ModID ModID, Modio::GallerySize Size,
																	 Modio::GalleryIndex Index) const
			{
				return GetMediaInternal(MakeGalleryFolderPath(ModID, Index), Modio::Detail::ToString(Size));
			}

			// @todo: Should all functions be able to operate on any directory anywhere or should we ensure that you
			// can only use the functions within a "sandbox" directory and don't accidentally create files outside it

			bool DirectoryExists(const Modio::filesystem::path& DirectoryPath) const
			{
				return PlatformImplementation->DirectoryExists(DirectoryPath);
			}

			/// @brief Creates a directory at an arbitrary fully-qualified location
			/// @param FolderPath The full path to the directory
			/// @return True if the folder already exists or was created successfully
			bool CreateFolder(const Modio::filesystem::path& FolderPath) const
			{
				return PlatformImplementation->CreateFolder(FolderPath);
			}

			/// @brief Checks for the existence of a file on disk
			/// @param FilePath The fully-qualified path of the file we wish to check
			/// @return True if the path has a filename component and if that file exists
			bool FileExists(const Modio::filesystem::path& FilePath) const
			{
				MODIO_PROFILE_SCOPE(FileExists);
				return PlatformImplementation->FileExists(FilePath);
			}

			/// @brief Replaces the file at DestinationFilePath with the file at SourceFilePath in a single blocking
			/// call. Source file must exist, and will be deleted on success.
			/// @param SourceFilePath Path to the file that will replace the file at DestinationFilePath.
			/// @param DestinationFilePath Path to the file to be replaced
			/// @return True on success
			bool MoveAndOverwriteFile(const Modio::filesystem::path& SourceFilePath,
									  const Modio::filesystem::path& DestinationFilePath)
			{
				return PlatformImplementation->MoveAndOverwriteFile(SourceFilePath, DestinationFilePath);
			}

			/// @brief Attempts to delete a file from disk
			/// @param FilePath The fully-qualified path of the file we wish to delete
			/// @return True if the path has a filename component and we deleted it successfully
			bool DeleteFile(const Modio::filesystem::path& FilePath) const
			{
				MODIO_PROFILE_SCOPE(DeleteFile);
				bool Result = PlatformImplementation->DeleteFile(FilePath);

				if (Result == false)
				{
					Modio::Detail::Logger().Log(LogLevel::Error, LogCategory::File,
												"DeleteFile operation was not successful, path: {}", FilePath.string());
				}

				return Result;
			}

			const Modio::filesystem::path& GetRootTempStoragePath() const
			{
				return PlatformImplementation->GetRootTempStoragePath();
			}

			const Modio::filesystem::path& GetRootLocalStoragePath() const
			{
				return PlatformImplementation->GetRootLocalStoragePath();
			}

			const Modio::filesystem::path GetModRootInstallationPath() const
			{
				return PlatformImplementation->GetModRootInstallationPath();
			}

			const Modio::filesystem::path GetTempModRootInstallationPath() const
			{
				return PlatformImplementation->GetTempModRootInstallationPath();
			}

			static Modio::filesystem::path GetDefaultModInstallationDirectory(Modio::GameID GameID)
			{
				Modio::filesystem::path CommonDataPath;
				Modio::Detail::FileSystemImplementation::GetDefaultCommonDataPath(CommonDataPath);
				return CommonDataPath / fmt::format("{}/mods/", GameID);
			}

			bool CheckSpaceAvailable(const Modio::filesystem::path& Destination, Modio::FileSize DesiredSize)
			{
				MODIO_PROFILE_SCOPE(CheckSpaceAvailable);
				return PlatformImplementation->CheckSpaceAvailable(Destination, DesiredSize);
			}

			Modio::FileSize GetSpaceAvailable(const Modio::filesystem::path& Destination)
			{
				MODIO_PROFILE_SCOPE(GetSpaceAvailable);
				return PlatformImplementation->GetSpaceAvailable(Destination);
			}

			/// @brief Recursively searches the media cache to create a queue of image paths and the total
			/// size of all cached images combined. Note that image paths are added to the queue as they are found
			/// without additional sorting.
			/// @param ImagePaths A queue to be populated with paths to images in the media cache.
			/// @param TotalSize The total combined size in bytes of all the images in the media cache. 
			/// @return Error code indicating success or failure of the operation.
			Modio::ErrorCode QueryImageCache(std::queue<Modio::filesystem::path>& ImagePaths,
											 Modio::FileSize& TotalSize)
			{
				uintmax_t OutSize = 0;
				std::queue<Modio::filesystem::path> OutPaths {};

				Modio::filesystem::path CachePath = MakeMediaCachePath();
				if (DirectoryExists(CachePath))
				{
					Modio::ErrorCode ec;
					auto Iterator = Modio::filesystem::recursive_directory_iterator(CachePath, ec);
					auto End = Modio::filesystem::recursive_directory_iterator();
					const std::vector<std::string> ImageExtensions = {".png", ".jpg", ".jpeg"};
					while (Iterator != End)
					{
						// ensure iterator is created and incremented correctly
						if (ec)
						{
							Modio::Detail::Logger().Log(LogLevel::Error, LogCategory::File,
														"Recursive directory iterator failed with error {}",
														ec.message());
							return Modio::ErrorCode(Modio::make_error_code(Modio::FilesystemError::ReadError));
						}
						// ignore directories
						bool bRegularFile = Iterator->is_regular_file(ec);
						if (ec || !bRegularFile)
						{
							Iterator.increment(ec);
							continue;
						}
						// check the current file is an image, and add its information if so
						const Modio::filesystem::path& FullPath = Iterator->path();
						for (const auto& Ext : ImageExtensions)
						{
							if (FullPath.extension() == Ext)
							{
								uintmax_t ImageSize = Modio::filesystem::file_size(FullPath, ec);
								if (ec)
								{
									Modio::Detail::Logger().Log(LogLevel::Error, LogCategory::File,
																"Failed to get size of image {} - error message: {}",
																FullPath.string(), ec.message());
									return Modio::ErrorCode(Modio::make_error_code(Modio::FilesystemError::ReadError));
								}
								OutSize += ImageSize;
								OutPaths.push(FullPath);
								break;
							}
						}
						Iterator.increment(ec);
					}
				}
				ImagePaths = OutPaths;
				TotalSize = Modio::FileSize(OutSize);
				Modio::Detail::Logger().Log(LogLevel::Trace, LogCategory::File,
											"Media cache contains {} images totalling {} bytes in size",
											ImagePaths.size(), TotalSize);
				return {};
			}

		private:
			/// @brief Internal method for getting filepath of a mod media file if it exists
			/// @param FolderPath The path that contains the different sizes for the image
			/// @param SizeSuffix Image size suffix
			/// @return Fully-qualified path to the mod media file, or empty Optional on error
			Modio::Optional<Modio::filesystem::path> GetMediaInternal(const Modio::filesystem::path& FolderPath,
																	  const std::string& SizeSuffix) const
			{
				namespace fs = Modio::filesystem;
				fs::path ImageCachePath = FolderPath;
				std::error_code ec;
				auto InitialIterator = fs::directory_iterator(ImageCachePath, ec);
				if (ec)
				{
					Modio::Detail::Logger().Log(LogLevel::Error, LogCategory::File,
												"GetMediaInternal failed to create a directory iterator with error {}",
												ec.message());
					return {};
				}

				for (auto& Entry = InitialIterator; Entry != end(InitialIterator); Entry.increment(ec))
				{
					bool bRegularFile = Entry->is_regular_file(ec);
					if (ec || !bRegularFile)
					{
						continue;
					}

					const fs::path& FullPath = Entry->path();
					fs::path FilenameNoExtension = FullPath.filename().stem();
					if (Modio::Detail::String::EndsWith(FilenameNoExtension.u8string(), fmt::format("_{}", SizeSuffix)))
					{
						return FullPath;
					}
				}

				return {};
			}

			std::shared_ptr<FileSystemImplementation> PlatformImplementation;
		};
	} // namespace Detail
} // namespace Modio
