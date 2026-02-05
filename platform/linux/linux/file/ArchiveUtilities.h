/*
 *  Copyright (C) 2021-2025 mod.io Pty Ltd. <https://mod.io>
 *
 *  This file is part of the mod.io SDK.
 *
 *  Distributed under the MIT License. (See accompanying file LICENSE or
 *   view online at <https://github.com/modio/modio-sdk/blob/main/LICENSE>)
 *
 */

#pragma once

#include "modio/compression/ModioArchiveReader.h"
#include "modio/core/ModioCoreTypes.h"
#include "modio/core/ModioErrorCode.h"
#include "modio/core/ModioServices.h"
#include "modio/detail/AsioWrapper.h"
#include "modio/detail/HedleyWrapper.h"
#include "modio/detail/ModioProfiling.h"
#include "modio/file/ModioFileService.h"

namespace Modio
{
	namespace Detail
	{

#include <asio/yield.hpp>
		class ValidateDiskSpaceForArchiveOp
		{
			/// @docinternal
			/// @brief A structure to represent the storage requirements for an uncompressed folder
			struct ArchiveDiskRequirements
			{
				/// @brief the total disk space required for the archive
				size_t DiskSize;
				/// @brief the total number of files to be written out
				size_t NumberOfFiles;
			};

			/// @docinternal
			/// @brief Platform specific helper to compute the required disk space for a given archive contents
			/// @param Begin the start of the archive contents
			/// @param End the end of the archive contents
			/// @return the storage requirements for the archive contents
			static inline ArchiveDiskRequirements ComputeDiskRequirementsForArchive(
				std::vector<Modio::Detail::ArchiveFileImplementation::ArchiveEntry>::iterator Begin,
				std::vector<Modio::Detail::ArchiveFileImplementation::ArchiveEntry>::iterator End)
			{
				size_t TotalSize = 0;
				size_t TotalFileCount = 0;
				while (Begin != End)
				{
					// Files on cache storage consume a minimum of 16Kb, and will always be aligned
					// to 16Kb in sizes when written to the disk.
					size_t ThisFileSize = Begin->UncompressedSize;
					TotalSize += ThisFileSize;
					TotalFileCount++;

					++Begin;
				}
				return ArchiveDiskRequirements {TotalSize, TotalFileCount};
			}

			Modio::filesystem::path OutputPath {};
			Modio::Optional<Modio::ModID> ModId {};
			ArchiveDiskRequirements Requirements {};
			ModioAsio::coroutine CoroutineState {};

		public:
			ValidateDiskSpaceForArchiveOp(Modio::filesystem::path& OutputPath,
										  std::vector<ArchiveFileImplementation::ArchiveEntry>::iterator ArchiveBegin,
										  std::vector<ArchiveFileImplementation::ArchiveEntry>::iterator ArchiveEnd,
										  Modio::Optional<Modio::ModID> ModId)
				: OutputPath(OutputPath),
				  ModId(ModId),
				  Requirements(ComputeDiskRequirementsForArchive(ArchiveBegin, ArchiveEnd)),
				  CoroutineState()
			{}

			ValidateDiskSpaceForArchiveOp(Modio::filesystem::path& OutputPath, size_t DiskSize, size_t NumberOfFiles,
										  Modio::Optional<Modio::ModID> ModId)
				: OutputPath(OutputPath),
				  ModId(ModId),
				  Requirements({DiskSize, NumberOfFiles}),
				  CoroutineState()
			{}

			template<typename CoroType>
			void operator()(CoroType& Self, Modio::ErrorCode ec = {})
			{
				MODIO_PROFILE_SCOPE(ValidateDiskSpaceForArchiveOp);

				Modio::Detail::Logger().Log(
					LogLevel::Trace, LogCategory::File,
					"Validating disk space for expanded archive of {} files, and {} bytes on disk, at path {}",
					Requirements.NumberOfFiles, Requirements.DiskSize, OutputPath.u8string());

				Modio::Detail::FileService& FService =
					Modio::Detail::Services::GetGlobalService<Modio::Detail::FileService>();

				reenter(CoroutineState)
				{
					if (FService.CheckSpaceAvailable(OutputPath, Modio::FileSize(Requirements.DiskSize)))
					{
						Self.complete({});
						return;
					}

					Modio::Detail::Logger().Log(LogLevel::Error, LogCategory::File,
												"Do not have disk space to store the expanded arhive {}, with error {}",
												OutputPath.u8string(), ec.message());
					Self.complete(Modio::make_error_code(Modio::FilesystemError::InsufficientSpace));
					return;
				}
			}
		};

		template<typename CompletionTokenType>
		auto ValidateDiskSpaceForArchiveAsync(Modio::filesystem::path& OutputPath,
											  std::vector<ArchiveFileImplementation::ArchiveEntry>::iterator Begin,
											  std::vector<ArchiveFileImplementation::ArchiveEntry>::iterator End,
											  Modio::Optional<Modio::ModID> ModId, CompletionTokenType&& Token)
		{
			return ModioAsio::async_compose<CompletionTokenType, void(Modio::ErrorCode)>(
				ValidateDiskSpaceForArchiveOp(OutputPath, Begin, End, ModId), Token,
				Modio::Detail::Services::GetGlobalContext().get_executor());
		}

		template<typename CompletionTokenType>
		auto ValidateDiskSpaceForArchiveAsync(Modio::filesystem::path& OutputPath, size_t DiskSize,
											  size_t NumberOfFiles, Modio::Optional<Modio::ModID> ModId,
											  CompletionTokenType&& Token)
		{
			return ModioAsio::async_compose<CompletionTokenType, void(Modio::ErrorCode)>(
				ValidateDiskSpaceForArchiveOp(OutputPath, DiskSize, NumberOfFiles, ModId), Token,
				Modio::Detail::Services::GetGlobalContext().get_executor());
		}

		static inline Modio::ErrorCode NotifyModUninstall(Modio::filesystem::path /*InstallPath*/,
														  Modio::ModID /*ModId*/, bool /*bIsTempMod*/)
		{
			return {};
		}
#include <asio/unyield.hpp>

	} // namespace Detail
} // namespace Modio
