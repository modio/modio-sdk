/*
 *  Copyright (C) 2021 mod.io Pty Ltd. <https://mod.io>
 *
 *  This file is part of the mod.io SDK.
 *
 *  Distributed under the MIT License. (See accompanying file LICENSE or
 *   view online at <https://github.com/modio/modio-sdk/blob/main/LICENSE>)
 *
 */

// MIRRORED TO gdk/file/FileSystemImplementation.h, UPDATE THAT FILE IF THIS IS UPDATED
#pragma once
#include "common/detail/ops/file/DeleteFolderOp.h"
#include "common/detail/ops/file/ReadSomeFromFileBufferedOp.h"
#include "common/detail/ops/file/ReadSomeFromFileOp.h"
#include "common/detail/ops/file/StreamReadOp.h"
#include "common/file/FileObjectImplementation.h"
#include "modio/core/ModioInitializeOptions.h"
#include "modio/detail/FmtWrapper.h"
#include "modio/detail/ModioConstants.h"
#include "modio/detail/entities/ModioAvatar.h"
#include "modio/detail/entities/ModioImage.h"
#include "modio/detail/entities/ModioLogo.h"
#include "modio/detail/file/IFileObjectImplementation.h"
#include "modio/detail/file/IFileServiceImplementation.h"
#include "modio/file/ModioFileService.h"
#include <memory>

namespace Modio
{
	namespace Detail
	{
		template<typename Subplatform>
		class FileSystemImplementationBase : public Modio::Detail::IFileServiceImplementation
		{
			asio::io_context::service& OwningService;
			/// <summary>
			/// The root path for local persistent storage - all file paths are treated as relative to this root
			/// </summary>
			Modio::filesystem::path RootLocalStoragePath;
			Modio::filesystem::path UserDataPath;
			Modio::filesystem::path RootTempPath;
			Modio::GameID CurrentGameID;
			Modio::UserHandleType CurrentSessionIdentifier;

		protected:
			std::shared_ptr<Modio::Detail::FileSharedState> SharedState;

		public:
			/// <summary>
			/// Member typedef used by Modio::Detail::FileSystem
			/// </summary>
			using IOObjectImplementationType = std::shared_ptr<FileObjectImplementation>;

			std::vector<std::weak_ptr<FileObjectImplementation>> OpenFileObjects;

			FileSystemImplementationBase(asio::io_context::service& OwningService) : OwningService(OwningService)
			{
				SharedState = static_cast<Subplatform*>(this)->MakeSharedState();
			}

			/// <summary>
			/// Delegated initializer for platform implementations of IO objects
			/// </summary>
			/// <param name="PlatformIOObjectInstance"></param>
			void InitializeIOObjectImplementation(IOObjectImplementationType& PlatformIOObjectInstance)
			{
				PlatformIOObjectInstance =
					std::make_shared<FileObjectImplementation>(OwningService.get_io_context(), RootLocalStoragePath);

				OpenFileObjects.erase(std::remove_if(OpenFileObjects.begin(), OpenFileObjects.end(),
													 [](const std::weak_ptr<FileObjectImplementation>& CheckedObject) {
														 return CheckedObject.expired();
													 }),
									  OpenFileObjects.end());
				OpenFileObjects.push_back(PlatformIOObjectInstance);
			}
			void DestroyIOObjectImplementation(IOObjectImplementationType& PlatformIOObjectInstance)
			{
				if (PlatformIOObjectInstance)
				{
					PlatformIOObjectInstance->Destroy();
				}
			}

			void MoveIOObjectImplementation(IOObjectImplementationType& Implementation,
											IOObjectImplementationType& OtherImplementation)
			{
				Implementation = std::move(OtherImplementation);
			}

			void Shutdown() override
			{
				if (SharedState)
				{
					SharedState->bCancelRequested = true;
				}
				for (auto FileObject : OpenFileObjects)
				{
					if (auto FileImpl = FileObject.lock())
					{
						FileImpl->CancelAll();
					}
				}
			}

			template<typename CompletionTokenType>
			auto InitializeAsync(Modio::InitializeOptions InitParams, CompletionTokenType&& Token)
			{
				CurrentGameID = InitParams.GameID;
				CurrentSessionIdentifier = InitParams.User;
				return asio::async_compose<CompletionTokenType, void(std::error_code)>(
					static_cast<Subplatform*>(this)->MakeInitializeStorageOp(InitParams, RootLocalStoragePath,
																			 UserDataPath, RootTempPath),
					Token, Modio::Detail::Services::GetGlobalContext().get_executor());
			}

