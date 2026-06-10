/*
 *  Copyright (C) 2026 mod.io Pty Ltd. <https://mod.io>
 *
 *  This file is part of the mod.io SDK.
 *
 *  Distributed under the MIT License. (See accompanying file LICENSE or
 *   view online at <https://github.com/modio/modio-sdk/blob/main/LICENSE>)
 *
 */

#pragma once

#include "modio/core/ModioBuffer.h"
#include "modio/core/ModioErrorCode.h"
#include "modio/core/ModioLogger.h"
#include "modio/core/ModioStdTypes.h"
#include "modio/detail/AsioWrapper.h"
#include "modio/detail/ModioObjectTrack.h"
#include "modio/detail/ModioSDKSessionData.h"
#include "modio/detail/ModioStringHelpers.h"
#include "modio/file/ModioFile.h"

#include <asio/yield.hpp>

namespace Modio
{
	namespace Detail
	{
		/// @brief Recursively copies a folder from SourcePath to DestinationPath using async file read/write operations
		/// using Modio::Detail::File.

		class CopyTempModToInstallDirOp : public Modio::Detail::BaseOperation<CopyTempModToInstallDirOp>
		{
			struct CopyTempModToInstallDirImpl
			{
				Modio::filesystem::path SourceDirectoryPath;
				Modio::filesystem::path DestinationDirectoryPath;
				std::vector<std::pair<Modio::filesystem::path, Modio::filesystem::path>> FilesToCopy;
				std::vector<std::pair<Modio::filesystem::path, Modio::filesystem::path>>::iterator FileIterator;
				Modio::StableStorage<Modio::Detail::File> SourceFile;
				Modio::StableStorage<Modio::Detail::File> DestinationFile;
				Modio::Detail::DynamicBuffer TransferBuffer;
				std::uintmax_t CurrentFileSize = 0;
				std::uintmax_t BytesProcessed = 0;
				std::uintmax_t CurrentBufferSize = 0;
				Modio::Detail::FileService& PlatformFileService;
			};

			Modio::StableStorage<CopyTempModToInstallDirImpl> Impl;
			ModioAsio::coroutine CoroutineState {};
			static constexpr std::uintmax_t ChunkSize = 512 * 1024;

		public:
			CopyTempModToInstallDirOp(Modio::filesystem::path SourcePath, Modio::filesystem::path DestinationPath)
			{
				Impl = std::make_shared<CopyTempModToInstallDirImpl>(CopyTempModToInstallDirImpl {
					std::move(SourcePath),
					std::move(DestinationPath),
					{},
					{},
					nullptr,
					nullptr,
					{},
					0,
					0,
					0,
					Modio::Detail::Services::GetGlobalService<Modio::Detail::FileService>()});
			}

