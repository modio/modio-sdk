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
#include "modio/detail/AsioWrapper.h"
#include "modio/detail/FilesystemWrapper.h"
#include "modio/detail/compression/zip/ArchiveFileImplementation.h"
#include "modio/detail/compression/zip/ZipStructures.h"
#include "modio/file/ModioFile.h"
#include <memory>

// For reference: https://pkware.cachefly.net/webdocs/casestudies/APPNOTE.TXT

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
				OutputFile = std::make_unique<Modio::Detail::File>(ArchiveFile->FilePath,
																   Modio::Detail::FileMode::ReadWrite, false);
			}

			template<typename CoroType>
			void operator()(CoroType& Self, Modio::ErrorCode ec = {})
			{
				reenter(CoroutineState)
				{
					// Determine the start of the central directory
					OutputFile->Seek(Modio::FileOffset(OutputFile->GetFileSize()));
					StartOfCentralDirectory = OutputFile->Tell();

					// If the central directory is outside of "4294967295" bytes, tag the archive as Zip64
					if (StartOfCentralDirectory >= Constants::ZipTag::MAX32)
					{
						ArchiveFile->bIsZip64 = true;
					}
					// If the total number of files is greater than "65535", tag the archive as Zip64
					if (ArchiveFile->GetNumberOfEntries() >= Constants::ZipTag::MAX16)
					{
						ArchiveFile->bIsZip64 = true;
					}

					// The iterator will be safe to dereference across a `yield` because it's pointing to a vector
					// stored in the shared_ptr, i.e. moving this operation won't change the location in memory of the
					// object in ArchiveFile
					CurrentArchiveEntry = ArchiveFile->begin();
					while (CurrentArchiveEntry != ArchiveFile->end())
					{
						{
							// Offset from start of archive file to this file's local directory header
							uint64_t LocalFileOffset = CurrentArchiveEntry->FileOffset;

							// Check if Zip64 extra field information is required for the size and/or offset fields
							bool bIncludeExtraFieldSizes =
								(CurrentArchiveEntry->CompressedSize >= Constants::ZipTag::MAX32) ||
								(CurrentArchiveEntry->UncompressedSize >= Constants::ZipTag::MAX32);
							bool bIncludeExtraFieldOffset = LocalFileOffset >= Constants::ZipTag::MAX32;

							// If we require any extra fields, the first 4 bytes (signature + size) are always
							// required
							uint16_t ExtraFieldLength = (bIncludeExtraFieldSizes || bIncludeExtraFieldOffset) ? 4 : 0;
							if (bIncludeExtraFieldSizes)
							{
								// Include 8 bytes each for compressed and uncompressed data size fields
								ExtraFieldLength += 16;
							}
							if (bIncludeExtraFieldOffset)
							{
								// Include 8 bytes for offset of local header record field
								ExtraFieldLength += 8;
							}

							std::string FileName = CurrentArchiveEntry->FilePath.generic_u8string();

							SizeOfCentralDirectory +=
								(Constants::ZipTag::CentralFileHeaderFixedSize + FileName.size() + ExtraFieldLength);

							// Marshal central directory header fields into a buffer
							RecordBuffer = std::make_unique<Modio::Detail::Buffer>(
								Constants::ZipTag::CentralFileHeaderFixedSize + FileName.size() + ExtraFieldLength);
							// Header signature
							Modio::Detail::TypedBufferWrite(Constants::ZipTag::CentralFileHeaderSignature,
															*RecordBuffer, 0)
								// Zip version used for creating the archive
								.FollowedBy<std::uint16_t>(Constants::ZipTag::ZipVersion)
								// Minimum zip version required for extraction
								.FollowedBy<std::uint16_t>((ArchiveFile->bIsZip64 || ExtraFieldLength > 0)
															   ? Constants::ZipTag::Zip64Version
															   : Constants::ZipTag::ZipVersion)
								// General purpose bit-flag
								.FollowedBy<std::uint16_t>(0)
								// Compression method
								.FollowedBy<std::uint16_t>(static_cast<uint16_t>(CurrentArchiveEntry->Compression))
								// Last modified time
								.FollowedBy<std::uint16_t>(0)
								// Last modified date
								.FollowedBy<std::uint16_t>(0)
								// CRC-32
								.FollowedBy<std::uint32_t>(CurrentArchiveEntry->CRCValue)
								// Compressed size. Must be set to MAX32 if actual value is included in Zip64
								// Extended Information
								.FollowedBy<std::uint32_t>(
									bIncludeExtraFieldSizes
										? Constants::ZipTag::MAX32
										: static_cast<std::uint32_t>(CurrentArchiveEntry->CompressedSize))
								// Uncompressed size. Must be set to MAX32 if actual value is included in Zip64
								// Extended Information
								.FollowedBy<std::uint32_t>(
									bIncludeExtraFieldSizes
										? Constants::ZipTag::MAX32
										: static_cast<std::uint32_t>(CurrentArchiveEntry->UncompressedSize))
								// File name length
								.FollowedBy<std::uint16_t>(std::uint16_t(FileName.size()))
								// Extra field length
								.FollowedBy<std::uint16_t>(ExtraFieldLength)
								// File comment length
								.FollowedBy<std::uint16_t>(0)
								// Disk number (will always be a single disk)
								.FollowedBy<std::uint16_t>(0)
								// Internal file attributes
								.FollowedBy<std::uint16_t>(0)
								// External file attributes
								.FollowedBy<std::uint32_t>(CurrentArchiveEntry->bIsDirectory ? 0x10 : 0)
								// Offset of local file header from start of disk (only one disk, so start of file)
								// Must be set to MAX32 if actual value is included in Zip64 Extended Information
								.FollowedBy<std::uint32_t>(bIncludeExtraFieldOffset
															   ? Constants::ZipTag::MAX32
															   : static_cast<std::uint32_t>(LocalFileOffset));

							// File name
							std::copy(FileName.begin(), FileName.end(),
									  RecordBuffer->Data() + Constants::ZipTag::CentralFileHeaderFixedSize);

							// Zip64 Extended Information Extra Field
							if (ExtraFieldLength > 0)
							{
								// Make sure the archive is marked as Zip64 for the End of Central Directory Record
								ArchiveFile->bIsZip64 = true;

								Modio::Detail::Logger().Log(Modio::LogLevel::Trace, Modio::LogCategory::File,
															"Including Zip64 Extended Information Extra Field in the "
															"Central Directory for file {}",
															FileName);

								// Zip64 Extra Field Signature
								Modio::Detail::TypedBufferWrite(
									Constants::ZipTag::Zip64ExtraFieldSignature, *RecordBuffer,
									Constants::ZipTag::CentralFileHeaderFixedSize + FileName.size())
									// Size of this extra block, excluding leading 4 bytes (signature and size
									// fields)
									.FollowedBy<uint16_t>(ExtraFieldLength - 4)
									// Uncompressed size
									.ConditionalFollowedBy<uint64_t>(CurrentArchiveEntry->UncompressedSize,
																	 bIncludeExtraFieldSizes)
									// Compressed size
									.ConditionalFollowedBy<uint64_t>(CurrentArchiveEntry->CompressedSize,
																	 bIncludeExtraFieldSizes)
									// Relative Header Offset
									.ConditionalFollowedBy<uint64_t>(LocalFileOffset, bIncludeExtraFieldOffset);
							}
						}

						// Write out the central directory header for this entry
						yield OutputFile->WriteAsync(std::move(*RecordBuffer), std::move(Self));
						if (ec)
						{
							Self.complete(ec);
							return;
						}
						CurrentArchiveEntry++;
					}

					// Zip64 End of Central Directory Record and Locator
					if (ArchiveFile->bIsZip64)
					{
						// Marshal required fields into buffer
						// We omit the only variable sized field (zip64 extensible data sector) so can use a known fixed
						// buffer size
						RecordBuffer = std::make_unique<Modio::Detail::Buffer>(
							Constants::ZipTag::Zip64EndCentralDirectoryHeaderFixedSize +
							Constants::ZipTag::Zip64EndCentralDirectoryLocatorSize);
						// Zip64 EOCD signature
						Modio::Detail::TypedBufferWrite(Constants::ZipTag::Zip64EndCentralDirectorySignature,
														*RecordBuffer, 0)
							// ZIP64 EOCD size, exclude leading 12 bytes (signature + size)
							.FollowedBy<std::uint64_t>(Constants::ZipTag::Zip64EndCentralDirectoryHeaderFixedSize - 12)
							// Version made by
							.FollowedBy<std::uint16_t>(Constants::ZipTag::ZipVersion)
							// Version needed to extract
							.FollowedBy<std::uint16_t>(Constants::ZipTag::Zip64Version)
							// this disk number
							.FollowedBy<std::uint32_t>(0)
							// disk number with EOCD start
							.FollowedBy<std::uint32_t>(0)
							// central directory entries (this disk)
							.FollowedBy<std::uint64_t>(ArchiveFile->GetNumberOfEntries())
							// central directory entries (total)
							.FollowedBy<std::uint64_t>(ArchiveFile->GetNumberOfEntries())
							// central directory size
							.FollowedBy<std::uint64_t>(SizeOfCentralDirectory)
							// central directory offset with respect to the starting disk (only one disk, so from offset
							// from start of file)
							.FollowedBy<std::uint64_t>(StartOfCentralDirectory)
							// zip64 EOCD locator signature
							.FollowedBy<std::uint32_t>(Constants::ZipTag::Zip64EndCentralDirectoryLocatorSignature)
							// disk number with Zip64 EOCD start
							.FollowedBy<std::uint32_t>(0)
							// relative offset of zip64 EOCD record
							.FollowedBy<std::uint64_t>(OutputFile->Tell())
							// total number of disks
							.FollowedBy<std::uint32_t>(1);

						// Write the Zip64 End of Central Directory Record and Locator to the archive file
						yield OutputFile->WriteAsync(std::move(*RecordBuffer), std::move(Self));
					}

					// End of Central Directory Record (EOCD)

					// Marshal required fields into buffer
					// We omit the only variable sized field (.ZIP file comment) so can use a known fixed buffer size
					RecordBuffer =
						std::make_unique<Modio::Detail::Buffer>(Constants::ZipTag::EndOfCentralDirectoryFixedSize);

					// EOCD signature
					Modio::Detail::TypedBufferWrite(Constants::ZipTag::EndCentralDirectorySignature, *RecordBuffer, 0)
						// this disk number
						.FollowedBy<std::uint16_t>(0)
						// disk number with EOCD start
						.FollowedBy<std::uint16_t>(0)
						// central directory entries (this disk).
						// Must be set to MAX16 if actual value is included in Zip64 EOCD Record
						.FollowedBy<std::uint16_t>(ArchiveFile->bIsZip64 ? Constants::ZipTag::MAX16
																		 : ArchiveFile->GetNumberOfEntries())
						// central directory entries (total).
						// Must be set to MAX16 if actual value is included in Zip64 EOCD Record
						.FollowedBy<std::uint16_t>(ArchiveFile->bIsZip64 ? Constants::ZipTag::MAX16
																		 : ArchiveFile->GetNumberOfEntries())
						// central directory size
						// Must be set to MAX32 if actual value is included in Zip64 EOCD Record
						.FollowedBy<std::uint32_t>(ArchiveFile->bIsZip64
													   ? Constants::ZipTag::MAX32
													   : static_cast<std::uint32_t>(SizeOfCentralDirectory))
						// central directory offset with respect to the starting disk (only one disk, so from offset
						// from start of file). Must be set to MAX32 if actual value is included in Zip64 EOCD Record
						.FollowedBy<std::uint32_t>(ArchiveFile->bIsZip64
													   ? Constants::ZipTag::MAX32
													   : static_cast<std::uint32_t>(StartOfCentralDirectory))
						// .ZIP file comment length
						.FollowedBy<std::uint16_t>(0);

					// Write the End of Central Directory Record to archive file
					yield OutputFile->WriteAsync(std::move(*RecordBuffer), std::move(Self));

					// Close file handle
					OutputFile.reset();

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
			Modio::FileOffset StartOfCentralDirectory;
			std::uint64_t SizeOfCentralDirectory = 0;
		};
#include <asio/unyield.hpp>
	} // namespace Detail
} // namespace Modio
