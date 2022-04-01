/*
 *  Copyright (C) 2021 mod.io Pty Ltd. <https://mod.io>
 *
 *  This file is part of the mod.io SDK. All rights reserved.
 *  Redistribution prohibited without prior written consent of mod.io Pty Ltd.
 *
 */

#pragma once
#include "file/FileObjectImplementation.h"
#include "macos/FileSharedState.h"
#include "macos/detail/ops/file/DeleteFolderOp.h"
#include "macos/detail/ops/file/InitializeFileSystemOp.h"
#include "macos/detail/ops/file/ReadSomeFromFileBufferedOp.h"
#include "macos/detail/ops/file/ReadSomeFromFileOp.h"
#include "macos/detail/ops/file/WriteSomeToFileOp.h"
#include "modio/detail/FmtWrapper.h"
#include "modio/detail/ModioConstants.h"
#include "modio/detail/ModioSDKSessionData.h"
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
		class FileSystemImplementation : public Modio::Detail::IFileServiceImplementation
		{
			asio::io_context::service& OwningService;
			/// <summary>
			/// The root path for local persistent storage - all file paths are treated as relative to this root
			/// </summary>
			Modio::filesystem::path RootLocalStoragePath;
			Modio::filesystem::path UserDataPath;
			Modio::filesystem::path RootTempPath;
			Modio::GameID CurrentGameID;
			std::shared_ptr<Modio::Detail::FileSharedState> SharedState;

		public:
			/// <summary>
			/// Member typedef used by Modio::Detail::FileSystem
			/// </summary>
			using IOObjectImplementationType = std::shared_ptr<FileObjectImplementation>;

			std::vector<std::weak_ptr<FileObjectImplementation>> OpenFileObjects;

			FileSystemImplementation(asio::io_context::service& OwningService) : OwningService(OwningService)
			{
				SharedState = std::make_shared<Modio::Detail::FileSharedState>();
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

			void Shutdown()
			{
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
				return asio::async_compose<CompletionTokenType, void(Modio::ErrorCode)>(
					Modio::Detail::InitializeFileSystemOp(InitParams, SharedState, RootLocalStoragePath, UserDataPath,
														  RootTempPath),
					Token, Modio::Detail::Services::GetGlobalContext().get_executor());
			}

			template<typename CompletionTokenType>
			auto WriteSomeAtAsync(IOObjectImplementationType PlatformIOObjectInstance, std::uintmax_t Offset,
								  Modio::Detail::Buffer Buffer, CompletionTokenType&& Token)
			{
				return asio::async_compose<CompletionTokenType, void(std::error_code)>(
					WriteSomeToFileOp(PlatformIOObjectInstance, SharedState, Modio::FileOffset(Offset),
									  std::move(Buffer)),
					Token, Modio::Detail::Services::GetGlobalContext().get_executor());
			}

			template<typename CompletionTokenType>
			auto ReadSomeAtAsync(IOObjectImplementationType PlatformIOObjectInstance, std::uintmax_t Offset,
								 std::uintmax_t Length, CompletionTokenType&& Token)
			{
				return asio::async_compose<CompletionTokenType,
										   void(std::error_code, Modio::Optional<Modio::Detail::Buffer>)>(
					ReadSomeFromFileOp(PlatformIOObjectInstance, SharedState, Modio::FileOffset(Offset),
									   Modio::FileSize(Length)),
					Token, Modio::Detail::Services::GetGlobalContext().get_executor());
			}

			template<typename CompletionTokenType>
			auto ReadSomeAtAsync(IOObjectImplementationType PlatformIOObjectInstance, std::uintmax_t Offset,
								 std::uintmax_t MaxBytesToRead, Modio::Detail::DynamicBuffer Destination,
								 CompletionTokenType&& Token)
			{
				return asio::async_compose<CompletionTokenType, void(std::error_code)>(
					ReadSomeFromFileBufferedOp(PlatformIOObjectInstance, SharedState, Modio::FileOffset(Offset),
											   Modio::FileSize(MaxBytesToRead), Destination),
					Token, Modio::Detail::Services::GetGlobalContext().get_executor());
			}

			template<typename CompletionTokenType>
			auto ReadAsync(IOObjectImplementationType PlatformIOObjectInstance, std::uintmax_t MaxBytesToRead,
						   Modio::Detail::DynamicBuffer Destination, CompletionTokenType&& Token)
			{
				return asio::async_compose<CompletionTokenType, void(std::error_code)>(
					ReadSomeFromFileBufferedOp(PlatformIOObjectInstance, SharedState, {},
											   Modio::FileSize(MaxBytesToRead), Destination),
					Token, Modio::Detail::Services::GetGlobalContext().get_executor());
			}

			template<typename CompletionTokenType>
			auto WriteAsync(IOObjectImplementationType PlatformIOObjectInstance, Modio::Detail::Buffer Buffer,
							CompletionTokenType&& Token)
			{
				return asio::async_compose<CompletionTokenType, void(std::error_code)>(
					WriteSomeToFileOp(PlatformIOObjectInstance, SharedState, {}, std::move(Buffer)), Token,
					Modio::Detail::Services::GetGlobalContext().get_executor());
			}

			template<typename CompletionTokenType>
			auto DeleteFolderAsync(Modio::filesystem::path FolderPath, CompletionTokenType&& Token)
			{
				return asio::async_compose<CompletionTokenType, void(std::error_code)>(
					DeleteFolderOp(FolderPath), Token, Modio::Detail::Services::GetGlobalContext().get_executor());
			}

			// do we need to maintain a cache of temporary files?
			Modio::filesystem::path MakeTempFilePath(std::string FileName) const
			{
				return RootTempPath / FileName;
			}

			Modio::filesystem::path MakeModPath(Modio::ModID ModID) const
			{
				return RootLocalStoragePath / fmt::format("{}/mods/{}", CurrentGameID, ModID);
			}

			Modio::filesystem::path MakeModMediaFilePath(Modio::ModID ModID, Modio::LogoSize Size,
														 const std::string& OriginalFilename) const
			{
				const Modio::filesystem::path OriginalFilePath = OriginalFilename;
				return MakeLogoFolderPath(ModID) / fmt::format("{}_{}{}", OriginalFilePath.stem().u8string(),
															   Modio::Detail::ToString(Size),
															   OriginalFilePath.extension().u8string());
			}

			Modio::filesystem::path MakeModMediaFilePath(Modio::ModID ModID, Modio::GallerySize Size,
														 Modio::GalleryIndex ImageIndex,
														 const std::string& OriginalFileName) const
			{
				const Modio::filesystem::path OriginalFilePath = OriginalFileName;
				return MakeGalleryFolderPath(ModID, ImageIndex) /
					   fmt::format("{}_{}{}", OriginalFilePath.stem().u8string(), Modio::Detail::ToString(Size),
								   OriginalFilePath.extension().u8string());
			}

			Modio::filesystem::path MakeLogoFolderPath(Modio::ModID ModID) const
			{
				// @todonow: Change this to temporary storage
				return RootLocalStoragePath / fmt::format("{}/cache/mods/{}/logos/", CurrentGameID, ModID);
			}

			Modio::filesystem::path MakeGalleryFolderPath(Modio::ModID ModID, int ImageIndex) const
			{
				// @todonow: Change this to temporary storage
				return RootLocalStoragePath /
					   fmt::format("{}/cache/mods/{}/gallery/{}/", CurrentGameID, ModID, ImageIndex);
			}

			Modio::filesystem::path MakeUserMediaFilePath(Modio::UserID UserId, Modio::AvatarSize Size,
														  const std::string& OriginalFilename) const
			{
				const Modio::filesystem::path OriginalFilePath = OriginalFilename;
				return MakeAvatarFolderPath(UserId) / fmt::format("{}_{}{}", OriginalFilePath.stem().u8string(),
																  Modio::Detail::ToString(Size),
																  OriginalFilePath.extension().u8string());
			}

			Modio::filesystem::path MakeAvatarFolderPath(Modio::UserID UserId) const
			{
				// @todonow: Change this to temporary storage
				return RootLocalStoragePath / fmt::format("{}/cache/users/{}/avatars/", CurrentGameID, UserId);
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

			bool CreateFolder(const Modio::filesystem::path& FolderPath) const
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

			bool FileExists(const Modio::filesystem::path& FilePath) const
			{
				std::error_code ec;
				if (Modio::filesystem::exists(FilePath, ec) && !ec)
				{
					return Modio::filesystem::is_regular_file(FilePath, ec) && !ec;
				}

				return false;
			}

			bool DeleteFile(const Modio::filesystem::path& FilePath) const
			{
				std::error_code ec;

				if (!Modio::filesystem::is_regular_file(FilePath, ec) && !ec)
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

			const Modio::filesystem::path& UserDataFolder() const
			{
				return UserDataPath;
			}

			virtual const Modio::filesystem::path LocalMetadataFolder() const override
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

			virtual Modio::ErrorCode ApplyGlobalConfigOverrides(
				const std::map<std::string, std::string> Overrides) override
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
				Modio::ErrorCode ec;
				Modio::filesystem::path ValidDestination = Destination.root_path();

				Modio::filesystem::space_info SpaceInfo = Modio::filesystem::space(ValidDestination, ec);
				if (!ec && SpaceInfo.available >= DesiredSize)
				{
					return true;
				}
				else
				{
					return false;
				}
			}
		};
	} // namespace Detail
} // namespace Modio
