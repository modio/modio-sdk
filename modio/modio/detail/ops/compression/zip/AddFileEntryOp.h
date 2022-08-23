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

#include "modio/core/ModioBuffer.h"
#include "modio/core/ModioCoreTypes.h"
#include "modio/core/ModioErrorCode.h"
#include "modio/core/ModioModCollectionEntry.h"
#include "modio/detail/AsioWrapper.h"
#include "modio/detail/FilesystemWrapper.h"
#include "modio/detail/ModioCRC.h"
#include "modio/detail/compression/zip/ArchiveFileImplementation.h"
#include "modio/detail/compression/zip/ZipStructures.h"
#include "modio/detail/compression/zlib/deflate_stream.hpp"
#include "modio/file/ModioFile.h"
#include <algorithm>
#include <memory>

namespace Modio
{
	namespace Detail
	{
#include <asio/yield.hpp>
		class AddFileEntryOp
		{
		public:
			AddFileEntryOp(std::shared_ptr<Modio::Detail::ArchiveFileImplementation> ArchiveFile,
						   Modio::filesystem::path SourceFilePath, Modio::filesystem::path PathInsideArchive,
						   std::weak_ptr<Modio::ModProgressInfo> ProgressInfo)
				: ArchiveFile(ArchiveFile),
				  PathInsideArchive(PathInsideArchive),
				  CompressedOutputBuffer(1),
				  LocalFileHeaderBuffer(GetLocalFileHeaderSize()),
				  ProgressInfo(ProgressInfo)
			{
				InputFile = std::make_unique<Modio::Detail::File>(SourceFilePath, false);
				OutputFile = std::make_unique<Modio::Detail::File>(ArchiveFile->FilePath, false);
				CompressionStream = std::make_unique<boost::beast::zlib::deflate_stream>();
				InputFileSize = InputFile->GetFileSize();
				IsZip64 = InputFileSize >= (UINT32_MAX - 1);
			};