			template<typename CoroType>
			void operator()(CoroType& Self, Modio::ErrorCode ec = {})
			{
				if (!Impl)
				{
					Self.complete(Modio::make_error_code(Modio::GenericError::OperationCanceled));
					return;
				}

				if (!Modio::Detail::SDKSessionData::IsModManagementEnabled())
				{
					Self.complete(Modio::make_error_code(Modio::GenericError::OperationCanceled));
					return;
				}

				reenter(CoroutineState)
				{
					Modio::Detail::Logger().Log(
						Modio::LogLevel::Trace, Modio::LogCategory::File, "Begin copy directory from {} to {}",
						Modio::ToModioString(Impl->SourceDirectoryPath.u8string()), 
						Modio::ToModioString(Impl->DestinationDirectoryPath.u8string()));

					// Build the file list and replicate the directory structure
					{
						Modio::ErrorCode IteratorEc;
						Modio::filesystem::recursive_directory_iterator DirectoryIterator(Impl->SourceDirectoryPath,
																						  IteratorEc);
						if (IteratorEc)
						{
							Modio::Detail::Logger().Log(Modio::LogLevel::Error, Modio::LogCategory::File,
														"Failed to iterate source directory {}: {}",
														Modio::ToModioString(Impl->SourceDirectoryPath.u8string()), IteratorEc.message());
							Self.complete(Modio::make_error_code(Modio::FilesystemError::ReadError));
							return;
						}

						Impl->PlatformFileService.CreateFolder(Impl->DestinationDirectoryPath);

						while (DirectoryIterator != end(DirectoryIterator))
						{
							Modio::filesystem::path RelativePath = Modio::filesystem::relative(
								DirectoryIterator->path(), Impl->SourceDirectoryPath, IteratorEc);
							if (IteratorEc)
							{
								Self.complete(IteratorEc);
								return;
							}
							Modio::filesystem::path DestinationEntryPath =
								Impl->DestinationDirectoryPath / RelativePath;

							if (Modio::filesystem::is_directory(DirectoryIterator->path(), IteratorEc))
							{
								Impl->PlatformFileService.CreateFolder(DestinationEntryPath);
							}
							else if (Modio::filesystem::is_regular_file(DirectoryIterator->path(), IteratorEc))
							{
								Impl->FilesToCopy.emplace_back(DirectoryIterator->path(), DestinationEntryPath);
							}
							DirectoryIterator.increment(IteratorEc);
						}
					}

					Impl->FileIterator = Impl->FilesToCopy.begin();

					while (Impl->FileIterator != Impl->FilesToCopy.end())
					{
						// Ensure the parent directory of the destination exists
						Impl->PlatformFileService.CreateFolder(Impl->FileIterator->second.parent_path());

						// Open source file for reading and destination for writing
						Impl->SourceFile = std::make_shared<Modio::Detail::File>(Impl->FileIterator->first,
																				 Modio::Detail::FileMode::ReadOnly);
						Impl->DestinationFile = std::make_shared<Modio::Detail::File>(
							Impl->FileIterator->second, Modio::Detail::FileMode::ReadWrite, true);

						Impl->CurrentFileSize = Impl->SourceFile->GetFileSize();
						Impl->BytesProcessed = 0;

						Modio::Detail::Logger().Log(Modio::LogLevel::Trace, Modio::LogCategory::File,
													"Copying file {} ({} bytes) to {}",
													Modio::ToModioString(Impl->FileIterator->first.u8string()), Impl->CurrentFileSize,
													Modio::ToModioString(Impl->FileIterator->second.u8string()));

						while (Impl->BytesProcessed < Impl->CurrentFileSize)
						{
							Impl->TransferBuffer.Clear();

							yield Impl->SourceFile->ReadAsync(
								std::min<std::uintmax_t>(ChunkSize, Impl->CurrentFileSize - Impl->BytesProcessed),
								Impl->TransferBuffer, std::move(Self));

							if (ec && ec != Modio::GenericError::EndOfFile)
							{
								Modio::Detail::Logger().Log(Modio::LogLevel::Error, Modio::LogCategory::File,
															"Failed to read from {}: {}",
															Modio::ToModioString(Impl->FileIterator->first.u8string()), ec.message());
								Self.complete(ec);
								return;
							}

							Impl->CurrentBufferSize = Impl->TransferBuffer.size();
							Impl->BytesProcessed += Impl->CurrentBufferSize;

							if (Impl->CurrentBufferSize == 0)
							{
								Modio::Detail::Logger().Log(Modio::LogLevel::Error, Modio::LogCategory::File,
															"Read zero bytes from {}",
															Modio::ToModioString(Impl->FileIterator->first.u8string()));
								Self.complete(Modio::make_error_code(Modio::FilesystemError::ReadError));
								return;
							}

							while (Impl->TransferBuffer.size())
							{
								yield Impl->DestinationFile->WriteAsync(
									Impl->TransferBuffer.TakeInternalBuffer().value(), std::move(Self));
								if (ec)
								{
									Modio::Detail::Logger().Log(Modio::LogLevel::Error, Modio::LogCategory::File,
																"Failed to write to {}: {}",
																Modio::ToModioString(Impl->FileIterator->second.u8string()), ec.message());
									Self.complete(ec);
									return;
								}
							}
						}

						Modio::Detail::Logger().Log(
							Modio::LogLevel::Trace, Modio::LogCategory::File, "Finished copying file {} to {}",
							Modio::ToModioString(Impl->FileIterator->first.u8string()), Modio::ToModioString(Impl->FileIterator->second.u8string()));

						// Close files
						Impl->SourceFile.reset();
						Impl->DestinationFile.reset();

						Impl->FileIterator++;

						yield ModioAsio::post(Modio::Detail::Services::GetGlobalContext().get_executor(),
											  std::move(Self));
					}

					Modio::Detail::Logger().Log(
						Modio::LogLevel::Trace, Modio::LogCategory::File, "Finished copy directory from {} to {}",
						Modio::ToModioString(Impl->SourceDirectoryPath.u8string()), Modio::ToModioString(Impl->DestinationDirectoryPath.u8string()));
					Impl.reset();
					Self.complete({});
					return;
				}
			}
		};

		template<typename CopyTempModToInstallDirCompleteCallback>
		void CopyTempModToInstallDirAsync(Modio::filesystem::path SourcePath, Modio::filesystem::path DestinationPath,
										  CopyTempModToInstallDirCompleteCallback&& OnCopyTempModToInstallDirComplete)
		{
			return ModioAsio::async_compose<CopyTempModToInstallDirCompleteCallback, void(Modio::ErrorCode)>(
				Modio::Detail::CopyTempModToInstallDirOp(SourcePath, DestinationPath),
				OnCopyTempModToInstallDirComplete, Modio::Detail::Services::GetGlobalContext().get_executor());
		}

	} // namespace Detail
} // namespace Modio

#include <asio/unyield.hpp>