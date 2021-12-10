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

#include "modio/detail/compression/zlib/inflate_stream.hpp"
#include "modio/detail/compression/zlib/zlib.hpp"
#include "modio/core/ModioBuffer.h"
#include "modio/core/ModioStdTypes.h"
#include "modio/detail/AsioWrapper.h"
#include "modio/detail/ModioObjectTrack.h"
#include "modio/detail/compression/zip/ArchiveFileImplementation.h"
#include "modio/file/ModioFile.h"
#include <asio/yield.hpp>
#include <cstdint>

namespace Modio
{
	namespace Detail
	{
		class ExtractEntryDeflateOp : public Modio::Detail::BaseOperation<ExtractEntryDeflateOp>
		{
			struct ExtractEntryImpl
			{
				Modio::Detail::File ArchiveFile;
				std::shared_ptr<ArchiveFileImplementation> ArchiveFileImpl;
				ArchiveFileImplementation::ArchiveEntry EntryToExtract;
				Modio::filesystem::path RootDirectoryToExtractTo;
				Modio::Detail::DynamicBuffer FileData;
				std::uintmax_t BytesProcessed;
				Modio::Detail::File DestinationFile;
				Modio::Optional<std::weak_ptr<Modio::ModProgressInfo>> ProgressInfo;

				boost::beast::zlib::inflate_stream ZStream;
				boost::beast::zlib::z_params ZState;
				Modio::ErrorCode DeflateStatus;
				Modio::Optional<Modio::Detail::Buffer> DecompressedData;
			};

			Modio::StableStorage<ExtractEntryImpl> Impl;
			asio::coroutine CoroutineState;

		public:
			ExtractEntryDeflateOp(std::shared_ptr<ArchiveFileImplementation> ArchiveFileImpl,
								  ArchiveFileImplementation::ArchiveEntry EntryToExtract,
								  Modio::filesystem::path RootDirectoryToExtractTo,
								  Modio::Optional<std::weak_ptr<Modio::ModProgressInfo>> ProgressInfo)
			{
				Impl = std::make_shared<ExtractEntryImpl>(ExtractEntryImpl {
					Modio::Detail::File(ArchiveFileImpl->FilePath, false), ArchiveFileImpl, EntryToExtract,
					RootDirectoryToExtractTo, Modio::Detail::DynamicBuffer {}, 0u,
					Modio::Detail::File(RootDirectoryToExtractTo / EntryToExtract.FilePath, true), ProgressInfo});
			};

			ExtractEntryDeflateOp(ExtractEntryDeflateOp&& Other)
				: Impl(std::move(Other.Impl)),
				  CoroutineState(std::move(Other.CoroutineState)) {};

			template<typename CoroType>
			void operator()(CoroType& Self, Modio::ErrorCode ec = {})
			{
				reenter(CoroutineState)
				{
					Modio::Detail::Logger().Log(Modio::LogLevel::Info, Modio::LogCategory::Compression,
												"Extracting entry {}", Impl->EntryToExtract.FilePath.u8string());
					// Set the initial position for reading
					Impl->ArchiveFile.Seek(Modio::FileOffset(Impl->EntryToExtract.FileOffset));

					while (Impl->BytesProcessed < Impl->EntryToExtract.CompressedSize)
					{
						// Don't try to read beyond the end of the compressed file, so make sure to read the minimum of
						// our read chunk size or the remaining bytes for the file entry

						yield Impl->ArchiveFile.ReadAsync(
							std::min<std::uintmax_t>(Impl->EntryToExtract.CompressedSize - Impl->BytesProcessed,
													 64 * 1024),
							Impl->FileData, std::move(Self));

						if (ec && ec != Modio::GenericError::EndOfFile)
						{
							Self.complete(ec);
							return;
						}

						// If the ec is "EndOfFile", the line below should reset its state in case it is used later on.
						ec = {};
						
						Impl->BytesProcessed += Impl->FileData.size();
						// Checking for size here means we can assume there will be an internal buffer to take from
						// filedata
						if (Impl->FileData.size() == 0)
						{
							Self.complete(Modio::make_error_code(Modio::FilesystemError::ReadError));
							return;
						}

						{
							auto FileDataBuffers = Impl->FileData.data();
							Impl->ZState.next_in = FileDataBuffers.begin()->data();
							Impl->ZState.avail_in = FileDataBuffers.begin()->size();
							if (Impl->ZState.avail_in == 0)
							{
								auto thing = 0;
							}
						}

						while (!Impl->DeflateStatus && Impl->ZState.avail_in > 0)
						{
							Impl->DecompressedData = Modio::Detail::Buffer(64 * 1024, 1024 * 4);
							Impl->ZState.next_out = Impl->DecompressedData->Data();
							Impl->ZState.avail_out = Impl->DecompressedData->GetSize();
							Impl->ZState.total_out = 0;

							Impl->ZStream.write(Impl->ZState, boost::beast::zlib::Flush::none, Impl->DeflateStatus);
							if (!Impl->DeflateStatus || Impl->DeflateStatus == Modio::ZlibError::EndOfStream)
							{
								// Copy the required range out of the pre-allocated buffer
								yield Impl->DestinationFile.WriteAsync(
									Impl->DecompressedData->CopyRange(0, Impl->ZState.total_out), std::move(Self));

								// Update progress on how much data we have written to disc
								if (Impl->ProgressInfo.has_value())
								{
									if (!Impl->ProgressInfo->expired())
									{
										Impl->ProgressInfo->lock()->CurrentlyExtractedBytes +=
											Modio::FileSize(Impl->ZState.total_out);
									}
									else
									{
										Self.complete(Modio::make_error_code(
											Modio::ModManagementError::InstallOrUpdateCancelled));
										return;
									}
								}
							}
						}
						// take the first chunk out of the dynamic buffer so that we effectively consume those bytes
						Modio::Optional<Modio::Detail::Buffer> Unused = Impl->FileData.TakeInternalBuffer();

						if (ec && ec != Modio::ZlibError::EndOfStream)
						{
							Self.complete(ec);
							return;
						}
					}
					Impl.reset();
					Self.complete(ec);
				}
			}
		};

	} // namespace Detail
} // namespace Modio

#include <asio/unyield.hpp>
