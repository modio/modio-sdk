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
#include "modio/detail/compression/zip/ZipStructures.h"
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
                uint64_t LocalFileOffset = 0;
                
                reenter(CoroutineState)
				{
                    // For reference: https://pkware.cachefly.net/webdocs/casestudies/APPNOTE.TXT
					OutputFile->Seek(Modio::FileOffset(OutputFile->GetFileSize()));
					StartOfCentralDirectory = OutputFile->Tell();
					// The iterator will be safe to dereference across a `yield` because it's pointing to a vector
					// stored in the shared_ptr, ie moving this operation won't change the location in memory of the
					// object in ArchiveFile
					CurrentArchiveEntry = ArchiveFile->begin();
					while (CurrentArchiveEntry != ArchiveFile->end())
					{
						{
                            uint64_t HeaderSize = 46 + CurrentArchiveEntry->FilePath.generic_u8string().size() + Constants::ZipTag::TotalExtraFieldSizeFileEntries;
                            LocalFileOffset = CurrentArchiveEntry->FileOffset;
                            // Calculate the size of the record buffer;
                            RecordBuffer = std::make_unique<Modio::Detail::Buffer>(HeaderSize);
                            
                            bool ExtraFieldMax = (CurrentArchiveEntry->CompressedSize >= Constants::ZipTag::MAX32) ||
                                                 (CurrentArchiveEntry->UncompressedSize >= Constants::ZipTag::MAX32) ||
                                                 (LocalFileOffset >= Constants::ZipTag::MAX32);
                            
                            Modio::Detail::TypedBufferWrite(Constants::ZipTag::CentralDirectoryFileHeader, *RecordBuffer, 0)
                                .FollowedBy<std::uint16_t>(Constants::ZipTag::ZipVersion) // Zip version used for creating the archive
                                .FollowedBy<std::uint16_t>(ArchiveFile->bIsZip64 ? Constants::ZipTag::ZipVersion : Constants::ZipTag::Zip64Version) // Minimum zip version required for extraction
                                .FollowedBy<std::uint16_t>(0) // General purpose bit flag
                                .FollowedBy<std::uint16_t>(static_cast<uint16_t>(CurrentArchiveEntry->Compression))
                                .FollowedBy<std::uint16_t>(0) // Last modified time
                                .FollowedBy<std::uint16_t>(0) // Last modified date
                                .FollowedBy<std::uint32_t>(CurrentArchiveEntry->CRCValue)
                                .FollowedBy<std::uint32_t>(
                                    ExtraFieldMax ? Constants::ZipTag::MAX32 : CurrentArchiveEntry->CompressedSize)
                                .FollowedBy<std::uint32_t>(
                                    ExtraFieldMax ? Constants::ZipTag::MAX32 : CurrentArchiveEntry->UncompressedSize)
                                .FollowedBy<std::uint16_t>(
                                    (std::uint16_t) CurrentArchiveEntry->FilePath.generic_u8string().size())
                                .FollowedBy<std::uint16_t>(Constants::ZipTag::TotalExtraFieldSizeFileEntries) // Extra field length
                                .FollowedBy<std::uint16_t>(0) // File comment length
                                .FollowedBy<std::uint16_t>(0) // Disk number (will always be a single disk)
                                .FollowedBy<std::uint16_t>(0) // Internal file attributes
                                .FollowedBy<std::uint32_t>(CurrentArchiveEntry->bIsDirectory ? 0x10 : 0) // External file attributes
                                .FollowedBy<std::uint32_t>(
                                // Offset of local file header from start of disk (only one disk, so offset from start of file)
                                    ExtraFieldMax ? Constants::ZipTag::MAX32 : LocalFileOffset);
                        }
                        
						{
							std::string FileName = CurrentArchiveEntry->FilePath.generic_u8string();
							std::copy(FileName.begin(), FileName.end(), RecordBuffer->Data() + 46);
						}
                        
                        // Add "Extra Field" information regardless of Zip64 status
                        {
                            Modio::Detail::TypedBufferWrite(Constants::ZipTag::ExtendedInformationFieldHeaderID, *RecordBuffer, 46 + CurrentArchiveEntry->FilePath.generic_u8string().size())
                                .FollowedBy<uint16_t>(Constants::ZipTag::ExtraFieldSize)
                                .FollowedBy<uint64_t>(CurrentArchiveEntry->UncompressedSize) // Original uncompressed file size
                                .FollowedBy<uint64_t>(CurrentArchiveEntry->CompressedSize) // Size of compressed data
                                .FollowedBy<uint64_t>(LocalFileOffset); // Offset of local header record
                        }

						yield OutputFile->WriteAsync(std::move(*RecordBuffer), std::move(Self));
						if (ec)
						{
							Self.complete(ec);
							return;
						}
						CurrentArchiveEntry++;
					}
                    
                    if (ArchiveFile->bIsZip64 == true)
                    {
                        // When it is Zip64, the ECD in Zip32 needs to report the Zip64 size
                        SizeCentralDir = OutputFile->Tell() - StartOfCentralDirectory;
                        // Zip64 has to write first the End of Central Directory of the 64 bit versions
                        // plus the End of central directory locator
                        RecordBuffer = std::make_unique<Modio::Detail::Buffer>(Constants::ZipTag::EndCentralDirectoryHeaderSize64 + Constants::ZipTag::EndCentralDirectoryLocatorSize64);
                        Modio::Detail::TypedBufferWrite(Constants::ZipTag::EndCentralDirectory64, *RecordBuffer, 0)
                            .FollowedBy<std::uint64_t>(44) // Size of the EOCD64 after this size record
                            .FollowedBy<std::uint16_t>(0) // Version made by
                            .FollowedBy<std::uint16_t>(Constants::ZipTag::Zip64Version) // Version needed to extract
                            .FollowedBy<std::uint32_t>(0) // Number of this disk
                            .FollowedBy<std::uint32_t>(0) // Disk where central directory starts
                            // Number of central directory records on this disk
                            .FollowedBy<std::uint64_t>(ArchiveFile->GetNumberOfEntries())
                            // Total number of central directory records
                            .FollowedBy<std::uint64_t>(ArchiveFile->GetNumberOfEntries())
                            // Size of central directory (bytes)
                            .FollowedBy<std::uint64_t>(SizeCentralDir)
                            // Offset of start of central directory, relative to start of archive
                            .FollowedBy<std::uint64_t>(StartOfCentralDirectory)
                            // Zip64 end of central dir locator signature "0x07064b50"
                            .FollowedBy<std::uint32_t>(Constants::ZipTag::EndCentralDirectoryLocator64)
                            // number of the disk with the start of the zip64 end of central directory
                            .FollowedBy<std::uint32_t>(0)
                            // Relative offset of the zip64 end of central directory record
                            .FollowedBy<std::uint64_t>(OutputFile->Tell())
                            // total number of disks
                            .FollowedBy<std::uint32_t>(1);
                        
                        // Write the ECD64 to file
                        yield OutputFile->WriteAsync(std::move(*RecordBuffer), std::move(Self));
                    }
                    
                    RecordBuffer = std::make_unique<Modio::Detail::Buffer>(22);
                    Modio::Detail::TypedBufferWrite(Constants::ZipTag::EndCentralDirectory, *RecordBuffer, 0)
                        .FollowedBy<std::uint16_t>(0) // Number of this disk
                        .FollowedBy<std::uint16_t>(0) // Disk where central directory starts
                        .FollowedBy<std::uint16_t>(ArchiveFile->GetNumberOfEntries()) // Number of entries in the central directory on current disk
                        .FollowedBy<std::uint16_t>(ArchiveFile->GetNumberOfEntries()) // Number of entries in the central directory
                        .FollowedBy<std::uint32_t>(ArchiveFile->bIsZip64 == true ? SizeCentralDir :
                            OutputFile->Tell() - StartOfCentralDirectory) // Size of central directory
                        // For some reason, the macOS decompressor does not like tp have #Entries and Size of CD to "0xFF" when not necessary
                        .FollowedBy<std::uint32_t>(ArchiveFile->bIsZip64 == true ? Constants::ZipTag::MAX32 :
                            StartOfCentralDirectory) // Position of central directory in file
                        .FollowedBy<std::uint16_t>(0); // Length of comment
                    
                    
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
            Modio::FileOffset StartOfCentralDirectory;
            std::uint64_t SizeCentralDir = 0;
		};
#include <asio/unyield.hpp>
	} // namespace Detail
} // namespace Modio