			template<typename CoroType>
			void operator()(CoroType& Self, Modio::ErrorCode ec = {})
			{
				constexpr std::size_t ChunkOfBytes = 64 * 1024;
				std::shared_ptr<Modio::ModProgressInfo> PinnedProgressInfo = ProgressInfo.lock();

				if (PinnedProgressInfo == nullptr)
				{
					Self.complete(Modio::make_error_code(Modio::GenericError::OperationCanceled));
					return;
				}

				reenter(CoroutineState)
				{
					OutputFile->Seek(Modio::FileOffset(OutputFile->GetFileSize()));
					LocalHeaderOffset = OutputFile->Tell();

					// Skip forward, leaving enough space for our local file header as we have to write that when we're
					// done
					OutputFile->Seek(Modio::FileOffset(GetLocalFileHeaderSize()), SeekDirection::Forward);
					FileDataOffset = OutputFile->Tell();

					while (BytesProcessed < InputFileSize)
					{
						// Set a property to the maximum bytes to read. If the file is smaller than "ChunkOfBytes",
						// it is better to just read FileSize. It also applies to the last part of the file.
						MaxBytesToRead = (BytesProcessed + ChunkOfBytes) < InputFileSize
											 ? ChunkOfBytes
											 : InputFileSize - BytesProcessed;
						// Read in a chunk from the file we're compressing
						yield InputFile->ReadAsync(MaxBytesToRead, InputFileBuffer, std::move(Self));
						if (ec)
						{
							Self.complete(ec);
							return;
						}

						// Doing this in a loop in case ReadAsync stored multiple sub-buffers
						while ((NextBuf = InputFileBuffer.TakeInternalBuffer()))
						{
							// Compress the current sub-buffer
							CompressionState.avail_in = NextBuf->GetSize();
							CompressionState.next_in = NextBuf->Data();
							// A slighly larger CompressedOutputBuffer helps to avoid a case where avail_in
							// does not process all input into the avail_out
							CompressedOutputBuffer = Modio::Detail::Buffer(MaxBytesToRead + 100);
							CompressionState.avail_out = CompressedOutputBuffer.GetSize();
							CompressionState.next_out = CompressedOutputBuffer.Data();
							CompressionStream->write(CompressionState, boost::beast::zlib::Flush::none, ec);
							if (ec && ec != Modio::ZlibError::EndOfStream)
							{
								Self.complete(ec);
								return;
							}

							// As long as the no more "CompressionState.avail_in" bytes remain, calculate the rolling
							// CRC
							if (CompressionState.avail_in == 0)
							{
								// Calculate rolling CRC for this sub-buffer
								InputCRC = Modio::Detail::CRC32(NextBuf.value(), InputCRC);
							}
							else
							{
								// In a very edge scenarios, CompressionState could have some "avail_in" bytes
								// remaining, (despite a larger CompressedOutputBuffer). To make sure those bytes are
								// compressed, a simple solution is outlined below:
								// - Calculate the CRC of the bytes that were passed along
								// - Move the "Seek" pointer of InputFile to the last successful bytes read
								// - Clear the buffer to avoid any data mismatch

								// In case of mismatch between BytesProcessed & total_in, only calculate
								// the portion of NextBuf processed by the CompressionStream
								InputCRC = Modio::Detail::CRC32(NextBuf.value(), InputCRC, CompressionState.avail_in);

								// Then move the offset in the file to the last bytes read + 1, which is the section
								InputFile->Seek(Modio::FileOffset(CompressionState.total_in));

								// Make sure the InputFileBuffer is cleared so it does not try to "Take" a buffer
								// in the next iteration
								InputFileBuffer.Clear();

								// Continue execution as normal, given that possibly avail_out could have something to
								// process
							}

							// Check if we've generated any output yet, ie we've consumed some of the output buffer
							// so avail_out (free space in the output buffer) is now less than it was before
							// This way we're only trying to write compressed data to our output file if there's some
							// data to actually write
							if (CompressionState.avail_out != CompressedOutputBuffer.GetSize())
							{
								yield OutputFile->WriteAsync(
									CompressedOutputBuffer.CopyRange(CompressedOutputBuffer.begin(),
																	 CompressedOutputBuffer.begin() +
																		 CompressedOutputBuffer.GetSize() -
																		 CompressionState.avail_out),
									std::move(Self));

								if (ec)
								{
									Self.complete(ec);
									return;
								}
							}
						}

						// BytesProcessed is correctly assesed after CompressionStream has written
						// all the bytes to the CompressionStream
						BytesProcessed = Modio::FileSize(CompressionState.total_in);
						// Update The ProgressInfo with the BytesProcessed updated
						PinnedProgressInfo->CurrentlyExtractedBytes += BytesProcessed;
					}

					// Finish the zlib stream for the current file
                    // Only with a File that has bytes in it
                    if (InputFileSize > 0)
					{
						// In case the CompressionState still has data available from the last iteration
						// keep the last pointer alive. If not, then apply nullptr
						if (CompressionState.avail_in == 0)
						{
							CompressionState.next_in = nullptr;
						}

						CompressedOutputBuffer = Modio::Detail::Buffer(ChunkOfBytes);
						CompressionState.avail_out = CompressedOutputBuffer.GetSize();
						CompressionState.next_out = CompressedOutputBuffer.Data();
						CompressionStream->write(CompressionState, boost::beast::zlib::Flush::finish, ec);
						if (ec && ec != Modio::ZlibError::EndOfStream)
						{
							Self.complete(ec);
							return;
						}
						// Again, check that the last call to the zlib stream actually produced some data for us
						if (CompressionState.avail_out != CompressedOutputBuffer.GetSize())
						{
							yield OutputFile->WriteAsync(
								CompressedOutputBuffer.CopyRange(CompressedOutputBuffer.begin(),
																 CompressedOutputBuffer.begin() +
																	 CompressedOutputBuffer.GetSize() -
																	 CompressionState.avail_out),
								std::move(Self));
							if (ec)
							{
								Self.complete(ec);
								return;
							}
						}
					}

					EndOffset = OutputFile->Tell();

					if (IsZip64 == false)
					{
						// Marshal fields into our local file header's data buffer
						Modio::Detail::TypedBufferWrite(Constants::ZipTag::LocalDirectoryHeader, LocalFileHeaderBuffer,
														0) // header signature
							.FollowedBy<uint16_t>(Constants::ZipTag::ZipVersion) // Minimum version to extract
							.FollowedBy<uint16_t>(0) // General Purpose bitflag
							.FollowedBy<uint16_t>(Constants::ZipTag::Deflate) // Compression Method
							.FollowedBy<uint16_t>(0) // Last Modified time
							.FollowedBy<uint16_t>(0) // Last Modified date
							.FollowedBy<uint32_t>(InputCRC) // CRC32 of input data
							.FollowedBy<uint32_t>((std::uint32_t) CompressionState.total_out) // Size of output data
							.FollowedBy<uint32_t>((std::uint32_t) CompressionState.total_in) // Size of Input Data
							.FollowedBy<uint16_t>(
								(std::uint16_t) PathInsideArchive.generic_u8string().size()) // Size of filename
							// Size of extra field is 28: Header ID + Size Extra Field +
							// Original uncompressed + Size of compressed data + Offset of local header record
							.FollowedBy<uint16_t>(Constants::ZipTag::TotalExtraFieldSizeFileEntries);
					}
					else
					{
						// Mark the ArchiveFile to handle Zip64 when it will write the Central Directory
						ArchiveFile->bIsZip64 = true;

						// Marshal fields into our local file header's data buffer
						Modio::Detail::TypedBufferWrite(Constants::ZipTag::LocalDirectoryHeader, LocalFileHeaderBuffer,
														0) // header signature
							.FollowedBy<uint16_t>(Constants::ZipTag::Zip64Version) // Minimum version to extract
							.FollowedBy<uint16_t>(0) // General Purpose bitflag
							.FollowedBy<uint16_t>(Constants::ZipTag::Deflate) // Compression Method
							.FollowedBy<uint16_t>(0) // Last Modified time
							.FollowedBy<uint16_t>(0) // Last Modified date
							.FollowedBy<uint32_t>(InputCRC) // CRC32 of input data
							.FollowedBy<uint32_t>(Constants::ZipTag::MAX32) // Size of output data
							.FollowedBy<uint32_t>(Constants::ZipTag::MAX32) // Size of Input Data
							.FollowedBy<uint16_t>(
								(std::uint16_t) PathInsideArchive.generic_u8string().size()) // Size of filename
							// Size of extra field is 28: Header ID + Size Extra Field +
							// Original uncompressed + Size of compressed data + Offset of local header record
							.FollowedBy<uint16_t>(Constants::ZipTag::TotalExtraFieldSizeFileEntries);
					}

					// TypedBufferWrite doesn't currently handle strings so manually marshal the filename
					{
						std::string FileName = PathInsideArchive.generic_u8string();
						std::copy(FileName.begin(), FileName.end(), LocalFileHeaderBuffer.Data() + 30);
					}

					// Add "Extra Field" information regardless of Zip64 status
					{
						Modio::Detail::TypedBufferWrite(
                            Constants::ZipTag::ExtendedInformationFieldHeaderID, LocalFileHeaderBuffer,
							30 + PathInsideArchive.generic_u8string()
									 .size()) // header signature
											  // Here we only need to mention the (un)compressed and offset bytes
							.FollowedBy<uint16_t>(Constants::ZipTag::ExtraFieldSize)
							.FollowedBy<uint64_t>(CompressionState.total_in) // Original uncompressed file size
							.FollowedBy<uint64_t>(CompressionState.total_out) // Size of compressed data
							.FollowedBy<uint64_t>(LocalHeaderOffset); // Offset of local header record
					}

					// Write the local file header at the correct offset so it's just before the compressed data
					yield OutputFile->WriteSomeAtAsync(LocalHeaderOffset, std::move(LocalFileHeaderBuffer),
													   std::move(Self));
					if (ec)
					{
						Self.complete(ec);
						return;
					}

					// Add the entry to the archive file so that it knows what to put in the central directory when it
					// is finalized
					ArchiveFile->AddEntry(PathInsideArchive.generic_u8string(), LocalHeaderOffset,
										  CompressionState.total_out, CompressionState.total_in,
										  ArchiveFileImplementation::CompressionMethod::Deflate, InputCRC);
					Self.complete({});
					return;
				}
			}

