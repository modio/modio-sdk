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
#include "modio/detail/ModioStringHelpers.h"
#include "modio/detail/entities/ModioAvatar.h"
#include "modio/detail/entities/ModioImage.h"
#include "modio/detail/entities/ModioLogo.h"
#include <iostream>
#include <memory>

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

			void move_assign(implementation_type& Implementation, Modio::Detail::FileService& OtherService,
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
															   std::forward<CompletionHandlerType>(std::move(Handler)));
			}

			template<typename CompletionTokenType>
			auto WriteSomeAtAsync(implementation_type& PlatformIOObject, std::uintmax_t Offset,
								  Modio::Detail::Buffer Buffer, CompletionTokenType&& Token)
			{
				return PlatformImplementation->WriteSomeAtAsync(PlatformIOObject, Offset, std::move(Buffer),
																std::forward<CompletionTokenType>(std::move(Token)));
			}

			template<typename CompletionTokenType>
			auto ReadSomeAtAsync(implementation_type& PlatformIOObject, std::uintmax_t Offset, std::uintmax_t Length,
								 CompletionTokenType&& Token)
			{
				return PlatformImplementation->ReadSomeAtAsync(PlatformIOObject, Offset, Length,
															   std::forward<CompletionTokenType>(std::move(Token)));
			}

			template<typename CompletionTokenType>
			auto ReadSomeAtAsync(implementation_type& PlatformIOObject, std::uintmax_t Offset,
								 std::uintmax_t MaxBytesToRead, Modio::Detail::DynamicBuffer Destination,
								 CompletionTokenType&& Token)
			{
				return PlatformImplementation->ReadSomeAtAsync(PlatformIOObject, Offset, MaxBytesToRead, Destination,
															   std::forward<CompletionTokenType>(std::move(Token)));
			}

			template<typename CompletionTokenType>
			auto ReadAsync(implementation_type& PlatformIOObject, std::uintmax_t MaxBytesToRead,
						   Modio::Detail::DynamicBuffer Destination, CompletionTokenType&& Token)
			{
				return PlatformImplementation->ReadAsync(PlatformIOObject, MaxBytesToRead, Destination,
														 std::forward<CompletionTokenType>(std::move(Token)));
			}

			template<typename CompletionTokenType>
			auto WriteAsync(implementation_type& PlatformIOObject, Modio::Detail::Buffer Buffer,
							CompletionTokenType&& Token)
			{
				return PlatformImplementation->WriteAsync(PlatformIOObject, std::move(Buffer),
														  std::forward<CompletionTokenType>(std::move(Token)));
			}

			template<typename CompletionTokenType>
			auto DeleteFolderAsync(Modio::filesystem::path FolderPath, CompletionTokenType&& Token)
			{
				return PlatformImplementation->DeleteFolderAsync(FolderPath,
																 std::forward<CompletionTokenType>(std::move(Token)));
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

			Modio::filesystem::path MakeModPath(Modio::ModID ModID) const
			{
				return PlatformImplementation->MakeModPath(ModID);
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
				return PlatformImplementation->FileExists(FilePath);
			}

			/// @brief Attempts to delete a file from disk
			/// @param FilePath The fully-qualifieid path of the file we wish to delete
			/// @return True if the path has a filename component and we deleted it successfully
			bool DeleteFile(const Modio::filesystem::path& FilePath) const
			{
				return PlatformImplementation->DeleteFile(FilePath);
			}

			const Modio::filesystem::path& GetRootLocalStoragePath() const
			{
				return PlatformImplementation->GetRootLocalStoragePath();
			}

			bool CheckSpaceAvailable(const Modio::filesystem::path& Destination, Modio::FileSize DesiredSize)
			{
				return PlatformImplementation->CheckSpaceAvailable(Destination, DesiredSize);
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