			template<typename CompletionTokenType>
			auto WriteSomeAtAsync(IOObjectImplementationType PlatformIOObjectInstance, std::uintmax_t Offset,
								  Modio::Detail::Buffer Buffer, CompletionTokenType&& Token)
			{
				return asio::async_compose<CompletionTokenType, void(std::error_code)>(
					static_cast<Subplatform*>(this)->MakeWriteSomeToFileOp(PlatformIOObjectInstance, Offset,
																		   std::move(Buffer)),
					Token, Modio::Detail::Services::GetGlobalContext().get_executor());
			}

			template<typename CompletionTokenType>
			auto ReadSomeAtAsync(IOObjectImplementationType PlatformIOObjectInstance, std::uintmax_t Offset,
								 std::uintmax_t Length, CompletionTokenType&& Token)
			{
				return asio::async_compose<CompletionTokenType,
										   void(std::error_code, Modio::Optional<Modio::Detail::Buffer>)>(
					ReadSomeFromFileOp(PlatformIOObjectInstance, Offset, Length), Token,
					Modio::Detail::Services::GetGlobalContext().get_executor());
			}

			template<typename CompletionTokenType>
			auto ReadSomeAtAsync(IOObjectImplementationType PlatformIOObjectInstance, std::uintmax_t Offset,
								 std::uintmax_t MaxBytesToRead, Modio::Detail::DynamicBuffer Destination,
								 CompletionTokenType&& Token)
			{
				return asio::async_compose<CompletionTokenType, void(std::error_code)>(
					ReadSomeFromFileBufferedOp(PlatformIOObjectInstance, Modio::FileOffset(Offset), MaxBytesToRead,
											   Destination),
					Token, Modio::Detail::Services::GetGlobalContext().get_executor());
			}

			template<typename CompletionTokenType>
			auto ReadAsync(IOObjectImplementationType PlatformIOObjectInstance, std::uintmax_t MaxBytesToRead,
						   Modio::Detail::DynamicBuffer Destination, CompletionTokenType&& Token)
			{
				return asio::async_compose<CompletionTokenType, void(std::error_code)>(
					ReadSomeFromFileBufferedOp(PlatformIOObjectInstance, PlatformIOObjectInstance->Tell(),
											   MaxBytesToRead, Destination),
					Token, Modio::Detail::Services::GetGlobalContext().get_executor());
			}

			template<typename CompletionTokenType>
			auto WriteAsync(IOObjectImplementationType PlatformIOObjectInstance, Modio::Detail::Buffer Buffer,
							CompletionTokenType&& Token)
			{
				return asio::async_compose<CompletionTokenType, void(std::error_code)>(
					static_cast<Subplatform*>(this)->MakeStreamWriteOp(PlatformIOObjectInstance, std::move(Buffer)),
					Token, Modio::Detail::Services::GetGlobalContext().get_executor());
			}

			template<typename CompletionTokenType>
			auto DeleteFolderAsync(Modio::filesystem::path FolderPath, CompletionTokenType&& Token)
			{
				return asio::async_compose<CompletionTokenType, void(std::error_code)>(
					DeleteFolderOp(FolderPath, SharedState), Token,
					Modio::Detail::Services::GetGlobalContext().get_executor());
			}

			// do we need to maintain a cache of temporary files?
			Modio::filesystem::path MakeTempFilePath(std::string FileName) const override
			{
				return RootTempPath / FileName;
			}

			Modio::filesystem::path MakeModPath(Modio::ModID ModID) const override
			{
				return RootLocalStoragePath / fmt::format("{}/mods/{}", CurrentGameID, ModID);
			}

			Modio::filesystem::path MakeTempModPath(Modio::ModID ModID) const override
			{
				return RootLocalStoragePath /
					   fmt::format("{}/temp/{}/{}", CurrentGameID, CurrentSessionIdentifier, ModID);
			}

			Modio::filesystem::path GetModRootInstallationPath() const override
			{
				return RootLocalStoragePath / fmt::format("{}/mods/", CurrentGameID);
			}

			Modio::filesystem::path GetTempModRootInstallationPath() const override
			{
				return RootLocalStoragePath / fmt::format("{}/temp/{}/", CurrentGameID, CurrentSessionIdentifier);
			}

			Modio::filesystem::path MakeMediaCachePath() const override
			{
				return RootLocalStoragePath / fmt::format("{}/cache/", CurrentGameID);
			}

