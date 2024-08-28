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
#include "modio/detail/HedleyWrapper.h"
#include "modio/detail/ModioCRC.h"
#include "modio/detail/compression/zip/ArchiveFileImplementation.h"
#include "modio/detail/compression/zip/ZipStructures.h"
#include "modio/detail/compression/zlib/deflate_stream.hpp"
#include "modio/file/ModioFile.h"
#include <algorithm>
#include <memory>

MODIO_DIAGNOSTIC_PUSH

MODIO_ALLOW_DEPRECATED_SYMBOLS

// For reference: https://pkware.cachefly.net/webdocs/casestudies/APPNOTE.TXT

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
						   std::shared_ptr<uint64_t> FileHash, std::weak_ptr<Modio::ModProgressInfo> ProgressInfo)
				: ArchiveFile(ArchiveFile),
				  PathInsideArchive(PathInsideArchive),
				  CompressedOutputBuffer(1),
				  ProgressInfo(ProgressInfo)
			{
				InputFile =
					std::make_unique<Modio::Detail::File>(SourceFilePath, Modio::Detail::FileMode::ReadOnly, false);
				OutputFile = std::make_unique<Modio::Detail::File>(ArchiveFile->FilePath,
																   Modio::Detail::FileMode::ReadWrite, false);
				CompressionStream = std::make_unique<boost::beast::zlib::deflate_stream>();
				FileName = PathInsideArchive.generic_u8string();
				InputFileSize = InputFile->GetFileSize();
				IsZip64 = InputFileSize >= (UINT32_MAX - 1);
				RollingFileHash = FileHash;
			}

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
					// If a file name uses a double dot the operation will fail
					if (InputFile->GetPath().filename().string().find("..") != std::string::npos)
					{
						Modio::Detail::Logger().Log(Modio::LogLevel::Warning, Modio::LogCategory::File,
													"File `{}` uses more than one dot in its name, which is forbidden",
													InputFile->GetPath().filename().string());
						Self.complete(Modio::make_error_code(Modio::FilesystemError::ReadError));
						return;
					}

					// Determine the start of this local file entry in the archive
					// This value will be used to locate this entry as needed
					OutputFile->Seek(Modio::FileOffset(OutputFile->GetFileSize()));
					LocalHeaderOffset = OutputFile->Tell();

					if (IsZip64)
					{
						// Include Zip64 Extended Information Extra Field
						LocalHeaderSize = Constants::ZipTag::LocalFileHeaderSize + FileName.size() +
										  Constants::ZipTag::Zip64LocalFileExtraFieldSize;
						// Mark the ArchiveFile to handle Zip64 when writing the Central Directory
						ArchiveFile->bIsZip64 = true;
					}
					else
					{
						LocalHeaderSize = Constants::ZipTag::LocalFileHeaderSize + FileName.size();
					}

					// Determine the offset for the file data, located immediately after the header
					OutputFile->Seek(Modio::FileOffset(LocalHeaderSize), SeekDirection::Forward);
					FileDataOffset = OutputFile->Tell();

					// Process and compress the file data
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
							// A slightly larger CompressedOutputBuffer helps to avoid a case where avail_in
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

						// BytesProcessed is correctly assessed after CompressionStream has written
						// all the bytes to the CompressionStream
						BytesProcessed = Modio::FileSize(CompressionState.total_in);
						// Update The ProgressInfo with MaxBytesToRead
						IncrementCurrentProgress(*PinnedProgressInfo.get(), Modio::FileSize(MaxBytesToRead));
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

					// Determine the end of this local file entry in the archive
					EndOffset = OutputFile->Tell();

					// Marshal fields of fixed sizes into our local file header's data buffer
					LocalFileHeaderBuffer = std::make_unique<Modio::Detail::Buffer>(LocalHeaderSize);
					// Header signature
					Modio::Detail::TypedBufferWrite(Constants::ZipTag::LocalFileHeaderSignature, *LocalFileHeaderBuffer,
													0)
						// Minimum version to extract
						.FollowedBy<uint16_t>(IsZip64 ? Constants::ZipTag::Zip64Version : Constants::ZipTag::ZipVersion)
						// General Purpose bit-flag
						.FollowedBy<uint16_t>(0)
						// Compression Method
						.FollowedBy<uint16_t>(Constants::ZipTag::Deflate)
						// Last modified time
						.FollowedBy<uint16_t>(0)
						// Last modified date
						.FollowedBy<uint16_t>(0)
						// CRC-32 of uncompressed data
						.FollowedBy<uint32_t>(InputCRC)
						// Compressed size. Must be set to MAX32 if actual value is included in Zip64 Extended
						// Information
						.FollowedBy<uint32_t>(IsZip64 ? Constants::ZipTag::MAX32
													  : std::uint32_t(CompressionState.total_out))
						// Uncompressed size. Must be set to MAX32 if actual value is included in Zip64 Extended
						// Information
						.FollowedBy<uint32_t>(IsZip64 ? Constants::ZipTag::MAX32
													  : std::uint32_t(CompressionState.total_in))
						// File name length
						.FollowedBy<uint16_t>(std::uint16_t(FileName.size()))
						// Extra field length
						.FollowedBy<uint16_t>(IsZip64 ? Constants::ZipTag::Zip64LocalFileExtraFieldSize : 0);

					// Manually write (variable sized) file name
					std::copy(FileName.begin(), FileName.end(),
							  LocalFileHeaderBuffer->Data() + Constants::ZipTag::LocalFileHeaderSize);

					// Include Zip64 Extended Information Extra Field if file size exceeds Zip32 limit
					if (IsZip64)
					{
						Modio::Detail::Logger().Log(
							Modio::LogLevel::Trace, Modio::LogCategory::File,
							"Including Zip64 Extended Information Extra Field in the local directory for file {}",
							FileName);

						// Zip64 Extra Field Signature
						Modio::Detail::TypedBufferWrite(Constants::ZipTag::Zip64ExtraFieldSignature,
														*LocalFileHeaderBuffer,
														Constants::ZipTag::LocalFileHeaderSize + FileName.size())
							// Size of this extra block, excluding leading 4 bytes (signature and size fields)
							.FollowedBy<uint16_t>(Constants::ZipTag::Zip64LocalFileExtraFieldSize - 4)
							// Uncompressed size
							.FollowedBy<uint64_t>(CompressionState.total_in)
							// Compressed size
							.FollowedBy<uint64_t>(CompressionState.total_out);
					}

					// Write the local file header immediately before the compressed data
					yield OutputFile->WriteSomeAtAsync(LocalHeaderOffset, std::move(*LocalFileHeaderBuffer),
													   std::move(Self));

					if (ec)
					{
						Self.complete(ec);
						return;
					}

					// Add this entry to the archive file object
					// These details will be written to the central directory
					ArchiveFile->AddEntry(FileName, LocalHeaderOffset, CompressionState.total_out,
										  CompressionState.total_in,
										  ArchiveFileImplementation::CompressionMethod::Deflate, InputCRC);

					*RollingFileHash = *RollingFileHash ^ InputCRC;

					// Close file handles
					InputFile.reset();
					OutputFile.reset();

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
			std::unique_ptr<Modio::Detail::Buffer> LocalFileHeaderBuffer;
			Modio::FileSize BytesProcessed;
			Modio::FileOffset LocalHeaderOffset;
			Modio::FileOffset FileDataOffset;
			Modio::FileOffset EndOffset;
			std::size_t MaxBytesToRead = 0;
			std::size_t LocalHeaderSize = 0;
			std::uint32_t InputCRC = 0;
			asio::coroutine CoroutineState;
			Modio::Optional<Modio::Detail::Buffer> NextBuf;
			std::shared_ptr<uint64_t> RollingFileHash;
			std::weak_ptr<Modio::ModProgressInfo> ProgressInfo;
			std::string FileName;
		};
#include <asio/unyield.hpp>
	} // namespace Detail
} // namespace Modio

MODIO_DIAGNOSTIC_POP