		private:
			boost::beast::zlib::z_params CompressionState;
			std::unique_ptr<boost::beast::zlib::deflate_stream> CompressionStream;
			std::shared_ptr<Modio::Detail::ArchiveFileImplementation> ArchiveFile;
			std::unique_ptr<Modio::Detail::File> InputFile;
			std::unique_ptr<Modio::Detail::File> OutputFile;
			std::uint64_t InputFileSize = 0;
			bool IsZip64 = false;
			Modio::filesystem::path PathInsideArchive;
			Modio::Detail::DynamicBuffer InputFileBuffer;
			Modio::Detail::Buffer CompressedOutputBuffer;
			Modio::Detail::Buffer LocalFileHeaderBuffer;
			Modio::FileSize BytesProcessed;
			Modio::FileOffset LocalHeaderOffset;
			Modio::FileOffset FileDataOffset;
			Modio::FileOffset EndOffset;
			std::size_t MaxBytesToRead = 0;
			std::uint32_t InputCRC = 0;
			asio::coroutine CoroutineState;
			Modio::Optional<Modio::Detail::Buffer> NextBuf;
			std::weak_ptr<Modio::ModProgressInfo> ProgressInfo;
			/// @brief Helper function so when we add zip64 support we can easily change how much space to allocate for
			/// the local header
			/// @return
			std::size_t GetLocalFileHeaderSize()
			{
				// Always include the "extra field" length even if it is not used (28 bytes at the end)
				return 30 + PathInsideArchive.generic_u8string().size() + Constants::ZipTag::TotalExtraFieldSizeFileEntries;
			}
		};
#include <asio/unyield.hpp>
	} // namespace Detail
} // namespace Modio
