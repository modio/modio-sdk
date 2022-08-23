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

#include "modio/core/ModioStdTypes.h"
#include "modio/detail/ModioProfiling.h"
#include "modio/detail/compression/zip/ArchiveFileImplementation.h"
#include "modio/detail/compression/zip/ZipStructures.h"
#include "modio/file/ModioFile.h"

#include <asio/yield.hpp>
namespace Modio
{
	namespace Detail
	{
		class ParseArchiveContentsOp
		{
			Modio::StableStorage<Modio::Detail::File> ArchiveFileOnDisk;
			Modio::StableStorage<ArchiveFileImplementation> ArchiveState;
			asio::coroutine CoroutineState;
			std::uintmax_t CurrentSearchOffset = 0;
			std::uintmax_t CurrentRecordOffset = 0;
			std::vector<ArchiveFileImplementation::ArchiveEntry> PendingEntries;
			std::uintmax_t CurrentFixupEntryIndex = 0;
            std::uintmax_t BytesToRead = 20;
            
		public:
			ParseArchiveContentsOp(std::shared_ptr<ArchiveFileImplementation> ArchiveState)
				: ArchiveState(ArchiveState) {};

			template<typename CoroType>
			void operator()(CoroType& Self, Modio::ErrorCode ec = {},
							Modio::Optional<Modio::Detail::Buffer> FileChunk = {})
			{
				constexpr std::uint64_t ChunkOfBytes = 64 * 1024;
				std::uint64_t FileSize = 0;
				std::uint64_t MaxBytesToRead = 0;
				MODIO_PROFILE_SCOPE(ParseArchiveContents);
				reenter(CoroutineState)
				{	
					{
						ArchiveFileOnDisk = std::make_shared<Modio::Detail::File>(ArchiveState->FilePath, false);
						FileSize = ArchiveFileOnDisk->GetFileSize();
						CurrentSearchOffset = FileSize - std::min(FileSize, ChunkOfBytes);

						// If the FileSize is larger than 4,294,967,295 bytes (2^32−1 bytes, or 4 GB minus 1 byte)
						// it is automatically a Zip64 file
						if (FileSize >= (UINT32_MAX - 1))
						{
                            BytesToRead = 48;
							ArchiveState->bIsZip64 = true;
						}
					}

					while (ArchiveState->ZipMagicOffset == 0)
					{
						MaxBytesToRead = (FileSize < ChunkOfBytes ? FileSize : ChunkOfBytes);
						yield ArchiveFileOnDisk->ReadSomeAtAsync(CurrentSearchOffset, MaxBytesToRead, std::move(Self));
						if (ec && ec != Modio::GenericError::EndOfFile)
						{
							Self.complete(ec);
							return;
						}
						else
						{
							if (FileChunk.has_value())
							{
								Modio::Detail::Buffer& Chunk = FileChunk.value();
								if (Chunk.GetSize() >= 4)
								{
                                    // Use a helper function to search the "Zip Magic Offset" in a buffer
                                    std::tie(ArchiveState->ZipMagicOffset, ArchiveState->bIsZip64) = ZipStructures::FindOffsetInBuffer(Chunk, ArchiveState->bIsZip64);
                                    // Add what has been Currently Searched
                                    ArchiveState->ZipMagicOffset += CurrentSearchOffset;
								}
								else
								{
									Modio::Detail::Logger().Log(Modio::LogLevel::Info, Modio::LogCategory::File,
																"Read less than 4 bytes from archive file.");
								}
							}
							else
							{
								Self.complete(Modio::make_error_code(Modio::FilesystemError::ReadError));
								return;
							}
						}

						// If the ec is "EndOfFile", the line below should reset its state in case it is used later on.
						ec = {};

						// If we haven't found the signature and we're at the end of the file, bail
						if (ArchiveState->ZipMagicOffset == 0 && CurrentSearchOffset == 0)
						{
							Modio::Detail::Logger().Log(Modio::LogLevel::Error, Modio::LogCategory::Compression,
														"file had no central directory magic");

							Self.complete(Modio::make_error_code(Modio::ArchiveError::InvalidHeader));
							return;
						}
						
						if (CurrentSearchOffset >= ChunkOfBytes)
						{
							CurrentSearchOffset -= ChunkOfBytes;
						}
						else
						{
							CurrentSearchOffset = 0;
						}
					}
                    
                    // If Zip64, the End Central Directory (ECD) contains 56 bytes. Zip is only 20
                    BytesToRead = ArchiveState->bIsZip64 == true ? 56 : 20;
                    
                    // Now we've located the central directory metadata, do a direct read on that region (in case a
                    // chunk boundary from the earlier search splits it)
                    yield ArchiveFileOnDisk->ReadSomeAtAsync(ArchiveState->ZipMagicOffset, BytesToRead, std::move(Self));
                    
                    // If we got EOF whilst trying to read the 20 bytes containing the directory information, inform the
                    // consumer
                    if (ec == Modio::GenericError::EndOfFile)
					{
						Modio::Detail::Logger().Log(Modio::LogLevel::Error, Modio::LogCategory::Compression,
													"Truncated central directory metadata");

						Self.complete(Modio::make_error_code(Modio::ArchiveError::InvalidHeader));
						return;
					}

					if (FileChunk.has_value())
					{
                        std::tie(ArchiveState->NumberOfRecords,
                                 ArchiveState->CentralDirectorySize,
                                 ArchiveState->CentralDirectoryOffset) =
                            ZipStructures::ReadCentralDirectory(FileChunk.value(), ArchiveState->bIsZip64);
                        
						yield ArchiveFileOnDisk->ReadSomeAtAsync(ArchiveState->CentralDirectoryOffset,
																 ArchiveState->CentralDirectorySize, std::move(Self));
                        
						if (FileChunk.has_value() && FileChunk->GetSize() != ArchiveState->CentralDirectorySize)
						{
							// Could not read full central directory
							Modio::Detail::Logger().Log(Modio::LogLevel::Error, Modio::LogCategory::Compression,
														"Could not read full central directory");

							Self.complete(Modio::make_error_code(Modio::ArchiveError::InvalidHeader));
							return;
						}

						CurrentRecordOffset = 0;

						for (std::uint64_t RecordIndex = 0; RecordIndex < ArchiveState->NumberOfRecords; RecordIndex++)
						{
                            ArchiveFileImplementation::ArchiveEntry Entry;
                            Modio::ErrorCode Err;
                            std::tie(Entry, CurrentRecordOffset, Err) = ZipStructures::ArchiveParse(FileChunk.value(), CurrentRecordOffset);
                            
                            if (Err)
                            {
                                Self.complete(Err);
                                return;
                            }
                            
                            ArchiveState->TotalExtractedSize += Modio::FileSize(Entry.UncompressedSize);
                            PendingEntries.push_back(Entry);
						}
                        
						// Now we have the information about the local headers for all the files, we need to read those
						// parts of the zip locate the actual file offsets, specially because ExtraFieldLenght might not
                        // be the same between the Central Directory and the Local Directory .-_-.
						for (CurrentFixupEntryIndex = 0; CurrentFixupEntryIndex < PendingEntries.size();
							 CurrentFixupEntryIndex++)
						{
							yield ArchiveFileOnDisk->ReadSomeAtAsync(PendingEntries[CurrentFixupEntryIndex].FileOffset,
																	 30, std::move(Self));

							if (ec && ec != Modio::GenericError::EndOfFile)
							{
								Self.complete(ec);
								return;
							}

							std::uint16_t LocalFileLength =
								Modio::Detail::TypedBufferRead<std::uint16_t>(FileChunk.value(), 26);
							std::uint16_t LocalExtraFieldLength =
								Modio::Detail::TypedBufferRead<std::uint16_t>(FileChunk.value(), 28);
							PendingEntries[CurrentFixupEntryIndex].FileOffset +=
								30 + LocalFileLength + LocalExtraFieldLength;
							ArchiveState->AddEntry(PendingEntries[CurrentFixupEntryIndex]);
						}
                        
						ArchiveFileOnDisk.reset();
						Self.complete(Modio::ErrorCode {});
					}
					else
					{
						Self.complete(Modio::make_error_code(Modio::FilesystemError::ReadError));
						return;
					}
				}
			}
		};

	} // namespace Detail
} // namespace Modio
#include <asio/unyield.hpp>