			Modio::filesystem::path MakeModMediaFilePath(Modio::ModID ModID, Modio::LogoSize Size,
														 const std::string& OriginalFilename) const override
			{
				const Modio::filesystem::path OriginalFilePath = OriginalFilename;
				return MakeLogoFolderPath(ModID) / fmt::format("{}_{}{}", OriginalFilePath.stem().u8string(),
															   Modio::Detail::ToString(Size),
															   OriginalFilePath.extension().u8string());
			}

			Modio::filesystem::path MakeModMediaFilePath(Modio::ModID ModID, Modio::GallerySize Size,
														 Modio::GalleryIndex ImageIndex,
														 const std::string& OriginalFileName) const override
			{
				const Modio::filesystem::path OriginalFilePath = OriginalFileName;
				return MakeGalleryFolderPath(ModID, ImageIndex) /
					   fmt::format("{}_{}{}", OriginalFilePath.stem().u8string(), Modio::Detail::ToString(Size),
								   OriginalFilePath.extension().u8string());
			}

			Modio::filesystem::path MakeLogoFolderPath(Modio::ModID ModID) const override
			{
				// @todonow: Change this to temporary storage
				return MakeMediaCachePath() / fmt::format("mods/{}/logos/", ModID);
			}

			Modio::filesystem::path MakeGalleryFolderPath(Modio::ModID ModID,
														  Modio::GalleryIndex ImageIndex) const override
			{
				// @todonow: Change this to temporary storage
				return MakeMediaCachePath() / fmt::format("mods/{}/gallery/{}/", ModID, ImageIndex);
			}

			Modio::filesystem::path MakeUserMediaFilePath(Modio::UserID UserId, Modio::AvatarSize Size,
														  const std::string& OriginalFilename) const override
			{
				const Modio::filesystem::path OriginalFilePath = OriginalFilename;
				return MakeAvatarFolderPath(UserId) / fmt::format("{}_{}{}", OriginalFilePath.stem().u8string(),
																  Modio::Detail::ToString(Size),
																  OriginalFilePath.extension().u8string());
			}

			Modio::filesystem::path MakeAvatarFolderPath(Modio::UserID UserId) const override
			{
				// @todonow: Change this to temporary storage
				return MakeMediaCachePath() / fmt::format("users/{}/avatars/", UserId);
			}

			bool DirectoryExists(const Modio::filesystem::path& DirectoryPath) const override
			{
				Modio::ErrorCode ec;
				if (Modio::filesystem::exists(DirectoryPath, ec) && !ec)
				{
					return Modio::filesystem::is_directory(DirectoryPath, ec) && !ec;
				}

				return false;
			}

			bool CreateFolder(const Modio::filesystem::path& FolderPath) const override
			{
				std::error_code ec;
				// Using function calling taking ec as it doesn't throw when failing
				bool Result = Modio::filesystem::create_directories(FolderPath, ec);

				// @todo: Right now we are discarding relevant information about the error of why the path can't be
				// created take a decision about how we should get that information back in a good way

				if (!Result && !ec)
				{
					// This is a special case, this means that the directory wasn't created as it's already there
					// so let's call that a success
					return true;
				}

				// Error condition, print error
				if (!Result)
				{
					Modio::Detail::Logger().Log(LogLevel::Warning, LogCategory::File,
												"Failed to create directory: {}, with code: {} and message {}",
												FolderPath.u8string(), ec.value(), ec.message());
				}

				return Result;
			}

			bool FileExists(const Modio::filesystem::path& FilePath) const override
			{
				namespace fs = Modio::filesystem;

				std::error_code ec;
				if (fs::exists(FilePath, ec) && !ec)
				{
					return fs::is_regular_file(FilePath, ec) && !ec;
				}

				return false;
			}

			bool DeleteFile(const Modio::filesystem::path& FilePath) const
			{
				namespace fs = Modio::filesystem;

				std::error_code ec;
				if (!fs::is_regular_file(FilePath, ec) && !ec)
				{
					Modio::Detail::Logger().Log(
						LogLevel::Warning, LogCategory::File,
						"Failed to delete file: {}, not a regular file. Additional information: {}",
						FilePath.u8string(), ec ? ec.message() : "");
					return false;
				}
				if (!Modio::filesystem::remove(FilePath, ec) || ec)
				{
					Modio::Detail::Logger().Log(LogLevel::Warning, LogCategory::File,
												"Failed to delete file: {}, with code {} and message: \"{}\"",
												FilePath.u8string(), ec.value(), ec.message());
					return false;
				}

				return true;
			}

