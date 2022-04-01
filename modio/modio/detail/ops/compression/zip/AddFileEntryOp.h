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
#include "modio/detail/AsioWrapper.h"
#include "modio/detail/FilesystemWrapper.h"
#include "modio/detail/ModioCRC.h"
#include "modio/detail/compression/zip/ArchiveFileImplementation.h"
#include "modio/detail/compression/zlib/deflate_stream.hpp"
#include "modio/file/ModioFile.h"
#include "modio/core/ModioModCollectionEntry.h"
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
						   Modio::filesystem::path SourceFilePath, Modio::filesystem::path PathInsideArchive, std::weak_ptr<Modio::ModProgressInfo> ProgressInfo)
				: ArchiveFile(ArchiveFile),
				  PathInsideArchive(PathInsideArchive),
				  CompressedOutputBuffer(1),
				  LocalFileHeaderBuffer(GetLocalFileHeaderSize()),
				  ProgressInfo(ProgressInfo)
			{
				InputFile = std::make_unique<Modio::Detail::File>(SourceFilePath, false);
				OutputFile = std::make_unique<Modio::Detail::File>(ArchiveFile->FilePath, false);
			};

			template<typename CoroType>
			void operator()(CoroType& Self, Modio::ErrorCode ec = {})
			{
				constexpr std::size_t ChunkOfBytes = 64 * 1024;
				const std::size_t FileSize = InputFile->GetFileSize();
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

					while (BytesProcessed < FileSize)
					{
						// Set a property to the maximum bytes to read. If the file is smaller than "ChunkOfBytes",
						// it is better to just read FileSize. It also applies to the last part of the file.
						MaxBytesToRead = (BytesProcessed + ChunkOfBytes) < FileSize ? ChunkOfBytes : FileSize - BytesProcessed;
						// Read in a chunk from the file we're compressing
						yield InputFile->ReadAsync(MaxBytesToRead, InputFileBuffer, std::move(Self));
						if (ec)
						{
							Self.complete(ec);
							return;
						}

						BytesProcessed += Modio::FileSize(InputFileBuffer.size());
						PinnedProgressInfo->CurrentlyExtractedBytes += Modio::FileSize(InputFileBuffer.size());
						// Doing this in a loop in case ReadAsync stored multiple sub-buffers
						while ((NextBuf = InputFileBuffer.TakeInternalBuffer()))
						{
							// Compress the current sub-buffer
							CompressionState.avail_in = NextBuf->GetSize();
							CompressionState.next_in = NextBuf->Data();
							CompressedOutputBuffer = Modio::Detail::Buffer(MaxBytesToRead);
							CompressionState.avail_out = CompressedOutputBuffer.GetSize();
							CompressionState.next_out = CompressedOutputBuffer.Data();
							CompressionStream.write(CompressionState, boost::beast::zlib::Flush::none, ec);
							if (ec && ec != Modio::ZlibError::EndOfStream)
							{
								Self.complete(ec);
								return;
							}

							// Calculate rolling CRC for this sub-buffer
							InputCRC = Modio::Detail::CRC32(NextBuf.value(), InputCRC);

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
					}

					// Finish the zlib stream for the current file
					{
						CompressionState.avail_in = 0;
						CompressionState.next_in = nullptr;
						CompressedOutputBuffer = Modio::Detail::Buffer(ChunkOfBytes);
						CompressionState.avail_out = CompressedOutputBuffer.GetSize();
						CompressionState.next_out = CompressedOutputBuffer.Data();
						CompressionStream.write(CompressionState, boost::beast::zlib::Flush::finish, ec);
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

					// Marshal fields into our local file header's data buffer
					Modio::Detail::TypedBufferWrite(LocalHeaderMagic, LocalFileHeaderBuffer, 0) // header signature
						.FollowedBy<uint16_t>(20) // Minimum version to extract
						.FollowedBy<uint16_t>(0) // General Purpose bitflag
						.FollowedBy<uint16_t>(8) // Compression Method
						.FollowedBy<uint16_t>(0) // Last Modified time
						.FollowedBy<uint16_t>(0) // Last Modified date
						.FollowedBy<uint32_t>(InputCRC) // CRC32 of input data
						.FollowedBy<uint32_t>((std::uint32_t) CompressionState.total_out) // Size of output data
						.FollowedBy<uint32_t>((std::uint32_t) CompressionState.total_in) // Size of Input Data
						.FollowedBy<uint32_t>(
							(std::uint32_t) PathInsideArchive.generic_u8string().size()) // Size of filename
						.FollowedBy<uint32_t>(
							0); // Size of extra field (currently not in use, will change when zip64 support is added)

					// TypedBufferWrite doesn't currently handle strings so manually marshal the filename
					{
						std::string FileName = PathInsideArchive.generic_u8string();
						std::copy(FileName.begin(), FileName.end(), LocalFileHeaderBuffer.Data() + 30);
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
					ArchiveFile->AddEntry(PathInsideArchive.generic_u8string(), FileDataOffset,
										  CompressionState.total_out, CompressionState.total_in,
										  ArchiveFileImplementation::CompressionMethod::Deflate, InputCRC);
					Self.complete({});
					return;
				}
			}

		private:
			boost::beast::zlib::z_params CompressionState;
			boost::beast::zlib::deflate_stream CompressionStream;
            // This variable had troubles as a constexpr when compiling on macOS + g++11
			const uint32_t LocalHeaderMagic = 0x04034b50;
			std::shared_ptr<Modio::Detail::ArchiveFileImplementation> ArchiveFile;
			std::unique_ptr<Modio::Detail::File> InputFile;
			std::unique_ptr<Modio::Detail::File> OutputFile;
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
				return 30 + PathInsideArchive.generic_u8string().size();
			}
		};
#include <asio/unyield.hpp>
	} // namespace Detail
} // namespace Modio
