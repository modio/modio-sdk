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
		class ExtractEntryStoreOp : public Modio::Detail::BaseOperation<ExtractEntryStoreOp>
		{
			struct ExtractEntryImpl
			{
				Modio::Detail::File ArchiveFile;
				std::shared_ptr<ArchiveFileImplementation> ArchiveFileImpl;
				ArchiveFileImplementation::ArchiveEntry EntryToExtract;
				Modio::filesystem::path RootDirectoryToExtractTo;
				Modio::Detail::DynamicBuffer FileData;
				std::uintmax_t BytesProcessed = 0;
				Modio::Detail::File DestinationFile;
				Modio::Optional<std::weak_ptr<Modio::ModProgressInfo>> ProgressInfo;
				std::uintmax_t CurrentBufferSize = 0;
			};

			Modio::StableStorage<ExtractEntryImpl> Impl;
			asio::coroutine CoroutineState;

		public:
			ExtractEntryStoreOp(std::shared_ptr<ArchiveFileImplementation> ArchiveFileImpl,
								ArchiveFileImplementation::ArchiveEntry EntryToExtract,
								Modio::filesystem::path RootDirectoryToExtractTo,
								Modio::Optional<std::weak_ptr<Modio::ModProgressInfo>> ProgressInfo)
			{
				Impl = std::make_shared<ExtractEntryImpl>(ExtractEntryImpl {
					Modio::Detail::File(ArchiveFileImpl->FilePath, false), ArchiveFileImpl, EntryToExtract,
					RootDirectoryToExtractTo, Modio::Detail::DynamicBuffer {}, 0u,
					Modio::Detail::File(RootDirectoryToExtractTo / EntryToExtract.FilePath, true), ProgressInfo});
			};

			ExtractEntryStoreOp(ExtractEntryStoreOp&& Other)
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
						Impl->CurrentBufferSize = Impl->FileData.size();
						Impl->BytesProcessed += Impl->CurrentBufferSize;
						// Checking for size here means we can assume there will be an internal buffer to take from
						// filedata
						if (Impl->CurrentBufferSize == 0)
						{
							Self.complete(Modio::make_error_code(Modio::FilesystemError::ReadError));
							return;
						}

						while (Impl->FileData.size())
						{
						//Can safely assume that we'll get a value from TakeInternalBuffer because we've checked the size
							yield Impl->DestinationFile.WriteAsync(Impl->FileData.TakeInternalBuffer().value(),
																   std::move(Self));
							if (ec)
							{
								Self.complete(ec);
								return;
							}
						}

						// Update progress on how much data we have written to disc
						if (Impl->ProgressInfo.has_value())
						{
							if (!Impl->ProgressInfo->expired())
							{
								Impl->ProgressInfo->lock()->CurrentlyExtractedBytes +=
									Modio::FileSize(Impl->CurrentBufferSize);
							}
							else
							{
								Self.complete(
									Modio::make_error_code(Modio::ModManagementError::InstallOrUpdateCancelled));
								return;
							}
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