			bool MoveAndOverwriteFile(const Modio::filesystem::path& SourceFilePath,
									  const Modio::filesystem::path& DestinationFilePath)
			{
				if (!FileExists(SourceFilePath))
				{
					Modio::Detail::Logger().Log(
						LogLevel::Error, LogCategory::File,
						"Source file {} does not exist. Failed to perform MoveAndOverwriteFile()",
						SourceFilePath.u8string());
					return false;
				}
				if (!FileExists(DestinationFilePath))
				{
					// No destination file to overwrite, can rename the source file instead
					Modio::Detail::Logger().Log(LogLevel::Warning, LogCategory::File,
												"Destination file to overwrite does not exist: {}",
												DestinationFilePath.u8string());
					Modio::Detail::Logger().Log(LogLevel::Info, LogCategory::File, "Renaming file {} to {}",
												SourceFilePath.u8string(), DestinationFilePath.u8string());

					Modio::ErrorCode ec;
					Modio::filesystem::rename(SourceFilePath, DestinationFilePath, ec);
					if (ec)
					{
						Modio::Detail::Logger().Log(LogLevel::Error, LogCategory::File,
													"Failed to rename file {} to {}, with code {} and message: \"{}\"",
													SourceFilePath.u8string(), DestinationFilePath.u8string(),
													ec.value(), ec.message());
						return false;
					}
					else
					{
						return true;
					}
				}

				Modio::Detail::Logger().Log(LogLevel::Detailed, LogCategory::File, "Replacing file {} with {}",
											DestinationFilePath.u8string(), SourceFilePath.u8string());

				// Ignore merge and ACL errors because we're only concerned about the data itself, not security and
				// location
				if (ReplaceFileW(DestinationFilePath.c_str(), SourceFilePath.c_str(), nullptr,
								 REPLACEFILE_IGNORE_MERGE_ERRORS | REPLACEFILE_IGNORE_ACL_ERRORS, nullptr, nullptr))
				{
					return true;
				}
				else
				{
					// Store GetLastError() so it's not lost in UE implementation
					DWORD LastError = GetLastError();
					Modio::Detail::Logger().Log(LogLevel::Error, LogCategory::File,
												"Failed to replace file {} with {}. Received error {}",
												DestinationFilePath.u8string(), SourceFilePath.u8string(), LastError);
					return false;
				}
			}

			const Modio::filesystem::path& UserDataFolder() const override
			{
				return UserDataPath;
			}

			Modio::filesystem::path LocalMetadataFolder() const override
			{
				// No longer static const because we want to be able to reconfigure RootLocalStoragePath;
				// Note trailing slash
				Modio::filesystem::path MetadataPath =
					Modio::filesystem::path(RootLocalStoragePath) / fmt::format("{}/metadata/", CurrentGameID);

				return MetadataPath;
			}

			const Modio::filesystem::path& GetRootLocalStoragePath() const override
			{
				return RootLocalStoragePath;
			}

			const Modio::filesystem::path& GetRootTempStoragePath() const override
			{
				return RootTempPath;
			}

			Modio::ErrorCode ApplyGlobalConfigOverrides(const std::map<std::string, std::string> Overrides) override
			{
				auto RootValue = Overrides.find(Modio::Detail::Constants::JSONKeys::RootLocalStoragePath);
				if (RootValue != Overrides.end())
				{
					// Note trailing slash
					RootLocalStoragePath = Modio::filesystem::path(RootValue->second) / "";
				}

				Modio::Detail::Logger().Log(Modio::LogLevel::Info, Modio::LogCategory::File,
											"Overriding RootLocalStoragePath to {}", RootLocalStoragePath.u8string());

				if (!CreateFolder(RootLocalStoragePath))
				{
					return Modio::make_error_code(Modio::FilesystemError::UnableToCreateFolder);
				}
				else
				{
					return {};
				}
			}

			bool CheckSpaceAvailable(const Modio::filesystem::path& Destination, Modio::FileSize DesiredSize) override
			{
				const Modio::FileSize SpaceAvailable = GetSpaceAvailable(Destination);
				if (SpaceAvailable >= DesiredSize)
				{
					return true;
				}
				else
				{
					return false;
				}
			}

			Modio::FileSize GetSpaceAvailable(const Modio::filesystem::path& Destination) override
			{
				Modio::ErrorCode ec;
				const Modio::filesystem::path ValidDestination = Destination.root_path();

				const Modio::filesystem::space_info SpaceInfo = Modio::filesystem::space(ValidDestination, ec);
				if (!ec)
				{
					return FileSize(SpaceInfo.available);
				}

				const DWORD LastError = GetLastError();
				Modio::Detail::Logger().Log(LogLevel::Error, LogCategory::File,
											"Failed to get space available. Received error {}", LastError);
				return Modio::FileSize(0);
			}
		};
	} // namespace Detail
} // namespace Modio
