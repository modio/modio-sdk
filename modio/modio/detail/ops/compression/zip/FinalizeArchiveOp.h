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

#include "modio/detail/AsioWrapper.h"
#include "modio/detail/FilesystemWrapper.h"
#include "modio/detail/compression/zip/ArchiveFileImplementation.h"
#include "modio/file/ModioFile.h"
#include <memory>

namespace Modio
{
	namespace Detail
	{
#include <asio/yield.hpp>
		class FinalizeArchiveOp
		{
		public:
			FinalizeArchiveOp(std::shared_ptr<Modio::Detail::ArchiveFileImplementation> ArchiveFile)
				: ArchiveFile(ArchiveFile)
			{
				OutputFile = std::make_unique<Modio::Detail::File>(ArchiveFile->FilePath, false);
			};

			template<typename CoroType>
			void operator()(CoroType& Self, Modio::ErrorCode ec = {})
			{
				reenter(CoroutineState)
				{
					OutputFile->Seek(Modio::FileOffset(OutputFile->GetFileSize()));
					StartOfCentralDirectory = OutputFile->Tell();
					// The iterator will be safe to dereference across a `yield` because it's pointing to a vector
					// stored in the shared_ptr, ie moving this operation won't change the location in memory of the
					// object in ArchiveFile
					CurrentArchiveEntry = ArchiveFile->begin();
					while (CurrentArchiveEntry != ArchiveFile->end())
					{
						// Calculate the size of the record buffer;
						RecordBuffer = std::make_unique<Modio::Detail::Buffer>(
							46 + CurrentArchiveEntry->FilePath.generic_u8string().size());

						Modio::Detail::TypedBufferWrite(CentralDirectoryMagic, *RecordBuffer, 0)
							.FollowedBy<std::uint16_t>(20) // Zip version used for creating the archive
							.FollowedBy<std::uint16_t>(20) // Minimum zip version required for extraction
							.FollowedBy<std::uint16_t>(0) // General purpose bit flag
							.FollowedBy<std::uint16_t>(static_cast<uint16_t>(CurrentArchiveEntry->Compression))
							.FollowedBy<std::uint16_t>(0) // Last modified time
							.FollowedBy<std::uint16_t>(0) // Last modified date
							.FollowedBy<std::uint32_t>(CurrentArchiveEntry->CRCValue)
							.FollowedBy<std::uint32_t>(CurrentArchiveEntry->CompressedSize)
							.FollowedBy<std::uint32_t>(CurrentArchiveEntry->UncompressedSize)
							.FollowedBy<std::uint16_t>(
								(std::uint16_t) CurrentArchiveEntry->FilePath.generic_u8string().size())
							.FollowedBy<std::uint16_t>(0) // Extra field length
							.FollowedBy<std::uint16_t>(0) // File comment length
							.FollowedBy<std::uint16_t>(0) // Disk number (will always be a single disk)
							.FollowedBy<std::uint16_t>(0) // Internal file attributes
							.FollowedBy<std::uint32_t>(CurrentArchiveEntry->bIsDirectory ? 0x10 : 0) // External file attributes
							.FollowedBy<std::uint32_t>((std::uint32_t) CurrentArchiveEntry->FileOffset - 30 -
													   (std::uint32_t) CurrentArchiveEntry->FilePath.generic_u8string()
														   .size()); // Offset of local file header from start of disk
																	 // (only one disk, so offset from start of file)
						{
							std::string FileName = CurrentArchiveEntry->FilePath.generic_u8string();
							std::copy(FileName.begin(), FileName.end(), RecordBuffer->Data() + 46);
						}

						yield OutputFile->WriteAsync(std::move(*RecordBuffer), std::move(Self));
						if (ec)
						{
							Self.complete(ec);
							return;
						}
						CurrentArchiveEntry++;
					}

					RecordBuffer = std::make_unique<Modio::Detail::Buffer>(22);
					Modio::Detail::TypedBufferWrite(EndOfCentralDirectoryMagic, *RecordBuffer, 0)
						.FollowedBy<std::uint16_t>(0) // Number of this disk
						.FollowedBy<std::uint16_t>(0) // Disk where central directory starts
						.FollowedBy<std::uint16_t>(
							ArchiveFile
								->GetNumberOfEntries()) // Number of entries in the central directory on current disk
						.FollowedBy<std::uint16_t>(
							ArchiveFile->GetNumberOfEntries()) // Number of entries in the central directory
						.FollowedBy<std::uint32_t>(OutputFile->Tell() -
												   StartOfCentralDirectory) // Size of central directory
						.FollowedBy<std::uint32_t>(StartOfCentralDirectory) // Position of central directory in file
						.FollowedBy<std::uint32_t>(0); // Length of comment

					yield OutputFile->WriteAsync(std::move(*RecordBuffer), std::move(Self));
					Self.complete(ec);
					return;
				}
			}

		private:
			std::shared_ptr<Modio::Detail::ArchiveFileImplementation> ArchiveFile;
			std::unique_ptr<Modio::Detail::File> OutputFile;
			asio::coroutine CoroutineState;
			std::unique_ptr<Modio::Detail::Buffer> RecordBuffer;
			std::vector<Modio::Detail::ArchiveFileImplementation::ArchiveEntry>::const_iterator CurrentArchiveEntry;
            // These variable had troubles as a constexpr when compiling on macOS + g++11
			const std::uint32_t CentralDirectoryMagic = 0x02014b50;
			const std::uint32_t EndOfCentralDirectoryMagic = 0x06054b50;
			Modio::FileOffset StartOfCentralDirectory;
		};
#include <asio/unyield.hpp>
	} // namespace Detail
} // namespace Modio
